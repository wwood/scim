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

#include <stdio.h>
#include <iconv.h>
#include <errno.h>
#include <stdlib.h>

#include "datatype.h"
#include "codeset_conv.h"


// 用 iconv() 对单个字符进行编码转换：输入 charset -> UTF-8
void
ccin_locale_charset_to_UTF8_for_hanzi (u_char * locale_charset,
									   u_char * hanzi_charset_string,
									   u_int hanzi_charset_string_count,
									   ccinHanziChar_t * hanzi_utf8_string)
{
	iconv_t cd;
	u_int hanzi_charset_string_len = hanzi_charset_string_count * 2;
	char *utf8_string_buf = (char *) hanzi_utf8_string[0];
	u_int utf8_string_len_left = hanzi_charset_string_count * 3;;

	// 打开 charset -> UTF-8 的转换描述符
	cd = iconv_open ("UTF-8", locale_charset);
	if (cd == (iconv_t) (-1))
	{
		perror ("iconv_open");
		exit (1);
	}
	iconv (cd, (char **) &hanzi_charset_string,
		   &hanzi_charset_string_len, (char **) &utf8_string_buf,
		   &utf8_string_len_left);

	// 关闭转换描述符
	if (iconv_close (cd) < 0)
	{
		perror ("iconv_close");
		exit (1);
	}
}

/*
 *
 */
void
ccin_UTF8_to_locale_charset_for_hanzi (u_char * locale_charset,
									   ccinHanziChar_t * hanzi_utf8_string,
									   u_int hanzi_utf8_string_count,
									   u_char * charset_hanzi_string)
{
	iconv_t cd;
	u_int hanzi_utf8_string_len = hanzi_utf8_string_count * 3;	//UTF8 for GBK hanzi
	char *charset_hanzi_string_buf = (char *) charset_hanzi_string;
	u_int charset_string_len_left = hanzi_utf8_string_count * 2;

	// 打开  UTF-8 -> charset 的转换描述符
	cd = iconv_open ("GBK", "UTF-8");
	//cd = iconv_open (locale_charset,"UTF-8");
	if (cd == (iconv_t) (-1))
	{
		perror ("iconv_open");
		exit (1);
	}

	iconv (cd, (char **) &hanzi_utf8_string, &hanzi_utf8_string_len,
		   &charset_hanzi_string_buf, &charset_string_len_left);

	// 关闭转换描述符
	if (iconv_close (cd) < 0)
	{
		perror ("iconv_close");
		exit (1);
	}
}

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
