/** @file scim_pinyin_phrase.cpp
 * implementation of PinyinPhrase, PinyinPhraseLib and related classes.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_phrase.cpp,v 1.3 2006/01/13 06:31:46 suzhe Exp $
 *
 */
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

static const char scim_pinyin_lib_text_header [] = "SCIM_Pinyin_Library_TEXT";
static const char scim_pinyin_lib_binary_header [] = "SCIM_Pinyin_Library_BINARY";
static const char scim_pinyin_lib_version [] = "VERSION_0_1";

static const char scim_pinyin_phrase_idx_lib_text_header [] = "SCIM_Pinyin_Phrase_Index_Library_TEXT";
static const char scim_pinyin_phrase_idx_lib_binary_header [] = "SCIM_Pinyin_Phrase_Index_Library_BINARY";
static const char scim_pinyin_phrase_idx_lib_version [] = "VERSION_0_1";

bool
PinyinPhraseLessThan::operator () (const PinyinPhrase &lhs,
								   const PinyinPhrase &rhs) const
{
	if (lhs.get_phrase () < rhs.get_phrase ()) return true;
	else if (lhs.get_phrase () == rhs.get_phrase ()) {
		for (unsigned int i=0; i<lhs.length(); i++) {
			if (m_less (lhs.get_key (i), rhs.get_key (i))) return true;
			else if (m_less (rhs.get_key (i), lhs.get_key (i))) return false;
		}
	}
	return false;
}

bool
PinyinPhraseEqualTo::operator () (const PinyinPhrase &lhs,
								  const PinyinPhrase &rhs) const
{
	if (lhs.get_lib () == rhs.get_lib ()&&
		 lhs.get_pinyin_offset () == rhs.get_pinyin_offset () &&
		 lhs.get_phrase_offset () == rhs.get_phrase_offset ())
		return true;
	else if (!(lhs.get_phrase () == rhs.get_phrase ())) return false;
	else {
		for (unsigned int i=0; i<lhs.length(); i++)
			if (!m_equal (lhs.get_key (i), rhs.get_key (i))) return false;
	}
	return true;
}

bool
PinyinPhraseLib::output_pinyin_lib (std::ostream &os, bool binary)
{
	if (m_pinyin_lib.size () == 0) return false;

	if (binary) {
		unsigned char bytes [4];

		os << scim_pinyin_lib_binary_header << "\n";
		os << scim_pinyin_lib_version << "\n";

		scim_uint32tobytes (bytes, m_pinyin_lib.size ());
		os.write ((char*) bytes, sizeof (unsigned char) * 4);

		for (PinyinKeyVector::iterator i=m_pinyin_lib.begin (); i!=m_pinyin_lib.end (); i++)
			i->output_binary (os);

	} else {
		uint32 count = 0;
		os << scim_pinyin_lib_text_header << "\n";
		os << scim_pinyin_lib_version << "\n";
		os << m_pinyin_lib.size () << "\n";

		for (PinyinKeyVector::iterator i=m_pinyin_lib.begin (); i!=m_pinyin_lib.end (); i++) {
			i->output_text (os);
			os << " ";
			count ++;
			if (count == 32) {
				os << "\n";
				count = 0;
			}
		}
	}
	return true;
}

bool
PinyinPhraseLib::input_pinyin_lib (const PinyinValidator &validator, std::istream &is)
{
	if (!is) return false;

	m_pinyin_lib.clear ();

	char header [40];
	bool binary;

	//check header
	is.getline (header, 40);
	if (strncmp (header,
		scim_pinyin_lib_text_header,
		strlen (scim_pinyin_lib_text_header)) == 0) {
		binary = false;
	} else if (strncmp (header,
		scim_pinyin_lib_binary_header,
		strlen (scim_pinyin_lib_binary_header)) == 0) {
		binary = true;
	} else {
		return false;
	}
	
	is.getline (header, 40);
	if (strncmp (header, scim_pinyin_lib_version, strlen (scim_pinyin_lib_version)) != 0)
		return false;

	unsigned char bytes [4];
	PinyinKey key;
	uint32 number;

	//get length
	if (binary) {
		is.read ((char*) bytes, sizeof(unsigned char) * 4);
		number = scim_bytestouint32 (bytes);
	} else {
		is.getline (header, 40);
		number = atoi (header);
	}

	if (number <= 0) return false;

	m_pinyin_lib.reserve (number + 256);

	if (binary) {
		for (uint32 i=0; i<number; i++) {
			key.input_binary (validator, is);
			m_pinyin_lib.push_back (key);
		}
	} else {
		for (uint32 i=0; i<number; i++) {
			key.input_text (validator, is);
			m_pinyin_lib.push_back (key);
		}
	}

	return true;
}

