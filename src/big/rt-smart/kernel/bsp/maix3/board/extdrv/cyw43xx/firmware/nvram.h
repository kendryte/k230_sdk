// #AP6212A_NVRAM_V1.0.2_20191121
// # 2.4 GHz, 20 MHz BW mode

// # The following parameter values are just placeholders, need to be updated.
static const uint8_t wifi_nvram_4343[] CYW43_RESOURCE_ATTRIBUTE =
"manfid=0x2d0\x00"
"prodid=0x0726\x00"
"vendid=0x14e4\x00"
"devid=0x43e2\x00"
"boardtype=0x0726\x00"
"boardrev=0x1101\x00"
"boardnum=22\x00"
"macaddr=00:90:4c:c5:12:38\x00"
"sromrev=11\x00"
"boardflags=0x00404201\x00"
"xtalfreq=26000\x00"
"nocrc=1\x00"
"ag0=255\x00"
"aa2g=1\x00"
"ccode=ALL\x00"

"pa0itssit=0x20\x00"
"extpagain2g=0\x00"

// #PA parameters for 2.4GHz, measured at CHIP OUTPUT
"pa2ga0=-215,5267,-656\x00"
"AvVmid_c0=0x0,0xc8\x00"
"cckpwroffset0=5\x00"

// # PPR params
"maxp2ga0=80\x00"
"txpwrbckof=6\x00"
"cckbw202gpo=0x6666\x00"
"legofdmbw202gpo=0xaaaaaaaa\x00"
"mcsbw202gpo=0xbbbbbbbb\x00"

// # OFDM IIR :
"ofdmdigfilttype=18\x00"
"ofdmdigfilttypebe=18\x00"
// # PAPD mode:
"papdmode=2\x00"

"il0macaddr=00:90:4c:c5:12:38\x00"
"wl0id=0x431b\x00"

// #OOB parameters
"hostwake=0x40\x00"
"hostrdy=0x41\x00"
"usbrdy=0x03\x00"
"usbrdydelay=100\x00"
"deadman_to=0xffffffff\x00"
// # muxenab: 0x1 for UART enable, 0x10 for Host awake
"muxenab=0x10\x00"
// # CLDO PWM voltage settings - 0x4 - 1.1 volt
// #cldo_pwm=0x4
"glitch_based_crsmin=1\x00"
"noccpwrlmt=1\x00"
"\x00\x00"
;