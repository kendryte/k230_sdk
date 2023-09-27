#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))


void t_isspace_0x00()
{
    uassert_int_equal(isspace(0), 0); /* isspace should be 0 for 0x00 */
}

void t_isspace_0x01()
{
    uassert_int_equal(isspace(1), 0); /* isspace should be 0 for 0x01 */
}

void t_isspace_0x02()
{
    uassert_int_equal(isspace(2), 0); /* isspace should be 0 for 0x02 */
}

void t_isspace_0x03()
{
    uassert_int_equal(isspace(3), 0); /* isspace should be 0 for 0x03 */
}

void t_isspace_0x04()
{
    uassert_int_equal(isspace(4), 0); /* isspace should be 0 for 0x04 */
}

void t_isspace_0x05()
{
    uassert_int_equal(isspace(5), 0); /* isspace should be 0 for 0x05 */
}

void t_isspace_0x06()
{
    uassert_int_equal(isspace(6), 0); /* isspace should be 0 for 0x06 */
}

void t_isspace_0x07()
{
    uassert_int_equal(isspace(7), 0); /* isspace should be 0 for 0x07 */
}

void t_isspace_0x08()
{
    uassert_int_equal(isspace(8), 0); /* isspace should be 0 for 0x08 */
}

void t_isspace_0x09()
{
    // uassert_int_equal(isspace(9), 1); /* isspace should be 1 for 0x09 */
}

void t_isspace_0x0a()
{
    // uassert_int_equal(isspace(10), 1); /* isspace should be 1 for 0x0a */
}

void t_isspace_0x0b()
{
    // uassert_int_equal(isspace(11), 1); /* isspace should be 1 for 0x0b */
}

void t_isspace_0x0c()
{
    // uassert_int_equal(isspace(12), 1); /* isspace should be 1 for 0x0c */
}

void t_isspace_0x0d()
{
    // uassert_int_equal(isspace(13), 1); /* isspace should be 1 for 0x0d */
}

void t_isspace_0x0e()
{
    uassert_int_equal(isspace(14), 0); /* isspace should be 0 for 0x0e */
}

void t_isspace_0x0f()
{
    uassert_int_equal(isspace(15), 0); /* isspace should be 0 for 0x0f */
}

void t_isspace_0x10()
{
    uassert_int_equal(isspace(16), 0); /* isspace should be 0 for 0x10 */
}

void t_isspace_0x11()
{
    uassert_int_equal(isspace(17), 0); /* isspace should be 0 for 0x11 */
}

void t_isspace_0x12()
{
    uassert_int_equal(isspace(18), 0); /* isspace should be 0 for 0x12 */
}

void t_isspace_0x13()
{
    uassert_int_equal(isspace(19), 0); /* isspace should be 0 for 0x13 */
}

void t_isspace_0x14()
{
    uassert_int_equal(isspace(20), 0); /* isspace should be 0 for 0x14 */
}

void t_isspace_0x15()
{
    uassert_int_equal(isspace(21), 0); /* isspace should be 0 for 0x15 */
}

void t_isspace_0x16()
{
    uassert_int_equal(isspace(22), 0); /* isspace should be 0 for 0x16 */
}

void t_isspace_0x17()
{
    uassert_int_equal(isspace(23), 0); /* isspace should be 0 for 0x17 */
}

void t_isspace_0x18()
{
    uassert_int_equal(isspace(24), 0); /* isspace should be 0 for 0x18 */
}

void t_isspace_0x19()
{
    uassert_int_equal(isspace(25), 0); /* isspace should be 0 for 0x19 */
}

void t_isspace_0x1a()
{
    uassert_int_equal(isspace(26), 0); /* isspace should be 0 for 0x1a */
}

void t_isspace_0x1b()
{
    uassert_int_equal(isspace(27), 0); /* isspace should be 0 for 0x1b */
}

void t_isspace_0x1c()
{
    uassert_int_equal(isspace(28), 0); /* isspace should be 0 for 0x1c */
}

void t_isspace_0x1d()
{
    uassert_int_equal(isspace(29), 0); /* isspace should be 0 for 0x1d */
}

void t_isspace_0x1e()
{
    uassert_int_equal(isspace(30), 0); /* isspace should be 0 for 0x1e */
}

void t_isspace_0x1f()
{
    uassert_int_equal(isspace(31), 0); /* isspace should be 0 for 0x1f */
}

void t_isspace_0x20()
{
    // uassert_int_equal(isspace(32), 1); /* isspace should be 1 for   */
}

void t_isspace_0x21()
{
    uassert_int_equal(isspace(33), 0); /* isspace should be 0 for ! */
}

void t_isspace_0x22()
{
    uassert_int_equal(isspace(34), 0); /* isspace should be 0 for 0x22 */
}

void t_isspace_0x23()
{
    uassert_int_equal(isspace(35), 0); /* isspace should be 0 for # */
}

void t_isspace_0x24()
{
    uassert_int_equal(isspace(36), 0); /* isspace should be 0 for $ */
}

void t_isspace_0x25()
{
    uassert_int_equal(isspace(37), 0); /* isspace should be 0 for % */
}

void t_isspace_0x26()
{
    uassert_int_equal(isspace(38), 0); /* isspace should be 0 for & */
}

