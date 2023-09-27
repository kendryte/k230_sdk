#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))

void t_islower_0x00()
{
    uassert_int_equal(islower(0), 0); /* islower should be 0 for 0x00 */
}

void t_islower_0x01()
{
    uassert_int_equal(islower(1), 0); /* islower should be 0 for 0x01 */
}

void t_islower_0x02()
{
    uassert_int_equal(islower(2), 0); /* islower should be 0 for 0x02 */
}

void t_islower_0x03()
{
    uassert_int_equal(islower(3), 0); /* islower should be 0 for 0x03 */
}

void t_islower_0x04()
{
    uassert_int_equal(islower(4), 0); /* islower should be 0 for 0x04 */
}

void t_islower_0x05()
{
    uassert_int_equal(islower(5), 0); /* islower should be 0 for 0x05 */
}

void t_islower_0x06()
{
    uassert_int_equal(islower(6), 0); /* islower should be 0 for 0x06 */
}

void t_islower_0x07()
{
    uassert_int_equal(islower(7), 0); /* islower should be 0 for 0x07 */
}

void t_islower_0x08()
{
    uassert_int_equal(islower(8), 0); /* islower should be 0 for 0x08 */
}

void t_islower_0x09()
{
    uassert_int_equal(islower(9), 0); /* islower should be 0 for 0x09 */
}

void t_islower_0x0a()
{
    uassert_int_equal(islower(10), 0); /* islower should be 0 for 0x0a */
}

void t_islower_0x0b()
{
    uassert_int_equal(islower(11), 0); /* islower should be 0 for 0x0b */
}

void t_islower_0x0c()
{
    uassert_int_equal(islower(12), 0); /* islower should be 0 for 0x0c */
}

void t_islower_0x0d()
{
    uassert_int_equal(islower(13), 0); /* islower should be 0 for 0x0d */
}

void t_islower_0x0e()
{
    uassert_int_equal(islower(14), 0); /* islower should be 0 for 0x0e */
}

void t_islower_0x0f()
{
    uassert_int_equal(islower(15), 0); /* islower should be 0 for 0x0f */
}

void t_islower_0x10()
{
    uassert_int_equal(islower(16), 0); /* islower should be 0 for 0x10 */
}

void t_islower_0x11()
{
    uassert_int_equal(islower(17), 0); /* islower should be 0 for 0x11 */
}

void t_islower_0x12()
{
    uassert_int_equal(islower(18), 0); /* islower should be 0 for 0x12 */
}

void t_islower_0x13()
{
    uassert_int_equal(islower(19), 0); /* islower should be 0 for 0x13 */
}

void t_islower_0x14()
{
    uassert_int_equal(islower(20), 0); /* islower should be 0 for 0x14 */
}

void t_islower_0x15()
{
    uassert_int_equal(islower(21), 0); /* islower should be 0 for 0x15 */
}

void t_islower_0x16()
{
    uassert_int_equal(islower(22), 0); /* islower should be 0 for 0x16 */
}

void t_islower_0x17()
{
    uassert_int_equal(islower(23), 0); /* islower should be 0 for 0x17 */
}

void t_islower_0x18()
{
    uassert_int_equal(islower(24), 0); /* islower should be 0 for 0x18 */
}

void t_islower_0x19()
{
    uassert_int_equal(islower(25), 0); /* islower should be 0 for 0x19 */
}

void t_islower_0x1a()
{
    uassert_int_equal(islower(26), 0); /* islower should be 0 for 0x1a */
}

void t_islower_0x1b()
{
    uassert_int_equal(islower(27), 0); /* islower should be 0 for 0x1b */
}

void t_islower_0x1c()
{
    uassert_int_equal(islower(28), 0); /* islower should be 0 for 0x1c */
}

void t_islower_0x1d()
{
    uassert_int_equal(islower(29), 0); /* islower should be 0 for 0x1d */
}

void t_islower_0x1e()
{
    uassert_int_equal(islower(30), 0); /* islower should be 0 for 0x1e */
}

void t_islower_0x1f()
{
    uassert_int_equal(islower(31), 0); /* islower should be 0 for 0x1f */
}

void t_islower_0x20()
{
    uassert_int_equal(islower(32), 0); /* islower should be 0 for   */
}

void t_islower_0x21()
{
    uassert_int_equal(islower(33), 0); /* islower should be 0 for ! */
}

void t_islower_0x22()
{
    uassert_int_equal(islower(34), 0); /* islower should be 0 for 0x22 */
}

void t_islower_0x23()
{
    uassert_int_equal(islower(35), 0); /* islower should be 0 for # */
}

void t_islower_0x24()
{
    uassert_int_equal(islower(36), 0); /* islower should be 0 for $ */
}

void t_islower_0x25()
{
    uassert_int_equal(islower(37), 0); /* islower should be 0 for % */
}

void t_islower_0x26()
{
    uassert_int_equal(islower(38), 0); /* islower should be 0 for & */
}

