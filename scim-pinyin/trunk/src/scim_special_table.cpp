/** @file scim_special_table.cpp
 * implementation of SpecialTable class.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_special_table.cpp,v 1.1.1.1 2005/01/06 13:31:02 suzhe Exp $
 *
 */

#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uscs_C_STRING

#include <time.h>
#include <scim.h>
#include "scim_pinyin_private.h"
#include "scim_special_table.h"

static inline String
_trim_blank (const String &str)
{
	String::size_type begin, len;

	begin = str.find_first_not_of (" \t\n\v");

	if (begin == String::npos)
		return String ();

	len = str.find_last_not_of (" \t\n\v");

	if (len != String::npos)
		len = len - begin + 1;

	return str.substr (begin, len);
}

static inline String
_get_param_portion (const String &str, const String &delim = "=")
{
	String ret = str;
	String::size_type pos = ret.find_first_of (String (" \t\v") + delim);

	if (pos != String::npos)
		ret.erase (pos, String::npos);

	return ret;
}

static inline String
_get_value_portion (const String &str, const String &delim = "=")
{
	String ret = str;
	String::size_type pos;

	pos = ret.find_first_of (delim);

	if (pos != String::npos)
		ret.erase (0, pos + 1);

	pos = ret.find_first_not_of(" \t\v");

	if (pos != String::npos)
		ret.erase (0, pos);

	pos = ret.find_last_not_of(" \t\v");

	if (pos != String::npos)
		ret.erase (pos + 1, String::npos);

	return ret;
}

static inline String  
_get_line (std::istream &is)
{
	char temp [1024];
	String res;

	while (1) {
		is.getline (temp, 1023);
		res = _trim_blank (String (temp));

		if (res.length () > 0 && res [0] != '#') return res;
		if (is.eof ()) return String ();
	}
}

static inline WideString
_hex_to_wide_string (const String &str)
{
	WideString ret;
	uint32 i = 0;

	while (i <= str.length () - 6 && str [i] == '0' && tolower (str [i+1]) == 'x') {
		ucs4_t wc = (ucs4_t) strtol (str.substr (i+2, 4).c_str (), NULL, 16);

		if (wc)
			ret.push_back (wc);

		i += 6;
	}
	return ret;
}

class SpecialKeyItemLessThanByKey
	: public std::binary_function <
				std::pair <String, String>,
				std::pair <String, String>,
				bool>
{
public:
	bool operator () (const std::pair <String, String> & lhs,
					  const std::pair <String, String> & rhs) const {
		int eq = strncmp (lhs.first.c_str (), rhs.first.c_str (), std::min (lhs.first.length(), rhs.first.length()));

		if (eq < 0) return true;
		if (eq > 0) return false;
		return lhs.first.length () < rhs.first.length ();
	}
};

class SpecialKeyItemLessThanByKeyStrictLength
	: public std::binary_function <
				std::pair <String, String>,
				std::pair <String, String>,
				bool>
{
	size_t m_min_len;
public:
	SpecialKeyItemLessThanByKeyStrictLength (size_t min_len) : m_min_len (min_len) {}

	bool operator () (const std::pair <String, String> & lhs,
					  const std::pair <String, String> & rhs) const {

		int eq = strncmp (lhs.first.c_str (), rhs.first.c_str (), std::min (lhs.first.length(), rhs.first.length()));

		if (eq < 0) return true;
		if (eq > 0) return false;

		if (lhs.first.length () >= rhs.first.length ()) return false;

		if (lhs.first.length () < m_min_len) return true;

		return false;
	}
};

SpecialTable::SpecialTable ()
	: m_max_key_length (0)
{
}

SpecialTable::SpecialTable (std::istream &is)
	: m_max_key_length (0)
{
	load (is);
}

void
SpecialTable::load (std::istream &is)
{
	String line;
	String key;
	String entry;
	std::vector<String> items;
	int count = 0;

	while (1) {
		line = _get_line (is);

		if (!line.length ()) break;

		key = _get_param_portion (line);
		entry = _get_value_portion (line);

		if (!key.length () || !entry.length ()) break;

		scim_split_string_list (items, entry, ',');

		for (std::vector<String>::iterator it = items.begin (); it != items.end (); ++it) {
			if (it->length ()) {
				m_special_map.push_back (std::make_pair (key, *it));
				count ++;
				if (key.length () > m_max_key_length)
					m_max_key_length = key.length ();
			}
		}
	}

	std::sort (m_special_map.begin (), m_special_map.end ());

	m_special_map.erase (std::unique (m_special_map.begin (), m_special_map.end ()), m_special_map.end ());

	std::stable_sort (m_special_map.begin (), m_special_map.end (), SpecialKeyItemLessThanByKey ());
}

