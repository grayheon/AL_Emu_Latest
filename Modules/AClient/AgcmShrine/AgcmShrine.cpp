#include "AgcmShrine.h"
#include "AgcmUIManager2.h"
#include "AgpmShrineBattle.h"
#include "AuXmlParser.h"
#include "AgcChatManager.h"
#include "AgcmEventEffect.h"
#include "AuStrTable.h"
//#include "AuKoreanTest.h"//JK_쉬라인머지

#define SHRINE_OBJECT_ENABLE					5185			//사용가능 상태
#define SHRINE_OBJECT_DISABLE_ANIM		2230			//사용불능 전 애니메이션 오브젝트
#define SHRINE_OBJECT_DISABLE				2231			//사용불능 상태

#define SHRINE_INFO_LIST_MAX					3				//쉬라인 상황 정보 페이지

#define SHRINE_OBJECT_ENABLE_JOIN			300000		//5분

//쉬라인 정보창 하단 탭 처리 하기위해 탭 베이스가 되는 컨트롤 이름
//활성된 탭은 이 컨트롤을 Show시키고 다른 탭 컨트롤은 Hide 시킨다.
//E_SHRINE_TAP_BUTTON_TYPE에도 동일하게 인덱싱처리
string g_ShrineTapButtonName[] =
{
	"C_Tap_Shrine_Info_Form",	//상황
	"",
};

enum E_SHRINE_TAP_BUTTON_TYPE
{
	E_TAP_INFO,			//상황
	E_TAP_MAX,
};

//////////////////////////////////////////////////////////////////////////
void AcUIShrineRender::OnWindowRender()
{ 
	if( m_AgcmShrine && m_AgcmShrine->m_pUI && m_AgcmShrine->m_pUI->m_eStatus != AGCDUI_STATUS_OPENING  ) 
		m_AgcmShrine->m_ShrineIcon.Render( m_AgcmShrine->m_pUIMgr->m_v2dCurMousePos, -1, TRUE ); 
};

//////////////////////////////////////////////////////////////////////////
AgcmShrine::AgcmShrine(void)
:m_pUIMgr(NULL)
,m_pAgpmShrine(NULL)
,m_pAgpmCharacter(NULL)
,m_pAgcmCharacter(NULL)
,m_pAgcmEventEffect(NULL)
,m_pUDShrineHP(NULL)
,m_pUDShrineUpdate(NULL)
,m_pUDShrineInfoList(NULL)
,m_pUDShrineListPage(NULL)
,m_pUI(NULL)
,m_iEventShrineHPShow(0)
,m_iEventShrineHPHide(0)
,m_iCurProcessShrine(0)
,m_iEventShrineUpdate(0)
,m_iEventShrineOpenSound(0)
,m_iEventShrineCloseSound(0)
,m_iImprintProcessTime(0)
,m_iImprintAccTime(0)
,m_iImprintMaxTime(0)
,m_iShrineMsgPos(150)
,m_iShrineHPPos(50)
,m_dwNormalMessageColor(0xffffff00)
,m_dwStartMessageColor(0xffff0000)
,m_dwCloseMessageColor(0xffff0000)
,m_bStartImprint(FALSE)
,m_dwRequestTime(0)
,m_iCurPage(0)
,m_iShrineEnableCount(8)
,m_pUICtrlShrineButton(NULL)
,m_bIsOpenMaximizedWindow(FALSE)
,m_bIsOpenMinimizedWindow(FALSE)
,m_iEventShrineBattleInfoOpen(0)
,m_nBattleInfoPosX(-1)
,m_nBattleInfoPosY(-1)
,m_pUDMaxTime(NULL)
,m_pUDMyWin(NULL)
,m_pUDMyLose(NULL)
,m_pUDRaceWin(NULL)
,m_pUDRaceLose(NULL)
,m_pUDMiniTime(NULL)
,m_nMyWin(0)
,m_nMyLose(0)
,m_nRaceWin(0)
,m_nRaceLose(0)
,m_dwRefreshTime(0)
,m_nShrineBattleInfoTime(60)
,m_pShrineCharacter(NULL)
,m_nOpenedShrinePointID(-1)
,m_nOpenedTime(0)
,m_dwPrintNextTimeChat(0)
,m_dwPrintNextTimeCenter(0)
,m_dwCheckFiveMin(0)
{
	SetModuleName("AgcmShrine");
	EnableIdle(TRUE);

	memset( m_pUDOccupationCount, NULL, sizeof( AgcdUIUserData* ) * AURACE_TYPE_MAX );
}

AgcmShrine::~AgcmShrine(void)
{
}

BOOL AgcmShrine::OnAddModule()
{
	m_pUIMgr = (AgcmUIManager2 *)	GetModule("AgcmUIManager2");
	m_pAgpmShrine = (AgpmShrineBattle *)	GetModule("AgpmShrineBattle");
	m_pAgpmCharacter = (AgpmCharacter*) GetModule("AgpmCharacter");
	m_pAgcmCharacter = (AgcmCharacter*) GetModule("AgcmCharacter");
	m_pAgcmEventEffect = (AgcmEventEffect*)GetModule("AgcmEventEffect");

	if( NULL == m_pUIMgr || NULL == m_pAgpmShrine || NULL == m_pAgpmCharacter || NULL == m_pAgcmCharacter || NULL == m_pAgcmEventEffect)
		return FALSE;

	if( AddFunction() == FALSE )
		return FALSE;
	if(AddDisplay() == FALSE )
		return FALSE;
	if( AddUserData() == FALSE )
		return FALSE;
	if( AddEvent() == FALSE )
		return FALSE;

	m_pAgpmShrine->SetCallBackShrinePointTimeBarUpdate( CBShrineTimeToggleShow, this );
	//m_pAgpmShrine->SetCallBackShrinePointTryImprint( CBResultImprint, this );
	m_pAgpmShrine->SetCallBackShrinePointStatusUpdate( CBShrinePointStatusUpdate, this );
	m_pAgpmShrine->SetCallBackShrinePointInformation( CBShrinePointUpdateInfo, this );
	m_pAgpmShrine->SetCallBackShrinePointInformationTotal( CBAllShrinePointUpdateInfo, this );
	m_pAgpmShrine->SetCallBackShrinePointMessageCode( CBShrineMessage, this ); 
	m_pAgpmShrine->SetCallBackShrinePointTryImprintResult( CBResultImprint, this );
	m_pAgpmShrine->SetCallBackShrinePointReadyToStart( CBReadyToStart, this );
	m_pAgpmShrine->SetCallBackShrinePointPvPRecord( CBShrinePvPRecord, this );
	m_pAgpmShrine->SetCallBackShrinePointRaceRecord( CBShrineRaceRecord, this );

	_LoadShrineMessage();

	m_ShrineRender.OnInit();
	m_ShrineRender.Set( this );

	m_pUIMgr->AddCustomControl( "ShrineRender", &m_ShrineRender );
	return TRUE;
}

BOOL AgcmShrine::OnInit()
{
	m_ShrineIcon.Init( (AgcmFont*)GetModule("AgcmFont")  );

	//전투중
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_BATTLE, "Shrine_Icon_Battle.png" );

	//닫힘
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_DISABLE, "Shrine_Icon_Disable.png" );

	//열림
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_ENABLE, "Shrine_Icon_Enable_a.png" );
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_ENABLE, "Shrine_Icon_Enable_b.png" );
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_ENABLE, "Shrine_Icon_Enable_c.png" );
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_ENABLE, "Shrine_Icon_Enable_d.png" );

	//점령 종족
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_HUMN, "Shrine_Icon_Human.png" );
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_ORC, "Shrine_Icon_Orc.png" );
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_MOONELF, "Shrine_Icon_Moon.png" );
	m_ShrineIcon.AddTexture( AGUIDrawMark::E_TYPE_SHRINE_DRAGONSION, "Shrine_Icon_Dra.png" );


	return TRUE;
}

BOOL AgcmShrine::OnDestroy()
{
	for(MAP_MSG_RACEINFO_ITR itr = m_mapMsgRaceInfo.begin() ; itr != m_mapMsgRaceInfo.end() ; ++itr)
	{
		delete(itr->second);
		itr->second = NULL;
	}

	for(MAP_MSG_MESSAGE_ITR itr = m_mapMsgMessage.begin() ; itr != m_mapMsgMessage.end() ; ++itr)
	{
		delete(itr->second);
		itr->second = NULL;
	}

	m_mapMsgMessage.clear();
	m_mapMsgRaceInfo.clear();

	m_listShrineMessageCenter.clear();
	m_listShrineMessageChat.clear();

	return TRUE;
}

//쉬라인 메세지를 읽어 들인다.
BOOL AgcmShrine::_LoadShrineMessage()
{
	AuXmlParser XMLDocument;
	if( !XMLDocument.LoadXMLFile( "INI\\shrineMessage.xml" ) )
	{
		return FALSE;
	}

	AuXmlNode*		pRootNode	=	XMLDocument.FirstChild("ShrineMessage");
	AuXmlNode* pRaceNode = pRootNode->FirstChild("RaceCode");
	if( pRaceNode )
	{
		AuXmlNode* pNode = pRaceNode->FirstChild();
		while( pNode )
		{
			S_SHRINE_MSG_RACE_INFO *pInfo = new S_SHRINE_MSG_RACE_INFO;
			const char* pChar = pNode->FirstChild("RaceTid")->ToElement()->GetText();
			pInfo->iRaceTID = atoi( pChar );

			pChar = pNode->FirstChild("RaceName")->ToElement()->GetText();
			/*memcpy( pInfo->szRaceName, pChar, strlen( pChar ) * sizeof( char ) );*/
			strcpy_s(pInfo->szRaceName, strlen( pChar ) * sizeof( char ) + sizeof( char ), pChar);

			pInfo->dwRaceColor = _GetColorFromXml( pNode->FirstChild("Color") );
			m_mapMsgRaceInfo.insert( make_pair(pInfo->iRaceTID, pInfo) );

			pNode = pNode->NextSibling();
		}

	}

	AuXmlNode* pMsgNode = pRootNode->FirstChild("MessageCode");
	if( pMsgNode )
	{
		AuXmlNode* pNode = pMsgNode->FirstChild();
		while( pNode )
		{
			S_SHRINE_MSG_MESSAGE *pMsg = new S_SHRINE_MSG_MESSAGE;

			const char* pChar = 0;

			if( pNode->FirstChild("MessageTid")->ToElement() )
			{
				pChar = pNode->FirstChild("MessageTid")->ToElement()->GetText();
				pMsg->iMsgTID = atoi( pChar );
			}

			if( pNode->FirstChild("MessageShowType") )			
			{
				pChar = pNode->FirstChild("MessageShowType")->ToElement()->GetText();
				pMsg->eShowType = (S_SHRINE_MSG_MESSAGE::E_MSG_SHOW_TYPE)atoi( pChar );
			}

			// 			pChar = pNode->FirstChild("MessageShowTarget")->ToElement()->GetText();
			// 			pMsg->eShowTarget = (S_SHRINE_MSG_MESSAGE::E_MSG_SHOW_TARGET)atoi( pChar );

			if( pNode->FirstChild("MessageShowTarget") )
			{
				pChar = pNode->FirstChild("MessageShowTarget")->ToElement()->GetText();
				INT32 nPlaceCode	= atoi( pChar );
				pChar = pNode->FirstChild("MessageShowTargetRace")->ToElement()->GetText();
				INT32 nRaceCode	= atoi( pChar );
				INT32 nTargetCode = 0;

				switch(nPlaceCode)
				{
				case 0:
					nTargetCode = (INT32)(S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_IN_SHRINE | S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_NOT_IN_SHRINE);
					break;

				case 1:
					nTargetCode = (INT32)S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_IN_SHRINE;
					break;

				case 2:
					nTargetCode = (INT32)S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_NOT_IN_SHRINE;
					break;
				}

				switch(nRaceCode)
				{
				case 0:
					nTargetCode |= (INT32)(S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_SAME_RACE | S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_OTHER_RACE);
					break;

				case 1:
					nTargetCode |= (INT32)S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_SAME_RACE;
					break;

				case 2:
					nTargetCode |= (INT32)S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_OTHER_RACE;
					break;
				}

				pMsg->eShowTarget = (S_SHRINE_MSG_MESSAGE::E_MSG_SHOW_TARGET)nTargetCode;
			}


			if( pNode->FirstChild("MessageContent") )
			{
				pChar = pNode->FirstChild("MessageContent")->ToElement()->GetText();
				strcpy_s( pMsg->szMessage, sizeof( pMsg->szMessage ), pChar );
			}

			if( pNode->FirstChild("Color") )
			{
				pMsg->dwColor = _GetColorFromXml(pNode->FirstChild("Color"));
			}

			m_mapMsgMessage.insert( make_pair( pMsg->iMsgTID, pMsg ) );

			pNode = pNode->NextSibling();
		}
	}

	AuXmlNode* pPosNode = pRootNode->FirstChild("MessagePos");
	if( pPosNode )
	{
		const char* pChar = pPosNode->FirstChild()->ToElement()->GetText();
		m_iShrineMsgPos = atoi( pChar );
	}

	AuXmlNode* pColorNode = pRootNode->FirstChild("MessageColor");
	if( pColorNode )
	{
		AuXmlNode* pNode = pColorNode->FirstChild();
		m_dwNormalMessageColor = _GetColorFromXml( pNode, m_dwNormalMessageColor );
		pNode = pColorNode->NextSibling();
		m_dwStartMessageColor = _GetColorFromXml( pNode, m_dwStartMessageColor );
		pNode = pColorNode->NextSibling();
		m_dwCloseMessageColor = _GetColorFromXml( pNode, m_dwCloseMessageColor );
	}

	pPosNode = pRootNode->FirstChild("HPUIPos");
	if( pPosNode )
	{
		const char* pChar = pPosNode->FirstChild()->ToElement()->GetText();
		m_iShrineHPPos = atoi( pChar );
	}



	return TRUE;
}

