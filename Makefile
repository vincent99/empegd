CFLAGS	:= -Wall -g -O2
LDLIBS	:= -lm
INC	:= -I../

CC    	:= /usr/local/armtools-empeg/bin/arm-empeg-linux-gcc
STRIP 	:= /usr/local/armtools-empeg/bin/arm-empeg-linux-strip
AR			:= /usr/local/armtools-empeg/bin/arm-empeg-linux-ar
RANLIB	:= /usr/local/armtools-empeg/bin/arm-empeg-linux-ranlib

#-----------------------

all: empegd

empegd: empegd.c
	$(CC) $(CFLAGS) $(INC) -c empegd.c -o display

clean: 
	rm  *.o empegd
