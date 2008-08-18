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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ccinput.h"
#include "pinyin_parse.h"
#include "glossary_adjust.h"
#include "glossary_lookup.h"

//#define _TIME_DEBUG
#ifdef _TIME_DEBUG
#include <time.h>
#endif

extern const ccinSyllable_t g_standard_syllable_table[SYLLABLE_TOTAL];
extern u_short g_syllable_hash[LETTER_NUM][2];

extern ccinGlossaryTableInfo_t g_sys_global_glossary;
extern ccinGlossaryTableInfo_t g_user_global_glossary;
extern ccinFuzzyPinYinKey_t g_fuzzy_syllable[];
extern ccinFuzzyPinYinKey_t g_fuzzy_final[];
extern ccinFuzzyPinYinKey_t g_fuzzy_initial[];

extern int g_flag_is_GBK;

extern u_short g_freq_adjust_GB_word_table[LETTER_NUM];
extern u_short g_freq_adjust_two_word_table[LETTER_NUM];
extern u_short g_freq_adjust_three_word_table[LETTER_NUM];
extern u_short g_freq_adjust_four_word_table[LETTER_NUM];


void ccin_phrase_syllable_extract (ccinSyllable_t pinyin_array_to_lookup[],
								   u_short pinyin_total,
								   u_long flag_fuzzy,
								   u_short * first_syllable_pinyin_key,
								   u_short * whole_pinyin_key,
								   u_short * pinyin_key_counter,
								   u_short *
								   flag_simply_spell_and_fuzzy_initial);
void ccin_query_recursive_by_number (u_short syllable_num,
									 u_short * first_syllable_pinyin_key,
									 u_short * whole_pinyin_key,
									 u_short * pinyin_key_counter,
									 u_short *
									 flag_simply_spell_and_fuzzy_initial,
									 ccinGlossaryTableInfo_t *
									 glossary_to_lookup,
									 ccinLookupResult_t * result_phrase);
int ccin_query_one_phrase (u_short * phrase_pinyin_key,
						   u_short syllable_num,
						   u_short * whole_pinyin_key,
						   u_short * pinyin_key_counter,
						   u_short * flag_simply_spell_and_fuzzy_initial);
void ccin_insert_result_orderly_and_sum (void *phrase,
										 ccinPhraseType_t phrase_type,
										 ccinLookupResult_t * result_phrase);


//just compare two strings backwards. ONLY 0 if equal.
int
ccin_str_right_n_compare (const char *str1, const char *str2, size_t num)
{
	if ((str1 == NULL) || (str2 == NULL))
		return -1;

	u_short length1 = strlen (str1);
	u_short length2 = strlen (str2);

	if (length1 < num)
		return -1;
	if (length2 < num)
		return -1;

	char *pointer1 = (char *) str1 + length1 - num;
	char *pointer2 = (char *) str2 + length2 - num;

	return (strncmp (pointer1, pointer2, num));
}


ccinLookupResult_t *
ccin_init_lookup_result ()
{
	ccinLookupResult_t *lookup_list;

	lookup_list = (ccinLookupResult_t *) malloc (sizeof (ccinLookupResult_t));
	if (lookup_list != NULL)
		memset (lookup_list, 0, sizeof (ccinLookupResult_t));

	return lookup_list;
}

//清空。每次选择前也要做。
void
ccin_reset_lookup_result (ccinLookupResult_t * lookup_list)
{
	if (lookup_list == NULL)
		return;

	lookup_list->lookup_total = 0;
	lookup_list->lookup_word_gb_num = 0;
	lookup_list->lookup_word_gbk_num = 0;
	lookup_list->lookup_two_word_num = 0;
	lookup_list->lookup_three_word_num = 0;
	lookup_list->lookup_four_word_num = 0;
	lookup_list->lookup_above_four_num = 0;

	{
		ccinGBWordInfoList_t *p0, *p1;

		for (p0 = lookup_list->lookup_word_gb_list; p0 != NULL;)
		{
			p1 = p0;
			p0 = p0->next;
			free (p1);
		}
		lookup_list->lookup_word_gb_list = NULL;
	}

	{
		ccinGBKWordInfoList_t *p0, *p1;

		for (p0 = lookup_list->lookup_word_gbk_list; p0 != NULL;)
		{
			p1 = p0;
			p0 = p0->next;
			free (p1);
		}
		lookup_list->lookup_word_gbk_list = NULL;
	}

	{
		ccinPhraseTwoWordInfoList_t *p0, *p1;

		for (p0 = lookup_list->lookup_two_word_list; p0 != NULL;)
		{
			p1 = p0;
			p0 = p0->next;
			free (p1);
		}
		lookup_list->lookup_two_word_list = NULL;
	}

	{
		ccinPhraseThreeWordInfoList_t *p0, *p1;

		for (p0 = lookup_list->lookup_three_word_list; p0 != NULL;)
		{
			p1 = p0;
			p0 = p0->next;
			free (p1);
		}
		lookup_list->lookup_three_word_list = NULL;
	}

	{
		ccinPhraseFourWordInfoList_t *p0, *p1;

		for (p0 = lookup_list->lookup_four_word_list; p0 != NULL;)
		{
			p1 = p0;
			p0 = p0->next;
			free (p1);
		}
		lookup_list->lookup_four_word_list = NULL;
	}

	{
		ccinLongPhraseInfoList_t *p0, *p1;

		for (p0 = lookup_list->lookup_above_four_list; p0 != NULL;)
		{
			p1 = p0;
			p0 = p0->next;
			free (p1);
		}
		lookup_list->lookup_above_four_list = NULL;
	}
}



//传入：拼音组、拼音个数
//注意！传入的拼音组是逆序放置的，即首音节在最后。
void
ccin_find_matching_results (ccinSyllable_t pinyin_array_to_lookup[],
							u_short pinyin_total,
							ccinLookupResult_t * result_phrase,
							u_long flag_fuzzy)
{
	if (result_phrase == NULL)
	{
		return;
	}
	if (pinyin_total == 0)
	{
		return;
	}

	u_short first_syllable_pinyin_key[MAX_EXTRACTED_SYLLABLE];

	//MAX_EXTRACTED_SYLLABLE是一个音节最多可能（由于简拼加模糊）产生的全拼音节（l,n,r），现为70
	u_short
		whole_pinyin_key[MAX_EXTRACTED_FUZZY_SYLLABLE]
		[MAX_SYLLABLE_IN_PHRASE];
	//MAX_EXTRACTED_FUZZY_SYLLABLE是一个音节最多可能（只是由于模糊）产生的全拼或者简拼音节（huang或lang目前为6），现为8
	//首音节不需要
	u_short pinyin_key_counter[MAX_SYLLABLE_IN_PHRASE];

	//每一个音节实际对应的whole_pinyin_key的个数，可能有简拼，看下面各音节标志决定
	//当然，首音节位置记录的意义不同，是全拼展开的个数
	u_short flag_simply_spell_and_fuzzy_initial[MAX_SYLLABLE_IN_PHRASE];

	//各音节的简拼与模糊音标志，参见ccinSyllableType_t定义
	//首音节不需要

	//策略：先只做简拼匹配，然后再对实际上非简拼的某些音节做校验并加入查找结果。
	//一、拼音译码，首音节展开，再记住所有简拼标志和可能简拼模糊
	//二、嵌套递归查询：
	//  由词库一次性过滤，每个词条逐个音节判断淘汰（全简拼混合），胜者直接进结果
	//  递归只是简单由长到短。长词可考虑一次完成而进入四字，待定……

#ifdef _TIME_DEBUG
	clock_t start, end;
	double elapsed;

	start = clock ();
#endif
	//一、拼音译码，首音节展开，再记住所有简拼标志和可能简拼模糊（最多两个声母）
	ccin_phrase_syllable_extract (pinyin_array_to_lookup, pinyin_total,
								  flag_fuzzy, first_syllable_pinyin_key,
								  &whole_pinyin_key[0][0],
								  pinyin_key_counter,
								  flag_simply_spell_and_fuzzy_initial);
#if 0
	int i;

	for (i = 0; i < pinyin_key_counter[0]; i++)
		printf ("%d,%d ", i, first_syllable_pinyin_key[i]);
	printf ("\n\n");
#endif

#ifdef _TIME_DEBUG
	end = clock ();
	elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	start = end;
	printf ("py time: %f\n", elapsed);
#endif
	ccin_query_recursive_by_number (pinyin_total,
								  first_syllable_pinyin_key,
								  &whole_pinyin_key[0][0],
								  pinyin_key_counter,
								  flag_simply_spell_and_fuzzy_initial,
								  &g_sys_global_glossary, result_phrase);
	ccin_query_recursive_by_number (pinyin_total,
								  first_syllable_pinyin_key,
								  &whole_pinyin_key[0][0],
								  pinyin_key_counter,
								  flag_simply_spell_and_fuzzy_initial,
								  &g_user_global_glossary, result_phrase);
#ifdef _TIME_DEBUG
	end = clock ();
	elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	start = end;
	printf ("lk and sort time: %f\n", elapsed);
#endif
}

