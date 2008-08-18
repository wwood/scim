/** @file scim_pinyin_phrase.h
 * definition of PinyinPhrase, PinyinPhraseLib and related classes.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_phrase.h,v 1.2 2006/01/13 06:31:46 suzhe Exp $
 *
 */

#if !defined (__SCIM_PINYIN_PHRASE_H)
#define __SCIM_PINYIN_PHRASE_H

using namespace scim;

class PinyinPhrase;
class PinyinPhraseLib;

class PinyinPhraseLessThan;
class PinyinPhraseEqualTo;

class PinyinPhraseLessThanByOffset;
class PinyinPhraseEqualToByOffset;
class PinyinPhraseLessThanByOffsetSP;
class PinyinPhraseEqualToByOffsetSP;

typedef std::pair <uint32, uint32> PinyinPhraseOffsetPair;
typedef std::vector <PinyinPhraseOffsetPair> PinyinPhraseOffsetVector;

typedef bool (*PinyinPhraseValidatorFunc) (const PinyinPhrase &phrase);

//////////////////////////////////////////////////////////////////////////////
//declaration of PinyinPhrase class
class PinyinPhrase
{
	PinyinPhraseLib *m_lib;

	uint32 m_phrase_offset;
	uint32 m_pinyin_offset;

public:
	PinyinPhrase (PinyinPhraseLib *lib,
				  uint32 phrase_offset,
				  uint32 pinyin_offset)
		: m_lib (lib),
		  m_phrase_offset (phrase_offset),
		  m_pinyin_offset (pinyin_offset) { }

	bool valid () const; 

	bool is_enable () const { return valid () && get_phrase ().is_enable (); }

	uint32 check_attribute (uint32 attr = SCIM_PHRASE_ATTR_MASK_ALL) const {
		return get_phrase ().check_attribute (attr);
	}

	void toggle_attribute (uint32 attr, uint32 value) {
		get_phrase ().toggle_attribute (attr, value);
	}

	uint32 length () const { return get_phrase ().length (); }

	uint32 frequency () const { return get_phrase ().frequency (); }

	void refresh (uint32 shift) { get_phrase ().refresh (shift); }

	void set_frequency (uint32 freq = 0) { get_phrase ().set_frequency (freq); }

	ucs4_t operator [] (uint32 index) const { return get_phrase () [index]; }

	void enable () { get_phrase ().enable (); }

	void disable () { get_phrase ().disable (); }

	operator Phrase () const { return get_phrase (); }

	const PinyinPhraseLib * get_lib () const { return m_lib; }

	uint32 get_phrase_offset () const { return m_phrase_offset; }

	uint32 get_pinyin_offset () const { return m_pinyin_offset; }

	PinyinKey get_key (uint32 index) const; 

	Phrase get_phrase () const;
};

//////////////////////////////////////////////////////////////////////////////
//declaration of PinyinPhrase comparision class
class PinyinPhraseLessThan
	: public std::binary_function <PinyinPhrase, PinyinPhrase, bool>
{
	PinyinKeyLessThan m_less;
public:
	PinyinPhraseLessThan (const PinyinCustomSettings &custom)
		: m_less (custom) {}

	PinyinPhraseLessThan (const PinyinKeyLessThan &le)
		: m_less (le) {}

	bool operator () (const PinyinPhrase &lhs, const PinyinPhrase &rhs) const;
};

class PinyinPhraseEqualTo
	: public std::binary_function <PinyinPhrase, PinyinPhrase, bool>
{
	PinyinKeyEqualTo m_equal;
public:
	PinyinPhraseEqualTo (const PinyinCustomSettings &custom)
		: m_equal (custom) {}

	PinyinPhraseEqualTo (const PinyinKeyEqualTo &eq)
		: m_equal (eq) {}

	bool operator () (const PinyinPhrase &lhs, const PinyinPhrase &rhs) const;
};

