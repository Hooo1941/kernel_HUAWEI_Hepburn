/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: linlixin2@huawei.com
 * Create: 2020-10-28
 */

#include "track.h"

#include <huawei_platform/log/hw_log.h>
#include <securec.h>

#include "rsmc_rx_ctrl.h"
#include "rsmc_spi_ctrl.h"
#include "rsmc_x800_device.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_TRACK
HWLOG_REGIST();

static s8 magic_code[FRM_PERIOD] = {0};

#define TAB_LEN 1024
#define PI (TAB_LEN * 2)
#define QUAN_COEF 65536
#define MAX_ATAN 32767
#define MIN_ATAN (-32768)
static const int tan_tab[TAB_LEN] = {
	0, 101, 201, 302, 402, 503, 603, 704, 804, 905, 1005, 1106, 1207, 1307,
	1408, 1508, 1609, 1709, 1810, 1911, 2011, 2112, 2213, 2313, 2414, 2515,
	2615, 2716, 2817, 2917, 3018, 3119, 3220, 3320, 3421, 3522, 3623, 3724,
	3825, 3925, 4026, 4127, 4228, 4329, 4430, 4531, 4632, 4733, 4834, 4935,
	5036, 5138, 5239, 5340, 5441, 5542, 5644, 5745, 5846, 5948, 6049, 6150,
	6252, 6353, 6455, 6556, 6658, 6759, 6861, 6963, 7064, 7166, 7268, 7370,
	7471, 7573, 7675, 7777, 7879, 7981, 8083, 8185, 8287, 8389, 8492, 8594,
	8696, 8798, 8901, 9003, 9106, 9208, 9311, 9413, 9516, 9619, 9721, 9824,
	9927, 10030, 10133, 10236, 10339, 10442, 10545, 10648, 10751, 10854,
	10958, 11061, 11165, 11268, 11372, 11475, 11579, 11682, 11786, 11890,
	11994, 12098, 12202, 12306, 12410, 12514, 12618, 12723, 12827, 12931,
	13036, 13140, 13245, 13350, 13454, 13559, 13664, 13769, 13874, 13979,
	14084, 14189, 14295, 14400, 14506, 14611, 14717, 14822, 14928, 15034,
	15140, 15246, 15352, 15458, 15564, 15670, 15776, 15883, 15989, 16096,
	16202, 16309, 16416, 16523, 16630, 16737, 16844, 16951, 17058, 17166,
	17273, 17381, 17489, 17596, 17704, 17812, 17920, 18028, 18136, 18245,
	18353, 18461, 18570, 18679, 18787, 18896, 19005, 19114, 19223, 19332,
	19442, 19551, 19661, 19770, 19880, 19990, 20100, 20210, 20320, 20430,
	20541, 20651, 20762, 20872, 20983, 21094, 21205, 21316, 21427, 21539,
	21650, 21762, 21873, 21985, 22097, 22209, 22321, 22433, 22546, 22658,
	22771, 22884, 22997, 23110, 23223, 23336, 23449, 23563, 23676, 23790,
	23904, 24018, 24132, 24246, 24360, 24475, 24590, 24704, 24819, 24934,
	25049, 25165, 25280, 25396, 25511, 25627, 25743, 25859, 25975, 26092,
	26208, 26325, 26442, 26559, 26676, 26793, 26911, 27028, 27146, 27264,
	27382, 27500, 27618, 27737, 27855, 27974, 28093, 28212, 28331, 28451,
	28570, 28690, 28810, 28930, 29050, 29170, 29291, 29412, 29533, 29654,
	29775, 29896, 30018, 30139, 30261, 30383, 30506, 30628, 30751, 30873,
	30996, 31119, 31243, 31366, 31490, 31614, 31738, 31862, 31986, 32111,
	32236, 32360, 32486, 32611, 32736, 32862, 32988, 33114, 33240, 33367,
	33494, 33621, 33748, 33875, 34002, 34130, 34258, 34386, 34514, 34643,
	34772, 34901, 35030, 35159, 35289, 35418, 35548, 35679, 35809, 35940,
	36071, 36202, 36333, 36465, 36596, 36728, 36861, 36993, 37126, 37259,
	37392, 37525, 37659, 37793, 37927, 38061, 38196, 38330, 38465, 38601,
	38736, 38872, 39008, 39144, 39281, 39418, 39555, 39692, 39829, 39967,
	40105, 40244, 40382, 40521, 40660, 40799, 40939, 41079, 41219, 41360,
	41500, 41641, 41782, 41924, 42066, 42208, 42350, 42493, 42636, 42779,
	42923, 43066, 43210, 43355, 43500, 43644, 43790, 43935, 44081, 44227,
	44374, 44521, 44668, 44815, 44963, 45111, 45259, 45408, 45557, 45706,
	45856, 46005, 46156, 46306, 46457, 46608, 46760, 46912, 47064, 47216,
	47369, 47523, 47676, 47830, 47984, 48139, 48294, 48449, 48605, 48761,
	48917, 49074, 49231, 49388, 49546, 49704, 49863, 50022, 50181, 50341,
	50501, 50661, 50822, 50983, 51145, 51307, 51469, 51632, 51795, 51958,
	52122, 52287, 52451, 52617, 52782, 52948, 53114, 53281, 53448, 53616,
	53784, 53952, 54121, 54291, 54460, 54631, 54801, 54972, 55144, 55316,
	55488, 55661, 55834, 56008, 56182, 56357, 56532, 56707, 56883, 57060,
	57237, 57414, 57592, 57771, 57950, 58129, 58309, 58489, 58670, 58851,
	59033, 59216, 59398, 59582, 59766, 59950, 60135, 60320, 60506, 60693,
	60880, 61067, 61255, 61444, 61633, 61823, 62013, 62204, 62395, 62587,
	62780, 62973, 63167, 63361, 63556, 63751, 63947, 64143, 64341, 64538,
	64737, 64936, 65135, 65335, 65536, 65737, 65939, 66142, 66345, 66549,
	66754, 66959, 67165, 67371, 67578, 67786, 67994, 68203, 68413, 68624,
	68835, 69046, 69259, 69472, 69686, 69900, 70116, 70332, 70548, 70766,
	70984, 71203, 71422, 71642, 71864, 72085, 72308, 72531, 72755, 72980,
	73206, 73432, 73659, 73887, 74116, 74345, 74576, 74807, 75039, 75271,
	75505, 75739, 75974, 76210, 76447, 76685, 76924, 77163, 77404, 77645,
	77887, 78130, 78374, 78618, 78864, 79111, 79358, 79607, 79856, 80106,
	80357, 80609, 80863, 81117, 81372, 81628, 81885, 82143, 82402, 82662,
	82923, 83184, 83448, 83712, 83977, 84243, 84510, 84778, 85047, 85318,
	85589, 85862, 86135, 86410, 86686, 86963, 87241, 87520, 87801, 88082,
	88365, 88649, 88934, 89220, 89508, 89796, 90086, 90377, 90670, 90963,
	91258, 91554, 91852, 92150, 92450, 92751, 93054, 93358, 93663, 93970,
	94277, 94587, 94897, 95209, 95523, 95838, 96154, 96471, 96791, 97111,
	97433, 97757, 98082, 98408, 98736, 99065, 99396, 99729, 100063, 100399,
	100736, 101075, 101415, 101757, 102101, 102447, 102794, 103142, 103493,
	103845, 104199, 104554, 104911, 105270, 105631, 105994, 106358, 106724,
	107092, 107462, 107834, 108208, 108583, 108961, 109340, 109722, 110105,
	110490, 110877, 111267, 111658, 112051, 112447, 112844, 113244, 113646,
	114050, 114456, 114864, 115275, 115687, 116102, 116519, 116939, 117361,
	117785, 118211, 118640, 119071, 119505, 119941, 120379, 120820, 121264,
	121710, 122158, 122609, 123063, 123519, 123978, 124440, 124904, 125371,
	125841, 126314, 126789, 127267, 127748, 128232, 128719, 129209, 129702,
	130198, 130696, 131198, 131703, 132211, 132723, 133237, 133755, 134276,
	134800, 135327, 135858, 136393, 136930, 137471, 138016, 138564, 139116,
	139671, 140230, 140793, 141359, 141929, 142503, 143081, 143663, 144248,
	144838, 145432, 146029, 146631, 147237, 147847, 148461, 149080, 149703,
	150330, 150962, 151598, 152239, 152884, 153534, 154189, 154848, 155512,
	156181, 156855, 157534, 158218, 158907, 159601, 160300, 161005, 161715,
	162430, 163151, 163878, 164610, 165347, 166091, 166840, 167595, 168356,
	169123, 169896, 170675, 171460, 172252, 173051, 173855, 174667, 175485,
	176309, 177141, 177979, 178825, 179677, 180537, 181404, 182279, 183161,
	184050, 184948, 185853, 186766, 187687, 188616, 189553, 190499, 191453,
	192416, 193388, 194368, 195357, 196356, 197363, 198380, 199407, 200443,
	201489, 202544, 203610, 204686, 205773, 206870, 207977, 209096, 210225,
	211366, 212518, 213681, 214856, 216043, 217242, 218454, 219677, 220914,
	222163, 223426, 224701, 225990, 227293, 228610, 229941, 231286, 232646,
	234021, 235411, 236817, 238238, 239675, 241128, 242598, 244084, 245588,
	247109, 248648, 250204, 251780, 253373, 254986, 256618, 258270, 259942,
	261634, 263348, 265082, 266838, 268617, 270417, 272241, 274088, 275959,
	277854, 279774, 281720, 283691, 285689, 287713, 289765, 291845, 293954,
	296091, 298259, 300457, 302686, 304947, 307241, 309568, 311929, 314324,
	316755, 319222, 321727, 324269, 326851, 329472, 332134, 334837, 337584,
	340374, 343208, 346089, 349017, 351993, 355019, 358095, 361223, 364405,
	367641, 370934, 374284, 377693, 381164, 384696, 388293, 391956, 395687,
	399487, 403359, 407305, 411327, 415428, 419608, 423872, 428221, 432658,
	437186, 441808, 446526, 451344, 456265, 461292, 466428, 471678, 477046,
	482534, 488148, 493892, 499770, 505787, 511949, 518259, 524725, 531352,
	538145, 545112, 552259, 559593, 567122, 574854, 582796, 590958, 599349,
	607979, 616858, 625997, 635407, 645102, 655095, 665398, 676028, 686999,
	698329, 710035, 722138, 734656, 747612, 761030, 774935, 789353, 804314,
	819850, 835993, 852780, 870252, 888450, 907421, 927215, 947888, 969499,
	992113, 1015802, 1040646, 1066730, 1094150, 1123011, 1153431, 1185539,
	1219479, 1255414, 1293525, 1334016, 1377117, 1423089, 1472229, 1524877,
	1581422, 1642314, 1708075, 1779314, 1856744, 1941210, 2033717, 2135471,
	2247933, 2372887, 2512538, 2669641, 2847686, 3051162, 3285936, 3559834,
	3883525, 4271948, 4746679, 5340086, 6103027, 7120271, 8544398,
	10680573, 14240843, 21361348, 42722796
};
#define MAX_CHAN_NUM 8
#define CLK_HZ 38400000
#define IF_HZ (-4250438)
#define REF_TIME 16250