void t_islower_0x27()
{
    uassert_int_equal(islower(39), 0); /* islower should be 0 for ' */
}

void t_islower_0x28()
{
    uassert_int_equal(islower(40), 0); /* islower should be 0 for ( */
}

void t_islower_0x29()
{
    uassert_int_equal(islower(41), 0); /* islower should be 0 for ) */
}

void t_islower_0x2a()
{
    uassert_int_equal(islower(42), 0); /* islower should be 0 for * */
}

void t_islower_0x2b()
{
    uassert_int_equal(islower(43), 0); /* islower should be 0 for + */
}

void t_islower_0x2c()
{
    uassert_int_equal(islower(44), 0); /* islower should be 0 for , */
}

void t_islower_0x2d()
{
    uassert_int_equal(islower(45), 0); /* islower should be 0 for - */
}

void t_islower_0x2e()
{
    uassert_int_equal(islower(46), 0); /* islower should be 0 for . */
}

void t_islower_0x2f()
{
    uassert_int_equal(islower(47), 0); /* islower should be 0 for / */
}

void t_islower_0x30()
{
    uassert_int_equal(islower(48), 0); /* islower should be 0 for 0 */
}

void t_islower_0x31()
{
    uassert_int_equal(islower(49), 0); /* islower should be 0 for 1 */
}

void t_islower_0x32()
{
    uassert_int_equal(islower(50), 0); /* islower should be 0 for 2 */
}

void t_islower_0x33()
{
    uassert_int_equal(islower(51), 0); /* islower should be 0 for 3 */
}

void t_islower_0x34()
{
    uassert_int_equal(islower(52), 0); /* islower should be 0 for 4 */
}

void t_islower_0x35()
{
    uassert_int_equal(islower(53), 0); /* islower should be 0 for 5 */
}

void t_islower_0x36()
{
    uassert_int_equal(islower(54), 0); /* islower should be 0 for 6 */
}

void t_islower_0x37()
{
    uassert_int_equal(islower(55), 0); /* islower should be 0 for 7 */
}

void t_islower_0x38()
{
    uassert_int_equal(islower(56), 0); /* islower should be 0 for 8 */
}

void t_islower_0x39()
{
    uassert_int_equal(islower(57), 0); /* islower should be 0 for 9 */
}

void t_islower_0x3a()
{
    uassert_int_equal(islower(58), 0); /* islower should be 0 for : */
}

void t_islower_0x3b()
{
    uassert_int_equal(islower(59), 0); /* islower should be 0 for ; */
}

void t_islower_0x3c()
{
    uassert_int_equal(islower(60), 0); /* islower should be 0 for < */
}

void t_islower_0x3d()
{
    uassert_int_equal(islower(61), 0); /* islower should be 0 for = */
}

void t_islower_0x3e()
{
    uassert_int_equal(islower(62), 0); /* islower should be 0 for > */
}

void t_islower_0x3f()
{
    uassert_int_equal(islower(63), 0); /* islower should be 0 for ? */
}

void t_islower_0x40()
{
    uassert_int_equal(islower(64), 0); /* islower should be 0 for @ */
}

void t_islower_0x41()
{
    uassert_int_equal(islower(65), 0); /* islower should be 0 for A */
}

void t_islower_0x42()
{
    uassert_int_equal(islower(66), 0); /* islower should be 0 for B */
}

void t_islower_0x43()
{
    uassert_int_equal(islower(67), 0); /* islower should be 0 for C */
}

void t_islower_0x44()
{
    uassert_int_equal(islower(68), 0); /* islower should be 0 for D */
}

void t_islower_0x45()
{
    uassert_int_equal(islower(69), 0); /* islower should be 0 for E */
}

void t_islower_0x46()
{
    uassert_int_equal(islower(70), 0); /* islower should be 0 for F */
}

void t_islower_0x47()
{
    uassert_int_equal(islower(71), 0); /* islower should be 0 for G */
}

void t_islower_0x48()
{
    uassert_int_equal(islower(72), 0); /* islower should be 0 for H */
}

void t_islower_0x49()
{
    uassert_int_equal(islower(73), 0); /* islower should be 0 for I */
}

void t_islower_0x4a()
{
    uassert_int_equal(islower(74), 0); /* islower should be 0 for J */
}

void t_islower_0x4b()
{
    uassert_int_equal(islower(75), 0); /* islower should be 0 for K */
}

void t_islower_0x4c()
{
    uassert_int_equal(islower(76), 0); /* islower should be 0 for L */
}

void t_islower_0x4d()
{
    uassert_int_equal(islower(77), 0); /* islower should be 0 for M */
}

void t_islower_0x4e()
{
    uassert_int_equal(islower(78), 0); /* islower should be 0 for N */
}

void t_islower_0x4f()
{
    uassert_int_equal(islower(79), 0); /* islower should be 0 for O */
}

