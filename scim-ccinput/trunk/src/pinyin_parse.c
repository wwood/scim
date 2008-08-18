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
#include <ctype.h>

#include "ccinput.h"

/* A global variable, should be moved in a new file, with other global flag
 *   such as flag_cn_punctuation, flag_GBK etc.
 * 全局变量，根据用户配置指向实际使用的方案。
 */
ccinSPMappingKey_t * g_sp_config_working;

extern ccinSyllable_t g_standard_syllable_table[SYLLABLE_TOTAL];
extern u_short g_syllable_hash[LETTER_NUM][2];
extern ccinFuzzyInvalidSyllable_t g_fuzzy_invalid_syllable_table[];

extern ccinSPMappingKey_t g_sp_config_st [];
extern ccinSPMappingKey_t g_sp_config_zr [];
extern ccinSPMappingKey_t g_sp_config_ms [];
extern ccinSPMappingKey_t g_sp_config_zg [];
extern ccinSPMappingKey_t g_sp_config_zn [];
extern ccinSPMappingKey_t g_sp_config_ls [];

int ccin_sp_to_qp (u_char sp_initial, u_char sp_final,
				   ccinSyllable_t dest_qp, u_long fuzzy_flag);

/*
 * 按给定字符串的长度origin_string_len匹配查询字符串origin_string是否在标
 * 准拼音码中，也就是说，完全是标准音节。
 * 返回0表示不匹配；1到MaxStandardSyllable表示其在标准拼音码表中的索引位置。
 */
u_short
is_standard_pinyin (const char *origin_string, u_short origin_string_len)
{
	char temp = origin_string[0];
	u_short i, uplimit;

	if (origin_string_len > 1)
	{
		if ('h' == origin_string[1])
		{
			if ('c' == temp)
				temp += 6;
			else if ('s' == temp)
				temp += 2;
			else if ('z' == temp)
				temp -= 4;
		}
	}
	i = g_syllable_hash[temp - 'a'][0] - 1;
	uplimit = i + g_syllable_hash[temp - 'a'][1];

	for (; i <= uplimit; i++)
	{
		if (strlen (g_standard_syllable_table[i]) == origin_string_len)
			//先限定一下长度，既防止部分比较，又过滤很多（至少一半）不必要的比较。
			if (0 ==
				strncmp (origin_string, g_standard_syllable_table[i],
						 origin_string_len))
				return (i + 1);
	}
	return 0;
}

/*
 * 按给定字符串的长度origin_string_len匹配查询某字符串origin_string是否是模糊音节。
 * 返回0表示不匹配；其他值表示模糊原因，需要一个枚举类型定义（位操作的？）
 */
u_short
is_fuzzy_pinyin (const char *origin_string,
				 u_short origin_string_len, u_long fuzzy_flag)
{
	u_short i;

	if (fuzzy_flag == 0)
		return 0;

	i = 0;
	while (g_fuzzy_invalid_syllable_table[i].syllable[0] != '\0')
	{
//printf("%xH, %xH\n", fuzzy_flag, g_fuzzy_invalid_syllable_table[i].fuzzy_cause);
		if (fuzzy_flag & g_fuzzy_invalid_syllable_table[i].fuzzy_cause)
		{						//有该模糊标志
			if (origin_string_len ==
				strlen (g_fuzzy_invalid_syllable_table[i].syllable))
				if (0 ==
					strncmp (origin_string,
							 g_fuzzy_invalid_syllable_table[i].syllable,
							 origin_string_len))
					return (i + 1);
		}
		i++;
	}

	return 0;
}


/*
 * 输入待解析串。
 * 返回解析的音节数；-1表示解析错误，调用者应该保持自己的上次解析结果不变；
 *     -2表示解析数量到达最大，调用者应该只取用前MAX_SYLLABLE_IN_PHRASE个。
 * 注意：解析后的字母串为逆序！
 * has_separator[i]为零表示【倒数！】第i+1个音节后面没有用户自加的隔音符。
 *     用于处理预编辑串显示。
 */
