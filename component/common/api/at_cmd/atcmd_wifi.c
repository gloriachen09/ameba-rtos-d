#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "log_service.h"
#include "atcmd_wifi.h"
#include "atcmd_lwip.h"
#include <lwip_netconf.h>
#include "tcpip.h"
#include <dhcp/dhcps.h>
#if CONFIG_WLAN
#include <wlan/wlan_test_inc.h>
#include <wifi/wifi_conf.h>
#include <wifi/wifi_util.h>
#endif

#if ATCMD_VER == ATVER_2
#include "flash_api.h"
#include "device_lock.h"
#include <wlan_fast_connect/example_wlan_fast_connect.h>
#endif

#if ATCMD_VER == ATVER_2 || WIFI_LOGO_CERTIFICATION_CONFIG
#include <lwip/sockets.h>
#endif

#ifdef WIFI_PERFORMANCE_MONITOR
#include "wifi_performance_monitor.h"
#endif

#include "platform_opts.h"
#if defined(CONFIG_PLATFORM_8710C)||defined(CONFIG_PLATFORM_8721D)
#include "platform_opts_bt.h"
#endif

/******************************************************************************/
#define	_AT_WLAN_SET_SSID_          "ATW0"
#define	_AT_WLAN_SET_PASSPHRASE_    "ATW1"
#define	_AT_WLAN_SET_KEY_ID_        "ATW2"
#define	_AT_WLAN_AP_SET_SSID_       "ATW3"
#define	_AT_WLAN_AP_SET_SEC_KEY_    "ATW4"
#define	_AT_WLAN_AP_SET_CHANNEL_    "ATW5"
#define _AT_WLAN_SET_BSSID_         "ATW6"
#define _AT_WLAN_SET_WPA_MODE_      "ATW8"
#define	_AT_WLAN_AP_ACTIVATE_       "ATWA"
#define _AT_WLAN_AP_STA_ACTIVATE_   "ATWB"
#define	_AT_WLAN_JOIN_NET_          "ATWC"
#define	_AT_WLAN_DISC_NET_          "ATWD"
#define	_AT_WLAN_WEB_SERVER_        "ATWE"
#define _AT_WLAN_P2P_FIND_          "ATWF"
#define _AT_WLAN_P2P_START_         "ATWG"
#define _AT_WLAN_P2P_STOP_          "ATWH"
#define _AT_WLAN_PING_TEST_         "ATWI"
#define _AT_WLAN_P2P_CONNECT_       "ATWJ"
#define _AT_WLAN_P2P_DISCONNECT_    "ATWK"
#define _AT_WLAN_SSL_CLIENT_        "ATWL"
#define _AT_WLAN_PROMISC_           "ATWM"
#define _AT_WLAN_P2P_INFO_          "ATWN"
#define _AT_WLAN_OTA_UPDATE_        "ATWO"
#define	_AT_WLAN_POWER_             "ATWP"
#define	_AT_WLAN_SIMPLE_CONFIG_     "ATWQ"
#define	_AT_WLAN_GET_RSSI_          "ATWR"
#define	_AT_WLAN_SCAN_              "ATWS"
#define _AT_WLAN_SCAN_WITH_SSID_    "ATWs"
#define _AT_WLAN_TCP_TEST_          "ATWT"
#define _AT_WLAN_UDP_TEST_          "ATWU"
#define _AT_WLAN_WPS_               "ATWW"
#define _AT_WLAN_AP_WPS_            "ATWw"
#define _AT_WLAN_AIRKISS_           "ATWX"
#define _AT_WLAN_IWPRIV_            "ATWZ"
#define	_AT_WLAN_INFO_              "ATW?"

#define	_AT_WLAN_EXTEND_POWER_MODE_        "ATXP"

#ifndef CONFIG_SSL_CLIENT
#define CONFIG_SSL_CLIENT       0
#endif
#ifndef CONFIG_OTA_UPDATE
#define CONFIG_OTA_UPDATE       0
#endif
#ifndef CONFIG_BSD_TCP
#define CONFIG_BSD_TCP	        1
#endif
#ifndef CONFIG_ENABLE_P2P
#define CONFIG_ENABLE_P2P		0
#endif
#define SCAN_WITH_SSID		1

#ifndef CONFIG_WOWLAN_SERVICE
#define CONFIG_WOWLAN_SERVICE 0
#endif

#if CONFIG_BRIDGE
extern void show_bridgeif_fdbd(struct netif *netif);
#endif

#if CONFIG_LWIP_LAYER
extern void cmd_iperf(int argc, char **argv);
extern void cmd_ping(int argc, char **argv);
extern void cmd_ssl_client(int argc, char **argv);
#endif 

#if CONFIG_WLAN
extern void cmd_promisc(int argc, char **argv);
extern void cmd_update(int argc, char **argv);
extern void cmd_simple_config(int argc, char **argv);
#if defined(CONFIG_INCLUDE_DPP_CONFIG) && CONFIG_INCLUDE_DPP_CONFIG
extern void cmd_dpp(int argc, char **argv);
#endif
#if CONFIG_ENABLE_WPS
extern void cmd_wps(int argc, char **argv);
#endif

#if defined(CONFIG_ENABLE_WPS_AP) && CONFIG_ENABLE_WPS_AP
extern void cmd_ap_wps(int argc, char **argv);
extern int wpas_wps_dev_config(u8 *dev_addr, u8 bregistrar);
#endif
#if CONFIG_ENABLE_P2P
extern void cmd_wifi_p2p_start(int argc, char **argv);
extern void cmd_wifi_p2p_stop(int argc, char **argv);
extern void cmd_p2p_listen(int argc, char **argv);
extern void cmd_p2p_find(int argc, char **argv);
extern void cmd_p2p_peers(int argc, char **argv);
extern void cmd_p2p_info(int argc, char **argv);
extern void cmd_p2p_disconnect(int argc, char **argv);
extern void cmd_p2p_connect(int argc, char **argv);
extern int cmd_wifi_p2p_auto_go_start(int argc, char **argv);
#endif //CONFIG_ENABLE_P2P
#if CONFIG_AIRKISS
extern int airkiss_start(rtw_network_info_t *);
extern int airkiss_stop(void);
#endif
#if CONFIG_LWIP_LAYER
extern struct netif xnetif[NET_IF_NUM]; 
#endif
#if CONFIG_WOWLAN_SERVICE
extern void cmd_wowlan_service(int argc, char **argv);
#endif
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
extern void inic_c2h_wifi_info(const char *atcmd, char status);
extern void inic_c2h_msg(const char *atcmd, u8 status, char *msg, u16 msg_len);
#endif
extern int wifi_set_beacon_mode(int mode);

/* fastconnect use wifi AT command. Not init_wifi_struct when log service disabled
 * static initialize all values for using fastconnect when log service disabled
 */
static rtw_network_info_t wifi = {0};
static rtw_scan_result_t scan_ap = {0};
static rtw_ap_info_t ap = {0};
static unsigned char password[65] = {0};
#ifdef CONFIG_AP_SECURITY
int security = -1;
#endif

#if ATCMD_VER == ATVER_2 || WIFI_LOGO_CERTIFICATION_CONFIG
unsigned char sta_ip[4] = {192,168,1,80}, sta_netmask[4] = {255,255,255,0}, sta_gw[4] = {192,168,1,1};
u8 use_static_ip = 0;
#endif

#if ATCMD_VER == ATVER_2
unsigned char dhcp_mode_sta = 1, dhcp_mode_ap = 1;
unsigned char  ap_ip[4] = {192,168,43,1},  ap_netmask[4] = {255,255,255,0},  ap_gw[4] = {192,168,43,1};
static void atcmd_wifi_disconn_hdl( char* buf, int buf_len, int flags, void* userdata);
static void atcmd_wifi_connected_hdl( char* buf, int buf_len, int flags, void* userdata);
#endif

rtw_mode_t wifi_mode = RTW_MODE_STA;

#if ATCMD_VER == ATVER_2
rtw_mode_t wifi_mode_new = RTW_MODE_STA;
#endif

static void init_wifi_struct(void)
{
	memset(wifi.ssid.val, 0, sizeof(wifi.ssid.val));
	memset(wifi.bssid.octet, 0, ETH_ALEN);	
	memset(password, 0, sizeof(password));
	wifi.ssid.len = 0;
	wifi.password = NULL;
	wifi.password_len = 0;
	wifi.key_id = -1;
	memset(ap.ssid.val, 0, sizeof(ap.ssid.val));
	ap.ssid.len = 0;
	ap.password = NULL;
	ap.password_len = 0;
	ap.channel = 1;
#ifdef CONFIG_AP_SECURITY
	security = -1;
#endif
}

