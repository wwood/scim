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
 *  Authors: SHAO CHangQing <shaochangqing@ccoss.com.cn>
 *
 */


#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

//#define DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ccinput.h"
#include "file_operation.h"
#include "glossary_adjust.h"
#include "glossary_lookup.h"


void ccin_load_system_glossary ();
void ccin_load_system_frequency ();
void ccin_load_user_glossary ();
void ccin_load_user_frequency ();
void ccin_release_system_glossary ();
void ccin_release_system_frequency ();
void ccin_release_user_glossary ();
void ccin_release_user_frequency ();
void ccin_create_freq_list ();


//全局词库
ccinGlossaryTableInfo_t g_sys_global_glossary;
ccinGlossaryTableInfo_t g_user_global_glossary;


static char *p;					//词库暂存变量,马上要释放掉
static char *pUsr;				//用户词库暂存变量,马上要释放掉
static char *pFreq;				//词频暂存变量,程序退出时释放掉
static char *pUsrFreq;			//用户词频暂存变量,程序退出时释放掉
static int sys_freq_file_size;	//记录系统词频文件长度
static int user_freq_file_size;	//记录用户词频文件长度
static int user_glossary_file_size;	//记录用户词库文件长度
static int user_load_flags = 1;	//标志用户词库加载情况,1为加载成功,0为加载失败

//系统词库文件头
static ccinGlossaryFileHead_t *system_glossary_file_head;

//用户词库文件头
static ccinGlossaryFileHead_t user_glossary_file_head;

//系统词频文件头
static ccinGlossaryFileHead_t *system_glossary_freq_file_head;

//用户词频文件头
static ccinGlossaryFileHead_t user_glossary_freq_file_head;

//用于系统词库文件和用户词库文件的音节段头结构
static ccinSyllableSegmentHead_t *system_syllable_segment_head[SYLLABLE_TOTAL];
static ccinSyllableSegmentHead_t *user_syllable_segment_head[SYLLABLE_TOTAL];
static ccinSyllableSegmentHead_t *user_frequency_segment_head[SYLLABLE_TOTAL];

//用于系统词频文件的音节段头结构
//static ccinSyllableSegmentHead_t system_syllable_freq_file_head[SYLLABLE_TOTAL];
//static ccinSyllableSegmentHead_t user_syllable_freq_file_head[SYLLABLE_TOTAL];


void
ccin_open_imfactory ()
{
#if 0
	u_short key[2];
	u_short key1[3];
	u_short key2[4];
	u_short key3[5];
	u_short key4[8];

	key[0] = 1;
	key[1] = 10;

	key1[0] = 226;
	key1[1] = 309;
	key1[2] = 268;

	key2[0] = 348;
	key2[1] = 255;
	key2[2] = 364;
	key2[3] = 364;

	key3[0] = 404;
	key3[1] = 111;
	key3[2] = 273;
	key3[3] = 204;
	key3[4] = 136;
	key3[5] = 86;
	key3[6] = 144;

	key4[0] = 404;
	key4[1] = 111;
	key4[2] = 273;
	key4[3] = 204;
	key4[4] = 136;
	key4[5] = 86;
	key4[6] = 144;
	key4[7] = 144;
#endif

	ccin_load_system_glossary ();
	ccin_load_system_frequency ();
	ccin_load_user_glossary ();
	ccin_load_user_frequency ();
	ccin_create_freq_list ();

#if 0
	ccin_add_user_phrase (3, "您是群您 ", key1);	//啊里里
#endif
};

int
parse_flags (ccinSyllableSegmentHead_t * phrasetable)
{
	int j, count;

	count = 0;

	for (j = 0; j < 16; j++)
	{
		if (phrasetable)
			if ((phrasetable->word_phrase_flag >> j) & 0x01)
			{
				count = count + 1;
			}
	}
	return count;
};

void
create_freqfilehead (FILE * fp)
{
	int i = 0, count = 0;
	int sysfreq_len = 0;		//系统词库信息头长度
	int cur_phoffset = 0;		//当前正在处理的拼音码的在系统词库中的偏移量

	sysfreq_len = sizeof (ccinGlossaryFileHead_t);
	cur_phoffset = sysfreq_len;
	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		count = parse_flags (user_frequency_segment_head[i]);
		user_glossary_freq_file_head.phrase_syllable_segment_head_info[i].size = sizeof (u_short) * (3 + count);	//音节头长度

		if (i == 0)
		{
			user_glossary_freq_file_head.
				phrase_syllable_segment_head_info[i].offset = cur_phoffset;
			continue;
		}

		cur_phoffset +=
			(g_user_global_glossary.sys_syllable_info[i - 1].
			 phrase_two_word_num +
			 g_user_global_glossary.sys_syllable_info[i -
													1].
			 phrase_three_word_num +
			 g_user_global_glossary.sys_syllable_info[i -
													1].
			 phrase_four_word_num) * sizeof (u_char) +
			user_glossary_freq_file_head.
			phrase_syllable_segment_head_info[i - 1].size;

		user_glossary_freq_file_head.phrase_syllable_segment_head_info[i].
			offset = cur_phoffset;
	}
	i = SYLLABLE_TOTAL - 1;		//计算系统词频文件的字节长度
	user_freq_file_size =
		user_glossary_freq_file_head.phrase_syllable_segment_head_info[i].
		offset +
		user_glossary_freq_file_head.phrase_syllable_segment_head_info[i].
		size +
		(g_user_global_glossary.sys_syllable_info[i].phrase_two_word_num +
		 g_user_global_glossary.sys_syllable_info[i].phrase_three_word_num +
		 g_user_global_glossary.sys_syllable_info[i].phrase_four_word_num) *
		sizeof (u_char);

	fwrite (&user_glossary_freq_file_head, sizeof (ccinGlossaryFileHead_t), 1,
			fp);
}

void
create_UsrFrequencyFileSegmentHead ()
{

	int i;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{

		if (!user_frequency_segment_head[i])
			user_frequency_segment_head[i] =
				(ccinSyllableSegmentHead_t *)
				malloc (sizeof (ccinSyllableSegmentHead_t));

		user_frequency_segment_head[i]->word_phrase_total = 0;
		user_frequency_segment_head[i]->word_phrase_flag = 0;

		if (g_user_global_glossary.sys_syllable_info[i].
			phrase_two_word_num != 0)
		{
			user_frequency_segment_head[i]->word_phrase_flag |= 0x01 << 2;
			user_frequency_segment_head[i]->word_phrase_total +=
				g_user_global_glossary.sys_syllable_info[i].
				phrase_two_word_num;
		}
		if (g_user_global_glossary.sys_syllable_info[i].
			phrase_three_word_num != 0)
		{
			user_frequency_segment_head[i]->word_phrase_flag |= 0x01 << 3;
			user_frequency_segment_head[i]->word_phrase_total +=
				g_user_global_glossary.sys_syllable_info[i].
				phrase_three_word_num;
		}
		if (g_user_global_glossary.sys_syllable_info[i].
			phrase_four_word_num != 0)
		{
			user_frequency_segment_head[i]->word_phrase_flag |= 0x01 << 4;
			user_frequency_segment_head[i]->word_phrase_total +=
				g_user_global_glossary.sys_syllable_info[i].
				phrase_four_word_num;
		}
	}
}

void
save_user_FrequencyFileSegmentHead (FILE * fp, int i)
{
	fwrite (&user_frequency_segment_head[i]->pinyin_key, sizeof (u_short),
			1, fp);
	fwrite (&user_frequency_segment_head[i]->word_phrase_flag,
			sizeof (u_short), 1, fp);
	fwrite (&user_frequency_segment_head[i]->word_phrase_total,
			sizeof (u_short), 1, fp);

	if (g_user_global_glossary.sys_syllable_info[i].phrase_two_word_num != 0)
		fwrite (&g_user_global_glossary.sys_syllable_info[i].
				phrase_two_word_num, sizeof (u_short), 1, fp);

	if (g_user_global_glossary.sys_syllable_info[i].phrase_three_word_num !=
		0)
		fwrite (&g_user_global_glossary.sys_syllable_info[i].
				phrase_three_word_num, sizeof (u_short), 1, fp);

	if (g_user_global_glossary.sys_syllable_info[i].phrase_four_word_num !=
		0)
		fwrite (&g_user_global_glossary.sys_syllable_info[i].
				phrase_four_word_num, sizeof (u_short), 1, fp);
}

void
write_UsrSyllableFileSegmentHead (FILE * fp, int i)
{

	int j;
	int above_four[10];
	ccinLongPhraseInfo_t *tmp;

	for (j = 0; j < 10; j++)
		above_four[j] = 0;

	tmp = g_user_global_glossary.sys_syllable_info[i].sys_phrase_above_four;

	user_syllable_segment_head[i]->pinyin_key = i + 1;

	while (tmp)
	{
		if (tmp->word_number != 0)
			user_syllable_segment_head[i]->word_phrase_flag |=
				0x01 << tmp->word_number;
		above_four[tmp->word_number - 5]++;
		tmp = tmp->pos_next;
	}

	fwrite (&user_syllable_segment_head[i]->pinyin_key, sizeof (u_short),
			1, fp);
	fwrite (&user_syllable_segment_head[i]->word_phrase_flag,
			sizeof (u_short), 1, fp);
	fwrite (&user_syllable_segment_head[i]->word_phrase_total,
			sizeof (u_short), 1, fp);

	if (g_user_global_glossary.sys_syllable_info[i].phrase_two_word_num != 0)
		fwrite (&g_user_global_glossary.sys_syllable_info[i].
				phrase_two_word_num, sizeof (u_short), 1, fp);

	if (g_user_global_glossary.sys_syllable_info[i].phrase_three_word_num !=
		0)
		fwrite (&g_user_global_glossary.sys_syllable_info[i].
				phrase_three_word_num, sizeof (u_short), 1, fp);

	if (g_user_global_glossary.sys_syllable_info[i].phrase_four_word_num !=
		0)
		fwrite (&g_user_global_glossary.sys_syllable_info[i].
				phrase_four_word_num, sizeof (u_short), 1, fp);

	for (j = 0; j < 10; j++)
		if (above_four[j] != 0)
		{
			fwrite (&above_four[j], sizeof (u_short), 1, fp);
		}
}

void
create_UsrSyllableFileSegmentHead ()
{

	int i, j;
	int above_four[10];
	ccinLongPhraseInfo_t *tmp;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		for (j = 0; j < 10; j++)
			above_four[j] = 0;

		if (!user_syllable_segment_head[i])
		{
			user_syllable_segment_head[i] =
				(ccinSyllableSegmentHead_t *)
				malloc (sizeof (ccinSyllableSegmentHead_t));
			bzero (user_syllable_segment_head[i],
				   sizeof (ccinSyllableSegmentHead_t));
		}

		user_syllable_segment_head[i]->word_phrase_total = 0;
		user_syllable_segment_head[i]->word_phrase_flag = 0;

		if (g_user_global_glossary.sys_syllable_info[i].
			phrase_two_word_num != 0)
		{
			user_syllable_segment_head[i]->word_phrase_flag |= 0x01 << 2;
			user_syllable_segment_head[i]->word_phrase_total +=
				g_user_global_glossary.sys_syllable_info[i].
				phrase_two_word_num;
		}
		if (g_user_global_glossary.sys_syllable_info[i].
			phrase_three_word_num != 0)
		{
			user_syllable_segment_head[i]->word_phrase_flag |= 0x01 << 3;
			user_syllable_segment_head[i]->word_phrase_total +=
				g_user_global_glossary.sys_syllable_info[i].
				phrase_three_word_num;
		}
		if (g_user_global_glossary.sys_syllable_info[i].
			phrase_four_word_num != 0)
		{
			user_syllable_segment_head[i]->word_phrase_flag |= 0x01 << 4;
			user_syllable_segment_head[i]->word_phrase_total +=
				g_user_global_glossary.sys_syllable_info[i].
				phrase_four_word_num;
		}

		tmp =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_above_four;

		while (tmp)
		{
			if (tmp->word_number != 0)
				user_syllable_segment_head[i]->word_phrase_flag |=
					0x01 << tmp->word_number;
			above_four[tmp->word_number - 5]++;
			tmp = tmp->pos_next;
		}

		for (j = 0; j < 10; j++)
			user_syllable_segment_head[i]->word_phrase_total +=
				above_four[j];
	}
}

