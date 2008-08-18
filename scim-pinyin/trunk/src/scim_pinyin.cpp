/** @file scim_pinyin.cpp
 * implementation of PinyinKey, PinyinTable and related classes.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin.cpp,v 1.4 2006/01/13 06:31:46 suzhe Exp $
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
#include "scim_pinyin.h"

// Internal functions
static int
__scim_pinyin_compare_initial (const PinyinCustomSettings &custom,
                               PinyinInitial lhs,
                               PinyinInitial rhs);

static int
__scim_pinyin_compare_final (const PinyinCustomSettings &custom,
                             PinyinFinal lhs,
                             PinyinFinal rhs);

static int
__scim_pinyin_compare_tone (const PinyinCustomSettings &custom,
                            PinyinTone lhs,
                            PinyinTone rhs);


// Data definition

static const char scim_pinyin_table_text_header [] = "SCIM_Pinyin_Table_TEXT";
static const char scim_pinyin_table_binary_header [] = "SCIM_Pinyin_Table_BINARY";
static const char scim_pinyin_table_version [] = "VERSION_0_4";

static const PinyinCustomSettings scim_default_custom_settings = 
{
    true, false, true,
    {false, false, false, false, false, false, false, false, false, false}
};

static const PinyinValidator scim_default_pinyin_validator;

/**
 * struct of pinyin token.
 *
 * this struct store the informations of a pinyin token
 * (an initial or final)
 */
struct PinyinToken
{
    char str[8];    /**< ASCII name of the token. */
    ucs4_t wstr[4];    /**< Chinese name in unicode. */
    int len;        /**< length of ASCII name. */
    int wlen;        /**< length of Chinese name. */
};

/**
 * struct to index PinyinToken list.
 */
struct PinyinTokenIndex
{
    int start;
    int num;
};

static const PinyinToken scim_pinyin_initials[] =
{
    {"", {0}, 0, 0},
    {"b", {0x3105,0}, 1, 1},
    {"c", {0x3118,0}, 1, 1},
    {"ch",{0x3114,0}, 2, 1},
    {"d", {0x3109,0}, 1, 1},
    {"f", {0x3108,0}, 1, 1},
    {"g", {0x310d,0}, 1, 1},
    {"h", {0x310f,0}, 1, 1},
    {"j", {0x3110,0}, 1, 1},
    {"k", {0x310e,0}, 1, 1},
    {"l", {0x310c,0}, 1, 1},
    {"m", {0x3107,0}, 1, 1},
    {"n", {0x310b,0}, 1, 1},
    {"p", {0x3106,0}, 1, 1},
    {"q", {0x3111,0}, 1, 1},
    {"r", {0x3116,0}, 1, 1},
    {"s", {0x3119,0}, 1, 1},
    {"sh",{0x3115,0}, 2, 1},
    {"t", {0x310a,0}, 1, 1},
    {"w", {0x3128,0}, 1, 1},
    {"x", {0x3112,0}, 1, 1},
    {"y", {0x3129,0}, 1, 1},
    {"z", {0x3117,0}, 1, 1},
    {"zh",{0x3113,0}, 2, 1}
};

static const PinyinToken scim_pinyin_finals[] =
{
    {"", {0}, 0, 0},
    {"a",   {0x311a,0},        1, 1},
    {"ai",  {0x311e,0},        2, 1},
    {"an",  {0x3122,0},        2, 1},
    {"ang", {0x3124,0},        3, 1},
    {"ao",  {0x3120,0},        2, 1},
    {"e",   {0x311c,0},        1, 1},
    {"ei",  {0x311f,0},        2, 1},
    {"en",  {0x3123,0},        2, 1},
    {"eng", {0x3125,0},        3, 1},
    {"er",  {0x3126,0},        2, 1},
    {"i",   {0x3127,0},        1, 1},
    {"ia",  {0x3127,0x311a,0}, 2, 2},
    {"ian", {0x3127,0x3122,0}, 3, 2},
    {"iang",{0x3127,0x3124,0}, 4, 2},
    {"iao", {0x3127,0x3120,0}, 3, 2},
    {"ie",  {0x3127,0x311d,0}, 2, 2},
    {"in",  {0x3127,0x3123,0}, 2, 2},
    {"ing", {0x3127,0x3125,0}, 3, 2},
    {"iong",{0x3129,0x3125,0}, 4, 2},
    {"iou", {0x3127,0x3121,0}, 3, 2},
    {"iu",  {0x3127,0x3121,0}, 2, 2},
    {"ng",  {0x3123,0},        2, 1},
    {"o",   {0x311b,0},        1, 1},
    {"ong", {0x3128,0x3125,0}, 3, 2},
    {"ou",  {0x3121,0},        2, 1},
    {"u",   {0x3128,0},        1, 1},
    {"ua",  {0x3128,0x311a,0}, 2, 2},
    {"uai", {0x3128,0x311e,0}, 3, 2},
    {"uan", {0x3128,0x3122,0}, 3, 2},
    {"uang",{0x3128,0x3124,0}, 4, 2},
    {"ue",  {0x3129,0x311d,0}, 2, 2},
    {"uei", {0x3128,0x311f,0}, 3, 2},
    {"uen", {0x3128,0x3123,0}, 3, 2},
    {"ueng",{0x3128,0x3125,0}, 4, 2},
    {"ui",  {0x3128,0x311f,0}, 2, 2},
    {"un",  {0x3128,0x3123,0}, 2, 2},
    {"uo",  {0x3128,0x311b,0}, 2, 2},
    {"v",   {0x3129,0},        1, 1},
    {"van", {0x3129,0x3122,0}, 3, 2},
    {"ve",  {0x3129,0x311c,0}, 2, 2},
    {"vn",  {0x3129,0x3123,0}, 2, 2}
};

static const PinyinToken scim_pinyin_tones [] =
{
    {"", {0}, 0, 0},
    {"1", {0x302A,0}, 1, 1},
    {"2", {0x302B,0}, 1, 1},
    {"3", {0x302C,0}, 1, 1},
    {"4", {0x302D,0}, 1, 1},
    {"5", {0x02D9,0}, 1, 1}
};

static const PinyinTokenIndex scim_pinyin_initials_index[] =
{
    //a     b      c      d     e       f      g      h      i      j      k      l      m 
    {-1,0},{1,1}, {2,2}, {4,1}, {-1,0},{5,1}, {6,1}, {7,1}, {-1,0},{8,1}, {9,1}, {10,1},{11,1},
    //n     o      p      q      r      s      t      u      v      w      x      y      z
    {12,1},{-1,0},{13,1},{14,1},{15,1},{16,2},{18,1},{-1,0},{-1,0},{19,1},{20,1},{21,1},{22,2}
};

static const PinyinTokenIndex scim_pinyin_finals_index[] =
{
    //a     b      c      d      e     f      g      h      i       j      k      l      m 
    {1,5}, {-1,0},{-1,0},{-1,0},{6,5},{-1,0},{-1,0},{-1,0},{11,11},{-1,0},{-1,0},{-1,0},{-1,0},
    //n     o      p      q      r      s      t      u      v      w      x      y      z
    {22,1},{23,3},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{26,12},{38,4},{-1,0},{-1,0},{-1,0},{-1,0}
};

static const int scim_number_of_initials = sizeof (scim_pinyin_initials) / sizeof (PinyinToken);
static const int scim_number_of_finals = sizeof (scim_pinyin_finals) / sizeof (PinyinToken);



