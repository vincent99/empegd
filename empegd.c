#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
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
#include <hijack.h>
#include <linux/empeg.h>

#define RAW_W 128
#define RAW_H 32
#define RAW_PPB 2
#define RAW_ROWBYTES RAW_W/RAW_PPB
#define RAW_SIZE RAW_ROWBYTES * RAW_H

static int sock;
static struct sockaddr_in server_addr, client_addr;
static unsigned int client_addr_len;

static char rawBmp[128 * 32 / 8];
static char *raw = rawBmp;
static int screen;

static int kbd;
static unsigned long buttons[2] = { 2, 0 };
static int;


int main( int argc, char *argv[]) {
  if ( fork() != 0 ) {
    return 0;
  }

  int pid = getpid();
  sprintf("empegd started, pid=%d\n", pid);

  screen = open("/proc/empeg_screen.raw", O_RDONLY);
  kbd = open("/dev/display", O_WRONLY);

  int port = atoi(argv[1]);
  char input[512];
  int res;

  /*Create socket */
  sock=socket(AF_INET, SOCK_DGRAM, 0);
  if(sock == -1) {
    perror("Error: socket failed");
  }

  bzero((char*) &server_addr, sizeof(server_addr));
  server_addr.sin_family=AF_INET;
  server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  server_addr.sin_port=htons(port);

  /*Bind server socket and listen for incoming clients*/
  res = bind(sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr));
  if(res == -1)
      perror("Error: bind call failed");

  sprintf("empegd listening on port=%d\n", port);

   while ( 1 ) {
    client_addr_len = sizeof(client_addr);
    res = recvfrom(sock, input, strlen(input)+1, 0, (struct sockaddr*) &client_addr, &client_addr_len);
    if ( res == -1 ) {
        perror("Error: recvfrom call failed");
    }

    printf("SERVER: read %d bytes from %s: %s\n", res, inet_ntoa(client_addr.sin_addr), input);

    if ( input[0] == 'q' || input[0] == 'Q' ) {
      break;
    } else if ( input[0] == 'f' || input[0] == 'F' ) {
      frame();
    } else {
      key(input[0]);
    }
  }

  res = close(sock);
  if(res == -1) {
    perror("Error: bind call failed");
  }
}

void frame() {
  lseek(screen, 0, SEEK_SET);
  read(screen, raw, RAW_SIZE);

  int res = sendto(sock, raw, RAW_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(client_addr));
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
