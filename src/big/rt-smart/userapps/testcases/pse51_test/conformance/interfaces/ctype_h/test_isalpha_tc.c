#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))

void t_isalpha_0x00()
{
    uassert_int_equal(isalpha(0), 0); /* isalpha should be 0 for 0x00 */
}

void t_isalpha_0x01()
{
    uassert_int_equal(isalpha(1), 0); /* isalpha should be 0 for 0x01 */
}

void t_isalpha_0x02()
{
    uassert_int_equal(isalpha(2), 0); /* isalpha should be 0 for 0x02 */
}

void t_isalpha_0x03()
{
    uassert_int_equal(isalpha(3), 0); /* isalpha should be 0 for 0x03 */
}

void t_isalpha_0x04()
{
    uassert_int_equal(isalpha(4), 0); /* isalpha should be 0 for 0x04 */
}

void t_isalpha_0x05()
{
    uassert_int_equal(isalpha(5), 0); /* isalpha should be 0 for 0x05 */
}

void t_isalpha_0x06()
{
    uassert_int_equal(isalpha(6), 0); /* isalpha should be 0 for 0x06 */
}

void t_isalpha_0x07()
{
    uassert_int_equal(isalpha(7), 0); /* isalpha should be 0 for 0x07 */
}

void t_isalpha_0x08()
{
    uassert_int_equal(isalpha(8), 0); /* isalpha should be 0 for 0x08 */
}

void t_isalpha_0x09()
{
    uassert_int_equal(isalpha(9), 0); /* isalpha should be 0 for 0x09 */
}

void t_isalpha_0x0a()
{
    uassert_int_equal(isalpha(10), 0); /* isalpha should be 0 for 0x0a */
}

void t_isalpha_0x0b()
{
    uassert_int_equal(isalpha(11), 0); /* isalpha should be 0 for 0x0b */
}

void t_isalpha_0x0c()
{
    uassert_int_equal(isalpha(12), 0); /* isalpha should be 0 for 0x0c */
}

void t_isalpha_0x0d()
{
    uassert_int_equal(isalpha(13), 0); /* isalpha should be 0 for 0x0d */
}

void t_isalpha_0x0e()
{
    uassert_int_equal(isalpha(14), 0); /* isalpha should be 0 for 0x0e */
}

void t_isalpha_0x0f()
{
    uassert_int_equal(isalpha(15), 0); /* isalpha should be 0 for 0x0f */
}

void t_isalpha_0x10()
{
    uassert_int_equal(isalpha(16), 0); /* isalpha should be 0 for 0x10 */
}

void t_isalpha_0x11()
{
    uassert_int_equal(isalpha(17), 0); /* isalpha should be 0 for 0x11 */
}

void t_isalpha_0x12()
{
    uassert_int_equal(isalpha(18), 0); /* isalpha should be 0 for 0x12 */
}

void t_isalpha_0x13()
{
    uassert_int_equal(isalpha(19), 0); /* isalpha should be 0 for 0x13 */
}

void t_isalpha_0x14()
{
    uassert_int_equal(isalpha(20), 0); /* isalpha should be 0 for 0x14 */
}

void t_isalpha_0x15()
{
    uassert_int_equal(isalpha(21), 0); /* isalpha should be 0 for 0x15 */
}

void t_isalpha_0x16()
{
    uassert_int_equal(isalpha(22), 0); /* isalpha should be 0 for 0x16 */
}

void t_isalpha_0x17()
{
    uassert_int_equal(isalpha(23), 0); /* isalpha should be 0 for 0x17 */
}

void t_isalpha_0x18()
{
    uassert_int_equal(isalpha(24), 0); /* isalpha should be 0 for 0x18 */
}

void t_isalpha_0x19()
{
    uassert_int_equal(isalpha(25), 0); /* isalpha should be 0 for 0x19 */
}

void t_isalpha_0x1a()
{
    uassert_int_equal(isalpha(26), 0); /* isalpha should be 0 for 0x1a */
}

void t_isalpha_0x1b()
{
    uassert_int_equal(isalpha(27), 0); /* isalpha should be 0 for 0x1b */
}

void t_isalpha_0x1c()
{
    uassert_int_equal(isalpha(28), 0); /* isalpha should be 0 for 0x1c */
}

void t_isalpha_0x1d()
{
    uassert_int_equal(isalpha(29), 0); /* isalpha should be 0 for 0x1d */
}

void t_isalpha_0x1e()
{
    uassert_int_equal(isalpha(30), 0); /* isalpha should be 0 for 0x1e */
}

void t_isalpha_0x1f()
{
    uassert_int_equal(isalpha(31), 0); /* isalpha should be 0 for 0x1f */
}

void t_isalpha_0x20()
{
    uassert_int_equal(isalpha(32), 0); /* isalpha should be 0 for   */
}

void t_isalpha_0x21()
{
    uassert_int_equal(isalpha(33), 0); /* isalpha should be 0 for ! */
}

void t_isalpha_0x22()
{
    uassert_int_equal(isalpha(34), 0); /* isalpha should be 0 for 0x22 */
}

void t_isalpha_0x23()
{
    uassert_int_equal(isalpha(35), 0); /* isalpha should be 0 for # */
}

void t_isalpha_0x24()
{
    uassert_int_equal(isalpha(36), 0); /* isalpha should be 0 for $ */
}

void t_isalpha_0x25()
{
    uassert_int_equal(isalpha(37), 0); /* isalpha should be 0 for % */
}

void t_isalpha_0x26()
{
    uassert_int_equal(isalpha(38), 0); /* isalpha should be 0 for & */
}

