
include $(MAKE_INCLUDE_GEN)
#IFLAGS               += -I$(BASEDIR)/component/common/network/coap/include
IFLAGS               += -I$(BASEDIR)/component/common/network/libcoap/include

.PHONY: all clean

#*****************************************************************************#
#                               VARIABLES	                              #
#*****************************************************************************#
ifeq ($(CONFIG_MATTER_EN), y)
LWIP_VERSION := lwip_v2.1.2
LWIPDIR = $(MATTER_DIR)/common/lwip/$(LWIP_VERSION)
else
LWIP_VERSION := lwip_v2.0.2
LWIPDIR = $(BASEDIR)/component/common/network/lwip/$(LWIP_VERSION)
endif
NETWORK = $(BASEDIR)/component/common/network

#*****************************************************************************#
#                               Source FILE LIST                              #
#*****************************************************************************#
OBJS +=	\
		$(NETWORK)/sntp/sntp.o\
		$(NETWORK)/websocket/wsserver_tls.o\
		$(NETWORK)/websocket/wsclient_tls.o\
		$(LWIPDIR)/src/core/ipv4/dhcp.o \
		$(LWIPDIR)/src/core/ipv6/dhcp6.o \
		$(LWIPDIR)/src/core/dns.o \
		$(NETWORK)/dhcp/dhcps.o \
		$(LWIPDIR)/src/core/init.o \
		$(LWIPDIR)/src/core/ipv4/autoip.o \
		$(LWIPDIR)/src/core/ipv4/icmp.o \
		$(LWIPDIR)/src/core/ipv6/icmp6.o \
		$(LWIPDIR)/src/core/ipv6/inet6.o \
		$(LWIPDIR)/src/core/ipv6/mld6.o \
		$(LWIPDIR)/src/core/ipv6/nd6.o \
		$(LWIPDIR)/src/api/err.o \
		$(NETWORK)/httpc/httpc_tls.o\
		$(NETWORK)/httpd/httpd_tls.o\
		$(NETWORK)/mDNS/mDNSPlatform.o\
		$(LWIPDIR)/src/core/stats.o \
		$(LWIPDIR)/src/core/sys.o \
		$(NETWORK)/coap/sn_coap_ameba_port.o\
		$(NETWORK)/coap/sn_coap_builder.o\
		$(NETWORK)/coap/sn_coap_header_check.o\
		$(NETWORK)/coap/sn_coap_parser.o\
		$(NETWORK)/coap/sn_coap_protocol.o

ifeq ($(LWIP_VERSION), lwip_v2.2.0)
OBJS += $(LWIPDIR)/src/core/ipv4/acd.o
endif


#ifeq ($(CONFIG_HIGH_TP),y)
#OBJS = 
#endif
#*****************************************************************************#
#                               Include Dependency                            #
#*****************************************************************************#
-include $(OBJS:.o=.d)


#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#
all: CORE_TARGETS COPY_RAM_OBJS
#*****************************************************************************#
#              GENERATE OBJECT FILE 
#*****************************************************************************#
CORE_TARGETS: $(OBJS)

#*****************************************************************************#
#              CLEAN GENERATED FILES                                          #
#*****************************************************************************#
clean: CLEAN_OBJS 
	$(REMOVE) *.o
	$(REMOVE) *.i
	$(REMOVE) *.s
	$(REMOVE) *.d

-include $(DEPS)
