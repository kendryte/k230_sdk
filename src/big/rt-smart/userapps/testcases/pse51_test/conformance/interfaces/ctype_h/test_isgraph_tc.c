#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))

void t_isgraph_0x00()
{
    uassert_int_equal(isgraph(0), 0); /* isgraph should be 0 for 0x00 */
}

void t_isgraph_0x01()
{
    uassert_int_equal(isgraph(1), 0); /* isgraph should be 0 for 0x01 */
}

void t_isgraph_0x02()
{
    uassert_int_equal(isgraph(2), 0); /* isgraph should be 0 for 0x02 */
}

void t_isgraph_0x03()
{
    uassert_int_equal(isgraph(3), 0); /* isgraph should be 0 for 0x03 */
}

void t_isgraph_0x04()
{
    uassert_int_equal(isgraph(4), 0); /* isgraph should be 0 for 0x04 */
}

void t_isgraph_0x05()
{
    uassert_int_equal(isgraph(5), 0); /* isgraph should be 0 for 0x05 */
}

void t_isgraph_0x06()
{
    uassert_int_equal(isgraph(6), 0); /* isgraph should be 0 for 0x06 */
}

void t_isgraph_0x07()
{
    uassert_int_equal(isgraph(7), 0); /* isgraph should be 0 for 0x07 */
}

void t_isgraph_0x08()
{
    uassert_int_equal(isgraph(8), 0); /* isgraph should be 0 for 0x08 */
}

void t_isgraph_0x09()
{
    uassert_int_equal(isgraph(9), 0); /* isgraph should be 0 for 0x09 */
}

void t_isgraph_0x0a()
{
    uassert_int_equal(isgraph(10), 0); /* isgraph should be 0 for 0x0a */
}

void t_isgraph_0x0b()
{
    uassert_int_equal(isgraph(11), 0); /* isgraph should be 0 for 0x0b */
}

void t_isgraph_0x0c()
{
    uassert_int_equal(isgraph(12), 0); /* isgraph should be 0 for 0x0c */
}

void t_isgraph_0x0d()
{
    uassert_int_equal(isgraph(13), 0); /* isgraph should be 0 for 0x0d */
}

void t_isgraph_0x0e()
{
    uassert_int_equal(isgraph(14), 0); /* isgraph should be 0 for 0x0e */
}

void t_isgraph_0x0f()
{
    uassert_int_equal(isgraph(15), 0); /* isgraph should be 0 for 0x0f */
}

void t_isgraph_0x10()
{
    uassert_int_equal(isgraph(16), 0); /* isgraph should be 0 for 0x10 */
}

void t_isgraph_0x11()
{
    uassert_int_equal(isgraph(17), 0); /* isgraph should be 0 for 0x11 */
}

void t_isgraph_0x12()
{
    uassert_int_equal(isgraph(18), 0); /* isgraph should be 0 for 0x12 */
}

void t_isgraph_0x13()
{
    uassert_int_equal(isgraph(19), 0); /* isgraph should be 0 for 0x13 */
}

void t_isgraph_0x14()
{
    uassert_int_equal(isgraph(20), 0); /* isgraph should be 0 for 0x14 */
}

void t_isgraph_0x15()
{
    uassert_int_equal(isgraph(21), 0); /* isgraph should be 0 for 0x15 */
}

void t_isgraph_0x16()
{
    uassert_int_equal(isgraph(22), 0); /* isgraph should be 0 for 0x16 */
}

void t_isgraph_0x17()
{
    uassert_int_equal(isgraph(23), 0); /* isgraph should be 0 for 0x17 */
}

void t_isgraph_0x18()
{
    uassert_int_equal(isgraph(24), 0); /* isgraph should be 0 for 0x18 */
}

void t_isgraph_0x19()
{
    uassert_int_equal(isgraph(25), 0); /* isgraph should be 0 for 0x19 */
}

void t_isgraph_0x1a()
{
    uassert_int_equal(isgraph(26), 0); /* isgraph should be 0 for 0x1a */
}

void t_isgraph_0x1b()
{
    uassert_int_equal(isgraph(27), 0); /* isgraph should be 0 for 0x1b */
}

void t_isgraph_0x1c()
{
    uassert_int_equal(isgraph(28), 0); /* isgraph should be 0 for 0x1c */
}

void t_isgraph_0x1d()
{
    uassert_int_equal(isgraph(29), 0); /* isgraph should be 0 for 0x1d */
}

void t_isgraph_0x1e()
{
    uassert_int_equal(isgraph(30), 0); /* isgraph should be 0 for 0x1e */
}

void t_isgraph_0x1f()
{
    uassert_int_equal(isgraph(31), 0); /* isgraph should be 0 for 0x1f */
}

void t_isgraph_0x20()
{
    uassert_int_equal(isgraph(32), 0); /* isgraph should be 0 for   */
}

void t_isgraph_0x21()
{
    uassert_int_equal(isgraph(33), 1); /* isgraph should be 1 for ! */
}

void t_isgraph_0x22()
{
    uassert_int_equal(isgraph(34), 1); /* isgraph should be 1 for 0x22 */
}

void t_isgraph_0x23()
{
    uassert_int_equal(isgraph(35), 1); /* isgraph should be 1 for # */
}

void t_isgraph_0x24()
{
    uassert_int_equal(isgraph(36), 1); /* isgraph should be 1 for $ */
}

void t_isgraph_0x25()
{
    uassert_int_equal(isgraph(37), 1); /* isgraph should be 1 for % */
}

