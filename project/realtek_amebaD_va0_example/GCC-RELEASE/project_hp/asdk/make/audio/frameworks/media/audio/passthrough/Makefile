
include $(MAKE_INCLUDE_GEN)

.PHONY: all lib clean


#*****************************************************************************#
#                               Object FILE LIST                              #
#*****************************************************************************#
OBJS =

#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#

# Define the Rules to build the core targets
all: CORE_TARGETS
	make -C tests  all

lib: CORE_TARGETS
	make -C src lib

#*****************************************************************************#
#              GENERATE OBJECT FILE                                           #
#*****************************************************************************#
CORE_TARGETS: $(OBJS)




#*****************************************************************************#
#                        RULES TO CLEAN TARGETS                               #
#*****************************************************************************#
clean:
	make -C src clean
	make -C tests clean
-include $(DEPS)
