#ifndef _QUWEI_H
#define _QUWEI_H

#include "ime.h"

INPUT_RETURN_VALUE DoQWInput(const KeyEvent& key);
INPUT_RETURN_VALUE QWGetCandWords (SEARCH_MODE mode);
char *QWGetCandWord (int iIndex);
char           *GetQuWei (int, int);

#endif