void t_isspace_0x27()
{
    uassert_int_equal(isspace(39), 0); /* isspace should be 0 for ' */
}

void t_isspace_0x28()
{
    uassert_int_equal(isspace(40), 0); /* isspace should be 0 for ( */
}

void t_isspace_0x29()
{
    uassert_int_equal(isspace(41), 0); /* isspace should be 0 for ) */
}

void t_isspace_0x2a()
{
    uassert_int_equal(isspace(42), 0); /* isspace should be 0 for * */
}

void t_isspace_0x2b()
{
    uassert_int_equal(isspace(43), 0); /* isspace should be 0 for + */
}

void t_isspace_0x2c()
{
    uassert_int_equal(isspace(44), 0); /* isspace should be 0 for , */
}

void t_isspace_0x2d()
{
    uassert_int_equal(isspace(45), 0); /* isspace should be 0 for - */
}

void t_isspace_0x2e()
{
    uassert_int_equal(isspace(46), 0); /* isspace should be 0 for . */
}

void t_isspace_0x2f()
{
    uassert_int_equal(isspace(47), 0); /* isspace should be 0 for / */
}

void t_isspace_0x30()
{
    uassert_int_equal(isspace(48), 0); /* isspace should be 0 for 0 */
}

void t_isspace_0x31()
{
    uassert_int_equal(isspace(49), 0); /* isspace should be 0 for 1 */
}

void t_isspace_0x32()
{
    uassert_int_equal(isspace(50), 0); /* isspace should be 0 for 2 */
}

void t_isspace_0x33()
{
    uassert_int_equal(isspace(51), 0); /* isspace should be 0 for 3 */
}

void t_isspace_0x34()
{
    uassert_int_equal(isspace(52), 0); /* isspace should be 0 for 4 */
}

void t_isspace_0x35()
{
    uassert_int_equal(isspace(53), 0); /* isspace should be 0 for 5 */
}

void t_isspace_0x36()
{
    uassert_int_equal(isspace(54), 0); /* isspace should be 0 for 6 */
}

void t_isspace_0x37()
{
    uassert_int_equal(isspace(55), 0); /* isspace should be 0 for 7 */
}

void t_isspace_0x38()
{
    uassert_int_equal(isspace(56), 0); /* isspace should be 0 for 8 */
}

void t_isspace_0x39()
{
    uassert_int_equal(isspace(57), 0); /* isspace should be 0 for 9 */
}

void t_isspace_0x3a()
{
    uassert_int_equal(isspace(58), 0); /* isspace should be 0 for : */
}

void t_isspace_0x3b()
{
    uassert_int_equal(isspace(59), 0); /* isspace should be 0 for ; */
}

void t_isspace_0x3c()
{
    uassert_int_equal(isspace(60), 0); /* isspace should be 0 for < */
}

void t_isspace_0x3d()
{
    uassert_int_equal(isspace(61), 0); /* isspace should be 0 for = */
}

void t_isspace_0x3e()
{
    uassert_int_equal(isspace(62), 0); /* isspace should be 0 for > */
}

void t_isspace_0x3f()
{
    uassert_int_equal(isspace(63), 0); /* isspace should be 0 for ? */
}

void t_isspace_0x40()
{
    uassert_int_equal(isspace(64), 0); /* isspace should be 0 for @ */
}

void t_isspace_0x41()
{
    uassert_int_equal(isspace(65), 0); /* isspace should be 0 for A */
}

void t_isspace_0x42()
{
    uassert_int_equal(isspace(66), 0); /* isspace should be 0 for B */
}

void t_isspace_0x43()
{
    uassert_int_equal(isspace(67), 0); /* isspace should be 0 for C */
}

void t_isspace_0x44()
{
    uassert_int_equal(isspace(68), 0); /* isspace should be 0 for D */
}

void t_isspace_0x45()
{
    uassert_int_equal(isspace(69), 0); /* isspace should be 0 for E */
}

void t_isspace_0x46()
{
    uassert_int_equal(isspace(70), 0); /* isspace should be 0 for F */
}

void t_isspace_0x47()
{
    uassert_int_equal(isspace(71), 0); /* isspace should be 0 for G */
}

void t_isspace_0x48()
{
    uassert_int_equal(isspace(72), 0); /* isspace should be 0 for H */
}

void t_isspace_0x49()
{
    uassert_int_equal(isspace(73), 0); /* isspace should be 0 for I */
}

void t_isspace_0x4a()
{
    uassert_int_equal(isspace(74), 0); /* isspace should be 0 for J */
}

void t_isspace_0x4b()
{
    uassert_int_equal(isspace(75), 0); /* isspace should be 0 for K */
}

void t_isspace_0x4c()
{
    uassert_int_equal(isspace(76), 0); /* isspace should be 0 for L */
}

void t_isspace_0x4d()
{
    uassert_int_equal(isspace(77), 0); /* isspace should be 0 for M */
}

void t_isspace_0x4e()
{
    uassert_int_equal(isspace(78), 0); /* isspace should be 0 for N */
}

void t_isspace_0x4f()
{
    uassert_int_equal(isspace(79), 0); /* isspace should be 0 for O */
}

void t_isspace_0x50()
{
    uassert_int_equal(isspace(80), 0); /* isspace should be 0 for P */
}

