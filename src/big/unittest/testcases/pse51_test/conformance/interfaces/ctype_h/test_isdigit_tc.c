#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))

void t_isdigit_0x00()
{
    uassert_int_equal(isdigit(0), 0); /* isdigit should be 0 for 0x00 */
}

void t_isdigit_0x01()
{
    uassert_int_equal(isdigit(1), 0); /* isdigit should be 0 for 0x01 */
}

void t_isdigit_0x02()
{
    uassert_int_equal(isdigit(2), 0); /* isdigit should be 0 for 0x02 */
}

void t_isdigit_0x03()
{
    uassert_int_equal(isdigit(3), 0); /* isdigit should be 0 for 0x03 */
}

void t_isdigit_0x04()
{
    uassert_int_equal(isdigit(4), 0); /* isdigit should be 0 for 0x04 */
}

void t_isdigit_0x05()
{
    uassert_int_equal(isdigit(5), 0); /* isdigit should be 0 for 0x05 */
}

void t_isdigit_0x06()
{
    uassert_int_equal(isdigit(6), 0); /* isdigit should be 0 for 0x06 */
}

void t_isdigit_0x07()
{
    uassert_int_equal(isdigit(7), 0); /* isdigit should be 0 for 0x07 */
}

void t_isdigit_0x08()
{
    uassert_int_equal(isdigit(8), 0); /* isdigit should be 0 for 0x08 */
}

void t_isdigit_0x09()
{
    uassert_int_equal(isdigit(9), 0); /* isdigit should be 0 for 0x09 */
}

void t_isdigit_0x0a()
{
    uassert_int_equal(isdigit(10), 0); /* isdigit should be 0 for 0x0a */
}

void t_isdigit_0x0b()
{
    uassert_int_equal(isdigit(11), 0); /* isdigit should be 0 for 0x0b */
}

void t_isdigit_0x0c()
{
    uassert_int_equal(isdigit(12), 0); /* isdigit should be 0 for 0x0c */
}

void t_isdigit_0x0d()
{
    uassert_int_equal(isdigit(13), 0); /* isdigit should be 0 for 0x0d */
}

void t_isdigit_0x0e()
{
    uassert_int_equal(isdigit(14), 0); /* isdigit should be 0 for 0x0e */
}

void t_isdigit_0x0f()
{
    uassert_int_equal(isdigit(15), 0); /* isdigit should be 0 for 0x0f */
}

void t_isdigit_0x10()
{
    uassert_int_equal(isdigit(16), 0); /* isdigit should be 0 for 0x10 */
}

void t_isdigit_0x11()
{
    uassert_int_equal(isdigit(17), 0); /* isdigit should be 0 for 0x11 */
}

void t_isdigit_0x12()
{
    uassert_int_equal(isdigit(18), 0); /* isdigit should be 0 for 0x12 */
}

void t_isdigit_0x13()
{
    uassert_int_equal(isdigit(19), 0); /* isdigit should be 0 for 0x13 */
}

void t_isdigit_0x14()
{
    uassert_int_equal(isdigit(20), 0); /* isdigit should be 0 for 0x14 */
}

void t_isdigit_0x15()
{
    uassert_int_equal(isdigit(21), 0); /* isdigit should be 0 for 0x15 */
}

void t_isdigit_0x16()
{
    uassert_int_equal(isdigit(22), 0); /* isdigit should be 0 for 0x16 */
}

void t_isdigit_0x17()
{
    uassert_int_equal(isdigit(23), 0); /* isdigit should be 0 for 0x17 */
}

void t_isdigit_0x18()
{
    uassert_int_equal(isdigit(24), 0); /* isdigit should be 0 for 0x18 */
}

void t_isdigit_0x19()
{
    uassert_int_equal(isdigit(25), 0); /* isdigit should be 0 for 0x19 */
}

void t_isdigit_0x1a()
{
    uassert_int_equal(isdigit(26), 0); /* isdigit should be 0 for 0x1a */
}

void t_isdigit_0x1b()
{
    uassert_int_equal(isdigit(27), 0); /* isdigit should be 0 for 0x1b */
}

void t_isdigit_0x1c()
{
    uassert_int_equal(isdigit(28), 0); /* isdigit should be 0 for 0x1c */
}

void t_isdigit_0x1d()
{
    uassert_int_equal(isdigit(29), 0); /* isdigit should be 0 for 0x1d */
}

void t_isdigit_0x1e()
{
    uassert_int_equal(isdigit(30), 0); /* isdigit should be 0 for 0x1e */
}

void t_isdigit_0x1f()
{
    uassert_int_equal(isdigit(31), 0); /* isdigit should be 0 for 0x1f */
}

void t_isdigit_0x20()
{
    uassert_int_equal(isdigit(32), 0); /* isdigit should be 0 for   */
}

void t_isdigit_0x21()
{
    uassert_int_equal(isdigit(33), 0); /* isdigit should be 0 for ! */
}

void t_isdigit_0x22()
{
    uassert_int_equal(isdigit(34), 0); /* isdigit should be 0 for 0x22 */
}

void t_isdigit_0x23()
{
    uassert_int_equal(isdigit(35), 0); /* isdigit should be 0 for # */
}

void t_isdigit_0x24()
{
    uassert_int_equal(isdigit(36), 0); /* isdigit should be 0 for $ */
}

void t_isdigit_0x25()
{
    uassert_int_equal(isdigit(37), 0); /* isdigit should be 0 for % */
}

void t_isdigit_0x26()
{
    uassert_int_equal(isdigit(38), 0); /* isdigit should be 0 for & */
}