void
create_usrfilehead (FILE * fp)
{
	int i = 0, count = 0;
	int syshead_len = 0;		//系统词库信息头长度
	int cur_phoffset = 0;		//当前正在处理的拼音码的在系统词库中的偏移量
	ccinLongPhraseInfo_t *tmp;

	syshead_len = sizeof (ccinGlossaryFileHead_t);
	cur_phoffset = syshead_len;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		user_glossary_file_head.phrase_syllable_segment_head_info[i].size =
			0;
		user_glossary_file_head.phrase_syllable_segment_head_info[i].
			offset = 0;
	}

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		count = parse_flags (user_syllable_segment_head[i]);
		user_glossary_file_head.phrase_syllable_segment_head_info[i].size = sizeof (u_short) * (3 + count);	//计算每个拼音码头长度
		if (i == 0)
		{
			user_glossary_file_head.phrase_syllable_segment_head_info[i].
				offset = cur_phoffset;
			continue;
		}

		cur_phoffset +=
			g_user_global_glossary.sys_syllable_info[i -
												   1].phrase_two_word_num *
			(sizeof (char) * sizeof (ccinHanziChar_t) * 2 +
			 sizeof (u_short)) + g_user_global_glossary.sys_syllable_info[i -
																		1].
			phrase_three_word_num * (sizeof (char) *
									 sizeof (ccinHanziChar_t) * 3 +
									 sizeof (u_short) * 2) +
			g_user_global_glossary.sys_syllable_info[i -
												   1].
			phrase_four_word_num * (sizeof (char) *
									sizeof (ccinHanziChar_t) * 4 +
									sizeof (u_short) * 3);

		tmp =
			g_user_global_glossary.sys_syllable_info[i -
												   1].
			sys_phrase_above_four;

		while (tmp)
		{
			cur_phoffset +=
				sizeof (char) * sizeof (ccinHanziChar_t) *
				(tmp->word_number) + sizeof (u_short) * (tmp->word_number -
														 1);
			tmp = tmp->pos_next;
		}

		cur_phoffset +=
			user_glossary_file_head.phrase_syllable_segment_head_info[i -
																	  1].
			size;
		user_glossary_file_head.phrase_syllable_segment_head_info[i].
			offset = cur_phoffset;
	}

	i = SYLLABLE_TOTAL - 1;
	user_glossary_file_size =
		user_glossary_file_head.phrase_syllable_segment_head_info[i].
		offset +
		user_glossary_file_head.phrase_syllable_segment_head_info[i].size +
		g_user_global_glossary.sys_syllable_info[i].phrase_two_word_num *
		(sizeof (ccinHanziChar_t) * 2 + sizeof (u_short)) +
		g_user_global_glossary.sys_syllable_info[i].phrase_three_word_num *
		(sizeof (char) * sizeof (ccinHanziChar_t) * 3 +
		 sizeof (u_short) * 2) +
		g_user_global_glossary.sys_syllable_info[i].phrase_four_word_num *
		(sizeof (char) * sizeof (ccinHanziChar_t) * 4 +
		 sizeof (u_short) * 3);

	tmp = g_user_global_glossary.sys_syllable_info[i].sys_phrase_above_four;

	while (tmp)
	{
		user_glossary_file_size +=
			sizeof (char) * sizeof (ccinHanziChar_t) * (tmp->word_number) +
			sizeof (u_short) * (tmp->word_number - 1);
		tmp = tmp->pos_next;
	}

	fwrite (&user_glossary_file_head, sizeof (ccinGlossaryFileHead_t), 1, fp);
}

void
create_gb_list (ccinGBWordInfo_t * head, ccinGBWordInfo_t * list)
{
	ccinGBWordInfo_t *tmp1;

	if (head == NULL)
		return;
	if (head == list)
		return;

	tmp1 = head;

	while (tmp1->pos_next)
	{
		tmp1 = tmp1->pos_next;
	}
	tmp1->pos_next = list;
};

void
create_gbk_list (ccinGBKWordInfo_t * head, ccinGBKWordInfo_t * list)
{
	ccinGBKWordInfo_t *tmp1;

	if (head == NULL)
		return;
	if (head == list)
		return;

	tmp1 = head;

	while (tmp1->pos_next)
	{
		tmp1 = tmp1->pos_next;
	}
	tmp1->pos_next = list;
};


int
del_word_from_two_word_list (ccinPhraseTwoWordInfo_t ** head,
							 ccinPhraseTwoWordInfo_t * list)
{
	ccinPhraseTwoWordInfo_t *tmp;
	ccinPhraseTwoWordInfo_t *tmp1;
	int ret = -1;

	if (*head == NULL)
		return -1;

	if (*head == list)
	{
		*head = list->pos_next;
		return 0;
	}

	tmp = *head;
	tmp1 = tmp->pos_next;

	while (tmp)
	{
		if (tmp == list)
		{
			tmp1->pos_next = list->pos_next;
			ret = 0;
			break;
		}
		tmp1 = tmp;
		tmp = tmp->pos_next;
	}
	return ret;
};

int
del_word_from_three_word_list (ccinPhraseThreeWordInfo_t ** head,
							   ccinPhraseThreeWordInfo_t * list)
{
	ccinPhraseThreeWordInfo_t *tmp;
	ccinPhraseThreeWordInfo_t *tmp1;
	int ret = -1;

	if (*head == NULL)
		return -1;

	if (*head == list)
	{
		*head = list->pos_next;
		return 0;
	}

	tmp = *head;
	tmp1 = tmp;

	while (tmp)
	{
		if (tmp == list)
		{
			tmp1->pos_next = list->pos_next;
			ret = 0;
			break;
		}
		tmp1 = tmp;
		tmp = tmp->pos_next;
	}
	return ret;
};

int
del_word_from_four_word_list (ccinPhraseFourWordInfo_t ** head,
							  ccinPhraseFourWordInfo_t * list)
{
	ccinPhraseFourWordInfo_t *tmp;
	ccinPhraseFourWordInfo_t *tmp1;
	int ret = -1;

	if (*head == NULL)
		return -1;
	if (*head == list)
	{
		*head = list->pos_next;
		return 0;
	}

	tmp = *head;
	tmp1 = tmp;

	while (tmp)
	{
		if (tmp == list)
		{
			tmp1->pos_next = list->pos_next;
			ret = 0;
			break;
		}
		tmp1 = tmp;
		tmp = tmp->pos_next;
	}
	return ret;
};

int
del_word_from_above_word_list (ccinLongPhraseInfo_t ** head,
							   ccinLongPhraseInfo_t * list)
{
	ccinLongPhraseInfo_t *tmp;
	ccinLongPhraseInfo_t *tmp1;
	int ret = -1;

	if (*head == NULL)
		return -1;
	if (*head == list)
	{
		*head = list->pos_next;
		return 0;
	}

	tmp = *head;
	tmp1 = tmp;

	while (tmp)
	{
		if (tmp == list)
		{
			tmp1->pos_next = list->pos_next;
			ret = 0;
			break;
		}
		tmp1 = tmp;
		tmp = tmp->pos_next;
	}
	return ret;
};

void
create_two_word_list (ccinPhraseTwoWordInfo_t * head,
					  ccinPhraseTwoWordInfo_t * list)
{
	ccinPhraseTwoWordInfo_t *tmp;

	if (head == NULL)
		return;
	if (head == list)
		return;

	tmp = head;

	while (tmp)
	{
		if (!tmp->pos_next)
			break;
		tmp = tmp->pos_next;
	}
	tmp->pos_next = list;
};

void
create_three_word_list (ccinPhraseThreeWordInfo_t * head,
						ccinPhraseThreeWordInfo_t * list)
{
	ccinPhraseThreeWordInfo_t *tmp;

	if (head == NULL)
		return;
	if (head == list)
		return;

	tmp = head;

	while (tmp)
	{
		if (!tmp->pos_next)
			break;
		tmp = tmp->pos_next;
	}
	tmp->pos_next = list;
};

void
create_four_word_list (ccinPhraseFourWordInfo_t * head,
					   ccinPhraseFourWordInfo_t * list)
{
	ccinPhraseFourWordInfo_t *tmp;

	if (head == NULL)
		return;
	if (head == list)
		return;

	tmp = head;

	while (tmp)
	{
		if (!tmp->pos_next)
			break;
		tmp = tmp->pos_next;
	}
	tmp->pos_next = list;
};

void
create_above_four_word_list (ccinLongPhraseInfo_t * head,
							 ccinLongPhraseInfo_t * list)
{
	ccinLongPhraseInfo_t *tmp;

	if (head == NULL)
		return;
	if (head == list)
		return;

	tmp = head;

	while (tmp)
	{
		if (!tmp->pos_next)
			break;
		tmp = tmp->pos_next;
	}
	tmp->pos_next = list;
};

void
create_freq_gb_list (ccinGBWordInfo_t * node)
{
	ccinGBWordInfo_t *head;
	ccinGBWordInfo_t *tmp;
	ccinGBWordInfo_t *node_next;
	ccinGBWordInfo_t *cur_node;

	head = node;
	tmp = head;
	//找出最大词频和最小词频
	while (tmp)
	{
		if (tmp->freq > head->freq)
		{
			head = tmp;
		}
		tmp = tmp->pos_next;
	}

	tmp = node;
	while (tmp != NULL)
	{
		cur_node = head;
		while (cur_node != NULL)
		{
			if (head == tmp)
				break;

			node_next = cur_node->freq_next;

			if (!node_next)
			{
				if (cur_node->freq > tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_prev = cur_node;
					break;
				}
				else
				{
					tmp->freq_next = cur_node;
					cur_node->freq_prev = tmp;
					break;
				}
			}
			else
			{
				if (node_next->freq < tmp->freq &&
					cur_node->freq > tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_next = node_next;
					tmp->freq_prev = cur_node;
					node_next->freq_prev = tmp;
					break;
				}
			}
			cur_node = cur_node->freq_next;
		}
		tmp = tmp->pos_next;
	}
}

void
del_two_word_freq_list (ccinPhraseTwoWordInfo_t * node)
{
	ccinPhraseTwoWordInfo_t *node_prev;
	ccinPhraseTwoWordInfo_t *node_next;

	node_prev = node->freq_prev;
	node_next = node->freq_next;
	if (node_prev)
		node_prev->freq_next = node_next;
	if (node_next)
		node_next->freq_prev = node_prev;

}

void
del_three_word_freq_list (ccinPhraseThreeWordInfo_t * node)
{
	ccinPhraseThreeWordInfo_t *node_prev;
	ccinPhraseThreeWordInfo_t *node_next;

	node_prev = node->freq_prev;
	node_next = node->freq_next;
	if (node_prev)
		node_prev->freq_next = node_next;
	if (node_next)
		node_next->freq_prev = node_prev;

}

void
del_four_word_freq_list (ccinPhraseFourWordInfo_t * node)
{
	ccinPhraseFourWordInfo_t *node_prev;
	ccinPhraseFourWordInfo_t *node_next;

	node_prev = node->freq_prev;
	node_next = node->freq_next;
	if (node_prev)
		node_prev->freq_next = node_next;
	if (node_next)
		node_next->freq_prev = node_prev;

}

void
insert_two_word_freq_list (ccinPhraseTwoWordInfo_t * node,
						   ccinPhraseTwoWordInfo_t * list)
{
	ccinPhraseTwoWordInfo_t *tmp;

	if (node == list)
		return;

	tmp = node;
	while (tmp)
	{
		if (!tmp->freq_next)
		{
			break;
		}
		tmp = tmp->freq_next;
	}
	tmp->freq_next = list;
	list->freq_next = NULL;
	list->freq_prev = tmp;
}

void
insert_three_word_freq_list (ccinPhraseThreeWordInfo_t * node,
							 ccinPhraseThreeWordInfo_t * list)
{
	ccinPhraseThreeWordInfo_t *tmp;

	if (node == list)
		return;

	tmp = node;
	while (tmp)
	{
		if (!tmp->freq_next)
		{
			break;
		}
		tmp = tmp->freq_next;
	}
	tmp->freq_next = list;
	list->freq_next = NULL;
	list->freq_prev = tmp;
}

void
insert_four_word_freq_list (ccinPhraseFourWordInfo_t * node,
							ccinPhraseFourWordInfo_t * list)
{
	ccinPhraseFourWordInfo_t *tmp;

	if (node == list)
		return;

	tmp = node;
	while (tmp)
	{
		if (!tmp->freq_next)
		{
			break;
		}
		tmp = tmp->freq_next;
	}
	tmp->freq_next = list;
	list->freq_next = NULL;
	list->freq_prev = tmp;
}


void
merge_two_word_freq_list (ccinPhraseTwoWordInfo_t * sys,
						  ccinPhraseTwoWordInfo_t * usr)
{
	ccinPhraseTwoWordInfo_t *head;
	ccinPhraseTwoWordInfo_t *tmp;
	ccinPhraseTwoWordInfo_t *node_next;
	ccinPhraseTwoWordInfo_t *cur_node;

	tmp = sys;
	head = sys;
	//找出最大词频和最小词频
	while (tmp)
	{
		if (tmp->freq > head->freq)
		{
			head = tmp;
		}
		tmp = tmp->pos_next;
	}

	if (!head)
		head = usr;

	tmp = usr;
	while (tmp)
	{
		if (tmp->freq > head->freq)

		{
			head = tmp;
		}
		tmp = tmp->pos_next;
	}

	tmp = sys;
	while (tmp != NULL)
	{
		cur_node = head;
		while (cur_node != NULL)
		{
			if (head == tmp)
				break;

			node_next = cur_node->freq_next;

			if (!node_next)
			{
				if (cur_node->freq >= tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_prev = cur_node;
					break;
				}
				else
				{
					tmp->freq_next = cur_node;
					cur_node->freq_prev = tmp;
					break;
				}
			}
			else
			{
				if (node_next->freq <= tmp->freq &&
					cur_node->freq > tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_next = node_next;
					tmp->freq_prev = cur_node;
					node_next->freq_prev = tmp;
					break;
				}
			}
			cur_node = cur_node->freq_next;
		}
		tmp = tmp->pos_next;
	}

	tmp = usr;
	while (tmp != NULL)
	{
		cur_node = head;
		while (cur_node != NULL)
		{
			if (head == tmp)
				break;

			node_next = cur_node->freq_next;

			if (!node_next)
			{
				if (cur_node->freq >= tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_prev = cur_node;
					break;
				}
				else
				{
					tmp->freq_next = cur_node;
					cur_node->freq_prev = tmp;
					break;
				}
			}
			else
			{

				if (node_next->freq <= tmp->freq &&
					cur_node->freq > tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_next = node_next;
					tmp->freq_prev = cur_node;
					node_next->freq_prev = tmp;
					break;
				}
			}
			cur_node = cur_node->freq_next;
		}
		tmp = tmp->pos_next;
	}
}