void t_islower_0x50()
{
    uassert_int_equal(islower(80), 0); /* islower should be 0 for P */
}

void t_islower_0x51()
{
    uassert_int_equal(islower(81), 0); /* islower should be 0 for Q */
}

void t_islower_0x52()
{
    uassert_int_equal(islower(82), 0); /* islower should be 0 for R */
}

void t_islower_0x53()
{
    uassert_int_equal(islower(83), 0); /* islower should be 0 for S */
}

void t_islower_0x54()
{
    uassert_int_equal(islower(84), 0); /* islower should be 0 for T */
}

void t_islower_0x55()
{
    uassert_int_equal(islower(85), 0); /* islower should be 0 for U */
}

void t_islower_0x56()
{
    uassert_int_equal(islower(86), 0); /* islower should be 0 for V */
}

void t_islower_0x57()
{
    uassert_int_equal(islower(87), 0); /* islower should be 0 for W */
}

void t_islower_0x58()
{
    uassert_int_equal(islower(88), 0); /* islower should be 0 for X */
}

void t_islower_0x59()
{
    uassert_int_equal(islower(89), 0); /* islower should be 0 for Y */
}

void t_islower_0x5a()
{
    uassert_int_equal(islower(90), 0); /* islower should be 0 for Z */
}

void t_islower_0x5b()
{
    uassert_int_equal(islower(91), 0); /* islower should be 0 for [ */
}

void t_islower_0x5c()
{
    uassert_int_equal(islower(92), 0); /* islower should be 0 for 0x5c */
}

void t_islower_0x5d()
{
    uassert_int_equal(islower(93), 0); /* islower should be 0 for ] */
}

void t_islower_0x5e()
{
    uassert_int_equal(islower(94), 0); /* islower should be 0 for ^ */
}

void t_islower_0x5f()
{
    uassert_int_equal(islower(95), 0); /* islower should be 0 for _ */
}

void t_islower_0x60()
{
    uassert_int_equal(islower(96), 0); /* islower should be 0 for ` */
}

void t_islower_0x61()
{
    uassert_int_equal(islower(97), 1); /* islower should be 1 for a */
}

void t_islower_0x62()
{
    uassert_int_equal(islower(98), 1); /* islower should be 1 for b */
}

void t_islower_0x63()
{
    uassert_int_equal(islower(99), 1); /* islower should be 1 for c */
}

void t_islower_0x64()
{
    uassert_int_equal(islower(100), 1); /* islower should be 1 for d */
}

void t_islower_0x65()
{
    uassert_int_equal(islower(101), 1); /* islower should be 1 for e */
}

void t_islower_0x66()
{
    uassert_int_equal(islower(102), 1); /* islower should be 1 for f */
}

void t_islower_0x67()
{
    uassert_int_equal(islower(103), 1); /* islower should be 1 for g */
}

void t_islower_0x68()
{
    uassert_int_equal(islower(104), 1); /* islower should be 1 for h */
}

void t_islower_0x69()
{
    uassert_int_equal(islower(105), 1); /* islower should be 1 for i */
}

void t_islower_0x6a()
{
    uassert_int_equal(islower(106), 1); /* islower should be 1 for j */
}

void t_islower_0x6b()
{
    uassert_int_equal(islower(107), 1); /* islower should be 1 for k */
}

void t_islower_0x6c()
{
    uassert_int_equal(islower(108), 1); /* islower should be 1 for l */
}

void t_islower_0x6d()
{
    uassert_int_equal(islower(109), 1); /* islower should be 1 for m */
}

void t_islower_0x6e()
{
    uassert_int_equal(islower(110), 1); /* islower should be 1 for n */
}

void t_islower_0x6f()
{
    uassert_int_equal(islower(111), 1); /* islower should be 1 for o */
}

void t_islower_0x70()
{
    uassert_int_equal(islower(112), 1); /* islower should be 1 for p */
}

void t_islower_0x71()
{
    uassert_int_equal(islower(113), 1); /* islower should be 1 for q */
}

void t_islower_0x72()
{
    uassert_int_equal(islower(114), 1); /* islower should be 1 for r */
}

void t_islower_0x73()
{
    uassert_int_equal(islower(115), 1); /* islower should be 1 for s */
}

void t_islower_0x74()
{
    uassert_int_equal(islower(116), 1); /* islower should be 1 for t */
}

void t_islower_0x75()
{
    uassert_int_equal(islower(117), 1); /* islower should be 1 for u */
}

void t_islower_0x76()
{
    uassert_int_equal(islower(118), 1); /* islower should be 1 for v */
}

void t_islower_0x77()
{
    uassert_int_equal(islower(119), 1); /* islower should be 1 for w */
}

void t_islower_0x78()
{
    uassert_int_equal(islower(120), 1); /* islower should be 1 for x */
}

void t_islower_0x79()
{
    uassert_int_equal(islower(121), 1); /* islower should be 1 for y */
}

