include ../config.mk

ALKAID_PATH ?= $(shell pwd)/../..






TARGET_LIBS  := libsstar.a




$(TARGET_NAME): $(TARGET_LIBS)
 
$(TARGET_LIBS): $(CPP_OBJS) $(CPP_SRCS) $(C_OBJS) $(C_SRCS)
	@$(AR) crv $@ $^