static void print_scan_result( rtw_scan_result_t* record )
{
#if (defined(CONFIG_EXAMPLE_UART_ATCMD) && CONFIG_EXAMPLE_UART_ATCMD) || (defined(CONFIG_EXAMPLE_SPI_ATCMD) && CONFIG_EXAMPLE_SPI_ATCMD)
	at_printf("%s,%d,%s,%d,"MAC_FMT"\r\n", record->SSID.val, record->channel,
			( record->security == RTW_SECURITY_OPEN ) ? "Open" :
			( record->security == RTW_SECURITY_WEP_PSK ) ? "WEP" :
			( record->security == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
			( record->security == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
			( record->security == RTW_SECURITY_WPA_MIXED_PSK ) ? "WPA Mixed" :
			( record->security == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
			( record->security == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
			( record->security == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
			( record->security == RTW_SECURITY_WPA_WPA2_TKIP_PSK) ? "WPA/WPA2 TKIP" :
			( record->security == RTW_SECURITY_WPA_WPA2_AES_PSK) ? "WPA/WPA2 AES" :
			( record->security == RTW_SECURITY_WPA_WPA2_MIXED_PSK) ? "WPA/WPA2 Mixed" :
			( record->security == RTW_SECURITY_WPA_TKIP_ENTERPRISE ) ? "WPA TKIP Enterprise" :
			( record->security == RTW_SECURITY_WPA_AES_ENTERPRISE ) ? "WPA AES Enterprise" :
			( record->security == RTW_SECURITY_WPA_MIXED_ENTERPRISE ) ? "WPA Mixed Enterprise" :
			( record->security == RTW_SECURITY_WPA2_TKIP_ENTERPRISE ) ? "WPA2 TKIP Enterprise" :
			( record->security == RTW_SECURITY_WPA2_AES_ENTERPRISE ) ? "WPA2 AES Enterprise" :
			( record->security == RTW_SECURITY_WPA2_MIXED_ENTERPRISE ) ? "WPA2 Mixed Enterprise" :
			( record->security == RTW_SECURITY_WPA_WPA2_TKIP_ENTERPRISE ) ? "WPA/WPA2 TKIP Enterprise" :
			( record->security == RTW_SECURITY_WPA_WPA2_AES_ENTERPRISE ) ? "WPA/WPA2 AES Enterprise" :
			( record->security == RTW_SECURITY_WPA_WPA2_MIXED_ENTERPRISE ) ? "WPA/WPA2 Mixed Enterprise" :
#ifdef CONFIG_SAE_SUPPORT
			( record->security == RTW_SECURITY_WPA3_AES_PSK) ? "WPA3-SAE AES" :
			( record->security == RTW_SECURITY_WPA2_WPA3_MIXED) ? "WPA2/WPA3-SAE AES" :
			( record->security == RTW_SECURITY_WPA3_ENTERPRISE) ? "WPA3 Enterprise" :
#endif
			"Unknown",
			record->signal_strength, MAC_ARG(record->BSSID.octet)   );
#else
	RTW_API_INFO("%s\t ", ( record->bss_type == RTW_BSS_TYPE_ADHOC ) ? "Adhoc" : "Infra");
	RTW_API_INFO(MAC_FMT, MAC_ARG(record->BSSID.octet));
	RTW_API_INFO(" %d\t ", record->signal_strength);
	RTW_API_INFO(" %d\t  ", record->channel);
	RTW_API_INFO(" %d\t  ", record->wps_type);
	RTW_API_INFO("%s\t\t ", ( record->security == RTW_SECURITY_OPEN ) ? "Open" :
								 ( record->security == RTW_SECURITY_WEP_PSK ) ? "WEP" :
								 ( record->security == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
								 ( record->security == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
								 ( record->security == RTW_SECURITY_WPA_MIXED_PSK ) ? "WPA Mixed" :
								 ( record->security == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
								 ( record->security == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
								 ( record->security == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
								 ( record->security == RTW_SECURITY_WPA_WPA2_TKIP_PSK) ? "WPA/WPA2 TKIP" :
								 ( record->security == RTW_SECURITY_WPA_WPA2_AES_PSK) ? "WPA/WPA2 AES" :
								 ( record->security == RTW_SECURITY_WPA_WPA2_MIXED_PSK) ? "WPA/WPA2 Mixed" :
								 ( record->security == RTW_SECURITY_WPA_TKIP_ENTERPRISE ) ? "WPA TKIP Enterprise" :
								 ( record->security == RTW_SECURITY_WPA_AES_ENTERPRISE ) ? "WPA AES Enterprise" :
								 ( record->security == RTW_SECURITY_WPA_MIXED_ENTERPRISE ) ? "WPA Mixed Enterprise" :
								 ( record->security == RTW_SECURITY_WPA2_TKIP_ENTERPRISE ) ? "WPA2 TKIP Enterprise" :
								 ( record->security == RTW_SECURITY_WPA2_AES_ENTERPRISE ) ? "WPA2 AES Enterprise" :
								 ( record->security == RTW_SECURITY_WPA2_MIXED_ENTERPRISE ) ? "WPA2 Mixed Enterprise" :
								 ( record->security == RTW_SECURITY_WPA_WPA2_TKIP_ENTERPRISE ) ? "WPA/WPA2 TKIP Enterprise" :
								 ( record->security == RTW_SECURITY_WPA_WPA2_AES_ENTERPRISE ) ? "WPA/WPA2 AES Enterprise" :
								 ( record->security == RTW_SECURITY_WPA_WPA2_MIXED_ENTERPRISE ) ? "WPA/WPA2 Mixed Enterprise" :
#ifdef CONFIG_SAE_SUPPORT
								 ( record->security == RTW_SECURITY_WPA3_AES_PSK) ? "WPA3-SAE AES" :
								 ( record->security == RTW_SECURITY_WPA2_WPA3_MIXED) ? "WPA2/WPA3-SAE AES" :
								 ( record->security == RTW_SECURITY_WPA3_ENTERPRISE) ? "WPA3 Enterprise" :
#endif
								 "Unknown");

	RTW_API_INFO(" %s ", record->SSID.val);
	RTW_API_INFO("\r\n");
#endif
}

static rtw_result_t app_scan_result_handler( rtw_scan_handler_result_t* malloced_scan_result )
{
	static int ApNum = 0;

	if (malloced_scan_result->scan_complete != RTW_TRUE) {
		rtw_scan_result_t* record = &malloced_scan_result->ap_details;
		record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

#if (defined(CONFIG_EXAMPLE_UART_ATCMD) && CONFIG_EXAMPLE_UART_ATCMD) || (defined(CONFIG_EXAMPLE_SPI_ATCMD) && CONFIG_EXAMPLE_SPI_ATCMD)
		if (malloced_scan_result->user_data != NULL) {
			rtw_scan_result_t* user_data = malloced_scan_result->user_data;
			rtw_ssid_t* target_ssid = &user_data->SSID;
			if((record->SSID.len == target_ssid->len) && (!memcmp(record->SSID.val, target_ssid->val, target_ssid->len))) {
				if ((record->security == 0 && user_data->security == 0) || (record->security != 0 && user_data->security != 0)) {
					if (user_data->signal_strength == 0) {
						rtw_memcpy(user_data, record, sizeof(rtw_scan_result_t) - sizeof(rtw_802_11_band_t));
					} else if (user_data->signal_strength < record->signal_strength) {
						rtw_memcpy(user_data, record, sizeof(rtw_scan_result_t) - sizeof(rtw_802_11_band_t));
					}
				}
			}
		} else {
			at_printf("\r\nAP : %d,", ++ApNum);
			print_scan_result(record);
		}
		
#else
		RTW_API_INFO("%d\t ", ++ApNum);
		print_scan_result(record);
#endif
		
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
		if(malloced_scan_result->user_data)
			memcpy((void *)((char *)malloced_scan_result->user_data+(ApNum-1)*sizeof(rtw_scan_result_t)), (char *)record, sizeof(rtw_scan_result_t));
#endif
	} else{
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
		inic_c2h_msg("ATWS", RTW_SUCCESS, (char *)malloced_scan_result->user_data, ApNum*sizeof(rtw_scan_result_t));
		if(malloced_scan_result->user_data)
			free(malloced_scan_result->user_data);
		inic_c2h_msg("ATWS", RTW_SUCCESS, NULL, 0);
#endif
#if (defined(CONFIG_EXAMPLE_UART_ATCMD) && CONFIG_EXAMPLE_UART_ATCMD) || (defined(CONFIG_EXAMPLE_SPI_ATCMD) && CONFIG_EXAMPLE_SPI_ATCMD)
	if (malloced_scan_result->user_data == NULL) {
		at_printf("%sOK\r\n", "+WLSCAN:");
		ATCMD_NEWLINE_HASHTAG();
	} else {
		rtw_scan_result_t* user_data = malloced_scan_result->user_data;
		user_data->band = 1;
	}
#endif
		ApNum = 0;
	}
	return RTW_SUCCESS;
}

void fATWD(void *arg){
	/* To avoid gcc warnings */
	( void ) arg;

	int timeout = 20;
	char essid[33];
	volatile int ret = RTW_SUCCESS;
#if ATCMD_VER == ATVER_2
	int error_no = 0;
#endif

	printf("[ATWD]: _AT_WLAN_DISC_NET_\n\r");
	printf("\n\rDeassociating AP ...");

	if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) < 0) {
		printf("\n\rnot connected yet");
		goto exit_success;
	}
#if ATCMD_VER == ATVER_2
	wifi_unreg_event_handler(WIFI_EVENT_DISCONNECT, atcmd_wifi_disconn_hdl);
#endif
	if((ret = wifi_disconnect()) < 0) {
		printf("\n\rERROR: Operation failed!");
#if ATCMD_VER == ATVER_2
		error_no = 3;
#endif
		goto exit;
	}

	while(1) {
		if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) < 0) {
			printf("\n\rWIFI disconnect succeed");
			break;
		}

		if(timeout == 0) {
			printf("\n\rERROR: Deassoc timeout!");
			ret = RTW_TIMEOUT;
#if ATCMD_VER == ATVER_2
			error_no = 4;
#endif
			break;
		}

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout --;
	}
	printf("\n\r");

#if CONFIG_LWIP_LAYER
	LwIP_ReleaseIP(WLAN0_IDX);
#endif
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATWD", ret, NULL, 0);
#endif
        init_wifi_struct( );
#if ATCMD_VER == ATVER_2
	if(error_no==0) {
		at_printf("%sOK\r\n", "+WLDISCONN:");
	} else {
		at_printf("%sERROR:%d\r\n","+WLDISCONN:",error_no);
	}
	ATCMD_NEWLINE_HASHTAG();
#endif
	return;
exit_success:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATWD", RTW_SUCCESS, NULL, 0);
#endif
	init_wifi_struct( );
#if ATCMD_VER == ATVER_2
	at_printf("%sOK\r\n", "+WLDISCONN:");
	ATCMD_NEWLINE_HASHTAG();
#endif	
	return;
}

#if (CONFIG_INCLUDE_SIMPLE_CONFIG)
void fATWQ(void *arg){
	int argc=0;
	char *argv[2] = {0};
	printf("[ATWQ]: _AT_WLAN_SIMPLE_CONFIG_\n\r");
	argv[argc++] = "wifi_simple_config";
	if(arg){
		argv[argc++] = arg;
	}
#if ATCMD_VER == ATVER_2
	wifi_unreg_event_handler(WIFI_EVENT_DISCONNECT, atcmd_wifi_disconn_hdl);
#endif		
	cmd_simple_config(argc, argv);
}
#endif

#if defined(CONFIG_INCLUDE_DPP_CONFIG) && CONFIG_INCLUDE_DPP_CONFIG
void fATWq(void *arg)
{
    int argc;
    char *argv[MAX_ARGC] = {0};
	
	char buf[256] = {0};
    printf("[ATWq]:\n\r");
    if(arg){
          strncpy(buf, arg, sizeof(buf));
          argc = parse_param(buf, argv);
   }
	cmd_dpp(argc, argv);
}
#endif

void fATWS(void *arg){
        char buf[32] = {0};
        u8 *channel_list = NULL;
        u8 *pscan_config = NULL;
        int num_channel = 0;
        int i, argc = 0;
        char *argv[MAX_ARGC] = {0};
	volatile int ret = RTW_SUCCESS;
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	u8 *inic_scan_buf = NULL;
#endif
#if ATCMD_VER == ATVER_2
	int error_no = 0;
#endif
        printf("[ATWS]: _AT_WLAN_SCAN_\n\r");
        if(arg){
          strncpy(buf, arg, sizeof(buf));
          argc = parse_param(buf, argv);
          if(argc < 2){
		ret = RTW_BADARG;
#if ATCMD_VER == ATVER_2
		error_no = 1;
#endif
            goto exit;
	   }
          num_channel = atoi(argv[1]);
          channel_list = (u8*)malloc(num_channel);
          if(!channel_list){
                  printf("[ATWS]ERROR: Can't malloc memory for channel list\n\r");
                  ret = RTW_BUFFER_UNAVAILABLE_TEMPORARY;
#if ATCMD_VER == ATVER_2
                  error_no = 2;
#endif
                  goto exit;
          }
          pscan_config = (u8*)malloc(num_channel);
	  	  if(!pscan_config){
				printf("[ATWS]ERROR: Can't malloc memory for pscan_config\n\r");
				ret = RTW_BUFFER_UNAVAILABLE_TEMPORARY;
#if ATCMD_VER == ATVER_2
				error_no = 3;
#endif
				goto exit;
		  }
          //parse command channel list
          for(i = 2; i <= argc -1 ; i++){
                  *(channel_list + i - 2) = (u8)atoi(argv[i]);
                  *(pscan_config + i - 2) = PSCAN_ENABLE;
          }

          if((ret = wifi_set_pscan_chan(channel_list, pscan_config, num_channel)) < 0){
              printf("[ATWS]ERROR: wifi set partial scan channel fail\n\r");
#if ATCMD_VER == ATVER_2
              error_no = 4;
#endif
              goto exit;
          }
        }
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_scan_buf = malloc(65*sizeof(rtw_scan_result_t));
	if(inic_scan_buf == NULL){
		ret = RTW_BUFFER_UNAVAILABLE_TEMPORARY;
		goto exit;
	}
        memset(inic_scan_buf, 0, 65*sizeof(rtw_scan_result_t));
	if((ret = wifi_scan_networks(app_scan_result_handler, inic_scan_buf)) != RTW_SUCCESS){
		printf("[ATWS]ERROR: wifi scan failed\n\r");
		goto exit;
	}
#else
	if((ret = wifi_scan_networks(app_scan_result_handler, NULL )) != RTW_SUCCESS){
		printf("[ATWS]ERROR: wifi scan failed\n\r");
#if ATCMD_VER == ATVER_2
		error_no = 5;
#endif
		goto exit;
	}
#endif
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	if(ret != RTW_SUCCESS){
		if(inic_scan_buf)
			free(inic_scan_buf);
		inic_c2h_msg("ATWS", ret, NULL, 0);
	}
#endif
#if ATCMD_VER == ATVER_2
	if(error_no)
		at_printf("%sERROR:%d\r\n", "+WLSCAN:",error_no);
		ATCMD_NEWLINE_HASHTAG();
#endif
	if(arg && channel_list)
		free(channel_list);
	if(arg && pscan_config)
		free(pscan_config);
}

#ifdef WIFI_PERFORMANCE_MONITOR
/**
  * @brief print the time of WIFI TRx path.
  * @param  arg: the command "ATWm"
  * @retval None
  */
void fATWm(void *arg) {
	wifi_performance_print();
}
#endif

void fATWx(void *arg){
	/* To avoid gcc warnings */
	( void ) arg;

	int i = 0;
#if CONFIG_LWIP_LAYER
	u8 *mac = LwIP_GetMAC(&xnetif[0]);
	u8 *ip = LwIP_GetIP(&xnetif[0]);
	u8 *gw = LwIP_GetGW(&xnetif[0]);
	u8 *msk = LwIP_GetMASK(&xnetif[0]);
	u8 mac_1[6];
#if LWIP_IPV6
	u8 *ipv6_0 = LwIP_GetIPv6_linklocal(&xnetif[0]);
#endif
#endif
	u8 *ifname[2] = {(u8*)WLAN0_NAME,(u8*)WLAN1_NAME};
	rtw_wifi_setting_t setting;

	printf("[ATW?]: _AT_WLAN_INFO_\n\r");
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	int ret = RTW_SUCCESS;
	int info_sz = 0;
	u8 *info = malloc(NET_IF_NUM*sizeof(rtw_wifi_setting_t)+3*sizeof(rtw_mac_t));
	if(info == NULL)
		ret = RTW_BUFFER_UNAVAILABLE_TEMPORARY;
#endif
	for(i=0;i<NET_IF_NUM;i++){
		if(rltk_wlan_running(i)){
#if CONFIG_LWIP_LAYER
			mac = LwIP_GetMAC(&xnetif[i]);
			ip = LwIP_GetIP(&xnetif[i]);
			gw = LwIP_GetGW(&xnetif[i]);
			msk = LwIP_GetMASK(&xnetif[i]);
#if LWIP_IPV6
			ipv6_0 = LwIP_GetIPv6_linklocal(&xnetif[i]);
#endif /* LWIP_IPV6 */
#endif
			printf("\n\r\nWIFI %s Status: Running",  ifname[i]);
			printf("\n\r==============================");

			rltk_wlan_statistic(i);

			wifi_get_setting((const char*)ifname[i],&setting);
			wifi_show_setting((const char*)ifname[i],&setting);
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
			if(info){
				memcpy(info+info_sz, (void *)&setting, sizeof(rtw_wifi_setting_t));
				info_sz += sizeof(rtw_wifi_setting_t);
			}
#endif

#if CONFIG_LWIP_LAYER
#if (defined(CONFIG_EXAMPLE_UART_ATCMD) && CONFIG_EXAMPLE_UART_ATCMD) || (defined(CONFIG_EXAMPLE_SPI_ATCMD) && CONFIG_EXAMPLE_SPI_ATCMD)
			at_printf("%02x:%02x:%02x:%02x:%02x:%02x,", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
			at_printf("%d.%d.%d.%d,", ip[0], ip[1], ip[2], ip[3]);
			at_printf("%d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);
#endif
			printf("\n\rInterface (%s)", ifname[i]);
			printf("\n\r==============================");
			printf("\n\r\tMAC => %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			printf("\n\r\tIP  => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
			printf("\n\r\tGW  => %d.%d.%d.%d", gw[0], gw[1], gw[2], gw[3]);
			printf("\n\r\tmsk  => %d.%d.%d.%d\n\r", msk[0], msk[1], msk[2], msk[3]);

			wext_get_bssid((const char *)ifname[i], mac_1);
			printf("\n\r\tBSSID => %02x:%02x:%02x:%02x:%02x:%02x\n\r", mac_1[0], mac_1[1], mac_1[2], mac_1[3], mac_1[4], mac_1[5]);
#if LWIP_IPV6
			printf("\n\r\tLink-local IPV6 => %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n\r",
				ipv6_0[0], ipv6_0[1],  ipv6_0[2],  ipv6_0[3],  ipv6_0[4],  ipv6_0[5],  ipv6_0[6], ipv6_0[7],
				ipv6_0[8], ipv6_0[9], ipv6_0[10], ipv6_0[11], ipv6_0[12], ipv6_0[13], ipv6_0[14], ipv6_0[15]);
#endif /* LWIP_IPV6 */
#endif
			if(setting.mode == RTW_MODE_AP || i == 1)
			{
				int client_number;
				struct {
					int    count;
					rtw_mac_t mac_list[AP_STA_NUM];
				} client_info;

				client_info.count = AP_STA_NUM;
				wifi_get_associated_client_list(&client_info, sizeof(client_info));

				printf("\n\rAssociated Client List:");
				printf("\n\r==============================");

				if(client_info.count == 0)
					printf("\n\rClient Num: %d\n\r", client_info.count);
				else
				{
					printf("\n\rClient Num: %d", client_info.count);
					for( client_number=0; client_number < client_info.count; client_number++ )
					{
						printf("\n\rClient %d:", client_number + 1);
						printf("\n\r\tMAC => "MAC_FMT"",
										MAC_ARG(client_info.mac_list[client_number].octet));
#if (defined(CONFIG_EXAMPLE_UART_ATCMD) && CONFIG_EXAMPLE_UART_ATCMD) || (defined(CONFIG_EXAMPLE_SPI_ATCMD) && CONFIG_EXAMPLE_SPI_ATCMD)
						at_printf("CLIENT : %d,"MAC_FMT"\r\n", client_number + 1, MAC_ARG(client_info.mac_list[client_number].octet));
#endif
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
						if(info){
							memcpy(info+info_sz, (void *)&client_info.mac_list[client_number], sizeof(rtw_mac_t));
							info_sz += sizeof(rtw_mac_t);
						}
#endif
					}
					printf("\n\r");
				}
			}
		}
// show the ethernet interface info
		else
		{
#if CONFIG_ETHERNET || CONFIG_BRIDGE
      if(i == NET_IF_NUM - 1)
      {
#if CONFIG_LWIP_LAYER
        mac = LwIP_GetMAC(&xnetif[i]);
        ip = LwIP_GetIP(&xnetif[i]);
        gw = LwIP_GetGW(&xnetif[i]);
#if CONFIG_BRIDGE
        printf("\n\rInterface BR\n");
#else
        printf("\n\rInterface ethernet\n");
#endif
        printf("\n\r==============================");
        printf("\n\r\tMAC => %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
        printf("\n\r\tIP  => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        printf("\n\r\tGW  => %d.%d.%d.%d\n\r", gw[0], gw[1], gw[2], gw[3]);
#endif // end CONFIG_LWIP_LAYER
#if CONFIG_BRIDGE
        printf("\n\r==============================");
        printf("\n\rport   mac                ageing");
        show_bridgeif_fdbd(&xnetif[i]);
#endif
      }
#endif // end CONFIG_ETHERNET
		}
	}

#if defined(configUSE_TRACE_FACILITY) && (configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS == 1)
	{
		signed char pcWriteBuffer[1024];
		vTaskList((char*)pcWriteBuffer);
		printf("\n\rTask List: \n\r%s", pcWriteBuffer);
	}
#endif

#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATW?", ret, NULL, 0);
	else
		inic_c2h_msg("ATW?", RTW_SUCCESS, (char *)info, info_sz);

	if(info)
		free(info);
	info = NULL;
#endif

#if (defined(CONFIG_EXAMPLE_UART_ATCMD) && CONFIG_EXAMPLE_UART_ATCMD) || (defined(CONFIG_EXAMPLE_SPI_ATCMD) && CONFIG_EXAMPLE_SPI_ATCMD)
	at_printf("%sOK\r\n", "+WLSTATE:");
	ATCMD_NEWLINE_HASHTAG();
#endif

}

#if ATCMD_VER == ATVER_1
#ifdef CONFIG_GHD_EN
void fGHPM(void *arg)
{
	(void) arg;
	ghd_enter_pair_linkback_mode(1);
}

void fGHLB(void *arg)
{
	(void) arg;
	ghd_enter_pair_linkback_mode(2);
}

void fGHRL(void *arg)
{
	unsigned int retry_limit;
	retry_limit = (unsigned char) atoi((const char *)arg);
	rltk_wlan_set_retry_limit(retry_limit);
}

void fGHRA(void* arg)
{
	char* c_arg = (char*)arg;
	unsigned char j = 0;
	unsigned int rate_fallback_1_4 = 0;
	unsigned int rate_fallback_5_8 = 0;
	unsigned char len = strlen(c_arg);
	
	for (int i = 0; i <4; i++) {
		if (j == len)
			break;
		rate_fallback_1_4 |= (*(c_arg + j) - '0') << (i*8);
		j++;
	}
	
	for (int i = 0; i <4; i++) {
		if (j == len)
			break;
		rate_fallback_5_8 |= (*(c_arg + j) - '0') << (i*8);
		j++;
	}

	rltk_wlan_set_auto_rate_fallback(rate_fallback_1_4, rate_fallback_5_8);
}
#endif
void fATW0(void *arg){
	volatile int ret = RTW_SUCCESS;
	(void) ret;
        if(!arg){
          printf("[ATW0]Usage: ATW0=SSID\n\r");
		ret = RTW_BADARG;
		goto exit;
        }
	printf("[ATW0]: _AT_WLAN_SET_SSID_ [%s]\n\r", (char*)arg);
	strncpy((char *)wifi.ssid.val, (char*)arg, sizeof(wifi.ssid.val));
	wifi.ssid.len = strlen((char*)arg);
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW0", ret, NULL, 0);
#endif
	return;
}

void fATW1(void *arg){
	volatile int ret = RTW_SUCCESS;
	(void) ret;
	if(!arg){
		printf("[ATW1]Usage: ATW1=PASSPHRASE\n\r");
		ret = RTW_BADARG;
		goto exit;
	}	
	printf("[ATW1]: _AT_WLAN_SET_PASSPHRASE_ [%s]\n\r", (char*)arg); 

	strncpy((char *)password, (char*)arg, sizeof(password));	
	wifi.password = password;
	wifi.password_len = strlen((char*)arg);
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW1", ret, NULL, 0);
#endif
	return;	
}

void fATW2(void *arg){
	volatile int ret = RTW_SUCCESS;
	(void) ret;
	if(!arg){
		printf("[ATW2]Usage: ATW2=KEYID\n\r");
		ret = RTW_BADARG;
		goto exit;
	}	
        printf("[ATW2]: _AT_WLAN_SET_KEY_ID_ [%s]\n\r", (char*)arg);
		if((strlen((const char *)arg) != 1 ) || (*(char*)arg <'0' ||*(char*)arg >'3')) {
			printf("\n\rWrong WEP key id. Must be one of 0,1,2, or 3.");
			ret = RTW_BADARG;
			goto exit;
		}
	wifi.key_id = atoi((const char *)(arg));
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW2", ret, NULL, 0);
#endif
	return;
}

void fATW3(void *arg){
	volatile int ret = RTW_SUCCESS;
	(void) ret;
        if(!arg){
          printf("[ATW3]Usage: ATW3=SSID\n\r");
		ret = RTW_BADARG;
		goto exit;
        }

	ap.ssid.len = strlen((char*)arg);

	if(ap.ssid.len > 32){
          printf("[ATW3]Error: SSID length can't exceed 32\n\r");
		ret = RTW_BADARG;
		goto exit;
    }
	strncpy((char *)ap.ssid.val, (char*)arg, sizeof(ap.ssid.val));

	printf("[ATW3]: _AT_WLAN_AP_SET_SSID_ [%s]\n\r", ap.ssid.val); 
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW3", ret, NULL, 0);
#endif
	return;
}

void fATW4(void *arg){
    volatile int ret = RTW_SUCCESS;
	(void) ret;
    if(!arg){
        printf("[ATW4]Usage: ATW4=PASSWORD\n\r");
        ret = RTW_BADARG;
        goto exit;
    }
    strncpy((char *)password, (char*)arg, sizeof(password));
    ap.password = password;
    ap.password_len = strlen((char*)arg);
    printf("[ATW4]: _AT_WLAN_AP_SET_SEC_KEY_ [%s]\n\r", ap.password);
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
    inic_c2h_msg("ATW4", ret, NULL, 0);
#endif
    return;
}

void fATW5(void *arg){
    volatile int ret = RTW_SUCCESS;
	(void) ret;
    if(!arg){
       printf("[ATW5]Usage: ATW5=CHANNEL\n\r");
       ret = RTW_BADARG;
       goto exit;
    }
    ap.channel = (unsigned char) atoi((const char *)arg);
    printf("[ATW5]: _AT_WLAN_AP_SET_CHANNEL_ [channel %d]\n\r", ap.channel);
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
    inic_c2h_msg("ATW5", ret, NULL, 0);
#endif
    return;
}

void fATW6(void *arg){
	u32		mac[ETH_ALEN];
	u32		i;
	volatile int ret = RTW_SUCCESS;
	(void) ret;
	if(!arg){
		printf("[ATW6]Usage: ATW6=BSSID\n\r");
		ret =  RTW_BADARG;
		goto exit;
	}
	printf("[ATW6]: _AT_WLAN_SET_BSSID_ [%s]\n\r", (char*)arg);
	sscanf(arg, MAC_FMT, mac, mac + 1, mac + 2, mac + 3, mac + 4, mac + 5);
	for(i = 0; i < ETH_ALEN; i ++)
		wifi.bssid.octet[i] = (u8)mac[i] & 0xFF;
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW6", ret, NULL, 0);
#endif
	return;
}

#ifdef CONFIG_AP_SECURITY
void fATW7(void *arg){
	if(!arg){
		printf("[ATW7]Usage: ATW7=[security]\n\r");
		printf("        0 : Open\n\r");
		printf("        1 : WEP\n\r");
		printf("        2 : WPA2 AES\n\r");
		printf("        3 : WPA3 AES\n\r");
		printf("        4 : WPA2/WPA3 AES\n\r");
		return;
	}
	volatile int ret = RTW_SUCCESS;
	(void) ret;
	printf("[ATW7]: _AT_WLAN_SET_SECURITY [%s]\n\r", (char*)arg);
	if((strlen((const char *)arg) != 1 ) || (*(char*)arg <'0' ||*(char*)arg >'4')) {
		printf("\n\rWrong num. Must be one of 0,1,2,3 or 4.");
		ret = RTW_BADARG;
		goto exit;
	}
	security = atoi((const char *)(arg));
	if ((security == 1) && (CONFIG_AP_SECURITY != 1)) {
		printf("\n\rset CONFIG_AP_SECURITY MACRO to 1 in platform_opts.h to set AP security to 1(WEP)");
	}
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATW7", ret, NULL, 0);
#endif
	return;
}
#endif

void fATW8(void *arg){
	if(!arg){
		printf("[ATW8]Usage: ATW8=[WPA_MODE]\n\r");
		printf("        0 : WPA_AUTO_MODE\n\r");
		printf("        1 : WPA_ONLY_MODE\n\r");
		printf("        2 : WPA2_ONLY_MODE\n\r");
		printf("        3 : WPA3_ONLY_MODE\n\r");
		printf("        4 : WPA_WPA2_MIXED_MODE\n\r");
		printf("        5 : WPA2_WPA3_MIXED_MODE\n\r");
		return;
	}
	u32 wpa_mode = (u32) atoi((const char *)arg);

	if(wpa_mode<=WPA2_WPA3_MIXED_MODE){
		wifi_set_wpa_mode(wpa_mode);
		switch(wpa_mode){
			case 0:
				printf("[ATW8]: _AT_WLAN_AP_SET_WPA_MODE_ [WPA_AUTO_MODE]\n\r");
				break;
			case 1:
				printf("[ATW8]: _AT_WLAN_AP_SET_WPA_MODE_ [WPA_ONLY_MODE]\n\r");
				break;
			case 2:
				printf("[ATW8]: _AT_WLAN_AP_SET_WPA_MODE_ [WPA2_ONLY_MODE]\n\r");
				break;
			case 3:
				printf("[ATW8]: _AT_WLAN_AP_SET_WPA_MODE_ [WPA3_ONLY_MODE]\n\r");
				break;
			case 4:
				printf("[ATW8]: _AT_WLAN_AP_SET_WPA_MODE_ [WPA_WPA2_MIXED_MODE]\n\r");
				break;
			case 5:
				printf("[ATW8]: _AT_WLAN_AP_SET_WPA_MODE_ [WPA2_WPA3_MIXED_MODE]\n\r");
				break;
			default:
				printf("[ATW8]: _AT_WLAN_AP_SET_WPA_MODE_ [WRONG WPA MODE]\n\r");
				break;
		}
	}
	else{
		printf("[ATW8] Wrong parameter\n\r");
	}

	return;
}

#if CONFIG_AUTO_RECONNECT
void fATW9(void *arg){
	u8 mode = (u8) atoi((const char *)arg);

	if(!arg){
		printf("[ATW9] Usage : ATW9=[AUTO_RECONNECT_MODE_ENABLE]\n\r");
		printf("        0 : Disable auto-reconnect\n\r");
		printf("        1 : Enable auto-reconnect\n\r");
		return;
	}

	if(mode == 0){
		wifi_set_autoreconnect(0);
		printf("[ATW9] Disable auto-reconnect\n\r");
	}else if(mode == 1){
		wifi_set_autoreconnect(1);
		printf("[ATW9]: Enable auto-reconnect\n\r");
	}else{
		printf("[ATW9] Wrong parameter\n\r");
	}

	return;
}
#endif

void fATWA(void *arg){
	/* To avoid gcc warnings */
	( void ) arg;
#if CONFIG_LWIP_LAYER
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	struct netif * pnetif = &xnetif[0];
#endif
	int timeout = 20;
	volatile int ret = RTW_SUCCESS;
	printf("[ATWA]: _AT_WLAN_AP_ACTIVATE_\n\r"); 
	if(ap.ssid.val[0] == 0){
		printf("[ATWA]Error: SSID can't be empty\n\r");
		ret = RTW_BADARG;
		goto exit;
	}
	if(ap.password == NULL){
		ap.security_type = RTW_SECURITY_OPEN;
	}
	else{
		if(ap.password_len <= RTW_WPA2_MAX_PSK_LEN &&
			ap.password_len >= RTW_MIN_PSK_LEN){ 
			ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
			if(ap.password_len == RTW_WPA2_MAX_PSK_LEN){//password_len=64 means pre-shared key, pre-shared key should be 64 hex characters
				unsigned char i,j;
				for(i = 0;i < RTW_WPA2_MAX_PSK_LEN;i++){
					j = ap.password[i];
					if(!((j >='0' && j<='9') || (j >='A' && j<='F') || (j >='a' && j<='f'))){
						printf("[ATWA]Error: password should be 64 hex characters or 8-63 ASCII characters\n\r");
						ret = RTW_INVALID_KEY;
						goto exit;
					}
				}
			}
		}
#if defined(CONFIG_AP_SECURITY) && CONFIG_AP_SECURITY
		else if(ap.password_len == 5){
			ap.security_type = RTW_SECURITY_WEP_PSK;
		}
#endif
		else{
			printf("[ATWA]Error: password should be 64 hex characters or 8-63 ASCII characters\n\r");
			ret = RTW_INVALID_KEY;
			goto exit;
		}
	}
#ifdef CONFIG_AP_SECURITY
		if(security == 0)
			ap.security_type = RTW_SECURITY_OPEN;
#if CONFIG_AP_SECURITY
		else if(security == 1)
			ap.security_type = RTW_SECURITY_WEP_PSK;
#endif
		else if(security == 2)
			ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
		else if(security == 3)
			ap.security_type = RTW_SECURITY_WPA3_AES_PSK;
		else if(security == 4)
			ap.security_type = RTW_SECURITY_WPA2_WPA3_MIXED;
#endif

#if CONFIG_LWIP_LAYER
	dhcps_deinit();
#endif

#if (defined(CONFIG_PLATFORM_8710C)|| defined(CONFIG_PLATFORM_8721D)) && (defined(CONFIG_BT) && CONFIG_BT)
	if (wifi_set_mode(RTW_MODE_AP) < 0){
	    printf("\n\rERROR: Wifi on failed!");
	    ret = RTW_ERROR;
	    goto exit;
	}
#else
	wifi_off();
	vTaskDelay(20);
	if (wifi_on(RTW_MODE_AP) < 0){
		printf("\n\rERROR: Wifi on failed!");
		ret = RTW_ERROR;
		goto exit;
	}
#endif
	printf("\n\rStarting AP ...");

#if defined(CONFIG_ENABLE_WPS_AP) && CONFIG_ENABLE_WPS_AP
	wpas_wps_dev_config(pnetif->hwaddr, 1);
#endif
	if((ret = wifi_start_ap((char*)ap.ssid.val, ap.security_type, (char*)ap.password, ap.ssid.len, ap.password_len, ap.channel) )< 0) {
		printf("\n\rERROR: Operation failed!");
		goto exit;
	}

	while(1) {
		char essid[33];

		if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) > 0) {
			if(strcmp((const char *) essid, (const char *)ap.ssid.val) == 0) {
				printf("\n\r%s started\n", ap.ssid.val);
				ret = RTW_SUCCESS;
				break;
			}
		}

		if(timeout == 0) {
			printf("\n\rERROR: Start AP timeout!");
			ret = RTW_TIMEOUT;		
			break;
		}

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout --;
	}
	
#if defined( CONFIG_ENABLE_AP_POLLING_CLIENT_ALIVE )&&( CONFIG_ENABLE_AP_POLLING_CLIENT_ALIVE == 1 )
	wifi_set_ap_polling_sta(1);
#endif

#if CONFIG_LWIP_LAYER
	//LwIP_UseStaticIP(pnetif);
#if LWIP_VERSION_MAJOR >= 2
	IP4_ADDR(ip_2_ip4(&ipaddr), GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	IP4_ADDR(ip_2_ip4(&netmask), NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
	IP4_ADDR(ip_2_ip4(&gw), GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	netif_set_addr(pnetif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask),ip_2_ip4(&gw));
#else
	IP4_ADDR(&ipaddr, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
	IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	netif_set_addr(pnetif, &ipaddr, &netmask,&gw);
#endif
#ifdef CONFIG_DONT_CARE_TP
	pnetif->flags |= NETIF_FLAG_IPSWITCH;
#endif
	dhcps_init(pnetif);
#endif

exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_wifi_info("ATWA", ret);
#endif
	init_wifi_struct( );
}

static int _find_ap_from_scan_buf(char*buf, int buflen, char *target_ssid, void *user_data)
{
	rtw_wifi_setting_t *pwifi = (rtw_wifi_setting_t *)user_data;
	int plen = 0;
	
	while(plen < buflen){
		u8 len, ssid_len, security_mode;
		char *ssid;

		// len offset = 0
		len = (int)*(buf + plen);
		// check end
		if(len == 0) break;
		// ssid offset = 14
		ssid_len = len - 14;
		ssid = buf + plen + 14 ;
		if((ssid_len == strlen(target_ssid))
			&& (!memcmp(ssid, target_ssid, ssid_len)))
		{
			strncpy((char*)pwifi->ssid, target_ssid, 33);
			// channel offset = 13
			pwifi->channel = *(buf + plen + 13);
			// security_mode offset = 11
			security_mode = (u8)*(buf + plen + 11);
			if(security_mode == RTW_ENCODE_ALG_NONE)
				pwifi->security_type = RTW_SECURITY_OPEN;
			else if(security_mode == RTW_ENCODE_ALG_WEP)
				pwifi->security_type = RTW_SECURITY_WEP_PSK;
			else if(security_mode == RTW_ENCODE_ALG_CCMP)
				pwifi->security_type = RTW_SECURITY_WPA2_AES_PSK;
		}
		plen += len;
	}
	return 0;
}

static int _get_ap_security_mode(IN char * ssid, OUT rtw_security_t *security_mode, OUT u8 * channel)
{
	rtw_wifi_setting_t wifi;
	u32 scan_buflen = 1000;

	memset(&wifi, 0, sizeof(wifi));

	if(wifi_scan_networks_with_ssid(_find_ap_from_scan_buf, (void*)&wifi, scan_buflen, ssid, strlen(ssid)) != RTW_SUCCESS){
		printf("Wifi scan failed!\n");
		return 0;
	}

	if(strcmp((const char *)wifi.ssid, ssid) == 0){
		*security_mode = wifi.security_type;
		*channel = wifi.channel;
		return 1;
	}
	
	return 0;
}
extern u8 rltk_wlan_channel_switch_announcement_is_enable(void);
void fATWC(void *arg){
	/* To avoid gcc warnings */
	( void ) arg;

	int mode, ret;
	unsigned long tick1 = xTaskGetTickCount();
	unsigned long tick2, tick3;
	char empty_bssid[6] = {0}, assoc_by_bssid = 0;

	printf("[ATWC]: _AT_WLAN_JOIN_NET_\n\r");
	if(memcmp (wifi.bssid.octet, empty_bssid, 6))
		assoc_by_bssid = 1;
	else if(wifi.ssid.val[0] == 0){
		printf("[ATWC]Error: SSID can't be empty\n\r");
		ret = RTW_BADARG;
		goto EXIT;
	}
	if(wifi.password != NULL){
		if((wifi.key_id >= 0)&&(wifi.key_id <= 3)) {
			wifi.security_type = RTW_SECURITY_WEP_PSK;
		}
		else{
			wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
		}
	}
	else{
		wifi.security_type = RTW_SECURITY_OPEN;
	}
	//Check if in AP mode
	wext_get_mode(WLAN0_NAME, &mode);
	if(mode == RTW_MODE_MASTER) {
#if CONFIG_LWIP_LAYER
		dhcps_deinit();
#endif

#if (defined(CONFIG_PLATFORM_8710C)||defined(CONFIG_PLATFORM_8721D)) && (defined(CONFIG_BT) && CONFIG_BT)
		if (wifi_set_mode(RTW_MODE_STA) < 0){
		    printf("\n\rERROR: Wifi on failed!");
		    ret = RTW_ERROR;
		    goto EXIT;
		}
#else	
		wifi_off();
		vTaskDelay(20);
		if (wifi_on(RTW_MODE_STA) < 0){
			printf("\n\rERROR: Wifi on failed!");
            ret = RTW_ERROR;
			goto EXIT;
		}
#endif
	}

#if CONFIG_INIC_EN //get security mode from scan list
	u8 connect_channel;
	u8 pscan_config;
	//the keyID may be not set for WEP which may be confued with WPA2 
	if((wifi.security_type == RTW_SECURITY_UNKNOWN)||(wifi.security_type == RTW_SECURITY_WPA2_AES_PSK))
	{
		int security_retry_count = 0;
		while (1) {
			if (_get_ap_security_mode((char*)wifi.ssid.val, &wifi.security_type, &connect_channel))
				break;
			security_retry_count++;
			if(security_retry_count >= 3){
				printf("Can't get AP security mode and channel.\n");
				ret = RTW_NOTFOUND;
				goto EXIT;
			}
		}
		if(wifi.security_type == RTW_SECURITY_WEP_PSK || wifi.security_type == RTW_SECURITY_WEP_SHARED)
			wifi.key_id = (wifi.key_id <0 || wifi.key_id >3)?0:wifi.key_id;
#if 0 //implemented in wifi_connect()
		//hex to ascii conversion
		if(wifi.security_type == RTW_SECURITY_WEP_PSK)
		{
			if(wifi.password_len == 10)
			{
				u32 p[5];
				u8 pwd[6], i = 0; 
				sscanf((const char*)wifi.password, "%02x%02x%02x%02x%02x", &p[0], &p[1], &p[2], &p[3], &p[4]);
				for(i=0; i< 5; i++)
					pwd[i] = (u8)p[i];
				pwd[5] = '\0';
				memset(wifi.password, 0, 65);
				strcpy((char*)wifi.password, (char*)pwd);
				wifi.password_len = 5;
			}else if(wifi.password_len == 26){
				u32 p[13];
				u8 pwd[14], i = 0;
				sscanf((const char*)wifi.password, "%02x%02x%02x%02x%02x%02x%02x"\
				"%02x%02x%02x%02x%02x%02x", &p[0], &p[1], &p[2], &p[3], &p[4],\
				&p[5], &p[6], &p[7], &p[8], &p[9], &p[10], &p[11], &p[12]);
				for(i=0; i< 13; i++)
					pwd[i] = (u8)p[i];
				pwd[13] = '\0';
				memset(wifi.password, 0, 65);
				strcpy((char*)wifi.password, (char*)pwd);
				wifi.password_len = 13;
			}
		}
#endif
	}
        pscan_config = PSCAN_ENABLE;
	if(connect_channel > 0 && connect_channel < 14)
		wifi_set_pscan_chan(&connect_channel, &pscan_config, 1);
#endif

	if(rltk_wlan_channel_switch_announcement_is_enable()){
		if(wifi_mode == RTW_MODE_STA_AP){
			u8 connect_channel;
			int security_retry_count = 0;
			while (1) {
				if (_get_ap_security_mode((char*)wifi.ssid.val, &wifi.security_type, &connect_channel))
					break;
				security_retry_count++;
				if(security_retry_count >= 3){
					printf("Can't get AP security mode and channel.\n");
					ret = RTW_NOTFOUND;
					goto EXIT;
				}
			}
			//disable wlan1 issue_deauth when channel switched by wlan0
			ret = wifi_set_ch_deauth(0);
			if(ret != 0){
				printf("wifi_set_ch_deauth FAIL\n");
				goto EXIT;
			}
			//softap switch chl and inform
			if(wifi_ap_switch_chl_and_inform(connect_channel)!=RTW_SUCCESS)
				printf("wifi_ap_switch_chl_and_inform FAIL\n");
		}
	}

	if(assoc_by_bssid){
		printf("\n\rJoining BSS by BSSID "MAC_FMT" ...\n\r", MAC_ARG(wifi.bssid.octet));
		ret = wifi_connect_bssid(wifi.bssid.octet, (char*)wifi.ssid.val, wifi.security_type, (char*)wifi.password, 
						ETH_ALEN, wifi.ssid.len, wifi.password_len, wifi.key_id, NULL);		
	} else {
		printf("\n\rJoining BSS by SSID %s...\n\r", (char*)wifi.ssid.val);
		ret = wifi_connect((char*)wifi.ssid.val, wifi.security_type, (char*)wifi.password, wifi.ssid.len,
						wifi.password_len, wifi.key_id, NULL);
	}
	
	if(ret!= RTW_SUCCESS){
		if(ret == RTW_INVALID_KEY)
			printf("\n\rERROR:Invalid Key ");
		
		printf("\n\rERROR: Can't connect to AP");
		goto EXIT;
	}

	tick2 = xTaskGetTickCount();
	printf("\r\nConnected after %dms.\n", (tick2-tick1));

#if WIFI_LOGO_CERTIFICATION_CONFIG
	if(!use_static_ip)
#endif
	{
#if CONFIG_LWIP_LAYER
		/* Start DHCPClient */
		LwIP_DHCP(0, DHCP_START);
		tick3 = xTaskGetTickCount();
		printf("\r\n\nGot IP after %dms.\n", (tick3-tick1));
#endif
	}
	printf("\n\r");
EXIT:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_wifi_info("ATWC", ret);
#endif

	init_wifi_struct( );
}

#if SCAN_WITH_SSID
void fATWs(void *arg){
	char buf[32] = {0};
	u8 *channel_list = NULL;
	u8 *pscan_config = NULL;
	int scan_buf_len = 500;
	int num_channel = 0;
	int i, argc = 0;
	char *argv[MAX_ARGC] = {0};
	printf("[ATWs]: _AT_WLAN_SCAN_WITH_SSID_ [%s]\n\r",  (char*)wifi.ssid.val);
	if(arg){
		strncpy(buf, arg, sizeof(buf));
		argc = parse_param(buf, argv);
		if(argc == 2){
			scan_buf_len = atoi(argv[1]);  	
			if(scan_buf_len < 36){
				printf("[ATWs] BUFFER_LENGTH too short\n\r");
				goto exit;
			}
		}else if(argc > 2){
			num_channel = atoi(argv[1]);
			channel_list = (u8*)malloc(num_channel);
			if(!channel_list){
				printf("[ATWs]ERROR: Can't malloc memory for channel list\n\r");
				goto exit;
			}
			pscan_config = (u8*)malloc(num_channel);
			if(!pscan_config){
				printf("[ATWs]ERROR: Can't malloc memory for pscan_config\n\r");
				goto exit;
		  	}			
			//parse command channel list
			for(i = 2; i <= argc -1 ; i++){
				*(channel_list + i - 2) = (u8)atoi(argv[i]);
				*(pscan_config + i - 2) = PSCAN_ENABLE;	
			}

			if(wifi_set_pscan_chan(channel_list, pscan_config, num_channel) < 0){
				printf("[ATWs]ERROR: wifi set partial scan channel fail\n\r");
				goto exit;
			}
		}
	}else{
		printf("[ATWs]For Scan all channel Usage: ATWs=BUFFER_LENGTH\n\r");          
		printf("[ATWs]For Scan partial channel Usage: ATWs=num_channels[channel_num1, ...]\n\r");
		goto exit;
	}

	if(wifi_scan_networks_with_ssid_by_extended_security(NULL, NULL, scan_buf_len, (char*)wifi.ssid.val, wifi.ssid.len) != RTW_SUCCESS){
		printf("[ATWs]ERROR: wifi scan failed\n\r");
	}
exit:
	init_wifi_struct( );
	if(arg && channel_list)
		free(channel_list);
	if(arg && pscan_config)
		free(pscan_config);	
}
#endif

void fATWR(void *arg){
	/* To avoid gcc warnings */
	( void ) arg;
	
	int rssi = 0;
	printf("[ATWR]: _AT_WLAN_GET_RSSI_\n\r"); 
	wifi_get_rssi(&rssi);
	printf("\n\rwifi_get_rssi: rssi = %d", rssi);
	printf("\n\r");
}

void fATWP(void *arg){
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	if(!arg){
		printf("[ATWP]Usage: ATWP=0/1\n\r");
		return;
	}
	else
	{
		argc = parse_param(arg, argv);
		if (argc != 2) {
			printf("[ATWP]Usage: ATWP=0/1\n\r");
			return;
		}
	}
	if(argv[1] != NULL){
		if (strcmp(argv[1],"1") == 0) {
			if(wifi_on(RTW_MODE_STA)<0){
				printf("\n\rERROR: Wifi on failed!\n");
			}
		}
		else if (strcmp(argv[1],"0") == 0) {
				wifi_off();
		}
		else
		{
			printf("[ATWP]Usage: ATWP=0/1\n\r");
		}
	}
}

#if CONFIG_WOWLAN_SERVICE
//for wowlan setting
void fATWV(void *arg){
	int argc;
	char *argv[MAX_ARGC] = {0};
	
	printf("[ATWV]: _AT_WLAN_WOWLAN_\r\n");

	argc = parse_param(arg, argv);

	cmd_wowlan_service(argc, argv);

	return;
}
#endif

#ifdef  CONFIG_CONCURRENT_MODE
void fATWB(void *arg)
{
	/* To avoid gcc warnings */
	( void ) arg;
	
	int timeout = 20;//, mode;
	volatile int ret = RTW_SUCCESS;
#if CONFIG_LWIP_LAYER
	struct netif * pnetiff = (struct netif *)&xnetif[1];
#endif
	printf("[ATWB](_AT_WLAN_AP_STA_ACTIVATE_)\n\r"); 
	if(ap.ssid.val[0] == 0){
          printf("[ATWB]Error: SSID can't be empty\n\r");
		ret = RTW_BADARG;
		goto exit;
        }
	if(ap.password == NULL){
          ap.security_type = RTW_SECURITY_OPEN;
	} else {
		if(ap.password_len <= RTW_WPA2_MAX_PSK_LEN &&
			ap.password_len >= RTW_MIN_PSK_LEN){ 
			ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
			if(ap.password_len == RTW_WPA2_MAX_PSK_LEN){//password_len=64 means pre-shared key, pre-shared key should be 64 hex characters
				unsigned char i,j;
				for(i = 0;i < RTW_WPA2_MAX_PSK_LEN;i++){
					j = ap.password[i];
					if(!((j >='0' && j<='9') || (j >='A' && j<='F') || (j >='a' && j<='f'))){
						printf("[ATWB]Error: password should be 64 hex characters or 8-63 ASCII characters\n\r");
						ret = RTW_INVALID_KEY;
						goto exit;
					}
				}
			}
		} else {
			printf("[ATWB]Error: password should be 64 hex characters or 8-63 ASCII characters\n\r");
			ret = RTW_INVALID_KEY;
			goto exit;
		}
	}
#if CONFIG_LWIP_LAYER
	dhcps_deinit();
#endif

#if (defined(CONFIG_PLATFORM_8710C) || defined(CONFIG_PLATFORM_8721D)) && (defined(CONFIG_BT) && CONFIG_BT)
	if (wifi_set_mode(RTW_MODE_STA_AP) < 0){
		printf("\n\rERROR: Wifi on failed!");
		ret = RTW_ERROR;
		goto exit;
	}
#else
	wifi_off();
	vTaskDelay(20);
	if ((ret = wifi_on(RTW_MODE_STA_AP)) < 0){
		printf("\n\rERROR: Wifi on failed!");
		ret = RTW_ERROR;
		goto exit;
	}
#endif

	printf("\n\rStarting AP ...");
	if((ret = wifi_start_ap((char*)ap.ssid.val, ap.security_type, (char*)ap.password, ap.ssid.len, ap.password_len, ap.channel)) < 0) {
		printf("\n\rERROR: Operation failed!");
		goto exit;
	}
	while(1) {
		char essid[33];

		if(wext_get_ssid(WLAN1_NAME, (unsigned char *) essid) > 0) {
			if(strcmp((const char *) essid, (const char *)ap.ssid.val) == 0) {
				printf("\n\r%s started\n", ap.ssid.val);
				ret = RTW_SUCCESS;
				break;
			}
		}

		if(timeout == 0) {
			printf("\n\rERROR: Start AP timeout!");
			ret = RTW_TIMEOUT;
			break;
		}

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout --;
	}
#if CONFIG_LWIP_LAYER
	LwIP_UseStaticIP(&xnetif[1]);
#ifdef CONFIG_DONT_CARE_TP
	pnetiff->flags |= NETIF_FLAG_IPSWITCH;
#endif
	dhcps_init(pnetiff);
#endif

#if defined( CONFIG_ENABLE_AP_POLLING_CLIENT_ALIVE )&&( CONFIG_ENABLE_AP_POLLING_CLIENT_ALIVE == 1 )
	wifi_set_ap_polling_sta(1);
#endif
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_wifi_info("ATWB", ret);
#endif
	init_wifi_struct();
}

void fATWb(void *arg)
{
	/* To avoid gcc warnings */
	volatile int ret = RTW_SUCCESS;
	( void ) ret;
	( void ) arg;
	int argc = 0;
	( void ) argc;
	char *argv[MAX_ARGC] = {0};
	if(!arg){
		printf("[ATWb]: _AT_WLAN_AP_STA_CONTROL_\n\r");
		printf("[ATWb] Usage: ATWI=[-s|-b]\n");
		printf("\n\r     -s    remove softap\n");
		printf("  \r     -b    add back softap\n");
		return;
	}
	else
	{
		argc = parse_param(arg, argv);
		if (strcmp(argv[1],"-s") == 0) {
			printf("[ATWb]: remove softap\n\r");
			wifi_set_mode(RTW_MODE_STA);
			goto exit;
		}
		else if (strcmp(argv[1],"-b") == 0){
			printf("[ATWb]: add back softap\n\r");
			wifi_set_mode(RTW_MODE_STA_AP);
			goto exit;
		}
		else{
			printf("[ATWb]: error usage\n\r");
			return;
		}
	}
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_wifi_info("ATWb", ret);
#endif
	init_wifi_struct();
	return;
}

//This AT cmd is used in concurrent mode after resume interface2, there's no wifi on/off in this command
void fATWa(void *arg)
{
	/* To avoid gcc warnings */
	( void ) arg;

	int timeout = 20;//, mode;
	volatile int ret = RTW_SUCCESS;
#if CONFIG_LWIP_LAYER
	struct netif * pnetiff = (struct netif *)&xnetif[1];
#endif
	printf("[ATWa](_AT_WLAN_START_AP_ON_IF2_)\n\r");
	if(ap.ssid.val[0] == 0){
          printf("[ATWa]Error: SSID can't be empty\n\r");
		ret = RTW_BADARG;
		goto exit;
        }
	if(ap.password == NULL){
          ap.security_type = RTW_SECURITY_OPEN;
        }
	else{
		if(ap.password_len <= RTW_WPA2_MAX_PSK_LEN &&
			ap.password_len >= RTW_MIN_PSK_LEN){
			ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
			if(ap.password_len == RTW_WPA2_MAX_PSK_LEN){//password_len=64 means pre-shared key, pre-shared key should be 64 hex characters
				unsigned char i,j;
				for(i = 0;i < RTW_WPA2_MAX_PSK_LEN;i++){
					j = ap.password[i];
					if(!((j >='0' && j<='9') || (j >='A' && j<='F') || (j >='a' && j<='f'))){
						printf("[ATWa]Error: password should be 64 hex characters or 8-63 ASCII characters\n\r");
						ret = RTW_INVALID_KEY;
						goto exit;
					}
				}
			}
		}
		else{
			printf("[ATWa]Error: password should be 64 hex characters or 8-63 ASCII characters\n\r");
			ret = RTW_INVALID_KEY;
			goto exit;
		}
	}

#if CONFIG_LWIP_LAYER
	dhcps_deinit();
#endif

	printf("\n\rStarting AP ...");
	if((ret = wifi_start_ap((char*)ap.ssid.val, ap.security_type, (char*)ap.password, ap.ssid.len, ap.password_len, ap.channel)) < 0) {
		printf("\n\rERROR: Operation failed!");
		goto exit;
	}
	while(1) {
		char essid[33];

		if(wext_get_ssid(WLAN1_NAME, (unsigned char *) essid) > 0) {
			if(strcmp((const char *) essid, (const char *)ap.ssid.val) == 0) {
				printf("\n\r%s started\n", ap.ssid.val);
				ret = RTW_SUCCESS;
				break;
			}
		}

		if(timeout == 0) {
			printf("\n\rERROR: Start AP timeout!");
			ret = RTW_TIMEOUT;
			break;
		}

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout --;
	}
#if CONFIG_LWIP_LAYER
	LwIP_UseStaticIP(&xnetif[1]);
#ifdef CONFIG_DONT_CARE_TP
	pnetiff->flags |= NETIF_FLAG_IPSWITCH;
#endif
	dhcps_init(pnetiff);
#endif

#if defined( CONFIG_ENABLE_AP_POLLING_CLIENT_ALIVE )&&( CONFIG_ENABLE_AP_POLLING_CLIENT_ALIVE == 1 )
	wifi_set_ap_polling_sta(1);
#endif
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_wifi_info("ATWa", ret);
#endif
	init_wifi_struct();
}
#endif

#ifdef CONFIG_PROMISC
void fATWM(void *arg){ 
        int argc;
        char *argv[MAX_ARGC] = {0};
        argv[0] = "wifi_promisc";        
		printf("[ATWM]: _AT_WLAN_PROMISC_\n\r");
        if(!arg){
          printf("[ATWM]Usage: ATWM=DURATION_SECONDS[with_len]");
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
		inic_c2h_msg("ATWM", RTW_BADARG, NULL, 0);
#endif
          return;
        }
        if((argc = parse_param(arg, argv)) > 1){
          cmd_promisc(argc, argv);
        }        
}
#endif

void fATWW(void *arg){
#if CONFIG_ENABLE_WPS
        int argc = 0;
        char *argv[4];
        printf("[ATWW]: _AT_WLAN_WPS_\n\r");
        if(!arg){
          printf("[ATWW]Usage: ATWW=pbc/pin\n\r");
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
		inic_c2h_msg("ATWW", RTW_BADARG, NULL, 0);
#endif
          return;
        }
        argv[0] = "wifi_wps";
        if((argc = parse_param(arg, argv)) > 1){
            cmd_wps(argc, argv);
        }
#else
	printf("Please set CONFIG_ENABLE_WPS 1 in platform_opts.h to enable ATWW command\n");
#endif
}
void fATWw(void *arg){
	/* To avoid gcc warnings */
	( void ) arg;
#if defined(CONFIG_ENABLE_WPS_AP) && CONFIG_ENABLE_WPS_AP
        int argc = 0;
        char *argv[4];
        printf("[ATWw]: _AT_WLAN_AP_WPS_\n\r");
        if(!arg){
          printf("[ATWw]Usage: ATWw=pbc/pin\n\r");
          return;
        }
        argv[argc++] = "wifi_ap_wps";
        argv[argc++] = arg;
        cmd_ap_wps(argc, argv);
#endif		
}

#if CONFIG_ENABLE_P2P
void fATWG(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWG]: _AT_WLAN_P2P_START_\n\r");
        argv[argc++] = "p2p_start";
        cmd_wifi_p2p_start(argc, argv);
}

void fATWg(void *arg){
	int argc = 0;
	char *argv[4];
	int ret =0;
	printf("[ATWg]: _AT_WLAN_P2P_AUTO_GO_START_\n\r");
	argv[argc++] = "p2p_auto_go_start";
	ret = cmd_wifi_p2p_auto_go_start(argc, argv);
	if(ret < 0)
		printf("\r\n[ATWG]: Nothing to do. Please enter ATWG to initialize P2P.\n\r");
}

void fATWH(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWH]: _AT_WLAN_P2P_STOP_\n\r");
        argv[argc++] = "p2p_stop";
        cmd_wifi_p2p_stop(argc, argv);
}
void fATWJ(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWJ]: _AT_WLAN_P2P_CONNECT_\n\r");
        argv[0] = "p2p_connect";
        if(!arg){
		printf("ATWc=[DEST_MAC,pbc/pin]\n\r");
		return;
        }
        if((argc = parse_param(arg, argv)) > 1){
		cmd_p2p_connect(argc, argv);
        }        
}
void fATWK(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWK]: _AT_WLAN_P2P_DISCONNECT_\n\r");
        argv[argc++] = "p2p_disconnect";
        cmd_p2p_disconnect(argc, argv);
}
void fATWN(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWN]: _AT_WLAN_P2P_INFO_\n\r");
        argv[argc++] = "p2p_info";
        cmd_p2p_info(argc, argv);
}
void fATWF(void *arg){
        int argc = 0;
        char *argv[4];
        printf("[ATWF]: _AT_WLAN_P2P_FIND_\n\r");
        argv[argc++] = "p2p_find";
        cmd_p2p_find(argc, argv);
}
#endif

#ifdef CONFIG_BT_COEXIST_SOC
extern int rltk_coex_set_wifi_slot(u8 wifi_slot);
void fATWE(void *arg){
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	u8 bitmask = 0;
	u8 wifi_slot = 0;
	int ret = 0;
	printf("[ATWE]: _AT_WLAN_BT_COEX_\n\r");

	if(!arg){
		printf("\n\r[ATWE] Usage: ATWE=[task_name],[parameter]\n");
		printf("\n\r 1) task_name: wifi_slot\n");
		printf("\n\r    parameter: value between 5 to 95. \n");
		printf("\n\r    parameter: the settings of wifi_slot is valid only when in (BLE SCAN + WIFI CONNECTED) status.\n");
		printf("\n\r 2) task_name: wlan_slot_random\n");
		printf("\n\r    parameter: 0 - off, 1 - on.\n");
		printf("\n\r 3) task_name: customer_option\n");
		printf("\n\r    parameter 1: 1 - Let WIFI always > BT when authenticating with WPA3-AP.\n");
		printf("\r                 0 - Use default case. \n");
		printf("\n\r    parameter 2: 1 - Let WIFI always > BT during the 4-Way with WPA3-AP.\n");
		printf("\r                 0 - Use default case. \n");
		printf("\n\r    parameter 3: 1 - Let WIFI > BT in wifi slot, BT > WIFI in bt slot when ble scan + wifi connected.\n");
		printf("\r                 0 - Use default case. \n");
		printf("\n\r   Example:\n");
		printf("\r     ATWE=wifi_slot,50\n");
		printf("\r     ATWE=wlan_slot_random,1\n");
		printf("\r     ATWE=customer_option,0,1,1\n");
		printf("\r     ATWE=wlan_lps_coex,1\n");
		return;
	}

	argv[0] = "coex_task";
	argc = parse_param(arg, argv);
	if(argc > 1){
		if (!strcmp(argv[1], "wifi_slot")) {
			wifi_slot = atoi(argv[2]);
			if (wifi_slot < 5) {
				printf("\n\r warning! wifi_slot < 5%,It wil be all bt.\n");
			} else if (wifi_slot > 95) {
				printf("\n\r warning! wifi_slot > 95%,It wil be all wifi.\n");
			}
			rltk_coex_set_wifi_slot(wifi_slot);
		} else if (!strcmp(argv[1], "wlan_slot_random")) {
			rltk_coex_set_wlan_slot_random(atoi(argv[2]));
		} else if (!strcmp(argv[1], "customer_option")) {
			bitmask = (atoi(argv[2])?1:0) | (atoi(argv[3])?2:0)\
				| (atoi(argv[4])?4:0);
			rltk_coex_set_wlan_slot_preempting(bitmask);
		} else if (!strcmp(argv[1], "wlan_lps_coex")) {
			ret = rltk_coex_set_wlan_lps_coex(atoi(argv[2]));
			if (ret < 0) {
				printf("\n This CMD should be used when BT is enabled.\n");
			}
		} else {
			printf("\n Wrong order.\n");
			printf("\n See usage by input: ATWE.\n");
		}
	} else {
		printf("\n Wrong Order.\n");
		printf("\n See usage by input: ATWE.\n");
	}
}
#endif //CONFIG_BT_COEXIST_SOC

#if CONFIG_OTA_UPDATE
void fATWO(void *arg){
        int argc = 0;
        char *argv[MAX_ARGC] = {0};
        printf("[ATWO]: _AT_WLAN_OTA_UPDATE_\n\r");
        if(!arg){
          printf("[ATWO]Usage: ATWO=IP[PORT] or ATWO= REPOSITORY[FILE_PATH]\n\r");
          return;
        }
        argv[0] = "update";
        if((argc = parse_param(arg, argv)) != 3){
          printf("[ATWO]Usage: ATWO=IP[PORT] or ATWO= REPOSITORY[FILE_PATH]\n\r");
          return;
        }
        cmd_update(argc, argv);
}
#endif

#if CONFIG_AIRKISS
void fATWX(void *arg)
{
	int argc;
	int ret = RTW_SUCCESS;
	unsigned char *argv[MAX_ARGC] = {0};
	
	argv[0] = "airkiss";
	argc = parse_param(arg, argv);
	if(argc == 2) {
		if( strcmp(argv[1], "start") == 0) {
			ret = airkiss_start(NULL);
		}else if(strcmp(argv[1], "stop") == 0) {
			ret = airkiss_stop();
		}else {
			printf("\r\n[ATWX] Usage: ATWX=[start/stop]");
		}
	}else {
		printf("\r\n[ATWX] start/stop airkiss config\r\n");
		printf("\r\n[ATWX] Usage: ATWX=[start/stop]");
		ret = RTW_ERROR;
	}
	
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATWX", RTW_ERROR, NULL, 0);
#endif	
}
#endif

void fATWZ(void *arg){
        char buf[32] = {0};
        char *copy = buf;
        int i = 0;
        int len = 0;
	volatile int ret = RTW_SUCCESS;
	(void) ret;

        printf("[ATWZ]: _AT_WLAN_IWPRIV_\n\r");
        if(!arg){
          printf("[ATWZ]Usage: ATWZ=COMMAND[PARAMETERS]\n\r");
		ret = RTW_BADARG;
		goto exit;
        }
        strncpy(copy, arg, sizeof(buf));
        len = strlen(copy);
        do{
          if((*(copy+i)=='['))
            *(copy+i)=' ';
          if((*(copy+i)==']')||(*(copy+i)=='\0')){
            *(copy+i)='\0';
            break;
          }
        }while((i++) < len);
        
        i = 0;
        do{
          if((*(copy+i)==',')) {
            *(copy+i)=' ';
            break;
          }
        }while((i++) < len);
        
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	ret = wext_private_command_with_retval(WLAN0_NAME, copy, buf, 32);
	printf("\n\rPrivate Message: %s", (char *) buf);
	if(ret == RTW_SUCCESS)
		inic_c2h_msg("ATWZ", ret, buf, strlen(buf));
#else
        wext_private_command(WLAN0_NAME, copy, 1);
#endif
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	if(ret != RTW_SUCCESS)
		inic_c2h_msg("ATWZ", ret, NULL, 0);
#endif
	return;	// exit label cannot be last statement
}

#ifdef CONFIG_POWER_SAVING
void fATXP(void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	volatile int ret = 0;
	(void) ret;
	int mode, dtim;
	int tdma_slot_period, tdma_rfon_period_len_1, tdma_rfon_period_len_2, tdma_rfon_period_len_3;
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	char *res = NULL;
	int res_len = 0;
#endif

	printf("[ATXP]: _AT_WLAN_POWER_MODE_\r\n");

	if (!arg) {
		printf("[ATXP] Usage: ATXP=lps/ips/dtim/tdma/bcn_mode[mode]\r\n");
		ret = RTW_BADARG;
		goto exit;
	} else {
		argc = parse_param(arg, argv);
		if (argc < 3) {
			printf("[ATXP] Usage: ATXP=lps/ips/dtim/tdma/bcn_mode[mode]\r\n");
			ret = RTW_BADARG;
			goto exit;
		}
	}

	if (strcmp(argv[1], "lps") == 0) {
		mode = atoi(argv[2]);
		if (mode >= 0 && mode < 0xFF) {
			printf("lps mode:%d\r\n", mode);
			wifi_set_power_mode(0xff, mode);
		}
	}

	if (strcmp(argv[1], "ips") == 0) {
		mode = atoi(argv[2]);
		if (mode >= 0 && mode < 0xFF) {
			printf("ips mode:%d\r\n", mode);
			wifi_set_power_mode(mode, 0xFF);
		}
	}

	if (strcmp(argv[1], "tdma") == 0) {
		if (argc >= 6) {
			tdma_slot_period = atoi(argv[2]);
			tdma_rfon_period_len_1 = atoi(argv[3]);
			tdma_rfon_period_len_2 = atoi(argv[4]);
			tdma_rfon_period_len_3 = atoi(argv[5]);
			printf("tdma param: %d %d %d %d\r\n", tdma_slot_period, tdma_rfon_period_len_1, tdma_rfon_period_len_2, tdma_rfon_period_len_3);
			wifi_set_tdma_param(tdma_slot_period, tdma_rfon_period_len_1, tdma_rfon_period_len_2, tdma_rfon_period_len_3);
		}
	}

	if (strcmp(argv[1], "dtim") == 0) {
		dtim = atoi(argv[2]);
		printf("dtim: %d\r\n", dtim);
		wifi_set_lps_dtim(dtim);
	}

	if (strcmp(argv[1], "bcn_mode") == 0) {
		mode = atoi(argv[2]);
		printf("Beacon mode: %d\r\n", mode);
		wifi_set_beacon_mode(mode);
	}

	if (strcmp(argv[1], "lps_thresh") == 0) {
		mode = atoi(argv[2]);
		printf("LPS thresh: ");
		if(mode == 0)
			printf("packet count threshold\n\r");
		else if(mode == 1)
			printf("enter lps directly\n\r");
		else
			printf("tp threshold\n\r");
		wifi_set_lps_thresh(mode);
	}

	if (strcmp(argv[1], "lps_level") == 0) {
		mode = atoi(argv[2]);
		printf("lps_level: %d\r\n", mode);
		wifi_set_lps_level(mode);
	}

	if (strcmp(argv[1], "get") == 0) {
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
		char buf[32];
		int index = 0;
		memset(buf, 0, sizeof(buf));
		snprintf(buf, 32, "%s,%s,", argv[1], argv[2]);
		index = strnlen(buf, sizeof(buf));
#endif
		if(strcmp(argv[2], "dtim") == 0){
			wifi_get_lps_dtim((unsigned char *)&dtim);
			printf("get dtim: %d\r\n", (unsigned char)dtim);
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
			snprintf(buf+index, sizeof(buf)-index, "0x%02x", (unsigned char)dtim);
			res = (char *)buf;
			res_len = strnlen(buf, sizeof(buf));
#endif
		}
	}
	
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATXP", ret, res, res_len);
	res = NULL;
	res_len = 0;
#endif
	return;
}
#endif

void print_wlan_help(void *arg){
	/* To avoid gcc warnings */
	( void ) arg;
	
	printf("\n\rWLAN AT COMMAND SET:");
	printf("\n\r==============================");
        printf("\n\r1. Wlan Scan for Network Access Point");
        printf("\n\r   # ATWS");
        printf("\n\r2. Connect to an AES AP");
        printf("\n\r   # ATW0=SSID");
        printf("\n\r   # ATW1=PASSPHRASE");
        printf("\n\r   # ATWC");
        printf("\n\r3. Create an AES AP");
        printf("\n\r   # ATW3=SSID");
        printf("\n\r   # ATW4=PASSPHRASE");
        printf("\n\r   # ATW5=CHANNEL");
        printf("\n\r   # ATWA");
        printf("\n\r4. Ping");
        printf("\n\r   # ATWI=xxx.xxx.xxx.xxx");
}

void fATWc(void *arg){
    volatile int ret = RTW_SUCCESS;
	(void) ret;
    if(!arg){
       printf("[ATWc]Usage: ATWc= 1/0 (Enable/Disable 40M bandwidth, default disable 40M)\n\r");
       ret = RTW_BADARG;
       goto exit;
    }
    u8 enable = (unsigned char) atoi((const char *)arg);
    wext_set_bw40_enable(enable);
    printf("[ATWc]: _AT_WLAN_ENABLE_40M_BANDWIDTH_ [%d]\n\r", enable);

exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
    inic_c2h_msg("ATWc", ret, NULL, 0);
#endif
    return;
}

void fATWd(void *arg){
    volatile int ret = RTW_SUCCESS;
	(void) ret;
    if(!arg){
       printf("[ATWd]Usage: ATWd= 0/1/2 (disable/normal/carrier_sense)\n\r");
       ret = RTW_BADARG;
       goto exit;
    }
    rtw_adaptivity_mode_t adaptivity_mode = (unsigned char) atoi((const char *)arg);
    wext_set_adaptivity(adaptivity_mode);
    printf("[ATWd]: _AT_WLAN_SET_ADAPTIVITY_ [%d]\n\r", adaptivity_mode);

exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
    inic_c2h_msg("ATWd", ret, NULL, 0);
#endif
    return;
}

#if WIFI_LOGO_CERTIFICATION_CONFIG

struct ip_addr g_ipaddr;
struct ip_addr g_netmask;
struct ip_addr g_gw;

void fATPE(void *arg)
{
    int argc, error_no = 0;
    char *argv[MAX_ARGC] = {0};
    unsigned int ip_addr = 0;
    //unsigned char sta_ip[4] = {192,168,3,80}, sta_netmask[4] = {255,255,255,0}, sta_gw[4] = {192,168,3,1};

    if(!arg){
        AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
            "\r\n[ATPE] Usage : ATPE=<ip>(,<gateway>,<mask>)");
        error_no = 1;
        goto exit;
    }

    argc = parse_param(arg, argv);
	
    if( (argc > 4) || (argc < 2) ){
        //at_printf("\r\n[ATPE] ERROR : command format error");
        error_no = 1;
        goto exit;
    }

    if(argv[1] != NULL){
        ip_addr = inet_addr(argv[1]);
		IP4_ADDR(ip_2_ip4(&g_ipaddr), ip_addr&0xff, (ip_addr>>8)&0xff, (ip_addr>>16)&0xff, (ip_addr>>24)&0xff);
    }
    else{
        //at_printf("\r\n[ATPE] ERROR : parameter format error");
        error_no = 2;
        goto exit;
    }

    if(argv[2] != NULL){
        ip_addr = inet_addr(argv[2]);
		IP4_ADDR(ip_2_ip4(&g_gw), ip_addr&0xff, (ip_addr>>8)&0xff, (ip_addr>>16)&0xff, (ip_addr>>24)&0xff);

    }
	
    if(argv[3] != NULL){
        ip_addr = inet_addr(argv[3]);
		IP4_ADDR(ip_2_ip4(&g_netmask), ip_addr&0xff, (ip_addr>>8)&0xff, (ip_addr>>16)&0xff, (ip_addr>>24)&0xff);

    }

	//IP4_ADDR(ip_2_ip4(&netmask), 255, 255, 255, 0);
	netif_set_addr(&xnetif[0], ip_2_ip4(&g_ipaddr), ip_2_ip4(&g_netmask),ip_2_ip4(&g_gw));

exit:
	if(error_no==0){
		at_printf("[ATPE] OK\r\n");
		use_static_ip = 1;
	}
    else
        at_printf("[ATPE] ERROR:%d\r\n",error_no);

    return;

}

#ifdef CONFIG_SAE_SUPPORT
void fATWGRP(void *arg){

    unsigned char grp_id = 0 , i = 0, error = 0;
	int target_grp_id[10] = {19, 20};

	if(!arg)
	{
		error = 1;
	}
	else
	{
		grp_id = atoi((const char *)(arg));
		
		for(i = 0; i < 2; i++)
			if(grp_id == target_grp_id[i])
				break;
		
		if(i == 2)
			error = 1;
	}
	
	if(error)
	{
		printf("[ATGP]error cmd  !!\n\r");
		printf("[ATGP]Usage: ATGP = group_id \n\r");
		printf("      *************************************************\n\r");
		printf("      ECC group: 19, 20 \n\r");
		printf("      *************************************************\n\r");
	}
	else
	{
		printf("[ATGP]: _AT_WLAN_SET_GRPID [%s]\n\r", (char*)arg);
		wifi_set_group_id(grp_id);
	}
	
	return;
}
#endif

#ifdef CONFIG_PMKSA_CACHING
void fATWPMK(void *arg){

    unsigned char pmk_enable = 0,error = 0;

	if(!arg)
	{
		error = 1;
	}
	else
	{
		if(1 != atoi((const char *)(arg)))
			pmk_enable = 0;
		else
			pmk_enable = 1;
		
		printf("pmk_enable = %d\r\n",pmk_enable);	
		printf("[ATPM]: _AT_WLAN_SET_PMK [%s]\n\r", (char*)arg);
		wifi_set_pmk_cache_enable(pmk_enable);

	}
	
	if(error)
	{
		printf("[ATPM]error cmd  !!\n\r");
		printf("[ATPM]Usage: ATPM = enable \n\r");
		printf("      *************************************************\n\r");
		printf("      1: enable; 0: disable \r\n");
		printf("      *************************************************\n\r");
	}

}
#endif

#ifdef CONFIG_IEEE80211W
void fATWPMF(void *arg){

	int ret;
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	unsigned char pmf_mode;
	
	printf("[ATMF]: _AT_WLAN_PROTECTED_MANAGEMENT_FRAME_\r\n");

	if (!arg) {
		printf("[ATMF] Usage: ATMF=none/optional/required\r\n");
		ret = RTW_BADARG;
		goto exit;
	} else {
		argc = parse_param(arg, argv);
		if (argc < 2) {
			printf("[ATMF] Usage: ATMF=none/optional/required\r\n");
			ret = RTW_BADARG;
			goto exit;
		}
	}
	
	if (strcmp(argv[1], "none") == 0) {
		pmf_mode = 0;
		ret = wifi_set_pmf(pmf_mode);
		if(ret == 0)
			printf("[ATMF]: set station no management protection\r\n");	
	}
	
	if (strcmp(argv[1], "optional") == 0) {
		pmf_mode = 1;
		ret = wifi_set_pmf(pmf_mode);
		if(ret == 0)
			printf("[ATMF]: set station pmf optional\r\n");
	}

	if (strcmp(argv[1], "required") == 0) {
		pmf_mode = 2;
		ret = wifi_set_pmf(pmf_mode);
		if(ret == 0)
			printf("[ATMF]: set station pmf required\r\n");
	}
	
exit:
#if defined(CONFIG_INIC_CMD_RSP) && CONFIG_INIC_CMD_RSP
	inic_c2h_msg("ATMF", ret, NULL, 0);
#endif
	return;
}
#endif
#endif

#elif ATCMD_VER == ATVER_2 // UART module at command

//ATPA=<ssid>,<pwd>,<chl>,<hidden>[,<max_conn>]
void fATPA(void *arg)
{
	int argc, error_no = 0;
	char *argv[MAX_ARGC] = {0};

#if CONFIG_LWIP_LAYER
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	struct netif * pnetif;
#endif
	int timeout = 20;
	unsigned char hidden_ssid = 0;

	if(!arg){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"\r\n[ATPA] Usage: ATPA=<ssid>,<pwd>,<chl>,<hidden>[,<max_conn>]");
		error_no = 1;
		goto exit;
	}

	argc = parse_param(arg, argv);
	if(argc < 5){
		//at_printf("\r\n[ATPA] ERROR : command format error");
		error_no = 1;
		goto exit;
	}

	if( (wifi_mode_new!=RTW_MODE_AP) && (wifi_mode_new!=RTW_MODE_STA_AP) ){
		//at_printf("\r\n[ATPA] ERROR : wifi mode error");
		error_no = 5;
		goto exit;
	}

	//SSID
	if(argv[1] != NULL){
		ap.ssid.len = strlen((char*)argv[1]);
		if(ap.ssid.len > 32){
			//at_printf("\r\n[ATPA] ERROR : SSID length can't exceed 32");
			error_no = 2;
			goto exit;
		}
		strncpy((char *)ap.ssid.val, (char*)argv[1], sizeof(ap.ssid.val));
	}
	else{
		//at_printf("\r\n[ATPA] ERROR : SSID can't be empty");
		error_no = 2;
		goto exit;
	}

	//PASSWORD
	if(argv[2] != NULL){
		if( (strlen(argv[2]) < 8) || (strlen(argv[2]) > 64)){
			//at_printf("\r\n[ATPA] ERROR : PASSWORD length error");
			error_no = 2;
			goto exit;
		}
		strncpy((char *)password, (char*)argv[2], sizeof(password));
		ap.password = password;
		ap.password_len = strlen((char*)argv[2]);
		ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
	}
	else{
		ap.security_type = RTW_SECURITY_OPEN;
	}

	//CHANNEL
	if(argv[3] != NULL){
		ap.channel = (unsigned char) atoi((const char *)argv[3]);
#if !defined(CONFIG_PLATFORM_8721D)
		if( (ap.channel < 0) || (ap.channel > 11) ){
			//at_printf("\r\n[ATPA] ERROR : channel number error");
			error_no = 2;
			goto exit;
		}
#endif
	}

	//HIDDEN SSID
	if(argv[4] != NULL){
		if( (atoi(argv[4]) != 0) && (atoi(argv[4]) != 1)){
			//at_printf("\r\n[ATPA] ERROR : HIDDEN SSID must be 0 or 1");
			error_no = 2;
			goto exit;
		}
		hidden_ssid = (unsigned char) atoi((const char *)argv[4]);
	}

	//MAX NUMBER OF STATION
	if(argv[5] != NULL){
		unsigned char max_sta = atoi(argv[5]);
		if(wext_set_sta_num(max_sta) != 0){
			error_no = 2;
			goto exit;
		}
	}

#if CONFIG_LWIP_LAYER
	dhcps_deinit();
#endif

	wifi_unreg_event_handler(WIFI_EVENT_DISCONNECT, atcmd_wifi_disconn_hdl);
	wifi_unreg_event_handler(WIFI_EVENT_FOURWAY_HANDSHAKE_DONE, atcmd_wifi_connected_hdl);
	wifi_unreg_event_handler(WIFI_EVENT_CONNECT, atcmd_wifi_connected_hdl);

#if (defined(CONFIG_PLATFORM_8710C)||defined(CONFIG_PLATFORM_8721D)) && (defined(CONFIG_BT) && CONFIG_BT)
	if (wifi_set_mode(wifi_mode_new) < 0){
	    //at_printf("\r\n[ATPA] ERROR : Wifi on failed");
	    error_no = 3;
	    goto exit;
	}
#else
	wifi_off();
	vTaskDelay(20);

	if (wifi_on(wifi_mode_new) < 0){
		//at_printf("\r\n[ATPA] ERROR : Wifi on failed");
		error_no = 3;
		goto exit;
	}
#endif

	if(hidden_ssid){
		if(wifi_start_ap_with_hidden_ssid((char*)ap.ssid.val, ap.security_type, (char*)ap.password, ap.ssid.len, ap.password_len, ap.channel) < 0) {
			//at_printf("\r\n[ATPA] ERROR : Start AP failed");
			error_no = 4;
			goto exit;
		}
	}
	else{
		if(wifi_start_ap((char*)ap.ssid.val, ap.security_type, (char*)ap.password, ap.ssid.len, ap.password_len, ap.channel) < 0) {
			//at_printf("\r\n[ATPA] ERROR : Start AP failed");
			error_no = 4;
			goto exit;
		}
	}

	while(1) {
		char essid[33];
		if(wifi_mode_new == RTW_MODE_AP ){
			if(wext_get_ssid( WLAN0_NAME , (unsigned char *) essid) > 0) {
				if(strcmp((const char *) essid, (const char *)ap.ssid.val) == 0) {
					break;
				}
			}
		}
		else if(wifi_mode_new == RTW_MODE_STA_AP ){
			if(wext_get_ssid( WLAN1_NAME , (unsigned char *) essid) > 0) {
				if(strcmp((const char *) essid, (const char *)ap.ssid.val) == 0) {
					break;
				}
			}
		}

		if(timeout == 0) {
			//at_printf("\r\n[ATPA] ERROR : Start AP timeout");
			error_no = 4;
			break;
		}

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout --;
	}
#if CONFIG_LWIP_LAYER
	if(wifi_mode_new == RTW_MODE_STA_AP)
		pnetif = &xnetif[1];
	else
		pnetif = &xnetif[0];

	LwIP_UseStaticIP(pnetif);

	if(dhcp_mode_ap == 1)
		dhcps_init(pnetif);
#endif

exit:
	init_wifi_struct();

	if(error_no == 0)
		at_printf("%sOK\r\n", "+WLSOFTAP:");
	else
		at_printf("%sERROR:%d\r\n", "+WLSOFTAP:", error_no);
	ATCMD_NEWLINE_HASHTAG();
	return;
}

/*find ap with "ssid" from scan list*/
static int _find_ap_from_scan_buf(char*buf, int buflen, char *target_ssid, void *user_data)
{
	rtw_wifi_setting_t *pwifi = (rtw_wifi_setting_t *)user_data;
	int plen = 0;
	
	while(plen < buflen){
		u8 len, ssid_len, security_mode;
		char *ssid;

		// len offset = 0
		len = (int)*(buf + plen);
		// check end
		if(len == 0) break;
		// ssid offset = 14
		ssid_len = len - 14;
		ssid = buf + plen + 14 ;
		if((ssid_len == strlen(target_ssid))
			&& (!memcmp(ssid, target_ssid, ssid_len)))
		{
			strncpy((char*)pwifi->ssid, target_ssid, 33);
			// channel offset = 13
			pwifi->channel = *(buf + plen + 13);
			// security_mode offset = 11
			security_mode = (u8)*(buf + plen + 11);
			if(security_mode == RTW_ENCODE_ALG_NONE)
				pwifi->security_type = RTW_SECURITY_OPEN;
			else if(security_mode == RTW_ENCODE_ALG_WEP)
				pwifi->security_type = RTW_SECURITY_WEP_PSK;
			else if(security_mode == RTW_ENCODE_ALG_CCMP)
				pwifi->security_type = RTW_SECURITY_WPA2_AES_PSK;
		}
		plen += len;
	}
	return 0;
}

/*get ap security mode from scan list*/
static int _get_ap_security_mode(IN char * ssid, OUT rtw_security_t *security_mode, OUT u8 * channel)
{
	rtw_wifi_setting_t wifi;
	u32 scan_buflen = 1000;

	memset(&wifi, 0, sizeof(wifi));

	if(wifi_scan_networks_with_ssid(_find_ap_from_scan_buf, (void*)&wifi, scan_buflen, ssid, strlen(ssid)) != RTW_SUCCESS){
		printf("Wifi scan failed!\n");
		return 0;
	}

	if(strcmp(wifi.ssid, ssid) == 0){
		*security_mode = wifi.security_type;
		*channel = wifi.channel;
		return 1;
	}
	
	return 0;
}

//ATPN=<ssid>,<pwd>[,<key_id>,<bssid>]
static void atcmd_wifi_disconn_hdl( char* buf, int buf_len, int flags, void* userdata)
{
	at_printf("%sOK\r\n", "+WLDISCONN:");
	ATCMD_NEWLINE_HASHTAG();
}

static void atcmd_wifi_connected_hdl( char* buf, int buf_len, int flags, void* userdata)
{
	at_printf("%sOK\r\n", "+WLCONN:");
	ATCMD_NEWLINE_HASHTAG();
}

void fATPN(void *arg)
{
	int argc, error_no = 0;
	int i,j;
	char *argv[MAX_ARGC] = {0};

	int mode, ret;
	unsigned long tick1 = xTaskGetTickCount();
	unsigned long tick2, tick3;
	char empty_bssid[6] = {0}, assoc_by_bssid = 0;
	u8 connect_channel;
	u8 pscan_config;

	if(!arg){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"\r\n[ATPN] Usage : ATPN=<ssid>,<pwd>[,<key_id>,<bssid>]");
		error_no = 1;
		goto exit;
	}

	argc = parse_param(arg, argv);
	if( (argc < 2) || (argc > 5) ){
		//at_printf("\r\n[ATPN] ERROR : command format error");
		error_no = 1;
		goto exit;
	}

	if( (wifi_mode_new!=RTW_MODE_STA) && (wifi_mode_new!=RTW_MODE_STA_AP) ){
		//at_printf("\r\n[ATPN] ERROR : wifi mode error");
		error_no = 5;
		goto exit;
	}

	//SSID
	if(argv[1] != NULL){
		strncpy((char *)wifi.ssid.val, (char*)argv[1], sizeof(wifi.ssid.val));
		wifi.ssid.len = strlen((char*)argv[1]);
	}else{
		//at_printf("\r\n[ATPN] ERROR : SSID can't be Empty");
		error_no = 2;
		goto exit;
	}
	wifi.security_type = RTW_SECURITY_OPEN;

	//PASSWORD
	if(argv[2] != NULL){
		int pwd_len = strlen(argv[2]);
		if(pwd_len > 64 || (pwd_len < 8 && pwd_len != 5)){
			//at_printf("\r\n[ATPN] ERROR : PASSWORD format error");
			error_no = 2;
			goto exit;
		}
		strncpy((char *)password, (char*)argv[2], sizeof(password));
		wifi.password = password;
		wifi.password_len = strlen((char*)argv[2]);
		wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
	}

	//KEYID
	if(argv[3] != NULL){
		if((strlen((const char *)argv[3]) != 1 ) || (*(char*)argv[3] <'0' ||*(char*)argv[3] >'3')) {
			//at_printf("\r\n[ATPN] ERROR : Wrong WEP key id. Must be one of 0,1,2, or 3");
			error_no = 2;
			goto exit;
		}
		wifi.key_id = atoi((const char *)(argv[3]));
		wifi.security_type = RTW_SECURITY_WEP_PSK;
	}

	//BSSID
	if(argv[4] != NULL){
		if(strlen(argv[4]) != 12){
			//at_printf("\r\n[ATPN] ERROR : BSSID format error");
			error_no = 2;
			goto exit;
		}
		for (i=0, j=0; i<ETH_ALEN; i++, j+=2){
			wifi.bssid.octet[i] = key_2char2num(argv[4][j], argv[4][j+1]);
			assoc_by_bssid = 1;
		}
	}

	//Check if in AP mode
	wext_get_mode(WLAN0_NAME, &mode);
	if(mode == RTW_MODE_MASTER) {
#if CONFIG_LWIP_LAYER
		dhcps_deinit();
#endif
#if (defined(CONFIG_PLATFORM_8710C)||defined(CONFIG_PLATFORM_8721D)) && (defined(CONFIG_BT) && CONFIG_BT)
		if (wifi_set_mode(RTW_MODE_STA) < 0){
		    //at_printf("\r\n[ATPN] ERROR: Wifi on failed");
		    error_no = 3;
		    goto exit;
		}
#else
		wifi_off();
		vTaskDelay(20);
		if (wifi_on(RTW_MODE_STA) < 0){
			//at_printf("\r\n[ATPN] ERROR: Wifi on failed");
			error_no = 3;
			goto exit;
		}
#endif
	}

#if CONFIG_INIC_EN
	/************************************************************
	*    Get security mode from scan list, if it's WEP and key_id isn't set by user,
	*    system will use default key_id = 0
	************************************************************/
	//the keyID may be not set for WEP which may be confued with WPA2 
	if((wifi.security_type == RTW_SECURITY_UNKNOWN)||(wifi.security_type == RTW_SECURITY_WPA2_AES_PSK))
	{
		int security_retry_count = 0;
		while (1) {
			if (_get_ap_security_mode((char*)wifi.ssid.val, &wifi.security_type, &connect_channel))
				break;
			security_retry_count++;
			if(security_retry_count >= 3){
				printf("Can't get AP security mode and channel.\n");
				error_no = 6;
				goto exit;
			}
		}
		if(wifi.security_type == RTW_SECURITY_WEP_PSK || wifi.security_type == RTW_SECURITY_WEP_SHARED)
			wifi.key_id = (wifi.key_id <0 || wifi.key_id >3)?0:wifi.key_id;
	}
	pscan_config = PSCAN_ENABLE;
	if(connect_channel > 0 && connect_channel < 14)
		wifi_set_pscan_chan(&connect_channel, &pscan_config, 1);
#endif

	if (rltk_wlan_channel_switch_announcement_is_enable()) {
        printf("[ATPN]rltk_wlan_channel_switch_announcement_is_enable\r\n");

		if (scan_ap.signal_strength == 0 || scan_ap.SSID.len == 0 || wifi.ssid.len != scan_ap.SSID.len || memcmp(wifi.ssid.val, scan_ap.SSID.val, wifi.ssid.len)) {
			memset(&scan_ap, 0, sizeof(rtw_scan_result_t));
	
			strncpy(scan_ap.SSID.val, wifi.ssid.val, wifi.ssid.len);
			scan_ap.SSID.len = wifi.ssid.len;
			scan_ap.security = wifi.security_type;

			u8 scan_count = 0;
			int scan_timeout = 500;
			while (scan_ap.signal_strength == 0 && scan_count < 3) {
				wifi_scan_networks(app_scan_result_handler, &scan_ap);

				scan_ap.band = 0;
				while (scan_ap.band == 0 && scan_timeout != 0) {
					vTaskDelay(10);
					scan_timeout--;
				}
				scan_count++;
				scan_timeout = 500;
			}
		}

		if (scan_ap.signal_strength == 0 || scan_ap.signal_strength <= -80) {
			memset(&scan_ap, 0, sizeof(rtw_scan_result_t));
			error_no = 6;
			goto exit;
		}
		
		connect_channel = scan_ap.channel;
		wifi.security_type = scan_ap.security;
		memcpy(wifi.bssid.octet, scan_ap.BSSID.octet, 6);

		if(wifi.security_type == RTW_SECURITY_WEP_PSK || wifi.security_type == RTW_SECURITY_WEP_SHARED) {
			wifi.key_id = (wifi.key_id <0 || wifi.key_id > 3) ? 0 : wifi.key_id;
		}
		assoc_by_bssid = 1;

		if(wifi_mode == RTW_MODE_STA_AP) {
			//disable wlan1 issue_deauth when channel switched by wlan0
			ret = wifi_set_ch_deauth(0);
			if(ret != 0){
				printf("wifi_set_ch_deauth FAIL\n");
				error_no = 3;
				goto exit;
			}
			//softap switch chl and inform
			if(connect_channel > 0 && connect_channel <= 165) { 
				if(wifi_ap_switch_chl_and_inform(connect_channel)!=RTW_SUCCESS) {
					printf("wifi_ap_switch_chl_and_inform FAIL\n");
				}
			}
		}

		pscan_config = PSCAN_ENABLE;
		if(connect_channel > 0 && connect_channel <= 165) {
			wifi_set_pscan_chan(&connect_channel, &pscan_config, 1);
		}
	}

	if(wifi_mode == RTW_MODE_STA_AP) {
		ret = wifi_set_ch_deauth(0);
		if(ret != 0){
			printf("wifi_set_ch_deauth FAIL\n");
			error_no = 3;
			goto exit;
		}
	}

	wifi_unreg_event_handler(WIFI_EVENT_DISCONNECT, atcmd_wifi_disconn_hdl);
	wifi_unreg_event_handler(WIFI_EVENT_FOURWAY_HANDSHAKE_DONE, atcmd_wifi_connected_hdl);
	wifi_unreg_event_handler(WIFI_EVENT_CONNECT, atcmd_wifi_connected_hdl);

	if(assoc_by_bssid){
		ret = wifi_connect_bssid(wifi.bssid.octet, (char*)wifi.ssid.val, wifi.security_type, (char*)wifi.password,
						ETH_ALEN, wifi.ssid.len, wifi.password_len, wifi.key_id, NULL);
	} else {
		ret = wifi_connect((char*)wifi.ssid.val, wifi.security_type, (char*)wifi.password, wifi.ssid.len,
						wifi.password_len, wifi.key_id, NULL);
	}

	if(ret!= RTW_SUCCESS){
		error_no = 4;
		goto exit;
	}

#if CONFIG_LWIP_LAYER
	if (dhcp_mode_sta == 2){
		struct netif * pnetif = &xnetif[0];
		LwIP_UseStaticIP(pnetif);
		dhcps_init(pnetif);
	}
	else{
		ret = LwIP_DHCP(0, DHCP_START);
		if(ret != DHCP_ADDRESS_ASSIGNED)
			error_no = 7;
	}
#endif


exit:
	if(wifi_mode == RTW_MODE_STA_AP) {
		wifi_set_ch_deauth(1);
	}

	memset(&scan_ap, 0, sizeof(rtw_scan_result_t));
	
	init_wifi_struct();

	if (wifi.security_type == RTW_SECURITY_OPEN) {
		wifi_reg_event_handler(WIFI_EVENT_CONNECT, atcmd_wifi_connected_hdl, NULL);
	} else {
		wifi_reg_event_handler(WIFI_EVENT_FOURWAY_HANDSHAKE_DONE, atcmd_wifi_connected_hdl, NULL);
	}
	
	if(error_no == 0){
		wifi_reg_event_handler(WIFI_EVENT_DISCONNECT, atcmd_wifi_disconn_hdl, NULL);
		at_printf("%sOK\r\n", "+WLCONN:");
	}
	else
		at_printf("%sERROR:%d\r\n", "+WLCONN:", error_no);
	ATCMD_NEWLINE_HASHTAG();
    return;
}

#if ENABLE_SET_AP_MODE
//ATPH=<mode>,<enable>
void fATPH(void *arg)
{
	int argc, error_no = 0;
	char *argv[MAX_ARGC] = {0};
	int mode,enable;

	if(!arg){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"\r\n[ATPH] Usage : ATPH=<mode>,<enable>");
		error_no = 1;
		goto exit;
	}

	argc = parse_param(arg, argv);
	if(argc != 3){
		//at_printf("\r\n[ATPH] ERROR : command format error");
		error_no = 1;
		goto exit;
	}

	if(argv[1] != NULL){
		mode = atoi((const char *)(argv[1]));
		if(mode != 1 && mode != 2){
			//at_printf("\r\n[ATPH] ERROR : parameter must be 1 or 2");
			error_no = 2;
			goto exit;
		}
	}

	if(argv[2] != NULL){
		enable = atoi((const char *)(argv[2]));
		if(enable != 1 && enable != 2){
			//at_printf("\r\n[ATPH] ERROR : parameter must be 1 or 2");
			error_no = 2;
			goto exit;
		}
		if(mode == 1)
			dhcp_mode_ap = enable;
		else if(mode == 2)
			dhcp_mode_sta = enable;
	}

exit:
	if(error_no==0)
		at_printf("%sOK\r\n", "+WLDMOD:");
	else
		at_printf("%sERROR:%d\r\n", "+WLDMOD:", error_no);
	ATCMD_NEWLINE_HASHTAG();
	return;

}
#endif

//ATPE=<ip>(,<gateway>,<mask>)
void fATPE(void *arg)
{
	int argc, error_no = 0;
	char *argv[MAX_ARGC] = {0};
	unsigned int ip_addr = 0;
	//unsigned char sta_ip[4] = {192,168,3,80}, sta_netmask[4] = {255,255,255,0}, sta_gw[4] = {192,168,3,1};

	if(!arg){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"\r\n[ATPE] Usage : ATPE=<ip>(,<gateway>,<mask>)");
		error_no = 1;
		goto exit;
	}

	argc = parse_param(arg, argv);
	if( (argc > 4) || (argc < 2) ){
		//at_printf("\r\n[ATPE] ERROR : command format error");
		error_no = 1;
		goto exit;
	}

	if(argv[1] != NULL){
		ip_addr = inet_addr(argv[1]);
		sta_ip[0] = (unsigned char) ip_addr & 0xff;
		sta_ip[1] = (unsigned char) (ip_addr >> 8) & 0xff;
		sta_ip[2] = (unsigned char) (ip_addr >> 16) & 0xff;
		sta_ip[3] = (unsigned char) (ip_addr >> 24) & 0xff;
	}
	else{
		//at_printf("\r\n[ATPE] ERROR : parameter format error");
		error_no = 2;
		goto exit;
	}

	if(argv[2] != NULL){
		ip_addr = inet_addr(argv[2]);
		sta_gw[0] = (unsigned char) ip_addr & 0xff;
		sta_gw[1] = (unsigned char) (ip_addr >> 8) & 0xff;
		sta_gw[2] = (unsigned char) (ip_addr >> 16) & 0xff;
		sta_gw[3] = (unsigned char) (ip_addr >> 24) & 0xff;
	}
	else{
		sta_gw[0] = sta_ip[0];
		sta_gw[1] = sta_ip[1];
		sta_gw[2] = sta_ip[2];
		sta_gw[3] = 1;
	}

	if(argv[3] != NULL){
		ip_addr = inet_addr(argv[3]);
		sta_netmask[0] = (unsigned char) ip_addr & 0xff;
		sta_netmask[1] = (unsigned char) (ip_addr >> 8) & 0xff;
		sta_netmask[2] = (unsigned char) (ip_addr >> 16) & 0xff;
		sta_netmask[3] = (unsigned char) (ip_addr >> 24) & 0xff;
	}
	else{
		sta_netmask[0] = 255;
		sta_netmask[1] = 255;
		sta_netmask[2] = 255;
		sta_netmask[3] = 0;
	}

exit:
	if(error_no==0)
		at_printf("%sOK\r\n", "+WLSTATICIP:");
	else
		at_printf("%sERROR:%d\r\n", "+WLSTATICIP:", error_no);
	ATCMD_NEWLINE_HASHTAG();
	return;

}

#if ENABLE_SET_AP_MODE
//ATPF=<start_ip>,<end_ip>,<gateway>
void fATPF(void *arg)
{
	int argc, error_no = 0;
	char *argv[MAX_ARGC] = {0};
	unsigned int ip_addr = 0;
	struct ip_addr start_ip, end_ip;

	if(!arg){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"\r\n[ATPF] Usage : ATPF=<start_ip>,<end_ip>,<ip>(,<gateway>,<mask>)");
		error_no = 1;
		goto exit;
	}

	argc = parse_param(arg, argv);
	if( (argc != 4) ){
		//at_printf("\r\n[ATPF] ERROR : command format error");
		error_no = 1;
		goto exit;
	}

	if(argv[1] != NULL){
#if LWIP_VERSION_MAJOR >= 2
		ip_addr_set_ip4_u32(&start_ip, inet_addr(argv[1]));
#else
		start_ip.addr = inet_addr(argv[1]);
#endif
	}
	else{
		//at_printf("\r\n[ATPF] ERROR : parameter format error");
		error_no = 2;
		goto exit;
	}

	if(argv[2] != NULL){
#if LWIP_VERSION_MAJOR >= 2
		ip_addr_set_ip4_u32(&end_ip, inet_addr(argv[2]));
#else
		end_ip.addr = inet_addr(argv[2]);
#endif
	}
	else{
		//at_printf("\r\n[ATPF] ERROR : parameter format error");
		error_no = 2;
		goto exit;
	}

	dhcps_set_addr_pool(1,&start_ip,&end_ip);

	if(argv[3] != NULL){
		ip_addr = inet_addr(argv[3]);
		ap_ip[0] = (unsigned char) ip_addr & 0xff;
		ap_ip[1] = (unsigned char) (ip_addr >> 8) & 0xff;
		ap_ip[2] = (unsigned char) (ip_addr >> 16) & 0xff;
		ap_ip[3] = (unsigned char) (ip_addr >> 24) & 0xff;
	}
	else{
		//at_printf("\r\n[ATPF] ERROR : parameter format error");
		error_no = 2;
		goto exit;
	}

	ap_gw[0] = ap_ip[0];
	ap_gw[1] = ap_ip[1];
	ap_gw[2] = ap_ip[2];
	ap_gw[3] = ap_ip[3];

	ap_netmask[0] = 255;
	ap_netmask[1] = 255;
	ap_netmask[2] = 255;
	ap_netmask[3] = 0;

exit:
	if(error_no==0)
		at_printf("%sOK\r\n", "+WLDRULE:");
	else
		at_printf("%sERROR:%d\r\n", "+WLDRULE:", error_no);
	ATCMD_NEWLINE_HASHTAG();
	return;
}
#endif

int atcmd_wifi_read_info_from_flash(u8 *read_data, u32 read_len)
{
	atcmd_update_partition_info(AT_PARTITION_WIFI, AT_PARTITION_READ, read_data, read_len);
	return 0;
}

void atcmd_wifi_write_info_to_flash(rtw_wifi_setting_t *setting, int enable)
{
	struct atcmd_wifi_conf *data_to_flash;
	rtw_wifi_setting_t *old_setting;

	flash_t flash;
	u32 channel = 0, i, write_needed = 0;
	u8 index = 0;
	u32 data;

	data_to_flash = (struct atcmd_wifi_conf *)malloc(sizeof(struct atcmd_wifi_conf));
	
	if(data_to_flash) {	
		if(enable){
			memset((u8 *)data_to_flash, 0, sizeof(struct atcmd_wifi_conf));
			atcmd_update_partition_info(AT_PARTITION_WIFI, AT_PARTITION_READ, (u8 *)data_to_flash, sizeof(struct atcmd_wifi_conf));
			old_setting = &(data_to_flash->setting);
			if(memcmp((u8 *)old_setting, setting, sizeof(rtw_wifi_setting_t))){
				memcpy(old_setting, setting, sizeof(rtw_wifi_setting_t));
				write_needed = 1;
			}
			if(setting->mode == RTW_MODE_STA || setting->mode == RTW_MODE_STA_AP){
				struct wlan_fast_reconnect reconn;
				int found = 0;
				/*clean wifi ssid,key and bssid*/
				memset((u8 *)&reconn, 0, sizeof(struct wlan_fast_reconnect));

				channel = setting->channel;

				memset(psk_essid[index], 0, sizeof(psk_essid[index]));
				strncpy(psk_essid[index], setting->ssid, sizeof(psk_essid[index]));
				switch(setting->security_type){
					case RTW_SECURITY_OPEN:
						memset(psk_passphrase[index], 0, sizeof(psk_passphrase[index]));
						memset(wpa_global_PSK[index], 0, sizeof(wpa_global_PSK[index]));
						reconn.security_type = RTW_SECURITY_OPEN;
						break;
					case RTW_SECURITY_WEP_PSK:
						channel |= (setting->key_idx) << 28;
						memset(psk_passphrase[index], 0, sizeof(psk_passphrase[index]));
						memset(wpa_global_PSK[index], 0, sizeof(wpa_global_PSK[index]));
						memcpy(psk_passphrase[index], setting->password, sizeof(psk_passphrase[index]));
						reconn.security_type = RTW_SECURITY_WEP_PSK;
						break;
					case RTW_SECURITY_WPA_TKIP_PSK:
						reconn.security_type = RTW_SECURITY_WPA_TKIP_PSK;
						break;
					case RTW_SECURITY_WPA2_AES_PSK:
						reconn.security_type = RTW_SECURITY_WPA2_AES_PSK;
						break;
					default:
						break;
				}

				memcpy(reconn.psk_essid, psk_essid[index], sizeof(reconn.psk_essid));
				if (strlen(psk_passphrase64) == IW_WPA2_PASSPHRASE_MAX_SIZE) {
					memcpy(reconn.psk_passphrase, psk_passphrase64, sizeof(reconn.psk_passphrase));
				} else {
					memcpy(reconn.psk_passphrase, psk_passphrase[index], sizeof(reconn.psk_passphrase));
				}
				memcpy(reconn.wpa_global_PSK, wpa_global_PSK[index], sizeof(reconn.wpa_global_PSK));
				memcpy(&(reconn.channel), &channel, 4);

				if(data_to_flash->reconn_num < 0 || data_to_flash->reconn_num > ATCMD_WIFI_CONN_STORE_MAX_NUM ||
					data_to_flash->reconn_last_index < 0 || data_to_flash->reconn_last_index > ATCMD_WIFI_CONN_STORE_MAX_NUM
				){
					data_to_flash->reconn_num = 0;
					data_to_flash->reconn_last_index = -1;
				}

				reconn.enable = enable;
				for(i = 0; i < data_to_flash->reconn_num; i++){
					if(memcmp((u8 *)&reconn, (u8 *)&(data_to_flash->reconn[i]), sizeof(struct wlan_fast_reconnect)) == 0) {
						AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ALWAYS,
							"the same profile found in flash");
						found = 1;
						break;
					}
				}
				if(!found){
					data_to_flash->reconn_last_index++;
					if(data_to_flash->reconn_last_index >= ATCMD_WIFI_CONN_STORE_MAX_NUM)
						data_to_flash->reconn_last_index -= ATCMD_WIFI_CONN_STORE_MAX_NUM;
					memcpy((u8 *)&data_to_flash->reconn[data_to_flash->reconn_last_index], (u8 *)&reconn, sizeof(struct wlan_fast_reconnect));
					data_to_flash->reconn_num++;
					if(data_to_flash->reconn_num > ATCMD_WIFI_CONN_STORE_MAX_NUM)
						data_to_flash->reconn_num = ATCMD_WIFI_CONN_STORE_MAX_NUM;
					write_needed = 1;
				}
			}
			if(write_needed || data_to_flash->auto_enable != enable){
				data_to_flash->auto_enable = enable;
				atcmd_update_partition_info(AT_PARTITION_WIFI, AT_PARTITION_WRITE, (u8 *)data_to_flash, sizeof(struct atcmd_wifi_conf));
			}
		}else{
			atcmd_update_partition_info(AT_PARTITION_WIFI, AT_PARTITION_ERASE, (u8 *)data_to_flash, sizeof(struct atcmd_wifi_conf));
		}
	}
	if(data_to_flash) {
		free(data_to_flash);
	}
}

