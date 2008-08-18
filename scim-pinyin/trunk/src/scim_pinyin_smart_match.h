/** @file scim_smart_match.h
 * Smart pinyin match functions.
 */

/* 
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_smart_match.h,v 1.1.1.1 2005/01/06 13:31:02 suzhe Exp $
 */

#if !defined (__SCIM_PINYIN_SMART_MATCH_H)
#define __SCIM_PINYIN_SMART_MATCH_H

using namespace scim;

typedef std::vector <CharVector>   CharVectorVector;
typedef std::vector <PhraseVector> PhraseVectorVector;

/**
 * search out all matched phrases according to keys begin to end.
 */

size_t
scim_pinyin_search_matches (CharVector      & chars,
							PhraseVector    & phrases,
							PinyinParsedKeyVector::const_iterator begin,
							PinyinParsedKeyVector::const_iterator end,
							PinyinTable     * pinyin_table,
							PinyinPhraseLib * usr_lib,
							PinyinPhraseLib * sys_lib,
							const IConvert  * validator_one = 0,
							const IConvert  * validator_two = 0,
							bool              new_search    = false,
							bool              match_longer  = false);

void
scim_pinyin_update_matches_cache (CharVectorVector   & chars_cache,
								  PhraseVectorVector & phrases_cache,
								  PinyinParsedKeyVector::const_iterator begin,
								  PinyinParsedKeyVector::const_iterator end,
								  PinyinParsedKeyVector::const_iterator invalid,
								  PinyinTable     * pinyin_table,
								  PinyinPhraseLib * usr_lib,
								  PinyinPhraseLib * sys_lib,
								  const IConvert  * validator_one = 0,
								  const IConvert  * validator_two = 0,
								  bool              full_search   = false,
								  bool              match_longer  = false);

/**
 * 
 * @param matched_phrases the phrases which is matched with the keys,
 *                        a zero length phrase indicates a char.
 *
 * @param phrases_cache   the cache to store all searched phrases for each key.
 *
 */

WideString
scim_pinyin_smart_match (PhraseVector       & matched_phrases,

						 CharVectorVector::iterator    chars_cache,

						 PhraseVectorVector::iterator  phrases_cache,

						 PinyinParsedKeyVector::const_iterator begin,
						 PinyinParsedKeyVector::const_iterator end,

						 PinyinTable        * pinyin_table,

						 PinyinPhraseLib    * usr_lib,
						 PinyinPhraseLib    * sys_lib,

						 uint32               recursive_max = 100,

						 const IConvert     * validator_one = NULL,
						 const IConvert     * validator_two = NULL);

#endif // End of __SCIM_PINYIN_SMART_MATCH_H
/*
vi:ts=4:nowrap:ai
*/
