#
# USB peripheral controller drivers
#
ifeq ($(CONFIG_USB_GADGET_DEBUG),y)
	EXTRA_CFLAGS		+= -DDEBUG
endif

obj-$(CONFIG_USB_DUMMY_HCD)	+= dummy_hcd.o
obj-$(CONFIG_USB_NET2280)	+= net2280.o
obj-$(CONFIG_USB_AMD5536UDC)	+= amd5536udc.o
obj-$(CONFIG_USB_PXA25X)	+= pxa25x_udc.o
obj-$(CONFIG_USB_PXA27X)	+= pxa27x_udc.o
obj-$(CONFIG_USB_GOKU)		+= goku_udc.o
obj-$(CONFIG_USB_OMAP)		+= omap_udc.o
obj-$(CONFIG_USB_LH7A40X)	+= lh7a40x_udc.o
obj-$(CONFIG_USB_S3C2410)	+= s3c2410_udc.o
obj-$(CONFIG_USB_DWC_OTG)	:= dwc_otg.o
dwc_otg-y			        := dwc_otg_driver.o
dwc_otg-y			        += dwc_otg_attr.o
dwc_otg-y			        += dwc_otg_cil.o
dwc_otg-y			        += dwc_otg_cil_intr.o
dwc_otg-y			        += dwc_otg_pcd.o
dwc_otg-y			        += dwc_otg_pcd_intr.o
dwc_otg-y			        += dwc_otg_hcd.o
dwc_otg-y			        += dwc_otg_hcd_intr.o
dwc_otg-y			        += dwc_otg_hcd_queue.o
dwc_otg-y					+= ../dwc_common_port/dwc_cc.o
dwc_otg-y					+= ../dwc_common_port/dwc_crypto.o
dwc_otg-y					+= ../dwc_common_port/dwc_dh.o
dwc_otg-y					+= ../dwc_common_port/dwc_mem.o
dwc_otg-y					+= ../dwc_common_port/dwc_modpow.o
dwc_otg-y					+= ../dwc_common_port/dwc_notifier.o
dwc_otg-y					+= ../dwc_common_port/dwc_common_linux.o
dwc_otg-y			        += dwc_otg_hcd_ddma.o
dwc_otg-y			        += dwc_otg_pcd_linux.o
dwc_otg-y			        += dwc_otg_hcd_linux.o
obj-$(CONFIG_USB_S3C_OTGD)	+= s3c_udc_otg.o
g_android-objs			:= android.o f_adb.o f_mass_storage.o $(C_UTILS)
obj-$(CONFIG_USB_AT91)		+= at91_udc.o
obj-$(CONFIG_USB_ATMEL_USBA)	+= atmel_usba_udc.o
obj-$(CONFIG_USB_FSL_USB2)	+= fsl_usb2_udc.o
obj-$(CONFIG_USB_M66592)	+= m66592-udc.o
obj-$(CONFIG_USB_FSL_QE)	+= fsl_qe_udc.o

#
# USB gadget drivers
#
g_zero-objs			:= zero.o
g_ether-objs			:= ether.o
g_serial-objs			:= serial.o
g_midi-objs			:= gmidi.o
gadgetfs-objs			:= inode.o
g_file_storage-objs		:= file_storage.o
g_printer-objs			:= printer.o
g_cdc-objs			:= cdc2.o

obj-$(CONFIG_USB_ZERO)		+= g_zero.o
obj-$(CONFIG_USB_ETH)		+= g_ether.o
obj-$(CONFIG_USB_GADGETFS)	+= gadgetfs.o
obj-$(CONFIG_USB_FILE_STORAGE)	+= g_file_storage.o
obj-$(CONFIG_USB_G_SERIAL)	+= g_serial.o
obj-$(CONFIG_USB_G_PRINTER)	+= g_printer.o
obj-$(CONFIG_USB_MIDI_GADGET)	+= g_midi.o
obj-$(CONFIG_USB_CDC_COMPOSITE) += g_cdc.o
obj-$(CONFIG_USB_ANDROID)	+= g_android.o

