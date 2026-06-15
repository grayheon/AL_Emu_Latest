// AcUIMinimap.cpp: implementation of the AcUIMinimap class.
//
//////////////////////////////////////////////////////////////////////

#include "AgcmMinimap.h"
#include "AcUIMinimap.h"
#include "AgcEngine.h"
#include "Apmmap.h"
#include "AgcmTargeting.h"
#include "AgcmMap.h"
#include "AgcmUIChatting2.h"


#define	 MINIMAP_ALPHA_TEXTURE		"Main_Minimap_Alpha.png"

// 맵 확대할수 있는 최대 값
#define	MAP_ENLARGEMENT_MAX			1.600000000f

// 맵 축소할수 있는 최대 값
#define  MAP_REDUCTION_MAX			0.400000000f

// 맵 축소/확대을 할 때 한번에 변하는 값
#define  MAP_INCREASE_DATA			0.200000000f


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AcUIMinimap::AcUIMinimap()
{
	m_pAgcmMinimap		= NULL;
	m_pAgpmCharacter	= NULL;

	m_nXPos	= 0;
	m_nZPos	= 0;

	m_nDivisionIndex	= -1;

	m_pRenderTexture	=	NULL;
	m_lDisplayCount		=	0;

	m_fMapSizeOffset	=	1.0f;
	m_pRenderCamera		=	NULL;
}

AcUIMinimap::~AcUIMinimap()
{

}

void AcUIMinimap::OnWindowRender	()
{
	PROFILE("AcUIMinimap::OnWindowRender");

	if( !m_pAgcmMinimap->GetMinimapON() )
		return;

	if (!this->m_pAgcmMinimap->m_pcsSelfCharacter)
		return;

	if( !m_pRenderTexture || !m_pMaskTex )
		return;

	// 맵을 실제로 그린다
	INT32 nStartX	=	0;
	INT32 nStartZ	=	0;

	this->ClientToScreen(&nStartX, &nStartZ);

	INT	nOffsetX	=	( (INT)(this->w * m_fMapSizeOffset) - ACUIMINIMAP_TEXTURE_SIZE ) / 2;
	INT nOffsetY	=	( (INT)(this->h * m_fMapSizeOffset) - ACUIMINIMAP_TEXTURE_SIZE ) / 2;

	g_pEngine->DrawIm2DPixelMask( m_pRenderTexture , m_pMaskTex , nStartX , nStartZ , this->w , this->h , -nOffsetX , -nOffsetY , ACUIMINIMAP_TEXTURE_SIZE+nOffsetX , ACUIMINIMAP_TEXTURE_SIZE+nOffsetY );

	// 커서를 그린다
	RenderCursor();

	// 퀘스트 팝업
	RenderDisplayPopup();

	// 지정된 위치 팝업
	RenderDisplayRegisterPopup();
}

BOOL AcUIMinimap::OnInit			()
{
	w = 150;
	h = 150;
	m_fCurrentTurnY	= 0.0f;

	_RenderCameraCreate();

	return TRUE;
}


BOOL AcUIMinimap::OnIdle( UINT32 ulClockCount )
{

	if( !m_pAgcmMinimap->GetMinimapON() )
		return TRUE;

	if (!this->m_pAgcmMinimap->m_pcsSelfCharacter)
		return TRUE;

	if( !m_pRenderTexture )
	{
		m_pRenderTexture	=	RwTextureCreate( RwCameraGetRaster(m_pRenderCamera) );
		RwTextureSetFilterMode( m_pRenderTexture , rwFILTERLINEAR );
		m_pMaskTex			=	RwTextureRead( MINIMAP_ALPHA_TEXTURE , NULL );

		if( m_pRenderTexture )
			RwTextureSetAddressing( m_pRenderTexture , rwTEXTUREADDRESSCLAMP );
		if( m_pMaskTex )
			RwTextureSetAddressing( m_pMaskTex , rwTEXTUREADDRESSCLAMP );
	}

	if( !m_pRenderTexture || !m_pMaskTex )
		return FALSE;

	RenderMinimap();
	RenderETC();
	RenderPartyMember();
	RenderRegisterPos();

	return TRUE;
}


void AcUIMinimap::OnClose			()
{
}

