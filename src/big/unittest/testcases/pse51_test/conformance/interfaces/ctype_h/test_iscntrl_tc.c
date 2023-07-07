#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))


void t_iscntrl_0x00()
{
    uassert_int_equal(iscntrl(0), 1); /* iscntrl should be 1 for 0x00 */
}

void t_iscntrl_0x01()
{
    uassert_int_equal(iscntrl(1), 1); /* iscntrl should be 1 for 0x01 */
}

void t_iscntrl_0x02()
{
    uassert_int_equal(iscntrl(2), 1); /* iscntrl should be 1 for 0x02 */
}

void t_iscntrl_0x03()
{
    uassert_int_equal(iscntrl(3), 1); /* iscntrl should be 1 for 0x03 */
}

void t_iscntrl_0x04()
{
    uassert_int_equal(iscntrl(4), 1); /* iscntrl should be 1 for 0x04 */
}

void t_iscntrl_0x05()
{
    uassert_int_equal(iscntrl(5), 1); /* iscntrl should be 1 for 0x05 */
}

void t_iscntrl_0x06()
{
    uassert_int_equal(iscntrl(6), 1); /* iscntrl should be 1 for 0x06 */
}

void t_iscntrl_0x07()
{
    uassert_int_equal(iscntrl(7), 1); /* iscntrl should be 1 for 0x07 */
}

void t_iscntrl_0x08()
{
    uassert_int_equal(iscntrl(8), 1); /* iscntrl should be 1 for 0x08 */
}

void t_iscntrl_0x09()
{
    uassert_int_equal(iscntrl(9), 1); /* iscntrl should be 1 for 0x09 */
}

void t_iscntrl_0x0a()
{
    uassert_int_equal(iscntrl(10), 1); /* iscntrl should be 1 for 0x0a */
}

void t_iscntrl_0x0b()
{
    uassert_int_equal(iscntrl(11), 1); /* iscntrl should be 1 for 0x0b */
}

void t_iscntrl_0x0c()
{
    uassert_int_equal(iscntrl(12), 1); /* iscntrl should be 1 for 0x0c */
}

void t_iscntrl_0x0d()
{
    uassert_int_equal(iscntrl(13), 1); /* iscntrl should be 1 for 0x0d */
}

void t_iscntrl_0x0e()
{
    uassert_int_equal(iscntrl(14), 1); /* iscntrl should be 1 for 0x0e */
}

void t_iscntrl_0x0f()
{
    uassert_int_equal(iscntrl(15), 1); /* iscntrl should be 1 for 0x0f */
}

void t_iscntrl_0x10()
{
    uassert_int_equal(iscntrl(16), 1); /* iscntrl should be 1 for 0x10 */
}

void t_iscntrl_0x11()
{
    uassert_int_equal(iscntrl(17), 1); /* iscntrl should be 1 for 0x11 */
}

void t_iscntrl_0x12()
{
    uassert_int_equal(iscntrl(18), 1); /* iscntrl should be 1 for 0x12 */
}

void t_iscntrl_0x13()
{
    uassert_int_equal(iscntrl(19), 1); /* iscntrl should be 1 for 0x13 */
}

void t_iscntrl_0x14()
{
    uassert_int_equal(iscntrl(20), 1); /* iscntrl should be 1 for 0x14 */
}

void t_iscntrl_0x15()
{
    uassert_int_equal(iscntrl(21), 1); /* iscntrl should be 1 for 0x15 */
}

void t_iscntrl_0x16()
{
    uassert_int_equal(iscntrl(22), 1); /* iscntrl should be 1 for 0x16 */
}

void t_iscntrl_0x17()
{
    uassert_int_equal(iscntrl(23), 1); /* iscntrl should be 1 for 0x17 */
}

void t_iscntrl_0x18()
{
    uassert_int_equal(iscntrl(24), 1); /* iscntrl should be 1 for 0x18 */
}

void t_iscntrl_0x19()
{
    uassert_int_equal(iscntrl(25), 1); /* iscntrl should be 1 for 0x19 */
}

void t_iscntrl_0x1a()
{
    uassert_int_equal(iscntrl(26), 1); /* iscntrl should be 1 for 0x1a */
}

void t_iscntrl_0x1b()
{
    uassert_int_equal(iscntrl(27), 1); /* iscntrl should be 1 for 0x1b */
}

void t_iscntrl_0x1c()
{
    uassert_int_equal(iscntrl(28), 1); /* iscntrl should be 1 for 0x1c */
}

void t_iscntrl_0x1d()
{
    uassert_int_equal(iscntrl(29), 1); /* iscntrl should be 1 for 0x1d */
}

void t_iscntrl_0x1e()
{
    uassert_int_equal(iscntrl(30), 1); /* iscntrl should be 1 for 0x1e */
}

void t_iscntrl_0x1f()
{
    uassert_int_equal(iscntrl(31), 1); /* iscntrl should be 1 for 0x1f */
}

void t_iscntrl_0x20()
{
    uassert_int_equal(iscntrl(32), 0); /* iscntrl should be 0 for   */
}

void t_iscntrl_0x21()
{
    uassert_int_equal(iscntrl(33), 0); /* iscntrl should be 0 for ! */
}

void t_iscntrl_0x22()
{
    uassert_int_equal(iscntrl(34), 0); /* iscntrl should be 0 for 0x22 */
}

void t_iscntrl_0x23()
{
    uassert_int_equal(iscntrl(35), 0); /* iscntrl should be 0 for # */
}

void t_iscntrl_0x24()
{
    uassert_int_equal(iscntrl(36), 0); /* iscntrl should be 0 for $ */
}

void t_iscntrl_0x25()
{
    uassert_int_equal(iscntrl(37), 0); /* iscntrl should be 0 for % */
}

void t_iscntrl_0x26()
{
    uassert_int_equal(iscntrl(38), 0); /* iscntrl should be 0 for & */
}

