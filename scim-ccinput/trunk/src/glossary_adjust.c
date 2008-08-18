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
 *  Authors: ZHANG XiaoJun <zhangxiaojun@ccoss.com.cn>
 *
 */


#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

#include <stdio.h>
#include <string.h>

#include "ccinput.h"
#include "glossary_adjust.h"


extern ccinSyllable_t g_standard_syllable_table[SYLLABLE_TOTAL];
extern u_short g_syllable_hash[LETTER_NUM][2];

u_short g_freq_adjust_GB_word_table[LETTER_NUM];
u_short g_freq_adjust_two_word_table[LETTER_NUM];
u_short g_freq_adjust_three_word_table[LETTER_NUM];
u_short g_freq_adjust_four_word_table[LETTER_NUM];


// node_ptr is the pointer of the phrase selected by user
// node_type refers to the enum type in "ccinput.h"
void
ccin_phrase_freq_adjust (void *node_ptr, ccinPhraseType_t node_type)
{
#if 0
	int i;
#endif
	ccinGBWordInfo_t *gb_word_node_first_ptr, *gb_word_node_this_ptr,
		*gb_word_node_cur_ptr;
	ccinPhraseTwoWordInfo_t *two_word_node_first_ptr,
		*two_word_node_this_ptr, *two_word_node_cur_ptr;
	ccinPhraseThreeWordInfo_t *three_word_node_first_ptr,
		*three_word_node_this_ptr, *three_word_node_cur_ptr;
	ccinPhraseFourWordInfo_t *four_word_node_first_ptr,
		*four_word_node_this_ptr, *four_word_node_cur_ptr;
#if 0
	unsigned char hanzi_string[8];
#endif

	if (node_ptr == NULL)
		return;

	switch (node_type)
	{
	case WORD_GB:
		gb_word_node_this_ptr = (ccinGBWordInfo_t *) node_ptr;
		gb_word_node_first_ptr = gb_word_node_this_ptr;
		while (gb_word_node_first_ptr->freq_prev != NULL)
			gb_word_node_first_ptr = gb_word_node_first_ptr->freq_prev;
#if 0
		//printf("Current freq: %d\n", gb_word_node_this_ptr->freq);
		printf ("Before adjust..........\n");
		i = 1;
		gb_word_node_cur_ptr = gb_word_node_first_ptr;
		while (gb_word_node_cur_ptr != NULL)
		{
			ccin_UTF8_to_locale_charset_for_hanzi ("GB18030",
												 (ccinHanziChar_t *) &
												 gb_word_node_cur_ptr->
												 word, 1, hanzi_string);
			hanzi_string[2] = '\0';
			printf ("%d.%s(%d) ", i, hanzi_string,
					gb_word_node_cur_ptr->freq);
			gb_word_node_cur_ptr = gb_word_node_cur_ptr->freq_next;
			i++;
		}
		printf ("\n");
#endif

		if ((gb_word_node_this_ptr->freq_prev == NULL)
			&& (gb_word_node_this_ptr->freq == MAX_FREQ_NUM))
		{						//1-
			/*
			 *  If this node is first node and frequency value equal MAX_FREQ_NUM
			 *  then
			 *        the word or phrase is first selected word or phrase.  Nothing do.
			 */
			;
		}						//if 1-
		else
		{						//2-
			if ((gb_word_node_this_ptr->freq_prev == NULL)
				&& (gb_word_node_this_ptr->freq != MAX_FREQ_NUM))
			{					//2-1
				/*
				 * If this node is first node and frequency value don't equal MAX)FREQ_NUM
				 * then
				 *      calculate new frequency value for first node. Nothing do.
				 */
				gb_word_node_this_ptr->freq =
					((MAX_FREQ_NUM -
					  gb_word_node_this_ptr->freq) / 2) +
					gb_word_node_this_ptr->freq + 1;
			}					//if 2-1
			else
			{					//2-2
				/*
				 * Calculate new freq.
				 */
				gb_word_node_this_ptr->freq =
					((MAX_FREQ_NUM -
					  gb_word_node_this_ptr->freq) / 2) +
					gb_word_node_this_ptr->freq + 1;
				/*
				 * In this subroute, "Goto" is better than " Loop"
				 */
gb_word_nested:
				/*
				 * At same time, Extract the gb word node from the chain list
				 */
				if (gb_word_node_this_ptr->freq_next == NULL)
				{				//2-2-1
					/*
					 * If this node is last node.
					 */
					gb_word_node_this_ptr->freq_prev->freq_next = NULL;
					gb_word_node_this_ptr->freq_prev = NULL;
					gb_word_node_this_ptr->freq_next = NULL;
				}				//if 2-2-1
				else
				{				//2-2-2
					gb_word_node_this_ptr->freq_prev->freq_next =
						gb_word_node_this_ptr->freq_next;
					gb_word_node_this_ptr->freq_next->freq_prev =
						gb_word_node_this_ptr->freq_prev;
					gb_word_node_this_ptr->freq_prev = NULL;
					gb_word_node_this_ptr->freq_next = NULL;
				}				//else 2-2-2
				/*
				 *  comp this node frequency value with node chain table. Insert this node to chain
				 *  table according to result.
				 */
				if (gb_word_node_this_ptr->freq >
					gb_word_node_first_ptr->freq)
				{				//2-2-3
					/*
					 * If freq value of this node  is bigger than  freq value of this node
					 * then
					 *   Insert this node to first. at same time, nothing do.
					 */
					gb_word_node_this_ptr->freq_next = gb_word_node_first_ptr;
					gb_word_node_first_ptr->freq_prev = gb_word_node_this_ptr;
					gb_word_node_first_ptr = gb_word_node_this_ptr;
				}				//if 2-2-3
				else			//2-2-4
				{
					if (gb_word_node_this_ptr->freq ==
						gb_word_node_first_ptr->freq)
					{			//2-2-4-1
						/*
						 * If freq value of this node equal freq value of this node
						 * then
						 *  Insert this node to first. at same time, adust other node freq value.
						 *
						 */
						gb_word_node_this_ptr->freq_next =
							gb_word_node_first_ptr;
						gb_word_node_first_ptr->freq_prev =
							gb_word_node_this_ptr;
						gb_word_node_first_ptr->freq =
							gb_word_node_first_ptr->freq - FREQ_ADJUST_VALUE;
						gb_word_node_this_ptr = gb_word_node_first_ptr;
						gb_word_node_first_ptr =
							gb_word_node_this_ptr->freq_prev;
						//
						//?????????????????????????????????????????????????????
						goto gb_word_nested;
					}			//if 2-2-4-1
					else
					{			// 2-2-4-2
						/*
						 * this node freq vlaue is littler than first node freq value
						 */
						if (gb_word_node_first_ptr->freq_next == NULL)
						{		//2-2-4-2-1
							/*
							 * If current chain table has only a node
							 * then
							 *     Insert this node after first node
							 */
							gb_word_node_first_ptr->freq_next =
								gb_word_node_this_ptr;
							gb_word_node_this_ptr->freq_prev =
								gb_word_node_first_ptr;
							gb_word_node_this_ptr->freq_next = NULL;
						}		//if 2-2-4-2-1
						else
						{		//2-2-4-2-2
							gb_word_node_cur_ptr = gb_word_node_first_ptr;
							while ((gb_word_node_this_ptr->freq <
									gb_word_node_cur_ptr->freq)
								   && (gb_word_node_this_ptr->freq <
									   gb_word_node_cur_ptr->freq_next->freq))
							{
								gb_word_node_cur_ptr =
									gb_word_node_cur_ptr->freq_next;
								if (gb_word_node_cur_ptr->freq_next == NULL)
									break;
							}
							if (gb_word_node_cur_ptr->freq_next == NULL)
							{
								gb_word_node_cur_ptr->freq_next =
									gb_word_node_this_ptr;
								gb_word_node_this_ptr->freq_prev =
									gb_word_node_cur_ptr;
								gb_word_node_this_ptr->freq_next = NULL;
							}
							else
							{	// - new
								gb_word_node_cur_ptr =
									gb_word_node_cur_ptr->freq_next;
								if (gb_word_node_this_ptr->freq >
									gb_word_node_cur_ptr->freq)
								{	//2-2-4-2-2-1
									/*
									 * If this node freq value is bigger than current node freq value ,then
									 * Insert this node before current node.
									 */
									gb_word_node_cur_ptr->freq_prev->
										freq_next = gb_word_node_this_ptr;
									gb_word_node_this_ptr->freq_prev =
										gb_word_node_cur_ptr->freq_prev;
									gb_word_node_this_ptr->freq_next =
										gb_word_node_cur_ptr;
									gb_word_node_cur_ptr->freq_prev =
										gb_word_node_this_ptr;
								}	//if 2-2-4-2-2-1
								else
								{	//2-2-4-2-2-2
									if (gb_word_node_this_ptr->freq ==
										gb_word_node_cur_ptr->freq)
									{	//2-2-4-2-2-2-1
										/*
										 *  If this node freq value equal current node freq value
										 *  then
										 *     Insert this node before current node. at same time
										 *     adjust current node freq
										 */
										gb_word_node_cur_ptr->
											freq_prev->freq_next =
											gb_word_node_this_ptr;
										gb_word_node_this_ptr->
											freq_prev =
											gb_word_node_cur_ptr->freq_prev;
										gb_word_node_this_ptr->
											freq_next = gb_word_node_cur_ptr;
										gb_word_node_cur_ptr->
											freq_prev = gb_word_node_this_ptr;
										if (gb_word_node_cur_ptr->
											freq == MIN_FREQ_NUM)
										{	//2-2-4-2-2-2-1-1
											/*
											 * If current node freq value equal MIN_FREQ_NUM ,then
											 *   nothing do.
											 */
											;
										}	//if 2-2-4-2-2-2-1-1
										else
										{	//2-2-4-2-2-2-1-2
											/*
											 * If current ndoe freq noequal MIN_FREQ_NUM,then
											 * adjust current node frequency.
											 */
											gb_word_node_cur_ptr->
												freq =
												gb_word_node_cur_ptr->
												freq - 1;
											gb_word_node_this_ptr =
												gb_word_node_cur_ptr;
											//??????????????????????????????????????????
											goto gb_word_nested;
										}	//else 2-2-4-2-2-2-1-2
									}	//if 2-2-4-2-2-2-1
								}	//else 2-2-4-2-2-2
							}	// -new
						}		//else 2-2-4-2-2
					}			//else 2-2-4-2
				}				//else 2-2-4
			}					//else 2-2
		}						//else 2-
#if 0
		printf ("After adjust.......\n");
		i = 1;
		gb_word_node_cur_ptr = gb_word_node_first_ptr;
		while (gb_word_node_cur_ptr != NULL)
		{
			ccin_UTF8_to_locale_charset_for_hanzi ("GB18030",
												 (ccinHanziChar_t *) &
												 gb_word_node_cur_ptr->
												 word, 1, hanzi_string);
			hanzi_string[2] = '\0';
			printf ("%d.%s(%d) ", i, hanzi_string,
					gb_word_node_cur_ptr->freq);
			gb_word_node_cur_ptr = gb_word_node_cur_ptr->freq_next;
			i++;
		}
		printf ("\n");
#endif
		break;

	case PHRASE_TWO:
		two_word_node_this_ptr = (ccinPhraseTwoWordInfo_t *) node_ptr;
		two_word_node_first_ptr = two_word_node_this_ptr;
		while (two_word_node_first_ptr->freq_prev != NULL)
			two_word_node_first_ptr = two_word_node_first_ptr->freq_prev;
#if 0
		//printf("Current freq: %d\n", two_word_node_this_ptr->freq);
		printf ("Before adjust..........\n");
		i = 1;
		two_word_node_cur_ptr = two_word_node_first_ptr;
		while (two_word_node_cur_ptr != NULL)
		{
			ccin_UTF8_to_locale_charset_for_hanzi ("GB18030",
												 (ccinHanziChar_t *) &
												 two_word_node_cur_ptr->
												 phrase, 2, hanzi_string);
			hanzi_string[4] = '\0';
			printf ("%d.%s(%d) ", i, hanzi_string,
					two_word_node_cur_ptr->freq);
			two_word_node_cur_ptr = two_word_node_cur_ptr->freq_next;
			i++;
		}
		printf ("\n");
#endif

		if ((two_word_node_this_ptr->freq_prev == NULL)
			&& (two_word_node_this_ptr->freq == MAX_FREQ_NUM))
		{						//1-
			/*
			 *  If this node is first node and frequency value equal MAX_FREQ_NUM
			 *  then
			 *        the word or phrase is first selected word or phrase.  Nothing do.
			 */
			;
		}						//if 1-
		else
		{						//2-
			if ((two_word_node_this_ptr->freq_prev == NULL)
				&& (two_word_node_this_ptr->freq != MAX_FREQ_NUM))
			{					//2-1
				/*
				 * If this node is first node and frequency value don't equal MAX)FREQ_NUM
				 * then
				 *      calculate new frequency value for first node. Nothing do.
				 */
				two_word_node_this_ptr->freq =
					((MAX_FREQ_NUM -
					  two_word_node_this_ptr->freq) / 2) +
					two_word_node_this_ptr->freq + 1;
			}					//if 2-1
			else
			{					//2-2
				/*
				 * Calculate new freq.
				 */
				two_word_node_this_ptr->freq =
					((MAX_FREQ_NUM -
					  two_word_node_this_ptr->freq) / 2) +
					two_word_node_this_ptr->freq + 1;
				/*
				 * In this subroute, "Goto" is better than " Loop"
				 */
two_word_nested:
				/*
				 * At same time, Extract the two word node from the chain list
				 */
				if (two_word_node_this_ptr->freq_next == NULL)
				{				//2-2-1
					/*
					 * If this node is last node.
					 */
					two_word_node_this_ptr->freq_prev->freq_next = NULL;
					two_word_node_this_ptr->freq_prev = NULL;
					two_word_node_this_ptr->freq_next = NULL;
				}				//if 2-2-1
				else
				{				//2-2-2
					two_word_node_this_ptr->freq_prev->freq_next =
						two_word_node_this_ptr->freq_next;
					two_word_node_this_ptr->freq_next->freq_prev =
						two_word_node_this_ptr->freq_prev;
					two_word_node_this_ptr->freq_prev = NULL;
					two_word_node_this_ptr->freq_next = NULL;
				}				//else 2-2-2
				/*
				 *  comp this node frequency value with node chain table. Insert this node to chain
				 *  table according to result.
				 */
				if (two_word_node_this_ptr->freq >
					two_word_node_first_ptr->freq)
				{				//2-2-3
					/*
					 * If freq value of this node  is bigger than  freq value of this node
					 * then
					 *   Insert this node to first. at same time, nothing do.
					 */
					two_word_node_this_ptr->freq_next =
						two_word_node_first_ptr;
					two_word_node_first_ptr->freq_prev =
						two_word_node_this_ptr;
					two_word_node_first_ptr = two_word_node_this_ptr;
				}				//if 2-2-3
				else			//2-2-4
				{
					if (two_word_node_this_ptr->freq ==
						two_word_node_first_ptr->freq)
					{			//2-2-4-1
						/*
						 * If freq value of this node equal freq value of this node
						 * then
						 *  Insert this node to first. at same time, adust other node freq value.
						 *
						 */
						two_word_node_this_ptr->freq_next =
							two_word_node_first_ptr;
						two_word_node_first_ptr->freq_prev =
							two_word_node_this_ptr;
						two_word_node_first_ptr->freq =
							two_word_node_first_ptr->freq - FREQ_ADJUST_VALUE;
						two_word_node_this_ptr = two_word_node_first_ptr;
						two_word_node_first_ptr =
							two_word_node_this_ptr->freq_prev;
						//
						//?????????????????????????????????????????????????????
						goto two_word_nested;
					}			//if 2-2-4-1
					else
					{			// 2-2-4-2
						/*
						 * this node freq vlaue is littler than first node freq value
						 */
						if (two_word_node_first_ptr->freq_next == NULL)
						{		//2-2-4-2-1
							/*
							 * If current chain table has only a node
							 * then
							 *     Insert this node after first node
							 */
							two_word_node_first_ptr->freq_next =
								two_word_node_this_ptr;
							two_word_node_this_ptr->freq_prev =
								two_word_node_first_ptr;
							two_word_node_this_ptr->freq_next = NULL;
						}		//if 2-2-4-2-1
						else
						{		//2-2-4-2-2
							two_word_node_cur_ptr = two_word_node_first_ptr;
							while ((two_word_node_this_ptr->freq <
									two_word_node_cur_ptr->freq)
								   && (two_word_node_this_ptr->freq <
									   two_word_node_cur_ptr->
									   freq_next->freq))
							{
								two_word_node_cur_ptr =
									two_word_node_cur_ptr->freq_next;
								if (two_word_node_cur_ptr->freq_next == NULL)
									break;
							}
							if (two_word_node_cur_ptr->freq_next == NULL)
							{
								two_word_node_cur_ptr->freq_next =
									two_word_node_this_ptr;
								two_word_node_this_ptr->freq_prev =
									two_word_node_cur_ptr;
								two_word_node_this_ptr->freq_next = NULL;
							}
							else
							{	//-new
								two_word_node_cur_ptr =
									two_word_node_cur_ptr->freq_next;
								if (two_word_node_this_ptr->freq >
									two_word_node_cur_ptr->freq)
								{	//2-2-4-2-2-1
									/*
									 * If this node freq value is bigger than current node freq value ,then
									 * Insert this node before current node.
									 */
									two_word_node_cur_ptr->freq_prev->
										freq_next = two_word_node_this_ptr;
									two_word_node_this_ptr->freq_prev =
										two_word_node_cur_ptr->freq_prev;
									two_word_node_this_ptr->freq_next =
										two_word_node_cur_ptr;
									two_word_node_cur_ptr->freq_prev =
										two_word_node_this_ptr;
								}	//if 2-2-4-2-2-1
								else
								{	//2-2-4-2-2-2
									if (two_word_node_this_ptr->freq ==
										two_word_node_cur_ptr->freq)
									{	//2-2-4-2-2-2-1
										/*
										 *  If this node freq value equal current node freq value
										 *  then
										 *     Insert this node before current node. at same time
										 *     adjust current node freq
										 */
										two_word_node_cur_ptr->
											freq_prev->freq_next =
											two_word_node_this_ptr;
										two_word_node_this_ptr->
											freq_prev =
											two_word_node_cur_ptr->freq_prev;
										two_word_node_this_ptr->
											freq_next = two_word_node_cur_ptr;
										two_word_node_cur_ptr->
											freq_prev =
											two_word_node_this_ptr;
										if (two_word_node_cur_ptr->
											freq == MIN_FREQ_NUM)
										{	//2-2-4-2-2-2-1-1
											/*
											 * If current node freq value equal MIN_FREQ_NUM ,then
											 *   nothing do.
											 */
											;
										}	//if 2-2-4-2-2-2-1-1
										else
										{	//2-2-4-2-2-2-1-2
											/*
											 * If current ndoe freq noequal MIN_FREQ_NUM,then
											 * adjust current node frequency.
											 */
											two_word_node_cur_ptr->
												freq =
												two_word_node_cur_ptr->
												freq - 1;
											two_word_node_this_ptr =
												two_word_node_cur_ptr;
											//??????????????????????????????????????????
											goto two_word_nested;
										}	//else 2-2-4-2-2-2-1-2
									}	//if 2-2-4-2-2-2-1
								}	//else 2-2-4-2-2-2
							}	// - new
						}		//else 2-2-4-2-2
					}			//else 2-2-4-2
				}				//else 2-2-4
			}					//else 2-2
		}						//else 2-
#if 0
		printf ("After adjust.......\n");
		i = 1;
		two_word_node_cur_ptr = two_word_node_first_ptr;
		while (two_word_node_cur_ptr != NULL)
		{
			ccin_UTF8_to_locale_charset_for_hanzi ("GB18030",
												 (ccinHanziChar_t *) &
												 two_word_node_cur_ptr->
												 phrase, 2, hanzi_string);
			hanzi_string[4] = '\0';
			printf ("%d.%s(%d) ", i, hanzi_string,
					two_word_node_cur_ptr->freq);
			two_word_node_cur_ptr = two_word_node_cur_ptr->freq_next;
			i++;
		}
		printf ("\n");
#endif
		break;

	case PHRASE_THREE:
		three_word_node_this_ptr = (ccinPhraseThreeWordInfo_t *) node_ptr;
		three_word_node_first_ptr = three_word_node_this_ptr;
		while (three_word_node_first_ptr->freq_prev != NULL)
			three_word_node_first_ptr = three_word_node_first_ptr->freq_prev;
#if 0
		//printf("Current freq: %d\n", three_word_node_this_ptr->freq);
		printf ("Before adjust..........\n");
		i = 1;
		three_word_node_cur_ptr = three_word_node_first_ptr;
		while (three_word_node_cur_ptr != NULL)
		{
			ccin_UTF8_to_locale_charset_for_hanzi ("GB18030",
												 (ccinHanziChar_t *) &
												 three_word_node_cur_ptr->
												 phrase, 3, hanzi_string);
			hanzi_string[6] = '\0';
			printf ("%d.%s(%d) ", i, hanzi_string,
					three_word_node_cur_ptr->freq);
			three_word_node_cur_ptr = three_word_node_cur_ptr->freq_next;
			i++;
		}
		printf ("\n");
#endif

		if ((three_word_node_this_ptr->freq_prev == NULL)
			&& (three_word_node_this_ptr->freq == MAX_FREQ_NUM))
		{						//1-
			/*
			 *  If this node is first node and frequency value equal MAX_FREQ_NUM
			 *  then
			 *        the word or phrase is first selected word or phrase.  Nothing do.
			 */
			;
		}						//if 1-
		else
		{						//2-
			if ((three_word_node_this_ptr->freq_prev == NULL)
				&& (three_word_node_this_ptr->freq != MAX_FREQ_NUM))
			{					//2-1
				/*
				 * If this node is first node and frequency value don't equal MAX)FREQ_NUM
				 * then
				 *      calculate new frequency value for first node. Nothing do.
				 */
				three_word_node_this_ptr->freq =
					((MAX_FREQ_NUM -
					  three_word_node_this_ptr->freq) / 2) +
					three_word_node_this_ptr->freq + 1;
			}					//if 2-1
			else
			{					//2-2
				/*
				 * Calculate new freq.
				 */
				three_word_node_this_ptr->freq =
					((MAX_FREQ_NUM -
					  three_word_node_this_ptr->freq) / 2) +
					three_word_node_this_ptr->freq + 1;
				/*
				 * In this subroute, "Goto" is better than " Loop"
				 */
three_word_nested:
				/*
				 * At same time, Extract the two word node from the chain list
				 */
				if (three_word_node_this_ptr->freq_next == NULL)
				{				//2-2-1
					/*
					 * If this node is last node.
					 */
					three_word_node_this_ptr->freq_prev->freq_next = NULL;
					three_word_node_this_ptr->freq_prev = NULL;
					three_word_node_this_ptr->freq_next = NULL;
				}				//if 2-2-1
				else
				{				//2-2-2
					three_word_node_this_ptr->freq_prev->freq_next =
						three_word_node_this_ptr->freq_next;
					three_word_node_this_ptr->freq_next->freq_prev =
						three_word_node_this_ptr->freq_prev;
					three_word_node_this_ptr->freq_prev = NULL;
					three_word_node_this_ptr->freq_next = NULL;
				}				//else 2-2-2
				/*
				 *  comp this node frequency value with node chain table. Insert this node to chain
				 *  table according to result.
				 */
				if (three_word_node_this_ptr->freq >
					three_word_node_first_ptr->freq)
				{				//2-2-3
					/*
					 * If freq value of this node  is bigger than  freq value of this node
					 * then
					 *   Insert this node to first. at same time, nothing do.
					 */
					three_word_node_this_ptr->freq_next =
						three_word_node_first_ptr;
					three_word_node_first_ptr->freq_prev =
						three_word_node_this_ptr;
					three_word_node_first_ptr = three_word_node_this_ptr;
				}				//if 2-2-3
				else			//2-2-4
				{
					if (three_word_node_this_ptr->freq ==
						three_word_node_first_ptr->freq)
					{			//2-2-4-1
						/*
						 * If freq value of this node equal freq value of this node
						 * then
						 *  Insert this node to first. at same time, adust other node freq value.
						 *
						 */
						three_word_node_this_ptr->freq_next =
							three_word_node_first_ptr;
						three_word_node_first_ptr->freq_prev =
							three_word_node_this_ptr;
						three_word_node_first_ptr->freq =
							three_word_node_first_ptr->freq -
							FREQ_ADJUST_VALUE;
						three_word_node_this_ptr = three_word_node_first_ptr;
						three_word_node_first_ptr =
							three_word_node_this_ptr->freq_prev;
						//
						//?????????????????????????????????????????????????????
						goto three_word_nested;
					}			//if 2-2-4-1
					else
					{			// 2-2-4-2
						/*
						 * this node freq vlaue is littler than first node freq value
						 */
						if (three_word_node_first_ptr->freq_next == NULL)
						{		//2-2-4-2-1
							/*
							 * If current chain table has only a node
							 * then
							 *     Insert this node after first node
							 */
							three_word_node_first_ptr->freq_next =
								three_word_node_this_ptr;
							three_word_node_this_ptr->freq_prev =
								three_word_node_first_ptr;
							three_word_node_this_ptr->freq_next = NULL;
						}		//if 2-2-4-2-1
						else
						{		//2-2-4-2-2
							three_word_node_cur_ptr =
								three_word_node_first_ptr;
							while ((three_word_node_this_ptr->freq <
									three_word_node_cur_ptr->freq)
								   && (three_word_node_this_ptr->freq <
									   three_word_node_cur_ptr->
									   freq_next->freq))
							{
								three_word_node_cur_ptr =
									three_word_node_cur_ptr->freq_next;
								if (three_word_node_cur_ptr->
									freq_next == NULL)
									break;
							}
							if (three_word_node_cur_ptr->freq_next == NULL)
							{
								three_word_node_cur_ptr->freq_next =
									three_word_node_this_ptr;
								three_word_node_this_ptr->freq_prev =
									three_word_node_cur_ptr;
								three_word_node_this_ptr->freq_next = NULL;
							}
							else
							{	// - new
								three_word_node_cur_ptr =
									three_word_node_cur_ptr->freq_next;
								if (three_word_node_this_ptr->freq >
									three_word_node_cur_ptr->freq)
								{	//2-2-4-2-2-1
									/*
									 * If this node freq value is bigger than current node freq value ,then
									 * Insert this node before current node.
									 */
									three_word_node_cur_ptr->
										freq_prev->freq_next =
										three_word_node_this_ptr;
									three_word_node_this_ptr->
										freq_prev =
										three_word_node_cur_ptr->freq_prev;
									three_word_node_this_ptr->
										freq_next = three_word_node_cur_ptr;
									three_word_node_cur_ptr->
										freq_prev = three_word_node_this_ptr;
								}	//if 2-2-4-2-2-1
								else
								{	//2-2-4-2-2-2
									if (three_word_node_this_ptr->
										freq == three_word_node_cur_ptr->freq)
									{	//2-2-4-2-2-2-1
										/*
										 *  If this node freq value equal current node freq value
										 *  then
										 *     Insert this node before current node. at same time
										 *     adjust current node freq
										 */
										three_word_node_cur_ptr->
											freq_prev->freq_next =
											three_word_node_this_ptr;
										three_word_node_this_ptr->
											freq_prev =
											three_word_node_cur_ptr->
											freq_prev;
										three_word_node_this_ptr->
											freq_next =
											three_word_node_cur_ptr;
										three_word_node_cur_ptr->
											freq_prev =
											three_word_node_this_ptr;
										if (three_word_node_cur_ptr->
											freq == MIN_FREQ_NUM)
										{	//2-2-4-2-2-2-1-1
											/*
											 * If current node freq value equal MIN_FREQ_NUM ,then
											 *   nothing do.
											 */
											;
										}	//if 2-2-4-2-2-2-1-1
										else
										{	//2-2-4-2-2-2-1-2
											/*
											 * If current ndoe freq noequal MIN_FREQ_NUM,then
											 * adjust current node frequency.
											 */
											three_word_node_cur_ptr->
												freq =
												three_word_node_cur_ptr->
												freq - 1;
											three_word_node_this_ptr =
												three_word_node_cur_ptr;
											//??????????????????????????????????????????
											goto three_word_nested;
										}	//else 2-2-4-2-2-2-1-2
									}	//if 2-2-4-2-2-2-1
								}	//else 2-2-4-2-2-2
							}	//else - new
						}		//else 2-2-4-2-2
					}			//else 2-2-4-2
				}				//else 2-2-4
			}					//else 2-2
		}						//else 2-
#if 0
		printf ("After adjust.......\n");
		i = 1;
		three_word_node_cur_ptr = three_word_node_first_ptr;
		while (three_word_node_cur_ptr != NULL)
		{
			ccin_UTF8_to_locale_charset_for_hanzi ("GB18030",
												 (ccinHanziChar_t *) &
												 three_word_node_cur_ptr->
												 phrase, 3, hanzi_string);
			hanzi_string[6] = '\0';
			printf ("%d.%s(%d) ", i, hanzi_string,
					three_word_node_cur_ptr->freq);
			three_word_node_cur_ptr = three_word_node_cur_ptr->freq_next;
			i++;
		}
		printf ("\n");
#endif

		break;

	case PHRASE_FOUR:
		four_word_node_this_ptr = (ccinPhraseFourWordInfo_t *) node_ptr;
		four_word_node_first_ptr = four_word_node_this_ptr;
		while (four_word_node_first_ptr->freq_prev != NULL)
			four_word_node_first_ptr = four_word_node_first_ptr->freq_prev;
#if 0
		//printf("Current freq: %d\n", four_word_node_this_ptr->freq);
		printf ("Before adjust..........\n");
		i = 1;
		four_word_node_cur_ptr = four_word_node_first_ptr;
		while (four_word_node_cur_ptr != NULL)
		{
			ccin_UTF8_to_locale_charset_for_hanzi ("GB18030",
												 (ccinHanziChar_t *) &
												 four_word_node_cur_ptr->
												 phrase, 4, hanzi_string);
			hanzi_string[8] = '\0';
			printf ("%d.%s(%d) ", i, hanzi_string,
					four_word_node_cur_ptr->freq);
			four_word_node_cur_ptr = four_word_node_cur_ptr->freq_next;
			i++;
		}
		printf ("\n");
#endif

		if ((four_word_node_this_ptr->freq_prev == NULL)
			&& (four_word_node_this_ptr->freq == MAX_FREQ_NUM))
		{						//1-
			/*
			 *  If this node is first node and frequency value equal MAX_FREQ_NUM
			 *  then
			 *        the word or phrase is first selected word or phrase.  Nothing do.
			 */
			;
		}						//if 1-
		else
		{						//2-
			if ((four_word_node_this_ptr->freq_prev == NULL)
				&& (four_word_node_this_ptr->freq != MAX_FREQ_NUM))
			{					//2-1
				/*
				 * If this node is first node and frequency value don't equal MAX)FREQ_NUM
				 * then
				 *      calculate new frequency value for first node. Nothing do.
				 */
				four_word_node_this_ptr->freq =
					((MAX_FREQ_NUM -
					  four_word_node_this_ptr->freq) / 2) +
					four_word_node_this_ptr->freq + 1;
			}					//if 2-1
			else
			{					//2-2
				/*
				 * Calculate new freq.
				 */
				four_word_node_this_ptr->freq =
					((MAX_FREQ_NUM -
					  four_word_node_this_ptr->freq) / 2) +
					four_word_node_this_ptr->freq + 1;
				/*
				 * In this subroute, "Goto" is better than " Loop"
				 */
four_word_nested:
				/*
				 * At same time, Extract the four word node from the chain list
				 */
				if (four_word_node_this_ptr->freq_next == NULL)
				{				//2-2-1
					/*
					 * If this node is last node.
					 */
					four_word_node_this_ptr->freq_prev->freq_next = NULL;
					four_word_node_this_ptr->freq_prev = NULL;
					four_word_node_this_ptr->freq_next = NULL;
				}				//if 2-2-1
				else
				{				//2-2-2
					four_word_node_this_ptr->freq_prev->freq_next =
						four_word_node_this_ptr->freq_next;
					four_word_node_this_ptr->freq_next->freq_prev =
						four_word_node_this_ptr->freq_prev;
					four_word_node_this_ptr->freq_prev = NULL;
					four_word_node_this_ptr->freq_next = NULL;
				}				//else 2-2-2
				/*
				 *  comp this node frequency value with node chain table. Insert this node to chain
				 *  table according to result.
				 */
				if (four_word_node_this_ptr->freq >
					four_word_node_first_ptr->freq)
				{				//2-2-3
					/*
					 * If freq value of this node  is bigger than  freq value of this node
					 * then
					 *   Insert this node to first. at same time, nothing do.
					 */
					four_word_node_this_ptr->freq_next =
						four_word_node_first_ptr;
					four_word_node_first_ptr->freq_prev =
						four_word_node_this_ptr;
					four_word_node_first_ptr = four_word_node_this_ptr;
				}				//if 2-2-3
				else			//2-2-4
				{
					if (four_word_node_this_ptr->freq ==
						four_word_node_first_ptr->freq)
					{			//2-2-4-1
						/*
						 * If freq value of this node equal freq value of first node
						 * then
						 *  Insert this node to first. at same time, adust other node freq value.
						 *
						 */
						four_word_node_this_ptr->freq_next =
							four_word_node_first_ptr;
						four_word_node_first_ptr->freq_prev =
							four_word_node_this_ptr;
						four_word_node_first_ptr->freq =
							four_word_node_first_ptr->freq -
							FREQ_ADJUST_VALUE;
						four_word_node_this_ptr = four_word_node_first_ptr;
						four_word_node_first_ptr =
							four_word_node_this_ptr->freq_prev;
						//
						//?????????????????????????????????????????????????????
						goto four_word_nested;
					}			//if 2-2-4-1
					else
					{			// 2-2-4-2
						/*
						 * this node freq vlaue is littler than first node freq value
						 */
						if (four_word_node_first_ptr->freq_next == NULL)
						{		//2-2-4-2-1
							/*
							 * If current chain table has only a node
							 * then
							 *     Insert this node after first node
							 */
							four_word_node_first_ptr->freq_next =
								four_word_node_this_ptr;
							four_word_node_this_ptr->freq_prev =
								four_word_node_first_ptr;
							four_word_node_this_ptr->freq_next = NULL;
						}		//if 2-2-4-2-1
						else
						{		//2-2-4-2-2
							four_word_node_cur_ptr = four_word_node_first_ptr;
							while ((four_word_node_this_ptr->freq <
									four_word_node_cur_ptr->freq)
								   && (four_word_node_this_ptr->freq <
									   four_word_node_cur_ptr->
									   freq_next->freq))
							{
								four_word_node_cur_ptr =
									four_word_node_cur_ptr->freq_next;
								if (four_word_node_cur_ptr->freq_next == NULL)
									break;

							}
							if (four_word_node_cur_ptr->freq_next == NULL)
							{
								four_word_node_cur_ptr->freq_next =
									four_word_node_this_ptr;
								four_word_node_this_ptr->freq_prev =
									four_word_node_cur_ptr;
								four_word_node_this_ptr->freq_next = NULL;
							}
							else
							{	// - new
								four_word_node_cur_ptr =
									four_word_node_cur_ptr->freq_next;
								if (four_word_node_this_ptr->freq >
									four_word_node_cur_ptr->freq)
								{	// 2-2-4-2-2-1
									/*
									 * If this node freq value is bigger than current node freq value ,then
									 * Insert this node before current node.
									 */
									four_word_node_cur_ptr->freq_prev->
										freq_next = four_word_node_this_ptr;
									four_word_node_this_ptr->
										freq_prev =
										four_word_node_cur_ptr->freq_prev;
									four_word_node_this_ptr->
										freq_next = four_word_node_cur_ptr;
									four_word_node_cur_ptr->freq_prev =
										four_word_node_this_ptr;
								}	//if 2-2-4-2-2-1
								else
								{	//2-2-4-2-2-2
									if (four_word_node_this_ptr->
										freq == four_word_node_cur_ptr->freq)
									{	//2-2-4-2-2-2-1
										/*
										 *  If this node freq value equal current node freq value
										 *  then
										 *     Insert this node before current node. at same time
										 *     adjust current node freq
										 */
										four_word_node_cur_ptr->
											freq_prev->freq_next =
											four_word_node_this_ptr;
										four_word_node_this_ptr->
											freq_prev =
											four_word_node_cur_ptr->freq_prev;
										four_word_node_this_ptr->
											freq_next =
											four_word_node_cur_ptr;
										four_word_node_cur_ptr->
											freq_prev =
											four_word_node_this_ptr;
										if (four_word_node_cur_ptr->
											freq == MIN_FREQ_NUM)
										{	//2-2-4-2-2-2-1-1
											/*
											 * If current node freq value equal MIN_FREQ_NUM ,then
											 *   nothing do.
											 */
											;
										}	//if 2-2-4-2-2-2-1-1
										else
										{	//2-2-4-2-2-2-1-2
											/*
											 * If current ndoe freq noequal MIN_FREQ_NUM,then
											 * adjust current node frequency.
											 */
											four_word_node_cur_ptr->
												freq =
												four_word_node_cur_ptr->
												freq - 1;
											four_word_node_this_ptr =
												four_word_node_cur_ptr;
											//??????????????????????????????????????????
											goto four_word_nested;
										}	//else 2-2-4-2-2-2-1-2
									}	//if 2-2-4-2-2-2-1
								}	//else 2-2-4-2-2-2
							}	//else - new
						}		//else 2-2-4-2-2
					}			//else 2-2-4-2
				}				//else 2-2-4
			}					//else 2-2
		}						//else 2-
#if 0
		printf ("After adjust.......\n");
		i = 1;
		four_word_node_cur_ptr = four_word_node_first_ptr;
		while (four_word_node_cur_ptr != NULL)
		{
			ccin_UTF8_to_locale_charset_for_hanzi ("GB18030",
												 (ccinHanziChar_t *) &
												 four_word_node_cur_ptr->
												 phrase, 4, hanzi_string);
			hanzi_string[8] = '\0';
			printf ("%d.%s(%d) ", i, hanzi_string,
					four_word_node_cur_ptr->freq);
			four_word_node_cur_ptr = four_word_node_cur_ptr->freq_next;
			i++;
		}
		printf ("\n");
#endif

		break;
	default:
//      printf("Node type error!\n");
		break;
	}							//switch
}