void t_isalpha_0x27()
{
    uassert_int_equal(isalpha(39), 0); /* isalpha should be 0 for ' */
}

void t_isalpha_0x28()
{
    uassert_int_equal(isalpha(40), 0); /* isalpha should be 0 for ( */
}

void t_isalpha_0x29()
{
    uassert_int_equal(isalpha(41), 0); /* isalpha should be 0 for ) */
}

void t_isalpha_0x2a()
{
    uassert_int_equal(isalpha(42), 0); /* isalpha should be 0 for * */
}

void t_isalpha_0x2b()
{
    uassert_int_equal(isalpha(43), 0); /* isalpha should be 0 for + */
}

void t_isalpha_0x2c()
{
    uassert_int_equal(isalpha(44), 0); /* isalpha should be 0 for , */
}

void t_isalpha_0x2d()
{
    uassert_int_equal(isalpha(45), 0); /* isalpha should be 0 for - */
}

void t_isalpha_0x2e()
{
    uassert_int_equal(isalpha(46), 0); /* isalpha should be 0 for . */
}

void t_isalpha_0x2f()
{
    uassert_int_equal(isalpha(47), 0); /* isalpha should be 0 for / */
}

void t_isalpha_0x30()
{
    uassert_int_equal(isalpha(48), 0); /* isalpha should be 0 for 0 */
}

void t_isalpha_0x31()
{
    uassert_int_equal(isalpha(49), 0); /* isalpha should be 0 for 1 */
}

void t_isalpha_0x32()
{
    uassert_int_equal(isalpha(50), 0); /* isalpha should be 0 for 2 */
}

void t_isalpha_0x33()
{
    uassert_int_equal(isalpha(51), 0); /* isalpha should be 0 for 3 */
}

void t_isalpha_0x34()
{
    uassert_int_equal(isalpha(52), 0); /* isalpha should be 0 for 4 */
}

void t_isalpha_0x35()
{
    uassert_int_equal(isalpha(53), 0); /* isalpha should be 0 for 5 */
}

void t_isalpha_0x36()
{
    uassert_int_equal(isalpha(54), 0); /* isalpha should be 0 for 6 */
}

void t_isalpha_0x37()
{
    uassert_int_equal(isalpha(55), 0); /* isalpha should be 0 for 7 */
}

void t_isalpha_0x38()
{
    uassert_int_equal(isalpha(56), 0); /* isalpha should be 0 for 8 */
}

void t_isalpha_0x39()
{
    uassert_int_equal(isalpha(57), 0); /* isalpha should be 0 for 9 */
}

void t_isalpha_0x3a()
{
    uassert_int_equal(isalpha(58), 0); /* isalpha should be 0 for : */
}

void t_isalpha_0x3b()
{
    uassert_int_equal(isalpha(59), 0); /* isalpha should be 0 for ; */
}

void t_isalpha_0x3c()
{
    uassert_int_equal(isalpha(60), 0); /* isalpha should be 0 for < */
}

void t_isalpha_0x3d()
{
    uassert_int_equal(isalpha(61), 0); /* isalpha should be 0 for = */
}

void t_isalpha_0x3e()
{
    uassert_int_equal(isalpha(62), 0); /* isalpha should be 0 for > */
}

void t_isalpha_0x3f()
{
    uassert_int_equal(isalpha(63), 0); /* isalpha should be 0 for ? */
}

void t_isalpha_0x40()
{
    uassert_int_equal(isalpha(64), 0); /* isalpha should be 0 for @ */
}

void t_isalpha_0x41()
{
    uassert_int_equal(isalpha(65), 1); /*isalpha should be 1 for A */
}

void t_isalpha_0x42()
{
    uassert_int_equal(isalpha(66), 1); /*isalpha should be 1 for B */
}

void t_isalpha_0x43()
{
    uassert_int_equal(isalpha(67), 1); /*isalpha should be 1 for C */
}

void t_isalpha_0x44()
{
    uassert_int_equal(isalpha(68), 1); /*isalpha should be 1 for D */
}

void t_isalpha_0x45()
{
    uassert_int_equal(isalpha(69), 1); /*isalpha should be 1 for E */
}

void t_isalpha_0x46()
{
    uassert_int_equal(isalpha(70), 1); /*isalpha should be 1 for F */
}

void t_isalpha_0x47()
{
    uassert_int_equal(isalpha(71), 1); /*isalpha should be 1 for G */
}

void t_isalpha_0x48()
{
    uassert_int_equal(isalpha(72), 1); /*isalpha should be 1 for H */
}

void t_isalpha_0x49()
{
    uassert_int_equal(isalpha(73), 1); /*isalpha should be 1 for I */
}

void t_isalpha_0x4a()
{
    uassert_int_equal(isalpha(74), 1); /*isalpha should be 1 for J */
}

void t_isalpha_0x4b()
{
    uassert_int_equal(isalpha(75), 1); /*isalpha should be 1 for K */
}

void t_isalpha_0x4c()
{
    uassert_int_equal(isalpha(76), 1); /*isalpha should be 1 for L */
}

void t_isalpha_0x4d()
{
    uassert_int_equal(isalpha(77), 1); /*isalpha should be 1 for M */
}

void t_isalpha_0x4e()
{
    uassert_int_equal(isalpha(78), 1); /*isalpha should be 1 for N */
}

void t_isalpha_0x4f()
{
    uassert_int_equal(isalpha(79), 1); /*isalpha should be 1 for O */
}

