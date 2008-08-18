/** @file scim_pinyin_smart_match.cpp
 */

/*
 * Smart Pinyin Input Method
 *
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_smart_match.cpp,v 1.1.1.1 2005/01/06 13:31:00 suzhe Exp $
 *
 */

#define Uses_STL_AUTOPTR
#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_C_STDIO
#define Uses_SCIM_UTILITY
#define Uses_SCIM_DEBUG
#define Uses_SCIM_ICONV

#define DEBUG 0

#include <scim.h>
#include "scim_pinyin_private.h"
#include "scim_phrase.h"
#include "scim_pinyin.h"
#include "scim_pinyin_phrase.h"
#include "scim_pinyin_smart_match.h"

typedef std::vector <size_t>       UIntVector;
typedef std::vector <UIntVector>   UIntVectorVector;

typedef std::vector <double>       DoubleVector;

size_t
scim_pinyin_search_matches (CharVector      & chars,
							PhraseVector    & phrases,
							PinyinParsedKeyVector::const_iterator begin,
							PinyinParsedKeyVector::const_iterator end,
							PinyinTable     * pinyin_table,
							PinyinPhraseLib * usr_lib,
							PinyinPhraseLib * sys_lib,
							const IConvert  * validator_one,
							const IConvert  * validator_two,
							bool              new_search,
							bool              match_longer)
{
	int min_length;
	int max_length = end - begin;

	if (max_length <= 0 || (!usr_lib && !sys_lib) || !pinyin_table)
		return 0;

	PhraseVector usr_phrase_vec;
	PhraseVector sys_phrase_vec;
	CharVector   char_vec;

	if (match_longer) max_length = -1;

	if (new_search || !chars.size ()) {
		chars.clear ();
		phrases.clear ();
		min_length = 1;
	} else if (phrases.size ()){
		min_length = phrases [0].length () + 1;
	} else {
		min_length = 2;
	}

	// Already has the result in phrases.
	if (max_length > 0 && min_length > max_length)
		return phrases.size () + chars.size ();

	if (max_length >= 2) {
		// Search phrases in user library.
		if (usr_lib) {
			usr_lib->find_phrases  (usr_phrase_vec,
									begin,
									end,
									(min_length > 1) ? min_length : 2,
									max_length);
		}

		// Search phrases in system library.
		if (sys_lib) {
			sys_lib->find_phrases  (sys_phrase_vec,
									begin,
									end,
									(min_length > 1) ? min_length : 2,
									max_length);
		}
	}

	if (min_length == 1)
		pinyin_table->find_chars (char_vec, *begin);

	// Insert new phrases into phrases vector.
	PhraseVector::const_iterator pit;
	CharVector::const_iterator cit;
	WideString wstr;

	for (pit = usr_phrase_vec.begin (); pit != usr_phrase_vec.end (); ++ pit) {
		wstr = pit->get_content ();
		if ((!validator_one || validator_one->test_convert (wstr)) &&
			(!validator_two || validator_two->test_convert (wstr)))
			phrases.push_back (*pit);
	}

	for (pit = sys_phrase_vec.begin (); pit != sys_phrase_vec.end (); ++ pit) {
		if (!std::binary_search (usr_phrase_vec.begin (),
								 usr_phrase_vec.end (),
								 *pit,
								 PhraseExactLessThan ())) {
			wstr = pit->get_content ();
			if ((!validator_one || validator_one->test_convert (wstr)) &&
				(!validator_two || validator_two->test_convert (wstr)))
				phrases.push_back (*pit);
		}
	}

	for (cit = char_vec.begin (); cit != char_vec.end (); ++ cit) {
		if ((!validator_one || validator_one->test_convert (&(*cit), 1)) &&
			(!validator_two || validator_two->test_convert (&(*cit), 1)))
			chars.push_back (*cit);
	}

	// Sort phrases by Length and Frequency.
	std::sort (phrases.begin (), phrases.end (), PhraseLessThan ());

#if DEBUG
	std::cerr << "Searching key " << *begin << "(" 
			  <<  min_length << "-" << max_length << ")"
			  << ", found " << phrases.size () + chars.size () << ".\n";
#endif

	return chars.size () + phrases.size ();
}

