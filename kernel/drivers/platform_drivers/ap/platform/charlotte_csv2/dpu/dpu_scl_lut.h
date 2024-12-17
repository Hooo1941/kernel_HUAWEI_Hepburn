#ifndef DPU_SCL_LUT_H
#define DPU_SCL_LUT_H 
#define ARSR_LUT_H_OFFSET 64
#define set_tap6_coef(v0,v1,v2) {.reg = {.c0 = v0, .c1 = v1, .c2 = v2}}
#define set_tap4_coef(v0,v1) {.reg = {.c0 = v0, .c1 = v1}}
#define set_scf_coef(v0,v1) {.reg = {.c0 = v0, .c1 = v1}}
enum {
 TAP4 = 4,
 TAP5,
 TAP6,
};
enum {
 VERITICAL = 0,
 HORIZONTAL,
};
union arsr_lut_tap6 {
 struct {
  int c0 : 9;
  int c1 : 9;
  int c2 : 9;
  int : 5;
 } reg;
 unsigned int value;
};
union arsr_lut_tap4 {
 struct {
  int c0 : 9;
  int c1 : 9;
  int : 14;
 } reg;
 unsigned int value;
};
union scf_lut {
 struct {
  int c0 : 16;
  int c1 : 16;
 } reg;
 unsigned int value;
};
static union scf_lut g_scf_coef_tap6[] = {
 set_scf_coef(-187, 105), set_scf_coef(1186, 105), set_scf_coef(-187, 2), set_scf_coef(0, 0),
 set_scf_coef(-182, 86), set_scf_coef(1186, 124), set_scf_coef(-192, 2), set_scf_coef(0, 0),
 set_scf_coef(-176, 67), set_scf_coef(1185, 143), set_scf_coef(-197, 2), set_scf_coef(0, 0),
 set_scf_coef(-170, 49), set_scf_coef(1182, 163), set_scf_coef(-202, 2), set_scf_coef(0, 0),
 set_scf_coef(-166, 32), set_scf_coef(1180, 184), set_scf_coef(-207, 1), set_scf_coef(0, 0),
 set_scf_coef(-160, 15), set_scf_coef(1176, 204), set_scf_coef(-212, 1), set_scf_coef(0, 0),
 set_scf_coef(-155, -2), set_scf_coef(1171, 225), set_scf_coef(-216, 1), set_scf_coef(0, 0),
 set_scf_coef(-149, -18), set_scf_coef(1166, 246), set_scf_coef(-221, 0), set_scf_coef(0, 0),
 set_scf_coef(-145, -34), set_scf_coef(1160, 268), set_scf_coef(-225, 0), set_scf_coef(0, 0),
 set_scf_coef(-139, -49), set_scf_coef(1153, 290), set_scf_coef(-230, -1), set_scf_coef(0, 0),
 set_scf_coef(-134, -63), set_scf_coef(1145, 312), set_scf_coef(-234, -2), set_scf_coef(0, 0),
 set_scf_coef(-129, -78), set_scf_coef(1137, 334), set_scf_coef(-238, -2), set_scf_coef(0, 0),
 set_scf_coef(-124, -91), set_scf_coef(1128, 357), set_scf_coef(-241, -5), set_scf_coef(0, 0),
 set_scf_coef(-119, -104), set_scf_coef(1118, 379), set_scf_coef(-245, -5), set_scf_coef(0, 0),
 set_scf_coef(-114, -117), set_scf_coef(1107, 402), set_scf_coef(-248, -6), set_scf_coef(0, 0),
 set_scf_coef(-109, -129), set_scf_coef(1096, 425), set_scf_coef(-251, -8), set_scf_coef(0, 0),
 set_scf_coef(-104, -141), set_scf_coef(1083, 448), set_scf_coef(-254, -8), set_scf_coef(0, 0),
 set_scf_coef(-100, -152), set_scf_coef(1071, 471), set_scf_coef(-257, -9), set_scf_coef(0, 0),
 set_scf_coef(-95, -162), set_scf_coef(1057, 494), set_scf_coef(-259, -11), set_scf_coef(0, 0),
 set_scf_coef(-90, -172), set_scf_coef(1043, 517), set_scf_coef(-261, -13), set_scf_coef(0, 0),
 set_scf_coef(-86, -181), set_scf_coef(1028, 540), set_scf_coef(-263, -14), set_scf_coef(0, 0),
 set_scf_coef(-82, -190), set_scf_coef(1013, 563), set_scf_coef(-264, -16), set_scf_coef(0, 0),
 set_scf_coef(-77, -199), set_scf_coef(997, 586), set_scf_coef(-265, -18), set_scf_coef(0, 0),
 set_scf_coef(-73, -207), set_scf_coef(980, 609), set_scf_coef(-266, -19), set_scf_coef(0, 0),
 set_scf_coef(-69, -214), set_scf_coef(963, 632), set_scf_coef(-266, -22), set_scf_coef(0, 0),
 set_scf_coef(-65, -221), set_scf_coef(945, 655), set_scf_coef(-266, -24), set_scf_coef(0, 0),
 set_scf_coef(-62, -227), set_scf_coef(927, 678), set_scf_coef(-266, -26), set_scf_coef(0, 0),
 set_scf_coef(-58, -233), set_scf_coef(908, 700), set_scf_coef(-265, -28), set_scf_coef(0, 0),
 set_scf_coef(-54, -238), set_scf_coef(889, 722), set_scf_coef(-264, -31), set_scf_coef(0, 0),
 set_scf_coef(-51, -243), set_scf_coef(870, 744), set_scf_coef(-262, -34), set_scf_coef(0, 0),
 set_scf_coef(-48, -247), set_scf_coef(850, 766), set_scf_coef(-260, -37), set_scf_coef(0, 0),
 set_scf_coef(-45, -251), set_scf_coef(829, 787), set_scf_coef(-257, -39), set_scf_coef(0, 0),
 set_scf_coef(-42, -255), set_scf_coef(809, 809), set_scf_coef(-255, -42), set_scf_coef(0, 0),
 set_scf_coef(-187, 105), set_scf_coef(1186, 105), set_scf_coef(-187, 2), set_scf_coef(0, 0),
 set_scf_coef(-182, 86), set_scf_coef(1186, 124), set_scf_coef(-192, 2), set_scf_coef(0, 0),
 set_scf_coef(-176, 67), set_scf_coef(1185, 143), set_scf_coef(-197, 2), set_scf_coef(0, 0),
 set_scf_coef(-170, 49), set_scf_coef(1182, 163), set_scf_coef(-202, 2), set_scf_coef(0, 0),
 set_scf_coef(-166, 32), set_scf_coef(1180, 184), set_scf_coef(-207, 1), set_scf_coef(0, 0),
 set_scf_coef(-160, 15), set_scf_coef(1176, 204), set_scf_coef(-212, 1), set_scf_coef(0, 0),
 set_scf_coef(-155, -2), set_scf_coef(1171, 225), set_scf_coef(-216, 1), set_scf_coef(0, 0),
 set_scf_coef(-149, -18), set_scf_coef(1166, 246), set_scf_coef(-221, 0), set_scf_coef(0, 0),
 set_scf_coef(-145, -34), set_scf_coef(1160, 268), set_scf_coef(-225, 0), set_scf_coef(0, 0),
 set_scf_coef(-139, -49), set_scf_coef(1153, 290), set_scf_coef(-230, -1), set_scf_coef(0, 0),
 set_scf_coef(-134, -63), set_scf_coef(1145, 312), set_scf_coef(-234, -2), set_scf_coef(0, 0),
 set_scf_coef(-129, -78), set_scf_coef(1137, 334), set_scf_coef(-238, -2), set_scf_coef(0, 0),
 set_scf_coef(-124, -91), set_scf_coef(1128, 357), set_scf_coef(-241, -5), set_scf_coef(0, 0),
 set_scf_coef(-119, -104), set_scf_coef(1118, 379), set_scf_coef(-245, -5), set_scf_coef(0, 0),
 set_scf_coef(-114, -117), set_scf_coef(1107, 402), set_scf_coef(-248, -6), set_scf_coef(0, 0),
 set_scf_coef(-109, -129), set_scf_coef(1096, 425), set_scf_coef(-251, -8), set_scf_coef(0, 0),
 set_scf_coef(-104, -141), set_scf_coef(1083, 448), set_scf_coef(-254, -8), set_scf_coef(0, 0),
 set_scf_coef(-100, -152), set_scf_coef(1071, 471), set_scf_coef(-257, -9), set_scf_coef(0, 0),
 set_scf_coef(-95, -162), set_scf_coef(1057, 494), set_scf_coef(-259, -11), set_scf_coef(0, 0),
 set_scf_coef(-90, -172), set_scf_coef(1043, 517), set_scf_coef(-261, -13), set_scf_coef(0, 0),
 set_scf_coef(-86, -181), set_scf_coef(1028, 540), set_scf_coef(-263, -14), set_scf_coef(0, 0),
 set_scf_coef(-82, -190), set_scf_coef(1013, 563), set_scf_coef(-264, -16), set_scf_coef(0, 0),
 set_scf_coef(-77, -199), set_scf_coef(997, 586), set_scf_coef(-265, -18), set_scf_coef(0, 0),
 set_scf_coef(-73, -207), set_scf_coef(980, 609), set_scf_coef(-266, -19), set_scf_coef(0, 0),
 set_scf_coef(-69, -214), set_scf_coef(963, 632), set_scf_coef(-266, -22), set_scf_coef(0, 0),
 set_scf_coef(-65, -221), set_scf_coef(945, 655), set_scf_coef(-266, -24), set_scf_coef(0, 0),
 set_scf_coef(-62, -227), set_scf_coef(927, 678), set_scf_coef(-266, -26), set_scf_coef(0, 0),
 set_scf_coef(-58, -233), set_scf_coef(908, 700), set_scf_coef(-265, -28), set_scf_coef(0, 0),
 set_scf_coef(-54, -238), set_scf_coef(889, 722), set_scf_coef(-264, -31), set_scf_coef(0, 0),
 set_scf_coef(-51, -243), set_scf_coef(870, 744), set_scf_coef(-262, -34), set_scf_coef(0, 0),
 set_scf_coef(-48, -247), set_scf_coef(850, 766), set_scf_coef(-260, -37), set_scf_coef(0, 0),
 set_scf_coef(-45, -251), set_scf_coef(829, 787), set_scf_coef(-257, -39), set_scf_coef(0, 0),
 set_scf_coef(-42, -255), set_scf_coef(809, 809), set_scf_coef(-255, -42), set_scf_coef(0, 0)
};
static union scf_lut g_scf_coef_tap5[] = {
 set_scf_coef(-94, 608), set_scf_coef(608, -94), set_scf_coef(-4, 0), set_scf_coef(0, 0),
 set_scf_coef(-94, 594), set_scf_coef(619, -91), set_scf_coef(-4, 0), set_scf_coef(0, 0),
 set_scf_coef(-96, 579), set_scf_coef(635, -89), set_scf_coef(-5, 0), set_scf_coef(0, 0),
 set_scf_coef(-96, 563), set_scf_coef(650, -87), set_scf_coef(-6, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 548), set_scf_coef(665, -85), set_scf_coef(-7, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 532), set_scf_coef(678, -82), set_scf_coef(-7, 0), set_scf_coef(0, 0),
 set_scf_coef(-98, 516), set_scf_coef(693, -79), set_scf_coef(-8, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 500), set_scf_coef(705, -75), set_scf_coef(-9, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 484), set_scf_coef(720, -72), set_scf_coef(-11, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 468), set_scf_coef(733, -68), set_scf_coef(-12, 0), set_scf_coef(0, 0),
 set_scf_coef(-96, 452), set_scf_coef(744, -63), set_scf_coef(-13, 0), set_scf_coef(0, 0),
 set_scf_coef(-95, 436), set_scf_coef(755, -58), set_scf_coef(-14, 0), set_scf_coef(0, 0),
 set_scf_coef(-94, 419), set_scf_coef(768, -53), set_scf_coef(-16, 0), set_scf_coef(0, 0),
 set_scf_coef(-93, 403), set_scf_coef(779, -48), set_scf_coef(-17, 0), set_scf_coef(0, 0),
 set_scf_coef(-92, 387), set_scf_coef(789, -42), set_scf_coef(-18, 0), set_scf_coef(0, 0),
 set_scf_coef(-90, 371), set_scf_coef(799, -36), set_scf_coef(-20, 0), set_scf_coef(0, 0),
 set_scf_coef(-89, 355), set_scf_coef(809, -29), set_scf_coef(-22, 0), set_scf_coef(0, 0),
 set_scf_coef(-87, 339), set_scf_coef(817, -22), set_scf_coef(-23, 0), set_scf_coef(0, 0),
 set_scf_coef(-86, 324), set_scf_coef(826, -15), set_scf_coef(-25, 0), set_scf_coef(0, 0),
 set_scf_coef(-84, 308), set_scf_coef(835, -8), set_scf_coef(-27, 0), set_scf_coef(0, 0),
 set_scf_coef(-82, 293), set_scf_coef(842, 0), set_scf_coef(-29, 0), set_scf_coef(0, 0),
 set_scf_coef(-80, 277), set_scf_coef(849, 9), set_scf_coef(-31, 0), set_scf_coef(0, 0),
 set_scf_coef(-78, 262), set_scf_coef(855, 18), set_scf_coef(-33, 0), set_scf_coef(0, 0),
 set_scf_coef(-75, 247), set_scf_coef(860, 27), set_scf_coef(-35, 0), set_scf_coef(0, 0),
 set_scf_coef(-73, 233), set_scf_coef(865, 36), set_scf_coef(-37, 0), set_scf_coef(0, 0),
 set_scf_coef(-71, 218), set_scf_coef(870, 46), set_scf_coef(-39, 0), set_scf_coef(0, 0),
 set_scf_coef(-69, 204), set_scf_coef(874, 56), set_scf_coef(-41, 0), set_scf_coef(0, 0),
 set_scf_coef(-66, 190), set_scf_coef(876, 67), set_scf_coef(-43, 0), set_scf_coef(0, 0),
 set_scf_coef(-64, 176), set_scf_coef(879, 78), set_scf_coef(-45, 0), set_scf_coef(0, 0),
 set_scf_coef(-62, 163), set_scf_coef(882, 89), set_scf_coef(-48, 0), set_scf_coef(0, 0),
 set_scf_coef(-59, 150), set_scf_coef(883, 100), set_scf_coef(-50, 0), set_scf_coef(0, 0),
 set_scf_coef(-57, 137), set_scf_coef(883, 112), set_scf_coef(-51, 0), set_scf_coef(0, 0),
 set_scf_coef(-55, 125), set_scf_coef(884, 125), set_scf_coef(-55, 0), set_scf_coef(0, 0),
 set_scf_coef(-94, 608), set_scf_coef(608, -94), set_scf_coef(-4, 0), set_scf_coef(0, 0),
 set_scf_coef(-94, 594), set_scf_coef(619, -91), set_scf_coef(-4, 0), set_scf_coef(0, 0),
 set_scf_coef(-96, 579), set_scf_coef(635, -89), set_scf_coef(-5, 0), set_scf_coef(0, 0),
 set_scf_coef(-96, 563), set_scf_coef(650, -87), set_scf_coef(-6, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 548), set_scf_coef(665, -85), set_scf_coef(-7, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 532), set_scf_coef(678, -82), set_scf_coef(-7, 0), set_scf_coef(0, 0),
 set_scf_coef(-98, 516), set_scf_coef(693, -79), set_scf_coef(-8, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 500), set_scf_coef(705, -75), set_scf_coef(-9, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 484), set_scf_coef(720, -72), set_scf_coef(-11, 0), set_scf_coef(0, 0),
 set_scf_coef(-97, 468), set_scf_coef(733, -68), set_scf_coef(-12, 0), set_scf_coef(0, 0),
 set_scf_coef(-96, 452), set_scf_coef(744, -63), set_scf_coef(-13, 0), set_scf_coef(0, 0),
 set_scf_coef(-95, 436), set_scf_coef(755, -58), set_scf_coef(-14, 0), set_scf_coef(0, 0),
 set_scf_coef(-94, 419), set_scf_coef(768, -53), set_scf_coef(-16, 0), set_scf_coef(0, 0),
 set_scf_coef(-93, 403), set_scf_coef(779, -48), set_scf_coef(-17, 0), set_scf_coef(0, 0),
 set_scf_coef(-92, 387), set_scf_coef(789, -42), set_scf_coef(-18, 0), set_scf_coef(0, 0),
 set_scf_coef(-90, 371), set_scf_coef(799, -36), set_scf_coef(-20, 0), set_scf_coef(0, 0),
 set_scf_coef(-89, 355), set_scf_coef(809, -29), set_scf_coef(-22, 0), set_scf_coef(0, 0),
 set_scf_coef(-87, 339), set_scf_coef(817, -22), set_scf_coef(-23, 0), set_scf_coef(0, 0),
 set_scf_coef(-86, 324), set_scf_coef(826, -15), set_scf_coef(-25, 0), set_scf_coef(0, 0),
 set_scf_coef(-84, 308), set_scf_coef(835, -8), set_scf_coef(-27, 0), set_scf_coef(0, 0),
 set_scf_coef(-82, 293), set_scf_coef(842, 0), set_scf_coef(-29, 0), set_scf_coef(0, 0),
 set_scf_coef(-80, 277), set_scf_coef(849, 9), set_scf_coef(-31, 0), set_scf_coef(0, 0),
 set_scf_coef(-78, 262), set_scf_coef(855, 18), set_scf_coef(-33, 0), set_scf_coef(0, 0),
 set_scf_coef(-75, 247), set_scf_coef(860, 27), set_scf_coef(-35, 0), set_scf_coef(0, 0),
 set_scf_coef(-73, 233), set_scf_coef(865, 36), set_scf_coef(-37, 0), set_scf_coef(0, 0),
 set_scf_coef(-71, 218), set_scf_coef(870, 46), set_scf_coef(-39, 0), set_scf_coef(0, 0),
 set_scf_coef(-69, 204), set_scf_coef(874, 56), set_scf_coef(-41, 0), set_scf_coef(0, 0),
 set_scf_coef(-66, 190), set_scf_coef(876, 67), set_scf_coef(-43, 0), set_scf_coef(0, 0),
 set_scf_coef(-64, 176), set_scf_coef(879, 78), set_scf_coef(-45, 0), set_scf_coef(0, 0),
 set_scf_coef(-62, 163), set_scf_coef(882, 89), set_scf_coef(-48, 0), set_scf_coef(0, 0),
 set_scf_coef(-59, 150), set_scf_coef(883, 100), set_scf_coef(-50, 0), set_scf_coef(0, 0),
 set_scf_coef(-57, 137), set_scf_coef(883, 112), set_scf_coef(-51, 0), set_scf_coef(0, 0),
 set_scf_coef(-55, 125), set_scf_coef(884, 125), set_scf_coef(-55, 0), set_scf_coef(0, 0),
};
static union scf_lut g_scf_coef_tap4[] = {
 set_scf_coef(214, 599), set_scf_coef(214, -3),
 set_scf_coef(207, 597), set_scf_coef(223, -3),
 set_scf_coef(200, 596), set_scf_coef(231, -3),
 set_scf_coef(193, 596), set_scf_coef(238, -3),
 set_scf_coef(186, 595), set_scf_coef(246, -3),
 set_scf_coef(178, 594), set_scf_coef(255, -3),
 set_scf_coef(171, 593), set_scf_coef(263, -3),
 set_scf_coef(165, 591), set_scf_coef(271, -3),
 set_scf_coef(158, 589), set_scf_coef(279, -2),
 set_scf_coef(151, 587), set_scf_coef(288, -2),
 set_scf_coef(145, 584), set_scf_coef(296, -1),
 set_scf_coef(139, 582), set_scf_coef(304, -1),
 set_scf_coef(133, 578), set_scf_coef(312, 1),
 set_scf_coef(127, 575), set_scf_coef(321, 1),
 set_scf_coef(121, 572), set_scf_coef(329, 2),
 set_scf_coef(115, 568), set_scf_coef(337, 4),
 set_scf_coef(109, 564), set_scf_coef(346, 5),
 set_scf_coef(104, 560), set_scf_coef(354, 6),
 set_scf_coef(98, 555), set_scf_coef(362, 9),
 set_scf_coef(94, 550), set_scf_coef(370, 10),
 set_scf_coef(88, 546), set_scf_coef(379, 11),
 set_scf_coef(84, 540), set_scf_coef(387, 13),
 set_scf_coef(79, 535), set_scf_coef(395, 15),
 set_scf_coef(74, 530), set_scf_coef(403, 17),
 set_scf_coef(70, 524), set_scf_coef(411, 19),
 set_scf_coef(66, 518), set_scf_coef(419, 21),
 set_scf_coef(62, 512), set_scf_coef(427, 23),
 set_scf_coef(57, 506), set_scf_coef(435, 26),
 set_scf_coef(54, 499), set_scf_coef(443, 28),
 set_scf_coef(50, 492), set_scf_coef(451, 31),
 set_scf_coef(47, 486), set_scf_coef(457, 34),
 set_scf_coef(43, 479), set_scf_coef(465, 37),
 set_scf_coef(40, 472), set_scf_coef(472, 40),
 set_scf_coef(214, 599), set_scf_coef(214, -3),
 set_scf_coef(207, 597), set_scf_coef(223, -3),
 set_scf_coef(200, 596), set_scf_coef(231, -3),
 set_scf_coef(193, 596), set_scf_coef(238, -3),
 set_scf_coef(186, 595), set_scf_coef(246, -3),
 set_scf_coef(178, 594), set_scf_coef(255, -3),
 set_scf_coef(171, 593), set_scf_coef(263, -3),
 set_scf_coef(165, 591), set_scf_coef(271, -3),
 set_scf_coef(158, 589), set_scf_coef(279, -2),
 set_scf_coef(151, 587), set_scf_coef(288, -2),
 set_scf_coef(145, 584), set_scf_coef(296, -1),
 set_scf_coef(139, 582), set_scf_coef(304, -1),
 set_scf_coef(133, 578), set_scf_coef(312, 1),
 set_scf_coef(127, 575), set_scf_coef(321, 1),
 set_scf_coef(121, 572), set_scf_coef(329, 2),
 set_scf_coef(115, 568), set_scf_coef(337, 4),
 set_scf_coef(109, 564), set_scf_coef(346, 5),
 set_scf_coef(104, 560), set_scf_coef(354, 6),
 set_scf_coef(98, 555), set_scf_coef(362, 9),
 set_scf_coef(94, 550), set_scf_coef(370, 10),
 set_scf_coef(88, 546), set_scf_coef(379, 11),
 set_scf_coef(84, 540), set_scf_coef(387, 13),
 set_scf_coef(79, 535), set_scf_coef(395, 15),
 set_scf_coef(74, 530), set_scf_coef(403, 17),
 set_scf_coef(70, 524), set_scf_coef(411, 19),
 set_scf_coef(66, 518), set_scf_coef(419, 21),
 set_scf_coef(62, 512), set_scf_coef(427, 23),
 set_scf_coef(57, 506), set_scf_coef(435, 26),
 set_scf_coef(54, 499), set_scf_coef(443, 28),
 set_scf_coef(50, 492), set_scf_coef(451, 31),
 set_scf_coef(47, 486), set_scf_coef(457, 34),
 set_scf_coef(43, 479), set_scf_coef(465, 37),
 set_scf_coef(40, 472), set_scf_coef(472, 40)
};
static union arsr_lut_tap4 g_arsr_coef_tap4[] = {
 set_tap4_coef(31, 194), set_tap4_coef(31, 0),
 set_tap4_coef(23, 206), set_tap4_coef(44, -17),
 set_tap4_coef(14, 203), set_tap4_coef(57, -18),
 set_tap4_coef(6, 198), set_tap4_coef(70, -18),
 set_tap4_coef(0, 190), set_tap4_coef(85, -19),
 set_tap4_coef(-5, 180), set_tap4_coef(99, -18),
 set_tap4_coef(-10, 170), set_tap4_coef(114, -18),
 set_tap4_coef(-13, 157), set_tap4_coef(129, -17),
 set_tap4_coef(-15, 143), set_tap4_coef(143, -15),
 set_tap4_coef(-3, 254), set_tap4_coef(6, -1),
 set_tap4_coef(-9, 255), set_tap4_coef(13, -3),
 set_tap4_coef(-18, 254), set_tap4_coef(27, -7),
 set_tap4_coef(-23, 245), set_tap4_coef(44, -10),
 set_tap4_coef(-27, 233), set_tap4_coef(64, -14),
 set_tap4_coef(-29, 218), set_tap4_coef(85, -18),
 set_tap4_coef(-29, 198), set_tap4_coef(108, -21),
 set_tap4_coef(-29, 177), set_tap4_coef(132, -24),
 set_tap4_coef(-27, 155), set_tap4_coef(155, -27),
};
static union arsr_lut_tap6 g_arsr_coef_tap6[] = {
 set_tap6_coef(-22, 43, 214), set_tap6_coef(43, -22, 0),
 set_tap6_coef(-18, 29, 205), set_tap6_coef(53, -23, 10),
 set_tap6_coef(-16, 18, 203), set_tap6_coef(67, -25, 9),
 set_tap6_coef(-13, 9, 198), set_tap6_coef(80, -26, 8),
 set_tap6_coef(-10, 0, 191), set_tap6_coef(95, -27, 7),
 set_tap6_coef(-7, -7, 182), set_tap6_coef(109, -27, 6),
 set_tap6_coef(-5, -14, 174), set_tap6_coef(124, -27, 4),
 set_tap6_coef(-2, -18, 162), set_tap6_coef(137, -25, 2),
 set_tap6_coef(0, -22, 150), set_tap6_coef(150, -22, 0),
 set_tap6_coef(0, -3, 254), set_tap6_coef(6, -1, 0),
 set_tap6_coef(4, -12, 252), set_tap6_coef(15, -5, 2),
 set_tap6_coef(7, -22, 245), set_tap6_coef(31, -9, 4),
 set_tap6_coef(10, -29, 234), set_tap6_coef(49, -14, 6),
 set_tap6_coef(12, -34, 221), set_tap6_coef(68, -19, 8),
 set_tap6_coef(13, -37, 206), set_tap6_coef(88, -24, 10),
 set_tap6_coef(14, -38, 189), set_tap6_coef(108, -29, 12),
 set_tap6_coef(14, -38, 170), set_tap6_coef(130, -33, 13),
 set_tap6_coef(14, -36, 150), set_tap6_coef(150, -36, 14),
};
struct scf_lut_tap_table {
 unsigned char tap;
 unsigned int tap_size;
 unsigned int offset;
 unsigned int *scf_lut_tap;
};
struct arsr_lut_tap_table {
 unsigned char direction;
 unsigned int tap_size;
 unsigned int offset;
 unsigned int *arsr_lut_tap;
};
static struct scf_lut_tap_table g_scf_lut_tap_tlb[] = {
 { TAP6, sizeof(g_scf_coef_tap6), DPU_VSCF_V0_SCF_VIDEO_6TAP_COEF_ADDR(0, 0), (unsigned int *)g_scf_coef_tap6 },
 { TAP5, sizeof(g_scf_coef_tap5), DPU_VSCF_V0_SCF_VIDEO_5TAP_COEF_ADDR(0, 0), (unsigned int *)g_scf_coef_tap5 },
 { TAP4, sizeof(g_scf_coef_tap4), DPU_VSCF_V0_SCF_VIDEO_4TAP_COEF_ADDR(0, 0), (unsigned int *)g_scf_coef_tap4 }
};
static struct arsr_lut_tap_table g_arsr_lut_tap_tlb[] = {
 { VERITICAL, sizeof(g_arsr_coef_tap6), DPU_ARSR_COEFY_V_ADDR(0, 0), (unsigned int *)g_arsr_coef_tap6 },
 { HORIZONTAL, sizeof(g_arsr_coef_tap6) + ARSR_LUT_H_OFFSET * sizeof(unsigned int),
  DPU_ARSR_COEFY_H_ADDR(0, 0), (unsigned int *)g_arsr_coef_tap6 },
 { VERITICAL, sizeof(g_arsr_coef_tap4), DPU_ARSR_COEFA_V_ADDR(0, 0), (unsigned int *)g_arsr_coef_tap4 },
 { HORIZONTAL, sizeof(g_arsr_coef_tap4) + ARSR_LUT_H_OFFSET * sizeof(unsigned int),
  DPU_ARSR_COEFA_H_ADDR(0, 0), (unsigned int *)g_arsr_coef_tap4 },
 { VERITICAL, sizeof(g_arsr_coef_tap4), DPU_ARSR_COEFUV_V_ADDR(0, 0), (unsigned int *)g_arsr_coef_tap4 },
 { HORIZONTAL, sizeof(g_arsr_coef_tap4) + ARSR_LUT_H_OFFSET * sizeof(unsigned int),
  DPU_ARSR_COEFUV_H_ADDR(0, 0), (unsigned int *)g_arsr_coef_tap4 }
};
#endif
