#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))


void t_isprint_0x00()
{
    uassert_int_equal(isprint(0), 0);  /* isprint should be 0 for 0x00 */
}

void t_isprint_0x01()
{
    uassert_int_equal(isprint(1), 0);  /* isprint should be 0 for 0x01 */
}

void t_isprint_0x02()
{
    uassert_int_equal(isprint(2), 0);  /* isprint should be 0 for 0x02 */
}

void t_isprint_0x03()
{
    uassert_int_equal(isprint(3), 0);  /* isprint should be 0 for 0x03 */
}

void t_isprint_0x04()
{
    uassert_int_equal(isprint(4), 0);  /* isprint should be 0 for 0x04 */
}

void t_isprint_0x05()
{
    uassert_int_equal(isprint(5), 0);  /* isprint should be 0 for 0x05 */
}

void t_isprint_0x06()
{
    uassert_int_equal(isprint(6), 0);  /* isprint should be 0 for 0x06 */
}

void t_isprint_0x07()
{
    uassert_int_equal(isprint(7), 0);  /* isprint should be 0 for 0x07 */
}

void t_isprint_0x08()
{
    uassert_int_equal(isprint(8), 0);  /* isprint should be 0 for 0x08 */
}

void t_isprint_0x09()
{
    uassert_int_equal(isprint(9), 0);  /* isprint should be 0 for 0x09 */
}

void t_isprint_0x0a()
{
    uassert_int_equal(isprint(10), 0);  /* isprint should be 0 for 0x0a */
}

void t_isprint_0x0b()
{
    uassert_int_equal(isprint(11), 0);  /* isprint should be 0 for 0x0b */
}

void t_isprint_0x0c()
{
    uassert_int_equal(isprint(12), 0);  /* isprint should be 0 for 0x0c */
}

void t_isprint_0x0d()
{
    uassert_int_equal(isprint(13), 0);  /* isprint should be 0 for 0x0d */
}

void t_isprint_0x0e()
{
    uassert_int_equal(isprint(14), 0);  /* isprint should be 0 for 0x0e */
}

void t_isprint_0x0f()
{
    uassert_int_equal(isprint(15), 0);  /* isprint should be 0 for 0x0f */
}

void t_isprint_0x10()
{
    uassert_int_equal(isprint(16), 0);  /* isprint should be 0 for 0x10 */
}

void t_isprint_0x11()
{
    uassert_int_equal(isprint(17), 0);  /* isprint should be 0 for 0x11 */
}

void t_isprint_0x12()
{
    uassert_int_equal(isprint(18), 0);  /* isprint should be 0 for 0x12 */
}

void t_isprint_0x13()
{
    uassert_int_equal(isprint(19), 0);  /* isprint should be 0 for 0x13 */
}

void t_isprint_0x14()
{
    uassert_int_equal(isprint(20), 0);  /* isprint should be 0 for 0x14 */
}

void t_isprint_0x15()
{
    uassert_int_equal(isprint(21), 0);  /* isprint should be 0 for 0x15 */
}

void t_isprint_0x16()
{
    uassert_int_equal(isprint(22), 0);  /* isprint should be 0 for 0x16 */
}

void t_isprint_0x17()
{
    uassert_int_equal(isprint(23), 0);  /* isprint should be 0 for 0x17 */
}

void t_isprint_0x18()
{
    uassert_int_equal(isprint(24), 0);  /* isprint should be 0 for 0x18 */
}

void t_isprint_0x19()
{
    uassert_int_equal(isprint(25), 0);  /* isprint should be 0 for 0x19 */
}

void t_isprint_0x1a()
{
    uassert_int_equal(isprint(26), 0);  /* isprint should be 0 for 0x1a */
}

void t_isprint_0x1b()
{
    uassert_int_equal(isprint(27), 0);  /* isprint should be 0 for 0x1b */
}

void t_isprint_0x1c()
{
    uassert_int_equal(isprint(28), 0);  /* isprint should be 0 for 0x1c */
}

void t_isprint_0x1d()
{
    uassert_int_equal(isprint(29), 0);  /* isprint should be 0 for 0x1d */
}

void t_isprint_0x1e()
{
    uassert_int_equal(isprint(30), 0);  /* isprint should be 0 for 0x1e */
}

void t_isprint_0x1f()
{
    uassert_int_equal(isprint(31), 0);  /* isprint should be 0 for 0x1f */
}

void t_isprint_0x20()
{
    uassert_int_equal(isprint(32), 1); /* isprint should be 1 for   */
}

void t_isprint_0x21()
{
    uassert_int_equal(isprint(33), 1); /* isprint should be 1 for ! */
}

void t_isprint_0x22()
{
    uassert_int_equal(isprint(34), 1); /* isprint should be 1 for 0x22 */
}

void t_isprint_0x23()
{
    uassert_int_equal(isprint(35), 1); /* isprint should be 1 for # */
}

void t_isprint_0x24()
{
    uassert_int_equal(isprint(36), 1); /* isprint should be 1 for $ */
}

void t_isprint_0x25()
{
    uassert_int_equal(isprint(37), 1); /* isprint should be 1 for % */
}

void t_isprint_0x26()
{
    uassert_int_equal(isprint(38), 1); /* isprint should be 1 for & */
}