void t_isgraph_0x26()
{
    uassert_int_equal(isgraph(38), 1); /* isgraph should be 1 for & */
}

void t_isgraph_0x27()
{
    uassert_int_equal(isgraph(39), 1); /* isgraph should be 1 for ' */
}

void t_isgraph_0x28()
{
    uassert_int_equal(isgraph(40), 1); /* isgraph should be 1 for ( */
}

void t_isgraph_0x29()
{
    uassert_int_equal(isgraph(41), 1); /* isgraph should be 1 for ) */
}

void t_isgraph_0x2a()
{
    uassert_int_equal(isgraph(42), 1); /* isgraph should be 1 for * */
}

void t_isgraph_0x2b()
{
    uassert_int_equal(isgraph(43), 1); /* isgraph should be 1 for + */
}

void t_isgraph_0x2c()
{
    uassert_int_equal(isgraph(44), 1); /* isgraph should be 1 for , */
}

void t_isgraph_0x2d()
{
    uassert_int_equal(isgraph(45), 1); /* isgraph should be 1 for - */
}

void t_isgraph_0x2e()
{
    uassert_int_equal(isgraph(46), 1); /* isgraph should be 1 for . */
}

void t_isgraph_0x2f()
{
    uassert_int_equal(isgraph(47), 1); /* isgraph should be 1 for / */
}

void t_isgraph_0x30()
{
    uassert_int_equal(isgraph(48), 1); /* isgraph should be 1 for 0 */
}

void t_isgraph_0x31()
{
    uassert_int_equal(isgraph(49), 1); /* isgraph should be 1 for 1 */
}

void t_isgraph_0x32()
{
    uassert_int_equal(isgraph(50), 1); /* isgraph should be 1 for 2 */
}

void t_isgraph_0x33()
{
    uassert_int_equal(isgraph(51), 1); /* isgraph should be 1 for 3 */
}

void t_isgraph_0x34()
{
    uassert_int_equal(isgraph(52), 1); /* isgraph should be 1 for 4 */
}

void t_isgraph_0x35()
{
    uassert_int_equal(isgraph(53), 1); /* isgraph should be 1 for 5 */
}

void t_isgraph_0x36()
{
    uassert_int_equal(isgraph(54), 1); /* isgraph should be 1 for 6 */
}

void t_isgraph_0x37()
{
    uassert_int_equal(isgraph(55), 1); /* isgraph should be 1 for 7 */
}

void t_isgraph_0x38()
{
    uassert_int_equal(isgraph(56), 1); /* isgraph should be 1 for 8 */
}

void t_isgraph_0x39()
{
    uassert_int_equal(isgraph(57), 1); /* isgraph should be 1 for 9 */
}

void t_isgraph_0x3a()
{
    uassert_int_equal(isgraph(58), 1); /* isgraph should be 1 for : */
}

void t_isgraph_0x3b()
{
    uassert_int_equal(isgraph(59), 1); /* isgraph should be 1 for ; */
}

void t_isgraph_0x3c()
{
    uassert_int_equal(isgraph(60), 1); /* isgraph should be 1 for < */
}

void t_isgraph_0x3d()
{
    uassert_int_equal(isgraph(61), 1); /* isgraph should be 1 for = */
}

void t_isgraph_0x3e()
{
    uassert_int_equal(isgraph(62), 1); /* isgraph should be 1 for > */
}

void t_isgraph_0x3f()
{
    uassert_int_equal(isgraph(63), 1); /* isgraph should be 1 for ? */
}

void t_isgraph_0x40()
{
    uassert_int_equal(isgraph(64), 1); /* isgraph should be 1 for @ */
}

void t_isgraph_0x41()
{
    uassert_int_equal(isgraph(65), 1); /* isgraph should be 1 for A */
}

void t_isgraph_0x42()
{
    uassert_int_equal(isgraph(66), 1); /* isgraph should be 1 for B */
}

void t_isgraph_0x43()
{
    uassert_int_equal(isgraph(67), 1); /* isgraph should be 1 for C */
}

void t_isgraph_0x44()
{
    uassert_int_equal(isgraph(68), 1); /* isgraph should be 1 for D */
}

void t_isgraph_0x45()
{
    uassert_int_equal(isgraph(69), 1); /* isgraph should be 1 for E */
}

void t_isgraph_0x46()
{
    uassert_int_equal(isgraph(70), 1); /* isgraph should be 1 for F */
}

void t_isgraph_0x47()
{
    uassert_int_equal(isgraph(71), 1); /* isgraph should be 1 for G */
}

void t_isgraph_0x48()
{
    uassert_int_equal(isgraph(72), 1); /* isgraph should be 1 for H */
}

void t_isgraph_0x49()
{
    uassert_int_equal(isgraph(73), 1); /* isgraph should be 1 for I */
}

void t_isgraph_0x4a()
{
    uassert_int_equal(isgraph(74), 1); /* isgraph should be 1 for J */
}

void t_isgraph_0x4b()
{
    uassert_int_equal(isgraph(75), 1); /* isgraph should be 1 for K */
}

void t_isgraph_0x4c()
{
    uassert_int_equal(isgraph(76), 1); /* isgraph should be 1 for L */
}

void t_isgraph_0x4d()
{
    uassert_int_equal(isgraph(77), 1); /* isgraph should be 1 for M */
}

void t_isgraph_0x4e()
{
    uassert_int_equal(isgraph(78), 1); /* isgraph should be 1 for N */
}

void t_isgraph_0x4f()
{
    uassert_int_equal(isgraph(79), 1); /* isgraph should be 1 for O */
}

