#include "AcUIEventButton.h"
#include "AgcmUIManager2.h"


#define		DEFAULT_LINE_SIZE	2

AcUIEventButton*	AcUIEventButton::m_pLastClickDown	=	NULL;

AcUIEventButton::AcUIEventButton( VOID )
{
	m_pData1	=	NULL;
	m_pData2	=	NULL;
	m_fnClickEvent	=	NULL;
	m_bCustomRender	=	FALSE;
	m_bMenu			=	FALSE;

	m_fnDragEvent	=	NULL;
	m_pDragClass	=	NULL;
	m_pDragData1	=	NULL;
	m_pDragData2	=	NULL;
	m_bDragStart	=	FALSE;

	m_fnFocusEvent	=	NULL;
	m_pFocusClass	=	NULL;
	m_pFocusData1	=	NULL;
	m_pFocusData2	=	NULL;

	m_fnKillFocusEvent	=	NULL;
	m_pKillFocusClass	=	NULL;
	m_pKillFocusData1	=	NULL;
	m_pKillFocusData2	=	NULL;

	m_pvTempData		=	NULL;
	m_pvTempData2		=	NULL;

	m_dwLineColor		=	0xeeffffff;
	m_dwBackgroundColor	=	0x40ffffff;
	m_pUIMenu			=	NULL;
}

AcUIEventButton::~AcUIEventButton( VOID )
{
	DestroyButtonMenu();
}

VOID	AcUIEventButton::SetCallbackClickEvent( PVOID pClass , fnEventCallback EventCallback )
{
	m_pClass		=	pClass;
	m_fnClickEvent	=	EventCallback;
}

VOID	AcUIEventButton::SetCallbackDragEvent( PVOID pClass , fnEventCallback EventCallback )
{
	m_pDragClass	=	pClass;
	m_fnDragEvent	=	EventCallback;
}

VOID	AcUIEventButton::SetCallbackSetFocusEvent( PVOID pClass , fnEventCallback EventCallback )
{
	m_pFocusClass	=	pClass;
	m_fnFocusEvent	=	EventCallback;
}

VOID	AcUIEventButton::SetCallbackKillFocusEvent( PVOID pClass , fnEventCallback EventCallback )
{
	m_pKillFocusClass	=	pClass;
	m_fnKillFocusEvent	=	EventCallback;
}

BOOL	AcUIEventButton::OnLButtonDown( RsMouseStatus *ms )
{
	// 이벤트가 없을경우 걍 넘긴다
	if( !m_fnClickEvent && !m_fnDragEvent )
		return FALSE;

	AcUIButton::OnLButtonDown( ms );

	m_pLastClickDown	=	this;

	m_vStartDrag.x	=	ms->pos.x;
	m_vStartDrag.y	=	ms->pos.y;

	m_bDragStart	=	TRUE;

	SetModal();

	return TRUE;
}

BOOL	AcUIEventButton::OnLButtonUp( RsMouseStatus *ms )
{
	// 이벤트가 없을경우 걍 넘긴다
	if( !m_fnClickEvent && !m_fnDragEvent )
		return FALSE;

	AcUIButton::OnLButtonUp( ms );

	if( m_fnClickEvent && m_pLastClickDown == this )
	{
		m_fnClickEvent( m_pClass , m_pData1 , m_pData2 , this );
	}

	if( m_pLastClickDown )
	{
		m_pLastClickDown->ReleaseModal();
		m_pLastClickDown->m_bDragStart	=	FALSE;
	}

	m_pLastClickDown	=	NULL;
	m_bDragStart		=	FALSE;
	ReleaseModal();

	return TRUE;
}