DWORD AgcmShrine::_GetColorFromXml( TiXmlNode* pNode, DWORD dwDefaultColor )
{
	if( NULL == pNode )
		return dwDefaultColor;

	const char* pColorR = pNode->ToElement()->Attribute( "r" );
	const char* pColorG = pNode->ToElement()->Attribute( "g" );
	const char* pColorB = pNode->ToElement()->Attribute( "b" );

	if( !pColorR || !pColorG || !pColorB )
		return dwDefaultColor;

	char r = atoi( pColorR );
	char g = atoi( pColorG );
	char b = atoi( pColorB );

	return 0xff000000 | (0x00ff0000 & (r<<16) | 0x0000ff00 & (g<<8) | 0x000000ff & b);
}

BOOL AgcmShrine::AddFunction()
{
	m_pUIMgr->AddFunction( this, "Func_OpenShrineInfo",	CBOpenShrineInfo, 0 );
	m_pUIMgr->AddFunction( this, "Func_ShrineIconClick",	CBShrineIconClick, 0 );

	m_pUIMgr->AddFunction( this, "Func_ShrineInfoTapStatus",	CBShrineTapStatus, 0 );
	m_pUIMgr->AddFunction( this, "Func_ShrineInfoTapScroe",	CBShrineTapScroe, 0 );

	m_pUIMgr->AddFunction( this, "Func_ShrineInfoListPrev",	CBInfoListPrev, 0 );
	m_pUIMgr->AddFunction( this, "Func_ShrineInfoListNext",	CBInfoListNext, 0 );

	m_pUIMgr->AddFunction( this, "Func_ShrineBattleInfo_Maximize", CBMaximizeShrineBattleInfo, 0);
	m_pUIMgr->AddFunction( this, "Func_ShrineBattleInfo_Minimize", CBMinimizeShrineBattleInfo, 0);

	m_pUIMgr->AddFunction( this, "Func_ShrineBattleInfo_Open", CBShrineBattleInfoOpenUI, 0);
	return TRUE;
}

