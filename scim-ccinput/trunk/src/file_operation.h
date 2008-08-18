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


#ifndef _FILE_OPERATION_H
#define _FILE_OPERATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

void ccin_open_imfactory ();
void ccin_save_system_frequency ();
void ccin_save_user_frequency ();
void ccin_save_user_glossary ();
void ccin_close_imfactory ();

/*
 * 参数说明:
 * int phrase_num:当前用户自造词的字个数,如:用户自造词为2字词,phrase_num=2
 * ccinHanziChar_t *phrase:当前用户自造词的字符串,如,"天下"
 * u_short pinyin_key[]:当前用户自造词的拼音码数组,如:两字词数组的大小为2,pinyin_key[0],pinyin_key[1]
 */
void ccin_add_user_phrase (int phrase_num, ccinHanziChar_t * phrase,
						   u_short pinyin_key[]);

/*
 * 参数说明:
 * ccinPhraseType_t phraseType:词类型枚举变量,请参看ccinput.h中的定义;
 * void* node:要删除的词链表结点.
 */
int ccin_del_user_phrase (ccinPhraseType_t phraseType, void *node);


#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