void t_islower_0x7a()
{
    uassert_int_equal(islower(122), 1); /* islower should be 1 for z */
}

void t_islower_0x7b()
{
    uassert_int_equal(islower(123), 0); /* islower should be 0 for { */
}

void t_islower_0x7c()
{
    uassert_int_equal(islower(124), 0); /* islower should be 0 for | */
}

void t_islower_0x7d()
{
    uassert_int_equal(islower(125), 0); /* islower should be 0 for } */
}

void t_islower_0x7e()
{
    uassert_int_equal(islower(126), 0); /* islower should be 0 for ~ */
}

void t_islower_0x7f()
{
    uassert_int_equal(islower(127), 0); /* islower should be 0 for 0x7f */
}

void t_islower_0x80()
{
    uassert_int_equal(islower(128), 0); /* islower should be 0 for 0x80 */
}

void t_islower_0x81()
{
    uassert_int_equal(islower(129), 0); /* islower should be 0 for 0x81 */
}

void t_islower_0x82()
{
    uassert_int_equal(islower(130), 0); /* islower should be 0 for 0x82 */
}

void t_islower_0x83()
{
    uassert_int_equal(islower(131), 0); /* islower should be 0 for 0x83 */
}

void t_islower_0x84()
{
    uassert_int_equal(islower(132), 0); /* islower should be 0 for 0x84 */
}

void t_islower_0x85()
{
    uassert_int_equal(islower(133), 0); /* islower should be 0 for 0x85 */
}

void t_islower_0x86()
{
    uassert_int_equal(islower(134), 0); /* islower should be 0 for 0x86 */
}

void t_islower_0x87()
{
    uassert_int_equal(islower(135), 0); /* islower should be 0 for 0x87 */
}

void t_islower_0x88()
{
    uassert_int_equal(islower(136), 0); /* islower should be 0 for 0x88 */
}

void t_islower_0x89()
{
    uassert_int_equal(islower(137), 0); /* islower should be 0 for 0x89 */
}

void t_islower_0x8a()
{
    uassert_int_equal(islower(138), 0); /* islower should be 0 for 0x8a */
}

void t_islower_0x8b()
{
    uassert_int_equal(islower(139), 0); /* islower should be 0 for 0x8b */
}

void t_islower_0x8c()
{
    uassert_int_equal(islower(140), 0); /* islower should be 0 for 0x8c */
}

void t_islower_0x8d()
{
    uassert_int_equal(islower(141), 0); /* islower should be 0 for 0x8d */
}

void t_islower_0x8e()
{
    uassert_int_equal(islower(142), 0); /* islower should be 0 for 0x8e */
}

void t_islower_0x8f()
{
    uassert_int_equal(islower(143), 0); /* islower should be 0 for 0x8f */
}

void t_islower_0x90()
{
    uassert_int_equal(islower(144), 0); /* islower should be 0 for 0x90 */
}

void t_islower_0x91()
{
    uassert_int_equal(islower(145), 0); /* islower should be 0 for 0x91 */
}

void t_islower_0x92()
{
    uassert_int_equal(islower(146), 0); /* islower should be 0 for 0x92 */
}

void t_islower_0x93()
{
    uassert_int_equal(islower(147), 0); /* islower should be 0 for 0x93 */
}

void t_islower_0x94()
{
    uassert_int_equal(islower(148), 0); /* islower should be 0 for 0x94 */
}

void t_islower_0x95()
{
    uassert_int_equal(islower(149), 0); /* islower should be 0 for 0x95 */
}

void t_islower_0x96()
{
    uassert_int_equal(islower(150), 0); /* islower should be 0 for 0x96 */
}

void t_islower_0x97()
{
    uassert_int_equal(islower(151), 0); /* islower should be 0 for 0x97 */
}

void t_islower_0x98()
{
    uassert_int_equal(islower(152), 0); /* islower should be 0 for 0x98 */
}

void t_islower_0x99()
{
    uassert_int_equal(islower(153), 0); /* islower should be 0 for 0x99 */
}

void t_islower_0x9a()
{
    uassert_int_equal(islower(154), 0); /* islower should be 0 for 0x9a */
}

void t_islower_0x9b()
{
    uassert_int_equal(islower(155), 0); /* islower should be 0 for 0x9b */
}

void t_islower_0x9c()
{
    uassert_int_equal(islower(156), 0); /* islower should be 0 for 0x9c */
}

void t_islower_0x9d()
{
    uassert_int_equal(islower(157), 0); /* islower should be 0 for 0x9d */
}

void t_islower_0x9e()
{
    uassert_int_equal(islower(158), 0); /* islower should be 0 for 0x9e */
}

void t_islower_0x9f()
{
    uassert_int_equal(islower(159), 0); /* islower should be 0 for 0x9f */
}

void t_islower_0xa0()
{
    uassert_int_equal(islower(160), 0); /* islower should be 0 for 0xa0 */
}