void t_iscntrl_0x27()
{
    uassert_int_equal(iscntrl(39), 0); /* iscntrl should be 0 for ' */
}

void t_iscntrl_0x28()
{
    uassert_int_equal(iscntrl(40), 0); /* iscntrl should be 0 for ( */
}

void t_iscntrl_0x29()
{
    uassert_int_equal(iscntrl(41), 0); /* iscntrl should be 0 for ) */
}

void t_iscntrl_0x2a()
{
    uassert_int_equal(iscntrl(42), 0); /* iscntrl should be 0 for * */
}

void t_iscntrl_0x2b()
{
    uassert_int_equal(iscntrl(43), 0); /* iscntrl should be 0 for + */
}

void t_iscntrl_0x2c()
{
    uassert_int_equal(iscntrl(44), 0); /* iscntrl should be 0 for , */
}

void t_iscntrl_0x2d()
{
    uassert_int_equal(iscntrl(45), 0); /* iscntrl should be 0 for - */
}

void t_iscntrl_0x2e()
{
    uassert_int_equal(iscntrl(46), 0); /* iscntrl should be 0 for . */
}

void t_iscntrl_0x2f()
{
    uassert_int_equal(iscntrl(47), 0); /* iscntrl should be 0 for / */
}

void t_iscntrl_0x30()
{
    uassert_int_equal(iscntrl(48), 0); /* iscntrl should be 0 for 0 */
}

void t_iscntrl_0x31()
{
    uassert_int_equal(iscntrl(49), 0); /* iscntrl should be 0 for 1 */
}

void t_iscntrl_0x32()
{
    uassert_int_equal(iscntrl(50), 0); /* iscntrl should be 0 for 2 */
}

void t_iscntrl_0x33()
{
    uassert_int_equal(iscntrl(51), 0); /* iscntrl should be 0 for 3 */
}

void t_iscntrl_0x34()
{
    uassert_int_equal(iscntrl(52), 0); /* iscntrl should be 0 for 4 */
}

void t_iscntrl_0x35()
{
    uassert_int_equal(iscntrl(53), 0); /* iscntrl should be 0 for 5 */
}

void t_iscntrl_0x36()
{
    uassert_int_equal(iscntrl(54), 0); /* iscntrl should be 0 for 6 */
}

void t_iscntrl_0x37()
{
    uassert_int_equal(iscntrl(55), 0); /* iscntrl should be 0 for 7 */
}

void t_iscntrl_0x38()
{
    uassert_int_equal(iscntrl(56), 0); /* iscntrl should be 0 for 8 */
}

void t_iscntrl_0x39()
{
    uassert_int_equal(iscntrl(57), 0); /* iscntrl should be 0 for 9 */
}

void t_iscntrl_0x3a()
{
    uassert_int_equal(iscntrl(58), 0); /* iscntrl should be 0 for : */
}

void t_iscntrl_0x3b()
{
    uassert_int_equal(iscntrl(59), 0); /* iscntrl should be 0 for ; */
}

void t_iscntrl_0x3c()
{
    uassert_int_equal(iscntrl(60), 0); /* iscntrl should be 0 for < */
}

void t_iscntrl_0x3d()
{
    uassert_int_equal(iscntrl(61), 0); /* iscntrl should be 0 for = */
}

void t_iscntrl_0x3e()
{
    uassert_int_equal(iscntrl(62), 0); /* iscntrl should be 0 for > */
}

void t_iscntrl_0x3f()
{
    uassert_int_equal(iscntrl(63), 0); /* iscntrl should be 0 for ? */
}

void t_iscntrl_0x40()
{
    uassert_int_equal(iscntrl(64), 0); /* iscntrl should be 0 for @ */
}

void t_iscntrl_0x41()
{
    uassert_int_equal(iscntrl(65), 0); /* iscntrl should be 0 for A */
}

void t_iscntrl_0x42()
{
    uassert_int_equal(iscntrl(66), 0); /* iscntrl should be 0 for B */
}

void t_iscntrl_0x43()
{
    uassert_int_equal(iscntrl(67), 0); /* iscntrl should be 0 for C */
}

void t_iscntrl_0x44()
{
    uassert_int_equal(iscntrl(68), 0); /* iscntrl should be 0 for D */
}

void t_iscntrl_0x45()
{
    uassert_int_equal(iscntrl(69), 0); /* iscntrl should be 0 for E */
}

void t_iscntrl_0x46()
{
    uassert_int_equal(iscntrl(70), 0); /* iscntrl should be 0 for F */
}

void t_iscntrl_0x47()
{
    uassert_int_equal(iscntrl(71), 0); /* iscntrl should be 0 for G */
}

void t_iscntrl_0x48()
{
    uassert_int_equal(iscntrl(72), 0); /* iscntrl should be 0 for H */
}

void t_iscntrl_0x49()
{
    uassert_int_equal(iscntrl(73), 0); /* iscntrl should be 0 for I */
}

void t_iscntrl_0x4a()
{
    uassert_int_equal(iscntrl(74), 0); /* iscntrl should be 0 for J */
}

void t_iscntrl_0x4b()
{
    uassert_int_equal(iscntrl(75), 0); /* iscntrl should be 0 for K */
}

void t_iscntrl_0x4c()
{
    uassert_int_equal(iscntrl(76), 0); /* iscntrl should be 0 for L */
}

void t_iscntrl_0x4d()
{
    uassert_int_equal(iscntrl(77), 0); /* iscntrl should be 0 for M */
}

void t_iscntrl_0x4e()
{
    uassert_int_equal(iscntrl(78), 0); /* iscntrl should be 0 for N */
}

void t_iscntrl_0x4f()
{
    uassert_int_equal(iscntrl(79), 0); /* iscntrl should be 0 for O */
}

void t_iscntrl_0x50()
{
    uassert_int_equal(iscntrl(80), 0); /* iscntrl should be 0 for P */
}

