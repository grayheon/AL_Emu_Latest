#include "AgpmShrineBattle.h"
#include "AuTimeStamp.h"
#include "AuExcelTxtLib.h"
#include "ApUtil.h"
#include "AuExcelBinaryLib.h"
#include "AgpdCharacter.h"
#include "AgpmBattleGround.h"

AgpmShrineBattle::AgpmShrineBattle()
{
	SetModuleName("AgpmShrineBattle");
	SetModuleType(APMODULE_TYPE_PUBLIC);
	SetPacketType(AGPM_SHRINEBATTLE_PACKET_TYPE);

}

AgpmShrineBattle::~AgpmShrineBattle()
{
}

BOOL AgpmShrineBattle::OnAddModule()
{
	m_pAgpmCharacter = (AgpmCharacter *) GetModule("AgpmCharacter");

	if (!m_pAgpmCharacter)
		return FALSE;



	return TRUE;
}

BOOL AgpmShrineBattle::OnInit()
{
	m_pAgpmCharacter	= (AgpmCharacter*) GetModule("AgpmCharacter");
	m_pAgpmSkill		= (AgpmSkill*) GetModule("AgpmSkill");
	m_pAgpmBattleGround	= (AgpmBattleGround *) GetModule("AgpmBattleGround");

	if(!m_pAgpmCharacter || !m_pAgpmSkill || !m_pAgpmBattleGround)
		return FALSE;

	return TRUE;
}

BOOL AgpmShrineBattle::OnDestroy()
{
	return TRUE;
}