void
scim_pinyin_update_matches_cache (CharVectorVector   & chars_cache,
								  PhraseVectorVector & phrases_cache,
								  PinyinParsedKeyVector::const_iterator begin,
								  PinyinParsedKeyVector::const_iterator end,
								  PinyinParsedKeyVector::const_iterator invalid,
								  PinyinTable     * pinyin_table,
								  PinyinPhraseLib * usr_lib,
								  PinyinPhraseLib * sys_lib,
								  const IConvert  * validator_one,
								  const IConvert  * validator_two,
								  bool              full_search,
								  bool              match_longer)
{
	size_t length = end - begin;

	if (end <= begin || invalid < begin ||
		invalid > end || (!usr_lib && !sys_lib) || !pinyin_table)
		return;

	size_t i;

	//Add empty slot for new keys.
	if (length > phrases_cache.size ()) {
		size_t n = length - phrases_cache.size ();
		for (i = 0; i < n; ++i)
			phrases_cache.push_back (PhraseVector ());
	} else if (phrases_cache.size () > length) {
		phrases_cache.erase (phrases_cache.begin () + length, phrases_cache.end ());
	}

	if (length > chars_cache.size ()) {
		size_t n = length - chars_cache.size ();
		for (i = 0; i < n; ++i)
			chars_cache.push_back (CharVector ());
	} else if (chars_cache.size () > length) {
		chars_cache.erase (chars_cache.begin () + length, chars_cache.end ());
	}

	size_t invalid_pos = invalid - begin;

	PinyinParsedKeyVector::const_iterator kit;
	PhraseVectorVector::iterator pvvit;
	PhraseVector::iterator pvit;

	CharVectorVector::iterator cvvit;
	CharVector::iterator cvit;

#if DEBUG
	std::cerr << "Updating phrases cache, length = " << length
			  << ", invalid = " << invalid_pos << "\n";
#endif

	if (invalid_pos > length)
		invalid_pos = length;

	//Clear cache for invalid keys.
	for (pvvit = phrases_cache.begin () + invalid_pos,
		 cvvit = chars_cache.begin () + invalid_pos,
		 kit = invalid;
		 kit != end;
		 ++ pvvit, ++ cvvit, ++ kit) {
		if (full_search) {
			scim_pinyin_search_matches (
				*cvvit,
				*pvvit,
				kit,
				end,
				pinyin_table,
				usr_lib,
				sys_lib,
				validator_one,
				validator_two,
				true,
				match_longer);
		} else {
			pvvit->clear ();
			cvvit->clear ();
		}
	}

	//Clear invalid phrases for valid keys, and update the cache for
	//These keys.
	for (pvvit = phrases_cache.begin (),
		 cvvit = chars_cache.begin (),
		 kit = begin;
		 kit != invalid;
		 ++ pvvit, ++ cvvit, ++ kit) {
		if (pvvit->size ()) {
			for (pvit = pvvit->begin (); pvit != pvvit->end (); ++ pvit) {
				if (pvit->length () <= (invalid_pos - (kit - begin)))
					break;
			}
			pvvit->erase (pvvit->begin (), pvit);
			scim_pinyin_search_matches (
					*cvvit,
					*pvvit,
					kit,
					end,
					pinyin_table,
					usr_lib,
					sys_lib,
					validator_one,
					validator_two,
					false,
					match_longer);
		}
	}
}


static void
__calc_phrases_length_index (UIntVector         & index,
							 const PhraseVector & phrases)
{
	index.clear ();
	index.push_back (0);

	for (size_t i = 1; i < phrases.size (); ++ i) {
		if (phrases [i].length () != phrases [i-1].length ())
			index.push_back (i);
	}

	index.push_back (phrases.size ());
}