void t_iscntrl_0x51()
{
    uassert_int_equal(iscntrl(81), 0); /* iscntrl should be 0 for Q */
}

void t_iscntrl_0x52()
{
    uassert_int_equal(iscntrl(82), 0); /* iscntrl should be 0 for R */
}

void t_iscntrl_0x53()
{
    uassert_int_equal(iscntrl(83), 0); /* iscntrl should be 0 for S */
}

void t_iscntrl_0x54()
{
    uassert_int_equal(iscntrl(84), 0); /* iscntrl should be 0 for T */
}

void t_iscntrl_0x55()
{
    uassert_int_equal(iscntrl(85), 0); /* iscntrl should be 0 for U */
}

void t_iscntrl_0x56()
{
    uassert_int_equal(iscntrl(86), 0); /* iscntrl should be 0 for V */
}

void t_iscntrl_0x57()
{
    uassert_int_equal(iscntrl(87), 0); /* iscntrl should be 0 for W */
}

void t_iscntrl_0x58()
{
    uassert_int_equal(iscntrl(88), 0); /* iscntrl should be 0 for X */
}

void t_iscntrl_0x59()
{
    uassert_int_equal(iscntrl(89), 0); /* iscntrl should be 0 for Y */
}

void t_iscntrl_0x5a()
{
    uassert_int_equal(iscntrl(90), 0); /* iscntrl should be 0 for Z */
}

void t_iscntrl_0x5b()
{
    uassert_int_equal(iscntrl(91), 0); /* iscntrl should be 0 for [ */
}

void t_iscntrl_0x5c()
{
    uassert_int_equal(iscntrl(92), 0); /* iscntrl should be 0 for 0x5c */
}

void t_iscntrl_0x5d()
{
    uassert_int_equal(iscntrl(93), 0); /* iscntrl should be 0 for ] */
}

void t_iscntrl_0x5e()
{
    uassert_int_equal(iscntrl(94), 0); /* iscntrl should be 0 for ^ */
}

void t_iscntrl_0x5f()
{
    uassert_int_equal(iscntrl(95), 0); /* iscntrl should be 0 for _ */
}

void t_iscntrl_0x60()
{
    uassert_int_equal(iscntrl(96), 0); /* iscntrl should be 0 for ` */
}

void t_iscntrl_0x61()
{
    uassert_int_equal(iscntrl(97), 0); /* iscntrl should be 0 for a */
}

void t_iscntrl_0x62()
{
    uassert_int_equal(iscntrl(98), 0); /* iscntrl should be 0 for b */
}

void t_iscntrl_0x63()
{
    uassert_int_equal(iscntrl(99), 0); /* iscntrl should be 0 for c */
}

void t_iscntrl_0x64()
{
    uassert_int_equal(iscntrl(100), 0); /* iscntrl should be 0 for d */
}

void t_iscntrl_0x65()
{
    uassert_int_equal(iscntrl(101), 0); /* iscntrl should be 0 for e */
}

void t_iscntrl_0x66()
{
    uassert_int_equal(iscntrl(102), 0); /* iscntrl should be 0 for f */
}

void t_iscntrl_0x67()
{
    uassert_int_equal(iscntrl(103), 0); /* iscntrl should be 0 for g */
}

void t_iscntrl_0x68()
{
    uassert_int_equal(iscntrl(104), 0); /* iscntrl should be 0 for h */
}

void t_iscntrl_0x69()
{
    uassert_int_equal(iscntrl(105), 0); /* iscntrl should be 0 for i */
}

void t_iscntrl_0x6a()
{
    uassert_int_equal(iscntrl(106), 0); /* iscntrl should be 0 for j */
}

void t_iscntrl_0x6b()
{
    uassert_int_equal(iscntrl(107), 0); /* iscntrl should be 0 for k */
}

void t_iscntrl_0x6c()
{
    uassert_int_equal(iscntrl(108), 0); /* iscntrl should be 0 for l */
}

void t_iscntrl_0x6d()
{
    uassert_int_equal(iscntrl(109), 0); /* iscntrl should be 0 for m */
}

void t_iscntrl_0x6e()
{
    uassert_int_equal(iscntrl(110), 0); /* iscntrl should be 0 for n */
}

void t_iscntrl_0x6f()
{
    uassert_int_equal(iscntrl(111), 0); /* iscntrl should be 0 for o */
}

void t_iscntrl_0x70()
{
    uassert_int_equal(iscntrl(112), 0); /* iscntrl should be 0 for p */
}

void t_iscntrl_0x71()
{
    uassert_int_equal(iscntrl(113), 0); /* iscntrl should be 0 for q */
}

void t_iscntrl_0x72()
{
    uassert_int_equal(iscntrl(114), 0); /* iscntrl should be 0 for r */
}

void t_iscntrl_0x73()
{
    uassert_int_equal(iscntrl(115), 0); /* iscntrl should be 0 for s */
}

void t_iscntrl_0x74()
{
    uassert_int_equal(iscntrl(116), 0); /* iscntrl should be 0 for t */
}

void t_iscntrl_0x75()
{
    uassert_int_equal(iscntrl(117), 0); /* iscntrl should be 0 for u */
}

void t_iscntrl_0x76()
{
    uassert_int_equal(iscntrl(118), 0); /* iscntrl should be 0 for v */
}

void t_iscntrl_0x77()
{
    uassert_int_equal(iscntrl(119), 0); /* iscntrl should be 0 for w */
}

void t_iscntrl_0x78()
{
    uassert_int_equal(iscntrl(120), 0); /* iscntrl should be 0 for x */
}

void t_iscntrl_0x79()
{
    uassert_int_equal(iscntrl(121), 0); /* iscntrl should be 0 for y */
}

void t_iscntrl_0x7a()
{
    uassert_int_equal(iscntrl(122), 0); /* iscntrl should be 0 for z */
}

