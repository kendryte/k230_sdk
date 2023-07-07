#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))

void t_isascii_0x00()
{
    uassert_int_equal(isascii(0), 1); /* isascii should be 1 for 0x00 */
}

void t_isascii_0x01()
{
    uassert_int_equal(isascii(1), 1); /* isascii should be 1 for 0x01 */
}

void t_isascii_0x02()
{
    uassert_int_equal(isascii(2), 1); /* isascii should be 1 for 0x02 */
}

void t_isascii_0x03()
{
    uassert_int_equal(isascii(3), 1); /* isascii should be 1 for 0x03 */
}

void t_isascii_0x04()
{
    uassert_int_equal(isascii(4), 1); /* isascii should be 1 for 0x04 */
}

void t_isascii_0x05()
{
    uassert_int_equal(isascii(5), 1); /* isascii should be 1 for 0x05 */
}

void t_isascii_0x06()
{
    uassert_int_equal(isascii(6), 1); /* isascii should be 1 for 0x06 */
}

void t_isascii_0x07()
{
    uassert_int_equal(isascii(7), 1); /* isascii should be 1 for 0x07 */
}

void t_isascii_0x08()
{
    uassert_int_equal(isascii(8), 1); /* isascii should be 1 for 0x08 */
}

void t_isascii_0x09()
{
    uassert_int_equal(isascii(9), 1); /* isascii should be 1 for 0x09 */
}

void t_isascii_0x0a()
{
    uassert_int_equal(isascii(10), 1); /* isascii should be 1 for 0x0a */
}

void t_isascii_0x0b()
{
    uassert_int_equal(isascii(11), 1); /* isascii should be 1 for 0x0b */
}

void t_isascii_0x0c()
{
    uassert_int_equal(isascii(12), 1); /* isascii should be 1 for 0x0c */
}

void t_isascii_0x0d()
{
    uassert_int_equal(isascii(13), 1); /* isascii should be 1 for 0x0d */
}

void t_isascii_0x0e()
{
    uassert_int_equal(isascii(14), 1); /* isascii should be 1 for 0x0e */
}

void t_isascii_0x0f()
{
    uassert_int_equal(isascii(15), 1); /* isascii should be 1 for 0x0f */
}

void t_isascii_0x10()
{
    uassert_int_equal(isascii(16), 1); /* isascii should be 1 for 0x10 */
}

void t_isascii_0x11()
{
    uassert_int_equal(isascii(17), 1); /* isascii should be 1 for 0x11 */
}

void t_isascii_0x12()
{
    uassert_int_equal(isascii(18), 1); /* isascii should be 1 for 0x12 */
}

void t_isascii_0x13()
{
    uassert_int_equal(isascii(19), 1); /* isascii should be 1 for 0x13 */
}

void t_isascii_0x14()
{
    uassert_int_equal(isascii(20), 1); /* isascii should be 1 for 0x14 */
}

void t_isascii_0x15()
{
    uassert_int_equal(isascii(21), 1); /* isascii should be 1 for 0x15 */
}

void t_isascii_0x16()
{
    uassert_int_equal(isascii(22), 1); /* isascii should be 1 for 0x16 */
}

void t_isascii_0x17()
{
    uassert_int_equal(isascii(23), 1); /* isascii should be 1 for 0x17 */
}

void t_isascii_0x18()
{
    uassert_int_equal(isascii(24), 1); /* isascii should be 1 for 0x18 */
}

void t_isascii_0x19()
{
    uassert_int_equal(isascii(25), 1); /* isascii should be 1 for 0x19 */
}

void t_isascii_0x1a()
{
    uassert_int_equal(isascii(26), 1); /* isascii should be 1 for 0x1a */
}

void t_isascii_0x1b()
{
    uassert_int_equal(isascii(27), 1); /* isascii should be 1 for 0x1b */
}

void t_isascii_0x1c()
{
    uassert_int_equal(isascii(28), 1); /* isascii should be 1 for 0x1c */
}

void t_isascii_0x1d()
{
    uassert_int_equal(isascii(29), 1); /* isascii should be 1 for 0x1d */
}

void t_isascii_0x1e()
{
    uassert_int_equal(isascii(30), 1); /* isascii should be 1 for 0x1e */
}

void t_isascii_0x1f()
{
    uassert_int_equal(isascii(31), 1); /* isascii should be 1 for 0x1f */
}

void t_isascii_0x20()
{
    uassert_int_equal(isascii(32), 1); /* isascii should be 1 for   */
}

void t_isascii_0x21()
{
    uassert_int_equal(isascii(33), 1); /* isascii should be 1 for ! */
}

void t_isascii_0x22()
{
    uassert_int_equal(isascii(34), 1); /* isascii should be 1 for 0x22 */
}

void t_isascii_0x23()
{
    uassert_int_equal(isascii(35), 1); /* isascii should be 1 for # */
}

void t_isascii_0x24()
{
    uassert_int_equal(isascii(36), 1); /* isascii should be 1 for $ */
}

void t_isascii_0x25()
{
    uassert_int_equal(isascii(37), 1); /* isascii should be 1 for % */
}

void t_isascii_0x26()
{
    uassert_int_equal(isascii(38), 1); /* isascii should be 1 for & */
}

