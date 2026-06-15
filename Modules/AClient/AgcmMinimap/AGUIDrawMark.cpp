// AcUIWorldmap.cpp: implementation of the AcUIWorldmap class.
//
//////////////////////////////////////////////////////////////////////
#include "AGUIDrawMark.h"
#include "AgcmUIManager2.h"



//////////////////////////////////////////////////////////////////////////
// AGUIDrawMark
//////////////////////////////////////////////////////////////////////////
AGUIDrawMark::AGUIDrawMark() : m_iCreateCount(1), m_pAgcmFont( NULL ), m_iCurHitMark(-1), m_pMapUI(NULL)
{
	m_uiTime = 0;
	m_uiTime2 = 0;
};

AGUIDrawMark::~AGUIDrawMark()
{
	for( LIST_TEX_INFO_ITR it = m_listTextureInfo.begin() ; it != m_listTextureInfo.end(); ++it )
	{
		RwTextureDestroy( it->pTexture );
	}

	m_listTextureInfo.clear();
};

BOOL	 AGUIDrawMark::Init( AgcmFont* pAgcmFont )
{
	m_pAgcmFont = pAgcmFont; 
	return TRUE; 
}

//텍스쳐 추가
BOOL AGUIDrawMark::AddTexture( E_MARKINFO_TYPE eType, char* pszTextureName  )
{
	INT32 iTexID = _LoadTexture( pszTextureName );
	if( m_mapTextureID.end() != m_mapTextureID.find( eType ) )
		m_mapTextureID[ eType ].push_back( iTexID);
	else
	{
		VECTOR_TEXTUREID tmp;
		tmp.push_back( iTexID );
		m_mapTextureID[eType] = tmp;
	}
	return TRUE;
}

//마킹찾기
AGUIDrawMark::LIST_MARK_INFO_ITR	AGUIDrawMark::Find( INT32 iID )
{
	for( LIST_MARK_INFO_ITR it = m_listMarkInfo.begin(); it != m_listMarkInfo.end(); ++it )
	{
		if( it->iID == iID )
			return it;
	}
	return MarkListEnd();
}

BOOL AGUIDrawMark::Add( AgpdCharacter* pChar, int iGroup, E_MARKINFO_TYPE eType, BOOL bTempShow )
{
	//추가되는 아뒤의 마킹이 없다면 새로 추가
	if( Find( pChar->GetID() ) == MarkListEnd() )
	{
		AGUIDrawMark::S_MARK_INFO markInfo;
		markInfo.eType = eType;
		markInfo.pChar = pChar;
		markInfo.iID = pChar->GetID();
		markInfo.iGroup = iGroup;
		markInfo.bTempShow = bTempShow;
		memcpy( markInfo.strComment, pChar->m_szID, sizeof( pChar->m_szID ) );
		

		m_listMarkInfo.push_back( markInfo );

		return TRUE;
	}

	return FALSE;
}

//좌표로 추가되는 마킹 정보
void AGUIDrawMark::_SetInfo(S_MARK_INFO* pInfo, INT32 iID, RwV2d vPos, char* pszComment, int iGroup, E_MARKINFO_TYPE eType, BOOL bTempShow )
{
	if( NULL == pInfo )
		return;

	pInfo->eType = eType;
	pInfo->iPosX = (INT32)vPos.x;
	pInfo->iPosY = (INT32)vPos.y;
	pInfo->iID = iID;
	pInfo->iGroup = iGroup;
	// memcpy( pInfo->strComment, pszComment, strlen(pszComment) * sizeof( char ) );
	strcpy_s( pInfo->strComment , MAX_PATH , pszComment );

	if( eType == E_TYPE_X_MARK )
		pInfo->fIntervalTime = 0.1f;

	pInfo->bGlitter = FALSE;
	if( eType == E_TYPE_SHRINE_ENABLE )
	{
		pInfo->fIntervalTime = 0.1f;
		pInfo->bGlitter = TRUE;
	}
	pInfo->bTempShow = bTempShow;
}

BOOL AGUIDrawMark::Add( INT32 iID, RwV2d vPos, char* pszComment, int iGroup, E_MARKINFO_TYPE eType, BOOL bTempShow )
{
	LIST_MARK_INFO_ITR it;
	if( (it = Find( iID )) == MarkListEnd() )
	{
		AGUIDrawMark::S_MARK_INFO markInfo;

		_SetInfo( &markInfo, iID, vPos, pszComment, iGroup, eType, bTempShow );
		m_listMarkInfo.push_back( markInfo );

		return TRUE;
	}

	return FALSE;
}

