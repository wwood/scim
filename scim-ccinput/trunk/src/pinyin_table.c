/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

/*
 *  CCInput
 *
 *  Copyright (C) 2003, 2004 CCOSS, Inc.
 *
 *
 *  CCInput is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  CCInput is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: ZHANG Qian <zhangqian@ccoss.com.cn>
 *
 */


#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

#include <sys/types.h>
#include <string.h>

#include "ccinput.h"

//标准拼音码表
const ccinSyllable_t g_standard_syllable_table[SYLLABLE_TOTAL] = {
	"a", "ai", "an", "ang", "ao",
	"ba", "bai", "ban", "bang", "bao", "bei", "ben", "beng", "bi", "bian",
		"biao", "bie", "bin", "bing", "bo", "bu",
	"ca", "cai", "can", "cang", "cao", "ce", "cen", "ceng", "ci", "cong",
		"cou", "cu", "cuan", "cui", "cun", "cuo",
	"cha", "chai", "chan", "chang", "chao", "che", "chen", "cheng", "chi",
		"chong", "chou", "chu", "chuai", "chuan", "chuang", "chui", "chun",
		"chuo",
	"da", "dai", "dan", "dang", "dao", "de", "dei", "den" /*GBK*/, "deng",
		"di", "dia", "dian", "diao", "die", "ding", "diu", "dong", "dou",
		"du", "duan", "dui", "dun", "duo",
	"e", "ei", "en", "eng" /*GBK*/, "er",
	"fa", "fan", "fang", "fei", "fen", "feng", "fo", "fou", "fu",
	"ga", "gai", "gan", "gang", "gao", "ge", "gei", "gen", "geng", "gong",
		"gou", "gu", "gua", "guai", "guan", "guang", "gui", "gun", "guo",
	"ha", "hai", "han", "hang", "hao", "he", "hei", "hen", "heng", "hong",
		"hou", "hu", "hua", "huai", "huan", "huang", "hui", "hun", "huo",
	"ji", "jia", "jian", "jiang", "jiao", "jie", "jin", "jing", "jiong",
		"jiu", "ju", "juan", "jue", "jun",
	"ka", "kai", "kan", "kang", "kao", "ke", "kei" /*GBK*/, "ken", "keng",
		"kong", "kou", "ku", "kua", "kuai", "kuan", "kuang", "kui", "kun",
		"kuo",
	"la", "lai", "lan", "lang", "lao", "le", "lei", "leng", "li", "lia",
		"lian", "liang", "liao", "lie", "lin", "ling", "liu", "lo", "long",
		"lou", "lu", "luan", "lue", "lun", "luo", "lv",
	"m", "ma", "mai", "man", "mang", "mao", "me", "mei", "men", "meng", "mi",
		"mian", "miao", "mie", "min", "ming", "miu", "mo", "mou", "mu",
	"n", "na", "nai", "nan", "nang", "nao", "ne", "nei", "nen", "neng", "ng",
		"ni", "nian", "niang", "niao", "nie", "nin", "ning", "niu", "nong",
		"nou", "nu", "nuan", "nue", "nuo", "nv",
	"o", "ou",
	"pa", "pai", "pan", "pang", "pao", "pei", "pen", "peng", "pi", "pian",
		"piao", "pie", "pin", "ping", "po", "pou", "pu",
	"qi", "qia", "qian", "qiang", "qiao", "qie", "qin", "qing", "qiong",
		"qiu", "qu", "quan", "que", "qun",
	"ran", "rang", "rao", "re", "ren", "reng", "ri", "rong", "rou", "ru",
		"rua" /*GBK*/, "ruan", "rui", "run", "ruo",
	"sa", "sai", "san", "sang", "sao", "se", "sen", "seng", "si", "song",
		"sou", "su", "suan", "sui", "sun", "suo",
	"sha", "shai", "shan", "shang", "shao", "she", "shei", "shen", "sheng",
		"shi", "shou", "shu", "shua", "shuai", "shuan", "shuang", "shui",
		"shun", "shuo",
	"ta", "tai", "tan", "tang", "tao", "te", "tei", "teng", "ti", "tian",
		"tiao", "tie", "ting", "tong", "tou", "tu", "tuan", "tui", "tun",
		"tuo",
	"wa", "wai", "wan", "wang", "wei", "wen", "weng", "wo", "wu",
	"xi", "xia", "xian", "xiang", "xiao", "xie", "xin", "xing", "xiong",
		"xiu", "xu", "xuan", "xue", "xun",
	"ya", "yan", "yang", "yao", "ye", "yi", "yin", "ying", "yo", "yong",
		"you", "yu", "yuan", "yue", "yun",
	"za", "zai", "zan", "zang", "zao", "ze", "zei", "zen", "zeng", "zi",
		"zong", "zou", "zu", "zuan", "zui", "zun", "zuo",
	"zha", "zhai", "zhan", "zhang", "zhao", "zhe", "zhei", "zhen", "zheng",
		"zhi", "zhong", "zhou", "zhu", "zhua", "zhuai", "zhuan", "zhuang",
		"zhui", "zhun", "zhuo",
};

