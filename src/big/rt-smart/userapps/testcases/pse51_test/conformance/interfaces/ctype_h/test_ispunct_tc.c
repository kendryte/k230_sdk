#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))

void t_ispunct_0x00()
{
    uassert_int_equal(ispunct(0), 0); /* ispunct should be 0 for 0x00 */
}

void t_ispunct_0x01()
{
    uassert_int_equal(ispunct(1), 0); /* ispunct should be 0 for 0x01 */
}

void t_ispunct_0x02()
{
    uassert_int_equal(ispunct(2), 0); /* ispunct should be 0 for 0x02 */
}

void t_ispunct_0x03()
{
    uassert_int_equal(ispunct(3), 0); /* ispunct should be 0 for 0x03 */
}

void t_ispunct_0x04()
{
    uassert_int_equal(ispunct(4), 0); /* ispunct should be 0 for 0x04 */
}

void t_ispunct_0x05()
{
    uassert_int_equal(ispunct(5), 0); /* ispunct should be 0 for 0x05 */
}

void t_ispunct_0x06()
{
    uassert_int_equal(ispunct(6), 0); /* ispunct should be 0 for 0x06 */
}

void t_ispunct_0x07()
{
    uassert_int_equal(ispunct(7), 0); /* ispunct should be 0 for 0x07 */
}

void t_ispunct_0x08()
{
    uassert_int_equal(ispunct(8), 0); /* ispunct should be 0 for 0x08 */
}

void t_ispunct_0x09()
{
    uassert_int_equal(ispunct(9), 0); /* ispunct should be 0 for 0x09 */
}

void t_ispunct_0x0a()
{
    uassert_int_equal(ispunct(10), 0); /* ispunct should be 0 for 0x0a */
}

void t_ispunct_0x0b()
{
    uassert_int_equal(ispunct(11), 0); /* ispunct should be 0 for 0x0b */
}

void t_ispunct_0x0c()
{
    uassert_int_equal(ispunct(12), 0); /* ispunct should be 0 for 0x0c */
}

void t_ispunct_0x0d()
{
    uassert_int_equal(ispunct(13), 0); /* ispunct should be 0 for 0x0d */
}

void t_ispunct_0x0e()
{
    uassert_int_equal(ispunct(14), 0); /* ispunct should be 0 for 0x0e */
}

void t_ispunct_0x0f()
{
    uassert_int_equal(ispunct(15), 0); /* ispunct should be 0 for 0x0f */
}

void t_ispunct_0x10()
{
    uassert_int_equal(ispunct(16), 0); /* ispunct should be 0 for 0x10 */
}

void t_ispunct_0x11()
{
    uassert_int_equal(ispunct(17), 0); /* ispunct should be 0 for 0x11 */
}

void t_ispunct_0x12()
{
    uassert_int_equal(ispunct(18), 0); /* ispunct should be 0 for 0x12 */
}

void t_ispunct_0x13()
{
    uassert_int_equal(ispunct(19), 0); /* ispunct should be 0 for 0x13 */
}

void t_ispunct_0x14()
{
    uassert_int_equal(ispunct(20), 0); /* ispunct should be 0 for 0x14 */
}

void t_ispunct_0x15()
{
    uassert_int_equal(ispunct(21), 0); /* ispunct should be 0 for 0x15 */
}

void t_ispunct_0x16()
{
    uassert_int_equal(ispunct(22), 0); /* ispunct should be 0 for 0x16 */
}

void t_ispunct_0x17()
{
    uassert_int_equal(ispunct(23), 0); /* ispunct should be 0 for 0x17 */
}

void t_ispunct_0x18()
{
    uassert_int_equal(ispunct(24), 0); /* ispunct should be 0 for 0x18 */
}

void t_ispunct_0x19()
{
    uassert_int_equal(ispunct(25), 0); /* ispunct should be 0 for 0x19 */
}

void t_ispunct_0x1a()
{
    uassert_int_equal(ispunct(26), 0); /* ispunct should be 0 for 0x1a */
}

void t_ispunct_0x1b()
{
    uassert_int_equal(ispunct(27), 0); /* ispunct should be 0 for 0x1b */
}

void t_ispunct_0x1c()
{
    uassert_int_equal(ispunct(28), 0); /* ispunct should be 0 for 0x1c */
}

void t_ispunct_0x1d()
{
    uassert_int_equal(ispunct(29), 0); /* ispunct should be 0 for 0x1d */
}

void t_ispunct_0x1e()
{
    uassert_int_equal(ispunct(30), 0); /* ispunct should be 0 for 0x1e */
}

void t_ispunct_0x1f()
{
    uassert_int_equal(ispunct(31), 0); /* ispunct should be 0 for 0x1f */
}

void t_ispunct_0x20()
{
    uassert_int_equal(ispunct(32), 0); /* ispunct should be 0 for   */
}

void t_ispunct_0x21()
{
    uassert_int_equal(ispunct(33), 1); /* ispunct should be 1 for ! */
}

void t_ispunct_0x22()
{
    uassert_int_equal(ispunct(34), 1); /* ispunct should be 1 for 0x22 */
}

void t_ispunct_0x23()
{
    uassert_int_equal(ispunct(35), 1); /* ispunct should be 1 for # */
}

void t_ispunct_0x24()
{
    uassert_int_equal(ispunct(36), 1); /* ispunct should be 1 for $ */
}

void t_ispunct_0x25()
{
    uassert_int_equal(ispunct(37), 1); /* ispunct should be 1 for % */
}