BOOL AgpmShrineBattle::OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)
{
	if(!pvPacket || nSize == 0)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE* pPacket = (PACKET_AGPP_SHRINEBATTLE*)pvPacket;

	switch(pPacket->nParam)
	{
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_INFORMATION_REQUEST:
		return OnProcessShrinePointInformationRequest(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_INFORMATION:
		return OnProcessShrinePointInformation(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_INFORMATION_TOTAL:
		return OnProcessShrinePointInformationTotal(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_TIMEBAR_STATE:
		return OnProcessShrinePointTimeBarUpdate(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_STATUS_UPDATE:
		return OnProcessShrinePointStatusUpdate(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_TRYIMPRINT:
		return OnProcessShrinePointTryImprint(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_IMPRINT_RESULT:
		return OnProcessShrinePointTryImprintResult(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_MESSAGE_CODE:
		return OnProcessShrinePointMessageCode(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_JOIN_ANSWER:
		return OnProcessShrinePointJoinAnswer(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_READY_TO_START:
		return OnProcessShrinePointReadyToStart( pPacket );
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_PVP_RECORD:
		return OnProcessShrinePointPvPRecord(pPacket);
		break;
	case AGPMSHRINEBATTLE_PACKET_SHRINEPOINT_RACE_RECORD:
		return OnProcessShrinePointRaceRecord(pPacket);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL AgpmShrineBattle::IsInShrineRegion(INT32 lRegionIndex)
{
	if(m_stShrineBattle.m_lMaxRegionIndex >= lRegionIndex && m_stShrineBattle.m_lMinRegionIndex <= lRegionIndex)
		return TRUE;
	return FALSE;
}

BOOL AgpmShrineBattle::OnProcessShrinePointInformationRequest(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_INFORMATION_REQUEST, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointInformation(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	if(!pPacket)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_INFORMATION* pPacket2 = (PACKET_AGPP_SHRINEBATTLE_INFORMATION*) pPacket;

	if(pPacket2->nShrinePointID == 0)
		return FALSE;

	IterShrinePoint iter = m_stShrineBattle.FindByShrinePointID(pPacket2->nShrinePointID);
	if(!iter)
		return FALSE;

	iter->m_nStartHour = pPacket2->stShrinePoint.m_nStartHour;
	iter->m_nIntervalHour = pPacket2->stShrinePoint.m_nIntervalHour;
	iter->m_nTimeLimitTime = pPacket2->stShrinePoint.m_nTimeLimitTime;
	iter->m_nImprintTime = pPacket2->stShrinePoint.m_nImprintTime;
	iter->m_nOccupyTime = pPacket2->stShrinePoint.m_nOccupyTime;

	iter->m_nCurrentStatus = pPacket2->stShrinePoint.m_nCurrentStatus;
	iter->m_nOccupationRace = pPacket2->stShrinePoint.m_nOccupationRace;
	iter->m_nTryOccupyRace = pPacket2->stShrinePoint.m_nTryOccupyRace;
	iter->m_nBattleOpenTime = pPacket2->stShrinePoint.m_nBattleOpenTime;
	iter->m_nImprintStartTime = pPacket2->stShrinePoint.m_nImprintStartTime;
	iter->m_nOccupyStartTime = pPacket2->stShrinePoint.m_nOccupyStartTime;
	iter->m_nOccupyCharacterCid = pPacket2->stShrinePoint.m_nOccupyCharacterCid;
	iter->m_nElapsedTime = pPacket2->stShrinePoint.m_nElapsedTime;

	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_INFORMATION, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointInformationTotal(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	if(!pPacket)
		return FALSE;

	if(!m_stShrineBattle.m_bLoad)
	{
		PACKET_AGPP_SHRINEBATTLE_INFORMATION_TOTAL* pPacket2 = (PACKET_AGPP_SHRINEBATTLE_INFORMATION_TOTAL*) pPacket;

		for(int i = 0; i < AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; i++)
		{
			if(pPacket2->m_stPointInformation[i].nShrinePointID == 0)
				break;

			AgpdShrinePoint stShrinePoint;

			stShrinePoint.m_nShrinePointID = pPacket2->m_stPointInformation[i].nShrinePointID;
			strncpy(stShrinePoint.m_szPointName, pPacket2->m_stPointInformation[i].stShrinePoint.m_szPointName, AGPDSHRINEBATTLE_POINT_MAX_NAME_LENGTH);

			stShrinePoint.m_nStartHour = pPacket2->m_stPointInformation[i].stShrinePoint.m_nStartHour;
			stShrinePoint.m_nIntervalHour = pPacket2->m_stPointInformation[i].stShrinePoint.m_nIntervalHour;
			stShrinePoint.m_nTimeLimitTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nTimeLimitTime;
			stShrinePoint.m_nImprintTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nImprintTime;
			stShrinePoint.m_nOccupyTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nOccupyTime;

			stShrinePoint.m_nCurrentStatus = pPacket2->m_stPointInformation[i].stShrinePoint.m_nCurrentStatus;
			stShrinePoint.m_nOccupationRace = pPacket2->m_stPointInformation[i].stShrinePoint.m_nOccupationRace;
			stShrinePoint.m_nTryOccupyRace = pPacket2->m_stPointInformation[i].stShrinePoint.m_nTryOccupyRace;
			stShrinePoint.m_nBattleOpenTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nBattleOpenTime;
			stShrinePoint.m_nImprintStartTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nImprintStartTime;
			stShrinePoint.m_nOccupyStartTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nOccupyStartTime;
			stShrinePoint.m_nOccupyCharacterCid = pPacket2->m_stPointInformation[i].stShrinePoint.m_nOccupyCharacterCid;
			stShrinePoint.m_nElapsedTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nElapsedTime;

			m_stShrineBattle.m_vtShrinePoint.push_back(stShrinePoint);
		}

		m_stShrineBattle.m_bLoad = TRUE;
	}
	else
	{
		PACKET_AGPP_SHRINEBATTLE_INFORMATION_TOTAL* pPacket2 = (PACKET_AGPP_SHRINEBATTLE_INFORMATION_TOTAL*) pPacket;

		for(int i = 0; i < AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER; i++)
		{
			if(pPacket2->m_stPointInformation[i].nShrinePointID == 0)
				break;

			IterShrinePoint iter = m_stShrineBattle.FindByShrinePointID(pPacket2->m_stPointInformation[i].nShrinePointID);
			if(!iter)
				continue;

			iter->m_nStartHour = pPacket2->m_stPointInformation[i].stShrinePoint.m_nStartHour;
			iter->m_nIntervalHour = pPacket2->m_stPointInformation[i].stShrinePoint.m_nIntervalHour;
			iter->m_nTimeLimitTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nTimeLimitTime;
			iter->m_nImprintTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nImprintTime;
			iter->m_nOccupyTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nOccupyTime;

			iter->m_nCurrentStatus = pPacket2->m_stPointInformation[i].stShrinePoint.m_nCurrentStatus;
			iter->m_nOccupationRace = pPacket2->m_stPointInformation[i].stShrinePoint.m_nOccupationRace;
			iter->m_nTryOccupyRace = pPacket2->m_stPointInformation[i].stShrinePoint.m_nTryOccupyRace;
			iter->m_nBattleOpenTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nBattleOpenTime;
			iter->m_nImprintStartTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nImprintStartTime;
			iter->m_nOccupyStartTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nOccupyStartTime;
			iter->m_nOccupyCharacterCid = pPacket2->m_stPointInformation[i].stShrinePoint.m_nOccupyCharacterCid;
			iter->m_nElapsedTime = pPacket2->m_stPointInformation[i].stShrinePoint.m_nElapsedTime;
		}
	}


	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_INFORMATION_TOTAL, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointTimeBarUpdate(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_TIMEBAR_STATE, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointStatusUpdate(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_STATUS_UPDATE, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointTryImprint(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_TRYIMPRINT, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointTryImprintResult(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_TRYIMPRINT_RESULT, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointJoinAnswer(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_JOIN_ANSWER, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointReadyToStart(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_READY_TO_START, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointMessageCode(PACKET_AGPP_SHRINEBATTLE* pPacket)
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_MESSAGE_CODE, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointPvPRecord( PACKET_AGPP_SHRINEBATTLE* pPacket )
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_PVP_RECORD, pPacket, NULL);
}

BOOL AgpmShrineBattle::OnProcessShrinePointRaceRecord( PACKET_AGPP_SHRINEBATTLE* pPacket )
{
	return EnumCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_RACE_RECORD, pPacket, NULL);
}


BOOL AgpmShrineBattle::SetCallBackShrinePointInformationRequest(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_INFORMATION_REQUEST, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointInformation(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_INFORMATION, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointInformationTotal(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_INFORMATION_TOTAL, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointTimeBarUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_TIMEBAR_STATE, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointStatusUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_STATUS_UPDATE, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointTryImprint(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_TRYIMPRINT, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointTryImprintResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_TRYIMPRINT_RESULT, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointMessageCode(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_MESSAGE_CODE, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointJoinAnswer(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_JOIN_ANSWER, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointReadyToStart(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_READY_TO_START, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointPvPRecord( ApModuleDefaultCallBack pfCallback, PVOID pClass )
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_PVP_RECORD, pfCallback, pClass);
}

BOOL AgpmShrineBattle::SetCallBackShrinePointRaceRecord( ApModuleDefaultCallBack pfCallback, PVOID pClass )
{
	return SetCallback(AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_RACE_RECORD, pfCallback, pClass);
}


BOOL AgpmShrineBattle::StreamReadShrineRegionData(CHAR* pszFile, BOOL bDecryption)
{
	using namespace profile;
	using namespace AuExcel;

	Timer t;
#define CHECK()	t.Log( __FILE__ , __LINE__ );

	if (!pszFile || !strlen(pszFile))
		return FALSE;

	AuExcelLib * pExcel = LoadExcelFile( pszFile , bDecryption );
	AuAutoPtr< AuExcelLib >	ptrExcel = pExcel;

	if( pExcel == NULL)
	{
		TRACE("AgpmShrineBattle::StreamReadShrineRegionData() Error (1) !!!\n");
		return FALSE;
	}

	m_stShrineBattle.m_vtShrineRegion.clear();

	for(INT32 lRow = 1; lRow < pExcel->GetRow();lRow++)
	{
		AgpdShrineRegion stShrineRegion;

		stShrineRegion.m_nRegionIndex = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_REGIONINDEX, lRow);
		stShrineRegion.m_bFixShrineRegion = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_FIXSHRINEREGION, lRow);

		if(stShrineRegion.m_bFixShrineRegion)
			stShrineRegion.m_nShrinePointID = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_FIXSHRINEPOINTID, lRow);

		stShrineRegion.m_stHumanBasePoint.x	= (FLOAT)pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_HUMAN_BASE_POINTX, lRow);
		stShrineRegion.m_stHumanBasePoint.z	= (FLOAT)pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_HUMAN_BASE_POINTZ, lRow);

		stShrineRegion.m_stOrcBasePoint.x	= (FLOAT)pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_ORC_BASE_POINTX, lRow);
		stShrineRegion.m_stOrcBasePoint.z	= (FLOAT)pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_ORC_BASE_POINTZ, lRow);

		stShrineRegion.m_stMoonElfBasePoint.x	= (FLOAT)pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_MOONELF_BASE_POINTX, lRow);
		stShrineRegion.m_stMoonElfBasePoint.z	= (FLOAT)pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_MOONELF_BASE_POINTZ, lRow);

		stShrineRegion.m_stScionBasePoint.x	= (FLOAT)pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_SCION_BASE_POINTX, lRow);
		stShrineRegion.m_stScionBasePoint.z	= (FLOAT)pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_SCION_BASE_POINTZ, lRow);


		stShrineRegion.m_arrayRegionIndex[0] = stShrineRegion.m_nRegionIndex;
		stShrineRegion.m_arrayRegionIndex[1] = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION1, lRow);
		stShrineRegion.m_arrayRegionIndex[2] = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION2, lRow);
		stShrineRegion.m_arrayRegionIndex[3] = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION3, lRow);
		stShrineRegion.m_arrayRegionIndex[4] = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION4, lRow);
		stShrineRegion.m_arrayRegionIndex[5] = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION5, lRow);

		for(int i = 0; i < MAX_SHRINE_HAS_REGEION_NUM; ++i)
		{
			if(m_stShrineBattle.m_lMaxRegionIndex < stShrineRegion.m_arrayRegionIndex[i])
			{
				m_stShrineBattle.m_lMaxRegionIndex = stShrineRegion.m_arrayRegionIndex[i];
			}
			if(m_stShrineBattle.m_lMinRegionIndex == -1 || (m_stShrineBattle.m_lMinRegionIndex > stShrineRegion.m_arrayRegionIndex[i]))
			{
				m_stShrineBattle.m_lMinRegionIndex = stShrineRegion.m_arrayRegionIndex[i];				
			}
		}

		m_stShrineBattle.m_vtShrineRegion.push_back(stShrineRegion);
	}

	CHECK();

	return TRUE;
}

BOOL AgpmShrineBattle::StreamReadShrinePointData(CHAR* pszFile, BOOL bDecryption)
{
	using namespace profile;
	using namespace AuExcel;

	Timer t;
	#define CHECK()	t.Log( __FILE__ , __LINE__ );

	if (!pszFile || !strlen(pszFile))
		return FALSE;

	AuExcelLib * pExcel = LoadExcelFile( pszFile , bDecryption );
	AuAutoPtr< AuExcelLib >	ptrExcel = pExcel;

	if( pExcel == NULL)
	{
		TRACE("AgpmShrineBattle::StreamReadShrinePointData() Error (1) !!!\n");
		return FALSE;
	}

	m_stShrineBattle.m_vtShrinePoint.clear();

	for(INT32 lRow = 1; lRow < pExcel->GetRow();lRow++)
	{
		AgpdShrinePoint stShrinePoint;

		stShrinePoint.m_nShrinePointID = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_POINTID, lRow);
		CHAR* pszName = pExcel->GetData(AGPMSHRINEBATTLE_EXCEL_COLUMN_POINT_STRING_NAME, lRow);
		strncpy(stShrinePoint.m_szPointName, pszName, AGPDSHRINEBATTLE_POINT_MAX_NAME_LENGTH);

		stShrinePoint.m_nStartHour = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_STANDARD_START_TIME, lRow);
		stShrinePoint.m_nIntervalHour = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_INTERVAL_TIME, lRow);
		stShrinePoint.m_nTimeLimitTime = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_BATTLETIME_LIMIT, lRow);
		stShrinePoint.m_nImprintTime = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_IMPRINTTIME_LIMIT, lRow);
		stShrinePoint.m_nOccupyTime = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_OCCUPYTIME_LIMIT, lRow);
		stShrinePoint.m_nLevelMin = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_LEVEL_MIN, lRow);	//JK_쉬라인추가
		stShrinePoint.m_nLevelMax = pExcel->GetDataToInt(AGPMSHRINEBATTLE_EXCEL_COLUMN_LEVEL_MAX, lRow);	//JK_쉬라인추가

		m_stShrineBattle.m_vtShrinePoint.push_back(stShrinePoint);
	}

	CHECK();

	return TRUE;
}