//Pinyin Phrase Library
PinyinPhraseLib::PinyinPhraseLib (const PinyinCustomSettings &custom,
								  const PinyinValidator *validator,
				 				  PinyinTable *pinyin_table,
								  const char *libfile,
								  const char *pylibfile,
								  const char *idxfile)
	: m_pinyin_table (pinyin_table),
	  m_validator (validator),
	  m_pinyin_key_less (custom),
	  m_pinyin_key_equal (custom),
	  m_pinyin_phrase_less_by_offset (this, custom),
	  m_pinyin_phrase_equal_by_offset (this, custom)
{
	if (!m_validator) m_validator = PinyinValidator::get_default_pinyin_validator ();

	load_lib (libfile, pylibfile, idxfile);
}

PinyinPhraseLib::PinyinPhraseLib (const PinyinCustomSettings &custom,
								  const PinyinValidator *validator,
				 				  PinyinTable *pinyin_table,
								  std::istream &is_lib,
								  std::istream &is_pylib,
								  std::istream &is_idx)
	: m_pinyin_table (pinyin_table),
	  m_validator (validator),
	  m_pinyin_key_less (custom),
	  m_pinyin_key_equal (custom),
	  m_pinyin_phrase_less_by_offset (this, custom),
	  m_pinyin_phrase_equal_by_offset (this, custom)
{
	if (!m_validator) m_validator = PinyinValidator::get_default_pinyin_validator ();

	input (is_lib, is_pylib, is_idx);
}

class __PinyinPhraseOutputIndexFuncBinary {
	std::ostream &m_os;
public:
	__PinyinPhraseOutputIndexFuncBinary (std::ostream &os) : m_os (os) { }
	void operator () (const PinyinPhrase & phrase) {
		if (phrase.is_enable ()) {
			unsigned char bytes [8];
			scim_uint32tobytes (bytes, phrase.get_phrase_offset ());
			scim_uint32tobytes (bytes+4, phrase.get_pinyin_offset ());
			m_os.write ((char*) bytes, sizeof (unsigned char) * 8);
		}
	}
};

class __PinyinPhraseOutputIndexFuncText {
	std::ostream &m_os;
public:
	__PinyinPhraseOutputIndexFuncText (std::ostream &os) : m_os (os) { }
	void operator () (const PinyinPhrase & phrase) {
		if (phrase.is_enable ()) {
			m_os << phrase.get_phrase_offset () << " ";
			m_os << phrase.get_pinyin_offset ();
			m_os << "\n";
		}
	}
};

bool
PinyinPhraseLib::output_indexes (std::ostream &os, bool binary)
{
	uint32 phrase_number = count_phrase_number ();

	if (binary) {
		unsigned char bytes [4];
		os << scim_pinyin_phrase_idx_lib_binary_header << "\n";
		os << scim_pinyin_phrase_idx_lib_version << "\n";

		scim_uint32tobytes (bytes, phrase_number);
		os.write ((char*) bytes, sizeof (unsigned char) * 4);

		__PinyinPhraseOutputIndexFuncBinary func(os);

		for_each_phrase (func);
	} else {
		os << scim_pinyin_phrase_idx_lib_text_header << "\n";
		os << scim_pinyin_phrase_idx_lib_version << "\n";
		os << phrase_number << "\n";

		__PinyinPhraseOutputIndexFuncText func(os);

		for_each_phrase (func);
	}
	return true;
}