void t_islower_0xa1()
{
    uassert_int_equal(islower(161), 0); /* islower should be 0 for 0xa1 */
}

void t_islower_0xa2()
{
    uassert_int_equal(islower(162), 0); /* islower should be 0 for 0xa2 */
}

void t_islower_0xa3()
{
    uassert_int_equal(islower(163), 0); /* islower should be 0 for 0xa3 */
}

void t_islower_0xa4()
{
    uassert_int_equal(islower(164), 0); /* islower should be 0 for 0xa4 */
}

void t_islower_0xa5()
{
    uassert_int_equal(islower(165), 0); /* islower should be 0 for 0xa5 */
}

void t_islower_0xa6()
{
    uassert_int_equal(islower(166), 0); /* islower should be 0 for 0xa6 */
}

void t_islower_0xa7()
{
    uassert_int_equal(islower(167), 0); /* islower should be 0 for 0xa7 */
}

void t_islower_0xa8()
{
    uassert_int_equal(islower(168), 0); /* islower should be 0 for 0xa8 */
}

void t_islower_0xa9()
{
    uassert_int_equal(islower(169), 0); /* islower should be 0 for 0xa9 */
}

void t_islower_0xaa()
{
    uassert_int_equal(islower(170), 0); /* islower should be 0 for 0xaa */
}

void t_islower_0xab()
{
    uassert_int_equal(islower(171), 0); /* islower should be 0 for 0xab */
}

void t_islower_0xac()
{
    uassert_int_equal(islower(172), 0); /* islower should be 0 for 0xac */
}

void t_islower_0xad()
{
    uassert_int_equal(islower(173), 0); /* islower should be 0 for 0xad */
}

void t_islower_0xae()
{
    uassert_int_equal(islower(174), 0); /* islower should be 0 for 0xae */
}

void t_islower_0xaf()
{
    uassert_int_equal(islower(175), 0); /* islower should be 0 for 0xaf */
}

void t_islower_0xb0()
{
    uassert_int_equal(islower(176), 0); /* islower should be 0 for 0xb0 */
}

void t_islower_0xb1()
{
    uassert_int_equal(islower(177), 0); /* islower should be 0 for 0xb1 */
}

void t_islower_0xb2()
{
    uassert_int_equal(islower(178), 0); /* islower should be 0 for 0xb2 */
}

void t_islower_0xb3()
{
    uassert_int_equal(islower(179), 0); /* islower should be 0 for 0xb3 */
}

void t_islower_0xb4()
{
    uassert_int_equal(islower(180), 0); /* islower should be 0 for 0xb4 */
}

void t_islower_0xb5()
{
    uassert_int_equal(islower(181), 0); /* islower should be 0 for 0xb5 */
}

void t_islower_0xb6()
{
    uassert_int_equal(islower(182), 0); /* islower should be 0 for 0xb6 */
}

void t_islower_0xb7()
{
    uassert_int_equal(islower(183), 0); /* islower should be 0 for 0xb7 */
}

void t_islower_0xb8()
{
    uassert_int_equal(islower(184), 0); /* islower should be 0 for 0xb8 */
}

void t_islower_0xb9()
{
    uassert_int_equal(islower(185), 0); /* islower should be 0 for 0xb9 */
}

void t_islower_0xba()
{
    uassert_int_equal(islower(186), 0); /* islower should be 0 for 0xba */
}

void t_islower_0xbb()
{
    uassert_int_equal(islower(187), 0); /* islower should be 0 for 0xbb */
}

void t_islower_0xbc()
{
    uassert_int_equal(islower(188), 0); /* islower should be 0 for 0xbc */
}

void t_islower_0xbd()
{
    uassert_int_equal(islower(189), 0); /* islower should be 0 for 0xbd */
}

void t_islower_0xbe()
{
    uassert_int_equal(islower(190), 0); /* islower should be 0 for 0xbe */
}

void t_islower_0xbf()
{
    uassert_int_equal(islower(191), 0); /* islower should be 0 for 0xbf */
}

void t_islower_0xc0()
{
    uassert_int_equal(islower(192), 0); /* islower should be 0 for 0xc0 */
}

void t_islower_0xc1()
{
    uassert_int_equal(islower(193), 0); /* islower should be 0 for 0xc1 */
}

void t_islower_0xc2()
{
    uassert_int_equal(islower(194), 0); /* islower should be 0 for 0xc2 */
}

void t_islower_0xc3()
{
    uassert_int_equal(islower(195), 0); /* islower should be 0 for 0xc3 */
}

void t_islower_0xc4()
{
    uassert_int_equal(islower(196), 0); /* islower should be 0 for 0xc4 */
}

void t_islower_0xc5()
{
    uassert_int_equal(islower(197), 0); /* islower should be 0 for 0xc5 */
}

void t_islower_0xc6()
{
    uassert_int_equal(islower(198), 0); /* islower should be 0 for 0xc6 */
}

