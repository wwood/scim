#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH

#include <scim.h>
#include "scim_fcitx_imengine.h"
using namespace scim;

#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

#include "version.h"
#include "PYFA.h"
#include "py.h"
#include "sp.h"
#include "ime.h"
#include "table.h"

/*
 * ��ȡ�û��������ļ�
 */
void LoadConfig (Bool bMode)
{
    FILE           *fp;
    char            str[PATH_MAX], *pstr;
    char            strPath[PATH_MAX];
    int             i;
    int             r, g, b;	//���������

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcim/config");

    fp = fopen (strPath, "rt");

    if (!fp) {
	SaveConfig ();
	LoadConfig (True);	//����Ĭ��ֵ
	return;
    }

    for (;;) {
	if (!fgets (str, PATH_MAX, fp))
	    break;

	i = strlen (str) - 1;
	while (str[i] == ' ' || str[i] == '\n')
	    str[i--] = '\0';

	pstr = str;
	if (*pstr == ' ')
	    pstr++;
	if (pstr[0] == '#')
	    continue;

	if (strstr (pstr, "��ѡ�ʸ���=")) {
	    pstr += 11;
	    iMaxCandWord = atoi (pstr);
	    if (iMaxCandWord > 10)
		iMaxCandWord = MAX_CAND_WORD;
	}
	else if (strstr (pstr, "���ֺ����Ƿ���=")) {
	    pstr += 17;
	    bEngPuncAfterNumber = atoi (pstr);
	}
	else if (strstr (pstr, "�Ƿ��Զ�����������=")) {
	    pstr += 19;
	    bAutoHideInputWindow = atoi (pstr);
	}
	else if (strstr (pstr, "cursor color=") && bMode) {
	    pstr +=13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    cursorColor=SCIM_RGB_COLOR(r, g, b);
	}
	else if (strstr (pstr, "���봰��ʾɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[0]=SCIM_RGB_COLOR(r, g, b);
	}
	else if (strstr (pstr, "���봰�û�����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[1]=SCIM_RGB_COLOR(r, g, b);
	}
	else if (strstr (pstr, "���봰���ɫ=") && bMode) {
	    pstr += 13;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[2]=SCIM_RGB_COLOR(r, g, b);
	}
	else if (strstr (pstr, "���봰��һ����ѡ��ɫ=") && bMode) {
	    pstr += 21;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[3]=SCIM_RGB_COLOR(r, g, b);
	}
	else if (strstr (pstr, "���봰�û�����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[4]=SCIM_RGB_COLOR(r, g, b);
	}
	else if (strstr (pstr, "���봰��ʾ����ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[5]=SCIM_RGB_COLOR(r, g, b);
	}
	else if (strstr (pstr, "���봰�����ı�ɫ=") && bMode) {
	    pstr += 17;
	    sscanf (pstr, "%d %d %d\n", &r, &g, &b);
	    messageColor[6]=SCIM_RGB_COLOR(r, g, b);
	}
	else if (strstr (pstr, "��Ӣ�Ŀ����л���=")) {
	    pstr += 17;
	    SetSwitchKey (pstr);
	}
	else if (strstr (pstr, "˫����Ӣ���л�=")) {
	    pstr += 15;
	    bDoubleSwitchKey = atoi (pstr);
	}
	else if (strstr (pstr, "GBK֧��=")) {
	    pstr += 8;
	    SetHotKey (pstr, hkGBK);
	}
	else if (strstr (pstr, "�ֺ�����Ӣ��=")) {
	    pstr += 13;
	    bEngAfterSemicolon = atoi (pstr);
	}
	else if (strstr (pstr, "��д��ĸ����Ӣ��=")) {
	    pstr += 17;
	    bEngAfterCap = atoi (pstr);
	}
	else if (strstr (pstr, "���뷽ʽ��ֹ��ҳ=")) {
	    pstr += 17;
	    bDisablePagingInLegend = atoi (pstr);
	}
	else if (strstr (pstr, "����֧��=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkLegend);
	}
	else if (strstr (pstr, "Enter����Ϊ=")) {
	    pstr += 12;
	    enterToDo = (ENTER_TO_DO) atoi (pstr);
	}
	else if (strstr (pstr, "ȫ���=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkCorner);
	}
	else if (strstr (pstr, "���ı��=")) {
	    pstr += 9;
	    SetHotKey (pstr, hkPunc);
	}
	else if (strstr (pstr, "��һҳ=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkPrevPage);
	}
	else if (strstr (pstr, "��һҳ=")) {
	    pstr += 7;
	    SetHotKey (pstr, hkNextPage);
	}
	else if (strstr (pstr, "�ڶ�����ѡ��ѡ���=")) {
	    pstr += 19;
	    if (!strcasecmp (pstr, "SHIFT")) {
		i2ndSelectKey = KeyEvent("Shift+Shift_L");
		i2ndSelectKeyPress = KeyEvent("Shift_L");
		i3rdSelectKey = KeyEvent("Shift+Shift_R");
		i3rdSelectKeyPress = KeyEvent("Shift_R");
	    }
	    else if (!strcasecmp (pstr, "CTRL")) {
		i2ndSelectKey = KeyEvent("Control+Control_L");
		i2ndSelectKeyPress = KeyEvent("Control_L");
		i3rdSelectKey = KeyEvent("Control+Control_R");
		i3rdSelectKeyPress = KeyEvent("Control_R");
	    }
	}

	else if (strstr (pstr, "ʹ��ƴ��=")) {
	    pstr += 9;
	    bUsePinyin = atoi (pstr);
	}
	else if (strstr (pstr, "ʹ��˫ƴ=")) {
	    pstr += 9;
	    bUseSP = atoi (pstr);
	}
	else if (strstr (pstr, "ʹ����λ=")) {
	    pstr += 9;
	    bUseQW = atoi (pstr);
	}
	else if (strstr (pstr, "ʹ�����=")) {
	    pstr += 9;
	    bUseTable = atoi (pstr);
	}
	else if (strstr (pstr, "��ʾ�ʿ��еĴ���=")) {
	    pstr += 17;
	    bPhraseTips = atoi (pstr);
	}

	else if (strstr (str, "ʹ��ȫƴ=")) {
	    pstr += 9;
	    bFullPY = atoi (pstr);
	}
	else if (strstr (str, "ƴ���Զ����=")) {
	    pstr += 13;
	    bPYCreateAuto = atoi (pstr);
	}
	else if (strstr (str, "�����Զ����=")) {
	    pstr += 13;
	    bPYSaveAutoAsPhrase = atoi (pstr);
	}
	else if (strstr (str, "����ƴ��������=")) {
	    pstr += 15;
	    SetHotKey (pstr, hkPYAddFreq);
	}
	else if (strstr (str, "ɾ��ƴ��������=")) {
	    pstr += 15;
	    SetHotKey (pstr, hkPYDelFreq);
	}
	else if (strstr (str, "ɾ��ƴ���û�����=")) {
	    pstr += 17;
	    SetHotKey (pstr, hkPYDelUserPhr);
	}
	else if (strstr (str, "ƴ���Դʶ��ּ�=")) {
	    pstr += 15;
	    cPYYCDZ[0] = pstr[0];
	    cPYYCDZ[1] = pstr[1];
	}
	else if (strstr (str, "ƴ���������������ʽ=")) {
	    pstr += 21;
	    baseOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (strstr (str, "ƴ���������������ʽ=")) {
	    pstr += 21;
	    phraseOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (strstr (str, "ƴ�����ô����������ʽ=")) {
	    pstr += 23;
	    freqOrder = (ADJUSTORDER) atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��an��ang=")) {
	    pstr += 16;
	    MHPY_C[0].bMode = atoi (pstr);
	    MHPY_S[5].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��en��eng=")) {
	    pstr += 16;
	    MHPY_C[1].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��ian��iang=")) {
	    pstr += 18;
	    MHPY_C[2].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��in��ing=")) {
	    pstr += 16;
	    MHPY_C[3].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��ou��u=")) {
	    pstr += 14;
	    MHPY_C[4].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��uan��uang=")) {
	    pstr += 18;
	    MHPY_C[5].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��c��ch=")) {
	    pstr += 14;
	    MHPY_S[0].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��f��h=")) {
	    pstr += 13;
	    MHPY_S[1].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��l��n=")) {
	    pstr += 13;
	    MHPY_S[2].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��s��sh=")) {
	    pstr += 14;
	    MHPY_S[3].bMode = atoi (pstr);
	}
	else if (strstr (str, "�Ƿ�ģ��z��zh=")) {
	    pstr += 14;
	    MHPY_S[4].bMode = atoi (pstr);
	}
    }

    fclose (fp);

}

void SaveConfig (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcim/");

    if (access (strPath, 0))
	mkdir (strPath, S_IRWXU);

    strcat (strPath, "config");
    fp = fopen (strPath, "wt");
    if (!fp) {
	fprintf (stderr, "\n�޷������ļ� config��\n");
	return;
    }

    fprintf (fp, "[����]\n");

    fprintf (fp, "\n[���]\n");
    fprintf (fp, "���ֺ����Ƿ���=%d\n", bEngPuncAfterNumber);
    fprintf (fp, "Enter����Ϊ=%d\n", enterToDo);
    fprintf (fp, "�ֺ�����Ӣ��=%d\n", bEngAfterSemicolon);
    fprintf (fp, "��д��ĸ����Ӣ��=%d\n", bEngAfterCap);
    fprintf (fp, "���뷽ʽ��ֹ��ҳ=%d\n", bDisablePagingInLegend);

    fprintf (fp, "\n[����]\n");
    fprintf (fp, "��ѡ�ʸ���=%d\n", iMaxCandWord);
    fprintf (fp, "�Ƿ��Զ�����������=%d\n", bAutoHideInputWindow);
    
    fprintf (fp, "cursor color=%d %d %d\n",
	     SCIM_RGB_COLOR_RED(cursorColor), SCIM_RGB_COLOR_GREEN(cursorColor), SCIM_RGB_COLOR_BLUE(cursorColor));
    fprintf (fp, "���봰��ʾɫ=%d %d %d\n", 
	     SCIM_RGB_COLOR_RED(messageColor[0]), SCIM_RGB_COLOR_GREEN(messageColor[0]), SCIM_RGB_COLOR_BLUE(messageColor[0]));
    fprintf (fp, "���봰�û�����ɫ=%d %d %d\n", 
	     SCIM_RGB_COLOR_RED(messageColor[1]), SCIM_RGB_COLOR_GREEN(messageColor[1]), SCIM_RGB_COLOR_BLUE(messageColor[1]));
    fprintf (fp, "���봰���ɫ=%d %d %d\n", 
	     SCIM_RGB_COLOR_RED(messageColor[2]), SCIM_RGB_COLOR_GREEN(messageColor[2]), SCIM_RGB_COLOR_BLUE(messageColor[2]));
    fprintf (fp, "���봰��һ����ѡ��ɫ=%d %d %d\n", 
	     SCIM_RGB_COLOR_RED(messageColor[3]), SCIM_RGB_COLOR_GREEN(messageColor[3]), SCIM_RGB_COLOR_BLUE(messageColor[3]));
    fprintf (fp, "#����ɫֵֻ����ƴ���е��û������\n");
    fprintf (fp, "���봰�û�����ɫ=%d %d %d\n", 
	     SCIM_RGB_COLOR_RED(messageColor[4]), SCIM_RGB_COLOR_GREEN(messageColor[4]), SCIM_RGB_COLOR_BLUE(messageColor[4]));
    fprintf (fp, "���봰��ʾ����ɫ=%d %d %d\n", 
	     SCIM_RGB_COLOR_RED(messageColor[5]), SCIM_RGB_COLOR_GREEN(messageColor[5]), SCIM_RGB_COLOR_BLUE(messageColor[5]));
    fprintf (fp, "#��ʡ�ƴ���ĵ���/ϵͳ�����ʹ�ø���ɫ\n");
    fprintf (fp, "���봰�����ı�ɫ=%d %d %d\n", 
	     SCIM_RGB_COLOR_RED(messageColor[6]), SCIM_RGB_COLOR_GREEN(messageColor[6]), SCIM_RGB_COLOR_BLUE(messageColor[6]));


    fprintf (fp, "\n#���ˡ���Ӣ�Ŀ����л������⣬�������ȼ���������Ϊ�������м��ÿո�ָ�\n");
    fprintf (fp, "[�ȼ�]\n");
    fprintf (fp, "#��Ӣ�Ŀ����л��� ��������ΪL_CTRL R_CTRL L_SHIFT R_SHIFT\n");
    fprintf (fp, "��Ӣ�Ŀ����л���=Shift_L\n");
    fprintf (fp, "˫����Ӣ���л�=%d\n", bDoubleSwitchKey);
    fprintf (fp, "GBK֧��=Alt+m\n");
    fprintf (fp, "����֧��=Alt+l\n");
    fprintf (fp, "ȫ���=Shift+space\n");
    fprintf (fp, "���ı��=Alt+space\n");
    fprintf (fp, "��һҳ=comma\n");
    fprintf (fp, "��һҳ=minus\n");
    fprintf (fp, "��һҳ=period\n");
    fprintf (fp, "��һҳ=equal\n");
    fprintf (fp, "#�ڶ�����ѡ��ѡ��� SHIFT/CTRL\n");
    fprintf (fp, "�ڶ�����ѡ��ѡ���=CTRL\n");

    fprintf (fp, "\n[���뷨]\n");
    fprintf (fp, "ʹ��ƴ��=%d\n", bUsePinyin);
    fprintf (fp, "ʹ��˫ƴ=%d\n", bUseSP);
    fprintf (fp, "ʹ����λ=%d\n", bUseQW);
    fprintf (fp, "ʹ�����=%d\n", bUseTable);
    fprintf (fp, "��ʾ�ʿ��еĴ���=%d\n", bPhraseTips);

    fprintf (fp, "\n[ƴ��]\n");
    fprintf (fp, "ʹ��ȫƴ=%d\n", bFullPY);
    fprintf (fp, "ƴ���Զ����=%d\n", bPYCreateAuto);
    fprintf (fp, "�����Զ����=%d\n", bPYSaveAutoAsPhrase);
    fprintf (fp, "����ƴ��������=Control+8\n");
    fprintf (fp, "ɾ��ƴ��������=Control+7\n");
    fprintf (fp, "ɾ��ƴ���û�����=Control+Delete\n");
    fprintf (fp, "#ƴ���Դʶ��ּ����Ⱥź�����Ӽ�����Ҫ�пո�\n");
    fprintf (fp, "ƴ���Դʶ��ּ�=%c%c\n", cPYYCDZ[0], cPYYCDZ[1]);
    fprintf (fp, "#���������ʽ˵����0-->������  1-->���ٵ���  2-->��Ƶ�ʵ���\n");
    fprintf (fp, "ƴ���������������ʽ=%d\n", baseOrder);
    fprintf (fp, "ƴ���������������ʽ=%d\n", phraseOrder);
    fprintf (fp, "ƴ�����ô����������ʽ=%d\n", freqOrder);
    fprintf (fp, "�Ƿ�ģ��an��ang=%d\n", MHPY_C[0].bMode);
    fprintf (fp, "�Ƿ�ģ��en��eng=%d\n", MHPY_C[1].bMode);
    fprintf (fp, "�Ƿ�ģ��ian��iang=%d\n", MHPY_C[2].bMode);
    fprintf (fp, "�Ƿ�ģ��in��ing=%d\n", MHPY_C[3].bMode);
    fprintf (fp, "�Ƿ�ģ��ou��u=%d\n", MHPY_C[4].bMode);
    fprintf (fp, "�Ƿ�ģ��uan��uang=%d\n", MHPY_C[5].bMode);
    fprintf (fp, "�Ƿ�ģ��c��ch=%d\n", MHPY_S[0].bMode);
    fprintf (fp, "�Ƿ�ģ��f��h=%d\n", MHPY_S[1].bMode);
    fprintf (fp, "�Ƿ�ģ��l��n=%d\n", MHPY_S[2].bMode);
    fprintf (fp, "�Ƿ�ģ��s��sh=%d\n", MHPY_S[3].bMode);
    fprintf (fp, "�Ƿ�ģ��z��zh=%d\n", MHPY_S[4].bMode);

    fclose (fp);
}

void LoadProfile (void)
{
    FILE           *fp;
    char            str[PATH_MAX], *pstr;
    char            strPath[PATH_MAX];
    int             i;
    Bool            bRetVal;


    bRetVal = False;
    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcim/profile");

    fp = fopen (strPath, "rt");

    if (fp) {
	for (;;) {
	    if (!fgets (str, PATH_MAX, fp))
		break;

	    i = strlen (str) - 1;
	    while (str[i] == ' ' || str[i] == '\n')
		str[i--] = '\0';

	    pstr = str;

	    if (strstr (str, "�汾=")) {
		pstr += 5;

		if (!strcasecmp (FCITX_VERSION, pstr))
		    bRetVal = True;
	    }
	    else if (strstr (str, "�Ƿ�ȫ��=")) {
		pstr += 9;
		bCorner = atoi (pstr);
	    }
	    else if (strstr (str, "�Ƿ����ı��=")) {
		pstr += 13;
		bChnPunc = atoi (pstr);
	    }
	    else if (strstr (str, "�Ƿ�GBK=")) {
		pstr += 8;
		bUseGBK = atoi (pstr);
	    }
	    else if (strstr (str, "�Ƿ�����=")) {
		pstr += 9;
		bUseLegend = atoi (pstr);
	    }
	    else if (strstr (str, "��ǰ���뷨=")) {
		pstr += 11;
		iIMIndex = atoi (pstr);
	    }
	    else if (strstr (str, "��ֹ�ü����л�=")) {
		pstr += 15;
		bLocked = atoi (pstr);
	    }
	}

	fclose (fp);
    }

    if (!bRetVal) {
	SaveConfig ();
	SaveProfile ();
    }
}

void SaveProfile (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcim/");

    if (access (strPath, 0))
	mkdir (strPath, S_IRWXU);

    strcat (strPath, "profile");
    fp = fopen (strPath, "wt");

    if (!fp) {
	fprintf (stderr, "\n�޷������ļ� profile!\n");
	return;
    }

    fprintf (fp, "�汾=%s\n", FCITX_VERSION);
    fprintf (fp, "�Ƿ�ȫ��=%d\n", bCorner);
    fprintf (fp, "�Ƿ����ı��=%d\n", bChnPunc);
    fprintf (fp, "�Ƿ�GBK=%d\n", bUseGBK);
    fprintf (fp, "�Ƿ�����=%d\n", bUseLegend);
    fprintf (fp, "��ǰ���뷨=%d\n", iIMIndex);
    fprintf (fp, "��ֹ�ü����л�=%d\n", bLocked);

    

    fclose (fp);
}

void SetHotKey (char *strKeys, KeyEvent * hotkey)
{
  if (hotkey[1].empty())
    hotkey[1] = KeyEvent(strKeys);
  else {
    hotkey[0]=hotkey[1];
    hotkey[1]=KeyEvent(strKeys);
  }
}


/*
 * �����ļ����ж�����
 * ע��:�ļ��еĿ���Ҳ��Ϊһ�д���
 */
int CalculateRecordNumber (FILE * fpDict)
{
    char            strText[101];
    int             nNumber = 0;

    for (;;) {
	if (!fgets (strText, 100, fpDict))
	    break;

	nNumber++;
    }
    rewind (fpDict);

    return nNumber;
}

void SetSwitchKey (char *str)
{
    switchKeyPress = KeyEvent(str);
    char *buff = (char*)malloc(sizeof(char)*(strlen(str)+10));
    if(strstr(str, "Control"))
	sprintf(buff, "Control+%s", str);
    else
	sprintf(buff, "Shift+%s", str);
    switchKey = KeyEvent(buff);
    free(buff);
}


Bool CheckHZCharset (char *strHZ)
{
    if (!bUseGBK) {
	//GB2312�ĺ��ֱ������Ϊ����һ���ֽڵ�ֵ��0xA1��0xFE֮��(ʵ��Ϊ0xF7)���ڶ����ֽڵ�ֵ��0xA1��0xFE֮��
	//���ڲ鵽������˵����һ�����ú�ʵ����������
	int             i;

	for (i = 0; i < strlen (strHZ); i++) {
	    if ((unsigned char) strHZ[i] < (unsigned char) 0xA1 || (unsigned char) strHZ[i] > (unsigned char) 0xF7 || (unsigned char) strHZ[i + 1] < (unsigned char) 0xA1 || (unsigned char) strHZ[i + 1] > (unsigned char) 0xFE)
		return False;
	    i++;
	}
    }

    return True;
}