void
merge_three_word_freq_list (ccinPhraseThreeWordInfo_t * sys,
							ccinPhraseThreeWordInfo_t * usr)
{
	ccinPhraseThreeWordInfo_t *head;
	ccinPhraseThreeWordInfo_t *tmp;
	ccinPhraseThreeWordInfo_t *node_next;
	ccinPhraseThreeWordInfo_t *cur_node;

	tmp = sys;
	head = sys;
	//找出最大词频和最小词频
	while (tmp)
	{
		if (tmp->freq > head->freq)
		{
			head = tmp;
		}
		tmp = tmp->pos_next;
	}

	if (!head)
		head = usr;

	tmp = usr;
	while (tmp)
	{
		if (tmp->freq > head->freq)
		{
			head = tmp;
		}
		tmp = tmp->pos_next;
	}

	tmp = sys;
	while (tmp != NULL)
	{
		cur_node = head;
		while (cur_node != NULL)
		{
			if (head == tmp)
				break;

			node_next = cur_node->freq_next;

			if (!node_next)
			{
				if (cur_node->freq >= tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_prev = cur_node;
					break;
				}
				else
				{
					tmp->freq_next = cur_node;
					cur_node->freq_prev = tmp;
					break;
				}
			}
			else
			{
				if (node_next->freq <= tmp->freq &&
					cur_node->freq > tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_next = node_next;
					tmp->freq_prev = cur_node;
					node_next->freq_prev = tmp;
					break;
				}
			}
			cur_node = cur_node->freq_next;
		}
		tmp = tmp->pos_next;
	}

	tmp = usr;
	while (tmp != NULL)
	{
		cur_node = head;
		while (cur_node != NULL)
		{
			if (head == tmp)
				break;

			node_next = cur_node->freq_next;

			if (!node_next)
			{
				if (cur_node->freq >= tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_prev = cur_node;
					break;
				}
				else
				{
					tmp->freq_next = cur_node;
					cur_node->freq_prev = tmp;
					break;
				}
			}
			else
			{
				if (node_next->freq <= tmp->freq &&
					cur_node->freq > tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_next = node_next;
					tmp->freq_prev = cur_node;
					node_next->freq_prev = tmp;
					break;
				}
			}
			cur_node = cur_node->freq_next;
		}
		tmp = tmp->pos_next;
	}
}

void
merge_four_word_freq_list (ccinPhraseFourWordInfo_t * sys,
						   ccinPhraseFourWordInfo_t * usr)
{
	ccinPhraseFourWordInfo_t *head;
	ccinPhraseFourWordInfo_t *tmp;
	ccinPhraseFourWordInfo_t *node_next;
	ccinPhraseFourWordInfo_t *cur_node;

	tmp = sys;
	head = sys;
	//找出最大词频和最小词频
	while (tmp)
	{
		if (tmp->freq > head->freq)
		{
			head = tmp;
		}
		tmp = tmp->pos_next;
	}

	if (!head)
		head = usr;

	tmp = usr;
	while (tmp)
	{
		if (tmp->freq > head->freq)
		{
			head = tmp;
		}
		tmp = tmp->pos_next;
	}

	tmp = sys;
	while (tmp != NULL)
	{
		cur_node = head;
		while (cur_node != NULL)
		{
			if (head == tmp)
				break;

			node_next = cur_node->freq_next;

			if (!node_next)
			{
				if (cur_node->freq >= tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_prev = cur_node;
					break;
				}
				else
				{
					tmp->freq_next = cur_node;
					cur_node->freq_prev = tmp;
					break;
				}
			}
			else
			{
				if (node_next->freq <= tmp->freq &&
					cur_node->freq > tmp->freq)
				{

					cur_node->freq_next = tmp;
					tmp->freq_next = node_next;
					tmp->freq_prev = cur_node;
					node_next->freq_prev = tmp;
					break;
				}
			}
			cur_node = cur_node->freq_next;
		}
		tmp = tmp->pos_next;
	}

	tmp = usr;
	while (tmp != NULL)
	{
		cur_node = head;
		while (cur_node != NULL)
		{
			if (head == tmp)
				break;

			node_next = cur_node->freq_next;

			if (!node_next)
			{
				if (cur_node->freq >= tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_prev = cur_node;
					break;
				}
				else
				{
					tmp->freq_next = cur_node;
					cur_node->freq_prev = tmp;
					break;
				}
			}
			else
			{
				if (node_next->freq <= tmp->freq &&
					cur_node->freq > tmp->freq)
				{
					cur_node->freq_next = tmp;
					tmp->freq_next = node_next;
					tmp->freq_prev = cur_node;
					node_next->freq_prev = tmp;
					break;
				}
			}
			cur_node = cur_node->freq_next;
		}
		tmp = tmp->pos_next;
	}
}
void
ccin_create_freq_list ()
{
	int i;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		create_freq_gb_list (g_sys_global_glossary.sys_syllable_info[i].
							 sys_phrase_word_gb);
		merge_two_word_freq_list (g_sys_global_glossary.sys_syllable_info[i].
								  sys_phrase_two_word,
								  g_user_global_glossary.
								  sys_syllable_info[i].
								  sys_phrase_two_word);
		merge_three_word_freq_list (g_sys_global_glossary.
									sys_syllable_info[i].
									sys_phrase_three_word,
									g_user_global_glossary.
									sys_syllable_info[i].
									sys_phrase_three_word);
		merge_four_word_freq_list (g_sys_global_glossary.
								   sys_syllable_info[i].
								   sys_phrase_four_word,
								   g_user_global_glossary.
								   sys_syllable_info[i].
								   sys_phrase_four_word);
	}
}


void
init_GlossarySyllableInfo ()
{
	int i;
	int count;					//记录标志数
	int loop;
	int flags = 0;				//标志位置记录
	int j = 0;					//取出词条循环变量
	int offset;					//指针的每个音节段内的偏移量
	u_short above_four[11];		//处理超过四字词时使用该数组存储相应的字词数量
	ccinGBWordInfo_t *gb_list;		//单个gb词条临时变量
	ccinGBKWordInfo_t *gbk_list;	//单个gbk词条临时变量
	ccinPhraseTwoWordInfo_t *two_word_list;	//单个两字词条临时变量
	ccinPhraseThreeWordInfo_t *three_word_list;	//单个三字词条临时变量
	ccinPhraseFourWordInfo_t *four_word_list;	//单个四字词条临时变量
	ccinLongPhraseInfo_t *above_four_word_list;	//单个超过四字词条临时变量
	char *tmp;

	g_sys_global_glossary.all_system_phrase_total =
		system_glossary_file_head->phrase_total;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		g_sys_global_glossary.sys_syllable_info[i].syllable_phrase_total =
			system_syllable_segment_head[i]->word_phrase_total;
		//初始化字词个数变量
		g_sys_global_glossary.sys_syllable_info[i].phrase_word_gb_num = 0;
		g_sys_global_glossary.sys_syllable_info[i].phrase_word_gbk_num = 0;
		g_sys_global_glossary.sys_syllable_info[i].phrase_two_word_num = 0;
		g_sys_global_glossary.sys_syllable_info[i].phrase_three_word_num = 0;
		g_sys_global_glossary.sys_syllable_info[i].phrase_four_word_num = 0;
		g_sys_global_glossary.sys_syllable_info[i].phrase_above_four_num = 0;
		g_sys_global_glossary.sys_syllable_info[i].sys_phrase_word_gb = NULL;
		g_sys_global_glossary.sys_syllable_info[i].sys_phrase_word_gbk =
			NULL;
		g_sys_global_glossary.sys_syllable_info[i].sys_phrase_two_word =
			NULL;
		g_sys_global_glossary.sys_syllable_info[i].sys_phrase_three_word =
			NULL;
		g_sys_global_glossary.sys_syllable_info[i].sys_phrase_four_word =
			NULL;
		g_sys_global_glossary.sys_syllable_info[i].sys_phrase_above_four =
			NULL;


		count =
			(system_glossary_file_head->
			 phrase_syllable_segment_head_info[i].size -
			 3 * sizeof (u_short)) / 2;
		flags = 0;
		offset = 0;

		//处理1-4字
		for (loop = 0; loop < 5; loop++)
		{
			if ((system_syllable_segment_head[i]->
				 word_phrase_flag >> loop) & 0x01)
			{
				switch (loop)
				{
				case 0:
					g_sys_global_glossary.sys_syllable_info[i].
						phrase_word_gb_num =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					if (!g_sys_global_glossary.sys_syllable_info[i].
						sys_phrase_word_gb)
					{
						g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_word_gb =
							(ccinGBWordInfo_t *) malloc (sizeof (ccinGBWordInfo_t)
													 *
													 g_sys_global_glossary.
													 sys_syllable_info[i].
													 phrase_word_gb_num);
					}
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_word_gb_num; j++)
					{
						tmp =
							(char *) p +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].offset +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].size;
						gb_list =
							g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_word_gb + j;
						strncpy (gb_list->word,
								 tmp + j * sizeof (ccinHanziChar_t),
								 sizeof (ccinHanziChar_t));
						gb_list->pinyin_key =
							system_syllable_segment_head[i]->pinyin_key;
						gb_list->freq = 0;
						gb_list->pos_next = NULL;
						gb_list->freq_next = NULL;
						gb_list->freq_prev = NULL;
						create_gb_list (g_sys_global_glossary.
										sys_syllable_info[i].
										sys_phrase_word_gb, gb_list);
					}
					offset =
						g_sys_global_glossary.sys_syllable_info[i].
						phrase_word_gb_num * sizeof (ccinHanziChar_t);
					flags++;
					break;
				case 1:
					g_sys_global_glossary.sys_syllable_info[i].
						phrase_word_gbk_num =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					if (!g_sys_global_glossary.sys_syllable_info[i].
						sys_phrase_word_gbk)
					{
						g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_word_gbk =
							(ccinGBKWordInfo_t *)
							malloc (sizeof (ccinGBKWordInfo_t) *
									g_sys_global_glossary.
									sys_syllable_info[i].
									phrase_word_gbk_num);
					}
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_word_gbk_num; j++)
					{
						tmp =
							(char *) p +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].offset +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].size +
							offset;
						gbk_list =
							g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_word_gbk + j;
						strncpy (gbk_list->word,
								 tmp + j * sizeof (ccinHanziChar_t),
								 sizeof (ccinHanziChar_t));
						gbk_list->pinyin_key =
							system_syllable_segment_head[i]->pinyin_key;
						gbk_list->pos_next = NULL;
						create_gbk_list (g_sys_global_glossary.
										 sys_syllable_info[i].
										 sys_phrase_word_gbk, gbk_list);
					}
					flags++;
					offset +=
						g_sys_global_glossary.sys_syllable_info[i].
						phrase_word_gbk_num * sizeof (ccinHanziChar_t);
					break;
				case 2:
					g_sys_global_glossary.sys_syllable_info[i].
						phrase_two_word_num =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					if (!g_sys_global_glossary.sys_syllable_info[i].
						sys_phrase_two_word)
					{
						g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_two_word =
							(ccinPhraseTwoWordInfo_t *)
							malloc (sizeof (ccinPhraseTwoWordInfo_t) *
									g_sys_global_glossary.
									sys_syllable_info[i].
									phrase_two_word_num);
					}
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_two_word_num; j++)
					{
						tmp =
							(char *) p +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].offset +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].size +
							offset;
						two_word_list =
							g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_two_word + j;
						strncpy ((char *) two_word_list->phrase,
								 tmp + j * (2 * sizeof (ccinHanziChar_t) +
											sizeof (u_short)),
								 2 * sizeof (ccinHanziChar_t));
						two_word_list->pinyin_key[0] =
							system_syllable_segment_head[i]->pinyin_key;
						memcpy (&two_word_list->pinyin_key[1],
								tmp + j * (2 * sizeof (ccinHanziChar_t) +
										   sizeof (u_short)) +
								2 * sizeof (ccinHanziChar_t),
								sizeof (u_short));
						two_word_list->freq = 0;
						two_word_list->pos_next = NULL;
						two_word_list->freq_next = NULL;
						two_word_list->freq_prev = NULL;
						two_word_list->is_system = 0;
						create_two_word_list (g_sys_global_glossary.
											  sys_syllable_info[i].
											  sys_phrase_two_word,
											  two_word_list);
					}
					flags++;
					offset +=
						g_sys_global_glossary.sys_syllable_info[i].
						phrase_two_word_num * (sizeof (ccinHanziChar_t) * 2 +
											   sizeof (u_short));
					break;
				case 3:
					g_sys_global_glossary.sys_syllable_info[i].
						phrase_three_word_num =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					if (!g_sys_global_glossary.sys_syllable_info[i].
						sys_phrase_three_word)
					{
						g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_three_word =
							(ccinPhraseThreeWordInfo_t *)
							malloc (sizeof (ccinPhraseThreeWordInfo_t) *
									g_sys_global_glossary.
									sys_syllable_info[i].
									phrase_three_word_num);
					}
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_three_word_num; j++)
					{
						tmp =
							(char *) p +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].offset +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].size +
							offset;
						three_word_list =
							g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_three_word + j;
						strncpy ((char *) three_word_list->phrase,
								 tmp + j * (3 * sizeof (ccinHanziChar_t) +
											2 * sizeof (u_short)),
								 3 * sizeof (ccinHanziChar_t));
						three_word_list->pinyin_key[0] =
							system_syllable_segment_head[i]->pinyin_key;
						memcpy (&three_word_list->pinyin_key[1],
								tmp + j * (3 * sizeof (ccinHanziChar_t) +
										   2 * sizeof (u_short)) +
								3 * sizeof (ccinHanziChar_t),
								2 * sizeof (u_short));
						three_word_list->freq = 0;
						three_word_list->pos_next = NULL;
						three_word_list->freq_next = NULL;
						three_word_list->freq_prev = NULL;
						three_word_list->is_system = 0;
						create_three_word_list (g_sys_global_glossary.
												sys_syllable_info[i].
												sys_phrase_three_word,
												three_word_list);
					}
					flags++;
					offset +=
						g_sys_global_glossary.sys_syllable_info[i].
						phrase_three_word_num * (sizeof (ccinHanziChar_t) *
												 3 + sizeof (u_short) * 2);
					break;
				case 4:
					g_sys_global_glossary.sys_syllable_info[i].
						phrase_four_word_num =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					if (!g_sys_global_glossary.sys_syllable_info[i].
						sys_phrase_four_word)
					{
						g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_four_word =
							(ccinPhraseFourWordInfo_t *)
							malloc (sizeof (ccinPhraseFourWordInfo_t) *
									g_sys_global_glossary.
									sys_syllable_info[i].
									phrase_four_word_num);
					}
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_four_word_num; j++)
					{
						tmp =
							(char *) p +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].offset +
							system_glossary_file_head->
							phrase_syllable_segment_head_info[i].size +
							offset;
						four_word_list =
							g_sys_global_glossary.sys_syllable_info[i].
							sys_phrase_four_word + j;
						strncpy ((char *) four_word_list->phrase,
								 tmp + j * (4 * sizeof (ccinHanziChar_t) +
											3 * sizeof (u_short)),
								 4 * sizeof (ccinHanziChar_t));
						four_word_list->pinyin_key[0] =
							system_syllable_segment_head[i]->pinyin_key;
						memcpy (&four_word_list->pinyin_key[1],
								tmp + j * (4 * sizeof (ccinHanziChar_t) +
										   3 * sizeof (u_short)) +
								4 * sizeof (ccinHanziChar_t),
								3 * sizeof (u_short));
						four_word_list->freq = 0;
						four_word_list->pos_next = NULL;
						four_word_list->freq_next = NULL;
						four_word_list->freq_prev = NULL;
						four_word_list->is_system = 0;
						create_four_word_list (g_sys_global_glossary.
											   sys_syllable_info[i].
											   sys_phrase_four_word,
											   four_word_list);
					}
					flags++;
					offset +=
						g_sys_global_glossary.sys_syllable_info[i].
						phrase_four_word_num * (sizeof (ccinHanziChar_t) *
												4 + 3 * sizeof (u_short));
					break;
				}
			}
		}

		//处理四字以上字词
		for (loop = 0; loop < 11; loop++)
			above_four[loop] = 0;

		for (loop = 5; loop < 16; loop++)
		{
			if ((system_syllable_segment_head[i]->
				 word_phrase_flag >> loop) & 0x01)
			{
				switch (loop)
				{
				case 5:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 6:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 7:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 8:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 9:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 10:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 11:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 12:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 13:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 14:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 15:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(system_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				}
			}
		}

		for (loop = 0; loop < 11; loop++)
		{
			g_sys_global_glossary.sys_syllable_info[i].
				phrase_above_four_num += above_four[loop];
		}

		if (!g_sys_global_glossary.sys_syllable_info[i].sys_phrase_above_four
			&& g_sys_global_glossary.sys_syllable_info[i].
			phrase_above_four_num != 0)
		{
			g_sys_global_glossary.sys_syllable_info[i].
				sys_phrase_above_four =
				(ccinLongPhraseInfo_t *) malloc (sizeof (ccinLongPhraseInfo_t) *
											 g_sys_global_glossary.
											 sys_syllable_info[i].
											 phrase_above_four_num);
			above_four_word_list =
				g_sys_global_glossary.sys_syllable_info[i].
				sys_phrase_above_four;
		}
//continue;
		j = 0;
		for (loop = 5; loop < 16; loop++)
		{
			if (above_four[loop - 5] != 0 &&
				g_sys_global_glossary.sys_syllable_info[i].
				phrase_above_four_num != 0)
			{
				//for(j=0; j<g_sys_global_glossary.sys_syllable_info[i].phrase_above_four_num;j++)
				for (j = 0; j < above_four[loop - 5]; j++)
					//while((above_four[loop-5]--)!=0)
				{
					tmp =
						(char *) p +
						system_glossary_file_head->
						phrase_syllable_segment_head_info[i].offset +
						system_glossary_file_head->
						phrase_syllable_segment_head_info[i].size + offset;

					strncpy ((char *) above_four_word_list->phrase, tmp,
							 loop * sizeof (ccinHanziChar_t));
					above_four_word_list->pinyin_key[0] =
						system_syllable_segment_head[i]->pinyin_key;

					memcpy (&above_four_word_list->pinyin_key[1],
							tmp + loop * sizeof (ccinHanziChar_t),
							(loop - 1) * sizeof (u_short));
					above_four_word_list->word_number = loop;
					above_four_word_list->is_system = 0;
					above_four_word_list->pos_next = NULL;
					create_above_four_word_list (g_sys_global_glossary.
												 sys_syllable_info[i].
												 sys_phrase_above_four,
												 above_four_word_list);
					above_four_word_list++;
					offset +=
						(sizeof (ccinHanziChar_t) * loop +
						 (loop - 1) * sizeof (u_short));
				}
			}
		}
	}
}