void
ccin_phrase_syllable_extract (ccinSyllable_t pinyin_array_to_lookup[],
							  u_short pinyin_total,
							  u_long flag_fuzzy,
							  u_short * first_syllable_pinyin_key,
							  u_short * whole_pinyin_key,
							  u_short * pinyin_key_counter,
							  u_short * flag_simply_spell_and_fuzzy_initial)
{
//接口不对外，所以不检测输入参数
	//首音节完全同上，记录在first_syllable_pinyin_key中；
	//其他音节若是简拼则只模糊不做全拼展开，模糊出的声母也记在数组whole_pinyin_key中。
	u_short i;

	for (i = 0; i < pinyin_total; i++)
	{
		//对每个音，用pinyin_key_counter[i]计数，将可能的模糊pinyin_key查询出来并赋值。
		int flag_need_scan_final_fuzzy = 0;	//韵母重复扫描已模糊生成的新音节
		u_short current_idx = pinyin_total - 1 - i;
		u_short pinyin_len = strlen (pinyin_array_to_lookup[current_idx]);
		u_short *curr_pinyin_key_buffer =
			whole_pinyin_key + i * MAX_EXTRACTED_FUZZY_SYLLABLE;
		curr_pinyin_key_buffer[0] =
			is_standard_pinyin (pinyin_array_to_lookup[current_idx],
								pinyin_len);
		u_short is_illagel_but_fuzzy = 0;

		if (curr_pinyin_key_buffer[0] == 0)
			is_illagel_but_fuzzy =
				is_fuzzy_pinyin (pinyin_array_to_lookup[current_idx],
								 pinyin_len, flag_fuzzy);
		pinyin_key_counter[i] = 1;
		flag_simply_spell_and_fuzzy_initial[i] = 0;	//以备全拼的情况。
		if ((curr_pinyin_key_buffer[0] == 0
			 && is_illagel_but_fuzzy == 0) || ((pinyin_len == 1)
											   &&
											   (pinyin_array_to_lookup
												[current_idx][0] == 'm'
												||
												pinyin_array_to_lookup
												[current_idx][0] == 'n')))
			//简拼判断。m n 是特例，也当作简拼。
		{
//          pinyin_key_counter[i] = 0;
			flag_simply_spell_and_fuzzy_initial[i] |= SYLLABLE_TYPE_SIMPLY_SPELL;	//简拼位标志
#if 0
			simply_spell_counter++;
			if (simply_spell_counter > 4)
			{					//临时措施
				printf ("\n\nHas reached 4 times simply spell!!!\n\n");
				break;
			}
#endif
		}

		//模糊处理
		//1 全拼情况：整音节模糊、声母模糊、韵母模糊。
		if (flag_simply_spell_and_fuzzy_initial[i] == 0)
		{
			if (curr_pinyin_key_buffer[0] == 0)
				//全拼但是因模糊而合法
				//把当前这个也需要替换掉，所以计数从零开始
				pinyin_key_counter[i] = 0;
			if (flag_fuzzy != 0)	//便于无模糊时快速跳过
			{
				//整音节 1对
				if (flag_fuzzy & FUZZY_SYLLABLE_1)
				{
					u_short temp_fuzzy_whole_syllable_pinyin_key1 =
						is_standard_pinyin (g_fuzzy_syllable[0].p_pinyinkey_1,
											strlen (g_fuzzy_syllable[0].
													p_pinyinkey_1));
					u_short temp_fuzzy_whole_syllable_pinyin_key2 =
						is_standard_pinyin (g_fuzzy_syllable[0].p_pinyinkey_2,
											strlen (g_fuzzy_syllable[0].
													p_pinyinkey_2));
					if (temp_fuzzy_whole_syllable_pinyin_key1 ==
						curr_pinyin_key_buffer[0])
					{
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							temp_fuzzy_whole_syllable_pinyin_key2;
						pinyin_key_counter[i]++;
					}
					else if (temp_fuzzy_whole_syllable_pinyin_key2 ==
							 curr_pinyin_key_buffer[0])
					{
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							temp_fuzzy_whole_syllable_pinyin_key1;
						pinyin_key_counter[i]++;
					}
//printf ("num: %d, key: %d", pinyin_key_counter[i], pinyin_key[i][pinyin_key_counter[i]-1]);
				}
				//声母 7对
				//当然理论上这里也需要对整音节模糊生成的新拼音重复做，但是目前实际上不需要了。
				if (flag_fuzzy & FUZZY_INITIAL_1)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[0].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[0].p_pinyinkey_2);
					if (strncmp
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_initial[0].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{			//先比较“ch”，以避免重复判断出错。
//printf ("%s, %s\n", pinyin_array_to_lookup[current_idx], g_fuzzy_initial[0].p_pinyinkey_2);
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length2;
						//temp_new_length此时是余下的输入拼音串的长度
						strncpy (temp_pinyin,
								 g_fuzzy_initial[0].p_pinyinkey_1,
								 temp_fuzzy_length1);
						strncpy (temp_pinyin + temp_fuzzy_length1,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length2, temp_new_length);
						temp_new_length += temp_fuzzy_length1;
						//temp_new_length此时是由模糊而新生成的拼音串的长度
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer[pinyin_key_counter[i]] != 0)	//生成的拼音合法
							pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_initial[0].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length1;
						//余下的输入拼音串的长度
						strncpy (temp_pinyin,
								 g_fuzzy_initial[0].p_pinyinkey_2,
								 temp_fuzzy_length2);
						strncpy (temp_pinyin + temp_fuzzy_length2,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length1, temp_new_length);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer[pinyin_key_counter[i]] != 0)	//生成的拼音合法
							pinyin_key_counter[i]++;
					}
//printf ("num: %d, key: %d\n", pinyin_key_counter[i], pinyin_key[i][pinyin_key_counter[i]-1]);
				}
				if (flag_fuzzy & FUZZY_INITIAL_2)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[1].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[1].p_pinyinkey_2);
					if (strncmp
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_initial[1].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length2;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[1].p_pinyinkey_1,
								 temp_fuzzy_length1);
						strncpy (temp_pinyin + temp_fuzzy_length1,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length2, temp_new_length);
						temp_new_length += temp_fuzzy_length1;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_initial[1].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length1;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[1].p_pinyinkey_2,
								 temp_fuzzy_length2);
						strncpy (temp_pinyin + temp_fuzzy_length2,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length1, temp_new_length);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_3)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[2].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[2].p_pinyinkey_2);
					if (strncmp
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_initial[2].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length2;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[2].p_pinyinkey_1,
								 temp_fuzzy_length1);
						strncpy (temp_pinyin + temp_fuzzy_length1,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length2, temp_new_length);
						temp_new_length += temp_fuzzy_length1;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_initial[2].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length1;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[2].p_pinyinkey_2,
								 temp_fuzzy_length2);
						strncpy (temp_pinyin + temp_fuzzy_length2,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length1, temp_new_length);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_4)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[3].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[3].p_pinyinkey_2);
					if (strncmp
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_initial[3].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length2;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[3].p_pinyinkey_1,
								 temp_fuzzy_length1);
						strncpy (temp_pinyin + temp_fuzzy_length1,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length2, temp_new_length);
						temp_new_length += temp_fuzzy_length1;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_initial[3].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length1;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[3].p_pinyinkey_2,
								 temp_fuzzy_length2);
						strncpy (temp_pinyin + temp_fuzzy_length2,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length1, temp_new_length);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_5)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[4].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[4].p_pinyinkey_2);
					if (strncmp
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_initial[4].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length2;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[4].p_pinyinkey_1,
								 temp_fuzzy_length1);
						strncpy (temp_pinyin + temp_fuzzy_length1,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length2, temp_new_length);
						temp_new_length += temp_fuzzy_length1;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_initial[4].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length1;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[4].p_pinyinkey_2,
								 temp_fuzzy_length2);
						strncpy (temp_pinyin + temp_fuzzy_length2,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length1, temp_new_length);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_6)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[5].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[5].p_pinyinkey_2);
					if (strncmp
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_initial[5].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length2;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[5].p_pinyinkey_1,
								 temp_fuzzy_length1);
						strncpy (temp_pinyin + temp_fuzzy_length1,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length2, temp_new_length);
						temp_new_length += temp_fuzzy_length1;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_initial[5].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length1;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[5].p_pinyinkey_2,
								 temp_fuzzy_length2);
						strncpy (temp_pinyin + temp_fuzzy_length2,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length1, temp_new_length);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_7)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[6].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[6].p_pinyinkey_2);
					if (strncmp
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_initial[6].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length2;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[6].p_pinyinkey_1,
								 temp_fuzzy_length1);
						strncpy (temp_pinyin + temp_fuzzy_length1,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length2, temp_new_length);
						temp_new_length += temp_fuzzy_length1;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_initial[6].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length =
							strlen (pinyin_array_to_lookup
									[current_idx]) - temp_fuzzy_length1;
						strncpy (temp_pinyin,
								 g_fuzzy_initial[6].p_pinyinkey_2,
								 temp_fuzzy_length2);
						strncpy (temp_pinyin + temp_fuzzy_length2,
								 pinyin_array_to_lookup[current_idx] +
								 temp_fuzzy_length1, temp_new_length);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
				}
				//韵母
				//这里需要对以上整音节和声母模糊生成的新拼音重复做一遍韵母模糊
				if (pinyin_key_counter[i] > 1)
					flag_need_scan_final_fuzzy = 1;
				if (flag_fuzzy & FUZZY_FINAL_1)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_final[0].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_final[0].p_pinyinkey_2);

					if (flag_need_scan_final_fuzzy != 0)
						//先对于由整音节和声母模糊生成的新拼音做，再对原拼音做。
						//顺序本无所谓，程序设计编码方便而已。
					{
						int j, temp_uplimit = pinyin_key_counter[i];

						for (j = 1; j < temp_uplimit; j++)
						{		//很可能也就一次
							temp_new_length =
								strlen (g_standard_syllable_table
										[curr_pinyin_key_buffer[j] - 1]);
							strncpy (temp_pinyin,
									 g_standard_syllable_table
									 [curr_pinyin_key_buffer[j] - 1],
									 temp_new_length);
							temp_pinyin[temp_new_length] = '\0';	//必要！
							//制作出新拼音的字符串
//printf ("new py string:%s; len:%d\n", temp_pinyin, temp_new_length);
							if (ccin_str_right_n_compare
								(temp_pinyin,
								 g_fuzzy_final[0].p_pinyinkey_2,
								 temp_fuzzy_length2) == 0)
							{
								temp_new_length -= temp_fuzzy_length2;
								//temp_new_length此时是相同的韵母之前的长度
								strncpy (temp_pinyin + temp_new_length,
										 g_fuzzy_final[0].p_pinyinkey_1,
										 temp_fuzzy_length1);
								temp_new_length += temp_fuzzy_length1;
								//temp_new_length此时是由模糊而新生成的拼音串的长度
								temp_pinyin[temp_new_length] = '\0';	//may not necessary
								curr_pinyin_key_buffer
									[pinyin_key_counter[i]] =
									is_standard_pinyin (temp_pinyin,
														temp_new_length);
								if (curr_pinyin_key_buffer
									[pinyin_key_counter[i]] != 0)
									pinyin_key_counter[i]++;
							}
							else if (ccin_str_right_n_compare
									 (temp_pinyin,
									  g_fuzzy_final[0].p_pinyinkey_1,
									  temp_fuzzy_length1) == 0)
							{
								temp_new_length -= temp_fuzzy_length1;
								strncpy (temp_pinyin + temp_new_length,
										 g_fuzzy_final[0].p_pinyinkey_2,
										 temp_fuzzy_length2);
								temp_new_length += temp_fuzzy_length2;
								temp_pinyin[temp_new_length] = '\0';	//may not necessary
								curr_pinyin_key_buffer
									[pinyin_key_counter[i]] =
									is_standard_pinyin (temp_pinyin,
														temp_new_length);
								if (curr_pinyin_key_buffer
									[pinyin_key_counter[i]] != 0)
									pinyin_key_counter[i]++;
							}
						}
					}

					temp_new_length =
						strlen (pinyin_array_to_lookup[current_idx]);
					//temp_new_length此时是输入拼音串的长度
					if (ccin_str_right_n_compare
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_final[0].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						temp_new_length -= temp_fuzzy_length2;
						//temp_new_length此时是相同的韵母之前的长度
						strncpy (temp_pinyin,
								 pinyin_array_to_lookup[current_idx],
								 temp_new_length);
						strncpy (temp_pinyin + temp_new_length,
								 g_fuzzy_final[0].p_pinyinkey_1,
								 temp_fuzzy_length1);
						temp_new_length += temp_fuzzy_length1;
						//temp_new_length此时是由模糊而新生成的拼音串的长度
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
					else if (ccin_str_right_n_compare
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_final[0].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length -= temp_fuzzy_length1;
						strncpy (temp_pinyin,
								 pinyin_array_to_lookup[current_idx],
								 temp_new_length);
						strncpy (temp_pinyin + temp_new_length,
								 g_fuzzy_final[0].p_pinyinkey_2,
								 temp_fuzzy_length2);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_FINAL_2)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_final[1].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_final[1].p_pinyinkey_2);

					if (flag_need_scan_final_fuzzy != 0)
					{
						int j, temp_uplimit = pinyin_key_counter[i];

						for (j = 1; j < temp_uplimit; j++)
						{		//很可能也就一次
							temp_new_length =
								strlen (g_standard_syllable_table
										[curr_pinyin_key_buffer[j] - 1]);
							strncpy (temp_pinyin,
									 g_standard_syllable_table
									 [curr_pinyin_key_buffer[j] - 1],
									 temp_new_length);
							temp_pinyin[temp_new_length] = '\0';	//必要！
							if (ccin_str_right_n_compare
								(temp_pinyin,
								 g_fuzzy_final[1].p_pinyinkey_2,
								 temp_fuzzy_length2) == 0)
							{
								temp_new_length -= temp_fuzzy_length2;
								strncpy (temp_pinyin + temp_new_length,
										 g_fuzzy_final[1].p_pinyinkey_1,
										 temp_fuzzy_length1);
								temp_new_length += temp_fuzzy_length1;
								temp_pinyin[temp_new_length] = '\0';	//may not necessary
								curr_pinyin_key_buffer
									[pinyin_key_counter[i]] =
									is_standard_pinyin (temp_pinyin,
														temp_new_length);
								if (curr_pinyin_key_buffer
									[pinyin_key_counter[i]] != 0)
									pinyin_key_counter[i]++;
							}
							else if (ccin_str_right_n_compare
									 (temp_pinyin,
									  g_fuzzy_final[1].p_pinyinkey_1,
									  temp_fuzzy_length1) == 0)
							{
								temp_new_length -= temp_fuzzy_length1;
								strncpy (temp_pinyin + temp_new_length,
										 g_fuzzy_final[1].p_pinyinkey_2,
										 temp_fuzzy_length2);
								temp_new_length += temp_fuzzy_length2;
								temp_pinyin[temp_new_length] = '\0';	//may not necessary
								curr_pinyin_key_buffer
									[pinyin_key_counter[i]] =
									is_standard_pinyin (temp_pinyin,
														temp_new_length);
								if (curr_pinyin_key_buffer
									[pinyin_key_counter[i]] != 0)
									pinyin_key_counter[i]++;
							}
						}
					}

					temp_new_length =
						strlen (pinyin_array_to_lookup[current_idx]);
					//temp_new_length此时是输入拼音串的长度
					if (ccin_str_right_n_compare
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_final[1].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						temp_new_length -= temp_fuzzy_length2;
						//temp_new_length此时是相同的韵母之前的长度
						strncpy (temp_pinyin,
								 pinyin_array_to_lookup[current_idx],
								 temp_new_length);
						strncpy (temp_pinyin + temp_new_length,
								 g_fuzzy_final[1].p_pinyinkey_1,
								 temp_fuzzy_length1);
						temp_new_length += temp_fuzzy_length1;
						//temp_new_length此时是由模糊而新生成的拼音串的长度
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
					else if (ccin_str_right_n_compare
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_final[1].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length -= temp_fuzzy_length1;
						strncpy (temp_pinyin,
								 pinyin_array_to_lookup[current_idx],
								 temp_new_length);
						strncpy (temp_pinyin + temp_new_length,
								 g_fuzzy_final[1].p_pinyinkey_2,
								 temp_fuzzy_length2);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_FINAL_3)
				{
					u_char temp_pinyin[MAX_LETTER_IN_SYLLABLE];
					u_short temp_fuzzy_length1, temp_fuzzy_length2;
					u_short temp_new_length;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_final[2].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_final[2].p_pinyinkey_2);

					if (flag_need_scan_final_fuzzy != 0)
					{
						int j, temp_uplimit = pinyin_key_counter[i];

						for (j = 1; j < temp_uplimit; j++)
						{		//很可能也就一次
							temp_new_length =
								strlen (g_standard_syllable_table
										[curr_pinyin_key_buffer[j] - 1]);
							strncpy (temp_pinyin,
									 g_standard_syllable_table
									 [curr_pinyin_key_buffer[j] - 1],
									 temp_new_length);
							temp_pinyin[temp_new_length] = '\0';	//必要！
							if (ccin_str_right_n_compare
								(temp_pinyin,
								 g_fuzzy_final[2].p_pinyinkey_2,
								 temp_fuzzy_length2) == 0)
							{
								temp_new_length -= temp_fuzzy_length2;
								strncpy (temp_pinyin + temp_new_length,
										 g_fuzzy_final[2].p_pinyinkey_1,
										 temp_fuzzy_length1);
								temp_new_length += temp_fuzzy_length1;
								temp_pinyin[temp_new_length] = '\0';	//may not necessary
								curr_pinyin_key_buffer
									[pinyin_key_counter[i]] =
									is_standard_pinyin (temp_pinyin,
														temp_new_length);
								if (curr_pinyin_key_buffer
									[pinyin_key_counter[i]] != 0)
									pinyin_key_counter[i]++;
							}
							else if (ccin_str_right_n_compare
									 (temp_pinyin,
									  g_fuzzy_final[2].p_pinyinkey_1,
									  temp_fuzzy_length1) == 0)
							{
								temp_new_length -= temp_fuzzy_length1;
								strncpy (temp_pinyin + temp_new_length,
										 g_fuzzy_final[2].p_pinyinkey_2,
										 temp_fuzzy_length2);
								temp_new_length += temp_fuzzy_length2;
								temp_pinyin[temp_new_length] = '\0';	//may not necessary
								curr_pinyin_key_buffer
									[pinyin_key_counter[i]] =
									is_standard_pinyin (temp_pinyin,
														temp_new_length);
								if (curr_pinyin_key_buffer
									[pinyin_key_counter[i]] != 0)
									pinyin_key_counter[i]++;
							}
						}
					}

					temp_new_length =
						strlen (pinyin_array_to_lookup[current_idx]);
					//temp_new_length此时是输入拼音串的长度
					if (ccin_str_right_n_compare
						(pinyin_array_to_lookup[current_idx],
						 g_fuzzy_final[2].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						temp_new_length -= temp_fuzzy_length2;
						//temp_new_length此时是相同的韵母之前的长度
						strncpy (temp_pinyin,
								 pinyin_array_to_lookup[current_idx],
								 temp_new_length);
						strncpy (temp_pinyin + temp_new_length,
								 g_fuzzy_final[2].p_pinyinkey_1,
								 temp_fuzzy_length1);
						temp_new_length += temp_fuzzy_length1;
						//temp_new_length此时是由模糊而新生成的拼音串的长度
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
					else if (ccin_str_right_n_compare
							 (pinyin_array_to_lookup[current_idx],
							  g_fuzzy_final[2].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						temp_new_length -= temp_fuzzy_length1;
						strncpy (temp_pinyin,
								 pinyin_array_to_lookup[current_idx],
								 temp_new_length);
						strncpy (temp_pinyin + temp_new_length,
								 g_fuzzy_final[2].p_pinyinkey_2,
								 temp_fuzzy_length2);
						temp_new_length += temp_fuzzy_length2;
						temp_pinyin[temp_new_length] = '\0';	//may not necessary
						curr_pinyin_key_buffer[pinyin_key_counter[i]] =
							is_standard_pinyin (temp_pinyin, temp_new_length);
						if (curr_pinyin_key_buffer
							[pinyin_key_counter[i]] != 0)
							pinyin_key_counter[i]++;
					}
				}
			}
		}						//全拼的模糊展开的处理结束
		else
			//简拼情况：只作声母模糊。
		{
			char initial_of_syllable[MAX_EXTRACTED_FUZZY_SYLLABLE][2] =
				{ {'\0', '\0'}, {'\0', '\0'}, {'\0', '\0'}, {'\0', '\0'},
				{'\0', '\0'}, {'\0', '\0'}, {'\0', '\0'}, {'\0', '\0'} };

			//其实不会大于3，不过以后的事情谁知道……
			pinyin_key_counter[i] = 1;
			initial_of_syllable[0][0] =
				pinyin_array_to_lookup[current_idx][0];
			if (pinyin_len > 1)
				initial_of_syllable[0][1] =
					pinyin_array_to_lookup[current_idx][1];

			if (flag_fuzzy != 0)	//便于无模糊时快速跳过
			{					//处理模糊（只有声母）
				if (flag_fuzzy & FUZZY_INITIAL_1)
				{
					u_short temp_fuzzy_length1, temp_fuzzy_length2;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[0].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[0].p_pinyinkey_2);
					if (strncmp
						(initial_of_syllable[0],
						 g_fuzzy_initial[0].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{			//先比较“ch”，以避免重复判断出错。
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[0].p_pinyinkey_1,
								 temp_fuzzy_length1);
						pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (initial_of_syllable[0],
							  g_fuzzy_initial[0].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[0].p_pinyinkey_2,
								 temp_fuzzy_length2);
						pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_2)
				{
					u_short temp_fuzzy_length1, temp_fuzzy_length2;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[1].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[1].p_pinyinkey_2);
					if (strncmp
						(initial_of_syllable[0],
						 g_fuzzy_initial[1].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[1].p_pinyinkey_1,
								 temp_fuzzy_length1);
						pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (initial_of_syllable[0],
							  g_fuzzy_initial[1].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[1].p_pinyinkey_2,
								 temp_fuzzy_length2);
						pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_3)
				{
					u_short temp_fuzzy_length1, temp_fuzzy_length2;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[2].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[2].p_pinyinkey_2);
					if (strncmp
						(initial_of_syllable[0],
						 g_fuzzy_initial[2].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[2].p_pinyinkey_1,
								 temp_fuzzy_length1);
						pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (initial_of_syllable[0],
							  g_fuzzy_initial[2].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[2].p_pinyinkey_2,
								 temp_fuzzy_length2);
						pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_4)
				{
					u_short temp_fuzzy_length1, temp_fuzzy_length2;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[3].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[3].p_pinyinkey_2);
					if (strncmp
						(initial_of_syllable[0],
						 g_fuzzy_initial[3].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[3].p_pinyinkey_1,
								 temp_fuzzy_length1);
						pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (initial_of_syllable[0],
							  g_fuzzy_initial[3].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[3].p_pinyinkey_2,
								 temp_fuzzy_length2);
						pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_5)
				{
					u_short temp_fuzzy_length1, temp_fuzzy_length2;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[4].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[4].p_pinyinkey_2);
					if (strncmp
						(initial_of_syllable[0],
						 g_fuzzy_initial[4].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[4].p_pinyinkey_1,
								 temp_fuzzy_length1);
						pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (initial_of_syllable[0],
							  g_fuzzy_initial[4].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[4].p_pinyinkey_2,
								 temp_fuzzy_length2);
						pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_6)
				{
					u_short temp_fuzzy_length1, temp_fuzzy_length2;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[5].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[5].p_pinyinkey_2);
					if (strncmp
						(initial_of_syllable[0],
						 g_fuzzy_initial[5].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[5].p_pinyinkey_1,
								 temp_fuzzy_length1);
						pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (initial_of_syllable[0],
							  g_fuzzy_initial[5].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[5].p_pinyinkey_2,
								 temp_fuzzy_length2);
						pinyin_key_counter[i]++;
					}
				}
				if (flag_fuzzy & FUZZY_INITIAL_7)
				{
					u_short temp_fuzzy_length1, temp_fuzzy_length2;

					temp_fuzzy_length1 =
						strlen (g_fuzzy_initial[6].p_pinyinkey_1);
					temp_fuzzy_length2 =
						strlen (g_fuzzy_initial[6].p_pinyinkey_2);
					if (strncmp
						(initial_of_syllable[0],
						 g_fuzzy_initial[6].p_pinyinkey_2,
						 temp_fuzzy_length2) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[6].p_pinyinkey_1,
								 temp_fuzzy_length1);
						pinyin_key_counter[i]++;
					}
					else if (strncmp
							 (initial_of_syllable[0],
							  g_fuzzy_initial[6].p_pinyinkey_1,
							  temp_fuzzy_length1) == 0)
					{
						strncpy (initial_of_syllable
								 [pinyin_key_counter[i]],
								 g_fuzzy_initial[6].p_pinyinkey_2,
								 temp_fuzzy_length2);
						pinyin_key_counter[i]++;
					}
				}
			}

			//简拼的index存入whole_pinyin_key
			u_short j = 0;

			while (initial_of_syllable[j][0] != '\0'
				   && j <= pinyin_key_counter[i])
			{
				char first_letter = initial_of_syllable[j][0];

				if ('h' == initial_of_syllable[j][1])
				{
					if ('c' == first_letter)
						first_letter += 6;
					else if ('s' == first_letter)
						first_letter += 2;
					else if ('z' == first_letter)
						first_letter -= 4;
				}
				first_letter -= 'a';
				curr_pinyin_key_buffer[j] = (u_short) first_letter;

				j++;
			}
		}						//简拼情况只作声母模糊。结束