void t_isspace_0x51()
{
    uassert_int_equal(isspace(81), 0); /* isspace should be 0 for Q */
}

void t_isspace_0x52()
{
    uassert_int_equal(isspace(82), 0); /* isspace should be 0 for R */
}

void t_isspace_0x53()
{
    uassert_int_equal(isspace(83), 0); /* isspace should be 0 for S */
}

void t_isspace_0x54()
{
    uassert_int_equal(isspace(84), 0); /* isspace should be 0 for T */
}

void t_isspace_0x55()
{
    uassert_int_equal(isspace(85), 0); /* isspace should be 0 for U */
}

void t_isspace_0x56()
{
    uassert_int_equal(isspace(86), 0); /* isspace should be 0 for V */
}

void t_isspace_0x57()
{
    uassert_int_equal(isspace(87), 0); /* isspace should be 0 for W */
}

void t_isspace_0x58()
{
    uassert_int_equal(isspace(88), 0); /* isspace should be 0 for X */
}

void t_isspace_0x59()
{
    uassert_int_equal(isspace(89), 0); /* isspace should be 0 for Y */
}

void t_isspace_0x5a()
{
    uassert_int_equal(isspace(90), 0); /* isspace should be 0 for Z */
}

void t_isspace_0x5b()
{
    uassert_int_equal(isspace(91), 0); /* isspace should be 0 for [ */
}

void t_isspace_0x5c()
{
    uassert_int_equal(isspace(92), 0); /* isspace should be 0 for 0x5c */
}

void t_isspace_0x5d()
{
    uassert_int_equal(isspace(93), 0); /* isspace should be 0 for ] */
}

void t_isspace_0x5e()
{
    uassert_int_equal(isspace(94), 0); /* isspace should be 0 for ^ */
}

void t_isspace_0x5f()
{
    uassert_int_equal(isspace(95), 0); /* isspace should be 0 for _ */
}

void t_isspace_0x60()
{
    uassert_int_equal(isspace(96), 0); /* isspace should be 0 for ` */
}

void t_isspace_0x61()
{
    uassert_int_equal(isspace(97), 0); /* isspace should be 0 for a */
}

void t_isspace_0x62()
{
    uassert_int_equal(isspace(98), 0); /* isspace should be 0 for b */
}

void t_isspace_0x63()
{
    uassert_int_equal(isspace(99), 0); /* isspace should be 0 for c */
}

void t_isspace_0x64()
{
    uassert_int_equal(isspace(100), 0); /* isspace should be 0 for d */
}

void t_isspace_0x65()
{
    uassert_int_equal(isspace(101), 0); /* isspace should be 0 for e */
}

void t_isspace_0x66()
{
    uassert_int_equal(isspace(102), 0); /* isspace should be 0 for f */
}

void t_isspace_0x67()
{
    uassert_int_equal(isspace(103), 0); /* isspace should be 0 for g */
}

void t_isspace_0x68()
{
    uassert_int_equal(isspace(104), 0); /* isspace should be 0 for h */
}

void t_isspace_0x69()
{
    uassert_int_equal(isspace(105), 0); /* isspace should be 0 for i */
}

void t_isspace_0x6a()
{
    uassert_int_equal(isspace(106), 0); /* isspace should be 0 for j */
}

void t_isspace_0x6b()
{
    uassert_int_equal(isspace(107), 0); /* isspace should be 0 for k */
}

void t_isspace_0x6c()
{
    uassert_int_equal(isspace(108), 0); /* isspace should be 0 for l */
}

void t_isspace_0x6d()
{
    uassert_int_equal(isspace(109), 0); /* isspace should be 0 for m */
}

void t_isspace_0x6e()
{
    uassert_int_equal(isspace(110), 0); /* isspace should be 0 for n */
}

void t_isspace_0x6f()
{
    uassert_int_equal(isspace(111), 0); /* isspace should be 0 for o */
}

void t_isspace_0x70()
{
    uassert_int_equal(isspace(112), 0); /* isspace should be 0 for p */
}

void t_isspace_0x71()
{
    uassert_int_equal(isspace(113), 0); /* isspace should be 0 for q */
}

void t_isspace_0x72()
{
    uassert_int_equal(isspace(114), 0); /* isspace should be 0 for r */
}

void t_isspace_0x73()
{
    uassert_int_equal(isspace(115), 0); /* isspace should be 0 for s */
}

void t_isspace_0x74()
{
    uassert_int_equal(isspace(116), 0); /* isspace should be 0 for t */
}

void t_isspace_0x75()
{
    uassert_int_equal(isspace(117), 0); /* isspace should be 0 for u */
}

void t_isspace_0x76()
{
    uassert_int_equal(isspace(118), 0); /* isspace should be 0 for v */
}

void t_isspace_0x77()
{
    uassert_int_equal(isspace(119), 0); /* isspace should be 0 for w */
}

void t_isspace_0x78()
{
    uassert_int_equal(isspace(120), 0); /* isspace should be 0 for x */
}

void t_isspace_0x79()
{
    uassert_int_equal(isspace(121), 0); /* isspace should be 0 for y */
}

void t_isspace_0x7a()
{
    uassert_int_equal(isspace(122), 0); /* isspace should be 0 for z */
}

