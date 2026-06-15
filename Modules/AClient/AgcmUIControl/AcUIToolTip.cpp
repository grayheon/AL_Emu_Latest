#include "AcUIToolTip.h"

#include "ApMemoryTracker.h"
#include "AcUIEventButton.h"

#include "AuStrTable.h"
#include "AgcmUIManager2.h"


#define		EXIT_BUTTON_IMAGE_NORMAL		"Btn_X_a.png"
#define		EXIT_BUTTON_IMAGE_OVER			"Btn_X_b.png"
#define		EXIT_BUTTON_IMAGE_CLICK			"Btn_X_c.png"

#define		TOOLTIP_LINE_SIZE				2


RwTexture* AcUIToolTip::m_pTTTexture[ ACUITOOLTIP_TTTEXTURE_NUM ] = { NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL };
BOOL AcUIToolTip::m_bViewToolTip = TRUE;

/*****************************************************************
*   Function : AcUIToolTip
*   Comment  : 생성자 
*   Date&Time : 2003-07-15, 오후 12:06
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
AcUIToolTip::AcUIToolTip( void )
{
	m_nType = TYPE_TOOLTIP;

	// 초기 크기 - 최소 크기가 된다 
	w = ACUITOOLTIP_TTBOX_INIT_WIDTH;
	h = ACUITOOLTIP_TTBOX_INIT_HEIGHT;

	m_lStringStartY	= ACUITOOLTIP_TTBOX_STRING_START_Y;
	m_bViewToolTip = TRUE;

	// Tooltip때문에 Focus에 영향이 있어서 UseInput을 FALSE로 주어서, WindowFromPoint()에서 안걸리도록 한다.
	m_Property.bUseInput = FALSE;
	m_Property.bTopmost	= TRUE;
	m_bDrawTooltipBox = TRUE;
	m_pParentUIWindow = NULL;

	m_bEnableExit	=	FALSE;

	subTooltipType_	=	sub_tooltip_none;

	m_bEnableLineColor	=	FALSE;
	m_dwLineColor		=	0xffffffff;

	m_nOffsetX			=	0;
	m_nOffsetY			=	0;

	m_pExitButton	=	NULL;
}

/*****************************************************************
*   Function : ~AcUIToolTip
*   Comment  : 소멸자 
*   Date&Time : 2003-07-15, 오후 12:06
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
AcUIToolTip::~AcUIToolTip( void )
{	
	DEF_SAFEDELETE( m_pExitButton );
}

/*****************************************************************
*   Function : OnWindowRender
*   Comment  : Window Render virtual function overriding
*   Date&Time : 2003-07-15, 오후 3:15
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
void AcUIToolTip::OnWindowRender( void )
{
	PROFILE( "AcUIToolTip::OnWindowRender" );
	if( m_bViewToolTip )
	{
		_UpdateExitButtonPosition();

		DrawToolTipBox();
		DrawItemInfo();
		
	}
}

/*****************************************************************
*   Function : DrawToolTipBox
*   Comment  : 
*   Date&Time : 2003-07-15, 오후 3:21
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
void AcUIToolTip::DrawToolTipBox( void )
{
	// ToolTip그려질 위치 

	RwV2d	vRenderPos	=	GetRenderPos();

	// 그리자!
	My2DVertex BoxFan[ 4 ];
	DWORD color = 0x00ffffff;
		
	float fx = vRenderPos.x;
	float fy = vRenderPos.y;
	float lLTW = 0.0f;		// Line Texture Width
	float lCW = 0.0f;		// Corner Width

	if( m_bDrawTooltipBox )
	{
		lLTW = ACUITOOLTIP_TTBOX_LINE_TEXTURE_WIDTH;	// Line Texture Width
		lCW = ACUITOOLTIP_TTBOX_LINE_CORNER_WIDTH;		// Corner Width
	}

	color |= ( ( DWORD )( 255 * ( m_pfAlpha ? *m_pfAlpha : 1 ) ) << 24 );
	
	for( INT32 nFanCount = 0 ; nFanCount < 4 ; ++nFanCount )
	{
		BoxFan[ nFanCount ].color = color;
		BoxFan[ nFanCount ].z = 0.0f;
		BoxFan[ nFanCount ].rhw = 1.0f;
	}

	BoxFan[ 0 ].u = 1.0f;	BoxFan[ 0 ].v = 1.0f;
	BoxFan[ 1 ].u = 1.0f;	BoxFan[ 1 ].v = 0.0f;
	BoxFan[ 2 ].u = 0.0f;	BoxFan[ 2 ].v = 1.0f;
	BoxFan[ 3 ].u = 0.0f;	BoxFan[ 3 ].v = 0.0f;

	RwD3D9SetFVF( MY2D_VERTEX_FLAG );

	if( m_bDrawTooltipBox )
	{
		_DrawToolTipBoxCorner( BoxFan, fx, fy, lCW, lLTW );
	}

	_DrawToolTipBoxBody( BoxFan, fx, fy, lCW, lLTW );

	if( m_bDrawTooltipBox )
	{
		_DrawToolTipBoxEdge( BoxFan, fx, fy, lCW, lLTW );
	}

	_DrawLine( (INT32)fx , (INT32)fy );
}

/*****************************************************************
*   Function : SetBoxIm2D
*   Comment  : 
*   Date&Time : 2003-07-15, 오후 3:37
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
void AcUIToolTip::SetBoxIm2D( My2DVertex* vert, float x, float y, float w, float h )
{
	//@{ 2006/06/17 burumal
	vert[ 0 ].x = x + w + 0.5f; 	vert[ 0 ].y = y + h + 0.5f;
	vert[ 1 ].x = x + w + 0.5f; 	vert[ 1 ].y = y - 0.5f;
	vert[ 2 ].x = x - 0.5f;			vert[ 2 ].y = y + h + 0.5f;
	vert[ 3 ].x = x - 0.5f;			vert[ 3 ].y = y - 0.5f;
	//@}
}

void AcUIToolTip::SetBoxIm2D2( My2DVertex* vert, float x, float y, float w, float h )
{
	//@{ 2006/06/17 burumal
	vert[ 0 ].x = x + w + 0.5f; 	vert[ 0 ].y = y - 0.5f;
	vert[ 1 ].x = x + w + 0.5f; 	vert[ 1 ].y = y + h + 0.5f;
	vert[ 2 ].x = x - 0.5f;			vert[ 2 ].y = y + h + 0.5f;
	vert[ 3 ].x = x - 0.5f;			vert[ 3 ].y = y - 0.5f;
	//@}
}

/*****************************************************************
*   Function : OnClose
*   Comment  : On Close virtual function overriding 
*   Date&Time : 2003-07-16, 오후 4:56
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
void AcUIToolTip::OnClose( void )
{
	AcUIBase::OnClose();
	DeleteAllStringInfo();
}

/*****************************************************************
*   Function : AddString
*   Comment  : AddString
*   Date&Time : 2003-09-23, 오후 9:25
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
BOOL AcUIToolTip::AddString( CHAR* pszString, INT32 l_x, DWORD lColor , INT32 lFontType )
{
	if( !pszString ) return FALSE;
	if( strlen( pszString ) <= 0 ) return FALSE;

	AgcdUIToolTipItemInfo stTTItemInfo;

	stTTItemInfo.pstTexture	= NULL;
	stTTItemInfo.lFontType = lFontType;
	stTTItemInfo.lColor = lColor;

	if( m_bDrawTooltipBox )
	{
		stTTItemInfo.lStartY = m_lStringStartY;
		stTTItemInfo.lStartX = l_x	+ ACUITOOLTIP_TTBOX_STRING_START_X;
	}
	else
	{
		stTTItemInfo.lStartY = 0;
		stTTItemInfo.lStartX = 0;
	}
	
	stTTItemInfo.pszString = new CHAR[ ( strlen( pszString ) + 1 ) ];
	memset( stTTItemInfo.pszString, 0, strlen( pszString ) + 1 );
	strcpy( stTTItemInfo.pszString, pszString );

	m_listStringInfo.AddTail( stTTItemInfo );	

	// Tool Tip Size 조절하기 
	INT32 lStringExtent = m_pAgcmFont->GetTextExtent( lFontType, pszString, strlen( pszString ) );

	if( m_bDrawTooltipBox )
	{
		if( w < ( ACUITOOLTIP_TTBOX_INIT_WIDTH + stTTItemInfo.lStartX + lStringExtent - 4 ) )
		{
			w = ACUITOOLTIP_TTBOX_INIT_WIDTH + stTTItemInfo.lStartX + lStringExtent - 4;
		}
	}
	else
	{
		if( w < ( stTTItemInfo.lStartX + lStringExtent - 4 ) )
		{
			w = stTTItemInfo.lStartX + lStringExtent - 4;
		}
	}
		
	return TRUE;
}

BOOL AcUIToolTip::AddToolTipTexture( RwTexture* pstTexture, INT32 lWidth, INT32 l_x, DWORD lColor, BOOL bIsOverWrite )
{
	if( !pstTexture ) return FALSE;

	AgcdUIToolTipItemInfo stTTItemInfo;

	stTTItemInfo.pszString	= NULL;
	stTTItemInfo.pstTexture	= pstTexture;
	stTTItemInfo.lColor		= lColor;
	stTTItemInfo.lStartY	= m_lStringStartY;
	stTTItemInfo.lStartX	= l_x + ACUITOOLTIP_TTBOX_STRING_START_X;

	m_listStringInfo.AddTail( stTTItemInfo );

	// Tool Tip Size 조절하기 
	if( !bIsOverWrite )
	{
		if( w < ( ACUITOOLTIP_TTBOX_INIT_WIDTH + stTTItemInfo.lStartX + lWidth - 12 ) )
		{
			w = ACUITOOLTIP_TTBOX_INIT_WIDTH + stTTItemInfo.lStartX + lWidth - 12;
		}
	}
		
	return TRUE;
}

/*****************************************************************
*   Function : AddNewLine
*   Comment  : AddNewLine
*   Date&Time : 2003-09-23, 오후 9:32
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
BOOL AcUIToolTip::AddNewLine( INT32 lLineGap_Y )
{
	AgcdUIToolTipItemInfo stTTItemInfo;
	stTTItemInfo.bNewLine =	TRUE;

	m_listStringInfo.AddTail( stTTItemInfo );
	m_lStringStartY += lLineGap_Y;

	// Tool Tip Size 조절하기 
	h = ACUITOOLTIP_TTBOX_INIT_HEIGHT + m_lStringStartY;
	return TRUE;
}

/*****************************************************************
*   Function : DeleteAllStringInfo
*   Comment  : DeleteAllStringInfo
*   Date&Time : 2003-09-23, 오후 9:38
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
void AcUIToolTip::DeleteAllStringInfo( void )
{
	AuNode< AgcdUIToolTipItemInfo >* pListNode = NULL;
	pListNode = m_listStringInfo.GetHeadNode();

	AgcdUIToolTipItemInfo stTTItemInfo;

	while( pListNode )
	{
		stTTItemInfo = m_listStringInfo.GetNext( pListNode );		
		if( stTTItemInfo.pszString )
		{
			delete[] ( stTTItemInfo.pszString );
		}
	}

	m_listStringInfo.RemoveAll();

	w = ACUITOOLTIP_TTBOX_INIT_WIDTH;
	h = ACUITOOLTIP_TTBOX_INIT_HEIGHT;
	m_lStringStartY	= ACUITOOLTIP_TTBOX_STRING_START_Y;

	m_nOffsetX	=	0;
	m_nOffsetY	=	0;
}

/*****************************************************************
*   Function : AcUIToolTip
*   Comment  : AcUIToolTip
*   Date&Time : 2003-09-24, 오후 3:19
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
void AcUIToolTip::DrawItemInfo( void )
{
	// 시작 
	if( !m_pAgcmFont ) 
		return;

	RwV2d	vRenderPos	=	GetRenderPos();
	UINT8	nAlpha		=	( UINT8 )( 255.0f * ( m_pfAlpha ? *m_pfAlpha : 1) );


	AuNode< AgcdUIToolTipItemInfo >* pListNode = m_listStringInfo.GetHeadNode();

	int	before_type = -1;
	while( pListNode )
	{
		AgcdUIToolTipItemInfo& stTTItemInfo = m_listStringInfo.GetNext( pListNode );

		FLOAT fPosX = ( FLOAT )( vRenderPos.x + stTTItemInfo.lStartX );
		FLOAT fPosY = ( FLOAT )( vRenderPos.y + stTTItemInfo.lStartY );
		

		if( stTTItemInfo.pszString )
		{
			if( before_type != stTTItemInfo.lFontType )
			{
				if( before_type != -1 )	m_pAgcmFont->FontDrawEnd();
				m_pAgcmFont->FontDrawStart( stTTItemInfo.lFontType );
				before_type = stTTItemInfo.lFontType;
			}

			m_pAgcmFont->DrawTextIM2D( fPosX, fPosY, stTTItemInfo.pszString, stTTItemInfo.lFontType, nAlpha, stTTItemInfo.lColor );
		}
		else if( stTTItemInfo.pstTexture && RwTextureGetRaster( stTTItemInfo.pstTexture ) )
		{
			g_pEngine->DrawIm2D( stTTItemInfo.pstTexture, fPosX, fPosY,
				( FLOAT )RwRasterGetWidth( RwTextureGetRaster( stTTItemInfo.pstTexture ) ),
				( FLOAT )RwRasterGetHeight( RwTextureGetRaster( stTTItemInfo.pstTexture ) ),
				0.0f, 0.0f, 1.0f, 1.0f, 0x00ffffff, nAlpha );
		}
	}
	m_pAgcmFont->FontDrawEnd();

	char * sub_type_string = 0;

	// 착용 아이템이라는 메세지를 뿌려준다.
	switch( subTooltipType_ )
	{
	case sub_tooltip_equip :
		{
			sub_type_string = ClientStr().GetStr( STI_COMPARE_EQUIPPEDITEM );
		}
		break;

	case sub_tooltip_refinery :
		{
			AgcmUIManager2 * uiManager = (AgcmUIManager2*)g_pEngine->GetModule( "AgcmUIManager2" );
			if( uiManager )
				sub_type_string = uiManager->GetUIMessage( "refinery_result" );
		}
		break;
	}

	if( sub_type_string )
	{
		m_pAgcmFont->FontDrawStart( 0 );

		INT	nWidth	=	m_pAgcmFont->GetTextExtent( 2, sub_type_string , strlen(sub_type_string) );
		m_pAgcmFont->DrawTextIM2D( vRenderPos.x + w - nWidth /*- ACUITOOLTIP_TTBOX_STRING_START_X*/ - 8 
			, vRenderPos.y + ACUITOOLTIP_TTBOX_STRING_START_Y + 18, sub_type_string 
			, 2, nAlpha , m_dwLineColor , true , false );

		m_pAgcmFont->FontDrawEnd();
	}

}