void t_isascii_0x27()
{
    uassert_int_equal(isascii(39), 1); /* isascii should be 1 for ' */
}

void t_isascii_0x28()
{
    uassert_int_equal(isascii(40), 1); /* isascii should be 1 for ( */
}

void t_isascii_0x29()
{
    uassert_int_equal(isascii(41), 1); /* isascii should be 1 for ) */
}

void t_isascii_0x2a()
{
    uassert_int_equal(isascii(42), 1); /* isascii should be 1 for * */
}

void t_isascii_0x2b()
{
    uassert_int_equal(isascii(43), 1); /* isascii should be 1 for + */
}

void t_isascii_0x2c()
{
    uassert_int_equal(isascii(44), 1); /* isascii should be 1 for , */
}

void t_isascii_0x2d()
{
    uassert_int_equal(isascii(45), 1); /* isascii should be 1 for - */
}

void t_isascii_0x2e()
{
    uassert_int_equal(isascii(46), 1); /* isascii should be 1 for . */
}

void t_isascii_0x2f()
{
    uassert_int_equal(isascii(47), 1); /* isascii should be 1 for / */
}

void t_isascii_0x30()
{
    uassert_int_equal(isascii(48), 1); /* isascii should be 1 for 0 */
}

void t_isascii_0x31()
{
    uassert_int_equal(isascii(49), 1); /* isascii should be 1 for 1 */
}

void t_isascii_0x32()
{
    uassert_int_equal(isascii(50), 1); /* isascii should be 1 for 2 */
}

void t_isascii_0x33()
{
    uassert_int_equal(isascii(51), 1); /* isascii should be 1 for 3 */
}

void t_isascii_0x34()
{
    uassert_int_equal(isascii(52), 1); /* isascii should be 1 for 4 */
}

void t_isascii_0x35()
{
    uassert_int_equal(isascii(53), 1); /* isascii should be 1 for 5 */
}

void t_isascii_0x36()
{
    uassert_int_equal(isascii(54), 1); /* isascii should be 1 for 6 */
}

void t_isascii_0x37()
{
    uassert_int_equal(isascii(55), 1); /* isascii should be 1 for 7 */
}

void t_isascii_0x38()
{
    uassert_int_equal(isascii(56), 1); /* isascii should be 1 for 8 */
}

void t_isascii_0x39()
{
    uassert_int_equal(isascii(57), 1); /* isascii should be 1 for 9 */
}

void t_isascii_0x3a()
{
    uassert_int_equal(isascii(58), 1); /* isascii should be 1 for : */
}

void t_isascii_0x3b()
{
    uassert_int_equal(isascii(59), 1); /* isascii should be 1 for ; */
}

void t_isascii_0x3c()
{
    uassert_int_equal(isascii(60), 1); /* isascii should be 1 for < */
}

void t_isascii_0x3d()
{
    uassert_int_equal(isascii(61), 1); /* isascii should be 1 for = */
}

void t_isascii_0x3e()
{
    uassert_int_equal(isascii(62), 1); /* isascii should be 1 for > */
}

void t_isascii_0x3f()
{
    uassert_int_equal(isascii(63), 1); /* isascii should be 1 for ? */
}

void t_isascii_0x40()
{
    uassert_int_equal(isascii(64), 1); /* isascii should be 1 for @ */
}

void t_isascii_0x41()
{
    uassert_int_equal(isascii(65), 1); /* isascii should be 1 for A */
}

void t_isascii_0x42()
{
    uassert_int_equal(isascii(66), 1); /* isascii should be 1 for B */
}

void t_isascii_0x43()
{
    uassert_int_equal(isascii(67), 1); /* isascii should be 1 for C */
}

void t_isascii_0x44()
{
    uassert_int_equal(isascii(68), 1); /* isascii should be 1 for D */
}

void t_isascii_0x45()
{
    uassert_int_equal(isascii(69), 1); /* isascii should be 1 for E */
}

void t_isascii_0x46()
{
    uassert_int_equal(isascii(70), 1); /* isascii should be 1 for F */
}

void t_isascii_0x47()
{
    uassert_int_equal(isascii(71), 1); /* isascii should be 1 for G */
}

void t_isascii_0x48()
{
    uassert_int_equal(isascii(72), 1); /* isascii should be 1 for H */
}

void t_isascii_0x49()
{
    uassert_int_equal(isascii(73), 1); /* isascii should be 1 for I */
}

void t_isascii_0x4a()
{
    uassert_int_equal(isascii(74), 1); /* isascii should be 1 for J */
}

void t_isascii_0x4b()
{
    uassert_int_equal(isascii(75), 1); /* isascii should be 1 for K */
}

void t_isascii_0x4c()
{
    uassert_int_equal(isascii(76), 1); /* isascii should be 1 for L */
}

void t_isascii_0x4d()
{
    uassert_int_equal(isascii(77), 1); /* isascii should be 1 for M */
}

void t_isascii_0x4e()
{
    uassert_int_equal(isascii(78), 1); /* isascii should be 1 for N */
}

void t_isascii_0x4f()
{
    uassert_int_equal(isascii(79), 1); /* isascii should be 1 for O */
}

void t_isascii_0x50()
{
    uassert_int_equal(isascii(80), 1); /* isascii should be 1 for P */
}

