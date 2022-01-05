include clear-config.mk
COMPILE_TARGET:=lib

SRCS:=iniparser.c dictionary.c strlib.c
DEP_INCS+=$(PROJ_ROOT)/../sdk/verify/feature/feature_hdmi/inc

include add-config.mk