void* AcUIToolTip::_GetRasterVoidPtr( INT32 nTextureIndex )
{
	return ( void* )m_pTTTexture[ nTextureIndex ] ? RwTextureGetRaster( m_pTTTexture[ nTextureIndex ] ) : NULL;
}

void AcUIToolTip::_DrawToolTipBoxBody( My2DVertex* pVert, float fPosX, float fPosY,
									  float fCornerWidth, float fLineTextureWidth )
{
	if( !pVert ) return;

	void* pTexture = _GetRasterVoidPtr( 0 );

	// 중앙 
	SetBoxIm2D( pVert, fPosX + fCornerWidth, fPosY + fCornerWidth, w - 2.0f * fCornerWidth,	h - 2.0f * fCornerWidth );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, pTexture );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );

	// 중앙 왼쪽
	SetBoxIm2D( pVert, fPosX + fLineTextureWidth, fPosY + fCornerWidth, fCornerWidth - fLineTextureWidth, h - 2.0f * fCornerWidth );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, pTexture );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );

	// 중앙 오른쪽
	SetBoxIm2D( pVert, fPosX + w - fCornerWidth, fPosY + fCornerWidth, fCornerWidth - fLineTextureWidth, h - 2.0f * fCornerWidth );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, pTexture );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );

	// 중앙 위쪽
	SetBoxIm2D( pVert, fPosX + fCornerWidth, fPosY + fLineTextureWidth, w - 2.0f * fCornerWidth, fCornerWidth - fLineTextureWidth );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, pTexture );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );

	// 중앙 아래쪽
	SetBoxIm2D( pVert, fPosX + fCornerWidth, fPosY + h - fCornerWidth, w - 2.0f * fCornerWidth, fCornerWidth - fLineTextureWidth );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, pTexture );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );
}