BOOL	AcUIMinimap::RenderMinimap	()
{
	if (this->m_pAgcmMinimap->m_pcsSelfCharacter == NULL)
		return TRUE;

	INT32	nDivisionIndex , nPartIndex;
	{
		FLOAT	fX = this->m_pAgcmMinimap->m_pcsSelfCharacter->m_stPos.x , fZ = this->m_pAgcmMinimap->m_pcsSelfCharacter->m_stPos.z;
		FLOAT	fLocalX , fLocalZ	;

		// 좌측위 픽셀의 디비젼 인덱스를 구함.
		fX	-= MAP_SECTOR_WIDTH * MAP_DEFAULT_DEPTH * GetTextureSize() / ( 2.0f * 2.0f * GetTextureSize() );
		fZ	-= MAP_SECTOR_WIDTH * MAP_DEFAULT_DEPTH * GetTextureSize() / ( 2.0f * 2.0f * GetTextureSize() );

		nDivisionIndex	= GetDivisionIndexF( fX , fZ );

		// 해당 디비젼에서의 옵셋을 구함..
		INT32	lFirstSectorIndexX	= ArrayIndexToSectorIndexX( GetFirstSectorXInDivision( nDivisionIndex ) );
		INT32	lFirstSectorIndexZ	= ArrayIndexToSectorIndexZ( GetFirstSectorZInDivision( nDivisionIndex ) );

		fLocalX = this->m_pAgcmMinimap->m_pcsSelfCharacter->m_stPos.x - GetSectorStartX( lFirstSectorIndexX );
		fLocalZ = this->m_pAgcmMinimap->m_pcsSelfCharacter->m_stPos.z - GetSectorStartZ( lFirstSectorIndexZ );

		INT32	nX , nZ;

		nX	= ( INT32 ) ( fLocalX / ( MAP_SECTOR_WIDTH * MAP_DEFAULT_DEPTH / 4.0f ) );
		nZ	= ( INT32 ) ( fLocalZ / ( MAP_SECTOR_WIDTH * MAP_DEFAULT_DEPTH / 4.0f ) );

		if( nX > 3 ) nX = 3;
		if( nZ > 3 ) nZ = 3;

		nPartIndex = nX + nZ * 4;
	}

	// nDivisionIndex , nPartIndex;

	FLOAT	fLocalX , fLocalZ	;
	INT32	lFirstSectorIndexX	= ArrayIndexToSectorIndexX( GetFirstSectorXInDivision( nDivisionIndex ) );
	INT32	lFirstSectorIndexZ	= ArrayIndexToSectorIndexZ( GetFirstSectorZInDivision( nDivisionIndex ) );
	fLocalX = GetSectorStartX( lFirstSectorIndexX );
	fLocalZ = GetSectorStartZ( lFirstSectorIndexZ );

	m_fWidth	= MAP_SECTOR_WIDTH * MAP_DEFAULT_DEPTH / 2.0f;

	INT32	nDVX				= GetDivisionXIndex( nDivisionIndex );
	INT32	nDVZ				= GetDivisionZIndex( nDivisionIndex );

	INT32	nDivisionRight			= MakeDivisionIndex( nDVX + 1	, nDVZ		);
	INT32	nDivisionBottom			= MakeDivisionIndex( nDVX		, nDVZ + 1	);
	INT32	nDivisionRightBottom	= MakeDivisionIndex( nDVX + 1	, nDVZ + 1	);
	INT32	nDivisionTemp			;
	INT32	nOffset					;

	INT32	nHalf				= GetTextureSize() >> 1;

	RwTexture	* pMinimapTexture = NULL;
	RwTexture	* pCurrentTexture = NULL;
	bool		bFirst = true;

	BOOL		bInvalidMapPosition = TRUE;

	for( int j = 0 ; j < 6 ; ++ j )
	{
		for( int i = 0 ; i < 5 ; ++ i )
		{
			// 디비젼 인덱스 체크.
			nDivisionTemp	= MakeDivisionIndex( nDVX + i / 2 , nDVZ + j / 2 );
			nOffset			= i % 2 + ( j % 2 ) * 2;
			pCurrentTexture = RenderDivision( nDivisionTemp , nOffset );

			if( pCurrentTexture )
			{
				if( bFirst  )
				{
					bFirst = false;
					pMinimapTexture = pCurrentTexture;
				}
				else
				{
					if( pMinimapTexture != pCurrentTexture )
						bInvalidMapPosition = FALSE;
				}
			}
		}
	}

	if( this->m_pAgcmMinimap->m_pcsSelfCharacter )
	{
		ApmMap::RegionTemplate*	pTemplate = this->m_pAgcmMinimap->m_pcsApmMap->GetTemplate( this->m_pAgcmMinimap->m_pcsSelfCharacter->m_nBindingRegionIndex );

		if( pTemplate && pTemplate->ti.stType.bDisableMinimap )
		{
			this->m_pAgcmMinimap->m_pcsAgcmUIManager2->ThrowEvent(this->m_pAgcmMinimap->m_lEventCloseMiniMapUI);
		}
	}

	return TRUE;
}

RwTexture *	AcUIMinimap::RenderDivision		( INT32 nDivisionIdnex , INT32 nPartIndex  )
{
	RwTexture * pTexture ;
	FLOAT fStartX	= __GetDivisionStartX( nDivisionIdnex , nPartIndex );
	FLOAT fStartZ	= __GetDivisionStartZ( nDivisionIdnex , nPartIndex );

	INT32	nX		=	GetMMPos( fStartX ) - m_nXPos + ( GetTextureSize() / 2) ;
	INT32	nZ		=	GetMMPos( fStartZ ) - m_nZPos + ( GetTextureSize() / 2) ;

	INT32	nStartX =	0;
	INT32	nStartZ	=	0;
	INT32	nEndX	=	0;
	INT32	nEndZ	=	0;
	INT32	nUStart =	0; 
	INT32	nVStart =	0;
	INT32	nUEnd	=	ACUIMINIMAP_MAP_SIZE;
	INT32	nVEnd	=	ACUIMINIMAP_MAP_SIZE;

	nStartX	= nX;
	nStartZ	= nZ;
	nEndX	= nStartX	+ ACUIMINIMAP_MAP_SIZE;
	nEndZ	= nStartZ	+ ACUIMINIMAP_MAP_SIZE;

	if( nStartX < 0 )
	{
		nUStart	= (-nStartX);
		nStartX	= 0;
	}

	if( nStartZ < 0 )
	{
		nVStart	= (-nStartZ );
		nStartZ = 0;
	}

	if( nEndX >= GetTextureSize() )
	{
		nUEnd	-= nEndX - GetTextureSize();
		nEndX	= GetTextureSize() ;
	}
	if( nEndZ >= GetTextureSize() )
	{
		nVEnd	-= nEndZ - GetTextureSize();
		nEndZ	= GetTextureSize();
	}

	pTexture = this->m_pAgcmMinimap->GetMinimapTexture( nDivisionIdnex , nPartIndex );
	RwRaster*	pRaster		=	RwCameraGetRaster( m_pRenderCamera );

	if( ( nEndX - nStartX ) > 0 && ( nEndZ - nStartZ ) > 0 && pTexture )
	{
		g_pEngine->DrawIm2DPixelRaster( pRaster						,
			pTexture					, 
			nStartX						, 
			nStartZ						, 
			nEndX - nStartX				,
			nEndZ - nStartZ				,
			nUStart						,			
			nVStart						,			
			nUEnd						,			
			nVEnd						,
			m_lColor					,
			( UINT8 ) ( ((m_lColor >> 24) & 0xff) * (m_pfAlpha ? *m_pfAlpha : 1)) );


		return pTexture;
	}
	else
	{
		return NULL;
	}
}

