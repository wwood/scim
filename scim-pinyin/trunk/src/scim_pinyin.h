/** @file scim_pinyin.h
 *  @brief the definitions of pinyin related classes and structs.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin.h,v 1.3 2005/08/06 15:19:01 suzhe Exp $
 *
 */

#if !defined (__SCIM_PINYIN_H)
#define __SCIM_PINYIN_H
#include "config.h"
#include "scim_stl_map.h"

#define SCIM_PINYIN_KEY_MAXLEN    7
#define SCIM_MAX_CHAR_FREQUENCY (~0)

using namespace scim;

// Predefinition of some classes and structs
class PinyinKey;
class PinyinEntry;
class PinyinTable;
class PinyinValidator;
class PinyinParser;

class PinyinKeyLessThan;
class PinyinKeyEqualTo;

class PinyinKeyExactLessThan;
class PinyinKeyExactEqualTo;

struct PinyinParsedKey;
struct PinyinCustomSettings;

typedef std::vector<ucs4_t>           CharVector;
typedef std::vector<PinyinKey>        PinyinKeyVector;
typedef std::vector<PinyinParsedKey>  PinyinParsedKeyVector;
typedef std::pair<ucs4_t, uint32>     CharFrequencyPair;

/**
 * enums of pinyin initial element.
 *
 * A pinyin key can be divided into three tokens:
 * Initial -- such as B P M F D T N L  etc.
 * Final   -- such as A O E I U V etc.
 * Tone    -- can be 1, 2, 3, 4 and 5.
 */
enum PinyinInitial
{
    SCIM_PINYIN_ZeroInitial = 0,    /**< zero initial. indicates invaild initial */
    SCIM_PINYIN_Bo  = 1,
    SCIM_PINYIN_Ci  = 2,
    SCIM_PINYIN_Chi = 3,
    SCIM_PINYIN_De  = 4,
    SCIM_PINYIN_Fo  = 5,
    SCIM_PINYIN_Ge  = 6,
    SCIM_PINYIN_He  = 7,
    SCIM_PINYIN_Ji  = 8,
    SCIM_PINYIN_Ke  = 9,
    SCIM_PINYIN_Le  =10,
    SCIM_PINYIN_Mo  =11,
    SCIM_PINYIN_Ne  =12,
    SCIM_PINYIN_Po  =13,
    SCIM_PINYIN_Qi  =14,
    SCIM_PINYIN_Ri  =15,
    SCIM_PINYIN_Si  =16,
    SCIM_PINYIN_Shi =17,
    SCIM_PINYIN_Te  =18,
    SCIM_PINYIN_Wo  =19,
    SCIM_PINYIN_Xi  =20,
    SCIM_PINYIN_Yi  =21,
    SCIM_PINYIN_Zi  =22,
    SCIM_PINYIN_Zhi =23,
    SCIM_PINYIN_LastInitial = SCIM_PINYIN_Zhi,    /**< the last initial */
    SCIM_PINYIN_InitialNumber = SCIM_PINYIN_LastInitial + 1
};

/**
 * enums of pinyin final element.
 */