void AcUIToolTip::_DrawToolTipBoxCorner( My2DVertex* pVert, float fPosX, float fPosY,
										float fCornerWidth, float fLineTextureWidth )
{
	if( !pVert ) return;

	// Top - Left 코너 
	SetBoxIm2D( pVert, fPosX,	fPosY,	32,	32 );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, _GetRasterVoidPtr( 5 ) );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );

	// Top - Right 코너 
	SetBoxIm2D( pVert, fPosX + w - fCornerWidth, fPosY, 32, 32 );
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, _GetRasterVoidPtr( 6 ) );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );

	// Bottom - Left 코너 
	SetBoxIm2D( pVert, fPosX,	fPosY + h - fCornerWidth, 32, 32 );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, _GetRasterVoidPtr( 7 ) );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );

	// Bottom - Right 코너 
	SetBoxIm2D( pVert, fPosX + w - fCornerWidth, fPosY + h - fCornerWidth, 32, 32 );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, _GetRasterVoidPtr( 8 ) );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );
}

void AcUIToolTip::_DrawToolTipBoxEdge( My2DVertex* pVert, float fPosX, float fPosY,
									  float fCornerWidth, float fLineTextureWidth )
{
	if( !pVert ) return;

	// Left 라인 
	SetBoxIm2D( pVert, fPosX, fPosY + fCornerWidth, fLineTextureWidth,	h - 2.0f * fCornerWidth );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, _GetRasterVoidPtr( 3 ) );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );

	// Right 라인 
	SetBoxIm2D( pVert, fPosX + w - fLineTextureWidth, fPosY + fCornerWidth, fLineTextureWidth, h - 2.0f * fCornerWidth );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, _GetRasterVoidPtr( 4 ) );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, pVert, SIZE_MY2D_VERTEX );

	// 가로선은 box구성 순서를 틀리게 해야 제대로 나옴(이유는 알수 없음 ㅡㅡ;)
	pVert[ 0 ].v = 0.0f;
	pVert[ 1 ].v = 1.0f;
	pVert[ 2 ].v = 1.0f;
	pVert[ 3 ].v = 0.0f;

	// Top 라인 
	SetBoxIm2D2( pVert, fPosX + fCornerWidth, fPosY, w -  2.0f * fCornerWidth,	fLineTextureWidth );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, _GetRasterVoidPtr( 1 ) );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, pVert, SIZE_MY2D_VERTEX );

	// Bottom 라인 
	SetBoxIm2D2( pVert, fPosX + fCornerWidth, fPosY + h - fLineTextureWidth, w -  2.0f * fCornerWidth, fLineTextureWidth );
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, _GetRasterVoidPtr( 2 ) );
	RwD3D9DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, pVert , SIZE_MY2D_VERTEX );

	// 바꾼거 원상복구
	pVert[ 0 ].v = 1.0f;
	pVert[ 1 ].v = 0.0f;
	pVert[ 2 ].v = 1.0f;
	pVert[ 3 ].v = 0.0f;
}