BOOL AgcmShrine::AddDisplay()
{
	m_pUIMgr->AddDisplay( this, "DP_ShrineHPMax", 0,	CBShrineHPMAX,	AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "DP_ShrineHPCur", 0,	CBShrineHPCUR,	AGCDUI_USERDATA_TYPE_INT32 );

	m_pUIMgr->AddDisplay( this, "DP_ShrineHPDP", 0,	CBShrineHPDP,	AGCDUI_USERDATA_TYPE_INT32 );

	m_pUIMgr->AddDisplay( this, "DP_Shrine_Name", 0, CBShrineUpdate, AGCDUI_USERDATA_TYPE_INT32 );

	m_pUIMgr->AddDisplay( this, "DP_Shrine_Occ_H_Count", 0, CBShrineOccHCount, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "DP_Shrine_Occ_O_Count", 0, CBShrineOccOCount, AGCDUI_USERDATA_TYPE_INT32 );

	//Shrine 현황리스트
	m_pUIMgr->AddDisplay( this, "DP_Shrine_List_Status", 0, CBDPListStatus, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "DP_Shrine_List_Name", 0, CBDPListName, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "DP_Shrine_List_ShrineStatusCur", 0, CBDPListShrineStatusBarCur, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "DP_Shrine_List_ShrineStatusMax", 0, CBDPListShrineStatusBarMax, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "DP_Shrine_List_ShrineStatusText", 0, CBDPListShrineStatusText, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "DP_Shrine_List_OccupyRace", 0, CBDPListOccupyRace, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "DP_Shrine_List_OccupyPCName", 0, CBDPListOccupyPCName, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "DP_Shrine_List_RemainTiime", 0, CBDPListRemainTime, AGCDUI_USERDATA_TYPE_INT32 );

	//Shrine 현황 리스트 페이지 표시
	m_pUIMgr->AddDisplay( this, "DP_Shrine_List_Page", 0, CBDPListPage, AGCDUI_USERDATA_TYPE_INT32 );

	//Shrine Battle Info
	m_pUIMgr->AddDisplay( this, "LeftTimeMax_Time_DISP", 0, CBDPMaxLefttime, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "LeftTime_Mini_DISP", 0, CBDPMiniLefttime, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "MyWin_DISP", 0, CBDPMaxMyWin, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "MyLose_DISP", 0, CBDPMaxMyLose, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "RaceWin_DISP", 0, CBDPMaxRaceWin, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUIMgr->AddDisplay( this, "RaceLose_DISP", 0, CBDPMaxRaceLose, AGCDUI_USERDATA_TYPE_INT32 );

	return TRUE;
}

BOOL AgcmShrine::AddUserData()
{
	INT32		iDummyData = 0;
	m_pUDShrineHP = m_pUIMgr->AddUserData("UDShrineHP",  &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );

	m_pUDShrineUpdate = m_pUIMgr->AddUserData( "UD_Shrine_Update", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUDShrineListPage = m_pUIMgr->AddUserData( "UD_Shrine_List_Page", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );

	INT32		iarrInfoListIndex[SHRINE_INFO_LIST_MAX];
	for( int i=0; i<SHRINE_INFO_LIST_MAX; ++i )
	{
		iarrInfoListIndex[i] = i;
	}
	m_pUDShrineInfoList = m_pUIMgr->AddUserData( "UD_Shrine_Info_List", &iarrInfoListIndex, sizeof(INT32), SHRINE_INFO_LIST_MAX, AGCDUI_USERDATA_TYPE_INT32 );


	m_pUDOccupationCount[AURACE_TYPE_HUMAN] = m_pUIMgr->AddUserData( "UD_Occupate_H_Count", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUDOccupationCount[AURACE_TYPE_ORC] = m_pUIMgr->AddUserData( "UD_Occupate_O_Count", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUDOccupationCount[AURACE_TYPE_MOONELF] = m_pUIMgr->AddUserData( "UD_Occupate_M_Count", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUDOccupationCount[AURACE_TYPE_DRAGONSCION] = m_pUIMgr->AddUserData( "UD_Occupate_D_Count", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );

	m_pUDMaxTime = m_pUIMgr->AddUserData( "LeftTimeMax_Time_UD", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );

	m_pUDMyWin = m_pUIMgr->AddUserData( "MyWin_UD", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUDMyLose = m_pUIMgr->AddUserData( "MyLose_UD", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUDRaceWin = m_pUIMgr->AddUserData( "RaceWin_UD", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );
	m_pUDRaceLose = m_pUIMgr->AddUserData( "RaceLose_UD", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );

	m_pUDMiniTime = m_pUIMgr->AddUserData( "LeftTime_Mini_UD", &iDummyData, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32 );

	return TRUE;
}

BOOL AgcmShrine::AddEvent()
{
	m_iEventShrineHPShow	= m_pUIMgr->AddEvent( "Event_Shrine_HP_Show" );
	m_iEventShrineHPHide	= m_pUIMgr->AddEvent( "Event_Shrine_HP_Hide" );
	m_iEventShrineBattleInfoOpen	= m_pUIMgr->AddEvent( "ShrineBattleInfoOpen" );

	m_iEventShrineUpdate = m_pUIMgr->AddEvent( "Event_Shrine_Update" );

	m_iEventShrineOpenSound = m_pUIMgr->AddEvent( "ShrineOpenSound" );
	m_iEventShrineCloseSound = m_pUIMgr->AddEvent( "ShrineCloseSound" );
	return TRUE;
}

BOOL	AgcmShrine::OnIdle(UINT32 ulClockCount)
{
	static DWORD dwTime1 = 0;
	static DWORD dwTime2 = timeGetTime();

	dwTime1 = timeGetTime();
	DWORD dwDelta = dwTime1 - dwTime2;
	for( int i=0; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
	{
		//쉬라인이 점령된 이후 애니메이션 진행 시간을 체크해서
		//시간에 맞춰 SHRINE_OBJECT_DISABLE상태로 변경한다.

		AgpdCharacter*		pcsShrineObject	=	m_pAgpmCharacter->GetCharacter( m_arrShrinPointInfo[i].m_nShrineObjectCID );
		if( !pcsShrineObject )
			continue;

		if( m_arShrineObject[i].iAccTime >= 0 )
		{
			m_arShrineObject[i].iAccTime += dwDelta;

			AgcdCharacter*		pcdShrineObject	=	m_pAgcmCharacter->GetCharacterData( pcsShrineObject );
			if( !pcdShrineObject )
				continue;

			INT		nAnimTime	=	(INT)(m_pAgcmCharacter->GetCurrentAnimDuration( pcdShrineObject ) * 1000.f);

			if( m_arShrineObject[i].iAccTime > nAnimTime )
			{
				m_arShrineObject[i].iAccTime = -1;
				m_arShrineObject[i].iTID = SHRINE_OBJECT_DISABLE;

				if( _ChangeShrineObject(SHRINE_OBJECT_DISABLE, m_arrShrinPointInfo[i].m_nShrineObjectCID, m_arrShrinPointInfo[i].m_szPointName ) == FALSE )
					continue;
			}
		}
		else
		{
			//세팅된 상태와 오브젝트의 상태가 상이할 경우 오브젝트 변경
			if( pcsShrineObject->m_lTID1 != m_arShrineObject[i].iTID )
			{
				_ChangeShrineObject(m_arShrineObject[i].iTID, m_arrShrinPointInfo[i].m_nShrineObjectCID, m_arrShrinPointInfo[i].m_szPointName );
			}
		}
	}

	if(m_nOpenedTime != 0)
	{
		m_dwCheckFiveMin += dwDelta;
		if(m_dwCheckFiveMin > 5*1000*60)
		{
			m_dwCheckFiveMin = 0;
			m_nOpenedTime = 0;
		}
	}

	m_dwPrintNextTimeChat += dwDelta;
	if(m_listShrineMessageChat.size() != 0)
	{
		if(m_dwPrintNextTimeChat > 1000)
		{
			m_dwPrintNextTimeChat = 0;
			SelfPrintMessageChat(*(m_listShrineMessageChat.begin()));
			m_listShrineMessageChat.pop_front();
		}
	}

	m_dwPrintNextTimeCenter += dwDelta;
	if(m_listShrineMessageCenter.size() != 0)
	{
		if(m_dwPrintNextTimeCenter > 2000)
		{
			m_dwPrintNextTimeCenter = 0;
			SelfPrintMessageCenter(*(m_listShrineMessageCenter.begin()));
			m_listShrineMessageCenter.pop_front();
		}
	}

	BOOL bIsClosedAll = TRUE;
	//쉬라인 공방전 중간에 접속한 사람에게도 쉬라인배틀 정보창을 보여주자
	for( int i=0 ; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER ; ++i)
	{
		if( m_arrShrinPointInfo[i].m_nCurrentStatus > AGPDSHRINEBATTLE_STATUS_NORMAL && m_arrShrinPointInfo[i].m_nCurrentStatus < AGPDSHRINEBATTLE_STATUS_END )
		{
			bIsClosedAll = FALSE;
			// 쉬라인이 열려있으면 남은 시간 갱신을 해준다.
			m_nShrineBattleInfoTime = (INT32)(m_arrShrinPointInfo[i].m_nRemainTime * 0.001f / 60) + 1;
			if(m_nShrineBattleInfoTime > 60)
				m_nShrineBattleInfoTime = 60;
			if(!IsOpenedAnyBattleWindow())
			{
				//쉬라인 공방전 상태이면서 쉬라인 전투창이 열려있지 않으면
				//즉 쉬라인 시간 중간에 들어왔으면 창을 열어준다.
				m_pUIMgr->ThrowEvent(m_iEventShrineBattleInfoOpen);
				m_nOpenedTime = m_arrShrinPointInfo[i].m_nBattleOpenTime;
				CheckBattleWindow();
			}
			else
			{
				//열려있다면 3초마다 남은시간(분단위) 표시를 해준다.
				m_dwRefreshTime += dwDelta;
				if(m_dwRefreshTime > 3000)
				{
					m_dwRefreshTime = 0;
					AgpdCharacter* pSelfCharacter = m_pAgcmCharacter->GetSelfCharacter();
					if(pSelfCharacter)
					{
						PACKET_AGPP_SHRINEBATTLE_INFORMATION_REQUEST pPacket(  pSelfCharacter->GetID() );
						BOOL bResult	= g_pEngine->SendPacket( pPacket );
						m_pUIMgr->SetUserDataRefresh(m_pUDMaxTime);
						m_pUIMgr->SetUserDataRefresh(m_pUDMiniTime);
					}
				}
			}
			static INT32 RemainTimeFlag = 0;
			if(m_nShrineBattleInfoTime>=60)
				RemainTimeFlag = 0;
			else if(RemainTimeFlag == 0 && m_nShrineBattleInfoTime == 55)
			{
				RemainTimeFlag = 1;
				MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_CANNOT_USE_ICON);
			}
			else if(RemainTimeFlag == 1 && m_nShrineBattleInfoTime == 10)
			{
				RemainTimeFlag = 2;
				MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH_REMAIN_10MIN);
			}
			else if(RemainTimeFlag == 2 && m_nShrineBattleInfoTime == 5)
			{
				RemainTimeFlag = 0;
				MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH_REMAIN_5MIN);
			}
			break;
		}
	}

	if(bIsClosedAll)
		CloseAllBattleWindow();

	//쉬라인 정보창이 켜져 있을 경우 3초에 한번씩 업뎃 요청
	if( m_pUI && m_pUI->m_eStatus == AGCDUI_STATUS_OPENED )
	{
		m_dwRequestTime += dwDelta;
		if( m_dwRequestTime > 3000 )
		{
			m_dwRequestTime = 0;
			//request infomation
			AgpdCharacter* pSelfCharacter = m_pAgcmCharacter->GetSelfCharacter();
			if(pSelfCharacter)
			{
				PACKET_AGPP_SHRINEBATTLE_INFORMATION_REQUEST pPacket(  pSelfCharacter->GetID() );
				BOOL bResult	= g_pEngine->SendPacket( pPacket );
			}
		}
	}

	//미니맵 쉬라인 정보 버튼
	if( m_pUICtrlShrineButton )
	{
		m_pUICtrlShrineButton->m_pcsBase->ShowWindow( FALSE );
		for( int i=0; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
		{
			if( _IsJoinShrine( i ) == (BOOL)TRUE )
			{
				m_pUICtrlShrineButton->m_pcsBase->ShowWindow( TRUE );
				break;
			}
		}
	}


	dwTime2 = dwTime1;
	return TRUE;
}

BOOL AgcmShrine::ClickShrine( AgpdCharacter *pcsSelfChar, AgpdCharacter* pObject )
{
	if (!pObject)
		return FALSE;

	//쉬라인 각인체인지 체크 한다.
	if( m_pAgpmCharacter->IsShrineTypifier( pObject ) == FALSE )
	{
		return FALSE;
	}

	//쉬라인 각인체 클릭시 상태에 따른 메세지 출력
	AgpdShrinePoint* pShrineInfo = _GetShrineInfo( m_iCurProcessShrine );
	if(  pShrineInfo )
	{
		switch( pShrineInfo->m_nCurrentStatus )
		{
		case AGPDSHRINEBATTLE_STATUS_OPEN:
			if( m_pAgpmShrine->IsInTryImprintRange( pShrineInfo, pcsSelfChar ) == FALSE)
			{
				char* pszMessage = m_pUIMgr->GetUIMessage( "Shrine_Imprint_Range" );
				AgcChatManager::OnAddShrineMessage( pszMessage );
				return FALSE;
			}
			break;
		case AGPDSHRINEBATTLE_STATUS_TRYIMPRINT:
		case AGPDSHRINEBATTLE_STATUS_IMPRINT:
			{
				if( 0 == strcmp(pcsSelfChar->m_szID, pShrineInfo->m_szOccupyCharacterName) )
				{
					char* pszMessage = m_pUIMgr->GetUIMessage( "Shrine_Imprint_Myself" );
					AgcChatManager::OnAddShrineMessage( pszMessage );
				}
				else
				{
					char* pszMessage = m_pUIMgr->GetUIMessage( "Shrine_OtherPlayer_Imprint" );
					AgcChatManager::OnAddShrineMessage( pszMessage );
				}
			}
			return FALSE;
		case AGPDSHRINEBATTLE_STATUS_TRYOCCUPY:
		case AGPDSHRINEBATTLE_STATUS_OCCUPY:
			{
				if( 0 == strcmp(pcsSelfChar->m_szID, pShrineInfo->m_szOccupyCharacterName) )
				{
					char* pszMessage = m_pUIMgr->GetUIMessage( "Shrine_Occupy_Myself" );
					AgcChatManager::OnAddShrineMessage( pszMessage );
				}
				else
				{
					char* pszMessage = m_pUIMgr->GetUIMessage( "Shrine_OtherPlayer_Occupy" );
					AgcChatManager::OnAddShrineMessage( pszMessage );
				}
			}
			return FALSE;
		case AGPDSHRINEBATTLE_STATUS_END:
		case AGPDSHRINEBATTLE_STATUS_NORMAL:
			{
				char* pszMessage = m_pUIMgr->GetUIMessage( "Shrine_Close" );
				AgcChatManager::OnAddShrineMessage( pszMessage );
			}
			return FALSE;
		}
	}

	//각인이 가능한 상태면 내게 각인 해라!!
	PACKET_AGPP_SHRINEBATTLE_TRYIMPRINT Packet( pcsSelfChar->GetID(), 0 );
	g_pEngine->SendPacket( Packet );

	return TRUE;
}

//UI.INI파일을 로딩 직후 컨트롤 탭
BOOL AgcmShrine::SetTapControl()
{
	AgcdUI* pUIShrine = m_pUIMgr->GetUI("UI_Shrine_info");
	if( NULL == pUIShrine )
		return FALSE;

	m_vecTapbutton.reserve( E_TAP_MAX );
	m_vecTapbutton.resize( E_TAP_MAX );

	AgcdUIControl *pControl = NULL;
	INT32		i=0;
	for ( pControl = m_pUIMgr->GetSequenceControl(pUIShrine, &i); pControl; pControl = m_pUIMgr->GetSequenceControl(pUIShrine, &i) )
	{
		if( pControl )
		{
			for(INT32 cnt = 0; cnt<E_TAP_MAX; ++cnt )
			{
				if( strcmp( pControl->m_szName, g_ShrineTapButtonName[cnt].c_str() ) == 0 )
				{
					m_vecTapbutton[cnt] = pControl;
					break;
				}
			}
		}

		//쉬라인 아이콘 오프셋 세팅
		if( strcmp( pControl->m_szName, "C_Event_Panel") == 0 )
		{
			m_vIconOffset.x = pControl->m_pcsBase->x;
			m_vIconOffset.y = pControl->m_pcsBase->y;
		}

	}

	//미니맵 컨트롤의 쉬라인 컨트롤 포인트 세팅
	AgcdUI* pUIMiniMap = m_pUIMgr->GetUI("MiniMap");
	if( pUIMiniMap )
	{
		AgcdUIControl *pControl = NULL;
		INT32		i=0;
		for ( pControl = m_pUIMgr->GetSequenceControl(pUIMiniMap, &i); pControl; pControl = m_pUIMgr->GetSequenceControl(pUIMiniMap, &i) )
		{
			if( strcmp( pControl->m_szName, "CONTROL_MINIMAP_SHRINE") == 0 )
			{
				m_pUICtrlShrineButton =  pControl;
				break;
			}
		}
	}

	return TRUE;
}

void AgcmShrine::EnterGameMode()
{
	for( int i=0; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
	{
		m_arShrineObject[i].reset();
	}
}

AgcmShrine::S_SHRINE_MSG_RACE_INFO* AgcmShrine::_GetMsgRaceInfo( INT32 iTID )
{
	MAP_MSG_RACEINFO_ITR it = m_mapMsgRaceInfo.find(iTID);
	if(it != m_mapMsgRaceInfo.end())
		return it->second;
	// 	for( LIST_MSG_RACEINFO_ITR it = m_listMsgRaceInfo.begin(); it != m_listMsgRaceInfo.end(); ++it )
	// 	{
	// 		if( iTID == (*it)->iRaceTID )
	// 			return (*it);
	// 	}

	return NULL;
}

AgcmShrine::S_SHRINE_MSG_MESSAGE* AgcmShrine::_GetMsgMessge( INT32 iTID )
{
	MAP_MSG_MESSAGE_ITR it = m_mapMsgMessage.find(iTID);
	if(it != m_mapMsgMessage.end())
		return it->second;
	// 	for( LIST_MSG_MESSAGE_ITR it = m_listMsgMessage.begin(); it != m_listMsgMessage.end(); ++it )
	// 	{
	// 		if( iTID == (*it)->iMsgTID )
	// 			return (*it);
	// 	}

	return NULL;
}

BOOL AgcmShrine::_CopyShrinInfo( AgpdShrinePoint* pDestShrineInfo, AgpdShrinePoint* pSrcShrineInfo )
{
	//default copy
	*pDestShrineInfo = *pSrcShrineInfo;

	//예외처리
	if(pDestShrineInfo->m_nShrinePointID < 0 && pDestShrineInfo->m_nShrinePointID > AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER)
		pDestShrineInfo->m_nShrinePointID = 0;
	if(pDestShrineInfo->m_nOccupationRace < AURACE_TYPE_NONE && pDestShrineInfo->m_nOccupationRace > AURACE_TYPE_MAX)
		pDestShrineInfo->m_nOccupationRace = AURACE_TYPE_NONE;
	if(pDestShrineInfo->m_nTryOccupyRace < AURACE_TYPE_NONE && pDestShrineInfo->m_nTryOccupyRace > AURACE_TYPE_MAX)
		pDestShrineInfo->m_nTryOccupyRace = AURACE_TYPE_NONE;


	//memory copy
	memcpy( pDestShrineInfo->m_szPointName, pSrcShrineInfo->m_szPointName, strlen( pSrcShrineInfo->m_szPointName) * sizeof(char) );
	memcpy( pDestShrineInfo->m_szOccupyCharacterName, pSrcShrineInfo->m_szOccupyCharacterName, strlen( pSrcShrineInfo->m_szOccupyCharacterName) * sizeof(char));

	return TRUE;
}

//쉬라인 정렬
void AgcmShrine::_SortShrineInfo()
{
	std::map< INT32,S_SHRINE_SORT_INFO > mapOpen, mapClose;
	S_SHRINE_SORT_INFO tmp;
	m_iShrineEnableCount = 0;
	for( int i=0; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
	{
		if( m_arrShrinPointInfo[i].m_nShrinePointID > 0 )
		{
			tmp.iShrinePointID = m_arrShrinPointInfo[i].m_nShrinePointID;

			//닫힌것
			if( m_arrShrinPointInfo[i].m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_NORMAL 
				|| m_arrShrinPointInfo[i].m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_END )
			{
				mapClose[m_arrShrinPointInfo[i].m_nBattleOpenTime+i] = tmp;
			}
			//열린것
			else
			{
				mapOpen[ m_arrShrinPointInfo[i].m_nBattleOpenTime+i] = tmp;
			}

			m_iShrineEnableCount++;
		}
	}

	//쉬라인 열린것 부터 차례대로 넣는다.
	int i = 0;
	for( std::map< INT32,S_SHRINE_SORT_INFO >::reverse_iterator it = mapOpen.rbegin(); it != mapOpen.rend(); ++it , ++i )
	{
		m_arrShrineSortInfo[i] = it->second;
	}

	//닫힌것 넣기
	for( std::map< INT32,S_SHRINE_SORT_INFO >::reverse_iterator it = mapClose.rbegin(); it != mapClose.rend(); ++it , ++i )
	{
		m_arrShrineSortInfo[i] = it->second;
	}

}

//쉬라인 조인 가능한지 체크
BOOL AgcmShrine::_IsJoinShrine( INT32 iIndex )
{
	if( iIndex >= m_iShrineEnableCount )
		return FALSE;

	//시작 5분전 알림
	if( m_arShrineObject[iIndex].bReadyToStart )
		return TRUE;

	UINT32 iElapsedTime = m_arrShrinPointInfo[iIndex].m_nTimeLimitTime - m_arrShrinPointInfo[iIndex].m_nRemainTime;
	if( (iElapsedTime < 0 || iElapsedTime >=  SHRINE_OBJECT_ENABLE_JOIN)
		||( m_arrShrinPointInfo[iIndex].m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_END ||
		m_arrShrinPointInfo[iIndex].m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_NORMAL ))
		return FALSE;

	return TRUE;
}

AgpdShrinePoint* AgcmShrine::_GetShrineInfo( INT32 iShrineID )
{
	for( int i=0; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
	{
		if( m_arrShrinPointInfo[i].m_nShrinePointID == iShrineID )
			return &m_arrShrinPointInfo[i];
	}

	return NULL;
}

INT32 AgcmShrine::_GetShrineInfoIndex( INT32 iShrineID )
{
	for( int i=0; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
	{
		if( m_arrShrinPointInfo[i].m_nShrinePointID == iShrineID )
			return i;
	}

	return -1;
}

INT32 AgcmShrine::_GetShrineInfoIndex( char* pszTmp, char* pszIconName )
{
	char szName[512];
	INT32 i = 0;
	for( ; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
	{
		sprintf( szName, pszTmp, i+1 );
		if( strcmp( szName, pszIconName) == 0 )
			return i;
	}

	return -1;
}

INT32 AgcmShrine::_GetShrineOccupateCount( INT32 iRace )
{
	int iCount = 0;
	for( int i=0; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
	{
		if( m_arrShrinPointInfo[i].m_nOccupationRace == iRace )
			iCount++;
	}

	return iCount;
}

//상황판 쉬라인 아이콘 변경
BOOL AgcmShrine::_ChangeShrineIcon(  INT32 iIndex )
{
	ASSERT( iIndex > -1 && iIndex < (INT32)m_vecShrineIconUI.size() && iIndex < AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER );

	if( iIndex < 0 )
		return FALSE;

	if( iIndex >= (INT32)m_vecShrineIconUI.size() )
		return FALSE;

	if( iIndex >= AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER )
		return FALSE;

	RwV2d vPos;
	vPos.x = (RwReal)( m_vecShrineIconUI[iIndex]->m_pcsBase->x + m_vecShrineIconUI[iIndex]->m_pcsBase->w/2) + m_vIconOffset.x;
	vPos.y = (RwReal)( m_vecShrineIconUI[iIndex]->m_pcsBase->y+ m_vecShrineIconUI[iIndex]->m_pcsBase->h/2)  + m_vIconOffset.y;

	char szComment[128] = {0, };
	/*if( (m_arrShrinPointInfo[iIndex].m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_END || m_arrShrinPointInfo[iIndex].m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_NORMAL) 
	&&  m_arrShrinPointInfo[iIndex].m_nOccupationRace != AURACE_TYPE_NONE )
	{
	sprintf( szComment, "%s 점령", _GetMsgRaceInfo(m_arrShrinPointInfo[iIndex].m_nOccupationRace)->szRaceName );
	}*/

	if( (m_arrShrinPointInfo[iIndex].m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_OPEN 
		|| m_arrShrinPointInfo[iIndex].m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_TRYIMPRINT 
		|| m_arrShrinPointInfo[iIndex].m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_IMPRINT
		|| m_arrShrinPointInfo[iIndex].m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_TRYOCCUPY
		|| m_arShrineObject[iIndex].bReadyToStart == (BOOL)TRUE )
		&& _IsJoinShrine( iIndex ) == (BOOL)TRUE )
	{
		sprintf( szComment, m_pUIMgr->GetUIMessage("Shrine_Move_Button_Click") );
	}

	//점령 포인트
	if( m_pUDOccupationCount[m_arrShrinPointInfo[iIndex].m_nOccupationRace] 
	||  (m_pUDOccupationCount[m_arrShrinPointInfo[iIndex].m_nOccupationRace]  && m_arrShrinPointInfo[iIndex].m_nOccupationRace == 0) )
		m_pUIMgr->SetUserDataRefresh( m_pUDOccupationCount[m_arrShrinPointInfo[iIndex].m_nOccupationRace] );

	m_ShrineIcon.Update(iIndex, vPos, szComment, -1,  _GetShrineStatus(&m_arrShrinPointInfo[iIndex]) );

	return TRUE;
}

//쉬라인 상태에 따라 상태 아이콘에 맞는 인덱스 컨버팅
AGUIDrawMark::E_MARKINFO_TYPE AgcmShrine::_GetShrineStatus( AgpdShrinePoint* pShrineInfo )
{
	if( pShrineInfo )
	{
		INT32 iIndex = _GetShrineInfoIndex( pShrineInfo->m_nShrinePointID);
		if( m_arShrineObject[iIndex].bReadyToStart )
			return AGUIDrawMark::E_TYPE_SHRINE_ENABLE;

		switch( pShrineInfo->m_nCurrentStatus )
		{
		case AGPDSHRINEBATTLE_STATUS_OPEN:
			return AGUIDrawMark::E_TYPE_SHRINE_ENABLE;

		case AGPDSHRINEBATTLE_STATUS_TRYIMPRINT:
		case AGPDSHRINEBATTLE_STATUS_IMPRINT:
		case AGPDSHRINEBATTLE_STATUS_TRYOCCUPY:
			return AGUIDrawMark::E_TYPE_SHRINE_BATTLE;

		case AGPDSHRINEBATTLE_STATUS_END:
		case AGPDSHRINEBATTLE_STATUS_NORMAL:
			if( pShrineInfo->m_nOccupationRace == AURACE_TYPE_NONE )
				return AGUIDrawMark::E_TYPE_SHRINE_DISABLE;

		case AGPDSHRINEBATTLE_STATUS_OCCUPY:
			{
				switch( pShrineInfo->m_nOccupationRace )
				{
				case AURACE_TYPE_HUMAN:
					return AGUIDrawMark::E_TYPE_SHRINE_HUMN;
				case AURACE_TYPE_ORC:
					return AGUIDrawMark::E_TYPE_SHRINE_ORC;
				case AURACE_TYPE_MOONELF:
					return AGUIDrawMark::E_TYPE_SHRINE_MOONELF;
				case AURACE_TYPE_DRAGONSCION:
					return AGUIDrawMark::E_TYPE_SHRINE_DRAGONSION;
				}
			}

		}
	}
	return AGUIDrawMark::E_TYPE_SHRINE_DISABLE;
}

//쉬라인 타임바를 보여라
void AgcmShrine::_ShowTimeBar( INT32 iShrineID, BOOL bShow, BOOL bImprint )
{
	if( bImprint )
	{
		m_bStartImprint = bShow;
		m_iImprintProcessTime = 0;
		m_iImprintAccTime = 0;
	}
	else
		m_bStartImprint = FALSE;

	m_pUIMgr->ThrowEvent( (bShow)?m_iEventShrineHPShow : m_iEventShrineHPHide );

	m_pUIMgr->SetUserDataRefresh( m_pUDShrineHP );


	AgcdUI* pUI = m_pUIMgr->GetUI( "UI_SHRINE_HP" );
	if( pUI )
	{
		pUI->m_pcsUIWindow->m_bSmoothOpening = FALSE;
		pUI->m_pcsUIWindow->ShowWindow( bShow );
		if( bShow )
		{
			LPDIRECT3DDEVICE9 pDevice = (LPDIRECT3DDEVICE9)RwD3D9GetCurrentD3DDevice();
			LPDIRECT3DSURFACE9		pBackBuffer = NULL;
			D3DSURFACE_DESC	surf_desc;

			pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
			pBackBuffer->GetDesc( &surf_desc );
			pBackBuffer->Release();

			int x = surf_desc.Width / 2 - pUI->m_pcsUIWindow->w / 2 ;

			pUI->m_pcsUIWindow->MoveWindow( x, m_iShrineHPPos );
		}
	}

}


BOOL AgcmShrine::_ChangeShrineObject( INT32 iShrineObjectTID , INT32 iCID, char* szObjectName  )
{
	AgpdCharacter* pShrine = m_pAgpmCharacter->GetCharacter(iCID);
	if( !pShrine )
		return FALSE;

	AgpmFactors* pAgpmFactor = (AgpmFactors*)GetModule("AgpmFactors");
	INT32		ulCharType	=	AGPMCHAR_TYPE_NONE;
	if( pAgpmFactor )
		pAgpmFactor->GetValue( &pShrine->m_csFactor , (INT32*)&ulCharType , AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_TYPE , AGPD_FACTORS_CHARTYPE_TYPE_TYPE );

	//위치 가져오고
	RwV3d vPos;
	vPos.x = pShrine->m_stPos.x;
	vPos.y = pShrine->m_stPos.y;
	vPos.z = pShrine->m_stPos.z;

	//기존 오브젝트 삭제
	m_pAgpmCharacter->RemoveCharacter( iCID );

	//새로운 오브젝트 추가
	pShrine = m_pAgpmCharacter->AddCharacter( iCID, iShrineObjectTID, szObjectName );
	if( !pShrine )
		return FALSE;
	if( pAgpmFactor )
		pAgpmFactor->SetValue( &pShrine->m_csFactor , ulCharType , AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_TYPE , AGPD_FACTORS_CHARTYPE_TYPE_TYPE );


	if( !m_pAgcmCharacter->LoadCharacterData( pShrine, FALSE ) ) 
		return FALSE;

	//기존 오브젝트 위치로 이동
	AgcdCharacter *pAgcdShrine = m_pAgcmCharacter->GetCharacterData( pShrine );
	if( !pAgcdShrine )
		return FALSE;

	RwFrame*	pClumpFrame	=	RpClumpGetFrame( pAgcdShrine->m_pClump );
	if( !pClumpFrame )
		return FALSE;

	RwFrameTranslate( pClumpFrame , &vPos, rwCOMBINEPOSTCONCAT);
	pShrine->m_stPos.x = vPos.x;
	pShrine->m_stPos.y = vPos.y;
	pShrine->m_stPos.z = vPos.z;

	return TRUE;
}

//쉬라인 상황판 상황 정보 탭처리
BOOL AgcmShrine::_ChangeTapUI( INT32 iTapIndex )
{
	if(  iTapIndex >= E_TAP_MAX )
		return FALSE;

	for( int i=0; i<E_TAP_MAX; ++i )
	{
		if( m_vecTapbutton[i] )
			m_vecTapbutton[i]->m_pcsBase->ShowWindow( FALSE );
	}

	if( m_vecTapbutton[iTapIndex] )
		m_vecTapbutton[iTapIndex]->m_pcsBase->ShowWindow( TRUE );

	return TRUE;
}

BOOL AgcmShrine::_MessagePrintCheckerType( S_SHRINE_MSG_MESSAGE* pMessage, INT32 nTarget )
{
	if( pMessage->eShowType == S_SHRINE_MSG_MESSAGE::E_MSG_SHOW_ALL || pMessage->eShowType == nTarget )
		return TRUE;

	return FALSE;
}

BOOL AgcmShrine::_MessagePrintCheckerTargetPlace( S_SHRINE_MSG_MESSAGE* _pMessage, INT32 _nTarget, BOOL _IsInShrine )
{
	if((_pMessage->eShowTarget & _nTarget) && _IsInShrine)
		return TRUE;

	return FALSE;
}

BOOL AgcmShrine::_MessagePrintCheckerTargetRace( S_SHRINE_MSG_MESSAGE* _pMessage, INT32 _nTarget, INT32 _IsSameRace )
{
	if((_pMessage->eShowTarget & _nTarget) && _IsSameRace)
		return TRUE;

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//time bar ON/OFF
BOOL AgcmShrine::CBShrineTimeToggleShow(PVOID pData, PVOID pClass, PVOID pCustData) 
{
	if( NULL == pClass  ||  NULL == pData )
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	PACKET_AGPP_SHRINEBATTLE_TIMEBAR_STATE* pInfo = (PACKET_AGPP_SHRINEBATTLE_TIMEBAR_STATE*)pData;


	PVOID* pBuf = (PVOID*)pData;

	INT32 iShrineID = pInfo->nShrinePointID;
	BOOL bShowHP = pInfo->bAppear;

	pThis->_ShowTimeBar( iShrineID, bShowHP, FALSE );
	return TRUE;
}

//update shrine status 
BOOL AgcmShrine::CBShrinePointStatusUpdate(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if( NULL == pClass  ||  NULL == pData )
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	PACKET_AGPP_SHRINEBATTLE_STATUS_UPDATE* pStatus = (PACKET_AGPP_SHRINEBATTLE_STATUS_UPDATE*)pData;

	AgpdShrinePoint* pShrineInfo = pThis->_GetShrineInfo( pStatus->stShrinePoint.m_nShrinePointID );
	if( NULL == pShrineInfo )
		return FALSE;

	pThis->_CopyShrinInfo( pShrineInfo, &pStatus->stShrinePoint );

	//쉬라인 정보 정렬
	pThis->_SortShrineInfo();
	pThis->m_pUIMgr->SetUserDataRefresh( pThis->m_pUDShrineInfoList );

	//업뎃
	pThis->_ChangeShrineIcon( pThis->_GetShrineInfoIndex( pShrineInfo->m_nShrinePointID ) );

	return TRUE;
}

//update shrine info
BOOL AgcmShrine::CBShrinePointUpdateInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if( NULL == pClass  ||  NULL == pData )
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	PACKET_AGPP_SHRINEBATTLE_INFORMATION* pInfo = (PACKET_AGPP_SHRINEBATTLE_INFORMATION*)pData;

	AgpdShrinePoint* pShrineInfo = pThis->_GetShrineInfo( pInfo->stShrinePoint.m_nShrinePointID );
	if( NULL == pShrineInfo )
		return FALSE;


	pThis->m_iCurProcessShrine = pInfo->stShrinePoint.m_nShrinePointID;

	pThis->_CopyShrinInfo( pShrineInfo, &pInfo->stShrinePoint );

	//업뎃
	INT32 iShrineIndex = pThis->_GetShrineInfoIndex( pShrineInfo->m_nShrinePointID );
	pThis->_ChangeShrineIcon( iShrineIndex );

	for( int i=0; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
	{
		if( i != iShrineIndex )
			pThis->m_arShrineObject[i].iPrevStatus = -1;
	}

	if( pThis->m_arShrineObject[iShrineIndex].iAccTime == -1 
		&& pThis->m_arShrineObject[iShrineIndex].iPrevStatus != pThis->m_arrShrinPointInfo[iShrineIndex]. m_nCurrentStatus )
	{
		//Shrine Object Enable
		//쉬라인 오브젝트 사용 가능 상태로
		INT32 iCID = pThis->m_arrShrinPointInfo[iShrineIndex].m_nShrineObjectCID;

		INT32	 iTID = SHRINE_OBJECT_ENABLE;
		if( pThis->m_arrShrinPointInfo[iShrineIndex]. m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_END 
			|| pThis->m_arrShrinPointInfo[iShrineIndex]. m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_NORMAL)
			iTID = SHRINE_OBJECT_DISABLE;

		if( iTID != pThis->m_arShrineObject[iShrineIndex].iTID )
		{
			pThis->_ChangeShrineObject(iTID, iCID,  pThis->m_arrShrinPointInfo[iShrineIndex].m_szPointName );
			pThis->m_arShrineObject[iShrineIndex].iTID = iTID;
		}
	}

	if(  pThis->m_arrShrinPointInfo[iShrineIndex]. m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_OPEN 
		|| pThis->m_arrShrinPointInfo[iShrineIndex]. m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_END 
		|| pThis->m_arrShrinPointInfo[iShrineIndex]. m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_NORMAL )
	{
		pThis->m_arShrineObject[iShrineIndex].iPrevStatus = pThis->m_arrShrinPointInfo[iShrineIndex]. m_nCurrentStatus;
		pThis->_ShowTimeBar( pInfo->nShrinePointID, FALSE, TRUE );
		if(pThis->m_pShrineCharacter)
		{
			pThis->m_pAgcmEventEffect->RemoveCommonCharacterEffect(pThis->m_pShrineCharacter, E_COMMON_CHAR_TYPE_SHRINE_IMPRINT);
			pThis->m_pAgcmEventEffect->RemoveCommonCharacterEffect(pThis->m_pShrineCharacter, E_COMMON_CHAR_TYPE_SHRINE_OCCUPY);
			pThis->m_pShrineCharacter->m_bShrineEffect = FALSE;
		}
	}

	//타임바에 보여질 스트링 세팅
	S_SHRINE_MSG_RACE_INFO* pRaceInfo = pThis->_GetMsgRaceInfo( pShrineInfo->m_nTryOccupyRace );
	if(pRaceInfo )
		pThis->m_strCurRace = pRaceInfo->szRaceName;

	AgpdCharacter* pCharInfo = pThis->m_pAgpmCharacter->GetCharacter( pShrineInfo->m_nOccupyCharacterCid );
	if( pCharInfo )
	{
		pThis->m_strCurName = pCharInfo->m_szID;
		pThis->m_pShrineCharacter = pCharInfo;
	}

	if( pShrineInfo->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT ) //각인 중
	{
		char* pszTemp = pThis->m_pUIMgr->GetUIMessage("Shrine_Status_Imprint") ;
		pThis->m_strCurAction = pszTemp? pszTemp : "";

		//타임바 보여주고 업뎃 시작하자
		pThis->_ShowTimeBar( pInfo->nShrinePointID, TRUE, TRUE );
		pThis->m_pUIMgr->SetUserDataRefresh( pThis->m_pUDShrineHP );

		//각인 이펙트
		if( pCharInfo && pCharInfo->m_bShrineEffect == FALSE )
		{
			pThis->m_pAgcmEventEffect->SetCommonCharacterEffect( (ApBase*)pCharInfo, E_COMMON_CHAR_TYPE_SHRINE_IMPRINT );
			pCharInfo->m_bShrineEffect = TRUE;
		}
	}
	else if( pShrineInfo->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_IMPRINT ) //각인 완료
	{
		pThis->m_pUIMgr->ThrowEvent( pThis->m_iEventShrineHPHide );
	}
	else if(pShrineInfo->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYOCCUPY)//점령 중
	{
		pThis->m_pUIMgr->SetUserDataRefresh( pThis->m_pUDShrineHP );

		char* pszTemp = pThis->m_pUIMgr->GetUIMessage("Shrine_Status_Occupy") ;
		pThis->m_strCurAction = pszTemp? pszTemp : "";

		//점령 이펙트
		if( pCharInfo && pCharInfo->m_bShrineEffect == FALSE )
		{
			pThis->m_pAgcmEventEffect->SetCommonCharacterEffect( (ApBase*)pCharInfo, E_COMMON_CHAR_TYPE_SHRINE_OCCUPY );
			pCharInfo->m_bShrineEffect = TRUE;
		}

	}

	pThis->m_pUIMgr->SetUserDataRefresh( pThis->m_pUDShrineInfoList );

	return TRUE;
}

//all update shrine info
BOOL AgcmShrine::CBAllShrinePointUpdateInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if( NULL == pClass  ||  NULL == pData )
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	PACKET_AGPP_SHRINEBATTLE_INFORMATION_TOTAL* pAllInfo = (PACKET_AGPP_SHRINEBATTLE_INFORMATION_TOTAL*)pData;


	for( int i=0; i<AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; ++i )
	{
		//shrine data copy
		pThis->_CopyShrinInfo( &pThis->m_arrShrinPointInfo[i], &pAllInfo->m_stPointInformation[i].stShrinePoint );

		//쉬라인 오브젝트 세팅
		INT32 iCID = pThis->m_arrShrinPointInfo[i].m_nShrineObjectCID;

		INT32	 iTID = SHRINE_OBJECT_ENABLE;
		if( pThis->m_arrShrinPointInfo[i]. m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_END 
			|| pThis->m_arrShrinPointInfo[i]. m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_NORMAL)
			iTID = SHRINE_OBJECT_DISABLE;

		if( iTID != pThis->m_arShrineObject[i].iTID )
		{
			pThis->_ChangeShrineObject(iTID, iCID,  pThis->m_arrShrinPointInfo[i].m_szPointName );
			pThis->m_arShrineObject[i].iTID = iTID;
		}

	}

	//쉬라인 정보 정렬
	pThis->_SortShrineInfo();

	//update shrine name
	pThis->m_pUIMgr->RefreshUserData( pThis->m_pUDShrineUpdate, TRUE );

	//update Shrine icon
	//find icon control
	if( pThis->m_vecShrineIconUI.size() == 0 )
	{
		INT32 i = 0;
		AgcdUI* pUI = pThis->m_pUIMgr->GetUI( "UI_Shrine_info" );
		pThis->m_pUI = pUI;
		if( pUI )
		{
			INT32 iIndex = 1;
			for ( AgcdUIControl* pControl = pThis->m_pUIMgr->GetSequenceControl(pUI, &i); pControl; pControl = pThis->m_pUIMgr->GetSequenceControl(pUI, &i) )
			{
				char szName[512];
				sprintf( szName, "C_Shrine_icon_%d", iIndex );

				if( pControl && strcmp( szName, pControl->m_szName ) == 0 )
				{
					pThis->m_vecShrineIconUI.push_back( pControl );
					iIndex++;

				}
			}
		}
	}

	//all delete
	pThis->m_ShrineIcon.DeleteAll();

	for( int i=0; i<(int)pThis->m_vecShrineIconUI.size(); ++i )
	{
		pThis->_ChangeShrineIcon(i);
	}

	//상황 정보 업뎃
	pThis->m_pUDShrineInfoList->m_stUserData.m_lCount = 3;
	pThis->m_pUDShrineInfoList->m_bUpdateList = TRUE;
	pThis->m_pUDShrineInfoList->m_bNeedRefresh = TRUE;
	pThis->m_pUIMgr->SetUserDataRefresh( pThis->m_pUDShrineInfoList );

	return TRUE;
}

//imprint result
BOOL AgcmShrine::CBResultImprint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if( NULL == pClass  ||  NULL == pData )
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	PACKET_AGPP_SHRINEBATTLE_IMPRINT_RESULT* pInfo = (PACKET_AGPP_SHRINEBATTLE_IMPRINT_RESULT*)pData;


	AgpdCharacter* pCharData = pThis->m_pAgpmCharacter->GetCharacter( pInfo->nCid );
	if( pCharData )
	{
		if( pInfo->bResult)
		{
			//각인 시도
			if( pInfo->nPrevStatus == AGPDSHRINEBATTLE_STATUS_OPEN )
			{

			}
			//시도중 이펙트 표시
			else if( pInfo->nPrevStatus == AGPDSHRINEBATTLE_STATUS_IMPRINT || pInfo->nPrevStatus == AGPDSHRINEBATTLE_STATUS_TRYOCCUPY  )
			{
				pThis->m_pAgcmEventEffect->RemoveCommonCharacterEffect( (ApBase*)pCharData, E_COMMON_CHAR_TYPE_SHRINE_IMPRINT);	
				pCharData->m_bShrineEffect = FALSE;
			}
			//점령 완료시 이펙트 제거
			else if( pInfo->nPrevStatus == AGPDSHRINEBATTLE_STATUS_OCCUPY )
			{
				pThis->m_pAgcmEventEffect->RemoveCommonCharacterEffect( (ApBase*)pCharData, E_COMMON_CHAR_TYPE_SHRINE_OCCUPY );	
				pCharData->m_bShrineEffect = FALSE;
				//애니
				//pThis->m_pAgcmCharacter->StartAnimation(pCharData, AGCMCHAR_ANIM_TYPE_WAIT );

				//쉬라인 오브젝트 사용 불능 애니메이션 오브젝트로
				INT32 iShrineIndex = pThis->_GetShrineInfoIndex( pInfo->nShrinePointID );
				if(  iShrineIndex > -1 )
				{
					INT32 iCID = pInfo->nShrineObjectCID;
					pThis->_ChangeShrineObject(SHRINE_OBJECT_DISABLE_ANIM, iCID,  pThis->m_arrShrinPointInfo[iShrineIndex].m_szPointName );

					pThis->m_arShrineObject[iShrineIndex].iAccTime = 0;
				}

			}
		}
		//실패시 이펙트 제거
		else 
		{
			//각인중 취소는 없애주자
			if( pInfo->nPrevStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT )
			{
				pThis->_ShowTimeBar( pInfo->nShrinePointID, FALSE, TRUE );
				pThis->MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_COULD_BE_CANCEL);
				//애니
				//pAgcmCharacter->StartAnimation(pCharData, AGCMCHAR_ANIM_TYPE_WAIT );
			}

			pThis->m_pAgcmEventEffect->RemoveCommonCharacterEffect( (ApBase*)pCharData, E_COMMON_CHAR_TYPE_SHRINE_IMPRINT);
			pThis->m_pAgcmEventEffect->RemoveCommonCharacterEffect( (ApBase*)pCharData, E_COMMON_CHAR_TYPE_SHRINE_OCCUPY );
			pCharData->m_bShrineEffect = FALSE;
		}

	}


	return TRUE;
}

//notify Shrine Open
BOOL AgcmShrine::CBReadyToStart(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if( NULL == pClass  ||  NULL == pData )
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	PACKET_AGPP_SHRINEBATTLE_READY_TO_OPEN* pMsg = (PACKET_AGPP_SHRINEBATTLE_READY_TO_OPEN*)pData;
	AgpdShrinePoint* pShrine = pThis->_GetShrineInfo( pMsg->nShrinePointID );
	if( NULL == pShrine )
		return FALSE;

	//미니맵 하단 아이콘 출력
	INT32 iIndex = pThis->_GetShrineInfoIndex( pShrine->m_nShrinePointID );
	pThis->m_arShrineObject[ iIndex ].bReadyToStart = TRUE;
	pThis->m_pUICtrlShrineButton->m_pcsBase->StartAnimation();

	char szTmp[128];
	char* pszMessage = pThis->m_pUIMgr->GetUIMessage( "Shrine_Ready_to_Start_Msg" );
	sprintf( szTmp, pszMessage , pShrine->m_szPointName, pMsg->nRestMinute );
	pThis->m_pUIMgr->m_csShrineEffectMessage.SetEffectText(szTmp,5, pThis->m_iShrineMsgPos, FALSE, pThis->m_dwNormalMessageColor, 1.0f, 5000 );
	AgcChatManager::OnAddShrineMessage( szTmp );

	if( pThis->m_arShrineObject[ iIndex ].bFirstShow == FALSE )
	{
		//메세지 출력
		char szTmp[128];
		char* pszMessage = pThis->m_pUIMgr->GetUIMessage( "Shrine_Ready_to_Start" );
		sprintf( szTmp, pszMessage , pShrine->m_szPointName, pMsg->nRestMinute );
		pThis->MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_ICON_ACTIVATED);
		pThis->m_arShrineObject[iIndex].bFirstShow = TRUE;
// 		if( pThis->m_pUIMgr->ActionMessageOKCancelDialog(szTmp ) != 0 )
// 		{
// 			AgpdCharacter* pSelfCharacter = pThis->m_pAgcmCharacter->GetSelfCharacter();
// 			if(pSelfCharacter && pSelfCharacter->m_bIsTrasform)
// 			{
// 				char szTmp[128];
// 				char* pszMessage = pThis->m_pUIMgr->GetUIMessage( "Teleport_TransformChar" );
// 				sprintf( szTmp, pszMessage);
// 
// 				pThis->m_pUIMgr->ActionMessageOKDialog(szTmp);
// 				return FALSE;
// 			}
// 
// 			PACKET_AGPP_SHRINEBATTLE_JOIN_ANSWER pPacket(  pThis->m_pAgcmCharacter->GetSelfCharacter()->GetID(), pShrine->m_nShrinePointID );
// 			BOOL bResult	= g_pEngine->SendPacket( pPacket );
// 		}

	}

	return TRUE;
}


//shrine message
BOOL AgcmShrine::CBShrineMessage(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if( NULL == pClass  ||  NULL == pData )
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	PACKET_AGPP_SHRINEBATTLE_MESSAGE_CODE* pMsg = (PACKET_AGPP_SHRINEBATTLE_MESSAGE_CODE*)pData;
	BOOL bIsInShrine = FALSE;

	if( NULL == pThis->m_pAgcmCharacter && NULL == pThis->m_pAgcmCharacter->GetSelfCharacter())
		return FALSE;

	AgpdCharacter* pSelfCharacter = pThis->m_pAgcmCharacter->GetSelfCharacter();
	AgpdCharacter* pShrineCharacter = NULL;

	//쉬라인 지역에 있지 않은 캐릭터는 일부 메시지를 받지 않는다.
	if(pThis->m_pAgpmShrine->IsInShrineRegion(pSelfCharacter))
	{
		bIsInShrine = TRUE;
	}

	S_SHRINE_MSG_MESSAGE* pMessage = pThis->_GetMsgMessge( pMsg->eMessageCode );
	AgpdShrinePoint* pShrine = pThis->_GetShrineInfo( pMsg->nShrinePointID );
	if( NULL == pShrine || NULL == pMessage )
		return FALSE;

	pThis->m_nOpenedShrinePointID = pMsg->nShrinePointID;

	S_SHRINE_MSG_RACE_INFO *pRaceInfo = pThis->_GetMsgRaceInfo( pMsg->nRaceID );

	char szTmp[512];
	INT32 iIndex = 0;
	//BOOL bIsKoreanTest = AuKoreanTest::IsKoreanTest();//JK_쉬라인머지
	switch( (_eAgpmShrineBattleMessageCode)pMsg->eMessageCode )
	{
	case AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH2:
		pThis->CloseAllBattleWindow();
		sprintf(szTmp, pMessage->szMessage);
		break;

	case AGPMSHRINEBATTLE_MESSAGE_CODE_OPEN:
		pThis->m_pUICtrlShrineButton->m_pcsBase->StartAnimation();
		pThis->CloseAllBattleWindow();
		pThis->m_pUIMgr->ThrowEvent(pThis->m_iEventShrineBattleInfoOpen);
		pThis->m_pUIMgr->ThrowEvent(pThis->m_iEventShrineOpenSound);
		pThis->m_bIsOpenMaximizedWindow = TRUE;
		pThis->m_bIsOpenMinimizedWindow = FALSE;
		sprintf(szTmp, pMessage->szMessage, pShrine->m_szPointName );
		pThis->_ReservePrintMessage(szTmp, pMessage);
		iIndex = pThis->_GetShrineInfoIndex( pMsg->nShrinePointID );
		pThis->m_arShrineObject[iIndex].bReadyToStart = FALSE;
		pThis->m_arShrineObject[iIndex].bFirstShow = FALSE;
		pThis->MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_START);
		pThis->MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_OCCUPY_DURING_X_MIN);
		pThis->m_nOpenedTime = pThis->GetClockCount();
		break;

	case AGPMSHRINEBATTLE_MESSAGE_CODE_CLOSE:
		pThis->m_pUIMgr->ThrowEvent(pThis->m_iEventShrineCloseSound);
		sprintf(szTmp, pMessage->szMessage, pShrine->m_szPointName );
		pThis->_ReservePrintMessage(szTmp, pMessage);
		iIndex = pThis->_GetShrineInfoIndex( pMsg->nShrinePointID );
		pThis->m_arShrineObject[iIndex].bReadyToStart = FALSE;
		pThis->m_arShrineObject[iIndex].bFirstShow = FALSE;
		pThis->CloseAllBattleWindow();
		pThis->MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH2);
		break;

	case AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_OCCUPY:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT:
		pThis->m_pShrineCharacter = pThis->m_pAgpmCharacter->GetCharacter(pMsg->szPointCharacterName);
		pThis->MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_PROTECT_OCCUPIER, pMsg);
		pThis->MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_DEFEAT_OCCUPIER, pMsg);
	case AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT_FAIL: 
	case AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_OCCUPY_FAIL:
		// [쉬라인포인트], [종족명], [각인자] 순서로 표시되는 메시지
		if( NULL == pRaceInfo )
			return FALSE;

		//if(AP_SERVICE_AREA_GLOBAL == g_eServiceArea && !bIsKoreanTest)
		if(AP_SERVICE_AREA_GLOBAL == g_eServiceArea )//JK_쉬라인머지
			sprintf(szTmp, pMessage->szMessage, pRaceInfo->szRaceName, pMsg->szPointCharacterName, pShrine->m_szPointName );
		else
			sprintf(szTmp, pMessage->szMessage, pShrine->m_szPointName, pRaceInfo->szRaceName, pMsg->szPointCharacterName  );

		pThis->_ReservePrintMessage(szTmp, pMessage);
		break;

	case AGPMSHRINEBATTLE_MESSAGE_CODE_IMPRINT:
		// [쉬라인포인트], [종족명], [각인자] 순서로 표시되는 메시지
		if( NULL == pRaceInfo )
			return FALSE;

		//if(AP_SERVICE_AREA_GLOBAL == g_eServiceArea && !bIsKoreanTest)
		if(AP_SERVICE_AREA_GLOBAL == g_eServiceArea)//JK_쉬라인머지
			sprintf(szTmp, pMessage->szMessage, pRaceInfo->szRaceName, pMsg->szPointCharacterName, pShrine->m_szPointName );
		else
			sprintf(szTmp, pMessage->szMessage, pShrine->m_szPointName, pRaceInfo->szRaceName, pMsg->szPointCharacterName  );

		pThis->_ReservePrintMessage(szTmp, pMessage);
		break;

	case AGPMSHRINEBATTLE_MESSAGE_CODE_OCCUPYIED_BY_RACE:
		if( NULL == pRaceInfo )
			return FALSE;
		sprintf(szTmp, pMessage->szMessage, pRaceInfo->szRaceName);
		pThis->_ReservePrintMessage(szTmp, pMessage);
		pThis->MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_GET_DUNGEON_TICKET);
		pThis->MakeShrineMessageByMessageCode(AGPMSHRINEBATTLE_MESSAGE_CODE_CAN_USE_DUNGEON);
		break;
	}

	return TRUE;
}

BOOL AgcmShrine::CBShrinePvPRecord( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if(!pData || !pClass)
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	PACKET_AGPP_SHRINEBATTLE_SCORE* pPacket = (PACKET_AGPP_SHRINEBATTLE_SCORE*)pData;

	pThis->m_nMyWin		= pPacket->m_lWin;
	pThis->m_nMyLose	= pPacket->m_lLose;

	pThis->m_pUIMgr->SetUserDataRefresh(pThis->m_pUDMyWin);
	pThis->m_pUIMgr->SetUserDataRefresh(pThis->m_pUDMyLose);

	return TRUE;
}

BOOL AgcmShrine::CBShrineRaceRecord( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if(!pData || !pClass)
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	PACKET_AGPP_SHRINEBATTLE_RACE_SCORE* pPacket = (PACKET_AGPP_SHRINEBATTLE_RACE_SCORE*)pData;

	pThis->m_nRaceWin	= pPacket->m_lWin;
	pThis->m_nRaceLose	= pPacket->m_lLose;

	pThis->m_pUIMgr->SetUserDataRefresh(pThis->m_pUDRaceWin);
	pThis->m_pUIMgr->SetUserDataRefresh(pThis->m_pUDRaceLose);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//function
//////////////////////////////////////////////////////////////////////////
BOOL AgcmShrine::CBOpenShrineInfo(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass || NULL == pData1 )
		return FALSE;

	AgcmShrine* pThis = (AgcmShrine*)pClass;
	AgcdUI* pUI = (AgcdUI*)pData1;

	if(!pThis->m_pAgcmCharacter->GetSelfCharacter())
		return FALSE;

	LPDIRECT3DDEVICE9 pDevice = (LPDIRECT3DDEVICE9)RwD3D9GetCurrentD3DDevice();
	LPDIRECT3DSURFACE9		pBackBuffer = NULL;
	D3DSURFACE_DESC	surf_desc;

	pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
	pBackBuffer->GetDesc( &surf_desc );
	pBackBuffer->Release();

	int x = surf_desc.Width / 2 - pUI->m_pcsUIWindow->w / 2 ;
	int y = surf_desc.Height/ 2 - pUI->m_pcsUIWindow->h / 2 ;

	pUI->m_pcsUIWindow->MoveWindow( x,y, pUI->m_pcsUIWindow->w,pUI->m_pcsUIWindow->h);

	pThis->m_ShrineIcon.SetMapUI( pUI );
	//minasang: sound
	if( pUI->m_eStatus == AGCDUI_STATUS_CLOSING )
		pThis->m_pUIMgr->ThrowEvent( pThis->m_pUIMgr->m_nEventToggleUIClose );
	else
		pThis->m_pUIMgr->ThrowEvent( pThis->m_pUIMgr->m_nEventToggleUIOpen );

	//request infomation
	PACKET_AGPP_SHRINEBATTLE_INFORMATION_REQUEST pPacket(  pThis->m_pAgcmCharacter->GetSelfCharacter()->GetID() );
	BOOL bResult	= g_pEngine->SendPacket( pPacket );

	return TRUE;
}

BOOL AgcmShrine::CBShrineIconClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	INT32 iIndex = pThis->_GetShrineInfoIndex( "C_Shrine_icon_%d", pcsSourceControl->m_szName );
	AgpdShrinePoint* pShrinePoint = &pThis->m_arrShrinPointInfo[iIndex];
	if( pShrinePoint == NULL )
		return FALSE;

	if( pThis->_IsJoinShrine( iIndex ) == FALSE )
		return FALSE;

	AgpdCharacter* pSelfCharacter = pThis->m_pAgcmCharacter->GetSelfCharacter();
	if(pSelfCharacter && pSelfCharacter->m_bIsTrasform)
	{
		char szTmp[128];
		char* pszMessage = pThis->m_pUIMgr->GetUIMessage( "Teleport_TransformChar" );
		sprintf( szTmp, pszMessage);

		pThis->m_pUIMgr->ActionMessageOKDialog(szTmp);
		return FALSE;
	}
	//쉬라인 참가 가능할때 참가 여부를 묻는 메세지창
	if(		pShrinePoint->m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_OPEN 
		||	pShrinePoint->m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_TRYIMPRINT 
		||	pShrinePoint->m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_IMPRINT
		||	pShrinePoint->m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_TRYOCCUPY
		||	pThis->m_arShrineObject[iIndex].bReadyToStart == (BOOL)TRUE )
	{
		char szTmp[128];
		char* pszMessage = pThis->m_pUIMgr->GetUIMessage( "Shrine_Join_Question" );
		sprintf( szTmp, pszMessage , pShrinePoint->m_szPointName );
		if( pThis->m_pUIMgr->ActionMessageOKCancelDialog(szTmp ) != 0 )
		{
			PACKET_AGPP_SHRINEBATTLE_JOIN_ANSWER pPacket(  pThis->m_pAgcmCharacter->GetSelfCharacter()->GetID(), pShrinePoint->m_nShrinePointID );
			BOOL bResult	= g_pEngine->SendPacket( pPacket );
		}
	}

	return TRUE;
}

// 쉬라인 전투 창 UI

BOOL AgcmShrine::CBMaximizeShrineBattleInfo( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass || NULL == pData1 || NULL == pData2)
		return FALSE;

	AgcmShrine* pThis = static_cast<AgcmShrine*>(pClass);
	AgcdUI* pUIMaximized = static_cast<AgcdUI*>(pData1);
	AgcdUI* pUIMinimized = static_cast<AgcdUI*>(pData2);

	pThis->m_nBattleInfoPosX = pUIMinimized->m_pcsUIWindow->x;
	pThis->m_nBattleInfoPosY = pUIMinimized->m_pcsUIWindow->y;

	if(pThis->IsOpenedMiniWindow())
	{
		pThis->m_pUIMgr->CloseUI(pUIMinimized);
		pThis->m_bIsOpenMinimizedWindow = FALSE;
	}
	pUIMaximized->m_pcsUIWindow->x = pThis->m_nBattleInfoPosX;
	pUIMaximized->m_pcsUIWindow->y = pThis->m_nBattleInfoPosY;

	pThis->m_pUIMgr->OpenUI(pUIMaximized);
	pThis->m_bIsOpenMaximizedWindow = TRUE;

	return TRUE;
}

BOOL AgcmShrine::CBMinimizeShrineBattleInfo( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass || NULL == pData1 || NULL == pData2)
		return FALSE;

	AgcmShrine* pThis = static_cast<AgcmShrine*>(pClass);
	AgcdUI* pUIMaximized = static_cast<AgcdUI*>(pData1);
	AgcdUI* pUIMinimized = static_cast<AgcdUI*>(pData2);

	pThis->m_nBattleInfoPosX = pUIMaximized->m_pcsUIWindow->x;
	pThis->m_nBattleInfoPosY = pUIMaximized->m_pcsUIWindow->y;

	if(pThis->IsOpenedMaxWindow())
	{
		pThis->m_pUIMgr->CloseUI(pUIMaximized);
		pThis->m_bIsOpenMaximizedWindow = FALSE;
	}
	pUIMinimized->m_pcsUIWindow->x = pThis->m_nBattleInfoPosX;
	pUIMinimized->m_pcsUIWindow->y = pThis->m_nBattleInfoPosY;

	pThis->m_pUIMgr->OpenUI(pUIMinimized);
	pThis->m_bIsOpenMinimizedWindow = TRUE;

	return TRUE;
}