void t_ispunct_0x26()
{
    uassert_int_equal(ispunct(38), 1); /* ispunct should be 1 for & */
}

void t_ispunct_0x27()
{
    uassert_int_equal(ispunct(39), 1); /* ispunct should be 1 for ' */
}

void t_ispunct_0x28()
{
    uassert_int_equal(ispunct(40), 1); /* ispunct should be 1 for ( */
}

void t_ispunct_0x29()
{
    uassert_int_equal(ispunct(41), 1); /* ispunct should be 1 for ) */
}

void t_ispunct_0x2a()
{
    uassert_int_equal(ispunct(42), 1); /* ispunct should be 1 for * */
}

void t_ispunct_0x2b()
{
    uassert_int_equal(ispunct(43), 1); /* ispunct should be 1 for + */
}

void t_ispunct_0x2c()
{
    uassert_int_equal(ispunct(44), 1); /* ispunct should be 1 for , */
}

void t_ispunct_0x2d()
{
    uassert_int_equal(ispunct(45), 1); /* ispunct should be 1 for - */
}

void t_ispunct_0x2e()
{
    uassert_int_equal(ispunct(46), 1); /* ispunct should be 1 for . */
}

void t_ispunct_0x2f()
{
    uassert_int_equal(ispunct(47), 1); /* ispunct should be 1 for / */
}

void t_ispunct_0x30()
{
    uassert_int_equal(ispunct(48), 0); /* ispunct should be 0 for 0 */
}

void t_ispunct_0x31()
{
    uassert_int_equal(ispunct(49), 0); /* ispunct should be 0 for 1 */
}

void t_ispunct_0x32()
{
    uassert_int_equal(ispunct(50), 0); /* ispunct should be 0 for 2 */
}

void t_ispunct_0x33()
{
    uassert_int_equal(ispunct(51), 0); /* ispunct should be 0 for 3 */
}

void t_ispunct_0x34()
{
    uassert_int_equal(ispunct(52), 0); /* ispunct should be 0 for 4 */
}

void t_ispunct_0x35()
{
    uassert_int_equal(ispunct(53), 0); /* ispunct should be 0 for 5 */
}

void t_ispunct_0x36()
{
    uassert_int_equal(ispunct(54), 0); /* ispunct should be 0 for 6 */
}

void t_ispunct_0x37()
{
    uassert_int_equal(ispunct(55), 0); /* ispunct should be 0 for 7 */
}

void t_ispunct_0x38()
{
    uassert_int_equal(ispunct(56), 0); /* ispunct should be 0 for 8 */
}

void t_ispunct_0x39()
{
    uassert_int_equal(ispunct(57), 0); /* ispunct should be 0 for 9 */
}

void t_ispunct_0x3a()
{
    uassert_int_equal(ispunct(58), 1); /* ispunct should be 1 for : */
}

void t_ispunct_0x3b()
{
    uassert_int_equal(ispunct(59), 1); /* ispunct should be 1 for ; */
}

void t_ispunct_0x3c()
{
    uassert_int_equal(ispunct(60), 1); /* ispunct should be 1 for < */
}

void t_ispunct_0x3d()
{
    uassert_int_equal(ispunct(61), 1); /* ispunct should be 1 for = */
}

void t_ispunct_0x3e()
{
    uassert_int_equal(ispunct(62), 1); /* ispunct should be 1 for > */
}

void t_ispunct_0x3f()
{
    uassert_int_equal(ispunct(63), 1); /* ispunct should be 1 for ? */
}

void t_ispunct_0x40()
{
    uassert_int_equal(ispunct(64), 1); /* ispunct should be 1 for @ */
}

void t_ispunct_0x41()
{
    uassert_int_equal(ispunct(65), 0); /* ispunct should be 0 for A */
}

void t_ispunct_0x42()
{
    uassert_int_equal(ispunct(66), 0); /* ispunct should be 0 for B */
}

void t_ispunct_0x43()
{
    uassert_int_equal(ispunct(67), 0); /* ispunct should be 0 for C */
}

void t_ispunct_0x44()
{
    uassert_int_equal(ispunct(68), 0); /* ispunct should be 0 for D */
}

void t_ispunct_0x45()
{
    uassert_int_equal(ispunct(69), 0); /* ispunct should be 0 for E */
}

void t_ispunct_0x46()
{
    uassert_int_equal(ispunct(70), 0); /* ispunct should be 0 for F */
}

void t_ispunct_0x47()
{
    uassert_int_equal(ispunct(71), 0); /* ispunct should be 0 for G */
}

void t_ispunct_0x48()
{
    uassert_int_equal(ispunct(72), 0); /* ispunct should be 0 for H */
}

void t_ispunct_0x49()
{
    uassert_int_equal(ispunct(73), 0); /* ispunct should be 0 for I */
}

void t_ispunct_0x4a()
{
    uassert_int_equal(ispunct(74), 0); /* ispunct should be 0 for J */
}

void t_ispunct_0x4b()
{
    uassert_int_equal(ispunct(75), 0); /* ispunct should be 0 for K */
}

void t_ispunct_0x4c()
{
    uassert_int_equal(ispunct(76), 0); /* ispunct should be 0 for L */
}

void t_ispunct_0x4d()
{
    uassert_int_equal(ispunct(77), 0); /* ispunct should be 0 for M */
}

void t_ispunct_0x4e()
{
    uassert_int_equal(ispunct(78), 0); /* ispunct should be 0 for N */
}

void t_ispunct_0x4f()
{
    uassert_int_equal(ispunct(79), 0); /* ispunct should be 0 for O */
}

