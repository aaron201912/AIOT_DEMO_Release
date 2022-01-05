ALKAID_PATH ?= $(shell pwd)/../..
include ../config.mk
$(info mysdk=$(ALKAID_PATH))
DEP_LIB := libsstar.a







$(DEP_LIB):
	@$(MAKE) -C ../common $@;
#	$(TARGET_NAME): $(CPP_OBJS) $(CPP_SRCS) $(C_OBJS) $(C_SRCS)
#	@echo "generate $@"
#	@$(CC) -o $@ $(C_OBJS) $(CPP_OBJS) $(LIB_PATH) $(LIB_NAME) -lm -lpthread
#


$(TARGET_NAME): $(CPP_OBJS) $(CPP_SRCS) $(C_OBJS) $(C_SRCS)  $(DEP_LIB)
	@echo "generate $@"
	@$(CC) -o $@  $(C_OBJS) $(CPP_OBJS) $(LIB_PATH) $(LIB_NAME) -lm -lpthread
	@$(MAKE) finish