class PinyinPhraseLessThanByOffset
	: public std::binary_function < std::pair <uint32, uint32>,
									std::pair <uint32, uint32>, bool >
{
	PinyinPhraseLib *m_lib;
	PinyinKeyLessThan m_less;
public:
	PinyinPhraseLessThanByOffset (PinyinPhraseLib *lib,
								  const PinyinCustomSettings &custom)
		: m_lib (lib), m_less (custom) { }

	PinyinPhraseLessThanByOffset (PinyinPhraseLib *lib,
								  const PinyinKeyLessThan &le)
		: m_lib (lib), m_less (le) { }

	bool operator () (const std::pair <uint32, uint32> & lhs,
					  const std::pair <uint32, uint32> & rhs) const;
};

class PinyinPhraseEqualToByOffset
	: public std::binary_function < std::pair <uint32, uint32>,
									std::pair <uint32, uint32>, bool >
{
	PinyinPhraseLib *m_lib;
	PinyinKeyEqualTo m_equal;
public:
	PinyinPhraseEqualToByOffset (PinyinPhraseLib *lib,
								 const PinyinCustomSettings &custom)
		: m_lib (lib), m_equal (custom) { }

	PinyinPhraseEqualToByOffset (PinyinPhraseLib *lib,
								 const PinyinKeyEqualTo &eq)
		: m_lib (lib), m_equal (eq) { }

	bool operator () (const std::pair <uint32, uint32> & lhs,
					  const std::pair <uint32, uint32> & rhs) const;
};

class PinyinPhrasePinyinLessThanByOffset
	: public std::binary_function < std::pair <uint32, uint32>,
									std::pair <uint32, uint32>, bool >
{
	PinyinPhraseLib *m_lib;
	PinyinKeyLessThan m_less;
public:
	PinyinPhrasePinyinLessThanByOffset (PinyinPhraseLib *lib,
										const PinyinKeyLessThan &le)
		: m_lib (lib), m_less (le) { }

	PinyinPhrasePinyinLessThanByOffset (PinyinPhraseLib *lib,
										const PinyinCustomSettings &custom)
		: m_lib (lib), m_less (custom) { }

	bool operator () (const std::pair <uint32, uint32> & lhs,
					  const std::pair <uint32, uint32> & rhs) const;
};

class PinyinPhrasePhraseLessThanByOffset
	: public std::binary_function < std::pair <uint32, uint32>,
									std::pair <uint32, uint32>, bool >
{
	PinyinPhraseLib *m_lib;
public:
	PinyinPhrasePhraseLessThanByOffset (PinyinPhraseLib *lib)
		: m_lib (lib) { }

	bool operator () (const std::pair <uint32, uint32> & lhs,
					  const std::pair <uint32, uint32> & rhs) const;
};

//a PinyinPhraseVector with a PinyinKey
class PinyinPhraseEntry
{
	struct PinyinPhraseEntryImpl
	{
		PinyinKey m_key;
		PinyinPhraseOffsetVector m_phrases;
		int m_ref;

		PinyinPhraseEntryImpl (PinyinKey key, const PinyinPhraseOffsetVector &vec)
			: m_key (key), m_phrases (vec), m_ref (1) { }

		void ref () { m_ref ++; }
		void unref () { if ((--m_ref) == 0) delete this; }
	};

	PinyinPhraseEntryImpl *m_impl;

public:
	PinyinPhraseEntry () {
		m_impl = new PinyinPhraseEntryImpl (PinyinKey (), PinyinPhraseOffsetVector ());
	}

	PinyinPhraseEntry (PinyinKey key) {
		m_impl = new PinyinPhraseEntryImpl (key, PinyinPhraseOffsetVector ());
	}

	PinyinPhraseEntry (PinyinKey key, const PinyinPhraseOffsetVector &vec) {
		m_impl = new PinyinPhraseEntryImpl (key, vec);
	}

	PinyinPhraseEntry (const PinyinPhraseEntry &entry) {
		m_impl = entry.m_impl;
		m_impl->ref ();
	}

	~PinyinPhraseEntry () {
		m_impl->unref ();
	}

	PinyinPhraseEntry& operator = (const PinyinPhraseEntry &entry) {
		if (this != &entry) {
			m_impl->unref ();
			m_impl = entry.m_impl;
			m_impl->ref ();
		}
		return *this;
	}

	operator PinyinKey () const { return m_impl->m_key; }