void	AcUIMinimap::PositionUpdate()
{
	if( NULL == this->m_pAgcmMinimap ) return;
	if( NULL == this->m_pAgcmMinimap->m_pcsSelfCharacter ) return;

	// 위치계산..
	m_nXPos	=	GetMMPos( this->m_pAgcmMinimap->m_pcsSelfCharacter->m_stPos.x );
	m_nZPos	=	GetMMPos( this->m_pAgcmMinimap->m_pcsSelfCharacter->m_stPos.z );                          
}

BOOL AcUIMinimap::OnMouseMove	( RsMouseStatus *ms	)
{
	m_prevMouseState = *ms;

	//ScreenToClient( stat );

	return FALSE;
}

BOOL AcUIMinimap::OnLButtonDown	( RsMouseStatus *ms	)
{
	INT32	nCenterX	=	( (INT)(this->w * m_fMapSizeOffset) >> 1 );
	INT32	nCenterZ	=	( (INT)(this->h * m_fMapSizeOffset) >> 1 );

	FLOAT	fX , fZ;
	fX = (FLOAT)GetMMPosINV( (INT32) (ms->pos.x*m_fMapSizeOffset + m_nXPos - nCenterX ));
	fZ = (FLOAT)GetMMPosINV( (INT32) (ms->pos.y*m_fMapSizeOffset + m_nZPos - nCenterZ) );

	char str[ 256 ];
	sprintf( str , "미니맵 클릭 (%f,%f)\n" , fX , fZ );
	TRACE( str );

	AgcmMap * pAgcmMap = ( AgcmMap * ) this->m_pAgcmMinimap->GetModule( "AgcmMap" );

	// 마고자 (2005-03-11 오후 3:09:47) : 
	// 패스파인딩 사용함.
	AuPOS pos;
	pos.x = fX;
	pos.z = fZ;
	if( pAgcmMap )
		pos.y = pAgcmMap->GetHeight( fX , fZ , SECTOR_MAX_HEIGHT );
	else
		pos.y = 0.0f;

	AgcmTargeting	*pAgcmTargeting = ( AgcmTargeting * ) this->m_pAgcmMinimap->GetModule( "AgcmTargeting" );
	ASSERT( NULL != pAgcmTargeting );
	if( pAgcmTargeting )
		pAgcmTargeting->MoveSelfCharacter( &pos );

	return FALSE;
}

BOOL AcUIMinimap::OnRButtonUp	( RsMouseStatus *ms	)
{
	if( g_pEngine->IsCtrlDown() )
		return FALSE;

	INT32	nCenterX	=	( (INT)(this->w * m_fMapSizeOffset) >> 1 );
	INT32	nCenterZ	=	( (INT)(this->h * m_fMapSizeOffset) >> 1 );

	FLOAT	fX , fZ;
	fX = (float)GetMMPosINV( (INT32) (ms->pos.x*m_fMapSizeOffset )+ m_nXPos - nCenterX );
	fZ = (float)GetMMPosINV( (INT32) (ms->pos.y*m_fMapSizeOffset) + m_nZPos - nCenterZ );

	char strMessage[ 256 ];
	sprintf( strMessage , "/move %f,%f" , fX , fZ );

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )m_pAgcmMinimap->GetModule( "AgcmUIChatting2" );
	if( pcmUIChatting )
	{
		pcmUIChatting->OnParseCommand( strMessage );
	}

	return FALSE;
}

BOOL AcUIMinimap::OnRButtonDblClk( RsMouseStatus *ms )
{
	INT32	nCenterX	=	( (INT)(this->w * m_fMapSizeOffset) >> 1 );
	INT32	nCenterZ	=	( (INT)(this->h * m_fMapSizeOffset) >> 1 );

	RwV2d	v2D;

	v2D.x = (RwReal)GetMMPosINV( (INT32) (ms->pos.x*m_fMapSizeOffset + m_nXPos - nCenterX) );
	v2D.y = (RwReal)GetMMPosINV( (INT32) (ms->pos.y*m_fMapSizeOffset + m_nZPos - nCenterZ) );

	// 해당 지역에 좌표가 있다면 삭제
	if( !DestroyMapPosition( &v2D ) )
	{
		// 해당 지역에 좌표가 없다면 생성
		m_pAgcmMinimap->OpenDlgAndAddPos( &v2D );
	}

	ZeroMemory( &m_prevMouseState , sizeof(m_prevMouseState) );

	return FALSE;
}