void t_isalpha_0x50()
{
    uassert_int_equal(isalpha(80), 1); /*isalpha should be 1 for P */
}

void t_isalpha_0x51()
{
    uassert_int_equal(isalpha(81), 1); /*isalpha should be 1 for Q */
}

void t_isalpha_0x52()
{
    uassert_int_equal(isalpha(82), 1); /*isalpha should be 1 for R */
}

void t_isalpha_0x53()
{
    uassert_int_equal(isalpha(83), 1); /*isalpha should be 1 for S */
}

void t_isalpha_0x54()
{
    uassert_int_equal(isalpha(84), 1); /*isalpha should be 1 for T */
}

void t_isalpha_0x55()
{
    uassert_int_equal(isalpha(85), 1); /*isalpha should be 1 for U */
}

void t_isalpha_0x56()
{
    uassert_int_equal(isalpha(86), 1); /*isalpha should be 1 for V */
}

void t_isalpha_0x57()
{
    uassert_int_equal(isalpha(87), 1); /*isalpha should be 1 for W */
}

void t_isalpha_0x58()
{
    uassert_int_equal(isalpha(88), 1); /*isalpha should be 1 for X */
}

void t_isalpha_0x59()
{
    uassert_int_equal(isalpha(89), 1); /*isalpha should be 1 for Y */
}

void t_isalpha_0x5a()
{
    uassert_int_equal(isalpha(90), 1); /*isalpha should be 1 for Z */
}

void t_isalpha_0x5b()
{
    uassert_int_equal(isalpha(91), 0); /* isalpha should be 0 for [ */
}

void t_isalpha_0x5c()
{
    uassert_int_equal(isalpha(92), 0); /* isalpha should be 0 for 0x5c */
}

void t_isalpha_0x5d()
{
    uassert_int_equal(isalpha(93), 0); /* isalpha should be 0 for ] */
}

void t_isalpha_0x5e()
{
    uassert_int_equal(isalpha(94), 0); /* isalpha should be 0 for ^ */
}

void t_isalpha_0x5f()
{
    uassert_int_equal(isalpha(95), 0); /* isalpha should be 0 for _ */
}

void t_isalpha_0x60()
{
    uassert_int_equal(isalpha(96), 0); /* isalpha should be 0 for ` */
}

void t_isalpha_0x61()
{
    uassert_int_equal(isalpha(97), 1); /*isalpha should be 1 for a */
}

void t_isalpha_0x62()
{
    uassert_int_equal(isalpha(98), 1); /*isalpha should be 1 for b */
}

void t_isalpha_0x63()
{
    uassert_int_equal(isalpha(99), 1); /*isalpha should be 1 for c */
}

void t_isalpha_0x64()
{
    uassert_int_equal(isalpha(100), 1); /*isalpha should be 1 for d */
}

void t_isalpha_0x65()
{
    uassert_int_equal(isalpha(101), 1); /*isalpha should be 1 for e */
}

void t_isalpha_0x66()
{
    uassert_int_equal(isalpha(102), 1); /*isalpha should be 1 for f */
}

void t_isalpha_0x67()
{
    uassert_int_equal(isalpha(103), 1); /*isalpha should be 1 for g */
}

void t_isalpha_0x68()
{
    uassert_int_equal(isalpha(104), 1); /*isalpha should be 1 for h */
}

void t_isalpha_0x69()
{
    uassert_int_equal(isalpha(105), 1); /*isalpha should be 1 for i */
}

void t_isalpha_0x6a()
{
    uassert_int_equal(isalpha(106), 1); /*isalpha should be 1 for j */
}

void t_isalpha_0x6b()
{
    uassert_int_equal(isalpha(107), 1); /*isalpha should be 1 for k */
}

void t_isalpha_0x6c()
{
    uassert_int_equal(isalpha(108), 1); /*isalpha should be 1 for l */
}

void t_isalpha_0x6d()
{
    uassert_int_equal(isalpha(109), 1); /*isalpha should be 1 for m */
}

void t_isalpha_0x6e()
{
    uassert_int_equal(isalpha(110), 1); /*isalpha should be 1 for n */
}

void t_isalpha_0x6f()
{
    uassert_int_equal(isalpha(111), 1); /*isalpha should be 1 for o */
}

void t_isalpha_0x70()
{
    uassert_int_equal(isalpha(112), 1); /*isalpha should be 1 for p */
}

void t_isalpha_0x71()
{
    uassert_int_equal(isalpha(113), 1); /*isalpha should be 1 for q */
}

void t_isalpha_0x72()
{
    uassert_int_equal(isalpha(114), 1); /*isalpha should be 1 for r */
}

void t_isalpha_0x73()
{
    uassert_int_equal(isalpha(115), 1); /*isalpha should be 1 for s */
}

void t_isalpha_0x74()
{
    uassert_int_equal(isalpha(116), 1); /*isalpha should be 1 for t */
}

void t_isalpha_0x75()
{
    uassert_int_equal(isalpha(117), 1); /*isalpha should be 1 for u */
}

void t_isalpha_0x76()
{
    uassert_int_equal(isalpha(118), 1); /*isalpha should be 1 for v */
}

void t_isalpha_0x77()
{
    uassert_int_equal(isalpha(119), 1); /*isalpha should be 1 for w */
}

void t_isalpha_0x78()
{
    uassert_int_equal(isalpha(120), 1); /*isalpha should be 1 for x */
}

void t_isalpha_0x79()
{
    uassert_int_equal(isalpha(121), 1); /*isalpha should be 1 for y */
}

void t_isalpha_0x7a()
{
    uassert_int_equal(isalpha(122), 1); /*isalpha should be 1 for z */
}