void t_iscntrl_0x7b()
{
    uassert_int_equal(iscntrl(123), 0); /* iscntrl should be 0 for { */
}

void t_iscntrl_0x7c()
{
    uassert_int_equal(iscntrl(124), 0); /* iscntrl should be 0 for | */
}

void t_iscntrl_0x7d()
{
    uassert_int_equal(iscntrl(125), 0); /* iscntrl should be 0 for } */
}

void t_iscntrl_0x7e()
{
    uassert_int_equal(iscntrl(126), 0); /* iscntrl should be 0 for ~ */
}

void t_iscntrl_0x7f()
{
    uassert_int_equal(iscntrl(127), 1); /* iscntrl should be 1 for 0x7f */
}

void t_iscntrl_0x80()
{
    uassert_int_equal(iscntrl(128), 0); /* iscntrl should be 0 for 0x80 */
}

void t_iscntrl_0x81()
{
    uassert_int_equal(iscntrl(129), 0); /* iscntrl should be 0 for 0x81 */
}

void t_iscntrl_0x82()
{
    uassert_int_equal(iscntrl(130), 0); /* iscntrl should be 0 for 0x82 */
}

void t_iscntrl_0x83()
{
    uassert_int_equal(iscntrl(131), 0); /* iscntrl should be 0 for 0x83 */
}

void t_iscntrl_0x84()
{
    uassert_int_equal(iscntrl(132), 0); /* iscntrl should be 0 for 0x84 */
}

void t_iscntrl_0x85()
{
    uassert_int_equal(iscntrl(133), 0); /* iscntrl should be 0 for 0x85 */
}

void t_iscntrl_0x86()
{
    uassert_int_equal(iscntrl(134), 0); /* iscntrl should be 0 for 0x86 */
}

void t_iscntrl_0x87()
{
    uassert_int_equal(iscntrl(135), 0); /* iscntrl should be 0 for 0x87 */
}

void t_iscntrl_0x88()
{
    uassert_int_equal(iscntrl(136), 0); /* iscntrl should be 0 for 0x88 */
}

void t_iscntrl_0x89()
{
    uassert_int_equal(iscntrl(137), 0); /* iscntrl should be 0 for 0x89 */
}

void t_iscntrl_0x8a()
{
    uassert_int_equal(iscntrl(138), 0); /* iscntrl should be 0 for 0x8a */
}

void t_iscntrl_0x8b()
{
    uassert_int_equal(iscntrl(139), 0); /* iscntrl should be 0 for 0x8b */
}

void t_iscntrl_0x8c()
{
    uassert_int_equal(iscntrl(140), 0); /* iscntrl should be 0 for 0x8c */
}

void t_iscntrl_0x8d()
{
    uassert_int_equal(iscntrl(141), 0); /* iscntrl should be 0 for 0x8d */
}

void t_iscntrl_0x8e()
{
    uassert_int_equal(iscntrl(142), 0); /* iscntrl should be 0 for 0x8e */
}

void t_iscntrl_0x8f()
{
    uassert_int_equal(iscntrl(143), 0); /* iscntrl should be 0 for 0x8f */
}

void t_iscntrl_0x90()
{
    uassert_int_equal(iscntrl(144), 0); /* iscntrl should be 0 for 0x90 */
}

void t_iscntrl_0x91()
{
    uassert_int_equal(iscntrl(145), 0); /* iscntrl should be 0 for 0x91 */
}

void t_iscntrl_0x92()
{
    uassert_int_equal(iscntrl(146), 0); /* iscntrl should be 0 for 0x92 */
}

void t_iscntrl_0x93()
{
    uassert_int_equal(iscntrl(147), 0); /* iscntrl should be 0 for 0x93 */
}

void t_iscntrl_0x94()
{
    uassert_int_equal(iscntrl(148), 0); /* iscntrl should be 0 for 0x94 */
}

void t_iscntrl_0x95()
{
    uassert_int_equal(iscntrl(149), 0); /* iscntrl should be 0 for 0x95 */
}

void t_iscntrl_0x96()
{
    uassert_int_equal(iscntrl(150), 0); /* iscntrl should be 0 for 0x96 */
}

void t_iscntrl_0x97()
{
    uassert_int_equal(iscntrl(151), 0); /* iscntrl should be 0 for 0x97 */
}

void t_iscntrl_0x98()
{
    uassert_int_equal(iscntrl(152), 0); /* iscntrl should be 0 for 0x98 */
}

void t_iscntrl_0x99()
{
    uassert_int_equal(iscntrl(153), 0); /* iscntrl should be 0 for 0x99 */
}

void t_iscntrl_0x9a()
{
    uassert_int_equal(iscntrl(154), 0); /* iscntrl should be 0 for 0x9a */
}

void t_iscntrl_0x9b()
{
    uassert_int_equal(iscntrl(155), 0); /* iscntrl should be 0 for 0x9b */
}

void t_iscntrl_0x9c()
{
    uassert_int_equal(iscntrl(156), 0); /* iscntrl should be 0 for 0x9c */
}

void t_iscntrl_0x9d()
{
    uassert_int_equal(iscntrl(157), 0); /* iscntrl should be 0 for 0x9d */
}

void t_iscntrl_0x9e()
{
    uassert_int_equal(iscntrl(158), 0); /* iscntrl should be 0 for 0x9e */
}

void t_iscntrl_0x9f()
{
    uassert_int_equal(iscntrl(159), 0); /* iscntrl should be 0 for 0x9f */
}

void t_iscntrl_0xa0()
{
    uassert_int_equal(iscntrl(160), 0); /* iscntrl should be 0 for 0xa0 */
}

void t_iscntrl_0xa1()
{
    uassert_int_equal(iscntrl(161), 0); /* iscntrl should be 0 for 0xa1 */
}