void t_isgraph_0x50()
{
    uassert_int_equal(isgraph(80), 1); /* isgraph should be 1 for P */
}

void t_isgraph_0x51()
{
    uassert_int_equal(isgraph(81), 1); /* isgraph should be 1 for Q */
}

void t_isgraph_0x52()
{
    uassert_int_equal(isgraph(82), 1); /* isgraph should be 1 for R */
}

void t_isgraph_0x53()
{
    uassert_int_equal(isgraph(83), 1); /* isgraph should be 1 for S */
}

void t_isgraph_0x54()
{
    uassert_int_equal(isgraph(84), 1); /* isgraph should be 1 for T */
}

void t_isgraph_0x55()
{
    uassert_int_equal(isgraph(85), 1); /* isgraph should be 1 for U */
}

void t_isgraph_0x56()
{
    uassert_int_equal(isgraph(86), 1); /* isgraph should be 1 for V */
}

void t_isgraph_0x57()
{
    uassert_int_equal(isgraph(87), 1); /* isgraph should be 1 for W */
}

void t_isgraph_0x58()
{
    uassert_int_equal(isgraph(88), 1); /* isgraph should be 1 for X */
}

void t_isgraph_0x59()
{
    uassert_int_equal(isgraph(89), 1); /* isgraph should be 1 for Y */
}

void t_isgraph_0x5a()
{
    uassert_int_equal(isgraph(90), 1); /* isgraph should be 1 for Z */
}

void t_isgraph_0x5b()
{
    uassert_int_equal(isgraph(91), 1); /* isgraph should be 1 for [ */
}

void t_isgraph_0x5c()
{
    uassert_int_equal(isgraph(92), 1); /* isgraph should be 1 for 0x5c */
}

void t_isgraph_0x5d()
{
    uassert_int_equal(isgraph(93), 1); /* isgraph should be 1 for ] */
}

void t_isgraph_0x5e()
{
    uassert_int_equal(isgraph(94), 1); /* isgraph should be 1 for ^ */
}

void t_isgraph_0x5f()
{
    uassert_int_equal(isgraph(95), 1); /* isgraph should be 1 for _ */
}

void t_isgraph_0x60()
{
    uassert_int_equal(isgraph(96), 1); /* isgraph should be 1 for ` */
}

void t_isgraph_0x61()
{
    uassert_int_equal(isgraph(97), 1); /* isgraph should be 1 for a */
}

void t_isgraph_0x62()
{
    uassert_int_equal(isgraph(98), 1); /* isgraph should be 1 for b */
}

void t_isgraph_0x63()
{
    uassert_int_equal(isgraph(99), 1); /* isgraph should be 1 for c */
}

void t_isgraph_0x64()
{
    uassert_int_equal(isgraph(100), 1); /* isgraph should be 1 for d */
}

void t_isgraph_0x65()
{
    uassert_int_equal(isgraph(101), 1); /* isgraph should be 1 for e */
}

void t_isgraph_0x66()
{
    uassert_int_equal(isgraph(102), 1); /* isgraph should be 1 for f */
}

void t_isgraph_0x67()
{
    uassert_int_equal(isgraph(103), 1); /* isgraph should be 1 for g */
}

void t_isgraph_0x68()
{
    uassert_int_equal(isgraph(104), 1); /* isgraph should be 1 for h */
}

void t_isgraph_0x69()
{
    uassert_int_equal(isgraph(105), 1); /* isgraph should be 1 for i */
}

void t_isgraph_0x6a()
{
    uassert_int_equal(isgraph(106), 1); /* isgraph should be 1 for j */
}

void t_isgraph_0x6b()
{
    uassert_int_equal(isgraph(107), 1); /* isgraph should be 1 for k */
}

void t_isgraph_0x6c()
{
    uassert_int_equal(isgraph(108), 1); /* isgraph should be 1 for l */
}

void t_isgraph_0x6d()
{
    uassert_int_equal(isgraph(109), 1); /* isgraph should be 1 for m */
}

void t_isgraph_0x6e()
{
    uassert_int_equal(isgraph(110), 1); /* isgraph should be 1 for n */
}

void t_isgraph_0x6f()
{
    uassert_int_equal(isgraph(111), 1); /* isgraph should be 1 for o */
}

void t_isgraph_0x70()
{
    uassert_int_equal(isgraph(112), 1); /* isgraph should be 1 for p */
}

void t_isgraph_0x71()
{
    uassert_int_equal(isgraph(113), 1); /* isgraph should be 1 for q */
}

void t_isgraph_0x72()
{
    uassert_int_equal(isgraph(114), 1); /* isgraph should be 1 for r */
}

void t_isgraph_0x73()
{
    uassert_int_equal(isgraph(115), 1); /* isgraph should be 1 for s */
}

void t_isgraph_0x74()
{
    uassert_int_equal(isgraph(116), 1); /* isgraph should be 1 for t */
}

void t_isgraph_0x75()
{
    uassert_int_equal(isgraph(117), 1); /* isgraph should be 1 for u */
}

void t_isgraph_0x76()
{
    uassert_int_equal(isgraph(118), 1); /* isgraph should be 1 for v */
}

void t_isgraph_0x77()
{
    uassert_int_equal(isgraph(119), 1); /* isgraph should be 1 for w */
}

void t_isgraph_0x78()
{
    uassert_int_equal(isgraph(120), 1); /* isgraph should be 1 for x */
}

void t_isgraph_0x79()
{
    uassert_int_equal(isgraph(121), 1); /* isgraph should be 1 for y */
}