void t_ispunct_0x50()
{
    uassert_int_equal(ispunct(80), 0); /* ispunct should be 0 for P */
}

void t_ispunct_0x51()
{
    uassert_int_equal(ispunct(81), 0); /* ispunct should be 0 for Q */
}

void t_ispunct_0x52()
{
    uassert_int_equal(ispunct(82), 0); /* ispunct should be 0 for R */
}

void t_ispunct_0x53()
{
    uassert_int_equal(ispunct(83), 0); /* ispunct should be 0 for S */
}

void t_ispunct_0x54()
{
    uassert_int_equal(ispunct(84), 0); /* ispunct should be 0 for T */
}

void t_ispunct_0x55()
{
    uassert_int_equal(ispunct(85), 0); /* ispunct should be 0 for U */
}

void t_ispunct_0x56()
{
    uassert_int_equal(ispunct(86), 0); /* ispunct should be 0 for V */
}

void t_ispunct_0x57()
{
    uassert_int_equal(ispunct(87), 0); /* ispunct should be 0 for W */
}

void t_ispunct_0x58()
{
    uassert_int_equal(ispunct(88), 0); /* ispunct should be 0 for X */
}

void t_ispunct_0x59()
{
    uassert_int_equal(ispunct(89), 0); /* ispunct should be 0 for Y */
}

void t_ispunct_0x5a()
{
    uassert_int_equal(ispunct(90), 0); /* ispunct should be 0 for Z */
}

void t_ispunct_0x5b()
{
    uassert_int_equal(ispunct(91), 1); /* ispunct should be 1 for [ */
}

void t_ispunct_0x5c()
{
    uassert_int_equal(ispunct(92), 1); /* ispunct should be 1 for 0x5c */
}

void t_ispunct_0x5d()
{
    uassert_int_equal(ispunct(93), 1); /* ispunct should be 1 for ] */
}

void t_ispunct_0x5e()
{
    uassert_int_equal(ispunct(94), 1); /* ispunct should be 1 for ^ */
}

void t_ispunct_0x5f()
{
    uassert_int_equal(ispunct(95), 1); /* ispunct should be 1 for _ */
}

void t_ispunct_0x60()
{
    uassert_int_equal(ispunct(96), 1); /* ispunct should be 1 for ` */
}

void t_ispunct_0x61()
{
    uassert_int_equal(ispunct(97), 0); /* ispunct should be 0 for a */
}

void t_ispunct_0x62()
{
    uassert_int_equal(ispunct(98), 0); /* ispunct should be 0 for b */
}

void t_ispunct_0x63()
{
    uassert_int_equal(ispunct(99), 0); /* ispunct should be 0 for c */
}

void t_ispunct_0x64()
{
    uassert_int_equal(ispunct(100), 0); /* ispunct should be 0 for d */
}

void t_ispunct_0x65()
{
    uassert_int_equal(ispunct(101), 0); /* ispunct should be 0 for e */
}

void t_ispunct_0x66()
{
    uassert_int_equal(ispunct(102), 0); /* ispunct should be 0 for f */
}

void t_ispunct_0x67()
{
    uassert_int_equal(ispunct(103), 0); /* ispunct should be 0 for g */
}

void t_ispunct_0x68()
{
    uassert_int_equal(ispunct(104), 0); /* ispunct should be 0 for h */
}

void t_ispunct_0x69()
{
    uassert_int_equal(ispunct(105), 0); /* ispunct should be 0 for i */
}

void t_ispunct_0x6a()
{
    uassert_int_equal(ispunct(106), 0); /* ispunct should be 0 for j */
}

void t_ispunct_0x6b()
{
    uassert_int_equal(ispunct(107), 0); /* ispunct should be 0 for k */
}

void t_ispunct_0x6c()
{
    uassert_int_equal(ispunct(108), 0); /* ispunct should be 0 for l */
}

void t_ispunct_0x6d()
{
    uassert_int_equal(ispunct(109), 0); /* ispunct should be 0 for m */
}

void t_ispunct_0x6e()
{
    uassert_int_equal(ispunct(110), 0); /* ispunct should be 0 for n */
}

void t_ispunct_0x6f()
{
    uassert_int_equal(ispunct(111), 0); /* ispunct should be 0 for o */
}

void t_ispunct_0x70()
{
    uassert_int_equal(ispunct(112), 0); /* ispunct should be 0 for p */
}

void t_ispunct_0x71()
{
    uassert_int_equal(ispunct(113), 0); /* ispunct should be 0 for q */
}

void t_ispunct_0x72()
{
    uassert_int_equal(ispunct(114), 0); /* ispunct should be 0 for r */
}

void t_ispunct_0x73()
{
    uassert_int_equal(ispunct(115), 0); /* ispunct should be 0 for s */
}

void t_ispunct_0x74()
{
    uassert_int_equal(ispunct(116), 0); /* ispunct should be 0 for t */
}

void t_ispunct_0x75()
{
    uassert_int_equal(ispunct(117), 0); /* ispunct should be 0 for u */
}

void t_ispunct_0x76()
{
    uassert_int_equal(ispunct(118), 0); /* ispunct should be 0 for v */
}

void t_ispunct_0x77()
{
    uassert_int_equal(ispunct(119), 0); /* ispunct should be 0 for w */
}

void t_ispunct_0x78()
{
    uassert_int_equal(ispunct(120), 0); /* ispunct should be 0 for x */
}

void t_ispunct_0x79()
{
    uassert_int_equal(ispunct(121), 0); /* ispunct should be 0 for y */
}