bool
PinyinPhraseLib::input_indexes (std::istream &is)
{
	char header [40];
	bool binary = false;

	if (!is) return false;

	//check index file
	is.getline (header, 40);
	if (strncmp (header,
		scim_pinyin_phrase_idx_lib_text_header,
		strlen (scim_pinyin_phrase_idx_lib_text_header)) == 0) {
		binary = false;
	} else if (strncmp (header,
		scim_pinyin_phrase_idx_lib_binary_header,
		strlen (scim_pinyin_phrase_idx_lib_binary_header)) == 0) {
		binary = true;
	} else {
		return false;
	}

	is.getline (header, 40);
	if (strncmp (header, scim_pinyin_phrase_idx_lib_version,
					strlen (scim_pinyin_phrase_idx_lib_version)) != 0)
		return false;

	unsigned char bytes [8];
	uint32 number;

	//get index number
	if (binary) {
		is.read ((char*) bytes, sizeof(unsigned char) * 4);
		number = scim_bytestouint32 (bytes);
	} else {
		is.getline (header, 40);
		number = atoi (header);
	}

	if (number == 0) return false;

	clear_phrase_index ();

	if (binary) {
		for (uint32 i=0; i<number; i++) {
			is.read ((char*) bytes, sizeof(unsigned char) * 8);

			insert_pinyin_phrase_into_index (scim_bytestouint32 (bytes),
											  scim_bytestouint32 (bytes+4));
		}
	} else {
		uint32 phrase_offset;
		uint32 pinyin_offset;
		for (uint32 i=0; i<number; i++) {
			is >> phrase_offset;
			is >> pinyin_offset;

			insert_pinyin_phrase_into_index (phrase_offset, pinyin_offset);
		}
	}

	sort_phrase_tables ();

	return true;
}

bool
PinyinPhraseLib::output (std::ostream &os_lib,
						 std::ostream &os_pylib,
						 std::ostream &os_idx,
						 bool binary)
{
	bool ret = true;
	if (!(os_lib || os_pylib || os_idx))
		return false;

	if (os_lib && !m_phrase_lib.output (os_lib, binary))
		ret = false;

	if (os_pylib && !output_pinyin_lib (os_pylib, binary))
		ret = false;

	if (os_idx && !output_indexes (os_idx, binary))
		ret = false;

	return ret;
}

bool
PinyinPhraseLib::input (std::istream &is_lib,
						std::istream &is_pylib,
						std::istream &is_idx)
{
	if (m_phrase_lib.input (is_lib)) {
		if (is_idx && input_pinyin_lib (*m_validator, is_pylib)) {
			if (!input_indexes (is_idx)) {
				create_pinyin_index ();
				return true;
			}
		} else {
			create_pinyin_index ();
			return true;
		}
		return true;
	}
	return false;
}

bool
PinyinPhraseLib::input (std::istream &is_lib)
{
	if (m_phrase_lib.input (is_lib)) {
		create_pinyin_index ();
		return true;
	}
	return false;
}

bool
PinyinPhraseLib::load_lib (const char *libfile,
						   const char *pylibfile,
						   const char *idxfile)
{
	std::ifstream is_lib(libfile);
	std::ifstream is_pylib (pylibfile);
	std::ifstream is_idx (idxfile);
	if (!is_lib) return false;
	input (is_lib, is_pylib, is_idx);
	compact_memory ();
	return number_of_phrases () != 0;
}

bool
PinyinPhraseLib::save_lib (const char *libfile,
						   const char *pylibfile,
						   const char *idxfile,
						   bool binary)
{
	std::ofstream os_lib(libfile);
	std::ofstream os_pylib(pylibfile);
	std::ofstream os_idx(idxfile);
	return output (os_lib, os_pylib, os_idx, binary);
}

void
PinyinPhraseLib::update_custom_settings (const PinyinCustomSettings &custom,
										 const PinyinValidator *validator)
{
	m_pinyin_key_less  = PinyinKeyLessThan (custom);
	m_pinyin_key_equal = PinyinKeyEqualTo (custom);
	m_pinyin_phrase_less_by_offset  = PinyinPhraseLessThanByOffset (this, custom);
	m_pinyin_phrase_equal_by_offset = PinyinPhraseEqualToByOffset (this, custom);

	m_validator = validator;

	if (!m_validator)
		m_validator = PinyinValidator::get_default_pinyin_validator ();

	sort_phrase_tables ();
}

int
PinyinPhraseLib::find_phrases (PhraseVector &vec,
							   const PinyinKeyVector &keys,
							   bool noshorter,
							   bool nolonger)
{
	int minlen, maxlen;

	if (noshorter) minlen = keys.size();
	else minlen = 1;

	if (nolonger) maxlen = keys.size();
	else maxlen = -1;

	return find_phrases (vec, keys.begin(), keys.end(), minlen, maxlen);
}

