// AcUIBase.cpp: implementation of the AcUIBase class.
//
//////////////////////////////////////////////////////////////////////
#include "rwcore.h"
#include "rpworld.h"
#include <skeleton.h>
#include <menu.h>
#include <camera.h>

#include "AcUIBase.h"
#include "AgcmUIManager2.h"
#include <atltypes.h>

#define ACUIBASE_TEXT_ROW_MARGIN									4

AgcmUIControl* AcUIBase::m_pAgcmUIControl = NULL;
AgcmSound* AcUIBase::m_pAgcmSound = NULL;
AgcmFont* AcUIBase::m_pAgcmFont = NULL;
RwTexture* AcUIBase::m_pBaseTexture = NULL;

extern AgcEngine* g_pEngine;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
AcUIBase::AcUIBase( void )
: m_bBlink( FALSE ), m_lColorSecond( 0 ), m_lBlinkGap( 1000 ), m_bBold( FALSE )
{
	m_bUIWindow = TRUE;		// 이것은 UI Window이다. (AgcWindow에서 상속받은 AcUIBase라는 것을 알기 위해 사용
	m_bUseDrag = FALSE;
	m_bPassPostMessage = TRUE;
	m_bClipImage = FALSE;

	// 타입 세팅..
	m_nType	= TYPE_BASE;

	m_stRenderInfo.fStartU = 0.0f;
	m_stRenderInfo.fStartV = 0.0f;
	m_stRenderInfo.fEndU = 1.0f;
	m_stRenderInfo.fEndV = 1.0f;	
	m_stRenderInfo.lRenderColor = 0xffffffff;
	m_stRenderInfo.cRenderAlpha = 255;
	m_stRenderInfo.fScale = 1.0f;
	m_stRenderInfo.lRenderID = 1;
	m_stRenderInfo.lDefaultRenderID = 1;

	m_szStaticString = NULL;
	m_lStaticStringLength = 0;

	m_fScale = 0.0f;
	m_lColor = 0;
	m_bShadow =	FALSE;
	m_lFontType	= 0;

	m_bDrawStaticString	= TRUE;
	m_bExistHandler = FALSE;

	m_rectHandlerArea.x = 0;
	m_rectHandlerArea.y = 0;
	m_rectHandlerArea.w = 0;
	m_rectHandlerArea.h = 0;

	m_rectOriginalPos.x = 0;
	m_rectOriginalPos.y = 0;
	m_rectOriginalPos.w = 0;
	m_rectOriginalPos.h = 0;

	m_lCurrentStatusID = -1;
	m_lDefaultStatusID = -1;

	for( INT32 nStatusCount = 0 ; nStatusCount < ACUIBASE_STATUS_MAX_NUM ; ++nStatusCount )
	{
		memset( m_astStatus[ nStatusCount ].m_szStatusName, 0, sizeof( m_astStatus[ nStatusCount ].m_szStatusName ) );

		m_astStatus[ nStatusCount ].m_rectStatusPos.x = 0;
		m_astStatus[ nStatusCount ].m_rectStatusPos.y = 0;
		m_astStatus[ nStatusCount ].m_rectStatusPos.w = 0;
		m_astStatus[ nStatusCount ].m_rectStatusPos.h = 0;
		m_astStatus[ nStatusCount ].m_lStatusImageID =	0;
		m_astStatus[ nStatusCount ].m_lStatusID = -1;
		m_astStatus[ nStatusCount ].m_bSetting = FALSE;
		m_astStatus[ nStatusCount ].m_bVisible = TRUE;
	}

	m_bActiveStatus = TRUE;
	m_bDrawOutline = FALSE;

	memset( &m_stUserData, 0, sizeof( AcUIBaseData ) );
	m_bShrinkMode = FALSE;

	m_eHAlign = ACUIBASE_HALIGN_LEFT;
	m_eVAlign = ACUIBASE_VALIGN_TOP;
	m_lStringX = 0;
	m_lStringY = 0;

	m_lStringOffsetX = 0;
	m_lStringOffsetY = 0;

	m_lImageOffsetX	= 0;
	m_lImageOffsetY	= 0;
	m_lStringWidth = 0;

	if( m_pAgcmFont )
	{
		m_lStringHeight	= ( INT32 )( m_pAgcmFont->m_astFonts[ m_lFontType ].lSize * m_fScale ) + ACUIBASE_TEXT_ROW_MARGIN;
	}

	m_bImageNumber			= FALSE;
	m_bAutoFitString		= FALSE;
	m_bAutoClippingString	= FALSE;
	m_bStringNumberComma	= FALSE;
	m_pfAlpha				= NULL;
	m_bAnimation			= FALSE;
	m_bAnimationDefault		= FALSE;
	m_pAnimationData		= NULL;
	m_pCurrentAnim			= NULL;
	m_ulClockCount			= 0	;
	m_ulPrevClockCount		= 0	;
	m_szTooltip				= NULL;
	m_bTooltipForFitString	= FALSE;

	m_bResizeWindow = FALSE;
	m_eDefaultRenderType	= UICM_BASE_DEFAULT_RENDER;
	m_dwDefaultRenderColor	= 0x50000000;
	m_dwSmoothTime			= 1000;
	m_dwStartTime			= 0;

	m_pfnMouseWheelCB		= NULL;
	m_pMouseWheelClass		= NULL;

	m_bSizeEx				= FALSE;
	m_bPosEx				= FALSE;

	m_bPreLoad = FALSE;
	m_bEnablePrevLoad = TRUE;

}

AcUIBase::~AcUIBase( void )
{
	if( m_szStaticString )
	{
		delete[] m_szStaticString;
		m_szStaticString = NULL;
	}

	if( m_csTextureList.GetCount() )
	{
		m_csTextureList.DestroyTextureList();
	}

	RemoveAllAnimation();
	ASSERT( 0 == m_csTextureList.GetCount() );
}

BOOL AcUIBase::OnCommand( INT32	nID, PVOID pParam )
{
	if( m_bPassPostMessage )
	{
		if( pParent ) 
		{
			pParent->SendMessage( MESSAGE_COMMAND, ( PVOID )nID, pParam );
		}
	}

	return TRUE;
}

VOID AcUIBase::PlayUISound( char* pszFileName )
{
	if( !m_pAgcmSound ) return;

	char szSoundFileName[ AGCMSOUND_SOUND_FILENAME_LENGTH ] = { 0, };
	strcpy( szSoundFileName, AGCMSOUND_SOUND_FILEPATH_UI );
	strncat( szSoundFileName, pszFileName, AGCMSOUND_SOUND_FILENAME_LENGTH );
}

INT32 AcUIBase::AddImage( char * filename, BOOL bLoad , BOOL bResize )
// 리턴값은 등록된 이미지의 시퀀스 인덱스.
{
	INT32 lImageID = m_csTextureList.AddImage( filename, bLoad );

	// 윈도우 크기 설정..
	RwTexture* pTexture = m_csTextureList.GetImage_ID( lImageID );

	if( pTexture && bResize )
	{
		if( pTexture->raster )
		{
			w = RwRasterGetWidth( pTexture->raster );
			h = RwRasterGetHeight( pTexture->raster );
		}
	}

	// virtual 함수 실행 
	OnAddImage( pTexture );
	if( m_csTextureList.GetCount() == 1 )
	{
		SetShrinkModeRenderInfo();
	}

	return ( lImageID - 1 );
}

VOID AcUIBase::DestroyImage( VOID )
{
	m_csTextureList.DestroyTextureList();
}

INT32 AcUIBase::AddOnlyThisImage( char * filename, BOOL bLoad )
{
	INT32 lImageID = m_csTextureList.AddOnlyThisImage( filename, bLoad );

	// 윈도우 크기 설정..
	RwTexture *pTexture = m_csTextureList.GetImage_ID( lImageID );
	if( pTexture )
	{
		if( pTexture->raster )
		{
			w = RwRasterGetWidth( pTexture->raster );
			h = RwRasterGetHeight( pTexture->raster );
		}
	}

	// virtual 함수 실행 
	OnAddImage( pTexture );

	return ( lImageID - 1 );
}

VOID AcUIBase::OnClose( void )
{
	if( pParent )
	{
		pParent->SendMessage( MESSAGE_COMMAND, ( PVOID )UICM_BASE_MESSAGE_CLOSE, ( PVOID ) &m_lControlID );
	}

	// 등록된 이미지 디스트로이..
	if( m_csTextureList.GetCount() )
	{
		m_csTextureList.DestroyTextureList();
	}
}