void t_isascii_0x51()
{
    uassert_int_equal(isascii(81), 1); /* isascii should be 1 for Q */
}

void t_isascii_0x52()
{
    uassert_int_equal(isascii(82), 1); /* isascii should be 1 for R */
}

void t_isascii_0x53()
{
    uassert_int_equal(isascii(83), 1); /* isascii should be 1 for S */
}

void t_isascii_0x54()
{
    uassert_int_equal(isascii(84), 1); /* isascii should be 1 for T */
}

void t_isascii_0x55()
{
    uassert_int_equal(isascii(85), 1); /* isascii should be 1 for U */
}

void t_isascii_0x56()
{
    uassert_int_equal(isascii(86), 1); /* isascii should be 1 for V */
}

void t_isascii_0x57()
{
    uassert_int_equal(isascii(87), 1); /* isascii should be 1 for W */
}

void t_isascii_0x58()
{
    uassert_int_equal(isascii(88), 1); /* isascii should be 1 for X */
}

void t_isascii_0x59()
{
    uassert_int_equal(isascii(89), 1); /* isascii should be 1 for Y */
}

void t_isascii_0x5a()
{
    uassert_int_equal(isascii(90), 1); /* isascii should be 1 for Z */
}

void t_isascii_0x5b()
{
    uassert_int_equal(isascii(91), 1); /* isascii should be 1 for [ */
}

void t_isascii_0x5c()
{
    uassert_int_equal(isascii(92), 1); /* isascii should be 1 for 0x5c */
}

void t_isascii_0x5d()
{
    uassert_int_equal(isascii(93), 1); /* isascii should be 1 for ] */
}

void t_isascii_0x5e()
{
    uassert_int_equal(isascii(94), 1); /* isascii should be 1 for ^ */
}

void t_isascii_0x5f()
{
    uassert_int_equal(isascii(95), 1); /* isascii should be 1 for _ */
}

void t_isascii_0x60()
{
    uassert_int_equal(isascii(96), 1); /* isascii should be 1 for ` */
}

void t_isascii_0x61()
{
    uassert_int_equal(isascii(97), 1); /* isascii should be 1 for a */
}

void t_isascii_0x62()
{
    uassert_int_equal(isascii(98), 1); /* isascii should be 1 for b */
}

void t_isascii_0x63()
{
    uassert_int_equal(isascii(99), 1); /* isascii should be 1 for c */
}

void t_isascii_0x64()
{
    uassert_int_equal(isascii(100), 1); /* isascii should be 1 for d */
}

void t_isascii_0x65()
{
    uassert_int_equal(isascii(101), 1); /* isascii should be 1 for e */
}

void t_isascii_0x66()
{
    uassert_int_equal(isascii(102), 1); /* isascii should be 1 for f */
}

void t_isascii_0x67()
{
    uassert_int_equal(isascii(103), 1); /* isascii should be 1 for g */
}

void t_isascii_0x68()
{
    uassert_int_equal(isascii(104), 1); /* isascii should be 1 for h */
}

void t_isascii_0x69()
{
    uassert_int_equal(isascii(105), 1); /* isascii should be 1 for i */
}

void t_isascii_0x6a()
{
    uassert_int_equal(isascii(106), 1); /* isascii should be 1 for j */
}

void t_isascii_0x6b()
{
    uassert_int_equal(isascii(107), 1); /* isascii should be 1 for k */
}

void t_isascii_0x6c()
{
    uassert_int_equal(isascii(108), 1); /* isascii should be 1 for l */
}

void t_isascii_0x6d()
{
    uassert_int_equal(isascii(109), 1); /* isascii should be 1 for m */
}

void t_isascii_0x6e()
{
    uassert_int_equal(isascii(110), 1); /* isascii should be 1 for n */
}

void t_isascii_0x6f()
{
    uassert_int_equal(isascii(111), 1); /* isascii should be 1 for o */
}

void t_isascii_0x70()
{
    uassert_int_equal(isascii(112), 1); /* isascii should be 1 for p */
}

void t_isascii_0x71()
{
    uassert_int_equal(isascii(113), 1); /* isascii should be 1 for q */
}

void t_isascii_0x72()
{
    uassert_int_equal(isascii(114), 1); /* isascii should be 1 for r */
}

void t_isascii_0x73()
{
    uassert_int_equal(isascii(115), 1); /* isascii should be 1 for s */
}

void t_isascii_0x74()
{
    uassert_int_equal(isascii(116), 1); /* isascii should be 1 for t */
}

void t_isascii_0x75()
{
    uassert_int_equal(isascii(117), 1); /* isascii should be 1 for u */
}

void t_isascii_0x76()
{
    uassert_int_equal(isascii(118), 1); /* isascii should be 1 for v */
}

void t_isascii_0x77()
{
    uassert_int_equal(isascii(119), 1); /* isascii should be 1 for w */
}

void t_isascii_0x78()
{
    uassert_int_equal(isascii(120), 1); /* isascii should be 1 for x */
}

void t_isascii_0x79()
{
    uassert_int_equal(isascii(121), 1); /* isascii should be 1 for y */
}

void t_isascii_0x7a()
{
    uassert_int_equal(isascii(122), 1); /* isascii should be 1 for z */
}

