#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))


void t_isxdigit_0x00()
{
    uassert_int_equal(isxdigit(0), 0); /* isxdigit should be 0 for 0x00 */
}

void t_isxdigit_0x01()
{
    uassert_int_equal(isxdigit(1), 0); /* isxdigit should be 0 for 0x01 */
}

void t_isxdigit_0x02()
{
    uassert_int_equal(isxdigit(2), 0); /* isxdigit should be 0 for 0x02 */
}

void t_isxdigit_0x03()
{
    uassert_int_equal(isxdigit(3), 0); /* isxdigit should be 0 for 0x03 */
}

void t_isxdigit_0x04()
{
    uassert_int_equal(isxdigit(4), 0); /* isxdigit should be 0 for 0x04 */
}

void t_isxdigit_0x05()
{
    uassert_int_equal(isxdigit(5), 0); /* isxdigit should be 0 for 0x05 */
}

void t_isxdigit_0x06()
{
    uassert_int_equal(isxdigit(6), 0); /* isxdigit should be 0 for 0x06 */
}

void t_isxdigit_0x07()
{
    uassert_int_equal(isxdigit(7), 0); /* isxdigit should be 0 for 0x07 */
}

void t_isxdigit_0x08()
{
    uassert_int_equal(isxdigit(8), 0); /* isxdigit should be 0 for 0x08 */
}

void t_isxdigit_0x09()
{
    uassert_int_equal(isxdigit(9), 0); /* isxdigit should be 0 for 0x09 */
}

void t_isxdigit_0x0a()
{
    uassert_int_equal(isxdigit(10), 0); /* isxdigit should be 0 for 0x0a */
}

void t_isxdigit_0x0b()
{
    uassert_int_equal(isxdigit(11), 0); /* isxdigit should be 0 for 0x0b */
}

void t_isxdigit_0x0c()
{
    uassert_int_equal(isxdigit(12), 0); /* isxdigit should be 0 for 0x0c */
}

void t_isxdigit_0x0d()
{
    uassert_int_equal(isxdigit(13), 0); /* isxdigit should be 0 for 0x0d */
}

void t_isxdigit_0x0e()
{
    uassert_int_equal(isxdigit(14), 0); /* isxdigit should be 0 for 0x0e */
}

void t_isxdigit_0x0f()
{
    uassert_int_equal(isxdigit(15), 0); /* isxdigit should be 0 for 0x0f */
}

void t_isxdigit_0x10()
{
    uassert_int_equal(isxdigit(16), 0); /* isxdigit should be 0 for 0x10 */
}

void t_isxdigit_0x11()
{
    uassert_int_equal(isxdigit(17), 0); /* isxdigit should be 0 for 0x11 */
}

void t_isxdigit_0x12()
{
    uassert_int_equal(isxdigit(18), 0); /* isxdigit should be 0 for 0x12 */
}

void t_isxdigit_0x13()
{
    uassert_int_equal(isxdigit(19), 0); /* isxdigit should be 0 for 0x13 */
}

void t_isxdigit_0x14()
{
    uassert_int_equal(isxdigit(20), 0); /* isxdigit should be 0 for 0x14 */
}

void t_isxdigit_0x15()
{
    uassert_int_equal(isxdigit(21), 0); /* isxdigit should be 0 for 0x15 */
}

void t_isxdigit_0x16()
{
    uassert_int_equal(isxdigit(22), 0); /* isxdigit should be 0 for 0x16 */
}

void t_isxdigit_0x17()
{
    uassert_int_equal(isxdigit(23), 0); /* isxdigit should be 0 for 0x17 */
}

void t_isxdigit_0x18()
{
    uassert_int_equal(isxdigit(24), 0); /* isxdigit should be 0 for 0x18 */
}

void t_isxdigit_0x19()
{
    uassert_int_equal(isxdigit(25), 0); /* isxdigit should be 0 for 0x19 */
}

void t_isxdigit_0x1a()
{
    uassert_int_equal(isxdigit(26), 0); /* isxdigit should be 0 for 0x1a */
}

void t_isxdigit_0x1b()
{
    uassert_int_equal(isxdigit(27), 0); /* isxdigit should be 0 for 0x1b */
}

void t_isxdigit_0x1c()
{
    uassert_int_equal(isxdigit(28), 0); /* isxdigit should be 0 for 0x1c */
}

void t_isxdigit_0x1d()
{
    uassert_int_equal(isxdigit(29), 0); /* isxdigit should be 0 for 0x1d */
}

void t_isxdigit_0x1e()
{
    uassert_int_equal(isxdigit(30), 0); /* isxdigit should be 0 for 0x1e */
}

void t_isxdigit_0x1f()
{
    uassert_int_equal(isxdigit(31), 0); /* isxdigit should be 0 for 0x1f */
}

void t_isxdigit_0x20()
{
    uassert_int_equal(isxdigit(32), 0); /* isxdigit should be 0 for   */
}

void t_isxdigit_0x21()
{
    uassert_int_equal(isxdigit(33), 0); /* isxdigit should be 0 for ! */
}

void t_isxdigit_0x22()
{
    uassert_int_equal(isxdigit(34), 0); /* isxdigit should be 0 for 0x22 */
}

void t_isxdigit_0x23()
{
    uassert_int_equal(isxdigit(35), 0); /* isxdigit should be 0 for # */
}

void t_isxdigit_0x24()
{
    uassert_int_equal(isxdigit(36), 0); /* isxdigit should be 0 for $ */
}

void t_isxdigit_0x25()
{
    uassert_int_equal(isxdigit(37), 0); /* isxdigit should be 0 for % */
}

void t_isxdigit_0x26()
{
    uassert_int_equal(isxdigit(38), 0); /* isxdigit should be 0 for & */
}

void t_isxdigit_0x27()
{
    uassert_int_equal(isxdigit(39), 0); /* isxdigit should be 0 for ' */
}

