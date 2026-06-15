#include "AcUIMenu.h"


#define		DEFAULT_OFFSET_X	10
#define		DEFAULT_OFFSET_Y	10

#define		DEFAULT_MENU_GAP	2

#define		DEFAULT_BACK_CORNER_X	27
#define		DEFAULT_BACK_CORNER_Y	27

#define		DEFAULT_BACK_BODY_X		18
#define		DEFAULT_BACK_BODY_Y		18

#define		DEFAULT_SELECT			17

RwTexture*	AcUIMenu::m_pBackTexture[ E_MENU_BACK_TEXTURE_COUNT ];
BOOL		AcUIMenu::m_bTextureLoad	=	FALSE;

AcUIMenu::AcUIMenu( VOID )
{
	VisibleMenuName( FALSE );
	m_dwBackgroundColor		=	0x80ff0000;
	m_dwSelectColor			=	0xee0000ff;

	m_dwLineColor			=	0xffffffff;
	m_dwDisableFontColor	=	0xffa0a0a0;

	SetBackgroundMode( E_MENU_DRAW_RGB );

	m_v2dCurrentMousePos.x	=	0;
	m_v2dCurrentMousePos.y	=	0;

	m_nLastMouseDownIndex	=	-1;
	m_pParentUIMenu			=	NULL;

	m_Property.bTopmost		=	TRUE;

	m_nType					=	TYPE_MENU;

	m_bEnableLastYPos		=	FALSE;
	m_fLastYPos				=	100;
}

AcUIMenu::~AcUIMenu( VOID )
{
	Clear();
	//TextureClear();
}

VOID	AcUIMenu::Initialize( eDrawType	eType )
{
	SetBackgroundMode( eType );

	if( eType == E_MENU_DRAW_TEXTURE )
	{
		if( !m_bTextureLoad )
		{
			_SetBackImage( "PopupBtn_Base1.png" , E_MENU_BACK_TEXTURE_LEFT_TOP );
			_SetBackImage( "PopupBtn_Base2.png" , E_MENU_BACK_TEXTURE_TOP );
			_SetBackImage( "PopupBtn_Base3.png" , E_MENU_BACK_TEXTURE_RIGHT_TOP );

			_SetBackImage( "PopupBtn_Base4.png" , E_MENU_BACK_TEXTURE_LEFT );
			_SetBackImage( "PopupBtn_Base5.png" , E_MENU_BACK_TEXTURE_CENTER );
			_SetBackImage( "PopupBtn_Base6.png" , E_MENU_BACK_TEXTURE_RIGHT );

			_SetBackImage( "PopupBtn_Base7.png" , E_MENU_BACK_TEXTURE_LEFT_BOTTOM );
			_SetBackImage( "PopupBtn_Base8.png" , E_MENU_BACK_TEXTURE_BOTTOM );
			_SetBackImage( "PopupBtn_Base9.png" , E_MENU_BACK_TEXTURE_RIGHT_BOTTOM );

			_SetBackImage( "PopupBtn_Select1.png" , E_MENU_BACK_TEXTURE_SELECT_LEFT );
			_SetBackImage( "PopupBtn_Select2.png" , E_MENU_BACK_TEXTURE_SELECT_CENTER );
			_SetBackImage( "PopupBtn_Select3.png" , E_MENU_BACK_TEXTURE_SELECT_RIGHT );

			m_bTextureLoad	=	TRUE;
		}
	}

	ShowWindow( FALSE );
}

VOID	AcUIMenu::Clear( VOID )
{
	MapChildMenuIter	Iter	=	m_mapChildMenu.begin();
	for( ; Iter != m_mapChildMenu.end() ; ++Iter )
	{
		DEF_SAFEDELETE( Iter->second );
	}

	m_mapChildMenu.clear();
	m_listMenuEvent.clear();

}

VOID	AcUIMenu::ClearEvent( VOID )
{
	m_listMenuEvent.clear();
}

VOID	AcUIMenu::TextureClear( VOID )
{
	for( INT i = 0 ; i < E_MENU_BACK_TEXTURE_COUNT ; ++i )
	{
		if( m_pBackTexture[i] )
		{
			RwTextureDestroy( m_pBackTexture[i] );
			m_pBackTexture[i]	=	NULL;
		}
	}
}