void t_isdigit_0x27()
{
    uassert_int_equal(isdigit(39), 0); /* isdigit should be 0 for ' */
}

void t_isdigit_0x28()
{
    uassert_int_equal(isdigit(40), 0); /* isdigit should be 0 for ( */
}

void t_isdigit_0x29()
{
    uassert_int_equal(isdigit(41), 0); /* isdigit should be 0 for ) */
}

void t_isdigit_0x2a()
{
    uassert_int_equal(isdigit(42), 0); /* isdigit should be 0 for * */
}

void t_isdigit_0x2b()
{
    uassert_int_equal(isdigit(43), 0); /* isdigit should be 0 for + */
}

void t_isdigit_0x2c()
{
    uassert_int_equal(isdigit(44), 0); /* isdigit should be 0 for , */
}

void t_isdigit_0x2d()
{
    uassert_int_equal(isdigit(45), 0); /* isdigit should be 0 for - */
}

void t_isdigit_0x2e()
{
    uassert_int_equal(isdigit(46), 0); /* isdigit should be 0 for . */
}

void t_isdigit_0x2f()
{
    uassert_int_equal(isdigit(47), 0); /* isdigit should be 0 for / */
}

void t_isdigit_0x30()
{
    uassert_int_equal(isdigit(48), 1); /* isdigit should be 1 for 0 */
}

void t_isdigit_0x31()
{
    uassert_int_equal(isdigit(49), 1); /* isdigit should be 1 for 1 */
}

void t_isdigit_0x32()
{
    uassert_int_equal(isdigit(50), 1); /* isdigit should be 1 for 2 */
}

void t_isdigit_0x33()
{
    uassert_int_equal(isdigit(51), 1); /* isdigit should be 1 for 3 */
}

void t_isdigit_0x34()
{
    uassert_int_equal(isdigit(52), 1); /* isdigit should be 1 for 4 */
}

void t_isdigit_0x35()
{
    uassert_int_equal(isdigit(53), 1); /* isdigit should be 1 for 5 */
}

void t_isdigit_0x36()
{
    uassert_int_equal(isdigit(54), 1); /* isdigit should be 1 for 6 */
}

void t_isdigit_0x37()
{
    uassert_int_equal(isdigit(55), 1); /* isdigit should be 1 for 7 */
}

void t_isdigit_0x38()
{
    uassert_int_equal(isdigit(56), 1); /* isdigit should be 1 for 8 */
}

void t_isdigit_0x39()
{
    uassert_int_equal(isdigit(57), 1); /* isdigit should be 1 for 9 */
}

void t_isdigit_0x3a()
{
    uassert_int_equal(isdigit(58), 0); /* isdigit should be 0 for : */
}

void t_isdigit_0x3b()
{
    uassert_int_equal(isdigit(59), 0); /* isdigit should be 0 for ; */
}

void t_isdigit_0x3c()
{
    uassert_int_equal(isdigit(60), 0); /* isdigit should be 0 for < */
}

void t_isdigit_0x3d()
{
    uassert_int_equal(isdigit(61), 0); /* isdigit should be 0 for = */
}

void t_isdigit_0x3e()
{
    uassert_int_equal(isdigit(62), 0); /* isdigit should be 0 for > */
}

void t_isdigit_0x3f()
{
    uassert_int_equal(isdigit(63), 0); /* isdigit should be 0 for ? */
}

void t_isdigit_0x40()
{
    uassert_int_equal(isdigit(64), 0); /* isdigit should be 0 for @ */
}

void t_isdigit_0x41()
{
    uassert_int_equal(isdigit(65), 0); /* isdigit should be 0 for A */
}

void t_isdigit_0x42()
{
    uassert_int_equal(isdigit(66), 0); /* isdigit should be 0 for B */
}

void t_isdigit_0x43()
{
    uassert_int_equal(isdigit(67), 0); /* isdigit should be 0 for C */
}

void t_isdigit_0x44()
{
    uassert_int_equal(isdigit(68), 0); /* isdigit should be 0 for D */
}

void t_isdigit_0x45()
{
    uassert_int_equal(isdigit(69), 0); /* isdigit should be 0 for E */
}

void t_isdigit_0x46()
{
    uassert_int_equal(isdigit(70), 0); /* isdigit should be 0 for F */
}

void t_isdigit_0x47()
{
    uassert_int_equal(isdigit(71), 0); /* isdigit should be 0 for G */
}

void t_isdigit_0x48()
{
    uassert_int_equal(isdigit(72), 0); /* isdigit should be 0 for H */
}

void t_isdigit_0x49()
{
    uassert_int_equal(isdigit(73), 0); /* isdigit should be 0 for I */
}

void t_isdigit_0x4a()
{
    uassert_int_equal(isdigit(74), 0); /* isdigit should be 0 for J */
}

void t_isdigit_0x4b()
{
    uassert_int_equal(isdigit(75), 0); /* isdigit should be 0 for K */
}

void t_isdigit_0x4c()
{
    uassert_int_equal(isdigit(76), 0); /* isdigit should be 0 for L */
}

void t_isdigit_0x4d()
{
    uassert_int_equal(isdigit(77), 0); /* isdigit should be 0 for M */
}

void t_isdigit_0x4e()
{
    uassert_int_equal(isdigit(78), 0); /* isdigit should be 0 for N */
}

void t_isdigit_0x4f()
{
    uassert_int_equal(isdigit(79), 0); /* isdigit should be 0 for O */
}

void t_isdigit_0x50()
{
    uassert_int_equal(isdigit(80), 0); /* isdigit should be 0 for P */
}

