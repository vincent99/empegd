CFLAGS	:= -Wall -g -O2
LDLIBS	:= -lm
INC	:= -I../

CC    	:= /usr/local/empeg-toolchain/bin/arm-empeg-linux-gcc
STRIP 	:= /usr/local/empeg-toolchain/bin/arm-empeg-linux-strip
AR	:= /usr/local/empeg-toolchain/bin/arm-empeg-linux-ar
RANLIB	:= /usr/local/empeg-toolchain/bin/arm-empeg-linux-ranlib

#-----------------------

all: empegd

empegd: empegd.c
	$(CC) $(CFLAGS) $(LDLIBS) $(INC) empegd.c -o empegd

clean: 
	rm  *.o empegd