int atcmd_wifi_restore_from_flash(void)
{
	flash_t		flash;
	struct atcmd_wifi_conf *data;
	rtw_wifi_setting_t *setting;
	struct wlan_fast_reconnect *reconn;
	uint32_t	channel;
	uint32_t    security_type;
	uint8_t     pscan_config;
	char key_id[2] = {0};
	int ret = -1, i;
	int mode;
	rtw_network_info_t wifi = {
		{0},    // ssid
		{0},    // bssid
		0,      // security
		NULL,   // password
		0,      // password len
		-1      // key id
	};

	data = (struct atcmd_wifi_conf *)rtw_zmalloc(sizeof(struct atcmd_wifi_conf));
	if(data){
		atcmd_update_partition_info(AT_PARTITION_WIFI, AT_PARTITION_READ, (u8 *)data, sizeof(struct atcmd_wifi_conf));
		if(data->auto_enable != 1)
			goto exit;
		setting = &data->setting;
		if(setting->mode == RTW_MODE_AP || setting->mode == RTW_MODE_STA_AP){
			//start AP here
			goto exit;
		}
		
		//Check if in AP mode
		wext_get_mode(WLAN0_NAME, &mode);
		if(mode == RTW_MODE_MASTER) {
#if CONFIG_LWIP_LAYER
			dhcps_deinit();
#endif
			wifi_off();
			vTaskDelay(20);
			if (wifi_on(RTW_MODE_STA) < 0){
				printf("\n\rERROR: Wifi on failed!");
				ret = -1;
				goto exit;
			}
		}
		
#if CONFIG_AUTO_RECONNECT
		//setup reconnection flag
		wifi_set_autoreconnect(2);
#endif
		if (data->reconn[data->reconn_last_index].security_type == RTW_SECURITY_OPEN) {
			wifi_reg_event_handler(WIFI_EVENT_CONNECT, atcmd_wifi_connected_hdl, NULL);
		} else {
			wifi_reg_event_handler(WIFI_EVENT_FOURWAY_HANDSHAKE_DONE, atcmd_wifi_connected_hdl, NULL);
		}

		int last_index = data->reconn_last_index;
		for(i = 0; i < data->reconn_num; i++){
			reconn = &data->reconn[last_index];
			last_index ++;
			if(last_index >= ATCMD_WIFI_CONN_STORE_MAX_NUM)
				last_index -= ATCMD_WIFI_CONN_STORE_MAX_NUM;
			if(reconn->enable != 1){
				continue;
			}
			memcpy(psk_essid, reconn->psk_essid, sizeof(reconn->psk_essid));
			memcpy(psk_passphrase, reconn->psk_passphrase, sizeof(reconn->psk_passphrase));
			memcpy(wpa_global_PSK, reconn->wpa_global_PSK, sizeof(reconn->wpa_global_PSK));
			channel = reconn->channel;
			snprintf(key_id, sizeof(key_id),"%d",(char) (channel>>28));
			channel &= 0xff;
			security_type = reconn->security_type;
			pscan_config = PSCAN_ENABLE | PSCAN_FAST_SURVEY;
			//set partial scan for entering to listen beacon quickly
			wifi_set_pscan_chan((uint8_t *)&channel, &pscan_config, 1);

			wifi.security_type = security_type;
			//SSID
			strncpy((char *)wifi.ssid.val, (char*)psk_essid, sizeof(wifi.ssid.val));
			wifi.ssid.len = strlen((char*)psk_essid);

			switch(security_type){
				case RTW_SECURITY_WEP_PSK:
					wifi.password = (unsigned char*) psk_passphrase;
					wifi.password_len = strlen((char*)psk_passphrase);
					wifi.key_id = atoi((const char *)key_id);
					break;
				case RTW_SECURITY_WPA_TKIP_PSK:
				case RTW_SECURITY_WPA2_AES_PSK:
					wifi.password = (unsigned char*) psk_passphrase;
					wifi.password_len = strlen((char*)psk_passphrase);
					break;
				default:
					break;
			}

			ret = wifi_connect((char*)wifi.ssid.val, wifi.security_type, (char*)wifi.password, wifi.ssid.len,
				wifi.password_len, wifi.key_id, NULL);
			if(ret == RTW_SUCCESS){
				LwIP_DHCP(0, DHCP_START);
				ret = 0;
				break;
			}
		}
	}

exit:
	if(ret == 0)
		wifi_reg_event_handler(WIFI_EVENT_DISCONNECT, atcmd_wifi_disconn_hdl, NULL);
	if(data)
		rtw_mfree((u8 *)data, sizeof(struct wlan_fast_reconnect));
	return ret;
}