void t_isprint_0x27()
{
    uassert_int_equal(isprint(39), 1); /* isprint should be 1 for ' */
}

void t_isprint_0x28()
{
    uassert_int_equal(isprint(40), 1); /* isprint should be 1 for ( */
}

void t_isprint_0x29()
{
    uassert_int_equal(isprint(41), 1); /* isprint should be 1 for ) */
}

void t_isprint_0x2a()
{
    uassert_int_equal(isprint(42), 1); /* isprint should be 1 for * */
}

void t_isprint_0x2b()
{
    uassert_int_equal(isprint(43), 1); /* isprint should be 1 for + */
}

void t_isprint_0x2c()
{
    uassert_int_equal(isprint(44), 1); /* isprint should be 1 for , */
}

void t_isprint_0x2d()
{
    uassert_int_equal(isprint(45), 1); /* isprint should be 1 for - */
}

void t_isprint_0x2e()
{
    uassert_int_equal(isprint(46), 1); /* isprint should be 1 for . */
}

void t_isprint_0x2f()
{
    uassert_int_equal(isprint(47), 1); /* isprint should be 1 for / */
}

void t_isprint_0x30()
{
    uassert_int_equal(isprint(48), 1); /* isprint should be 1 for 0 */
}

void t_isprint_0x31()
{
    uassert_int_equal(isprint(49), 1); /* isprint should be 1 for 1 */
}

void t_isprint_0x32()
{
    uassert_int_equal(isprint(50), 1); /* isprint should be 1 for 2 */
}

void t_isprint_0x33()
{
    uassert_int_equal(isprint(51), 1); /* isprint should be 1 for 3 */
}

void t_isprint_0x34()
{
    uassert_int_equal(isprint(52), 1); /* isprint should be 1 for 4 */
}

void t_isprint_0x35()
{
    uassert_int_equal(isprint(53), 1); /* isprint should be 1 for 5 */
}

void t_isprint_0x36()
{
    uassert_int_equal(isprint(54), 1); /* isprint should be 1 for 6 */
}

void t_isprint_0x37()
{
    uassert_int_equal(isprint(55), 1); /* isprint should be 1 for 7 */
}

void t_isprint_0x38()
{
    uassert_int_equal(isprint(56), 1); /* isprint should be 1 for 8 */
}

void t_isprint_0x39()
{
    uassert_int_equal(isprint(57), 1); /* isprint should be 1 for 9 */
}

void t_isprint_0x3a()
{
    uassert_int_equal(isprint(58), 1); /* isprint should be 1 for : */
}

void t_isprint_0x3b()
{
    uassert_int_equal(isprint(59), 1); /* isprint should be 1 for ; */
}

void t_isprint_0x3c()
{
    uassert_int_equal(isprint(60), 1); /* isprint should be 1 for < */
}

void t_isprint_0x3d()
{
    uassert_int_equal(isprint(61), 1); /* isprint should be 1 for = */
}

void t_isprint_0x3e()
{
    uassert_int_equal(isprint(62), 1); /* isprint should be 1 for > */
}

void t_isprint_0x3f()
{
    uassert_int_equal(isprint(63), 1); /* isprint should be 1 for ? */
}

void t_isprint_0x40()
{
    uassert_int_equal(isprint(64), 1); /* isprint should be 1 for @ */
}

void t_isprint_0x41()
{
    uassert_int_equal(isprint(65), 1); /* isprint should be 1 for A */
}

void t_isprint_0x42()
{
    uassert_int_equal(isprint(66), 1); /* isprint should be 1 for B */
}

void t_isprint_0x43()
{
    uassert_int_equal(isprint(67), 1); /* isprint should be 1 for C */
}

void t_isprint_0x44()
{
    uassert_int_equal(isprint(68), 1); /* isprint should be 1 for D */
}

void t_isprint_0x45()
{
    uassert_int_equal(isprint(69), 1); /* isprint should be 1 for E */
}

void t_isprint_0x46()
{
    uassert_int_equal(isprint(70), 1); /* isprint should be 1 for F */
}

void t_isprint_0x47()
{
    uassert_int_equal(isprint(71), 1); /* isprint should be 1 for G */
}

void t_isprint_0x48()
{
    uassert_int_equal(isprint(72), 1); /* isprint should be 1 for H */
}

void t_isprint_0x49()
{
    uassert_int_equal(isprint(73), 1); /* isprint should be 1 for I */
}

void t_isprint_0x4a()
{
    uassert_int_equal(isprint(74), 1); /* isprint should be 1 for J */
}

void t_isprint_0x4b()
{
    uassert_int_equal(isprint(75), 1); /* isprint should be 1 for K */
}

void t_isprint_0x4c()
{
    uassert_int_equal(isprint(76), 1); /* isprint should be 1 for L */
}

void t_isprint_0x4d()
{
    uassert_int_equal(isprint(77), 1); /* isprint should be 1 for M */
}

void t_isprint_0x4e()
{
    uassert_int_equal(isprint(78), 1); /* isprint should be 1 for N */
}

void t_isprint_0x4f()
{
    uassert_int_equal(isprint(79), 1); /* isprint should be 1 for O */
}

void t_isprint_0x50()
{
    uassert_int_equal(isprint(80), 1); /* isprint should be 1 for P */
}

