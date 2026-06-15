#pragma once

#include "AcUIList.h"
#include "AcUIScroll.h"

#include "AgcChatMessage.h"
#include "AcUIEventButton.h"
#include "AgcChatTab.h"

#include <list>
#include <map>
#include <set>
#include <deque>

class AcUIMenu;

// 대화 내용을 표시해주는 View UI

class AgcUIChatMessage
{
private:
	typedef std::list< AgcChatMessage* >							ListChatMessage;
	typedef std::list< AgcChatMessage* >::iterator					ListChatMessageIter;

	typedef std::list< AgcChatTab* >								ListChatTab;
	typedef std::list< AgcChatTab* >::iterator						ListChatTabIter;

	typedef std::deque< stChatUsingMsgEntry >						DequeUsingMessage;
public:
	AgcUIChatMessage			( VOID );
	~AgcUIChatMessage			( VOID );

	VOID						Clear					( VOID );
	VOID						ClearUI					( VOID );

	VOID						Initialize				( AgcdUI*	pUI );

	VOID						SetUI					( AgcdUI*	pUI )	{	m_pUI			=	pUI; pUI->m_pcsUIWindow->m_pfAlpha = &m_fAlpha;	}
	VOID						SetManager				( PVOID pManager )	{	m_pManagerClass	=	pManager;	}

	VOID						SetAlpha				( FLOAT fAlpha )	{	m_fAlpha		=	fAlpha;		}
	FLOAT						GetAlpha				( VOID )			{	return m_fAlpha;				}

	VOID						Refresh					( BOOL bMessageRefresh = FALSE );

	VOID						MoveChatWindow			( INT nX , INT nY , INT nW = -1 , INT nH = -1 );

	VOID						OpenSetupMenu			( VOID );

	
	stOriginalMessageInfo*		AddMessage				( UINT ulMsgType , CHAR* pSenderName , CHAR* pMessage , DWORD dwCustomColor = 0 , UINT32 ulCharType = AGPMCHAR_TYPE_NONE );

	BOOL						AddTab					( CONST string& strTabName , UINT ulMsgType , BOOL bPrimary = FALSE );		//	해당 UI에 탭을 추가한다.
	BOOL						DelTab					( CONST string& strTabName );						//	해당 탭을 제거한다.
	AgcChatTab*					GetTab					( INT nIndex );
	INT							GetTabCount				( VOID );

	VOID						InitMessageInstance		( PVOID pvManager );

//	BOOL						AddMessageInstance		( AgcChatMessage*	pMessage );

	AgcChatMessage*				GetMessageInstance		( INT nIndex );
	INT							GetMessageInstanceCount	( VOID );

	BOOL						DestroyMessageInstance	( AgcChatMessage*	pMessage );
	BOOL						DestroyMessageInstance	( INT	nIndex );

	VOID						AllDestroyMessageInstance( VOID );

	BOOL						AddUsingMessage			( stChatMsgEntry*	pEntry );


	BOOL						IsUsableType			( UINT ulMsgType );

	BOOL						SelectTab				( AgcChatTab*	pChatTab );		//	해당탭이 선택이 된다.
	
	// 분리 하고 싶으면 Set
	VOID						SetSeparationTab		( AgcChatMessage*	pChatMessage )	{	m_pSeparationChat	=	pChatMessage;	}
	AgcChatMessage*				GetSeparationTab		( VOID )							{	return m_pSeparationChat;				}

	VOID						SetLastButton			( INT nIndex );

	// 크기가 바뀌면 아이템 갯수도 변한다
	VOID						SetListItemMaxCount		( INT nMaxCount = 0 );


	VOID						SetShowEvent			( BOOL bEnable )		{	m_bEventShow	=	bEnable;	}
	VOID						SetHideEvent			( BOOL bEnable )		{	m_bEventHide	=	bEnable;	}
	VOID						SetDockingEvent			( BOOL bEnable )		{	m_bDockingEvent	=	bEnable;	}

	BOOL						GetShowEvent			( VOID )				{	return m_bEventShow;	}
	BOOL						GetHideEvent			( VOID )				{	return m_bEventHide;	}
	BOOL						GetDockingEvent			( VOID )				{	return m_bDockingEvent;	}

	VOID						ForceDetailShow			( VOID );
	VOID						ForceDetailHide			( VOID );

	VOID						AttachScroll			( VOID );
	VOID						DetachScroll			( VOID );

	VOID						AttachResize			( VOID );
	VOID						DetachResize			( VOID );