//ATPG=<enable>
void fATPG(void *arg)
{
	int argc, error_no = 0;
	char *argv[MAX_ARGC] = {0};
//	flash_t flash;
//	struct wlan_fast_reconnect read_data = {0};

	if(!arg){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"\r\n[ATPG] Usage : ATPG=<enable>");
		error_no = 1;
		goto exit;
	}
	argc = parse_param(arg, argv);
	if(argc != 2){
		//at_printf("\r\n[ATPG] ERROR : command format error");
		error_no = 1;
		goto exit;
	}

	//ENABLE FAST CONNECT
	if(argv[1] != NULL){
		if('?' == argv[1][0]) {
			struct atcmd_wifi_conf *data = (struct atcmd_wifi_conf *)rtw_zmalloc(sizeof(struct atcmd_wifi_conf));
			if(data){
				atcmd_update_partition_info(AT_PARTITION_WIFI, AT_PARTITION_READ, (u8 *)data, sizeof(struct atcmd_wifi_conf));
			} else {
				error_no = 3;
				goto exit;
			}
			at_printf("%s%d\r\n", "+WLAUTOCONN:", data->auto_enable == 1 ? 1 : 0);
			ATCMD_NEWLINE_HASHTAG();
			rtw_free(data);
			return;
	    }
#if 0
		device_mutex_lock(RT_DEV_LOCK_FLASH);
		flash_stream_read(&flash, FAST_RECONNECT_DATA, sizeof(struct wlan_fast_reconnect), (u8 *) &read_data);
		read_data.enable = atoi((const char *)(argv[1]));
		if(read_data.enable != 0 && read_data.enable != 1){
			//at_printf("\r\n[ATPG] ERROR : parameter must be 0 or 1");
			error_no = 2;
			device_mutex_unlock(RT_DEV_LOCK_FLASH);
			goto exit;
		}
		flash_erase_sector(&flash, FAST_RECONNECT_DATA);
		flash_stream_write(&flash, FAST_RECONNECT_DATA, sizeof(struct wlan_fast_reconnect), (u8 *) &read_data);
		device_mutex_unlock(RT_DEV_LOCK_FLASH);
#else
		rtw_wifi_setting_t setting;
		int enable = atoi((const char *)(argv[1]));
		if(enable != 0 && enable != 1){
			error_no = 2;
			goto exit;
		}
		if(enable == 1){
			u8 *ifname[1] = {WLAN0_NAME};
			if(wifi_get_setting((const char*)ifname[0],&setting)){
				AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
					"wifi_get_setting fail");
				error_no = 3;
				goto exit;
			}
		}
		atcmd_wifi_write_info_to_flash(&setting, enable);