void t_isascii_0x7b()
{
    uassert_int_equal(isascii(123), 1); /* isascii should be 1 for { */
}

void t_isascii_0x7c()
{
    uassert_int_equal(isascii(124), 1); /* isascii should be 1 for | */
}

void t_isascii_0x7d()
{
    uassert_int_equal(isascii(125), 1); /* isascii should be 1 for } */
}

void t_isascii_0x7e()
{
    uassert_int_equal(isascii(126), 1); /* isascii should be 1 for ~ */
}

void t_isascii_0x7f()
{
    uassert_int_equal(isascii(127), 1); /* isascii should be 1 for 0x7f */
}

void t_isascii_0x80()
{
    uassert_int_equal(isascii(128), 0); /* isascii should be 0 for 0x80 */
}

void t_isascii_0x81()
{
    uassert_int_equal(isascii(129), 0); /* isascii should be 0 for 0x81 */
}

void t_isascii_0x82()
{
    uassert_int_equal(isascii(130), 0); /* isascii should be 0 for 0x82 */
}

void t_isascii_0x83()
{
    uassert_int_equal(isascii(131), 0); /* isascii should be 0 for 0x83 */
}

void t_isascii_0x84()
{
    uassert_int_equal(isascii(132), 0); /* isascii should be 0 for 0x84 */
}

void t_isascii_0x85()
{
    uassert_int_equal(isascii(133), 0); /* isascii should be 0 for 0x85 */
}

void t_isascii_0x86()
{
    uassert_int_equal(isascii(134), 0); /* isascii should be 0 for 0x86 */
}

void t_isascii_0x87()
{
    uassert_int_equal(isascii(135), 0); /* isascii should be 0 for 0x87 */
}

void t_isascii_0x88()
{
    uassert_int_equal(isascii(136), 0); /* isascii should be 0 for 0x88 */
}

void t_isascii_0x89()
{
    uassert_int_equal(isascii(137), 0); /* isascii should be 0 for 0x89 */
}

void t_isascii_0x8a()
{
    uassert_int_equal(isascii(138), 0); /* isascii should be 0 for 0x8a */
}

void t_isascii_0x8b()
{
    uassert_int_equal(isascii(139), 0); /* isascii should be 0 for 0x8b */
}

void t_isascii_0x8c()
{
    uassert_int_equal(isascii(140), 0); /* isascii should be 0 for 0x8c */
}

void t_isascii_0x8d()
{
    uassert_int_equal(isascii(141), 0); /* isascii should be 0 for 0x8d */
}

void t_isascii_0x8e()
{
    uassert_int_equal(isascii(142), 0); /* isascii should be 0 for 0x8e */
}

void t_isascii_0x8f()
{
    uassert_int_equal(isascii(143), 0); /* isascii should be 0 for 0x8f */
}

void t_isascii_0x90()
{
    uassert_int_equal(isascii(144), 0); /* isascii should be 0 for 0x90 */
}

void t_isascii_0x91()
{
    uassert_int_equal(isascii(145), 0); /* isascii should be 0 for 0x91 */
}

void t_isascii_0x92()
{
    uassert_int_equal(isascii(146), 0); /* isascii should be 0 for 0x92 */
}

void t_isascii_0x93()
{
    uassert_int_equal(isascii(147), 0); /* isascii should be 0 for 0x93 */
}

void t_isascii_0x94()
{
    uassert_int_equal(isascii(148), 0); /* isascii should be 0 for 0x94 */
}

void t_isascii_0x95()
{
    uassert_int_equal(isascii(149), 0); /* isascii should be 0 for 0x95 */
}

void t_isascii_0x96()
{
    uassert_int_equal(isascii(150), 0); /* isascii should be 0 for 0x96 */
}

void t_isascii_0x97()
{
    uassert_int_equal(isascii(151), 0); /* isascii should be 0 for 0x97 */
}

void t_isascii_0x98()
{
    uassert_int_equal(isascii(152), 0); /* isascii should be 0 for 0x98 */
}

void t_isascii_0x99()
{
    uassert_int_equal(isascii(153), 0); /* isascii should be 0 for 0x99 */
}

void t_isascii_0x9a()
{
    uassert_int_equal(isascii(154), 0); /* isascii should be 0 for 0x9a */
}

void t_isascii_0x9b()
{
    uassert_int_equal(isascii(155), 0); /* isascii should be 0 for 0x9b */
}

void t_isascii_0x9c()
{
    uassert_int_equal(isascii(156), 0); /* isascii should be 0 for 0x9c */
}

void t_isascii_0x9d()
{
    uassert_int_equal(isascii(157), 0); /* isascii should be 0 for 0x9d */
}

void t_isascii_0x9e()
{
    uassert_int_equal(isascii(158), 0); /* isascii should be 0 for 0x9e */
}

void t_isascii_0x9f()
{
    uassert_int_equal(isascii(159), 0); /* isascii should be 0 for 0x9f */
}

void t_isascii_0xa0()
{
    uassert_int_equal(isascii(160), 0); /* isascii should be 0 for 0xa0 */
}

void t_isascii_0xa1()
{
    uassert_int_equal(isascii(161), 0); /* isascii should be 0 for 0xa1 */
}