void t_ispunct_0x7a()
{
    uassert_int_equal(ispunct(122), 0); /* ispunct should be 0 for z */
}

void t_ispunct_0x7b()
{
    uassert_int_equal(ispunct(123), 1); /* ispunct should be 1 for { */
}

void t_ispunct_0x7c()
{
    uassert_int_equal(ispunct(124), 1); /* ispunct should be 1 for | */
}

void t_ispunct_0x7d()
{
    uassert_int_equal(ispunct(125), 1); /* ispunct should be 1 for } */
}

void t_ispunct_0x7e()
{
    uassert_int_equal(ispunct(126), 1); /* ispunct should be 1 for ~ */
}

void t_ispunct_0x7f()
{
    uassert_int_equal(ispunct(127), 0); /* ispunct should be 0 for 0x7f */
}

void t_ispunct_0x80()
{
    uassert_int_equal(ispunct(128), 0); /* ispunct should be 0 for 0x80 */
}

void t_ispunct_0x81()
{
    uassert_int_equal(ispunct(129), 0); /* ispunct should be 0 for 0x81 */
}

void t_ispunct_0x82()
{
    uassert_int_equal(ispunct(130), 0); /* ispunct should be 0 for 0x82 */
}

void t_ispunct_0x83()
{
    uassert_int_equal(ispunct(131), 0); /* ispunct should be 0 for 0x83 */
}

void t_ispunct_0x84()
{
    uassert_int_equal(ispunct(132), 0); /* ispunct should be 0 for 0x84 */
}

void t_ispunct_0x85()
{
    uassert_int_equal(ispunct(133), 0); /* ispunct should be 0 for 0x85 */
}

void t_ispunct_0x86()
{
    uassert_int_equal(ispunct(134), 0); /* ispunct should be 0 for 0x86 */
}

void t_ispunct_0x87()
{
    uassert_int_equal(ispunct(135), 0); /* ispunct should be 0 for 0x87 */
}

void t_ispunct_0x88()
{
    uassert_int_equal(ispunct(136), 0); /* ispunct should be 0 for 0x88 */
}

void t_ispunct_0x89()
{
    uassert_int_equal(ispunct(137), 0); /* ispunct should be 0 for 0x89 */
}

void t_ispunct_0x8a()
{
    uassert_int_equal(ispunct(138), 0); /* ispunct should be 0 for 0x8a */
}

void t_ispunct_0x8b()
{
    uassert_int_equal(ispunct(139), 0); /* ispunct should be 0 for 0x8b */
}

void t_ispunct_0x8c()
{
    uassert_int_equal(ispunct(140), 0); /* ispunct should be 0 for 0x8c */
}

void t_ispunct_0x8d()
{
    uassert_int_equal(ispunct(141), 0); /* ispunct should be 0 for 0x8d */
}

void t_ispunct_0x8e()
{
    uassert_int_equal(ispunct(142), 0); /* ispunct should be 0 for 0x8e */
}

void t_ispunct_0x8f()
{
    uassert_int_equal(ispunct(143), 0); /* ispunct should be 0 for 0x8f */
}

void t_ispunct_0x90()
{
    uassert_int_equal(ispunct(144), 0); /* ispunct should be 0 for 0x90 */
}

void t_ispunct_0x91()
{
    uassert_int_equal(ispunct(145), 0); /* ispunct should be 0 for 0x91 */
}

void t_ispunct_0x92()
{
    uassert_int_equal(ispunct(146), 0); /* ispunct should be 0 for 0x92 */
}

void t_ispunct_0x93()
{
    uassert_int_equal(ispunct(147), 0); /* ispunct should be 0 for 0x93 */
}

void t_ispunct_0x94()
{
    uassert_int_equal(ispunct(148), 0); /* ispunct should be 0 for 0x94 */
}

void t_ispunct_0x95()
{
    uassert_int_equal(ispunct(149), 0); /* ispunct should be 0 for 0x95 */
}

void t_ispunct_0x96()
{
    uassert_int_equal(ispunct(150), 0); /* ispunct should be 0 for 0x96 */
}

void t_ispunct_0x97()
{
    uassert_int_equal(ispunct(151), 0); /* ispunct should be 0 for 0x97 */
}

void t_ispunct_0x98()
{
    uassert_int_equal(ispunct(152), 0); /* ispunct should be 0 for 0x98 */
}

void t_ispunct_0x99()
{
    uassert_int_equal(ispunct(153), 0); /* ispunct should be 0 for 0x99 */
}

void t_ispunct_0x9a()
{
    uassert_int_equal(ispunct(154), 0); /* ispunct should be 0 for 0x9a */
}

void t_ispunct_0x9b()
{
    uassert_int_equal(ispunct(155), 0); /* ispunct should be 0 for 0x9b */
}

void t_ispunct_0x9c()
{
    uassert_int_equal(ispunct(156), 0); /* ispunct should be 0 for 0x9c */
}

void t_ispunct_0x9d()
{
    uassert_int_equal(ispunct(157), 0); /* ispunct should be 0 for 0x9d */
}

void t_ispunct_0x9e()
{
    uassert_int_equal(ispunct(158), 0); /* ispunct should be 0 for 0x9e */
}

void t_ispunct_0x9f()
{
    uassert_int_equal(ispunct(159), 0); /* ispunct should be 0 for 0x9f */
}

void t_ispunct_0xa0()
{
    uassert_int_equal(ispunct(160), 0); /* ispunct should be 0 for 0xa0 */
}

void t_ispunct_0xa1()
{
    uassert_int_equal(ispunct(161), 0); /* ispunct should be 0 for 0xa1 */
}