//업뎃처리
BOOL AGUIDrawMark::Update( INT32 iID, RwV2d vPos, char* pszComment, int iGroup, E_MARKINFO_TYPE eType, BOOL bTempShow )
{
	LIST_MARK_INFO_ITR it;
	//없음 추가 시키고
	if( (it = Find( iID )) == MarkListEnd() )
		Add( iID, vPos, pszComment , iGroup, eType, bTempShow );
	//있음 정보 업뎃하자
	else
		_SetInfo( &(*it), iID, vPos, pszComment, iGroup, eType, bTempShow );

	return TRUE;
}


BOOL AGUIDrawMark::Delete( INT32 iID, E_MARKINFO_TYPE eType  )
{
	LIST_MARK_INFO_ITR it = Find( iID );
	if( it == MarkListEnd() || it->eType != eType )
		return FALSE;
	m_listMarkInfo.erase( it );

	return TRUE;
}

BOOL AGUIDrawMark::Delete( INT32 iID )
{
	LIST_MARK_INFO_ITR it = Find( iID );
	if( it == MarkListEnd() )
		return FALSE;

	m_listMarkInfo.erase( it );

	return TRUE;
}

void AGUIDrawMark::DeleteTempShowElements()
{
	for( LIST_MARK_INFO_ITR it = m_listMarkInfo.begin(); it != m_listMarkInfo.end();  )
	{
		if( it->bTempShow )
			it = m_listMarkInfo.erase( it );
		else
			++it;
	}
}

AGUIDrawMark::LIST_TEX_INFO_ITR AGUIDrawMark::_FindTexture( INT32 iID )
{
	for( LIST_TEX_INFO_ITR it = m_listTextureInfo.begin(); it != m_listTextureInfo.end(); ++it )
	{
		if( it->iIndex == iID )
		{
			return it;
		}
	}

	return _TextureListEnd();
}

INT32 AGUIDrawMark::_LoadTexture( char* pszName )
{
	RwTexture* pTexture = RwTextureRead( pszName, NULL );
	ASSERT( NULL != pTexture && "파일로딩체크" );
	if( !pTexture )	return -1;

	RwTextureSetAddressing( pTexture , rwTEXTUREADDRESSCLAMP );

	S_TEXTURE_INFO info;
	info.pTexture = pTexture;
	info.iIndex = m_iCreateCount++;

	m_listTextureInfo.push_back( info );

	return info.iIndex;
}

//m_listMarkInfo에서 타입이 eType인것들을  vecID의 것과 동기화 처리
void  AGUIDrawMark::UpdateData(std::vector< INT32 >& vecID, AGUIDrawMark::E_MARKINFO_TYPE eType )
{
	std::vector<INT32> vecDeleteID;
	vecDeleteID.reserve( m_listMarkInfo.size() );
	for( LIST_MARK_INFO_ITR it = m_listMarkInfo.begin(); it != m_listMarkInfo.end(); ++it )
	{
		if( eType != it->eType )
			continue;
		
		//vecID에 해당 아뒤가 있는지 체크
		std::vector<INT32>::iterator id_it = find( vecID.begin(), vecID.end(), it->iID );

		//있으면 패스
		if( id_it != vecID.end() )
			continue;

		//없으면 지울 리스트에 추가
		vecDeleteID.push_back( it->iID );
	}

	//지울리스트에 채워진 정보  삭제
	INT32 iSize = (INT32)vecDeleteID.size();
	if( iSize > 0 )
	{
		for( int i=0; i<iSize; ++i )
		{
			Delete( vecDeleteID[i], eType );
		}
	}

}

