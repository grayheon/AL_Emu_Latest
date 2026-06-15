#include "AgsmEpicZone.h"
#include "StrUtil.h"
#include "AgpmEventSpawn.h"
#include "AgsmEventSpawn.h"
#include "AgsmBattleGround.h"
#include "AgpmCharacter.h"
#include "AgsmCharacter.h"
#include "AgsmPvP.h"
#include "AgpmEpicZone.h"
#include "ApmMap.h"
#include "AgppBattleGround.h"
#include "AgpmParty.h"

AgsmEpicZone* AgsmEpicZone::m_pInstance = NULL;

AgsmEpicZone::AgsmEpicZone()
	:m_ulClockCount(0), m_pLuaState(0)
{
	m_pInstance = this;
	m_Mutex.Init();

	SetModuleName("AgsmEpicZone");
	SetPacketType(AGPM_EPICZONE_PACKET_TYPE);

	m_pcsAgpmEventSpawn		= NULL;
	m_pcsApmEventManager	= NULL;
	m_pcsAgsmEventSpawn		= NULL;
	m_pcsAgsmBattleGroud	= NULL;
	m_pcsAgpmCharacter		= NULL;
	m_pcsAgpmEpicZone		= NULL;
	m_pcsAgsmCharacter		= NULL;
	m_pcsApmMap				= NULL;
	m_pcsAgpmParty			= NULL;
	m_pagsmPvP				= NULL;

	m_pLuaFileNode = NULL;
	m_pSpawnListNode = NULL;

	m_pLuaState = lua_open();
	luaL_openlibs(m_pLuaState);
}

AgsmEpicZone::~AgsmEpicZone()
{
	if(m_pLuaState)
		lua_close(m_pLuaState);
}

BOOL AgsmEpicZone::OnInit()
{
	if(FALSE == LuaRegister(m_pLuaState))
		return FALSE;

	if(FALSE == LoadXml())
		return FALSE;

	m_pcsAgpmEventSpawn		= (AgpmEventSpawn*)GetModule("AgpmEventSpawn");
	m_pcsApmEventManager	= (ApmEventManager*)GetModule("ApmEventManager");
	m_pcsAgsmEventSpawn		= (AgsmEventSpawn*)GetModule("AgsmEventSpawn");
	m_pcsAgsmBattleGroud	= (AgsmBattleGround*)GetModule("AgsmBattleGround");
	m_pcsAgpmCharacter		= (AgpmCharacter*)GetModule("AgpmCharacter");
	m_pcsAgpmEpicZone		= (AgpmEpicZone*)GetModule("AgpmEpicZone");
	m_pcsAgsmCharacter		= (AgsmCharacter*)GetModule("AgsmCharacter");
	m_pcsApmMap				= (ApmMap*)GetModule("ApmMap");
	m_pcsAgpmParty			= (AgpmParty*)GetModule("AgpmParty");
	m_pagsmPvP				= (AgsmPvP*)GetModule("AgsmPvP");

	if(!m_pcsAgpmEventSpawn || 
	   !m_pcsApmEventManager || !m_pcsAgsmEventSpawn ||
	   !m_pcsAgsmBattleGroud || !m_pcsAgpmCharacter ||
	   !m_pcsAgpmEpicZone || !m_pcsAgsmCharacter || !m_pcsApmMap || !m_pcsAgpmParty ||
	   !m_pagsmPvP) 
		return FALSE;

	return TRUE;
}