void
init_GlossaryFileHead ()
{
	system_glossary_file_head = (ccinGlossaryFileHead_t *) p;

#ifdef DEBUG
//	for(i=0;i<SYLLABLE_TOTAL;i++)
//		printf("%d %d |",system_glossary_file_head->phrase_syllable_segment_head_info[i].offset,system_glossary_file_head->phrase_syllable_segment_head_info[i].size);
#endif
}

void
init_SyllableFileSegmentHead ()
{
	int i;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		system_syllable_segment_head[i] = (ccinSyllableSegmentHead_t *) (p +
																	 system_glossary_file_head->
																	 phrase_syllable_segment_head_info
																	 [i].
																	 offset);
#ifdef DEBUG
		//printf("%d %d %d %d\n",system_syllable_segment_head[i]->pinyin_key,system_syllable_segment_head[i]->word_phrase_flag,system_syllable_segment_head[i]->word_phrase_total,*(u_short*)(&system_syllable_segment_head[i]->word_phrase_num_pointer));
#endif
	}
}

void
ccin_load_system_glossary ()
{
	FILE *fp;
	int size;
	char buf[4];

	char *path;
	char buf1[255];

//   printf("%s\n",SYSTEM_FILE_PATH "" SYSTEM_GLOSSARY_FILE_NAME); 
	path = getenv ("HOME");
	bzero (buf1, 255);
	strcat (buf1, path);
	strcat (buf1, USER_FILE_PATH);
	strcat (buf1, SYSTEM_GLOSSARY_FILE_NAME);
	if ((fp = fopen (buf, "rb")) == NULL)
	{
		fp = fopen (SYSTEM_FILE_PATH "" SYSTEM_GLOSSARY_FILE_NAME, "rb");
		if (!fp)
		{
			perror ("ccin_load_system_glossary function error");
			exit (0);
		}
	}

	if (fseek (fp, -4, SEEK_END) == -1 ||
		fread (&size, sizeof (int), 1, fp) != 1 || size != ftell (fp) - 4)
	{
		perror ("ccin_load_system_glossary function error.");
		exit (0);
	}

	p = (char *) malloc (sizeof (char) * size);	//分配系统词库文件大小的内存区域

	fseek (fp, 0, SEEK_SET);

	fread (p, size, 1, fp);

	init_GlossaryFileHead ();
	init_SyllableFileSegmentHead ();
	init_GlossarySyllableInfo ();

	fclose (fp);
};

void
ccin_load_user_frequency ()
{
	FILE *fp;
	int size;
	int i = 0, j = 0;
	int offset;
	int loop;
	u_short flag;
	char *path;
	char buf[255];

	path = getenv ("HOME");
	bzero (buf, 255);
	strcat (buf, path);
	strcat (buf, USER_FILE_PATH);
	strcat (buf, USER_FREQUENCY_FILE_NAME);
	if ((fp = fopen (buf, "rb")) == NULL)
	{
		fp = fopen (SYSTEM_FILE_PATH "" USER_FREQUENCY_FILE_NAME, "rb");
		if (!fp)
		{
//      perror("ccin_load_user_frequency function error");
			user_load_flags = 0;
			return;
		}
	}

	if (fseek (fp, -4, SEEK_END) == -1 ||
		fread (&size, sizeof (int), 1, fp) != 1 || size != ftell (fp) - 4)
	{
//    perror("ccin_load_user_frequency function error");
		user_load_flags = 0;
		return;
	}

	pUsrFreq = (char *) malloc (size);	//分配系统词频文件大小的内存区域

	fseek (fp, 0, SEEK_SET);

	fread (pUsrFreq, size, 1, fp);

	memcpy (&user_glossary_freq_file_head, pUsrFreq,
			sizeof (ccinGlossaryFileHead_t));

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		offset =
			user_glossary_freq_file_head.
			phrase_syllable_segment_head_info[i].offset +
			user_glossary_freq_file_head.
			phrase_syllable_segment_head_info[i].size;
		flag =
			*(u_short *) (pUsrFreq +
						  user_glossary_freq_file_head.
						  phrase_syllable_segment_head_info[i].offset +
						  sizeof (u_short));
		for (loop = 0; loop < 5; loop++)
		{
//      if((system_syllable_segment_head[i]->word_phrase_flag >> loop)&0x01)
			if ((flag >> loop) & 0x01)
			{
				switch (loop)
				{
				case 0:
					break;
				case 1:
					break;
				case 2:
					for (j = 0;
						 j <
						 g_user_global_glossary.sys_syllable_info[i].
						 phrase_two_word_num; j++)
					{
						(g_user_global_glossary.sys_syllable_info[i].
						 sys_phrase_two_word + j)->freq =
*(u_char *) (pUsrFreq + offset);
						offset++;
					}
					break;
				case 3:
					for (j = 0;
						 j <
						 g_user_global_glossary.sys_syllable_info[i].
						 phrase_three_word_num; j++)
					{
						(g_user_global_glossary.sys_syllable_info[i].
						 sys_phrase_three_word + j)->freq =
*(u_char *) (pUsrFreq + offset);
						offset++;
					}
					break;
				case 4:
					for (j = 0;
						 j <
						 g_user_global_glossary.sys_syllable_info[i].
						 phrase_four_word_num; j++)
					{
						(g_user_global_glossary.sys_syllable_info[i].
						 sys_phrase_four_word + j)->freq =
*(u_char *) (pUsrFreq + offset);
						offset++;
					}
					break;
				}
			}
		}
	}
	fclose (fp);
};

