#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))


void t_isupper_0x00()
{
    uassert_int_equal(isupper(0), 0); /* isupper should be 0 for 0x00 */
}

void t_isupper_0x01()
{
    uassert_int_equal(isupper(1), 0); /* isupper should be 0 for 0x01 */
}

void t_isupper_0x02()
{
    uassert_int_equal(isupper(2), 0); /* isupper should be 0 for 0x02 */
}

void t_isupper_0x03()
{
    uassert_int_equal(isupper(3), 0); /* isupper should be 0 for 0x03 */
}

void t_isupper_0x04()
{
    uassert_int_equal(isupper(4), 0); /* isupper should be 0 for 0x04 */
}

void t_isupper_0x05()
{
    uassert_int_equal(isupper(5), 0); /* isupper should be 0 for 0x05 */
}

void t_isupper_0x06()
{
    uassert_int_equal(isupper(6), 0); /* isupper should be 0 for 0x06 */
}

void t_isupper_0x07()
{
    uassert_int_equal(isupper(7), 0); /* isupper should be 0 for 0x07 */
}

void t_isupper_0x08()
{
    uassert_int_equal(isupper(8), 0); /* isupper should be 0 for 0x08 */
}

void t_isupper_0x09()
{
    uassert_int_equal(isupper(9), 0); /* isupper should be 0 for 0x09 */
}

void t_isupper_0x0a()
{
    uassert_int_equal(isupper(10), 0); /* isupper should be 0 for 0x0a */
}

void t_isupper_0x0b()
{
    uassert_int_equal(isupper(11), 0); /* isupper should be 0 for 0x0b */
}

void t_isupper_0x0c()
{
    uassert_int_equal(isupper(12), 0); /* isupper should be 0 for 0x0c */
}

void t_isupper_0x0d()
{
    uassert_int_equal(isupper(13), 0); /* isupper should be 0 for 0x0d */
}

void t_isupper_0x0e()
{
    uassert_int_equal(isupper(14), 0); /* isupper should be 0 for 0x0e */
}

void t_isupper_0x0f()
{
    uassert_int_equal(isupper(15), 0); /* isupper should be 0 for 0x0f */
}

void t_isupper_0x10()
{
    uassert_int_equal(isupper(16), 0); /* isupper should be 0 for 0x10 */
}

void t_isupper_0x11()
{
    uassert_int_equal(isupper(17), 0); /* isupper should be 0 for 0x11 */
}

void t_isupper_0x12()
{
    uassert_int_equal(isupper(18), 0); /* isupper should be 0 for 0x12 */
}

void t_isupper_0x13()
{
    uassert_int_equal(isupper(19), 0); /* isupper should be 0 for 0x13 */
}

void t_isupper_0x14()
{
    uassert_int_equal(isupper(20), 0); /* isupper should be 0 for 0x14 */
}

void t_isupper_0x15()
{
    uassert_int_equal(isupper(21), 0); /* isupper should be 0 for 0x15 */
}

void t_isupper_0x16()
{
    uassert_int_equal(isupper(22), 0); /* isupper should be 0 for 0x16 */
}

void t_isupper_0x17()
{
    uassert_int_equal(isupper(23), 0); /* isupper should be 0 for 0x17 */
}

void t_isupper_0x18()
{
    uassert_int_equal(isupper(24), 0); /* isupper should be 0 for 0x18 */
}

void t_isupper_0x19()
{
    uassert_int_equal(isupper(25), 0); /* isupper should be 0 for 0x19 */
}

void t_isupper_0x1a()
{
    uassert_int_equal(isupper(26), 0); /* isupper should be 0 for 0x1a */
}

void t_isupper_0x1b()
{
    uassert_int_equal(isupper(27), 0); /* isupper should be 0 for 0x1b */
}

void t_isupper_0x1c()
{
    uassert_int_equal(isupper(28), 0); /* isupper should be 0 for 0x1c */
}

void t_isupper_0x1d()
{
    uassert_int_equal(isupper(29), 0); /* isupper should be 0 for 0x1d */
}

void t_isupper_0x1e()
{
    uassert_int_equal(isupper(30), 0); /* isupper should be 0 for 0x1e */
}

void t_isupper_0x1f()
{
    uassert_int_equal(isupper(31), 0); /* isupper should be 0 for 0x1f */
}

void t_isupper_0x20()
{
    uassert_int_equal(isupper(32), 0); /* isupper should be 0 for   */
}

void t_isupper_0x21()
{
    uassert_int_equal(isupper(33), 0); /* isupper should be 0 for ! */
}

void t_isupper_0x22()
{
    uassert_int_equal(isupper(34), 0); /* isupper should be 0 for 0x22 */
}

void t_isupper_0x23()
{
    uassert_int_equal(isupper(35), 0); /* isupper should be 0 for # */
}

void t_isupper_0x24()
{
    uassert_int_equal(isupper(36), 0); /* isupper should be 0 for $ */
}

void t_isupper_0x25()
{
    uassert_int_equal(isupper(37), 0); /* isupper should be 0 for % */
}

void t_isupper_0x26()
{
    uassert_int_equal(isupper(38), 0); /* isupper should be 0 for & */
}