BOOL AcUIMinimap::DestroyMapPosition( RwV2d* pvPosition )
{
	RwTexture		*pRegisterPosTexture	=	m_pAgcmMinimap->GetRegisterPosTexture();
	AgpdCharacter	*pcsSelfCharacter	=	m_pAgcmMinimap->m_pcsSelfCharacter; 
	RwRaster*		pCameraRaster		=	RwCameraGetRaster( m_pRenderCamera );
	INT32			nX					=	0;
	INT32			nZ					=	0;
	INT32			nMouseX				=	0;
	INT32			nMouseZ				=	0;

	INT32			nOffsetX, nOffsetZ	;

	if( !pCameraRaster )
		return FALSE;

	if( pRegisterPosTexture && pcsSelfCharacter )
	{
		INT32	nCenterX		=	( (INT)(this->w * MAP_ENLARGEMENT_MAX) >> 1 );
		INT32	nCenterZ		=	( (INT)(this->h * MAP_ENLARGEMENT_MAX) >> 1 );
		INT		nMouseOffsetX	=	( (INT)(this->w * m_fMapSizeOffset) - ACUIMINIMAP_TEXTURE_SIZE ) / 2;
		INT 	nMouseOffsetY	=	( (INT)(this->h * m_fMapSizeOffset) - ACUIMINIMAP_TEXTURE_SIZE ) / 2;

		INT		nTextureSize	=	RwRasterGetWidth( RwTextureGetRaster(pRegisterPosTexture) );
		INT		nHalfTextureSize=	nTextureSize / 2;

		nMouseOffsetX	=	abs( nMouseOffsetX );
		nMouseOffsetY	=	abs( nMouseOffsetY );

		INT		nCount			= m_pAgcmMinimap->GetMapPositionCount();
		for( INT i = 0 ; i < nCount ; ++i )
		{
			stMapRegisterInfo*	pInfo	=	m_pAgcmMinimap->GetMapPositionIndex( i );
			if( !pInfo )
				continue;

			nX = GetMMPos( pInfo->m_vPosition.x ) - m_nXPos + nCenterX;
			nZ = GetMMPos( pInfo->m_vPosition.y ) - m_nZPos + nCenterZ;

			nMouseX = (INT)(m_prevMouseState.pos.x * m_fMapSizeOffset) + nMouseOffsetX;
			nMouseZ = (INT)(m_prevMouseState.pos.y * m_fMapSizeOffset) + nMouseOffsetY;

			if( nX < nHalfTextureSize	) 
				continue;

			if( nX >=(INT)(this->h * MAP_ENLARGEMENT_MAX) - nHalfTextureSize	) 
				continue;

			if( nZ < nHalfTextureSize	) 
				continue;

			if( nZ >= (INT)(this->h * MAP_ENLARGEMENT_MAX) - nHalfTextureSize	) 
				continue;

			nOffsetX	= nX - nHalfTextureSize;
			nOffsetZ	= nZ - nHalfTextureSize;

			// 마우스가 좌표 위에 있다면 삭제한다
			if( (nMouseX >= nOffsetX && nMouseX <= nOffsetX+nTextureSize)  &&
				(nMouseZ >= nOffsetZ && nMouseZ <= nOffsetZ+nTextureSize)  )
			{
				m_pAgcmMinimap->DestroyMapPosition( pInfo->m_nID );
				return TRUE;
			}


		}
	}

	return FALSE;
}

VOID AcUIMinimap::RenderRegisterPos( VOID )
{
	UINT8			uAlphaWindow		=	( UINT8 ) ( ((m_lColor >> 24) & 0xff) * (m_pfAlpha ? *m_pfAlpha : 1.0f) );
	RwTexture		*pRegisterPosTexture	=	m_pAgcmMinimap->GetRegisterPosTexture();
	AgpdCharacter	*pcsSelfCharacter	=	m_pAgcmMinimap->m_pcsSelfCharacter; 
	RwRaster*		pCameraRaster		=	RwCameraGetRaster( m_pRenderCamera );
	INT32			nX					=	0;
	INT32			nZ					=	0;
	INT32			nMouseX				=	0;
	INT32			nMouseZ				=	0;

	INT32			nOffsetX, nOffsetZ	;

	if( !pCameraRaster )
		return;

	if( pRegisterPosTexture && pcsSelfCharacter )
	{
		INT32	nCenterX		=	( (INT)(this->w * MAP_ENLARGEMENT_MAX) >> 1 );
		INT32	nCenterZ		=	( (INT)(this->h * MAP_ENLARGEMENT_MAX) >> 1 );
		INT		nTextureSize	=	RwRasterGetWidth( RwTextureGetRaster(pRegisterPosTexture) );
		INT		nHalfTextureSize=	nTextureSize / 2;

		INT		nCount			= m_pAgcmMinimap->GetMapPositionCount();
		for( INT i = 0 ; i < nCount ; ++i )
		{
			stMapRegisterInfo*	pInfo	=	m_pAgcmMinimap->GetMapPositionIndex( i );
			if( !pInfo )
				continue;

			nX = GetMMPos( pInfo->m_vPosition.x ) - m_nXPos + nCenterX;
			nZ = GetMMPos( pInfo->m_vPosition.y ) - m_nZPos + nCenterZ;


			if( nX < nHalfTextureSize	) 
				continue;

			if( nX >=(INT)(this->h * MAP_ENLARGEMENT_MAX) - nHalfTextureSize	) 
				continue;

			if( nZ < nHalfTextureSize	) 
				continue;

			if( nZ >= (INT)(this->h * MAP_ENLARGEMENT_MAX) - nHalfTextureSize	) 
				continue;

			nOffsetX	= nX - nHalfTextureSize;
			nOffsetZ	= nZ - nHalfTextureSize;

			DWORD	dwCurrentTime	=	g_pEngine->GetTickCount();
			DWORD	dwEndTime		=	pInfo->m_dwStartTime+pInfo->m_dwDurationTime;
			if( dwEndTime > dwCurrentTime )
			{
				DWORD	dwRest	=	dwEndTime - dwCurrentTime;

				uAlphaWindow	=	(UINT8)(uAlphaWindow * ( 1.0f - ( ( dwRest % pInfo->m_dwInterval ) / (FLOAT)pInfo->m_dwInterval ) ));
			}

			// 우선 좌표 텍스쳐를 찍어주고..
			g_pEngine->DrawIm2DPixelRaster( pCameraRaster   ,
				pRegisterPosTexture	, 
				nOffsetX			, 
				nOffsetZ			, 
				nTextureSize		,
				nTextureSize		,
				0					,
				0					,
				-1					,
				-1					,
				m_lColor			,
				uAlphaWindow
				);
		}
	}
}