#define SIG_ALPHA_COEFF 16
#define NOS_ALPHA_COEFF 64

#define PLL_DATA_LEN 5
#define DOPPLER_WIN_1S 200
#define DOPPLER_WIN_100MS 20
#define CODE_NCO_WIN 100
#define CARR_NCO_WIN 100

// enlarge 4096
// Bn = 10Hz, Ts = 5ms, 3    omega=Bn/0.8333 = 12
#define LOCKED_PLL_G0   117965 // (28.8)
#define LOCKED_PLL_G1   3244 // (0.792)
#define LOCKED_PLL_G2   176 // (0.0432)

// enlarge 2^25
#define LOCKED_DLL_G0   51326101 // (1.529637)
#define LOCKED_DLL_G1   74966 // (2.234173e-3)
#define LOCKED_DLL_G2   217 // (6.472487e-6)

#define WIN_LEN 2
#define WIDTH 10

struct chn_info {
	struct complex e;
	struct complex p;
	struct complex l;
	int q;
	u32 win_idx;
	struct complex eh[WIN_LEN];
	struct complex ph[WIN_LEN];
	struct complex lh[WIN_LEN];
	int idx;
	s64 doppler;
	s64 dither;
	s64 code_doppler;
	s64 code_dither;
	s64 pll_gain[3];
	s64 dll_gain[3];
	u32 carr_nco_word;
	u32 code_nco_word;
	s64 i_power;
	s64 q_power;
	s64 i_sum;
	s64 cn0;
};

