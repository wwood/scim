/** @file scim_pinyin_imengine.h
 * definition of Pinyin IMEngine related classes.
 */

/* 
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_imengine_config_keys.h,v 1.2 2005/08/06 15:19:01 suzhe Exp $
 */

#if !defined (__SCIM_PINYIN_IMENGINE_CONFIG_KEYS_H)
#define __SCIM_PINYIN_IMENGINE_CONFIG_KEYS_H

#define SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_PHRASE_LIB                "/IMEngine/Pinyin/System/PhraseLib"
#define SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_PINYIN_TABLE              "/IMEngine/Pinyin/System/PinyinTable"
#define SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_PINYIN_PHRASE_LIB         "/IMEngine/Pinyin/System/PinyinPhraseLib"
#define SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_PINYIN_PHRASE_INDEX       "/IMEngine/Pinyin/System/PinyinPhraseIndex"
#define SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_SPECIAL_TABLE             "/IMEngine/Pinyin/System/SpecialTable"

#define SCIM_CONFIG_IMENGINE_PINYIN_USER_PHRASE_LIB                  "/IMEngine/Pinyin/User/PhraseLib"
#define SCIM_CONFIG_IMENGINE_PINYIN_USER_PINYIN_TABLE                "/IMEngine/Pinyin/User/PinyinTable"
#define SCIM_CONFIG_IMENGINE_PINYIN_USER_PINYIN_PHRASE_LIB           "/IMEngine/Pinyin/User/PinyinPhraseLib"
#define SCIM_CONFIG_IMENGINE_PINYIN_USER_PINYIN_PHRASE_INDEX         "/IMEngine/Pinyin/User/PinyinPhraseIndex"
#define SCIM_CONFIG_IMENGINE_PINYIN_USER_SPECIAL_TABLE               "/IMEngine/Pinyin/User/SpecialTable"

#define SCIM_CONFIG_IMENGINE_PINYIN_USER_DATA_BINARY                 "/IMEngine/Pinyin/User/DataBinary"

#define SCIM_CONFIG_IMENGINE_PINYIN_SMART_MATCH_LEVEL                "/IMEngine/Pinyin/SmartMatchLevel"
#define SCIM_CONFIG_IMENGINE_PINYIN_MAX_USER_PHRASE_LENGTH           "/IMEngine/Pinyin/MaxUserPhraseLength"
#define SCIM_CONFIG_IMENGINE_PINYIN_MAX_PREEDIT_LENGTH               "/IMEngine/Pinyin/MaxPreeditLength"
#define SCIM_CONFIG_IMENGINE_PINYIN_BURST_STACK_SIZE                 "/IMEngine/Pinyin/BurstStackSize"

#define SCIM_CONFIG_IMENGINE_PINYIN_TONE                             "/IMEngine/Pinyin/Tone"
#define SCIM_CONFIG_IMENGINE_PINYIN_INCOMPLETE                       "/IMEngine/Pinyin/Incomplete"
#define SCIM_CONFIG_IMENGINE_PINYIN_DYNAMIC_ADJUST                   "/IMEngine/Pinyin/DynamicAdjust"
#define SCIM_CONFIG_IMENGINE_PINYIN_DYNAMIC_SENSITIVITY              "/IMEngine/Pinyin/DynamicSensitivity"
#define SCIM_CONFIG_IMENGINE_PINYIN_FULL_WIDTH_PUNCT_KEY             "/IMEngine/Pinyin/FullWidthPunctKey"
#define SCIM_CONFIG_IMENGINE_PINYIN_FULL_WIDTH_LETTER_KEY            "/IMEngine/Pinyin/FullWidthLetterKey"
#define SCIM_CONFIG_IMENGINE_PINYIN_MODE_SWITCH_KEY                  "/IMEngine/Pinyin/ModeSwitchKey"
#define SCIM_CONFIG_IMENGINE_PINYIN_CHINESE_SWITCH_KEY               "/IMEngine/Pinyin/ChineseSwitchKey"
#define SCIM_CONFIG_IMENGINE_PINYIN_PAGE_UP_KEY                      "/IMEngine/Pinyin/PageUpKey"
#define SCIM_CONFIG_IMENGINE_PINYIN_PAGE_DOWN_KEY                    "/IMEngine/Pinyin/PageDownKey"
#define SCIM_CONFIG_IMENGINE_PINYIN_DISABLE_PHRASE_KEY               "/IMEngine/Pinyin/DisablePhraseKey"
#define SCIM_CONFIG_IMENGINE_PINYIN_MATCH_LONGER_PHRASE              "/IMEngine/Pinyin/MatchLongerPhrase"
#define SCIM_CONFIG_IMENGINE_PINYIN_AUTO_COMBINE_PHRASE              "/IMEngine/Pinyin/AutoCombinePhrase"
#define SCIM_CONFIG_IMENGINE_PINYIN_AUTO_FILL_PREEDIT                "/IMEngine/Pinyin/AutoFillPreedit"
#define SCIM_CONFIG_IMENGINE_PINYIN_ALWAYS_SHOW_LOOKUP               "/IMEngine/Pinyin/AlwaysShowLookup"
#define SCIM_CONFIG_IMENGINE_PINYIN_SHOW_ALL_KEYS                    "/IMEngine/Pinyin/ShowAllKeys"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY                        "/IMEngine/Pinyin/Ambiguity"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_ANY                    "/IMEngine/Pinyin/Ambiguity/Any"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_ZhiZi                  "/IMEngine/Pinyin/Ambiguity/ZhiZi"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_ChiCi                  "/IMEngine/Pinyin/Ambiguity/ChiCi"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_ShiSi                  "/IMEngine/Pinyin/Ambiguity/ShiSi"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_NeLe                   "/IMEngine/Pinyin/Ambiguity/NeLe"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_LeRi                   "/IMEngine/Pinyin/Ambiguity/LeRi"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_FoHe                   "/IMEngine/Pinyin/Ambiguity/FoHe"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_AnAng                  "/IMEngine/Pinyin/Ambiguity/AnAng"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_EnEng                  "/IMEngine/Pinyin/Ambiguity/EnEng"
#define SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_InIng                  "/IMEngine/Pinyin/Ambiguity/InIng"
#define SCIM_CONFIG_IMENGINE_PINYIN_SAVE_PERIOD                      "/IMEngine/Pinyin/SavePeriod"

#define SCIM_CONFIG_IMENGINE_PINYIN_SHUANG_PIN                       "/IMEngine/Pinyin/ShuangPin"
#define SCIM_CONFIG_IMENGINE_PINYIN_SHUANG_PIN_SCHEME                "/IMEngine/Pinyin/ShuangPinScheme"
#endif
/*
vi:expandtab:ts=4:nowrap:ai
*/

