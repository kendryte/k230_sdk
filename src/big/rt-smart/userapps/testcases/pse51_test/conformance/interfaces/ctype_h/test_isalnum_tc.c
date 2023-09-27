#include <ctype.h>
#include <assert.h>

#define uassert_int_equal(a, b)      assert((a) == (b))

void t_isalnum_0x00()
{
    uassert_int_equal(isalnum(0), 0);  /* isalnum should be 0 for 0x00 */
}

void t_isalnum_0x01()
{
    uassert_int_equal(isalnum(1), 0);  /* isalnum should be 0 for 0x01 */
}

void t_isalnum_0x02()
{
    uassert_int_equal(isalnum(2), 0);  /* isalnum should be 0 for 0x02 */
}

void t_isalnum_0x03()
{
    uassert_int_equal(isalnum(3), 0);  /* isalnum should be 0 for 0x03 */
}

void t_isalnum_0x04()
{
    uassert_int_equal(isalnum(4), 0);  /* isalnum should be 0 for 0x04 */
}

void t_isalnum_0x05()
{
    uassert_int_equal(isalnum(5), 0);  /* isalnum should be 0 for 0x05 */
}

void t_isalnum_0x06()
{
    uassert_int_equal(isalnum(6), 0);  /* isalnum should be 0 for 0x06 */
}

void t_isalnum_0x07()
{
    uassert_int_equal(isalnum(7), 0);  /* isalnum should be 0 for 0x07 */
}

void t_isalnum_0x08()
{
    uassert_int_equal(isalnum(8), 0);  /* isalnum should be 0 for 0x08 */
}

void t_isalnum_0x09()
{
    uassert_int_equal(isalnum(9), 0);  /* isalnum should be 0 for 0x09 */
}

void t_isalnum_0x0a()
{
    uassert_int_equal(isalnum(10), 0);  /* isalnum should be 0 for 0x0a */
}

void t_isalnum_0x0b()
{
    uassert_int_equal(isalnum(11), 0);  /* isalnum should be 0 for 0x0b */
}

void t_isalnum_0x0c()
{
    uassert_int_equal(isalnum(12), 0);  /* isalnum should be 0 for 0x0c */
}

void t_isalnum_0x0d()
{
    uassert_int_equal(isalnum(13), 0);  /* isalnum should be 0 for 0x0d */
}

void t_isalnum_0x0e()
{
    uassert_int_equal(isalnum(14), 0);  /* isalnum should be 0 for 0x0e */
}

void t_isalnum_0x0f()
{
    uassert_int_equal(isalnum(15), 0);  /* isalnum should be 0 for 0x0f */
}

void t_isalnum_0x10()
{
    uassert_int_equal(isalnum(16), 0);  /* isalnum should be 0 for 0x10 */
}

void t_isalnum_0x11()
{
    uassert_int_equal(isalnum(17), 0);  /* isalnum should be 0 for 0x11 */
}

void t_isalnum_0x12()
{
    uassert_int_equal(isalnum(18), 0);  /* isalnum should be 0 for 0x12 */
}

void t_isalnum_0x13()
{
    uassert_int_equal(isalnum(19), 0);  /* isalnum should be 0 for 0x13 */
}

void t_isalnum_0x14()
{
    uassert_int_equal(isalnum(20), 0);  /* isalnum should be 0 for 0x14 */
}

void t_isalnum_0x15()
{
    uassert_int_equal(isalnum(21), 0);  /* isalnum should be 0 for 0x15 */
}

void t_isalnum_0x16()
{
    uassert_int_equal(isalnum(22), 0);  /* isalnum should be 0 for 0x16 */
}

void t_isalnum_0x17()
{
    uassert_int_equal(isalnum(23), 0);  /* isalnum should be 0 for 0x17 */
}

void t_isalnum_0x18()
{
    uassert_int_equal(isalnum(24), 0);  /* isalnum should be 0 for 0x18 */
}

void t_isalnum_0x19()
{
    uassert_int_equal(isalnum(25), 0);  /* isalnum should be 0 for 0x19 */
}

void t_isalnum_0x1a()
{
    uassert_int_equal(isalnum(26), 0);  /* isalnum should be 0 for 0x1a */
}

void t_isalnum_0x1b()
{
    uassert_int_equal(isalnum(27), 0);  /* isalnum should be 0 for 0x1b */
}

void t_isalnum_0x1c()
{
    uassert_int_equal(isalnum(28), 0);  /* isalnum should be 0 for 0x1c */
}

void t_isalnum_0x1d()
{
    uassert_int_equal(isalnum(29), 0);  /* isalnum should be 0 for 0x1d */
}

void t_isalnum_0x1e()
{
    uassert_int_equal(isalnum(30), 0);  /* isalnum should be 0 for 0x1e */
}

void t_isalnum_0x1f()
{
    uassert_int_equal(isalnum(31), 0);  /* isalnum should be 0 for 0x1f */
}

void t_isalnum_0x20()
{
    uassert_int_equal(isalnum(32), 0);  /* isalnum should be 0 for   */
}

void t_isalnum_0x21()
{
    uassert_int_equal(isalnum(33), 0);  /* isalnum should be 0 for ! */
}

void t_isalnum_0x22()
{
    uassert_int_equal(isalnum(34), 0);  /* isalnum should be 0 for 0x22 */
}

void t_isalnum_0x23()
{
    uassert_int_equal(isalnum(35), 0);  /* isalnum should be 0 for # */
}

void t_isalnum_0x24()
{
    uassert_int_equal(isalnum(36), 0);  /* isalnum should be 0 for $ */
}

void t_isalnum_0x25()
{
    uassert_int_equal(isalnum(37), 0);  /* isalnum should be 0 for % */
}

