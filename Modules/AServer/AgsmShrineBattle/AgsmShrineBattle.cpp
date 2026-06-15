#include "AgsmShrineBattle.h"
#include "AgpmCharacter.h"
#include "AgsmCharacter.h"
#include "AgpmSkill.h"
#include "AgsmSkill.h"
#include "AgsmAreaChatting.h"
#include "ApmMap.h"
#include "ApmEventManager.h"
#include "AgsmDeath.h"
#include "AgpmEventSpawn.h"
#include "AgpdEventSpawn.h"
#include "AgsmEventSpawn.h"
#include "AgsmSystemMessage.h"
#include "AgpmItem.h"
#include "AgsmItem.h"
#include "AgsmItemManager.h"
#include "AgpmBattleGround.h"
#include "AgsmBattleGround.h"
#include "AgsmPvP.h"
#include "MagDebug.h"
#include "AgppWarInfo.h"

AgsmShrineBattle::AgsmShrineBattle()
{
	SetModuleName("AgsmShrineBattle");
	SetModuleType(APMODULE_TYPE_SERVER);

	m_pAgpmShrineBattle = NULL;
	m_pAgpmCharacter = NULL;
	m_pAgsmCharacter = NULL;
	m_pAgpmSkill = NULL;
	m_pAgsmSkill = NULL;
	m_pAgsmAreaChatting = NULL;
	m_pApmMap = NULL;
	m_pAgpmFactors = NULL;
	m_pAgpmEventManager = NULL;
	m_pAgsmDeath = NULL;
	m_pAgpmEventSpawn = NULL;
	m_pAgsmEventSpawn = NULL;
	m_pAgsmSystemMessage = NULL;
	m_pAgpmItem	= NULL;
	m_pAgsmItem = NULL;
	m_pAgsmItemManager = NULL;
	m_pAgpmBattleGround = NULL;
	m_pAgsmBattleGround = NULL;
	m_pAgsmPvP = NULL;
	m_pAgsmWarInfo = NULL;
	m_pAgpmEventBinding = NULL;

	m_bLoadComplete = FALSE;
	m_bAllowMove = FALSE;

	m_ulCheckClockCount = 0;
	m_ulPreCheckClockCount = 0;
	m_ulLastRacePvPSendTime = 0;

	EnableIdle(TRUE);
}

AgsmShrineBattle::~AgsmShrineBattle()
{

}

BOOL AgsmShrineBattle::OnAddModule()
{
	return TRUE;
}

BOOL AgsmShrineBattle::OnInit()
{
	m_pAgpmShrineBattle = (AgpmShrineBattle *) GetModule("AgpmShrineBattle");
	m_pAgpmCharacter = (AgpmCharacter *) GetModule("AgpmCharacter");
	m_pAgsmCharacter = (AgsmCharacter *) GetModule("AgsmCharacter");
	m_pAgpmSkill = (AgpmSkill *) GetModule("AgpmSkill");
	m_pAgsmSkill = (AgsmSkill *) GetModule("AgsmSkill");
	m_pAgsmAreaChatting = (AgsmAreaChatting *) GetModule("AgsmAreaChatting");
	m_pApmMap = (ApmMap *) GetModule("ApmMap");
	m_pAgpmFactors = (AgpmFactors *) GetModule("AgpmFactors");
	m_pAgpmEventManager = (ApmEventManager *) GetModule("ApmEventManager");
	m_pAgsmDeath = (AgsmDeath *) GetModule("AgsmDeath");
	m_pAgpmEventSpawn = (AgpmEventSpawn *) GetModule("AgpmEventSpawn");
	m_pAgsmEventSpawn = (AgsmEventSpawn *) GetModule("AgsmEventSpawn");
	m_pAgsmSystemMessage = (AgsmSystemMessage *) GetModule("AgsmSystemMessage");
	m_pAgpmItem = (AgpmItem *)GetModule("AgpmItem");
	m_pAgsmItem = (AgsmItem *)GetModule("AgsmItem");
	m_pAgsmItemManager = (AgsmItemManager *) GetModule("AgsmItemManager");
	m_pAgpmBattleGround = (AgpmBattleGround *) GetModule("AgpmBattleGround");
	m_pAgsmBattleGround = (AgsmBattleGround *) GetModule("AgsmBattleGround");
	m_pAgsmPvP = (AgsmPvP *) GetModule("AgsmPvP");
	m_pAgpmEventBinding = (AgpmEventBinding *)GetModule("AgpmEventBinding");
	m_pAgpmEventTeleport = (AgpmEventTeleport *)GetModule("AgpmEventTeleport");

	m_pAgsmWarInfo = (AgsmWarInfo *)GetModule("AgsmWarInfo");
	m_pAgpmPvP = (AgpmPvP *)GetModule("AgpmPvP");

	if(!m_pAgpmShrineBattle || !m_pAgpmCharacter || !m_pAgsmCharacter || !m_pAgpmSkill || !m_pAgsmSkill
		|| !m_pAgsmAreaChatting || !m_pApmMap || !m_pAgpmFactors || !m_pAgpmEventManager || !m_pAgpmEventSpawn || !m_pAgsmEventSpawn ||
		!m_pAgsmSystemMessage || !m_pAgpmItem || !m_pAgsmItem || !m_pAgsmItemManager || !m_pAgpmBattleGround || !m_pAgsmBattleGround || !m_pAgsmPvP
		|| !m_pAgsmWarInfo || !m_pAgpmEventBinding || !m_pAgpmEventTeleport || !m_pAgpmPvP)
		return FALSE;

	if (!m_pAgpmCharacter->SetCallbackAddCharacterToMap(CBAddCharacterToMap, this))
		return FALSE;

	if(!m_pAgpmCharacter->SetCallbackBindingRegionChange(CBRegionChange, this))
		return FALSE;

	if(!m_pAgpmShrineBattle->SetCallBackShrinePointInformationRequest(CBShrineBattleInformationRequest, this))
		return FALSE;

	if(!m_pAgpmShrineBattle->SetCallBackShrinePointTryImprint(CBShrineBattleTryImprint, this))
		return FALSE;

	if(!m_pAgpmShrineBattle->SetCallBackShrinePointJoinAnswer(CBShrineBattleJoinAnswer, this))
		return FALSE;

	if(!m_pAgsmDeath->SetCallbackCharacterDead(CBDeath, this))
		return FALSE;

	if(!m_pAgpmCharacter->SetCallbackMoveChar(CBMoveCharacter, this))
		return FALSE;

	if (!m_pAgsmEventSpawn->SetCallbackSpawn(CBSpawnCharacter, this))
		return FALSE;

	if (!m_pAgpmCharacter->SetCallbackRemoveChar(CBRemoveCharacter, this))
		return FALSE;

	if(!m_pAgsmCharacter->SetCallbackEnterGameworld(CBEnterGameWorld, this))
		return FALSE;

	if (!m_pAgpmEventTeleport->SetCallbackSyncTeleportLoading(CBTeleportLoading, this))
		return FALSE;


	return TRUE;
}

BOOL AgsmShrineBattle::OnDestroy()
{
	return TRUE;
}

BOOL AgsmShrineBattle::OnIdle(UINT32 ulClockCount)
{
	if(m_ulCheckClockCount + 1000 > ulClockCount)
		return TRUE;

	if(m_bLoadComplete == FALSE)
		return TRUE;

	ProcessShrineBattle(ulClockCount);
	CheckRacePvPSend(ulClockCount);

	m_ulCheckClockCount = ulClockCount;

	return TRUE;
}

BOOL AgsmShrineBattle::ProcessShrineBattle(UINT32 ulClockCount)
{
	if(!m_pAgpmShrineBattle)
		return TRUE;

	if(!m_pAgpmShrineBattle->m_stShrineBattle.m_bLoad)
		return TRUE;

	IterShrinePoint iter;

	BOOL bAllEnd = TRUE;

	SendRemainTimeCheck(ulClockCount);

	for(iter =m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
	{
		CloseCheck(iter, ulClockCount);

		switch(iter->m_nCurrentStatus)
		{
		case AGPDSHRINEBATTLE_STATUS_OPEN:
			{
				OnStatusOpen(iter, ulClockCount);
			}
			break;
		case AGPDSHRINEBATTLE_STATUS_TRYIMPRINT:
			{
				OnStatusTryImprint(iter, ulClockCount);
			}
			break;
		case AGPDSHRINEBATTLE_STATUS_IMPRINT:
			{
				OnStatusImprint(iter, ulClockCount);
			}
			break;
		case AGPDSHRINEBATTLE_STATUS_TRYOCCUPY:
			{
				OnStatusTryOccupy(iter, ulClockCount);
			}
			break;
		case AGPDSHRINEBATTLE_STATUS_OCCUPY:
			{
				OnStatusOccupy(iter, ulClockCount);
			}
			break;
		case AGPDSHRINEBATTLE_STATUS_END:
			{
				OnStatusEnd(iter, ulClockCount);
			}
			break;

		case AGPDSHRINEBATTLE_STATUS_NORMAL:
			{
				OnStatusNormal(iter, ulClockCount);
			}
			break;

		default:
			break;
		}
	}

	/*
	if(bAllEnd)
	{
		iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin();

		CTime cur = CTime::GetCurrentTime();

		if(iter->m_nIntervalHour != 0 && cur.GetMinute() == 0 && (cur.GetHour() - iter->m_nStartHour)%iter->m_nIntervalHour == 0)
			OpenAllShrine();
	}
	*/
	
	return TRUE;
}

BOOL AgsmShrineBattle::SendRemainTimeCheck(UINT32 ulClockCount)
{
	IterShrineRegion iterRegion;
	IterShrinePoint iterPoint;

	for(iterPoint =m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iterPoint != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iterPoint)
	{
		if(iterPoint->m_nCurrentStatus >= AGPDSHRINEBATTLE_STATUS_OPEN && iterPoint->m_nCurrentStatus < AGPDSHRINEBATTLE_STATUS_END)
		{
			iterRegion = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(iterPoint->m_nShrinePointID);
			if(iterRegion)
			{
				if(iterRegion->m_ulLastSendTimeRemainTime + 1000 * 60 > ulClockCount) continue;

				iterRegion->m_ulLastSendTimeRemainTime = ulClockCount;

				UINT32 nTime = (UINT32)(iterPoint->m_nRemainTime * 0.001f);
				nTime /= 60;
				++nTime;

				if(nTime > iterPoint->m_nTimeLimitTime/(1000*60)) nTime = iterPoint->m_nTimeLimitTime/(1000*60);
				else if(nTime < 0) nTime = 0;

				SendPacketRemainTime(iterPoint, nTime);		
			}
		}
	}
	return TRUE;
}

BOOL AgsmShrineBattle::CloseCheck(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount)
{
	// status와 상관없이 제한시간을 넘기면 쉬라인전투를 닫기게 한다.
	if(!pstShrinePoint)
		return FALSE;

	CTime stCur = CTime::GetCurrentTime();

	// Normal 상태가 아니라면 RemainTime을 Setting해준다.
	if(pstShrinePoint->m_nCurrentStatus != AGPDSHRINEBATTLE_STATUS_NORMAL)
	{
		pstShrinePoint->m_nRemainTime = (pstShrinePoint->m_nBattleOpenTime + pstShrinePoint->m_nTimeLimitTime) - ulClockCount;

		//AgsEngine::GetInstance()->AddTimer(30000, 0, this, ShrineScheduler, NULL);
	}


	// Normal 상태가 아닌 상태(즉 쉬라인이 열리고 있는 상태)에서 쉬라인 TimeLimit가 지나면 쉬라인을 닫는다.
	if(pstShrinePoint->m_nCurrentStatus >= AGPDSHRINEBATTLE_STATUS_OPEN && pstShrinePoint->m_nCurrentStatus < AGPDSHRINEBATTLE_STATUS_END)
	{
	//	if( (((pstShrinePoint->m_nStartHour + (pstShrinePoint->m_nTimeLimitTime / (60 * 60 * 1000))) % 23) <= stCur.GetHour())
	//		&& ( (pstShrinePoint->m_nTimeLimitTime / (60 * 1000)) <= stCur.GetMinute()) )
		if(pstShrinePoint->m_nBattleOpenTime+ pstShrinePoint->m_nTimeLimitTime < ulClockCount)
		{
			StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_END, ulClockCount);

			EndShrineDungeon();

			SendPacketRemainTime(pstShrinePoint, 0);
			SendEndWar(pstShrinePoint);

			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "ShrinePoint Battle Draw : ID = %s, Hour = %d", pstShrinePoint->m_szPointName, stCur.GetHour());
			AuLogFile_s("LOG\\ShrineOpenStatic.log", strCharBuff);

			return SendPacketShrineBattleInformationToRegion(pstShrinePoint);
		}
	}

	// END 상태일때 쉬라인 인터벌 시간이 끝나면 Normal 상태로 바꿔준다.
	if(pstShrinePoint->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_END && 
	   ((pstShrinePoint->m_nStartHour + pstShrinePoint->m_nIntervalHour) % 23) == stCur.GetHour())
	{
		StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_NORMAL, ulClockCount);

		char strCharBuff[256] = { 0, };
		sprintf_s(strCharBuff, sizeof(strCharBuff), "ShrinePoint Change to Normal : ID = %s, Hour = %d", pstShrinePoint->m_szPointName, stCur.GetHour());
		AuLogFile_s("LOG\\ShrineOpenStatic.log", strCharBuff);

		return SendPacketShrineBattleInformationToRegion(pstShrinePoint);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::OnStatusOpen(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount)
{
	if(!pstShrinePoint)
		return FALSE;

	return SendPacketShrineBattleInformationToRegion(pstShrinePoint);
}