static const PinyinInitial scim_shuang_pin_stone_initial_map [] =
{
    SCIM_PINYIN_ZeroInitial,    // A
    SCIM_PINYIN_Bo,             // B
    SCIM_PINYIN_Ci,             // C
    SCIM_PINYIN_De,             // D
    SCIM_PINYIN_ZeroInitial,    // E
    SCIM_PINYIN_Fo,             // F
    SCIM_PINYIN_Ge,             // G
    SCIM_PINYIN_He,             // H
    SCIM_PINYIN_Shi,            // I
    SCIM_PINYIN_Ji,             // J
    SCIM_PINYIN_Ke,             // K
    SCIM_PINYIN_Le,             // L
    SCIM_PINYIN_Mo,             // M
    SCIM_PINYIN_Ne,             // N
    SCIM_PINYIN_ZeroInitial,    // O
    SCIM_PINYIN_Po,             // P
    SCIM_PINYIN_Qi,             // Q
    SCIM_PINYIN_Ri,             // R
    SCIM_PINYIN_Si,             // S
    SCIM_PINYIN_Te,             // T
    SCIM_PINYIN_Chi,            // U
    SCIM_PINYIN_Zhi,            // V
    SCIM_PINYIN_Wo,             // W
    SCIM_PINYIN_Xi,             // X
    SCIM_PINYIN_Yi,             // Y
    SCIM_PINYIN_Zi,             // Z
    SCIM_PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal scim_shuang_pin_stone_final_map [][2] =
{
    { SCIM_PINYIN_A,   SCIM_PINYIN_ZeroFinal },         // A
    { SCIM_PINYIN_Ia,  SCIM_PINYIN_Ua        },         // B
    { SCIM_PINYIN_Uan, SCIM_PINYIN_ZeroFinal },         // C
    { SCIM_PINYIN_Ao,  SCIM_PINYIN_ZeroFinal },         // D
    { SCIM_PINYIN_E,   SCIM_PINYIN_ZeroFinal },         // E
    { SCIM_PINYIN_An,  SCIM_PINYIN_ZeroFinal },         // F
    { SCIM_PINYIN_Ang, SCIM_PINYIN_ZeroFinal },         // G
    { SCIM_PINYIN_Uang,SCIM_PINYIN_Iang      },         // H
    { SCIM_PINYIN_I,   SCIM_PINYIN_ZeroFinal },         // I
    { SCIM_PINYIN_Ian, SCIM_PINYIN_ZeroFinal },         // J
    { SCIM_PINYIN_Iao, SCIM_PINYIN_ZeroFinal },         // K
    { SCIM_PINYIN_In,  SCIM_PINYIN_ZeroFinal },         // L
    { SCIM_PINYIN_Ie,  SCIM_PINYIN_ZeroFinal },         // M
    { SCIM_PINYIN_Iu,  SCIM_PINYIN_ZeroFinal },         // N
    { SCIM_PINYIN_Uo,  SCIM_PINYIN_O         },         // O
    { SCIM_PINYIN_Ou,  SCIM_PINYIN_ZeroFinal },         // P
    { SCIM_PINYIN_Ing, SCIM_PINYIN_Er        },         // Q
    { SCIM_PINYIN_En,  SCIM_PINYIN_ZeroFinal },         // R
    { SCIM_PINYIN_Ai,  SCIM_PINYIN_ZeroFinal },         // S
    { SCIM_PINYIN_Ng,  SCIM_PINYIN_Eng       },         // T
    { SCIM_PINYIN_U,   SCIM_PINYIN_ZeroFinal },         // U
    { SCIM_PINYIN_V,   SCIM_PINYIN_Ui        },         // V
    { SCIM_PINYIN_Ei,  SCIM_PINYIN_ZeroFinal },         // W
    { SCIM_PINYIN_Uai, SCIM_PINYIN_Ue        },         // X
    { SCIM_PINYIN_Ong, SCIM_PINYIN_Iong      },         // Y
    { SCIM_PINYIN_Un,  SCIM_PINYIN_ZeroFinal },         // Z
    { SCIM_PINYIN_ZeroFinal, SCIM_PINYIN_ZeroFinal },   // ;
};


static const PinyinInitial scim_shuang_pin_zrm_initial_map [] =
{
    SCIM_PINYIN_ZeroInitial,    // A
    SCIM_PINYIN_Bo,             // B
    SCIM_PINYIN_Ci,             // C
    SCIM_PINYIN_De,             // D
    SCIM_PINYIN_ZeroInitial,    // E
    SCIM_PINYIN_Fo,             // F
    SCIM_PINYIN_Ge,             // G
    SCIM_PINYIN_He,             // H
    SCIM_PINYIN_Chi,            // I
    SCIM_PINYIN_Ji,             // J
    SCIM_PINYIN_Ke,             // K
    SCIM_PINYIN_Le,             // L
    SCIM_PINYIN_Mo,             // M
    SCIM_PINYIN_Ne,             // N
    SCIM_PINYIN_ZeroInitial,    // O
    SCIM_PINYIN_Po,             // P
    SCIM_PINYIN_Qi,             // Q
    SCIM_PINYIN_Ri,             // R
    SCIM_PINYIN_Si,             // S
    SCIM_PINYIN_Te,             // T
    SCIM_PINYIN_Shi,            // U
    SCIM_PINYIN_Zhi,            // V
    SCIM_PINYIN_Wo,             // W
    SCIM_PINYIN_Xi,             // X
    SCIM_PINYIN_Yi,             // Y
    SCIM_PINYIN_Zi,             // Z
    SCIM_PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal scim_shuang_pin_zrm_final_map [][2] =
{
    { SCIM_PINYIN_A,   SCIM_PINYIN_ZeroFinal },         // A
    { SCIM_PINYIN_Ou,  SCIM_PINYIN_ZeroFinal },         // B
    { SCIM_PINYIN_Iao, SCIM_PINYIN_ZeroFinal },         // C
    { SCIM_PINYIN_Uang,SCIM_PINYIN_Iang      },         // D
    { SCIM_PINYIN_E,   SCIM_PINYIN_ZeroFinal },         // E
    { SCIM_PINYIN_En,  SCIM_PINYIN_ZeroFinal },         // F
    { SCIM_PINYIN_Ng,  SCIM_PINYIN_Eng       },         // G
    { SCIM_PINYIN_Ang, SCIM_PINYIN_ZeroFinal },         // H
    { SCIM_PINYIN_I,   SCIM_PINYIN_ZeroFinal },         // I
    { SCIM_PINYIN_An,  SCIM_PINYIN_ZeroFinal },         // J
    { SCIM_PINYIN_Ao,  SCIM_PINYIN_ZeroFinal },         // K
    { SCIM_PINYIN_Ai,  SCIM_PINYIN_ZeroFinal },         // L
    { SCIM_PINYIN_Ian, SCIM_PINYIN_ZeroFinal },         // M
    { SCIM_PINYIN_In,  SCIM_PINYIN_ZeroFinal },         // N
    { SCIM_PINYIN_Uo,  SCIM_PINYIN_O         },         // O
    { SCIM_PINYIN_Un,  SCIM_PINYIN_ZeroFinal },         // P
    { SCIM_PINYIN_Iu,  SCIM_PINYIN_ZeroFinal },         // Q
    { SCIM_PINYIN_Uan, SCIM_PINYIN_Er        },         // R
    { SCIM_PINYIN_Ong, SCIM_PINYIN_Iong      },         // S
    { SCIM_PINYIN_Ue,  SCIM_PINYIN_ZeroFinal },         // T
    { SCIM_PINYIN_U,   SCIM_PINYIN_ZeroFinal },         // U
    { SCIM_PINYIN_V,   SCIM_PINYIN_Ui        },         // V
    { SCIM_PINYIN_Ia,  SCIM_PINYIN_Ua        },         // W
    { SCIM_PINYIN_Ie,  SCIM_PINYIN_ZeroFinal },         // X
    { SCIM_PINYIN_Ing, SCIM_PINYIN_Uai       },         // Y
    { SCIM_PINYIN_Ei,  SCIM_PINYIN_ZeroFinal },         // Z
    { SCIM_PINYIN_ZeroFinal, SCIM_PINYIN_ZeroFinal },   // ;
};


static const PinyinInitial scim_shuang_pin_ms_initial_map [] =
{
    SCIM_PINYIN_ZeroInitial,    // A
    SCIM_PINYIN_Bo,             // B
    SCIM_PINYIN_Ci,             // C
    SCIM_PINYIN_De,             // D
    SCIM_PINYIN_ZeroInitial,    // E
    SCIM_PINYIN_Fo,             // F
    SCIM_PINYIN_Ge,             // G
    SCIM_PINYIN_He,             // H
    SCIM_PINYIN_Chi,            // I
    SCIM_PINYIN_Ji,             // J
    SCIM_PINYIN_Ke,             // K
    SCIM_PINYIN_Le,             // L
    SCIM_PINYIN_Mo,             // M
    SCIM_PINYIN_Ne,             // N
    SCIM_PINYIN_ZeroInitial,    // O
    SCIM_PINYIN_Po,             // P
    SCIM_PINYIN_Qi,             // Q
    SCIM_PINYIN_Ri,             // R
    SCIM_PINYIN_Si,             // S
    SCIM_PINYIN_Te,             // T
    SCIM_PINYIN_Shi,            // U
    SCIM_PINYIN_Zhi,            // V
    SCIM_PINYIN_Wo,             // W
    SCIM_PINYIN_Xi,             // X
    SCIM_PINYIN_Yi,             // Y
    SCIM_PINYIN_Zi,             // Z
    SCIM_PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal scim_shuang_pin_ms_final_map [][2] =
{
    { SCIM_PINYIN_A,   SCIM_PINYIN_ZeroFinal },         // A
    { SCIM_PINYIN_Ou,  SCIM_PINYIN_ZeroFinal },         // B
    { SCIM_PINYIN_Iao, SCIM_PINYIN_ZeroFinal },         // C
    { SCIM_PINYIN_Uang,SCIM_PINYIN_Iang      },         // D
    { SCIM_PINYIN_E,   SCIM_PINYIN_ZeroFinal },         // E
    { SCIM_PINYIN_En,  SCIM_PINYIN_ZeroFinal },         // F
    { SCIM_PINYIN_Ng,  SCIM_PINYIN_Eng       },         // G
    { SCIM_PINYIN_Ang, SCIM_PINYIN_ZeroFinal },         // H
    { SCIM_PINYIN_I,   SCIM_PINYIN_ZeroFinal },         // I
    { SCIM_PINYIN_An,  SCIM_PINYIN_ZeroFinal },         // J
    { SCIM_PINYIN_Ao,  SCIM_PINYIN_ZeroFinal },         // K
    { SCIM_PINYIN_Ai,  SCIM_PINYIN_ZeroFinal },         // L
    { SCIM_PINYIN_Ian, SCIM_PINYIN_ZeroFinal },         // M
    { SCIM_PINYIN_In,  SCIM_PINYIN_ZeroFinal },         // N
    { SCIM_PINYIN_Uo,  SCIM_PINYIN_O         },         // O
    { SCIM_PINYIN_Un,  SCIM_PINYIN_ZeroFinal },         // P
    { SCIM_PINYIN_Iu,  SCIM_PINYIN_ZeroFinal },         // Q
    { SCIM_PINYIN_Uan, SCIM_PINYIN_Er        },         // R
    { SCIM_PINYIN_Ong, SCIM_PINYIN_Iong      },         // S
    { SCIM_PINYIN_Ue,  SCIM_PINYIN_ZeroFinal },         // T
    { SCIM_PINYIN_U,   SCIM_PINYIN_ZeroFinal },         // U
    { SCIM_PINYIN_V,   SCIM_PINYIN_Ui        },         // V
    { SCIM_PINYIN_Ia,  SCIM_PINYIN_Ua        },         // W
    { SCIM_PINYIN_Ie,  SCIM_PINYIN_ZeroFinal },         // X
    { SCIM_PINYIN_Uai, SCIM_PINYIN_V         },         // Y
    { SCIM_PINYIN_Ei,  SCIM_PINYIN_ZeroFinal },         // Z
    { SCIM_PINYIN_Ing, SCIM_PINYIN_ZeroFinal },         // ;
};


static const PinyinInitial scim_shuang_pin_ziguang_initial_map [] =
{
    SCIM_PINYIN_Chi,            // A
    SCIM_PINYIN_Bo,             // B
    SCIM_PINYIN_Ci,             // C
    SCIM_PINYIN_De,             // D
    SCIM_PINYIN_ZeroInitial,    // E
    SCIM_PINYIN_Fo,             // F
    SCIM_PINYIN_Ge,             // G
    SCIM_PINYIN_He,             // H
    SCIM_PINYIN_Shi,            // I
    SCIM_PINYIN_Ji,             // J
    SCIM_PINYIN_Ke,             // K
    SCIM_PINYIN_Le,             // L
    SCIM_PINYIN_Mo,             // M
    SCIM_PINYIN_Ne,             // N
    SCIM_PINYIN_ZeroInitial,    // O
    SCIM_PINYIN_Po,             // P
    SCIM_PINYIN_Qi,             // Q
    SCIM_PINYIN_Ri,             // R
    SCIM_PINYIN_Si,             // S
    SCIM_PINYIN_Te,             // T
    SCIM_PINYIN_Zhi,            // U
    SCIM_PINYIN_ZeroInitial,    // V
    SCIM_PINYIN_Wo,             // W
    SCIM_PINYIN_Xi,             // X
    SCIM_PINYIN_Yi,             // Y
    SCIM_PINYIN_Zi,             // Z
    SCIM_PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal scim_shuang_pin_ziguang_final_map [][2] =
{
    { SCIM_PINYIN_A,   SCIM_PINYIN_ZeroFinal },         // A
    { SCIM_PINYIN_Iao, SCIM_PINYIN_ZeroFinal },         // B
    { SCIM_PINYIN_Ing, SCIM_PINYIN_ZeroFinal },         // C
    { SCIM_PINYIN_Ie,  SCIM_PINYIN_ZeroFinal },         // D
    { SCIM_PINYIN_E,   SCIM_PINYIN_ZeroFinal },         // E
    { SCIM_PINYIN_Ian, SCIM_PINYIN_ZeroFinal },         // F
    { SCIM_PINYIN_Uang,SCIM_PINYIN_Iang      },         // G
    { SCIM_PINYIN_Ong, SCIM_PINYIN_Iong      },         // H
    { SCIM_PINYIN_I,   SCIM_PINYIN_ZeroFinal },         // I
    { SCIM_PINYIN_Iu,  SCIM_PINYIN_Er        },         // J
    { SCIM_PINYIN_Ei,  SCIM_PINYIN_ZeroFinal },         // K
    { SCIM_PINYIN_Uan, SCIM_PINYIN_ZeroFinal },         // L
    { SCIM_PINYIN_Un,  SCIM_PINYIN_ZeroFinal },         // M
    { SCIM_PINYIN_Ui,  SCIM_PINYIN_Ue        },         // N
    { SCIM_PINYIN_Uo,  SCIM_PINYIN_O         },         // O
    { SCIM_PINYIN_Ai,  SCIM_PINYIN_ZeroFinal },         // P
    { SCIM_PINYIN_Ao,  SCIM_PINYIN_ZeroFinal },         // Q
    { SCIM_PINYIN_An,  SCIM_PINYIN_ZeroFinal },         // R
    { SCIM_PINYIN_Ang, SCIM_PINYIN_ZeroFinal },         // S
    { SCIM_PINYIN_Ng,  SCIM_PINYIN_Eng       },         // T
    { SCIM_PINYIN_U,   SCIM_PINYIN_ZeroFinal },         // U
    { SCIM_PINYIN_V,   SCIM_PINYIN_ZeroFinal },         // V
    { SCIM_PINYIN_En,  SCIM_PINYIN_ZeroFinal },         // W
    { SCIM_PINYIN_Ia,  SCIM_PINYIN_Ua        },         // X
    { SCIM_PINYIN_In,  SCIM_PINYIN_Uai       },         // Y
    { SCIM_PINYIN_Ou,  SCIM_PINYIN_ZeroFinal },         // Z
    { SCIM_PINYIN_ZeroFinal, SCIM_PINYIN_ZeroFinal },   // ;
};


static const PinyinInitial scim_shuang_pin_abc_initial_map [] =
{
    SCIM_PINYIN_Zhi,            // A
    SCIM_PINYIN_Bo,             // B
    SCIM_PINYIN_Ci,             // C
    SCIM_PINYIN_De,             // D
    SCIM_PINYIN_Chi,            // E
    SCIM_PINYIN_Fo,             // F
    SCIM_PINYIN_Ge,             // G
    SCIM_PINYIN_He,             // H
    SCIM_PINYIN_ZeroInitial,    // I
    SCIM_PINYIN_Ji,             // J
    SCIM_PINYIN_Ke,             // K
    SCIM_PINYIN_Le,             // L
    SCIM_PINYIN_Mo,             // M
    SCIM_PINYIN_Ne,             // N
    SCIM_PINYIN_ZeroInitial,    // O
    SCIM_PINYIN_Po,             // P
    SCIM_PINYIN_Qi,             // Q
    SCIM_PINYIN_Ri,             // R
    SCIM_PINYIN_Si,             // S
    SCIM_PINYIN_Te,             // T
    SCIM_PINYIN_ZeroInitial,    // U
    SCIM_PINYIN_Shi,            // V
    SCIM_PINYIN_Wo,             // W
    SCIM_PINYIN_Xi,             // X
    SCIM_PINYIN_Yi,             // Y
    SCIM_PINYIN_Zi,             // Z
    SCIM_PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal scim_shuang_pin_abc_final_map [][2] =
{
    { SCIM_PINYIN_A,   SCIM_PINYIN_ZeroFinal },         // A
    { SCIM_PINYIN_Ou,  SCIM_PINYIN_ZeroFinal },         // B
    { SCIM_PINYIN_In,  SCIM_PINYIN_Uai       },         // C
    { SCIM_PINYIN_Ia,  SCIM_PINYIN_Ua        },         // D
    { SCIM_PINYIN_E,   SCIM_PINYIN_ZeroFinal },         // E
    { SCIM_PINYIN_En,  SCIM_PINYIN_ZeroFinal },         // F
    { SCIM_PINYIN_Ng,  SCIM_PINYIN_Eng       },         // G
    { SCIM_PINYIN_Ang, SCIM_PINYIN_ZeroFinal },         // H
    { SCIM_PINYIN_I,   SCIM_PINYIN_ZeroFinal },         // I
    { SCIM_PINYIN_An,  SCIM_PINYIN_ZeroFinal },         // J
    { SCIM_PINYIN_Ao,  SCIM_PINYIN_ZeroFinal },         // K
    { SCIM_PINYIN_Ai,  SCIM_PINYIN_ZeroFinal },         // L
    { SCIM_PINYIN_Ui,  SCIM_PINYIN_Ue        },         // M
    { SCIM_PINYIN_Un,  SCIM_PINYIN_ZeroFinal },         // N
    { SCIM_PINYIN_Uo,  SCIM_PINYIN_O         },         // O
    { SCIM_PINYIN_Uan, SCIM_PINYIN_ZeroFinal },         // P
    { SCIM_PINYIN_Ei,  SCIM_PINYIN_ZeroFinal },         // Q
    { SCIM_PINYIN_Iu,  SCIM_PINYIN_Er        },         // R
    { SCIM_PINYIN_Ong, SCIM_PINYIN_Iong      },         // S
    { SCIM_PINYIN_Uang,SCIM_PINYIN_Iang      },         // T
    { SCIM_PINYIN_U,   SCIM_PINYIN_ZeroFinal },         // U
    { SCIM_PINYIN_V,   SCIM_PINYIN_ZeroFinal },         // V
    { SCIM_PINYIN_Ian, SCIM_PINYIN_ZeroFinal },         // W
    { SCIM_PINYIN_Ie,  SCIM_PINYIN_ZeroFinal },         // X
    { SCIM_PINYIN_Ing, SCIM_PINYIN_ZeroFinal },         // Y
    { SCIM_PINYIN_Iao, SCIM_PINYIN_ZeroFinal },         // Z
    { SCIM_PINYIN_ZeroFinal, SCIM_PINYIN_ZeroFinal },   // ;
};


static const PinyinInitial scim_shuang_pin_liushi_initial_map [] =
{
    SCIM_PINYIN_ZeroInitial,    // A
    SCIM_PINYIN_Bo,             // B
    SCIM_PINYIN_Ci,             // C
    SCIM_PINYIN_De,             // D
    SCIM_PINYIN_ZeroInitial,    // E
    SCIM_PINYIN_Fo,             // F
    SCIM_PINYIN_Ge,             // G
    SCIM_PINYIN_He,             // H
    SCIM_PINYIN_Chi,            // I
    SCIM_PINYIN_Ji,             // J
    SCIM_PINYIN_Ke,             // K
    SCIM_PINYIN_Le,             // L
    SCIM_PINYIN_Mo,             // M
    SCIM_PINYIN_Ne,             // N
    SCIM_PINYIN_ZeroInitial,    // O
    SCIM_PINYIN_Po,             // P
    SCIM_PINYIN_Qi,             // Q
    SCIM_PINYIN_Ri,             // R
    SCIM_PINYIN_Si,             // S
    SCIM_PINYIN_Te,             // T
    SCIM_PINYIN_Shi,            // U
    SCIM_PINYIN_Zhi,            // V
    SCIM_PINYIN_Wo,             // W
    SCIM_PINYIN_Xi,             // X
    SCIM_PINYIN_Yi,             // Y
    SCIM_PINYIN_Zi,             // Z
    SCIM_PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal scim_shuang_pin_liushi_final_map [][2] =
{
    { SCIM_PINYIN_A,   SCIM_PINYIN_ZeroFinal },         // A
    { SCIM_PINYIN_Ao,  SCIM_PINYIN_ZeroFinal },         // B
    { SCIM_PINYIN_Ang, SCIM_PINYIN_ZeroFinal },         // C
    { SCIM_PINYIN_Uan, SCIM_PINYIN_ZeroFinal },         // D
    { SCIM_PINYIN_E,   SCIM_PINYIN_ZeroFinal },         // E
    { SCIM_PINYIN_An,  SCIM_PINYIN_ZeroFinal },         // F
    { SCIM_PINYIN_Ong, SCIM_PINYIN_Iong      },         // G
    { SCIM_PINYIN_Ui,  SCIM_PINYIN_Ue        },         // H
    { SCIM_PINYIN_I,   SCIM_PINYIN_ZeroFinal },         // I
    { SCIM_PINYIN_Ia,  SCIM_PINYIN_Ua        },         // J
    { SCIM_PINYIN_Un,  SCIM_PINYIN_ZeroFinal },         // K
    { SCIM_PINYIN_Iu,  SCIM_PINYIN_ZeroFinal },         // L
    { SCIM_PINYIN_In,  SCIM_PINYIN_ZeroFinal },         // M
    { SCIM_PINYIN_Uang,SCIM_PINYIN_Iang      },         // N
    { SCIM_PINYIN_Uo,  SCIM_PINYIN_O         },         // O
    { SCIM_PINYIN_Ng,  SCIM_PINYIN_Eng       },         // P
    { SCIM_PINYIN_Ing, SCIM_PINYIN_ZeroFinal },         // Q
    { SCIM_PINYIN_Ou,  SCIM_PINYIN_Er        },         // R
    { SCIM_PINYIN_Ai,  SCIM_PINYIN_ZeroFinal },         // S
    { SCIM_PINYIN_Ian, SCIM_PINYIN_ZeroFinal },         // T
    { SCIM_PINYIN_U,   SCIM_PINYIN_ZeroFinal },         // U
    { SCIM_PINYIN_V,   SCIM_PINYIN_En        },         // V
    { SCIM_PINYIN_Ei,  SCIM_PINYIN_ZeroFinal },         // W
    { SCIM_PINYIN_Ie,  SCIM_PINYIN_ZeroFinal },         // X
    { SCIM_PINYIN_Uai, SCIM_PINYIN_ZeroFinal },         // Y
    { SCIM_PINYIN_Iao, SCIM_PINYIN_ZeroFinal },         // Z
    { SCIM_PINYIN_ZeroFinal, SCIM_PINYIN_ZeroFinal },   // ;
};


//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinKey

const char*
PinyinKey::get_initial_string () const
{
    return scim_pinyin_initials [m_initial].str;
}

const ucs4_t*
PinyinKey::get_initial_wide_string () const
{
    return scim_pinyin_initials [m_initial].wstr;
}

const char*
PinyinKey::get_final_string () const
{
    return scim_pinyin_finals [m_final].str;
}

const ucs4_t*
PinyinKey::get_final_wide_string () const
{
    return scim_pinyin_finals [m_final].wstr;
}

const char*
PinyinKey::get_tone_string () const
{
    return scim_pinyin_tones [m_tone].str;
}

const ucs4_t*
PinyinKey::get_tone_wide_string () const
{
    return scim_pinyin_tones [m_tone].wstr;
}

String
PinyinKey::get_key_string () const
{
    char key [16];
    snprintf (key, 15, "%s%s%s", get_initial_string(), get_final_string(), get_tone_string ());

    return String (key);
}

WideString
PinyinKey::get_key_wide_string () const
{
    return WideString (get_initial_wide_string ()) + WideString (get_final_wide_string()) + WideString (get_tone_wide_string ());
}

std::ostream&
PinyinKey::output_text (std::ostream &os) const
{
    return os << get_key_string ();
}

std::istream&
PinyinKey::input_text (const PinyinValidator &validator, std::istream &is)
{
    String key;
    is >> key;
    set (validator, key.c_str());
    return is;
}

std::ostream&
PinyinKey::output_binary (std::ostream &os) const
{
    unsigned char key [2];
    store_to_bytes (key);
    os.write ((const char*) key, sizeof (char) * 2);
    return os;
}

std::istream&
PinyinKey::input_binary (const PinyinValidator &validator, std::istream &is)
{
    unsigned char key [2];
    is.read ((char*) key, sizeof (char) * 2);
    extract_from_bytes (key [0], key [1]);
    if (!validator (*this)) {
        m_tone = SCIM_PINYIN_ZeroTone;
        if (!validator (*this)) {
            m_final = SCIM_PINYIN_ZeroFinal;
            if (!validator (*this))
                m_initial = SCIM_PINYIN_ZeroInitial;
        }
    }
    return is;
}

int
PinyinKey::set (const PinyinValidator &validator, const char *str, int len)
{
    if (!str || ! (*str))
        return 0;

    PinyinDefaultParser parser;

    return parser.parse_one_key (validator, *this, str, len);
}

//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinValidator
PinyinValidator::PinyinValidator (const PinyinTable *table)
{
    initialize (table);
}

void
PinyinValidator::initialize (const PinyinTable *table)
{
    memset (m_bitmap, 0, sizeof (m_bitmap));

    if (!table || !table->size()) return;

    for (int i=0; i<SCIM_PINYIN_InitialNumber; i++) {
        for (int j=0; j<SCIM_PINYIN_FinalNumber; j++) {
            for (int k=0; k<SCIM_PINYIN_ToneNumber; k++) {
                PinyinKey key(static_cast<PinyinInitial>(i),
                              static_cast<PinyinFinal>(j),
                              static_cast<PinyinTone>(k));
                if (!table->has_key (key)) {
                    int val = (k * SCIM_PINYIN_FinalNumber + j) * SCIM_PINYIN_InitialNumber + i;
                    m_bitmap [val >> 3] |= (1 << (val % 8));
                }
            }
        }
    }
}

bool
PinyinValidator::operator () (PinyinKey key) const
{
    if (key.get_initial () == SCIM_PINYIN_ZeroInitial && key.get_final () == SCIM_PINYIN_ZeroFinal)
        return false;

    int val = (key.get_tone () * SCIM_PINYIN_FinalNumber + key.get_final ()) *
                SCIM_PINYIN_InitialNumber + key.get_initial ();

    return  (m_bitmap [ val >> 3 ] & (1 << (val % 8))) == 0;
}

const PinyinValidator *
PinyinValidator::get_default_pinyin_validator ()
{
    return &scim_default_pinyin_validator;
}

//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinParser
PinyinParser::~PinyinParser ()
{
}

void
PinyinParser::normalize (PinyinKey &key) const
{
    static struct ReplaceRulePair {
        PinyinInitial initial;
        PinyinFinal   final;
        PinyinInitial new_initial;
        PinyinFinal   new_final;
    } rules [] = 
    {
#if 0
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_I,    SCIM_PINYIN_Yi, SCIM_PINYIN_I},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Ia,   SCIM_PINYIN_Yi, SCIM_PINYIN_A},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Ian,  SCIM_PINYIN_Yi, SCIM_PINYIN_An},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Iang, SCIM_PINYIN_Yi, SCIM_PINYIN_Ang},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Iao,  SCIM_PINYIN_Yi, SCIM_PINYIN_Ao},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Ie,   SCIM_PINYIN_Yi, SCIM_PINYIN_E},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_In,   SCIM_PINYIN_Yi, SCIM_PINYIN_In},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Ing,  SCIM_PINYIN_Yi, SCIM_PINYIN_Ing},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Iong, SCIM_PINYIN_Yi, SCIM_PINYIN_Ong},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Iou,  SCIM_PINYIN_Yi, SCIM_PINYIN_Ou},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Iu,   SCIM_PINYIN_Yi, SCIM_PINYIN_U},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_U,    SCIM_PINYIN_Wo, SCIM_PINYIN_U},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Ua,   SCIM_PINYIN_Wo, SCIM_PINYIN_A},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Uai,  SCIM_PINYIN_Wo, SCIM_PINYIN_Ai},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Uan,  SCIM_PINYIN_Wo, SCIM_PINYIN_An},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Uang, SCIM_PINYIN_Wo, SCIM_PINYIN_Ang},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Uei,  SCIM_PINYIN_Wo, SCIM_PINYIN_Ei},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Uen,  SCIM_PINYIN_Wo, SCIM_PINYIN_En},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Ueng, SCIM_PINYIN_Wo, SCIM_PINYIN_Eng},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Ui,   SCIM_PINYIN_Wo, SCIM_PINYIN_Ei},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Un,   SCIM_PINYIN_Wo, SCIM_PINYIN_En},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Uo,   SCIM_PINYIN_Wo, SCIM_PINYIN_O},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Ue,   SCIM_PINYIN_Yi, SCIM_PINYIN_Ue},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_V,    SCIM_PINYIN_Yi, SCIM_PINYIN_U},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Van,  SCIM_PINYIN_Yi, SCIM_PINYIN_Uan},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Ve,   SCIM_PINYIN_Yi, SCIM_PINYIN_Ue},
        {SCIM_PINYIN_ZeroInitial, SCIM_PINYIN_Vn,   SCIM_PINYIN_Yi, SCIM_PINYIN_Un},
