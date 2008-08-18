/** @file scim_phrase.h
 * definition of Phrase and PhraseLib classes.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_phrase.h,v 1.1.1.1 2005/01/06 13:30:58 suzhe Exp $
 */

#if !defined (__SCIM_PHRASE_H)
#define __SCIM_PHRASE_H

/*  The structure of phrase lib:
 *
 *         Phrase 1             Phrase 2
 *  +------+------+------+...+------+------+--...
 *  |UINT32|UINT32|UINT32|   |UINT32|UINT32|UINT32
 *  |Header|Attrib|phrase|   |Header|Attrib|phrase
 *  +------+------+------+...+------+------+--...
 *
 *  format of Header:
 *  31                                  0
 *  +--------+--------+--------+--------+
 *  |OO  FREQ|AGE/FREQ|AGE/FREQ|FREQ|LEN|
 *  +--------+--------+--------+--------+
 *
 *  The MSB of Header is OK Flag.
 *
 *  format of Attribute:
 *  31       23       15       7        0
 *  +--------+--------+--------+--------+
 *  |BURSTVAL|        |        | VVVNNNN|
 *  +--------+--------+--------+--------+
 *
 *  Bits 0-3 : Type of NOUN
 *  Bits 4-6 : Type of VERB
 *  Bits   7 : ADJECTIVE
 *  Bits   8 : ADVERB
 *  Bits   9 : CONJUNCTION
 *  Bits  10 : PREPOSITION
 *  Bits  11 : AUXILIARY
 *  Bits  12 : STRUCTURE
 *  Bits  13 : CLASS
 *  Bits  14 : NUMBER
 *  Bits  15 : PRONOUN
 *  Bits  16 : EXPRESS
 *  Bits  17 : ECHO
 *
 *  BURSTVAL : The value to increase the frequency dramatically
 *             for recently used phrase.
 */

#define SCIM_PHRASE_FLAG_OK          0x80000000
#define SCIM_PHRASE_FLAG_ENABLE      0x40000000

#define SCIM_PHRASE_BURST_MASK       0xFF000000

#define SCIM_PHRASE_ATTR_MASK_ALL    0xFFFFFF
#define SCIM_PHRASE_ATTR_MASK_NOUN   0x00000F	//Noun
#define SCIM_PHRASE_ATTR_MASK_VERB   0x000070	//Verb
#define SCIM_PHRASE_ATTR_MASK_NOUN_VERB	0x00007F //Noun + Verb
#define SCIM_PHRASE_ATTR_ADJ         0x000080	//Adjective
#define SCIM_PHRASE_ATTR_ADV         0x000100	//Adjective
#define SCIM_PHRASE_ATTR_CONJ        0x000200	//Conjuction
#define SCIM_PHRASE_ATTR_PREP        0x000400	//Preposition
#define SCIM_PHRASE_ATTR_AUX         0x000800	//Auxiliary
#define SCIM_PHRASE_ATTR_STRUCT      0x001000	//Structure
#define SCIM_PHRASE_ATTR_CLASS       0x002000	//Class
#define SCIM_PHRASE_ATTR_NUMBER      0x004000	//Number
#define SCIM_PHRASE_ATTR_PRON        0x008000	//Pronoun
#define SCIM_PHRASE_ATTR_EXPR        0x010000	//Express
#define SCIM_PHRASE_ATTR_ECHO        0x020000	//Echo

#define SCIM_PHRASE_NOUN_SURNAME     0x01
#define SCIM_PHRASE_NOUN_GIVENNAME   0x02
#define SCIM_PHRASE_NOUN_PLACE       0x03
#define SCIM_PHRASE_NOUN_FOREIGN     0x04
#define SCIM_PHRASE_NOUN_ORGNAME     0x05
#define SCIM_PHRASE_NOUN_ADMINUNIT   0x06
#define SCIM_PHRASE_NOUN_CALLNAME    0x07
#define SCIM_PHRASE_NOUN_TIME        0x08
#define SCIM_PHRASE_NOUN_LOCATION    0x09
#define SCIM_PHRASE_NOUN_POSITION    0x0A
#define SCIM_PHRASE_NOUN_GENERAL     0x0F

