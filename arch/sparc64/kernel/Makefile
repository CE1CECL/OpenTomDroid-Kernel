#
# Makefile for the linux kernel.
#

EXTRA_AFLAGS := -ansi
EXTRA_CFLAGS := 

CFLAGS_REMOVE_ftrace.o = -pg

extra-y		:= head.o init_task.o vmlinux.lds

obj-y		:= process.o setup.o cpu.o idprom.o reboot.o \
		   traps.o auxio.o una_asm.o sysfs.o iommu.o \
		   irq.o ptrace.o time.o sys_sparc.o signal.o \
		   unaligned.o central.o starfire.o \
		   power.o sbus.o sparc64_ksyms.o ebus.o \
		   visemul.o prom.o of_device.o hvapi.o sstate.o mdesc.o

obj-$(CONFIG_DYNAMIC_FTRACE) += ftrace.o
obj-$(CONFIG_STACKTRACE) += stacktrace.o
obj-$(CONFIG_PCI)	 += pci.o pci_common.o psycho_common.o \
			    pci_psycho.o pci_sabre.o pci_schizo.o \
			    pci_sun4v.o pci_sun4v_asm.o pci_fire.o
obj-$(CONFIG_PCI_MSI)	+= pci_msi.o
obj-$(CONFIG_SMP)	 += smp.o trampoline.o hvtramp.o
obj-$(CONFIG_COMPAT) += sys32.o sys_sparc32.o signal32.o
obj-$(CONFIG_MODULES) += module.o
obj-$(CONFIG_US3_FREQ) += us3_cpufreq.o
obj-$(CONFIG_US2E_FREQ) += us2e_cpufreq.o
obj-$(CONFIG_US3_MC) += chmc.o
obj-$(CONFIG_KPROBES) += kprobes.o
obj-$(CONFIG_SUN_LDOMS) += ldc.o vio.o viohs.o ds.o
obj-$(CONFIG_AUDIT) += audit.o
obj-$(CONFIG_AUDIT)$(CONFIG_COMPAT) += compat_audit.o
obj-y += $(obj-yy)
obj-$(CONFIG_KGDB) += kgdb.o
