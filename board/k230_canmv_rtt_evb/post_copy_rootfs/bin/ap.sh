usage()
{
    cat <<EOF
    usage: $0 wlanx  ssid password;
    example: $0 wlan0  wjx_pc 123456789 g/a/ac/b
EOF
}
[ $# -lt 3 ] && { usage $* ;} && { exit 1 ;}
set -x;
wlandev="$1"
ssid="$2"
password="$3"
hw_mode="g"

[ $# -ge 4 ] && { hw_mode="$4" ;}

killall hostapd
killall udhcpd



cat >/etc/udhcpd.conf <<EOF
#cat /etc/udhcpd.conf
start        192.168.10.20
end        192.168.10.160
interface    ${wlandev}
EOF



cat >/etc/hostapd.conf <<EOF
#cat /etc/hostapd.conf
ctrl_interface=/var/run/hostapd
driver=nl80211
ieee80211n=1
interface=${wlandev}
EOF

if [ ${hw_mode} == "ac" ]; then
echo "hw_mode=a" >> /etc/hostapd.conf
echo "ieee80211ac=1" >> /etc/hostapd.conf
elif [ ${hw_mode} == "a" ]; then
    echo "hw_mode=${hw_mode}">> /etc/hostapd.conf
else
echo "hw_mode=${hw_mode}">> /etc/hostapd.conf
echo "channel=11" >> /etc/hostapd.conf
fi

cat >>/etc/hostapd.conf <<EOF
beacon_int=100
dtim_period=1
auth_algs=1
ap_isolate=0
ignore_broadcast_ssid=0
wpa=2
wpa_key_mgmt=WPA-PSK
ieee80211w=1
wpa_pairwise=CCMP
wpa_group_rekey=3600
wpa_ptk_rekey=0
ssid=${ssid}
wpa_passphrase=${password}
EOF


ifconfig ${wlandev} up
ifconfig ${wlandev} 192.168.10.1
hostapd /etc/hostapd.conf -B
udhcpd /etc/udhcpd.conf &
sleep 5
hostapd_cli status