void t_isalnum_0x26()
{
    uassert_int_equal(isalnum(38), 0);  /* isalnum should be 0 for & */
}

void t_isalnum_0x27()
{
    uassert_int_equal(isalnum(39), 0);  /* isalnum should be 0 for ' */
}

void t_isalnum_0x28()
{
    uassert_int_equal(isalnum(40), 0);  /* isalnum should be 0 for ( */
}

void t_isalnum_0x29()
{
    uassert_int_equal(isalnum(41), 0);  /* isalnum should be 0 for ) */
}

void t_isalnum_0x2a()
{
    uassert_int_equal(isalnum(42), 0);  /* isalnum should be 0 for * */
}

void t_isalnum_0x2b()
{
    uassert_int_equal(isalnum(43), 0);  /* isalnum should be 0 for + */
}

void t_isalnum_0x2c()
{
    uassert_int_equal(isalnum(44), 0);  /* isalnum should be 0 for , */
}

void t_isalnum_0x2d()
{
    uassert_int_equal(isalnum(45), 0);  /* isalnum should be 0 for - */
}

void t_isalnum_0x2e()
{
    uassert_int_equal(isalnum(46), 0);  /* isalnum should be 0 for . */
}

void t_isalnum_0x2f()
{
    uassert_int_equal(isalnum(47), 0);  /* isalnum should be 0 for / */
}

void t_isalnum_0x30()
{
    uassert_int_equal(isalnum(48) , 1); /* isalnum should be 1 for 0 */
}

void t_isalnum_0x31()
{
    uassert_int_equal(isalnum(49) , 1); /* isalnum should be 1 for 1 */
}

void t_isalnum_0x32()
{
    uassert_int_equal(isalnum(50) , 1); /* isalnum should be 1 for 2 */
}

void t_isalnum_0x33()
{
    uassert_int_equal(isalnum(51) , 1); /* isalnum should be 1 for 3 */
}

void t_isalnum_0x34()
{
    uassert_int_equal(isalnum(52) , 1); /* isalnum should be 1 for 4 */
}

void t_isalnum_0x35()
{
    uassert_int_equal(isalnum(53) , 1); /* isalnum should be 1 for 5 */
}

void t_isalnum_0x36()
{
    uassert_int_equal(isalnum(54) , 1); /* isalnum should be 1 for 6 */
}

void t_isalnum_0x37()
{
    uassert_int_equal(isalnum(55) , 1); /* isalnum should be 1 for 7 */
}

void t_isalnum_0x38()
{
    uassert_int_equal(isalnum(56) , 1); /* isalnum should be 1 for 8 */
}

void t_isalnum_0x39()
{
    uassert_int_equal(isalnum(57) , 1); /* isalnum should be 1 for 9 */
}

void t_isalnum_0x3a()
{
    uassert_int_equal(isalnum(58), 0);  /* isalnum should be 0 for : */
}

void t_isalnum_0x3b()
{
    uassert_int_equal(isalnum(59), 0);  /* isalnum should be 0 for ; */
}

void t_isalnum_0x3c()
{
    uassert_int_equal(isalnum(60), 0);  /* isalnum should be 0 for < */
}

void t_isalnum_0x3d()
{
    uassert_int_equal(isalnum(61), 0);  /* isalnum should be 0 for = */
}

void t_isalnum_0x3e()
{
    uassert_int_equal(isalnum(62), 0);  /* isalnum should be 0 for > */
}

void t_isalnum_0x3f()
{
    uassert_int_equal(isalnum(63), 0);  /* isalnum should be 0 for ? */
}

void t_isalnum_0x40()
{
    uassert_int_equal(isalnum(64), 0);  /* isalnum should be 0 for @ */
}

void t_isalnum_0x41()
{
    uassert_int_equal(isalnum(65) , 1); /* isalnum should be 1 for A */
}

void t_isalnum_0x42()
{
    uassert_int_equal(isalnum(66) , 1); /* isalnum should be 1 for B */
}

void t_isalnum_0x43()
{
    uassert_int_equal(isalnum(67) , 1); /* isalnum should be 1 for C */
}

void t_isalnum_0x44()
{
    uassert_int_equal(isalnum(68) , 1); /* isalnum should be 1 for D */
}

void t_isalnum_0x45()
{
    uassert_int_equal(isalnum(69) , 1); /* isalnum should be 1 for E */
}

void t_isalnum_0x46()
{
    uassert_int_equal(isalnum(70) , 1); /* isalnum should be 1 for F */
}

void t_isalnum_0x47()
{
    uassert_int_equal(isalnum(71) , 1); /* isalnum should be 1 for G */
}

void t_isalnum_0x48()
{
    uassert_int_equal(isalnum(72) , 1); /* isalnum should be 1 for H */
}

void t_isalnum_0x49()
{
    uassert_int_equal(isalnum(73) , 1); /* isalnum should be 1 for I */
}

void t_isalnum_0x4a()
{
    uassert_int_equal(isalnum(74) , 1); /* isalnum should be 1 for J */
}

void t_isalnum_0x4b()
{
    uassert_int_equal(isalnum(75) , 1); /* isalnum should be 1 for K */
}

void t_isalnum_0x4c()
{
    uassert_int_equal(isalnum(76) , 1); /* isalnum should be 1 for L */
}

void t_isalnum_0x4d()
{
    uassert_int_equal(isalnum(77) , 1); /* isalnum should be 1 for M */
}

void t_isalnum_0x4e()
{
    uassert_int_equal(isalnum(78) , 1); /* isalnum should be 1 for N */
}