#endif
        {SCIM_PINYIN_Ne,          SCIM_PINYIN_Ve,   SCIM_PINYIN_Ne, SCIM_PINYIN_Ue},
        {SCIM_PINYIN_Le,          SCIM_PINYIN_Ve,   SCIM_PINYIN_Le, SCIM_PINYIN_Ue},
        {SCIM_PINYIN_Ji,          SCIM_PINYIN_V,    SCIM_PINYIN_Ji, SCIM_PINYIN_U},
        {SCIM_PINYIN_Ji,          SCIM_PINYIN_Van,  SCIM_PINYIN_Ji, SCIM_PINYIN_Uan},
        {SCIM_PINYIN_Ji,          SCIM_PINYIN_Ve,   SCIM_PINYIN_Ji, SCIM_PINYIN_Ue},
        {SCIM_PINYIN_Ji,          SCIM_PINYIN_Vn,   SCIM_PINYIN_Ji, SCIM_PINYIN_Un},
        {SCIM_PINYIN_Qi,          SCIM_PINYIN_V,    SCIM_PINYIN_Qi, SCIM_PINYIN_U},
        {SCIM_PINYIN_Qi,          SCIM_PINYIN_Van,  SCIM_PINYIN_Qi, SCIM_PINYIN_Uan},
        {SCIM_PINYIN_Qi,          SCIM_PINYIN_Ve,   SCIM_PINYIN_Qi, SCIM_PINYIN_Ue},
        {SCIM_PINYIN_Qi,          SCIM_PINYIN_Vn,   SCIM_PINYIN_Qi, SCIM_PINYIN_Un},
        {SCIM_PINYIN_Xi,          SCIM_PINYIN_V,    SCIM_PINYIN_Xi, SCIM_PINYIN_U},
        {SCIM_PINYIN_Xi,          SCIM_PINYIN_Van,  SCIM_PINYIN_Xi, SCIM_PINYIN_Uan},
        {SCIM_PINYIN_Xi,          SCIM_PINYIN_Ve,   SCIM_PINYIN_Xi, SCIM_PINYIN_Ue},
        {SCIM_PINYIN_Xi,          SCIM_PINYIN_Vn,   SCIM_PINYIN_Xi, SCIM_PINYIN_Un}
    };

    for (size_t i=0; i < sizeof(rules)/sizeof(ReplaceRulePair); ++i) {
        if (rules[i].initial == key.get_initial () && rules[i].final == key.get_final ()) {
            key.set_initial (rules[i].new_initial);
            key.set_final (rules[i].new_final);
            break;
        }
    }

    if (key.get_initial () != SCIM_PINYIN_ZeroInitial) {
        switch (key.get_final ()) {
            case SCIM_PINYIN_Iou:
                key.set_final (SCIM_PINYIN_Iu);
                break;
            case SCIM_PINYIN_Uei:
                key.set_final (SCIM_PINYIN_Ui);
                break;
            case SCIM_PINYIN_Uen:
                key.set_final (SCIM_PINYIN_Un);
                break;
        }
    }
}