void t_isascii_0xa2()
{
    uassert_int_equal(isascii(162), 0); /* isascii should be 0 for 0xa2 */
}

void t_isascii_0xa3()
{
    uassert_int_equal(isascii(163), 0); /* isascii should be 0 for 0xa3 */
}

void t_isascii_0xa4()
{
    uassert_int_equal(isascii(164), 0); /* isascii should be 0 for 0xa4 */
}

void t_isascii_0xa5()
{
    uassert_int_equal(isascii(165), 0); /* isascii should be 0 for 0xa5 */
}

void t_isascii_0xa6()
{
    uassert_int_equal(isascii(166), 0); /* isascii should be 0 for 0xa6 */
}

void t_isascii_0xa7()
{
    uassert_int_equal(isascii(167), 0); /* isascii should be 0 for 0xa7 */
}

void t_isascii_0xa8()
{
    uassert_int_equal(isascii(168), 0); /* isascii should be 0 for 0xa8 */
}

void t_isascii_0xa9()
{
    uassert_int_equal(isascii(169), 0); /* isascii should be 0 for 0xa9 */
}

void t_isascii_0xaa()
{
    uassert_int_equal(isascii(170), 0); /* isascii should be 0 for 0xaa */
}

void t_isascii_0xab()
{
    uassert_int_equal(isascii(171), 0); /* isascii should be 0 for 0xab */
}

void t_isascii_0xac()
{
    uassert_int_equal(isascii(172), 0); /* isascii should be 0 for 0xac */
}

void t_isascii_0xad()
{
    uassert_int_equal(isascii(173), 0); /* isascii should be 0 for 0xad */
}

void t_isascii_0xae()
{
    uassert_int_equal(isascii(174), 0); /* isascii should be 0 for 0xae */
}

void t_isascii_0xaf()
{
    uassert_int_equal(isascii(175), 0); /* isascii should be 0 for 0xaf */
}

void t_isascii_0xb0()
{
    uassert_int_equal(isascii(176), 0); /* isascii should be 0 for 0xb0 */
}

void t_isascii_0xb1()
{
    uassert_int_equal(isascii(177), 0); /* isascii should be 0 for 0xb1 */
}

void t_isascii_0xb2()
{
    uassert_int_equal(isascii(178), 0); /* isascii should be 0 for 0xb2 */
}

void t_isascii_0xb3()
{
    uassert_int_equal(isascii(179), 0); /* isascii should be 0 for 0xb3 */
}

void t_isascii_0xb4()
{
    uassert_int_equal(isascii(180), 0); /* isascii should be 0 for 0xb4 */
}

void t_isascii_0xb5()
{
    uassert_int_equal(isascii(181), 0); /* isascii should be 0 for 0xb5 */
}

void t_isascii_0xb6()
{
    uassert_int_equal(isascii(182), 0); /* isascii should be 0 for 0xb6 */
}

void t_isascii_0xb7()
{
    uassert_int_equal(isascii(183), 0); /* isascii should be 0 for 0xb7 */
}

void t_isascii_0xb8()
{
    uassert_int_equal(isascii(184), 0); /* isascii should be 0 for 0xb8 */
}

void t_isascii_0xb9()
{
    uassert_int_equal(isascii(185), 0); /* isascii should be 0 for 0xb9 */
}

void t_isascii_0xba()
{
    uassert_int_equal(isascii(186), 0); /* isascii should be 0 for 0xba */
}

void t_isascii_0xbb()
{
    uassert_int_equal(isascii(187), 0); /* isascii should be 0 for 0xbb */
}

void t_isascii_0xbc()
{
    uassert_int_equal(isascii(188), 0); /* isascii should be 0 for 0xbc */
}

void t_isascii_0xbd()
{
    uassert_int_equal(isascii(189), 0); /* isascii should be 0 for 0xbd */
}

void t_isascii_0xbe()
{
    uassert_int_equal(isascii(190), 0); /* isascii should be 0 for 0xbe */
}

void t_isascii_0xbf()
{
    uassert_int_equal(isascii(191), 0); /* isascii should be 0 for 0xbf */
}

void t_isascii_0xc0()
{
    uassert_int_equal(isascii(192), 0); /* isascii should be 0 for 0xc0 */
}

void t_isascii_0xc1()
{
    uassert_int_equal(isascii(193), 0); /* isascii should be 0 for 0xc1 */
}

void t_isascii_0xc2()
{
    uassert_int_equal(isascii(194), 0); /* isascii should be 0 for 0xc2 */
}

void t_isascii_0xc3()
{
    uassert_int_equal(isascii(195), 0); /* isascii should be 0 for 0xc3 */
}

void t_isascii_0xc4()
{
    uassert_int_equal(isascii(196), 0); /* isascii should be 0 for 0xc4 */
}

void t_isascii_0xc5()
{
    uassert_int_equal(isascii(197), 0); /* isascii should be 0 for 0xc5 */
}

void t_isascii_0xc6()
{
    uassert_int_equal(isascii(198), 0); /* isascii should be 0 for 0xc6 */
}

void t_isascii_0xc7()
{
    uassert_int_equal(isascii(199), 0); /* isascii should be 0 for 0xc7 */
}