//UI맵 상에 좌표 얻기
AGUIDrawMark::S_MAP_POS AGUIDrawMark::_GetMapPos( S_MAP_POS& stPos )
{
	//맵 영역에 위치한지 체크
	if( m_stMapArea.SX < stPos.x && m_stMapArea.EX > stPos.x
		&& m_stMapArea.SZ < stPos.y && m_stMapArea.EZ > stPos.y )
	{
		int		iX = 0 , iY = 0 ;
		m_pMapUI->m_pcsUIWindow->ClientToScreen( &iX , &iY );

		//world map ui의 스크린 시작 좌표 얻어 실제 매칭 영역을 더해 준다.
		int StartX = iX + m_stUIMatchingArea.SX;
		int StartY = iY + m_stUIMatchingArea.SZ;
		int EndX = iX + m_stUIMatchingArea.EX;
		int EndY = iY + m_stUIMatchingArea.EZ;


		//선형 보간을 사용 현재 위치에 해당하는 팩터값을 구해 ui 맵상의 좌표를 매핑시킴
		S_MAP_POS stMapPos;
		float fFactor = ( (float)(stPos.x - m_stMapArea.SX ) / (float)( m_stMapArea.EX - m_stMapArea.SX ) );
		stMapPos.x = (INT32)(StartX * (1.f - fFactor ) + ( EndX * fFactor ));

		fFactor = ( (float)(stPos.y - m_stMapArea.SZ ) / (float)( m_stMapArea.EZ - m_stMapArea.SZ ) );
		stMapPos.y = (INT32)(StartY * (1.f - fFactor ) + ( EndY * fFactor ));

		return stMapPos;
	}

	return stPos;
}

//UI맵에서 월드 맵 좌표 뽑기
AGUIDrawMark::S_MAP_POS AGUIDrawMark::GetWorldPos(S_MAP_POS& stPos)
{
	
	int	StartX = m_stMapArea.SX; 
	int	StartY = m_stMapArea.SZ ;
	int EndX = m_stMapArea.EX ;
	int EndY = m_stMapArea.EZ ;

	int		iX = 0 , iY = 0 ;
	m_pMapUI->m_pcsUIWindow->ClientToScreen( &iX , &iY );

	//스크린 좌표 얻기
	stPos.x -= (iX + m_stUIMatchingArea.SX);
	stPos.y -= (iY + m_stUIMatchingArea.SZ);

	//실제 매칭한 영역 얻기
	int StartUIX = 0;
	int StartUIY = 0;
	int EndUIX = m_stUIMatchingArea.EX - m_stUIMatchingArea.SX;
	int EndUIY =m_stUIMatchingArea.EZ - m_stUIMatchingArea.SZ;

	//선형 보간을 이용하여 월드 좌표를 구한다.
	S_MAP_POS stMapPos;
	float fFactor = ( (float)(stPos.x - StartUIX ) / (float)( EndUIX) );
	stMapPos.x = (INT32)(StartX * (1.f - fFactor ) + ( EndX * fFactor ));

	fFactor = ( (float)(stPos.y - StartUIY ) / (float)( EndUIY) );
	stMapPos.y = (INT32)(StartY * (1.f - fFactor ) + ( EndY * fFactor ));

	return stMapPos;

}

