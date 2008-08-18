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
#include <stdlib.h>
#include <string.h>

#include "ccinput.h"
#include "glossary_lookup.h"
#include "imcontext.h"


ccinIMContext_t *
ccin_initialize_context ()
{
	ccinIMContext_t *im_context;

	im_context = (ccinIMContext_t *) malloc (sizeof (ccinIMContext_t));
	if (im_context != NULL)
	{
		memset (im_context->origin_pinyin_buffer, 0,
				MAX_CHAR_IN_ORIGIN_PINYIN_STRING);
		memset (im_context->undecomposed_pinyin_buffer, 0,
				MAX_CHAR_IN_ORIGIN_PINYIN_STRING);
		memset (im_context->translated_hanzi_buffer, 0,
				MAX_SYLLABLE_IN_PHRASE * sizeof (ccinHanziChar_t));

		memset (im_context->display_pinyin_buffer, 0,
				MAX_CHAR_IN_ORIGIN_PINYIN_STRING);
		memset (im_context->pinyin_syllable_buffer, 0,
				MAX_SYLLABLE_IN_PHRASE * MAX_LETTER_IN_SYLLABLE);
		im_context->current_total_pinyin_length = 0;
		im_context->current_pinyin_position = 0;

		im_context->lookup_result = ccin_init_lookup_result ();

		im_context->flag_chinese = TRUE;	//中英切换
		im_context->flag_cn_punctuation = TRUE;	//中标点
		im_context->flag_SBC = FALSE;	//全角
		im_context->flag_traditional = FALSE;	//繁体
	}

	return im_context;
}

void
ccin_reset_context (ccinIMContext_t * im_context)
{
	if (im_context == NULL)
		return;

	memset (im_context->origin_pinyin_buffer, 0,
			MAX_CHAR_IN_ORIGIN_PINYIN_STRING);
	memset (im_context->undecomposed_pinyin_buffer, 0,
			MAX_CHAR_IN_ORIGIN_PINYIN_STRING);
	memset (im_context->translated_hanzi_buffer, 0,
			MAX_SYLLABLE_IN_PHRASE * sizeof (ccinHanziChar_t));

	memset (im_context->display_pinyin_buffer, 0,
			MAX_CHAR_IN_ORIGIN_PINYIN_STRING);
	memset (im_context->pinyin_syllable_buffer, 0,
			MAX_SYLLABLE_IN_PHRASE * MAX_LETTER_IN_SYLLABLE);
	im_context->current_total_pinyin_length = 0;
	im_context->current_pinyin_position = 0;

	ccin_reset_lookup_result (im_context->lookup_result);

	im_context->flag_chinese = TRUE;	//中英切换
	im_context->flag_cn_punctuation = TRUE;	//中标点
	im_context->flag_SBC = FALSE;	//全角
	im_context->flag_traditional = FALSE;	//繁体
}


#if 0
main ()
{
	ccinIMContext_t *aaa = ccin_initialize_context ();

	ccin_reset_context (aaa);
	free (aaa);
}
#endif

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
