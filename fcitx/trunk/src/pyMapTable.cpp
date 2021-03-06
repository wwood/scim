#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH

#include "scim.h"
#include "scim_fcitx_imengine.h"
using namespace scim;

#include "pyMapTable.h"

ConsonantMap    consonantMapTable[] = {
    {"a", 'A'}
    ,
    {"ai", 'B'}
    ,
    {"an", 'C'}
    ,
    {"ang", 'D'}
    ,
    {"ao", 'E'}
    ,

    {"e", 'F'}
    ,
    {"ei", 'G'}
    ,
    {"en", 'H'}
    ,
    {"eng", 'I'}
    ,

    {"i", 'J'}
    ,
    {"ia", 'K'}
    ,
    {"ian", 'L'}
    ,
    {"iang", 'M'}
    ,
    {"iao", 'N'}
    ,
    {"ie", 'O'}
    ,
    {"in", 'P'}
    ,
    {"ing", 'Q'}
    ,
    {"iong", 'R'}
    ,
    {"iu", 'S'}
    ,

    {"o", 'T'}
    ,
    {"ong", 'U'}
    ,
    {"ou", 'V'}
    ,

    {"u", 'W'}
    ,
    {"ua", 'X'}
    ,
    {"uai", 'Y'}
    ,
    {"uan", 'Z'}
    ,
    {"uang", 'a'}
    ,
    {"ue", 'b'}
    ,
    {"ui", 'c'}
    ,
    {"un", 'd'}
    ,
    {"uo", 'e'}
    ,

    {"v", 'f'}
    ,
    {"ve", 'g'}
    ,
/*	{"ve",'A'},
	{"v", 'B'},
	
	{"uo", 'C'},
	{"un", 'D'},
	{"ui", 'E'},
	{"ue", 'F'},
	{"uang", 'G'},
	{"uan", 'H'},
	{"uai", 'I'},
	{"ua", 'J'},
	{"u", 'K'},

	{"ou", 'L'},
	{"ong", 'M'},
	{"o", 'N'},

	{"iu", 'O'},
	{"iong", 'P'},
	{"ing", 'Q'},
	{"in", 'R'},
	{"ie", 'S'},
	{"iao", 'T'},
	{"iang", 'U'},
	{"ian", 'V'},
	{"ia", 'W'},
	{"i", 'X'},
	
	{"eng", 'Y'},
	{"en", 'Z'},
	{"ei", 'a'},
	{"e", 'b'},
	
	{"ao", 'c'},
	{"ang", 'd'},
	{"an", 'e'},
	{"ai", 'f'},
	{"a", 'g'},
  */
    {"\0", '\0'}
    ,
};

SyllabaryMap    syllabaryMapTable[] = {
    /*{"b", 'A'},
       {"c", 'B'},
       {"ch", 'C'},
       {"d", 'D'},
       {"f", 'E'},
       {"g", 'F'},
       {"h", 'G'},
       {"j", 'F'},
       {"k", 'I'},
       {"l", 'J'},
       {"m", 'K'},
       {"n", 'L'},
       {"p", 'M'},
       {"q", 'N'},
       {"r", 'O'},
       {"s", 'P'},
       {"sh", 'Q'},
       {"t", 'R'},
       {"w", 'S'},
       {"x", 'T'},
       {"y", 'U'},
       {"z", 'V'},
       {"zh", 'W'}, */

    {"zh", 'A'}
    ,
    {"z", 'B'}
    ,
    {"y", 'C'}
    ,
    {"x", 'D'}
    ,
    {"w", 'E'}
    ,
    {"t", 'F'}
    ,
    {"sh", 'G'}
    ,
    {"s", 'H'}
    ,
    {"r", 'I'}
    ,
    {"q", 'J'}
    ,
    {"p", 'K'}
    ,
    {"ou", 'L'}
    ,
    {"o", 'M'}
    ,
    {"ng", 'N'}
    ,
    {"n", 'O'}
    ,
    {"m", 'P'}
    ,
    {"l", 'Q'}
    ,
    {"k", 'R'}
    ,
    {"j", 'S'}
    ,
    {"h", 'T'}
    ,
    {"g", 'U'}
    ,
    {"f", 'V'}
    ,
    {"er", 'W'}
    ,
    {"en", 'X'}
    ,
    {"ei", 'Y'}
    ,
    {"e", 'Z'}
    ,
    {"d", 'a'}
    ,
    {"ch", 'b'}
    ,
    {"c", 'c'}
    ,
    {"b", 'd'}
    ,
    {"ao", 'e'}
    ,
    {"ang", 'f'}
    ,
    {"an", 'g'}
    ,
    {"ai", 'h'}
    ,
    {"a", 'i'}
    ,

    {"\0", '\0'}
};