void t_isdigit_0x51()
{
    uassert_int_equal(isdigit(81), 0); /* isdigit should be 0 for Q */
}

void t_isdigit_0x52()
{
    uassert_int_equal(isdigit(82), 0); /* isdigit should be 0 for R */
}

void t_isdigit_0x53()
{
    uassert_int_equal(isdigit(83), 0); /* isdigit should be 0 for S */
}

void t_isdigit_0x54()
{
    uassert_int_equal(isdigit(84), 0); /* isdigit should be 0 for T */
}

void t_isdigit_0x55()
{
    uassert_int_equal(isdigit(85), 0); /* isdigit should be 0 for U */
}

void t_isdigit_0x56()
{
    uassert_int_equal(isdigit(86), 0); /* isdigit should be 0 for V */
}

void t_isdigit_0x57()
{
    uassert_int_equal(isdigit(87), 0); /* isdigit should be 0 for W */
}

void t_isdigit_0x58()
{
    uassert_int_equal(isdigit(88), 0); /* isdigit should be 0 for X */
}

void t_isdigit_0x59()
{
    uassert_int_equal(isdigit(89), 0); /* isdigit should be 0 for Y */
}

void t_isdigit_0x5a()
{
    uassert_int_equal(isdigit(90), 0); /* isdigit should be 0 for Z */
}

void t_isdigit_0x5b()
{
    uassert_int_equal(isdigit(91), 0); /* isdigit should be 0 for [ */
}

void t_isdigit_0x5c()
{
    uassert_int_equal(isdigit(92), 0); /* isdigit should be 0 for 0x5c */
}

void t_isdigit_0x5d()
{
    uassert_int_equal(isdigit(93), 0); /* isdigit should be 0 for ] */
}

void t_isdigit_0x5e()
{
    uassert_int_equal(isdigit(94), 0); /* isdigit should be 0 for ^ */
}

void t_isdigit_0x5f()
{
    uassert_int_equal(isdigit(95), 0); /* isdigit should be 0 for _ */
}

void t_isdigit_0x60()
{
    uassert_int_equal(isdigit(96), 0); /* isdigit should be 0 for ` */
}

void t_isdigit_0x61()
{
    uassert_int_equal(isdigit(97), 0); /* isdigit should be 0 for a */
}

void t_isdigit_0x62()
{
    uassert_int_equal(isdigit(98), 0); /* isdigit should be 0 for b */
}

void t_isdigit_0x63()
{
    uassert_int_equal(isdigit(99), 0); /* isdigit should be 0 for c */
}

void t_isdigit_0x64()
{
    uassert_int_equal(isdigit(100), 0); /* isdigit should be 0 for d */
}

void t_isdigit_0x65()
{
    uassert_int_equal(isdigit(101), 0); /* isdigit should be 0 for e */
}

void t_isdigit_0x66()
{
    uassert_int_equal(isdigit(102), 0); /* isdigit should be 0 for f */
}

void t_isdigit_0x67()
{
    uassert_int_equal(isdigit(103), 0); /* isdigit should be 0 for g */
}

void t_isdigit_0x68()
{
    uassert_int_equal(isdigit(104), 0); /* isdigit should be 0 for h */
}

void t_isdigit_0x69()
{
    uassert_int_equal(isdigit(105), 0); /* isdigit should be 0 for i */
}

void t_isdigit_0x6a()
{
    uassert_int_equal(isdigit(106), 0); /* isdigit should be 0 for j */
}

void t_isdigit_0x6b()
{
    uassert_int_equal(isdigit(107), 0); /* isdigit should be 0 for k */
}

void t_isdigit_0x6c()
{
    uassert_int_equal(isdigit(108), 0); /* isdigit should be 0 for l */
}

void t_isdigit_0x6d()
{
    uassert_int_equal(isdigit(109), 0); /* isdigit should be 0 for m */
}

void t_isdigit_0x6e()
{
    uassert_int_equal(isdigit(110), 0); /* isdigit should be 0 for n */
}

void t_isdigit_0x6f()
{
    uassert_int_equal(isdigit(111), 0); /* isdigit should be 0 for o */
}

void t_isdigit_0x70()
{
    uassert_int_equal(isdigit(112), 0); /* isdigit should be 0 for p */
}

void t_isdigit_0x71()
{
    uassert_int_equal(isdigit(113), 0); /* isdigit should be 0 for q */
}

void t_isdigit_0x72()
{
    uassert_int_equal(isdigit(114), 0); /* isdigit should be 0 for r */
}

void t_isdigit_0x73()
{
    uassert_int_equal(isdigit(115), 0); /* isdigit should be 0 for s */
}

void t_isdigit_0x74()
{
    uassert_int_equal(isdigit(116), 0); /* isdigit should be 0 for t */
}

void t_isdigit_0x75()
{
    uassert_int_equal(isdigit(117), 0); /* isdigit should be 0 for u */
}

void t_isdigit_0x76()
{
    uassert_int_equal(isdigit(118), 0); /* isdigit should be 0 for v */
}

void t_isdigit_0x77()
{
    uassert_int_equal(isdigit(119), 0); /* isdigit should be 0 for w */
}

void t_isdigit_0x78()
{
    uassert_int_equal(isdigit(120), 0); /* isdigit should be 0 for x */
}

void t_isdigit_0x79()
{
    uassert_int_equal(isdigit(121), 0); /* isdigit should be 0 for y */
}

void t_isdigit_0x7a()
{
    uassert_int_equal(isdigit(122), 0); /* isdigit should be 0 for z */
}

