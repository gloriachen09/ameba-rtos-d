
include $(MAKE_INCLUDE_GEN)

.PHONY: all clean

#*****************************************************************************#
#                               VARIABLES	                              #
#*****************************************************************************#
DRV_DIR = $(BASEDIR)/component/common/drivers/usb
COM_DIR = $(DRV_DIR)/common
DEV_DIR = $(DRV_DIR)/device
IFLAGS += -I$(COM_DIR)
IFLAGS += -I$(DEV_DIR)/core
ifeq ($(CONFIG_USBD_CDC_ACM),y)
DIR = $(DEV_DIR)/cdc_acm
endif
ifeq ($(CONFIG_USBD_COMPOSITE),y)
DIR = $(DEV_DIR)/composite
endif
ifeq ($(CONFIG_USBD_HID),y)
DIR = $(DEV_DIR)/hid
endif
ifeq ($(CONFIG_USBD_MSC),y)
DIR = $(DEV_DIR)/msc
endif
ifeq ($(CONFIG_USBD_VENDOR),y)
DIR = $(DEV_DIR)/vendor
endif

IFLAGS += -I$(DIR)

GLOBAL_CFLAGS += -DCONFIG_USB_NEW_STACK_EN

vpath %.c $(DIR)

#*****************************************************************************#
#                               Source FILE LIST                              #
#*****************************************************************************#
ifeq ($(CONFIG_USBD_CDC_ACM),y)
CSRC = $(DIR)/usbd_cdc_acm.c
endif

ifeq ($(CONFIG_USBD_COMPOSITE),y)
CSRC += $(DIR)/usbd_composite.c \
	$(DIR)/usbd_composite_cdc_acm.c \
	$(DIR)/usbd_composite_hid.c
endif

ifeq ($(CONFIG_USBD_HID),y)
CSRC = $(DIR)/usbd_hid.c
endif

ifeq ($(CONFIG_USBD_MSC),y)
CSRC = $(DIR)/usbd_msc.c \
	$(DIR)/usbd_scsi.c
endif

ifeq ($(CONFIG_USBD_VENDOR),y)
CSRC = $(DIR)/usbd_vendor.c
endif

#*****************************************************************************#
#                               Object FILE LIST                              #
#*****************************************************************************#
OBJS = $(notdir $(CSRC:.c=.o))

#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#

# Define the Rules to build the core targets
COPY_RAM_OBJS: CORE_TARGETS
all: COPY_RAM_OBJS

#*****************************************************************************#
#              GENERATE OBJECT FILE
#*****************************************************************************#
CORE_TARGETS:	$(OBJS)


#*****************************************************************************#
#                        RULES TO CLEAN TARGETS                               #
#*****************************************************************************#
clean: CLEAN_OBJS
	$(REMOVE) *.o
	$(REMOVE) *.i
	$(REMOVE) *.s*
	$(REMOVE) *.d

-include $(DEPS)