PinyinDefaultParser::~PinyinDefaultParser ()
{
}

int
PinyinDefaultParser::parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len) const
{
    int initial_len = 0;
    int final_len   = 0;
    int tone_len    = 0;

    const char *ptr;

    PinyinInitial initial;
    PinyinFinal   final;
    PinyinTone    tone;

    key.clear ();

    if (!str || !len) return 0;

    if (len < 0) len = strlen (str);

    while (len > 0) {
        ptr = str;

        initial = SCIM_PINYIN_ZeroInitial;
        final   = SCIM_PINYIN_ZeroFinal;
        tone    = SCIM_PINYIN_ZeroTone;

        final_len = parse_final (final, ptr, len);
        ptr += final_len;
        len -= final_len;
 
        // An initial is present
        if (final == SCIM_PINYIN_ZeroFinal) {
            initial_len = parse_initial (initial, ptr, len);
            ptr += initial_len;
            len -= initial_len;
            if (len){
                final_len = parse_final (final, ptr, len);
                ptr += final_len;
                len -= final_len;
            }
        }

        if (len)
            tone_len = parse_tone (tone, ptr, len);
 
        key.set (initial, final, tone);
 
        normalize (key);

        // A valid key was found, return.
        if (validator (key)) break;

        // The key is invalid, reduce the len and find again.
        len = initial_len + final_len + tone_len - 1;

        initial_len = final_len = tone_len = 0;

        key.clear ();
    }

    return initial_len + final_len + tone_len;
}