enum PinyinFinal
{
    SCIM_PINYIN_ZeroFinal = 0,    /**< zero final. indicates invalid final */
    SCIM_PINYIN_A    = 1,
    SCIM_PINYIN_Ai   = 2,
    SCIM_PINYIN_An   = 3,
    SCIM_PINYIN_Ang  = 4,
    SCIM_PINYIN_Ao   = 5,
    SCIM_PINYIN_E    = 6,
    SCIM_PINYIN_Ei   = 7,
    SCIM_PINYIN_En   = 8,
    SCIM_PINYIN_Eng  = 9,
    SCIM_PINYIN_Er   =10,
    SCIM_PINYIN_I    =11,
    SCIM_PINYIN_Ia   =12,
    SCIM_PINYIN_Ian  =13,
    SCIM_PINYIN_Iang =14,
    SCIM_PINYIN_Iao  =15,
    SCIM_PINYIN_Ie   =16,
    SCIM_PINYIN_In   =17,
    SCIM_PINYIN_Ing  =18,
    SCIM_PINYIN_Iong =19,
    SCIM_PINYIN_Iou  =20,
    SCIM_PINYIN_Iu   =21,
    SCIM_PINYIN_Ng   =22,
    SCIM_PINYIN_O    =23,
    SCIM_PINYIN_Ong  =24,
    SCIM_PINYIN_Ou   =25,
    SCIM_PINYIN_U    =26,
    SCIM_PINYIN_Ua   =27,
    SCIM_PINYIN_Uai  =28,
    SCIM_PINYIN_Uan  =29,
    SCIM_PINYIN_Uang =30,
    SCIM_PINYIN_Ue   =31,
    SCIM_PINYIN_Uei  =32,
    SCIM_PINYIN_Uen  =33,
    SCIM_PINYIN_Ueng =34,
    SCIM_PINYIN_Ui   =35,
    SCIM_PINYIN_Un   =36,
    SCIM_PINYIN_Uo   =37,
    SCIM_PINYIN_V    =38,
    SCIM_PINYIN_Van  =39,
    SCIM_PINYIN_Ve   =40,
    SCIM_PINYIN_Vn   =41,
    SCIM_PINYIN_LastFinal = SCIM_PINYIN_Vn,    /**< the last final */
    SCIM_PINYIN_FinalNumber = SCIM_PINYIN_LastFinal + 1
};

/**
 * enums of pinyin tone element.
 */
enum PinyinTone
{
    SCIM_PINYIN_ZeroTone = 0,    /**< zero tone. this will be matched with all other tones. */
    SCIM_PINYIN_First  = 1,
    SCIM_PINYIN_Second = 2,
    SCIM_PINYIN_Third  = 3,
    SCIM_PINYIN_Fourth = 4,
    SCIM_PINYIN_Fifth  = 5,
    SCIM_PINYIN_LastTone = SCIM_PINYIN_Fifth, /**< the last tone */
    SCIM_PINYIN_ToneNumber = SCIM_PINYIN_LastTone + 1
};

/**
 * enums of Shuang Pin Schemes.
 */
enum PinyinShuangPinScheme
{
    SCIM_SHUANG_PIN_STONE   = 0,
    SCIM_SHUANG_PIN_ZRM     = 1,
    SCIM_SHUANG_PIN_MS      = 2,
    SCIM_SHUANG_PIN_ZIGUANG = 3,
    SCIM_SHUANG_PIN_ABC     = 4,
    SCIM_SHUANG_PIN_LIUSHI  = 5,
    SCIM_SHUANG_PIN_DEFAULT = SCIM_SHUANG_PIN_ZRM
};

/**
 * enums of pinyin ambiguities.
 * Some pinyin element maybe confused by somebody,
 * We can allow these ambiguities.
 */
enum PinyinAmbiguity
{
    SCIM_PINYIN_AmbAny= 0,
    SCIM_PINYIN_AmbZhiZi,
    SCIM_PINYIN_AmbChiCi,
    SCIM_PINYIN_AmbShiSi,
    SCIM_PINYIN_AmbNeLe,
    SCIM_PINYIN_AmbLeRi,
    SCIM_PINYIN_AmbFoHe,
    SCIM_PINYIN_AmbAnAng,
    SCIM_PINYIN_AmbEnEng,
    SCIM_PINYIN_AmbInIng,
    SCIM_PINYIN_AmbLast = SCIM_PINYIN_AmbInIng
};

/**
 * struct of pinyin custom settings.
 * user can custom the behavor of scim by these settings.
 */
struct PinyinCustomSettings
{
    bool use_tone;
        /**< ignore pinyin tone or not. */

    bool use_incomplete;
        /**< allow incomplete pinyin key which only has inital. */

    bool use_dynamic_adjust;
        /**< dynamic ajdust the word frequency or not. */

    bool use_ambiguities [SCIM_PINYIN_AmbLast + 1];
        /**< allow ambiguous pinyin elements or not. */
};

/**
 * Pinyin key class.
 * 
 * A pinyin key is a composed element of an initial, a final and a tone,
 * which represents one or several Chinese ideographs
 */
class PinyinKey
{
    unsigned int m_initial : 6;        /**< pinyin initial */
    unsigned int m_final : 6;        /**< pinyin final */
    unsigned int m_tone : 4;        /**< pinyin tone */

    friend class PinyinKeyLessThan;
    friend class PinyinKeyEqualTo;

