#ifndef _IME_H
#define _IME_H

#include "xim.h"

#include "scim_fcitx_imengine.h"

#define MAX_CAND_WORD	10
#define MAX_USER_INPUT 300

#define HOT_KEY_COUNT	2
#define MAX_HZ_SAVED    1024

#define MAX_IM_NAME	12

#define NAME_OF_PINYIN		"pinyin"
#define NAME_OF_SHUANGPIN	"shuangpin"
#define NAME_OF_QUWEI		"quwei"

#define TEMP_FILE		"FCITX_DICT_TEMP"

typedef enum _SEARCH_MODE {
    SM_FIRST,
    SM_NEXT,
    SM_PREV
} SEARCH_MODE;

typedef enum ADJUST_ORDER {
    AD_NO,
    AD_FAST,
    AD_FREQ
} ADJUSTORDER;

typedef enum _INPUT_RETURN_VALUE {
    //IRV_UNKNOWN = -1,
    IRV_DO_NOTHING = 0,
    IRV_DONOT_PROCESS,
    IRV_DONOT_PROCESS_CLEAN,
    IRV_CLEAN,
    IRV_TO_PROCESS,
    IRV_DISPLAY_MESSAGE,
    IRV_DISPLAY_CANDWORDS,
    IRV_DISPLAY_LAST,
    IRV_PUNC,
    IRV_ENG,
    IRV_GET_LEGEND,
    IRV_GET_CANDWORDS,
    IRV_GET_CANDWORDS_NEXT
} INPUT_RETURN_VALUE;

typedef enum _ENTER_TO_DO {
    K_ENTER_NOTHING,
    K_ENTER_CLEAN,
    K_ENTER_SEND
} ENTER_TO_DO;

typedef struct _SINGLE_HZ {
    char            strHZ[3];
} SINGLE_HZ;

typedef enum _KEY_RELEASED {
    KR_OTHER = 0,
    KR_CTRL,
    KR_2ND_SELECTKEY,
    KR_3RD_SELECTKEY
} KEY_RELEASED;

typedef struct {
    char            strName[MAX_IM_NAME + 1];
    void            (*ResetIM) (void);
                    INPUT_RETURN_VALUE (*DoInput) (const KeyEvent&);
                    INPUT_RETURN_VALUE (*GetCandWords) (SEARCH_MODE);
    char           *(*GetCandWord) (int);
    char           *(*GetLegendCandWord) (int);
                    Bool (*PhraseTips) (void);
    void            (*Init) (void);
    void            (*Destroy) (void);
} IM;


#define MESSAGE_MAX_LENGTH	300	//����������ʾ������ȣ����ַ���

/* ������������ʾ�����ݷ�Ϊ���¼��� */
#define MESSAGE_TYPE_COUNT	7

typedef enum {
    MSG_TIPS,			//��ʾ�ı�
    MSG_INPUT,			//�û�������
    MSG_INDEX,			//��ѡ��ǰ������
    MSG_FIRSTCAND,		//��һ����ѡ��
    MSG_USERPHR,		//�û�����
    MSG_CODE,			//��ʾ�ı���
    MSG_OTHER			//�����ı�
} MSG_TYPE;

typedef struct {
    char            strMsg[MESSAGE_MAX_LENGTH + 1];
    int        type;
} MESSAGE;


extern IM             *im;
extern INT8            iIMCount;

extern int             iMaxCandWord;
extern int             iCandPageCount;
extern int             iCurrentCandPage;
extern int             iCandWordCount;

extern int             iLegendCandWordCount;
extern int             iLegendCandPageCount;
extern int             iCurrentLegendCandPage;

extern int             iCodeInputCount;

// *************************************************************
extern char            strCodeInput[];
extern char            strStringGet[];	//�������뷨���ص���Ҫ�͵��ͻ������е��ִ�
extern unsigned int   messageColor[];
extern unsigned int   cursorColor;
// *************************************************************

extern ENTER_TO_DO     enterToDo;

extern Bool            bCorner;	//ȫ����л�
extern Bool            bChnPunc;	//��Ӣ�ı���л�
extern Bool            bUseGBK;	//�Ƿ�֧��GBK
extern Bool            bIsDoInputOnly;	//�����Ƿ�ֻ�����뷨���������
extern Bool            bLastIsNumber;	//��һ�������ǲ��ǰ���������
extern Bool            bInCap;	//�ǲ��Ǵ��ڴ�д���Ӣ��״̬
extern Bool            bAutoHideInputWindow;	//�Ƿ��Զ�����������
extern Bool            bEngPuncAfterNumber;	//���ֺ��������Ƿ���(ֻ��'.'/','��Ч)
extern Bool            bPhraseTips;
extern INT8            lastIsSingleHZ;
extern int 	       iCursorPos;
extern Bool            bEngAfterSemicolon;
extern Bool            bEngAfterCap;
extern Bool            bDisablePagingInLegend;

extern KeyEvent             i2ndSelectKey;	//�ڶ�����ѡ��ѡ�����Ϊɨ����
extern KeyEvent             i2ndSelectKeyPress;	//�ڶ�����ѡ��ѡ�����Ϊɨ����
extern KeyEvent             i3rdSelectKey;	//��������ѡ��ѡ�����Ϊɨ����
extern KeyEvent             i3rdSelectKeyPress;	//��������ѡ��ѡ�����Ϊɨ����


extern KEY_RELEASED    keyReleased;
extern Bool		bDoubleSwitchKey;
extern KeyEvent          switchKey;
extern KeyEvent          switchKeyPress;

//�ȼ�����
extern KeyEvent         hkGBK[];
extern KeyEvent         hkLegend[];
extern KeyEvent         hkCorner[];	//ȫ����л�
extern KeyEvent         hkPunc[];	//���ı��
extern KeyEvent         hkNextPage[];	//��һҳ
extern KeyEvent         hkPrevPage[];	//��һҳ

extern Bool            bUseLegend;
extern Bool            bIsInLegend;

extern INT8            iIMIndex;
extern Bool            bUsePinyin;
extern Bool            bUseSP;
extern Bool            bUseQW;
extern Bool            bUseTable;


extern MESSAGE         messageUp[];
extern uint            uMessageUp;
extern MESSAGE         messageDown[];
extern uint            uMessageDown;
extern Bool            bShowPrev;
extern Bool            bShowNext;
extern Bool	       bShowCursor;
extern Bool		bLocked;


void            ResetInput (void);
bool            ProcessKey (FcitxInstance &fInst, const KeyEvent& key);

Bool            IsHotKey (const KeyEvent& key, KeyEvent * hotkey);
INPUT_RETURN_VALUE ChangeCorner (FcitxInstance& finst);
INPUT_RETURN_VALUE ChangePunc (FcitxInstance& finst);
INPUT_RETURN_VALUE ChangeGBK (FcitxInstance& finst);
INPUT_RETURN_VALUE ChangeLegend (FcitxInstance& finst);
void            SwitchIM (INT8 index);
void            DoPhraseTips ();

void            RegisterNewIM (char *strName, 
			       void (*ResetIM) (void), 
			       INPUT_RETURN_VALUE (*DoInput) (int), 
			       INPUT_RETURN_VALUE (*GetCandWords) (SEARCH_MODE), 
			       char *(*GetCandWord) (int), 
			       char *(*GetLegendCandWord) (int),
			       Bool (*PhraseTips) (void), 
			       void (*Init) (void), 
			       void (*Destroy) (void));


Bool            IsIM (char *strName);
void            SaveIM (void);
void            SetIM (void);
void 		CloseIM(FcitxInstance&);
#endif
