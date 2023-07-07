/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-14     JasonHu      first version
 */

#include <rtthread.h>
#include <rthw.h>
#include <rtdbg.h>
#include <rtconfig.h>

#include <interrupt.h>
#include <stackframe.h>
#include <backtrace.h>
#include <pic.h>
#include <lwp_arch.h>

#ifdef RT_USING_SIGNALS
#include <lwp_signal.h>
#endif /* RT_USING_SIGNALS */

enum HW_EXCEPTION_TYPE
{
    HW_EXCEPT_DIVIDE = 0,                          /* Division error: DIV and IDIV instructions */
    HW_EXCEPT_DEBUG,                               /* Debugging exceptions: access to any code and data */
    HW_EXCEPT_INTERRUPT,                           /* Unshielded interrupt: Unshielded external interrupt */
    HW_EXCEPT_BREAKPOINT,                          /* Debug breakpoint: instruction INT3 */
    HW_EXCEPT_OVERFLOW,                            /* Overflow: instruction INTO */
    HW_EXCEPT_BOUND_RANGE,                         /* Out of bounds: command BOUND */
    HW_EXCEPT_INVALID_OPCODE,                      /* Invalid (undefined) opcode: 
                                                      instruction UD2 or invalid instruction */
    HW_EXCEPT_DEVICE_NOT_AVAILABLE,                /* Device unavailable (no math processor): 
                                                      floating point or WAIT/FWAIT instructions */
    HW_EXCEPT_DOUBLE_FAULT,                        /* Double error: all instructions that can generate an exception 
                                                      or NMI or INTR */
    HW_EXCEPT_COPROCESSOR_SEGMENT_OVERRUN,         /* Assist the processor segment to cross the boundary: 
                                                      floating-point instructions (IA32 processors after 386 
                                                      no longer generate such exceptions) */
    HW_EXCEPT_INVALID_TSS,                         /* Invalid TSS: When switching tasks or accessing TSS */
    HW_EXCEPT_SEGMENT_NOT_PRESENT,                 /* Segment does not exist: when loading segment registers 
                                                      or accessing system segments */
    HW_EXCEPT_STACK_FAULT,                         /* Stack segmentation error: stack operation or loading SS */
    HW_EXCEPT_GENERAL_PROTECTION,                  /* General protection error: memory or other protection check */
    HW_EXCEPT_PAGE_FAULT,                          /* Page fault: memory access */
    HW_EXCEPT_RESERVED,                            /* INTEL reserved, not used */
    HW_EXCEPT_X87_FLOAT_POINT,                     /* X87FPU floating point error (math error): 
                                                      X87FPU floating point instruction or WAIT/FWAIIT instruction */
    HW_EXCEPT_ALIGNMENT_CHECK,                     /* Alignment check: data access in memory (supported from 486) */
    HW_EXCEPT_MACHINE_CHECK,                       /* Machine Check: The error code (if any) and source 
                                                      depend on the specific mode (Pentium CPU starts to support) */
    HW_EXCEPT_SIMD_FLOAT_POINT,                    /* SIMD floating-point exceptions: SSE and SSE2 floating-point 
                                                      instructions (supported by Pentium III) */
};

typedef void (*rt_hw_intr_handler_t)(rt_hw_stack_frame_t *);

static rt_hw_intr_handler_t interrupt_handlers[MAX_INTR_NR] = {0};
static struct rt_irq_desc irq_desc[MAX_IRQ_NR] = {0};

static char *hw_exception_names[] = {
    "#DE Divide Error",
    "#DB Debug Exception",
    "NMI Interrupt",
    "#BP Breakpoint Exception",
    "#OF Overflow Exception",
    "#BR BOUND Range Exceeded Exception",
    "#UD Invalid Opcode Exception",
    "#NM Device Not Available Exception",
    "#DF Double Fault Exception",
    "Coprocessor Segment Overrun",
    "#TS Invalid TSS Exception",
    "#NP Segment Not Present",
    "#SS Stack Fault Exception",
    "#GP General Protection Exception",
    "#PF Page-Fault Exception",
    "Reserved",
    "#MF x87 FPU Floating-Point Error",
    "#AC Alignment Check Exception",
    "#MC Machine-Check Exception",
    "#XF SIMD Floating-Point Exception",
    "Unknown Exception"
};

static void exception_frame_dump(rt_hw_stack_frame_t *frame);

static void rt_hw_interrupt_handle(int vector, void *param)
{
    rt_kprintf("UN-handled interrupt %d occurred!!!\n", vector);
}