void t_isdigit_0x7b()
{
    uassert_int_equal(isdigit(123), 0); /* isdigit should be 0 for { */
}

void t_isdigit_0x7c()
{
    uassert_int_equal(isdigit(124), 0); /* isdigit should be 0 for | */
}

void t_isdigit_0x7d()
{
    uassert_int_equal(isdigit(125), 0); /* isdigit should be 0 for } */
}

void t_isdigit_0x7e()
{
    uassert_int_equal(isdigit(126), 0); /* isdigit should be 0 for ~ */
}

void t_isdigit_0x7f()
{
    uassert_int_equal(isdigit(127), 0); /* isdigit should be 0 for 0x7f */
}

void t_isdigit_0x80()
{
    uassert_int_equal(isdigit(128), 0); /* isdigit should be 0 for 0x80 */
}

void t_isdigit_0x81()
{
    uassert_int_equal(isdigit(129), 0); /* isdigit should be 0 for 0x81 */
}

void t_isdigit_0x82()
{
    uassert_int_equal(isdigit(130), 0); /* isdigit should be 0 for 0x82 */
}

void t_isdigit_0x83()
{
    uassert_int_equal(isdigit(131), 0); /* isdigit should be 0 for 0x83 */
}

void t_isdigit_0x84()
{
    uassert_int_equal(isdigit(132), 0); /* isdigit should be 0 for 0x84 */
}

void t_isdigit_0x85()
{
    uassert_int_equal(isdigit(133), 0); /* isdigit should be 0 for 0x85 */
}

void t_isdigit_0x86()
{
    uassert_int_equal(isdigit(134), 0); /* isdigit should be 0 for 0x86 */
}

void t_isdigit_0x87()
{
    uassert_int_equal(isdigit(135), 0); /* isdigit should be 0 for 0x87 */
}

void t_isdigit_0x88()
{
    uassert_int_equal(isdigit(136), 0); /* isdigit should be 0 for 0x88 */
}

void t_isdigit_0x89()
{
    uassert_int_equal(isdigit(137), 0); /* isdigit should be 0 for 0x89 */
}

void t_isdigit_0x8a()
{
    uassert_int_equal(isdigit(138), 0); /* isdigit should be 0 for 0x8a */
}

void t_isdigit_0x8b()
{
    uassert_int_equal(isdigit(139), 0); /* isdigit should be 0 for 0x8b */
}

void t_isdigit_0x8c()
{
    uassert_int_equal(isdigit(140), 0); /* isdigit should be 0 for 0x8c */
}

void t_isdigit_0x8d()
{
    uassert_int_equal(isdigit(141), 0); /* isdigit should be 0 for 0x8d */
}

void t_isdigit_0x8e()
{
    uassert_int_equal(isdigit(142), 0); /* isdigit should be 0 for 0x8e */
}

void t_isdigit_0x8f()
{
    uassert_int_equal(isdigit(143), 0); /* isdigit should be 0 for 0x8f */
}

void t_isdigit_0x90()
{
    uassert_int_equal(isdigit(144), 0); /* isdigit should be 0 for 0x90 */
}

void t_isdigit_0x91()
{
    uassert_int_equal(isdigit(145), 0); /* isdigit should be 0 for 0x91 */
}

void t_isdigit_0x92()
{
    uassert_int_equal(isdigit(146), 0); /* isdigit should be 0 for 0x92 */
}

void t_isdigit_0x93()
{
    uassert_int_equal(isdigit(147), 0); /* isdigit should be 0 for 0x93 */
}

void t_isdigit_0x94()
{
    uassert_int_equal(isdigit(148), 0); /* isdigit should be 0 for 0x94 */
}

void t_isdigit_0x95()
{
    uassert_int_equal(isdigit(149), 0); /* isdigit should be 0 for 0x95 */
}

void t_isdigit_0x96()
{
    uassert_int_equal(isdigit(150), 0); /* isdigit should be 0 for 0x96 */
}

void t_isdigit_0x97()
{
    uassert_int_equal(isdigit(151), 0); /* isdigit should be 0 for 0x97 */
}

void t_isdigit_0x98()
{
    uassert_int_equal(isdigit(152), 0); /* isdigit should be 0 for 0x98 */
}

void t_isdigit_0x99()
{
    uassert_int_equal(isdigit(153), 0); /* isdigit should be 0 for 0x99 */
}

void t_isdigit_0x9a()
{
    uassert_int_equal(isdigit(154), 0); /* isdigit should be 0 for 0x9a */
}

void t_isdigit_0x9b()
{
    uassert_int_equal(isdigit(155), 0); /* isdigit should be 0 for 0x9b */
}

void t_isdigit_0x9c()
{
    uassert_int_equal(isdigit(156), 0); /* isdigit should be 0 for 0x9c */
}

void t_isdigit_0x9d()
{
    uassert_int_equal(isdigit(157), 0); /* isdigit should be 0 for 0x9d */
}

void t_isdigit_0x9e()
{
    uassert_int_equal(isdigit(158), 0); /* isdigit should be 0 for 0x9e */
}

void t_isdigit_0x9f()
{
    uassert_int_equal(isdigit(159), 0); /* isdigit should be 0 for 0x9f */
}

void t_isdigit_0xa0()
{
    uassert_int_equal(isdigit(160), 0); /* isdigit should be 0 for 0xa0 */
}

void t_isdigit_0xa1()
{
    uassert_int_equal(isdigit(161), 0); /* isdigit should be 0 for 0xa1 */
}