void t_isxdigit_0x28()
{
    uassert_int_equal(isxdigit(40), 0); /* isxdigit should be 0 for ( */
}

void t_isxdigit_0x29()
{
    uassert_int_equal(isxdigit(41), 0); /* isxdigit should be 0 for ) */
}

void t_isxdigit_0x2a()
{
    uassert_int_equal(isxdigit(42), 0); /* isxdigit should be 0 for * */
}

void t_isxdigit_0x2b()
{
    uassert_int_equal(isxdigit(43), 0); /* isxdigit should be 0 for + */
}

void t_isxdigit_0x2c()
{
    uassert_int_equal(isxdigit(44), 0); /* isxdigit should be 0 for , */
}

void t_isxdigit_0x2d()
{
    uassert_int_equal(isxdigit(45), 0); /* isxdigit should be 0 for - */
}

void t_isxdigit_0x2e()
{
    uassert_int_equal(isxdigit(46), 0); /* isxdigit should be 0 for . */
}

void t_isxdigit_0x2f()
{
    uassert_int_equal(isxdigit(47), 0); /* isxdigit should be 0 for / */
}

void t_isxdigit_0x30()
{
    uassert_int_equal(isxdigit(48), 1); /* isxdigit should be 1 for 0 */
}

void t_isxdigit_0x31()
{
    uassert_int_equal(isxdigit(49), 1); /* isxdigit should be 1 for 1 */
}

void t_isxdigit_0x32()
{
    uassert_int_equal(isxdigit(50), 1); /* isxdigit should be 1 for 2 */
}

void t_isxdigit_0x33()
{
    uassert_int_equal(isxdigit(51), 1); /* isxdigit should be 1 for 3 */
}

void t_isxdigit_0x34()
{
    uassert_int_equal(isxdigit(52), 1); /* isxdigit should be 1 for 4 */
}

void t_isxdigit_0x35()
{
    uassert_int_equal(isxdigit(53), 1); /* isxdigit should be 1 for 5 */
}

void t_isxdigit_0x36()
{
    uassert_int_equal(isxdigit(54), 1); /* isxdigit should be 1 for 6 */
}

void t_isxdigit_0x37()
{
    uassert_int_equal(isxdigit(55), 1); /* isxdigit should be 1 for 7 */
}

void t_isxdigit_0x38()
{
    uassert_int_equal(isxdigit(56), 1); /* isxdigit should be 1 for 8 */
}

void t_isxdigit_0x39()
{
    uassert_int_equal(isxdigit(57), 1); /* isxdigit should be 1 for 9 */
}

void t_isxdigit_0x3a()
{
    uassert_int_equal(isxdigit(58), 0); /* isxdigit should be 0 for : */
}

void t_isxdigit_0x3b()
{
    uassert_int_equal(isxdigit(59), 0); /* isxdigit should be 0 for ; */
}

void t_isxdigit_0x3c()
{
    uassert_int_equal(isxdigit(60), 0); /* isxdigit should be 0 for < */
}

void t_isxdigit_0x3d()
{
    uassert_int_equal(isxdigit(61), 0); /* isxdigit should be 0 for = */
}

void t_isxdigit_0x3e()
{
    uassert_int_equal(isxdigit(62), 0); /* isxdigit should be 0 for > */
}

void t_isxdigit_0x3f()
{
    uassert_int_equal(isxdigit(63), 0); /* isxdigit should be 0 for ? */
}

void t_isxdigit_0x40()
{
    uassert_int_equal(isxdigit(64), 0); /* isxdigit should be 0 for @ */
}

void t_isxdigit_0x41()
{
    uassert_int_equal(isxdigit(65), 1); /* isxdigit should be 1 for A */
}

void t_isxdigit_0x42()
{
    uassert_int_equal(isxdigit(66), 1); /* isxdigit should be 1 for B */
}

void t_isxdigit_0x43()
{
    uassert_int_equal(isxdigit(67), 1); /* isxdigit should be 1 for C */
}

void t_isxdigit_0x44()
{
    uassert_int_equal(isxdigit(68), 1); /* isxdigit should be 1 for D */
}

void t_isxdigit_0x45()
{
    uassert_int_equal(isxdigit(69), 1); /* isxdigit should be 1 for E */
}

void t_isxdigit_0x46()
{
    uassert_int_equal(isxdigit(70), 1); /* isxdigit should be 1 for F */
}

void t_isxdigit_0x47()
{
    uassert_int_equal(isxdigit(71), 0); /* isxdigit should be 0 for G */
}

void t_isxdigit_0x48()
{
    uassert_int_equal(isxdigit(72), 0); /* isxdigit should be 0 for H */
}

void t_isxdigit_0x49()
{
    uassert_int_equal(isxdigit(73), 0); /* isxdigit should be 0 for I */
}

void t_isxdigit_0x4a()
{
    uassert_int_equal(isxdigit(74), 0); /* isxdigit should be 0 for J */
}

void t_isxdigit_0x4b()
{
    uassert_int_equal(isxdigit(75), 0); /* isxdigit should be 0 for K */
}

void t_isxdigit_0x4c()
{
    uassert_int_equal(isxdigit(76), 0); /* isxdigit should be 0 for L */
}

void t_isxdigit_0x4d()
{
    uassert_int_equal(isxdigit(77), 0); /* isxdigit should be 0 for M */
}

void t_isxdigit_0x4e()
{
    uassert_int_equal(isxdigit(78), 0); /* isxdigit should be 0 for N */
}

void t_isxdigit_0x4f()
{
    uassert_int_equal(isxdigit(79), 0); /* isxdigit should be 0 for O */
}

void t_isxdigit_0x50()
{
    uassert_int_equal(isxdigit(80), 0); /* isxdigit should be 0 for P */
}