void
SpecialTable::clear (void)
{
	SpecialMap ().swap (m_special_map);
	m_max_key_length = 0;
}

bool
SpecialTable::valid (void) const
{
	return m_special_map.size () != 0;
}

int
SpecialTable::get_max_key_length (void) const
{
	return m_max_key_length;
}


int
SpecialTable::find (std::vector <WideString> & result,
					const String			 & key) const
{
	SpecialMap::const_iterator lower_it =
		std::lower_bound (m_special_map.begin (),
						  m_special_map.end (), 
						  std::make_pair (key, String ()),
						  SpecialKeyItemLessThanByKeyStrictLength (
							std::max (key.length (), (size_t)3)));

	SpecialMap::const_iterator upper_it =
		std::upper_bound (m_special_map.begin (),
						  m_special_map.end (), 
						  std::make_pair (key, String ()),
						  SpecialKeyItemLessThanByKeyStrictLength (
							std::max (key.length (), (size_t)3)));

	result.clear ();

	for (SpecialMap::const_iterator it = lower_it; it != upper_it; ++ it)
		result.push_back (translate (it->second));

	std::sort (result.begin (), result.end ());

	result.erase (std::unique (result.begin (), result.end ()), result.end ());

	return (int) result.size ();
}

WideString
SpecialTable::translate (const String & str) const
{
	if (str.length () > 2 && str [0] == 'X' && str [1] == '_') {
		if (str.length () > 7 && str [2] == 'D' &&
			str [3] == 'A' && str [4] == 'T' &&
			str [5] == 'E' && str [6] == '_')
			return get_date (str [7] - '1');
		else if (str.length () > 7 && str [2] == 'T' &&
			str [3] == 'I' && str [4] == 'M' &&
			str [5] == 'E' && str [6] == '_')
			return get_time (str [7] - '1');
		else if (str.length () > 6 && str [2] == 'D' &&
			str [3] == 'A' && str [4] == 'Y' && str [5] == '_')
			return get_day (str [6] - '1');
	} else if (str.length () > 5 && str [0] == '0' && (str [1] == 'x' || str [1] == 'X')) {
		return _hex_to_wide_string (str);
	}

	return utf8_mbstowcs (str);
}

static const char * __chinese_number_little_simp [] =
{
	"〇", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十", NULL
};

static const char * __chinese_number_little_trad [] =
{
	"零", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十", NULL
};

static const char * __chinese_number_big_simp [] =
{
	"零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖", "拾", NULL
};

static const char * __chinese_number_big_trad [] =
{
	"零", "壹", "貳", "參", "肆", "伍", "陸", "柒", "捌", "玖", "拾", NULL
};

static const char * __chinese_week_1 [] =
{
	"日", "一", "二", "三", "四", "五", "六", NULL
};

static const char * __chinese_week_2 [] =
{
	"天", "一", "二", "三", "四", "五", "六", NULL
};

static void
get_broken_down_time (struct tm &buf)
{
	time_t simple_time = time (NULL);

#ifdef HAVE_LOCALTIME_R
	localtime_r (&simple_time, &buf);
#else
	struct tm *tmp = localtime (&simple_time);
	if (tmp) memcpy (&buf, tmp, sizeof (struct tm));
#endif
}

WideString
SpecialTable::get_date (int type) const
{
	struct tm cur_time;
	String result;

	get_broken_down_time (cur_time);

	cur_time.tm_year += 1900;
	cur_time.tm_year %= 10000;
	cur_time.tm_mon ++;

	if (type == 0) {
		char buf [80];
		snprintf (buf, 80, "%d年%d月%d日",
			cur_time.tm_year, cur_time.tm_mon, cur_time.tm_mday);
		result = String (buf);
	} else if (type <= 4) {
		const char ** numbers;
		switch (type) {
			case 1: numbers = __chinese_number_little_simp; break;
			case 2: numbers = __chinese_number_little_trad; break;
			case 3: numbers = __chinese_number_big_simp; break;
			case 4: numbers = __chinese_number_big_trad; break;
		}

		result = String (numbers [cur_time.tm_year / 1000]);
		cur_time.tm_year %= 1000;
		result += String (numbers [cur_time.tm_year / 100]);
		cur_time.tm_year %= 100;
		result += String (numbers [cur_time.tm_year / 10]);
		cur_time.tm_year %= 10;
		result += String (numbers [cur_time.tm_year]);
		result += String ("年");

		if (cur_time.tm_mon < 10)
			result += String (numbers [cur_time.tm_mon]);
		else {
			result += String (numbers [10]);
			if (cur_time.tm_mon > 10)
				result += String (numbers [cur_time.tm_mon % 10]);
		}
		result += String ("月");

		if (cur_time.tm_mday < 10)
			result += String (numbers [cur_time.tm_mday]);
		else {
			if (cur_time.tm_mday >= 20)
				result += String (numbers [cur_time.tm_mday / 10]);
			result += String (numbers [10]);
			if (cur_time.tm_mday % 10)
				result += String (numbers [cur_time.tm_mday % 10]);
		}
		result += String ("日");
	} else {
		char buf [80];
		snprintf (buf, 80, "%d-%d-%d",
			cur_time.tm_year, cur_time.tm_mon, cur_time.tm_mday);
		result = String (buf);
	}
	return utf8_mbstowcs (result);
}

