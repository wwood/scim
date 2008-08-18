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


#ifndef _DATATYPE_H
#define _DATATYPE_H

#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

//#include <stdint.h>
#include <sys/types.h>

#if 0
#ifdef __STDC_ISO_10646__
typedef wchar_t ucs4_t;
#else
typedef u_long ucs4_t;
#endif

typedef u_short uint16;
typedef u_long uint32;
#endif

typedef u_char ccinUTF8Char_t[3];
typedef ccinUTF8Char_t ccinHanziChar_t;	//utf8±àÂë

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
