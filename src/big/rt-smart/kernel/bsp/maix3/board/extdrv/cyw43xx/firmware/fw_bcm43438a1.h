#include "incbin.h"

#define __AVX512BW__
INCBIN(fw_bcm43438a1, "fw_bcm43438a1.bin");
#define CYW43_WIFI_FW_LEN (gfw_bcm43438a1Size)
const uintptr_t *fw_data = (uintptr_t *)gfw_bcm43438a1Data;
#undef __AVX512BW__