WideString
SpecialTable::get_day  (int type) const
{
	struct tm cur_time;

	get_broken_down_time (cur_time);

	switch (type) {
		case 1:
			return utf8_mbstowcs (String ("星期") + String (__chinese_week_2 [cur_time.tm_wday]));
		case 2:
			return utf8_mbstowcs (String ("礼拜") + String (__chinese_week_1 [cur_time.tm_wday]));
		case 3:
			return utf8_mbstowcs (String ("礼拜") + String (__chinese_week_2 [cur_time.tm_wday]));
		default:
			return utf8_mbstowcs (String ("星期") + String (__chinese_week_1 [cur_time.tm_wday]));
	}
}

WideString
SpecialTable::get_time (int type) const
{
	struct tm cur_time;
	String result;
	char buf [80];
	const char ** numbers;

	get_broken_down_time (cur_time);

	switch (type) {
		case 0:
		case 1:
			snprintf (buf, 80, "%d%s%d分",
				cur_time.tm_hour, ((type == 0) ? "点" : "點"), cur_time.tm_min);
			result = String (buf);
			break;
		case 2:
		case 3:
			snprintf (buf, 80, "%s%d%s%d分",
				(cur_time.tm_hour <= 12) ? "上午" : "下午",
				(cur_time.tm_hour <= 12) ? cur_time.tm_hour : (cur_time.tm_hour - 12),
				((type == 2) ? "点" : "點"),
				cur_time.tm_min);
			result = String (buf);
			break;
		case 4:
		case 5:
			numbers = (type == 4) ? __chinese_number_little_simp : __chinese_number_little_trad;

			if (cur_time.tm_hour < 10)
				result += String (numbers [cur_time.tm_hour]);
			else {
				if (cur_time.tm_hour >= 20)
					result += String (numbers [cur_time.tm_hour / 10]);
				result += String (numbers [10]);
				if (cur_time.tm_hour % 10)
					result += String (numbers [cur_time.tm_hour % 10]);
			}

			result += String ((type == 4) ? "点" : "點");

			if (cur_time.tm_min < 10)
				result += String (numbers [cur_time.tm_min]);
			else {
				if (cur_time.tm_min >= 20)
					result += String (numbers [cur_time.tm_min / 10]);
				result += String (numbers [10]);
				if (cur_time.tm_min % 10)
					result += String (numbers [cur_time.tm_min % 10]);
			}

			result += String ("分");
			break;
		case 6:
		case 7:
			numbers = (type == 4) ? __chinese_number_little_simp : __chinese_number_little_trad;

			if (cur_time.tm_hour <= 12)
				result += String ("上午");
			else {
				result += String ("下午");
				cur_time.tm_hour -= 12;
			}

			if (cur_time.tm_hour < 10)
				result += String (numbers [cur_time.tm_hour]);
			else {
				result += String (numbers [10]);
				if (cur_time.tm_hour % 10)
					result += String (numbers [cur_time.tm_hour % 10]);
			}

			result += String ((type == 6) ? "点" : "點");

			if (cur_time.tm_min < 10)
				result += String (numbers [cur_time.tm_min]);
			else {
				if (cur_time.tm_min >= 20)
					result += String (numbers [cur_time.tm_min / 10]);
				result += String (numbers [10]);
				if (cur_time.tm_min % 10)
					result += String (numbers [cur_time.tm_min % 10]);
			}

			result += String ("分");
			break;

		default:
			snprintf (buf, 80, "%d:%d", cur_time.tm_hour, cur_time.tm_min);
			result = String (buf);
			break;
	}
	return utf8_mbstowcs (result);
}

/*
vi:ts=4:nowrap:ai
*/