#endif
	}

exit:
	if(error_no==0)
		at_printf("%sOK\r\n", "+WLAUTOCONN:");
	else
		at_printf("%sERROR:%d\r\n", "+WLAUTOCONN:", error_no);
	ATCMD_NEWLINE_HASHTAG();
	return;
}

//ATPG=<enable>
void fATRC(void *arg)
{
	int error_no = 0;
	int argc = 0, mode = 0;
	char *argv[MAX_ARGC] = {0};

	if (arg == NULL) {
		printf("[+WLRECONN] Invalid parameter\r\n");
		error_no = 1;
		goto end;
	}

	argc = parse_param(arg, argv);
	if (argc != 2 || argv[1] == NULL) {
		printf("[+WLRECONN] Invalid parameter number\r\n");
		error_no = 1;
		goto end;
	}

	mode = atoi(argv[1]);
	if (mode == 0) {
		printf("[+WLRECONN] Disable autoreconnect\r\n");
		wifi_set_autoreconnect(0);
	} else if (mode == 1) {
		printf("[+WLRECONN] Enable autoreconnect with default times\r\n");
		wifi_set_autoreconnect(1);
	} else if (mode == 2){
		printf("[+WLRECONN] Enable autoreconnect with infinate times\r\n");
		wifi_set_autoreconnect(2);
	} else {
		error_no = 2;
	}
	

end:
	if (error_no == 0) {
		at_printf("%sOK\r\n", "+WLRECONN:");
	} else {
		at_printf("%sERROR:%d\r\n", "+WLRECONN:", error_no);
	}
	ATCMD_NEWLINE_HASHTAG();
}

