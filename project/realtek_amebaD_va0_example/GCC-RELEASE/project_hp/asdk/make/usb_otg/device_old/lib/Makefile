
include $(MAKE_INCLUDE_GEN)

.PHONY: all clean

#*****************************************************************************#
#                               VARIABLES	                              #
#*****************************************************************************#
DIR = $(BASEDIR)/component/soc/realtek/amebad/fwlib/usb_otg/device/src
IFLAGS += -I$(DIR)/../inc
TARGET_LIB = lib_usbd_old

vpath %.c $(DIR)

#*****************************************************************************#
#                               Source FILE LIST                              #
#*****************************************************************************#
CSRC += $(DIR)/dwc_otg_cil.c \
	$(DIR)/dwc_otg_cil_intr.c \
	$(DIR)/dwc_otg_common.c \
	$(DIR)/dwc_otg_driver.c \
	$(DIR)/dwc_otg_driver_rom.c \
	$(DIR)/dwc_otg_pcd.c \
	$(DIR)/dwc_otg_pcd_intr.c \
	$(DIR)/dwc_otg_pcd_os.c \
	$(DIR)/usb_ep_autoconfig.c \
	$(DIR)/usb_composite.c \
	$(DIR)/usb.c
	
#*****************************************************************************#
#                               Object FILE LIST                              #
#*****************************************************************************#
OBJS = $(notdir $(CSRC:.c=.o))
	
#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#

# Define the Rules to build the core targets
all: CORE_TARGETS
	$(AR) rvs $(TARGET_LIB).a $(OBJS)
	$(FROMELF) --strip-debug $(TARGET_LIB).a
	$(MOVE) -f $(TARGET_LIB).a $(ROOTDIR)/lib/application
	
#*****************************************************************************#
#              GENERATE OBJECT FILE 
#*****************************************************************************#
CORE_TARGETS:	$(OBJS)


#*****************************************************************************#
#                        RULES TO CLEAN TARGETS                               #
#*****************************************************************************#
clean: CLEAN_OBJS
	$(REMOVE) *.d
	$(REMOVE) *.i
	$(REMOVE) *.o
	$(REMOVE) *.s
	$(REMOVE) *.su

-include $(DEPS)