struct track_ctx {
	struct chn_info info[MAX_CHAN_NUM];
	s64 doppler_1s[DOPPLER_WIN_1S];
	u32 index_1s;
	u32 count_1s;
	s64 doppler_100ms[DOPPLER_WIN_100MS];
	u32 index_100ms;
	u32 count_100ms;
	u32 code_nco[CODE_NCO_WIN];
	u32 code_nco_idx;
	u32 code_nco_cnt;
	u32 carr_nco[CARR_NCO_WIN];
	u32 carr_nco_idx;
	u32 carr_nco_cnt;
	u8 chn_idx;
};
static struct track_ctx g_track_ctx;

static int atan(int k)
{
	int index;
	int kabs = (k >= 0) ? k : (-k);
	int left = 0;
	int right = TAB_LEN;
	int cur;

	for (index = TAB_LEN; index > 0; index = index / TIMES_2) {
		cur = (left + right) / TIMES_2;
		if (tan_tab[cur] <= kabs)
			left = cur;
		else
			right = cur;
	}
	if (kabs > tan_tab[TAB_LEN - 1])
		return k > 0 ? TAB_LEN : -TAB_LEN;
	else
		return k > 0 ? left : -left;
}

static int atan2(int y, int x)
{
	int k;

	if (y >= MAX_ATAN)
		y = MAX_ATAN;
	if (y <= MIN_ATAN)
		y = MAX_ATAN;
	if (x == 0) {
		if (y > 0)
			return PI / TIMES_2;
		else
			return -PI / TIMES_2;
	}
	k = (y * QUAN_COEF) / x;
	if (x > 0)
		return atan(k);
	else if (y > 0)
		return atan(k) + PI;
	else
		return atan(k) - PI;
	return 0;
}

