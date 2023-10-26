// Copyright (C) 1988-1994 Sun Microsystems, Inc. 2550 Garcia Avenue
// Mountain View, California  94043 All rights reserved.
//
// Any person is hereby authorized to download, copy, use, create bug fixes,
// and distribute, subject to the following conditions:
//
// 	1.  the software may not be redistributed for a fee except as
// 	    reasonable to cover media costs;
// 	2.  any copy of the software must include this notice, as well as
// 	    any other embedded copyright notices; and
// 	3.  any distribution of this software or derivative works thereof
// 	    must comply with all applicable U.S. export control laws.
//
// THE SOFTWARE IS MADE AVAILABLE "AS IS" AND WITHOUT EXPRESS OR IMPLIED
// WARRANTY OF ANY KIND, INCLUDING BUT NOT LIMITED TO THE IMPLIED
// WARRANTIES OF DESIGN, MERCHANTIBILITY, FITNESS FOR A PARTICULAR
// PURPOSE, NON-INFRINGEMENT, PERFORMANCE OR CONFORMANCE TO
// SPECIFICATIONS.
//
// BY DOWNLOADING AND/OR USING THIS SOFTWARE, THE USER WAIVES ALL CLAIMS
// AGAINST SUN MICROSYSTEMS, INC. AND ITS AFFILIATED COMPANIES IN ANY
// JURISDICTION, INCLUDING BUT NOT LIMITED TO CLAIMS FOR DAMAGES OR
// EQUITABLE RELIEF BASED ON LOSS OF DATA, AND SPECIFICALLY WAIVES EVEN
// UNKNOWN OR UNANTICIPATED CLAIMS OR LOSSES, PRESENT AND FUTURE.
//
// IN NO EVENT WILL SUN MICROSYSTEMS, INC. OR ANY OF ITS AFFILIATED
// COMPANIES BE LIABLE FOR ANY LOST REVENUE OR PROFITS OR OTHER SPECIAL,
// INDIRECT AND CONSEQUENTIAL DAMAGES, EVEN IF IT HAS BEEN ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGES.
//
// This file is provided with no support and without any obligation on the
// part of Sun Microsystems, Inc. ("Sun") or any of its affiliated
// companies to assist in its use, correction, modification or
// enhancement.  Nevertheless, and without creating any obligation on its
// part, Sun welcomes your comments concerning the software and requests
// that they be sent to fdlibm-comments@sunpro.sun.com.
// overflow boundary
T(RN,    0x1.633ce8fb9f87dp+9, 0x1.ffffffffffd3bp+1023,   0x1.a6b164p-4, INEXACT)
T(RZ,    0x1.633ce8fb9f87dp+9, 0x1.ffffffffffd3ap+1023,  -0x1.cb29d4p-1, INEXACT)
T(RU,    0x1.633ce8fb9f87dp+9, 0x1.ffffffffffd3bp+1023,   0x1.a6b164p-4, INEXACT)
T(RD,    0x1.633ce8fb9f87dp+9, 0x1.ffffffffffd3ap+1023,  -0x1.cb29d4p-1, INEXACT)
T(RN,   -0x1.633ce8fb9f87dp+9, 0x1.ffffffffffd3bp+1023,   0x1.a6b164p-4, INEXACT)
T(RN,    0x1.633ce8fb9f87ep+9,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN,   -0x1.633ce8fb9f87ep+9,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RZ,   -0x1.633ce8fb9f87ep+9, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RU,   -0x1.633ce8fb9f87ep+9,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RD,   -0x1.633ce8fb9f87ep+9, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
// coshd(0 or tiny) :=: 1.0
T(RN,                 0x1p-67,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,                -0x1p-67,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,               0x1p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,              -0x1p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,               0x1p-1074,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,              -0x1p-1074,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,                  0x0p+0,                  0x1p+0,          0x0p+0, 0)
T(RN,                 -0x0p+0,                  0x1p+0,          0x0p+0, 0)
// random arguments between -50,50
T(RN,   -0x1.adeefb2b5006dp+3,   0x1.4de3262eea3cap+18,   0x1.e30caep-3, INEXACT)
T(RN,    0x1.1ce3efb825911p+5,   0x1.4c46db4f09332p+50,  -0x1.1bee26p-3, INEXACT)
T(RN,    0x1.602e109de7505p+5,   0x1.6cd152f852538p+62,   0x1.432474p-2, INEXACT)
T(RN,   -0x1.0b245fba96889p+5,   0x1.2121104afc5efp+47,    0x1.b8303p-2, INEXACT)
T(RN,   -0x1.b171ee27084ddp+3,   0x1.749cc0eb38f31p+18,   0x1.1a3cf8p-3, INEXACT)
T(RN,   -0x1.f6eff1b093c41p+0,    0x1.d16cbf8794c45p+1,  -0x1.6a0662p-3, INEXACT)
T(RN,    0x1.ceaa3d18455f5p+4,   0x1.a507cd0be14cdp+40,   0x1.40d57cp-4, INEXACT)
T(RN,    0x1.560914a51b239p+5,   0x1.9a9b0ddd8b0c7p+60,   0x1.5db126p-2, INEXACT)
T(RN,   -0x1.0ce901079de4dp+3,   0x1.16e676fb41d68p+11,   0x1.5681cap-3, INEXACT)
T(RN,   -0x1.7f35b3103b871p+5,   0x1.13ae32648dd07p+68,     0x1.ebb6p-4, INEXACT)
// coshd(nan/inf)
T(RN,                     nan,                     nan,          0x0p+0, 0)
T(RN,                     nan,                     nan,          0x0p+0, 0)
T(RN,                     inf,                     inf,          0x0p+0, 0)
T(RN,                    -inf,                     inf,          0x0p+0, 0)
T(RD,                     inf,                     inf,          0x0p+0, 0)
T(RD,                 -0x0p+0,                  0x1p+0,          0x0p+0, 0)
T(RD,                    -inf,                     inf,          0x0p+0, 0)
T(RD,               0x1p-1074,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,               0x1p-1073,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,               0x1p-1024,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,               0x1p-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RD, 0x1.ffffffffffffcp-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RD, 0x1.ffffffffffffep-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,               0x1p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RD, 0x1.0000000000001p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RD, 0x1.0000000000002p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,               0x1p-1021,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,               0x1p-1020,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,                 0x1p-27,                  0x1p+0,         -0x1p-3, INEXACT)
T(RD,              -0x1p-1074,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,              -0x1p-1073,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,              -0x1p-1024,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,              -0x1p-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,-0x1.ffffffffffffcp-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,-0x1.ffffffffffffep-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,              -0x1p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,-0x1.0000000000001p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,-0x1.0000000000002p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,              -0x1p-1021,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,              -0x1p-1020,                  0x1p+0,          0x0p+0, INEXACT)
T(RD,                -0x1p-27,                  0x1p+0,         -0x1p-3, INEXACT)
T(RD,              0x1.634p+9, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD,               0x1p+1022, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD,               0x1p+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD, 0x1.ffffffffffffep+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD, 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD,             -0x1.634p+9, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD,              -0x1p+1022, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD,              -0x1p+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD,-0x1.ffffffffffffep+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD,-0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RD,                     nan,                     nan,          0x0p+0, 0)
T(RD,                     nan,                     nan,          0x0p+0, 0)
T(RD,                 0x1p-25,    0x1.0000000000002p+0, -0x1.555554p-53, INEXACT)
T(RD,                -0x1p-25,    0x1.0000000000002p+0, -0x1.555554p-53, INEXACT)
T(RN,               0x1p-1073,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,               0x1p-1024,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,               0x1p-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RN, 0x1.ffffffffffffcp-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RN, 0x1.ffffffffffffep-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RN, 0x1.0000000000001p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RN, 0x1.0000000000002p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,               0x1p-1021,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,               0x1p-1020,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,                 0x1p-27,                  0x1p+0,         -0x1p-3, INEXACT)
T(RN,                 0x1p-25,    0x1.0000000000002p+0, -0x1.555554p-53, INEXACT)
T(RN,              -0x1p-1073,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,              -0x1p-1024,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,              -0x1p-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,-0x1.ffffffffffffcp-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,-0x1.ffffffffffffep-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,-0x1.0000000000001p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,-0x1.0000000000002p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,              -0x1p-1021,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,              -0x1p-1020,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,                -0x1p-27,                  0x1p+0,         -0x1p-3, INEXACT)
T(RN,                -0x1p-25,    0x1.0000000000002p+0, -0x1.555554p-53, INEXACT)
T(RN,              0x1.634p+9,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN,               0x1p+1022,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN,               0x1p+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN, 0x1.ffffffffffffep+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN, 0x1.fffffffffffffp+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN,             -0x1.634p+9,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN,              -0x1p+1022,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN,              -0x1p+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN,-0x1.ffffffffffffep+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RN,-0x1.fffffffffffffp+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU,                  0x0p+0,                  0x1p+0,          0x0p+0, 0)
T(RU,                     inf,                     inf,          0x0p+0, 0)
T(RU,                 -0x0p+0,                  0x1p+0,          0x0p+0, 0)
T(RU,                    -inf,                     inf,          0x0p+0, 0)
T(RU,              0x1.634p+9,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU,               0x1p+1022,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU,               0x1p+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU, 0x1.ffffffffffffep+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU, 0x1.fffffffffffffp+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU,             -0x1.634p+9,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU,              -0x1p+1022,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU,              -0x1p+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU,-0x1.ffffffffffffep+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU,-0x1.fffffffffffffp+1023,                     inf,          0x0p+0, INEXACT|OVERFLOW)
T(RU,                     nan,                     nan,          0x0p+0, 0)
T(RU,                     nan,                     nan,          0x0p+0, 0)
T(RU,               0x1p-1074,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,               0x1p-1073,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,               0x1p-1024,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,               0x1p-1023,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU, 0x1.ffffffffffffcp-1023,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU, 0x1.ffffffffffffep-1023,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,               0x1p-1022,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU, 0x1.0000000000001p-1022,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU, 0x1.0000000000002p-1022,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,               0x1p-1021,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,               0x1p-1020,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,                 0x1p-27,    0x1.0000000000001p+0,        0x1.cp-1, INEXACT)
T(RU,                 0x1p-25,    0x1.0000000000003p+0,          0x1p+0, INEXACT)
T(RU,              -0x1p-1074,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,              -0x1p-1073,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,              -0x1p-1024,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,              -0x1p-1023,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,-0x1.ffffffffffffcp-1023,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,-0x1.ffffffffffffep-1023,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,              -0x1p-1022,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,-0x1.0000000000001p-1022,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,-0x1.0000000000002p-1022,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,              -0x1p-1021,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,              -0x1p-1020,    0x1.0000000000001p+0,          0x1p+0, INEXACT)
T(RU,                -0x1p-27,    0x1.0000000000001p+0,        0x1.cp-1, INEXACT)
T(RU,                -0x1p-25,    0x1.0000000000003p+0,          0x1p+0, INEXACT)
T(RZ,                  0x0p+0,                  0x1p+0,          0x0p+0, 0)
T(RZ,                     inf,                     inf,          0x0p+0, 0)
T(RZ,                 -0x0p+0,                  0x1p+0,          0x0p+0, 0)
T(RZ,                    -inf,                     inf,          0x0p+0, 0)
T(RZ,               0x1p-1074,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,               0x1p-1073,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,               0x1p-1024,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,               0x1p-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ, 0x1.ffffffffffffcp-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ, 0x1.ffffffffffffep-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,               0x1p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ, 0x1.0000000000001p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ, 0x1.0000000000002p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,               0x1p-1021,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,               0x1p-1020,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,                 0x1p-27,                  0x1p+0,         -0x1p-3, INEXACT)
T(RZ,              -0x1p-1074,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,              -0x1p-1073,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,              -0x1p-1024,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,              -0x1p-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,-0x1.ffffffffffffcp-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,-0x1.ffffffffffffep-1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,              -0x1p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,-0x1.0000000000001p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,-0x1.0000000000002p-1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,              -0x1p-1021,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,              -0x1p-1020,                  0x1p+0,          0x0p+0, INEXACT)
T(RZ,                -0x1p-27,                  0x1p+0,         -0x1p-3, INEXACT)
T(RZ,              0x1.634p+9, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ,               0x1p+1022, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ,               0x1p+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ, 0x1.ffffffffffffep+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ, 0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ,             -0x1.634p+9, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ,              -0x1p+1022, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ,              -0x1p+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ,-0x1.ffffffffffffep+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ,-0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023,         -0x1p+0, INEXACT|OVERFLOW)
T(RZ,                     nan,                     nan,          0x0p+0, 0)
T(RZ,                     nan,                     nan,          0x0p+0, 0)
T(RZ,                 0x1p-25,    0x1.0000000000002p+0, -0x1.555554p-53, INEXACT)
T(RZ,                -0x1p-25,    0x1.0000000000002p+0, -0x1.555554p-53, INEXACT)