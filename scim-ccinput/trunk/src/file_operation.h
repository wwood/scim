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
 * ����˵��:
 * int phrase_num:��ǰ�û�����ʵ��ָ���,��:�û������Ϊ2�ִ�,phrase_num=2
 * ccinHanziChar_t *phrase:��ǰ�û�����ʵ��ַ���,��,"����"
 * u_short pinyin_key[]:��ǰ�û�����ʵ�ƴ��������,��:���ִ�����Ĵ�СΪ2,pinyin_key[0],pinyin_key[1]
 */
void ccin_add_user_phrase (int phrase_num, ccinHanziChar_t * phrase,
						   u_short pinyin_key[]);

/*
 * ����˵��:
 * ccinPhraseType_t phraseType:������ö�ٱ���,��ο�ccinput.h�еĶ���;
 * void* node:Ҫɾ���Ĵ�������.
 */
int ccin_del_user_phrase (ccinPhraseType_t phraseType, void *node);


#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
