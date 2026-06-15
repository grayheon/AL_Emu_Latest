#ifndef	__AGCMUICHATTING2_H__
#define	__AGCMUICHATTING2_H__



#include "AcUIChattingEdit.h"
#include "AgcmUIManager2.h"
#include "AgcChatManager.h"
#include "COptionFlagController.h"
#include "CChattingInputController.h"
#include "CBGMTitleViewer.h"
#include <string>

#define UI_MESSAGE_ID_CHAT_OPTION_SKILL_EFFECT_OFF			"Option_SkillEffect_Off"
#define UI_MESSAGE_ID_CHAT_OPTION_SKILL_EFFECT_ON			"Option_SkillEffect_On"
#define UI_MESSAGE_ID_CHAT_OPTION_OFF_COLOR					"Negative_Text_Color"
#define UI_MESSAGE_ID_CHAT_OPTION_ON_COLOR					"Positive_Text_Color"
#define UI_MESSAGE_ID_CHAT_OPTION_OTHER_SKILL_EFFECT_OFF	"Option_OtherSkillEffect_Off"
#define UI_MESSAGE_ID_CHAT_OPTION_OTHER_SKILL_EFFECT_ON		"Option_OtherSkillEffect_On"


class AgcmUIChatting2 : public AgcModule, public AgcChatManager, public COptionFlagController, public CChattingInputController, public CBGMTitleViewer
{
private:
	DWORD												m_dwTimeLastSendAreaMsg;

public:
	AgcmUIChatting2( void );
	virtual ~AgcmUIChatting2( void );

public :
	BOOL				OnAddModule						( void );
	BOOL				OnInit							( void );
	BOOL				OnIdle							( UINT32 ulClockCount );
	BOOL				OnDestroy						( void );

public :
	BOOL				OnSendMessage					( AgpdChattingType eType, char* pMessage, int nLength, char* pTargetID , BOOL bParse = TRUE );

	VOID				SetLastWhisper					( CONST CHAR *pMessage );

public:
	BOOL				IsEnableSendAreaChatting		( DWORD dwTime );
	BOOL				UpdateTimeLastSendAreaMsg		( DWORD dwTime );

public :
	static BOOL			CBSetSelfCharacter				( void* pData, void* pClass, void* pCustData );
	static BOOL			CBCheckNCloseInput				( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl );
	static BOOL			CBActiveChatInput				( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pTarget, AgcdUIControl* pcsSourceControl );
	static BOOL			CBSendWhisper					( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pTarget, AgcdUIControl* pcsSourceControl );
	static BOOL			CBClickSyncPosBtn				( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pTarget, AgcdUIControl* pcsSourceControl );
	static BOOL			CBClickEscapeConfirmOk			( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pTarget, AgcdUIControl* pcsSourceControl );

};

#endif