void t_ispunct_0xa2()
{
    uassert_int_equal(ispunct(162), 0); /* ispunct should be 0 for 0xa2 */
}

void t_ispunct_0xa3()
{
    uassert_int_equal(ispunct(163), 0); /* ispunct should be 0 for 0xa3 */
}

void t_ispunct_0xa4()
{
    uassert_int_equal(ispunct(164), 0); /* ispunct should be 0 for 0xa4 */
}

void t_ispunct_0xa5()
{
    uassert_int_equal(ispunct(165), 0); /* ispunct should be 0 for 0xa5 */
}

void t_ispunct_0xa6()
{
    uassert_int_equal(ispunct(166), 0); /* ispunct should be 0 for 0xa6 */
}

void t_ispunct_0xa7()
{
    uassert_int_equal(ispunct(167), 0); /* ispunct should be 0 for 0xa7 */
}

void t_ispunct_0xa8()
{
    uassert_int_equal(ispunct(168), 0); /* ispunct should be 0 for 0xa8 */
}

void t_ispunct_0xa9()
{
    uassert_int_equal(ispunct(169), 0); /* ispunct should be 0 for 0xa9 */
}

void t_ispunct_0xaa()
{
    uassert_int_equal(ispunct(170), 0); /* ispunct should be 0 for 0xaa */
}

void t_ispunct_0xab()
{
    uassert_int_equal(ispunct(171), 0); /* ispunct should be 0 for 0xab */
}

void t_ispunct_0xac()
{
    uassert_int_equal(ispunct(172), 0); /* ispunct should be 0 for 0xac */
}

void t_ispunct_0xad()
{
    uassert_int_equal(ispunct(173), 0); /* ispunct should be 0 for 0xad */
}

void t_ispunct_0xae()
{
    uassert_int_equal(ispunct(174), 0); /* ispunct should be 0 for 0xae */
}

void t_ispunct_0xaf()
{
    uassert_int_equal(ispunct(175), 0); /* ispunct should be 0 for 0xaf */
}

void t_ispunct_0xb0()
{
    uassert_int_equal(ispunct(176), 0); /* ispunct should be 0 for 0xb0 */
}

void t_ispunct_0xb1()
{
    uassert_int_equal(ispunct(177), 0); /* ispunct should be 0 for 0xb1 */
}

void t_ispunct_0xb2()
{
    uassert_int_equal(ispunct(178), 0); /* ispunct should be 0 for 0xb2 */
}

void t_ispunct_0xb3()
{
    uassert_int_equal(ispunct(179), 0); /* ispunct should be 0 for 0xb3 */
}

void t_ispunct_0xb4()
{
    uassert_int_equal(ispunct(180), 0); /* ispunct should be 0 for 0xb4 */
}

void t_ispunct_0xb5()
{
    uassert_int_equal(ispunct(181), 0); /* ispunct should be 0 for 0xb5 */
}

void t_ispunct_0xb6()
{
    uassert_int_equal(ispunct(182), 0); /* ispunct should be 0 for 0xb6 */
}

void t_ispunct_0xb7()
{
    uassert_int_equal(ispunct(183), 0); /* ispunct should be 0 for 0xb7 */
}

void t_ispunct_0xb8()
{
    uassert_int_equal(ispunct(184), 0); /* ispunct should be 0 for 0xb8 */
}

void t_ispunct_0xb9()
{
    uassert_int_equal(ispunct(185), 0); /* ispunct should be 0 for 0xb9 */
}

void t_ispunct_0xba()
{
    uassert_int_equal(ispunct(186), 0); /* ispunct should be 0 for 0xba */
}

void t_ispunct_0xbb()
{
    uassert_int_equal(ispunct(187), 0); /* ispunct should be 0 for 0xbb */
}

void t_ispunct_0xbc()
{
    uassert_int_equal(ispunct(188), 0); /* ispunct should be 0 for 0xbc */
}

void t_ispunct_0xbd()
{
    uassert_int_equal(ispunct(189), 0); /* ispunct should be 0 for 0xbd */
}

void t_ispunct_0xbe()
{
    uassert_int_equal(ispunct(190), 0); /* ispunct should be 0 for 0xbe */
}

void t_ispunct_0xbf()
{
    uassert_int_equal(ispunct(191), 0); /* ispunct should be 0 for 0xbf */
}

void t_ispunct_0xc0()
{
    uassert_int_equal(ispunct(192), 0); /* ispunct should be 0 for 0xc0 */
}

void t_ispunct_0xc1()
{
    uassert_int_equal(ispunct(193), 0); /* ispunct should be 0 for 0xc1 */
}

void t_ispunct_0xc2()
{
    uassert_int_equal(ispunct(194), 0); /* ispunct should be 0 for 0xc2 */
}

void t_ispunct_0xc3()
{
    uassert_int_equal(ispunct(195), 0); /* ispunct should be 0 for 0xc3 */
}

void t_ispunct_0xc4()
{
    uassert_int_equal(ispunct(196), 0); /* ispunct should be 0 for 0xc4 */
}

void t_ispunct_0xc5()
{
    uassert_int_equal(ispunct(197), 0); /* ispunct should be 0 for 0xc5 */
}

void t_ispunct_0xc6()
{
    uassert_int_equal(ispunct(198), 0); /* ispunct should be 0 for 0xc6 */
}

void t_ispunct_0xc7()
{
    uassert_int_equal(ispunct(199), 0); /* ispunct should be 0 for 0xc7 */
}

void t_ispunct_0xc8()
{
    uassert_int_equal(ispunct(200), 0); /* ispunct should be 0 for 0xc8 */
}