VOID AcUIMinimap::RenderPartyMember()
{
	UINT8			uAlphaWindow		= ( UINT8 ) ( ((m_lColor >> 24) & 0xff) * (m_pfAlpha ? *m_pfAlpha : 1.0f) );
	RwTexture		*pMemberPosTexture	= m_pAgcmMinimap->GetMemberPosTexture();
	AgpdCharacter	*pcsSelfCharacter	= m_pAgcmMinimap->m_pcsSelfCharacter; 
	RwRaster*		pCameraRaster		=	RwCameraGetRaster( m_pRenderCamera );
	INT32 nX , nZ;
	INT32	nOffsetX, nOffsetZ	;

	if( !pCameraRaster )
		return;

	if( pMemberPosTexture && pcsSelfCharacter )
	{
		INT32	nCenterX		= ( (INT)(this->w * MAP_ENLARGEMENT_MAX) >> 1 );
		INT32	nCenterZ		= ( (INT)(this->h * MAP_ENLARGEMENT_MAX) >> 1 );

		AgpdParty	*pcsParty	= this->m_pAgcmMinimap->m_pcsAgpmParty->GetPartyLock(pcsSelfCharacter); 

		if (pcsParty && m_pAgpmCharacter) 
		{ 
			for (int i = 0 ; i < pcsParty->m_nCurrentMember; ++i) 
			{
				AgpdCharacter	*pcsMember	= m_pAgpmCharacter->GetCharacter(pcsParty->m_lMemberListID[i]);

				if (pcsMember == pcsSelfCharacter	||
					pcsMember == NULL				) 
					continue; 

				nX = GetMMPos( pcsMember->m_stPos.x ) - m_nXPos + nCenterX;
				nZ = GetMMPos( pcsMember->m_stPos.z ) - m_nZPos + nCenterZ;

				if( nX < 4	) 
					continue;

				if( nX >=(INT)(this->h * MAP_ENLARGEMENT_MAX) - 4	) 
					continue;

				if( nZ < 4	) 
					continue;

				if( nZ >= (INT)(this->h * MAP_ENLARGEMENT_MAX) - 4	) 
					continue;

				nOffsetX	= nX - 4;
				nOffsetZ	= nZ - 4;

				g_pEngine->DrawIm2DPixelRaster( pCameraRaster   ,
						pMemberPosTexture	, 
						nOffsetX			, 
						nOffsetZ			, 
						8					,
						8					,
						0					,
						0					,
						-1					,
						-1					,
						m_lColor			,
						uAlphaWindow
						);

			} 

			pcsParty->m_Mutex.Release();
		}	
	}
}

VOID AcUIMinimap::RenderETC()
{
	UINT8	uAlphaWindow = ( UINT8 ) ( ((m_lColor >> 24) & 0xff) * (m_pfAlpha ? *m_pfAlpha : 1.0f) );

	INT32			nX , nZ;
	INT32			nOffsetX, nOffsetZ	;
	RwRaster*		pCameraRaster	=	RwCameraGetRaster( m_pRenderCamera );

	m_lDisplayCount	=	0;

	if( !pCameraRaster )
		return;

	INT32	nCenterX, nCenterZ;
		BOOL	bNeedPopup	= FALSE;
	char*	pStrText		= NULL;

		AgcmMinimap::MPInfo *	pArray = this->m_pAgcmMinimap->GetMPArray();
		INT32					nCount = this->m_pAgcmMinimap->GetMPCount();

		RwTexture * pTexturePoint;
		INT32		nWidth		;
		INT32		nWidthHalf	;

	nCenterX	= ( (INT)(this->w * MAP_ENLARGEMENT_MAX) >> 1 );
	nCenterZ	= ( (INT)(this->h * MAP_ENLARGEMENT_MAX) >> 1 );

		for( int i = 0 ; i < nCount ; i ++ )
		{
			if( !pArray[ i ].bDisabled )
			{
				pArray[ i ].bRendered = FALSE;

				switch( pArray[ i ].nType )
				{
				case AgcmMinimap::MPInfo::MPI_QUESTNEW:
					{
						pTexturePoint	= this->m_pAgcmMinimap->GetNpcQuestNewTexture();
						nWidth			= 16;
					pArray[ i ].nTextureWidth	=	16;
					}
					break;
				case AgcmMinimap::MPInfo::MPI_QUESTINCOMPLETE:
					{
						pTexturePoint	= this->m_pAgcmMinimap->GetNpcQuestIncompleteTexture();
					pArray[ i ].nTextureWidth	=	16;
					}
					break;

				default:
				case AgcmMinimap::MPInfo::MPI_NPC:
				case AgcmMinimap::MPInfo::MPI_NONE:
					{
						pTexturePoint	= this->m_pAgcmMinimap->GetNpcNormalTexture();
					pArray[ i ].nTextureWidth	=	10;
					}
					break;
				case AgcmMinimap::MPInfo::MPI_SIEGENORMAL:
					{
						pTexturePoint	= this->m_pAgcmMinimap->GetTextureSiegeNormal();
					pArray[ i ].nTextureWidth	=	17;
					}
					break;
				case AgcmMinimap::MPInfo::MPI_SIEGEDESTROIED:
					{
						pTexturePoint	= this->m_pAgcmMinimap->GetTextureSiegeDestroied();
					pArray[ i ].nTextureWidth	=	20;
					}
					break;
				}

			nWidthHalf		= pArray[ i ].nTextureWidth >> 1;

				// protection!
			if( NULL == pTexturePoint ) 
				continue;

			nX = GetMMPos( pArray[ i ].pos.x ) - m_nXPos + nCenterX;
			nZ = GetMMPos( pArray[ i ].pos.z ) - m_nZPos + nCenterZ;

			if( nX < nWidthHalf	) 
				continue;

			if( nX >=(INT)(this->h * MAP_ENLARGEMENT_MAX) - nWidthHalf	) 
				continue;

			if( nZ < nWidthHalf	) 
				continue;

			if( nZ >= (INT)(this->h * MAP_ENLARGEMENT_MAX) - nWidthHalf	) 
				continue;

				nOffsetX	= nX - nWidthHalf;
				nOffsetZ	= nZ - nWidthHalf;

			g_pEngine->DrawIm2DPixelRaster(	pCameraRaster   ,
						pTexturePoint	, 
						nOffsetX			, 
						nOffsetZ			, 
				pArray[ i ].nTextureWidth	,
				pArray[ i ].nTextureWidth	,
						0					,
						0					,
						-1					,
						-1					,
						m_lColor					,
						uAlphaWindow );

				pArray[ i ].bRendered = TRUE;
			}
		}


}

