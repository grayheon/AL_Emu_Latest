#include "AgcUIChatMessage.h"
#include "AgcmUIManager2.h"
#include "AgcChatManager.h"
#include "AgcmFont.h"

#include "AcUIMenu.h"

#include "AuStrTable.h"

#define	 	SCROLL_BUTTON_CENTER_NORMAL			"MainChat_Scroll_a.png"
#define	 	SCROLL_BUTTON_CENTER_ONMOUSE		"MainChat_Scroll_b.png"
#define	 	SCROLL_BUTTON_CENTER_CLICK			"MainChat_Scroll_c.png"
#define	 	SCROLL_BUTTON_CENTER_DISABLE		"MainChat_Scroll_c.png"

#define		SCROLL_BUTTON_UP_NORMAL				"MainChat_Scroll_UP_a.png"
#define		SCROLL_BUTTON_UP_ONMOUSE			"MainChat_Scroll_UP_b.png"
#define		SCROLL_BUTTON_UP_CLICK				"MainChat_Scroll_UP_c.png"
#define		SCROLL_BUTTON_UP_DISABLE			"MainChat_Scroll_UP_c.png"

#define		SCROLL_BUTTON_DN_NORMAL				"MainChat_Scroll_DN_a.png"
#define		SCROLL_BUTTON_DN_ONMOUSE			"MainChat_Scroll_DN_b.png"
#define		SCROLL_BUTTON_DN_CLICK				"MainChat_Scroll_DN_c.png"
#define		SCROLL_BUTTON_DN_DISABLE			"MainChat_Scroll_DN_c.png"

#define		RESIZE_BUTTON_NORAML				"MainChat_Btn_Expand_a.png"
#define		RESIZE_BUTTON_ONMOUSE				"MainChat_Btn_Expand_b.png"
#define		RESIZE_BUTTON_CLICK					"MainChat_Btn_Expand_c.png"
#define		RESIZE_BUTTON_DISABLE				"MainChat_Btn_Expand_c.png"

#define		SETUP_BUTTON_NORAML					"MainChat_Btn_Option_a.png"
#define		SETUP_BUTTON_ONMOUSE				"MainChat_Btn_Option_b.png"
#define		SETUP_BUTTON_CLICK					"MainChat_Btn_Option_c.png"
#define		SETUP_BUTTON_DISABLE				"MainChat_Btn_Option_c.png"

#define		MOVE_BUTTON_NORAML					"MainChat_Btn_Move_a.png"
#define		MOVE_BUTTON_ONMOUSE					"MainChat_Btn_Move_b.png"
#define		MOVE_BUTTON_CLICK					"MainChat_Btn_Move_c.png"
#define		MOVE_BUTTON_DISABLE					"MainChat_Btn_Move_c.png"

#define		TAB_BUTTON_NORMAL					"MainChat_Tab_a.png"
#define		TAB_BUTTON_ONMOUSE					"MainChat_Tab_b.png"
#define		TAB_BUTTON_CLICK					"MainChat_Tab_c.png"

const int nSizeOfName = 32;

#define		ACTIVE_BUTTON_TEXT_COLOR			0xffffff00

AgcUIChatMessage::AgcUIChatMessage( VOID )
{
	m_pUI				=	NULL;
	m_pSeparationChat	=	NULL;
	m_nMaxListCount		=	0;
	m_pManagerClass		=	NULL;
	m_fAlpha			=	1.0f;

	m_bEventShow		=	FALSE;
	m_bEventHide		=	FALSE;
	m_bDockingEvent		=	FALSE;
	m_nFontType			=	0;
	m_bTabInput			=	TRUE;
	m_nFontHeight		=	20;
	m_pChatTab			=	NULL;
	m_nIndex			=	0;
}

AgcUIChatMessage::~AgcUIChatMessage( VOID )
{
	ClearUI();
	Clear();
}