void t_ispunct_0xc9()
{
    uassert_int_equal(ispunct(201), 0); /* ispunct should be 0 for 0xc9 */
}

void t_ispunct_0xca()
{
    uassert_int_equal(ispunct(202), 0); /* ispunct should be 0 for 0xca */
}

void t_ispunct_0xcb()
{
    uassert_int_equal(ispunct(203), 0); /* ispunct should be 0 for 0xcb */
}

void t_ispunct_0xcc()
{
    uassert_int_equal(ispunct(204), 0); /* ispunct should be 0 for 0xcc */
}

void t_ispunct_0xcd()
{
    uassert_int_equal(ispunct(205), 0); /* ispunct should be 0 for 0xcd */
}

void t_ispunct_0xce()
{
    uassert_int_equal(ispunct(206), 0); /* ispunct should be 0 for 0xce */
}

void t_ispunct_0xcf()
{
    uassert_int_equal(ispunct(207), 0); /* ispunct should be 0 for 0xcf */
}

void t_ispunct_0xd0()
{
    uassert_int_equal(ispunct(208), 0); /* ispunct should be 0 for 0xd0 */
}

void t_ispunct_0xd1()
{
    uassert_int_equal(ispunct(209), 0); /* ispunct should be 0 for 0xd1 */
}

void t_ispunct_0xd2()
{
    uassert_int_equal(ispunct(210), 0); /* ispunct should be 0 for 0xd2 */
}

void t_ispunct_0xd3()
{
    uassert_int_equal(ispunct(211), 0); /* ispunct should be 0 for 0xd3 */
}

void t_ispunct_0xd4()
{
    uassert_int_equal(ispunct(212), 0); /* ispunct should be 0 for 0xd4 */
}

void t_ispunct_0xd5()
{
    uassert_int_equal(ispunct(213), 0); /* ispunct should be 0 for 0xd5 */
}

void t_ispunct_0xd6()
{
    uassert_int_equal(ispunct(214), 0); /* ispunct should be 0 for 0xd6 */
}

void t_ispunct_0xd7()
{
    uassert_int_equal(ispunct(215), 0); /* ispunct should be 0 for 0xd7 */
}

void t_ispunct_0xd8()
{
    uassert_int_equal(ispunct(216), 0); /* ispunct should be 0 for 0xd8 */
}

void t_ispunct_0xd9()
{
    uassert_int_equal(ispunct(217), 0); /* ispunct should be 0 for 0xd9 */
}

void t_ispunct_0xda()
{
    uassert_int_equal(ispunct(218), 0); /* ispunct should be 0 for 0xda */
}

void t_ispunct_0xdb()
{
    uassert_int_equal(ispunct(219), 0); /* ispunct should be 0 for 0xdb */
}

void t_ispunct_0xdc()
{
    uassert_int_equal(ispunct(220), 0); /* ispunct should be 0 for 0xdc */
}

void t_ispunct_0xdd()
{
    uassert_int_equal(ispunct(221), 0); /* ispunct should be 0 for 0xdd */
}

void t_ispunct_0xde()
{
    uassert_int_equal(ispunct(222), 0); /* ispunct should be 0 for 0xde */
}

void t_ispunct_0xdf()
{
    uassert_int_equal(ispunct(223), 0); /* ispunct should be 0 for 0xdf */
}

void t_ispunct_0xe0()
{
    uassert_int_equal(ispunct(224), 0); /* ispunct should be 0 for 0xe0 */
}

void t_ispunct_0xe1()
{
    uassert_int_equal(ispunct(225), 0); /* ispunct should be 0 for 0xe1 */
}

void t_ispunct_0xe2()
{
    uassert_int_equal(ispunct(226), 0); /* ispunct should be 0 for 0xe2 */
}

void t_ispunct_0xe3()
{
    uassert_int_equal(ispunct(227), 0); /* ispunct should be 0 for 0xe3 */
}

void t_ispunct_0xe4()
{
    uassert_int_equal(ispunct(228), 0); /* ispunct should be 0 for 0xe4 */
}

void t_ispunct_0xe5()
{
    uassert_int_equal(ispunct(229), 0); /* ispunct should be 0 for 0xe5 */
}

void t_ispunct_0xe6()
{
    uassert_int_equal(ispunct(230), 0); /* ispunct should be 0 for 0xe6 */
}

void t_ispunct_0xe7()
{
    uassert_int_equal(ispunct(231), 0); /* ispunct should be 0 for 0xe7 */
}

void t_ispunct_0xe8()
{
    uassert_int_equal(ispunct(232), 0); /* ispunct should be 0 for 0xe8 */
}

void t_ispunct_0xe9()
{
    uassert_int_equal(ispunct(233), 0); /* ispunct should be 0 for 0xe9 */
}

void t_ispunct_0xea()
{
    uassert_int_equal(ispunct(234), 0); /* ispunct should be 0 for 0xea */
}

void t_ispunct_0xeb()
{
    uassert_int_equal(ispunct(235), 0); /* ispunct should be 0 for 0xeb */
}

void t_ispunct_0xec()
{
    uassert_int_equal(ispunct(236), 0); /* ispunct should be 0 for 0xec */
}

void t_ispunct_0xed()
{
    uassert_int_equal(ispunct(237), 0); /* ispunct should be 0 for 0xed */
}

void t_ispunct_0xee()
{
    uassert_int_equal(ispunct(238), 0); /* ispunct should be 0 for 0xee */
}