void t_iscntrl_0xa2()
{
    uassert_int_equal(iscntrl(162), 0); /* iscntrl should be 0 for 0xa2 */
}

void t_iscntrl_0xa3()
{
    uassert_int_equal(iscntrl(163), 0); /* iscntrl should be 0 for 0xa3 */
}

void t_iscntrl_0xa4()
{
    uassert_int_equal(iscntrl(164), 0); /* iscntrl should be 0 for 0xa4 */
}

void t_iscntrl_0xa5()
{
    uassert_int_equal(iscntrl(165), 0); /* iscntrl should be 0 for 0xa5 */
}

void t_iscntrl_0xa6()
{
    uassert_int_equal(iscntrl(166), 0); /* iscntrl should be 0 for 0xa6 */
}

void t_iscntrl_0xa7()
{
    uassert_int_equal(iscntrl(167), 0); /* iscntrl should be 0 for 0xa7 */
}

void t_iscntrl_0xa8()
{
    uassert_int_equal(iscntrl(168), 0); /* iscntrl should be 0 for 0xa8 */
}

void t_iscntrl_0xa9()
{
    uassert_int_equal(iscntrl(169), 0); /* iscntrl should be 0 for 0xa9 */
}

void t_iscntrl_0xaa()
{
    uassert_int_equal(iscntrl(170), 0); /* iscntrl should be 0 for 0xaa */
}

void t_iscntrl_0xab()
{
    uassert_int_equal(iscntrl(171), 0); /* iscntrl should be 0 for 0xab */
}

void t_iscntrl_0xac()
{
    uassert_int_equal(iscntrl(172), 0); /* iscntrl should be 0 for 0xac */
}

void t_iscntrl_0xad()
{
    uassert_int_equal(iscntrl(173), 0); /* iscntrl should be 0 for 0xad */
}

void t_iscntrl_0xae()
{
    uassert_int_equal(iscntrl(174), 0); /* iscntrl should be 0 for 0xae */
}

void t_iscntrl_0xaf()
{
    uassert_int_equal(iscntrl(175), 0); /* iscntrl should be 0 for 0xaf */
}

void t_iscntrl_0xb0()
{
    uassert_int_equal(iscntrl(176), 0); /* iscntrl should be 0 for 0xb0 */
}

void t_iscntrl_0xb1()
{
    uassert_int_equal(iscntrl(177), 0); /* iscntrl should be 0 for 0xb1 */
}

void t_iscntrl_0xb2()
{
    uassert_int_equal(iscntrl(178), 0); /* iscntrl should be 0 for 0xb2 */
}

void t_iscntrl_0xb3()
{
    uassert_int_equal(iscntrl(179), 0); /* iscntrl should be 0 for 0xb3 */
}

void t_iscntrl_0xb4()
{
    uassert_int_equal(iscntrl(180), 0); /* iscntrl should be 0 for 0xb4 */
}

void t_iscntrl_0xb5()
{
    uassert_int_equal(iscntrl(181), 0); /* iscntrl should be 0 for 0xb5 */
}

void t_iscntrl_0xb6()
{
    uassert_int_equal(iscntrl(182), 0); /* iscntrl should be 0 for 0xb6 */
}

void t_iscntrl_0xb7()
{
    uassert_int_equal(iscntrl(183), 0); /* iscntrl should be 0 for 0xb7 */
}

void t_iscntrl_0xb8()
{
    uassert_int_equal(iscntrl(184), 0); /* iscntrl should be 0 for 0xb8 */
}

void t_iscntrl_0xb9()
{
    uassert_int_equal(iscntrl(185), 0); /* iscntrl should be 0 for 0xb9 */
}

void t_iscntrl_0xba()
{
    uassert_int_equal(iscntrl(186), 0); /* iscntrl should be 0 for 0xba */
}

void t_iscntrl_0xbb()
{
    uassert_int_equal(iscntrl(187), 0); /* iscntrl should be 0 for 0xbb */
}

void t_iscntrl_0xbc()
{
    uassert_int_equal(iscntrl(188), 0); /* iscntrl should be 0 for 0xbc */
}

void t_iscntrl_0xbd()
{
    uassert_int_equal(iscntrl(189), 0); /* iscntrl should be 0 for 0xbd */
}

void t_iscntrl_0xbe()
{
    uassert_int_equal(iscntrl(190), 0); /* iscntrl should be 0 for 0xbe */
}

void t_iscntrl_0xbf()
{
    uassert_int_equal(iscntrl(191), 0); /* iscntrl should be 0 for 0xbf */
}

void t_iscntrl_0xc0()
{
    uassert_int_equal(iscntrl(192), 0); /* iscntrl should be 0 for 0xc0 */
}

void t_iscntrl_0xc1()
{
    uassert_int_equal(iscntrl(193), 0); /* iscntrl should be 0 for 0xc1 */
}

void t_iscntrl_0xc2()
{
    uassert_int_equal(iscntrl(194), 0); /* iscntrl should be 0 for 0xc2 */
}

void t_iscntrl_0xc3()
{
    uassert_int_equal(iscntrl(195), 0); /* iscntrl should be 0 for 0xc3 */
}

void t_iscntrl_0xc4()
{
    uassert_int_equal(iscntrl(196), 0); /* iscntrl should be 0 for 0xc4 */
}

void t_iscntrl_0xc5()
{
    uassert_int_equal(iscntrl(197), 0); /* iscntrl should be 0 for 0xc5 */
}

void t_iscntrl_0xc6()
{
    uassert_int_equal(iscntrl(198), 0); /* iscntrl should be 0 for 0xc6 */
}

void t_iscntrl_0xc7()
{
    uassert_int_equal(iscntrl(199), 0); /* iscntrl should be 0 for 0xc7 */
}

void t_iscntrl_0xc8()
{
    uassert_int_equal(iscntrl(200), 0); /* iscntrl should be 0 for 0xc8 */
}

