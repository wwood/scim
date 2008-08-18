/** @file scim_special_table.h
 *  @brief the definitions of special table related classes and structs.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_special_table.h,v 1.1.1.1 2005/01/06 13:31:02 suzhe Exp $
 *
 */

#if !defined (__SCIM_SPECIAL_TABLE_H)
#define __SCIM_SPECIAL_TABLE_H
#include "config.h"

using namespace scim;

class SpecialTable
{
	typedef std::vector <std::pair <String, String> > SpecialMap;

	SpecialMap m_special_map;

	int m_max_key_length;

public:
	SpecialTable ();
	SpecialTable (std::istream &is);

	void load (std::istream &is);

	void clear (void);

	int find (std::vector <WideString> & result, const String & key) const;

	bool valid (void) const;

	int get_max_key_length (void) const;

private:
	WideString translate (const String & str) const;

	WideString get_date (int type) const;
	WideString get_time (int type) const;
	WideString get_day  (int type) const;
};

#endif

/*
vi:ts=4:nowrap:ai
*/
