#!/bin/sh
usage()
{
    echo "help"
    cat <<EOF
    usage: $0 wlanx  ssid password;
    example: $0 wlan0  wjx_pc 123456789

EOF
}
[ $# -ne 3 ] && { usage $* ;} && { exit 1 ;}
set -x;
wlandev="$1"
ssid="$2"
password="$3"


set -x;
cat >/etc/wpa_supplicant.conf  <<EOF
ctrl_interface=/var/run/wpa_supplicant
ap_scan=1

network={
  key_mgmt=NONE
}
EOF

$(ps | grep wpa_supplicant | grep -v grep  >/dev/null 2>&1 ;) && { $(killall wpa_supplicant); sleep 10 ;}
wpa_supplicant -D nl80211 -i ${wlandev} -c /etc/wpa_supplicant.conf -B
sleep 2
wpa_cli -i ${wlandev} scan;
sleep 1
wpa_cli -i ${wlandev} scan_result;
sleep 1
wpa_cli -i ${wlandev} add_network
#wpa_cli -i ${wlandev} set_network 1 ssid '"hsap"'  #2.4G
wpa_cli -i ${wlandev} set_network 1 ssid \"${ssid}\"  #5G
wpa_cli -i ${wlandev} set_network 1 psk \"${password}\"
#wpa_cli -i ${wlandev}  list_network           #查看当前设备下当前记住几个SSID

wpa_cli -i ${wlandev} select_network 1
udhcpc -i ${wlandev} -q -n #
#ifmetric ${wlandev} 100  #设置路由优先级
wpa_cli -i ${wlandev}  status   //查看网络状态
