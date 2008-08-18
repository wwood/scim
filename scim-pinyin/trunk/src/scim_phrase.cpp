/** @file scim_phrase.cpp
 * implementation of Phrase and PhraseLib classes.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_phrase.cpp,v 1.1.1.1 2005/01/06 13:30:58 suzhe Exp $
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

#define SCIM_PHRASE_MAX_RELATION 1000

#include <scim.h>
#include "scim_pinyin_private.h"
#include "scim_phrase.h"

static const char scim_phrase_lib_text_header [] = "SCIM_Phrase_Library_TEXT";
static const char scim_phrase_lib_binary_header [] = "SCIM_Phrase_Library_BINARY";
static const char scim_phrase_lib_version [] = "VERSION_0_6";

bool
PhraseEqualTo::operator () (const Phrase &lhs, const Phrase &rhs) const
{
	if (lhs.get_length() != rhs.get_length())
		return false;
	else if (lhs.m_lib == rhs.m_lib && lhs.m_offset == rhs.m_offset) 
		return true;
	for (uint32 i=0; i<lhs.get_length(); i++) {
		if (lhs.get_char (i) != rhs.get_char (i)) return false;
	}
	return true;
}

bool
PhraseLessThan::operator () (const Phrase &lhs, const Phrase &rhs) const
{
	// Longer phrases will be in front of the shorter phrases
	if (lhs.get_length () > rhs.get_length ()) return true;
	else if (lhs.get_length () < rhs.get_length()) return false;

	if (lhs.get_frequency () > rhs.get_frequency ()) return true;
	else if (lhs.get_frequency () < rhs.get_frequency ()) return false;

	for (uint32 i=0; i<lhs.get_length(); i++) {
		if (lhs.get_char (i) < rhs.get_char (i)) return true;
		else if (lhs.get_char (i) > rhs.get_char (i)) return false;
	}

	return false;
}

bool
PhraseLessThanByFrequency::operator () (const Phrase &lhs, const Phrase &rhs) const
{
	if (lhs.get_frequency () > rhs.get_frequency ()) return true;
	else if (lhs.get_frequency () < rhs.get_frequency ()) return false;

	if (lhs.get_length () > rhs.get_length ()) return true;
	else if (lhs.get_length () < rhs.get_length()) return false;

	for (uint32 i=0; i<lhs.get_length(); i++) {
		if (lhs.get_char (i) < rhs.get_char (i)) return true;
		else if (lhs.get_char (i) > rhs.get_char (i)) return false;
	}

	return false;
}

bool
PhraseExactEqualTo::operator () (const Phrase &lhs, const Phrase &rhs) const
{
	if (lhs.get_length() != rhs.get_length())
		return false;
	else if (lhs.m_lib == rhs.m_lib && lhs.m_offset == rhs.m_offset) 
		return true;
	for (uint32 i=0; i<lhs.get_length(); i++) {
		if (lhs.get_char (i) != rhs.get_char (i)) return false;
	}
	return true;
}

bool
PhraseExactLessThan::operator () (const Phrase &lhs, const Phrase &rhs) const
{
	// Longer phrases will be in front of the shorter phrases
	if (lhs.get_length () > rhs.get_length ()) return true;
	else if (lhs.get_length () < rhs.get_length()) return false;

	for (uint32 i=0; i<lhs.get_length(); i++) {
		if (lhs.get_char (i) < rhs.get_char (i)) return true;
		else if (lhs.get_char (i) > rhs.get_char (i)) return false;
	}

	return false;
}

PhraseLib::PhraseLib (const char *libfile)
	: m_burst_stack_size (255)
{
	load_lib (libfile);
}

PhraseLib::PhraseLib (std::istream &is)
	: m_burst_stack_size (255)
{
	input (is);
}

bool
PhraseLib::output (std::ostream &os, bool binary) const
{
	if (m_offsets.size () == 0 || m_content.size () == 0)
		return false;

	if (binary) {
		unsigned char bytes [16];

		os << scim_phrase_lib_binary_header << "\n";
		os << scim_phrase_lib_version << "\n";

		scim_uint32tobytes (bytes, m_offsets.size ());
		scim_uint32tobytes (bytes+4, m_content.size ());
		scim_uint32tobytes (bytes+8, m_phrase_relation_map.size ());

		os.write ((char*) bytes, sizeof (unsigned char) * 12);

		// Save phrase contents
		uint32 offset = 0;
		while (offset < m_content.size ()) {
			output_phrase_binary (os, offset);
			offset += get_phrase_length (offset) + 2;
		}

		// Save phrase relation data
		// Use two bytes to store each relation data.
		for (PhraseRelationMap::const_iterator it = m_phrase_relation_map.begin ();
			 it != m_phrase_relation_map.end (); ++ it) {
			scim_uint32tobytes (bytes, it->first.first);
			scim_uint32tobytes (bytes+4, it->first.second);
			scim_uint32tobytes (bytes+8, it->second);
			os.write ((char*) bytes, sizeof (unsigned char) * 10);
		}
	} else {
		os << scim_phrase_lib_text_header << "\n";
		os << scim_phrase_lib_version << "\n";
		os << m_offsets.size () << "\n";
		os << m_content.size () << "\n";
		os << m_phrase_relation_map.size () << "\n";

		uint32 offset = 0;
		while (offset < m_content.size ()) {
			output_phrase_text (os, offset);
			offset += get_phrase_length (offset) + 2;
			os << "\n";
		}

		os << "\n";

		// Save phrase relation data
		for (PhraseRelationMap::const_iterator it = m_phrase_relation_map.begin ();
			 it != m_phrase_relation_map.end (); ++ it) {
			os  << it->first.first  << " "
				<< it->first.second << " "
				<< it->second << "\n";
		}
	}

	return true;
}

bool
PhraseLib::input (std::istream &is)
{
	if (!is) return false;

	clear ();

	char temp [40];
	bool binary;

	// Check lib file
	is.getline (temp, 40);
	if (strncmp (temp,
		scim_phrase_lib_text_header,
		strlen (scim_phrase_lib_text_header)) == 0) {
		binary = false;
	} else if (strncmp (temp,
		scim_phrase_lib_binary_header,
		strlen (scim_phrase_lib_binary_header)) == 0) {
		binary = true;
	} else {
		return false;
	}

	is.getline (temp, 40);

	if (strncmp (temp, scim_phrase_lib_version, strlen (scim_phrase_lib_version)) != 0)
		return false;

	// Read phrase library

	uint32 number;
	uint32 content_size;
	uint32 relation_map_size;
	uint32 i;

	//get length of each file
	if (binary) {
		is.read (temp, sizeof(char) * 12);
		number = scim_bytestouint32 ((unsigned char *)temp);
		content_size = scim_bytestouint32 ((unsigned char *)temp + 4); 
		relation_map_size = scim_bytestouint32 ((unsigned char *)temp + 8); 
	} else {
		is.getline (temp, 40);
		number = atoi (temp);
		is.getline (temp, 40);
		content_size = atoi (temp);
		is.getline (temp, 40);
		relation_map_size = atoi (temp);
	}

	if (number <= 0 || content_size <= 0) return false;

	m_offsets.reserve (number + 16);
	m_content.reserve (content_size + 256);

	WideString buf;
	uint32 header;
	uint32 attr;
	uint32 offset;

	if (binary) {
		unsigned char bytes [16];

		for (i=0; i<number; i++) {
			if (input_phrase_binary (is, header, attr, buf)) {
				offset = m_content.size ();
				m_offsets.push_back (offset);
				m_content.push_back ((ucs4_t) header);
				m_content.push_back ((ucs4_t) attr);
				m_content.insert (m_content.end (), buf.begin (), buf.end ());
			}
		}

		uint32 offset1, offset2, relation;
		bytes [10] = bytes [11] = 0;
		for (i=0; i<relation_map_size; i++) {
			is.read ((char*) bytes, sizeof(char) * 10);
			offset1 = scim_bytestouint32 (bytes);
			offset2 = scim_bytestouint32 (bytes + 4);
			relation = scim_bytestouint32 (bytes + 8) & 0xFFFF;

			if (is_phrase_ok (offset1) && is_phrase_ok (offset2))
				m_phrase_relation_map [std::make_pair (offset1, offset2)] = relation;
			else
				break;
		}
	} else {
		uint32 last_offset = 0;
		for (i=0; i<number; i++) {
			if (input_phrase_text (is, header, attr, buf)) {
				if (i > 0 && get_phrase_content (last_offset) == WideString (buf)) {
					set_phrase_attribute (last_offset, attr | get_phrase_attribute (last_offset));
				} else {
					offset = m_content.size ();
					m_offsets.push_back (offset);
					m_content.push_back ((ucs4_t) header);
					m_content.push_back ((ucs4_t) attr);
					m_content.insert (m_content.end (), buf.begin (), buf.end ());
					last_offset = offset;
				}
			}
		}

		uint32 offset1, offset2, relation;
		for (i=0; i<relation_map_size; i++) {
			is >> offset1;
			is >> offset2;
			is >> relation;
			if (is_phrase_ok (offset1) && is_phrase_ok (offset2))
				m_phrase_relation_map [std::make_pair (offset1, offset2)] = (relation & 0xFFFF);
			else
				break;
		}
	}

	std::sort (m_offsets.begin (), m_offsets.end (),
				PhraseExactLessThanByOffset (this));

	// Create m_burst_stack
	std::vector <std::pair <uint32, uint32> > burst_phrases;

	for (i=0; i<m_offsets.size (); ++ i) {
		uint32 burst = get_phrase_burst (m_offsets [i]);
		if (burst)
			burst_phrases.push_back (std::pair <uint32, uint32> (burst, m_offsets [i]));
	}

	std::sort (burst_phrases.begin (), burst_phrases.end ());

	for (i=0; i<burst_phrases.size (); ++ i)
		m_burst_stack.push_back (burst_phrases [i].second);

	set_burst_stack_size (m_burst_stack_size);

	std::vector <uint32> (m_offsets).swap (m_offsets);
	std::vector <ucs4_t> (m_content).swap (m_content);

	return true;
}

bool
PhraseLib::load_lib (const char *libfile)
{
	std::ifstream is (libfile);
	if (!is) return false;
	return input (is) && m_offsets.size () != 0;
}

bool
PhraseLib::save_lib (const char *libfile, bool binary) const
{
	std::ofstream os (libfile);
	if (!os) return false;
	return output (os, binary);
}

void
PhraseLib::refresh (const Phrase &phrase, uint32 shift)
{
	find (phrase).refresh (shift);
}

Phrase
PhraseLib::find (const WideString &phrase)
{
	if (!phrase.length () || !number_of_phrases () ||
		 phrase.length () > SCIM_PHRASE_MAX_LENGTH)
		return Phrase ();

	uint32 offset = m_content.size ();
	uint32 header = SCIM_PHRASE_FLAG_ENABLE + SCIM_PHRASE_FLAG_OK;
	uint32 attr = 0;

	Phrase tmp (this, offset);

	m_content.push_back (header);
	m_content.push_back (attr);
	m_content.insert (m_content.end (), phrase.begin (), phrase.end ());

	set_phrase_length (offset, phrase.length ());

	std::vector<uint32>::iterator it =
		std::lower_bound (m_offsets.begin (), m_offsets.end (), offset,
						  PhraseExactLessThanByOffset (this));

	if (it != m_offsets.end () && Phrase (this, *it) == tmp)
		tmp = Phrase (this, *it);
	else
		tmp = Phrase ();

	m_content.erase (m_content.begin () + offset, m_content.end ());

	return tmp;
}

Phrase
PhraseLib::find (const Phrase &phrase)
{
	if (!phrase.valid () || number_of_phrases () == 0)
		return Phrase ();
	if (phrase.m_lib == this && phrase.m_offset + phrase.get_length () + 2 <= m_content.size ())
		return phrase;

	WideString content = phrase.get_content ();

	uint32 offset = m_content.size ();
	uint32 header = SCIM_PHRASE_FLAG_ENABLE + SCIM_PHRASE_FLAG_OK;
	uint32 attr = 0;

	Phrase tmp (this, offset);

	m_content.push_back (header);
	m_content.push_back (attr);
	m_content.insert (m_content.end (), content.begin (), content.end ());

	set_phrase_length (offset, content.length ());

	std::vector<uint32>::iterator it =
		std::lower_bound (m_offsets.begin (), m_offsets.end (), offset,
						  PhraseExactLessThanByOffset (this));

	if (it != m_offsets.end () && Phrase (this, *it) == phrase)
		tmp = Phrase (this, *it);
	else
		tmp = Phrase ();

	m_content.erase (m_content.begin () + offset, m_content.end ());

	return tmp;
}

Phrase
PhraseLib::append (const WideString &phrase, uint32 freq)
{
	if (!phrase.length () || phrase.length () > SCIM_PHRASE_MAX_LENGTH)
		return Phrase ();

	Phrase tmp = find (phrase);

	if (tmp.valid ()) {
		if (!tmp.is_enable ())
			tmp.enable ();
		return tmp;
	}

	if (m_offsets.capacity () <= m_offsets.size () + 1)
		m_offsets.reserve (m_offsets.size () + 16);

	if (m_content.capacity () <= m_content.size () + 1)
		m_content.reserve (m_content.size () + 256);

	uint32 offset = m_content.size ();
	uint32 header = SCIM_PHRASE_FLAG_ENABLE + SCIM_PHRASE_FLAG_OK;
	uint32 attr = 0;

	m_offsets.push_back (offset);

	m_content.push_back (header);
	m_content.push_back (attr);
	m_content.insert (m_content.end (), phrase.begin (), phrase.end ());

	set_phrase_length (offset, phrase.length ());

	set_phrase_frequency (offset, freq);

	std::sort (m_offsets.begin (), m_offsets.end (),
				PhraseExactLessThanByOffset (this));

	return Phrase (this, offset);
}

Phrase
PhraseLib::append (const Phrase &phrase, uint32 freq)
{
	if (!phrase.valid ())
		return Phrase ();

	Phrase tmp = find (phrase);

	if (tmp.valid ()) {
		if (!tmp.is_enable ())
			tmp.enable ();
		return tmp;
	}

	if (m_offsets.capacity () <= m_offsets.size () + 1)
		m_offsets.reserve (m_offsets.size () + 16);

	if (m_content.capacity () <= m_content.size () + 1)
		m_content.reserve (m_content.size () + 256);

	WideString content = phrase.get_content ();

	uint32 offset = m_content.size ();
	uint32 header = SCIM_PHRASE_FLAG_ENABLE + SCIM_PHRASE_FLAG_OK;
	uint32 attr = 0;

	m_offsets.push_back (offset);

	m_content.push_back (header);
	m_content.push_back (attr);
	m_content.insert (m_content.end (), content.begin (), content.end ());

	set_phrase_length (offset, content.length ());

	set_phrase_frequency (offset, phrase.get_frequency ());

	if (freq > 0)
		set_phrase_frequency (offset, freq);

	std::sort (m_offsets.begin (), m_offsets.end (),
				PhraseExactLessThanByOffset (this));

	return Phrase (this, offset);
}

void
PhraseLib::refine_library (bool remove_disabled)
{
	if (!valid () || number_of_phrases () == 0)
		return;

	std::sort (m_offsets.begin (), m_offsets.end (), 
			   PhraseExactLessThanByOffset(this));

	m_offsets.erase (std::unique (m_offsets.begin (), m_offsets.end (), 
								  PhraseExactEqualToByOffset (this)),
					 m_offsets.end ());

	std::vector <uint32> tmp_offsets;
	std::vector <ucs4_t> tmp_content;

	tmp_offsets.reserve (m_offsets.size () + 16);
	tmp_content.reserve (m_content.size ());

	for (std::vector <uint32>::iterator it = m_offsets.begin (); it != m_offsets.end (); it++) {
		if (is_phrase_ok (*it) && (!remove_disabled || is_phrase_enable (*it))) {
			tmp_offsets.push_back (tmp_content.size ());

			std::vector <ucs4_t>::iterator old_phrase_begin = m_content.begin () + *it;
			std::vector <ucs4_t>::iterator old_phrase_end   = old_phrase_begin + get_phrase_length (*it) + 2;
			tmp_content.insert (tmp_content.end (), old_phrase_begin, old_phrase_end);

			std::cerr << tmp_offsets.size () << "\b\b\b\b\b\b\b\b" << std::flush;
		}
	}

	m_offsets = tmp_offsets;
	m_content = tmp_content;

	std::sort (m_offsets.begin (), m_offsets.end (), 
			   PhraseExactLessThanByOffset (this));
}

uint32
PhraseLib::get_max_phrase_length ()
{
	uint32 len = 0;
	for (std::vector <uint32>::iterator i=m_offsets.begin (); i!=m_offsets.end (); i++) {
		if (is_phrase_ok (*i) && len < get_phrase_length (*i))
			len = get_phrase_length (*i);
	}
	return len;
}

uint32
PhraseLib::get_max_phrase_frequency ()
{
	uint32 freq = 0;
	for (std::vector <uint32>::iterator i=m_offsets.begin (); i!=m_offsets.end (); i++) {
		if (is_phrase_ok (*i) && freq < get_phrase_frequency (*i))
			freq = get_phrase_frequency (*i);
	}
	return freq;
}

bool
PhraseLib::input_phrase_text (std::istream &is, uint32 & header, uint32 & attr, WideString & buf)
{
	char temp [256];
	bool disabled = false;

	is.getline (temp, 255);

	if (strlen (temp) < 2) return false;

	String entry_str (temp);
	String phrase_str = entry_str.substr (0,entry_str.find_first_of ('\t'));
	String freq_str   = entry_str.substr (phrase_str.size()+1,
							entry_str.find_first_of('\t',
							phrase_str.size()+1)-phrase_str.size()+1);
	String attr_str   = entry_str.substr (entry_str.find_last_of ('\t')+1)+String (" ");

	String burst_str;

	String::size_type burst_pos = freq_str.find_first_of ('*');

	if (burst_pos != String::npos) {
		burst_str = freq_str.substr (burst_pos + 1);
	}

	uint32 freq = atoi (freq_str.c_str());
	uint32 burst = atoi (burst_str.c_str ());

	if (phrase_str.length () && phrase_str [0] == '#') {
		disabled = true;
		phrase_str.erase (phrase_str.begin ());
	}

	buf = utf8_mbstowcs (phrase_str);

	int len = buf.length ();

	if (len > 0) {
		if (len > SCIM_PHRASE_MAX_LENGTH) {
			len = SCIM_PHRASE_MAX_LENGTH;
			buf = buf.substr (0, len);
		}

		header = SCIM_PHRASE_FLAG_OK |
				 ((freq & 0x3FFFFFF) << 4) |
				 ((uint32) len & 0x0F);

		if (!disabled) header |= SCIM_PHRASE_FLAG_ENABLE;

		attr = (burst << 24);

		while (attr_str.length()) {
			String ats = attr_str.substr (0,attr_str.find_first_of (' ')+1);
			attr_str.erase (0, ats.length());
			if (ats.find ("ADJ") == 0)
				attr |= SCIM_PHRASE_ATTR_ADJ;
			if (ats.find ("ADV") == 0)
				attr |= SCIM_PHRASE_ATTR_ADV;
			if (ats.find ("AUX") == 0)
				attr |= SCIM_PHRASE_ATTR_AUX;
			if (ats.find ("CLAS") == 0)
				attr |= SCIM_PHRASE_ATTR_CLASS;
			if (ats.find ("CONJ") == 0)
				attr |= SCIM_PHRASE_ATTR_CONJ;
			if (ats.find ("COOR") == 0)
				attr |= SCIM_PHRASE_ATTR_CONJ;
			if (ats.find ("ECHO") == 0)
				attr |= SCIM_PHRASE_ATTR_ECHO;
			if (ats.find ("EXPR") == 0)
				attr |= SCIM_PHRASE_ATTR_EXPR;
			if (ats.find ("N ") == 0)
				attr |= SCIM_PHRASE_ATTR_MASK_NOUN;
			if (ats.find ("NUM") == 0)
				attr |= SCIM_PHRASE_ATTR_NUMBER;
			if (ats.find ("PREP") == 0)
				attr |= SCIM_PHRASE_ATTR_PREP;
			if (ats.find ("PRON") == 0)
				attr |= SCIM_PHRASE_ATTR_PRON;
			if (ats.find ("STRU") == 0)
				attr |= SCIM_PHRASE_ATTR_STRUCT;
			if (ats.find ("V ") == 0)
				attr |= SCIM_PHRASE_ATTR_MASK_VERB;
		}
	}

	return true;
}

void
PhraseLib::output_phrase_text (std::ostream &os, uint32 offset) const
{
	if (is_phrase_ok (offset)) {
		String mbs;

		mbs = utf8_wcstombs (get_phrase_content (offset));

		if (!is_phrase_enable (offset))
			os << '#';

		os << mbs << "\t" << get_phrase_frequency (offset);

		if (get_phrase_burst (offset))
			os << "*" << get_phrase_burst (offset);

		os << "\t";

		uint32 attr = get_phrase_attribute (offset);

		if (attr & SCIM_PHRASE_ATTR_MASK_NOUN)
			os << "N ";
		if (attr & SCIM_PHRASE_ATTR_MASK_VERB)
			os << "V ";
		if (attr & SCIM_PHRASE_ATTR_ADJ)
			os << "ADJ ";
		if (attr & SCIM_PHRASE_ATTR_ADV)
			os << "ADV ";
		if (attr & SCIM_PHRASE_ATTR_CONJ)
			os << "CONJ ";
		if (attr & SCIM_PHRASE_ATTR_PREP)
			os << "PREP ";
		if (attr & SCIM_PHRASE_ATTR_AUX)
			os << "AUX ";
		if (attr & SCIM_PHRASE_ATTR_STRUCT)
			os << "STRUCT ";
		if (attr & SCIM_PHRASE_ATTR_CLASS)
			os << "CLASS ";
		if (attr & SCIM_PHRASE_ATTR_NUMBER)
			os << "NUM ";
		if (attr & SCIM_PHRASE_ATTR_PRON)
			os << "PRON ";
		if (attr & SCIM_PHRASE_ATTR_EXPR)
			os << "EXPR ";
		if (attr & SCIM_PHRASE_ATTR_ECHO)
			os << "ECHO ";
	}
}

bool
PhraseLib::input_phrase_binary (std::istream &is, uint32 & header, uint32 & attr, WideString & buf)
{
	unsigned char bytes [8];
	ucs4_t wc;

	is.read ((char*) bytes, sizeof(unsigned char) * 8);

	header = scim_bytestouint32 (bytes);
	attr   = scim_bytestouint32 (bytes + 4); 

	uint32 len = header & 0x0F;

	buf = WideString ();

	for (uint32 i=0; i<len; i++) {
		if ((wc = utf8_read_wchar (is)) == 0)
			return false;
		buf.push_back (wc);
	}

	return header & SCIM_PHRASE_FLAG_OK;
}

void
PhraseLib::output_phrase_binary (std::ostream &os, uint32 offset) const
{
	unsigned char bytes [8];

	if (is_phrase_ok (offset)) {
		scim_uint32tobytes (bytes, get_phrase_header (offset));
		scim_uint32tobytes (bytes+4, get_phrase_attribute (offset));

		os.write ((char*) bytes, sizeof (unsigned char) * 8);

		for (uint32 i=0; i<get_phrase_length (offset); i++)
			utf8_write_wchar (os, get_phrase_content (offset, i));
	}
}

void
PhraseLib::set_burst_stack_size (uint32 size)
{
	if (size > 255) size = 255;
	else if (size == 0) size = 1;

	m_burst_stack_size = size;
	if (m_burst_stack.size () > size) {
		for (std::vector<uint32>::iterator it = m_burst_stack.begin ();
				it != m_burst_stack.begin () + m_burst_stack.size () - size;
				++ it)
			set_phrase_burst (*it, 0);

		m_burst_stack.erase (m_burst_stack.begin (),
			m_burst_stack.begin () + m_burst_stack.size () - size);
	}
}

void
PhraseLib::burst_phrase (uint32 offset)
{
	if (!m_burst_stack_size) return;

	for (unsigned int i = 0; i < m_burst_stack.size (); ++ i) {
		if (m_burst_stack [i] == offset)
			m_burst_stack.erase (m_burst_stack.begin () + i);
		else
			set_phrase_burst (m_burst_stack [i], get_phrase_burst (m_burst_stack [i]) - 1);
	}

	if (m_burst_stack.size () >= m_burst_stack_size) {
		set_phrase_burst (m_burst_stack [0], 0);
		m_burst_stack.erase (m_burst_stack.begin ());
	}

	m_burst_stack.push_back (offset);
	set_phrase_burst (offset, 255);
}

uint32
PhraseLib::get_phrase_relation (const Phrase & first, const Phrase & second, bool local)
{
	if (local && (first.get_phrase_lib () != this || second.get_phrase_lib () != this))
		return 0;

	if (!m_phrase_relation_map.size ())
		return 0;

	Phrase lhs = find (first);
	Phrase rhs = find (second);

	if (lhs.valid () && rhs.valid ()) {
		PhraseRelationMap::const_iterator it =
			m_phrase_relation_map.find (
				std::make_pair (lhs.get_phrase_offset (), rhs.get_phrase_offset ()));
		if (it != m_phrase_relation_map.end ());
			return it->second;
	}

	return 0;
}

void
PhraseLib::set_phrase_relation (const Phrase & first, const Phrase & second, uint32 relation)
{
	Phrase lhs = find (first);
	Phrase rhs = find (second);

	if (lhs.valid () && rhs.valid ()) {
		if (relation) { 
			m_phrase_relation_map [
				std::make_pair (lhs.get_phrase_offset (),
								rhs.get_phrase_offset ())] = (relation & 0xffff);
		} else {
			m_phrase_relation_map.erase (
				std::make_pair (lhs.get_phrase_offset (),
								rhs.get_phrase_offset ()));
		}
	}
}

void
PhraseLib::refresh_phrase_relation (const Phrase & first, const Phrase & second, uint32 shift)
{
	Phrase lhs = find (first);
	Phrase rhs = find (second);

	if (lhs.valid () && rhs.valid ()) {
		PhraseRelationMap::iterator it =
			m_phrase_relation_map.find (
				std::make_pair (lhs.get_phrase_offset (),
								rhs.get_phrase_offset ()));

		if (it != m_phrase_relation_map.end ()) {
			uint32 delta = (0xFFFF - (it->second & 0xFFFF));
			if (delta) {
				delta >>= shift;
				if (!delta) ++ delta;
				it->second += delta;
				if (it->second > SCIM_PHRASE_MAX_RELATION)
					it->second = SCIM_PHRASE_MAX_RELATION;
			}
		} else {
			m_phrase_relation_map [
				std::make_pair (lhs.get_phrase_offset (),
								rhs.get_phrase_offset ())] = 1;
		}
	}
}

void
PhraseLib::optimize_phrase_relation_map (uint32 max_size)
{
	if (m_phrase_relation_map.size () < max_size)
		return;

	if (!max_size) {
		m_phrase_relation_map.clear ();
		return;
	}

	std::vector <std::pair <uint32, std::pair <uint32, uint32> > > invert_map;
	std::vector <std::pair <uint32, std::pair <uint32, uint32> > >::iterator imit; 

	invert_map.reserve (m_phrase_relation_map.size ());

	for (PhraseRelationMap::iterator it = m_phrase_relation_map.begin ();
			it != m_phrase_relation_map.end (); ++ it) {
		invert_map.push_back (std::make_pair (it->second, it->first));
	}

	std::sort (invert_map.begin (), invert_map.end ());

	max_size = m_phrase_relation_map.size () - max_size;

	m_phrase_relation_map.clear ();

	for (imit = invert_map.begin () + max_size; imit != invert_map.end (); ++ imit)
		m_phrase_relation_map.insert (std::make_pair (imit->second,imit->first));
}

/*
vi:ts=4:nowrap:ai
*/