void t_isascii_0xc8()
{
    uassert_int_equal(isascii(200), 0); /* isascii should be 0 for 0xc8 */
}

void t_isascii_0xc9()
{
    uassert_int_equal(isascii(201), 0); /* isascii should be 0 for 0xc9 */
}

void t_isascii_0xca()
{
    uassert_int_equal(isascii(202), 0); /* isascii should be 0 for 0xca */
}

void t_isascii_0xcb()
{
    uassert_int_equal(isascii(203), 0); /* isascii should be 0 for 0xcb */
}

void t_isascii_0xcc()
{
    uassert_int_equal(isascii(204), 0); /* isascii should be 0 for 0xcc */
}

void t_isascii_0xcd()
{
    uassert_int_equal(isascii(205), 0); /* isascii should be 0 for 0xcd */
}

void t_isascii_0xce()
{
    uassert_int_equal(isascii(206), 0); /* isascii should be 0 for 0xce */
}

void t_isascii_0xcf()
{
    uassert_int_equal(isascii(207), 0); /* isascii should be 0 for 0xcf */
}

void t_isascii_0xd0()
{
    uassert_int_equal(isascii(208), 0); /* isascii should be 0 for 0xd0 */
}

void t_isascii_0xd1()
{
    uassert_int_equal(isascii(209), 0); /* isascii should be 0 for 0xd1 */
}

void t_isascii_0xd2()
{
    uassert_int_equal(isascii(210), 0); /* isascii should be 0 for 0xd2 */
}

void t_isascii_0xd3()
{
    uassert_int_equal(isascii(211), 0); /* isascii should be 0 for 0xd3 */
}

void t_isascii_0xd4()
{
    uassert_int_equal(isascii(212), 0); /* isascii should be 0 for 0xd4 */
}

void t_isascii_0xd5()
{
    uassert_int_equal(isascii(213), 0); /* isascii should be 0 for 0xd5 */
}

void t_isascii_0xd6()
{
    uassert_int_equal(isascii(214), 0); /* isascii should be 0 for 0xd6 */
}

void t_isascii_0xd7()
{
    uassert_int_equal(isascii(215), 0); /* isascii should be 0 for 0xd7 */
}

void t_isascii_0xd8()
{
    uassert_int_equal(isascii(216), 0); /* isascii should be 0 for 0xd8 */
}

void t_isascii_0xd9()
{
    uassert_int_equal(isascii(217), 0); /* isascii should be 0 for 0xd9 */
}

void t_isascii_0xda()
{
    uassert_int_equal(isascii(218), 0); /* isascii should be 0 for 0xda */
}

void t_isascii_0xdb()
{
    uassert_int_equal(isascii(219), 0); /* isascii should be 0 for 0xdb */
}

void t_isascii_0xdc()
{
    uassert_int_equal(isascii(220), 0); /* isascii should be 0 for 0xdc */
}

void t_isascii_0xdd()
{
    uassert_int_equal(isascii(221), 0); /* isascii should be 0 for 0xdd */
}

void t_isascii_0xde()
{
    uassert_int_equal(isascii(222), 0); /* isascii should be 0 for 0xde */
}

void t_isascii_0xdf()
{
    uassert_int_equal(isascii(223), 0); /* isascii should be 0 for 0xdf */
}

void t_isascii_0xe0()
{
    uassert_int_equal(isascii(224), 0); /* isascii should be 0 for 0xe0 */
}

void t_isascii_0xe1()
{
    uassert_int_equal(isascii(225), 0); /* isascii should be 0 for 0xe1 */
}

void t_isascii_0xe2()
{
    uassert_int_equal(isascii(226), 0); /* isascii should be 0 for 0xe2 */
}

void t_isascii_0xe3()
{
    uassert_int_equal(isascii(227), 0); /* isascii should be 0 for 0xe3 */
}

void t_isascii_0xe4()
{
    uassert_int_equal(isascii(228), 0); /* isascii should be 0 for 0xe4 */
}

void t_isascii_0xe5()
{
    uassert_int_equal(isascii(229), 0); /* isascii should be 0 for 0xe5 */
}

void t_isascii_0xe6()
{
    uassert_int_equal(isascii(230), 0); /* isascii should be 0 for 0xe6 */
}

void t_isascii_0xe7()
{
    uassert_int_equal(isascii(231), 0); /* isascii should be 0 for 0xe7 */
}

void t_isascii_0xe8()
{
    uassert_int_equal(isascii(232), 0); /* isascii should be 0 for 0xe8 */
}

void t_isascii_0xe9()
{
    uassert_int_equal(isascii(233), 0); /* isascii should be 0 for 0xe9 */
}

void t_isascii_0xea()
{
    uassert_int_equal(isascii(234), 0); /* isascii should be 0 for 0xea */
}

void t_isascii_0xeb()
{
    uassert_int_equal(isascii(235), 0); /* isascii should be 0 for 0xeb */
}

void t_isascii_0xec()
{
    uassert_int_equal(isascii(236), 0); /* isascii should be 0 for 0xec */
}

void t_isascii_0xed()
{
    uassert_int_equal(isascii(237), 0); /* isascii should be 0 for 0xed */
}

void t_isascii_0xee()
{
    uassert_int_equal(isascii(238), 0); /* isascii should be 0 for 0xee */
}