INT		AcUIMenu::AddMenuEvent( stMenuEvent& rMenuEvent )
{
	// 이미 등록되어 있으면 Add를 못한다
	if( GetMenuEvent( rMenuEvent.m_strMenuName ) )
		return -1;

	m_listMenuEvent.push_back( rMenuEvent );

	return (INT)(m_listMenuEvent.size()-1);
}

stMenuEvent*	AcUIMenu::GetMenuEvent( CONST string& strMenuName )
{
	ListMenuEventIter	Iter	=	m_listMenuEvent.begin();
	for( ; Iter != m_listMenuEvent.end() ; ++Iter )
	{
		if( Iter->m_strMenuName	==	strMenuName )
			return &(*Iter);
	}

	return NULL;
}

stMenuEvent*	AcUIMenu::GetMenuEvent( INT nIndex )
{
	ListMenuEventIter	Iter	=	m_listMenuEvent.begin();
	for( INT i = 0  ; Iter != m_listMenuEvent.end() ; ++Iter , ++i )
	{
		if( nIndex == i )
			return &(*Iter);
	}

	return NULL;
}

BOOL	AcUIMenu::DelMenuEvent( INT nIndex )
{
	stMenuEvent*	pEventMenu	=	GetMenuEvent( nIndex );

	if( !pEventMenu )
		return FALSE;

	ListMenuEventIter	Iter	=	m_listMenuEvent.begin();
	for( ; Iter != m_listMenuEvent.end() ; ++Iter )
	{
		if( pEventMenu->m_strMenuName == Iter->m_strMenuName )
		{
			m_listMenuEvent.erase( Iter );
			return TRUE;
		}
	}

	return FALSE;
}

BOOL	AcUIMenu::DelMenuEvent( CONST string& strName )
{
	stMenuEvent*	pEventMenu	=	GetMenuEvent( strName );

	if( !pEventMenu )
		return FALSE;

	ListMenuEventIter	Iter	=	m_listMenuEvent.begin();
	for( ; Iter != m_listMenuEvent.end() ; ++Iter )
	{
		if( pEventMenu->m_strMenuName == Iter->m_strMenuName )
		{
			m_listMenuEvent.erase( Iter );
			return TRUE;
		}
	}

	return FALSE;
}

AcUIMenu*	AcUIMenu::CreateChildMenu( CONST string& strName )
{
	AcUIMenu*	pChildMenu	=	GetChildMenu(strName);
	if( pChildMenu )
		return pChildMenu;

	pChildMenu	=	new AcUIMenu;
	ASSERT( pChildMenu );

	pChildMenu->SetMenuName( strName );
	m_mapChildMenu.insert( make_pair(strName , pChildMenu) );

	pChildMenu->m_pParentUIMenu	=	this;
	pChildMenu->ShowWindow( FALSE );

	pChildMenu->Initialize( m_eDrawType );

	return pChildMenu;
}

AcUIMenu*	AcUIMenu::GetChildMenu( CONST string& strName )
{
	MapChildMenuIter	Iter	=	m_mapChildMenu.find( strName );
	if( Iter == m_mapChildMenu.end() )
		return NULL;

	return Iter->second;
}

BOOL	AcUIMenu::DeleteChildMenu( CONST string& strName )
{
	MapChildMenuIter	Iter	=	m_mapChildMenu.find( strName );
	if( Iter == m_mapChildMenu.end() )
		return FALSE;

	DeleteChild( Iter->second );
	DEF_SAFEDELETE( Iter->second );

	m_mapChildMenu.erase( Iter );

	return TRUE;
}

BOOL	AcUIMenu::DeleteChildMenu( AcUIMenu*& rChildMenu )
{
	BOOL	bResult	=	DeleteChildMenu( rChildMenu->GetMenuName() );

	if( bResult )
		rChildMenu	=	NULL;

	return bResult;
}

VOID	AcUIMenu::OnWindowRender( VOID )
{
	_RenderMenuBackground();
	_RenderMenuText();
}

BOOL	AcUIMenu::OnLButtonDblClk( RsMouseStatus *ms )
{
	return TRUE;
}