#if 0
		int kk = 0;

		for (; kk < pinyin_key_counter[i]; kk++)
		{
			printf ("py_key[%d][%d] : %d(%s)\n", i, kk,
					curr_pinyin_key_buffer[kk],
					g_standard_syllable_table[curr_pinyin_key_buffer[kk] - 1]);
		}
#endif
	}

	//首音节的处理：拼音码展开。
	if ((flag_simply_spell_and_fuzzy_initial[0] &
		 SYLLABLE_TYPE_SIMPLY_SPELL) != 0)
	{
		//简拼处理。
		//此时有可能由于模糊处理而得到不止一个初始简拼！
		u_short first_syllable_key_counter = 0;

		for (i = 0; i < pinyin_key_counter[0]; i++)
		{
			u_char first_letter = *(whole_pinyin_key + i);
			u_short number_expand = g_syllable_hash[first_letter][1];	//简拼展开的个数
			int k = first_syllable_key_counter;

			first_syllable_key_counter += number_expand;
			first_syllable_pinyin_key[k] = g_syllable_hash[first_letter][0];
			for (; k < first_syllable_key_counter; k++)
			{
				first_syllable_pinyin_key[k + 1] =
					first_syllable_pinyin_key[k] + 1;
			}
		}
		pinyin_key_counter[0] = first_syllable_key_counter;
	}
	else						//首音节的全拼复制到first_syllable_pinyin_key
	{
		for (i = 0; i < pinyin_key_counter[0]; i++)
		{
			first_syllable_pinyin_key[i] = *(whole_pinyin_key + i);
		}
	}
}