void t_isalnum_0x4f()
{
    uassert_int_equal(isalnum(79) , 1); /* isalnum should be 1 for O */
}

void t_isalnum_0x50()
{
    uassert_int_equal(isalnum(80) , 1); /* isalnum should be 1 for P */
}

void t_isalnum_0x51()
{
    uassert_int_equal(isalnum(81) , 1); /* isalnum should be 1 for Q */
}

void t_isalnum_0x52()
{
    uassert_int_equal(isalnum(82) , 1); /* isalnum should be 1 for R */
}

void t_isalnum_0x53()
{
    uassert_int_equal(isalnum(83) , 1); /* isalnum should be 1 for S */
}

void t_isalnum_0x54()
{
    uassert_int_equal(isalnum(84) , 1); /* isalnum should be 1 for T */
}

void t_isalnum_0x55()
{
    uassert_int_equal(isalnum(85) , 1); /* isalnum should be 1 for U */
}

void t_isalnum_0x56()
{
    uassert_int_equal(isalnum(86) , 1); /* isalnum should be 1 for V */
}

void t_isalnum_0x57()
{
    uassert_int_equal(isalnum(87) , 1); /* isalnum should be 1 for W */
}

void t_isalnum_0x58()
{
    uassert_int_equal(isalnum(88) , 1); /* isalnum should be 1 for X */
}

void t_isalnum_0x59()
{
    uassert_int_equal(isalnum(89) , 1); /* isalnum should be 1 for Y */
}

void t_isalnum_0x5a()
{
    uassert_int_equal(isalnum(90) , 1); /* isalnum should be 1 for Z */
}

void t_isalnum_0x5b()
{
    uassert_int_equal(isalnum(91), 0);  /* isalnum should be 0 for [ */
}

void t_isalnum_0x5c()
{
    uassert_int_equal(isalnum(92), 0);  /* isalnum should be 0 for 0x5c */
}

void t_isalnum_0x5d()
{
    uassert_int_equal(isalnum(93), 0);  /* isalnum should be 0 for ] */
}

void t_isalnum_0x5e()
{
    uassert_int_equal(isalnum(94), 0);  /* isalnum should be 0 for ^ */
}

void t_isalnum_0x5f()
{
    uassert_int_equal(isalnum(95), 0);  /* isalnum should be 0 for _ */
}

void t_isalnum_0x60()
{
    uassert_int_equal(isalnum(96), 0);  /* isalnum should be 0 for ` */
}

void t_isalnum_0x61()
{
    uassert_int_equal(isalnum(97) , 1); /* isalnum should be 1 for a */
}

void t_isalnum_0x62()
{
    uassert_int_equal(isalnum(98) , 1); /* isalnum should be 1 for b */
}

void t_isalnum_0x63()
{
    uassert_int_equal(isalnum(99) , 1); /* isalnum should be 1 for c */
}

void t_isalnum_0x64()
{
    uassert_int_equal(isalnum(100) , 1); /* isalnum should be 1 for d */
}

void t_isalnum_0x65()
{
    uassert_int_equal(isalnum(101) , 1); /* isalnum should be 1 for e */
}

void t_isalnum_0x66()
{
    uassert_int_equal(isalnum(102) , 1); /* isalnum should be 1 for f */
}

void t_isalnum_0x67()
{
    uassert_int_equal(isalnum(103) , 1); /* isalnum should be 1 for g */
}

void t_isalnum_0x68()
{
    uassert_int_equal(isalnum(104) , 1); /* isalnum should be 1 for h */
}

void t_isalnum_0x69()
{
    uassert_int_equal(isalnum(105) , 1); /* isalnum should be 1 for i */
}

void t_isalnum_0x6a()
{
    uassert_int_equal(isalnum(106) , 1); /* isalnum should be 1 for j */
}

void t_isalnum_0x6b()
{
    uassert_int_equal(isalnum(107) , 1); /* isalnum should be 1 for k */
}

void t_isalnum_0x6c()
{
    uassert_int_equal(isalnum(108) , 1); /* isalnum should be 1 for l */
}

void t_isalnum_0x6d()
{
    uassert_int_equal(isalnum(109) , 1); /* isalnum should be 1 for m */
}

void t_isalnum_0x6e()
{
    uassert_int_equal(isalnum(110) , 1); /* isalnum should be 1 for n */
}

void t_isalnum_0x6f()
{
    uassert_int_equal(isalnum(111) , 1); /* isalnum should be 1 for o */
}

void t_isalnum_0x70()
{
    uassert_int_equal(isalnum(112) , 1); /* isalnum should be 1 for p */
}

void t_isalnum_0x71()
{
    uassert_int_equal(isalnum(113) , 1); /* isalnum should be 1 for q */
}

void t_isalnum_0x72()
{
    uassert_int_equal(isalnum(114) , 1); /* isalnum should be 1 for r */
}

void t_isalnum_0x73()
{
    uassert_int_equal(isalnum(115) , 1); /* isalnum should be 1 for s */
}

void t_isalnum_0x74()
{
    uassert_int_equal(isalnum(116) , 1); /* isalnum should be 1 for t */
}

void t_isalnum_0x75()
{
    uassert_int_equal(isalnum(117) , 1); /* isalnum should be 1 for u */
}

void t_isalnum_0x76()
{
    uassert_int_equal(isalnum(118) , 1); /* isalnum should be 1 for v */
}

void t_isalnum_0x77()
{
    uassert_int_equal(isalnum(119) , 1); /* isalnum should be 1 for w */
}

void t_isalnum_0x78()
{
    uassert_int_equal(isalnum(120) , 1); /* isalnum should be 1 for x */
}

void t_isalnum_0x79()
{
    uassert_int_equal(isalnum(121) , 1); /* isalnum should be 1 for y */
}

