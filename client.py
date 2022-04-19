#!/usr/bin/env python3

import socket
#from PIL import ImageFile
import curses

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)  # UDP
client.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
server.settimeout(0.2)

def px(val, high):
  if high:
    num = (val & 0b11110000) >> 4
  else:
    num = val & 0b00001111

  if num == 15:
    return '█'
  elif num > 2:
    return '▓'
  elif num == 2:
    return '▒'
  elif num == 1:
    return '░'
  else:
    return ' '

client.bind(("", 1024))

def write(key):
  print("write:", key,"\n")
  server.sendto(bytes(key,'ascii'), ("255.255.255.255", 1025))

def main(scr):
  curses.noecho()
  curses.cbreak()

  while True:
    scr.clear()
    scr.keypad(True)
    scr.nodelay(True)

    data, addr = client.recvfrom(2048)

    print(chr(27)+'[2j')
    print('\033c')
    print('\x1bc')

    for y in range(32):
      line = [' ']*128
      for x in range(64):
        line[(2*x)  ] = px(data[64*y + x], False)
        line[(2*x)+1] = px(data[64*y + x], True)

      str = ''.join(line)
      scr.addstr(y, 0, str)

    scr.refresh()
    key = scr.getch()
    if key != -1:
      print("Key", key)

    if key == curses.KEY_UP:
      write('Uu')
    elif key == curses.KEY_DOWN:
      write('Dd')
    elif key == curses.KEY_LEFT:
      write('Ll')
    elif key == curses.KEY_RIGHT:
      write('Rr')
    elif key == ord(',') or key == ord('-'):
      write('-')
    elif key == ord('.') or key == ord('='):
      write('+')
    elif key == 10:
      write('Kk')
    elif key == ord('q'):
      write('q')
    elif key >= ord('A') and key <= ord('z'):
      write(chr(key))


curses.wrapper(main)
curses.clear()
curses.nocbreak()
curses.echo()
curses.endwin()
