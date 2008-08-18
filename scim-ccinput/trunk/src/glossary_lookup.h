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


#ifndef _GLOSSARY_LOOKUP_H
#define _GLOSSARY_LOOKUP_H

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

#include "ccinput.h"

void ccin_find_matching_results (ccinSyllable_t pinyin_array_to_lookup[],
								 u_short pinyin_total,
								 ccinLookupResult_t * result_phrase,
								 u_long flag_fuzzy);

ccinLookupResult_t *ccin_init_lookup_result ();
void ccin_reset_lookup_result (ccinLookupResult_t *);

int ccin_gb_word_cmp (ccinGBWordInfo_t *, ccinGBWordInfo_t *);
int ccin_two_word_phrase_cmp (ccinPhraseTwoWordInfo_t *,
							  ccinPhraseTwoWordInfo_t *);
int ccin_three_word_phrase_cmp (ccinPhraseThreeWordInfo_t *,
								ccinPhraseThreeWordInfo_t *);
int ccin_four_word_phrase_cmp (ccinPhraseFourWordInfo_t *,
							   ccinPhraseFourWordInfo_t *);
int ccin_long_phrase_cmp (ccinLongPhraseInfo_t *, ccinLongPhraseInfo_t *);

int ccin_is_phrase_existed_in_glossary (u_short word_number,
										ccinHanziChar_t * phrase,
										u_short * pinyin_key);

int ccin_str_right_n_compare (const char *str1, const char *str2, size_t num);


#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
