include clear-config.mk
CFLAGS:=-O0
LIBS:=m
SRCS:=fb_clrkey.c ../common/fb_common.c
DEP_INCS+=fb/common
include add-config.mk
