#include "AgsmWarInfo.h"
#include "AgppWarInfo.h"


AgsmWarInfo::AgsmWarInfo()
{
	SetModuleName("AgsmWarInfo");
	m_pcsAgpmWarInfo = NULL;
	m_pAgsmCharacter = NULL;
	m_pAgpmCharacter = NULL;

	m_cLastWarType = 0;
	m_cLastWarStep = 0;
	m_cLastMessageStep = 0;
	m_cLastAddInfoFlag = 0;
	m_cLastRemainTime = 0;

}

AgsmWarInfo::~AgsmWarInfo()
{

}

BOOL AgsmWarInfo::OnAddModule()
{
	m_pcsAgpmWarInfo = (AgpmWarInfo *)GetModule("AgpmWarInfo");
	m_pAgpmCharacter = (AgpmCharacter *) GetModule("AgpmCharacter");
	m_pAgsmCharacter = (AgsmCharacter *)GetModule("AgsmCharacter");

	if (!m_pAgpmCharacter || !m_pcsAgpmWarInfo || !m_pAgsmCharacter)
		return FALSE;

	m_nIndexADCharPvPData = m_pAgpmCharacter->AttachCharacterData(this, sizeof(AgpdCharacterWarPVPData), ConstructorPvPData, DestructorPvPData);



	m_pcsAgpmWarInfo->SetCallbackRequestWarInfo(CBOperationRequestWarState, this);
	return TRUE;
}

BOOL AgsmWarInfo::OnInit()
{
	return TRUE;
}


BOOL AgsmWarInfo::OnDestroy()
{
	return TRUE;
}


BOOL AgsmWarInfo::CBOperationRequestWarState(PVOID pData, PVOID pClass, PVOID pCustData)
{
	return TRUE;
}

BOOL AgsmWarInfo::SendWarInfo(INT8 cWarType, INT8 cWarStep, INT8 cMessageStep)
{
	PACKET_WAR_INFO_STATE packetWarInfoState(cWarType, cWarStep, cMessageStep);

	SendPacketAllUser(packetWarInfoState);

	m_cLastWarType = cWarType;
	m_cLastWarStep = cWarStep;
	m_cLastMessageStep = cMessageStep;

	return TRUE;
}

BOOL AgsmWarInfo::SendWarInfoToCharacter(AgpdCharacter *pcsCharacter, INT8 cWarType, INT8 cWarStep, INT8 cMessageStep)
{
	PACKET_WAR_INFO_STATE packetWarInfoState(cWarType, cWarStep, cMessageStep);

	m_cLastWarType = cWarType;
	m_cLastWarStep = cWarStep;
	m_cLastMessageStep = cMessageStep;

	ASSERT(pcsCharacter);
	UINT32 ulNID = m_pAgsmCharacter->GetCharDPNID(pcsCharacter);
	SendPacketUser(packetWarInfoState, ulNID);

	return TRUE;
}

BOOL AgsmWarInfo::SendTimeStep(INT8 cWarType, INT8 cTime)
{
	m_cLastRemainTime = cTime;
	PACKET_WAR_INFO_TIME packetTimeStep(cWarType, cTime);

	SendPacketAllUser(packetTimeStep);
	return TRUE;
}


BOOL AgsmWarInfo::SendAddInfo(INT8 cWarType, INT8 cAddInfoFlag, INT8 cUpdateNum)
{
	PACKET_WAR_INFO_ADD packetAddInfo(cWarType, cAddInfoFlag, cUpdateNum);
	SendPacketAllUser(packetAddInfo);

	m_cLastAddInfoFlag = cAddInfoFlag;
	return TRUE;
}