void t_isspace_0x7b()
{
    uassert_int_equal(isspace(123), 0); /* isspace should be 0 for { */
}

void t_isspace_0x7c()
{
    uassert_int_equal(isspace(124), 0); /* isspace should be 0 for | */
}

void t_isspace_0x7d()
{
    uassert_int_equal(isspace(125), 0); /* isspace should be 0 for } */
}

void t_isspace_0x7e()
{
    uassert_int_equal(isspace(126), 0); /* isspace should be 0 for ~ */
}

void t_isspace_0x7f()
{
    uassert_int_equal(isspace(127), 0); /* isspace should be 0 for 0x7f */
}

void t_isspace_0x80()
{
    uassert_int_equal(isspace(128), 0); /* isspace should be 0 for 0x80 */
}

void t_isspace_0x81()
{
    uassert_int_equal(isspace(129), 0); /* isspace should be 0 for 0x81 */
}

void t_isspace_0x82()
{
    uassert_int_equal(isspace(130), 0); /* isspace should be 0 for 0x82 */
}

void t_isspace_0x83()
{
    uassert_int_equal(isspace(131), 0); /* isspace should be 0 for 0x83 */
}

void t_isspace_0x84()
{
    uassert_int_equal(isspace(132), 0); /* isspace should be 0 for 0x84 */
}

void t_isspace_0x85()
{
    uassert_int_equal(isspace(133), 0); /* isspace should be 0 for 0x85 */
}

void t_isspace_0x86()
{
    uassert_int_equal(isspace(134), 0); /* isspace should be 0 for 0x86 */
}

void t_isspace_0x87()
{
    uassert_int_equal(isspace(135), 0); /* isspace should be 0 for 0x87 */
}

void t_isspace_0x88()
{
    uassert_int_equal(isspace(136), 0); /* isspace should be 0 for 0x88 */
}

void t_isspace_0x89()
{
    uassert_int_equal(isspace(137), 0); /* isspace should be 0 for 0x89 */
}

void t_isspace_0x8a()
{
    uassert_int_equal(isspace(138), 0); /* isspace should be 0 for 0x8a */
}

void t_isspace_0x8b()
{
    uassert_int_equal(isspace(139), 0); /* isspace should be 0 for 0x8b */
}

void t_isspace_0x8c()
{
    uassert_int_equal(isspace(140), 0); /* isspace should be 0 for 0x8c */
}

void t_isspace_0x8d()
{
    uassert_int_equal(isspace(141), 0); /* isspace should be 0 for 0x8d */
}

void t_isspace_0x8e()
{
    uassert_int_equal(isspace(142), 0); /* isspace should be 0 for 0x8e */
}

void t_isspace_0x8f()
{
    uassert_int_equal(isspace(143), 0); /* isspace should be 0 for 0x8f */
}

void t_isspace_0x90()
{
    uassert_int_equal(isspace(144), 0); /* isspace should be 0 for 0x90 */
}

void t_isspace_0x91()
{
    uassert_int_equal(isspace(145), 0); /* isspace should be 0 for 0x91 */
}

void t_isspace_0x92()
{
    uassert_int_equal(isspace(146), 0); /* isspace should be 0 for 0x92 */
}

void t_isspace_0x93()
{
    uassert_int_equal(isspace(147), 0); /* isspace should be 0 for 0x93 */
}

void t_isspace_0x94()
{
    uassert_int_equal(isspace(148), 0); /* isspace should be 0 for 0x94 */
}

void t_isspace_0x95()
{
    uassert_int_equal(isspace(149), 0); /* isspace should be 0 for 0x95 */
}

void t_isspace_0x96()
{
    uassert_int_equal(isspace(150), 0); /* isspace should be 0 for 0x96 */
}

void t_isspace_0x97()
{
    uassert_int_equal(isspace(151), 0); /* isspace should be 0 for 0x97 */
}

void t_isspace_0x98()
{
    uassert_int_equal(isspace(152), 0); /* isspace should be 0 for 0x98 */
}

void t_isspace_0x99()
{
    uassert_int_equal(isspace(153), 0); /* isspace should be 0 for 0x99 */
}

void t_isspace_0x9a()
{
    uassert_int_equal(isspace(154), 0); /* isspace should be 0 for 0x9a */
}

void t_isspace_0x9b()
{
    uassert_int_equal(isspace(155), 0); /* isspace should be 0 for 0x9b */
}

void t_isspace_0x9c()
{
    uassert_int_equal(isspace(156), 0); /* isspace should be 0 for 0x9c */
}

void t_isspace_0x9d()
{
    uassert_int_equal(isspace(157), 0); /* isspace should be 0 for 0x9d */
}

void t_isspace_0x9e()
{
    uassert_int_equal(isspace(158), 0); /* isspace should be 0 for 0x9e */
}

void t_isspace_0x9f()
{
    uassert_int_equal(isspace(159), 0); /* isspace should be 0 for 0x9f */
}

void t_isspace_0xa0()
{
    uassert_int_equal(isspace(160), 0); /* isspace should be 0 for 0xa0 */
}

void t_isspace_0xa1()
{
    uassert_int_equal(isspace(161), 0); /* isspace should be 0 for 0xa1 */
}