BOOL	AcUIMenu::OnLButtonDown( RsMouseStatus *ms )
{
	INT	nEventX	=	0;
	INT	nEventY	=	0;
	INT	nIndex	=	_GetCurrentEventIndex( nEventX , nEventY );

	m_nLastMouseDownIndex	=	nIndex;

	return TRUE;
}

BOOL	AcUIMenu::OnLButtonUp( RsMouseStatus *ms )
{
	INT	nEventX	=	0;
	INT	nEventY	=	0;
	INT nIndex	=	_GetCurrentEventIndex( nEventX , nEventY );

	// Event Go
	if( nIndex == m_nLastMouseDownIndex )
	{
		stMenuEvent*	pEvent	=	GetMenuEvent( nIndex );
		if( !pEvent )
			return FALSE;

		if( pEvent->m_bEnable)
		{
			_ProcessEvent( pEvent );

			if( pEvent->m_eEventType !=	E_MENU_EVENT_SUB_MENU )
			{
				ShowWindow( FALSE );

				AgcWindow*	pParentWindow	=	m_pParentUIMenu;
				while( pParentWindow && pParentWindow->m_nType == TYPE_MENU )
				{
					pParentWindow->ShowWindow( FALSE );
					pParentWindow	=	pParentWindow->pParent;
				}
			}
		}

	}

	return TRUE;
}

BOOL	AcUIMenu::OnIdle( UINT32 ulClockCount )
{
	if( g_pEngine->m_pFocusedWindow && g_pEngine->m_pFocusedWindow != this )
	{
		if( g_pEngine->m_pFocusedWindow->m_nType	== TYPE_MENU )
			return TRUE;

		ShowWindow( FALSE );
	}


	return TRUE;
}

BOOL	AcUIMenu::OnMouseMove( RsMouseStatus *ms )
{

	INT	nX		=	0;
	INT	nY		=	0;
	ClientToScreen( &nX, &nY);


	m_v2dCurrentMousePos	=	ms->pos;

	m_v2dCurrentMousePos.x	=	nX	+ ms->pos.x;
	m_v2dCurrentMousePos.y	=	nY	+ ms->pos.y;

	INT	nEventX	=	0;
	INT	nEventY	=	0;
	INT	nIndex	=	_GetCurrentEventIndex( nEventX , nEventY );

	ListMenuEventIter	Iter	=	m_listMenuEvent.begin();
	for( INT i = 0 ; Iter != m_listMenuEvent.end() ; ++Iter , ++i )
	{
		if( Iter->m_eEventType	==	E_MENU_EVENT_SUB_MENU && Iter->m_pvData && Iter->m_bEnable )
		{
			AcUIMenu*	pMenu	=	(AcUIMenu*)Iter->m_pvData;

			// 창이 꺼져 있는 상태면 마우스 위치 초기화를 해준다
			if( pMenu->m_Property.bVisible == FALSE )
			{
				pMenu->m_v2dCurrentMousePos.x	=	0;
				pMenu->m_v2dCurrentMousePos.y	=	0;
			}

			if( nIndex == i )
			{
				INT		nStringHeight	=	m_pAgcmFont->m_iYL[ m_lFontType ];

				pMenu->MoveWindow( x+w+DEFAULT_MENU_GAP , y+(i*(nStringHeight+DEFAULT_MENU_GAP))+DEFAULT_OFFSET_Y , 30 , 30 );
			}

			pMenu->ShowWindow( nIndex == i ? TRUE : FALSE );
			
		}
	}

	return TRUE;
}

VOID	AcUIMenu::OnSetFocus( VOID )
{
	AcUIBase::OnSetFocus();
}

VOID	AcUIMenu::OnKillFocus( VOID )
{
	AcUIBase::OnKillFocus();

}

VOID	AcUIMenu::_ProcessEvent( stMenuEvent* pEvent )
{
	if( !pEvent )
		return;

	switch( pEvent->m_eEventType )
	{

	case E_MENU_EVENT_FUNCTION:
		{
			((fnMenuEvent)pEvent->m_pvData)( pEvent->m_pClass , pEvent->m_pData1 , pEvent->m_pData2 , this );
		}

		break;
	}
}

