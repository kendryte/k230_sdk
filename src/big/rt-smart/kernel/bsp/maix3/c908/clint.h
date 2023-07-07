#ifndef CLINT_H__
#define CLINT_H__

void clint_timer_cmp_set_val(unsigned long val);
void clint_soft_irq_clear(void);
void clint_soft_irq_init(void);

void clint_timer_init(void);

#define CLINT_SSIP0_OFFSET  (0xc000)
#define CLINT_STIMECMPL0    (0xd000)
#define CLINT_STIMECMPH0    (0xd004)

#define CLINT                    (0xF04000000UL)

#define CLINT_MTIMECMPL(hartid)  (CLINT + 0x4000 + 4*(hartid))
#define CLINT_MTIMECMPH(hartid)  (CLINT + 0x4004 + 4*(hartid))

#define CLINT_STIMECMPL(hartid)  (CLINT + CLINT_STIMECMPL0 + 4*(hartid))
#define CLINT_STIMECMPH(hartid)  (CLINT + CLINT_STIMECMPH0 + 4*(hartid))

#endif