    friend class PinyinKeyExactLessThan;
    friend class PinyinKeyExactEqualTo;

public:
    /**
     * constructor.
     *
     * the default constructor of class PinyinKey.
     */
    PinyinKey (PinyinInitial initial = SCIM_PINYIN_ZeroInitial,
               PinyinFinal final     = SCIM_PINYIN_ZeroFinal,
               PinyinTone tone       = SCIM_PINYIN_ZeroTone)
        : m_initial (initial), m_final (final), m_tone (tone)
    {
    }

    /**
     * constructor.
     *
     * construct a PinyinKey object from a key string, with
     * specified validator.
     *
     * @sa PinyinValidator
     */
    PinyinKey (const PinyinValidator &validator, const char *str, int len = -1) {
        set (validator, str, len);
    }

    /**
     * clear the PinyinKey.
     */
    void clear () {
        m_initial = SCIM_PINYIN_ZeroInitial;
        m_final = SCIM_PINYIN_ZeroFinal;
        m_tone = SCIM_PINYIN_ZeroTone;
    }

    /**
     * read the pinyin key value from a key string.
     * 
     * @param validator a PinyinValidator object to validate the key.
     * @param key a ASCII string including one or more pinyin keys.
     * @return the number of characters used by this pinyin key.
     */ 
    int set (const PinyinValidator &validator, const char *str, int len = -1);

    /**
     * set the pinyin key value to initial, final and tone.
     */
    void set (PinyinInitial initial = SCIM_PINYIN_ZeroInitial,
              PinyinFinal final     = SCIM_PINYIN_ZeroFinal,
              PinyinTone tone       = SCIM_PINYIN_ZeroTone) {
        m_initial = initial;
        m_final = final;
        m_tone = tone;
    }

    void set_initial (PinyinInitial initial = SCIM_PINYIN_ZeroInitial) {
        m_initial = initial;
    }

    void set_final (PinyinFinal final = SCIM_PINYIN_ZeroFinal) {
        m_final = final;
    }

    void set_tone (PinyinTone tone = SCIM_PINYIN_ZeroTone) {
        m_tone = tone;
    }

    /**
     * get pinyin initial of this key.
     */
    PinyinInitial get_initial () const {
        return static_cast<PinyinInitial>(m_initial);
    }

    /**
     * get pinyin final of this key.
     */
    PinyinFinal get_final () const {
        return static_cast<PinyinFinal>(m_final);
    }

    /**
     * get pinyin tone of this key.
     */
    PinyinTone get_tone () const {
        return static_cast<PinyinTone>(m_tone);
    }

    /**
     * get the ASCII name of pinyin initial of this key.
     */
    const char* get_initial_string () const;

    /**
     * get the Chinese name of pinyin initial of this key.
     */
    const ucs4_t* get_initial_wide_string () const;

    /**
     * get the ASCII name of pinyin final of this key.
     */
    const char* get_final_string () const;

    /**
     * get the Chinese name of pinyin final of this key.
     */
    const ucs4_t* get_final_wide_string () const;

    /**
     * get the ASCII name of pinyin tone of this key.
     */
    const char* get_tone_string () const;

    /**
     * get the Chinese name of pinyin tone of this key.
     */
    const ucs4_t* get_tone_wide_string () const;

    /**
     * get the ASCII name of this key.
     */
    String get_key_string () const;

    /**
     * get the Chinese name of this key.
     */
    WideString get_key_wide_string () const;

    /**
     * check if the key is empty.
     */
    bool empty () const {
        return  m_initial == SCIM_PINYIN_ZeroInitial && m_final == SCIM_PINYIN_ZeroFinal;
    }

    /**
     * output the pinyin key in text format.
     * 
     * @param os the ostream object to output to.
     */
    std::ostream& output_text (std::ostream &os) const;

    /**
     * output the pinyin key in binary format.
     * 
     * @param os the ostream object to output to.
     */
    std::ostream& output_binary (std::ostream &os) const;

    /**
     * input the pinyin key in text format.
     *
     * @param validator the PinyinValidator object to validate the key.
     * @paam is the istream object to input from.
     */
    std::istream& input_text (const PinyinValidator &validator, std::istream &is);