	PinyinPhraseOffsetVector & get_vector () {
		if (m_impl->m_ref > 1) {
			PinyinPhraseEntryImpl *impl =
				new PinyinPhraseEntryImpl (m_impl->m_key, m_impl->m_phrases);
			m_impl->unref ();
			m_impl = impl;
		}
		return m_impl->m_phrases;
	}

	void compact_memory () {
		if (m_impl)
			PinyinPhraseOffsetVector (m_impl->m_phrases).swap (m_impl->m_phrases);
	}
};

//Definition of PinyinPhraseLib
class PinyinPhraseLib
{
	typedef std::vector<PinyinPhraseEntry> PinyinPhraseTable;

	PinyinTable                  *m_pinyin_table;
	const PinyinValidator        *m_validator;

	PinyinKeyLessThan             m_pinyin_key_less;
	PinyinKeyEqualTo              m_pinyin_key_equal;

	PinyinPhraseLessThanByOffset  m_pinyin_phrase_less_by_offset;
	PinyinPhraseEqualToByOffset   m_pinyin_phrase_equal_by_offset;

	PinyinKeyVector               m_pinyin_lib;

	//to speed up phrase looking up, the phrases are divided to several groups
	//the first level of group is divided by phrase length, this level uses vector.
	//the second level is divided by the PinyinKey of the first Hanzi, this level uses map.
	PinyinPhraseTable m_phrases [SCIM_PHRASE_MAX_LENGTH];

	PhraseLib                     m_phrase_lib;

	friend class PinyinPhrase;
	friend class PinyinPhraseLessThanByOffset;
	friend class PinyinPhraseEqualToByOffset;
	friend class PinyinPhraseLessThanByOffsetSP;
	friend class PinyinPhraseEqualToByOffsetSP;

	friend class PinyinPhrasePinyinLessThanByOffset;
	friend class PinyinPhrasePhraseLessThanByOffset;
public:
	PinyinPhraseLib (const PinyinCustomSettings &custom,
					 const PinyinValidator *validator,
					 PinyinTable *pinyin_table,
					 const char *libfile = NULL,
					 const char *pylibfile = NULL,
					 const char *idxfile = NULL);
	PinyinPhraseLib (const PinyinCustomSettings &custom,
					 const PinyinValidator *validator,
					 PinyinTable *pinyin_table,
					 std::istream &is_lib,
					 std::istream &is_pylib,
					 std::istream &is_idx);

	bool output (std::ostream &os_lib, std::ostream &os_pylib, std::ostream &os_idx, bool binary = false);
	bool input (std::istream &is_lib, std::istream &is_pylib, std::istream &is_idx);
	bool input (std::istream &is_lib);

	bool load_lib (const char *libfile, const char *pylibfile = NULL, const char *idxfile = NULL);
	bool save_lib (const char *libfile, const char *pylibfile = NULL, const char *idxfile = NULL, bool binary = false);

	bool valid () const {
		return m_validator != NULL && m_pinyin_table != NULL;
	}

	void update_custom_settings (const PinyinCustomSettings &custom,
								 const PinyinValidator *validator);

	// find all phrases according to the key string
	// if noshorter == true then do not find the phrases shorter than keys
	// if nolonger == true than do not find the phrases longer than keys
	int find_phrases (PhraseVector &vec,
					  const char *keys,
					  bool noshorter = false,
					  bool nolonger = false);

	int find_phrases (PhraseVector &vec,
					  const PinyinKeyVector &keys,
					  bool noshorter = false,
					  bool nolonger = false);
	
	int find_phrases (PhraseVector &vec,
					  const PinyinParsedKeyVector &keys,
					  bool noshorter = false,
					  bool nolonger = false);

	int find_phrases (PhraseVector &vec,
					  const PinyinKeyVector::const_iterator &begin,
					  const PinyinKeyVector::const_iterator &end,
					  int minlen = 1,
					  int maxlen = -1);

	int find_phrases (PhraseVector &vec,
					  const PinyinParsedKeyVector::const_iterator &begin,
					  const PinyinParsedKeyVector::const_iterator &end,
					  int minlen = 1,
					  int maxlen = -1);