void t_ispunct_0xef()
{
    uassert_int_equal(ispunct(239), 0); /* ispunct should be 0 for 0xef */
}

void t_ispunct_0xf0()
{
    uassert_int_equal(ispunct(240), 0); /* ispunct should be 0 for 0xf0 */
}

void t_ispunct_0xf1()
{
    uassert_int_equal(ispunct(241), 0); /* ispunct should be 0 for 0xf1 */
}

void t_ispunct_0xf2()
{
    uassert_int_equal(ispunct(242), 0); /* ispunct should be 0 for 0xf2 */
}

void t_ispunct_0xf3()
{
    uassert_int_equal(ispunct(243), 0); /* ispunct should be 0 for 0xf3 */
}

void t_ispunct_0xf4()
{
    uassert_int_equal(ispunct(244), 0); /* ispunct should be 0 for 0xf4 */
}

void t_ispunct_0xf5()
{
    uassert_int_equal(ispunct(245), 0); /* ispunct should be 0 for 0xf5 */
}

void t_ispunct_0xf6()
{
    uassert_int_equal(ispunct(246), 0); /* ispunct should be 0 for 0xf6 */
}

void t_ispunct_0xf7()
{
    uassert_int_equal(ispunct(247), 0); /* ispunct should be 0 for 0xf7 */
}

void t_ispunct_0xf8()
{
    uassert_int_equal(ispunct(248), 0); /* ispunct should be 0 for 0xf8 */
}

void t_ispunct_0xf9()
{
    uassert_int_equal(ispunct(249), 0); /* ispunct should be 0 for 0xf9 */
}

void t_ispunct_0xfa()
{
    uassert_int_equal(ispunct(250), 0); /* ispunct should be 0 for 0xfa */
}

void t_ispunct_0xfb()
{
    uassert_int_equal(ispunct(251), 0); /* ispunct should be 0 for 0xfb */
}

void t_ispunct_0xfc()
{
    uassert_int_equal(ispunct(252), 0); /* ispunct should be 0 for 0xfc */
}

void t_ispunct_0xfd()
{
    uassert_int_equal(ispunct(253), 0); /* ispunct should be 0 for 0xfd */
}

void t_ispunct_0xfe()
{
    uassert_int_equal(ispunct(254), 0); /* ispunct should be 0 for 0xfe */
}

void t_ispunct_0xff()
{
    uassert_int_equal(ispunct(255), 0); /* ispunct should be 0 for 0xff */
}



