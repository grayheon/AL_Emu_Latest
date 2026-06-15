// AcUIWorldmap.cpp: implementation of the AcUIWorldmap class.
//
//////////////////////////////////////////////////////////////////////
#include "AcUIWorldmap.h"
#include "AgcmUIManager2.h"

#include "AgcmCharacter.h"
#include "AgcmUIChatting2.h"

#include "AuStrTable.h"
#include "AgpmSiegeWar.h"
//////////////////////////////////////////////////////////////////////////
/* 월드맵 수정 법
//////////////////////////////////////////////////////////////////////////
//기획파트
1.INI/WORLDMAPNAME.XML에 추가하고자 하는 지역 이름을 넣는다.
2.리전 툴 작업
-해당 리전에 리전 텍스쳐와 로드 텍스쳐 세팅
-그룹과 상위 그룹 세팅
-월드좌표 계산
-월드맵 텍스쳐 액티브 영역 세팅 하기

//그래픽파트	
1.UI에서 [UI_WORLD_MAP]에서 
-존 하이라트 컨트롤 추가

//프로그래밍파트
1.UI_1024X768.INI파일에서
-존 하이라트 컨트롤에 CtrlGroup(상위그룹), CtrlSubGroup(자신의 그룹)을 직접 추가
(그룹 이름은 WORLDMAPNAME.XML에 추가한 이름과 동일해야 함)
*/
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void AcUIWorldmapRender::OnWindowRender()
{ 
	if( m_pAgcmUIWorldmap && m_pAgcmUIWorldmap->m_pUIWorldMap->m_eStatus != AGCDUI_STATUS_OPENING ) 
	{
		m_pAgcmUIWorldmap->m_DrawMark.Render( m_pAgcmUIWorldmap->m_pcsUIMgr->m_v2dCurMousePos, m_pAgcmUIWorldmap->m_iCurGroup, m_pAgcmUIWorldmap->m_bShowWorldMap ); 
		m_pAgcmUIWorldmap->m_pcsUIMgr->RefreshUserData( m_pAgcmUIWorldmap->m_pcdUIUserData, TRUE );
	}
};

AgcmUIWorldmap::AgcmUIWorldmap()
: m_pcsUIMgr(NULL)
,m_pcsApmMap(NULL)
,m_pAgpdSelfChar(NULL)
,m_pUIWorldMap(NULL)
,m_pcsParty(NULL)
,m_pcsAgpmChar(NULL)
,m_pUDWorldName(NULL)
,m_pUDRegionName(NULL)
,m_bChecking(FALSE)
,m_bShowRoadMap(FALSE)
,m_iCurGroup(0)
,m_nEventGetBtnGuildMarkHuman(-1)
,m_nEventGetBtnGuildMarkHuman2(-1)
,m_nEventGetBtnGuildMarkHuman3(-1)
,m_nEventGetBtnGuildMarkOrc(-1)
,m_nEventGetBtnGuildMarkOrc2(-1)
,m_nEventGetBtnGuildMarkOrc3(-1)
,m_nEventGetBtnGuildMarkMoonElf(-1)
,m_nEventGetBtnGuildMarkMoonElf2(-1)
,m_nEventGetBtnGuildMarkMoonElf3(-1)
,m_nEventGetBtnGuildMarkDragonSion(-1)
,m_nEventGetBtnGuildMarkDragonSion2(-1)
,m_nEventGetBtnGuildMarkDragonSion3(-1)
,m_nEventGetBtnGuildMarkArchlord(-1)
,m_nEventGetBtnGuildMarkArchlord2(-1)
,m_nEventGetBtnGuildMarkArchlord3(-1)
,m_pCastleGuildMarkHuman(NULL)
,m_pCastleGuildMarkHuman2(NULL)
,m_pCastleGuildMarkHuman3(NULL)
,m_pCastleGuildMarkOrc(NULL)
,m_pCastleGuildMarkOrc2(NULL)
,m_pCastleGuildMarkOrc3(NULL)
,m_pCastleGuildMarkMoonElf(NULL)
,m_pCastleGuildMarkMoonElf2(NULL)
,m_pCastleGuildMarkMoonElf3(NULL)
,m_pCastleGuildMarkDragonSion(NULL)
,m_pCastleGuildMarkDragonSion2(NULL)
,m_pCastleGuildMarkDragonSion3(NULL)
,m_pCastleGuildMarkArchlord(NULL)
,m_pCastleGuildMarkArchlord2(NULL)
,m_pCastleGuildMarkArchlord3(NULL)
,m_pcdUIUserData(NULL)
,m_bViewCastleInfo(FALSE)
{
	SetModuleName("AgcmUIWorldmap");
	EnableIdle( TRUE );
	memset( m_strDisplayText, 0, sizeof( CHAR ) * 32 );
}

AgcmUIWorldmap::~AgcmUIWorldmap()
{

}

BOOL AgcmUIWorldmap::OnAddModule()
{
	m_pcsUIMgr =		(AgcmUIManager2 *)	GetModule("AgcmUIManager2");
	m_pcsApmMap =	(ApmMap*)GetModule("ApmMap");
	m_pcsParty =		(AgpmParty*)GetModule("AgpmParty");
	m_pcsAgpmChar = (AgpmCharacter*)GetModule("AgpmCharacter");
	m_pcsAgcmUIChatting2	= ( AgcmUIChatting2*)		GetModule( "AgcmUIChatting2" );
	if( NULL == m_pcsUIMgr || NULL == m_pcsApmMap || NULL == m_pcsParty || NULL == m_pcsAgpmChar || NULL == m_pcsAgcmUIChatting2)
		return FALSE;

	AgcmCharacter* pAgcmChar = (AgcmCharacter*)GetModule("AgcmCharacter");
	pAgcmChar->SetCallbackSelfRegionChange	( CBBindingRegionChange , this );

	if( AddEvent() == FALSE )
		return FALSE;

	if( AddFunction() == FALSE )
		return FALSE;

	if( AddDisplay() == FALSE )
		return FALSE;

	if( AddUserData() == FALSE )
		return FALSE;

	if( AddBoolean() == FALSE )
		return FALSE;

	return TRUE;
}