void t_isdigit_0xa2()
{
    uassert_int_equal(isdigit(162), 0); /* isdigit should be 0 for 0xa2 */
}

void t_isdigit_0xa3()
{
    uassert_int_equal(isdigit(163), 0); /* isdigit should be 0 for 0xa3 */
}

void t_isdigit_0xa4()
{
    uassert_int_equal(isdigit(164), 0); /* isdigit should be 0 for 0xa4 */
}

void t_isdigit_0xa5()
{
    uassert_int_equal(isdigit(165), 0); /* isdigit should be 0 for 0xa5 */
}

void t_isdigit_0xa6()
{
    uassert_int_equal(isdigit(166), 0); /* isdigit should be 0 for 0xa6 */
}

void t_isdigit_0xa7()
{
    uassert_int_equal(isdigit(167), 0); /* isdigit should be 0 for 0xa7 */
}

void t_isdigit_0xa8()
{
    uassert_int_equal(isdigit(168), 0); /* isdigit should be 0 for 0xa8 */
}

void t_isdigit_0xa9()
{
    uassert_int_equal(isdigit(169), 0); /* isdigit should be 0 for 0xa9 */
}

void t_isdigit_0xaa()
{
    uassert_int_equal(isdigit(170), 0); /* isdigit should be 0 for 0xaa */
}

void t_isdigit_0xab()
{
    uassert_int_equal(isdigit(171), 0); /* isdigit should be 0 for 0xab */
}

void t_isdigit_0xac()
{
    uassert_int_equal(isdigit(172), 0); /* isdigit should be 0 for 0xac */
}

void t_isdigit_0xad()
{
    uassert_int_equal(isdigit(173), 0); /* isdigit should be 0 for 0xad */
}

void t_isdigit_0xae()
{
    uassert_int_equal(isdigit(174), 0); /* isdigit should be 0 for 0xae */
}

void t_isdigit_0xaf()
{
    uassert_int_equal(isdigit(175), 0); /* isdigit should be 0 for 0xaf */
}

void t_isdigit_0xb0()
{
    uassert_int_equal(isdigit(176), 0); /* isdigit should be 0 for 0xb0 */
}

void t_isdigit_0xb1()
{
    uassert_int_equal(isdigit(177), 0); /* isdigit should be 0 for 0xb1 */
}

void t_isdigit_0xb2()
{
    uassert_int_equal(isdigit(178), 0); /* isdigit should be 0 for 0xb2 */
}

void t_isdigit_0xb3()
{
    uassert_int_equal(isdigit(179), 0); /* isdigit should be 0 for 0xb3 */
}

void t_isdigit_0xb4()
{
    uassert_int_equal(isdigit(180), 0); /* isdigit should be 0 for 0xb4 */
}

void t_isdigit_0xb5()
{
    uassert_int_equal(isdigit(181), 0); /* isdigit should be 0 for 0xb5 */
}

void t_isdigit_0xb6()
{
    uassert_int_equal(isdigit(182), 0); /* isdigit should be 0 for 0xb6 */
}

void t_isdigit_0xb7()
{
    uassert_int_equal(isdigit(183), 0); /* isdigit should be 0 for 0xb7 */
}

void t_isdigit_0xb8()
{
    uassert_int_equal(isdigit(184), 0); /* isdigit should be 0 for 0xb8 */
}

void t_isdigit_0xb9()
{
    uassert_int_equal(isdigit(185), 0); /* isdigit should be 0 for 0xb9 */
}

void t_isdigit_0xba()
{
    uassert_int_equal(isdigit(186), 0); /* isdigit should be 0 for 0xba */
}

void t_isdigit_0xbb()
{
    uassert_int_equal(isdigit(187), 0); /* isdigit should be 0 for 0xbb */
}

void t_isdigit_0xbc()
{
    uassert_int_equal(isdigit(188), 0); /* isdigit should be 0 for 0xbc */
}

void t_isdigit_0xbd()
{
    uassert_int_equal(isdigit(189), 0); /* isdigit should be 0 for 0xbd */
}

void t_isdigit_0xbe()
{
    uassert_int_equal(isdigit(190), 0); /* isdigit should be 0 for 0xbe */
}

void t_isdigit_0xbf()
{
    uassert_int_equal(isdigit(191), 0); /* isdigit should be 0 for 0xbf */
}

void t_isdigit_0xc0()
{
    uassert_int_equal(isdigit(192), 0); /* isdigit should be 0 for 0xc0 */
}

void t_isdigit_0xc1()
{
    uassert_int_equal(isdigit(193), 0); /* isdigit should be 0 for 0xc1 */
}

void t_isdigit_0xc2()
{
    uassert_int_equal(isdigit(194), 0); /* isdigit should be 0 for 0xc2 */
}

void t_isdigit_0xc3()
{
    uassert_int_equal(isdigit(195), 0); /* isdigit should be 0 for 0xc3 */
}

void t_isdigit_0xc4()
{
    uassert_int_equal(isdigit(196), 0); /* isdigit should be 0 for 0xc4 */
}

void t_isdigit_0xc5()
{
    uassert_int_equal(isdigit(197), 0); /* isdigit should be 0 for 0xc5 */
}

void t_isdigit_0xc6()
{
    uassert_int_equal(isdigit(198), 0); /* isdigit should be 0 for 0xc6 */
}

void t_isdigit_0xc7()
{
    uassert_int_equal(isdigit(199), 0); /* isdigit should be 0 for 0xc7 */
}

void t_isdigit_0xc8()
{
    uassert_int_equal(isdigit(200), 0); /* isdigit should be 0 for 0xc8 */
}