static void hw_general_handler(rt_hw_stack_frame_t *frame)
{
    rt_kprintf("general intr %d handled\n", frame->vec_no);
}

static void hw_external_handler(rt_hw_stack_frame_t *frame)
{
    int irqno = frame->vec_no - IRQ_INTR_BASE;
    if (irqno < 0 || irqno >= MAX_IRQ_NR)
    {
        dbg_log(DBG_ERROR, "unknown IRQ %d occurred!!\n", irqno);
        return;
    }
    irq_desc[irqno].handler(irqno, irq_desc[irqno].param);
    rt_hw_pic_ack(irqno);
}

#ifdef RT_USING_LWP
static int check_user_stack(rt_hw_stack_frame_t *frame)
{
    if (frame->vec_no == EXCEPTION_PAGE_FAULT)
    {
        void *fault_addr = (void *)read_cr2();  // get page fault addr
        rt_interrupt_leave();
        if (arch_expand_user_stack(fault_addr))
        {
            rt_interrupt_enter();
            return 1;
        }
        rt_interrupt_enter();
    }
    return 0;
}
#endif  /* RT_USING_LWP */

static void hw_exception_handler(rt_hw_stack_frame_t *frame)
{
#ifdef RT_USING_LWP
    if (check_user_stack(frame))
        return;
#endif  /* RT_USING_LWP */
    rt_thread_t cur = rt_thread_self();
    rt_kprintf("thread name: %s\n", cur->name);

#ifdef RT_USING_LWP
    if (cur->lwp)
    {
        struct rt_lwp *lwp = cur->lwp;
        rt_kprintf("thread id:%d\n", lwp->pid);
    }
#endif  /* RT_USING_LWP */

    exception_frame_dump(frame);
    rt_hw_print_backtrace();

#ifdef RT_USING_SIGNALS
    dbg_log(DBG_ERROR, "[exception] send signal to thread %s\n", rt_thread_self()->name);
    /* send signal to thread */
    switch (frame->vec_no)
    {
    case HW_EXCEPT_DIVIDE:
    case HW_EXCEPT_INVALID_OPCODE:
        lwp_thread_kill(rt_thread_self(), SIGILL);
        return;
    case HW_EXCEPT_DEVICE_NOT_AVAILABLE:
        lwp_thread_kill(rt_thread_self(), SIGIO);
        return;
    case HW_EXCEPT_COPROCESSOR_SEGMENT_OVERRUN:
    case HW_EXCEPT_X87_FLOAT_POINT:
    case HW_EXCEPT_SIMD_FLOAT_POINT:
        lwp_thread_kill(rt_thread_self(), SIGFPE);
        return;
    case HW_EXCEPT_OVERFLOW:
    case HW_EXCEPT_BOUND_RANGE:
    case HW_EXCEPT_INVALID_TSS:
    case HW_EXCEPT_ALIGNMENT_CHECK:
        lwp_thread_kill(rt_thread_self(), SIGBUS);
        return;
    case HW_EXCEPT_SEGMENT_NOT_PRESENT:
    case HW_EXCEPT_GENERAL_PROTECTION:
        lwp_thread_kill(rt_thread_self(), SIGSEGV);
        return;
    case HW_EXCEPT_STACK_FAULT:
        lwp_thread_kill(rt_thread_self(), SIGSTKFLT);
        return;
    case HW_EXCEPT_MACHINE_CHECK:
    case HW_EXCEPT_INTERRUPT:
        lwp_thread_kill(rt_thread_self(), SIGINT);
        return;
    case HW_EXCEPT_DOUBLE_FAULT:
        lwp_thread_kill(rt_thread_self(), SIGKILL);
        return;
    case HW_EXCEPT_DEBUG:
    case HW_EXCEPT_BREAKPOINT:
        lwp_thread_kill(rt_thread_self(), SIGTRAP);
        return;
    default:
        break;
    }
#endif

    /* unhandled exception */
    rt_hw_interrupt_disable();
    for (;;)
    {
    }
}

rt_base_t rt_hw_interrupt_disable(void)
{
    rt_base_t level;
    __asm__ __volatile__("pushfl ; popl %0 ; cli":"=g" (level): :"memory");
    return level;
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    __asm__ __volatile__("pushl %0 ; popfl": :"g" (level):"memory", "cc");
}