void t_isalnum_0x7a()
{
    uassert_int_equal(isalnum(122) , 1); /* isalnum should be 1 for z */
}

void t_isalnum_0x7b()
{
    uassert_int_equal(isalnum(123), 0);  /* isalnum should be 0 for { */
}

void t_isalnum_0x7c()
{
    uassert_int_equal(isalnum(124), 0);  /* isalnum should be 0 for | */
}

void t_isalnum_0x7d()
{
    uassert_int_equal(isalnum(125), 0);  /* isalnum should be 0 for } */
}

void t_isalnum_0x7e()
{
    uassert_int_equal(isalnum(126), 0);  /* isalnum should be 0 for ~ */
}

void t_isalnum_0x7f()
{
    uassert_int_equal(isalnum(127), 0);  /* isalnum should be 0 for 0x7f */
}

void t_isalnum_0x80()
{
    uassert_int_equal(isalnum(128), 0);  /* isalnum should be 0 for 0x80 */
}

void t_isalnum_0x81()
{
    uassert_int_equal(isalnum(129), 0);  /* isalnum should be 0 for 0x81 */
}

void t_isalnum_0x82()
{
    uassert_int_equal(isalnum(130), 0);  /* isalnum should be 0 for 0x82 */
}

void t_isalnum_0x83()
{
    uassert_int_equal(isalnum(131), 0);  /* isalnum should be 0 for 0x83 */
}

void t_isalnum_0x84()
{
    uassert_int_equal(isalnum(132), 0);  /* isalnum should be 0 for 0x84 */
}

void t_isalnum_0x85()
{
    uassert_int_equal(isalnum(133), 0);  /* isalnum should be 0 for 0x85 */
}

void t_isalnum_0x86()
{
    uassert_int_equal(isalnum(134), 0);  /* isalnum should be 0 for 0x86 */
}

void t_isalnum_0x87()
{
    uassert_int_equal(isalnum(135), 0);  /* isalnum should be 0 for 0x87 */
}

void t_isalnum_0x88()
{
    uassert_int_equal(isalnum(136), 0);  /* isalnum should be 0 for 0x88 */
}

void t_isalnum_0x89()
{
    uassert_int_equal(isalnum(137), 0);  /* isalnum should be 0 for 0x89 */
}

void t_isalnum_0x8a()
{
    uassert_int_equal(isalnum(138), 0);  /* isalnum should be 0 for 0x8a */
}

void t_isalnum_0x8b()
{
    uassert_int_equal(isalnum(139), 0);  /* isalnum should be 0 for 0x8b */
}

void t_isalnum_0x8c()
{
    uassert_int_equal(isalnum(140), 0);  /* isalnum should be 0 for 0x8c */
}

void t_isalnum_0x8d()
{
    uassert_int_equal(isalnum(141), 0);  /* isalnum should be 0 for 0x8d */
}

void t_isalnum_0x8e()
{
    uassert_int_equal(isalnum(142), 0);  /* isalnum should be 0 for 0x8e */
}

void t_isalnum_0x8f()
{
    uassert_int_equal(isalnum(143), 0);  /* isalnum should be 0 for 0x8f */
}

void t_isalnum_0x90()
{
    uassert_int_equal(isalnum(144), 0);  /* isalnum should be 0 for 0x90 */
}

void t_isalnum_0x91()
{
    uassert_int_equal(isalnum(145), 0);  /* isalnum should be 0 for 0x91 */
}

void t_isalnum_0x92()
{
    uassert_int_equal(isalnum(146), 0);  /* isalnum should be 0 for 0x92 */
}

void t_isalnum_0x93()
{
    uassert_int_equal(isalnum(147), 0);  /* isalnum should be 0 for 0x93 */
}

void t_isalnum_0x94()
{
    uassert_int_equal(isalnum(148), 0);  /* isalnum should be 0 for 0x94 */
}

void t_isalnum_0x95()
{
    uassert_int_equal(isalnum(149), 0);  /* isalnum should be 0 for 0x95 */
}

void t_isalnum_0x96()
{
    uassert_int_equal(isalnum(150), 0);  /* isalnum should be 0 for 0x96 */
}

void t_isalnum_0x97()
{
    uassert_int_equal(isalnum(151), 0);  /* isalnum should be 0 for 0x97 */
}

void t_isalnum_0x98()
{
    uassert_int_equal(isalnum(152), 0);  /* isalnum should be 0 for 0x98 */
}

void t_isalnum_0x99()
{
    uassert_int_equal(isalnum(153), 0);  /* isalnum should be 0 for 0x99 */
}

void t_isalnum_0x9a()
{
    uassert_int_equal(isalnum(154), 0);  /* isalnum should be 0 for 0x9a */
}

void t_isalnum_0x9b()
{
    uassert_int_equal(isalnum(155), 0);  /* isalnum should be 0 for 0x9b */
}

void t_isalnum_0x9c()
{
    uassert_int_equal(isalnum(156), 0);  /* isalnum should be 0 for 0x9c */
}

void t_isalnum_0x9d()
{
    uassert_int_equal(isalnum(157), 0);  /* isalnum should be 0 for 0x9d */
}

void t_isalnum_0x9e()
{
    uassert_int_equal(isalnum(158), 0);  /* isalnum should be 0 for 0x9e */
}

void t_isalnum_0x9f()
{
    uassert_int_equal(isalnum(159), 0);  /* isalnum should be 0 for 0x9f */
}

void t_isalnum_0xa0()
{
    uassert_int_equal(isalnum(160), 0);  /* isalnum should be 0 for 0xa0 */
}