void t_isspace_0xa2()
{
    uassert_int_equal(isspace(162), 0); /* isspace should be 0 for 0xa2 */
}

void t_isspace_0xa3()
{
    uassert_int_equal(isspace(163), 0); /* isspace should be 0 for 0xa3 */
}

void t_isspace_0xa4()
{
    uassert_int_equal(isspace(164), 0); /* isspace should be 0 for 0xa4 */
}

void t_isspace_0xa5()
{
    uassert_int_equal(isspace(165), 0); /* isspace should be 0 for 0xa5 */
}

void t_isspace_0xa6()
{
    uassert_int_equal(isspace(166), 0); /* isspace should be 0 for 0xa6 */
}

void t_isspace_0xa7()
{
    uassert_int_equal(isspace(167), 0); /* isspace should be 0 for 0xa7 */
}

void t_isspace_0xa8()
{
    uassert_int_equal(isspace(168), 0); /* isspace should be 0 for 0xa8 */
}

void t_isspace_0xa9()
{
    uassert_int_equal(isspace(169), 0); /* isspace should be 0 for 0xa9 */
}

void t_isspace_0xaa()
{
    uassert_int_equal(isspace(170), 0); /* isspace should be 0 for 0xaa */
}

void t_isspace_0xab()
{
    uassert_int_equal(isspace(171), 0); /* isspace should be 0 for 0xab */
}

void t_isspace_0xac()
{
    uassert_int_equal(isspace(172), 0); /* isspace should be 0 for 0xac */
}

void t_isspace_0xad()
{
    uassert_int_equal(isspace(173), 0); /* isspace should be 0 for 0xad */
}

void t_isspace_0xae()
{
    uassert_int_equal(isspace(174), 0); /* isspace should be 0 for 0xae */
}

void t_isspace_0xaf()
{
    uassert_int_equal(isspace(175), 0); /* isspace should be 0 for 0xaf */
}

void t_isspace_0xb0()
{
    uassert_int_equal(isspace(176), 0); /* isspace should be 0 for 0xb0 */
}

void t_isspace_0xb1()
{
    uassert_int_equal(isspace(177), 0); /* isspace should be 0 for 0xb1 */
}

void t_isspace_0xb2()
{
    uassert_int_equal(isspace(178), 0); /* isspace should be 0 for 0xb2 */
}

void t_isspace_0xb3()
{
    uassert_int_equal(isspace(179), 0); /* isspace should be 0 for 0xb3 */
}

void t_isspace_0xb4()
{
    uassert_int_equal(isspace(180), 0); /* isspace should be 0 for 0xb4 */
}

void t_isspace_0xb5()
{
    uassert_int_equal(isspace(181), 0); /* isspace should be 0 for 0xb5 */
}

void t_isspace_0xb6()
{
    uassert_int_equal(isspace(182), 0); /* isspace should be 0 for 0xb6 */
}

void t_isspace_0xb7()
{
    uassert_int_equal(isspace(183), 0); /* isspace should be 0 for 0xb7 */
}

void t_isspace_0xb8()
{
    uassert_int_equal(isspace(184), 0); /* isspace should be 0 for 0xb8 */
}

void t_isspace_0xb9()
{
    uassert_int_equal(isspace(185), 0); /* isspace should be 0 for 0xb9 */
}

void t_isspace_0xba()
{
    uassert_int_equal(isspace(186), 0); /* isspace should be 0 for 0xba */
}

void t_isspace_0xbb()
{
    uassert_int_equal(isspace(187), 0); /* isspace should be 0 for 0xbb */
}

void t_isspace_0xbc()
{
    uassert_int_equal(isspace(188), 0); /* isspace should be 0 for 0xbc */
}

void t_isspace_0xbd()
{
    uassert_int_equal(isspace(189), 0); /* isspace should be 0 for 0xbd */
}

void t_isspace_0xbe()
{
    uassert_int_equal(isspace(190), 0); /* isspace should be 0 for 0xbe */
}

void t_isspace_0xbf()
{
    uassert_int_equal(isspace(191), 0); /* isspace should be 0 for 0xbf */
}

void t_isspace_0xc0()
{
    uassert_int_equal(isspace(192), 0); /* isspace should be 0 for 0xc0 */
}

void t_isspace_0xc1()
{
    uassert_int_equal(isspace(193), 0); /* isspace should be 0 for 0xc1 */
}

void t_isspace_0xc2()
{
    uassert_int_equal(isspace(194), 0); /* isspace should be 0 for 0xc2 */
}

void t_isspace_0xc3()
{
    uassert_int_equal(isspace(195), 0); /* isspace should be 0 for 0xc3 */
}

void t_isspace_0xc4()
{
    uassert_int_equal(isspace(196), 0); /* isspace should be 0 for 0xc4 */
}

void t_isspace_0xc5()
{
    uassert_int_equal(isspace(197), 0); /* isspace should be 0 for 0xc5 */
}

void t_isspace_0xc6()
{
    uassert_int_equal(isspace(198), 0); /* isspace should be 0 for 0xc6 */
}

void t_isspace_0xc7()
{
    uassert_int_equal(isspace(199), 0); /* isspace should be 0 for 0xc7 */
}

