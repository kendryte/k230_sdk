#ifndef __IPCM_PLATFORM_H__
#define __IPCM_PLATFORM_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

#include "device_config.h"

#define __NODES_DESC_MEM_BASE__ 0x1fd000
#define __NODES_DESC_MEM_OFFSET__   0x0

#define MAILBOX_REG_BASE            0x91104000
#define CPU2DSP_INT_EN0	            0x0000
#define CPU2DSP_INT_SET0	        0x0004
#define CPU2DSP_INT_CLEAR0	        0x0008
#define CPU2DSP_INT_STATUS0	        0x000C
#define CPU2DSP_INT_ERR0	        0x0010
#define DSP2CPU_INT_EN0	            0x0014
#define DSP2CPU_INT_SET0	        0x0018
#define DSP2CPU_INT_CLEAR0	        0x001C
#define DSP2CPU_INT_STATUS0	        0x0020
#define DSP2CPU_INT_ERR0	        0x0024

#define INTR_RAW_EN_EABLE           (1 << 16)
#define INTR_RAW_EN_DISABLE         (0 << 16)
#define INTR_SOFT_RST_VALID         (1 << 1)
#define INTR_SORT_RST_INVAILD       (0 << 1)
#define INTR_EN_EABLE               (1 << 0)
#define INTR_EN_DISABLE             (0 << 0)

#define IRQN_MAILBOX_CPUATOCPUB_0_INTERRUPT     (16 + 93)
#define IRQN_MAILBOX_CPUATOCPUB_1_INTERRUPT     (16 + 94)
#define IRQN_MAILBOX_CPUATOCPUB_2_INTERRUPT     (16 + 95)
#define IRQN_MAILBOX_CPUATOCPUB_3_INTERRUPT     (16 + 96)

#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif
