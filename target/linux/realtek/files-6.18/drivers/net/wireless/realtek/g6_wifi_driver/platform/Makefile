include $(KLIB_BUILD)/.config

# Auto select the right platform if supported
include $(src)/platform.mk

########## PLATFORM MAKEFILE ##########################
# Import platform assigned KSRC and CROSS_COMPILE
include $(wildcard $(src)/platform/*.mk)

.PHONY: romfs
romfs:

ifneq ($(ROMFSDIR),)

########## Install PE images ##########
ifeq ($(CONFIG_RTL_WIFI_OFFLOAD), y)
ifeq ($(CONFIG_ARCH_CORTINA_VENUS), y)
	$(shell cp -rf $(src)/platform/rtl_wfo/zephyr_77B_*.bin $(ROMFSDIR)/bin/PE_images/)
endif
ifeq ($(CONFIG_ARCH_REALTEK_TAURUS), y)
	$(shell cp -rf $(src)/platform/rtl_wfo/zephyr_77C_*.bin $(ROMFSDIR)/bin/PE_images/)
endif
ifeq ($(CONFIG_ARCH_REALTEK_9607F), y)
	$(shell cp -rf $(src)/platform/rtl_wfo/zephyr_07F_*.bin $(ROMFSDIR)/bin/PE_images/)
endif
endif # CONFIG_RTL_WIFI_OFFLOAD

########## Install FW images ##########
ifeq ($(CONFIG_RTW_FW_BUILTIN),)

ifeq ($(CONFIG_RTL8852AE), y)
	mkdir -p $(ROMFSDIR)/lib/firmware/rtl8852ae
	for bin in $(shell ls $(src)/phl/hal_g6/mac/fw_ax/rtl8852a/ | grep FW_AP_U); do \
		cp -f $(src)/phl/hal_g6/mac/fw_ax/rtl8852a/$$bin $(ROMFSDIR)/lib/firmware/rtl8852ae/RTL8852A$${bin,,}; \
	done
endif

ifeq ($(CONFIG_RTL8852CE), y)
	mkdir -p $(ROMFSDIR)/lib/firmware/rtl8852ce
	for bin in $(shell ls $(src)/phl/hal_g6/mac/fw_ax/rtl8852c/ | grep FW_AP_U); do \
		cp -f $(src)/phl/hal_g6/mac/fw_ax/rtl8852c/$$bin $(ROMFSDIR)/lib/firmware/rtl8852ce/RTL8852C$${bin,,}; \
	done
endif

ifeq ($(CONFIG_RTL8852DE), y)
	mkdir -p $(ROMFSDIR)/lib/firmware/rtl8852de
	for bin in $(shell ls $(src)/phl/hal_g6/mac/fw_ax/rtl8852d/ | grep FW_AP_U); do \
		cp -f $(src)/phl/hal_g6/mac/fw_ax/rtl8852d/$$bin $(ROMFSDIR)/lib/firmware/rtl8852de/RTL8852D$${bin,,}; \
	done
endif

ifeq ($(CONFIG_RTL8192XB), y)
	mkdir -p $(ROMFSDIR)/lib/firmware/rtl8192xbe
	for bin in $(shell ls $(src)/phl/hal_g6/mac/fw_ax/rtl8192xb/ | grep FW_AP_U); do \
		cp -f $(src)/phl/hal_g6/mac/fw_ax/rtl8192xb/$$bin $(ROMFSDIR)/lib/firmware/rtl8192xbe/RTL8192XB$${bin,,}; \
	done
endif

ifeq ($(CONFIG_RTL8832BR), y)
	mkdir -p $(ROMFSDIR)/lib/firmware/rtl8192xbe
	for bin in $(shell ls $(src)/phl/hal_g6/mac/fw_ax/rtl8192xb/ | grep FW_AP_U); do \
		cp -f $(src)/phl/hal_g6/mac/fw_ax/rtl8192xb/$$bin $(ROMFSDIR)/lib/firmware/rtl8192xbe/RTL8192XB$${bin,,}; \
	done
endif


endif # CONFIG_RTW_FW_BUILTIN

endif # ROMFSDIR