static double
__calc_sequence_score (const UIntVector                         & seq,
					   const CharVectorVector::const_iterator     chars_begin,
					   const PhraseVectorVector::const_iterator   phrases_begin,
					   const UIntVectorVector::const_iterator     indexes_begin,
					   PinyinParsedKeyVector::const_iterator      keys_begin,
					   PinyinTable                              * pinyin_table)
{
	static double phrase_length_factor [SCIM_PHRASE_MAX_LENGTH+1] =
		{        1,      320,    16000,    24000,
		     32000,    48000,    56000,    64000,
		     72000,    80000,    90000,   100000,
		    110000,   120000,   130000,   140000,
		};

	size_t start = 0;
	size_t num;
	size_t minfreq;
	size_t freq;

	size_t tmp_score;

	double score = 0;

	for (UIntVector::const_iterator si = seq.begin (); si != seq.end (); ++ si) {

		// calculate phrase score
		tmp_score = 0;

		if (*si == 1) {
			CharVector::const_iterator it = (chars_begin + start)->begin ();
			minfreq = pinyin_table->get_char_frequency (*it, *(keys_begin+start));
			tmp_score += minfreq;
			minfreq >>= 1;

			for (++it, num = 1; it != (chars_begin + start)->end (); ++ it, ++ num) {
				freq = pinyin_table->get_char_frequency (*it, *(keys_begin+start));
				if (freq < minfreq) break;
				tmp_score += freq;
			}

			score += (tmp_score+1) / num * phrase_length_factor [0];

		} else {
			for (UIntVector::const_iterator pi = (indexes_begin + start)->begin ();
				 pi != (indexes_begin + start)->end () - 1; ++ pi) {

				if (((phrases_begin + start)->begin () + *pi)->length () == *si) {

					PhraseVector::const_iterator it = ((phrases_begin + start)->begin () + *pi);

					minfreq = it->frequency ();
					tmp_score += minfreq;
					minfreq >>= 1;

					for (++ it, num = 1; it != ((phrases_begin + start)->begin () + *(pi+1)); ++ it, ++ num) {
						freq = it->frequency ();
						if (freq < minfreq) break;
						tmp_score += freq;
					}

					score += (tmp_score+1) / num * phrase_length_factor [*si-1];

					break;
				}
			}
		}

		start += *si;
	}

	return score + 1;
}

static inline double
__calc_sequence_length_score (size_t len,
							  const UIntVector & seq)
{
	UIntVector::const_iterator it = seq.begin ();
	size_t factor = (*it) * (*it);

	for (++ it; it != seq.end (); ++ it)
		factor += (*it) * (*it);

	return (double) factor / (double) (seq.size () + len);

//	return (double) (len - seq_len + 1) / (double) (len);
//	return (double) (len - seq_len + 1);
//	return (double) (len) / (double) (seq_len);
//	return 1;
}

static inline double
__calc_sequence_length_score_inv (size_t len,
								  const UIntVector & seq)
{
	UIntVector::const_iterator it = seq.begin ();
	size_t factor = (*it) * (*it);

	for (++ it; it != seq.end (); ++ it)
		factor += (*it) * (*it);

	return (double) (seq.size () + len) / (double) factor;

//	return (double) (len) / (double) (len - seq_len + 1);
//	return 1.0 / (double) (len - seq_len + 1);
//	return (double) (seq_len) / (double) (len);
//	return 1;
}

