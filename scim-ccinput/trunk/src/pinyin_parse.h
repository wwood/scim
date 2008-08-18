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


#ifndef _PINYIN_PARSE_H
#define _PINYIN_PARSE_H

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

u_short is_standard_pinyin (const char *origin_string,
							u_short origin_string_len);

u_short is_fuzzy_pinyin (const char *origin_string,
						 u_short origin_string_len, u_long flag_fuzzy);

//以后应该是传入imcontext结构……
int ccin_parse_pinyin (const char *origin_string,
					   u_short origin_string_len,
					   ccinSyllable_t parsed_pinyin[],
					   u_short * has_separator, u_long flag_fuzzy);

int ccin_parse_shuangpin (const char *origin_string,
						  u_short origin_string_len,
						  ccinSyllable_t parsed_shuangpin[],
						  ccinSyllable_t parsed_quanpin[],
						  u_short * has_separator, u_long flag_fuzzy);

void ccin_set_sp_config (ccinSPConfigureEnum_t config_sn);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