#define SCIM_PHRASE_VERB_LINK        0x10 //Link verb.
#define SCIM_PHRASE_VERB_AUX         0x20 //Auxiliary verb.
#define SCIM_PHRASE_VERB_DIR         0x30 //Vert that denote the direction of another verb.
#define SCIM_PHRASE_VERB_RESULT      0x40 //Verb that describe the result of its preceding verb.
#define SCIM_PHRASE_VERB_GENERAL     0x70

#define SCIM_PHRASE_FREQUENCY_MASK   0x3FFFFFF0
#define SCIM_PHRASE_MAX_FREQUENCY    0x3FFFFFF
#define SCIM_PHRASE_MAX_LENGTH		 15
#define SCIM_PHRASE_MAX_FREQ_BITS    26

/* FIXME: gcc 3.2 and 2.96 are buggy?
 * using hash_map will cause program segfault at ~PinyinPhraseEntry;
 */

#if 0
#if defined (HAVE_HASH_MAP)
#include <hash_map>
#elif defined (HAVE_EXT_HASH_MAP)
#include <ext/hash_map>
#else
#include <map>
#endif
#endif

#include <map>

using namespace scim;

class PhraseError: public Exception {
public:
	PhraseError (const String& what_arg)
		: Exception (String("Phrase: ") + what_arg) { }
};

class Phrase;
class PhraseLib;
class PhraseLessThan;
class PhraseLessThanByFrequency;
class PhraseEqualTo;
class PhraseExactLessThan;
class PhraseExactEqualTo;

class PhraseLessThanByOffset;
class PhraseEqualToByOffset;
class PhraseExactLessThanByOffset;
class PhraseExactEqualToByOffset;

typedef std::vector<Phrase> PhraseVector;

// Pure Chinese Phrase
class Phrase
{
	PhraseLib *m_lib;
	uint32 m_offset;

	friend class PhraseLessThan;
	friend class PhraseLessThanByFrequency;
	friend class PhraseEqualTo;
	friend class PhraseExactLessThan;
	friend class PhraseExactEqualTo;
	friend class PhraseLib;

public:
	Phrase (PhraseLib *lib = NULL, uint32 offset = 0)
		: m_lib (lib), m_offset (offset) { }

	bool valid () const; 

	bool is_enable () const; 

	uint32 check_attribute (uint32 attr = SCIM_PHRASE_ATTR_MASK_ALL) const;

	void toggle_attribute (uint32 attr, uint32 value);

	uint32 length () const; 

	/**
	 * incrase frequency by 1/(2^shift).
	 */
	void refresh (uint32 shift = 26);

	void set_frequency (uint32 freq = 0);

	uint32 frequency () const;

	ucs4_t operator [] (uint32 index) const;

	WideString get_content () const;

	void enable ();

	void disable ();

	std::ostream& output_text (std::ostream &os) const;

	bool operator == (const Phrase &rhs) const;

	bool operator <  (const Phrase &rhs) const;

	bool operator != (const Phrase &rhs) const;

	const PhraseLib * get_phrase_lib () const { return m_lib; }

	uint32 get_phrase_offset () const { return m_offset; }

private:
	uint32 get_header () const;

	uint32 get_attribute () const;

	uint32 get_length () const;

	uint32 get_frequency () const;

	uint32 get_char (uint32 index) const;
};

class PhraseLessThan
	: public std::binary_function <Phrase, Phrase, bool>
{
public:
	bool operator () (const Phrase &lhs,
					  const Phrase &rhs) const;
};

class PhraseLessThanByFrequency
	: public std::binary_function <Phrase, Phrase, bool>
{
public:
	bool operator () (const Phrase &lhs,
					  const Phrase &rhs) const;
};

class PhraseEqualTo
	: public std::binary_function <Phrase, Phrase, bool>
{
public:
	bool operator () (const Phrase &lhs,
					  const Phrase &rhs) const;
};

class PhraseExactLessThan
	: public std::binary_function <Phrase, Phrase, bool>
{
public:
	bool operator () (const Phrase &lhs,
					  const Phrase &rhs) const;
};

class PhraseExactEqualTo
	: public std::binary_function <Phrase, Phrase, bool>
{
public:
	bool operator () (const Phrase &lhs,
					  const Phrase &rhs) const;
};