void t_isalpha_0x7b()
{
    uassert_int_equal(isalpha(123), 0); /* isalpha should be 0 for { */
}

void t_isalpha_0x7c()
{
    uassert_int_equal(isalpha(124), 0); /* isalpha should be 0 for | */
}

void t_isalpha_0x7d()
{
    uassert_int_equal(isalpha(125), 0); /* isalpha should be 0 for } */
}

void t_isalpha_0x7e()
{
    uassert_int_equal(isalpha(126), 0); /* isalpha should be 0 for ~ */
}

void t_isalpha_0x7f()
{
    uassert_int_equal(isalpha(127), 0); /* isalpha should be 0 for 0x7f */
}

void t_isalpha_0x80()
{
    uassert_int_equal(isalpha(128), 0); /* isalpha should be 0 for 0x80 */
}

void t_isalpha_0x81()
{
    uassert_int_equal(isalpha(129), 0); /* isalpha should be 0 for 0x81 */
}

void t_isalpha_0x82()
{
    uassert_int_equal(isalpha(130), 0); /* isalpha should be 0 for 0x82 */
}

void t_isalpha_0x83()
{
    uassert_int_equal(isalpha(131), 0); /* isalpha should be 0 for 0x83 */
}

void t_isalpha_0x84()
{
    uassert_int_equal(isalpha(132), 0); /* isalpha should be 0 for 0x84 */
}

void t_isalpha_0x85()
{
    uassert_int_equal(isalpha(133), 0); /* isalpha should be 0 for 0x85 */
}

void t_isalpha_0x86()
{
    uassert_int_equal(isalpha(134), 0); /* isalpha should be 0 for 0x86 */
}

void t_isalpha_0x87()
{
    uassert_int_equal(isalpha(135), 0); /* isalpha should be 0 for 0x87 */
}

void t_isalpha_0x88()
{
    uassert_int_equal(isalpha(136), 0); /* isalpha should be 0 for 0x88 */
}

void t_isalpha_0x89()
{
    uassert_int_equal(isalpha(137), 0); /* isalpha should be 0 for 0x89 */
}

void t_isalpha_0x8a()
{
    uassert_int_equal(isalpha(138), 0); /* isalpha should be 0 for 0x8a */
}

void t_isalpha_0x8b()
{
    uassert_int_equal(isalpha(139), 0); /* isalpha should be 0 for 0x8b */
}

void t_isalpha_0x8c()
{
    uassert_int_equal(isalpha(140), 0); /* isalpha should be 0 for 0x8c */
}

void t_isalpha_0x8d()
{
    uassert_int_equal(isalpha(141), 0); /* isalpha should be 0 for 0x8d */
}

void t_isalpha_0x8e()
{
    uassert_int_equal(isalpha(142), 0); /* isalpha should be 0 for 0x8e */
}

void t_isalpha_0x8f()
{
    uassert_int_equal(isalpha(143), 0); /* isalpha should be 0 for 0x8f */
}

void t_isalpha_0x90()
{
    uassert_int_equal(isalpha(144), 0); /* isalpha should be 0 for 0x90 */
}

void t_isalpha_0x91()
{
    uassert_int_equal(isalpha(145), 0); /* isalpha should be 0 for 0x91 */
}

void t_isalpha_0x92()
{
    uassert_int_equal(isalpha(146), 0); /* isalpha should be 0 for 0x92 */
}

void t_isalpha_0x93()
{
    uassert_int_equal(isalpha(147), 0); /* isalpha should be 0 for 0x93 */
}

void t_isalpha_0x94()
{
    uassert_int_equal(isalpha(148), 0); /* isalpha should be 0 for 0x94 */
}

void t_isalpha_0x95()
{
    uassert_int_equal(isalpha(149), 0); /* isalpha should be 0 for 0x95 */
}

void t_isalpha_0x96()
{
    uassert_int_equal(isalpha(150), 0); /* isalpha should be 0 for 0x96 */
}

void t_isalpha_0x97()
{
    uassert_int_equal(isalpha(151), 0); /* isalpha should be 0 for 0x97 */
}

void t_isalpha_0x98()
{
    uassert_int_equal(isalpha(152), 0); /* isalpha should be 0 for 0x98 */
}

void t_isalpha_0x99()
{
    uassert_int_equal(isalpha(153), 0); /* isalpha should be 0 for 0x99 */
}

void t_isalpha_0x9a()
{
    uassert_int_equal(isalpha(154), 0); /* isalpha should be 0 for 0x9a */
}

void t_isalpha_0x9b()
{
    uassert_int_equal(isalpha(155), 0); /* isalpha should be 0 for 0x9b */
}

void t_isalpha_0x9c()
{
    uassert_int_equal(isalpha(156), 0); /* isalpha should be 0 for 0x9c */
}

void t_isalpha_0x9d()
{
    uassert_int_equal(isalpha(157), 0); /* isalpha should be 0 for 0x9d */
}

void t_isalpha_0x9e()
{
    uassert_int_equal(isalpha(158), 0); /* isalpha should be 0 for 0x9e */
}

void t_isalpha_0x9f()
{
    uassert_int_equal(isalpha(159), 0); /* isalpha should be 0 for 0x9f */
}

void t_isalpha_0xa0()
{
    uassert_int_equal(isalpha(160), 0); /* isalpha should be 0 for 0xa0 */
}

void t_isalpha_0xa1()
{
    uassert_int_equal(isalpha(161), 0); /* isalpha should be 0 for 0xa1 */
}

