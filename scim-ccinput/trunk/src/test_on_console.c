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


#pragma pack(push, 1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "imcontext.h"
#include "file_operation.h"
#include "pinyin_parse.h"
#include "glossary_lookup.h"
#include "glossary_adjust.h"
#include "ccinput.h"
#include "codeset_conv.h"

int main ()
{
	char user_input_buffer[MAX_CHAR_IN_ORIGIN_PINYIN_STRING];
	ccinIMContext_t *im_context;
	int pinyin_parse_number;
	u_short has_separator[MAX_SYLLABLE_IN_PHRASE];
	u_long flag_fuzzy = 0xFFFF;
//  flag_fuzzy = 0; //关闭所有模糊音
	int i;

	system ("clear");
	printf
		("********************************************************************************************************\n");
	printf
		("*                                        东东输入法控制台测试                                          *\n");
	printf
		("********************************************************************************************************\n");
	printf ("Test ccinput on console...\n");
	printf ("Do NOT input chars above 50!\n");

	ccin_open_imfactory ();
	im_context = ccin_initialize_context ();

	while (1)
	{
//音节解析部分始
		printf
			("------------------------------------------------------------------------------------------------------------\n");
		printf ("\t[quit] to quit.\n");
		printf ("输入拼音串:");
		scanf ("%s", user_input_buffer);
		im_context->current_total_pinyin_length = strlen (user_input_buffer);
		if (im_context->current_total_pinyin_length >=
			MAX_CHAR_IN_ORIGIN_PINYIN_STRING)
		{
			printf ("Input too long! \n");
			ccin_reset_context (im_context);	//not necessary in fact
			continue;
		}
		if (strncmp ("quit", user_input_buffer, 4) == 0)
			break;

		//需要检测是否只由小写字母和隔音符号组成
		int input_error = 0;

		for (i = 0; i < im_context->current_total_pinyin_length; i++)
		{
			if (islower (user_input_buffer[i]) == 0
				&& user_input_buffer[i] != '\'')
			{
				input_error = 1;
				break;
			}
		}
		if (1 == input_error)
		{
			printf ("Input only lower alpha or ''' please.\n");
			continue;
		}

		strncpy (im_context->origin_pinyin_buffer, user_input_buffer,
				 im_context->current_total_pinyin_length);
		pinyin_parse_number =
			ccin_parse_pinyin (im_context->origin_pinyin_buffer,
							 im_context->current_total_pinyin_length,
							 im_context->pinyin_syllable_buffer,
							 has_separator, flag_fuzzy);
		if (pinyin_parse_number == -2)
		{
			pinyin_parse_number = 9;
		}
		if (pinyin_parse_number < 1)
		{
			printf ("Illegal input pinyin!\n");
			ccin_reset_context (im_context);	//not necessary in fact
			continue;
		}
		printf
			("-------------------------------------------输出结果分析----------------------------------------------------\n");
		printf ("分解的音节总数: %d\n分解的音节: ", pinyin_parse_number);
		for (i = 1; i <= pinyin_parse_number; i++)
		{						//拼音组是逆序放置的，即首音节在最后。
			printf ("%d.%6s ", i,
					im_context->
					pinyin_syllable_buffer[pinyin_parse_number - i]);
			if (has_separator[pinyin_parse_number - i] != 0)
				printf ("'");
		}
//      printf ("\n");
//      for (i=0; i<9; i++)
//          printf ("%d; ", has_separator[i]);
		printf ("\n");
//音节解析部分终

//词库查询部分始
		i = 1;
		int j = 1;
		u_char buffer_for_display[28];	//3*9=27

		ccin_find_matching_results (im_context->pinyin_syllable_buffer,
									pinyin_parse_number,
									im_context->lookup_result,
									flag_fuzzy);
		printf ("查询的字词总数 : %d\n",
				im_context->lookup_result->lookup_total);
		if (im_context->lookup_result->lookup_above_four_num > 0)
			printf ("长字词数: %3d  ",
					im_context->lookup_result->lookup_above_four_num);
		if (im_context->lookup_result->lookup_four_word_num > 0)
			printf ("四字词数: %3d  ",
					im_context->lookup_result->lookup_four_word_num);
		if (im_context->lookup_result->lookup_three_word_num > 0)
			printf ("三字词数: %3d  ",
					im_context->lookup_result->lookup_three_word_num);
		if (im_context->lookup_result->lookup_two_word_num > 0)
			printf ("二字词数: %3d  ",
					im_context->lookup_result->lookup_two_word_num);
		if (im_context->lookup_result->lookup_word_gb_num > 0)
			printf ("GB字数: %3d  ",
					im_context->lookup_result->lookup_word_gb_num);
		if (im_context->lookup_result->lookup_word_gbk_num > 0)
			printf ("GBK字数: %3d",
					im_context->lookup_result->lookup_word_gbk_num);
		if (im_context->lookup_result->lookup_total > 0)
			printf ("\n");
		{
			printf ("长字词查询结果:\n");
			ccinLongPhraseInfoList_t *list;

			j = 1;
			for (list = im_context->lookup_result->lookup_above_four_list;
				 list != NULL; list = list->next)
			{
				memcpy (buffer_for_display,
						list->lookup_long_phrase->phrase,
						list->lookup_long_phrase->word_number *
						sizeof (ccinHanziChar_t));
				buffer_for_display[list->lookup_long_phrase->word_number *
								   sizeof (ccinHanziChar_t)] = '\0';
				printf ("%2d.%s ", i, buffer_for_display);
				i++;
				if (++j == 8)
				{
					printf ("\n");
					j = 1;
				}
			}
			printf ("\n");
		}
		{
			printf ("四字词查询结果:\n");
			ccinPhraseFourWordInfoList_t *list;

			j = 1;
			for (list = im_context->lookup_result->lookup_four_word_list;
				 list != NULL; list = list->next)
			{
				memcpy (buffer_for_display,
						list->lookup_phrase_word_four->phrase,
						4 * sizeof (ccinHanziChar_t));
				buffer_for_display[4 * sizeof (ccinHanziChar_t)] = '\0';
				printf ("%3d.%s(%3d)  ", i, buffer_for_display,
						list->lookup_phrase_word_four->freq);
				i++;
				if (++j == 8)
				{
					printf ("\n");
					j = 1;
				}
			}
			printf ("\n");
		}
		{
			printf ("三字词查询结果:\n");
			ccinPhraseThreeWordInfoList_t *list;

			j = 1;
			for (list = im_context->lookup_result->lookup_three_word_list;
				 list != NULL; list = list->next)
			{
				memcpy (buffer_for_display,
						list->lookup_phrase_word_three->phrase,
						3 * sizeof (ccinHanziChar_t));
				buffer_for_display[3 * sizeof (ccinHanziChar_t)] = '\0';
				printf ("%3d.%s(%3d)  ", i, buffer_for_display,
						list->lookup_phrase_word_three->freq);
				i++;
				if (++j == 8)
				{
					printf ("\n");
					j = 1;
				}
			}
			printf ("\n");
		}
		{
			printf ("二字词查询结果:\n");
			ccinPhraseTwoWordInfoList_t *list;

			j = 1;
			for (list = im_context->lookup_result->lookup_two_word_list;
				 list != NULL; list = list->next)
			{
				memcpy (buffer_for_display,
						list->lookup_phrase_word_two->phrase,
						2 * sizeof (ccinHanziChar_t));
				buffer_for_display[2 * sizeof (ccinHanziChar_t)] = '\0';
				printf ("%4d.%s(%3d)  ", i, buffer_for_display,
						list->lookup_phrase_word_two->freq);
				i++;
				if (++j == 7)
				{
					printf ("\n");
					j = 1;
				}
			}
			printf ("\n");
		}
		{
			printf ("GB字查询结果:\n");
			ccinGBWordInfoList_t *list;

			j = 1;
			for (list = im_context->lookup_result->lookup_word_gb_list;
				 list != NULL; list = list->next)
			{
				memcpy (buffer_for_display, list->lookup_word_gb->word,
						sizeof (ccinHanziChar_t));
				buffer_for_display[sizeof (ccinHanziChar_t)] = '\0';
				printf ("%4d.%s(%3d)  ", i, buffer_for_display,
						list->lookup_word_gb->freq);
				i++;
				if (++j == 8)
				{
					printf ("\n");
					j = 1;
				}
			}
			printf ("\n");
		}
		{
			printf ("GBK字查询结果:\n");
			ccinGBKWordInfoList_t *list;

			j = 1;
			for (list = im_context->lookup_result->lookup_word_gbk_list;
				 list != NULL; list = list->next)
			{
				memcpy (buffer_for_display, list->lookup_word_gbk->word,
						sizeof (ccinHanziChar_t));
				buffer_for_display[sizeof (ccinHanziChar_t)] = '\0';
				printf ("%4d.%s ", i, buffer_for_display);
				i++;
				if (++j == 13)
				{
					printf ("\n");
					j = 1;
				}
			}
			printf ("\n");
		}
//词库查询部分终

//频率调整部分始
		int user_select_number;

		printf
			("------------------------------------------------------------------------------------------------------------\n");
		do
		{
			printf ("Input number to select a phrase :\t");
			scanf ("%d", &user_select_number);
		}
		while (user_select_number >= i || user_select_number < 1);
		void *selected_phrase = NULL;
		ccinPhraseType_t selected_phrase_type = WORD_GBK;
		ccinGBWordInfoList_t *gb_list;
		ccinPhraseTwoWordInfoList_t *phrase_two_word_list;
		ccinPhraseThreeWordInfoList_t *phrase_three_word_list;
		ccinPhraseFourWordInfoList_t *phrase_four_word_list;

		if (user_select_number >
			im_context->lookup_result->lookup_above_four_num)
		{
			user_select_number -=
				im_context->lookup_result->lookup_above_four_num;
			if (user_select_number <=
				im_context->lookup_result->lookup_four_word_num)
			{
				phrase_four_word_list =
					im_context->lookup_result->lookup_four_word_list;
				while (--user_select_number)
					phrase_four_word_list = phrase_four_word_list->next;
//FIXME:不够安全,需要检测->next是否为NULL
				selected_phrase =
					(void *) phrase_four_word_list->lookup_phrase_word_four;
				selected_phrase_type = PHRASE_FOUR;
			}
			else
			{
				user_select_number -=
					im_context->lookup_result->lookup_four_word_num;
				if (user_select_number <=
					im_context->lookup_result->lookup_three_word_num)
				{
					phrase_three_word_list =
						im_context->lookup_result->lookup_three_word_list;
					while (--user_select_number)
						phrase_three_word_list = phrase_three_word_list->next;
					selected_phrase =
						(void *) phrase_three_word_list->
						lookup_phrase_word_three;
					selected_phrase_type = PHRASE_THREE;
				}
				else
				{
					user_select_number -=
						im_context->lookup_result->lookup_three_word_num;
					if (user_select_number <=
						im_context->lookup_result->lookup_two_word_num)
					{
						phrase_two_word_list =
							im_context->lookup_result->lookup_two_word_list;
						while (--user_select_number)
							phrase_two_word_list = phrase_two_word_list->next;
						selected_phrase =
							(void *) phrase_two_word_list->
							lookup_phrase_word_two;
						selected_phrase_type = PHRASE_TWO;
					}
					else
					{
						user_select_number -=
							im_context->lookup_result->lookup_two_word_num;
						if (user_select_number <=
							im_context->lookup_result->lookup_word_gb_num)
						{
							gb_list =
								im_context->lookup_result->
								lookup_word_gb_list;
							while (--user_select_number)
								gb_list = gb_list->next;
							selected_phrase =
								(void *) gb_list->lookup_word_gb;
							selected_phrase_type = WORD_GB;
						}
					}
				}
			}
		}
//      printf("你选择了：%s\n", selected_phrase);
		ccin_phrase_freq_adjust (selected_phrase, selected_phrase_type);
//频率调整部分终
		ccin_reset_context (im_context);
	}

	if (im_context != NULL)
		free (im_context);
	ccin_close_imfactory ();
	return 0;
}


#pragma pack(pop)