int
PinyinPhraseLib::find_phrases (PhraseVector &vec,
							   const PinyinParsedKeyVector &keys,
							   bool noshorter,
							   bool nolonger)
{
	int minlen, maxlen;

	if (noshorter) minlen = keys.size();
	else minlen = 1;

	if (nolonger) maxlen = keys.size();
	else maxlen = -1;

	PinyinKeyVector nkeys;

	for (PinyinParsedKeyVector::const_iterator i=keys.begin(); i!=keys.end(); i++)
		nkeys.push_back (*i);

	return find_phrases (vec, nkeys.begin(), nkeys.end(), minlen, maxlen);
}

int
PinyinPhraseLib::find_phrases (PhraseVector &vec,
							   const PinyinParsedKeyVector::const_iterator &begin,
							   const PinyinParsedKeyVector::const_iterator &end,
							   int minlen,
							   int maxlen)
{
	PinyinKeyVector nkeys;

	for (PinyinParsedKeyVector::const_iterator i=begin; i!=end; i++)
		nkeys.push_back (*i);

	return find_phrases (vec, nkeys.begin(), nkeys.end(), minlen, maxlen);
}

int
PinyinPhraseLib::find_phrases (PhraseVector &vec,
							   const char *keys,
							   bool noshorter,
							   bool nolonger)
{
	PinyinParsedKeyVector pykeys;
	PinyinDefaultParser parser;

	parser.parse (*m_validator, pykeys, keys);

	return find_phrases (vec, pykeys, noshorter, nolonger);
}

int
PinyinPhraseLib::find_phrases (PhraseVector &vec,
							   const PinyinKeyVector::const_iterator &begin,
							   const PinyinKeyVector::const_iterator &end,
							   int minlen,
							   int maxlen)
{
	if (begin >= end) return 0;

	minlen -= 1;
	if (minlen < 0) minlen = 0;

	if (maxlen <= 0) maxlen = SCIM_PHRASE_MAX_LENGTH;
	else maxlen = std::min (maxlen, SCIM_PHRASE_MAX_LENGTH);

	if (minlen >= maxlen) return 0;

	std::pair<PinyinPhraseTable::iterator, PinyinPhraseTable::iterator> ptit;

	for (int i=minlen; i<maxlen; i++) {
		ptit = std::equal_range (m_phrases [i].begin (),
								 m_phrases [i].end (),
								 (*begin),
								 m_pinyin_key_less);
		
		PinyinKeyVector::const_iterator pos = begin + (std::min ((int)(end-begin-1), i));

		for (PinyinPhraseTable::iterator tit=ptit.first; tit!=ptit.second; tit++) {
			find_phrases_impl (vec,
							 tit->get_vector ().begin(),
							 tit->get_vector ().end(),
							 begin,
							 pos,
							 end);
		}
	}

	std::sort (vec.begin(), vec.end(), PhraseExactLessThan ());
	vec.erase (std::unique (vec.begin(), vec.end(), PhraseExactEqualTo ()), vec.end());

	return vec.size ();
}

void
PinyinPhraseLib::find_phrases_impl (PhraseVector &pv,
									const PinyinPhraseOffsetVector::iterator &begin,
									const PinyinPhraseOffsetVector::iterator &end,
									const PinyinKeyVector::const_iterator &key_begin,
									const PinyinKeyVector::const_iterator &key_pos,
									const PinyinKeyVector::const_iterator &key_end)
{
	if (begin == end) return;

	if (key_pos == key_begin) {
		for (PinyinPhraseOffsetVector::iterator i=begin; i!=end; i++) {
			if (valid_pinyin_phrase (i->first, i->second) &&
				get_phrase (i->first).is_enable ())
				pv.push_back (get_phrase (i->first));
		}
		return;
	}

	std::sort (begin, end, PinyinPhraseLessThanByOffsetSP (this, m_pinyin_key_less, key_pos-key_begin));

	std::pair<PinyinPhraseOffsetVector::iterator, PinyinPhraseOffsetVector::iterator> it =
		std::equal_range (begin, end, *key_pos,
						  PinyinPhraseLessThanByOffsetSP (this, m_pinyin_key_less, key_pos-key_begin));

	return find_phrases_impl (pv, it.first, it.second, key_begin, key_pos-1, key_end);
}

Phrase
PinyinPhraseLib::append (const Phrase &phrase, const PinyinKeyVector &keys)
{
	if (!phrase.valid () || !valid ())
		return Phrase ();

	Phrase tmp = m_phrase_lib.find (phrase);

	if (tmp.valid () && tmp.is_enable ())
		return tmp;

	tmp = m_phrase_lib.append (phrase);

	if (!tmp.valid () || !tmp.is_enable ())
		return Phrase ();

	insert_phrase_into_index (tmp, keys);
	return tmp;
}