int
PinyinDefaultParser::parse (const PinyinValidator &validator, PinyinParsedKeyVector &keys, const char *str, int len) const
{
    keys.clear ();

    if (!str) return 0;

    if (len < 0) len = strlen (str);

    ParsedKeyCache cache;
    int real_start;
    int num_keys;

    len = parse_recursive (validator, real_start, num_keys, cache, str, len, 0, 0);

    keys = cache [real_start];

    return len;
}

int
PinyinDefaultParser::parse_recursive (const PinyinValidator &validator,
                                      int                   &real_start,
                                      int                   &num_keys,
                                      ParsedKeyCache        &cache,
                                      const char            *str,
                                      int                    len,
                                      int                    level,
                                      int                    start) const
{
    if (*str == 0 || len == 0) return 0;

    int used_len = 0;

    real_start = 0;
    num_keys = 0;

    if (*str == '\'') {
        ++used_len;
        ++str;
        ++start;
        --len;
    }

    if (!isalpha (*str) || !len)
        return 0;

    ParsedKeyCache::iterator it = cache.find (start);

    real_start = start;

    // The best keys start from this position have been found, just return the result.
    if (it != cache.end ()) {
        num_keys = it->second.size ();

        if (!num_keys) return 0;

        return it->second.back ().get_pos () + it->second.back ().get_length () - start;
    }

    PinyinKey first_key;
    PinyinKey best_first_key;

    int first_len = 0;
    int best_first_len = 0;

    int remained_len = 0;
    int best_remained_len = 0;

    int remained_keys = 0;
    int best_remained_keys = 0;

    int remained_start = 0;
    int best_remained_start = 0;

    first_len = parse_one_key (validator, first_key, str, len);

    if (!first_len) {
        cache [start] = PinyinParsedKeyVector ();
        return 0;
    }

    best_first_key = first_key;
    best_first_len = first_len;

    if (len > first_len) {
        char ch1 = str [first_len -1];
        char ch2 = str [first_len];

        best_remained_len = parse_recursive (validator,
                                             best_remained_start,
                                             best_remained_keys,
                                             cache,
                                             str + first_len,
                                             len - first_len,
                                             level + 1,
                                             start + first_len);

        // For those keys which the last char is 'g' or 'n' or 'r', try put the end char into the next key.
        if (first_len > 1 &&
            (ch1 == 'g' || ch1 == 'n' || ch1 == 'r' || ch1 == 'h') &&
            (ch2 == 'a' || ch2 == 'e' || ch2 == 'i' || ch2 == 'o' || ch2 == 'u' || ch2 == 'v')) {

            first_len = parse_one_key (validator, first_key, str, first_len - 1);

            if (first_len) {
                remained_len = parse_recursive (validator,
                                                remained_start,
                                                remained_keys,
                                                cache, 
                                                str + first_len,
                                                len - first_len,
                                                level + 1,
                                                start + first_len);

                // A better seq was found.
                if (remained_len != 0 && remained_len >= best_remained_len && (remained_len + first_len) > best_first_len &&
                    (remained_keys <= best_remained_keys || best_remained_keys == 0)) {
                    best_first_len = first_len;
                    best_first_key = first_key;
                    best_remained_len = remained_len;
                    best_remained_keys = remained_keys;
                    best_remained_start = remained_start;
                }
            }
        }
    }

    cache [start].push_back (PinyinParsedKey (best_first_key, start, best_first_len));

    if (best_remained_len) {
        for (PinyinParsedKeyVector::iterator it = cache [best_remained_start].begin (); it != cache [best_remained_start].end (); ++it)
            cache [start].push_back (*it);
    }

    num_keys = best_remained_keys + 1;

    return used_len + best_first_len + best_remained_len;
}

int
PinyinDefaultParser::parse_initial (PinyinInitial &initial, const char *str, int len) const
{
    int lastlen = 0;

    initial = SCIM_PINYIN_ZeroInitial;

    if (str && *str >= 'a' && *str <= 'z') {
        int start = scim_pinyin_initials_index [*str - 'a'].start;
        int end = scim_pinyin_initials_index [*str - 'a'].num + start;

        if (start > 0) {
            if (len < 0) len = strlen (str);

            for (int i = start; i < end; ++i) {
                if (len >= scim_pinyin_initials [i].len && scim_pinyin_initials [i].len >= lastlen) {
                    int j;
                    for (j = 1; j < scim_pinyin_initials [i].len; ++j) {
                        if (str [j] != scim_pinyin_initials [i].str [j])
                            break;
                    }
                    if (j == scim_pinyin_initials [i].len) {
                        initial = static_cast<PinyinInitial>(i);
                        lastlen = scim_pinyin_initials [i].len;
                    }
                }
            }
        }
    }

    return lastlen;
}

int
PinyinDefaultParser::parse_final (PinyinFinal &final, const char *str, int len) const
{
    int lastlen = 0;

    final = SCIM_PINYIN_ZeroFinal;

    if (str && *str >= 'a' && *str <= 'z') {
        int start = scim_pinyin_finals_index [*str - 'a'].start;
        int end = scim_pinyin_finals_index [*str - 'a'].num + start;

        if (start > 0) {
            if (len < 0) len = strlen (str);

            for (int i = start; i < end; ++i) {
                if (len >= scim_pinyin_finals [i].len && scim_pinyin_finals [i].len >= lastlen) {
                    int j;
                    for (j = 1; j < scim_pinyin_finals [i].len; ++j) {
                        if (str [j] != scim_pinyin_finals [i].str [j])
                            break;
                    }
                    if (j == scim_pinyin_finals [i].len) {
                        final = static_cast<PinyinFinal>(i);
                        lastlen = scim_pinyin_finals [i].len;
                    }
                }
            }
        }
    }

    return lastlen;
}