VOID AcUIBase::OnMoveWindow( void )
{
	switch( m_eHAlign )
	{
	case ACUIBASE_HALIGN_RIGHT :	m_lStringX = w - m_lStringWidth;				break;
	case ACUIBASE_HALIGN_CENTER :	m_lStringX = ( w - m_lStringWidth ) / 2;		break;
	default :						m_lStringX = 0;									break;
	}

	switch( m_eVAlign )
	{
	case ACUIBASE_VALIGN_BOTTOM :	m_lStringY = h - m_lStringHeight;				break;
	case ACUIBASE_VALIGN_CENTER :	m_lStringY = ( h - m_lStringHeight ) / 2;		break;
	default :						m_lStringY = 0;									break;
	}

	if( (m_bAutoFitString || m_bAutoClippingString) && m_lStringWidth > w )
	{
		MakeStringFit();
		m_lStringWidth = ( INT32 )( m_pAgcmFont->GetTextExtent( m_lFontType, m_szStaticString,strlen( m_szStaticString ) ) * m_fScale );
	}
}

BOOL AcUIBase::OnLButtonDown( RsMouseStatus *ms	)
{
	if( m_bExistHandler )
	{
		if( m_bShrinkMode || CheckInHandlerArea( ms ) )
		{
			return OnLButtonDownForHandler( ms );
		}
	}
	else 
	{
		if(m_Property.bResizable)
		{
			if(m_bResizeWindow == (BOOL)TRUE) return TRUE;
			if( _CheckMouseReSizeArea(ms))
			{
				m_bResizeWindow = TRUE;
				return TRUE;
			}
		}
		// Movable이면 Move시작
		if( m_Property.bMovable )
		{
			MovableOn( ms );
			return TRUE;
		}
	}

	return FALSE;
}

BOOL AcUIBase::OnLButtonUp( RsMouseStatus *ms )
{
	if(m_bResizeWindow) m_bResizeWindow = FALSE;
	if( m_bExistHandler )
	{
		if( m_bShrinkMode || CheckInHandlerArea( ms ) )
		{
			return OnLButtonUpForHandler( ms );
		}
	}
	else 
	{
		// 창이 이동후에 물린 콜백 실행( 도킹 )
		if( m_pfnDocking )
			m_pfnDocking( m_pClass , m_pData1 , m_pData2 );

		if( m_bMoveMovableWindow )
		{
			MovableOff();
			return TRUE;
		}
	}
	
	if( pParent && m_bActiveStatus )
	{
		if( pParent->SendMessage( MESSAGE_COMMAND, ( PVOID )UICM_BASE_MESSAGE_LCLICK, ( PVOID )&m_lControlID ) )
		{
			return FALSE;
		}
	}

	return FALSE;
}

BOOL AcUIBase::OnLButtonDblClk( RsMouseStatus *ms )
{
	if( m_bExistHandler )
	{
		if( m_bShrinkMode || CheckInHandlerArea( ms ) )
		{
			return OnLButtonDblClkForHandler( ms );
		}
	}

	if( pParent && m_bActiveStatus )
	{
		if( pParent->SendMessage( MESSAGE_COMMAND, ( PVOID )UICM_BASE_MESSAGE_LDBLCLICK, ( PVOID )&m_lControlID ) )
		{
			return FALSE;
		}
	}

	return FALSE;
}

BOOL AcUIBase::OnRButtonUp( RsMouseStatus *ms )
{
	if( pParent && m_bActiveStatus )
	{
		if( pParent->SendMessage( MESSAGE_COMMAND, ( PVOID )UICM_BASE_MESSAGE_RCLICK, ( PVOID )&m_lControlID ) )
		{
			return FALSE;
		}
	}

	return FALSE;
}

BOOL AcUIBase::OnRButtonDblClk( RsMouseStatus *ms )
{
	if( pParent && m_bActiveStatus )
	{
		if( pParent->SendMessage( MESSAGE_COMMAND, ( PVOID )UICM_BASE_MESSAGE_RDBLCLICK, ( PVOID )&m_lControlID ) )
		{
			return FALSE;
		}
	}

	return FALSE;
}

BOOL AcUIBase::OnLButtonDblClkForHandler( RsMouseStatus *ms )
{
	return TRUE;
}

BOOL AcUIBase::OnLButtonUpForHandler( RsMouseStatus *ms )		
{
	// 창이 이동후에 물린 콜백 실행( 도킹 )
	if( m_pfnDocking )
		m_pfnDocking( m_pClass , m_pData1 , m_pData2 );

	if( m_bMoveMovableWindow )
	{
		MovableOff();
		return TRUE;
	}

	return TRUE;
}

BOOL AcUIBase::OnLButtonDownForHandler( RsMouseStatus *ms )	
{
	// Movable이면 Move시작
	if( m_Property.bMovable )
	{
		MovableOn( ms );
		return TRUE;
	}

	return TRUE;
}

BOOL AcUIBase::OnMouseMove( RsMouseStatus *ms )
{
	if(m_bResizeWindow == (BOOL)TRUE)
	{
		return FALSE;
	}
	// Window Move중이라면 
	if( m_bMoveMovableWindow )
	{
		ClientToScreen( *ms );	
		MoveWindow( ( INT32 )( ms->pos.x - m_v2dDragPoint.x ), ( INT32 )( ms->pos.y - m_v2dDragPoint.y ), w, h );
		return FALSE;
	}

	return FALSE;
}

BOOL AcUIBase::OnMouseWheel( INT32 lDelta )
{
	if( m_pfnMouseWheelCB )
	{
		m_pfnMouseWheelCB( m_pMouseWheelClass , (PVOID)lDelta , NULL , NULL );
		return TRUE;
	}

	return FALSE;
}

VOID AcUIBase::OnWindowRender( void )
{
	PROFILE( "AcUIBase::OnWindowRender" );

	BOOL bRenderBaseTexture	= TRUE;
	BOOL bDrawStaticString = m_bDrawStaticString;

	_UpdateAnimation( &bRenderBaseTexture, &bDrawStaticString );

	switch( m_eDefaultRenderType )
	{
	case UICM_BASE_DEFAULT_RENDER:
		_RenderBaseTexture( bRenderBaseTexture );
		break;

	case UICM_BASE_RGBA_RENDER:
		_RenderRGBARender();
		break;

	case UICM_BASE_SMOOTH_RGBA_RENDER:
		_RenderSmoothRGBARender();
		break;

	}

	if( bDrawStaticString )
	{
		_RenderText();
	}

	if( m_bDrawOutline )
	{
		DrawOutline();
	}
}