BOOL	AgcmUIWorldmap::OnInit()
{
	//월드맵 지역 이름 읽기
	//_LoadWorldmapName();
	WORLD_MAP::LoadWorldName();

	m_DrawMark.Init( (AgcmFont*)GetModule("AgcmFont")  );
	// 내 표시
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_SELF,  "Main_Minimap_Cursor.png" );

	//다른 PC 표시
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_PARTY,  "Main_Minimap_Pos_Member.png" );

	//위치 표시
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_X_MARK,  "Main_Minimap_Pos_Point_a.png" );
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_X_MARK,  "Main_Minimap_Pos_Point_b.png" );
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_X_MARK,  "Main_Minimap_Pos_Point_c.png" );
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_X_MARK,  "Main_Minimap_Pos_Point_d.png" );

	//NPC들
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_NPC,  "Main_Minimap_Pos_NPC0.png" );
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_NPC_QUEST_NEW,  "Main_Minimap_Pos_NPC2.png" );
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_NPC_QUEST_COMPLETE,  "Main_Minimap_Pos_NPC1.png" );

	//PC
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_PC,  "Main_Minimap_Pos_PC.png" );

	//SIEGE
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_SIEGE,  "Main_Minimap_Pos_Siege0.png" );
	m_DrawMark.AddTexture(AGUIDrawMark::E_TYPE_SIEGE_DESTROY,  "Main_Minimap_Pos_Siege1.png" );

	m_WorldMapRender.OnInit();
	m_WorldMapRender.Set( this );

	m_pcsUIMgr->AddCustomControl( "WorldMap", &m_WorldMapRender );

	return TRUE;
}

int AgcmUIWorldmap::_GetGroupIndex( const char* strGroupName )
{
	if( strGroupName && strlen(strGroupName) )
	{
		for( int i=0; i<(int)WORLD_MAP::vecGroupName.size(); ++i )
		{
			if( strcmp( strGroupName, WORLD_MAP::vecGroupName[i].c_str() ) == 0 )
				return i;
		}
	}

	return -1;
}

BOOL	AgcmUIWorldmap::OnDestroy()
{
	if(m_pUIWorldMap && m_pUIWorldMap->m_pcsUIWindow)
		m_pUIWorldMap->m_pcsUIWindow->m_bPreLoad = FALSE;
	return TRUE;
}

BOOL	AgcmUIWorldmap::OnIdle(UINT32 ulClockCount)
{
	//매 프레임 파티원들을 체킹 할 필요 없다.
	if( m_bChecking )
	{
		if( m_pUIWorldMap && (m_pUIWorldMap->m_eStatus == AGCDUI_STATUS_ADDED || m_pUIWorldMap->m_eStatus == AGCDUI_STATUS_CLOSING) )
		{	
			m_bChecking = FALSE;
	}

		//파티원 세팅
		//파티원들 실시간 추가 삭제 처리를 위해 이부분에 처리 하였다. 
		AgpdParty* pParty = m_pcsParty->GetParty( m_pAgpdSelfChar );
		static BOOL bDone = FALSE;
		if( pParty )
	{
			std::vector< INT32 > vecID;
			vecID.reserve( pParty->m_nCurrentMember );
			for( int i=0; i<pParty->m_nCurrentMember; ++i )
	{
				INT32 iID = pParty->m_lMemberListID[i];
				vecID.push_back( iID );

				//파티원을 가져오기
				AgpdCharacter* pChar = m_pcsAgpmChar->GetCharacter( iID );
				if( NULL == pChar )
					continue;

				//월드 맵에 추가된 파티원 가져 오기
				AGUIDrawMark::LIST_MARK_INFO_ITR it = m_DrawMark.Find( iID );

				//이미 추가 되어 있으면 패스
				if( it != m_DrawMark.MarkListEnd() )
					continue;

				//해당 파티원의 리전 정보 가져오기
				ApmMap::RegionTemplate* pRegionTemplate =	m_pcsApmMap->GetTemplate( pChar->m_nBindingRegionIndex );
				if( NULL == pRegionTemplate )
					return FALSE;

				//맵 정보 얻어오기
				ApmMap::WorldMap* pMapInfo = m_pcsApmMap->GetWorldMapInfo( pRegionTemplate->nWorldMapIndex );
				if( NULL == pMapInfo )
					return FALSE;

				//그룹 인덱스 얻어오기
				int iGroup =  _GetGroupIndex( pMapInfo->szGroupName );

				//월드맵에 표시할 마킹정보 추가
				m_DrawMark.Add( pChar, iGroup, AGUIDrawMark::E_TYPE_PARTY  );
	}

			m_DrawMark.UpdateData( vecID,  AGUIDrawMark::E_TYPE_PARTY );
			bDone = FALSE;
	}
		else if( FALSE == bDone )
	{
			std::vector< INT32 > vecID;
			m_DrawMark.UpdateData( vecID,  AGUIDrawMark::E_TYPE_PARTY );
			bDone = TRUE;
	}

	}
	return TRUE;
}