void t_isxdigit_0x51()
{
    uassert_int_equal(isxdigit(81), 0); /* isxdigit should be 0 for Q */
}

void t_isxdigit_0x52()
{
    uassert_int_equal(isxdigit(82), 0); /* isxdigit should be 0 for R */
}

void t_isxdigit_0x53()
{
    uassert_int_equal(isxdigit(83), 0); /* isxdigit should be 0 for S */
}

void t_isxdigit_0x54()
{
    uassert_int_equal(isxdigit(84), 0); /* isxdigit should be 0 for T */
}

void t_isxdigit_0x55()
{
    uassert_int_equal(isxdigit(85), 0); /* isxdigit should be 0 for U */
}

void t_isxdigit_0x56()
{
    uassert_int_equal(isxdigit(86), 0); /* isxdigit should be 0 for V */
}

void t_isxdigit_0x57()
{
    uassert_int_equal(isxdigit(87), 0); /* isxdigit should be 0 for W */
}

void t_isxdigit_0x58()
{
    uassert_int_equal(isxdigit(88), 0); /* isxdigit should be 0 for X */
}

void t_isxdigit_0x59()
{
    uassert_int_equal(isxdigit(89), 0); /* isxdigit should be 0 for Y */
}

void t_isxdigit_0x5a()
{
    uassert_int_equal(isxdigit(90), 0); /* isxdigit should be 0 for Z */
}

void t_isxdigit_0x5b()
{
    uassert_int_equal(isxdigit(91), 0); /* isxdigit should be 0 for [ */
}

void t_isxdigit_0x5c()
{
    uassert_int_equal(isxdigit(92), 0); /* isxdigit should be 0 for 0x5c */
}

void t_isxdigit_0x5d()
{
    uassert_int_equal(isxdigit(93), 0); /* isxdigit should be 0 for ] */
}

void t_isxdigit_0x5e()
{
    uassert_int_equal(isxdigit(94), 0); /* isxdigit should be 0 for ^ */
}

void t_isxdigit_0x5f()
{
    uassert_int_equal(isxdigit(95), 0); /* isxdigit should be 0 for _ */
}

void t_isxdigit_0x60()
{
    uassert_int_equal(isxdigit(96), 0); /* isxdigit should be 0 for ` */
}

void t_isxdigit_0x61()
{
    uassert_int_equal(isxdigit(97), 1); /* isxdigit should be 1 for a */
}

void t_isxdigit_0x62()
{
    uassert_int_equal(isxdigit(98), 1); /* isxdigit should be 1 for b */
}

void t_isxdigit_0x63()
{
    uassert_int_equal(isxdigit(99), 1); /* isxdigit should be 1 for c */
}

void t_isxdigit_0x64()
{
    uassert_int_equal(isxdigit(100), 1); /* isxdigit should be 1 for d */
}

void t_isxdigit_0x65()
{
    uassert_int_equal(isxdigit(101), 1); /* isxdigit should be 1 for e */
}

void t_isxdigit_0x66()
{
    uassert_int_equal(isxdigit(102), 1); /* isxdigit should be 1 for f */
}

void t_isxdigit_0x67()
{
    uassert_int_equal(isxdigit(103), 0); /* isxdigit should be 0 for g */
}

void t_isxdigit_0x68()
{
    uassert_int_equal(isxdigit(104), 0); /* isxdigit should be 0 for h */
}

void t_isxdigit_0x69()
{
    uassert_int_equal(isxdigit(105), 0); /* isxdigit should be 0 for i */
}

void t_isxdigit_0x6a()
{
    uassert_int_equal(isxdigit(106), 0); /* isxdigit should be 0 for j */
}

void t_isxdigit_0x6b()
{
    uassert_int_equal(isxdigit(107), 0); /* isxdigit should be 0 for k */
}

void t_isxdigit_0x6c()
{
    uassert_int_equal(isxdigit(108), 0); /* isxdigit should be 0 for l */
}

void t_isxdigit_0x6d()
{
    uassert_int_equal(isxdigit(109), 0); /* isxdigit should be 0 for m */
}

void t_isxdigit_0x6e()
{
    uassert_int_equal(isxdigit(110), 0); /* isxdigit should be 0 for n */
}

void t_isxdigit_0x6f()
{
    uassert_int_equal(isxdigit(111), 0); /* isxdigit should be 0 for o */
}

void t_isxdigit_0x70()
{
    uassert_int_equal(isxdigit(112), 0); /* isxdigit should be 0 for p */
}

void t_isxdigit_0x71()
{
    uassert_int_equal(isxdigit(113), 0); /* isxdigit should be 0 for q */
}

void t_isxdigit_0x72()
{
    uassert_int_equal(isxdigit(114), 0); /* isxdigit should be 0 for r */
}

void t_isxdigit_0x73()
{
    uassert_int_equal(isxdigit(115), 0); /* isxdigit should be 0 for s */
}

void t_isxdigit_0x74()
{
    uassert_int_equal(isxdigit(116), 0); /* isxdigit should be 0 for t */
}

void t_isxdigit_0x75()
{
    uassert_int_equal(isxdigit(117), 0); /* isxdigit should be 0 for u */
}

void t_isxdigit_0x76()
{
    uassert_int_equal(isxdigit(118), 0); /* isxdigit should be 0 for v */
}

void t_isxdigit_0x77()
{
    uassert_int_equal(isxdigit(119), 0); /* isxdigit should be 0 for w */
}

void t_isxdigit_0x78()
{
    uassert_int_equal(isxdigit(120), 0); /* isxdigit should be 0 for x */
}

void t_isxdigit_0x79()
{
    uassert_int_equal(isxdigit(121), 0); /* isxdigit should be 0 for y */
}

void t_isxdigit_0x7a()
{
    uassert_int_equal(isxdigit(122), 0); /* isxdigit should be 0 for z */
}