void rt_hw_interrupt_dispatch(rt_hw_stack_frame_t *frame)
{
    rt_ubase_t vec_no = frame->vec_no;
    if (vec_no < 0 || vec_no >= MAX_INTR_NR)
    {
        dbg_log(DBG_ERROR, "unknown intr vector %x!\n", frame->vec_no);
        return;
    }
    interrupt_handlers[vec_no](frame);
}

void rt_hw_stack_frame_dump(rt_hw_stack_frame_t *frame)
{
    rt_kprintf("edi:%x\nesi:%x\nebp:%x\nesp dummy:%x\nebx:%x\nedx:%x\necx:%x\neax:%x\n",
        frame->edi, frame->esi, frame->ebp, frame->esp_dummy,
        frame->ebx, frame->edx, frame->ecx, frame->eax);
    rt_kprintf("gs:%x\nfs:%x\nes:%x\nds:%x\nerror code:%x\neip:%x\ncs:%x\neflags:%x\nesp:%x\nss:%x\n",
        frame->gs, frame->fs, frame->es, frame->ds, frame->error_code,
        frame->eip, frame->cs, frame->eflags, frame->esp, frame->ss);
}

static void exception_frame_dump(rt_hw_stack_frame_t *frame)
{
    rt_kprintf("\n!!! Stack frame: exception name %s\n", hw_exception_names[frame->vec_no]);
    if (frame->vec_no == 14)
    {
        rt_kprintf("page fault addr: %p\n", read_cr2());
    }
    rt_hw_stack_frame_dump(frame);
    if (frame->error_code != 0xFFFFFFFF)
    {
        if (frame->error_code & 1)
        {
            rt_kprintf("    External Event: NMI,hard interruption,ect.\n");
        }
        else
        {
            rt_kprintf("    Not External Event: inside.\n");
        }
        if (frame->error_code & (1 << 1))
        {
            rt_kprintf("    IDT: selector in idt.\n");
        }
        else
        {
            rt_kprintf("    IDT: selector in gdt or ldt.\n");
        }
        if(frame->error_code & (1 <<2 ))
        {
            rt_kprintf("    TI: selector in ldt.\n");
        }
        else
        {
            rt_kprintf("    TI: selector in gdt.\n");
        }
        rt_kprintf("    Selector: idx %d\n", (frame->error_code&0xfff8)>>3);
    }
}

/**
 * This function will mask a interrupt.
 * @param vector the interrupt number
 */
void rt_hw_interrupt_mask(int vector)
{
    rt_hw_pic_disable(vector);
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void rt_hw_interrupt_umask(int vector)
{
    rt_hw_pic_enable(vector);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
        void *param, const char *name)
{
    rt_isr_handler_t old_handler = RT_NULL;

    if(vector < MAX_IRQ_NR)
    {
        old_handler = irq_desc[vector].handler;
        if (handler != RT_NULL)
        {
            irq_desc[vector].handler = (rt_isr_handler_t)handler;
            irq_desc[vector].param = param;
#ifdef RT_USING_INTERRUPT_INFO
            rt_snprintf(irq_desc[vector].name, RT_NAME_MAX - 1, "%s", name);
            irq_desc[vector].counter = 0;
#endif
        }
    }

    return old_handler;
}

extern volatile rt_ubase_t rt_interrupt_from_thread;
extern volatile rt_ubase_t rt_interrupt_to_thread;
extern volatile rt_ubase_t rt_thread_switch_interrupt_flag;
/**
 * This function will initialize hardware interrupt
 */
void rt_hw_interrupt_init(void)
{
    rt_interrupt_from_thread = 0;
    rt_interrupt_to_thread = 0;
    rt_thread_switch_interrupt_flag = 0;
    int i;
    for (i = 0; i < MAX_INTR_NR; i++)
    {
        if (i < IRQ_INTR_BASE)
        {
            interrupt_handlers[i] = hw_exception_handler;
        }
        else if (i >= IRQ_INTR_BASE && i < IRQ_INTR_BASE + MAX_IRQ_NR)
        {
            interrupt_handlers[i] = hw_external_handler;
        }
        else
        {
            interrupt_handlers[i] = hw_general_handler;
        }
    }
    for (i = 0; i < MAX_IRQ_NR; i++)
    {
        irq_desc[i].handler = rt_hw_interrupt_handle;
        irq_desc[i].param = RT_NULL;
#ifdef RT_USING_INTERRUPT_INFO
        rt_snprintf(irq_desc[i].name, RT_NAME_MAX - 1, "default");
        irq_desc[i].counter = 0;
#endif
    }
    /* init intr controller */
    rt_hw_pic_init();
}