VOID	AcUIMenu::_RenderMenuText( VOID )
{
	INT		nAbsolute_x		=	0;
	INT		nAbsolute_y		=	0;
	INT		nStringHeight	=	m_pAgcmFont->m_iYL[ m_lFontType ];
	INT		nTextExtent		=	m_pAgcmFont->GetTextExtent( m_lFontType, " >" , strlen( " >" ) );
	ClientToScreen( &nAbsolute_x, &nAbsolute_y );

	// Font Draw
	m_pAgcmFont->FontDrawStart( m_lFontType );

	ListMenuEventIter	Iter	=	m_listMenuEvent.begin();

	for( INT i = 0 ; Iter != m_listMenuEvent.end() ; ++Iter , ++i )
	{
		INT		nX		=	0;
		INT		nY		=	0;
		INT		nTemp	=	0;
		DWORD	dwColor	=	0;

		nTemp	=	m_pAgcmFont->GetTextExtent( m_lFontType, (CHAR*)Iter->m_strMenuName.c_str() , (INT32)Iter->m_strMenuName.length() );
		nX		=	nAbsolute_x + DEFAULT_OFFSET_X;
		nY		=	nAbsolute_y + ( i*(nStringHeight+DEFAULT_MENU_GAP) ) + DEFAULT_OFFSET_Y;

		
		if( Iter->m_bEnable )
			dwColor	=	Iter->m_dwFontColor;
		else
			dwColor	=	m_dwDisableFontColor;

		m_pAgcmFont->DrawTextIM2D( (float)nX , (float)nY , (CHAR*)Iter->m_strMenuName.c_str() , m_lFontType , 255 , dwColor );

		if( Iter->m_eEventType	==	E_MENU_EVENT_SUB_MENU )
		{
			m_pAgcmFont->DrawTextIM2D( (float)(nX+w-nTextExtent-DEFAULT_OFFSET_Y) , (float)nY , " >" , m_lFontType , 255 , dwColor  );
		}
	}

	m_pAgcmFont->FontDrawEnd();
}