void t_isalnum_0xa1()
{
    uassert_int_equal(isalnum(161), 0);  /* isalnum should be 0 for 0xa1 */
}

void t_isalnum_0xa2()
{
    uassert_int_equal(isalnum(162), 0);  /* isalnum should be 0 for 0xa2 */
}

void t_isalnum_0xa3()
{
    uassert_int_equal(isalnum(163), 0);  /* isalnum should be 0 for 0xa3 */
}

void t_isalnum_0xa4()
{
    uassert_int_equal(isalnum(164), 0);  /* isalnum should be 0 for 0xa4 */
}

void t_isalnum_0xa5()
{
    uassert_int_equal(isalnum(165), 0);  /* isalnum should be 0 for 0xa5 */
}

void t_isalnum_0xa6()
{
    uassert_int_equal(isalnum(166), 0);  /* isalnum should be 0 for 0xa6 */
}

void t_isalnum_0xa7()
{
    uassert_int_equal(isalnum(167), 0);  /* isalnum should be 0 for 0xa7 */
}

void t_isalnum_0xa8()
{
    uassert_int_equal(isalnum(168), 0);  /* isalnum should be 0 for 0xa8 */
}

void t_isalnum_0xa9()
{
    uassert_int_equal(isalnum(169), 0);  /* isalnum should be 0 for 0xa9 */
}

void t_isalnum_0xaa()
{
    uassert_int_equal(isalnum(170), 0);  /* isalnum should be 0 for 0xaa */
}

void t_isalnum_0xab()
{
    uassert_int_equal(isalnum(171), 0);  /* isalnum should be 0 for 0xab */
}

void t_isalnum_0xac()
{
    uassert_int_equal(isalnum(172), 0);  /* isalnum should be 0 for 0xac */
}

void t_isalnum_0xad()
{
    uassert_int_equal(isalnum(173), 0);  /* isalnum should be 0 for 0xad */
}

void t_isalnum_0xae()
{
    uassert_int_equal(isalnum(174), 0);  /* isalnum should be 0 for 0xae */
}

void t_isalnum_0xaf()
{
    uassert_int_equal(isalnum(175), 0);  /* isalnum should be 0 for 0xaf */
}

void t_isalnum_0xb0()
{
    uassert_int_equal(isalnum(176), 0);  /* isalnum should be 0 for 0xb0 */
}

void t_isalnum_0xb1()
{
    uassert_int_equal(isalnum(177), 0);  /* isalnum should be 0 for 0xb1 */
}

void t_isalnum_0xb2()
{
    uassert_int_equal(isalnum(178), 0);  /* isalnum should be 0 for 0xb2 */
}

void t_isalnum_0xb3()
{
    uassert_int_equal(isalnum(179), 0);  /* isalnum should be 0 for 0xb3 */
}

void t_isalnum_0xb4()
{
    uassert_int_equal(isalnum(180), 0);  /* isalnum should be 0 for 0xb4 */
}

void t_isalnum_0xb5()
{
    uassert_int_equal(isalnum(181), 0);  /* isalnum should be 0 for 0xb5 */
}

void t_isalnum_0xb6()
{
    uassert_int_equal(isalnum(182), 0);  /* isalnum should be 0 for 0xb6 */
}

void t_isalnum_0xb7()
{
    uassert_int_equal(isalnum(183), 0);  /* isalnum should be 0 for 0xb7 */
}

void t_isalnum_0xb8()
{
    uassert_int_equal(isalnum(184), 0);  /* isalnum should be 0 for 0xb8 */
}

void t_isalnum_0xb9()
{
    uassert_int_equal(isalnum(185), 0);  /* isalnum should be 0 for 0xb9 */
}

void t_isalnum_0xba()
{
    uassert_int_equal(isalnum(186), 0);  /* isalnum should be 0 for 0xba */
}

void t_isalnum_0xbb()
{
    uassert_int_equal(isalnum(187), 0);  /* isalnum should be 0 for 0xbb */
}

void t_isalnum_0xbc()
{
    uassert_int_equal(isalnum(188), 0);  /* isalnum should be 0 for 0xbc */
}

void t_isalnum_0xbd()
{
    uassert_int_equal(isalnum(189), 0);  /* isalnum should be 0 for 0xbd */
}

void t_isalnum_0xbe()
{
    uassert_int_equal(isalnum(190), 0);  /* isalnum should be 0 for 0xbe */
}

void t_isalnum_0xbf()
{
    uassert_int_equal(isalnum(191), 0);  /* isalnum should be 0 for 0xbf */
}

void t_isalnum_0xc0()
{
    uassert_int_equal(isalnum(192), 0);  /* isalnum should be 0 for 0xc0 */
}

void t_isalnum_0xc1()
{
    uassert_int_equal(isalnum(193), 0);  /* isalnum should be 0 for 0xc1 */
}

void t_isalnum_0xc2()
{
    uassert_int_equal(isalnum(194), 0);  /* isalnum should be 0 for 0xc2 */
}

void t_isalnum_0xc3()
{
    uassert_int_equal(isalnum(195), 0);  /* isalnum should be 0 for 0xc3 */
}

void t_isalnum_0xc4()
{
    uassert_int_equal(isalnum(196), 0);  /* isalnum should be 0 for 0xc4 */
}

void t_isalnum_0xc5()
{
    uassert_int_equal(isalnum(197), 0);  /* isalnum should be 0 for 0xc5 */
}

void t_isalnum_0xc6()
{
    uassert_int_equal(isalnum(198), 0);  /* isalnum should be 0 for 0xc6 */
}

void t_isalnum_0xc7()
{
    uassert_int_equal(isalnum(199), 0);  /* isalnum should be 0 for 0xc7 */
}

