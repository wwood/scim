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


#ifndef _CODESET_CONV_H
#define _CODESET_CONV_H

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)


//Function define
// 用 iconv() 对单个字符进行编码转换：输入 charset -> UTF-8
void ccin_locale_charset_to_UTF8_for_hanzi (u_char *, u_char *, u_int,
											ccinHanziChar_t *);
// 用 iconv() 对单个字符进行编码转换：输入 charset -> UTF-8
void ccin_UTF8_to_locale_charset_for_hanzi (u_char *, ccinHanziChar_t *,
											u_int, u_char *);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