VOID	AcUIMinimap::_RenderCameraClear( VOID )
{
	if( !m_pRenderCamera )
		return;

	RwRaster*	pCameraRaster	=	RwCameraGetRaster( m_pRenderCamera );
	RwRaster*	pCameraZRaster	=	RwCameraGetZRaster( m_pRenderCamera );

	if( pCameraZRaster )
		RwRasterDestroy( pCameraZRaster );

	RwCameraDestroy( m_pRenderCamera );
	m_pRenderCamera	=	NULL;

	if( m_pRenderTexture )
					{
		RwTextureDestroy( m_pRenderTexture );
		m_pRenderTexture	=	NULL;
					}
	else if( pCameraRaster )
	{
		RwRasterDestroy( pCameraRaster );
	}

}


BOOL	AcUIMinimap::_RenderCameraCreate( VOID )
{
	_RenderCameraClear();

	RwFrame*		pCameraFrame			=	NULL;
	RwRaster*		pCameraTextureRaster	=	NULL;
	RwRaster*		pCameraZBuffer			=	NULL;

	m_pRenderCamera		=	RwCameraCreate();

	if( !m_pRenderCamera )
		return FALSE;

	pCameraFrame		=	RwCameraGetFrame( g_pEngine->m_pCamera );
	if( !pCameraFrame )
		return FALSE;

	RwCameraSetFrame( m_pRenderCamera , pCameraFrame );

	pCameraTextureRaster		=	RwRasterCreate( GetTextureSize() , GetTextureSize() , 0 , rwRASTERTYPECAMERATEXTURE );
	pCameraZBuffer				=	RwRasterCreate( GetTextureSize() , GetTextureSize() , 0 , rwRASTERTYPEZBUFFER );

	if( !pCameraTextureRaster && !pCameraZBuffer )
					{
		RwCameraSetFrame( m_pRenderCamera , NULL );
		RwFrameDestroy( pCameraFrame );
		RwCameraDestroy( m_pRenderCamera );
		return FALSE;
					}

	RwCameraSetRaster( m_pRenderCamera , pCameraTextureRaster );
	RwCameraSetZRaster( m_pRenderCamera , pCameraZBuffer );

	return TRUE;

}

VOID AcUIMinimap::RenderCursor()
{
	INT	nCenterX	=	0;
	INT	nCenterZ	=	0;

	UINT8	uAlphaWindow	=	( UINT8 ) ( ((m_lColor >> 24) & 0xff) * (m_pfAlpha ? *m_pfAlpha : 1.0f) );

	nCenterX	= ( this->w >> 1 );
	nCenterZ	= ( this->h >> 1 );

		// 커서 출력.
		{
			RwTexture * pCursor = this->m_pAgcmMinimap->GetCursorTexture();

			// 중앙점..
			nCenterX	= ( this->w >> 1 );
			nCenterZ	= ( this->h >> 1 );
			if( pCursor )
			{
				// 커서 출력..
				// 방위 알아냄.. 
				INT32	nOffsetX, nOffsetZ	;
			INT		nTextureWidth	=	RwRasterGetWidth( RwTextureGetRaster(pCursor) );
			INT		nTextureHeight	=	RwRasterGetHeight( RwTextureGetRaster(pCursor) );

			nTextureHeight	-=	10;

			nOffsetX	= nCenterX - (nTextureWidth/2)	;
			nOffsetZ	= nCenterZ - (nTextureHeight/2) ;

				this->ClientToScreen(&nOffsetX, &nOffsetZ);
				this->ClientToScreen(&nCenterX, &nCenterZ);

				g_pEngine->DrawIm2DRotate( 
					pCursor, 
					(float) ( nOffsetX ), 
					(float) ( nOffsetZ ), 
				(float) ( nTextureWidth )	,	// 이미지 싸이즈
				(float) ( nTextureHeight )	,	// 이미지 헤이트.
					(float) 0.0f	,			
					(float) 0.0f	,			
				(float) 1.0f	,			
				(float) 0.91f,
					( FLOAT ) nCenterX , ( FLOAT ) nCenterZ , 360 - this->m_fCurrentTurnY + 180,
					m_lColor					,
					uAlphaWindow
					);

				// 커서 빙빙..
				static	UINT32	uLastTime	= GetTickCount();
				UINT32	uTimeDelta	= GetTickCount() - uLastTime;
				uLastTime = GetTickCount();

				FLOAT	fCurrentY	= m_fCurrentTurnY										;
				FLOAT	fTurnY		= this->m_pAgcmMinimap->m_pcsSelfCharacter->m_fTurnY	;

				FLOAT	fDelta	= fTurnY - fCurrentY;
				FLOAT	fDelta2	;
				if( fDelta > 180.0f )
				{
					fDelta -= 360;
				}
				else if( fDelta < -180.0f )
				{
					fDelta += 360;
				}

				if( fDelta > 0 )
				{
					m_fCurrentTurnY += ( FLOAT )uTimeDelta * __sRate;

					fDelta2	= fTurnY - m_fCurrentTurnY;
					if( fDelta2 > 180.0f )
					{
						fDelta2 -= 360;
					}
					else if( fDelta2 < -180.0f )
					{
						fDelta2 += 360;
					}

					if( fDelta2 < 0 )
					{
						m_fCurrentTurnY = this->m_pAgcmMinimap->m_pcsSelfCharacter->m_fTurnY;
					}
				}
				else if( fDelta < 0 )
				{
					m_fCurrentTurnY -= ( FLOAT )uTimeDelta * __sRate;

					fDelta2	= fTurnY - m_fCurrentTurnY;
					if( fDelta2 > 180.0f )
					{
						fDelta2 -= 360;
					}
					else if( fDelta2 < -180.0f )
					{ 
						fDelta2 += 360;
					}

					if( fDelta2 > 0 )
					{
						m_fCurrentTurnY = this->m_pAgcmMinimap->m_pcsSelfCharacter->m_fTurnY;
					}
				}
				else
				{
					// do nothing..
				}
			}
		}

}