static s16 u2s(u16 data, u32 width)
{
	s16 out;
	u16 flag = 0xffff;

	flag = (u16)(flag << (width - 1));
	if ((flag & data) != 0)
		out = (s16)(flag | data);
	else
		out = (s16)(data);
	return out;
}

static void set_peak_val(struct track_msg *msg, u32 index, u32 w_idx, struct chn_info *info)
{
	u32 ms_idx;
	int sym;
	s16 tmp1;
	int tmp = 0;
	struct corr_value value;

	if (msg == NULL || info == NULL)
		return;
	ms_idx = (msg->ms_idx + index) % FRM_PERIOD;
	sym = magic_code[ms_idx];
	// er
	value.er = u2s(msg->e_tap[index].real, WIDTH);
	info->eh[w_idx].real += value.er * sym;
	// ei
	value.ei = u2s(msg->e_tap[index].imag, WIDTH);
	info->eh[w_idx].imag += value.ei * sym;
	// pr
	value.pr = u2s(msg->p_tap[index].real, WIDTH);
	info->ph[w_idx].real += value.pr * sym;
	// pi
	value.pi = u2s(msg->p_tap[index].imag, WIDTH);
	info->ph[w_idx].imag += value.pi * sym;
	// lr
	value.lr = u2s(msg->l_tap[index].real, WIDTH);
	info->lh[w_idx].real += value.lr * sym;
	// li
	value.li = u2s(msg->l_tap[index].imag, WIDTH);
	info->lh[w_idx].imag += value.li * sym;

	tmp1 = u2s(msg->e_tap[index].imag, WIDTH);
	tmp += tmp1 * tmp1;
	tmp1 = u2s(msg->p_tap[index].imag, WIDTH);
	tmp += tmp1 * tmp1;
	tmp1 = u2s(msg->l_tap[index].imag, WIDTH);
	tmp += tmp1 * tmp1;
	info->q += tmp;
}

