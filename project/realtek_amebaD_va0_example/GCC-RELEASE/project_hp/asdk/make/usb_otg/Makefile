
include $(MAKE_INCLUDE_GEN)

.PHONY: all clean

#*****************************************************************************#
#                               Object FILE LIST                              #
#*****************************************************************************#
OBJS = 

#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#

# Define the Rules to build the core targets
all: CORE_TARGETS COPY_RAM_OBJS

ifeq ($(CONFIG_USB_DEVICE_OLD_EN),y)
	make -C device_old all
endif
ifeq ($(CONFIG_USB_HOST_OLD_EN),y)
	make -C host_old all
endif

ifeq ($(CONFIG_USB_DEVICE_EN),y)
	make -C device all
endif
ifeq ($(CONFIG_USB_HOST_EN),y)
	make -C host all
endif
	
#*****************************************************************************#
#              GENERATE OBJECT FILE
#*****************************************************************************#
CORE_TARGETS: $(OBJS)


#*****************************************************************************#
#                        RULES TO CLEAN TARGETS                               #
#*****************************************************************************#
clean:
	make -C device_old clean
	make -C host_old clean
	make -C device clean
	make -C host clean

-include $(DEPS)