VOID AcUIMinimap::RenderDisplayRegisterPopup( VOID )
{
	UINT8			uAlphaWindow		=	( UINT8 ) ( ((m_lColor >> 24) & 0xff) * (m_pfAlpha ? *m_pfAlpha : 1.0f) );
	RwTexture		*pRegisterPosTexture	=	m_pAgcmMinimap->GetRegisterPosTexture();
	AgpdCharacter	*pcsSelfCharacter	=	m_pAgcmMinimap->m_pcsSelfCharacter; 
	RwRaster*		pCameraRaster		=	RwCameraGetRaster( m_pRenderCamera );
	INT32			nX					=	0;
	INT32			nZ					=	0;
	INT32			nMouseX				=	0;
	INT32			nMouseZ				=	0;

	INT32			nOffsetX, nOffsetZ	;

	if( !pCameraRaster )
		return;

	if( pRegisterPosTexture && pcsSelfCharacter )
			{
		INT32	nCenterX		=	( (INT)(this->w * MAP_ENLARGEMENT_MAX) >> 1 );
		INT32	nCenterZ		=	( (INT)(this->h * MAP_ENLARGEMENT_MAX) >> 1 );
		INT		nMouseOffsetX	=	( (INT)(this->w * m_fMapSizeOffset) - ACUIMINIMAP_TEXTURE_SIZE ) / 2;
		INT 	nMouseOffsetY	=	( (INT)(this->h * m_fMapSizeOffset) - ACUIMINIMAP_TEXTURE_SIZE ) / 2;

		INT		nTextureSize	=	RwRasterGetWidth( RwTextureGetRaster(pRegisterPosTexture) );
		INT		nHalfTextureSize=	nTextureSize / 2;

		nMouseOffsetX	=	abs( nMouseOffsetX );
		nMouseOffsetY	=	abs( nMouseOffsetY );

		INT		nCount			= m_pAgcmMinimap->GetMapPositionCount();
		for( INT i = 0 ; i < nCount ; ++i )
		{
			stMapRegisterInfo*	pInfo	=	m_pAgcmMinimap->GetMapPositionIndex( i );
			if( !pInfo )
					continue;

			nX = GetMMPos( pInfo->m_vPosition.x ) - m_nXPos + nCenterX;
			nZ = GetMMPos( pInfo->m_vPosition.y ) - m_nZPos + nCenterZ;

			nMouseX = (INT)(m_prevMouseState.pos.x * m_fMapSizeOffset) + nMouseOffsetX;
			nMouseZ = (INT)(m_prevMouseState.pos.y * m_fMapSizeOffset) + nMouseOffsetY;

			if( nX < nHalfTextureSize	) 
				continue;

			if( nX >=(INT)(this->h * MAP_ENLARGEMENT_MAX) - nHalfTextureSize	) 
				continue;

			if( nZ < nHalfTextureSize	) 
				continue;

			if( nZ >= (INT)(this->h * MAP_ENLARGEMENT_MAX) - nHalfTextureSize	) 
				continue;

			nOffsetX	= nX - nHalfTextureSize;
			nOffsetZ	= nZ - nHalfTextureSize;

			// 마우스가 텍스쳐 위에 있으면 툴팁을 찍어준다
			if( (nMouseX >= nOffsetX && nMouseX <= nOffsetX+nTextureSize)  &&
				(nMouseZ >= nOffsetZ && nMouseZ <= nOffsetZ+nTextureSize)  )
			{
				m_pAgcmMinimap->m_pcsAgcmFont->FontDrawStart(m_lFontType);

				INT	nTextWidth	= m_pAgcmMinimap->m_pcsAgcmFont->GetTextExtent( 0 , (CHAR*)pInfo->m_strName.c_str() , (INT32)pInfo->m_strName.length() );
				INT	nPopupX		= (INT)(m_prevMouseState.pos.x + (m_fMapSizeOffset*4) - (nTextWidth/2));
				INT	nPopupY		= (INT)(m_prevMouseState.pos.y - (m_fMapSizeOffset*8));

				ClientToScreen( &nPopupX , &nPopupY );

				m_pAgcmMinimap->m_pcsAgcmFont->DrawTextIM2D( (FLOAT) nPopupX , (FLOAT) nPopupY , (CHAR*)pInfo->m_strName.c_str(),
					0 ,			
					uAlphaWindow,	
					0xfffff8e7,
					true,		
					false,		
					true,		
					0x77000000	
					);

				m_pAgcmMinimap->m_pcsAgcmFont->FontDrawEnd();

			}


		}
	}

}