static void proc_pilot_data(struct track_msg *msg)
{
	u32 index, w_idx;
	struct chn_info *info = NULL;
	int ret;

	if (msg == NULL)
		return;
	info = &g_track_ctx.info[msg->chn_idx];
	w_idx = info->win_idx;
	w_idx = (w_idx + 1) % WIN_LEN;
	ret = memset_s(&info->eh[w_idx], sizeof(struct complex), 0x00, sizeof(struct complex));
	ret = memset_s(&info->ph[w_idx], sizeof(struct complex), 0x00, sizeof(struct complex));
	ret = memset_s(&info->lh[w_idx], sizeof(struct complex), 0x00, sizeof(struct complex));
	if (ret != EOK) {
		hwlog_err("%s: memset_s failed", __func__);
		return;
	}

	info->q = 0;
	for (index = 0; index < PLL_DATA_LEN; index++)
		set_peak_val(msg, index, w_idx, info);

	info->win_idx = w_idx;
	ret = memset_s(&info->e, sizeof(struct complex), 0x00, sizeof(struct complex));
	ret = memset_s(&info->p, sizeof(struct complex), 0x00, sizeof(struct complex));
	ret = memset_s(&info->l, sizeof(struct complex), 0x00, sizeof(struct complex));
	if (ret != EOK) {
		hwlog_err("%s: memset_s failed", __func__);
		return;
	}

	for (index = 0; index < WIN_LEN; index++) {
		info->e.imag += info->eh[index].imag;
		info->e.real += info->eh[index].real;
		info->p.imag += info->ph[index].imag;
		info->p.real += info->ph[index].real;
		info->l.imag += info->lh[index].imag;
		info->l.real += info->lh[index].real;
	}
	hwlog_info("%s:ch:%d, ms:%d,er:%d,ei:%d,pr:%d,pi:%d,lr:%d,li:%d,Q:%d",
		__func__, msg->chn_idx, msg->ms_idx, info->e.real,
		info->e.imag, info->p.real, info->p.imag,
		info->l.real, info->l.imag, info->q);
}

static s32 cost(s32 theta, s32 real, s64 q_power)
{
	return theta;
}

static void calc_cnr(struct chn_info *info, int idx)
{
	s64 cnr;
	s64 temp1;

	if (info == NULL)
		return;
	info->i_sum = info->i_sum - info->i_sum / SIG_ALPHA_COEFF +
		info->p.real;
	temp1 = info->p.real * info->p.real /
		(PLL_DATA_LEN * PLL_DATA_LEN * WIN_LEN * WIN_LEN);
	info->i_power = info->i_power - info->i_power / SIG_ALPHA_COEFF + temp1;
	info->q_power = info->q_power - info->q_power / NOS_ALPHA_COEFF +
		(info->q / (PLL_DATA_LEN * TIMES_3));
	cnr = (info->i_power * (1024 * NOS_ALPHA_COEFF / SIG_ALPHA_COEFF))
		/ info->q_power;
	hwlog_info("%s:ch:%d,Ip:%lld,Qp:%lld,Isum:%lld,cnr:%lld,flag:%d",
		__func__, idx, info->i_power, info->q_power,
		info->i_sum, cnr, info->p.real);
}

static u32 calc_clkdrift_1s(s64 doppler)
{
	u32 clkdrift_1s = 0;
	if (g_track_ctx.count_1s < DOPPLER_WIN_1S) {
		u32 index = g_track_ctx.index_1s;

		g_track_ctx.doppler_1s[index] = doppler;
		g_track_ctx.count_1s++;
		g_track_ctx.index_1s = (g_track_ctx.index_1s + 1) % DOPPLER_WIN_1S;
	} else if (g_track_ctx.count_1s == DOPPLER_WIN_1S) {
		s64 max = g_track_ctx.doppler_1s[0];
		s64 min = g_track_ctx.doppler_1s[0];
		u32 i, index;

		for (i = 0; i < DOPPLER_WIN_1S; i++) {
			index = (g_track_ctx.index_1s + i) % DOPPLER_WIN_1S;
			if (g_track_ctx.doppler_1s[index] > max)
				max = g_track_ctx.doppler_1s[index];
		}
		for (i = 0; i < DOPPLER_WIN_1S; i++) {
			index = (g_track_ctx.index_1s + i) % DOPPLER_WIN_1S;
			if (g_track_ctx.doppler_1s[index] < min)
				min = g_track_ctx.doppler_1s[index];
		}
		clkdrift_1s = (u32)(max - min);
		index = g_track_ctx.index_1s;
		g_track_ctx.doppler_1s[index] = doppler;
		g_track_ctx.index_1s = (index + 1) % DOPPLER_WIN_1S;
	}
	return clkdrift_1s;
}