BOOL AgpmShrineBattle::StreamReadShrineMessageData(const string &pszName)
{
	AuXmlParser	parser;

	parser.LoadXMLFile(pszName);

	AuXmlNode *pRootNode = parser.FirstChild("ShrineMessage");
	if(!pRootNode)
		return FALSE;

	AuXmlNode *pRaceCode = pRootNode->FirstChild("RaceCode");
	if(!pRaceCode)
		return FALSE;

	AuXmlNode *pRace = pRaceCode->FirstChild("Race");
	if(!pRace)
		return FALSE;

	for(AuXmlNode *pNode = pRace; pNode; pNode = pNode->NextSibling())
	{
		AuXmlElement *pElemRaceTid = pNode->FirstChildElement("RaceTid");
		if(!pElemRaceTid)
			continue;

		INT32 nRaceTid = atoi(pElemRaceTid->GetText());
		if(nRaceTid <= AURACE_TYPE_NONE || nRaceTid >= AURACE_TYPE_MAX)
			return FALSE;

		AuXmlElement *pElemRaceName = pNode->FirstChildElement("RaceName");
		if(!pElemRaceName)
			return FALSE;

		strncpy(m_stShrineBattle.m_szRaceName[nRaceTid], pElemRaceName->GetText(), AGPDSHRINEBATTLE_RACE_NAME_MAX_LENGTH);
	}

	AuXmlNode *pMessage = pRootNode->FirstChild("Message");
	if(!pMessage)
		return FALSE;

	for(AuXmlNode *pNode = pMessage; pNode; pNode = pNode->NextSibling())
	{
		AuXmlElement *pElemMessageTid = pNode->FirstChildElement("MessageTid");
		if(!pElemMessageTid)
			continue;

		INT32 nMessageTid = atoi(pElemMessageTid->GetText());
		if(nMessageTid <= AGPMSHRINEBATTLE_MESSAGE_CODE_NONE || nMessageTid >= AGPMSHRINEBATTLE_MESSAGE_CODE_MAX)
			return FALSE;

		AuXmlElement *pElemMessageContent = pNode->FirstChildElement("MessageContent");
		if(!pElemMessageContent)
			return FALSE;

		strncpy(m_stShrineBattle.m_szMessage[nMessageTid], pElemMessageContent->GetText(), AGPDSHRINEBATTLE_MESSAGE_MAX_LENGTH);
	}

	return TRUE;
}