void t_isgraph_0x7a()
{
    uassert_int_equal(isgraph(122), 1); /* isgraph should be 1 for z */
}

void t_isgraph_0x7b()
{
    uassert_int_equal(isgraph(123), 1); /* isgraph should be 1 for { */
}

void t_isgraph_0x7c()
{
    uassert_int_equal(isgraph(124), 1); /* isgraph should be 1 for | */
}

void t_isgraph_0x7d()
{
    uassert_int_equal(isgraph(125), 1); /* isgraph should be 1 for } */
}

void t_isgraph_0x7e()
{
    uassert_int_equal(isgraph(126), 1); /* isgraph should be 1 for ~ */
}

void t_isgraph_0x7f()
{
    uassert_int_equal(isgraph(127), 0); /* isgraph should be 0 for 0x7f */
}

void t_isgraph_0x80()
{
    uassert_int_equal(isgraph(128), 0); /* isgraph should be 0 for 0x80 */
}

void t_isgraph_0x81()
{
    uassert_int_equal(isgraph(129), 0); /* isgraph should be 0 for 0x81 */
}

void t_isgraph_0x82()
{
    uassert_int_equal(isgraph(130), 0); /* isgraph should be 0 for 0x82 */
}

void t_isgraph_0x83()
{
    uassert_int_equal(isgraph(131), 0); /* isgraph should be 0 for 0x83 */
}

void t_isgraph_0x84()
{
    uassert_int_equal(isgraph(132), 0); /* isgraph should be 0 for 0x84 */
}

void t_isgraph_0x85()
{
    uassert_int_equal(isgraph(133), 0); /* isgraph should be 0 for 0x85 */
}

void t_isgraph_0x86()
{
    uassert_int_equal(isgraph(134), 0); /* isgraph should be 0 for 0x86 */
}

void t_isgraph_0x87()
{
    uassert_int_equal(isgraph(135), 0); /* isgraph should be 0 for 0x87 */
}

void t_isgraph_0x88()
{
    uassert_int_equal(isgraph(136), 0); /* isgraph should be 0 for 0x88 */
}

void t_isgraph_0x89()
{
    uassert_int_equal(isgraph(137), 0); /* isgraph should be 0 for 0x89 */
}

void t_isgraph_0x8a()
{
    uassert_int_equal(isgraph(138), 0); /* isgraph should be 0 for 0x8a */
}

void t_isgraph_0x8b()
{
    uassert_int_equal(isgraph(139), 0); /* isgraph should be 0 for 0x8b */
}

void t_isgraph_0x8c()
{
    uassert_int_equal(isgraph(140), 0); /* isgraph should be 0 for 0x8c */
}

void t_isgraph_0x8d()
{
    uassert_int_equal(isgraph(141), 0); /* isgraph should be 0 for 0x8d */
}

void t_isgraph_0x8e()
{
    uassert_int_equal(isgraph(142), 0); /* isgraph should be 0 for 0x8e */
}

void t_isgraph_0x8f()
{
    uassert_int_equal(isgraph(143), 0); /* isgraph should be 0 for 0x8f */
}

void t_isgraph_0x90()
{
    uassert_int_equal(isgraph(144), 0); /* isgraph should be 0 for 0x90 */
}

void t_isgraph_0x91()
{
    uassert_int_equal(isgraph(145), 0); /* isgraph should be 0 for 0x91 */
}

void t_isgraph_0x92()
{
    uassert_int_equal(isgraph(146), 0); /* isgraph should be 0 for 0x92 */
}

void t_isgraph_0x93()
{
    uassert_int_equal(isgraph(147), 0); /* isgraph should be 0 for 0x93 */
}

void t_isgraph_0x94()
{
    uassert_int_equal(isgraph(148), 0); /* isgraph should be 0 for 0x94 */
}

void t_isgraph_0x95()
{
    uassert_int_equal(isgraph(149), 0); /* isgraph should be 0 for 0x95 */
}

void t_isgraph_0x96()
{
    uassert_int_equal(isgraph(150), 0); /* isgraph should be 0 for 0x96 */
}

void t_isgraph_0x97()
{
    uassert_int_equal(isgraph(151), 0); /* isgraph should be 0 for 0x97 */
}

void t_isgraph_0x98()
{
    uassert_int_equal(isgraph(152), 0); /* isgraph should be 0 for 0x98 */
}

void t_isgraph_0x99()
{
    uassert_int_equal(isgraph(153), 0); /* isgraph should be 0 for 0x99 */
}

void t_isgraph_0x9a()
{
    uassert_int_equal(isgraph(154), 0); /* isgraph should be 0 for 0x9a */
}

void t_isgraph_0x9b()
{
    uassert_int_equal(isgraph(155), 0); /* isgraph should be 0 for 0x9b */
}

void t_isgraph_0x9c()
{
    uassert_int_equal(isgraph(156), 0); /* isgraph should be 0 for 0x9c */
}

void t_isgraph_0x9d()
{
    uassert_int_equal(isgraph(157), 0); /* isgraph should be 0 for 0x9d */
}

void t_isgraph_0x9e()
{
    uassert_int_equal(isgraph(158), 0); /* isgraph should be 0 for 0x9e */
}

void t_isgraph_0x9f()
{
    uassert_int_equal(isgraph(159), 0); /* isgraph should be 0 for 0x9f */
}

void t_isgraph_0xa0()
{
    uassert_int_equal(isgraph(160), 0); /* isgraph should be 0 for 0xa0 */
}

void t_isgraph_0xa1()
{
    uassert_int_equal(isgraph(161), 0); /* isgraph should be 0 for 0xa1 */
}