    /**
     * input the pinyin key in binary format.
     *
     * @param validator the PinyinValidator object to validate the key.
     * @param is the istream object to input from.
     */
    std::istream& input_binary (const PinyinValidator &validator, std::istream &is);

    /**
     * extract PinyinKey data from two bytes.
     */
    void extract_from_bytes (unsigned char byte0,
                             unsigned char byte1) {
        m_initial = (byte0 & 0x3F) % (SCIM_PINYIN_LastInitial+1);
        m_final = ((byte0>>6) | ((byte1 & 0xF)<<2)) % (SCIM_PINYIN_LastFinal+1);
        m_tone = (byte1>>4) % (SCIM_PINYIN_LastTone+1);
    }

    /**
     * store PinyinKey data into two bytes.
     */
    void store_to_bytes (unsigned char *bytes) const {
        if (bytes) {
            bytes[0] =  (static_cast<unsigned char>(m_initial)) |
                        (static_cast<unsigned char>(m_final) << 6);
            bytes[1] =  (static_cast<unsigned char>(m_final) >> 2) |
                        (static_cast<unsigned char>(m_tone) << 4);
        }
    }

    bool operator == (PinyinKey rhs) const {
        return m_initial == rhs.m_initial && m_final == rhs.m_final && m_tone == rhs.m_tone;
    }

    bool operator != (PinyinKey rhs) const {
        return ! (*this == rhs);
    }
};

/**
 * Validator of PinyinKey object.
 *
 * This class is used to validate a PinyinKey object.
 */
class PinyinValidator
{
    char m_bitmap [(SCIM_PINYIN_InitialNumber * SCIM_PINYIN_FinalNumber * SCIM_PINYIN_ToneNumber) / 8 + 1];

public:
    PinyinValidator (const PinyinTable *table = 0);

    /**
     * initialize the validator with specified custom settings
     * and PinyinTable.
     */
    void initialize (const PinyinTable *table = 0);

    /**
     * overloaded operator () function to validate a pinyin key.
     *
     * @param key the key to be validated.
     * @return true = the key is valid, otherwise it's invalid.
     */
    bool operator () (PinyinKey key) const;

public:
    static const PinyinValidator * get_default_pinyin_validator ();
};

/**
 * Parser to translate string into PinyinKey.
 */
class PinyinParser
{
public: 
    virtual ~PinyinParser ();

    /**
     * Translate only one PinyinKey from the string.
     *
     * @return the number of chars were actually used.
     */
    virtual int parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len = -1) const = 0;

    /**
     * Translate the source string into a set of PinyinParsedKeys.
     *
     * @return the number of chars were actually used.
     */
    virtual int parse (const PinyinValidator &validator, PinyinParsedKeyVector &keys, const char *str, int len = -1) const = 0;

protected:
    void normalize (PinyinKey &key) const;
};

/**
 * The default Pinyin Parser which parses full pinyin string into PinyinKeys.
 */
class PinyinDefaultParser : public PinyinParser
{
#if SCIM_USE_STL_HASH_MAP
    typedef std::hash_map<int, PinyinParsedKeyVector, std::hash <int> > ParsedKeyCache;
#elif SCIM_USE_STL_EXT_HASH_MAP
    typedef __gnu_cxx::hash_map<int, PinyinParsedKeyVector, __gnu_cxx::hash <int> > ParsedKeyCache;
#else
    typedef std::map<int, PinyinParsedKeyVector> ParsedKeyCache;
#endif

public: 
    virtual ~PinyinDefaultParser ();

    virtual int parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len = -1) const;
    virtual int parse (const PinyinValidator &validator, PinyinParsedKeyVector &keys, const char *str, int len = -1) const;

private:
    int parse_recursive (const PinyinValidator &validator,
                         int                   &real_start,
                         int                   &num_keys,
                         ParsedKeyCache        &cache,
                         const char            *str,
                         int                    len,
                         int                    level,
                         int                    start) const;

    int parse_initial (PinyinInitial &initial, const char *str, int len) const;
    int parse_final (PinyinFinal &final, const char *str, int len) const;
    int parse_tone (PinyinTone &tone, const char *str, int len) const;
};

/* The valid input chars of ShuangPin is a-z and ';'
 */