VOID	AgcUIChatMessage::Initialize( AgcdUI* pUI )
{
	if( !pUI )
		return;

	AgcmUIManager2*	pcmUIManager2	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule("AgcmUIManager2") );
	if( !pcmUIManager2 )
		return;
	
	// 채팅 UI 설정
	SetUI( pUI );

	// 탭버튼 텍스쳐 지정
	for( INT i = 0 ; i < MAX_TAB_BUTTON ; ++i )
	{
		m_UITabButton[i].AddButtonImage( TAB_BUTTON_NORMAL	, ACUIBUTTON_MODE_NORMAL );
		m_UITabButton[i].AddButtonImage( TAB_BUTTON_ONMOUSE , ACUIBUTTON_MODE_ONMOUSE );
		m_UITabButton[i].AddButtonImage( TAB_BUTTON_CLICK	, ACUIBUTTON_MODE_CLICK );
		m_UITabButton[i].SetButtonEnable( TRUE );
	}

	// Custom Render를 한다...
	m_pUI->m_pcsUIWindow->SetDefaultRenderType( UICM_BASE_RGBA_RENDER );
	m_pUI->m_pcsUIWindow->MoveWindow( 50 , 50 , MAX_LISTBOX_STRING , MAX_LISTBOX_STRING );

	// List를 붙인다
	m_pListControl	=	pcmUIManager2->AddControl( pUI , AcUIBase::TYPE_LIST , 0 , 0 );
	if( !m_pListControl )
		return;

	m_pListControl->m_pcsBase->MoveWindow( SCROLL_SIZE_X , (INT32)m_UITabButton[0].m_fTextureHeight , MAX_LISTBOX_STRING-SCROLL_SIZE_X , MAX_LISTBOX_STRING-(INT32)m_UITabButton[0].m_fTextureHeight );

	// List에 아이템을 붙인다
	m_pListItem		=	pcmUIManager2->AddControl( pUI , AcUIBase::TYPE_LIST_ITEM , 0 , 0 , m_pListControl->m_pcsBase , NULL , NULL );
	if( !m_pListItem )
		return;

	// List에 Item Control 세팅
	((AcUIList*)m_pListControl->m_pcsBase)->SetListItemWindowMoveInfo( m_pListItem->m_pcsBase->w , m_pListItem->m_pcsBase->h , m_pListItem->m_pcsBase->x , m_pListItem->m_pcsBase->y );
	pUI->m_pcsUIWindow->UpdateChildWindow();
	m_pListControl->m_uoData.m_stList.m_pcsListItemFormat	=	m_pListItem;
	m_pListControl->m_pcsBase->DeleteChild( m_pListItem->m_pcsBase , FALSE , FALSE );

	// Resize 버튼을 붙인다
	m_UISizeButton.AddButtonImage( RESIZE_BUTTON_NORAML , ACUIBUTTON_MODE_NORMAL );
	m_UISizeButton.AddButtonImage( RESIZE_BUTTON_ONMOUSE, ACUIBUTTON_MODE_ONMOUSE );
	m_UISizeButton.AddButtonImage( RESIZE_BUTTON_CLICK	, ACUIBUTTON_MODE_CLICK );
	m_UISizeButton.AddButtonImage( RESIZE_BUTTON_DISABLE, ACUIBUTTON_MODE_DISABLE );
	m_UISizeButton.SetButtonEnable( TRUE );

	m_UISizeButton.MoveWindow( 0 , 0 , (INT32)m_UISizeButton.m_fTextureWidth , (INT32)m_UISizeButton.m_fTextureHeight );
	m_UISizeButton.SetCallbackDragEvent( m_pManagerClass , AgcChatManager::CBResizeEvent );
	m_UISizeButton.SetDragData( this , NULL );

	m_pUI->m_pcsUIWindow->AddChild( &m_UISizeButton );

	// 이동 버튼
	m_UIMoveButton.AddButtonImage( MOVE_BUTTON_NORAML , ACUIBUTTON_MODE_NORMAL );
	m_UIMoveButton.AddButtonImage( MOVE_BUTTON_ONMOUSE, ACUIBUTTON_MODE_ONMOUSE );
	m_UIMoveButton.AddButtonImage( MOVE_BUTTON_CLICK	, ACUIBUTTON_MODE_CLICK );
	m_UIMoveButton.AddButtonImage( MOVE_BUTTON_DISABLE, ACUIBUTTON_MODE_DISABLE );
	m_UIMoveButton.SetButtonEnable( TRUE );
	
	m_UIMoveButton.MoveWindow( 0 , 0 , (INT32)m_UIMoveButton.m_fTextureWidth , (INT32)m_UIMoveButton.m_fTextureHeight );
	m_UIMoveButton.SetCallbackDragEvent( m_pManagerClass , AgcChatManager::CBMoveEvent );
	m_UIMoveButton.SetDragData( this , NULL );
	m_UIMoveButton.SetDockingCallback( m_pManagerClass , this , NULL , AgcChatManager::CBDockingCallback );

	m_pUI->m_pcsUIWindow->AddChild( &m_UIMoveButton );

	// 옵션 버튼
	m_UISetupButton.AddButtonImage( SETUP_BUTTON_NORAML , ACUIBUTTON_MODE_NORMAL );
	m_UISetupButton.AddButtonImage( SETUP_BUTTON_ONMOUSE, ACUIBUTTON_MODE_ONMOUSE );
	m_UISetupButton.AddButtonImage( SETUP_BUTTON_CLICK	, ACUIBUTTON_MODE_CLICK );
	m_UISetupButton.AddButtonImage( SETUP_BUTTON_DISABLE, ACUIBUTTON_MODE_DISABLE );
	m_UISetupButton.SetButtonEnable( TRUE );

	m_UISetupButton.MoveWindow( 0 , 0 , (INT32)m_UISetupButton.m_fTextureWidth , (INT32)m_UISetupButton.m_fTextureHeight );
	m_UISetupButton.SetCallbackClickEvent( m_pManagerClass , AgcChatManager::CBSetupEvent );
	m_UISetupButton.SetData( this , NULL );

	m_pUI->m_pcsUIWindow->AddChild( &m_UISetupButton );

	// Scroll을 붙인다
	m_pUI->m_pcsUIWindow->AddChild( &m_UIScroll );
	m_UIScroll.MoveWindow( 0 , 50 , 50 , (INT32)(MAX_LISTBOX_STRING-m_UITabButton[0].m_fTextureHeight) );

	m_UIScroll.SetDrawCenterLine( TRUE );
	m_UIScroll.SetCenterLineSize( 6 );

	// Scroll에 업&다운 , 가운데 버튼을 차일드로 붙인다
	m_UIScroll.AddChild( &m_UIButtonDown );
	m_UIScroll.AddChild( &m_UIButtonUP );
	m_UIScroll.AddChild( &m_UIScrollButton );

	m_UIScroll.SetRefreshCallback( m_pManagerClass , AgcChatManager::CBScrollRefresh , this , NULL , NULL );

	// 스크롤에 셋팅
	m_UIScroll.SetScrollButton( &m_UIScrollButton );
	m_UIScroll.SetScrollDownButton( &m_UIButtonDown );
	m_UIScroll.SetScrollUpButton( &m_UIButtonUP );

	// 스크롤 중앙 버튼 세팅
	m_UIScrollButton.AddButtonImage( SCROLL_BUTTON_CENTER_NORMAL	, ACUIBUTTON_MODE_NORMAL );
	m_UIScrollButton.AddButtonImage( SCROLL_BUTTON_CENTER_ONMOUSE	, ACUIBUTTON_MODE_ONMOUSE );
	m_UIScrollButton.AddButtonImage( SCROLL_BUTTON_CENTER_CLICK		, ACUIBUTTON_MODE_CLICK );
	m_UIScrollButton.AddButtonImage( SCROLL_BUTTON_CENTER_DISABLE	, ACUIBUTTON_MODE_DISABLE );
	m_UIScrollButton.SetButtonEnable( TRUE );

	m_UIScrollButton.MoveWindow( 0 , 0 , (INT32)m_UIScrollButton.m_fTextureWidth ,(INT32) m_UIScrollButton.m_fTextureHeight );

	// 스크롤 업 버튼 세팅
	m_UIButtonUP.AddButtonImage( SCROLL_BUTTON_UP_NORMAL	, ACUIBUTTON_MODE_NORMAL );
	m_UIButtonUP.AddButtonImage( SCROLL_BUTTON_UP_ONMOUSE	, ACUIBUTTON_MODE_ONMOUSE );
	m_UIButtonUP.AddButtonImage( SCROLL_BUTTON_UP_CLICK		, ACUIBUTTON_MODE_CLICK );
	m_UIButtonUP.AddButtonImage( SCROLL_BUTTON_UP_DISABLE	, ACUIBUTTON_MODE_DISABLE );
	m_UIButtonUP.SetButtonEnable( TRUE );

	// 스크롤 다운 버튼 세팅
	m_UIButtonDown.AddButtonImage( SCROLL_BUTTON_DN_NORMAL	, ACUIBUTTON_MODE_NORMAL );
	m_UIButtonDown.AddButtonImage( SCROLL_BUTTON_DN_ONMOUSE , ACUIBUTTON_MODE_ONMOUSE );
	m_UIButtonDown.AddButtonImage( SCROLL_BUTTON_DN_CLICK	, ACUIBUTTON_MODE_CLICK );
	m_UIButtonDown.AddButtonImage( SCROLL_BUTTON_DN_DISABLE , ACUIBUTTON_MODE_DISABLE );
	m_UIButtonDown.SetButtonEnable( TRUE );


	// Tab Button을 붙인다.
	for( INT i = 0 ; i < MAX_TAB_BUTTON ; ++i )
	{
		m_pUI->m_pcsUIWindow->AddChild( &m_UITabButton[i] );
		m_UITabButton[i].m_Property.bVisible	=	FALSE;
	}

	m_UIScroll.SetScrollButtonInfo( TRUE , 50 , 100 , 1.0f  );

	// Menu 
	m_UIMenu.Initialize( E_MENU_DRAW_TEXTURE );
	pcmUIManager2->AddWindow( &m_UIMenu );

	AcUIMenu*	pFontMenu	=	m_UIMenu.CreateChildMenu( "FontSizeMenu" );
	AcUIMenu*	pTabMenu	=	m_UIMenu.CreateChildMenu( "TabButtonMenu" );

	pcmUIManager2->AddWindow( pFontMenu );
	pcmUIManager2->AddWindow( pTabMenu );
	
	SetListItemMaxCount();

	// Refresh
	Refresh();
}