//쉬라인 상황 정보 탭 클릭시
BOOL AgcmShrine::CBShrineTapStatus(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	pThis->_ChangeTapUI( (INT32)E_TAP_INFO );

	return TRUE;
}

//쉬라인 상황 정보 탭 클릭시
BOOL AgcmShrine::CBShrineTapScroe(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	//필요시 추가 하면 된다.
	//pThis->_ChangeTapUI( (INT32)E_TAP_SCROE);

	return TRUE;
}

//쉬라인 상황 정보 페이지 이전 버튼
BOOL AgcmShrine::CBInfoListPrev(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	pThis->m_iCurPage--;
	if( pThis->m_iCurPage < 0 )
		pThis->m_iCurPage = 0;

	pThis->m_pUIMgr->SetUserDataRefresh( pThis->m_pUDShrineInfoList );
	pThis->m_pUIMgr->SetUserDataRefresh( pThis->m_pUDShrineListPage );
	return TRUE;
}

//쉬라인 상황 정보 페이지 다음 버튼
BOOL AgcmShrine::CBInfoListNext(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if( NULL == pClass)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	pThis->m_iCurPage++;
	INT32 iMaxPage = pThis->m_iShrineEnableCount / 3;

	if( pThis->m_iCurPage > iMaxPage )
		pThis->m_iCurPage = iMaxPage;

	pThis->m_pUIMgr->SetUserDataRefresh( pThis->m_pUDShrineInfoList );
	pThis->m_pUIMgr->SetUserDataRefresh( pThis->m_pUDShrineListPage );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//Display
//////////////////////////////////////////////////////////////////////////
BOOL AgcmShrine::CBShrineHPMAX( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	AgpdShrinePoint* pShrinePoint = pThis->_GetShrineInfo( pThis->m_iCurProcessShrine );
	if( pShrinePoint == NULL )
		return FALSE;

	if( pThis->m_bStartImprint )
		*plValue = pShrinePoint->m_nImprintTime;
	else
		*plValue = pShrinePoint->m_nOccupyTime;

	sprintf( szDisplay, "%d", *plValue);
	return TRUE;
}

BOOL AgcmShrine::CBShrineHPCUR( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	AgpdShrinePoint* pShrinePoint = pThis->_GetShrineInfo( pThis->m_iCurProcessShrine );
	if( pShrinePoint == NULL )
		return FALSE;

	*plValue = pShrinePoint->m_nElapsedTime;

	sprintf( szDisplay, "%d", *plValue);
	return TRUE;
}

BOOL AgcmShrine::CBShrineHPDP( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	AgpdShrinePoint* pShrinePoint = pThis->_GetShrineInfo( pThis->m_iCurProcessShrine );
	if( pShrinePoint == NULL )
		return FALSE;

	//BOOL bIsKoreanTest = AuKoreanTest::IsKoreanTest();//JK_쉬라인머지

	INT32 iCur = 0;
	INT32 iMax = 0; 

	iCur = pShrinePoint->m_nElapsedTime;

	if( pThis->m_bStartImprint )
		iMax = pShrinePoint->m_nImprintTime;
	else
		iMax = pShrinePoint->m_nOccupyTime;

	char *pszTemp = pThis->m_pUIMgr->GetUIMessage("Shrine_Gauge");
	if( pszTemp != NULL )
		//if(AP_SERVICE_AREA_GLOBAL == g_eServiceArea && !bIsKoreanTest)
		if(AP_SERVICE_AREA_GLOBAL == g_eServiceArea )//JK_쉬라인머지
			sprintf( szDisplay, pszTemp, pThis->m_strCurName.GetBuffer(0), pThis->m_strCurRace.GetBuffer(0), pThis->m_strCurAction.GetBuffer(0), iCur/1000, iMax/1000 );
		else
			sprintf( szDisplay, pszTemp, pThis->m_strCurRace.GetBuffer(0), pThis->m_strCurName.GetBuffer(0), pThis->m_strCurAction.GetBuffer(0), iCur/1000, iMax/1000 );

	return TRUE;
}

BOOL AgcmShrine::CBShrineUpdate( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	int i = pThis->_GetShrineInfoIndex("C_Shrine_%d", pcsSourceControl->m_szName);
	sprintf( szDisplay, "%s", pThis->m_arrShrinPointInfo[i].m_szPointName );

	return TRUE;
}

BOOL AgcmShrine::CBShrineOccHCount( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	sprintf( szDisplay, "%d", pThis->_GetShrineOccupateCount(AURACE_TYPE_HUMAN ) );

	return TRUE;
}

BOOL AgcmShrine::CBShrineOccOCount( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	sprintf( szDisplay, "%d", pThis->_GetShrineOccupateCount(AURACE_TYPE_ORC ) );

	return TRUE;
}

//Shrine Information status Display
BOOL AgcmShrine::CBDPListStatus( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	INT32 iIndex = pThis->m_iCurPage * 3 + pcsSourceControl->m_lUserDataIndex;
	if( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID == 0 )
		pcsSourceControl->m_pcsBase->ShowWindow( FALSE );
	else
		pcsSourceControl->m_pcsBase->ShowWindow( TRUE );

	if( pThis->m_arShrineObject[iIndex].bReadyToStart == FALSE )
	{
		INT32 iID = pThis->_GetShrineInfoIndex( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID );
		switch( pThis->_GetShrineStatus( &pThis->m_arrShrinPointInfo[iID] ) )
		{
		case	AGUIDrawMark::E_TYPE_SHRINE_ENABLE:
		case	AGUIDrawMark::E_TYPE_SHRINE_BATTLE:
			pcsSourceControl->m_pcsBase->SetStatus(pcsSourceControl->m_pcsBase->m_astStatus[ 0 ].m_lStatusID);
			break;
		default:
			pcsSourceControl->m_pcsBase->SetStatus(pcsSourceControl->m_pcsBase->m_astStatus[ 1 ].m_lStatusID);
			break;
		}
	}
	else
	{
		pcsSourceControl->m_pcsBase->SetStatus(pcsSourceControl->m_pcsBase->m_astStatus[ 1 ].m_lStatusID);
	}

	return TRUE;
}

//쉬라인 이름
BOOL AgcmShrine::CBDPListName( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{ 
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	INT32 iIndex = pThis->m_iCurPage * 3 + pcsSourceControl->m_lUserDataIndex;
	INT32 iID = pThis->_GetShrineInfoIndex( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID );
	sprintf( szDisplay, "%s", pThis->m_arrShrinPointInfo[iID].m_szPointName );

	return TRUE;
}

//쉬라인 타임바
BOOL AgcmShrine::CBDPListShrineStatusBarCur( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{ 
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	INT32 iIndex = pThis->m_iCurPage * 3 + pcsSourceControl->m_lUserDataIndex;
	INT32 iID = pThis->_GetShrineInfoIndex( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID );

	if(pThis->m_arrShrinPointInfo[iID].m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_END 
		|| pThis->m_arrShrinPointInfo[iID].m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_NORMAL)
		*plValue = 100;
	else
		*plValue = pThis->m_arrShrinPointInfo[iID].m_nElapsedTime;

	sprintf( szDisplay, "%d", *plValue);

	return TRUE;
}

BOOL AgcmShrine::CBDPListShrineStatusBarMax( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{ 
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	INT32 iIndex = pThis->m_iCurPage * 3 + pcsSourceControl->m_lUserDataIndex;
	INT32 iID = pThis->_GetShrineInfoIndex( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID );

	if( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID == 0 )
		pcsSourceControl->m_pcsBase->ShowWindow( FALSE );
	else
		pcsSourceControl->m_pcsBase->ShowWindow( TRUE );

	if(pThis->m_arrShrinPointInfo[iID].m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_END 
		|| pThis->m_arrShrinPointInfo[iID].m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_NORMAL)
	{
		*plValue = 100;
		((AcUIBar*)pcsSourceControl->m_pcsBase)->SetBodyImageID(3);
		((AcUIBar*)pcsSourceControl->m_pcsBase)->SetEdgeImageID(3);
	}
	else
	{
		switch( pThis->m_arrShrinPointInfo[iID].m_nCurrentStatus )
		{
		case AGPDSHRINEBATTLE_STATUS_OPEN:
			*plValue = 0;
			((AcUIBar*)pcsSourceControl->m_pcsBase)->SetBodyImageID(1);
			((AcUIBar*)pcsSourceControl->m_pcsBase)->SetEdgeImageID(1);
			break;
		case AGPDSHRINEBATTLE_STATUS_TRYIMPRINT:
		case AGPDSHRINEBATTLE_STATUS_IMPRINT:
			*plValue =  pThis->m_arrShrinPointInfo[iID].m_nImprintTime;
			((AcUIBar*)pcsSourceControl->m_pcsBase)->SetBodyImageID(1);
			((AcUIBar*)pcsSourceControl->m_pcsBase)->SetEdgeImageID(1);
			break;
		case AGPDSHRINEBATTLE_STATUS_TRYOCCUPY:
		case AGPDSHRINEBATTLE_STATUS_OCCUPY:
			*plValue =  pThis->m_arrShrinPointInfo[iID].m_nOccupyTime;
			((AcUIBar*)pcsSourceControl->m_pcsBase)->SetBodyImageID(2);
			((AcUIBar*)pcsSourceControl->m_pcsBase)->SetEdgeImageID(1);
			break;
		}
	}

	sprintf( szDisplay, "%d", *plValue);

	return TRUE;

}

//쉬라인 상태 텍스트
BOOL AgcmShrine::CBDPListShrineStatusText( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{ 
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	INT32 iIndex = pThis->m_iCurPage * 3 + pcsSourceControl->m_lUserDataIndex;
	INT32 iID = pThis->_GetShrineInfoIndex( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID );

	char* pszStatus = NULL;
	char szTemp[64] = {0,};

	switch( pThis->m_arrShrinPointInfo[iID].m_nCurrentStatus )
	{
	case AGPDSHRINEBATTLE_STATUS_OPEN:
		pszStatus = pThis->m_pUIMgr->GetUIMessage("Shrine_Status_Open");
		break;
	case AGPDSHRINEBATTLE_STATUS_TRYIMPRINT:
	case AGPDSHRINEBATTLE_STATUS_IMPRINT:
		pszStatus = pThis->m_pUIMgr->GetUIMessage("Shrine_Status_Imprint");
		break;
	case AGPDSHRINEBATTLE_STATUS_TRYOCCUPY:
	case AGPDSHRINEBATTLE_STATUS_OCCUPY:
		pszStatus = pThis->m_pUIMgr->GetUIMessage("Shrine_Status_Occupy");
		break;
	case AGPDSHRINEBATTLE_STATUS_END:
	case AGPDSHRINEBATTLE_STATUS_NORMAL:
		sprintf(szTemp, pThis->m_pUIMgr->GetUIMessage("Shrine_Open_Time"), pThis->m_arrShrinPointInfo[iID].m_nStartHour);
		pszStatus = szTemp;
		break;

	}

	if( pszStatus != NULL )
		sprintf( szDisplay, "%s", pszStatus );

	return TRUE;
}

//쉬라인 점령자 종족 아이콘
BOOL AgcmShrine::CBDPListOccupyRace( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{ 
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	INT32 iIndex = pThis->m_iCurPage * 3 + pcsSourceControl->m_lUserDataIndex;
	INT32 iID = pThis->_GetShrineInfoIndex( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID );

	switch( pThis->_GetShrineStatus( &pThis->m_arrShrinPointInfo[iID] ) )
	{
	case	AGUIDrawMark::E_TYPE_SHRINE_HUMN:
		pcsSourceControl->m_pcsBase->SetStatus(pcsSourceControl->m_pcsBase->m_astStatus[ 1 ].m_lStatusID);
		break;
	case	AGUIDrawMark::E_TYPE_SHRINE_ORC:
		pcsSourceControl->m_pcsBase->SetStatus(pcsSourceControl->m_pcsBase->m_astStatus[ 2 ].m_lStatusID);
		break;
	case	AGUIDrawMark::E_TYPE_SHRINE_MOONELF:
		pcsSourceControl->m_pcsBase->SetStatus(pcsSourceControl->m_pcsBase->m_astStatus[ 4 ].m_lStatusID);
		break;
	case	AGUIDrawMark::E_TYPE_SHRINE_DRAGONSION:
		pcsSourceControl->m_pcsBase->SetStatus(pcsSourceControl->m_pcsBase->m_astStatus[ 3 ].m_lStatusID);
		break;
	default:
		pcsSourceControl->m_pcsBase->SetStatus(pcsSourceControl->m_pcsBase->m_astStatus[ 0 ].m_lStatusID);
	}

	return TRUE;
}


//쉬라인 각인자/점령자 이름
BOOL AgcmShrine::CBDPListOccupyPCName( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{ 
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	INT32 iIndex = pThis->m_iCurPage * 3 + pcsSourceControl->m_lUserDataIndex;
	INT32 iID = pThis->_GetShrineInfoIndex( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID );

	if( iID < pThis->m_iShrineEnableCount )
		sprintf( szDisplay, "%s",	 pThis->m_arrShrinPointInfo[iID].m_szOccupyCharacterName );
	else
		sprintf( szDisplay, "" );

	return TRUE;
}

//쉬라인 닫힐 때 까지의 시간
BOOL AgcmShrine::CBDPListRemainTime( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass || NULL == pcsSourceControl)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	INT32 iIndex = pThis->m_iCurPage * 3 + pcsSourceControl->m_lUserDataIndex;
	INT32 iID = pThis->_GetShrineInfoIndex( pThis->m_arrShrineSortInfo[iIndex].iShrinePointID );

	AgpdShrinePoint& shrinePoint = pThis->m_arrShrinPointInfo[iID];

	if( shrinePoint.m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_END ||
		shrinePoint.m_nCurrentStatus ==  AGPDSHRINEBATTLE_STATUS_NORMAL )
		return FALSE;

	UINT32 iSceonds = (UINT32)(shrinePoint.m_nRemainTime * 0.001f);
	UINT32 iHour = iSceonds / 60;

	sprintf( szDisplay, "%02d : %02d", iHour, iSceonds%60 );

	return TRUE;
}

//Shrine Info List Page
BOOL AgcmShrine::CBDPListPage( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{

	if( NULL == pClass)
		return FALSE;
	AgcmShrine* pThis = (AgcmShrine*)pClass;

	sprintf( szDisplay, "%d / %d", pThis->m_iCurPage+1, pThis->m_iShrineEnableCount/3+1 );
	return TRUE;
}

BOOL AgcmShrine::CBDPMaxLefttime( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass )
		return FALSE;

	AgcmShrine* pThis = static_cast<AgcmShrine*>(pClass);

	sprintf( szDisplay, "%d %s", pThis->m_nShrineBattleInfoTime, ClientStr().GetStr(STI_MINUTE) );
	return TRUE;
}

BOOL AgcmShrine::CBDPMiniLefttime( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass )
		return FALSE;

	AgcmShrine* pThis = static_cast<AgcmShrine*>(pClass);

	sprintf( szDisplay, "%d %s", pThis->m_nShrineBattleInfoTime, ClientStr().GetStr(STI_MINUTE) );
	return TRUE;
}

BOOL AgcmShrine::CBDPMaxMyWin( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass )
		return FALSE;

	AgcmShrine* pThis = static_cast<AgcmShrine*>(pClass);

	sprintf(szDisplay, "%d", pThis->m_nMyWin);
	return TRUE;
}

BOOL AgcmShrine::CBDPMaxMyLose( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass )
		return FALSE;

	AgcmShrine* pThis = static_cast<AgcmShrine*>(pClass);

	sprintf(szDisplay, "%d", pThis->m_nMyLose);
	return TRUE;
}

BOOL AgcmShrine::CBDPMaxRaceWin( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass )
		return FALSE;

	AgcmShrine* pThis = static_cast<AgcmShrine*>(pClass);

	sprintf(szDisplay, "%d", pThis->m_nRaceWin);
	return TRUE;
}

BOOL AgcmShrine::CBDPMaxRaceLose( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	if( NULL == pClass )
		return FALSE;

	AgcmShrine* pThis = static_cast<AgcmShrine*>(pClass);

	sprintf(szDisplay, "%d", pThis->m_nRaceLose);
	return TRUE;
}

BOOL AgcmShrine::CloseAllBattleWindow()
{
	AgcdUI* pMaxWindow = m_pUIMgr->GetUI(SHRINEBATTLEWINDOWNAME_MAX);
	AgcdUI* pMiniWindow = m_pUIMgr->GetUI(SHRINEBATTLEWINDOWNAME_MINI);

	if(!pMaxWindow || !pMiniWindow)
		return FALSE;

	m_pUIMgr->CloseUI(pMaxWindow, 0, 0, 1, 0);
	m_pUIMgr->CloseUI(pMiniWindow, 0, 0, 1, 0);

	m_nShrineBattleInfoTime = 60;

	m_bIsOpenMaximizedWindow = FALSE;
	m_bIsOpenMinimizedWindow = FALSE;

	return TRUE;
}

BOOL AgcmShrine::CheckBattleWindow()
{
	AgcdUI* pMaxWindow = m_pUIMgr->GetUI(SHRINEBATTLEWINDOWNAME_MAX);
	AgcdUI* pMiniWindow = m_pUIMgr->GetUI(SHRINEBATTLEWINDOWNAME_MINI);

	if(!pMaxWindow || !pMiniWindow)
		return FALSE;

	if(pMaxWindow->m_eStatus == AGCDUI_STATUS_OPENING || pMaxWindow->m_eStatus == AGCDUI_STATUS_OPENED)
		m_bIsOpenMaximizedWindow = TRUE;
	else
		m_bIsOpenMaximizedWindow = FALSE;

	if(pMiniWindow->m_eStatus == AGCDUI_STATUS_OPENING || pMiniWindow->m_eStatus == AGCDUI_STATUS_OPENED)
		m_bIsOpenMinimizedWindow = TRUE;
	else
		m_bIsOpenMinimizedWindow = FALSE;

	return TRUE;
}


BOOL AgcmShrine::CBShrineBattleInfoOpenUI( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl )
{
	if(!pClass || !pData1 || !pData2)
		return FALSE;

	AgcmShrine *pThis = (AgcmShrine *)pClass;
	AgcdUI* pMaxWindow = (AgcdUI*)pData1;
	AgcdUI* pMiniWindow = (AgcdUI*)pData2;

	if(pMaxWindow->m_eStatus != AGCDUI_STATUS_OPENED && pMaxWindow->m_eStatus != AGCDUI_STATUS_OPENING && pMiniWindow->m_eStatus != AGCDUI_STATUS_OPENED && pMiniWindow->m_eStatus != AGCDUI_STATUS_OPENING )
	{
		pThis->CloseAllBattleWindow();
		pThis->m_nMyLose = 0;
		pThis->m_nMyWin = 0;
		pThis->m_nRaceLose = 0;
		pThis->m_nRaceWin = 0;
		pThis->m_nShrineBattleInfoTime = 60;
		pThis->m_pUIMgr->OpenUI(pMaxWindow);
		pThis->m_bIsOpenMaximizedWindow = TRUE;
		pThis->m_bIsOpenMinimizedWindow = FALSE;
	}

	return TRUE;
}

BOOL AgcmShrine::MakeShrineMessageByMessageCode( eAgpmShrineBattleMessageCode eCode, PVOID pData1 )
{
	S_SHRINE_MSG_MESSAGE* pMessage = _GetMsgMessge( eCode );
	PACKET_AGPP_SHRINEBATTLE_MESSAGE_CODE* pMsg = (PACKET_AGPP_SHRINEBATTLE_MESSAGE_CODE*)pData1;
	if(!pMessage || -1 == m_nOpenedShrinePointID)
		return FALSE;

	CHAR *pszRace[AURACE_TYPE_MAX];
	pszRace[AURACE_TYPE_HUMAN] = ClientStr().GetStr(STI_RACE_BATTLE_HUMAN);
	pszRace[AURACE_TYPE_ORC] = ClientStr().GetStr(STI_RACE_BATTLE_ORC);
	pszRace[AURACE_TYPE_MOONELF] = ClientStr().GetStr(STI_RACE_BATTLE_MOONELF);
	pszRace[AURACE_TYPE_DRAGONSCION] = ClientStr().GetStr(STI_DRAGONSCION);	

	char szTmp[512] = {0,};
	//BOOL bIsKoreanTest = AuKoreanTest::IsKoreanTest();

	switch(eCode)
	{
	case AGPMSHRINEBATTLE_MESSAGE_CODE_OPEN_REMAIN_5MIN:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_OPEN_REMAIN_1MIN:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_ICON_ACTIVATED:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_START:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_CANNOT_USE_ICON:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_COULD_BE_CANCEL:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_GET_DUNGEON_TICKET:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH_REMAIN_10MIN:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH_REMAIN_5MIN:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH2:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_CAN_USE_DUNGEON:
		sprintf(szTmp, pMessage->szMessage);
		break;

	case AGPMSHRINEBATTLE_MESSAGE_CODE_OCCUPY_DURING_X_MIN:
		sprintf(szTmp, pMessage->szMessage, (INT32)(m_arrShrinPointInfo[m_nOpenedShrinePointID].m_nOccupyTime*0.001/60));
		break;

	case AGPMSHRINEBATTLE_MESSAGE_CODE_PROTECT_OCCUPIER:
	case AGPMSHRINEBATTLE_MESSAGE_CODE_DEFEAT_OCCUPIER:
		if(!m_pShrineCharacter || !m_pAgpmCharacter )
			return FALSE;
		if(m_pShrineCharacter->GetID() != m_pAgcmCharacter->GetSelfCharacter()->GetID())
			sprintf(szTmp, pMessage->szMessage, pszRace[m_pAgpmCharacter->GetCharacterRace(m_pAgcmCharacter->GetSelfCharacter()).detail.nRace], m_pShrineCharacter->m_szID);

		break;
	case AGPMSHRINEBATTLE_MESSAGE_CODE_VICTORY_MESSAGE:
		sprintf(szTmp, pMessage->szMessage, _GetMsgRaceInfo(pMsg->nRaceID)->szRaceName, m_pShrineCharacter->m_szID);
		break;
	}

	_ReservePrintMessage(szTmp, pMessage);

	return TRUE;
}

BOOL AgcmShrine::_SelfPrintShrineMessageChat(std::string strMessage)
{
	m_listShrineMessageChat.push_back(strMessage);
	return TRUE;
}

BOOL AgcmShrine::_SelfPrintShrineMessageCenter( S_SHRINE_MSG_NODE strMessage )
{
	m_listShrineMessageCenter.push_back(strMessage);
	return TRUE;
}

BOOL AgcmShrine::SelfPrintMessageChat( std::string strMessage )
{
	AgcChatManager::OnAddShrineMessage( (char*)strMessage.c_str() );
	return TRUE;
}

BOOL AgcmShrine::SelfPrintMessageCenter( S_SHRINE_MSG_NODE nodeShrineMsg )
{
	m_pUIMgr->m_csShrineEffectMessage.SetEffectText((char*)nodeShrineMsg.strMessage.c_str(), 5, m_iShrineMsgPos, FALSE, nodeShrineMsg.strColor, 1.0f, 5000 );
	return TRUE;
}

BOOL AgcmShrine::_ReservePrintMessage( std::string strMessage, PVOID pData1 /*= NULL*/ )
{
	if(strMessage.size() == 0 || !pData1)
		return FALSE;

	S_SHRINE_MSG_MESSAGE* pMessage = (S_SHRINE_MSG_MESSAGE*)pData1;

	BOOL bIsPrintable = FALSE;

	if(_MessagePrintCheckerTargetRace(pMessage, S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_SAME_RACE, m_pAgpmCharacter->IsSameRace(m_pShrineCharacter, m_pAgcmCharacter->GetSelfCharacter()))||
		_MessagePrintCheckerTargetRace(pMessage, S_SHRINE_MSG_MESSAGE::E_SHRINE_MSG_TARGET_OTHER_RACE, !m_pAgpmCharacter->IsSameRace(m_pShrineCharacter, m_pAgcmCharacter->GetSelfCharacter())))
	{
		bIsPrintable = TRUE;
	}

	if(bIsPrintable)
	{
		if( _MessagePrintCheckerType(pMessage, S_SHRINE_MSG_MESSAGE::E_MSG_SHOW_MAIN) )
		{
			S_SHRINE_MSG_NODE _message;
			_message.strMessage = strMessage;
			_message.strColor = pMessage->dwColor;
			_SelfPrintShrineMessageCenter(_message);
		}

		if( _MessagePrintCheckerType(pMessage, S_SHRINE_MSG_MESSAGE::E_MSG_SHOW_CHAT) )
		{
			_SelfPrintShrineMessageChat(strMessage);
		}
	}
	return TRUE;
}

BOOL AgcmShrine::IsOtherRace( AgpdCharacter* pcdCharacter )
{
	if( !pcdCharacter )
		return FALSE;
	if( !m_pAgpmShrine->IsInShrineRegionByIndex(pcdCharacter) )
		return FALSE;
	if(	!m_pAgpmCharacter->IsPC(pcdCharacter) )
		return FALSE;

	AgpdCharacter* pcdSelfCharacter = m_pAgcmCharacter->GetSelfCharacter();
	if(pcdSelfCharacter && m_pAgpmCharacter->IsSameRace(pcdSelfCharacter, pcdCharacter))
		return FALSE;

	return TRUE;
}