void
ccin_load_system_frequency ()
{
	FILE *fp;
	int size;
	int i = 0, j = 0;
	int offset;
	int loop;
	u_short flag;
	char *path;
	char buf[255];

	path = getenv ("HOME");
	bzero (buf, 255);
	strcat (buf, path);
	strcat (buf, USER_FILE_PATH);
	strcat (buf, SYSTEM_FREQUENCY_FILE_NAME);

	if ((fp = fopen (buf, "rb")) == NULL)
	{
		fp = fopen (SYSTEM_FILE_PATH "" SYSTEM_FREQUENCY_FILE_NAME, "rb");
		if (!fp)
		{
			perror ("ccin_load_system_frequency function error");
			exit (0);
		}
	}

	if (fseek (fp, -4, SEEK_END) == -1 ||
		fread (&size, sizeof (int), 1, fp) != 1 || size != ftell (fp) - 4)
	{
		perror ("ccin_load_system_frequency function error");
		exit (0);
	}

	sys_freq_file_size = size;
	pFreq = (char *) malloc (size);	//分配系统词频文件大小的内存区域

	fseek (fp, 0, SEEK_SET);

	fread (pFreq, size, 1, fp);

	system_glossary_freq_file_head = (ccinGlossaryFileHead_t *) pFreq;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		offset =
			system_glossary_freq_file_head->
			phrase_syllable_segment_head_info[i].offset +
			system_glossary_freq_file_head->
			phrase_syllable_segment_head_info[i].size;
		flag =
			*(u_short *) (pFreq +
						  system_glossary_freq_file_head->
						  phrase_syllable_segment_head_info[i].offset +
						  sizeof (u_short));
		for (loop = 0; loop < 5; loop++)
		{
//      if((system_syllable_segment_head[i]->word_phrase_flag >> loop)&0x01)
			if ((flag >> loop) & 0x01)
			{
				switch (loop)
				{
				case 0:
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_word_gb_num; j++)
					{
						(g_sys_global_glossary.sys_syllable_info[i].
						 sys_phrase_word_gb + j)->freq =
*(u_char *) (pFreq + offset);
						offset++;
					}
					break;
				case 1:
					break;
				case 2:
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_two_word_num; j++)
					{
						(g_sys_global_glossary.sys_syllable_info[i].
						 sys_phrase_two_word + j)->freq =
*(u_char *) (pFreq + offset);
						offset++;
					}
					break;
				case 3:
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_three_word_num; j++)
					{
						(g_sys_global_glossary.sys_syllable_info[i].
						 sys_phrase_three_word + j)->freq =
*(u_char *) (pFreq + offset);
						offset++;
					}
					break;
				case 4:
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_four_word_num; j++)
					{
						(g_sys_global_glossary.sys_syllable_info[i].
						 sys_phrase_four_word + j)->freq =
*(u_char *) (pFreq + offset);
						offset++;
					}
					break;
				}
			}
		}
	}
	fclose (fp);
	free (p);					//因为在此函数中还要用到system_syllable_segment_head,所以要在此处释放掉该指针.
};

void
init_UsrGlossaryFileHead ()
{
	memcpy (&user_glossary_file_head, (ccinGlossaryFileHead_t *) pUsr,
			sizeof (ccinGlossaryFileHead_t));

#ifdef DEBUG
//  for(i=0;i<SYLLABLE_TOTAL;i++)
//  printf("%d %d |",user_glossary_file_head.phrase_syllable_segment_head_info[i].offset,user_glossary_file_head.phrase_syllable_segment_head_info[i].size);
#endif
}

void
init_UsrSyllableFileSegmentHead ()
{

	int i;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		user_syllable_segment_head[i] =
			(ccinSyllableSegmentHead_t *) (pUsr +
									   user_glossary_file_head.
									   phrase_syllable_segment_head_info
									   [i].offset);
#ifdef DEBUG
//    printf("%d %d %d %d %d\n",user_glossary_file_head.phrase_syllable_segment_head_info[i].offset,user_syllable_segment_head[i]->pinyin_key,user_syllable_segment_head[i]->word_phrase_flag,user_syllable_segment_head[i]->word_phrase_total,*((u_short*)(&user_syllable_segment_head[i]->word_phrase_num_pointer)+2));
#endif
	}
}

void
init_user_GlossarySyllableInfo ()
{
	int i;
	int count;					//记录标志数
	int loop;
	int flags = 0;				//标志位置记录
	int j = 0;					//取出词条循环变量
	int offset;					//指针的每个音节段内的偏移量
	u_short above_four[11];		//处理超过四字词时使用该数组存储相应的字词数量
	ccinPhraseTwoWordInfo_t *two_word_list;	//单个两字词条临时变量
	ccinPhraseThreeWordInfo_t *three_word_list;	//单个三字词条临时变量
	ccinPhraseFourWordInfo_t *four_word_list;	//单个四字词条临时变量
	ccinLongPhraseInfo_t *above_four_word_list;	//单个超过四字词条临时变量
	char *tmp;

	g_user_global_glossary.all_system_phrase_total =
		user_glossary_file_head.phrase_total;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		g_user_global_glossary.sys_syllable_info[i].syllable_phrase_total =
			user_syllable_segment_head[i]->word_phrase_total;
		//初始化字词个数变量
		g_user_global_glossary.sys_syllable_info[i].phrase_word_gb_num = 0;
		g_user_global_glossary.sys_syllable_info[i].phrase_word_gbk_num = 0;
		g_user_global_glossary.sys_syllable_info[i].phrase_two_word_num = 0;
		g_user_global_glossary.sys_syllable_info[i].phrase_three_word_num =
			0;
		g_user_global_glossary.sys_syllable_info[i].phrase_four_word_num = 0;
		g_user_global_glossary.sys_syllable_info[i].phrase_above_four_num =
			0;
		g_user_global_glossary.sys_syllable_info[i].sys_phrase_word_gb =
			NULL;
		g_user_global_glossary.sys_syllable_info[i].sys_phrase_word_gbk =
			NULL;
		g_user_global_glossary.sys_syllable_info[i].sys_phrase_two_word =
			NULL;
		g_user_global_glossary.sys_syllable_info[i].sys_phrase_three_word =
			NULL;
		g_user_global_glossary.sys_syllable_info[i].sys_phrase_four_word =
			NULL;
		g_user_global_glossary.sys_syllable_info[i].sys_phrase_above_four =
			NULL;


		count =
			(user_glossary_file_head.phrase_syllable_segment_head_info[i].
			 size - 3 * sizeof (u_short)) / 2;
		flags = 0;
		offset = 0;
		//处理1-4字
		for (loop = 0; loop < 5; loop++)
		{
			if ((user_syllable_segment_head[i]->
				 word_phrase_flag >> loop) & 0x01)
			{
				switch (loop)
				{
				case 0:
					break;
				case 1:
					break;
				case 2:
					g_user_global_glossary.sys_syllable_info[i].
						phrase_two_word_num =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					if (!g_user_global_glossary.sys_syllable_info[i].
						sys_phrase_two_word)
					{
						g_user_global_glossary.sys_syllable_info[i].
							sys_phrase_two_word =
							(ccinPhraseTwoWordInfo_t *)
							malloc (sizeof (ccinPhraseTwoWordInfo_t) *
									g_user_global_glossary.
									sys_syllable_info[i].
									phrase_two_word_num);
					}
					for (j = 0;
						 j <
						 g_user_global_glossary.sys_syllable_info[i].
						 phrase_two_word_num; j++)
					{
						tmp =
							(char *) pUsr +
							user_glossary_file_head.
							phrase_syllable_segment_head_info[i].offset +
							user_glossary_file_head.
							phrase_syllable_segment_head_info[i].size +
							offset;
						two_word_list =
							g_user_global_glossary.sys_syllable_info[i].
							sys_phrase_two_word + j;
						strncpy ((char *) two_word_list->phrase,
								 tmp + j * (2 * sizeof (ccinHanziChar_t) +
											sizeof (u_short)),
								 2 * sizeof (ccinHanziChar_t));
						two_word_list->pinyin_key[0] = i + 1;
						memcpy (&two_word_list->pinyin_key[1],
								tmp + j * (2 * sizeof (ccinHanziChar_t) +
										   sizeof (u_short)) +
								2 * sizeof (ccinHanziChar_t),
								sizeof (u_short));
						two_word_list->freq = 0;
						two_word_list->pos_next = NULL;
						two_word_list->freq_next = NULL;
						two_word_list->freq_prev = NULL;
						two_word_list->is_system = 0;
						create_two_word_list (g_user_global_glossary.
											  sys_syllable_info[i].
											  sys_phrase_two_word,
											  two_word_list);
					}
					flags++;
					offset +=
						g_user_global_glossary.sys_syllable_info[i].
						phrase_two_word_num * (sizeof (ccinHanziChar_t) * 2 +
											   sizeof (u_short));
					break;
				case 3:
					g_user_global_glossary.sys_syllable_info[i].
						phrase_three_word_num =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					if (!g_user_global_glossary.sys_syllable_info[i].
						sys_phrase_three_word)
					{
						g_user_global_glossary.sys_syllable_info[i].
							sys_phrase_three_word =
							(ccinPhraseThreeWordInfo_t *)
							malloc (sizeof (ccinPhraseThreeWordInfo_t) *
									g_user_global_glossary.
									sys_syllable_info[i].
									phrase_three_word_num);
					}
					for (j = 0;
						 j <
						 g_user_global_glossary.sys_syllable_info[i].
						 phrase_three_word_num; j++)
					{
						tmp =
							(char *) pUsr +
							user_glossary_file_head.
							phrase_syllable_segment_head_info[i].offset +
							user_glossary_file_head.
							phrase_syllable_segment_head_info[i].size +
							offset;
						three_word_list =
							g_user_global_glossary.sys_syllable_info[i].
							sys_phrase_three_word + j;
						strncpy ((char *) three_word_list->phrase,
								 tmp + j * (3 * sizeof (ccinHanziChar_t) +
											2 * sizeof (u_short)),
								 3 * sizeof (ccinHanziChar_t));

						memcpy (&three_word_list->pinyin_key[1],
								tmp + j * (3 * sizeof (ccinHanziChar_t) +
										   2 * sizeof (u_short)) +
								3 * sizeof (ccinHanziChar_t),
								2 * sizeof (u_short));

						three_word_list->pinyin_key[0] = i + 1;
						three_word_list->freq = 0;
						three_word_list->pos_next = NULL;
						three_word_list->freq_next = NULL;
						three_word_list->freq_prev = NULL;
						three_word_list->is_system = 0;
						create_three_word_list (g_user_global_glossary.
												sys_syllable_info[i].
												sys_phrase_three_word,
												three_word_list);
					}
					flags++;
					offset +=
						g_user_global_glossary.sys_syllable_info[i].
						phrase_three_word_num * (sizeof (ccinHanziChar_t) *
												 3 + sizeof (u_short) * 2);
					break;
				case 4:
					g_user_global_glossary.sys_syllable_info[i].
						phrase_four_word_num =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					if (!g_user_global_glossary.sys_syllable_info[i].
						sys_phrase_four_word)
					{
						g_user_global_glossary.sys_syllable_info[i].
							sys_phrase_four_word =
							(ccinPhraseFourWordInfo_t *)
							malloc (sizeof (ccinPhraseFourWordInfo_t) *
									g_user_global_glossary.
									sys_syllable_info[i].
									phrase_four_word_num);
					}
					for (j = 0;
						 j <
						 g_user_global_glossary.sys_syllable_info[i].
						 phrase_four_word_num; j++)
					{
						tmp =
							(char *) pUsr +
							user_glossary_file_head.
							phrase_syllable_segment_head_info[i].offset +
							user_glossary_file_head.
							phrase_syllable_segment_head_info[i].size +
							offset;
						four_word_list =
							g_user_global_glossary.sys_syllable_info[i].
							sys_phrase_four_word + j;
						strncpy ((char *) four_word_list->phrase,
								 tmp + j * (4 * sizeof (ccinHanziChar_t) +
											3 * sizeof (u_short)),
								 4 * sizeof (ccinHanziChar_t));
						four_word_list->pinyin_key[0] = i + 1;
						memcpy (&four_word_list->pinyin_key[1],
								tmp + j * (4 * sizeof (ccinHanziChar_t) +
										   3 * sizeof (u_short)) +
								4 * sizeof (ccinHanziChar_t),
								3 * sizeof (u_short));
						four_word_list->freq = 0;
						four_word_list->pos_next = NULL;
						four_word_list->freq_next = NULL;
						four_word_list->freq_prev = NULL;
						four_word_list->is_system = 0;
						create_four_word_list (g_user_global_glossary.
											   sys_syllable_info[i].
											   sys_phrase_four_word,
											   four_word_list);
					}
					flags++;
					offset +=
						g_user_global_glossary.sys_syllable_info[i].
						phrase_four_word_num * (sizeof (ccinHanziChar_t) *
												4 + 3 * sizeof (u_short));
					break;
				}
			}
		}

		//处理四字以上字词
		for (loop = 0; loop < 11; loop++)
			above_four[loop] = 0;

		for (loop = 5; loop < 16; loop++)
		{
			if ((user_syllable_segment_head[i]->
				 word_phrase_flag >> loop) & 0x01)
			{
				switch (loop)
				{
				case 5:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 6:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 7:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 8:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 9:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 10:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 11:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 12:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 13:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 14:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				case 15:
					above_four[loop - 5] =
						*(u_short
						  *) ((u_short
							   *) (&(user_syllable_segment_head[i]->
									 word_phrase_num_pointer)) + flags);
					flags++;
					break;
				}
			}
		}

		for (loop = 0; loop < 11; loop++)
		{
			g_user_global_glossary.sys_syllable_info[i].
				phrase_above_four_num += above_four[loop];
		}

		if (!g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_above_four
			&& g_user_global_glossary.sys_syllable_info[i].
			phrase_above_four_num != 0)
		{
			g_user_global_glossary.sys_syllable_info[i].
				sys_phrase_above_four =
				(ccinLongPhraseInfo_t *) malloc (sizeof (ccinLongPhraseInfo_t) *
											 g_user_global_glossary.
											 sys_syllable_info[i].
											 phrase_above_four_num);
			above_four_word_list =
				g_user_global_glossary.sys_syllable_info[i].
				sys_phrase_above_four;
		}
//continue;
		j = 0;
		for (loop = 5; loop < 16; loop++)
		{
			if (above_four[loop - 5] != 0 &&
				g_user_global_glossary.sys_syllable_info[i].
				phrase_above_four_num != 0)
			{
				//for(j=0; j<g_user_global_glossary.sys_syllable_info[i].phrase_above_four_num;j++)
				for (j = 0; j < above_four[loop - 5]; j++)
					//while((above_four[loop-5]--)!=0)
				{
					tmp =
						(char *) pUsr +
						user_glossary_file_head.
						phrase_syllable_segment_head_info[i].offset +
						user_glossary_file_head.
						phrase_syllable_segment_head_info[i].size + offset;

					strncpy ((char *) above_four_word_list->phrase, tmp,
							 loop * sizeof (ccinHanziChar_t));
					above_four_word_list->pinyin_key[0] = i + 1;

					memcpy (&above_four_word_list->pinyin_key[1],
							tmp + loop * sizeof (ccinHanziChar_t),
							(loop - 1) * sizeof (u_short));
					above_four_word_list->word_number = loop;
					above_four_word_list->is_system = 0;
					above_four_word_list->pos_next = NULL;
					create_above_four_word_list (g_user_global_glossary.
												 sys_syllable_info[i].
												 sys_phrase_above_four,
												 above_four_word_list);
					above_four_word_list++;
					offset +=
						(sizeof (ccinHanziChar_t) * loop +
						 (loop - 1) * sizeof (u_short));
				}
			}
		}
	}
}