void t_isalnum_0xc8()
{
    uassert_int_equal(isalnum(200), 0);  /* isalnum should be 0 for 0xc8 */
}

void t_isalnum_0xc9()
{
    uassert_int_equal(isalnum(201), 0);  /* isalnum should be 0 for 0xc9 */
}

void t_isalnum_0xca()
{
    uassert_int_equal(isalnum(202), 0);  /* isalnum should be 0 for 0xca */
}

void t_isalnum_0xcb()
{
    uassert_int_equal(isalnum(203), 0);  /* isalnum should be 0 for 0xcb */
}

void t_isalnum_0xcc()
{
    uassert_int_equal(isalnum(204), 0);  /* isalnum should be 0 for 0xcc */
}

void t_isalnum_0xcd()
{
    uassert_int_equal(isalnum(205), 0);  /* isalnum should be 0 for 0xcd */
}

void t_isalnum_0xce()
{
    uassert_int_equal(isalnum(206), 0);  /* isalnum should be 0 for 0xce */
}

void t_isalnum_0xcf()
{
    uassert_int_equal(isalnum(207), 0);  /* isalnum should be 0 for 0xcf */
}

void t_isalnum_0xd0()
{
    uassert_int_equal(isalnum(208), 0);  /* isalnum should be 0 for 0xd0 */
}

void t_isalnum_0xd1()
{
    uassert_int_equal(isalnum(209), 0);  /* isalnum should be 0 for 0xd1 */
}

void t_isalnum_0xd2()
{
    uassert_int_equal(isalnum(210), 0);  /* isalnum should be 0 for 0xd2 */
}

void t_isalnum_0xd3()
{
    uassert_int_equal(isalnum(211), 0);  /* isalnum should be 0 for 0xd3 */
}

void t_isalnum_0xd4()
{
    uassert_int_equal(isalnum(212), 0);  /* isalnum should be 0 for 0xd4 */
}

void t_isalnum_0xd5()
{
    uassert_int_equal(isalnum(213), 0);  /* isalnum should be 0 for 0xd5 */
}

void t_isalnum_0xd6()
{
    uassert_int_equal(isalnum(214), 0);  /* isalnum should be 0 for 0xd6 */
}

void t_isalnum_0xd7()
{
    uassert_int_equal(isalnum(215), 0);  /* isalnum should be 0 for 0xd7 */
}

void t_isalnum_0xd8()
{
    uassert_int_equal(isalnum(216), 0);  /* isalnum should be 0 for 0xd8 */
}

void t_isalnum_0xd9()
{
    uassert_int_equal(isalnum(217), 0);  /* isalnum should be 0 for 0xd9 */
}

void t_isalnum_0xda()
{
    uassert_int_equal(isalnum(218), 0);  /* isalnum should be 0 for 0xda */
}

void t_isalnum_0xdb()
{
    uassert_int_equal(isalnum(219), 0);  /* isalnum should be 0 for 0xdb */
}

void t_isalnum_0xdc()
{
    uassert_int_equal(isalnum(220), 0);  /* isalnum should be 0 for 0xdc */
}

void t_isalnum_0xdd()
{
    uassert_int_equal(isalnum(221), 0);  /* isalnum should be 0 for 0xdd */
}

void t_isalnum_0xde()
{
    uassert_int_equal(isalnum(222), 0);  /* isalnum should be 0 for 0xde */
}

void t_isalnum_0xdf()
{
    uassert_int_equal(isalnum(223), 0);  /* isalnum should be 0 for 0xdf */
}

void t_isalnum_0xe0()
{
    uassert_int_equal(isalnum(224), 0);  /* isalnum should be 0 for 0xe0 */
}

void t_isalnum_0xe1()
{
    uassert_int_equal(isalnum(225), 0);  /* isalnum should be 0 for 0xe1 */
}

void t_isalnum_0xe2()
{
    uassert_int_equal(isalnum(226), 0);  /* isalnum should be 0 for 0xe2 */
}

void t_isalnum_0xe3()
{
    uassert_int_equal(isalnum(227), 0);  /* isalnum should be 0 for 0xe3 */
}

void t_isalnum_0xe4()
{
    uassert_int_equal(isalnum(228), 0);  /* isalnum should be 0 for 0xe4 */
}

void t_isalnum_0xe5()
{
    uassert_int_equal(isalnum(229), 0);  /* isalnum should be 0 for 0xe5 */
}

void t_isalnum_0xe6()
{
    uassert_int_equal(isalnum(230), 0);  /* isalnum should be 0 for 0xe6 */
}

void t_isalnum_0xe7()
{
    uassert_int_equal(isalnum(231), 0);  /* isalnum should be 0 for 0xe7 */
}

void t_isalnum_0xe8()
{
    uassert_int_equal(isalnum(232), 0);  /* isalnum should be 0 for 0xe8 */
}

void t_isalnum_0xe9()
{
    uassert_int_equal(isalnum(233), 0);  /* isalnum should be 0 for 0xe9 */
}

void t_isalnum_0xea()
{
    uassert_int_equal(isalnum(234), 0);  /* isalnum should be 0 for 0xea */
}

void t_isalnum_0xeb()
{
    uassert_int_equal(isalnum(235), 0);  /* isalnum should be 0 for 0xeb */
}

void t_isalnum_0xec()
{
    uassert_int_equal(isalnum(236), 0);  /* isalnum should be 0 for 0xec */
}

void t_isalnum_0xed()
{
    uassert_int_equal(isalnum(237), 0);  /* isalnum should be 0 for 0xed */
}

void t_isalnum_0xee()
{
    uassert_int_equal(isalnum(238), 0);  /* isalnum should be 0 for 0xee */
}

