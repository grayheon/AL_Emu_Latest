#include "AgcmWarInfo.h"
#include "AgppWarInfo.h"

AgcmWarInfo::AgcmWarInfo()
{
	SetModuleName("AgcmWarInfo");

	m_pcsAgpmWarInfo = NULL;
	m_pcsAgcmUIWarInfo = NULL;

	m_cLastWarType = 0;
	m_cLastWarStep = 0;
	m_cLastMessageStep = 0;
	m_cLastAddInfoFlag = 0;
	m_cLastRemainTime = 0;

	m_EndTime = 0;
	m_bWarEnd = FALSE;

	m_lCharacterWinCount = 0;
	m_lCharacterLoseCount = 0;

	m_lRaceWinCount = 0;
	m_lRaceLoseCount = 0;

	EnableIdle(TRUE);

}
AgcmWarInfo::~AgcmWarInfo()
{

}

BOOL AgcmWarInfo::OnInit()
{
	return TRUE;
}
BOOL AgcmWarInfo::OnAddModule()
{
	m_pcsAgpmWarInfo = (AgpmWarInfo *)GetModule("AgpmWarInfo");
	m_pcsAgcmUIWarInfo = (AgcmUIWarInfo *)GetModule("AgcmUIWarInfo");

	if(!m_pcsAgpmWarInfo || !m_pcsAgcmUIWarInfo) return FALSE;

	m_pcsAgpmWarInfo->SetCallbackUpdateWarState(CBOperationUpdateWarState, this);

	m_pcsAgpmWarInfo->SetCallbackUpdateTimeInfo(CBOperationUpdateTimeStep, this);

	m_pcsAgpmWarInfo->SetCallbackUpdateAddInfo(CBOperationUpdateAddInfo, this);

	m_pcsAgpmWarInfo->SetCallbackUpdateCharacterPvPInfo(CBOperationUpdateCharacterPvPInfo, this);
	
	m_pcsAgpmWarInfo->SetCallbackUpdateRacePvPInfo(CBOperationUpdateRacePvPInfo, this);

	m_pcsAgpmWarInfo->SetCallbackEndWar(CBOperationEndWar, this);

	m_pcsAgpmWarInfo->SetCallbackFactor(CBOperationUpdateFactor, this);
	


	return TRUE;
}

BOOL AgcmWarInfo::OnIdle(UINT32 ulClockCount)
{
	if(m_bWarEnd == (BOOL)TRUE)
	{
		if(m_EndTime + WAR_INFO_CLOSE_TIME < AuTimeStamp::GetCurrentTimeStamp())
		{
			m_pcsAgcmUIWarInfo->SetPvPInfo(0, 0, 0, 0);
			m_pcsAgcmUIWarInfo->CloseWarInfo();
			m_bWarEnd = FALSE;
		}

	}
	return TRUE;
}
BOOL AgcmWarInfo::OnDestroy()
{
	return TRUE;
}

BOOL AgcmWarInfo::CBOperationUpdateWarState(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(pData == NULL || pClass == NULL) return FALSE;
	AgcmWarInfo *pThis = (AgcmWarInfo *)pClass;
	PACKET_WAR_INFO_STATE *pPacketWarInfo = (PACKET_WAR_INFO_STATE *)pData;

	if(pPacketWarInfo->cWarType >= WarInfoTypeMax) return FALSE;
	if(pThis->m_pcsAgcmUIWarInfo->IsWarInfoUIOpen())
	{
		if(pThis->m_cLastWarType == ShrineBattle && pPacketWarInfo->cWarType == BattleGround)
			return TRUE;
	}
	
	pThis->m_cLastWarType = pPacketWarInfo->cWarType;
	pThis->m_cLastWarStep = pPacketWarInfo->cWarStep;
	pThis->m_cLastMessageStep = pPacketWarInfo->cMessageStep;

	pThis->m_bWarEnd = FALSE;
	pThis->m_pcsAgcmUIWarInfo->OpenWarInfo();

	pThis->m_pcsAgcmUIWarInfo->SetStepMessage(pThis->m_mapWarInfoData[pPacketWarInfo->cWarType], pPacketWarInfo->cWarStep);
	pThis->m_pcsAgcmUIWarInfo->UpdateCenterMessage(pThis->m_mapWarInfoData[pPacketWarInfo->cWarType][pPacketWarInfo->cWarStep]
	,pThis->m_mapWarAddMessageData[pPacketWarInfo->cWarType], pPacketWarInfo->cMessageStep, pThis->m_cLastAddInfoFlag);
	return TRUE;
}


BOOL AgcmWarInfo::CBOperationUpdateTimeStep(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(pData == NULL || pClass == NULL) return FALSE;
	AgcmWarInfo *pThis = (AgcmWarInfo *)pClass;
	PACKET_WAR_INFO_TIME *pPacketWarInfo = (PACKET_WAR_INFO_TIME *)pData;

	if(pThis->m_pcsAgcmUIWarInfo->IsWarInfoUIOpen())
	{
		if(pThis->m_cLastWarType == ShrineBattle && pPacketWarInfo->cWarType == BattleGround)
			return TRUE;
	}

	pThis->m_cLastRemainTime = pPacketWarInfo->cRemainMin;

	pThis->m_pcsAgcmUIWarInfo->OpenWarInfo();
	pThis->m_pcsAgcmUIWarInfo->UpdateTimeStep(pThis->m_cLastRemainTime);
	return TRUE;
}