void t_isalpha_0xa2()
{
    uassert_int_equal(isalpha(162), 0); /* isalpha should be 0 for 0xa2 */
}

void t_isalpha_0xa3()
{
    uassert_int_equal(isalpha(163), 0); /* isalpha should be 0 for 0xa3 */
}

void t_isalpha_0xa4()
{
    uassert_int_equal(isalpha(164), 0); /* isalpha should be 0 for 0xa4 */
}

void t_isalpha_0xa5()
{
    uassert_int_equal(isalpha(165), 0); /* isalpha should be 0 for 0xa5 */
}

void t_isalpha_0xa6()
{
    uassert_int_equal(isalpha(166), 0); /* isalpha should be 0 for 0xa6 */
}

void t_isalpha_0xa7()
{
    uassert_int_equal(isalpha(167), 0); /* isalpha should be 0 for 0xa7 */
}

void t_isalpha_0xa8()
{
    uassert_int_equal(isalpha(168), 0); /* isalpha should be 0 for 0xa8 */
}

void t_isalpha_0xa9()
{
    uassert_int_equal(isalpha(169), 0); /* isalpha should be 0 for 0xa9 */
}

void t_isalpha_0xaa()
{
    uassert_int_equal(isalpha(170), 0); /* isalpha should be 0 for 0xaa */
}

void t_isalpha_0xab()
{
    uassert_int_equal(isalpha(171), 0); /* isalpha should be 0 for 0xab */
}

void t_isalpha_0xac()
{
    uassert_int_equal(isalpha(172), 0); /* isalpha should be 0 for 0xac */
}

void t_isalpha_0xad()
{
    uassert_int_equal(isalpha(173), 0); /* isalpha should be 0 for 0xad */
}

void t_isalpha_0xae()
{
    uassert_int_equal(isalpha(174), 0); /* isalpha should be 0 for 0xae */
}

void t_isalpha_0xaf()
{
    uassert_int_equal(isalpha(175), 0); /* isalpha should be 0 for 0xaf */
}

void t_isalpha_0xb0()
{
    uassert_int_equal(isalpha(176), 0); /* isalpha should be 0 for 0xb0 */
}

void t_isalpha_0xb1()
{
    uassert_int_equal(isalpha(177), 0); /* isalpha should be 0 for 0xb1 */
}

void t_isalpha_0xb2()
{
    uassert_int_equal(isalpha(178), 0); /* isalpha should be 0 for 0xb2 */
}

void t_isalpha_0xb3()
{
    uassert_int_equal(isalpha(179), 0); /* isalpha should be 0 for 0xb3 */
}

void t_isalpha_0xb4()
{
    uassert_int_equal(isalpha(180), 0); /* isalpha should be 0 for 0xb4 */
}

void t_isalpha_0xb5()
{
    uassert_int_equal(isalpha(181), 0); /* isalpha should be 0 for 0xb5 */
}

void t_isalpha_0xb6()
{
    uassert_int_equal(isalpha(182), 0); /* isalpha should be 0 for 0xb6 */
}

void t_isalpha_0xb7()
{
    uassert_int_equal(isalpha(183), 0); /* isalpha should be 0 for 0xb7 */
}

void t_isalpha_0xb8()
{
    uassert_int_equal(isalpha(184), 0); /* isalpha should be 0 for 0xb8 */
}

void t_isalpha_0xb9()
{
    uassert_int_equal(isalpha(185), 0); /* isalpha should be 0 for 0xb9 */
}

void t_isalpha_0xba()
{
    uassert_int_equal(isalpha(186), 0); /* isalpha should be 0 for 0xba */
}

void t_isalpha_0xbb()
{
    uassert_int_equal(isalpha(187), 0); /* isalpha should be 0 for 0xbb */
}

void t_isalpha_0xbc()
{
    uassert_int_equal(isalpha(188), 0); /* isalpha should be 0 for 0xbc */
}

void t_isalpha_0xbd()
{
    uassert_int_equal(isalpha(189), 0); /* isalpha should be 0 for 0xbd */
}

void t_isalpha_0xbe()
{
    uassert_int_equal(isalpha(190), 0); /* isalpha should be 0 for 0xbe */
}

void t_isalpha_0xbf()
{
    uassert_int_equal(isalpha(191), 0); /* isalpha should be 0 for 0xbf */
}

void t_isalpha_0xc0()
{
    uassert_int_equal(isalpha(192), 0); /* isalpha should be 0 for 0xc0 */
}

void t_isalpha_0xc1()
{
    uassert_int_equal(isalpha(193), 0); /* isalpha should be 0 for 0xc1 */
}

void t_isalpha_0xc2()
{
    uassert_int_equal(isalpha(194), 0); /* isalpha should be 0 for 0xc2 */
}

void t_isalpha_0xc3()
{
    uassert_int_equal(isalpha(195), 0); /* isalpha should be 0 for 0xc3 */
}

void t_isalpha_0xc4()
{
    uassert_int_equal(isalpha(196), 0); /* isalpha should be 0 for 0xc4 */
}

void t_isalpha_0xc5()
{
    uassert_int_equal(isalpha(197), 0); /* isalpha should be 0 for 0xc5 */
}

void t_isalpha_0xc6()
{
    uassert_int_equal(isalpha(198), 0); /* isalpha should be 0 for 0xc6 */
}

void t_isalpha_0xc7()
{
    uassert_int_equal(isalpha(199), 0); /* isalpha should be 0 for 0xc7 */
}

void t_isalpha_0xc8()
{
    uassert_int_equal(isalpha(200), 0); /* isalpha should be 0 for 0xc8 */
}