void t_isprint_0x51()
{
    uassert_int_equal(isprint(81), 1); /* isprint should be 1 for Q */
}

void t_isprint_0x52()
{
    uassert_int_equal(isprint(82), 1); /* isprint should be 1 for R */
}

void t_isprint_0x53()
{
    uassert_int_equal(isprint(83), 1); /* isprint should be 1 for S */
}

void t_isprint_0x54()
{
    uassert_int_equal(isprint(84), 1); /* isprint should be 1 for T */
}

void t_isprint_0x55()
{
    uassert_int_equal(isprint(85), 1); /* isprint should be 1 for U */
}

void t_isprint_0x56()
{
    uassert_int_equal(isprint(86), 1); /* isprint should be 1 for V */
}

void t_isprint_0x57()
{
    uassert_int_equal(isprint(87), 1); /* isprint should be 1 for W */
}

void t_isprint_0x58()
{
    uassert_int_equal(isprint(88), 1); /* isprint should be 1 for X */
}

void t_isprint_0x59()
{
    uassert_int_equal(isprint(89), 1); /* isprint should be 1 for Y */
}

void t_isprint_0x5a()
{
    uassert_int_equal(isprint(90), 1); /* isprint should be 1 for Z */
}

void t_isprint_0x5b()
{
    uassert_int_equal(isprint(91), 1); /* isprint should be 1 for [ */
}

void t_isprint_0x5c()
{
    uassert_int_equal(isprint(92), 1); /* isprint should be 1 for 0x5c */
}

void t_isprint_0x5d()
{
    uassert_int_equal(isprint(93), 1); /* isprint should be 1 for ] */
}

void t_isprint_0x5e()
{
    uassert_int_equal(isprint(94), 1); /* isprint should be 1 for ^ */
}

void t_isprint_0x5f()
{
    uassert_int_equal(isprint(95), 1); /* isprint should be 1 for _ */
}

void t_isprint_0x60()
{
    uassert_int_equal(isprint(96), 1); /* isprint should be 1 for ` */
}

void t_isprint_0x61()
{
    uassert_int_equal(isprint(97), 1); /* isprint should be 1 for a */
}

void t_isprint_0x62()
{
    uassert_int_equal(isprint(98), 1); /* isprint should be 1 for b */
}

void t_isprint_0x63()
{
    uassert_int_equal(isprint(99), 1); /* isprint should be 1 for c */
}

void t_isprint_0x64()
{
    uassert_int_equal(isprint(100), 1); /* isprint should be 1 for d */
}

void t_isprint_0x65()
{
    uassert_int_equal(isprint(101), 1); /* isprint should be 1 for e */
}

void t_isprint_0x66()
{
    uassert_int_equal(isprint(102), 1); /* isprint should be 1 for f */
}

void t_isprint_0x67()
{
    uassert_int_equal(isprint(103), 1); /* isprint should be 1 for g */
}

void t_isprint_0x68()
{
    uassert_int_equal(isprint(104), 1); /* isprint should be 1 for h */
}

void t_isprint_0x69()
{
    uassert_int_equal(isprint(105), 1); /* isprint should be 1 for i */
}

void t_isprint_0x6a()
{
    uassert_int_equal(isprint(106), 1); /* isprint should be 1 for j */
}

void t_isprint_0x6b()
{
    uassert_int_equal(isprint(107), 1); /* isprint should be 1 for k */
}

void t_isprint_0x6c()
{
    uassert_int_equal(isprint(108), 1); /* isprint should be 1 for l */
}

void t_isprint_0x6d()
{
    uassert_int_equal(isprint(109), 1); /* isprint should be 1 for m */
}

void t_isprint_0x6e()
{
    uassert_int_equal(isprint(110), 1); /* isprint should be 1 for n */
}

void t_isprint_0x6f()
{
    uassert_int_equal(isprint(111), 1); /* isprint should be 1 for o */
}

void t_isprint_0x70()
{
    uassert_int_equal(isprint(112), 1); /* isprint should be 1 for p */
}

void t_isprint_0x71()
{
    uassert_int_equal(isprint(113), 1); /* isprint should be 1 for q */
}

void t_isprint_0x72()
{
    uassert_int_equal(isprint(114), 1); /* isprint should be 1 for r */
}

void t_isprint_0x73()
{
    uassert_int_equal(isprint(115), 1); /* isprint should be 1 for s */
}

void t_isprint_0x74()
{
    uassert_int_equal(isprint(116), 1); /* isprint should be 1 for t */
}

void t_isprint_0x75()
{
    uassert_int_equal(isprint(117), 1); /* isprint should be 1 for u */
}

void t_isprint_0x76()
{
    uassert_int_equal(isprint(118), 1); /* isprint should be 1 for v */
}

void t_isprint_0x77()
{
    uassert_int_equal(isprint(119), 1); /* isprint should be 1 for w */
}

void t_isprint_0x78()
{
    uassert_int_equal(isprint(120), 1); /* isprint should be 1 for x */
}

void t_isprint_0x79()
{
    uassert_int_equal(isprint(121), 1); /* isprint should be 1 for y */
}

void t_isprint_0x7a()
{
    uassert_int_equal(isprint(122), 1); /* isprint should be 1 for z */
}