void t_isupper_0x27()
{
    uassert_int_equal(isupper(39), 0); /* isupper should be 0 for ' */
}

void t_isupper_0x28()
{
    uassert_int_equal(isupper(40), 0); /* isupper should be 0 for ( */
}

void t_isupper_0x29()
{
    uassert_int_equal(isupper(41), 0); /* isupper should be 0 for ) */
}

void t_isupper_0x2a()
{
    uassert_int_equal(isupper(42), 0); /* isupper should be 0 for * */
}

void t_isupper_0x2b()
{
    uassert_int_equal(isupper(43), 0); /* isupper should be 0 for + */
}

void t_isupper_0x2c()
{
    uassert_int_equal(isupper(44), 0); /* isupper should be 0 for , */
}

void t_isupper_0x2d()
{
    uassert_int_equal(isupper(45), 0); /* isupper should be 0 for - */
}

void t_isupper_0x2e()
{
    uassert_int_equal(isupper(46), 0); /* isupper should be 0 for . */
}

void t_isupper_0x2f()
{
    uassert_int_equal(isupper(47), 0); /* isupper should be 0 for / */
}

void t_isupper_0x30()
{
    uassert_int_equal(isupper(48), 0); /* isupper should be 0 for 0 */
}

void t_isupper_0x31()
{
    uassert_int_equal(isupper(49), 0); /* isupper should be 0 for 1 */
}

void t_isupper_0x32()
{
    uassert_int_equal(isupper(50), 0); /* isupper should be 0 for 2 */
}

void t_isupper_0x33()
{
    uassert_int_equal(isupper(51), 0); /* isupper should be 0 for 3 */
}

void t_isupper_0x34()
{
    uassert_int_equal(isupper(52), 0); /* isupper should be 0 for 4 */
}

void t_isupper_0x35()
{
    uassert_int_equal(isupper(53), 0); /* isupper should be 0 for 5 */
}

void t_isupper_0x36()
{
    uassert_int_equal(isupper(54), 0); /* isupper should be 0 for 6 */
}

void t_isupper_0x37()
{
    uassert_int_equal(isupper(55), 0); /* isupper should be 0 for 7 */
}

void t_isupper_0x38()
{
    uassert_int_equal(isupper(56), 0); /* isupper should be 0 for 8 */
}

void t_isupper_0x39()
{
    uassert_int_equal(isupper(57), 0); /* isupper should be 0 for 9 */
}

void t_isupper_0x3a()
{
    uassert_int_equal(isupper(58), 0); /* isupper should be 0 for : */
}

void t_isupper_0x3b()
{
    uassert_int_equal(isupper(59), 0); /* isupper should be 0 for ; */
}

void t_isupper_0x3c()
{
    uassert_int_equal(isupper(60), 0); /* isupper should be 0 for < */
}

void t_isupper_0x3d()
{
    uassert_int_equal(isupper(61), 0); /* isupper should be 0 for = */
}

void t_isupper_0x3e()
{
    uassert_int_equal(isupper(62), 0); /* isupper should be 0 for > */
}

void t_isupper_0x3f()
{
    uassert_int_equal(isupper(63), 0); /* isupper should be 0 for ? */
}

void t_isupper_0x40()
{
    uassert_int_equal(isupper(64), 0); /* isupper should be 0 for @ */
}

void t_isupper_0x41()
{
    uassert_int_equal(isupper(65), 1); /* isupper should be 1 for A */
}

void t_isupper_0x42()
{
    uassert_int_equal(isupper(66), 1); /* isupper should be 1 for B */
}

void t_isupper_0x43()
{
    uassert_int_equal(isupper(67), 1); /* isupper should be 1 for C */
}

void t_isupper_0x44()
{
    uassert_int_equal(isupper(68), 1); /* isupper should be 1 for D */
}

void t_isupper_0x45()
{
    uassert_int_equal(isupper(69), 1); /* isupper should be 1 for E */
}

void t_isupper_0x46()
{
    uassert_int_equal(isupper(70), 1); /* isupper should be 1 for F */
}

void t_isupper_0x47()
{
    uassert_int_equal(isupper(71), 1); /* isupper should be 1 for G */
}

void t_isupper_0x48()
{
    uassert_int_equal(isupper(72), 1); /* isupper should be 1 for H */
}

void t_isupper_0x49()
{
    uassert_int_equal(isupper(73), 1); /* isupper should be 1 for I */
}

void t_isupper_0x4a()
{
    uassert_int_equal(isupper(74), 1); /* isupper should be 1 for J */
}

void t_isupper_0x4b()
{
    uassert_int_equal(isupper(75), 1); /* isupper should be 1 for K */
}

void t_isupper_0x4c()
{
    uassert_int_equal(isupper(76), 1); /* isupper should be 1 for L */
}

void t_isupper_0x4d()
{
    uassert_int_equal(isupper(77), 1); /* isupper should be 1 for M */
}

void t_isupper_0x4e()
{
    uassert_int_equal(isupper(78), 1); /* isupper should be 1 for N */
}

void t_isupper_0x4f()
{
    uassert_int_equal(isupper(79), 1); /* isupper should be 1 for O */
}

void t_isupper_0x50()
{
    uassert_int_equal(isupper(80), 1); /* isupper should be 1 for P */
}