//递归的包装，遍历词库所有待查词条
//不对外，因此不检查输入参数
void
ccin_query_recursive_by_number (u_short syllable_num,
								u_short * first_syllable_pinyin_key,
								u_short * whole_pinyin_key,
								u_short * pinyin_key_counter,
								u_short *
								flag_simply_spell_and_fuzzy_initial,
								ccinGlossaryTableInfo_t *
								glossary_to_lookup,
								ccinLookupResult_t * result_phrase)
{
	u_short i;
	ccinGBWordInfo_t *gb_list;
	ccinGBKWordInfo_t *gbk_list;
	ccinPhraseTwoWordInfo_t *phrase_two_word_list;
	ccinPhraseThreeWordInfo_t *phrase_three_word_list;
	ccinPhraseFourWordInfo_t *phrase_four_word_list;
	ccinLongPhraseInfo_t *long_phrase_list;

	switch (syllable_num)
	{
	case 9:
	case 8:
	case 7:
	case 6:
	case 5:
		for (i = 0; i < pinyin_key_counter[0]; i++)
		{
			long_phrase_list =
				glossary_to_lookup->
				sys_syllable_info[first_syllable_pinyin_key[i] -
								  1].sys_phrase_above_four;
			while (long_phrase_list != NULL)
			{
//printf("==%d, %d\n",long_phrase_list->word_number,long_phrase_list->pinyin_key[long_phrase_list->word_number-1]);
				if (long_phrase_list->word_number > syllable_num)
				{
					long_phrase_list = long_phrase_list->pos_next;
					continue;
				}
				if (ccin_query_one_phrase (long_phrase_list->pinyin_key,
										   long_phrase_list->word_number,
										   whole_pinyin_key,
										   pinyin_key_counter,
										   flag_simply_spell_and_fuzzy_initial)
					!= 0)
				{
					ccin_insert_result_orderly_and_sum (long_phrase_list,
													  PHRASE_LONG,
													  result_phrase);
				}
				long_phrase_list = long_phrase_list->pos_next;
			}
		}
		ccin_query_recursive_by_number (4, first_syllable_pinyin_key,
										whole_pinyin_key,
										pinyin_key_counter,
										flag_simply_spell_and_fuzzy_initial,
										glossary_to_lookup, result_phrase);
		break;

	case 4:
		for (i = 0; i < pinyin_key_counter[0]; i++)
		{
			phrase_four_word_list =
				glossary_to_lookup->
				sys_syllable_info[first_syllable_pinyin_key[i] -
								  1].sys_phrase_four_word;
			while (phrase_four_word_list != NULL)
			{
				if (ccin_query_one_phrase
					(phrase_four_word_list->pinyin_key, 4,
					 whole_pinyin_key, pinyin_key_counter,
					 flag_simply_spell_and_fuzzy_initial) != 0)
				{
					ccin_insert_result_orderly_and_sum
						(phrase_four_word_list, PHRASE_FOUR, result_phrase);
				}
				phrase_four_word_list = phrase_four_word_list->pos_next;
			}
		}
		ccin_query_recursive_by_number (3, first_syllable_pinyin_key,
										whole_pinyin_key,
										pinyin_key_counter,
										flag_simply_spell_and_fuzzy_initial,
										glossary_to_lookup, result_phrase);
		break;

	case 3:
		for (i = 0; i < pinyin_key_counter[0]; i++)
		{
			phrase_three_word_list =
				glossary_to_lookup->
				sys_syllable_info[first_syllable_pinyin_key[i] -
								  1].sys_phrase_three_word;
			while (phrase_three_word_list != NULL)
			{
				if (ccin_query_one_phrase
					(phrase_three_word_list->pinyin_key, 3,
					 whole_pinyin_key, pinyin_key_counter,
					 flag_simply_spell_and_fuzzy_initial) != 0)
				{
					ccin_insert_result_orderly_and_sum
						(phrase_three_word_list, PHRASE_THREE, result_phrase);
				}
				phrase_three_word_list = phrase_three_word_list->pos_next;
			}
		}
		ccin_query_recursive_by_number (2, first_syllable_pinyin_key,
										whole_pinyin_key,
										pinyin_key_counter,
										flag_simply_spell_and_fuzzy_initial,
										glossary_to_lookup, result_phrase);
		break;

	case 2:
		for (i = 0; i < pinyin_key_counter[0]; i++)
		{
			phrase_two_word_list =
				glossary_to_lookup->
				sys_syllable_info[first_syllable_pinyin_key[i] -
								  1].sys_phrase_two_word;
			while (phrase_two_word_list != NULL)
			{
				if (ccin_query_one_phrase
					(phrase_two_word_list->pinyin_key, 2,
					 whole_pinyin_key, pinyin_key_counter,
					 flag_simply_spell_and_fuzzy_initial) != 0)
				{
					ccin_insert_result_orderly_and_sum
						(phrase_two_word_list, PHRASE_TWO, result_phrase);
				}
				phrase_two_word_list = phrase_two_word_list->pos_next;
			}
		}
		ccin_query_recursive_by_number (1, first_syllable_pinyin_key,
										whole_pinyin_key,
										pinyin_key_counter,
										flag_simply_spell_and_fuzzy_initial,
										glossary_to_lookup, result_phrase);
		break;

	case 1:
		for (i = 0; i < pinyin_key_counter[0]; i++)
		{
			gb_list =
				glossary_to_lookup->
				sys_syllable_info[first_syllable_pinyin_key[i] -
								  1].sys_phrase_word_gb;
			while (gb_list != NULL)
			{
				ccin_insert_result_orderly_and_sum (gb_list, WORD_GB,
													result_phrase);
				gb_list = gb_list->pos_next;
			}
			if (g_flag_is_GBK != FALSE)
			{
				gbk_list =
					glossary_to_lookup->
					sys_syllable_info[first_syllable_pinyin_key[i] -
									  1].sys_phrase_word_gbk;
				while (gbk_list != NULL)
				{
					ccin_insert_result_orderly_and_sum (gbk_list, WORD_GBK,
														result_phrase);
					gbk_list = gbk_list->pos_next;
				}
			}
		}
		break;

	default:
		break;
	}
}