void t_isprint_0x7b()
{
    uassert_int_equal(isprint(123), 1); /* isprint should be 1 for { */
}

void t_isprint_0x7c()
{
    uassert_int_equal(isprint(124), 1); /* isprint should be 1 for | */
}

void t_isprint_0x7d()
{
    uassert_int_equal(isprint(125), 1); /* isprint should be 1 for } */
}

void t_isprint_0x7e()
{
    uassert_int_equal(isprint(126), 1); /* isprint should be 1 for ~ */
}

void t_isprint_0x7f()
{
    uassert_int_equal(isprint(127), 0);  /* isprint should be 0 for 0x7f */
}

void t_isprint_0x80()
{
    uassert_int_equal(isprint(128), 0);  /* isprint should be 0 for 0x80 */
}

void t_isprint_0x81()
{
    uassert_int_equal(isprint(129), 0);  /* isprint should be 0 for 0x81 */
}

void t_isprint_0x82()
{
    uassert_int_equal(isprint(130), 0);  /* isprint should be 0 for 0x82 */
}

void t_isprint_0x83()
{
    uassert_int_equal(isprint(131), 0);  /* isprint should be 0 for 0x83 */
}

void t_isprint_0x84()
{
    uassert_int_equal(isprint(132), 0);  /* isprint should be 0 for 0x84 */
}

void t_isprint_0x85()
{
    uassert_int_equal(isprint(133), 0);  /* isprint should be 0 for 0x85 */
}

void t_isprint_0x86()
{
    uassert_int_equal(isprint(134), 0);  /* isprint should be 0 for 0x86 */
}

void t_isprint_0x87()
{
    uassert_int_equal(isprint(135), 0);  /* isprint should be 0 for 0x87 */
}

void t_isprint_0x88()
{
    uassert_int_equal(isprint(136), 0);  /* isprint should be 0 for 0x88 */
}

void t_isprint_0x89()
{
    uassert_int_equal(isprint(137), 0);  /* isprint should be 0 for 0x89 */
}

void t_isprint_0x8a()
{
    uassert_int_equal(isprint(138), 0);  /* isprint should be 0 for 0x8a */
}

void t_isprint_0x8b()
{
    uassert_int_equal(isprint(139), 0);  /* isprint should be 0 for 0x8b */
}

void t_isprint_0x8c()
{
    uassert_int_equal(isprint(140), 0);  /* isprint should be 0 for 0x8c */
}

void t_isprint_0x8d()
{
    uassert_int_equal(isprint(141), 0);  /* isprint should be 0 for 0x8d */
}

void t_isprint_0x8e()
{
    uassert_int_equal(isprint(142), 0);  /* isprint should be 0 for 0x8e */
}

void t_isprint_0x8f()
{
    uassert_int_equal(isprint(143), 0);  /* isprint should be 0 for 0x8f */
}

void t_isprint_0x90()
{
    uassert_int_equal(isprint(144), 0);  /* isprint should be 0 for 0x90 */
}

void t_isprint_0x91()
{
    uassert_int_equal(isprint(145), 0);  /* isprint should be 0 for 0x91 */
}

void t_isprint_0x92()
{
    uassert_int_equal(isprint(146), 0);  /* isprint should be 0 for 0x92 */
}

void t_isprint_0x93()
{
    uassert_int_equal(isprint(147), 0);  /* isprint should be 0 for 0x93 */
}

void t_isprint_0x94()
{
    uassert_int_equal(isprint(148), 0);  /* isprint should be 0 for 0x94 */
}

void t_isprint_0x95()
{
    uassert_int_equal(isprint(149), 0);  /* isprint should be 0 for 0x95 */
}

void t_isprint_0x96()
{
    uassert_int_equal(isprint(150), 0);  /* isprint should be 0 for 0x96 */
}

void t_isprint_0x97()
{
    uassert_int_equal(isprint(151), 0);  /* isprint should be 0 for 0x97 */
}

void t_isprint_0x98()
{
    uassert_int_equal(isprint(152), 0);  /* isprint should be 0 for 0x98 */
}

void t_isprint_0x99()
{
    uassert_int_equal(isprint(153), 0);  /* isprint should be 0 for 0x99 */
}

void t_isprint_0x9a()
{
    uassert_int_equal(isprint(154), 0);  /* isprint should be 0 for 0x9a */
}

void t_isprint_0x9b()
{
    uassert_int_equal(isprint(155), 0);  /* isprint should be 0 for 0x9b */
}

void t_isprint_0x9c()
{
    uassert_int_equal(isprint(156), 0);  /* isprint should be 0 for 0x9c */
}

void t_isprint_0x9d()
{
    uassert_int_equal(isprint(157), 0);  /* isprint should be 0 for 0x9d */
}

void t_isprint_0x9e()
{
    uassert_int_equal(isprint(158), 0);  /* isprint should be 0 for 0x9e */
}

void t_isprint_0x9f()
{
    uassert_int_equal(isprint(159), 0);  /* isprint should be 0 for 0x9f */
}

void t_isprint_0xa0()
{
    uassert_int_equal(isprint(160), 0);  /* isprint should be 0 for 0xa0 */
}

void t_isprint_0xa1()
{
    uassert_int_equal(isprint(161), 0);  /* isprint should be 0 for 0xa1 */
}

