#!/usr/bin/env python3

import socket
from PIL import ImageFile
import curses

client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)  # UDP
client.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
server.settimeout(0.2)

def px(num):
  if num == 255:
    return 'X'
  elif num == 170:
    return 'X'
  elif num == 85:
    return 'x'
  elif num == 0:
    return ' '
  else:
    return '?'

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

    data, addr = client.recvfrom(4096)
    parser = ImageFile.Parser()
    parser.feed(data)
    picture = parser.close()

    print(chr(27)+'[2j')
    print('\033c')
    print('\x1bc')

    data = list(picture.getdata(0))
    for y in range(32):
      line = data[128*y:(128*(y+1))]
      str = ''.join(map(px, line))
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
