#
# Main S.Ha.R.K. makefile
#

ifndef BASE
BASE=.
endif

include $(BASE)/config/config.mk

#

.PHONY: install all clean cleanall distrib depend

install all clean cleanall depend:
	make -C $(OSLIB) $@ 
	make -C kernel $@ 
	make -C modules $@ 
	make -C drivers $@
	make -C lib $@
	make -C libc $@
	make -C ports $@
	make -C tracer $@
ifeq ($(findstring YES,$(SHARK_FS)) , YES)
	make -C fs $@
endif