void t_isxdigit_0x7b()
{
    uassert_int_equal(isxdigit(123), 0); /* isxdigit should be 0 for { */
}

void t_isxdigit_0x7c()
{
    uassert_int_equal(isxdigit(124), 0); /* isxdigit should be 0 for | */
}

void t_isxdigit_0x7d()
{
    uassert_int_equal(isxdigit(125), 0); /* isxdigit should be 0 for } */
}

void t_isxdigit_0x7e()
{
    uassert_int_equal(isxdigit(126), 0); /* isxdigit should be 0 for ~ */
}

void t_isxdigit_0x7f()
{
    uassert_int_equal(isxdigit(127), 0); /* isxdigit should be 0 for 0x7f */
}

void t_isxdigit_0x80()
{
    uassert_int_equal(isxdigit(128), 0); /* isxdigit should be 0 for 0x80 */
}

void t_isxdigit_0x81()
{
    uassert_int_equal(isxdigit(129), 0); /* isxdigit should be 0 for 0x81 */
}

void t_isxdigit_0x82()
{
    uassert_int_equal(isxdigit(130), 0); /* isxdigit should be 0 for 0x82 */
}

void t_isxdigit_0x83()
{
    uassert_int_equal(isxdigit(131), 0); /* isxdigit should be 0 for 0x83 */
}

void t_isxdigit_0x84()
{
    uassert_int_equal(isxdigit(132), 0); /* isxdigit should be 0 for 0x84 */
}

void t_isxdigit_0x85()
{
    uassert_int_equal(isxdigit(133), 0); /* isxdigit should be 0 for 0x85 */
}

void t_isxdigit_0x86()
{
    uassert_int_equal(isxdigit(134), 0); /* isxdigit should be 0 for 0x86 */
}

void t_isxdigit_0x87()
{
    uassert_int_equal(isxdigit(135), 0); /* isxdigit should be 0 for 0x87 */
}

void t_isxdigit_0x88()
{
    uassert_int_equal(isxdigit(136), 0); /* isxdigit should be 0 for 0x88 */
}

void t_isxdigit_0x89()
{
    uassert_int_equal(isxdigit(137), 0); /* isxdigit should be 0 for 0x89 */
}

void t_isxdigit_0x8a()
{
    uassert_int_equal(isxdigit(138), 0); /* isxdigit should be 0 for 0x8a */
}

void t_isxdigit_0x8b()
{
    uassert_int_equal(isxdigit(139), 0); /* isxdigit should be 0 for 0x8b */
}

void t_isxdigit_0x8c()
{
    uassert_int_equal(isxdigit(140), 0); /* isxdigit should be 0 for 0x8c */
}

void t_isxdigit_0x8d()
{
    uassert_int_equal(isxdigit(141), 0); /* isxdigit should be 0 for 0x8d */
}

void t_isxdigit_0x8e()
{
    uassert_int_equal(isxdigit(142), 0); /* isxdigit should be 0 for 0x8e */
}

void t_isxdigit_0x8f()
{
    uassert_int_equal(isxdigit(143), 0); /* isxdigit should be 0 for 0x8f */
}

void t_isxdigit_0x90()
{
    uassert_int_equal(isxdigit(144), 0); /* isxdigit should be 0 for 0x90 */
}

void t_isxdigit_0x91()
{
    uassert_int_equal(isxdigit(145), 0); /* isxdigit should be 0 for 0x91 */
}

void t_isxdigit_0x92()
{
    uassert_int_equal(isxdigit(146), 0); /* isxdigit should be 0 for 0x92 */
}

void t_isxdigit_0x93()
{
    uassert_int_equal(isxdigit(147), 0); /* isxdigit should be 0 for 0x93 */
}

void t_isxdigit_0x94()
{
    uassert_int_equal(isxdigit(148), 0); /* isxdigit should be 0 for 0x94 */
}

void t_isxdigit_0x95()
{
    uassert_int_equal(isxdigit(149), 0); /* isxdigit should be 0 for 0x95 */
}

void t_isxdigit_0x96()
{
    uassert_int_equal(isxdigit(150), 0); /* isxdigit should be 0 for 0x96 */
}

void t_isxdigit_0x97()
{
    uassert_int_equal(isxdigit(151), 0); /* isxdigit should be 0 for 0x97 */
}

void t_isxdigit_0x98()
{
    uassert_int_equal(isxdigit(152), 0); /* isxdigit should be 0 for 0x98 */
}

void t_isxdigit_0x99()
{
    uassert_int_equal(isxdigit(153), 0); /* isxdigit should be 0 for 0x99 */
}

void t_isxdigit_0x9a()
{
    uassert_int_equal(isxdigit(154), 0); /* isxdigit should be 0 for 0x9a */
}

void t_isxdigit_0x9b()
{
    uassert_int_equal(isxdigit(155), 0); /* isxdigit should be 0 for 0x9b */
}

void t_isxdigit_0x9c()
{
    uassert_int_equal(isxdigit(156), 0); /* isxdigit should be 0 for 0x9c */
}

void t_isxdigit_0x9d()
{
    uassert_int_equal(isxdigit(157), 0); /* isxdigit should be 0 for 0x9d */
}

void t_isxdigit_0x9e()
{
    uassert_int_equal(isxdigit(158), 0); /* isxdigit should be 0 for 0x9e */
}

void t_isxdigit_0x9f()
{
    uassert_int_equal(isxdigit(159), 0); /* isxdigit should be 0 for 0x9f */
}

void t_isxdigit_0xa0()
{
    uassert_int_equal(isxdigit(160), 0); /* isxdigit should be 0 for 0xa0 */
}

void t_isxdigit_0xa1()
{
    uassert_int_equal(isxdigit(161), 0); /* isxdigit should be 0 for 0xa1 */
}

void t_isxdigit_0xa2()
{
    uassert_int_equal(isxdigit(162), 0); /* isxdigit should be 0 for 0xa2 */
}

