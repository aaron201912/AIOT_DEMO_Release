

ALKAID_PATH ?= $(shell pwd)/..
include ./config.mk


$(info mysdk=$(ALKAID_PATH))



dirs := $(shell pwd)
TARGET_NAMES += $(foreach dir, $(dirs), $(wildcard $(dir)/*/.)) 
TARGET_DIR := $(TARGET_NAMES:%/.=%)
BIN := $(notdir $(TARGET_DIR))
TARGET_BIN := $(foreach dir, $(TARGET_DIR), $(dir)/$(notdir $(dir)))






export ALKAID_PATH


.PHONY: all prepare clean $(TARGET_DIR) $(BIN)

all:  $(BIN) 
	$(foreach dir,$(BIN),$(MAKE) -C $(dir) $@;)
	


clean:
	rm -Rf $(CPP_OBJS)
	rm -f $(C_OBJS)
	$(foreach dir,$(BIN),$(MAKE) -C $(dir) $@;)



$(foreach dir,$(BIN),$(dir)) : $(DEP_LIB)
	@echo "<<<<========================================================"
	@echo "create $@"
	@$(MAKE) -C $@ all; \