static void 
__recursive_search_best_sequence (CharVectorVector::iterator     chars_cache,
								  PhraseVectorVector::iterator   phrases_cache,
								  UIntVectorVector::iterator     indexes_cache,
								  UIntVectorVector::iterator     sequence_cache,
								  DoubleVector::iterator         score_cache,

								  PinyinParsedKeyVector::const_iterator keys_begin,
								  PinyinParsedKeyVector::const_iterator keys_end,

								  PinyinTable                  * pinyin_table,
								  PinyinPhraseLib              * usr_lib,
								  PinyinPhraseLib              * sys_lib,

								  const IConvert               * validator_one,
								  const IConvert               * validator_two,

								  size_t                         start,
								  size_t                       & length,
								  size_t                       & recursive_count,
								  size_t                         recursive_max)
{
	++ recursive_count;

	if (recursive_count > recursive_max)
		return;	

	//Update phrases cache if it's empty at position start
	if (!(phrases_cache + start)->size () || !(chars_cache + start)->size ()) {
		scim_pinyin_search_matches (
				*(chars_cache + start),
				*(phrases_cache + start),
				keys_begin + start,
				keys_end,
				pinyin_table,
				usr_lib,
				sys_lib,
				validator_one,
				validator_two,
				true,
				false);

		//Cannot find any phrases for this key, adjust the length and return.
		if (!(chars_cache + start)->size ()) {
			length = start;
			return;
		}
	}

	//Update the indexes cache if it's empty
	if (!(indexes_cache + start)->size ()) {
		__calc_phrases_length_index (
				*(indexes_cache + start),
				*(phrases_cache + start));
	}

	UIntVector seq;
	double     score;

	if ((phrases_cache + start)->size ()) {
		//Recursive search the best sequence
		for (UIntVector::const_iterator pi = (indexes_cache + start)->begin ();
				pi != (indexes_cache + start)->end () - 1; ++ pi) {
			size_t pl = ((phrases_cache + start)->begin () + (*pi))->length ();
			seq.clear ();
    
			if (start + pl > length) continue;
    
			seq.push_back (pl);
			score = __calc_sequence_score (seq,
										   chars_cache + start,
										   phrases_cache + start,
										   indexes_cache + start,
										   keys_begin + start,
										   pinyin_table);
    
			if (start + pl < length) {
				//If the following sequence is not present, then calculate it.
				if (!(sequence_cache + start + pl)->size ()) {
					__recursive_search_best_sequence (
							chars_cache,
							phrases_cache,
							indexes_cache,
							sequence_cache,
							score_cache,
							keys_begin,
							keys_end,
							pinyin_table,
							usr_lib,
							sys_lib,
							validator_one,
							validator_two,
							start + pl,
							length,
							recursive_count,
							recursive_max);
				}
				if ((sequence_cache + start + pl)->size ()) {
    
					score += (*(score_cache + start + pl) *
								__calc_sequence_length_score_inv (
									length - start - pl,
									*(sequence_cache + start + pl))
							 );
    
					seq.insert (seq.end (),
								(sequence_cache + start + pl)->begin (), 
								(sequence_cache + start + pl)->end ()); 
				}
			}
    
			score = score * __calc_sequence_length_score (length - start, seq);
    
			//A better sequence has been found, store it.
			if (score > *(score_cache + start)) {
				*(score_cache + start) = score;
				*(sequence_cache + start) = seq;
			}

#if DEBUG
			std::cerr << score << " : " << start << "\t>> ";

			for (size_t i = 0; i < seq.size (); ++ i) {
				std::cerr << seq [i] << " ";
			}
			std::cerr << "\n";
#endif

			// End search if it's a whole word
			// or a two char phrase in the end of the string.
			if (start + pl == length)
				return;
		}
	}

	if ((chars_cache + start)->size ()) {
		seq.clear ();
		seq.push_back (1);
		score = __calc_sequence_score (seq,
									   chars_cache + start,
									   phrases_cache + start,
									   indexes_cache + start,
									   keys_begin + start,
									   pinyin_table);

		if (start + 1 < length) {
			//If the following sequence is not present, then calculate it.
			if (!(sequence_cache + start + 1)->size ()) {
				__recursive_search_best_sequence (
						chars_cache,
						phrases_cache,
						indexes_cache,
						sequence_cache,
						score_cache,
						keys_begin,
						keys_end,
						pinyin_table,
						usr_lib,
						sys_lib,
						validator_one,
						validator_two,
						start + 1,
						length,
						recursive_count,
						recursive_max);
			}
			if ((sequence_cache + start + 1)->size ()) {

				score += (*(score_cache + start + 1) *
							__calc_sequence_length_score_inv (
								length - start - 1,
								*(sequence_cache + start + 1))
						 );

				seq.insert (seq.end (),
							(sequence_cache + start + 1)->begin (), 
							(sequence_cache + start + 1)->end ()); 
			}
		}

		score = score * __calc_sequence_length_score (length - start, seq);

		//A better sequence has been found, store it.
		if (score > *(score_cache + start)) {
			*(score_cache + start) = score;
			*(sequence_cache + start) = seq;
		}

#if DEBUG
		std::cerr << score << " : " << start << "\t>> ";

		for (size_t i = 0; i < seq.size (); ++ i) {
			std::cerr << seq [i] << " ";
		}
		std::cerr << "\n";
#endif
	}
}