BOOL AgsmShrineBattle::OnStatusTryImprint(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount)
{
	// 시간을 체크해서 status를 Imprint로 바꿔준다.
	if(!pstShrinePoint)
		return FALSE;

	AgpdCharacter *pcsCharacter = m_pAgpmCharacter->GetCharacter(pstShrinePoint->m_nOccupyCharacterCid);
	if(!pcsCharacter)
		return FALSE;

	if(pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
	{
		INT32 nPrevStatus = pstShrinePoint->m_nCurrentStatus;

		SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, nPrevStatus, FALSE);

		// 실패라고 말해준다.
		SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT_FAIL, pstShrinePoint->m_nShrinePointID, pstShrinePoint->m_nTryOccupyRace, pstShrinePoint->m_nOccupyCharacterCid);

		StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_OPEN, pstShrinePoint->m_nBattleOpenTime, FALSE);

		pstShrinePoint->m_nOccupyCharacterCid = AP_INVALID_CID;
		ZeroMemory(pstShrinePoint->m_szOccupyCharacterName, sizeof(pstShrinePoint->m_szOccupyCharacterName));

		return FALSE;
	}


	pstShrinePoint->m_nElapsedTime = ulClockCount - pstShrinePoint->m_nImprintStartTime;

	if(pstShrinePoint->m_nImprintStartTime + pstShrinePoint->m_nImprintTime < ulClockCount)
	{
		StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_IMPRINT, ulClockCount);
	}

	SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, AGPDSHRINEBATTLE_STATUS_TRYIMPRINT, TRUE);

	return SendPacketShrineBattleInformationToRegion(pstShrinePoint);
}

BOOL AgsmShrineBattle::OnStatusImprint(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount)
{
	// status를 TryOccupy로 바꿔준다.
	if(!pstShrinePoint)
		return FALSE;

	AgpdCharacter *pcsCharacter = m_pAgpmCharacter->GetCharacter(pstShrinePoint->m_nOccupyCharacterCid);
	if(!pcsCharacter)
		return FALSE;

	if(pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
	{
		INT32 nPrevStatus = pstShrinePoint->m_nCurrentStatus;

		SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, nPrevStatus, FALSE);

		// 실패라고 말해준다.
		SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT_FAIL, pstShrinePoint->m_nShrinePointID, pstShrinePoint->m_nTryOccupyRace, pstShrinePoint->m_nOccupyCharacterCid);

		StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_OPEN, pstShrinePoint->m_nBattleOpenTime, FALSE);

		pstShrinePoint->m_nOccupyCharacterCid = AP_INVALID_CID;
		ZeroMemory(pstShrinePoint->m_szOccupyCharacterName, sizeof(pstShrinePoint->m_szOccupyCharacterName));

		return FALSE;
	}

	IterShrineRegion iterRegion = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
	if(!iterRegion)
		return FALSE;

	StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_TRYOCCUPY, ulClockCount);

	SendPacketShrineTimebarStatusToRegion(pstShrinePoint, TRUE, iterRegion->m_nRegionIndex);

	SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, AGPDSHRINEBATTLE_STATUS_IMPRINT, TRUE);

	return SendPacketShrineBattleInformationToRegion(pstShrinePoint);
}

BOOL AgsmShrineBattle::OnStatusTryOccupy(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount)
{
	// 시간을 체크해서 status를 Occupy로 바꿔준다.
	if(!pstShrinePoint)
		return FALSE;

	AgpdCharacter *pcsCharacter = m_pAgpmCharacter->GetCharacter(pstShrinePoint->m_nOccupyCharacterCid);
	if(!pcsCharacter)
		return FALSE;

	if(pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
	{
		INT32 nPrevStatus = pstShrinePoint->m_nCurrentStatus;

		SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, nPrevStatus, FALSE);

		// 실패라고 말해준다.
		SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_OCCUPY_FAIL, pstShrinePoint->m_nShrinePointID, pstShrinePoint->m_nTryOccupyRace, pstShrinePoint->m_nOccupyCharacterCid);

		StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_OPEN, pstShrinePoint->m_nBattleOpenTime, FALSE);

		pstShrinePoint->m_nOccupyCharacterCid = AP_INVALID_CID;
		ZeroMemory(pstShrinePoint->m_szOccupyCharacterName, sizeof(pstShrinePoint->m_szOccupyCharacterName));

		return FALSE;
	}


	IterShrineRegion iterRegion = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
	if(!iterRegion)
		return FALSE;

	pstShrinePoint->m_nElapsedTime = ulClockCount - pstShrinePoint->m_nOccupyStartTime;

	INT32 nRace = m_pAgpmCharacter->GetCharacterRace(pcsCharacter).detail.nRace;
	pstShrinePoint->m_nTryOccupyRace = nRace;

	if(pstShrinePoint->m_nOccupyStartTime + pstShrinePoint->m_nOccupyTime < ulClockCount)
	{
		StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_OCCUPY, ulClockCount);

		SendPacketShrineTimebarStatusToRegion(pstShrinePoint, FALSE, iterRegion->m_nRegionIndex);

		SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, AGPDSHRINEBATTLE_STATUS_TRYOCCUPY, TRUE);

	}

	return SendPacketShrineBattleInformationToRegion(pstShrinePoint);
}

BOOL AgsmShrineBattle::OnStatusOccupy(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount)
{
	// status를 End로 바꿔준다.
	// 쉬라인 점령 대가를 지불한다.
	if(!pstShrinePoint)
		return FALSE;

	AgpdCharacter *pcsCharacter = m_pAgpmCharacter->GetCharacter(pstShrinePoint->m_nOccupyCharacterCid);
	if(!pcsCharacter)
		return FALSE;

	ShrineBattleRewardProcess(pstShrinePoint);
	StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_END, ulClockCount);

	SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, AGPDSHRINEBATTLE_STATUS_OCCUPY, TRUE);

	return SendPacketShrineBattleInformationToRegion(pstShrinePoint);
}

BOOL AgsmShrineBattle::OnStatusEnd(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount)
{
	if(!pstShrinePoint)
		return FALSE;

	

	if(!pstShrinePoint->m_bTeleportToTown)
	{
		AgsEngine::GetInstance()->AddTimer(60000, pstShrinePoint->m_nShrinePointID, this, PartyIsOverGoHome, pstShrinePoint);
		pstShrinePoint->m_bTeleportToTown = TRUE;

		IterShrineRegion iterRegion = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
		if(iterRegion)
		{
			iterRegion->m_nHumanPlayerCount   = 0;
			iterRegion->m_nOrcPlayerCount     = 0;
			iterRegion->m_nMoonElfPlayerCount = 0;
			iterRegion->m_nScionPlayerCount   = 0;
		}

		CTime stCur = CTime::GetCurrentTime();

		char strCharBuff[256] = { 0, };
		sprintf_s(strCharBuff, sizeof(strCharBuff), "ShrinePoint End : ID = %s, EndHour = %d", pstShrinePoint->m_szPointName, stCur.GetHour());
		AuLogFile_s("LOG\\ShrineOpenStatic.log", strCharBuff);
	}
	
	SendPacketShrineBattleInformationToRegion(pstShrinePoint);

	return TRUE;
}

BOOL AgsmShrineBattle::OnStatusNormal(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount)
{
	if(NULL == pstShrinePoint)
		return FALSE;

	CTime stCur = CTime::GetCurrentTime();

	CTime stPreCheck = stCur + CTimeSpan(0, 0, AGSM_SHRINE_READY_TO_STARTTIME, 0);

	// 사전 공지 타임 (이때 쉬라인 판을 섞어놓는다.)
	if(IsShrineBattleOpenDay() && stPreCheck.GetHour() == pstShrinePoint->m_nStartHour && stPreCheck.GetMinute() <= AGSM_SHRINE_READY_TO_STARTTIME)
	{
		if(m_ulPreCheckClockCount + 60000 < ulClockCount || m_ulPreCheckClockCount == ulClockCount)// 동일 시간에 오픈되는 경우 때문에 같은 ulClockCount의 경우 처리해준다.
		{
			INT32 lRestMinute = AGSM_SHRINE_READY_TO_STARTTIME - stPreCheck.GetMinute();

			m_ulPreCheckClockCount = ulClockCount;

			// 쉬라인으로 이동을 허가하는 시간 (AGSM_SHRINE_READY_TO_STARTTIME) 간 전에 쉬라인을 준비해놓는다.
			if (m_bAllowMove == FALSE)
			{
				// Shrine Object를 Random하게 배치한다.
				if(FALSE == m_pAgpmShrineBattle->RandomPlaceShrineRegion(pstShrinePoint))
					return FALSE;

				m_bAllowMove = TRUE;
			}

			SendShrineWarInfo(pstShrinePoint,0,0);
			
					

			SendPacketShrineBattleFullInformationToWorld();
			SendPacketShrineBattleReadyToStartToWorld(lRestMinute, pstShrinePoint);
		}
	}

	// Normal인 상태에서 시간이 되면 Open해준다.
	if( pstShrinePoint->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_NORMAL && IsShrineBattleOpenDay() &&
	   (pstShrinePoint->m_nStartHour % 24) == stCur.GetHour())
	{
		BOOL bOpenResult = FALSE;

		bOpenResult = OpenShrine(pstShrinePoint);
			
		char strCharBuff[256] = { 0, };
		sprintf_s(strCharBuff, sizeof(strCharBuff), "ShrinePoint Open : ID = %s, OpenHour = %d, Result = %d", pstShrinePoint->m_szPointName, stCur.GetHour(), bOpenResult);
		AuLogFile_s("LOG\\ShrineOpenStatic.log", strCharBuff);
	}

	SendPacketShrineBattleInformationToRegion(pstShrinePoint);

	return TRUE;

}

BOOL AgsmShrineBattle::IsShrineBattleOpenDay()
{
	/*
	CTime stCur = CTime::GetCurrentTime();

	if (stCur.GetDayOfWeek() == 1 ||
		stCur.GetDayOfWeek() == 4 ||
		stCur.GetDayOfWeek() == 7)
		return FALSE;
	*/
	return TRUE;

}

AuXmlNode* AgsmShrineBattle::GetRootNode()
{
	static AuXmlNode *m_pNoticeNode = NULL;
	
	if(NULL == m_pNoticeNode)
		m_pNoticeNode = m_pAgsmSystemMessage->GetRootNode()->FirstChildElement("Shrine");

	return m_pNoticeNode;
}

BOOL AgsmShrineBattle::OpenAllShrine()
{
	m_pAgpmShrineBattle->ShuffleShrineRegion();

	ShuffleShrineObjectSetting();

	SendPacketShrineBattleFullInformationToWorld();

	//SendPacketShrineBattleMessageCodeToWorld(AGPMSHRINEBATTLE_MESSAGE_CODE_ALL_OPEN, 0, 0, 0);

	for(IterShrinePoint iter =m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
		StatusChange(iter, AGPDSHRINEBATTLE_STATUS_OPEN, GetClockCount());

	return TRUE;
}

BOOL AgsmShrineBattle::OpenShrine(AgpdShrinePoint *pcsShrinePoint)
{
	if(NULL == pcsShrinePoint)
		return FALSE;

	StatusChange(pcsShrinePoint, AGPDSHRINEBATTLE_STATUS_OPEN, GetClockCount());


	SendPacketShrineBattleFullInformationToWorld();

	IterShrineRegion iterShrineRegion = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pcsShrinePoint->m_nShrinePointID);
	if(iterShrineRegion)
	{
		iterShrineRegion->m_ulLastSendTimeRemainTime = 0;
		iterShrineRegion->InitRacePvPScore();
	}

	//SendPacketShrineBattleMessageCodeToWorld(AGPMSHRINEBATTLE_MESSAGE_CODE_OPEN, pcsShrinePoint->m_nShrinePointID, AURACE_TYPE_NONE, AP_INVALID_CID);

	// 시작과 동시에 ShrineDungeon도 오픈해준다.
	EndShrineDungeon();

	if(NULL == AgsEngine::GetInstance()->AddTimer(60000, pcsShrinePoint->m_nShrinePointID, this, OpenShrineDungeonTimer, NULL))
	{
		OpenShrineDungeon();
	}

	m_bAllowMove = FALSE;

	pcsShrinePoint->m_bTeleportToTown = FALSE;

	return TRUE;
}

BOOL AgsmShrineBattle::OpenAllShirineByCommand()
{
	CloseAllShrineByCommand();

	return OpenAllShrine();
}

BOOL AgsmShrineBattle::OpenShirineByCommand(INT32 lShrinePointID)
{
	IterShrinePoint iter = m_pAgpmShrineBattle->m_stShrineBattle.FindByShrinePointID(lShrinePointID);
	if(!iter)
		return FALSE;


	// Shrine Object를 Random하게 배치한다.
	if(FALSE == m_pAgpmShrineBattle->RandomPlaceShrineRegion(iter))
		return FALSE;
	

	return OpenShrine(iter);
}