void t_islower_0xc7()
{
    uassert_int_equal(islower(199), 0); /* islower should be 0 for 0xc7 */
}

void t_islower_0xc8()
{
    uassert_int_equal(islower(200), 0); /* islower should be 0 for 0xc8 */
}

void t_islower_0xc9()
{
    uassert_int_equal(islower(201), 0); /* islower should be 0 for 0xc9 */
}

void t_islower_0xca()
{
    uassert_int_equal(islower(202), 0); /* islower should be 0 for 0xca */
}

void t_islower_0xcb()
{
    uassert_int_equal(islower(203), 0); /* islower should be 0 for 0xcb */
}

void t_islower_0xcc()
{
    uassert_int_equal(islower(204), 0); /* islower should be 0 for 0xcc */
}

void t_islower_0xcd()
{
    uassert_int_equal(islower(205), 0); /* islower should be 0 for 0xcd */
}

void t_islower_0xce()
{
    uassert_int_equal(islower(206), 0); /* islower should be 0 for 0xce */
}

void t_islower_0xcf()
{
    uassert_int_equal(islower(207), 0); /* islower should be 0 for 0xcf */
}

void t_islower_0xd0()
{
    uassert_int_equal(islower(208), 0); /* islower should be 0 for 0xd0 */
}

void t_islower_0xd1()
{
    uassert_int_equal(islower(209), 0); /* islower should be 0 for 0xd1 */
}

void t_islower_0xd2()
{
    uassert_int_equal(islower(210), 0); /* islower should be 0 for 0xd2 */
}

void t_islower_0xd3()
{
    uassert_int_equal(islower(211), 0); /* islower should be 0 for 0xd3 */
}

void t_islower_0xd4()
{
    uassert_int_equal(islower(212), 0); /* islower should be 0 for 0xd4 */
}

void t_islower_0xd5()
{
    uassert_int_equal(islower(213), 0); /* islower should be 0 for 0xd5 */
}

void t_islower_0xd6()
{
    uassert_int_equal(islower(214), 0); /* islower should be 0 for 0xd6 */
}

void t_islower_0xd7()
{
    uassert_int_equal(islower(215), 0); /* islower should be 0 for 0xd7 */
}

void t_islower_0xd8()
{
    uassert_int_equal(islower(216), 0); /* islower should be 0 for 0xd8 */
}

void t_islower_0xd9()
{
    uassert_int_equal(islower(217), 0); /* islower should be 0 for 0xd9 */
}

void t_islower_0xda()
{
    uassert_int_equal(islower(218), 0); /* islower should be 0 for 0xda */
}

void t_islower_0xdb()
{
    uassert_int_equal(islower(219), 0); /* islower should be 0 for 0xdb */
}

void t_islower_0xdc()
{
    uassert_int_equal(islower(220), 0); /* islower should be 0 for 0xdc */
}

void t_islower_0xdd()
{
    uassert_int_equal(islower(221), 0); /* islower should be 0 for 0xdd */
}

void t_islower_0xde()
{
    uassert_int_equal(islower(222), 0); /* islower should be 0 for 0xde */
}

void t_islower_0xdf()
{
    uassert_int_equal(islower(223), 0); /* islower should be 0 for 0xdf */
}

void t_islower_0xe0()
{
    uassert_int_equal(islower(224), 0); /* islower should be 0 for 0xe0 */
}

void t_islower_0xe1()
{
    uassert_int_equal(islower(225), 0); /* islower should be 0 for 0xe1 */
}

void t_islower_0xe2()
{
    uassert_int_equal(islower(226), 0); /* islower should be 0 for 0xe2 */
}

void t_islower_0xe3()
{
    uassert_int_equal(islower(227), 0); /* islower should be 0 for 0xe3 */
}

void t_islower_0xe4()
{
    uassert_int_equal(islower(228), 0); /* islower should be 0 for 0xe4 */
}

void t_islower_0xe5()
{
    uassert_int_equal(islower(229), 0); /* islower should be 0 for 0xe5 */
}

void t_islower_0xe6()
{
    uassert_int_equal(islower(230), 0); /* islower should be 0 for 0xe6 */
}

void t_islower_0xe7()
{
    uassert_int_equal(islower(231), 0); /* islower should be 0 for 0xe7 */
}

void t_islower_0xe8()
{
    uassert_int_equal(islower(232), 0); /* islower should be 0 for 0xe8 */
}

void t_islower_0xe9()
{
    uassert_int_equal(islower(233), 0); /* islower should be 0 for 0xe9 */
}

void t_islower_0xea()
{
    uassert_int_equal(islower(234), 0); /* islower should be 0 for 0xea */
}

void t_islower_0xeb()
{
    uassert_int_equal(islower(235), 0); /* islower should be 0 for 0xeb */
}

void t_islower_0xec()
{
    uassert_int_equal(islower(236), 0); /* islower should be 0 for 0xec */
}