//maybe以后应该是传入imcontext结构……
int
ccin_parse_pinyin (const char *origin_string,
				   u_short origin_string_len,
				   ccinSyllable_t parsed_pinyin[],
				   u_short * has_separator, u_long fuzzy_flag)
{
	int syllable_total;
	char *current_pos;

//边界条件检查
	if (origin_string_len >= MAX_CHAR_IN_ORIGIN_PINYIN_STRING)
		return -1;
	if (origin_string == NULL)
		return -1;
	if (has_separator == NULL)
		return -1;
	int i;

	for (i = 0; i < origin_string_len; i++)
	{							//输入串检查，在此处做效率不高
		if (islower (origin_string[i]) == 0 && origin_string[i] != '\'')
		{
			return -1;
		}
	}

	syllable_total = 0;
	current_pos = (char *) (origin_string + origin_string_len);	//to last
	while ((current_pos > origin_string)
		   && (syllable_total < MAX_SYLLABLE_IN_PHRASE))
	{
		if (*(current_pos - 1) == '\'')
		{						/* 可能连续的隔音符号，只是跳过 */
			current_pos--;
			continue;
		}
		int length_to_compare = current_pos - origin_string;

		if (length_to_compare >= MAX_LETTER_IN_SYLLABLE)
			length_to_compare = MAX_LETTER_IN_SYLLABLE - 1;

		//处理隔音符号
		int i;

		for (i = 2; i <= length_to_compare; i++)
		{
			if (*(current_pos - i) == '\'')
			{					//最近的隔音符号处开始
				//可能会有编译警告，因为内部改变了循环控制条件。
				length_to_compare = i - 1;
				break;
			}
		}

		char *pTempStart = current_pos - length_to_compare;

		while (is_standard_pinyin (pTempStart, length_to_compare) == 0
			   && is_fuzzy_pinyin (pTempStart, length_to_compare,
								   fuzzy_flag) == 0 && length_to_compare > 1)
		{
			pTempStart++;
			length_to_compare--;
		}
		if (length_to_compare == 1)	//单个字母
		{
			if (*pTempStart == 'i' || *pTempStart == 'u'
				|| *pTempStart == 'v')
			{
				// ..free.. not need now
				return -1;
			}
			if (*pTempStart == 'h'
				&& (*(pTempStart - 1) == 'c'
					|| *(pTempStart - 1) == 's' || *(pTempStart - 1) == 'z'))
			{
				length_to_compare = 2;
				pTempStart--;
			}
		}

//有匹配结果，不必处理模糊，只前移
		strncpy (parsed_pinyin[syllable_total], pTempStart,
				 length_to_compare);
		parsed_pinyin[syllable_total][length_to_compare] = 0;
		has_separator[syllable_total] = 0;	//一般没有用户自加的隔音符号
		if (*current_pos == '\''
			&& current_pos != (char *) (origin_string + origin_string_len))
			has_separator[syllable_total] = 1;

		syllable_total++;
		current_pos -= length_to_compare;
	}

	if (syllable_total >= MAX_SYLLABLE_IN_PHRASE)
		return -2;				//见函数说明
	return syllable_total;
}

/*
 * 双拼的解析：分段、合法性检验
 * 输入待解析串origin_string。
 * 返回解析的音节数；-1表示解析错误，调用者应该保持自己的上次解析结果不变；
 *     -2表示解析数量到达最大，调用者应该只取用前MAX_SYLLABLE_IN_PHRASE个。
 * has_separator[i]为零表示【倒数！】第i+1个音节后面没有用户自加的隔音符。
 *     用于处理预编辑串显示。
 * parsed_shuangpin 解析的双拼字母串数组
 * parsed_quanpin 解析后展开的全拼字母串数组
 * 注意：为与全拼一致，解析后的字母串也都是逆序！
 */
int
ccin_parse_shuangpin (const char *origin_string,
					  u_short origin_string_len,
					  ccinSyllable_t parsed_shuangpin[],
					  ccinSyllable_t parsed_quanpin[],
					  u_short * has_separator, u_long fuzzy_flag)
{
	int syllable_total;
	char *current_pos;

/* 边界条件检查 */
	if (origin_string_len >= MAX_CHAR_IN_ORIGIN_PINYIN_STRING)
		return -1;
	if (origin_string == NULL)
		return -1;
	if (has_separator == NULL)
		return -1;
	int i;

	for (i = 0; i < origin_string_len; i++)
	{							/* 双拼只有小写字母和“;”合法 */
		if (islower (origin_string[i]) == 0 && origin_string[i] != ';')
		{
			return -1;
		}
	}

	syllable_total = 0;
	current_pos = (char *) (origin_string + origin_string_len);
	while ((current_pos > origin_string)
		   && (syllable_total < MAX_SYLLABLE_IN_PHRASE))
	{
		if (*(current_pos - 1) == '\'')
		{						/* 可能连续的隔音符号，只是跳过 */
			current_pos--;
			continue;
		}

		if (current_pos < origin_string+2)	/* 剩下一个字母，现定义为非法 */
			return -1;

		/* 处理隔音符号情况 */
		if (*(current_pos - 2) == '\'')
		{					/* 出现被隔开的单个字符，现定义为非法 */
			return -1;
		}

		int quanpin_len = ccin_sp_to_qp (*(current_pos - 2),
										 *(current_pos - 1),
										 parsed_quanpin[syllable_total],
										 fuzzy_flag);
		if (quanpin_len < 0)
			return -1;
		strncpy (parsed_shuangpin[syllable_total], current_pos - 2, 2);
		parsed_shuangpin[syllable_total][2] = '\0';

		has_separator[syllable_total] = 0;	//一般没有用户自加的隔音符号
		if (*current_pos == '\''
			&& current_pos != (char *) (origin_string + origin_string_len))
			has_separator[syllable_total] = 1;
		syllable_total++;
		current_pos -= 2;
	}

	if (syllable_total >= MAX_SYLLABLE_IN_PHRASE)
		return -2;				//见函数说明
	return syllable_total;
}