VOID AcUIToolTip::SetMoveWindow( BOOL bMoveWindow )
{
	m_Property.bUseInput	=	bMoveWindow;
	m_Property.bMovable		=	bMoveWindow;
}

VOID AcUIToolTip::EnableExitButton( BOOL bEnable )
{
	if( m_bEnableExit == bEnable )
		return;

	if( bEnable )
	{
		_InitExitButton();
	}

	AcUIEventButton*	pEventButton	=	static_cast< AcUIEventButton* >(m_pExitButton);

	if( bEnable )
		AddChild( pEventButton  );
	else
		DeleteChild( pEventButton , FALSE );

	m_bEnableExit	=	bEnable;

}

VOID AcUIToolTip::_InitExitButton( VOID )
{
	if( !m_pExitButton )
	{
		m_pExitButton	=	new AcUIEventButton;
	}

	AcUIEventButton*	pEventButton	=	static_cast< AcUIEventButton* >(m_pExitButton);

	pEventButton->m_Property.bMovable	=	FALSE;

	pEventButton->ClearImage();

	pEventButton->SetButtonEnable( TRUE );
	pEventButton->AddButtonImage( EXIT_BUTTON_IMAGE_NORMAL , ACUIBUTTON_MODE_NORMAL );
	pEventButton->AddButtonImage( EXIT_BUTTON_IMAGE_OVER	, ACUIBUTTON_MODE_ONMOUSE );
	pEventButton->AddButtonImage( EXIT_BUTTON_IMAGE_CLICK , ACUIBUTTON_MODE_CLICK );

	pEventButton->SetCallbackClickEvent( this , CBTooltipExit );
}