	void refresh (const Phrase &phrase, uint32 shift = 24) {
		m_phrase_lib.refresh (phrase, shift);
	}

	uint32 number_of_phrases () const {
		return m_phrase_lib.number_of_phrases ();
	}

	void clear () {
		clear_phrase_index ();
		m_phrase_lib.clear ();
		m_pinyin_lib.clear ();
	}


	void refine_library (PinyinPhraseValidatorFunc pinyin_phrase_validator = 0);

	Phrase find (const Phrase &phrase) {
		return m_phrase_lib.find (phrase);
	}

	Phrase find (const WideString &phrase) {
		return m_phrase_lib.find (phrase);
	}

	Phrase append (const Phrase &phrase) {
		return append (phrase, PinyinKeyVector ());
	}

	Phrase append (const WideString &phrase) {
		return append (phrase, PinyinKeyVector ());
	}

	Phrase append (const Phrase &phrase, const PinyinKeyVector &keys);
	Phrase append (const WideString &phrase, const PinyinKeyVector &keys);

	void dump_content (std::ostream &os, int minlen = 1, int maxlen = SCIM_PHRASE_MAX_LENGTH);

	void set_burst_stack_size (uint32 size) {
		m_phrase_lib.set_burst_stack_size (size);
	}

	uint32 get_phrase_relation (const Phrase & first, const Phrase & second, bool local = true) {
		return m_phrase_lib.get_phrase_relation (first, second, local);
	}

	void set_phrase_relation (const Phrase & first, const Phrase & second, uint32 relation = 0) {
		m_phrase_lib.set_phrase_relation (first, second, relation);
	}

	void refresh_phrase_relation (const Phrase & first, const Phrase & second, uint32 shift = 16) {
		m_phrase_lib.refresh_phrase_relation (first, second, shift);
	}

	void optimize_phrase_relation_map (uint32 max_size = 131072) {
		m_phrase_lib.optimize_phrase_relation_map (max_size);
	}

	void optimize_phrase_frequencies (uint32 max_freq = (SCIM_PHRASE_MAX_FREQUENCY >> 1));

private:
	Phrase get_phrase (uint32 phrase_offset) {
		return Phrase (&m_phrase_lib, phrase_offset);
	}

	bool valid_pinyin_phrase (uint32 phrase_offset, uint32 pinyin_offset) {
		Phrase phrase (&m_phrase_lib, phrase_offset);
		return phrase.valid () &&
				pinyin_offset <= m_pinyin_lib.size () - phrase.length ();
	}

	PinyinKey get_pinyin_key (uint32 pinyin_offset) const {
		return m_pinyin_lib [pinyin_offset];
	}

	bool input_pinyin_lib  (const PinyinValidator &validator, std::istream &is);
	bool output_pinyin_lib (std::ostream &os, bool binary = false);

private:
	void compact_memory ();

	void sort_phrase_tables ();
	void refine_phrase_index (PinyinPhraseValidatorFunc pinyin_phrase_validator);
	void refine_pinyin_lib ();
	void clear_phrase_index ();

	bool output_indexes (std::ostream &os, bool binary = false);
	bool input_indexes (std::istream &is);

	void create_pinyin_index ();

	uint32 count_phrase_number ();

	bool insert_pinyin_phrase_into_index (uint32 phrase_index, uint32 pinyin_index);
	bool insert_phrase_into_index (const Phrase &phrase, const PinyinKeyVector &keys);

	template<class T>
	void for_each_phrase (T &op);

	template<class T>
	void for_each_phrase_level_one (uint32 len, T &op);

	template<class T>
	void for_each_phrase_level_two (const PinyinPhraseTable::iterator &begin,
									const PinyinPhraseTable::iterator &end,
									T &op);
	template<class T>
	void for_each_phrase_level_three (const PinyinPhraseOffsetVector::iterator &begin,
									  const PinyinPhraseOffsetVector::iterator &end,
									  T &op);

	void find_phrases_impl (PhraseVector &pv,
							const PinyinPhraseOffsetVector::iterator &begin,
							const PinyinPhraseOffsetVector::iterator &end,
							const PinyinKeyVector::const_iterator &key_begin,
							const PinyinKeyVector::const_iterator &key_pos,
							const PinyinKeyVector::const_iterator &Key_end);
};