BOOL AcUIBase::RenderTexture( INT32 lRenderID )
{
	PROFILE( "AcUIBase::OnWindowRender - RenderTexture()" );

	RwTexture* pTexture = m_stRenderInfo.lRenderID ? m_csTextureList.GetImage_ID( lRenderID ) : NULL;
	if( !pTexture || !pTexture->raster ) return FALSE;

	FLOAT fPosX = ( FLOAT )m_lAbsolute_x + m_lImageOffsetX;
	FLOAT fPosY = ( FLOAT )m_lAbsolute_y + m_lImageOffsetY;
	UINT8 nAlpha = GetAlpha();

	// 그려 보자꾸나 
	if( m_bShrinkMode )
	{
		g_pEngine->DrawIm2D( pTexture, fPosX + m_stRenderInfo.fOffsetX, fPosY + m_stRenderInfo.fOffsetY, w, h, m_stRenderInfo.fStartU, m_stRenderInfo.fStartV,
			m_stRenderInfo.fEndU, m_stRenderInfo.fEndV,	m_stRenderInfo.lRenderColor, nAlpha , m_Property.bClipping ? &m_rectClipping : NULL  );
	}
	else if( m_bClipImage )
	{
		INT32 lWidth = RwRasterGetWidth( pTexture->raster );
		INT32 lHeight = RwRasterGetHeight( pTexture->raster );
		FLOAT fU = 1.0f;
		FLOAT fV = 1.0f;

		if( lWidth * m_stRenderInfo.fScale > w )
		{
			fU = w / ( lWidth * m_stRenderInfo.fScale );
			lWidth = w;
		}

		if( lHeight * m_stRenderInfo.fScale > h )
		{
			fV = h / ( lHeight * m_stRenderInfo.fScale );
			lHeight = h;
		}

		g_pEngine->DrawIm2D( pTexture, fPosX + m_stRenderInfo.fOffsetX, fPosY + m_stRenderInfo.fOffsetY, ( FLOAT )lWidth, ( FLOAT )lHeight, 0.0f, 0.0f, fU, fV,
			m_stRenderInfo.lRenderColor, nAlpha , m_Property.bClipping ? &m_rectClipping : NULL  );

	}
	else
	{
		/*FLOAT fWidth = ( FLOAT )( RwRasterGetWidth( pTexture->raster ) ) * m_stRenderInfo.fScale;
		FLOAT fHeight = ( FLOAT )( RwRasterGetHeight( pTexture->raster ) ) * m_stRenderInfo.fScale;

		if( m_bSizeEx )
		{
			fWidth	=	w;
			fHeight	=	h;
		}

		g_pEngine->DrawIm2D( pTexture, fPosX + m_stRenderInfo.fOffsetX, fPosY + m_stRenderInfo.fOffsetY , fWidth, fHeight, m_stRenderInfo.fStartU, m_stRenderInfo.fStartV,
			m_stRenderInfo.fEndU, m_stRenderInfo.fEndV,	m_stRenderInfo.lRenderColor, nAlpha , m_Property.bClipping ? &m_rectClipping : NULL );*/
		RwRaster * camRaster = RwCameraGetRaster( g_pEngine->m_pCamera );

		if( camRaster )
		{
			float curScale = camRaster->height / AgcmUIManager2::GetModeHeight();

			if( w == camRaster->width && h == camRaster->height && (m_stRenderInfo.fScale == 1.0f || m_stRenderInfo.fScale == curScale) )
			{
				m_stRenderInfo.fScale = curScale;
				fPosX = (camRaster->width * 0.5f) - (AgcmUIManager2::GetModeWidth()*0.5f*m_stRenderInfo.fScale);

				g_pEngine->DrawIm2D( pTexture, 0, 0, (float)camRaster->width, (float)camRaster->height, m_stRenderInfo.fStartU, m_stRenderInfo.fStartV,
					m_stRenderInfo.fEndU, m_stRenderInfo.fEndV,	0x00000000, 255);
			}
		}

		FLOAT fWidth = ( FLOAT )( RwRasterGetWidth( pTexture->raster ) ) * m_stRenderInfo.fScale;
		FLOAT fHeight = ( FLOAT )( RwRasterGetHeight( pTexture->raster ) ) * m_stRenderInfo.fScale;

		if( strstr(m_csTextureList.GetImageName_ID( lRenderID ), "LG_Base_") != 0 ) // 캐선창 배경은 하드코딩
		{
			float wScale = camRaster->width / AgcmUIManager2::GetModeWidth();
			g_pEngine->DrawIm2D( pTexture, 0, 0, RwRasterGetWidth( pTexture->raster ) * wScale, fHeight, m_stRenderInfo.fStartU, m_stRenderInfo.fStartV,
				m_stRenderInfo.fEndU, m_stRenderInfo.fEndV,	m_stRenderInfo.lRenderColor, nAlpha );
		}
		else 
		{
			g_pEngine->DrawIm2D( pTexture, fPosX + m_stRenderInfo.fOffsetX, fPosY + m_stRenderInfo.fOffsetY , fWidth, fHeight, m_stRenderInfo.fStartU, m_stRenderInfo.fStartV,
				m_stRenderInfo.fEndU, m_stRenderInfo.fEndV,	m_stRenderInfo.lRenderColor, nAlpha , m_Property.bClipping ? &m_rectClipping : NULL );
		}
	}

	return TRUE;
}

VOID AcUIBase::OnAddImage( RwTexture* pTexture )
{
	return;
}

VOID AcUIBase::DrawOutline( void )
{
	int m_lAbsolute_x = 0;
	int m_lAbsolute_y = 0;
	ClientToScreen( &m_lAbsolute_x, &m_lAbsolute_y );

	m_rectClipping.right = m_lAbsolute_x + w + 1;
	m_rectClipping.bottom = m_lAbsolute_y + h + 1;
	
	_DrawOutLine( ( FLOAT )m_lAbsolute_x,			( FLOAT )m_lAbsolute_y,			( FLOAT )w,	1.0f );
	_DrawOutLine( ( FLOAT )m_lAbsolute_x,			( FLOAT )( m_lAbsolute_y + h ), ( FLOAT )w,	1.0f );
	_DrawOutLine( ( FLOAT )m_lAbsolute_x,			( FLOAT )m_lAbsolute_y,			1.0f,		( FLOAT )h );
	_DrawOutLine( ( FLOAT )( m_lAbsolute_x + w ),	( FLOAT )m_lAbsolute_y,			1.0f,		( FLOAT )h );
}

BOOL AcUIBase::OnInit( void )
{
	SetDefaultRenderTexture( m_stRenderInfo.lDefaultRenderID );
	SetStatus( m_lDefaultStatusID, FALSE );
	m_bAnimation = m_bAnimationDefault;

	if( pParent )
	{
		if( pParent->m_bUIWindow && ( ( AcUIBase* )pParent )->m_pfAlpha )
		{
			m_pfAlpha = ( ( AcUIBase* )pParent )->m_pfAlpha;
		}

		SendMessage( MESSAGE_COMMAND, ( PVOID )UICM_BASE_MESSAGE_INIT, &m_lControlID );
	}

	return TRUE;
}

VOID AcUIBase::SetRenderColor( DWORD lColor )
{
	m_stRenderInfo.lRenderColor = lColor;
}

VOID AcUIBase::SetRenderAlpha( UINT8 cAlpha )
{
	m_stRenderInfo.cRenderAlpha = cAlpha;
}

BOOL AcUIBase::SetRednerTexture( INT32 lID )
{
	if( lID >= 0 )
	{
		m_stRenderInfo.lRenderID = lID;
	}

	if( m_bShrinkMode )
	{
		// Shrink용 u, v setting 
		SetShrinkModeRenderInfo();
	}
	return TRUE;
}

VOID AcUIBase::SetRenderScale( FLOAT fScale )
{
	m_stRenderInfo.fScale = fScale;
}

VOID AcUIBase::SetDefaultRenderTexture( INT32 lID )
{
	m_stRenderInfo.lDefaultRenderID = lID;
}

DWORD AcUIBase::GetRenderColor( void )
{
	return m_stRenderInfo.lRenderColor;
}

UINT8 AcUIBase::GetRenderAlpha( void )
{
	return m_stRenderInfo.cRenderAlpha;
}

INT32 AcUIBase::GetRenderTexture( void )
{
	return m_stRenderInfo.lRenderID;
}

INT32 AcUIBase::GetDefaultRenderTexture( void )
{
	return m_stRenderInfo.lDefaultRenderID;
}

VOID AcUIBase::SetStaticStringExt( char* szString, FLOAT fScale, INT32 lFontType, DWORD lColor, bool bShadow, AcUIBaseHAlign eHAlign, AcUIBaseVAlign eVAlign, BOOL bImageNumber, BOOL bBold )
{
	m_bBold = bBold;
	m_fScale = fScale;
	if( m_lCurrentStatusID == -1 )
	{
		m_lColor = lColor;
	}
	else
	{
		m_lColor = m_astStatus[ m_lCurrentStatusID & ( ACUIBASE_STATUS_ID_BIT_CONSTANT - 1 ) ].m_lStringColor;
	}

	m_bShadow	= bShadow;
	m_lFontType	= lFontType;
	m_eHAlign	= eHAlign;
	m_eVAlign	= eVAlign;

	m_bImageNumber = bImageNumber;
	SetStaticString( szString );
}

