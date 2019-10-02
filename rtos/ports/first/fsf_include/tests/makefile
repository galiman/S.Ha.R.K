ifndef OS
OS=S
endif

ifndef WL
WL=0
endif

ifeq ($(OS), S) 
include ../../../../shark.cfg
endif

ifeq ($(OS), M)
endif

export OS
export WL

ifeq ($(findstring NEW,$(TRACER)) , NEW)
TRC = newtrace
endif

dirs := $(filter-out CVS cvs tefses shark_glue marte_glue common tabledriven makefile readme.txt newtrace README.TXT, $(wildcard *))
dirs += $(TRC)
p_all := $(addprefix prefixall_, $(dirs))
p_install := $(addprefix prefixinstall_, $(dirs))
p_clean := $(addprefix prefixclean_, $(dirs))
pcleanall := $(addprefix prefixcleanall_, $(dirs))
pdepend := $(addprefix prefixdepend_, $(dirs))


.PHONY: all clean



all: $(p_all)

clean: $(p_clean)

prefixall_%:
	make -C $*

prefixclean_%:
	make -C $* clean