VOID	AgcUIChatMessage::Clear( VOID )
{
	ListChatMessageIter		Iter	=	m_listChatMessage.begin();
	for( ; Iter != m_listChatMessage.end() ; ++Iter )
	{
		DEF_SAFEDELETE( (*Iter) );
	}

	m_listChatMessage.clear();

	ListChatTabIter			TabIter	=	m_listChatTab.begin();
	for( ; TabIter != m_listChatTab.end() ; ++TabIter )
	{
		DEF_SAFEDELETE( (*TabIter) );
	}

	m_listChatTab.clear();
}

VOID	AgcUIChatMessage::ClearUI( VOID )
{
	AllDeleteListItem();

	AgcmUIManager2*	pcmUIManager2	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule("AgcmUIManager2") );
	if( !pcmUIManager2 )
		return;

	if( !m_pUI || !m_pUI->m_pcsUIWindow )
		return;

	m_UIScroll.AllDeleteChild( FALSE , FALSE );
	m_UIScroll.UpdateChildWindow();

	AgcUIWindow*	pWindow	=	m_pUI->m_pcsUIWindow;

	pWindow->AllDeleteChild( FALSE , FALSE );
	pWindow->UpdateChildWindow();

	AcUIMenu*	pFontMenu	=	m_UIMenu.GetChildMenu( "FontSizeMenu" );
	if( pFontMenu )
	{
		pcmUIManager2->RemoveWindow( pFontMenu , TRUE );
	}
	AcUIMenu*	pTabMenu	=	m_UIMenu.GetChildMenu( "TabButtonMenu" );
	if( pTabMenu )
	{
		pcmUIManager2->RemoveWindow( pTabMenu , TRUE );
	}

	pcmUIManager2->RemoveWindow( &m_UIMenu , TRUE );

	pcmUIManager2->CloseUI( m_pUI , FALSE , FALSE , TRUE , FALSE );

	pcmUIManager2->DestroyUI( m_pUI );
}

VOID	AgcUIChatMessage::OpenSetupMenu( VOID )
{
	_InitMenuEvent();

	INT		nX		=	0;
	INT		nY		=	0;
	m_UISetupButton.ClientToScreen( &nX , &nY );

	m_UIMenu.SetFocusedWindow();
	m_UIMenu.MoveWindow( nX , nY , 100 , 100 );

	m_UIMenu.ShowWindow( TRUE );
}

VOID	AgcUIChatMessage::Refresh( BOOL bMessageRefresh /* = FALSE */ )
{
	RefreshTabButton();
	RefreshResize();
	RefreshSetup();

	SetListItemMaxCount();

	RefreshList( bMessageRefresh );
	RefreshScroll();
}

stOriginalMessageInfo*	AgcUIChatMessage::AddMessage( UINT ulMsgType , CHAR* pSenderName , CHAR* pMessage , DWORD dwCustomColor /* = 0  */, UINT32 ulCharType /* = AGPMCHAR_TYPE_NONE */ )
{
	stOriginalMessageInfo*	pMessageInfo	=	NULL;
	ListChatMessageIter		Iter			=	m_listChatMessage.begin();
	for( ; Iter != m_listChatMessage.end() ; ++Iter )
	{
		if( (*Iter)->GetMessageType() & ulMsgType )
		{
			pMessageInfo	=	 (*Iter)->AddMessage( pSenderName , pMessage , dwCustomColor , ulCharType );

			stChatMsgEntry*	pEntry	=	(*Iter)->PopNewMessage();
			while( pEntry )
			{
				AddUsingMessage( pEntry );
				pEntry	=	(*Iter)->PopNewMessage();
			}

			break;
		}
	}

	RefreshScroll();
	RefreshList();

	return pMessageInfo;
}

BOOL	AgcUIChatMessage::AddTab( CONST string& strTabName , UINT ulMsgType , BOOL bPrimary )
{
	if( m_listChatTab.size() >= MAX_TAB_BUTTON )
	{
		return FALSE;
	}

	AgcChatTab*	pChatTab	=	new AgcChatTab;
	if( !pChatTab )
		return FALSE;

	pChatTab->SetTabName( strTabName );
	pChatTab->SetTabFlag( ulMsgType );
	pChatTab->SetPrimary( bPrimary );

	// Primary 속성이 있는 탭은 무조건 맨 앞쪽으로 온다.
	if( bPrimary )
		m_listChatTab.push_front( pChatTab );
	else
		m_listChatTab.push_back( pChatTab );

	return TRUE;
}

BOOL	AgcUIChatMessage::DelTab( CONST string& strTabName )
{
	// 탭이 하나도 없다면 실패
	if( !m_listChatTab.size() )
		return FALSE;

	ListChatTabIter		Iter	=	m_listChatTab.begin();
	for( ; Iter != m_listChatTab.end() ; ++Iter )
	{
		if( (*Iter)->GetTabName() == strTabName )
		{
			if( m_pChatTab	== (*Iter) )
				m_pChatTab	=	NULL;

			DEF_SAFEDELETE( (*Iter) );
			m_listChatTab.erase( Iter );
			return TRUE;
		}
	}

	return FALSE;
}

INT		AgcUIChatMessage::GetTabCount( VOID )
{
	return (INT)m_listChatTab.size();
}

AgcChatTab*		AgcUIChatMessage::GetTab( INT nIndex )
{
	AgcChatTab*		pChatTab	=	NULL;
	ListChatTabIter	Iter		=	m_listChatTab.begin();

	for( INT i = 0 ; Iter != m_listChatTab.end() ; ++Iter , ++i )
	{
		if( nIndex == i )
		{
			pChatTab	=	(*Iter);
			break;
		}
	}

	return pChatTab;
}

//BOOL	AgcUIChatMessage::AddMessageInstance( AgcChatMessage* pMessage )
//{
//	ListChatMessageIter	Iter	=	find( m_listChatMessage.begin() , m_listChatMessage.end() , pMessage );
//	if( Iter != m_listChatMessage.end() )
//		return FALSE;
//
//	m_listChatMessage.push_back( pMessage );
//
//	return TRUE;
//}


BOOL		AgcUIChatMessage::IsUseMessageType( AgcChatMessage* pChatMessage  )
{
	ListChatMessageIter	Iter	=	m_listChatMessage.begin();
	for( ; Iter != m_listChatMessage.end() ; ++Iter )
	{
		if( (*Iter) == pChatMessage )
			return TRUE;
	}

	return FALSE;
}