void t_isupper_0x51()
{
    uassert_int_equal(isupper(81), 1); /* isupper should be 1 for Q */
}

void t_isupper_0x52()
{
    uassert_int_equal(isupper(82), 1); /* isupper should be 1 for R */
}

void t_isupper_0x53()
{
    uassert_int_equal(isupper(83), 1); /* isupper should be 1 for S */
}

void t_isupper_0x54()
{
    uassert_int_equal(isupper(84), 1); /* isupper should be 1 for T */
}

void t_isupper_0x55()
{
    uassert_int_equal(isupper(85), 1); /* isupper should be 1 for U */
}

void t_isupper_0x56()
{
    uassert_int_equal(isupper(86), 1); /* isupper should be 1 for V */
}

void t_isupper_0x57()
{
    uassert_int_equal(isupper(87), 1); /* isupper should be 1 for W */
}

void t_isupper_0x58()
{
    uassert_int_equal(isupper(88), 1); /* isupper should be 1 for X */
}

void t_isupper_0x59()
{
    uassert_int_equal(isupper(89), 1); /* isupper should be 1 for Y */
}

void t_isupper_0x5a()
{
    uassert_int_equal(isupper(90), 1); /* isupper should be 1 for Z */
}

void t_isupper_0x5b()
{
    uassert_int_equal(isupper(91), 0); /* isupper should be 0 for [ */
}

void t_isupper_0x5c()
{
    uassert_int_equal(isupper(92), 0); /* isupper should be 0 for 0x5c */
}

void t_isupper_0x5d()
{
    uassert_int_equal(isupper(93), 0); /* isupper should be 0 for ] */
}

void t_isupper_0x5e()
{
    uassert_int_equal(isupper(94), 0); /* isupper should be 0 for ^ */
}

void t_isupper_0x5f()
{
    uassert_int_equal(isupper(95), 0); /* isupper should be 0 for _ */
}

void t_isupper_0x60()
{
    uassert_int_equal(isupper(96), 0); /* isupper should be 0 for ` */
}

void t_isupper_0x61()
{
    uassert_int_equal(isupper(97), 0); /* isupper should be 0 for a */
}

void t_isupper_0x62()
{
    uassert_int_equal(isupper(98), 0); /* isupper should be 0 for b */
}

void t_isupper_0x63()
{
    uassert_int_equal(isupper(99), 0); /* isupper should be 0 for c */
}

void t_isupper_0x64()
{
    uassert_int_equal(isupper(100), 0); /* isupper should be 0 for d */
}

void t_isupper_0x65()
{
    uassert_int_equal(isupper(101), 0); /* isupper should be 0 for e */
}

void t_isupper_0x66()
{
    uassert_int_equal(isupper(102), 0); /* isupper should be 0 for f */
}

void t_isupper_0x67()
{
    uassert_int_equal(isupper(103), 0); /* isupper should be 0 for g */
}

void t_isupper_0x68()
{
    uassert_int_equal(isupper(104), 0); /* isupper should be 0 for h */
}

void t_isupper_0x69()
{
    uassert_int_equal(isupper(105), 0); /* isupper should be 0 for i */
}

void t_isupper_0x6a()
{
    uassert_int_equal(isupper(106), 0); /* isupper should be 0 for j */
}

void t_isupper_0x6b()
{
    uassert_int_equal(isupper(107), 0); /* isupper should be 0 for k */
}

void t_isupper_0x6c()
{
    uassert_int_equal(isupper(108), 0); /* isupper should be 0 for l */
}

void t_isupper_0x6d()
{
    uassert_int_equal(isupper(109), 0); /* isupper should be 0 for m */
}

void t_isupper_0x6e()
{
    uassert_int_equal(isupper(110), 0); /* isupper should be 0 for n */
}

void t_isupper_0x6f()
{
    uassert_int_equal(isupper(111), 0); /* isupper should be 0 for o */
}

void t_isupper_0x70()
{
    uassert_int_equal(isupper(112), 0); /* isupper should be 0 for p */
}

void t_isupper_0x71()
{
    uassert_int_equal(isupper(113), 0); /* isupper should be 0 for q */
}

void t_isupper_0x72()
{
    uassert_int_equal(isupper(114), 0); /* isupper should be 0 for r */
}

void t_isupper_0x73()
{
    uassert_int_equal(isupper(115), 0); /* isupper should be 0 for s */
}

void t_isupper_0x74()
{
    uassert_int_equal(isupper(116), 0); /* isupper should be 0 for t */
}

void t_isupper_0x75()
{
    uassert_int_equal(isupper(117), 0); /* isupper should be 0 for u */
}

void t_isupper_0x76()
{
    uassert_int_equal(isupper(118), 0); /* isupper should be 0 for v */
}

void t_isupper_0x77()
{
    uassert_int_equal(isupper(119), 0); /* isupper should be 0 for w */
}

void t_isupper_0x78()
{
    uassert_int_equal(isupper(120), 0); /* isupper should be 0 for x */
}

void t_isupper_0x79()
{
    uassert_int_equal(isupper(121), 0); /* isupper should be 0 for y */
}

void t_isupper_0x7a()
{
    uassert_int_equal(isupper(122), 0); /* isupper should be 0 for z */
}

