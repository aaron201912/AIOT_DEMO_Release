include clear-config.mk
CFLAGS:=-O0
SRCS:=fb_dispattr.c ../common/fb_common.c
DEP_INCS+=fb/common
include add-config.mk
