#
#
#

.PHONY: test all install clean cleanall
.PHONY: $(PROGS)

SUBMAKE:=$(BASE)/config/example2.mk

test: $(PROGS)

all install:

clean::
	$(RM) *.o
	$(RM) *.err
	-rm -f $(PROGS)

cleanall:: clean
	$(RM) deps*
	$(RM) *.bak
	$(RM) *.~
	$(RM) *.?~
	$(RM) *.??~

