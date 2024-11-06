#pragma once
#ifndef __MPI_DE200_H__
#define __MPI_DE200_H__

#include "k_dewarp_comm.h"
#include "k_isp_comm.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int kd_mpi_dewarp_reset(void);
int kd_mpi_dewarp_dwe_enable_bus(void);
int kd_mpi_dewarp_dwe_disable_bus(void);
int kd_mpi_dewarp_dwe_disable_irq(void);
int kd_mpi_dewarp_clear_irq(bool select);
int kd_mpi_dewarp_dwe_s_params(struct k_dwe_hw_info* params);
int kd_mpi_dewarp_set_map_lut_addr(uint32_t addr);
int kd_mpi_dewarp_start_dwe(void);
int kd_mpi_dewarp_vse_s_params(struct k_vse_params* params);
uint32_t kd_mpi_dewarp_read_irq(bool select);
int kd_mpi_dewarp_update_buffer(uint32_t* addr);
int kd_mpi_dewarp_set_mi_info(void);
int kd_mpi_dewarp_set_dst_buffer_addr(uint32_t addr);
int kd_mpi_dewarp_start_dma_read(uint32_t addr);
int kd_mpi_dewarp_mask_irq(uint32_t mask);
int kd_mpi_dewarp_set_dma_buffer_info(uint32_t addr);
int kd_mpi_dewarp_poll_irq(void);
int kd_mpi_dewarp_init(void);
void kd_mpi_dewarp_exit(void);

static inline int kd_mpi_dewarp_disable_irq(void) {
    return kd_mpi_dewarp_dwe_disable_irq();
}
static inline int kd_mpi_dewarp_disable_bus(void) {
    return kd_mpi_dewarp_dwe_disable_bus();
}
static inline int kd_mpi_dewarp_enable_bus(void) {
    return kd_mpi_dewarp_dwe_enable_bus();
}

// for vicap
void kd_mpi_vicap_dw_exit(k_isp_dev dev_num);
int kd_mpi_vicap_dw_init(k_isp_dev dev_num);
int kd_mpi_vicap_dw_load(struct k_dw_load_request *lr);
int kd_mpi_vicap_dw_dump_register(k_isp_dev dev_num, FILE* f);
/**
 * DO kd_mpi_vicap_init BEFORE kd_mpi_dw_init !!!!
 * VICAP WILL RESET DEWARP
 */
int kd_mpi_dw_init(struct k_dw_settings* settings);
int kd_mpi_dw_exit(unsigned dev_id);

#ifdef __cplusplus
}
#endif

#endif