void t_isprint_0xa2()
{
    uassert_int_equal(isprint(162), 0);  /* isprint should be 0 for 0xa2 */
}

void t_isprint_0xa3()
{
    uassert_int_equal(isprint(163), 0);  /* isprint should be 0 for 0xa3 */
}

void t_isprint_0xa4()
{
    uassert_int_equal(isprint(164), 0);  /* isprint should be 0 for 0xa4 */
}

void t_isprint_0xa5()
{
    uassert_int_equal(isprint(165), 0);  /* isprint should be 0 for 0xa5 */
}

void t_isprint_0xa6()
{
    uassert_int_equal(isprint(166), 0);  /* isprint should be 0 for 0xa6 */
}

void t_isprint_0xa7()
{
    uassert_int_equal(isprint(167), 0);  /* isprint should be 0 for 0xa7 */
}

void t_isprint_0xa8()
{
    uassert_int_equal(isprint(168), 0);  /* isprint should be 0 for 0xa8 */
}

void t_isprint_0xa9()
{
    uassert_int_equal(isprint(169), 0);  /* isprint should be 0 for 0xa9 */
}

void t_isprint_0xaa()
{
    uassert_int_equal(isprint(170), 0);  /* isprint should be 0 for 0xaa */
}

void t_isprint_0xab()
{
    uassert_int_equal(isprint(171), 0);  /* isprint should be 0 for 0xab */
}

void t_isprint_0xac()
{
    uassert_int_equal(isprint(172), 0);  /* isprint should be 0 for 0xac */
}

void t_isprint_0xad()
{
    uassert_int_equal(isprint(173), 0);  /* isprint should be 0 for 0xad */
}

void t_isprint_0xae()
{
    uassert_int_equal(isprint(174), 0);  /* isprint should be 0 for 0xae */
}

void t_isprint_0xaf()
{
    uassert_int_equal(isprint(175), 0);  /* isprint should be 0 for 0xaf */
}

void t_isprint_0xb0()
{
    uassert_int_equal(isprint(176), 0);  /* isprint should be 0 for 0xb0 */
}

void t_isprint_0xb1()
{
    uassert_int_equal(isprint(177), 0);  /* isprint should be 0 for 0xb1 */
}

void t_isprint_0xb2()
{
    uassert_int_equal(isprint(178), 0);  /* isprint should be 0 for 0xb2 */
}

void t_isprint_0xb3()
{
    uassert_int_equal(isprint(179), 0);  /* isprint should be 0 for 0xb3 */
}

void t_isprint_0xb4()
{
    uassert_int_equal(isprint(180), 0);  /* isprint should be 0 for 0xb4 */
}

void t_isprint_0xb5()
{
    uassert_int_equal(isprint(181), 0);  /* isprint should be 0 for 0xb5 */
}

void t_isprint_0xb6()
{
    uassert_int_equal(isprint(182), 0);  /* isprint should be 0 for 0xb6 */
}

void t_isprint_0xb7()
{
    uassert_int_equal(isprint(183), 0);  /* isprint should be 0 for 0xb7 */
}

void t_isprint_0xb8()
{
    uassert_int_equal(isprint(184), 0);  /* isprint should be 0 for 0xb8 */
}

void t_isprint_0xb9()
{
    uassert_int_equal(isprint(185), 0);  /* isprint should be 0 for 0xb9 */
}

void t_isprint_0xba()
{
    uassert_int_equal(isprint(186), 0);  /* isprint should be 0 for 0xba */
}

void t_isprint_0xbb()
{
    uassert_int_equal(isprint(187), 0);  /* isprint should be 0 for 0xbb */
}

void t_isprint_0xbc()
{
    uassert_int_equal(isprint(188), 0);  /* isprint should be 0 for 0xbc */
}

void t_isprint_0xbd()
{
    uassert_int_equal(isprint(189), 0);  /* isprint should be 0 for 0xbd */
}

void t_isprint_0xbe()
{
    uassert_int_equal(isprint(190), 0);  /* isprint should be 0 for 0xbe */
}

void t_isprint_0xbf()
{
    uassert_int_equal(isprint(191), 0);  /* isprint should be 0 for 0xbf */
}

void t_isprint_0xc0()
{
    uassert_int_equal(isprint(192), 0);  /* isprint should be 0 for 0xc0 */
}

void t_isprint_0xc1()
{
    uassert_int_equal(isprint(193), 0);  /* isprint should be 0 for 0xc1 */
}

void t_isprint_0xc2()
{
    uassert_int_equal(isprint(194), 0);  /* isprint should be 0 for 0xc2 */
}

void t_isprint_0xc3()
{
    uassert_int_equal(isprint(195), 0);  /* isprint should be 0 for 0xc3 */
}

void t_isprint_0xc4()
{
    uassert_int_equal(isprint(196), 0);  /* isprint should be 0 for 0xc4 */
}

void t_isprint_0xc5()
{
    uassert_int_equal(isprint(197), 0);  /* isprint should be 0 for 0xc5 */
}

void t_isprint_0xc6()
{
    uassert_int_equal(isprint(198), 0);  /* isprint should be 0 for 0xc6 */
}

void t_isprint_0xc7()
{
    uassert_int_equal(isprint(199), 0);  /* isprint should be 0 for 0xc7 */
}

