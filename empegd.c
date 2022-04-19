#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "hijack.h"
#include "empeg.h"

#define RAW_W 128
#define RAW_H 32
#define RAW_PPB 2
#define RAW_ROWBYTES RAW_W/RAW_PPB
#define RAW_SIZE RAW_ROWBYTES * RAW_H
#define PNG_SIZE 1152

#define INPUT_BUF 512
#define PORT 1024

static int sock;
static struct sockaddr_in bcast_addr, server_addr, client_addr;
static unsigned int client_addr_len;

static char rawBmp[128 * 32 / 8];
static char *raw = rawBmp;
static int screen;

static int kbd;
static unsigned long buttons[2] = { 2, 0 };

void frame() {
  int res;

  lseek(screen, 0, SEEK_SET);
  read(screen, raw, PNG_SIZE);

  res = sendto(sock, raw, PNG_SIZE, 0, (struct sockaddr*) &bcast_addr, sizeof(bcast_addr));
  if ( res == -1 ) {
    perror("Error: sendto call failed");
  }
}

void key(char input) {
  int valid = 1;

  switch ( input ) {
    case '+':	// Knob Right
      buttons[1] = IR_KNOB_RIGHT;
      break;
    case '-':	// Knob Left
      buttons[1] = IR_KNOB_LEFT;
      break;
    case 'K':	// Knob Button
      buttons[1] = IR_KNOB_PRESSED;
      break;
    case 'k':
      buttons[1] = IR_KNOB_RELEASED;
      break;

    case 'U':	// Up
      buttons[1] = IR_TOP_BUTTON_PRESSED;
      break;
    case 'u':
      buttons[1] = IR_TOP_BUTTON_RELEASED;
      break;

    case 'D':	// Down
      buttons[1] = IR_BOTTOM_BUTTON_PRESSED;
      break;
    case 'd':
      buttons[1] = IR_BOTTOM_BUTTON_RELEASED;
      break;

    case 'L':	// Left
      buttons[1] = IR_LEFT_BUTTON_PRESSED;
      break;
    case 'l':
      buttons[1] = IR_LEFT_BUTTON_RELEASED;
      break;

    case 'R':	// Right
      buttons[1] = IR_RIGHT_BUTTON_PRESSED;
      break;
    case 'r':
      buttons[1] = IR_RIGHT_BUTTON_RELEASED;
      break;

    case 'X':	// Button 11
      buttons[1] = IR_RIO_2_PRESSED;
      break;
    case 'x':
      buttons[1] = IR_RIO_2_RELEASED;
      break;

    case 'V':	// Button 12
      buttons[1] = IR_RIO_INFO_PRESSED;
      break;
    case 'v':
      buttons[1] = IR_RIO_INFO_RELEASED;
      break;

    case 'B':	// Button 13
      buttons[1] = IR_RIO_8_PRESSED;
      break;
    case 'b':
      buttons[1] = IR_RIO_8_RELEASED;
      break;

    case 'M':	// Button 21
      buttons[1] = IR_RIO_5_PRESSED;
      break;
    case 'm':
      buttons[1] = IR_RIO_5_RELEASED;
      break;

    case 'C':	// Button 22
      buttons[1] = IR_RIO_VISUAL_PRESSED;
      break;
    case 'c':
      buttons[1] = IR_RIO_VISUAL_RELEASED;
      break;

    case 'N':	// Button 23
      buttons[1] = IR_RIO_0_PRESSED;
      break;
    case 'n':
      buttons[1] = IR_RIO_0_RELEASED;
      break;

    default:
      printf("Unknown key\n");
      valid = 0;
  }

  if ( valid ) {
    ioctl(kbd, EMPEG_HIJACK_INJECTBUTTONS, &buttons);
  }
}

int main( int argc, char *argv[]) {
  int pid, i;
  char input[INPUT_BUF];
  int res;

  client_addr_len = sizeof(client_addr);

//  if ( fork() != 0 ) {
//    return 0;
//  }
//
//  pid = getpid();
//  printf("empegd started, pid=%d\n", pid);

  screen = open("/proc/empeg_screen.png", O_RDONLY);
  kbd = open("/dev/display", O_WRONLY);

  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if ( sock == -1 ) {
    perror("Error: socket failed");
    exit(1);
  }

  int tmp = 1;
  res = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &tmp, sizeof(tmp));
  if ( res ) {
    perror("Error: enabling broadcast");
    close(sock);
    exit(1);
  }

  tmp = 8192;
  res = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &tmp, sizeof(tmp));
  if ( res ) {
    perror("Error: configuring buffer");
    close(sock);
    exit(1);
  }

  memset(&bcast_addr, 0, sizeof(bcast_addr));
  bcast_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "255.255.255.255", &bcast_addr.sin_addr);
  bcast_addr.sin_port = htons(PORT);

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);
  server_addr.sin_port = htons(PORT+1);

  res = bind(sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr));
  if(res == -1) {
    perror("Error: bind failed");
  }

  fcntl(sock, F_SETFL, O_NONBLOCK);

  printf("empegd listening on port=%d\n", PORT);

   while ( 1 ) {
    frame();

    res = recvfrom(sock, input, INPUT_BUF, 0, (struct sockaddr*) &client_addr, &client_addr_len);
    if ( res > 0 ) {
      for ( i = 0 ; i < res ; i++ ) {
        if ( input[i] == 'q' || input[i] == 'Q' ) {
          break;
        } else {
          key(input[i]);
        }
      }
    }

    sleep(1);
  }

  res = close(sock);
  if(res == -1) {
    perror("Error: bind call failed");
  }

  return 0;
}