static u32 calc_clkdrift_100ms(s64 doppler)
{
	u32 clkdrift_100ms = 0;
	if (g_track_ctx.count_100ms < DOPPLER_WIN_100MS) {
		u32 index = g_track_ctx.index_100ms;

		g_track_ctx.doppler_100ms[index] = doppler;
		g_track_ctx.count_100ms++;
		g_track_ctx.index_100ms = (g_track_ctx.index_100ms + 1) % DOPPLER_WIN_100MS;
	} else if (g_track_ctx.count_100ms == DOPPLER_WIN_100MS) {
		s64 max = g_track_ctx.doppler_100ms[0];
		s64 min = g_track_ctx.doppler_100ms[0];
		u32 i, index;

		for (i = 0; i < DOPPLER_WIN_100MS; i++) {
			index = (g_track_ctx.index_100ms + i) % DOPPLER_WIN_100MS;
			if (g_track_ctx.doppler_100ms[index] > max)
				max = g_track_ctx.doppler_100ms[index];
		}
		for (i = 0; i < DOPPLER_WIN_100MS; i++) {
			index = (g_track_ctx.index_100ms + i) % DOPPLER_WIN_100MS;
			if (g_track_ctx.doppler_100ms[index] < min)
				min = g_track_ctx.doppler_100ms[index];
		}
		clkdrift_100ms = (u32)(max - min);
		index = g_track_ctx.index_100ms;
		g_track_ctx.doppler_100ms[index] = doppler;
		g_track_ctx.index_100ms = (index + 1) % DOPPLER_WIN_100MS;
	}
	return clkdrift_100ms;
}

static void calc_carr_nco(struct chn_info *info, int idx, u32* clkdrift_1s, u32* clkdrift_100ms)
{
	s32 theta;
	s32 err;
	s64 carr_doppler;

	if (info == NULL)
		return;
	// carrier
	theta = -atan2(info->p.imag, info->p.real);
	err = cost(theta, info->p.real, info->q_power / NOS_ALPHA_COEFF) / TIMES_2;
	// enlarge 2048 * 4096,pll_gain enlarge 4096,err enlarge 2048
	info->dither += err * info->pll_gain[IDX_2];
	info->doppler += info->dither + err * info->pll_gain[IDX_1];
	// enlarge 2048 * 4096,pll_gain enlarge 4096,err enlarge 2048
	carr_doppler = info->doppler + err * info->pll_gain[IDX_0];
	info->carr_nco_word = (u32)(CARR_NCO_FW0 + carr_doppler / CARR_NCO_COEF);

	if (clkdrift_1s != NULL && clkdrift_100ms != NULL) {
		*clkdrift_100ms = calc_clkdrift_100ms(info->doppler);
		*clkdrift_1s = calc_clkdrift_1s(info->doppler);

		hwlog_info("%s:CARR:%d,%d,err:%d,dit:%lld,dop:%lld,carr:%lld,Nco:%08x,c1:%d,c2:%d",
			__func__, idx, theta, err, info->dither, info->doppler,
			carr_doppler, info->carr_nco_word, *clkdrift_1s, *clkdrift_100ms);
	}

	g_track_ctx.carr_nco[g_track_ctx.carr_nco_idx] = info->carr_nco_word;
	g_track_ctx.carr_nco_idx = (g_track_ctx.carr_nco_idx + 1) % CARR_NCO_WIN;
	if (g_track_ctx.carr_nco_cnt <= CARR_NCO_WIN - 1)
		g_track_ctx.carr_nco_cnt++;
}

static void calc_code_nco(struct chn_info *info)
{
	s64 temp1;
	s64 temp2;
	s64 err;
	s64 code_doppler;

	if (info == NULL)
		return;
	temp1 = info->e.real * info->e.real + info->e.imag * info->e.imag;
	temp2 = info->l.real * info->l.real + info->l.imag * info->l.imag;
	err = temp1 - temp2;

	temp1 = info->p.real * info->p.real + info->p.imag * info->p.imag;
	err = (err * CODE_NCO_ZOOM) / temp1;
	err = cost(err, info->p.real, info->q_power / NOS_ALPHA_COEFF);
	if (err > CODE_NCO_ZOOM)
		err = CODE_NCO_ZOOM;
	if (err < -CODE_NCO_ZOOM)
		err = -CODE_NCO_ZOOM;
	// enlarge 2^35，dll gain enlarge 2^25，dll_err enlarge 1024
	info->code_dither += err * info->dll_gain[2];
	info->code_doppler += info->code_dither + err * info->dll_gain[1];
	code_doppler = info->code_doppler + err * info->dll_gain[0];
	info->code_nco_word = (u32)(CODE_NCO_FW0 + code_doppler / CODE_NCO_COEF);
	hwlog_info("%s:COD:%lld,err:%lld,dit:%lld,dop:%lld,dop2:%lld,Nco:%08x",
		__func__, temp1, err, info->code_dither, info->code_doppler,
		code_doppler, info->code_nco_word);

	g_track_ctx.code_nco[g_track_ctx.code_nco_idx] = info->code_nco_word;
	g_track_ctx.code_nco_idx = (g_track_ctx.code_nco_idx + 1) % CODE_NCO_WIN;
	if (g_track_ctx.code_nco_cnt <= CODE_NCO_WIN - 1)
		g_track_ctx.code_nco_cnt++;
}