void t_islower_0xed()
{
    uassert_int_equal(islower(237), 0); /* islower should be 0 for 0xed */
}

void t_islower_0xee()
{
    uassert_int_equal(islower(238), 0); /* islower should be 0 for 0xee */
}

void t_islower_0xef()
{
    uassert_int_equal(islower(239), 0); /* islower should be 0 for 0xef */
}

void t_islower_0xf0()
{
    uassert_int_equal(islower(240), 0); /* islower should be 0 for 0xf0 */
}

void t_islower_0xf1()
{
    uassert_int_equal(islower(241), 0); /* islower should be 0 for 0xf1 */
}

void t_islower_0xf2()
{
    uassert_int_equal(islower(242), 0); /* islower should be 0 for 0xf2 */
}

void t_islower_0xf3()
{
    uassert_int_equal(islower(243), 0); /* islower should be 0 for 0xf3 */
}

void t_islower_0xf4()
{
    uassert_int_equal(islower(244), 0); /* islower should be 0 for 0xf4 */
}

void t_islower_0xf5()
{
    uassert_int_equal(islower(245), 0); /* islower should be 0 for 0xf5 */
}

void t_islower_0xf6()
{
    uassert_int_equal(islower(246), 0); /* islower should be 0 for 0xf6 */
}

void t_islower_0xf7()
{
    uassert_int_equal(islower(247), 0); /* islower should be 0 for 0xf7 */
}

void t_islower_0xf8()
{
    uassert_int_equal(islower(248), 0); /* islower should be 0 for 0xf8 */
}

void t_islower_0xf9()
{
    uassert_int_equal(islower(249), 0); /* islower should be 0 for 0xf9 */
}

void t_islower_0xfa()
{
    uassert_int_equal(islower(250), 0); /* islower should be 0 for 0xfa */
}

void t_islower_0xfb()
{
    uassert_int_equal(islower(251), 0); /* islower should be 0 for 0xfb */
}

void t_islower_0xfc()
{
    uassert_int_equal(islower(252), 0); /* islower should be 0 for 0xfc */
}

void t_islower_0xfd()
{
    uassert_int_equal(islower(253), 0); /* islower should be 0 for 0xfd */
}

void t_islower_0xfe()
{
    uassert_int_equal(islower(254), 0); /* islower should be 0 for 0xfe */
}

void t_islower_0xff()
{
    uassert_int_equal(islower(255), 0); /* islower should be 0 for 0xff */
}