void t_isgraph_0xa2()
{
    uassert_int_equal(isgraph(162), 0); /* isgraph should be 0 for 0xa2 */
}

void t_isgraph_0xa3()
{
    uassert_int_equal(isgraph(163), 0); /* isgraph should be 0 for 0xa3 */
}

void t_isgraph_0xa4()
{
    uassert_int_equal(isgraph(164), 0); /* isgraph should be 0 for 0xa4 */
}

void t_isgraph_0xa5()
{
    uassert_int_equal(isgraph(165), 0); /* isgraph should be 0 for 0xa5 */
}

void t_isgraph_0xa6()
{
    uassert_int_equal(isgraph(166), 0); /* isgraph should be 0 for 0xa6 */
}

void t_isgraph_0xa7()
{
    uassert_int_equal(isgraph(167), 0); /* isgraph should be 0 for 0xa7 */
}

void t_isgraph_0xa8()
{
    uassert_int_equal(isgraph(168), 0); /* isgraph should be 0 for 0xa8 */
}

void t_isgraph_0xa9()
{
    uassert_int_equal(isgraph(169), 0); /* isgraph should be 0 for 0xa9 */
}

void t_isgraph_0xaa()
{
    uassert_int_equal(isgraph(170), 0); /* isgraph should be 0 for 0xaa */
}

void t_isgraph_0xab()
{
    uassert_int_equal(isgraph(171), 0); /* isgraph should be 0 for 0xab */
}

void t_isgraph_0xac()
{
    uassert_int_equal(isgraph(172), 0); /* isgraph should be 0 for 0xac */
}

void t_isgraph_0xad()
{
    uassert_int_equal(isgraph(173), 0); /* isgraph should be 0 for 0xad */
}

void t_isgraph_0xae()
{
    uassert_int_equal(isgraph(174), 0); /* isgraph should be 0 for 0xae */
}

void t_isgraph_0xaf()
{
    uassert_int_equal(isgraph(175), 0); /* isgraph should be 0 for 0xaf */
}

void t_isgraph_0xb0()
{
    uassert_int_equal(isgraph(176), 0); /* isgraph should be 0 for 0xb0 */
}

void t_isgraph_0xb1()
{
    uassert_int_equal(isgraph(177), 0); /* isgraph should be 0 for 0xb1 */
}

void t_isgraph_0xb2()
{
    uassert_int_equal(isgraph(178), 0); /* isgraph should be 0 for 0xb2 */
}

void t_isgraph_0xb3()
{
    uassert_int_equal(isgraph(179), 0); /* isgraph should be 0 for 0xb3 */
}

void t_isgraph_0xb4()
{
    uassert_int_equal(isgraph(180), 0); /* isgraph should be 0 for 0xb4 */
}

void t_isgraph_0xb5()
{
    uassert_int_equal(isgraph(181), 0); /* isgraph should be 0 for 0xb5 */
}

void t_isgraph_0xb6()
{
    uassert_int_equal(isgraph(182), 0); /* isgraph should be 0 for 0xb6 */
}

void t_isgraph_0xb7()
{
    uassert_int_equal(isgraph(183), 0); /* isgraph should be 0 for 0xb7 */
}

void t_isgraph_0xb8()
{
    uassert_int_equal(isgraph(184), 0); /* isgraph should be 0 for 0xb8 */
}

void t_isgraph_0xb9()
{
    uassert_int_equal(isgraph(185), 0); /* isgraph should be 0 for 0xb9 */
}

void t_isgraph_0xba()
{
    uassert_int_equal(isgraph(186), 0); /* isgraph should be 0 for 0xba */
}

void t_isgraph_0xbb()
{
    uassert_int_equal(isgraph(187), 0); /* isgraph should be 0 for 0xbb */
}

void t_isgraph_0xbc()
{
    uassert_int_equal(isgraph(188), 0); /* isgraph should be 0 for 0xbc */
}

void t_isgraph_0xbd()
{
    uassert_int_equal(isgraph(189), 0); /* isgraph should be 0 for 0xbd */
}

void t_isgraph_0xbe()
{
    uassert_int_equal(isgraph(190), 0); /* isgraph should be 0 for 0xbe */
}

void t_isgraph_0xbf()
{
    uassert_int_equal(isgraph(191), 0); /* isgraph should be 0 for 0xbf */
}

void t_isgraph_0xc0()
{
    uassert_int_equal(isgraph(192), 0); /* isgraph should be 0 for 0xc0 */
}

void t_isgraph_0xc1()
{
    uassert_int_equal(isgraph(193), 0); /* isgraph should be 0 for 0xc1 */
}

void t_isgraph_0xc2()
{
    uassert_int_equal(isgraph(194), 0); /* isgraph should be 0 for 0xc2 */
}

void t_isgraph_0xc3()
{
    uassert_int_equal(isgraph(195), 0); /* isgraph should be 0 for 0xc3 */
}

void t_isgraph_0xc4()
{
    uassert_int_equal(isgraph(196), 0); /* isgraph should be 0 for 0xc4 */
}

void t_isgraph_0xc5()
{
    uassert_int_equal(isgraph(197), 0); /* isgraph should be 0 for 0xc5 */
}

void t_isgraph_0xc6()
{
    uassert_int_equal(isgraph(198), 0); /* isgraph should be 0 for 0xc6 */
}

void t_isgraph_0xc7()
{
    uassert_int_equal(isgraph(199), 0); /* isgraph should be 0 for 0xc7 */
}

void t_isgraph_0xc8()
{
    uassert_int_equal(isgraph(200), 0); /* isgraph should be 0 for 0xc8 */
}