void t_iscntrl_0xc9()
{
    uassert_int_equal(iscntrl(201), 0); /* iscntrl should be 0 for 0xc9 */
}

void t_iscntrl_0xca()
{
    uassert_int_equal(iscntrl(202), 0); /* iscntrl should be 0 for 0xca */
}

void t_iscntrl_0xcb()
{
    uassert_int_equal(iscntrl(203), 0); /* iscntrl should be 0 for 0xcb */
}

void t_iscntrl_0xcc()
{
    uassert_int_equal(iscntrl(204), 0); /* iscntrl should be 0 for 0xcc */
}

void t_iscntrl_0xcd()
{
    uassert_int_equal(iscntrl(205), 0); /* iscntrl should be 0 for 0xcd */
}

void t_iscntrl_0xce()
{
    uassert_int_equal(iscntrl(206), 0); /* iscntrl should be 0 for 0xce */
}

void t_iscntrl_0xcf()
{
    uassert_int_equal(iscntrl(207), 0); /* iscntrl should be 0 for 0xcf */
}

void t_iscntrl_0xd0()
{
    uassert_int_equal(iscntrl(208), 0); /* iscntrl should be 0 for 0xd0 */
}

void t_iscntrl_0xd1()
{
    uassert_int_equal(iscntrl(209), 0); /* iscntrl should be 0 for 0xd1 */
}

void t_iscntrl_0xd2()
{
    uassert_int_equal(iscntrl(210), 0); /* iscntrl should be 0 for 0xd2 */
}

void t_iscntrl_0xd3()
{
    uassert_int_equal(iscntrl(211), 0); /* iscntrl should be 0 for 0xd3 */
}

void t_iscntrl_0xd4()
{
    uassert_int_equal(iscntrl(212), 0); /* iscntrl should be 0 for 0xd4 */
}

void t_iscntrl_0xd5()
{
    uassert_int_equal(iscntrl(213), 0); /* iscntrl should be 0 for 0xd5 */
}

void t_iscntrl_0xd6()
{
    uassert_int_equal(iscntrl(214), 0); /* iscntrl should be 0 for 0xd6 */
}

void t_iscntrl_0xd7()
{
    uassert_int_equal(iscntrl(215), 0); /* iscntrl should be 0 for 0xd7 */
}

void t_iscntrl_0xd8()
{
    uassert_int_equal(iscntrl(216), 0); /* iscntrl should be 0 for 0xd8 */
}

void t_iscntrl_0xd9()
{
    uassert_int_equal(iscntrl(217), 0); /* iscntrl should be 0 for 0xd9 */
}

void t_iscntrl_0xda()
{
    uassert_int_equal(iscntrl(218), 0); /* iscntrl should be 0 for 0xda */
}

void t_iscntrl_0xdb()
{
    uassert_int_equal(iscntrl(219), 0); /* iscntrl should be 0 for 0xdb */
}

void t_iscntrl_0xdc()
{
    uassert_int_equal(iscntrl(220), 0); /* iscntrl should be 0 for 0xdc */
}

void t_iscntrl_0xdd()
{
    uassert_int_equal(iscntrl(221), 0); /* iscntrl should be 0 for 0xdd */
}

void t_iscntrl_0xde()
{
    uassert_int_equal(iscntrl(222), 0); /* iscntrl should be 0 for 0xde */
}

void t_iscntrl_0xdf()
{
    uassert_int_equal(iscntrl(223), 0); /* iscntrl should be 0 for 0xdf */
}

void t_iscntrl_0xe0()
{
    uassert_int_equal(iscntrl(224), 0); /* iscntrl should be 0 for 0xe0 */
}

void t_iscntrl_0xe1()
{
    uassert_int_equal(iscntrl(225), 0); /* iscntrl should be 0 for 0xe1 */
}

void t_iscntrl_0xe2()
{
    uassert_int_equal(iscntrl(226), 0); /* iscntrl should be 0 for 0xe2 */
}

void t_iscntrl_0xe3()
{
    uassert_int_equal(iscntrl(227), 0); /* iscntrl should be 0 for 0xe3 */
}

void t_iscntrl_0xe4()
{
    uassert_int_equal(iscntrl(228), 0); /* iscntrl should be 0 for 0xe4 */
}

void t_iscntrl_0xe5()
{
    uassert_int_equal(iscntrl(229), 0); /* iscntrl should be 0 for 0xe5 */
}

void t_iscntrl_0xe6()
{
    uassert_int_equal(iscntrl(230), 0); /* iscntrl should be 0 for 0xe6 */
}

void t_iscntrl_0xe7()
{
    uassert_int_equal(iscntrl(231), 0); /* iscntrl should be 0 for 0xe7 */
}

void t_iscntrl_0xe8()
{
    uassert_int_equal(iscntrl(232), 0); /* iscntrl should be 0 for 0xe8 */
}

void t_iscntrl_0xe9()
{
    uassert_int_equal(iscntrl(233), 0); /* iscntrl should be 0 for 0xe9 */
}

void t_iscntrl_0xea()
{
    uassert_int_equal(iscntrl(234), 0); /* iscntrl should be 0 for 0xea */
}

void t_iscntrl_0xeb()
{
    uassert_int_equal(iscntrl(235), 0); /* iscntrl should be 0 for 0xeb */
}

void t_iscntrl_0xec()
{
    uassert_int_equal(iscntrl(236), 0); /* iscntrl should be 0 for 0xec */
}

void t_iscntrl_0xed()
{
    uassert_int_equal(iscntrl(237), 0); /* iscntrl should be 0 for 0xed */
}

void t_iscntrl_0xee()
{
    uassert_int_equal(iscntrl(238), 0); /* iscntrl should be 0 for 0xee */
}

void t_iscntrl_0xef()
{
    uassert_int_equal(iscntrl(239), 0); /* iscntrl should be 0 for 0xef */
}

