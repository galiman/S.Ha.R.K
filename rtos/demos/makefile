include ../shark.cfg

ifeq ($(findstring NEW,$(TRACER)) , NEW)
TRC = newtrace
endif

dirs := $(filter-out CVS cvs first dynademo newtrace makefile readme.txt README.TXT myscript.sh, $(wildcard *))
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