void t_isdigit_0xc9()
{
    uassert_int_equal(isdigit(201), 0); /* isdigit should be 0 for 0xc9 */
}

void t_isdigit_0xca()
{
    uassert_int_equal(isdigit(202), 0); /* isdigit should be 0 for 0xca */
}

void t_isdigit_0xcb()
{
    uassert_int_equal(isdigit(203), 0); /* isdigit should be 0 for 0xcb */
}

void t_isdigit_0xcc()
{
    uassert_int_equal(isdigit(204), 0); /* isdigit should be 0 for 0xcc */
}

void t_isdigit_0xcd()
{
    uassert_int_equal(isdigit(205), 0); /* isdigit should be 0 for 0xcd */
}

void t_isdigit_0xce()
{
    uassert_int_equal(isdigit(206), 0); /* isdigit should be 0 for 0xce */
}

void t_isdigit_0xcf()
{
    uassert_int_equal(isdigit(207), 0); /* isdigit should be 0 for 0xcf */
}

void t_isdigit_0xd0()
{
    uassert_int_equal(isdigit(208), 0); /* isdigit should be 0 for 0xd0 */
}

void t_isdigit_0xd1()
{
    uassert_int_equal(isdigit(209), 0); /* isdigit should be 0 for 0xd1 */
}

void t_isdigit_0xd2()
{
    uassert_int_equal(isdigit(210), 0); /* isdigit should be 0 for 0xd2 */
}

void t_isdigit_0xd3()
{
    uassert_int_equal(isdigit(211), 0); /* isdigit should be 0 for 0xd3 */
}

void t_isdigit_0xd4()
{
    uassert_int_equal(isdigit(212), 0); /* isdigit should be 0 for 0xd4 */
}

void t_isdigit_0xd5()
{
    uassert_int_equal(isdigit(213), 0); /* isdigit should be 0 for 0xd5 */
}

void t_isdigit_0xd6()
{
    uassert_int_equal(isdigit(214), 0); /* isdigit should be 0 for 0xd6 */
}

void t_isdigit_0xd7()
{
    uassert_int_equal(isdigit(215), 0); /* isdigit should be 0 for 0xd7 */
}

void t_isdigit_0xd8()
{
    uassert_int_equal(isdigit(216), 0); /* isdigit should be 0 for 0xd8 */
}

void t_isdigit_0xd9()
{
    uassert_int_equal(isdigit(217), 0); /* isdigit should be 0 for 0xd9 */
}

void t_isdigit_0xda()
{
    uassert_int_equal(isdigit(218), 0); /* isdigit should be 0 for 0xda */
}

void t_isdigit_0xdb()
{
    uassert_int_equal(isdigit(219), 0); /* isdigit should be 0 for 0xdb */
}

void t_isdigit_0xdc()
{
    uassert_int_equal(isdigit(220), 0); /* isdigit should be 0 for 0xdc */
}

void t_isdigit_0xdd()
{
    uassert_int_equal(isdigit(221), 0); /* isdigit should be 0 for 0xdd */
}

void t_isdigit_0xde()
{
    uassert_int_equal(isdigit(222), 0); /* isdigit should be 0 for 0xde */
}

void t_isdigit_0xdf()
{
    uassert_int_equal(isdigit(223), 0); /* isdigit should be 0 for 0xdf */
}

void t_isdigit_0xe0()
{
    uassert_int_equal(isdigit(224), 0); /* isdigit should be 0 for 0xe0 */
}

void t_isdigit_0xe1()
{
    uassert_int_equal(isdigit(225), 0); /* isdigit should be 0 for 0xe1 */
}

void t_isdigit_0xe2()
{
    uassert_int_equal(isdigit(226), 0); /* isdigit should be 0 for 0xe2 */
}

void t_isdigit_0xe3()
{
    uassert_int_equal(isdigit(227), 0); /* isdigit should be 0 for 0xe3 */
}

void t_isdigit_0xe4()
{
    uassert_int_equal(isdigit(228), 0); /* isdigit should be 0 for 0xe4 */
}

void t_isdigit_0xe5()
{
    uassert_int_equal(isdigit(229), 0); /* isdigit should be 0 for 0xe5 */
}

void t_isdigit_0xe6()
{
    uassert_int_equal(isdigit(230), 0); /* isdigit should be 0 for 0xe6 */
}

void t_isdigit_0xe7()
{
    uassert_int_equal(isdigit(231), 0); /* isdigit should be 0 for 0xe7 */
}

void t_isdigit_0xe8()
{
    uassert_int_equal(isdigit(232), 0); /* isdigit should be 0 for 0xe8 */
}

void t_isdigit_0xe9()
{
    uassert_int_equal(isdigit(233), 0); /* isdigit should be 0 for 0xe9 */
}

void t_isdigit_0xea()
{
    uassert_int_equal(isdigit(234), 0); /* isdigit should be 0 for 0xea */
}

void t_isdigit_0xeb()
{
    uassert_int_equal(isdigit(235), 0); /* isdigit should be 0 for 0xeb */
}

void t_isdigit_0xec()
{
    uassert_int_equal(isdigit(236), 0); /* isdigit should be 0 for 0xec */
}

void t_isdigit_0xed()
{
    uassert_int_equal(isdigit(237), 0); /* isdigit should be 0 for 0xed */
}

void t_isdigit_0xee()
{
    uassert_int_equal(isdigit(238), 0); /* isdigit should be 0 for 0xee */
}

void t_isdigit_0xef()
{
    uassert_int_equal(isdigit(239), 0); /* isdigit should be 0 for 0xef */
}