static int testcase(void)
{
    t_islower_0x00();
    t_islower_0x01();
    t_islower_0x02();
    t_islower_0x03();
    t_islower_0x04();
    t_islower_0x05();
    t_islower_0x06();
    t_islower_0x07();
    t_islower_0x08();
    t_islower_0x09();
    t_islower_0x0a();
    t_islower_0x0b();
    t_islower_0x0c();
    t_islower_0x0d();
    t_islower_0x0e();
    t_islower_0x0f();
    t_islower_0x10();
    t_islower_0x11();
    t_islower_0x12();
    t_islower_0x13();
    t_islower_0x14();
    t_islower_0x15();
    t_islower_0x16();
    t_islower_0x17();
    t_islower_0x18();
    t_islower_0x19();
    t_islower_0x1a();
    t_islower_0x1b();
    t_islower_0x1c();
    t_islower_0x1d();
    t_islower_0x1e();
    t_islower_0x1f();
    t_islower_0x20();
    t_islower_0x21();
    t_islower_0x22();
    t_islower_0x23();
    t_islower_0x24();
    t_islower_0x25();
    t_islower_0x26();
    t_islower_0x27();
    t_islower_0x28();
    t_islower_0x29();
    t_islower_0x2a();
    t_islower_0x2b();
    t_islower_0x2c();
    t_islower_0x2d();
    t_islower_0x2e();
    t_islower_0x2f();
    t_islower_0x30();
    t_islower_0x31();
    t_islower_0x32();
    t_islower_0x33();
    t_islower_0x34();
    t_islower_0x35();
    t_islower_0x36();
    t_islower_0x37();
    t_islower_0x38();
    t_islower_0x39();
    t_islower_0x3a();
    t_islower_0x3b();
    t_islower_0x3c();
    t_islower_0x3d();
    t_islower_0x3e();
    t_islower_0x3f();
    t_islower_0x40();
    t_islower_0x41();
    t_islower_0x42();
    t_islower_0x43();
    t_islower_0x44();
    t_islower_0x45();
    t_islower_0x46();
    t_islower_0x47();
    t_islower_0x48();
    t_islower_0x49();
    t_islower_0x4a();
    t_islower_0x4b();
    t_islower_0x4c();
    t_islower_0x4d();
    t_islower_0x4e();
    t_islower_0x4f();
    t_islower_0x50();
    t_islower_0x51();
    t_islower_0x52();
    t_islower_0x53();
    t_islower_0x54();
    t_islower_0x55();
    t_islower_0x56();
    t_islower_0x57();
    t_islower_0x58();
    t_islower_0x59();
    t_islower_0x5a();
    t_islower_0x5b();
    t_islower_0x5c();
    t_islower_0x5d();
    t_islower_0x5e();
    t_islower_0x5f();
    t_islower_0x60();
    t_islower_0x61();
    t_islower_0x62();
    t_islower_0x63();
    t_islower_0x64();
    t_islower_0x65();
    t_islower_0x66();
    t_islower_0x67();
    t_islower_0x68();
    t_islower_0x69();
    t_islower_0x6a();
    t_islower_0x6b();
    t_islower_0x6c();
    t_islower_0x6d();
    t_islower_0x6e();
    t_islower_0x6f();
    t_islower_0x70();
    t_islower_0x71();
    t_islower_0x72();
    t_islower_0x73();
    t_islower_0x74();
    t_islower_0x75();
    t_islower_0x76();
    t_islower_0x77();
    t_islower_0x78();
    t_islower_0x79();
    t_islower_0x7a();
    t_islower_0x7b();
    t_islower_0x7c();
    t_islower_0x7d();
    t_islower_0x7e();
    t_islower_0x7f();
    t_islower_0x80();
    t_islower_0x81();
    t_islower_0x82();
    t_islower_0x83();
    t_islower_0x84();
    t_islower_0x85();
    t_islower_0x86();
    t_islower_0x87();
    t_islower_0x88();
    t_islower_0x89();
    t_islower_0x8a();
    t_islower_0x8b();
    t_islower_0x8c();
    t_islower_0x8d();
    t_islower_0x8e();
    t_islower_0x8f();
    t_islower_0x90();
    t_islower_0x91();
    t_islower_0x92();
    t_islower_0x93();
    t_islower_0x94();
    t_islower_0x95();
    t_islower_0x96();
    t_islower_0x97();
    t_islower_0x98();
    t_islower_0x99();
    t_islower_0x9a();
    t_islower_0x9b();
    t_islower_0x9c();
    t_islower_0x9d();
    t_islower_0x9e();
    t_islower_0x9f();
    t_islower_0xa0();
    t_islower_0xa1();
    t_islower_0xa2();
    t_islower_0xa3();
    t_islower_0xa4();
    t_islower_0xa5();
    t_islower_0xa6();
    t_islower_0xa7();
    t_islower_0xa8();
    t_islower_0xa9();
    t_islower_0xaa();
    t_islower_0xab();
    t_islower_0xac();
    t_islower_0xad();
    t_islower_0xae();
    t_islower_0xaf();
    t_islower_0xb0();
    t_islower_0xb1();
    t_islower_0xb2();
    t_islower_0xb3();
    t_islower_0xb4();
    t_islower_0xb5();
    t_islower_0xb6();
    t_islower_0xb7();
    t_islower_0xb8();
    t_islower_0xb9();
    t_islower_0xba();
    t_islower_0xbb();
    t_islower_0xbc();
    t_islower_0xbd();
    t_islower_0xbe();
    t_islower_0xbf();
    t_islower_0xc0();
    t_islower_0xc1();
    t_islower_0xc2();
    t_islower_0xc3();
    t_islower_0xc4();
    t_islower_0xc5();
    t_islower_0xc6();
    t_islower_0xc7();
    t_islower_0xc8();
    t_islower_0xc9();
    t_islower_0xca();
    t_islower_0xcb();
    t_islower_0xcc();
    t_islower_0xcd();
    t_islower_0xce();
    t_islower_0xcf();
    t_islower_0xd0();
    t_islower_0xd1();
    t_islower_0xd2();
    t_islower_0xd3();
    t_islower_0xd4();
    t_islower_0xd5();
    t_islower_0xd6();
    t_islower_0xd7();
    t_islower_0xd8();
    t_islower_0xd9();
    t_islower_0xda();
    t_islower_0xdb();
    t_islower_0xdc();
    t_islower_0xdd();
    t_islower_0xde();
    t_islower_0xdf();
    t_islower_0xe0();
    t_islower_0xe1();
    t_islower_0xe2();
    t_islower_0xe3();
    t_islower_0xe4();
    t_islower_0xe5();
    t_islower_0xe6();
    t_islower_0xe7();
    t_islower_0xe8();
    t_islower_0xe9();
    t_islower_0xea();
    t_islower_0xeb();
    t_islower_0xec();
    t_islower_0xed();
    t_islower_0xee();
    t_islower_0xef();
    t_islower_0xf0();
    t_islower_0xf1();
    t_islower_0xf2();
    t_islower_0xf3();
    t_islower_0xf4();
    t_islower_0xf5();
    t_islower_0xf6();
    t_islower_0xf7();
    t_islower_0xf8();
    t_islower_0xf9();
    t_islower_0xfa();
    t_islower_0xfb();
    t_islower_0xfc();
    t_islower_0xfd();
    t_islower_0xfe();
    t_islower_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