VOID AcUIToolTip::_UpdateExitButtonPosition( VOID )
{
	if( !m_bEnableExit )
		return;

	AcUIEventButton*	pEventButton	=	static_cast< AcUIEventButton* >(m_pExitButton);
	if( !pEventButton )
		return;

	pEventButton->MoveWindow(	 (INT32)(this->w - pEventButton->m_fTextureWidth - 3) , 
								3	,
								 (INT32)(pEventButton->m_fTextureWidth)	,
								 (INT32)(pEventButton->m_fTextureHeight	));


}
BOOL AcUIToolTip::CBTooltipExit( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AcUIToolTip*	pThis	=	static_cast< AcUIToolTip* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->ShowWindow( FALSE );

	return TRUE;
}

VOID	AcUIToolTip::_DrawLine( INT32 nPosX , INT32 nPosY )
{
	if( !m_bEnableLineColor )
		return;

	// 윤곽 라인을 그린다
	g_pEngine->Draw2DPanel( nPosX-TOOLTIP_LINE_SIZE	, nPosY-TOOLTIP_LINE_SIZE	, w+TOOLTIP_LINE_SIZE*2	, TOOLTIP_LINE_SIZE		, m_dwLineColor );
	g_pEngine->Draw2DPanel( nPosX+w					, nPosY-TOOLTIP_LINE_SIZE	, TOOLTIP_LINE_SIZE		, h+TOOLTIP_LINE_SIZE*2	, m_dwLineColor );
	g_pEngine->Draw2DPanel( nPosX-TOOLTIP_LINE_SIZE	, nPosY-TOOLTIP_LINE_SIZE	, TOOLTIP_LINE_SIZE		, h+TOOLTIP_LINE_SIZE*2	, m_dwLineColor );
	g_pEngine->Draw2DPanel( nPosX-TOOLTIP_LINE_SIZE	, nPosY+h					, w+TOOLTIP_LINE_SIZE*2	, TOOLTIP_LINE_SIZE		, m_dwLineColor );
		
}