static size_t
__calc_best_sequence (UIntVector         & seq,
					  CharVectorVector::iterator   chars_cache,
					  PhraseVectorVector::iterator phrases_cache,
					  UIntVectorVector::iterator   indexes_cache,
					  PinyinParsedKeyVector::const_iterator keys_begin,
					  PinyinParsedKeyVector::const_iterator keys_end,
					  PinyinTable        * pinyin_table,
					  PinyinPhraseLib    * usr_lib,
					  PinyinPhraseLib    * sys_lib,
					  const IConvert     * validator_one,
					  const IConvert     * validator_two,
					  size_t               recursive_max)
{
	size_t length = keys_end - keys_begin;
	size_t recursive_count = 0;

	UIntVectorVector sequence_cache ((UIntVectorVector::size_type) length);
	DoubleVector     score_cache    ((DoubleVector::size_type) length);

	__recursive_search_best_sequence (
			chars_cache,
			phrases_cache,
			indexes_cache,
			sequence_cache.begin (),
			score_cache.begin (),
			keys_begin,
			keys_end,
			pinyin_table,
			usr_lib,
			sys_lib,
			validator_one,
			validator_two,
			0,
			length,
			recursive_count,
			recursive_max);

	seq.swap (sequence_cache [0]);

	return length;
}

#if 1
static double
__calc_matched_score (const PhraseVector & phrases,
					  PinyinParsedKeyVector::const_iterator keys_begin,
					  PinyinTable        * pinyin_table,
					  PinyinPhraseLib    * usr_lib,
					  PinyinPhraseLib    * sys_lib)
{
	if (!phrases.size ()) return 0;

	PhraseVector::const_iterator it = phrases.begin ();

	double score = 0;
	size_t relation = 1;

	if (it->length () > 1) {
		score = it->frequency () + 1;
	} else if (it->length () == 1 && pinyin_table) {
		score = pinyin_table->get_char_frequency ((*it)[0], *keys_begin) + 1;
	}

	if (it->length ())
		keys_begin += it->length ();
	else
		keys_begin ++;

	for (++ it; it != phrases.end (); ++ it) {
		if (usr_lib)
			relation += usr_lib->get_phrase_relation (*(it-1), *it);
		if (sys_lib)
			relation += sys_lib->get_phrase_relation (*(it-1), *it);

		if (it->length () > 1) {
			score += it->frequency () + 1;
		} else if (it->length () == 1 && pinyin_table) {
			score += pinyin_table->get_char_frequency ((*it) [0], *keys_begin);
		}

		if (it->length ())
			keys_begin += it->length ();
		else
			keys_begin ++;
	}

	return (score / phrases.size ()) * relation;
}