void t_isprint_0xc8()
{
    uassert_int_equal(isprint(200), 0);  /* isprint should be 0 for 0xc8 */
}

void t_isprint_0xc9()
{
    uassert_int_equal(isprint(201), 0);  /* isprint should be 0 for 0xc9 */
}

void t_isprint_0xca()
{
    uassert_int_equal(isprint(202), 0);  /* isprint should be 0 for 0xca */
}

void t_isprint_0xcb()
{
    uassert_int_equal(isprint(203), 0);  /* isprint should be 0 for 0xcb */
}

void t_isprint_0xcc()
{
    uassert_int_equal(isprint(204), 0);  /* isprint should be 0 for 0xcc */
}

void t_isprint_0xcd()
{
    uassert_int_equal(isprint(205), 0);  /* isprint should be 0 for 0xcd */
}

void t_isprint_0xce()
{
    uassert_int_equal(isprint(206), 0);  /* isprint should be 0 for 0xce */
}

void t_isprint_0xcf()
{
    uassert_int_equal(isprint(207), 0);  /* isprint should be 0 for 0xcf */
}

void t_isprint_0xd0()
{
    uassert_int_equal(isprint(208), 0);  /* isprint should be 0 for 0xd0 */
}

void t_isprint_0xd1()
{
    uassert_int_equal(isprint(209), 0);  /* isprint should be 0 for 0xd1 */
}

void t_isprint_0xd2()
{
    uassert_int_equal(isprint(210), 0);  /* isprint should be 0 for 0xd2 */
}

void t_isprint_0xd3()
{
    uassert_int_equal(isprint(211), 0);  /* isprint should be 0 for 0xd3 */
}

void t_isprint_0xd4()
{
    uassert_int_equal(isprint(212), 0);  /* isprint should be 0 for 0xd4 */
}

void t_isprint_0xd5()
{
    uassert_int_equal(isprint(213), 0);  /* isprint should be 0 for 0xd5 */
}

void t_isprint_0xd6()
{
    uassert_int_equal(isprint(214), 0);  /* isprint should be 0 for 0xd6 */
}

void t_isprint_0xd7()
{
    uassert_int_equal(isprint(215), 0);  /* isprint should be 0 for 0xd7 */
}

void t_isprint_0xd8()
{
    uassert_int_equal(isprint(216), 0);  /* isprint should be 0 for 0xd8 */
}

void t_isprint_0xd9()
{
    uassert_int_equal(isprint(217), 0);  /* isprint should be 0 for 0xd9 */
}

void t_isprint_0xda()
{
    uassert_int_equal(isprint(218), 0);  /* isprint should be 0 for 0xda */
}

void t_isprint_0xdb()
{
    uassert_int_equal(isprint(219), 0);  /* isprint should be 0 for 0xdb */
}

void t_isprint_0xdc()
{
    uassert_int_equal(isprint(220), 0);  /* isprint should be 0 for 0xdc */
}

void t_isprint_0xdd()
{
    uassert_int_equal(isprint(221), 0);  /* isprint should be 0 for 0xdd */
}

void t_isprint_0xde()
{
    uassert_int_equal(isprint(222), 0);  /* isprint should be 0 for 0xde */
}

void t_isprint_0xdf()
{
    uassert_int_equal(isprint(223), 0);  /* isprint should be 0 for 0xdf */
}

void t_isprint_0xe0()
{
    uassert_int_equal(isprint(224), 0);  /* isprint should be 0 for 0xe0 */
}

void t_isprint_0xe1()
{
    uassert_int_equal(isprint(225), 0);  /* isprint should be 0 for 0xe1 */
}

void t_isprint_0xe2()
{
    uassert_int_equal(isprint(226), 0);  /* isprint should be 0 for 0xe2 */
}

void t_isprint_0xe3()
{
    uassert_int_equal(isprint(227), 0);  /* isprint should be 0 for 0xe3 */
}

void t_isprint_0xe4()
{
    uassert_int_equal(isprint(228), 0);  /* isprint should be 0 for 0xe4 */
}

void t_isprint_0xe5()
{
    uassert_int_equal(isprint(229), 0);  /* isprint should be 0 for 0xe5 */
}

void t_isprint_0xe6()
{
    uassert_int_equal(isprint(230), 0);  /* isprint should be 0 for 0xe6 */
}

void t_isprint_0xe7()
{
    uassert_int_equal(isprint(231), 0);  /* isprint should be 0 for 0xe7 */
}

void t_isprint_0xe8()
{
    uassert_int_equal(isprint(232), 0);  /* isprint should be 0 for 0xe8 */
}

void t_isprint_0xe9()
{
    uassert_int_equal(isprint(233), 0);  /* isprint should be 0 for 0xe9 */
}

void t_isprint_0xea()
{
    uassert_int_equal(isprint(234), 0);  /* isprint should be 0 for 0xea */
}

void t_isprint_0xeb()
{
    uassert_int_equal(isprint(235), 0);  /* isprint should be 0 for 0xeb */
}

void t_isprint_0xec()
{
    uassert_int_equal(isprint(236), 0);  /* isprint should be 0 for 0xec */
}

void t_isprint_0xed()
{
    uassert_int_equal(isprint(237), 0);  /* isprint should be 0 for 0xed */
}

