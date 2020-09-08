PROJECT_DIR = ${HOME}/project
PROJECT_APP_DIR = $(PROJECT_DIR)/diameter
IDIR = ../include $(PROJECT_DIR)/os/include $(PROJECT_APP_DIR)/common/include $(PROJECT_APP_DIR)/diaBase/include $(PROJECT_APP_DIR)/diaCx/include
#INC=$(foreach d, $(IDIR), -I$d)
INC=$(IDIR:%=-I%)
CC=gcc

src = $(wildcard *.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)  # one dependency file for each source

CFLAGS=$(INC) -g -DPREMEM
DEBUG = true
ifeq ($(DEBUG), true)
    override CFLAGS += -DDEBUG -DPREMEM_DEBUG 
endif


libdia.a: libdiacommon.a libdiabase.a libdiacx.a
	@mkdir -p $(PROJECT_APP_DIR)/bin-debug
	$(AR) -cr $@ $(PROJECT_APP_DIR)/bin-debug/*.o
#	$(AR) -cr $@ $^
#	rm -rf $^
#	@rm -rf $(PROJECT_APP_DIR)/bin-debug

-include $(dep)   # include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.c
	@mkdir -p $(dir $@)
	$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: libdiacommon.a
libdiacommon.a:
	@mkdir -p $(PROJECT_APP_DIR)/bin-debug
	cd $(PROJECT_APP_DIR)/common; $(MAKE); cd debug; cp $@ $(PROJECT_APP_DIR)/bin-debug; cd $(PROJECT_APP_DIR)/bin-debug; $(AR) -x $@
#	cd $(PROJECT_APP_DIR)/common; $(MAKE); cd debug; cp $@ $(PROJECT_APP_DIR); cd $(PROJECT_APP_DIR)

.PHONY: libdiabase.a
libdiabase.a:
	@mkdir -p $(PROJECT_APP_DIR)/debugBin
	cd $(PROJECT_APP_DIR)/diaBase; $(MAKE); cd debug; cp $@ $(PROJECT_APP_DIR)/bin-debug; cd $(PROJECT_APP_DIR)/bin-debug; $(AR) -x $@
#	cd $(PROJECT_APP_DIR)/diaBase; $(MAKE); cd debug; cp $@ $(PROJECT_APP_DIR); cd $(PROJECT_APP_DIR)

.PHONY: libdiacx.a
libdiacx.a:
	@mkdir -p $(PROJECT_APP_DIR)/debugBin
	cd $(PROJECT_APP_DIR)/diaCx; $(MAKE); cd debug; cp $@ $(PROJECT_APP_DIR)/bin-debug; cd $(PROJECT_APP_DIR)/bin-debug; $(AR) -x $@
#	cd $(PROJECT_APP_DIR)/diaCx; $(MAKE); cd debug; cp $@ $(PROJECT_APP_DIR); cd $(PROJECT_APP_DIR)

.PHONY: clean
clean: diacommonclean diabaseclean diacxclean
	rm -f $(dep)
	rm -rf $(PROJECT_APP_DIR)/bin-debug
	rm -rf *.a

.PHONY: diacommonclean
diacommonclean:
	cd $(PROJECT_APP_DIR)/common/debug; rm -f *.o *.a *.d; cd -

.PHONY: diabaseclean
diabaseclean:
	cd $(PROJECT_APP_DIR)/diaBase/debug; rm -f *.o *.a *.d; cd -

.PHONY: diacxclean
diacxclean:
	cd $(PROJECT_APP_DIR)/diaCx/debug; rm -f *.o *.a *.d; cd -


.PHONY: cleandep
cleandep:
	rm -f $(dep)