VOID AcUIBase::SetStaticString( char* szString )
{
	if( szString )
	{
		if( m_bTooltipForFitString )
		{
			SetTooltip( NULL, FALSE );
		}

		if( m_szStaticString )
		{
			delete[] m_szStaticString;
			m_szStaticString = NULL;
		}

		if( !m_bStringNumberComma )
		{
			m_lStaticStringLength = strlen( szString );
			m_szStaticString = new CHAR[ m_lStaticStringLength + 3 ];

			//@{ 2006/05/02 burumal
			memset( m_szStaticString, 0, sizeof( CHAR ) * m_lStaticStringLength + 3 );
			memcpy( m_szStaticString, szString, sizeof( CHAR ) * m_lStaticStringLength );
			//@}
		}
		else
		{
			INT32 lIndex;
			INT32 lIndex2;
			INT32 lIndexConverted;
			INT32 lDigits = 0;
			INT32 lStringLength = strlen( szString );

			m_lStaticStringLength = lStringLength + lStringLength / 3 + 1;
			m_szStaticString = new CHAR[ m_lStaticStringLength + 3 ];

			//@{ 2006/05/02 burumal
			memset( m_szStaticString, 0, sizeof( CHAR ) * m_lStaticStringLength + 3 );
			//@}
			for( lIndex = 0, lIndexConverted = 0; lIndex < lStringLength; ++lIndex, ++lIndexConverted )
			{
				if( lIndexConverted >= m_lStaticStringLength ) break;
				if( szString[ lIndex ] < '0' || szString[ lIndex ] > '9' )
				{
					m_szStaticString[lIndexConverted] = szString[ lIndex ];
					lDigits = 0;
				}
				else
				{
					if( !lDigits )
					{
						for( lIndex2 = lIndex; szString[ lIndex2 ] >= '0' && szString[ lIndex2 ] <= '9'; ++lIndex2 )
						{
							++lDigits;
						}
					}
					else
					{
						--lDigits;
						if( !( lDigits % 3 ) )
						{
							m_szStaticString[ lIndexConverted++ ] = ',';
							if( lIndexConverted >= m_lStaticStringLength ) break;
						}
					}

					m_szStaticString[ lIndexConverted ] = szString[ lIndex ];
				}
			}

			m_szStaticString[ lIndexConverted ] = 0;
		}
	}

	m_lStringHeight	= ( INT32 )( m_pAgcmFont->m_astFonts[ m_lFontType ].lSize * m_fScale ) + ACUIBASE_TEXT_ROW_MARGIN;
	if( m_szStaticString )
	{
		m_lStringWidth = ( INT32 )( m_pAgcmFont->GetTextExtent( m_lFontType, m_szStaticString,strlen( m_szStaticString ) ) * m_fScale );

		// 길이 초과시 말줄임표 표시 (일단 막자)
		if( (m_bAutoFitString || m_bAutoClippingString) && m_lStringWidth > w )
		{
			MakeStringFit();
			m_lStringWidth = ( INT32 )( m_pAgcmFont->GetTextExtent( m_lFontType, m_szStaticString,strlen( m_szStaticString ) ) * m_fScale );
		}
	}

	OnMoveWindow();
}

VOID AcUIBase::MakeStringFit( CHAR *szString, INT32 lWidth )
{
	if( !szString )
	{
		szString = m_szStaticString;
	}

	if( lWidth == -1 )
	{
		lWidth = w;
	}

	if( lWidth > m_pAgcmFont->GetTextExtent( m_lFontType, szString, strlen( szString ) ) * m_fScale ) return;

	INT32 lStringLength = strlen( szString );
	INT32 lIndex;
	INT32 lStringEnd = 0;
	BOOL bHangul = FALSE;
	INT32 lCondensation = 0;
	const CHAR* szCondensation = "...";
	if( !m_bAutoClippingString )
		lCondensation = ( INT32 )( m_pAgcmFont->GetTextExtent( m_lFontType, ( CHAR* )szCondensation, strlen( szCondensation ) ) * m_fScale );

	for( lIndex = 0; lIndex < lStringLength; ++lIndex )
	{
		if( lWidth - lCondensation < m_pAgcmFont->GetTextExtent( m_lFontType, szString, lIndex ) * m_fScale ) break;

		lStringEnd = lIndex;
		if( lIndex < m_lStaticStringLength - 1 && ( szString[ lIndex ] & 0x80 ) )
		{
			++lIndex;
		}
	}

	INT32 CondensationSize = 0;
	if( !m_bAutoClippingString )
		CondensationSize = ( INT32 )strlen( szCondensation );

	if( lStringEnd + CondensationSize >= m_lStaticStringLength - 1 )
	{
		if( szString[ lStringEnd - 2 ] & 0x80 )
		{
			lStringEnd -= 2;
		}
		else
		{
			--lStringEnd;
		}
	}

// 	if( !m_szTooltip )
// 	{
// 		SetTooltip( szString, TRUE );
// 	}
	if( !m_bAutoClippingString )
		strcpy( szString + lStringEnd, szCondensation );
	else
		szString[lStringEnd] = '\0';
}

VOID AcUIBase::EnableHandler( BOOL bEnable )
{
	m_bExistHandler = bEnable;
}

VOID AcUIBase::SetHandler( INT32 lHandler_x, INT32 lHandler_y, INT32 lHandler_w, INT32 lHandler_h )
{
	m_bExistHandler = TRUE;

	m_rectHandlerArea.x = lHandler_x;
	m_rectHandlerArea.y = lHandler_y;
	m_rectHandlerArea.w = lHandler_w;
	m_rectHandlerArea.h = lHandler_h;
}

BOOL AcUIBase::SetShrinkMode( BOOL bShrink )
{
	// Invalid Check
	if( FALSE == m_bExistHandler ) return FALSE;
	if( m_bShrinkMode == bShrink ) return FALSE;

	if( FALSE == bShrink )	// Original Pos 로 Expand
	{
		m_bShrinkMode = bShrink;	
		MoveWindow( x - m_rectHandlerArea.x, y - m_rectHandlerArea.y, m_rectOriginalPos.w, m_rectOriginalPos.h );

		// Set Original Pos
		m_rectOriginalPos.x = 0;
		m_rectOriginalPos.y = 0;
		m_rectOriginalPos.w = 0;
		m_rectOriginalPos.h = 0;

		// Set Render UV Info
		m_stRenderInfo.fStartU = 0.0f;
		m_stRenderInfo.fStartV = 0.0f;
		m_stRenderInfo.fEndU = 1.0f;	
		m_stRenderInfo.fEndV = 1.0f;
	}
	else 
	{
		m_bShrinkMode = bShrink;

		// Set Original Pos
		m_rectOriginalPos.x = x;
		m_rectOriginalPos.y = y;
		m_rectOriginalPos.w = w;
		m_rectOriginalPos.h = h;

		// Shrink용 u, v setting 
		SetShrinkModeRenderInfo();	
		MoveWindow( x + m_rectHandlerArea.x, y + m_rectHandlerArea.y, m_rectHandlerArea.w, m_rectHandlerArea.h );
	}

	MoveChildByShrink( bShrink );
	return TRUE;
}