int
PinyinDefaultParser::parse_tone (PinyinTone &tone, const char *str, int len) const
{
    tone = SCIM_PINYIN_ZeroTone;

    if (str && (len >= 1 || len < 0)) {
        int kt = (*str) - '0';
        if (kt >= SCIM_PINYIN_First && kt <= SCIM_PINYIN_LastTone) {
            tone = static_cast<PinyinTone>(kt);
            return 1;
        }
    }
    return 0;
}


PinyinShuangPinParser::PinyinShuangPinParser (PinyinShuangPinScheme scheme)
{
    set_scheme (scheme);
}

PinyinShuangPinParser::~PinyinShuangPinParser ()
{
}

int
PinyinShuangPinParser::parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len) const
{
    key.clear ();

    if (!str || !len || ! (*str)) return 0;

    if (len < 0) len = strlen (str);

    PinyinInitial initial    = SCIM_PINYIN_ZeroInitial;
    PinyinFinal   final      = SCIM_PINYIN_ZeroFinal;
    PinyinFinal   final_cands [4] = { SCIM_PINYIN_ZeroFinal, SCIM_PINYIN_ZeroFinal, SCIM_PINYIN_ZeroFinal, SCIM_PINYIN_ZeroFinal };

    PinyinTone    tone = SCIM_PINYIN_ZeroTone;

    int idx [2] = {-1, -1};
    int used_len = 0;

    size_t i;
    bool matched = false;

    for (i = 0; i < 2 && i < len; ++i) {
        if (str [i] >= 'a' && str [i] <= 'z') idx [i] = str [i] - 'a';
        else if (str [i] == ';') idx [i] = 26;
    }

    // parse initial or final
    if (idx [0] >= 0) {
        initial = m_initial_map [idx[0]];
        final_cands [0] = m_final_map [idx[0]][0];
        final_cands [1] = m_final_map [idx[0]][1];
    }

    if (initial == SCIM_PINYIN_ZeroInitial && final_cands [0] == SCIM_PINYIN_ZeroFinal)
        return 0;

    // parse final, if str [0] == 'o' (idx [0] == 14) then just skip to parse final.
    if (idx [1] >= 0 && (initial != SCIM_PINYIN_ZeroInitial || idx[0] == 14)) {
        final_cands [2] = m_final_map [idx [1]][0];
        final_cands [3] = m_final_map [idx [1]][1];

        for (i = 2; i < 4; ++i) {
            if (final_cands [i] != SCIM_PINYIN_ZeroFinal) {
                key.set (initial, final_cands [i]);
                normalize (key);

                if (validator (key)) {
                    final = final_cands [i];
                    matched = true;
                    used_len = 2;
                    str += 2;
                    len -= 2;
                    break;
                }
            }
        }
    }

    if (!matched) {
        initial = SCIM_PINYIN_ZeroInitial;
        for (i = 0; i < 2; ++i) {
            key.set (initial, final_cands [i]);
            normalize (key);

            if (validator (key)) {
                final = final_cands [i];
                matched = true;
                used_len = 1;
                ++str;
                --len;
                break;
            }
        }
    }

    if (!matched) return 0;

    // parse tone
    if (len) {
        int kt = (*str) - '0';
        if (kt >= SCIM_PINYIN_First && kt <= SCIM_PINYIN_LastTone) {
            tone = static_cast<PinyinTone>(kt);

            key.set (initial, final, tone);

            if (validator (key))
                return used_len + 1;
        }
    }

    return used_len;
}

int
PinyinShuangPinParser::parse (const PinyinValidator &validator, PinyinParsedKeyVector &keys, const char *str, int len) const
{
    keys.clear ();

    if (!str || !len || ! (*str)) return 0;

    if (len < 0) len = strlen (str);

    int used_len = 0;

    PinyinParsedKey key;

    while (used_len < len) {
        if (*str == '\'') {
            ++str;
            ++used_len;
            continue;
        }

        int one_len = parse_one_key (validator, key, str, len);

        if (one_len) {
            key.set_pos (used_len);
            key.set_length (one_len);
            keys.push_back (key);
        } else {
            break;
        }

        str += one_len;
        used_len += one_len;
    }

    return used_len;
}

void
PinyinShuangPinParser::set_scheme (PinyinShuangPinScheme scheme)
{
    const PinyinInitial (*initial_map)[27] = 0;
    const PinyinFinal   (*final_map)[27][2] = 0;

    switch (scheme) {
        case SCIM_SHUANG_PIN_STONE:
            initial_map = &scim_shuang_pin_stone_initial_map;
            final_map   = &scim_shuang_pin_stone_final_map;
            break;
        case SCIM_SHUANG_PIN_ZRM:
            initial_map = &scim_shuang_pin_zrm_initial_map;
            final_map   = &scim_shuang_pin_zrm_final_map;
            break;
        case SCIM_SHUANG_PIN_MS:
            initial_map = &scim_shuang_pin_ms_initial_map;
            final_map   = &scim_shuang_pin_ms_final_map;
            break;
        case SCIM_SHUANG_PIN_ZIGUANG:
            initial_map = &scim_shuang_pin_ziguang_initial_map;
            final_map   = &scim_shuang_pin_ziguang_final_map;
            break;
        case SCIM_SHUANG_PIN_ABC:
            initial_map = &scim_shuang_pin_abc_initial_map;
            final_map   = &scim_shuang_pin_abc_final_map;
            break;
        case SCIM_SHUANG_PIN_LIUSHI:
            initial_map = &scim_shuang_pin_liushi_initial_map;
            final_map   = &scim_shuang_pin_liushi_final_map;
            break;
        default:
            for (size_t i = 0; i < 27; ++i) {
                m_initial_map [i] = SCIM_PINYIN_ZeroInitial;
                m_final_map [i][0] = m_final_map [i][1] = SCIM_PINYIN_ZeroFinal;
            }
            return;
    }

    for (size_t i = 0; i < 27; ++i) {
        m_initial_map [i] = (*initial_map) [i];
        m_final_map [i][0] = (*final_map) [i][0];
        m_final_map [i][1] = (*final_map) [i][1];
    }
}

//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinKey comparision classes
static int
__scim_pinyin_compare_initial (const PinyinCustomSettings &custom,
                               PinyinInitial lhs,
                               PinyinInitial rhs)
{
    // Ambiguity LeRi, NeLe, FoHe will break binary search
    // we treat them as special cases
    if (custom.use_ambiguities [SCIM_PINYIN_AmbLeRi]) {
        if (lhs == SCIM_PINYIN_Ri) lhs = SCIM_PINYIN_Le;
        if (rhs == SCIM_PINYIN_Ri) rhs = SCIM_PINYIN_Le;
    }

    if (custom.use_ambiguities [SCIM_PINYIN_AmbNeLe]) {
        if (lhs == SCIM_PINYIN_Ne) lhs = SCIM_PINYIN_Le;
        if (rhs == SCIM_PINYIN_Ne) rhs = SCIM_PINYIN_Le;
    }

    if (custom.use_ambiguities [SCIM_PINYIN_AmbFoHe]) {
        if (lhs == SCIM_PINYIN_He) lhs = SCIM_PINYIN_Fo;
        if (rhs == SCIM_PINYIN_He) rhs = SCIM_PINYIN_Fo;
    }

    if ((lhs == rhs) ||
        (custom.use_ambiguities [SCIM_PINYIN_AmbZhiZi] &&
         ((lhs == SCIM_PINYIN_Zhi && rhs == SCIM_PINYIN_Zi) ||
          (lhs == SCIM_PINYIN_Zi && rhs == SCIM_PINYIN_Zhi))) ||
              
        (custom.use_ambiguities [SCIM_PINYIN_AmbChiCi] &&
         ((lhs == SCIM_PINYIN_Chi && rhs == SCIM_PINYIN_Ci) ||
          (lhs == SCIM_PINYIN_Ci && rhs == SCIM_PINYIN_Chi))) ||
              
        (custom.use_ambiguities [SCIM_PINYIN_AmbShiSi] &&
         ((lhs == SCIM_PINYIN_Shi && rhs == SCIM_PINYIN_Si) ||
          (lhs == SCIM_PINYIN_Si && rhs == SCIM_PINYIN_Shi)))) 
        return 0;
    else if (lhs < rhs) return -1;
    return 1;
}

static int
__scim_pinyin_compare_final (const PinyinCustomSettings &custom,
                             PinyinFinal lhs,
                             PinyinFinal rhs)
{
    if(((lhs == rhs) ||
        (custom.use_ambiguities [SCIM_PINYIN_AmbAnAng] &&
         ((lhs == SCIM_PINYIN_An && rhs == SCIM_PINYIN_Ang) ||
          (lhs == SCIM_PINYIN_Ang && rhs == SCIM_PINYIN_An))) ||
              
        (custom.use_ambiguities [SCIM_PINYIN_AmbEnEng] &&
         ((lhs == SCIM_PINYIN_En && rhs == SCIM_PINYIN_Eng) ||
          (lhs == SCIM_PINYIN_Eng && rhs == SCIM_PINYIN_En))) ||
              
         (custom.use_ambiguities [SCIM_PINYIN_AmbInIng] &&
         ((lhs == SCIM_PINYIN_In && rhs == SCIM_PINYIN_Ing) ||
          (lhs == SCIM_PINYIN_Ing && rhs == SCIM_PINYIN_In)))))
        return 0;
    else if (custom.use_incomplete && (lhs == SCIM_PINYIN_ZeroFinal || rhs == SCIM_PINYIN_ZeroFinal))
        return 0;
    else if (lhs < rhs) return -1;
    return 1;
}