Phrase
PinyinPhraseLib::append (const WideString &phrase, const PinyinKeyVector &keys)
{
	if (phrase.length () == 0 || !valid ())
		return Phrase ();

	Phrase tmp = m_phrase_lib.find (phrase);

	if (tmp.valid () && tmp.is_enable ())
		return tmp;

	tmp = m_phrase_lib.append (phrase);

	if (!tmp.valid ())
		return Phrase ();

	insert_phrase_into_index (tmp, keys);
	return tmp;
}

bool
PinyinPhraseLib::insert_phrase_into_index (const Phrase &phrase, const PinyinKeyVector &keys)
{
	if (!phrase.valid ()) return false;

	// First find out all of the chars which have no valid key in keys.
	WideString content = phrase.get_content ();
	WideString nokey_content;

	PinyinKeyVector final_keys;

	std::vector<uint32> content_state;

	std::vector<PinyinKeyVector> key_vv;

	uint32 pinyin_offset = m_pinyin_lib.size ();

	uint32 i,j,k;

	for (i=0; i<content.length (); ++i) {
		if (i < keys.size () &&
			keys [i].get_initial () != SCIM_PINYIN_ZeroInitial &&
			keys [i].get_final () != SCIM_PINYIN_ZeroFinal) {
			//This key is valid, store it into final_key.
			final_keys.push_back (keys [i]);
			content_state.push_back (1);
		} else {
			//This key is invalid, put the content into nokey_content,
			//and store a zero key into final_keys,
			//and store the position into invalid_key_pos.
			nokey_content.push_back (content [i]);
			final_keys.push_back (PinyinKey ());
			content_state.push_back (0);
		}
	}

	if (nokey_content.length ())
		m_pinyin_table->find_key_strings (key_vv, nokey_content);
	else
		key_vv.push_back (PinyinKeyVector ());

	std::sort (m_phrases [content.length () -1].begin (),
			   m_phrases [content.length () -1].end (),
			   PinyinKeyExactLessThan ());

	if (m_pinyin_lib.capacity () < m_pinyin_lib.size () + key_vv.size () * content.length ())
		m_pinyin_lib.reserve (m_pinyin_lib.size () + key_vv.size () * content.length () + 1);

	for (i=0; i<key_vv.size(); ++i) {
		for (j=0, k=0; j<content.length (); ++j) { 
			if (content_state [j])
				m_pinyin_lib.push_back (final_keys [j]);
			else
				m_pinyin_lib.push_back (key_vv [i][k++]);
		}

		insert_pinyin_phrase_into_index (phrase.get_phrase_offset (),
										 pinyin_offset);

		pinyin_offset = m_pinyin_lib.size ();
	}

	std::sort (m_phrases [content.length () -1].begin (),
			   m_phrases [content.length () -1].end (), m_pinyin_key_less);

	return true;
}

bool
PinyinPhraseLib::insert_pinyin_phrase_into_index (uint32 phrase_index, uint32 pinyin_index)
{
	if (!valid_pinyin_phrase (phrase_index, pinyin_index))
		return false;

	uint32 len = get_phrase (phrase_index).length();

	if (len <= 0) return false;

	PinyinKey key = get_pinyin_key (pinyin_index);

	PinyinPhraseTable::iterator ptit= 
		std::lower_bound (m_phrases[len-1].begin (), m_phrases[len-1].end (), key, PinyinKeyExactLessThan ());

	if (ptit != m_phrases[len-1].end () && PinyinKeyExactEqualTo () (*ptit,key)) {
		ptit->get_vector ().push_back (PinyinPhraseOffsetPair (phrase_index, pinyin_index));
	} else {
		PinyinPhraseEntry entry (key);
		entry.get_vector ().push_back (PinyinPhraseOffsetPair (phrase_index, pinyin_index));

		if (ptit != m_phrases [len-1].end () &&
			ptit >= m_phrases [len-1].begin () &&
			m_phrases[len-1].size () > 0) {
			m_phrases[len-1].insert (ptit, entry);
		} else {
			m_phrases[len-1].push_back (entry);
		}
	}
	return true;
}