void
ccin_load_user_glossary ()
{
	FILE *fp;
	char *path;
	int size;
	char buf[255];

	path = getenv ("HOME");
	bzero (buf, 255);
	strcat (buf, path);
	strcat (buf, USER_FILE_PATH);
	strcat (buf, USER_GLOSSARY_FILE_NAME);

	if ((fp = fopen (buf, "rb")) == NULL)
	{
		if ((fp =
			 fopen (SYSTEM_FILE_PATH "" USER_GLOSSARY_FILE_NAME,
					"rb")) == NULL)
		{
//    perror("ccin_load_user_glossary function error");
			user_load_flags = 0;
			return;
		}
	}

	if (fseek (fp, -4, SEEK_END) == -1 ||
		fread (&size, sizeof (int), 1, fp) != 1 || size != ftell (fp) - 4)
	{
//    perror("ccin_load_user_glossary function error");
		user_load_flags = 0;
		return;
	}

	pUsr = (char *) malloc (sizeof (char) * size);	//分配系统词库文件大小的内存区域

	fseek (fp, 0, SEEK_SET);

	fread (pUsr, size, 1, fp);
	init_UsrGlossaryFileHead ();
	init_UsrSyllableFileSegmentHead ();
	init_user_GlossarySyllableInfo ();
	fclose (fp);
};

void
ccin_save_system_frequency ()
{
	FILE *fp;
	int i = 0, j = 0;
	int offset;
	int loop;
	u_short flag;
	char buf[255];
	char *path;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		offset =
			system_glossary_freq_file_head->
			phrase_syllable_segment_head_info[i].offset +
			system_glossary_freq_file_head->
			phrase_syllable_segment_head_info[i].size;
		flag =
			*(u_short *) (pFreq +
						  system_glossary_freq_file_head->
						  phrase_syllable_segment_head_info[i].offset +
						  sizeof (u_short));
		for (loop = 0; loop < 5; loop++)
		{
			//    if((system_syllable_segment_head[i]->word_phrase_flag >> loop)&0x01)
			if ((flag >> loop) & 0x01)
			{
				switch (loop)
				{
				case 0:
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_word_gb_num; j++)
					{
						*(u_char *) (pFreq + offset) =
							(g_sys_global_glossary.sys_syllable_info[i].
							 sys_phrase_word_gb + j)->freq;
						offset++;
					}
					break;
				case 1:
					break;
				case 2:
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_two_word_num; j++)
					{
						*(u_char *) (pFreq + offset) =
							(g_sys_global_glossary.sys_syllable_info[i].
							 sys_phrase_two_word + j)->freq;
						offset++;
					}
					break;
				case 3:
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_three_word_num; j++)
					{
						*(u_char *) (pFreq + offset) =
							(g_sys_global_glossary.sys_syllable_info[i].
							 sys_phrase_three_word + j)->freq;
						offset++;
					}
					break;
				case 4:
					for (j = 0;
						 j <
						 g_sys_global_glossary.sys_syllable_info[i].
						 phrase_four_word_num; j++)
					{
						*(u_char *) (pFreq + offset) =
							(g_sys_global_glossary.sys_syllable_info[i].
							 sys_phrase_four_word + j)->freq;
						offset++;
					}
					break;
				}
			}
		}
	}

	path = getenv ("HOME");
	bzero (buf, 255);
	strcat (buf, path);
	strcat (buf, USER_FILE_PATH);
	mkdir (buf, 0700);
	strcat (buf, SYSTEM_FREQUENCY_FILE_NAME);
	if ((fp = fopen (buf, "wb")) == NULL)
	{
		perror (buf);
		exit (0);
	}

	fwrite (pFreq, sys_freq_file_size, 1, fp);
	fwrite (&sys_freq_file_size, sizeof (int), 1, fp);
	fclose (fp);
};

void
ccin_save_user_frequency ()
{
	int i;
	FILE *fp;
	char buf[255];
	char *path;
	ccinPhraseTwoWordInfo_t *tmpTwoWord;
	ccinPhraseThreeWordInfo_t *tmpThreeWord;
	ccinPhraseFourWordInfo_t *tmpFourWord;

	path = getenv ("HOME");
	bzero (buf, 255);
	strcat (buf, path);
	strcat (buf, USER_FILE_PATH);
	mkdir (buf, 0700);
	strcat (buf, USER_FREQUENCY_FILE_NAME);

//  if(chang_user_phrase_flags == 0) //chang_user_phrase_flags=0说明没有用户自造词,直接退出.
//    return;

	if ((fp = fopen (buf, "wb")) == NULL)
	{
		perror (buf);
		exit (0);
	}
	create_UsrFrequencyFileSegmentHead ();
	create_freqfilehead (fp);

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		save_user_FrequencyFileSegmentHead (fp, i);

		tmpTwoWord =
			g_user_global_glossary.sys_syllable_info[i].sys_phrase_two_word;
		while (tmpTwoWord)
		{
			fwrite (&tmpTwoWord->freq, sizeof (u_char), 1, fp);
			tmpTwoWord = tmpTwoWord->pos_next;
		}
		tmpThreeWord =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_three_word;
		while (tmpThreeWord)
		{
			fwrite (&tmpThreeWord->freq, sizeof (u_char), 1, fp);
			tmpThreeWord = tmpThreeWord->pos_next;
		}
		tmpFourWord =
			g_user_global_glossary.sys_syllable_info[i].sys_phrase_four_word;
		while (tmpFourWord)
		{
			fwrite (&tmpFourWord->freq, sizeof (u_char), 1, fp);
			tmpFourWord = tmpFourWord->pos_next;
		}
	}

	fwrite (&user_freq_file_size, sizeof (int), 1, fp);
	fclose (fp);
}


void
ccin_add_user_phrase (int phrase_num, ccinHanziChar_t * phrase,
				 u_short pinyin_key[])
{
	ccinPhraseTwoWordInfo_t *tmp_two;
	ccinPhraseThreeWordInfo_t *tmp_three;
	ccinPhraseFourWordInfo_t *tmp_four;
	ccinLongPhraseInfo_t *tmp_above;

//当前用户词的在系统词库和用户词库中的唯一性检查。
	if (ccin_is_phrase_existed_in_glossary (phrase_num, phrase, pinyin_key) !=
		0)
		return;
	//chang_user_phrase_flags = 1;
	if (!user_syllable_segment_head[pinyin_key[0] - 1])
	{
		user_syllable_segment_head[pinyin_key[0] - 1] =
			(ccinSyllableSegmentHead_t *)
			malloc (sizeof (ccinSyllableSegmentHead_t));
		bzero (user_syllable_segment_head[pinyin_key[0] - 1],
			   sizeof (ccinSyllableSegmentHead_t));
	}

	switch (phrase_num)
	{
	case 2:
		if (!g_user_global_glossary.sys_syllable_info[pinyin_key[0] - 1].
			sys_phrase_two_word)
		{
			g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
												   1].sys_phrase_two_word =
				(ccinPhraseTwoWordInfo_t *)
				malloc (sizeof (ccinPhraseTwoWordInfo_t));
			tmp_two =
				g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
													   1].
				sys_phrase_two_word;
		}
		else
		{
			tmp_two =
				(ccinPhraseTwoWordInfo_t *)
				malloc (sizeof (ccinPhraseTwoWordInfo_t));
		}
		tmp_two->pos_next = NULL;
		tmp_two->freq_prev = NULL;
		tmp_two->freq_next = NULL;
		tmp_two->freq = DEFAULT_ORIGINAL_FREQ;
		memcpy (tmp_two->pinyin_key, pinyin_key,
				phrase_num * sizeof (u_short));

		memcpy (tmp_two->phrase, phrase,
				phrase_num * sizeof (ccinHanziChar_t));
		g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
											   1].phrase_two_word_num++;
		user_syllable_segment_head[pinyin_key[0] - 1]->word_phrase_flag |=
			(0x01 << phrase_num);
		create_two_word_list (g_user_global_glossary.
							  sys_syllable_info[pinyin_key[0] -
												1].sys_phrase_two_word,
							  tmp_two);
		if (g_sys_global_glossary.sys_syllable_info[pinyin_key[0] - 1].
			sys_phrase_two_word)
			insert_two_word_freq_list (g_sys_global_glossary.
									   sys_syllable_info[pinyin_key[0] -
														 1].
									   sys_phrase_two_word, tmp_two);
		else
			insert_two_word_freq_list (g_user_global_glossary.
									   sys_syllable_info[pinyin_key[0] -
														 1].
									   sys_phrase_two_word, tmp_two);
		//Todo add frequency adjust
		ccin_phrase_freq_adjust ((void *) tmp_two, PHRASE_TWO);
		break;
	case 3:
		if (!g_user_global_glossary.sys_syllable_info[pinyin_key[0] - 1].
			sys_phrase_three_word)
		{
			g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
												   1].
				sys_phrase_three_word =
				(ccinPhraseThreeWordInfo_t *)
				malloc (sizeof (ccinPhraseThreeWordInfo_t));
			tmp_three =
				g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
													   1].
				sys_phrase_three_word;
		}
		else
		{
			tmp_three =
				(ccinPhraseThreeWordInfo_t *)
				malloc (sizeof (ccinPhraseThreeWordInfo_t));
		}

		tmp_three->pos_next = NULL;
		tmp_three->freq_prev = NULL;
		tmp_three->freq_next = NULL;
		tmp_three->freq = 240;
		memcpy (tmp_three->pinyin_key, pinyin_key,
				phrase_num * sizeof (u_short));

		memcpy (tmp_three->phrase, phrase,
				phrase_num * sizeof (ccinHanziChar_t));
		g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
											   1].phrase_three_word_num++;
		user_syllable_segment_head[pinyin_key[0] - 1]->word_phrase_flag |=
			0x01 << phrase_num;
		create_three_word_list (g_user_global_glossary.
								sys_syllable_info[pinyin_key[0] -
												  1].sys_phrase_three_word,
								tmp_three);
		if (g_sys_global_glossary.sys_syllable_info[pinyin_key[0] - 1].
			sys_phrase_three_word)
			insert_three_word_freq_list (g_sys_global_glossary.
										 sys_syllable_info[pinyin_key[0] -
														   1].
										 sys_phrase_three_word, tmp_three);
		else
			insert_three_word_freq_list (g_user_global_glossary.
										 sys_syllable_info[pinyin_key[0] -
														   1].
										 sys_phrase_three_word, tmp_three);
		//Todo add frequency adjust here
		ccin_phrase_freq_adjust ((void *) tmp_three, PHRASE_THREE);
		break;
	case 4:
		if (!g_user_global_glossary.sys_syllable_info[pinyin_key[0] - 1].
			sys_phrase_four_word)
		{
			g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
												   1].
				sys_phrase_four_word =
				(ccinPhraseFourWordInfo_t *)
				malloc (sizeof (ccinPhraseFourWordInfo_t));
			tmp_four =
				g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
													   1].
				sys_phrase_four_word;
		}
		else
		{
			tmp_four =
				(ccinPhraseFourWordInfo_t *)
				malloc (sizeof (ccinPhraseFourWordInfo_t));
		}
		tmp_four->pos_next = NULL;
		tmp_four->freq_prev = NULL;
		tmp_four->freq_next = NULL;
		tmp_four->freq = 240;
		memcpy (tmp_four->pinyin_key, pinyin_key,
				phrase_num * sizeof (u_short));

		memcpy (tmp_four->phrase, phrase,
				phrase_num * sizeof (ccinHanziChar_t));
		g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
											   1].phrase_four_word_num++;
		user_syllable_segment_head[pinyin_key[0] - 1]->word_phrase_flag |=
			0x01 << phrase_num;
		create_four_word_list (g_user_global_glossary.
							   sys_syllable_info[pinyin_key[0] -
												 1].sys_phrase_four_word,
							   tmp_four);
		if (g_sys_global_glossary.sys_syllable_info[pinyin_key[0] - 1].
			sys_phrase_four_word)
			insert_four_word_freq_list (g_sys_global_glossary.
										sys_syllable_info[pinyin_key[0] -
														  1].
										sys_phrase_four_word, tmp_four);
		else
			insert_four_word_freq_list (g_user_global_glossary.
										sys_syllable_info[pinyin_key[0] -
														  1].
										sys_phrase_four_word, tmp_four);
		//Todo add frequency adjust
		ccin_phrase_freq_adjust ((void *) tmp_four, PHRASE_FOUR);
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		if (!g_user_global_glossary.sys_syllable_info[pinyin_key[0] - 1].
			sys_phrase_above_four)
		{
			tmp_above =
				(ccinLongPhraseInfo_t *) malloc (sizeof (ccinLongPhraseInfo_t));
			g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
												   1].
				sys_phrase_above_four = tmp_above;
		}
		else
		{
			tmp_above =
				(ccinLongPhraseInfo_t *) malloc (sizeof (ccinLongPhraseInfo_t));
		}

		tmp_above->pos_next = NULL;
		tmp_above->word_number = phrase_num;
		memcpy (tmp_above->pinyin_key, pinyin_key,
				phrase_num * sizeof (u_short));

		memcpy (tmp_above->phrase, phrase,
				phrase_num * sizeof (ccinHanziChar_t));
		g_user_global_glossary.sys_syllable_info[pinyin_key[0] -
											   1].phrase_above_four_num++;
		user_syllable_segment_head[pinyin_key[0] - 1]->word_phrase_flag |=
			0x01 << phrase_num;
		create_above_four_word_list (g_user_global_glossary.
									 sys_syllable_info[pinyin_key[0] -
													   1].
									 sys_phrase_above_four, tmp_above);
		break;
	}
}