void t_isupper_0x7b()
{
    uassert_int_equal(isupper(123), 0); /* isupper should be 0 for { */
}

void t_isupper_0x7c()
{
    uassert_int_equal(isupper(124), 0); /* isupper should be 0 for | */
}

void t_isupper_0x7d()
{
    uassert_int_equal(isupper(125), 0); /* isupper should be 0 for } */
}

void t_isupper_0x7e()
{
    uassert_int_equal(isupper(126), 0); /* isupper should be 0 for ~ */
}

void t_isupper_0x7f()
{
    uassert_int_equal(isupper(127), 0); /* isupper should be 0 for 0x7f */
}

void t_isupper_0x80()
{
    uassert_int_equal(isupper(128), 0); /* isupper should be 0 for 0x80 */
}

void t_isupper_0x81()
{
    uassert_int_equal(isupper(129), 0); /* isupper should be 0 for 0x81 */
}

void t_isupper_0x82()
{
    uassert_int_equal(isupper(130), 0); /* isupper should be 0 for 0x82 */
}

void t_isupper_0x83()
{
    uassert_int_equal(isupper(131), 0); /* isupper should be 0 for 0x83 */
}

void t_isupper_0x84()
{
    uassert_int_equal(isupper(132), 0); /* isupper should be 0 for 0x84 */
}

void t_isupper_0x85()
{
    uassert_int_equal(isupper(133), 0); /* isupper should be 0 for 0x85 */
}

void t_isupper_0x86()
{
    uassert_int_equal(isupper(134), 0); /* isupper should be 0 for 0x86 */
}

void t_isupper_0x87()
{
    uassert_int_equal(isupper(135), 0); /* isupper should be 0 for 0x87 */
}

void t_isupper_0x88()
{
    uassert_int_equal(isupper(136), 0); /* isupper should be 0 for 0x88 */
}

void t_isupper_0x89()
{
    uassert_int_equal(isupper(137), 0); /* isupper should be 0 for 0x89 */
}

void t_isupper_0x8a()
{
    uassert_int_equal(isupper(138), 0); /* isupper should be 0 for 0x8a */
}

void t_isupper_0x8b()
{
    uassert_int_equal(isupper(139), 0); /* isupper should be 0 for 0x8b */
}

void t_isupper_0x8c()
{
    uassert_int_equal(isupper(140), 0); /* isupper should be 0 for 0x8c */
}

void t_isupper_0x8d()
{
    uassert_int_equal(isupper(141), 0); /* isupper should be 0 for 0x8d */
}

void t_isupper_0x8e()
{
    uassert_int_equal(isupper(142), 0); /* isupper should be 0 for 0x8e */
}

void t_isupper_0x8f()
{
    uassert_int_equal(isupper(143), 0); /* isupper should be 0 for 0x8f */
}

void t_isupper_0x90()
{
    uassert_int_equal(isupper(144), 0); /* isupper should be 0 for 0x90 */
}

void t_isupper_0x91()
{
    uassert_int_equal(isupper(145), 0); /* isupper should be 0 for 0x91 */
}

void t_isupper_0x92()
{
    uassert_int_equal(isupper(146), 0); /* isupper should be 0 for 0x92 */
}

void t_isupper_0x93()
{
    uassert_int_equal(isupper(147), 0); /* isupper should be 0 for 0x93 */
}

void t_isupper_0x94()
{
    uassert_int_equal(isupper(148), 0); /* isupper should be 0 for 0x94 */
}

void t_isupper_0x95()
{
    uassert_int_equal(isupper(149), 0); /* isupper should be 0 for 0x95 */
}

void t_isupper_0x96()
{
    uassert_int_equal(isupper(150), 0); /* isupper should be 0 for 0x96 */
}

void t_isupper_0x97()
{
    uassert_int_equal(isupper(151), 0); /* isupper should be 0 for 0x97 */
}

void t_isupper_0x98()
{
    uassert_int_equal(isupper(152), 0); /* isupper should be 0 for 0x98 */
}

void t_isupper_0x99()
{
    uassert_int_equal(isupper(153), 0); /* isupper should be 0 for 0x99 */
}

void t_isupper_0x9a()
{
    uassert_int_equal(isupper(154), 0); /* isupper should be 0 for 0x9a */
}

void t_isupper_0x9b()
{
    uassert_int_equal(isupper(155), 0); /* isupper should be 0 for 0x9b */
}

void t_isupper_0x9c()
{
    uassert_int_equal(isupper(156), 0); /* isupper should be 0 for 0x9c */
}

void t_isupper_0x9d()
{
    uassert_int_equal(isupper(157), 0); /* isupper should be 0 for 0x9d */
}

void t_isupper_0x9e()
{
    uassert_int_equal(isupper(158), 0); /* isupper should be 0 for 0x9e */
}

void t_isupper_0x9f()
{
    uassert_int_equal(isupper(159), 0); /* isupper should be 0 for 0x9f */
}

void t_isupper_0xa0()
{
    uassert_int_equal(isupper(160), 0); /* isupper should be 0 for 0xa0 */
}

void t_isupper_0xa1()
{
    uassert_int_equal(isupper(161), 0); /* isupper should be 0 for 0xa1 */
}

