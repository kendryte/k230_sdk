/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "soc_types.h"
#include "securec.h"
#include "soc_base.h"

#include <getopt.h>
#include "soc_msg.h"
#include "hal_iotwifi_cli.h"
#include "cJSON.h"
#include "cJSON_Utils.h"

static struct option long_options[] = {
    //wifi_mac_t
	{"setmac", 	required_argument, NULL, '0'},     //mac addr
	{"getmac", 	no_argument,       NULL, '1'},
    //wifi_ip_t
	{"setip", 	required_argument, NULL, '2'},     //ip addr
    {"netmask", required_argument, NULL, '3'},     //ip addr
    {"gateway", required_argument, NULL, '4'},     //ip addr
	{"getip", 	no_argument,       NULL, '5'},

    //wifi_sleep_t
    {"wkgpios", 	required_argument, NULL, '6'}, //wake gpios
	{"sleep", 	    required_argument, NULL, '7'}, //sleep level
    {"period", 	    required_argument, NULL, '8'}, //sleep period
    //wifi_conn_t
	{"connect", 	required_argument, NULL, '9'}, //ap ssid
    {"bssid", 	    required_argument, NULL, 'A'},     //bssid
    {"password", 	required_argument, NULL, 'B'},     //password
    {"auth", 	    required_argument, NULL, 'C'},     //auth_type
    //wifi_keepalive_t
	{"keepalive", 	required_argument, NULL, 'D'},  //svrip
    {"port", 	    required_argument, NULL, 'E'},      //svrport
    {"katime", 	    required_argument, NULL, 'F'},      //tcp_keepalive_time
    {"kaintvl", 	required_argument, NULL, 'G'},      //tcp_keepalive_intvl
    {"kaprobes", 	required_argument, NULL, 'H'},      //tcp_keepalive_probes
    {"hrintvl", 	required_argument, NULL, 'I'},      //heartbeat_intvl
    {"hrprobes", 	required_argument, NULL, 'J'},      //heartbeat_probes
    //wifi_hstanby_t
    //{"hstanby", 	no_argument, NULL, 'K'},      //host stanby
    {"config", 	    required_argument, NULL, 'L'},      //config
    {"dirsleep",    no_argument,       NULL, 'M'},      //simple sleeps

    {"help", 	    required_argument, NULL, 'Z'},      //usage
	{0, 		0, 				   0, 	   0},
};

void usage(void)
{
    printf(
        "./iotwifi_cli --getmac\n"
        "./iotwifi_cli --setmac <mac addr>\n"
        "----example: ./iotwifi_cli --setmac 90:2B:D2:E4:CE:40\n\n"
        
        "./iotwifi_cli --getip\n"
        "./iotwifi_cli --setip <ip addr> --netmask <netmask> --gateway <gateway>\n"
        "----example: ./iotwifi_cli --setip 192.168.1.100 --netmask 255.255.255.0 --gateway 192.168.1.1\n\n"
        
        "./iotwifi_cli --sleep <level> --period <period ms> --wkgpios <gpio number array>\n"
        "-----------: level = 0, exit sleep\n"
        "-----------: level = 1, light sleep\n"
        "-----------: level = 2, deep sleep\n" 
        "-----------: level = 3, ultra deep sleep\n"
        "-----------: period = 33ms ~~ 4000ms\n"
        "----example: ./iotwifi_cli --sleep 2 --period 2000 --wkgpios 5,7\n\n"

        "./iotwifi_cli --connect <ap name> --bssid <ap mac> --password <ap password> --auth <auth type>\n"
        "-----------: bssid = empty or ap mac address\n"
        "-----------: auth = 0, OPEN\n"
        "-----------: auth = 1, WEP\n"
        "-----------: auth = 2, WPA2_PSK\n"
        "-----------: auth = 3, WPA_WPA2_PSK\n"
        "-----------: auth = 7, SAE\n"
        "-----------: auth = 8, WPA3_WPA2_PSK\n"
        "----example: ./iotwifi_cli --connect myap --bssid a0:36:bc:9a:b1:d8 --password myap123456 --auth 2\n"
        "----example: ./iotwifi_cli --connect myap --password myap123456 --auth 2\n\n"

        "./iotwifi_cli --keepalive <server ip> --port <server port> --katime <tcp_keepalive_time>"
                " --kaintvl <tcp_keepalive_intvl> --kaprobes <tcp_keepalive_probes> --hrintvl <heartbeat_intvl>"
                " --hrprobes <heartbeat_probes>\n"
        "----example: ./iotwifi_cli --keepalive 192.168.1.110 --port 10101 --katime 300"
                " --kaintvl 2 --kaprobes 10 --hrintvl 60 --hrprobes 5\n\n"
        /*
        "./iotwifi_cli --hstanby --sleep <level> --period <period ms>\n"
        "----example: ./iotwifi_cli --hstanby --sleep 2 --period 2000\n\n"
        */
        "./iotwifi_cli --config <config file>\n"
        "----example: ./iotwifi_cli --config wifi.conf\n\n"

        "./iotwifi_cli --dirsleep\n\n"
        );
}

