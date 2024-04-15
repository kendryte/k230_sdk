/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DT_K230_EVB_H
#define DT_K230_EVB_H

#define K230_MSC_3V3 0
#define K230_MSC_1V8 1

// #define BANK_VOLTAGE_IO0_IO1       K230_MSC_1V8  // FIXED
// #define BANK_VOLTAGE_IO2_IO13      K230_MSC_1V8
// #define BANK_VOLTAGE_IO14_IO25     K230_MSC_1V8
// #define BANK_VOLTAGE_IO26_IO37     K230_MSC_1V8
// #define BANK_VOLTAGE_IO38_IO49     K230_MSC_1V8
// #define BANK_VOLTAGE_IO50_IO61     K230_MSC_3V3
// #define BANK_VOLTAGE_IO62_IO63     K230_MSC_1V8

#define ST   (0)
#define DS   (1)
#define PD   (5)
#define PU   (6)
#define OE   (7)
#define IE   (8)
#define MSC  (9)
#define SL   (10)
#define SEL  (11)  //function select

#define IO0  (0*4)
#define IO1  (1*4)
#define IO2  (2*4)
#define IO3  (3*4)
#define IO4  (4*4)
#define IO5  (5*4)
#define IO6  (6*4)
#define IO7  (7*4)
#define IO8  (8*4)
#define IO9  (9*4)
#define IO10  (10*4)
#define IO11  (11*4)
#define IO12  (12*4)
#define IO13  (13*4)
#define IO14  (14*4)
#define IO15  (15*4)
#define IO16  (16*4)
#define IO17  (17*4)
#define IO18  (18*4)
#define IO19  (19*4)
#define IO20  (20*4)
#define IO21  (21*4)
#define IO22  (22*4)
#define IO23  (23*4)
#define IO24  (24*4)
#define IO25  (25*4)
#define IO26  (26*4)
#define IO27  (27*4)
#define IO28  (28*4)
#define IO29  (29*4)
#define IO30  (30*4)
#define IO31  (31*4)
#define IO32  (32*4)
#define IO33  (33*4)
#define IO34  (34*4)
#define IO35  (35*4)
#define IO36  (36*4)
#define IO37  (37*4)
#define IO38  (38*4)
#define IO39  (39*4)
#define IO40  (40*4)
#define IO41  (41*4)
#define IO42  (42*4)
#define IO43  (43*4)
#define IO44  (44*4)
#define IO45  (45*4)
#define IO46  (46*4)
#define IO47  (47*4)
#define IO48  (48*4)
#define IO49  (49*4)
#define IO50  (50*4)
#define IO51  (51*4)
#define IO52  (52*4)
#define IO53  (53*4)
#define IO54  (54*4)
#define IO55  (55*4)
#define IO56  (56*4)
#define IO57  (57*4)
#define IO58  (58*4)
#define IO59  (59*4)
#define IO60  (60*4)
#define IO61  (61*4)
#define IO62  (62*4)
#define IO63  (63*4)

#define IO64  (0*4)
#define IO65  (1*4)
#define IO66  (2*4)
#define IO67  (3*4)
#define IO68  (4*4)
#define IO69  (5*4)
#define IO70  (6*4)
#define IO71  (7*4)

#define PMU_IOSEL_INT   0x2
#define PMU_IOSEL_GPIO  0x1

#endif /* DT_K230_EVB_H */