static int
__scim_pinyin_compare_tone (const PinyinCustomSettings &custom,
                            PinyinTone lhs,
                            PinyinTone rhs)
{
    if(lhs == rhs || lhs == SCIM_PINYIN_ZeroTone || rhs == SCIM_PINYIN_ZeroTone || !custom.use_tone)
        return 0;
    else if (lhs < rhs) return -1;
    return 1;
}

bool
PinyinKeyLessThan::operator () (PinyinKey lhs, PinyinKey rhs) const
{
    switch (__scim_pinyin_compare_initial (m_custom,
                static_cast<PinyinInitial>(lhs.m_initial),
                static_cast<PinyinInitial>(rhs.m_initial))) {
        case 0:
            switch (__scim_pinyin_compare_final (m_custom,
                      static_cast<PinyinFinal>(lhs.m_final),
                      static_cast<PinyinFinal>(rhs.m_final))) {
                case 0:
                    switch (__scim_pinyin_compare_tone (m_custom,
                             static_cast<PinyinTone>(lhs.m_tone),
                             static_cast<PinyinTone>(rhs.m_tone))) {
                        case -1:
                            return true;
                        default:
                            return false;
                    }
                case -1:
                    return true;
                default:
                    return false;
            }
        case -1:
            return true;
        default:
            return false;
    }
    return false;
}