class PinyinShuangPinParser : public PinyinParser
{
    PinyinInitial m_initial_map [27];
    PinyinFinal   m_final_map [27][2];

public:
    /**
     * Constructor 
     *
     * @param scheme the predefined ShuangPin scheme to be used.
     */
    PinyinShuangPinParser (PinyinShuangPinScheme scheme = SCIM_SHUANG_PIN_DEFAULT);

    virtual ~PinyinShuangPinParser ();

    virtual int parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len = -1) const;
    virtual int parse (const PinyinValidator &validator, PinyinParsedKeyVector &keys, const char *str, int len = -1) const;

public:
    void set_scheme (PinyinShuangPinScheme scheme = SCIM_SHUANG_PIN_DEFAULT);
};

/**
 * a binary functional class to do less than comparison
 * between two pinyin keys.
 *
 * the user custom settings will be taken account into the comparison.
 */
class PinyinKeyLessThan
    : public std::binary_function <PinyinKey, PinyinKey, bool>
{
    PinyinCustomSettings m_custom;
public:
    PinyinKeyLessThan (const PinyinCustomSettings &custom)
        : m_custom (custom) {}

    bool operator () (PinyinKey lhs, PinyinKey rhs) const;
};

/**
 * a binary functional class to do equal to comparison
 * between two pinyin keys.
 */
class PinyinKeyEqualTo
    : public std::binary_function <PinyinKey, PinyinKey, bool>
{
    PinyinCustomSettings m_custom;
public:
    PinyinKeyEqualTo (const PinyinCustomSettings &custom)
        : m_custom (custom) {}

    bool operator () (PinyinKey lhs, PinyinKey rhs) const;
};

/**
 * a binary functional class to do bitwise less than comparison
 * between two pinyin keys.
 */
class PinyinKeyExactLessThan
    : public std::binary_function <PinyinKey, PinyinKey, bool>
{
public:
    bool operator () (PinyinKey lhs, PinyinKey rhs) const {
        if (lhs.m_initial < rhs.m_initial)
            return true;
        else if (lhs.m_initial == rhs.m_initial) {
            if (lhs.m_final < rhs.m_final)
                return true;
            else if (lhs.m_final == rhs.m_final && lhs.m_tone < rhs.m_tone)
                return true;
        }
        return false;
    }
};

/**
 * a binary functional class to do bitwise equal to comparison
 * between two pinyin keys.
 */
class PinyinKeyExactEqualTo
    : public std::binary_function <PinyinKey, PinyinKey, bool>
{
public:
    bool operator () (PinyinKey lhs, PinyinKey rhs) const {
        if (lhs.m_initial == rhs.m_initial &&
            lhs.m_final == rhs.m_final &&
            lhs.m_tone == rhs.m_tone)
            return true;
        return false;
    }
};

/**
 * this class is for storing a key which is parsed from a string.
 */
struct PinyinParsedKey : public PinyinKey
{
    int m_pos;        /**< the position of this key in the whole string. */
    int m_length;    /**< the length of string used by this key. */

public:
    /**
     * constructor
     */
    PinyinParsedKey (PinyinInitial initial = SCIM_PINYIN_ZeroInitial,
                     PinyinFinal final = SCIM_PINYIN_ZeroFinal,
                     PinyinTone tone = SCIM_PINYIN_ZeroTone,
                     int pos = 0,
                     int length = 0)
        : PinyinKey (initial, final, tone), m_pos (pos), m_length (length) { }

    PinyinParsedKey (const PinyinKey &key, int pos = 0, int length = 0)
        : PinyinKey (key), m_pos (pos), m_length (length) { }

    /**
     * get the key's position in the whole string.
     */
    int get_pos () const { return m_pos; }

    /**
     * get length of the key string.
     */
    int get_length () const { return m_length; }

    /**
     * get the key's end position in the whole string.
     */
    int get_end_pos () const { return m_pos + m_length; }
    
    /**
     * set the key's position.
     */
    void set_pos (int pos) { m_pos = pos; }

    /**
     * set the key's length.
     */
    void set_length (int length) { m_length = length; }
};