void t_isascii_0xef()
{
    uassert_int_equal(isascii(239), 0); /* isascii should be 0 for 0xef */
}

void t_isascii_0xf0()
{
    uassert_int_equal(isascii(240), 0); /* isascii should be 0 for 0xf0 */
}

void t_isascii_0xf1()
{
    uassert_int_equal(isascii(241), 0); /* isascii should be 0 for 0xf1 */
}

void t_isascii_0xf2()
{
    uassert_int_equal(isascii(242), 0); /* isascii should be 0 for 0xf2 */
}

void t_isascii_0xf3()
{
    uassert_int_equal(isascii(243), 0); /* isascii should be 0 for 0xf3 */
}

void t_isascii_0xf4()
{
    uassert_int_equal(isascii(244), 0); /* isascii should be 0 for 0xf4 */
}

void t_isascii_0xf5()
{
    uassert_int_equal(isascii(245), 0); /* isascii should be 0 for 0xf5 */
}

void t_isascii_0xf6()
{
    uassert_int_equal(isascii(246), 0); /* isascii should be 0 for 0xf6 */
}

void t_isascii_0xf7()
{
    uassert_int_equal(isascii(247), 0); /* isascii should be 0 for 0xf7 */
}

void t_isascii_0xf8()
{
    uassert_int_equal(isascii(248), 0); /* isascii should be 0 for 0xf8 */
}

void t_isascii_0xf9()
{
    uassert_int_equal(isascii(249), 0); /* isascii should be 0 for 0xf9 */
}

void t_isascii_0xfa()
{
    uassert_int_equal(isascii(250), 0); /* isascii should be 0 for 0xfa */
}

void t_isascii_0xfb()
{
    uassert_int_equal(isascii(251), 0); /* isascii should be 0 for 0xfb */
}

void t_isascii_0xfc()
{
    uassert_int_equal(isascii(252), 0); /* isascii should be 0 for 0xfc */
}

void t_isascii_0xfd()
{
    uassert_int_equal(isascii(253), 0); /* isascii should be 0 for 0xfd */
}

void t_isascii_0xfe()
{
    uassert_int_equal(isascii(254), 0); /* isascii should be 0 for 0xfe */
}

void t_isascii_0xff()
{
    uassert_int_equal(isascii(255), 0); /* isascii should be 0 for 0xff */
}