BOOL AgcmUIWorldmap::AddEvent()
{
	m_nEventPaperSound = m_pcsUIMgr->AddEvent( "MapUISoundEvent" );

	if( m_nEventGetBtnGuildMarkHuman < 0 )
	{
		m_nEventGetBtnGuildMarkHuman = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleHuman" );
		if( m_nEventGetBtnGuildMarkHuman < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkHuman2 < 0 )
	{
		m_nEventGetBtnGuildMarkHuman2 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleHuman2" );
		if( m_nEventGetBtnGuildMarkHuman2 < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkHuman3 < 0 )
	{
		m_nEventGetBtnGuildMarkHuman3 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleHuman3" );
		if( m_nEventGetBtnGuildMarkHuman3 < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkOrc < 0 )
	{
		m_nEventGetBtnGuildMarkOrc = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleOrc" );
		if( m_nEventGetBtnGuildMarkOrc < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkOrc2 < 0 )
	{
		m_nEventGetBtnGuildMarkOrc2 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleOrc2" );
		if( m_nEventGetBtnGuildMarkOrc2 < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkOrc3 < 0 )
	{
		m_nEventGetBtnGuildMarkOrc3 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleOrc3" );
		if( m_nEventGetBtnGuildMarkOrc3 < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkMoonElf < 0 )
	{
		m_nEventGetBtnGuildMarkMoonElf = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleMoonElf" );
		if( m_nEventGetBtnGuildMarkMoonElf < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkMoonElf2 < 0 )
	{
		m_nEventGetBtnGuildMarkMoonElf2 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleMoonElf2" );
		if( m_nEventGetBtnGuildMarkMoonElf2 < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkMoonElf3 < 0 )
	{
		m_nEventGetBtnGuildMarkMoonElf3 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleMoonElf3" );
		if( m_nEventGetBtnGuildMarkMoonElf3 < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkDragonSion < 0 )
	{
		m_nEventGetBtnGuildMarkDragonSion = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleDragonSion" );
		if( m_nEventGetBtnGuildMarkDragonSion < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkDragonSion2 < 0 )
	{
		m_nEventGetBtnGuildMarkDragonSion2 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleDragonSion2" );
		if( m_nEventGetBtnGuildMarkDragonSion2 < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkDragonSion3 < 0 )
	{
		m_nEventGetBtnGuildMarkDragonSion3 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleDragonSion3" );
		if( m_nEventGetBtnGuildMarkDragonSion3 < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkArchlord < 0 )
	{
		m_nEventGetBtnGuildMarkArchlord = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleArchlord" );
		if( m_nEventGetBtnGuildMarkArchlord < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkArchlord2 < 0 )
	{
		m_nEventGetBtnGuildMarkArchlord2 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleArchlord2" );
		if( m_nEventGetBtnGuildMarkArchlord2 < 0 )
			return FALSE;
	}

	if( m_nEventGetBtnGuildMarkArchlord3 < 0 )
	{
		m_nEventGetBtnGuildMarkArchlord3 = m_pcsUIMgr->AddEvent( "WorldMap_GetBtnCastleArchlord3" );
		if( m_nEventGetBtnGuildMarkArchlord3 < 0 )
			return FALSE;
	}

	return TRUE;
}

BOOL	AgcmUIWorldmap::AddFunction()
{
	m_pcsUIMgr->AddFunction( this, "WorldMap_Toggle",			CBWorldMapToggle,					0 );	//월드맵 ui open / close
	m_pcsUIMgr->AddFunction( this, "WorldMap_RightClick",		CBRightClick,						0 );	//월드맵 상 마우스 우클릭
	m_pcsUIMgr->AddFunction( this, "WorldMap_LeftClick",		CBLeftClick,						0 );	//	    ""                    좌클릭
	m_pcsUIMgr->AddFunction( this, "WorldMap_LoadMapToggle",	CBRoadMapToggle,					0 );   //월드맵 로드맵 open / close
	m_pcsUIMgr->AddFunction( this, "WorldMap_MapEventPos",		CBMapEventPos,						0 );   //맵상에 설명 추가

	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleHuman", CB_GetBtnCastleHuman, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleHuman2", CB_GetBtnCastleHuman2, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleHuman3", CB_GetBtnCastleHuman3, 0 );

	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleOrc", CB_GetBtnCastleOrc, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleOrc2", CB_GetBtnCastleOrc2, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleOrc3", CB_GetBtnCastleOrc3, 0 );

	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleMoonElf", CB_GetBtnCastleMoonElf, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleMoonElf2", CB_GetBtnCastleMoonElf2, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleMoonElf3", CB_GetBtnCastleMoonElf3, 0 );

	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleDragonSion", CB_GetBtnCastleDragonSion, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleDragonSion2", CB_GetBtnCastleDragonSion2, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleDragonSion3", CB_GetBtnCastleDragonSion3, 0 );

	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleArchlord", CB_GetBtnCastleArchlord, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleArchlord2", CB_GetBtnCastleArchlord2, 0 );
	m_pcsUIMgr->AddFunction( this, "WorldMap_GetBtnCastleArchlord3", CB_GetBtnCastleArchlord3, 0 );

	return TRUE;
}

BOOL AgcmUIWorldmap::AddDisplay()
{
	m_pcsUIMgr->AddDisplay( this, "WorldMap_WorldName",	1,	CBDisplayWorldName,	AGCDUI_USERDATA_TYPE_INT32 );
	m_pcsUIMgr->AddDisplay( this, "WorldMap_RegionName",	2,	CBDisplayRegionName,	AGCDUI_USERDATA_TYPE_INT32 );

	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerGuildHuman", 0, CB_OnDisplayOwnerGuildHuman, AGCDUI_USERDATA_TYPE_STRING );
	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerNameHuman", 0, CB_OnDisplayOwnerNameHuman, AGCDUI_USERDATA_TYPE_STRING );

	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerGuildOrc", 0, CB_OnDisplayOwnerGuildOrc, AGCDUI_USERDATA_TYPE_STRING );
	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerNameOrc", 0, CB_OnDisplayOwnerNameOrc, AGCDUI_USERDATA_TYPE_STRING );

	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerGuildMoonElf", 0, CB_OnDisplayOwnerGuildMoonElf, AGCDUI_USERDATA_TYPE_STRING );
	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerNameMoonElf", 0, CB_OnDisplayOwnerNameMoonElf, AGCDUI_USERDATA_TYPE_STRING );

	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerGuildDragonScion", 0, CB_OnDisplayOwnerGuildDragonScion, AGCDUI_USERDATA_TYPE_STRING );
	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerNameDragonScion", 0, CB_OnDisplayOwnerNameDragonScion, AGCDUI_USERDATA_TYPE_STRING );

	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerGuildArchlord", 0, CB_OnDisplayOwnerGuildArchlord, AGCDUI_USERDATA_TYPE_STRING );
	m_pcsUIMgr->AddDisplay( this, "WorldMap_DisplayOwnerNameArchlord", 0, CB_OnDisplayOwnerNameArchlord, AGCDUI_USERDATA_TYPE_STRING );


	return TRUE;
}

BOOL	AgcmUIWorldmap::AddUserData()
{

	INT32		iDummyData;
	m_pUDWorldName = m_pcsUIMgr->AddUserData( "WorldMap_WorldName",  &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );
	if( m_pUDWorldName == NULL )
		return FALSE;

	m_pUDRegionName = m_pcsUIMgr->AddUserData( "WorldMap_RegionName",  &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );
	if( m_pUDRegionName == NULL )
		return FALSE;

	m_pcdUIUserData = m_pcsUIMgr->AddUserData( "WorldMap_UserData", m_strDisplayText, sizeof( CHAR ), 32, AGCDUI_USERDATA_TYPE_STRING );


	return TRUE;
}

BOOL	AgcmUIWorldmap::AddBoolean()
{

	return TRUE;
}

//맵 체인지 될때 모든 컨트롤 숨긴다.
BOOL AgcmUIWorldmap::AllHideControl()
{
	int iCount = 0;
	for( int i=0; i<(int)m_vecCtrlGroup.size(); ++i )
	{
		iCount = (int)m_vecCtrlGroup[i].size();
		for( int j=0;j<iCount; ++j )
		{
			if( m_vecCtrlGroup[i][j] )
		{
				m_vecCtrlGroup[i][j]->m_pcsBase->ShowWindow( FALSE );
		}
	}
	}
	return TRUE;
}

//해당 그룹에 속하는 컨트롤만 보여지게 한다.
BOOL AgcmUIWorldmap::ShowControl( int iGroup )
{
	if( iGroup ==  -1 )
		return FALSE;

	int iGroupCount = (int)WORLD_MAP::vecGroupName.size();
	if(iGroup < iGroupCount )
			{
		for( int j=0;j<(int)m_vecCtrlGroup[iGroup].size(); ++j )
				{
			if( m_vecCtrlGroup[iGroup][j] )
				{
				m_vecCtrlGroup[iGroup][j]->m_pcsBase->ShowWindow( TRUE );
				}
			}
			}

	if(  FALSE == m_bShowRoadMap )
		{
		for( int j=0;j<(int)m_vecCtrlGroup[iGroupCount].size(); ++j )
			{
			if( m_vecCtrlGroup[iGroupCount][j] )
				{
				m_vecCtrlGroup[iGroupCount][j]->m_pcsBase->ShowWindow( TRUE );
				}
				}
			}
	return TRUE;
}

//UI.INI파일이 다 읽혀진 직후에 그룹화를 처리 한다.
BOOL AgcmUIWorldmap::SetGrouping()
{
	m_pUIWorldMap = m_pcsUIMgr->GetUI( "UI_WORLD_MAP" );
	if( NULL == m_pUIWorldMap )
		return FALSE;
	// 	
	// 	//월드맵 UI위치 잡기
	// 	LPDIRECT3DDEVICE9 pDevice = (LPDIRECT3DDEVICE9)RwD3D9GetCurrentD3DDevice();
	// 	LPDIRECT3DSURFACE9		pBackBuffer = NULL;
	// 	D3DSURFACE_DESC	surf_desc;
	// 
	// 	pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
	// 	pBackBuffer->GetDesc( &surf_desc );
	// 	pBackBuffer->Release();
	// 	
	// 	//minasang: 오른쪽하단에 붙이깅.
	// 	//int x = surf_desc.Width / 2 - m_pUIWorldMap->m_pcsUIWindow->w / 2 ;
	// 	//int y = surf_desc.Height/ 2 - m_pUIWorldMap->m_pcsUIWindow->h / 2 ;
	// 	int x = surf_desc.Width - m_pUIWorldMap->m_pcsUIWindow->w;
	// 	int y = surf_desc.Height - m_pUIWorldMap->m_pcsUIWindow->h;
	// 	m_pUIWorldMap->m_pcsUIWindow->MoveWindow( x,y);

	//컨트롤 그룹핑
	int iGroupCount = (int)WORLD_MAP::vecGroupName.size();
	m_vecCtrlGroup.reserve( iGroupCount + 1 );
	m_vecCtrlGroup.resize( iGroupCount  + 1);

	AgcdUIControl *pControl = NULL;
	INT32		i=0;
	for ( pControl = m_pcsUIMgr->GetSequenceControl(m_pUIWorldMap, &i); pControl; pControl = m_pcsUIMgr->GetSequenceControl(m_pUIWorldMap, &i) )
	{
		if( pControl )
		{

			pControl->m_pcsBase->m_csTextureList.LoadTextures();

			int iGroup = _GetGroupIndex( pControl->m_szGroupName );
			if( iGroup > -1 )
		{
				pControl->m_pcsBase->ShowWindow( FALSE );
				m_vecCtrlGroup[iGroup].push_back( pControl );
		}
			//이외 컨트롤들
			else
		{
				if( pControl->m_lType != AcUIBase::TYPE_CUSTOM )
					m_vecCtrlGroup[iGroupCount].push_back( pControl );
		}

		}
		}

	//AllHideControl();
	return TRUE;
}

BOOL AgcmUIWorldmap::AddPoint( const char* pComment, RwV2d vPos, AGUIDrawMark::E_MARKINFO_TYPE eType , INT iRegionIndex, BOOL bTempShow )
{
	// 아뒤 만들기
	INT	iID	=	(INT)(vPos.x + vPos.y );	
	iID		+=	strlen( pComment );	
	iID		=	abs( iID ); 

	ApmMap::RegionTemplate*	pRegionTemplate	=	NULL;

	if( iRegionIndex == -1 )
	{
		INT16 iRegion = m_pcsApmMap->GetRegion( vPos.x, vPos.y );
		pRegionTemplate =	m_pcsApmMap->GetTemplate( iRegion );
	}
	else
	{
		pRegionTemplate =	m_pcsApmMap->GetTemplate( iRegionIndex );
	}

	if( NULL == pRegionTemplate )
		return FALSE;

	ApmMap::WorldMap* pMapInfo = m_pcsApmMap->GetWorldMapInfo( pRegionTemplate->nWorldMapIndex );
	if( NULL == pMapInfo )
		return FALSE;

	int iGroupIndex = _GetGroupIndex( pMapInfo->szGroupName );
	m_DrawMark.Add( iID, vPos , (char*)pComment, iGroupIndex, eType, bTempShow );

	return TRUE;
}

BOOL	AgcmUIWorldmap::RemovePoint( const char* pComment, RwV2d vPos  )
{
	INT	iID	=	(INT)(vPos.x + vPos.y );
	iID		+=	strlen( pComment );
	iID		=	abs( iID );
	return	m_DrawMark.Delete( iID );
}

BOOL AgcmUIWorldmap::RemovePoint( INT32 iID )
{
	return m_DrawMark.Delete(iID);
}

BOOL AgcmUIWorldmap::OpenWorldMap( INT nRegionTemplateIndex )
{
	AgcmCharacter* pAgcmChar = (AgcmCharacter*)GetModule("AgcmCharacter");
	if( NULL == pAgcmChar )
		return FALSE;

	m_pAgpdSelfChar = pAgcmChar->GetSelfCharacter();
	if( NULL == m_pAgpdSelfChar )
		return FALSE;

	ApmMap::RegionTemplate* pRegionTemplate =	NULL;

	if( nRegionTemplateIndex == -1 )
	{
		pRegionTemplate	=	m_pcsApmMap->GetTemplate( m_pAgpdSelfChar->m_nBindingRegionIndex );
	}

	else
	{
		pRegionTemplate		=	m_pcsApmMap->GetTemplate( nRegionTemplateIndex );
	}

	if( !pRegionTemplate )
		return FALSE;

	if( pRegionTemplate->nWorldMapIndex == -1 )
		return FALSE;

	if( _ChangeMap( pRegionTemplate->nWorldMapIndex ) == FALSE )
		return FALSE;

	m_pcsUIMgr->OpenUI( m_pUIWorldMap );
	//minasang: sound 
	m_pcsUIMgr->ThrowEvent( m_nEventPaperSound );

	m_pUIWorldMap->m_pcsUIWindow->m_bPreLoad = TRUE;

	return TRUE;
}

BOOL AgcmUIWorldmap::CloseWorldMap()
{
	S_Close( this, m_pUIWorldMap );
	m_bShowWorldMap = FALSE;

	m_DrawMark.DeleteTempShowElements();

	return TRUE;
}

BOOL	AgcmUIWorldmap::_ChangeMap( INT32 iWorldMapIndex )
{
	ApmMap::WorldMap* pMapInfo = m_pcsApmMap->GetWorldMapInfo( iWorldMapIndex );
	if( NULL == pMapInfo )
		return FALSE;

	int iGroupIndex = _GetGroupIndex(pMapInfo->szGroupName);

	// 적당한 맵이 없을 때
	if( -1 == iGroupIndex )
		return FALSE;

	S_ChangeMap( this, m_pUIWorldMap,  pMapInfo->szGroupName );

	m_DrawMark.SetMapUI( m_pUIWorldMap );

	//내꺼 세팅
	m_DrawMark.Add( m_pAgpdSelfChar,iGroupIndex, AGUIDrawMark::E_TYPE_SELF );

	//마우스 리전 이름 세팅
	m_bChecking = TRUE;

	m_bShowWorldMap = TRUE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//static function
//////////////////////////////////////////////////////////////////////////
void AgcmUIWorldmap::S_ChangeMap(AgcmUIWorldmap* pThis, AgcdUI* pWorlMap, const char* pszGroupName )
{
	if( pszGroupName == NULL || strlen( pszGroupName ) == 0 )
		return;

	pThis->AllHideControl();

	pThis->m_iCurGroup = pThis->_GetGroupIndex( pszGroupName );

	//텍스쳐 로딩
	ApmMap::WorldMap* pMapInfo = pThis->m_pcsApmMap->GetWorldMapInfoByGroupName( pszGroupName );
	if(  pMapInfo )
	{
		// 최상단 인덱스는 월드 대륙이다.
		// 이 곳에 성주를 표시해줘야 한다.
		if(pMapInfo->nMID == 1)
		{
			pThis->OnGetControls();
			pThis->m_bViewCastleInfo = TRUE;
		}
		else
		{
			pThis->m_bViewCastleInfo = FALSE;
		}


		//월드맵 UI위치 잡기
		RwRaster * raster = RwCameraGetRaster( g_pEngine->m_pCamera );

		if(raster)
		{
			int x = pWorlMap->m_pcsUIWindow->x;
			int y = pWorlMap->m_pcsUIWindow->y;

			// 화면 벗어났는지 체크
			if(x+15 > (int)raster->width)
				x = raster->width - pWorlMap->m_pcsUIWindow->w;
			else if(x < 0)
				x = 0;
			if(y > (int)raster->height)
				y = raster->height - pWorlMap->m_pcsUIWindow->h;
			else if(y < 0)
				y = 0;

			pWorlMap->m_pcsUIWindow->MoveWindow(x, y);
		}

		char* pszTexName = NULL;

		//월드맵의 배경 텍스쳐 
		if( pThis->m_bShowRoadMap )
				{
			pszTexName = pMapInfo->szRegionRoadTexture;
				}
				else
				{
			pszTexName = pMapInfo->szRegionTexture;
				}

		//이미지 추가
		int iIndex =  pWorlMap->m_pcsUIWindow->AddOnlyThisImage( pszTexName );
		pWorlMap->m_pcsUIWindow->SetRednerTexture( iIndex + 1 );

		//맵 영역 계산 - 리전툴에서 월드맵 텍스쳐 빈 여백을 제외한 실제 표시 영역을 월드 좌표롤 바꾼값들
		pThis->m_DrawMark.SetMapArea( AGUIDrawMark::S_MAP_AREA(  (INT32) pMapInfo->xStart,  (INT32) pMapInfo->zStart, (INT32) pMapInfo->xEnd, (INT32) pMapInfo->zEnd  ) );

		//리전툴에서 월드맵 텍스쳐 빈 여백을 제외한 실제 표시 영역
		AGUIDrawMark::S_MAP_AREA stArea(  pMapInfo->iSX,  pMapInfo->iSY, pMapInfo->iEX, pMapInfo->iEY  );
		//세팅이 안되어 있거나 로드맵이 떴을 경우 매칭영역은 텍스쳐 크기로
		if(pMapInfo->iSX == 0 && pMapInfo->iEX == 0 || pMapInfo->iSY == 0 &&  pMapInfo->iEY  )
		{
			stArea.SX = 0; 
			stArea.SZ = pWorlMap->m_pcsUIWindow->w;
			stArea.EX = 0;
			stArea.EZ = pWorlMap->m_pcsUIWindow->h;
					}
		pThis->m_DrawMark.SetUIMatchingArea(  stArea );

				}
	//해당 이미지가 없을경우
	else
			{

			}

	if( pThis->m_bShowRoadMap )
	{
		//UI 마우스 이벤트 안먹히게
		pWorlMap->m_pcsUIWindow->m_bIgnore = TRUE;
		}
		else
		{
		//그룹에 속한 컨트롤들 보여준다.
		pThis->ShowControl( pThis->m_iCurGroup );
		//마우스 이벤트 받아 들이자
		pWorlMap->m_pcsUIWindow->m_bIgnore = FALSE;
	}

	//월드 이름 변경
	pThis->m_pcsUIMgr->SetUserDataRefresh( pThis->m_pUDWorldName );


}

//////////////////////////////////////////////////////////////////////////
//CALLBACK FUNCTION
//////////////////////////////////////////////////////////////////////////
BOOL AgcmUIWorldmap::CBWorldMapToggle(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass || NULL == pData1 )
		return FALSE;

	AgcmUIWorldmap* pThis = (AgcmUIWorldmap*)pClass;
	AgcdUI* pWorlMap = (AgcdUI*)pData1;

	if( pThis->m_bShowRoadMap )
	{
		pThis->m_pcsUIMgr->CloseUI( pWorlMap , 0, 0, 0, 0 );

		pThis->m_bChecking = FALSE;
		pThis->m_bShowRoadMap = FALSE;
			}

	//minasang: sound
	pThis->m_pcsUIMgr->ThrowEvent( pThis->m_nEventPaperSound );

	//닫힌 상태라면
	if( pWorlMap->m_eStatus == AGCDUI_STATUS_ADDED )
	{
		return pThis->OpenWorldMap();
	}

	//열린 상태
	else 
	{
		return pThis->CloseWorldMap();
		}

		return TRUE;
}

void AgcmUIWorldmap::S_Close(AgcmUIWorldmap* pThis, AgcdUI* pWorlMap)
{
	pThis->m_pcsUIMgr->CloseUI( pWorlMap );
	pThis->m_bChecking = FALSE;					//맵이 닫히면 파티원 체킹을 끄자...
	pThis->m_bViewCastleInfo = FALSE;
}

BOOL AgcmUIWorldmap::CBRightClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass || NULL == pData1 )
		return FALSE;

	AgcmUIWorldmap* pThis = (AgcmUIWorldmap*)pClass;
	AgcdUI* pWorlMap = (AgcdUI*)pData1;

	//shift + 우클릭 이동처리
	if( g_pEngine->IsLShiftDown() )
	{
		AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )pThis->GetModule( "AgcmUIChatting2" );
		if( pcmUIChatting )
		{
			AGUIDrawMark::S_MAP_POS stPos = pThis->m_DrawMark.GetWorldPos( AGUIDrawMark::S_MAP_POS( (int)pThis->m_pcsUIMgr->m_v2dCurMousePos.x, (int)pThis->m_pcsUIMgr->m_v2dCurMousePos.y) );

			char strMessage[ 256 ];
			sprintf( strMessage , "/move %f,%f" , (float)stPos.x , (float)stPos.y );

			pcmUIChatting->OnParseCommand( strMessage );
		}

		return TRUE;
			}

	if( pThis->m_iCurGroup == -1 || pThis->m_iCurGroup >=(int) WORLD_MAP::vecGroupName.size() )
				return FALSE;

	//현재 열려있는 맵의 그룹에 해당하는 맵정보를 얻어 온다.
	ApmMap::WorldMap* pMapInfo = pThis->m_pcsApmMap->GetWorldMapInfoByGroupName( WORLD_MAP::vecGroupName[pThis->m_iCurGroup].c_str() );
	if( NULL == pMapInfo )
			return FALSE;

	//현재 맵의 상위 단계 그룹으로 맵을 바꾼다.
	S_ChangeMap( pThis, pWorlMap, pMapInfo->szParentGroupNmae );
	return TRUE;
}

BOOL AgcmUIWorldmap::CBLeftClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass || NULL == pData1 || NULL == pcsSourceControl )
		return FALSE;

	//컨트롤 + 좌클릭 : 맵상에 x표시 및 설명 추가
	if( g_pEngine->IsCtrlDown() )
	{
		CBMapEventPos(pClass, pData1, pData2, pData3, pData4, pData5, pcsTarget, pcsSourceControl );
		return FALSE;
	}

	AgcmUIWorldmap* pThis = (AgcmUIWorldmap*)pClass;
	AgcdUI* pWorlMap = (AgcdUI*)pData1;

	//선택한 컨트롤의 하위단계 그룹으로 맵 교체
	S_ChangeMap( pThis, pWorlMap, pcsSourceControl->m_szSubGroupName );
	return TRUE;
}

BOOL AgcmUIWorldmap::CBRoadMapToggle(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass || NULL == pData1 )
		return FALSE;

	AgcmUIWorldmap* pThis = (AgcmUIWorldmap*)pClass;
	AgcdUI* pWorlMap = (AgcdUI*)pData1;

	AgcmCharacter* pAgcmChar = (AgcmCharacter*)pThis->GetModule("AgcmCharacter");
	if( NULL == pAgcmChar )
	return FALSE;

	pThis->m_pAgpdSelfChar = pAgcmChar->GetSelfCharacter();
	if( NULL == pThis->m_pAgpdSelfChar )
		return FALSE;

	//플래그 변경
	pThis->m_bShowRoadMap = !pThis->m_bShowRoadMap;

	//minasang: sound
	pThis->m_pcsUIMgr->ThrowEvent( pThis->m_nEventPaperSound );

	//로드맵이 열릴때
	if( pThis->m_bShowRoadMap  )
	{
		pThis->m_pcsUIMgr->OpenUI( pWorlMap );
		ApmMap::RegionTemplate* pRegionTemplate = pThis->m_pcsApmMap->GetTemplate( pThis->m_pAgpdSelfChar->m_nBindingRegionIndex );
		if( pRegionTemplate )
			pThis->_ChangeMap( pRegionTemplate->nWorldMapIndex );

	}
	//로드맵이 닫힐때
	else
	{
		S_Close(pThis, pWorlMap);
	}

	return TRUE;
}

BOOL AgcmUIWorldmap::CBMapEventPos(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass || NULL == pData1 )
		return FALSE;

	AgcmUIWorldmap* pThis = (AgcmUIWorldmap*)pClass;
	AgcdUI* pWorlMap = (AgcdUI*)pData1;

	if( FALSE == g_pEngine->IsCtrlDown() )
		return FALSE;

	INT32 iCurMark = pThis->m_DrawMark.GetHitMark();

	//기존 표시 삭제
	if( iCurMark > -1 )
	{
		pThis->m_DrawMark.Delete( iCurMark, AGUIDrawMark::E_TYPE_X_MARK );
	}
	//표시 추가
	else
	{
		CHAR	szName[ MAX_PATH ];
		ZeroMemory( szName , MAX_PATH );

		AGUIDrawMark::S_MAP_POS stPos = pThis->m_DrawMark.GetWorldPos( AGUIDrawMark::S_MAP_POS( (int)pThis->m_pcsUIMgr->m_v2dCurMousePos.x, (int)pThis->m_pcsUIMgr->m_v2dCurMousePos.y) );

		CHAR*	pszMessage	=	pThis->m_pcsUIMgr->GetUIMessage( "Position_NameInput_Message" );
		if( pThis->m_pcsUIMgr->ActionMessageEditOKCancelDialog( pszMessage , szName  )  == FALSE )
			return FALSE;

		INT	nID	=	(INT)(pThis->m_pcsUIMgr->m_v2dCurMousePos.x + pThis->m_pcsUIMgr->m_v2dCurMousePos.y );

		nID		+=	strlen( szName );
		nID		=	abs( nID );

		RwV2d vPos;
		vPos.x = (float)stPos.x;
		vPos.y = (float)stPos.y;

		INT16 iRegion = pThis->m_pcsApmMap->GetRegion( vPos.x, vPos.y );
		ApmMap::RegionTemplate* pRegionTemplate =	pThis->m_pcsApmMap->GetTemplate( iRegion );
		if( NULL == pRegionTemplate )
			return FALSE;

		ApmMap::WorldMap* pMapInfo = pThis->m_pcsApmMap->GetWorldMapInfo( pRegionTemplate->nWorldMapIndex );
		if( NULL == pMapInfo )
			return FALSE;

		int iGroup = pThis->_GetGroupIndex( pMapInfo->szGroupName );
		BOOL bRet= pThis->m_DrawMark.Add( nID, vPos , szName, iGroup, AGUIDrawMark::E_TYPE_X_MARK );

		if( bRet )
		{
			pThis->m_pcsAgcmUIChatting2->OnReadyMapPosRegister( szName , nID , stPos.x , stPos.y );
		}


	}

	return TRUE;
}

BOOL	AgcmUIWorldmap::CBBindingRegionChange(PVOID	pData,PVOID	pClass,PVOID	pCustData)
{
	AgcmUIWorldmap*	pThis		= (AgcmUIWorldmap*) pClass;
	AgpdCharacter*	pdCharacter = (AgpdCharacter*)	pData;
	INT16 nPrevRegionIndex = pCustData ? *(INT16*)pCustData : -1;	// NULL 로 올때는 ADD_CHARACTER_TO_MAP 에서 불린것임.

	// 템플릿 얻어내고..
	ApmMap::RegionTemplate*	pTemplate		= pThis->m_pcsApmMap->GetTemplate( pdCharacter->m_nBindingRegionIndex );
	ApmMap::RegionTemplate*	pTemplatePrev	= pThis->m_pcsApmMap->GetTemplate( nPrevRegionIndex );

	if( pTemplate && pTemplatePrev )
	{
		if( pTemplate->nIndex != pTemplatePrev->nIndex )
		{
			//열린 상태에서만 바꿔주자
			if( pThis->m_pUIWorldMap->m_eStatus != AGCDUI_STATUS_ADDED)
				pThis->OpenWorldMap(pTemplate->nIndex );
		}
	}
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//display function
//////////////////////////////////////////////////////////////////////////
BOOL AgcmUIWorldmap::CBDisplayWorldName( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if(  NULL == pClass )
		return FALSE;

	AgcmUIWorldmap* pThis = (AgcmUIWorldmap*)pClass;

	if( pThis->m_iCurGroup == -1 || pThis->m_iCurGroup >= (int)WORLD_MAP::vecGroupName.size() )
		return FALSE;

	ApmMap::WorldMap* pMapInfo = pThis->m_pcsApmMap->GetWorldMapInfoByGroupName(  WORLD_MAP::vecGroupName[pThis->m_iCurGroup].c_str() );
	if( NULL == pMapInfo )
		return FALSE;

	sprintf( szDisplay,"%s", pMapInfo->strComment );

	return TRUE;
}

BOOL AgcmUIWorldmap::CBDisplayRegionName( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass || NULL == pcsSourceControl )
		return FALSE;

	AgcmUIWorldmap* pThis = (AgcmUIWorldmap*)pClass;

	if( pThis->m_iCurGroup == -1 || pThis->m_iCurGroup >= (int)WORLD_MAP::vecGroupName.size() )
		return FALSE;

	ApmMap::WorldMap* pMapInfo = pThis->m_pcsApmMap->GetWorldMapInfoByGroupName( WORLD_MAP::vecGroupName[pThis->m_iCurGroup].c_str() );
	if( NULL == pMapInfo )
	{
		sprintf( szDisplay,"%s", "None Data" );
		return FALSE;
	}

	sprintf( szDisplay,"%s", pMapInfo->strComment  );

	return TRUE;
}

void AgcmUIWorldmap::OnGetControls( VOID )
{
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkHuman );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkHuman2 );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkHuman3 );

	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkOrc );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkOrc2 );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkOrc3 );

	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkMoonElf );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkMoonElf2 );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkMoonElf3 );

	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkDragonSion );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkDragonSion2 );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkDragonSion3 );

	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkArchlord );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkArchlord2 );
	m_pcsUIMgr->ThrowEvent( m_nEventGetBtnGuildMarkArchlord3 );
}