BOOL AgsmShrineBattle::CloseAllShrineByCommand()
{
	UINT32 ulClockCount = GetClockCount();
	IterShrinePoint iter;

	for(iter =m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
		StatusChange(iter, AGPDSHRINEBATTLE_STATUS_NORMAL, ulClockCount);

	

	EndShrineDungeon();
	m_pAgsmWarInfo->SendEndWarInfo(ShrineBattle);

	return TRUE;
}

BOOL AgsmShrineBattle::ShrineBattleRewardProcess(AgpdShrinePoint* pstShrinePoint)
{
	//ToDo
	//쉬라인전투의 보상을 하고 결과를 알려준다.
	if(!pstShrinePoint)
		return FALSE;

	// 그 지역에 있는 해당 종족에만 버프를 지급
	ApAdmin* pcsRegionCharacterCID = m_pAgsmAreaChatting->GetRegionAdmin();

	if(!pcsRegionCharacterCID)
		return FALSE;

	AgpdCharacter *pcsPointCharacter = m_pAgpmCharacter->GetCharacter(pstShrinePoint->m_nOccupyCharacterCid);
	if(!pcsPointCharacter)
		return FALSE;

	IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
	if(!iter)
		return FALSE;

	AgpdCharacter *pcsShrineCharacter = m_pAgpmCharacter->GetCharacter(pstShrinePoint->m_nShrineObjectCID);
	if(!pcsShrineCharacter)
		return FALSE;

	/************************************************************************/
	/* Carving Character                                                    */
	/************************************************************************/
	ApplyReward(pcsPointCharacter, pcsShrineCharacter, AGSM_SHRINE_REWARD_CARVING);
	
	INT32	lIndex	= 0;
	UINT32	*pnCID	= (UINT32 *)pcsRegionCharacterCID[iter->m_nRegionIndex].GetObjectSequence(&lIndex);
	while(pnCID)
	{
		AgpdCharacter *pcsCharacter = m_pAgpmCharacter->GetCharacter(*pnCID);
		if(pcsCharacter)
		{
			/************************************************************************/
			/* Common	                                                            */
			/************************************************************************/

			ApplyReward(pcsCharacter, pcsShrineCharacter, AGSM_SHRINE_REWARD_COMMON);

			/************************************************************************/
			/* WinRace									                            */
			/************************************************************************/
			if(m_pAgpmCharacter->GetRaceFromTemplate(pcsCharacter->m_pcsCharacterTemplate) == pstShrinePoint->m_nOccupationRace)
			{
				ApplyReward(pcsCharacter, pcsShrineCharacter, AGSM_SHRINE_REWARD_WIN_RACE);
			}

			pnCID	= (UINT32 *)pcsRegionCharacterCID[iter->m_nRegionIndex].GetObjectSequence(&lIndex);
		}
	}

	char strCharBuff[256] = { 0, };
	sprintf_s(strCharBuff, sizeof(strCharBuff), "ShrinePoint reward : ID = %s, OccupationRace = %d", pstShrinePoint->m_szPointName, pstShrinePoint->m_nOccupationRace);
	AuLogFile_s("LOG\\ShrineOpenStatic.log", strCharBuff);
	
	return TRUE;
}

BOOL AgsmShrineBattle::OpenShrineDungeonTimer(INT32 lCID, PVOID pClass, UINT32 uDelay, PVOID pvData)
{
	if(!lCID || !pClass)
		return FALSE;

	AgsmShrineBattle *pThis = static_cast<AgsmShrineBattle *>( pClass );

	pThis->OpenShrineDungeon();

	return TRUE;
}


BOOL AgsmShrineBattle::OpenShrineDungeon()
{
	//JK_쉬라인머지..던전은 일단 뺀다
/*
	if( m_pAgsmBattleGround )
	{
		if( m_pAgsmBattleGround->IsShrineDungeonOpen() )
			return TRUE;

		CTime bg_start = CTime::GetCurrentTime();

		INT32 nDay		=  bg_start.GetDayOfWeek();
		INT32 nHour		=  bg_start.GetHour();
		INT32 nMin		=  bg_start.GetMinute();
		INT32 nDuring	=  120;							//	진행시간

		m_pAgsmBattleGround->SetBattleGroundTime(TRUE, nDay, nHour, nMin, nDuring, FALSE);

		m_pAgsmBattleGround->UpdateShrineDungeonState(TRUE);
	}
*/
	return TRUE;
}

BOOL AgsmShrineBattle::EndShrineDungeon()
{
	/*
	if( m_pAgsmBattleGround )
	{
		m_pAgsmBattleGround->SetBattleGroundTime(TRUE, 0, 0, 0, 0, FALSE);

		if( TRUE == m_pAgsmBattleGround->IsShrineDungeonOpen() )
		{
			m_pAgsmBattleGround->UpdateShrineDungeonState(FALSE);
		}
	}

	m_pAgsmBattleGround->InitShrineDungeon();
	*/
	return TRUE;
}

//JK_쉬라인 추가
BOOL AgsmShrineBattle::PartyIsOverGoHome(INT32 lCID, PVOID pClass, UINT32 uDelay, PVOID pvData)
{
	if (!pClass || !pvData)
		return FALSE;

	AgsmShrineBattle	*pThis		= (AgsmShrineBattle *)	pClass;
	AgpdShrinePoint* pstShrinePoint	= (AgpdShrinePoint*)		pvData;


	ApAdmin* pcsRegionCharacterCID = pThis->m_pAgsmAreaChatting->GetRegionAdmin();

	if(!pcsRegionCharacterCID)
		return FALSE;

	IterShrineRegion iter = pThis->m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
	
	if(!iter)
		return FALSE;

	for(int i = 0; i < MAX_SHRINE_HAS_REGEION_NUM; ++i)
	{

		INT32	lIndex	= 0;
		INT32	*pnCID	= NULL;

		while(pnCID = (INT32 *)pcsRegionCharacterCID[iter->m_arrayRegionIndex[i]].GetObjectSequence(&lIndex))
		{
			AgpdCharacter *pcsCharacter = pThis->m_pAgpmCharacter->GetCharacter(*pnCID);
			if (pcsCharacter == NULL)
				continue;

			if (pThis->m_pAgpmCharacter->IsPC(pcsCharacter) && pThis->m_pAgsmCharacter->GetCharDPNID(pcsCharacter) != 0)
			{

				AuPOS	stNewTargetPos	= pcsCharacter->m_stPos;
				pThis->m_pAgpmEventBinding->GetBindingPositionForNewCharacter(pcsCharacter, &stNewTargetPos);

				pThis->m_pAgpmCharacter->StopCharacter(pcsCharacter, NULL);
				pThis->m_pAgpmCharacter->UpdatePosition(pcsCharacter, &stNewTargetPos, FALSE, TRUE);

			}
			
		}
	}

	return TRUE;



}


BOOL AgsmShrineBattle::StatusChange(AgpdShrinePoint* pstShrinePoint, INT32 nStatus, UINT32 ulClockCount, BOOL bSendMessage)
{
	//쉬라인전투의 status를 업데이트하고 전유저에 상태를 업데이트해준다.
	if(!pstShrinePoint)
		return FALSE;

	if(!m_pAgpmShrineBattle->m_stShrineBattle.m_bLoad)
		return FALSE;

	INT32 nPreStatus = pstShrinePoint->m_nCurrentStatus;
	pstShrinePoint->m_nCurrentStatus = nStatus;

	IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
	if(!iter)
		return FALSE;

	switch(nStatus)
	{
	case AGPDSHRINEBATTLE_STATUS_OPEN:
		{
			pstShrinePoint->m_nBattleOpenTime = ulClockCount;
			pstShrinePoint->m_nOccupyCharacterCid = AP_INVALID_CID;
			pstShrinePoint->m_nOccupationRace = AURACE_TYPE_NONE;
			pstShrinePoint->m_nTryOccupyRace = AURACE_TYPE_NONE;
			pstShrinePoint->m_nImprintStartTime = 0;
			pstShrinePoint->m_nOccupyStartTime = 0;
			

			if(bSendMessage)
				SendPacketShrineBattleMessageCodeToWorld(AGPMSHRINEBATTLE_MESSAGE_CODE_OPEN, pstShrinePoint->m_nShrinePointID, AURACE_TYPE_NONE, AP_INVALID_CID);

			OnStatusOpen(pstShrinePoint, ulClockCount);

			SendShrineWarInfo(pstShrinePoint,1,0);
			
		}
		break;

	case AGPDSHRINEBATTLE_STATUS_TRYIMPRINT:
		{
			pstShrinePoint->m_nImprintStartTime = ulClockCount;

			if(bSendMessage)
				SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT, pstShrinePoint->m_nShrinePointID, pstShrinePoint->m_nTryOccupyRace, pstShrinePoint->m_nOccupyCharacterCid);
			

			OnStatusTryImprint(pstShrinePoint, ulClockCount);

			SendShrineWarInfo(pstShrinePoint,1,0);
		}
		break;
	case AGPDSHRINEBATTLE_STATUS_IMPRINT:
		{
			if(bSendMessage)
				SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_IMPRINT, pstShrinePoint->m_nShrinePointID, pstShrinePoint->m_nTryOccupyRace, pstShrinePoint->m_nOccupyCharacterCid);

			OnStatusImprint(pstShrinePoint, ulClockCount);


		}
		break;
	case AGPDSHRINEBATTLE_STATUS_TRYOCCUPY:
		{
			pstShrinePoint->m_nOccupyStartTime = ulClockCount;

			if(bSendMessage)
				SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_OCCUPY, pstShrinePoint->m_nShrinePointID, pstShrinePoint->m_nTryOccupyRace, pstShrinePoint->m_nOccupyCharacterCid);

			OnStatusTryOccupy(pstShrinePoint, ulClockCount);
			
			SendShrineWarInfo(pstShrinePoint,2,0);
			
		}
		break;
	case AGPDSHRINEBATTLE_STATUS_OCCUPY:
		{
			pstShrinePoint->m_nOccupationRace = pstShrinePoint->m_nTryOccupyRace;

			if(bSendMessage)
				SendPacketShrineBattleMessageCodeToWorld(AGPMSHRINEBATTLE_MESSAGE_CODE_OCCUPYIED_BY_RACE, pstShrinePoint->m_nShrinePointID, pstShrinePoint->m_nTryOccupyRace, pstShrinePoint->m_nOccupyCharacterCid);

			OnStatusOccupy(pstShrinePoint, ulClockCount);

			SendShrineWarInfo(pstShrinePoint,3,0);

			SendEndWar(pstShrinePoint);
		}
		break;
	case AGPDSHRINEBATTLE_STATUS_END:
		{
			if(bSendMessage)
				SendPacketShrineBattleMessageCodeToWorld(AGPMSHRINEBATTLE_MESSAGE_CODE_CLOSE, pstShrinePoint->m_nShrinePointID, pstShrinePoint->m_nTryOccupyRace, pstShrinePoint->m_nOccupyCharacterCid);

			OnStatusEnd(pstShrinePoint, ulClockCount);
			
		}
		break;
	default:
		break;
	}

	return SendPacketShrineBattleStatusChangeToWorld(pstShrinePoint, nPreStatus);
}