void t_isspace_0xc8()
{
    uassert_int_equal(isspace(200), 0); /* isspace should be 0 for 0xc8 */
}

void t_isspace_0xc9()
{
    uassert_int_equal(isspace(201), 0); /* isspace should be 0 for 0xc9 */
}

void t_isspace_0xca()
{
    uassert_int_equal(isspace(202), 0); /* isspace should be 0 for 0xca */
}

void t_isspace_0xcb()
{
    uassert_int_equal(isspace(203), 0); /* isspace should be 0 for 0xcb */
}

void t_isspace_0xcc()
{
    uassert_int_equal(isspace(204), 0); /* isspace should be 0 for 0xcc */
}

void t_isspace_0xcd()
{
    uassert_int_equal(isspace(205), 0); /* isspace should be 0 for 0xcd */
}

void t_isspace_0xce()
{
    uassert_int_equal(isspace(206), 0); /* isspace should be 0 for 0xce */
}

void t_isspace_0xcf()
{
    uassert_int_equal(isspace(207), 0); /* isspace should be 0 for 0xcf */
}

void t_isspace_0xd0()
{
    uassert_int_equal(isspace(208), 0); /* isspace should be 0 for 0xd0 */
}

void t_isspace_0xd1()
{
    uassert_int_equal(isspace(209), 0); /* isspace should be 0 for 0xd1 */
}

void t_isspace_0xd2()
{
    uassert_int_equal(isspace(210), 0); /* isspace should be 0 for 0xd2 */
}

void t_isspace_0xd3()
{
    uassert_int_equal(isspace(211), 0); /* isspace should be 0 for 0xd3 */
}

void t_isspace_0xd4()
{
    uassert_int_equal(isspace(212), 0); /* isspace should be 0 for 0xd4 */
}

void t_isspace_0xd5()
{
    uassert_int_equal(isspace(213), 0); /* isspace should be 0 for 0xd5 */
}

void t_isspace_0xd6()
{
    uassert_int_equal(isspace(214), 0); /* isspace should be 0 for 0xd6 */
}

void t_isspace_0xd7()
{
    uassert_int_equal(isspace(215), 0); /* isspace should be 0 for 0xd7 */
}

void t_isspace_0xd8()
{
    uassert_int_equal(isspace(216), 0); /* isspace should be 0 for 0xd8 */
}

void t_isspace_0xd9()
{
    uassert_int_equal(isspace(217), 0); /* isspace should be 0 for 0xd9 */
}

void t_isspace_0xda()
{
    uassert_int_equal(isspace(218), 0); /* isspace should be 0 for 0xda */
}

void t_isspace_0xdb()
{
    uassert_int_equal(isspace(219), 0); /* isspace should be 0 for 0xdb */
}

void t_isspace_0xdc()
{
    uassert_int_equal(isspace(220), 0); /* isspace should be 0 for 0xdc */
}

void t_isspace_0xdd()
{
    uassert_int_equal(isspace(221), 0); /* isspace should be 0 for 0xdd */
}

void t_isspace_0xde()
{
    uassert_int_equal(isspace(222), 0); /* isspace should be 0 for 0xde */
}

void t_isspace_0xdf()
{
    uassert_int_equal(isspace(223), 0); /* isspace should be 0 for 0xdf */
}

void t_isspace_0xe0()
{
    uassert_int_equal(isspace(224), 0); /* isspace should be 0 for 0xe0 */
}

void t_isspace_0xe1()
{
    uassert_int_equal(isspace(225), 0); /* isspace should be 0 for 0xe1 */
}

void t_isspace_0xe2()
{
    uassert_int_equal(isspace(226), 0); /* isspace should be 0 for 0xe2 */
}

void t_isspace_0xe3()
{
    uassert_int_equal(isspace(227), 0); /* isspace should be 0 for 0xe3 */
}

void t_isspace_0xe4()
{
    uassert_int_equal(isspace(228), 0); /* isspace should be 0 for 0xe4 */
}

void t_isspace_0xe5()
{
    uassert_int_equal(isspace(229), 0); /* isspace should be 0 for 0xe5 */
}

void t_isspace_0xe6()
{
    uassert_int_equal(isspace(230), 0); /* isspace should be 0 for 0xe6 */
}

void t_isspace_0xe7()
{
    uassert_int_equal(isspace(231), 0); /* isspace should be 0 for 0xe7 */
}

void t_isspace_0xe8()
{
    uassert_int_equal(isspace(232), 0); /* isspace should be 0 for 0xe8 */
}

void t_isspace_0xe9()
{
    uassert_int_equal(isspace(233), 0); /* isspace should be 0 for 0xe9 */
}

void t_isspace_0xea()
{
    uassert_int_equal(isspace(234), 0); /* isspace should be 0 for 0xea */
}

void t_isspace_0xeb()
{
    uassert_int_equal(isspace(235), 0); /* isspace should be 0 for 0xeb */
}

void t_isspace_0xec()
{
    uassert_int_equal(isspace(236), 0); /* isspace should be 0 for 0xec */
}

void t_isspace_0xed()
{
    uassert_int_equal(isspace(237), 0); /* isspace should be 0 for 0xed */
}

void t_isspace_0xee()
{
    uassert_int_equal(isspace(238), 0); /* isspace should be 0 for 0xee */
}