void t_isalnum_0xef()
{
    uassert_int_equal(isalnum(239), 0);  /* isalnum should be 0 for 0xef */
}

void t_isalnum_0xf0()
{
    uassert_int_equal(isalnum(240), 0);  /* isalnum should be 0 for 0xf0 */
}

void t_isalnum_0xf1()
{
    uassert_int_equal(isalnum(241), 0);  /* isalnum should be 0 for 0xf1 */
}

void t_isalnum_0xf2()
{
    uassert_int_equal(isalnum(242), 0);  /* isalnum should be 0 for 0xf2 */
}

void t_isalnum_0xf3()
{
    uassert_int_equal(isalnum(243), 0);  /* isalnum should be 0 for 0xf3 */
}

void t_isalnum_0xf4()
{
    uassert_int_equal(isalnum(244), 0);  /* isalnum should be 0 for 0xf4 */
}

void t_isalnum_0xf5()
{
    uassert_int_equal(isalnum(245), 0);  /* isalnum should be 0 for 0xf5 */
}

void t_isalnum_0xf6()
{
    uassert_int_equal(isalnum(246), 0);  /* isalnum should be 0 for 0xf6 */
}

void t_isalnum_0xf7()
{
    uassert_int_equal(isalnum(247), 0);  /* isalnum should be 0 for 0xf7 */
}

void t_isalnum_0xf8()
{
    uassert_int_equal(isalnum(248), 0);  /* isalnum should be 0 for 0xf8 */
}

void t_isalnum_0xf9()
{
    uassert_int_equal(isalnum(249), 0);  /* isalnum should be 0 for 0xf9 */
}

void t_isalnum_0xfa()
{
    uassert_int_equal(isalnum(250), 0);  /* isalnum should be 0 for 0xfa */
}

void t_isalnum_0xfb()
{
    uassert_int_equal(isalnum(251), 0);  /* isalnum should be 0 for 0xfb */
}

void t_isalnum_0xfc()
{
    uassert_int_equal(isalnum(252), 0);  /* isalnum should be 0 for 0xfc */
}

void t_isalnum_0xfd()
{
    uassert_int_equal(isalnum(253), 0);  /* isalnum should be 0 for 0xfd */
}

void t_isalnum_0xfe()
{
    uassert_int_equal(isalnum(254), 0);  /* isalnum should be 0 for 0xfe */
}

void t_isalnum_0xff()
{
    uassert_int_equal(isalnum(255), 0);  /* isalnum should be 0 for 0xff */
}