VOID	AcUIMenu::_RenderMenuBackground( VOID )
{
	INT		nSize			=	(INT)m_listMenuEvent.size();
	INT		nStringExtent	=	0;
	INT		nStringHeight	=	m_pAgcmFont->m_iYL[ m_lFontType ];
	BOOL	bMenuExtent		=	FALSE;

	// 가장 넓은 길이의 문자열을 찾는다( 이게 넓이값이 된다 )
	ListMenuEventIter	Iter	=	m_listMenuEvent.begin();
	for( ; Iter != m_listMenuEvent.end() ; ++Iter )
	{
		INT	nTemp	=	0;
		nTemp	=	m_pAgcmFont->GetTextExtent( m_lFontType, (CHAR*)Iter->m_strMenuName.c_str() , (INT32)Iter->m_strMenuName.length() );

		if( Iter->m_eEventType	== E_MENU_EVENT_SUB_MENU )
			bMenuExtent	=	TRUE;

		if( nStringExtent < nTemp )
			nStringExtent	=	nTemp;
	}

	// 우선 글자크기와 공백 사이즈를 알아낸다
	w	=	nStringExtent + DEFAULT_OFFSET_X + DEFAULT_OFFSET_X;
	h	=	(DEFAULT_MENU_GAP*(nSize-1)) + (nStringHeight*nSize) + DEFAULT_OFFSET_Y + DEFAULT_OFFSET_Y;

	if( bMenuExtent )
	{
		INT	nTextExtent	=	m_pAgcmFont->GetTextExtent( m_lFontType, " >" , strlen( " >" ) );
		w	=	w	+	nTextExtent;
	}

	// Texture로 백그라운드를 그린다면 Width 와 Height를 조정한다
	if( m_eDrawType == E_MENU_DRAW_TEXTURE )
	{
		// w의 디폴트 크기를 맞춘다
		if( w < DEFAULT_BACK_CORNER_X+DEFAULT_BACK_CORNER_X )
		{
			w	=	DEFAULT_BACK_CORNER_X+DEFAULT_BACK_CORNER_X;
		}

		if( h < DEFAULT_BACK_CORNER_Y+DEFAULT_BACK_CORNER_Y )
		{
			h	=	DEFAULT_BACK_CORNER_Y+DEFAULT_BACK_CORNER_Y;
		}

		// Width 크기를 DEFAULT_BACK_BODY_X 크기의 배수로 맞춘다.
		//if( (w % DEFAULT_BACK_BODY_X) )
		//	w	=	w + DEFAULT_BACK_BODY_X - (w % DEFAULT_BACK_BODY_X);

		// Height 크기를 DEFAULT_BACK_BODY_Y 크기의 배수로 맞춘다.
		//if( (h % DEFAULT_BACK_BODY_Y) )
		//	h	=	h +	DEFAULT_BACK_BODY_Y - (h % DEFAULT_BACK_BODY_Y );
	}	

	if( m_bEnableLastYPos )
	{
		INT		nX	=	0;
		INT		nY	=	0;
		ClientToScreen( &nX , &nY );

		y = (INT16)(y + (m_fLastYPos -	nY - h)) ;

		m_bEnableLastYPos	=	FALSE;
	}


	INT			nAbsolute_x		=	0;
	INT			nAbsolute_y		=	0;
	AgcWindow*	pTempWindow		=	pParent;
	ClientToScreen( &nAbsolute_x, &nAbsolute_y );


	while( pTempWindow && pTempWindow->m_nType == TYPE_MENU )
	{
		pTempWindow	=	pTempWindow->pParent;
	}

	if( pTempWindow )
	{
		BOOL	bUpdateX	=	FALSE;
		BOOL	bUpdateY	=	FALSE;

		if( nAbsolute_x + w > pTempWindow->w )
		{
			x = nAbsolute_x	= pTempWindow->w - ( w );
			bUpdateX	=	TRUE;
		}

		if( nAbsolute_y + h > pTempWindow->h ) 
		{
			y = nAbsolute_y	= pTempWindow->h - ( h );
			bUpdateY	=	TRUE;
		}

		pTempWindow	=	pParent;
		while( pTempWindow )
		{
			if( bUpdateX )
				x	=	x - pTempWindow->x;

			if( bUpdateY )
				y	=	y - pTempWindow->y;
			pTempWindow	=	pTempWindow->pParent;
		}
	}

	INT	nEventPosX	=	0;
	INT	nEventPosY	=	0;


	if( m_eDrawType	== E_MENU_DRAW_RGB )
	{
		// 판넬을 만든다
		g_pEngine->Draw2DPanel( nAbsolute_x , nAbsolute_y , w , h , m_dwBackgroundColor );	//	백판.

		// 윤곽 라인을 그린다
		g_pEngine->Draw2DPanel( nAbsolute_x-DEFAULT_MENU_GAP	, nAbsolute_y-DEFAULT_MENU_GAP	, w+DEFAULT_MENU_GAP*2	, DEFAULT_MENU_GAP		, m_dwLineColor );
		g_pEngine->Draw2DPanel( nAbsolute_x+w					, nAbsolute_y-DEFAULT_MENU_GAP	, DEFAULT_MENU_GAP		, h+DEFAULT_MENU_GAP*2	, m_dwLineColor );
		g_pEngine->Draw2DPanel( nAbsolute_x-DEFAULT_MENU_GAP	, nAbsolute_y-DEFAULT_MENU_GAP	, DEFAULT_MENU_GAP		, h+DEFAULT_MENU_GAP*2	, m_dwLineColor );
		g_pEngine->Draw2DPanel( nAbsolute_x-DEFAULT_MENU_GAP	, nAbsolute_y+h					, w+DEFAULT_MENU_GAP*2	, DEFAULT_MENU_GAP		, m_dwLineColor );

		//	선택 된 이미지를 찍어준다.
		if( _GetCurrentEventIndex( nEventPosX , nEventPosY) != -1 )	
			g_pEngine->Draw2DPanel( nEventPosX , nEventPosY , w , nStringHeight , m_dwSelectColor );
	}

	// Texture로 배경을 그려준다 ㅠ,.ㅠ
	else if( m_eDrawType == E_MENU_DRAW_TEXTURE )
	{
		// 우선 코너를 그린다
		_DrawTextureCorner( nAbsolute_x , nAbsolute_y , w , h );

		// 코너를 제외한 안을 채운다
		_DrawTextureBody( nAbsolute_x , nAbsolute_y , w , h );

		// 선택된 이미지를 그려준다
		if( _GetCurrentEventIndex( nEventPosX , nEventPosY) != -1 )
			_DrawTextureSelect( nEventPosX , nEventPosY , w , nStringHeight );
	}

}