typedef int (* msg_xfer_fn)(void *);

typedef struct
{
    msg_xfer_fn msg_xfer;
    void *data;
    int length;
} msg_xfer_t;

static int parse_wake_gpios(char *wkgpios);
static int prepare_config(char *conf_file, wifi_config_t *config);
void dump_config(wifi_config_t *config);

int main(int argc, const char **argv)
{
    int ret = -1;
    int option_index = 0;
    signed char opt = 0x00;
    int cmd = SOCCHN_CMD_END;

    wifi_mac_t mac;
    memset(&mac, 0x00, sizeof(wifi_mac_t));
    wifi_ip_t ip;
    memset(&ip, 0x00, sizeof(wifi_ip_t));
    wifi_connect_t conn;
    memset(&conn, 0x00, sizeof(wifi_connect_t));
    wifi_keepalive_t keepalive;
    memset(&keepalive, 0x00, sizeof(wifi_keepalive_t));
    wifi_sleep_t sleep;
    memset(&sleep, 0x00, sizeof(wifi_sleep_t));
    wifi_config_t config;
    memset(&config, 0x00, sizeof(wifi_config_t));

    char conf_file[256];
    char wkgpios[128];

    msg_xfer_t msg_xfers[SOCCHN_CMD_END+1] = {
        [SOCCHN_CMD_SET_MAC]        = {kd_set_wifi_mac,         &mac,       sizeof(wifi_mac_t)},
        [SOCCHN_CMD_GET_MAC]        = {kd_get_wifi_mac,         &mac,       sizeof(wifi_mac_t)},
        [SOCCHN_CMD_SET_IP]         = {kd_set_wifi_ip,          &ip,        sizeof(wifi_ip_t)},
        [SOCCHN_CMD_GET_IP]         = {kd_get_wifi_ip,          &ip,        sizeof(wifi_ip_t)},
        [SOCCHN_CMD_SET_CONNECT]    = {kd_set_wifi_connect,     &conn,      sizeof(wifi_connect_t)},
        [SOCCHN_CMD_SET_KEEPALIVE]  = {kd_set_wifi_keepalive,   &keepalive, sizeof(wifi_keepalive_t)},
        [SOCCHN_CMD_SET_SLEEP]      = {kd_set_wifi_sleep,       &sleep,     sizeof(wifi_sleep_t)},
        [SOCCHN_CMD_SET_CONFIG]     = {kd_wifi_config,          &config,    sizeof(wifi_config_t)},
    };

    if (argc < 2)
    {
        usage();
        return -1;
    }

    while (-1 != (opt = getopt_long(argc, argv, "0:12:3:4:56:7:8:9:A:B:C:D:E:F:G:H:I:J:K:L:MZ", long_options, &option_index)))
    {
		switch (opt)
		{
			case '0':
				cmd = SOCCHN_CMD_SET_MAC;
                sscanf(optarg, "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", &mac.mac[0], &mac.mac[1], &mac.mac[2], 
                                                                &mac.mac[3], &mac.mac[4], &mac.mac[5]);
				break;				
			case '1':
				cmd = SOCCHN_CMD_GET_MAC;
                break;

			case '2':
				cmd = SOCCHN_CMD_SET_IP;
                sscanf(optarg, "%d.%d.%d.%d", &ip.ip[0], &ip.ip[1], &ip.ip[2], &ip.ip[3]);
				break;
			case '3':
				cmd = SOCCHN_CMD_SET_IP;
                sscanf(optarg, "%d.%d.%d.%d", &ip.netmask[0], &ip.netmask[1], &ip.netmask[2], &ip.netmask[3]);
				break;
			case '4':
				cmd = SOCCHN_CMD_SET_IP;
                sscanf(optarg, "%d.%d.%d.%d", &ip.gateway[0], &ip.gateway[1], &ip.gateway[2], &ip.gateway[3]);
				break;
			case '5':
                cmd = SOCCHN_CMD_GET_IP;
				break;

			case '6':
                cmd = SOCCHN_CMD_SET_SLEEP;
                memset(wkgpios, 0x00, sizeof(wkgpios));
                strncpy(wkgpios, optarg, sizeof(wkgpios)-1);
                sleep.wake_gpios = parse_wake_gpios(wkgpios);
				break;
			case '7':
                cmd = SOCCHN_CMD_SET_SLEEP;
                sleep.level = strtoul(optarg, NULL, 10);
				break;
			case '8':
                cmd = SOCCHN_CMD_SET_SLEEP;
                sleep.period = strtoul(optarg, NULL, 10);
				break;

			case '9':
                cmd = SOCCHN_CMD_SET_CONNECT;
				strncpy(conn.ssid, optarg, sizeof(conn.ssid));
				break;
			case 'A':
                cmd = SOCCHN_CMD_SET_CONNECT;
				strncpy(conn.bssid, optarg, sizeof(conn.bssid));
				break;
			case 'B':
                cmd = SOCCHN_CMD_SET_CONNECT;
				strncpy(conn.key, optarg, sizeof(conn.key));
				break;
            case 'C':
                cmd = SOCCHN_CMD_SET_CONNECT;
				conn.auth = strtoul(optarg, NULL, 10);
				break;
            
			case 'D':
                cmd = SOCCHN_CMD_SET_KEEPALIVE;
				strncpy(keepalive.svrip, optarg, sizeof(keepalive.svrip));
				break;
			case 'E':
                cmd = SOCCHN_CMD_SET_KEEPALIVE;
				keepalive.svrport = strtoul(optarg, NULL, 10);
				break; 
			case 'F':
                cmd = SOCCHN_CMD_SET_KEEPALIVE;
				keepalive.tcp_keepalive_time = strtoul(optarg, NULL, 10);
				break;  
			case 'G':
                cmd = SOCCHN_CMD_SET_KEEPALIVE;
				keepalive.tcp_keepalive_intvl = strtoul(optarg, NULL, 10);
				break; 
			case 'H':
                cmd = SOCCHN_CMD_SET_KEEPALIVE;
				keepalive.tcp_keepalive_probes = strtoul(optarg, NULL, 10);
				break; 
			case 'I':
                cmd = SOCCHN_CMD_SET_KEEPALIVE;
				keepalive.heartbeat_intvl = strtoul(optarg, NULL, 10);
				break; 
			case 'J':
                cmd = SOCCHN_CMD_SET_KEEPALIVE;
				keepalive.heartbeat_probes = strtoul(optarg, NULL, 10);
				break; 

			/* delete this command
            case 'K':
                cmd = SOCCHN_CMD_SET_HSTANBY;
				break; 
            */
            case 'L':
                cmd = SOCCHN_CMD_SET_CONFIG;
                memset(conf_file, 0x00, sizeof(conf_file));
                strncpy(conf_file, optarg, sizeof(conf_file)-1);
                prepare_config(conf_file, &config);
                dump_config(&config);
				break; 

           	case 'M':
                cmd = SOCCHN_CMD_SET_SLEEP;
				break; 

			case 'Z':
			default:
				usage();
				ret = -1;
                goto EXIT;
		}
    }

    if (cmd < 0 || cmd >= SOCCHN_CMD_END || !msg_xfers[cmd].msg_xfer)
    {
        printf("cmd invalid or msg_xfers[%d].msg_xfer is NULL\n", cmd, cmd);
        return -1;
    }

    if (msg_xfers[cmd].msg_xfer(msg_xfers[cmd].data) < 0)
    {
        printf("error: send msg %d failed\n", cmd);
        return -1;
    }

    ret = 0;
EXIT:
    return ret;
}