//对词库中的一个词条（由phrase_pinyin_key给出），判断是否符合本次输入要求
//不对外，因此不检查输入参数
int
ccin_query_one_phrase (u_short * phrase_pinyin_key,
					   u_short syllable_num,
					   u_short * whole_pinyin_key,
					   u_short * pinyin_key_counter,
					   u_short * flag_simply_spell_and_fuzzy_initial)
{
	u_short i, j;

	//首音节不用！ => i=1
	for (i = 1; i < syllable_num; i++)
	{
		int is_this_syllable_matching = 0;
		u_short *this_syllable_ptr =
			whole_pinyin_key + i * MAX_EXTRACTED_FUZZY_SYLLABLE;
		u_short dest_key = *(phrase_pinyin_key + i);

		if ((flag_simply_spell_and_fuzzy_initial[i] &
			 SYLLABLE_TYPE_SIMPLY_SPELL) != 0)
			//简拼。与词条的全拼比较
		{
			for (j = 0; j < pinyin_key_counter[i]; j++)
			{
				u_char idx = (u_char) * (this_syllable_ptr + j);

//printf("i=%d, j=%d, dest_key=%d, idx=%d\n", i, j, dest_key, idx);
				if (idx > LETTER_NUM)	//防错处理
					continue;
				u_short temp = dest_key - g_syllable_hash[idx][0];

				if (temp < g_syllable_hash[idx][1])	// && temp>=0)
				{				//其实肯定大于零，因为是u_short类型
					is_this_syllable_matching = 1;
					break;
				}
			}
		}
		else					//此音节为全拼
		{
			for (j = 0; j < pinyin_key_counter[i]; j++)
			{
				if (*(this_syllable_ptr + j) == dest_key)
				{
					is_this_syllable_matching = 1;
					break;
				}
			}
		}
		if (0 == is_this_syllable_matching)	//不匹配
		{
//printf("Kick 1 !!");
			return 0;
		}
	}
	return 1;
}