void t_isprint_0xee()
{
    uassert_int_equal(isprint(238), 0);  /* isprint should be 0 for 0xee */
}

void t_isprint_0xef()
{
    uassert_int_equal(isprint(239), 0);  /* isprint should be 0 for 0xef */
}

void t_isprint_0xf0()
{
    uassert_int_equal(isprint(240), 0);  /* isprint should be 0 for 0xf0 */
}

void t_isprint_0xf1()
{
    uassert_int_equal(isprint(241), 0);  /* isprint should be 0 for 0xf1 */
}

void t_isprint_0xf2()
{
    uassert_int_equal(isprint(242), 0);  /* isprint should be 0 for 0xf2 */
}

void t_isprint_0xf3()
{
    uassert_int_equal(isprint(243), 0);  /* isprint should be 0 for 0xf3 */
}

void t_isprint_0xf4()
{
    uassert_int_equal(isprint(244), 0);  /* isprint should be 0 for 0xf4 */
}

void t_isprint_0xf5()
{
    uassert_int_equal(isprint(245), 0);  /* isprint should be 0 for 0xf5 */
}

void t_isprint_0xf6()
{
    uassert_int_equal(isprint(246), 0);  /* isprint should be 0 for 0xf6 */
}

void t_isprint_0xf7()
{
    uassert_int_equal(isprint(247), 0);  /* isprint should be 0 for 0xf7 */
}

void t_isprint_0xf8()
{
    uassert_int_equal(isprint(248), 0);  /* isprint should be 0 for 0xf8 */
}

void t_isprint_0xf9()
{
    uassert_int_equal(isprint(249), 0);  /* isprint should be 0 for 0xf9 */
}

void t_isprint_0xfa()
{
    uassert_int_equal(isprint(250), 0);  /* isprint should be 0 for 0xfa */
}

void t_isprint_0xfb()
{
    uassert_int_equal(isprint(251), 0);  /* isprint should be 0 for 0xfb */
}

void t_isprint_0xfc()
{
    uassert_int_equal(isprint(252), 0);  /* isprint should be 0 for 0xfc */
}

void t_isprint_0xfd()
{
    uassert_int_equal(isprint(253), 0);  /* isprint should be 0 for 0xfd */
}

void t_isprint_0xfe()
{
    uassert_int_equal(isprint(254), 0);  /* isprint should be 0 for 0xfe */
}

void t_isprint_0xff()
{
    uassert_int_equal(isprint(255), 0);  /* isprint should be 0 for 0xff */
}