class PhraseLessThanByOffset
	: public std::binary_function <uint32, uint32, bool>
{
	PhraseLessThan m_lt;
	PhraseLib *m_lib;

public:
	PhraseLessThanByOffset (PhraseLib *lib)
		: m_lib (lib) { }

	bool operator () (uint32 lhs, uint32 rhs) const {
		return m_lt (Phrase (m_lib, lhs), Phrase (m_lib, rhs));
	}
};

class PhraseEqualToByOffset
	: public std::binary_function <uint32, uint32, bool>
{
	PhraseEqualTo m_eq;
	PhraseLib *m_lib;

public:
	PhraseEqualToByOffset (PhraseLib *lib)
		: m_lib (lib) { }

	bool operator () (uint32 lhs, uint32 rhs) const {
		return m_eq (Phrase (m_lib, lhs), Phrase (m_lib, rhs));
	}
};

class PhraseExactLessThanByOffset
	: public std::binary_function <uint32, uint32, bool>
{
	PhraseExactLessThan m_exlt;
	PhraseLib *m_lib;

public:
	PhraseExactLessThanByOffset (PhraseLib *lib)
		: m_lib (lib) { }

	bool operator () (uint32 lhs, uint32 rhs) const {
		return m_exlt (Phrase (m_lib, lhs), Phrase (m_lib, rhs));
	}
};

class PhraseExactEqualToByOffset
	: public std::binary_function <uint32, uint32, bool>
{
	PhraseExactEqualTo m_exeq;
	PhraseLib *m_lib;

public:
	PhraseExactEqualToByOffset (PhraseLib *lib)
		: m_lib (lib) { }

	bool operator () (uint32 lhs, uint32 rhs) const {
		return m_exeq (Phrase (m_lib, lhs), Phrase (m_lib, rhs));
	}
};

/* Phrase Library
 * structure of phrase library binary:
 * Header  (string)
 * Version (string)
 * number_of_phrases (uint32)
 * content_size (uint32 in ucs4_t)
 * contents 
 */

struct hash_uint32_pair
{
	size_t operator () (const std::pair <uint32, uint32> &s) const {
		return (s.first & 0xAAAAAAAA) | (s.second & 0x55555555);
	}
};

class PhraseLib
{
#if 0
#if defined (HAVE_HASH_MAP)
	typedef std::hash_map <std::pair <uint32,uint32>, uint32, hash_uint32_pair> PhraseRelationMap;
#elif defined (HAVE_EXT_HASH_MAP)
	typedef __gnu_cxx::hash_map <std::pair <uint32,uint32>, uint32, hash_uint32_pair> PhraseRelationMap;
#else
	typedef std::map <std::pair <uint32, uint32>, uint32> PhraseRelationMap;
#endif
#endif

	typedef std::map <std::pair <uint32, uint32>, uint32> PhraseRelationMap;

	std::vector <uint32>         m_offsets;
	std::vector <ucs4_t>         m_content;
	std::vector <uint32>         m_burst_stack;

	uint32                       m_burst_stack_size;

	PhraseRelationMap            m_phrase_relation_map;

	friend class Phrase;

public:
	PhraseLib (const char *libfile = NULL);
	PhraseLib (std::istream &is);

	const PhraseLib & operator = (const PhraseLib &rhs) {
		if (this != &rhs) {
			m_offsets = rhs.m_offsets;
			m_content = rhs.m_content;
			m_burst_stack = rhs.m_burst_stack;
			m_burst_stack_size = rhs.m_burst_stack_size;
		}
		return *this;
	}

	bool input (std::istream &is);
	bool output (std::ostream &os, bool binary = false) const;

	bool load_lib (const char *libfile);
	bool save_lib (const char *libfile, bool binary = false) const;

	bool valid () const {
		return true;
	}

	Phrase find (const WideString &phrase);
	Phrase find (const Phrase &phrase);
	Phrase append (const WideString &phrase, uint32 freq = 0);
	Phrase append (const Phrase &phrase, uint32 freq = 0);

	void refresh (const Phrase &phrase, uint32 shift = 24);
	void refresh (const WideString &phrase, uint32 shift = 24) {
		refresh (find (phrase), shift);
	}