class PinyinPhraseLessThanByOffsetSP
{
	PinyinPhraseLib *m_lib;
	const PinyinKeyLessThan &m_less;
	uint32 m_pos;
public:
	PinyinPhraseLessThanByOffsetSP (PinyinPhraseLib *lib,
									const PinyinKeyLessThan &less,
									uint32 pos = 0)
		: m_lib (lib), m_less (less), m_pos (pos) { }

	bool operator () (const std::pair <uint32, uint32> &lhs,
					  const std::pair <uint32, uint32> &rhs) const {
		if (m_less (m_lib->get_pinyin_key (lhs.second + m_pos),
					m_lib->get_pinyin_key (rhs.second + m_pos)))
			return true;
		return false;
	}

	bool operator () (const std::pair <uint32, uint32> &lhs,
					  const PinyinKeyVector &rhs) const {
		if (m_less (m_lib->get_pinyin_key (lhs.second + m_pos), rhs [m_pos]))
			return true;
		return false;
	}

	bool operator () (const PinyinKeyVector &lhs,
					  const std::pair <uint32, uint32> &rhs) const {
		if (m_less (lhs [m_pos], m_lib->get_pinyin_key (rhs.second + m_pos)))
			return true;
		return false;
	}

	bool operator () (const std::pair <uint32, uint32> &lhs,
					  const PinyinKey &rhs) const {
		if (m_less (m_lib->get_pinyin_key (lhs.second + m_pos), rhs))
			return true;
		return false;
	}

	bool operator () (const PinyinKey &lhs,
					  const std::pair <uint32, uint32> &rhs) const {
		if (m_less (lhs, m_lib->get_pinyin_key (rhs.second + m_pos)))
			return true;
		return false;
	}
};

class PinyinPhraseEqualToByOffsetSP
{
	PinyinPhraseLib *m_lib;
	const PinyinKeyEqualTo &m_equal;
	uint32 m_pos;
public:
	PinyinPhraseEqualToByOffsetSP (PinyinPhraseLib *lib,
								   const PinyinKeyEqualTo &equal,
								   uint32 pos = 0)
		: m_lib (lib), m_equal (equal), m_pos (pos) { }

	bool operator () (const std::pair <uint32, uint32> &lhs,
					  const std::pair <uint32, uint32> &rhs) const {
		if (m_equal (m_lib->get_pinyin_key (lhs.second + m_pos),
					 m_lib->get_pinyin_key (rhs.second + m_pos)))
			return true;
		return false;
	}

	bool operator () (const std::pair <uint32, uint32> &lhs,
					  const PinyinKeyVector &rhs) const {
		if (m_equal (m_lib->get_pinyin_key (lhs.second + m_pos), rhs [m_pos]))
			return true;
		return false;
	}

	bool operator () (const PinyinKeyVector &lhs,
					  const std::pair <uint32, uint32> &rhs) const {
		if (m_equal (lhs [m_pos], m_lib->get_pinyin_key (rhs.second + m_pos)))
			return true;
		return false;
	}

	bool operator () (const std::pair <uint32, uint32> &lhs,
					  const PinyinKey &rhs) const {
		if (m_equal (m_lib->get_pinyin_key (lhs.second + m_pos), rhs))
			return true;
		return false;
	}

	bool operator() (const PinyinKey &lhs,
					 const std::pair <uint32, uint32> &rhs) const {
		if (m_equal (lhs, m_lib->get_pinyin_key (rhs.second + m_pos)))
			return true;
		return false;
	}
};

//Implementation of pinyin phrase less than and equal to operator.
inline bool
PinyinPhraseLessThanByOffset::operator () (const std::pair <uint32, uint32> & lhs,
										   const std::pair <uint32, uint32> & rhs) const
{
	if (m_lib->get_phrase (lhs.first) < m_lib->get_phrase (rhs.first))
		return true;
	else if (m_lib->get_phrase (lhs.first) == m_lib->get_phrase (rhs.first)) {
		for (uint32 i=0; i<m_lib->get_phrase (lhs.first).length (); i++) {
			if (m_less (m_lib->get_pinyin_key (lhs.second + i),
						m_lib->get_pinyin_key (rhs.second + i)))
				return true;
			else if (m_less (m_lib->get_pinyin_key (rhs.second + i),
							 m_lib->get_pinyin_key (lhs.second + i)))
				return false;
		}
	}
	return false;
}

