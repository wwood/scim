/** @file scim_pinyin_global.h
 * definition of PinyinGlobal class.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_global.h,v 1.1.1.1 2005/01/06 13:30:58 suzhe Exp $
 */

/*
 * the purpose of class PinyinGlobal is to store
 * global configuration and data of Pinyin input method.
 * 
 */

#if !defined (__SCIM_PINYIN_GLOBAL_H)
#define __SCIM_PINYIN_GLOBAL_H

using namespace scim;
using namespace std;

class PinyinGlobalError: public Exception {
public:
	PinyinGlobalError (const String& what_arg)
		: Exception (String("PinyinGlobal: ") + what_arg) { }
};

class PinyinTable;
class PinyinValidator;
class PinyinPhraseLib;

struct PinyinCustomSettings;
enum PinyinAmbiguity;

class PinyinGlobal
{
	PinyinCustomSettings      *m_custom;

	PinyinTable               *m_pinyin_table;
	PinyinValidator           *m_pinyin_validator;

	PinyinPhraseLib           *m_sys_phrase_lib;
	PinyinPhraseLib           *m_user_phrase_lib;

public:
	PinyinGlobal ();

	~PinyinGlobal ();

	bool use_tone () const;
	bool use_ambiguity (const PinyinAmbiguity &amb) const;
	bool use_incomplete () const;
	bool use_dynamic_adjust () const;

	void toggle_tone (bool use = false);
	void toggle_incomplete (bool use = false);
	void toggle_dynamic_adjust (bool use = true);
	void toggle_ambiguity (const PinyinAmbiguity &amb, bool use = false);
	
	bool load_pinyin_table (istream &is_sys);
	bool load_pinyin_table (istream &is_sys, istream &is_user);
	bool load_pinyin_table (const char *sys, const char *user = NULL);

	bool save_pinyin_table (ostream &os_user, bool binary = true) const;
	bool save_pinyin_table (const char *user, bool binary = true) const;

	PinyinTable * get_pinyin_table () {
		return m_pinyin_table;
	}
	const PinyinTable * get_pinyin_table () const {
		return m_pinyin_table;
	}

	bool load_sys_phrase_lib (istream &is_lib);
	bool load_sys_phrase_lib (istream &is_lib, istream &is_pylib, istream &is_idx);
	bool load_sys_phrase_lib (const char *lib, const char *pylib = NULL, const char *idx = NULL);

	bool load_user_phrase_lib (istream &is_lib);
	bool load_user_phrase_lib (istream &is_lib, istream &is_pylib, istream &is_idx);
	bool load_user_phrase_lib (const char *lib, const char *pylib = NULL, const char *idx = NULL);

	bool save_user_phrase_lib (ostream &os_lib, ostream &os_pylib, ostream &os_idx, bool binary = true) const;
	bool save_user_phrase_lib (const char *lib, const char *pylib, const char *idx, bool binary = true) const;

	bool save_sys_phrase_lib (ostream &os_lib, ostream &os_pylib, ostream &os_idx, bool binary = true) const;
	bool save_sys_phrase_lib (const char *lib, const char *pylib, const char *idx, bool binary = true) const;

	PinyinPhraseLib * get_user_phrase_lib () {
		return m_user_phrase_lib;
	}

	const PinyinPhraseLib * get_user_phrase_lib () const {
		return m_user_phrase_lib;
	}

	PinyinPhraseLib * get_sys_phrase_lib () {
		return m_sys_phrase_lib;
	}
	const PinyinPhraseLib * get_sys_phrase_lib () const {
		return m_sys_phrase_lib;
	}
	
	const PinyinValidator & get_pinyin_validator () const {
		return *m_pinyin_validator;
	}

	void update_custom_settings ();
};

#endif
/*
vi:ts=4:nowrap:ai
*/