VOID AcUIMinimap::RenderDisplayPopup()
{

	AgcmMinimap::MPInfo *	pArray = this->m_pAgcmMinimap->GetMPArray();
	INT32					nCount = this->m_pAgcmMinimap->GetMPCount();

	UINT8			uAlphaWindow		=	( UINT8 ) ( ((m_lColor >> 24) & 0xff) * (m_pfAlpha ? *m_pfAlpha : 1.0f) );
	AgpdCharacter	*pcsSelfCharacter	=	m_pAgcmMinimap->m_pcsSelfCharacter; 
	RwRaster*		pCameraRaster		=	RwCameraGetRaster( m_pRenderCamera );
	INT32			nX					=	0;
	INT32			nZ					=	0;
	INT32			nMouseX				=	0;
	INT32			nMouseZ				=	0;

	INT32	nCenterX		=	( (INT)(this->w * MAP_ENLARGEMENT_MAX) >> 1 );
	INT32	nCenterZ		=	( (INT)(this->h * MAP_ENLARGEMENT_MAX) >> 1 );
	INT		nMouseOffsetX	=	( (INT)(this->w * m_fMapSizeOffset) - ACUIMINIMAP_TEXTURE_SIZE ) / 2;
	INT 	nMouseOffsetY	=	( (INT)(this->h * m_fMapSizeOffset) - ACUIMINIMAP_TEXTURE_SIZE ) / 2;

	INT32			nOffsetX=	0;
	INT32			nOffsetZ=	0;

	nMouseOffsetX	=	abs( nMouseOffsetX );
	nMouseOffsetY	=	abs( nMouseOffsetY );

	for( int i = 0 ; i < nCount ; i ++ )
	{
		if ( pArray[ i ].bDisabled || !pArray[ i ].strText[0] )
			continue;

		INT		nTextureSize	=	pArray[ i ].nTextureWidth;
		INT		nHalfTextureSize=	nTextureSize / 2;

		nX = GetMMPos( pArray[ i ].pos.x ) - m_nXPos + nCenterX;
		nZ = GetMMPos( pArray[ i ].pos.z ) - m_nZPos + nCenterZ;

		nMouseX = (INT)((m_prevMouseState.pos.x * m_fMapSizeOffset) + nMouseOffsetX);
		nMouseZ = (INT)((m_prevMouseState.pos.y * m_fMapSizeOffset) + nMouseOffsetY);

		if( nX < nHalfTextureSize	) 
			continue;

		if( nX >=(INT)(this->h * MAP_ENLARGEMENT_MAX) - nHalfTextureSize	) 
			continue;

		if( nZ < nHalfTextureSize	) 
			continue;

		if( nZ >= (INT)(this->h * MAP_ENLARGEMENT_MAX) - nHalfTextureSize	) 
			continue;

		nOffsetX	= nX - nHalfTextureSize;
		nOffsetZ	= nZ - nHalfTextureSize;

		// 마우스가 텍스쳐 위에 있으면 툴팁을 찍어준다
		if( (nMouseX >= nOffsetX && nMouseX <= nOffsetX+nTextureSize)  &&
			(nMouseZ >= nOffsetZ && nMouseZ <= nOffsetZ+nTextureSize)  )
	{

			// 팝업 표시..
			m_pAgcmMinimap->m_pcsAgcmFont->FontDrawStart(m_lFontType);

			INT	nTextWidth	= m_pAgcmMinimap->m_pcsAgcmFont->GetTextExtent( 0 , pArray[ i ].strText , strlen(pArray[ i ].strText) );
			INT	nPopupX		= (INT)(m_prevMouseState.pos.x + (m_fMapSizeOffset*4) - (nTextWidth/2));
			INT	nPopupY		=(INT)( m_prevMouseState.pos.y - (m_fMapSizeOffset*8));

			ClientToScreen( &nPopupX , &nPopupY );

			m_pAgcmMinimap->m_pcsAgcmFont->DrawTextIM2D(
				( FLOAT ) nPopupX, ( FLOAT ) nPopupY ,
				pArray[ i ].strText,	//  Text
				0 ,			//  font type
				uAlphaWindow,		//  alpha
				0xfffff8e7, //g_TextColor,	// color
				true,		// Draw Shadow
				false,		// Blod
				true,		// Draw Background
				0x77000000	//g_BackColor	// Background color
				);

			m_pAgcmMinimap->m_pcsAgcmFont->FontDrawEnd();
		}
	}

}

// 맵 확대
FLOAT	AcUIMinimap::MapEnlargement( VOID )
{
	INT		nMapReductionMax	=	(INT)(MAP_REDUCTION_MAX*10);
	INT		nMapSizeOffset		=	(INT)(m_fMapSizeOffset*10);

	nMapSizeOffset	-=	(INT)(MAP_INCREASE_DATA*10);


	if( nMapReductionMax <= nMapSizeOffset )
	{
		m_fMapSizeOffset	=	nMapSizeOffset / 10.f;

		_RenderCameraCreate();	//	Camera Texture를 새로 만든다
		PositionUpdate();
	}

	return m_fMapSizeOffset;
}

// 맵 축소
FLOAT	AcUIMinimap::MapReduction( VOID )
{
	INT		nMapReductionMax	=	(INT)(MAP_ENLARGEMENT_MAX*10);
	INT		nMapSizeOffset		=	(INT)(m_fMapSizeOffset*10);

	nMapSizeOffset	+=	(INT)(MAP_INCREASE_DATA*10);

	if( nMapReductionMax >= nMapSizeOffset )
	{
		m_fMapSizeOffset	=	nMapSizeOffset / 10.f;

		_RenderCameraCreate(); // Camera Texture를 새로 만든다
		PositionUpdate();
			}

	return m_fMapSizeOffset;
}

INT	AcUIMinimap::GetTextureSize( VOID )
{
	INT		nReturnValue	=	ACUIMINIMAP_TEXTURE_SIZE;

	return nReturnValue;
}

INT32 AcUIMinimap::GetMMPos( FLOAT fPos )
{
	return	(INT32)( fPos / (MAP_SECTOR_WIDTH * MAP_DEFAULT_DEPTH) *	( ACUIMINIMAP_MAP_SIZE << 1) );
}

INT32 AcUIMinimap::GetMMPosINV( INT32 nPos )
{
	return	(INT32)((FLOAT)( nPos ) / ( ACUIMINIMAP_MAP_SIZE << 1 ) * ( MAP_SECTOR_WIDTH * MAP_DEFAULT_DEPTH ));
}