inline bool
PinyinPhraseEqualToByOffset::operator () (const std::pair <uint32, uint32> & lhs,
										  const std::pair <uint32, uint32> & rhs) const
{
	if (lhs.first == rhs.first && lhs.second == rhs.second)
		return true;
	else if (m_lib->get_phrase (lhs.first) != m_lib->get_phrase (rhs.first))
		return false;
	else {
		for (uint32 i=0; i<m_lib->get_phrase (lhs.first).length (); i++)
			if (!m_equal (m_lib->get_pinyin_key (lhs.second + i),
						  m_lib->get_pinyin_key (rhs.second + i)))
				return false;
	}
	return true;
}

inline bool
PinyinPhrasePinyinLessThanByOffset::operator () (const std::pair <uint32, uint32> & lhs,
												 const std::pair <uint32, uint32> & rhs) const
{
	for (uint32 i=0; i<m_lib->get_phrase (lhs.first).length (); i++) {
		if (m_less (m_lib->get_pinyin_key (lhs.second + i),
					m_lib->get_pinyin_key (rhs.second + i)))
			return true;
		else if (m_less (m_lib->get_pinyin_key (rhs.second + i),
						 m_lib->get_pinyin_key (lhs.second + i)))
			return false;
	}

	return m_lib->get_phrase (lhs.first) < m_lib->get_phrase (rhs.first);
}

inline bool
PinyinPhrasePhraseLessThanByOffset::operator () (const std::pair <uint32, uint32> & lhs,
												 const std::pair <uint32, uint32> & rhs) const
{
	return m_lib->get_phrase (lhs.first) < m_lib->get_phrase (rhs.first);
}

//Implementation of some PinyinPhraseLib members.
template<class T> void
PinyinPhraseLib::for_each_phrase (T &op)
{
	for (uint32 i=0; i<SCIM_PHRASE_MAX_LENGTH; i++)
		for_each_phrase_level_two (m_phrases[i].begin(), m_phrases[i].end(), op);
}

template<class T> void
PinyinPhraseLib::for_each_phrase_level_one (uint32 len, T &op)
{
	if (len > 0 && len <= SCIM_PHRASE_MAX_LENGTH)
		for_each_phrase_level_two (m_phrases[len-1].begin(), m_phrases[len-1].end(), op);
}

template<class T> void
PinyinPhraseLib::for_each_phrase_level_two (const PinyinPhraseTable::iterator &begin,
											const PinyinPhraseTable::iterator &end,
											T &op)
{
	for (PinyinPhraseTable::iterator i=begin; i!=end; i++) 
		for_each_phrase_level_three (
						i->get_vector ().begin(),
						i->get_vector ().end(),
						op);
}

template<class T> void
PinyinPhraseLib::for_each_phrase_level_three (const PinyinPhraseOffsetVector::iterator &begin,
												  const PinyinPhraseOffsetVector::iterator &end,
												  T &op)
{
	for (PinyinPhraseOffsetVector::iterator i=begin; i!=end; i++)
		if (valid_pinyin_phrase (i->first, i->second))
			op (PinyinPhrase (this, i->first, i->second));
}


// Implementation of some PinyinPhrase memebers.
inline bool
PinyinPhrase::valid () const
{
	return m_lib != NULL &&
			m_lib->valid_pinyin_phrase (m_phrase_offset, m_pinyin_offset);
}

inline PinyinKey
PinyinPhrase::get_key (uint32 index) const
{
	if (valid () && index < length ())
		return m_lib->get_pinyin_key (m_pinyin_offset + index);
	return PinyinKey ();
}

inline Phrase
PinyinPhrase::get_phrase () const
{
	if (m_lib != NULL)
		return m_lib->get_phrase (m_phrase_offset);
	return Phrase ();
}
#endif
/*
vi:ts=4:nowrap:ai
*/