int
ccin_del_user_phrase (ccinPhraseType_t phraseType, void *node)
{
	int index;
	int ret;
	int offset;

	if (node == NULL)
	{
		return -1;
	}

	switch (phraseType)
	{
	case WORD_GB:
	case WORD_GBK:
		break;
	case PHRASE_TWO:
		index = ((ccinPhraseTwoWordInfo_t *) node)->pinyin_key[0] - 1;

		ret =
			del_word_from_two_word_list (&g_user_global_glossary.
										 sys_syllable_info[index].
										 sys_phrase_two_word,
										 (ccinPhraseTwoWordInfo_t *) node);

		if (ret == 0)
		{
			del_two_word_freq_list ((ccinPhraseTwoWordInfo_t *) node);
			free (node);
			node = NULL;
			g_user_global_glossary.sys_syllable_info[index].
				phrase_two_word_num--;

			if (g_user_global_glossary.sys_syllable_info[index].
				phrase_two_word_num == 0)
			{
				user_syllable_segment_head[index]->word_phrase_flag &=
					~(0x01 << 2);
				g_user_global_glossary.sys_syllable_info[index].
					sys_phrase_two_word = NULL;
			}
			//chang_user_phrase_flags = 1;
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	case PHRASE_THREE:
		index = ((ccinPhraseThreeWordInfo_t *) node)->pinyin_key[0] - 1;

		ret =
			del_word_from_three_word_list (&g_user_global_glossary.
										   sys_syllable_info[index].
										   sys_phrase_three_word,
										   (ccinPhraseThreeWordInfo_t *) node);

		if (ret == 0)
		{
			del_three_word_freq_list ((ccinPhraseThreeWordInfo_t *) node);
			free ((ccinPhraseThreeWordInfo_t *) node);
			node = NULL;
			g_user_global_glossary.sys_syllable_info[index].
				phrase_three_word_num--;

			if (g_user_global_glossary.sys_syllable_info[index].
				phrase_three_word_num == 0)
			{
				user_syllable_segment_head[index]->word_phrase_flag &=
					~(0x01 << 3);
				g_user_global_glossary.sys_syllable_info[index].
					sys_phrase_three_word = NULL;
			}
			//chang_user_phrase_flags = 1;
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	case PHRASE_FOUR:
		index = ((ccinPhraseFourWordInfo_t *) node)->pinyin_key[0] - 1;

		ret =
			del_word_from_four_word_list (&g_user_global_glossary.
										  sys_syllable_info[index].
										  sys_phrase_four_word,
										  (ccinPhraseFourWordInfo_t *) node);

		if (ret == 0)
		{
			del_four_word_freq_list ((ccinPhraseFourWordInfo_t *) node);
			free ((ccinPhraseFourWordInfo_t *) node);
			node = NULL;
			g_user_global_glossary.sys_syllable_info[index].
				phrase_four_word_num--;

			if (g_user_global_glossary.sys_syllable_info[index].
				phrase_four_word_num == 0)
			{
				user_syllable_segment_head[index]->word_phrase_flag &=
					~(0x01 << 4);
				g_user_global_glossary.sys_syllable_info[index].
					sys_phrase_four_word = NULL;
			}
			//chang_user_phrase_flags = 1;
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	case PHRASE_FIVE:
	case PHRASE_SIX:
	case PHRASE_SEVEN:
	case PHRASE_EIGHT:
	case PHRASE_NINE:
		if (phraseType == PHRASE_FIVE)
			offset = 5;
		if (phraseType == PHRASE_SIX)
			offset = 6;
		if (phraseType == PHRASE_SEVEN)
			offset = 7;
		if (phraseType == PHRASE_EIGHT)
			offset = 8;
		if (phraseType == PHRASE_NINE)
			offset = 9;

		index = ((ccinLongPhraseInfo_t *) node)->pinyin_key[0] - 1;

		ret =
			del_word_from_above_word_list (&g_user_global_glossary.
										   sys_syllable_info[index].
										   sys_phrase_above_four,
										   (ccinLongPhraseInfo_t *) node);

		if (ret == 0)
		{
			free ((ccinLongPhraseInfo_t *) node);
			node = NULL;
			g_user_global_glossary.sys_syllable_info[index].
				phrase_above_four_num--;

			if (g_user_global_glossary.sys_syllable_info[index].
				phrase_above_four_num == 0)
			{
				user_syllable_segment_head[index]->word_phrase_flag &=
					~(0x01 << offset);
				g_user_global_glossary.sys_syllable_info[index].
					sys_phrase_above_four = NULL;
			}
			//chang_user_phrase_flags = 1;
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	}
	return 0;
}

void
ccin_save_user_glossary ()
{
	int i;
	FILE *fp;
	char buf[255];
	char *path;
	ccinPhraseTwoWordInfo_t *tmpTwoWord;
	ccinPhraseThreeWordInfo_t *tmpThreeWord;
	ccinPhraseFourWordInfo_t *tmpFourWord;
	ccinLongPhraseInfo_t *tmpLongWord;

	path = getenv ("HOME");
	bzero (buf, 255);
	strcat (buf, path);
	strcat (buf, USER_FILE_PATH);
	mkdir (buf, 0700);
	strcat (buf, USER_GLOSSARY_FILE_NAME);

	//if(chang_user_phrase_flags == 0) //chang_user_phrase_flags=0说明没有用户自造词,直接退出.
	//  return;

	if ((fp = fopen (buf, "wb")) == NULL)
	{
		perror (buf);
		exit (0);
	}
	create_UsrSyllableFileSegmentHead ();
	create_usrfilehead (fp);

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		write_UsrSyllableFileSegmentHead (fp, i);

		tmpTwoWord =
			g_user_global_glossary.sys_syllable_info[i].sys_phrase_two_word;
		while (tmpTwoWord)
		{
			fwrite (tmpTwoWord->phrase, 2 * sizeof (ccinHanziChar_t), 1, fp);
			fwrite (&tmpTwoWord->pinyin_key[1], sizeof (u_short), 1, fp);
			tmpTwoWord = tmpTwoWord->pos_next;
		}
		tmpThreeWord =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_three_word;
		while (tmpThreeWord)
		{
			fwrite (tmpThreeWord->phrase, 3 * sizeof (ccinHanziChar_t), 1,
					fp);
			fwrite (&tmpThreeWord->pinyin_key[1], 2 * sizeof (u_short), 1,
					fp);
			tmpThreeWord = tmpThreeWord->pos_next;
		}
		tmpFourWord =
			g_user_global_glossary.sys_syllable_info[i].sys_phrase_four_word;
		while (tmpFourWord)
		{
			fwrite (tmpFourWord->phrase, 4 * sizeof (ccinHanziChar_t), 1,
					fp);
			fwrite (&tmpFourWord->pinyin_key[1], 3 * sizeof (u_short), 1,
					fp);
			tmpFourWord = tmpFourWord->pos_next;
		}

		tmpLongWord =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_above_four;
		while (tmpLongWord)
		{
			if (tmpLongWord->word_number == 5)
			{
				fwrite (tmpLongWord->phrase,
						tmpLongWord->word_number * sizeof (ccinHanziChar_t),
						1, fp);
				fwrite (&tmpLongWord->pinyin_key[1],
						(tmpLongWord->word_number - 1) * sizeof (u_short),
						1, fp);
			}
			tmpLongWord = tmpLongWord->pos_next;
		}
		tmpLongWord =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_above_four;
		while (tmpLongWord)
		{
			if (tmpLongWord->word_number == 6)
			{
				fwrite (tmpLongWord->phrase,
						tmpLongWord->word_number * sizeof (ccinHanziChar_t),
						1, fp);
				fwrite (&tmpLongWord->pinyin_key[1],
						(tmpLongWord->word_number - 1) * sizeof (u_short),
						1, fp);
			}
			tmpLongWord = tmpLongWord->pos_next;
		}
		tmpLongWord =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_above_four;
		while (tmpLongWord)
		{
			if (tmpLongWord->word_number == 7)
			{
				fwrite (tmpLongWord->phrase,
						tmpLongWord->word_number * sizeof (ccinHanziChar_t),
						1, fp);
				fwrite (&tmpLongWord->pinyin_key[1],
						(tmpLongWord->word_number - 1) * sizeof (u_short),
						1, fp);
			}
			tmpLongWord = tmpLongWord->pos_next;
		}
		tmpLongWord =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_above_four;
		while (tmpLongWord)
		{
			if (tmpLongWord->word_number == 8)
			{
				fwrite (tmpLongWord->phrase,
						tmpLongWord->word_number * sizeof (ccinHanziChar_t),
						1, fp);
				fwrite (&tmpLongWord->pinyin_key[1],
						(tmpLongWord->word_number - 1) * sizeof (u_short),
						1, fp);
			}
			tmpLongWord = tmpLongWord->pos_next;
		}
		tmpLongWord =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_above_four;
		while (tmpLongWord)
		{
			if (tmpLongWord->word_number == 9)
			{
				fwrite (tmpLongWord->phrase,
						tmpLongWord->word_number * sizeof (ccinHanziChar_t),
						1, fp);
				fwrite (&tmpLongWord->pinyin_key[1],
						(tmpLongWord->word_number - 1) * sizeof (u_short),
						1, fp);
			}
			tmpLongWord = tmpLongWord->pos_next;
		}
	}

	fwrite (&user_glossary_file_size, sizeof (int), 1, fp);
	fclose (fp);
};

void
ccin_release_system_glossary ()
{
	int i;

	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		free (g_sys_global_glossary.sys_syllable_info[i].sys_phrase_word_gb);
		free (g_sys_global_glossary.sys_syllable_info[i].
			  sys_phrase_word_gbk);
		free (g_sys_global_glossary.sys_syllable_info[i].
			  sys_phrase_two_word);
		free (g_sys_global_glossary.sys_syllable_info[i].
			  sys_phrase_three_word);
		free (g_sys_global_glossary.sys_syllable_info[i].
			  sys_phrase_four_word);
		free (g_sys_global_glossary.sys_syllable_info[i].
			  sys_phrase_above_four);
	}
};

void
ccin_release_user_glossary ()
{
	int i;
	void *tmp;
	void *tmp1;

	free (pUsr);
	for (i = 0; i < SYLLABLE_TOTAL; i++)
	{
		tmp =
			g_user_global_glossary.sys_syllable_info[i].sys_phrase_two_word;
		free (user_frequency_segment_head[i]);
		while (tmp)
		{
			tmp1 = (void *) (((ccinPhraseTwoWordInfo_t *) tmp)->pos_next);
			free (tmp);
			tmp = tmp1;
		}
		tmp =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_three_word;
		while (tmp)
		{
			tmp1 = (void *) (((ccinPhraseThreeWordInfo_t *) tmp)->pos_next);
			free (tmp);
			tmp = tmp1;
		}
		tmp =
			g_user_global_glossary.sys_syllable_info[i].sys_phrase_four_word;
		while (tmp)
		{
			tmp1 = (void *) (((ccinPhraseFourWordInfo_t *) tmp)->pos_next);
			free (tmp);
			tmp = tmp1;
		}
		tmp =
			g_user_global_glossary.sys_syllable_info[i].
			sys_phrase_above_four;
		while (tmp)
		{
			tmp1 = (void *) (((ccinLongPhraseInfo_t *) tmp)->pos_next);
			free (tmp);
			tmp = tmp1;
		}
	}
};

void
ccin_release_system_frequency ()
{
	free (pFreq);
};

void
ccin_release_user_frequency ()
{
	free (pUsrFreq);
};

void
ccin_close_imfactory ()
{
	ccin_save_system_frequency ();
	ccin_save_user_frequency ();
	ccin_save_user_glossary ();
	ccin_release_system_glossary ();
	ccin_release_system_frequency ();
	ccin_release_user_glossary ();
	ccin_release_user_frequency ();
};

#ifdef DEBUG
#include <iconv.h>
iconv_t utf2gb;
char inbuf[255];
char outbuf[255];
char *inp;
char *outp;
size_t in;
size_t out;

void
utf2gb_convert (char *inp, char *outp, int *i, int *o, char *conv, int len)
{
	bzero (inbuf, 255);
	bzero (outbuf, 255);
	in = 255;
	out = 255;
	strncpy (inbuf, conv, len);
	inp = inbuf;
	outp = outbuf;
	iconv (utf2gb, &inp, i, &outp, o);
}

void
test ()
{
	int i, j;
	void *tmp;
	void *max_node;
	void *min_node;

	//ccinGBWordInfo_t* tmp;
	//ccinGBWordInfo_t* max_node;
	//ccinGBWordInfo_t* min_node;
	utf2gb = iconv_open ("GBK", "UTF-8");
	i = 0;
//    printf("gb单字 %d\n",g_sys_global_glossary.sys_syllable_info[i].phrase_word_gb_num);
	tmp =
		(void *) (g_sys_global_glossary.sys_syllable_info[i].
				  sys_phrase_word_gb);
	while (tmp)
	{
		//for(j=0; j<g_sys_global_glossary.sys_syllable_info[i].phrase_word_gb_num;j++)
		//{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						((ccinGBWordInfo_t *) tmp)->word, 3);
		printf (" pinyin_key %d word %s freq %d ",
				((ccinGBWordInfo_t *) tmp)->pinyin_key, outbuf,
				((ccinGBWordInfo_t *) tmp)->freq);
		tmp = (void *) (((ccinGBWordInfo_t *) tmp)->pos_next);
	}
	printf ("\n");
	printf ("gb单字词频后项链表排序\n");
	tmp =
		(void *) (g_sys_global_glossary.sys_syllable_info[i].
				  sys_phrase_word_gb + 0);
	max_node = tmp;
	while (tmp)
	{
		if (((ccinGBWordInfo_t *) max_node)->freq <
			((ccinGBWordInfo_t *) tmp)->freq)
			max_node = tmp;
		tmp = (void *) (((ccinGBWordInfo_t *) tmp)->pos_next);
	}
	j = 0;
	tmp = max_node;
	while (tmp)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						((ccinGBWordInfo_t *) tmp)->word, 3);
		printf ("word %s freq %d ", outbuf, ((ccinGBWordInfo_t *) tmp)->freq);
		tmp = (void *) (((ccinGBWordInfo_t *) tmp)->freq_next);
		j++;
	}
	printf ("\n");
	printf ("gb单字词频前项链表排序\n");
	tmp =
		(void *) (g_sys_global_glossary.sys_syllable_info[i].
				  sys_phrase_word_gb + 0);
	min_node = tmp;
	while (tmp)
	{
		if (((ccinGBWordInfo_t *) min_node)->freq >
			((ccinGBWordInfo_t *) tmp)->freq)
			min_node = tmp;
		tmp = (void *) (((ccinGBWordInfo_t *) tmp)->pos_next);
	}
	j = 0;
	tmp = min_node;
	while (tmp)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						((ccinGBWordInfo_t *) tmp)->word, 3);
		printf ("word %s freq %d ", outbuf, ((ccinGBWordInfo_t *) tmp)->freq);
		tmp = (void *) (((ccinGBWordInfo_t *) tmp)->freq_prev);
		j++;
	}
	printf ("\n");

	printf ("gbk单字 %d\n",
			g_sys_global_glossary.sys_syllable_info[i].phrase_word_gbk_num);
	for (j = 0;
		 j < g_sys_global_glossary.sys_syllable_info[i].phrase_word_gbk_num;
		 j++)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						(g_sys_global_glossary.sys_syllable_info[i].
						 sys_phrase_word_gbk + j)->word, 3);
		printf ("pinyin_key %d word %s ",
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_word_gbk + j)->pinyin_key, outbuf);
	}
	printf ("\n");
	printf ("两字词 %d\n",
			g_sys_global_glossary.sys_syllable_info[i].phrase_two_word_num);
	for (j = 0;
		 j < g_sys_global_glossary.sys_syllable_info[i].phrase_two_word_num;
		 j++)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						(g_sys_global_glossary.sys_syllable_info[i].
						 sys_phrase_two_word + j)->phrase, 6);
		printf ("pinyin_key %d %d phrase %s freq %d ",
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_two_word + j)->pinyin_key[0],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_two_word + j)->pinyin_key[1], outbuf,
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_two_word + j)->freq);
	}
	printf ("\n");
	printf ("两字词频后项链表排序\n");
	tmp =
		(void *) (g_sys_global_glossary.sys_syllable_info[i].
				  sys_phrase_two_word + 0);
	max_node = tmp;
	while (tmp)
	{
		if (((ccinPhraseTwoWordInfo_t *) max_node)->freq <
			((ccinPhraseTwoWordInfo_t *) tmp)->freq)
			max_node = tmp;
		tmp = (void *) (((ccinPhraseTwoWordInfo_t *) tmp)->pos_next);
	}
	j = 0;
	tmp = max_node;
	while (tmp)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						((ccinPhraseTwoWordInfo_t *) tmp)->phrase, 6);
		printf ("word %s freq %d ", outbuf,
				((ccinPhraseTwoWordInfo_t *) tmp)->freq);
		tmp = (void *) (((ccinPhraseTwoWordInfo_t *) tmp)->freq_next);
		j++;
	}
	printf ("\n");
	printf ("两字词频前项链表排序\n");
	tmp =
		(void *) (g_user_global_glossary.sys_syllable_info[i].
				  sys_phrase_two_word + 0);
	min_node = tmp;
	while (tmp)
	{
		if (((ccinPhraseTwoWordInfo_t *) min_node)->freq >
			((ccinPhraseTwoWordInfo_t *) tmp)->freq)
			min_node = tmp;
		tmp = (void *) (((ccinPhraseTwoWordInfo_t *) tmp)->pos_next);
	}
	j = 0;
	tmp = min_node;
	while (tmp)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						((ccinPhraseTwoWordInfo_t *) tmp)->phrase, 6);
		printf ("word %s freq %d ", outbuf,
				((ccinPhraseTwoWordInfo_t *) tmp)->freq);
		tmp = (void *) (((ccinPhraseTwoWordInfo_t *) tmp)->freq_prev);
		j++;
	}
	printf ("\n");

	printf ("三字词 %d\n",
			g_sys_global_glossary.sys_syllable_info[i].
			phrase_three_word_num);
	for (j = 0;
		 j <
		 g_sys_global_glossary.sys_syllable_info[i].phrase_three_word_num;
		 j++)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						(g_sys_global_glossary.sys_syllable_info[i].
						 sys_phrase_three_word + j)->phrase, 9);
		printf ("pinyin_key %d %d %d phrase %s freq %d ",
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_three_word + j)->pinyin_key[0],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_three_word + j)->pinyin_key[1],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_three_word + j)->pinyin_key[2], outbuf,
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_three_word + j)->freq);
	}
	printf ("\n");
	printf ("三字词频后项链表排序\n");
	tmp =
		(void *) (g_sys_global_glossary.sys_syllable_info[i].
				  sys_phrase_three_word + 0);
	max_node = tmp;
	while (tmp)
	{
		if (((ccinPhraseThreeWordInfo_t *) max_node)->freq <
			((ccinPhraseThreeWordInfo_t *) tmp)->freq)
			max_node = tmp;
		tmp = (void *) (((ccinPhraseThreeWordInfo_t *) tmp)->pos_next);
	}
	j = 0;
	tmp = max_node;
	while (tmp)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						((ccinPhraseThreeWordInfo_t *) tmp)->phrase, 9);
		printf ("word %s freq %d ", outbuf,
				((ccinPhraseThreeWordInfo_t *) tmp)->freq);
		tmp = (void *) (((ccinPhraseThreeWordInfo_t *) tmp)->freq_next);
		j++;
	}
	printf ("\n");
	printf ("三字词频前项链表排序\n");
	tmp =
		(void *) (g_user_global_glossary.sys_syllable_info[i].
				  sys_phrase_three_word + 0);
	min_node = tmp;
	while (tmp)
	{
		if (((ccinPhraseThreeWordInfo_t *) min_node)->freq >
			((ccinPhraseThreeWordInfo_t *) tmp)->freq)
			min_node = tmp;
		tmp = (void *) (((ccinPhraseThreeWordInfo_t *) tmp)->pos_next);
	}
	j = 0;
	tmp = min_node;
	while (tmp)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						((ccinPhraseThreeWordInfo_t *) tmp)->phrase, 9);
		printf ("word %s freq %d ", outbuf,
				((ccinPhraseThreeWordInfo_t *) tmp)->freq);
		tmp = (void *) (((ccinPhraseThreeWordInfo_t *) tmp)->freq_prev);
		j++;
	}
	printf ("\n");
	printf ("四字词 %d\n",
			g_sys_global_glossary.sys_syllable_info[i].phrase_four_word_num);
	for (j = 0;
		 j < g_sys_global_glossary.sys_syllable_info[i].phrase_four_word_num;
		 j++)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						(g_sys_global_glossary.sys_syllable_info[i].
						 sys_phrase_four_word + j)->phrase, 12);
		printf ("pinyin_key %d %d %d %d phrase %s freq %d",
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_four_word + j)->pinyin_key[0],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_four_word + j)->pinyin_key[1],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_four_word + j)->pinyin_key[2],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_four_word + j)->pinyin_key[3], outbuf,
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_four_word + j)->freq);
	}
	printf ("\n");
	printf ("四字词频后项链表排序\n");
	tmp =
		(void *) (g_sys_global_glossary.sys_syllable_info[i].
				  sys_phrase_four_word + 0);
	max_node = tmp;
	while (tmp)
	{
		if (((ccinPhraseFourWordInfo_t *) max_node)->freq <
			((ccinPhraseFourWordInfo_t *) tmp)->freq)
			max_node = tmp;
		tmp = (void *) (((ccinPhraseFourWordInfo_t *) tmp)->pos_next);
	}
	j = 0;
	tmp = max_node;
	while (tmp)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						((ccinPhraseFourWordInfo_t *) tmp)->phrase, 12);

		printf ("word %s freq %d ", outbuf,
				((ccinPhraseFourWordInfo_t *) tmp)->freq);
		tmp = (void *) (((ccinPhraseFourWordInfo_t *) tmp)->freq_next);
		j++;
	}
	printf ("\n");
	printf ("四字词频前项链表排序\n");
	tmp =
		(void *) (g_user_global_glossary.sys_syllable_info[i].
				  sys_phrase_four_word + 0);
	min_node = tmp;
	while (tmp)
	{
		if (((ccinPhraseFourWordInfo_t *) min_node)->freq >
			((ccinPhraseFourWordInfo_t *) tmp)->freq)
			min_node = tmp;
		tmp = (void *) (((ccinPhraseFourWordInfo_t *) tmp)->pos_next);
	}
	j = 0;
	tmp = min_node;
	while (tmp)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						((ccinPhraseFourWordInfo_t *) tmp)->phrase, 12);
		printf ("word %s freq %d ", outbuf,
				((ccinPhraseFourWordInfo_t *) tmp)->freq);
		tmp = (void *) (((ccinPhraseFourWordInfo_t *) tmp)->freq_prev);
		j++;
	}
	printf ("\n");
	printf ("四以上字词,%d\n",
			g_sys_global_glossary.sys_syllable_info[i].
			phrase_above_four_num);
	for (j = 0;
		 j <
		 g_sys_global_glossary.sys_syllable_info[i].phrase_above_four_num;
		 j++)
	{
		utf2gb_convert (inbuf, outbuf, &in, &out,
						(g_sys_global_glossary.sys_syllable_info[i].
						 sys_phrase_above_four + j)->phrase, 27);
		printf ("pinyin_key %d %d %d %d %d %d %d %d %d phrase %s ",
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_above_four + j)->pinyin_key[0],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_above_four + j)->pinyin_key[1],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_above_four + j)->pinyin_key[2],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_above_four + j)->pinyin_key[3],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_above_four + j)->pinyin_key[4],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_above_four + j)->pinyin_key[5],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_above_four + j)->pinyin_key[6],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_above_four + j)->pinyin_key[7],
				(g_sys_global_glossary.sys_syllable_info[i].
				 sys_phrase_above_four + j)->pinyin_key[8], outbuf);
	}
	printf ("\n");

	iconv_close (utf2gb);
}
#endif

#if 0
int
main (int argc, char **argv)
{
	ccin_open_imfactory ();
#ifdef DEBUG
	test ();
#endif
	ccin_close_imfactory ();
}
#endif

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