void t_isxdigit_0xa3()
{
    uassert_int_equal(isxdigit(163), 0); /* isxdigit should be 0 for 0xa3 */
}

void t_isxdigit_0xa4()
{
    uassert_int_equal(isxdigit(164), 0); /* isxdigit should be 0 for 0xa4 */
}

void t_isxdigit_0xa5()
{
    uassert_int_equal(isxdigit(165), 0); /* isxdigit should be 0 for 0xa5 */
}

void t_isxdigit_0xa6()
{
    uassert_int_equal(isxdigit(166), 0); /* isxdigit should be 0 for 0xa6 */
}

void t_isxdigit_0xa7()
{
    uassert_int_equal(isxdigit(167), 0); /* isxdigit should be 0 for 0xa7 */
}

void t_isxdigit_0xa8()
{
    uassert_int_equal(isxdigit(168), 0); /* isxdigit should be 0 for 0xa8 */
}

void t_isxdigit_0xa9()
{
    uassert_int_equal(isxdigit(169), 0); /* isxdigit should be 0 for 0xa9 */
}

void t_isxdigit_0xaa()
{
    uassert_int_equal(isxdigit(170), 0); /* isxdigit should be 0 for 0xaa */
}

void t_isxdigit_0xab()
{
    uassert_int_equal(isxdigit(171), 0); /* isxdigit should be 0 for 0xab */
}

void t_isxdigit_0xac()
{
    uassert_int_equal(isxdigit(172), 0); /* isxdigit should be 0 for 0xac */
}

void t_isxdigit_0xad()
{
    uassert_int_equal(isxdigit(173), 0); /* isxdigit should be 0 for 0xad */
}

void t_isxdigit_0xae()
{
    uassert_int_equal(isxdigit(174), 0); /* isxdigit should be 0 for 0xae */
}

void t_isxdigit_0xaf()
{
    uassert_int_equal(isxdigit(175), 0); /* isxdigit should be 0 for 0xaf */
}

void t_isxdigit_0xb0()
{
    uassert_int_equal(isxdigit(176), 0); /* isxdigit should be 0 for 0xb0 */
}

void t_isxdigit_0xb1()
{
    uassert_int_equal(isxdigit(177), 0); /* isxdigit should be 0 for 0xb1 */
}

void t_isxdigit_0xb2()
{
    uassert_int_equal(isxdigit(178), 0); /* isxdigit should be 0 for 0xb2 */
}

void t_isxdigit_0xb3()
{
    uassert_int_equal(isxdigit(179), 0); /* isxdigit should be 0 for 0xb3 */
}

void t_isxdigit_0xb4()
{
    uassert_int_equal(isxdigit(180), 0); /* isxdigit should be 0 for 0xb4 */
}

void t_isxdigit_0xb5()
{
    uassert_int_equal(isxdigit(181), 0); /* isxdigit should be 0 for 0xb5 */
}

void t_isxdigit_0xb6()
{
    uassert_int_equal(isxdigit(182), 0); /* isxdigit should be 0 for 0xb6 */
}

void t_isxdigit_0xb7()
{
    uassert_int_equal(isxdigit(183), 0); /* isxdigit should be 0 for 0xb7 */
}

void t_isxdigit_0xb8()
{
    uassert_int_equal(isxdigit(184), 0); /* isxdigit should be 0 for 0xb8 */
}

void t_isxdigit_0xb9()
{
    uassert_int_equal(isxdigit(185), 0); /* isxdigit should be 0 for 0xb9 */
}

void t_isxdigit_0xba()
{
    uassert_int_equal(isxdigit(186), 0); /* isxdigit should be 0 for 0xba */
}

void t_isxdigit_0xbb()
{
    uassert_int_equal(isxdigit(187), 0); /* isxdigit should be 0 for 0xbb */
}

void t_isxdigit_0xbc()
{
    uassert_int_equal(isxdigit(188), 0); /* isxdigit should be 0 for 0xbc */
}

void t_isxdigit_0xbd()
{
    uassert_int_equal(isxdigit(189), 0); /* isxdigit should be 0 for 0xbd */
}

void t_isxdigit_0xbe()
{
    uassert_int_equal(isxdigit(190), 0); /* isxdigit should be 0 for 0xbe */
}

void t_isxdigit_0xbf()
{
    uassert_int_equal(isxdigit(191), 0); /* isxdigit should be 0 for 0xbf */
}

void t_isxdigit_0xc0()
{
    uassert_int_equal(isxdigit(192), 0); /* isxdigit should be 0 for 0xc0 */
}

void t_isxdigit_0xc1()
{
    uassert_int_equal(isxdigit(193), 0); /* isxdigit should be 0 for 0xc1 */
}

void t_isxdigit_0xc2()
{
    uassert_int_equal(isxdigit(194), 0); /* isxdigit should be 0 for 0xc2 */
}

void t_isxdigit_0xc3()
{
    uassert_int_equal(isxdigit(195), 0); /* isxdigit should be 0 for 0xc3 */
}

void t_isxdigit_0xc4()
{
    uassert_int_equal(isxdigit(196), 0); /* isxdigit should be 0 for 0xc4 */
}

void t_isxdigit_0xc5()
{
    uassert_int_equal(isxdigit(197), 0); /* isxdigit should be 0 for 0xc5 */
}

void t_isxdigit_0xc6()
{
    uassert_int_equal(isxdigit(198), 0); /* isxdigit should be 0 for 0xc6 */
}

void t_isxdigit_0xc7()
{
    uassert_int_equal(isxdigit(199), 0); /* isxdigit should be 0 for 0xc7 */
}

void t_isxdigit_0xc8()
{
    uassert_int_equal(isxdigit(200), 0); /* isxdigit should be 0 for 0xc8 */
}

void t_isxdigit_0xc9()
{
    uassert_int_equal(isxdigit(201), 0); /* isxdigit should be 0 for 0xc9 */
}