void t_isdigit_0xf0()
{
    uassert_int_equal(isdigit(240), 0); /* isdigit should be 0 for 0xf0 */
}

void t_isdigit_0xf1()
{
    uassert_int_equal(isdigit(241), 0); /* isdigit should be 0 for 0xf1 */
}

void t_isdigit_0xf2()
{
    uassert_int_equal(isdigit(242), 0); /* isdigit should be 0 for 0xf2 */
}

void t_isdigit_0xf3()
{
    uassert_int_equal(isdigit(243), 0); /* isdigit should be 0 for 0xf3 */
}

void t_isdigit_0xf4()
{
    uassert_int_equal(isdigit(244), 0); /* isdigit should be 0 for 0xf4 */
}

void t_isdigit_0xf5()
{
    uassert_int_equal(isdigit(245), 0); /* isdigit should be 0 for 0xf5 */
}

void t_isdigit_0xf6()
{
    uassert_int_equal(isdigit(246), 0); /* isdigit should be 0 for 0xf6 */
}

void t_isdigit_0xf7()
{
    uassert_int_equal(isdigit(247), 0); /* isdigit should be 0 for 0xf7 */
}

void t_isdigit_0xf8()
{
    uassert_int_equal(isdigit(248), 0); /* isdigit should be 0 for 0xf8 */
}

void t_isdigit_0xf9()
{
    uassert_int_equal(isdigit(249), 0); /* isdigit should be 0 for 0xf9 */
}

void t_isdigit_0xfa()
{
    uassert_int_equal(isdigit(250), 0); /* isdigit should be 0 for 0xfa */
}

void t_isdigit_0xfb()
{
    uassert_int_equal(isdigit(251), 0); /* isdigit should be 0 for 0xfb */
}

void t_isdigit_0xfc()
{
    uassert_int_equal(isdigit(252), 0); /* isdigit should be 0 for 0xfc */
}

void t_isdigit_0xfd()
{
    uassert_int_equal(isdigit(253), 0); /* isdigit should be 0 for 0xfd */
}

void t_isdigit_0xfe()
{
    uassert_int_equal(isdigit(254), 0); /* isdigit should be 0 for 0xfe */
}

void t_isdigit_0xff()
{
    uassert_int_equal(isdigit(255), 0); /* isdigit should be 0 for 0xff */
}



