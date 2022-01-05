include clear-config.mk
CFLAGS:=-O0
SRCS:=fb_multi.c ../common/fb_common.c
DEP_INCS+=fb/common
include add-config.mk
