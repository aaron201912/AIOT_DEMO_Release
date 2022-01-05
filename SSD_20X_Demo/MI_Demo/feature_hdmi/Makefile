CROSS_COMPILE ?=arm-linux-gnueabihf-
CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
AR  = $(CROSS_COMPILE)ar

COM_FLAGS = -Wall -O2 -fPIC -mcpu=cortex-a9 -mfpu=neon-fp16 -mfloat-abi=hard -mthumb-interwork -marm
C_FLAGS  = $(COM_FLAGS) -std=gnu11
C_FLAGS += -DHDMI_AUDIO_DEVID=2
CPP_FLAGS  = $(COM_FLAGS) -std=gnu++11

INCLUDES  += -I./inc  -I$(ALKAID_PATH)/project/release/include 
INCLUDES  += -I$(ALKAID_PATH)/project/kbuild/4.9.84/i2m/include/uapi/mstar


TARGET_NAME  = feature_hdmi



C_SRCS    =  hdmi.c common/dictionary.c common/iniparser.c common/strlib.c



CPP_OBJS = $(patsubst %.cpp, %.cpp.o, $(CPP_SRCS))
C_OBJS = $(patsubst %.c, %.c.o, $(C_SRCS))

LIB_PATH  = -L. -L$(ALKAID_PATH)/project/release/nvr/i2m/common/glibc/8.2.1/mi_libs/dynamic
LIB_NAME += -lm -lrt -lmi_vdec -lmi_sys -ldl -lmi_hdmi -lmi_disp -lmi_ao  -ldl -lmi_common


.PHONY: all prepare clean

all:  $(TARGET_NAME) 




clean:
	@rm -Rf $(CPP_OBJS)
	@rm -f $(C_OBJS)
	@rm -Rf $(TARGET_NAME)



$(TARGET_NAME): $(CPP_OBJS) $(CPP_SRCS) $(C_OBJS) $(C_SRCS)
	@echo "generate $@"
	@$(CC) -o $@ $(C_OBJS) $(CPP_OBJS) $(LIB_PATH) $(LIB_NAME) -lm -lpthread

%.c.o : %.c
	@echo "compile $@"
	@$(CC) $(C_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

%.cpp.o : %.cpp
	@echo "compile $@"
	@$(CPP) $(CPP_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@