BOOL AgpmShrineBattle::ShuffleShrineRegion()
{
	for(IterShrineRegion iter = m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		if(!iter->m_bFixShrineRegion)
			iter->m_nShrinePointID = -1;
	}

	INT32 nPointIndex = 0;
	for(IterShrinePoint iter = m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
	{
		INT32 nIndex = m_csRand.randInt(m_stShrineBattle.m_vtShrineRegion.size() - nPointIndex);

		INT32 nRegionIndex = 0;

		for(IterShrineRegion iter2 = m_stShrineBattle.m_vtShrineRegion.begin(); iter2 != m_stShrineBattle.m_vtShrineRegion.end(); ++iter2)
		{
			if(iter2->m_nShrinePointID != -1)
				continue;

			if(nRegionIndex == nIndex)
			{
				iter2->m_nShrinePointID = iter->m_nShrinePointID;
				break;
			}

			nRegionIndex++;
		}

		nPointIndex++;
	}

	return TRUE;
}

BOOL AgpmShrineBattle::RandomPlaceShrineRegion(AgpdShrinePoint *pstShrinePoint)
{
	if(NULL == pstShrinePoint)
		return FALSE;
/*
	// 현재 진행되고 있는지 확인한 후
	if(pstShrinePoint->m_nCurrentStatus != AGPDSHRINEBATTLE_STATUS_NORMAL)
		return FALSE;
*/

	std::vector<INT32> TempVector;

	for(IterShrinePoint iter =m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
	{
		// Normal이지 않은 지역만 담는다.
		if(iter->m_nCurrentStatus != AGPDSHRINEBATTLE_STATUS_NORMAL)
		{
			continue;
		}

		IterShrineRegion IterRegion = m_stShrineBattle.FindRegionByShrinePointID(iter->m_nShrinePointID);
		if(IterRegion)
		{
			TempVector.push_back(IterRegion->m_nRegionIndex);
		}
	}

	if(TempVector.size() <= 0)
		return FALSE;

	INT32 nIndex = m_csRand.randInt((int)TempVector.size());

	// 현재 pstShrinePoint가 옮겨가야하는 Region
	IterShrineRegion IterRegion		= m_stShrineBattle.FindRegionByRegionIndex(TempVector[nIndex]);

	// 현재 pstShrinePoint가 있던 Region
	IterShrineRegion BackupRegion	= m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
	if(IterRegion && BackupRegion)
	{	
		AuPOS stCurrentPos;
		AuPOS stPastPos;

		AgpdShrinePoint *pcsPastShrine	= m_stShrineBattle.FindByShrinePointID(IterRegion->m_nShrinePointID);
		if(NULL == pcsPastShrine)
			return FALSE;

		AgpdCharacter *pcsPastChar		= m_pAgpmCharacter->GetCharacter(pcsPastShrine->m_nShrineObjectCID);
		AgpdCharacter *pcsCurrentChar	= m_pAgpmCharacter->GetCharacter(pstShrinePoint->m_nShrineObjectCID);
		if(!pcsPastChar || !pcsCurrentChar)
		{
			return FALSE;
		}

		// Region에 ShrineID를 바꿔준다
		INT32 lTemp						= IterRegion->m_nShrinePointID;
		IterRegion->m_nShrinePointID	= pstShrinePoint->m_nShrinePointID;
		BackupRegion->m_nShrinePointID	= lTemp;

		/*
		// Shrine의 CID도 바꿔준다.
		lTemp								= pcsPastShrine->m_nShrineObjectCID;
		pcsPastShrine->m_nShrineObjectCID	= pstShrinePoint->m_nShrineObjectCID;
		pstShrinePoint->m_nShrineObjectCID  = lTemp;
		*/

		// 바뀐 Shrine Object Character의 위치를 서로 바꿔준다.
		stPastPos						= pcsPastChar->m_stPos;
		stCurrentPos					= pcsCurrentChar->m_stPos;

		{
			AuAutoLock pLock(pcsPastChar->m_Mutex);
			if(pLock.Result() == FALSE)
				return FALSE;

			m_pAgpmCharacter->SetActionBlockTime(pcsPastChar, 3000);

			m_pAgpmCharacter->StopCharacter(pcsPastChar, NULL);
			m_pAgpmCharacter->UpdatePosition(pcsPastChar, &stCurrentPos, FALSE, TRUE);
		}

		{
			AuAutoLock pLock(pcsCurrentChar->m_Mutex);
			if(pLock.Result() == FALSE)
				return FALSE;

			m_pAgpmCharacter->SetActionBlockTime(pcsCurrentChar, 3000);

			m_pAgpmCharacter->StopCharacter(pcsCurrentChar, NULL);
			m_pAgpmCharacter->UpdatePosition(pcsCurrentChar, &stPastPos, FALSE, TRUE);
		}
	}

	return TRUE;
}

BOOL AgpmShrineBattle::IsInShrineRegion(AgpdCharacter *pcsCharacter)
{
	if(NULL == pcsCharacter)
		return FALSE;

	for(IterShrineRegion iter = m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		if(iter->m_nRegionIndex == pcsCharacter->m_nBindingRegionIndex)
			return TRUE;
	}

	return FALSE;
}

BOOL AgpmShrineBattle::IsInShrineRegionByIndex( AgpdCharacter *pcsCharacter )
{
	if(NULL == pcsCharacter)
		return FALSE;

	switch(pcsCharacter->m_nBindingRegionIndex)
	{
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING		:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS			:
	case 	REGIONINDEX_SHRINEBATTLE_WATER				:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA				:
	case 	REGIONINDEX_SHRINEBATTLE_WIND				:
	case 	REGIONINDEX_SHRINEBATTLE_POISON				:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE				:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT				:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_HUMAN	:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_ORC		:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_MOONELF	:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_SCION	:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_HUMAN		:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_ORC		:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_MOONELF	:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_SCION		:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_HUMAN		:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_SCION		:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_HUMAN			:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_SCION			:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_HUMAN			:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_SCION			:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_HUMAN		:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_SCION		:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_HUMAN			:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_SCION			:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_HUMAN		:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_SCION		:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_OUTSIDE	:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_OUTSIDE	:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_OUTSIDE		:
		return TRUE;
		break;
	}
	return FALSE;
}

BOOL AgpmShrineBattle::IsInShrineRegionByIndex( UINT32 nRegionIndex)
{

	switch(nRegionIndex)
	{
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING		:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS			:
	case 	REGIONINDEX_SHRINEBATTLE_WATER				:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA				:
	case 	REGIONINDEX_SHRINEBATTLE_WIND				:
	case 	REGIONINDEX_SHRINEBATTLE_POISON				:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE				:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT				:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_HUMAN	:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_ORC		:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_MOONELF	:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_SCION	:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_HUMAN		:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_ORC		:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_MOONELF	:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_SCION		:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_HUMAN		:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_SCION		:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_HUMAN			:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_SCION			:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_HUMAN			:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_SCION			:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_HUMAN		:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_SCION		:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_HUMAN			:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_SCION			:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_HUMAN		:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_ORC			:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_MOONELF		:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_SCION		:
	case 	REGIONINDEX_SHRINEBATTLE_LIGHTNING_OUTSIDE	:
	case 	REGIONINDEX_SHRINEBATTLE_SIRIPUS_OUTSIDE	:
	case 	REGIONINDEX_SHRINEBATTLE_WATER_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_LAVA_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_WIND_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_POISON_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_FIRE_OUTSIDE		:
	case 	REGIONINDEX_SHRINEBATTLE_SPIRIT_OUTSIDE		:
		return TRUE;
		break;
	}
	return FALSE;
}
//JK_쉬라인추가 케릭별 부활위치
UINT32 AgpmShrineBattle::FindResurrectionIndexInShrineArea(AgpdCharacter *pcsCharacter )
{
	if(!pcsCharacter) return 0;

	UINT32 nResurrectionIndex = 0;
	 
	IterShrinePoint iter = FindShrinePointAreaInvalveCharacter(pcsCharacter->m_nBindingRegionIndex);
	if (!iter) return 0;
	
	IterShrineRegion iterRegion = m_stShrineBattle.FindRegionByShrinePointID(iter->m_nShrinePointID);
	if(iterRegion)
	{
		INT32 nRace = m_pAgpmCharacter->GetCharacterRace(pcsCharacter).detail.nRace;
		switch(nRace)
		{
		case AURACE_TYPE_HUMAN:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[2];
			break;
		case AURACE_TYPE_ORC:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[3];
			break;
		case AURACE_TYPE_MOONELF:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[4];
			break;
		case AURACE_TYPE_DRAGONSCION:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[5];
			break;
		default:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[2];
			break;
		}
	}
	return nResurrectionIndex;
}
//JK_쉬라인추가 케릭별 부활위치..케릭터 접속시 바인딩 될 리전에서 부활위치를 찾는다.
UINT32 AgpmShrineBattle::FindResurrectionIndexInShrineArea(AgpdCharacter *pcsCharacter, UINT32 nBindingRegionIndex )
{
	if(!pcsCharacter) return 0;

	UINT32 nResurrectionIndex = 0;

	IterShrinePoint iter = FindShrinePointAreaInvalveCharacter(nBindingRegionIndex);
	if (!iter) return 0;

	IterShrineRegion iterRegion = m_stShrineBattle.FindRegionByShrinePointID(iter->m_nShrinePointID);
	if(iterRegion)
	{
		INT32 nRace = m_pAgpmCharacter->GetCharacterRace(pcsCharacter).detail.nRace;
		switch(nRace)
		{
		case AURACE_TYPE_HUMAN:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[2];
			break;
		case AURACE_TYPE_ORC:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[3];
			break;
		case AURACE_TYPE_MOONELF:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[4];
			break;
		case AURACE_TYPE_DRAGONSCION:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[5];
			break;
		default:
			nResurrectionIndex = iterRegion->m_arrayRegionIndex[2];
			break;
		}
	}
	return nResurrectionIndex;
}

BOOL AgpmShrineBattle::IsInShrineRegionPast(AgpdCharacter *pcsCharacter)
{
	if(NULL == pcsCharacter)
		return FALSE;

	for(IterShrineRegion iter = m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		if(iter->m_nRegionIndex == pcsCharacter->m_nBeforeRegionIndex)
			return TRUE;
	}

	return FALSE;
}

BOOL AgpmShrineBattle::IsInShrineRegionPast(UINT32 m_nPrevRegionIndex)
{
	for(IterShrineRegion iter = m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		if(iter->m_nRegionIndex == m_nPrevRegionIndex)
			return TRUE;
	}

	return FALSE;
}

IterShrinePoint AgpmShrineBattle::FindShrinePointAreaInvalveCharacter(UINT32 nRegionIndex)
{

	for(IterShrineRegion iter = m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		IterShrinePoint iter2 = m_stShrineBattle.FindByShrinePointID(iter->m_nShrinePointID);

		for(int i = 0; i < MAX_SHRINE_HAS_REGEION_NUM; ++i)
		{

			if(iter->m_arrayRegionIndex[i] == nRegionIndex)
				return iter2;
		}
	}

	return NULL;
}

BOOL AgpmShrineBattle::IsShrineBattleOpenAtThisArea(UINT32 nRegionIndex)
{

	IterShrinePoint iter =  FindShrinePointAreaInvalveCharacter(nRegionIndex);
	if(!iter) return FALSE;

	if(iter->m_nCurrentStatus >= AGPDSHRINEBATTLE_STATUS_OPEN && iter->m_nCurrentStatus < AGPDSHRINEBATTLE_STATUS_END)
		return TRUE;

	return FALSE;
}

BOOL AgpmShrineBattle::IsInTryImprintRange(AgpdShrinePoint *pcsShrinePoint, AgpdCharacter *pcsCharacter)
{
	if(NULL == pcsCharacter || NULL == pcsShrinePoint)
		return FALSE;

	AgpdCharacter *pcsShrineChar = m_pAgpmCharacter->GetCharacter(pcsShrinePoint->m_nShrineObjectCID);
	if(NULL == pcsShrineChar)
		return FALSE;

	FLOAT fDistanceX	= pcsShrineChar->m_stPos.x - pcsCharacter->m_stPos.x;
	FLOAT fDistanceZ	= pcsShrineChar->m_stPos.z - pcsCharacter->m_stPos.z;

	// 쉬라인 각인 거리범위안에 캐릭터가 있는지 여부
	if(fDistanceX * fDistanceX + fDistanceZ * fDistanceZ <= (AGPDSHRINE_IMPRINT_DISTANCE * 100) * (AGPDSHRINE_IMPRINT_DISTANCE * 100))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL AgpmShrineBattle::LoadShrineBattleData()
{
	if(!StreamReadShrinePointData("ini\\ShrineTable.txt", FALSE))
		return FALSE;

	if(!StreamReadShrineRegionData("ini\\ShrineRegion.txt", FALSE))
		return FALSE;

	AuXmlDocument XmlData;

	if(!XmlData.LoadFile("ini\\ShrineReward.xml"))
		return FALSE;

	AuXmlNode *pRootNode = XmlData.FirstChild("ShrineReward");
	if(!pRootNode)
		return FALSE;

	AuXmlNode *pCommonNode	= pRootNode->FirstChild("Common");
	AuXmlNode *pWinRaceNode = pRootNode->FirstChild("WinRace");
	AuXmlNode *pCarvingNode = pRootNode->FirstChild("Carving");
	AuXmlNode *pDungeonNode	= pRootNode->FirstChild("Dungeon");

	if(!pCommonNode || !pWinRaceNode || !pCarvingNode || !pDungeonNode)
		return FALSE;

	// Common
	AuXmlNode *pSkillNode = pCommonNode->FirstChild("Skill");
	if(pSkillNode)
	{
		for(AuXmlNode *pShrineCharNode = pSkillNode->FirstChild("ShrineCharacter"); pShrineCharNode; pShrineCharNode = pShrineCharNode->NextSibling())
		{
			if(pShrineCharNode->ToElement())
			{
				vector<INT32> TempVector;

				INT32 lShrineCharTID = atoi(pShrineCharNode->ToElement()->Attribute("TID"));

				for(AuXmlNode* pNode = pShrineCharNode->FirstChild("TID"); pNode; pNode = pNode->NextSibling())
				{
					if(pNode->ToElement())
					{
						const CHAR *pSkillTID = pNode->ToElement()->GetText();
						if(pSkillTID)
							TempVector.push_back(atoi(pSkillTID));
					}
				}	

				m_stShrineReward.m_stCommon.m_mapSkillList.insert(make_pair(lShrineCharTID, TempVector));
			}
		}
	}

	AuXmlNode *pItemNode = pCommonNode->FirstChild("Item");
	if(pItemNode)
	{
		for(AuXmlNode* pNode = pItemNode->FirstChild("TID"); pNode; pNode = pNode->NextSibling())
		{
			if(pNode->ToElement())
			{
				const CHAR *pItemTID		= pNode->ToElement()->GetText();
				const CHAR *pItemCount		= pNode->ToElement()->Attribute("Count");
				const CHAR *pItemTimeLimit	= pNode->ToElement()->Attribute("TimeLimit");

				if(pItemTID && pItemCount)
				{
					AgpdShrineReward::ItemInfo stItemInfo;
	
					if(pItemTimeLimit)
					{
						stItemInfo.m_bUseTimeLimit	= TRUE;
						stItemInfo.m_ulTimeLimit	= atoi(pItemTimeLimit);
					}

					stItemInfo.m_ulCount = atoi(pItemCount);

					m_stShrineReward.m_stCommon.m_mapItem.insert(make_pair(atoi(pItemTID), stItemInfo));
				}
			}
		}
	}

	AuXmlNode *pCharismaPointNode = pCommonNode->FirstChild("Charisma");
	if(pCharismaPointNode)
	{
		AuXmlElement *pElemPoint  = pCharismaPointNode->FirstChildElement("Point");
		if(pElemPoint)
		{
			const CHAR *pCharismaPoint = pElemPoint->GetText();
			if(pCharismaPoint)
				m_stShrineReward.m_stCommon.m_nCharismaPoint = atoi(pCharismaPoint);
		}
	}

	// WinRace
	pSkillNode = pWinRaceNode->FirstChild("Skill");
	if(pSkillNode)
	{
		for(AuXmlNode *pShrineCharNode = pSkillNode->FirstChild("ShrineCharacter"); pShrineCharNode; pShrineCharNode = pShrineCharNode->NextSibling())
		{
			if(pShrineCharNode->ToElement())
			{
				vector<INT32> TempVector;

				INT32 lShrineCharTID = atoi(pShrineCharNode->ToElement()->Attribute("TID"));

				for(AuXmlNode* pNode = pShrineCharNode->FirstChild("TID"); pNode; pNode = pNode->NextSibling())
				{
					if(pNode->ToElement())
					{
						const CHAR *pSkillTID = pNode->ToElement()->GetText();
						if(pSkillTID)
							TempVector.push_back(atoi(pSkillTID));
					}
				}	

				m_stShrineReward.m_stWinRace.m_mapSkillList.insert(make_pair(lShrineCharTID, TempVector));
			}
		}
	}

	pItemNode = pWinRaceNode->FirstChild("Item");
	if(pItemNode)
	{
		for(AuXmlNode* pNode = pItemNode->FirstChild("TID"); pNode; pNode = pNode->NextSibling())
		{
			if(pNode->ToElement())
			{
				const CHAR *pItemTID		= pNode->ToElement()->GetText();
				const CHAR *pItemCount		= pNode->ToElement()->Attribute("Count");
				const CHAR *pItemTimeLimit	= pNode->ToElement()->Attribute("TimeLimit");

				if(pItemTID && pItemCount)
				{
					AgpdShrineReward::ItemInfo stItemInfo;

					if(pItemTimeLimit)
					{
						stItemInfo.m_bUseTimeLimit	= TRUE;
						stItemInfo.m_ulTimeLimit	= atoi(pItemTimeLimit);
					}

					stItemInfo.m_ulCount = atoi(pItemCount);

					m_stShrineReward.m_stWinRace.m_mapItem.insert(make_pair(atoi(pItemTID), stItemInfo));
				}
			}
		}
	}

	pCharismaPointNode = pWinRaceNode->FirstChild("Charisma");
	if(pCharismaPointNode)
	{
		AuXmlElement *pElemPoint  = pCharismaPointNode->FirstChildElement("Point");
		if(pElemPoint)
		{
			const CHAR *pCharismaPoint = pElemPoint->GetText();
			if(pCharismaPoint)
				m_stShrineReward.m_stWinRace.m_nCharismaPoint = atoi(pCharismaPoint);
		}
	}


	// Carving
	pSkillNode = pCarvingNode->FirstChild("Skill");
	if(pSkillNode)
	{
		for(AuXmlNode *pShrineCharNode = pSkillNode->FirstChild("ShrineCharacter"); pShrineCharNode; pShrineCharNode = pShrineCharNode->NextSibling())
		{
			if(pShrineCharNode->ToElement())
			{
				vector<INT32> TempVector;

				INT32 lShrineCharTID = atoi(pShrineCharNode->ToElement()->Attribute("TID"));

				for(AuXmlNode* pNode = pShrineCharNode->FirstChild("TID"); pNode; pNode = pNode->NextSibling())
				{
					if(pNode->ToElement())
					{
						const CHAR *pSkillTID = pNode->ToElement()->GetText();
						if(pSkillTID)
							TempVector.push_back(atoi(pSkillTID));
					}
				}	

				m_stShrineReward.m_stCarvingCharacter.m_mapSkillList.insert(make_pair(lShrineCharTID, TempVector));
			}
		}
	}

	pItemNode = pCarvingNode->FirstChild("Item");
	if(pItemNode)
	{
		for(AuXmlNode* pNode = pItemNode->FirstChild("TID"); pNode; pNode = pNode->NextSibling())
		{
			if(pNode->ToElement())
			{
				const CHAR *pItemTID		= pNode->ToElement()->GetText();
				const CHAR *pItemCount		= pNode->ToElement()->Attribute("Count");
				const CHAR *pItemTimeLimit	= pNode->ToElement()->Attribute("TimeLimit");

				if(pItemTID && pItemCount)
				{
					AgpdShrineReward::ItemInfo stItemInfo;

					if(pItemTimeLimit)
					{
						stItemInfo.m_bUseTimeLimit	= TRUE;
						stItemInfo.m_ulTimeLimit	= atoi(pItemTimeLimit);
					}

					stItemInfo.m_ulCount = atoi(pItemCount);

					m_stShrineReward.m_stCarvingCharacter.m_mapItem.insert(make_pair(atoi(pItemTID), stItemInfo));
				}
			}
		}
	}

	pCharismaPointNode = pCarvingNode->FirstChild("Charisma");
	if(pCharismaPointNode)
	{
		AuXmlElement *pElemPoint  = pCharismaPointNode->FirstChildElement("Point");
		if(pElemPoint)
		{
			const CHAR *pCharismaPoint = pElemPoint->GetText();
			if(pCharismaPoint)
				m_stShrineReward.m_stCarvingCharacter.m_nCharismaPoint = atoi(pCharismaPoint);
		}
	}

	//JK_쉬라인머지 ..던전은 일단 뺀다
	/*
	// Dungeon
	for(AuXmlNode* pNode = pDungeonNode->FirstChild("Region"); pNode; pNode = pNode->NextSibling())
	{
		if(pNode->ToElement())
		{
			const CHAR *pIndex			= pNode->ToElement()->Attribute("Index");
			const CHAR *pMinLevel		= pNode->ToElement()->Attribute("MinLevel");
			const CHAR *pMaxLevel		= pNode->ToElement()->Attribute("MaxLevel");
			const CHAR *pRegionIndex	= pNode->ToElement()->GetText();

			if(pIndex && pMinLevel && pMaxLevel && pRegionIndex)
			{
				AgpdShrineDungeonInfo::DungeonRegion stDungeonInfo;

				stDungeonInfo.m_nIndex			= atoi(pIndex);
				stDungeonInfo.m_nMinLevel		= atoi(pMinLevel);
				stDungeonInfo.m_nMaxLevel		= atoi(pMaxLevel);
				stDungeonInfo.m_nRegionIndex	= atoi(pRegionIndex);

				m_pAgpmBattleGround->m_stShrineDungeonInfo.m_mapDungeonRegionInfo.insert(pair<INT32, AgpdShrineDungeonInfo::DungeonRegion>(atoi(pIndex), stDungeonInfo));
			}
		}
	}
	*/
	ShuffleShrineRegion();

	m_stShrineBattle.m_bLoad = TRUE;

	return TRUE;
}