INT	AcUIMenu::_GetCurrentEventIndex(  OUT INT& nEventX , OUT INT& nEventY   )
{
	INT		nAbsolute_x		=	0;
	INT		nAbsolute_y		=	0;
	INT		nStringHeight	=	m_pAgcmFont->m_iYL[ m_lFontType ];
	INT		nIndex			=	-1;
	INT		nOffset			=	0;

	ClientToScreen( &nAbsolute_x, &nAbsolute_y );


	// 현재 마우스 커서 위치가 어디 이벤트 위에 있는지 찾는다
	if( m_v2dCurrentMousePos.x > nAbsolute_x &&	m_v2dCurrentMousePos.x < (nAbsolute_x+w) )
	{
		if( m_v2dCurrentMousePos.y > nAbsolute_y && m_v2dCurrentMousePos.y < (nAbsolute_y+h) )
		{
			nOffset	=	(INT)(m_v2dCurrentMousePos.y) - nAbsolute_y - (DEFAULT_OFFSET_Y-DEFAULT_MENU_GAP) - (DEFAULT_MENU_GAP/2);

			if( nOffset > 0 )
				nIndex	=	nOffset / (nStringHeight+DEFAULT_MENU_GAP);
			else
				nIndex	=	-1;

			nEventX		=	nAbsolute_x;
			nEventY		=	nAbsolute_y + ( nIndex*(nStringHeight+DEFAULT_MENU_GAP) ) + DEFAULT_OFFSET_Y;
		}
	}

	if( (INT)m_listMenuEvent.size() <= nIndex )
		nIndex	=	-1;

	return nIndex;
}

#define DRAWIM2D(a,b,c,d,e) g_pEngine->DrawIm2D(a, (float)b, (float)c, (float)d, (float)e)
VOID	AcUIMenu::_DrawTextureCorner( INT nPosX , INT nPosY , INT nWidth , INT nHeight )
{

	// 위 아래 
	DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_LEFT_TOP] , nPosX ,													 nPosY ,													DEFAULT_BACK_CORNER_X , DEFAULT_BACK_CORNER_Y );
	DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_RIGHT_TOP] , nPosX+nWidth-DEFAULT_BACK_CORNER_X ,	nPosY ,													DEFAULT_BACK_CORNER_X , DEFAULT_BACK_CORNER_Y );
	DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_LEFT_BOTTOM] , nPosX ,												nPosY+nHeight-DEFAULT_BACK_CORNER_Y , DEFAULT_BACK_CORNER_X , DEFAULT_BACK_CORNER_Y );
	DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_RIGHT_BOTTOM] , nPosX+nWidth-DEFAULT_BACK_CORNER_X , nPosY+nHeight-DEFAULT_BACK_CORNER_Y , DEFAULT_BACK_CORNER_X , DEFAULT_BACK_CORNER_Y );

	INT		nCount		=	nWidth	/	DEFAULT_BACK_CORNER_X;
	INT		nRest		=	nWidth  %	DEFAULT_BACK_CORNER_X;

	for( INT i = 1 ; i < (nCount-1) ; ++i )
	{
		DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_TOP]	, nPosX+(DEFAULT_BACK_CORNER_X*i)  , nPosY , DEFAULT_BACK_CORNER_X , DEFAULT_BACK_CORNER_Y );
		DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_BOTTOM] , nPosX+(DEFAULT_BACK_CORNER_X*i)  , nPosY+nHeight-DEFAULT_BACK_CORNER_Y , DEFAULT_BACK_CORNER_X , DEFAULT_BACK_CORNER_Y );
	}

	if( nRest )
	{
		DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_TOP]	, nPosX+(DEFAULT_BACK_CORNER_X*(nCount-1)) , nPosY , nRest , DEFAULT_BACK_CORNER_Y );
		DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_BOTTOM] , nPosX+(DEFAULT_BACK_CORNER_X*(nCount-1)) , nPosY+nHeight-DEFAULT_BACK_CORNER_Y , nRest , DEFAULT_BACK_CORNER_Y );
	}
}

