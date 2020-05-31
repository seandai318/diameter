PROJECT_DIR = /home/ama/project
PROJECT_APP_DIR = $(PROJECT_DIR)/diameter
IDIR = $(PROJECT_DIR)/os/include $(PROJECT_APP_DIR)/common/include $(PROJECT_APP_DIR)/diaBase/include $(PROJECT_APP_DIR)/diaCx/include $(PROJECT_APP_DIR)/diaSh/include $(PROJECT_DIR)/sip-stack/transport/include
#INC=$(foreach d, $(IDIR), -I$d)
INC=$(IDIR:%=-I%)
AR=ar

src = $(wildcard *.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)  # one dependency file for each source

CFLAGS=$(INC) -g -DPREMEM
DEBUG = true
ifeq ($(DEBUG), true)
    override CFLAGS += -DDEBUG -DPREMEM_DEBUG
endif

LDFLAGS = -lpthread

libdiacommon.a: $(obj)
	$(AR) -cr $@ $^

-include $(dep)   # include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.c
	@mkdir -p $(dir $@)
	$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@


.PHONY: clean
clean:
	rm -f $(dep) *.o *.a

.PHONY: cleandep
cleandep:
	rm -f $(dep)