// node_type refers to the enum type in "ccinput.h"
void
ccin_phrase_freq_adjust_again (void *node_ptr, ccinPhraseType_t node_type)
{
	u_int letter_index;
	ccinGBWordInfo_t *gb_word_node_ptr;
	ccinPhraseTwoWordInfo_t *two_word_node_ptr;
	ccinPhraseThreeWordInfo_t *three_word_node_ptr;
	ccinPhraseFourWordInfo_t *four_word_node_ptr;

	switch (node_type)
	{
	case WORD_GB:
		gb_word_node_ptr = (ccinGBWordInfo_t *) node_ptr;
		if (gb_word_node_ptr->freq == MAX_FREQ_NUM)
		{
			letter_index =
				ccin_get_syllable_first_letter_index (gb_word_node_ptr->
													pinyin_key);
			g_freq_adjust_GB_word_table[letter_index] =
				gb_word_node_ptr->pinyin_key;
//          printf("letter_index: %d\n",letter_index);
//          printf("pinyin_key: %d\n",gb_word_node_ptr->pinyin_key);
		}
		break;

	case PHRASE_TWO:
		two_word_node_ptr = (ccinPhraseTwoWordInfo_t *) node_ptr;
		if (two_word_node_ptr->freq == MAX_FREQ_NUM)
		{
			letter_index =
				ccin_get_syllable_first_letter_index (two_word_node_ptr->
													pinyin_key[0]);
			g_freq_adjust_two_word_table[letter_index] =
				two_word_node_ptr->pinyin_key[0];
//          printf("letter_index: %d\n",letter_index);
//          printf("pinyin_key: %d\n",two_word_node_ptr->pinyin_key[0]);
		}
		break;

	case PHRASE_THREE:
		three_word_node_ptr = (ccinPhraseThreeWordInfo_t *) node_ptr;
		if (three_word_node_ptr->freq == MAX_FREQ_NUM)
		{
			letter_index =
				ccin_get_syllable_first_letter_index
				(three_word_node_ptr->pinyin_key[0]);
			g_freq_adjust_three_word_table[letter_index] =
				three_word_node_ptr->pinyin_key[0];
//          printf("letter_index: %d\n",letter_index);
//          printf("pinyin_key: %d\n",three_word_node_ptr->pinyin_key[0]);
		}
		break;

	case PHRASE_FOUR:
		four_word_node_ptr = (ccinPhraseFourWordInfo_t *) node_ptr;
		if (four_word_node_ptr->freq == MAX_FREQ_NUM)
		{
			letter_index =
				ccin_get_syllable_first_letter_index
				(four_word_node_ptr->pinyin_key[0]);
			g_freq_adjust_four_word_table[letter_index] =
				four_word_node_ptr->pinyin_key[0];
//          printf("letter_index: %d\n",letter_index);
//          printf("pinyin_key: %d\n",four_word_node_ptr->pinyin_key[0]);
		}
		break;

	default:
//      printf("Node type error!\n");
		break;
	}
}

