
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
all: CORE_TARGETS
#	make -C dct all
	make -C fatfs all
	make -C littlefs all
	make -C kv all
ifeq ($(CONFIG_BT_EN), y)
	make -C ftl all
endif

#*****************************************************************************#
#              GENERATE OBJECT FILE 
#*****************************************************************************#
CORE_TARGETS: $(OBJS)

	


#*****************************************************************************#
#                        RULES TO CLEAN TARGETS                               #
#*****************************************************************************#
clean: 
	make -C dct clean
	make -C fatfs clean
	make -C ftl clean
	make -C littlefs clean
	make -C kv clean