BOOL		AgcUIChatMessage::IsUsing( VOID )
{
	INT		nCount	=	(INT)m_listChatTab.size();

	return nCount ? TRUE : FALSE;
}

VOID		AgcUIChatMessage::SetFontSize( INT nFontType , BOOL bRefresh )
{
	AllDeleteListItem();

	AgcmFont*	pcmFont	=	static_cast< AgcmFont* >( g_pEngine->GetModule( "AgcmFont" ) );

	ListChatMessageIter	Iter	=	m_listChatMessage.begin();
	for( ; Iter != m_listChatMessage.end() ; ++Iter )
	{
		(*Iter)->SetFontType( nFontType , bRefresh );
	}

	m_nFontType	=	nFontType;

	if( pcmFont )
	{
		m_nFontHeight	= ( INT32 )( pcmFont->m_astFonts[ m_nFontType ].lSize ) + 6;
	}

	if( bRefresh )
		Refresh( TRUE );
}

VOID		AgcUIChatMessage::SetTabInput( BOOL bEnable )
{
	for( INT i = 0 ; i < MAX_TAB_BUTTON ; ++i )
	{
		m_UITabButton[i].m_Property.bUseInput	=	bEnable;
	}

	m_bTabInput	=	bEnable;
}

VOID		AgcUIChatMessage::MoveChatWindow( INT nX , INT nY , INT nW , INT nH )
{
	if( m_pUI )
	{
		RwV2d&	vSize	=	GetUISize();

		if( nW == -1 )
			nW	=	(int)vSize.x;

		if( nH == -1 )
			nH	=	(int)vSize.y;

		AgcmUIManager2*	pcmUIManager	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );

		// 화면에 넘어가면 넘어가지 않게 이동시킨다
		if( (nX + nW) >= pcmUIManager->m_lWindowWidth )
			nX	=	pcmUIManager->m_lWindowWidth - nW;

		else if( nX < 0 )
			nX	=	0;

		if( (nY + nH) >= pcmUIManager->m_lWindowHeight )
			nY	=	pcmUIManager->m_lWindowHeight - nH;

		else if( nY < 0 )
			nY	=	0;

		m_pUI->m_pcsUIWindow->MoveWindow( nX , nY , nW  ,nH );
	}
}

RwV2d		AgcUIChatMessage::GetUIPos( VOID )
{
	RwV2d	vPos	=	{ 0.0f , 0.0f };

	if( m_pUI )
	{
		vPos.x	=	m_pUI->m_pcsUIWindow->x;
		vPos.y	=	m_pUI->m_pcsUIWindow->y;
	}
	
	return vPos;
}

RwV2d		AgcUIChatMessage::GetUISize( VOID )
{
	RwV2d	vPos	=	{ 0.0f , 0.0f };

	if( m_pUI )
	{
		vPos.x	=	m_pUI->m_pcsUIWindow->w;
		vPos.y	=	m_pUI->m_pcsUIWindow->h;
	}

	return vPos;
}

AgcChatMessage*	AgcUIChatMessage::GetMessageInstance( INT nIndex )
{
	ListChatMessageIter	Iter	=	m_listChatMessage.begin();
	for( INT i = 0 ; Iter != m_listChatMessage.end() ; ++Iter , ++i )
	{
		if( i == nIndex )
		{
			return (*Iter);
		}
	}

	return NULL;
}

INT		AgcUIChatMessage::GetMessageInstanceCount( VOID )
{
	INT		nCount	=	(INT)m_listChatMessage.size();

	return nCount;
}

BOOL		AgcUIChatMessage::DestroyMessageInstance( INT nIndex )
{
	ListChatMessageIter	Iter	=	m_listChatMessage.begin();
	for( INT i = 0 ; Iter != m_listChatMessage.end() ; ++Iter , ++i )
	{
		if( i == nIndex )
		{
			m_listChatMessage.erase( Iter );
			return TRUE;
		}
	}

	return FALSE;
}

BOOL		AgcUIChatMessage::DestroyMessageInstance( AgcChatMessage* pMessage )
{
	ListChatMessageIter	Iter	=	m_listChatMessage.begin();
	for( ; Iter != m_listChatMessage.end() ; ++Iter )
	{
		if( (*Iter) == pMessage )
		{
			m_listChatMessage.erase( Iter );
			return TRUE;
		}
	}

	return FALSE;
}

VOID		AgcUIChatMessage::AllDestroyMessageInstance( VOID )
{
	m_listChatMessage.clear();
	
	RefreshTabButton();
}

VOID		AgcUIChatMessage::SetListItemMaxCount( INT nMaxCount )
{
	if( !m_pListControl || !m_pListItem )
		return;

	AcUIList*		pUIList	=	static_cast< AcUIList* >(m_pListControl->m_pcsBase);
	AcUIListItem*	pUIItem	=	static_cast< AcUIListItem* >(m_pListItem->m_pcsBase);

	RwV2d	vSize	=	GetUISize();

	vSize.x	=	vSize.x	- SCROLL_SIZE_X;
	vSize.y	=	vSize.y	- m_UITabButton[0].m_fTextureHeight - m_UISetupButton.m_fTextureHeight;

	// UI 크기 조절 및 위치 세팅을 한다.
	pUIList->MoveWindow( SCROLL_SIZE_X , (INT32)m_UITabButton[0].m_fTextureHeight , (INT32)vSize.x , (INT32)vSize.y );

	if( !nMaxCount )
	{
		nMaxCount	=	(INT)pUIList->h / m_nFontHeight;
	}

	//if( m_nMaxListCount == nMaxCount )
	//	return;

	AllDeleteListItem();

	pUIItem->MoveWindow( 0 , 0 , m_pListControl->m_pcsBase->w , m_nFontHeight );
	pUIList->SetListItemWindowMoveInfo( pUIList->w , m_nFontHeight , 0 , 0 );

	pUIList->SetListItemWindowTotalNum( nMaxCount );
	pUIList->SetListItemColumn( 1 );
	pUIList->SetListItemWindowVisibleRow( nMaxCount );

	m_nMaxListCount	=	nMaxCount;
}

VOID		AgcUIChatMessage::ForceDetailShow( VOID )
{
	ShowTabButton();
	AttachResize();
	AttachScroll();
	AttachMove();
	AttachSetup();
}

VOID		AgcUIChatMessage::ForceDetailHide( VOID )
{
	HideTabButton();
	DetachResize();
	DetachScroll();
	DetachMove();
	DetachSetup();
}

VOID		AgcUIChatMessage::AttachScroll( VOID )
{
	m_UIScroll.m_Property.bVisible	=	TRUE;
}

VOID		AgcUIChatMessage::DetachScroll( VOID )
{
	m_UIScroll.m_Property.bVisible	=	FALSE;
}

VOID		AgcUIChatMessage::AttachResize( VOID )
{
	m_UISizeButton.m_Property.bVisible	=	TRUE;
}

VOID		AgcUIChatMessage::DetachResize( VOID )
{
	m_UISizeButton.m_Property.bVisible	=	FALSE;
}