void t_isalpha_0xc9()
{
    uassert_int_equal(isalpha(201), 0); /* isalpha should be 0 for 0xc9 */
}

void t_isalpha_0xca()
{
    uassert_int_equal(isalpha(202), 0); /* isalpha should be 0 for 0xca */
}

void t_isalpha_0xcb()
{
    uassert_int_equal(isalpha(203), 0); /* isalpha should be 0 for 0xcb */
}

void t_isalpha_0xcc()
{
    uassert_int_equal(isalpha(204), 0); /* isalpha should be 0 for 0xcc */
}

void t_isalpha_0xcd()
{
    uassert_int_equal(isalpha(205), 0); /* isalpha should be 0 for 0xcd */
}

void t_isalpha_0xce()
{
    uassert_int_equal(isalpha(206), 0); /* isalpha should be 0 for 0xce */
}

void t_isalpha_0xcf()
{
    uassert_int_equal(isalpha(207), 0); /* isalpha should be 0 for 0xcf */
}

void t_isalpha_0xd0()
{
    uassert_int_equal(isalpha(208), 0); /* isalpha should be 0 for 0xd0 */
}

void t_isalpha_0xd1()
{
    uassert_int_equal(isalpha(209), 0); /* isalpha should be 0 for 0xd1 */
}

void t_isalpha_0xd2()
{
    uassert_int_equal(isalpha(210), 0); /* isalpha should be 0 for 0xd2 */
}

void t_isalpha_0xd3()
{
    uassert_int_equal(isalpha(211), 0); /* isalpha should be 0 for 0xd3 */
}

void t_isalpha_0xd4()
{
    uassert_int_equal(isalpha(212), 0); /* isalpha should be 0 for 0xd4 */
}

void t_isalpha_0xd5()
{
    uassert_int_equal(isalpha(213), 0); /* isalpha should be 0 for 0xd5 */
}

void t_isalpha_0xd6()
{
    uassert_int_equal(isalpha(214), 0); /* isalpha should be 0 for 0xd6 */
}

void t_isalpha_0xd7()
{
    uassert_int_equal(isalpha(215), 0); /* isalpha should be 0 for 0xd7 */
}

void t_isalpha_0xd8()
{
    uassert_int_equal(isalpha(216), 0); /* isalpha should be 0 for 0xd8 */
}

void t_isalpha_0xd9()
{
    uassert_int_equal(isalpha(217), 0); /* isalpha should be 0 for 0xd9 */
}

void t_isalpha_0xda()
{
    uassert_int_equal(isalpha(218), 0); /* isalpha should be 0 for 0xda */
}

void t_isalpha_0xdb()
{
    uassert_int_equal(isalpha(219), 0); /* isalpha should be 0 for 0xdb */
}

void t_isalpha_0xdc()
{
    uassert_int_equal(isalpha(220), 0); /* isalpha should be 0 for 0xdc */
}

void t_isalpha_0xdd()
{
    uassert_int_equal(isalpha(221), 0); /* isalpha should be 0 for 0xdd */
}

void t_isalpha_0xde()
{
    uassert_int_equal(isalpha(222), 0); /* isalpha should be 0 for 0xde */
}

void t_isalpha_0xdf()
{
    uassert_int_equal(isalpha(223), 0); /* isalpha should be 0 for 0xdf */
}

void t_isalpha_0xe0()
{
    uassert_int_equal(isalpha(224), 0); /* isalpha should be 0 for 0xe0 */
}

void t_isalpha_0xe1()
{
    uassert_int_equal(isalpha(225), 0); /* isalpha should be 0 for 0xe1 */
}

void t_isalpha_0xe2()
{
    uassert_int_equal(isalpha(226), 0); /* isalpha should be 0 for 0xe2 */
}

void t_isalpha_0xe3()
{
    uassert_int_equal(isalpha(227), 0); /* isalpha should be 0 for 0xe3 */
}

void t_isalpha_0xe4()
{
    uassert_int_equal(isalpha(228), 0); /* isalpha should be 0 for 0xe4 */
}

void t_isalpha_0xe5()
{
    uassert_int_equal(isalpha(229), 0); /* isalpha should be 0 for 0xe5 */
}

void t_isalpha_0xe6()
{
    uassert_int_equal(isalpha(230), 0); /* isalpha should be 0 for 0xe6 */
}

void t_isalpha_0xe7()
{
    uassert_int_equal(isalpha(231), 0); /* isalpha should be 0 for 0xe7 */
}

void t_isalpha_0xe8()
{
    uassert_int_equal(isalpha(232), 0); /* isalpha should be 0 for 0xe8 */
}

void t_isalpha_0xe9()
{
    uassert_int_equal(isalpha(233), 0); /* isalpha should be 0 for 0xe9 */
}

void t_isalpha_0xea()
{
    uassert_int_equal(isalpha(234), 0); /* isalpha should be 0 for 0xea */
}

void t_isalpha_0xeb()
{
    uassert_int_equal(isalpha(235), 0); /* isalpha should be 0 for 0xeb */
}

void t_isalpha_0xec()
{
    uassert_int_equal(isalpha(236), 0); /* isalpha should be 0 for 0xec */
}

void t_isalpha_0xed()
{
    uassert_int_equal(isalpha(237), 0); /* isalpha should be 0 for 0xed */
}

void t_isalpha_0xee()
{
    uassert_int_equal(isalpha(238), 0); /* isalpha should be 0 for 0xee */
}