void t_iscntrl_0xf0()
{
    uassert_int_equal(iscntrl(240), 0); /* iscntrl should be 0 for 0xf0 */
}

void t_iscntrl_0xf1()
{
    uassert_int_equal(iscntrl(241), 0); /* iscntrl should be 0 for 0xf1 */
}

void t_iscntrl_0xf2()
{
    uassert_int_equal(iscntrl(242), 0); /* iscntrl should be 0 for 0xf2 */
}

void t_iscntrl_0xf3()
{
    uassert_int_equal(iscntrl(243), 0); /* iscntrl should be 0 for 0xf3 */
}

void t_iscntrl_0xf4()
{
    uassert_int_equal(iscntrl(244), 0); /* iscntrl should be 0 for 0xf4 */
}

void t_iscntrl_0xf5()
{
    uassert_int_equal(iscntrl(245), 0); /* iscntrl should be 0 for 0xf5 */
}

void t_iscntrl_0xf6()
{
    uassert_int_equal(iscntrl(246), 0); /* iscntrl should be 0 for 0xf6 */
}

void t_iscntrl_0xf7()
{
    uassert_int_equal(iscntrl(247), 0); /* iscntrl should be 0 for 0xf7 */
}

void t_iscntrl_0xf8()
{
    uassert_int_equal(iscntrl(248), 0); /* iscntrl should be 0 for 0xf8 */
}

void t_iscntrl_0xf9()
{
    uassert_int_equal(iscntrl(249), 0); /* iscntrl should be 0 for 0xf9 */
}

void t_iscntrl_0xfa()
{
    uassert_int_equal(iscntrl(250), 0); /* iscntrl should be 0 for 0xfa */
}

void t_iscntrl_0xfb()
{
    uassert_int_equal(iscntrl(251), 0); /* iscntrl should be 0 for 0xfb */
}

void t_iscntrl_0xfc()
{
    uassert_int_equal(iscntrl(252), 0); /* iscntrl should be 0 for 0xfc */
}

void t_iscntrl_0xfd()
{
    uassert_int_equal(iscntrl(253), 0); /* iscntrl should be 0 for 0xfd */
}

void t_iscntrl_0xfe()
{
    uassert_int_equal(iscntrl(254), 0); /* iscntrl should be 0 for 0xfe */
}

void t_iscntrl_0xff()
{
    uassert_int_equal(iscntrl(255), 0); /* iscntrl should be 0 for 0xff */
}



