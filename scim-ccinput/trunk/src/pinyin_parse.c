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
 * ȫ�ֱ����������û�����ָ��ʵ��ʹ�õķ�����
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
 * �������ַ����ĳ���origin_string_lenƥ���ѯ�ַ���origin_string�Ƿ��ڱ�
 * ׼ƴ�����У�Ҳ����˵����ȫ�Ǳ�׼���ڡ�
 * ����0��ʾ��ƥ�䣻1��MaxStandardSyllable��ʾ���ڱ�׼ƴ������е�����λ�á�
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
			//���޶�һ�³��ȣ��ȷ�ֹ���ֱȽϣ��ֹ��˺ܶࣨ����һ�룩����Ҫ�ıȽϡ�
			if (0 ==
				strncmp (origin_string, g_standard_syllable_table[i],
						 origin_string_len))
				return (i + 1);
	}
	return 0;
}

/*
 * �������ַ����ĳ���origin_string_lenƥ���ѯĳ�ַ���origin_string�Ƿ���ģ�����ڡ�
 * ����0��ʾ��ƥ�䣻����ֵ��ʾģ��ԭ����Ҫһ��ö�����Ͷ��壨λ�����ģ���
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
		{						//�и�ģ����־
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
 * �������������
 * ���ؽ�������������-1��ʾ�������󣬵�����Ӧ�ñ����Լ����ϴν���������䣻
 *     -2��ʾ��������������󣬵�����Ӧ��ֻȡ��ǰMAX_SYLLABLE_IN_PHRASE����
 * ע�⣺���������ĸ��Ϊ����
 * has_separator[i]Ϊ���ʾ������������i+1�����ں���û���û��Լӵĸ�������
 *     ���ڴ���Ԥ�༭����ʾ��
 */