static void 
__recursive_search_best_matched (PhraseVector             & matched,
								 PhraseVector             & best_matched,
								 double                   & best_score,
								 size_t                   & recursive_count,
								 const UIntVector         & seq,
								 CharVectorVector::const_iterator   chars_cache,
								 PhraseVectorVector::const_iterator phrases_cache,
								 UIntVectorVector::const_iterator   indexes_cache,
								 PinyinParsedKeyVector::const_iterator keys_begin,
								 PinyinTable              * pinyin_table,
								 PinyinPhraseLib          * usr_lib,
								 PinyinPhraseLib          * sys_lib,
								 size_t                     level,
								 size_t                     length,
								 size_t                     recursive_max)
{
	if (matched.size () == length) {
		double score = __calc_matched_score (matched,
											 keys_begin,
											 pinyin_table,
											 usr_lib,
											 sys_lib);

		if (score > best_score) {
			best_score = score;
			best_matched = matched;
		}

#if DEBUG
		std::cerr << score << "\t>> ";
		for (size_t i = 0; i < matched.size (); ++ i) {
			std::cerr << utf8_wcstombs (matched [i].get_content ()) << " ";
		}
		std::cerr << "\n";
#endif
		recursive_count ++;
		return;
	}

	if (recursive_count > recursive_max)
		return;

	if (seq [level] == 1) {
		CharVector::const_iterator it  = (chars_cache->begin ());
		CharVector::const_iterator end = (chars_cache->begin () +
											std::min ((size_t)chars_cache->size (), (size_t)4));

		Phrase ph;
		WideString content;

		for (; it != end; ++ it) {

			content += *it;
			if (usr_lib)
				ph = usr_lib->find (content);
			if (sys_lib && !ph.valid ())
				ph = sys_lib->find (content);
			content.clear ();

			if (ph.valid ())
				matched.push_back (ph);
			else
				matched.push_back (Phrase ());

			__recursive_search_best_matched (
					matched,
					best_matched,
					best_score,
					recursive_count,
					seq,
					chars_cache + seq [level],
					phrases_cache + seq [level],
					indexes_cache + seq [level],
					keys_begin,
					pinyin_table,
					usr_lib,
					sys_lib,
					level + 1,
					length,
					recursive_max);

			matched.pop_back ();
		}
	} else {
		for (UIntVector::const_iterator pi = indexes_cache->begin ();
				pi != indexes_cache->end () - 1; ++ pi) {
			if ((phrases_cache->begin () + *pi)->length () == seq [level]) {
				PhraseVector::const_iterator it  = (phrases_cache->begin () + (*pi));
				PhraseVector::const_iterator end = (phrases_cache->begin () + std::min (*(pi+1), (*pi) + (size_t) 4));
				for (; it != end; ++ it) {
					matched.push_back (*it);
					__recursive_search_best_matched (
							matched,
							best_matched,
							best_score,
							recursive_count,
							seq,
							chars_cache + seq [level],
							phrases_cache + seq [level],
							indexes_cache + seq [level],
							keys_begin,
							pinyin_table,
							usr_lib,
							sys_lib,
							level + 1,
							length,
							recursive_max);
					matched.pop_back ();
				}
				break;
			}
		}
	}
}

static void
__calc_best_matched (PhraseVector             & matched,
					 const UIntVector         & seq,
					 CharVectorVector::const_iterator      chars_cache,
					 PhraseVectorVector::const_iterator    phrases_cache,
					 UIntVectorVector::const_iterator      indexes_cache,
					 PinyinParsedKeyVector::const_iterator keys_begin,
					 PinyinTable              * pinyin_table,
					 PinyinPhraseLib          * usr_lib,
					 PinyinPhraseLib          * sys_lib,
					 uint32                     recursive_max)
{
	if (seq.size ()) {
		double best_score = 0;
		size_t recursive_count = 0;
		size_t start = 0;
		size_t finish = 0;
		size_t numphrase = 0;

		PhraseVector best_matched;
		PhraseVector result;

		UIntVector::const_iterator begin;
		UIntVector::const_iterator end;

		best_matched.reserve (seq.size ());

		result.reserve (seq.size ());

		begin = end = seq.begin ();

		do {
			finish += (*end);
			++ end;

			numphrase = 0;

			if (end != seq.end ()) {
				if (*end == 1) {
					numphrase = ((chars_cache + finish)->size ());
				} else {
					for (UIntVector::const_iterator pi = (indexes_cache + finish)->begin ();
						pi != (indexes_cache + finish)->end () - 1; ++ pi) {
						if (((phrases_cache + finish)->begin () + *pi)->length () == *end) {
							numphrase = (*(pi+1)) - (*pi);
							break;
						}
					}
				}
			}

			if (end == seq.end () || numphrase == 1) {
				if (end == seq.end ()) {
					__recursive_search_best_matched (
						matched,
						best_matched,
						best_score,
						recursive_count,
						seq,
						chars_cache + start,
						phrases_cache + start,
						indexes_cache + start,
						keys_begin + start,
						pinyin_table,
						usr_lib,
						sys_lib,
						begin - seq.begin (),
						end - begin, 
						recursive_max);

					result.insert (result.end (), best_matched.begin (), best_matched.end ());
				} else {
					__recursive_search_best_matched (
						matched,
						best_matched,
						best_score,
						recursive_count,
						seq,
						chars_cache + start,
						phrases_cache + start,
						indexes_cache + start,
						keys_begin + start,
						pinyin_table,
						usr_lib,
						sys_lib,
						begin - seq.begin (),
						end - begin + 1,
						recursive_max);

					result.insert (result.end (), best_matched.begin (), best_matched.end () - 1);
				}

				start = finish;
				begin = end;
				matched.clear ();
				best_matched.clear ();
				best_score = 0;
				recursive_count = 0;
			}
		} while (end != seq.end ());

		matched.swap (result);
	}
}
#endif