	uint32 get_max_phrase_length ();
	uint32 get_max_phrase_frequency ();

	uint32 number_of_phrases () const {
		return m_offsets.size ();
	}

	uint32 size () const {
		return  m_offsets.size () * sizeof (uint32) +
				m_content.size () * sizeof (ucs4_t);
	}

	void clear () {
		m_offsets.clear ();
		m_content.clear ();
		m_burst_stack.clear ();
	}

	void refine_library (bool remove_disabled = false);

	Phrase get_phrase_by_index (uint32 index) {
		if (index < number_of_phrases ()) {
			Phrase phrase (this, m_offsets [index]);
			if (phrase.valid ()) return phrase;
		}
		return Phrase ();
	}

	Phrase get_phrase_by_offset (uint32 offset) {
		if (offset < m_content.size ()) {
			Phrase phrase (this, offset);
			if (phrase.valid ()) return phrase;
		}
		return Phrase ();
	}

	void set_burst_stack_size (uint32 size);

	uint32 get_phrase_relation (const Phrase & first, const Phrase & second, bool local = true);

	void set_phrase_relation (const Phrase & first, const Phrase & second, uint32 relation = 0);

	void refresh_phrase_relation (const Phrase & first, const Phrase & second, uint32 shift = 16);

	void optimize_phrase_relation_map (uint32 max_size = 131072);

private:
	void enable_phrase (uint32 offset) {
		m_content [offset] |= SCIM_PHRASE_FLAG_ENABLE;
	}

	void disable_phrase (uint32 offset) {
		m_content [offset] &= (~ SCIM_PHRASE_FLAG_ENABLE);
	}

	bool is_phrase_enable (uint32 offset) const {
		return (get_phrase_header (offset) & SCIM_PHRASE_FLAG_ENABLE) != 0;
	}

	bool is_phrase_content_ok (uint32 offset) const {
		return  offset + get_phrase_length (offset) + 2 <= m_content.size ();
	}

	bool is_phrase_ok (uint32 offset) const {
		return  is_phrase_content_ok (offset) &&
				(get_phrase_header (offset) & SCIM_PHRASE_FLAG_OK) != 0;
	}

	uint32 get_phrase_header (uint32 offset) const {
		return (uint32) m_content [offset];
	}

	void set_phrase_header (uint32 offset, uint32 header) {
		m_content [offset] = (ucs4_t) header;
	}

	uint32 get_phrase_attribute (uint32 offset) const {
		return m_content [offset + 1];
	}

	void set_phrase_attribute (uint32 offset, uint32 attr) {
		m_content [offset + 1] = ((ucs4_t) attr & SCIM_PHRASE_ATTR_MASK_ALL) |
							 	 (m_content [offset + 1] & SCIM_PHRASE_BURST_MASK);
	}

	void toggle_phrase_attribute (uint32 offset, uint32 attr, uint32 value) {
		m_content [offset + 1] = (m_content [offset + 1] & (~ (ucs4_t) attr)) |
								 ((ucs4_t) value & SCIM_PHRASE_ATTR_MASK_ALL);
	}

	uint32 get_phrase_length (uint32 offset) const {
		return (uint32) m_content [offset] & 0x0F;
	}

	void set_phrase_length (uint32 offset, uint32 length) {
		m_content [offset] &= (~ 0x0F);
		m_content [offset] |= (ucs4_t) length & 0x0F;
	}

	uint32 get_phrase_frequency (uint32 offset) const {
		return (uint32) ((m_content [offset] >> 4) & 0x3FFFFFF);
	}

	void set_phrase_frequency (uint32 offset, uint32 freq) {
		if (freq > 0x3FFFFFF) freq = 0x3FFFFFF;

		m_content [offset] &= (~ 0x3FFFFFF0);
		m_content [offset] |= (ucs4_t) ((freq & 0x3FFFFFF) << 4);
	}

	uint32 get_phrase_burst (uint32 offset) const {
		return (uint32) (m_content [offset+1] >> 24) & 0xff;
	}

	void set_phrase_burst (uint32 offset, uint32 burst) {
		m_content [offset + 1] = (ucs4_t) (burst << 24) |
						(m_content [offset + 1] & SCIM_PHRASE_ATTR_MASK_ALL);
	}

