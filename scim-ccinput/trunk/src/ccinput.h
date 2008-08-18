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
#define LETTER_NUM	26			//+ch sh zh �Ci u v
#define MAX_LETTER_IN_SYLLABLE	7
#define MAX_SYLLABLE_IN_PHRASE	9
#define MAX_CHAR_IN_ORIGIN_PINYIN_STRING	50
#define DEFAULT_ORIGINAL_FREQ  240


#define MAX_EXTRACTED_SYLLABLE	68
	//68��һ�����������ܣ����ڼ�ƴ��ģ����������ȫƴ���ڣ�l,n,r��
#define MAX_EXTRACTED_FUZZY_SYLLABLE	8
	//8��һ�����������ܣ�ֻ������ģ����������ȫƴ���߼�ƴ���ڣ�huang��langĿǰΪ6��

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


//һ������
typedef u_char ccinSyllable_t[MAX_LETTER_IN_SYLLABLE];


//�������Ͷ����Ϊ���֣�GB�������͡�GBK�������͡�һ��������͡����������͡�
//GB�����������ݰ�����ֵָ�롢ƴ����ֵָ�롢��Ƶֵ��Ƶ������ָ�롣
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

//GBK�����������ݰ�����ֵָ�롢ƴ����ֵָ�롣
typedef struct ccinGBKWordInfo
{
	struct ccinGBKWordInfo *pos_next;
	ccinHanziChar_t word;
	u_short pinyin_key;
}
ccinGBKWordInfo_t;

//һ��������Ͱ�����ֵָ�롢ƴ����ֵָ�롢��Ƶֵ��Ƶ������ָ�롢�û������־��
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

//���������Ͱ���������������ֵָ�롢ƴ����ֵָ�롢��Ƶֵ��Ƶ������ָ�롢�û������־��
typedef struct ccinLongPhraseInfo
{
	struct ccinLongPhraseInfo *pos_next;
	u_short word_number;
	ccinHanziChar_t phrase[MAX_SYLLABLE_IN_PHRASE];
	u_short pinyin_key[MAX_SYLLABLE_IN_PHRASE];
	u_int is_system;
}
ccinLongPhraseInfo_t;


//�����ʿ���ÿ�����ڶε������ִ���Ϣ
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

//�����ʿ������������漰���ִ���Ϣ
typedef struct ccinGlossaryTableInfo
{
	u_long all_system_phrase_total;
	ccinGlossarySyllableInfo_t sys_syllable_info[SYLLABLE_TOTAL];
}
ccinGlossaryTableInfo_t;


//�й�ͬ�����ڵĴ����ļ����ͷ��Ϣ����
typedef struct ccinSyllableSegmentHeadInfo
{
	u_long offset;				//�ýṹ���ļ��е�ƫ����
	u_short size;				//�ýṹ���ֽ���
}
ccinSyllableSegmentHeadInfo_t;

//�ʿ��ļ�ͷ����
typedef struct ccinGlossaryFileHead
{
	u_long phrase_total;
	ccinSyllableSegmentHeadInfo_t
		phrase_syllable_segment_head_info[SYLLABLE_TOTAL];
}
ccinGlossaryFileHead_t;

//�й�ͬ�����ڵĴ����ļ����ͷ���ͣ��ʿ��ļ���
typedef struct ccinSyllableSegmentHead
{
	u_short pinyin_key;			//ƴ����ֵ(2)
	u_short word_phrase_flag;	//�ִʱ�־λ(2)
	u_short word_phrase_total;	//�ִ�����(2)
	u_short *word_phrase_num_pointer;	//���ִ�����ָ��(?)
//no use
//�����ĸ������Ĵ�����Ŀ����Ҫ���ʣ��õ�ֵ�󸳵���ĵط���ccinGlossarySyllableInfo_t��
//  u_short *word_numbers;  //�������Ĵ�����Ŀ(�ɱ�־λ��)
}
ccinSyllableSegmentHead_t;


//���¹���ģ����
typedef struct ccinFuzzyPinYinKey
{
	char *p_pinyinkey_1;
	char *p_pinyinkey_2;
}
ccinFuzzyPinYinKey_t;

//Masks for fuzzy pinyin cause
enum ccinFuzzyCause
{								//ʵ���������涨��ĸ���ccinFuzzyPinYinKey_tһһ��Ӧ
	FUZZY_SYLLABLE_1 = 0x01,	//������ģ��
	FUZZY_FINAL_1 = 0x10,		//��ĸģ��
	FUZZY_FINAL_2 = 0x20,
	FUZZY_FINAL_3 = 0x40,
	FUZZY_INITIAL_1 = 0x100,	//��ĸģ��
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
	u_long fuzzy_cause;			//��ʾģ��ԭ�����
}
ccinFuzzyInvalidSyllable_t;


//���¹���˫ƴ
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
//0x01��ƴ��0x02������ģ����0x04��ĸģ����0x08��ĸģ����
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
	//���û�����������м����״̬�ļ�¼��
	u_char origin_pinyin_buffer[MAX_CHAR_IN_ORIGIN_PINYIN_STRING];	//50
	u_char undecomposed_pinyin_buffer[MAX_CHAR_IN_ORIGIN_PINYIN_STRING];
	ccinHanziChar_t translated_hanzi_buffer[MAX_SYLLABLE_IN_PHRASE];	//9
	u_char display_pinyin_buffer[MAX_CHAR_IN_ORIGIN_PINYIN_STRING];	//��SCIMҪ���pre_edit��

	ccinSyllable_t pinyin_syllable_buffer[MAX_SYLLABLE_IN_PHRASE];	//ƴ������������õģ��������������
	u_short current_total_pinyin_length;
	u_short current_pinyin_position;

	//���ѽ�����ƴ���������ļ��������ѡ����
	ccinLookupResult_t *lookup_result;
	u_long lookup_result_display_first;
	u_long lookup_result_display_last;

	u_long flag_fuzzy;			//�û����ó�ʼΪû��ģ������
	//������ÿ���FuzzyCause_t���飬��ͬ��λ��ʾ��ͬ��ģ�����á�
	int flag_chinese;			//��Ӣ�л�
	int flag_cn_punctuation;	//�б��
	int flag_SBC;				//ȫ��
	int flag_GBK;				//GBK֧��
	int flag_traditional;		//����
}
ccinIMContext_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