void t_isalpha_0xef()
{
    uassert_int_equal(isalpha(239), 0); /* isalpha should be 0 for 0xef */
}

void t_isalpha_0xf0()
{
    uassert_int_equal(isalpha(240), 0); /* isalpha should be 0 for 0xf0 */
}

void t_isalpha_0xf1()
{
    uassert_int_equal(isalpha(241), 0); /* isalpha should be 0 for 0xf1 */
}

void t_isalpha_0xf2()
{
    uassert_int_equal(isalpha(242), 0); /* isalpha should be 0 for 0xf2 */
}

void t_isalpha_0xf3()
{
    uassert_int_equal(isalpha(243), 0); /* isalpha should be 0 for 0xf3 */
}

void t_isalpha_0xf4()
{
    uassert_int_equal(isalpha(244), 0); /* isalpha should be 0 for 0xf4 */
}

void t_isalpha_0xf5()
{
    uassert_int_equal(isalpha(245), 0); /* isalpha should be 0 for 0xf5 */
}

void t_isalpha_0xf6()
{
    uassert_int_equal(isalpha(246), 0); /* isalpha should be 0 for 0xf6 */
}

void t_isalpha_0xf7()
{
    uassert_int_equal(isalpha(247), 0); /* isalpha should be 0 for 0xf7 */
}

void t_isalpha_0xf8()
{
    uassert_int_equal(isalpha(248), 0); /* isalpha should be 0 for 0xf8 */
}

void t_isalpha_0xf9()
{
    uassert_int_equal(isalpha(249), 0); /* isalpha should be 0 for 0xf9 */
}

void t_isalpha_0xfa()
{
    uassert_int_equal(isalpha(250), 0); /* isalpha should be 0 for 0xfa */
}

void t_isalpha_0xfb()
{
    uassert_int_equal(isalpha(251), 0); /* isalpha should be 0 for 0xfb */
}

void t_isalpha_0xfc()
{
    uassert_int_equal(isalpha(252), 0); /* isalpha should be 0 for 0xfc */
}

void t_isalpha_0xfd()
{
    uassert_int_equal(isalpha(253), 0); /* isalpha should be 0 for 0xfd */
}

void t_isalpha_0xfe()
{
    uassert_int_equal(isalpha(254), 0); /* isalpha should be 0 for 0xfe */
}

void t_isalpha_0xff()
{
    uassert_int_equal(isalpha(255), 0); /* isalpha should be 0 for 0xff */
}