void t_isgraph_0xc9()
{
    uassert_int_equal(isgraph(201), 0); /* isgraph should be 0 for 0xc9 */
}

void t_isgraph_0xca()
{
    uassert_int_equal(isgraph(202), 0); /* isgraph should be 0 for 0xca */
}

void t_isgraph_0xcb()
{
    uassert_int_equal(isgraph(203), 0); /* isgraph should be 0 for 0xcb */
}

void t_isgraph_0xcc()
{
    uassert_int_equal(isgraph(204), 0); /* isgraph should be 0 for 0xcc */
}

void t_isgraph_0xcd()
{
    uassert_int_equal(isgraph(205), 0); /* isgraph should be 0 for 0xcd */
}

void t_isgraph_0xce()
{
    uassert_int_equal(isgraph(206), 0); /* isgraph should be 0 for 0xce */
}

void t_isgraph_0xcf()
{
    uassert_int_equal(isgraph(207), 0); /* isgraph should be 0 for 0xcf */
}

void t_isgraph_0xd0()
{
    uassert_int_equal(isgraph(208), 0); /* isgraph should be 0 for 0xd0 */
}

void t_isgraph_0xd1()
{
    uassert_int_equal(isgraph(209), 0); /* isgraph should be 0 for 0xd1 */
}

void t_isgraph_0xd2()
{
    uassert_int_equal(isgraph(210), 0); /* isgraph should be 0 for 0xd2 */
}

void t_isgraph_0xd3()
{
    uassert_int_equal(isgraph(211), 0); /* isgraph should be 0 for 0xd3 */
}

void t_isgraph_0xd4()
{
    uassert_int_equal(isgraph(212), 0); /* isgraph should be 0 for 0xd4 */
}

void t_isgraph_0xd5()
{
    uassert_int_equal(isgraph(213), 0); /* isgraph should be 0 for 0xd5 */
}

void t_isgraph_0xd6()
{
    uassert_int_equal(isgraph(214), 0); /* isgraph should be 0 for 0xd6 */
}

void t_isgraph_0xd7()
{
    uassert_int_equal(isgraph(215), 0); /* isgraph should be 0 for 0xd7 */
}

void t_isgraph_0xd8()
{
    uassert_int_equal(isgraph(216), 0); /* isgraph should be 0 for 0xd8 */
}

void t_isgraph_0xd9()
{
    uassert_int_equal(isgraph(217), 0); /* isgraph should be 0 for 0xd9 */
}

void t_isgraph_0xda()
{
    uassert_int_equal(isgraph(218), 0); /* isgraph should be 0 for 0xda */
}

void t_isgraph_0xdb()
{
    uassert_int_equal(isgraph(219), 0); /* isgraph should be 0 for 0xdb */
}

void t_isgraph_0xdc()
{
    uassert_int_equal(isgraph(220), 0); /* isgraph should be 0 for 0xdc */
}

void t_isgraph_0xdd()
{
    uassert_int_equal(isgraph(221), 0); /* isgraph should be 0 for 0xdd */
}

void t_isgraph_0xde()
{
    uassert_int_equal(isgraph(222), 0); /* isgraph should be 0 for 0xde */
}

void t_isgraph_0xdf()
{
    uassert_int_equal(isgraph(223), 0); /* isgraph should be 0 for 0xdf */
}

void t_isgraph_0xe0()
{
    uassert_int_equal(isgraph(224), 0); /* isgraph should be 0 for 0xe0 */
}

void t_isgraph_0xe1()
{
    uassert_int_equal(isgraph(225), 0); /* isgraph should be 0 for 0xe1 */
}

void t_isgraph_0xe2()
{
    uassert_int_equal(isgraph(226), 0); /* isgraph should be 0 for 0xe2 */
}

void t_isgraph_0xe3()
{
    uassert_int_equal(isgraph(227), 0); /* isgraph should be 0 for 0xe3 */
}

void t_isgraph_0xe4()
{
    uassert_int_equal(isgraph(228), 0); /* isgraph should be 0 for 0xe4 */
}

void t_isgraph_0xe5()
{
    uassert_int_equal(isgraph(229), 0); /* isgraph should be 0 for 0xe5 */
}

void t_isgraph_0xe6()
{
    uassert_int_equal(isgraph(230), 0); /* isgraph should be 0 for 0xe6 */
}

void t_isgraph_0xe7()
{
    uassert_int_equal(isgraph(231), 0); /* isgraph should be 0 for 0xe7 */
}

void t_isgraph_0xe8()
{
    uassert_int_equal(isgraph(232), 0); /* isgraph should be 0 for 0xe8 */
}

void t_isgraph_0xe9()
{
    uassert_int_equal(isgraph(233), 0); /* isgraph should be 0 for 0xe9 */
}

void t_isgraph_0xea()
{
    uassert_int_equal(isgraph(234), 0); /* isgraph should be 0 for 0xea */
}

void t_isgraph_0xeb()
{
    uassert_int_equal(isgraph(235), 0); /* isgraph should be 0 for 0xeb */
}

void t_isgraph_0xec()
{
    uassert_int_equal(isgraph(236), 0); /* isgraph should be 0 for 0xec */
}

void t_isgraph_0xed()
{
    uassert_int_equal(isgraph(237), 0); /* isgraph should be 0 for 0xed */
}

void t_isgraph_0xee()
{
    uassert_int_equal(isgraph(238), 0); /* isgraph should be 0 for 0xee */
}

