include $(AM_HOME)/Makefile.check
.PHONY: all run clean latest $(ALL)

ALL = $(basename $(notdir $(shell find tests/. -name "*.c")))

all: $(addprefix Makefile., $(ALL))
	@echo "" $(ALL)

$(ALL): %: Makefile.%

Makefile.%: tests/%.c latest
	@/bin/echo -e "NAME = $*\nSRCS = $<\nLIBS += klib\ninclude $${AM_HOME}/Makefile.app" > $@
	-@make -s -f $@ ARCH=$(ARCH) $(MAKECMDGOALS)
	-@rm -f Makefile.$*

run: all

rundummy:
	make ARCH=x86-nemu ALL=dummy run

clean: 
	rm -rf Makefile.* build/

latest:
