#ifndef __CLASS_CHATTING_MESSAGE_MANAGER_H__
#define __CLASS_CHATTING_MESSAGE_MANAGER_H__

#include "AcChatDefine.h"
#include "AgpmChatting.h"

#include "CTextFilter.h"
#include "CChattingCommander.h"
#include "CSystemMsgManager.h"

class AgcmUIChatting2;
class AgcmMinimap;
class AgcmUIItem;
class AgcmItem;
class AgcmCharacter;
class AgcChatMessage;
class AgcUIChatMessage;
class AgpmChatting;
class AgcmTextBoardMng;
class AgcmUINotice;
class AgcChatTab;
class AgcUIChatMessage;

class AgcChatManager : public CTextFilter, public CChatCmd, public CSystemMsgManager
{
private:
	// 채팅창 UI
	typedef std::vector< AgcUIChatMessage* >				ListUIChatMessage;
	typedef std::vector< AgcUIChatMessage* >::iterator		ListUIChatMessageIter;

	// 메세지가 저장 되는 곳
	typedef std::map< UINT , AgcChatMessage* >				MapChatMessage;
	typedef std::map< UINT , AgcChatMessage* >::iterator	MapChatMessageIter;

	typedef std::list< stDockingEvent >						ListDockingEvent;
	typedef std::list< stDockingEvent >::iterator			ListDockingEventIter;

	typedef	std::list< stChatWindowInfo >					ListChatWindowInfo;
	typedef std::list< stChatWindowInfo >::iterator			ListChatWindowInfoIter;

	MapChatMessage									m_mapChatMessage;
	ListUIChatMessage								m_listUIChatMessage;
	ListDockingEvent								m_listDockingEvent;
	ListChatWindowInfo								m_listChatWindowBase;

	BOOL											m_bIsNeedSave;
	BOOL											m_bIsReady;
	BOOL											m_bIsIdleStart;
	AgcdUIEventMap*									m_pUIWhisperEventMap;

	AgcmUIManager2*									m_pcsAgcmUIManager2;
	AgcmUIChatting2*								m_pcsAgcmUIChatting2;
	AgcmMinimap*									m_pcsAgcmMinimap;
	AgcmUIItem*										m_pcsAgcmUIItem;
	AgcmItem*										m_pcsAgcmItem;
	AgcmCharacter*									m_pcsAgcmCharacter;
	AgcmTextBoardMng*								m_pcsAgcmTextBoardMng;
	AgcmUINotice*									m_pcsAgcmUINotice;

	AgpmCharacter*									m_pcsAgpmCharacter;

	eTabOperation									m_eTabOperation;
	AgcUIChatMessage*								m_pOperationChatUI;
	AgcChatTab*										m_pOperationChatTab;
	UINT											m_ulOperationFlag;

	AgcdUIUserData*									m_pcsDisplayUserData;
	AgcdUIUserData*									m_pcsAllDisplayUserData;

	INT												m_lTabOptionAddOpenEvent;
	INT												m_lTabOptionEditOpenEvent;
	INT												m_lTabOptionUIClose;

	BOOL											m_arrTabFlag[ CHAT_MESSAGE_COUNT ];
	INT												m_Dummy;

	ListUIChatMessage								m_listDeleteUI;

public :
	AgcChatManager				( VOID );
	virtual ~AgcChatManager		( VOID );

	BOOL				OnRegisterUIEvent			( VOID );
	BOOL				OnRegisterCallBack			( VOID );
	BOOL				OnRegisterTextFormat		( VOID );

	VOID				LoadBaseFile				( CONST string&	strFileName );
	VOID				LoadChatFile				( CONST string&	strFileName );
	VOID				SaveChatFile				( CONST string& strFileName );

	VOID				ChatIdle					( DWORD dwTick );

	BOOL				OnUpdateMessageDialog		( __int64 nCurrTime );
	BOOL				OnAddMessage				( UINT eMsgType  , char* pSenderName , char* pMessage , DWORD dwCustomColor = 0x00000000 , INT32 lSenderCID = 0 , UINT32 ulCharType = AGPMCHAR_TYPE_NONE, const char* pTargetName = NULL );

	BOOL				OnCollectControls			( VOID );

	BOOL				AddDockingEvent				( stDockingEvent& stEvent );

	VOID				ShowChatUI					( AgcUIChatMessage*	pUIChatMessage );
	VOID				HideChatUI					( AgcUIChatMessage* pUIChatMessage );

	VOID				AllForceDetailShow			( VOID );
	VOID				AllForceDetailHide			( VOID );

	VOID				AllShowChatUI				( VOID );
	VOID				AllHideChatUI				( VOID );

	VOID				AllClearChatUI				( VOID );

	VOID				AllClearMessage				( VOID );

	BOOL				IsNeedSave					( VOID )		{	return m_bIsNeedSave;			}
	VOID				SetEnableIdle				( BOOL bEnable ){	m_bIsIdleStart	=	bEnable;	}

	AgcUIChatMessage*	SetEmptyUIData				( AgcChatMessage*	pChatMessage , AgcUIChatMessage*	pUITarget = NULL );
	AgcUIChatMessage*	GetEmptyUIData				( VOID );

	AgcUIChatMessage*	CreateUIChatMessage			( VOID );

	BOOL				AddUIChatMessage			( AgcUIChatMessage* pUIChatMessage );
	BOOL				DestroyUIChatMessage		( AgcUIChatMessage* pUIChatMessage );

	AgcChatMessage*		GetChatMessage				( UINT unMessageType ); //	해당 타입의 메세지
	AgcUIChatMessage*	GetUIChatMessage			( INT nIndex );			//	해당 인덱스의 메세지UI

	AgcUIChatMessage*	_CopyUIChatMessage			( AgcUIChatMessage* pUIChatMessage );

	VOID				SetTabButtonSize			( INT nWidth , INT nHeight );
	VOID				SetScrollWidth				( INT nWidth );
	VOID				SetMinChatSize				( INT nWidth , INT nHeight );

	VOID				_ItemEvent					( INT nID );
	VOID				_WhisperEvent				( CONST CHAR* szBuffer );
	VOID				_MapPosEvent				( CONST CHAR* szBuffer , INT nX , INT nY , INT nID );

	UINT				_ParseMessageText			( CONST CHAR*	pMessageText );
	VOID				_DefaultChatSetting			( VOID );
	VOID				_LoadChatSetting			( stChatWindowInfo&	rInfo );

	VOID				_DockingPreEventStart		( AgcUIChatMessage* pDstUI );
	VOID				_DockingPreEventEnd			( AgcUIChatMessage* pDstUI );
	BOOL				_DockingCheck				( AgcUIChatMessage*	pSrcUI , AgcUIChatMessage*	pDstUI , OUT INT& nLength );
	VOID				_CombineDockingUI			( AgcUIChatMessage* pSrcUI , AgcUIChatMessage*	pDstUI );

	static BOOL			CBEventCallback				( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL			CBDragCallback				( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL			CBResizeEvent				( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL			CBMoveEvent					( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL			CBSetupEvent				( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );

	static BOOL			CBScrollRefresh				( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 );

	static BOOL			CallBack_RecvMsgChatting	( PVOID pData, PVOID pClass, PVOID pCustData );

	static BOOL			CallBack_WhisperOffline		( PVOID pData, PVOID pClass, PVOID pCustData );
	static BOOL			CAllBack_WhisperBlock			( PVOID pData, PVOID pClass, PVOID pCustData );

	static DWORD		GetTextColor				( char* pColorName );
	static BOOL			OnAddChattingMessage		( UINT eType, AgpdChatData* ppdData );
	static BOOL			OnAddSystemMessage			( char* pMessage, DWORD dwCustomColor = 0x00000000 );
	static BOOL			OnAddNoticeMessage			( char* pMessage );
	static BOOL			OnAddQuestMessage			( char* pMessage );
	static BOOL			OnAddShrineMessage			( char* pMessage );

	static VOID			CBDockingCallback			( PVOID pClass , PVOID pData1 , PVOID pData2 );	// Docking Event발생
	static VOID			CBDockingPreEvent			( PVOID pClass , PVOID pData1 , PVOID pData2 ); // Docking 가능한 지역에 이동시 발생

	// 옵션에서 확인 버튼을 눌렀을때
	static BOOL			CBOptionOK					( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl );

	// 탭의 옵션 체크버튼을 눌렀을때
	static BOOL			CBTabOptionFlagCheck		( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl );

	// 탭 옵션 체크을 전체로 선택
	static BOOL			CBTabAllOptionFlagCheck		( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl );

	static BOOL			CBInitOpenAddTab			( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl );
	static BOOL			CBInitOpenEditTab			( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl );

	static BOOL			CBDisplayAllCheck			( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL			CBDisplayCheck				( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL			CBDisplayName				( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL			CBDisplayColor				( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );


	static BOOL			CBTabDeleteEvent			( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu );
	static BOOL			CBTabEditEvent				( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu );

	static BOOL			CBAddTab					( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu );

private:
	VOID				_AddOptionTab				( CONST CHAR* szTabName );
	VOID				_EditOptionTab				( CONST CHAR* szTabName );

	string				_GetDisplayName				( INT	lIndex );
	DWORD				_GetDisplayColor			( INT	lIndex );

	VOID				_ParseMessageData			( AgpdChatData* pChatData );
};



#endif