static int parse_wake_gpios(char *wkgpios)
{
    int wake_gpios = 0;
    int gpio_nr = 0;

    char *s = ",";
    char *token = NULL;

    if (!wkgpios)
       return wake_gpios;

    token = strtok(wkgpios, s);
    while (token != NULL)
    {
        gpio_nr = strtoul(token, NULL, 10);
        wake_gpios |= (1 << gpio_nr);
        token = strtok(NULL, s);
    }
    printf("wake gpios = %d\n", wake_gpios);
    return wake_gpios;
}

static int prepare_config(char *conf_file, wifi_config_t *config)
{
    int ret = -1;
    int fd = -1;
    struct stat filestat;
    char *data = NULL;

    cJSON *cjson = NULL;

    cJSON *cjson_sleep = NULL;
    cJSON *cjson_sleep_level = NULL;
    cJSON *cjson_sleep_period = NULL;
    cJSON *cjson_sleep_wake_gpios = NULL;

    cJSON *cjson_conn = NULL;
    cJSON *cjson_conn_ssid = NULL;
    cJSON *cjson_conn_auth = NULL;
    cJSON *cjson_conn_key = NULL;
    cJSON *cjson_conn_bssid = NULL;
    cJSON *cjson_conn_pairwise = NULL;

    cJSON *cjson_keepalive = NULL;
    cJSON *cjson_keepalive_svrip = NULL;
    cJSON *cjson_keepalive_svrport = NULL;
    cJSON *cjson_keepalive_time = NULL;
    cJSON *cjson_keepalive_intvl = NULL;
    cJSON *cjson_keepalive_probes = NULL;
    cJSON *cjson_keepalive_hrintvl = NULL;
    cJSON *cjson_keepalive_hrprobes = NULL;

    cJSON *cjson_item = NULL;

    int gpio_nr = 0;
    int i = 0;

    if (!conf_file || access(conf_file, F_OK))
    {
        printf("config file %s is not exist\n", conf_file);
        ret = -1;
        goto EXIT;
    }

    if (!config)
    {
        printf("config is NULL\n");
        ret = -1;
        goto EXIT;
    }

    fd = open(conf_file, O_RDONLY);
    if (fd < 0)
    {
        printf("open %s failed\n", conf_file);
        ret = -1;
        goto EXIT;
    }

    memset(&filestat, 0x00, sizeof(filestat));
    fstat(fd, &filestat);

    data = (char *) malloc(filestat.st_size);
    if (!data)
    {
        printf("malloc memory %ld bytes failed\n", filestat.st_size);
        ret = -1;
        goto EXIT;
    }
    memset (data, 0x00, filestat.st_size);

    ret = read(fd, data, filestat.st_size);
    if (ret != filestat.st_size)
    {
        printf("read lenght %d != target length %ld\n", ret, filestat.st_size);
        ret = -1;
        goto EXIT;
    }

    cjson = cJSON_Parse(data);

    cjson_sleep = cJSON_GetObjectItem(cjson, "sleep");
    cjson_sleep_level = cJSON_GetObjectItem(cjson_sleep, "level");
    cjson_sleep_period = cJSON_GetObjectItem(cjson_sleep, "period");
    cjson_sleep_wake_gpios = cJSON_GetObjectItem(cjson_sleep, "wake_gpios");
    config->sleep.level = cjson_sleep_level->valueint;
    config->sleep.period = cjson_sleep_period->valueint;
    for (i = 0; i < cJSON_GetArraySize(cjson_sleep_wake_gpios); i++)
    { 
        cjson_item = cJSON_GetArrayItem(cjson_sleep_wake_gpios, i);
        config->sleep.wake_gpios |= (1 << cjson_item->valueint);
    }

    cjson_conn = cJSON_GetObjectItem(cjson, "conn");
    cjson_conn_ssid = cJSON_GetObjectItem(cjson_conn, "ssid");
    cjson_conn_auth = cJSON_GetObjectItem(cjson_conn, "auth");
    cjson_conn_key = cJSON_GetObjectItem(cjson_conn, "key");
    cjson_conn_bssid = cJSON_GetObjectItem(cjson_conn, "bssid");
    cjson_conn_pairwise = cJSON_GetObjectItem(cjson_conn, "pairwise");
    strncpy(config->conn.ssid, cjson_conn_ssid->valuestring, sizeof(config->conn.ssid)-1);
    strncpy(config->conn.bssid, cjson_conn_bssid->valuestring, sizeof(config->conn.bssid)-1);
    strncpy(config->conn.key, cjson_conn_key->valuestring, sizeof(config->conn.key)-1);
    config->conn.auth = cjson_conn_auth->valueint;
    config->conn.pairwise = cjson_conn_pairwise->valueint;

    cjson_keepalive = cJSON_GetObjectItem(cjson, "keepalive");
    cjson_keepalive_svrip = cJSON_GetObjectItem(cjson_keepalive, "svrip");
    cjson_keepalive_svrport = cJSON_GetObjectItem(cjson_keepalive, "svrport");
    cjson_keepalive_time = cJSON_GetObjectItem(cjson_keepalive, "time");
    cjson_keepalive_intvl = cJSON_GetObjectItem(cjson_keepalive, "intvl");
    cjson_keepalive_probes = cJSON_GetObjectItem(cjson_keepalive, "probes");
    cjson_keepalive_hrintvl = cJSON_GetObjectItem(cjson_keepalive, "hrintvl");
    cjson_keepalive_hrprobes = cJSON_GetObjectItem(cjson_keepalive, "hrprobes");
    strncpy(config->keepalive.svrip, cjson_keepalive_svrip->valuestring, sizeof(config->keepalive.svrip)-1);
    config->keepalive.svrport = cjson_keepalive_svrport->valueint;
    config->keepalive.tcp_keepalive_time = cjson_keepalive_time->valueint;
    config->keepalive.tcp_keepalive_intvl = cjson_keepalive_intvl->valueint;
    config->keepalive.tcp_keepalive_probes = cjson_keepalive_probes->valueint;
    config->keepalive.heartbeat_intvl = cjson_keepalive_hrintvl->valueint;
    config->keepalive.heartbeat_probes = cjson_keepalive_hrprobes->valueint;

    config->config_mask = (CFG_MASK_SLEEP | CFG_MASK_CONNECT | CFG_MASK_KEEPALIVE);

    ret = 0;
EXIT:
    if (fd)
    {
        close(fd);
        fd = -1;
    }

    if (data)
    {
        free(data);
        data = NULL;
    }

    return ret;
}

void dump_config(wifi_config_t *config)
{
    printf("sleep leve = %d, period = %d, wake_gpios = %d\n", 
        config->sleep.level, config->sleep.period, config->sleep.wake_gpios);

    printf("ssid: %s, bssid: %s, password: %s, auth_type: %d\r\n",
        config->conn.ssid, config->conn.bssid, config->conn.key, config->conn.auth); 

    printf("svrip = %s, svrport = %d\n"
            "katime = %d, kaintvl = %d, kaprobes = %d\n"
            "hrintvl = %d, hrprobes = %d\n",
            config->keepalive.svrip, config->keepalive.svrport,
            config->keepalive.tcp_keepalive_time, config->keepalive.tcp_keepalive_intvl, 
            config->keepalive.tcp_keepalive_probes,
            config->keepalive.heartbeat_intvl, config->keepalive.heartbeat_probes);
}