VOID		AgcUIChatMessage::AttachMove( VOID )
{
	m_UIMoveButton.m_Property.bVisible	=	TRUE;
}

VOID		AgcUIChatMessage::DetachMove( VOID )
{
	m_UIMoveButton.m_Property.bVisible	=	FALSE;
}

VOID		AgcUIChatMessage::AttachSetup( VOID )
{
	m_UISetupButton.m_Property.bVisible	=	TRUE;
}

VOID		AgcUIChatMessage::DetachSetup( VOID )
{
	m_UISetupButton.m_Property.bVisible	=	FALSE;
}

VOID		AgcUIChatMessage::ShowTabButton( VOID )
{
	for( INT i = 0 ; i < MAX_TAB_BUTTON ; ++i )
	{
		m_UITabButton[i].m_Property.bVisible	=	TRUE;
	}
}

VOID		AgcUIChatMessage::HideTabButton( VOID )
{
	for( INT i = 0 ; i < MAX_TAB_BUTTON ; ++i )
	{
		m_UITabButton[i].m_Property.bVisible	=	FALSE;
	}
}

VOID		AgcUIChatMessage::AllDeleteListItem( VOID )
{
	// 붙어 있는 메세지를 다 지운다
	if( m_pListControl )
	{
		AcUIList*	pUIList	=	static_cast< AcUIList* >(m_pListControl->m_pcsBase);
		if( pUIList )
		{
			for( INT i = 0 ; i < m_nMaxListCount ; ++i )
			{
				AcUIListItem*	pUIListItem	=	pUIList->GetListItemPointer_Index( i );
				if( pUIListItem )
				{
					pUIListItem->AllDeleteChild( FALSE , FALSE );
					pUIListItem->UpdateChildWindow();
				}
			}
		}
	}
}

VOID		AgcUIChatMessage::RefreshTabButton( VOID )
{
	if( !m_pUI )
		return;

	// 우선 붙어 있는 버튼을 다 제거후 비활성화 버튼 색깔로 바꿔준다.
	for( INT k = 0 ; k < MAX_TAB_BUTTON ; ++k )
	{
		m_UITabButton[k].m_Property.bVisible	=	FALSE;
		m_UITabButton[k].m_lColor				=	0xff888888;
	}

	INT		nOffset	=	(int)m_UISetupButton.m_fTextureWidth;

	ListChatTabIter	TabIter	=	m_listChatTab.begin();
	INT				i		=	0;

	for( i = 0 ; TabIter != m_listChatTab.end() ; ++TabIter , ++i )
	{
		// 선택된 탭이 없다면 첫번째가 무조건 선택되게 한다.
		if( !m_pChatTab )
		{
			m_pChatTab	=	(*TabIter);
		}

		m_UITabButton[i].m_Property.bVisible	=	TRUE;

		string		strTabName	=	(*TabIter)->GetTabName();
		DWORD		dwColor		=	0xff707070;

		m_UITabButton[i].SetCustomRender( TRUE );
		m_UITabButton[i].SetCustomLineColor( 0xffffffff );
		m_UITabButton[i].SetCustomBGColor( 0x30ffffff );

		m_UITabButton[i].ClearMenuEvent();

		m_UITabButton[i].SetRButtonClickMenu( FALSE );

		// 선택된 탭이라면 글자색을 바꿔준다.
		if( (*TabIter) == m_pChatTab )
		{
			dwColor	=	0xffffffff;
		}

		if( !(*TabIter)->IsPrimary() )
		{
			m_UITabButton[i].SetRButtonClickMenu( TRUE );

			CHAR	szMenuName[ MAX_PATH ];
			sprintf_s( szMenuName , MAX_PATH , "%s" , ClientStr().GetStr(STI_TAB_BUTTON_MENU_DELETE) );
			m_UITabButton[i].AddMenuEvent( m_pManagerClass , szMenuName , (*TabIter) , this , AgcChatManager::CBTabDeleteEvent );

			sprintf_s( szMenuName , MAX_PATH , "%s" , ClientStr().GetStr(STI_TAB_BUTTON_MENU_EDIT) );
			m_UITabButton[i].AddMenuEvent( m_pManagerClass , szMenuName , (*TabIter) , this , AgcChatManager::CBTabEditEvent );

			
		}

		m_UITabButton[i].SetCallbackClickEvent( this , CBTabClickEvent );
		m_UITabButton[i].SetData( (*TabIter) , NULL );

		// Drag 이벤트를 붙인다
		m_UITabButton[i].SetCallbackDragEvent( m_pManagerClass , AgcChatManager::CBDragCallback );
		m_UITabButton[i].SetDragData( (*TabIter) , this );

		// 이름을 바꾼다
		m_UITabButton[i].SetStaticStringExt(  (CHAR*)strTabName.c_str() , 1.0f , 0 , dwColor , true , ACUIBASE_HALIGN_CENTER , ACUIBASE_VALIGN_CENTER );

		// 해당 위치로 옮기고 크기를 변경한다
		m_UITabButton[i].MoveWindow( (INT32)(nOffset +(i*m_UITabButton[0].m_fTextureWidth)) , 0 , (INT32)m_UITabButton[0].m_fTextureWidth , (INT32)m_UITabButton[0].m_fTextureHeight );

		m_UITabButton[i].SetDockingCallback( m_pManagerClass , this , NULL , AgcChatManager::CBDockingCallback );
	}

	// UI 크기를 자동으로 늘려야 된다.. 탭 버튼이 많아지면
	RwV2d&	vSize	=	GetUISize();

	if( (i*m_UITabButton[0].m_fTextureWidth)+nOffset > vSize.x )
	{
		MoveChatWindow(	m_pUI->m_pcsUIWindow->x	,
						m_pUI->m_pcsUIWindow->y ,
						(INT16)((i*m_UITabButton[0].m_fTextureWidth)+nOffset)	,
						m_pUI->m_pcsUIWindow->h	);


		SetListItemMaxCount();
		

	}
}

VOID		AgcUIChatMessage::RefreshList( BOOL bRefreshMessage )
{
	if( !m_pUI || !m_pListControl || !m_pListItem )
		return;

	AcUIList*		pUIList	=	static_cast< AcUIList* >(m_pListControl->m_pcsBase);
	AcUIListItem*	pUIItem	=	static_cast< AcUIListItem* >(m_pListItem->m_pcsBase);

	RwV2d	vSize	=	GetUISize();

	vSize.x	-=	SCROLL_SIZE_X;
	vSize.y	-=	m_UITabButton[0].m_fTextureHeight-m_UISetupButton.m_fTextureHeight;

	// UI 크기 조절 및 위치 세팅을 한다.
	pUIList->MoveWindow( SCROLL_SIZE_X , (INT32)m_UITabButton[0].m_fTextureHeight , (INT32)vSize.x , (INT32)vSize.y );

	AllDeleteListItem();
	_RefreshMessage( bRefreshMessage );

	for( INT i = 0 ; i < m_nMaxListCount ; ++i )
	{
		AcUIListItem*	pListItem	=	pUIList->GetListItemPointer_Index( i );
		if( !pListItem )
			continue;

		INT		nOffset	=	0;
		INT		nIndex	=	0;
		INT		nTemp	=	(INT)(m_dequeUsingMessage.size()-m_nMaxListCount);

		if( nTemp > 0 )
		{
			nOffset	=	(INT)( m_UIScroll.GetScrollValue() * nTemp );
			nIndex	=	nOffset + i;
		}

		else
		{
			nOffset	=	nTemp;
			INT		nResult	=	nOffset + i;
			if( nResult < 0 )
				continue;

			nIndex	=	nResult;
		}

		stChatMsgEntry*	pEntry	=	_GetUsingMessage( nIndex );
		if( pEntry )
		{
			ListUIControlIter	Iter	=	pEntry->m_listText.begin();
			for( ; Iter != pEntry->m_listText.end() ; ++Iter )
			{
				pListItem->AddChild( (*Iter)->m_pcsUIBase );
			}
		}

		pListItem->UpdateChildWindow();
	}
}

