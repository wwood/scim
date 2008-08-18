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


#ifndef _CCINPUT_H
#define _CCINPUT_H

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

#include "datatype.h"

#define SYLLABLE_TOTAL	413		//should same with PinyinTable
#define LETTER_NUM	26			//+ch sh zh –i u v
#define MAX_LETTER_IN_SYLLABLE	7
#define MAX_SYLLABLE_IN_PHRASE	9
#define MAX_CHAR_IN_ORIGIN_PINYIN_STRING	50
#define DEFAULT_ORIGINAL_FREQ  240


#define MAX_EXTRACTED_SYLLABLE	68
	//68是一个音节最多可能（由于简拼加模糊）产生的全拼音节（l,n,r）
#define MAX_EXTRACTED_FUZZY_SYLLABLE	8
	//8是一个音节最多可能（只是由于模糊）产生的全拼或者简拼音节（huang或lang目前为6）

#define MAX_FREQ_NUM	255
#define MIN_FREQ_NUM	0
#define FREQ_ADJUST_VALUE	1

#define _CCIN_FOR_SCIM

#ifndef _CCIN_FOR_SCIM
#define SYSTEM_FILE_PATH	"../data/"
#define USER_FILE_PATH	"/.ccinput/"
#else
#define SYSTEM_FILE_PATH	"/usr/share/scim/ccinput/"
#define USER_FILE_PATH	"/.ccinput/"
#endif

#define SYSTEM_GLOSSARY_FILE_NAME	"sysgloss.tbl"
#define SYSTEM_FREQUENCY_FILE_NAME	"sysfreq.tbl"
#define USER_GLOSSARY_FILE_NAME       "usrgloss.tbl"
#define USER_FREQUENCY_FILE_NAME      "usrfreq.tbl"


//一个音节
typedef u_char ccinSyllable_t[MAX_LETTER_IN_SYLLABLE];


//词条类型定义分为四种：GB单字类型、GBK单字类型、一般词条类型、长词条类型。
//GB单字类型内容包括字值指针、拼音码值指针、字频值、频率链表指针。
typedef struct ccinGBWordInfo
{
	struct ccinGBWordInfo *pos_next;
	ccinHanziChar_t word;
	u_short pinyin_key;
	u_char freq;
	struct ccinGBWordInfo *freq_prev;
	struct ccinGBWordInfo *freq_next;
}
ccinGBWordInfo_t;

//GBK单字类型内容包括字值指针、拼音码值指针。
typedef struct ccinGBKWordInfo
{
	struct ccinGBKWordInfo *pos_next;
	ccinHanziChar_t word;
	u_short pinyin_key;
}
ccinGBKWordInfo_t;

//一般词条类型包括词值指针、拼音码值指针、词频值、频率链表指针、用户自造标志。
typedef struct ccinPhraseTwoWordInfo
{
	struct ccinPhraseTwoWordInfo *pos_next;
	ccinHanziChar_t phrase[2];
	u_short pinyin_key[2];
	u_char freq;
	struct ccinPhraseTwoWordInfo *freq_prev;
	struct ccinPhraseTwoWordInfo *freq_next;
	u_int is_system;
}
ccinPhraseTwoWordInfo_t;
typedef struct ccinPhraseThreeWordInfo
{
	struct ccinPhraseThreeWordInfo *pos_next;
	ccinHanziChar_t phrase[3];
	u_short pinyin_key[3];
	u_char freq;
	struct ccinPhraseThreeWordInfo *freq_prev;
	struct ccinPhraseThreeWordInfo *freq_next;
	u_int is_system;
}
ccinPhraseThreeWordInfo_t;
typedef struct ccinPhraseFourWordInfo
{
	struct ccinPhraseFourWordInfo *pos_next;
	ccinHanziChar_t phrase[4];
	u_short pinyin_key[4];
	u_char freq;
	struct ccinPhraseFourWordInfo *freq_prev;
	struct ccinPhraseFourWordInfo *freq_next;
	u_int is_system;
}
ccinPhraseFourWordInfo_t;

//长词条类型包括词条字数、词值指针、拼音码值指针、词频值、频率链表指针、用户自造标志。
typedef struct ccinLongPhraseInfo
{
	struct ccinLongPhraseInfo *pos_next;
	u_short word_number;
	ccinHanziChar_t phrase[MAX_SYLLABLE_IN_PHRASE];
	u_short pinyin_key[MAX_SYLLABLE_IN_PHRASE];
	u_int is_system;
}
ccinLongPhraseInfo_t;