RwV2d	AcUIToolTip::GetRenderPos( UINT*	unTooltipStatus , INT nOffsetX , INT nOffsetY )
{
	int		nAbsolute_x = 0;
	int		nAbsolute_y = 0;
	RwV2d	vPos;

	if( nOffsetX >= 0 )
		m_nOffsetX	=	nOffsetX;

	if( nOffsetY >= 0 )
		m_nOffsetY	=	nOffsetY;

	ClientToScreen( &nAbsolute_x, &nAbsolute_y );

	if( unTooltipStatus )
	{
		*unTooltipStatus	=	ACUITOOLTIP_RIGHT_TOOLTIP;
	}

	// 움직이지 않는 툴팁만 위치 조정을 한다.
	if( !m_Property.bMovable )
	{
		// 툴팁 윈도우가 화면 밖으로 넘어가는지 검사해서 넘어가면 안쪽으로 들어오게 처리해준다.
		if( pParent )
		{
			if( nAbsolute_x + w + m_nOffsetX > pParent->w ) 
			{
				nAbsolute_x = nAbsolute_x -w;
				//nAbsolute_x = pParent->w - ( w );

				if( unTooltipStatus )
				{
					if( nAbsolute_x > 0 )
					{
						*unTooltipStatus = ACUITOOLTIP_LEFT_TOOLTIP;
					}
					else
					{
						nAbsolute_x = pParent->w - w;

						if( nAbsolute_x < 0 )
							nAbsolute_x = 0;
					}
				}
				else
				{
					if( nAbsolute_x <= 0 )
					{
						nAbsolute_x = pParent->w - w;

						if( nAbsolute_x < 0 )
							nAbsolute_x = 0;
					}
				}
			}

			if( nAbsolute_y + h + m_nOffsetY > pParent->h ) 
			{
				nAbsolute_y = nAbsolute_y - h;
				//nAbsolute_y = pParent->h - ( h );

				if( nAbsolute_y < 0 )
					nAbsolute_y = 0;
			}
		}
	}

	vPos.x	=	(FLOAT)nAbsolute_x;
	vPos.y	=	(FLOAT)nAbsolute_y;

	return vPos;
}