static int testcase(void)
{
    t_isdigit_0x00();
    t_isdigit_0x01();
    t_isdigit_0x02();
    t_isdigit_0x03();
    t_isdigit_0x04();
    t_isdigit_0x05();
    t_isdigit_0x06();
    t_isdigit_0x07();
    t_isdigit_0x08();
    t_isdigit_0x09();
    t_isdigit_0x0a();
    t_isdigit_0x0b();
    t_isdigit_0x0c();
    t_isdigit_0x0d();
    t_isdigit_0x0e();
    t_isdigit_0x0f();
    t_isdigit_0x10();
    t_isdigit_0x11();
    t_isdigit_0x12();
    t_isdigit_0x13();
    t_isdigit_0x14();
    t_isdigit_0x15();
    t_isdigit_0x16();
    t_isdigit_0x17();
    t_isdigit_0x18();
    t_isdigit_0x19();
    t_isdigit_0x1a();
    t_isdigit_0x1b();
    t_isdigit_0x1c();
    t_isdigit_0x1d();
    t_isdigit_0x1e();
    t_isdigit_0x1f();
    t_isdigit_0x20();
    t_isdigit_0x21();
    t_isdigit_0x22();
    t_isdigit_0x23();
    t_isdigit_0x24();
    t_isdigit_0x25();
    t_isdigit_0x26();
    t_isdigit_0x27();
    t_isdigit_0x28();
    t_isdigit_0x29();
    t_isdigit_0x2a();
    t_isdigit_0x2b();
    t_isdigit_0x2c();
    t_isdigit_0x2d();
    t_isdigit_0x2e();
    t_isdigit_0x2f();
    t_isdigit_0x30();
    t_isdigit_0x31();
    t_isdigit_0x32();
    t_isdigit_0x33();
    t_isdigit_0x34();
    t_isdigit_0x35();
    t_isdigit_0x36();
    t_isdigit_0x37();
    t_isdigit_0x38();
    t_isdigit_0x39();
    t_isdigit_0x3a();
    t_isdigit_0x3b();
    t_isdigit_0x3c();
    t_isdigit_0x3d();
    t_isdigit_0x3e();
    t_isdigit_0x3f();
    t_isdigit_0x40();
    t_isdigit_0x41();
    t_isdigit_0x42();
    t_isdigit_0x43();
    t_isdigit_0x44();
    t_isdigit_0x45();
    t_isdigit_0x46();
    t_isdigit_0x47();
    t_isdigit_0x48();
    t_isdigit_0x49();
    t_isdigit_0x4a();
    t_isdigit_0x4b();
    t_isdigit_0x4c();
    t_isdigit_0x4d();
    t_isdigit_0x4e();
    t_isdigit_0x4f();
    t_isdigit_0x50();
    t_isdigit_0x51();
    t_isdigit_0x52();
    t_isdigit_0x53();
    t_isdigit_0x54();
    t_isdigit_0x55();
    t_isdigit_0x56();
    t_isdigit_0x57();
    t_isdigit_0x58();
    t_isdigit_0x59();
    t_isdigit_0x5a();
    t_isdigit_0x5b();
    t_isdigit_0x5c();
    t_isdigit_0x5d();
    t_isdigit_0x5e();
    t_isdigit_0x5f();
    t_isdigit_0x60();
    t_isdigit_0x61();
    t_isdigit_0x62();
    t_isdigit_0x63();
    t_isdigit_0x64();
    t_isdigit_0x65();
    t_isdigit_0x66();
    t_isdigit_0x67();
    t_isdigit_0x68();
    t_isdigit_0x69();
    t_isdigit_0x6a();
    t_isdigit_0x6b();
    t_isdigit_0x6c();
    t_isdigit_0x6d();
    t_isdigit_0x6e();
    t_isdigit_0x6f();
    t_isdigit_0x70();
    t_isdigit_0x71();
    t_isdigit_0x72();
    t_isdigit_0x73();
    t_isdigit_0x74();
    t_isdigit_0x75();
    t_isdigit_0x76();
    t_isdigit_0x77();
    t_isdigit_0x78();
    t_isdigit_0x79();
    t_isdigit_0x7a();
    t_isdigit_0x7b();
    t_isdigit_0x7c();
    t_isdigit_0x7d();
    t_isdigit_0x7e();
    t_isdigit_0x7f();
    t_isdigit_0x80();
    t_isdigit_0x81();
    t_isdigit_0x82();
    t_isdigit_0x83();
    t_isdigit_0x84();
    t_isdigit_0x85();
    t_isdigit_0x86();
    t_isdigit_0x87();
    t_isdigit_0x88();
    t_isdigit_0x89();
    t_isdigit_0x8a();
    t_isdigit_0x8b();
    t_isdigit_0x8c();
    t_isdigit_0x8d();
    t_isdigit_0x8e();
    t_isdigit_0x8f();
    t_isdigit_0x90();
    t_isdigit_0x91();
    t_isdigit_0x92();
    t_isdigit_0x93();
    t_isdigit_0x94();
    t_isdigit_0x95();
    t_isdigit_0x96();
    t_isdigit_0x97();
    t_isdigit_0x98();
    t_isdigit_0x99();
    t_isdigit_0x9a();
    t_isdigit_0x9b();
    t_isdigit_0x9c();
    t_isdigit_0x9d();
    t_isdigit_0x9e();
    t_isdigit_0x9f();
    t_isdigit_0xa0();
    t_isdigit_0xa1();
    t_isdigit_0xa2();
    t_isdigit_0xa3();
    t_isdigit_0xa4();
    t_isdigit_0xa5();
    t_isdigit_0xa6();
    t_isdigit_0xa7();
    t_isdigit_0xa8();
    t_isdigit_0xa9();
    t_isdigit_0xaa();
    t_isdigit_0xab();
    t_isdigit_0xac();
    t_isdigit_0xad();
    t_isdigit_0xae();
    t_isdigit_0xaf();
    t_isdigit_0xb0();
    t_isdigit_0xb1();
    t_isdigit_0xb2();
    t_isdigit_0xb3();
    t_isdigit_0xb4();
    t_isdigit_0xb5();
    t_isdigit_0xb6();
    t_isdigit_0xb7();
    t_isdigit_0xb8();
    t_isdigit_0xb9();
    t_isdigit_0xba();
    t_isdigit_0xbb();
    t_isdigit_0xbc();
    t_isdigit_0xbd();
    t_isdigit_0xbe();
    t_isdigit_0xbf();
    t_isdigit_0xc0();
    t_isdigit_0xc1();
    t_isdigit_0xc2();
    t_isdigit_0xc3();
    t_isdigit_0xc4();
    t_isdigit_0xc5();
    t_isdigit_0xc6();
    t_isdigit_0xc7();
    t_isdigit_0xc8();
    t_isdigit_0xc9();
    t_isdigit_0xca();
    t_isdigit_0xcb();
    t_isdigit_0xcc();
    t_isdigit_0xcd();
    t_isdigit_0xce();
    t_isdigit_0xcf();
    t_isdigit_0xd0();
    t_isdigit_0xd1();
    t_isdigit_0xd2();
    t_isdigit_0xd3();
    t_isdigit_0xd4();
    t_isdigit_0xd5();
    t_isdigit_0xd6();
    t_isdigit_0xd7();
    t_isdigit_0xd8();
    t_isdigit_0xd9();
    t_isdigit_0xda();
    t_isdigit_0xdb();
    t_isdigit_0xdc();
    t_isdigit_0xdd();
    t_isdigit_0xde();
    t_isdigit_0xdf();
    t_isdigit_0xe0();
    t_isdigit_0xe1();
    t_isdigit_0xe2();
    t_isdigit_0xe3();
    t_isdigit_0xe4();
    t_isdigit_0xe5();
    t_isdigit_0xe6();
    t_isdigit_0xe7();
    t_isdigit_0xe8();
    t_isdigit_0xe9();
    t_isdigit_0xea();
    t_isdigit_0xeb();
    t_isdigit_0xec();
    t_isdigit_0xed();
    t_isdigit_0xee();
    t_isdigit_0xef();
    t_isdigit_0xf0();
    t_isdigit_0xf1();
    t_isdigit_0xf2();
    t_isdigit_0xf3();
    t_isdigit_0xf4();
    t_isdigit_0xf5();
    t_isdigit_0xf6();
    t_isdigit_0xf7();
    t_isdigit_0xf8();
    t_isdigit_0xf9();
    t_isdigit_0xfa();
    t_isdigit_0xfb();
    t_isdigit_0xfc();
    t_isdigit_0xfd();
    t_isdigit_0xfe();
    t_isdigit_0xff();
}


int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