void
ccin_insert_result_orderly_and_sum (void *phrase,
									ccinPhraseType_t phrase_type,
									ccinLookupResult_t * result_phrase)
{
	ccinGBWordInfo_t *gb_list;
	ccinGBKWordInfo_t *gbk_list;
	ccinPhraseTwoWordInfo_t *phrase_two_word_list;
	ccinPhraseThreeWordInfo_t *phrase_three_word_list;
	ccinPhraseFourWordInfo_t *phrase_four_word_list;
	ccinLongPhraseInfo_t *long_phrase_list;

	ccinGBWordInfoList_t *result_gb_list_curr, *result_gb_list_next,
		*result_gb_list_new;
	ccinGBKWordInfoList_t *result_gbk_list_curr, *result_gbk_list_new;
	ccinPhraseTwoWordInfoList_t *result_two_word_list_curr,
		*result_two_word_list_next, *result_two_word_list_new;
	ccinPhraseThreeWordInfoList_t *result_three_word_list_curr,
		*result_three_word_list_next, *result_three_word_list_new;
	ccinPhraseFourWordInfoList_t *result_four_word_list_curr,
		*result_four_word_list_next, *result_four_word_list_new;
	ccinLongPhraseInfoList_t *result_long_phrase_list_curr,
		*result_long_phrase_list_next, *result_long_phrase_list_new;

	switch (phrase_type)
	{
	case WORD_GB:
		gb_list = (ccinGBWordInfo_t *) phrase;
		result_gb_list_new =
			(ccinGBWordInfoList_t *) malloc (sizeof (ccinGBWordInfoList_t));
		result_gb_list_new->lookup_word_gb = gb_list;
		result_gb_list_new->next = NULL;
		result_gb_list_curr = result_phrase->lookup_word_gb_list;
		if (result_gb_list_curr == NULL)	//现无
		{
			result_phrase->lookup_word_gb_list = result_gb_list_new;
		}
		else
		{
			if (ccin_gb_word_cmp
				(gb_list, result_gb_list_curr->lookup_word_gb) > 0)
			{					//比现在的表头大，则做新表头
//printf("I'm new head!\n");
				result_gb_list_new->next = result_gb_list_curr;
				result_phrase->lookup_word_gb_list = result_gb_list_new;
			}
			else				//不做头
			{
//printf("I'm NOT new head!\n");
				while (result_gb_list_curr->next != NULL)
				{
					result_gb_list_next = result_gb_list_curr->next;
					if (ccin_gb_word_cmp
						(gb_list, result_gb_list_next->lookup_word_gb) > 0)
						break;	//只跳一层，好加在……
					result_gb_list_curr = result_gb_list_next;
				}				//或者是刚比表中元素大，或者是表尾
				if (result_gb_list_curr->next == NULL)
				{				//最小，加在尾部
					result_gb_list_curr->next = result_gb_list_new;
				}
				else
				{				//大于next而小于curr，插入其间
					result_gb_list_next = result_gb_list_curr->next;
					result_gb_list_curr->next = result_gb_list_new;
					result_gb_list_new->next = result_gb_list_next;
				}
			}
		}
		result_phrase->lookup_word_gb_num++;
		result_phrase->lookup_total++;
		break;

	case WORD_GBK:
		gbk_list = (ccinGBKWordInfo_t *) phrase;
		result_gbk_list_new =
			(ccinGBKWordInfoList_t *) malloc (sizeof (ccinGBKWordInfoList_t));
		result_gbk_list_new->lookup_word_gbk = gbk_list;
		result_gbk_list_new->next = NULL;
		result_gbk_list_curr = result_phrase->lookup_word_gbk_list;
		if (result_gbk_list_curr == NULL)	//现无
		{
			result_phrase->lookup_word_gbk_list = result_gbk_list_new;
		}
		else
		{
			while (result_gbk_list_curr->next != NULL)
			{
				result_gbk_list_curr = result_gbk_list_curr->next;
			}
			//只是加在尾部
			result_gbk_list_curr->next = result_gbk_list_new;
		}
		result_phrase->lookup_word_gbk_num++;
		result_phrase->lookup_total++;
		break;

	case PHRASE_TWO:
		phrase_two_word_list = (ccinPhraseTwoWordInfo_t *) phrase;
		result_two_word_list_new =
			(ccinPhraseTwoWordInfoList_t *)
			malloc (sizeof (ccinPhraseTwoWordInfoList_t));
		result_two_word_list_new->lookup_phrase_word_two =
			phrase_two_word_list;
		result_two_word_list_new->next = NULL;
		result_two_word_list_curr = result_phrase->lookup_two_word_list;
		if (result_two_word_list_curr == NULL)	//现无
		{
			result_phrase->lookup_two_word_list = result_two_word_list_new;
		}
		else
		{
			if (ccin_two_word_phrase_cmp
				(phrase_two_word_list,
				 result_two_word_list_curr->lookup_phrase_word_two) > 0)
			{					//比现在的表头大，则做新表头
//printf("I'm new head!\n");
				result_two_word_list_new->next = result_two_word_list_curr;
				result_phrase->lookup_two_word_list =
					result_two_word_list_new;
			}
			else				//不做头
			{
//printf("I'm NOT new head!\n");
				while (result_two_word_list_curr->next != NULL)
				{
					result_two_word_list_next =
						result_two_word_list_curr->next;
					if (ccin_two_word_phrase_cmp
						(phrase_two_word_list,
						 result_two_word_list_next->
						 lookup_phrase_word_two) > 0)
						break;	//只跳一层，好加在……
					result_two_word_list_curr = result_two_word_list_next;
				}				//或者是刚比表中元素大，或者是表尾
				if (result_two_word_list_curr->next == NULL)
				{				//最小，加在尾部
					result_two_word_list_curr->next =
						result_two_word_list_new;
				}
				else
				{				//大于next而小于curr，插入其间
					result_two_word_list_next =
						result_two_word_list_curr->next;
					result_two_word_list_curr->next =
						result_two_word_list_new;
					result_two_word_list_new->next =
						result_two_word_list_next;
				}
			}
		}
		result_phrase->lookup_two_word_num++;
		result_phrase->lookup_total++;
		break;

	case PHRASE_THREE:
		phrase_three_word_list = (ccinPhraseThreeWordInfo_t *) phrase;
		result_three_word_list_new =
			(ccinPhraseThreeWordInfoList_t *)
			malloc (sizeof (ccinPhraseThreeWordInfoList_t));
		result_three_word_list_new->lookup_phrase_word_three =
			phrase_three_word_list;
		result_three_word_list_new->next = NULL;
		result_three_word_list_curr = result_phrase->lookup_three_word_list;
		if (result_three_word_list_curr == NULL)	//现无
		{
			result_phrase->lookup_three_word_list =
				result_three_word_list_new;
		}
		else
		{
			if (ccin_three_word_phrase_cmp
				(phrase_three_word_list,
				 result_three_word_list_curr->lookup_phrase_word_three) > 0)
			{					//比现在的表头大，则做新表头
				result_three_word_list_new->next =
					result_three_word_list_curr;
				result_phrase->lookup_three_word_list =
					result_three_word_list_new;
			}
			else				//不做头
			{
				while (result_three_word_list_curr->next != NULL)
				{
					result_three_word_list_next =
						result_three_word_list_curr->next;
					if (ccin_three_word_phrase_cmp
						(phrase_three_word_list,
						 result_three_word_list_next->
						 lookup_phrase_word_three) > 0)
						break;	//只跳一层，好加在……
					result_three_word_list_curr = result_three_word_list_next;
				}				//或者是刚比表中元素大，或者是表尾
				if (result_three_word_list_curr->next == NULL)
				{				//最小，加在尾部
					result_three_word_list_curr->next =
						result_three_word_list_new;
				}
				else
				{				//大于next而小于curr，插入其间
					result_three_word_list_next =
						result_three_word_list_curr->next;
					result_three_word_list_curr->next =
						result_three_word_list_new;
					result_three_word_list_new->next =
						result_three_word_list_next;
				}
			}
		}
		result_phrase->lookup_three_word_num++;
		result_phrase->lookup_total++;
		break;

	case PHRASE_FOUR:
		phrase_four_word_list = (ccinPhraseFourWordInfo_t *) phrase;
		result_four_word_list_new =
			(ccinPhraseFourWordInfoList_t *)
			malloc (sizeof (ccinPhraseFourWordInfoList_t));
		result_four_word_list_new->lookup_phrase_word_four =
			phrase_four_word_list;
		result_four_word_list_new->next = NULL;
		result_four_word_list_curr = result_phrase->lookup_four_word_list;
		if (result_four_word_list_curr == NULL)	//现无
		{
			result_phrase->lookup_four_word_list = result_four_word_list_new;
		}
		else
		{
			if (ccin_four_word_phrase_cmp
				(phrase_four_word_list,
				 result_four_word_list_curr->lookup_phrase_word_four) > 0)
			{					//比现在的表头大，则做新表头
				result_four_word_list_new->next = result_four_word_list_curr;
				result_phrase->lookup_four_word_list =
					result_four_word_list_new;
			}
			else				//不做头
			{
				while (result_four_word_list_curr->next != NULL)
				{
					result_four_word_list_next =
						result_four_word_list_curr->next;
					if (ccin_four_word_phrase_cmp
						(phrase_four_word_list,
						 result_four_word_list_next->
						 lookup_phrase_word_four) > 0)
						break;	//只跳一层，好加在……
					result_four_word_list_curr = result_four_word_list_next;
				}				//或者是刚比表中元素大，或者是表尾
				if (result_four_word_list_curr->next == NULL)
				{				//最小，加在尾部
					result_four_word_list_curr->next =
						result_four_word_list_new;
				}
				else
				{				//大于next而小于curr，插入其间
					result_four_word_list_next =
						result_four_word_list_curr->next;
					result_four_word_list_curr->next =
						result_four_word_list_new;
					result_four_word_list_new->next =
						result_four_word_list_next;
				}
			}
		}
		result_phrase->lookup_four_word_num++;
		result_phrase->lookup_total++;
		break;

	case PHRASE_LONG:
		long_phrase_list = (ccinLongPhraseInfo_t *) phrase;
		result_long_phrase_list_new =
			(ccinLongPhraseInfoList_t *)
			malloc (sizeof (ccinLongPhraseInfoList_t));
		result_long_phrase_list_new->lookup_long_phrase = long_phrase_list;
		result_long_phrase_list_new->next = NULL;
		result_long_phrase_list_curr = result_phrase->lookup_above_four_list;
		if (result_long_phrase_list_curr == NULL)	//现无
		{
			result_phrase->lookup_above_four_list =
				result_long_phrase_list_new;
		}
		else
		{
			if (ccin_long_phrase_cmp
				(long_phrase_list,
				 result_long_phrase_list_curr->lookup_long_phrase) > 0)
			{					//比现在的表头大，则做新表头
				result_long_phrase_list_new->next =
					result_long_phrase_list_curr;
				result_phrase->lookup_above_four_list =
					result_long_phrase_list_new;
			}
			else				//不做头
			{
				while (result_long_phrase_list_curr->next != NULL)
				{
					result_long_phrase_list_next =
						result_long_phrase_list_curr->next;
					if (ccin_long_phrase_cmp
						(long_phrase_list,
						 result_long_phrase_list_next->
						 lookup_long_phrase) > 0)
						break;	//只跳一层，好加在……
					result_long_phrase_list_curr =
						result_long_phrase_list_next;
				}				//或者是刚比表中元素大，或者是表尾
				if (result_long_phrase_list_curr->next == NULL)
				{				//最小，加在尾部
					result_long_phrase_list_curr->next =
						result_long_phrase_list_new;
				}
				else
				{				//大于next而小于curr，插入其间
					result_long_phrase_list_next =
						result_long_phrase_list_curr->next;
					result_long_phrase_list_curr->next =
						result_long_phrase_list_new;
					result_long_phrase_list_new->next =
						result_long_phrase_list_next;
				}
			}
		}
		result_phrase->lookup_above_four_num++;
		result_phrase->lookup_total++;
		break;

	default:
		break;
	}
}