BOOL AgsmEpicZone::LoadXml()
{
	////////////////////////////////////////////////////////////////////////////
	// xml
	if(!m_XmlData.LoadFile("ini\\NPCManager\\EpicZone.xml"))
		return FALSE;

	AuXmlNode *pRootNode = m_XmlData.FirstChild("EpicZone");
	if(!pRootNode)
		return FALSE;

	// EpicZoneRaid관련 Node와 SpawnList Node는 저장해둔다.
	m_pLuaFileNode = pRootNode->FirstChild("EpicZoneRaid");
	m_pSpawnListNode = pRootNode->FirstChild("SpawnList");
	if(!m_pLuaFileNode || !m_pSpawnListNode)
		return FALSE;

	for(AuXmlNode* pNode = m_pLuaFileNode; pNode; pNode = pNode->NextSibling())
	{
		EpicBossZone stEpicZone;
		INT32		 nBossZoneID = 0;

		AuXmlElement *pElemFile  = pNode->FirstChildElement("LUAFile");
		if(pElemFile)
		{
			CHAR szFileName[MAX_PATH] = { 0, };
			CHAR *pFileName = (CHAR*)pElemFile->GetText();
			sprintf_s(szFileName, sizeof(szFileName), "ini\\Lua\\%s", pFileName);
			strcpy(stEpicZone.m_szLuaFileName, szFileName);
		
			AuXmlElement *pElemTime			= pNode->FirstChildElement("PlayTime") ;
			AuXmlElement *pElemZoneID		= pNode->FirstChildElement("ID");
			AuXmlElement *pElemRegionIndex  = pNode->FirstChildElement("RegionIndex");
	
			if(pElemTime && pElemZoneID && pElemRegionIndex)
			{
				stEpicZone.m_ulPlayTime = atoi(pElemTime->GetText()) * 1000;
				stEpicZone.m_ulRegionIndex = atoi(pElemRegionIndex->GetText());
				nBossZoneID = atoi(pElemZoneID->GetText());
				stEpicZone.m_ulBossZoneID = nBossZoneID;

				m_mapBossZone.insert(EpicBossZonePair(nBossZoneID, stEpicZone));
			}
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::OnTimer(UINT32 ulClockCount)
{
	INT_PTR GetOwningThreadID = m_Mutex.GetOwningThreadID();
	if(GetOwningThreadID != 0 && GetOwningThreadID != ::GetCurrentThreadId())
		return FALSE;

	AuAutoLock pLock(m_Mutex);
	if(!pLock.Result())
		return FALSE;

	///////////////////////////////////////////////////////////////////////////////

	if(ulClockCount < m_ulClockCount + 1000*60)
		return FALSE;

	m_ulClockCount = ulClockCount;

	EpicBossZoneIter iter = m_mapBossZone.begin();
	if(m_pLuaState)
	{
		while(iter != m_mapBossZone.end())
		{
			m_pLuaPack.Open(iter->second.m_szLuaFileName, (Decrypt_CFunction)StrUtil::Decrypt);
			if(!m_pLuaPack.dobuffer(m_pLuaState))
			{
				iter++;
				continue;
			}

			lua_getglobal(m_pLuaState, "OnTimer");
			int status = lua_pcall(m_pLuaState, 0, -1, 0);

			if(status != 0)
			{
				char strCharBuff[256] = { 0, };
				sprintf_s(strCharBuff, sizeof(strCharBuff), "%s", lua_tostring(m_pLuaState, -1));
				AuLogFile_s("Log\\LuaError.log", strCharBuff);
				fprintf(stderr, "\t%s\n", lua_tostring(m_pLuaState, -1));

				return FALSE;
			}

			iter++;
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::LuaRegister(lua_State* pLuaState)
{
	if(pLuaState)
	{
		lua_register(pLuaState, "GetEpicBossZoneState",				AgsmEpicZone::GetEpicBossZoneState);
		lua_register(pLuaState, "SetEpicBossZoneState",				AgsmEpicZone::SetEpicBossZoneState);
		lua_register(pLuaState, "IsEpicBossZoneTimeOver",			AgsmEpicZone::IsEpicBossZoneTimeOver);
		lua_register(pLuaState, "SpawnEpicZoneMonster",				AgsmEpicZone::SpawnEpicZoneMonster);
		lua_register(pLuaState, "RemoveMonster",					AgsmEpicZone::RemoveMonster);
		lua_register(pLuaState, "ReturnToEntry",					AgsmEpicZone::ReturnToEntry);
	}	

	return TRUE;
}

BOOL AgsmEpicZone::SetEpicBossZoneStartTime(UINT32 lBossZoneID, UINT32 lClockCount)
{
	if(lBossZoneID < 1)
		return FALSE;

	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter != m_mapBossZone.end())
	{
		if(iter->second.m_ulStartTime == 0)
		{
			iter->second.m_ulStartTime = lClockCount;
			iter->second.m_ulEndTime = iter->second.m_ulStartTime + iter->second.m_ulPlayTime;
			iter->second.m_eBossZoneState = EPICBOSSZONE_STATE_START;
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::IsEpicBossZoneTimeOver(UINT32 lBossZoneID)
{
	if(lBossZoneID < 1)
		return FALSE;

	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter != m_mapBossZone.end())
	{
		if(iter->second.m_ulStartTime == 0)
			return FALSE;

		if(iter->second.m_ulEndTime < GetClockCount())
			return TRUE;
	}

	return FALSE;
}

BOOL AgsmEpicZone::IsEnableEntrance(UINT32 lBossZoneID)
{
	if(lBossZoneID < 1)
		return FALSE;

	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter != m_mapBossZone.end())
	{
		if(iter->second.m_eBossZoneState != EPICBOSSZONE_STATE_NONE ||
			iter->second.m_ulStartTime != 0)
			return FALSE;
	}

	return TRUE;
}

BOOL AgsmEpicZone::InitEpicBossZone(UINT32 lBossZoneID)
{
	if(lBossZoneID < 1)
		return FALSE;

	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter != m_mapBossZone.end())
	{
		if(iter->second.m_ulStartTime == 0 && iter->second.m_eBossZoneState == EPICBOSSZONE_STATE_NONE)
		{
			return TRUE;
		}
		else
		{
			iter->second.m_ulStartTime = 0;
			iter->second.m_ulEndTime = 0;
			iter->second.m_eBossZoneState = EPICBOSSZONE_STATE_NONE;
			iter->second.m_pSpawnVector.clear();
			iter->second.m_pBossVector.clear();
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::LoadSpawnData(EpicBossZone *pstEpicBossZone)
{
	if(NULL == pstEpicBossZone)
		return FALSE;

	pstEpicBossZone->m_pSpawnVector.clear();
	pstEpicBossZone->m_pBossVector.clear();

	AuXmlNode* pNode = m_pSpawnListNode;
	if(!pNode)
		return FALSE;

	for(AuXmlNode* pElem = pNode->FirstChildElement("Group"); pElem; pElem = pElem->NextSiblingElement())
	{
		_CURRENT_SPAWN pSpawn;
		pSpawn.strSpawnGroupName = std::string("temp");
		pSpawn.pElem = pElem->FirstChildElement("SpawnGroup");

		if(!pSpawn.pElem)
			continue;

		CHAR* pBossZoneID = (CHAR*)pSpawn.pElem->Attribute("BossZoneID");
		if(!pBossZoneID) 
			continue;
		
		if(pstEpicBossZone->m_ulBossZoneID != atoi(pBossZoneID)) 
			continue;

		pstEpicBossZone->m_pSpawnVector.push_back(pSpawn);
		
		for(AuXmlElement *pElement = pSpawn.pElem; pElement; pElement = pElement->NextSiblingElement())
		{
			AuXmlElement* pBossElem = (pElement) ? pElement->FirstChildElement("Boss") : NULL;
			if(pBossElem)
			{
				_CURRENT_SPAWN pBossSpawn;
				pBossSpawn.strSpawnGroupName = pElement->Attribute("Name");
				pBossSpawn.pElem = pElement;
				pstEpicBossZone->m_pBossVector.push_back(pBossSpawn);
			}
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::LoadSpawnData(INT32 lBossZoneID)
{
	if(lBossZoneID < 1)
		return FALSE;

	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter == m_mapBossZone.end())
		return FALSE;
	
	return LoadSpawnData(&iter->second);
}

BOOL AgsmEpicZone::ProcessSpawn(CHAR *pstrGroupName)
{
	if(NULL == pstrGroupName)
		return FALSE;

	AgpdSpawnGroup *pcsSpawnGroup = m_pcsAgpmEventSpawn->GetGroup(pstrGroupName);
	if(NULL == pcsSpawnGroup)
		return FALSE;
	
	for(ApdEvent *pstEvent = pcsSpawnGroup->m_pstChild; pstEvent; pstEvent = ((AgpdSpawn *)pstEvent->m_pvData)->m_pstNext)
	{
		AgpdSpawn *pstSpawn = (AgpdSpawn*)pstEvent->m_pvData;
		if(!pstSpawn)
			continue;

		for(INT32 nSpawnNum=0; nSpawnNum < pstSpawn->m_lTotalCharacter; ++nSpawnNum)
		{
			AuMATRIX	*pstDirection = NULL;
			AuPOS		stSpawnPos = *m_pcsApmEventManager->GetBasePos(pstEvent->m_pcsSource, &pstDirection);

			m_pcsAgsmEventSpawn->ProcessSpawn(pstEvent, 0);
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::RemoveMonster(INT32 lBossZoneID)
{
	if(lBossZoneID < 1)
		return FALSE;

	AgpdSpawnGroup*		pstGroup = NULL;
	INT32				lIndex = 0;
	
	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter != m_mapBossZone.end())
	{
		INT32 lRegionIndex = iter->second.m_ulRegionIndex;

		for(CSpawnVector::iterator it = iter->second.m_pSpawnVector.begin(); it != iter->second.m_pSpawnVector.end(); it++)
		{
			// 진행중인 SpawnGroup이 존재한다면 이쪽부터 정리를 시작해줘야한다. 
			CHAR* pPlaySpawnGroupName = (CHAR*)it->strSpawnGroupName.c_str();
			if(pPlaySpawnGroupName)
			{
				RemoveMonsterFromSpawnGroup(pPlaySpawnGroupName, lRegionIndex);
			}

			if(!it->pElem)
				continue;

			for(AuXmlElement *pElem = it->pElem; pElem; pElem = pElem->NextSiblingElement())
			{
				std::string strSpawnGroup = pElem->Attribute("Name");
				RemoveMonsterFromSpawnGroup((CHAR*)strSpawnGroup.c_str(), lRegionIndex);		
			}
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::SpawnEpicZoneMonster(INT32 lBossZoneID)
{
	if(lBossZoneID < 1)
		return FALSE;

	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter != m_mapBossZone.end())
	{
		SendRemainTime(&iter->second, GetRemainTime(lBossZoneID));

		for(CSpawnVector::iterator it = iter->second.m_pSpawnVector.begin(); it != iter->second.m_pSpawnVector.end(); it++)
		{
			if(it->pElem == NULL) continue;

			if(m_pcsAgsmBattleGroud->GetSpawnGroupCharCount((CHAR*)it->strSpawnGroupName.c_str()) == 0)
			{
				std::string strNextGroup = it->pElem->Attribute("Name");

				ProcessSpawn((CHAR*)strNextGroup.c_str());

				CHAR* szNotice = (CHAR*)it->pElem->Attribute("Notice");
				if(szNotice)
				{
					SendEpicZoneNotice(szNotice, (INT32)AGPM_BATTLEGROUND_NOTICE_EPICZONE, &iter->second);
				}

				it->pElem = it->pElem->NextSiblingElement();
				it->strSpawnGroupName = strNextGroup;
			}
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::ReturnToEntry(INT32 lBossZoneID, FLOAT PosX, FLOAT PosZ)
{
	if(lBossZoneID < 1)
		return FALSE;

	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter == m_mapBossZone.end())
		return FALSE;

	ApmMap::RegionTemplate *pcsEpicBossZoneRegion	= m_pcsApmMap->GetTemplate(iter->second.m_ulRegionIndex);
	if(!pcsEpicBossZoneRegion)
		return FALSE;

	INT32 lIndex = 0;
	AgpdCharacter *pcsCharacter = NULL;
	while(pcsCharacter = m_pcsAgpmCharacter->GetCharSequence(&lIndex))
	{
		if(!m_pcsAgpmCharacter->IsPC(pcsCharacter) || m_pcsAgsmCharacter->GetCharDPNID(pcsCharacter) == 0) continue;
		if(!m_pcsAgpmEpicZone->IsInEpicZone(pcsCharacter)) continue;

		ApmMap::RegionTemplate *pcsCharacterRegion = m_pcsApmMap->GetTemplate(pcsCharacter->m_nBindingRegionIndex);
		if(!pcsCharacterRegion) continue;

		if(m_pcsApmMap->IsSameRegionInvolveParent(pcsEpicBossZoneRegion, pcsCharacterRegion))
		{
			// 해당 캐릭터의 상태를 이동가능한 상태로 바꾸어준다.
			if(m_pcsAgpmCharacter->IsCombatMode(pcsCharacter))
			{
				m_pcsAgpmCharacter->ResetCombatMode(pcsCharacter);
			}

			m_pcsAgpmCharacter->UpdateActionStatus(pcsCharacter, AGPDCHAR_STATUS_NOT_ACTION);

			m_pcsAgsmCharacter->MoveCharacterPosition(pcsCharacter, PosX, PosZ);
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::OnDeadProcess(AgpdCharacter *pcsDeadCharacter, AgpdCharacter *pcsAttackCharacter)
{
	if(NULL == pcsDeadCharacter || NULL == pcsDeadCharacter->m_pcsCharacterTemplate)
		return FALSE;

	m_pagsmPvP->ProcessCharisma(pcsAttackCharacter, pcsDeadCharacter);

	if(pcsAttackCharacter)
	{
		if(!m_pcsAgpmEpicZone->IsInEpicZone(pcsAttackCharacter))
			return FALSE;
	}

	if(!m_pcsAgpmEpicZone->IsInEpicZone(pcsDeadCharacter))
		return FALSE;

	BOOL bInEpicBossZone = FALSE;

	EpicBossZoneIter iter = m_mapBossZone.begin();
	while(iter != m_mapBossZone.end())
	{	
		ApmMap::RegionTemplate *pcsEpicBossZoneRegion   = m_pcsApmMap->GetTemplate(iter->second.m_ulRegionIndex);
		ApmMap::RegionTemplate *pcsCharacterRegion		= m_pcsApmMap->GetTemplate(pcsDeadCharacter->m_nBindingRegionIndex);
		if(!pcsEpicBossZoneRegion || !pcsCharacterRegion)
		{
			continue;
			iter++;
		}

		if(m_pcsApmMap->IsSameRegionInvolveParent(pcsEpicBossZoneRegion, pcsCharacterRegion))
		{
			bInEpicBossZone = TRUE;
			break;
		}

		iter++;
	}

	if(bInEpicBossZone)
	{
		if(m_pcsAgpmCharacter->IsPC(pcsDeadCharacter) && pcsDeadCharacter->m_szID != '\0')
			return FALSE;

		AgpdSpawnADChar *pagpdSpawnADChar = m_pcsAgpmEventSpawn->GetCharacterData(pcsDeadCharacter);
		if(!pagpdSpawnADChar)
			return FALSE;

		if(!pagpdSpawnADChar->m_pstSpawn || !pagpdSpawnADChar->m_pstSpawn->m_pstParent)
			return FALSE;

		string strSpawnGroup(pagpdSpawnADChar->m_pstSpawn->m_pstParent->m_szName);

		for(CSpawnVector::iterator BossIter = iter->second.m_pBossVector.begin(); BossIter != iter->second.m_pBossVector.end(); BossIter++)
		{
			if(BossIter->pElem == NULL) continue;
			if(BossIter->strSpawnGroupName != strSpawnGroup) continue;

			AuXmlElement *pBossElem = BossIter->pElem->FirstChildElement("Boss");
			if(!pBossElem) continue;

			INT32 BossTID;
			pBossElem->Attribute("TID", &BossTID);

			if(BossTID != pcsDeadCharacter->m_pcsCharacterTemplate->m_lID) continue;

			CHAR *szNotice = (CHAR*)pBossElem->Attribute("KillNotice");
			if(szNotice)
			{
				SendEpicZoneNotice(szNotice, (INT32)AGPM_BATTLEGROUND_NOTICE_EPICZONE, &iter->second);
			}

			iter->second.m_ulEndTime = GetClockCount() + EPICBOSSZONEINTERVAL;
		}
	}

	return TRUE;
}

BOOL AgsmEpicZone::RemoveMonsterFromSpawnGroup(CHAR* szSpawnGroupName, INT32 lRegionIndex)
{
	if(NULL == szSpawnGroupName)
		return FALSE;

	ApmMap::RegionTemplate *pcsEpicBossZoneRegion	= m_pcsApmMap->GetTemplate(lRegionIndex);
	if(!pcsEpicBossZoneRegion)
		return FALSE;

	AgpdSpawnGroup*		pstGroup = NULL;

	pstGroup = m_pcsAgpmEventSpawn->GetGroup(szSpawnGroupName);
	if(!pstGroup)
		return FALSE;

	for(ApdEvent *pstEvent = pstGroup->m_pstChild; pstEvent; pstEvent = ((AgpdSpawn*)pstEvent->m_pvData)->m_pstNext)
	{
		AgpdSpawn* pSpawnData = (AgpdSpawn*)pstEvent->m_pvData;
		if(!pSpawnData)
			break;

		AgpdSpawnInfo stSpawnCharacters[AGPDSPAWN_MAX_SPAWNED_CHAR];
		memcpy(stSpawnCharacters, pSpawnData->m_stSpawnCharacters, sizeof(pSpawnData->m_stSpawnCharacters));

		for (int i = 0; i < pSpawnData->m_lTotalCharacter; ++i)
		{
			if (stSpawnCharacters[i].m_lID == AP_INVALID_CID)
				continue;

			AgpdCharacter *pcsCharacter = m_pcsAgpmCharacter->GetCharacter(stSpawnCharacters[i].m_lID);
			if(!pcsCharacter)
				continue;

			if(m_pcsAgpmEpicZone->IsInEpicZone(pcsCharacter) == FALSE) continue;

			ApmMap::RegionTemplate *pcsCharacterRegion = m_pcsApmMap->GetTemplate(pcsCharacter->m_nBindingRegionIndex);
			if(!pcsCharacterRegion ) continue;

			if(m_pcsApmMap->IsSameRegionInvolveParent(pcsEpicBossZoneRegion, pcsCharacterRegion)) 
			{						
				m_pcsAgpmCharacter->RemoveCharacter(stSpawnCharacters[i].m_lID, FALSE, FALSE);
			}
		}
	}

	return TRUE;
}

/*
BOOL AgsmEpicZone::RegisterEntry(INT32 lBossZoneID, AgpdCharacter *pcsPartyLeader)
{
	if(lBossZoneID < 1 || NULL == pcsPartyLeader)
		return FALSE;

	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter == m_mapBossZone.end())
		return FALSE;

	if(iter->second.m_pEntryVector.empty() == FALSE)
		return FALSE;

	AgpdParty *pcsParty = m_pcsAgpmParty->GetParty(pcsPartyLeader);
	if(pcsParty)
	{
		for(int lIndex=0; lIndex < pcsParty->m_nCurrentMember; lIndex++)
		{
			AgpdCharacter *pcsTarget = m_pcsAgpmCharacter->GetCharacter(pcsParty->m_lMemberListID[lIndex]);
			if(pcsTarget)
			{
				if(m_pcsAgpmCharacter->IsPC(pcsTarget) || m_pcsAgsmCharacter->GetCharDPNID(pcsTarget))
				{
					iter->second.m_pEntryVector.push_back(pcsTarget);
				}
			}
		}
	}
	else
	{
		if(m_pcsAgpmCharacter->IsPC(pcsPartyLeader) || m_pcsAgsmCharacter->GetCharDPNID(pcsPartyLeader))
		{
			iter->second.m_pEntryVector.push_back(pcsPartyLeader);
		}
	}

	return TRUE;
}
*/

BOOL AgsmEpicZone::SendEpicZoneNotice(CHAR* szNotice, INT32 lNoticeType, EpicBossZone *pstEpicBossZone, CHAR* szDirectNotice)
{
	if(NULL == szNotice || NULL == pstEpicBossZone)
		return FALSE;

	AgpdCharacter *pcsCharacter = NULL;
	INT32 lIndex = 0;

	ApmMap::RegionTemplate *pcsEpicBossZoneRegion	= m_pcsApmMap->GetTemplate(pstEpicBossZone->m_ulRegionIndex);

	while (pcsCharacter = m_pcsAgpmCharacter->GetCharSequence(&lIndex))
	{
		if (!m_pcsAgpmCharacter->IsPC(pcsCharacter) || m_pcsAgsmCharacter->GetCharDPNID(pcsCharacter) == 0) continue;
		
		ApmMap::RegionTemplate *pcsCharacterRegion		= m_pcsApmMap->GetTemplate(pcsCharacter->m_nBindingRegionIndex);
		if(NULL == pcsCharacterRegion || NULL == pcsEpicBossZoneRegion)
			continue;

		if(m_pcsApmMap->IsSameRegionInvolveParent(pcsCharacterRegion, pcsEpicBossZoneRegion))
		{
			m_pcsAgsmBattleGroud->SendBattleGroundNotice(pcsCharacter, szNotice, lNoticeType, szDirectNotice);
		}
	}

	return TRUE;
}

INT32 AgsmEpicZone::GetRemainTime(INT32 lBossZoneID)
{
	if(lBossZoneID < 1)
		return 0;

	EpicBossZoneIter iter = m_mapBossZone.find(lBossZoneID);
	if(iter == m_mapBossZone.end())
		return 0;

	INT32 lRemainTime = 0;

	// 시간을 구해서 유저들에게 알려준다.
	CTime atlCurrentTime((time_t)(GetClockCount() / 1000));
	CTime atlLastTime((time_t)(iter->second.m_ulEndTime / 1000));

	CTimeSpan atlRemainTime = atlLastTime - atlCurrentTime;

	lRemainTime = (INT32)atlRemainTime.GetTotalMinutes();
	
	if(lRemainTime <= 1)
		lRemainTime = 1;

	return lRemainTime;
}

BOOL AgsmEpicZone::SendRemainTime(EpicBossZone *pstEpicBossZone, INT32 lRemainTime)
{
	if(NULL == pstEpicBossZone)
		return FALSE;

	// 10분 단위로 체크해서 뿌려준다.
	if((lRemainTime % 5) == 0)
	{
		AuXmlNode *pNotice = m_pcsAgsmBattleGroud->GetEpicNoticeNode();
		if(pNotice)
		{
			AuXmlElement* pElem = pNotice->FirstChildElement("Notice_01");
			if(pElem)
			{
				CHAR strNotice[MAX_NOTICE_LEN] = { 0, };
				CHAR* strFormat = (CHAR*)pElem->GetText();
				sprintf( strNotice, strFormat, lRemainTime);
				SendEpicZoneNotice("Notice_01", (INT32)AGPM_BATTLEGROUND_NOTICE_EPICZONE, pstEpicBossZone, strNotice);
			}
		}
	}
	// 남은시간이 10분 미만일때에는 매번 뿌려준다.
	else if(lRemainTime < 5)
	{
		AuXmlNode *pNotice = m_pcsAgsmBattleGroud->GetEpicNoticeNode();
		if(pNotice)
		{
			AuXmlElement* pElem = pNotice->FirstChildElement("Notice_01");
			if(pElem)
			{
				CHAR strNotice[MAX_NOTICE_LEN] = { 0, };
				CHAR* strFormat = (CHAR*)pElem->GetText();
				sprintf( strNotice, strFormat, lRemainTime);
				SendEpicZoneNotice("Notice_01", (INT32)AGPM_BATTLEGROUND_NOTICE_EPICZONE, pstEpicBossZone, strNotice);
			}
		}
	}

	return TRUE;
}

int AgsmEpicZone::GetEpicBossZoneState(lua_State *L)
{
	AgsmEpicZone* pThis = AgsmEpicZone::GetInstance();

	INT32 nBossZoneID = (INT32)lua_tonumber(L, -1);

	lua_pop(L, 1);

	EpicBossZoneIter iter = pThis->m_mapBossZone.find(nBossZoneID);
	if(iter != pThis->m_mapBossZone.end())
	{
		INT32 lState = (INT32)iter->second.m_eBossZoneState;
		lua_pushinteger(L, iter->second.m_eBossZoneState);
	}

	return 1;
}

int AgsmEpicZone::SetEpicBossZoneState(lua_State* L)
{
	AgsmEpicZone* pThis = AgsmEpicZone::GetInstance();

	INT32 nBossZoneID = (INT32)lua_tonumber(L, -2);
	eEpicBossZoneState eState = (eEpicBossZoneState)lua_tointeger(L, -1);

	lua_pop(L, 2);

	EpicBossZoneIter iter = pThis->m_mapBossZone.find(nBossZoneID);
	if(iter != pThis->m_mapBossZone.end())
	{
		iter->second.m_eBossZoneState = eState;

		switch(iter->second.m_eBossZoneState)
		{
		case 0:
			{
				pThis->InitEpicBossZone(nBossZoneID);
			} break;
		case 2:
			{
				// EpicBossZone이 시작되면 SpawnData를 Load해준다.
				pThis->LoadSpawnData(nBossZoneID);
			} break;
		case 3:
			{
				// 퀘스트 종료시에는 모든 Monster들을 다 Remove해준다.
				pThis->RemoveMonster(nBossZoneID);
			} break;

		}
	}

	return 1;
}

int AgsmEpicZone::IsEpicBossZoneTimeOver(lua_State* L)
{
	AgsmEpicZone* pThis = AgsmEpicZone::GetInstance();

	INT32 nBossZoneID = (INT32)lua_tonumber(L, -1);
	INT32 bTimeOver = 0;

	lua_pop(L, 1);

	bTimeOver = (INT32)pThis->IsEpicBossZoneTimeOver(nBossZoneID);

	lua_pushinteger(L, bTimeOver);

	return 1;
}

int AgsmEpicZone::SpawnEpicZoneMonster(lua_State* L)
{
	AgsmEpicZone* pThis = AgsmEpicZone::GetInstance();

	INT32 nBossZoneID = (INT32)lua_tonumber(L, -1);
	INT32 RemainStep = 0;

	lua_pop(L, 1);

	RemainStep = pThis->SpawnEpicZoneMonster(nBossZoneID);

	return RemainStep;
}

int AgsmEpicZone::RemoveMonster(lua_State *L)
{
	AgsmEpicZone* pThis = AgsmEpicZone::GetInstance();

	INT32 nBossZoneID = (INT32)lua_tonumber(L, -1);

	lua_pop(L, 1);

	pThis->RemoveMonster(nBossZoneID);

	return 1;
}

int AgsmEpicZone::ReturnToEntry(lua_State* L)
{
	AgsmEpicZone* pThis = AgsmEpicZone::GetInstance();

	INT32 lBossZoneID = (INT32)lua_tonumber(L, -3);

	FLOAT PosX = (FLOAT)lua_tonumber(L, -2);
	FLOAT PosZ = (FLOAT)lua_tonumber(L, -1);

	lua_pop(L, 3);

	pThis->ReturnToEntry(lBossZoneID, PosX, PosZ);

	return 1;
}