VOID		AgcUIChatMessage::RefreshResize( VOID )
{
	if( !m_pUI )
		return;

	RwV2d&	vSize	=	GetUISize();

	m_UISizeButton.MoveWindow(	(INT32)(vSize.x - m_UISizeButton.m_fTextureWidth ),
								(INT32)(vSize.y - m_UISizeButton.m_fTextureHeight) ,
								(INT32)m_UISizeButton.m_fTextureWidth	,
								(INT32)m_UISizeButton.m_fTextureHeight			);

}

VOID		AgcUIChatMessage::RefreshSetup( VOID )
{
	if( !m_pUI )
		return;

	RwV2d&	vSize	=	GetUISize();

	m_UISetupButton.MoveWindow( 0	,
								(INT32)(vSize.y	- m_UISetupButton.m_fTextureHeight) ,
								(INT32)m_UISetupButton.m_fTextureWidth	,
								(INT32)m_UISetupButton.m_fTextureHeight				);
}

VOID		AgcUIChatMessage::RefreshScroll( VOID )
{
	if( !m_pUI )
		return;

	INT		nCount	=	m_dequeUsingMessage.size();
	INT		nSize	=	nCount	-	m_nMaxListCount;
	if( nSize < 0 )
		nSize	=	0;

	m_UIScroll.m_fScrollUnit	=	1.0f / nSize;

	FLOAT	fScrollValue	=	m_UIScroll.GetScrollValue();

	RwV2d&	vSize	=	GetUISize();
	vSize.y	= vSize.y - m_UITabButton[0].m_fTextureHeight - m_UISetupButton.m_fTextureHeight;

	m_UIScroll.MoveWindow( 0 , (INT32)m_UITabButton[0].m_fTextureHeight , SCROLL_SIZE_X , (INT32)vSize.y );
	m_UIButtonUP.MoveWindow( 0 , 0 , (INT32)m_UIButtonUP.m_fTextureWidth , (INT32)m_UIButtonUP.m_fTextureHeight );
	m_UIButtonDown.MoveWindow( 0 , (INT32)(m_UIScroll.h - m_UIButtonDown.m_fTextureHeight) , (INT32)m_UIButtonDown.m_fTextureWidth , (INT32)m_UIButtonDown.m_fTextureHeight );
	m_UIScroll.SetScrollButtonInfo( TRUE , (INT32)m_UIButtonUP.m_fTextureHeight , (INT32)(m_UIScroll.h - m_UIButtonDown.m_fTextureHeight - m_UIScrollButton.m_fTextureHeight) , (1.0f / nSize) );
	m_UIScroll.SetScrollValue( fScrollValue );
}

VOID		AgcUIChatMessage::_InitUIList( VOID )
{
	// List 위치 세팅 및 크기 조절
	// 텍스쳐 세팅

	RefreshList();
}

VOID		AgcUIChatMessage::_InitUIScroll( VOID )
{
	// Scroll 위치 세팅 및 크기 조절
	// 텍스쳐 세팅

	RefreshScroll();
}

VOID		AgcUIChatMessage::_InitUITab( VOID )
{
	// Tab 위치 세팅 및 크기 조절
	// 텍스쳐 세팅

	for( INT i = 0 ; i < MAX_TAB_BUTTON ; ++i )
	{
		m_UITabButton[i].SetCustomRender( TRUE );
	}

	RefreshTabButton();
}

VOID		AgcUIChatMessage::_InitMenuEvent( VOID )
{

	AcUIMenu*	pFontMenu	=	m_UIMenu.GetChildMenu( "FontSizeMenu" );
	AcUIMenu*	pTabMenu	=	m_UIMenu.GetChildMenu( "TabButtonMenu" );

	stMenuEvent		MenuEvent;

	m_UIMenu.ClearEvent();
	pFontMenu->ClearEvent();
	pTabMenu->ClearEvent();

	pFontMenu->SetDisableFontColor(ACTIVE_BUTTON_TEXT_COLOR);
	pTabMenu->SetDisableFontColor(ACTIVE_BUTTON_TEXT_COLOR);


	CHAR szNameOfMenu[nSizeOfName];
	
	// Font Child menu 등록
	ZeroMemory(szNameOfMenu, nSizeOfName);
	sprintf_s(szNameOfMenu, nSizeOfName, "%s", ClientStr().GetStr( STI_TAB_OPTION_NAME_FONTSIZE ));
	MenuEvent.m_strMenuName	=	szNameOfMenu;
	MenuEvent.m_eEventType	=	E_MENU_EVENT_SUB_MENU;
	MenuEvent.m_pvData		=	pFontMenu;
	m_UIMenu.AddMenuEvent( MenuEvent );

	// Tab Child Menu 등록
	ZeroMemory(szNameOfMenu, nSizeOfName);
	sprintf_s(szNameOfMenu, nSizeOfName, "%s", ClientStr().GetStr( STI_TAB_OPTION_NAME_TAB_MENU ));
	MenuEvent.m_strMenuName	=	szNameOfMenu;
	MenuEvent.m_pvData		=	pTabMenu;
	m_UIMenu.AddMenuEvent( MenuEvent );

	ZeroMemory(szNameOfMenu, nSizeOfName);
	sprintf_s(szNameOfMenu, nSizeOfName, "%s", ClientStr().GetStr( STI_TAB_OPTION_NAME_ADD_TAB ));
	MenuEvent.m_strMenuName	=	szNameOfMenu;
	MenuEvent.m_eEventType	=	E_MENU_EVENT_FUNCTION;
	MenuEvent.m_bEnable		=	TRUE;
	MenuEvent.m_pClass		=	m_pManagerClass;
	MenuEvent.m_pvData		=	AgcChatManager::CBAddTab;
	MenuEvent.m_pData1		=	this;
	m_UIMenu.AddMenuEvent( MenuEvent );

	MenuEvent.m_eEventType	=	E_MENU_EVENT_FUNCTION;
	MenuEvent.m_pvData		=	CBMenuEventFontSize;
	MenuEvent.m_pClass		=	this;
	
#ifndef _AREA_GLOBAL_
	MenuEvent.m_strMenuName	=	"11";
	MenuEvent.m_pData1		=	(PVOID)FONT_SIZE_11;
	MenuEvent.m_bEnable		=	FONT_SIZE_11 == GetFontSize() ? FALSE : TRUE;
	pFontMenu->AddMenuEvent( MenuEvent );
#endif

	MenuEvent.m_strMenuName	=	"12";
	MenuEvent.m_pData1		=	(PVOID)FONT_SIZE_12;
	MenuEvent.m_bEnable		=	FONT_SIZE_12 == GetFontSize() ? FALSE : TRUE;
	pFontMenu->AddMenuEvent( MenuEvent );

	MenuEvent.m_strMenuName	=	"13";
	MenuEvent.m_pData1		=	(PVOID)FONT_SIZE_13;
	MenuEvent.m_bEnable		=	FONT_SIZE_13 == GetFontSize() ? FALSE : TRUE;
	pFontMenu->AddMenuEvent( MenuEvent );

	MenuEvent.m_strMenuName	=	"14";
	MenuEvent.m_pData1		=	(PVOID)FONT_SIZE_14;
	MenuEvent.m_bEnable		=	FONT_SIZE_14 == GetFontSize() ? FALSE : TRUE;
	pFontMenu->AddMenuEvent( MenuEvent );

	MenuEvent.m_strMenuName	=	"15";
	MenuEvent.m_pData1		=	(PVOID)FONT_SIZE_15;
	MenuEvent.m_bEnable		=	FONT_SIZE_15 == GetFontSize() ? FALSE : TRUE;
	pFontMenu->AddMenuEvent( MenuEvent );

	// Tab Menu 등록
	MenuEvent.m_strMenuName	=	"ON";
	MenuEvent.m_eEventType	=	E_MENU_EVENT_FUNCTION;
	MenuEvent.m_pData1		=	(PVOID)1;
	MenuEvent.m_pvData		=	CBMenuEventTabEnable;
	MenuEvent.m_pClass		=	this;
	MenuEvent.m_bEnable		=	!GetTabInput();
	pTabMenu->AddMenuEvent( MenuEvent );

	MenuEvent.m_strMenuName	=	"OFF";
	MenuEvent.m_pData1		=	(PVOID)0;
	MenuEvent.m_bEnable		=	GetTabInput();
	pTabMenu->AddMenuEvent( MenuEvent );
}