BOOL AgsmWarInfo::SendLastInfoForChar(AgpdCharacter *pcsCharacter)
{
	ASSERT(pcsCharacter);
	UINT32 ulNID = m_pAgsmCharacter->GetCharDPNID(pcsCharacter);

	PACKET_WAR_INFO_STATE packetWarInfoState(m_cLastWarType, m_cLastWarStep, m_cLastMessageStep);
	SendPacketUser(packetWarInfoState, ulNID);

	PACKET_WAR_INFO_ADD packetAddInfo(m_cLastWarType, m_cLastAddInfoFlag, 1);
	SendPacketUser(packetAddInfo, ulNID);

	PACKET_WAR_INFO_TIME packetTimeStep(m_cLastWarType, m_cLastRemainTime);
	SendPacketUser(packetTimeStep, ulNID);


	AgpdCharacterWarPVPData *pcsADCharPvPData	= GetCharacterData(pcsCharacter);
	PACKET_WAR_INFO_CHARACTER_PVP_INFO packetCharacterPvPInfo(m_cLastWarType, pcsADCharPvPData->GetWinCount(), pcsADCharPvPData->GetLoseCount());
	SendPacketUser(packetCharacterPvPInfo, ulNID);


	AuRace stRace  = m_pAgpmCharacter->GetCharacterRace(pcsCharacter);
	PACKET_WAR_INFO_RACE_PVP_INFO packetRacePvPInfo(m_cLastWarType, m_arRaceScore[stRace.detail.nRace].m_lWinScore, m_arRaceScore[stRace.detail.nRace].m_lLoseScore);
	SendPacketUser(packetRacePvPInfo, ulNID);

	return TRUE;
}

BOOL AgsmWarInfo::StartWar(INT8 cWarType)
{
	m_cLastWarType = cWarType;
	m_cLastWarStep = 0;
	m_cLastMessageStep = 0;
	m_cLastAddInfoFlag = 0;
	m_cLastRemainTime = 0;
	InitRaceScore();
	InitADCharacterPvPInfo();
	return TRUE;

}

BOOL AgsmWarInfo::ConstructorPvPData(PVOID pData, PVOID pClass, PVOID pCustData)
{
	return TRUE;
}

BOOL AgsmWarInfo::DestructorPvPData(PVOID pData, PVOID pClass, PVOID pCustData)
{
	return TRUE;
}

AgpdCharacterWarPVPData* AgsmWarInfo::GetCharacterData(AgpdCharacter *pcsCharacter)
{
	// Module Data 가져오기
	return (AgpdCharacterWarPVPData *) m_pAgpmCharacter->GetAttachedModuleData(m_nIndexADCharPvPData, (void *) pcsCharacter);
}


BOOL AgsmWarInfo::ProcessPvPResult(INT8 cWarType, AgpdCharacter* pcsWinCharacter, AgpdCharacter *pcsLoseCharacter )
{
	
	if (NULL == pcsLoseCharacter || NULL == pcsWinCharacter)
		return FALSE;
	
	if (pcsWinCharacter->m_eType != APBASE_TYPE_CHARACTER)
		return FALSE;

	if (!m_pAgpmCharacter->IsPC(pcsLoseCharacter) || !m_pAgpmCharacter->IsPC(pcsWinCharacter))
		return FALSE;
	

	AgpdCharacterWarPVPData *pcsADCharWin  = GetCharacterData(pcsWinCharacter);
	AgpdCharacterWarPVPData *pcsADCharLose	= GetCharacterData(pcsLoseCharacter);

	if (NULL == pcsADCharLose || NULL == pcsADCharWin)
		return FALSE;

	// 개인의 승패를 기록해 준다.
	pcsADCharWin->AddWinCount();
	pcsADCharLose->AddLoseCount();

	// 종족의 승패를 기록해 준다.
	AuRace stWinnerRace = m_pAgpmCharacter->GetCharacterRace(pcsWinCharacter);
	AuRace stLoserRace  = m_pAgpmCharacter->GetCharacterRace(pcsLoseCharacter);

	IncreaseRaceWinScore((AuRaceType)stWinnerRace.detail.nRace);
	IncreaseRaceLoseScore((AuRaceType)stLoserRace.detail.nRace);


	PACKET_WAR_INFO_CHARACTER_PVP_INFO stWinnerPacket(cWarType, pcsADCharWin->GetWinCount(), pcsADCharWin->GetLoseCount());
	PACKET_WAR_INFO_CHARACTER_PVP_INFO stLoserPacket(cWarType, pcsADCharLose->GetWinCount(), pcsADCharLose->GetLoseCount());


	SendPacketUser(stWinnerPacket, m_pAgsmCharacter->GetCharDPNID(pcsWinCharacter));
	SendPacketUser(stLoserPacket, m_pAgsmCharacter->GetCharDPNID(pcsLoseCharacter));


	return TRUE;

}


