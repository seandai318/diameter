PROJECT_DIR ?= ${HOME}/project
PROJECT_APP_DIR = $(PROJECT_DIR)/diameter
IDIR = $(PROJECT_DIR)/os/include $(PROJECT_APP_DIR)/common/include $(PROJECT_APP_DIR)/diaBase/include $(PROJECT_APP_DIR)/diaCx/include $(PROJECT_APP_DIR)/diaSh/include $(PROJECT_DIR)/sip-stack/transport/include
#INC=$(foreach d, $(IDIR), -I$d)
INC=$(IDIR:%=-I%)

SRC_DIR = $(PROJECT_APP_DIR)/common/src
OBJ_DIR = $(PROJECT_APP_DIR)/common/debug
src = $(wildcard $(SRC_DIR)/*.c)
obj = $(src:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
dep = $(obj:.o=.d)  # one dependency file for each source

ifdef $(APP_BASE)
    include $(APP_BASE)/Makefile.cflags
else
    CC=gcc
    AR=ar
    CFLAGS=$(INC) -g -DPREMEM -std=gnu99
    DEBUG = true
    ifeq ($(DEBUG), true)
        override CFLAGS += -DDEBUG -DPREMEM_DEBUG
    endif
endif

LDFLAGS = -lpthread

libdiacommon.a: $(obj)
	cd $(OBJ_DIR);	$(AR) -cr $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	cd $(SRC_DIR); $(CC) $(CFLAGS) -c $(notdir $<) -o $@
	
-include $(dep)   # include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@


.PHONY: clean
clean:
	rm -f $(dep) $(obj) $(OBJ_DIR)/*.a

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: all
all:
	$(info $(obj))
	$(info $(dep))
	@mkdir -p test