//设本音节为u_char * ThisSyllable，则可快速定位其拼音码值编号在SyllableHash[ThisSyllable[0]-'a'][0]与SyllableHash[ThisSyllable[0]-'a'][0]+SyllableHash[ThisSyllable[0]-'a'][1]之间；若ThisSyllable[0]为'c'且ThisSyllable[1]为'h'，起始位置加6；若ThisSyllable[0]为's'且ThisSyllable[1]为'h'，起始位置加2；若ThisSyllable[0]为'z'且ThisSyllable[1]为'h'，起始位置减4。
const u_short g_syllable_hash[LETTER_NUM][2] = {
	{1, 5},						//a
	{6, 16},					//b
	{22, 16},					//c
	{56, 23},					//d
	{79, 5},					//e
	{84, 9},					//f
	{93, 19},					//g
	{112, 19},					//h
//ch为首的音节，需要做加6的换算。借用i的位置。
	{38, 18},					//ch
	{131, 14},					//j
	{145, 19},					//k
	{164, 26},					//l
	{190, 20},					//m
	{210, 26},					//n
	{236, 2},					//o
	{238, 17},					//p
	{255, 14},					//q
	{269, 15},					//r
	{284, 16},					//s
	{319, 20},					//t
//sh为首的音节，需要做加2的换算。借用u的位置。
	{300, 19},					//sh
//zh为首的音节，需要做减4的换算。借用v的位置。
	{394, 20},					//zh
	{339, 9},					//w
	{348, 14},					//x
	{362, 15},					//y
	{377, 17}					//z
};


//以下关于模糊音
const ccinFuzzyPinYinKey_t g_fuzzy_syllable[] = {	//整音节
	{"wang", "huang"},
//  {"\0", "\0"}    //End flag
};
const ccinFuzzyPinYinKey_t g_fuzzy_final[] = {	//韵母
	{"an", "ang"},				//把{"ian", "iang"}和{"uan", "uang"}统一在此处）
	{"en", "eng"},
	{"in", "ing"},
//  {"\0", "\0"}    //End flag
};
const ccinFuzzyPinYinKey_t g_fuzzy_initial[] = {	//声母
	{"c", "ch"},
	{"s", "sh"},
	{"z", "zh"},
	{"f", "h"},
	{"l", "n"},
	{"k", "g"},
	{"r", "l"},
//  {"\0", "\0"}    //End flag
};