static int testcase(void)
{
    t_isalnum_0x00();
    t_isalnum_0x01();
    t_isalnum_0x02();
    t_isalnum_0x03();
    t_isalnum_0x04();
    t_isalnum_0x05();
    t_isalnum_0x06();
    t_isalnum_0x07();
    t_isalnum_0x08();
    t_isalnum_0x09();
    t_isalnum_0x0a();
    t_isalnum_0x0b();
    t_isalnum_0x0c();
    t_isalnum_0x0d();
    t_isalnum_0x0e();
    t_isalnum_0x0f();
    t_isalnum_0x10();
    t_isalnum_0x11();
    t_isalnum_0x12();
    t_isalnum_0x13();
    t_isalnum_0x14();
    t_isalnum_0x15();
    t_isalnum_0x16();
    t_isalnum_0x17();
    t_isalnum_0x18();
    t_isalnum_0x19();
    t_isalnum_0x1a();
    t_isalnum_0x1b();
    t_isalnum_0x1c();
    t_isalnum_0x1d();
    t_isalnum_0x1e();
    t_isalnum_0x1f();
    t_isalnum_0x20();
    t_isalnum_0x21();
    t_isalnum_0x22();
    t_isalnum_0x23();
    t_isalnum_0x24();
    t_isalnum_0x25();
    t_isalnum_0x26();
    t_isalnum_0x27();
    t_isalnum_0x28();
    t_isalnum_0x29();
    t_isalnum_0x2a();
    t_isalnum_0x2b();
    t_isalnum_0x2c();
    t_isalnum_0x2d();
    t_isalnum_0x2e();
    t_isalnum_0x2f();
    t_isalnum_0x30();
    t_isalnum_0x31();
    t_isalnum_0x32();
    t_isalnum_0x33();
    t_isalnum_0x34();
    t_isalnum_0x35();
    t_isalnum_0x36();
    t_isalnum_0x37();
    t_isalnum_0x38();
    t_isalnum_0x39();
    t_isalnum_0x3a();
    t_isalnum_0x3b();
    t_isalnum_0x3c();
    t_isalnum_0x3d();
    t_isalnum_0x3e();
    t_isalnum_0x3f();
    t_isalnum_0x40();
    t_isalnum_0x41();
    t_isalnum_0x42();
    t_isalnum_0x43();
    t_isalnum_0x44();
    t_isalnum_0x45();
    t_isalnum_0x46();
    t_isalnum_0x47();
    t_isalnum_0x48();
    t_isalnum_0x49();
    t_isalnum_0x4a();
    t_isalnum_0x4b();
    t_isalnum_0x4c();
    t_isalnum_0x4d();
    t_isalnum_0x4e();
    t_isalnum_0x4f();
    t_isalnum_0x50();
    t_isalnum_0x51();
    t_isalnum_0x52();
    t_isalnum_0x53();
    t_isalnum_0x54();
    t_isalnum_0x55();
    t_isalnum_0x56();
    t_isalnum_0x57();
    t_isalnum_0x58();
    t_isalnum_0x59();
    t_isalnum_0x5a();
    t_isalnum_0x5b();
    t_isalnum_0x5c();
    t_isalnum_0x5d();
    t_isalnum_0x5e();
    t_isalnum_0x5f();
    t_isalnum_0x60();
    t_isalnum_0x61();
    t_isalnum_0x62();
    t_isalnum_0x63();
    t_isalnum_0x64();
    t_isalnum_0x65();
    t_isalnum_0x66();
    t_isalnum_0x67();
    t_isalnum_0x68();
    t_isalnum_0x69();
    t_isalnum_0x6a();
    t_isalnum_0x6b();
    t_isalnum_0x6c();
    t_isalnum_0x6d();
    t_isalnum_0x6e();
    t_isalnum_0x6f();
    t_isalnum_0x70();
    t_isalnum_0x71();
    t_isalnum_0x72();
    t_isalnum_0x73();
    t_isalnum_0x74();
    t_isalnum_0x75();
    t_isalnum_0x76();
    t_isalnum_0x77();
    t_isalnum_0x78();
    t_isalnum_0x79();
    t_isalnum_0x7a();
    t_isalnum_0x7b();
    t_isalnum_0x7c();
    t_isalnum_0x7d();
    t_isalnum_0x7e();
    t_isalnum_0x7f();
    t_isalnum_0x80();
    t_isalnum_0x81();
    t_isalnum_0x82();
    t_isalnum_0x83();
    t_isalnum_0x84();
    t_isalnum_0x85();
    t_isalnum_0x86();
    t_isalnum_0x87();
    t_isalnum_0x88();
    t_isalnum_0x89();
    t_isalnum_0x8a();
    t_isalnum_0x8b();
    t_isalnum_0x8c();
    t_isalnum_0x8d();
    t_isalnum_0x8e();
    t_isalnum_0x8f();
    t_isalnum_0x90();
    t_isalnum_0x91();
    t_isalnum_0x92();
    t_isalnum_0x93();
    t_isalnum_0x94();
    t_isalnum_0x95();
    t_isalnum_0x96();
    t_isalnum_0x97();
    t_isalnum_0x98();
    t_isalnum_0x99();
    t_isalnum_0x9a();
    t_isalnum_0x9b();
    t_isalnum_0x9c();
    t_isalnum_0x9d();
    t_isalnum_0x9e();
    t_isalnum_0x9f();
    t_isalnum_0xa0();
    t_isalnum_0xa1();
    t_isalnum_0xa2();
    t_isalnum_0xa3();
    t_isalnum_0xa4();
    t_isalnum_0xa5();
    t_isalnum_0xa6();
    t_isalnum_0xa7();
    t_isalnum_0xa8();
    t_isalnum_0xa9();
    t_isalnum_0xaa();
    t_isalnum_0xab();
    t_isalnum_0xac();
    t_isalnum_0xad();
    t_isalnum_0xae();
    t_isalnum_0xaf();
    t_isalnum_0xb0();
    t_isalnum_0xb1();
    t_isalnum_0xb2();
    t_isalnum_0xb3();
    t_isalnum_0xb4();
    t_isalnum_0xb5();
    t_isalnum_0xb6();
    t_isalnum_0xb7();
    t_isalnum_0xb8();
    t_isalnum_0xb9();
    t_isalnum_0xba();
    t_isalnum_0xbb();
    t_isalnum_0xbc();
    t_isalnum_0xbd();
    t_isalnum_0xbe();
    t_isalnum_0xbf();
    t_isalnum_0xc0();
    t_isalnum_0xc1();
    t_isalnum_0xc2();
    t_isalnum_0xc3();
    t_isalnum_0xc4();
    t_isalnum_0xc5();
    t_isalnum_0xc6();
    t_isalnum_0xc7();
    t_isalnum_0xc8();
    t_isalnum_0xc9();
    t_isalnum_0xca();
    t_isalnum_0xcb();
    t_isalnum_0xcc();
    t_isalnum_0xcd();
    t_isalnum_0xce();
    t_isalnum_0xcf();
    t_isalnum_0xd0();
    t_isalnum_0xd1();
    t_isalnum_0xd2();
    t_isalnum_0xd3();
    t_isalnum_0xd4();
    t_isalnum_0xd5();
    t_isalnum_0xd6();
    t_isalnum_0xd7();
    t_isalnum_0xd8();
    t_isalnum_0xd9();
    t_isalnum_0xda();
    t_isalnum_0xdb();
    t_isalnum_0xdc();
    t_isalnum_0xdd();
    t_isalnum_0xde();
    t_isalnum_0xdf();
    t_isalnum_0xe0();
    t_isalnum_0xe1();
    t_isalnum_0xe2();
    t_isalnum_0xe3();
    t_isalnum_0xe4();
    t_isalnum_0xe5();
    t_isalnum_0xe6();
    t_isalnum_0xe7();
    t_isalnum_0xe8();
    t_isalnum_0xe9();
    t_isalnum_0xea();
    t_isalnum_0xeb();
    t_isalnum_0xec();
    t_isalnum_0xed();
    t_isalnum_0xee();
    t_isalnum_0xef();
    t_isalnum_0xf0();
    t_isalnum_0xf1();
    t_isalnum_0xf2();
    t_isalnum_0xf3();
    t_isalnum_0xf4();
    t_isalnum_0xf5();
    t_isalnum_0xf6();
    t_isalnum_0xf7();
    t_isalnum_0xf8();
    t_isalnum_0xf9();
    t_isalnum_0xfa();
    t_isalnum_0xfb();
    t_isalnum_0xfc();
    t_isalnum_0xfd();
    t_isalnum_0xfe();
    t_isalnum_0xff();
}

int main(void)
{
    testcase();
    printf("{Test PASSED}\n");
    return 0;
}