	WideString get_phrase_content (uint32 offset) const {
		return WideString (m_content.begin () + offset + 2,
							   m_content.begin () + offset + 2 + 
							   get_phrase_length (offset));
	}

	ucs4_t get_phrase_content (uint32 offset, uint32 pos) const {
		return m_content [offset + pos + 2];
	}

	void refresh_phrase (uint32 offset, uint32 shift) {
		uint32 freq = get_phrase_frequency (offset);
		uint32 delta = (SCIM_PHRASE_MAX_FREQUENCY - freq);
		if (delta) {
			delta >>= shift;
			if (!delta) ++ delta;
			set_phrase_frequency (offset, freq + delta);
		}
		burst_phrase (offset);
	}

	void burst_phrase (uint32 offset);

	bool input_phrase_text (std::istream &is, uint32 & head, uint32 & attr, WideString & buf);
	bool input_phrase_binary (std::istream &is, uint32 & head, uint32 & attr, WideString & buf);
	void output_phrase_text (std::ostream &os, uint32 offset) const;
	void output_phrase_binary (std::ostream &os, uint32 offset) const;
};

// Implementation of Phrase inline member functions.
inline bool
Phrase::operator == (const Phrase &rhs) const
{
	return PhraseEqualTo ()(*this, rhs);
}

inline bool
Phrase::operator < (const Phrase &rhs) const
{
	return PhraseLessThan ()(*this, rhs);
}

inline bool
Phrase::operator != (const Phrase &rhs) const
{
	return !((*this) == rhs);
}

inline bool
Phrase::valid () const
{
	return  m_lib != NULL && m_lib->is_phrase_ok (m_offset);
}

inline bool
Phrase::is_enable () const
{
	return  valid () && m_lib->is_phrase_enable (m_offset);
}

inline uint32
Phrase::check_attribute (uint32 attr) const
{
	if (valid ())
		return (m_lib->get_phrase_attribute (m_offset) & attr);
	return 0;
}

inline void
Phrase::toggle_attribute (uint32 attr, uint32 value)
{
	if (valid ())
		m_lib->toggle_phrase_attribute (m_offset, attr, value);
}

inline uint32
Phrase::length () const
{
	if (valid ())
		return m_lib->get_phrase_length (m_offset);
	return 0;
}

inline void
Phrase::refresh (uint32 shift)
{
	if (valid ())
		m_lib->refresh_phrase (m_offset, shift);
}

inline uint32
Phrase::frequency () const
{
	if (valid ()) return get_frequency ();
	return 0;
}

inline void
Phrase::set_frequency (uint32 freq)
{
	if (valid ())
		m_lib->set_phrase_frequency (m_offset, freq);
}

inline ucs4_t
Phrase::operator [] (uint32 index) const
{
	if (valid () && index < m_lib->get_phrase_length (m_offset))
		return m_lib->get_phrase_content (m_offset, index);
	return 0;
}

inline WideString
Phrase::get_content () const
{
	if (valid ())
		return m_lib->get_phrase_content (m_offset);
	return WideString ();
}

inline void
Phrase::enable ()
{
	if (valid ())
		m_lib->enable_phrase (m_offset);
}

inline void
Phrase::disable ()
{
	if (valid ())
		m_lib->disable_phrase (m_offset);
}

inline std::ostream&
Phrase::output_text (std::ostream &os) const
{
	if (valid ())
		m_lib->output_phrase_text (os, m_offset);
	return os;
}

inline uint32
Phrase::get_header () const
{
	return m_lib->get_phrase_header (m_offset);
}

inline uint32
Phrase::get_attribute () const
{
	return m_lib->get_phrase_attribute (m_offset);
}

inline uint32
Phrase::get_length () const
{
	return m_lib->get_phrase_length (m_offset);
}

inline uint32
Phrase::get_frequency () const
{
	uint32 freq = m_lib->get_phrase_frequency (m_offset);
	uint32 burst = m_lib->get_phrase_burst (m_offset);

	return freq * ((burst >> 4) + 1);
}

inline uint32
Phrase::get_char (uint32 index) const
{
	return m_lib->get_phrase_content (m_offset, index);
}

#endif

/*
vi:ts=4:nowrap:ai
*/
