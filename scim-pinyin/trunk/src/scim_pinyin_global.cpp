/** @file scim_pinyin_global.cpp
 * implementation of PinyinGlobal class.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_global.cpp,v 1.2 2005/08/06 12:06:49 suzhe Exp $
 */

/*
 * the purpose of class PinyinGlobal is to store
 * global configuration and data of Pinyin input method.
 * 
 */
#define Uses_STL_AUTOPTR
#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO
#define Uses_SCIM_UTILITY
#define Uses_SCIM_SERVER
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_LOOKUP_TABLE

#include <scim.h>
#include "scim_pinyin_private.h"
#include "scim_phrase.h"
#include "scim_pinyin.h"
#include "scim_pinyin_phrase.h"
#include "scim_pinyin_global.h"


// functions of PinyinGlobal

PinyinGlobal::PinyinGlobal ()
	: m_custom (NULL),
	  m_pinyin_table (NULL),
	  m_pinyin_validator ( NULL),
	  m_sys_phrase_lib (NULL),
	  m_user_phrase_lib (NULL)
{
	m_custom                = new PinyinCustomSettings;
	m_pinyin_validator      = new PinyinValidator ();
	m_pinyin_table          = new PinyinTable (*m_custom, m_pinyin_validator);
	m_sys_phrase_lib        = new PinyinPhraseLib (*m_custom, m_pinyin_validator, m_pinyin_table);
	m_user_phrase_lib       = new PinyinPhraseLib (*m_custom, m_pinyin_validator, m_pinyin_table);

	if (!m_pinyin_table || !m_sys_phrase_lib || !m_user_phrase_lib ||
		!m_pinyin_validator || !m_custom) {
		delete m_custom;
		delete m_pinyin_table;
		delete m_sys_phrase_lib;
		delete m_user_phrase_lib;
		delete m_pinyin_validator;
		throw PinyinGlobalError ("memory allocation error!");
	}

	toggle_tone (true);
	toggle_incomplete (false);
	toggle_dynamic_adjust (true);
	toggle_ambiguity (SCIM_PINYIN_AmbAny, false);

	update_custom_settings ();
}

PinyinGlobal::~PinyinGlobal ()
{
	delete m_custom;
	delete m_pinyin_table;
	delete m_sys_phrase_lib;
	delete m_user_phrase_lib;
	delete m_pinyin_validator;
}

bool
PinyinGlobal::use_tone () const
{
	return m_custom->use_tone;
}

bool
PinyinGlobal::use_ambiguity (const PinyinAmbiguity &amb) const
{
	return m_custom->use_ambiguities [static_cast<int>(amb)];
}

bool
PinyinGlobal::use_incomplete () const
{
	return m_custom->use_incomplete;
}

bool
PinyinGlobal::use_dynamic_adjust () const
{
	return m_custom->use_dynamic_adjust;
}

void
PinyinGlobal::toggle_tone (bool use)
{
	m_custom->use_tone  = use;
}

void
PinyinGlobal::toggle_incomplete (bool use)
{
	m_custom->use_incomplete = use;
}

void
PinyinGlobal::toggle_dynamic_adjust (bool use)
{
	m_custom->use_dynamic_adjust = use;
}

void
PinyinGlobal::toggle_ambiguity (const PinyinAmbiguity &amb, bool use)
{
	if (amb == SCIM_PINYIN_AmbAny) {
		for (unsigned int i=0; i<sizeof (m_custom->use_ambiguities)/sizeof(bool); i++)
			m_custom->use_ambiguities [i] = use;
	} else {
		m_custom->use_ambiguities [0] = false;
		m_custom->use_ambiguities [static_cast<int>(amb)] = use;
		for (unsigned int i=1; i<sizeof (m_custom->use_ambiguities)/sizeof(bool); i++) {
			if (m_custom->use_ambiguities [i] == true) {
				m_custom->use_ambiguities [0] = true;
				break;
			}
		}
	}
}

