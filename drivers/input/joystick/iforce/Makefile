#
# Makefile for the I-Force driver
#
# By Johann Deneux <johann.deneux@gmail.com>
#

# Goal definition
iforce-objs	:= iforce-ff.o iforce-main.o iforce-packets.o

obj-$(CONFIG_JOYSTICK_IFORCE)	+= iforce.o

ifeq ($(CONFIG_JOYSTICK_IFORCE_232),y)
	iforce-objs += iforce-serio.o
endif

ifeq ($(CONFIG_JOYSTICK_IFORCE_USB),y)
	iforce-objs += iforce-usb.o
endif

EXTRA_CFLAGS = -Wno-implicit-function-declaration