BOOL	AcUIEventButton::OnMouseMove( RsMouseStatus *ms )
{
	// 이벤트가 없을경우 걍 넘긴다
	if( !m_fnClickEvent && !m_fnDragEvent )
		return FALSE;

	AcUIButton::OnMouseMove( ms );

	if( g_pEngine->IsMouseLeftBtnDown() && m_pLastClickDown )
	{
		if( m_pLastClickDown )
		{
			PVOID	pvData[2];

			pvData[0]	=	m_pLastClickDown->m_pDragData1;
			pvData[1]	=	m_pLastClickDown->m_pDragData2;

			if( m_pLastClickDown && m_pLastClickDown->m_fnDragEvent && m_pLastClickDown->m_bDragStart )
				m_pLastClickDown->m_fnDragEvent( m_pLastClickDown->m_pDragClass , &pvData , &m_pLastClickDown->m_vStartDrag , m_pLastClickDown );
		}

		return TRUE;
	}

	else
	{
		m_pLastClickDown	=	NULL;
		m_bDragStart		=	FALSE;
	}

	return FALSE;
}

BOOL	AcUIEventButton::OnRButtonDown( RsMouseStatus *ms )
{
	AcUIButton::OnRButtonDown( ms );

	m_pLastClickDown	=	this;

	return TRUE;
}

BOOL	AcUIEventButton::OnRButtonUp( RsMouseStatus *ms )
{
	AcUIButton::OnRButtonUp( ms );

	INT		nX	=	(int)ms->pos.x;
	INT		nY	=	(int)ms->pos.y;

	ClientToScreen( &nX , &nY );

	if( m_bMenu && m_pLastClickDown	==	this )
	{
		m_pUIMenu->Initialize( E_MENU_DRAW_TEXTURE );
		m_pUIMenu->SetFocusedWindow();
		m_pUIMenu->MoveWindow( nX , nY , 100 , 100 );
		m_pUIMenu->ShowWindow( TRUE );
	}

	m_pLastClickDown	=	NULL;

	return TRUE;
}

VOID	AcUIEventButton::OnWindowRender( VOID )
{
	AcUIButton::OnWindowRender();

	if( m_bCustomRender )
	{
		switch( GetButtonMode() )
		{
		case ACUIBUTTON_MODE_ONMOUSE:
			_DrawCustomOnMouse();
			break;

		case ACUIBUTTON_MODE_CLICK:
			_DrawCustomOnClick();
			break;
		}
	}
}

VOID	AcUIEventButton::_DrawCustomOnMouse( VOID )
{
	INT		nAbsolute_x	=	0;
	INT		nAbsolute_y	=	0;

	ClientToScreen( &nAbsolute_x, &nAbsolute_y );

	// 윤곽 라인을 그린다
	g_pEngine->Draw2DPanel( nAbsolute_x-DEFAULT_LINE_SIZE	, nAbsolute_y-DEFAULT_LINE_SIZE	, w+DEFAULT_LINE_SIZE*2	, DEFAULT_LINE_SIZE		, m_dwLineColor );
	g_pEngine->Draw2DPanel( nAbsolute_x+w					, nAbsolute_y-DEFAULT_LINE_SIZE	, DEFAULT_LINE_SIZE		, h+DEFAULT_LINE_SIZE*2	, m_dwLineColor );
	g_pEngine->Draw2DPanel( nAbsolute_x-DEFAULT_LINE_SIZE	, nAbsolute_y-DEFAULT_LINE_SIZE	, DEFAULT_LINE_SIZE		, h+DEFAULT_LINE_SIZE*2	, m_dwLineColor );
	g_pEngine->Draw2DPanel( nAbsolute_x-DEFAULT_LINE_SIZE	, nAbsolute_y+h					, w+DEFAULT_LINE_SIZE*2	, DEFAULT_LINE_SIZE		, m_dwLineColor );
}

VOID	AcUIEventButton::_DrawCustomOnClick( VOID )
{
	INT		nAbsolute_x	=	0;
	INT		nAbsolute_y	=	0;

	ClientToScreen( &nAbsolute_x, &nAbsolute_y );

	g_pEngine->Draw2DPanel( nAbsolute_x , nAbsolute_y , w , h , m_dwBackgroundColor );	//	백판.

	_DrawCustomOnMouse();
}