void t_isgraph_0xef()
{
    uassert_int_equal(isgraph(239), 0); /* isgraph should be 0 for 0xef */
}

void t_isgraph_0xf0()
{
    uassert_int_equal(isgraph(240), 0); /* isgraph should be 0 for 0xf0 */
}

void t_isgraph_0xf1()
{
    uassert_int_equal(isgraph(241), 0); /* isgraph should be 0 for 0xf1 */
}

void t_isgraph_0xf2()
{
    uassert_int_equal(isgraph(242), 0); /* isgraph should be 0 for 0xf2 */
}

void t_isgraph_0xf3()
{
    uassert_int_equal(isgraph(243), 0); /* isgraph should be 0 for 0xf3 */
}

void t_isgraph_0xf4()
{
    uassert_int_equal(isgraph(244), 0); /* isgraph should be 0 for 0xf4 */
}

void t_isgraph_0xf5()
{
    uassert_int_equal(isgraph(245), 0); /* isgraph should be 0 for 0xf5 */
}

void t_isgraph_0xf6()
{
    uassert_int_equal(isgraph(246), 0); /* isgraph should be 0 for 0xf6 */
}

void t_isgraph_0xf7()
{
    uassert_int_equal(isgraph(247), 0); /* isgraph should be 0 for 0xf7 */
}

void t_isgraph_0xf8()
{
    uassert_int_equal(isgraph(248), 0); /* isgraph should be 0 for 0xf8 */
}

void t_isgraph_0xf9()
{
    uassert_int_equal(isgraph(249), 0); /* isgraph should be 0 for 0xf9 */
}

void t_isgraph_0xfa()
{
    uassert_int_equal(isgraph(250), 0); /* isgraph should be 0 for 0xfa */
}

void t_isgraph_0xfb()
{
    uassert_int_equal(isgraph(251), 0); /* isgraph should be 0 for 0xfb */
}

void t_isgraph_0xfc()
{
    uassert_int_equal(isgraph(252), 0); /* isgraph should be 0 for 0xfc */
}

void t_isgraph_0xfd()
{
    uassert_int_equal(isgraph(253), 0); /* isgraph should be 0 for 0xfd */
}

void t_isgraph_0xfe()
{
    uassert_int_equal(isgraph(254), 0); /* isgraph should be 0 for 0xfe */
}

void t_isgraph_0xff()
{
    uassert_int_equal(isgraph(255), 0); /* isgraph should be 0 for 0xff */
}