//maybe�Ժ�Ӧ���Ǵ���imcontext�ṹ����
int
ccin_parse_pinyin (const char *origin_string,
				   u_short origin_string_len,
				   ccinSyllable_t parsed_pinyin[],
				   u_short * has_separator, u_long fuzzy_flag)
{
	int syllable_total;
	char *current_pos;

//�߽��������
	if (origin_string_len >= MAX_CHAR_IN_ORIGIN_PINYIN_STRING)
		return -1;
	if (origin_string == NULL)
		return -1;
	if (has_separator == NULL)
		return -1;
	int i;

	for (i = 0; i < origin_string_len; i++)
	{							//���봮��飬�ڴ˴���Ч�ʲ���
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
		{						/* ���������ĸ������ţ�ֻ������ */
			current_pos--;
			continue;
		}
		int length_to_compare = current_pos - origin_string;

		if (length_to_compare >= MAX_LETTER_IN_SYLLABLE)
			length_to_compare = MAX_LETTER_IN_SYLLABLE - 1;

		//�����������
		int i;

		for (i = 2; i <= length_to_compare; i++)
		{
			if (*(current_pos - i) == '\'')
			{					//����ĸ������Ŵ���ʼ
				//���ܻ��б��뾯�棬��Ϊ�ڲ��ı���ѭ������������
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
		if (length_to_compare == 1)	//������ĸ
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

//��ƥ���������ش���ģ����ֻǰ��
		strncpy (parsed_pinyin[syllable_total], pTempStart,
				 length_to_compare);
		parsed_pinyin[syllable_total][length_to_compare] = 0;
		has_separator[syllable_total] = 0;	//һ��û���û��Լӵĸ�������
		if (*current_pos == '\''
			&& current_pos != (char *) (origin_string + origin_string_len))
			has_separator[syllable_total] = 1;

		syllable_total++;
		current_pos -= length_to_compare;
	}

	if (syllable_total >= MAX_SYLLABLE_IN_PHRASE)
		return -2;				//������˵��
	return syllable_total;
}

/*
 * ˫ƴ�Ľ������ֶΡ��Ϸ��Լ���
 * �����������origin_string��
 * ���ؽ�������������-1��ʾ�������󣬵�����Ӧ�ñ����Լ����ϴν���������䣻
 *     -2��ʾ��������������󣬵�����Ӧ��ֻȡ��ǰMAX_SYLLABLE_IN_PHRASE����
 * has_separator[i]Ϊ���ʾ������������i+1�����ں���û���û��Լӵĸ�������
 *     ���ڴ���Ԥ�༭����ʾ��
 * parsed_shuangpin ������˫ƴ��ĸ������
 * parsed_quanpin ������չ����ȫƴ��ĸ������
 * ע�⣺Ϊ��ȫƴһ�£����������ĸ��Ҳ��������
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

/* �߽�������� */
	if (origin_string_len >= MAX_CHAR_IN_ORIGIN_PINYIN_STRING)
		return -1;
	if (origin_string == NULL)
		return -1;
	if (has_separator == NULL)
		return -1;
	int i;

	for (i = 0; i < origin_string_len; i++)
	{							/* ˫ƴֻ��Сд��ĸ�͡�;���Ϸ� */
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
		{						/* ���������ĸ������ţ�ֻ������ */
			current_pos--;
			continue;
		}

		if (current_pos < origin_string+2)	/* ʣ��һ����ĸ���ֶ���Ϊ�Ƿ� */
			return -1;

		/* �������������� */
		if (*(current_pos - 2) == '\'')
		{					/* ���ֱ������ĵ����ַ����ֶ���Ϊ�Ƿ� */
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

		has_separator[syllable_total] = 0;	//һ��û���û��Լӵĸ�������
		if (*current_pos == '\''
			&& current_pos != (char *) (origin_string + origin_string_len))
			has_separator[syllable_total] = 1;
		syllable_total++;
		current_pos -= 2;
	}

	if (syllable_total >= MAX_SYLLABLE_IN_PHRASE)
		return -2;				//������˵��
	return syllable_total;
}

/*
 * shuangpin to quanpin
 * in : ˫ƴ��ĸ�ַ� ˫ƴ��ĸ�ַ�
 * out : ת��Ŀ��ȫƴ������
 *    The caller should assure the buffer could contain 7 chars
 * ret : ת��Ŀ��ȫƴ���ַ���, -1=fail
 */
int
ccin_sp_to_qp (u_char sp_initial, u_char sp_final, ccinSyllable_t dest_qp, u_long fuzzy_flag)
{
	if (g_sp_config_working == NULL)	/* ˫ƴ��λӳ���δ��ֵ */
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
		 else					/* ��;�������⴦��Ϊ��� */
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
	if ('\'' != *pointer_temp)	/* ������ĸ��� */
	{
		dest_total = strlen (pointer_temp);
		strncpy (dest_qp, pointer_temp, dest_total);
		dest_qp[dest_total] = '\0';
	}

	pointer_temp = g_sp_config_working[sp_final].sp_final_key_1;
//printf("dst:%s, cat:%s\n", dest_qp, pointer_temp);

	strcat (dest_qp, pointer_temp);
	if (is_standard_pinyin (dest_qp, strlen(dest_qp)) == 0
		/* ��Ϊ˫ƴ�ġ��޳�ͻ˫��ĸ�����⣬�����Բ��Ϸ���ƴ����ģ��
		&& is_fuzzy_pinyin (dest_qp, strlen(dest_qp), fuzzy_flag) == 0*/)
	{			/* illegal sp1 */
		pointer_temp = g_sp_config_working[sp_final].sp_final_key_2;
		if (pointer_temp == NULL)
			return -1;
		dest_qp[dest_total] = '\0';
//printf("dst:%s, cat:%s\n", dest_qp, pointer_temp);
		strcat (dest_qp, pointer_temp);

		if (is_standard_pinyin (dest_qp, strlen(dest_qp)) == 0
			/* ��Ϊ˫ƴ�ġ��޳�ͻ˫��ĸ�����⣬�����Բ��Ϸ���ƴ����ģ��
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
	char origin_string[MAX_CHAR_IN_ORIGIN_PINYIN_STRING + 1];	//���ֽ��ƴ�����ַ�������
	u_short origin_string_len;	//���ֽ��ƴ�����ַ������ȡ�
	ccinSyllable_t pinyin_syllable_buffer[MAX_SYLLABLE_IN_PHRASE];

	int temp, i;

	u_long flag_fuzzy = 0;

	flag_fuzzy |= FUZZY_FINAL_1 | FUZZY_INITIAL_3;	//ģ��������
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