WideString
scim_pinyin_smart_match (PhraseVector       & matched_phrases,

						 CharVectorVector::iterator    chars_cache,
						 PhraseVectorVector::iterator  phrases_cache,

						 PinyinParsedKeyVector::const_iterator begin,
						 PinyinParsedKeyVector::const_iterator end,

						 PinyinTable        * pinyin_table,

						 PinyinPhraseLib    * usr_lib,
						 PinyinPhraseLib    * sys_lib,

						 uint32               recursive_max,

						 const IConvert     * validator_one,
						 const IConvert     * validator_two)
{
	UIntVectorVector indexes_cache;

	UIntVector best_seq;

	WideString result;

	size_t length, start;

	if (end <= begin) return result;

#if DEBUG
	std::cerr << "\n";
	for (PinyinParsedKeyVector::const_iterator it = begin; it != end; ++it)
		std::cerr << *it << " ";
	std::cerr << "\n";
#endif

	indexes_cache = UIntVectorVector ((UIntVectorVector::size_type) (end - begin));

	length = __calc_best_sequence (
					best_seq,
					chars_cache,
					phrases_cache,
					indexes_cache.begin (),
					begin,
					end,
					pinyin_table,
					usr_lib,
					sys_lib,
					validator_one,
					validator_two,
					recursive_max);

	// Create result string
	matched_phrases.clear ();

#if 1
	__calc_best_matched (
					matched_phrases,
					best_seq,
					chars_cache,
					phrases_cache,
					indexes_cache.begin (),
					begin,
					pinyin_table,
					usr_lib,
					sys_lib,
					recursive_max);

	start = 0;
	for (PhraseVector::iterator it = matched_phrases.begin (); it != matched_phrases.end (); ++ it) {
		if (it->length ()) {
			result += it->get_content ();
			start += it->length ();
		} else {
			result += (*(chars_cache + start)) [0];
			start ++;
		}
	}
#else

	start = 0;

	for (UIntVector::iterator it = best_seq.begin (); it != best_seq.end (); ++ it) {
		if (*it == 1) {
			WideString content;
			content += (*(chars_cache + start)) [0];
			result += content;
			if (sys_lib)
				matched_phrases.push_back (sys_lib->find (content));
			else if (usr_lib)
				matched_phrases.push_back (usr_lib->find (content));
		} else {
			for (UIntVector::const_iterator pi = indexes_cache [start].begin ();
				 pi != indexes_cache [start].end () - 1; ++ pi) {
				if ((*(phrases_cache + start))[*pi].length () == *it) {
					result += (*(phrases_cache + start))[*pi].get_content ();
					matched_phrases.push_back ((*(phrases_cache + start))[*pi]);
					break;
				}
			}
		}
		start += *it;
	}
#endif

#if DEBUG
	std::cout<< "\n";
	std::cout << "(" << 
		__calc_sequence_score (best_seq,
							   chars_cache,
							   phrases_cache,
							   indexes_cache.begin (),
							   begin,
							   pinyin_table)
		<< ") ";

	for (size_t i = 0; i < best_seq.size (); ++ i) {
		std::cout << best_seq [i] << " ";
	}

	std::cout << "\n";

	start = 0;

	for (size_t i = 0; i < best_seq.size (); ++ i) {
		std::cout << "| ";
		for (size_t j = 0; j < best_seq [i]; ++ j) {
			std::cout << *(begin + j + start) << " ";
		}

		start += best_seq [i];
	}
	std::cout << "\n";

	std::cout << utf8_wcstombs (result) << "\n";
#endif

	return result;
}

/*
vi:ts=4:nowrap:ai
*/