bool
PinyinGlobal::load_pinyin_table (istream &is_sys)
{
	m_pinyin_table->clear ();

	if (is_sys && m_pinyin_table->input (is_sys) && m_pinyin_table->size ()) {
		m_pinyin_validator->initialize (m_pinyin_table);
		return true;
	}
	m_pinyin_validator->initialize (NULL);
	return false;
}

bool
PinyinGlobal::load_pinyin_table (istream &is_sys, istream &is_user)
{
	m_pinyin_table->clear ();

	if (is_user && m_pinyin_table->input (is_user) && m_pinyin_table->size ()) {
		if (is_sys && m_pinyin_table->input (is_sys)) {
			m_pinyin_validator->initialize (m_pinyin_table);
			return true;
		}
	}
	m_pinyin_validator->initialize (NULL);
	return false;
}

bool
PinyinGlobal::load_pinyin_table (const char *sys, const char *user)
{
	if (sys && user) {
		ifstream is_sys (sys);
		ifstream is_user (user);
		if (is_user && load_pinyin_table (is_sys, is_user))
			return true;
		return load_pinyin_table (is_sys);
	} else if (sys) {
		ifstream is_sys (sys);
		return load_pinyin_table (is_sys);
	}
	return false;
}

bool
PinyinGlobal::save_pinyin_table (ostream &os_user, bool binary) const
{
	if (os_user)
		return m_pinyin_table->output (os_user, binary);
	else
		return false;
}

bool
PinyinGlobal::save_pinyin_table (const char *user, bool binary) const
{
	if (user) {
		ofstream os_user (user);
		return save_pinyin_table (os_user, binary);
	}
	return false;
}

bool
PinyinGlobal::load_sys_phrase_lib (istream &is_lib)
{
	return m_sys_phrase_lib->input (is_lib);
}

bool
PinyinGlobal::load_sys_phrase_lib (istream &is_lib, istream &is_pylib, istream &is_idx)
{
	return m_sys_phrase_lib->input (is_lib, is_pylib, is_idx);
}

bool
PinyinGlobal::load_sys_phrase_lib (const char *lib, const char *pylib, const char *idx)
{
	return m_sys_phrase_lib->load_lib (lib, pylib, idx);
}

bool
PinyinGlobal::load_user_phrase_lib (istream &is_lib)
{
	return m_user_phrase_lib->input (is_lib);
}

bool
PinyinGlobal::load_user_phrase_lib (istream &is_lib, istream &is_pylib, istream &is_idx)
{
	return m_user_phrase_lib->input (is_lib, is_pylib, is_idx);
}

bool
PinyinGlobal::load_user_phrase_lib (const char *lib, const char *pylib, const char *idx)
{
	return m_user_phrase_lib->load_lib (lib, pylib, idx);
}

bool
PinyinGlobal::save_user_phrase_lib (ostream &os_lib, ostream &os_pylib, ostream &os_idx, bool binary) const
{
	return m_user_phrase_lib->output (os_lib, os_pylib, os_idx, binary);
}

bool
PinyinGlobal::save_user_phrase_lib (const char *lib, const char *pylib, const char *idx, bool binary) const
{
	return m_user_phrase_lib->save_lib (lib, pylib, idx, binary);
}

bool
PinyinGlobal::save_sys_phrase_lib (ostream &os_lib, ostream &os_pylib, ostream &os_idx, bool binary) const
{
	return m_sys_phrase_lib->output (os_lib, os_pylib, os_idx, binary);
}

bool
PinyinGlobal::save_sys_phrase_lib (const char *lib, const char *pylib, const char *idx, bool binary) const
{
	return m_sys_phrase_lib->save_lib (lib, pylib, idx, binary);
}

void
PinyinGlobal::update_custom_settings ()
{
	m_pinyin_table->update_custom_settings (*m_custom, m_pinyin_validator);
	m_sys_phrase_lib->update_custom_settings (*m_custom, m_pinyin_validator);
	m_user_phrase_lib->update_custom_settings (*m_custom, m_pinyin_validator);
	m_pinyin_validator->initialize (m_pinyin_table);
}

/*
vi:ts=4:nowrap:ai
*/