VOID	AcUIMenu::_DrawTextureBody( INT nPosX , INT nPosY , INT nWidth , INT nHeight )
{
	// 가운데
	INT		nBodyWidthCount			=	(nWidth	 / DEFAULT_BACK_BODY_X);
	INT		nBodyWidthRest			=	(nWidth	 % DEFAULT_BACK_BODY_X);

	INT		nBodyHeightCount		=	(nHeight / DEFAULT_BACK_BODY_Y) - ((DEFAULT_BACK_CORNER_Y*2)/DEFAULT_BACK_BODY_Y);
	INT		nBodyHeightRest			=	(nHeight % DEFAULT_BACK_BODY_Y);

	for( INT i = 0 ; i <= nBodyHeightCount ; ++i )
	{
		INT		nStartPosY	=	nPosY+DEFAULT_BACK_CORNER_Y+(DEFAULT_BACK_BODY_Y*i);
		INT		nDrawHeight	=	DEFAULT_BACK_BODY_Y;

		if( i == nBodyHeightCount )
			nDrawHeight	=	nBodyHeightRest;

		if( !nDrawHeight )
			continue;

		DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_LEFT] , nPosX , nStartPosY , DEFAULT_BACK_BODY_X , nDrawHeight );
		DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_RIGHT] , nPosX+nWidth-DEFAULT_BACK_BODY_X , nStartPosY , DEFAULT_BACK_BODY_X , nDrawHeight );

		for( INT k = 1 ; k <= (nBodyWidthCount-1) ; ++k )
		{
			INT		nDrawWidth	=	DEFAULT_BACK_BODY_X;
			if( k == (nBodyWidthCount-1)  )
				nDrawWidth	=	nBodyWidthRest;

			if( nDrawWidth )
				DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_CENTER] , nPosX+(DEFAULT_BACK_BODY_X*k) , nStartPosY , nDrawWidth , nDrawHeight  );
		}
	}

}

VOID	AcUIMenu::_DrawTextureSelect( INT nPosX , INT nPosY , INT nWidth , INT nHeight )
{
	INT		nSelectWidth	=	nWidth	-	(DEFAULT_MENU_GAP*2);
	INT		nCount			=	nSelectWidth	/	DEFAULT_SELECT;
	INT		nRest			=	nSelectWidth	%	DEFAULT_SELECT;

	DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_SELECT_LEFT]  , nPosX+DEFAULT_MENU_GAP , nPosY , DEFAULT_SELECT , DEFAULT_SELECT );
	DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_SELECT_RIGHT] , nPosX+nWidth-DEFAULT_SELECT-DEFAULT_MENU_GAP , nPosY , DEFAULT_SELECT , DEFAULT_SELECT );

	for( INT i = 1 ; i <= (nCount-1) ; ++i )
	{
		INT		nDrawWidth	=	DEFAULT_SELECT;

		if( i == (nCount-1) )
		{
			nDrawWidth	=	nRest;
		}

		if( nDrawWidth )
			DRAWIM2D( m_pBackTexture[E_MENU_BACK_TEXTURE_SELECT_CENTER] , nPosX+(DEFAULT_SELECT*i)+DEFAULT_MENU_GAP , nPosY , nDrawWidth , DEFAULT_SELECT );
	}
}


VOID	AcUIMenu::EnableEvent( CONST string& strName , BOOL bEnable )
{
	stMenuEvent*	pEvent	=	GetMenuEvent( strName );
	
	if( pEvent )
		pEvent->m_bEnable	=	bEnable;
}

VOID	AcUIMenu::EnableEvent( INT nIndex , BOOL bEnable )
{
	stMenuEvent*	pEvent	=	GetMenuEvent( nIndex );

	if( pEvent )
		pEvent->m_bEnable	=	bEnable;
}

VOID	AcUIMenu::_SetBackImage( CONST string& strTextureFileName , eBackTexture eTextureType )
{
	if( m_pBackTexture[eTextureType] )
	{
		RwTextureDestroy( m_pBackTexture[eTextureType] );
		m_pBackTexture[eTextureType]	=	NULL;
	}

	m_pBackTexture[eTextureType]	=	RwTextureRead( (CHAR*)strTextureFileName.c_str() , NULL );

	if( m_pBackTexture[eTextureType] )
	{
		RwTextureSetFilterMode( m_pBackTexture[eTextureType], rwFILTERNEAREST );
		RwTextureSetAddressing( m_pBackTexture[eTextureType], rwTEXTUREADDRESSCLAMP );
	}
}