//ATPM=<mac>
void fATPM(void *arg)
{
	int argc, error_no = 0;
	char *argv[MAX_ARGC] = {0};

	if(!arg){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"\r\n[ATPM] Usage : ATPM=<mac>");
		error_no = 1;
		goto exit;
	}

	argc = parse_param(arg, argv);
	if(argc != 2){
		//at_printf("\r\n[ATPM] ERROR : command format error");
		error_no = 1;
		goto exit;
	}

	if(argv[1] != NULL){
		if(strlen(argv[1]) != 12){
			//at_printf("\r\n[ATPM] ERROR : BSSID format error");
			error_no = 2;
			goto exit;
		}
		wifi_set_mac_address(argv[1]);
	}

exit:
	if(error_no==0)
		at_printf("%sOK\r\n", "+WLMAC:");
	else
		at_printf("%sERROR:%d\r\n","+WLMAC:",error_no);
	ATCMD_NEWLINE_HASHTAG();
	return;

}

#if ENABLE_SET_AP_MODE
//ATPW=<mode>
void fATPW(void *arg)
{
	int argc, error_no = 0;
	char *argv[MAX_ARGC] = {0};

	if(!arg){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"\r\n[ATPW] Usage : ATPW=<mode>");
		error_no = 1;
		goto exit;
	}

	argc = parse_param(arg, argv);
	if(argc != 2){
		//at_printf("\r\n[ATPW] ERROR : command format error");
		error_no = 1;
		goto exit;
	}

	if(argv[1] != NULL){
	        if('?' == argv[1][0]){
	            at_printf("%s%d\r\n", "+WLMODE:", wifi_mode_new);
	            goto exit;
	        }
	        else{
        		wifi_mode_new = atoi((const char *)(argv[1]));
        		if((wifi_mode_new!=RTW_MODE_STA) && (wifi_mode_new!=RTW_MODE_AP) && (wifi_mode_new!=RTW_MODE_STA_AP) ){
        			//at_printf("\r\n[ATPW] ERROR : parameter must be 1 , 2 or 3");
        			error_no = 2;
					goto exit;
        		}
				wifi_set_mode(wifi_mode_new);
		}
	}

exit:
	if(error_no==0)
		at_printf("%sOK\r\n", "+WLMODE:");
	else
		at_printf("%sERROR:%d\r\n","+WLMODE:",error_no);
	ATCMD_NEWLINE_HASHTAG();
	return;
}
#endif