const ccinFuzzyInvalidSyllable_t g_fuzzy_invalid_syllable_table[] = {
	{"zuang", FUZZY_FINAL_1 | FUZZY_INITIAL_3},	//{"uan","uang"}, {"z","zh"}
	{"zuai", FUZZY_INITIAL_3},	//{"z","zh"}
	{"zua", FUZZY_INITIAL_3},	//{"z","zh"}
//zhei＝zei（但是应该有zhei而miniChinput没有，也即是说，标准拼音表中应该加入） 

	{"yuang", FUZZY_FINAL_1},	//{"uan","uang"} 紫光没有

	{"xuang", FUZZY_FINAL_1},	//{"uan","uang"} 紫光没有

	{"tuang", FUZZY_FINAL_1},	//{"uan","uang"} 紫光没有
	{"tin", FUZZY_FINAL_3},		//{"in","ing"}
	{"tiang", FUZZY_FINAL_1},	//{"ian","iang"} 紫光没有
	{"ten", FUZZY_FINAL_2},		//{"en","eng"}

	{"suang", FUZZY_FINAL_1 | FUZZY_INITIAL_2},	//{"uan","uang"}, {"s","sh"}
	{"suai", FUZZY_INITIAL_2},	//{"s","sh"}
	{"sua", FUZZY_INITIAL_2},	//{"s","sh"}
	{"shong", FUZZY_INITIAL_2},	//{"s","sh"}
	{"sei", FUZZY_INITIAL_2},	//{"s","sh"}

//  {"rv", FUZZY_INITIAL_7},  //{"r","l"} 紫光没有
//  {"rue", fuzzy_ initial_7},  //{"r","l"} 紫光没有
	{"ruang", FUZZY_FINAL_1},	//{"uan","uang"} 紫光没有
//  {"rei", fuzzy_ initial_7},  //{"r","l"} 紫光没有
//  {"rai", fuzzy_ initial_7},  //{"r","l"} 紫光没有
//  {"ra", fuzzy_ initial_7},  //{"r","l"} 紫光没有
//【ri之后的拼音一般不会模糊为li*，如lia lian liang liao lie lin ling liu，故不录】

	{"quang", FUZZY_FINAL_1},	//{"uan","uang"} 紫光没有

	{"piang", FUZZY_FINAL_1},	//{"ian","iang"} 紫光没有

//  {"nun", FUZZY_INITIAL_5},  //{"l","n"}
	{"nuang", FUZZY_FINAL_1},	//{"uan","uang"} 紫光没有
//  {"nia", FUZZY_INITIAL_5},  //{"l","n"} 紫光没有

	{"miang", FUZZY_FINAL_1},	//{"ian","iang"} 紫光没有

	{"luang", FUZZY_FINAL_1},	//{"uan","uang"} 紫光没有
	{"len", FUZZY_INITIAL_5 | FUZZY_INITIAL_7},	//{"l","n"}, {“r”,”l”}

	{"juang", FUZZY_FINAL_1},	//{"uan","uang"} 紫光没有

//  {"fuo", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有
//  {"fun", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有
//  {"fui", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有
//  {"fuang", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有
//  {"fuan", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有
//  {"fuai", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有
//  {"fua", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有
//  {"fe", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有
//  {"fao", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有
//  {"fai", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有

//  {"ho", FUZZY_INITIAL_4},  //{"f","h"} 紫光没有

	{"duang", FUZZY_FINAL_1},	//{"uan","uang"} 紫光没有
	{"din", FUZZY_FINAL_3},		//{"in","ing"}
	{"diang", FUZZY_FINAL_1},	//{"ian","iang"} 紫光没有

	{"cuang", FUZZY_FINAL_1 | FUZZY_INITIAL_1},	//{"uan","uang"}, {"c","ch"}
	{"cuai", FUZZY_INITIAL_1},	//{"c","ch"}

	{"biang", FUZZY_FINAL_1},	//{"ian","iang"} 紫光没有

	{"\0", 0}					//End flag
};


/* 以下关于双拼 */
/* “中文之星/四通利方”方案 */
const ccinSPMappingKey_t g_sp_config_st [LETTER_NUM+1] =
{
	{NULL, "a", NULL},	//'A' key
	{"b", "ia", "ua"},	//'B' key
	{"c", "uan", NULL},
	{"d", "ao", NULL},
	{NULL, "e", NULL},
	{"f", "an", NULL},
	{"g", "ang", NULL},
	{"h", "uang", "iang"},
	{"sh", "i", NULL},
	{"j", "ian", NULL},
	{"k", "iao", NULL},
	{"l", "in", NULL},
	{"m", "ie", NULL},
	{"n", "iu", NULL},
	{"'", "o", "uo"},
	{"p", "ou", NULL},
	{"q", "ing", "er"},
	{"r", "en", NULL},
	{"s", "ai", NULL},
	{"t", "eng", NULL},
	{"ch", "u", NULL},
	{"zh", "v", "ui"},
	{"w", "ei", NULL},
	{"x", "uai", "ue"},
	{"y", "ong", "iong"},
	{"z", "un", NULL},	//'Z' key
	{NULL, NULL, NULL},	//';' key
};

/* “自然码”方案 */
const ccinSPMappingKey_t g_sp_config_zr [LETTER_NUM+1] =
{
	{NULL, "a", NULL},	//'A' key
	{"b", "ou", NULL},	//'B' key
	{"c", "iao", NULL},
	{"d", "uang", "iang"},
	{NULL, "e", NULL},
	{"f", "en", NULL},
	{"g", "eng", NULL},
	{"h", "ang", NULL},
	{"ch", "i", NULL},
	{"j", "an", NULL},
	{"k", "ao", NULL},
	{"l", "ai", NULL},
	{"m", "ian", NULL},
	{"n", "in", NULL},
	{"'", "o", "uo"},
	{"p", "un", NULL},
	{"q", "iu", NULL},
	{"r", "uan", "er"},
	{"s", "ong", "iong"},
	{"t", "ue", NULL},
	{"sh", "u", NULL},
	{"zh", "v", "ui"},
	{"w", "ia", "ua"},
	{"x", "ie", NULL},
	{"y", "ing", "uai"},
	{"z", "ei", NULL},	//'Z' key
	{NULL, NULL, NULL},	//';' key
};