BOOL AgsmWarInfo::InitRaceScore()
{
	for(int i = 0; i < AURACE_TYPE_MAX; ++i)
	{
		m_arRaceScore[i].m_lLoseScore = 0;
		m_arRaceScore[i].m_lWinScore = 0;
	}
	return TRUE;
}

BOOL AgsmWarInfo::IncreaseRaceWinScore(AuRaceType eRaceType)
{
	m_arRaceScore[eRaceType].IncreaseWinScore();
	return TRUE;
}

BOOL AgsmWarInfo::IncreaseRaceLoseScore(AuRaceType eRaceType)
{
	m_arRaceScore[eRaceType].IncreaseLoseScore();
	return TRUE;
}



BOOL AgsmWarInfo::SendRacePvPInfoToAll(INT8 cWarType)
{
	AgpdCharacter* pcsTarget = NULL;
	INT32 nIndex = 0;
	while(pcsTarget = m_pAgpmCharacter->GetCharSequence(&nIndex))
	{
		INT32 lWinScore		= 0;
		INT32 lLoseScore	= 0;

		DPNID dwNID = m_pAgsmCharacter->GetCharDPNID(pcsTarget);

		if(m_pAgpmCharacter->IsPC(pcsTarget) && dwNID != 0)
		{
			INT16 nRace = m_pAgpmCharacter->GetCharacterRace(pcsTarget).detail.nRace;
			if(nRace <= 0 || nRace >= AURACE_TYPE_MAX) continue;

			lWinScore  = m_arRaceScore[nRace].m_lWinScore;
			lLoseScore = m_arRaceScore[nRace].m_lLoseScore;

			PACKET_WAR_INFO_RACE_PVP_INFO stPacket(cWarType, lWinScore, lLoseScore);
			AgsEngine::GetInstance()->SendPacket(stPacket, dwNID);
		}		
	}

	return TRUE;
}

BOOL AgsmWarInfo::InitADCharacterPvPInfo()
{
	AgpdCharacter	*pcsCharacter = NULL;
	INT32			lIndex = 0;

	for (pcsCharacter = m_pAgpmCharacter->GetCharSequence(&lIndex); pcsCharacter; pcsCharacter = m_pAgpmCharacter->GetCharSequence(&lIndex))
	{
		AgpdCharacterWarPVPData *pcsADChar = GetCharacterData(pcsCharacter);
		if (pcsADChar)
		{
			pcsADChar->Init();
		}
	}
	return TRUE;
}


BOOL AgsmWarInfo::SendEndWarInfo(INT8 cWarType)
{
	PACKET_WAR_INFO_WAR_END_INFO packetEndWar(cWarType);
	SendPacketAllUser(packetEndWar);
	return TRUE;
}

BOOL AgsmWarInfo::SendEndWarInfoToCharacter(INT8 cWarType, AgpdCharacter *pcsCharacter)
{
	ASSERT(pcsCharacter);
	UINT32 ulNID = m_pAgsmCharacter->GetCharDPNID(pcsCharacter);
	PACKET_WAR_INFO_WAR_END_INFO packetEndWar(cWarType);
	AgsEngine::GetInstance()->SendPacket(packetEndWar, ulNID);

	return TRUE;
}


BOOL AgsmWarInfo::SendTimeStepToCharacter(INT8 cWarType, AgpdCharacter *pcsCharacter, INT8 cTime)
{
	ASSERT(pcsCharacter);
	UINT32 ulNID = m_pAgsmCharacter->GetCharDPNID(pcsCharacter);
	m_cLastRemainTime = cTime;
	PACKET_WAR_INFO_TIME packetTimeStep(cWarType, cTime);
	AgsEngine::GetInstance()->SendPacket(packetTimeStep, ulNID);
	return TRUE;
}
