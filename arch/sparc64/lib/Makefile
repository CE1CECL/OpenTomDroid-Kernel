#
# Makefile for Sparc64 library files..
#

EXTRA_AFLAGS := -ansi
EXTRA_CFLAGS := 

lib-y := PeeCeeI.o copy_page.o clear_page.o strlen.o strncmp.o \
	 memscan.o strncpy_from_user.o strlen_user.o memcmp.o checksum.o \
	 bzero.o csum_copy.o csum_copy_from_user.o csum_copy_to_user.o \
	 VISsave.o atomic.o bitops.o \
	 U1memcpy.o U1copy_from_user.o U1copy_to_user.o \
	 U3memcpy.o U3copy_from_user.o U3copy_to_user.o U3patch.o \
	 NGmemcpy.o NGcopy_from_user.o NGcopy_to_user.o NGpatch.o \
	 NGpage.o NGbzero.o \
	 NG2memcpy.o NG2copy_from_user.o NG2copy_to_user.o NG2patch.o \
	 NG2page.o \
	 GENmemcpy.o GENcopy_from_user.o GENcopy_to_user.o GENpatch.o \
	 GENpage.o GENbzero.o \
	 copy_in_user.o user_fixup.o memmove.o \
	 mcount.o ipcsum.o rwsem.o xor.o

obj-y += iomap.o