class CharFrequencyPairLessThanByChar {
public:
    bool operator () (const CharFrequencyPair &lhs,
                      const CharFrequencyPair &rhs) const {
        return lhs.first < rhs.first;
    }
    bool operator () (const CharFrequencyPair &lhs,
                      ucs4_t rhs) const {
        return lhs.first < rhs;
    }
    bool operator () (ucs4_t lhs,
                      const CharFrequencyPair &rhs) const {
        return lhs < rhs.first;
    }
};

class CharFrequencyPairGreaterThanByChar {
public:
    bool operator () (const CharFrequencyPair &lhs,
                      const CharFrequencyPair &rhs) const {
        return lhs.first > rhs.first;
    }
    bool operator () (const CharFrequencyPair &lhs,
                      ucs4_t rhs) const {
        return lhs.first > rhs;
    }
    bool operator () (ucs4_t lhs,
                      const CharFrequencyPair &rhs) const {
        return lhs > rhs.first;
    }
};

class CharFrequencyPairLessThanByFrequency {
public:
    bool operator () (const CharFrequencyPair &lhs,
                      const CharFrequencyPair &rhs) const {
        return lhs.second < rhs.second;
    }
    bool operator () (const CharFrequencyPair &lhs,
                      uint32 rhs) const {
        return lhs.second < rhs;
    }
    bool operator () (uint32 lhs,
                      const CharFrequencyPair &rhs) const {
        return lhs < rhs.second;
    }
};

class CharFrequencyPairGreaterThanByFrequency {
public:
    bool operator () (const CharFrequencyPair &lhs,
                      const CharFrequencyPair &rhs) const {
        return lhs.second > rhs.second;
    }
    bool operator () (const CharFrequencyPair &lhs,
                      uint32 rhs) const {
        return lhs.second > rhs;
    }
    bool operator () (uint32 lhs,
                      const CharFrequencyPair &rhs) const {
        return lhs > rhs.second;
    }
};

class CharFrequencyPairLessThanByCharAndFrequency {
public:
    bool operator () (const CharFrequencyPair &lhs,
                      const CharFrequencyPair &rhs) const {
        if (lhs.first < rhs.first) return true;
        if (lhs.first > rhs.first) return false;
        return lhs.second < rhs.second;
    }
};

class CharFrequencyPairGreaterThanByCharAndFrequency {
public:
    bool operator () (const CharFrequencyPair &lhs,
                      const CharFrequencyPair &rhs) const {
        if (lhs.first > rhs.first) return true;
        if (lhs.first < rhs.first) return false;
        return lhs.second > rhs.second;
    }
};

class CharFrequencyPairEqualToByChar {
public:
    bool operator () (const CharFrequencyPair &lhs,
                      const CharFrequencyPair &rhs) const {
        return lhs.first == rhs.first;
    }
};

/**
 * A PinyinEntry has a pinyin key and a set of ucs4_t,
 * whose pronouncation are same as the key.
 */
class PinyinEntry
{
    PinyinKey m_key;
        /**< the pinyin key of this entry */

    std::vector <CharFrequencyPair> m_chars;
        /**< the vector to store the chars and their frequencies */

public:
    /**
     * constructor
     */
    PinyinEntry (PinyinKey key)
        : m_key (key) {}

    /**
     * copy constructor
     */
    PinyinEntry (const PinyinEntry &entry)
        : m_key (entry.m_key), m_chars (entry.m_chars) {}

    /**
     * constructor.
     * read this entry from a stream.
     */
    PinyinEntry (const PinyinValidator &validator,
                 std::istream &is,
                 bool binary = false) {
        if (binary) input_binary (validator, is);
        else input_text (validator, is);
    }

    /**
     * copy operator.
     */
    const PinyinEntry& operator = (const PinyinEntry &entry) {
        if (this != &entry) {
            m_key = entry.m_key;
            m_chars = entry.m_chars;
        }
        return *this;
    }

    /**
     * set pinyin key of this entry.
     */
    void set_key (PinyinKey key) {
        m_key = key;
    }

    /**
     * get the pinyin key of this entry.
     */
    PinyinKey get_key () const {
        return m_key;
    }

    /**
     * check if this entry has the char.
     */
    bool has_char (ucs4_t c) const {
        return std::binary_search (
                        m_chars.begin (),
                        m_chars.end (),
                        c,
                        CharFrequencyPairLessThanByChar ());
    }

    /**
     * sort all chars. 
     */
    void sort () {
        std::sort (m_chars.begin(), m_chars.end());
    }