class __PinyinPhraseCountNumber
{
	uint32 m_number;
public:
	__PinyinPhraseCountNumber () : m_number (0) { }
	uint32 get_number () { return m_number; }
	void operator () (const PinyinPhrase &phrase) {
		if (phrase.is_enable ())
			m_number ++;
	}
};

uint32
PinyinPhraseLib::count_phrase_number ()
{
	__PinyinPhraseCountNumber counter;

	for_each_phrase (counter);
	
	return counter.get_number();
}

void
PinyinPhraseLib::create_pinyin_index ()
{
	if (!m_pinyin_table || !m_pinyin_table->size()) return;

	clear_phrase_index ();

	uint32 pinyin_offset = 0;

	WideString content;
	Phrase phrase;

	for (uint32 i=0; i<m_phrase_lib.number_of_phrases (); i++) {
		phrase = m_phrase_lib.get_phrase_by_index (i);

		content = phrase.get_content ();

		std::vector<PinyinKeyVector> key_vv;
		m_pinyin_table->find_key_strings (key_vv, content);

		for (uint32 j=0; j<key_vv.size(); j++) {
			for (uint32 k=0; k<key_vv[j].size(); k++)
				m_pinyin_lib.push_back (key_vv[j][k]);

			insert_pinyin_phrase_into_index (phrase.get_phrase_offset (), pinyin_offset);

			pinyin_offset = m_pinyin_lib.size ();
		}
#if 0
		if (key_vv.size () > 1 && content.length () > 1) {
			for (uint32 x=0; x<key_vv.size (); x++) {
				std::cerr << phrase.frequency () << "\t| " << 
						utf8_wcstombs (content) << " =";
				for (uint32 y=0; y<key_vv[x].size (); y++)
					std::cerr << " " << key_vv[x][y];
				std::cerr << "\n";
			}
		}
#endif
		std::cout << "." << std::flush;
	}

	sort_phrase_tables ();

	std::cout << "Phrase Number = " << count_phrase_number () << "\n";
}

void
PinyinPhraseLib::sort_phrase_tables ()
{
	for (uint32 i=0; i<SCIM_PHRASE_MAX_LENGTH; i++) {
		if (m_phrases [i].size ())
			std::sort (m_phrases[i].begin (), m_phrases[i].end (), m_pinyin_key_less);
	}
}

void
PinyinPhraseLib::refine_phrase_index (PinyinPhraseValidatorFunc pinyin_phrase_validator)
{
	for (uint32 i=0; i<SCIM_PHRASE_MAX_LENGTH; i++) {
		for (PinyinPhraseTable::iterator tit=m_phrases[i].begin(); tit!=m_phrases[i].end(); tit++) {
			std::sort (tit->get_vector ().begin (),
					   tit->get_vector ().end (),
					   m_pinyin_phrase_less_by_offset);
			tit->get_vector ().erase (
							std::unique (tit->get_vector ().begin (),
										 tit->get_vector ().end (),
										 m_pinyin_phrase_equal_by_offset),
							tit->get_vector ().end ());
			if (pinyin_phrase_validator) {
				PinyinPhraseOffsetVector tmp;
				tmp.reserve (tit->get_vector ().size ());
				for (PinyinPhraseOffsetVector::iterator vit=tit->get_vector ().begin ();
														vit!=tit->get_vector ().end ();
														vit++) {
					if (pinyin_phrase_validator (PinyinPhrase (this, vit->first, vit->second)))
						tmp.push_back (*vit);
				}
				tit->get_vector () = tmp;
			}
		}
	}
}