VOID	AcUIEventButton::AddMenuEvent( PVOID pClass , CONST string& strName , PVOID pData1 , PVOID pData2 , fnMenuEvent pfnEvent )
{
	if( !m_pUIMenu )
		return;

	stMenuEvent	MenuEvent;

	MenuEvent.m_bEnable		=	TRUE;
	MenuEvent.m_strMenuName	=	strName;
	MenuEvent.m_eEventType	=	E_MENU_EVENT_FUNCTION;
	MenuEvent.m_pClass		=	pClass;
	MenuEvent.m_pvData		=	pfnEvent;
	MenuEvent.m_pData1		=	pData1;
	MenuEvent.m_pData2		=	pData2;

	m_pUIMenu->AddMenuEvent( MenuEvent );
}

VOID	AcUIEventButton::EditMenuEventName( CONST string& strMenuName , CONST string& strChangeName )
{
	if( !strMenuName.length() || !strChangeName.length() )
		return;

	stMenuEvent*	pMenuEvent	=	m_pUIMenu->GetMenuEvent( strMenuName );
	if( !pMenuEvent )
		return;

	pMenuEvent->m_strMenuName	=	strChangeName;
}

VOID	AcUIEventButton::ClearMenuEvent( VOID )
{
	if( !m_pUIMenu )
		return;

	m_pUIMenu->ClearEvent();
}

VOID	AcUIEventButton::DestroyButtonMenu( VOID )
{
	AgcmUIManager2*	pcmUIManager2	=	static_cast< AgcmUIManager2* >(g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !pcmUIManager2 )
		return;

	if( m_pUIMenu )
	{
		pcmUIManager2->RemoveWindow( m_pUIMenu , TRUE );
		pcmUIManager2->RefreshWindow();

		DEF_SAFEDELETE( m_pUIMenu );
	}

}

VOID	AcUIEventButton::SetRButtonClickMenu( BOOL bEnable )
{
	AgcmUIManager2*	pcmUIManager2	=	static_cast< AgcmUIManager2* >(g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !pcmUIManager2 )
		return;

	if( m_bMenu	== bEnable )
		return;

	m_bMenu	=	bEnable;

	if( m_bMenu )
	{
		m_pUIMenu	=	new AcUIMenu;
		pcmUIManager2->AddWindow( m_pUIMenu );

		m_pUIMenu->Initialize( E_MENU_DRAW_TEXTURE );
	}

	else
	{
		pcmUIManager2->RemoveWindow( m_pUIMenu , TRUE );
		DEF_SAFEDELETE( m_pUIMenu );
	}
}


VOID	AcUIEventButton::SetLastClickButton( AcUIEventButton* pButton )
{
	m_vStartDrag		=	m_pLastClickDown->m_vStartDrag;
	m_pLastClickDown->m_bDragStart	=	FALSE;

	m_bDragStart		=	TRUE;

	if( !pButton )
	{
		m_pLastClickDown	=	this;
	}

	else
	{
		m_pLastClickDown	=	pButton;
	}
}

VOID	AcUIEventButton::SetFocus( VOID )
{
}

VOID	AcUIEventButton::KillFocus( VOID )
{
}


VOID	AcUIEventButton::OnSetFocus( VOID )
{
	AcUIButton::OnSetFocus();

	if( m_fnFocusEvent )
		m_fnFocusEvent( m_pFocusClass , m_pFocusData1 , m_pFocusData2 , this );
}

VOID	AcUIEventButton::OnKillFocus( VOID )
{
	AcUIButton::OnKillFocus();

	if( m_fnKillFocusEvent )
		m_fnKillFocusEvent( m_pKillFocusClass , m_pKillFocusData1 , m_pKillFocusData2 , this );
}

