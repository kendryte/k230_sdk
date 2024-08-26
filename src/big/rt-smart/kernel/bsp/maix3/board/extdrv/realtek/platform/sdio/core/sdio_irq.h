#ifndef _MMC_SDIO_IRQ_H
#define _MMC_SDIO_IRQ_H

#include "../include/card.h"

int sdio_claim_irq(struct sdio_func *func, void(*handler)(struct sdio_func *));
int sdio_release_irq(struct sdio_func *func);
void sdio_irq_thread(void* exinf);

#endif