	VOID						AttachMove				( VOID );
	VOID						DetachMove				( VOID );

	VOID						AttachSetup				( VOID );
	VOID						DetachSetup				( VOID );

	VOID						ShowTabButton			( VOID );
	VOID						HideTabButton			( VOID );

	VOID						AllDeleteListItem		( VOID );

	VOID						RefreshTabButton		( VOID );								//	Tab Button 갱신
	VOID						RefreshScroll			( VOID );								//	Scroll 갱신
	VOID						RefreshList				( BOOL bRefreshMessage = FALSE);		//	List 갱신
	VOID						RefreshResize			( VOID );
	VOID						RefreshSetup			( VOID );

	BOOL						IsUseMessageType		( AgcChatMessage* pChatMessage );
	BOOL						IsUsing					( VOID );

	VOID						SetFontSize				( INT nFontType , BOOL bRefresh = TRUE );
	VOID						SetTabInput				( BOOL bEnable );

	INT							GetFontSize				( VOID )			{	return m_nFontType;	}
	BOOL						GetTabInput				( VOID )			{	return m_bTabInput;	}

	VOID						SetScrollValue			( FLOAT fValue );
	FLOAT						GetScrollValue			( VOID );

	AgcdUI*						GetUI					( VOID )			{	return m_pUI;		}
	RwV2d						GetUIPos				( VOID );
	RwV2d						GetUISize				( VOID );

	AgcChatMessage*				GetChatMessage			( UINT ulMsgType );

	VOID						SetIndex				( INT nIndex )		{	m_nIndex = nIndex;	}
	INT							GetIndex				( VOID )			{	return m_nIndex;	}

	static BOOL					CBTabClickEvent			( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL					CBMenuEventDocking		( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu );

	static BOOL					CBMenuEventFontSize		( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu );
	static BOOL					CBMenuEventTabEnable	( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu );

protected:
	VOID						_InitUIList				( VOID );	//	위치 & 텍스쳐 세팅
	VOID						_InitUIScroll			( VOID );	//	위치 & 텍스쳐 세팅
	VOID						_InitUITab				( VOID );	//	위치 & 텍스쳐 세팅
	VOID						_InitMenuEvent			( VOID );

	stChatMsgEntry*				_GetUsingMessage		( INT nIndex );

	// 0일 경우 List Control 크기가 자동으로 들어간다
	// 메세지들은 저 길이가 넘으면 자동 줄바꿈이 된다
	VOID						_RefreshMessage			( BOOL bRefresh = FALSE , INT nTextLength = 0 );

	AcUIEventButton				m_UITabButton[MAX_TAB_BUTTON];	//	탭 버튼

	AcUIEventButton				m_UISizeButton;		//	사이즈를 늘릴때 사용
	AcUIEventButton				m_UIMoveButton;		//	이동 버튼
	AcUIEventButton				m_UISetupButton;	//	환경설정 버튼

	AcUIMenu					m_UIMenu;			//	환경설정 메뉴

	AcUIScroll					m_UIScroll;			//	대화가 많아지면 자동으로 붙을 스크롤

	AcUIScrollButton			m_UIScrollButton;	// Scroll 버튼
	AcUIButton					m_UIButtonUP;		//	스크롤 업 버튼
	AcUIButton					m_UIButtonDown;		//	스크롤 다운 버튼

	AgcdUIControl*				m_pListControl;		//	대화 리스트 컨트롤
	AgcdUIControl*				m_pListItem;		//	대화 리스트 아이템

	AgcChatMessage*				m_pSeparationChat;	//	분리 요청을 한 메세지 종류
	AgcdUI*						m_pUI;				//	실제로 사용될 UI

	ListChatMessage				m_listChatMessage;

	ListChatTab					m_listChatTab;		//	탭 분류 리스트
	AgcChatTab*					m_pChatTab;			//	현재 선택된 탭 종류

	DequeUsingMessage			m_dequeUsingMessage;	//	실제로 List Control에 들어갈 메세지

	PVOID						m_pManagerClass;	// Manager Class

	INT32						m_nMaxListCount;
	INT32						m_nFontType;
	INT32						m_nFontHeight;

	FLOAT						m_fAlpha;

	BOOL						m_bTabInput;

	BOOL						m_bDockingEvent;	//	Docking Event 발동 중

	BOOL						m_bEventShow;		//	Show Event가 발동 중
	BOOL						m_bEventHide;		//	Hide Event가 발동 중

	INT							m_nIndex;			//	채팅UI창의 인덱스번호
};