/* “微软”方案 */
const ccinSPMappingKey_t g_sp_config_ms [LETTER_NUM+1] =
{
	{NULL, "a", NULL},	//'A' key
	{"b", "ou", NULL},	//'B' key
	{"c", "iao", NULL},
	{"d", "uang", "iang"},
	{NULL, "e", NULL},
	{"f", "en", NULL},
	{"g", "eng", NULL},
	{"h", "ang", NULL},
	{"ch", "i", NULL},
	{"j", "an", NULL},
	{"k", "ao", NULL},
	{"l", "ai", NULL},
	{"m", "ian", NULL},
	{"n", "in", NULL},
	{"'", "o", "uo"},
	{"p", "un", NULL},
	{"q", "iu", NULL},
	{"r", "uan", "er"},
	{"s", "ong", "iong"},
	{"t", "ue", NULL},
	{"sh", "u", NULL},
	{"zh", "ui", "ue"},
	{"w", "ia", "ua"},
	{"x", "ie", NULL},
	{"y", "uai", "v"},
	{"z", "ei", NULL},	//'Z' key
	{NULL, "ing", NULL},	//';' key
};

/* “紫光”方案 */
const ccinSPMappingKey_t g_sp_config_zg [LETTER_NUM+1] =
{
	{"ch", "a", NULL},	//'A' key
	{"b", "iao", NULL},	//'B' key
	{"c", "ing", NULL},
	{"d", "ie", NULL},
	{NULL, "e", NULL},
	{"f", "ian", NULL},
	{"g", "uang", "iang"},
	{"h", "ong", "iong"},
	{"sh", "i", NULL},
	{"j", "iu", "er"},
	{"k", "ei", NULL},
	{"l", "uan", NULL},
	{"m", "un", NULL},
	{"n", "ui", "ue"},
	{"'", "o", "uo"},
	{"p", "ai", NULL},
	{"q", "ao", NULL},
	{"r", "an", NULL},
	{"s", "ang", NULL},
	{"t", "eng", NULL},
	{"zh", "u", NULL},
	{NULL, "v", NULL},
	{"w", "en", NULL},
	{"x", "ia", "ua"},
	{"y", "in", "uai"},
	{"z", "ou", NULL},	//'Z' key
	{NULL, NULL, NULL},	//';' key
};

/* “智能ABC”方案 */
const ccinSPMappingKey_t g_sp_config_zn [LETTER_NUM+1] =
{
	{"zh", "a", NULL},	//'A' key
	{"b", "ou", NULL},	//'B' key
	{"c", "in", "uai"},
	{"d", "ia", "ua"},
	{"ch", "e", NULL},
	{"f", "en", NULL},
	{"g", "eng", NULL},
	{"h", "ang", NULL},
	{NULL, "i", NULL},
	{"j", "an", NULL},
	{"k", "ao", NULL},
	{"l", "ai", NULL},
	{"m", "ui", "ue"},
	{"n", "un", NULL},
	{"'", "o", "uo"},
	{"p", "uan", NULL},
	{"q", "ei", NULL},
	{"r", "iu", "er"},
	{"s", "ong", "iong"},
	{"t", "uang", "iang"},
	{NULL, "u", NULL},
	{"sh", "v", NULL},
	{"w", "ian", NULL},
	{"x", "ie", NULL},
	{"y", "ing", NULL},
	{"z", "iao", NULL},	//'Z' key
	{NULL, NULL, NULL},	//';' key
};

/* “刘氏双拼”方案 */
const ccinSPMappingKey_t g_sp_config_ls [LETTER_NUM+1] =
{
	{NULL, "a", NULL},	//'A' key
	{"b", "ao", NULL},	//'B' key
	{"c", "ang", NULL},
	{"d", "uan", NULL},
	{NULL, "e", NULL},
	{"f", "an", NULL},
	{"g", "ong", "iong"},
	{"h", "ui", "ue"},
	{"ch", "i", NULL},
	{"j", "ia", "ua"},
	{"k", "un", NULL},
	{"l", "iu", NULL},
	{"m", "in", NULL},
	{"n", "uang", "iang"},
	{"'", "o", "uo"},
	{"p", "eng", NULL},
	{"q", "ing", NULL},
	{"r", "ou", "er"},
	{"s", "ai", NULL},
	{"t", "ian", NULL},
	{"sh", "u", NULL},
	{"zh", "v", "en"},
	{"w", "ei", NULL},
	{"x", "ie", NULL},
	{"y", "uai", NULL},
	{"z", "iao", NULL},	//'Z' key
	{NULL, NULL, NULL},	//';' key
};


#pragma pack(pop)

#ifdef __cplusplus
}
#endif