void t_isupper_0xa2()
{
    uassert_int_equal(isupper(162), 0); /* isupper should be 0 for 0xa2 */
}

void t_isupper_0xa3()
{
    uassert_int_equal(isupper(163), 0); /* isupper should be 0 for 0xa3 */
}

void t_isupper_0xa4()
{
    uassert_int_equal(isupper(164), 0); /* isupper should be 0 for 0xa4 */
}

void t_isupper_0xa5()
{
    uassert_int_equal(isupper(165), 0); /* isupper should be 0 for 0xa5 */
}

void t_isupper_0xa6()
{
    uassert_int_equal(isupper(166), 0); /* isupper should be 0 for 0xa6 */
}

void t_isupper_0xa7()
{
    uassert_int_equal(isupper(167), 0); /* isupper should be 0 for 0xa7 */
}

void t_isupper_0xa8()
{
    uassert_int_equal(isupper(168), 0); /* isupper should be 0 for 0xa8 */
}

void t_isupper_0xa9()
{
    uassert_int_equal(isupper(169), 0); /* isupper should be 0 for 0xa9 */
}

void t_isupper_0xaa()
{
    uassert_int_equal(isupper(170), 0); /* isupper should be 0 for 0xaa */
}

void t_isupper_0xab()
{
    uassert_int_equal(isupper(171), 0); /* isupper should be 0 for 0xab */
}

void t_isupper_0xac()
{
    uassert_int_equal(isupper(172), 0); /* isupper should be 0 for 0xac */
}

void t_isupper_0xad()
{
    uassert_int_equal(isupper(173), 0); /* isupper should be 0 for 0xad */
}

void t_isupper_0xae()
{
    uassert_int_equal(isupper(174), 0); /* isupper should be 0 for 0xae */
}

void t_isupper_0xaf()
{
    uassert_int_equal(isupper(175), 0); /* isupper should be 0 for 0xaf */
}

void t_isupper_0xb0()
{
    uassert_int_equal(isupper(176), 0); /* isupper should be 0 for 0xb0 */
}

void t_isupper_0xb1()
{
    uassert_int_equal(isupper(177), 0); /* isupper should be 0 for 0xb1 */
}

void t_isupper_0xb2()
{
    uassert_int_equal(isupper(178), 0); /* isupper should be 0 for 0xb2 */
}

void t_isupper_0xb3()
{
    uassert_int_equal(isupper(179), 0); /* isupper should be 0 for 0xb3 */
}

void t_isupper_0xb4()
{
    uassert_int_equal(isupper(180), 0); /* isupper should be 0 for 0xb4 */
}

void t_isupper_0xb5()
{
    uassert_int_equal(isupper(181), 0); /* isupper should be 0 for 0xb5 */
}

void t_isupper_0xb6()
{
    uassert_int_equal(isupper(182), 0); /* isupper should be 0 for 0xb6 */
}

void t_isupper_0xb7()
{
    uassert_int_equal(isupper(183), 0); /* isupper should be 0 for 0xb7 */
}

void t_isupper_0xb8()
{
    uassert_int_equal(isupper(184), 0); /* isupper should be 0 for 0xb8 */
}

void t_isupper_0xb9()
{
    uassert_int_equal(isupper(185), 0); /* isupper should be 0 for 0xb9 */
}

void t_isupper_0xba()
{
    uassert_int_equal(isupper(186), 0); /* isupper should be 0 for 0xba */
}

void t_isupper_0xbb()
{
    uassert_int_equal(isupper(187), 0); /* isupper should be 0 for 0xbb */
}

void t_isupper_0xbc()
{
    uassert_int_equal(isupper(188), 0); /* isupper should be 0 for 0xbc */
}

void t_isupper_0xbd()
{
    uassert_int_equal(isupper(189), 0); /* isupper should be 0 for 0xbd */
}

void t_isupper_0xbe()
{
    uassert_int_equal(isupper(190), 0); /* isupper should be 0 for 0xbe */
}

void t_isupper_0xbf()
{
    uassert_int_equal(isupper(191), 0); /* isupper should be 0 for 0xbf */
}

void t_isupper_0xc0()
{
    uassert_int_equal(isupper(192), 0); /* isupper should be 0 for 0xc0 */
}

void t_isupper_0xc1()
{
    uassert_int_equal(isupper(193), 0); /* isupper should be 0 for 0xc1 */
}

void t_isupper_0xc2()
{
    uassert_int_equal(isupper(194), 0); /* isupper should be 0 for 0xc2 */
}

void t_isupper_0xc3()
{
    uassert_int_equal(isupper(195), 0); /* isupper should be 0 for 0xc3 */
}

void t_isupper_0xc4()
{
    uassert_int_equal(isupper(196), 0); /* isupper should be 0 for 0xc4 */
}

void t_isupper_0xc5()
{
    uassert_int_equal(isupper(197), 0); /* isupper should be 0 for 0xc5 */
}

void t_isupper_0xc6()
{
    uassert_int_equal(isupper(198), 0); /* isupper should be 0 for 0xc6 */
}

void t_isupper_0xc7()
{
    uassert_int_equal(isupper(199), 0); /* isupper should be 0 for 0xc7 */
}

