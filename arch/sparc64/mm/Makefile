# Makefile for the linux Sparc64-specific parts of the memory manager.
#

EXTRA_AFLAGS := -ansi
EXTRA_CFLAGS := 

obj-y    := ultra.o tlb.o tsb.o fault.o init.o generic.o

obj-$(CONFIG_HUGETLB_PAGE) += hugetlbpage.o