void t_isxdigit_0xca()
{
    uassert_int_equal(isxdigit(202), 0); /* isxdigit should be 0 for 0xca */
}

void t_isxdigit_0xcb()
{
    uassert_int_equal(isxdigit(203), 0); /* isxdigit should be 0 for 0xcb */
}

void t_isxdigit_0xcc()
{
    uassert_int_equal(isxdigit(204), 0); /* isxdigit should be 0 for 0xcc */
}

void t_isxdigit_0xcd()
{
    uassert_int_equal(isxdigit(205), 0); /* isxdigit should be 0 for 0xcd */
}

void t_isxdigit_0xce()
{
    uassert_int_equal(isxdigit(206), 0); /* isxdigit should be 0 for 0xce */
}

void t_isxdigit_0xcf()
{
    uassert_int_equal(isxdigit(207), 0); /* isxdigit should be 0 for 0xcf */
}

void t_isxdigit_0xd0()
{
    uassert_int_equal(isxdigit(208), 0); /* isxdigit should be 0 for 0xd0 */
}

void t_isxdigit_0xd1()
{
    uassert_int_equal(isxdigit(209), 0); /* isxdigit should be 0 for 0xd1 */
}

void t_isxdigit_0xd2()
{
    uassert_int_equal(isxdigit(210), 0); /* isxdigit should be 0 for 0xd2 */
}

void t_isxdigit_0xd3()
{
    uassert_int_equal(isxdigit(211), 0); /* isxdigit should be 0 for 0xd3 */
}

void t_isxdigit_0xd4()
{
    uassert_int_equal(isxdigit(212), 0); /* isxdigit should be 0 for 0xd4 */
}

void t_isxdigit_0xd5()
{
    uassert_int_equal(isxdigit(213), 0); /* isxdigit should be 0 for 0xd5 */
}

void t_isxdigit_0xd6()
{
    uassert_int_equal(isxdigit(214), 0); /* isxdigit should be 0 for 0xd6 */
}

void t_isxdigit_0xd7()
{
    uassert_int_equal(isxdigit(215), 0); /* isxdigit should be 0 for 0xd7 */
}

void t_isxdigit_0xd8()
{
    uassert_int_equal(isxdigit(216), 0); /* isxdigit should be 0 for 0xd8 */
}

void t_isxdigit_0xd9()
{
    uassert_int_equal(isxdigit(217), 0); /* isxdigit should be 0 for 0xd9 */
}

void t_isxdigit_0xda()
{
    uassert_int_equal(isxdigit(218), 0); /* isxdigit should be 0 for 0xda */
}

void t_isxdigit_0xdb()
{
    uassert_int_equal(isxdigit(219), 0); /* isxdigit should be 0 for 0xdb */
}

void t_isxdigit_0xdc()
{
    uassert_int_equal(isxdigit(220), 0); /* isxdigit should be 0 for 0xdc */
}

void t_isxdigit_0xdd()
{
    uassert_int_equal(isxdigit(221), 0); /* isxdigit should be 0 for 0xdd */
}

void t_isxdigit_0xde()
{
    uassert_int_equal(isxdigit(222), 0); /* isxdigit should be 0 for 0xde */
}

void t_isxdigit_0xdf()
{
    uassert_int_equal(isxdigit(223), 0); /* isxdigit should be 0 for 0xdf */
}

void t_isxdigit_0xe0()
{
    uassert_int_equal(isxdigit(224), 0); /* isxdigit should be 0 for 0xe0 */
}

void t_isxdigit_0xe1()
{
    uassert_int_equal(isxdigit(225), 0); /* isxdigit should be 0 for 0xe1 */
}

void t_isxdigit_0xe2()
{
    uassert_int_equal(isxdigit(226), 0); /* isxdigit should be 0 for 0xe2 */
}

void t_isxdigit_0xe3()
{
    uassert_int_equal(isxdigit(227), 0); /* isxdigit should be 0 for 0xe3 */
}

void t_isxdigit_0xe4()
{
    uassert_int_equal(isxdigit(228), 0); /* isxdigit should be 0 for 0xe4 */
}

void t_isxdigit_0xe5()
{
    uassert_int_equal(isxdigit(229), 0); /* isxdigit should be 0 for 0xe5 */
}

void t_isxdigit_0xe6()
{
    uassert_int_equal(isxdigit(230), 0); /* isxdigit should be 0 for 0xe6 */
}

void t_isxdigit_0xe7()
{
    uassert_int_equal(isxdigit(231), 0); /* isxdigit should be 0 for 0xe7 */
}

void t_isxdigit_0xe8()
{
    uassert_int_equal(isxdigit(232), 0); /* isxdigit should be 0 for 0xe8 */
}

void t_isxdigit_0xe9()
{
    uassert_int_equal(isxdigit(233), 0); /* isxdigit should be 0 for 0xe9 */
}

void t_isxdigit_0xea()
{
    uassert_int_equal(isxdigit(234), 0); /* isxdigit should be 0 for 0xea */
}

void t_isxdigit_0xeb()
{
    uassert_int_equal(isxdigit(235), 0); /* isxdigit should be 0 for 0xeb */
}

void t_isxdigit_0xec()
{
    uassert_int_equal(isxdigit(236), 0); /* isxdigit should be 0 for 0xec */
}

void t_isxdigit_0xed()
{
    uassert_int_equal(isxdigit(237), 0); /* isxdigit should be 0 for 0xed */
}

void t_isxdigit_0xee()
{
    uassert_int_equal(isxdigit(238), 0); /* isxdigit should be 0 for 0xee */
}

void t_isxdigit_0xef()
{
    uassert_int_equal(isxdigit(239), 0); /* isxdigit should be 0 for 0xef */
}

