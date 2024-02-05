#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>


/***********************CMPI*****************************/
int __attribute__((weak)) cmpi_init(void)
{
    rt_kprintf("no cmpi library!\n");
    return -1;
}
extern int cmpi_init(void);
/***************************************************/

/********************LOG*****************************/
int __attribute__((weak)) log_init(void)
{
    rt_kprintf("no log library!\n");
    return -1;
}
extern int log_init(void);
/***************************************************/

/********************MMZ*****************************/
#ifndef MEM_MMZ_BASE
#define MEM_MMZ_BASE 0x10000000UL
#endif

#ifndef MEM_MMZ_SIZE
#define MEM_MMZ_SIZE 0xfbff000UL
#endif
int __attribute__((weak)) mmz_init(unsigned long  mmz_start, unsigned long mmz_size)
{
    rt_kprintf("no mmz library!\n");
    return -1;
}
extern int mmz_init(unsigned long  mmz_start, unsigned long mmz_size);
/****************************************************/

/********************MMZ_USEDEV**********************/
int __attribute__((weak)) mmz_userdev_init(void)
{
    rt_kprintf("no mmz userdev library!\n");
    return -1;
}
extern int mmz_userdev_init(void);
/****************************************************/

/********************MMZ_USEDEV**********************/
int __attribute__((weak)) vb_init(void)
{
    rt_kprintf("no vb library!\n");
    return -1;
}
extern int vb_init(void);
/****************************************************/


/********************sysctrl*************************/
int __attribute__((weak)) sysctrl_init(void)
{
    rt_kprintf("no sysctrl library!\n");
    return -1;
}
extern int sysctrl_init(void);
/****************************************************/

static int mpp_base_init(void)
{
    int ret = -1;

    /*cmpi(Common Manageability Programming Interface) init*/
    ret = cmpi_init();
    if(ret) {
        rt_kprintf("cmpi_init error!\n");
        //while(1);
    }

    /*log init*/
    ret = log_init();
    if(ret) {
        rt_kprintf("log_init error!\n");
        //while(1);
    }

    /*mmz(media memory zone) init*/
    ret = mmz_init(MEM_MMZ_BASE, MEM_MMZ_SIZE);
    if(ret) {
        rt_kprintf("mmz_init error!\n");
        //while(1);
    }

    /*mmz userdev init*/
    ret = mmz_userdev_init();
    if(ret) {
        rt_kprintf("mmz_userdev_init error!\n");
        //while(1);
    }

    /*sysctrl ctrl init*/
    ret = sysctrl_init();
    if(ret) {
        rt_kprintf("sysctrl_init error!\n");
        //while(1);
    }

    ret = vb_init();
    if(ret) {
        rt_kprintf("vb_init error!\n");
        //while(1);
    }

    return 0;
}

/*************media freq device**********************/
int __attribute__((weak)) media_freq_init(void)
{
    rt_kprintf("no media_freq_init library!\n");
    return -1;
}
extern int media_freq_init(void);

/*************virtual video input device*************/
int __attribute__((weak)) vvi_init(void)
{
    rt_kprintf("no vvi_init library!\n");
    return -1;
}
extern int vvi_init(void);
/****************************************************/

/*************virtual video input device*************/
int __attribute__((weak)) vvo_init(void)
{
    rt_kprintf("no vvo_init library!\n");
    return -1;
}
extern int vvo_init(void);
/****************************************************/

/*********************dma device*********************/
int __attribute__((weak)) dma_init(void)
{
    rt_kprintf("no dma_init library!\n");
    return -1;
}
extern int dma_init(void);
/****************************************************/

/*************video encoder device*************/
int __attribute__((weak)) venc_init(void)
{
    rt_kprintf("no venc_init library!\n");
    return -1;
}
extern int venc_init(void);
/****************************************************/

/*************video decoder device*************/
int __attribute__((weak)) vdec_init(void)
{
    rt_kprintf("no vdec_init library!\n");
    return -1;
}
extern int vdec_init(void);
/****************************************************/

/*************ai input device*************/
int __attribute__((weak)) ai_init(void)
{
    rt_kprintf("no ai_init library!\n");
    return -1;
}
extern int ai_init(void);
/****************************************************/

/*************ao input device*************/
int __attribute__((weak)) ao_init(void)
{
    rt_kprintf("no ao_init library!\n");
    return -1;
}
extern int ao_init(void);

/*************audio encode device*************/
int __attribute__((weak)) aenc_init(void)
{
    rt_kprintf("no aenc_init library!\n");
    return -1;
}
extern int aenc_init(void);
/****************************************************/

/*************audio decoder device*************/
int __attribute__((weak)) adec_init(void)
{
    rt_kprintf("no adec_init library!\n");
    return -1;
}
extern int adec_init(void);

/*************audio codec device*************/
int __attribute__((weak)) acodec_init(void)
{
    rt_kprintf("no adec_init library!\n");
    return -1;
}
extern int acodec_init(void);

/************* video output device*************/
int __attribute__((weak)) kd_vo_init(void)
{
    rt_kprintf("no kd_vo_init library!\n");
    return -1;
}
extern int kd_vo_init(void);

/************* video output device*************/
int __attribute__((weak)) connector_device_init(void)
{
    rt_kprintf("no kd_vo_init library!\n");
    return -1;
}
extern int connector_device_init(void);

/*********************dpu device*********************/
int __attribute__((weak)) dpu_init(void)
{
    rt_kprintf("no dpu_init library!\n");
    return -1;
}
extern int dpu_init(void);
/****************************************************/

/****************************************************/

/************* vdss device*************/
int __attribute__((weak)) vdss_init(void)
{
	rt_kprintf("no vdss_init library!\n");
    return -1;
}
extern int vdss_init(void);
/*************vicap device*************/
int __attribute__((weak)) vicap_init(void)
{
    rt_kprintf("no vicap_init library!\n");
    return -1;
}
extern int vicap_init(void);
/****************************************************/
int __attribute__((weak)) dewarp_init(void)
{
    rt_kprintf("no dewarp_init library!\n");
    return -1;
}
extern int dewarp_init(void);
/****************************************************/

/*************pm device*************/
int __attribute__((weak)) pm_core_init(void)
{
    rt_kprintf("no pm_core_init library!\n");
    return -1;
}
extern int pm_core_init(void);
/****************************************************/

/*************fft device*************/
int __attribute__((weak)) fft_device_init(void)
{
    rt_kprintf("no fft_device_init library!\n");
    return -1;
}
extern int fft_device_init(void);
/****************************************************/

/*************nonai_2d device*************/
int __attribute__((weak)) nonai_2d_init(void)
{
    rt_kprintf("no nonai_2d_init library!\n");
    return -1;
}
extern int nonai_2d_init(void);
/****************************************************/

static int mpp_device_init(void)
{
    media_freq_init();
    vicap_init();
    vvi_init();
    vvo_init();
    dma_init();
    venc_init();
    vdec_init();
    ai_init();
    ao_init();
    aenc_init();
    adec_init();
    acodec_init();
    kd_vo_init();
    connector_device_init();
    vdss_init();
    dpu_init();
    dewarp_init();
    pm_core_init();
    fft_device_init();
    nonai_2d_init();
    return 0;
}

int mpp_init(void)
{
    mpp_base_init();
    return mpp_device_init();
}