void AgcmUIWorldmap::OnUpdateCastleInfo( AuCastleOwnerType eCastleType, AuCastleInfoType eInfoType, void* pData )
{
	switch( eInfoType )
	{
	case CastleInfo_OwnerGuildName :
		{
			CHAR* pDisplay = ( CHAR* )pData;
			if( !pDisplay ) return;

			CHAR* pName = GetCastleOwnerGuildName( eCastleType );
			if( m_bViewCastleInfo )
			{
				if( pName && strlen( pName ) > 0 )
				{
					sprintf( pDisplay, "[%s]", pName );
				}
				else
				{
					CHAR* pText = ClientStr().GetStr( STI_STATICTEXT_NO_TEXT );
					sprintf( pDisplay, "[%s]", pText );
				}
			}
			else
			{
				sprintf( pDisplay, "%s", "" );
			}
		}
		break;

	case CastleInfo_GuildMasterName :
		{
			CHAR* pDisplay = ( CHAR* )pData;
			if( !pDisplay ) return;

			CHAR* pGuildMasterName = GetCastleOwnerGuildMasterName( eCastleType );
			if( m_bViewCastleInfo )
			{
				if( pGuildMasterName && strlen( pGuildMasterName ) > 0 )
				{
					sprintf( pDisplay, "%s", pGuildMasterName );
				}
				else
				{
					CHAR* pText = ClientStr().GetStr( STI_STATICTEXT_NO_TEXT );
					sprintf( pDisplay, "%s", pText );
				}
			}
			else
			{
				sprintf( pDisplay, "%s", "" );
			}
		}
		break;
	}

	OnUpdateGuildMarkTexture( eCastleType );

}