//>0, gb_1 is bigger
int
ccin_gb_word_cmp (ccinGBWordInfo_t * gb_1, ccinGBWordInfo_t * gb_2)
{
	if (gb_1 == NULL)
	{
		if (gb_2 == NULL)
			return 0;
		else
			return -1;
	}
	else if (gb_2 == NULL)
		return 1;

	int ret_value = gb_1->freq - gb_2->freq;

	if ((0 == ret_value) && (MAX_FREQ_NUM == gb_1->freq))
	{							//按最高频之后的附加比较策略
		u_char first_letter_index =
			ccin_get_syllable_first_letter_index (gb_1->pinyin_key);
		if (first_letter_index > 0)
		{
			if (gb_1->pinyin_key ==
				g_freq_adjust_GB_word_table[first_letter_index])
				ret_value = 1;
			else if (gb_2->pinyin_key ==
					 g_freq_adjust_GB_word_table[first_letter_index])
				ret_value = -1;
		}
	}
	return (ret_value);
}

int
ccin_two_word_phrase_cmp (ccinPhraseTwoWordInfo_t * phrase1,
						  ccinPhraseTwoWordInfo_t * phrase2)
{
	if (phrase1 == NULL)
	{
		if (phrase2 == NULL)
			return 0;
		else
			return -1;
	}
	else if (phrase2 == NULL)
		return 1;

	int ret_value = phrase1->freq - phrase2->freq;

	if ((0 == ret_value) && (MAX_FREQ_NUM == phrase1->freq))
	{							//按最高频之后的附加比较策略
		u_char first_letter_index =
			ccin_get_syllable_first_letter_index (phrase1->pinyin_key[0]);
		if (first_letter_index > 0)
		{
			if (phrase1->pinyin_key[0] ==
				g_freq_adjust_two_word_table[first_letter_index])
				ret_value = 1;
			else if (phrase2->pinyin_key[0] ==
					 g_freq_adjust_two_word_table[first_letter_index])
				ret_value = -1;
		}
	}
	return (ret_value);
}

int
ccin_three_word_phrase_cmp (ccinPhraseThreeWordInfo_t * phrase1,
							ccinPhraseThreeWordInfo_t * phrase2)
{
	if (phrase1 == NULL)
	{
		if (phrase2 == NULL)
			return 0;
		else
			return -1;
	}
	else if (phrase2 == NULL)
		return 1;

	int ret_value = phrase1->freq - phrase2->freq;

	if ((0 == ret_value) && (MAX_FREQ_NUM == phrase1->freq))
	{							//按最高频之后的附加比较策略
		u_char first_letter_index =
			ccin_get_syllable_first_letter_index (phrase1->pinyin_key[0]);
		if (first_letter_index > 0)
		{
			if (phrase1->pinyin_key[0] ==
				g_freq_adjust_three_word_table[first_letter_index])
				ret_value = 1;
			else if (phrase2->pinyin_key[0] ==
					 g_freq_adjust_three_word_table[first_letter_index])
				ret_value = -1;
		}
	}
	return (ret_value);
}

int
ccin_four_word_phrase_cmp (ccinPhraseFourWordInfo_t * phrase1,
						   ccinPhraseFourWordInfo_t * phrase2)
{
	if (phrase1 == NULL)
	{
		if (phrase2 == NULL)
			return 0;
		else
			return -1;
	}
	else if (phrase2 == NULL)
		return 1;

	int ret_value = phrase1->freq - phrase2->freq;

	if ((0 == ret_value) && (MAX_FREQ_NUM == phrase1->freq))
	{							//按最高频之后的附加比较策略
		u_char first_letter_index =
			ccin_get_syllable_first_letter_index (phrase1->pinyin_key[0]);
		if (first_letter_index > 0)
		{
			if (phrase1->pinyin_key[0] ==
				g_freq_adjust_four_word_table[first_letter_index])
				ret_value = 1;
			else if (phrase2->pinyin_key[0] ==
					 g_freq_adjust_four_word_table[first_letter_index])
				ret_value = -1;
		}
	}
	return (ret_value);
}

int
ccin_long_phrase_cmp (ccinLongPhraseInfo_t * phrase1, ccinLongPhraseInfo_t * phrase2)
{
	if (phrase1 == NULL)
	{
		if (phrase2 == NULL)
			return 0;
		else
			return -1;
	}
	else if (phrase2 == NULL)
		return 1;

	return (phrase1->word_number - phrase2->word_number);
}


//检验某个给定字数、内容、拼音值的词条词库中是否已经存在
//给例如用户造词模块提供接口。
//返回0表示不存在；1表示在系统词库中；2表示在用户词库中。
int
ccin_is_phrase_existed_in_glossary (u_short word_number,
									ccinHanziChar_t * phrase, u_short * pinyin_key)
{
	if (phrase == NULL)
		return -1;
	if (pinyin_key == NULL)
		return -1;
	if (word_number < 2 || word_number > 9)
		return -1;

	ccinPhraseTwoWordInfo_t *phrase_two_word_list;
	ccinPhraseThreeWordInfo_t *phrase_three_word_list;
	ccinPhraseFourWordInfo_t *phrase_four_word_list;
	ccinLongPhraseInfo_t *long_phrase_list;

//printf ("word num: %d\n", word_number);
	switch (word_number)
	{
	case 2:
		phrase_two_word_list =
			g_sys_global_glossary.sys_syllable_info[pinyin_key[0] -
												  1].sys_phrase_two_word;
		while (phrase_two_word_list != NULL)
		{
			if (memcmp
				(phrase_two_word_list->pinyin_key, pinyin_key,
				 word_number * sizeof (u_short)) == 0)
			{
				if (memcmp
					(phrase_two_word_list->phrase, phrase,
					 word_number * sizeof (ccinHanziChar_t)) == 0)
				{
					return 1;	//system glossary bingo
				}
			}
			phrase_two_word_list = phrase_two_word_list->pos_next;
		}
		phrase_two_word_list =
			g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
												   1].sys_phrase_two_word;
		while (phrase_two_word_list != NULL)
		{
			if (memcmp
				(phrase_two_word_list->pinyin_key, pinyin_key,
				 word_number * sizeof (u_short)) == 0)
			{
				if (memcmp
					(phrase_two_word_list->phrase, phrase,
					 word_number * sizeof (ccinHanziChar_t)) == 0)
				{
					return 2;	//user glossary bingo
				}
			}
			phrase_two_word_list = phrase_two_word_list->pos_next;
		}
		break;

	case 3:
		phrase_three_word_list =
			g_sys_global_glossary.sys_syllable_info[pinyin_key[0] -
												  1].sys_phrase_three_word;
		while (phrase_three_word_list != NULL)
		{
			if (memcmp
				(phrase_three_word_list->pinyin_key, pinyin_key,
				 word_number * sizeof (u_short)) == 0)
			{
				if (memcmp
					(phrase_three_word_list->phrase, phrase,
					 word_number * sizeof (ccinHanziChar_t)) == 0)
				{
					return 1;	//system glossary bingo
				}
			}
			phrase_three_word_list = phrase_three_word_list->pos_next;
		}
		phrase_three_word_list =
			g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
												   1].sys_phrase_three_word;
		while (phrase_three_word_list != NULL)
		{
			if (memcmp
				(phrase_three_word_list->pinyin_key, pinyin_key,
				 word_number * sizeof (u_short)) == 0)
			{
				if (memcmp
					(phrase_three_word_list->phrase, phrase,
					 word_number * sizeof (ccinHanziChar_t)) == 0)
				{
					return 2;	//user glossary bingo
				}
			}
			phrase_three_word_list = phrase_three_word_list->pos_next;
		}
		break;

	case 4:
		phrase_four_word_list =
			g_sys_global_glossary.sys_syllable_info[pinyin_key[0] -
												  1].sys_phrase_four_word;
		while (phrase_four_word_list != NULL)
		{
			if (memcmp
				(phrase_four_word_list->pinyin_key, pinyin_key,
				 word_number * sizeof (u_short)) == 0)
			{
				if (memcmp
					(phrase_four_word_list->phrase, phrase,
					 word_number * sizeof (ccinHanziChar_t)) == 0)
				{
					return 1;	//system glossary bingo
				}
			}
			phrase_four_word_list = phrase_four_word_list->pos_next;
		}
		phrase_four_word_list =
			g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
												   1].sys_phrase_four_word;
		while (phrase_four_word_list != NULL)
		{
			if (memcmp
				(phrase_four_word_list->pinyin_key, pinyin_key,
				 word_number * sizeof (u_short)) == 0)
			{
				if (memcmp
					(phrase_four_word_list->phrase, phrase,
					 word_number * sizeof (ccinHanziChar_t)) == 0)
				{
					return 2;	//user glossary bingo
				}
			}
			phrase_four_word_list = phrase_four_word_list->pos_next;
		}
		break;

	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		long_phrase_list =
			g_sys_global_glossary.sys_syllable_info[pinyin_key[0] -
												  1].sys_phrase_above_four;
		while (long_phrase_list != NULL)
		{
			if (long_phrase_list->word_number == word_number)
			{
				if (memcmp
					(long_phrase_list->pinyin_key, pinyin_key,
					 word_number * sizeof (u_short)) == 0)
				{
					if (memcmp
						(long_phrase_list->phrase, phrase,
						 word_number * sizeof (ccinHanziChar_t)) == 0)
					{
						return 1;	//system glossary bingo
					}
				}
			}
			long_phrase_list = long_phrase_list->pos_next;
		}
		long_phrase_list =
			g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
												   1].sys_phrase_above_four;
		while (long_phrase_list != NULL)
		{
			if (long_phrase_list->word_number == word_number)
			{
				if (memcmp
					(long_phrase_list->pinyin_key, pinyin_key,
					 word_number * sizeof (u_short)) == 0)
				{
					if (memcmp
						(long_phrase_list->phrase, phrase,
						 word_number * sizeof (ccinHanziChar_t)) == 0)
					{
						return 2;	//user glossary bingo
					}
				}
			}
			long_phrase_list = long_phrase_list->pos_next;
		}
		break;

	default:
		break;
	}

	return 0;
}