BOOL AgsmShrineBattle::SendPacketShrineBattleInformationToCharacter(AgpdShrinePoint* pstShrinePoint, AgpdCharacter *pcsCharacter)
{
	if(!pstShrinePoint)
		return FALSE;

	if(!pcsCharacter)
		return FALSE;

	AgsdCharacter * pcsAgsdCharacter = m_pAgsmCharacter->GetADCharacter(pcsCharacter);

	if(!pcsAgsdCharacter)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_INFORMATION Packet;

	Packet.nShrinePointID = pstShrinePoint->m_nShrinePointID;
	Packet.stShrinePoint = *pstShrinePoint;

	BOOL bResult = AgsEngine::GetInstance()->SendPacket(Packet, pcsAgsdCharacter->m_dpnidCharacter);

	return bResult;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleInformationToRegion(AgpdShrinePoint* pstShrinePoint)
{
	if(!pstShrinePoint)
		return FALSE;

	ApAdmin* pcsRegionCharacterCID = m_pAgsmAreaChatting->GetRegionAdmin();

	if(!pcsRegionCharacterCID)
		return TRUE;

	PACKET_AGPP_SHRINEBATTLE_INFORMATION Packet;

	Packet.nShrinePointID = pstShrinePoint->m_nShrinePointID;
	Packet.stShrinePoint = *pstShrinePoint;

	IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
	if(!iter)
		return FALSE;

	INT32	lIndex	= 0;
	UINT32	*pnCID	= (UINT32 *)pcsRegionCharacterCID[iter->m_nRegionIndex].GetObjectSequence(&lIndex);

	while(pnCID)
	{
		DPNID npNid = m_pAgsmCharacter->GetCharDPNID(*pnCID);
		
		AgsEngine::GetInstance()->SendPacket(Packet, npNid);

		pnCID	= (UINT32 *)pcsRegionCharacterCID[iter->m_nRegionIndex].GetObjectSequence(&lIndex);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleInformationToWorld(AgpdShrinePoint* pstShrinePoint)
{
	if(!pstShrinePoint)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_INFORMATION Packet;

	Packet.nShrinePointID = pstShrinePoint->m_nShrinePointID;
	Packet.stShrinePoint = *pstShrinePoint;

	ApAdmin *pcsAllAdmin = m_pAgsmAreaChatting->GetAllAdmin();

	if(!pcsAllAdmin)
		return FALSE;

	INT32	lIndex	= 0;
	UINT32	*pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);

	while(pulNID)
	{
		AgsEngine::GetInstance()->SendPacket(Packet, *pulNID);

		pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleReadyToStartToWorld(INT32 lRestMinute, AgpdShrinePoint* pstShrinePoint)
{
	if (NULL == pstShrinePoint)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_READY_TO_OPEN stPacket(lRestMinute, pstShrinePoint->m_nShrinePointID);

	ApAdmin *pcsAllAdmin = m_pAgsmAreaChatting->GetAllAdmin();

	if(!pcsAllAdmin)
		return FALSE;

	INT32	lIndex	= 0;
	UINT32	*pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);

	while(pulNID)
	{
		AgsEngine::GetInstance()->SendPacket(stPacket, *pulNID);

		pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleStatusChangeToWorld(AgpdShrinePoint* pstShrinePoint, INT32 nPreStatus)
{
	if(!pstShrinePoint)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_STATUS_UPDATE Packet(nPreStatus, *pstShrinePoint);

	ApAdmin *pcsAllAdmin = m_pAgsmAreaChatting->GetAllAdmin();

	if(!pcsAllAdmin)
		return FALSE;

	INT32	lIndex	= 0;
	UINT32	*pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);

	while(pulNID)
	{
		AgsEngine::GetInstance()->SendPacket(Packet, *pulNID);

		pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleFullInformationToCharacter(AgpdCharacter *pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	AgsdCharacter * pcsAgsdCharacter = m_pAgsmCharacter->GetADCharacter(pcsCharacter);

	if(!pcsAgsdCharacter)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_INFORMATION_TOTAL Packet;

	IterShrineRegion iter;
	INT nPointNumber = 0;

	for(iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		IterShrinePoint iter2 = m_pAgpmShrineBattle->m_stShrineBattle.FindByShrinePointID(iter->m_nShrinePointID);
		if(!iter2)
			continue;

		PACKET_AGPP_SHRINEBATTLE_INFORMATION packetInformation;
		packetInformation.nShrinePointID = iter2->m_nShrinePointID;
		packetInformation.stShrinePoint = *iter2;
		Packet.m_stPointInformation[nPointNumber] = packetInformation;
		nPointNumber++;
	}

	BOOL bResult = AgsEngine::GetInstance()->SendPacket(Packet, pcsAgsdCharacter->m_dpnidCharacter);

	return bResult;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleFullInformationToWorld()
{
	PACKET_AGPP_SHRINEBATTLE_INFORMATION_TOTAL Packet;

	IterShrineRegion iter;
	INT nPointNumber = 0;

	for(iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		IterShrinePoint iter2 = m_pAgpmShrineBattle->m_stShrineBattle.FindByShrinePointID(iter->m_nShrinePointID);
		if(!iter2)
			continue;

		PACKET_AGPP_SHRINEBATTLE_INFORMATION packetInformation;
		packetInformation.nShrinePointID = iter2->m_nShrinePointID;
		packetInformation.stShrinePoint = *iter2;
		Packet.m_stPointInformation[nPointNumber] = packetInformation;
		nPointNumber++;
	}

	ApAdmin *pcsAllAdmin = m_pAgsmAreaChatting->GetAllAdmin();

	if(!pcsAllAdmin)
		return FALSE;

	INT32	lIndex	= 0;
	UINT32	*pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);

	while(pulNID)
	{
		AgsEngine::GetInstance()->SendPacket(Packet, *pulNID);

		pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleRacePvPInformationToWorld()
{
	/*AgpdCharacter* pcsTarget = NULL;
	INT32 nIndex = 0;
	while(pcsTarget = m_pAgpmCharacter->GetCharSequence(&nIndex))
	{
		INT32 lWinScore		= 0;
		INT32 lLoseScore	= 0;

		if(m_pAgpmCharacter->IsPC(pcsTarget) && m_pAgsmCharacter->GetCharDPNID(pcsTarget) != 0)
		{
			switch(m_pAgpmCharacter->GetCharacterRace(pcsTarget).detail.nRace)
			{
			case AURACE_TYPE_HUMAN:
				{
					lWinScore  = m_pAgpmShrineBattle->GetRaceWinScore(AURACE_TYPE_HUMAN); 
					lLoseScore = m_pAgpmShrineBattle->GetRaceLoseScore(AURACE_TYPE_HUMAN);
				} break;

			case AURACE_TYPE_ORC:
				{
					lWinScore  = m_pAgpmShrineBattle->GetRaceWinScore(AURACE_TYPE_ORC); 
					lLoseScore = m_pAgpmShrineBattle->GetRaceLoseScore(AURACE_TYPE_ORC);
				} break;

			case AURACE_TYPE_MOONELF:
				{
					lWinScore  = m_pAgpmShrineBattle->GetRaceWinScore(AURACE_TYPE_MOONELF); 
					lLoseScore = m_pAgpmShrineBattle->GetRaceLoseScore(AURACE_TYPE_MOONELF);
				} break;

			case AURACE_TYPE_DRAGONSCION:
				{
					lWinScore  = m_pAgpmShrineBattle->GetRaceWinScore(AURACE_TYPE_DRAGONSCION); 
					lLoseScore = m_pAgpmShrineBattle->GetRaceLoseScore(AURACE_TYPE_DRAGONSCION);
				} break;
			}

			PACKET_AGPP_SHRINEBATTLE_RACE_SCORE stPacket(lWinScore, lLoseScore);
			
			AgsEngine::GetInstance()->SendPacket(stPacket, m_pAgsmCharacter->GetCharDPNID(pcsTarget));
		}		
	}*///check 

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleImprintResult(AgpdShrinePoint* pstShrinePoint, AgpdCharacter *pcsCharacter, INT32 nPrevStatus, BOOL bResult)
{
	if(!pstShrinePoint)
		return FALSE;

	if(!pcsCharacter)
		return FALSE;

	AgsdCharacter * pcsAgsdCharacter = m_pAgsmCharacter->GetADCharacter(pcsCharacter);

	if(!pcsAgsdCharacter)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_IMPRINT_RESULT packet(pcsCharacter->m_lID, pstShrinePoint->m_nShrinePointID, nPrevStatus, bResult, pstShrinePoint->m_nShrineObjectCID);


	ApAdmin *pcsAllAdmin = m_pAgsmAreaChatting->GetAllAdmin();

	if(!pcsAllAdmin)
		return FALSE;

	INT32	lIndex	= 0;
	UINT32	*pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);

	while(pulNID)
	{
		AgsEngine::GetInstance()->SendPacket(packet, *pulNID);

		pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleMessageCodeToCharacter(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid, AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	AgsdCharacter * pcsAgsdCharacter = m_pAgsmCharacter->GetADCharacter(pcsCharacter);

	if(!pcsAgsdCharacter)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_MESSAGE_CODE packet(eMessageCode, nShrinePointID, nRaceID);

	AgpdCharacter *pcsPointCharacter = m_pAgpmCharacter->GetCharacter(nPointCid);
	if(pcsPointCharacter)
		strncpy(packet.szPointCharacterName, pcsPointCharacter->m_szID, AGPDCHARACTER_MAX_ID_LENGTH);

	BOOL bResult = AgsEngine::GetInstance()->SendPacket(packet, pcsAgsdCharacter->m_dpnidCharacter);

	return bResult;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleMessageCodeToRace(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid, INT32 nToRaceID)
{
	PACKET_AGPP_SHRINEBATTLE_MESSAGE_CODE packet(eMessageCode, nShrinePointID, nRaceID);

	AgpdCharacter *pcsPointCharacter = m_pAgpmCharacter->GetCharacter(nPointCid);
	if(pcsPointCharacter)
		strncpy(packet.szPointCharacterName, pcsPointCharacter->m_szID, AGPDCHARACTER_MAX_ID_LENGTH);

	AgpdCharacter* pcsTarget = NULL;
	INT32 nIndex = 0;
	while(pcsTarget = m_pAgpmCharacter->GetCharSequence(&nIndex))
	{
		if(m_pAgpmCharacter->IsPC(pcsTarget) && m_pAgsmCharacter->GetCharDPNID(pcsTarget) != 0 && m_pAgpmCharacter->IsSameRace(pcsTarget, nToRaceID))
			SendPacketShrineBattleMessageCodeToCharacter(eMessageCode, nShrinePointID, nRaceID, nPointCid, pcsTarget);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleMessageCodeToRegion(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid, INT32 nRegionIndex)
{
	ApAdmin* pcsRegionCharacterCID = m_pAgsmAreaChatting->GetRegionAdmin();

	if(!pcsRegionCharacterCID)
		return TRUE;

	PACKET_AGPP_SHRINEBATTLE_MESSAGE_CODE packet(eMessageCode, nShrinePointID, nRaceID);

	AgpdCharacter *pcsPointCharacter = m_pAgpmCharacter->GetCharacter(nPointCid);
	if(pcsPointCharacter)
		strncpy(packet.szPointCharacterName, pcsPointCharacter->m_szID, AGPDCHARACTER_MAX_ID_LENGTH);

	INT32	lIndex	= 0;
	UINT32	*pnCID	= (UINT32 *)pcsRegionCharacterCID[nRegionIndex].GetObjectSequence(&lIndex);

	while(pnCID)
	{
		DPNID npNid = m_pAgsmCharacter->GetCharDPNID(*pnCID);

		AgsEngine::GetInstance()->SendPacket(packet, npNid);

		pnCID	= (UINT32 *)pcsRegionCharacterCID[nRegionIndex].GetObjectSequence(&lIndex);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleMessageCodeToShrineArea(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid)
{
	ApAdmin* pcsRegionCharacterCID = m_pAgsmAreaChatting->GetRegionAdmin();

	if(!pcsRegionCharacterCID)
		return TRUE;

	PACKET_AGPP_SHRINEBATTLE_MESSAGE_CODE packet(eMessageCode, nShrinePointID, nRaceID);

	AgpdCharacter *pcsPointCharacter = m_pAgpmCharacter->GetCharacter(nPointCid);
	if(pcsPointCharacter)
		strncpy(packet.szPointCharacterName, pcsPointCharacter->m_szID, AGPDCHARACTER_MAX_ID_LENGTH);

	IterShrineRegion iterRegion = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(nShrinePointID);
	if(!iterRegion)
		return FALSE;


	UINT32	*pnCID = NULL;

	for(int i = 0; i < MAX_SHRINE_HAS_REGEION_NUM; ++i)
	{
		INT32	lIndex	= 0;
		pnCID	= (UINT32 *)pcsRegionCharacterCID[iterRegion->m_arrayRegionIndex[i]].GetObjectSequence(&lIndex);

		while(pnCID)
		{
			DPNID npNid = m_pAgsmCharacter->GetCharDPNID(*pnCID);

			AgsEngine::GetInstance()->SendPacket(packet, npNid);

			pnCID	= (UINT32 *)pcsRegionCharacterCID[iterRegion->m_arrayRegionIndex[i]].GetObjectSequence(&lIndex);
		}
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineBattleMessageCodeToWorld(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid)
{
	PACKET_AGPP_SHRINEBATTLE_MESSAGE_CODE packet(eMessageCode, nShrinePointID, nRaceID);

	AgpdCharacter *pcsPointCharacter = m_pAgpmCharacter->GetCharacter(nPointCid);
	if(pcsPointCharacter)
		strncpy(packet.szPointCharacterName, pcsPointCharacter->m_szID, AGPDCHARACTER_MAX_ID_LENGTH);

	ApAdmin *pcsAllAdmin = m_pAgsmAreaChatting->GetAllAdmin();

	if(!pcsAllAdmin)
		return FALSE;

	INT32	lIndex	= 0;
	UINT32	*pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);

	while(pulNID)
	{
		AgsEngine::GetInstance()->SendPacket(packet, *pulNID);

		pulNID	= (UINT32 *) pcsAllAdmin->GetObjectSequence(&lIndex);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketShrineTimebarStatusToCharacter(AgpdShrinePoint* pstShrinePoint, BOOL bAppear, AgpdCharacter *pcsCharacter)
{
	if(!pstShrinePoint)
		return FALSE;

	AgsdCharacter * pcsAgsdCharacter = m_pAgsmCharacter->GetADCharacter(pcsCharacter);

	if(!pcsAgsdCharacter)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_TIMEBAR_STATE packet(pstShrinePoint->m_nShrinePointID, bAppear);

	BOOL bResult = AgsEngine::GetInstance()->SendPacket(packet, pcsAgsdCharacter->m_dpnidCharacter);

	return bResult;
}

BOOL AgsmShrineBattle::SendPacketShrineTimebarStatusToRegion(AgpdShrinePoint* pstShrinePoint, BOOL bAppear, INT32 nRegionIndex)
{
	if(!pstShrinePoint)
		return FALSE;

	PACKET_AGPP_SHRINEBATTLE_TIMEBAR_STATE packet(pstShrinePoint->m_nShrinePointID, bAppear);

	ApAdmin* pcsRegionCharacterCID = m_pAgsmAreaChatting->GetRegionAdmin();

	if(!pcsRegionCharacterCID)
		return FALSE;

	INT32	lIndex	= 0;
	UINT32	*pnCID	= (UINT32 *)pcsRegionCharacterCID[nRegionIndex].GetObjectSequence(&lIndex);

	while(pnCID)
	{
		DPNID npNid = m_pAgsmCharacter->GetCharDPNID(*pnCID);

		AgsEngine::GetInstance()->SendPacket(packet, npNid);

		pnCID	= (UINT32 *)pcsRegionCharacterCID[nRegionIndex].GetObjectSequence(&lIndex);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketInShrineRegion(PACKET_HEADER& pPacket, PACKET_PRIORITY ePriority, INT16 nFlag)
{
	ApAdmin* pcsRegionCharacterCID = m_pAgsmAreaChatting->GetRegionAdmin();

	if(!pcsRegionCharacterCID)
		return FALSE;

	IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.begin();
	while(iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.end())
	{
		INT32 nRegionIndex = iter->m_nRegionIndex;

		INT32	lIndex	= 0;
		UINT32	*pnCID	= (UINT32 *)pcsRegionCharacterCID[nRegionIndex].GetObjectSequence(&lIndex);

		while(pnCID)
		{
			DPNID npNid = m_pAgsmCharacter->GetCharDPNID(*pnCID);

			AgsEngine::GetInstance()->SendPacket(pPacket, npNid);

			pnCID	= (UINT32 *)pcsRegionCharacterCID[nRegionIndex].GetObjectSequence(&lIndex);
		}

		++iter;
	}

	return TRUE;
}

BOOL AgsmShrineBattle::CBAddCharacterToMap(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmShrineBattle	*pThis				= (AgsmShrineBattle *)	pClass;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)		pData;

	if (!pThis->m_pAgpmCharacter->IsPC(pcsCharacter))
		return TRUE;

	//쉬라인 정보를 보내준다.
	return pThis->SendPacketShrineBattleFullInformationToCharacter(pcsCharacter);
}

BOOL AgsmShrineBattle::CBRemoveCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmShrineBattle *pThis			= (AgsmShrineBattle *)pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	return pThis->RemoveCharacter(pcsCharacter);
}

BOOL AgsmShrineBattle::CBRegionChange(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgsmShrineBattle* pThis = (AgsmShrineBattle*)pClass;
	INT16 nPrevRegionIndex = pCustData ? *(INT16*)pCustData : -1;

	//리전 변경시 상세정보를 보내주도록 한다.
	return pThis->RegionChange(pcsCharacter, nPrevRegionIndex);
}

BOOL AgsmShrineBattle::CBShrineBattleInformationRequest(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgsmShrineBattle *pThis = (AgsmShrineBattle *) pClass;
	PACKET_AGPP_SHRINEBATTLE_INFORMATION_REQUEST *pPacket = (PACKET_AGPP_SHRINEBATTLE_INFORMATION_REQUEST *)pData;

	AgpdCharacter *pcsCharacter = pThis->m_pAgpmCharacter->GetCharacter(pPacket->nCid);
	if(NULL == pcsCharacter)
		return FALSE;
	
	//쉬라인 정보 요청시 정보 보내주도록 한다.
	return pThis->SendPacketShrineBattleFullInformationToCharacter(pcsCharacter);
}

BOOL AgsmShrineBattle::CBShrineBattleTryImprint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgsmShrineBattle *pThis = (AgsmShrineBattle *) pClass;
	PACKET_AGPP_SHRINEBATTLE_TRYIMPRINT* pPacket = (PACKET_AGPP_SHRINEBATTLE_TRYIMPRINT*) pData;

	AgpdCharacter *pcsCharacter = pThis->m_pAgpmCharacter->GetCharacter(pPacket->nCid);
	IterShrinePoint iterShrinePoint = pThis->FindShrinePointSameRegion(pcsCharacter);

	if(!pcsCharacter || !iterShrinePoint)
		return FALSE;

	return pThis->ShrineBattleTryImprint(pcsCharacter, iterShrinePoint);
}

BOOL AgsmShrineBattle::CBShrineBattleJoinAnswer(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgsmShrineBattle *pThis = (AgsmShrineBattle *) pClass;
	PACKET_AGPP_SHRINEBATTLE_JOIN_ANSWER* pPacket = (PACKET_AGPP_SHRINEBATTLE_JOIN_ANSWER*) pData;

	AgpdCharacter *pcsCharacter = pThis->m_pAgpmCharacter->GetCharacter(pPacket->nCID);
	if(NULL == pcsCharacter)
		return FALSE;

	// 변신중이면 못 들어온다..메세지는 클라에서.
	if(pcsCharacter->m_bIsTrasform)
		return FALSE;

	UINT32 ulNID = pThis->m_pAgsmCharacter->GetCharDPNID(pcsCharacter);

	INT32 lShrinePointID = pPacket->nShrinePointID;

	INT32 nLevel = pThis->m_pAgpmCharacter->GetLevel(pcsCharacter);

	IterShrinePoint iter = pThis->m_pAgpmShrineBattle->m_stShrineBattle.FindByShrinePointID(lShrinePointID);
	if(iter)
	{
		//현재 쉬라인 지역에 들어와 있다면 초대에 응할수 없다.
		if (iter == pThis->m_pAgpmShrineBattle->FindShrinePointAreaInvalveCharacter(pcsCharacter->m_nBindingRegionIndex))
		{
			return FALSE;
		}
		
		// 공성중인 공성지역에서는 쉬라인으로 이동할 수 없다
		BOOL bIsInSiegeWarIngArea = FALSE;
		BOOL bIsParticipantSiegeWar = FALSE;
		PVOID pvBuffer[2];
		pvBuffer[0] = &bIsInSiegeWarIngArea;
		pvBuffer[1] = &bIsParticipantSiegeWar;
		pThis->m_pAgpmPvP->EnumCallback(AGPMPVP_CB_IS_IN_SIEGEWAR_ING_AREA, pcsCharacter, pvBuffer);

		if ( bIsInSiegeWarIngArea )
		{
			pThis->m_pAgsmSystemMessage->SendSystemMessageWithType(ulNID, AGPMSYSTEMMESSAGE_PACKET_MODAL_MESSAGE, AGPMSYSTEMMESSAGE_CODE_SHRINEBATTLE_ERR_MOVE_IN_SIEGE);
			return FALSE;
		}

		// 레벨 제한 체크하기전에 현재 카오틱 프론티어에 있다면, 원래의 레벨로 체크해야한다
		if (pThis->m_pApmMap->GetTargetPositionLevelLimit(pcsCharacter->m_stPos))
		{
			nLevel = pThis->m_pAgpmCharacter->GetLevelBefore(pcsCharacter);
		}

		//쉬라인 제한 레벨보다 케릭터 레벨이 높다면 못들어감//JK_쉬라인추가
		if( (iter->m_nLevelMax && iter->m_nLevelMax < nLevel) || (iter->m_nLevelMin && iter->m_nLevelMin > nLevel))
		{
			//뭐라고 메세지라도 보내면...
			pThis->m_pAgsmSystemMessage->SendSystemMessageWithType(ulNID, AGPMSYSTEMMESSAGE_PACKET_MODAL_MESSAGE, AGPMSYSTEMMESSAGE_CODE_SHRINEBATTLE_LEVEL_LIMIT);
			return FALSE;
		}

		// 해당 쉬라인이 보통 상태이거나 끝난 상태이면 허가하지 않는다.
		if(iter->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_NORMAL || iter->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_END)
		{
			// 가능하면 클라에 왜 안되는지 이유를 말해주는 SystemMessage좀 날려주고
			// 단 m_bAllowMove가 TRUE인 상황은 통과시켜준다.
			if (FALSE == pThis->m_bAllowMove)
				return FALSE;
		}

		// 해당 지역으로 UpdatePosition을 해준다.
		IterShrineRegion iterRegion = pThis->m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(lShrinePointID);
		if(iterRegion)
		{
			AuPOS stBasePos;
			ZeroMemory(&stBasePos, sizeof(stBasePos));

			// 종족별로 지정위치를 바꿔준다.
			if(pThis->m_pAgpmCharacter->GetCharacterRace(pcsCharacter).detail.nRace == AURACE_TYPE_HUMAN)
			{
				if(iterRegion->m_nHumanPlayerCount >= SHRINEBATTLEPLAYER_MAX_PER_RACE)
				{
					pThis->m_pAgsmSystemMessage->SendSystemMessageWithType(ulNID, AGPMSYSTEMMESSAGE_PACKET_MODAL_MESSAGE, AGPMSYSTEMMESSAGE_CODE_SHRINEBATTLE_PLAYERCOUNT_LIMIT);
					return FALSE;
				}

				stBasePos = iterRegion->m_stHumanBasePoint;
				iterRegion->m_nHumanPlayerCount++;
			}
			else if(pThis->m_pAgpmCharacter->GetCharacterRace(pcsCharacter).detail.nRace == AURACE_TYPE_ORC)
			{
				if(iterRegion->m_nOrcPlayerCount >= SHRINEBATTLEPLAYER_MAX_PER_RACE) 
				{
					pThis->m_pAgsmSystemMessage->SendSystemMessageWithType(ulNID, AGPMSYSTEMMESSAGE_PACKET_MODAL_MESSAGE, AGPMSYSTEMMESSAGE_CODE_SHRINEBATTLE_PLAYERCOUNT_LIMIT);
					return FALSE;
				}

				stBasePos = iterRegion->m_stOrcBasePoint;
				iterRegion->m_nOrcPlayerCount++;
			}
			else if(pThis->m_pAgpmCharacter->GetCharacterRace(pcsCharacter).detail.nRace == AURACE_TYPE_MOONELF)
			{
				if(iterRegion->m_nMoonElfPlayerCount >= SHRINEBATTLEPLAYER_MAX_PER_RACE) 
				{
					pThis->m_pAgsmSystemMessage->SendSystemMessageWithType(ulNID, AGPMSYSTEMMESSAGE_PACKET_MODAL_MESSAGE, AGPMSYSTEMMESSAGE_CODE_SHRINEBATTLE_PLAYERCOUNT_LIMIT);
					return FALSE;
				}

				stBasePos = iterRegion->m_stMoonElfBasePoint;
				iterRegion->m_nMoonElfPlayerCount++;
			}
			else if(pThis->m_pAgpmCharacter->GetCharacterRace(pcsCharacter).detail.nRace == AURACE_TYPE_DRAGONSCION)
			{
				if(iterRegion->m_nScionPlayerCount >= SHRINEBATTLEPLAYER_MAX_PER_RACE)
				{
					pThis->m_pAgsmSystemMessage->SendSystemMessageWithType(ulNID, AGPMSYSTEMMESSAGE_PACKET_MODAL_MESSAGE, AGPMSYSTEMMESSAGE_CODE_SHRINEBATTLE_PLAYERCOUNT_LIMIT);
					return FALSE;
				}

				stBasePos = iterRegion->m_stScionBasePoint;
				iterRegion->m_nScionPlayerCount++;
			}

			AuAutoLock pLock(pcsCharacter->m_Mutex);
			if(!pLock.Result())
				return FALSE;

			pThis->m_pAgpmCharacter->StopCharacter(pcsCharacter, NULL);
			pThis->m_pAgpmCharacter->UpdatePosition(pcsCharacter, &stBasePos, 0, TRUE);
		}

		pThis->m_pAgsmWarInfo->SendTimeStepToCharacter(ShrineBattle, pcsCharacter, iter->m_nTimeLimitTime/(1000*60));


		switch(iter->m_nCurrentStatus)
		{
		case AGPDSHRINEBATTLE_STATUS_NORMAL:
			pThis->m_pAgsmWarInfo->SendWarInfoToCharacter(pcsCharacter, ShrineBattle, 0, 0);
			break;

		case AGPDSHRINEBATTLE_STATUS_OPEN:
		case AGPDSHRINEBATTLE_STATUS_TRYIMPRINT:
			pThis->m_pAgsmWarInfo->SendWarInfoToCharacter(pcsCharacter, ShrineBattle, 1, 0);
			break;

		case AGPDSHRINEBATTLE_STATUS_IMPRINT:
		case AGPDSHRINEBATTLE_STATUS_TRYOCCUPY:
			pThis->m_pAgsmWarInfo->SendWarInfoToCharacter(pcsCharacter, ShrineBattle, 2, 0);
			break;

		case AGPDSHRINEBATTLE_STATUS_OCCUPY:
		case AGPDSHRINEBATTLE_STATUS_END:
			pThis->m_pAgsmWarInfo->SendWarInfoToCharacter(pcsCharacter, ShrineBattle, 3, 0);
			pThis->m_pAgsmWarInfo->SendEndWarInfoToCharacter(ShrineBattle, pcsCharacter);
			break;
		default:
			break;
		}
	}

	return TRUE;
}

BOOL AgsmShrineBattle::CBDeath(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgpdCharacter* pcsDeadCharacter = (AgpdCharacter*)pData;
	AgsmShrineBattle* pThis = (AgsmShrineBattle*)pClass;
	ApBase* pcsAttackerBase = pCustData ? (ApBase*)pCustData : NULL;

	//각인도중 죽었을때는 각인 취소해준다.
	pThis->Death(pcsDeadCharacter, pcsAttackerBase);

	

	return TRUE;
}

BOOL AgsmShrineBattle::CBMoveCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(NULL == pClass || NULL == pData)
		return FALSE;

	AgsmShrineBattle*	pThis			= (AgsmShrineBattle *) pClass;
	AgpdCharacter*	pcsAgpdCharacter	= (AgpdCharacter *) pData;

	//각인도중 이동하면 각인 취소해준다.
	return pThis->Move(pcsAgpdCharacter);
}

BOOL AgsmShrineBattle::CBSpawnCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmShrineBattle	*pThis			= (AgsmShrineBattle *)	pClass;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)	pData;

	pThis->SettingShrineObject(pcsCharacter);

	return TRUE;
}

BOOL AgsmShrineBattle::RegionChange(AgpdCharacter *pcsCharacter, INT16 nPrevRegionIndex)
{
	if(!pcsCharacter)
		return FALSE;

	// 쉬라인경기장에서 일반 지역으로 나오는 경우
	if(m_pAgpmShrineBattle->IsInShrineRegionByIndex(nPrevRegionIndex) && !m_pAgpmShrineBattle->IsInShrineRegionByIndex(pcsCharacter))
	{
		m_pAgsmSkill->RecastSaveSkill(pcsCharacter);
	}
	// 일반지역에서 쉬라인경기장 안으로 들어오는 경우
	if( !m_pAgpmShrineBattle->IsInShrineRegionByIndex(nPrevRegionIndex) && m_pAgpmShrineBattle->IsInShrineRegionByIndex(pcsCharacter))
	{
		m_pAgsmSkill->EndRaceBuffedSkill(pcsCharacter,TRUE);
		AgpdCharacterWarPVPData *pAgpdPvPData  = m_pAgsmWarInfo->GetCharacterData(pcsCharacter);
		if(pAgpdPvPData) pAgpdPvPData->Init();

		// 버프들을 지워주고
		m_pAgsmSkill->EndAllBuffedSkillExceptCashSkill((ApBase*)pcsCharacter, TRUE);
		// 쉬라인에 들어와서 자기의 버프스킬은 다시 캐스팅 할 수 있도록 캐스팅 초기화
		AgpdSkillAttachData *pcsAttachData = m_pAgpmSkill->GetAttachSkillData((ApBase*)pcsCharacter);
		if (pcsAttachData)
		{
			for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
			{
				if (pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
					break;

				AgpdSkill *pcsSkill = pcsAttachData->m_apcsSkill[i];
				if (pcsSkill)
				{
					m_pAgsmSkill->ProcessInitCoolTime(pcsCharacter, pcsSkill->m_pcsTemplate->m_lID);
				}
			}
		}
	}


	// 현재도 쉬라인이 아니고 전에도 쉬라인 리젼이 아닌사람은 그냥 통과
	if(!m_pAgpmShrineBattle->IsInShrineRegion(pcsCharacter) && !m_pAgpmShrineBattle->IsInShrineRegionPast(nPrevRegionIndex))
		return TRUE;


	// 리전변경시 이루어져야 될 것을 한다.
	// 각인자, 점령시도중인 캐릭터가 지역을 빠져나가면 status와 charater 상태를 초기화
	// 진입시 그 리전에 해당하는 쉬라인 정보를 보내준다.

	for(IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		IterShrinePoint iter2 = m_pAgpmShrineBattle->m_stShrineBattle.FindByShrinePointID(iter->m_nShrinePointID);

		if(iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT || iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYOCCUPY)
		{
			INT32 nPrevStatus = iter2->m_nCurrentStatus;

			if(pcsCharacter->m_lID == iter2->m_nOccupyCharacterCid && pcsCharacter->m_nBindingRegionIndex != iter->m_nRegionIndex)
			{
				if(iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT)
				{
					SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT_FAIL, iter2->m_nShrinePointID, iter2->m_nTryOccupyRace, iter2->m_nOccupyCharacterCid);

					iter2->m_nOccupyCharacterCid = AP_INVALID_CID;
					ZeroMemory(iter2->m_szOccupyCharacterName, sizeof(iter2->m_szOccupyCharacterName));
				}
				else
				{
					SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_OCCUPY_FAIL, iter2->m_nShrinePointID, iter2->m_nTryOccupyRace, iter2->m_nOccupyCharacterCid);
					SendPacketShrineTimebarStatusToRegion(iter2, FALSE, iter->m_nRegionIndex);

					iter2->m_nOccupyCharacterCid = AP_INVALID_CID;
					ZeroMemory(iter2->m_szOccupyCharacterName, sizeof(iter2->m_szOccupyCharacterName));
				}

				//StatusChange(iter2, AGPDSHRINEBATTLE_STATUS_OPEN, GetClockCount(), FALSE);
				StatusChange(iter2, AGPDSHRINEBATTLE_STATUS_OPEN, iter2->m_nBattleOpenTime , FALSE);//JK_쉬라인머지 점렬취소시 쉬라인 진행 시간 초기화되는 현상 변경

				SendPacketShrineBattleImprintResult(iter2, pcsCharacter, nPrevStatus, FALSE);
			}
		}

		// 새로 쉬라인 지역으로 들어온사람
		//if(m_pAgpmShrineBattle->IsInShrineRegion(pcsCharacter) && !m_pAgpmShrineBattle->IsInShrineRegionPast(nPrevRegionIndex))
		if(m_pAgpmShrineBattle->IsInShrineRegion(pcsCharacter) && !m_pAgpmShrineBattle->IsInShrineRegionPast(nPrevRegionIndex) 
			&& (pcsCharacter->m_nBindingRegionIndex == iter->m_nRegionIndex))//JK_쉬라인머지..해당 리전인 경우
		{
			SendPacketShrineBattleInformationToCharacter(iter2, pcsCharacter);
			
			if(iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYOCCUPY)
			{
				/*
				AgpdCharacter *pcsOccupyChar = m_pAgpmCharacter->GetCharacter(iter2->m_nOccupyCharacterCid);
				if(NULL == pcsOccupyChar)
					return TRUE;

				SendPacketShrineBattleImprintResult(iter2, pcsOccupyChar, iter2->m_nCurrentStatus, TRUE);
				*/
		
				SendPacketShrineTimebarStatusToCharacter(iter2, TRUE, pcsCharacter);
			}

			// 투명상태로 쉬라인에 들어오는 경우 투명을 풀어준다.
			if (m_pAgpmCharacter->IsStatusTransparent(pcsCharacter) == TRUE)
				m_pAgpmCharacter->UpdateUnsetSpecialStatus(pcsCharacter, AGPDCHAR_SPECIAL_STATUS_TRANSPARENT);

			// 변신중이면 원래로 되돌린다.
			if(pcsCharacter->m_bIsTrasform)
				m_pAgpmCharacter->RestoreTransformCharacter(pcsCharacter);
	
		}
		// 쉬라인 지역에 있다가 나간사람
		else if(m_pAgpmShrineBattle->IsInShrineRegionPast(nPrevRegionIndex) && !m_pAgpmShrineBattle->IsInShrineRegion(pcsCharacter))
		{
			SendPacketShrineTimebarStatusToCharacter(iter2, FALSE, pcsCharacter);
		}
	}

	return TRUE;
}

BOOL AgsmShrineBattle::Death(AgpdCharacter *pcsCharacter, ApBase* pcsAttackerBase)
{
	if(!pcsCharacter)
		return FALSE;
	// 쉬라인 지역이 아니면 처리하지 않는다.
	if (FALSE == m_pAgpmShrineBattle->IsInShrineRegion(pcsCharacter))
		return FALSE;

	
	IterShrinePoint iter  = FindShrinePointInCharacter(pcsCharacter);
	// 쉬라인이 각인시도, 각인, 점령시도, 점렴 시가 아니면 처리하지 않는다.
	if (iter->m_nCurrentStatus < AGPDSHRINEBATTLE_STATUS_TRYIMPRINT && iter->m_nCurrentStatus > AGPDSHRINEBATTLE_STATUS_OCCUPY)
		return TRUE;

	for(IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		IterShrinePoint iter2 = m_pAgpmShrineBattle->m_stShrineBattle.FindByShrinePointID(iter->m_nShrinePointID);

		if(iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT ||iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_IMPRINT ||  iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYOCCUPY)
		{
			INT32 nPrevStatus = iter2->m_nCurrentStatus;

			if(pcsCharacter->m_lID == iter2->m_nOccupyCharacterCid)
			{
				if(iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT || iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_IMPRINT)
					SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT_FAIL, iter2->m_nShrinePointID, iter2->m_nTryOccupyRace, iter2->m_nOccupyCharacterCid);
				else
					SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_OCCUPY_FAIL, iter2->m_nShrinePointID, iter2->m_nTryOccupyRace, iter2->m_nOccupyCharacterCid);

				// 죽인 캐릭터가 있는경우, 점령권한을 옮겨주기위해 캐릭터 락걸기
				AuAutoLock Lock;
				if ( pcsAttackerBase )
				{
					Lock.SetMutualEx( pcsAttackerBase->m_Mutex );
					Lock.Lock();
					if ( !Lock.Result() )
					{
						// 이런 경우는 있으면 안되겠지만 혹시나...
						char strCharBuff[256] = { 0, };
						sprintf_s(strCharBuff, sizeof(strCharBuff), "[ERROR] AgsmShrineBattle::Death() : Failed Lock.Result()" );
						AuLogFile_s(LOG_ERROR_AGSM_SHRINE_BATTLE, strCharBuff);

						// 락 못걸면 점령권한 옮겨줌과 동시에 해당 캐릭터가 죽어버리면, 죽은 상태로 점령이 진행되는 문제가 발생할수 있음
					}
				}

				BOOL bStatusOpen = TRUE;
				if((iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYOCCUPY) && pcsAttackerBase) // 죽인 사람이 존재할때...
				{
					if(APBASE_TYPE_CHARACTER == pcsAttackerBase->m_eType && m_pAgpmCharacter->IsPC((AgpdCharacter*)pcsAttackerBase))
					{
						bStatusOpen = FALSE;

						// OccupyCharacter 정보 갱신
						iter2->m_nOccupyCharacterCid = pcsAttackerBase->m_lID;
						strncpy_s(iter2->m_szOccupyCharacterName, sizeof(iter2->m_szOccupyCharacterName), ((AgpdCharacter*)pcsAttackerBase)->m_szID, sizeof(iter2->m_szOccupyCharacterName));
						INT32 nRace = m_pAgpmCharacter->GetCharacterRace((AgpdCharacter*)pcsAttackerBase).detail.nRace;
						iter2->m_nTryOccupyRace = nRace;
					}
					else
					{
						iter2->m_nOccupyCharacterCid = AP_INVALID_CID;
						ZeroMemory(iter2->m_szOccupyCharacterName, sizeof(iter2->m_szOccupyCharacterName));
						iter2->m_nTryOccupyRace = AURACE_TYPE_NONE;
					}
				}

				SendPacketShrineTimebarStatusToRegion(iter2, FALSE, iter->m_nRegionIndex);

				if(bStatusOpen)
				{
					//StatusChange(iter2, AGPDSHRINEBATTLE_STATUS_OPEN, GetClockCount(), FALSE);
					StatusChange(iter2, AGPDSHRINEBATTLE_STATUS_OPEN, iter2->m_nBattleOpenTime , FALSE);//JK_쉬라인머지 점령실패시 시간 초기화 하던 현상 수정
					SendPacketShrineBattleImprintResult(iter2, pcsCharacter, nPrevStatus, FALSE);
				}
				else
				{
					StatusChange(iter2, AGPDSHRINEBATTLE_STATUS_TRYOCCUPY, GetClockCount(), TRUE);

					SendPacketShrineTimebarStatusToRegion(iter2, TRUE, iter->m_nRegionIndex);

					// 실패했다고 결과를 보내주고
					SendPacketShrineBattleImprintResult(iter2, pcsCharacter, nPrevStatus, FALSE);

					// 새로운 캐릭터가 시도한다고 보내준다.
					SendPacketShrineBattleImprintResult(iter2, (AgpdCharacter*)pcsAttackerBase, AGPDSHRINEBATTLE_STATUS_TRYOCCUPY, TRUE);
				}
			}
		}

		if(pcsCharacter->m_nBindingRegionIndex == iter->m_nRegionIndex)
			SendPacketShrineBattleInformationToCharacter(iter2, pcsCharacter);
	}

	return TRUE;
}

BOOL AgsmShrineBattle::Move(AgpdCharacter *pcsCharacter)
{
	if (NULL == pcsCharacter)
		return FALSE;

	if(FALSE == m_pAgpmShrineBattle->IsInShrineRegion(pcsCharacter))
		return TRUE;

	for(IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		IterShrinePoint iter2 = m_pAgpmShrineBattle->m_stShrineBattle.FindByShrinePointID(iter->m_nShrinePointID);

		if(iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT)
		{
			INT32 nPrevStatus = iter2->m_nCurrentStatus;

			if(pcsCharacter->m_lID == iter2->m_nOccupyCharacterCid)
			{
				SendPacketShrineBattleImprintResult(iter2, pcsCharacter, nPrevStatus, FALSE);

				// 실패라고 말해준다.
				SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT_FAIL, iter2->m_nShrinePointID, iter2->m_nTryOccupyRace, iter2->m_nOccupyCharacterCid);

				StatusChange(iter2, AGPDSHRINEBATTLE_STATUS_OPEN, iter2->m_nBattleOpenTime, FALSE);

				iter2->m_nOccupyCharacterCid = AP_INVALID_CID;
				ZeroMemory(iter2->m_szOccupyCharacterName, sizeof(iter2->m_szOccupyCharacterName));
			}
		}
	}
	
	return TRUE;
}

BOOL AgsmShrineBattle::RemoveCharacter(AgpdCharacter *pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	// 리전변경시 이루어져야 될 것을 한다.
	// 각인자, 점령시도중인 캐릭터가 지역을 빠져나가면 status와 charater 상태를 초기화

	for(IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		IterShrinePoint iter2 = m_pAgpmShrineBattle->m_stShrineBattle.FindByShrinePointID(iter->m_nShrinePointID);

		if(iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT || iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYOCCUPY)
		{
			INT32 nPrevStatus = iter2->m_nCurrentStatus;

			if(pcsCharacter->m_lID == iter2->m_nOccupyCharacterCid && pcsCharacter->m_nBindingRegionIndex == iter->m_nRegionIndex)
			{
				if(iter2->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT)
					SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT_FAIL, iter2->m_nShrinePointID, iter2->m_nTryOccupyRace, iter2->m_nOccupyCharacterCid);
				else
				{
					SendPacketShrineBattleMessageCodeToShrineArea(AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_OCCUPY_FAIL, iter2->m_nShrinePointID, iter2->m_nTryOccupyRace, iter2->m_nOccupyCharacterCid);
					SendPacketShrineTimebarStatusToRegion(iter2, FALSE, iter->m_nRegionIndex);
				}

				StatusChange(iter2, AGPDSHRINEBATTLE_STATUS_OPEN, GetClockCount(), FALSE);

				SendPacketShrineBattleImprintResult(iter2, pcsCharacter, nPrevStatus, FALSE);
			}
		}
	}

	return TRUE;
}

BOOL AgsmShrineBattle::ShrineBattleTryImprint(AgpdCharacter *pcsCharacter, AgpdShrinePoint* pstShrinePoint)
{
	if(!pcsCharacter || !pstShrinePoint)
		return FALSE;

	AuAutoLock Lock(pcsCharacter->m_Mutex);
	if(!Lock.Result())
		return FALSE;

	if(pstShrinePoint->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYIMPRINT || pstShrinePoint->m_nCurrentStatus == AGPDSHRINEBATTLE_STATUS_TRYOCCUPY)
		return FALSE;

	// Distance Check
	if(FALSE == m_pAgpmShrineBattle->IsInTryImprintRange(pstShrinePoint, pcsCharacter))
		return FALSE;

	// Moving Check
	if(TRUE == pcsCharacter->m_bMove)
		return FALSE;

	if(pstShrinePoint->m_nOccupyCharacterCid != AP_INVALID_CID)
		return SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, pstShrinePoint->m_nCurrentStatus, FALSE);

	if(pstShrinePoint->m_nCurrentStatus != AGPDSHRINEBATTLE_STATUS_OPEN)
		return SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, pstShrinePoint->m_nCurrentStatus, FALSE);

	if (m_pAgpmCharacter->IsAllBlockStatus(pcsCharacter))
		return SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, pstShrinePoint->m_nCurrentStatus, FALSE);

	//Todo
	//pcsCharacter를 block해야하는데 어디서 할지...

	pstShrinePoint->m_nOccupyCharacterCid = pcsCharacter->m_lID;
	strncpy_s(pstShrinePoint->m_szOccupyCharacterName, sizeof(pstShrinePoint->m_szOccupyCharacterName), pcsCharacter->m_szID, sizeof(pstShrinePoint->m_szOccupyCharacterName));

	INT32 nRace = m_pAgpmCharacter->GetCharacterRace(pcsCharacter).detail.nRace;
	pstShrinePoint->m_nTryOccupyRace = nRace;

	SendPacketShrineBattleImprintResult(pstShrinePoint, pcsCharacter, pstShrinePoint->m_nCurrentStatus, TRUE);


	// 각인자가 무적상태이면 무적을 풀어준다.
	if(m_pAgpmPvP->IsInvincible(pcsCharacter))
		m_pAgsmPvP->UnsetInvincible(pcsCharacter);

	if(m_pAgpmCharacter->IsStatusInvincible(pcsCharacter))
		m_pAgpmCharacter->UpdateUnsetSpecialStatus(pcsCharacter, AGPDCHAR_SPECIAL_STATUS_INVINCIBLE);

	

	UINT32 ulClock = GetClockCount();

	StatusChange(pstShrinePoint, AGPDSHRINEBATTLE_STATUS_TRYIMPRINT, ulClock);

	return TRUE;
}

IterShrinePoint AgsmShrineBattle::FindShrinePointInCharacter(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return NULL;

	for(IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrineRegion.end(); ++iter)
	{
		IterShrinePoint iter2 = m_pAgpmShrineBattle->m_stShrineBattle.FindByShrinePointID(iter->m_nShrinePointID);

		if(iter->m_nRegionIndex == pcsCharacter->m_nBindingRegionIndex)
			return iter2;
	}

	return NULL;
}

IterShrinePoint AgsmShrineBattle::FindShrinePointSameRegion(AgpdCharacter* pcsCharacter)
{
	if(NULL == pcsCharacter)
		return NULL;

	for(IterShrinePoint iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
	{
		AgpdCharacter *pcsShrineCharacter = m_pAgpmCharacter->GetCharacter(iter->m_nShrineObjectCID);
		if(NULL == pcsShrineCharacter)
			continue;

		if(pcsShrineCharacter->m_nBindingRegionIndex == pcsCharacter->m_nBindingRegionIndex)
			return iter;
	}

	return NULL;
}


BOOL AgsmShrineBattle::SpawnShrineObject(AgpdShrinePoint *pstShrinePoint)
{
	if(NULL == pstShrinePoint)
		return FALSE;

	if(TRUE == pstShrinePoint->m_bSpawnObject)
		return FALSE;

	AgpdSpawnGroup *	pstGroup;
	INT32				lIndex = 0;

	UINT32				ulClockCount	= GetClockCount();

	for (pstGroup = m_pAgpmEventSpawn->GetGroupSequence(&lIndex);
		pstGroup;
		pstGroup = m_pAgpmEventSpawn->GetGroupSequence(&lIndex))
	{
		for (ApdEvent	*pstEvent = pstGroup->m_pstChild; pstEvent; pstEvent = ((AgpdSpawn *) pstEvent->m_pvData)->m_pstNext)
		{
			if (!pstEvent->m_pvData)
				break;

			AgpdSpawn *pstSpawnPrime = (AgpdSpawn*)pstEvent->m_pvData;
			if(!pstSpawnPrime)
				break;

			// 쉬라인 타입
			if (AGPDSPAWN_SHRINE_OBJECT == ((AgpdSpawn *) pstEvent->m_pvData)->m_lSiegeWarType)
			{
				AuMATRIX	*pstDirection	= NULL;;
				AuPOS		stSpawnPos	= *m_pAgpmEventManager->GetBasePos(pstEvent->m_pcsSource, &pstDirection);

				IterShrineRegion iter = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
				if(!iter)
					return FALSE;

				if(iter->m_nRegionIndex == m_pApmMap->GetRegion(stSpawnPos.x, stSpawnPos.z))
					m_pAgsmEventSpawn->ProcessSpawn(pstEvent, ulClockCount);

				pstShrinePoint->m_bSpawnObject = TRUE;
			}
		}
	}

	return TRUE;
}

BOOL AgsmShrineBattle::SettingShrineObject(AgpdCharacter *pcsShrineCharacter)
{
	if (NULL == pcsShrineCharacter)
		return FALSE;

	if(FALSE == m_pAgpmCharacter->IsShrineTypifier(pcsShrineCharacter))
		return FALSE;

	BOOL bPass = TRUE;

	IterShrinePoint iter = FindShrinePointInCharacter(pcsShrineCharacter); 
	if (iter != NULL)
	{
		iter->m_nShrineObjectCID = pcsShrineCharacter->m_lID;
	}
	else 
		return FALSE;

	iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin();
	while(iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end())
	{
		if(iter->m_nShrineObjectCID == AP_INVALID_CID)
		{
			bPass = FALSE;
			break;
		}

		++iter;
	}

	m_bLoadComplete = bPass;

	return TRUE;
}

BOOL AgsmShrineBattle::ShuffleShrineObjectSetting()
{
	vector<INT32> vShrineObjectCID;

	for(IterShrinePoint iter =m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
	{
		// Vector에 집어넣고
		vShrineObjectCID.push_back(iter->m_nShrineObjectCID);
	}

	vector<INT32>::iterator Iter;
	for(Iter = vShrineObjectCID.begin(); Iter != vShrineObjectCID.end(); Iter++)
	{
		AgpdCharacter *pcsShrineObject = m_pAgpmCharacter->GetCharacter(*Iter);
		if(NULL != pcsShrineObject)
		{
			SettingShrineObject(pcsShrineObject);
		}
	}

	return TRUE;
}

BOOL AgsmShrineBattle::ShuffleShrineObjectSetting(AgpdShrinePoint *pstShrinePoint)
{
	if(NULL == pstShrinePoint)
		return FALSE;

	IterShrineRegion Iter = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pstShrinePoint->m_nShrinePointID);
	if(Iter)
	{
		AgpdCharacter *pcsShrineObject = GetShrineObjectInRegion(Iter->m_nRegionIndex);
		if(pcsShrineObject)
		{
			if(FALSE == m_pAgpmCharacter->IsShrineTypifier(pcsShrineObject))
				return FALSE;

			pstShrinePoint->m_nShrineObjectCID = pcsShrineObject->m_lID;
		}
	}

	return TRUE;
}

AgpdCharacter* AgsmShrineBattle::GetShrineObjectInRegion(INT32 lRegionIndex)
{
	AgpdCharacter *pcsCharacter = NULL;

	for(IterShrinePoint iter = m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
	{
		pcsCharacter = m_pAgpmCharacter->GetCharacter(iter->m_nShrineObjectCID);
		if(pcsCharacter)
		{
			if(FALSE == m_pAgpmCharacter->IsShrineTypifier(pcsCharacter))
				return NULL;

			if(lRegionIndex == pcsCharacter->m_nBindingRegionIndex)
				break;
		}
	}

	return pcsCharacter;
}

INT32 AgsmShrineBattle::GetPlayerCountPerRace(AgpdShrineRegion* pcsShrineRegion, INT32 nRace )
{
	if(!pcsShrineRegion) return 0;

	INT32 nPlayerCount = 0;

	ApAdmin* pcsRegionCharacterCID = m_pAgsmAreaChatting->GetRegionAdmin();


	UINT32	*pnCID = NULL;

	for(int i = 0; i < MAX_SHRINE_HAS_REGEION_NUM; ++i)
	{
		INT32	lIndex	= 0;
		pnCID	= (UINT32 *)pcsRegionCharacterCID[pcsShrineRegion->m_arrayRegionIndex[i]].GetObjectSequence(&lIndex);

		while(pnCID)
		{
			AgpdCharacter *pCharacter = m_pAgpmCharacter->GetCharacter(*pnCID);
			if(pCharacter)
			{
				INT16 nPlayerRace = m_pAgpmCharacter->GetCharacterRace(pCharacter).detail.nRace;
				ASSERT(nPlayerRace < AURACE_TYPE_MAX);

				if(nPlayerRace == nRace)
					nPlayerCount++;
			}

			pnCID	= (UINT32 *)pcsRegionCharacterCID[pcsShrineRegion->m_arrayRegionIndex[i]].GetObjectSequence(&lIndex);
		}
	}

	switch(nRace)
	{
	case AURACE_TYPE_HUMAN:
		pcsShrineRegion->m_nHumanPlayerCount = nPlayerCount;
		break;
	case AURACE_TYPE_ORC:
		pcsShrineRegion->m_nOrcPlayerCount = nPlayerCount;
		break;
	case AURACE_TYPE_MOONELF:
		pcsShrineRegion->m_nMoonElfPlayerCount = nPlayerCount;
		break;
	case AURACE_TYPE_DRAGONSCION:
		pcsShrineRegion->m_nScionPlayerCount = nPlayerCount;
		break;
	default:
		break;
	}

	return nPlayerCount;

}
BOOL AgsmShrineBattle::ApplyReward(AgpdCharacter *pcsCharacter, AgpdCharacter *pcsShrineCharacter, AgsmShrineRewardType eType)
{
	if(NULL == pcsCharacter || NULL == pcsShrineCharacter)
		return FALSE;

	std::map<INT32, AgpdShrineReward::ItemInfo>		TempItemMap;
	std::map<INT32, AgpdShrineReward::SkillVector>	TempSkillListMap;
	
	INT32 lCharismaPoint = 0;

	switch (eType)
	{
	case AGSM_SHRINE_REWARD_COMMON:
		{
			TempItemMap			= m_pAgpmShrineBattle->m_stShrineReward.m_stCommon.m_mapItem;
			TempSkillListMap	= m_pAgpmShrineBattle->m_stShrineReward.m_stCommon.m_mapSkillList;
			lCharismaPoint		= m_pAgpmShrineBattle->m_stShrineReward.m_stCommon.m_nCharismaPoint;
		} break;

	case AGSM_SHRINE_REWARD_WIN_RACE:
		{
			TempItemMap			= m_pAgpmShrineBattle->m_stShrineReward.m_stWinRace.m_mapItem;
			TempSkillListMap	= m_pAgpmShrineBattle->m_stShrineReward.m_stWinRace.m_mapSkillList;
			lCharismaPoint		= m_pAgpmShrineBattle->m_stShrineReward.m_stWinRace.m_nCharismaPoint;
		} break;

	case AGSM_SHRINE_REWARD_CARVING:
		{
			TempItemMap			= m_pAgpmShrineBattle->m_stShrineReward.m_stCarvingCharacter.m_mapItem;
			TempSkillListMap	= m_pAgpmShrineBattle->m_stShrineReward.m_stCarvingCharacter.m_mapSkillList;
			lCharismaPoint		= m_pAgpmShrineBattle->m_stShrineReward.m_stCarvingCharacter.m_nCharismaPoint;
		} break;
	}

	// 1. 카리스마 포인트 지급
	m_pAgpmCharacter->AddCharismaPoint(pcsCharacter, lCharismaPoint);

	m_pAgsmPvP->SendSystemMessage(AGPMPVP_SYSTEM_CODE_CHARISMA_UP, m_pAgsmCharacter->GetCharDPNID(pcsCharacter), NULL, NULL, lCharismaPoint, 1);

	// 2. 아이템 지급
	std::map<INT32, AgpdShrineReward::ItemInfo>::iterator ItemIter = TempItemMap.begin();
	while (ItemIter != TempItemMap.end())
	{
		INT32 lItemTID							= ItemIter->first;
		AgpdShrineReward::ItemInfo stItemInfo	= ItemIter->second;

		AgpdItemTemplate* pItemTemplate			= m_pAgpmItem->GetItemTemplate(lItemTID);
		if (!pItemTemplate)
		{
			ItemIter++;
			continue;
		}

		AgpdItem* pcsItem = NULL;
		if (pItemTemplate->m_bStackable)
			pcsItem = m_pAgsmItemManager->CreateItem(pItemTemplate->m_lID, pcsCharacter, stItemInfo.m_ulCount);
		else
			pcsItem = m_pAgsmItemManager->CreateItem(pItemTemplate->m_lID, pcsCharacter);

		if(!pcsItem)
		{
			ItemIter++;
			continue;
		}

		// 귀속아이템이라면 귀속과 주인을 세팅해준다. 2005.11.02. steeple
		if(m_pAgpmItem->GetBoundType(pcsItem->m_pcsItemTemplate) == E_AGPMITEM_BIND_ON_ACQUIRE)
		{
			m_pAgpmItem->SetBoundType(pcsItem, E_AGPMITEM_BIND_ON_ACQUIRE);
			m_pAgpmItem->SetBoundOnOwner(pcsItem, pcsCharacter);
		}

		// Skill Plus 해준다. 2007.02.05. steeple
		m_pAgsmItem->ProcessItemSkillPlus(pcsItem, pcsCharacter);

		if(stItemInfo.m_bUseTimeLimit)
		{
			UINT32 lCurrentTimeStamp	= AuTimeStamp::GetCurrentTimeStamp();
			UINT32 lExpireTimeStamp		= AuTimeStamp::AddTime(lCurrentTimeStamp, 0, 0, 0, stItemInfo.m_ulTimeLimit);
			pcsItem->m_lExpireTime		= lExpireTimeStamp;

			AgsdItemADChar* pcsItemADChar = m_pAgsmItem->GetADCharacter(pcsCharacter);
			if( pcsItemADChar )
				pcsItemADChar->m_bUseTimeLimitItem = TRUE;

			m_pAgsmCharacter->SetProcessTime(pcsCharacter, AGSDCHAR_IDLE_TYPE_ITEM, GetClockCount());
			m_pAgsmCharacter->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_ITEM, AGSDCHAR_IDLE_INTERVAL_200_MS);
		}

		m_pAgpmItem->AddItemToInventory(pcsCharacter, pcsItem);

		m_pAgsmItem->WriteItemLog(AGPDLOGTYPE_ITEM_SHRINE_GIVE, pcsCharacter->m_lID,
			pcsItem,
			pcsItem->m_nCount ? pcsItem->m_nCount : 1
			);

		m_pAgsmItem->SendPacketPickupItemResult(
			AGPMITEM_PACKET_PICKUP_ITEM_RESULT_SUCCESS, 
			(pcsItem) ? pcsItem->m_lID : AP_INVALID_IID, 
			pItemTemplate->m_lID, 
			(pcsItem->m_nCount) ? pcsItem->m_nCount : 1, 
			m_pAgsmCharacter->GetCharDPNID(pcsCharacter));

		ItemIter++;
	}

	// 3. 스킬 지급 
	std::map<INT32, AgpdShrineReward::SkillVector>::iterator SkillIter = TempSkillListMap.find(pcsShrineCharacter->m_pcsCharacterTemplate->m_lID);
	if(SkillIter != TempSkillListMap.end())
	{
		for(vector<INT32>::iterator iter = SkillIter->second.begin(); iter != SkillIter->second.end(); ++iter)
		{
			AgpdSkillTemplate* pcsSkillTemplate = m_pAgpmSkill->GetSkillTemplate(*iter);
			if(!pcsSkillTemplate)
				continue;

			INT32 lSkillLevel = 1;

			m_pAgsmSkill->CastSkill(pcsCharacter, *iter, lSkillLevel, NULL, FALSE, FALSE);
		}
	}
	
	return TRUE;
}

BOOL AgsmShrineBattle::CBEnterGameWorld(PVOID pData, PVOID pClass, PVOID pCustData)
{
/*	if(!pData || !pClass)
		return FALSE;


	AgsmShrineBattle *pThis		=	(AgsmShrineBattle *)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;

	if (FALSE == pThis->m_pAgpmCharacter->IsPC(pcsCharacter))
		return TRUE;*/

	//IterShrinePoint iter;
	//for(iter = pThis->m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iter != pThis->m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
	//{

	//	if(iter->m_nCurrentStatus >= AGPDSHRINEBATTLE_STATUS_OPEN && iter->m_nCurrentStatus < AGPDSHRINEBATTLE_STATUS_END)
	//	{
	//		pThis->m_pAgsmWarInfo->SendLastInfoForChar(pcsCharacter);
	//		return TRUE;
	//	}
	//}
	return TRUE;
}

BOOL AgsmShrineBattle::CBTeleportLoading(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;
	AgsmShrineBattle *pThis		=	(AgsmShrineBattle *)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	
	if(pThis->m_pAgpmShrineBattle->IsInShrineRegion(pcsCharacter->m_nBindingRegionIndex) == TRUE)
	{
		pThis->m_pAgsmWarInfo->SendEndWarInfoToCharacter(ShrineBattle, pcsCharacter);
	}
	return TRUE;
}


BOOL AgsmShrineBattle::PvPDeath(AgpdCharacter *pWinCharacter, AgpdCharacter *pLoseCharacter)
{
	if(!pWinCharacter || !pLoseCharacter) return FALSE;

	//PvP Result Process

	// 쉬라인 지역이 아니면 처리하지 않는다.
	if (FALSE == m_pAgpmShrineBattle->IsInShrineRegion(pWinCharacter) || 
		FALSE == m_pAgpmShrineBattle->IsInShrineRegion(pLoseCharacter))
		return FALSE;
	
	


	// 쉬라인이 열리고 있지 않으면 처리하지 않는다.
	IterShrinePoint iter  = FindShrinePointInCharacter(pWinCharacter);
	IterShrinePoint iter2 = FindShrinePointInCharacter(pLoseCharacter);

	if (!(iter->m_nCurrentStatus >= AGPDSHRINEBATTLE_STATUS_OPEN && iter->m_nCurrentStatus < AGPDSHRINEBATTLE_STATUS_END) || 
		!(iter2->m_nCurrentStatus >= AGPDSHRINEBATTLE_STATUS_OPEN && iter2->m_nCurrentStatus < AGPDSHRINEBATTLE_STATUS_END))
	{
		return TRUE;
	}

	m_pAgsmWarInfo->ProcessPvPResult(ShrineBattle, (AgpdCharacter *)pWinCharacter, pLoseCharacter);

	if ( m_pAgsmPvP->IsEnableUpdateOfRacePvPResult(pWinCharacter, pLoseCharacter) )
	{
		AgpdPvPADChar* pcsAttachedPvP1 = m_pAgpmPvP->GetADCharacter(pWinCharacter);
		AgpdPvPADChar* pcsAttachedPvP2 = m_pAgpmPvP->GetADCharacter(pLoseCharacter);

		// 이긴놈에게 승리를 언져주고 진놈에겐 패배를 언져주자
		pcsAttachedPvP1->m_lWin++;
		pcsAttachedPvP2->m_lLose++;

		// PvP Result Process
		m_pAgpmPvP->AddRaceWinResult(pWinCharacter, (AuRaceType)m_pAgpmCharacter->GetCharacterRace(pLoseCharacter).detail.nRace);
		m_pAgpmPvP->AddRaceLoseResult(pLoseCharacter, (AuRaceType)m_pAgpmCharacter->GetCharacterRace(pWinCharacter).detail.nRace);

		// PvP Result Update DB
		m_pAgsmPvP->SendPvPResultToDB(pWinCharacter);
		m_pAgsmPvP->SendPvPResultToDB(pLoseCharacter);

		// 결과 패킷을 보내준다.
		m_pAgsmPvP->SendPvPResult(pWinCharacter, pLoseCharacter);

		m_pAgsmPvP->UpdatePvPStatus(pWinCharacter);
		m_pAgsmPvP->UpdatePvPStatus(pLoseCharacter);

		// 길드, 파티 처리
		m_pAgsmPvP->NotifyDeadToFriendGuildMembers(pWinCharacter, pLoseCharacter);
		m_pAgsmPvP->NotifyDeadToPartyMembers(pWinCharacter, pLoseCharacter);
	}

	IterShrineRegion iterRegion = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(iter->m_nShrinePointID);
	if(!iterRegion)
		return FALSE;

	INT16 nRace = m_pAgpmCharacter->GetCharacterRace(pWinCharacter).detail.nRace;
	ASSERT(nRace < AURACE_TYPE_MAX);

	iterRegion->m_arrayRaceScore[nRace].IncreaseWinScore();

	nRace = m_pAgpmCharacter->GetCharacterRace(pLoseCharacter).detail.nRace;
	ASSERT(nRace < AURACE_TYPE_MAX);
	iterRegion->m_arrayRaceScore[nRace].IncreaseLoseScore();


	return TRUE;
}

BOOL AgsmShrineBattle::CheckRacePvPSend(UINT32 ulClockCount)
{
	if(m_ulLastRacePvPSendTime + RACE_PVP_SEND_TERM_MIN * 1000 * 60 < ulClockCount)
	{
		m_ulLastRacePvPSendTime = ulClockCount;

		IterShrinePoint iter;

		BOOL bAllEnd = TRUE;

		for(iter =m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
		{
			if(iter->m_nCurrentStatus > AGPDSHRINEBATTLE_STATUS_NORMAL || iter->m_nCurrentStatus < AGPDSHRINEBATTLE_STATUS_END)
			{
				SendPacketRacePvPInfoToShrineRegion(iter);
			}
		}
	}
	return TRUE;
}


BOOL AgsmShrineBattle::SendShrineWarInfo(AgpdShrinePoint *pcsShrinePoint, INT8 cWarStep, INT8 cMessageStep)
{
	PACKET_WAR_INFO_STATE packetWarInfoState(ShrineBattle, cWarStep, cMessageStep);
	SendPacketInShrineRegion(pcsShrinePoint, packetWarInfoState);
	return TRUE;
}

BOOL AgsmShrineBattle::SendEndWar(AgpdShrinePoint *pcsShrinePoint)
{
	PACKET_WAR_INFO_WAR_END_INFO packetEndWar(ShrineBattle);
	SendPacketInShrineRegion(pcsShrinePoint, packetEndWar);
	return TRUE;
}


BOOL AgsmShrineBattle::SendPacketInShrineRegion(AgpdShrinePoint *pcsShrinePoint, PACKET_HEADER& pPacket)
{
	ASSERT(pcsShrinePoint);

	ApAdmin* pcsRegionCharacterCID = m_pAgsmAreaChatting->GetRegionAdmin();

	IterShrineRegion iterRegion = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pcsShrinePoint->m_nShrinePointID);
	
	ASSERT(iterRegion);

	UINT32	*pnCID = NULL;

	for(int i = 0; i < MAX_SHRINE_HAS_REGEION_NUM; ++i)
	{
		INT32	lIndex	= 0;
		pnCID	= (UINT32 *)pcsRegionCharacterCID[iterRegion->m_arrayRegionIndex[i]].GetObjectSequence(&lIndex);

		while(pnCID)
		{
			DPNID npNid = m_pAgsmCharacter->GetCharDPNID(*pnCID);

			AgsEngine::GetInstance()->SendPacket(pPacket, npNid);

			pnCID	= (UINT32 *)pcsRegionCharacterCID[iterRegion->m_arrayRegionIndex[i]].GetObjectSequence(&lIndex);
		}
	}
	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketRacePvPInfoToShrineRegion(AgpdShrinePoint *pcsShrinePoint)
{
	if(!pcsShrinePoint) return TRUE;

	ApAdmin* pcsRegionCharacterCID = m_pAgsmAreaChatting->GetRegionAdmin();

	IterShrineRegion iterRegion = m_pAgpmShrineBattle->m_stShrineBattle.FindRegionByShrinePointID(pcsShrinePoint->m_nShrinePointID);
	if(!iterRegion)
		return FALSE;

	
	UINT32	*pnCID = NULL;

	for(int i = 0; i < MAX_SHRINE_HAS_REGEION_NUM; ++i)
	{
		INT32	lIndex	= 0;
		pnCID	= (UINT32 *)pcsRegionCharacterCID[iterRegion->m_arrayRegionIndex[i]].GetObjectSequence(&lIndex);

		while(pnCID)
		{
			AgpdCharacter *pCharacter = m_pAgpmCharacter->GetCharacter(*pnCID);
			if(pCharacter)
			{
				INT16 nRace = m_pAgpmCharacter->GetCharacterRace(pCharacter).detail.nRace;
				ASSERT(nRace < AURACE_TYPE_MAX);

				DPNID npNid = m_pAgsmCharacter->GetCharDPNID(*pnCID);

				PACKET_WAR_INFO_RACE_PVP_INFO packet(ShrineBattle, iterRegion->m_arrayRaceScore[nRace].m_lWinScore, iterRegion->m_arrayRaceScore[nRace].m_lLoseScore);
				AgsEngine::GetInstance()->SendPacket(packet, npNid);
			}

			pnCID	= (UINT32 *)pcsRegionCharacterCID[iterRegion->m_arrayRegionIndex[i]].GetObjectSequence(&lIndex);
		}
	}
	return TRUE;

}

BOOL AgsmShrineBattle::SendPacketToShrineAll(PACKET_HEADER& pPacket)
{

	IterShrinePoint iter;

	for(iter =m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.begin(); iter != m_pAgpmShrineBattle->m_stShrineBattle.m_vtShrinePoint.end(); ++iter)
	{
		SendPacketInShrineRegion(iter, pPacket);
	}
	return TRUE;
}

BOOL AgsmShrineBattle::SendPacketRemainTime(AgpdShrinePoint *pstShrinePoint, INT8 cTime)
{
	PACKET_WAR_INFO_TIME packetTimeStep(ShrineBattle, cTime);
	SendPacketInShrineRegion(pstShrinePoint, packetTimeStep);
	return TRUE;
}