void AGUIDrawMark::Render( RwV2d vMousePos, int iCurGroup, BOOL bShowMap )
{
	m_uiTime = GetTickCount();
	float fDeltaT = (m_uiTime - m_uiTime2) * 0.001f;

	BOOL bHitTest = FALSE;
	float	fX, fY, fW, fH, fCX, fCY;
	 LIST_MARK_INFO_ITR it_hitInfo;
	m_iCurHitMark = -1;
	for( LIST_MARK_INFO_ITR it = m_listMarkInfo.begin(); it != m_listMarkInfo.end(); ++it )
	{
		S_MARK_INFO& stInfo = (*it);
		if( stInfo.eType > E_TYPE_X_MARK && stInfo.iGroup > -1 && stInfo.iGroup  != iCurGroup ) 
			continue;

		S_MAP_POS stInputPos;
		switch( stInfo.eType )
		{
			case E_TYPE_SELF:
			case E_TYPE_PARTY:
			{
				if( stInfo.pChar )
				{
					stInputPos.x = (INT32)stInfo.pChar->m_stPos.x;
					stInputPos.y = (INT32)stInfo.pChar->m_stPos.z;
				}
				break;
			}
			default:
			{

				INT32 iUIX = 0, iUIY = 0;
				if( m_pMapUI )
				{
					iUIX = m_pMapUI->m_pcsUIWindow->x;
					iUIY = m_pMapUI->m_pcsUIWindow->y;
				}
				stInputPos.x = stInfo.iPosX + iUIX;
				stInputPos.y = stInfo.iPosY + iUIY;
				break;
			}
		}

		S_MAP_POS stPos;
		if( stInfo.iGroup > -1 )
			 stPos = _GetMapPos( stInputPos );
		else
		{
			stPos = stInputPos;
		}

		//반짝 반짝 처리
		if( bShowMap && 0.f < stInfo.fIntervalTime )
		{
			if( (stInfo.bGlitter == FALSE && stInfo.iGlitterCount < 3 ) || ( stInfo.bGlitter = TRUE && stInfo.eType == E_TYPE_SHRINE_ENABLE) )
			{
				stInfo.fCurTime += fDeltaT;
				if( stInfo.fCurTime > stInfo.fIntervalTime )
				{
					stInfo.iCurTexIndex++;
					if( stInfo.iCurTexIndex >= (int)m_mapTextureID[stInfo.eType].size() )
					{
						stInfo.iCurTexIndex = 0;
						stInfo.iGlitterCount++;
					}
					stInfo.fCurTime = 0.f;
				}
			}
		}

		if( stInfo.iCurTexIndex >= (int)m_mapTextureID[stInfo.eType].size() )
			stInfo.iCurTexIndex = 0;
	
		LIST_TEX_INFO_ITR Texit = _FindTexture( m_mapTextureID[stInfo.eType][stInfo.iCurTexIndex]  );
		if( Texit == _TextureListEnd() )
			continue;

		if( NULL == Texit->pTexture )
			continue;


		RwTexture*	pTempTexture = Texit->pTexture;

		fW	= (float)pTempTexture->raster->width;
		fH	= (float)pTempTexture->raster->height;

		
		//자신의 위치 표시는 미니맵쪽이랑 동일
		if( stInfo.eType == E_TYPE_SELF )
		{
			fX =  22.f;//fW * 0.5f;
			fY =  50.f;//fH * 0.65f;
			static float fCurAngle = 0.f;
			float fDelta = stInfo.pChar->m_fTurnY - fCurAngle;
			float fAbsDelta = abs(fDelta );
			if( fAbsDelta > 180 )
			{
				fCurAngle = stInfo.pChar->m_fTurnY;
			}
			else if( fAbsDelta > 0.01f ) //0.01f이하는 무시하자
			{
				// 커서 빙빙..
				FLOAT	fCurrentY	= fCurAngle;
				FLOAT	fTurnY		= stInfo.pChar->m_fTurnY;

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
					fCurAngle += ( FLOAT )fDeltaT*200.f;

					fDelta2	= fTurnY - fCurAngle;
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
						fCurAngle = fTurnY;
					}
				}
				else if( fDelta < 0 )
				{
					fCurAngle -= ( FLOAT )fDeltaT*200.f;

					fDelta2	= fTurnY - fCurrentY;
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
						fCurAngle = fTurnY;
					}
				}
				else
				{
					// do nothing..
				}
			}
			g_pEngine->DrawIm2DRotate( Texit->pTexture, stPos.x -fX, stPos.y -fY, fW, fH, 0.f, 0.f, 1.f, 1.f, (float)stPos.x, (float)stPos.y, 360.f - fCurAngle  + 180.f);

			// 맵상 툴팁 제공을 위한 임시 변수 값 변경
			fW = fH = 17.f;

			fX =  (float)stPos.x - fW * 0.5f;
			fY =  (float)stPos.y - fH * 0.5f;
		}
		else
		{
			fX =  (float)stPos.x - fW * 0.5f;
			fY =  (float)stPos.y - fH * 0.5f;
			g_pEngine->DrawIm2D( Texit->pTexture, fX , fY , fW, fH );
		}
		
		//커서가 위치해 있는지 체크
		if( FALSE == bHitTest )
		{
			if( vMousePos.x > fX && vMousePos.x < fX + fW && vMousePos.y > fY && vMousePos.y < fY + fH )
			{
				it_hitInfo = it;
				fCX = vMousePos.x;
				fCY = vMousePos.y;
				bHitTest = TRUE;
				m_iCurHitMark = stInfo.iID;
			}
		}
	}

	if( bHitTest )
	{
		if( m_pAgcmFont )
		{
			m_pAgcmFont->FontDrawStart(0);

			//	INT	nTextWidth	= m_pAgcmFont->GetTextExtent( 0 , stInfo.strComment , strlen( stInfo.strComment ) );
			m_pAgcmFont->DrawTextIM2D( fCX , fCY-15.f , it_hitInfo->strComment,
				0 ,			//  font type
				255,		//  alpha
				0xfffff8e7, //g_TextColor,	// color
				true,		// Draw Shadow
				false,		// Blod
				true,		// Draw Background
				0x77000000	//g_BackColor	// Background color
				);

			m_pAgcmFont->FontDrawEnd();
		}
	}

	m_uiTime2 = m_uiTime;
}
