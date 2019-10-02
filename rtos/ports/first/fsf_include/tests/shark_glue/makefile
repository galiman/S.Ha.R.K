#
# util_linux -> compiles the tracer utilities under Linux
#

ifndef BASE
BASE=../../..
endif

.PHONY: util_dos util_linux clean

all: udpdump

udpdump: udpdump.c
	gcc -Wimplicit-function-declaration -Wall -ggdb\
	-I$(BASE)/tracer/include -I$(BASE)/oslib udpdump.c -o udpdump

clean:
	rm -rf *.o udpdump


