include clear-config.mk
CFLAGS:=-O0
SRCS:=fb_dbbuf.c ../common/fb_common.c
DEP_INCS+=fb/common
include add-config.mk