/*****************************************************************
*   Function : MoveChildByShrink
*   Comment  : Shrink 시 Child들을 다시 정돈한다 , Expand시에는 bShirink를 FALSE를 준다  
*   Date&Time : 2003-09-24, 오후 2:20
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
VOID AcUIBase::MoveChildByShrink( BOOL bShrink )
{
	INT32 lMove_x =	0;
	INT32 lMove_y =	0;

	AgcWindowNode* cur_node = m_listChild.head;
	AgcWindow* pWindow;

	if( FALSE == bShrink )		// Expand시 
	{	
		while( cur_node )
		{
			pWindow = cur_node->pWindow;
			pWindow->MoveWindow( pWindow->x + m_rectHandlerArea.x, pWindow->y + m_rectHandlerArea.y, pWindow->w, pWindow->h );

			// 영역내에 존재 여부로 Show TRUE, FALSE 여부 결정 
			if( pWindow->m_bVisibleChildWindowForShink )
			{
				pWindow->ShowWindow( TRUE );
			}
			else 
			{
				pWindow->ShowWindow( FALSE );
			}

			pWindow->m_bVisibleChildWindowForShink = FALSE ;
			cur_node = cur_node->next;
		}
	}
	else						// Shrink시  
	{
		while( cur_node )
		{
			pWindow = cur_node->pWindow;

			// 영역내에 존재 여부로 Show TRUE, FALSE 여부 결정 
			if( pWindow->x >= ( m_rectHandlerArea.x + x ) && pWindow->y >= ( m_rectHandlerArea.y + y ) &&
				( pWindow->w + pWindow->x ) <= ( m_rectHandlerArea.x + x + m_rectHandlerArea.w ) &&
				( pWindow->h + pWindow->y ) <= ( m_rectHandlerArea.y + y + m_rectHandlerArea.h ) )
			{
				if( pWindow->m_Property.bVisible )
				{
					pWindow->m_bVisibleChildWindowForShink = TRUE;
				}
			}
			else 
			{
				if( pWindow->m_Property.bVisible )
				{
					pWindow->m_bVisibleChildWindowForShink = TRUE;
				}

				pWindow->ShowWindow( FALSE );
			}

			// MoveWindow 
			pWindow->MoveWindow( pWindow->x - m_rectHandlerArea.x, pWindow->y - m_rectHandlerArea.y, pWindow->w, pWindow->h );
			cur_node = cur_node->next;
		}
	}
}

BOOL AcUIBase::CheckInHandlerArea( RsMouseStatus* ms )
{
	if( !m_bExistHandler ) return FALSE;
	if( ms->pos.x >= m_rectHandlerArea.x && ms->pos.x < ( m_rectHandlerArea.x + m_rectHandlerArea.w ) )
	{
		if( ms->pos.y >= m_rectHandlerArea.y && ms->pos.y < ( m_rectHandlerArea.y + m_rectHandlerArea.h ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

INT32 AcUIBase::SetStatusInfo( INT32 lPosx, INT32 lPosy, INT32 lPosw, INT32 lPosh, INT32 lImageID, CHAR* pszName, INT32 lStatusID, BOOL bVisible, DWORD lStringColor )
{
	// 빈 Index를 찾아내기 
	INT32 lVacantIndex = -1;

	if( lStatusID == -1 )
	{
		for( INT32 nStatusCount = 0 ; nStatusCount < ACUIBASE_STATUS_MAX_NUM ; ++nStatusCount )
		{
			if( FALSE == m_astStatus[ nStatusCount ].m_bSetting )
			{
				lVacantIndex = nStatusCount;
				break;
			}
		}

		if( -1 == lVacantIndex ) return -1;

		// Index에 집어 넣기 
		if( m_astStatus[ lVacantIndex ].m_lStatusID < 0 )
		{
			m_astStatus[ lVacantIndex ].m_lStatusID = lVacantIndex;
		}
		else 
		{
			m_astStatus[ lVacantIndex ].m_lStatusID += ACUIBASE_STATUS_ID_BIT_CONSTANT;
			if( ACUIBASE_STATUS_ID_MAX < m_astStatus[ lVacantIndex ].m_lStatusID )
			{
				m_astStatus[ lVacantIndex ].m_lStatusID = m_astStatus[ lVacantIndex ].m_lStatusID & ( ACUIBASE_STATUS_ID_BIT_CONSTANT - 1 );
			}
		}
	}
	else
	{
		lVacantIndex = lStatusID & ( ACUIBASE_STATUS_ID_BIT_CONSTANT - 1 );
		m_astStatus[lVacantIndex].m_lStatusID = lStatusID;
	}

	memset( m_astStatus[ lVacantIndex ].m_szStatusName, 0, sizeof( m_astStatus[lVacantIndex].m_szStatusName ) );
	strncpy( m_astStatus[ lVacantIndex ].m_szStatusName, pszName, ACUIBASE_STATUS_NAME_LENGTH - 1 );

	m_astStatus[ lVacantIndex ].m_rectStatusPos.x =	lPosx;
	m_astStatus[ lVacantIndex ].m_rectStatusPos.y =	lPosy;
	m_astStatus[ lVacantIndex ].m_rectStatusPos.w =	lPosw;
	m_astStatus[ lVacantIndex ].m_rectStatusPos.h =	lPosh;
	m_astStatus[ lVacantIndex ].m_lStatusImageID = lImageID;
	m_astStatus[ lVacantIndex ].m_bSetting = TRUE;
	m_astStatus[ lVacantIndex ].m_bVisible = bVisible;
	m_astStatus[ lVacantIndex ].m_lStringColor = lStringColor;

	return lVacantIndex;
}

BOOL AcUIBase::SetStatus( INT32 lStatusID, BOOL bSaveCurrent )
{
	if( lStatusID < 0 )
	{
		if( m_lDefaultStatusID < 0 ) return FALSE;
		lStatusID = m_lDefaultStatusID;
	}

	stStatusInfo* pstStatus = GetStatusInfo_ID( lStatusID );
	if( !pstStatus ) return FALSE;

	if( bSaveCurrent && m_lCurrentStatusID >= 0 )
	{
		stStatusInfo* pstPrevStatus = GetStatusInfo_ID( m_lCurrentStatusID );
		if( pstPrevStatus )
		{
			pstPrevStatus->m_rectStatusPos.x = x;
			pstPrevStatus->m_rectStatusPos.y = y;
			pstPrevStatus->m_rectStatusPos.w = w;
			pstPrevStatus->m_rectStatusPos.h = h;

			pstPrevStatus->m_lStatusImageID = m_stRenderInfo.lRenderID;

			pstPrevStatus->m_bVisible = m_Property.bVisible;
			pstPrevStatus->m_lStringColor = m_lColor;
		}
	}

	MoveWindow( pstStatus->m_rectStatusPos.x, pstStatus->m_rectStatusPos.y, pstStatus->m_rectStatusPos.w, pstStatus->m_rectStatusPos.h );
	SetRednerTexture( pstStatus->m_lStatusImageID );

	m_Property.bVisible	= pstStatus->m_bVisible;
	m_lColor = pstStatus->m_lStringColor;
	m_lCurrentStatusID = lStatusID;
	return TRUE;
}

stStatusInfo* AcUIBase::GetStatusInfo_ID( INT32 lStatusID )
{
	if( lStatusID < 0 ) return NULL;

	INT32 lArrayIndex = lStatusID & ( ACUIBASE_STATUS_ID_BIT_CONSTANT - 1 );

	// Check
	if( lArrayIndex >= ACUIBASE_STATUS_MAX_NUM || lArrayIndex < 0 )	return NULL;
	if( m_astStatus[ lArrayIndex ].m_lStatusID != lStatusID || FALSE == m_astStatus[ lArrayIndex ].m_bSetting )	return NULL;

	//return 
	return m_astStatus + lArrayIndex;
}

stStatusInfo* AcUIBase::GetStatusInfo_Index( INT32 lStatusIndex )
{
	if( lStatusIndex >= ACUIBASE_STATUS_MAX_NUM || lStatusIndex < 0 ) return NULL;
	if( m_astStatus[ lStatusIndex ].m_bSetting == FALSE ) return NULL;
	return &( m_astStatus[ lStatusIndex ] );
}

BOOL AcUIBase::DeleteStatusInfo_ID( INT32 lStatusID )
{
	if( lStatusID < 0 ) return FALSE;

	INT32 lArrayIndex = lStatusID & ( ACUIBASE_STATUS_ID_BIT_CONSTANT - 1 );

	// Check
	if( lArrayIndex >= ACUIBASE_STATUS_MAX_NUM || lArrayIndex < 0  ) return FALSE;	
	if( m_astStatus[ lArrayIndex ].m_bSetting == FALSE ) return FALSE;

	m_astStatus[ lArrayIndex ].m_bSetting = FALSE;
	return TRUE;
}

BOOL AcUIBase::DeleteStatusInfo_Index( INT32 lStatusIndex )		
{
	if( lStatusIndex >= ACUIBASE_STATUS_MAX_NUM || lStatusIndex < 0 ) return FALSE;
	if( m_astStatus[ lStatusIndex ].m_bSetting == FALSE ) return FALSE;

	m_astStatus[ lStatusIndex ].m_bSetting = FALSE;
	return TRUE;
}

BOOL AcUIBase::DeleteAllStatusInfo( void )
{
	for( INT32 nStatusCount = 0 ; nStatusCount < ACUIBASE_STATUS_MAX_NUM ; ++nStatusCount )
	{
		m_astStatus[ nStatusCount ].m_bSetting = FALSE;
	}

	return TRUE;
}

BOOL AcUIBase::PreTranslateInputMessage( RsEvent event, PVOID param )
{
	if( !m_bActiveStatus )
	{
		AgcWindowNode* cur_node = m_listChild.tail;
		RsMouseStatus* pMStatus	= ( RsMouseStatus* ) param;

		while( cur_node )
		{
			if( cur_node->pWindow->HitTest( ( INT32 ) pMStatus->pos.x, ( INT32 )pMStatus->pos.y ) )
			{
				if( cur_node->pWindow->PreTranslateInputMessage( event, param ) )
				{
					return TRUE;
				}
			}

			cur_node = cur_node->prev;
		}

		switch( event )
		{
		case rsKEYDOWN :
			{
				AcUIBase::OnKeyDown( ( RsKeyStatus* )param );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;

		case rsKEYUP :
			{
				AcUIBase::OnKeyUp( ( RsKeyStatus* )param );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;

		case rsCHAR :
			{
				AcUIBase::OnChar( ( char* )param , 0 );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;			

		case rsIMECOMPOSING	:
			{
				AcUIBase::OnIMEComposing( ( char* )param , 0 );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;

		case rsLEFTBUTTONDOWN :
			{
				ScreenToClient( *( RsMouseStatus* )param );
				AcUIBase::OnLButtonDown( ( RsMouseStatus* )param );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;

		case rsLEFTBUTTONUP	:
			{
				ScreenToClient( *( RsMouseStatus* )param );
				AcUIBase::OnLButtonUp( ( RsMouseStatus* )param );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;

		case rsLEFTBUTTONDBLCLK	:
			{
				ScreenToClient( *( RsMouseStatus* )param );
				AcUIBase::OnLButtonDblClk( ( RsMouseStatus* )param );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;

		case rsRIGHTBUTTONDOWN :
			{
				ScreenToClient( *( RsMouseStatus* )param );
				AcUIBase::OnRButtonDown( ( RsMouseStatus* )param );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;

		case rsRIGHTBUTTONUP :
			{
				ScreenToClient( *( RsMouseStatus* )param );
				AcUIBase::OnRButtonUp( ( RsMouseStatus* )param );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;

		case rsRIGHTBUTTONDBLCLK :
			{
				ScreenToClient( *( RsMouseStatus* )param );
				AcUIBase::OnRButtonDblClk( ( RsMouseStatus* )param );
				g_pEngine->m_pFocusedWindow	= this;
				return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

BOOL AcUIBase::SetUserData( PVOID pvData, INT32 lDataSize, INT32 lDataCount )
{
	m_stUserData.m_pvData = pvData;
	m_stUserData.m_lDataSize = lDataSize;
	m_stUserData.m_lCount = lDataCount;
	return OnSetUserData();
}

PVOID AcUIBase::GetUserData( INT32 *plDataSize, INT32 *plDataCount )
{
	if( plDataSize ) 
	{
		*plDataSize	= m_stUserData.m_lDataSize;
	}

	if( plDataCount )
	{
		*plDataCount = m_stUserData.m_lCount;
	}

	return m_stUserData.m_pvData;
}

VOID AcUIBase::SetShrinkModeRenderInfo( void )
{
	if ( !m_bExistHandler ) return;

	// 인단 현재 Texture를 가져온다 
	RwTexture* pTexture = m_stRenderInfo.lRenderID ? m_csTextureList.GetImage_ID( m_stRenderInfo.lRenderID ) : NULL;

	// Set Render UV Info 
	if( !pTexture )
	{
		m_stRenderInfo.fStartU = 0.0f;
		m_stRenderInfo.fStartV = 0.0f;
		m_stRenderInfo.fEndU = 0.0f;	
		m_stRenderInfo.fEndV = 0.0f;	
	}
	else 
	{
		FLOAT fImageWidth = RwRasterGetWidth( RwTextureGetRaster( pTexture ) ) * m_stRenderInfo.fScale;
		FLOAT fImageHeight = RwRasterGetHeight( RwTextureGetRaster( pTexture ) ) * m_stRenderInfo.fScale;

		m_stRenderInfo.fStartU = ( FLOAT )m_rectHandlerArea.x / fImageWidth;
		m_stRenderInfo.fStartV = ( FLOAT )m_rectHandlerArea.y / fImageHeight;
		m_stRenderInfo.fEndU = ( FLOAT )( m_rectHandlerArea.w + m_rectHandlerArea.x ) / fImageWidth;
		m_stRenderInfo.fEndV = ( FLOAT )( m_rectHandlerArea.h + m_rectHandlerArea.y ) / fImageHeight;
	}
}

/*****************************************************************
*   Function : OnSetFocus
*   Comment  : OnSetFocus
*   Date&Time : 2003-11-07, 오전 11:35
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
VOID AcUIBase::OnSetFocus( void )
{
	if( pParent )
	{
		pParent->SendMessage( MESSAGE_COMMAND, ( PVOID )UICM_BASE_MESSAGE_SET_FOCUS, ( PVOID )&m_lControlID );
	}
}

/*****************************************************************
*   Function : OnKillFocus
*   Comment  : OnKillFocus
*   Date&Time : 2003-11-07, 오전 11:35
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
VOID AcUIBase::OnKillFocus( void )
{
	if( pParent )
	{
		pParent->SendMessage( MESSAGE_COMMAND, ( PVOID )UICM_BASE_MESSAGE_KILL_FOCUS, ( PVOID )&m_lControlID );
	}

}

/*****************************************************************
*   Function : OnEndDrag
*   Comment  : OnEndDrag
*   Date&Time : 2003-09-18, 오후 9:00
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
BOOL AcUIBase::OnDragDrop( PVOID pParam1, PVOID pParam2 )
{
	// 처리 
	RsMouseStatus clMouseStatus	= *( ( RsMouseStatus* )pParam1 );
	ScreenToClient( clMouseStatus );

	m_stDragDropInfo.pSourceWindow = ( AcUIBase* )pParam2;
	m_stDragDropInfo.pTargetWindow = this;
	m_stDragDropInfo.lX	= ( INT32 )clMouseStatus.pos.x;
	m_stDragDropInfo.lY	= ( INT32 )clMouseStatus.pos.y;

	m_bDragDropMessageExist = TRUE;
	if( pParent && m_bActiveStatus )
	{
		return pParent->SendMessage( MESSAGE_COMMAND, ( PVOID )UICM_BASE_MESSAGE_DRAG_DROP, ( PVOID )&m_lControlID );
	}

	return FALSE;
}

AcUIDragInfo* AcUIBase::GetDragDropMessage( void )
{
	if( m_bDragDropMessageExist )
	{
		m_bDragDropMessageExist = FALSE;
		return &m_stDragDropInfo;
	}

	return NULL;
}

AcUIBaseAnimation* AcUIBase::AddAnimation( INT32 lTextureID, UINT32 ulClockCount, BOOL bRenderText, BOOL bRenderBaseTexture )
{
	AcUIBaseAnimation* pAnimData = new AcUIBaseAnimation;
	if( !pAnimData ) return NULL;

	pAnimData->m_lTextureID = lTextureID;
	pAnimData->m_ulClockCount = ulClockCount;
	pAnimData->m_bRenderString = bRenderText;
	pAnimData->m_bRenderBaseTexture = bRenderBaseTexture;

	if( !m_pAnimationData )
	{
		m_pAnimationData = pAnimData;
	}
	else
	{
		AcUIBaseAnimation* pTempData = m_pAnimationData;
		while( pTempData->m_pNext )
		{
			pTempData = pTempData->m_pNext;
		}

		pTempData->m_pNext = pAnimData;
	}

	return pAnimData;
}

VOID AcUIBase::RemoveAllAnimation( void )
{
	if( !m_pAnimationData )	return;

	AcUIBaseAnimation* pAnimData = m_pAnimationData;
	AcUIBaseAnimation* pAnimDataNext;

	while( pAnimData )
	{
		pAnimDataNext = pAnimData->m_pNext;
		delete pAnimData;

		pAnimData = pAnimDataNext;
	}

	m_pAnimationData = NULL;
	m_bAnimation = FALSE;
}

BOOL AcUIBase::RemoveAnimation( AcUIBaseAnimation *pAnimation )
{
	if( !m_pAnimationData || !pAnimation ) return FALSE;

	AcUIBaseAnimation* pAnimData = m_pAnimationData;
	while( pAnimData )
	{
		if( pAnimData->m_pNext == pAnimation )
		{
			pAnimData->m_pNext = pAnimation->m_pNext;
			delete pAnimation;

			return TRUE;
		}

		pAnimData = pAnimData->m_pNext;
	}

	return FALSE;
}

BOOL AcUIBase::StartAnimation( void )
{
	if( !m_pAnimationData )	return FALSE;

	m_pCurrentAnim = m_pAnimationData;
	m_ulClockCount = 0;
	m_ulPrevClockCount = 0;
	m_bAnimation = TRUE;

	return TRUE;
}

VOID AcUIBase::StopAnimation( void )
{
	m_bAnimation = FALSE;
}

VOID AcUIBase::SetTooltip( CHAR *szTooltip, BOOL bForFitString )
{
	if( szTooltip && m_szTooltip && !strcmp( szTooltip, m_szTooltip ) )	return;
	if( m_szTooltip )
	{
		delete[] m_szTooltip;
		m_szTooltip = NULL;
	}

	if( szTooltip )
	{		
		m_szTooltip = new CHAR[ strlen( szTooltip ) + 1 ];
		strcpy( m_szTooltip, szTooltip );
	}

	m_bTooltipForFitString = bForFitString;
}

void AcUIBase::CloseUI( void )
{
	AgcWindowNode* cur_node = m_listChild.head;
	while( cur_node )
	{
		// 윈도우 찾음.
		AcUIBase* pcUIBase = ( AcUIBase* ) cur_node->pWindow;
		pcUIBase->CloseUI();
		cur_node = cur_node->next;
	}

	OnCloseUI();
}

DWORD AcUIBase::GetColor( void )
{
	if( !(m_bBlink & TEXT_BLINK) )
		return m_lColor;

	DWORD uCurrentTime = ::GetTickCount();
	uCurrentTime = uCurrentTime % m_lBlinkGap;

	FLOAT fTime = ( FLOAT )uCurrentTime / ( FLOAT ) m_lBlinkGap;

	DWORD dFirst = m_lColor < m_lColorSecond ? m_lColor : m_lColorSecond;
	DWORD dSecond = m_lColor < m_lColorSecond ? m_lColorSecond : m_lColor;

	DWORD lColor = dFirst + ( DWORD )( ( double )( dSecond - dFirst ) * ( sin( DEF_2PI * fTime ) + 1 ) / 2 );
	return lColor;
}

UINT8 AcUIBase::GetAlpha()
{
	UINT8 nAlpha = ( UINT8 )( m_stRenderInfo.cRenderAlpha * ( m_pfAlpha ? *m_pfAlpha : 1 ) );

	if( !(m_bBlink & TEXTURE_BLINK) )
		return nAlpha;

	DWORD uCurrentTime = ::GetTickCount();
	uCurrentTime = uCurrentTime % m_lBlinkGap;

	FLOAT fTime = ( FLOAT )uCurrentTime / ( FLOAT ) m_lBlinkGap;

	UINT8 dFirst = (UINT8)(m_lColor < m_lColorSecond ? m_lColor : m_lColorSecond);
	UINT8 dSecond = (UINT8)(m_lColor < m_lColorSecond ? m_lColorSecond : m_lColor);

	nAlpha = dFirst + ( UINT8 )( ( double )( dSecond - dFirst ) * ( sin( DEF_2PI * fTime ) + 1 ) / 2 );

	return nAlpha;
}

VOID	AcUIBase::_RenderRGBARender( VOID )
{
	INT		nAbsolute_x		=	0;
	INT		nAbsolute_y		=	0;
	DWORD	dwAlpha			=	0;
	DWORD	dwRenderColor	=	0;

	ClientToScreen( &nAbsolute_x, &nAbsolute_y );

	dwAlpha			=	(DWORD)( (m_dwDefaultRenderColor >> 24) * (m_pfAlpha ? *m_pfAlpha : 1) );
	dwRenderColor	=	(m_dwDefaultRenderColor & 0x00ffffff) +	(dwAlpha << 24);

	g_pEngine->Draw2DPanel( nAbsolute_x , nAbsolute_y , w , h , dwRenderColor );	
}

VOID	AcUIBase::_RenderSmoothRGBARender( VOID )
{
	if( !m_dwStartTime )
		return;

	INT		nAbsolute_x	=	0;
	INT		nAbsolute_y	=	0;

	ClientToScreen( &nAbsolute_x, &nAbsolute_y );

	DWORD		dwCurrentTick	=	g_pEngine->GetTickCount();
	DWORD		dwRest			=	dwCurrentTick - m_dwStartTime;
	if( dwRest > 1000 )
		dwRest = 1000;

	FLOAT		fRest			=	(FLOAT)dwRest / (FLOAT)m_dwSmoothTime;
	DWORD		dwAlpha			=	(DWORD)((m_dwDefaultRenderColor >> 24) * fRest);
	DWORD		dwRenderColor	=	(m_dwDefaultRenderColor & 0x00ffffff) +	(dwAlpha << 24);

	// 안을 채운다
	g_pEngine->Draw2DPanel( nAbsolute_x , nAbsolute_y , w , h , dwRenderColor );

	dwAlpha			=	(DWORD)(255 * fRest);
	dwRenderColor	=	(m_dwDefaultRenderColor & 0x00ffffff) +	(dwAlpha << 24);

	g_pEngine->Draw2DPanel( nAbsolute_x-1	, nAbsolute_y-1	, w+1*2	, 1		, dwRenderColor );
	g_pEngine->Draw2DPanel( nAbsolute_x+w	, nAbsolute_y-1	, 1		, h+1*2	, dwRenderColor );
	g_pEngine->Draw2DPanel( nAbsolute_x-1	, nAbsolute_y-1	, 1		, h+1*2	, dwRenderColor );
	g_pEngine->Draw2DPanel( nAbsolute_x-1	, nAbsolute_y+h	, w+1*2	, 1		, dwRenderColor );
}

void AcUIBase::_RenderBaseTexture( BOOL bRenderBaseTexture )
{
	m_lAbsolute_x = 0;
	m_lAbsolute_y = 0;
	ClientToScreen( &m_lAbsolute_x, &m_lAbsolute_y );

	if( m_csTextureList.GetCount() > 0 && bRenderBaseTexture )
	{
		RenderTexture( m_stRenderInfo.lRenderID );
	}
	else if( m_pBaseTexture && m_clProperty.bRenderBlank )
	{
		PROFILE( "AcUIBase::OnWindowRender - DrawBlank" );
		g_pEngine->DrawIm2D( m_pBaseTexture, ( FLOAT )m_lAbsolute_x, ( FLOAT )m_lAbsolute_y, 
			w, h, 0.0f, 0.0f, 1.0f, 1.0f, m_stRenderInfo.lRenderColor, 
			( UINT8 )( m_stRenderInfo.cRenderAlpha * ( m_pfAlpha ? *m_pfAlpha : 1 ) ) , m_Property.bClipping ? &m_rectClipping : NULL  );
	}

	if( m_bAnimation && m_pCurrentAnim )
	{
		RenderTexture( m_pCurrentAnim->m_lTextureID );
	}
}

void AcUIBase::_UpdateAnimation( BOOL* pbRenderBaseTexture, BOOL* pbDrawStaticString )
{
	if( !pbRenderBaseTexture || !pbDrawStaticString ) return;

	if( m_bAnimation && m_pAnimationData )
	{
		UINT32 ulClockCount	= GetTickCount();

		if( !m_ulPrevClockCount )
		{
			m_ulPrevClockCount	= ulClockCount;
		}

		m_ulClockCount += ulClockCount - m_ulPrevClockCount;
		m_ulPrevClockCount = ulClockCount;

		if( !m_pCurrentAnim )
		{
			m_ulClockCount = 0;
			m_pCurrentAnim = m_pAnimationData;
		}
		else
		{
			while( TRUE )
			{
				if( m_pCurrentAnim->m_ulClockCount >= m_ulClockCount ) break;
				if( !m_pCurrentAnim->m_pNext )
				{
					m_ulClockCount -= m_pCurrentAnim->m_ulClockCount;
					m_pCurrentAnim = m_pAnimationData;
				}
				else
				{
					m_pCurrentAnim	= m_pCurrentAnim->m_pNext;
				}
			}
		}

		*pbDrawStaticString = m_pCurrentAnim->m_bRenderString;
		*pbRenderBaseTexture = m_pCurrentAnim->m_bRenderBaseTexture;
	}
}

void AcUIBase::_RenderText( void )
{
	PROFILE("AcUIBase::OnWindowRender - DrawText");
	DWORD lColor = GetColor();

	if( m_szStaticString && strlen( m_szStaticString ) && m_pAgcmFont )
	{
		UINT8 nAlpha = ( UINT8 )( ( ( lColor >> 24 ) & 0xff ) * ( m_pfAlpha ? *m_pfAlpha : 1 ) );

		if( m_bImageNumber )
		{
			m_pAgcmFont->DrawTextImgFont(	( FLOAT )( m_lAbsolute_x + m_lStringX + m_lStringOffsetX ),
				( FLOAT )( m_lAbsolute_y + m_lStringY + m_lStringOffsetY ),
				m_szStaticString, m_lFontType,
				( UINT8 )( ( lColor >> 16 ) & 0xff ),
				( UINT8 )( ( lColor >> 8 ) & 0xff ),
				( UINT8 )( ( lColor >> 0 ) & 0xff ), nAlpha );
		}
		else
		{
			AgcdFontClippingBox		stClippingBox;

			stClippingBox.x	=	(FLOAT)m_rectClipping.left;
			stClippingBox.y	=	(FLOAT)m_rectClipping.top;
			stClippingBox.w	=	(FLOAT)(m_rectClipping.right  - m_rectClipping.left);
			stClippingBox.h	=	(FLOAT)(m_rectClipping.bottom - m_rectClipping.top);

			m_pAgcmFont->FontDrawStart( m_lFontType );

			if( m_Property.bClipping )
				m_pAgcmFont->SetClippingArea( &stClippingBox );

			m_pAgcmFont->DrawTextIM2DScale( ( FLOAT )( m_lAbsolute_x + m_lStringX + m_lStringOffsetX ), 
				( FLOAT )( m_lAbsolute_y + m_lStringY + m_lStringOffsetY ), 
				m_fScale * m_stRenderInfo.fScale, m_szStaticString, m_lFontType, 
				nAlpha,	lColor, m_bShadow ? true : false , -1, m_Property.bClipping/*, m_bBold */);
			m_pAgcmFont->FontDrawEnd();

			m_pAgcmFont->SetClippingArea( NULL );
		}
	}
}