void AgcmUIWorldmap::OnUpdateGuildMarkTexture( AuCastleOwnerType eCastleType )
{
	if( !m_bViewCastleInfo ) return;

	AcUIButton* pBtn = NULL;
	RwTexture* pTexture = NULL;
	switch( eCastleType )
	{
	case CastleOwner_Human :
		{
			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkBG( eCastleType );
			RenderButton( m_pCastleGuildMarkHuman, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkPattern( eCastleType );
			RenderButton( m_pCastleGuildMarkHuman2, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkSimbol( eCastleType );
			RenderButton( m_pCastleGuildMarkHuman3, pTexture );
		}
		break;

	case CastleOwner_Orc :
		{
			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkBG( eCastleType );
			RenderButton( m_pCastleGuildMarkOrc, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkPattern( eCastleType );
			RenderButton( m_pCastleGuildMarkOrc2, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkSimbol( eCastleType );
			RenderButton( m_pCastleGuildMarkOrc3, pTexture );
		}
		break;

	case CastleOwner_MoonElf :
		{
			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkBG( eCastleType );
			RenderButton( m_pCastleGuildMarkMoonElf, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkPattern( eCastleType );
			RenderButton( m_pCastleGuildMarkMoonElf2, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkSimbol( eCastleType );
			RenderButton( m_pCastleGuildMarkMoonElf3, pTexture );
		}
		break;

	case CastleOwner_DragonScion :
		{
			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkBG( eCastleType );
			RenderButton( m_pCastleGuildMarkDragonSion, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkPattern( eCastleType );
			RenderButton( m_pCastleGuildMarkDragonSion2, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkSimbol( eCastleType );
			RenderButton( m_pCastleGuildMarkDragonSion3, pTexture );
		}
		break;

	case CastleOwner_Archlord :
		{
			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkBG( eCastleType );
			RenderButton( m_pCastleGuildMarkArchlord, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkPattern( eCastleType );
			RenderButton( m_pCastleGuildMarkArchlord2, pTexture );

			pTexture = ( RwTexture* )GetCastleOwnerGuildMarkSimbol( eCastleType );
			RenderButton( m_pCastleGuildMarkArchlord3, pTexture );
		}
		break;

	default :					return;								break;
	}
}

void AgcmUIWorldmap::RenderButton( AcUIButton* pBtn, void* pTexture )
{
	if( pBtn && pTexture )
	{
		INT32 nPosX = pBtn->m_lAbsolute_x;
		INT32 nPosY = pBtn->m_lAbsolute_y;
		INT32 nWidth = pBtn->w;
		INT32 nHeight = pBtn->h;

		g_pEngine->DrawIm2D( ( RwTexture* )pTexture, ( float )nPosX, ( float )nPosY, ( float )nWidth, ( float )nHeight );
	}
}
void* AgcmUIWorldmap::GetCastleInfomation( AuCastleOwnerType eCastleType )
{
	if( !m_pcsAgpmChar ) return NULL;

	AgpmSiegeWar* ppmSiegeWar = ( AgpmSiegeWar* )m_pcsAgpmChar->GetModule( "AgpmSiegeWar" );
	if( !ppmSiegeWar ) return NULL;

	CHAR* pCastleName = NULL;
	switch( eCastleType )
	{
	case CastleOwner_Human :		pCastleName = ClientStr().GetStr( STI_STATICTEXT_CASTLENAME_HUMAN );		break;
	case CastleOwner_Orc :			pCastleName = ClientStr().GetStr( STI_STATICTEXT_CASTLENAME_ORC );			break;
	case CastleOwner_MoonElf :		pCastleName = ClientStr().GetStr( STI_STATICTEXT_CASTLENAME_MOONELF );		break;
	case CastleOwner_DragonScion :	pCastleName = ClientStr().GetStr( STI_STATICTEXT_CASTLENAME_DRAGONSCION );	break;
	case CastleOwner_Archlord :		pCastleName = ClientStr().GetStr( STI_STATICTEXT_CASTLENAME_ARCHLORD );		break;
	default :						return NULL;																break;
	}

	return ppmSiegeWar->GetSiegeWarInfo( pCastleName );
}

void* AgcmUIWorldmap::GetCastleOwnerGuild( AuCastleOwnerType eCastleType )
{
	AgpmGuild* ppmGuild = ( AgpmGuild* )m_pcsAgpmChar->GetModule( "AgpmGuild" );
	if( !ppmGuild ) return NULL;

	CHAR* pGuildName = GetCastleOwnerGuildName( eCastleType );
	if( pGuildName && strlen( pGuildName ) > 0 )
	{
		return ppmGuild->GetGuild( pGuildName );
	}

	return NULL;
}

CHAR* AgcmUIWorldmap::GetCastleOwnerGuildName( AuCastleOwnerType eCastleType )
{
	AgpdSiegeWar* ppdSiegeWar = ( AgpdSiegeWar* )GetCastleInfomation( eCastleType );
	if( !ppdSiegeWar ) return NULL;

	return ppdSiegeWar->m_strOwnerGuildName.GetBuffer();
}

CHAR* AgcmUIWorldmap::GetCastleOwnerGuildMasterName( AuCastleOwnerType eCastleType )
{
	AgpdSiegeWar* ppdSiegeWar = ( AgpdSiegeWar* )GetCastleInfomation( eCastleType );
	if( !ppdSiegeWar ) return NULL;

	return ppdSiegeWar->m_strOwnerGuildMasterName.GetBuffer();
}

void* AgcmUIWorldmap::GetCastleOwnerGuildMarkBG( AuCastleOwnerType eCastleType )
{
	AgpmGuild* ppmGuild = ( AgpmGuild* )m_pcsAgpmChar->GetModule( "AgpmGuild" );
	if( !ppmGuild ) return NULL;

	AgpdGuild* ppdGuild = ( AgpdGuild* )GetCastleOwnerGuild( eCastleType );
	if( !ppdGuild ) return NULL;

	return ppmGuild->GetGuildMarkTexture( AGPMGUILD_ID_MARK_BOTTOM, ppdGuild->m_lGuildMarkTID, TRUE );
}

void* AgcmUIWorldmap::GetCastleOwnerGuildMarkPattern( AuCastleOwnerType eCastleType )
{
	AgpmGuild* ppmGuild = ( AgpmGuild* )m_pcsAgpmChar->GetModule( "AgpmGuild" );
	if( !ppmGuild ) return NULL;

	AgpdGuild* ppdGuild = ( AgpdGuild* )GetCastleOwnerGuild( eCastleType );
	if( !ppdGuild ) return NULL;

	return ppmGuild->GetGuildMarkTexture( AGPMGUILD_ID_MARK_PATTERN, ppdGuild->m_lGuildMarkTID, TRUE );
}

void* AgcmUIWorldmap::GetCastleOwnerGuildMarkSimbol( AuCastleOwnerType eCastleType )
{
	AgpmGuild* ppmGuild = ( AgpmGuild* )m_pcsAgpmChar->GetModule( "AgpmGuild" );
	if( !ppmGuild ) return NULL;

	AgpdGuild* ppdGuild = ( AgpdGuild* )GetCastleOwnerGuild( eCastleType );
	if( !ppdGuild ) return NULL;

	return ppmGuild->GetGuildMarkTexture( AGPMGUILD_ID_MARK_SYMBOL, ppdGuild->m_lGuildMarkTID, TRUE );
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleHuman( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkHuman = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleHuman2( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkHuman2 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleHuman3( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkHuman3 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleOrc( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkOrc = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleOrc2( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkOrc2 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleOrc3( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkOrc3 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleMoonElf( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkMoonElf = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleMoonElf2( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkMoonElf2 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleMoonElf3( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkMoonElf3 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleDragonSion( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkDragonSion = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleDragonSion2( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkDragonSion2 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleDragonSion3( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkDragonSion3 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );

	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleArchlord( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkArchlord = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleArchlord2( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkArchlord2 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_GetBtnCastleArchlord3( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pData1 ) return FALSE;

	pThis->m_pCastleGuildMarkArchlord3 = ( ( AcUIButton* )( ( AgcdUIControl* )pData1 )->m_pcsBase );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerGuildHuman( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_Human, CastleInfo_OwnerGuildName, pDisplay );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerNameHuman( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_Human, CastleInfo_GuildMasterName, pDisplay );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerGuildOrc( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_Orc, CastleInfo_OwnerGuildName, pDisplay );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerNameOrc( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_Orc, CastleInfo_GuildMasterName, pDisplay );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerGuildMoonElf( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_MoonElf, CastleInfo_OwnerGuildName, pDisplay );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerNameMoonElf( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_MoonElf, CastleInfo_GuildMasterName, pDisplay );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerGuildDragonScion( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_DragonScion, CastleInfo_OwnerGuildName, pDisplay );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerNameDragonScion( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_DragonScion, CastleInfo_GuildMasterName, pDisplay );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerGuildArchlord( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_Archlord, CastleInfo_OwnerGuildName, pDisplay );
	return TRUE;
}

BOOL AgcmUIWorldmap::CB_OnDisplayOwnerNameArchlord( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue )
{
	AgcmUIWorldmap* pThis = ( AgcmUIWorldmap* )pClass;
	if( !pThis || !pDisplay ) return FALSE;

	pThis->OnUpdateCastleInfo( CastleOwner_Archlord, CastleInfo_GuildMasterName, pDisplay );
	return TRUE;
}

