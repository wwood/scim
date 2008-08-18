#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#include "scim.h"
#include "scim_fcitx_imengine.h"
using namespace scim;

#include "punc.h"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "ime.h"
#include "tools.h"

ChnPunc        *chnPunc = NULL;


int LoadPuncDict (void)
{
    FILE           *fpDict;
    int             iRecordNo;
    char            strText[11];
    char            strPath[PATH_MAX];
    char           *pstr;
    int             i;

    strcpy (strPath, SCIM_FCITXDATADIR);
    strcat (strPath, PUNC_DICT_FILENAME);
    fpDict = fopen (strPath, "rt");

    if (!fpDict)
	return False;

    iRecordNo = CalculateRecordNumber (fpDict);
    chnPunc = (ChnPunc *) malloc (sizeof (ChnPunc) * (iRecordNo + 1));

    iRecordNo = 0;
    for (;;) {
	if (!fgets (strText, 10, fpDict))
	    break;
	i = strlen (strText) - 1;

	while ((strText[i] == '\n') || (strText[i] == ' ')) {
	    if (!i)
		break;
	    i--;
	}

	if (i) {
	    strText[i + 1] = '\0';
	    pstr = strText;
	    while (*pstr != ' ')
		chnPunc[iRecordNo].ASCII = *pstr++;

	    while (*pstr == ' ')
		pstr++;

	    chnPunc[iRecordNo].iCount = 0;
	    chnPunc[iRecordNo].iWhich = 0;
	    while (*pstr) {
		i = 0;
		while (*pstr != ' ' && *pstr)
		    chnPunc[iRecordNo].strChnPunc[chnPunc[iRecordNo].iCount][i++] = *pstr++;
		chnPunc[iRecordNo].strChnPunc[chnPunc[iRecordNo].iCount][i] = '\0';
		while (*pstr == ' ')
		    pstr++;
		chnPunc[iRecordNo].iCount++;
	    }
	    iRecordNo++;
	}
    }

    chnPunc[iRecordNo].ASCII = '\0';

    fclose (fpDict);
    return True;
}

/*
 * �ж��ַ��ǲ��Ǳ�����
 * ���򷵻ر����ŵ����������򷵻�-1
 */
int IsPunc (const KeyEvent& key)
{
    if ( !chnPunc || key.mask )
	return -1;
    
    int             iIndex = 0;
    int iKey = key.get_ascii_code();

    while (chnPunc[iIndex].ASCII) {
	if (chnPunc[iIndex].ASCII == iKey)
	    return iIndex;
	iIndex++;
    }

    return -1;
}