void t_isupper_0xc8()
{
    uassert_int_equal(isupper(200), 0); /* isupper should be 0 for 0xc8 */
}

void t_isupper_0xc9()
{
    uassert_int_equal(isupper(201), 0); /* isupper should be 0 for 0xc9 */
}

void t_isupper_0xca()
{
    uassert_int_equal(isupper(202), 0); /* isupper should be 0 for 0xca */
}

void t_isupper_0xcb()
{
    uassert_int_equal(isupper(203), 0); /* isupper should be 0 for 0xcb */
}

void t_isupper_0xcc()
{
    uassert_int_equal(isupper(204), 0); /* isupper should be 0 for 0xcc */
}

void t_isupper_0xcd()
{
    uassert_int_equal(isupper(205), 0); /* isupper should be 0 for 0xcd */
}

void t_isupper_0xce()
{
    uassert_int_equal(isupper(206), 0); /* isupper should be 0 for 0xce */
}

void t_isupper_0xcf()
{
    uassert_int_equal(isupper(207), 0); /* isupper should be 0 for 0xcf */
}

void t_isupper_0xd0()
{
    uassert_int_equal(isupper(208), 0); /* isupper should be 0 for 0xd0 */
}

void t_isupper_0xd1()
{
    uassert_int_equal(isupper(209), 0); /* isupper should be 0 for 0xd1 */
}

void t_isupper_0xd2()
{
    uassert_int_equal(isupper(210), 0); /* isupper should be 0 for 0xd2 */
}

void t_isupper_0xd3()
{
    uassert_int_equal(isupper(211), 0); /* isupper should be 0 for 0xd3 */
}

void t_isupper_0xd4()
{
    uassert_int_equal(isupper(212), 0); /* isupper should be 0 for 0xd4 */
}

void t_isupper_0xd5()
{
    uassert_int_equal(isupper(213), 0); /* isupper should be 0 for 0xd5 */
}

void t_isupper_0xd6()
{
    uassert_int_equal(isupper(214), 0); /* isupper should be 0 for 0xd6 */
}

void t_isupper_0xd7()
{
    uassert_int_equal(isupper(215), 0); /* isupper should be 0 for 0xd7 */
}

void t_isupper_0xd8()
{
    uassert_int_equal(isupper(216), 0); /* isupper should be 0 for 0xd8 */
}

void t_isupper_0xd9()
{
    uassert_int_equal(isupper(217), 0); /* isupper should be 0 for 0xd9 */
}

void t_isupper_0xda()
{
    uassert_int_equal(isupper(218), 0); /* isupper should be 0 for 0xda */
}

void t_isupper_0xdb()
{
    uassert_int_equal(isupper(219), 0); /* isupper should be 0 for 0xdb */
}

void t_isupper_0xdc()
{
    uassert_int_equal(isupper(220), 0); /* isupper should be 0 for 0xdc */
}

void t_isupper_0xdd()
{
    uassert_int_equal(isupper(221), 0); /* isupper should be 0 for 0xdd */
}

void t_isupper_0xde()
{
    uassert_int_equal(isupper(222), 0); /* isupper should be 0 for 0xde */
}

void t_isupper_0xdf()
{
    uassert_int_equal(isupper(223), 0); /* isupper should be 0 for 0xdf */
}

void t_isupper_0xe0()
{
    uassert_int_equal(isupper(224), 0); /* isupper should be 0 for 0xe0 */
}

void t_isupper_0xe1()
{
    uassert_int_equal(isupper(225), 0); /* isupper should be 0 for 0xe1 */
}

void t_isupper_0xe2()
{
    uassert_int_equal(isupper(226), 0); /* isupper should be 0 for 0xe2 */
}

void t_isupper_0xe3()
{
    uassert_int_equal(isupper(227), 0); /* isupper should be 0 for 0xe3 */
}

void t_isupper_0xe4()
{
    uassert_int_equal(isupper(228), 0); /* isupper should be 0 for 0xe4 */
}

void t_isupper_0xe5()
{
    uassert_int_equal(isupper(229), 0); /* isupper should be 0 for 0xe5 */
}

void t_isupper_0xe6()
{
    uassert_int_equal(isupper(230), 0); /* isupper should be 0 for 0xe6 */
}

void t_isupper_0xe7()
{
    uassert_int_equal(isupper(231), 0); /* isupper should be 0 for 0xe7 */
}

void t_isupper_0xe8()
{
    uassert_int_equal(isupper(232), 0); /* isupper should be 0 for 0xe8 */
}

void t_isupper_0xe9()
{
    uassert_int_equal(isupper(233), 0); /* isupper should be 0 for 0xe9 */
}

void t_isupper_0xea()
{
    uassert_int_equal(isupper(234), 0); /* isupper should be 0 for 0xea */
}

void t_isupper_0xeb()
{
    uassert_int_equal(isupper(235), 0); /* isupper should be 0 for 0xeb */
}

void t_isupper_0xec()
{
    uassert_int_equal(isupper(236), 0); /* isupper should be 0 for 0xec */
}

void t_isupper_0xed()
{
    uassert_int_equal(isupper(237), 0); /* isupper should be 0 for 0xed */
}

void t_isupper_0xee()
{
    uassert_int_equal(isupper(238), 0); /* isupper should be 0 for 0xee */
}