extern char *http_post_header(char *host, char *resource, char *type, int data_len);
 extern char *http_get_header(char *host, char *resource);

 struct hostent *http_host;
 char *http_resourse;
 char *http_content;
 int  http_type;
 int http_port;
 char *host ;
 char *post_data;
 char* port_ssl;
 int ssl_verify;

 void http_client_ver2(void)
 {
	// char *message_fmt = "POST / HTTP/1.0\r\n\r\n"; 
	 int sockfd, bytes;
	 char message[512],*response;
	 int error_no = 0;
     struct sockaddr_in serv_addr;

	memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(http_port);
    memcpy(&serv_addr.sin_addr.s_addr,http_host->h_addr,4);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	 
	 if(sockfd < 0)
	 	{

		printf("[ERROR] http client Create socket failed\n");
		 error_no = 20;
		goto exit;
	 
	 	}
	 
	 if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
		 printf("[ERROR] http client connect failed\n");
		 error_no = 21;
		goto exit;
	 }

	// printf("http_type is %d ",http_type);
 if  (http_type == 2)
	
{
	
	 //send get request
	sprintf(message,"%s",http_get_header(host,http_resourse));
	printf("\nGET request header:\n%s",message);
	 bytes = write(sockfd,message,256);
	 if (bytes < 0){
		 printf("[ERROR] http client send get packet failed\n");
	     error_no = 21;
		goto exit;}
	 
		}
 
    //set post request 
   

 else if(http_type ==3 )
 	{

	sprintf(message,"%s",http_post_header(host, http_resourse,http_content ,strlen(post_data)));
	printf("\nPOST request header:\n%s",message);
	bytes = write(sockfd,message,1024);
	
	
		if (bytes < 0)
			{printf("[ERROR] http client send post packet failed\n");
		    error_no = 22;
		    goto exit;}

			write(sockfd,post_data,strlen(post_data));
			printf("\nRequest body :\n%s\n",post_data);
			

	    }

	else
		{
          printf("\n\rhttp_type set err\n\r");
		  goto exit;
	
	   }

 	
	 //receive response
     fd_set read_fds;
     struct timeval timeout;
     timeout.tv_sec = 3;
     timeout.tv_usec = 0;
     FD_ZERO(&read_fds);
     FD_SET(sockfd, &read_fds);
	
	 response = malloc(1500+1);
	 if(response ==NULL)
		 printf("[ERROR] malloc failed\n");
	 printf("Response:\n");
	 do {
		 memset(response,0,1500+1);
         if(select(sockfd+1, &read_fds, NULL, NULL, &timeout)) {
            bytes = read(sockfd,response,1500);
            if (bytes < 0) {
                printf("[ERROR] http client receive packet failed\n");
                break;
            }
            if (bytes == 0) {
                printf("TCP connection is closed\n");
                break;
            }
            at_printf("%s",response);
         }
         else {
            printf("Timeout! No data \n");
            break;
         }
		 
	 } while (bytes > 0);
	 
 exit:
     if(error_no != 0)
     {
        at_printf("\r\n%sERROR:%d\r\n", "+HTTPCLIENT:", error_no);
     }
 	 
	 ATCMD_NEWLINE_HASHTAG();
	 //close the socket
	  free(response);
	  close(sockfd);

 return;
 }

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/version.h"
#include "mbedtls/config.h"
#include "mbedtls/certs.h"


 static void my_debug(void *ctx, int level, const char *file, int line, const char *str)
{
	/* To avoid gcc warnings */
	( void ) ctx;
	( void ) level;
	
	//printf("\n\r%s:%d: %s\n\r", file, line, str);
    printf("%s", str);
}

extern int rtw_get_random_bytes(void* dst, u32 size);
static int my_random(void *p_rng, unsigned char *output, size_t output_len)
{
	/* To avoid gcc warnings */
	( void ) p_rng;
	
	rtw_get_random_bytes(output, output_len);
	return 0;
}

static void* my_calloc(size_t nelements, size_t elementSize)
{
	size_t size;
	void *ptr = NULL;

	size = nelements * elementSize;
	ptr = pvPortMalloc(size);

	if(ptr)
		memset(ptr, 0, size);
 return ptr;
}

#define STACKSIZE     1150
#define SERVER_PORT   "443"
#define DEBUG_LEVEL   0

#define my_free		vPortFree


static  unsigned char test_client_key[2048] =  \
	"-----BEGIN PRIVATE KEY-----\r\n" \
	"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCsTIOSoYt6mO+C\r\n" \
	"PE4Lg1dk8E6k18k4yyRrt72l6PU4da6qCBUiuVE/GY76BzoUhR2I7aHjUiY7g5dj\r\n" \
	"a/SdNzsdC0jxBax782DcZ/Z2keawq2SrxsxIs65d0d6mmqiua11lO4uvT2LD5T5W\r\n" \
	"HGwJV/pRYsBmc0Nmtm65gSLspwm5ydgAjP5ZcfGrAwvL8vyX0sbDlBKhsZO4nMv6\r\n" \
	"I17Kfn6RFaXYeuf7gXu476QAqxkTFgOJ4e+wp9XkH98atdB8wrKk3CXp0Q5VLV+M\r\n" \
	"l6CS77DNuS9cS5ffVUwemoJNpOg8Ew97Ys6cDRyE6AINUMTXuyicamDM8mkbFOZw\r\n" \
	"RBdkkMIpAgMBAAECggEAAqQPljVEqoXikefzuEh0k/b89ZjbHDQ85CvGS980tv9S\r\n" \
	"xIdoL4aQoLt3Dg0DTmI18C3YP+A39syPCvaSIsDAdrMAFLWmDaZO00ICrJGS7B1M\r\n" \
	"ZzeeVPTwFL8NPxd6w2Wg4lbVzsOUO3VtjatU2pXPH+U4FrnojA/6xXwduWvcAc14\r\n" \
	"QVlbnDhd4sCLlm+3aNwUH0wY/DgEmeqYaCrIsAvKaayovDIkMAHprvDPR4MONv7r\r\n" \
	"whmxi1m7SS8jRRv7Ueci7wylB1Oa/oEoqdG9hJSKb5JKKJ9b4O5qO193ahVrlOr/\r\n" \
	"f1CBD1xuuYZJdyThQEq3g8QVZ0ejpKQdA/Cgr9avWwKBgQDKlsHB4j990F/nfkTd\r\n" \
	"uP5tRTBxga2Wl+0YV256lxyLbP67Fn+IQAtlCwVQaaToYdc9m7La5ewAhCTxTUa2\r\n" \
	"+KosygpK1DCwT4GnNuSqCie/TQIivx1c42z8k605RRw1RoXvTN9mTodkEV+xnuqu\r\n" \
	"pMt3Qqxk0DpKJTT2b+CR8mFLDwKBgQDZuWgb0xPX2ocFeIugkj/GWP9FzpbUfs3y\r\n" \
	"Y9Yvztk7Rfd3OAKqv6QKe9alkhi8l/rq4PGf7jclzXWGZYRl37oDk9K6V9qlm5zl\r\n" \
	"UmkY25fmQrpuX1/MjwKle3UtHTgfkV+n8FQqBowyiXJmPR/GIZ1MDQGay6YsZW9h\r\n" \
	"RSmjJR//RwKBgG0KG5uJdPb40sW8dsCIM5kIpnxReecWOqzhUjt1Xg3FSo3UHCxv\r\n" \
	"nDJ0bhXL5tCU6M7mIKwlGJNmjKO8HhWZXa7hyAxijRo4kOY5a3YCJUaX4lBlYR5/\r\n" \
	"4kdIDPJIOliKbshQLeXY2pKHr2PgzxvU4xZ7smJImTcTM6B4svya4rcNAoGABkx+\r\n" \
	"3o80u8HuthHOuE6C2dIpUHOxmWQxwRlyUN/DGRvlFOXh7d6teEAs7Y1UeeBLug/X\r\n" \
	"7bCOl+FhzQmS0v+t+fyE1C9GfBQMbfX8ZGO1+Utk6VyJAPoCMZdoxG1b+k3SXqFt\r\n" \
	"TenylGO1k4+hI7JFQrX9neTWYeJtXnsJwVaJnnECgYEAtgLApWfQVyr3yyGh7ur3\r\n" \
	"lYqQI5U6EAH8p8yA3otTeI22BgOvARH57yjK2PDyhFuA2efFX2+ipNAIXmiiIl1K\r\n" \
	"wvu0YpiZAkH8D0dmxECS/GaluoqhA7pK6QjlRS2sFZ9n4C8vet79bFYZQ4fXazKw\r\n" \
	"w4KeMcb6l+GyVQv9CSdB5lw=\r\n" \
	"-----END PRIVATE KEY-----\r\n";

static unsigned char test_client_crt[2048] =  \
	"-----BEGIN CERTIFICATE-----\r\n" \
	"MIIDozCCAosCFG8LuhxSx/IA0zALJmeB5JUV7jmiMA0GCSqGSIb3DQEBCwUAMIGN\r\n" \
	"MQswCQYDVQQGEwJDTjEQMA4GA1UECAwHSmlhbmdTdTEPMA0GA1UEBwwGU3VaaG91\r\n" \
	"MRAwDgYDVQQKDAdSZWFsc2lsMQ8wDQYDVQQLDAZjbGllbnQxFDASBgNVBAMMC2Ns\r\n" \
	"aWVudF90ZXN0MSIwIAYJKoZIhvcNAQkBFhNjbGllbnRfdGVzdEAxMjMuY29tMB4X\r\n" \
	"DTIzMDYwNjA3MTk0NloXDTMzMDYwMzA3MTk0NlowgY0xCzAJBgNVBAYTAkNOMRAw\r\n" \
	"DgYDVQQIDAdKaWFuZ1N1MQ8wDQYDVQQHDAZTdVpob3UxEDAOBgNVBAoMB1JlYWxz\r\n" \
	"aWwxDzANBgNVBAsMBmNsaWVudDEUMBIGA1UEAwwLY2xpZW50X3Rlc3QxIjAgBgkq\r\n" \
	"hkiG9w0BCQEWE2NsaWVudF90ZXN0QDEyMy5jb20wggEiMA0GCSqGSIb3DQEBAQUA\r\n" \
	"A4IBDwAwggEKAoIBAQCsTIOSoYt6mO+CPE4Lg1dk8E6k18k4yyRrt72l6PU4da6q\r\n" \
	"CBUiuVE/GY76BzoUhR2I7aHjUiY7g5dja/SdNzsdC0jxBax782DcZ/Z2keawq2Sr\r\n" \
	"xsxIs65d0d6mmqiua11lO4uvT2LD5T5WHGwJV/pRYsBmc0Nmtm65gSLspwm5ydgA\r\n" \
	"jP5ZcfGrAwvL8vyX0sbDlBKhsZO4nMv6I17Kfn6RFaXYeuf7gXu476QAqxkTFgOJ\r\n" \
	"4e+wp9XkH98atdB8wrKk3CXp0Q5VLV+Ml6CS77DNuS9cS5ffVUwemoJNpOg8Ew97\r\n" \
	"Ys6cDRyE6AINUMTXuyicamDM8mkbFOZwRBdkkMIpAgMBAAEwDQYJKoZIhvcNAQEL\r\n" \
	"BQADggEBAAJYPqTxLqrQeTzQ3C9gvueH/30COVIz8q5bzipUafePbEy46ynpwuDc\r\n" \
	"OMVJGqdXb/orO0SEAacj7AXI49W07HRfU5uSu/OjdTa+CGEkabMF+oYYswrAOq5d\r\n" \
	"JjSXUmAn5VWjqJT4lzIegZENrYsTc1uPSg1mIitpOew4KU8LpnGbXNFiZB/8Y5TN\r\n" \
	"5rd3PD7OklBNLBcDf7/hY5PXPy2a7nrpA6Cd2fHULFfTGVxQIbCW+oDXXTjLGeuT\r\n" \
	"ky9y9L1KOHnIIl1T3kM40gSMWfAGP1bANUYhojB4zK+T2PuzsBon9vgzJylhqoOF\r\n" \
	"l7c6sZbFeWhh2E/rMWbat23ALMH0LI8=\r\n" \
	"-----END CERTIFICATE-----\r\n";
static  unsigned char test_ca_crt[2048] =  \
	"-----BEGIN CERTIFICATE-----\r\n" \
	"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"\
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"\
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"\
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"\
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"\
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"\
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"\
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"\
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"\
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"\
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"\
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"\
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"\
     "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"\
     "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"\
     "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"\
     "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"\
     "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"\
	"-----END CERTIFICATE-----\r\n";



    mbedtls_x509_crt client_ca_x509;
	mbedtls_x509_crt client_x509;
	mbedtls_pk_context client_pk;
	mbedtls_pk_context client_pub_key;


static void  https_client_ver2(void)

{   int ret, len,https_err;
	int retry_count = 0;
	unsigned char buf[512];
	mbedtls_net_context server_fd;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	
	

	#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1) && defined(CONFIG_SSL_CLIENT_PRIVATE_IN_TZ) && (CONFIG_SSL_CLIENT_PRIVATE_IN_TZ == 1)
	rtw_create_secure_context(STACKSIZE*4);
	extern int NS_ENTRY secure_mbedtls_platform_set_calloc_free(void);
	secure_mbedtls_platform_set_calloc_free();
	extern void NS_ENTRY secure_set_ns_device_lock(void (*device_mutex_lock_func)(uint32_t), void (*device_mutex_unlock_func)(uint32_t));
	secure_set_ns_device_lock(device_mutex_lock, device_mutex_unlock);
#endif

	mbedtls_platform_set_calloc_free(my_calloc, my_free);


/*  
 *  Prepare the certificate and key
 */

	if(ssl_verify == 2){

    printf("\n\r set client ssl verify \n\r");
    mbedtls_x509_crt_init(&client_ca_x509);
	mbedtls_x509_crt_init(&client_x509);
	mbedtls_pk_init(&client_pk);
	

	

		if ((ret = mbedtls_x509_crt_parse(&client_x509, (const unsigned char *) test_client_crt, strlen((char const *)test_client_crt) + 1)) != 0) {
			https_err =12;
			printf(" failed\n  ! mbedtls_x509_crt_parse returned %d,http_err is %d\n\n", ret,https_err);
			 goto exit;
		}
	
		if ((ret = mbedtls_x509_crt_parse(&client_ca_x509, (const unsigned char *) test_ca_crt, strlen((char const *)test_ca_crt) + 1)) != 0) {
             https_err =13;
			printf(" failed\n  ! mbedtls_x509_crt_parse returned %d,http_err is %d\n\n", ret,https_err);
			goto exit;
		}
		
	if ((ret = mbedtls_pk_parse_key(&client_pk, (const unsigned char *) test_client_key, strlen((char const *)test_client_key) + 1, NULL, 0)) != 0) {
		https_err =14;
		printf(" failed\n  ! mbedtls_pk_parse_key returned %d,http_err is %d\n\n", ret,https_err);
		 goto exit;

	    }

		
	}


	/*
	 * 1. Start the connection
	 */
	printf("\n\r  . Connecting to tcp/%s/%s...\n\r", host, port_ssl);

	mbedtls_net_init(&server_fd);

	if((ret = mbedtls_net_connect(&server_fd, host, port_ssl, MBEDTLS_NET_PROTO_TCP)) != 0) {
		https_err =14;
		printf(" failed\n\r  ! mbedtls_net_connect returned %d,http_err is %d\n", ret,https_err);
		goto exit;
	}

	printf("  . Connect ok\n\r");

	/*
	 * 2. Setup stuff
	 */
	printf("  . Setting up the SSL/TLS structure...");

	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);
    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

	if((ret = mbedtls_ssl_config_defaults(&conf,
		MBEDTLS_SSL_IS_CLIENT,
		MBEDTLS_SSL_TRANSPORT_STREAM,
		MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        https_err =15;
		printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d,http_err is %d\n", ret,https_err);
		goto exit;
	}


if(ssl_verify == 2){

	    mbedtls_ssl_conf_ca_chain(&conf, &client_ca_x509, NULL);
	    mbedtls_ssl_conf_own_cert(&conf, &client_x509, &client_pk);
		//mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);

			}
 else 
	{
	mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
 }

    mbedtls_ssl_conf_rng(&conf, my_random, NULL);
	
    //mbedtls_debug_set_threshold(3);
    mbedtls_ssl_conf_dbg(&conf, my_debug, NULL);
	
	if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
		https_err =16;
		printf(" failed\n  ! mbedtls_ssl_setup returned %d,http_err is %d\n\n", ret,https_err);
		goto exit;
	}

	printf(" ok\n");

	/*
	 * 3. Handshake
	 */
	printf("  . Performing the SSL/TLS handshake...\n\r");

	while((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
		if((ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE
			&& ret != MBEDTLS_ERR_NET_RECV_FAILED) || retry_count >= 5) {
			
            https_err =16;
			printf(" failed\n\r  ! mbedtls_ssl_handshake returned -0x%x,http_err is %d\n\n", -ret,https_err);
			goto exit;
		}

		retry_count++;
	}

	printf("\n\r  . SSL/TLS handshake ok\n");
	printf("\n\r  . Use ciphersuite %s\n", mbedtls_ssl_get_ciphersuite(&ssl));


	//printf("http_type is %d ",http_type);
	
if	(http_type == 2)
  {
	/*
	 * 4. Write the GET request
	 */
	printf("\n\r  > Write to server:");

	//len = sprintf((char *) buf, GET_REQUEST);
	len = sprintf(buf, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", http_resourse, host);

	while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0) {
		if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
			https_err =17;
			printf(" failed\n\r  ! mbedtls_ssl_write returned %d,http_err is %d\n", ret,https_err);
			goto exit;
		}
	}

	len = ret;
	printf(" %d bytes written\n\n%s", len, (char *) buf);
}


 else if(http_type == 3)
     {
    /*
	 *5. Write the POST request
     */
		printf("\n\r  > Write POST to server:");

        //post_datachar *post_data_1 = "param1=test_data1&param2=test_data2";
       // char *len_str = http_itoa(strlen(post_data));
		len = sprintf(buf, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", http_resourse, host, http_content,strlen(post_data) );
     
		while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0) {
				if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
					https_err =18;
					printf(" failed\n\r  ! mbedtls_ssl_write returned %d,http_err is %d\n\n", ret,https_err);
					goto exit;
				}
			}

		    len = ret;
			printf(" %d bytes header  written\n\n%s", len, (char *) buf);
			
	     while(( ret = mbedtls_ssl_write(&ssl, (unsigned char*)post_data,strlen(post_data ))) <=0){
		  
		         printf(" failed\n\r  ! mbedtls_ssl_write poat_data returned %d\n\r",ret);
				 	
					goto exit;
	     	}
		  printf("write post data to server %s",post_data);
			
}

else {
	printf("\n\rhttp_type set err\n\r");
		  goto exit;
      }

     /*
	 * 6. Read the HTTP response
	 */
	printf("  < Read from server:" );

   int read_size = 0, resource_size = 0;
   len = sizeof(buf) - 1;
   memset(buf, 0, sizeof(buf));

	
		while((read_size = mbedtls_ssl_read(&ssl, buf, len)) > 0) {
			

			printf("\n\r read resource %d bytes\n\r", read_size);
            at_printf("%s", buf);
			resource_size += read_size;
		}

		printf("exit read. ret = -0x%x\n", -read_size);
		

	

	mbedtls_ssl_close_notify(&ssl);

exit:
    mbedtls_net_free(&server_fd);
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
	if(ssl_verify == 2)
		{
        mbedtls_x509_crt_free(&client_ca_x509);
		mbedtls_x509_crt_free(&client_x509);
		mbedtls_pk_free(&client_pk);

	}



}
 static void https_client_thread_ver2(void *param)
 {
	 /* To avoid gcc warnings */
	 ( void ) param;
	
	 https_client_ver2();
	 vTaskDelete(NULL);
 }


 
 static void http_client_thread_ver2(void *param)
 {
	 /* To avoid gcc warnings */
	( void ) param;
	 
	 http_client_ver2();
	 vTaskDelete(NULL);
 }

/* http/https get/post example   */
void fATPb(void *arg)
{
 int argc;
 char *argv[MAX_ARGC] = {0};
 int error_no = 0;
  int transport_type;

 argc = parse_param(arg, argv);
 
	if((!arg) || (argc != 9)){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"[+HTTPCLIENT] Usage: AT+HTTPCLIENT=<HTTP:1/HTTPS:2><host>,<port>,<GET:2/POST:3>,<path>,<ca:1:N/2:Y>,<content-type>,<data>\n\r "\
			"          eg:AT+HTTPCLIENT=2,httpbin.org,443,3,/post,2,application/json,param1=test_data1&param2=test_data2 \n\r   "\
			"             AT+HTTPCLIENT=1,httpbin.org,80,2,/get?param1=test_data1&param2=test_data2,0,0,0 \n\r "
			);
		error_no = 1;
		goto err_exit;
		}

	transport_type=atoi((char*)argv[1]);
    if((transport_type <= 0) || (transport_type > 2))
		{
    printf("\n\r set transport type failed \n\r");
	error_no = 2;
	goto err_exit;
	}

	
	http_port = atoi((char*)argv[3]);
	if((http_port <= 0) || (http_port > 65535))
		{
    printf("\n\r set http port failed \n\r");
	error_no = 3;
	goto err_exit;
	}
	port_ssl = argv[3];
    http_host = gethostbyname(argv[2]);
	
		if(http_host == NULL) {
	 printf("\n\r set http host failed \n\r");
	error_no = 4;
	goto err_exit;
    	}
	
	host=argv[2];
	
    http_type = atoi((char*)argv[4]);//GET:2/POST:3
    
    	if((http_type <= 1) || (http_type > 3)) {
	 printf("\n\r set http type failed \n\r");
	error_no = 5;
	goto err_exit;
    	}
   

	http_resourse=argv[5];
		if(http_resourse == NULL) {
	 printf("\n\r set http resourse failed \n\r");
	error_no = 6;
	goto err_exit;
    	}	

    http_content = argv[7];
		if((http_type == 3) && (http_content == NULL))
			{
			printf("\n\r set post content  failed \n\r");
			   error_no = 7;
			   goto err_exit;


		}

	
	 post_data=argv[8]; 
		
	 if((http_type == 3) && (post_data == NULL))
				 {
				 printf("\n\r set post data failed \n\r");
					error_no = 8;
					goto err_exit;
	 
	 
			 }

    ssl_verify = atoi((char*)argv[6]);

	 if((transport_type == 2) && (( ssl_verify <= 0) || (ssl_verify > 2)))
				 {
				 printf("\n\r set https client verify failed \n\r");
					error_no = 9;
					goto err_exit;
	 
	 
			 }