bool
PinyinKeyEqualTo::operator () (PinyinKey lhs, PinyinKey rhs) const
{
    if (!__scim_pinyin_compare_initial (m_custom,
            static_cast<PinyinInitial>(lhs.m_initial),
            static_cast<PinyinInitial>(rhs.m_initial)) &&
        !__scim_pinyin_compare_final (m_custom,
            static_cast<PinyinFinal>(lhs.m_final),
            static_cast<PinyinFinal>(rhs.m_final)) &&
        !__scim_pinyin_compare_tone (m_custom,
            static_cast<PinyinTone>(lhs.m_tone),
            static_cast<PinyinTone>(rhs.m_tone)))
        return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinEntry
std::ostream&
PinyinEntry::output_text (std::ostream &os) const
{
    m_key.output_text (os) << "\t" << size() << "\t";

    for (std::vector<CharFrequencyPair>::const_iterator i = m_chars.begin(); i != m_chars.end(); i++) {
        utf8_write_wchar (os, i->first);
        os << i->second << ' ';
    }

    os << '\n';

    return os;
}

std::ostream&
PinyinEntry::output_binary (std::ostream &os) const
{
    unsigned char bytes [8];

    m_key.output_binary (os);

    scim_uint32tobytes (bytes, (uint32) size());

    os.write ((char*)bytes, sizeof (unsigned char) * 4);

    for (std::vector<CharFrequencyPair>::const_iterator i = m_chars.begin(); i != m_chars.end(); i++) {
        utf8_write_wchar (os, i->first);
        scim_uint32tobytes (bytes, i->second);
        os.write ((char*)bytes, sizeof (unsigned char) * 4);
    }

    return os;
}

std::istream&
PinyinEntry::input_text (const PinyinValidator &validator, std::istream &is)
{
    m_chars.clear();
    String value;
    uint32 n, len, freq;
    ucs4_t wc;

    m_key.input_text (validator, is);
    is >> n;
    m_chars.reserve (n+1);

    for (uint32 i=0; i<n; i++) {
        is >> value;
        if ((len = utf8_mbtowc (&wc, (const unsigned char*)(value.c_str()), value.length())) > 0) {
            if (value.length () > len)
                freq = atoi (value.c_str() + len);
            else
                freq = 0;
            m_chars.push_back (CharFrequencyPair (wc,freq));
        }
    }
    sort ();

    std::vector <CharFrequencyPair> (m_chars).swap (m_chars);

    return is;
}

std::istream&
PinyinEntry::input_binary (const PinyinValidator &validator, std::istream &is)
{
    m_chars.clear();
    uint32 n, freq;
    ucs4_t wc;

    unsigned char bytes [8];

    m_key.input_binary (validator, is);

    is.read ((char*)bytes, sizeof (unsigned char) * 4);
    n = scim_bytestouint32 (bytes);
    m_chars.reserve (n+1);

    for (uint32 i=0; i<n; i++) {
        if ((wc = utf8_read_wchar (is)) > 0) {
            is.read ((char*)bytes, sizeof (unsigned char) * 4);
            freq = scim_bytestouint32 (bytes);
            m_chars.push_back (CharFrequencyPair (wc, freq));
        }
    }
    sort ();

    std::vector <CharFrequencyPair> (m_chars).swap (m_chars);

    return is;
}

//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinTable
PinyinTable::PinyinTable (const PinyinCustomSettings &custom,
                          const PinyinValidator *validator,
                          std::istream &is)
    : m_revmap_ok (false),
      m_pinyin_key_less (custom),
      m_pinyin_key_equal (custom),
      m_validator (validator),
      m_custom (custom)
{
    if (!m_validator) m_validator = PinyinValidator::get_default_pinyin_validator ();

    input (is);
}

PinyinTable::PinyinTable (const PinyinCustomSettings &custom,
                          const PinyinValidator *validator,
                          const char *tablefile)
    : m_revmap_ok (false),
      m_pinyin_key_less (custom),
      m_pinyin_key_equal (custom),
      m_validator (validator),
      m_custom (custom)
{
    if (!m_validator) m_validator = PinyinValidator::get_default_pinyin_validator ();

    if (tablefile) load_table (tablefile);
}

bool
PinyinTable::output (std::ostream &os, bool binary) const
{
    unsigned char bytes [8];

    if (!binary) {
        os << scim_pinyin_table_text_header << "\n";
        os << scim_pinyin_table_version << "\n";
        os << m_table.size () << "\n";

        for (PinyinEntryVector::const_iterator i = m_table.begin(); i!=m_table.end(); i++)
            i->output_text (os);

    } else {
        os << scim_pinyin_table_binary_header << "\n";
        os << scim_pinyin_table_version << "\n";

        scim_uint32tobytes (bytes, (uint32) m_table.size ());
        os.write ((char*)bytes, sizeof (unsigned char) * 4);

        for (PinyinEntryVector::const_iterator i = m_table.begin(); i!=m_table.end(); i++)
            i->output_binary (os);
    }
    return true;
}

bool
PinyinTable::input (std::istream &is)
{
    char header [40];
    bool binary;

    if (!is) return false;
    
    is.getline (header, 40);

    if (strncmp (header,
        scim_pinyin_table_text_header,
        strlen (scim_pinyin_table_text_header)) == 0) {
        binary = false;
    } else if (strncmp (header,
        scim_pinyin_table_binary_header,
        strlen (scim_pinyin_table_binary_header)) == 0) {
        binary = true;
    } else {
        return false;
    }

    is.getline (header, 40);
    if (strncmp (header, scim_pinyin_table_version, strlen (scim_pinyin_table_version)) != 0)
        return false;

    uint32 i;
    uint32 n;
    PinyinEntryVector::iterator ev;

    if (!binary) {
        is >> n;

        // load pinyin table
        for (i=0; i<n; i++) {
            PinyinEntry entry (*m_validator, is, false);

            if (!m_custom.use_tone) {
                entry.set_key (PinyinKey (entry.get_key ().get_initial (),
                                              entry.get_key ().get_final (),
                                              SCIM_PINYIN_ZeroTone));
            }

            if (entry.get_key().get_final() == SCIM_PINYIN_ZeroFinal) {
                std::cerr << "Invalid entry: " << entry << "\n";
            } else {
                if ((ev = find_exact_entry (entry)) == m_table.end())
                    m_table.push_back (entry);
                else {
                    for (uint32 i=0; i<entry.size(); i++) {
                        ev->insert (entry.get_char_with_frequency_by_index (i));
                    }
                }
            }
        }
    } else {
        unsigned char bytes [8];
        is.read ((char*) bytes, sizeof (unsigned char) * 4);
        n = scim_bytestouint32 (bytes);

        // load pinyin table
        for (i=0; i<n; i++) {
            PinyinEntry entry (*m_validator, is, true);

            if (!m_custom.use_tone) {
                entry.set_key (PinyinKey (entry.get_key ().get_initial (),
                                              entry.get_key ().get_final (),
                                              SCIM_PINYIN_ZeroTone));
            }

            if (entry.get_key().get_final() == SCIM_PINYIN_ZeroFinal) {
                std::cerr << "Invalid entry: " << entry << "\n";
            } else {
                if ((ev = find_exact_entry (entry)) == m_table.end())
                    m_table.push_back (entry);
                else {
                    for (uint32 i=0; i<entry.size(); i++) {
                        ev->insert (entry.get_char_with_frequency_by_index (i));
                    }
                }
            }
        }
    }
    sort ();

    return true;
}

bool
PinyinTable::load_table (const char *tablefile)
{
    std::ifstream ifs(tablefile);
    if (!ifs) return false;
    if (input (ifs) && m_table.size () != 0) return true;
    return false;
}

bool
PinyinTable::save_table (const char *tablefile, bool binary) const
{
    std::ofstream ofs(tablefile);
    if (!ofs) return false;
    if (output (ofs, binary)) return true;
    return false;
}

void
PinyinTable::update_custom_settings (const PinyinCustomSettings &custom,
                                     const PinyinValidator *validator)
{
    m_pinyin_key_less  = PinyinKeyLessThan (custom);
    m_pinyin_key_equal = PinyinKeyEqualTo (custom);
    m_validator = validator;

    if (!m_validator)
        m_validator = PinyinValidator::get_default_pinyin_validator ();

    m_custom = custom;
    sort ();
}

int
PinyinTable::get_all_chars (std::vector<ucs4_t> &vec) const
{
    std::vector<CharFrequencyPair> all;

    vec.clear ();

    get_all_chars_with_frequencies (all);

    for (std::vector<CharFrequencyPair>::const_iterator i = all.begin ();
            i != all.end (); ++i)
        vec.push_back (i->first);

    return vec.size ();
}

int
PinyinTable::get_all_chars_with_frequencies (std::vector<CharFrequencyPair> &vec) const
{
    vec.clear ();

    for (PinyinEntryVector::const_iterator i = m_table.begin (); i!= m_table.end (); i++)
        i->get_all_chars_with_frequencies (vec);

    if (!vec.size ()) return 0;

    std::sort (vec.begin (), vec.end (), CharFrequencyPairGreaterThanByCharAndFrequency ());
    vec.erase (std::unique (vec.begin (), vec.end (), CharFrequencyPairEqualToByChar ()), vec.end ());
    std::sort (vec.begin (), vec.end (), CharFrequencyPairGreaterThanByFrequency ());

    return vec.size ();
}

int
PinyinTable::find_chars (std::vector <ucs4_t> &vec, PinyinKey key) const
{
    std::vector<CharFrequencyPair> all;

    vec.clear ();

    find_chars_with_frequencies (all, key);

    for (std::vector<CharFrequencyPair>::const_iterator i = all.begin ();
            i != all.end (); ++i)
        vec.push_back (i->first);

    return vec.size ();
}

int
PinyinTable::find_chars_with_frequencies (std::vector <CharFrequencyPair> &vec, PinyinKey key) const
{
    vec.clear ();

    std::pair<PinyinEntryVector::const_iterator, PinyinEntryVector::const_iterator> range =
        std::equal_range(m_table.begin(), m_table.end(), key, m_pinyin_key_less);

    for (PinyinEntryVector::const_iterator i = range.first; i!= range.second; i++) {
        i->get_all_chars_with_frequencies (vec);
    }

    if (!vec.size ()) return 0;

    std::sort (vec.begin (), vec.end (), CharFrequencyPairGreaterThanByCharAndFrequency ());
    vec.erase (std::unique (vec.begin (), vec.end (), CharFrequencyPairEqualToByChar ()), vec.end ());
    std::sort (vec.begin (), vec.end (), CharFrequencyPairGreaterThanByFrequency ());

    return vec.size ();
}

void
PinyinTable::erase (ucs4_t hz, const char *key)
{
    erase (hz, PinyinKey (*m_validator, key));
}

void
PinyinTable::erase (ucs4_t hz, PinyinKey key)
{
    if (key.empty ()) {
        for (PinyinEntryVector::iterator i = m_table.begin(); i != m_table.end(); i++)
            i->erase (hz);
    } else {
        std::pair<PinyinEntryVector::iterator, PinyinEntryVector::iterator> range =
            std::equal_range(m_table.begin(), m_table.end(), key, m_pinyin_key_less);
        for (PinyinEntryVector::iterator i = range.first; i!= range.second; i++)
            i->erase (hz);
    }
    erase_from_reverse_map (hz, key);
}

uint32
PinyinTable::get_char_frequency (ucs4_t ch, PinyinKey key)
{
    PinyinKeyVector keyvec;
    uint32 freq = 0;

    if (key.empty ())
        find_keys (keyvec, ch);
    else
        keyvec.push_back (key);

    for (PinyinKeyVector::iterator i = keyvec.begin (); i != keyvec.end (); ++i) {
        std::pair<PinyinEntryVector::iterator, PinyinEntryVector::iterator> range =
            std::equal_range(m_table.begin(), m_table.end(), *i, m_pinyin_key_less);
        for (PinyinEntryVector::iterator vi = range.first; vi!= range.second; ++vi) {
            freq += vi->get_char_frequency (ch);
        }
    }

    return freq;
}

void
PinyinTable::set_char_frequency (ucs4_t ch, uint32 freq, PinyinKey key)
{
    PinyinKeyVector keyvec;

    if (key.empty ())
        find_keys (keyvec, ch);
    else
        keyvec.push_back (key);

    for (PinyinKeyVector::iterator i = keyvec.begin (); i != keyvec.end (); ++i) {
        std::pair<PinyinEntryVector::iterator, PinyinEntryVector::iterator> range =
            std::equal_range(m_table.begin(), m_table.end(), *i, m_pinyin_key_less);
        for (PinyinEntryVector::iterator vi = range.first; vi != range.second; ++vi) {
            vi->set_char_frequency (ch, freq / (keyvec.size () * (range.second - range.first)));
        }
    }
}

void
PinyinTable::refresh (ucs4_t hz, uint32 shift, PinyinKey key)
{
    if (!hz) return;

    PinyinKeyVector keyvec;

    if (key.empty ())
        find_keys (keyvec, hz);
    else
        keyvec.push_back (key);

    for (PinyinKeyVector::iterator i = keyvec.begin (); i != keyvec.end (); ++i) {
        std::pair<PinyinEntryVector::iterator, PinyinEntryVector::iterator> range =
            std::equal_range(m_table.begin(), m_table.end(), *i, m_pinyin_key_less);
        for (PinyinEntryVector::iterator vi = range.first; vi!= range.second; ++vi) {
            vi->refresh_char_frequency (hz, shift);
        }
    }
}

void
PinyinTable::insert (ucs4_t hz, const char *key)
{
    insert (hz, PinyinKey (*m_validator, key));
}

void
PinyinTable::insert (ucs4_t hz, PinyinKey key)
{
    PinyinEntryVector::iterator i =
        std::lower_bound (m_table.begin(), m_table.end(), key, m_pinyin_key_less);

    if (i != m_table.end() && m_pinyin_key_equal (*i, key)) {
        i->insert (CharFrequencyPair (hz,0));
    } else {
        PinyinEntry entry (key);
        entry.insert (CharFrequencyPair (hz,0));
        m_table.insert (i, entry);
    }
    insert_to_reverse_map (hz, key);
}

size_t
PinyinTable::size () const
{
    size_t num = 0;
    for (PinyinEntryVector::const_iterator i = m_table.begin(); i!= m_table.end(); i++)
        num += i->size ();

    return num;
}

int
PinyinTable::find_keys (PinyinKeyVector &vec, ucs4_t code)
{
    if (!m_revmap_ok) create_reverse_map ();

    vec.clear ();

    std::pair<ReversePinyinMap::const_iterator, ReversePinyinMap::const_iterator> result = 
        m_revmap.equal_range (code);
    
    for (ReversePinyinMap::const_iterator i = result.first; i != result.second; i++)
        vec.push_back (i->second);

    return vec.size ();
}

int
PinyinTable::find_key_strings (std::vector<PinyinKeyVector> &vec, const WideString & str)
{
    vec.clear ();

    PinyinKeyVector *key_vectors = new PinyinKeyVector [str.size()];

    for (uint32 i=0; i<str.length (); i++)
        find_keys (key_vectors[i], str [i]);

    PinyinKeyVector key_buffer;

    create_pinyin_key_vector_vector (vec, key_buffer, key_vectors, 0, str.size());

    delete [] key_vectors;
    return vec.size ();
}

bool
PinyinTable::has_key (const char *key) const
{
    return has_key (PinyinKey (*m_validator, key));
}

bool
PinyinTable::has_key (PinyinKey key) const
{
    return std::binary_search (m_table.begin(), m_table.end(), key, m_pinyin_key_less);
}

void
PinyinTable::sort ()
{
    std::sort (m_table.begin(), m_table.end(), m_pinyin_key_less);
}

void
PinyinTable::create_reverse_map ()
{
    m_revmap.clear();

    PinyinKey key;

    for (PinyinEntryVector::iterator i = m_table.begin(); i != m_table.end(); i++) {
        key = i->get_key();
        for (unsigned int j = 0; j < i->size (); j++) {
            m_revmap.insert (ReversePinyinPair (i->get_char_by_index (j), key));
        }
    }

    m_revmap_ok = true;
}

void
PinyinTable::insert_to_reverse_map (ucs4_t code, PinyinKey key)
{
    if (key.empty ())
        return;

    std::pair<ReversePinyinMap::iterator, ReversePinyinMap::iterator> result = 
        m_revmap.equal_range (code);

    for (ReversePinyinMap::iterator i = result.first; i != result.second; i++)
        if (m_pinyin_key_equal (i->second, key)) return;

    m_revmap.insert (ReversePinyinPair (code, key));
}

void
PinyinTable::erase_from_reverse_map (ucs4_t code, PinyinKey key)
{
    if (key.empty ()) {
        m_revmap.erase (code);
    } else {
        std::pair<ReversePinyinMap::iterator, ReversePinyinMap::iterator> result = 
            m_revmap.equal_range (code);

        for (ReversePinyinMap::iterator i = result.first; i != result.second; i++)
            if (m_pinyin_key_equal (i->second, key)) {
                m_revmap.erase (i);
                break;
            }
    }
}

PinyinTable::PinyinEntryVector::iterator
PinyinTable::find_exact_entry (PinyinKey key)
{
    PinyinKeyExactEqualTo eq;
    for (PinyinEntryVector::iterator i=m_table.begin (); i!=m_table.end (); i++)
        if (eq (*i, key)) return i;
    return m_table.end ();
}

void
PinyinTable::create_pinyin_key_vector_vector (std::vector<PinyinKeyVector> &vv,
                                              PinyinKeyVector &key_buffer,
                                              PinyinKeyVector *key_vectors,
                                              int index,
                                              int len)
{
    for (unsigned int i=0; i< key_vectors[index].size(); i++) {
        key_buffer.push_back ((key_vectors[index])[i]);
        if (index == len-1) {
            vv.push_back (key_buffer);
        } else {
            create_pinyin_key_vector_vector (vv, key_buffer, key_vectors, index+1, len);
        }
        key_buffer.pop_back ();
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