void t_isupper_0xef()
{
    uassert_int_equal(isupper(239), 0); /* isupper should be 0 for 0xef */
}

void t_isupper_0xf0()
{
    uassert_int_equal(isupper(240), 0); /* isupper should be 0 for 0xf0 */
}

void t_isupper_0xf1()
{
    uassert_int_equal(isupper(241), 0); /* isupper should be 0 for 0xf1 */
}

void t_isupper_0xf2()
{
    uassert_int_equal(isupper(242), 0); /* isupper should be 0 for 0xf2 */
}

void t_isupper_0xf3()
{
    uassert_int_equal(isupper(243), 0); /* isupper should be 0 for 0xf3 */
}

void t_isupper_0xf4()
{
    uassert_int_equal(isupper(244), 0); /* isupper should be 0 for 0xf4 */
}

void t_isupper_0xf5()
{
    uassert_int_equal(isupper(245), 0); /* isupper should be 0 for 0xf5 */
}

void t_isupper_0xf6()
{
    uassert_int_equal(isupper(246), 0); /* isupper should be 0 for 0xf6 */
}

void t_isupper_0xf7()
{
    uassert_int_equal(isupper(247), 0); /* isupper should be 0 for 0xf7 */
}

void t_isupper_0xf8()
{
    uassert_int_equal(isupper(248), 0); /* isupper should be 0 for 0xf8 */
}

void t_isupper_0xf9()
{
    uassert_int_equal(isupper(249), 0); /* isupper should be 0 for 0xf9 */
}

void t_isupper_0xfa()
{
    uassert_int_equal(isupper(250), 0); /* isupper should be 0 for 0xfa */
}

void t_isupper_0xfb()
{
    uassert_int_equal(isupper(251), 0); /* isupper should be 0 for 0xfb */
}

void t_isupper_0xfc()
{
    uassert_int_equal(isupper(252), 0); /* isupper should be 0 for 0xfc */
}

void t_isupper_0xfd()
{
    uassert_int_equal(isupper(253), 0); /* isupper should be 0 for 0xfd */
}

void t_isupper_0xfe()
{
    uassert_int_equal(isupper(254), 0); /* isupper should be 0 for 0xfe */
}

void t_isupper_0xff()
{
    uassert_int_equal(isupper(255), 0); /* isupper should be 0 for 0xff */
}