VOID		AgcUIChatMessage::_RefreshMessage( BOOL bRefresh , INT nTextLength )
{

	m_dequeUsingMessage.clear();

	if( !nTextLength )
	{
		nTextLength	=	m_pListControl->m_pcsBase->w;
	}

	ListChatMessageIter	Iter	=	m_listChatMessage.begin();
	for( ; Iter != m_listChatMessage.end() ; ++Iter )
	{
		if( !(*Iter)->IsEnable() )
			continue;

		// 최대 길이 설정
		(*Iter)->SetTextLength( nTextLength );

		if( bRefresh )
			(*Iter)->RefreshMessageEntry();

		INT		nCount	=	(*Iter)->GetMessageCount();
		for( INT i = 0 ; i < nCount ; ++i )
		{
			stChatMsgEntry*	pEntry	=	(*Iter)->GetMessage( i );
			if( pEntry )
			{
				m_dequeUsingMessage.push_back( stChatUsingMsgEntry(pEntry) );
			}
		}
	}

	if( m_dequeUsingMessage.size() > 0 )
		stable_sort( m_dequeUsingMessage.begin() , m_dequeUsingMessage.end() );
}

stChatMsgEntry*	AgcUIChatMessage::_GetUsingMessage( INT nIndex )
{
	stChatMsgEntry*					pEntry	=	NULL;

	if( (INT)m_dequeUsingMessage.size() <= nIndex )
		return NULL;

	return m_dequeUsingMessage[nIndex].m_pEntry;
}


BOOL	AgcUIChatMessage::CBTabClickEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcUIChatMessage*	pThis			=	static_cast< AgcUIChatMessage* >(pClass);
	AgcChatTab*			pChatTab		=	static_cast< AgcChatTab* >(pData1);
	if( !pThis || !pChatTab )
		return FALSE;

	pThis->SelectTab( pChatTab );
	pThis->Refresh( TRUE );
	
	return TRUE;
}



BOOL	AgcUIChatMessage::CBMenuEventDocking( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu )
{	
	AgcUIChatMessage*	pThis			=	static_cast< AgcUIChatMessage* >(pClass);
	AgcChatMessage*		pChatMessage	=	static_cast< AgcChatMessage* >(pClass);
	if( !pThis || !pChatMessage )
		return FALSE;

	pThis->SetSeparationTab( pChatMessage );
	
	return TRUE;
}

BOOL	AgcUIChatMessage::CBMenuEventFontSize( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu )
{
	AgcUIChatMessage*	pThis			=	static_cast< AgcUIChatMessage* >(pClass);
	if( !pThis )
		return FALSE;

	INT					nFontType		=	(INT)pData1;

	pThis->SetFontSize( nFontType );

	return TRUE;
}

BOOL	AgcUIChatMessage::CBMenuEventTabEnable( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu )
{
	AgcUIChatMessage*	pThis			=	static_cast< AgcUIChatMessage* >(pClass);
	if( !pThis )
		return FALSE;

	BOOL				bEnable			=	(BOOL)pData1;

	pThis->SetTabInput( bEnable );

	return TRUE;
}

VOID	AgcUIChatMessage::SetLastButton( INT nIndex )
{
	if( nIndex < MAX_TAB_BUTTON )
	{
		m_UITabButton[ nIndex ].SetLastClickButton();
		m_UITabButton[ nIndex ].SetModal();
	}
}


BOOL	AgcUIChatMessage::AddUsingMessage( stChatMsgEntry* pEntry )
{
	if( !pEntry )
		return FALSE;

	m_dequeUsingMessage.push_back( stChatUsingMsgEntry(pEntry) );

	stable_sort( m_dequeUsingMessage.begin() , m_dequeUsingMessage.end() );

	return TRUE;
}

VOID	AgcUIChatMessage::SetScrollValue( FLOAT fValue )
{
	m_UIScroll.SetScrollValue( 1.0f );
}

FLOAT	AgcUIChatMessage::GetScrollValue( VOID )
{
	return m_UIScroll.GetScrollValue();
}	


BOOL	AgcUIChatMessage::IsUsableType( UINT ulMsgType )
{
	// 선택된 탭이 없으면 실패
	if( !m_pChatTab )
		return FALSE;

	return m_pChatTab->IsUsableType( ulMsgType );
}

