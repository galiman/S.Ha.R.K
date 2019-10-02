#
# util_linux -> compiles the tracer utilities under Linux
#

ifndef BASE
BASE=../../..
endif

.PHONY: util_dos util_linux clean

all:
	@echo Targets: util_dos util_linux clean
	@echo Note: udpdump and list are available only under linux

util_linux: udpdump list pwcet_gen extract

udpdump: udpdump.c
	gcc -Wimplicit-function-declaration -Wall -ggdb\
	-I$(BASE)/tracer/include -I$(BASE)/oslib udpdump.c -o udpdump

list: list.c
	gcc -Wimplicit-function-declaration -Wall -ggdb\
        -I$(BASE)/tracer/include -I$(BASE)/oslib list.c -o list

pwcet_gen: pwcet_gen.c
	gcc -Wimplicit-function-declaration -Wall -ggdb\
	-I$(BASE)/tracer/include -I$(BASE)/oslib pwcet_gen.c -o pwcet_gen

extract: extract.c
	gcc -Wimplicit-function-declaration -Wall -ggdb -lm\
	-I$(BASE)/tracer/include -I$(BASE)/oslib extract.c -o extract

clean:
	rm -rf *.o udpdump list pwcet_gen extract