static int proc_msidx(struct chn_info *info)
{
	const u32 magic_code_cnt = 10;
	const u32 idx_max = 26;
	int cnt;

	if (info == NULL)
		return -EINVAL;
	info->idx++;
	for (cnt = 0; cnt < magic_code_cnt; cnt++) {
		if (magic_code[cnt] != 1)
			break;
	}
	if (info->idx < idx_max && cnt < magic_code_cnt)
		return -EINVAL;
	return 0;
}

s32 track_msg_calc(struct track_msg *msg)
{
	int chn_idx;
	struct chn_info *info = NULL;

	if (msg == NULL)
		return -1;
	// data preprocessing
	proc_pilot_data(msg);
	chn_idx = msg->chn_idx;
	g_track_ctx.chn_idx = msg->chn_idx;
	// user space compensation switching error
	info = &g_track_ctx.info[chn_idx];
	if (proc_msidx(info) < 0)
		return 1;
	// Calculate the carrier NCO.
	calc_carr_nco(info, chn_idx, &msg->clkdrift_1s, &msg->clkdrift_100ms);
	// Compute Code NCO
	calc_code_nco(info);
	// Send the correction calculation result.
	track_adjust(chn_idx, info->carr_nco_word, info->code_nco_word);
	// Calculate the carrier-to-noise ratio.
	calc_cnr(info, chn_idx);
	// PLL symbol
	return (info->p.real > 0) ? 1 : (-1);
}

void refresh_code_nco(void)
{
	struct chn_info *info = NULL;
	u32 i, index, code_nco;
	s64 nco_total = 0;
	hwlog_info("%s", __func__);
	if (g_track_ctx.code_nco_cnt == 0) {
		hwlog_err("%s: no code nco", __func__);
		return;
	}
	for (i = 0; i < g_track_ctx.code_nco_cnt; i++) {
		index = (g_track_ctx.code_nco_idx - g_track_ctx.code_nco_cnt + i + CODE_NCO_WIN) % CODE_NCO_WIN;
		nco_total += g_track_ctx.code_nco[index];
	}
	code_nco = (u32)(nco_total / (s64)g_track_ctx.code_nco_cnt);
	hwlog_info("%s: total:%lld,codeNco:%08x", __func__, nco_total, code_nco);
	info = &g_track_ctx.info[g_track_ctx.chn_idx];
	track_adjust(g_track_ctx.chn_idx, info->carr_nco_word, code_nco);
	update_nco_word(g_track_ctx.chn_idx);
}

void refresh_carr_nco(void)
{
	struct chn_info *info = NULL;
	u32 i, index, carr_nco;
	s64 nco_total = 0;
	hwlog_info("%s", __func__);
	if (g_track_ctx.carr_nco_cnt == 0) {
		hwlog_err("%s: no carr nco", __func__);
		return;
	}
	for (i = 0; i < g_track_ctx.carr_nco_cnt; i++) {
		index = (g_track_ctx.carr_nco_idx - g_track_ctx.carr_nco_cnt + i + CARR_NCO_WIN) % CARR_NCO_WIN;
		nco_total += g_track_ctx.carr_nco[index];
	}
	carr_nco = (u32)(nco_total / (s64)g_track_ctx.carr_nco_cnt);
	hwlog_info("%s: total:%lld,carrNco:%08x", __func__, nco_total, carr_nco);
	info = &g_track_ctx.info[g_track_ctx.chn_idx];
	track_adjust(g_track_ctx.chn_idx, carr_nco, info->code_nco_word);
	update_nco_word(g_track_ctx.chn_idx);
}