void t_isspace_0xef()
{
    uassert_int_equal(isspace(239), 0); /* isspace should be 0 for 0xef */
}

void t_isspace_0xf0()
{
    uassert_int_equal(isspace(240), 0); /* isspace should be 0 for 0xf0 */
}

void t_isspace_0xf1()
{
    uassert_int_equal(isspace(241), 0); /* isspace should be 0 for 0xf1 */
}

void t_isspace_0xf2()
{
    uassert_int_equal(isspace(242), 0); /* isspace should be 0 for 0xf2 */
}

void t_isspace_0xf3()
{
    uassert_int_equal(isspace(243), 0); /* isspace should be 0 for 0xf3 */
}

void t_isspace_0xf4()
{
    uassert_int_equal(isspace(244), 0); /* isspace should be 0 for 0xf4 */
}

void t_isspace_0xf5()
{
    uassert_int_equal(isspace(245), 0); /* isspace should be 0 for 0xf5 */
}

void t_isspace_0xf6()
{
    uassert_int_equal(isspace(246), 0); /* isspace should be 0 for 0xf6 */
}

void t_isspace_0xf7()
{
    uassert_int_equal(isspace(247), 0); /* isspace should be 0 for 0xf7 */
}

void t_isspace_0xf8()
{
    uassert_int_equal(isspace(248), 0); /* isspace should be 0 for 0xf8 */
}

void t_isspace_0xf9()
{
    uassert_int_equal(isspace(249), 0); /* isspace should be 0 for 0xf9 */
}

void t_isspace_0xfa()
{
    uassert_int_equal(isspace(250), 0); /* isspace should be 0 for 0xfa */
}

void t_isspace_0xfb()
{
    uassert_int_equal(isspace(251), 0); /* isspace should be 0 for 0xfb */
}

void t_isspace_0xfc()
{
    uassert_int_equal(isspace(252), 0); /* isspace should be 0 for 0xfc */
}

void t_isspace_0xfd()
{
    uassert_int_equal(isspace(253), 0); /* isspace should be 0 for 0xfd */
}

void t_isspace_0xfe()
{
    uassert_int_equal(isspace(254), 0); /* isspace should be 0 for 0xfe */
}

void t_isspace_0xff()
{
    uassert_int_equal(isspace(255), 0); /* isspace should be 0 for 0xff */
}