static int testcase(void)
{
    t_isgraph_0x00();
    t_isgraph_0x01();
    t_isgraph_0x02();
    t_isgraph_0x03();
    t_isgraph_0x04();
    t_isgraph_0x05();
    t_isgraph_0x06();
    t_isgraph_0x07();
    t_isgraph_0x08();
    t_isgraph_0x09();
    t_isgraph_0x0a();
    t_isgraph_0x0b();
    t_isgraph_0x0c();
    t_isgraph_0x0d();
    t_isgraph_0x0e();
    t_isgraph_0x0f();
    t_isgraph_0x10();
    t_isgraph_0x11();
    t_isgraph_0x12();
    t_isgraph_0x13();
    t_isgraph_0x14();
    t_isgraph_0x15();
    t_isgraph_0x16();
    t_isgraph_0x17();
    t_isgraph_0x18();
    t_isgraph_0x19();
    t_isgraph_0x1a();
    t_isgraph_0x1b();
    t_isgraph_0x1c();
    t_isgraph_0x1d();
    t_isgraph_0x1e();
    t_isgraph_0x1f();
    t_isgraph_0x20();
    t_isgraph_0x21();
    t_isgraph_0x22();
    t_isgraph_0x23();
    t_isgraph_0x24();
    t_isgraph_0x25();
    t_isgraph_0x26();
    t_isgraph_0x27();
    t_isgraph_0x28();
    t_isgraph_0x29();
    t_isgraph_0x2a();
    t_isgraph_0x2b();
    t_isgraph_0x2c();
    t_isgraph_0x2d();
    t_isgraph_0x2e();
    t_isgraph_0x2f();
    t_isgraph_0x30();
    t_isgraph_0x31();
    t_isgraph_0x32();
    t_isgraph_0x33();
    t_isgraph_0x34();
    t_isgraph_0x35();
    t_isgraph_0x36();
    t_isgraph_0x37();
    t_isgraph_0x38();
    t_isgraph_0x39();
    t_isgraph_0x3a();
    t_isgraph_0x3b();
    t_isgraph_0x3c();
    t_isgraph_0x3d();
    t_isgraph_0x3e();
    t_isgraph_0x3f();
    t_isgraph_0x40();
    t_isgraph_0x41();
    t_isgraph_0x42();
    t_isgraph_0x43();
    t_isgraph_0x44();
    t_isgraph_0x45();
    t_isgraph_0x46();
    t_isgraph_0x47();
    t_isgraph_0x48();
    t_isgraph_0x49();
    t_isgraph_0x4a();
    t_isgraph_0x4b();
    t_isgraph_0x4c();
    t_isgraph_0x4d();
    t_isgraph_0x4e();
    t_isgraph_0x4f();
    t_isgraph_0x50();
    t_isgraph_0x51();
    t_isgraph_0x52();
    t_isgraph_0x53();
    t_isgraph_0x54();
    t_isgraph_0x55();
    t_isgraph_0x56();
    t_isgraph_0x57();
    t_isgraph_0x58();
    t_isgraph_0x59();
    t_isgraph_0x5a();
    t_isgraph_0x5b();
    t_isgraph_0x5c();
    t_isgraph_0x5d();
    t_isgraph_0x5e();
    t_isgraph_0x5f();
    t_isgraph_0x60();
    t_isgraph_0x61();
    t_isgraph_0x62();
    t_isgraph_0x63();
    t_isgraph_0x64();
    t_isgraph_0x65();
    t_isgraph_0x66();
    t_isgraph_0x67();
    t_isgraph_0x68();
    t_isgraph_0x69();
    t_isgraph_0x6a();
    t_isgraph_0x6b();
    t_isgraph_0x6c();
    t_isgraph_0x6d();
    t_isgraph_0x6e();
    t_isgraph_0x6f();
    t_isgraph_0x70();
    t_isgraph_0x71();
    t_isgraph_0x72();
    t_isgraph_0x73();
    t_isgraph_0x74();
    t_isgraph_0x75();
    t_isgraph_0x76();
    t_isgraph_0x77();
    t_isgraph_0x78();
    t_isgraph_0x79();
    t_isgraph_0x7a();
    t_isgraph_0x7b();
    t_isgraph_0x7c();
    t_isgraph_0x7d();
    t_isgraph_0x7e();
    t_isgraph_0x7f();
    t_isgraph_0x80();
    t_isgraph_0x81();
    t_isgraph_0x82();
    t_isgraph_0x83();
    t_isgraph_0x84();
    t_isgraph_0x85();
    t_isgraph_0x86();
    t_isgraph_0x87();
    t_isgraph_0x88();
    t_isgraph_0x89();
    t_isgraph_0x8a();
    t_isgraph_0x8b();
    t_isgraph_0x8c();
    t_isgraph_0x8d();
    t_isgraph_0x8e();
    t_isgraph_0x8f();
    t_isgraph_0x90();
    t_isgraph_0x91();
    t_isgraph_0x92();
    t_isgraph_0x93();
    t_isgraph_0x94();
    t_isgraph_0x95();
    t_isgraph_0x96();
    t_isgraph_0x97();
    t_isgraph_0x98();
    t_isgraph_0x99();
    t_isgraph_0x9a();
    t_isgraph_0x9b();
    t_isgraph_0x9c();
    t_isgraph_0x9d();
    t_isgraph_0x9e();
    t_isgraph_0x9f();
    t_isgraph_0xa0();
    t_isgraph_0xa1();
    t_isgraph_0xa2();
    t_isgraph_0xa3();
    t_isgraph_0xa4();
    t_isgraph_0xa5();
    t_isgraph_0xa6();
    t_isgraph_0xa7();
    t_isgraph_0xa8();
    t_isgraph_0xa9();
    t_isgraph_0xaa();
    t_isgraph_0xab();
    t_isgraph_0xac();
    t_isgraph_0xad();
    t_isgraph_0xae();
    t_isgraph_0xaf();
    t_isgraph_0xb0();
    t_isgraph_0xb1();
    t_isgraph_0xb2();
    t_isgraph_0xb3();
    t_isgraph_0xb4();
    t_isgraph_0xb5();
    t_isgraph_0xb6();
    t_isgraph_0xb7();
    t_isgraph_0xb8();
    t_isgraph_0xb9();
    t_isgraph_0xba();
    t_isgraph_0xbb();
    t_isgraph_0xbc();
    t_isgraph_0xbd();
    t_isgraph_0xbe();
    t_isgraph_0xbf();
    t_isgraph_0xc0();
    t_isgraph_0xc1();
    t_isgraph_0xc2();
    t_isgraph_0xc3();
    t_isgraph_0xc4();
    t_isgraph_0xc5();
    t_isgraph_0xc6();
    t_isgraph_0xc7();
    t_isgraph_0xc8();
    t_isgraph_0xc9();
    t_isgraph_0xca();
    t_isgraph_0xcb();
    t_isgraph_0xcc();
    t_isgraph_0xcd();
    t_isgraph_0xce();
    t_isgraph_0xcf();
    t_isgraph_0xd0();
    t_isgraph_0xd1();
    t_isgraph_0xd2();
    t_isgraph_0xd3();
    t_isgraph_0xd4();
    t_isgraph_0xd5();
    t_isgraph_0xd6();
    t_isgraph_0xd7();
    t_isgraph_0xd8();
    t_isgraph_0xd9();
    t_isgraph_0xda();
    t_isgraph_0xdb();
    t_isgraph_0xdc();
    t_isgraph_0xdd();
    t_isgraph_0xde();
    t_isgraph_0xdf();
    t_isgraph_0xe0();
    t_isgraph_0xe1();
    t_isgraph_0xe2();
    t_isgraph_0xe3();
    t_isgraph_0xe4();
    t_isgraph_0xe5();
    t_isgraph_0xe6();
    t_isgraph_0xe7();
    t_isgraph_0xe8();
    t_isgraph_0xe9();
    t_isgraph_0xea();
    t_isgraph_0xeb();
    t_isgraph_0xec();
    t_isgraph_0xed();
    t_isgraph_0xee();
    t_isgraph_0xef();
    t_isgraph_0xf0();
    t_isgraph_0xf1();
    t_isgraph_0xf2();
    t_isgraph_0xf3();
    t_isgraph_0xf4();
    t_isgraph_0xf5();
    t_isgraph_0xf6();
    t_isgraph_0xf7();
    t_isgraph_0xf8();
    t_isgraph_0xf9();
    t_isgraph_0xfa();
    t_isgraph_0xfb();
    t_isgraph_0xfc();
    t_isgraph_0xfd();
    t_isgraph_0xfe();
    t_isgraph_0xff();
}


int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