void t_isxdigit_0xf0()
{
    uassert_int_equal(isxdigit(240), 0); /* isxdigit should be 0 for 0xf0 */
}

void t_isxdigit_0xf1()
{
    uassert_int_equal(isxdigit(241), 0); /* isxdigit should be 0 for 0xf1 */
}

void t_isxdigit_0xf2()
{
    uassert_int_equal(isxdigit(242), 0); /* isxdigit should be 0 for 0xf2 */
}

void t_isxdigit_0xf3()
{
    uassert_int_equal(isxdigit(243), 0); /* isxdigit should be 0 for 0xf3 */
}

void t_isxdigit_0xf4()
{
    uassert_int_equal(isxdigit(244), 0); /* isxdigit should be 0 for 0xf4 */
}

void t_isxdigit_0xf5()
{
    uassert_int_equal(isxdigit(245), 0); /* isxdigit should be 0 for 0xf5 */
}

void t_isxdigit_0xf6()
{
    uassert_int_equal(isxdigit(246), 0); /* isxdigit should be 0 for 0xf6 */
}

void t_isxdigit_0xf7()
{
    uassert_int_equal(isxdigit(247), 0); /* isxdigit should be 0 for 0xf7 */
}

void t_isxdigit_0xf8()
{
    uassert_int_equal(isxdigit(248), 0); /* isxdigit should be 0 for 0xf8 */
}

void t_isxdigit_0xf9()
{
    uassert_int_equal(isxdigit(249), 0); /* isxdigit should be 0 for 0xf9 */
}

void t_isxdigit_0xfa()
{
    uassert_int_equal(isxdigit(250), 0); /* isxdigit should be 0 for 0xfa */
}

void t_isxdigit_0xfb()
{
    uassert_int_equal(isxdigit(251), 0); /* isxdigit should be 0 for 0xfb */
}

void t_isxdigit_0xfc()
{
    uassert_int_equal(isxdigit(252), 0); /* isxdigit should be 0 for 0xfc */
}

void t_isxdigit_0xfd()
{
    uassert_int_equal(isxdigit(253), 0); /* isxdigit should be 0 for 0xfd */
}

void t_isxdigit_0xfe()
{
    uassert_int_equal(isxdigit(254), 0); /* isxdigit should be 0 for 0xfe */
}

void t_isxdigit_0xff()
{
    uassert_int_equal(isxdigit(255), 0); /* isxdigit should be 0 for 0xff */
}