void
PinyinPhraseLib::refine_pinyin_lib ()
{
	PinyinKeyVector tmp_pinyin_lib;

	PinyinKeyVector::const_iterator result;
	PinyinKeyVector::const_iterator vit_begin;
	PinyinKeyVector::const_iterator vit_end;

	uint32 len;
	uint32 pinyin_offset;

	tmp_pinyin_lib.reserve (m_pinyin_lib.size () + 1);

	for (int i=SCIM_PHRASE_MAX_LENGTH-1; i>=0; i--) {
		for (PinyinPhraseTable::iterator tit=m_phrases[i].begin(); tit!=m_phrases[i].end(); tit++) {
			for (PinyinPhraseOffsetVector::iterator vit=tit->get_vector ().begin();
					vit!=tit->get_vector ().end(); vit++) {
				len = get_phrase (vit->first).length ();

				if (len > 0) {
					vit_begin = m_pinyin_lib.begin () + vit->second;
					vit_end   = vit_begin + len;

					for (result  = tmp_pinyin_lib.begin ();
						 result != tmp_pinyin_lib.end ();
						 result ++) {
						uint32 j;
						for (j=0; j< len && result + j < tmp_pinyin_lib.end (); j++) {
							if (!m_pinyin_key_equal (*(result+j), *(vit_begin + j)))
								break;
						}
						if (j == len)
							break;
					}

					/*
					result = std::find_end (tmp_pinyin_lib.begin (),
										  tmp_pinyin_lib.end (),
										  vit_begin,
										  vit_end,
										  m_pinyin_key_equal);
					*/

					if (result != tmp_pinyin_lib.end ())
						pinyin_offset = result - tmp_pinyin_lib.begin ();
					else {
						pinyin_offset = tmp_pinyin_lib.size ();
						for (uint32 j=0; j<len; j++)
							tmp_pinyin_lib.push_back (get_pinyin_key (vit->second + j));
					}
					vit->second = pinyin_offset;
				}
				std::cout << "." << std::flush;
			}
		}	
	}

	std::cout << "\n";

	m_pinyin_lib = tmp_pinyin_lib;
}

void
PinyinPhraseLib::refine_library (PinyinPhraseValidatorFunc pinyin_phrase_validator) 
{
	std::cout << "\n" << "refining phrase index." << "\n";
	refine_phrase_index (pinyin_phrase_validator);
	std::cout << "\n" << "refining pinyin lib." << "\n";
	refine_pinyin_lib ();
}

void
PinyinPhraseLib::clear_phrase_index ()
{
	for (int i=0; i<SCIM_PHRASE_MAX_LENGTH; i++)
		m_phrases [i].clear ();
}

void
PinyinPhraseLib::compact_memory ()
{
	PinyinKeyVector (m_pinyin_lib).swap (m_pinyin_lib);

	for (uint32 i=0; i<SCIM_PHRASE_MAX_LENGTH; i++) {
		for (uint32 j=0; j<m_phrases [i].size (); j++)
			(m_phrases [i])[j].compact_memory ();
	}
}

void
PinyinPhraseLib::dump_content (std::ostream &os, int minlen, int maxlen)
{
	PinyinPhraseLessThanByOffset less_op (this, m_pinyin_key_less);
	if (minlen < 1) minlen = 1;
	if (maxlen > SCIM_PHRASE_MAX_LENGTH) maxlen = SCIM_PHRASE_MAX_LENGTH;

	for (int i = minlen; i <= maxlen; ++ i) {
		PinyinPhraseOffsetVector offsets;
		for (PinyinPhraseTable::iterator tit = m_phrases [i-1].begin (); tit != m_phrases [i-1].end (); ++ tit) {
			PinyinPhraseOffsetVector::iterator begin = tit->get_vector ().begin ();
			PinyinPhraseOffsetVector::iterator end = tit->get_vector ().end ();
			offsets.insert (offsets.end (), begin, end);
		}

		std::sort (offsets.begin (), offsets.end (), less_op);

		for (PinyinPhraseOffsetVector::iterator oit = offsets.begin (); oit != offsets.end (); ++ oit) {
			bool before = false, after = false;

			os << get_phrase (oit->first).frequency () << "\t";
			if (oit > offsets.begin () && get_phrase ((oit-1)->first) == get_phrase (oit->first)) before = true;
			if (oit < offsets.end () - 1 && get_phrase ((oit+1)->first) == get_phrase (oit->first)) after = true;
			if (before || after) os << "+";
			else os << "-";
			os << utf8_wcstombs (get_phrase (oit->first).get_content ());
			os << " =";
			for (unsigned int j = 0; j < get_phrase (oit->first).length (); ++ j)
				os << " " << get_pinyin_key (oit->second + j);
			os << "\n";
		}
	}
}

void
PinyinPhraseLib::optimize_phrase_frequencies (uint32 max_freq)
{
	uint32 freq = m_phrase_lib.get_max_phrase_frequency ();

	if (freq < max_freq || !max_freq) return;

	double ratio = ((double) max_freq) / freq;

	Phrase phrase;
	
	for (int i = 0; i<(int)m_phrase_lib.number_of_phrases (); ++i) {
		phrase = m_phrase_lib.get_phrase_by_index (i);
		phrase.set_frequency ((uint32)(phrase.frequency () * ratio));
	}
}

/*
vi:ts=4:nowrap:ai
*/
