CROSS_COMPILE ?=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

$(info mysdk=$(ALKAID_PATH))
COM_FLAGS = -Wall -g -O2 -fPIC -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -marm
#COM_FLAGS += -mfloat-abi=hard
C_FLAGS  = $(COM_FLAGS) -std=gnu11
CPP_FLAGS  = $(COM_FLAGS) -std=gnu++11

ENABLE_HDMI ?= 0

dirs := $(shell pwd)
OUTPUT_DIR := ./out


INCLUDES  :=  -I$(ALKAID_PATH)/fb_stable/common/ -I$(ALKAID_PATH)/project/release/include
LIB_PATH  := -L$(ALKAID_PATH)/fb_stable/common/ -L$(ALKAID_PATH)/project/release/nvr/i2m/common/glibc/8.2.1/mi_libs/dynamic
LIB_NAME := -lm -lmi_vdec -lmi_sys -lmi_disp -lmi_gfx -lmi_ao -lmi_common -ldl -lsstar



TARGET_NAME  := $(notdir $(dirs:%/=%))

CPP_SRCS := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))
CPP_OBJS := $(foreach n,$(CPP_SRCS),$(addsuffix .cpp.o,$(basename ${n})))

C_SRCS := $(foreach dir,$(dirs),$(wildcard $(dir)/*.c))
C_OBJS := $(foreach n,$(C_SRCS),$(addsuffix .c.o,$(basename ${n})))


ifeq ($(ENABLE_HDMI), 1)
LIB_PATH += -lmi_hdmi
else
LIB_NAME += -lmi_panel
endif
.PHONY: all prepare clean
all:  $(TARGET_NAME) 
prepare:
	@echo
	@echo ">>>>========================================================"
	@echo "TARGET_NAME = $(TARGET_NAME)"
	@echo

clean:
	@rm -Rf $(CPP_OBJS)
	@rm -f $(C_OBJS)
	@rm -Rf $(OUTPUT_DIR)
	-rm -f $(ALKAID_PATH)/fb_stable/common/libsstar.a

finish:
	@echo "<<<<========================================================"
	@rm -Rf $(CPP_OBJS)
	@rm -f $(C_OBJS)
	@mkdir -p $(OUTPUT_DIR)
	@mv $(TARGET_NAME) $(OUTPUT_DIR) -v
	@echo "make Done"
	@echo


%.c.o : %.c
	@echo "compile $@"
	@$(CC) -DENABLE_HDMI=$(ENABLE_HDMI)  $(C_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

%.cpp.o : %.cpp
	@echo "compile $@"
	@$(CPP) -DENABLE_HDMI=$(ENABLE_HDMI)  $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@