    /**
     * clear this entry.
     */
    void clear () {
        std::vector <CharFrequencyPair> ().swap (m_chars);
    }

    /**
     * return entry size (number of chars).
     */
    size_t size () const {
        return m_chars.size();
    }

    /**
     * insert a char into this entry.
     */
    void insert (const CharFrequencyPair &ch) {
        std::vector<CharFrequencyPair>::iterator i =
            std::lower_bound (
                        m_chars.begin (),
                        m_chars.end (),
                        ch.first,
                        CharFrequencyPairLessThanByChar ());

        if (i != m_chars.end () && i->first == ch.first) {
                if (ch.second > i->second)
                    i->second = ch.second;
        } else {
            m_chars.insert (i, ch);
        }
    }

    /**
     * erase a char from this entry.
     */
    void erase (ucs4_t c) {
        std::vector<CharFrequencyPair>::iterator i =
            std::lower_bound (
                        m_chars.begin (),
                        m_chars.end (),
                        c,
                        CharFrequencyPairLessThanByChar ());

        if (i != m_chars.end() && i->first == c) m_chars.erase (i);
    }

    /**
     * get the char at position index.
     */
    ucs4_t get_char_by_index (unsigned int index) const {
        return m_chars [index].first;
    }

    /**
     * get the char with its frequency.
     */
    const CharFrequencyPair & get_char_with_frequency_by_index (unsigned int index) const {
        return m_chars [index];
    }

    int get_all_chars (std::vector<ucs4_t> &vec) const {
        for (std::vector<CharFrequencyPair>::const_iterator i = m_chars.begin ();
                i != m_chars.end (); ++ i)
            vec.push_back (i->first);
        return vec.size ();
    }

    int get_all_chars_with_frequencies (std::vector<CharFrequencyPair> &vec) const {
        for (std::vector<CharFrequencyPair>::const_iterator i = m_chars.begin ();
                i != m_chars.end (); ++ i)
            vec.push_back (*i);
        return vec.size ();
    }

    uint32 get_char_frequency (ucs4_t ch) const {
        std::vector<CharFrequencyPair>::const_iterator i =
            std::lower_bound (
                        m_chars.begin (),
                        m_chars.end (),
                        ch,
                        CharFrequencyPairLessThanByChar ());

        if (i != m_chars.end() && i->first == ch)
            return i->second;

        return 0;
    }

    void set_char_frequency (ucs4_t ch, uint32 freq) {
        std::vector<CharFrequencyPair>::iterator i =
            std::lower_bound (
                        m_chars.begin (),
                        m_chars.end (),
                        ch,
                        CharFrequencyPairLessThanByChar ());

        if (i != m_chars.end() && i->first == ch)
            i->second = freq;
    }

    void refresh_char_frequency (ucs4_t ch, uint32 shift) {
        std::vector<CharFrequencyPair>::iterator i =
            std::lower_bound (
                        m_chars.begin (),
                        m_chars.end (),
                        ch,
                        CharFrequencyPairLessThanByChar ());

        if (i != m_chars.end() && i->first == ch) {
            uint32 delta = (SCIM_MAX_CHAR_FREQUENCY - i->second);
            if (delta) {
                delta >>= shift;
                if (!delta) ++ delta;
                i->second = i->second + delta;
            }
        }
    }

    /**
     * @sa get_key
     */
    operator PinyinKey () const {
        return m_key;
    }

    /**
     * output the content of this entry to ostream in text format.
     */
    std::ostream& output_text (std::ostream &os) const;

    /**
     * read the content of this entry from istream in text format.
     */
    std::istream& input_text (const PinyinValidator &validator, std::istream &is);
    
    /**
     * output in binary format.
     */
    std::ostream& output_binary (std::ostream &os) const;

    /**
     * input in binary format.
     */
    std::istream& input_binary (const PinyinValidator &validator, std::istream &is);
};

/**
 * a table to store all of the Hanzi characters and its pinyin keys.
 */