static int testcase(void)
{
    t_ispunct_0x00();
    t_ispunct_0x01();
    t_ispunct_0x02();
    t_ispunct_0x03();
    t_ispunct_0x04();
    t_ispunct_0x05();
    t_ispunct_0x06();
    t_ispunct_0x07();
    t_ispunct_0x08();
    t_ispunct_0x09();
    t_ispunct_0x0a();
    t_ispunct_0x0b();
    t_ispunct_0x0c();
    t_ispunct_0x0d();
    t_ispunct_0x0e();
    t_ispunct_0x0f();
    t_ispunct_0x10();
    t_ispunct_0x11();
    t_ispunct_0x12();
    t_ispunct_0x13();
    t_ispunct_0x14();
    t_ispunct_0x15();
    t_ispunct_0x16();
    t_ispunct_0x17();
    t_ispunct_0x18();
    t_ispunct_0x19();
    t_ispunct_0x1a();
    t_ispunct_0x1b();
    t_ispunct_0x1c();
    t_ispunct_0x1d();
    t_ispunct_0x1e();
    t_ispunct_0x1f();
    t_ispunct_0x20();
    t_ispunct_0x21();
    t_ispunct_0x22();
    t_ispunct_0x23();
    t_ispunct_0x24();
    t_ispunct_0x25();
    t_ispunct_0x26();
    t_ispunct_0x27();
    t_ispunct_0x28();
    t_ispunct_0x29();
    t_ispunct_0x2a();
    t_ispunct_0x2b();
    t_ispunct_0x2c();
    t_ispunct_0x2d();
    t_ispunct_0x2e();
    t_ispunct_0x2f();
    t_ispunct_0x30();
    t_ispunct_0x31();
    t_ispunct_0x32();
    t_ispunct_0x33();
    t_ispunct_0x34();
    t_ispunct_0x35();
    t_ispunct_0x36();
    t_ispunct_0x37();
    t_ispunct_0x38();
    t_ispunct_0x39();
    t_ispunct_0x3a();
    t_ispunct_0x3b();
    t_ispunct_0x3c();
    t_ispunct_0x3d();
    t_ispunct_0x3e();
    t_ispunct_0x3f();
    t_ispunct_0x40();
    t_ispunct_0x41();
    t_ispunct_0x42();
    t_ispunct_0x43();
    t_ispunct_0x44();
    t_ispunct_0x45();
    t_ispunct_0x46();
    t_ispunct_0x47();
    t_ispunct_0x48();
    t_ispunct_0x49();
    t_ispunct_0x4a();
    t_ispunct_0x4b();
    t_ispunct_0x4c();
    t_ispunct_0x4d();
    t_ispunct_0x4e();
    t_ispunct_0x4f();
    t_ispunct_0x50();
    t_ispunct_0x51();
    t_ispunct_0x52();
    t_ispunct_0x53();
    t_ispunct_0x54();
    t_ispunct_0x55();
    t_ispunct_0x56();
    t_ispunct_0x57();
    t_ispunct_0x58();
    t_ispunct_0x59();
    t_ispunct_0x5a();
    t_ispunct_0x5b();
    t_ispunct_0x5c();
    t_ispunct_0x5d();
    t_ispunct_0x5e();
    t_ispunct_0x5f();
    t_ispunct_0x60();
    t_ispunct_0x61();
    t_ispunct_0x62();
    t_ispunct_0x63();
    t_ispunct_0x64();
    t_ispunct_0x65();
    t_ispunct_0x66();
    t_ispunct_0x67();
    t_ispunct_0x68();
    t_ispunct_0x69();
    t_ispunct_0x6a();
    t_ispunct_0x6b();
    t_ispunct_0x6c();
    t_ispunct_0x6d();
    t_ispunct_0x6e();
    t_ispunct_0x6f();
    t_ispunct_0x70();
    t_ispunct_0x71();
    t_ispunct_0x72();
    t_ispunct_0x73();
    t_ispunct_0x74();
    t_ispunct_0x75();
    t_ispunct_0x76();
    t_ispunct_0x77();
    t_ispunct_0x78();
    t_ispunct_0x79();
    t_ispunct_0x7a();
    t_ispunct_0x7b();
    t_ispunct_0x7c();
    t_ispunct_0x7d();
    t_ispunct_0x7e();
    t_ispunct_0x7f();
    t_ispunct_0x80();
    t_ispunct_0x81();
    t_ispunct_0x82();
    t_ispunct_0x83();
    t_ispunct_0x84();
    t_ispunct_0x85();
    t_ispunct_0x86();
    t_ispunct_0x87();
    t_ispunct_0x88();
    t_ispunct_0x89();
    t_ispunct_0x8a();
    t_ispunct_0x8b();
    t_ispunct_0x8c();
    t_ispunct_0x8d();
    t_ispunct_0x8e();
    t_ispunct_0x8f();
    t_ispunct_0x90();
    t_ispunct_0x91();
    t_ispunct_0x92();
    t_ispunct_0x93();
    t_ispunct_0x94();
    t_ispunct_0x95();
    t_ispunct_0x96();
    t_ispunct_0x97();
    t_ispunct_0x98();
    t_ispunct_0x99();
    t_ispunct_0x9a();
    t_ispunct_0x9b();
    t_ispunct_0x9c();
    t_ispunct_0x9d();
    t_ispunct_0x9e();
    t_ispunct_0x9f();
    t_ispunct_0xa0();
    t_ispunct_0xa1();
    t_ispunct_0xa2();
    t_ispunct_0xa3();
    t_ispunct_0xa4();
    t_ispunct_0xa5();
    t_ispunct_0xa6();
    t_ispunct_0xa7();
    t_ispunct_0xa8();
    t_ispunct_0xa9();
    t_ispunct_0xaa();
    t_ispunct_0xab();
    t_ispunct_0xac();
    t_ispunct_0xad();
    t_ispunct_0xae();
    t_ispunct_0xaf();
    t_ispunct_0xb0();
    t_ispunct_0xb1();
    t_ispunct_0xb2();
    t_ispunct_0xb3();
    t_ispunct_0xb4();
    t_ispunct_0xb5();
    t_ispunct_0xb6();
    t_ispunct_0xb7();
    t_ispunct_0xb8();
    t_ispunct_0xb9();
    t_ispunct_0xba();
    t_ispunct_0xbb();
    t_ispunct_0xbc();
    t_ispunct_0xbd();
    t_ispunct_0xbe();
    t_ispunct_0xbf();
    t_ispunct_0xc0();
    t_ispunct_0xc1();
    t_ispunct_0xc2();
    t_ispunct_0xc3();
    t_ispunct_0xc4();
    t_ispunct_0xc5();
    t_ispunct_0xc6();
    t_ispunct_0xc7();
    t_ispunct_0xc8();
    t_ispunct_0xc9();
    t_ispunct_0xca();
    t_ispunct_0xcb();
    t_ispunct_0xcc();
    t_ispunct_0xcd();
    t_ispunct_0xce();
    t_ispunct_0xcf();
    t_ispunct_0xd0();
    t_ispunct_0xd1();
    t_ispunct_0xd2();
    t_ispunct_0xd3();
    t_ispunct_0xd4();
    t_ispunct_0xd5();
    t_ispunct_0xd6();
    t_ispunct_0xd7();
    t_ispunct_0xd8();
    t_ispunct_0xd9();
    t_ispunct_0xda();
    t_ispunct_0xdb();
    t_ispunct_0xdc();
    t_ispunct_0xdd();
    t_ispunct_0xde();
    t_ispunct_0xdf();
    t_ispunct_0xe0();
    t_ispunct_0xe1();
    t_ispunct_0xe2();
    t_ispunct_0xe3();
    t_ispunct_0xe4();
    t_ispunct_0xe5();
    t_ispunct_0xe6();
    t_ispunct_0xe7();
    t_ispunct_0xe8();
    t_ispunct_0xe9();
    t_ispunct_0xea();
    t_ispunct_0xeb();
    t_ispunct_0xec();
    t_ispunct_0xed();
    t_ispunct_0xee();
    t_ispunct_0xef();
    t_ispunct_0xf0();
    t_ispunct_0xf1();
    t_ispunct_0xf2();
    t_ispunct_0xf3();
    t_ispunct_0xf4();
    t_ispunct_0xf5();
    t_ispunct_0xf6();
    t_ispunct_0xf7();
    t_ispunct_0xf8();
    t_ispunct_0xf9();
    t_ispunct_0xfa();
    t_ispunct_0xfb();
    t_ispunct_0xfc();
    t_ispunct_0xfd();
    t_ispunct_0xfe();
    t_ispunct_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