//描述词库中每个音节段的所有字词信息
typedef struct ccinGlossarySyllableInfo
{
	u_short syllable_phrase_total;
	u_short phrase_word_gb_num;
	ccinGBWordInfo_t *sys_phrase_word_gb;
	u_short phrase_word_gbk_num;
	ccinGBKWordInfo_t *sys_phrase_word_gbk;
	u_short phrase_two_word_num;
	ccinPhraseTwoWordInfo_t *sys_phrase_two_word;
	u_short phrase_three_word_num;
	ccinPhraseThreeWordInfo_t *sys_phrase_three_word;
	u_short phrase_four_word_num;
	ccinPhraseFourWordInfo_t *sys_phrase_four_word;
	u_short phrase_above_four_num;
	ccinLongPhraseInfo_t *sys_phrase_above_four;
}
ccinGlossarySyllableInfo_t;

//描述词库中所有音节涉及的字词信息
typedef struct ccinGlossaryTableInfo
{
	u_long all_system_phrase_total;
	ccinGlossarySyllableInfo_t sys_syllable_info[SYLLABLE_TOTAL];
}
ccinGlossaryTableInfo_t;


//有共同首音节的词组文件块的头信息类型
typedef struct ccinSyllableSegmentHeadInfo
{
	u_long offset;				//该结构在文件中的偏移量
	u_short size;				//该结构的字节数
}
ccinSyllableSegmentHeadInfo_t;

//词库文件头类型
typedef struct ccinGlossaryFileHead
{
	u_long phrase_total;
	ccinSyllableSegmentHeadInfo_t
		phrase_syllable_segment_head_info[SYLLABLE_TOTAL];
}
ccinGlossaryFileHead_t;

//有共同首音节的词组文件块的头类型（词库文件）
typedef struct ccinSyllableSegmentHead
{
	u_short pinyin_key;			//拼音码值(2)
	u_short word_phrase_flag;	//字词标志位(2)
	u_short word_phrase_total;	//字词总数(2)
	u_short *word_phrase_num_pointer;	//各字词数的指针(?)
//no use
//后续的各字数的词条数目还是要访问，得到值后赋到别的地方（ccinGlossarySyllableInfo_t）
//  u_short *word_numbers;  //各字数的词条数目(由标志位定)
}
ccinSyllableSegmentHead_t;


//以下关于模糊音
typedef struct ccinFuzzyPinYinKey
{
	char *p_pinyinkey_1;
	char *p_pinyinkey_2;
}
ccinFuzzyPinYinKey_t;

//Masks for fuzzy pinyin cause
enum ccinFuzzyCause
{								//实际上与下面定义的各个ccinFuzzyPinYinKey_t一一对应
	FUZZY_SYLLABLE_1 = 0x01,	//整音节模糊
	FUZZY_FINAL_1 = 0x10,		//韵母模糊
	FUZZY_FINAL_2 = 0x20,
	FUZZY_FINAL_3 = 0x40,
	FUZZY_INITIAL_1 = 0x100,	//声母模糊
	FUZZY_INITIAL_2 = 0x200,
	FUZZY_INITIAL_3 = 0x400,
	FUZZY_INITIAL_4 = 0x800,
	FUZZY_INITIAL_5 = 0x1000,
	FUZZY_INITIAL_6 = 0x2000,
	FUZZY_INITIAL_7 = 0x4000,
};
typedef struct ccinFuzzyInvalidSyllable
{
	char syllable[MAX_LETTER_IN_SYLLABLE];
	u_long fuzzy_cause;			//表示模糊原因的项
}
ccinFuzzyInvalidSyllable_t;


//以下关于双拼
typedef struct ccinSPMappingKey
{
	char * sp_initial_key;
	char * sp_final_key_1;
	char * sp_final_key_2;
}
ccinSPMappingKey_t;

typedef enum ccinSPConfigureEnum
{
	SP_CONFIG_ST = 0,
	SP_CONFIG_ZR,
	SP_CONFIG_MS,
	SP_CONFIG_ZG,
	SP_CONFIG_ZN,
	SP_CONFIG_LS,
}
ccinSPConfigureEnum_t;