class PinyinTable
{
#if SCIM_USE_STL_HASH_MAP
    typedef std::hash_multimap<ucs4_t,PinyinKey, std::hash <unsigned long> > ReversePinyinMap;
#elif SCIM_USE_STL_EXT_HASH_MAP
    typedef __gnu_cxx::hash_multimap<ucs4_t,PinyinKey, __gnu_cxx::hash <unsigned long> > ReversePinyinMap;
#else
    typedef std::multimap<ucs4_t, PinyinKey> ReversePinyinMap;
#endif

    typedef std::pair<ucs4_t,PinyinKey> ReversePinyinPair;
    typedef std::vector<PinyinEntry> PinyinEntryVector;

    /**
     * the vector to store all of the pinyin entries.
     */
    PinyinEntryVector m_table;

    /**
     * the multimap to store reverse pinyin map.
     *
     * The reverse pinyin map is used to do Hanzi -> Pinyin mapping.
     */
    ReversePinyinMap m_revmap;

    /**
     * indicates that if the reverse map is OK.
     */
    bool m_revmap_ok;

    /**
     * less than function object of PinyinKey.
     */
    PinyinKeyLessThan m_pinyin_key_less;

    /**
     * equal to function object of PinyinKey.
     */
    PinyinKeyEqualTo m_pinyin_key_equal;

    /**
     * the validator to valdiate all of the pinyin keys.
     */
    const PinyinValidator *m_validator;

    PinyinCustomSettings m_custom;
public:
    /**
     * constructor.
     *
     * @param custom the custom settings to construct less than and equal to 
     *               function object of PinyinKey.
     * @param validator the validator to validate all of the pinyin keys.
     * @param tablefile the file name of pinyin table.
     */
    PinyinTable (const PinyinCustomSettings &custom,
                 const PinyinValidator *validator,
                 const char *tablefile = 0);

    PinyinTable (const PinyinCustomSettings &custom,
                 const PinyinValidator *validator,
                 std::istream &is);

    bool output (std::ostream &os, bool binary = false) const;
    bool input (std::istream &is);

    bool load_table (const char *tablefile);
    bool save_table (const char *tablefile, bool binary = false) const;

    void update_custom_settings (const PinyinCustomSettings &custom,
                                 const PinyinValidator *validator);

    int get_all_chars (std::vector<ucs4_t> &vec) const;
    int get_all_chars_with_frequencies (std::vector<CharFrequencyPair> &vec) const;

    int find_chars (std::vector<ucs4_t> &vec, PinyinKey key) const;
    int find_chars_with_frequencies (std::vector<CharFrequencyPair> &vec, PinyinKey key) const;

    int find_keys (PinyinKeyVector &vec, ucs4_t code);

    int find_key_strings (std::vector<PinyinKeyVector> &vec, const WideString & str);

    void erase (ucs4_t hz, const char *key);
    void erase (ucs4_t hz, PinyinKey key);

    uint32 get_char_frequency (ucs4_t ch, PinyinKey key = PinyinKey ());

    void set_char_frequency (ucs4_t ch, uint32 freq, PinyinKey key = PinyinKey ());

    /**
     * grow the char frequency by 1/(2^shift).
     */
    void refresh (ucs4_t hz, uint32 shift = 31, PinyinKey key = PinyinKey ());

    void insert (ucs4_t hz, const char *key);
    void insert (ucs4_t hz, PinyinKey key);

    size_t size () const;

    size_t number_of_entry () const { return m_table.size (); }

    // clear this table
    void clear () {
        m_table.clear ();
        m_revmap.clear ();
        m_revmap_ok = false;
    }

    bool has_key (const char *key) const;
    bool has_key (PinyinKey key) const;

private:
    /**
     * sort all pinyin entries.
     */
    void sort ();

    void create_reverse_map ();

    void insert_to_reverse_map (ucs4_t code, PinyinKey key);
    void erase_from_reverse_map (ucs4_t code, PinyinKey key);

    PinyinEntryVector::iterator find_exact_entry (PinyinKey key);

    void create_pinyin_key_vector_vector (std::vector<PinyinKeyVector> &vv,
                                          PinyinKeyVector &key_buffer,
                                          PinyinKeyVector *key_vectors,
                                          int index,
                                          int len);
};

inline std::ostream&
operator << (std::ostream& os, PinyinKey key)
{
    return key.output_text (os);
}

inline std::ostream&
operator << (std::ostream& os, const PinyinEntry &entry)
{
    return entry.output_text (os);
}

#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