#if 0
//
//从查找结果链表中删除一个节点。用于用户删除自造词。
//
int
delete_phrase_from_result_list (ccinPhraseType_t phrase_type,
								void *phrase_node,
								ccinLookupResult_t * result_list)
{
	if (phrase_node == NULL)
		return -1;
	if (result_list == NULL)
		return -1;
	if (0 == result_list->lookup_total)
		return -1;
	int ret_value = -1;

	ccinGBWordInfo_t *gb_list;
	ccinGBKWordInfo_t *gbk_list;
	ccinPhraseTwoWordInfo_t *phrase_two_word_list;
	ccinPhraseThreeWordInfo_t *phrase_three_word_list;
	ccinPhraseFourWordInfo_t *phrase_four_word_list;
	ccinLongPhraseInfo_t *long_phrase_list;

	ccinGBWordInfoList_t *result_gb_list_curr, *result_gb_list_next,
		*result_gb_list_new;
	ccinGBKWordInfoList_t *result_gbk_list_curr, *result_gbk_list_new;
	ccinPhraseTwoWordInfoList_t *result_two_word_list_curr,
		*result_two_word_list_next, *result_two_word_list_new;
	ccinPhraseThreeWordInfoList_t *result_three_word_list_curr,
		*result_three_word_list_next, *result_three_word_list_new;
	ccinPhraseFourWordInfoList_t *result_four_word_list_curr,
		*result_four_word_list_next, *result_four_word_list_new;
	ccinLongPhraseInfoList_t *result_long_phrase_list_curr,
		*result_long_phrase_list_next, *result_long_phrase_list_new;

	switch (phrase_type)
	{
	case WORD_GB:
		break;

	case WORD_GBK:
		break;

	case PHRASE_TWO:
		if (0 == result_list->lookup_two_word_num)
		{						//counter error, how to do?
			ret_value = -1;
			break;				//return now
		}
		phrase_two_word_list = (ccinPhraseTwoWordInfo_t *) phrase_node;
		result_two_word_list_curr = result_list->lookup_two_word_list;

		//list head
		if (result_two_word_list_curr == NULL)
		{
			ret_value = -1;
			break;				//return now
		}
		if (phrase_two_word_list ==
			result_two_word_list_curr->lookup_phrase_word_two)
		{						//match, delete this node as head.
			result_two_word_list_next = result_two_word_list_curr->next;
			free (result_two_word_list_curr);
			result_two_word_list_curr == NULL;
			result_list->lookup_two_word_list = result_two_word_list_next;
			result_list->lookup_two_word_num--;
			result_list->lookup_total--;
			ret_value = 0;
		}
		else
		{						//not list head
			while (result_two_word_list_curr->next != NULL)
			{
				result_two_word_list_next = result_two_word_list_curr->next;
				if (phrase_two_word_list ==
					result_two_word_list_next->lookup_phrase_word_two)
				{				//match, delete this node"_next".
					result_two_word_list_curr->next =
						result_two_word_list_next->next;
					free (result_two_word_list_next);
					result_two_word_list_next == NULL;
					result_list->lookup_two_word_num--;
					result_list->lookup_total--;
					ret_value = 0;
					break;
				}
				result_two_word_list_curr = result_two_word_list_next;
			}
		}
		break;

	case PHRASE_THREE:
		break;

	case PHRASE_FOUR:
		break;

	case PHRASE_LONG:
		break;

	default:
		break;
	}

	return ret_value;
}
#endif


/*
ccin_set_lookup_page_size ()
ccin_lookup_page_up ()
ccin_lookup_page_down ()
ccin_get_lookup_select ()
ccin_get_current_page ()
*/


#if 0
main ()
{

/*
	ccinHanziChar_t zz[8];
	printf ("%d\n", sizeof(zz));
*/
//验证算key

/*
	SyllableFileSegmentHead_t * uu=(SyllableFileSegmentHead_t *)malloc(sizeof(SyllableFileSegmentHead_t));
	char vv[10] = "abcdefghi";
	uu = (SyllableFileSegmentHead_t *)vv;
	printf ("%X\n", uu->pinyin_key);
	printf ("%x\n", (*uu->word_numbers));
*/
	ccin_open_imfactory ();
//  init_global_data_struction ();
//should be 10, huhu
	ccin_init_lookup_result *yy = ccin_init_lookup_result ();

//  ccinSyllable_t xx[1] = {"ha"};
//  ccin_find_matching_results (xx, 1, yy, 0x0);
//  ccinSyllable_t xx[1] = {"h"};
//  ccin_find_matching_results (xx, 1, yy, 0x0);
//  ccinSyllable_t xx[2] = {"a","bo"};
//  ccinSyllable_t xx[2] = {"wang", "cheng"}; //橙黄 称王
//  ccinSyllable_t xx[2] = {"shang", "feng"};
	ccinSyllable_t xx[2] = { "sh", "feng" };

//  ccin_find_matching_results (xx, 2, yy, 0xffff); //fuzzy c and an and wang
	ccin_find_matching_results (xx, 2, yy, 0x0);
//  ccinSyllable_t xx[4] = {"a","j","m","d"};
//  ccin_find_matching_results (xx, 4, yy);
//  printf ("%d\n", yy->lookup_two_word_list->lookup_phrase_word_two->pinyin_key[0]);
//  ccinSyllable_t xx[5] = {"a","er","b","n","y"};
//  ccin_find_matching_results (xx, 5, yy);

	u_char hanzi_string[19];

//  ccin_UTF8_to_locale_charset_for_hanzi ("GB18030", (ccinHanziChar_t *)&yy->lookup_word_gb_list->next->lookup_word_gb->word, 1, hanzi_string);
//  hanzi_string[2] = '\0';
//  ccin_UTF8_to_locale_charset_for_hanzi ("GB18030", (ccinHanziChar_t *)&yy->lookup_two_word_list->next->next->lookup_phrase_word_two->phrase, 2, hanzi_string);
//  hanzi_string[4] = '\0';
//  printf ("%s\n", hanzi_string);
//  ccin_UTF8_to_locale_charset_for_hanzi ("GB18030", (ccinHanziChar_t *)&yy->lookup_four_word_list->lookup_phrase_word_four->phrase, 4, hanzi_string);
//  hanzi_string[8] = '\0';
//  ccin_UTF8_to_locale_charset_for_hanzi ("GB18030", (ccinHanziChar_t *)&yy->lookup_above_four_list->lookup_long_phrase->phrase, 5, hanzi_string);
//  hanzi_string[10] = '\0';
//  printf ("%s\n", hanzi_string);

//  memcpy (hanzi_string, yy->lookup_above_four_list->lookup_long_phrase->phrase, 15);
//  memcpy (hanzi_string, yy->lookup_word_gb_list->lookup_word_gb->word, 3);
//  hanzi_string[16] = '\0';
//  printf ("%s\n", hanzi_string);

	ccinPhraseTwoWordInfoList_t *iii;

	printf ("GB: %d\n", yy->lookup_word_gb_num);
	printf ("GBK: %d\n", yy->lookup_word_gbk_num);
	for (iii = yy->lookup_two_word_list; iii != NULL; iii = iii->next)
	{
		memcpy (hanzi_string, iii->lookup_phrase_word_two->phrase, 6);
		hanzi_string[6] = '\0';
		printf ("%s%d key%d  ", hanzi_string,
				iii->lookup_phrase_word_two->freq,
				iii->lookup_phrase_word_two->pinyin_key[1]);
	}
	printf ("2-word: %d\n", yy->lookup_two_word_num);
	ccin_reset_lookup_result (yy);

	ccin_close_imfactory ();
	free (yy);

//  printf ("%d\n", ccin_str_right_n_compare ("dan", "an", 2));
}
#endif

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