BOOL AgcmWarInfo::CBOperationUpdateAddInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(pData == NULL || pClass == NULL) return FALSE;

	AgcmWarInfo *pThis = (AgcmWarInfo *)pClass;
	PACKET_WAR_INFO_ADD *pPacketWarInfo = (PACKET_WAR_INFO_ADD *)pData;

	if(pThis->m_pcsAgcmUIWarInfo->IsWarInfoUIOpen())
	{
		if(pThis->m_cLastWarType == ShrineBattle && pPacketWarInfo->cWarType == BattleGround)
			return TRUE;
	}


	pThis->m_cLastAddInfoFlag = pPacketWarInfo->cAddInfoFlag;

	pThis->m_pcsAgcmUIWarInfo->UpdateCenterMessage(pThis->m_mapWarInfoData[pThis->m_cLastWarType][pThis->m_cLastWarStep]
	,pThis->m_mapWarAddMessageData[pThis->m_cLastWarType], pThis->m_cLastMessageStep, pThis->m_cLastAddInfoFlag, TRUE);

	return TRUE;
}


BOOL AgcmWarInfo::CBOperationUpdateCharacterPvPInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(pData == NULL || pClass == NULL) return FALSE;

	AgcmWarInfo *pThis = (AgcmWarInfo *)pClass;
	PACKET_WAR_INFO_RACE_PVP_INFO *pPacketCharacterPvPInfo = (PACKET_WAR_INFO_RACE_PVP_INFO *)pData;

	if(pThis->m_pcsAgcmUIWarInfo->IsWarInfoUIOpen())
	{
		if(pThis->m_cLastWarType == ShrineBattle && pPacketCharacterPvPInfo->cWarType == BattleGround)
			return TRUE;
	}

	pThis->m_lCharacterWinCount = pPacketCharacterPvPInfo->lWinCount;
	pThis->m_lCharacterLoseCount = pPacketCharacterPvPInfo->lLoseCount;

	pThis->UpdatePvPInfo();

	return TRUE;

}

BOOL AgcmWarInfo::CBOperationUpdateRacePvPInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(pData == NULL || pClass == NULL) return FALSE;

	AgcmWarInfo *pThis = (AgcmWarInfo *)pClass;
	PACKET_WAR_INFO_RACE_PVP_INFO *pPacketRacePvPInfo = (PACKET_WAR_INFO_RACE_PVP_INFO *)pData;

	if(pThis->m_pcsAgcmUIWarInfo->IsWarInfoUIOpen())
	{
		if(pThis->m_cLastWarType == ShrineBattle && pPacketRacePvPInfo->cWarType == BattleGround)
			return TRUE;
	}

	pThis->m_lRaceWinCount = pPacketRacePvPInfo->lWinCount;
	pThis->m_lRaceLoseCount = pPacketRacePvPInfo->lLoseCount;

	pThis->UpdatePvPInfo();

	return TRUE;

}

BOOL AgcmWarInfo::CBOperationEndWar(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(pClass == NULL) return FALSE;
	AgcmWarInfo *pThis = (AgcmWarInfo *)pClass;

	PACKET_WAR_INFO_WAR_END_INFO *pPacket = (PACKET_WAR_INFO_WAR_END_INFO *)pData;

	if(pThis->m_pcsAgcmUIWarInfo->IsWarInfoUIOpen())
	{
		if(pThis->m_cLastWarType == ShrineBattle && pPacket->cWarType == BattleGround)
			return TRUE;
	}

	pThis->EndWar();
	return TRUE;
}

BOOL AgcmWarInfo::CBOperationUpdateFactor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(pData == NULL || pClass == NULL) return FALSE;

	AgcmWarInfo *pThis = (AgcmWarInfo *)pClass;
	PACKET_WAR_INFO_FACTOR *pPacketFactor = (PACKET_WAR_INFO_FACTOR *)pData;

	if(pThis->m_pcsAgcmUIWarInfo->IsWarInfoUIOpen())
	{
		if(pThis->m_cLastWarType == ShrineBattle && pPacketFactor->cWarType == BattleGround)
			return TRUE;
	}

	pThis->m_pcsAgcmUIWarInfo->SetFactor(pPacketFactor->szFactor1, pPacketFactor->szFactor2, pPacketFactor->szFactor3);
	return TRUE;
}

BOOL AgcmWarInfo::EndWar()
{
	if(m_bWarEnd == (BOOL)TRUE)  return TRUE;
	m_bWarEnd = TRUE;

	m_cLastRemainTime = 0;

	m_cLastAddInfoFlag = 0;
	m_cLastMessageStep = 0;
	m_cLastWarStep = 0;
	m_cLastWarType = 0;

	m_lCharacterWinCount = 0;
	m_lCharacterLoseCount = 0;
	m_lRaceLoseCount = 0;
	m_lRaceWinCount = 0;


	m_EndTime = AuTimeStamp::GetCurrentTimeStamp();
	m_cLastRemainTime = 0;
	m_pcsAgcmUIWarInfo->UpdateTimeStep(0);
	return TRUE;
}

BOOL AgcmWarInfo::StartWar()
{

	m_cLastRemainTime = 0;
	m_cLastWarStep = 0;
	m_cLastMessageStep = 0;
	m_cLastAddInfoFlag = 0;

	m_bWarEnd = FALSE;

	m_lCharacterWinCount = 0;
	m_lCharacterLoseCount = 0;

	m_lRaceWinCount = 0;
	m_lRaceLoseCount = 0;
	return TRUE;
}

BOOL AgcmWarInfo::UpdatePvPInfo()
{
	m_pcsAgcmUIWarInfo->SetPvPInfo(m_lCharacterWinCount, m_lCharacterLoseCount, m_lRaceWinCount, m_lRaceLoseCount);
	return TRUE;
}