include $(MAKE_INCLUDE_GEN)

.PHONY: lib  clean

#*****************************************************************************#
#                                  VARIABLES                                  #
#*****************************************************************************#

#*****************************************************************************#
#                               CFLAGS                                        #
#*****************************************************************************#

#*****************************************************************************#
#                               Source FILE LIST                              #
#*****************************************************************************#

#*****************************************************************************#
#                               Object FILE LIST                              #
#*****************************************************************************#
OBJS = 

#*****************************************************************************#
#                               Dependency	                                  #
#*****************************************************************************#

#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#

# Define the Rules to build the core targets
COPY_RAM_OBJS: CORE_TARGETS

lib: COPY_RAM_OBJS
	$(AR) rvs lib_base.a *.o
	$(FROMELF) -g -D lib_base.a
	$(MOVE) -f lib_base.a $(ROOTDIR)/lib/application/

#*****************************************************************************#
#              GENERATE OBJECT FILE
#*****************************************************************************#
CORE_TARGETS:	$(OBJS)

#*****************************************************************************#
#                        RULES TO CLEAN TARGETS                               #
#*****************************************************************************#
clean: CLEAN_OBJS
	$(REMOVE) *.o
