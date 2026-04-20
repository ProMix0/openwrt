EXTRA_CFLAGS += -DCONFIG_RTL8852D
IC_NAME := rtl8852d

ifeq ($(CONFIG_MP_INCLUDED), y)
### 8852D Default Enable VHT MP HW TX MODE ###
#EXTRA_CFLAGS += -DCONFIG_MP_VHT_HW_TX_MODE
#CONFIG_MP_VHT_HW_TX_MODE = y
endif

ifeq ($(CONFIG_PHL_ARCH), y)
HAL = phl/hal_g6
else
HAL = hal_g6
endif

ifeq ($(CONFIG_USB_HCI), y)
	FILE_NAME = rtl8852du
endif
ifeq ($(CONFIG_PCI_HCI), y)
	FILE_NAME = rtl8852de
endif
ifeq ($(CONFIG_SDIO_HCI), y)
	FILE_NAME = rtl8852ds
endif


_HAL_IC_FILES +=	$(HAL)/$(IC_NAME)/$(IC_NAME)_halinit.o \
			$(HAL)/$(IC_NAME)/$(IC_NAME)_mac.o \
			$(HAL)/$(IC_NAME)/$(IC_NAME)_cmd.o \
			$(HAL)/$(IC_NAME)/$(IC_NAME)_phy.o \
			$(HAL)/$(IC_NAME)/$(IC_NAME)_ops.o \
			$(HAL)/$(IC_NAME)/hal_trx_8852d.o

_HAL_IC_FILES +=	$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_halinit.o \
			$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_io.o \
			$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_led.o \
			$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_ops.o

#ifeq ($(CONFIG_SDIO_HCI), y)
#_HAL_IC_FILES += $(HAL)/$(IC_NAME)/$(HCI_NAME)/hal_trx_8852ds.o
#endif

ifeq ($(CONFIG_USB_HCI), y)
_HAL_IC_FILES += $(HAL)/$(IC_NAME)/$(HCI_NAME)/hal_trx_8852du.o
endif

ifeq ($(CONFIG_PCI_HCI), y)
_HAL_IC_FILES += $(HAL)/$(IC_NAME)/$(HCI_NAME)/hal_trx_8852de.o
endif