static int testcase(void)
{
    t_isalpha_0x00();
    t_isalpha_0x01();
    t_isalpha_0x02();
    t_isalpha_0x03();
    t_isalpha_0x04();
    t_isalpha_0x05();
    t_isalpha_0x06();
    t_isalpha_0x07();
    t_isalpha_0x08();
    t_isalpha_0x09();
    t_isalpha_0x0a();
    t_isalpha_0x0b();
    t_isalpha_0x0c();
    t_isalpha_0x0d();
    t_isalpha_0x0e();
    t_isalpha_0x0f();
    t_isalpha_0x10();
    t_isalpha_0x11();
    t_isalpha_0x12();
    t_isalpha_0x13();
    t_isalpha_0x14();
    t_isalpha_0x15();
    t_isalpha_0x16();
    t_isalpha_0x17();
    t_isalpha_0x18();
    t_isalpha_0x19();
    t_isalpha_0x1a();
    t_isalpha_0x1b();
    t_isalpha_0x1c();
    t_isalpha_0x1d();
    t_isalpha_0x1e();
    t_isalpha_0x1f();
    t_isalpha_0x20();
    t_isalpha_0x21();
    t_isalpha_0x22();
    t_isalpha_0x23();
    t_isalpha_0x24();
    t_isalpha_0x25();
    t_isalpha_0x26();
    t_isalpha_0x27();
    t_isalpha_0x28();
    t_isalpha_0x29();
    t_isalpha_0x2a();
    t_isalpha_0x2b();
    t_isalpha_0x2c();
    t_isalpha_0x2d();
    t_isalpha_0x2e();
    t_isalpha_0x2f();
    t_isalpha_0x30();
    t_isalpha_0x31();
    t_isalpha_0x32();
    t_isalpha_0x33();
    t_isalpha_0x34();
    t_isalpha_0x35();
    t_isalpha_0x36();
    t_isalpha_0x37();
    t_isalpha_0x38();
    t_isalpha_0x39();
    t_isalpha_0x3a();
    t_isalpha_0x3b();
    t_isalpha_0x3c();
    t_isalpha_0x3d();
    t_isalpha_0x3e();
    t_isalpha_0x3f();
    t_isalpha_0x40();
    t_isalpha_0x41();
    t_isalpha_0x42();
    t_isalpha_0x43();
    t_isalpha_0x44();
    t_isalpha_0x45();
    t_isalpha_0x46();
    t_isalpha_0x47();
    t_isalpha_0x48();
    t_isalpha_0x49();
    t_isalpha_0x4a();
    t_isalpha_0x4b();
    t_isalpha_0x4c();
    t_isalpha_0x4d();
    t_isalpha_0x4e();
    t_isalpha_0x4f();
    t_isalpha_0x50();
    t_isalpha_0x51();
    t_isalpha_0x52();
    t_isalpha_0x53();
    t_isalpha_0x54();
    t_isalpha_0x55();
    t_isalpha_0x56();
    t_isalpha_0x57();
    t_isalpha_0x58();
    t_isalpha_0x59();
    t_isalpha_0x5a();
    t_isalpha_0x5b();
    t_isalpha_0x5c();
    t_isalpha_0x5d();
    t_isalpha_0x5e();
    t_isalpha_0x5f();
    t_isalpha_0x60();
    t_isalpha_0x61();
    t_isalpha_0x62();
    t_isalpha_0x63();
    t_isalpha_0x64();
    t_isalpha_0x65();
    t_isalpha_0x66();
    t_isalpha_0x67();
    t_isalpha_0x68();
    t_isalpha_0x69();
    t_isalpha_0x6a();
    t_isalpha_0x6b();
    t_isalpha_0x6c();
    t_isalpha_0x6d();
    t_isalpha_0x6e();
    t_isalpha_0x6f();
    t_isalpha_0x70();
    t_isalpha_0x71();
    t_isalpha_0x72();
    t_isalpha_0x73();
    t_isalpha_0x74();
    t_isalpha_0x75();
    t_isalpha_0x76();
    t_isalpha_0x77();
    t_isalpha_0x78();
    t_isalpha_0x79();
    t_isalpha_0x7a();
    t_isalpha_0x7b();
    t_isalpha_0x7c();
    t_isalpha_0x7d();
    t_isalpha_0x7e();
    t_isalpha_0x7f();
    t_isalpha_0x80();
    t_isalpha_0x81();
    t_isalpha_0x82();
    t_isalpha_0x83();
    t_isalpha_0x84();
    t_isalpha_0x85();
    t_isalpha_0x86();
    t_isalpha_0x87();
    t_isalpha_0x88();
    t_isalpha_0x89();
    t_isalpha_0x8a();
    t_isalpha_0x8b();
    t_isalpha_0x8c();
    t_isalpha_0x8d();
    t_isalpha_0x8e();
    t_isalpha_0x8f();
    t_isalpha_0x90();
    t_isalpha_0x91();
    t_isalpha_0x92();
    t_isalpha_0x93();
    t_isalpha_0x94();
    t_isalpha_0x95();
    t_isalpha_0x96();
    t_isalpha_0x97();
    t_isalpha_0x98();
    t_isalpha_0x99();
    t_isalpha_0x9a();
    t_isalpha_0x9b();
    t_isalpha_0x9c();
    t_isalpha_0x9d();
    t_isalpha_0x9e();
    t_isalpha_0x9f();
    t_isalpha_0xa0();
    t_isalpha_0xa1();
    t_isalpha_0xa2();
    t_isalpha_0xa3();
    t_isalpha_0xa4();
    t_isalpha_0xa5();
    t_isalpha_0xa6();
    t_isalpha_0xa7();
    t_isalpha_0xa8();
    t_isalpha_0xa9();
    t_isalpha_0xaa();
    t_isalpha_0xab();
    t_isalpha_0xac();
    t_isalpha_0xad();
    t_isalpha_0xae();
    t_isalpha_0xaf();
    t_isalpha_0xb0();
    t_isalpha_0xb1();
    t_isalpha_0xb2();
    t_isalpha_0xb3();
    t_isalpha_0xb4();
    t_isalpha_0xb5();
    t_isalpha_0xb6();
    t_isalpha_0xb7();
    t_isalpha_0xb8();
    t_isalpha_0xb9();
    t_isalpha_0xba();
    t_isalpha_0xbb();
    t_isalpha_0xbc();
    t_isalpha_0xbd();
    t_isalpha_0xbe();
    t_isalpha_0xbf();
    t_isalpha_0xc0();
    t_isalpha_0xc1();
    t_isalpha_0xc2();
    t_isalpha_0xc3();
    t_isalpha_0xc4();
    t_isalpha_0xc5();
    t_isalpha_0xc6();
    t_isalpha_0xc7();
    t_isalpha_0xc8();
    t_isalpha_0xc9();
    t_isalpha_0xca();
    t_isalpha_0xcb();
    t_isalpha_0xcc();
    t_isalpha_0xcd();
    t_isalpha_0xce();
    t_isalpha_0xcf();
    t_isalpha_0xd0();
    t_isalpha_0xd1();
    t_isalpha_0xd2();
    t_isalpha_0xd3();
    t_isalpha_0xd4();
    t_isalpha_0xd5();
    t_isalpha_0xd6();
    t_isalpha_0xd7();
    t_isalpha_0xd8();
    t_isalpha_0xd9();
    t_isalpha_0xda();
    t_isalpha_0xdb();
    t_isalpha_0xdc();
    t_isalpha_0xdd();
    t_isalpha_0xde();
    t_isalpha_0xdf();
    t_isalpha_0xe0();
    t_isalpha_0xe1();
    t_isalpha_0xe2();
    t_isalpha_0xe3();
    t_isalpha_0xe4();
    t_isalpha_0xe5();
    t_isalpha_0xe6();
    t_isalpha_0xe7();
    t_isalpha_0xe8();
    t_isalpha_0xe9();
    t_isalpha_0xea();
    t_isalpha_0xeb();
    t_isalpha_0xec();
    t_isalpha_0xed();
    t_isalpha_0xee();
    t_isalpha_0xef();
    t_isalpha_0xf0();
    t_isalpha_0xf1();
    t_isalpha_0xf2();
    t_isalpha_0xf3();
    t_isalpha_0xf4();
    t_isalpha_0xf5();
    t_isalpha_0xf6();
    t_isalpha_0xf7();
    t_isalpha_0xf8();
    t_isalpha_0xf9();
    t_isalpha_0xfa();
    t_isalpha_0xfb();
    t_isalpha_0xfc();
    t_isalpha_0xfd();
    t_isalpha_0xfe();
    t_isalpha_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