static int testcase(void)
{
    t_isascii_0x00();
    t_isascii_0x01();
    t_isascii_0x02();
    t_isascii_0x03();
    t_isascii_0x04();
    t_isascii_0x05();
    t_isascii_0x06();
    t_isascii_0x07();
    t_isascii_0x08();
    t_isascii_0x09();
    t_isascii_0x0a();
    t_isascii_0x0b();
    t_isascii_0x0c();
    t_isascii_0x0d();
    t_isascii_0x0e();
    t_isascii_0x0f();
    t_isascii_0x10();
    t_isascii_0x11();
    t_isascii_0x12();
    t_isascii_0x13();
    t_isascii_0x14();
    t_isascii_0x15();
    t_isascii_0x16();
    t_isascii_0x17();
    t_isascii_0x18();
    t_isascii_0x19();
    t_isascii_0x1a();
    t_isascii_0x1b();
    t_isascii_0x1c();
    t_isascii_0x1d();
    t_isascii_0x1e();
    t_isascii_0x1f();
    t_isascii_0x20();
    t_isascii_0x21();
    t_isascii_0x22();
    t_isascii_0x23();
    t_isascii_0x24();
    t_isascii_0x25();
    t_isascii_0x26();
    t_isascii_0x27();
    t_isascii_0x28();
    t_isascii_0x29();
    t_isascii_0x2a();
    t_isascii_0x2b();
    t_isascii_0x2c();
    t_isascii_0x2d();
    t_isascii_0x2e();
    t_isascii_0x2f();
    t_isascii_0x30();
    t_isascii_0x31();
    t_isascii_0x32();
    t_isascii_0x33();
    t_isascii_0x34();
    t_isascii_0x35();
    t_isascii_0x36();
    t_isascii_0x37();
    t_isascii_0x38();
    t_isascii_0x39();
    t_isascii_0x3a();
    t_isascii_0x3b();
    t_isascii_0x3c();
    t_isascii_0x3d();
    t_isascii_0x3e();
    t_isascii_0x3f();
    t_isascii_0x40();
    t_isascii_0x41();
    t_isascii_0x42();
    t_isascii_0x43();
    t_isascii_0x44();
    t_isascii_0x45();
    t_isascii_0x46();
    t_isascii_0x47();
    t_isascii_0x48();
    t_isascii_0x49();
    t_isascii_0x4a();
    t_isascii_0x4b();
    t_isascii_0x4c();
    t_isascii_0x4d();
    t_isascii_0x4e();
    t_isascii_0x4f();
    t_isascii_0x50();
    t_isascii_0x51();
    t_isascii_0x52();
    t_isascii_0x53();
    t_isascii_0x54();
    t_isascii_0x55();
    t_isascii_0x56();
    t_isascii_0x57();
    t_isascii_0x58();
    t_isascii_0x59();
    t_isascii_0x5a();
    t_isascii_0x5b();
    t_isascii_0x5c();
    t_isascii_0x5d();
    t_isascii_0x5e();
    t_isascii_0x5f();
    t_isascii_0x60();
    t_isascii_0x61();
    t_isascii_0x62();
    t_isascii_0x63();
    t_isascii_0x64();
    t_isascii_0x65();
    t_isascii_0x66();
    t_isascii_0x67();
    t_isascii_0x68();
    t_isascii_0x69();
    t_isascii_0x6a();
    t_isascii_0x6b();
    t_isascii_0x6c();
    t_isascii_0x6d();
    t_isascii_0x6e();
    t_isascii_0x6f();
    t_isascii_0x70();
    t_isascii_0x71();
    t_isascii_0x72();
    t_isascii_0x73();
    t_isascii_0x74();
    t_isascii_0x75();
    t_isascii_0x76();
    t_isascii_0x77();
    t_isascii_0x78();
    t_isascii_0x79();
    t_isascii_0x7a();
    t_isascii_0x7b();
    t_isascii_0x7c();
    t_isascii_0x7d();
    t_isascii_0x7e();
    t_isascii_0x7f();
    t_isascii_0x80();
    t_isascii_0x81();
    t_isascii_0x82();
    t_isascii_0x83();
    t_isascii_0x84();
    t_isascii_0x85();
    t_isascii_0x86();
    t_isascii_0x87();
    t_isascii_0x88();
    t_isascii_0x89();
    t_isascii_0x8a();
    t_isascii_0x8b();
    t_isascii_0x8c();
    t_isascii_0x8d();
    t_isascii_0x8e();
    t_isascii_0x8f();
    t_isascii_0x90();
    t_isascii_0x91();
    t_isascii_0x92();
    t_isascii_0x93();
    t_isascii_0x94();
    t_isascii_0x95();
    t_isascii_0x96();
    t_isascii_0x97();
    t_isascii_0x98();
    t_isascii_0x99();
    t_isascii_0x9a();
    t_isascii_0x9b();
    t_isascii_0x9c();
    t_isascii_0x9d();
    t_isascii_0x9e();
    t_isascii_0x9f();
    t_isascii_0xa0();
    t_isascii_0xa1();
    t_isascii_0xa2();
    t_isascii_0xa3();
    t_isascii_0xa4();
    t_isascii_0xa5();
    t_isascii_0xa6();
    t_isascii_0xa7();
    t_isascii_0xa8();
    t_isascii_0xa9();
    t_isascii_0xaa();
    t_isascii_0xab();
    t_isascii_0xac();
    t_isascii_0xad();
    t_isascii_0xae();
    t_isascii_0xaf();
    t_isascii_0xb0();
    t_isascii_0xb1();
    t_isascii_0xb2();
    t_isascii_0xb3();
    t_isascii_0xb4();
    t_isascii_0xb5();
    t_isascii_0xb6();
    t_isascii_0xb7();
    t_isascii_0xb8();
    t_isascii_0xb9();
    t_isascii_0xba();
    t_isascii_0xbb();
    t_isascii_0xbc();
    t_isascii_0xbd();
    t_isascii_0xbe();
    t_isascii_0xbf();
    t_isascii_0xc0();
    t_isascii_0xc1();
    t_isascii_0xc2();
    t_isascii_0xc3();
    t_isascii_0xc4();
    t_isascii_0xc5();
    t_isascii_0xc6();
    t_isascii_0xc7();
    t_isascii_0xc8();
    t_isascii_0xc9();
    t_isascii_0xca();
    t_isascii_0xcb();
    t_isascii_0xcc();
    t_isascii_0xcd();
    t_isascii_0xce();
    t_isascii_0xcf();
    t_isascii_0xd0();
    t_isascii_0xd1();
    t_isascii_0xd2();
    t_isascii_0xd3();
    t_isascii_0xd4();
    t_isascii_0xd5();
    t_isascii_0xd6();
    t_isascii_0xd7();
    t_isascii_0xd8();
    t_isascii_0xd9();
    t_isascii_0xda();
    t_isascii_0xdb();
    t_isascii_0xdc();
    t_isascii_0xdd();
    t_isascii_0xde();
    t_isascii_0xdf();
    t_isascii_0xe0();
    t_isascii_0xe1();
    t_isascii_0xe2();
    t_isascii_0xe3();
    t_isascii_0xe4();
    t_isascii_0xe5();
    t_isascii_0xe6();
    t_isascii_0xe7();
    t_isascii_0xe8();
    t_isascii_0xe9();
    t_isascii_0xea();
    t_isascii_0xeb();
    t_isascii_0xec();
    t_isascii_0xed();
    t_isascii_0xee();
    t_isascii_0xef();
    t_isascii_0xf0();
    t_isascii_0xf1();
    t_isascii_0xf2();
    t_isascii_0xf3();
    t_isascii_0xf4();
    t_isascii_0xf5();
    t_isascii_0xf6();
    t_isascii_0xf7();
    t_isascii_0xf8();
    t_isascii_0xf9();
    t_isascii_0xfa();
    t_isascii_0xfb();
    t_isascii_0xfc();
    t_isascii_0xfd();
    t_isascii_0xfe();
    t_isascii_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
