#include "stdafx.h"
#include "MapTool.h"
#include "rwcore.h"
#include "rpworld.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rpcollis.h"
#include "rtpick.h"
#include "rtcharse.h"
#include "rpmatfx.h"

#include <skeleton.h>
#include <menu.h>
#include <camera.h>
#include <AgcEngine.h>
#include "AcuRpMTexture.h"
#include "MainWindow.h"
#include "MainFrm.h"

#include "MapSelectDlg.h"
#include "ProgressDlg.h"

#include "bobbyutil.h"

#include "rpusrdat.h"
#include "ApmObject.h"
#include "AgcmObject.h"

#include "SectorCreateDlg.h"
#include "AcUIBase.h"

#include "MyEngine.h"

#include "ObjectPropertiesDlg.h"

#include <d3dx9.h>

#include "AcuObjecWire.h"

extern AgcmPostFX*			g_pcsAgcmPostFX			;

extern CMainFrame *			g_pMainFrame	;
extern MainWindow			g_MainWindow	;

BOOL	g_bUsePickingOnly	= FALSE;

BOOL	GetWorldPosToScreenPos( RwV3d* pWorldPos , POINT* pPoint );
void	__RenderAtomicOutLine( RpAtomic* pAtomic );
void	_GetIntersectPos( AuPOS* pPos , RwV3d* pNormal , RwV3d *pStart , RwV3d *pEnd , RwV3d *pCollisionPoint );
BOOL	__DivisionRenderSectorGridCallback ( DivisionInfo * pDivisionInfo , PVOID pData );

struct	FindWorldSectorInfo
{
	RwV3d			v;
	RpWorldSector*	pSector;
};

struct stAlphaCalcurate
{
	UINT32	uTileIndex;
	UINT32	uPortion;

	void	Empty()
	{
		uPortion	= 0;
		uTileIndex	= ALEF_TEXTURE_NO_TEXTURE;
	}
};

RpWorldSector *	RsWorldFindSector( RpWorldSector * pSector , void * pData )
{
	FindWorldSectorInfo	*pFind = ( FindWorldSectorInfo * ) pData;
	
	const RwBBox	*pBBox = RpWorldSectorGetBBox( pSector );
	if( pBBox->inf.x < pFind->v.x && pBBox->sup.x > pFind->v.x &&
		pBBox->inf.z < pFind->v.z && pBBox->sup.z > pFind->v.z )
	{
		pFind->pSector	= pSector;
		return NULL;
	}
	else
		return pSector;
}

BOOL	IsPointLightObject( ApdObject* pstObject )
{
	return g_pcsApmEventManager->GetEvent( pstObject , APDEVENT_FUNCTION_POINTLIGHT ) ? TRUE : FALSE;
}

FLOAT	_InRadianRange( FLOAT fDegree )
{
	while( fDegree < 0 )
		fDegree += 360.0f;
	while( fDegree >= 360 )
		fDegree -= 360.0f;

	return fDegree;
}

inline	FLOAT	__LimitHeight( FLOAT fHeight )
{
	return max( SECTOR_MIN_HEIGHT, min( fHeight, SECTOR_MAX_HEIGHT ) );
}

RpCollisionTriangle * RsIntersectionCallBackGeometryTriangleAtomic( RpIntersection *intersection, RpCollisionTriangle *collTriangle, RwReal distance, void *data )
{
	MainWindow * pWindow = (MainWindow*) data;
	pWindow->m_nFoundPolygonIndex	= collTriangle->index;
	pWindow->m_Position.x			= intersection->t.line.start.x + (intersection->t.line.end.x - intersection->t.line.start.x) * distance;
	pWindow->m_Position.y			= intersection->t.line.start.y + (intersection->t.line.end.y - intersection->t.line.start.y) * distance;
	pWindow->m_Position.z			= intersection->t.line.start.z + (intersection->t.line.end.z - intersection->t.line.start.z) * distance;

	return NULL;
}

BOOL	__AgcmRender_Maptool_PreRenderCallback (PVOID pData, PVOID pClass, PVOID pCustData)
{
	g_MainWindow.DrawGridInfo();
	return TRUE;
}


void SectorPreviwTextureSetup( stFindMapInfo * pMapInfo , int tileindex  )
{
	if( tileindex != ALEF_SECTOR_DEFAULT_TILE_INDEX )	return;

	rsDWSectorInfo	dwSectorInfo;
	AGCMMAP_THIS->GetDWSector( pMapInfo->pSector, &dwSectorInfo );
	
	_PreviewTextureSetup( pMapInfo->pSector, pMapInfo->nSegmentX, pMapInfo->nSegmentZ + 1, tileindex, &dwSectorInfo.texCoords[0][dwSectorInfo.polygons[pMapInfo->nPolygonIndexFirst].vertIndex[0]] );
	_PreviewTextureSetup( pMapInfo->pSector, pMapInfo->nSegmentX + 1, pMapInfo->nSegmentZ, tileindex, &dwSectorInfo.texCoords[0][dwSectorInfo.polygons[pMapInfo->nPolygonIndexFirst].vertIndex[1]] );
	_PreviewTextureSetup( pMapInfo->pSector, pMapInfo->nSegmentX, pMapInfo->nSegmentZ, tileindex, &dwSectorInfo.texCoords[0][dwSectorInfo.polygons[pMapInfo->nPolygonIndexFirst].vertIndex[2]] );

	_PreviewTextureSetup( pMapInfo->pSector, pMapInfo->nSegmentX, pMapInfo->nSegmentZ + 1, tileindex, &dwSectorInfo.texCoords[0][dwSectorInfo.polygons[pMapInfo->nPolygonIndexFirst + 1].vertIndex[0]] );
	_PreviewTextureSetup( pMapInfo->pSector, pMapInfo->nSegmentX + 1, pMapInfo->nSegmentZ + 1, tileindex, &dwSectorInfo.texCoords[0][dwSectorInfo.polygons[pMapInfo->nPolygonIndexFirst + 1].vertIndex[1]] );
	_PreviewTextureSetup( pMapInfo->pSector, pMapInfo->nSegmentX + 1, pMapInfo->nSegmentZ, tileindex, &dwSectorInfo.texCoords[0][dwSectorInfo.polygons[pMapInfo->nPolygonIndexFirst + 1].vertIndex[2]] );
}

inline	BOOL	__CropCameraMovement( RwV3d * pPos , RwV3d * pTrans )
{
	RwV3d	posDest;
	RwV3dAdd( &posDest, pPos, pTrans );

	if( ISBUTTONDOWN( VK_CONTROL ) || !g_Const.m_bCameraBlocking )
	{
		// do nothing
	}
	else
	{
		FLOAT fHeight = 100.0f + AGCMMAP_THIS->GetHeight( posDest.x , posDest.z , SECTOR_MAX_HEIGHT );
		if( posDest.y < fHeight )
		{
			pTrans->y = fHeight;
			return FALSE;
		}
	}

	return TRUE;
}

BOOL	__CheckCircumstanceSectorPolygonInfo( ApWorldSector * pSector )
{
	for( int i=0; i<TD_DIRECTIONCOUNT; ++i )
	{
		ApWorldSector* pNearSector = pSector->GetNearSector( i );
		if( !pNearSector )	continue;

		rsDWSectorInfo	dwSectorInfo;
		AGCMMAP_THIS->GetDWSector( pNearSector, SECTOR_HIGHDETAIL, &dwSectorInfo );
		if( !dwSectorInfo.pDWSector || !dwSectorInfo.pDWSector->geometry )
			return FALSE;
	}
	
	return TRUE;
}

INT32 __GetTongMapIndex( INT32 nIndex , ApWorldSector * pSector , INT32 nSegmentX , INT32 nSegmentZ )
{
	// ¸¶°íÀÚ (2003-11-13 ¿ÀÈÄ 3:41:44) : ¸ÊÀÌ Ä¿Á®¼­ Åè¾Û ¼³Á¤¿¡ ¿À·ù°¡ »ý°åÀ½..
	int	dx , dz;
	switch( GET_TEXTURE_DIMENSION( nIndex ) )
	{
	case 4:		dx = 2; dz = 0; break;
	case 8:		dx = 2; dz = 0; break;
	default:	dx = 0; dz = 0; break;
	}

	return UPDATE_TEXTURE_OFFSET(
			nIndex , 
				(	pSector->GetArrayIndexX() * pSector->D_GetDepth() + nSegmentX + dx ) % GET_TEXTURE_DIMENSION( nIndex )		+
			(	(	pSector->GetArrayIndexZ() * pSector->D_GetDepth() + nSegmentZ + dz ) % GET_TEXTURE_DIMENSION( nIndex )		)
				* GET_TEXTURE_DIMENSION( nIndex )	);
}

//----------------------- MainWindow ---------------------------
MainWindow::MainWindow()
{
	m_bLButtonDown		= FALSE		;
	m_bRButtonDown		= FALSE		;

	m_bLCtrl			= FALSE		;
	m_bRCtrl			= FALSE		;

	m_nFoundPolygonIndex= 0			;

	m_bLoaded			= FALSE		;
	m_pCurrentGeometry	= NULL		;

	m_bForwardKeyDown	= FALSE		;
	m_bBackwardKeyDown	= FALSE		;
	m_bStepLeftKeyDown	= FALSE		;
	m_bStepRightKeyDown	= FALSE		;	
	m_bLiftUpKeyDown	= FALSE		;
	m_bLiftDownKeyDown	= FALSE		;
	m_bRotateCCWDown	= FALSE		;
	m_bRotateCWDown		= FALSE		;
	m_bUp				= FALSE		;
	m_bDown				= FALSE		;

	m_SelectedPosX1		= -1		;
	m_SelectedPosY1		= -1		;
	m_SelectedPosX2		= -1		;
	m_SelectedPosY2		= -1		;
	
	//m_pGlyph			= NULL		;

	m_bGeometryAdjust	= FALSE		;
	m_pSelectedGeometry1= NULL		;
	m_pSelectedGeometry2= NULL		;
	
	m_bTileSelecting	= FALSE		;
	m_ulPrevTime		= 0			;

	m_fCameraPanInitial	= 15.0f					;
	m_fCameraPanUnit	= 5.0f					;
	m_fCameraPan		= m_fCameraPanInitial	;

	m_nCurrentTileIndexX	= -1;
	m_nCurrentTileIndexZ	= -1;

	m_pAreaSphere			= NULL	;

	m_bObjectBrushing		= OBJECT_NONE	;
	m_pVertexBuffer			= NULL	;
	m_pVertexIndex			= NULL	;

	m_pVertexBuffer2		= NULL	;
	m_pVertexIndex2			= NULL	;
	m_pCameraSector			= NULL	;

#ifdef _PROFILE_
	m_bShowProfile	= FALSE;
	m_pCurNode = NULL;//AuProfileManager::Root;
	m_iCommand	= -1;
	m_iMyCurPage = 0;

	m_iCheckIndex = 0;
	for(int i=0;i<10;++i)
		m_pCheckedNode[i] = NULL;
	m_iProfileMode = 0;
#endif

	m_bCopyObject	= FALSE;
}

MainWindow::~MainWindow()
{

}

namespace NS_MAPTOOL_INI
{
	const RwUInt32	AMT_MAX_STR										= 256								;
	LPCSTR			AMT_INI_PATH									= "INI\\"							;
	LPCSTR			AMT_CHARACTER_TEMPLATE_PUBLIC_INI_NAME			= "CharacterTemplatePublic.ini"		;
	LPCSTR			AMT_CHARACTER_TEMPLATE_CLIENT_INI_NAME			= "CharacterTemplateClient.ini"		;
	LPCSTR			AMT_CHARACTER_TEMPLATE_ANIMATION_INI_NAME		= "CharacterTemplateAnimation.ini"	;
	LPCSTR			AMT_CHARACTER_TEMPLATE_CUSTOMIZE_INI_NAME		= "CharacterTemplateCustomize.ini"	;
	LPCSTR			AMT_CHARACTER_TEMPLATE_SKILL_INI_NAME			= "CharacterTemplateSkill.ini"		;
	LPCSTR			AMT_CHARACTER_TEMPLATE_SKILLSOUND_INI_NAME		= "CharacterTemplateSkillSound.ini"	;
	LPCSTR			AMT_CHARACTER_TEMPLATE_EVENT_EFFECT_INI_NAME	= "CharacterTemplateEventEffect.ini";
};

BOOL MainWindow::OnInit()		// ÃÊ±âÈ­ , µ¥ÀÌÅ¸ ¼öÁý & ±×·¡ÇÈ µ¥ÀÌÅ¸ ·Îµù.	WindowUI ÀÇ°æ¿ì OpenµÅ´Â ½ÃÁ¡, Full UIÀÇ °æ¿ì SettingµÅ´Â ½ÃÁ¡.
{
	char	path[ 1024 ];
	GetCurrentDirectory( 1024 , path );
	TRACE( "ÇöÀç ½ÇÇà À§Ä¡.. \"%s\"\n" , path );

	OnBrushUpdate();

	// ÁÂÇ¥Ãà »ý¼º.
	AxisCreate();

	// Character TemplateÀ» ÀÐÀÚ

	theApp.SetStartUpProcess( 41 );

	TRACE( "½ºÅ³ ÅÛÇÃ¸´ ·Îµù\n" );
	if( g_pcsAgpmSkill && !g_pcsAgpmSkill->StreamReadTemplate(".\\Ini\\SkillTemplate.ini") )
	{
		::MessageBox( NULL , "ÙéÛö?ö¢ÐüÒöÙ¼÷ù¡£Ù¼÷ùÊ¦ÒöÚ±ïá?Ê¥?¡£" , "¸ÊÅø" , MB_ICONERROR | MB_OK );
	}

	// netong Ãß°¡ (AgcmEventBindingDlg °ü·Ã)
	theApp.SetStartUpProcess( 42 );
	TRACE( "Ä³¸¯ÅÍ Å¸ÀÔ ·Îµù\n" );
	if( g_pcsAgpmFactors && !g_pcsAgpmFactors->CharacterTypeStreamRead(".\\Ini\\CharType.ini", FALSE ) )
	{
		::MessageBox( NULL , "Ä³¸¯ÅÍ Å¸ÀÔ ÀÐ±â¿¡ ½ÇÆÐÇß½À´Ï´Ù. Ä³¸¯ÅÍ Å¸ÀÔÀÌ Á¦´ë·Î ·ÎµåµÅÁö ¾Ê¾ÒÀ» ¼ö ÀÖ½À´Ï´Ù." , "¸ÊÅø" , MB_ICONERROR | MB_OK );
	}
	// netong Ãß°¡
	theApp.SetStartUpProcess( 43 );

	try
	{
		TRACE( "Ä³¸¯ÅÍ ÅÛÇÃ¸´ ·Îµù\n" );

		using namespace NS_MAPTOOL_INI;
		CHAR szIniPathNamePublic[AMT_MAX_STR];
		CHAR szIniPathNameClient[AMT_MAX_STR];
		CHAR szIniPathNameAnimation[AMT_MAX_STR];
		CHAR szIniPathNameCustomize[AMT_MAX_STR];
		CHAR szIniPathNameSkill[AMT_MAX_STR];
		CHAR szIniPathNameSkillSound[AMT_MAX_STR];
		CHAR szIniPathNameEventEffect[AMT_MAX_STR];

		sprintf( szIniPathNamePublic,		"%s%s", AMT_INI_PATH, AMT_CHARACTER_TEMPLATE_PUBLIC_INI_NAME );
		sprintf( szIniPathNameClient,		"%s%s", AMT_INI_PATH, AMT_CHARACTER_TEMPLATE_CLIENT_INI_NAME );
		sprintf( szIniPathNameAnimation,	"%s%s", AMT_INI_PATH, AMT_CHARACTER_TEMPLATE_ANIMATION_INI_NAME );
		sprintf( szIniPathNameCustomize,	"%s%s", AMT_INI_PATH, AMT_CHARACTER_TEMPLATE_CUSTOMIZE_INI_NAME );
		sprintf( szIniPathNameSkill,		"%s%s", AMT_INI_PATH, AMT_CHARACTER_TEMPLATE_SKILL_INI_NAME );
		sprintf( szIniPathNameSkillSound,	"%s%s", AMT_INI_PATH, AMT_CHARACTER_TEMPLATE_SKILLSOUND_INI_NAME );
		sprintf( szIniPathNameEventEffect,	"%s%s", AMT_INI_PATH, AMT_CHARACTER_TEMPLATE_EVENT_EFFECT_INI_NAME );
		
		CHAR szOutput[AMT_MAX_STR];

		bool bError = false;
		CHAR		szErrorCode[AMT_MAX_STR];
		CFileFind	csFind;
		if(csFind.FindFile(szIniPathNamePublic) == false ) {
			bError = true;
			sprintf(szOutput, "%s°¡ ¾ø½À´Ï´Ù!!!\n", szIniPathNamePublic);
		}
		if(csFind.FindFile(szIniPathNameClient) == false ) {
			bError = true;
			sprintf(szOutput, "%s°¡ ¾ø½À´Ï´Ù!!!\n", szIniPathNameClient);
		}
		if(csFind.FindFile(szIniPathNameAnimation) == false ) {
			bError = true;
			sprintf(szOutput, "%s°¡ ¾ø½À´Ï´Ù!!!\n", szIniPathNameAnimation);
		}
		if(csFind.FindFile(szIniPathNameCustomize) == false ) {
			bError = true;
			sprintf(szOutput, "%s°¡ ¾ø½À´Ï´Ù!!!\n", szIniPathNameCustomize);
		}
		if(csFind.FindFile(szIniPathNameSkill) == false ) {
			bError = true;
			sprintf(szOutput, "%s°¡ ¾ø½À´Ï´Ù!!!\n", szIniPathNameSkill);
		}
		if(csFind.FindFile(szIniPathNameSkillSound) == false ) {
			bError = true;
			sprintf(szOutput, "%s°¡ ¾ø½À´Ï´Ù!!!\n", szIniPathNameSkillSound);
		}
		if(csFind.FindFile(szIniPathNameEventEffect) == false ) {
			bError = true;
			sprintf(szOutput, "%s°¡ ¾ø½À´Ï´Ù!!!\n", szIniPathNameEventEffect);
		}
		csFind.Close();
		if( bError ) {
			::MessageBox(NULL , szOutput, "¸ÊÅø" , MB_OK | MB_ICONERROR);
			throw;
		}

		if(!g_pcsAgpmCharacter->StreamReadTemplate( szIniPathNamePublic, szErrorCode,	FALSE ) )
		{
			::MessageBox( NULL , szIniPathNamePublic, "¸ÊÅø" , MB_OK | MB_ICONERROR );
			throw;
		}
		if(!g_pcsAgcmCharacter->StreamReadTemplate( szIniPathNameClient, szErrorCode, FALSE))
		{
			::MessageBox( NULL , szIniPathNameClient, "¸ÊÅø" , MB_OK | MB_ICONERROR );
			throw;
		}
		if(!g_pcsAgcmCharacter->StreamReadTemplateAnimation( szIniPathNameAnimation, szErrorCode, FALSE))
		{
			::MessageBox( NULL , szIniPathNameAnimation, "¸ÊÅø" , MB_OK | MB_ICONERROR );
			throw;
		}
		if(!g_pcsAgcmCharacter->StreamReadTemplateCustomize( szIniPathNameCustomize, szErrorCode, FALSE))
		{
			::MessageBox( NULL , szIniPathNameCustomize, "¸ÊÅø" , MB_OK | MB_ICONERROR );
			throw;
		}
		if(!g_pcsAgcmSkill->StreamReadTemplateSkill( szIniPathNameSkill, szErrorCode, FALSE))
		{
			::MessageBox( NULL , szIniPathNameSkill, "¸ÊÅø" , MB_OK | MB_ICONERROR );
			throw;
		}
		if(!g_pcsAgcmSkill->StreamReadTemplateSkillSound( szIniPathNameSkillSound, szErrorCode, FALSE))
		{
			::MessageBox( NULL , szIniPathNameSkillSound, "¸ÊÅø" , MB_OK | MB_ICONERROR );
			throw;
		}
		if(!g_pcsAgcmEventEffect->StreamReadTemplate( szIniPathNameEventEffect, szErrorCode, FALSE))
		{
			::MessageBox( NULL , szIniPathNameEventEffect, "¸ÊÅø" , MB_OK | MB_ICONERROR );
			throw;
		}
		//@} kday

	}
	CRASH_SAFE_MESSAGE( "ÄÉ¸¯ÅÍ ÅÛÇÃ¸´ ·Îµù" );

	theApp.SetStartUpProcess( 45 );
	TRACE( "¾ÆÀÌÅÛ ÅÛÇÃ¸´ ·Îµù\n" );
	if( g_pcsAgpmItem && !g_pcsAgpmItem->StreamReadTemplates( "INI\\ItemTemplateEntry.ini", NULL, FALSE ) )
	{
		::MessageBox( NULL , "¾ÆÀÌÅÛ ÅÛÇÃ¸´ ÀÐ±â¿¡ ½ÇÆÐÇß½À´Ï´Ù. ÅÛÇÃ¸´ÀÌ Á¦´ë·Î ·ÎµåµÅÁö ¾Ê¾ÒÀ» ¼ö ÀÖ½À´Ï´Ù." , "¸ÊÅø" , MB_ICONERROR | MB_OK );
	}
	
	theApp.SetStartUpProcess( 47 );
	if( g_pcsAgcmPreLODManager )
	{
		g_pcsAgcmPreLODManager->CharPreLODStreamRead(".\\Ini\\CharacterPreLOD.ini");
		g_pcsAgcmPreLODManager->ItemPreLODStreamRead(".\\Ini\\ItemPreLOD.ini");
	}

	theApp.SetStartUpProcess( 49 );
	if (g_pcsAgcmWater)
	{
		try
		{
			if( g_pcsAgcmWater->LoadStatusInfoT1FromINI( "./Ini/WaterStatusT1.ini" , FALSE ) )
			{
				// Do nothing
			}
			else
			{
				::MessageBox( NULL , "./Ini/WaterStatusT1.ini ÆÄÀÏ ÀÐ±â¿¡ ½ÇÆÐÇß¾î¿ä. È®ÀÎ¹Ù¶÷" , "¸ÊÅø" , MB_OK | MB_ICONERROR );

				return FALSE;
			}
			
			if( g_pcsAgcmWater->LoadHWaterStatusFromINI( "./Ini/HWaterStatus.ini" , FALSE ) )
			{
				// Do nothing
			}
			else
			{
				::MessageBox( NULL , "./Ini/HWaterStatus.ini ÆÄÀÏ ÀÐ±â¿¡ ½ÇÆÐÇß¾î¿ä. È®ÀÎ¹Ù¶÷" , "¸ÊÅø" , MB_OK | MB_ICONERROR );

				return FALSE;
			}
		}
		catch( ... )
		{
			if( IDYES == ::MessageBox( NULL , "./Ini/WaterStatusT1.ini ÆÄÀÏ ÀÐ´Ù°¡ ÇÁ·Î±×·¥ Á×¾ú¾î¿ä . ÇöÀç ÇÁ·Î±×·¥À» Á×ÀÌ°Ú½À´Ï±î?" , "¸ÊÅø" , MB_YESNOCANCEL | MB_ICONERROR ) )
			{
				throw;
			}
		}
	}

	theApp.SetStartUpProcess( 50 );
	if (g_pcsAgcmGrass)
	{
		try
		{
			if( g_pcsAgcmGrass->LoadGrassInfoFromINI("./Ini/GrassTemplate.ini",FALSE) )
			{
				// do noting
			}
			else
			{
				::MessageBox( NULL , "./Ini/GrassTemplate.ini ÆÄÀÏ ÀÐ±â¿¡ ½ÇÆÐÇß¾î¿ä. È®ÀÎ¹Ù¶÷" , "¸ÊÅø" , MB_OK | MB_ICONERROR );

				return FALSE;
			}
		}
		catch( ... )
		{
			if( IDYES == ::MessageBox( NULL , "./Ini/GrassTemplate.ini ÆÄÀÏ ÀÐ´Ù°¡ ÇÁ·Î±×·¥ Á×¾ú¾î¿ä . ÇöÀç ÇÁ·Î±×·¥À» Á×ÀÌ°Ú½À´Ï±î?" , "¸ÊÅø" , MB_YESNOCANCEL | MB_ICONERROR ) )
			{
				throw;
			}
		}
	}

	theApp.SetStartUpProcess( 52 );

	TRACE( "NPCTrade ItemGroup ·Îµù\n" );
	if( g_pcsAgpmEventNPCDialog && !g_pcsAgpmEventNPCDialog->LoadMobDialogRes( ".\\Ini\\NPCDialog.txt" , FALSE ) )
	{
		
	}

	/*
	TRACE( "NPCTrade ItemGroup ·Îµù\n" );
	if( g_pcsAgpmEventNPCDialog && !g_pcsAgpmEventNPCDialog->LoadNPCDialogRes( ".\\Ini\\NPCDialog.txt" , FALSE ) )
	{

	}
	*/

	theApp.SetStartUpProcess( 53 );
	TRACE( "NPCTrade ItemGroup ·Îµù\n" );
	if( g_pcsAgpmEventNPCTrade && !g_pcsAgpmEventNPCTrade->LoadNPCTradeRes( ".\\Ini\\NPCTradeItemList.txt" , FALSE ) )
	{

	}

	theApp.SetStartUpProcess( 54 );
	{
		// ¿¡¸®¾î ¾ÆÅä¹Í »ý¼º..
		VERIFY( m_pAreaSphere = g_pcsAgcmEventNature->CreateSkyAtomic() );

		ChangeSphereAlpha			( 15 );
	}

	theApp.SetStartUpProcess( 55 );
	// ¹öÅØ½º ¹öÆÛ ¾ó·Ï.
	m_pVertexBuffer	= new RwIm3DVertex[ ( MAP_DEFAULT_DEPTH + 1 ) * ( MAP_DEFAULT_DEPTH + 1 ) ]; 
	m_pVertexIndex	= new RwImVertexIndex[ ( MAP_DEFAULT_DEPTH ) * ( MAP_DEFAULT_DEPTH ) * 4 * 2 ];
	m_pVertexBuffer2= new RwIm3DVertex[ ( MAP_DEFAULT_DEPTH + 1 ) * ( MAP_DEFAULT_DEPTH + 1 ) ]; 
	m_pVertexIndex2	= new RwImVertexIndex[ ( MAP_DEFAULT_DEPTH ) * ( MAP_DEFAULT_DEPTH ) * 2 ];

	theApp.SetStartUpProcess( 80 );

	if( g_pcsApmMap )
	{
		if( g_pcsApmMap->LoadTemplate( "./Ini/" REGIONTEMPLATEFILE ) )
		{
			// ¼º°ø
		}
		else
		{
			// ½ÇÆÐ..
			MD_SetErrorMessage( "¸®Á¯ ÅÛÇÃ¸´ ÆÄÀÏ ÀÐ±â ½ÇÆÐ~" );
		}

		if( g_pcsApmMap->LoadRegion( "./RegionTool/" REGIONFILE ) )
		{
			// ¼º°ø
		}
		else
		{
			// ½ÇÆÐ..
			MD_SetErrorMessage( "¸®Á¯ Á¤º¸ÆÄÀÏ ÆÄÀÏ ÀÐ±â ½ÇÆÐ~" );
		}
	}

	theApp.SetStartUpProcess( 81 );

	if( g_pcsAgpmQuest )
	{
		if( g_pcsAgpmQuest->StreamReadGroup( "./Ini/QuestGroup.ini" , FALSE ) )
		{

		}
		else
		{
			MD_SetErrorMessage( "AgpmQuest Äù½ºÆ® ±×·ìÀÐ±â ½ÇÆÐ" );
		}
	}

	theApp.SetStartUpProcess( 82 );

	if( g_pcsAgpmEventGacha )
	{
		g_pcsAgpmEventGacha->StreamReadGachaTypeTable( "./Ini/GachaTypeTable.txt" , FALSE );
	}

#ifdef _PROFILE_
	AuProfileManager::Reset();
#endif

	theApp.SetStartUpProcess( 85 );
	
	if( g_pcsAgcmShadow2 )
	{
		// ¸¶°íÀÚ (2004-10-01 ¿ÀÈÄ 3:40:33) : ½ÃÀÛÇÒ¶§ 
		g_pcsAgcmShadow2 ->SetEnable( FALSE );
		g_pcsAgcmRender->m_bDrawShadow2 = FALSE;
	}

	if( g_pcsAgcmNatureEffect )
	{
		g_pcsAgcmNatureEffect->LoadNatureEffectInfoFromINI("./ini/NatureEffect.ini",FALSE);
	}

	g_pcsAgcmRender->SetPreRenderAtomicCallback( CBPreRenderAtomicCallback , NULL );

	g_pcsAgcmEventNature->ApplySkySetting();


	return TRUE;
}

void	MainWindow::ChangeSphereAlpha	( INT32 nAlpha )
{
	if( !m_pAreaSphere )		return;

	nAlpha = max( 0, min( nAlpha, 255 ) );

	RpGeometry* pGeometry;
	VERIFY( pGeometry = RpAtomicGetGeometry( m_pAreaSphere ) );

	RpGeometryLock( pGeometry, rpGEOMETRYLOCKPRELIGHT );
	RpGeometrySetFlags( pGeometry, RpGeometryGetFlags( pGeometry ) | rpGEOMETRYMODULATEMATERIALCOLOR );

	RpMaterial* pMaterial;
	VERIFY( pMaterial = RpGeometryGetMaterial( pGeometry , 0 ) );

	RwRGBA newRGBA = *RpMaterialGetColor( pMaterial );
	newRGBA.alpha = nAlpha							;

	RpMaterialSetColor( pMaterial, &newRGBA	);
	RpGeometryUnlock( pGeometry );
}

// ¿ùµå¼½ÅÍ ¾ÆÅä¹ÍÀ» ¸®½ºÆÃÇÏ±â À§ÇÑ°Í..
RpAtomic *	_GetWorldSectorGeometryCallback( RpIntersection* intersection, RpWorldSector* sector, RpAtomic* pAtomic, RwReal distance, void* data )
{
	AuList< _FindWorldSectorGeometryStruct >* pListSector = (	AuList< _FindWorldSectorGeometryStruct > * ) data;

	INT32	index;
	if( ACUOBJECT_TYPE_WORLDSECTOR != AcuObject::GetAtomicType( pAtomic, &index ) )		// ¾ÆÅä¹ÍÀÇ Å¸ÀÔÀÌ ¿ùµå¼½ÅÍ ÀÏ¶§¸¸ µî·Ï..
		return pAtomic;

	_FindWorldSectorGeometryStruct	cs( distance, pAtomic );
	pListSector->AddTail( cs );
	return pAtomic;
}

BOOL MainWindow::OnLButtonDblClk( RsMouseStatus *ms	)
{
	ASSERT( ms );

	m_bLButtonDown		= TRUE	;
	RpClump	* pSelectedClump = NULL;

	switch( GetCurrentMode() )
	{
	case EDITMODE_OBJECT:
		{
			pSelectedClump = AGCMMAP_THIS->GetCursorClump( g_MyEngine.m_pCamera , & ms->pos , g_bUsePickingOnly );
			
			SetObjectSelection( pSelectedClump );

			if ( IsObjectSelected() )
			{
				UpdateObjectUndoInfo();
				return TRUE;
			}
			else
			{
				m_bLButtonDown = FALSE;
			}
			break;
		}
	case EDITMODE_OTHERS:
		{
			if( g_pMainFrame->m_pTileList->m_pOthers->Window_LButtonDblClk( ms ) )
			{
				// do nothing
			}
			else
			{
				g_pMainFrame->m_pTileList->m_pOthers->LButtonDblClkGeometry( &m_Position );
			}
		}
		break;
		
	default:
		break;
	}

	return FALSE;
}

BOOL MainWindow::OnLButtonDown	( RsMouseStatus *ms	)
{
	ASSERT( NULL != ms );

	m_bLButtonDown		= TRUE	;
	m_PrevMouseStatus	= *ms	;

	if( g_pMainFrame->m_Document.IsInFirstPersonViewMode() && !g_pMainFrame->m_pToolBar->GetFPSEditMode() )
	{
		BOOL bMoveFast = GetKeyState(VK_SHIFT) < 0 ? TRUE : FALSE;	// 1ÀÎÄª ½ÃÁ¡¿¡¼­ Ã³¸®..

		AuPOS	posDestPoint;

		AGCMMAP_THIS->m_ulCurTick --;
		if ( AGCMMAP_THIS->GetMapPositionFromMousePosition_tool( (int)ms->pos.x, (int)ms->pos.y,  &posDestPoint.x, &posDestPoint.y, &posDestPoint.z ) )
			g_pcsAgpmCharacter->MoveCharacter( g_pcsAgcmCharacter->m_pcsSelfCharacter, &posDestPoint, MD_NODIRECTION , FALSE, TRUE );

		SetObjectSelection( NULL );

		return TRUE;
	}


	// ¿ì¼± Å¬¸¯µÅ´Â ¾ÆÅä¹ÍÀÌ Á¸ÀçÇÏ´ÂÁö Á¡°Ë.

	RpClump	* pSelectedClump = NULL;

	switch( GetCurrentMode() )
	{
	case EDITMODE_OBJECT:
		{
			if( ISBUTTONDOWN( VK_MENU ) )
			{
				// ¿ÀºêÁ§Æ® ¸ÖÆ¼ ¼¿·º¼Ç..

				if( ISBUTTONDOWN( VK_SHIFT ) )
				{
					// Add..
					m_bObjectBrushing			= OBJECT_SELECTADD			;
				}
				else
				if( ISBUTTONDOWN( VK_CONTROL ) )
				{
					// Subtract..
					m_bObjectBrushing			= OBJECT_SELECTREMOVE			;
				}
				else
				{
					// ¼±ÅÃµÈ ³à¼®µéÀ» ³¯¸²..
					SetObjectSelection( NULL );
					m_bObjectBrushing			= OBJECT_SELECT			;
				}


				m_pointObjectSelectFirst.x	= ( INT32 ) ms->pos.x	;
				m_pointObjectSelectFirst.y	= ( INT32 ) ms->pos.y	;

				m_bLButtonDown = FALSE;
				return TRUE;
			}
			else
			{
				pSelectedClump = AGCMMAP_THIS->GetCursorClump( g_MyEngine.m_pCamera , & ms->pos , g_bUsePickingOnly );
				if( IsObjectSelected() )
				{
					if( pSelectedClump )
					{
						// ¼±ÅÃ‰?³à¼®µé Áß ÇÏ³ªÀÎÁö Á¡°Ë..
						RpClump	* pCurrentClump;
						AuNode< ObjectSelectStruct > * pNode = m_listObjectSelected.GetHeadNode();
						while( pNode )
						{
							pCurrentClump	= pNode->GetData().pClump;

							if( pSelectedClump	== pCurrentClump )
								break;

							m_listObjectSelected.GetNext( pNode );
						}

						// If Found?..
						if( pNode )
						{
							// ¼¿·º¼Ç À¯Áö..
						}
						else
						{
							// Remove Selection..

							SetObjectSelection( pSelectedClump );	
						}
					}
					else
					{
						// ¾ø´Â°æ¿ì..
						// shift 
						SetObjectSelection( NULL );
					}
				}
				else
				{
					SetObjectSelection( pSelectedClump );
				}

				if ( ISBUTTONDOWN( VK_MENU ) )
				{
					// ALT¸¦ ´©¸£°íÀÖÀ¸¸é ¼­ÅÃÀº ¸ðµÎ ¾ø¾îÁü..
					SetObjectSelection( NULL );
				}
				
				if ( IsObjectSelected() )
				{
					UpdateObjectUndoInfo();

					return TRUE;
				}
				else
				{
					m_bLButtonDown = FALSE;
				}
			}
			break;
		}
	case EDITMODE_OTHERS:
		{
			if( g_pMainFrame->m_pTileList->m_pOthers->Window_LButtonDown( ms ) ) return TRUE;

			pSelectedClump = AGCMMAP_THIS->GetCursorClump( g_MyEngine.m_pCamera , & ms->pos , g_bUsePickingOnly );

			if( pSelectedClump )
			{
				if( g_pMainFrame->m_pTileList->m_pOthers->SelectObject( pSelectedClump ) )
				{
					return TRUE;
				}

				// ¸®ÅÏ Æú½º¿¡¼± Áö¿À¸ÞÆ®¸® Ã¼Å©·Î ³Ñ¾î°¨.
			}
		}
		break;

	default:
		break;
	}


	switch( GetCurrentMode() )
	{
	case EDITMODE_GEOMETRY:
		{
			m_uLastGeometryPressTime	= GetTickCount();
			m_bGeometryAdjust			= TRUE			;

			switch( g_pMainFrame->m_Document.m_nBrushType )
			{
			default:
				break;
			case	BRUSHTYPE_EDGETURN		:
				{
					SaveSetChangeMoonee();

					ApDetailSegment * pSegment;
					int posx , posz;

					pSegment = GetCurrentPolygonsTileIndex(
						m_pCurrentGeometry		,
						m_nFoundPolygonIndex	,
						( INT32 ) ms->pos.x		,
						( INT32 ) ms->pos.y		,
						&posx					,
						&posz					);

					if( pSegment && m_pCurrentGeometry )
					{
						// ¸¶°íÀÚ (2004-06-15 ¿ÀÈÄ 3:42:16) : ¿§ÁöÅÏ Ã³¸®.
						m_UndoManager.StartActionBlock( CUndoManager::EDGETURN		);
						m_UndoManager.AddEdgeTurnActionunit( m_pCurrentGeometry , posx , posz , ! pSegment->stTileInfo.GetEdgeTurn() );
						AGCMMAP_THIS->D_SetEdgeTurn( m_pCurrentGeometry , m_pCurrentGeometry->GetCurrentDetail() ,
							posx , posz , ! pSegment->stTileInfo.GetEdgeTurn() );
						UnlockSectors();
						m_UndoManager.EndActionBlock();

						// ¸¶°íÀÚ (2004-06-25 ¿ÀÀü 11:32:42) : ¾÷µ¥ÀÌÆ® Åëº¸..
						g_pMainFrame->m_pTileList->m_pOthers->ChangeSectorGeometry( m_pCurrentGeometry );	
					}
				}
				break;
			case	BRUSHTYPE_GEOMETRY		:
			case	BRUSHTYPE_SMOOTHER		:
				SaveSetChangeGeometry();

				m_UndoManager.StartActionBlock( CUndoManager::GEOMETRY		);
				if( m_pCurrentGeometry && ISBUTTONDOWN( VK_MENU ) )
				{
					UINT	currenttick	= GetTickCount	();
					int				i , j				;
					float			applyheight			= 0;
					float			fSegPosX, fSegPosY	;
					ApWorldSector *	pWorkingSector		;
					stFindMapInfo	FindMapInfo			;
					float			distance			;
					ApDetailSegment	* pSegment	;

					int segmentx , segmentz;

					int x1 = ALEF_LOAD_RANGE_X1 , x2 = ALEF_LOAD_RANGE_X2 , z1 = ALEF_LOAD_RANGE_Y1 , z2 = ALEF_LOAD_RANGE_Y2;
					int	xc , zc;

					xc = m_pCurrentGeometry->GetArrayIndexX();
					zc = m_pCurrentGeometry->GetArrayIndexZ();

					x1 = xc - 1 ;
					z1 = zc - 1 ;
					x2 = xc + 1 ;
					z2 = zc + 1 ;
					
					// ºÒµµÁ® ¸ðµå..
					bool bFirstTime = true;
					for( j = z1 ; j <= z2 ; j ++ )
					{
						for( i = x1 ; i <= x2 ; i ++ )
						{
							// ¾ÆÀÚ Ã£¾Æº¸ÀÚ.
							pWorkingSector = g_pcsApmMap->GetSectorByArray(  i , 0 , j );

							if( NULL == pWorkingSector ) continue;

							if( !__CheckCircumstanceSectorPolygonInfo( pWorkingSector ) ) continue;

							if( pWorkingSector->IsLoadedDetailData() &&
								AGCMMAP_THIS->IsInSectorRadius( g_pcsApmMap->GetSectorByArray( i , 0 , j ) ,
									m_Position.x , m_Position.z , g_pMainFrame->m_Document.m_fBrushRadius )
								)
							{
								// ¾ÆÀÚ.;; 
								// ÀÌÁ¦ °¢°¢ ¼¼±×¸ÕÆ®¸¦ µÑ·¯¼­. ¿ø ¹üÀ§ ³»¿¡ µé¾îÀÖ´Â ¼¼±×¸ÕÆ®¶ó¸é Height¸¦ Àû¿ëÇÑ´Ù.

								for( segmentz = 0 ; segmentz < pWorkingSector->D_GetDepth() ; segmentz ++ )
								{
									for( segmentx = 0 ; segmentx < pWorkingSector->D_GetDepth() ; segmentx ++ )
									{
										pSegment = pWorkingSector->D_GetSegment( segmentx , segmentz , &fSegPosX , &fSegPosY );

										if( pSegment )
										{
											// ¹üÀ§Ã¼Å©
											if( ( distance = ( float ) sqrt( ( fSegPosX - m_Position.x ) * ( fSegPosX - m_Position.x ) + ( fSegPosY - m_Position.z ) * ( fSegPosY - m_Position.z ) ) )
												> g_pMainFrame->m_Document.m_fBrushRadius ) continue;
											
											// ÀÚ ÀÌÁ¦ ¿Ã¸®ÀÚ..
											
											if( GetWorldSectorInfo( &FindMapInfo , pWorkingSector , segmentx , segmentz ) )
											{
												if( bFirstTime )
												{
													bFirstTime	= false								;
													applyheight	= FindMapInfo.pSegment->height	;
												}

												if( applyheight < FindMapInfo.pSegment->height ) applyheight = FindMapInfo.pSegment->height;
											}
										}
										/////////////////////////////////////
									}
								}
							}
							// ¾ÆÀÚ¾ÆÀÚ.
						}
					}

					m_fApplyheight = __LimitHeight( applyheight );

				}

				break;
			case	BRUSHTYPE_VERTEXCOLOR	:
				{
					SaveSetChangeMoonee();
					m_UndoManager.StartActionBlock( CUndoManager::VERTEXCOLOR	);
				}
				break;
			}
			
		}
		return TRUE;
	default:
		break;
	}
	
	// ÁöÇü ¼±ÅÃ.
	// ÀÌ m_pFoundSector´Â Idle Time ¸¶´Ù Ã¼Å©ÇØ¼­ ÀúÀåµÅ´Â ³à¼®ÀÌ´Ù.
	if( m_pCurrentGeometry )
	{
		switch( GetCurrentMode() )
		{
		case EDITMODE_OTHERS:
			{
				g_pMainFrame->m_pTileList->m_pOthers->LButtonDownGeometry( &m_Position );
			}
			break;
		case EDITMODE_OBJECT:
			{
				if( g_pMainFrame->m_pTileList->m_pObjectWnd->IsObjectSelected() )
				{
					SaveSetChangeObjectList();

					if( ISBUTTONDOWN( VK_CONTROL ) )
					{
						m_bObjectBrushing	= OBJECT_BRUSHING;
					}
					else
					{
						// ¿ÀºêÁ§Æ® »ðÀÔ..
						m_bObjectBrushing	= OBJECT_NONE;

						VERIFY( g_pMainFrame->m_pTileList->m_pObjectWnd->AddObject(&m_Position) );
					}
				}
				else
					DisplayMessage( "¿ÀºêÁ§Æ®°¡ ¼±ÅÃµÅÁö¾Ê¾Ò½À´Ï´Ù." );
			}
			break;

		case EDITMODE_TILE:
			{
				SaveSetChangeMoonee();

				ApDetailSegment	*	pSelectedSegment	;
				int				posx, posz		;

				pSelectedSegment = GetCurrentPolygonsTileIndex(
					m_pCurrentGeometry		,
					m_nFoundPolygonIndex	,
					( INT32 ) ms->pos.x		,
					( INT32 ) ms->pos.y		,
					&posx					,
					&posz					);


				if( pSelectedSegment == NULL ) return TRUE;
		
				switch( g_pMainFrame->m_Document.m_nDWSectorSetting )
				{
				case DWSECTOR_ALLROUGH			:
					{					
						ApDetailSegment* pSegment = m_pCurrentGeometry->D_GetSegment( posx , posz );

						switch( GetCurrentLayer() )
						{
						case TILELAYER_ALPHATILEING		:
							{
									m_bTileAdjust	= TRUE;
									m_UndoManager.StartActionBlock( CUndoManager::TILE );
							}
							break;

						default:
							{
								// Control ¹öÆ°À» ´©¸£°í ÀÖÀ¸¸é ºí·°ÁöÁ¤..
								if( ISBUTTONDOWN( VK_CONTROL ) )
								{
									m_bGeometrySelection	= TRUE					;

									m_SelectedPosX1			= posx					;
									m_SelectedPosY1			= posz					;
									m_SelectedPosX2			= posx					;
									m_SelectedPosY2			= posz					;
									m_pSelectedGeometry1	= m_pCurrentGeometry	;
									m_pSelectedGeometry2	= m_pCurrentGeometry	;
								}
								else
								{
									m_bTileAdjust	= TRUE;
									m_UndoManager.StartActionBlock( CUndoManager::TILE );
								}
							}
							break;

						}
					}
					break;
				case DWSECTOR_EACHEACH			:
				case DWSECTOR_ALLDETAIL			:
				case DWSECTOR_ACCORDINGTOCURSOR	:
					{
						int	posx, posz;
						ApDetailSegment* pSegment = GetCurrentPolygonsTileIndex( m_pCurrentGeometry, m_nFoundPolygonIndex, (INT32)ms->pos.x, (INT32)ms->pos.y, &posx, &posz );
						if( pSegment )
						{
							switch( GetCurrentLayer() )
							{
							case TILELAYER_ALPHATILEING		:
								{
									SaveSetChangeMoonee();

									m_bTileAdjust	= TRUE;
									m_UndoManager.StartActionBlock( CUndoManager::TILE );
								}
								break;

							default:
								{
									// Control ¹öÆ°À» ´©¸£°í ÀÖÀ¸¸é ºí·°ÁöÁ¤..
									if( ISBUTTONDOWN( VK_CONTROL ) )
									{
										m_bGeometrySelection	= TRUE					;

										m_SelectedPosX1			= posx					;
										m_SelectedPosY1			= posz					;
										m_SelectedPosX2			= posx					;
										m_SelectedPosY2			= posz					;
										m_pSelectedGeometry1	= m_pCurrentGeometry	;
										m_pSelectedGeometry2	= m_pCurrentGeometry	;
									}
									else
									{
										m_bTileAdjust	= TRUE;
										m_UndoManager.StartActionBlock( CUndoManager::TILE );
									}
								}
								break;

							}
						}

					}
					break;
				}
			}
			break;
		}
	}
	
	return FALSE;
}

BOOL MainWindow::OnRButtonDown	( RsMouseStatus *ms	)
{
	m_bRButtonDown		= TRUE;
	m_PrevMouseStatus	= *ms	;
	m_PrevRButtonPos	= *ms	;

	switch( GetCurrentMode() )
	{
	case EDITMODE_OTHERS:
		{
			if( g_pMainFrame->m_pTileList->m_pOthers->Window_RButtonDown( ms ) ) return TRUE;
			g_pMainFrame->m_pTileList->m_pOthers->RButtonDownGeometry( &m_Position );
		}
		break;
	case EDITMODE_OBJECT:
	default:
		break;
	}

	return FALSE;
}

BOOL MainWindow::OnRButtonUp( RsMouseStatus *ms	)
{
	m_bRButtonDown	= FALSE;

	switch( GetCurrentMode() )
	{
	case EDITMODE_OBJECT:
		{
			if(	abs( m_PrevRButtonPos.pos.x - ms->pos.x ) < 5	&& abs( m_PrevRButtonPos.pos.y - ms->pos.y ) < 5	)
			{
				RpClump* pClump = AGCMMAP_THIS->GetCursorClump( g_MyEngine.m_pCamera , & ms->pos , g_bUsePickingOnly );
				if( !pClump )		return FALSE;

				AuNode< ObjectSelectStruct >* pNode	= m_listObjectSelected.GetHeadNode();
				while( pNode )
				{
					RpClump* pClumpInList	= pNode->GetData().pClump;
					if( pClumpInList == pClump )
						break;

					m_listObjectSelected.GetNext( pNode );
				}

				if( pNode )
				{
					// ¸®½ºÆ® ÃÖ»ó´ÜÀ¸·Î ¿Å°ÜµÒ..
					ObjectSelectStruct	stObject;
					stObject = pNode->GetData();
					m_listObjectSelected.RemoveNode( pNode );
					m_listObjectSelected.AddHead( stObject );
				}
				else		// ¼±ÅÃÇÑ°Å Áß¿¡ ¾øÀ¸¸é... ¼±ÅÃÇØ¹ö¸²..
					SetObjectSelection( pClump );		

				// ¿ÀºêÁ§Æ® ¸Þ´º È£ÃâÇÔ..
				POINT	point;
				GetCursorPos( &point );

				CMenu	menu;
				menu.LoadMenu( IDR_MAIN_OBJECT_POPUP );
				CMenu* pMenu = menu.GetSubMenu(0);
				TrackPopupMenu( pMenu->GetSafeHmenu(), TPM_TOPALIGN | TPM_LEFTALIGN, point.x, point.y, 0 , g_pMainFrame->m_wndView, NULL );
			}
		}
		break;
	case EDITMODE_OTHERS:
		{
			if( g_pMainFrame->m_pTileList->m_pOthers->Window_RButtonUp( ms ) )
				return TRUE;

			g_pMainFrame->m_pTileList->m_pOthers->RButtonUpGeometry( &m_Position );
		}
		break;
	default:
		break;
	}

	return FALSE;
}


void MainWindow::RenderStateClear()
{
	// Ç® ±×¸®±â Àü renderstate ÃÊ±âÈ­
	RwD3D9SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	RwD3D9SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	RwD3D9SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA  );
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA   );
	RwD3D9SetRenderState( D3DRS_BLENDOP , D3DBLENDOP_ADD );
}
/*
 *****************************************************************************
 */
static RpClump *
_destroyClumpCB(RpClump *c, void *d)
{
    RpWorldRemoveClump( g_pEngine->m_pWorld , c );
    RpClumpDestroy(c);

    return c;
}


void MainWindow::OnClose()
{
	if( m_pAreaSphere )
		RpAtomicDestroy( m_pAreaSphere );

	if( m_pVertexBuffer	) delete [] m_pVertexBuffer	; 
	if( m_pVertexIndex	) delete [] m_pVertexIndex	; 
	if( m_pVertexBuffer2) delete [] m_pVertexBuffer2; 
	if( m_pVertexIndex2	) delete [] m_pVertexIndex2	; 
}

void	MainWindow::SelectedObjectTurning( FLOAT	fDegree )
{
	AuNode< ObjectSelectStruct >	*	pNode	;
	ApdObject		*	pstApdObject	;
	int					oid		;
	RpClump *			pClump	;
	INT32				nType	;

	// ÁßÁ¡À» ±¸ÇÔ..
	RwV3d	center;
	// GetObjectCenter( & center );
	{
		double	dbCenterX = 0.0 , dbCenterZ = 0.0 , dbCenterY = 0.0;;

		{
			RwFrame	* pFrame;

			// ±×·ì¿¡ ¼ÓÇÑ ³à¼®ÀÌ ÀÖ´ÂÁö Á¡°Ë..
			pNode	= m_listObjectSelected.GetHeadNode();
			while( pNode )
			{
				pClump	= pNode->GetData().pClump;
				pFrame = RpClumpGetFrame( pClump );
				dbCenterX += pFrame->ltm.pos.x;
				dbCenterZ += pFrame->ltm.pos.z;
				dbCenterY += pFrame->ltm.pos.y;

				m_listObjectSelected.GetNext( pNode );
			}

			dbCenterX /= ( double ) m_listObjectSelected.GetCount();
			dbCenterZ /= ( double ) m_listObjectSelected.GetCount();
			dbCenterY /= ( double ) m_listObjectSelected.GetCount();
		}

		center.x	= ( FLOAT ) dbCenterX;
		center.y	= ( FLOAT ) dbCenterY;
		center.z	= ( FLOAT ) dbCenterZ;
	}
	
	RwMatrix	* pMatrix = RwMatrixCreate();
	{
		RwV3d		vToCetner;
		vToCetner.x		= -center.x;
		vToCetner.z		= -center.z;
		vToCetner.y		= 0.0f;

		RwV3d		vToOrigin;
		vToOrigin.x		= center.x;
		vToOrigin.z		= center.z;
		vToOrigin.y		= 0.0f;

		RwV3d		vAxis;
		vAxis.x		= 0.0f;
		vAxis.z		= 0.0f;
		vAxis.y		= 1.0f;

		RwMatrixTranslate( pMatrix , & vToCetner ,rwCOMBINEREPLACE );
		RwMatrixRotate( pMatrix , & vAxis , fDegree , rwCOMBINEPOSTCONCAT );
		RwMatrixTranslate( pMatrix , & vToOrigin , rwCOMBINEPOSTCONCAT );
	}

	pNode	= m_listObjectSelected.GetHeadNode();

	while( pNode )
	{
		pClump	= pNode->GetData().pClump;

		{
			oid = 0;
			// Object ID¸¦ ¾ò¾î³¿.
			nType = AcuObject::GetClumpType( pClump, &oid );

			switch( AcuObject::GetType( nType ) )
			{
			case	ACUOBJECT_TYPE_OBJECT			:
				break;
			default:
				oid = 0;
				break;
			}
			
			// ÇØ´ç ¿ÀºêÁ§Æ®ÀÇ Æ÷ÀÎÅÍ¸¦ ¾ò¾î³¿.
			pstApdObject	= g_pcsApmObject->	GetObject		( oid			);
			if( pstApdObject ) 
			{
				// °¢µµ Á¶Àý
				pstApdObject->m_fDegreeY = _InRadianRange( pstApdObject->m_fDegreeY + fDegree );

				// À§Ä¡ Á¶Àý..
				{
					RwV3d	pos;
					pos.x	= pstApdObject->m_stPosition.x;
					pos.y	= pstApdObject->m_stPosition.y;
					pos.z	= pstApdObject->m_stPosition.z;

					RwV3dTransformPoint( & pos , & pos , pMatrix );

					pstApdObject->m_stPosition.x	= pos.x;
					pstApdObject->m_stPosition.z	= pos.z;

					if( g_pMainFrame->m_pTileList->m_pObjectWnd->m_bSnapToHeight &&
						m_listObjectSelected.GetCount() > 1) 
					{
						// ¸¶°íÀÚ (2005-04-12 ¿ÀÀü 10:49:54) : 
						// ÇÑ°³ÀÏ¶© ³ôÀÌÁ¶Àý ¾ÈÇÔ..
						pstApdObject->m_stPosition.y	= AGCMMAP_THIS->GetHeight_Lowlevel_HeightOnly( pos.x , pos.z );
					}
				}
				g_pcsAgcmObject->SetupObjectClump_TransformOnly( pstApdObject );
				SaveSetChangeObjectList();
			}
		}

		m_listObjectSelected.GetNext( pNode );
	}

	RwMatrixDestroy( pMatrix );
}

BOOL MainWindow::OnKeyDown		( RsKeyStatus *ks	)
{
//	MTRACE( "KEY DOWN = '%c'\n" , ks->keyCharCode );
    switch( ks->keyCharCode )
    {
	case rsESC:
		{
			switch( GetCurrentMode() )
			{
			default:
				SetObjectSelection( NULL );
				break;

			case EDITMODE_OTHERS:
				{
					if( g_pMainFrame->m_pTileList->m_pOthers->Window_KeyDown( ks ) ) return TRUE;
				}
				break;
			}
		}
		break;
	case	'c':
		{
			switch( GetCurrentMode() )
			{
			case EDITMODE_OBJECT:
				{
					if( m_listObjectSelected.GetCount() )
					{
						// Ä«ÇÇ object flag ¼³Á¤.
						m_bCopyObject	= TRUE;
					}

				}
				break;
			default:
				break;
			}
		}
		break;
	case	't':
		{
			switch( GetCurrentMode() )
			{
			case EDITMODE_OBJECT:
				{
					SelectedObjectTurning( 45.0f );

					// ¾ðµÎ Á¤º¸..
					AuNode< ObjectSelectStruct >	*	pNode	;
					ApdObject		*	pstApdObject	;
					int					oid		;
					RpClump *			pClump	;
					INT32				nType	;

					BOOL					bFoundChildClump = FALSE;
					ObjectSelectStruct		* pObjectSelectStruct;
					ObjectUndoInfo	changedInfo		;

					pNode	= m_listObjectSelected.GetHeadNode();
					m_UndoManager.StartActionBlock		( CUndoManager::OBJECTMANAGE			);

					while( pNode )
					{
						pObjectSelectStruct	= &pNode->GetData();
						pClump	= pNode->GetData().pClump;

						oid = 0;
						// Object ID¸¦ ¾ò¾î³¿.
						nType = AcuObject::GetClumpType( pClump, &oid );

						switch( AcuObject::GetType( nType ) )
						{
						case	ACUOBJECT_TYPE_OBJECT			:
							break;
						default:
							oid = 0;
							break;
						}
						
						// ÇØ´ç ¿ÀºêÁ§Æ®ÀÇ Æ÷ÀÎÅÍ¸¦ ¾ò¾î³¿.
						pstApdObject	= g_pcsApmObject->	GetObject		( oid			);
						if( pstApdObject ) 
						{
							changedInfo.oid			= pObjectSelectStruct->undoInfo.oid;
							changedInfo.stScale		= pstApdObject->m_stScale		;	// Scale Vector
							changedInfo.stPosition	= pstApdObject->m_stPosition	;	// Base Position
							changedInfo.fDegreeX	= pstApdObject->m_fDegreeX		;	// Rotation Degree
							changedInfo.fDegreeY	= pstApdObject->m_fDegreeY		;	// Rotation Degree

							m_UndoManager.AddObjectActionUnit	( &pObjectSelectStruct->undoInfo , &changedInfo	);
						}

						m_listObjectSelected.GetNext( pNode );
					}

					m_UndoManager.EndActionBlock();

				}
				break;
			default:
				break;
			}
		}
		break;

	case	'<'	:
	case	'.'	:
		{
			// ¸¶°íÀÚ (2004-05-29 ¿ÀÈÄ 9:54:05) : Å¸ÀÏ ¸ðµå ½ºÄÉÀÏ º¯°æ Å°..
			switch( GetCurrentMode() )
			{
			case EDITMODE_TILE:
				{
					AuNode< int > * pNode = g_pMainFrame->m_Document.m_listSelectedTileIndex.GetHeadNode();
					if( pNode )
					{
						int * pData = &pNode->GetData();

						int nDimension = GET_TEXTURE_DIMENSION( *pData );
						nDimension ++;

						if( nDimension > 8 ) nDimension = 8;

						*pData = SET_TEXTURE_DIMENSION( *pData , nDimension );

						g_pMainFrame->m_pTileList->m_pTabWnd->UpdateDimensino();
					}		
				}
				break;
			case EDITMODE_OTHERS:
				{
					if( g_pMainFrame->m_pTileList->m_pOthers->Window_KeyDown( ks ) ) return TRUE;
				}
				break;
			}

		}
		break;
	case	'>'	:
	case	','	:
		{
			// ¸¶°íÀÚ (2004-05-29 ¿ÀÈÄ 9:54:05) : Å¸ÀÏ ¸ðµå ½ºÄÉÀÏ º¯°æ Å°..
			switch( GetCurrentMode() )
			{
			case EDITMODE_TILE:
				{
					AuNode< int > * pNode = g_pMainFrame->m_Document.m_listSelectedTileIndex.GetHeadNode();
					if( pNode )
					{
						int * pData = &pNode->GetData();

						int nDimension = GET_TEXTURE_DIMENSION( *pData );
						nDimension --;

						if( nDimension < 1 ) nDimension = 1;

						*pData = SET_TEXTURE_DIMENSION( *pData , nDimension );

						g_pMainFrame->m_pTileList->m_pTabWnd->UpdateDimensino();
					}	
				}
				break;
			case EDITMODE_OTHERS:
				{
					if( g_pMainFrame->m_pTileList->m_pOthers->Window_KeyDown( ks ) ) return TRUE;
				}
				break;
			}
		}
		break;
    case	rsLCTRL:
		{
			m_bLCtrl = TRUE;
		}
		break;
	
    case	rsRCTRL:
		{
			m_bRCtrl = TRUE;
		}
		break;

	case	KEYFORWARD		:	m_bUp				=	TRUE;	m_uLastKeyPressTime = GetTickCount(); break;
	case	KEYBACKWARD		:	m_bDown				=	TRUE;	m_uLastKeyPressTime = GetTickCount(); break;
    case	rsLEFT			:
	case	KEYSTEPLEFT		:
		{
			if( ISBUTTONDOWN( VK_CONTROL ) )
			{
				g_pMainFrame->OnSelectAll();
			}
			else
			{
				m_bStepLeftKeyDown	=	TRUE;
				m_uLastKeyPressTime = GetTickCount();
			}
		}
		break;
    case	rsRIGHT			:
	case	KEYSTEPRIGHT	:	m_bStepRightKeyDown	=	TRUE;	m_uLastKeyPressTime = GetTickCount(); break;
	case	KEYLIFTUP		:	m_bLiftUpKeyDown	=	TRUE;	m_uLastKeyPressTime = GetTickCount(); break;
	case	KEYLIFTDOWN		:	
		if( ISBUTTONDOWN( VK_CONTROL ) ) break;
		m_bLiftDownKeyDown	=	TRUE;
		m_uLastKeyPressTime = GetTickCount();
		break;
	case	KEYROTATECCW	:	m_bRotateCCWDown	=	TRUE;	m_uLastKeyPressTime = GetTickCount(); break;
	case	KEYROTATECW		:	m_bRotateCWDown		=	TRUE;	m_uLastKeyPressTime = GetTickCount(); break;
	case	rsUP			:	m_bForwardKeyDown	=	TRUE;	m_uLastKeyPressTime = GetTickCount(); break;
	case	rsDOWN			:	m_bBackwardKeyDown	=	TRUE;	m_uLastKeyPressTime = GetTickCount(); break;
		
	case	rsTAB:
		{
			// ¿ì¾î;;;

			if( m_pCurrentGeometry )
			{
				MoveToSectorCenter( m_pCurrentGeometry );
			}
			else
			{
				ApWorldSector	* pSector = NULL;

				if( g_pcsApmMap )
				{
					pSector = 		g_pcsApmMap->GetSectorByArray( 
											ALEF_LOAD_RANGE_X1		+ ( ALEF_LOAD_RANGE_X2 - ALEF_LOAD_RANGE_X1 ) / 2	,
											0 ,
											( ALEF_LOAD_RANGE_Y2 - ALEF_LOAD_RANGE_Y1 ) / 2 + ALEF_LOAD_RANGE_Y1		);

					if( NULL == pSector )
					{

						
						pSector = AGCMMAP_THIS->LoadSector(
											ArrayIndexToSectorIndexX( ALEF_LOAD_RANGE_X1		+ ( ALEF_LOAD_RANGE_X2 - ALEF_LOAD_RANGE_X1 ) / 2 )	,
											0 ,
											ArrayIndexToSectorIndexZ( ( ALEF_LOAD_RANGE_Y2 - ALEF_LOAD_RANGE_Y1 ) / 2 + ALEF_LOAD_RANGE_Y1		) ) ;
					}

					MoveToSectorCenter	( pSector );
				}
			}
		}
		break;


		/************** Parn´Ô ÀÛ¾÷ ½ÃÀÛ *****************/
	case rsDEL:
		{
			switch( GetCurrentMode() )
			{
			case EDITMODE_OBJECT:
				if( IsObjectSelected() )
				{
					SaveSetChangeObjectList();

					// ¼¿·º¼ÇÀ» ¤±µÎ Áö¿î´Ù.
					AuNode< ObjectSelectStruct >	*	pNode	= m_listObjectSelected.GetHeadNode();
					int						oid		;
					RpClump *				pClump	;

					BOOL					bFoundChildClump = FALSE;

					// ±×·ì¿¡ ¼ÓÇÑ ³à¼®ÀÌ ÀÖ´ÂÁö Á¡°Ë..
					while( pNode )
					{
						pClump	= pNode->GetData().pClump;

						if( g_pcsAgcmObject->IsGroupClump( pClump ) )
						{
							bFoundChildClump	= TRUE;
							break;
						}

						m_listObjectSelected.GetNext( pNode );
					}

					pNode	= m_listObjectSelected.GetHeadNode();

					while( pNode )
					{
						pClump	= pNode->GetData().pClump;

						if( bFoundChildClump )
						{
							// Â÷ÀÏµå°¡ ¾Æ´Ï¸é Áö¿ìÁö ¾Ê´Â´Ù..
							if( g_pcsAgcmObject->IsGroupClump( pClump ) )
							{
								oid		= g_pMainFrame->m_pTileList->m_pObjectWnd->DeleteObject( pClump );

								// ¾ðµÎ µ¥ÀÌÅ¸¿¡¼­ oid °ü·ÃµÈ ³à¼®µéÀ» »èÁ¦ÇÑ´Ù.
								if( oid >= 0 )
									m_UndoManager.ClearObjectUndoData( oid );
							}
							else
							{
								// do nothing..
							}
						}
						else
						{
							oid		= g_pMainFrame->m_pTileList->m_pObjectWnd->DeleteObject( pClump );

							// ¾ðµÎ µ¥ÀÌÅ¸¿¡¼­ oid °ü·ÃµÈ ³à¼®µéÀ» »èÁ¦ÇÑ´Ù.
							if( oid >= 0 )
								m_UndoManager.ClearObjectUndoData( oid );
						}

						m_listObjectSelected.GetNext( pNode );
					}

					SetObjectSelection( NULL );
				}
				break;

			case EDITMODE_OTHERS:
				{
					if( g_pMainFrame->m_pTileList->m_pOthers->Window_KeyDown( ks ) ) return TRUE;
				}
				break;
			}

			if( CMainFrame::MT_VM_DUNGEON == g_pMainFrame->m_bFullScreen )
			{
				// ´øÁ¯ ¸ðµå¶ó¸é..
				g_pMainFrame->m_pDungeonWnd->DeleteSelected();
			}
		}
		break;
		/************** Parn´Ô ÀÛ¾÷ ³¡ *****************/

	// ¸¶°íÀÚ (2004-06-22 ¿ÀÈÄ 8:23:18) : ÇÁ·ÎÆÄÀÏ¸µ~
	#ifdef _PROFILE_
	case rsPGUP:
		SetProfileCommand( PFC_PAGEUP );
		break;
	case rsPGDN:
		SetProfileCommand( PFC_PAGEDN );
		break;
	#endif

	//@{ Jaewon 20040817
	// test post-processing effects.
	case '\'':
		{
			
			if( g_pcsAgcmPostFX->CreateFX( "DownSample-BlurH-BlurV-BloomComp", E_PIPELINE_BLOOMCOMP ) )
			{
				g_pcsAgcmPostFX->PostFX_ON();

				DisplayMessage( "DownSample-BlurH-BlurV-BloomComp ÀÛµ¿~" );
			}
			else
			{
				DisplayMessage( "DownSample-BlurH-BlurV-BloomComp ÀÛµ¿ ½ÇÆÐ" );
			}
		}
		break;
	case 'p':
		{
			if( g_pcsAgcmPostFX->CreateFX( "DownSample-BlurHLC-BlurVLC-BloomComp", E_PIPELINE_BLOOMCOMP ) )
			{
				g_pcsAgcmPostFX->PostFX_ON();
				DisplayMessage( "DownSample-BlurHLC-BlurVLC-BloomComp ÀÛµ¿~" );
			}
			else
			{
				DisplayMessage( "DownSample-BlurHLC-BlurVLC-BloomComp ÀÛµ¿ ½ÇÆÐ" );
			}
		}
		break;
	case ';':
		{
			g_pcsAgcmPostFX->PostFX_OFF();
			DisplayMessage( "Post Fx È¿°ú OFF" );
		}
		break;
	case 'l':
		{
			if( g_pcsAgcmPostFX->CreateFX( "X2", E_PIPELINE_X2 ) )
			{
				g_pcsAgcmPostFX->PostFX_ON();
				DisplayMessage( "Modulate x2 ÀÛµ¿~" );
			}
			else
			{

				DisplayMessage( "Modulate x2 ÀÛµ¿ ½ÇÆÐ" );
			}
/*			if(g_pcsAgcmPostFX->isAutoExposureControlOn())
				g_pcsAgcmPostFX->AutoExposureControlOff();
			else
				g_pcsAgcmPostFX->AutoExposureControlOn();
*/		}
		break;
	//@} Jaewon

	default:
		break;
	}
//	MTRACE( "Currenttick %04d\n" , m_uLastKeyPressTime % 10000 );
	
	return TRUE;
}

BOOL MainWindow::OnKeyUp		( RsKeyStatus *ks	)
{
    switch( ks->keyCharCode )
    {
	case 'c':
		{
			// Ä«ÇÇ ÇÃ·¡±× Á¦°Å~
			m_bCopyObject = FALSE;
		}
		break;
    case rsLCTRL:
		{
			m_bLCtrl = FALSE;
		}
		break;
	
    case rsRCTRL:
		{
			m_bRCtrl = FALSE;
		}
		break;

	case	KEYFORWARD		:	m_bUp				=	FALSE;	break;
	case	KEYBACKWARD		:	m_bDown				=	FALSE;	break;
	case	rsLEFT			:
	case	KEYSTEPLEFT		:	m_bStepLeftKeyDown	=	FALSE;	break;
    case	rsRIGHT			:
	case	KEYSTEPRIGHT	:	m_bStepRightKeyDown	=	FALSE;	break;
	case	KEYLIFTUP		:	m_bLiftUpKeyDown	=	FALSE;	break;
	case	KEYLIFTDOWN		:	m_bLiftDownKeyDown	=	FALSE;	break;
	case	KEYROTATECCW	:	m_bRotateCCWDown	=	FALSE;	break;
	case	KEYROTATECW		:	m_bRotateCWDown		=	FALSE;	break;
	case	rsUP			:	m_bForwardKeyDown	=	FALSE;	break;
	case	rsDOWN			:	m_bBackwardKeyDown	=	FALSE;	break;

	default:
		break;
	}

	if( g_pMainFrame->m_Document.IsInFirstPersonViewMode() )
	{
		switch( ks->keyCharCode )
		{
		case	KEYFORWARD		:
		case	KEYBACKWARD		:
		case	KEYSTEPRIGHT	:
		case	KEYSTEPLEFT		:
			g_pcsAgcmCharacter->StopSelfCharacter();
			break;
		default:
			break;
		}
	}

	return TRUE;
}

BOOL MainWindow::OnMouseMove	( RsMouseStatus *ms	)
{
	PROFILE("MainWindow::OnMouseMove");
	switch( GetCurrentMode() )
	{
	case EDITMODE_TILE			:
		{
			if( m_bLButtonDown )
			{
				if( m_pCurrentGeometry )
				{
					ApDetailSegment	*	pSelectedSegment	;
					int				posx, posz		;
		//			stFindMapInfo	FindMapInfo		;

					pSelectedSegment = GetCurrentPolygonsTileIndex(
						m_pCurrentGeometry		,
						m_nFoundPolygonIndex	,
						( INT32 ) ms->pos.x		,
						( INT32 ) ms->pos.y		,
						&posx					,
						&posz					);


					if( pSelectedSegment == NULL )
					{
						// do no op
					}
					else
					{
						// ºí·° ÁöÁ¤..;
						if( m_bGeometrySelection )
						{
							m_SelectedPosX2			= posx				;
							m_SelectedPosY2			= posz				;

							m_pSelectedGeometry2	= m_pCurrentGeometry;
						}
					}
				}
			}
		}
		break;

	case EDITMODE_OBJECT		:
		{
			// ¸¶°íÀÚ (2005-09-02 ¿ÀÀü 11:21:41) : 
			// ¿ÁÆ®¸® Á¤º¸ Ãâ·Â..

			if( g_Const.m_bShowOctreeInfo )
			{
				if( ISBUTTONDOWN( VK_CONTROL ) )
				{
					// ¿ÁÆ®¸® Á¤º¸ Ç¥½Ã..

					RpClump	* pSelectedClump;
					pSelectedClump = AGCMMAP_THIS->GetCursorClump( g_MyEngine.m_pCamera , & ms->pos , g_bUsePickingOnly );

					if( NULL == pSelectedClump )
					{
						// octree°ü·Ã Á¤º¸¸¦ Ãâ·Â Áß´Ü
						g_pcsAgcmOcTree->SetDrawEnd();
					}
					else
					{
						// octree°ü·Ã Á¤º¸¸¦ Ãâ·Â ½ÃÀÛ
						g_pcsAgcmOcTree->SetDrawEnd();
						g_pcsAgcmOcTree->SetClumpAndDrawStart(pSelectedClump);
					}

					g_pcsAgcmOcTree->SetOctreeDebugSector( m_pCurrentGeometry );
				}
				else
				{
					g_pcsAgcmOcTree->SetDrawEnd();
				}
			}

			// ¿ÀºêÁ§Æ® ¼±ÅÃ º¯°æ..
			/********* Parn ´Ô ÀÛ¾÷ ½ÃÀÛ ********/
			if( m_bLButtonDown && IsObjectSelected() )
			{
				SaveSetChangeObjectList();

				if( m_bCopyObject )
				{
					m_bCopyObject = FALSE;

					// µàÇÃ¸®ÄÉÀÌÆ®.

					// ±×³É Translate..
					RpClump * pClump	;

					ApdObject		*	pstApdObject	;
					AgcdObject		*	pstAgcdObject	;
					ApdObject		*	pstNewApdObject	;
					AgcdObject		*	pstNewAgcdObject;

					int oid = 0;

					// Object ID¸¦ ¾ò¾î³¿.
					INT32	nType;
					

					AuNode< ObjectSelectStruct >	* pNode = m_listObjectSelected.GetHeadNode();
					while( pNode )
					{
						pClump	= pNode->GetData().pClump;
						// pPos	= RwMatrixGetPos( RwFrameGetLTM( RpClumpGetFrame( pClump ) ) );

						{
							// Á¤º¸ ¾ò±â.
							nType = AcuObject::GetClumpType( pClump, &oid );

							switch( AcuObject::GetType( nType ) )
							{
							case	ACUOBJECT_TYPE_OBJECT			:
								break;
							default:
								oid = 0;
								break;
							}

							// ÇØ´ç ¿ÀºêÁ§Æ®ÀÇ Æ÷ÀÎÅÍ¸¦ ¾ò¾î³¿.
							pstApdObject	= g_pcsApmObject->	GetObject		( oid			);
							if( NULL == pstApdObject ) continue;
							pstAgcdObject	= g_pcsAgcmObject->GetObjectData	( pstApdObject	);
							if( NULL == pstAgcdObject ) continue;

													// º¹»ç..
							pstNewApdObject = g_pcsApmObject->AddObject( 
								g_pcsApmObject->GetEmptyIndex( GetDivisionIndexF( pstApdObject->m_stPosition.x , pstApdObject->m_stPosition.z ) ) , pstApdObject->m_pcsTemplate->m_lID );
							ASSERT( NULL != pstNewApdObject );
							if( !pstNewApdObject )
							{
								MD_SetErrorMessage( "g_pcsApmObject->AddObject ¿ÀºêÁ§Æ® Ãß°¡½ÇÆÐ.." );
								continue;
							}

							// ¿ÀºêÁ§Æ® ºí·¯Å· Ãß°¡..
							AuBLOCKING			stBlockInfoCalcurated[APDOBJECT_MAX_BLOCK_INFO];		// Blocking Info
							pstNewApdObject->CalcWorldBlockInfo( pstApdObject->m_pcsTemplate->m_nBlockInfo , pstApdObject->m_pcsTemplate->m_astBlockInfo , stBlockInfoCalcurated );
							g_pcsApmMap->AddObjectBlocking( stBlockInfoCalcurated , pstNewApdObject->m_nBlockInfo );

							pstNewAgcdObject = g_pcsAgcmObject->GetObjectData(pstNewApdObject);

							ASSERT( pstNewAgcdObject != NULL );
							if( !pstNewAgcdObject )
							{
								MD_SetErrorMessage( "g_pcsAgcmObject->GetObjectData ¿ÀºêÁ§Æ® Å¬¶óÀÌ¾ðÆ® Á¤º¸ ¾ò±â ½ÇÆÐ" );
								continue;
							}

							// ÇÃ·¡±× °áÇÕ..
							pstNewAgcdObject->m_lObjectType = ACUOBJECT_TYPE_OBJECT;

							//pstApdObject->m_stBlockInfo.type = pstApdObjectTemplate->m_stBlockInfo.type;

							pstNewApdObject->m_stScale		= pstApdObject->m_stScale	;
							pstNewApdObject->m_stPosition	= pstApdObject->m_stPosition;
							pstNewApdObject->m_fDegreeX		= pstApdObject->m_fDegreeX	;
							pstNewApdObject->m_fDegreeY		= pstApdObject->m_fDegreeY	;

							if( g_pcsApmObject->UpdateInit( pstNewApdObject ) )
							{

							}
							else
							{
								MD_SetErrorMessage( "g_pcsApmObject->UpdateInit Å¬·³ÇÁ ¼³Á¤ ½ÇÆÐ" );
								continue;
							}
						}

						m_listObjectSelected.GetNext( pNode );
					}

				}

				ASSERT( NULL != g_pMainFrame							);
				ASSERT( NULL != g_pMainFrame->m_pTileList				);
				ASSERT( NULL != g_pMainFrame->m_pTileList->m_pObjectWnd	);

				FLOAT	fDeltaX	= ms->pos.x - m_PrevMouseStatus.pos.x;	
				FLOAT	fDeltaY	= ms->pos.y - m_PrevMouseStatus.pos.y;

				RwV3d	center;

				GetObjectCenter( & center );
				
				if( ms->control && ms->shift )
				// Ctrl + Shift ´Â YÃà RotateÀÌ´Ù.
				{
					// do nothing yet.
					//g_pMainFrame->m_pTileList->m_pObjectWnd->TransformObject(
					//	GetSelectedObject() , ms , fDeltaX , fDeltaY , &m_Position, 0 );

					SelectedObjectTurning( fDeltaX );
				}
				else if( ms->control )
				// Ctrl Àº ScaleÀÌ´Ù.
				{
					// do nothing yet.
					//g_pMainFrame->m_pTileList->m_pObjectWnd->TransformObject(
					//	GetSelectedObject() , ms , fDeltaX , fDeltaY , &m_Position, 0 );
					float	dx	= ( fDeltaX ) * 0.005f;

					AuNode< ObjectSelectStruct >	*	pNode	;
					ApdObject		*	pstApdObject	;
					int					oid		;
					RpClump *			pClump	;
					INT32				nType	;

					BOOL					bFoundChildClump = FALSE;

					RwMatrix	* pMatrix = RwMatrixCreate();
					{
						RwV3d		vToCetner;
						vToCetner.x		= -center.x;
						vToCetner.z		= -center.z;
						vToCetner.y		= -center.y;

						RwV3d		vToOrigin;
						vToOrigin.x		= center.x;
						vToOrigin.z		= center.z;
						vToOrigin.y		= center.y;

						RwV3d		vAxis;
						vAxis.x		= 0.0f;
						vAxis.z		= 0.0f;
						vAxis.y		= 1.0f;

						RwV3d		vScale;
						vScale.x	= 1.0f + dx;
						vScale.z	= 1.0f + dx;
						vScale.y	= 1.0f + dx;

						RwMatrixTranslate	( pMatrix , & vToCetner ,rwCOMBINEREPLACE );
						RwMatrixScale		( pMatrix , & vScale , rwCOMBINEPOSTCONCAT );
						RwMatrixTranslate	( pMatrix , & vToOrigin , rwCOMBINEPOSTCONCAT );
					}

					pNode	= m_listObjectSelected.GetHeadNode();

					while( pNode )
					{
						pClump	= pNode->GetData().pClump;

						{
							oid = 0;
							// Object ID¸¦ ¾ò¾î³¿.
							nType = AcuObject::GetClumpType( pClump, &oid );

							switch( AcuObject::GetType( nType ) )
							{
							case	ACUOBJECT_TYPE_OBJECT			:
								break;
							default:
								oid = 0;
								break;
							}
							
							// ÇØ´ç ¿ÀºêÁ§Æ®ÀÇ Æ÷ÀÎÅÍ¸¦ ¾ò¾î³¿.
							pstApdObject	= g_pcsApmObject->	GetObject		( oid			);
							if( pstApdObject ) 
							{
								// °¢µµ Á¶Àý
								pstApdObject->m_stScale.x += dx;
								pstApdObject->m_stScale.y += dx;
								pstApdObject->m_stScale.z += dx;

								// À§Ä¡ Á¶Àý..
								{
									RwV3d	pos;
									pos.x	= pstApdObject->m_stPosition.x;
									pos.y	= pstApdObject->m_stPosition.y;
									pos.z	= pstApdObject->m_stPosition.z;

									RwV3dTransformPoint( & pos , & pos , pMatrix );

									pstApdObject->m_stPosition.x	= pos.x;
									pstApdObject->m_stPosition.z	= pos.z;
									pstApdObject->m_stPosition.y	= pos.y;

									/*
									if( g_pMainFrame->m_pTileList->m_pObjectWnd->m_bSnapToHeight )
									{
										pstApdObject->m_stPosition.y	= AGCMMAP_THIS->GetHeight_Lowlevel_HeightOnly( pos.x , pos.z );
									}
									*/

									g_pcsAgcmObject->SetupObjectClump_TransformOnly( pstApdObject );
								}

								SaveSetChangeObjectList();
							}
						}

						m_listObjectSelected.GetNext( pNode );
					}

					RwMatrixDestroy( pMatrix );
				}
				else if( ms->shift )
				{
					// yÃàÀ¸·Î¸¸ ¿òÁ÷ÀÓ..
					//g_pMainFrame->m_pTileList->m_pObjectWnd->TransformObject(
					//	GetSelectedObject() , ms , fDeltaX , fDeltaY , &m_Position, 0 );

					// ±×³É Translate..
					RpClump * pClump	;

					ApdObject		*	pstApdObject	= NULL;
					int					oid		;
					INT32				nType	;

					// ÃÖÃÊÀÇ ¿ÀºêÁ§Æ®¸¦ ±¸ÇØ³¿..
					{
						nType = AcuObject::GetClumpType( GetSelectedObject() , &oid );
						switch( AcuObject::GetType( nType ) )
						{
						case	ACUOBJECT_TYPE_OBJECT			:
							break;
						default:
							oid = 0;
							break;
						}
						
						// ÇØ´ç ¿ÀºêÁ§Æ®ÀÇ Æ÷ÀÎÅÍ¸¦ ¾ò¾î³¿.
						pstApdObject	= g_pcsApmObject->	GetObject		( oid			);
					}

					FLOAT	fHeightDistance = 0.0f;

					// º¯È­ÇÒ ³ôÀÌ Â÷¸¦ ±¸ÇÔ.
					if( pstApdObject )
					{
						//RwV3d			*	camPos			= RwMatrixGetPos( RwFrameGetLTM( RwCameraGetFrame( g_pEngine->m_pCamera ) ) );
						RwV3d			*	camLookAt		= RwMatrixGetAt	( RwFrameGetLTM( RwCameraGetFrame( g_pEngine->m_pCamera ) ) );

						RwV3d normalvector	;
						RwV3d CollisionPoint;

						// ¸éÀÇ ³ë¸Öº¤ÅÍ..
						normalvector.x	= - camLookAt->x;
						normalvector.y	= 0.0f			;
						normalvector.z	= - camLookAt->z;

						RwLine	CameraPixelRay		;

						RwCameraCalcPixelRay( g_pEngine->m_pCamera , &CameraPixelRay	, &ms->pos );
						
						_GetIntersectPos( 
							&pstApdObject->m_stPosition	,
							&normalvector				,
							&CameraPixelRay.start		,
							&CameraPixelRay.end			,
							&CollisionPoint				);

						fHeightDistance = CollisionPoint.y - pstApdObject->m_stPosition.y;
					}

					AuNode< ObjectSelectStruct >	* pNode = m_listObjectSelected.GetHeadNode();

					// ±×·ì¿¡ ³ôÀÌ Àû¿ë.
					while( pNode )
					{
						pClump	= pNode->GetData().pClump;

						{
							oid = 0;
							// Object ID¸¦ ¾ò¾î³¿.
							nType = AcuObject::GetClumpType( pClump, &oid );

							switch( AcuObject::GetType( nType ) )
							{
							case	ACUOBJECT_TYPE_OBJECT			:
								break;
							default:
								oid = 0;
								break;
							}
							
							// ÇØ´ç ¿ÀºêÁ§Æ®ÀÇ Æ÷ÀÎÅÍ¸¦ ¾ò¾î³¿.
							pstApdObject	= g_pcsApmObject->	GetObject		( oid			);
							if( pstApdObject ) 
							{
								pstApdObject->m_stPosition.y += fHeightDistance;
								g_pcsAgcmObject->SetupObjectClump_TransformOnly( pstApdObject );
							}
						}

						m_listObjectSelected.GetNext( pNode );
					}
				}
				else
				{
					// ±×³É Translate..
					RpClump * pClump	;
					RwV3d	* pPos		;
					RwV3d	vOffset		;
					RwV3d	vNewPos		;

					AuNode< ObjectSelectStruct >	* pNode = m_listObjectSelected.GetHeadNode();

					while( pNode )
					{
						pClump	= pNode->GetData().pClump;
						pPos	= RwMatrixGetPos( RwFrameGetLTM( RpClumpGetFrame( pClump ) ) );

						// Áß½É°úÀÇ °Å¸®..
						vOffset.x	= pPos->x - center.x;
						vOffset.z	= pPos->z - center.z;


						vNewPos.x	= m_Position.x	+ vOffset.x;
						vNewPos.z	= m_Position.z	+ vOffset.z;
						vNewPos.y	= 1.0f;

						g_pMainFrame->m_pTileList->m_pObjectWnd->TransformObject(
							pClump , ms , fDeltaX , fDeltaY , &vNewPos, 0 );

						m_listObjectSelected.GetNext( pNode );
					}
				}
			}
			else
			{
				switch( m_bObjectBrushing )
				{
				case OBJECT_SELECT			:
				case OBJECT_SELECTADD		:
				case OBJECT_SELECTREMOVE	:
					{
						POINT	p2;
						p2.x	= ( INT32 ) ms->pos.x;
						p2.y	= ( INT32 ) ms->pos.y;

						SetObjectSelection( m_pointObjectSelectFirst , p2 , ms->control ? FALSE : TRUE );
					}
					break;
				default:
					break;
				}
			}
		}
		break;
	case EDITMODE_OTHERS:
		{
			if( g_pMainFrame->m_pTileList->m_pOthers->Window_MouseMove( ms ) ) return TRUE;

			if( m_pCurrentGeometry )
				g_pMainFrame->m_pTileList->m_pOthers->MouseMoveGeometry( &m_Position	);
			else
				g_pMainFrame->m_pTileList->m_pOthers->MouseMoveGeometry( NULL			);

		}
		break;
	default:
		break;
	}

	if( m_bRButtonDown )
	{
		if( g_pMainFrame->m_Document.IsInFirstPersonViewMode() )
		{
			if ( ms->pos.x != m_PrevMouseStatus.pos.x )
			{
				g_pcsAgcmCamera2->bRotWY(  -(ms->pos.x - m_PrevMouseStatus.pos.x) );
			}

			if ( ms->pos.y != m_PrevMouseStatus.pos.y )
			{
				g_pcsAgcmCamera2->bRotCX(  (ms->pos.y - m_PrevMouseStatus.pos.y) );
			}
		}
		else
		{
			// Ä«¸Þ¶ó ¿öÅ·
			float	dx	= ( ms->pos.x - m_PrevMouseStatus.pos.x );
			float	dy	= -( ms->pos.y - m_PrevMouseStatus.pos.y );

			RwMatrix	*m;
			RwFrame		*f;
			RwV3d xaxis	= { 1.0f , 0.0f , 0.0f };
			RwV3d yaxis	= { 0.0f , 1.0f , 0.0f };

			// 1ÀÎÄª ºä ÀÌ¸é..
			if( g_pMainFrame->m_Document.IsInFirstPersonViewMode() )
			{
				AgpdCharacter * pCharacter = g_pcsAgcmCharacter->GetSelfCharacter();
				//ASSERT( NULL != pCharacter );
				//g_pcsAgpmCharacter.TurnCharacter( pCharacter , 0 , dy * 0.1f );
				g_pcsAgcmCharacter->TurnSelfCharacter( pCharacter->m_fTurnX , pCharacter->m_fTurnY + (dx * 0.1f) );
			}
			else
			{
				f = RwCameraGetFrame( g_pEngine->m_pCamera);
				RwFrameRotate( f , &yaxis , dx * 0.1f , rwCOMBINEPRECONCAT );
				RwFrameRotate( f , &xaxis , dy * 0.1f , rwCOMBINEPRECONCAT );

				m = RwFrameGetMatrix( f );			RwV3dCrossProduct	( &m->right , &yaxis , &m->at	);

				m->right.y = 0.0f;

				RwV3dNormalize		( &m->right , &m->right			);
				RwV3dCrossProduct	( &m->up , &m->at , &m->right	);
				RwV3dNormalize		( &m->up , &m->up				);
				RwV3dCrossProduct	( &m->right , &m->up , &m->at	);

				RwFrameUpdateObjects(f);
			}
		}
	}

	/*
	if( g_pMainFrame->m_Document.IsInFirstPersonViewMode() )
	{
		FLOAT	fU, fV;
		MousePositionToUVCoordinate( &(ms->pos), &fU, &fV );
		g_pcsAgcmCamera->ControlCameraScreenToClient( fU, ms->delta.x );
	}
	*/

	m_PrevMouseStatus		= *ms				;
	return TRUE;
}

BOOL MainWindow::OnLButtonUp	( RsMouseStatus *ms	)
{
	//m_PrevMouseStatus	= *ms	;

	switch( GetCurrentMode() )
	{
	case EDITMODE_GEOMETRY		:
		{
			m_bGeometryAdjust = FALSE;
			if( m_pCurrentGeometry )
			{
				rsDWSectorInfo	dwSectorInfo	;

				AGCMMAP_THIS->GetDWSector( m_pCurrentGeometry , & dwSectorInfo		);
				if( dwSectorInfo.pDWSector )
					RpCollisionGeometryBuildData	( dwSectorInfo.pDWSector->geometry , NULL	);
//				for( int i = 0 ; i < 4 ; i ++ )
//					if( m_pCurrentGeometry->GetNearSector( i ] )
//						RpCollisionWorldSectorBuildData( m_pCurrentGeometry->GetNearSector( i ]->m_pWorldSector , NULL );
			}

			m_UndoManager.EndActionBlock();
		}
		break;
	case EDITMODE_TILE:
		{
			if( m_bTileSelecting )
			{
				if( ISBUTTONDOWN( VK_CONTROL ) )
				{
					// do no op..
				}
				else
				{
					m_bTileSelecting = FALSE;
				}
			}
			else
			if( m_bGeometrySelection )
			{
				ASSERT( NULL != m_pSelectedGeometry1 );
				ASSERT( NULL != m_pSelectedGeometry2 );

				// ¿¡·¯Ã³¸®..
				if(	NULL == m_pSelectedGeometry1 ||
					NULL == m_pSelectedGeometry2 )
					break;

				SaveSetChangeMoonee();

				// ¿µ¿ªÁöÁ¤ÇØ¼­ Å¸ÀÏ »Ñ¸®´Â ±â´É.

				// ¼½ÅÍ ÀÎµ¦½º ¹üÀ§¸¦ ±¸ÇÑ´Ù
				int	nSector_LeftTop_X		;
				int	nSector_LeftTop_Y		;

				int	nSector_Rightbottom_X	;
				int	nSector_Rightbottom_Y	;

				int	nPosX1 , nPosY1			;
				int	nPosX2 , nPosY2			;

				// LeftTop À» ±¸ÇØ³¿.
				if( m_pSelectedGeometry1->GetArrayIndexX() < m_pSelectedGeometry2->GetArrayIndexX() )
				{
					nSector_LeftTop_X		= m_pSelectedGeometry1->GetArrayIndexX()	;
					nSector_Rightbottom_X	= m_pSelectedGeometry2->GetArrayIndexX()	;

					nPosX1					= m_SelectedPosX1							;
					nPosX2					= m_SelectedPosX2							;
				}
				else
				{
					nSector_LeftTop_X		= m_pSelectedGeometry2->GetArrayIndexX()	;
					nSector_Rightbottom_X	= m_pSelectedGeometry1->GetArrayIndexX()	;

					nPosX1					= m_SelectedPosX2							;
					nPosX2					= m_SelectedPosX1							;
				}

				if( m_pSelectedGeometry1->GetArrayIndexZ() < m_pSelectedGeometry2->GetArrayIndexZ() )
				{
					nSector_LeftTop_Y		= m_pSelectedGeometry1->GetArrayIndexZ()	;
					nSector_Rightbottom_Y	= m_pSelectedGeometry2->GetArrayIndexZ()	;

					nPosY1					= m_SelectedPosY1							;
					nPosY2					= m_SelectedPosY2							;
				}
				else
				{
					nSector_LeftTop_Y		= m_pSelectedGeometry2->GetArrayIndexZ()	;
					nSector_Rightbottom_Y	= m_pSelectedGeometry1->GetArrayIndexZ()	;

					// ½º¿Ò~...;
					nPosY1					= m_SelectedPosY2							;
					nPosY2					= m_SelectedPosY1							;
				}
				
				// ºí·° ÁöÁ¤..;
				m_bGeometrySelection	= FALSE	;

				// Å¸ÀÏº¯°æ..
				
				stFindMapInfo	FindMapInfo		;
				rsDWSectorInfo	dwSectorInfo	;
				ApWorldSector *	pSector			;

				int x , z						;
				int randomtileindex				;

				int	nSectorX , nSectorY			;

				int	nX1 , nX2 , nY1 , nY2		;

//				DisplayMessage( AEM_NOTIFY , "Fill ±â´ÉÀº Undo¸¦ Áö¿øÇÏÁö ¾Ê½À´Ï´Ù. Âü°íÇÏ¼¼¿ä..;;" );
				m_UndoManager.StartActionBlock( CUndoManager::TILE );

				// ¿ùµå ¼½ÅÍ Æ®·¡¹ú½º.
				for( nSectorY = nSector_LeftTop_Y ; nSectorY <= nSector_Rightbottom_Y ; nSectorY ++ )
				{
					for( nSectorX = nSector_LeftTop_X ; nSectorX <= nSector_Rightbottom_X ; nSectorX ++ )
					{
						// ¿ì¼± ¼½ÅÍ Æ÷ÀÎÅÍ¸¦ ¾ò¾î³¿..
						pSector	= g_pcsApmMap->GetSectorByArray( nSectorX , 0 , nSectorY );

						// ¿©±â¼­.. Àû¿ëÇÒ ¹üÀ§¸¦ ±¸ÇÑ´Ù..
						if( NULL == pSector ) continue;
						if( nSectorX == nSector_LeftTop_X		&&
							nSectorX == nSector_Rightbottom_X	)
						{
							if( nPosX1 < nPosX2 )	{ nX1 = nPosX1 ; nX2 = nPosX2; }
							else					{ nX1 = nPosX2 ; nX2 = nPosX1; }
						}
						else
						{
							if( nSectorX == nSector_LeftTop_X		)	nX1		= nPosX1				;
							else										nX1		= 0						;
							if( nSectorX == nSector_Rightbottom_X	)	nX2		= nPosX2				;
							else										nX2		= pSector->D_GetDepth() -1;
						}

						// ¿©±â¼­.. Àû¿ëÇÒ ¹üÀ§¸¦ ±¸ÇÑ´Ù..
						if( nSectorY == nSector_LeftTop_Y		&&
							nSectorY == nSector_Rightbottom_Y	)
						{
							if( nPosY1 < nPosY2 )	{ nY1 = nPosY1 ; nY2 = nPosY2; }
							else					{ nY1 = nPosY2 ; nY2 = nPosY1; }
						}
						else
						{
							if( nSectorY == nSector_LeftTop_Y		)	nY1		= nPosY1				;
							else										nY1		= 0						;

							if( nSectorY == nSector_Rightbottom_Y	)	nY2		= nPosY2				;
							else										nY2		= pSector->D_GetDepth() -1;
						}

						// ¼½ÅÍ Á¤º¸¸¦ ¾ò¾î³»°í..
						AGCMMAP_THIS->GetDWSector( pSector ,  & dwSectorInfo );

						// Áö¿À¸ÞÆ®¸®¸¦ ¶ôÇÔ..

						for( z = nY1 ; z <= nY2 ; z ++ )
						{
							for( x = nX1 ; x <= nX2 ; x ++ )
							{
								if( GetWorldSectorInfo( &FindMapInfo , pSector , x , z ) )
								{
									if( FindMapInfo.pSector != pSector			) continue;

									randomtileindex = g_pMainFrame->m_Document.GetSelectedTileIndex();

									// Åë¸Ê Ã³¸®..
									randomtileindex = __GetTongMapIndex( randomtileindex , FindMapInfo.pSector , FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ );
									
									if(	FindMapInfo.pSegment->pIndex[ TD_FIRST	]	== randomtileindex			&&
										FindMapInfo.pSegment->pIndex[ TD_SECOND	]	== ALEF_TEXTURE_NO_TEXTURE	) continue;

									m_UndoManager.AddTileActionUnit( FindMapInfo.pSector , FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ ,
										randomtileindex );
									AGCMMAP_THIS->D_SetTile( FindMapInfo.pSector , FindMapInfo.pSector->GetCurrentDetail() ,
										FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ , randomtileindex );

								}
							}
						}
						// ¼½ÅÍ·çÇÁ ³¡..
						UnlockSectors();
					}
				}
	
				m_UndoManager.EndActionBlock();

				m_SelectedPosX1		= -1		;
				m_SelectedPosY1		= -1		;
				m_SelectedPosX2		= -1		;
				m_SelectedPosY2		= -1		;

				m_pSelectedGeometry1	= NULL	;
				m_pSelectedGeometry2	= NULL	;
			}
			else
			{
				m_bTileAdjust	= FALSE;
				m_UndoManager.EndActionBlock();
			}
		}
		break;
//	case EDITMODE_SUBDIVISION	:
//		{
//			if( m_pCurrentGeometry && m_pCurrentGeometry->IsLoadedDetailData() == FALSE ) break;
//			// ÀÚ ±×¸®ÀÚ..
//			int	lefttop_x;
//			int	lefttop_y;
//
//			int	rightbottom_x;
//			int	rightbottom_y;
//
//			// LeftTop À» ±¸ÇØ³¿.
//			if( m_SelectedPosX1 < m_SelectedPosX2 )
//			{
//				lefttop_x		= m_SelectedPosX1;
//				rightbottom_x	= m_SelectedPosX2;
//			}
//			else
//			{
//				lefttop_x		= m_SelectedPosX2;
//				rightbottom_x	= m_SelectedPosX1;
//			}
//
//			if( m_SelectedPosY1 < m_SelectedPosY2 )
//			{
//				lefttop_y		= m_SelectedPosY1;
//				rightbottom_y	= m_SelectedPosY2;
//			}
//			else
//			{
//				lefttop_y		= m_SelectedPosY2;
//				rightbottom_y	= m_SelectedPosY1;
//			}
//			
//			m_SelectedPosX1	=	lefttop_x		;
//			m_SelectedPosY1	=	lefttop_y		;
//			m_SelectedPosX2	=	rightbottom_x	;
//			m_SelectedPosY2	=	rightbottom_y	;
//
//			// ºí·° ÁöÁ¤..;
//			m_bGeometrySelection	= FALSE		;
//		}
		break;

	case EDITMODE_OTHERS:
		{
			if( g_pMainFrame->m_pTileList->m_pOthers->Window_LButtonUp( ms ) ) return TRUE;
			g_pMainFrame->m_pTileList->m_pOthers->LButtonUpGeometry( &m_Position );
		}
		break;
	case EDITMODE_OBJECT:
		{
			if( m_bLButtonDown && IsObjectSelected() )
			{
				// ¿ÀºêÁ§Æ® Á¤º¸ Ã£¾Æ³¿..
				ApdObject *		pstApdObject	;
				AgcdObject *	pstAgcdObject	;

				ObjectUndoInfo	changedInfo		;
				changedInfo.oid = 0;

				AuNode< ObjectSelectStruct >	* pNode = m_listObjectSelected.GetHeadNode();
				ObjectSelectStruct				* pObjectSelectStruct;

				m_UndoManager.StartActionBlock		( CUndoManager::OBJECTMANAGE			);
				while( pNode )
				{
					pObjectSelectStruct	= &pNode->GetData();

					pstApdObject	= g_pcsApmObject->	GetObject		( pObjectSelectStruct->undoInfo.oid	);
					ASSERT( NULL != pstApdObject	);
					pstAgcdObject	= g_pcsAgcmObject->GetObjectData	( pstApdObject				);
					ASSERT( NULL != pstAgcdObject	);

					changedInfo.oid			= pObjectSelectStruct->undoInfo.oid;
					changedInfo.stScale		= pstApdObject->m_stScale		;	// Scale Vector
					changedInfo.stPosition	= pstApdObject->m_stPosition	;	// Base Position
					changedInfo.fDegreeX	= pstApdObject->m_fDegreeX		;	// Rotation Degree
					changedInfo.fDegreeY	= pstApdObject->m_fDegreeY		;	// Rotation Degree

					m_UndoManager.AddObjectActionUnit	( &pObjectSelectStruct->undoInfo , &changedInfo	);

					m_listObjectSelected.GetNext( pNode );
				}

				m_UndoManager.EndActionBlock		(										);
			}
			else
			{
				switch( m_bObjectBrushing )
				{
				case	OBJECT_SELECT		:
					{
						m_listObjectSelected.RemoveAll();
						m_listObjectSelected = m_listObjectSelecting;
						m_listObjectSelecting.RemoveAll();
					}
					break;
				case	OBJECT_SELECTADD	:
					{
						// ¸¶°íÀÚ (2005-04-14 ¿ÀÈÄ 2:23:55) : 
						// ¼±ÅÃÁßÀÎ ³à¼®À» ±âÁ¸ ¸®½ºÆ®¿¡ Ãß°¡ÇÑ´Ù.
						AuNode< ObjectSelectStruct >	* pNode			= m_listObjectSelecting.GetHeadNode();
						ObjectSelectStruct				* pObjectselect	;
						while( pNode )
						{
							pObjectselect = &pNode->GetData();
							AddObjectSelection( pObjectselect->pClump );
							pNode = pNode->GetNextNode();
						}

						m_listObjectSelecting.RemoveAll();

					}
					break;
				case	OBJECT_SELECTREMOVE	:
					{
						// ¸¶°íÀÚ (2005-04-14 ¿ÀÈÄ 2:23:55) : 
						// ¼±ÅÃÁßÀÎ ³à¼®À» ±âÁ¸ ¸®½ºÆ®¿¡ Á¦°Å
						AuNode< ObjectSelectStruct >	* pNode			= m_listObjectSelecting.GetHeadNode();
						ObjectSelectStruct				* pObjectselect	;
						while( pNode )
						{
							pObjectselect = &pNode->GetData();
							RemoveObjectSelection( pObjectselect->pClump );
							pNode = pNode->GetNextNode();
						}

						m_listObjectSelecting.RemoveAll();

					}
					break;

				default:
					break;
				}

				m_bObjectBrushing = OBJECT_NONE;
			}
		}
		break;

	default:
		break;
	}

	m_bLButtonDown		= FALSE	;

	return TRUE;
}

void MainWindow::HighlightRender(void)
{
//    RwMatrix       *ltm = RwFrameGetLTM(RpClumpGetFrame(m_pSelectedClump));
//    RwIm3DVertex    vertices[8];
//    RwInt32         i;
//    RwImVertexIndex indices[24] = { 0, 1, 1, 3, 3, 2, 2, 0,
//                                    4, 5, 5, 7, 7, 6, 6, 4,
//                                    0, 4, 1, 5, 2, 6, 3, 7 };
//
//    for (i = 0; i < 8; i++)
//    {
//        RwIm3DVertexSetPos(vertices+i,
//            i&1 ? PickBox.sup.x : PickBox.inf.x,
//            i&2 ? PickBox.sup.y : PickBox.inf.y,
//            i&4 ? PickBox.sup.z : PickBox.inf.z);
//        RwIm3DVertexSetRGBA(vertices+i, 255, 0, 0, 255); 
//    }
//
//    if (RwIm3DTransform(vertices, 8, ltm, 0))
//    {
//        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, indices, 24);
//        RwIm3DEnd();
//    }
}

BOOL MainWindow::OnIdle			(UINT32 ulClockCount)
{
	PROFILE("MainWindow::OnIdle");
	//UINT	currenttick	= g_pEngine->GetTickCount	();
	//UINT	pasttick	= g_pEngine->GetPastTime	();

	static int nSkipCount = 0;
	if( nSkipCount ++ % 10 )
	{
		CameraIdleMovement		();
		GeometryIdleAdjustment	();
		TileIdleAdjustment		();
		ObjectIdleBrushing		();

		DetailIdleCheck			();
	}

	SectorCollisionCheck( & m_PrevMouseStatus );


	//¸¶¿ì½º Æ÷ÀÎÅÍ°¡ È­¸é ¾ç³¡¿¡ Á¸ÀçÇÒ¶§ Ä«¸Þ¶ó µ¹¸®±â 
	if( g_pMainFrame->m_Document.IsInFirstPersonViewMode() )
	{
		UINT32	ulDeltaTime = ulClockCount - m_ulPrevTime;
		m_ulPrevTime = ulClockCount;

		if (m_bLButtonDown && g_pMainFrame->m_pToolBar->GetFPSEditMode() == FALSE )
		{
			AuPOS	posDestPoint;

			if ( m_bLButtonDown )
				if ( AGCMMAP_THIS->GetMapPositionFromMousePosition_tool( (int)m_PrevMouseStatus.pos.x, (int)m_PrevMouseStatus.pos.y,  &posDestPoint.x, &posDestPoint.y, &posDestPoint.z ) )
					//g_pcsAgpmCharacter->MoveCharacter( g_pcsAgcmCharacter->m_pcsSelfCharacter, &posDestPoint, 0, FALSE, g_pcsAgcmCharacter->m_pcsSelfCharacter->m_bMoveFast );
					g_pcsAgpmCharacter->MoveCharacter( g_pcsAgcmCharacter->m_pcsSelfCharacter, &posDestPoint, MD_NODIRECTION , FALSE, TRUE );
					//g_pcsAgcmCharacter->MoveSelfCharacter( &posDestPoint, 0, FALSE, g_pcsAgcmCharacter->m_pcsSelfCharacter->m_bMoveFast );
		}	

		/*
		FLOAT	fU, fV;
		MousePositionToUVCoordinate( &(m_PrevMouseStatus.pos), &fU, &fV );
		if ( fU == 0.0f || fU == 1.0f ) 
		{
			if ( m_PrevMouseStatus.pos.x == 0.0f )
				g_pcsAgcmCamera->ChangeRotAngleVelocity( 1.0f * ulDeltaTime / 30.0f, TRUE );
			else 
				g_pcsAgcmCamera->ChangeRotAngleVelocity( -1.0f * ulDeltaTime / 30.0f , TRUE );
		}
		*/
	}
	
	if( GetCurrentMode() == EDITMODE_OTHERS )
	{
		g_pMainFrame->m_pTileList->m_pOthers->OnIdle( ulClockCount );
	}

#ifdef _PROFILE_
	AuProfileManager::Increment_Frame_Counter();
#endif
	
	//@{ kday 20050513
	// ¿ÀºêÁ§Æ® Ãß°¡Á¤º¸ Ç¥½Ã¸¦ À§ÇÑ IdleÃ³¸®.
	AcuObjecWire::bGetInst().bOnIdle();
	//@} kday

	return FALSE;
}

RtWorldImport *	MainWindow::CreateWorldImport	(  ApWorldSector * pGeo  )
{
/*
* Creates an world import and allocates and initializes the world's 
* vertices and triangles...
	*/
    RtWorldImport *			worldImport	;
    RtWorldImportTriangle *	triangles	;
    RtWorldImportVertex *	vertices	;
    RpMaterial *			material	;
    RwTexture *				texture		;
    RwTexture *				texture2	;
    RwInt32					matIndex	;
//    RwInt32					matIndex2	;
    RwSurfaceProperties		surfProp	;

	// µ¥ÀÌÅ¸ ÁØºñ.
	int	MAP_WIDTH	= GDEPTH;
	int	MAP_HEIGHT	= GDEPTH;

	int	MAP_SIZE	= ( ( MAP_WIDTH + 1 ) * ( MAP_HEIGHT + 1 ) );
	int	NOT			= ( ( MAP_WIDTH ) * ( MAP_HEIGHT ) * 2 );	// Number of Triangles
	int	NOV			= NOT * 3;

	RwV3d * VertexList = new RwV3d[ MAP_SIZE ];

	#define MAP(x,z)	( VertexList[ ( z ) * ( MAP_WIDTH + 1 ) + ( x ) ] )

    RwInt32 i, j, k;
	
	worldImport = RtWorldImportCreate();
	if( worldImport == NULL )
    {
        return NULL;
    }
	
	/* 
	* Allocate the memory to store the world's vertices and triangles... 
	*/
	RtWorldImportAddNumVertices(worldImport, NOV);
    RtWorldImportAddNumTriangles(worldImport, NOT);

	// Load Texture
    texture = RwTextureRead(RWSTRING ("texture.bmp"), RWSTRING ("grass2_mask1.png"));
    RwTextureSetFilterMode(texture, rwFILTERLINEAR);

    texture2 = RwTextureRead(RWSTRING ("grass2.png"), RWSTRING ("grass2_mask2.png"));
    RwTextureSetFilterMode(texture2, rwFILTERLINEAR);
	

    /* 
	* Create the materials. These materials are created with a reference
	* count of 1, meaning this application has ownership. Subsequently,
	* when the materials are associated with a worldImport, their reference
	* counts are incremented, indicating ownership also by the worldImport...
	*/ 
	
    material	= RpMaterialCreate();
    RpMaterialSetTexture(material, texture);

	RpMatFXMaterialSetEffects		( material , rpMATFXEFFECTDUAL  );
	RpMatFXMaterialSetDualTexture	( material , texture2  );
	RpMatFXMaterialSetupDualTexture	( material , texture2 , rwBLENDSRCCOLOR , rwBLENDDESTCOLOR  );//rwBLENDONE , rwBLENDONE );

    matIndex	= RtWorldImportAddMaterial(worldImport, material);
	
    /*
	* Set the surface reflection coefficients...
	*/
    surfProp.ambient = 0.3f;
    surfProp.diffuse = 0.7f;
    surfProp.specular = 0.0f;
    RpMaterialSetSurfaceProperties(material, &surfProp);
	
    vertices	= RtWorldImportGetVertices	(	worldImport	);
    triangles	= RtWorldImportGetTriangles	(	worldImport	);
	
	memset( vertices	, 0 , sizeof(	RtWorldImportVertex		) * NOV	);
	memset( triangles	, 0 , sizeof(	RtWorldImportTriangle	) * NOT	);
	
    /* 
	* Define vertices and triangles for pentagons... 
	*/
    for (i = 0, k = 0; i < MAP_HEIGHT + 1 ; i++)
    {
		for (j = 0; j < MAP_WIDTH + 1 ; j++, k += 4)
		{
			MAP(j, i).x = ( j - MAP_WIDTH	/ 2.0f ) * 10.0f;
			MAP(j, i).z = ( i - MAP_HEIGHT	/ 2.0f ) * 10.0f;
			MAP(j, i).y = 0;
		}
	}
	
	// k´Â vertex index
    for (i = 0, k = 0; i < MAP_HEIGHT ; i++)
    {
		for (j = 0; j < MAP_WIDTH ; j++, k += 6 ) // Æú¸®°Ç µÎ°³¾¿ Ã³¸®ÇÔ.
		{
			RwV3d normal = {0, 1, 0};
			
			// ¿ÞÂÊ À§ÀÇ°Í.
			vertices->OC				= MAP(j, i);
			vertices->normal			= normal;
			vertices->texCoords[0].u	= 0.0f;
			vertices->texCoords[0].v	= 0.0f;
			vertices->texCoords[1].u	= 0.0f;
			vertices->texCoords[1].v	= 0.0f;
//			vertices->texCoords[2].u	= 0.0f;
//			vertices->texCoords[2].v	= 0.0f;
			vertices->matIndex			= matIndex;
			vertices++;
			
			vertices->OC				= MAP(j + 1, i);
			vertices->normal			= normal;
			vertices->texCoords[0].u	= 1.0f;
			vertices->texCoords[0].v	= 0.0f;
			vertices->texCoords[1].u	= 0.0f;
			vertices->texCoords[1].v	= 0.0f;
//			vertices->texCoords[2].u	= 1.0f;
//			vertices->texCoords[2].v	= 0.0f;
			vertices->matIndex			= matIndex;
			vertices++;
			
			vertices->OC				= MAP(j, i + 1);
			vertices->normal			= normal;
			vertices->texCoords[0].u	= 0.0f;
			vertices->texCoords[0].v	= 1.0f;
			vertices->texCoords[1].u	= 0.0f;
			vertices->texCoords[1].v	= 0.0f;
//			vertices->texCoords[2].u	= 0.0f;
//			vertices->texCoords[2].v	= 1.0f;
			vertices->matIndex			= matIndex;
			vertices++;
			
			// ¿À¸¥ÂÊ ¾Æ·¡°Í.
			vertices->OC				= MAP(j + 1, i);
			vertices->normal			= normal;
			vertices->texCoords[0].u	= 1.0f;
			vertices->texCoords[0].v	= 0.0f;
			vertices->texCoords[1].u	= 0.0f;
			vertices->texCoords[1].v	= 0.0f;
//			vertices->texCoords[2].u	= 1.0f;
//			vertices->texCoords[2].v	= 0.0f;
			vertices->matIndex			= matIndex;
			vertices++;

			vertices->OC				= MAP(j + 1, i + 1);
			vertices->normal			= normal;
			vertices->texCoords[0].u	= 1.0f;
			vertices->texCoords[0].v	= 1.0f;
			vertices->texCoords[1].u	= 0.0f;
			vertices->texCoords[1].v	= 0.0f;
//			vertices->texCoords[2].u	= 1.0f;
//			vertices->texCoords[2].v	= 1.0f;
			vertices->matIndex			= matIndex;
			vertices++;
			
			vertices->OC				= MAP(j, i + 1);
			vertices->normal			= normal;
			vertices->texCoords[0].u	= 0.0f;
			vertices->texCoords[0].v	= 1.0f;
			vertices->texCoords[1].u	= 0.0f;
			vertices->texCoords[1].v	= 0.0f;
//			vertices->texCoords[2].u	= 0.0f;
//			vertices->texCoords[2].v	= 1.0f;
			vertices->matIndex			= matIndex;
			vertices++;

			// »ï°¢Çü ¼³Á¤.
			triangles->vertIndex[0] = k + 2;
			triangles->vertIndex[1] = k + 1;
			triangles->vertIndex[2] = k;
			triangles->matIndex = matIndex;
			triangles++;
			
			triangles->vertIndex[0] = k + 5;
			triangles->vertIndex[1] = k + 4;
			triangles->vertIndex[2] = k + 3;
			triangles->matIndex = matIndex;
			triangles++;
			/*
			if (i != 0 && j != 0)
			{
			v->y = (MAP(j, i - 1).y + MAP(j - 1, i)) / 2.0 + (rand() % 20) / 10.0 -1.0;
			}
			*/
		}
	}
	
    RwTextureDestroy(texture);
	RwTextureDestroy(texture2);

    RpMaterialDestroy(material);

	if( VertexList ) delete [] VertexList;
	
	return worldImport;
}

RpWorld *		MainWindow::CreateWorld			( void )
{
    RtWorldImport *worldImport;
    RtWorldImportParameters params;
    RpWorld *world;

    worldImport = CreateWorldImport( NULL );
    if( worldImport == NULL )
    {
        return NULL;
    }

    //RtWorldImportParametersInitialize(&params);
    //RtWorldImportParametersInit(&params);

    params.flags = rpWORLDTEXTURED | rpWORLDNORMALS | rpWORLDLIGHT | rpWORLDTEXTURED2;
    params.conditionGeometry	= FALSE	;
    params.calcNormals			= FALSE	;
	params.numTexCoordSets		= 2		; // ¸ÖÆ¼ÅØ½ºÃÄ Áö¿ø.

    /*
     * Create the BSP world from the worldImport. 
     * During this process the materials have their reference counts incremented
     * to indicate ownership also by the BSP world...
     */
    world = RtWorldImportCreateWorld(worldImport, &params);
    if( world )
    {
		RpCollisionBuildParam	param;
		RpCollisionWorldBuildData( world , & param );
    }

    /*
     * Destroy the worldImport.
     * This process will also decrement the reference counts on the 
     * materials without actually deleting these resources because they 
     * now have other owners (the BSP world). Now we can simply use 
     * RpWorldDestroy later when the application has finished with it...
     */
    RtWorldImportDestroy(worldImport);

    return world;
}

void MainWindow::LoadPolygonData()
{
	// ±âÁ¸¿¡ ÆÄÀÏ µ¥ÀÌÅ¸µéÀ» ÀÐ¾îµéÀÓ.

//	RwFrame		*pFrame = RwCameraGetFrame	( g_pEngine->m_pCamera	);
//	RwV3d		pos = { 0.0f , 10.0f , 0.0f };
//	RwFrameTranslate( pFrame , & pos , rwCOMBINEREPLACE );
}

void	MainWindow::CreateSector( BOOL bDetail )
{
	if( m_pCurrentGeometry )
	{
		if( bDetail )
		{
			CSectorCreateDlg dlg;
			switch( g_pMainFrame->m_Document.m_nDWSectorSetting )
			{
			case DWSECTOR_ALLROUGH			:
				{
					dlg.m_strDetail = "·¯ÇÁ¸Ê";
					if( m_pCurrentGeometry->IsLoadedDetailData( SECTOR_LOWDETAIL ) )
					{
						dlg.m_nDepth = m_pCurrentGeometry->D_GetDepth( SECTOR_LOWDETAIL );
					}
					else
					{
						//if( m_pCurrentGeometry->IsLoaded( SECTOR_HIGHDETAIL ) )
						dlg.m_nDepth = ALEF_SECTOR_ROUGH_DEPTH;
						//else
						//{
						//	DisplayMessage( AEM_ERROR , "Æ¼Å×ÀÏ¸ÊÀÌ ¾÷´Â °÷Àº ·¯ÇÁ¸ÊÀ» »ý¼ºÇÒ¼ö ¾ø½À´Ï´Ù." );
						//	return;
						//}
					}
				}
				break;
			case DWSECTOR_EACHEACH			:
			case DWSECTOR_ALLDETAIL			:
			case DWSECTOR_ACCORDINGTOCURSOR	:
				{
					dlg.m_strDetail = "µðÅ×ÀÏ¸Ê";
					if( m_pCurrentGeometry->IsLoadedDetailData( SECTOR_HIGHDETAIL ) )
					{
						dlg.m_nDepth = m_pCurrentGeometry->D_GetDepth( SECTOR_HIGHDETAIL );
					}
					else
						dlg.m_nDepth = ALEF_SECTOR_DEFAULT_DEPTH;
				}
				break;
			}

			if( dlg.DoModal() == IDOK )
			{
				switch( g_pMainFrame->m_Document.m_nDWSectorSetting )
				{
				case DWSECTOR_ALLROUGH			:
					{
						m_pCurrentGeometry->CreateDetailData( SECTOR_LOWDETAIL , dlg.m_nDepth , dlg.m_fDefaultHeight * 100.0f );
						m_pCurrentGeometry->SetCurrentDetail( SECTOR_LOWDETAIL );
						AGCMMAP_THIS->Update( m_pCurrentGeometry );
					}
					break;
				case DWSECTOR_EACHEACH			:
				case DWSECTOR_ALLDETAIL			:
				case DWSECTOR_ACCORDINGTOCURSOR	:
					{
						m_pCurrentGeometry->CreateDetailData( SECTOR_HIGHDETAIL , dlg.m_nDepth , dlg.m_fDefaultHeight * 100.0f );
						m_pCurrentGeometry->SetCurrentDetail( SECTOR_HIGHDETAIL );
						AGCMMAP_THIS->Update( m_pCurrentGeometry );
					}
					break;
				}
			}
		}
		else
		{
			int depth;

			switch( g_pMainFrame->m_Document.m_nDWSectorSetting )
			{
			case DWSECTOR_ALLROUGH			:
				{
					if( m_pCurrentGeometry->IsLoadedDetailData( SECTOR_LOWDETAIL ) )
					{
						// ¸Þ½ÃÁöÃ¢..
					}
					else
					{
						if( m_pCurrentGeometry->IsLoadedDetailData( SECTOR_HIGHDETAIL ) )	
						{
							// do no op
						}
						else
						{
							//DisplayMessage( AEM_ERROR , "Æ¼Å×ÀÏ¸ÊÀÌ ¾÷´Â °÷Àº ·¯ÇÁ¸ÊÀ» »ý¼ºÇÒ¼ö ¾ø½À´Ï´Ù." );
							//return;
						}
					}

					depth = ALEF_SECTOR_ROUGH_DEPTH;
					m_pCurrentGeometry->CreateDetailData( SECTOR_LOWDETAIL , depth );
					m_pCurrentGeometry->SetCurrentDetail( SECTOR_LOWDETAIL );
					AGCMMAP_THIS->Update( m_pCurrentGeometry );
				}
				break;
			case DWSECTOR_EACHEACH			:
			case DWSECTOR_ALLDETAIL			:
			case DWSECTOR_ACCORDINGTOCURSOR	:
				{
					if( m_pCurrentGeometry->IsLoadedDetailData( SECTOR_HIGHDETAIL ) )
					{
						// ¸Þ½ÃÁöÃ¢..
					}

					depth = ALEF_SECTOR_DEFAULT_DEPTH;
					m_pCurrentGeometry->CreateDetailData( SECTOR_HIGHDETAIL , depth );
					m_pCurrentGeometry->SetCurrentDetail( SECTOR_HIGHDETAIL );
					AGCMMAP_THIS->Update( m_pCurrentGeometry );
				}
				break;
			}
		}
	}
	else
	{
		// ±×³É Á¾·á.
	}
}

void		MainWindow::MoveToSectorCenter( ApWorldSector * pSector )
{
	if( g_pMainFrame->m_Document.IsInFirstPersonViewMode() ) return;

	ASSERT( NULL != g_pEngine				);
	ASSERT( NULL != g_pEngine->m_pCamera	);

	if( NULL == pSector				||
		NULL == g_pEngine			||
		NULL == g_pEngine->m_pCamera)
	{
		ApWorldSector	* pSector = NULL;

		if( g_pcsApmMap )
		{
			pSector = 		g_pcsApmMap->GetSectorByArray( 
									ALEF_LOAD_RANGE_X1		+ ( ALEF_LOAD_RANGE_X2 - ALEF_LOAD_RANGE_X1 ) / 2	,
									0 ,
									( ALEF_LOAD_RANGE_Y2 - ALEF_LOAD_RANGE_Y1 ) / 2 + ALEF_LOAD_RANGE_Y1		);

			if( NULL == pSector )
			{

				
				pSector = AGCMMAP_THIS->LoadSector(
									ArrayIndexToSectorIndexX( ALEF_LOAD_RANGE_X1		+ ( ALEF_LOAD_RANGE_X2 - ALEF_LOAD_RANGE_X1 ) / 2 )	,
									0 ,
									ArrayIndexToSectorIndexZ( ( ALEF_LOAD_RANGE_Y2 - ALEF_LOAD_RANGE_Y1 ) / 2 + ALEF_LOAD_RANGE_Y1		) ) ;
			}


			MoveToSectorCenter(pSector);
		}
		return;
	}

	RwFrame		*pFrame = RwCameraGetFrame	( g_pEngine->m_pCamera	);

 	ASSERT( NULL != pFrame					);

	ASSERT( pFrame == pFrame->root && "ÄÉ¸¯ÅÍ ½ÃÁ¡ »ç¿ëÈÄ¿¡ TABÀº Àá½Ã »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù." );
	if( pFrame != pFrame->root ) return;

	FLOAT	fHeight	= ALEF_SECTOR_WIDTH * 1.5f;
	
	RwV3d		pos = { 
						pSector->GetXStart() + ( pSector->GetXEnd() - pSector->GetXStart() ) / 2 ,
						fHeight,
						pSector->GetZStart() + ( pSector->GetZEnd() - pSector->GetZStart() ) / 2 + ALEF_SECTOR_WIDTH / 10.0f };

	RwV3d		at = {
						pSector->GetXStart() + ( pSector->GetXEnd() - pSector->GetXStart() ) / 2 ,
						0.0f ,
						pSector->GetZStart() + ( pSector->GetZEnd() - pSector->GetZStart() ) / 2 };
	RwV3d		up = {
						0.0f ,
						0.0f ,
						-1.0f };
	RwV3d		right = {
						1.0f ,
						0.0f ,
						0.0f };

	// ¸¶°íÀÚ (2003-11-25 ¿ÀÈÄ 1:01:12) : Ä«¸Þ¶ó ºüÁö´ÂÇö»óÁ¦°Å..
	fHeight = pSector->D_GetHeight( pos.x , pos.z );
	if( fHeight > pos.y ) pos.y = fHeight + ALEF_SECTOR_WIDTH / 10.0f;

	RwV3dSub		( &at , &pos	, &at	);
	RwV3dScale		( &at , &at		, -1.0f	);
	RwV3dNormalize	( &at , &at				);

	RwMatrix	*	pMatrix = RwFrameGetMatrix( pFrame );

	pMatrix->pos	= pos	;
	pMatrix->at		= at	;
	pMatrix->up		= up	;
	pMatrix->right	= right	;

	RwMatrixUpdate			( pMatrix						);
	RwFrameUpdateObjects	( pFrame						);
	RwCameraSetFrame		( g_pEngine->m_pCamera , pFrame	);

	RwMatrix	*m;
	//RwV3d xaxis	= { 1.0f , 0.0f , 0.0f };
	RwV3d		yaxis	= { 0.0f , 1.0f , 0.0f };
	
	m = RwFrameGetMatrix( pFrame );
	ASSERT( NULL != m );

	RwV3dCrossProduct	( &m->right	, &yaxis	, &m->at	);
	m->right.y = 0.0f;
	RwV3dNormalize		( &m->right	, &m->right	);
	RwV3dCrossProduct	( &m->up	, &m->at	, &m->right	);
	RwV3dNormalize		( &m->up	, &m->up	);
	RwV3dCrossProduct	( &m->right	, &m->up	, &m->at	);

	RwMatrixOrthoNormalize( m , m );

	AuPOS	posCamera;
	RwV3d	*pCameraPos = RwMatrixGetPos( m );
	ASSERT( NULL != pCameraPos );

	posCamera.x	= pCameraPos->x;
	posCamera.y	= pCameraPos->y;
	posCamera.z	= pCameraPos->z;

	if( g_pcsAgcmEventNature )
	{
		if( posCamera.y > 10000.0f / 1.2f )
			g_pcsAgcmEventNature->SetSkyHeight( posCamera.y * 1.2f );
		else
			g_pcsAgcmEventNature->SetSkyHeight( 10000.0f			);

		g_pcsAgcmEventNature->SetCharacterPosition( posCamera );
	}
}

void		MainWindow::CameraIdleMovement()
{
	PROFILE("MainWindow::CameraIdleMovement");
	UINT	currenttick	= GetTickCount	();
	UINT	pasttick	= currenttick - m_uLastKeyPressTime ;//g_pEngine->GetPastTime	();
	m_uLastKeyPressTime	= currenttick;

	/*
	// ²Å»ç¸® ²¸ÀÖ´Â Glyph Spin..
	if( m_pGlyph )
	{
		float angle;
		RwV3d	yaxis	= { 0.0f , 1.0f , 0.0f };
		angle		= 20.0f * ( ( float ) pasttick / 1000.0f );		// 30µµ.
		RpGlyphSetRotate( m_pGlyph , & yaxis , angle );
	}
	*/

	if( !m_bForwardKeyDown		&& !m_bBackwardKeyDown	&& !m_bStepLeftKeyDown	&&
		!m_bStepRightKeyDown	&& !m_bLiftUpKeyDown	&& !m_bLiftDownKeyDown	&&
		!m_bRotateCCWDown		&& !m_bRotateCWDown		&& !m_bUp				&&
		!m_bDown																)
		return;

	BOOL	bUp		, bForwardKeyDown	;
	BOOL	bDown	, bBackwardKeyDown	;

	bUp					= m_bUp				;
	bForwardKeyDown		= m_bForwardKeyDown	;

	bDown				= m_bDown			;
	bBackwardKeyDown	= m_bBackwardKeyDown;

	if( m_bUp				&& ISBUTTONDOWN( VK_CONTROL ) )
	{
		bUp					= FALSE	;
		bForwardKeyDown		= TRUE	;
	}

	if( m_bForwardKeyDown	&& ISBUTTONDOWN( VK_CONTROL ) )
	{
		bUp					= TRUE	;
		bForwardKeyDown		= FALSE	;
	}
	
	if( m_bDown				&& ISBUTTONDOWN( VK_CONTROL ) )
	{
		bDown				= FALSE	;
		bBackwardKeyDown	= TRUE	;
	}

	if( m_bBackwardKeyDown	&& ISBUTTONDOWN( VK_CONTROL ) )
	{
		bDown				= TRUE	;
		bBackwardKeyDown	= FALSE	;
	}

//	MTRACE( "Currenttick %04d - Delay %04d\n" , currenttick % 10000 , pasttick );


	RwFrame		*pFrame		= RwCameraGetFrame	( g_pEngine->m_pCamera	);
	RwMatrix	*pMatrix	= RwFrameGetMatrix	( pFrame				);

	RwV3d		up, right, at , pos , trans;

	right	= *RwMatrixGetRight	( pMatrix	);
	up		= *RwMatrixGetUp	( pMatrix	);
	at		= *RwMatrixGetAt	( pMatrix	);
	pos		= *RwMatrixGetPos	( pMatrix	);

	RwV3dNormalize( &right	, &right	);
	RwV3dNormalize( &up		, &up		);
	RwV3dNormalize( &at		, &at		);

	short state = GetAsyncKeyState( VK_SHIFT );

	float		distance;
	float		angle	;
	if( state < 0 )
	{
		distance	= g_pMainFrame->m_Document.m_fCameraMovingSpeed * 2.0f * ( ( float ) pasttick / 1000.0f );	// 1ÃÊ¿¡ 1000.0f ÀÌµ¿.
		angle		= 120.0f * ( ( float ) pasttick / 1000.0f );		// 30µµ.
	}
	else
	{
		distance	= g_pMainFrame->m_Document.m_fCameraMovingSpeed * ( ( float ) pasttick / 1000.0f );	// 1ÃÊ¿¡ 100.0f ÀÌµ¿.
		angle		= 60.0f * ( ( float ) pasttick / 1000.0f );		// 30µµ.
	}

	if( g_pMainFrame->m_Document.IsInFirstPersonViewMode() )
	{
		// FPS ºä¿¡¼­´Â ¾Æ¹«°Íµµ ÇÏÁö ¾Ê´Â´Ù.
	}
	else
	{
		if( bForwardKeyDown )
		{
			// ÀüÁø!
			RwV3dScale		( &trans	, &at		, distance				);
			if( __CropCameraMovement( &pos , &trans ) )
				RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
		}

		if( bBackwardKeyDown )
		{
			// ÈÄÁø!
			RwV3dScale		( &trans	, &at		, -distance				);
			if( __CropCameraMovement( &pos , &trans ) )
				RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
		}

		if( bUp )
		{
			// ¾ÕÀ¸·Î!
			at.y	= 0.0f;
			RwV3dNormalize( &at	, &at	);

			RwV3dScale		( &trans	, &at		, distance				);
			if( __CropCameraMovement( &pos , &trans ) )
				RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
		}

		if( bDown )
		{
			// µÚ·Î!
			at.y	= 0.0f;
			RwV3dNormalize( &at	, &at	);
			RwV3dScale		( &trans	, &at		, -distance				);
			if( __CropCameraMovement( &pos , &trans ) )
				RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
		}


		if( m_bStepLeftKeyDown )
		{
			// ÁÂ·Î!
			RwV3dScale		( &trans	, &right	, distance				);
			if( __CropCameraMovement( &pos , &trans ) )
				RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
		}
		if( m_bStepRightKeyDown )
		{
			// ¿ì·Î!
			RwV3dScale		( &trans	, &right	, -distance				);
			if( __CropCameraMovement( &pos , &trans ) )
				RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
		}

		if( m_bLiftUpKeyDown )
		{
			RwV3d yaxis	= { 0.0f , 1.0f , 0.0f };
			// lift up!
			RwV3dScale		( &trans	, &yaxis	, distance				);
			if( __CropCameraMovement( &pos , &trans ) )
				RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
		}

		if( m_bLiftDownKeyDown )
		{
			RwV3d yaxis	= { 0.0f , 1.0f , 0.0f };
			// --;.. drop!
			RwV3dScale		( &trans	, &yaxis	, -distance				);
			if( __CropCameraMovement( &pos , &trans ) )
				RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
		}

		if( m_bRotateCCWDown && m_pCurrentGeometry )
		{
			RwV3d	yaxis	= { 0.0f , 1.0f , 0.0f };
			RwV3d	point = m_Position;
			point.y	= 0.0f;

			RwV3d	ref;
			ref.x	= -point.x;
			ref.y	= -point.y;
			ref.z	= -point.z;

			// ÇÁ·¹ÀÓÀ» ÀÌµ¿..
			RwFrameTranslate( pFrame , &ref , rwCOMBINEPOSTCONCAT );

			// È¸Àü..
			RwFrameRotate( pFrame , &yaxis, angle , rwCOMBINEPOSTCONCAT );

			// ´Ù½Ã ¿øÀ§Ä¡·Î ÀÌµ¿..
			RwFrameTranslate( pFrame , &point , rwCOMBINEPOSTCONCAT );
		}

		if( m_bRotateCWDown && m_pCurrentGeometry )
		{
			RwV3d	yaxis	= { 0.0f , 1.0f , 0.0f };
			RwV3d	point = m_Position;
			point.y	= 0.0f;

			RwV3d	ref;
			ref.x	= -point.x;
			ref.y	= -point.y;
			ref.z	= -point.z;

			// ÇÁ·¹ÀÓÀ» ÀÌµ¿..
			RwFrameTranslate( pFrame , &ref , rwCOMBINEPOSTCONCAT );

			// È¸Àü..
			RwFrameRotate( pFrame , &yaxis, -angle , rwCOMBINEPOSTCONCAT );

			// ´Ù½Ã ¿øÀ§Ä¡·Î ÀÌµ¿..
			RwFrameTranslate( pFrame , &point , rwCOMBINEPOSTCONCAT );
		}

		// ÀÌ°Å ÇØÁà¾ßÇÔ..
		RwMatrixOrthoNormalize( pMatrix , pMatrix );
		
		// Event Nature Ä«¸Þ¶ó ¼³Á¤..

		AuPOS	posCamera;
		RwV3d	*pCameraPos = RwMatrixGetPos( pMatrix );
		ASSERT( NULL != pCameraPos );

		posCamera.x	= pCameraPos->x;
		posCamera.y	= pCameraPos->y;
		posCamera.z	= pCameraPos->z;
/*
		if( posCamera.y > 10000.0f / 1.2f )
			g_pcsAgcmEventNature->SetSkyHeight( posCamera.y * 1.2f );
		else
			g_pcsAgcmEventNature->SetSkyHeight( 10000.0f			);

*/
		if( g_pcsAgcmEventNature )
		{
			//if( pCameraPos->y < AgcmEventNature::SKY_HEIGHT )
			{
				g_pcsAgcmEventNature->ShowSky( TRUE );
			}
			//else
			//{
			//	g_pcsAgcmEventNature->ShowSky( FALSE );
			//}

			g_pcsAgcmEventNature->SetCharacterPosition( posCamera );
		}

		if( g_pcsAgcmShadow2 )
		{
			ApWorldSector * pSector = g_pcsApmMap->GetSector( pCameraPos->x , pCameraPos->z );
			if( m_pCameraSector != pSector )
			{
				g_MainWindow.m_pCameraSector = pSector;
				if( pSector )
				{
					g_pcsAgcmShadow2->CB_SECTOR_CAHNGE( ( PVOID ) pSector , ( PVOID ) g_pcsAgcmShadow2 , NULL );
				}
			}
		}

	}
}

RpWorld * MainWindow::LoadWorld()
{
	RpWorld *pWorld;
	pWorld = AGCMMAP_THIS->GetWorld();
	return pWorld;
}

int	MainWindow::GetCurrentMode()	// ÇöÀç ÀÛ¾÷ÁßÀÎ ¸ðµå¸¦ ¾ò¾î¿È.
{
	// ¸ðµå¸¦ Ã£¾Æ¼­ ¸®ÅÏ
	// ÇØ´ç ¸ðµå¿¡ ´ëÇÑ Á¤º¸´Â MapToolConst.h ¿¡ µé¾îÀÖ´Ù.
	return g_pMainFrame->m_Document.m_nSelectedMode;
}

int	MainWindow::GetCurrentLayer()	// ÇöÀç ÀÛ¾÷ÁßÀÎ ¸ðµå¸¦ ¾ò¾î¿È.
{
	// ¸ðµå¸¦ Ã£¾Æ¼­ ¸®ÅÏ
	// ÇØ´ç ¸ðµå¿¡ ´ëÇÑ Á¤º¸´Â MapToolConst.h ¿¡ µé¾îÀÖ´Ù.
	return g_pMainFrame->m_Document.m_nCurrentTileLayer;
}


ApDetailSegment * MainWindow::GetCurrentPolygonsTileIndex( ApWorldSector *pWorldSector , int polygonindex ,
													  int mousex , int mousey ,
													  int * pPosX , int *pPosY , // ÁÂÇ¥..
													  int *pPosDetailedX , int *pPosDetailedY )// ¼¼ºÎÁÂÇ¥ , ³ªÁß¿¡ »ç¿ëÇÔ.
{
// ¿ùµå¼½ÅÍ¿Í Æú¸®°Ç ÀÎµ¦½º·Î , ÇØ´ç Æú¸®°ÇÀÌ ¼ÓÇÑ ¼½ÅÍ¿Í , ±×ÀÇ ÁÂÇ¥¸¦ ¾ò¾î³½´Ù.
	if( pWorldSector == NULL ) return NULL;
	
	// ÇØ´ç Æú¸®°ÇÀ» Ã£¾Æ³¿.
	RpDWSector *	pDWSector		= AGCMMAP_THIS->RpDWSectorGetDetail		(	pWorldSector ,
										AGCMMAP_THIS->RpDWSectorGetCurrentDetail( pWorldSector )	);
	ASSERT( NULL != pDWSector			);
	ASSERT( NULL != pDWSector->geometry	);
	
	if( NULL == pDWSector->geometry ) return NULL;

	RpMorphTarget *	pMorphTarget	= RpGeometryGetMorphTarget	( pDWSector->geometry , 0	);
	
	ASSERT( NULL != pMorphTarget		);		

	RwV3d *			pVertices		= RpMorphTargetGetVertices	( pMorphTarget				);

	// ASSERT( NULL != pVertices		);		
	if( NULL == pVertices )
	{
		return NULL;
	}

	RpTriangle *	pTriangles		= RpGeometryGetTriangles	( pDWSector->geometry		);			//	Polygons themselves 

	ASSERT( NULL != pTriangles		);		

	int	polygoncount = RpGeometryGetNumTriangles( pDWSector->geometry );

	if( polygoncount == 0 ) return NULL;

	// ÁÂÇ¥¸¦ ±¸ÇØ³¿
	RwV3d	lefttop , rightbottom;
	int		i;
	{
		lefttop		= pVertices[ pTriangles[ polygonindex ].vertIndex[ 0 ] ];
		rightbottom	= pVertices[ pTriangles[ polygonindex ].vertIndex[ 0 ] ];

		for( i = 1 ; i < 3 ; i ++ )
		{
			if( lefttop.x		> pVertices[ pTriangles[ polygonindex ].vertIndex[ i ] ].x )
				lefttop.x		= pVertices[ pTriangles[ polygonindex ].vertIndex[ i ] ].x;
			if( lefttop.z		> pVertices[ pTriangles[ polygonindex ].vertIndex[ i ] ].z )
				lefttop.z		= pVertices[ pTriangles[ polygonindex ].vertIndex[ i ] ].z;

			if( rightbottom.x	< pVertices[ pTriangles[ polygonindex ].vertIndex[ i ] ].x )
				rightbottom.x	= pVertices[ pTriangles[ polygonindex ].vertIndex[ i ] ].x;
			if( rightbottom.z	< pVertices[ pTriangles[ polygonindex ].vertIndex[ i ] ].z )
				rightbottom.z	= pVertices[ pTriangles[ polygonindex ].vertIndex[ i ] ].z;
		}
	}

//	// lefttop , rightbottom ¿¡´Â y°ªÀº ÀÇ¹Ì°¡ ¾ø°í , °¢°¢ ÁÂÇ¥¸¦ °¡Áø´Ù.
//
//	ApWorldSector * pSector = g_pcsApmMap->GetSector( lefttop.x , lefttop.z );
//	if( pSector == NULL )
//	{
//		// Ã£´Â°Ô ¾ø¾î¿ä
//		return NULL;
//	}

	// ÁÂÇ¥¸¦ Ã£´Â´Ù.
	int	posX , posY;
	ApDetailSegment	*pSegment;

	if( pSegment = pWorldSector->D_GetSegment( lefttop.x , lefttop.z , &posX , &posY ) )
	{
		// Ã£¾Æµû.
		if( pPosX ) *pPosX = posX;
		if( pPosY ) *pPosY = posY;
		return pSegment;
	}
	else
	{
		// ³ªº¸°í ¾îÂ¼¶ó°í --;
		// ÁøÂ¥ ÀÌ·±ÀÏ ÀÖ³ª?
		return NULL;
	}
}

BOOL	MainWindow::GetWorldSectorInfo( stFindMapInfo * pMapInfo , ApWorldSector * pSector , int x , int z , int detailedX , int detailedZ )
{
	// ¸¶°íÀÚ (2004-06-16 ¿ÀÈÄ 2:53:11) : Ã³¸®º¯°æÀ¸·Î .. agcmmap À¸·Î ÅëÇÕ..
	return AGCMMAP_THIS->GetWorldSectorInfo( pMapInfo , pSector , x , z , detailedX , detailedZ );
}


float		MainWindow::SetSectorHeight( stFindMapInfo * pFindMapInfo , float height ) // 9°³ÀÇ ¹è¿­À» ÀÔ·ÂÀ¸·Î..
{
	ASSERT( NULL != pFindMapInfo			);
	ASSERT( NULL != pFindMapInfo->pSector	);

	if( pFindMapInfo->pSector == NULL) return 0.0f;

	SaveSetChangeGeometry();

	m_UndoManager.AddGeometryActionUnit( pFindMapInfo->pSector , pFindMapInfo->nSegmentX , pFindMapInfo->nSegmentZ , height );

	AGCMMAP_THIS->D_SetHeight( pFindMapInfo->pSector , pFindMapInfo->pSector->GetCurrentDetail() ,
		pFindMapInfo->nSegmentX , pFindMapInfo->nSegmentZ ,	height );

	return height;
}

void		MainWindow::DrawHeightGrid				()
{
	// ÇöÀç ÁöÇüÀÇ Wire Frame À» ±×·ÁÁÜ.
	INT32	r,g,b,a;
	r = 255 , g = 255 , b = 255 , a = 128;
	
	BOOL	bShowPolygonSplit;
	if( GetCurrentMode() == EDITMODE_GEOMETRY &&
		g_pMainFrame->m_Document.m_nBrushType == BRUSHTYPE_EDGETURN )
		bShowPolygonSplit = TRUE	;
	else
		bShowPolygonSplit = FALSE	;
	
	if( m_pCurrentGeometry && m_pCurrentGeometry->GetCurrentDetail() > SECTOR_EMPTY )
	{
		// Close Sector Á¤º¸ ±¸ÇØº½..
		// ( m_pCurrentGeometry->IsAvailableCloseSector( TD_WEST ) 
		// ¸¶°íÀÚ (2002-08-02 ¿ÀÈÄ 12:19:27) : Á¶¸¸°£ Ã³¸®ÇÏÀÚ..

		int		nDepth	= MAP_DEFAULT_DEPTH;//( int ) ( ( FLOAT ) m_pCurrentGeometry->D_GetDepth() );

		// ÁÂÇ¥ ¾ò¾î³¿..
		FLOAT	fStartX , fStartZ;
		INT32	nIndexX , nIndexZ;
		m_pCurrentGeometry->D_GetSegment( m_Position.x , m_Position.z , &nIndexX , &nIndexZ );
		m_pCurrentGeometry->D_GetSegment( nIndexX , nIndexZ , &fStartX , &fStartZ );
		fStartX	-= MAP_STEPSIZE * ( FLOAT ) nDepth / 2.0f;
		fStartZ	-= MAP_STEPSIZE * ( FLOAT ) nDepth / 2.0f;

		RwIm3DVertex	* pLineList	= m_pVertexBuffer	;
		RwIm3DVertex	* pLineList2= m_pVertexBuffer2	;
		RwImVertexIndex	* pIndex	= m_pVertexIndex	;
		RwImVertexIndex	* pIndex2	= m_pVertexIndex2	;
		ApDetailSegment	* pSegment	= NULL				;
		ApWorldSector	* pSector	= NULL				;

		float	fX	, fZ;
		int		i	, j ;
		float	height	;

		for( j = 0 ; j < nDepth + 1 ; j ++ )
		{
			for( i = 0 ; i < nDepth + 1 ; i ++ )
			{
				fX = fStartX + ( FLOAT ) i * MAP_STEPSIZE ;
				fZ = fStartZ + ( FLOAT ) j * MAP_STEPSIZE ;
//				fX = m_pCurrentGeometry->GetXStart() + ( FLOAT ) i * m_pCurrentGeometry->GetStepSizeX() ;
//				fZ = m_pCurrentGeometry->GetZStart() + ( FLOAT ) j * m_pCurrentGeometry->GetStepSizeZ() ;

				pSector		= g_pcsApmMap->GetSector( fX , fZ );
				if( pSector )
					pSegment	= pSector->D_GetSegment( fX , fZ );
				else
					pSegment	= NULL;

				height = m_pCurrentGeometry->D_GetHeight( SECTOR_HIGHDETAIL , fX , fZ );

				RwIm3DVertexSetPos	( &pLineList[ i + j * ( nDepth + 1 ) ] , 
										fX,
										height + ALEF_SECTOR_WIDTH / 500.0f,
										fZ);

				RwIm3DVertexSetU	( &pLineList[ i + j * ( nDepth + 1 ) ] , 1.0f );    
				RwIm3DVertexSetV	( &pLineList[ i + j * ( nDepth + 1 ) ] , 1.0f );
				RwIm3DVertexSetRGBA	( &pLineList[ i + j * ( nDepth + 1 ) ] , r , g , b , a );

				if( bShowPolygonSplit )
				{
					// ¸¶°íÀÚ (2004-06-16 ¿ÀÀü 10:30:50) :  Edge Turn Ç¥½Ã..
					pLineList2[ i + j * ( nDepth + 1 ) ] = pLineList[ i + j * ( nDepth + 1 ) ];
					RwIm3DVertexSetRGBA	( &pLineList2[ i + j * ( nDepth + 1 ) ] , 220 , 132 , 132 , 255 );
				}

				// ¸¶°íÀÚ (2004-06-15 ¿ÀÈÄ 3:15:54) : ÀÎµ¦½º Ã³¸®..

				if( i < nDepth && j < nDepth )
				{
					pIndex[ ( i + j * ( nDepth ) ) * 4 * 2 + 0	] = ( i		) +	( j		) * ( nDepth + 1 );
					pIndex[ ( i + j * ( nDepth ) ) * 4 * 2 + 1	] = ( i + 1	) +	( j		) * ( nDepth + 1 );

					pIndex[ ( i + j * ( nDepth ) ) * 4 * 2 + 2	] = ( i	+ 1	) +	( j		) * ( nDepth + 1 );
					pIndex[ ( i + j * ( nDepth ) ) * 4 * 2 + 3	] = ( i	+ 1	) +	( j + 1	) * ( nDepth + 1 );

					pIndex[ ( i + j * ( nDepth ) ) * 4 * 2 + 4	] = ( i	+ 1	) +	( j + 1	) * ( nDepth + 1 );
					pIndex[ ( i + j * ( nDepth ) ) * 4 * 2 + 5	] = ( i		) +	( j	+ 1	) * ( nDepth + 1 );

					pIndex[ ( i + j * ( nDepth ) ) * 4 * 2 + 6	] = ( i		) +	( j	+ 1	) * ( nDepth + 1 );
					pIndex[ ( i + j * ( nDepth ) ) * 4 * 2 + 7	] = ( i		) +	( j		) * ( nDepth + 1 );

					if( bShowPolygonSplit )
					{
						// ¸¶°íÀÚ (2004-06-16 ¿ÀÀü 10:31:39) : ¿§ÁöÅÏ Ç¥½Ã..
						if( pSegment && pSegment->stTileInfo.GetEdgeTurn() )
						{
							pIndex2[ ( i + j * ( nDepth ) ) * 2 + 0	] = ( i		) +	( j		) * ( nDepth + 1 );
							pIndex2[ ( i + j * ( nDepth ) ) * 2 + 1	] = ( i	+ 1	) +	( j	+ 1	) * ( nDepth + 1 );
						}
						else
						{
							pIndex2[ ( i + j * ( nDepth ) ) * 2 + 0	] = ( i	+ 1	) +	( j		) * ( nDepth + 1 );
							pIndex2[ ( i + j * ( nDepth ) ) * 2 + 1	] = ( i		) +	( j	+ 1	) * ( nDepth + 1 );
						}
					}
				}
			}
		}

		if( RwIm3DTransform( pLineList , ( nDepth + 1 ) * ( nDepth + 1 ) , NULL, rwIM3D_ALLOPAQUE ) )
		{                         
			RwIm3DRenderIndexedPrimitive( rwPRIMTYPELINELIST,
				pIndex , ( nDepth ) * ( nDepth ) * 4 * 2 );

			RwIm3DEnd();
		}

		if( bShowPolygonSplit && RwIm3DTransform( pLineList2 , ( nDepth + 1 ) * ( nDepth + 1 ) , NULL, rwIM3D_ALLOPAQUE ) )
		{                         
			RwIm3DRenderIndexedPrimitive( rwPRIMTYPELINELIST,
				pIndex2 , ( nDepth ) * ( nDepth ) * 2 );

			RwIm3DEnd();
		}
	}
}
void		MainWindow::DrawSectorGrid				()
{
	AGCMMAP_THIS->EnumLoadedDivision( __DivisionRenderSectorGridCallback , ( PVOID ) this );
}

void		MainWindow::DrawWireFrame	( INT32 nType )
{
	switch( nType )
	{
	case GRIDMODE_SECTOR_GRID		:
		{
			DrawSectorGrid();
		}
		break;
	case GRIDMODE_D_GETHEIGHT		:
		{
			DrawHeightGrid();
		}
		break;
	case GRIDMODE_HEIGHT_SECTOR		:
		{
			DrawSectorGrid();
			DrawHeightGrid();
		}
		break;
	default:
		break;
	}
}

BOOL	__DivisionRenderSectorGridCallback ( DivisionInfo * pDivisionInfo , PVOID pData )
{
	ApWorldSector * pSector;
	int x , z;
	for( z = pDivisionInfo->nZ	; z < pDivisionInfo->nZ + pDivisionInfo->nDepth ; ++ z )
	{
		for( x = pDivisionInfo->nX	; x < pDivisionInfo->nX + pDivisionInfo->nDepth ; ++ x )
		{
			pSector	= g_pcsApmMap->GetSectorByArray( x , 0 , z );

			if( NULL == pSector ) continue;

			AGCMMAP_THIS->RenderSectorGrid( pSector );
		}
	}

	return TRUE;
}

void		MainWindow::DetailChange	(int detail	)
{
	// µðÅ×ÀÏº¯È­..
//	if( m_pCurrentGeometry )
//	{
//		m_pCurrentGeometry->SetCurrentDetail( detail );
//		//RpDWSectorSetCurrentDetail( AGCMMAP_THIS->GetWorldSector( m_pCurrentGeometry )  , detail );
//		g_pMainFrame->PostMessage( WM_TOOLBAR_DETAILCHANGE , DETAIL_SET , detail );
//	}

	AGCMMAP_THIS->SetCurrentDetail( detail );
}

void		MainWindow::DrawGeometrySelection()
{
	// ÁöÇü ¼¿·ºÆ®¸¦ Ç¥½ÃÇØÁÜ..
	int	vertexcount	= 0			;
	int indexcount	= 0			;
	int	offsettileSelecting	= -1;
	int	offsetGeometry		= -1;

	int	indexoffsetselecting= -1;
	int	indexoffsetgeometry	= -1;
	int	segmentcount			;

	int width					;
	int	height					;

	int	nSector_LeftTop_X		;
	int	nSector_LeftTop_Y		;

	int	nSector_Rightbottom_X	;
	int	nSector_Rightbottom_Y	;

	int	nPosX1 , nPosY1			;
	int	nPosX2 , nPosY2			;

	rsDWSectorInfo	dwSectorInfo	;
	ApWorldSector *	pSector			;

	int	nSectorX , nSectorY			;
	int	nX1 , nX2 , nY1 , nY2		;		

	
	segmentcount = m_listTileSelect.GetCount();

	if( segmentcount > 0 )
	{
		//Å¸ÀÏ °³°³ ¼±ÅÂ¾×..
		

		offsettileSelecting		=	vertexcount			;
		vertexcount				+=	segmentcount * 4	;

		indexoffsetselecting	=	indexcount			;
		indexcount				+=	segmentcount * 8	;
	}

	// ÁöÇüºí·¯¾ï
	if( m_SelectedPosX1 >= 0 && m_SelectedPosY1 >= 0 && m_pSelectedGeometry1 &&
		m_SelectedPosX2 >= 0 && m_SelectedPosY2 >= 0 && m_pSelectedGeometry2 )
	{
		// ÀÚ ±×¸®ÀÚ..
		offsetGeometry		=	vertexcount			;
		indexoffsetgeometry	=	indexcount			;

		////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////

		if( m_pSelectedGeometry1->GetArrayIndexX() < m_pSelectedGeometry2->GetArrayIndexX() )
		{
			nSector_LeftTop_X		= m_pSelectedGeometry1->GetArrayIndexX()	;
			nSector_Rightbottom_X	= m_pSelectedGeometry2->GetArrayIndexX()	;

			nPosX1					= m_SelectedPosX1							;
			nPosX2					= m_SelectedPosX2							;
		}
		else
		{
			nSector_LeftTop_X		= m_pSelectedGeometry2->GetArrayIndexX()	;
			nSector_Rightbottom_X	= m_pSelectedGeometry1->GetArrayIndexX()	;

			nPosX1					= m_SelectedPosX2							;
			nPosX2					= m_SelectedPosX1							;
		}

		if( m_pSelectedGeometry1->GetArrayIndexZ() < m_pSelectedGeometry2->GetArrayIndexZ() )
		{
			nSector_LeftTop_Y		= m_pSelectedGeometry1->GetArrayIndexZ()	;
			nSector_Rightbottom_Y	= m_pSelectedGeometry2->GetArrayIndexZ()	;

			nPosY1					= m_SelectedPosY1							;
			nPosY2					= m_SelectedPosY2							;
		}
		else
		{
			nSector_LeftTop_Y		= m_pSelectedGeometry2->GetArrayIndexZ()	;
			nSector_Rightbottom_Y	= m_pSelectedGeometry1->GetArrayIndexZ()	;

			// ½º¿Ò~...;
			nPosY1					= m_SelectedPosY2							;
			nPosY2					= m_SelectedPosY1							;
		}

		// ¿ùµå ¼½ÅÍ Æ®·¡¹ú½º.
		for( nSectorY = nSector_LeftTop_Y ; nSectorY <= nSector_Rightbottom_Y ; nSectorY ++ )
		{
			for( nSectorX = nSector_LeftTop_X ; nSectorX <= nSector_Rightbottom_X ; nSectorX ++ )
			{
				// ¿ì¼± ¼½ÅÍ Æ÷ÀÎÅÍ¸¦ ¾ò¾î³¿..
				pSector	= g_pcsApmMap->GetSectorByArray( nSectorX , 0 , nSectorY );

				if( NULL == pSector ) continue;
				
				// ¿©±â¼­.. Àû¿ëÇÒ ¹üÀ§¸¦ ±¸ÇÑ´Ù..
				if( nSectorX == nSector_LeftTop_X		&&
					nSectorX == nSector_Rightbottom_X	)
				{
					if( nPosX1 < nPosX2 )	{ nX1 = nPosX1 ; nX2 = nPosX2; }
					else					{ nX1 = nPosX2 ; nX2 = nPosX1; }
				}
				else
				{
					if( nSectorX == nSector_LeftTop_X		)	nX1		= nPosX1				;
					else										nX1		= 0						;
					if( nSectorX == nSector_Rightbottom_X	)	nX2		= nPosX2				;
					else										nX2		= pSector->D_GetDepth() -1;
				}

				// ¿©±â¼­.. Àû¿ëÇÒ ¹üÀ§¸¦ ±¸ÇÑ´Ù..
				if( nSectorY == nSector_LeftTop_Y		&&
					nSectorY == nSector_Rightbottom_Y	)
				{
					if( nPosY1 < nPosY2 )	{ nY1 = nPosY1 ; nY2 = nPosY2; }
					else					{ nY1 = nPosY2 ; nY2 = nPosY1; }
				}
				else
				{
					if( nSectorY == nSector_LeftTop_Y		)	nY1		= nPosY1				;
					else										nY1		= 0						;

					if( nSectorY == nSector_Rightbottom_Y	)	nY2		= nPosY2				;
					else										nY2		= pSector->D_GetDepth() -1;
				}

				width		= ( nX2 - nX1 + 1 )					;
				height		= ( nY2 - nY1 + 1 )					;
				
				// ¹öÅØ½º ¹öÆÛ¸¦ ÁØºñÇÔ...
				vertexcount	+=	( width + 1 ) * ( height + 1 )	;
				indexcount	+=	width * height * 4 * 2			;
			}
		}
		/// ·çÇÁ Á¾·á..
	}

	// ±×¸±°Ô ¾øÀ¸¸é ÇÔ¼ö Á¾·á..
	if( vertexcount <= 0 ) return;

	// ¹öÆÛ ÁØºñ..
	RwIm3DVertex *		pLineList	= new RwIm3DVertex		[ vertexcount	];
	RwImVertexIndex	*	pIndex		= new RwImVertexIndex	[ indexcount	];

	if( offsettileSelecting != -1 )
	{
		// °¢°¢ ¼±ÅÃÇÑ°Å ±×¸®±â..
		AuNode< _TileSelectStruct > * pNode = m_listTileSelect.GetHeadNode();
		_TileSelectStruct	* pTile;
		ApDetailSegment	* pSegment	;
		float	fX , fZ		;
		float	height		;
		float	stepsize	;

		int		count = 0;

		while( pNode )
		{
			pTile = & pNode->GetData();

			pSegment = pTile->pSector->D_GetSegment( pTile->x , pTile->z , &fX , &fZ );

			if( pSegment )
				height = pSegment->height				+ ALEF_SECTOR_WIDTH / ALEF_MAX_HORI_SECTOR / 30;
			else
				height = ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT	+ ALEF_SECTOR_WIDTH / ALEF_MAX_HORI_SECTOR / 30;

			stepsize = pTile->pSector->GetStepSizeX();

			RwIm3DVertexSetPos	( &pLineList[ offsettileSelecting + count ] , fX , height , fZ	);
			RwIm3DVertexSetU	( &pLineList[ offsettileSelecting + count ] , 1.0f				);    
			RwIm3DVertexSetV	( &pLineList[ offsettileSelecting + count ] , 1.0f				);
			RwIm3DVertexSetRGBA	( &pLineList[ offsettileSelecting + count ] , 0 , 255 , 0 , 255 );
			count ++;

			height = pTile->pSector->D_GetHeight( fX + stepsize , fZ ) + ALEF_SECTOR_WIDTH / ALEF_MAX_HORI_SECTOR / 30;
			RwIm3DVertexSetPos	( &pLineList[ offsettileSelecting + count ] , fX + stepsize , height , fZ	);
			RwIm3DVertexSetU	( &pLineList[ offsettileSelecting + count ] , 1.0f				);    
			RwIm3DVertexSetV	( &pLineList[ offsettileSelecting + count ] , 1.0f				);
			RwIm3DVertexSetRGBA	( &pLineList[ offsettileSelecting + count ] , 0 , 255 , 0 , 255 );
			count ++;

			height = pTile->pSector->D_GetHeight( fX , fZ + stepsize ) + ALEF_SECTOR_WIDTH / ALEF_MAX_HORI_SECTOR / 30;
			RwIm3DVertexSetPos	( &pLineList[ offsettileSelecting + count ] , fX , height , fZ + stepsize );
			RwIm3DVertexSetU	( &pLineList[ offsettileSelecting + count ] , 1.0f				);    
			RwIm3DVertexSetV	( &pLineList[ offsettileSelecting + count ] , 1.0f				);
			RwIm3DVertexSetRGBA	( &pLineList[ offsettileSelecting + count ] , 0 , 255 , 0 , 255 );
			count ++;

			height = pTile->pSector->D_GetHeight( fX + stepsize , fZ + stepsize ) + ALEF_SECTOR_WIDTH / ALEF_MAX_HORI_SECTOR / 30;
			RwIm3DVertexSetPos	( &pLineList[ offsettileSelecting + count ] , fX + stepsize , height , fZ + stepsize );
			RwIm3DVertexSetU	( &pLineList[ offsettileSelecting + count ] , 1.0f				);    
			RwIm3DVertexSetV	( &pLineList[ offsettileSelecting + count ] , 1.0f				);
			RwIm3DVertexSetRGBA	( &pLineList[ offsettileSelecting + count ] , 0 , 255 , 0 , 255 );
			count ++;
			
			m_listTileSelect.GetNext( pNode );
		}

		for( int i = 0 , j = 0 ; i < segmentcount ; i ++ , j += 8 )
		{
			pIndex[ indexoffsetselecting + j + 0	] = offsettileSelecting + i * 4	+		0;
			pIndex[ indexoffsetselecting + j + 1	] = offsettileSelecting + i * 4	+		1;

			pIndex[ indexoffsetselecting + j + 2	] = offsettileSelecting + i * 4	+		1;
			pIndex[ indexoffsetselecting + j + 3	] = offsettileSelecting + i * 4	+		2;

			pIndex[ indexoffsetselecting + j + 4	] = offsettileSelecting + i * 4	+		2;
			pIndex[ indexoffsetselecting + j + 5	] = offsettileSelecting + i * 4	+		3;

			pIndex[ indexoffsetselecting + j + 6	] = offsettileSelecting + i * 4	+		3;
			pIndex[ indexoffsetselecting + j + 7	] = offsettileSelecting + i * 4	+		0;
		}
	}

	if( offsetGeometry != -1 )
	{
		// LeftTop , RightBottom À» ±¸ÇßÀ¸´Ï±î...
		// ÀÌÁ¦ RwIm3D·Î ¹Ú½º¸¦ ±×·ÁÁØ´Ù.

		ApDetailSegment * pSegment;
		float	fX , fZ;
		int		i , j;

		// ¿ùµå ¼½ÅÍ Æ®·¡¹ú½º.
		for( nSectorY = nSector_LeftTop_Y ; nSectorY <= nSector_Rightbottom_Y ; nSectorY ++ )
		{
			for( nSectorX = nSector_LeftTop_X ; nSectorX <= nSector_Rightbottom_X ; nSectorX ++ )
			{
				// ¿ì¼± ¼½ÅÍ Æ÷ÀÎÅÍ¸¦ ¾ò¾î³¿..
				pSector	= g_pcsApmMap->GetSectorByArray( nSectorX , 0 , nSectorY );

				if( NULL == pSector ) continue;
				// ¿©±â¼­.. Àû¿ëÇÒ ¹üÀ§¸¦ ±¸ÇÑ´Ù..
				if( nSectorX == nSector_LeftTop_X		&&
					nSectorX == nSector_Rightbottom_X	)
				{
					if( nPosX1 < nPosX2 )	{ nX1 = nPosX1 ; nX2 = nPosX2; }
					else					{ nX1 = nPosX2 ; nX2 = nPosX1; }
				}
				else
				{
					if( nSectorX == nSector_LeftTop_X		)	nX1		= nPosX1				;
					else										nX1		= 0						;
					if( nSectorX == nSector_Rightbottom_X	)	nX2		= nPosX2				;
					else										nX2		= pSector->D_GetDepth() -1;
				}

				// ¿©±â¼­.. Àû¿ëÇÒ ¹üÀ§¸¦ ±¸ÇÑ´Ù..
				if( nSectorY == nSector_LeftTop_Y		&&
					nSectorY == nSector_Rightbottom_Y	)
				{
					if( nPosY1 < nPosY2 )	{ nY1 = nPosY1 ; nY2 = nPosY2; }
					else					{ nY1 = nPosY2 ; nY2 = nPosY1; }
				}
				else
				{
					if( nSectorY == nSector_LeftTop_Y		)	nY1		= nPosY1				;
					else										nY1		= 0						;

					if( nSectorY == nSector_Rightbottom_Y	)	nY2		= nPosY2				;
					else										nY2		= pSector->D_GetDepth() -1;
				}

				// ¼½ÅÍ Á¤º¸¸¦ ¾ò¾î³»°í..
				AGCMMAP_THIS->GetDWSector( pSector ,  & dwSectorInfo );
				
				width		= ( nX2 - nX1 + 1 )					;
				height		= ( nY2 - nY1 + 1 )					;
				
				for( j = 0 ; j < height + 1 ; j ++ )
				{
					for( i = 0 ; i < width + 1 ; i ++ )
					{
						pSegment = pSector->D_GetSegment( nX1 + i , nY1 + j , &fX , &fZ );
						RwIm3DVertexSetPos	( &pLineList[ offsetGeometry + i + j * ( width + 1 ) ] , 
							fX,
							pSector->D_GetHeight( fX , fZ ) + 3.0f * ALEF_SECTOR_WIDTH / 1000.0f,
							fZ);

						RwIm3DVertexSetU	( &pLineList[ offsetGeometry + i + j * ( width + 1 ) ] , 1.0f );    
						RwIm3DVertexSetV	( &pLineList[ offsetGeometry + i + j * ( width + 1 ) ] , 1.0f );
						RwIm3DVertexSetRGBA	( &pLineList[ offsetGeometry + i + j * ( width + 1 ) ] , 255 , 0 , 0 , 255 );
					}
				}

				for( j = 0 ; j < height ; j ++ )
				{
					for( i = 0 ; i < width ; i ++ )
					{
						pIndex[ indexoffsetgeometry + ( i + j * ( width ) ) * 4 * 2 + 0	] = offsetGeometry +	( i		) +	( j		) * ( width + 1 );
						pIndex[ indexoffsetgeometry + ( i + j * ( width ) ) * 4 * 2 + 1	] = offsetGeometry +	( i + 1	) +	( j		) * ( width + 1 );

						pIndex[ indexoffsetgeometry + ( i + j * ( width ) ) * 4 * 2 + 2	] = offsetGeometry +	( i	+ 1	) +	( j		) * ( width + 1 );
						pIndex[ indexoffsetgeometry + ( i + j * ( width ) ) * 4 * 2 + 3	] = offsetGeometry +	( i	+ 1	) +	( j + 1	) * ( width + 1 );

						pIndex[ indexoffsetgeometry + ( i + j * ( width ) ) * 4 * 2 + 4	] = offsetGeometry +	( i	+ 1	) +	( j + 1	) * ( width + 1 );
						pIndex[ indexoffsetgeometry + ( i + j * ( width ) ) * 4 * 2 + 5	] = offsetGeometry +	( i		) +	( j	+ 1	) * ( width + 1 );

						pIndex[ indexoffsetgeometry + ( i + j * ( width ) ) * 4 * 2 + 6	] = offsetGeometry +	( i		) +	( j	+ 1	) * ( width + 1 );
						pIndex[ indexoffsetgeometry + ( i + j * ( width ) ) * 4 * 2 + 7	] = offsetGeometry +	( i		) +	( j		) * ( width + 1 );
					}
				}
				
				// ¿É¼Â ¸¸Å­ ³Ñ±è..
				offsetGeometry		+=	( width + 1 ) * ( height + 1 )	;
				indexoffsetgeometry	+=	width * height * 4 * 2			;
			}
		}

		// ¿ùµå ¼½ÅÍ Æ®·¹¹ú½º
				
		///////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////
	}

	if( RwIm3DTransform( pLineList , vertexcount , NULL, rwIM3D_ALLOPAQUE ) )
	{                         
		RwIm3DRenderIndexedPrimitive( rwPRIMTYPELINELIST,
			pIndex , indexcount );

		RwIm3DEnd();
	}
	delete [] pIndex	;
	delete [] pLineList	;
}

BOOL MainWindow::ChangeSegmentMaterial(stFindMapInfo *pMapInfo, int posx, int posz, int maskedindex , int alphaindex , int back)
{
//	static UINT8 aunEnv[27] = {0xff, 0x68, 0xd0, 0x16, 0x0b, 0x68, 0xd0, 0x16, 0x0b, 0xf8, 0xd6, 0x1f, 0x6b
//		, 0xdb,	0x7d, 0xfb, 0xfe, 0xdf, 0x7f, 0xfb, 0xfe, 0xdf, 0x7f, 0xd6, 0xf8, 0x1f, 0x6b};
//	static int offsetX[9] = {-1, 0, 1, -1, 1, -1, 0, 1, 0};
//	static int offsetY[9] = {-1, -1, -1, 0, 0, 1, 1, 1, 0};
//
//	int i;
//	int j;
//	int k;
//	int posx2, posy2;
//
//	stFindMapInfo FindMapSelf;
//	stFindMapInfo FindMapEnv;
//	UINT8 unEnvSelf;
//	UINT8 mask;
//	int match;
//	int matchmax;
//	int matchmask;
//	int tileindex2;
//	int materialindex;
//
//	if (back)
//	{
//		if (pMapInfo->pSegment->pIndex[ TD_FIRST ] == maskedindex	)
//			return TRUE;
//	}
//	else
//	{
//		if (pMapInfo->pSegment->pIndex[ TD_THIRD ] == maskedindex	)
//			return TRUE;
//
//		else
//
//		if (pMapInfo->pSegment->pIndex[ TD_FIRST ] == maskedindex	)
//			return TRUE;
//	}
//
//	materialindex = m_MaterialList.GetMatIndex(
//		
//		back ? maskedindex				:	pMapInfo->pSegment->pIndex[ TD_FIRST ]	,
//		back ? ALEF_TILE_DO_NOT_USE		:	alphaindex									,
//		back ? ALEF_TILE_DO_NOT_USE		:	maskedindex									) ;
//
//	if( materialindex != -1 )
//	{
//		// ¸Êµ¥ÀÌÅ¸¿¡µµ ÀúÀåÇÔ.
//		if (!back)
//		{
//			// Undo Log..
////			m_UndoManager.AddTileActionUnit(pMapInfo->pSector ,
////				pMapInfo->nPolygonIndexFirst , pMapInfo->nPolygonIndexFirst + 1 ,
////				m_pFoundSector->polygons[ pMapInfo->nPolygonIndexFirst		].matIndex , materialindex ,
////				&pMapInfo->pSegment->tileindex , pMapInfo->pSegment->tileindex , pMapInfo->pSegment->tileindex ,
////				&pMapInfo->pSegment->tileindex2, pMapInfo->pSegment->tileindex2 , maskedindex );
//
//			pMapInfo->pSegment->pIndex[ TD_SECOND	]	= alphaindex	;
//			pMapInfo->pSegment->pIndex[ TD_THIRD	]	= maskedindex	;
//		}
//		else
//		{
//			// Undo Log..
////			m_UndoManager.AddTileActionUnit( pMapInfo->pSector ,
////				pMapInfo->nPolygonIndexFirst , pMapInfo->nPolygonIndexFirst + 1 ,
////				m_pFoundSector->polygons[ pMapInfo->nPolygonIndexFirst		].matIndex , materialindex ,
////				&pMapInfo->pSegment->tileindex , pMapInfo->pSegment->tileindex , maskedindex ,
////				&pMapInfo->pSegment->tileindex2, pMapInfo->pSegment->tileindex2 , ALEF_TILE_DO_NOT_USE );
//
//			pMapInfo->pSegment->pIndex[ TD_FIRST	]	= maskedindex			;
//			pMapInfo->pSegment->pIndex[ TD_SECOND	]	= ALEF_TILE_DO_NOT_USE	;
//			pMapInfo->pSegment->pIndex[ TD_THIRD	]	= ALEF_TILE_DO_NOT_USE	;
//		}
//
//		m_pFoundSector->polygons[ pMapInfo->nPolygonIndexFirst		].matIndex		= materialindex;
//		m_pFoundSector->polygons[ pMapInfo->nPolygonIndexFirst + 1	].matIndex		= materialindex;
//	}
//	else
//	{
//		// ¿¡·¯..
//		MTRACE( "¾ËÆÄ ÅØ½ºÃÄ ±î´Âµ¥ ½ÇÆÐÇß½À´Ï´Ù\n" );
//	}
//
//	for (i = 0; i < 9; i++)
//	{
//	javascript:window.close()	posx2 = posx + offsetX[i];
//		posy2 = posz + offsetY[i];
//
//		if (GetWorldSectorInfo( &FindMapSelf , pMapInfo->pSector , posx2 , posy2))
//
//			if (
//				
//					(
//						!back
//						&&	GET_TEXTURE_TYPE	(FindMapSelf.pSegment->tileindex2) == GET_TEXTURE_TYPE	(pMapInfo->pSegment->tileindex2) 
//						&&	GET_TEXTURE_INDEX	(FindMapSelf.pSegment->tileindex2) == GET_TEXTURE_INDEX	(pMapInfo->pSegment->tileindex2)
//					)
//					||
//					back
//				)
//		{
//			unEnvSelf = 0;
//
//			for (j = 0; j < 8; j++)
//			{
//				if (GetWorldSectorInfo( &FindMapEnv , pMapInfo->pSector , posx2 + offsetX[j], posy2 + offsetY[j]))
//				{
//					if (	GET_TEXTURE_TYPE	(FindMapEnv.pSegment->tileindex) == GET_TEXTURE_TYPE	(FindMapSelf.pSegment->tileindex2) &&
//							GET_TEXTURE_INDEX	(FindMapEnv.pSegment->tileindex) == GET_TEXTURE_INDEX	(FindMapSelf.pSegment->tileindex2)	)
//					{
//						unEnvSelf |= (1 << (7 - j));
//					}
//					else
//					if (	GET_TEXTURE_TYPE	( FindMapSelf.pSegment->tileindex2	) &&
//							GET_TEXTURE_TYPE	( FindMapEnv.pSegment->tileindex2	) == GET_TEXTURE_TYPE	(FindMapSelf.pSegment->tileindex2) && 
//							GET_TEXTURE_INDEX	( FindMapEnv.pSegment->tileindex2	) == GET_TEXTURE_INDEX	(FindMapSelf.pSegment->tileindex2)	)
//					{
////						unEnvSelf |= ((aunEnv[GET_TEXTURE_MASK(FindMapEnv.pSegment->tileindex2)] >> j) & 1) << (7 - j);
//						unEnvSelf |= (1 << (7 - j));
//					}
//				}
//			}
//
//			matchmax	= 0;
//			matchmask	= 0;
//
//			for (mask = 0; mask < 26; mask++)
//			{
//				match = 0;
//
//				for (k = 0; k < 8; k++)
//				{
//					if (	(	(unEnvSelf & (1 << k)	) && (	aunEnv[mask] & (1 << k)	)	)	|| 
//							(	!(unEnvSelf & (1 << k)	) && !(	aunEnv[mask] & (1 << k)	)	)	)
//					{
//						match++;
//					}
//				}
//
//				if (match > matchmax)
//				{
//					matchmax	= match	;
//					matchmask	= mask	;
//				}
//			}
//
//			mask	= matchmask;
//
//			if (matchmax == 8 && mask == 0)
//				back = 1;
//			else
//				back = 0;
//
//			if (mask >= 26)
//				mask = 0;
//
//			tileindex2 = MAKE_TEXTURE_INDEX(
//					GET_TEXTURE_TYPE	( maskedindex )		,
//					GET_TEXTURE_INDEX	( maskedindex )		,
//					mask );
//
//			if (mask)
//			{
//				int materialindex = m_MaterialList.GetMatIndex(
//					FindMapSelf.pSegment->tileindex	,
//					tileindex2 ) ;
//
//				if( materialindex != -1 )
//				{
//					ChangeSegmentMaterial(&FindMapSelf, posx2, posy2, tileindex2);
//				}
//				else
//				{
//					// ¿¡·¯..
//					MTRACE( "¾ËÆÄ ÅØ½ºÃÄ ±î´Âµ¥ ½ÇÆÐÇß½À´Ï´Ù\n" );
//				}
//			}
//			else
//			{
//				ChangeSegmentMaterial(&FindMapSelf, posx2, posy2, tileindex2, back);
//			}
//		}
//	}

	return TRUE;
}

BOOL MainWindow::ChangeSegmentMaterial2(stFindMapInfo *pMapInfo, int posx, int posz, int maskedindex , int back)
{
//	static UINT8 aunEnv[27] = {0xff, 0x68, 0xd0, 0x16, 0x0b, 0x68, 0xd0, 0x16, 0x0b, 0xf8, 0xd6, 0x1f, 0x6b
//		, 0xdb,	0x7d, 0xfb, 0xfe, 0xdf, 0x7f, 0xfb, 0xfe, 0xdf, 0x7f, 0xd6, 0xf8, 0x1f, 0x6b};
//	static int offsetX[9] = {-1, 0, 1, -1, 1, -1, 0, 1, 0};
//	static int offsetY[9] = {-1, -1, -1, 0, 0, 1, 1, 1, 0};
//	static int offsetX2[9] = {-1,  0,  1,  0, -1,  1, -1,  1, 0};
//	static int offsetY2[9] = { 0,  1,  0, -1, -1,  1,  1, -1, 0};
//
//	int i2;
//	int i;
//	int j;
//	int k;
//	int posx2, posy2;
//
//	stFindMapInfo FindMapSelf;
//	stFindMapInfo FindMapEnv;
//	UINT8 unEnvSelf;
//	UINT8 mask;
//	int match;
//	int matchmax;
//	int matchexist;
//	int matchexistmax;
//	int matchmask;
//	int matIndex2;
//	int materialindex;
//	int back2;
//
//	if (back)
//	{
//		if (pMapInfo->pSegment->tileindex == maskedindex)
//			return TRUE;
//	}
//	else
//	{
//		if (pMapInfo->pSegment->tileindex2 == maskedindex)
//			return TRUE;
//
//		if (GET_TEXTURE_TYPE	(pMapInfo->pSegment->tileindex) == GET_TEXTURE_TYPE	(maskedindex) &&
//			GET_TEXTURE_INDEX	(pMapInfo->pSegment->tileindex) == GET_TEXTURE_INDEX(maskedindex)	)
//			return TRUE;
//	}
//
//	materialindex = m_MaterialList.GetMatIndex(
//		back ? maskedindex				:	pMapInfo->pSegment->tileindex	,
//		back ? ALEF_TILE_DO_NOT_USE		:	maskedindex							) ;
//
//	if( materialindex != -1 )
//	{
//		// ¸Êµ¥ÀÌÅ¸¿¡µµ ÀúÀåÇÔ.
//		if (!back)
//		{
//			m_UndoManager.AddTileActionUnit( pMapInfo->pSector ,
//				pMapInfo->nPolygonIndexFirst , pMapInfo->nPolygonIndexFirst + 1 ,
//				pMapInfo->pSector->m_pWorldSector->polygons[ pMapInfo->nPolygonIndexFirst		].matIndex , materialindex ,
//				&pMapInfo->pSegment->tileindex , pMapInfo->pSegment->tileindex , pMapInfo->pSegment->tileindex ,
//				&pMapInfo->pSegment->tileindex2, pMapInfo->pSegment->tileindex2 , maskedindex );
//
//			pMapInfo->pSegment->tileindex2	= maskedindex;
//		}
//		else
//		{
//			m_UndoManager.AddTileActionUnit( pMapInfo->pSector ,
//				pMapInfo->nPolygonIndexFirst , pMapInfo->nPolygonIndexFirst + 1 ,
//				pMapInfo->pSector->m_pWorldSector->polygons[ pMapInfo->nPolygonIndexFirst		].matIndex , materialindex ,
//				&pMapInfo->pSegment->tileindex , pMapInfo->pSegment->tileindex , maskedindex ,
//				&pMapInfo->pSegment->tileindex2, pMapInfo->pSegment->tileindex2 , ALEF_TILE_DO_NOT_USE );
//
//			pMapInfo->pSegment->tileindex = maskedindex;
//			pMapInfo->pSegment->tileindex2	= ALEF_TILE_DO_NOT_USE;
//		}
//
//		pMapInfo->pSector->m_pWorldSector->polygons[ pMapInfo->nPolygonIndexFirst		].matIndex		= materialindex;
//		pMapInfo->pSector->m_pWorldSector->polygons[ pMapInfo->nPolygonIndexFirst + 1	].matIndex		= materialindex;
//	}
//	else
//	{
//		// ¿¡·¯..
//		MTRACE( "¾ËÆÄ ÅØ½ºÃÄ ±î´Âµ¥ ½ÇÆÐÇß½À´Ï´Ù\n" );
//	}
//
//	for (i2 = 0; i2 < 2; i2++)
//	{
//		for (i = 0; i < 9; i++)
//		{
//			posx2 = posx + offsetX2[i];
//			posy2 = posz + offsetY2[i];
//
//			if (GetWorldSectorInfo( &FindMapSelf , pMapInfo->pSector , posx2 , posy2))
//			{
//				unEnvSelf = 0;
//
//				for (j = 0; j < 8; j++)
//				{
//					if (GetWorldSectorInfo( &FindMapEnv , pMapInfo->pSector , posx2 + offsetX[j], posy2 + offsetY[j]))
//					{
//						if (GET_TEXTURE_TYPE(FindMapEnv.pSegment->tileindex2) == GET_TEXTURE_TYPE(maskedindex))
//							unEnvSelf |= ((aunEnv[GET_TEXTURE_MASK(FindMapEnv.pSegment->tileindex2)] >> j) & 1) << (7 - j);
//						else if (GET_TEXTURE_TYPE(FindMapEnv.pSegment->tileindex) == GET_TEXTURE_TYPE(maskedindex))
//							unEnvSelf |= (1 << (7 - j));
//					}
//				}
//
//				if (unEnvSelf == 0)
//					break;
//
//				matchexistmax = 0;
//				matchmax = 0;
//				matchmask = 0;
//				for (mask = 0; mask < 27; mask++)
//				{
//					matchexist = 0;
//					match = 0;
//
//					for (k = 0; k < 8; k++)
//					{
//						if ((unEnvSelf & (1 << k)) && (aunEnv[mask] & (1 << k)))
//						{
//							match++;
//							matchexist++;
//						}
//						if (!(unEnvSelf & (1 << k)) && !(aunEnv[mask] & (1 << k)))
//						{
//							match++;
//						}
//					}
//
//					if (match > matchmax || (match == matchmax && matchexist > matchexistmax) )
//					{
//						matchmax = match;
//						matchmask = mask;
//
//						matchexistmax = matchexist;
//					}
//				}
//
//				mask = matchmask;
//
//				if (matchmax == 8 && mask == 0)
//					back2 = 1;
//				else
//					back2 = 0;
//
//				matIndex2 = MAKE_TEXTURE_INDEX(
//						GET_TEXTURE_TYPE	( maskedindex )		,
//						GET_TEXTURE_INDEX	( maskedindex )		,
//						mask );
//
//				if (mask)
//				{
//					int materialindex = m_MaterialList.GetMatIndex(
//						FindMapSelf.pSegment->tileindex	,
//						matIndex2 ) ;
//
//					if( materialindex != -1 )
//					{
//							ChangeSegmentMaterial2(&FindMapSelf, posx2, posy2, matIndex2);
//					}
//					else
//					{
//						// ¿¡·¯..
//						MTRACE( "¾ËÆÄ ÅØ½ºÃÄ ±î´Âµ¥ ½ÇÆÐÇß½À´Ï´Ù\n" );
//					}
//				}
//				else
//				{
//					ChangeSegmentMaterial2(&FindMapSelf, posx2, posy2, matIndex2, back2);
//				}
//			}
//		}
//	}
//
	return TRUE;
}

void MainWindow::OnBrushUpdate()
{
	switch( g_pMainFrame->m_Document.m_nBrushType )
	{
	case BRUSHTYPE_GEOMETRY:
	case BRUSHTYPE_SMOOTHER:
		// Áö±ÝÀº °°ÀÌ,..;
		//if( m_pGlyph ) RpGlyphSetRadius( m_pGlyph , g_pMainFrame->m_Document.m_fBrushRadius );
		break;
	}
}

void MainWindow::GeometryIdleAdjustment()
{
	RwV3d pos = m_Position;

	PROFILE("MainWindow::GeometryIdleAdjustment");
	// ÁöÇü ¿Ã¶ó¿À´Â°Í Ã³¸®..
	// Shift ¸¦ ´©¸¦°æ¿ì.. 


	UINT	currenttick	= GetTickCount	();
	UINT	pasttick	= currenttick - m_uLastGeometryPressTime ;//g_pEngine->GetPastTime	();
	m_uLastGeometryPressTime	= currenttick;

	float		adjustrate	;
	BOOL		bShift		;

	// Shift »óÅÂ Á¶»ç..
	short state = GetAsyncKeyState( VK_CONTROL );
	BOOL	bControlState	= FALSE;
	if( state < 0 )
	{
		adjustrate		= -ALEF_SECTOR_WIDTH / 10.0f *	( ( float ) pasttick / 1000.0f );
		bControlState	= TRUE;
	}
	else
	{
		adjustrate		= ALEF_SECTOR_WIDTH / 10.0f *	( ( float ) pasttick / 1000.0f );
		bControlState	= FALSE;	
	}

	// ¼½ÅÍ ¾îÀú½ºÆ®..
	if( m_bGeometryAdjust && m_pCurrentGeometry )
	{
		// TRACE( "%.2f,%.2f ÁÂÇ¥\n" , pos.x , pos.z );

		switch( g_pMainFrame->m_Document.m_nBrushType )
		{
		case BRUSHTYPE_GEOMETRY:
			{
				// ÁÂÇ¥¿¡¼­ r ¹üÀ§ ¾È¿¡ Á¸ÀçÇÏ´Â ¼½ÅÍ¸¦ Ã£¾Æ³½´Ù..
				state = GetAsyncKeyState( VK_SHIFT );
				if( state < 0 )
				{
					adjustrate	*= 3.0f;
					bShift		=	TRUE;
				}
				else
				{
					bShift		=	FALSE;
				}

				int				i , j				;
				float			applyheight			;
				float			fSegPosX, fSegPosY	;
				ApWorldSector *	pWorkingSector		;
				stFindMapInfo	FindMapInfo			;
				float			distance			;
				ApDetailSegment	* pSegment	;
				rsDWSectorInfo	dwSectorInfo		;

				BOOL			bFound = FALSE		;

				int segmentx , segmentz;

				int x1 = ALEF_LOAD_RANGE_X1 , x2 = ALEF_LOAD_RANGE_X2 , z1 = ALEF_LOAD_RANGE_Y1 , z2 = ALEF_LOAD_RANGE_Y2;
				int	xc , zc;

				xc = m_pCurrentGeometry->GetArrayIndexX();
				zc = m_pCurrentGeometry->GetArrayIndexZ();

				x1 = xc - 1 ;
				z1 = zc - 1 ;
				x2 = xc + 1 ;
				z2 = zc + 1 ;
		
				if( GetAsyncKeyState( VK_MENU ) < 0 )
				{
					// ÁöÇü ÆòÆòÇÏ°Ô ¸¸µé±â.

					for( j = z1 ; j <= z2 ; j ++ )
					{
						for( i = x1 ; i <= x2 ; i ++ )
						{
							// ¾ÆÀÚ Ã£¾Æº¸ÀÚ.
							pWorkingSector = g_pcsApmMap->GetSectorByArray(  i , 0 , j );
							if( NULL == pWorkingSector ) continue;

							bFound	= FALSE;

							if( !__CheckCircumstanceSectorPolygonInfo( pWorkingSector ) ) continue;

							if( pWorkingSector->IsLoadedDetailData() &&
								AGCMMAP_THIS->IsInSectorRadius( g_pcsApmMap->GetSectorByArray( i , 0 , j ) ,
								pos.x , pos.z , g_pMainFrame->m_Document.m_fBrushRadius )
								)
							{
								AGCMMAP_THIS->GetDWSector( pWorkingSector ,  & dwSectorInfo );

								//LockSector( pWorkingSector , ALEF_GEOMETRY_LOCK_MODE );

								if( NULL == dwSectorInfo.pDWSector ) continue;

								// ¾ÆÀÚ.;; 
								// ÀÌÁ¦ °¢°¢ ¼¼±×¸ÕÆ®¸¦ µÑ·¯¼­. ¿ø ¹üÀ§ ³»¿¡ µé¾îÀÖ´Â ¼¼±×¸ÕÆ®¶ó¸é Height¸¦ Àû¿ëÇÑ´Ù.
								
								for( segmentz = 0 ; segmentz < pWorkingSector->D_GetDepth() ; segmentz ++ )
								{
									for( segmentx = 0 ; segmentx < pWorkingSector->D_GetDepth() ; segmentx ++ )
									{
										pSegment = pWorkingSector->D_GetSegment( segmentx , segmentz , &fSegPosX , &fSegPosY );

										if( pSegment )
										{
											// ¹üÀ§Ã¼Å©
											if( ( distance = sqrt( ( fSegPosX - pos.x ) * ( fSegPosX - pos.x ) + ( fSegPosY - pos.z ) * ( fSegPosY - pos.z ) ) )
												> g_pMainFrame->m_Document.m_fBrushRadius ) continue;
											
											// ÀÚ ÀÌÁ¦ ¿Ã¸®ÀÚ..
											if( GetWorldSectorInfo( &FindMapInfo , pWorkingSector , segmentx , segmentz ) &&
												pWorkingSector == FindMapInfo.pSector )

											{
												bFound	= TRUE;
												SetSectorHeight( &FindMapInfo , m_fApplyheight );
											}
										}
										/////////////////////////////////////
									}
								}// ¼¼±×¸ÕÆ® ·çÇÁ..

								if( bFound ) g_pMainFrame->m_pTileList->m_pOthers->ChangeSectorGeometry( pWorkingSector );
							}
							// ¾ÆÀÚ¾ÆÀÚ.
						}
					}

					UnlockSectors( TRUE );
				}
				else
				{
					// ±×³É ÁöÇü ¿Ã¸®±â..
					for( j = z1 ; j <= z2 ; j ++ )
					{
						for( i = x1 ; i <= x2 ; i ++ )
						{
							pWorkingSector = g_pcsApmMap->GetSectorByArray(  i , 0 , j );
							if( NULL == pWorkingSector ) continue;

							bFound	= FALSE;

							if( !__CheckCircumstanceSectorPolygonInfo( pWorkingSector ) ) continue;

							// ¾ÆÀÚ Ã£¾Æº¸ÀÚ.
							if( pWorkingSector->IsLoadedDetailData() &&
								AGCMMAP_THIS->IsInSectorRadius( pWorkingSector ,
									pos.x , pos.z , g_pMainFrame->m_Document.m_fBrushRadius )
								)
							{
								// ¾ÆÀÚ.;; 
								// ÀÌÁ¦ °¢°¢ ¼¼±×¸ÕÆ®¸¦ µÑ·¯¼­. ¿ø ¹üÀ§ ³»¿¡ µé¾îÀÖ´Â ¼¼±×¸ÕÆ®¶ó¸é Height¸¦ Àû¿ëÇÑ´Ù.
								AGCMMAP_THIS->GetDWSector( pWorkingSector ,  & dwSectorInfo );
								//LockSector( pWorkingSector , rpGEOMETRYLOCKVERTICES | rpGEOMETRYLOCKNORMALS );
								//LockSector( pWorkingSector , ALEF_GEOMETRY_LOCK_MODE );


								
								for( segmentz = 0 ; segmentz < pWorkingSector->D_GetDepth() ; segmentz ++ )
								{
									for( segmentx = 0 ; segmentx < pWorkingSector->D_GetDepth() ; segmentx ++ )
									{
										pSegment = pWorkingSector->D_GetSegment( segmentx , segmentz , &fSegPosX , &fSegPosY );

										if( pSegment )
										{
											// ¹üÀ§Ã¼Å©
											if( ( distance = sqrt( ( fSegPosX - pos.x ) * ( fSegPosX - pos.x ) + ( fSegPosY - pos.z ) * ( fSegPosY - pos.z ) ) )
												> g_pMainFrame->m_Document.m_fBrushRadius ) continue;
											
											// ¿Ã¶ó°¥ ³ôÀÌ °è»ê..
											applyheight = adjustrate * ( FLOAT ) exp( ( - ( distance ) / g_pMainFrame->m_Document.m_fBrushRadius * ( distance ) / g_pMainFrame->m_Document.m_fBrushRadius / 2.0 ) * sqrt( 2 * 3.1415927 ) );

											// ÀÚ ÀÌÁ¦ ¿Ã¸®ÀÚ..
											if( GetWorldSectorInfo( &FindMapInfo , pWorkingSector , segmentx , segmentz ) &&
												pWorkingSector == FindMapInfo.pSector )
											{
												bFound = TRUE;
												SetSectorHeight( &FindMapInfo , __LimitHeight( FindMapInfo.pSegment->height + applyheight ) );
											}
										}
										/////////////////////////////////////
									}
								}

								if( bFound ) g_pMainFrame->m_pTileList->m_pOthers->ChangeSectorGeometry( pWorkingSector );
							}
							// ¾ÆÀÚ¾ÆÀÚ.
						}
					}

					UnlockSectors( TRUE );
				}

				// ¼½ÅÍ¾È¿¡¼­ ¹üÀ§¿¡ µé¾î°¡´Â Á¡µé¿¡°Ô ³ôÀÌ°ªÀ» ´õÇÑ´Ù..
			}
			break;
		case BRUSHTYPE_VERTEXCOLOR:
			{
				// ÁÂÇ¥¿¡¼­ r ¹üÀ§ ¾È¿¡ Á¸ÀçÇÏ´Â ¼½ÅÍ¸¦ Ã£¾Æ³½´Ù..
				state = GetAsyncKeyState( VK_SHIFT );
				if( state < 0 ) adjustrate *= 3.0f;

				int				i , j				;
				float			applyheight			;
				float			fSegPosX, fSegPosY	;
				ApWorldSector *	pWorkingSector		;
				stFindMapInfo	FindMapInfo			;
				float			distance			;
				ApDetailSegment	* pSegment	;
				RwRGBA			rgb					;
				rsDWSectorInfo	dwSectorInfo		;

				rgb = g_pMainFrame->GetVertexColor();

				int segmentx , segmentz;

				int x1 = ALEF_LOAD_RANGE_X1 , x2 = ALEF_LOAD_RANGE_X2 , z1 = ALEF_LOAD_RANGE_Y1 , z2 = ALEF_LOAD_RANGE_Y2;
				int	xc , zc;

				xc = m_pCurrentGeometry->GetArrayIndexX();
				zc = m_pCurrentGeometry->GetArrayIndexZ();

				x1 = xc - 1 ;
				z1 = zc - 1 ;
				x2 = xc + 1 ;
				z2 = zc + 1 ;

				for( j = z1 ; j <= z2 ; j ++ )
				{
					for( i = x1 ; i <= x2 ; i ++ )
					{
						// ¾ÆÀÚ Ã£¾Æº¸ÀÚ.
						pWorkingSector = g_pcsApmMap->GetSectorByArray(  i , 0 , j );
						if( NULL == pWorkingSector ) continue;

						if( !__CheckCircumstanceSectorPolygonInfo( pWorkingSector ) ) continue;
						
						if( pWorkingSector->IsLoadedDetailData() &&
							AGCMMAP_THIS->IsInSectorRadius( g_pcsApmMap->GetSectorByArray( i , 0 , j ) ,
							pos.x , pos.z , g_pMainFrame->m_Document.m_fBrushRadius )
							)
						{
							// ¾ÆÀÚ.;; 
							// ÀÌÁ¦ °¢°¢ ¼¼±×¸ÕÆ®¸¦ µÑ·¯¼­. ¿ø ¹üÀ§ ³»¿¡ µé¾îÀÖ´Â ¼¼±×¸ÕÆ®¶ó¸é Height¸¦ Àû¿ëÇÑ´Ù.

							AGCMMAP_THIS->GetDWSector( pWorkingSector ,  & dwSectorInfo );

							//LockSector( pWorkingSector , ALEF_VERTEXCOLOR_LOCK_MODE );
							
							for( segmentz = 0 ; segmentz < pWorkingSector->D_GetDepth() ; segmentz ++ )
							{
								for( segmentx = 0 ; segmentx < pWorkingSector->D_GetDepth() ; segmentx ++ )
								{
									pSegment = pWorkingSector->D_GetSegment( segmentx , segmentz , &fSegPosX , &fSegPosY );

									if( pSegment )
									{
										// ¹üÀ§Ã¼Å©
										if( ( distance = sqrt( ( fSegPosX - pos.x ) * ( fSegPosX - pos.x ) + ( fSegPosY - pos.z ) * ( fSegPosY - pos.z ) ) )
											> g_pMainFrame->m_Document.m_fBrushRadius ) continue;
										
										// ¿Ã¶ó°¥ ³ôÀÌ °è»ê..
										applyheight = adjustrate * ( FLOAT ) exp( ( - ( distance ) / g_pMainFrame->m_Document.m_fBrushRadius * ( distance ) / g_pMainFrame->m_Document.m_fBrushRadius / 2.0 ) * sqrt( 2 * 3.1415927 ) );

										// ÀÚ ÀÌÁ¦ ¿Ã¸®ÀÚ..
										if( GetWorldSectorInfo( &FindMapInfo , pWorkingSector , segmentx , segmentz ) )
										{
											m_UndoManager.AddVertexColorActionunit( FindMapInfo.pSector , FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ , RwRGBAToApRGBA( rgb ) );
											AGCMMAP_THIS->D_SetValue( FindMapInfo.pSector , FindMapInfo.pSector->GetCurrentDetail() , 
												FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ , RwRGBAToApRGBA( rgb ) );

										}
									}
									/////////////////////////////////////
								}
							}// ¼¼±×¸ÕÆ® ·çÇÁ

							
						}
						// ¾ÆÀÚ¾ÆÀÚ.
					}
				}

				UnlockSectors();

				// ¼½ÅÍ¾È¿¡¼­ ¹üÀ§¿¡ µé¾î°¡´Â Á¡µé¿¡°Ô ³ôÀÌ°ªÀ» ´õÇÑ´Ù..
			}
			break;
		case BRUSHTYPE_CHANGENORMAL:
			{
				// ÁÂÇ¥¿¡¼­ r ¹üÀ§ ¾È¿¡ Á¸ÀçÇÏ´Â ¼½ÅÍ¸¦ Ã£¾Æ³½´Ù..
				BOOL			bFlatShading		;
				state = GetAsyncKeyState( VK_SHIFT );
				if( state < 0 ) bFlatShading	= TRUE	;
				else			bFlatShading	= FALSE	;

				int				i , j				;
				float			fSegPosX, fSegPosY	;
				ApWorldSector *	pWorkingSector		;
				stFindMapInfo	FindMapInfo			;
				float			distance			;
				ApDetailSegment	* pSegment	;
				//ApSubDivisionSegmentInfo		* pDetailedSegmentInfo;
				//ApSubDivisionSegmentInfo::SubdivisionSegment	* pDetailSegment;
				RwRGBA			rgb					;
				rsDWSectorInfo	dwSectorInfo		;

				rgb = g_pMainFrame->GetVertexColor();

				int segmentx , segmentz;
				//int	detailx , detaily;

				int x1 = ALEF_LOAD_RANGE_X1 , x2 = ALEF_LOAD_RANGE_X2 , z1 = ALEF_LOAD_RANGE_Y1 , z2 = ALEF_LOAD_RANGE_Y2;
				int	xc , zc;

				xc = m_pCurrentGeometry->GetArrayIndexX();
				zc = m_pCurrentGeometry->GetArrayIndexZ();

				x1 = xc - 1 ;
				z1 = zc - 1 ;
				x2 = xc + 1 ;
				z2 = zc + 1 ;

				for( j = z1 ; j <= z2 ; j ++ )
				{
					for( i = x1 ; i <= x2 ; i ++ )
					{
						// ¾ÆÀÚ Ã£¾Æº¸ÀÚ.
						pWorkingSector = g_pcsApmMap->GetSectorByArray(  i , 0 , j );

						if( NULL == pWorkingSector ) continue;

						if( !__CheckCircumstanceSectorPolygonInfo( pWorkingSector ) ) continue;
						
						if( pWorkingSector->IsLoadedDetailData() &&
							AGCMMAP_THIS->IsInSectorRadius( g_pcsApmMap->GetSectorByArray( i , 0 , j ) ,
							pos.x , pos.z , g_pMainFrame->m_Document.m_fBrushRadius )
							)
						{
							// ¾ÆÀÚ.;; 
							// ÀÌÁ¦ °¢°¢ ¼¼±×¸ÕÆ®¸¦ µÑ·¯¼­. ¿ø ¹üÀ§ ³»¿¡ µé¾îÀÖ´Â ¼¼±×¸ÕÆ®¶ó¸é Height¸¦ Àû¿ëÇÑ´Ù.

							AGCMMAP_THIS->GetDWSector( pWorkingSector ,  & dwSectorInfo );

							//LockSector( pWorkingSector , ALEF_VERTEXCOLOR_LOCK_MODE );
							
							for( segmentz = 0 ; segmentz < pWorkingSector->D_GetDepth() ; segmentz ++ )
							{
								for( segmentx = 0 ; segmentx < pWorkingSector->D_GetDepth() ; segmentx ++ )
								{
									pSegment = pWorkingSector->D_GetSegment( segmentx , segmentz , &fSegPosX , &fSegPosY );

									if( pSegment )
									{
										// ¹üÀ§Ã¼Å©
										if( ( distance = sqrt( ( fSegPosX - pos.x ) * ( fSegPosX - pos.x ) + ( fSegPosY - pos.z ) * ( fSegPosY - pos.z ) ) )
											> g_pMainFrame->m_Document.m_fBrushRadius ) continue;
										
										// ÀÚ ÀÌÁ¦ ¿Ã¸®ÀÚ..
										if( GetWorldSectorInfo( &FindMapInfo , pWorkingSector , segmentx , segmentz ) )
										{
											pSegment->SetFlag( ApDetailSegment::USEFLATSHADING , bFlatShading );
											AGCMMAP_THIS->D_SetHeight( FindMapInfo.pSector , FindMapInfo.pSector->GetCurrentDetail() , 
												FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ , pSegment->height );

										}
									}
									/////////////////////////////////////
								}
							}// ¼¼±×¸ÕÆ® ·çÇÁ

							
						}
						// ¾ÆÀÚ¾ÆÀÚ.
					}
				}

				UnlockSectors();

				// ¼½ÅÍ¾È¿¡¼­ ¹üÀ§¿¡ µé¾î°¡´Â Á¡µé¿¡°Ô ³ôÀÌ°ªÀ» ´õÇÑ´Ù..
			}
			break;
		case BRUSHTYPE_SMOOTHER:
			{
				// ÁÂÇ¥¿¡¼­ r ¹üÀ§ ¾È¿¡ Á¸ÀçÇÏ´Â ¼½ÅÍ¸¦ Ã£¾Æ³½´Ù..
				state = GetAsyncKeyState( VK_SHIFT );
				BOOL bControl;
				
				// ¸¶°íÀÚ (2004-07-12 ¿ÀÈÄ 5:40:03) : ºüµ¥¸ðµå
				if( ISBUTTONDOWN( VK_CONTROL ) )
					bControl = TRUE;
				else
					bControl = FALSE;
				
				if( state < 0 ) 
				{
					adjustrate	= 0.8f;
					bShift		=	TRUE;
				}
				else			
				{
					adjustrate	= 0.3f;
					bShift		=	FALSE;
				}
				
				int				i , j				;
				float			applyheight			;
				float			fSegPosX, fSegPosY	;
				ApWorldSector *	pWorkingSector		;
				stFindMapInfo	FindMapInfo			;
				//stFindMapInfo	FindMapMatrix[ 9 ]	;
				float			distance			;
				float			prefHeight			;
//				float			takenheight			;
//				int				count				;
				ApDetailSegment	* pSegment	;
				//ApSubDivisionSegmentInfo		* pDetailedSegmentInfo;
				//ApSubDivisionSegmentInfo::SubdivisionSegment	* pDetailSegment;

				int segmentx , segmentz;	
				//int	detailx , detaily;
				float height , original;

				rsDWSectorInfo	dwSectorInfo		;

				int x1 = ALEF_LOAD_RANGE_X1 , x2 = ALEF_LOAD_RANGE_X2 , z1 = ALEF_LOAD_RANGE_Y1 , z2 = ALEF_LOAD_RANGE_Y2;
				int	xc , zc;

				xc = m_pCurrentGeometry->GetArrayIndexX();
				zc = m_pCurrentGeometry->GetArrayIndexZ();

				x1 = xc - 1 ;
				z1 = zc - 1 ;
				x2 = xc + 1 ;
				z2 = zc + 1 ;

				for( j = z1 ; j <= z2 ; j ++ )
				{
					for( i = x1 ; i <= x2 ; i ++ )
					{
						// ¾ÆÀÚ Ã£¾Æº¸ÀÚ.
						pWorkingSector = g_pcsApmMap->GetSectorByArray(  i , 0 , j );

						if( NULL == pWorkingSector ) continue;

						if( !__CheckCircumstanceSectorPolygonInfo( pWorkingSector ) ) continue;

						if( pWorkingSector && pWorkingSector->IsLoadedDetailData() &&
							AGCMMAP_THIS->IsInSectorRadius( g_pcsApmMap->GetSectorByArray( i , 0 , j ) ,
							pos.x , pos.z , g_pMainFrame->m_Document.m_fBrushRadius ) )
						{
							// ¾ÆÀÚ.;; 
							// ÀÌÁ¦ °¢°¢ ¼¼±×¸ÕÆ®¸¦ µÑ·¯¼­. ¿ø ¹üÀ§ ³»¿¡ µé¾îÀÖ´Â ¼¼±×¸ÕÆ®¶ó¸é Height¸¦ Àû¿ëÇÑ´Ù.

							AGCMMAP_THIS->GetDWSector( pWorkingSector ,  & dwSectorInfo );

							//LockSector( pWorkingSector , ALEF_GEOMETRY_LOCK_MODE );

							for( segmentz = 0 ; segmentz < pWorkingSector->D_GetDepth() ; segmentz ++ )
							{
								// ³ôÀÌ ¼³Á¤..
								prefHeight = pWorkingSector->D_GetHeight( pWorkingSector->GetXStart() , pWorkingSector->GetZStart() + ( float ) y * pWorkingSector->GetStepSizeX() );

								for( segmentx = 0 ; segmentx < pWorkingSector->D_GetDepth() ; segmentx ++ )
								{
									pSegment = pWorkingSector->D_GetSegment( segmentx , segmentz , &fSegPosX , &fSegPosY );

									if( pSegment )
									{
										// ¹üÀ§Ã¼Å©
										if( ( distance = sqrt( ( fSegPosX - pos.x ) * ( fSegPosX - pos.x ) + ( fSegPosY - pos.z ) * ( fSegPosY - pos.z ) ) )
											> g_pMainFrame->m_Document.m_fBrushRadius ) continue;
										
										// ¿Ã¶ó°¥ ³ôÀÌ °è»ê..
//										applyheight = adjustrate * exp( ( - ( distance ) / g_pMainFrame->m_Document.m_fBrushRadius * ( distance ) / g_pMainFrame->m_Document.m_fBrushRadius / 2.0 ) * sqrt( 2 * 3.1415927 ) );


										// ÀÚ ÀÌÁ¦ ¿Ã¸®ÀÚ..
										if( GetWorldSectorInfo( &FindMapInfo , pWorkingSector , segmentx , segmentz ) )
										{
											// 0 1 2
											// 3 4 5
											// 6 7 8 ÀÌ·¸°Ô Á¤º¸¸¦ ¼öÁý.
				
//											count = 0;
//											takenheight = pWorkingSector->GetHeight(		pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx - 1 ),
//																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz - 1 ) );
//											if( takenheight != ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT ) { count++; height = takenheight ; }
//											takenheight = pWorkingSector->GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 0 ) ,
//																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz - 1 ) );
//											if( takenheight != ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT ) { count++; height += takenheight ; }
//											takenheight = pWorkingSector->GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 1 ) ,
//																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz - 1 ) );
//											if( takenheight != ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT ) { count++; height += takenheight ; }
//											takenheight = pWorkingSector->GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx - 1 ) ,
//																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 0 ) );
//											if( takenheight != ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT ) { count++; height += takenheight ; }
//											takenheight = pWorkingSector->GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 1 ) ,
//																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 0 ) );
//											if( takenheight != ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT ) { count++; height += takenheight ; }
//											takenheight = pWorkingSector->GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx - 1 ) ,
//																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 1 ) );
//											if( takenheight != ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT ) { count++; height += takenheight ; }
//											takenheight = pWorkingSector->GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 0 ) ,
//																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 1 ) );
//											if( takenheight != ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT ) { count++; height += takenheight ; }
//											takenheight = pWorkingSector->GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 1 ) ,
//																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 1 ) );
//											if( takenheight != ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT ) { count++; height += takenheight ; }
//											takenheight = pWorkingSector->GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 0 ) ,
//																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 0 ) );
//											if( takenheight != ALEF_SECTOR_SEGMENT_DEFAULT_HEIGHT ) 
//												{ count++; height += takenheight ; original = takenheight; }
//											else
//											{	
//												count = 1;
//											}
//											height /= (float) count;
											
											height = pWorkingSector->D_GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx - 1 ),
																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz - 1 ) );
											height += pWorkingSector->D_GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 0 ) ,
																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz - 1 ) );
											height += pWorkingSector->D_GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 1 ) ,
																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz - 1 ) );
											height += pWorkingSector->D_GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx - 1 ) ,
																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 0 ) );
											height += pWorkingSector->D_GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 1 ) ,
																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 0 ) );
											height += pWorkingSector->D_GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx - 1 ) ,
																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 1 ) );
											height += pWorkingSector->D_GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 0 ) ,
																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 1 ) );
											height += pWorkingSector->D_GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 1 ) ,
																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 1 ) );
											height += original = pWorkingSector->D_GetHeight(	pWorkingSector->GetXStart() + pWorkingSector->GetStepSizeX() * ( segmentx + 0 ) ,
																					pWorkingSector->GetZStart() + pWorkingSector->GetStepSizeX() * ( segmentz + 0 ) );
											height /= 9.0f;
											
											if( bControl )
											{
												if( height > original )
												{
													if( height - original > 0.0f && height - original  < 0.5f )
														applyheight = 0.5f;
													else
														applyheight = ( height - original ) * adjustrate;
												}
												else 
													applyheight = 0.0f;
											}
											else
											{
												if( height < original )
												{
													if( original - height > 0.0f && original - height < 0.5f )
														applyheight = -0.5f;
													else
														applyheight = -( original - height ) * adjustrate;

												}
												else 
													applyheight = 0.0f;
											}
											
											SetSectorHeight( &FindMapInfo , __LimitHeight( original + applyheight ) );
										}
									}
									/////////////////////////////////////
								}

							}// ¼¼±×¸ÕÆ® ·çÇÁ

							
						}
						// ¾ÆÀÚ¾ÆÀÚ.
					}
				}
				// ¼½ÅÍ¾È¿¡¼­ ¹üÀ§¿¡ µé¾î°¡´Â Á¡µé¿¡°Ô ³ôÀÌ°ªÀ» ´õÇÑ´Ù..

				UnlockSectors( TRUE );
			}
			break;
		}

	}
}

void MainWindow::TileIdleAdjustment()
{
	PROFILE("MainWindow::TileIdleAdjustment");

	RwV3d	pos = m_Position;

	if( m_bTileSelecting && m_bLButtonDown )
	{
		// Å¸ÀÏ ¼±ÅÃÁß..
		if( m_pCurrentGeometry )
		{
			ApDetailSegment	*	pSelectedSegment	;
			int				posx, posz		;
			//stFindMapInfo	FindMapInfo		;

			pSelectedSegment = GetCurrentPolygonsTileIndex(
				m_pCurrentGeometry		,
				m_nFoundPolygonIndex	,
				( INT32 ) pos.x	,
				( INT32 ) pos.y	,
				&posx					,
				&posz					);

			if( pSelectedSegment == NULL ) return;
			else
			{
				AddTileSelect( m_pCurrentGeometry , posx , posz );
			}
		}
	}

	if( m_bTileAdjust == FALSE ) return;
	
	// ¸¶°íÀÚ (2004-04-16 ¿ÀÈÄ 3:31:32) : ·¯ÇÁ´Â ÀüÇô Ã³¸®ÇÏÁö ¾Ê´Â °ÍÀ» ±âº»À¸·Î ÇÑ´Ù.
	if( m_pCurrentGeometry && m_pCurrentGeometry->GetCurrentDetail() == SECTOR_HIGHDETAIL)
	{
		//ApWorldSector	*	pSelectedSector	;
		int				posx, posz		;
		stFindMapInfo	FindMapInfo		;
		rsDWSectorInfo	dwSectorInfo	;

		switch( GetCurrentLayer() )
		{
		case TILELAYER_BACKGROUND	:
		case TILELAYER_TONGMAP		:
			{
				if( GetAsyncKeyState( VK_MENU ) >= 0 && m_pCurrentGeometry->GetCurrentDetail() == SECTOR_HIGHDETAIL )
				{
					// ÁÂÇ¥¿¡¼­ r ¹üÀ§ ¾È¿¡ Á¸ÀçÇÏ´Â ¼½ÅÍ¸¦ Ã£¾Æ³½´Ù..
					// ºê·¯½Ã Àû¿ë..

					int				i , j				;
					float			fSegPosX, fSegPosY	;
					float			distance			;
					ApWorldSector *	pWorkingSector		;
					ApDetailSegment	* pSegment	;

					// Å¸ÀÏ ÀÎµ¦½º ÁØºñÇÔ..
					int randomtileindex = g_pMainFrame->m_Document.GetSelectedTileIndex();

					if( randomtileindex == 0 )
					{
						DisplayMessage( AEM_ERROR , "¼±ÅÃ‰?Å¸ÀÏÀÌ ¾ø¾î¼­ Å¸ÀÏÀ» Ãâ·ÂÇÒ ¼ö ¾ø½À´Ï´Ù." );
						m_bTileAdjust = FALSE;
						m_UndoManager.EndActionBlock();
						break;
					}

					// ¸ÓÆ¼¸®¾ó ÁØºñ..

					int segmentx , segmentz;

					int x1 = ALEF_LOAD_RANGE_X1 , x2 = ALEF_LOAD_RANGE_X2 , z1 = ALEF_LOAD_RANGE_Y1 , z2 = ALEF_LOAD_RANGE_Y2;
					int	xc , zc;

					xc = m_pCurrentGeometry->GetArrayIndexX();
					zc = m_pCurrentGeometry->GetArrayIndexZ();

					x1 = xc - 1 ;
					z1 = zc - 1 ;
					x2 = xc + 1 ;
					z2 = zc + 1 ;
			
					// ±×³É ÁöÇü ¿Ã¸®±â..
					for( j = z1 ; j <= z2 ; j ++ )
					{
						for( i = x1 ; i <= x2 ; i ++ )
						{
							// ¾ÆÀÚ Ã£¾Æº¸ÀÚ.
							pWorkingSector = g_pcsApmMap->GetSectorByArray(  i , 0 , j );

							if( NULL == pWorkingSector ) continue;

							if( !__CheckCircumstanceSectorPolygonInfo( pWorkingSector ) ) continue;

							if( pWorkingSector->IsLoadedDetailData() &&
								AGCMMAP_THIS->IsInSectorRadius( g_pcsApmMap->GetSectorByArray( i , 0 , j ) ,
									pos.x , pos.z , g_pMainFrame->m_Document.m_fBrushRadius )
								)
							{
								// ¾ÆÀÚ.;; 
								// ÀÌÁ¦ °¢°¢ ¼¼±×¸ÕÆ®¸¦ µÑ·¯¼­. ¿ø ¹üÀ§ ³»¿¡ µé¾îÀÖ´Â ¼¼±×¸ÕÆ®¶ó¸é Height¸¦ Àû¿ëÇÑ´Ù.
								AGCMMAP_THIS->GetDWSector( g_pcsApmMap->GetSectorByArray(  i , 0 , j ) , &dwSectorInfo );

								
								for( segmentz = 0 ; segmentz < pWorkingSector->D_GetDepth() ; segmentz ++ )
								{
									for( segmentx = 0 ; segmentx < pWorkingSector->D_GetDepth() ; segmentx ++ )
									{
										pSegment = pWorkingSector->D_GetSegment( segmentx , segmentz , &fSegPosX , &fSegPosY );

										// ÁßÁ¡À¸·Î ÀÌµ¿..
										fSegPosX	+= MAP_STEPSIZE / 2.0f;
										fSegPosY	+= MAP_STEPSIZE / 2.0f;

										randomtileindex = g_pMainFrame->m_Document.GetSelectedTileIndex();

										if( pSegment )
										{
											// ¹üÀ§Ã¼Å©
											if( ( distance = sqrt(	( fSegPosX - pos.x ) * ( fSegPosX - pos.x ) +
																	( fSegPosY - pos.z ) * ( fSegPosY - pos.z ) ) )
												> g_pMainFrame->m_Document.m_fBrushRadius ) continue;

											// Åë¸Ê Ã³¸®..
											randomtileindex = __GetTongMapIndex( randomtileindex , pWorkingSector , segmentx , segmentz );


											if( GetWorldSectorInfo( &FindMapInfo , pWorkingSector , segmentx , segmentz ) )
											{
												if( pWorkingSector != FindMapInfo.pSector )
													continue;

												ApplyAlpha( &FindMapInfo , 0 , randomtileindex , g_pMainFrame->m_Document.m_nCurrentAlphaTexture , FALSE );
											}


										}
										/////////////////////////////////////
									}
								}

								if( pWorkingSector && pWorkingSector->GetCurrentDetail() == SECTOR_LOWDETAIL )
									AGCMMAP_THIS->Update( pWorkingSector );
								else
									UnlockSectors();

							}
						}
					}// ¼½ÅÍ Æ÷ ·çÇÁ..
				}
				else
				{
					// ALT ´©¸£°í ÀÖÀ¸¸é µµÆ® ¸ðµå.
					ApDetailSegment	*	pSelectedSegment	;
					pSelectedSegment = GetCurrentPolygonsTileIndex(
						m_pCurrentGeometry		,
						m_nFoundPolygonIndex	,
						( INT32 ) pos.x	,
						( INT32 ) pos.y	,
						&posx					,
						&posz					);

					if( pSelectedSegment == NULL ) return;

					if( GetWorldSectorInfo( &FindMapInfo , m_pCurrentGeometry , posx , posz ) )
					{	
						AGCMMAP_THIS->GetDWSector( FindMapInfo.pSector ,  & dwSectorInfo );
						int randomtileindex = g_pMainFrame->m_Document.GetSelectedTileIndex();

						ASSERT( NULL != dwSectorInfo.pDWSector->geometry );

						if( randomtileindex == 0 )
						{
							DisplayMessage( AEM_ERROR , "¼±ÅÃ‰?Å¸ÀÏÀÌ ¾ø¾î¼­ Å¸ÀÏÀ» Ãâ·ÂÇÒ ¼ö ¾ø½À´Ï´Ù." );
							m_bTileAdjust = FALSE;
							m_UndoManager.EndActionBlock();
							break;
						}

						// Åë¸Ê Ã³¸®..
						randomtileindex = __GetTongMapIndex( randomtileindex , FindMapInfo.pSector , FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ );
						// ÀÌ¹Ì ÂïÈù°ÍÀº.. ÂïÁö ¾ÊÀ½..

						if( FindMapInfo.pSegment->pIndex[ TD_FIRST	] == randomtileindex			&& 
							FindMapInfo.pSegment->pIndex[ TD_SECOND	] == ALEF_TEXTURE_NO_TEXTURE	&&
							FindMapInfo.pSegment->pIndex[ TD_THIRD	] == ALEF_TEXTURE_NO_TEXTURE	&&
							FindMapInfo.pSegment->pIndex[ TD_FOURTH	] == ALEF_TEXTURE_NO_TEXTURE	&&
							FindMapInfo.pSegment->pIndex[ TD_FIFTH	] == ALEF_TEXTURE_NO_TEXTURE	&&
							FindMapInfo.pSegment->pIndex[ TD_SIXTH	] == ALEF_TEXTURE_NO_TEXTURE	) break;
						
						m_UndoManager.AddTileActionUnit( FindMapInfo.pSector , FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ ,
							randomtileindex );
						AGCMMAP_THIS->D_SetTile( FindMapInfo.pSector , FindMapInfo.pSector->GetCurrentDetail() ,
							FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ , randomtileindex );

						UnlockSectors();

					}// GetWorldSectorInfo
					else
					{
						int randomtileindex = g_pMainFrame->m_Document.GetSelectedTileIndex();

						randomtileindex = __GetTongMapIndex( randomtileindex , m_pCurrentGeometry , posx , posz );

						m_UndoManager.AddTileActionUnit( m_pCurrentGeometry , 
							posx , posz ,
							randomtileindex );
						AGCMMAP_THIS->D_SetTile( m_pCurrentGeometry , m_pCurrentGeometry->GetCurrentDetail() ,
							posx , posz , randomtileindex );
						AGCMMAP_THIS->Update( m_pCurrentGeometry );
					}

				}//	if( GetAsyncKeyState( VK_MENU ) >= 0 )
			}
			break;
		case TILELAYER_ALPHATILEING		:
			{
				ApDetailSegment	*	pSelectedSegment	;
				pSelectedSegment = GetCurrentPolygonsTileIndex(
					m_pCurrentGeometry		,
					m_nFoundPolygonIndex	,
					( INT32 ) pos.x	,
					( INT32 ) pos.y	,
					&posx					,
					&posz					);

				if( pSelectedSegment == NULL ) return;
				
				if( m_pCurrentGeometry->GetCurrentDetail() == SECTOR_LOWDETAIL )
				{
					// do nothing.
				}
				else
				if( GetWorldSectorInfo( &FindMapInfo , m_pCurrentGeometry , posx , posz ) )
				{	
					AGCMMAP_THIS->GetDWSector( m_pCurrentGeometry ,  & dwSectorInfo );

					ASSERT( NULL != dwSectorInfo.pDWSector->geometry );

					// ÇöÀç Å¸ÀÏÀ» ±âÁØÀ¸·Î ÁÖÀ§¿¡ ¾ËÆÄ¸¦ ¸ÔÀÎ´Ù.
					ApplyAlpha( &FindMapInfo , 0 , FindMapInfo.pSegment->pIndex[ TD_FIRST ] , g_pMainFrame->m_Document.m_nCurrentAlphaTexture , FALSE );

					UnlockSectors();

					m_bTileAdjust = FALSE;
				}
			}
			break;

		case TILELAYER_UPPER			:
			{
				// ¾îÆÛ ÅØ½ºÃÄ Âï±â..
				ApDetailSegment	*	pSelectedSegment	;
				pSelectedSegment = GetCurrentPolygonsTileIndex(
					m_pCurrentGeometry		,
					m_nFoundPolygonIndex	,
					( INT32 ) pos.x	,
					( INT32 ) pos.y	,
					&posx					,
					&posz					);

				if( pSelectedSegment == NULL ) return;

				if( GetWorldSectorInfo( &FindMapInfo , m_pCurrentGeometry , posx , posz ) )
				{	
					AGCMMAP_THIS->GetDWSector( FindMapInfo.pSector ,  & dwSectorInfo );
					int randomtileindex = g_pMainFrame->m_Document.GetSelectedTileIndex();

					if( randomtileindex == 0 )
					{
						DisplayMessage( AEM_ERROR , "¼±ÅÃ‰?Å¸ÀÏÀÌ ¾ø¾î¼­ Å¸ÀÏÀ» Ãâ·ÂÇÒ ¼ö ¾ø½À´Ï´Ù." );
						m_bTileAdjust = FALSE;
						m_UndoManager.EndActionBlock();
						break;
					}

					// ÀÌ¹Ì ÂïÈù°ÍÀº.. ÂïÁö ¾ÊÀ½..

					if( FindMapInfo.pSegment->pIndex[ TD_FOURTH ] == randomtileindex ) break;

					m_UndoManager.AddTileActionUnit( FindMapInfo.pSector , FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ ,
						FindMapInfo.pSegment->pIndex[ TD_FIRST	] ,
						FindMapInfo.pSegment->pIndex[ TD_SECOND	] ,
						FindMapInfo.pSegment->pIndex[ TD_THIRD	] ,
						FindMapInfo.pSegment->pIndex[ TD_FOURTH	] ,
						FindMapInfo.pSegment->pIndex[ TD_FIFTH	] ,
						randomtileindex ) ;
					AGCMMAP_THIS->D_SetTile( FindMapInfo.pSector , FindMapInfo.pSector->GetCurrentDetail() ,
						FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ ,
						FindMapInfo.pSegment->pIndex[ TD_FIRST	] ,
						FindMapInfo.pSegment->pIndex[ TD_SECOND	] ,
						FindMapInfo.pSegment->pIndex[ TD_THIRD	] ,
						FindMapInfo.pSegment->pIndex[ TD_FOURTH	] ,
						FindMapInfo.pSegment->pIndex[ TD_FIFTH	] ,
						randomtileindex ) ;

					UnlockSectors();

				}
			}
			break;

		}
	}

}


BOOL MainWindow::OnMouseWheel	( BOOL bForward		)
{
	if( g_pMainFrame->m_pTileList->m_pOthers->Window_MouseWheel( bForward ) ) return TRUE;
	
	if( GetAsyncKeyState( VK_SHIFT ) < 0 )
	{
		FLOAT fProjection = g_pEngine->GetProjection();

		// 5%¾¿..º¯È­..
		if( bForward )
		{
			fProjection	= fProjection + fProjection * 0.05f;
		}
		else
		{
			fProjection	= fProjection - fProjection * 0.05f;
		}

		g_pEngine->SetProjection( fProjection );

		DisplayMessage( "Perspective (%f)" , fProjection );
	}
	else
	{
		if( g_pMainFrame->m_Document.IsInFirstPersonViewMode() )
		{
			RwFrame	*pstCameraFrame	= RwCameraGetFrame(g_pEngine->m_pCamera);
			RwV3d	stXAxis			= {1.0f, 0.0f, 0.0f};
			RwV3d	stZoom			= {0.0f, 0.0f, bForward ? -20.0f:20.0f};
			FLOAT	fPan = bForward ? m_fCameraPanUnit:-m_fCameraPanUnit;

			//g_pcsAgcmCamera->ControlCameraDistance( bForward );
			g_pcsAgcmCamera2->bZoom( bForward ? 200.0f : -200.0f );
		}
		else
		{
			switch( g_pMainFrame->m_bFullScreen )
			{
			case	CMainFrame::MT_VM_DUNGEON		:
				{
					// ´øÁ¯ ¸ðµå¿¡¼± ±×¸®µå Å©±â¸¦ º¯°æÇÑ´Ù..

					int	nWidth = g_pMainFrame->m_pDungeonWnd->GetGridWidth();

					int	nDelta = nWidth / 4;
					if( nDelta <= 0 ) nDelta = 1;

					if( bForward )	nWidth += nDelta;
					else			nWidth -= nDelta;

					if( nWidth <= 0 ) nWidth = 1;

					g_pMainFrame->m_pDungeonWnd->SetGridWidth( nWidth );
				}
				break;
			default:
				{
					// ÁÜÀÎ / ÁÜ¾Æ¿ô.
					RwFrame		*pFrame		= RwCameraGetFrame	( g_pEngine->m_pCamera	);
					RwMatrix	*pMatrix	= RwFrameGetMatrix	( pFrame				);

					RwV3d		up, right, at , pos , trans;

					right	= *RwMatrixGetRight	( pMatrix	);
					up		= *RwMatrixGetUp	( pMatrix	);
					at		= *RwMatrixGetAt	( pMatrix	);
					pos		= *RwMatrixGetPos	( pMatrix	);

					RwV3dNormalize( &right	, &right	);
					RwV3dNormalize( &up		, &up		);
					RwV3dNormalize( &at		, &at		);

					short state = GetAsyncKeyState( VK_SHIFT );

					float		distance;
					if( state < 0 )
					{
						distance	= ALEF_SECTOR_WIDTH * 2.0f * ( ( float ) 800 / 1000.0f );	// 1ÃÊ¿¡ 1000.0f ÀÌµ¿.
					}
					else
					{
						distance	= ALEF_SECTOR_WIDTH * ( ( float ) 800 / 1000.0f );	// 1ÃÊ¿¡ 100.0f ÀÌµ¿.
					}

					if( !bForward )
					{
						// ÀüÁø!
						RwV3dScale		( &trans	, &at		, distance				);

						if( __CropCameraMovement( &pos , &trans ) )
							RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
					}
					else
					{
						// ÈÄÁø!
						RwV3dScale		( &trans	, &at		, -distance				);

						if( __CropCameraMovement( &pos , &trans ) )
							RwFrameTranslate( pFrame	, &trans	, rwCOMBINEPOSTCONCAT	);
					}

					AuPOS	posCamera;
					RwV3d	*pCameraPos = RwMatrixGetPos( pMatrix );

					posCamera.x	= pCameraPos->x;
					posCamera.y	= pCameraPos->y;
					posCamera.z	= pCameraPos->z;

					if( g_pcsAgcmEventNature )
					{
						if( posCamera.y > 10000.0f / 1.2f )
							g_pcsAgcmEventNature->SetSkyHeight( posCamera.y * 1.2f );
						else
							g_pcsAgcmEventNature->SetSkyHeight( 10000.0f			);

						g_pcsAgcmEventNature->SetCharacterPosition( posCamera );
					}

				}
				break;
			}
		}		
	}

	return TRUE;
}

int		MainWindow::GetTileIndexOnMouseCursor( int type )
{
	//m_PrevMouseStatus;

	if( m_pCurrentGeometry )
	{
		ApDetailSegment	*	pSelectedSegment	;
		int				posx, posz		;

		pSelectedSegment = GetCurrentPolygonsTileIndex(
			m_pCurrentGeometry					,
			m_nFoundPolygonIndex				,
			( INT32 ) m_PrevMouseStatus.pos.x	,
			( INT32 ) m_PrevMouseStatus.pos.y	,
			&posx								,
			&posz								);

		if( pSelectedSegment )
		{
			ApDetailSegment * pSegment;

			pSegment = m_pCurrentGeometry->D_GetSegment( posx , posz );

			if( pSegment )
			{
				return pSegment->pIndex[ type ]	;
			}
		}
	}

	return 0;
}

BOOL		MainWindow::GetVertexColorOnMouseCursor	( RwRGBA * pRGB )
{
	if(	m_pCurrentGeometry )
	{
		ApDetailSegment	*	pSelectedSegment	;
		int				posx, posz			;

		pSelectedSegment = GetCurrentPolygonsTileIndex(
			m_pCurrentGeometry					,
			m_nFoundPolygonIndex				,
			( INT32 ) m_PrevMouseStatus.pos.x	,
			( INT32 ) m_PrevMouseStatus.pos.y	,
			&posx								,
			&posz								);

		if( pSelectedSegment )
		{
			//RpDWSector		* pDWSector	;
			ApDetailSegment	* pSegment	;

			pSegment = m_pCurrentGeometry->D_GetSegment( posx , posz );

//			pDWSector	=	AGCMMAP_THIS->RpDWSectorGetDetail			( m_pCurrentGeometry ,
//							AGCMMAP_THIS->RpDWSectorGetCurrentDetail	( m_pCurrentGeometry ) );

			if( pSegment )
			{
				*pRGB = ApRGBAToRwRGBA( pSegment->vertexcolor );
				return TRUE;
			}
		}
	}

	return FALSE;
}

void MainWindow::PopupMenuSelect(int index)
{
	ASSERT( !"»ç¿ëÇÏÁö ¾ÊÀ½" );
//	if( index >= m_listClumpSorted.GetCount() ) return;
//
//	m_pSelectedClump = m_listClumpSorted[ index ].pClump;
//
//	ClumpGetBBox( m_pSelectedClump , &PickBox );
//
//	// ¿ÀºêÁ§Æ® Á¤º¸ Ã£¾Æ³¿..
//	RwFrame *pFrame = RpClumpGetFrame	( m_pSelectedClump			);
//
//	//RpUserDataArray *pUserDataArray	;
//	
//	ApdObject *		pstApdObject	;
//	AgcdObject *	pstAgcdObject	;
//	m_stObjectUndoInfo.oid = 0;
//	// Object ID¸¦ ¾ò¾î³¿.
//	if( ACUOBJECT_TYPE_OBJECT != AcuObject::GetClumpType( m_pSelectedClump, &m_stObjectUndoInfo.oid ) )
//		m_stObjectUndoInfo.oid = 0;
//	/*
//	for( int i = 0; i < RwFrameGetUserDataArrayCount( pFrame ) ; i++ )
//	{
//		pUserDataArray = RwFrameGetUserDataArray( pFrame, i );
//		if( pUserDataArray )
//		{
//			if( !strcmp( pcsTemplate( pUserDataArray ), "OID" ) )
//			{
//				m_stObjectUndoInfo.oid = RpUserDataArrayGetInt( pUserDataArray, 0 );
//			}
//		}
//	}
//	*/
//	
//	pstApdObject	= g_pcsApmObject->	GetObject		( m_stObjectUndoInfo.oid	);
//	ASSERT( NULL != pstApdObject	);
//	pstAgcdObject	= g_pcsAgcmObject->	GetObjectData	( pstApdObject				);
//	ASSERT( NULL != pstAgcdObject	);
//
//	m_stObjectUndoInfo.stScale		= pstApdObject->m_stScale		;	// Scale Vector
//	m_stObjectUndoInfo.stPosition	= pstApdObject->m_stPosition	;	// Base Position
//	m_stObjectUndoInfo.stAxis		= pstApdObject->m_stAxis		;	// Rotation Axis
//	m_stObjectUndoInfo.fDegree		= pstApdObject->m_fDegree		;	// Rotation Degree
}

void MainWindow::PopupMenuObject(int index)
{
	INT32					lID;

	if( !IsObjectSelected() )
		return;

	if( m_bForwardKeyDown		|| m_bBackwardKeyDown	|| m_bStepLeftKeyDown	||
		m_bStepRightKeyDown		|| m_bLiftUpKeyDown		|| m_bLiftDownKeyDown	||
		m_bRotateCCWDown		|| m_bRotateCWDown		|| m_bUp				||
		m_bDown																	)
		return;

	INT32	nType = AcuObject::GetClumpType( GetSelectedObject(), &lID );

	switch( AcuObject::GetType( nType ) )
	{
	case	ACUOBJECT_TYPE_OBJECT			:
		{
			switch (index)
			{
			case IDM_OBJECT_PROPERTY:
				{
					CObjectPropertiesDlg	dlg;
					ApdObject *				pcsObject = g_pcsApmObject->GetObject(lID);
					AgcdObject *			pstAgcdObject = g_pcsAgcmObject->GetObjectData(pcsObject);
					
					BOOL					bSuccess = FALSE;

					#ifndef _DEBUG
					try
					#endif // _DEBUG
					{
						dlg.SetObject(pcsObject, pstAgcdObject);

						if( dlg.DoModal() == IDOK)
						{
						}

						bSuccess = TRUE;
					}
					#ifndef _DEBUG
					catch(...)
					{
						if( dlg.IsWindowEnabled() ) dlg.DestroyWindow();
						
						char	str[] = "Dialog°¡ Á¤»óÀûÀ¸·Î Á¾·áµÅÁö ¾Ê¾Ò½À´Ï´Ù.. ÈÄµü ½êÀÌºêÇÏ°í ´Ù½ÃÄÑ¼¼¿ë. ¼¼ÀÌºê°¡ À§ÇèÇÒ¼öµµ ÀÖ½À´Ï´Ù! -_-; ±×·¡µµ ½êÀÌºê ÇÏ½Ã°Ú½À´Ï±î?.";
										if( IDYES == g_pMainFrame->MessageBox( str , "¿¡·¯" , MB_ICONERROR | MB_YESNO ) ) 
						{
							g_pMainFrame->Save();
						}
					}
					#endif // _DEBUG
				}

				break;

			case IDM_OBJECT_EVENT:
				{
					#ifndef _DEBUG
					try
					#endif // _DEBUG
					{
						ApdObject * pObject = g_pcsApmObject->GetObject( lID );
						ApdEvent *	pstEvent , * pstEventNext	;

						// EventNature Event Struct ¾ò¾î³¿..
						pstEvent	= g_pcsApmEventManager->GetEvent( pObject , APDEVENT_FUNCTION_NATURE );

						if( g_pcsApmEventManagerDlg->Open( g_pcsApmObject->GetObject( lID ) ) )
						{
							// ¸¶°íÀÚ (2004-06-10 ¿ÀÀü 11:46:44) : º¯È­»óÈ²À» ¸ðµâ¿¡ Åëº¸ÇÔ..
							pstEventNext 	= g_pcsApmEventManager->GetEvent( pObject , APDEVENT_FUNCTION_NATURE );

							if( pstEvent && !pstEventNext )
							{
								g_pcsAgcmEventNature->RemoveSkyObject( lID );
							}
							if( !pstEvent && pstEventNext )
							{
								g_pcsAgcmEventNature->AddSkyObject( lID );
							}

							SaveSetChangeObjectList();
						}

					}
					#ifndef _DEBUG
					catch(...)
					{
						char	str[] = "Dialog°¡ Á¤»óÀûÀ¸·Î Á¾·áµÅÁö ¾Ê¾Ò½À´Ï´Ù.. ÈÄµü ½êÀÌºêÇÏ°í ´Ù½ÃÄÑ¼¼¿ë. ¼¼ÀÌºê°¡ ´õ À§ÇèÇÒ ¼ö ÀÖ½À´Ï´Ù! -_-; ±×·¡µµ ½êÀÌºê ÇÏ½Ã°Ú½À´Ï±î?.";
						if( IDYES == g_pMainFrame->MessageBox( str , "¿¡·¯" , MB_ICONERROR | MB_YESNO ) ) 
						{
							g_pMainFrame->Save();
						}
					}
					#endif // _DEBUG
				}
				break;

			case IDM_OBJECTPOP_GROUPSAVE:
				{
					g_pMainFrame->OnObjectpopGroupsave();
				}
				break;
			}
		}
		break;

	default:
		return;
	}

}

void MainWindow::OnChangeCurrentSector( ApWorldSector * pCurrent , ApWorldSector * pFuture )
{
	// Æä¾î·±Æ®¿¡ Åëº¸ÇÔ..
	g_pMainFrame->PostMessage( WM_WORKINGSECTORCHANGE );

//	if( ( g_pMainFrame->m_Document.m_nBrushType == BRUSHTYPE_GEOMETRY || g_pMainFrame->m_Document.m_nBrushType == BRUSHTYPE_SMOOTHER ) &&
//		GetAsyncKeyState( VK_SHIFT ) < 0							&&
//		m_bGeometryAdjust && pCurrent && pFuture == NULL)
//	{
//		UpdateCollision( pCurrent );
//		UpdateCollision( pCurrent->GetNearSector( TD_NORTH	) );
//		UpdateCollision( pCurrent->GetNearSector( TD_EAST	) );
//		UpdateCollision( pCurrent->GetNearSector( TD_SOUTH	) );
//		UpdateCollision( pCurrent->GetNearSector( TD_WEST	) );
//	}

	// DWSector Setting¿¡ µû¶ó¼­... ÁÖÀ§ µðÅ×ÀÏ ¼½ÅÍ ¼³Á¤À» º¯°æÇØÁØ´Ù.
	switch( g_pMainFrame->m_Document.m_nDWSectorSetting )
	{
	case DWSECTOR_EACHEACH			:
	case DWSECTOR_ALLROUGH			:
	case DWSECTOR_ALLDETAIL			:
		// Do no op
		break;
	case DWSECTOR_ACCORDINGTOCURSOR	:
		{
			// Ä¿·±Æ®ÀÇ µðÅ×ÀÏÀ» ¸ðµÎ ´õ¹Ì·Î ¹Ù²Û´Ù.
			if( pCurrent )
			{
				ApWorldSector * pSector;
				int	x = pCurrent->GetIndexX();
				int	z = pCurrent->GetIndexZ();

				AGCMMAP_THIS->SetCurrentDetail( pCurrent , SECTOR_LOWDETAIL );

				// ÁÖº¯¼½ÅÍ..
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 0 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z + 0 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 0 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z + 0 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );

				// ÀÎÁ¢¼½ÅÍ..
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 0 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z + 0 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 0 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z + 0 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
			}

			if( pFuture )
			{
				ApWorldSector * pSector;
				int	x = pFuture->GetIndexX();
				int	z = pFuture->GetIndexZ();

				AGCMMAP_THIS->SetCurrentDetail( pFuture , SECTOR_HIGHDETAIL );

				// ÁÖº¯¼½ÅÍ..
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 0 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z - 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z + 0 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 2 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 0 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z + 2 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z + 0 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 2 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_LOWDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_LOWDETAIL );

				// ÀÎÁ¢¼½ÅÍ..
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_HIGHDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_HIGHDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 0 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_HIGHDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_HIGHDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z - 1 ) ) && pSector->GetCurrentDetail() != SECTOR_HIGHDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_HIGHDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z + 0 ) ) && pSector->GetCurrentDetail() != SECTOR_HIGHDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_HIGHDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 1 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_HIGHDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_HIGHDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x + 0 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_HIGHDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_HIGHDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z + 1 ) ) && pSector->GetCurrentDetail() != SECTOR_HIGHDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_HIGHDETAIL );
				if( ( pSector = g_pcsApmMap->GetSector( x - 1 , 0 , z + 0 ) ) && pSector->GetCurrentDetail() != SECTOR_HIGHDETAIL	) AGCMMAP_THIS->SetCurrentDetail( pSector , SECTOR_HIGHDETAIL );
			}



			// Ç»ÃÄÀÇ µðÅ×ÀÏÀ» º¯°æÇÑ´Ù.
		}
		break;
	}

	g_MyEngine.MoveSubCameraToCenter();
}

void	MainWindow::AddTileSelect( ApWorldSector * pSector , int x , int z )
{
	// µðÅ×ÀÏ ¼½ÅÍÀÎÁö Á¡°Ë
	ApDetailSegment * pSegment = pSector->D_GetSegment( x , z );

	// µðÅ×ÀÏ ¼¼±×¸ÕÆ®ÀÇ °æ¿ì »ðÀÔÇÏÁö ¾ÊÀ½..
	_TileSelectStruct	tile , *pTile;

	tile.x			= x			;
	tile.z			= z			;
	tile.pSector	= pSector	;

	AuNode< _TileSelectStruct >	* pNode = m_listTileSelect.GetHeadNode();

	while( pNode )
	{
		pTile = & pNode->GetData();

		//°ãÄ¡´Â°Å ÀÖ³ª?..;;
		if( pTile->pSector == pSector && pTile->x == x && pTile->z == z )
			return;

		m_listTileSelect.GetNext( pNode );
	}

	m_listTileSelect.AddTail( tile );

	// ¼±ÅÃµÈ °¹¼ö Ãâ·Â
	TRACE( "%d°³ÀÇ Å¸ÀÏÀÌ ¼±ÅÃµÅ¾ú½À´Ï´Ù.\n" , m_listTileSelect.GetCount() );
}

void	MainWindow::CancelTileSelect()
{
	m_bTileSelecting = FALSE;
	m_listTileSelect.RemoveAll();
}


void MainWindow::OnEditModeChange	(	int changedmode		, int prevmode	)
{
	if( changedmode == prevmode ) return;
	// ¸ðµå Ã¼ÀÎÁö½Ã ¸®¼Ò½º ¼¿·º¼Ç ¸®¹«ºù..
	switch( prevmode )
	{
	case	EDITMODE_GEOMETRY	:
	case	EDITMODE_TILE		:
	case	EDITMODE_OBJECT		:
	case	EDITMODE_OTHERS		:
		break;
	}

	// ¸ðµå µé¾î°¥¶§ ÃÊ±âÈ­,.
	switch( changedmode )
	{
	case	EDITMODE_GEOMETRY	:
	case	EDITMODE_TILE		:
	case	EDITMODE_OBJECT		:
	case	EDITMODE_OTHERS		:
		break;
	}

}
void MainWindow::OnTileLayerChange	(	int changedlayer	, int prevlayer	)
{
	if( changedlayer == prevlayer ) return;

	// ·¹ÀÌ¾î º¯È°¶§ ÃÊ±âÈ­.
	switch( prevlayer )
	{
	case	TILELAYER_BACKGROUND	:
	case	TILELAYER_TONGMAP		:
	case	TILELAYER_ALPHATILEING	:
	case	TILELAYER_UPPER			:
		break;
	}

	switch( changedlayer )
	{
	case	TILELAYER_BACKGROUND	:
	case	TILELAYER_TONGMAP		:
	case	TILELAYER_ALPHATILEING	:
	case	TILELAYER_UPPER			:
		break;
	}
}


void	MainWindow::ApplyAlphaOnTheSelection	()
{
//	// ¸ðµåÃ¼Å·
//	if( GetCurrentMode()	!= EDITMODE_TILE &&
//		GetCurrentLayer()	!= TILELAYER_ALPHATILEING )
//	{
//		// ¿¡·¯Ç¥½Ã
//		DisplayMessage( AEM_NOTIFY , "ÇöÀç ¸ðµå¿¡¼­´Â ÀÛµ¿ÇÏÁö ¾Ê½À´Ï´Ù. Å¸ÀÏ/¾ËÆÄÂï±â ¸ðµå·Î ¹Ù²ãÁÖ¼¼¿ä." );
//		return;
//	}
//
//	if( m_listTileSelect.GetCount() < 0 )
//	{
//		// ¿¡·¯Ç¥½Ã
//		DisplayMessage( AEM_NOTIFY , "ÁöÇüÀÌ ¼±ÅÃµÅ¾îÀÖÁö ¾Ê½À´Ï´Ù." );
//		return;
//	}
//
//	// Å¸ÀÏ ¼±ÅÃµÅ¾îÀÖ´ÂÁö..
//	if( g_pMainFrame->m_Document.GetSelectedTileIndex() == 0 )
//	{
//		DisplayMessage( AEM_ERROR , "¼±ÅÃ‰?Å¸ÀÏÀÌ ¾ø¾î¼­ Å¸ÀÏÀ» Ãâ·ÂÇÒ ¼ö ¾ø½À´Ï´Ù." );
//		return;
//	}
//
//	DisplayMessage( AEM_NORMAL , "¾ËÆÄÅ¸ÀÏÀ» °è»êÁßÀÔ´Ï´Ù." );
//
//	AuNode<_TileSelectStruct>	*pNode = m_listTileSelect.GetHeadNode();
//	stFindMapInfo				MapInfo;
//	_TileSelectStruct			*pTile;
//	int randomtileindex = g_pMainFrame->m_Document.GetSelectedTileIndex();
//	int	index;
//	index = MAKE_TEXTURE_INDEX(
//				GET_TEXTURE_TYPE	( randomtileindex )		,
//				GET_TEXTURE_INDEX	( randomtileindex )		,
//				0x00 );
//
//	m_UndoManager.StartActionBlock( CUndoManager::TILE );
//
//	CProgressDlg	dlg;
//	dlg.StartProgress( "¾ËÆÄÅ¸ÀÏ °è»êÁß..." , m_listTileSelect.GetCount() , g_pMainFrame );
//	int	count = 0;
//
//	RpWorldLock( g_pEngine->m_pWorld);
//
//	while( pNode )
//	{
//		pTile = &pNode->GetData();
//
//		if( GetWorldSectorInfo( &MapInfo , pTile->pSector , pTile->x , pTile->y ) )
//		{
//			ChangeSegmentMaterial2( &MapInfo , pTile->x , pTile->y , index , 1 );
//		}
//
//		m_listTileSelect.GetNext( pNode );
//
//		count ++;
//		dlg.SetProgress( count );
//	}
//
//	RpWorldUnlock( g_pEngine->m_pWorld);
//
//	m_UndoManager.EndActionBlock();
//
//	dlg.EndProgress();
//
//	DisplayMessage( AEM_NORMAL , "¾ËÆÄÅ¸ÀÏ °è»êÀÌ ³¡³µ½À´Ï´Ù." );
}

BOOL		MainWindow::ApplyAlpha	(stFindMapInfo *pMapInfo, int basetile , int maskedindex , int alphaindex , BOOL bReculsive , BOOL bFirst )
{
	// Offset -> Data
	static UINT8 aConvertTable_OD[ 16 ] = {
		0x01,	0x02,	0x04,	0x08,
		0x03,	0x0a,	0x05,	0x0c,
		0x07,	0x0b,	0x0d,	0x0e,
		0x09,	0x06,	0x00,	0x0f
	};

	// Data -> Offset
	static UINT8 aConvertTable_DO[ 16 ] = {
		0x0e,	0x00,	0x01,	0x04,
		0x02,	0x06,	0x0d,	0x08,
		0x03,	0x0c,	0x05,	0x09,
		0x07,	0x0a,	0x0b,	0x0f
	};

	
	static int offsetX[9] = {+1, 0, -1, +1, 0, -1, +1, 0, -1};
	static int offsetY[9] = {+1, +1, +1, 0, 0, 0, -1, -1, -1};

	static int aValue[16]	= { 
		0 , 1 , 1 , 2 ,
		1 , 2 , 2 , 3 ,
		1 , 2 , 2 , 3 ,
		2 , 3 , 3 , 4 };

	static int offsetPortion[ 9 ] = { 0x01 , 0x03 , 0x02 , 0x05 , 0x0f , 0x0a , 0x04 , 0x0c , 0x08 };

	int		posx2 , posy2;
	//BOOL	bRet;

	// ÇÏÇÏÇÏ! -_-+

	// °Ë»ç ÇÑ°÷ÀÎÁö Ã¼Å©.
	if( pMapInfo->pSegment->uCheckTime == g_pEngine->GetTickCount() )
		return FALSE;

	pMapInfo->pSegment->uCheckTime = g_pEngine->GetTickCount();

	// °Ë»çÇÑ ÇÃ·¡±×¸¦ ³²±è.
	// ÁÖÀ§ 9°³ÀÇ Á¤º¸¸¦ ¾ò¾î³¿.
	// ÀÚ½ÅÀÇ ÅØ½ºÃÄ¸¦ °áÁ¤ÇÔ.
	// 9°³¿¡ °¢°¢ ¸®ÄÃ½Ãºê ÄÝÀ» ³Ñ±è.
	// 

	stFindMapInfo pFindMapSelf[ 9 ];
	for ( int i = 0; i < 9; i++ )
	{
		if( i == 4 ) continue;

		posx2 = pMapInfo->nSegmentX + offsetX[i];
		posy2 = pMapInfo->nSegmentZ + offsetY[i];

		GetWorldSectorInfo( &pFindMapSelf[ i ] , pMapInfo->pSector , posx2 , posy2 );
	}


	// Åë¸ÊÃ³¸®.
	maskedindex = __GetTongMapIndex( maskedindex , pMapInfo->pSector , pMapInfo->nSegmentX , pMapInfo->nSegmentZ );
	basetile	= __GetTongMapIndex( basetile , pMapInfo->pSector , pMapInfo->nSegmentX , pMapInfo->nSegmentZ );

	// ¹Ù²ÙÀÚ!
	// ÀüºÎ´Ù Â÷ÀÖ´Â°Å´Ï±î. 
	UINT32	nAlphaLayer = pMapInfo->pSegment->pIndex[ TD_THIRD ];

//	if( IsSameTile( pMapInfo->pSegment->pIndex[ TD_FIRST ] , maskedindex ) &&
//		pMapInfo->pSegment->pIndex[ TD_THIRD ] != ALEF_TEXTURE_NO_TEXTURE )
	{
		// ¹ÙÅ¹ÀÌ ±ò³à¼®°ú °°°í , ±×À§¿¡ ¾ËÆÄ°¡ ¸ÔÇô ÀÖ´Â °æ¿ì..
		// ÀÏ¹ÝÀûÀÎ °æ¿ì..
		m_UndoManager.AddTileActionUnit( pMapInfo->pSector , pMapInfo->nSegmentX , pMapInfo->nSegmentZ ,
			maskedindex
			, 
			ALEF_TEXTURE_NO_TEXTURE		
			,
			ALEF_TEXTURE_NO_TEXTURE
			,
			ALEF_TEXTURE_NO_TEXTURE
			,
			ALEF_TEXTURE_NO_TEXTURE
			,
			pMapInfo->pSegment->pIndex[ TD_SIXTH ] );
		AGCMMAP_THIS->D_SetTile( pMapInfo->pSector , pMapInfo->pSector->GetCurrentDetail() ,
			pMapInfo->nSegmentX , pMapInfo->nSegmentZ ,
			maskedindex
			, 
			ALEF_TEXTURE_NO_TEXTURE		
			,
			ALEF_TEXTURE_NO_TEXTURE
			,
			ALEF_TEXTURE_NO_TEXTURE
			,
			ALEF_TEXTURE_NO_TEXTURE
			,
			pMapInfo->pSegment->pIndex[ TD_SIXTH ] );

		// ÁÖÀ§ Á¤º¸ º¯°æ..

		#define MAX_PATTERN	3
		// ¸¶°íÀÚ (2004-05-28 ¿ÀÈÄ 9:47:46) : ÇÑ°³´Â 4Å¸ÀÏ ¾ËÆÄ Ã³¸®¿ë..ÀÓ½Ã ¹öÆÛ.
		stAlphaCalcurate	TilePattern[ MAX_PATTERN + 1 ];
		//INT32				nMaskLayer	;
		INT32				j			;
		stAlphaCalcurate	TilePatternTmp;
		INT32				nLastLayer	= 0;

		for ( int i = 0; i < 9; i++ )
		{
			if( i == 4 ) continue;

			if ( pFindMapSelf[ i ].pSector	)
			{
				// Å¸ÀÏ Á¤º¸ ¼öÁý..
				nLastLayer	= 0;
				TilePattern[ 0 ].uTileIndex	= pFindMapSelf[ i ].pSegment->pIndex[ TD_FIRST ];
				TilePattern[ 0 ].uPortion	= 0x0f;	// ²ËÂü..

				TilePattern[ 1 ].uTileIndex	= pFindMapSelf[ i ].pSegment->pIndex[ TD_THIRD ];
				if( TilePattern[ 1 ].uTileIndex )
				{
					TilePattern[ 1 ].uPortion	= aConvertTable_OD[ GET_TEXTURE_OFFSET( pFindMapSelf[ i ].pSegment->pIndex[ TD_SECOND ] )];
					TilePattern[ 0 ].uPortion	&= ~TilePattern[ 1 ].uPortion		;
					nLastLayer = 1;
				}
				else
					TilePattern[ 1 ].uPortion	= 0;

				TilePattern[ 2 ].uTileIndex	= pFindMapSelf[ i ].pSegment->pIndex[ TD_FIFTH ];
				if( TilePattern[ 2 ].uTileIndex )
				{
					TilePattern[ 2 ].uPortion	= aConvertTable_OD[ GET_TEXTURE_OFFSET( pFindMapSelf[ i ].pSegment->pIndex[ TD_FOURTH ] )];
					TilePattern[ 0 ].uPortion	&= ~TilePattern[ 2 ].uPortion		;
					nLastLayer	= 2;
				}
				else
					TilePattern[ 2 ].uPortion	= 0;
				
				// °è»ê¿ë..
				TilePattern[ 3 ].Empty();

				if( IsSameTile( TilePattern[ 0 ].uTileIndex , maskedindex ) && 
					TilePattern[ 1 ].uTileIndex != ALEF_TEXTURE_NO_TEXTURE )
				{
					if( TilePattern[ 2 ].uTileIndex != ALEF_TEXTURE_NO_TEXTURE )
					{
						TilePatternTmp		= TilePattern[ 0 ]	;

						TilePattern[ 0 ]	= TilePattern[ 1 ]	;
						TilePattern[ 1 ]	= TilePattern[ 2 ]	;

						// µ¤Èú°Ô º£ÀÌ½º Å¸ÀÏÀÌ¶ó¸é..À§¾Æ·¡¸¦ ¹Ù²Û´Ù..
						TilePattern[ 2 ]	= TilePatternTmp	;
					}
					else
					{
						// µ¤Èú°Ô º£ÀÌ½º Å¸ÀÏÀÌ¶ó¸é..À§¾Æ·¡¸¦ ¹Ù²Û´Ù..
						TilePatternTmp		= TilePattern[ 0 ]	;
						TilePattern[ 0 ]	= TilePattern[ 1 ]	;
						TilePattern[ 1 ]	= TilePatternTmp	;
					}
				}

				// ¸¶½ºÅ© ·¹ÀÌ¾î¸¦ Ã£¾Æ³¿..
				for( j = 0 ; j <= nLastLayer ; ++j )
				{
					if( IsSameTile( TilePattern[ j ].uTileIndex , maskedindex ) )
					{
						break;
					}
				}

				if( j != nLastLayer + 1 )
				{
					TilePattern[ j ].uPortion	|= offsetPortion[ i ];

					if( TilePattern[ j ].uPortion == 0x0f )
					{
						// ´Ù¾ø¿£´Ù..
						TilePattern[ 0 ].uTileIndex	= TilePattern[ j ].uTileIndex	;
						TilePattern[ 0 ].uPortion	= 0x0f							;

						TilePattern[ 1 ].Empty();
						TilePattern[ 2 ].Empty();

						nLastLayer	= 0;
					}
					else
					{
						TilePatternTmp		= TilePattern[ j ]	;

						for( ; j < nLastLayer ; j ++ )
						{
							TilePattern[ j ] = TilePattern[ j + 1 ];
						}

						TilePattern[ j ] = TilePatternTmp;
					}
				}
				else
				{
					// Áßº¹µÇ´Â°Ô ¾ø´Ù..
					// ºó°Å Ã£±â..
					for( j = 0 ; j <= nLastLayer ; ++j )
					{
						if( TilePattern[ j ].uTileIndex == ALEF_TEXTURE_NO_TEXTURE )
						{
							break;
						}
					}

					// °°Àº°Å ¾ø´Ù.. Æ÷¼Ç Á¦ÀÏ ³·Àº³à¼® ¾ø¿¡±â ÀÛ¾÷..
					// ÀÏ´Ü ±×³É ¸¶Áö¸·°Å ¾ø¿¥..
					TilePattern[ j ].uTileIndex	= maskedindex		;
					TilePattern[ j ].uPortion	= offsetPortion[ i ];
					nLastLayer	= j;
				}

				// ¾ËÆÄ°¡ °ãÄ¡´Â °æ¿ì , ±ò¸®´Â ³à¼®ÀÌ ÀÖ´Â°æ¿ì Á¦°ÅÇÑ´Ù.
				for( j = 0 ; j < nLastLayer ; ++j )
				{
					if( TilePattern[ j ].uPortion &  (~TilePattern[ nLastLayer ].uPortion) )
					{
						// do nothing.
					}
					else
						break;
				}

				if( j != nLastLayer )
				{
					// ¾îÇã -_- ±×³ð »èÁ¦..
					// µ¤¾î½á¹ö¸²..
					for( ; j < nLastLayer ; j ++ )
					{
						TilePattern[ j ] = TilePattern[ j + 1 ];
					}

					TilePattern[ j ].Empty();
				}
				else
				{
					if( nLastLayer == 3 )
					{
						// ÀÌ°æ¿ì 4¼½ÅÍ ¾ËÆÄ..
						// ¾îÂ¿¼ö ¾øÀÌ.. °¡Àå Áß¿äµµ°¡ ³·¾Æ º¸ÀÌ´Â ·¹ÀÌ¾î ÇÏ³ª¸¦ Á¦°ÅÇÑ´Ù.
						// Áö±ÝÀº Á¶°Ç¾øÀÌ 1 ·¹ÀÌ¾î¸¦ Á¦°Å..ÇÑ´Ù.

//						for( j = 1 ; j < nLastLayer ; j ++ )
//						{
//							TilePattern[ j ] = TilePattern[ j + 1 ];
//						}
//						nLastLayer = 2;
//
//						TilePattern[ j ].Empty();

						// ¸¶°íÀÚ (2004-06-01 ¿ÀÈÄ 12:37:48) : ´Ù ¾ø¿¡¹ö¸²..
						TilePattern[ 0 ].uTileIndex	= 0x01010280		;// RoughMap Tile
						TilePattern[ 0 ].uPortion	= 0x0f						;

						TilePattern[ 1 ].Empty();
						TilePattern[ 2 ].Empty();

						nLastLayer	= 0;

					}
					else
					{
						// ¿ä±â°¡ º° ¹®Á¦ ¾øÀ½..
					}
				
				}


//				if( TilePattern[ 0 ].uPortion != 0x0f )
//				{
//					// ¹Ù´ÚÀÌ ¹Ù²ð°æ¿ì Ã³¸® 
//					TilePattern[ 1 ].uPortion	&= ~TilePattern[ 0 ].uPortion;
//					TilePattern[ 2 ].uPortion	&= ~TilePattern[ 0 ].uPortion;
//					TilePattern[ 0 ].uPortion = 0x0f;
//				}

				// Æ÷¼ÇÀ» Å¸ÀÏÀÎµ¦½º·Î º¯È¯..
				for( int k = 0 ; k <= nLastLayer ; ++ k )
				{
					if( TilePattern[ k ].uTileIndex )
					{
						TilePattern[ k ].uPortion		=
							UPDATE_TEXTURE_OFFSET( alphaindex , aConvertTable_DO[ TilePattern[ k ].uPortion ] );
						TilePattern[ k ].uTileIndex	=
							__GetTongMapIndex( TilePattern[ k ].uTileIndex , pFindMapSelf[ i ].pSector , pFindMapSelf[ i ].nSegmentX , pFindMapSelf[ i ].nSegmentZ );
					}
					else
					{
						TilePattern[ k ].uPortion		= ALEF_TEXTURE_NO_TEXTURE;
					}
				}

				// Å¸ÀÏ ¼¼ÆÃ..
				m_UndoManager.AddTileActionUnit( pFindMapSelf[ i ].pSector , pFindMapSelf[ i ].nSegmentX , pFindMapSelf[ i ].nSegmentZ ,
					TilePattern[ 0 ].uTileIndex	,
					TilePattern[ 1 ].uPortion	,
					TilePattern[ 1 ].uTileIndex	,
					TilePattern[ 2 ].uPortion	,
					TilePattern[ 2 ].uTileIndex	,
					ALEF_TEXTURE_NO_TEXTURE
					//pFindMapSelf[ i ].pSegment->pIndex[ TD_SIXTH	]
					);
				AGCMMAP_THIS->D_SetTile( pFindMapSelf[ i ].pSector , pFindMapSelf[ i ].pSector->GetCurrentDetail() ,
					pFindMapSelf[ i ].nSegmentX , pFindMapSelf[ i ].nSegmentZ ,
					TilePattern[ 0 ].uTileIndex	,
					TilePattern[ 1 ].uPortion	,
					TilePattern[ 1 ].uTileIndex	,
					TilePattern[ 2 ].uPortion	,
					TilePattern[ 2 ].uTileIndex	,
					ALEF_TEXTURE_NO_TEXTURE		
					//pFindMapSelf[ i ].pSegment->pIndex[ TD_SIXTH	]
					);
			}
		}
	}

	return FALSE;
}

BOOL		MainWindow::ApplyFill					(stFindMapInfo *pMapInfo, int basetile , int replaceindex , BOOL bReculsive , BOOL bFirst )
{
	static int offsetX[9] = {+1, 0, -1, +1, 0, -1, +1, 0, -1};
	static int offsetY[9] = {+1, +1, +1, 0, 0, 0, -1, -1, -1};

	int	posx2 , posy2;

	// ÇÏÇÏÇÏ! -_-+

	// °Ë»ç ÇÑ°÷ÀÎÁö Ã¼Å©.
	if( pMapInfo->pSegment->uCheckTime == ( UINT16 ) g_pEngine->GetTickCount() )
		return TRUE;

	pMapInfo->pSegment->uCheckTime = ( UINT16 ) g_pEngine->GetTickCount();

	// °Ë»çÇÑ ÇÃ·¡±×¸¦ ³²±è.
	// ÁÖÀ§ 9°³ÀÇ Á¤º¸¸¦ ¾ò¾î³¿.
	// ÀÚ½ÅÀÇ ÅØ½ºÃÄ¸¦ °áÁ¤ÇÔ.
	// 9°³¿¡ °¢°¢ ¸®ÄÃ½Ãºê ÄÝÀ» ³Ñ±è.
	// 
	if( bFirst )
	{
		// ¾îÇã -_-;;
		basetile = pMapInfo->pSegment->pIndex[ TD_FIRST ];
	}

	replaceindex = g_pMainFrame->m_Document.GetSelectedTileIndex();

	stFindMapInfo pFindMapSelf[ 9 ];
	int i;
	for ( i = 0; i < 9; i++ )
	{
		if( i == 4 )
		{
			continue;
		}

		posx2 = pMapInfo->nSegmentX + offsetX[i];
		posy2 = pMapInfo->nSegmentZ + offsetY[i];

		GetWorldSectorInfo( &pFindMapSelf[ i ] , pMapInfo->pSector , posx2 , posy2 );
	}

	for ( i = 0; i < 9; i++ )
	{
		if( i == 4 ) continue;

		if (	pFindMapSelf[ i ].pSector 
			&&	pFindMapSelf[ i ].pSector == pMapInfo->pSector // ¼½ÅÍ ¹üÀ§¸¦ ³Ñ¾î°¡Áø ¸øÇÏ°Ô..
			)
		{
			// ÀÌ Å¸ÀÏÀÌ ¾Æ´Ï¸é...
			if(	pFindMapSelf[ i ].pSegment->pIndex[ TD_SIXTH	] == ALEF_TEXTURE_NO_TEXTURE	&&
				pFindMapSelf[ i ].pSegment->pIndex[ TD_FIFTH	] == ALEF_TEXTURE_NO_TEXTURE	&&
				pFindMapSelf[ i ].pSegment->pIndex[ TD_FOURTH	] == ALEF_TEXTURE_NO_TEXTURE	&&
				pFindMapSelf[ i ].pSegment->pIndex[ TD_THIRD	] == ALEF_TEXTURE_NO_TEXTURE	&&
				pFindMapSelf[ i ].pSegment->pIndex[ TD_SECOND	] == ALEF_TEXTURE_NO_TEXTURE	&&
				GET_TEXTURE_OFFSET_MASKING_OUT ( pFindMapSelf[ i ].pSegment->pIndex[ TD_FIRST	] ) == GET_TEXTURE_OFFSET_MASKING_OUT ( basetile ) )
			{
				if( bReculsive ) 
					ApplyFill( & pFindMapSelf[ i ] , basetile , replaceindex , bReculsive , FALSE );
				else if( bFirst )
					ApplyFill( & pFindMapSelf[ i ] , basetile , replaceindex , bReculsive , FALSE );
			}
		}
	}

	// ¹Ù²ÙÀÚ!

	replaceindex = __GetTongMapIndex( replaceindex , pMapInfo->pSector , pMapInfo->nSegmentX , pMapInfo->nSegmentZ );

	m_UndoManager.AddTileActionUnit( pMapInfo->pSector , pMapInfo->nSegmentX , pMapInfo->nSegmentZ ,
		replaceindex);
	AGCMMAP_THIS->D_SetTile( pMapInfo->pSector , pMapInfo->pSector->GetCurrentDetail() ,
		pMapInfo->nSegmentX , pMapInfo->nSegmentZ ,
		replaceindex);

	return TRUE;
}

BOOL MainWindow::OnApplyFill( )
{
	RsMouseStatus *ms = & m_PrevMouseStatus;

	// ¿ì¼± Å¬¸¯µÅ´Â ¾ÆÅä¹ÍÀÌ Á¸ÀçÇÏ´ÂÁö Á¡°Ë.

	static bool bFirst = true;
	if( bFirst )
	{
		DisplayMessage( AEM_NOTIFY , "Âü°í : ÇÊ±â´ÉÀº ¼½ÅÍ¸¦ ³ÑÁö ¸øÇÕ´Ï´Ù." );
		bFirst = false;
	}

	// ÀÌ m_pFoundSector´Â Idle Time ¸¶´Ù Ã¼Å©ÇØ¼­ ÀúÀåµÅ´Â ³à¼®ÀÌ´Ù.
	if( m_pCurrentGeometry )
	{
		ApDetailSegment	*	pSelectedSegment	;
		int				posx, posz		;

		pSelectedSegment = GetCurrentPolygonsTileIndex(
			m_pCurrentGeometry		,
			m_nFoundPolygonIndex	,
			( INT32 ) ms->pos.x		,
			( INT32 ) ms->pos.y		,
			&posx					,
			&posz					);

		if( pSelectedSegment == NULL ) return TRUE;

		stFindMapInfo	FindMapInfo		;

		switch( GetCurrentMode() )
		{
		case EDITMODE_TILE:
			{
				switch( g_pMainFrame->m_Document.m_nDWSectorSetting )
				{
				case DWSECTOR_ALLROUGH			:
					{					
							break;
					}
					break;
				case DWSECTOR_EACHEACH			:
				case DWSECTOR_ALLDETAIL			:
				case DWSECTOR_ACCORDINGTOCURSOR	:

					{
						ApDetailSegment * pSegment;

						pSegment = m_pCurrentGeometry->D_GetSegment( posx , posz );

						if( pSegment )
						{
							// ¾Æ½Î;;
							if( GetWorldSectorInfo( &FindMapInfo , m_pCurrentGeometry , posx , posz ) )
							{
								m_UndoManager.StartActionBlock( CUndoManager::TILE );

								// ³»ºÎÀûÀ¸·Î ¶ôÀ»ÇÑ´Ù.
								ApplyFill( &FindMapInfo , 0 , 0 , TRUE );

								UnlockSectors();


								m_UndoManager.EndActionBlock();

							}
						}

					} /// case
					break;
				}// switch

			}
			break;
		}
	}
	
	return FALSE;
}

void  MainWindow::AxisCreate()
{
	RwReal LineListData[6][5] = 
	{
		{ 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},
		{ 0.100f,  .000f,  0.000f,  1.000f,  1.000f},
    
		{ 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
		{ 0.000f,  0.100f,  0.000f,  1.000f,  1.000f},

		{ 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
		{ 0.000f,  0.000f,  0.100f,  1.000f,  1.000f},
	};
	for(UINT8 i=0; i<6; i++)
    {
        RwIm3DVertexSetPos(&m_pLineList[i], 
            LineListData[i][0], LineListData[i][1], LineListData[i][2]);
        RwIm3DVertexSetU(&m_pLineList[i], LineListData[i][3]);    
        RwIm3DVertexSetV(&m_pLineList[i], LineListData[i][4]);  
    }

	RwRGBA SolidColor0 = {255, 255, 255, 255};
	RwRGBA SolidColor1 = {200,  64,  64, 255};
	RwRGBA SolidColor2 = {64,  200,  64, 255};
	RwRGBA SolidColor3 = {64,  64,  200, 255};
    
    /*
     * Every line changes from Red to White...
     */

    RwIm3DVertexSetRGBA(&m_pLineList[0], SolidColor1.red, SolidColor1.green,
        SolidColor1.blue, SolidColor1.alpha);

    RwIm3DVertexSetRGBA(&m_pLineList[1], SolidColor0.red, SolidColor0.green,
        SolidColor0.blue, SolidColor0.alpha);   
	
	RwIm3DVertexSetRGBA(&m_pLineList[2], SolidColor2.red, SolidColor2.green,
        SolidColor2.blue, SolidColor2.alpha);  

	RwIm3DVertexSetRGBA(&m_pLineList[3], SolidColor0.red, SolidColor0.green,
        SolidColor0.blue, SolidColor0.alpha);  

	RwIm3DVertexSetRGBA(&m_pLineList[4], SolidColor3.red, SolidColor3.green,
        SolidColor3.blue, SolidColor3.alpha);  

	RwIm3DVertexSetRGBA(&m_pLineList[5], SolidColor0.red, SolidColor0.green,
        SolidColor0.blue, SolidColor0.alpha);      
	
	return;
}


void MainWindow::AxisRender()
{
	RwUInt32 transformFlags;
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    transformFlags = rwIM3D_ALLOPAQUE;
	
	//Line-listÀÇ Matrix 
	RwMatrix transform; RwMatrixSetIdentity( &transform );
	RwV3d	 LineListPos;
	RwV3dAdd( &LineListPos, RwMatrixGetPos(RwFrameGetMatrix(RwCameraGetFrame( g_pEngine->m_pCamera))), 
	RwMatrixGetAt(RwFrameGetMatrix(RwCameraGetFrame( g_pEngine->m_pCamera))) );
	
	RwMatrixTranslate( &transform, &LineListPos, rwCOMBINEPOSTCONCAT);

	if( RwIm3DTransform(m_pLineList, 6, &transform, transformFlags) )
    {                         
        RwIm3DRenderPrimitive( rwPRIMTYPELINELIST );        
        
        RwIm3DEnd();
    }

    return;	
}

void	MainWindow::SectorCollisionCheck		( RsMouseStatus * pPos		)
{
	if( NULL == g_pEngine->m_pCamera	) return;
	if( NULL == AGCMMAP_THIS			) return;
	
	// ÇöÀç Ä¿¼­ À§Ä¡¿¡ ÀÖ´Â Æú¸®°ÇÀ» ¾Ë¾Æ³½´Ù.
	RwLine			pixelRay		;
	RwV2d			pos				;

	pos				= pPos->pos		;
	RwCameraCalcPixelRay( g_pEngine->m_pCamera , &pixelRay , &pos );

	/****** Parn ´Ô ÀÛ¾÷ ½ÃÀÛ ********/
	m_PixelRayEnd = pixelRay.end;
	/****** Parn ´Ô ÀÛ¾÷ ³¡ ********/

	// ÀÌÀü À§Ä¡ ÀúÀåÇØµÒ..
	ApWorldSector * pPrevSector = m_pCurrentGeometry;
	m_pCurrentGeometry		= NULL	;
	m_nFoundPolygonIndex	= 0		;


	RwV3d	prevPos	= m_Position;

	// ¸¶°íÀÚ (2005-04-12 ¿ÀÈÄ 12:00:50) : 
	m_pCurrentGeometry = AGCMMAP_THIS->GetMapPositionFromMousePosition_tool(
		( INT32 ) pPos->pos.x , ( INT32 ) pPos->pos.y , &m_Position.x , &m_Position.y , &m_Position.z , &m_nFoundPolygonIndex );

	if( m_pCurrentGeometry )
	{
		if(
				prevPos.x	!= m_Position.x	||
				prevPos.y	!= m_Position.y	||
				prevPos.z	!= m_Position.z	
		)
			g_pMainFrame->m_pTileList->m_pOthers->MouseMoveGeometry( &m_Position	);

	}
	else
		g_pMainFrame->m_pTileList->m_pOthers->MouseMoveGeometry( NULL			);

	

	// ÁÂÇ¥¸¦ Ã£´Â´Ù.
	int					posX , posZ	;
	ApDetailSegment	*	pSegment	;
	BOOL				bUpdateMMap	= FALSE;


	if( m_pCurrentGeometry && ( pSegment = m_pCurrentGeometry->D_GetSegment( m_Position.x , m_Position.z , &posX , &posZ ) ) )
	{
		if( m_nCurrentTileIndexX != posX || m_nCurrentTileIndexZ != posZ )
		{
			m_nCurrentTileIndexX	= posX;
			m_nCurrentTileIndexZ	= posZ;
			bUpdateMMap				= TRUE;
		}
	}
	else
	{
		m_nCurrentTileIndexX	= -1;
		m_nCurrentTileIndexZ	= -1;
	}


	// ¼½ÅÍ ÀÌµ¿ Ã¼Å©.
	if( m_pCurrentGeometry != pPrevSector )
	{
		OnChangeCurrentSector( pPrevSector , m_pCurrentGeometry );
		bUpdateMMap	= FALSE;
	}

	if( bUpdateMMap )
	{
		g_pMainFrame->PostMessage( WM_WORKINGSECTORCHANGE );
	}
}

void	MainWindow::DetailIdleCheck()
{
	if( m_pCurrentGeometry )
	switch( g_pMainFrame->m_Document.m_nDWSectorSetting )
	{
	case DWSECTOR_ALLROUGH			:
		{
			if( GetAsyncKeyState( VK_MENU ) < 0 )
			{
				if( m_pCurrentGeometry->GetCurrentDetail() == SECTOR_LOWDETAIL )
				{
					AGCMMAP_THIS->SetCurrentDetail( SECTOR_HIGHDETAIL );
				}
			}
			else
			{
				if( m_pCurrentGeometry->GetCurrentDetail() == SECTOR_HIGHDETAIL )
				{
					AGCMMAP_THIS->SetCurrentDetail( SECTOR_LOWDETAIL );
				}
			}
		}
		break;

	default:
		break;
	}
}

BOOL MainWindow::OnRButtonDblClk( RsMouseStatus *ms	)
{
	switch(  GetCurrentMode() )
	{
	case EDITMODE_OTHERS:
		{
			g_pMainFrame->m_pTileList->m_pOthers->RButtonDblClkGeometry( &m_Position );
		}
		break;

	case EDITMODE_OBJECT:
		{
			// ¿ÀºêÁ§Æ® ±×·ì Âï±â.
			// m_Position
			g_pMainFrame->OnObjectpopGroupLoad( &m_Position );
		}
		break;

	default:
		// do nothing.
		break;
	}

	return TRUE;
}

BOOL MainWindow::OnApplyTongMap				()
{
	RsMouseStatus *ms = & m_PrevMouseStatus;

	// ¿ì¼± Å¬¸¯µÅ´Â ¾ÆÅä¹ÍÀÌ Á¸ÀçÇÏ´ÂÁö Á¡°Ë.

	// ÀÌ m_pFoundSector´Â Idle Time ¸¶´Ù Ã¼Å©ÇØ¼­ ÀúÀåµÅ´Â ³à¼®ÀÌ´Ù.
	if( m_pCurrentGeometry )
	{
		ApDetailSegment	*	pSelectedSegment	;
		int				posx, posz		;

		pSelectedSegment = GetCurrentPolygonsTileIndex(
			m_pCurrentGeometry		,
			m_nFoundPolygonIndex	,
			( INT32 ) ms->pos.x		,
			( INT32 ) ms->pos.y		,
			&posx					,
			&posz					);

		if( pSelectedSegment == NULL ) return TRUE;

		stFindMapInfo	FindMapInfo		;

		switch( GetCurrentMode() )
		{
		case EDITMODE_TILE:
			{
				switch( g_pMainFrame->m_Document.m_nDWSectorSetting )
				{
				case DWSECTOR_EACHEACH			:
				case DWSECTOR_ACCORDINGTOCURSOR	:
					{					
					}
					break;
				case DWSECTOR_ALLROUGH			:
				case DWSECTOR_ALLDETAIL			:

					{
						switch( GetCurrentLayer() )
						{
						case TILELAYER_BACKGROUND	:
						case TILELAYER_TONGMAP		:

							{
								ApDetailSegment * pSegment;

								pSegment = m_pCurrentGeometry->D_GetSegment( posx , posz );

								if( pSegment )
								{
									// ¾Æ½Î;;

									int	nIndex = g_pMainFrame->m_Document.GetSelectedTileIndex();

									int	dim = GET_TEXTURE_DIMENSION( nIndex );

									int x , z;

									m_UndoManager.StartActionBlock( CUndoManager::TILE );

									for( z = 0 ; z < dim ; z ++ )
									{
										for( x = 0 ; x < dim ; x ++ )
										{
											if( GetWorldSectorInfo( &FindMapInfo , m_pCurrentGeometry , posx + x , posz + z ) )
											{
												m_UndoManager.AddTileActionUnit( FindMapInfo.pSector , 
													FindMapInfo.nSegmentX , FindMapInfo.nSegmentZ ,
													UPDATE_TEXTURE_OFFSET( nIndex , x + z * dim ) );
												AGCMMAP_THIS->D_SetTile(
													FindMapInfo.pSector			,
													FindMapInfo.nCurrentDetail	,
													FindMapInfo.nSegmentX		,
													FindMapInfo.nSegmentZ		,
													UPDATE_TEXTURE_OFFSET( nIndex , x + z * dim ) );
											}
										}
									}

									UnlockSectors();
									m_UndoManager.EndActionBlock();

								}
							}
							break;
						case TILELAYER_UPPER			:
							{
								ApDetailSegment * pSegment;

								pSegment = m_pCurrentGeometry->D_GetSegment( posx , posz );

								if( pSegment )
								{
									// ¾Æ½Î;;

									int	nIndex = g_pMainFrame->m_Document.GetSelectedTileIndex();

									int	dim = GET_TEXTURE_DIMENSION( nIndex );

									int x , z;

									m_UndoManager.StartActionBlock( CUndoManager::TILE );

									for( z = 0 ; z < dim ; z ++ )
									{
										for( x = 0 ; x < dim ; x ++ )
										{
											if( GetWorldSectorInfo( &FindMapInfo , m_pCurrentGeometry , posx + x , posz + z ) )
											{

												m_UndoManager.AddTileActionUnit(
													FindMapInfo.pSector ,
													FindMapInfo.nSegmentX ,
													FindMapInfo.nSegmentZ ,
													FindMapInfo.pSegment->pIndex[ TD_FIRST	] ,
													FindMapInfo.pSegment->pIndex[ TD_SECOND	] ,
													FindMapInfo.pSegment->pIndex[ TD_THIRD	] ,
													FindMapInfo.pSegment->pIndex[ TD_FOURTH	] ,
													FindMapInfo.pSegment->pIndex[ TD_FIFTH	] ,
													UPDATE_TEXTURE_OFFSET( nIndex , x + z * dim ) ) ;

												AGCMMAP_THIS->D_SetTile(
													FindMapInfo.pSector			,
													FindMapInfo.nCurrentDetail	,
													FindMapInfo.nSegmentX		,
													FindMapInfo.nSegmentZ		,

													FindMapInfo.pSegment->pIndex[ TD_FIRST	] ,
													FindMapInfo.pSegment->pIndex[ TD_SECOND	] ,
													FindMapInfo.pSegment->pIndex[ TD_THIRD	] ,
													FindMapInfo.pSegment->pIndex[ TD_FOURTH	] ,
													FindMapInfo.pSegment->pIndex[ TD_FIFTH	] ,
													UPDATE_TEXTURE_OFFSET( nIndex , x + z * dim ) );
											}
										}
									}

									UnlockSectors();
									m_UndoManager.EndActionBlock();

								}

							}
							break;
						default:
							{
							}
							break;
						}
						// ¿¡ÇàÇà..




					} /// case
					break;
				}// switch

			}
			break;
		}
	}
	
	return TRUE;
}

BOOL	MainWindow::OnAppyWater	()
{
	// ¸¶°íÀÚ (2005-04-04 ¿ÀÈÄ 3:57:38) : 
	// ¸¸µæ¾ÆÀú¾¾ÀÇ ¿äÃ»À¸·Î »èÁ¦.

	/*
	RsMouseStatus *ms = & m_PrevMouseStatus;

	// ¿ì¼± Å¬¸¯µÅ´Â ¾ÆÅä¹ÍÀÌ Á¸ÀçÇÏ´ÂÁö Á¡°Ë.
	// ÀÌ m_pFoundSector´Â Idle Time ¸¶´Ù Ã¼Å©ÇØ¼­ ÀúÀåµÅ´Â ³à¼®ÀÌ´Ù.
	if( m_pCurrentGeometry )
	{
		ApDetailSegment	*	pSelectedSegment	;
		int				posx, posz		;

		pSelectedSegment = GetCurrentPolygonsTileIndex(
			m_pCurrentGeometry		,
			m_nFoundPolygonIndex	,
			( INT32 ) ms->pos.x		,
			( INT32 ) ms->pos.y		,
			&posx					,
			&posz					);

		if( pSelectedSegment == NULL ) return TRUE;

		//stFindMapInfo	FindMapInfo		;

		switch( GetCurrentMode() )
		{
		case EDITMODE_TILE:
			{
				ApDetailSegment * pSegment;

				FLOAT	xPos , zPos;

				pSegment = m_pCurrentGeometry->D_GetSegment( posx , posz , & xPos , & zPos );

				// ³ôÀÌ 
				FLOAT	height = m_pCurrentGeometry->D_GetHeight( m_Position.x , m_Position.z );

				// x, zÁÂÇ¥¿Í , height 

				//ASSERT( !"¿öÅÍ¸¦ Ãß°¡ÇÔ!" );

				//g_pcsAgcmWater->AddWater	( xPos - 500.0f , zPos-500.0f , 1000.0f,1000.f, height + 30.0f , WT_LAKE );
				g_pcsAgcmWater->AddWater	( xPos - 3500.0f , zPos-3500.0f , 7000.0f,7000.f, 2500.0f , WATER_RIVER );
			}
			break;
		default:
			ASSERT( !"Å¸ÀÏ¼öÁ¤¸ðµå¿¡¼­¸¸ ¹° Ãß°¡°¡ °¡´ÉÇØ¿ä" );
			break;
		}
	}
	*/
	return TRUE;	
}

void MainWindow::DrawAreaSphere	( FLOAT x , FLOAT z	, FLOAT fScale )
{
	DrawAreaSphere( x , AGCMMAP_THIS->GetHeight( x , z , SECTOR_MAX_HEIGHT ) , z , fScale );
}

void MainWindow::DrawAreaSphere	( FLOAT x , FLOAT y ,  FLOAT z , FLOAT fScale )
{
	RwFrame		*	pFrame	;
	RwV3d			scale	;
	RwV3d			pos		;

	
	VERIFY( pFrame		=	RpAtomicGetFrame( m_pAreaSphere ) );

	scale.x		=	fScale	;
	scale.y		=	fScale	;
	scale.z		=	fScale	;

	RwFrameScale		( pFrame , &scale	, rwCOMBINEREPLACE		);

	pos.x		=	x;
	pos.y		=	y;
	pos.z		=	z;

	RwFrameTranslate	( pFrame , &pos		, rwCOMBINEPOSTCONCAT	);

	RpAtomicRender( m_pAreaSphere );
}

void MainWindow::DrawPillar( FLOAT x , FLOAT z )
{
	float y = AGCMMAP_THIS->GetHeight( x , z , SECTOR_MAX_HEIGHT );
	float y2 = y + 10000.0f;
	//  8°¢Çü ÇÊ¶ó..
	float	fSize = 10.0f;
	float	fSiz2 = fSize * 2 ;

	float fsin = sin( 3.1415927f * ( float ) ( GetTickCount() ) / 300.0f );
	INT32 nAlpha = 96 + 32 + ( int ) ( 32 * fsin );
	
	RwIm3DVertex 		pLineList[ 16 ]	;
	RwImVertexIndex		pIndex[ 48 ]	;

	RwIm3DVertexSetPos	( &pLineList[ 0 ] , x + fSize , y , z + fSiz2 );
	RwIm3DVertexSetPos	( &pLineList[ 1 ] , x + fSiz2 , y , z + fSize );
	RwIm3DVertexSetPos	( &pLineList[ 2 ] , x + fSiz2 , y , z - fSize );
	RwIm3DVertexSetPos	( &pLineList[ 3 ] , x + fSize , y , z - fSiz2 );
	RwIm3DVertexSetPos	( &pLineList[ 4 ] , x - fSize , y , z - fSiz2 );
	RwIm3DVertexSetPos	( &pLineList[ 5 ] , x - fSiz2 , y , z - fSize );
	RwIm3DVertexSetPos	( &pLineList[ 6 ] , x - fSiz2 , y , z + fSize );
	RwIm3DVertexSetPos	( &pLineList[ 7 ] , x - fSize , y , z + fSiz2 );

	RwIm3DVertexSetPos	( &pLineList[ 8 ] , x + fSize , y2 , z + fSiz2 );
	RwIm3DVertexSetPos	( &pLineList[ 9 ] , x + fSiz2 , y2 , z + fSize );
	RwIm3DVertexSetPos	( &pLineList[ 10] , x + fSiz2 , y2 , z - fSize );
	RwIm3DVertexSetPos	( &pLineList[ 11] , x + fSize , y2 , z - fSiz2 );
	RwIm3DVertexSetPos	( &pLineList[ 12] , x - fSize , y2 , z - fSiz2 );
	RwIm3DVertexSetPos	( &pLineList[ 13] , x - fSiz2 , y2 , z - fSize );
	RwIm3DVertexSetPos	( &pLineList[ 14] , x - fSiz2 , y2 , z + fSize );
	RwIm3DVertexSetPos	( &pLineList[ 15] , x - fSize , y2 , z + fSiz2 );

	for( int i = 0 ; i < 16 ; i ++ )
	{
		RwIm3DVertexSetU	( &pLineList[ i ] , 1.0f );    
		RwIm3DVertexSetV	( &pLineList[ i ] , 1.0f );
		RwIm3DVertexSetRGBA	( &pLineList[ i ] , 0 , 255 , 0 , nAlpha );
	}

	for( int i = 0 ; i < 8 ; i ++ )
	{
		pIndex[ i * 6 + 0 ]	= ( 0 + i ) % 8;
		pIndex[ i * 6 + 1 ]	= ( 1 + i ) % 8;
		pIndex[ i * 6 + 2 ]	= 8 + i;
		pIndex[ i * 6 + 3 ]	= ( 1 + i ) % 8;
		pIndex[ i * 6 + 4 ]	= 8 +( 1 + i ) % 8;
		pIndex[ i * 6 + 5 ]	= 8 + i;
	}

	if( RwIm3DTransform( pLineList , 16 , NULL, rwIM3D_VERTEXRGBA ) )
	{
		RwIm3DRenderIndexedPrimitive( rwPRIMTYPETRILIST, pIndex , 48 );

		RwIm3DEnd();
	}
}

void	MainWindow::DrawGeometryBlocking		()
{
	return;

	// ÇöÀç ÁöÇüÀÇ Wire Frame À» ±×·ÁÁÜ.
	if( m_pCurrentGeometry && m_pCurrentGeometry->D_GetDepth() > 0 )
	{		
		int		depth	= ( int ) ( ( FLOAT ) m_pCurrentGeometry->D_GetDepth() );

		float	fX	, fZ;
		float	fX2	, fZ2;
		int		i	, j ;
		float	height	;
		int		offset	;

		ApDetailSegment * pSegment;
		int	NOT			= 0;
		int	NOV			= 0;

		for( j = 0 ; j < depth ; j ++ )
		{
			for( int i = 0 ; i < depth ; i ++ )
			{
				// °¢¼½ÅÍ °Ë»ç..
				pSegment = m_pCurrentGeometry->D_GetSegment( SECTOR_HIGHDETAIL , i , j );
				if( pSegment )
				{
					// ¼½ÅÍ°¡ Á¸ÀçÇÏ°í µðÅ×ÀÏ ¼½ÅÍ°¡ ¾Æ´Ï¸é!.
					NOT += 2 * 4 ;
					NOV += 6 * 4 ;
				}
			}
		}

		int		depthBlocking	= depth * 2;

		RwIm3DVertex	* pLineList		= new RwIm3DVertex		[ NOV		];
		RwImVertexIndex	* pIndexPolygon	= new RwImVertexIndex	[ NOT * 3	];


		float	fStepSize			= m_pCurrentGeometry->GetStepSizeX();
		float	fStepSizeBlocking	= fStepSize / 2.0f					;

		int		indexVertex		= 0;
		int		indexTriangle	= 0;
		int		nBlock	;

		for( j = 0 ; j < depth ; j ++ )
		{
			for( i = 0 ; i < depth ; i ++ )
			{
				pSegment = m_pCurrentGeometry->D_GetSegment( SECTOR_HIGHDETAIL , i , j );

				if( pSegment )
				{
					for( offset = 0; offset < 4 ; ++ offset )
					{
						// Æú¸®°Ç 4 * 2 °³ »ý¼º..
						// ½º»çÆÃ Æ÷Áö¼Ç...
						nBlock	= pSegment->stTileInfo.GetBlocking( offset );
						fX = m_pCurrentGeometry->GetXStart() + ( FLOAT ) i * fStepSize + ( ( offset % 2 ) ? ( fStepSizeBlocking ) : 0.0f );
						fZ = m_pCurrentGeometry->GetZStart() + ( FLOAT ) j * fStepSize + ( ( offset / 2 ) ? ( fStepSizeBlocking ) : 0.0f );

						fX2	= fX + fStepSizeBlocking * 0.0f;
						fZ2	= fZ + fStepSizeBlocking * 0.0f;
						
						height = m_pCurrentGeometry->D_GetHeight( fX2 , fZ2 );

						RwIm3DVertexSetPos( &pLineList[ indexVertex ] , 
							fX2,
							height + ALEF_SECTOR_WIDTH / 500.0f,
							fZ2);

						RwIm3DVertexSetU( &pLineList[ indexVertex ] , 1.0f );    
						RwIm3DVertexSetV( &pLineList[ indexVertex ] , 1.0f );

						switch( nBlock )
						{
						case	TBF_GEOMETRY	:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 0   , 255 , 0   , 32 );	break;
						case	TBF_OBJECT		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 0   , 0   , 32 );	break;
						default					:
						case	TBF_NONE		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 255 , 255 , 32 );	break;
						}

						pIndexPolygon[ indexTriangle++ ]	= indexVertex ++;

						fX2	= fX + fStepSizeBlocking * 1.0f;
						fZ2	= fZ + fStepSizeBlocking * 0.0f;
						
						height = m_pCurrentGeometry->D_GetHeight( fX2 , fZ2 );

						RwIm3DVertexSetPos( &pLineList[ indexVertex ] , 
							fX2,
							height + ALEF_SECTOR_WIDTH / 500.0f,
							fZ2);

						RwIm3DVertexSetU( &pLineList[ indexVertex ] , 1.0f );    
						RwIm3DVertexSetV( &pLineList[ indexVertex ] , 1.0f );

						switch( nBlock )
						{
						case	TBF_GEOMETRY	:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 0   , 255 , 0   , 32 );	break;
						case	TBF_OBJECT		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 0   , 0   , 32 );	break;
						default					:
						case	TBF_NONE		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 255 , 255 , 32 );	break;
						}

						pIndexPolygon[ indexTriangle++ ]	= indexVertex ++;
						
						fX2	= fX + fStepSizeBlocking * 0.0f;
						fZ2	= fZ + fStepSizeBlocking * 1.0f;
						
						height = m_pCurrentGeometry->D_GetHeight( fX2 , fZ2 );

						RwIm3DVertexSetPos( &pLineList[ indexVertex ] , 
							fX2,
							height + ALEF_SECTOR_WIDTH / 500.0f,
							fZ2);

						RwIm3DVertexSetU( &pLineList[ indexVertex ] , 1.0f );    
						RwIm3DVertexSetV( &pLineList[ indexVertex ] , 1.0f );

						switch( nBlock )
						{
						case	TBF_GEOMETRY	:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 0   , 255 , 0   , 32 );	break;
						case	TBF_OBJECT		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 0   , 0   , 32 );	break;
						default					:
						case	TBF_NONE		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 255 , 255 , 32 );	break;
						}

						pIndexPolygon[ indexTriangle++ ]	= indexVertex ++;
						
						fX2	= fX + fStepSizeBlocking * 0.0f;
						fZ2	= fZ + fStepSizeBlocking * 1.0f;
						
						height = m_pCurrentGeometry->D_GetHeight( fX2 , fZ2 );

						RwIm3DVertexSetPos( &pLineList[ indexVertex ] , 
							fX2,
							height + ALEF_SECTOR_WIDTH / 500.0f,
							fZ2);

						RwIm3DVertexSetU( &pLineList[ indexVertex ] , 1.0f );    
						RwIm3DVertexSetV( &pLineList[ indexVertex ] , 1.0f );

						switch( nBlock )
						{
						case	TBF_GEOMETRY	:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 0   , 255 , 0   , 32 );	break;
						case	TBF_OBJECT		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 0   , 0   , 32 );	break;
						default					:
						case	TBF_NONE		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 255 , 255 , 32 );	break;
						}

						pIndexPolygon[ indexTriangle++ ]	= indexVertex ++;
						
						fX2	= fX + fStepSizeBlocking * 1.0f;
						fZ2	= fZ + fStepSizeBlocking * 0.0f;
						
						height = m_pCurrentGeometry->D_GetHeight( fX2 , fZ2 );

						RwIm3DVertexSetPos( &pLineList[ indexVertex ] , 
							fX2,
							height + ALEF_SECTOR_WIDTH / 500.0f,
							fZ2);

						RwIm3DVertexSetU( &pLineList[ indexVertex ] , 1.0f );    
						RwIm3DVertexSetV( &pLineList[ indexVertex ] , 1.0f );

						switch( nBlock )
						{
						case	TBF_GEOMETRY	:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 0   , 255 , 0   , 32 );	break;
						case	TBF_OBJECT		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 0   , 0   , 32 );	break;
						default					:
						case	TBF_NONE		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 255 , 255 , 32 );	break;
						}

						pIndexPolygon[ indexTriangle++ ]	= indexVertex ++;

						fX2	= fX + fStepSizeBlocking * 1.0f;
						fZ2	= fZ + fStepSizeBlocking * 1.0f;
						
						height = m_pCurrentGeometry->D_GetHeight( fX2 , fZ2 );

						RwIm3DVertexSetPos( &pLineList[ indexVertex ] , 
							fX2,
							height + ALEF_SECTOR_WIDTH / 500.0f,
							fZ2);

						RwIm3DVertexSetU( &pLineList[ indexVertex ] , 1.0f );    
						RwIm3DVertexSetV( &pLineList[ indexVertex ] , 1.0f );

						switch( nBlock )
						{
						case	TBF_GEOMETRY	:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 0   , 255 , 0   , 32 );	break;
						case	TBF_OBJECT		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 0   , 0   , 32 );	break;
						default					:
						case	TBF_NONE		:	RwIm3DVertexSetRGBA( &pLineList[ indexVertex ] , 255 , 255 , 255 , 32 );	break;
						}

						pIndexPolygon[ indexTriangle++ ]	= indexVertex ++;
					}
					
				}

				
			}
		}

		if( RwIm3DTransform( pLineList , NOV , NULL, rwIM3D_VERTEXRGBA ) )
		{                         
			RwIm3DRenderIndexedPrimitive( rwPRIMTYPETRILIST,
				pIndexPolygon , NOT * 3 );

			RwIm3DEnd();
		}

		delete [] pLineList;
		delete [] pIndexPolygon;
	}

}

void	MainWindow::ObjectIdleBrushing()
{
	PROFILE("MainWindow::ObjectIdleBrushing");

	if( m_bObjectBrushing						== OBJECT_BRUSHING		&& 
		m_pCurrentGeometry												&&
		m_pCurrentGeometry->GetCurrentDetail()	== SECTOR_HIGHDETAIL	&&
		g_pMainFrame->m_pTileList->m_pObjectWnd->m_bUseBrush			)
	{
		ASSERT( g_pMainFrame->m_pTileList->m_pObjectWnd->IsObjectSelected() );

		// ¿ÀºêÁ§Æ® ºê·¯½Ì...

/*	
		¿ÀºêÁ§Æ® ºê·¯½ÌÀº ÀÏ´Ü ºê·¯½ÃÀÇ ¹üÀ§¸¦ Ã¼Å©ÇØ¼­ ,
		¾î´À¾î´À Å¸ÀÏ¿¡ ¿ÀºêÁ§Æ®°¡ »ðÀÔÀÌ µÉ °ÍÀÎ°¡¸¦ÆÇ´ÜÇÑ´Ù.
		±×¸®°í ±× Å¸ÀÏµéÀ» °Ë»çÇÏ¸é¼­ , ÇØ´çÅ¸ÀÏ ¾È¿¡ ÀÖ´Â¿ÀºêÁ§Æ®ÀÇ
		¹Ðµµ¸¦ ±¸ÇÑ´Ù. ±×¸®°í ¹Ðµµ°¡ ÀÏÁ¤ ÀÌÇÏ¸é , ·»´ýÇÑ Á¶°ÇÀ» »ý¼ºÇÏ¿©
		±× À§Ä¡¿¡ ¿ÀºêÁ§Æ®¸¦ »ðÀÔÇÑ´Ù..

		ALT¸¦ ´©¸£°í ÀÖ´Â°æ¿ì¿£ , ¿ÀºêÁ§Æ® »èÁ¦ ±â´ÉÀ» ½ÇÇàÇÑ´Ù..
		¿ª½Ã Å¸ÀÏ´ÜÀ§ °Ë»ç¸¦ Áß½ÉÀ¸·ÎÇÏ¸ç , ÇØ´çÅ¸ÀÏ¾È¿¡ ÀÖ´Â ¿ÀºêÁ§Æ®¸¦ »èÁ¦ÇÑ´Ù..
*/

		// ÁÂÇ¥¿¡¼­ r ¹üÀ§ ¾È¿¡ Á¸ÀçÇÏ´Â ¼½ÅÍ¸¦ Ã£¾Æ³½´Ù..
		// ºê·¯½Ã Àû¿ë..

		//int				i , j				;
		//float			fSegPosX, fSegPosZ	;
		//float			distance			;
		ApWorldSector *	pWorkingSector		;
		//ApDetailSegment	* pSegment			;
		RwV3d			vPos				;

		//AgcdObject *	pSectorGroup		;

		float			fDensity		= 0.0f	;
		float			fStepSize		= m_pCurrentGeometry->GetStepSizeX();
		float			fBrushRadius	= g_pMainFrame->m_Document.m_fBrushRadius;// / 2.0f * 1.41421356f;
		float			fDensityRequire			;

		// ¸ÓÆ¼¸®¾ó ÁØºñ..

		//int segmentx , segmentz;

		int x1 = ALEF_LOAD_RANGE_X1 , x2 = ALEF_LOAD_RANGE_X2 , z1 = ALEF_LOAD_RANGE_Y1 , z2 = ALEF_LOAD_RANGE_Y2;
		int	xc , zc;

		xc = m_pCurrentGeometry->GetArrayIndexX();
		zc = m_pCurrentGeometry->GetArrayIndexZ();

		x1 = xc - 1 ;
		z1 = zc - 1 ;
		x2 = xc + 1 ;
		z2 = zc + 1 ;

		fDensity	= 0.0f;
		{
			INT32 lIndex = 0;
			INT32 nTID = g_pMainFrame->m_pTileList->m_pObjectWnd->GetObjectTID();

			for (	ApdObject	* pcsObject = g_pcsApmObject->GetObjectSequence(&lIndex)	;
					pcsObject														;
					pcsObject = g_pcsApmObject->GetObjectSequence(&lIndex)	)
			{
				if( pcsObject->m_pcsTemplate->m_lID == nTID )
				{
					if(							
						( sqrt(	( pcsObject->m_stPosition.x - m_Position.x ) *
								( pcsObject->m_stPosition.x - m_Position.x )
								+
								( pcsObject->m_stPosition.z - m_Position.z ) *
								( pcsObject->m_stPosition.z - m_Position.z ) )
						)
						> g_pMainFrame->m_Document.m_fBrushRadius ) continue;

					fDensity += 1.0f;
				}
			}
		}

		/*
		for( j = z1 ; j <= z2 ; j ++ )
		{
			for( i = x1 ; i <= x2 ; i ++ )
			{
				// ¾ÆÀÚ Ã£¾Æº¸ÀÚ.
				pWorkingSector = g_pcsApmMap->GetSectorByArray( i , 0 , j );

				if( NULL == pWorkingSector ) continue;

				if( !__CheckCircumstanceSectorPolygonInfo( pWorkingSector ) ) continue;

				if( pWorkingSector->IsLoadedDetailData() &&
					AGCMMAP_THIS->IsInSectorRadius( g_pcsApmMap->GetSectorByArray( i , 0 , j ) ,
						m_Position.x , m_Position.z , g_pMainFrame->m_Document.m_fBrushRadius )
					)
				{
					INT32 nTID = g_pMainFrame->m_pTileList->m_pObjectWnd->GetObjectTID();

					ApWorldSector::Dimension * pDimension = pWorkingSector->GetDimension( 0 );
					for( ApWorldSector::IdPos *	pObject = pDimension->pObjects ; pObject ; pObject = pObject->pNext )
					{
						ApdObject *pstObject = g_pcsApmObject->GetObjectA( pObject->id );
						if( pstObject->m_pcsTemplate->m_lID == nTID )
						{
							if(							
								( sqrt(	( pstObject->m_stPosition.x - m_Position.x ) *
										( pstObject->m_stPosition.x - m_Position.x )
										+
										( pstObject->m_stPosition.z - m_Position.z ) *
										( pstObject->m_stPosition.z - m_Position.z ) )
								)
								> g_pMainFrame->m_Document.m_fBrushRadius ) continue;

							fDensity += 1.0f;
						}
					}
				////////////////////////////////
				}
			}
		}// ¼½ÅÍ Æ÷ ·çÇÁ..
		*/

		// Calcurate fDensityRequire
		fDensityRequire	=
			3.1415927f	*	( float )	pow( fBrushRadius / fStepSize , 2 )	*
			g_pMainFrame->m_pTileList->m_pObjectWnd->GetDensity();

		if( fDensity < fDensityRequire )
		{
			do
			{
				FLOAT	r1 , r2;
				r1	= frand();
				r2	= frand();

				char	str[ 256 ];
				sprintf( str , "%f , %f\n" , r1 , r2 );
				TRACE( str );

				vPos.x	= ( m_Position.x - fBrushRadius ) + 2.0f * fBrushRadius * r1	;
				vPos.z	= ( m_Position.z - fBrushRadius ) + 2.0f * fBrushRadius * r2	;
			}
			while(
				fBrushRadius	<
				
				sqrt(	( vPos.x - m_Position.x ) *
						( vPos.x - m_Position.x )
						+
						( vPos.z - m_Position.z ) *
						( vPos.z - m_Position.z ) )
			);

			vPos.y			= AGCMMAP_THIS->GetHeight( vPos.x , vPos.z , SECTOR_MAX_HEIGHT )	;
			pWorkingSector	= g_pcsApmMap->GetSector( vPos.x , vPos.z )	;

			AuPOS	pos		;
			AuPOS	scale	= { 1.0f , 1.0f , 1.0f };
			FLOAT	fXRot	= 0.0f;
			FLOAT	fYRot	= 0.0f;

			pos.x	= vPos.x;
			pos.y	= vPos.y;
			pos.z	= vPos.z;

			fXRot	=	g_pMainFrame->m_pTileList->m_pObjectWnd->GetRotateX();
			fYRot	=	g_pMainFrame->m_pTileList->m_pObjectWnd->GetRotateY();

			scale.x	=	g_pMainFrame->m_pTileList->m_pObjectWnd->GetScale();
			scale.y	=	g_pMainFrame->m_pTileList->m_pObjectWnd->GetScale();
			scale.z	=	g_pMainFrame->m_pTileList->m_pObjectWnd->GetScale();

			{
				ApdObject * pObject;
				VERIFY( pObject = g_pMainFrame->m_pTileList->m_pObjectWnd->AddObject(&vPos) );
				pObject->m_stScale = scale;
				pObject->m_fDegreeX = fXRot;
				pObject->m_fDegreeY = fYRot;

				g_pcsApmObject->UpdateInit( pObject );
			}
		}
	}
}


BOOL	GetWorldPosToScreenPos		( RwV3d * pWorldPos , POINT * pPoint )
{
	RwMatrix	* pViewMatrix	= RwCameraGetViewMatrix( g_MyEngine.m_pCamera );
	ASSERT( NULL != pViewMatrix );

    RwCamera            *camera;
    D3DMATRIX           viewMatrix;
    D3DMATRIX           projMatrix;
	D3DMATRIX			projviewMatrix;
	D3DXMatrixIdentity((D3DXMATRIX *)&projMatrix);

    /*
     * Save the camera pointer
     */
    camera = g_MyEngine.m_pCamera;
      
	RwV3d	* vec;
	vec		= RwMatrixGetRight	( pViewMatrix );
	viewMatrix.m[0][0] = vec->x;
	viewMatrix.m[0][1] = vec->y;
	viewMatrix.m[0][2] = vec->z;
	vec		= RwMatrixGetUp		( pViewMatrix );
	viewMatrix.m[1][0] = vec->x;
	viewMatrix.m[1][1] = vec->y;
	viewMatrix.m[1][2] = vec->z;
	vec		= RwMatrixGetAt		( pViewMatrix );
	viewMatrix.m[2][0] = vec->x;
	viewMatrix.m[2][1] = vec->y;
	viewMatrix.m[2][2] = vec->z;
	vec		= RwMatrixGetPos	( pViewMatrix );
	viewMatrix.m[3][0] = vec->x;
	viewMatrix.m[3][1] = vec->y;
	viewMatrix.m[3][2] = vec->z;
	
	viewMatrix.m[0][3] = 0.0f;
    viewMatrix.m[1][3] = 0.0f;
	viewMatrix.m[2][3] = 0.0f;
    viewMatrix.m[3][3] = 1.0f;

    /*
     * Projection matrix
     */
    projMatrix.m[0][0] = camera->recipViewWindow.x;
    projMatrix.m[1][1] = camera->recipViewWindow.y;

    /* Shear X, Y by view offset with Z invariant */
    projMatrix.m[2][0] =
        camera->recipViewWindow.x * camera->viewOffset.x;
    projMatrix.m[2][1] =
        camera->recipViewWindow.y * camera->viewOffset.y;

    /* Translate to shear origin */
    projMatrix.m[3][0] =
        -camera->recipViewWindow.x * camera->viewOffset.x;

    projMatrix.m[3][1] =
        -camera->recipViewWindow.y * camera->viewOffset.y;

    /* Projection type */
    if (camera->projectionType == rwPARALLEL)
    {
        projMatrix.m[2][2] =
            1.0f / (camera->farPlane - camera->nearPlane);
        projMatrix.m[2][3] = 0.0f;
        projMatrix.m[3][3] = 1.0f;
    }
    else
    {
        projMatrix.m[2][2] =
            camera->farPlane / (camera->farPlane - camera->nearPlane);
        projMatrix.m[2][3] = 1.0f;
        projMatrix.m[3][3] = 0.0f;
    }

    projMatrix.m[3][2] = -projMatrix.m[2][2] * camera->nearPlane;

	D3DXMatrixMultiply((D3DXMATRIX *)&projviewMatrix,(const D3DXMATRIX *)&viewMatrix,(const D3DXMATRIX *)&projMatrix);

	D3DXVECTOR3 InV(pWorldPos->x,pWorldPos->y,pWorldPos->z),OutV;

	D3DXVec3TransformCoord((D3DXVECTOR3*)&OutV,(const D3DXVECTOR3*)&InV,(const D3DXMATRIX *)&projviewMatrix);

	pPoint->x	= ( INT32 ) ( OutV.x * RwRasterGetWidth	( RwCameraGetRaster( g_MyEngine.m_pCamera ) )/camera->recipViewWindow.x );
	pPoint->y	= ( INT32 ) ( OutV.y * RwRasterGetHeight( RwCameraGetRaster( g_MyEngine.m_pCamera ) )/camera->recipViewWindow.y );

	D3DXVec3TransformCoord((D3DXVECTOR3*)&OutV,(const D3DXVECTOR3*)&InV,(const D3DXMATRIX *)&viewMatrix);
	if( OutV.z < 0 )	return FALSE; // Ä«¸Þ¶ó µÚ¿¡ ÀÖ´Ù.
	else				return TRUE;
}

struct __stObjectSelectionData
{
	RECT	rect			;
	BOOL	bGeometryCheck	;
	UINT	uBrushType		;

	__stObjectSelectionData():bGeometryCheck( TRUE ),uBrushType( MainWindow::OBJECT_SELECT ) {}
};

static RpClump *   __CallBackObjectSelection ( RpClump * pClump , void *pData )
{
	__stObjectSelectionData * pOSData = ( __stObjectSelectionData * ) pData;
	RECT	* pRect = &pOSData->rect;

	INT32		index		;
	INT32		nType		= AcuObject::GetClumpType( pClump , & index );

	switch( AcuObject::GetType( nType ) )
	{
	case	ACUOBJECT_TYPE_OBJECT		:
		// ¿ÀºêÁ§Æ®¸¸.. µî·ÏÇÔ..		
		if( // ¸¶°íÀÚ (2004-11-30 ¿ÀÀü 11:14:57) : ´øÁ¯ ¿ÀºêÁ§Æ®ÀÌ¸é Æ¨°Ü³¿ ..
			nType & ACUOBJECT_TYPE_DUNGEON_STRUCTURE	) return pClump;
		else
			break;
	case	ACUOBJECT_TYPE_WORLDSECTOR	:
	case	ACUOBJECT_TYPE_CHARACTER	:
	case	ACUOBJECT_TYPE_ITEM			:
	case	ACUOBJECT_TYPE_NONE			:
	default:
		return pClump;
	}

	POINT	pointScreen;

	RwFrame		* pFrame		= RpClumpGetFrame	( pClump );
	RwMatrix	* pMatrix		= RwFrameGetLTM		( pFrame );

	RwV3d		posCamera;
	{
		RwFrame		* pFrameCamera	= RwCameraGetFrame( g_pEngine->m_pCamera );
		posCamera = pFrameCamera->ltm.pos;
	}
	RwV3d		posObject;
	{
		posObject = pFrame->ltm.pos;
	}

	FLOAT	fDistanceObject		;
	FLOAT	fDistanceGeometry	;

	if( GetWorldPosToScreenPos( RwMatrixGetPos( pMatrix ) , & pointScreen ) )
	{
		// Rect ¹üÀ§¾È¿¡ µé¾î°¡¸é..
		if(	pRect->left	<= pointScreen.x && pointScreen.x <= pRect->right	&&
			pRect->top	<= pointScreen.y && pointScreen.y <= pRect->bottom	)
		{
			RwV3d	posGeometry;

			if ( pOSData->bGeometryCheck && AGCMMAP_THIS->GetMapPositionFromMousePosition_tool( (int)pointScreen.x, (int)pointScreen.y,  &posGeometry.x, &posGeometry.y, &posGeometry.z ) )
			{
				// ÁöÇü¿¡ °É¸®´Â°Ô ÀÖÀ¸¸é..
				// Ä«¸Þ¶ó¿ÍÀÇ °Å¸®Ã¼Å©..
				fDistanceObject		= AUPOS_DISTANCE( posCamera , posObject		);
				fDistanceGeometry	= AUPOS_DISTANCE( posCamera , posGeometry	);

				// 30 ¹ÌÅÍÀÇ ¸¶ÁøÀ» µÒ.. ¶¥¿¡ ¹ÚÈù°Í Ã³¸®¿ë.
				if( fDistanceObject < fDistanceGeometry + 3000.0f )
				{
					g_MainWindow.AddObjectSelection( pClump , TRUE );
				}
				else
				{
					// ÁöÇü µÚ¿¡ ÀÖÀ¸¹Ç·Î ¼±ÅÃ ½ºÅµ..
					__asm NOP;
				}
			}
			else
			{
				// ÁöÇüÀº °É¸®´Â°Ô ¾ø´Ù.
				g_MainWindow.AddObjectSelection( pClump , TRUE );
			}
		}
	}

	return pClump;

}

void	MainWindow::SetObjectSelection			( POINT p1 , POINT p2 , BOOL bGeometryCheck	)
{
	// µÎÁ¡ »çÀÌ¿¡ ÀÖ´Â ¿ÀºêÁ§Æ®¸¦ ±¸ÇÑ´Ù..
	// ¿Ã¶ó°¡ÀÖ´Â ¸ðµç ¿ÀºêÁ§Æ®¿¡¼­..
	// GetWorldpoToScreenPos¸¦ È£Ãâ, À§ÀÇ ¹üÀ§¿¡ µé¾î°¡´ÂÁö Á¡°ËÇÑ´Ù..
	// ±×¸®°ï¸®½ºÆ®¿¡ ±× ³»¿ëÀ»Ãß°¡ÇÑ´Ù.

	__stObjectSelectionData odsdata;
	odsdata.bGeometryCheck		= bGeometryCheck	;
	odsdata.uBrushType			= m_bObjectBrushing	;

	RECT	rect;
	if( p1.x < p2.x )
	{
		rect.left	=	p1.x;
		rect.right	=	p2.x;
	}
	else
	{
		rect.left	=	p2.x;
		rect.right	=	p1.x;
	}
	
	if( p1.y < p2.y )
	{
		rect.top	=	p1.y;
		rect.bottom	=	p2.y;
	}
	else
	{
		rect.top	=	p2.y;
		rect.bottom	=	p1.y;
	}

	odsdata.rect			= rect			;
	odsdata.bGeometryCheck	= bGeometryCheck;

	m_listObjectSelecting.RemoveAll();

	AGCMMAP_THIS->LoadingSectorsForAllClumps( 0 , __CallBackObjectSelection , ( void * ) &odsdata );
}

void	MainWindow::SetObjectSelection			( RpClump * pClump		)
{
	// ¸®½ºÆ®¸¦ ÃÊ±âÈ­ÇÏ°í , pClump¸¸À» ¼±ÅÃ ¸®½ºÆ®¿¡ µî·ÏÇÑ´Ù.
	m_listObjectSelected.RemoveAll();

	if( pClump )
	{
		AddObjectSelection( pClump );
	}
}

void	MainWindow::AddObjectSelection			( RpClump * pClump, BOOL bSelecting 	)
{
	ASSERT( NULL != pClump );

	// Setting values..
	ObjectSelectStruct	objectselect;

	objectselect.pClump	=	pClump;

	// Find the object Infomation..
	RwFrame			*	pFrame			= RpClumpGetFrame	( pClump );

	//RpUserDataArray	*	pUserDataArray	;
	
	ApdObject		*	pstApdObject	;
	AgcdObject		*	pstAgcdObject	;

	ASSERT( NULL != pFrame );
	objectselect.undoInfo.oid = 0;
	// Object ID¸¦ ¾ò¾î³¿.
	INT32	nType = AcuObject::GetClumpType( pClump , &objectselect.undoInfo.oid );

	switch( AcuObject::GetType( nType ) )
	{
	case	ACUOBJECT_TYPE_OBJECT			:
		break;
	default:
		objectselect.undoInfo.oid = 0;
		ASSERT( "¿ÀºêÁ§Æ® Á¤º¸°¡ ÀÌ»óÇÕ´Ï´Ù!" );
		return;	// Protection..
	}

	pstApdObject	= g_pcsApmObject->	GetObject		( objectselect.undoInfo.oid	);
	ASSERT( NULL != pstApdObject			);
	if( NULL == pstApdObject )	return;	// Protection..
	pstAgcdObject	= g_pcsAgcmObject->GetObjectData	( pstApdObject				);
	ASSERT( NULL != pstAgcdObject			);
	if( NULL == pstAgcdObject )	return; // Protection..

	// Æ÷ÀÎÆ® ¶óÀÌÆ® ¿ÀºêÁ§Æ®´Â ¼±ÅÃ ¾È‰?
	if( IsPointLightObject( pstApdObject ) ) return;

	objectselect.undoInfo.stScale		= pstApdObject->m_stScale		;	// Scale Vector
	objectselect.undoInfo.stPosition	= pstApdObject->m_stPosition	;	// Base Position
	objectselect.undoInfo.fDegreeX		= pstApdObject->m_fDegreeX		;	// Rotation Degree
	objectselect.undoInfo.fDegreeY		= pstApdObject->m_fDegreeY		;	// Rotation Degree

	AuList< ObjectSelectStruct >	*pList;
	if( bSelecting )	pList = &m_listObjectSelecting;
	else				pList = &m_listObjectSelected;
	
	{
		AuNode< ObjectSelectStruct >	* pNode			= pList->GetHeadNode();
		ObjectSelectStruct				* pObjectselect	;
		while( pNode )
		{
			pObjectselect = &pNode->GetData();


			if( pObjectselect->pClump == pClump )
			{
				// Áßº¹!..
				return;
			}

			pNode = pNode->GetNextNode();
		}

	}
	
	// ¹üÀ§Ã¼Å©..
	if( IsInLoadedRange( pstApdObject->m_stPosition.x , pstApdObject->m_stPosition.z ) )
	{
		// ¸¶°íÀÚ (2004-05-31 ¿ÀÈÄ 9:02:44) : ¹üÀ§ Ã¼Å©¸¦ ÇÑ´Ù..¹üÀ§ ¹Û¿¡ ÀÖ´Â ³à¼®µéÀº
		// ÆíÁýÀÌ µÅÁö ¾Êµµ·Ï.
		pList->AddTail( objectselect );
	}
}

void	MainWindow::RemoveObjectSelection		( RpClump * pClump		)
{
	AuNode< ObjectSelectStruct >	* pNode			= m_listObjectSelected.GetHeadNode();
	ObjectSelectStruct				* pObjectselect	;
	while( pNode )
	{
		pObjectselect = &pNode->GetData();

		if( pObjectselect->pClump == pClump )
		{
			m_listObjectSelected.RemoveNode( pNode );
			return;
		}

		pNode = pNode->GetNextNode();
	}

	// ¾ø³× -_-?..
}


void	MainWindow::DrawObjectSelectionWire		()
{
	if( m_listObjectSelected	.GetCount() <= 0	&&
		m_listObjectSelecting	.GetCount() <= 0	) return;

	// ¸®½ºÆ®¿¡ µî·ÏµÇ¾îÀÖ´Â ¿ÀºêÁ§Æ®¿¡°üÇÑ WireFrameÀ» ±×¸°´Ù.
	static
    RwImVertexIndex indicesSample[24] = {	0, 1, 1, 3, 3, 2, 2, 0,
											4, 5, 5, 7, 7, 6, 6, 4,
											0, 4, 1, 5, 2, 6, 3, 7 };

    RwMatrix       *ltm			;// = RwFrameGetLTM(RpClumpGetFrame(m_pSelectedClump));
    RwIm3DVertex	pVertex		[ 8	 ];//= new RwIm3DVertex		//	* ( m_listObjectSelected.GetCount() + m_listObjectSelecting.GetCount() ) ];
	//RwImVertexIndex	pIndices	[ 24 ];//= new RwImVertexIndex	//	* ( m_listObjectSelected.GetCount() + m_listObjectSelecting.GetCount() ) ];
	RpClump			* pClump	;
    RwInt32         count = 0 , i;
	RwBBox			bbox		;

	AuNode< ObjectSelectStruct >	* pNode;

	pNode = m_listObjectSelected.GetHeadNode();
	while( pNode )
	{
		pClump = pNode->GetData().pClump;

		ASSERT( NULL != pClump );

		ClumpGetBBox( pClump , &bbox );

		for (i = 0; i < 8; i++)
		{
			RwIm3DVertexSetPos(
				pVertex + i ,
				i&1 ? bbox.sup.x : bbox.inf.x,
				i&2 ? bbox.sup.y : bbox.inf.y,
				i&4 ? bbox.sup.z : bbox.inf.z);
			RwIm3DVertexSetRGBA(pVertex + i, 255, 0, 0, 128); 
		}

		ltm			= RwFrameGetLTM(RpClumpGetFrame(pClump));
		if (RwIm3DTransform( pVertex , 8 , ltm , rwIM3D_VERTEXRGBA ) )
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, indicesSample , 24 );
			RwIm3DEnd();
		}
		count ++;

		pNode = pNode->GetNextNode();;
	}

	pNode = m_listObjectSelecting.GetHeadNode();
	while( pNode )
	{
		pClump = pNode->GetData().pClump;

		ASSERT( NULL != pClump );

		ClumpGetBBox( pClump , &bbox );

		for (i = 0; i < 8; i++)
		{
			RwIm3DVertexSetPos(
				pVertex + i ,
				i&1 ? bbox.sup.x : bbox.inf.x,
				i&2 ? bbox.sup.y : bbox.inf.y,
				i&4 ? bbox.sup.z : bbox.inf.z);
			RwIm3DVertexSetRGBA(pVertex + i , 0 , 0, 255 , 128); 
		}

		ltm			= RwFrameGetLTM(RpClumpGetFrame(pClump));
		if (RwIm3DTransform( pVertex , 8 , ltm , rwIM3D_VERTEXRGBA ) )
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, indicesSample , 24 );
			RwIm3DEnd();
		}
		count ++;

		pNode = pNode->GetNextNode();;
	}
}

void	MainWindow::GetObjectCenter( RwV3d *v )
{
	// ¼±ÅÃµÈ ¿ÀºêÁ§Æ®ÀÇ ½ëÅÍ¸¦ ±¸ÇÔ..
	ASSERT( m_listObjectSelected.GetCount() > 0 );

	RwV3d	lefttop		;
	RwV3d	rightbottom	;
	RwV3d	* pPos		;

	RpClump * pClump;

	AuNode< ObjectSelectStruct >	* pNode = m_listObjectSelected.GetHeadNode();
	pClump	= pNode->GetData().pClump;
	pPos	= RwMatrixGetPos( RwFrameGetLTM( RpClumpGetFrame( pClump ) ) );

	lefttop.x		=	pPos->x	;
	lefttop.z		=	pPos->z	;
	lefttop.y		=	pPos->y	;

	rightbottom.x	=	pPos->x	;
	rightbottom.z	=	pPos->z	;
	rightbottom.y	=	pPos->y	;

	m_listObjectSelected.GetNext( pNode );

	while( pNode )
	{
		pClump	= pNode->GetData().pClump;
		pPos	= RwMatrixGetPos( RwFrameGetLTM( RpClumpGetFrame( pClump ) ) );

		if( pPos->x			< lefttop.x	)	lefttop.x		= pPos->x;
		if( pPos->y			< lefttop.y	)	lefttop.y		= pPos->y;
		if( pPos->z			< lefttop.z	)	lefttop.z		= pPos->z;
		if( rightbottom.x	< pPos->x	)	rightbottom.x	= pPos->x;
		if( rightbottom.y	< pPos->y	)	rightbottom.y	= pPos->y;
		if( rightbottom.z	< pPos->z	)	rightbottom.z	= pPos->z;

		m_listObjectSelected.GetNext( pNode );
	}

	v->x	=	( lefttop.x + rightbottom.x ) / 2.0f;
	v->z	=	( lefttop.z + rightbottom.z ) / 2.0f;
	v->y	=	( lefttop.y + rightbottom.y ) / 2.0f;
}

void	MainWindow::UpdateObjectUndoInfo()
{
	AuNode< ObjectSelectStruct >	* pNode = m_listObjectSelected.GetHeadNode();
	ObjectSelectStruct				* pObjectSelectStruct;

	RwFrame							* pFrame		;
	ApdObject						* pstApdObject	;
	AgcdObject						* pstAgcdObject	;
	INT32							nType			;

	while( pNode )
	{
		pObjectSelectStruct	= &pNode->GetData();

		ASSERT( NULL != pObjectSelectStruct->pClump );

		// Find the object Infomation..
		pFrame = RpClumpGetFrame	( pObjectSelectStruct->pClump );

		ASSERT( NULL != pFrame );
		pObjectSelectStruct->undoInfo.oid = 0;
		// Object ID¸¦ ¾ò¾î³¿.
		nType = AcuObject::GetClumpType( pObjectSelectStruct->pClump, &pObjectSelectStruct->undoInfo.oid );

		switch( AcuObject::GetType( nType ) )
		{
		case	ACUOBJECT_TYPE_OBJECT			:
			break;
		default:
			pObjectSelectStruct->undoInfo.oid = 0;
		}

		pstApdObject	= g_pcsApmObject->	GetObject		( pObjectSelectStruct->undoInfo.oid	);
		ASSERT( NULL != pstApdObject			);
		if( NULL == pstApdObject )	return;	// Protection..
		pstAgcdObject	= g_pcsAgcmObject->GetObjectData	( pstApdObject						);
		ASSERT( NULL != pstAgcdObject			);
		if( NULL == pstAgcdObject )	return; // Protection..

		pObjectSelectStruct->undoInfo.stScale		= pstApdObject->m_stScale		;	// Scale Vector
		pObjectSelectStruct->undoInfo.stPosition	= pstApdObject->m_stPosition	;	// Base Position
		pObjectSelectStruct->undoInfo.fDegreeX		= pstApdObject->m_fDegreeX		;	// Rotation Degree
		pObjectSelectStruct->undoInfo.fDegreeY		= pstApdObject->m_fDegreeY		;	// Rotation Degree

		m_listObjectSelected.GetNext( pNode );
	}
}

void	MainWindow::Reset()
{
	m_pCurrentGeometry	= NULL;
	SetObjectSelection( NULL );
}

void	MainWindow::DrawGridInfo	()
{
	if( g_MyEngine.GetSubCameraType() != g_MyEngine.SC_OFF ) return;

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

	// ¿ÀºêÁ§Æ® ¼±ÅÃ Ç¥½Ã..
	DrawObjectSelectionWire();
	if ( IsObjectSelected() )
	{
		HighlightRender();
	}

	// Draw Geometry wire frame
	DrawWireFrame( g_pMainFrame->m_Document.m_nShowGrid );

	// GeometryBlocking
	DrawGeometryBlocking();
		
	// Draw Selection
	DrawGeometrySelection();

	if( m_bRButtonDown ) AxisRender();

	// ½ºÆù ¹üÀ§ ±×¸®±â...
	BOOL	bShowSphere = FALSE;
	if( GetCurrentMode() == EDITMODE_OBJECT )
	{
		bShowSphere = TRUE;
	}
	else if( GetCurrentMode() == EDITMODE_OTHERS && g_pMainFrame->m_pTileList->m_pOthers->GetCurrentPlugin() )
	{
		CUITileList_Others::Plugin * pPlugin = g_pMainFrame->m_pTileList->m_pOthers->GetCurrentPlugin();
		if( pPlugin->pWnd->m_stOption.bShowSphere )
			bShowSphere = TRUE;
		else
			bShowSphere = FALSE;
	}

	if( m_pAreaSphere && bShowSphere )
	{
		INT32			lIndex			;
		ApdObject	*	pcsObject		;
		ApdEvent	*	pEvent			;
		AuPOS		*	pPos			;

		lIndex = 0;

		INT32		nEventType = CAlefMapDocument::RST_NONE;
		switch( g_pMainFrame->m_Document.m_nRangeSphereType )
		{
		case CAlefMapDocument::RST_SPAWN	: nEventType	= APDEVENT_FUNCTION_SPAWN	;	break;
		case CAlefMapDocument::RST_NATURE	: nEventType	= APDEVENT_FUNCTION_NATURE	;	break;
		default:
			break;
		}

		if( CAlefMapDocument::RST_NONE != nEventType )
		for (	pcsObject = g_pcsApmObject->GetObjectSequence(&lIndex)	;
				pcsObject														;
				pcsObject = g_pcsApmObject->GetObjectSequence(&lIndex)	)
		{
			pEvent	= g_pcsApmEventManager->GetEvent( pcsObject , ( ApdEventFunction ) nEventType );

			if( pEvent )
			{
				if( pEvent->m_pstCondition )
				{
					ApdEventConditionArea *	pArea = pEvent->m_pstCondition->m_pstArea;
					if( pArea )
					{
						switch( pArea->m_eType )
						{
						case	APDEVENT_AREA_SPHERE	:
							{
								pPos = g_pcsApmEventManager->GetBasePos( pcsObject , NULL );

								DrawAreaSphere( pPos->x , pPos->y , pPos->z , pArea->m_uoData.m_fSphereRadius );
							}
							break;
						case	APDEVENT_AREA_FAN		:
						case	APDEVENT_AREA_BOX		:
							{
								TRACE( "Áö¿øÇÏÁö ¾Ê´Â Å¸ÀÔ -_-" );
							}
							break;
						}
					}
				}
				else
				{
					// = =?
				}
				

			}
			else
			{
				// do nothing
			}
		}
	}

	if( ( m_pAreaSphere																	&&
		g_Const.m_bUseBrushPolygon														&&
		( GetCurrentMode() == EDITMODE_TILE || GetCurrentMode() == EDITMODE_GEOMETRY )	&&
		!g_pMainFrame->m_Document.IsInFirstPersonViewMode()								)

		||

		(	GetCurrentMode() == EDITMODE_OBJECT &&
			ISBUTTONDOWN( VK_CONTROL )	)
		)
	{
		DrawAreaSphere( m_Position.x , m_Position.y , m_Position.z , g_pMainFrame->m_Document.m_fBrushRadius );
	}
	
	// ¼±ÅÃ ¹Ú½º Ç¥½Ã..
	if( m_bObjectBrushing == OBJECT_SELECT			|| 
		m_bObjectBrushing == OBJECT_SELECTADD		|| 
		m_bObjectBrushing == OBJECT_SELECTREMOVE	)
	{
		RwIm2DVertex LineList[ 8 ];
		RwReal recipCameraZ = 1.0f / RwCameraGetNearClipPlane( g_pEngine->m_pCamera );

		RwIm2DVertexSetIntRGBA		(&LineList[0], 255.0f , 255.0f , 255.0f , 255.0f );
        RwIm2DVertexSetScreenX		(&LineList[0], ( FLOAT ) m_pointObjectSelectFirst.x	);
        RwIm2DVertexSetScreenY		(&LineList[0], ( FLOAT ) m_pointObjectSelectFirst.y	);
        RwIm2DVertexSetScreenZ		(&LineList[0], RwIm2DGetNearScreenZ()		);
        RwIm2DVertexSetRecipCameraZ	(&LineList[0], recipCameraZ					);
        RwIm2DVertexSetU			(&LineList[0], 0.0f , recipCameraZ			);
        RwIm2DVertexSetV			(&LineList[0], 0.0f , recipCameraZ			);
		
		RwIm2DVertexSetIntRGBA		(&LineList[1], 255.0f , 255.0f , 255.0f , 255.0f );
        RwIm2DVertexSetScreenX		(&LineList[1], ( FLOAT ) m_PrevMouseStatus.pos.x		);
        RwIm2DVertexSetScreenY		(&LineList[1], ( FLOAT ) m_pointObjectSelectFirst.y	);
        RwIm2DVertexSetScreenZ		(&LineList[1], RwIm2DGetNearScreenZ()		);
        RwIm2DVertexSetRecipCameraZ	(&LineList[1], recipCameraZ					);
        RwIm2DVertexSetU			(&LineList[1], 0.0f , recipCameraZ			);
        RwIm2DVertexSetV			(&LineList[1], 0.0f , recipCameraZ			);

 		RwIm2DVertexSetIntRGBA		(&LineList[2], 255.0f , 255.0f , 255.0f , 255.0f );
		RwIm2DVertexSetScreenX		(&LineList[2], ( FLOAT ) m_PrevMouseStatus.pos.x		);
        RwIm2DVertexSetScreenY		(&LineList[2], ( FLOAT ) m_pointObjectSelectFirst.y	);
        RwIm2DVertexSetScreenZ		(&LineList[2], RwIm2DGetNearScreenZ()		);
        RwIm2DVertexSetRecipCameraZ	(&LineList[2], recipCameraZ					);
        RwIm2DVertexSetU			(&LineList[2], 0.0f , recipCameraZ			);
        RwIm2DVertexSetV			(&LineList[2], 0.0f , recipCameraZ			);
		
		RwIm2DVertexSetIntRGBA		(&LineList[3], 255.0f , 255.0f , 255.0f , 255.0f );
        RwIm2DVertexSetScreenX		(&LineList[3], ( FLOAT ) m_PrevMouseStatus.pos.x		);
        RwIm2DVertexSetScreenY		(&LineList[3], ( FLOAT ) m_PrevMouseStatus.pos.y		);
        RwIm2DVertexSetScreenZ		(&LineList[3], RwIm2DGetNearScreenZ()		);
        RwIm2DVertexSetRecipCameraZ	(&LineList[3], recipCameraZ					);
        RwIm2DVertexSetU			(&LineList[3], 0.0f , recipCameraZ			);
        RwIm2DVertexSetV			(&LineList[3], 0.0f , recipCameraZ			);
		
		RwIm2DVertexSetIntRGBA		(&LineList[4], 255.0f , 255.0f , 255.0f , 255.0f );
        RwIm2DVertexSetScreenX		(&LineList[4], ( FLOAT ) m_PrevMouseStatus.pos.x		);
        RwIm2DVertexSetScreenY		(&LineList[4], ( FLOAT ) m_PrevMouseStatus.pos.y		);
        RwIm2DVertexSetScreenZ		(&LineList[4], RwIm2DGetNearScreenZ()		);
        RwIm2DVertexSetRecipCameraZ	(&LineList[4], recipCameraZ					);
        RwIm2DVertexSetU			(&LineList[4], 0.0f , recipCameraZ			);
        RwIm2DVertexSetV			(&LineList[4], 0.0f , recipCameraZ			);
		
		RwIm2DVertexSetIntRGBA		(&LineList[5], 255.0f , 255.0f , 255.0f , 255.0f );
        RwIm2DVertexSetScreenX		(&LineList[5], ( FLOAT ) m_pointObjectSelectFirst.x	);
        RwIm2DVertexSetScreenY		(&LineList[5], ( FLOAT ) m_PrevMouseStatus.pos.y		);
        RwIm2DVertexSetScreenZ		(&LineList[5], RwIm2DGetNearScreenZ()		);
        RwIm2DVertexSetRecipCameraZ	(&LineList[5], recipCameraZ					);
        RwIm2DVertexSetU			(&LineList[5], 0.0f , recipCameraZ			);
        RwIm2DVertexSetV			(&LineList[5], 0.0f , recipCameraZ			);
		
		RwIm2DVertexSetIntRGBA		(&LineList[6], 255.0f , 255.0f , 255.0f , 255.0f );
        RwIm2DVertexSetScreenX		(&LineList[6], ( FLOAT ) m_pointObjectSelectFirst.x	);
        RwIm2DVertexSetScreenY		(&LineList[6], ( FLOAT ) m_PrevMouseStatus.pos.y		);
        RwIm2DVertexSetScreenZ		(&LineList[6], RwIm2DGetNearScreenZ()		);
        RwIm2DVertexSetRecipCameraZ	(&LineList[6], recipCameraZ					);
        RwIm2DVertexSetU			(&LineList[6], 0.0f , recipCameraZ			);
        RwIm2DVertexSetV			(&LineList[6], 0.0f , recipCameraZ			);
		
		RwIm2DVertexSetIntRGBA		(&LineList[7], 255.0f , 255.0f , 255.0f , 255.0f );
        RwIm2DVertexSetScreenX		(&LineList[7], ( FLOAT ) m_pointObjectSelectFirst.x	);
        RwIm2DVertexSetScreenY		(&LineList[7], ( FLOAT ) m_pointObjectSelectFirst.y	);
        RwIm2DVertexSetScreenZ		(&LineList[7], RwIm2DGetNearScreenZ()		);
        RwIm2DVertexSetRecipCameraZ	(&LineList[7], recipCameraZ					);
        RwIm2DVertexSetU			(&LineList[7], 0.0f , recipCameraZ			);
        RwIm2DVertexSetV			(&LineList[7], 0.0f , recipCameraZ			);
		

		RwIm2DRenderPrimitive(rwPRIMTYPELINELIST, LineList, 8);
	}
	if( GetCurrentMode() == EDITMODE_OTHERS )
	{
		g_pMainFrame->m_pTileList->m_pOthers->OnWindowRender();
	}
	
	if( g_Const.m_bShowCollisionAtomic && m_pCurrentGeometry )
	{
		// ¸¶°íÀÚ (2003-12-05 ¿ÀÈÄ 2:51:30) : 
		// ÄÝ¸®Á¯ ¾ÆÅä¹Í È®ÀÎÇÏ´Â ÄÚµå
		ApWorldSector	**pSectorList	= g_pcsApmMap->GetCurrentLoadedSectors();
		ApWorldSector	* pSector		;
		RpAtomic		* pCollision	;
		rsDWSectorInfo	stDetailInfo	;
		int				oid				;

		RwV3d			vOffset			;
		vOffset.x = 0.0f	;
		vOffset.y = 1000.0f	;
		vOffset.z = 0.0f	;

		pSector = m_pCurrentGeometry;
		
		AGCMMAP_THIS->GetDWSector( pSector , pSector->GetCurrentDetail() , &stDetailInfo );
		INT32	nType = AcuObject::GetAtomicType( stDetailInfo.pDWSector->atomic , &oid , NULL , NULL , NULL, ( void ** ) &pCollision );

		__RenderAtomicOutLine( pCollision );
	}
	RwD3D9SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	RwD3D9SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	RwD3D9SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	if( CMainFrame::MT_VM_DUNGEON == g_pMainFrame->m_bFullScreen )
	{
		// ´øÁ¯ ¸ðµå¶ó¸é.. Idle ÀÌ¹ÌÁö Ã³¸®..
		g_pMainFrame->m_pDungeonWnd->Idle();
	}

	// ¸¶°íÀÚ (2005-12-15 ¿ÀÈÄ 12:12:27) : Å×½ºÆ® ¼­¹öµ¥ÀÌÅ¸ Ç¥½Ã..
	if( m_pCurrentGeometry )
	{
		static ApWorldSector * psCalcuratedSector = NULL;
		static RwIm3DVertex		psLineList	[ 65536 ] 	;

		int nTotal = m_pCurrentGeometry->m_ServerSectorInfo.GetTotal();

		if( psCalcuratedSector != m_pCurrentGeometry && m_pCurrentGeometry->m_ServerSectorInfo.GetTotal() )
		{
			AuPOS	* pVector = m_pCurrentGeometry->m_ServerSectorInfo.GetVector();

			int nBlock = m_pCurrentGeometry->m_ServerSectorInfo.GetBlockingCount();
			int nRide = m_pCurrentGeometry->m_ServerSectorInfo.GetRidableCount();

			for( int i = 0 ; i < nTotal ; i ++ )
			{
				RwIm3DVertexSetPos( psLineList + i ,
					pVector[ i ].x	,
					pVector[ i ].y	,
					pVector[ i ].z	);

				if( i < nBlock )
				{
					if( i >= nTotal - nRide )
					{
						// ºí·¯Å· + ¶óÀÌ´õºí
						RwIm3DVertexSetRGBA( psLineList + i , 255 , 0 , 255 , 170 ); 
					}
					else
					{
						// ºê·¯Å· ¿Â¸®
						RwIm3DVertexSetRGBA( psLineList + i , 255 , 0 , 0 , 170 ); 
					}
				}
				else
				{
					// ¶óÀÌ´õºí ¿Â¸®.
					RwIm3DVertexSetRGBA( psLineList + i , 0 , 0 , 255 , 170 ); 
				}
			}

			psCalcuratedSector = m_pCurrentGeometry;
		}//if( psCalcuratedSector != m_pCurrentGeometry && m_pCurrentGeometry->m_ServerSectorInfo.GetTotal() )
		
		if( RwIm3DTransform( psLineList , nTotal , NULL, rwIM3D_VERTEXRGBA ) )
		{                         
			RwIm3DRenderPrimitive( rwPRIMTYPETRILIST );

			RwIm3DEnd();
		}
	}
}

void	MainWindow::OnWindowRender	()					 // Im2D·Î RenderÇÏ±â
{
	if( g_MyEngine.IsRenderSubCamera() ) return;
	
	static bool FirstRender = true;
	if( FirstRender )
	{
		FirstRender = false;

		RwRenderStateSet(rwRENDERSTATECULLMODE  , (void *) rwCULLMODECULLNONE	);
	}

//	g_pcsAgcmEventNature->SetFogSetting();

	if( g_pMainFrame->m_Document.m_bToggleShadingMode )
	{
		g_pMainFrame->m_Document.m_bToggleShadingMode = false;
		int	shademode;
		RwRenderStateGet( rwRENDERSTATESHADEMODE , ( void * ) &shademode );
		switch( shademode )
		{
		case	rwSHADEMODEFLAT		:
			RwRenderStateSet( rwRENDERSTATESHADEMODE , ( void * ) rwSHADEMODEGOURAUD );
			DisplayMessage( AEM_NOTIFY , "¼ÎÀÌµù ¸ðµå º¯°æ : Gouraud" );
			break;
		case	rwSHADEMODEGOURAUD	:
			RwRenderStateSet( rwRENDERSTATESHADEMODE , ( void * ) rwSHADEMODEFLAT );
			DisplayMessage( AEM_NOTIFY , "¼ÎÀÌµù ¸ðµå º¯°æ : Flat" );
			break;
		}
	}
	
	RwD3D9SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
	RwD3D9SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
	RwD3D9SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_POINT);

	// ¸¶°íÀÚ (2004-04-21 ¿ÀÈÄ 7:04:16) : ¹é°ñ¾ÆÀú¾¾ ÆùÆ®ÂÊ ¹ö±×·Î ºí·¯Å·.
	if( CMainFrame::MT_VM_NORMAL == g_pMainFrame->m_bFullScreen )
	{
		RwRaster*	raster = RwCameraGetRaster(g_pEngine->m_pCamera);

		if( g_pcsAgcmFont )
		{
			g_pcsAgcmFont->FontDrawStart(0);	// gemani

			char		str[ 256 ];
			RwFrame		*pFrame = RwCameraGetFrame	( g_pEngine->m_pCamera	);
			RwMatrix	*pMat	= RwFrameGetLTM	( pFrame );
			RwV3d		*pvd	;

			int			nVertOffset;
			int			nLine		= 0;
			int			nFontHeight	= 20;

			// ¸®Á¯Á¤º¸..
			{
				int nRegion;
				nRegion = g_pcsApmMap->GetRegion( m_Position.x , m_Position.z );

				ApmMap::RegionTemplate * pTemplate = g_pcsApmMap->GetTemplate( nRegion );

				if( pTemplate )
				{
					wsprintf( str , "(%d) %s" , pTemplate->nIndex , pTemplate->pStrName );
					g_pcsAgcmFont->DrawTextMapTool( 1 , 31 , str  , 0 , (UINT8)255 , RGB( 0 , 0 , 0 ) , false);
					g_pcsAgcmFont->DrawTextMapTool( 0 , 30 , str , 0 , (UINT8)255 , RGB( 255 , 255 , 255 ) , false);
				}
			}

			CRect		rect;
			g_pMainFrame->m_wndView.GetClientRect( rect ) ;
			nVertOffset	= rect.Height()	- ( nFontHeight + 3 ) * 5;
			
			pvd	= RwMatrixGetPos( pMat );
			sprintf( str , "Camera Pos ( %3.3f , %3.3f , %3.3f )" , pvd->x , pvd->y , pvd->z );
			g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 1 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * ( nLine ) + 1 ) , str  , 0 , (UINT8)255 , RGB( 0 , 0 , 0 ) , false);
			g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str  , 0 , (UINT8)255 , RGB( 255 , 255 , 255 ) , false);
			//g_pcsAgcmFont->DrawText(raster,( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str ,0);

			pvd	= RwMatrixGetAt( pMat );
			

			// ¸¶°íÀÚ (2004-04-02 ¿ÀÈÄ 7:04:58) : ·ÎÅ×ÀÌÆ® °ªÀ¸·Î È­¸é Ãâ·Â..
			FLOAT		fXAngle , fYAngle;

			fXAngle = ( FLOAT ) atan( pvd->y / pvd->z ) * 180.0f / 3.141592f;
			if (fXAngle < 0)
				fXAngle = -fXAngle;
			if (pvd->y < 0)
				fXAngle = -fXAngle;

			fYAngle = ( FLOAT ) atan(pvd->x / pvd->z) * 180.0f / 3.141592f;
			if (pvd->x >= 0 && pvd->z < 0)
				fYAngle += 180;
			else if (pvd->x < 0 && pvd->z < 0)
				fYAngle -= 180;

			sprintf( str , "Camera Look At ( %3.3f , %3.3f , %3.3f ) - ( RotX : %3.3f , RotY : %3.3f ) " , pvd->x , pvd->y , pvd->z , fXAngle , fYAngle );

			g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 1 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * ( nLine ) + 1 ) , str  , 0 , (UINT8)255 , RGB( 0 , 0 , 0 ) , false);
			g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str  , 0 , (UINT8)255 , RGB( 255 , 255 , 255 ) , false);
			//g_pcsAgcmFont->DrawText(raster,( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str ,0);

			if( m_pCurrentGeometry )
			{
				sprintf( str , "Height %.2f Meter ( %f )" , m_Position.y / 100.0f , m_Position.y );
				g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 1 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * ( nLine ) + 1 ) , str  , 0 , (UINT8)255 , RGB( 0 , 0 , 0 ) , false);
				g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str  , 0 , (UINT8)255 , RGB( 255 , 255 , 255 ) , false);
				//g_pcsAgcmFont->DrawText(raster,( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str ,0);
			}

			switch( GetCurrentMode() )
			{
			case EDITMODE_OBJECT:
				{
					if( IsObjectSelected() )
					{
						sprintf( str , "Total Object Count = ( %d / %d ) , ¼±ÅÃµÈ ¿ÀºêÁ§Æ® %d °³"	, 
							g_pcsApmObject->m_clObjects.GetObjectCount	()	,
							g_pcsApmObject->m_clObjects.GetCount		()	,
							m_listObjectSelected.GetCount()					);
						g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 1 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * ( nLine ) + 1 ) , str  , 0 , (UINT8)255 , RGB( 0 , 0 , 0 ) , false);
						g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str  , 0 , (UINT8)255 , RGB( 255 , 255 , 255 ) , false);
					}
					else
					{
						sprintf( str , "Total Object Count = ( %d / %d )"	, 
							g_pcsApmObject->m_clObjects.GetObjectCount	()	,
							g_pcsApmObject->m_clObjects.GetCount		()	);
						g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 1 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * ( nLine ) + 1 ) , str  , 0 , (UINT8)255 , RGB( 0 , 0 , 0 ) , false);
						g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str  , 0 , (UINT8)255 , RGB( 255 , 255 , 255 ) , false);
					}
				}
				break;
			default:
				break;
			}

			RpClump	* pCurrentClump = AGCMMAP_THIS->GetCursorClump( g_MyEngine.m_pCamera , & m_PrevMouseStatus.pos , g_bUsePickingOnly );

			if ( pCurrentClump )
			{
				// ¿ÀºêÁ§Æ® Á¤º¸ Ã£¾Æ³¿..
				RwFrame			*	pFrame			= RpClumpGetFrame	( pCurrentClump			);

				ApdObject		*	pstApdObject	;
				AgcdObject		*	pstAgcdObject	;
				AgcdObjectTemplate	*	pstAgcdObjectTempte		;

				ASSERT( NULL != pFrame );
				// Object ID¸¦ ¾ò¾î³¿.
				
				INT32	oid			= 0;
				
				switch( AcuObject::GetType( AcuObject::GetClumpType( pCurrentClump , &oid ) ) )
				{
				case	ACUOBJECT_TYPE_OBJECT			:
					{
						pstApdObject	= g_pcsApmObject->	GetObject		( oid			);
						pstAgcdObject	= g_pcsAgcmObject->GetObjectData	( pstApdObject	);

						pstAgcdObjectTempte		= g_pcsAgcmObject->GetTemplateData( pstApdObject->m_pcsTemplate );

						if( pstApdObject && pstAgcdObject )
						{
							sprintf( str , "Object Info ( OID = %d , TID = %d , Name = %s , Filename = %s)" , 
								pstApdObject->m_lID												, 
								pstApdObject->m_lTID											,
								pstApdObject->m_pcsTemplate->m_szName							,
								pstAgcdObjectTempte->m_stGroup.m_pstList->m_csData.m_pszDFFName	);
							g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 1 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * ( nLine ) + 1 ) , str  , 0 , (UINT8)255 , RGB( 0 , 0 , 0 ) , false);
							g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str  , 0 , (UINT8)255 , RGB( 255 , 255 , 255 ) , false);
							//g_pcsAgcmFont->DrawText(raster,( FLOAT ) 0 , ( FLOAT ) ( nVertOffset + ( nFontHeight + 3 ) * nLine++ ) , str ,0);
						}
					}
					break;
				default:
					break;
				}
			}

			// FPS Ç¥½Ã
			//if( m_FPSOn )
			{
				char	str[ 256 ];
				wsprintf( str , "FPS: %03d", g_MyEngine.GetFps() );
				g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 1 , ( FLOAT ) 1 , str  , 0 , (UINT8)255 , RGB( 0 , 0 , 0 ) , false);
				g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 0 , ( FLOAT ) 0 , str  , 0 , (UINT8)255 , RGB( 255 , 255 , 255 ) , false);
				//g_pcsAgcmFont->DrawText(raster,( FLOAT ) 0 , ( FLOAT ) 0 , str ,0);
				
//				wsprintf( str , "Tri: %d", g_pcsAgcmRender->m_iPolygonNum );
//				g_pcsAgcmFont->DrawTextMapTool( ( FLOAT ) 0 , ( FLOAT ) 20 , str  , 0 , (UINT8)255 , RGB( 255 , 255 , 255 ) , false);
//				//g_pcsAgcmFont->DrawText(raster,( FLOAT ) 0 , ( FLOAT ) 20 , str ,0);
			}

			#ifdef _PROFILE_
			// Profile
			if( IsProfile() )
			{
				bool	bUpdate = false;
				static	UINT32	tickdiff = 0;
				tickdiff += g_pcsAgcmTuner->m_uiCurTickDiff;
				if(tickdiff > 1000)
				{
					bUpdate = true;
					tickdiff = 0;
				}
				
				if(bUpdate)
				{
					// profile manager update
					if(m_pCurNode == NULL)
					{
						m_pCurNode = AuProfileManager::Root;
					}
					else if(m_pCurNode == AuProfileManager::Roots[0])
					{
						CProfileNode* tnode = m_pCurNode->Child;
						if(tnode) m_pCurNode = tnode;
					}
					else if(m_pCurNode == AuProfileManager::Roots[1])
					{
						CProfileNode* tnode = m_pCurNode->Child;
						if(tnode) m_pCurNode = tnode;
					}

					// child·Î ÀÌµ¿
					if(m_iCommand >= PFC_STEPINTO0 && m_iCommand <= PFC_STEPINTO0 + 9 )	
					{
						if(m_pCurNode)
						{
							CProfileNode* tnode = m_pCurNode->Get_Child(m_iCommand);
							if(tnode != NULL)
							{
								m_pCurNode = tnode;
								m_iMyCurPage = 0;
							}
						}

						m_iTotalItem = 0;
						CProfileNode* tnode = m_pCurNode;
						while(tnode)
						{
							tnode = tnode->Sibling;
							++m_iTotalItem;
						}
					}
					else if(m_iCommand >= PFC_REGISTER0 && m_iCommand <= PFC_REGISTER0 + 9)
					{
						if(m_pCurNode->Parent)
						{
							CProfileNode*	tnode = m_pCurNode->Parent->Child->Get_Sibling(m_iMyCurPage*10 + m_iCommand-50);
							if(tnode)
							{
								m_pCheckedNode[m_iCheckIndex++] = tnode;
								if(m_iCheckIndex >= 10 ) m_iCheckIndex = 0;
							}
						}
					}
					else if(m_iCommand == PFC_PAGEUP)
					{
						if(m_iMyCurPage > 0) --m_iMyCurPage;
						if(m_pCurNode->Parent)
						m_pCurNode = m_pCurNode->Parent->Child->Get_Sibling(m_iMyCurPage*10);
					}
					else if(m_iCommand == PFC_PAGEDN)
					{
						if(m_iTotalItem > 9) ++m_iMyCurPage;
						if(m_pCurNode->Parent)
						m_pCurNode = m_pCurNode->Parent->Child->Get_Sibling(m_iMyCurPage*10);
					}
					else if(m_iCommand == PFC_TOROOT)
					{
						if(m_pCurNode)
						{
							CProfileNode* tnode = m_pCurNode->Get_Parent();
							if(tnode != NULL)	m_pCurNode = tnode;

							if(m_pCurNode->Parent)
							m_pCurNode = m_pCurNode->Parent->Child->Get_Sibling(0);

							m_iMyCurPage = 0;
						}
						m_iTotalItem = 0;

						CProfileNode* tnode = m_pCurNode;
						while(tnode)
						{
							tnode = tnode->Sibling;
							++m_iTotalItem;
						}
					}
					else if(m_iCommand == PFC_TOLEAF)
					{
						m_pCurNode = AuProfileManager::Roots[0];
						m_iMyCurPage = 0;

						m_iTotalItem = 0;
						CProfileNode* tnode = m_pCurNode;
						while(tnode)
						{
							tnode = tnode->Sibling;
							++m_iTotalItem;
						}
					}
					else if(m_iCommand == PFC_RESET)
					{
						m_pCurNode = AuProfileManager::Roots[1];
						m_iMyCurPage = 0;

						m_iTotalItem = 0;
						CProfileNode* tnode = m_pCurNode;
						while(tnode)
						{
							tnode = tnode->Sibling;
							++m_iTotalItem;
						}
					}
					
					m_iCommand = PFC_NOCOMMAND;
					float		time = AuProfileManager::Get_Time_Since_Reset();
					float		time2 = AuProfileManager::Get_Time_Since_Start();

					// °¡Àå ¸¹Àº ½Ã°£ Àâ¾Æ ¸ÔÀº node°Ë»ö
					CProfileNode*	search_node = AuProfileManager::Root->Child;
					CProfileNode*	maxnode = search_node;
					if(maxnode)
					{
						FindMaxItem(search_node,&maxnode);
					}

					memset(profile_max_str,'\0',200);

					if (maxnode)
					{
						float tf = maxnode->Get_Total_Time()/time;
						sprintf(profile_max_str,"¡áMax>> %40s ::  %3d  :: %4f :: %3d %% ¢º %5d :: %3d %%",
						maxnode->Get_Name(),maxnode->Get_Total_Calls(),
						maxnode->Get_Total_Time(),(int)(tf * 100.0f),
						maxnode->AccumulationCalls,(int)(maxnode->AccumulationTime / time2 * 100.0f));
					}

					CProfileNode* cnode = m_pCurNode;

					int index = 0;

					for(int j = 0; j<10;++j)
					memset(profile_str[j],'\0',200);
					
					while(cnode && index<10)
					{
						float tf = cnode->Get_Total_Time()/time;
						sprintf(profile_str[index],"%3d - %40s ::  %3d  :: %4f :: %3d %% ¢º %5d :: %3d %%",
						index,cnode->Get_Name(),cnode->Get_Total_Calls(),
						cnode->Get_Total_Time(),(int)(tf * 100.0f),
						cnode->AccumulationCalls,(int)(cnode->AccumulationTime / time2 * 100.0f));
									
						++index;
						cnode = cnode->Get_Sibling();
					}

					for(int i=0;i<10;++i)
						memset(profile_check_str[i],'\0',200);

					for(i=0;i<10;++i)
					{
						if(m_pCheckedNode[i])
						{
							float tf = m_pCheckedNode[i]->Get_Total_Time()/time;
							sprintf(profile_check_str[i],"%3d - %70s ::  %3d  :: %4f :: %3d %% ¢º %5d :: %3d %%",i,m_pCheckedNode[i]->Get_Name(),m_pCheckedNode[i]->Get_Total_Calls(),
							m_pCheckedNode[i]->Get_Total_Time(),(int)(tf * 100.0f),
							m_pCheckedNode[i]->AccumulationCalls,(int)(m_pCheckedNode[i]->AccumulationTime / time2 * 100.0f));
						}
					}

					if(m_iProfileMode == 0)
						AuProfileManager::Reset();
				}

				if(m_iMyCurPage >0) g_pcsAgcmFont->DrawTextMapTool(50,10,"¡è", 0 ,180,0xffffffff,false);
				for(int i=0;i<10;++i)
				{
					g_pcsAgcmFont->DrawTextMapTool( ( float ) 50, ( float ) ( 30 + i*30 ),profile_str[i], 0 ,180,0xffffffff,false);
				}

				if(m_iTotalItem >9) g_pcsAgcmFont->DrawTextMapTool(60,10,"¡é", 0 ,180,0xffffffff,false);

				//g_pcsAgcmFont->DrawTextIM2DBack(50,360,profile_max_str, 0 ,210,0xffffffff,0x7fff0000);
				g_pcsAgcmFont->DrawTextMapTool(50,360,profile_max_str, 0 ,210,0xffffff00,false);

				for(i=0;i<10;++i)
				{
					g_pcsAgcmFont->DrawTextMapTool( ( float ) 50, ( float )  ( 400 + i*30 ) ,profile_check_str[i], 0 ,210,0xffffffff,false);
				}
			}
			#endif // _PROFILE_

			g_pcsAgcmFont->FontDrawEnd();	// gemani
		}

		RwRenderStateSet( rwRENDERSTATEZTESTENABLE, (void*)true );
	}
	
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 
            (void *)NULL);

	if( GetCurrentMode() == EDITMODE_OTHERS )
	{
		g_pMainFrame->m_pTileList->m_pOthers->OnWindowRender();
	}
	
	if( g_Const.m_bShowCollisionAtomic && m_pCurrentGeometry )
	{
		// ¸¶°íÀÚ (2003-12-05 ¿ÀÈÄ 2:51:30) : 
		// ÄÝ¸®Á¯ ¾ÆÅä¹Í È®ÀÎÇÏ´Â ÄÚµå
		ApWorldSector	**pSectorList	= g_pcsApmMap->GetCurrentLoadedSectors();
		ApWorldSector	* pSector		;
		RpAtomic		* pCollision	;
		rsDWSectorInfo	stDetailInfo	;
		int				oid				;

		RwV3d			vOffset			;
		vOffset.x = 0.0f	;
		vOffset.y = 1000.0f	;
		vOffset.z = 0.0f	;

		pSector = m_pCurrentGeometry;
		
		AGCMMAP_THIS->GetDWSector( pSector , pSector->GetCurrentDetail() , &stDetailInfo );
		INT32	nType = AcuObject::GetAtomicType( stDetailInfo.pDWSector->atomic , &oid , NULL , NULL , NULL, ( void ** ) &pCollision );

		__RenderAtomicOutLine( pCollision );
	}

	RwD3D9SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	RwD3D9SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	RwD3D9SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);


	// ¾ÆÅä¹Í Å×½ºÆ®
	static BOOL __bTestOn = FALSE;
	if( __bTestOn )
	{
		AuNode< ObjectSelectStruct > * pNode = m_listObjectSelected.GetHeadNode();

		// Ã³À½³à¼®¸¸ Ã¼Å©ÇÑ´Ù..
		RpAtomic * __TestAtomicCallback(RpAtomic *atomic, void *data);

		if( pNode )
		{
			RpClump * pClump = pNode->GetData().pClump;

			RpClumpForAllAtomics( pClump , __TestAtomicCallback , NULL );	
		}

		__bTestOn = FALSE;
	}

	static BOOL _bShowSphere = FALSE;

	if( _bShowSphere )
	{
		INT32			lIndex			;
		ApdObject	*	pcsObject		;

		lIndex = 0;

		ApWorldSector * pSectorObject;

		for (	pcsObject = g_pcsApmObject->GetObjectSequence(&lIndex)	;
				pcsObject														;
				pcsObject = g_pcsApmObject->GetObjectSequence(&lIndex)	)
		{
			pSectorObject = g_pcsApmMap->GetSector( pcsObject->m_stPosition.x , pcsObject->m_stPosition.z );

			if( pSectorObject == m_pCurrentGeometry )
			{
				AgcdObject * pstAgcdObject = g_pcsAgcmObject->GetObjectData(pcsObject);

				// AcuObjecWire::bGetInst().bRenderClump( pstAgcdObject->m_stGroup.m_pstList[ 0 ].m_csData.m_pstClump );
				DrawAreaSphere( pcsObject->m_stPosition.x , pcsObject->m_stPosition.y , pcsObject->m_stPosition.z ,
					g_pMainFrame->m_Document.m_fBrushRadius );
			}
		}
	}
}

RpAtomic * __TestAtomicCallback(RpAtomic *atomic, void *data)
{
	RpGeometry * pGeometry = RpAtomicGetGeometry( atomic );
	RwTexCoords	* texCoords ;

	texCoords = RpGeometryGetVertexTexCoords( pGeometry , rwTEXTURECOORDINATEINDEX0 );

	for( int i = 0 ; i < pGeometry->numVertices ; i++ )
	{
		texCoords[ i ].u *= 2.0f;
		texCoords[ i ].v *= 2.0f;
	}

	return atomic;
}


BOOL MainWindow::LockSector			( ApWorldSector * pSector	, RwInt32 nLockMode	)	// ÇöÀç ¼½ÅÍ¸¦ ¶ôÇÔ.
{
	return AGCMMAP_THIS->LockSector( pSector , nLockMode );
}

BOOL MainWindow::UnlockSectors		( BOOL bUpdateCollision  , BOOL bUpdateRough )	// ¶ôÇÑ ¸ðµç ¼½ÅÍ¸¦ ¾ð¶ôÇÔ.
{
	return AGCMMAP_THIS->UnlockSectors( bUpdateCollision , bUpdateRough );
}

void	__RenderAtomicOutLine( RpAtomic * pAtomic )
{
#ifdef _DEBUG
	if( NULL == pAtomic ) return;

	static RwIm3DVertex		pLineList	[ 65536 ] 	;
	static RwImVertexIndex	pIndex		[ 65536 ] 	;

	RpGeometry * pGeometry = RpAtomicGetGeometry( pAtomic );

	if( NULL == pGeometry ) return;

	RpMorphTarget * pMorphTarget = RpGeometryGetMorphTarget( pGeometry , 0 );

	if( NULL == pMorphTarget ) return;

	// ¹öÅØ½º Ä«ÇÇ.
	int i;
	for( i = 0 ; i < pGeometry->numVertices ; ++ i )
	{
        RwIm3DVertexSetPos( pLineList + i ,
				pMorphTarget->verts[ i ].x	,
				pMorphTarget->verts[ i ].y	+ 50.0f,
				pMorphTarget->verts[ i ].z	);

        RwIm3DVertexSetRGBA( pLineList + i , 255 , 124 , 180 , 255 ); 
	}

	for( i = 0 ; i < pGeometry->numTriangles ; ++ i )
	{
		pIndex[ 6 * i + 0	] = pGeometry->triangles[ i ].vertIndex[ 0 ];
		pIndex[ 6 * i + 1	] = pGeometry->triangles[ i ].vertIndex[ 1 ];
		pIndex[ 6 * i + 2	] = pGeometry->triangles[ i ].vertIndex[ 1 ];
		pIndex[ 6 * i + 3	] = pGeometry->triangles[ i ].vertIndex[ 2 ];
		pIndex[ 6 * i + 4	] = pGeometry->triangles[ i ].vertIndex[ 2 ];
		pIndex[ 6 * i + 5	] = pGeometry->triangles[ i ].vertIndex[ 0 ];
	}

	if( RwIm3DTransform( pLineList , pGeometry->numVertices , NULL, rwIM3D_ALLOPAQUE ) )
	{                         
		RwIm3DRenderIndexedPrimitive( rwPRIMTYPELINELIST,
			pIndex , pGeometry->numTriangles * 6 );

		RwIm3DEnd();
	}
#endif // _DEBUG
}

BOOL	IsInLoadedRange				( FLOAT x , FLOAT z )
{
	if( GetSectorStartX	( ArrayIndexToSectorIndexX(ALEF_LOAD_RANGE_X1) )	<= x	&&
		x < GetSectorEndX	( ArrayIndexToSectorIndexX(ALEF_LOAD_RANGE_X2) )		&&
		GetSectorStartZ	( ArrayIndexToSectorIndexZ(ALEF_LOAD_RANGE_Y1) )	<= z	&&
		z < GetSectorEndZ	( ArrayIndexToSectorIndexZ(ALEF_LOAD_RANGE_Y2) )		)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL	ClipLoadedRange				( FLOAT *pfX , FLOAT *pfZ )
{
	if( *pfX < GetSectorStartX	( ArrayIndexToSectorIndexX(ALEF_LOAD_RANGE_X1) ) ) *pfX = GetSectorStartX	( ArrayIndexToSectorIndexX(ALEF_LOAD_RANGE_X1) );
	if( *pfX > GetSectorEndX	( ArrayIndexToSectorIndexX(ALEF_LOAD_RANGE_X2) ) ) *pfX = GetSectorEndX		( ArrayIndexToSectorIndexX(ALEF_LOAD_RANGE_X2) );
	if( *pfZ < GetSectorStartZ	( ArrayIndexToSectorIndexZ(ALEF_LOAD_RANGE_Y1) ) ) *pfX = GetSectorStartZ	( ArrayIndexToSectorIndexZ(ALEF_LOAD_RANGE_Y1) );
	if( *pfZ > GetSectorEndZ	( ArrayIndexToSectorIndexZ(ALEF_LOAD_RANGE_Y2) ) ) *pfX = GetSectorEndZ		( ArrayIndexToSectorIndexZ(ALEF_LOAD_RANGE_Y2) );

	return TRUE;
}

BOOL	CBPreRenderAtomicCallback	( RpAtomic * pAtomic , void * pData	)
{
	//if( g_pMainFrame->m_Document.m_bShowSystemObject ) return TRUE;

	INT32	lID;
	INT32	nType = AcuObject::GetAtomicType( pAtomic, &lID );

	switch( AcuObject::GetType( nType ) )
	{
	case ACUOBJECT_TYPE_WORLDSECTOR:
		{
			// ÁöÇü °Ë»ç..
			if( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_GEOMTERY )
				return TRUE;
			else 
				return FALSE;
		}
	case ACUOBJECT_TYPE_OBJECT	:
		{
			if( !( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECTALL ) )
				return FALSE;

			if( nType & ACUOBJECT_TYPE_DUNGEON_DOME )
			{
                if( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECT_DOME )
				{
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}


			if( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_SYSTEMOBJECT )
			{
				return TRUE;
			}
			else
			if (
				( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECT_RIDABLE	) &&
				( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECT_BLOCKING	) &&
				( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECT_EVENT		) &&
				( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECT_GRASS		) &&
				( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECT_OTHERS		))
			{
				if( nType & ACUOBJECT_TYPE_IS_SYSTEM_OBJECT )
					return FALSE;
				else
					return TRUE;
			}
			if( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECT_RIDABLE )
			{
				if( nType & ACUOBJECT_TYPE_RIDABLE )
					return TRUE;
				else
					return FALSE;
			}
			else
			if( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECT_BLOCKING )
			{
				if( nType & ACUOBJECT_TYPE_BLOCKING )
					return TRUE;
				else
					return FALSE;
			}
			else
			if( g_pMainFrame->m_Document.m_uAtomicFilter & CAlefMapDocument::AF_OBJECT_EVENT  )
			{
				ApdObject *	pcsObject = g_pcsApmObject->GetObject(lID);
				if( NULL != pcsObject && g_pcsApmEventManager->GetEvent( pcsObject , ( ApdEventFunction ) g_pMainFrame->m_Document.m_uFilterEventID	) )
					return TRUE;
				else
					return FALSE;
			}
				
		}
		// ¿ä±îÁö ¿ÔÀ¸¸é º¼Àå ³¡ -_-
		return FALSE;
	default:
		return TRUE;
	}
}

#ifdef _PROFILE_
void MainWindow::FindMaxItem(CProfileNode* node, CProfileNode**		setnode)
{
	if(node->Child)
	{
		FindMaxItem(node->Child,setnode);
	}
	
	if(node->Sibling)
	{
		FindMaxItem(node->Sibling,setnode);
	}

	if(node->TotalTime > (*setnode)->TotalTime)
	{
		(*setnode) = node;
	}
}
#endif

void	MainWindow::UpdateCollision	( ApWorldSector * pSector	)
{
	if( NULL == pSector ) return;

	rsDWSectorInfo	dwSectorInfo;
	AGCMMAP_THIS->GetDWSector	( pSector ,  & dwSectorInfo				);
	RpCollisionGeometryBuildData( dwSectorInfo.pDWSector->geometry , NULL );
	
	RpMorphTarget * pDWMorph = RpGeometryGetMorphTarget( dwSectorInfo.pDWSector->geometry , 0 );
	RwSphere	sphere;
	RpMorphTargetCalcBoundingSphere	( pDWMorph , &sphere );
	RpMorphTargetSetBoundingSphere	( pDWMorph , &sphere );

	dwSectorInfo.pDWSector->atomic->boundingSphere		= sphere;
	dwSectorInfo.pDWSector->atomic->worldBoundingSphere	= sphere;
}

BOOL	MainWindow::LoadLocalMapFile()
{
	PROCESSIDLE();

	if( AGCMMAP_THIS )
		AGCMMAP_THIS->InitMapToolLoadRange	( ALEF_LOAD_RANGE_X1 , ALEF_LOAD_RANGE_Y1 , ALEF_LOAD_RANGE_X2 , ALEF_LOAD_RANGE_Y2 );
	
	PROCESSIDLE();

	if( AGCMMAP_THIS )
	{
		{
			CProgressDlg dlg;
			dlg		.StartProgress	( "ÁøÇà" , 100 , g_pMainFrame			);
			AGCMMAP_THIS->LoadAll	( TRUE , _ProgressCallback , ( void * ) &dlg	);
			dlg		.EndProgress	(										);
		}

		PROCESSIDLE();
		PROCESSIDLE();

		{
			CProgressDlg dlg;
			dlg			.StartProgress	( "ÁøÇà" , 100 , g_pMainFrame			);
			AGCMMAP_THIS->MakeDetailWorld( _ProgressCallback , ( void * ) &dlg	);
			dlg			.EndProgress	(										);
		}

		PROCESSIDLE();
		PROCESSIDLE();
		
		AGCMMAP_THIS->SetCurrentDetail( SECTOR_HIGHDETAIL );
	}

	PROCESSIDLE();
	PROCESSIDLE();

	try
	{
		g_pMainFrame->m_pTileList->m_pObjectWnd->LoadLocalObjectInfo();	
	}
	CRASH_SAFE_MESSAGE( "¿ÀºêÁ§Æ® Á¤º¸ ÀÐ±â" );

	PROCESSIDLE();
	PROCESSIDLE();

	// ´øÁ¯ ÆÄÀÏ ·Îµù..
	if( g_pMainFrame->m_pDungeonWnd->LoadDungeon() )
	{
		
	}
	else
	{
		TRACE( "´øÁ¯ ÀÐ±â ½ÇÆÐ..\n" );
	}

	PROCESSIDLE();
	PROCESSIDLE();

	MoveToSectorCenter	( NULL );

	ClearAllSaveFlags();

	return TRUE;
}

void	MainWindow::MoveTo( FLOAT fX , FLOAT fZ , FLOAT fDY )
{
	RwFrame* pFrame = RwCameraGetFrame( g_pEngine->m_pCamera	);
 	ASSERT( pFrame );

	ASSERT( pFrame == pFrame->root && "ÄÉ¸¯ÅÍ ½ÃÁ¡ »ç¿ëÈÄ¿¡ TABÀº Àá½Ã »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù." );
	if( pFrame != pFrame->root )		return;

	FLOAT	fHeight	= ALEF_SECTOR_WIDTH * 1.5f;
	
	RwV3d	pos = { fX, fHeight, fZ + ALEF_SECTOR_WIDTH / 10.0f };
	RwV3d	at = { fX, 0.f, fZ };
	RwV3d	up = { 0.f, 0.f, -1.f };
	RwV3d	right = { 1.f, 0.f, 0.f };

	// ¸¶°íÀÚ (2003-11-25 ¿ÀÈÄ 1:01:12) : Ä«¸Þ¶ó ºüÁö´ÂÇö»óÁ¦°Å..
	fHeight = AGCMMAP_THIS->GetHeight( pos.x , pos.z , SECTOR_MAX_HEIGHT );
	pos.y = fHeight + fDY;

	RwV3dSub( &at, &pos, &at );
	RwV3dScale( &at, &at, -1.f );
	RwV3dNormalize( &at, &at );

	RwMatrix* pMatrix = RwFrameGetMatrix( pFrame );
	pMatrix->pos	= pos;
	pMatrix->at		= at;
	pMatrix->up		= up;
	pMatrix->right	= right;

	RwMatrixUpdate( pMatrix );
	RwFrameUpdateObjects( pFrame );
	RwCameraSetFrame( g_pEngine->m_pCamera, pFrame	);

	RwMatrix* m = RwFrameGetMatrix( pFrame );
	ASSERT( m );

	RwV3d	yaxis	= { 0.0f , 1.0f , 0.0f };
	RwV3dCrossProduct	( &m->right	, &yaxis	, &m->at	);
	m->right.y = 0.0f;
	RwV3dNormalize		( &m->right	, &m->right	);
	RwV3dCrossProduct	( &m->up	, &m->at	, &m->right	);
	RwV3dNormalize		( &m->up	, &m->up	);
	RwV3dCrossProduct	( &m->right	, &m->up	, &m->at	);

	RwMatrixOrthoNormalize( m , m );

	RwV3d* pCameraPos = RwMatrixGetPos( m );
	ASSERT( pCameraPos );

	AuPOS	posCamera;
	posCamera.x	= pCameraPos->x;
	posCamera.y	= pCameraPos->y;
	posCamera.z	= pCameraPos->z;

	if( g_pcsAgcmEventNature )
	{
		if( posCamera.y > 10000.0f / 1.2f )
			g_pcsAgcmEventNature->SetSkyHeight( posCamera.y * 1.2f );
		else
			g_pcsAgcmEventNature->SetSkyHeight( 10000.0f			);

		g_pcsAgcmEventNature->SetCharacterPosition( posCamera );
	}

}
