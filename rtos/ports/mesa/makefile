
dirs := $(filter-out CVS cvs makefile readme include, $(wildcard *))
p_all := $(addprefix prefixall_, $(dirs))
p_install := $(addprefix prefixinstall_, $(dirs))
p_clean := $(addprefix prefixclean_, $(dirs))
pcleanall := $(addprefix prefixcleanall_, $(dirs))
pdepend := $(addprefix prefixdepend_, $(dirs))


.PHONY: all install clean cleanall depend

install: $(p_install)

all: $(p_all)

clean: $(p_clean)

cleanall: $(p_cleanall)

depend: $(pdepend)

prefixinstall_%:
	make -C $* install

prefixall_%:
	make -C $* all

prefixclean_%:
	make -C $* clean

prefixcleanall_%:
	make -C $* cleanall

prefixdepend_%:
	make -C $* depend