void refresh_nco(void)
{
	struct chn_info *info = NULL;
	u32 i, index, carr_nco, code_nco;
	s64 carr_nco_total = 0;
	s64 code_nco_total = 0;
	hwlog_info("%s", __func__);
	info = &g_track_ctx.info[g_track_ctx.chn_idx];
	if (g_track_ctx.carr_nco_cnt == 0) {
		hwlog_err("%s: no carr nco", __func__);
		carr_nco = info->carr_nco_word;
	} else {
		for (i = 0; i < g_track_ctx.carr_nco_cnt; i++) {
			index = (g_track_ctx.carr_nco_idx - g_track_ctx.carr_nco_cnt + i + CARR_NCO_WIN) % CARR_NCO_WIN;
			carr_nco_total += g_track_ctx.carr_nco[index];
		}
		carr_nco = (u32)(carr_nco_total / (s64)g_track_ctx.carr_nco_cnt);
		hwlog_info("%s: total:%lld,carrNco:%08x", __func__, carr_nco_total, carr_nco);
	}
	if (g_track_ctx.code_nco_cnt == 0) {
		hwlog_err("%s: no code nco", __func__);
		code_nco = info->code_nco_word;
	} else {
		for (i = 0; i < g_track_ctx.code_nco_cnt; i++) {
			index = (g_track_ctx.code_nco_idx - g_track_ctx.code_nco_cnt + i + CODE_NCO_WIN) % CODE_NCO_WIN;
			code_nco_total += g_track_ctx.code_nco[index];
		}
		code_nco = (u32)(code_nco_total / (s64)g_track_ctx.code_nco_cnt);
		hwlog_info("%s: total:%lld,codeNco:%08x", __func__, code_nco_total, code_nco);
	}
	track_adjust(g_track_ctx.chn_idx, carr_nco, code_nco);
	update_nco_word(g_track_ctx.chn_idx);
}

void init_track(struct acq2track_msg *msg)
{
	int index;
	u8 chn_idx;

	if (msg == NULL)
		return;
	chn_idx = msg->chn_idx;
	if (chn_idx >= MAX_CHAN_NUM)
		return;
	for (index = 0; index < MAX_CHAN_NUM; index++) {
		g_track_ctx.info[index].pll_gain[IDX_0] = LOCKED_PLL_G0;
		g_track_ctx.info[index].pll_gain[IDX_1] = LOCKED_PLL_G1;
		g_track_ctx.info[index].pll_gain[IDX_2] = LOCKED_PLL_G2;
		g_track_ctx.info[index].dll_gain[IDX_0] = LOCKED_DLL_G0;
		g_track_ctx.info[index].dll_gain[IDX_1] = LOCKED_DLL_G1;
		g_track_ctx.info[index].dll_gain[IDX_2] = LOCKED_DLL_G2;
	}
	g_track_ctx.info[chn_idx].doppler = msg->doppler;
	g_track_ctx.info[chn_idx].dither = msg->dither;
	g_track_ctx.info[chn_idx].code_doppler = msg->code_doppler;
	g_track_ctx.info[chn_idx].code_dither = msg->code_dither;
	g_track_ctx.info[chn_idx].carr_nco_word = msg->carr_nco_word;
	g_track_ctx.info[chn_idx].code_nco_word = msg->code_nco_word;
	g_track_ctx.info[chn_idx].idx = 0;
	g_track_ctx.info[chn_idx].win_idx = 0;
	g_track_ctx.info[chn_idx].i_power = 0;
	g_track_ctx.info[chn_idx].q_power = 0;
	g_track_ctx.info[chn_idx].i_sum = 0;
	g_track_ctx.info[chn_idx].cn0 = 0;
	for (index = 0; index < DOPPLER_WIN_1S; index++)
		g_track_ctx.doppler_1s[index] = 0;
	g_track_ctx.index_1s = 0;
	g_track_ctx.count_1s = 0;
	for (index = 0; index < DOPPLER_WIN_100MS; index++)
		g_track_ctx.doppler_100ms[index] = 0;
	g_track_ctx.index_100ms = 0;
	g_track_ctx.count_100ms = 0;
	for (index = 0; index < CODE_NCO_WIN; index++)
		g_track_ctx.code_nco[index] = 0;
	g_track_ctx.code_nco_idx = 0;
	g_track_ctx.code_nco_cnt = 0;
	for (index = 0; index < CARR_NCO_WIN; index++)
		g_track_ctx.carr_nco[index] = 0;
	g_track_ctx.carr_nco_idx = 0;
	g_track_ctx.carr_nco_cnt = 0;
}

void set_magic_code(struct rx_init_msg *msg)
{
	int ret;
	if (msg == NULL)
		return;
	ret = memcpy_s(magic_code, FRM_PERIOD, msg->magic_code, FRM_PERIOD);
	if (ret != EOK)
		hwlog_err("%s: memcpy_s failed", __func__);
}

