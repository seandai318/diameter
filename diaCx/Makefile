PROJECT_DIR ?= ${HOME}/project
PROJECT_APP_DIR = $(PROJECT_DIR)/diameter
IDIR = $(PROJECT_APP_DIR)/diaCx/include $(PROJECT_DIR)/os/include $(PROJECT_APP_DIR)/common/include $(PROJECT_DIR)/sip-stack/transport/include
#INC=$(foreach d, $(IDIR), -I$d)
INC=$(IDIR:%=-I%)

SRC_DIR = $(PROJECT_APP_DIR)/diaCx/src
OBJ_DIR = $(PROJECT_APP_DIR)/diaCx/debug
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

libdiacx.a: $(obj)
	@mkdir -p $(dir $<)
	cd $(OBJ_DIR);  $(AR) -cr $@ $^; cd -

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	cd $(SRC_DIR); $(CC) $(CFLAGS) -c $(notdir $<) -o $@

-include $(dep)   # include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@


.PHONY: clean
clean:
	rm -f $(obj) $(OBJ_DIR)/*.a
	rm -f $(dep)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