VOID	AgcUIChatMessage::InitMessageInstance( PVOID pvManager )
{
	ListChatMessageIter		Iter	=	m_listChatMessage.begin();
	for( ; Iter != m_listChatMessage.end() ; ++Iter )
	{
		DEF_SAFEDELETE( (*Iter) );
	}

	m_listChatMessage.clear();


	for( INT i = 0 ; i < CHAT_MESSAGE_COUNT ; ++i )
	{
		AgcChatMessage*	pChatMessage	=	new AgcChatMessage;
		ASSERT( pChatMessage );

		pChatMessage->SetMessageType( 1<<i );
		pChatMessage->SetEnable( TRUE );
		pChatMessage->SetTextLine( MESSAGE_COUNT_MAX );
		pChatMessage->SetTextLength( MAX_LISTBOX_STRING );

		pChatMessage->SetManager( pvManager );

		m_listChatMessage.push_back( pChatMessage );
	}


	// 메세지 포맷을 설정한다.
	AgcmUIManager2*			pcmUIManager		=	static_cast< AgcmUIManager2* >(g_pEngine->GetModule( "AgcmUIManager2" ) );
	AgcmResourceLoader*		pcmResourceLoader	=	static_cast< AgcmResourceLoader* >(g_pEngine->GetModule( "AgcmResourceLoader" ));

	ASSERT( pcmUIManager && pcmResourceLoader );

	GetChatMessage( CHAT_MESSAGE_NORMAL		)->SetTextFormat( CHAT_MESSAGE_NORMAL	, "[%s] %s"		, NULL , FALSE , TRUE , pcmResourceLoader->GetColor("NORMAL_CHAT") );

	GetChatMessage( CHAT_MESSAGE_GUILD		)->SetTextFormat( CHAT_MESSAGE_GUILD	, "%s[%s] %s"	, pcmUIManager->GetUIMessage("Chat_Guild_Header" )	, TRUE , TRUE , pcmResourceLoader->GetColor("GUILD_CHAT") );
	GetChatMessage( CHAT_MESSAGE_PARTY		)->SetTextFormat( CHAT_MESSAGE_PARTY	, "%s[%s] %s"	, pcmUIManager->GetUIMessage("Chat_Party_Header" )	, TRUE , TRUE , pcmResourceLoader->GetColor("PARTY_CHAT") );
	GetChatMessage( CHAT_MESSAGE_WHISPER	)->SetTextFormat( CHAT_MESSAGE_WHISPER	, "%s[%s] %s"	, pcmUIManager->GetUIMessage("Chat_Whisper_Header")	, TRUE , TRUE , pcmResourceLoader->GetColor("WHISPER_RECV") );
	GetChatMessage( CHAT_MESSAGE_SYSTEM		)->SetTextFormat( CHAT_MESSAGE_SYSTEM	, "%s %s"		, pcmUIManager->GetUIMessage("Chat_System_Header")	, TRUE , FALSE , pcmResourceLoader->GetColor("SYSTEM_CHAT2") );

// 	GetChatMessage( CHAT_MESSAGE_SYSTEM2 )->SetTextFormat( CHAT_MESSAGE_SYSTEM2 , "%s %s" , pcmUIManager->GetUIMessage("Chat_System_Header") , TRUE , FALSE , pcmResourceLoader->GetColor("SYSTEM_CHAT1") );
// 	GetChatMessage( CHAT_MESSAGE_SYSTEM3 )->SetTextFormat( CHAT_MESSAGE_SYSTEM3 , "%s %s" , pcmUIManager->GetUIMessage("Chat_System_Header") , TRUE , FALSE , pcmResourceLoader->GetColor("SYSTEM_CHAT1") );

	GetChatMessage( CHAT_MESSAGE_NOTICE		)->SetTextFormat( CHAT_MESSAGE_NOTICE	, "%s %s"		, pcmUIManager->GetUIMessage("Chat_Notice_Header")	, TRUE , FALSE , pcmResourceLoader->GetColor("NOTICE_CHAT1") );
	GetChatMessage( CHAT_MESSAGE_SIEGE		)->SetTextFormat( CHAT_MESSAGE_SIEGE	, "%s %s"		, pcmUIManager->GetUIMessage("Chat_Shrine_Header")	, TRUE , FALSE , pcmResourceLoader->GetColor("SHRINE_CHAT") );
	GetChatMessage( CHAT_MESSAGE_QUEST		)->SetTextFormat( CHAT_MESSAGE_QUEST	, "%s %s"		, pcmUIManager->GetUIMessage("Chat_Quest_Header")	, TRUE , FALSE , pcmResourceLoader->GetColor("QUEST_CHAT") );

// 	GetChatMessage( CHAT_MESSAGE_DAMAGEFROM )->SetTextFormat( CHAT_MESSAGE_DAMAGEFROM , "%s %s" , pcmUIManager->GetUIMessage("Chat_DamageFrom_Header") , TRUE , FALSE , pcmResourceLoader->GetColor("DAMAGEFROM_CHAT") );
// 	GetChatMessage( CHAT_MESSAGE_DAMAGETO )->SetTextFormat( CHAT_MESSAGE_DAMAGETO , "%s %s" , pcmUIManager->GetUIMessage("Chat_DamageTo_Header") , TRUE , FALSE , pcmResourceLoader->GetColor("DAMAGETO_CHAT") );
	GetChatMessage( CHAT_MESSAGE_ARCHLORD	)->SetTextFormat( CHAT_MESSAGE_ARCHLORD	, "%s"			, ""												, FALSE, FALSE, pcmResourceLoader->GetColor("Yellow") );
	GetChatMessage( CHAT_MESSAGE_SHOUT		)->SetTextFormat( CHAT_MESSAGE_SHOUT	, "%s[%s] %s"	, pcmUIManager->GetUIMessage( "Chat_Area_Header")	, TRUE , TRUE, pcmResourceLoader->GetColor("WORD_BALLON") );
	GetChatMessage( CHAT_MESSAGE_RACE		)->SetTextFormat( CHAT_MESSAGE_RACE		, "%s"			, ""												, FALSE, FALSE, pcmResourceLoader->GetColor("RACE_TRUMPET") );
	GetChatMessage( CHAT_MESSAGE_ELEMENTAL	)->SetTextFormat( CHAT_MESSAGE_ELEMENTAL, "%s"			, ""												, FALSE, FALSE, pcmResourceLoader->GetColor("Yellow") );
}


AgcChatMessage*	AgcUIChatMessage::GetChatMessage( UINT ulMsgType )
{
	ListChatMessageIter	Iter	=	m_listChatMessage.begin();
	for( ; Iter != m_listChatMessage.end() ; ++Iter )
	{
		if( (*Iter)->GetMessageType() & ulMsgType )
		{
			return (*Iter);
		}
	}

	return NULL;
}


BOOL	AgcUIChatMessage::SelectTab( AgcChatTab*	pChatTab )
{
	if( !pChatTab )
		return FALSE;

	// 없으면 이상한 탭이 들어온거다.
	ListChatTabIter	Iter	=	find( m_listChatTab.begin() , m_listChatTab.end() , pChatTab );
	if( Iter == m_listChatTab.end() )
		return FALSE;

	m_pChatTab	=	pChatTab;

	return TRUE;
}


