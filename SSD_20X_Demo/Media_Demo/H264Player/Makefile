CROSS_COMPILE ?=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

ALKAID_PATH ?= ./..

ENABLE_HDMI ?= 0

dirs := ./src
OUTPUT_DIR := ./out
$(info mysdk=$(ALKAID_PATH))
COM_FLAGS = -Wall -g -O2 -fPIC -march=armv7-a -mtune=cortex-a7 -mfpu=neon -marm
#COM_FLAGS += -mfloat-abi=hard
C_FLAGS  = $(COM_FLAGS) -std=gnu11
CPP_FLAGS  = $(COM_FLAGS) -std=gnu++11

INCLUDES  := -I./include -I$(ALKAID_PATH)/project/release/include

TARGET_NAME  := SsPlayer

CPP_SRCS := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))
CPP_OBJS := $(foreach n,$(CPP_SRCS),$(addsuffix .cpp.o,$(basename ${n})))

C_SRCS := $(foreach dir,$(dirs),$(wildcard $(dir)/*.c))
C_OBJS := $(foreach n,$(C_SRCS),$(addsuffix .c.o,$(basename ${n})))

LIB_PATH  := -L./lib -L$(ALKAID_PATH)/project/release/nvr/i2m/common/glibc/8.2.1/mi_libs/dynamic
LIB_NAME := -lm -lmi_vdec -lmi_sys -lmi_disp -lmi_gfx -lmi_ao -lmi_common -ldl


ifeq ($(ENABLE_HDMI), 1)
LIB_PATH += -lmi_hdmi
else
LIB_NAME += -lmi_panel
endif
.PHONY: all prepare clean

all: prepare $(TARGET_NAME) finish

prepare:
	@echo
	@echo ">>>>========================================================"
	@echo "TARGET_NAME = $(TARGET_NAME)"
	@echo

clean:
	@rm -Rf $(CPP_OBJS)
	@rm -f $(C_OBJS)
	@rm -Rf $(OUTPUT_DIR)

finish:
	@echo "<<<<========================================================"
	@rm -Rf $(CPP_OBJS)
	@rm -f $(C_OBJS)
	@mkdir -p $(OUTPUT_DIR)
	@mv $(TARGET_NAME) $(OUTPUT_DIR) -v
	@echo "make Done"
	@echo

$(TARGET_NAME): $(CPP_OBJS) $(CPP_SRCS) $(C_OBJS) $(C_SRCS)
	@echo "generate $@"
	@$(CC) -o $@ $(C_OBJS) $(CPP_OBJS) $(LIB_PATH) $(LIB_NAME) -lm -lpthread

%.c.o : %.c
	@echo "compile $@"
	@$(CC) -DENABLE_HDMI=$(ENABLE_HDMI)  $(C_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

%.cpp.o : %.cpp
	@echo "compile $@"
	@$(CPP) -DENABLE_HDMI=$(ENABLE_HDMI)  $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@