void AcUIBase::_DrawOutLine( FLOAT fPosX, FLOAT fPosY, FLOAT fWidth, FLOAT fHeight )
{
	if( g_pEngine )
	{
		g_pEngine->DrawIm2D( NULL, fPosX, fPosY, fWidth, fHeight, 0.0f, 0.0f, 1.0f, 1.0f, 0x00ffff00, 255 , &m_rectClipping  );
	}
}

VOID AcUIBase::SetDefaultRenderType( eDefaultRenderType eType )
{
	m_eDefaultRenderType	=	eType;
}

VOID AcUIBase::SetRenderRGBAColor( DWORD dwColor )
{
	m_dwDefaultRenderColor	=	dwColor;
}

VOID AcUIBase::SetRenderSmoothTime( DWORD dwTick )
{
	m_dwSmoothTime	=	dwTick;
}

VOID AcUIBase::SetRenderSmoothStartTime( DWORD dwTick )
{
	m_dwStartTime	=	dwTick;
}

INT AcUIBase::SetBlink( eBlinkType bBlink , DWORD dColor /*= 0 */, DWORD dGap /*= 300 */ )
{
	m_lColorSecond = dColor; 
	m_lBlinkGap = dGap;

	if( bBlink == NONE_BLINK )
		m_bBlink = NONE_BLINK;
	else
		m_bBlink |= bBlink;

	return m_bBlink;
}
BOOL AcUIBase::_CheckMouseReSizeArea(const RsMouseStatus *ms)
{
	AcUIBase *	pcsWindow = this;
	if(!pcsWindow) return FALSE;;

	INT32	lStartX = 0;
	INT32	lStartY = 0;
	INT32	lEndX = pcsWindow ? pcsWindow->w : 0;
	INT32	lEndY = pcsWindow ? pcsWindow->h : 0;
	INT32	lMidX;
	INT32	lMidY;


	lMidX = (lStartX + lEndX) / 2;
	lMidY = (lStartY + lEndY) / 2;

	CRect Left_Top, Center_Top, Right_Top;
	CRect Left_Center, Right_Center;
	CRect Left_Bottom, Center_Bottom, Right_Bottom;

	Left_Top = CRect( lStartX - RESIZE_BOX_SIZE, lStartY - RESIZE_BOX_SIZE, RESIZE_BOX_SIZE * 2, RESIZE_BOX_SIZE * 2 );
	Center_Top = CRect( lMidX - RESIZE_BOX_SIZE, lStartY - RESIZE_BOX_SIZE, RESIZE_BOX_SIZE * 2, RESIZE_BOX_SIZE * 2 );
	Right_Top = CRect( lEndX - RESIZE_BOX_SIZE, lStartY - RESIZE_BOX_SIZE, RESIZE_BOX_SIZE * 2, RESIZE_BOX_SIZE * 2 );

	Left_Center = CRect( lStartX - RESIZE_BOX_SIZE, lMidY - RESIZE_BOX_SIZE, RESIZE_BOX_SIZE * 2, RESIZE_BOX_SIZE * 2 );
	Right_Center = CRect( lEndX - RESIZE_BOX_SIZE, lMidY - RESIZE_BOX_SIZE, RESIZE_BOX_SIZE * 2, RESIZE_BOX_SIZE * 2 );

	Left_Bottom = CRect( lStartX - RESIZE_BOX_SIZE, lEndY - RESIZE_BOX_SIZE, RESIZE_BOX_SIZE * 2, RESIZE_BOX_SIZE * 2 );
	Center_Bottom = CRect( lMidX - RESIZE_BOX_SIZE, lEndY - RESIZE_BOX_SIZE, RESIZE_BOX_SIZE * 2, RESIZE_BOX_SIZE * 2 );
	Right_Bottom = CRect( lEndX - RESIZE_BOX_SIZE, lEndY - RESIZE_BOX_SIZE, RESIZE_BOX_SIZE * 2, RESIZE_BOX_SIZE * 2 );

	//left_top
	if((ms->pos.x < lStartX || ms->pos.x > lEndX) && (ms->pos.y < lStartY || ms->pos.y > lEndY))
	{
		return FALSE;
	}
	else if( ms->pos.x >= Left_Top.left && ms->pos.x <= Left_Top.left + Left_Top.right  && ms->pos.y >= Left_Top.top 
		&& ms->pos.y <= Left_Top.top + Left_Top.bottom )
	{
		return TRUE;
	}
	//center_top
	else if( ms->pos.x >= Center_Top.left && ms->pos.x <= Center_Top.left + Center_Top.right
		&& ms->pos.y >= Center_Top.top  && ms->pos.y <= Center_Top.top + Center_Top.bottom )
	{
		return TRUE;

	}
	//right_top
	else if( ms->pos.x >= Right_Top.left && ms->pos.x <= Right_Top.left + Right_Top.right
		&& ms->pos.y >= Right_Top.top  && ms->pos.y <= Right_Top.top + Right_Top.bottom )
	{
		return TRUE;

	}
	//left_center
	else if( ms->pos.x >= Left_Center.left  && ms->pos.x <= Left_Center.left + Left_Center.right
		&& ms->pos.y >= Left_Center.top && ms->pos.y <= Left_Center.top + Left_Center.bottom )
	{
		return TRUE;

	}
	//right_center
	else if( ms->pos.x >= Right_Center.left  && ms->pos.x <= Right_Center.left + Right_Center.right 
		&& ms->pos.y >= Right_Center.top && ms->pos.y <= Right_Center.top + Right_Center.bottom )
	{
		return TRUE;

	}
	//left_bottom
	else if( ms->pos.x >= Left_Bottom.left && ms->pos.x <= Left_Bottom.left + Left_Bottom.right
		&& ms->pos.y >= Left_Bottom.top && ms->pos.y <= Left_Bottom.top + Left_Bottom.bottom )
	{
		return TRUE;

	}
	//center_bottom
	else if( ms->pos.x >= Center_Bottom.left && ms->pos.x <= Center_Bottom.left + Center_Bottom.right 
		&& ms->pos.y >= Center_Bottom.top && ms->pos.y <= Center_Bottom.top + Center_Bottom.bottom )
	{
		return TRUE;

	}
	//right_bottom
	else if( ms->pos.x >= Right_Bottom.left && ms->pos.x <= Right_Bottom.left + Right_Bottom.right
		&& ms->pos.y >= Right_Bottom.top && ms->pos.y <= Right_Bottom.top + Right_Bottom.bottom )
	{
		return TRUE;
	}
	return FALSE;
}