typedef enum ccinPhraseType
{
	WORD_GB = 1,
	WORD_GBK = 0,
	PHRASE_TWO = 2,
	PHRASE_THREE,
	PHRASE_FOUR,
	PHRASE_LONG,
	PHRASE_FIVE = 5,
	PHRASE_SIX,
	PHRASE_SEVEN,
	PHRASE_EIGHT,
	PHRASE_NINE,
}
ccinPhraseType_t;

enum ccinSyllableType
{
//0x01简拼；0x02整音节模糊；0x04声母模糊；0x08韵母模糊。
	SYLLABLE_TYPE_SIMPLY_SPELL = 0x01,
	SYLLABLE_TYPE_FUZZY_WHOLE = 0x02,
	SYLLABLE_TYPE_FUZZY_INITIAL = 0x04,
	SYLLABLE_TYPE_FUZZY_FINAL = 0x08,
};


typedef struct ccinGBWordInfoList
{
	ccinGBWordInfo_t *lookup_word_gb;
	struct ccinGBWordInfoList *next;
}
ccinGBWordInfoList_t;
typedef struct ccinGBKWordInfoList
{
	ccinGBKWordInfo_t *lookup_word_gbk;
	struct ccinGBKWordInfoList *next;
}
ccinGBKWordInfoList_t;
typedef struct ccinPhraseTwoWordInfoList
{
	ccinPhraseTwoWordInfo_t *lookup_phrase_word_two;
	struct ccinPhraseTwoWordInfoList *next;
}
ccinPhraseTwoWordInfoList_t;
typedef struct ccinPhraseThreeWordInfoList
{
	ccinPhraseThreeWordInfo_t *lookup_phrase_word_three;
	struct ccinPhraseThreeWordInfoList *next;
}
ccinPhraseThreeWordInfoList_t;
typedef struct ccinPhraseFourWordInfoList
{
	ccinPhraseFourWordInfo_t *lookup_phrase_word_four;
	struct ccinPhraseFourWordInfoList *next;
}
ccinPhraseFourWordInfoList_t;
typedef struct ccinLongPhraseInfoList
{
	ccinLongPhraseInfo_t *lookup_long_phrase;
	struct ccinLongPhraseInfoList *next;
}
ccinLongPhraseInfoList_t;
typedef struct ccinLookupResult
{
	u_short lookup_total;
	u_short lookup_word_gb_num;
	ccinGBWordInfoList_t *lookup_word_gb_list;
	u_short lookup_word_gbk_num;
	ccinGBKWordInfoList_t *lookup_word_gbk_list;
	u_short lookup_two_word_num;
	ccinPhraseTwoWordInfoList_t *lookup_two_word_list;
	u_short lookup_three_word_num;
	ccinPhraseThreeWordInfoList_t *lookup_three_word_list;
	u_short lookup_four_word_num;
	ccinPhraseFourWordInfoList_t *lookup_four_word_list;
	u_short lookup_above_four_num;
	ccinLongPhraseInfoList_t *lookup_above_four_list;
}
ccinLookupResult_t;


typedef struct ccinIMContext
{
	//对用户键盘输入和中间解析状态的记录。
	u_char origin_pinyin_buffer[MAX_CHAR_IN_ORIGIN_PINYIN_STRING];	//50
	u_char undecomposed_pinyin_buffer[MAX_CHAR_IN_ORIGIN_PINYIN_STRING];
	ccinHanziChar_t translated_hanzi_buffer[MAX_SYLLABLE_IN_PHRASE];	//9
	u_char display_pinyin_buffer[MAX_CHAR_IN_ORIGIN_PINYIN_STRING];	//即SCIM要求的pre_edit串

	ccinSyllable_t pinyin_syllable_buffer[MAX_SYLLABLE_IN_PHRASE];	//拼音组是逆序放置的，即首音节在最后
	u_short current_total_pinyin_length;
	u_short current_pinyin_position;

	//对已解析的拼音串产生的检索结果备选链表。
	ccinLookupResult_t *lookup_result;
	u_long lookup_result_display_first;
	u_long lookup_result_display_last;

	u_long flag_fuzzy;			//用户设置初始为没有模糊处理
	//这个设置可由FuzzyCause_t检验，不同的位表示不同的模糊设置。
	int flag_chinese;			//中英切换
	int flag_cn_punctuation;	//中标点
	int flag_SBC;				//全角
	int flag_GBK;				//GBK支持
	int flag_traditional;		//繁体
}
ccinIMContext_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