/*
 * shuangpin to quanpin
 * in : 双拼声母字符 双拼韵母字符
 * out : 转换目标全拼缓冲区
 *    The caller should assure the buffer could contain 7 chars
 * ret : 转换目标全拼的字符数, -1=fail
 */
int
ccin_sp_to_qp (u_char sp_initial, u_char sp_final, ccinSyllable_t dest_qp, u_long fuzzy_flag)
{
	if (g_sp_config_working == NULL)	/* 双拼键位映射表未赋值 */
		return -1;

	if ((sp_initial > 'z') || (sp_initial < 'a'))
		return -1;
	else
		sp_initial -= 'a';
	if (g_sp_config_working[sp_initial].sp_initial_key == NULL)
		return -1;

	if (sp_final > 'z')
		return -1;
	else if (sp_final < 'a')
	{
		 if (sp_final != ';')
		 	return -1;
		 else					/* “;”，特殊处理为最后 */
		 	sp_final = LETTER_NUM;
	}
	else
		sp_final -= 'a';
	if (g_sp_config_working[sp_final].sp_final_key_1 == NULL
		&& g_sp_config_working[sp_final].sp_final_key_2 == NULL)
		return -1;

	u_char *pointer_temp = g_sp_config_working[sp_initial].sp_initial_key;
	if (pointer_temp == NULL)
		return -1;
	int dest_total = 0;
	dest_qp[dest_total] = '\0';
	if ('\'' != *pointer_temp)	/* 非无声母情况 */
	{
		dest_total = strlen (pointer_temp);
		strncpy (dest_qp, pointer_temp, dest_total);
		dest_qp[dest_total] = '\0';
	}

	pointer_temp = g_sp_config_working[sp_final].sp_final_key_1;
//printf("dst:%s, cat:%s\n", dest_qp, pointer_temp);

	strcat (dest_qp, pointer_temp);
	if (is_standard_pinyin (dest_qp, strlen(dest_qp)) == 0
		/* 因为双拼的“无冲突双韵母”问题，放弃对不合法的拼音的模糊
		&& is_fuzzy_pinyin (dest_qp, strlen(dest_qp), fuzzy_flag) == 0*/)
	{			/* illegal sp1 */
		pointer_temp = g_sp_config_working[sp_final].sp_final_key_2;
		if (pointer_temp == NULL)
			return -1;
		dest_qp[dest_total] = '\0';
//printf("dst:%s, cat:%s\n", dest_qp, pointer_temp);
		strcat (dest_qp, pointer_temp);

		if (is_standard_pinyin (dest_qp, strlen(dest_qp)) == 0
			/* 因为双拼的“无冲突双韵母”问题，放弃对不合法的拼音的模糊
			&& is_fuzzy_pinyin (dest_qp, strlen(dest_qp), fuzzy_flag) == 0*/)
		{		/* Both the two combinations are illegal. */
			return -1;
		}
	}

	dest_total = strlen (dest_qp);
	return dest_total;
}

void
ccin_set_sp_config (ccinSPConfigureEnum_t config_sn)
{
	switch (config_sn)
	{
		case SP_CONFIG_ST:
			g_sp_config_working = g_sp_config_st;
			break;
		case SP_CONFIG_ZR:
			g_sp_config_working = g_sp_config_zr;
			break;
		case SP_CONFIG_MS:
			g_sp_config_working = g_sp_config_ms;
			break;
		case SP_CONFIG_ZG:
			g_sp_config_working = g_sp_config_zg;
			break;
		case SP_CONFIG_ZN:
			g_sp_config_working = g_sp_config_zn;
			break;
		case SP_CONFIG_LS:
			g_sp_config_working = g_sp_config_ls;
			break;
		default:
			break;
	}
}


#if 0
main ()
{
	char origin_string[MAX_CHAR_IN_ORIGIN_PINYIN_STRING + 1];	//待分解的拼音码字符串区域。
	u_short origin_string_len;	//待分解的拼音码字符串长度。
	ccinSyllable_t pinyin_syllable_buffer[MAX_SYLLABLE_IN_PHRASE];

	int temp, i;

	u_long flag_fuzzy = 0;

	flag_fuzzy |= FUZZY_FINAL_1 | FUZZY_INITIAL_3;	//模糊音设置
	//an-ang z-zh

	while (1)
	{
		scanf ("%s", origin_string);
		origin_string_len = strlen (origin_string);
		if (origin_string_len > MAX_CHAR_IN_ORIGIN_PINYIN_STRING)
		{
			printf ("Too long input.\n");
			continue;
		}
		else
		{
//          printf ("%s\n", origin_string);
//          printf ("%d\n", is_standard_pinyin(origin_string, origin_string_len));
			temp =
				ccin_parse_pinyin (origin_string, origin_string_len,
								 pinyin_syllable_buffer, flag_fuzzy);
			printf ("syllable total : %d\n", temp);
			if (temp > 0)
				for (i = 0; i < temp; i++)
					printf ("%s\n", pinyin_syllable_buffer[i]);
			printf ("\n");
		}
	}
}
#endif

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