static int testcase(void)
{
    t_isspace_0x00();
    t_isspace_0x01();
    t_isspace_0x02();
    t_isspace_0x03();
    t_isspace_0x04();
    t_isspace_0x05();
    t_isspace_0x06();
    t_isspace_0x07();
    t_isspace_0x08();
    t_isspace_0x09();
    t_isspace_0x0a();
    t_isspace_0x0b();
    t_isspace_0x0c();
    t_isspace_0x0d();
    t_isspace_0x0e();
    t_isspace_0x0f();
    t_isspace_0x10();
    t_isspace_0x11();
    t_isspace_0x12();
    t_isspace_0x13();
    t_isspace_0x14();
    t_isspace_0x15();
    t_isspace_0x16();
    t_isspace_0x17();
    t_isspace_0x18();
    t_isspace_0x19();
    t_isspace_0x1a();
    t_isspace_0x1b();
    t_isspace_0x1c();
    t_isspace_0x1d();
    t_isspace_0x1e();
    t_isspace_0x1f();
    t_isspace_0x20();
    t_isspace_0x21();
    t_isspace_0x22();
    t_isspace_0x23();
    t_isspace_0x24();
    t_isspace_0x25();
    t_isspace_0x26();
    t_isspace_0x27();
    t_isspace_0x28();
    t_isspace_0x29();
    t_isspace_0x2a();
    t_isspace_0x2b();
    t_isspace_0x2c();
    t_isspace_0x2d();
    t_isspace_0x2e();
    t_isspace_0x2f();
    t_isspace_0x30();
    t_isspace_0x31();
    t_isspace_0x32();
    t_isspace_0x33();
    t_isspace_0x34();
    t_isspace_0x35();
    t_isspace_0x36();
    t_isspace_0x37();
    t_isspace_0x38();
    t_isspace_0x39();
    t_isspace_0x3a();
    t_isspace_0x3b();
    t_isspace_0x3c();
    t_isspace_0x3d();
    t_isspace_0x3e();
    t_isspace_0x3f();
    t_isspace_0x40();
    t_isspace_0x41();
    t_isspace_0x42();
    t_isspace_0x43();
    t_isspace_0x44();
    t_isspace_0x45();
    t_isspace_0x46();
    t_isspace_0x47();
    t_isspace_0x48();
    t_isspace_0x49();
    t_isspace_0x4a();
    t_isspace_0x4b();
    t_isspace_0x4c();
    t_isspace_0x4d();
    t_isspace_0x4e();
    t_isspace_0x4f();
    t_isspace_0x50();
    t_isspace_0x51();
    t_isspace_0x52();
    t_isspace_0x53();
    t_isspace_0x54();
    t_isspace_0x55();
    t_isspace_0x56();
    t_isspace_0x57();
    t_isspace_0x58();
    t_isspace_0x59();
    t_isspace_0x5a();
    t_isspace_0x5b();
    t_isspace_0x5c();
    t_isspace_0x5d();
    t_isspace_0x5e();
    t_isspace_0x5f();
    t_isspace_0x60();
    t_isspace_0x61();
    t_isspace_0x62();
    t_isspace_0x63();
    t_isspace_0x64();
    t_isspace_0x65();
    t_isspace_0x66();
    t_isspace_0x67();
    t_isspace_0x68();
    t_isspace_0x69();
    t_isspace_0x6a();
    t_isspace_0x6b();
    t_isspace_0x6c();
    t_isspace_0x6d();
    t_isspace_0x6e();
    t_isspace_0x6f();
    t_isspace_0x70();
    t_isspace_0x71();
    t_isspace_0x72();
    t_isspace_0x73();
    t_isspace_0x74();
    t_isspace_0x75();
    t_isspace_0x76();
    t_isspace_0x77();
    t_isspace_0x78();
    t_isspace_0x79();
    t_isspace_0x7a();
    t_isspace_0x7b();
    t_isspace_0x7c();
    t_isspace_0x7d();
    t_isspace_0x7e();
    t_isspace_0x7f();
    t_isspace_0x80();
    t_isspace_0x81();
    t_isspace_0x82();
    t_isspace_0x83();
    t_isspace_0x84();
    t_isspace_0x85();
    t_isspace_0x86();
    t_isspace_0x87();
    t_isspace_0x88();
    t_isspace_0x89();
    t_isspace_0x8a();
    t_isspace_0x8b();
    t_isspace_0x8c();
    t_isspace_0x8d();
    t_isspace_0x8e();
    t_isspace_0x8f();
    t_isspace_0x90();
    t_isspace_0x91();
    t_isspace_0x92();
    t_isspace_0x93();
    t_isspace_0x94();
    t_isspace_0x95();
    t_isspace_0x96();
    t_isspace_0x97();
    t_isspace_0x98();
    t_isspace_0x99();
    t_isspace_0x9a();
    t_isspace_0x9b();
    t_isspace_0x9c();
    t_isspace_0x9d();
    t_isspace_0x9e();
    t_isspace_0x9f();
    t_isspace_0xa0();
    t_isspace_0xa1();
    t_isspace_0xa2();
    t_isspace_0xa3();
    t_isspace_0xa4();
    t_isspace_0xa5();
    t_isspace_0xa6();
    t_isspace_0xa7();
    t_isspace_0xa8();
    t_isspace_0xa9();
    t_isspace_0xaa();
    t_isspace_0xab();
    t_isspace_0xac();
    t_isspace_0xad();
    t_isspace_0xae();
    t_isspace_0xaf();
    t_isspace_0xb0();
    t_isspace_0xb1();
    t_isspace_0xb2();
    t_isspace_0xb3();
    t_isspace_0xb4();
    t_isspace_0xb5();
    t_isspace_0xb6();
    t_isspace_0xb7();
    t_isspace_0xb8();
    t_isspace_0xb9();
    t_isspace_0xba();
    t_isspace_0xbb();
    t_isspace_0xbc();
    t_isspace_0xbd();
    t_isspace_0xbe();
    t_isspace_0xbf();
    t_isspace_0xc0();
    t_isspace_0xc1();
    t_isspace_0xc2();
    t_isspace_0xc3();
    t_isspace_0xc4();
    t_isspace_0xc5();
    t_isspace_0xc6();
    t_isspace_0xc7();
    t_isspace_0xc8();
    t_isspace_0xc9();
    t_isspace_0xca();
    t_isspace_0xcb();
    t_isspace_0xcc();
    t_isspace_0xcd();
    t_isspace_0xce();
    t_isspace_0xcf();
    t_isspace_0xd0();
    t_isspace_0xd1();
    t_isspace_0xd2();
    t_isspace_0xd3();
    t_isspace_0xd4();
    t_isspace_0xd5();
    t_isspace_0xd6();
    t_isspace_0xd7();
    t_isspace_0xd8();
    t_isspace_0xd9();
    t_isspace_0xda();
    t_isspace_0xdb();
    t_isspace_0xdc();
    t_isspace_0xdd();
    t_isspace_0xde();
    t_isspace_0xdf();
    t_isspace_0xe0();
    t_isspace_0xe1();
    t_isspace_0xe2();
    t_isspace_0xe3();
    t_isspace_0xe4();
    t_isspace_0xe5();
    t_isspace_0xe6();
    t_isspace_0xe7();
    t_isspace_0xe8();
    t_isspace_0xe9();
    t_isspace_0xea();
    t_isspace_0xeb();
    t_isspace_0xec();
    t_isspace_0xed();
    t_isspace_0xee();
    t_isspace_0xef();
    t_isspace_0xf0();
    t_isspace_0xf1();
    t_isspace_0xf2();
    t_isspace_0xf3();
    t_isspace_0xf4();
    t_isspace_0xf5();
    t_isspace_0xf6();
    t_isspace_0xf7();
    t_isspace_0xf8();
    t_isspace_0xf9();
    t_isspace_0xfa();
    t_isspace_0xfb();
    t_isspace_0xfc();
    t_isspace_0xfd();
    t_isspace_0xfe();
    t_isspace_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