int testcase()
{
    t_isupper_0x00();
    t_isupper_0x01();
    t_isupper_0x02();
    t_isupper_0x03();
    t_isupper_0x04();
    t_isupper_0x05();
    t_isupper_0x06();
    t_isupper_0x07();
    t_isupper_0x08();
    t_isupper_0x09();
    t_isupper_0x0a();
    t_isupper_0x0b();
    t_isupper_0x0c();
    t_isupper_0x0d();
    t_isupper_0x0e();
    t_isupper_0x0f();
    t_isupper_0x10();
    t_isupper_0x11();
    t_isupper_0x12();
    t_isupper_0x13();
    t_isupper_0x14();
    t_isupper_0x15();
    t_isupper_0x16();
    t_isupper_0x17();
    t_isupper_0x18();
    t_isupper_0x19();
    t_isupper_0x1a();
    t_isupper_0x1b();
    t_isupper_0x1c();
    t_isupper_0x1d();
    t_isupper_0x1e();
    t_isupper_0x1f();
    t_isupper_0x20();
    t_isupper_0x21();
    t_isupper_0x22();
    t_isupper_0x23();
    t_isupper_0x24();
    t_isupper_0x25();
    t_isupper_0x26();
    t_isupper_0x27();
    t_isupper_0x28();
    t_isupper_0x29();
    t_isupper_0x2a();
    t_isupper_0x2b();
    t_isupper_0x2c();
    t_isupper_0x2d();
    t_isupper_0x2e();
    t_isupper_0x2f();
    t_isupper_0x30();
    t_isupper_0x31();
    t_isupper_0x32();
    t_isupper_0x33();
    t_isupper_0x34();
    t_isupper_0x35();
    t_isupper_0x36();
    t_isupper_0x37();
    t_isupper_0x38();
    t_isupper_0x39();
    t_isupper_0x3a();
    t_isupper_0x3b();
    t_isupper_0x3c();
    t_isupper_0x3d();
    t_isupper_0x3e();
    t_isupper_0x3f();
    t_isupper_0x40();
    t_isupper_0x41();
    t_isupper_0x42();
    t_isupper_0x43();
    t_isupper_0x44();
    t_isupper_0x45();
    t_isupper_0x46();
    t_isupper_0x47();
    t_isupper_0x48();
    t_isupper_0x49();
    t_isupper_0x4a();
    t_isupper_0x4b();
    t_isupper_0x4c();
    t_isupper_0x4d();
    t_isupper_0x4e();
    t_isupper_0x4f();
    t_isupper_0x50();
    t_isupper_0x51();
    t_isupper_0x52();
    t_isupper_0x53();
    t_isupper_0x54();
    t_isupper_0x55();
    t_isupper_0x56();
    t_isupper_0x57();
    t_isupper_0x58();
    t_isupper_0x59();
    t_isupper_0x5a();
    t_isupper_0x5b();
    t_isupper_0x5c();
    t_isupper_0x5d();
    t_isupper_0x5e();
    t_isupper_0x5f();
    t_isupper_0x60();
    t_isupper_0x61();
    t_isupper_0x62();
    t_isupper_0x63();
    t_isupper_0x64();
    t_isupper_0x65();
    t_isupper_0x66();
    t_isupper_0x67();
    t_isupper_0x68();
    t_isupper_0x69();
    t_isupper_0x6a();
    t_isupper_0x6b();
    t_isupper_0x6c();
    t_isupper_0x6d();
    t_isupper_0x6e();
    t_isupper_0x6f();
    t_isupper_0x70();
    t_isupper_0x71();
    t_isupper_0x72();
    t_isupper_0x73();
    t_isupper_0x74();
    t_isupper_0x75();
    t_isupper_0x76();
    t_isupper_0x77();
    t_isupper_0x78();
    t_isupper_0x79();
    t_isupper_0x7a();
    t_isupper_0x7b();
    t_isupper_0x7c();
    t_isupper_0x7d();
    t_isupper_0x7e();
    t_isupper_0x7f();
    t_isupper_0x80();
    t_isupper_0x81();
    t_isupper_0x82();
    t_isupper_0x83();
    t_isupper_0x84();
    t_isupper_0x85();
    t_isupper_0x86();
    t_isupper_0x87();
    t_isupper_0x88();
    t_isupper_0x89();
    t_isupper_0x8a();
    t_isupper_0x8b();
    t_isupper_0x8c();
    t_isupper_0x8d();
    t_isupper_0x8e();
    t_isupper_0x8f();
    t_isupper_0x90();
    t_isupper_0x91();
    t_isupper_0x92();
    t_isupper_0x93();
    t_isupper_0x94();
    t_isupper_0x95();
    t_isupper_0x96();
    t_isupper_0x97();
    t_isupper_0x98();
    t_isupper_0x99();
    t_isupper_0x9a();
    t_isupper_0x9b();
    t_isupper_0x9c();
    t_isupper_0x9d();
    t_isupper_0x9e();
    t_isupper_0x9f();
    t_isupper_0xa0();
    t_isupper_0xa1();
    t_isupper_0xa2();
    t_isupper_0xa3();
    t_isupper_0xa4();
    t_isupper_0xa5();
    t_isupper_0xa6();
    t_isupper_0xa7();
    t_isupper_0xa8();
    t_isupper_0xa9();
    t_isupper_0xaa();
    t_isupper_0xab();
    t_isupper_0xac();
    t_isupper_0xad();
    t_isupper_0xae();
    t_isupper_0xaf();
    t_isupper_0xb0();
    t_isupper_0xb1();
    t_isupper_0xb2();
    t_isupper_0xb3();
    t_isupper_0xb4();
    t_isupper_0xb5();
    t_isupper_0xb6();
    t_isupper_0xb7();
    t_isupper_0xb8();
    t_isupper_0xb9();
    t_isupper_0xba();
    t_isupper_0xbb();
    t_isupper_0xbc();
    t_isupper_0xbd();
    t_isupper_0xbe();
    t_isupper_0xbf();
    t_isupper_0xc0();
    t_isupper_0xc1();
    t_isupper_0xc2();
    t_isupper_0xc3();
    t_isupper_0xc4();
    t_isupper_0xc5();
    t_isupper_0xc6();
    t_isupper_0xc7();
    t_isupper_0xc8();
    t_isupper_0xc9();
    t_isupper_0xca();
    t_isupper_0xcb();
    t_isupper_0xcc();
    t_isupper_0xcd();
    t_isupper_0xce();
    t_isupper_0xcf();
    t_isupper_0xd0();
    t_isupper_0xd1();
    t_isupper_0xd2();
    t_isupper_0xd3();
    t_isupper_0xd4();
    t_isupper_0xd5();
    t_isupper_0xd6();
    t_isupper_0xd7();
    t_isupper_0xd8();
    t_isupper_0xd9();
    t_isupper_0xda();
    t_isupper_0xdb();
    t_isupper_0xdc();
    t_isupper_0xdd();
    t_isupper_0xde();
    t_isupper_0xdf();
    t_isupper_0xe0();
    t_isupper_0xe1();
    t_isupper_0xe2();
    t_isupper_0xe3();
    t_isupper_0xe4();
    t_isupper_0xe5();
    t_isupper_0xe6();
    t_isupper_0xe7();
    t_isupper_0xe8();
    t_isupper_0xe9();
    t_isupper_0xea();
    t_isupper_0xeb();
    t_isupper_0xec();
    t_isupper_0xed();
    t_isupper_0xee();
    t_isupper_0xef();
    t_isupper_0xf0();
    t_isupper_0xf1();
    t_isupper_0xf2();
    t_isupper_0xf3();
    t_isupper_0xf4();
    t_isupper_0xf5();
    t_isupper_0xf6();
    t_isupper_0xf7();
    t_isupper_0xf8();
    t_isupper_0xf9();
    t_isupper_0xfa();
    t_isupper_0xfb();
    t_isupper_0xfc();
    t_isupper_0xfd();
    t_isupper_0xfe();
    t_isupper_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