static int testcase(void)
{
    t_iscntrl_0x00();
    t_iscntrl_0x01();
    t_iscntrl_0x02();
    t_iscntrl_0x03();
    t_iscntrl_0x04();
    t_iscntrl_0x05();
    t_iscntrl_0x06();
    t_iscntrl_0x07();
    t_iscntrl_0x08();
    t_iscntrl_0x09();
    t_iscntrl_0x0a();
    t_iscntrl_0x0b();
    t_iscntrl_0x0c();
    t_iscntrl_0x0d();
    t_iscntrl_0x0e();
    t_iscntrl_0x0f();
    t_iscntrl_0x10();
    t_iscntrl_0x11();
    t_iscntrl_0x12();
    t_iscntrl_0x13();
    t_iscntrl_0x14();
    t_iscntrl_0x15();
    t_iscntrl_0x16();
    t_iscntrl_0x17();
    t_iscntrl_0x18();
    t_iscntrl_0x19();
    t_iscntrl_0x1a();
    t_iscntrl_0x1b();
    t_iscntrl_0x1c();
    t_iscntrl_0x1d();
    t_iscntrl_0x1e();
    t_iscntrl_0x1f();
    t_iscntrl_0x20();
    t_iscntrl_0x21();
    t_iscntrl_0x22();
    t_iscntrl_0x23();
    t_iscntrl_0x24();
    t_iscntrl_0x25();
    t_iscntrl_0x26();
    t_iscntrl_0x27();
    t_iscntrl_0x28();
    t_iscntrl_0x29();
    t_iscntrl_0x2a();
    t_iscntrl_0x2b();
    t_iscntrl_0x2c();
    t_iscntrl_0x2d();
    t_iscntrl_0x2e();
    t_iscntrl_0x2f();
    t_iscntrl_0x30();
    t_iscntrl_0x31();
    t_iscntrl_0x32();
    t_iscntrl_0x33();
    t_iscntrl_0x34();
    t_iscntrl_0x35();
    t_iscntrl_0x36();
    t_iscntrl_0x37();
    t_iscntrl_0x38();
    t_iscntrl_0x39();
    t_iscntrl_0x3a();
    t_iscntrl_0x3b();
    t_iscntrl_0x3c();
    t_iscntrl_0x3d();
    t_iscntrl_0x3e();
    t_iscntrl_0x3f();
    t_iscntrl_0x40();
    t_iscntrl_0x41();
    t_iscntrl_0x42();
    t_iscntrl_0x43();
    t_iscntrl_0x44();
    t_iscntrl_0x45();
    t_iscntrl_0x46();
    t_iscntrl_0x47();
    t_iscntrl_0x48();
    t_iscntrl_0x49();
    t_iscntrl_0x4a();
    t_iscntrl_0x4b();
    t_iscntrl_0x4c();
    t_iscntrl_0x4d();
    t_iscntrl_0x4e();
    t_iscntrl_0x4f();
    t_iscntrl_0x50();
    t_iscntrl_0x51();
    t_iscntrl_0x52();
    t_iscntrl_0x53();
    t_iscntrl_0x54();
    t_iscntrl_0x55();
    t_iscntrl_0x56();
    t_iscntrl_0x57();
    t_iscntrl_0x58();
    t_iscntrl_0x59();
    t_iscntrl_0x5a();
    t_iscntrl_0x5b();
    t_iscntrl_0x5c();
    t_iscntrl_0x5d();
    t_iscntrl_0x5e();
    t_iscntrl_0x5f();
    t_iscntrl_0x60();
    t_iscntrl_0x61();
    t_iscntrl_0x62();
    t_iscntrl_0x63();
    t_iscntrl_0x64();
    t_iscntrl_0x65();
    t_iscntrl_0x66();
    t_iscntrl_0x67();
    t_iscntrl_0x68();
    t_iscntrl_0x69();
    t_iscntrl_0x6a();
    t_iscntrl_0x6b();
    t_iscntrl_0x6c();
    t_iscntrl_0x6d();
    t_iscntrl_0x6e();
    t_iscntrl_0x6f();
    t_iscntrl_0x70();
    t_iscntrl_0x71();
    t_iscntrl_0x72();
    t_iscntrl_0x73();
    t_iscntrl_0x74();
    t_iscntrl_0x75();
    t_iscntrl_0x76();
    t_iscntrl_0x77();
    t_iscntrl_0x78();
    t_iscntrl_0x79();
    t_iscntrl_0x7a();
    t_iscntrl_0x7b();
    t_iscntrl_0x7c();
    t_iscntrl_0x7d();
    t_iscntrl_0x7e();
    t_iscntrl_0x7f();
    t_iscntrl_0x80();
    t_iscntrl_0x81();
    t_iscntrl_0x82();
    t_iscntrl_0x83();
    t_iscntrl_0x84();
    t_iscntrl_0x85();
    t_iscntrl_0x86();
    t_iscntrl_0x87();
    t_iscntrl_0x88();
    t_iscntrl_0x89();
    t_iscntrl_0x8a();
    t_iscntrl_0x8b();
    t_iscntrl_0x8c();
    t_iscntrl_0x8d();
    t_iscntrl_0x8e();
    t_iscntrl_0x8f();
    t_iscntrl_0x90();
    t_iscntrl_0x91();
    t_iscntrl_0x92();
    t_iscntrl_0x93();
    t_iscntrl_0x94();
    t_iscntrl_0x95();
    t_iscntrl_0x96();
    t_iscntrl_0x97();
    t_iscntrl_0x98();
    t_iscntrl_0x99();
    t_iscntrl_0x9a();
    t_iscntrl_0x9b();
    t_iscntrl_0x9c();
    t_iscntrl_0x9d();
    t_iscntrl_0x9e();
    t_iscntrl_0x9f();
    t_iscntrl_0xa0();
    t_iscntrl_0xa1();
    t_iscntrl_0xa2();
    t_iscntrl_0xa3();
    t_iscntrl_0xa4();
    t_iscntrl_0xa5();
    t_iscntrl_0xa6();
    t_iscntrl_0xa7();
    t_iscntrl_0xa8();
    t_iscntrl_0xa9();
    t_iscntrl_0xaa();
    t_iscntrl_0xab();
    t_iscntrl_0xac();
    t_iscntrl_0xad();
    t_iscntrl_0xae();
    t_iscntrl_0xaf();
    t_iscntrl_0xb0();
    t_iscntrl_0xb1();
    t_iscntrl_0xb2();
    t_iscntrl_0xb3();
    t_iscntrl_0xb4();
    t_iscntrl_0xb5();
    t_iscntrl_0xb6();
    t_iscntrl_0xb7();
    t_iscntrl_0xb8();
    t_iscntrl_0xb9();
    t_iscntrl_0xba();
    t_iscntrl_0xbb();
    t_iscntrl_0xbc();
    t_iscntrl_0xbd();
    t_iscntrl_0xbe();
    t_iscntrl_0xbf();
    t_iscntrl_0xc0();
    t_iscntrl_0xc1();
    t_iscntrl_0xc2();
    t_iscntrl_0xc3();
    t_iscntrl_0xc4();
    t_iscntrl_0xc5();
    t_iscntrl_0xc6();
    t_iscntrl_0xc7();
    t_iscntrl_0xc8();
    t_iscntrl_0xc9();
    t_iscntrl_0xca();
    t_iscntrl_0xcb();
    t_iscntrl_0xcc();
    t_iscntrl_0xcd();
    t_iscntrl_0xce();
    t_iscntrl_0xcf();
    t_iscntrl_0xd0();
    t_iscntrl_0xd1();
    t_iscntrl_0xd2();
    t_iscntrl_0xd3();
    t_iscntrl_0xd4();
    t_iscntrl_0xd5();
    t_iscntrl_0xd6();
    t_iscntrl_0xd7();
    t_iscntrl_0xd8();
    t_iscntrl_0xd9();
    t_iscntrl_0xda();
    t_iscntrl_0xdb();
    t_iscntrl_0xdc();
    t_iscntrl_0xdd();
    t_iscntrl_0xde();
    t_iscntrl_0xdf();
    t_iscntrl_0xe0();
    t_iscntrl_0xe1();
    t_iscntrl_0xe2();
    t_iscntrl_0xe3();
    t_iscntrl_0xe4();
    t_iscntrl_0xe5();
    t_iscntrl_0xe6();
    t_iscntrl_0xe7();
    t_iscntrl_0xe8();
    t_iscntrl_0xe9();
    t_iscntrl_0xea();
    t_iscntrl_0xeb();
    t_iscntrl_0xec();
    t_iscntrl_0xed();
    t_iscntrl_0xee();
    t_iscntrl_0xef();
    t_iscntrl_0xf0();
    t_iscntrl_0xf1();
    t_iscntrl_0xf2();
    t_iscntrl_0xf3();
    t_iscntrl_0xf4();
    t_iscntrl_0xf5();
    t_iscntrl_0xf6();
    t_iscntrl_0xf7();
    t_iscntrl_0xf8();
    t_iscntrl_0xf9();
    t_iscntrl_0xfa();
    t_iscntrl_0xfb();
    t_iscntrl_0xfc();
    t_iscntrl_0xfd();
    t_iscntrl_0xfe();
    t_iscntrl_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