static int testcase(void)
{
    t_isxdigit_0x00();
    t_isxdigit_0x01();
    t_isxdigit_0x02();
    t_isxdigit_0x03();
    t_isxdigit_0x04();
    t_isxdigit_0x05();
    t_isxdigit_0x06();
    t_isxdigit_0x07();
    t_isxdigit_0x08();
    t_isxdigit_0x09();
    t_isxdigit_0x0a();
    t_isxdigit_0x0b();
    t_isxdigit_0x0c();
    t_isxdigit_0x0d();
    t_isxdigit_0x0e();
    t_isxdigit_0x0f();
    t_isxdigit_0x10();
    t_isxdigit_0x11();
    t_isxdigit_0x12();
    t_isxdigit_0x13();
    t_isxdigit_0x14();
    t_isxdigit_0x15();
    t_isxdigit_0x16();
    t_isxdigit_0x17();
    t_isxdigit_0x18();
    t_isxdigit_0x19();
    t_isxdigit_0x1a();
    t_isxdigit_0x1b();
    t_isxdigit_0x1c();
    t_isxdigit_0x1d();
    t_isxdigit_0x1e();
    t_isxdigit_0x1f();
    t_isxdigit_0x20();
    t_isxdigit_0x21();
    t_isxdigit_0x22();
    t_isxdigit_0x23();
    t_isxdigit_0x24();
    t_isxdigit_0x25();
    t_isxdigit_0x26();
    t_isxdigit_0x27();
    t_isxdigit_0x28();
    t_isxdigit_0x29();
    t_isxdigit_0x2a();
    t_isxdigit_0x2b();
    t_isxdigit_0x2c();
    t_isxdigit_0x2d();
    t_isxdigit_0x2e();
    t_isxdigit_0x2f();
    t_isxdigit_0x30();
    t_isxdigit_0x31();
    t_isxdigit_0x32();
    t_isxdigit_0x33();
    t_isxdigit_0x34();
    t_isxdigit_0x35();
    t_isxdigit_0x36();
    t_isxdigit_0x37();
    t_isxdigit_0x38();
    t_isxdigit_0x39();
    t_isxdigit_0x3a();
    t_isxdigit_0x3b();
    t_isxdigit_0x3c();
    t_isxdigit_0x3d();
    t_isxdigit_0x3e();
    t_isxdigit_0x3f();
    t_isxdigit_0x40();
    t_isxdigit_0x41();
    t_isxdigit_0x42();
    t_isxdigit_0x43();
    t_isxdigit_0x44();
    t_isxdigit_0x45();
    t_isxdigit_0x46();
    t_isxdigit_0x47();
    t_isxdigit_0x48();
    t_isxdigit_0x49();
    t_isxdigit_0x4a();
    t_isxdigit_0x4b();
    t_isxdigit_0x4c();
    t_isxdigit_0x4d();
    t_isxdigit_0x4e();
    t_isxdigit_0x4f();
    t_isxdigit_0x50();
    t_isxdigit_0x51();
    t_isxdigit_0x52();
    t_isxdigit_0x53();
    t_isxdigit_0x54();
    t_isxdigit_0x55();
    t_isxdigit_0x56();
    t_isxdigit_0x57();
    t_isxdigit_0x58();
    t_isxdigit_0x59();
    t_isxdigit_0x5a();
    t_isxdigit_0x5b();
    t_isxdigit_0x5c();
    t_isxdigit_0x5d();
    t_isxdigit_0x5e();
    t_isxdigit_0x5f();
    t_isxdigit_0x60();
    t_isxdigit_0x61();
    t_isxdigit_0x62();
    t_isxdigit_0x63();
    t_isxdigit_0x64();
    t_isxdigit_0x65();
    t_isxdigit_0x66();
    t_isxdigit_0x67();
    t_isxdigit_0x68();
    t_isxdigit_0x69();
    t_isxdigit_0x6a();
    t_isxdigit_0x6b();
    t_isxdigit_0x6c();
    t_isxdigit_0x6d();
    t_isxdigit_0x6e();
    t_isxdigit_0x6f();
    t_isxdigit_0x70();
    t_isxdigit_0x71();
    t_isxdigit_0x72();
    t_isxdigit_0x73();
    t_isxdigit_0x74();
    t_isxdigit_0x75();
    t_isxdigit_0x76();
    t_isxdigit_0x77();
    t_isxdigit_0x78();
    t_isxdigit_0x79();
    t_isxdigit_0x7a();
    t_isxdigit_0x7b();
    t_isxdigit_0x7c();
    t_isxdigit_0x7d();
    t_isxdigit_0x7e();
    t_isxdigit_0x7f();
    t_isxdigit_0x80();
    t_isxdigit_0x81();
    t_isxdigit_0x82();
    t_isxdigit_0x83();
    t_isxdigit_0x84();
    t_isxdigit_0x85();
    t_isxdigit_0x86();
    t_isxdigit_0x87();
    t_isxdigit_0x88();
    t_isxdigit_0x89();
    t_isxdigit_0x8a();
    t_isxdigit_0x8b();
    t_isxdigit_0x8c();
    t_isxdigit_0x8d();
    t_isxdigit_0x8e();
    t_isxdigit_0x8f();
    t_isxdigit_0x90();
    t_isxdigit_0x91();
    t_isxdigit_0x92();
    t_isxdigit_0x93();
    t_isxdigit_0x94();
    t_isxdigit_0x95();
    t_isxdigit_0x96();
    t_isxdigit_0x97();
    t_isxdigit_0x98();
    t_isxdigit_0x99();
    t_isxdigit_0x9a();
    t_isxdigit_0x9b();
    t_isxdigit_0x9c();
    t_isxdigit_0x9d();
    t_isxdigit_0x9e();
    t_isxdigit_0x9f();
    t_isxdigit_0xa0();
    t_isxdigit_0xa1();
    t_isxdigit_0xa2();
    t_isxdigit_0xa3();
    t_isxdigit_0xa4();
    t_isxdigit_0xa5();
    t_isxdigit_0xa6();
    t_isxdigit_0xa7();
    t_isxdigit_0xa8();
    t_isxdigit_0xa9();
    t_isxdigit_0xaa();
    t_isxdigit_0xab();
    t_isxdigit_0xac();
    t_isxdigit_0xad();
    t_isxdigit_0xae();
    t_isxdigit_0xaf();
    t_isxdigit_0xb0();
    t_isxdigit_0xb1();
    t_isxdigit_0xb2();
    t_isxdigit_0xb3();
    t_isxdigit_0xb4();
    t_isxdigit_0xb5();
    t_isxdigit_0xb6();
    t_isxdigit_0xb7();
    t_isxdigit_0xb8();
    t_isxdigit_0xb9();
    t_isxdigit_0xba();
    t_isxdigit_0xbb();
    t_isxdigit_0xbc();
    t_isxdigit_0xbd();
    t_isxdigit_0xbe();
    t_isxdigit_0xbf();
    t_isxdigit_0xc0();
    t_isxdigit_0xc1();
    t_isxdigit_0xc2();
    t_isxdigit_0xc3();
    t_isxdigit_0xc4();
    t_isxdigit_0xc5();
    t_isxdigit_0xc6();
    t_isxdigit_0xc7();
    t_isxdigit_0xc8();
    t_isxdigit_0xc9();
    t_isxdigit_0xca();
    t_isxdigit_0xcb();
    t_isxdigit_0xcc();
    t_isxdigit_0xcd();
    t_isxdigit_0xce();
    t_isxdigit_0xcf();
    t_isxdigit_0xd0();
    t_isxdigit_0xd1();
    t_isxdigit_0xd2();
    t_isxdigit_0xd3();
    t_isxdigit_0xd4();
    t_isxdigit_0xd5();
    t_isxdigit_0xd6();
    t_isxdigit_0xd7();
    t_isxdigit_0xd8();
    t_isxdigit_0xd9();
    t_isxdigit_0xda();
    t_isxdigit_0xdb();
    t_isxdigit_0xdc();
    t_isxdigit_0xdd();
    t_isxdigit_0xde();
    t_isxdigit_0xdf();
    t_isxdigit_0xe0();
    t_isxdigit_0xe1();
    t_isxdigit_0xe2();
    t_isxdigit_0xe3();
    t_isxdigit_0xe4();
    t_isxdigit_0xe5();
    t_isxdigit_0xe6();
    t_isxdigit_0xe7();
    t_isxdigit_0xe8();
    t_isxdigit_0xe9();
    t_isxdigit_0xea();
    t_isxdigit_0xeb();
    t_isxdigit_0xec();
    t_isxdigit_0xed();
    t_isxdigit_0xee();
    t_isxdigit_0xef();
    t_isxdigit_0xf0();
    t_isxdigit_0xf1();
    t_isxdigit_0xf2();
    t_isxdigit_0xf3();
    t_isxdigit_0xf4();
    t_isxdigit_0xf5();
    t_isxdigit_0xf6();
    t_isxdigit_0xf7();
    t_isxdigit_0xf8();
    t_isxdigit_0xf9();
    t_isxdigit_0xfa();
    t_isxdigit_0xfb();
    t_isxdigit_0xfc();
    t_isxdigit_0xfd();
    t_isxdigit_0xfe();
    t_isxdigit_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}