if (transport_type == 1)
{

 if(xTaskCreate(http_client_thread_ver2, "http_client_thread_ver2", 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
   {	
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,"[ATPb] ERROR: Create http client task failed.");
		error_no = 10;
		goto err_exit;
	}
}

else  if (transport_type == 2)
{
      if(xTaskCreate(https_client_thread_ver2, "https_client_thread_ver2", 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
   {	
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,"[ATPb] ERROR: Create https client task failed.");
		error_no = 11;
		goto err_exit;
	}
     
}
	exit:
	    at_printf("%sOK\r\n", "+HTTPCLIENT:");
		ATCMD_NEWLINE_HASHTAG();
		return;

    err_exit:		
		at_printf("%sERROR:%d\r\n", "+HTTPCLIENT:", error_no);
		ATCMD_NEWLINE_HASHTAG();
		return;
 }

/* read/set CA cert/pk_key  */


void fATPB(void *arg)
{
 int argc;
 char *argv[MAX_ARGC] = {0};
 int error_no = 0;
  int  cert_type;
  int ret;
  int cert_len;
  unsigned char *output_buf;
  argc = parse_param(arg, argv);
 
	if(!arg){
		AT_DBG_MSG(AT_FLAG_WIFI, AT_DBG_ERROR,
			"[+SSLCRET] Usage : AT+SSLCRET=<TYPE:client CA:1/privkey:2/server root CA :3/pubkey:4>,[<LENGTH>,<CRT>]\n\r"\
            "           eg: AT+SSLCRET=1\n\r"\
            "             : AT+SSLCRET=1,10,1234567890\n\r"
		);
		error_no = 1;
		goto err_exit;
		}

	cert_type=atoi((char*)argv[1]);
	if(cert_type == NULL)
		{
    printf("\n\r set cert type failed \n\r");
	error_no = 2;
	goto err_exit;
	}

		
/*  -----get certificate&key-------*/
     
    if( argc == 2 ){

	 if(cert_type == 1){

        output_buf = malloc(strlen(test_client_crt));
		 memset(output_buf,0,strlen(test_client_crt));
		
	    strncpy((char *)output_buf, test_client_crt, strlen(test_client_crt));
		output_buf[strlen(test_client_crt)] = '\0';
	    
		   
	    printf("\n\r read client CA pem is %s\n\r", output_buf);

		free(output_buf);
		
	    goto err_exit;
		 
		 }
        
		else  if(cert_type == 2){
			
		output_buf = malloc(strlen(test_client_key));
		memset(output_buf,0,strlen(test_client_key));

	    strncpy((char *)output_buf, test_client_key, strlen(test_client_key));
		output_buf[strlen(test_client_key)] = '\0';
		
        printf("\n\r read privkey is %s\n\r ", output_buf);
		
		free(output_buf);
		goto err_exit;
	
		 	}


		 else if(cert_type == 4){
		 

		 if ((ret = mbedtls_x509_crt_parse(&client_x509, (const unsigned char *) test_client_crt, strlen((char const *)test_client_crt) + 1)) != 0) {
					  
					 printf(" failed\n	! mbedtls_x509_crt_parse returned %d \n\n", ret);
					 error_no = 3;
					goto err_exit;
				 }
			 
				 if ((ret = mbedtls_x509_crt_parse(&client_x509, (const unsigned char *) test_ca_crt, strlen((char const *)test_ca_crt) + 1)) != 0) {
					  
					 printf(" failed\n	! mbedtls_x509_crt_parse returned %d \n\n", ret );
					 error_no = 4;
					goto err_exit;
				 }


	     mbedtls_pk_init(&client_pub_key);
		 memcpy(&client_pub_key, &client_x509.pk, sizeof(mbedtls_pk_context));
		 

		 output_buf = malloc(1024);
		 memset(output_buf,0,1024);
		 if( ( ret = mbedtls_pk_write_pubkey_pem( &client_pub_key, output_buf,1024) ) != 0 )
		 	{
            printf("parse public key  err");
			goto err_exit;
		 
		 	}

            printf("read pubkey is %s\n", output_buf);
			free(output_buf);
			
		 goto err_exit;
	
	        }

		 else if(cert_type == 3){


		 output_buf = malloc(strlen(test_ca_crt));
		 memset(output_buf,0,strlen(test_ca_crt));
		
	    strncpy((char *)output_buf, test_ca_crt, strlen(test_ca_crt));
		output_buf[strlen(test_ca_crt)] = '\0';
	   
		printf("\n\r read server root  CA  is %s\n\r", output_buf);

		free(output_buf);
		
	    goto err_exit;


		 }

		 else
		 	{

			 printf("\n\r cert type set invalid \n\r");
           goto err_exit;
		 }

		  }


	else if( argc == 4 ){



	cert_len=atoi((char*)argv[2]);
	
	printf("/n/rcert_len is %d/n/r",cert_len);

	output_buf = malloc(cert_len);
	
	memset(output_buf,0,cert_len);
	
	output_buf =  argv[3];


	 if(cert_type == 1){

	     printf("\n\r ----start write client CA pem----\n\r  ");
         strncpy(test_client_crt ,output_buf,strlen(output_buf)); 
		test_client_crt[strlen(output_buf)] ='\0';
		
		 

        printf("\n\r set client CA pem is \n\r%s\n\r", test_client_crt);

		free(output_buf);
		
	    goto err_exit;
		 
		 }


	 else if(cert_type == 2){
	 
			  printf("\n\r ----start write client privekey---\n\r  ");
			  strncpy(test_client_key ,output_buf,strlen(output_buf)); 
			  test_client_key[strlen(output_buf)] ='\0';
	 
			 printf("\n\r set client peiveley is \n\r%s\n\r", test_client_key);
	 
			 free(output_buf);
			 
			 goto err_exit;
			  
			  }


	else  if(cert_type == 3){
		 
				  printf("\n\r ----start write server root CA---\n\r  ");
				  strncpy(test_ca_crt ,output_buf,strlen(output_buf)); 
				  test_ca_crt[strlen(output_buf)] ='\0';
		 
				 printf("\n\r set server  root CA is \n\r%s\n\r", test_ca_crt);
		 
				 free(output_buf);
				 
				 goto err_exit;
				  
				  }

else 

{
           printf("\n\r cert type set invalid \n\r");
		   free(output_buf);
           goto err_exit;}

	}

else {

    printf("[ATPB] Usage : ATPB=<TYPE 1: client CA ??/privkey:2/server root CA :3/pubkey:4>,[<LENGTH>,<CRT>]");

}
    at_printf("%sOK\r\n", "+SSLCRET:");
	ATCMD_NEWLINE_HASHTAG();

err_exit:				
	at_printf("%sERROR:%d\r\n", "+SSLCRET:", error_no);
	ATCMD_NEWLINE_HASHTAG();
    return;
}

void fATWR(void *arg){
       /* To avoid gcc warnings */
       ( void ) arg;
       
       int rssi = 0;
       printf("[ATWR]: _AT_WLAN_GET_RSSI_\n\r"); 
       wifi_get_rssi(&rssi);
       at_printf("RSSI = %d\r\n", rssi);
       at_printf("%sOK\r\n", "+WLRSSI:");
	   ATCMD_NEWLINE_HASHTAG();
}

void fATWr(void *arg){
	int rssi = 0, argc = 0, error_no;
	char ssid[33] = {0};
	rtw_scan_result_t* user_data = &scan_ap;
	char *argv[MAX_ARGC] = {0};

	argc = parse_param(arg, argv);

	if (argc != 3 || argv[1] == NULL || argv[2] == NULL) {
		error_no = 1;
		goto exit;
	}

	memset(user_data, 0, sizeof(rtw_scan_result_t));
	
	strncpy(user_data->SSID.val, (char*)argv[1], strlen(argv[1]));
	user_data->SSID.len = strlen(argv[1]);
	user_data->security = atoi(argv[2]);

	for (int i = 0; i < 3; i++) {
		wifi_scan_networks(app_scan_result_handler, user_data);
		user_data->band = 0;
		while (user_data->band == 0) {
			vTaskDelay(10);
		}
	}
	
	if (user_data->signal_strength) {
		at_printf("RSSI = %d\r\n", user_data->signal_strength);
		at_printf("%sOK\r\n", "+WLAPRSSI:");
		ATCMD_NEWLINE_HASHTAG();
		return;
	} else {
		error_no = 2;
	}

exit:
	at_printf("%sERROR:%d\r\n", "+WLAPRSSI:", error_no);
	ATCMD_NEWLINE_HASHTAG();
}



void print_wlan_help(void *arg){
	at_printf("\r\nWLAN AT COMMAND SET:");
	at_printf("\r\n==============================");
	at_printf("\r\n1. Wlan Scan for Network Access Point");
	at_printf("\r\n   # ATWS");
	at_printf("\r\n2. Connect to an AES AP");
	at_printf("\r\n   # ATPN=<ssid>,<pwd>,<key_id>(,<bssid>)");
	at_printf("\r\n3. Create an AES AP");
	at_printf("\r\n   # ATPA=<ssid>,<pwd>,<chl>,<hidden>");
}

#endif // end of #if ATCMD_VER == ATVER_1

#endif // end of #if CONFIG_WLAN

#if CONFIG_LWIP_LAYER
#if ATCMD_VER == ATVER_1
void fATWL(void *arg){
	/* To avoid gcc warnings */
	( void ) arg;
#if CONFIG_SSL_CLIENT
	int argc;
	char *argv[MAX_ARGC] = {0};
        printf("[ATWL]: _AT_WLAN_SSL_CLIENT_\n\r"); 
        argv[0] = "ssl_client";
        if(!arg){
          printf("ATWL=SSL_SERVER_HOST\n\r");
          return;
        }
        if((argc = parse_param(arg, argv)) > 1){
          if(argc != 2) {
            printf("ATWL=SSL_SERVER_HOST\n\r");
            return;
          }

          cmd_ssl_client(argc, argv);
        }
#else
	printf("Please set CONFIG_SSL_CLIENT 1 in platform_opts.h to enable ATWL command\n");
#endif
}

void fATWI(void *arg){
    int argc;
    char *argv[MAX_ARGC] = {0};

    printf("[ATWI]: _AT_WLAN_PING_TEST_\n\r");

	if (!arg) {
		printf("\n\r[ATWI] Usage: ATWI=[host],[options]\n");
		printf("\n\r     stop      Terminate ping \n");
		printf("  \r     -t    #   Ping the specified host until stopped\n");
		printf("  \r     -n    #   Number of echo requests to send (default 4 times)\n");
		printf("  \r     -l    #   Send buffer size (default 32 bytes)\n");
		printf("\n\r   Example:\n");
		printf("  \r     ATWI=192.168.1.2,-n,100,-l,5000\n");
		return;
	}

	argv[0] = (char *)"ping";

    if((argc = parse_param(arg, argv)) > 1){
        cmd_ping(argc, argv);
    }
}

void fATWT(void *arg)
{
#if CONFIG_BSD_TCP
    int argc;
    char *argv[MAX_ARGC] = {0};

    printf("[ATWT]: _AT_WLAN_TCP_TEST_\n\r");

    if(!arg){
		printf("\n\r[ATWT] Usage: ATWT=[-s|-c,host|stop],[options]\n");
		printf("\n\r   Client/Server:\n");
		printf("  \r	  ?     		List all stream status\n");
		printf("  \r	 stop  #		terminate specific stream id or terminate all stream if no id specified\n");
		printf("  \r	 -i    #		seconds between periodic bandwidth reports\n");
		printf("  \r	 -l    #		length of buffer to read or write (default 1460 Bytes)\n");
		printf("  \r	 -p    #		server port to listen on/connect to (default 5001)\n");
		printf("\n\r   Server specific:\n");
		printf("  \r	 -s 			run in server mode\n");
		printf("\n\r   Client specific:\n");
		printf("  \r	 -c    <host>	run in client mode, connecting to <host>\n");
		printf("  \r	 -d 			Do a bidirectional test simultaneously\n");
		printf("  \r	 -t    #		time in seconds to transmit for (default 10 secs)\n");
		printf("  \r	 -n    #[KM]	number of bytes to transmit (instead of -t)\n");
		printf("\n\r   Example:\n");
		printf("  \r	 ATWT=-s,-p,5002\n");
		printf("  \r	 ATWT=-c,192.168.1.2,-t,100,-p,5002\n");
		return;
    }

    argv[0] = (char *)"tcp";

    if((argc = parse_param(arg, argv)) > 1){
        cmd_iperf(argc, argv);
    }
#else
	printf("Please set CONFIG_BSD_TCP 1 in platform_opts.h to enable ATWT command\n");
#endif
}

void fATWU(void *arg)
{
#if CONFIG_BSD_TCP
	int argc;
	char *argv[MAX_ARGC] = {0};

	printf("[ATWU]: _AT_WLAN_UDP_TEST_\n\r");

	if (!arg) {
		printf("\n\r[ATWU] Usage: ATWU=[-s|-c,host|stop][options]\n");
		printf("\n\r   Client/Server:\n");
		printf("  \r	  ?     		List all stream status\n");
		printf("  \r	 stop  #		terminate specific stream id or terminate all stream if no id specified\n");
		printf("  \r	 -i    #		seconds between periodic bandwidth reports\n");
		printf("  \r	 -l    #		length of buffer to read or write (default 1460 Bytes)\n");
		printf("  \r	 -p    #		server port to listen on/connect to (default 5001)\n");
		printf("\n\r   Server specific:\n");
		printf("  \r	 -s 			run in server mode\n");
		printf("\n\r   Client specific:\n");
		printf("  \r	 -b    #[KM]	for UDP, bandwidth to send at in bits/sec (default 1 Mbit/sec)\n");
		printf("  \r	 -c    <host>	run in client mode, connecting to <host>\n");
		printf("  \r	 -d 			Do a bidirectional test simultaneously\n");
		printf("  \r	 -t    #		time in seconds to transmit for (default 10 secs)\n");
		printf("  \r	 -n    #[KM]	number of bytes to transmit (instead of -t)\n");
		printf("  \r     -S    #        set the IP 'type of service'\n");
		printf("\n\r   Example:\n");
		printf("  \r     ATWU=-s,-p,5002\n");
		printf("  \r     ATWU=-c,192.168.1.2,-t,100,-p,5002\n");
		return;
	}

	argv[0] = (char *)"udp";

	if((argc = parse_param(arg, argv)) > 1){
	    cmd_iperf(argc, argv);
    }
#else
	printf("Please set CONFIG_BSD_TCP 1 in platform_opts.h to enable ATWU command\n");
#endif
}
#elif ATCMD_VER == ATVER_2 // uart at command
//move to atcmd_lwip.c
#endif
#endif
log_item_t at_wifi_items[ ] = {
#if ATCMD_VER == ATVER_1
#if CONFIG_LWIP_LAYER
	{"ATWL", fATWL,{NULL,NULL}},
	{"ATWI", fATWI,{NULL,NULL}}, 
	{"ATWT", fATWT,{NULL,NULL}},
	{"ATWU", fATWU,{NULL,NULL}},
#endif
#if WIFI_LOGO_CERTIFICATION_CONFIG
	{"ATPE", fATPE,}, // set static IP for STA
#ifdef CONFIG_SAE_SUPPORT
	{"ATGP", fATWGRP,}, // set SAE group
#endif
#ifdef CONFIG_PMKSA_CACHING
	{"ATPM", fATWPMK,},// enable pmk
#endif
#ifdef CONFIG_IEEE80211W
	{"ATMF", fATWPMF,{NULL,NULL}},
#endif
#endif
#ifdef CONFIG_GHD_EN
	{"HDPD", fGHPM,{NULL,NULL}},
	{"HDLB", fGHLB,{NULL,NULL}},
	{"GHRL", fGHRL,{NULL,NULL}},
	{"GHRA", fGHRA,{NULL,NULL}},
#endif
#if CONFIG_WLAN
	{"ATW0", fATW0,{NULL,NULL}},
	{"ATW1", fATW1,{NULL,NULL}},
	{"ATW2", fATW2,{NULL,NULL}},
	{"ATW3", fATW3,{NULL,NULL}},
	{"ATW4", fATW4,{NULL,NULL}},
	{"ATW5", fATW5,{NULL,NULL}},
	{"ATW6", fATW6,{NULL,NULL}},
	{"ATW8", fATW8,{NULL,NULL}},
#if CONFIG_AUTO_RECONNECT
	{"ATW9", fATW9,{NULL,NULL}}, //Set auto reconnect mode
#endif
#ifdef CONFIG_AP_SECURITY
	{"ATW7", fATW7,},
#endif	
	{"ATWA", fATWA,{NULL,NULL}},
#ifdef  CONFIG_CONCURRENT_MODE
	{"ATWB", fATWB,{NULL,NULL}},
	{"ATWb", fATWb,{NULL,NULL}},
	{"ATWa", fATWa,{NULL,NULL}},
#endif
	{"ATWC", fATWC,{NULL,NULL}},
	{"ATWD", fATWD,{NULL,NULL}},
	{"ATWP", fATWP,{NULL,NULL}},
#if CONFIG_WOWLAN_SERVICE
	{"ATWV", fATWV,},
#endif	
	{"ATWR", fATWR,{NULL,NULL}},
	{"ATWS", fATWS,{NULL,NULL}},
#ifdef WIFI_PERFORMANCE_MONITOR
	{"ATWm", fATWm,},
#endif
#if SCAN_WITH_SSID
	{"ATWs", fATWs,{NULL,NULL}},
#endif
#ifdef CONFIG_PROMISC
	{"ATWM", fATWM,{NULL,NULL}},
#endif
#ifdef CONFIG_BT_COEXIST_SOC
	{"ATWE", fATWE,{NULL,NULL}},
#endif
    {"ATWZ", fATWZ,{NULL,NULL}},
#if CONFIG_OTA_UPDATE
	{"ATWO", fATWO,},
#endif
#if (CONFIG_INCLUDE_SIMPLE_CONFIG)	
	{"ATWQ", fATWQ,{NULL,NULL}},
#endif	
#if defined(CONFIG_INCLUDE_DPP_CONFIG) && CONFIG_INCLUDE_DPP_CONFIG
	{"ATWq", fATWq,},
#endif
#ifdef CONFIG_WPS
	{"ATWW", fATWW,{NULL,NULL}},
	{"ATWw", fATWw,{NULL,NULL}}, //wps registrar for softap
#if CONFIG_ENABLE_P2P
	{"ATWG", fATWG,},  //p2p start
	{"ATWH", fATWH,},  //p2p stop
	{"ATWJ", fATWJ,},  //p2p connect
	{"ATWK", fATWK,},  //p2p disconnect
	{"ATWN", fATWN,},  //p2p info
	{"ATWF", fATWF,},  //p2p find
	{"ATWg", fATWg,},  //p2p auto go start
#endif
#endif

#if CONFIG_AIRKISS
	{"ATWX", fATWX,}, 
#endif
	{"ATW?", fATWx,{NULL,NULL}},
	{"ATW+ABC", fATWx,{NULL,NULL}},
#ifdef CONFIG_POWER_SAVING
	{"ATXP", fATXP,{NULL,NULL}},
#endif
	{"ATWc", fATWc,{NULL,NULL}},
	{"ATWd", fATWd,{NULL,NULL}},
#endif
#elif ATCMD_VER == ATVER_2 // uart at command
#if CONFIG_WLAN
	{"+WLSOFTAP", fATPA,}, // set AP
	{"+WLCONN", fATPN,}, // connect to Network
#if ENABLE_SET_AP_MODE
	{"+WLDMOD", fATPH,}, // set DHCP mode
	{"+WLDRULE", fATPF,}, // set DHCP rule for AP
#endif
	{"+WLSTATICIP", fATPE,}, // set static IP for STA
	{"+WLAUTOCONN", fATPG,}, // set auto connect
	{"+WLRECONN", fATRC,}, // set auto reconnect
	{"+WLMAC", fATPM,}, // set MAC address
#if ENABLE_SET_AP_MODE
	{"+WLMODE", fATPW,}, // set Wifi mode
#endif
	{"+WLDISCONN", fATWD,},
	{"+WLSCAN", fATWS,},
	{"+WLSTATE", fATWx,},
	{"+SSLCRET", fATPB,},  // read/write ca cert
	{"+HTTPCLIENT", fATPb,},  // set http client
	{"+WLRSSI", fATWR,},//Read RSSI
	{"+WLAPRSSI", fATWr,},//Read RSSI
#if (CONFIG_INCLUDE_SIMPLE_CONFIG)
	{"+WLSMPLCFG", fATWQ,},
#endif // #if (CONFIG_INCLUDE_SIMPLE_CONFIG)
#endif // #if CONFIG_WLAN
#endif // end of #if ATCMD_VER == ATVER_1
};

#if ATCMD_VER == ATVER_2
void print_wifi_at(void *arg){
	int index;
	int cmd_len = 0;

	cmd_len = sizeof(at_wifi_items)/sizeof(at_wifi_items[0]);
	for(index = 0; index < cmd_len; index++)
	{
		at_printf("AT%s\r\n", at_wifi_items[index].log_cmd);
	}
}
#endif

void at_wifi_init(void)
{
#if CONFIG_WLAN
	init_wifi_struct();
#endif
	log_service_add_table(at_wifi_items, sizeof(at_wifi_items)/sizeof(at_wifi_items[0]));
}

#if SUPPORT_LOG_SERVICE
log_module_init(at_wifi_init);
#endif