static int testcase(void)
{
    t_isprint_0x00();
    t_isprint_0x01();
    t_isprint_0x02();
    t_isprint_0x03();
    t_isprint_0x04();
    t_isprint_0x05();
    t_isprint_0x06();
    t_isprint_0x07();
    t_isprint_0x08();
    t_isprint_0x09();
    t_isprint_0x0a();
    t_isprint_0x0b();
    t_isprint_0x0c();
    t_isprint_0x0d();
    t_isprint_0x0e();
    t_isprint_0x0f();
    t_isprint_0x10();
    t_isprint_0x11();
    t_isprint_0x12();
    t_isprint_0x13();
    t_isprint_0x14();
    t_isprint_0x15();
    t_isprint_0x16();
    t_isprint_0x17();
    t_isprint_0x18();
    t_isprint_0x19();
    t_isprint_0x1a();
    t_isprint_0x1b();
    t_isprint_0x1c();
    t_isprint_0x1d();
    t_isprint_0x1e();
    t_isprint_0x1f();
    t_isprint_0x20();
    t_isprint_0x21();
    t_isprint_0x22();
    t_isprint_0x23();
    t_isprint_0x24();
    t_isprint_0x25();
    t_isprint_0x26();
    t_isprint_0x27();
    t_isprint_0x28();
    t_isprint_0x29();
    t_isprint_0x2a();
    t_isprint_0x2b();
    t_isprint_0x2c();
    t_isprint_0x2d();
    t_isprint_0x2e();
    t_isprint_0x2f();
    t_isprint_0x30();
    t_isprint_0x31();
    t_isprint_0x32();
    t_isprint_0x33();
    t_isprint_0x34();
    t_isprint_0x35();
    t_isprint_0x36();
    t_isprint_0x37();
    t_isprint_0x38();
    t_isprint_0x39();
    t_isprint_0x3a();
    t_isprint_0x3b();
    t_isprint_0x3c();
    t_isprint_0x3d();
    t_isprint_0x3e();
    t_isprint_0x3f();
    t_isprint_0x40();
    t_isprint_0x41();
    t_isprint_0x42();
    t_isprint_0x43();
    t_isprint_0x44();
    t_isprint_0x45();
    t_isprint_0x46();
    t_isprint_0x47();
    t_isprint_0x48();
    t_isprint_0x49();
    t_isprint_0x4a();
    t_isprint_0x4b();
    t_isprint_0x4c();
    t_isprint_0x4d();
    t_isprint_0x4e();
    t_isprint_0x4f();
    t_isprint_0x50();
    t_isprint_0x51();
    t_isprint_0x52();
    t_isprint_0x53();
    t_isprint_0x54();
    t_isprint_0x55();
    t_isprint_0x56();
    t_isprint_0x57();
    t_isprint_0x58();
    t_isprint_0x59();
    t_isprint_0x5a();
    t_isprint_0x5b();
    t_isprint_0x5c();
    t_isprint_0x5d();
    t_isprint_0x5e();
    t_isprint_0x5f();
    t_isprint_0x60();
    t_isprint_0x61();
    t_isprint_0x62();
    t_isprint_0x63();
    t_isprint_0x64();
    t_isprint_0x65();
    t_isprint_0x66();
    t_isprint_0x67();
    t_isprint_0x68();
    t_isprint_0x69();
    t_isprint_0x6a();
    t_isprint_0x6b();
    t_isprint_0x6c();
    t_isprint_0x6d();
    t_isprint_0x6e();
    t_isprint_0x6f();
    t_isprint_0x70();
    t_isprint_0x71();
    t_isprint_0x72();
    t_isprint_0x73();
    t_isprint_0x74();
    t_isprint_0x75();
    t_isprint_0x76();
    t_isprint_0x77();
    t_isprint_0x78();
    t_isprint_0x79();
    t_isprint_0x7a();
    t_isprint_0x7b();
    t_isprint_0x7c();
    t_isprint_0x7d();
    t_isprint_0x7e();
    t_isprint_0x7f();
    t_isprint_0x80();
    t_isprint_0x81();
    t_isprint_0x82();
    t_isprint_0x83();
    t_isprint_0x84();
    t_isprint_0x85();
    t_isprint_0x86();
    t_isprint_0x87();
    t_isprint_0x88();
    t_isprint_0x89();
    t_isprint_0x8a();
    t_isprint_0x8b();
    t_isprint_0x8c();
    t_isprint_0x8d();
    t_isprint_0x8e();
    t_isprint_0x8f();
    t_isprint_0x90();
    t_isprint_0x91();
    t_isprint_0x92();
    t_isprint_0x93();
    t_isprint_0x94();
    t_isprint_0x95();
    t_isprint_0x96();
    t_isprint_0x97();
    t_isprint_0x98();
    t_isprint_0x99();
    t_isprint_0x9a();
    t_isprint_0x9b();
    t_isprint_0x9c();
    t_isprint_0x9d();
    t_isprint_0x9e();
    t_isprint_0x9f();
    t_isprint_0xa0();
    t_isprint_0xa1();
    t_isprint_0xa2();
    t_isprint_0xa3();
    t_isprint_0xa4();
    t_isprint_0xa5();
    t_isprint_0xa6();
    t_isprint_0xa7();
    t_isprint_0xa8();
    t_isprint_0xa9();
    t_isprint_0xaa();
    t_isprint_0xab();
    t_isprint_0xac();
    t_isprint_0xad();
    t_isprint_0xae();
    t_isprint_0xaf();
    t_isprint_0xb0();
    t_isprint_0xb1();
    t_isprint_0xb2();
    t_isprint_0xb3();
    t_isprint_0xb4();
    t_isprint_0xb5();
    t_isprint_0xb6();
    t_isprint_0xb7();
    t_isprint_0xb8();
    t_isprint_0xb9();
    t_isprint_0xba();
    t_isprint_0xbb();
    t_isprint_0xbc();
    t_isprint_0xbd();
    t_isprint_0xbe();
    t_isprint_0xbf();
    t_isprint_0xc0();
    t_isprint_0xc1();
    t_isprint_0xc2();
    t_isprint_0xc3();
    t_isprint_0xc4();
    t_isprint_0xc5();
    t_isprint_0xc6();
    t_isprint_0xc7();
    t_isprint_0xc8();
    t_isprint_0xc9();
    t_isprint_0xca();
    t_isprint_0xcb();
    t_isprint_0xcc();
    t_isprint_0xcd();
    t_isprint_0xce();
    t_isprint_0xcf();
    t_isprint_0xd0();
    t_isprint_0xd1();
    t_isprint_0xd2();
    t_isprint_0xd3();
    t_isprint_0xd4();
    t_isprint_0xd5();
    t_isprint_0xd6();
    t_isprint_0xd7();
    t_isprint_0xd8();
    t_isprint_0xd9();
    t_isprint_0xda();
    t_isprint_0xdb();
    t_isprint_0xdc();
    t_isprint_0xdd();
    t_isprint_0xde();
    t_isprint_0xdf();
    t_isprint_0xe0();
    t_isprint_0xe1();
    t_isprint_0xe2();
    t_isprint_0xe3();
    t_isprint_0xe4();
    t_isprint_0xe5();
    t_isprint_0xe6();
    t_isprint_0xe7();
    t_isprint_0xe8();
    t_isprint_0xe9();
    t_isprint_0xea();
    t_isprint_0xeb();
    t_isprint_0xec();
    t_isprint_0xed();
    t_isprint_0xee();
    t_isprint_0xef();
    t_isprint_0xf0();
    t_isprint_0xf1();
    t_isprint_0xf2();
    t_isprint_0xf3();
    t_isprint_0xf4();
    t_isprint_0xf5();
    t_isprint_0xf6();
    t_isprint_0xf7();
    t_isprint_0xf8();
    t_isprint_0xf9();
    t_isprint_0xfa();
    t_isprint_0xfb();
    t_isprint_0xfc();
    t_isprint_0xfd();
    t_isprint_0xfe();
    t_isprint_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
