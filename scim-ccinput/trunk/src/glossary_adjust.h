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


#ifndef _GLOSSARY_ADJUST_H
#define _GLOSSARY_ADJUST_H

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

#include "ccinput.h"

void ccin_phrase_freq_adjust (void *node_ptr, ccinPhraseType_t node_type);
void ccin_phrase_freq_adjust_again (void *node_ptr, ccinPhraseType_t node_type);

char ccin_get_syllable_first_letter_index (u_short pinyin_key);
void ccin_init_freq_adjust_table ();

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