char
ccin_get_syllable_first_letter_index (u_short pinyin_key)
{
	if (pinyin_key > SYLLABLE_TOTAL)
		return -1;

	u_char ret_value = g_standard_syllable_table[pinyin_key - 1][0];
	u_char temp_value = '\0';

	if (strlen (g_standard_syllable_table[pinyin_key - 1]) > 1)
		temp_value = g_standard_syllable_table[pinyin_key - 1][1];
	//printf("pinyin: %c\n",g_standard_syllable_table[pinyin_key - 1][1]);
	if (temp_value == 'h')
	{
		if (ret_value == 'c')
		{
			ret_value += 6;
		}
		else if (ret_value == 's')
		{
			ret_value += 2;
		}
		else if (ret_value == 'z')
		{
			ret_value -= 4;
		}
	}
	ret_value -= 'a';

	return ret_value;
}


void
ccin_init_freq_adjust_table ()
{
	int i;

	for (i = 'a'; i <= 'z'; i++)
	{
		g_freq_adjust_GB_word_table[i - 'a'] = g_syllable_hash[i - 'a'][0];
		g_freq_adjust_two_word_table[i - 'a'] = g_syllable_hash[i - 'a'][0];
		g_freq_adjust_three_word_table[i - 'a'] = g_syllable_hash[i - 'a'][0];
		g_freq_adjust_four_word_table[i - 'a'] = g_syllable_hash[i - 'a'][0];
	}
}

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
