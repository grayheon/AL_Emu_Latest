/******************************************************************************
Module:  AgsmSkill.cpp
Notices: Copyright (c) NHN Studio 2002 netong
Purpose: 
Last Update: 2003. 03. 24
******************************************************************************/

#include "AgsmSkill.h"
#include "AgspSkill.h"
#include "AuMath.h"
#include "AgpmBattleGround.h"
#include "AgsmBattleGround.h"
#include "AgsmSkillEffect.h"
#include "AgsmTitle.h"

extern INT32	g_alFactorTable[AGPMSKILL_CONST_PERCENT_END + 1][3];

const TCHAR* BUFFED_SKILL_ERROR_FILE_NAME = _T("LOG\\BuffedSkillError.txt");

AgsmSkill::AgsmSkill()
{
	SetModuleName("AgsmSkill");

	SetModuleType(APMODULE_TYPE_SERVER);

	m_nIndexADSkill	= 0;

	SetPacketType(AGSMSKILL_PACKET_TYPE);

	m_csPacket.SetFlagLength(sizeof(INT16));
	m_csPacket.SetFieldType(
							AUTYPE_INT8,			1,			// operation
							AUTYPE_INT32,			1,			// skill id
							AUTYPE_PACKET,			1,			// m_csUpdateFactorResult;
							AUTYPE_PACKET,			1,			// m_csUpdateFactorPoint;
							AUTYPE_PACKET,			1,			// m_csUpdateFactorPercent;
							//AUTYPE_PACKET,			AGPMSKILL_MAX_AFFECTED_BASE,			// m_csModifyFactorPoint;
							//AUTYPE_PACKET,			AGPMSKILL_MAX_AFFECTED_BASE,			// m_csModifyFactorPercent;
							AUTYPE_INT32,			1,			// m_lModifySkillLevel;
							AUTYPE_INT32,			1,			// m_lModifyCharLevel;
							AUTYPE_INT32,			1,			// m_nMeleeDefensePoint;
							AUTYPE_INT32,			1,			// m_nMeleeReflectPoint;
							AUTYPE_INT32,			1,			// m_nMagicDefensePoint;
							AUTYPE_INT32,			1,			// m_nMagicReflectPoint;
							AUTYPE_INT32,			1,			// m_lDamageShield;

							AUTYPE_MEMORY_BLOCK,	1,			// m_stBuffedSkillCombatEffectArg;
							AUTYPE_MEMORY_BLOCK,	1,			// m_stBuffedSkillFactorEffectArg;
							
							AUTYPE_PACKET,			1,			// m_csPacketAttachBaseData;
							AUTYPE_PACKET,			1,			// db data (m_csPacketDBData)
							AUTYPE_END,				0
							);

	m_csPacketAttachBaseData.SetFlagLength(sizeof(INT8));
	m_csPacketAttachBaseData.SetFieldType(
							AUTYPE_PACKET,			AGPMSKILL_MAX_SKILL_BUFF,	// m_csBuffedSkillProcessInfo
							AUTYPE_PACKET,			1,							// target base packet
							AUTYPE_INT32,			1,							// m_lDamageAdjustRate
							AUTYPE_END,				0
							);

	m_csPacketBuffedSkillProcessInfo.SetFlagLength(sizeof(INT8));
	m_csPacketBuffedSkillProcessInfo.SetFieldType(
							AUTYPE_INT8,			1,			// m_csSkillOwner.m_eType;
							AUTYPE_INT32,			1,			// m_csSkillOwner.m_lID;
							AUTYPE_INT32,			1,			// m_lMasteryPoint;
							AUTYPE_PACKET,			1,			// m_pcsTempSkill;
							AUTYPE_END,				0
							);

	m_csPacketTempSkillInfo.SetFlagLength(sizeof(INT16));
	m_csPacketTempSkillInfo.SetFieldType(
							AUTYPE_INT32,			1,			// m_lSkillID
							AUTYPE_INT32,			1,			// m_pcsTemplate->m_lID
							AUTYPE_INT32,			1,			// m_lSkillLevel
							AUTYPE_UINT32,			1,			// m_tmEnd
							AUTYPE_UINT32,			1,			// m_tmInterval
							AUTYPE_UINT32,			1,			// m_tmNextProcess
							AUTYPE_PACKET,			1,			// m_csModifyFactorPoint
							AUTYPE_PACKET,			1,			// m_csModifyFactorPercent
							AUTYPE_INT32,			1,			// m_nMeleeDefensePoint
							AUTYPE_INT32,			1,			// m_nMeleeReflectPoint
							AUTYPE_INT32,			1,			// m_nMagicDefensePoint
							AUTYPE_INT32,			1,			// m_nMagicReflectPoint
							AUTYPE_UINT32,			1,			// current clock count
							AUTYPE_END,				0
							);

	m_csPacketDBData.SetFlagLength(sizeof(INT8));
	m_csPacketDBData.SetFieldType(
							AUTYPE_INT32,			1,			// CID
							AUTYPE_PACKET,			1,			// m_csPacketMasteryDBData
//							AUTYPE_PACKET,			1,
//							AUTYPE_PACKET,			1,
//							AUTYPE_PACKET,			1,
//							AUTYPE_PACKET,			1,
//							AUTYPE_PACKET,			1,
							AUTYPE_END,				0
							);

	m_csPacketMasteryDBData.SetFlagLength(sizeof(INT8));
	m_csPacketMasteryDBData.SetFieldType(
//							AUTYPE_INT8,			1,			// mastery index
//							AUTYPE_UINT8,			1,			// total input sp
							AUTYPE_MEMORY_BLOCK,	1,			// mastery skill tree
							AUTYPE_MEMORY_BLOCK,	1,			// mastery specialize
							AUTYPE_MEMORY_BLOCK,	1,			// compose list of product skill
							AUTYPE_END,				0
							);

	m_pagpmLog = NULL;
	m_pagpmBattleGround = NULL;
	m_pagsmBattleGround = NULL;
	m_pagsmSkillEffect = NULL;
}

AgsmSkill::~AgsmSkill()
{
}

BOOL AgsmSkill::OnAddModule()
{
	m_pagpmGrid			= (AgpmGrid *)			GetModule("AgpmGrid");

	m_papmMap			= (ApmMap *)			GetModule("ApmMap");
	m_papmObject		= (ApmObject *)			GetModule("ApmObject");
	m_papmEventManager	= (ApmEventManager *)	GetModule("ApmEventManager");
	m_pagpmFactors		= (AgpmFactors *)		GetModule("AgpmFactors");
	m_pagpmCharacter	= (AgpmCharacter *)		GetModule("AgpmCharacter");
	m_pagpmItem			= (AgpmItem *)			GetModule("AgpmItem");
	m_pagpmItemConvert	= (AgpmItemConvert *)	GetModule("AgpmItemConvert");
	m_pagpmCombat		= (AgpmCombat *)		GetModule("AgpmCombat");
	m_pagpmSkill		= (AgpmSkill *)			GetModule("AgpmSkill");
	m_pagpmParty		= (AgpmParty *)			GetModule("AgpmParty");
	//m_pagpmShrine		= (AgpmShrine *)		GetModule("AgpmShrine");
	m_pagpmLog			= (AgpmLog *)			GetModule("AgpmLog");
	m_pagpmEventNature	= (AgpmEventNature *)	GetModule("AgpmEventNature");
	m_pagpmSummons		= (AgpmSummons *)		GetModule("AgpmSummons");
	m_pagpmPvP			= (AgpmPvP *)			GetModule("AgpmPvP");
	m_pagpmEventSpawn	= (AgpmEventSpawn *)	GetModule("AgpmEventSpawn");
	m_pagpmEventSkillMaster = (AgpmEventSkillMaster *) GetModule("AgpmEventSkillMaster");
	m_pagpmGuild		= (AgpmGuild *)			GetModule("AgpmGuild");
	m_pagpmSiegeWar		= (AgpmSiegeWar *)		GetModule("AgpmSiegeWar");
	m_pagpmArchlord		= (AgpmArchlord *)		GetModule("AgpmArchlord");
	m_pagpmTimer		= (AgpmTimer *)			GetModule("AgpmTimer");
	m_pagpmOptimizedPacket2	= (AgpmOptimizedPacket2 *) GetModule("AgpmOptimizedPacket2");
	m_pagpmWantedCriminal = (AgpmWantedCriminal*) GetModule("AgpmWantedCriminal");
	m_pagpmBillInfo		= (AgpmBillInfo *)		GetModule("AgpmBillInfo");

	m_pagsmSystemMessage= (AgsmSystemMessage *) GetModule("AgsmSystemMessage");
	//m_pagsmDBStream		= (AgsmDBStream *)		GetModule("AgsmDBStream");
	m_pagsmFactors		= (AgsmFactors *)		GetModule("AgsmFactors");
	m_pagsmAOIFilter	= (AgsmAOIFilter *)		GetModule("AgsmAOIFilter");
	m_pagsmCharacter	= (AgsmCharacter *)		GetModule("AgsmCharacter");
	m_pagsmCharManager	= (AgsmCharManager *)	GetModule("AgsmCharManager");
	m_pagsmItem			= (AgsmItem *)			GetModule("AgsmItem");
	m_pagsmCombat		= (AgsmCombat *)		GetModule("AgsmCombat");
	m_pAgsmServerManager	= (AgsmServerManager *)	GetModule("AgsmServerManager2");
	m_pagsmParty		= (AgsmParty *)			GetModule("AgsmParty");
	m_pagsmSummons		= (AgsmSummons *)		GetModule("AgsmSummons");
	m_pagsmTimer		= (AgsmTimer *)			GetModule("AgsmTimer");
	m_pagsmEventNature	= (AgsmEventNature *)	GetModule("AgsmEventNature");
	//m_pagsmEventSpawn	= (AgsmEventSpawn *)	GetModule("AgsmEventSpawn");
	
	m_pagsmSkillEffect = new AgsmSkillEffect;

	// JNY TODO : Relay 서버 개발을 위해 m_pagsmDBStream를 체크하는 부분을 
	// 잠시 삭제합니다. 
	// 2004.2.16
	// 다시 복구 하거나 DB모듈 변경에 의한 변경작업을 해야합니다.
	
	if (!m_papmMap || 
		!m_papmObject ||
		!m_papmEventManager || 
		!m_pagpmFactors || 
		!m_pagpmCharacter || 
		!m_pagpmItem || 
		!m_pagpmItemConvert ||
		!m_pagpmCombat ||
		!m_pagpmSkill || 
		!m_pagpmParty || 
		!m_pagpmPvP ||
		//!m_pagpmShrine ||
		!m_pagpmEventNature ||
		//!m_pagpmEventSpawn ||
		!m_pagpmEventSkillMaster ||
		!m_pagpmGuild ||
		!m_pagpmSiegeWar)// ||
		//!m_pagpmWantedCriminal)// ||
		//!m_pagpmTimer)
		return FALSE;

	if (/*!m_pagsmDBStream ||*/
		!m_pagsmFactors || 
		!m_pagsmAOIFilter || 
		!m_pagsmCharacter || 
		!m_pagsmCharManager || 
		!m_pagsmItem || 
		!m_pagsmCombat)// ||
		//!m_pagsmTimer)// ||
		//!m_pagsmSummons)// ||
		//!m_pagsmEventSpawn)
		return FALSE;

	if (!m_pAgsmServerManager || 
		!m_pagsmParty)
		return FALSE;

	if (!m_papmEventManager->RegisterEvent(APDEVENT_FUNCTION_SKILL, NULL, NULL, ProcessIdleEvent, NULL, NULL, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackRemoveChar(CBRemoveCharacter, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackUpdateChar(CBUpdateCharacter, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackActionSkill(CBActionSkill, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackUpdateSpecialStatus(CBUpdateSpecialStatus, this))
		return FALSE;

	if (!m_pagsmCharacter->SetCallbackEnterGameworld(CBEnterGameWorld, this))
		return FALSE;

	if (!m_pagsmCharacter->SetCallbackAllUpdate(CBUpdateAllToDB, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackTransformStatus(CBTransformStatus, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackRestoreTransform(CBTransformRestore, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackEvolution(CBEvolution, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackRestoreEvolution(CBRestoreEvolution, this))
		return FALSE;

	if(!m_pagpmCharacter->SetCallbackBindingRegionChange(CBRegionChange, this))
		return FALSE;

	/*
	if (m_pagsmDBStream)
	{
		if (!m_pagsmDBStream->SetCallbackSkillResult(CBStreamDB, this))
			return FALSE;
	}
	*/

	if (!m_pagpmItem->SetCallbackEquip(CBEquipItem, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackUnEquip(CBUnEquipItem, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackUnUseCashItem(CBUnUseCashItem, this))
		return FALSE;

	if (!m_pagpmSkill->SetCallbackInitSkill(CBInitSkill, this))
		return FALSE;
	if (!m_pagpmSkill->SetCallbackRemoveSkill(CBRemoveSkill, this))
		return FALSE;
	if (!m_pagpmSkill->SetCallbackReceiveAction(CBReceiveAction, this))
		return FALSE;
	if (!m_pagpmSkill->SetCallbackReceiveRealHit(CBReceiveRealHit, this))
		return FALSE;

	if (!m_pagpmSkill->SetCallbackUpdateSkillPoint(CBUpdateSkillPoint, this))
		return FALSE;
	if (!m_pagpmSkill->SetCallbackUpdateDIRTPoint(CBUpdateDIRTPoint, this))
		return FALSE;

	if (!m_pagpmSkill->SetCallbackModifiedSkillLevel(CBModifiedSkillLevel, this))
		return FALSE;

	if (!m_pagpmSkill->SetCallbackGetModifiedSkillLevel(CBGetModifiedSkillLevel, this))
		return FALSE;
	
//#ifdef	__NEW_MASTERY__
//	if (!m_pagpmSkill->SetCallbackMasteryUpdate(CBMasteryUpdate, this))
//		return FALSE;
//	if (!m_pagpmSkill->SetCallbackMasteryNodeUpdate(CBMasteryNodeUpdate, this))
//		return FALSE;
//	if (!m_pagpmSkill->SetCallbackAddSPResult(CBMasteryAddSPResult, this))
//		return FALSE;
//#else
//	if (!m_pagpmSkill->SetCallbackMasteryChange(CBMasteryChange, this))
//		return FALSE;
//	if (!m_pagpmSkill->SetCallbackMasterySpecialize(CBMasterySpecialize, this))
//		return FALSE;
//	if (!m_pagpmSkill->SetCallbackMasteryUnSpecialize(CBMasteryUnSpecialize, this))
//		return FALSE;
//#endif	//__NEW_MASTERY__
//	if (!m_pagpmSkill->SetCallbackMasteryRollback(CBMasteryRollback, this))
//		return FALSE;

	/*
	if (!m_pagpmSkill->SetCallbackMasteryResult(CBMasteryResult, this))
		return FALSE;
	if (!m_pagpmSkill->SetCallbackMasteryUpdateTotalSP(CBMasteryUpdateTotalSP, this))
		return FALSE;
	*/

	if (!m_pagsmCharManager->SetCallbackCreateChar(CBCreateCharacter, this))
		return FALSE;

	if (!m_pagsmItem->SetCallbackPreCheckEnableSkillScroll(CBPreCheckEnableSkillScroll, this))
		return FALSE;
	if (!m_pagsmItem->SetCallbackEndBuffedSkillByItem(CBEndBuffedSkillByItem, this))
		return FALSE;
	if (!m_pagsmItem->SetCallbackUseItemReverseOrb(CBUseItemReverseOrb, this))
		return FALSE;
	if (!m_pagsmItem->SetCallbackPreCheckEnableJanusTalisman(CBPreCheckEnableJanusTalisman, this))
		return FALSE;

	if (!m_pagsmCombat->SetCallbackCheckDefense(CBMeleeAttack, this))
		return FALSE;
	if (!m_pagsmCombat->SetCallbackPreCheckCombat(CBPreCheckCombat, this))
		return FALSE;
	if (!m_pagsmCombat->SetCallbackPostCheckCombat(CBPostCheckCombat, this))
		return FALSE;
	if (!m_pagsmCombat->SetCallbackConvertDamage(CBConvertDamage, this))
		return FALSE;
	//if (!m_pagsmCombat->SetCallbackAttack(CBAttackFromCombat, this))
	//	return FALSE;


	//if (!SetCallbackMeleeAttackCheck(CBMeleeAttack, this))
	//	return FALSE;
	if (!SetCallbackMagicAttackCheck(CBMagicAttack, this))
		return FALSE;
	if (!SetCallbackPreCheckCast(CBPreCheckCast, this))
		return FALSE;
	if (!SetCallbackPostCheckCast(CBPostCheckCast, this))
		return FALSE;

	m_nIndexADSkill = m_pagpmSkill->AttachSkillData(this, sizeof(AgsdSkill), ConAgsdSkill, DesAgsdSkill);
	if (m_nIndexADSkill < 0)
		return FALSE;

	m_nIndexADCharacter = m_pagpmCharacter->AttachCharacterData(this, sizeof(AgsdSkillADBase), ConAgsdSkillADBase, DesAgsdSkillADBase);
	if (m_nIndexADCharacter < 0)
		return FALSE;

	/*
	m_nIndexADObject = m_papmObject->AttachObjectData(this, sizeof(AgsdSkillADBase), ConAgsdSkillADBase, DesAgsdSkillADBase);
	if (m_nIndexADObject < 0)
		return FALSE;
	*/

	if (!m_pagsmCharacter->SetCallbackSendEquipItem(CBSendCharacterViewInfo, this))
		return FALSE;

	if (!m_pagpmOptimizedPacket2)
	{
		if (!m_pagsmCharacter->SetCallbackSendCharacterAllInfo(CBSendCharacterAllInfo, this))
			return FALSE;
	}
	if (!m_pagsmCharacter->SetCallbackSendCharacterNewID(CBSendCharacterNewID, this))
		return FALSE;
	if (!m_pagsmCharacter->SetCallbackSendCharacterAllServerInfo(CBSendCharacterAllServerInfo, this))
		return FALSE;

	if (!m_pagsmCharacter->SetCallbackReCalcFactor(CBReCalcFactor, this))
		return FALSE;
	
	/*
	if (!m_pagsmCharacter->SetCallbackSaveCharacterDB(CBSaveCharacterDB, this))
		return FALSE;
	*/

	if(m_pagsmSummons)
	{
		if(!m_pagsmSummons->SetCallbackNewSummons(CBNewSummons, this))
			return FALSE;
		if(!m_pagsmSummons->SetCallbackEndSummons(CBEndSummons, this))
			return FALSE;
		if(!m_pagsmSummons->SetCallbackEndTaming(CBEndTaming, this))
			return FALSE;
		if(!m_pagsmSummons->SetCallbackEndFixed(CBEndFixed, this))
			return FALSE;
	}

	if(m_pagpmArchlord)
	{
		if(!m_pagpmArchlord->SetCallbackSetArchlord(CBSetArchlord, this))
			return FALSE;
		if(!m_pagpmArchlord->SetCallbackCancelArchlord(CBCancelArchlord, this))
			return FALSE;
	}

	m_lPointFactorTypeIndex			= m_pagpmFactors->SetFactorDataName(_T("Skill_UpdateFactorPoint"));
	m_lPercentFactorTypeIndex		= m_pagpmFactors->SetFactorDataName(_T("Skill_UpdateFactorPercent"));
	m_lResultFactorTypeIndex		= m_pagpmFactors->SetFactorDataName(_T("Skill_UpdateFactorResult"));
	m_lItemPointFactorTypeIndex		= m_pagpmFactors->SetFactorDataName(_T("Skill_UpdateFactorItemPoint"));
	m_lItemPercentFactorTypeIndex	= m_pagpmFactors->SetFactorDataName(_T("Skill_UpdateFactorItemPercent"));

	return TRUE;
}

BOOL AgsmSkill::OnInit()
{
	m_pagpmBattleGround	= (AgpmBattleGround*)GetModule("AgpmBattleGround");
	m_pagsmBattleGround	= (AgsmBattleGround*)GetModule("AgsmBattleGround");
	m_pagsmTitle		= (AgsmTitle*)GetModule("AgsmTitle");
	m_pagpmShrineBattle = (AgpmShrineBattle*)GetModule("AgpmShrineBattle");
	
	//if(!m_pagpmBattleGround || !m_pagsmBattleGround)
	//	return FALSE;

	if (!m_pagpmShrineBattle)
		return FALSE;

	if( m_pagsmSkillEffect )
		m_pagsmSkillEffect->Initialize((ApModuleManager*)AgsEngine::GetInstance());
	
	return TRUE;
}

BOOL AgsmSkill::OnDestroy()
{
	if( m_pagsmSkillEffect )
		delete m_pagsmSkillEffect;

	return TRUE;
}

//		ProcessIdleEvent
//	Functions
//		- ApmEventModule 에 등록한 Event에 대한 처리를 한다.
//				OnIdle() 에서 호출되는 Event 들에 대한 처리를 한다.
//			1. ProcessUseSkill() 을 호출해 현재 pcsBase가 사용하고 있는 스킬들을 처리한다.
//			2. 더이상 사용중인 스킬이 없다면 RemoveEvent()를 한다.
//	Arguments
//		- pData		: ApdEvent pointer
//		- pClass	: AgpmSkill Module pointer
//		- pCustData	: ulClockCount pointer
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ProcessIdleEvent(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmSkill::ProcessIdleEvent");

	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	ApdEvent		*pcsEvent		= (ApdEvent *)		pData;
	UINT32			ulClockCount	= *(UINT32 *)		pCustData;

	if (pcsEvent->m_eFunction != APDEVENT_FUNCTION_SKILL)
		return FALSE;

	// (ApBase *) pcsEvent->m_pcsSource 가 현재 사용중인 스킬을 모두 처리한다.
	pThis->ProcessUseSkill(pcsEvent, ulClockCount);

	AgpdSkillAttachData *pcsAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase *) pcsEvent->m_pcsSource);

	if (!pcsAttachData)
	{
		return FALSE;
	}

	// 사용중인 스킬이 없다면 Event를 삭제하고 리턴한다.
	if (pcsAttachData->m_alUseSkillID[0] == AP_INVALID_SKILLID)
	{
		if (!pcsEvent->m_pstCondition || !pcsEvent->m_pstCondition->m_pstTime)
		{
			if (!pThis->m_papmEventManager->SetCondition(pcsEvent, APDEVENT_COND_TIME))
				return FALSE;
		}

		pcsEvent->m_pstCondition->m_pstTime->m_lEndTimeOffset = 0;

		return TRUE;
	}

	return TRUE;
}

//		ProcessUseSkill
//	Functions
//		- pcsBase가 현재(ulClockCount)에 사용하고 있는 스킬들을 상태에 따라 하나씩 처리한다.
//	Arguments
//		- pcsEvent	:
//		- ulClockCount : current clock count
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ProcessUseSkill(ApdEvent *pcsEvent, UINT32 ulClockCount)
{
	if (!pcsEvent || !pcsEvent->m_pcsSource)
		return FALSE;

	ApBase *pcsBase = pcsEvent->m_pcsSource;

	AgpdSkillAttachData *pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

//	INT32	alTempUseSkillID[AGPMSKILL_MAX_SKILL_USE];
//	CopyMemory(alTempUseSkillID, pcsAttachData->m_alUseSkillID, sizeof(INT32) * AGPMSKILL_MAX_SKILL_USE);

	ApSafeArray<INT32, AGPMSKILL_MAX_SKILL_USE>		alTempUseSkillID;
	alTempUseSkillID.MemCopy(0, &pcsAttachData->m_alSkillID[0], AGPMSKILL_MAX_SKILL_USE);

	int j = 0;
	for (int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if (alTempUseSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill	*pcsSkill	= m_pagpmSkill->GetSkill(alTempUseSkillID[i]);
		if (!pcsSkill)
			continue;

		/*
		if (!m_pagpmSkill->m_csAdminSkill.GlobalWLock())
			continue;

		// 스킬 처리를 한다.
		AgpdSkill *pcsSkill = m_pagpmSkill->GetSkill(alTempUseSkillID[i]);
		if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_Mutex.WLock())
		{
			m_pagpmSkill->m_csAdminSkill.GlobalRelease();
			continue;
		}

		if (!m_pagpmSkill->m_csAdminSkill.GlobalRelease())
		{
			pcsSkill->m_Mutex.Release();
			continue;
		}
		*/

		AgpdSkillTemplate *pcsSkillTemplate = (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

		switch (pcsSkill->m_eStatus) {
		case AGPMSKILL_STATUS_NOT_CAST:
			//pcsSkill->m_Mutex.Release();
			continue;
			break;

		case AGPMSKILL_STATUS_CAST_DELAY:
			{
				if (pcsSkill->m_ulEndTime > ulClockCount)	// 아직 cast delay 가 지나지 않았다. 암것도 안한고 걍 넘어간다.
				{
					//pcsSkill->m_Mutex.Release();
					continue;
				}

				// cast delay가 다 지났다. 이넘은 이제 cast 해야 한다.
				INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);

				// time setting을 하고... (m_ulEndTime과 m_ulInterval을 구한다.)
				pcsSkill->m_ulStartTime				= ulClockCount;

				// 2005.09.12. steeple
				// 아 이거 일일이 다 찾아서 고쳐야 하는 거야 ㅠㅠ
				// 그러기 좀 뭐 해서 Buff 관련 스킬에만 이렇게 해줌.
				if(m_pagpmSkill->IsDurationByDistanceSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) ||
					m_pagpmSkill->IsDurationUnlimited((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
					pcsSkill->m_ulEndTime			= 0xFFFFFFFF;
				else
					pcsSkill->m_ulEndTime			= ulClockCount + m_pagpmSkill->GetSkillDurationTime(pcsSkill, lSkillLevel);

				pcsSkill->m_ulInterval				= m_pagpmSkill->GetSkillInterval(pcsSkill, lSkillLevel);
				if (pcsSkill->m_ulInterval)
					pcsSkill->m_ulNextProcessTime	= ulClockCount + pcsSkill->m_ulInterval;
				else
					pcsSkill->m_ulNextProcessTime	= pcsSkill->m_ulEndTime;

				// status를 변경하고...
				m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_CAST);

				if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
					m_pagpmCharacter->UpdateActionStatus((AgpdCharacter *) pcsSkill->m_pcsBase, AGPDCHAR_STATUS_NOT_ACTION);

				// 스킬 효과를 적용한다. (먼저 계산하고 적용한다)
				ApplySkill(pcsSkill, pcsEvent);

				//if (pcsSkill->m_ulStartTime == pcsSkill->m_ulEndTime)
				//	m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_NOT_CAST);

				EndSkill(pcsSkill);
			}
			break;

		/*
		case AGPMSKILL_STATUS_CAST_DELAY:
			{
				if (pcsSkill->m_ulEndTime > ulClockCount)	// 아직 cast delay 가 지나지 않았다. 암것도 안한고 걍 넘어간다.
				{
					pcsSkill->m_Mutex.Release();
					continue;
				}

				// cast delay가 다 지났다. 이넘은 이제 cast 해야 한다.

				// time setting을 하고... (m_ulEndTime과 m_ulInterval을 구한다.)
				pcsSkill->m_ulStartTime				= ulClockCount;
				pcsSkill->m_ulEndTime				= ulClockCount + m_pagpmSkill->GetSkillDurationTime(pcsSkill);
				pcsSkill->m_ulInterval				= m_pagpmSkill->GetSkillInterval(pcsSkill);
				if (pcsSkill->m_ulInterval)
					pcsSkill->m_ulNextProcessTime	= ulClockCount + pcsSkill->m_ulInterval;
				else
					pcsSkill->m_ulNextProcessTime	= pcsSkill->m_ulEndTime;

				// status를 변경하고...
				m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_CAST);

				if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
					m_pagpmCharacter->UpdateActionStatus((AgpdCharacter *) pcsSkill->m_pcsBase, AGPDCHAR_STATUS_NOT_ACTION);

				// 스킬 효과를 적용한다. (먼저 계산하고 적용한다)
				if (!ApplySkill(pcsSkill, pcsEvent))
					EndSkill(pcsSkill);
			}
			break;

		case AGPMSKILL_STATUS_CAST:
			{
				if (pcsSkillTemplate->m_bShrineSkill)
				{
					if (pcsSkill->m_csTargetBase.m_eType != APBASE_TYPE_NONE &&
						pcsSkill->m_csTargetBase.m_lID != 0)
					{
						ApplySkill(pcsSkill, pcsEvent);
					}
				}
				else if (pcsSkill->m_ulEndTime > ulClockCount)		// 아직 끝이 아니다.
				{
					if (pcsSkill->m_ulInterval == 0 || pcsSkill->m_ulNextProcessTime > ulClockCount)
					{
						// interval이 없는 경우, 혹은 interval만큼 시간이 안지난 경우이다.
						// 암것도 안하고 걍 넘어간다.
						pcsSkill->m_Mutex.Release();
						continue;
					}
					
				// interval 이 지났다. 처리하고 m_ulNextProcessTime을 다시 세팅한다.

					// time setting을 하고...
					pcsSkill->m_ulNextProcessTime += pcsSkill->m_ulInterval;

					// 스킬 효과를 적용한다. (여기선 다시 계산할 필요 없다)
					if (!ApplySkillByAffectedBase(pcsSkill, pcsEvent))
						EndSkill(pcsSkill);
				}
				else
				{
					if (pcsSkill->m_ulStartTime == pcsSkill->m_ulEndTime)
					{
						// 스킬 시작과 끝 시간이 같다는 얘기는 일회용 스킬로.. 
						// 아직 Cast 계산을 안했다는 얘기다.
						ApplySkill(pcsSkill, pcsEvent);
					}

					// 스킬 시간이 다되었다.

					// 이 스킬에 영향 받던 넘들을 모두 되돌린다. 
					//		예) pcsAgsdSkill->m_csUpdateFactor 값을 CalcFactor()를 이용해 뺀다.
					// 영향 받던 넘들에게 버프 없어진다고 알려준다.
					EndSkill(pcsSkill);

					pcsSkill->m_Mutex.Release();

					continue;
				}
			}
			break;
		*/

		}

		//pcsSkill->m_Mutex.Release();

		//pcsAttachData->m_alUseSkillID[j] = alTempUseSkillID[i];
		++j;
	}

	return TRUE;
}

//		ProcessBuffedSkill
//	Functions
//		- pcsBase에 현재 버프된 스킬들에 대한 처리를 한다.
//			1. Buffed List에서 버프된 스킬들에 정보를 가져온다.
//			2. 가져온 정보에 세팅되어 있는 시간들을 보고 처리 여부를 판단한다.
//			3. Interval 이 세팅되어 있는 경우 시간이 되면 처리를 한다.
//			4. End Skill Time 이 되면 FreeSkillEffect()를 호출해서 스킬 사용을 끝낸다.
///			5. Buffed List (AgsdSkillADBase) 에서 스킬 정보를 삭제한다.
//	Arguments
//		- pcsBase	:
//		- ulClockCount : current clock count
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ProcessBuffedSkill(ApBase *pcsBase, UINT32 ulClockCount)
{
	PROFILE("AgsmSkill::ProcessBuffedSkill");

	if (!pcsBase)
		return FALSE;

	AgpdSkillAttachData	*pcsAttachData	= m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

	AgsdSkillADBase		*pcsADBase		= GetADBase(pcsBase);
	if (!pcsADBase)
		return FALSE;

	// 버프된 스킬이 있는지 검사한다.
	if (pcsAttachData->m_astBuffSkillList[0].lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	INT32 lSkillTID = 0;
	INT_PTR lSkillMemoryAddress = 0;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			break;

		lSkillMemoryAddress = (INT_PTR)pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill;

		// 2005.11.17. steeple
		// 이런 경우가 대부분이길래 로그를 보강한다.
		// 2005.11.09. steeple
		// 이런 경우도 가끔 생기길래 남겨본다.
		if(!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate)
		{
			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "\nm_pcsTempSkill is Valid But Template is NULL (Address : 0x%08X, i : %d, CharID : %d, CharTID : %d, CharStatus : %d)\n",
				lSkillMemoryAddress, i, pcsBase->m_lID, ((AgpdCharacter*)pcsBase)->m_pcsCharacterTemplate->m_lID, ((AgpdCharacter*)pcsBase)->m_unActionStatus);
			AuLogFile_s((CHAR*)BUFFED_SKILL_ERROR_FILE_NAME, strCharBuff);

			for(INT32 lIndex = 0; lIndex < AGPMSKILL_MAX_SKILL_BUFF; lIndex++)
			{
				char strCharBuff2[256] = { 0, };
				sprintf_s(strCharBuff2, sizeof(strCharBuff2), "[PDAttach] index : %d, lSkillID : %d, lSkillTID : %d, lOwnerID %d, lCasterTID : %d\n",
					lIndex,
					pcsAttachData->m_astBuffSkillList[lIndex].lSkillID,
					pcsAttachData->m_astBuffSkillList[lIndex].lSkillTID,
					pcsAttachData->m_astBuffSkillList[lIndex].csOwner.m_lID,
					pcsAttachData->m_astBuffSkillList[lIndex].lCasterTID);
				AuLogFile_s((CHAR*)BUFFED_SKILL_ERROR_FILE_NAME, strCharBuff2);
			}

			for(INT32 lIndex = 0; lIndex < AGPMSKILL_MAX_SKILL_BUFF; lIndex++)
			{
				char strCharBuff2[256] = { 0, };
				sprintf_s(strCharBuff2, sizeof(strCharBuff2), "[SDAttach] index : %d, lSkillID : %d, lSkillTID : ?, lOwnerID %d, lSkillLevel : %d\n",
					lIndex,
					pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill ? pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_lID : -1,
					pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_csSkillOwner.m_lID,
					pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_lMasteryPoint);
				AuLogFile_s((CHAR*)BUFFED_SKILL_ERROR_FILE_NAME, strCharBuff2);
			}
			break;
		}

		// 2005.10.14. steeple
		// 저 밑에 코드에서 죽는 경우가 있는 데 무슨 스킬 때문에 죽는 지 알기 위해서 남겼3
		lSkillTID = pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate->m_lID;

		//타이틀에 의해 사용되고 있는 스킬이라면 해제시키지 않는다.
		if(m_pagsmTitle->GetEffectiveSkillTid((AgpdCharacter *)pcsBase) == lSkillTID)
			continue;

		if (pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulEndTime <= ulClockCount)
		{
			// 2005.07.07. steeple
			// 버프 종료하기 전에 이거 한번 불러준다.
			m_pagsmSkillEffect->ProcessSkillEffect2ActionOnAction(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill,
											pcsBase, AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE1, FALSE);

			EndBuffedSkill(pcsBase, i, TRUE, _T("ProcessBuffedSkill, EndTime <= ulClockCount"));
			--i;

			continue;
		}

		// 2005.07.18. steeple
		// Duration By Distance Skill Check
		if(!CheckSkillDurationByDistance(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill, pcsBase))
		{
			EndBuffedSkill(pcsBase, i, TRUE, _T("ProcessBuffedSkill, CheckSkillDurationByDistance return FALSE"));
			--i;
			continue;
		}

		// 2005.12.08. steeple
		// Duration is unlimited by Cash item
		if(!CheckCashSkillDuration(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill, pcsBase))
		{
			EndBuffedSkill(pcsBase, i, TRUE, _T("ProcessBuffedSkill, CheckCashSkillDuration return FALSE"));
			--i;
			continue;
		}

		// 2006.08.31. steeple
		// Check Caster's Status (Action on Action Type 5)
		if(!CheckCasterStatus(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill, pcsBase))
		{
			EndBuffedSkill(pcsBase, i, TRUE, _T("ProcessBuffedSkill, CheckCasterStatus return FALSE"));
			--i;
			continue;
		}

		// 2006.11.14. steeple
		// Check Affected DOT Count
		if(!CheckDOTCount(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill, pcsBase))
		{
			EndBuffedSkill(pcsBase, i, TRUE, _T("ProcessBuffedSkill, CheckDOTCount return FALSE"));
			--i;
			continue;
		}

		if (pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulNextProcessTime <= ulClockCount)
		{
			// 스킬 처리를 한다.
			//////////////////////////////////////////////////
			ApplySkillByBuffedList(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill, pcsBase);

			if (pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
				pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulNextProcessTime += pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulInterval;
			else
			{
				char strCharBuff2[256] = { 0, };
				sprintf_s(strCharBuff2, sizeof(strCharBuff2), "m_pcsTempSkill is NULL (Address : 0x%08X, SkilTID : %d, CharStatus : %d)\n",
					lSkillMemoryAddress, lSkillTID, ((AgpdCharacter*)pcsBase)->m_unActionStatus);
				AuLogFile_s((CHAR*)BUFFED_SKILL_ERROR_FILE_NAME, strCharBuff2);
			}

			continue;
		}
	}

	return TRUE;
}

// 2005.07.18. steeple
// 거리에 의해서 지속시간이 변하는 스킬이다.
// 오라 형태의 스킬로, 일정 범위를 벗어나면 버프를 해제해주어야 함.
// TRUE 로 리턴하면 그냥 계속 처리. FALSE 로 리턴하면 버프에서 빼준다.
BOOL AgsmSkill::CheckSkillDurationByDistance(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	if(!m_pagpmSkill->IsDurationByDistanceSkill(pcsSkillTemplate))
		return TRUE;

	// 여기까지 왔으면 Duration By Distance 스킬이다.
	// 스킬을 사용한 놈의 좌표를 얻는다.
	ApBase* pcsBase = m_pagpmSkill->GetBaseLock((INT8)pcsSkill->m_csBase.m_eType, pcsSkill->m_csBase.m_lID);
	if(!pcsBase)
		return FALSE;	// 못 얻었으면 버프에서 빼야 한다.

	BOOL bResult = TRUE;

	// 거리를 잰다.
    if(!m_pagpmFactors->IsInRange(&((AgpdCharacter*)pcsTarget)->m_stPos,
									&((AgpdCharacter*)pcsBase)->m_stPos,
									m_pagpmSkill->GetTargetArea1(pcsSkill),
									0))
		bResult = FALSE;

	pcsBase->m_Mutex.Release();

	return bResult;
}

// 2005.12.08. steeple
// 캐쉬아이템은 일단 지속시간이 무한대이고, 캐쉬 인벤에 해당 스킬 아이템이 사용중인지를 계속 체크하도록 하자.
// 만약 이걸로 인해서 느려진다면... 다른 방식을 찾아보자.
// TRUE 로 리턴하면 그냥 계속 처리. FALSE 로 리턴하면 버프에서 빼준다.
BOOL AgsmSkill::CheckCashSkillDuration(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	if(!m_pagpmSkill->IsCashSkill(pcsSkillTemplate))
		return TRUE;

	// Cash Inventory 에서 해당 스킬을 사용중인 아이템이 있는 지 확인한다.
	// 없으면 FALSE 로 리턴해준다.

	return TRUE;
}

// 2006.08.31. steeple
// 계속 캐스터의 상태를 확인해서 없거나, 죽어있으면 해당 버프를 빼준다.
BOOL AgsmSkill::CheckCasterStatus(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	// 현재(2006.08.31) 는 ActionOnActionType5 만 체크한다.
	if(m_pagpmSkill->GetActionOnActionType(pcsSkillTemplate) != 5)
		return TRUE;

	AgpdCharacter* pcsCaster = NULL;
	if(pcsSkill->m_bCloneObject &&
		pcsSkill->m_csBase.m_eType == APBASE_TYPE_CHARACTER)
		pcsCaster = m_pagpmCharacter->GetCharacter(pcsSkill->m_csBase.m_lID);

	// 없거나 죽어있거나 맵에 없다면 return FALSE
	if(!pcsCaster || pcsCaster->m_unActionStatus == AGPDCHAR_STATUS_DEAD ||
		!pcsCaster->m_bIsAddMap)
		return FALSE;

	return TRUE;
}

// 2006.11.14. steeple
// DOT 횟수가 처음 공격시에 하나 묻히는 현상 때문에 완전 따로 계산하는 걸로 분리하였다.
BOOL AgsmSkill::CheckDOTCount(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	if(!m_pagpmSkill->IsDOTSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
		return TRUE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return TRUE;

	if(pcsAgsdSkill->m_nAffectedDOTCount >= pcsAgsdSkill->m_nTotalDOTCount)
		return FALSE;

	return TRUE;
}

// 2006.06.27. steeple
// 레벨 차이에 따라서 사용이 불가능한지 체크한다.
// 현재는(2006.06.27) 타입이 하나이다. 추가 되면 추가 작업해준다.
BOOL AgsmSkill::CheckLevelDiff(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	if(m_pagpmSkill->IsLevelDiffSkill(pcsSkillTemplate) && pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_LEVEL_DIFF][lSkillLevel] > 0.0f)
	{
		// 레벨 비교
		INT32 lLevel = m_pagpmCharacter->GetLevel((AgpdCharacter*)pcsSkill->m_pcsBase);
		INT32 lTargetLevel = m_pagpmCharacter->GetLevel((AgpdCharacter*)pcsTarget);

		// 타겟 레벨과 캐스터 레벨의 차이가 Const 값 보다 크다.
		if((lTargetLevel - lLevel) >= (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_LEVEL_DIFF][lSkillLevel])
			return FALSE;
	}

	return TRUE;
}

// 2006.11.27. steeple
// 버프 스킬에서 Factor 를 올릴 때 꼭 한번만 해야 하는 건지 체크.
BOOL AgsmSkill::CheckOnceAffected(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_stConditionArg[0].lArg4 == 1)
		return TRUE;
	
	return FALSE;
}

// 2007.10.23. steeple
// 커넥션 스킬인지 체크해준다.
BOOL AgsmSkill::SetConnectionInfo(AgpdSkill* pcsSkill)
{
	if(!pcsSkill || !pcsSkill->m_pcsBase || !pcsSkill->m_pcsTemplate)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = static_cast<AgpdSkillTemplate*>(pcsSkill->m_pcsTemplate);
	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	INT32 lActionOnActionType = m_pagpmSkill->GetActionOnActionType(pcsSkillTemplate);
	if(lActionOnActionType == 7)
	{
		// 상대방의 버프를 체크해서 해당 스킬이 버프중인지 확인해야 한다.
		pcsAgsdSkill->m_stConnectionInfo.m_bNeedCheck = TRUE;
		pcsAgsdSkill->m_stConnectionInfo.m_eConditionType = AGSDSKILL_CONNECTION_CONDITION_TYPE_BUFF;
		pcsAgsdSkill->m_stConnectionInfo.m_lConditionTID = (INT32) pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_CONNECTION_TID][lSkillLevel];
	}
	else if(lActionOnActionType == 8)
	{
		// 캐스터가 투명인지 체크한다.
		pcsAgsdSkill->m_stConnectionInfo.m_bNeedCheck = TRUE;
		pcsAgsdSkill->m_stConnectionInfo.m_eConditionType = AGSDSKILL_CONNECTION_CONDITION_TYPE_TRANSPARENT;
		pcsAgsdSkill->m_stConnectionInfo.m_lConditionTID = 0;	// 그냥 투명인 것만 체크하면 될듯.
	}

	return TRUE;
}

// 2007.10.23. steeple
// 커넥션 스킬을 실제로 적용해야 하는 지 리턴한다.
BOOL AgsmSkill::CheckConnectionInfo(AgpdSkill* pcsSkill, ApBase* pcsTarget, BOOL bProcessInterval)
{
	if(!pcsSkill)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	// 세팅되어 있지 않다면 안해도 된다.
	if(pcsAgsdSkill->m_stConnectionInfo.m_bNeedCheck == FALSE)
		return FALSE;

	// 이미 통과되었다면 return TRUE
	if(bProcessInterval && pcsAgsdSkill->m_stConnectionInfo.m_bAuthorized)
		return TRUE;

	if(pcsAgsdSkill->m_stConnectionInfo.m_eConditionType == AGSDSKILL_CONNECTION_CONDITION_TYPE_BUFF)
	{
		// 상대방의 버프중에서 컨디션에 해당하는 놈을 찾는다.
		if(pcsTarget && m_pagpmSkill->IsBuffedSkillByTID(pcsTarget, pcsAgsdSkill->m_stConnectionInfo.m_lConditionTID))
		{
			pcsAgsdSkill->m_stConnectionInfo.m_bAuthorized = TRUE;
			return TRUE;
		}
	}

	if(pcsAgsdSkill->m_stConnectionInfo.m_eConditionType == AGSDSKILL_CONNECTION_CONDITION_TYPE_TRANSPARENT)
	{
		// 시전자가 해당 스킬로 투명이 풀린 것인지 확인한다.
		// 아래 pcsSkill->m_pcsBase 는 NULL 이 될수도 있다. 그러니 조심하자.
		if(pcsSkill->m_pcsBase && IsReleaseTransparentSkillTID(pcsSkill->m_pcsBase, pcsSkill->m_pcsTemplate->m_lID))
		{
			pcsAgsdSkill->m_stConnectionInfo.m_bAuthorized = TRUE;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL AgsmSkill::ConAgsdSkill(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmSkill	*pThis				= (AgsmSkill *) pClass;
	AgpdSkill	*pcsSkill			= (AgpdSkill *) pData;
	AgsdSkill	*pcsAgsdSkill		= pThis->GetADSkill(pcsSkill);
	if (!pcsAgsdSkill)
		return FALSE;

	pcsSkill->m_ulEndTime			= 0;
	pcsSkill->m_ulInterval			= 0;
	pcsSkill->m_ulNextProcessTime	= 0;
	pcsSkill->m_ulStartTime			= 0;
	pcsSkill->m_ulCastDelay			= 0;
	pcsSkill->m_ulRecastDelay		= 0;
	pcsSkill->m_pSaveSkillData.Clear();

	/*
	pThis->m_pagpmFactors->InitFactor(&pcsAgsdSkill->m_csUpdateFactorPoint);
	pThis->m_pagpmFactors->InitFactor(&pcsAgsdSkill->m_csUpdateFactorPercent);
	pThis->m_pagpmFactors->InitFactor(&pcsAgsdSkill->m_csUpdateFactorResult);

	pThis->m_pagpmFactors->InitFactor(&pcsAgsdSkill->m_csUpdateFactorItemPoint);
	pThis->m_pagpmFactors->InitFactor(&pcsAgsdSkill->m_csUpdateFactorItemPercent);
	*/

	/*
	for (int i = 0; i < AGPMSKILL_MAX_AFFECTED_BASE; ++i)
	{
		pThis->m_pagpmFactors->InitFactor(pcsAgsdSkill->m_csModifyFactorPoint + i);
		pThis->m_pagpmFactors->InitFactor(pcsAgsdSkill->m_csModifyFactorPercent + i);

		pcsAgsdSkill->m_csModifyFactorPoint[i].m_bPoint		= TRUE;
		pcsAgsdSkill->m_csModifyFactorPercent[i].m_bPoint	= FALSE;
	}
	*/

	/*
	pcsAgsdSkill->m_pcsUpdateFactorPoint->m_bPoint	= TRUE;
	pcsAgsdSkill->m_pcsUpdateFactorPercent->m_bPoint	= FALSE;
	*/

	pcsAgsdSkill->m_lModifyCharLevel				= 0;
	pcsAgsdSkill->m_lModifySkillLevel				= 0;

	pcsAgsdSkill->m_nMeleeDefensePoint				= 0;
	pcsAgsdSkill->m_nMeleeReflectPoint				= 0;
	pcsAgsdSkill->m_nMagicDefensePoint				= 0;
	pcsAgsdSkill->m_nMagicReflectPoint				= 0;

	ZeroMemory(&pcsAgsdSkill->m_stBuffedSkillCombatEffectArg, sizeof(AgpdSkillBuffedCombatEffectArg));
	ZeroMemory(&pcsAgsdSkill->m_stBuffedSkillFactorEffectArg, sizeof(AgpdSkillBuffedFactorEffectArg));

	pcsAgsdSkill->m_ulLastCastTwiceSkillClock		= 0;
	pcsAgsdSkill->m_alLastTwiceSkillTargetCID.MemSetAll();
	pcsAgsdSkill->m_lLastTwiceSkillNumTarget		= 0;

	pcsAgsdSkill->m_eMissedReason					= AGSDSKILL_MISSED_REASON_NORMAL;
	pcsAgsdSkill->m_bApplyDamageOnly				= FALSE;
	pcsAgsdSkill->m_bDamageOnlyTemporary			= FALSE;

	pcsAgsdSkill->m_nAffectedDOTCount				= 0;
	pcsAgsdSkill->m_nTotalDOTCount					= 0;

	pcsAgsdSkill->m_stInvincibleInfo.clear();

	pcsAgsdSkill->m_stConnectionInfo.init();

	return TRUE;
}

BOOL AgsmSkill::DesAgsdSkill(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmSkill	*pThis = (AgsmSkill *) pClass;
	AgsdSkill	*pcsAgsdSkill = pThis->GetADSkill((AgpdSkill *) pData);

	pThis->DestroyUpdateFactorPoint(pcsAgsdSkill);
	pThis->DestroyUpdateFactorPercent(pcsAgsdSkill);
	pThis->DestroyUpdateFactorResult(pcsAgsdSkill);
	pThis->DestroyUpdateFactorItemPoint(pcsAgsdSkill);
	pThis->DestroyUpdateFactorItemPercent(pcsAgsdSkill);

	/*
	for (int i = 0; i < AGPMSKILL_MAX_AFFECTED_BASE; ++i)
	{
		pThis->m_pagpmFactors->DestroyFactor(pcsAgsdSkill->m_csModifyFactorPoint + i);
		pThis->m_pagpmFactors->DestroyFactor(pcsAgsdSkill->m_csModifyFactorPercent + i);
	}
	*/

	return TRUE;
}

BOOL AgsmSkill::ConAgsdSkillADBase(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmSkill		*pThis		= (AgsmSkill *)			pClass;
	AgsdSkillADBase	*pcsADBase	= pThis->GetADBase((ApBase *) pData);
	if (!pcsADBase)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		pcsADBase->m_csBuffedSkillProcessInfo[i].m_csSkillOwner.m_eType		= APBASE_TYPE_NONE;
		pcsADBase->m_csBuffedSkillProcessInfo[i].m_csSkillOwner.m_lID		= 0;
		pcsADBase->m_csBuffedSkillProcessInfo[i].m_lMasteryPoint			= 0;

		pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill				= NULL;

		pcsADBase->m_pstMasteryInfo											= NULL;

		pcsADBase->m_ppcsSkillItem.MemSetAll();
	}

	for (int i = 0; i < AGSMSKILL_MAX_UNION_CONTROL_COUNT; ++i)
	{
		pcsADBase->m_UnionControlArray.m_astUnionControlInfo[i].init();
	}

	pcsADBase->m_ulLastReleasedTransparentClock = 0;
	pcsADBase->m_lReleaseTransparentSkillTID = 0;

	return TRUE;
}

BOOL AgsmSkill::DesAgsdSkillADBase(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmSkill		*pThis		= (AgsmSkill *)			pClass;
	AgsdSkillADBase	*pcsADBase	= pThis->GetADBase((ApBase *) pData);
	if (!pcsADBase)
		return FALSE;

	int i = 0;
	for (i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		/*
		pThis->m_pagpmFactors->DestroyFactor(&pcsADBase->m_csBuffedSkillProcessInfo[i].m_csModifyFactorPoint);
		pThis->m_pagpmFactors->DestroyFactor(&pcsADBase->m_csBuffedSkillProcessInfo[i].m_csModifyFactorPercent);
		*/

		if (pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
		{
			pThis->m_pagpmSkill->DestroySkill(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);

			pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill	= NULL;
		}
	}

	if (pcsADBase->m_pstMasteryInfo)
	{
		delete pcsADBase->m_pstMasteryInfo;
		pcsADBase->m_pstMasteryInfo			= NULL;
	}

	for (i = 0; i < AGSMSKILL_MAX_TEMPLATE_SKILL_BUFFER; ++i)
	{
		if (pcsADBase->m_ppcsSkillItem[i])
		{
			pThis->m_pagpmSkill->DestroySkill(pcsADBase->m_ppcsSkillItem[i]);
			pcsADBase->m_ppcsSkillItem[i]			= NULL;
		}
	}

	return TRUE;
}

AgsdSkill* AgsmSkill::GetADSkill(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return NULL;

	return (AgsdSkill *) m_pagpmSkill->GetAttachedModuleData(m_nIndexADSkill, pcsSkill);
}

AgsdSkillADBase* AgsmSkill::GetADBase(ApBase *pcsBase)
{
	if (!pcsBase)
		return NULL;

	switch (pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		return (AgsdSkillADBase *) m_pagpmCharacter->GetAttachedModuleData(m_nIndexADCharacter, (AgpdCharacter *) pcsBase);
		break;

	case APBASE_TYPE_OBJECT:
		return (AgsdSkillADBase *) m_papmObject->GetAttachedModuleData(m_nIndexADObject, (ApdObject *) pcsBase);
		break;
	}

	return NULL;
}

AgpdSkill* AgsmSkill::GetTempSkill(ApBase *pcsBase, INT32 lSkillTID)
{
	if (!pcsBase || lSkillTID == AP_INVALID_SKILLID)
		return NULL;

	switch (pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			AgsdSkillADBase	*pcsAttachData	= GetADBase(pcsBase);

			int i = 0 ;
			for (i = 0; i < AGSMSKILL_MAX_TEMPLATE_SKILL_BUFFER; ++i)
			{
				if (!pcsAttachData->m_ppcsSkillItem[i])
				{
					pcsAttachData->m_ppcsSkillItem[i]	= m_pagpmSkill->CreateSkill();

					if (pcsAttachData->m_ppcsSkillItem[i])
						EnumCallback(AGSMSKILL_CB_GET_NEW_SKILLID, &pcsAttachData->m_ppcsSkillItem[i]->m_lID, NULL);
					else
						return NULL;

					break;
				}
				else
				{
					if (pcsAttachData->m_ppcsSkillItem[i]->m_pcsTemplate->m_lID == lSkillTID ||
						(pcsAttachData->m_ppcsSkillItem[i]->m_eStatus == AGPMSKILL_STATUS_NOT_CAST &&
						 pcsAttachData->m_ppcsSkillItem[i]->m_ulRecastDelay < GetClockCount()))
						break;
				}
			}

			if (i == AGSMSKILL_MAX_TEMPLATE_SKILL_BUFFER)
				return NULL;

			if (pcsAttachData->m_ppcsSkillItem[i])
				pcsAttachData->m_ppcsSkillItem[i]->m_lTargetUnion	= m_pagpmCharacter->GetUnion(m_pagpmFactors->GetRace(&((AgpdCharacter *) pcsBase)->m_csFactor));

			return pcsAttachData->m_ppcsSkillItem[i];
		}
		break;
	}

	return NULL;
}

BOOL AgsmSkill::CastSkill(AgpdSkill *pcsSkill, ApBase csTargetBase, AuPOS posTarget, INT32 lTargetUnion, BOOL bForceAttack)
{
	if (!pcsSkill)
		return FALSE;

	pcsSkill->m_csTargetBase.m_eType	= csTargetBase.m_eType;
	pcsSkill->m_csTargetBase.m_lID		= csTargetBase.m_lID;

	pcsSkill->m_lTargetUnion			= lTargetUnion;

	pcsSkill->m_posTarget				= posTarget;

	pcsSkill->m_bForceAttack			= bForceAttack;

	AgsdSkill	*pcsAgsdSkill			= GetADSkill(pcsSkill);
	pcsAgsdSkill->m_lSkillScrollIID		= AP_INVALID_IID;

	return CastSkill(pcsSkill);
}

BOOL AgsmSkill::CastSkill(ApBase *pcsBase, INT32 lSkillTID, INT32 lSkillLevel, ApBase *pcsTargetBase, BOOL bForceAttack, BOOL bMonsterUse, INT32 lSkillScrollIID)
{
	AgsdCastSkill pcsAgsdCastSkill;
	pcsAgsdCastSkill.pcsBase			= pcsBase;
	pcsAgsdCastSkill.lSkillTID			= lSkillTID;
	pcsAgsdCastSkill.lSkillLevel		= lSkillLevel;
	pcsAgsdCastSkill.pcsTargetBase		= pcsTargetBase;
	pcsAgsdCastSkill.bForceAttack		= bForceAttack;
	pcsAgsdCastSkill.bMonsterUse		= bMonsterUse;
	pcsAgsdCastSkill.lSkillScrollIID	= lSkillScrollIID;

	return CastSkill(pcsAgsdCastSkill);
	/*if (!pcsBase || lSkillTID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillTemplate	*pcsSkillTemplate	= m_pagpmSkill->GetSkillTemplate(lSkillTID);
	if (!pcsSkillTemplate)
		return FALSE;

	AgpdSkill	*pcsSkill	= GetTempSkill(pcsBase, lSkillTID);
	if (!pcsSkill)
		return FALSE;

	pcsSkill->m_bForceAttack	= bForceAttack;

	if (pcsTargetBase)
	{
		pcsSkill->m_csTargetBase.m_eType	= pcsTargetBase->m_eType;
		pcsSkill->m_csTargetBase.m_lID		= pcsTargetBase->m_lID;
	}

	pcsSkill->m_lActivedSkillPoint		= lSkillLevel;
	pcsSkill->m_pcsBase					= pcsBase;
	pcsSkill->m_pcsTemplate				= pcsSkillTemplate;

	if ((pcsSkillTemplate->m_lTargetType & AGPMSKILL_TARGET_SELF_ONLY))
	{
		pcsSkill->m_csTargetBase.m_eType	= pcsBase->m_eType;
		pcsSkill->m_csTargetBase.m_lID		= pcsBase->m_lID;
	}

	m_pagpmFactors->InitFactor(&pcsSkill->m_csFactor);

	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);
	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_POINT);

	AgsdSkill	*pcsAgsdSkill			= GetADSkill(pcsSkill);
	pcsAgsdSkill->m_bIsCastSkill		= FALSE;
	pcsAgsdSkill->m_lSkillScrollIID		= lSkillScrollIID;

	CastSkill(pcsSkill, bMonsterUse);

	return pcsAgsdSkill->m_bIsCastSkill;*/
}

BOOL AgsmSkill::CastSkill(ApBase *pcsBase, INT32 lSkillTID, INT32 lSkillLevel, AuPOS *posDest, BOOL bForceAttack)
{
	if (!pcsBase || lSkillTID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillTemplate	*pcsSkillTemplate	= m_pagpmSkill->GetSkillTemplate(lSkillTID);
	if (!pcsSkillTemplate)
		return FALSE;

	AgpdSkill	*pcsSkill	= GetTempSkill(pcsBase, lSkillTID);
	if (!pcsSkill)
		return FALSE;

	pcsSkill->m_bForceAttack	= bForceAttack;

	if (posDest)
		pcsSkill->m_posTarget	= *posDest;

	pcsSkill->m_csTargetBase.m_eType	= APBASE_TYPE_NONE;
	pcsSkill->m_csTargetBase.m_lID		= 0;

	pcsSkill->m_lActivedSkillPoint		= lSkillLevel;
	pcsSkill->m_pcsBase					= pcsBase;
	pcsSkill->m_pcsTemplate				= pcsSkillTemplate;

	m_pagpmFactors->InitFactor(&pcsSkill->m_csFactor);

	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);
	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_POINT);

	AgsdSkill	*pcsAgsdSkill			= GetADSkill(pcsSkill);
	pcsAgsdSkill->m_bIsCastSkill		= FALSE;
	pcsAgsdSkill->m_lSkillScrollIID		= AP_INVALID_IID;

	CastSkill(pcsSkill);

	return pcsAgsdSkill->m_bIsCastSkill;
}

BOOL AgsmSkill::CastSkill(AgsdCastSkill& pAgsdCastSkill)
{
	if (!pAgsdCastSkill.pcsBase || pAgsdCastSkill.lSkillTID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillTemplate	*pcsSkillTemplate	= m_pagpmSkill->GetSkillTemplate(pAgsdCastSkill.lSkillTID);
	if (!pcsSkillTemplate)
		return FALSE;

	AgpdSkill	*pcsSkill	= GetTempSkill(pAgsdCastSkill.pcsBase, pAgsdCastSkill.lSkillTID);
	if (!pcsSkill)
		return FALSE;

	pcsSkill->m_bForceAttack	= pAgsdCastSkill.bForceAttack;

	if (pAgsdCastSkill.pcsTargetBase)
	{
		pcsSkill->m_csTargetBase.m_eType	= pAgsdCastSkill.pcsTargetBase->m_eType;
		pcsSkill->m_csTargetBase.m_lID		= pAgsdCastSkill.pcsTargetBase->m_lID;
	}

	pcsSkill->m_lActivedSkillPoint		= pAgsdCastSkill.lSkillLevel;
	pcsSkill->m_pcsBase					= pAgsdCastSkill.pcsBase;
	pcsSkill->m_pcsTemplate				= pcsSkillTemplate;

	if ((pcsSkillTemplate->m_lTargetType & AGPMSKILL_TARGET_SELF_ONLY))
	{
		pcsSkill->m_csTargetBase.m_eType	= pAgsdCastSkill.pcsBase->m_eType;
		pcsSkill->m_csTargetBase.m_lID		= pAgsdCastSkill.pcsBase->m_lID;
	}

	pcsSkill->m_pSaveSkillData = pAgsdCastSkill.m_pSaveSkillData;

	m_pagpmFactors->InitFactor(&pcsSkill->m_csFactor);
	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, pAgsdCastSkill.lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);
	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, pAgsdCastSkill.lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_POINT);

	AgsdSkill	*pcsAgsdSkill			= GetADSkill(pcsSkill);
	pcsAgsdSkill->m_bIsCastSkill		= FALSE;
	pcsAgsdSkill->m_lSkillScrollIID		= pAgsdCastSkill.lSkillScrollIID;

	CastSkill(pcsSkill, pAgsdCastSkill.bMonsterUse);

	return pcsAgsdSkill->m_bIsCastSkill;
}

//		CastSkill
//	Functions
//		- pcsSkill 을 사용한다.
//			1. m_alUseSkillID[]에 추가한다.
//			2. 스킬을 발동하기까지의 딜레이를 구한다.
//			3. 위에서 구한 딜레이가 0이 아니면 AGPMSKILL_STATUS_CAST_DELAY로 0이면 AGPMSKILL_STATUS_CAST로 상태변경한다.
//				(만약, 상태가 AGPMSKILL_STATUS_CAST로 변경되면 pcsSkill의 기능을 적용한다.)
//			4. pcsSkill 소유주가 ApmEventManager에 APDEVENT_FUNCTION_SKILL로 등록되어 있지 않다면 등록한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::CastSkill(AgpdSkill *pcsSkill, BOOL bMonsterUse)
{
	PROFILE("AgsmSkill::CastSkill");

	if (!pcsSkill || !pcsSkill->m_pcsBase || !pcsSkill->m_pcsTemplate)
		return FALSE;

	CHAR szTmp[64];
	sprintf(szTmp, "CastSkill;; CTID:%d, STID:%d", ((AgpdCharacter*)pcsSkill->m_pcsBase)->m_pcsCharacterTemplate->m_lID,
													pcsSkill->m_pcsTemplate->m_lID);
	//STOPWATCH2(GetModuleName(), szTmp);

	AgpdSkillTemplate *pcsSkillTemplate = (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

	// Check RegionPeculiarity (그 지역에서 쓸수있는 스킬인지 아닌지를 판단)
	if(m_papmMap->CheckRegionPerculiarity(((AgpdCharacter*)pcsSkill->m_pcsBase)->m_nBindingRegionIndex, APMMAP_PECULIARITY_SKILL, pcsSkillTemplate->m_lID) == APMMAP_PECULIARITY_RETURN_DISABLE_USE)
	{
		if(m_pagsmSystemMessage)
			m_pagsmSystemMessage->SendSystemMessage(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase),
			AGPMSYSTEMMESSAGE_CODE_DISABLE_USE_THIS_REGION);

		return FALSE;
	}

	// 임시적으로 무장해제스킬(독핸드, 인엑서러블 스트라이크, 임파시블 웨폰 소울) 사용불가 처리
	if( pcsSkillTemplate->m_lSpecialStatus & AGPMSKILL_SPECIAL_STATUS_DISARMAMENT )
	{
		if(m_pagsmSystemMessage)
			m_pagsmSystemMessage->SendSystemMessage(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase),
			AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS);

		return FALSE;
	}

	// Before Process Skill Packet Check Skill disturbing
	//JK_패시브스킬이 사라지는 현상..패시브스킬의 recasting되야하는 경우 disturbing에 걸려 casting 안되는 현상이 있다.
	if(!m_pagpmSkill->IsPassiveSkill(pcsSkillTemplate) )
	{
		if(m_pagpmCharacter->CheckEnableActionCharacter((AgpdCharacter*)pcsSkill->m_pcsBase, AGPDCHAR_DISTURB_ACTION_SKILL) == FALSE)
		{
			return FALSE;
		}
	}

	// Check RegionPeculiarity (해당 지역에 있는 타겟에게 버프를 줄수 있는지 없는지 여부)
	if (pcsSkill->m_csTargetBase.m_lID != 0 && pcsSkill->m_pcsBase->m_lID != pcsSkill->m_csTargetBase.m_lID)
	{
		if(m_papmMap->CheckRegionPerculiarity(((AgpdCharacter*)pcsSkill->m_pcsBase)->m_nBindingRegionIndex, APMMAP_PECULIARITY_REMOTE_BUFF) == APMMAP_PECULIARITY_RETURN_DISABLE_USE)
		{
			if(m_pagsmSystemMessage)
				m_pagsmSystemMessage->SendSystemMessage(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase),
				AGPMSYSTEMMESSAGE_CODE_DISABLE_USE_THIS_REGION);

			return FALSE;
		}
	}

	// CashItem Skill, Passive Skill은 ActionBlockCondition 체크하지 않는다. 2006.01.23. steeple
	// 투명 스킬도 체크하지 않는다. 원래는 ActionBlock 을 주지 않았지만, 주고 체크안하는 방식으로 변경. 2006.02.03. steeple
	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		if (m_pagpmCharacter->IsAllBlockStatus((AgpdCharacter *) pcsSkill->m_pcsBase))
			return FALSE;

		if (m_pagpmSkill->IsCashSkill(pcsSkillTemplate) == FALSE &&
			m_pagpmSkill->IsPassiveSkill(pcsSkillTemplate) == FALSE &&
			m_pagpmSkill->IsTransparentSkill(pcsSkillTemplate) == FALSE)
		{
			if (m_pagpmCharacter->IsActionBlockCondition((AgpdCharacter *) pcsSkill->m_pcsBase, AGPDCHAR_ACTION_BLOCK_TYPE_SKILL))
				return FALSE;
		}
	}

	AgsdSkill	*pcsAgsdSkill = GetADSkill(pcsSkill);
	if (!pcsAgsdSkill)
		return FALSE;

	AgpdSkillAttachData	*pcsSkillAttachData = m_pagpmSkill->GetAttachSkillData(pcsSkill->m_pcsBase);
	if (!pcsSkillAttachData)
		return FALSE;

	// 패시브 스킬이 아니고 감옥이라면 스킬 불가. 2007.05.04. steeple
	if(!m_pagpmSkill->IsPassiveSkill(pcsSkillTemplate) && m_pagpmCharacter->IsInJail((AgpdCharacter*)pcsSkill->m_pcsBase))
		return FALSE;

	UINT32 ulCurrentClock = GetClockCount();

	// 2006.10.24. steeple
	// Force Skill 이면 강제 공격 세팅해준다.
	SetForceSkill(pcsSkill);

	if (!pcsSkillTemplate->m_bShrineSkill)
	{
		// 각종 사항에 대한 체킹을 한다. (여기부터)
		//////////////////////////////////////////////////////////////////////////////////////////////////

		//몬스터가 사용하는 스킬을 경우. 만족 조건을 뺀다.
		if( bMonsterUse == TRUE )
		{
		}
		//그렇지 않은 경우.
		else
		{
			if (!m_pagpmSkill->IsSatisfyRequirement(pcsSkill))
			{
				SendCastSkill(pcsSkill, AGPMSKILL_ACTION_NEED_REQUIREMENT, NULL, 0);
				return FALSE;
			}
		}

		eAgpmSkillActionType	eActionType	= AGPMSKILL_ACTION_NOT_ENOUGH_HP;
		// hp, mp, sp등의 소모되는 cost 만큼이 있는지 검사한다.
		if (!m_pagpmSkill->IsSufficientCost(pcsSkill, &eActionType))
		{
			/*
			switch (eActionType) {
			case AGPMSKILL_ACTION_NOT_ENOUGH_MP:
				{
					AgpdItem	*pcsMPPotion	= m_pagpmItem->GetMPPotionInInventory((AgpdCharacter *) pcsSkill->m_pcsBase);

					if (pcsMPPotion)
						m_pagpmItem->UseItem(pcsMPPotion);
				}
				break;

			case AGPMSKILL_ACTION_NOT_ENOUGH_SP:
				{
					AgpdItem	*pcsSPPotion	= m_pagpmItem->GetSPPotionInInventory((AgpdCharacter *) pcsSkill->m_pcsBase);

					if (pcsSPPotion)
						m_pagpmItem->UseItem(pcsSPPotion);
				}
				break;
			}

			if (!m_pagpmSkill->IsSufficientCost(pcsSkill, &eActionType))
			*/
			{
				SendCastSkill(pcsSkill, eActionType, NULL, 0);
				return FALSE;
			}
		}

		// 2005.08.26. steeple
		// Stun 상태에서 Skill 사용 못한다. 패시브스킬은 사용이 가능해야 한다(아이템 착용에 따른 팩터계산때 패시브스킬로 이용한다)
		if( (((AgpdCharacter*)pcsSkill->m_pcsBase)->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_STUN) 
			&& !m_pagpmSkill->IsPassiveSkill(pcsSkill))
			return FALSE;

		// 2007.10.09. steeple
		// Sleep 상태에서 Skill 사용 못한다.
		if(m_pagpmCharacter->IsStatusSleep((AgpdCharacter*)pcsSkill->m_pcsBase))
			return FALSE;

		// 2005.10.17. steeple
		// Halt 상태에서는 Attack Skill (Debuff 포함) 을 사용 못한다.
		if(((AgpdCharacter*)pcsSkill->m_pcsBase)->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_HALT)
		{
			if(!HaltStatusCheck(pcsSkill))
				return FALSE;
		}

		// 2005.12.26. steeple
		// 캐쉬아이템 중에서는 탈 것 타고 있어도 써지는 놈이 있다.
		// 그 외에는 불가능.
		if(((AgpdCharacter*)pcsSkill->m_pcsBase)->m_bRidable)
		{
			// Passive Skill 은 탈것 타고 있어도 되야 한다. 2007.02.14. steeple
			if(!m_pagpmSkill->IsPassiveSkill(pcsSkillTemplate) &&
				!m_pagpmSkill->IsRideSkill(pcsSkillTemplate) &&
				IsEnableCashItemSkillOnRide(pcsSkill) == FALSE)
				return FALSE;
		}

		// 2005.10.04. steeple
		// 투명 상태에서는 스킬 사용 시 투명이 풀림.
		if(m_pagpmCharacter->IsStatusTransparent((AgpdCharacter*)pcsSkill->m_pcsBase))
		{
			// 2007.08.31. steeple
			// 패시브 스킬은 투명 풀지 않는다.
			//
			// 2005.12.28. steeple
			// 특수 상황에서 사용가능해야 하는 스킬을 제외하고 투명을 풀어준다.
			if(m_pagpmSkill->IsPassiveSkill(pcsSkillTemplate) == FALSE && IsEnableCashItemSkillOnRide(pcsSkill) == FALSE)
			{
				m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*)pcsSkill->m_pcsBase, AGPDCHAR_SPECIAL_STATUS_TRANSPARENT);

				// 2007.11.14. steeple
				// Connection Skill 이 투명 관련 이라면, 투명이 풀렸다고 세팅한다.
				if(m_pagpmSkill->GetActionOnActionType(pcsSkillTemplate) == 8)
					SetReleaseTransparent(pcsSkill);
			}
		}

		// 2007.11.14. steeple
		// 전투 중일 때도 투명 스킬 캐스팅 할 수 있는 스킬이 생겼다.
		//
		// 2005.10.04. steeple
		// 전투 중일 때 투명 스킬을 사용할 수 없다.
		if(m_pagpmSkill->IsTransparentSkill(pcsSkillTemplate) && m_pagpmSkill->IsForceTransparentSkill(pcsSkillTemplate) == FALSE)
		{
			// 2005.10.04. steeple
			if(m_pagpmCharacter->IsCombatMode((AgpdCharacter*)pcsSkill->m_pcsBase))
			{
				if(m_pagsmSystemMessage)
					m_pagsmSystemMessage->SendSystemMessage(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase),
															AGPMSYSTEMMESSAGE_CODE_TRANSPARENT_FAILURE_FOR_COMBAT);

				pcsAgsdSkill->m_eMissedReason = AGSDSKILL_MISSED_REASON_CANNOT_TRANSPARENT_FOR_COMBAT;
				return FALSE;
			}
		}

		// 이미 사용중인지 검사
		if (pcsSkill->m_eStatus != AGPMSKILL_STATUS_NOT_CAST)	// 이미 사용중인 스킬이다. 걍 리턴한다.
		{
			SendCastSkill(pcsSkill, AGPMSKILL_ACTION_ALREADY_USE_SKILL, NULL, 0);
			return TRUE;
		}

		// recast delay가 지났는지 검사
		if (!pcsSkillTemplate->m_bTwicePacket && pcsSkill->m_ulRecastDelay > ulCurrentClock)		// 아직 다시 시연할 수 있는 시간이 아니다.
		{
			SendCastSkill(pcsSkill, AGPMSKILL_ACTION_NOT_READY_CAST, NULL, 0);
			return TRUE;
		}
		else if(pcsSkillTemplate->m_bTwicePacket)
		{
			// 2006.07.19. steeple
			// Recast Delay 체크 추가.
			if(pcsSkill->m_ulRecastDelay > ulCurrentClock && !IsSetTwicePacketSkill(pcsSkill))
			{
				SendCastSkill(pcsSkill, AGPMSKILL_ACTION_NOT_READY_CAST, NULL, 0);
				return TRUE;
			}

			//// 2006.02.06. steeple
			//// 버퍼에 없을 때 RecastDelay 검사로 변경.
			//// 버퍼에 있을 때는 추가적으로 버퍼에 쌓기만 하면 된다.
			//if(!IsSetTwicePacketSkill(pcsSkill) && pcsSkill->m_ulRecastDelay > ulCurrentClock)
			//{
			//	SendCastSkill(pcsSkill, AGPMSKILL_ACTION_NOT_READY_CAST, NULL, 0);
			//	return TRUE;
			//}

			//// 2004.11.09. steeple
			//// Twice Packet 이면 Delay 검사를 다르게 한다.
			//// Recast Delay 를.... 흠... -_-;; 대략 잘 처리해준다.

			//// 2005.03.27. steeple
			//// 이미 Twice Packet Buffer 에 시전되어 있는데 RecastDelay 전에 패킷이 날아왔다면
			//if(IsSetTwicePacketSkill(pcsSkill) && pcsSkill->m_ulRecastDelay > ulCurrentClock)
			//{
			//	SendCastSkill(pcsSkill, AGPMSKILL_ACTION_NOT_READY_CAST, NULL, 0);
			//	return TRUE;
			//}
		}

		// 스킬을 다시 쓸 수 있는지 (모든 스킬에 적용되는 시간이 세팅되어 있는경우) 검사
		if (!m_pagpmSkill->IsPassiveSkill(pcsSkill) && pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
		{
			AgsdCharacter *pcsAgsdCharacter = m_pagsmCharacter->GetADCharacter((AgpdCharacter *) pcsSkill->m_pcsBase);
			if (!pcsAgsdCharacter)
				return FALSE;

			if (pcsAgsdCharacter->m_ulNextSkillTime > ulCurrentClock)
			{
				SendCastSkill(pcsSkill, AGPMSKILL_ACTION_NOT_READY_CAST, NULL, 0);
				return FALSE;
			}
		}

		// 2005.09.01. steeple
		// Summons 관련 스킬이라면 Max Summons 체크를 여기서 해본다.
		if(m_pagpmSkill->IsSummonsSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
		{
			// It is impossible when the character is resurrecting. 2008.06.24. steeple
			// 포탈 등의 로딩창이 뜨는 이동 중에는 변신물약 못 쓴다. 2008.04.08. steeple
			if(m_pagsmCharacter->IsNotLogoutStatus((AgpdCharacter*)pcsSkill->m_pcsBase) &&
				m_pagsmCharacter->IsResurrectingNow((AgpdCharacter*)pcsSkill->m_pcsBase) == FALSE)
				return FALSE;

			// AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE9 즉 둘다 만족하는 거는 일단 MAX수치를 확인하지 않고 넘어간다
			if(!(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE4 &&
				pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE7))
			{
				eAgpmSummonsSummonsCheckResult eCheckResult = m_pagpmSummons->IsSummonsEnable((AgpdCharacter*)pcsSkill->m_pcsBase, pcsSkill->m_bForceAttack);

				if(eCheckResult != AGPMSUMMONS_SUMMONS_CHECK_RESULT_SUCCESS)
				{
					m_pagsmSummons->SendSummonsCheckResult(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase), eCheckResult);
					SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
					return FALSE;
				}
			}

			INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
			if(!lSkillLevel)
				return FALSE;

			eAgpmSummonsSummonsCheckResult eCheckResult = m_pagpmSummons->IsSummonsEnableRegion((AgpdCharacter*)pcsSkill->m_pcsBase,
												(INT32)((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_SUMMONS_TID][lSkillLevel]);
			if(eCheckResult != AGPMSUMMONS_SUMMONS_CHECK_RESULT_SUCCESS)
			{
				m_pagsmSummons->SendSummonsCheckResult(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase), eCheckResult);
				SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
				return FALSE;
			}

			if ( ((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE11 ) 
			{
				if ( m_pagpmShrineBattle->IsInShrineRegionByIndex((AgpdCharacter*)pcsSkill->m_pcsBase)  )
				{
					eCheckResult = AGPMSUMMONS_SUMMONS_CHECK_RESULT_SUCCESS;
				}
				else
				{
					eCheckResult = AGPMSUMMONS_SUMMONS_CHECK_RESULT_FAILURE_SUMMON_ITEM;
				}
			}

			if(eCheckResult != AGPMSUMMONS_SUMMONS_CHECK_RESULT_SUCCESS)
			{
				m_pagsmSummons->SendSummonsCheckResult(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase), eCheckResult);
				SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
				return FALSE;
			}
		}

		// 2005.10.05. steeple
		// Fiexed Summons 관련 스킬이라면 Max Fixed Summons 체크를 여기서 해본다.
		if(m_pagpmSkill->IsFixedSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) && m_pagpmSummons && m_pagsmSummons)
		{
			// It is impossible when the character is resurrecting. 2008.06.24. steeple
			// 포탈 등의 로딩창이 뜨는 이동 중에는 변신물약 못 쓴다. 2008.04.08. steeple
			if(m_pagsmCharacter->IsNotLogoutStatus((AgpdCharacter*)pcsSkill->m_pcsBase) &&
				m_pagsmCharacter->IsResurrectingNow((AgpdCharacter*)pcsSkill->m_pcsBase) == FALSE)
				return FALSE;

			eAgpmSummonsFixedCheckResult eCheckResult = m_pagpmSummons->IsFixedEnable((AgpdCharacter*)pcsSkill->m_pcsBase);
			if(eCheckResult != AGPMSUMMONS_FIXED_CHECK_RESULT_SUCCESS)
			{
				m_pagsmSummons->SendFixedCheckResult(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase), eCheckResult);
				SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
				return FALSE;
			}

			// 각 TID 별 체크도 한다.
			INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
			if(!lSkillLevel)
				return FALSE;

			INT32 lFixedTID = (INT32)((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_SUMMONS_TID][lSkillLevel];
			INT32 lMaxEachCount = (INT32)((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_LIMIT_QUANTITY][lSkillLevel];

			// Owner 의 Fixed List 에서 해당 TID 의 개수 체크를 한다.
			INT32 lCurrentEachCount = m_pagpmSummons->GetEachFixedCount(static_cast<AgpdCharacter*>(pcsSkill->m_pcsBase), lFixedTID);
			if(lCurrentEachCount >= lMaxEachCount)
			{
				m_pagsmSummons->SendFixedCheckResult(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase), AGPMSUMMONS_FIXED_CHECK_RESULT_FAILURE_MAX_COUNT);
				SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
				return FALSE;
			}
		}

		// 2006.01.09. steeple
		// 패시브가 아니고, 공격 스킬이고
		if(!m_pagpmSkill->IsPassiveSkill(pcsSkill))
		{
			if(m_pagpmCharacter->IsPC((AgpdCharacter*)pcsSkill->m_pcsBase) &&
				m_pagpmCharacter->IsStatusInvincible((AgpdCharacter*)pcsSkill->m_pcsBase) &&
				pcsSkill->m_csTargetBase.m_lID != pcsSkill->m_pcsBase->m_lID)	// 자신에게 사용하는 스킬은 무적해제 하지않는다 (버프,아바타아이템스킬 사용시 무적문제)
			{
				// 시전자가 PC 이고, 무적 상태라면 풀어준다.
				m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*)pcsSkill->m_pcsBase, AGPDCHAR_SPECIAL_STATUS_INVINCIBLE);
			}
			else if(m_pagpmCharacter->IsStatusSummoner((AgpdCharacter*)pcsSkill->m_pcsBase) ||
					m_pagpmCharacter->IsStatusTame((AgpdCharacter*)pcsSkill->m_pcsBase))
			{
				// 공격자가 소환수나 테이밍인데, 주인이 무적이라면 풀어준다. 2006.01.11. steeple
				AgpdCharacter* pcsOwner = m_pagpmCharacter->GetCharacter(m_pagpmSummons->GetOwnerCID((AgpdCharacter*)pcsSkill->m_pcsBase));
				if(pcsOwner && !pcsOwner->m_bIsReadyRemove)
				{
					AuAutoLock csLock(pcsOwner->m_Mutex);
					if (csLock.Result())
					{
						if(m_pagpmCharacter->IsStatusInvincible(pcsOwner))
							m_pagpmCharacter->UpdateUnsetSpecialStatus(pcsOwner, AGPDCHAR_SPECIAL_STATUS_INVINCIBLE);
					}
				}
			}
		}

		if(m_pagpmSkill->IsEvolutionSkill(pcsSkillTemplate))
		{
			AgpdCharacter* pcsCharacter = (AgpdCharacter*)pcsSkill->m_pcsBase;
			if((pcsSkill->m_pSaveSkillData.eStep == SAVESKILL_NONE)
				&& !CheckEnableEvolution(pcsCharacter, pcsSkill))
			{
				SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
				return FALSE;
			}
		}


		BOOL	bMoveCharacter	= FALSE;
		AuPOS			pstCalcPos;

		if (!m_pagpmSkill->IsPassiveSkill(pcsSkill))
		{
			// 타겟이 제대로 된 넘인지 검사한다.
			AgpmSkillCheckTargetResult	eCheckTargetResult;

			AgpdCharacter		*pcsTargetCharacter	= NULL;

			if (pcsSkill->m_csTargetBase.m_lID != 0 && pcsSkill->m_pcsBase->m_lID != pcsSkill->m_csTargetBase.m_lID)
			{
				INT32			alCID[2];
				AgpdCharacter	*pacsCharacter[2];

				AuPOS			stTargetPos;

				alCID[0]		= pcsSkill->m_pcsBase->m_lID;
				alCID[1]		= pcsSkill->m_csTargetBase.m_lID;

				/*
				if (pcsSkill->m_pcsBase->m_lID == pcsSkill->m_csTargetBase.m_lID ||
					pcsSkill->m_csTargetBase.m_lID == AP_INVALID_CID)
				{
					eCheckTargetResult = m_pagpmSkill->IsValidTarget(pcsSkill, pcsSkill->m_pcsBase, pcsSkill->m_bForceAttack, &pstCalcPos);
				}
				*/
				//else
				//{
					pcsSkill->m_pcsBase->m_Mutex.Release();

					if (!m_pagpmCharacter->GetCharacterLock(alCID, 2, pacsCharacter))
					{
						pcsSkill->m_pcsBase->m_Mutex.WLock();
						return FALSE;
					}

					if (pcsSkill->m_pcsBase->m_lID == pacsCharacter[0]->m_lID)
					{
						eCheckTargetResult = m_pagpmSkill->IsValidTarget(pcsSkill, pacsCharacter[1], pcsSkill->m_bForceAttack, &pstCalcPos);

						stTargetPos	= pacsCharacter[1]->m_stPos;

						pcsTargetCharacter	= pacsCharacter[1];
					}
					else
					{
						eCheckTargetResult = m_pagpmSkill->IsValidTarget(pcsSkill, pacsCharacter[0], pcsSkill->m_bForceAttack, &pstCalcPos);

						stTargetPos	= pacsCharacter[0]->m_stPos;

						pcsTargetCharacter	= pacsCharacter[0];
					}
				//}

				//if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
				//	m_pagpmCharacter->TurnCharacter((AgpdCharacter *) pcsSkill->m_pcsBase, 0, m_pagpmCharacter->GetSelfCharacterTurnAngle((AgpdCharacter *) pcsSkill->m_pcsBase, &stTargetPos));
			}
			else
			{
				eCheckTargetResult = m_pagpmSkill->IsValidTarget(pcsSkill, pcsSkill->m_pcsBase, pcsSkill->m_bForceAttack, &pstCalcPos);
			}

			// 2004.09.21. steeple.
			// Skill 이 Debuff 형이라면, 해당 타켓에게 쓸 수 있는 지 검사한다.
			if(pcsTargetCharacter)
			{
				if(!IsDebuffSkillEnable(pcsSkill, pcsTargetCharacter))
				{
					eCheckTargetResult = AGPMSKILL_CHECK_TARGET_INVALID;
				}
				// 상대적 레벨 체크 추가 2006.06.27. steeple
				else if(!CheckLevelDiff(pcsSkill, pcsTargetCharacter))
				{
					eCheckTargetResult = AGPMSKILL_CHECK_TARGET_INVALID;
				}
			}

			// 2005.10.04. steeple
			// 여기서는 공식은 체크하지 않고, 가능한지 여부만 체크하도록 수정.
			//
			// 2005.09.01. steeple
			// Taming 이면 여기서 쳌흐
			if(m_pagpmSkill->IsTameSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
			{
				if(pcsTargetCharacter)
				{
					eAgpmSummonsTamableCheckResult eCheckResult = m_pagpmSummons->IsTameEnable((AgpdCharacter*)pcsSkill->m_pcsBase, pcsTargetCharacter, FALSE);

					if(eCheckResult == AGPMSUMMONS_TAMABLE_CHECK_RESULT_SUCCESS)
					{
						INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
						if(!lSkillLevel)
							return FALSE;

						INT32 lMaxTameCountSkill = (INT32)((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_LIMIT_QUANTITY][lSkillLevel];
						if(m_pagpmSummons->GetTameCount(static_cast<AgpdCharacter*>(pcsSkill->m_pcsBase)) >= lMaxTameCountSkill)
							eCheckResult = AGPMSUMMONS_TAMABLE_CHECK_RESULT_FAILURE_MAX_COUNT;
					}

					if(eCheckResult != AGPMSUMMONS_TAMABLE_CHECK_RESULT_SUCCESS)
					{
						if(m_pagsmSummons)
							m_pagsmSummons->SendTameCheckResult(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase), eCheckResult);
						eCheckTargetResult = AGPMSKILL_CHECK_TARGET_INVALID;
					}
				}
				else
				{
					if(m_pagsmSummons)
						m_pagsmSummons->SendTameCheckResult(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase), AGPMSUMMONS_TAMABLE_CHECK_RESULT_FAILURE_INVALID_TARGET);
					eCheckTargetResult = AGPMSKILL_CHECK_TARGET_INVALID;
				}
			}

			// 2005.10.24. steeple
			// Mutation 도 공식체크한다.
			if(m_pagpmSkill->IsMutationSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
			{
				if(pcsTargetCharacter)
				{
					if(m_pagpmCharacter->IsMonster(pcsTargetCharacter))
					{
						if(pcsTargetCharacter->m_pcsCharacterTemplate->m_eTamableType != AGPDCHAR_TAMABLE_TYPE_BY_FORMULA)
						{
							// Mutation 할 수 없는 놈이다.
							if(m_pagsmSystemMessage)
								m_pagsmSystemMessage->SendSystemMessage(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase), AGPMSYSTEMMESSAGE_CODE_MUTATION_FAILURE_INVALID_TARGET);
							eCheckTargetResult = AGPMSKILL_CHECK_TARGET_INVALID;
						}
					}
				}
				else
					eCheckTargetResult = AGPMSKILL_CHECK_TARGET_INVALID;
			}

			switch (eCheckTargetResult) {
			case AGPMSKILL_CHECK_TARGET_INVALID:
				if(m_pagsmSystemMessage)
					m_pagsmSystemMessage->SendSystemMessage(m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase), AGPMSYSTEMMESSAGE_CODE_CANNOT_ATTACKABLE_TARGET);

				if (pcsTargetCharacter)
					pcsTargetCharacter->m_Mutex.Release();
				return FALSE;
				break;

			case AGPMSKILL_CHECK_TARGET_NOT_IN_RANGE:
				{
					// 2005.10.13. steeple
					// 타겟이 멀면 그냥 사용하지 않아야 하는 스킬이라면 리턴 FALSE 해버린다.
					if (pcsSkillTemplate->m_lConditionType[0] & AGPMSKILL_CONDITION_SKIP_TARGET_NOT_IN_RANGE)
					{
						if (pcsTargetCharacter)
							pcsTargetCharacter->m_Mutex.Release();
						return FALSE;
						break;
					}

					// pstCalcPos로 이동한다.

					if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
					{
						AgpdCharacter *pcsCharacter	= (AgpdCharacter *)	pcsSkill->m_pcsBase;

						if (FALSE == (AGPDCHAR_SPECIAL_STATUS_DONT_MOVE & pcsCharacter->m_ulSpecialStatus))
						{
							pcsCharacter->m_stNextAction.m_bForceAction = FALSE;
							pcsCharacter->m_stNextAction.m_eActionType = AGPDCHAR_ACTION_TYPE_SKILL;
							pcsCharacter->m_stNextAction.m_csTargetBase.m_eType = pcsSkill->m_csTargetBase.m_eType;
							pcsCharacter->m_stNextAction.m_csTargetBase.m_lID = pcsSkill->m_csTargetBase.m_lID;
							pcsCharacter->m_stNextAction.m_lUserData[0] = pcsSkill->m_lID;
							pcsCharacter->m_stNextAction.m_lUserData[1] = (INT_PTR) pcsSkill->m_pcsBase;
							pcsCharacter->m_stNextAction.m_lUserData[2]	= pcsSkill->m_pcsTemplate->m_lID;
							pcsCharacter->m_stNextAction.m_lUserData[3]	= m_pagpmSkill->GetSkillLevel(pcsSkill);
							pcsCharacter->m_stNextAction.m_lUserData[4]	= (INT32) bMonsterUse;

							// 2006.10.20. steeple
							if(m_pagpmSkill->IsGroundTargetSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
							{
								// 2008.08.06. steeple
								// A pos var would be changed in a following function. So you have to call with a dummy var.
								AuPOS stTargetPos = pcsSkill->m_posTarget;
								m_pagpmCharacter->MoveCharacterToPositionWithDistance(pcsCharacter, stTargetPos, m_pagpmSkill->GetRange(pcsSkill) + pcsSkill->m_lTargetHitRange - 75, TRUE);
							}
							else
								m_pagpmCharacter->MoveCharacterFollow(pcsCharacter, pcsTargetCharacter, m_pagpmSkill->GetRange(pcsSkill) + pcsSkill->m_lTargetHitRange - 50, TRUE);
						}

						if (pcsTargetCharacter)
							pcsTargetCharacter->m_Mutex.Release();

						return TRUE;
					}
					break;
				}

			case AGPMSKILL_CHECK_TARGET_OK:
				{
					if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
					{
						AgpdCharacter *pcsCharacter	= (AgpdCharacter *)	pcsSkill->m_pcsBase;

						// 먼저 이전에 등록된 Action이 있는지 본다. 있다면 취소시킨다.
						pcsCharacter->m_stNextAction.m_eActionType	= AGPDCHAR_ACTION_TYPE_NONE;

						if (pcsTargetCharacter)
						{
							// 해당 방향으로 틀고
							m_pagpmCharacter->TurnCharacter(pcsCharacter, 0, m_pagpmCharacter->GetSelfCharacterTurnAngle(pcsCharacter, &(pcsTargetCharacter->m_stPos)));
						}

						if (pcsCharacter->m_bMove)
							m_pagpmCharacter->StopCharacter(pcsCharacter, NULL);
					}

					break;
				}
			}

			if (pcsTargetCharacter)
			{

				if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT &&
					!PreCheckProductSkill(pcsSkill, pcsTargetCharacter))
					{
					pcsTargetCharacter->m_Mutex.Release();
					return FALSE;
					}

				pcsSkill->m_posTarget = pcsTargetCharacter->m_stPos;
				pcsTargetCharacter->m_Mutex.Release();
			}
		
		}

		/*
		// 유효한 타겟인지 검사
		if (pcsSkill->m_csTargetBase.m_eType != APBASE_TYPE_NONE &&
			pcsSkill->m_csTargetBase.m_lID != 0)
		{
			if (!m_pagpmSkill->IsValidTarget(pcsSkill, &pcsSkill->m_csTargetBase, &pcsSkill->m_posTarget, pcsSkill->m_lTargetUnion, pcsSkill->m_bForceAttack))
				return FALSE;
		}
		else
		{
			if (!m_pagpmSkill->IsValidTarget(pcsSkill, &pcsSkill->m_posTarget, pcsSkill->m_bForceAttack))
				return FALSE;
		}
		*/
		// 각종 사항에 대한 체킹을 한다. (여기까지)
		//////////////////////////////////////////////////////////////////////////////////////////////////
	}

	if (m_pagpmSkill->IsPassiveSkill(pcsSkill))
	{
		// use passive skill list 에 추가할 수 있는지 검사한다.
		// 꽉차있는경우 추가할 수 없다. false 리턴

		if (!m_pagpmSkill->AddUsePassiveList(pcsSkill, pcsSkill->m_lID))
			return FALSE;
	}
	else
	{
		// use skill list 에 추가할 수 있는지 검사한다.
		// 꽉차있는경우 추가할 수 없다. false 리턴

		if (!m_pagpmSkill->AddUseList(pcsSkill, pcsSkill->m_lID))
		{
			SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
			return FALSE;
		}

		// 각종 검사를 통과했다. 이넘은 이제 스킬이 사용된당....
		//////////////////////////////////////////////////////////////////////////////////////////////////

		/*
		// 스킬이 사용된다고 알려준다.
		if (!SendCastSkill(pcsSkill, AGPMSKILL_ACTION_START_CAST_SKILL, NULL, 0))
		{
			m_pagpmSkill->RemoveUseList(pcsSkill, pcsSkill->m_lID);
			return FALSE;
		}

		// action status를 skill로 업데이트
		if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
			m_pagpmCharacter->UpdateActionStatus((AgpdCharacter *) pcsSkill->m_pcsBase, AGPDCHAR_STATUS_READY_SKILL);
		*/
	}

	pcsAgsdSkill->m_bIsCastSkill	= TRUE;

	// 사용 factor들을 초기화한다.
	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorPoint);
	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorPercent);
	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorResult);

	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPoint);
	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPercent);

	pcsAgsdSkill->m_lModifySkillLevel						= 0;
	pcsAgsdSkill->m_lModifyCharLevel						= 0;

	pcsAgsdSkill->m_nMeleeDefensePoint						= 0;
	pcsAgsdSkill->m_nMeleeReflectPoint						= 0;
	pcsAgsdSkill->m_nMagicDefensePoint						= 0;
	pcsAgsdSkill->m_nMagicReflectPoint						= 0;

	ZeroMemory(&pcsAgsdSkill->m_stBuffedSkillCombatEffectArg, sizeof(AgpdSkillBuffedCombatEffectArg));
	ZeroMemory(&pcsAgsdSkill->m_stBuffedSkillFactorEffectArg, sizeof(AgpdSkillBuffedFactorEffectArg));

	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
		EnumCallback(AGSMSKILL_CB_CAST_SKILL, pcsSkill->m_pcsBase, pcsSkill);

	/*
	for (int j = 0; j < AGPMSKILL_MAX_AFFECTED_BASE; ++j)
	{
		m_pagpmFactors->InitFactor(pcsAgsdSkill->m_csModifyFactorPoint + j);
		m_pagpmFactors->InitFactor(pcsAgsdSkill->m_csModifyFactorPercent + j);
	}
	*/

	if (m_pagpmSkill->IsPassiveSkill(pcsSkill))
	{
		pcsSkill->m_ulEndTime			= 0xFFFFFFFF;
		pcsSkill->m_ulInterval			= 0;
		pcsSkill->m_ulNextProcessTime	= 0;

		m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_CAST);

		ApdEvent *pcsEvent = m_papmEventManager->AddEvent(m_papmEventManager->GetEventData(pcsSkill->m_pcsBase), APDEVENT_FUNCTION_SKILL, pcsSkill->m_pcsBase, FALSE, 0, GetClockCount());
		if (!pcsEvent)
		{
			// event에 넣는게 실패했다. 초기화 시킨다.
			m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_NOT_CAST);
			m_pagpmSkill->RemoveUsePassiveList(pcsSkill, pcsSkill->m_lID);

			SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
			return FALSE;
		}

		BOOL bResult = ApplySkill(pcsSkill, pcsEvent);

		m_papmEventManager->RemoveEvent(pcsEvent);

		// Log - 2004.05.06. steeple
		//WriteSkillUseLog(pcsSkill);

		return bResult;
	}
	else
	{
		//if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
		//	m_pagsmCharacter->AddCastSkillCount((AgpdCharacter *) pcsSkill->m_pcsBase);

		// dirt 세팅
		SetModifiedSkillLevel(pcsSkill);

		INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			return FALSE;

		// 현재 이 스킬에 대한 시전 지연시간을 구한다. (pcsSkill->m_csFactor에 있는 값들을 이용해 구한다)
		// 위에서 구한 시간은 m_ulEndTime에 넣는다.
		pcsSkill->m_ulStartTime				= ulCurrentClock;
		UINT32	ulCastDelay					= m_pagpmSkill->GetCastDelay(pcsSkill);
		//UINT32	ulCastDelay					= m_pagpmSkill->GetCastDelay((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate, lSkillLevel);
		

		// 2005.12.4. steeple
		INT32 ulAdjustCastDelay = (INT32)((FLOAT)(ulCastDelay) * (FLOAT)m_pagpmSkill->GetAdjustCastDelay((AgpdCharacter*)pcsSkill->m_pcsBase) / 100.0f);
		ulCastDelay += ulAdjustCastDelay;

		if (pcsSkillTemplate->m_bShrineSkill)
		{
				pcsSkill->m_ulEndTime			= 0xFFFFFFFF;
				pcsSkill->m_ulInterval			= 0;
				pcsSkill->m_ulNextProcessTime	= 0;
		}
		else
		{
			// 2005.09.12. steeple
			// 아 이거 일일이 다 찾아서 고쳐야 하는 거야 ㅠㅠ
			// 그러기 좀 뭐 해서 Buff 관련 스킬에만 이렇게 해줌.
			if (m_pagpmSkill->IsDurationByDistanceSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) ||
				m_pagpmSkill->IsDurationUnlimited((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
				pcsSkill->m_ulEndTime			= 0xFFFFFFFF;
			else if (m_pagpmSkill->GetSkillDurationTime(pcsSkill, lSkillLevel) > 0)
			{
				// skill save - arycoat 2008.7
				UINT32 ulSkillDuration = m_pagpmSkill->GetSkillDurationTime(pcsSkill, lSkillLevel);
				if(pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_SAVE_EXPIREDATE)
				{
					if(pcsSkill->m_pSaveSkillData.eStep == SAVESKILL_NONE)
					{
						CTime pNow = CTime::GetCurrentTime();
						CTimeSpan pRemainTime( 0, 0, 0, ulSkillDuration/1000 );
						CTime pExpireDate = pNow + pRemainTime;

						pcsSkill->m_pSaveSkillData.m_pSaveData.SkillTID = pcsSkill->m_pcsTemplate->m_lID;
						pcsSkill->m_pSaveSkillData.m_pSaveData.ExpireDate = pExpireDate;
						pcsSkill->m_pSaveSkillData.eStep = SAVESKILL_INSERT;
					}
					else if(pcsSkill->m_pSaveSkillData.eStep == SAVESKILL_RECASTING)
					{
						CTime pNow = CTime::GetCurrentTime();
						CTime pExpireDate = pcsSkill->m_pSaveSkillData.m_pSaveData.ExpireDate;
						CTimeSpan pRemainTime = pExpireDate - pNow;
						
						if(pRemainTime < 0)
						{
							ProcessSkillSave(pcsSkill, (AgpdCharacter *) pcsSkill->m_pcsBase, AGSMDATABASE_OPERATION_DELETE);

							SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);

							return FALSE;
						}
						
						ulSkillDuration = (UINT32)pRemainTime.GetTotalSeconds() * 1000;
					}
				}

				pcsSkill->m_ulEndTime			= ulCurrentClock + ulCastDelay + ulSkillDuration;
			}
			else
				pcsSkill->m_ulEndTime			= ulCurrentClock;

			pcsSkill->m_ulInterval				= m_pagpmSkill->GetSkillInterval(pcsSkill, lSkillLevel);
			if (pcsSkill->m_ulInterval)
				pcsSkill->m_ulNextProcessTime	= ulCurrentClock + ulCastDelay + pcsSkill->m_ulInterval;
			else
				pcsSkill->m_ulNextProcessTime	= pcsSkill->m_ulEndTime;
		}

		m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_CAST);

		if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
		{
			//m_pagpmCharacter->UpdateActionStatus((AgpdCharacter *) pcsSkill->m_pcsBase, AGPDCHAR_STATUS_NOT_ACTION);

			if ( (!(pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT) &&
					m_pagpmSkill->IsCashSkill(pcsSkillTemplate) == FALSE) // 캐쉬 아이템도 빠진다.
					&& pcsSkill->m_pSaveSkillData.eStep != SAVESKILL_RECASTING)
				m_pagpmCharacter->SetActionBlockTime((AgpdCharacter *) pcsSkill->m_pcsBase, ulCastDelay, AGPDCHAR_ACTION_BLOCK_TYPE_SKILL);
		}

		BOOL	bAddEvent = FALSE;

		/*
		// m_ulEndTime이 0이라면 (즉, 시전 하는데 지연시간이 없다면) skill status를 AGPMSKILL_STATUS_CAST로
		// m_ulEndTime이 0이 아니라면 skill status를 AGPMSKILL_STATUS_CAST_DELAY로 세팅한다.
		if (pcsSkill->m_ulStartTime != pcsSkill->m_ulEndTime)
		{
			m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_CAST_DELAY);
		}
		else
		{
			if (pcsSkillTemplate->m_bShrineSkill)
			{
				pcsSkill->m_ulEndTime			= 0xFFFFFFFF;
				pcsSkill->m_ulInterval			= 0;
				pcsSkill->m_ulNextProcessTime	= 0;
			}
			else
			{
				pcsSkill->m_ulEndTime			= ulCurrentClock + m_pagpmSkill->GetSkillDurationTime(pcsSkill);
				pcsSkill->m_ulInterval				= m_pagpmSkill->GetSkillInterval(pcsSkill);
				if (pcsSkill->m_ulInterval)
					pcsSkill->m_ulNextProcessTime	= pcsSkill->m_ulStartTime + pcsSkill->m_ulInterval;
				else
					pcsSkill->m_ulNextProcessTime	= pcsSkill->m_ulEndTime;
			}

			m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_CAST);

			if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
				m_pagpmCharacter->UpdateActionStatus((AgpdCharacter *) pcsSkill->m_pcsBase, AGPDCHAR_STATUS_NOT_ACTION);

			// cast delay도 없고 skill duration time도 없는경우 (걍 바로한번 써지고 없어지는경우..)
			// event에 넣지 않고 걍 ApplySkill() => EndSkill() 하고 끝낸다.
			if (pcsSkill->m_ulNextProcessTime == pcsSkill->m_ulEndTime)
				bAddEvent = FALSE;
		}
		*/

		ApdEvent *pcsEvent = NULL;

		// 이벤트 메니저에 등록되어 있지 않다면 스킬 소유주를 ApmEventManager에 등록한다.
		if (pcsSkillAttachData->m_alUseSkillID[1] == AP_INVALID_SKILLID
			|| pcsSkill->m_pSaveSkillData.eStep != SAVESKILL_NONE )
		{
			if (m_pagpmSkill->IsEvolutionSkill(pcsSkillTemplate))
			{
				pcsEvent = m_papmEventManager->AddEvent(m_papmEventManager->GetEventData(pcsSkill->m_pcsBase), APDEVENT_FUNCTION_EVOLUTION, pcsSkill->m_pcsBase, bAddEvent, 0, GetClockCount());
			}
			else
			{
				pcsEvent = m_papmEventManager->AddEvent(m_papmEventManager->GetEventData(pcsSkill->m_pcsBase), APDEVENT_FUNCTION_SKILL, pcsSkill->m_pcsBase, bAddEvent, 0, GetClockCount());
			}
			if (!pcsEvent)
			{
				// event에 넣는게 실패했다. 초기화 시킨다.
				pcsSkillAttachData->m_alUseSkillID[0] = AP_INVALID_SKILLID;
				m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_NOT_CAST);

				if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
					m_pagpmCharacter->UpdateActionStatus((AgpdCharacter *) pcsSkill->m_pcsBase, AGPDCHAR_STATUS_NOT_ACTION);

				SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
				return FALSE;
			}
		}

		// cast delay도 없고 skill duration time도 없는경우 (걍 바로한번 써지고 없어지는경우..)
		// event에 넣지 않고 걍 ApplySkill() => EndSkill() 하고 끝낸다.
		//if (!bAddEvent && pcsEvent)
		{
			if (!ApplySkill(pcsSkill, pcsEvent))
			{
				//m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_NOT_CAST);
				EndSkill(pcsSkill);

				m_papmEventManager->RemoveEvent(pcsEvent);
				return FALSE;
			}

			//m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_NOT_CAST);

			if (!EndSkill(pcsSkill))
			{
				m_papmEventManager->RemoveEvent(pcsEvent);
				return FALSE;
			}

			m_papmEventManager->RemoveEvent(pcsEvent);

			// Log - 2004.05.06. steeple
			//WriteSkillUseLog(pcsSkill);

			return TRUE;
		}
	}

	return TRUE;
}

BOOL AgsmSkill::CancelSkill(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pcsSkill->m_pcsBase;
	if(!pcsCharacter)
		return FALSE;

	AgpdSkillTemplate *pcsSkillTemplate = (AgpdSkillTemplate *)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	// 버프스킬만 취소 할 수 있음 - arycoat 2008.10.13
	AgpdSkill* pcsFindSkill = FindBuffedSkill(pcsCharacter, pcsSkillTemplate->m_lID);
	if(!pcsFindSkill)
		return FALSE;

	if(m_pagsmSkillEffect->ProcessSkillEffectCancel(pcsSkill, pcsCharacter))
	{
		EndBuffedSkillByTID(pcsCharacter, pcsSkillTemplate->m_lID);
	}

	return TRUE;
}

BOOL AgsmSkill::CastPassiveSkill(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate)
		return FALSE;

	// cast 하는 이 스킬보다 한단계 낮은 같은 계열의 스킬이 있는지 검사해서
	// 있다면 그 스킬을 먼저 EndSkill 처리한다.
	//		(그래야 같은 능력을 갖는 스킬이 중복해서 사용되지 않는다.)
	/////////////////////////////////////////////////////////////////////////////////
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szBelowLevelSkillName &&
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szBelowLevelSkillName[0])
	{
		AgpdSkill *pcsBelowSkill = m_pagpmSkill->GetSkill(pcsSkill->m_pcsBase, ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szBelowLevelSkillName);
		if (pcsBelowSkill)
		{
			// 사용중인지 검사
			if (pcsBelowSkill->m_eStatus != AGPMSKILL_STATUS_NOT_CAST)	// 사용중인 스킬이다.
				EndPassiveSkill(pcsBelowSkill);
		}
	}

	AgpdCharacter *pcsCharacter = (AgpdCharacter*)pcsSkill->m_pcsBase;

	// cast 하는 스킬이 같은 클래스인지 확인한 후 캐스트 할수있게 한다.
	INT32	lBaseRace	= m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor);
	INT32	lBaseClass	= m_pagpmFactors->GetClass(&pcsCharacter->m_csFactor);

	if(!m_pagpmSkill->IsWantedRaceClassSkill(pcsSkill, lBaseRace, lBaseClass))
		return FALSE;

	return CastSkill(pcsSkill);
}

// 타겟 제한없는 종족스킬 관련 2008.06.02 iluvs
BOOL AgsmSkill::CastSkillUnlimitedTarget(AgsdCastSkill pcsCastSkill)
{
	ApBase* pcsBase			= pcsCastSkill.pcsBase;
	INT32 lSkillTID			= pcsCastSkill.lSkillTID;
	INT32 lSkillLevel		= pcsCastSkill.lSkillLevel;
	ApBase* pcsTargetBase	= pcsCastSkill.pcsTargetBase;
	BOOL bForceAttack		= pcsCastSkill.bForceAttack;
	BOOL bMonsterUse		= pcsCastSkill.bMonsterUse;
	INT32 lSkillScrollIID	= pcsCastSkill.lSkillScrollIID;

	//////////////////////////////////////////////////////////////////////////

	if (!pcsBase || !pcsTargetBase || lSkillTID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillTemplate	*pcsSkillTemplate	= m_pagpmSkill->GetSkillTemplate(lSkillTID);
	if (!pcsSkillTemplate)
		return FALSE;

	INT32 lAdditionalSkillTID = (INT32)pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel, m_csRandom.randInt(100));

	// 자기를 일단 타겟으로 가져온다.
	AgpdCharacter* pcsObtainSkill = m_pagpmCharacter->GetCharacter(pcsTargetBase->m_lID);
	AgpdCharacter* pcsTarget	  = m_pagpmCharacter->GetCharacter(pcsTargetBase->m_lID);
	if(!pcsObtainSkill || !pcsTarget)
		return FALSE;

	ApmMap::RegionTemplate* pstBase = m_papmMap->GetTemplate(pcsObtainSkill->m_nBindingRegionIndex);

	std::list<INT32> csTargetList;

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_RACE)
	{
		INT32 lIndex = 0;
		while(pcsTarget = m_pagpmCharacter->GetCharSequence(&lIndex))
		{
			if(m_pagpmCharacter->IsPC(pcsTarget) && m_pagsmCharacter->GetCharDPNID(pcsTarget) != 0 && m_pagpmCharacter->IsSameRace(pcsObtainSkill, pcsTarget))
			{
				ApmMap::RegionTemplate* pstTarget = m_papmMap->GetTemplate(pcsTarget->m_nBindingRegionIndex);

				if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_REGION)
				{
					if(!m_papmMap->IsSameRegion(pstBase, pstTarget)) continue;
				}

				if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_REGION_INVOLVE_PARENT)
				{
					if(!m_papmMap->IsSameRegionInvolveParent(pstBase, pstTarget)) continue;
				}

				if(lAdditionalSkillTID)
				{
					AgsdCastSkill pCastSkill = pcsCastSkill;
					pCastSkill.pcsBase			= static_cast<ApBase*>(pcsTarget);
					pCastSkill.lSkillTID		= lAdditionalSkillTID;
					pCastSkill.lSkillLevel		= lSkillLevel;
					pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsTarget);

					CastSkill(pCastSkill);
				}
				else
				{
					AgsdCastSkill pCastSkill = pcsCastSkill;
					pCastSkill.pcsBase			= pcsBase;
					pCastSkill.lSkillTID		= lSkillTID;
					pCastSkill.lSkillLevel		= lSkillLevel;
					pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsTarget);

					CastSkill(pCastSkill);
				}
			}
		}
	}

	return TRUE;
}

// 파티스킬을 시전하기. 2008.06.02 iluvs
BOOL AgsmSkill::CastPartySkill(AgsdCastSkill pcsCastSkill)
{
	ApBase* pcsBase			= pcsCastSkill.pcsBase;
	INT32 lSkillTID			= pcsCastSkill.lSkillTID;
	INT32 lSkillLevel		= pcsCastSkill.lSkillLevel;
	ApBase* pcsTargetBase	= pcsCastSkill.pcsTargetBase;
	BOOL bForceAttack		= pcsCastSkill.bForceAttack;
	BOOL bMonsterUse		= pcsCastSkill.bMonsterUse;
	INT32 lSkillScrollIID	= pcsCastSkill.lSkillScrollIID;

	//////////////////////////////////////////////////////////////////////////

	if (!pcsBase || !pcsTargetBase || lSkillTID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillTemplate	*pcsSkillTemplate	= m_pagpmSkill->GetSkillTemplate(lSkillTID);
	if (!pcsSkillTemplate)
		return FALSE;

	// 자기를 일단 타겟으로 가져온다.
	AgpdCharacter* pcsObtainSkill = m_pagpmCharacter->GetCharacter(pcsTargetBase->m_lID);
	AgpdCharacter* pcsTarget	  = m_pagpmCharacter->GetCharacter(pcsTargetBase->m_lID);
	if(!pcsObtainSkill || !pcsTarget)
		return FALSE;

	INT32 lAdditionalSkillTID = (INT32)pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel, m_csRandom.randInt(100));

	ApmMap::RegionTemplate* pstBase = m_papmMap->GetTemplate(pcsObtainSkill->m_nBindingRegionIndex);
	if (!pstBase)
		return FALSE;

	AgpdParty* pcsParty = m_pagpmParty->GetParty(pcsObtainSkill);
	
	if(!pcsParty)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_PARTY)
	{
		for(int lIndex=0; lIndex < pcsParty->m_nCurrentMember; lIndex++)
		{
			pcsTarget = m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[lIndex]);
			if(m_pagpmCharacter->IsPC(pcsTarget) && m_pagsmCharacter->GetCharDPNID(pcsTarget))
			{
				ApmMap::RegionTemplate* pstTarget = m_papmMap->GetTemplate(pcsTarget->m_nBindingRegionIndex);

				if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_REGION)
				{
					if(!m_papmMap->IsSameRegion(pstBase, pstTarget)) continue;
				}

				if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_REGION_INVOLVE_PARENT)
				{
					if(!m_papmMap->IsSameRegionInvolveParent(pstBase, pstTarget)) continue;
				}

				if(lAdditionalSkillTID)
				{
					AgsdCastSkill pCastSkill = pcsCastSkill;
					pCastSkill.pcsBase			= static_cast<ApBase*>(pcsTarget);
					pCastSkill.lSkillTID		= lAdditionalSkillTID;
					pCastSkill.lSkillLevel		= lSkillLevel;
					pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsTarget);

					CastSkill(pCastSkill);
				}
				else
				{
					AgsdCastSkill pCastSkill = pcsCastSkill;
					pCastSkill.pcsBase			= pcsBase;
					pCastSkill.lSkillTID		= lSkillTID;
					pCastSkill.lSkillLevel		= lSkillLevel;
					pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsTarget);

					CastSkill(pCastSkill);
				}
			}
		}
	}
	else
		return FALSE;

	return TRUE;
}

// 2009.03.26.iluvs
BOOL AgsmSkill::CastInvolveParentRegionSkill(AgsdCastSkill pcsCastSkill)
{
	ApBase* pcsBase			= pcsCastSkill.pcsBase;
	INT32	lSkillTID		= pcsCastSkill.lSkillTID;
	INT32	lSkillLevel		= pcsCastSkill.lSkillLevel;
	ApBase* pcsTargetBase	= pcsCastSkill.pcsTargetBase;
	BOOL	bForceAttack	= pcsCastSkill.bForceAttack;
	BOOL	bMonsterUse		= pcsCastSkill.bMonsterUse;
	INT32	lSkillScrollTID	= pcsCastSkill.lSkillScrollIID;

	///////////////////////////////////////////////////////////////////////////////

	if(!pcsBase || !pcsTargetBase || lSkillTID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillTemplate	*pcsSkillTemplate = m_pagpmSkill->GetSkillTemplate(lSkillTID);
	if(!pcsSkillTemplate)
		return FALSE;

	INT32 lAdditionalSkillTID = (INT32)pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel, m_csRandom.randInt(100));

	// 자기를 일단 타겟으로 가져온다.
	AgpdCharacter* pcsObtainSkill = m_pagpmCharacter->GetCharacter(pcsTargetBase->m_lID);
	AgpdCharacter* pcsTarget	  = m_pagpmCharacter->GetCharacter(pcsTargetBase->m_lID);
	if(!pcsObtainSkill || !pcsTarget)
		return FALSE;

	ApmMap::RegionTemplate* pstBase = m_papmMap->GetTemplate(pcsObtainSkill->m_nBindingRegionIndex);

	std::list<INT32> csTargetList;

	INT32 lIndex = 0;
	while(pcsTarget = m_pagpmCharacter->GetCharSequence(&lIndex))
	{
		if(m_pagpmCharacter->IsPC(pcsTarget) && m_pagsmCharacter->GetCharDPNID(pcsTarget))
		{
			ApmMap::RegionTemplate* pstTarget = m_papmMap->GetTemplate(pcsTarget->m_nBindingRegionIndex);

			if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_REGION_INVOLVE_PARENT)
			{
				if(!m_papmMap->IsSameRegionInvolveParent(pstBase, pstTarget)) continue;
			}

			if(lAdditionalSkillTID)
			{
				AgsdCastSkill pCastSkill = pcsCastSkill;
				pCastSkill.pcsBase			= static_cast<ApBase*>(pcsTarget);
				pCastSkill.lSkillTID		= lAdditionalSkillTID;
				pCastSkill.lSkillLevel		= lSkillLevel;
				pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsTarget);

				CastSkill(pCastSkill);
			}
			else
			{
				AgsdCastSkill pCastSkill = pcsCastSkill;
				pCastSkill.pcsBase			= pcsBase;
				pCastSkill.lSkillTID		= lSkillTID;
				pCastSkill.lSkillLevel		= lSkillLevel;
				pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsTarget);

				CastSkill(pCastSkill);
			}
		}
	}

	return TRUE;
}

// 2007.02.13. steeple
BOOL AgsmSkill::ReCastAllPassiveSkill(ApBase* pcsBase)
{
	if(!pcsBase)
		return FALSE;

	AgpdSkillAttachData	*pcsAttachData	= m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

	ApSafeArray<INT32, AGPMSKILL_MAX_SKILL_USE> alUsePassiveSkillID;
	alUsePassiveSkillID.MemCopy(0, pcsAttachData->m_alUsePassiveSkillID.begin(), AGPMSKILL_MAX_SKILL_USE);

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if(!alUsePassiveSkillID[i])
			break;

		AgpdSkill* pcsPassiveSkill = m_pagpmSkill->GetSkill(alUsePassiveSkillID[i]);
		if(!pcsPassiveSkill)
			continue;

		EndPassiveSkill(pcsPassiveSkill);
	}

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if(pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill *pcsSkill = pcsAttachData->m_apcsSkill[i];
		if(pcsSkill)
		{
			if(m_pagpmSkill->IsPassiveSkill(pcsSkill))
			{
				CastPassiveSkill(pcsSkill);
			}
		}
	}

	return TRUE;
}

// 2007.04.05. steeple
// 공성스킬 중에 Active(EnerGameWorld, Resurrection 등) 될 때 써줘야 하는 거.
BOOL AgsmSkill::CastSiegeWarSkillOnActive(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;
	
	return TRUE;
}

// 2008.04.07. steeple
// Condition2 에 해당하는 스킬을 조건에 맞게끔 Cast 해준다.
BOOL AgsmSkill::CastCondition2Skill(AgpdCharacter* pcsCharacter, INT32 lConditionType, BOOL bCash)
{
	if(!pcsCharacter)
		return FALSE;

	vector<INT32>::iterator iter = m_pagpmSkill->m_vcCondition2SkillTID.begin();
	while(iter != m_pagpmSkill->m_vcCondition2SkillTID.end())
	{
		AgpdSkillTemplate* pcsSkillTemplate = m_pagpmSkill->GetSkillTemplate(*iter);
		if(!pcsSkillTemplate)
		{
			++iter;
			continue;
		}
		
		BOOL bCast = TRUE;

		if(lConditionType & AGPMSKILL_CONDITION2_START_ON_RESURRECTION)
		{
			if(!(pcsSkillTemplate->m_ulCondition2 & AGPMSKILL_CONDITION2_START_ON_RESURRECTION))
				bCast = FALSE;
		}

		if(lConditionType & AGPMSKILL_CONDITION2_RELY_ON_REGION)
		{
			if(!(pcsSkillTemplate->m_ulCondition2 & AGPMSKILL_CONDITION2_RELY_ON_REGION))
				bCast = FALSE;
			else
			{
				// 이 상황에서는 현재(2008.04.07.) 로써는 카오틱 프론티어 뿐이다.
				// 나중에 배틀 그라운드에서 활용할 것 같다.

				// Region 이 카오틱 프론티어 일 때만 써준다. (현재. 2008.04.07)
				ApmMap::RegionTemplate* pcsPrevRegionTemplate = m_papmMap->GetTemplate(pcsCharacter->m_nBindingRegionIndex);
				if(!pcsPrevRegionTemplate || pcsPrevRegionTemplate->nLevelLimit == 0)
					bCast = FALSE;
			}
		}

		if(lConditionType & AGPMSKILL_CONDITION2_DONT_USE_BY_CASH)
		{
			// 얘는 체크가 되어 있을 때 안 써줘야 한다.
			if((pcsSkillTemplate->m_ulCondition2 & AGPMSKILL_CONDITION2_DONT_USE_BY_CASH) &&
				(bCash == TRUE))
				bCast = FALSE;
		}

		if(bCast)
		{
			CastSkill((ApBase*)pcsCharacter, pcsSkillTemplate->m_lID, 1, (AuPOS*)NULL, FALSE);
		}

		++iter;
	}

	return TRUE;
}

//		EndSkill
//	Functions
//		- pcsSkill이 끝났다.
//				1. 이 스킬에 영향 받던 넘들을 모두 되돌린다. 
//					예) pcsAgsdSkill->m_csUpdateFactor 값을 CalcFactor()를 이용해 뺀다.
//				2. 영향 받던 넘들에게 버프 없어진다고 알려준다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::EndSkill(AgpdSkill *pcsSkill)
{
	PROFILE("AgsmSkill::EndSkill");

	if (!pcsSkill || pcsSkill->m_eStatus == AGPMSKILL_STATUS_NOT_CAST)
	//if (!pcsSkill)
		return FALSE;

	if (!m_pagpmSkill->IsPassiveSkill(pcsSkill))
		SendCastSkill(pcsSkill, AGPMSKILL_ACTION_END_CAST_SKILL, NULL, 0);

	// recast delay를 다시 세팅한다.
	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	// 2005.07.25. steeple
	// 타겟이 없어서 스킬이 종료 된 경우 다시 바로 쓸 수 있게 해준다.
	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	UINT32 ulClockCount = GetClockCount();

	// 2005.12.04. steeple
	UINT32 ulRecastDelay = m_pagpmSkill->GetRecastDelay((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate, lSkillLevel);
	INT32 lAdjustDelay = (INT32)((FLOAT)(ulRecastDelay) * (FLOAT)m_pagpmSkill->GetAdjustRecastDelay((AgpdCharacter*)pcsSkill->m_pcsBase) / 100.0f);
	ulRecastDelay += lAdjustDelay;

	// 2006.07.19. steeple
	// Cast Delay 체크도 한다. 장퀘놈들 겁나 해킹해댄다.
	UINT32 ulCastDelay = m_pagpmSkill->GetCastDelay((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate, lSkillLevel);
	INT32 ulAdjustCastDelay = (INT32)((FLOAT)(ulCastDelay) * (FLOAT)m_pagpmSkill->GetAdjustCastDelay((AgpdCharacter*)pcsSkill->m_pcsBase) / 100.0f);
	ulCastDelay += ulAdjustCastDelay;

	// 2005.12.24. steeple 추가
	// 지속시간이 무한대인 놈은 RecastDelay 를 현재 클락으로 해준다.
	if(pcsAgsdSkill->m_eMissedReason == AGSDSKILL_MISSED_REASON_TARGET_NUM_ZERO ||
		m_pagpmSkill->IsDurationByDistanceSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) ||
		m_pagpmSkill->IsDurationUnlimited((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
	{
		pcsSkill->m_ulCastDelay = ulClockCount;
		pcsSkill->m_ulRecastDelay = ulClockCount;
	}
	else
	{
		pcsSkill->m_ulCastDelay = ulClockCount + (UINT32)((double)ulCastDelay / 2);		// CastDelay 는 rough 하게 체크한다.
		pcsSkill->m_ulRecastDelay = ulClockCount + ulRecastDelay;
	}

	// 스킬을 사용 안함 상태로 되돌린다. (초기화 시킨다)
	pcsSkill->m_ulStartTime			= 0;
	pcsSkill->m_ulEndTime			= 0;
	pcsSkill->m_ulNextProcessTime	= 0;
	pcsSkill->m_ulInterval			= 0;
	pcsSkill->m_pSaveSkillData.Clear();

	m_pagpmSkill->SetSkillStatus(pcsSkill, AGPMSKILL_STATUS_NOT_CAST);

	if (m_pagpmSkill->IsPassiveSkill(pcsSkill))
		m_pagpmSkill->RemoveUsePassiveList(pcsSkill, pcsSkill->m_lID);
	else
		m_pagpmSkill->RemoveUseList(pcsSkill, pcsSkill->m_lID);

	// 2005.09.16. steeple
	pcsAgsdSkill->m_eMissedReason = AGSDSKILL_MISSED_REASON_NORMAL;

	pcsAgsdSkill->m_bApplyDamageOnly = FALSE;

	pcsAgsdSkill->m_nAffectedDOTCount = 0;
	pcsAgsdSkill->m_nTotalDOTCount = 0;

	// 2007.10.23. steeple
	pcsAgsdSkill->m_stInvincibleInfo.clear();		// 초기화 안해도 되긴 하지만 해주자.
	pcsAgsdSkill->m_stConnectionInfo.init();

	// 사용된 factor들을 초기화한다.
	RestoreModifiedSkillLevel(pcsSkill);

	DestroyUpdateFactorPoint(pcsAgsdSkill);
	DestroyUpdateFactorPercent(pcsAgsdSkill);
	DestroyUpdateFactorResult(pcsAgsdSkill);
	DestroyUpdateFactorItemPoint(pcsAgsdSkill);
	DestroyUpdateFactorItemPercent(pcsAgsdSkill);

	return TRUE;
}

BOOL AgsmSkill::EndPassiveSkill(AgpdSkill *pcsSkill, BOOL bBelowSkillActive)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate)
		return FALSE;

	RestoreOriginalStatus(pcsSkill, pcsSkill->m_pcsBase);

	RestoreSkillFactor(pcsSkill, pcsSkill->m_pcsBase);

	if (!EndSkill(pcsSkill))
		return FALSE;

	if (bBelowSkillActive)
	{
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szBelowLevelSkillName &&
			((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szBelowLevelSkillName[0])
		{
			AgpdSkill	*pcsBelowSkill	= m_pagpmSkill->GetSkill(pcsSkill->m_pcsBase, ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szBelowLevelSkillName);
			if (pcsBelowSkill)
			{
				CastPassiveSkill(pcsBelowSkill);
			}
		}
	}

	return TRUE;
}

BOOL AgsmSkill::EndAllPassiveSkill(ApBase *pcsBase)
{
	if (!pcsBase)
		return FALSE;

	AgpdSkillAttachData	*pcsAttachData	= m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		AgpdSkill	*pcsPassiveSkill	= m_pagpmSkill->GetSkill(pcsAttachData->m_alUsePassiveSkillID[0]);
		if (!pcsPassiveSkill)
			continue;

		EndPassiveSkill(pcsPassiveSkill, FALSE);
	}

	return TRUE;
}

BOOL AgsmSkill::EndBuffedSkill(ApBase *pcsBase, INT32 lBuffedIndex, BOOL bCreateSkillTemplate, const TCHAR* szReason)
{
	/* Combine Korea Code
	PROFILE("AgsmSkill::EndBuffedSkill");

	if (!pcsBase || lBuffedIndex < 0 || lBuffedIndex >= AGPMSKILL_MAX_SKILL_BUFF)
		return FALSE;

	AgsdSkillADBase	*pcsADBase	= GetADBase(pcsBase);
	if (!pcsADBase)
		return FALSE;

	AgpdSkillAttachData	*pcsAttachData	= m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

	AgpdSkill* pcsSkill = pcsADBase->m_csBuffedSkillProcessInfo[lBuffedIndex].m_pcsTempSkill;
	if(pcsSkill)
	{
		AuAutoLock Lock(pcsSkill->m_Mutex);
		if(!Lock.Result())
			return FALSE;

		FreeSkillEffect(pcsSkill, pcsBase, lBuffedIndex);
		m_pagpmSkill->RemoveBuffedListByIndex(&pcsAttachData->m_astBuffSkillList[0], lBuffedIndex);

		if (pcsSkill)
		{
			if(m_pagpmSkill->IsCharacterConfigSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
			{
				EndUpdateCharacterConfig((AgpdCharacter*)pcsBase, pcsSkill);
			}

			m_pagpmFactors->DestroyFactor(&pcsSkill->m_csFactor);

			AgsdSkill	*pcsAgsdSkill	= GetADSkill(pcsSkill);

			DestroyUpdateFactorPoint(pcsAgsdSkill);
			DestroyUpdateFactorPercent(pcsAgsdSkill);
			DestroyUpdateFactorResult(pcsAgsdSkill);
			DestroyUpdateFactorItemPoint(pcsAgsdSkill);
			DestroyUpdateFactorItemPercent(pcsAgsdSkill);

			m_pagpmSkill->AddRemoveSkill(pcsSkill);

			pcsSkill = NULL;
		}
	}

	if (AGPMSKILL_MAX_SKILL_BUFF - lBuffedIndex - 1 > 0)
		pcsADBase->m_csBuffedSkillProcessInfo.MemCopy(lBuffedIndex, &pcsADBase->m_csBuffedSkillProcessInfo[lBuffedIndex + 1], AGPMSKILL_MAX_SKILL_BUFF - lBuffedIndex - 1);
	pcsADBase->m_csBuffedSkillProcessInfo.MemSet(AGPMSKILL_MAX_SKILL_BUFF - 1, 1);

	if (bCreateSkillTemplate)
		pcsADBase->m_csBuffedSkillProcessInfo[AGPMSKILL_MAX_SKILL_BUFF - 1].m_pcsTempSkill = m_pagpmSkill->CreateSkill();*/ //End Combine

	PROFILE("AgsmSkill::EndBuffedSkill");

	if (!pcsBase || lBuffedIndex < 0 || lBuffedIndex >= AGPMSKILL_MAX_SKILL_BUFF)
		return FALSE;

	AgsdSkillADBase	*pcsADBase	= GetADBase(pcsBase);
	if (!pcsADBase)
		return FALSE;

	AgpdSkillAttachData	*pcsAttachData	= m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

	FreeSkillEffect(pcsADBase->m_csBuffedSkillProcessInfo[lBuffedIndex].m_pcsTempSkill, pcsBase, lBuffedIndex);

	m_pagpmSkill->RemoveBuffedListByIndex(&pcsAttachData->m_astBuffSkillList[0], lBuffedIndex);

	AgpdSkill* pcsSkill = pcsADBase->m_csBuffedSkillProcessInfo[lBuffedIndex].m_pcsTempSkill;
	if (pcsSkill)
	{
		m_pagpmFactors->DestroyFactor(&pcsSkill->m_csFactor);

		AgsdSkill	*pcsAgsdSkill	= GetADSkill(pcsSkill);

		DestroyUpdateFactorPoint(pcsAgsdSkill);
		DestroyUpdateFactorPercent(pcsAgsdSkill);
		DestroyUpdateFactorResult(pcsAgsdSkill);
		DestroyUpdateFactorItemPoint(pcsAgsdSkill);
		DestroyUpdateFactorItemPercent(pcsAgsdSkill);

		m_pagpmSkill->AddRemoveSkill(pcsSkill);

		pcsSkill = NULL;
	}

	if (AGPMSKILL_MAX_SKILL_BUFF - lBuffedIndex - 1 > 0)
		pcsADBase->m_csBuffedSkillProcessInfo.MemCopy(lBuffedIndex, &pcsADBase->m_csBuffedSkillProcessInfo[lBuffedIndex + 1], AGPMSKILL_MAX_SKILL_BUFF - lBuffedIndex - 1);
	pcsADBase->m_csBuffedSkillProcessInfo.MemSet(AGPMSKILL_MAX_SKILL_BUFF - 1, 1);

	/*if (bCreateSkillTemplate)
	pcsADBase->m_csBuffedSkillProcessInfo[AGPMSKILL_MAX_SKILL_BUFF - 1].m_pcsTempSkill = m_pagpmSkill->CreateSkill();*/

	return TRUE;
}

BOOL AgsmSkill::EndBuffedSkill(ApBase *pcsBase, AgpdSkill *pcsSkill)
{
	if (!pcsBase || !pcsSkill)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsBase);
	if(!pcsAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (pcsAttachData->m_astBuffSkillList[i].lSkillID == pcsSkill->m_lID)
			return EndBuffedSkill(pcsBase, i, TRUE, _T("EndBuffedSkill(ApBase*, AgpdSkill*)"));
	}

	return FALSE;
}

BOOL AgsmSkill::EndBuffedSkillByTID(ApBase *pcsBase, INT32 lSkillTID, BOOL bInitCoolTime)
{
	if (!pcsBase || !lSkillTID)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsBase);
	if(!pcsAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (pcsAttachData->m_astBuffSkillList[i].lSkillTID == lSkillTID)
		{
			EndBuffedSkill(pcsBase, i, TRUE, _T("EndBuffedSkill(ApBase*, lSkillTID)"));
			if(bInitCoolTime)
				ProcessInitCoolTime((AgpdCharacter*)pcsBase, lSkillTID);	// 쿨타임 초기화 해준다.

			return TRUE;
		}
	}

	return FALSE;
}

BOOL AgsmSkill::EndAllBuffedSkill(ApBase *pcsBase, BOOL bCreateTempSkill)
{
	if (!pcsBase)
		return FALSE;

	AgsdSkillADBase	*pcsADBase	= GetADBase(pcsBase);
	if (!pcsADBase)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		//if (!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
		//	break;

		//EndBuffedSkill(pcsBase, i);

		EndBuffedSkill(pcsBase, 0, bCreateTempSkill, _T("EndAllBuffedSkill"));
	}

	return TRUE;
}

BOOL AgsmSkill::EndAllBuffedSkillExceptTitleSkill(ApBase *pcsBase, BOOL bCreateTempSkill)
{
	if (!pcsBase)
		return FALSE;

	AgsdSkillADBase	*pcsADBase	= GetADBase(pcsBase);
	if (!pcsADBase)
		return FALSE;

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		AgpdSkill* pcsTempSkill = pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill;

		if(!pcsTempSkill || !pcsTempSkill->m_pcsTemplate)
			break;

		if(m_pagsmTitle->GetEffectiveSkillTid((AgpdCharacter*)pcsBase) == ((AgpdSkillTemplate*)pcsTempSkill->m_pcsTemplate)->m_lID)
		{
			++lIndex;
			continue;
		}

		// 펫관련 스킬은 버프만 꺼주면 안되면 소환수 MAX도 줄여줘야한다.
		if(((AgpdSkillTemplate*)pcsTempSkill->m_pcsTemplate)->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE8)
		{
			AgpdSummonsADChar* pcsSummonsADChar = m_pagpmSummons->GetADCharacter((AgpdCharacter*)pcsBase);
			if(!pcsSummonsADChar)
				return FALSE;

			m_pagpmSummons->SetMaxSummonsCount((AgpdCharacter*)pcsBase, pcsSummonsADChar->m_SummonsArray.m_cMaxNum - 1);
		}

		EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndAllBuffedSkillExceptDebuff"));
	}

	return TRUE;
}

// 2005.06.20. steeple
// Buff 된 스킬중에서 Debuff 를 제외하고 없애준다.
BOOL AgsmSkill::EndAllBuffedSkillExceptDebuff(ApBase* pcsBase, BOOL bCreateTempSkill)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		AgpdSkill* pcsTempSkill = pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill;

		if(!pcsTempSkill || !pcsTempSkill->m_pcsTemplate)
			break;

		if(m_pagpmSkill->IsDebuffSkill((AgpdSkillTemplate*)pcsTempSkill->m_pcsTemplate))
		{
			++lIndex;
			continue;
		}
		
		// 펫관련 스킬은 버프만 꺼주면 안되면 소환수 MAX도 줄여줘야한다.
		if(((AgpdSkillTemplate*)pcsTempSkill->m_pcsTemplate)->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE8)
		{
			AgpdSummonsADChar* pcsSummonsADChar = m_pagpmSummons->GetADCharacter((AgpdCharacter*)pcsBase);
			if(!pcsSummonsADChar)
				return FALSE;

			m_pagpmSummons->SetMaxSummonsCount((AgpdCharacter*)pcsBase, pcsSummonsADChar->m_SummonsArray.m_cMaxNum - 1);
		}

		EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndAllBuffedSkillExceptDebuff"));
	}

	return TRUE;
}

BOOL AgsmSkill::EndAllBuffedSkillExceptCashSkill(ApBase* pcsBase, BOOL bCreateTempSkill)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate)
			break;

		if(m_pagpmSkill->IsCashSkill((AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate))
		{
			++lIndex;
			continue;
		}

		EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndAllBuffedSkillExceptCashSkill"));
	}

	return TRUE;
}

// 2007.07.20. steeple
// 캐쉬, 소환 버프를 제외하고 없앤다.
// bInitCoolTime 이 TRUE 이면, CoolTime 도 초기화 해준다.
BOOL AgsmSkill::EndAllBuffedSkillExceptCash_SummonsSkill(ApBase* pcsBase, BOOL bCreateTempSkill, BOOL bInitCoolTime, INT32 lMaxCount)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	std::vector<INT32> vcTmp;
	vcTmp.clear();

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate)
			break;

		AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate;
		INT32 lSkillTID = pcsSkillTemplate->m_lID;

		if(m_pagpmSkill->IsCashSkill(pcsSkillTemplate) ||
			m_pagpmSkill->IsSummonsSkill(pcsSkillTemplate) ||
			m_pagpmSkill->IsSummonsSkillForSiegeWar(pcsSkillTemplate) ||
			m_pagpmSkill->GetActionOnActionType(pcsSkillTemplate) == 6 ||
			m_pagpmSkill->IsCharacterConfigSkill(pcsSkillTemplate))
		{
			++lIndex;
			continue;
		}

		vcTmp.push_back(lSkillTID);
		++lIndex;
	}

	if(vcTmp.size() == 0)
		return TRUE;

	// 랜덤하게 지운다.
	std::random_shuffle(vcTmp.begin(), vcTmp.end());
	if(lMaxCount == 0)
		lMaxCount = (INT32) vcTmp.size();

	for(int i = 0; i < lMaxCount; ++i)
	{
		EndBuffedSkillByTID(pcsBase, vcTmp[i], bInitCoolTime);
	}

	return TRUE;
}

// 2005.11.09. steeple
// Buff 된 스킬중에서 Debuff 를 없앤다.
BOOL AgsmSkill::EndAllDebuffSkill(ApBase *pcsBase, BOOL bCreateTempSkill, INT32 lRate)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate)
			break;

		if(!m_pagpmSkill->IsDebuffSkill((AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate))
		{
			++lIndex;
			continue;
		}

		// SkillRate Check
		if(lRate != 100)
		{
			INT32 lRandom = m_csRandom.randInt(100);
			if(lRandom >= lRate)
			{
				++lIndex;
				continue;
			}
		}

		EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndAllDebuffSkill"));
	}

	return TRUE;
}

// 2005.08.25. steeple
// Buff 된 스킬중에서 이속을 올려주는 놈들을 지운다.
BOOL AgsmSkill::EndMovementBuffedSkill(ApBase* pcsBase, BOOL bCreateTempSkill)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = NULL;

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate)
			break;

		pcsAgsdSkill = GetADSkill(pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill);
		if(!pcsAgsdSkill)
			break;

		// Factor 를 찾아본다.
		FLOAT fValue = 0.0f;
		m_pagpmFactors->GetValue(pcsAgsdSkill->m_pcsUpdateFactorPoint, &fValue, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT);
		if((UINT32)fValue > 0)
		{
			EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndMovementBuffedSkill - 1"));
			continue;
		}

		fValue = 0.0f;
		m_pagpmFactors->GetValue(pcsAgsdSkill->m_pcsUpdateFactorPercent, &fValue, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT);
		if((UINT32)fValue > 0)
		{
			EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndMovementBuffedSkill - 2"));
			continue;
		}

		fValue = 0.0f;
		m_pagpmFactors->GetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPoint, &fValue, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT);
		if((UINT32)fValue > 0)
		{
			EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndMovementBuffedSkill - 3"));
			continue;
		}

		fValue = 0.0f;
		m_pagpmFactors->GetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPercent, &fValue, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT);
		if((UINT32)fValue > 0)
		{
			EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndMovementBuffedSkill - 4"));
			continue;
		}

		++lIndex;
	}

	return TRUE;
}

// 2005.10.09. steeple
// 소환수 버프 스킬을 없앤다.
BOOL AgsmSkill::EndSummonsBuffedSkill(ApBase* pcsBase, BOOL bCreateTempSkill, INT32 lTID)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = NULL;
	AgpdSkillTemplate* pcsSkillTemplate = NULL;
	INT32 lSkillLevel = 0;

	for(INT32 i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate)
			break;

		pcsSkillTemplate = (AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate;
		lSkillLevel = GetModifiedSkillLevel(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);
		if(!lSkillLevel)
			continue;

		if(m_pagpmSkill->IsSummonsSkill(pcsSkillTemplate) || m_pagpmSkill->IsFixedSkill(pcsSkillTemplate))
		{
			// lTID 가 0 이거나 (모든 소환관련된 놈 다 지운다.)
			// lTID 가 0 이 아니면 그 해당 TID 관련된 놈만 지운다.
			if(lTID == 0 || (lTID != 0 && lTID == (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SUMMONS_TID][lSkillLevel]))
			{
				EndBuffedSkill(pcsBase, i, bCreateTempSkill, _T("EndSummonsBuffedSkill"));
				--i;
				continue;
			}
		}
	}

	return TRUE;
}

// 2005.09.30. steeple
// 버프된 스킬 중에서 테임관련된 것들을 지운다.
BOOL AgsmSkill::EndTameBuffedSkill(ApBase* pcsBase, BOOL bCreateTempSkill)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = NULL;

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate)
			break;

		if(!m_pagpmSkill->IsTameSkill((AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate))
		{
			++lIndex;
			continue;
		}

		EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndTameBuffedSkill"));
	}

	return TRUE;
}

// 2005.10.07. steeple
// 버프된 스킬 중에서 투명 관련된 것들을 지운다.
BOOL AgsmSkill::EndTransparentBuffedSkill(ApBase* pcsBase, BOOL bCreateTempSkill)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = NULL;

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate)
			break;

		if(!m_pagpmSkill->IsTransparentSkill((AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate))
		{
			++lIndex;
			continue;
		}

		EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndTransparentBuffedSkill"));
	}

	return TRUE;
}

// 2005.12.09. steeple
// 버프된 캐쉬 아이템 스킬 중에서 넘어온 SkillTID 와 같은 것을 빼준다.
BOOL AgsmSkill::EndCashItemBuffedSkill(ApBase* pcsBase, INT32 lSkillTID, BOOL bCreateTempSkill)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = NULL;

	for(INT32 i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate)
			break;

		if(m_pagpmSkill->IsCashSkill((AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate) &&
			pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate->m_lID == lSkillTID)
		{
			EndBuffedSkill(pcsBase, i, bCreateTempSkill, _T("EndCashItemBuffedSkill"));
			--i;
			continue;
		}
	}

	return TRUE;
}

BOOL AgsmSkill::EndBuffedSkillOnEvolution(ApBase* pcsBase)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		AgpdSkill* pcsTempSkill = pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill;

		if(!pcsTempSkill || !pcsTempSkill->m_pcsTemplate)
			break;

		if(m_pagpmSkill->IsDebuffSkill((AgpdSkillTemplate*)pcsTempSkill->m_pcsTemplate))
		{
			++lIndex;
			continue;
		}

		if(m_pagpmSkill->IsCashSkill((AgpdSkillTemplate*)pcsTempSkill->m_pcsTemplate))
		{
			++lIndex;
			continue;
		}

		// 진화 스킬은 스킬 캐스팅이 끝날때 지워지므로 여기서는 남긴다.
		if(m_pagpmSkill->IsEvolutionSkill((AgpdSkillTemplate*)pcsTempSkill->m_pcsTemplate))
		{
			++lIndex;
			continue;
		}

		EndBuffedSkill(pcsBase, lIndex, TRUE, _T("EndAllBuffedSkillExceptDebuff"));
	}

	return TRUE;
}
//JK_쉬라인추가
BOOL AgsmSkill::EndRaceBuffedSkill(ApBase* pcsBase, BOOL bCreateTempSkill)
{
	if(!pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = NULL;

	INT32 lIndex = 0;
	while(lIndex < AGPMSKILL_MAX_SKILL_BUFF)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate)
			break;

		if(!m_pagpmSkill->IsBattleGroundRaceSkill((AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[lIndex].m_pcsTempSkill->m_pcsTemplate))
		{
			++lIndex;
			continue;
		}

		EndBuffedSkill(pcsBase, lIndex, bCreateTempSkill, _T("EndRaceBuffedSkill"));
	}

	return TRUE;
}

//		ApplySkill
//	Functions
//		- pcsSkill 의 효과를 적용한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsEvent
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ApplySkill(AgpdSkill *pcsSkill, ApdEvent *pcsEvent)
{
	PROFILE("AgsmSkill::ApplySkill");

 	if (!pcsSkill || !pcsSkill->m_pcsBase || !pcsSkill->m_pcsTemplate || !pcsEvent)
	{
		SendCastSkill(pcsSkill, AGPMSKILL_ACTION_FAILED_CAST, NULL, 0);
		return FALSE;
	}

	CHAR szTmp[64];
	sprintf(szTmp, "ApplySkill;; CTID:%d, STID:%d", ((AgpdCharacter*)pcsSkill->m_pcsBase)->m_pcsCharacterTemplate->m_lID,
													pcsSkill->m_pcsTemplate->m_lID);
	//STOPWATCH2(GetModuleName(), szTmp);

	AgpdSkillTemplate *pcsSkillTemplate = (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

	// 2007.04.04. steeple
	// 앞으로는 lCIDBuffer2 만 쓴다.
	ApSafeArray<INT_PTR, AGSMSKILL_MAX_TARGET_NUM>	lCIDBuffer;
	lCIDBuffer.MemSetAll();
	ApSafeArray<INT32, AGSMSKILL_MAX_TARGET_NUM>	lCIDBuffer2;
	lCIDBuffer2.MemSetAll();

	// 먼저 range conditon을 검사해 타겟을 가져온다.
	INT32	lNumTarget = RangeCheck(pcsSkill, &lCIDBuffer[0], AGSMSKILL_MAX_TARGET_NUM - 1, &lCIDBuffer2[0], AGSMSKILL_MAX_TARGET_NUM - 1, pcsEvent);

	BOOL	bIsTargetSelfCharacter	= FALSE;

	if (lNumTarget > AGSMSKILL_MAX_TARGET_NUM - 1)
		lNumTarget	= AGSMSKILL_MAX_TARGET_NUM - 1;

	// 2007.03.26. steeple
	// RangeCheck 에서는 ApBase* 로 넘어온다.
	// 여기서 다 CID 로 변경해준다.
	if (lNumTarget > 0)
	{
		for (int i = 0; i < lNumTarget; ++i)
		{
			if (lCIDBuffer2[i] == pcsSkill->m_pcsBase->m_lID)
			{
				bIsTargetSelfCharacter	= TRUE;
				break;
			}
		}

		// 그라운드 타겟은 아래 작업 하지 않는다. 2007.07.10. steeple
		if (!bIsTargetSelfCharacter && !m_pagpmSkill->IsGroundTargetSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
		{
			lCIDBuffer2[lNumTarget++] = pcsSkill->m_pcsBase->m_lID;
		}

		if (lNumTarget == 1 && bIsTargetSelfCharacter)
		{
			lCIDBuffer2[0] = pcsSkill->m_pcsBase->m_lID;
		}
		else
		{
			//for (int j = 0; j < lNumTarget; ++j)
			//	lCIDBuffer2[j] = ((ApBase*)lCIDBuffer[j])->m_lID;
		}
	}

	lNumTarget	= TargetFiltering(pcsSkill, lCIDBuffer2, lNumTarget);

	// 2005.09.19. steeple
	// 자기 자신이 타겟이었다고 하더라도 TargetFiltering 에서 빠질 수 있으므로 여기서 다시 한번 체크한다.
	bIsTargetSelfCharacter = FALSE;
	for(INT32 i = 0; i < lNumTarget; ++i)
	{
		if(pcsSkill->m_pcsBase->m_lID == lCIDBuffer2[i])
		{
			bIsTargetSelfCharacter = TRUE;
			break;
		}
	}

	// 2005.07.20. steeple
	// Dynamic Target 스킬이라면 타겟을 재조정 해준다.
	if(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lEffectType2[0] & AGPMSKILL_EFFECT2_DYNAMIC_TARGET)
		lNumTarget = DynamicTargetFiltering(pcsSkill, lCIDBuffer2, lNumTarget, bIsTargetSelfCharacter);

	ApSafeArray<AgpdCharacter*, AGSMSKILL_MAX_TARGET_NUM> pacsCharacter;
	pacsCharacter.MemSetAll();

	// 2006.10.25. steeple
	// 타겟이 없어도 캐스팅 해야하는 스킬도 캐스팅 해야한다.
	//
	//
	// 2005.05.09. steeple lNumTarget 이 0보다 클때만 캐스트
	if ((m_pagpmSkill->IsForceCastSkill(pcsSkillTemplate) || lNumTarget > 0) &&
		!m_pagpmSkill->IsSphereRangeSiegeWarSkill(pcsSkillTemplate) &&		// 얘는 개별적으로 캐스팅해준다.
		!(m_pagpmSkill->IsUnlimitedTargetSiegeWarSkill(pcsSkillTemplate) && lNumTarget == 1) &&
		!pcsSkillTemplate->m_bShrineSkill && !m_pagpmSkill->IsPassiveSkill(pcsSkill))
	{
		PROFILE("AgsmSkill::ApplySkill SendCastSkill");

		for(INT32 i = 0; i < lNumTarget; ++i)
			pacsCharacter[i] = m_pagpmCharacter->GetCharacter(lCIDBuffer2[i]);

		// 이 스킬 사용된다는 정보를 주변에 뿌려준다.
		if (!SendCastSkill(pcsSkill, AGPMSKILL_ACTION_CAST_SKILL, &lCIDBuffer2[0], lNumTarget * (INT16)sizeof(INT32), &pacsCharacter[0], lNumTarget))
			return FALSE;

		// 스킬 시전에 따라 소모되는 hp, mp, sp등을 계산한다.
		CostApply(pcsSkill);
	}

	// 2005.05.09. steeple. 타겟이 없어서 실패한 경우에는 리캐스트 딜레이를 다시 돌려준다.
	if(lNumTarget < 1)
	{
		// 2005.07.25. steeple 타겟이 없어서 실패한 경우 아래 체크를 해준다.
		AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
		if(pcsAgsdSkill)
			pcsAgsdSkill->m_eMissedReason = AGSDSKILL_MISSED_REASON_TARGET_NUM_ZERO;

		SendCastSkill(pcsSkill, AGPMSKILL_ACTION_TARGET_NUM_ZERO, NULL, 0);
		return FALSE;
	}

	// 2004.11.08. steeple
	// 패킷 두번 보내는 스킬인데, 첫번째 온 패킷처리다아아아아~
	if(IsTwicePacketSkill(pcsSkill))
		ApplySkillSetTwicePacket(pcsSkill, pcsEvent, lCIDBuffer2, lNumTarget);
	else
		ApplySkillFinal(pcsSkill, pcsEvent, lCIDBuffer2, lNumTarget, bIsTargetSelfCharacter);

	return TRUE;
}

// 2004.11.08. steeple
// 캐스트 할때와 실제 공격시점이 다른 스킬의 사용이다.
// 마지막에 캐스트한 스킬ID 와 시간을 저장하고, 타겟CID 를 저장한다.
BOOL AgsmSkill::ApplySkillSetTwicePacket(AgpdSkill* pcsSkill, ApdEvent* pcsEvent, ApSafeArray<INT32, AGSMSKILL_MAX_TARGET_NUM>& lCIDBuffer, INT32 lNumTarget)
{
	// pcsEvent 는 NULL 일 수 있다. NULL 체크 잘 확인하자!!!n
	if(!pcsSkill || !pcsSkill->m_pcsBase || /*!pcsEvent ||*/ lNumTarget < 1)
		return FALSE;

	if(!pcsSkill->m_pcsBase)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsSkill->m_pcsBase);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	// 빈 버퍼를 찾는다.
	INT32 i = 0;
	for(i; i < AGPMSKILL_MAX_TWICE_PACKET_BUFFER; i++)
	{
		if(pcsAttachData->m_alLastCastTwiceSkillID[i] == 0)
			break;
	}

	if(i == AGPMSKILL_MAX_TWICE_PACKET_BUFFER)
	{
		// 하나씩 앞으로 당기고 맨 마지막에 넣는다. 2005.07.25. steeple
		pcsAttachData->m_alLastCastTwiceSkillID.MemCopy(0, &pcsAttachData->m_alLastCastTwiceSkillID[1], AGPMSKILL_MAX_TWICE_PACKET_BUFFER - 1);
		pcsAttachData->m_alLastCastTwiceSkillID[AGPMSKILL_MAX_TWICE_PACKET_BUFFER - 1] = 0;
		i = AGPMSKILL_MAX_TWICE_PACKET_BUFFER - 1;
	}

	pcsAttachData->m_alLastCastTwiceSkillID[i] = pcsSkill->m_lID;

	pcsAgsdSkill->m_ulLastCastTwiceSkillClock = GetClockCount();
	pcsAgsdSkill->m_alLastTwiceSkillTargetCID.MemSetAll();
	for(int i = 0; i < lNumTarget; i++)
	{
		pcsAgsdSkill->m_alLastTwiceSkillTargetCID[i] = (INT32)lCIDBuffer[i];
	}
	pcsAgsdSkill->m_lLastTwiceSkillNumTarget = lNumTarget;

	return TRUE;
}

// 2005.01.31. steeple
// Buffering 을 넣었음. 후후훗~
//
// 2004.11.08. steeple
// 캐스트 할때와 실제 공격시점이 다른 스킬의 사용이다.
// 실제 공격시점이다. AttachedData 에 저장되어 있는 스킬ID, ClockCount 와 비교한 후
// 유효한 스킬이라면 타겟 배열을 만들어서 ApplySkillFinal 을 호출한다.
BOOL AgsmSkill::ApplySkillTwicePacket(AgpdSkill* pcsSkill, ApdEvent* pcsEvent)
{
	// pcsEvent 는 NULL 일 수 있다. NULL 체크 잘 확인하자!!!
	if(!pcsSkill || !pcsSkill->m_pcsBase)// || !pcsEvent)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsSkill->m_pcsBase);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	// Skill ID 검사
	int i = 0;
	for(i; i < AGPMSKILL_MAX_TWICE_PACKET_BUFFER; i++)
	{
		if(pcsAttachData->m_alLastCastTwiceSkillID[i] == pcsSkill->m_lID)
			break;
	}

	// 시전한 놈이 없으면 return
	if(i == AGPMSKILL_MAX_TWICE_PACKET_BUFFER)
		return FALSE;

	// 2006.07.19. steeple
	// 드디어 Twice Packet Real Hit 쪽에도 CastDelay 검사 추가.
	// 시간 검사
	// ...
	if(m_pagpmSkill->GetActionOnActionType((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) != 2)
	{
		UINT32 ulClockCount = GetClockCount();
		if(pcsSkill->m_ulCastDelay > ulClockCount)
			return FALSE;
	}

	// Target 검사
	if(pcsAgsdSkill->m_lLastTwiceSkillNumTarget < 1)
		return FALSE;

	ApSafeArray<INT32, AGSMSKILL_MAX_TARGET_NUM> lCIDBuffer;
	lCIDBuffer.MemSetAll();
	ApSafeArray<AgpdCharacter*, AGSMSKILL_MAX_TARGET_NUM> pacsCharacter;
	pacsCharacter.MemSetAll();

	AgpdCharacter* pcsCharacter = NULL;
	INT32 lNewNumTarget = 0;
	BOOL bIsTargetSelfCharacter = FALSE;

	for(int i = 0; i < pcsAgsdSkill->m_lLastTwiceSkillNumTarget; i++)
	{
		pcsCharacter = m_pagpmCharacter->GetCharacter(pcsAgsdSkill->m_alLastTwiceSkillTargetCID[i]);
		if(pcsCharacter)
		{
			lCIDBuffer[lNewNumTarget] = pcsAgsdSkill->m_alLastTwiceSkillTargetCID[i];
			pacsCharacter[lNewNumTarget] = pcsCharacter;
			lNewNumTarget++;

			if(pcsSkill->m_pcsBase && pcsAgsdSkill->m_alLastTwiceSkillTargetCID[i] == pcsSkill->m_pcsBase->m_lID)
				bIsTargetSelfCharacter = TRUE;
		}
	}

	if(lNewNumTarget > 0)
	{
		// 2004.12.09. steeple
		// 소스 날아가서 다시 작성.. TT
		// 스킬의 시작 시간, 끝나는 시간을 세팅해준다.
		INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);

		UINT32 ulCurrentClock = GetClockCount();
		AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;

		pcsSkill->m_ulStartTime = ulCurrentClock;
		UINT32 ulCastDelay = m_pagpmSkill->GetCastDelay((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate, lSkillLevel);

		// 2005.12.4. steeple
		INT32 ulAdjustCastDelay = (INT32)((FLOAT)(ulCastDelay) * (FLOAT)m_pagpmSkill->GetAdjustCastDelay((AgpdCharacter*)pcsSkill->m_pcsBase) / 100.0f);
		ulCastDelay += ulAdjustCastDelay;

		if(pcsSkillTemplate->m_bShrineSkill)
		{
				pcsSkill->m_ulEndTime = 0xFFFFFFFF;
				pcsSkill->m_ulInterval = 0;
				pcsSkill->m_ulNextProcessTime = 0;
		}
		else
		{
			// 2005.09.12. steeple
			// 아 이거 일일이 다 찾아서 고쳐야 하는 거야 ㅠㅠ
			// 그러기 좀 뭐 해서 Buff 관련 스킬에만 이렇게 해줌.
			if(m_pagpmSkill->IsDurationByDistanceSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) ||
				m_pagpmSkill->IsDurationUnlimited((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
				pcsSkill->m_ulEndTime = 0xFFFFFFF;
			else if(m_pagpmSkill->GetSkillDurationTime(pcsSkill, lSkillLevel) > 0)
				pcsSkill->m_ulEndTime = ulCurrentClock + /*ulCastDelay +*/ m_pagpmSkill->GetSkillDurationTime(pcsSkill, lSkillLevel);
			else
				pcsSkill->m_ulEndTime = ulCurrentClock;

			pcsSkill->m_ulInterval = m_pagpmSkill->GetSkillInterval(pcsSkill, lSkillLevel);
			if (pcsSkill->m_ulInterval)
				pcsSkill->m_ulNextProcessTime = ulCurrentClock + /*ulCastDelay +*/ pcsSkill->m_ulInterval;
			else
				pcsSkill->m_ulNextProcessTime = pcsSkill->m_ulEndTime;
		}

		// 2005.07.07. steeple
		// Target 이 Self 가 아니고 하나일때 ActionOnActionType2 를 불러준다.
		if(!bIsTargetSelfCharacter && lNewNumTarget == 1)
			m_pagsmSkillEffect->ProcessSkillEffect2ActionOnAction(pcsSkill, pacsCharacter[0], AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE2, FALSE);
		
		ApplySkillFinal(pcsSkill, pcsEvent, lCIDBuffer, lNewNumTarget, bIsTargetSelfCharacter);
	}

	// Last Twice Cast Skill Init.
	InitTwicePacketSkill(pcsSkill);

	// 2006.11.28. steeple
	EndSkill(pcsSkill);

	return TRUE;
}

// 2004.11.08. steeple
// ApplySkill 시에 하는 유효체크를 모두 성공한 후에 불리게 된다.
BOOL AgsmSkill::ApplySkillFinal(AgpdSkill* pcsSkill, ApdEvent* pcsEvent, ApSafeArray<INT32, AGSMSKILL_MAX_TARGET_NUM>& lCIDBuffer, INT32 lNumTarget, BOOL bIsTargetSelfCharacter)
{
	// pcsEvent 는 NULL 일 수 있다. NULL 체크 잘 확인하자!!!
	if(!pcsSkill || !pcsSkill->m_pcsBase || /*!pcsEvent ||*/ lNumTarget < 1)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	UINT32	ulCurrentClock	= GetClockCount();
	if (m_pagpmSkill->IsAttackSkill((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate))
		m_pagpmCharacter->SetCombatMode((AgpdCharacter *) pcsSkill->m_pcsBase, ulCurrentClock);

	//INT16	nAffectedIndex	= 0;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	CHAR szTmp[64];
	sprintf(szTmp, "ApplySkillFinal;; CTID:%d, STID:%d", ((AgpdCharacter*)pcsSkill->m_pcsBase)->m_pcsCharacterTemplate->m_lID,
														pcsSkill->m_pcsTemplate->m_lID);
	//STOPWATCH2(GetModuleName(), szTmp);

	// 2006.08.11. steeple
	// 타겟수가 제한없는 공성스킬이면 이거 한번 해준다.
	if(m_pagpmSkill->IsUnlimitedTargetSiegeWarSkill(pcsSkillTemplate))
	{
		ApplySkillUnlimitedTargetSiegeWar(pcsSkill, lCIDBuffer, lNumTarget, bIsTargetSelfCharacter);

		// 공격스킬인데 타겟이 자기 자신이라면 나가준다.
		if(lCIDBuffer[0] && m_pagpmPvP->IsAttackSkill(pcsSkillTemplate) &&
			pcsSkill->m_pcsBase->m_lID == lCIDBuffer[0])
			return FALSE;
	}

	// 2007.10.23. steeple
	// 커넥션 스킬인지 아닌지 설정을 해준다.
	SetConnectionInfo(pcsSkill);

	// 2005.05.24. steeple
	// SkillScroll 은 하나만 까줘야 하기에 이렇게 처리.
	BOOL bSkillScrollSubStack = FALSE;
	BOOL bCashItemSubStack = FALSE;	// 2005.12.12. steeple. 위의 것과 같은데 이놈은 CashItem 전용

	// 위에서 가져온 타겟을 하나하나 condition check, process skill effect를 처리한다.
	for(int i = 0; i < lNumTarget; ++i)
	{
		PROFILE("AgsmSkill::ApplySkill ProcessTarget");

		// 2005.09.15. steeple
		pcsAgsdSkill->m_eMissedReason = AGSDSKILL_MISSED_REASON_NORMAL;

		if (!pcsSkillTemplate->m_bShrineSkill)
		{
			m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorPoint);
			m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorPercent);
			m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorResult);
			m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPoint);
			m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPercent);
		}

		ApBase	*pcsTargetBase = (ApBase*)m_pagpmCharacter->GetCharacter(lCIDBuffer[i]);
		if(!pcsTargetBase)
			continue;

		if (!bIsTargetSelfCharacter && pcsTargetBase == pcsSkill->m_pcsBase)
			continue;

		if (((AgpdCharacter*)pcsTargetBase)->m_bIsReadyRemove)
			continue;

		if (pcsTargetBase != pcsSkill->m_pcsBase && !pcsTargetBase->m_Mutex.WLock())
			continue;

		// 2007.09.06. steeple
		// 죽은 타겟을 할 수 있는 스킬 추가.
		//
		// 2005.05.22. steeple
		// 패시브 스킬이면 체크 안한다.
		//
		// 2004.12.09. steeple
		// 소스 날아가서 다시 작성.
		// 죽었으면 continue
		if ((!m_pagpmSkill->IsPassiveSkill(pcsSkill) &&
			!m_pagpmSkill->IsDeadTarget(pcsSkillTemplate) &&
			((AgpdCharacter*)pcsTargetBase)->m_unActionStatus == AGPDCHAR_STATUS_DEAD) ||
			IsMaxBuffedSkill(pcsTargetBase, pcsSkill))
		{
			if (pcsSkill->m_pcsBase->m_lID != pcsTargetBase->m_lID)
				pcsTargetBase->m_Mutex.Release();

			continue;
		}

		// Classify 검사한다. 2005.01.26. steeple
		// 여기서 실패하면 걍 내보내준다.
		if(!IsEnableSkillByBuffedList(pcsSkill, pcsTargetBase))
		{
			// System Message 보낸다. 2005.01.31. steeple. 임시 땜빵
			if(m_pagpmPvP && pcsTargetBase->m_eType == APBASE_TYPE_CHARACTER)
				m_pagpmPvP->ProcessSystemMessage(AGPMPVP_SYSTEM_CODE_SKILL_CANNOT_APPLY_EFFECT, NULL, NULL, 0, 0, (AgpdCharacter*)pcsTargetBase);

			if (pcsSkill->m_pcsBase->m_lID != pcsTargetBase->m_lID)
				pcsTargetBase->m_Mutex.Release();

			continue;
		}

		// ItemScroll Skill 이라면 여기서 까준다. 2005.01.26. steeple
		if (pcsAgsdSkill->m_lSkillScrollIID != AP_INVALID_IID)
		{
			AgpdItem* pcsSkillScroll = m_pagpmItem->GetItem(pcsAgsdSkill->m_lSkillScrollIID);
			if(pcsSkillScroll && pcsSkillScroll->m_pcsItemTemplate)
			{
				INT32 lItemTID = ((AgpdItemTemplate*) pcsSkillScroll->m_pcsItemTemplate)->m_lID;
				m_pagsmItem->SendPacketUseItemByTID(pcsSkillScroll->m_pcsCharacter,
													lItemTID,
													pcsTargetBase->m_lID,
													m_pagpmItem->GetRemainTimeByTID(pcsSkillScroll->m_pcsCharacter, lItemTID));

				// 2005.05.24. steeple
				// 한번만 까주게 하려고 이렇게 바꿈.
				if(!bSkillScrollSubStack)
				{
					// 정량제, 혹은 정액제 아이템은 시간이 다 경과한 후에 지운다. 20051202, kelovon
					// 2005.12.12. steeple. 공격시 까는 것도 추가.
					if (pcsSkillScroll->m_pcsItemTemplate->m_eCashItemType != AGPMITEM_CASH_ITEM_TYPE_PLAY_TIME
						&& pcsSkillScroll->m_pcsItemTemplate->m_eCashItemType != AGPMITEM_CASH_ITEM_TYPE_REAL_TIME
						&& pcsSkillScroll->m_pcsItemTemplate->m_eCashItemType != AGPMITEM_CASH_ITEM_TYPE_STAMINA
						&& pcsSkillScroll->m_pcsItemTemplate->m_eCashItemType != AGPMITEM_CASH_ITEM_TYPE_ONE_ATTACK)
					{
						m_pagpmItem->SubItemStackCount(pcsSkillScroll, 1);
						bSkillScrollSubStack = TRUE;

						// Detect Skill 이라면 AddionalEffect 보내준다. 2006.10.18. steeple
						if(m_pagpmSkill->IsDetectSkill(pcsSkillTemplate))
							SendAdditionalEffect(pcsSkill->m_pcsBase, AGPMSKILL_ADDITIONAL_EFFECT_DETECT_SCROLL);
					}
				}
			}
		}
		else if(bCashItemSubStack == FALSE && m_pagpmSkill->IsPassiveSkill(pcsSkill) == FALSE)
		{
			// 스킬 사용시에 개수 줄여야 하는 캐쉬 아이템 처리. 2005.12.12. steeple
			if(m_pagpmSkill->IsSubCashItemStackCountOnSkill(pcsSkillTemplate))	// 빼야하는 것만 뺀다. 2006.01.10. steeple
				m_pagpmItem->SubCashItemStackCountOnAttack((AgpdCharacter*)pcsSkill->m_pcsBase);
			bCashItemSubStack = TRUE;
		}


		// 2006.10.18. steeple
		// Detect Skill 처리.
		// TargetFiltering 함수에서 자기 자신도 포함하기 때문이다.
		// 타겟이 없어도 아이템은 하나 줄여줘야 하므로 이곳에서 처리했다.
		if(m_pagpmSkill->IsDetectSkill(pcsSkillTemplate))
		{
			// 자기 자신이면 지나간다.
			if(pcsSkill->m_pcsBase->m_lID == pcsTargetBase->m_lID)
				continue;
	
			// 2007.07.04. steeple. Detect Type 이 추가 되면서 변경되었다.
			// Type1 에서만 아래 체크를 하자.
			//
			// 2006.09.21. steeple. 디텍트 체크
			// 완전 투명인 상태의 애들한테만 걸어준다.
			if(m_pagpmSkill->GetDetectType(pcsSkillTemplate) == 1)
			{
				if(!m_pagpmCharacter->IsStatusTransparent((AgpdCharacter*)pcsTargetBase) ||
					m_pagpmCharacter->IsStatusHalfTransparent((AgpdCharacter*)pcsTargetBase))
				{
					pcsTargetBase->m_Mutex.Release();
					continue;
				}
			}
		}


		/*
		if (!pcsSkill->m_bForceAttack)
		{
			if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER && pcsTargetBase->m_eType == APBASE_TYPE_CHARACTER)
			{
				INT32	lAttackUnion = m_pagpmCharacter->GetUnion((AgpdCharacter *) pcsSkill->m_pcsBase);
				INT32	lTargetUnion = m_pagpmCharacter->GetUnion((AgpdCharacter *) pcsTargetBase);
				if (lAttackUnion == lTargetUnion)
				{
					continue;
				}
			}
		}
		*/

		//여기까지 왔다면 시도는했다. 그러니 어그로를 올리자.
		// 2004.12.09. steeple
		// AGRO 를 1 만 올리면 너무 티가 안나서 10 으로 변경. (소스 날아가서 다시 작성.)
		//
		// 2005.06.15. steeple
		// 에너지가 많은 놈들에겐 티가 전혀 안난다. Agro Point 를 타겟의 MaxHP 의 10% 만큼 쌓아준다. -_-;;
		if (m_pagpmCharacter->IsPC((AgpdCharacter *)pcsTargetBase) == FALSE &&
			//!= m_pagpmCharacter->IsPC((AgpdCharacter *)pcsSkill->m_pcsBase) &&
			m_pagpmSkill->IsNotAddAgroAtCast(pcsSkillTemplate) == FALSE)	// Cast 시 Agro 올리지 않는 스킬인지 체크
		{
			INT32 lMaxHP = m_pagpmFactors->GetMaxHP(&((AgpdCharacter*)pcsTargetBase)->m_csFactor);
			if(lMaxHP > 0)
				m_pagsmCharacter->AddAgroPoint( (AgpdCharacter *)pcsTargetBase, (AgpdCharacter *)pcsSkill->m_pcsBase, AGPD_FACTORS_AGRO_TYPE_SLOW, (INT32)((FLOAT)lMaxHP / 10.0f) );
		}

		// 2005.11.29. steeple
		// 몹 몰이 패널티를 위한 작업.
		if(m_pagpmCharacter->IsPC((AgpdCharacter*)pcsSkill->m_pcsBase) && m_pagpmCharacter->IsMonster((AgpdCharacter*)pcsTargetBase))
		{
			m_pagsmCharacter->AddTargetInfo((AgpdCharacter*)pcsSkill->m_pcsBase, pcsTargetBase->m_lID, ulCurrentClock);
		}

		if (m_pagpmSkill->IsAttackSkill((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate))
			m_pagpmCharacter->SetCombatMode((AgpdCharacter *) pcsTargetBase, ulCurrentClock);

		// 스킬 공격에 대해서 무적인지 확인한다. 2007.07.02. steeple
		if (!SkillInvincibleCheck(pcsSkill, (AgpdCharacter*)pcsTargetBase))
		{
			SendMissCastSkill(pcsSkill, pcsTargetBase);

			if (pcsSkill->m_pcsBase->m_lID != pcsTargetBase->m_lID)
				pcsTargetBase->m_Mutex.Release();

			continue;
		}

		// pcsTargetBase의 버프된 스킬중 이 스킬에 대한 효과를 없앨 수 있는넘이 있는지 검사한다.
		////////////////////////////////////////////////////////////////////////
		UINT32	ulCheckResultPre	= 0;
		PVOID					pvBuffer[3];
		pvBuffer[0]				= (PVOID)	pcsSkill;
		pvBuffer[1]				= (PVOID)	pcsTargetBase;
		pvBuffer[2]				= (PVOID)	&ulCheckResultPre;

		// PreCheck시 Block 가능여부 판단을 포함시킴 
		EnumCallback(AGSMSKILL_CB_PRE_CHECK_CAST, pvBuffer, NULL);

		BOOL	bCheckAllOK = TRUE;

		// Skill Rate Check - 2004.09.17. steeple
		if (!SkillRateCheck(pcsSkill))
		{
			bCheckAllOK	= FALSE;
		}
		else
		{
			// 최대치만큼 루프를 돌면서 condition, process를 체크한다.
			for (int j = 0; j < AGPMSKILL_MAX_SKILL_CONDITION; ++j)
			{
				//if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[j] == 0)
				//	break;

				BOOL	bProcessSkill	= FALSE;

				// condition check...
				if (ConditionCheck(pcsSkill, pcsTargetBase, j))
				{
					bProcessSkill	= TRUE;
				}

				if (pcsSkillTemplate->m_lConditionType[j] & AGPMSKILL_CONDITION_SKIP_CHECK_FIRST || bProcessSkill)
				{
					// check that same skill is buffed
					////////////////////////////////////////////////////////////////////
					if (!ProcessAlreadyBuffedSkill(pcsSkill, pcsTargetBase))
					{
						bCheckAllOK = FALSE;
						break;
					}

					// 2005.03.09. steeple
					// Post Check 결과를 보고, Factor 를 적용시키지 않는 거라면 파괴하고 continue.
					if(ulCheckResultPre & AGPDCHAR_ACTION_RESULT_TYPE_ATTACK_BLOCK)
					{
						break;
					}


					// if condition is true, process skill effect
					////////////////////////////////////////////////////////////////////
					if( FALSE == m_pagsmSkillEffect->ProcessSkillEffect(pcsSkill, pcsTargetBase, j) )
						bCheckAllOK = FALSE;
/*
					// 몬스터가 큐어를 쓸때 이펙트 터뜨려준다.. 근데 하드코딩일수밖에는 없는가?!
					// 2008. 08. 20. iluvs
					if(pcsSkill->m_pcsTemplate->m_lID == 905)
						SendAdditionalEffect(pcsSkill->m_pcsBase, AGPMSKILL_ADDITIONAL_EFFECT_CURE);
*/
				}
				else
				{
					bCheckAllOK = FALSE;
					break;
				}
			}
		}

		// 2005.10.11. steeple
		if(pcsAgsdSkill->m_eMissedReason == AGSDSKILL_MISSED_REASON_TAME_FAILURE_ABNORMAL)
			bCheckAllOK = FALSE;

		if (pcsSkillTemplate->m_bShrineSkill && bCheckAllOK)
		{
			// 이 스킬 사용된다는 정보를 주변에 뿌려준다.
			if (!SendCastSkill(pcsSkill, AGPMSKILL_ACTION_CAST_SKILL, &lCIDBuffer[0], lNumTarget * (INT16)sizeof(INT32)))
			{
				if (pcsSkill->m_pcsBase->m_lID != pcsTargetBase->m_lID)
					pcsTargetBase->m_Mutex.Release();

				continue;
			}
		}

		if (!pcsSkillTemplate->m_bShrineSkill && bCheckAllOK)
		{
			// 스킬이 성공한경우 뭔가 새로운 부가처리가 필요한지 체크한다.
			////////////////////////////////////////////////////////////////////////
			UINT32	ulCheckResultPost	= 0;
			PVOID					pvBuffer[3];
			pvBuffer[0]				= (PVOID)	pcsSkill;
			pvBuffer[1]				= (PVOID)	pcsTargetBase;
			pvBuffer[2]				= (PVOID)	&ulCheckResultPost;

			EnumCallback(AGSMSKILL_CB_POST_CHECK_CAST, pvBuffer, NULL);

			// 2005.09.06. steeple
			// 스킬이 성공한 후 적용됬다는 콜백 불러준다.
			EnumCallback(AGSMSKILL_CB_AFFECTED_SKILL, pcsSkill, pcsTargetBase);

			if(ulCheckResultPre & AGPDCHAR_ACTION_RESULT_TYPE_ATTACK_BLOCK)
			{
				// Skill을 Block했으므로 더이상 스킬에 관한 처리를 해줄 필요가 없다
				SendCastResult(pcsSkill, pcsSkill->m_pcsBase, pcsTargetBase, NULL, AGPMSKILL_ACTION_BLOCK_CAST_SKILL, FALSE, TRUE);

				if (pcsSkill->m_pcsBase->m_lID != pcsTargetBase->m_lID)
					pcsTargetBase->m_Mutex.Release();

				continue;
			}

			if (!m_pagpmSkill->IsPassiveSkill(pcsSkill))
			{
				// 2006.01.23. steeple
				// BuffedList 에 들어가는 지 확인을 먼저 한다.
				// BuffedBase 에 추가한다.
				if(IsMaxBuffedSkill(pcsTargetBase, pcsSkill) || !AddBuffedList(pcsSkill, pcsTargetBase))
				{
					if (pcsSkill->m_pcsBase->m_lID != pcsTargetBase->m_lID)
						pcsTargetBase->m_Mutex.Release();

					continue;
				}

				if(m_pagpmSkill->IsEvolutionSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
					continue;
			}

			// factor를 보고 업데이트 시킨다.
			if (!UpdateSkillFactor(pcsSkill, pcsTargetBase))
			{
				RestoreSkillFactor(pcsSkill, pcsTargetBase);

				if (pcsSkill->m_pcsBase->m_lID != pcsTargetBase->m_lID)
					pcsTargetBase->m_Mutex.Release();

				continue;
			}

			if (!m_pagpmSkill->IsPassiveSkill(pcsSkill) && m_pagpmSkill->IsAttackSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) )
			{
				// Attack Callback 을 불러준다.
				// 이 안에서 Dead 가 세팅된다.
				ApplySkillAttack(pcsSkill, pcsTargetBase);
			}

			// 2005.11.15. steeple
			// 주위 PC 로부터의 타겟을 해제하는 스킬이라면 아래와 같이 한다.
			if(m_pagpmSkill->IsReleaseTargetSkill(pcsSkillTemplate))
				SendAdditionalEffect(pcsTargetBase, AGPMSKILL_ADDITIONAL_EFFECT_RELEASE_TARGET, 0);
		}

		if (!bCheckAllOK)
		{
			// 스킬을 사용할 수 없다고 알려주던 뭘하던 한다.
			//
			//////////////////////////////////////////////////////////////////////////////
			
			// 2005.09.15. steeple
			// 적용은 되지 않았지만, Miss 라고 뜨면 안되는 것들 때문에 이렇게 처리.
			if(pcsAgsdSkill->m_eMissedReason == AGSDSKILL_MISSED_REASON_NORMAL)
			{
				UINT32 ulAdditionalEffect = 0;
				if(m_pagpmSkill->IsStatusActionOnActionType4(pcsSkill->m_pcsBase) &&
					m_pagpmSkill->IsSubCashItemStackCountOnSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
					ulAdditionalEffect |= AGPDCHAR_ADDITIONAL_EFFECT_LENS_STONE;

				SendMissCastSkill(pcsSkill, pcsTargetBase, ulAdditionalEffect);
			}
		}

		if (pcsSkill->m_pcsBase->m_lID != pcsTargetBase->m_lID)
			pcsTargetBase->m_Mutex.Release();

	//JK_패시브스킬중 크리티컬이 UI에 표시되지않는 현상
	///////////////////////////
		if (m_pagpmSkill->IsCriticalUpPassiveSkill(pcsSkill)) 
		{
			UINT32	dpnid = m_pagsmCharacter->GetCharDPNID(pcsTargetBase->m_lID);
			if(dpnid != 0)
			{
				INT16	nPacketLength = 0;
				PVOID	pvPacket = m_pagpmSkill->MakePacketSkill(pcsSkill, &nPacketLength);

				BOOL	bRetval = SendPacket(pvPacket, nPacketLength, dpnid);

				m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
			}
		}
	//////////////////////////////

	}

	return TRUE;
}


//		ApplySkillByBuffedList
//	Functions
//		- pcsSkill 의 효과를 적용한다. (BuffedList에 있는 넘을 갖고 계산한다.)
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsEvent
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ApplySkillByBuffedList(AgpdSkill *pcsSkill, ApBase *pcsTarget)
{
	PROFILE("AgsmSkill::ApplySkillByBuffedList");

	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsTarget)
		return FALSE;

	// 최대치만큼 루프를 돌면서 condition, process를 체크한다.
	for (int j = 0; j < AGPMSKILL_MAX_SKILL_CONDITION; ++j)
	{
		//if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[j] == 0)
		//	break;

		m_pagsmSkillEffect->ProcessSkillEffect(pcsSkill, pcsTarget, j, TRUE);
	}

	// factor를 보고 업데이트 시킨다.
	UpdateSkillFactor(pcsSkill, pcsTarget, TRUE, TRUE);

	// 2005.01.28. steeple. Attack 후의 죽은 것을 체크한다.
	ApplySkillAttack(pcsSkill, pcsTarget);

	return TRUE;
}

// 2005.01.23. steeple
// Attack Callback 을 불러준다.
// 이 콜백안에서 해당 캐릭터가 Dead 인지 세팅이 된다.
BOOL AgsmSkill::ApplySkillAttack(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	if(m_pagpmSkill->IsPassiveSkill(pcsSkill))
		return TRUE;

	ApBase* pcsOwnerBase = NULL;
	if(pcsSkill->m_bCloneObject &&
		pcsSkill->m_csBase.m_eType == APBASE_TYPE_CHARACTER)
		pcsOwnerBase = (ApBase*)m_pagpmCharacter->GetCharacterLock(pcsSkill->m_csBase.m_lID);
	else
		pcsOwnerBase = pcsSkill->m_pcsBase;

	// callback function (AGSMSKILL_CB_ATTACK) 을 호출해준다.
	//////////////////////////////////////////////////////////////////////////
	stAgsmCombatAttackResult stAttackResult;

	if(pcsSkill->m_bCloneObject)
		stAttackResult.pAttackChar = (AgpdCharacter*)pcsOwnerBase;
	else
		stAttackResult.pAttackChar = (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER) ? (AgpdCharacter*)pcsSkill->m_pcsBase : NULL;

	stAttackResult.pTargetChar = (AgpdCharacter*)pcsTarget;
	stAttackResult.nDamage = 0;

	EnumCallback(AGSMSKILL_CB_ATTACK, &stAttackResult, pcsSkill);

	// 2007.06.27. steeple
	// 타겟이 sleep 상태면 풀어준다.
	if(!m_pagpmSkill->IsSleepSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) && m_pagpmCharacter->IsStatusSleep((AgpdCharacter*)pcsTarget))
		m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*)pcsTarget, AGPDCHAR_SPECIAL_STATUS_SLEEP);

	if(pcsOwnerBase && pcsSkill->m_bCloneObject)
		pcsOwnerBase->m_Mutex.Release();

	return TRUE;
}

// 2006.08.11. steeple
// 타겟카운트 제한없는 공성스킬들은 타겟을 따로 구한다.
BOOL AgsmSkill::ApplySkillUnlimitedTargetSiegeWar(AgpdSkill* pcsSkill, ApSafeArray<INT32, AGSMSKILL_MAX_TARGET_NUM>& lCIDBuffer, INT32 lNumTarget, BOOL bIsTargetSelfCharacter)
{
	if(!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	if(!m_pagpmSkill->IsUnlimitedTargetSiegeWarSkill(pcsSkillTemplate))
		return TRUE;

	// 본인이 아니면 딱히 할 건 없다.
	if(pcsSkill->m_pcsBase->m_lID != lCIDBuffer[0])
		return TRUE;

	// 자기를 일단 타겟으로 가져온다.
	AgpdCharacter* pcsTarget = m_pagpmCharacter->GetCharacter(pcsSkill->m_pcsBase->m_lID);
	if(!pcsTarget)
		return FALSE;

	// 현재 공성 상태인지 확인한다.
	AgpdSiegeWar* pcsSiegeWar = m_pagpmSiegeWar->GetSiegeWarInfo(pcsTarget);
	if(!pcsSiegeWar)
		return TRUE;

	if(pcsSiegeWar->m_eCurrentStatus < AGPD_SIEGE_WAR_STATUS_START ||
		pcsSiegeWar->m_eCurrentStatus >= AGPD_SIEGE_WAR_STATUS_TIME_OVER)
		return TRUE;

	std::list<INT32> csTargetList;

	//// 테스트로 자기 길드 넣었다.
	//m_pagpmGuild->GetMemberCIDList(pcsTarget, &csTargetList);

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_SIEGE_WAR_TO_ALL_ATTACKERS)
	{
		INT32 lIndex = 0;
		AgpdGuild** ppcsGuild = (AgpdGuild**)pcsSiegeWar->m_csAttackGuild.GetObjectSequence(&lIndex);
		while(ppcsGuild && *ppcsGuild)
		{
			AuAutoLock csLock2((*ppcsGuild)->m_Mutex);
			if (csLock2.Result())
				m_pagpmGuild->GetMemberCIDList(*ppcsGuild, &csTargetList);

			ppcsGuild = (AgpdGuild**)pcsSiegeWar->m_csAttackGuild.GetObjectSequence(&lIndex);
		}

		// 오브젝트 몬스터들도 넣어야 하나??
		//
		//
	}
	else if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_SIEGE_WAR_TO_ALL_DEFENDERS)
	{
		INT32 lIndex = 0;
		AgpdGuild** ppcsGuild = (AgpdGuild**)pcsSiegeWar->m_csDefenseGuild.GetObjectSequence(&lIndex);
		while(ppcsGuild && *ppcsGuild)
		{
			AuAutoLock csLock2((*ppcsGuild)->m_Mutex);
			if (csLock2.Result())
				m_pagpmGuild->GetMemberCIDList(*ppcsGuild, &csTargetList);

			ppcsGuild = (AgpdGuild**)pcsSiegeWar->m_csDefenseGuild.GetObjectSequence(&lIndex);
		}

		// 오브젝트 몬스터들도 넣어야 하나??
		//
		//
	}

	if(csTargetList.empty())
		return FALSE;

	ApSafeArray<INT32, AGSMSKILL_MAX_TARGET_NUM>	lCIDBuffer2;
	lCIDBuffer2.MemCopy(0, lCIDBuffer.begin(), AGSMSKILL_MAX_TARGET_NUM);

	// 한정 범위 스킬은 구별해서 해야한다.
	if(m_pagpmSkill->IsSphereRangeSiegeWarSkill(pcsSkillTemplate))
	{
		// 겹치는 것만 남긴다.
		std::sort(lCIDBuffer2.begin(), lCIDBuffer2.end());
		csTargetList.sort();

		std::list<INT32> csResultList;
		std::set_intersection(lCIDBuffer2.begin(), lCIDBuffer2.end(), csTargetList.begin(), csTargetList.end(),
							back_inserter(csResultList), less<INT32>());

		csTargetList.clear();
		std::copy(csResultList.begin(), csResultList.end(), back_inserter(csTargetList));
	}

	// 루프 돌면서 각각 ApplySkillFinal 불러준다.
	std::list<INT32>::iterator iter = csTargetList.begin();
	while(iter != csTargetList.end())
	{
		// 자기 자신은 넘어간다.
		if(*iter == pcsSkill->m_pcsBase->m_lID)
		{
			++iter;
			continue;
		}

		AgpdCharacter* pcsNewTarget = m_pagpmCharacter->GetCharacter(*iter);
		if(!pcsNewTarget)
		{
			++iter;
			continue;
		}

		// 대략적인 상태를 체크한다.
		if(!pcsNewTarget->m_bIsActiveData ||
			!pcsNewTarget->m_bIsAddMap ||
			pcsNewTarget->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		{
			++iter;
			continue;
		}

		lCIDBuffer2.MemSetAll();
		lCIDBuffer2[0] = *iter;

		// 타겟은 하나다.
		if(!SendCastSkill(pcsSkill, AGPMSKILL_ACTION_CAST_SKILL, &lCIDBuffer2[0], 1 * sizeof(INT32)))
		{
			++iter;
			continue;
		}


		ApplySkillFinal(pcsSkill, NULL, lCIDBuffer2, 1, FALSE);	// 타겟은 하나다.

		++iter;
	}

	return TRUE;
}

// 2005.01.25. steeple
// Charge 를 계산해준다.
BOOL AgsmSkill::ProcessChargeAttack(pstAgsmCombatAttackResult pstCombatResult)
{
	if(!pstCombatResult)
		return FALSE;

	if(!pstCombatResult->pAttackChar)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pstCombatResult->pAttackChar);
	if(!pcsAttachData)
		return FALSE;

	// Charge 가 버프중이지 않다면 바로 나간다.
	if(!(pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_CHARGE))
		return TRUE;

	BOOL bApplyCharge = FALSE;
	BOOL bAddChargeLevel = FALSE;

	// Charge 단계 계산하기
	if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lChargeLevel == AGPMSKILL_CHARGE_NONE)
	{	// Charge 버프 상태에서의 첫타!
		if(pstCombatResult->nDamage > 0)
			bAddChargeLevel = TRUE;
	}
	else	
	{
		if(pstCombatResult->nDamage > 0)
		{
			// 이때 Charge 가 MAX Level 이거나 끊긴 상황이면 폭발~
			if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lChargeLevel >= AGPMSKILL_CHARGE_3 ||
				pcsAttachData->m_lMeleeComboInCharge == 0)
				bApplyCharge = TRUE;
			else
				bAddChargeLevel = TRUE;
		}
		else
		{
			pcsAttachData->m_lMeleeComboInCharge = 0;
		}
	}

	// Buffed Skill 중에서 이 스킬을 찾아낸다.
	AgsdSkillADBase* pcsAgsdSkillADBase = GetADBase((ApBase*)pstCombatResult->pAttackChar);
	if(!pcsAgsdSkillADBase)
		return FALSE;

	int nBuffedIndex = 0;
	INT32 lSkillLevel = 0, lSkillTID = 0;
	BOOL bFound = FALSE;

	AgpdSkill* pcsSkill = NULL;
	AgsdSkill* pcsBuffedAgsdSkill = NULL;

	for(nBuffedIndex; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; nBuffedIndex++)
	{
		if(pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
			!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
			break;

		pcsSkill = pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill;
		pcsBuffedAgsdSkill = GetADSkill(pcsSkill);
		if(!pcsBuffedAgsdSkill)
			break;

		// Agsd에 버프된지 확인 여부는 lChargeDamage 로 한다.
		if(pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lChargeDamage != 0)
		{
			pstCombatResult->lSkillTID = pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
			lSkillTID = pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
			lSkillLevel = GetModifiedSkillLevel(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);
			if(!lSkillLevel)
				continue;

			SendUpdateBuffedList(pstCombatResult->pAttackChar, lSkillTID, pstCombatResult->pAttackChar->m_lID, TRUE,
								pcsSkill->m_ulEndTime - GetClockCount(), (UINT8)pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lChargeLevel,
								m_pagsmCharacter->GetCharDPNID(pstCombatResult->pAttackChar));
			bFound = TRUE;
			break;
		}
	}

	if(!bFound)
		return TRUE;

	// 차지 데미지를 폭발시킨다.
	if(bApplyCharge)
	{
		AgpdSkillTemplate* pcsSkillTemplate = m_pagpmSkill->GetSkillTemplate(pstCombatResult->lSkillTID);
		if(pcsSkillTemplate)
		{
			INT32 lChargeDamageIndex = 0;
			switch(pcsAttachData->m_stBuffedSkillCombatEffectArg.lChargeLevel)
			{
				case AGPMSKILL_CHARGE_1 :lChargeDamageIndex = AGPMSKILL_CONST_CHARGE_1; break;
				case AGPMSKILL_CHARGE_2 :lChargeDamageIndex = AGPMSKILL_CONST_CHARGE_2; break;
				case AGPMSKILL_CHARGE_3 :lChargeDamageIndex = AGPMSKILL_CONST_CHARGE_3; break;
				default: lChargeDamageIndex = 0; break;
			}
			pstCombatResult->nDamage += (INT32)pcsSkillTemplate->m_fUsedConstFactor[lChargeDamageIndex][lSkillLevel];
		}

		//pstCombatResult->ulCheckResult |= AGSMCOMBAT_CHECKRESULT_CHARGE;

		// Charge Buffed Skill 을 없앤다. 2005.01.23. steeple
		EndBuffedSkill((ApBase*)pstCombatResult->pAttackChar, nBuffedIndex, TRUE, _T("ProcessChareAttack - 1"));
	}
	// 차지 데미지를 폭발시키지 않고 차지 레벨을 올려준다.
	else if(bAddChargeLevel)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lChargeLevel++;
		pcsAttachData->m_lMeleeComboInCharge++;

		pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lChargeLevel = pcsAttachData->m_stBuffedSkillCombatEffectArg.lChargeLevel;
	}

	return TRUE;
}

// 2005.07.07. steeple
// ATField Attack 세부 내용을 처리해준다.
BOOL AgsmSkill::ProcessATFieldAttack(AgpdSkill* pcsSkill, AgpdCharacter* pcsAttacker, AgpdCharacter* pcsTarget)
{
	if(!pcsSkill || !pcsAttacker || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	BOOL bAttackable = FALSE;

	// 공격할 수 있는 대상인지 확인한 후 공격
	if(m_pagpmPvP->IsAttackable(pcsAttacker, pcsTarget, FALSE))
		bAttackable = TRUE;

	// 2005.10.06. steeple. 투명은 공격 불가
	if(m_pagpmCharacter->IsStatusFullTransparent(pcsTarget))
		bAttackable = FALSE;

	if(!bAttackable)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	INT32 lAttackLevel = m_pagpmCharacter->GetLevel(pcsAttacker);
	INT32 lTargetLevel = m_pagpmCharacter->GetLevel(pcsTarget);

	// 2005.09.20. steeple
	// Skill Rate 를 한번 체크해준다.
	if(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel] != 0.0f &&
		pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel] != 100.0f)
	{
		INT16 nRandomNumber = m_csRandom.randInt(100);
		if(nRandomNumber > pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel])
			return FALSE;
	}

	// 아래 공식은 DOT Damage 와 비슷하게 처리.
	// 각 Const 에서 Damage 를 더해온다.
	INT32 lDamage = 0;

	BOOL bIsPoisonDamage = FALSE;

	AgpdFactorAttribute	stAttrDamage;
	ZeroMemory(&stAttrDamage, sizeof(AgpdFactorAttribute));

	INT32 lSpiritConstIndex = 0;
	// 여기에서 Poison 값이 0보다 크면 bIsPoisonDamage를 TRUE로 세팅해줘야 한다.
	for(INT32 i = AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL; i <= AGPD_FACTORS_ATTRIBUTE_TYPE_ICE; ++i)
	{
		lSpiritConstIndex = GetSpiritEAConstIndexByFactor((AgpdFactorAttributeType)i);
		if(lSpiritConstIndex < AGPMSKILL_CONST_EA_DMG_PHYSICAL || 
			lSpiritConstIndex > AGPMSKILL_CONST_EA_DMG_THUNDER)
			continue;

		stAttrDamage.lValue[i] = (INT32)pcsSkillTemplate->m_fUsedConstFactor[lSpiritConstIndex][lSkillLevel];

		if (i == AGPD_FACTORS_ATTRIBUTE_TYPE_POISON &&
			pcsSkillTemplate->m_fUsedConstFactor[lSpiritConstIndex][lSkillLevel] > 0)
			bIsPoisonDamage	= TRUE;
	}

	// 2007.04.05. steeple
	// AT Field Damage 에도 레벨 공식 적용
	INT32 lOldDamage, lNewDamage;
	for(INT32 i = AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL; i <= AGPD_FACTORS_ATTRIBUTE_TYPE_ICE; ++i)
	{
		lOldDamage = stAttrDamage.lValue[i];
		lNewDamage = 0;

		if(m_pagpmCharacter->IsPC(pcsAttacker))
		{
			// PvP 에서는 데미지 덜 감소한다. 2005.08.02. steeple
			if(m_pagpmCharacter->IsPC(pcsTarget))
			{
				lNewDamage = (INT32) ( (lOldDamage)
							* ((100 - ((lAttackLevel - lTargetLevel > 0) ? 0 : (abs(lAttackLevel - lTargetLevel) * 1))) / 100.0) );
			}
			else
			{
				lNewDamage = (INT32) ( (lOldDamage)
							* ((100 - ((lAttackLevel - lTargetLevel > 0) ? 0 : (abs(lAttackLevel - lTargetLevel) * 5))) / 100.0) );
			}
		}
		else
		{
			lNewDamage = (INT32) ( (lOldDamage)
						* ((100 - ((lAttackLevel - lTargetLevel > 0) ? 0 : (abs(lAttackLevel - lTargetLevel) * 1))) / 100.0) );
		}
		
		if(lOldDamage > 0 && lNewDamage < 1)
			lNewDamage = 1;

		stAttrDamage.lValue[i] = lNewDamage;
		lDamage += lNewDamage;
	}

	// 다른 모듈에서 Poison과 관련된 적용해야할 값이 있는지 가져온다.
	if(bIsPoisonDamage)
	{
		UINT64	ulSpecialStatus		= (UINT64)	AGPDCHAR_SPECIAL_STATUS_SLOW;
		UINT32	ulAdjustDuration	= 0;
		INT32	lAdjustProbability	= 0;

		PVOID	pvBuffer[2];
		pvBuffer[0]	= (PVOID)	&lAdjustProbability;
		pvBuffer[1]	= (PVOID)	&ulAdjustDuration;

		EnumCallback(AGSMSKILL_CB_CHECK_POISON_STATUS_FACTOR, pcsTarget, pvBuffer);

		if(ulAdjustDuration > 0)
		{
			pcsSkill->m_ulEndTime -= ulAdjustDuration;
		}
	}

	if(lDamage < 1)
		return TRUE;

	// Damage를 반영한다.
	// 실제 캐릭터에 반영하고 History에도 반영한다.
	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);

	if (!SetUpdateFactorResult(pcsAgsdSkill))
		return FALSE;

	AgpdFactor* pcsFactorResult = (AgpdFactor*)m_pagpmFactors->SetFactor(pcsAgsdSkill->m_pcsUpdateFactorResult, NULL, AGPD_FACTORS_TYPE_RESULT);
	if(!pcsFactorResult)
		return FALSE;

	AgpdFactorCharPoint*pcsFactorCharPoint = (AgpdFactorCharPoint*)m_pagpmFactors->GetFactor(pcsFactorResult, AGPD_FACTORS_TYPE_CHAR_POINT);
	if(!pcsFactorCharPoint)
	{
		pcsFactorCharPoint = (AgpdFactorCharPoint*)m_pagpmFactors->InitCalcFactor(pcsFactorResult, AGPD_FACTORS_TYPE_CHAR_POINT);
		if(!pcsFactorCharPoint)
			return FALSE;
	}

	pcsFactorCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] += (-lDamage);
	for(INT32 i = AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL; i <= AGPD_FACTORS_ATTRIBUTE_TYPE_ICE; i++)
		pcsFactorCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_DMG_NORMAL + i] += (-stAttrDamage.lValue[i]);

	// 데미지를 다 구했으니 적용시킨다.
	UpdateSkillFactor(pcsSkill, pcsTarget, TRUE);

	// Attack 후 Dead 체크한다.
	stAgsmCombatAttackResult stAttackResult;

	stAttackResult.pAttackChar = pcsAttacker;
	stAttackResult.pTargetChar = pcsTarget;
	stAttackResult.nDamage = 0;

	EnumCallback(AGSMSKILL_CB_ATTACK, &stAttackResult, pcsSkill);

	return TRUE;
}

// 2005.07.18. steeple
// Action On Action Type3 의 실제 처리 부분.
BOOL AgsmSkill::ProcessActionOnActionType3(pstAgsmCombatAttackResult pstCombatResult)
{
	if(!pstCombatResult || !pstCombatResult->pAttackChar || !pstCombatResult->pTargetChar)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pstCombatResult->pTargetChar);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pstCombatResult->pTargetChar);
	if(!pcsADBase)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = NULL;

	// 타겟에게 버프되어 있는 스킬중에서 Action On Action Type3 인 걸 찾는다.
	for(INT32 i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; i++)
	{
		if(pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			break;

		pcsSkillTemplate = (AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate;
		if(!pcsSkillTemplate)
			continue;

		if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE3 ||
			pcsSkillTemplate->m_nProcessIntervalEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE3)
		{
			INT32 lSkillLevel = GetModifiedSkillLevel(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);
			if(!lSkillLevel)
				continue;

			INT32 lReflectSkillTID = pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel);

			CastSkill(static_cast<ApBase*>(pstCombatResult->pTargetChar),
						lReflectSkillTID,
						lSkillLevel,
						static_cast<ApBase*>(pstCombatResult->pAttackChar));
		}
	}

	return TRUE;
}

// 2007.07.10. steeple
// Action on Action Type6 의 실제 처리 부분.
BOOL AgsmSkill::ProcessActionOnActionType6(AgpdCharacter* pcsCharacter)
{
	// 전체 스킬 다 돌면서 Type6 인 녀석을 캐스팅 한다.
	if(!pcsCharacter)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsCharacter);
	if(!pcsADBase)
		return FALSE;

	AgpdSkill* pcsSkill = NULL;
	AgpdSkillTemplate* pcsSkillTemplate = NULL;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			break;

		pcsSkill = pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill;
		pcsSkillTemplate = static_cast<AgpdSkillTemplate*>(pcsSkill->m_pcsTemplate);
		if(!pcsSkillTemplate)
			break;

		if(m_pagpmSkill->GetActionOnActionType(pcsSkillTemplate) != 6)
			continue;

		INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			continue;

		INT32 lAdditionalSkillTID = (INT32)pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel, m_csRandom.randInt(100));
		
		AgpdCharacter* pcsMostDamager = NULL;
		AgsdCharacterHistoryEntry* pcsEntry = m_pagsmCharacter->GetMostDamager(pcsCharacter);
		if(pcsEntry)
		{
			// 캐릭터라면
			if(pcsEntry->m_csSource.m_eType == APBASE_TYPE_CHARACTER)
			{
				pcsMostDamager = m_pagpmCharacter->GetCharacter(pcsEntry->m_csSource.m_lID);
			}
			else if(pcsEntry->m_csSource.m_eType == APBASE_TYPE_PARTY)
			{
				pcsEntry = m_pagsmCharacter->GetMostDamagerInParty(pcsCharacter, pcsEntry->m_csSource.m_lID);
				if(pcsEntry && pcsEntry->m_csSource.m_eType == APBASE_TYPE_CHARACTER)
					pcsMostDamager = m_pagpmCharacter->GetCharacter(pcsEntry->m_csSource.m_lID);
			}
		}

		pcsSkillTemplate = m_pagpmSkill->GetSkillTemplate(lAdditionalSkillTID);

		if(m_pagpmSkill->IsUnlimitedTargetRaceSkill(pcsSkillTemplate))
		{
			AgsdCastSkill pCastSkill;
			pCastSkill.pcsBase			= static_cast<ApBase*>(pcsCharacter);
			pCastSkill.lSkillTID		= lAdditionalSkillTID;
			pCastSkill.lSkillLevel		= lSkillLevel;
			pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsMostDamager);
			pCastSkill.m_pSaveSkillData.m_pSaveData.Attribute	= AGPMSKILL_RANGE2_TARGET_RACE;

			CastSkillUnlimitedTarget(pCastSkill);
		}
		else if(m_pagpmSkill->IsPartySkill(pcsSkillTemplate))
		{
			AgsdCastSkill pCastSkill;
			pCastSkill.pcsBase			= static_cast<ApBase*>(pcsCharacter);
			pCastSkill.lSkillTID		= lAdditionalSkillTID;
			pCastSkill.lSkillLevel		= lSkillLevel;
			pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsMostDamager);
			pCastSkill.m_pSaveSkillData.m_pSaveData.Attribute	= AGPMSKILL_RANGE2_TARGET_PARTY;

			CastPartySkill(pCastSkill);
		}
		else if(m_pagpmSkill->IsInvolveParentRegionSkill(pcsSkillTemplate))
		{
			AgsdCastSkill pCastSkill;
			pCastSkill.pcsBase			= static_cast<ApBase*>(pcsCharacter);
			pCastSkill.lSkillTID		= lAdditionalSkillTID;
			pCastSkill.lSkillLevel		= lSkillLevel;
			pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsMostDamager);
			pCastSkill.m_pSaveSkillData.m_pSaveData.Attribute  = AGPMSKILL_RANGE2_TARGET_REGION_INVOLVE_PARENT;

			CastInvolveParentRegionSkill(pCastSkill);
		}
		else
		{
			INT32 lSkillTID = (INT32)pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel, m_csRandom.randInt(100));
			if(lSkillTID)
			{
				AgsdCastSkill pCastSkill;
				pCastSkill.pcsBase			= static_cast<ApBase*>(pcsMostDamager);
				pCastSkill.lSkillTID		= lSkillTID;
				pCastSkill.lSkillLevel		= lSkillLevel;
				pCastSkill.pcsTargetBase	= static_cast<ApBase*>(pcsMostDamager);
				
				CastSkill(pCastSkill);
			}
			else
			{
				CastSkill(static_cast<ApBase*>(pcsCharacter), lAdditionalSkillTID, lSkillLevel, static_cast<ApBase*>(pcsMostDamager));
			}
		}
	}

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsCharacter);
	if(!pcsAttachData)
		return FALSE;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if(pcsAttachData->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
			break;

		pcsSkill = m_pagpmSkill->GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
		if(!pcsSkill)
			pcsSkill = pcsAttachData->m_apcsUsePassiveSkill[i];

		if(!pcsSkill)
			continue;
		
		pcsSkillTemplate = static_cast<AgpdSkillTemplate*>(pcsSkill->m_pcsTemplate);
		if(!pcsSkillTemplate)
			continue;

		if(m_pagpmSkill->GetActionOnActionType(pcsSkillTemplate) != 6)
			continue;
			
		INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			continue;

		INT32 lAdditionalSkillTID = (INT32)pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel, m_csRandom.randInt(100));

		AgpdCharacter* pcsMostDamager = NULL;
		AgsdCharacterHistoryEntry* pcsEntry = m_pagsmCharacter->GetMostDamager(pcsCharacter);
		if(pcsEntry)
		{
			// 캐릭터라면
			if(pcsEntry->m_csSource.m_eType == APBASE_TYPE_CHARACTER)
			{
				pcsMostDamager = m_pagpmCharacter->GetCharacter(pcsEntry->m_csSource.m_lID);
			}
			else if(pcsEntry->m_csSource.m_eType == APBASE_TYPE_PARTY)
			{
				pcsEntry = m_pagsmCharacter->GetMostDamagerInParty(pcsCharacter, pcsEntry->m_csSource.m_lID);
				if(pcsEntry && pcsEntry->m_csSource.m_eType == APBASE_TYPE_CHARACTER)
					pcsMostDamager = m_pagpmCharacter->GetCharacter(pcsEntry->m_csSource.m_lID);
			}
		}

		CastSkill(static_cast<ApBase*>(pcsCharacter), lAdditionalSkillTID, lSkillLevel, static_cast<ApBase*>(pcsMostDamager));
	}

	return TRUE;
}

// 2005.07.15. steeple
// Action Passive Skill 의 실제 처리 부분.
BOOL AgsmSkill::ProcessActionPassiveAttack(pstAgsmCombatAttackResult pstCombatResult)
{
	if(!pstCombatResult || !pstCombatResult->pAttackChar || !pstCombatResult->pTargetChar)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pstCombatResult->pAttackChar);
	if(!pcsAttachData)
		return FALSE;

	// Attacker 의 Passive List 를 돌면서 체크해준다.
	for(INT32 i = 0; i < AGPMSKILL_MAX_SKILL_USE; i++)
	{
		if(pcsAttachData->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill* pcsSkill = m_pagpmSkill->GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
		if(!pcsSkill)
			pcsSkill = pcsAttachData->m_apcsUsePassiveSkill[i];

		if(!pcsSkill)
			continue;

		AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
		if(!pcsSkillTemplate)
			continue;

		if(!m_pagpmSkill->IsActionPassiveSkill(pcsSkill))
			continue;

		INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			continue;

        // 확률 검사.
		if(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel] != 0 &&
			pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel] != 100)
		{
			INT16 nRandomNumber = m_csRandom.randInt(100);
			if(nRandomNumber > pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel])
				continue;
		}

		// 실제 효과를 담당할 Skill 을 얻는다.
		INT32 lSkillTID = pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel);

		// 타겟에게 스킬 걸어준다.
		// 소환수가 쓰는 거라면 Monster Use 로 해준다.
		CastSkill(static_cast<ApBase*>(pstCombatResult->pAttackChar), lSkillTID, lSkillLevel, 
					static_cast<ApBase*>(pstCombatResult->pTargetChar));
	}

	return TRUE;
}

// 2007.06.27. steeple
// Divide Damage 처리 해준다.
BOOL AgsmSkill::ProcessDivideDamage(pstAgsmCombatAttackResult pstCombatResult)
{
	if(!pstCombatResult || !pstCombatResult->pAttackChar || !pstCombatResult->pTargetChar)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pstCombatResult->pTargetChar);
	if(!pcsAttachData)
		return FALSE;

	// Divide 걸려있지 않다.
	if(!(pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_DIVIDE))
		return TRUE;

	pstCombatResult->ulCheckResult |= AGSMCOMBAT_CHECKRESULT_DIVIDE;

	// 데이터를 세팅해주고, 처리는 AgsmCombat 에서 하게끔 한다.
	pstCombatResult->lDivideAttrRate = pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideAttrRate;
	pstCombatResult->lDivideAttrCaster = pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideAttrCaster;
	pstCombatResult->lDivideNormalRate = pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideNormalRate;
	pstCombatResult->lDivideNormalCaster = pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideNormalCaster;

	return TRUE;
}

// 2007.06.27. steeple
// Divide Damage (Skill) 처리해준다.
BOOL AgsmSkill::ProcessDivideDamage(AgpdSkill* pcsSkill, AgpdCharacter* pcsTarget, AgpdFactorAttribute* pstAttrDamage, INT32* plDamage, INT32* plSpiritDamage)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsTarget);
	if(!pcsAttachData)
		return FALSE;

	// Divide 걸려있지 않다.
	if(!(pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_DIVIDE))
		return TRUE;

	// 일반데미지
	if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideNormalRate && pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideNormalCaster)
	{
		INT32 lDamage = 0;
		if(plDamage)
			lDamage = *plDamage;

		// Caster 가 있어야만 작업한다. 또한 자기 자신은 아니여야 한다.
		AgpdCharacter* pcsCaster = m_pagpmCharacter->GetCharacter(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideNormalCaster);
		if(lDamage > 0 && pcsCaster && !pcsCaster->m_bIsReadyRemove && pcsCaster->m_lID != pcsTarget->m_lID)
		{
			AuAutoLock csLock(pcsCaster->m_Mutex);

			INT32 lReduceDamage = (INT32)((double)lDamage * (double)pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideNormalRate / (double)100);

			if(lReduceDamage > 0)
			{
				if(plDamage)
					*plDamage = *plDamage - lReduceDamage;

				// 줄인 만큼의 데미지는 Caster 에게 간다.
				m_pagsmCharacter->DecreaseHP(pcsCaster, lReduceDamage);

				// 추가효과 준다.
				m_pagsmCharacter->SendPacketCharUseEffect(pcsCaster, AGPDCHAR_ADDITIONAL_EFFECT_ABSORB_HP);			
			}

			//m_pagsmCombat->ApplyAttackDamage((AgpdCharacter*)pcsSkill->m_pcsBase, pcsCaster, lReduceDamage, lReduceDamage, NULL, AGPDCHAR_ACTION_RESULT_TYPE_ATTACK_SUCCESS, 0, TRUE/*force*/, 0);
		}
	}

	// 속성데미지
	if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideAttrRate && pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideAttrCaster)
	{
		// Caster 가 있어야만 작업한다. 또한 자기 자신은 아니여야 한다.
		AgpdCharacter* pcsCaster = m_pagpmCharacter->GetCharacter(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideAttrCaster);
		if(pstAttrDamage && pcsCaster && !pcsCaster->m_bIsReadyRemove && pcsCaster->m_lID != pcsTarget->m_lID)
		{
			AuAutoLock csLock(pcsCaster->m_Mutex);

			INT32 lReduceDamage = 0;
			INT32 lReduceDamageSum = 0;

			for(int i = AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL; i <= AGPD_FACTORS_ATTRIBUTE_TYPE_ICE; ++i)
			{
				lReduceDamage = 0;
				lReduceDamage = (INT32)((double)pstAttrDamage->lValue[i] *
										(double)pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideAttrRate / (double)100);
				lReduceDamageSum += lReduceDamage;

				pstAttrDamage->lValue[i] -= lReduceDamage;
			}

			if(lReduceDamageSum > 0)
			{
				if(plSpiritDamage)
					*plSpiritDamage = *plSpiritDamage - lReduceDamageSum;

				// 줄인 만큼의 데미지는 Caster 에게 간다.
				m_pagsmCharacter->DecreaseHP(pcsCaster, lReduceDamageSum);

				// 추가효과 준다.
				m_pagsmCharacter->SendPacketCharUseEffect(pcsCaster, AGPDCHAR_ADDITIONAL_EFFECT_ABSORB_HP);			
			}

			// 줄인 만큼의 데미지는 Caster 에게 간다.
			//m_pagsmCombat->ApplyAttackDamage((AgpdCharacter*)pcsSkill->m_pcsBase, pcsCaster, lReduceDamageSum, lReduceDamageSum, NULL, AGPDCHAR_ACTION_RESULT_TYPE_ATTACK_SUCCESS, 0, TRUE/*force*/, 0);
		}
	}

	return TRUE;
}

BOOL AgsmSkill::IsMaxBuffedSkill(ApBase *pcsBase, AgpdSkill *pcsSkill)
{
	if (!pcsBase || !pcsSkill)
		return TRUE;

	if (pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return FALSE;

	// Buff, DeBuff 스킬이 아니면 넣지 않는다.
	if(!(((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_nAttribute & AGPMSKILL_SKILL_ATTR_BUFF) &&
		!(((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_nAttribute & AGPMSKILL_SKILL_ATTR_DEBUFF))
		return FALSE;

	if(IsSetAttackDamageOnly(pcsSkill))
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsBase);
	if(!pcsAttachData)
		return FALSE;

	if (pcsAttachData->m_astBuffSkillList[AGPMSKILL_MAX_SKILL_BUFF - 1].lSkillTID == AP_INVALID_SKILLID)
		return FALSE;

	return TRUE;
}

// 2006.01.23. steeple
// return 값을 조사하게 변경하였다.
// return FALSE 가 나면, BuffedList 에 넣지 못한 것이므로, UpdateFactor 를 하면 안된다.
// 2005.01.23. steeple
// Buff 스킬을 Attached Base 에 넣어주고 패킷 보낸다.
BOOL AgsmSkill::AddBuffedList(AgpdSkill* pcsSkill, ApBase* pcsTargetBase)
{
	if(!pcsSkill || !pcsTargetBase)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsTargetBase);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsTargetBase);
	if(!pcsADBase)
		return FALSE;

	// Buff, DeBuff 스킬이 아니면 넣지 않는다.
	if(!(pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_BUFF) &&
		!(pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_DEBUFF))
		return TRUE;	// 얘는 TRUE 로 리턴

	// 2006.01.17. steeple
	// Attack Damage 만 주라고 세팅해놓은 상태면 넣지 않는다.
	if(IsSetAttackDamageOnly(pcsSkill))
		return TRUE;	// 얘는 TRUE 로 리턴

	if (0==m_pagpmSkill->GetActionOnActionType(pcsSkillTemplate) && IsSetDamageOnlyTemporary(pcsSkill))
	{
		AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
		if (!pcsAgsdSkill)
			return FALSE;

		pcsAgsdSkill->m_bDamageOnlyTemporary = FALSE;
		return TRUE;
	}

	INT32 lSpellCount = GetSpellCount(pcsSkill);
	INT32 lBuffedListIndex = 0;

	INT32 lCasterTID = 0;
	if(pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
		lCasterTID = ((AgpdCharacter*)pcsSkill->m_pcsBase)->m_pcsCharacterTemplate->m_lID;

	lBuffedListIndex = m_pagpmSkill->AddBuffedList(&pcsAttachData->m_astBuffSkillList[0], pcsSkillTemplate->m_lID, pcsSkill->m_lID, pcsSkill->m_pcsBase->m_eType, pcsSkill->m_pcsBase->m_lID, lCasterTID, lSpellCount, pcsSkill->m_ulEndTime - pcsSkill->m_ulStartTime, pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lChargeLevel);
	if(lBuffedListIndex < 0)
		return FALSE;

	if(SetSkillProcessInfo(pcsSkill, &pcsADBase->m_csBuffedSkillProcessInfo[lBuffedListIndex]) == FALSE)
	{
		// 일루 들어오면 실패한 거다.

		if(&pcsADBase->m_csBuffedSkillProcessInfo[lBuffedListIndex] == NULL)
		{
			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "AddBuffedList::pcsSkillProcessInfo is NULL, lIndex : %d, SkillTID : %d, CasterCID : %d, CasterTID : %d, TargetCID : %d, TargetTID : %d\n",
				lBuffedListIndex,
				pcsSkillTemplate->m_lID,
				pcsSkill->m_pcsBase->m_lID,
				lCasterTID,
				pcsTargetBase->m_lID,
				((AgpdCharacter*)pcsTargetBase)->m_pcsCharacterTemplate->m_lID);
			AuLogFile_s((CHAR*)BUFFED_SKILL_ERROR_FILE_NAME, strCharBuff);
		}
		else if(pcsADBase->m_csBuffedSkillProcessInfo[lBuffedListIndex].m_pcsTempSkill == NULL)
		{
			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "AddBuffedList::m_pcsTempSkill is NULL, lIndex : %d, SkillTID : %d, CasterCID : %d, CasterTID : %d, TargetCID : %d, TargetTID : %d\n",
				lBuffedListIndex,
				pcsSkillTemplate->m_lID,
				pcsSkill->m_pcsBase->m_lID,
				lCasterTID,
				pcsTargetBase->m_lID,
				((AgpdCharacter*)pcsTargetBase)->m_pcsCharacterTemplate->m_lID);
			AuLogFile_s((CHAR*)BUFFED_SKILL_ERROR_FILE_NAME, strCharBuff);
		}

		// 뺀다.
		m_pagpmSkill->RemoveBuffedListByIndex(&pcsAttachData->m_astBuffSkillList[0], lBuffedListIndex);
		return FALSE;
	}

	// skill save - arycoat 2008.7
	if( pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_SAVE_EXPIREDATE ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_SAVE_DURATION)
	{
		if(pcsSkill->m_pSaveSkillData.eStep == SAVESKILL_INSERT)
			ProcessSkillSave(pcsSkill, (AgpdCharacter *)pcsTargetBase, AGSMDATABASE_OPERATION_INSERT);
	}

	// 여기까지 왔으면 버프에 넣은 것.
	SendUpdateBuffedList(pcsTargetBase, pcsSkillTemplate->m_lID, lCasterTID, TRUE, pcsSkill->m_ulEndTime - pcsSkill->m_ulStartTime, (UINT8)pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lChargeLevel, m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsTargetBase), 
						(pcsSkill->m_pSaveSkillData.m_pSaveData.ExpireDate > 0) ? (UINT32)pcsSkill->m_pSaveSkillData.m_pSaveData.ExpireDate.GetTime() : 0);
	m_pagsmCharacter->SetIdleInterval((AgpdCharacter*)pcsTargetBase, AGSDCHAR_IDLE_TYPE_SKILL, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);

	return TRUE;
}

// 2005.01.26. steeple
// Classify 가 있는 스킬을 사용할 때 버프중에서 같은 Classify 스킬을 찾아봐서
// 쓰려는 레벨이 버프에 있는 놈보다 낮으면 쓰면 안된다.
BOOL AgsmSkill::IsEnableSkillByBuffedList(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsTarget)
		return FALSE;

	// 버프된 스킬들중 pcsSkill과 템플릿이 같은 넘이 있는지 검사한다.
	AgsdSkillADBase* pcsADBase = GetADBase(pcsTarget);
	if(!pcsADBase)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;

	INT32 lBuffedSkillLevel = 0;
	AgpdSkillTemplate* pcsBuffedSkillTemplate = NULL;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate)
			break;

		lBuffedSkillLevel = GetModifiedSkillLevel(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);
		if(!lBuffedSkillLevel)
			continue;

		pcsBuffedSkillTemplate = (AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate;

		//  같은 Classify 찾기
		if (pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lBuffedSkillLevel] != 0 &&
			pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lSkillLevel] != 0 &&
			pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lBuffedSkillLevel] ==
			pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lSkillLevel] &&
			pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_LEVEL][lBuffedSkillLevel] >
			pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_LEVEL][lSkillLevel]
			)
		{ 
			// 찾았는데 버프 되어 있는 놈이 레벨이 더 높다.

			// 2006.01.17. steeple
			// 만약에 Attack Damage 를 주는 스킬이라면, 데미지만 주는 스킬로 세팅해놓는다.
			if(m_pagpmSkill->IsAttackSkill(pcsSkillTemplate))
			{
				SetAttackDamageOnly(pcsSkill);
				return TRUE;
			}

			return FALSE;
		}
	}

	return TRUE;
}

// 2006.01.05. steeple
// SkillScroll 전용이다.
BOOL AgsmSkill::IsEnableSkillByBuffedList(AgpdItem* pcsItem, ApBase* pcsTarget)
{
	if(!pcsItem || !pcsTarget)
		return FALSE;

	// 버프된 스킬들중 pcsSkill과 템플릿이 같은 넘이 있는지 검사한다.
	AgsdSkillADBase* pcsADBase = GetADBase(pcsTarget);
	if(!pcsADBase)
		return FALSE;

	// 스킬 스크롤이 아니면 나간다.
	if(pcsItem->m_pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE ||
		((AgpdItemTemplateUsable*)pcsItem->m_pcsItemTemplate)->m_nUsableItemType != AGPMITEM_USABLE_TYPE_SKILL_SCROLL)
		return TRUE;

	AgpdSkillTemplate* pcsSkillTemplate = m_pagpmSkill->GetSkillTemplate(((AgpdItemTemplateUsableSkillScroll*)pcsItem->m_pcsItemTemplate)->m_lSkillTID);
	if(!pcsSkillTemplate)
		return FALSE;

	INT32 lSkillLevel = ((AgpdItemTemplateUsableSkillScroll*)pcsItem->m_pcsItemTemplate)->m_lSkillLevel;

	INT32 lBuffedSkillLevel = 0;
	AgpdSkillTemplate* pcsBuffedSkillTemplate = NULL;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate)
			break;

		lBuffedSkillLevel = GetModifiedSkillLevel(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);
		if(!lBuffedSkillLevel)
			continue;

		pcsBuffedSkillTemplate = (AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate;

		// 같은 스킬 찾기
		if(pcsSkillTemplate->m_lID == pcsBuffedSkillTemplate->m_lID && lBuffedSkillLevel > lSkillLevel)
		{
			// 2006.01.17. steeple
			// 만약에 Attack Damage 를 주는 스킬이라면, 여기서는 return TRUE 해준다. 일반적인 스킬의 처리와는 다르다. 
			if(m_pagpmSkill->IsAttackSkill(pcsSkillTemplate))
				return TRUE;

			return FALSE;
		}

		//  같은 Classify 찾기
		if (pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lBuffedSkillLevel] != 0 &&
			pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lSkillLevel] != 0 &&
			pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lBuffedSkillLevel] ==
			pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lSkillLevel] &&
			pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_LEVEL][lBuffedSkillLevel] >
			pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_LEVEL][lSkillLevel]
			)
		{
			// 찾았는데 버프 되어 있는 놈이 레벨이 더 높다.

			// 2006.01.17. steeple
			// 만약에 Attack Damage 를 주는 스킬이라면, 여기서는 return TRUE 해준다.  일반적인 스킬의 처리와는 다르다.
			if(m_pagpmSkill->IsAttackSkill(pcsSkillTemplate))
				return TRUE;

			return FALSE;
		}
	}

	return TRUE;
}

// 2004.11.08. steeple
// 캐스트 할때와 실제 공격시점이 다른 스킬의 사용이다.
// 해당 스킬이 패킷을 두번 보내면서 공격하는 스킬인지 파악한다.
BOOL AgsmSkill::IsTwicePacketSkill(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	return pcsSkillTemplate->m_bTwicePacket;
}

// 2004.11.08. steeple
// 캐스트 할때와 실제 공격시점이 다른 스킬의 사용이다.
// 해당 스킬이 캐스팅 된 후 세팅이 되어있는지 확인한다.
BOOL AgsmSkill::IsSetTwicePacketSkill(AgpdSkill* pcsSkill)
{
	if(!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	if(!IsTwicePacketSkill(pcsSkill))
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsSkill->m_pcsBase);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsSkill)
		return FALSE;

	// 세팅이 안되어 있다면 return FALSE
	//if(pcsAgsdSkill->m_ulLastCastTwiceSkillClock == 0 ||
	//	pcsAgsdSkill->m_lLastTwiceSkillNumTarget == 0)
	//	return FALSE;

	// Buffering 되어 있는 스킬을 찾아서 return TRUE
	for(int i = 0; i < AGPMSKILL_MAX_TWICE_PACKET_BUFFER; i++)
	{
		if(pcsAttachData->m_alLastCastTwiceSkillID[i] == pcsSkill->m_lID)
			return TRUE;
	}

	return FALSE;
}

// 2004.11.08. steeple
// 캐스트 할때와 실제 공격시점이 다른 스킬의 사용이다.
// 해당 스킬이 캐스팅 된 후 세팅이 되어있는지 확인한다.
BOOL AgsmSkill::InitTwicePacketSkill(AgpdSkill* pcsSkill)
{
	if(!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsSkill->m_pcsBase);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	// 해당 스킬을 Buffer 에서 빼준다.
	for(int i = 0; i < AGPMSKILL_MAX_TWICE_PACKET_BUFFER; i++)
	{
		if(pcsAttachData->m_alLastCastTwiceSkillID[i] == pcsSkill->m_lID)
		{
			if(i != AGPMSKILL_MAX_TWICE_PACKET_BUFFER - 1)
				pcsAttachData->m_alLastCastTwiceSkillID.MemCopy(i, &pcsAttachData->m_alLastCastTwiceSkillID[i+1], AGPMSKILL_MAX_TWICE_PACKET_BUFFER - i - 1);

			pcsAttachData->m_alLastCastTwiceSkillID.MemSet(AGPMSKILL_MAX_TWICE_PACKET_BUFFER - 1, 1);

			i = 0;	// 처음부터 다시 돌면서 있는 거 다 지운다. 2006.11.15. steeple
		}
	}

	pcsAgsdSkill->m_ulLastCastTwiceSkillClock = 0;
	pcsAgsdSkill->m_alLastTwiceSkillTargetCID.MemSetAll();
	pcsAgsdSkill->m_lLastTwiceSkillNumTarget = 0;

	return TRUE;
}

// 2005.12.28. steeple
// 해당 스킬이 특수 상황(탈 것, 변신, 투명) 에서도 사용가능한 건지 리턴.
BOOL AgsmSkill::IsEnableCashItemSkillOnRide(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	if(pcsAgsdSkill->m_lSkillScrollIID == AP_INVALID_IID)
		return FALSE;

	AgpdItem* pcsSkillScroll = m_pagpmItem->GetItem(pcsAgsdSkill->m_lSkillScrollIID);
	if(!pcsSkillScroll || !pcsSkillScroll->m_pcsItemTemplate)
		return FALSE;

	if(IS_CASH_ITEM(pcsSkillScroll->m_pcsItemTemplate->m_eCashItemType) && pcsSkillScroll->m_pcsItemTemplate->m_bEnableOnRide)
		return TRUE;

	return FALSE;
}

// 2006.01.17. steeple
// Attack Damage 만 주는 스킬이라고 세팅한다.
BOOL AgsmSkill::SetAttackDamageOnly(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	pcsAgsdSkill->m_bApplyDamageOnly = TRUE;

	return TRUE;
}

BOOL AgsmSkill::SetDamageOnlyTemporary(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	pcsAgsdSkill->m_bDamageOnlyTemporary = TRUE;

	return TRUE;
}

// 2006.01.17. steeple
// Attack Damage 만 준다고 세팅되어 있는 지 확인.
BOOL AgsmSkill::IsSetAttackDamageOnly(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	return pcsAgsdSkill->m_bApplyDamageOnly;
}

BOOL AgsmSkill::IsSetDamageOnlyTemporary(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	return pcsAgsdSkill->m_bDamageOnlyTemporary;
}

INT32 AgsmSkill::TargetFiltering(AgpdSkill *pcsSkill, ApSafeArray<INT32, AGSMSKILL_MAX_TARGET_NUM>& lCIDBuffer, INT32 lNumTarget)
{
	PROFILE("AgsmSkill::TargetFiltering");

	if (!pcsSkill || lNumTarget < 1)
		return 0;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return 0;

	CHAR szTmp[64];
	sprintf(szTmp, "TargetFiletring;; CTID:%d, STID:%d, lNumTarget:%d", ((AgpdCharacter*)pcsSkill->m_pcsBase)->m_pcsCharacterTemplate->m_lID,
																		pcsSkill->m_pcsTemplate->m_lID,
																		lNumTarget);
	//STOPWATCH2(GetModuleName(), szTmp);

	AgpdCharacter* pcsTarget = NULL;
	INT32	lRemoveTarget	= 0;

	for (int i = 0; i < lNumTarget; ++i)
	{
		BOOL	bRemoveTarget	= FALSE;

		pcsTarget = m_pagpmCharacter->GetCharacter(lCIDBuffer[i]);
		
		// Lock 하기 전에 기본적인 검사를 해준다.
		if(!pcsTarget || pcsTarget->m_bIsReadyRemove || !pcsTarget->m_Mutex.WLock())
		{
			// 이 경우는 밑장 볼것도 없이 바로 빼고 다음 꺼 처리한다.
			lCIDBuffer.MemCopy(i, &lCIDBuffer[i + 1], lNumTarget - i - 1);
			lCIDBuffer[lNumTarget - 1] = 0;

			--lNumTarget;
			--i;

			continue;
		}

		if (!bRemoveTarget &&
			!m_pagpmSkill->IsPassiveSkill(pcsSkill) &&	// 패시브 스킬이면 죽었을 때라도 상관없당. 2005.05.22. steeple
			!(pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT) &&
			!m_pagpmSkill->IsDeadTarget(pcsSkillTemplate) &&	// 죽은 놈을 타겟할 수 있는 스킬. 2007.08.23. steeple
			pcsTarget->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		{
			bRemoveTarget = TRUE;
		}

		if (!bRemoveTarget && m_pagpmCharacter->IsAllBlockStatus(pcsTarget))
			bRemoveTarget = TRUE;

		// 자기 자신이 target에 들어가는지 여부를 판단한다.
		if (!bRemoveTarget && 
			!(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SELF_ONLY) &&
			!(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE_INVOLVE_SELF) &&
			!(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_TARGET_ONLY_INVOLVE_SELF) &&
			!(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_ONLY_GUILD_MEMBERS) &&
			!m_pagpmSkill->IsUnlimitedTargetSiegeWarSkill(pcsSkillTemplate))
		{
			if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER && pcsTarget->m_lID == pcsSkill->m_pcsBase->m_lID)
			{
				bRemoveTarget	= TRUE;
			}
		}

		// 길드전용 스킬인데 리전지역이 길드제외 지역이면 targer에서 제외한다.
		if(!bRemoveTarget &&
			(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_ONLY_GUILD_MEMBERS))
		{
			if(pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER && (pcsTarget->m_lID != pcsSkill->m_pcsBase->m_lID))
			{
				if(m_papmMap->CheckRegionPerculiarity(pcsTarget->m_nBindingRegionIndex, APMMAP_PECULIARITY_GUILD_BUFF) == APMMAP_PECULIARITY_RETURN_DISABLE_USE)
				{
					bRemoveTarget = TRUE;
				}
			}
		}

		//파티전용 스킬인데 리전지역이 파티버프제외 지역이면 target에서 제외한다.
		if(!bRemoveTarget &&
			(m_pagpmSkill->IsPartySkill(pcsSkillTemplate)))
		{
			if(pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER && (pcsTarget->m_lID != pcsSkill->m_pcsBase->m_lID))
			{
				if(m_papmMap->CheckRegionPerculiarity(pcsTarget->m_nBindingRegionIndex, APMMAP_PECULIARITY_PARTY_BUFF) == APMMAP_PECULIARITY_RETURN_DISABLE_USE)
				{
					bRemoveTarget = TRUE;
				}
			}
		}

		// 부활 스킬은 현상수배범한테 쓸 수 없다. 2007.08.24. steeple
		if (!bRemoveTarget &&
			m_pagpmWantedCriminal &&
			m_pagpmSkill->IsResurrectionSkill(pcsSkillTemplate) &&
			m_pagpmWantedCriminal->GetWantedCriminal(pcsTarget->m_szID))
		{
			bRemoveTarget = TRUE;
		}

		if (!bRemoveTarget &&
			(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_TARGET_ONLY_ENEMY_UNITS) ||
			(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE_ONLY_ENEMY_UNITS) ||
			(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_BOX_ONLY_ENEMY_UNITS))
		{
			if (!m_pagpmCharacter->IsTargetable(pcsTarget))
				bRemoveTarget	= TRUE;

			// 2008.6.18 arycoat - battleground
			if(m_pagpmBattleGround->IsInBattleGround(pcsTarget))
			{
				AgpdCharacter* pcsCharacter = (AgpdCharacter*)pcsSkill->m_pcsBase;
				if(!m_pagpmBattleGround->IsAttackable(pcsCharacter, pcsTarget))
					bRemoveTarget = TRUE;
			}
		}

		if (!bRemoveTarget &&
			(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE_ONLY_PARTY_MEMBER))
		{
			if(pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
				return 0;;

			// 2005.09.12. steeple
			// 소환수 때문에 변경. 소환수라면 주인의 PartyID 를 구해야 한다.
			AgpdCharacter* pcsCharacter = NULL;
			BOOL bOwnerLock = FALSE;
			if(m_pagpmCharacter->IsStatusSummoner((AgpdCharacter*)pcsSkill->m_pcsBase))
			{
				pcsCharacter = m_pagpmCharacter->GetCharacterLock(m_pagpmSummons->GetOwnerCID((AgpdCharacter*)pcsSkill->m_pcsBase));
				bOwnerLock = TRUE;
			}
			else
			{
				pcsCharacter = (AgpdCharacter*)pcsSkill->m_pcsBase;
			}

			if(!pcsCharacter)
				return 0;

			// PartyID 를 따로 구해서 놀자.
			INT32 lCID = pcsCharacter->m_lID;
			INT32 lPartyCID = 0;
			AgpdPartyADChar* pcsPartyADChar = m_pagpmParty->GetADCharacter(pcsCharacter);
			if(pcsPartyADChar)
				lPartyCID = pcsPartyADChar->lPID;

			if(bOwnerLock)
				pcsCharacter->m_Mutex.Release();

			// 2005.01.10. steeple
			// 파티에 들어있지 않으면 타겟은 자기 자신이다.
			if(lPartyCID == AP_INVALID_PARTYID)
			{
				if(lCID != pcsTarget->m_lID)
				{
					bRemoveTarget = TRUE;
				}
			}

			// target list에서 같은 파티에 속해 있지 않은 넘들을 걸러낸다.
			if(!bRemoveTarget)
			{
				AgpdPartyADChar* pcsTargetPartyADChar = m_pagpmParty->GetADCharacter(pcsTarget);
				if(pcsTargetPartyADChar)
				{
					if(pcsTargetPartyADChar->lPID != lPartyCID)
						bRemoveTarget = TRUE;
				}
				else
				{
					bRemoveTarget = TRUE;
				}
			}
		}

		// Check Target Restriction
		if(!bRemoveTarget && m_pagpmSkill->CheckTargetRestriction(pcsSkill, (ApBase*)pcsTarget) == TRUE)
		{
			bRemoveTarget = TRUE;
		}

		// 2007.05.09. steeple
		// 쉐도우 체이서 공성오브젝트에는 사용하지 못한다.
		if(!bRemoveTarget && m_pagpmSkill->GetActionOnActionType(pcsSkillTemplate) == 2)
		{
			AgpdSiegeWarMonsterType eType = m_pagpmSiegeWar->GetSiegeWarMonsterType(pcsTarget);
			if(eType >= AGPD_SIEGE_MONSTER_GUARD_TOWER && eType <= AGPD_SIEGE_MONSTER_CATAPULT)
				bRemoveTarget = TRUE;
		}

		// 2007.06.05. steeple
		// 길드 전용
		if(!bRemoveTarget && pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_ONLY_GUILD_MEMBERS)
		{
			CHAR* szBaseGuildID = m_pagpmGuild->GetJoinedGuildID((AgpdCharacter*)pcsSkill->m_pcsBase);
			if(szBaseGuildID && _tcslen(szBaseGuildID) > 0)
			{
				// 캐스터가 길드원일 때는 길드원만 가능하다.
				CHAR* szTargetGuildID = m_pagpmGuild->GetJoinedGuildID(pcsTarget);
				if(!szBaseGuildID || !szTargetGuildID || _tcscmp(szBaseGuildID, szTargetGuildID) != 0)
					bRemoveTarget = TRUE;
			}
			else
			{
				// 길드원이 아닐 때는 자기만 가능하다.
				if(pcsSkill->m_pcsBase->m_lID != pcsTarget->m_lID)
					bRemoveTarget = TRUE;
			}
		}

		// 2005.01.11. steeple. PvP 용 체크
		if (!bRemoveTarget)
		{
			if(!m_pagpmSkill->CheckSkillPvP(pcsSkill, pcsTarget))
				bRemoveTarget = TRUE;
		}

		// 2006.06.27. steeple. Level Diff 체크
		if( !bRemoveTarget && !CheckLevelDiff(pcsSkill, pcsTarget))
			bRemoveTarget = TRUE;

		// 2005.05.09. steeple. Buff / Debuff 체크
		if (!bRemoveTarget)
		{
			// Buff(Debuff) 스킬일 때는 맥스가 차버렸는지 확인해야 함. 2005.05.09. steeple
			if((pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_BUFF ||
				pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_DEBUFF) &&
				m_pagpmSkill->GetBuffedSkillCount(pcsTarget) >= AGPMSKILL_MAX_SKILL_BUFF)
			{
				// 2006.01.17. steeple
				// Attack 스킬이라면 버프 없이 어택데미지만 주게끔 설정한다.
				if(m_pagpmSkill->IsAttackSkill(pcsSkillTemplate))
				{
					SetAttackDamageOnly(pcsSkill);
				}
				else
				{
					// 스킬 안써짐.
					bRemoveTarget = TRUE;
				}
			}
		}

		// 2005.08.31. steeple
		// 소환수 포함이라면 그냥 체크해본다. 얘는 모든 체크의 맨 밑에 온다.
		if((pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SUMMONS_ONLY) ||
			(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE_ONLY_SUMMONS))
		{
			// 타겟 체크
			if(m_pagpmCharacter->IsStatusSummoner(pcsTarget))
			{
				// 타겟의 주인 체크
				if(m_pagpmSummons->GetOwnerCID(pcsTarget) == pcsSkill->m_pcsBase->m_lID)
				{
					if(bRemoveTarget)
						bRemoveTarget = FALSE;
				}
				// 2005.09.12. steeple
				// 스킬을 쓴 놈이 소환수 인데, 서로의 주인이 같은지 확인
				else if(m_pagpmCharacter->IsStatusSummoner((AgpdCharacter*)pcsSkill->m_pcsBase) &&
						m_pagpmSummons->GetOwnerCID((AgpdCharacter*)pcsSkill->m_pcsBase) == m_pagpmSummons->GetOwnerCID(pcsTarget))
				{
					if(bRemoveTarget)
						bRemoveTarget = FALSE;
				}
				else
					bRemoveTarget = TRUE;
			}
			// 타겟이 소환수도 아닌데 아직 Remove 가 안된놈이다.
			else if(bRemoveTarget == FALSE)
			{
				// Self 포함일 때 Self 는 Pass. 소환수가 쓴거라면 소환수 주인도 패스.
				if(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SELF_ONLY ||
					pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE_INVOLVE_SELF ||
					pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_TARGET_ONLY_INVOLVE_SELF)
				{
					if((m_pagpmCharacter->IsStatusSummoner((AgpdCharacter*)pcsSkill->m_pcsBase)) &&
						(m_pagpmSummons->GetOwnerCID((AgpdCharacter*)pcsSkill->m_pcsBase) == pcsTarget->m_lID))
					{
						// 스킬 쓰는 놈이 소환수 이고, 대상이 주인이면 지우지 않는다.
					}
					else if(pcsSkill->m_pcsBase->m_lID != pcsTarget->m_lID)
					{
						bRemoveTarget = TRUE;
					}
				}
				else
					bRemoveTarget = TRUE;
			}
		}

		pcsTarget->m_Mutex.Release();

		if (bRemoveTarget)
		{
			if(lNumTarget < AGSMSKILL_MAX_TARGET_NUM)
				lCIDBuffer.MemCopy(i, &lCIDBuffer[i + 1], lNumTarget - i - 1);

			lCIDBuffer[lNumTarget - 1] = 0;

			--lNumTarget;
			--i;
		}
	}

	return lNumTarget;
}

// 2005.07.04. steeple
// 스킬 레벨에 따라서 타겟 수가 변하는 스킬을 사용할 때는 이러한 처리를 해주어야한다.
// 1. 자기 포함 스킬이라면 자기를 배열의 맨 앞으로,
// 2. 1을 처리 하고 난 후 원래 타겟을 그 다음 맨 앞으로,
// 3. 나머지는 CID 로 정렬한 후 큰놈부터 들어가게 됨.
INT32 AgsmSkill::DynamicTargetFiltering(AgpdSkill* pcsSkill, ApSafeArray<INT32, AGSMSKILL_MAX_TARGET_NUM>& lCIDBuffer, INT32 lNumTarget, BOOL bIsTargetSelfCharacter)
{
	if(!pcsSkill || lNumTarget < 1 || lNumTarget > AGSMSKILL_MAX_TARGET_NUM - 1)
		return 0;

	// Dynamic Target 스킬이 아니면 나간다.
	if(!(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lEffectType2[0] & AGPMSKILL_EFFECT2_DYNAMIC_TARGET))
		return lNumTarget;

	// 달랑 Self Target 만이면 여기서 Exit
	if(lNumTarget == 1 && bIsTargetSelfCharacter)
		return 1;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return 0;

	INT32 lMaxTarget = (INT32)((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_MAX_TARGET_NUM][lSkillLevel];

	// 뭐꼬 이건
	if(lMaxTarget < 1)
		return lNumTarget;

	// 먼저 넘어온 CIDBuffer 를 정렬해야한다. CID 순으로
	INT32 alSortedCID[AGSMSKILL_MAX_TARGET_NUM];		// 정렬용 임시 배열
	memset(alSortedCID, 0, sizeof(alSortedCID));

	for(INT32 i = 0; i < lNumTarget; i++)
		alSortedCID[i] = (INT32)lCIDBuffer[i];

	// 정렬한다. CID 가 큰놈이 앞으로 오게 된다. 0 이나 음수를 뒤로 보내기 위해서.
	sort(alSortedCID, alSortedCID + lNumTarget, greater<INT32>());

	// 실제 타겟의 배열.
	INT32 alDynamicTargetCID[AGSMSKILL_MAX_TARGET_NUM];
	memset(alDynamicTargetCID, 0, sizeof(alDynamicTargetCID));
	INT32 lNewNumTarget = 0;
	
	// Self Target 이라면
	if(bIsTargetSelfCharacter)
		alDynamicTargetCID[lNewNumTarget++] = pcsSkill->m_pcsBase->m_lID;

	// Base 타겟을 그 다음으로 넣어준다.
	AgpdCharacter* pcsTarget = m_pagpmCharacter->GetCharacter(pcsSkill->m_csTargetBase.m_lID);
	if(m_pagpmSkill->CheckSkillPvP(pcsSkill, pcsTarget))
		alDynamicTargetCID[lNewNumTarget++] = pcsSkill->m_csTargetBase.m_lID;

	// 루프를 돌면서 
	for(INT32 i = 0; i < lNumTarget; ++i)
	{
		// 얘가 맨 위에 있어야 한다. 2005.10.11. steeple
		if(lNewNumTarget >= lMaxTarget)
			break;

		if(bIsTargetSelfCharacter)
		{
			if(alDynamicTargetCID[0] != alSortedCID[i] && alDynamicTargetCID[1] != alSortedCID[i])
				alDynamicTargetCID[lNewNumTarget++] = alSortedCID[i];
		}
		else
		{
			if(alDynamicTargetCID[0] != alSortedCID[i])
				alDynamicTargetCID[lNewNumTarget++] = alSortedCID[i];
		}
	}

	// 인자로 넘어온 놈들을 초기화 하고
	lCIDBuffer.MemSetAll();
	for(INT32 i = 0; i < lNewNumTarget; ++i)
		lCIDBuffer[i] = alDynamicTargetCID[i];

	return lNewNumTarget;
}

// Skill Rate Check - 2004.09.17. steeple
BOOL AgsmSkill::SkillRateCheck(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	if(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel] == 0 ||
		pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel] == 100)
		return TRUE;

	// 2005.07.15. steeple
	// Passive 라면 일단 성공. Action Passive 는 따로 처리해준다.
	if(m_pagpmSkill->IsPassiveSkill(pcsSkill))
		return TRUE;

	// 여기에 걸리는 스킬은 처음 캐스트 할때는 Skill_Rate 를 무시하는 스킬이다.
	// 일반적으로 버프 계열 스킬이 이렇게 된다.
	for(int i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
	{
		if(pcsSkillTemplate->m_lConditionType[i] & AGPMSKILL_CONDITION_SKIP_CHECK_FIRST)
		{
			return TRUE;
		}
	}

	INT16 nRandomNumber = m_csRandom.randInt(100);
	if(nRandomNumber <= pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel])
		return TRUE;

	return FALSE;
}

// Skill Rate 를 리턴한다. Const 값이 0일때는 100으로 리턴한다. - 2007.06.30. steeple
INT32 AgsmSkill::GetSkillRate(AgpdSkill* pcsSkill)
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate)
		return 0;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return 0;

	if(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel] == 0 ||
		pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel] == 100)
		return 100;

	if(m_pagpmSkill->IsPassiveSkill(pcsSkill))
		return 100;

	return (INT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_RATE][lSkillLevel];
}

// Halt Status Check - 2005.10.17. steeple
BOOL AgsmSkill::HaltStatusCheck(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	//// Attack Skill 이면
	//if(m_pagpmSkill->IsAttackSkill(pcsSkillTemplate))
	//	return FALSE;

	//// Debuff 스킬이면
	//if(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_BUFF_TYPE][1] == 1)
	//	return FALSE;

	// 2006.01.10. steeple. (PvP 쪽의 체크로 변경)
	if(m_pagpmPvP->IsAttackSkill(pcsSkillTemplate))
		return FALSE;

	// 소환, 테임 스킬은 불가능
	if(m_pagpmSkill->IsSummonsSkill(pcsSkillTemplate) || m_pagpmSkill->IsTameSkill(pcsSkillTemplate) ||
		m_pagpmSkill->IsFixedSkill(pcsSkillTemplate))
		return FALSE;

	// AT Field Attack 불가능.
	if(m_pagpmSkill->IsATFieldAttackSkill(pcsSkillTemplate))
		return FALSE;

	return TRUE;
}

// Debuff 스킬을 타겟에게 걸 수 있는 지 체크 - 2004.09.21. steeple
// return FALSE 일 때만, 못거는 것임.
BOOL AgsmSkill::IsDebuffSkillEnable(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	// 2007.08.21. steeple
	// 이 작업 하지 않는다.
	return TRUE;

	if(!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	// pcsSkill 이 Debuff 스킬이 아니면 안써도 된다.
	if(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_BUFF_TYPE][lSkillLevel] != 1)
		return TRUE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsTarget);
	if(!pcsAttachData)
		return FALSE;

	// Target 에 Debuff 를 막는 스킬이 Buff 되어 있는 지 확인한다.
	AgsdSkillADBase* pcsADBase = GetADBase(pcsTarget);
	if(!pcsADBase)
		return FALSE;

	AgpdSkillTemplate* pcsBuffedTemplate = NULL;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID)
			break;

		if(!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			break;
		
		pcsBuffedTemplate = (AgpdSkillTemplate *) pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate;
		if(!pcsBuffedTemplate)
			continue;

		for(int j = 0; j < AGPMSKILL_MAX_SKILL_CONDITION; ++j)
		{
			// 버프된 스킬중에 Debuff 를 막는 놈을 발견하면 return FALSE
			if(pcsBuffedTemplate->m_lEffectType[j] & AGPMSKILL_EFFECT_DISPEL_MAGIC_CURE ||
				pcsBuffedTemplate->m_lProcessIntervalEffectType[j] & AGPMSKILL_EFFECT_DISPEL_MAGIC_CURE)
				return FALSE;
		}
	}

	return TRUE;
}

// 2006.10.24. steeple
BOOL AgsmSkill::SetForceSkill(AgpdSkill* pcsSkill)
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;

	// Template 에 세팅되어 있을 때만 추가로 해주는 것이다.
	if(m_pagpmSkill->IsSummonsSkill(pcsSkillTemplate))
	{
		if(m_pagpmSkill->IsForceSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
			pcsSkill->m_bForceAttack = TRUE;
		else
			pcsSkill->m_bForceAttack = FALSE;
	}
	else
	{
		if(m_pagpmSkill->IsForceSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
			pcsSkill->m_bForceAttack = TRUE;
	}

	return TRUE;
}

// 2007.07.02. steeple
// TRUE 로 리턴하면 스킬이 성공하는 것이고,
// FALSE 로 리턴하면 Skill Attack Invincible 이 발동한 것이다.
BOOL AgsmSkill::SkillInvincibleCheck(AgpdSkill* pcsSkill, AgpdCharacter* pcsTarget)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	// Dispel All Buff 도 체크
	// 공격 스킬인 경우에만 체크한다.
	if(!m_pagpmSkill->IsDispelAllBuffSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) &&
		!m_pagpmSkill->IsAttackSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
		return TRUE;

	AgsdCharacter* pcsAgsdTarget = m_pagsmCharacter->GetADCharacter(pcsTarget);
	if(!pcsAgsdTarget)
		return FALSE;

	if(!m_pagpmCharacter->IsStatusSkillATKInvincible(pcsTarget))
		return TRUE;

	INT32 lRandom = m_csRandom.randInt(100);
	if(pcsAgsdTarget->m_stInvincibleInfo.lSkillATKProbability > lRandom)
		return FALSE;	// 스킬공격에 대한 무적 발동.

	return TRUE;
}

// 1. range check functions
///////////////////////////////////////////////////////////////////////////////

//		RangeCheck
//	Functions
//		- pcsSkill 의 range type을 보고 range에 해당하는 target들을 구한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- plID : target id를 저장할 공간
//		- nMaxBuffer : 위 plID의 버퍼 크기
//		- pcsEvent : 이 스킬 event
//	Return value
//		- INT32 : 구한 target 갯수
///////////////////////////////////////////////////////////////////////////////
INT32 AgsmSkill::RangeCheck(AgpdSkill *pcsSkill, INT_PTR *plID, INT16 nMaxBuffer, INT32* plID2, INT16 nMaxBuffer2, ApdEvent *pcsEvent)
{
	PROFILE("AgsmSkill::RangeCheck");

	if (!pcsSkill || !plID || nMaxBuffer < 1 || !pcsEvent)
		return 0;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return 0;

	CHAR szTmp[64];
	sprintf(szTmp, "RangeCheck;; CTID:%d, STID:%d", ((AgpdCharacter*)pcsSkill->m_pcsBase)->m_pcsCharacterTemplate->m_lID,
													pcsSkill->m_pcsTemplate->m_lID);
	//STOPWATCH2(GetModuleName(), szTmp);

	INT32	lNumTarget = 0;

	if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SELF_ONLY)
	{
		lNumTarget += RangeCheckSelfOnly(pcsSkill, plID + lNumTarget, nMaxBuffer - lNumTarget, plID2 + lNumTarget, nMaxBuffer - lNumTarget, pcsEvent);
	}

	if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_TARGET_ONLY)
	{
		lNumTarget += RangeCheckTargetOnly(pcsSkill, plID + lNumTarget, nMaxBuffer - lNumTarget, plID2 + lNumTarget, nMaxBuffer - lNumTarget, pcsEvent);
	}

	if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE || m_pagpmSkill->IsSphereRangeSiegeWarSkill(pcsSkillTemplate))
	{
		lNumTarget += RangeCheckSphere(pcsSkill, plID + lNumTarget, nMaxBuffer - lNumTarget, plID2 + lNumTarget, nMaxBuffer - lNumTarget, pcsEvent);
	}

	if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_BOX)
	{
		lNumTarget += RangeCheckBox(pcsSkill, plID + lNumTarget, nMaxBuffer - lNumTarget, plID2 + lNumTarget, nMaxBuffer - lNumTarget, pcsEvent);
	}

	if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SUMMONS_ONLY)
	{
		lNumTarget += RangeCheckSummonsOnly(pcsSkill, plID + lNumTarget, nMaxBuffer - lNumTarget, plID2 + lNumTarget, nMaxBuffer - lNumTarget, pcsEvent);
	}

	// Range Type2
	//

	// 2006.08.11. steeple
	if(!m_pagpmSkill->IsSphereRangeSiegeWarSkill(pcsSkillTemplate) && m_pagpmSkill->IsUnlimitedTargetSiegeWarSkill(pcsSkillTemplate))
	{
		// 타겟숫자 제한없는 공성스킬이면 일단 자기만 타겟으로 해준다.
		lNumTarget += RangeCheckSelfOnly(pcsSkill, plID + lNumTarget, nMaxBuffer - lNumTarget, plID2 + lNumTarget, nMaxBuffer - lNumTarget, pcsEvent);
	}

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_SPHERE)
	{
		lNumTarget += RangeCheckSphere(pcsSkill, plID + lNumTarget, nMaxBuffer - lNumTarget, plID2 + lNumTarget, nMaxBuffer - lNumTarget, pcsEvent);
	}

	return lNumTarget;
}

//		RangeCheckSelfOnly
//	Functions
//		- pcsSkill 의 range type (AGSMSKILL_RANGE_SELF_ONLY) 을 처리한다.
//			그냥 자기 자신의 아뒤를 리턴한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- plID : target id를 저장할 공간
//		- nMaxBuffer : 위 plID의 버퍼 크기
//	Return value
//		- INT32 : 구한 target 갯수
///////////////////////////////////////////////////////////////////////////////
INT32 AgsmSkill::RangeCheckSelfOnly(AgpdSkill *pcsSkill, INT_PTR *plID, INT16 nMaxBuffer, INT32* plID2, INT16 nMaxBuffer2, ApdEvent *pcsEvent)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || !plID || nMaxBuffer < 1 || !pcsEvent)
		return 0;

	/*
	// condition setting
	if (!m_papmEventManager->SetCondition(pcsEvent, APDEVENT_COND_TARGET))
		return 0;

	pcsEvent->m_pstCondition->m_pstTarget->m_lCID = pcsSkill->m_pcsBase->m_lID;
	*/

	INT32	lNumTarget = 0;

	plID[0] = (INT_PTR) pcsSkill->m_pcsBase;
	plID2[0] = pcsSkill->m_pcsBase->m_lID;
	++lNumTarget;

	return lNumTarget;
}

//		RangeCheckTargetOnly
//	Functions
//		- pcsSkill 의 range type (AGSMSKILL_RANGE_TARGET_ONLY) 을 처리한다.
//			pcsSkill에 세팅되어있는 m_pcsTargetBase의 아뒤를 리턴한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- plID : target id를 저장할 공간
//		- nMaxBuffer : 위 plID의 버퍼 크기
//	Return value
//		- INT32 : 구한 target 갯수
///////////////////////////////////////////////////////////////////////////////
INT32 AgsmSkill::RangeCheckTargetOnly(AgpdSkill *pcsSkill, INT_PTR *plID, INT16 nMaxBuffer, INT32* plID2, INT16 nMaxBuffer2, ApdEvent *pcsEvent)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || !plID || nMaxBuffer < 1 || !pcsEvent ||
		pcsSkill->m_csTargetBase.m_eType == APBASE_TYPE_NONE || pcsSkill->m_csTargetBase.m_lID == 0)
		return 0;

	// condition setting
	if (!m_papmEventManager->SetCondition(pcsEvent, APDEVENT_COND_TARGET))
		return 0;

	pcsEvent->m_pstCondition->m_pstTarget->m_lCID = pcsSkill->m_csTargetBase.m_lID;

	INT32	lNumTarget = 0;

	plID[0] = (INT_PTR) GetBase((ApBase *) &pcsSkill->m_csTargetBase);
	plID2[0] = pcsSkill->m_csTargetBase.m_lID;
	if (plID[0] != 0)
		++lNumTarget;

	return lNumTarget;
}

//		RangeCheckSphere
//	Functions
//		- pcsSkill 의 range type (AGSMSKILL_RANGE_SPHERE) 을 처리한다.
//			
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- plID : target id를 저장할 공간
//		- nMaxBuffer : 위 plID의 버퍼 크기
//	Return value
//		- INT32 : 구한 target 갯수
///////////////////////////////////////////////////////////////////////////////
INT32 AgsmSkill::RangeCheckSphere(AgpdSkill *pcsSkill, INT_PTR *plID, INT16 nMaxBuffer, INT32* plID2, INT16 nMaxBuffer2, ApdEvent *pcsEvent)
{
	if (!pcsSkill || !plID || nMaxBuffer < 1 || !pcsEvent)
		return 0;

	// condition setting
	if (!m_papmEventManager->SetCondition(pcsEvent, APDEVENT_COND_AREA))
		return 0;

	pcsEvent->m_pstCondition->m_pstArea->m_eType					= APDEVENT_AREA_SPHERE;

	INT32	lRange = m_pagpmSkill->GetRange(pcsSkill);

	AuPOS	*pcsSourcePos = NULL;

	// range가 0이면 자기 자신이 범위의 중심.. 0이 아니면 target 의 위치나 target pos의 위치가 중심점이다.
	// 이때 range 범위 안에 있는지 검사한다. range 범위 밖이면 무시....
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_BASE_TARGET ||
		m_pagpmSkill->IsGroundTargetSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)
		)
	{
		pcsSourcePos = &pcsSkill->m_posTarget;
	}
	else
	{
		pcsSourcePos = m_papmEventManager->GetBasePos(pcsSkill->m_pcsBase, NULL);
	}

	if (!pcsSourcePos)
		return 0;

	INT32	lArea1	= m_pagpmSkill->GetTargetArea1(pcsSkill);

	if (lArea1 <= 0)
		lArea1	= lRange;

	pcsEvent->m_pstCondition->m_pstArea->m_uoData.m_fSphereRadius	= (FLOAT)(lArea1 + pcsSkill->m_lTargetHitRange);

	INT16	nNumTarget = m_papmEventManager->GetAreaCharacters(((AgpdCharacter*)pcsSkill->m_pcsBase)->m_nDimension, APMMAP_CHAR_TYPE_PC | APMMAP_CHAR_TYPE_NPC | APMMAP_CHAR_TYPE_MONSTER, pcsEvent, plID, nMaxBuffer, plID2, nMaxBuffer2, pcsSourcePos);

	// 2007.04.04. steeple
	// 공성스킬인데 한정범위라면 자신을 가장 앞으로 보내준다.
	if(m_pagpmSkill->IsSphereRangeSiegeWarSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
	{
		RangeCheckBaseTargetToFirst(plID, nMaxBuffer, plID2, nMaxBuffer2, (INT_PTR)pcsSkill->m_pcsBase, pcsSkill->m_pcsBase->m_lID);
	}

	// 2005.10.26. steeple
	// BaseTarget 이 있다면 Base 타겟을 타겟배열 맨 앞으로 해준다.
	else if(pcsSkill->m_csTargetBase.m_lID != 0)
	{
		ApBase* pcsTargetBase = m_papmEventManager->GetBase(pcsSkill->m_csTargetBase.m_eType, pcsSkill->m_csTargetBase.m_lID);
		if(pcsTargetBase)
			RangeCheckBaseTargetToFirst(plID, nMaxBuffer, plID2, nMaxBuffer2, (INT_PTR)pcsTargetBase, pcsTargetBase->m_lID);
	}

	return nNumTarget;
}

// 2005.07.07. steeple
// AT Field 의 Effect Area 를 Sphere 로 처리할 때 이것이 불린다.
BOOL AgsmSkill::RangeCheckSphereForATField(AgpdSkill *pcsSkill, ApBase* pcsTarget, INT_PTR *plID, INT16 nMaxBuffer, INT32* plID2, INT16 nMaxBuffer2, ApdEvent *pcsEvent)
{
	// 2005.08.29. steeple
	// 이 체크는 pcsTarget 이 없으면 걍 나가뿐진다.
	if(!pcsSkill || !plID || nMaxBuffer < 1 || !pcsEvent || !pcsTarget)
		return 0;

	if(pcsTarget->m_eType != APBASE_TYPE_CHARACTER)
		return 0;

	// condition setting
	if(!m_papmEventManager->SetCondition(pcsEvent, APDEVENT_COND_AREA))
		return 0;

	pcsEvent->m_pstCondition->m_pstArea->m_eType = APDEVENT_AREA_SPHERE;

	AuPOS csSourcePos = ((AgpdCharacter*)pcsTarget)->m_stPos;

	INT32 lArea1 = m_pagpmSkill->GetTargetArea1(pcsSkill);
	pcsEvent->m_pstCondition->m_pstArea->m_uoData.m_fSphereRadius = (FLOAT)lArea1;

	INT16 nNumTarget = m_papmEventManager->GetAreaCharacters(((AgpdCharacter*)pcsTarget)->m_nDimension,
											APMMAP_CHAR_TYPE_PC | APMMAP_CHAR_TYPE_NPC | APMMAP_CHAR_TYPE_MONSTER,
											pcsEvent, plID, nMaxBuffer, plID2, nMaxBuffer2, &csSourcePos);

	// 2005.10.26. steeple
	RangeCheckBaseTargetToFirst(plID, nMaxBuffer, plID2, nMaxBuffer2, (INT_PTR)pcsTarget, pcsTarget->m_lID);

	return nNumTarget;
}

//		RangeCheckBox
//	Functions
//		- pcsSkill 의 range type (AGSMSKILL_RANGE_BOX) 을 처리한다.
//			그냥 자기 자신의 아뒤를 리턴한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- plID : target id를 저장할 공간
//		- nMaxBuffer : 위 plID의 버퍼 크기
//	Return value
//		- INT32 : 구한 target 갯수
///////////////////////////////////////////////////////////////////////////////
INT32 AgsmSkill::RangeCheckBox(AgpdSkill *pcsSkill, INT_PTR *plID, INT16 nMaxBuffer, INT32* plID2, INT16 nMaxBuffer2, ApdEvent *pcsEvent)
{
	if (!pcsSkill || !plID || nMaxBuffer < 1 || !pcsEvent)
		return 0;

	// condition setting
	if (!m_papmEventManager->SetCondition(pcsEvent, APDEVENT_COND_AREA))
		return 0;

	pcsEvent->m_pstCondition->m_pstArea->m_eType		= APDEVENT_AREA_BOX;

	// range가 box 인 경우 무조건 범위 중심을 자기 자신으로 세팅한다.
	// 차후에 이런 경우가 아닌게 나오면 그때 적당히 수정한다.
	//
	//
	//
	//

	/*
	INT32	lRange = m_pagpmSkill->GetRange(pcsSkill);

	//ApBase	*pcsSourceBase = NULL;
	AuPOS	*pcsSourcePos = NULL;

	// range가 0이면 자기 자신이 범위의 중심.. 0이 아니면 target 의 위치나 target pos의 위치가 중심점이다.
	// 이때 range 범위 안에 있는지 검사한다. range 범위 밖이면 무시....
	if (lRange)
	{
		//if (pcsSkill->m_pcsTargetBase)
		//{
		//	pcsSourceBase = pcsSkill->m_pcsTargetBase;
		//	pcsSourcePos = m_papmEventManager->GetBasePos(pcsSkill->m_pcsTargetBase, NULL);
		//}
		//else
		//	return 0;
			//pcsSourcePos = &pcsSkill->m_posTarget;

		pcsSourcePos = &pcsSkill->m_posTarget;

		// target까지의 범위 검사 한다.
		if (pcsSourcePos)
		{
			AuPOS	*pcsBasePos = m_papmEventManager->GetBasePos(pcsSkill->m_pcsBase, NULL);
			if (!pcsBasePos)
				return 0;

			INT32	lTargetDistance = 
				sqrt( pow ( ( pcsBasePos->x - pcsSourcePos->x ), 2 ) + pow ( ( pcsBasePos->z - pcsSourcePos->z ), 2));

			if (lRange < lTargetDistance)
				return 0;
		}
	}
	else
	{
		//pcsSourceBase = pcsSkill->m_pcsBase;
		pcsSourcePos = m_papmEventManager->GetBasePos(pcsSkill->m_pcsBase, NULL);
	}

	if (!pcsSourcePos)
	//if (!pcsSourceBase)
		return 0;
	*/

	FLOAT	x = (FLOAT)m_pagpmSkill->GetTargetArea2(pcsSkill);
	FLOAT	z = (FLOAT)m_pagpmSkill->GetTargetArea1(pcsSkill);

	// 2005.10.26. steeple
	// TargetHitRange 를 더해준다. TT 이걸로 버그 해결된 듯..... 기쁘다. TT
	x += pcsSkill->m_lTargetHitRange;
	z += pcsSkill->m_lTargetHitRange;

	pcsEvent->m_pstCondition->m_pstArea->m_uoData.m_stBox.inf.x = (FLOAT)(-(x / 2.0));
	pcsEvent->m_pstCondition->m_pstArea->m_uoData.m_stBox.inf.y = (FLOAT)(-1000);
	pcsEvent->m_pstCondition->m_pstArea->m_uoData.m_stBox.inf.z = (FLOAT)(0);

	pcsEvent->m_pstCondition->m_pstArea->m_uoData.m_stBox.sup.x = (FLOAT)(x / 2.0);
	pcsEvent->m_pstCondition->m_pstArea->m_uoData.m_stBox.sup.y = (FLOAT)1000;
	pcsEvent->m_pstCondition->m_pstArea->m_uoData.m_stBox.sup.z = (FLOAT)z;

	INT32	nNumTarget = 0;
	nNumTarget = m_papmEventManager->GetAreaCharacters(((AgpdCharacter*)pcsSkill->m_pcsBase)->m_nDimension, APMMAP_CHAR_TYPE_PC | APMMAP_CHAR_TYPE_NPC | APMMAP_CHAR_TYPE_MONSTER, pcsEvent, plID, nMaxBuffer, plID2, nMaxBuffer2, pcsSkill->m_pcsBase);

	if (nNumTarget == 0)
		return 0;

	// 2005.10.26. steeple
	// BaseTarget 이 있다면 Base 타겟을 타겟배열 맨 앞으로 해준다.
	if(pcsSkill->m_csTargetBase.m_lID != 0)
	{
		ApBase* pcsTargetBase = m_papmEventManager->GetBase(pcsSkill->m_csTargetBase.m_eType, pcsSkill->m_csTargetBase.m_lID);
		if(pcsTargetBase)
			RangeCheckBaseTargetToFirst(plID, nMaxBuffer, plID2, nMaxBuffer2, (INT_PTR)pcsTargetBase, pcsTargetBase->m_lID);
	}

	return nNumTarget;
}

// 2005.09.04. steeple
// Only own summons or owner's summons
BOOL AgsmSkill::RangeCheckSummonsOnly(AgpdSkill* pcsSkill, INT_PTR* plID, INT16 nMaxBuffer, INT32* plID2, INT16 nMaxBuffer2, ApdEvent* pcsEvent)
{
	if(!m_pagpmSummons || !m_pagsmSummons)
		return 0;

	if(!pcsSkill || !pcsSkill->m_pcsBase || !plID || nMaxBuffer < 1 || !pcsEvent)
		return 0;

	AgpdSummonsADChar* pcsSummonsADChar = m_pagpmSummons->GetADCharacter((AgpdCharacter*)pcsSkill->m_pcsBase);
	if(!pcsSummonsADChar)
		return 0;

	AgpdCharacter* pcsOwner = NULL;
	AgpdSummonsADChar* pcsSummonsADOwner = NULL;
	BOOL bOwnerBySummons = FALSE;

	// PC 라면
	if(m_pagpmCharacter->IsPC((AgpdCharacter*)pcsSkill->m_pcsBase))
	{
		pcsOwner = (AgpdCharacter*)pcsSkill->m_pcsBase;
		pcsSummonsADOwner = pcsSummonsADChar;
	}
	else if(m_pagpmCharacter->IsStatusSummoner((AgpdCharacter*)pcsSkill->m_pcsBase))
	{
		pcsOwner = m_pagpmCharacter->GetCharacterLock(pcsSummonsADChar->m_lOwnerCID);
		if(pcsOwner)
		{
			bOwnerBySummons = TRUE;
			pcsSummonsADOwner = m_pagpmSummons->GetADCharacter(pcsOwner);
		}
	}

	if(!pcsOwner)
		return 0;

	INT16 nNumTarget = 0;
	ApBase* pcsTargetBase = NULL;

	if(pcsSummonsADOwner)
	{
		INT16 nIndex = 0;
		AgpdSummonsArray::iterator iter = pcsSummonsADOwner->m_SummonsArray.m_pSummons->begin();
		while(iter != pcsSummonsADOwner->m_SummonsArray.m_pSummons->end() && nIndex < nMaxBuffer)
		{
			pcsTargetBase = m_papmEventManager->GetBase(APBASE_TYPE_CHARACTER, iter->m_lCID);
			if(pcsTargetBase)
			{
				plID[nNumTarget] = (INT_PTR)pcsTargetBase;
				plID2[nNumTarget] = pcsTargetBase->m_lID;
				++nNumTarget;
			}

			++iter;
			++nIndex;
		}
	}

	if(bOwnerBySummons)
		pcsOwner->m_Mutex.Release();

	return nNumTarget;
}

// 2006.02.24. steeple
// 모두 INT_PTR 로 변경.
//
// 2005.10.26. steeple
// 범위형에서 BaseTarget 이 있을 때, BaseTarget 을 맨 앞에 넣어준다.
INT32 AgsmSkill::RangeCheckBaseTargetToFirst(INT_PTR* plID, INT16 nMaxBuffer, INT32* plID2, INT16 nMaxBuffer2, INT_PTR lBaseTarget, INT32 lBaseTargetCID)
{
	if(!plID || nMaxBuffer < 1 || lBaseTarget == 0)
		return 0;

	if(plID[0] == lBaseTarget)
		return 0;

	for(INT16 nIndex = 0; nIndex < nMaxBuffer; ++nIndex)
	{
		if(plID[nIndex] == 0)
			break;

		// 둘을 바꾼다. swap
		if(plID[nIndex] == lBaseTarget)
		{
			INT_PTR lTmp = plID[0];
			plID[0] = lBaseTarget;
			plID[nIndex] = lTmp;

			INT32 lTmp2 = plID2[0];
			plID2[0] = lBaseTargetCID;
			plID2[nIndex] = lTmp2;

			return 1;
		}
	}

	return 0;
}

// 2. condition check functions
///////////////////////////////////////////////////////////////////////////////

//		ConditionCheck
//	Functions
//		- pcsSkill 의 condition type에 따른 함수를 호출해 condition을 check한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheck(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	PROFILE("AgsmSkill::ConditionCheck");

	if (!pcsSkill || !pcsTarget)
		return FALSE;

	BOOL	bCheckResult = TRUE;

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MELEE_ATTACK)
	{
		bCheckResult &= ConditionCheckMeleeAttack(pcsSkill, pcsTarget, nIndex);
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MAGIC_ATTACK)
	{
		bCheckResult &= ConditionCheckMagicAttack(pcsSkill, pcsTarget, nIndex);
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_DEBUFF)
	{
		bCheckResult &= ConditionCheckDeBuff(pcsSkill, pcsTarget, nIndex);
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_FRIENDLY_UNIT)
	{
		bCheckResult &= ConditionCheckFriendlyUnit(pcsSkill, pcsTarget, nIndex);
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_LEVEL)
	{
		bCheckResult &= ConditionCheckLevel(pcsSkill, pcsTarget, nIndex);
	}

	/*
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_KILL_GUARDIAN)
	{
		bCheckResult &= ConditionCheckKillGuardian(pcsSkill, pcsTarget, nIndex);
	}
	*/

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_CHAR_TYPE)
	{
		bCheckResult &= ConditionCheckCharType(pcsSkill, pcsTarget, nIndex);
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_OWN_ITEM)
	{
		bCheckResult &= ConditionCheckOwnItem(pcsSkill, pcsTarget, nIndex);
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_PARTY_MEMBER)
	{
		bCheckResult &= ConditionCheckPartyMember(pcsSkill, pcsTarget, nIndex);
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_SERVER_CONTROL)
	{
		AgpdCharacter* pcsCharacter = (AgpdCharacter*)pcsSkill->m_pcsBase;
		if(m_pagpmBattleGround->IsInBattleGround(pcsCharacter))
			bCheckResult &= m_pagsmBattleGround->CheckControlSkill(pcsCharacter, pcsSkill);
	}

	// condition이 없으면 TRUE
	return bCheckResult;
}

//		ConditionCheckMeleeAttack
//	Functions
//		- pcsSkill 의 condition type (AGSMSKILL_CONDITION_MELEE_ATTACK) 에 따른 함수를 호출해 condition을 check한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheckMeleeAttack(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	if (!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdFactor	*pcsAttackFactorResult	= GetOwnerFactorResult(pcsSkill);
	AgpdFactor	*pcsTargetFactorResult	= GetResultFactor(pcsTarget);

	if (!pcsAttackFactorResult || !pcsTargetFactorResult)
		return FALSE;

	INT32	lAttackPoint = 0;

	AgpdFactorCharPointMax	*pcsCharPointMax = NULL;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	// 스킬에 필요한 아이템들을 착용하고 있는지 검사한다.

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MELEE_ATTACK_EQUIP_WEAPON)
	{
		if (!pcsSkill->m_pcsBase || pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
			return FALSE;

		AgpdGridItem	*pcsGridItem;

		pcsGridItem = m_pagpmItem->GetEquipItem( (AgpdCharacter *) pcsSkill->m_pcsBase, AGPMITEM_PART_HAND_RIGHT );

		if (pcsGridItem == NULL)
			return FALSE;
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MELEE_ATTACK_EQUIP_ARROW)
	{
		if (!pcsSkill->m_pcsBase || pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
			return FALSE;

		AgpdGridItem	*pcsGridItem;
		INT32			lItemID;

		lItemID = 0;

		pcsGridItem = m_pagpmItem->GetEquipItem( (AgpdCharacter *) pcsSkill->m_pcsBase, AGPMITEM_PART_HAND_LEFT );

		if( pcsGridItem != NULL )
		{
			lItemID = pcsGridItem->m_lItemID;
		}

		AgpdItem	*pcsItem	= m_pagpmItem->GetItem( lItemID );
		if (!pcsItem)
			return FALSE;

		if (((AgpdItemTemplateEquipWeapon *) pcsItem->m_pcsItemTemplate)->m_nType != AGPMITEM_TYPE_EQUIP ||
			((AgpdItemTemplateEquipWeapon *) pcsItem->m_pcsItemTemplate)->m_nKind != AGPMITEM_EQUIP_KIND_WEAPON ||
			(((AgpdItemTemplateEquipWeapon *) pcsItem->m_pcsItemTemplate)->m_nWeaponType != AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_BOW &&
			 ((AgpdItemTemplateEquipWeapon *) pcsItem->m_pcsItemTemplate)->m_nWeaponType != AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_CROSSBOW))
			return FALSE;
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MELEE_ATTACK_EQUIP_SHIELD)
	{
		if (!pcsSkill->m_pcsBase || pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
			return FALSE;

		AgpdGridItem	*pcsAgpdGridItem;

		pcsAgpdGridItem = m_pagpmItem->GetEquipItem( (AgpdCharacter *) pcsSkill->m_pcsBase, AGPMITEM_PART_HAND_LEFT );

		if (pcsAgpdGridItem == NULL)
			return FALSE;

		AgpdItem	*pcsItem	= m_pagpmItem->GetItem(pcsAgpdGridItem->m_lItemID);
		if (!pcsItem)
			return FALSE;

		if (((AgpdItemTemplateEquip *) pcsItem->m_pcsItemTemplate)->m_nType != AGPMITEM_TYPE_EQUIP ||
			((AgpdItemTemplateEquip *) pcsItem->m_pcsItemTemplate)->m_nKind != AGPMITEM_EQUIP_KIND_SHIELD)
			return FALSE;
	}

	// melee attack 명중식중 attack point 대신 지정된 attack point 상수를 사용하는 경우
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MELEE_ATTACK_USE_AP_CONST)
	{
		// attack point를 정의된 상수로 변경한다.
		m_pagpmFactors->GetValue(pcsAttackFactorResult, &lAttackPoint, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINT_TYPE_AP);
		m_pagpmFactors->SetValue(pcsAttackFactorResult, (INT32) (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_POINT_AP][lSkillLevel]), AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINT_TYPE_AP);
	}

	BOOL	bCheckResult	= FALSE;

	// 명중식 : ran (100) > (저항기준 상수A)-(10*(마스터리 포인트-대상의 LV)) 참이면 저항 실패
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MELEE_ATTACK_USE_FORMULA1)
	{
		INT16	nRandomNumber = m_csRandom.randInt(100);
		
		if (nRandomNumber > (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_RESIST_A][lSkillLevel] - (10 * (lSkillLevel - GetModifiedCharLevel(pcsTarget)))))
			bCheckResult = TRUE;
		else
			bCheckResult = FALSE;
	}
	// melee attack 명중식을 사용하는 경우
	else
	{
		// 명중여부를 판단한다.
		bCheckResult = m_pagpmCombat->IsAttackSuccess((AgpdCharacter *) pcsSkill->m_pcsBase, (AgpdCharacter *) pcsTarget);
	}

	if (bCheckResult && ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MELEE_ATTACK_REDUCE_HITRATE)
	{
		INT16	nRandomNumber = m_csRandom.randInt(100);

		if (nRandomNumber > pcsSkillTemplate->m_stConditionArg[nIndex].lArg1)
			bCheckResult = FALSE;
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MELEE_ATTACK_USE_AP_CONST)
	{
		// 위에서 변경한 값을 되돌린다.
		pcsCharPointMax->lValue[AGPD_FACTORS_CHARPOINT_TYPE_AP] = lAttackPoint;
	}

	return bCheckResult;
}

//		ConditionCheckDeBuff
//	Functions
//		- pcsSkill 의 condition type (AGSMSKILL_CONDITION_DEBUFF) 에 따른 함수를 호출해 condition을 check한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheckDeBuff(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	if (!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate		*pcsSkillTemplate			= (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

	if (!pcsSkillTemplate)
		return FALSE;

	INT32	lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	INT32	lTargetLevel = GetModifiedCharLevel(pcsTarget);

	INT16	nRandomNumber = m_csRandom.randInt(100);

	INT16	nRightResult = (INT16) (pcsSkillTemplate->m_stConditionArg[nIndex].lArg1 - ((lSkillLevel - lTargetLevel) * 5));

	if (nRightResult < pcsSkillTemplate->m_stConditionArg[nIndex].lArg2)
		nRightResult = pcsSkillTemplate->m_stConditionArg[nIndex].lArg2;
	if (nRightResult > pcsSkillTemplate->m_stConditionArg[nIndex].lArg3)
		nRightResult = pcsSkillTemplate->m_stConditionArg[nIndex].lArg3;

	if (nRandomNumber < nRightResult)
		return TRUE;

	return FALSE;
}

//		ConditionCheckFriendlyUnit
//	Functions
//		- pcsSkill 의 condition type (AGSMSKILL_CONDITION_FRIENDLY_UNIT) 에 따른 함수를 호출해 condition을 check한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheckFriendlyUnit(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	if (!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdFactor	*pcsAttackFactorResult	= GetOwnerFactorResult(pcsSkill);
	AgpdFactor	*pcsTargetFactorResult	= GetResultFactor(pcsTarget);

	INT32	lAttackRace	= 0;
	INT32	lTargetRace	= 0;

	m_pagpmFactors->GetValue(pcsAttackFactorResult, &lAttackRace, AGPD_FACTORS_TYPE_CHAR_TYPE, AGPD_FACTORS_CHARTYPE_TYPE_RACE);
	m_pagpmFactors->GetValue(pcsTargetFactorResult, &lTargetRace, AGPD_FACTORS_TYPE_CHAR_TYPE, AGPD_FACTORS_CHARTYPE_TYPE_RACE);

	// 동일 race 인지 검사
	if (lAttackRace != lTargetRace)
		return FALSE;

	// target이 PK 상태인지 검사
	//
	//

	return TRUE;
}

//		ConditionCheckMagicAttack
//	Functions
//		- pcsSkill 의 condition type (AGSMSKILL_CONDITION_MAGIC_ATTACK) 에 따른 함수를 호출해 condition을 check한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheckMagicAttack(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	if (!pcsSkill || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate		*pcsSkillTemplate			= (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

	if (!pcsSkillTemplate)
		return FALSE;

	if (pcsSkillTemplate->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MAGIC_ATTACK_USE_FORMULA1)
	{
		// ran (100) > (저항기준 상수A)-(10*(마스터리 포인트-대상의 LV)) 참이면 저항 실패

		INT32	lSkillLevel		= GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			return FALSE;

		INT32	lTargetLevel	= GetModifiedCharLevel(pcsTarget);

		INT32	lRightValue		= (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_RESIST_A][lSkillLevel] -
								  (10 * (lSkillLevel - lTargetLevel));

		INT32	lLeftValue		= (INT32) m_csRandom.randInt(100);

		if (lLeftValue > lRightValue)
			return TRUE;
	}

	if (pcsSkillTemplate->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_MAGIC_ATTACK_USE_FORMULA2)
	{
		// RAN(100)<90-(몬스터_LV-마스터리 포인트)^2
		//	(단 (몬스터_LV-마스터리 포인트<0일 경우는 0으로) 가 참일 때 저항 성공

		INT32	lTargetLevel	= GetModifiedCharLevel(pcsTarget);
		INT32	lSkillLevel		= GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			return FALSE;

		INT32	lTemp			= lTargetLevel - lSkillLevel;
		if (lTemp < 0)
			lTemp = 0;

		INT32	lRightValue		= (INT32) 90 - (INT32)pow((double)lTemp, (double)2);

		INT32	lLeftValue		= (INT32) m_csRandom.randInt(100);

		if (lLeftValue < lRightValue)
			return TRUE;
	}

	/*
	INT32	lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	INT32	lTargetLevel = GetModifiedCharLevel(pcsTarget);

	if (pcsSkillTemplate->m_stConditionArg[nIndex].lArg3 == 0 &&
		pcsSkillTemplate->m_stConditionArg[nIndex].lArg4 == 0)
	{
		if ((lTargetLevel - lSkillLevel) < 0)
		{
			lSkillLevel = 0;
			lTargetLevel = 0;
		}
	}

	INT16	lRightResult = (INT16) (pcsSkillTemplate->m_stConditionArg[nIndex].lArg1 
							- (pow((lTargetLevel - lSkillLevel), pcsSkillTemplate->m_stConditionArg[nIndex].lArg2)));

	if (pcsSkillTemplate->m_stConditionArg[nIndex].lArg3 != 0 &&
		pcsSkillTemplate->m_stConditionArg[nIndex].lArg4 != 0)
	{
		if (lRightResult < pcsSkillTemplate->m_stConditionArg[nIndex].lArg3)
			lRightResult = pcsSkillTemplate->m_stConditionArg[nIndex].lArg3;
		if (lRightResult > pcsSkillTemplate->m_stConditionArg[nIndex].lArg4)
			lRightResult = pcsSkillTemplate->m_stConditionArg[nIndex].lArg4;
	}

	INT16	nRandomNumber = m_csRandom.randInt(100);

	if (nRandomNumber < lRightResult)
		return TRUE;
	*/

	return FALSE;
}

//		ConditionCheckLevel
//	Functions
//		- pcsSkill 의 condition type (AGPMSKILL_CONDITION_LEVEL) 에 따른 함수를 호출해 condition을 check한다.
//			(arg1 <= base level <= arg2)
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheckLevel(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase|| !pcsSkill->m_pcsTemplate || 
		nIndex < 0 || nIndex >= AGPMSKILL_MAX_SKILL_CONDITION)
		return FALSE;

	INT32	lBaseLevel = 0;

	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		lBaseLevel	= m_pagpmCharacter->GetLevel((AgpdCharacter *) pcsSkill->m_pcsBase);
	}

	if (lBaseLevel <= 0)
		return FALSE;
	
	// arg1 <= base level 인지 검사
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg1 > lBaseLevel)
		return FALSE;

	// base level <= arg2 인지 검사
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg2 < lBaseLevel)
		return FALSE;

	return TRUE;
}

/*
//		ConditionCheckKillGuardian
//	Functions
//		- pcsSkill 의 condition type (AGPMSKILL_CONDITION_KILL_GUARDIAN) 에 따른 함수를 호출해 condition을 check한다.
//			(arg2 시간안에 arg1 <= # of kill guardian)
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheckKillGuardian(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || nIndex < 0 || nIndex >= AGPMSKILL_MAX_SKILL_CONDITION)
		return FALSE;

	if (!pcsSkill->m_pcsTemplate) return FALSE;

	if (pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_OBJECT) return FALSE;
	if (pcsTarget->m_eType != APBASE_TYPE_CHARACTER) return FALSE;

	// 몇명을 죽여야 하는지 검사한다.
	INT32	lKillCount = (INT32) ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg1;
	if (lKillCount < 1)
		return TRUE;

	INT32	lPartyMember = 0;

	// 파티에 가입되어 있는 경우 파티원이 몇명인지 얻어온다.
	if (pcsTarget->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgpdPartyADChar	*pcsPartyADChar = m_pagpmParty->GetADCharacter((PVOID) pcsTarget);
		if (pcsPartyADChar)
		{
			if (!pcsPartyADChar->pcsParty)
				pcsPartyADChar->pcsParty = m_pagpmParty->GetParty(pcsPartyADChar->lPID);

			if (pcsPartyADChar->pcsParty)
			{
				lPartyMember = pcsPartyADChar->pcsParty->m_nCurrentMember;
			}
		}
	}

	if (lPartyMember > AGPMPARTY_MIN_PARTY_MEMBER)
		lKillCount += (INT32) ((lPartyMember - AGPMPARTY_MIN_PARTY_MEMBER) * (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg3 / 100.0));

	// 죽인 가디언수가 몇인지 검사한다.

	// 일단 이 사원이 어느 사원인지 알아온다.
	AgpdShrineADObject	*pcsShrineADObject = m_pagpmShrine->GetADObject((ApdObject *) pcsSkill->m_pcsBase);
	if (pcsShrineADObject->m_lShrineID == AP_INVALID_SHRINEID)
		return FALSE;

	AgpdShrine			*pcsShrine	= m_pagpmShrine->GetShrine(pcsShrineADObject->m_lShrineID);
	if (!pcsShrine) return FALSE;

	for (int i = 0; i < AGPMSHRINE_MAX_LEVEL; ++i)
	{
		if (pcsShrine->m_lSkillID[i] == pcsSkill->m_lID)
			break;
	}
	if (i == AGPMSHRINE_MAX_LEVEL) return FALSE;

	// 죽인 가디언 정보를 가져온다.
	AgpdShrineADChar	*pcsShrineADChar = m_pagpmShrine->GetADCharacter((AgpdCharacter *) pcsTarget);

	if (pcsShrineADChar->m_lKillGuardianNum[pcsShrine->m_lID][i] >= lKillCount)
	{
		// 죽인 시간을 체크한다.
		INT32	lLastKillTimeIndex = pcsShrineADChar->m_lKillGuardianNum[pcsShrine->m_lID][i];
		if (AGPMSHRINE_MAX_KILL_GUARDIAN <= lLastKillTimeIndex)
			lLastKillTimeIndex = AGPMSHRINE_MAX_KILL_GUARDIAN;

		INT32	lFirstKillTime = pcsShrineADChar->m_ulKillGuardianTime[pcsShrine->m_lID][i][lLastKillTimeIndex - lKillCount];
		INT32	lLastKillTime = pcsShrineADChar->m_ulKillGuardianTime[pcsShrine->m_lID][i][lLastKillTimeIndex - 1];

		if ((lLastKillTime - lFirstKillTime) * 1000 > (INT32) ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg2)
			return FALSE;
	}

	return TRUE;
}
*/

//		ConditionCheckCharType
//	Functions
//		- pcsSkill 의 condition type (AGPMSKILL_CONDITION_CHAR_TYPE) 에 따른 함수를 호출해 condition을 check한다.
//			(arg1 == base race)
//			(arg2 == base gender)
//			(arg3 == base class)
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheckCharType(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || !pcsSkill->m_pcsTemplate || 
		nIndex < 0 || nIndex >= AGPMSKILL_MAX_SKILL_CONDITION)
		return FALSE;

	AgpdFactorCharType	*pcsBaseFactorType = NULL;

	// get base type
	switch (pcsSkill->m_pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			pcsBaseFactorType = 
				(AgpdFactorCharType *) m_pagpmFactors->GetFactor(&((AgpdCharacter *) pcsSkill->m_pcsBase)->m_csFactor, AGPD_FACTORS_TYPE_CHAR_TYPE);
		}
		break;

	default:
		pcsBaseFactorType = NULL;
		break;
	}

	if (!pcsBaseFactorType)
		return FALSE;

	// arg1 == base race
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg1 &&
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg1 != pcsBaseFactorType->lValue[AGPD_FACTORS_CHARTYPE_TYPE_RACE])
		return FALSE;

	// arg2 == base gender
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg2 &&
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg2 != pcsBaseFactorType->lValue[AGPD_FACTORS_CHARTYPE_TYPE_GENDER])
		return FALSE;

	// arg3 == base class
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg3 &&
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg3 != pcsBaseFactorType->lValue[AGPD_FACTORS_CHARTYPE_TYPE_CLASS])
		return FALSE;

	// string arg == character kind
	//if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString &&
	//	((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString[0])
	//{
	//	AgpmCharacterKind	eOwnerKind	= m_pagpmCharacter->GetCharacterKind(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString);
	//	if (eOwnerKind != AGPMCHAR_CHARACTER_KIND_NOT_SPECIFY)
	//	{
	//		AgpmCharacterKind	eTargetKind = m_pagpmCharacter->GetCharacterKind((AgpdCharacter *) pcsTarget);

	//		if (eOwnerKind != eTargetKind)
	//			return FALSE;
	//	}
	//}

	return TRUE;
}

//		ConditionCheckOwnItem
//	Functions
//		- pcsSkill 의 condition type (AGPMSKILL_CONDITION_OWN_ITEM) 에 따른 함수를 호출해 condition을 check한다.
//			(arg_string == 소유하고 있어야 하는 아템 템플릿 이름)
//			(arg1이 0이 아닌경우 아템 소유한 시간이 arg1 보다 작아야 한당)
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheckOwnItem(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || !pcsSkill->m_pcsTemplate || 
		nIndex < 0 || nIndex >= AGPMSKILL_MAX_SKILL_CONDITION)
		return FALSE;

	// arg_string 이름의 아이템 템플릿 아뒤를 가져온당....
	if (!((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString ||
		strlen(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString))
		return FALSE;

	AgpdItemTemplate *pcsItemTemplate = 
		m_pagpmItem->GetItemTemplate(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString);
	if (!pcsItemTemplate)
		return FALSE;

	// base가 위 템플릿의 아템을 가지고 있는지 검사한다.
	AgpdItemADChar	*pcsItemADChar = NULL;
	AgpdGridItem	*pcsAgpdGridItem;

	switch (pcsSkill->m_pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			pcsItemADChar = m_pagpmItem->GetADCharacter((AgpdCharacter *) pcsSkill->m_pcsBase);
		}
		break;

	default:
		break;
	}

	if (!pcsItemADChar)
		return FALSE;

	AgpdItem *pcsItem = NULL;

	// inventory 에서 살펴본다.
	pcsAgpdGridItem = m_pagpmGrid->GetItemByTemplate(&pcsItemADChar->m_csInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM, pcsItemTemplate->m_lID);
	if (pcsAgpdGridItem)
	{
		pcsItem = m_pagpmItem->GetItem(pcsAgpdGridItem->m_lItemID);
	}

	if (!pcsItem)
	{
		// equip slot 을 살펴본다.
		for (int i = 0; i < AGPMITEM_PART_NUM; ++i)
		{
			pcsAgpdGridItem = m_pagpmItem->GetEquipItem( (AgpdCharacter *) pcsSkill->m_pcsBase, i );

			if ( pcsAgpdGridItem != NULL )
			{
				AgpdItem *pcsItem = m_pagpmItem->GetItem(pcsAgpdGridItem->m_lItemID);
				if (pcsItem)
				{
					if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lID == pcsItemTemplate->m_lID)
						return TRUE;
				}
			}
		}
	}

	if (pcsItem)
	{
		// 아템을 소유한 시간이 언제인지 살펴본다.

		// arg1가 0인 경우는 체크할 필요가 없는 경우이다.
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg1 <= 0)
			return TRUE;

		AgsdItem *pcsAgsdItem = m_pagsmItem->GetADItem(pcsItem);
		if (!pcsAgsdItem)
			return FALSE;

		if ((GetClockCount() - pcsAgsdItem->m_ulOwnTime) <= (UINT32)((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg1)
			return TRUE;
	}

	return FALSE;
}

//		ConditionCheckPartyMember
//	Functions
//		- pcsSkill 의 condition type (AGPMSKILL_CONDITION_PARTY_MEMBER) 에 따른 함수를 호출해 condition을 check한다.
//			(arg1 <= # of party member)
//		- AGPMSKILL_CONDITION_SHOUT_WORD 이 세팅되어 있다면 파티원들의 소리친것도 검사한다.
//			(arg2 : 시간범위, arg_string : 소리칠 말)
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- nIndex	: 처리할 condition index
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ConditionCheckPartyMember(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nIndex)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || !pcsSkill->m_pcsTemplate || 
		nIndex < 0 || nIndex >= AGPMSKILL_MAX_SKILL_CONDITION)
		return FALSE;

	// 제대로 된 조건인지 검사한다.
	// shout word 가 세팅되어 있는데 실제 그 단어가 세팅되어 있지 않은경우가 있는지 검사한다.
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_SHOUT_WORD)
	{
		if (!((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString ||
			!strlen(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString))
			return FALSE;
	}

	// 파티에 들어있는지 검사한다.
	AgpdParty	*pcsParty = NULL;

	switch (pcsSkill->m_pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			AgpdPartyADChar *pcsPartyADChar = m_pagpmParty->GetADCharacter((AgpdCharacter *) pcsSkill->m_pcsBase);

			if (pcsPartyADChar->pcsParty)
				pcsParty = pcsPartyADChar->pcsParty;
			else
			{
				pcsParty = m_pagpmParty->GetParty(pcsPartyADChar->lPID);
				if (pcsParty)
					pcsPartyADChar->pcsParty = pcsParty;
			}
		}
		break;

	default:
		break;
	}

	if (!pcsParty)
		return FALSE;

	// 파티 멤버가 총 몇명인지 검사한다.
	//	(arg1 <= # of party member)
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].lArg1 > pcsParty->m_nCurrentMember)
		return FALSE;

	// 파티 멤버중 shrine 내에 들어와 있는 넘이 몇명인지 검사한다.
	//
	//
	//	// 만약 AGPMSKILL_CONDITION_SHOUT_WORD 이값이 세팅되어 있다면 shrine 내에 들어와있는 파티멤버들이
	//	// 정해진 시간 (arg2) 안에 arg_string 값을 소리쳤는지 검사한다.
	//	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lConditionType[nIndex] & AGPMSKILL_CONDITION_SHOUT_WORD)
	//	{
	//		if (!((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString ||
	//			!strlen(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stConditionArg[nIndex].szArgString))
	//			return FALSE;
	//
	//		// arg_string 값을 소리쳤는지 검사한다
	//		//
	//		//
	//		//
	//		//
	//		//
	//		//
	//	}
	//
	//
	//
	//

	return TRUE;
}

// 3. calculate & apply skill cost
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::CostApply(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lCostType & AGPMSKILL_COST_HP)
		CostConsumeHP(pcsSkill);
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lCostType & AGPMSKILL_COST_MP)
		CostConsumeMP(pcsSkill);
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lCostType & AGPMSKILL_COST_SP)
		CostConsumeSP(pcsSkill);
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lCostType & AGPMSKILL_COST_ARROW)
		CostConsumeArrow(pcsSkill);

	return TRUE;
}

BOOL AgsmSkill::CostConsumeHP(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	if (pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return TRUE;

	if (!m_pagpmCharacter->IsPC((AgpdCharacter *) pcsSkill->m_pcsBase))
		return TRUE;

	INT32	lCostHP	= m_pagpmSkill->GetCostHP(pcsSkill);
	if (lCostHP <= 0)
		return TRUE;

	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgpdFactor csCalcFactor;

		AgpdFactor *pcsResult = (AgpdFactor *) m_pagpmFactors->SetFactor(&csCalcFactor, NULL, AGPD_FACTORS_TYPE_RESULT);
		if (!pcsResult)
		{
			m_pagpmFactors->DestroyFactor(&csCalcFactor);
			return FALSE;
		}

		AgpdFactorCharPoint *pcsFactorCharPoint = (AgpdFactorCharPoint *) m_pagpmFactors->InitCalcFactor(pcsResult, AGPD_FACTORS_TYPE_CHAR_POINT);
		if (!pcsFactorCharPoint)
		{
			m_pagpmFactors->DestroyFactor(&csCalcFactor);
			return FALSE;
		}

		m_pagpmFactors->SetValue(&csCalcFactor, (-lCostHP), AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);

		PVOID	pvFactorPacket = m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsSkill->m_pcsBase)->m_csFactor, &csCalcFactor, TRUE, TRUE, TRUE, FALSE);

		m_pagpmFactors->DestroyFactor(&csCalcFactor);

		if (pvFactorPacket)
		{
			m_pagsmCharacter->SendPacketFactor(pvFactorPacket, (AgpdCharacter *) pcsSkill->m_pcsBase, PACKET_PRIORITY_4);

			m_pagpmFactors->m_csPacket.FreePacket(pvFactorPacket);
		}
	}	

	return TRUE;
}

BOOL AgsmSkill::CostConsumeMP(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	if (pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return TRUE;

	if (!m_pagpmCharacter->IsPC((AgpdCharacter *) pcsSkill->m_pcsBase))
		return TRUE;

	INT32	lCostMP	= m_pagpmSkill->GetCostMP(pcsSkill);
	if (lCostMP <= 0)
		return TRUE;

	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgpdFactor csCalcFactor;

		AgpdFactor *pcsResult = (AgpdFactor *) m_pagpmFactors->SetFactor(&csCalcFactor, NULL, AGPD_FACTORS_TYPE_RESULT);
		if (!pcsResult)
		{
			m_pagpmFactors->DestroyFactor(&csCalcFactor);
			return FALSE;
		}

		AgpdFactorCharPoint *pcsFactorCharPoint = (AgpdFactorCharPoint *) m_pagpmFactors->InitCalcFactor(pcsResult, AGPD_FACTORS_TYPE_CHAR_POINT);
		if (!pcsFactorCharPoint)
		{
			m_pagpmFactors->DestroyFactor(&csCalcFactor);
			return FALSE;
		}

		m_pagpmFactors->SetValue(&csCalcFactor, (-lCostMP), AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_MP);

		PVOID	pvFactorPacket = m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsSkill->m_pcsBase)->m_csFactor, &csCalcFactor, TRUE, TRUE, TRUE, FALSE);

		m_pagpmFactors->DestroyFactor(&csCalcFactor);

		if (pvFactorPacket)
		{
			m_pagsmCharacter->SendPacketFactor(pvFactorPacket, (AgpdCharacter *) pcsSkill->m_pcsBase, PACKET_PRIORITY_4);

			m_pagpmFactors->m_csPacket.FreePacket(pvFactorPacket);
		}
	}	

	return TRUE;
}

BOOL AgsmSkill::CostConsumeSP(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	if (pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return TRUE;

	if (!m_pagpmCharacter->IsPC((AgpdCharacter *) pcsSkill->m_pcsBase))
		return TRUE;

	INT32	lCostSP	= m_pagpmSkill->GetCostSP(pcsSkill);
	if (lCostSP <= 0)
		return TRUE;

	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgpdFactor csCalcFactor;

		AgpdFactor *pcsResult = (AgpdFactor *) m_pagpmFactors->SetFactor(&csCalcFactor, NULL, AGPD_FACTORS_TYPE_RESULT);
		if (!pcsResult)
		{
			m_pagpmFactors->DestroyFactor(&csCalcFactor);
			return FALSE;
		}

		AgpdFactorCharPoint *pcsFactorCharPoint = (AgpdFactorCharPoint *) m_pagpmFactors->InitCalcFactor(pcsResult, AGPD_FACTORS_TYPE_CHAR_POINT);
		if (!pcsFactorCharPoint)
		{
			m_pagpmFactors->DestroyFactor(&csCalcFactor);
			return FALSE;
		}

		m_pagpmFactors->SetValue(&csCalcFactor, (-lCostSP), AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_SP);

		PVOID	pvFactorPacket = m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsSkill->m_pcsBase)->m_csFactor, &csCalcFactor, TRUE, TRUE, TRUE, FALSE);

		m_pagpmFactors->DestroyFactor(&csCalcFactor);

		if (pvFactorPacket)
		{
			m_pagsmCharacter->SendPacketFactor(pvFactorPacket, (AgpdCharacter *) pcsSkill->m_pcsBase, PACKET_PRIORITY_4);

			m_pagpmFactors->m_csPacket.FreePacket(pvFactorPacket);
		}
	}	

	return TRUE;
}

BOOL AgsmSkill::CostConsumeArrow(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	if (pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return TRUE;

	if (!m_pagpmCharacter->IsPC((AgpdCharacter *) pcsSkill->m_pcsBase))
		return TRUE;

	INT32	lCostArrow	= m_pagpmSkill->GetCostArrow(pcsSkill);
	if (lCostArrow <= 0)
		return TRUE;

	if (m_pagpmItem->IsEquipBow((AgpdCharacter *) pcsSkill->m_pcsBase))
	{
		AgpdItem	*pcsItem	= NULL;

		do {
			pcsItem	= m_pagpmItem->GetArrowItemFromInventory((AgpdCharacter *) pcsSkill->m_pcsBase);
			if (!pcsItem)
				return FALSE;

			if (lCostArrow > pcsItem->m_nCount)
			{
				m_pagpmItem->SubItemStackCount(pcsItem, pcsItem->m_nCount);

				lCostArrow	-= pcsItem->m_nCount;
			}
			else
			{
				m_pagpmItem->SubItemStackCount(pcsItem, lCostArrow);

				lCostArrow	= 0;
			}
		} while (lCostArrow > 0);
	}
	else if (m_pagpmItem->IsEquipCrossBow((AgpdCharacter *) pcsSkill->m_pcsBase))
	{
		AgpdItem	*pcsItem	= NULL;

		do {
			pcsItem	= m_pagpmItem->GetBoltItemFromInventory((AgpdCharacter *) pcsSkill->m_pcsBase);
			if (!pcsItem)
				return FALSE;

			if (lCostArrow > pcsItem->m_nCount)
			{
				m_pagpmItem->SubItemStackCount(pcsItem, pcsItem->m_nCount);

				lCostArrow	-= pcsItem->m_nCount;
			}
			else
			{
				m_pagpmItem->SubItemStackCount(pcsItem, lCostArrow);

				lCostArrow	= 0;
			}
		} while (lCostArrow > 0);
	}
	else
		return FALSE;

	return TRUE;
}


// 2. 2. check and process that pcsSkill is already buffed
///////////////////////////////////////////////////////////////////////////////

//		ProcessAlreadyBuffedSkill
//	Functions
//		- pcsTarget에 이미 pcsSkill이 버프되어 있는지 검사한다.
//		  버프되어 있다면... 마스터리 포인트를 비교한다.
//			1. pcsSkill의 마스터리 포인트가 이미 버프된넘 포인트보다 더 큰 경우 (혹은 같은 경우)
//					: 기존 버프된 스킬을 없애고 pcsSkill을 새로 캐스트하면 된다.
//			2. 기존 버프된 넘 마스터리 포인트가 더 큰 경우
//					: pcsSkill은 그 효력을 발휘할 수 없다. FALSE를 걍 리턴한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//	Return value
//		- BOOL : check result (FALSE라면 pcsSkill은 효과를 적용할 수 없는경우이다)
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ProcessAlreadyBuffedSkill(AgpdSkill *pcsSkill, ApBase *pcsTarget)
{
	PROFILE("AgsmSkill::ProcessAlreadyBuffedSkill");

	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsTarget)
		return FALSE;

	// 버프된 스킬들중 pcsSkill과 템플릿이 같은 넘이 있는지 검사한다.

	AgsdSkillADBase	*pcsADBase	= GetADBase(pcsTarget);
	if (!pcsADBase) return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	INT32	lBuffedIndex	= (-1);
	BOOL	bClassifyChange = FALSE;
	BOOL	bUnionFind = FALSE;

	////////////////////////////////////////////////////////////////////////
	// 2005.01.25. steeple
	// 버프 등급이 들어가면서 메인 버프 효과가 같은 스킬도 찾아야 한다.

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;

	INT32 lBuffedSkillLevel = 0;
	AgpdSkillTemplate* pcsBuffedSkillTemplate = NULL;

	INT32 lBuffedSkillScrollID = 0;
	AgsdSkill* pcsBuffedAgsdSkill = NULL;

	// 2008.03.07. steeple
	// 먼저 같은 TID 부터 찾는다. 같은 TID 는 모든 것을 우선한다.
	for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate)
			break;

		pcsBuffedAgsdSkill = GetADSkill(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);
		if(!pcsBuffedAgsdSkill)
			break;

		lBuffedSkillLevel = GetModifiedSkillLevel(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);
		if(!lBuffedSkillLevel)
			continue;

		pcsBuffedSkillTemplate = (AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate;
		lBuffedSkillScrollID = pcsBuffedAgsdSkill->m_lSkillScrollIID;

		if (pcsBuffedSkillTemplate->m_lID == pcsSkillTemplate->m_lID)
		{
			lBuffedIndex	= i;
			break;
		}
	}

	// 2008.03.07. steeple
	// 같은 TID 의 스킬이 없을 때 아래 Search 를 해준다.
	if(lBuffedIndex == -1)
	{
		for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
		{
			if (!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill ||
				!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate)
				break;

			pcsBuffedAgsdSkill = GetADSkill(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);
			if(!pcsBuffedAgsdSkill)
				break;

			lBuffedSkillLevel = GetModifiedSkillLevel(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);
			if(!lBuffedSkillLevel)
				continue;

			pcsBuffedSkillTemplate = (AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate;
			lBuffedSkillScrollID = pcsBuffedAgsdSkill->m_lSkillScrollIID;

			//  같은 Classify 찾기
			if (pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lBuffedSkillLevel] != 0 &&
				pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lSkillLevel] != 0 &&
				pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lBuffedSkillLevel] ==
				pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_ID][lSkillLevel])
			{
				lBuffedIndex = i;
				bClassifyChange = TRUE;
				break;
			}

			// 2007.10.26. steeple
			// Union Skill Type Check
			if(m_pagpmSkill->IsSkillUnionType1(pcsSkillTemplate) && m_pagpmSkill->IsSkillUnionType1(pcsBuffedSkillTemplate) &&
				pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_SKILL_UNION][lSkillLevel] == 
				pcsBuffedSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_SKILL_UNION][lSkillLevel])
			{
				lBuffedIndex = i;
				bUnionFind = TRUE;
				break;
			}
		}
	}

	// 2005.01.25. steeple
	// 버프 등급의 변화도 비교해야한다.
	if (lBuffedIndex > (-1))
	{
		// Sckill Scroll 에 의한 것일 수도 있으니 일단 해당 아이템을 구해보자.
		// 아래 아이템은 NULL 일 수 있다.
		AgpdItem* pcsItem = m_pagpmItem->GetItem(lBuffedSkillScrollID);
		// 2006.01.23. steeple
		// UnuseItem 에서 콜백타고 와서 Buff 를 지우기 때문에, 중첩되서 지우지 않게끔 한다.
		BOOL bUnuseItem = FALSE;

		// 그냥 레벨만 비교
		if(!bClassifyChange)
		{
			// 2007.10.26. steeple
			// Union Skill 이라면 Max 치 계산해서 넘는다면 날린다.
			if(bUnionFind && m_pagpmSkill->IsSkillUnionType1(pcsSkillTemplate))
			{
				INT32 lUnionResult = ProcessUnionSkill(pcsSkill, pcsTarget, lBuffedIndex);
				if(lUnionResult == 2)
				{
					// 아이템이 사용중이라면 중지. 2006.01.05. steeple
					if(pcsItem && pcsItem->m_nInUseItem && pcsItem->m_pcsItemTemplate && pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE)
						bUnuseItem = m_pagsmItem->UnuseItem(pcsItem);

					if(!bUnuseItem)
						EndBuffedSkill(pcsTarget, lBuffedIndex, TRUE, _T("ProcessAlreadyBuffedSkill:: ProcessUnionSkill returned 2"));
				}
				else if(lUnionResult == 0)
				{
					// 뭔가 이상한 에러다.
					ASSERT(!"AgsmSkill::ProcessAlreadyBuffedSkill(..) ProcessUnionSkill invoked an abnormal error");
					return FALSE;
				}
			}

			// Level 은 부등호로 비교 2005.09.15. steeple
			else if(lSkillLevel > lBuffedSkillLevel)
			{
				// 아이템이 사용중이라면 중지. 2006.01.05. steeple
				if(pcsItem && pcsItem->m_nInUseItem && pcsItem->m_pcsItemTemplate && pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE)
					bUnuseItem = m_pagsmItem->UnuseItem(pcsItem);

				// 기존에 버프된 스킬을 없애버린다.
				if(!bUnuseItem)
					EndBuffedSkill(pcsTarget, lBuffedIndex, TRUE, _T("ProcessAlreadyBuffedSkill:: lSkillLevel > lBuffedSkillLevel"));
			}
			else if(lSkillLevel == lBuffedSkillLevel)
			{
				// 2005.09.15. steeple
				// 레벨에 같을 때 지속시간이 무한대인건 굳이 이전 걸 안없애도 된다.
				if(m_pagpmSkill->IsDurationByDistanceSkill(pcsBuffedSkillTemplate) ||
					m_pagpmSkill->IsDurationUnlimited(pcsBuffedSkillTemplate))
				{
					// 2005.09.15. steeple
					pcsAgsdSkill->m_eMissedReason = AGSDSKILL_MISSED_REASON_DURATION_UNLIMITED;
					return FALSE;
				}
				else
				{
					// 아이템이 사용중이라면 중지. 2006.01.05. steeple
					if(pcsItem && pcsItem->m_nInUseItem && pcsItem->m_pcsItemTemplate && pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE)
						bUnuseItem = m_pagsmItem->UnuseItem(pcsItem);

					if(!bUnuseItem)
						EndBuffedSkill(pcsTarget, lBuffedIndex, TRUE, _T("ProcessAlreadyBuffedSkill:: lSkillLevel == lBuffedSkillLevel and NormalSkill"));
				}
			}
			else
			{
				// 2006.01.17. steeple
				// 만약에 Attack Damage 를 주는 스킬이라면, 데미지만 주는 스킬로 세팅해놓는다.
				if(m_pagpmSkill->IsAttackSkill(pcsSkillTemplate))
				{
					SetAttackDamageOnly(pcsSkill);
					return TRUE;
				}

				return FALSE;
			}
		}
		// 버프 등급 비교
		else if(pcsBuffedSkillTemplate)
		{
			if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_LEVEL][lSkillLevel] >
				pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_LEVEL][lBuffedSkillLevel])
			{
				// 아이템이 사용중이라면 중지. 2006.01.05. steeple
				if(pcsItem && pcsItem->m_nInUseItem && pcsItem->m_pcsItemTemplate && pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE)
					bUnuseItem = m_pagsmItem->UnuseItem(pcsItem);

				// 기존에 버프된 스킬을 없애버린다.
				if(!bUnuseItem)
					EndBuffedSkill(pcsTarget, lBuffedIndex, TRUE, _T("ProcessAlreadyBuffedSkill:: BuffLevelCompare - 1"));
			}
			else if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_LEVEL][lSkillLevel] ==
				pcsBuffedSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_CLASSIFY_LEVEL][lBuffedSkillLevel])
			{
				// 2005.09.15. steeple
				// 레벨에 같을 때 지속시간이 무한대인건 굳이 이전 걸 안없애도 된다.
				if(m_pagpmSkill->IsDurationByDistanceSkill(pcsBuffedSkillTemplate) ||
					m_pagpmSkill->IsDurationUnlimited(pcsBuffedSkillTemplate))
				{
					// 2005.09.15. steeple
					pcsAgsdSkill->m_eMissedReason = AGSDSKILL_MISSED_REASON_DURATION_UNLIMITED;
					return FALSE;
				}
				else
				{
					// 아이템이 사용중이라면 중지. 2006.01.05. steeple
					if(pcsItem && pcsItem->m_nInUseItem && pcsItem->m_pcsItemTemplate && pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE)
						bUnuseItem = m_pagsmItem->UnuseItem(pcsItem);

					if(!bUnuseItem)
						EndBuffedSkill(pcsTarget, lBuffedIndex, TRUE, _T("ProcessAlreadyBuffedSkill:: BuffLevelCompare - 2"));
				}
			}
			else
			{
				// 2006.01.17. steeple
				// 만약에 Attack Damage 를 주는 스킬이라면, 데미지만 주는 스킬로 세팅해놓는다.
				if(m_pagpmSkill->IsAttackSkill(pcsSkillTemplate))
				{
					SetAttackDamageOnly(pcsSkill);
					return TRUE;
				}

				return FALSE;
			}
		}
	}	// lBuffedSIndex > -1

	return TRUE;
}

// 2007.10.26. steeple
// Union Skill 의 사용여부를 체크해서 리턴한다.
// return 1 이면 그냥 추가로 사용해주면 되고,
// return 2 이면 lBuffedIndex 의 BuffedSkill 을 해제 하여준다.
// return 0 이면 뭔가 이상한 에러다.
INT32 AgsmSkill::ProcessUnionSkill(AgpdSkill* pcsSkill, ApBase* pcsTarget, INT32 lBuffedIndex)
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsTarget || lBuffedIndex < 0 || lBuffedIndex >= AGPMSKILL_MAX_SKILL_BUFF)
		return 0;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsTarget);
	if(!pcsADBase)
		return 0;

	AgpdSkill* pcsBuffedSkill = pcsADBase->m_csBuffedSkillProcessInfo[lBuffedIndex].m_pcsTempSkill;
	if(!pcsBuffedSkill || !pcsBuffedSkill->m_pcsTemplate)
		return 0;

	AgpdSkillTemplate* pcsSkillTemplate = static_cast<AgpdSkillTemplate*>(pcsSkill->m_pcsTemplate);
	AgpdSkillTemplate* pcsBuffedSkillTemplate = static_cast<AgpdSkillTemplate*>(pcsBuffedSkill->m_pcsTemplate);
	
	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return 0;

	INT32 lBuffedSkillLevel = GetModifiedSkillLevel(pcsBuffedSkill);
	if(!lBuffedSkillLevel)
		return 0;

	INT32 lUnionID = (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_SKILL_UNION][lSkillLevel];
	INT32 lBuffedUnionID = (INT32)pcsBuffedSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_SKILL_UNION][lSkillLevel];
	if(lUnionID == 0 || lBuffedUnionID == 0 || lUnionID != lBuffedUnionID)
		return 0;

	// 해당 UnionID 로 몇 개나 현재 사용중인지 찾아본다.
	UnionControlArray::iterator iter = std::find(pcsADBase->m_UnionControlArray.m_astUnionControlInfo.begin(),
												pcsADBase->m_UnionControlArray.m_astUnionControlInfo.end(),
												lUnionID);
	if(iter == pcsADBase->m_UnionControlArray.m_astUnionControlInfo.end())
		return 1;

	// 있다면
	if(iter->m_lCurrentCount >= iter->m_lMaxCount)
		return 2;

	return 1;
}

//		UpdateFactorPermanently
//	Functions
//		- pcsSkill template의 UsedConstFactor에서 eConstIndex 값을 pcsTarget에 영구적으로 올려준다.
//			1. target의 base factor를 올려준다.
//			2. target의 point(혹은 percent) factor에도 같은 양을 더해준다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- pcsTarget : target base pointer
//		- eConstIndex	:
//	Return value
//		- BOOL : result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::UpdateFactorPermanently(AgpdSkill *pcsSkill, ApBase *pcsTarget, eAgpmSkillUsedConstFactor eConstIndex)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsTarget || eConstIndex > AGPMSKILL_CONST_PERCENT_END || eConstIndex < 0)
		return FALSE;

	AgpdSkillTemplate *pcsSkillTemplate = (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

	// target level을 가져온다.
	INT32	lTargetLevel	= 0;

	// target 의 펙터를 가져온다.
	AgpdFactor	*pcsFactor = NULL;
	AgpdFactor	*pcsFactorPoint = NULL;
	AgpdFactor	*pcsFactorPercent = NULL;

	switch (pcsTarget->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			pcsFactor = &((AgpdCharacter *) pcsTarget)->m_csFactor;
			pcsFactorPoint = &((AgpdCharacter *) pcsTarget)->m_csFactorPoint;
			pcsFactorPercent = &((AgpdCharacter *) pcsTarget)->m_csFactorPercent;

			lTargetLevel	= m_pagpmCharacter->GetLevel((AgpdCharacter *) pcsTarget);
		}
		break;

	default:
		break;
	}

	if (lTargetLevel < 1 || !pcsFactor)
		return FALSE;

	// 올려줄 펙터 값을 가져온다.
	if (pcsSkillTemplate->m_fUsedConstFactor[eConstIndex][lTargetLevel] == 0 || pcsSkillTemplate->m_fUsedConstFactor2[eConstIndex][lTargetLevel] == 0)
		return FALSE;

	AgpdFactorCharStatus	*pcsFactorTemp  = (AgpdFactorCharStatus *) m_pagpmFactors->SetFactor(pcsFactor, NULL, g_alFactorTable[eConstIndex][0]);
	AgpdFactorCharStatus	*pcsFactorPointTemp = NULL;

	BOOL	bIsPointFactor	= FALSE;
	if (eConstIndex < AGPMSKILL_CONST_POINT_END)
	{
		pcsFactorPointTemp  = (AgpdFactorCharStatus *) m_pagpmFactors->SetFactor(pcsFactorPoint, NULL, g_alFactorTable[eConstIndex][0]);
		bIsPointFactor	= TRUE;
	}
	else
		pcsFactorPointTemp  = (AgpdFactorCharStatus *) m_pagpmFactors->SetFactor(pcsFactorPercent, NULL, g_alFactorTable[eConstIndex][0]);

	if (!pcsFactorTemp || !pcsFactorPointTemp)
		return FALSE;

	// 2007.10.25. steeple
	BOOL bCheckConnection = TRUE;
	if(CheckConnectionInfo(pcsSkill, pcsTarget))
		bCheckConnection = TRUE;

	if (g_alFactorTable[eConstIndex][0] == AGPD_FACTORS_TYPE_DAMAGE || g_alFactorTable[eConstIndex][0] == AGPD_FACTORS_TYPE_DEFENSE)
	{
		FLOAT	fPrevValue	= 0.0f;
		FLOAT	fConstValue = pcsSkillTemplate->m_fUsedConstFactor[eConstIndex][lTargetLevel];
		if(bCheckConnection)
			fConstValue += pcsSkillTemplate->m_fUsedConstFactor2[eConstIndex][lTargetLevel];

		m_pagpmFactors->GetValue(pcsFactor, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1], g_alFactorTable[eConstIndex][2]);
		m_pagpmFactors->SetValue(pcsFactor, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1], g_alFactorTable[eConstIndex][2]);

		fPrevValue	= 0.0f;

		if (bIsPointFactor)
		{
			m_pagpmFactors->GetValue(pcsFactorPoint, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1], g_alFactorTable[eConstIndex][2]);
			m_pagpmFactors->SetValue(pcsFactorPoint, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1], g_alFactorTable[eConstIndex][2]);
		}
		else
		{
			m_pagpmFactors->GetValue(pcsFactorPercent, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1], g_alFactorTable[eConstIndex][2]);
			m_pagpmFactors->SetValue(pcsFactorPercent, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1], g_alFactorTable[eConstIndex][2]);
		}
	}
	else
	{
		FLOAT	fPrevValue	= 0.0f;
		FLOAT	fConstValue = pcsSkillTemplate->m_fUsedConstFactor[eConstIndex][lTargetLevel];
		if(bCheckConnection)
			fConstValue += pcsSkillTemplate->m_fUsedConstFactor2[eConstIndex][lTargetLevel];

		m_pagpmFactors->GetValue(pcsFactor, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1]);
		m_pagpmFactors->SetValue(pcsFactor, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1]);

		if (bIsPointFactor)
		{
			m_pagpmFactors->GetValue(pcsFactorPoint, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1]);

			// AttackRange 는 Factor 에서 cm 단위로 저장된다. Skill_Const 에서는 m 단위로 온다. 2006.03.28. steeple
			if (g_alFactorTable[eConstIndex][1] == AGPD_FACTORS_ATTACK_TYPE_ATTACKRANGE)
				m_pagpmFactors->SetValue(pcsFactorPoint, fPrevValue + (fConstValue * 100.0f), (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1]);
			else
				m_pagpmFactors->SetValue(pcsFactorPoint, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1]);
		}
		else
		{
			m_pagpmFactors->GetValue(pcsFactorPercent, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1]);
			m_pagpmFactors->SetValue(pcsFactorPercent, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[eConstIndex][0], g_alFactorTable[eConstIndex][1]);
		}
	}

	return TRUE;
}

// 4. end skill effect functions
///////////////////////////////////////////////////////////////////////////////

//		EndSkillEffect
//	Functions
//		- pcsSkill 의 skill end effect type에 따른 함수를 호출해 실제 스킬 effect를 적용한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- csTarget : target base
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::EndSkillEffect(AgpdSkill *pcsSkill, ApBase *pcsTarget)
{
	if (!pcsSkill)
		return FALSE;

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEndEffectType & AGPMSKILL_ENDSKILL_CONSUME_HP)
		EndSkillEffectConsumeHP(pcsSkill, pcsTarget);

	return TRUE;
}

//		EndSkillEffectConsumeHP
//	Functions
//		- pcsSkill 의 skill end effect type에 따른 함수를 호출해 실제 스킬 effect를 적용한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- csTarget : target base
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::EndSkillEffectConsumeHP(AgpdSkill *pcsSkill, ApBase* pcsTarget)
{
	if (!pcsSkill || !pcsTarget)
		return FALSE;

	if (pcsTarget->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgpdCharacter	*pcsCharacter	= (AgpdCharacter *) pcsTarget;

		INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			return FALSE;

		INT32	lCostHP	= (INT32)((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_ENDSKILL_COST_HP][lSkillLevel];
		INT32	lCurrentHP	= 0;

		AgpdFactor	*pcsResultFactor = (AgpdFactor *) m_pagpmFactors->GetFactor(&pcsCharacter->m_csFactor, AGPD_FACTORS_TYPE_RESULT);
		if (!pcsResultFactor)
			return FALSE;

		m_pagpmFactors->GetValue(pcsResultFactor, &lCurrentHP, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);
		if (lCurrentHP == 0)
			return TRUE;

		// 1이하로는 줄지 않게 한다.
		if (lCostHP > lCurrentHP - 1)
			lCostHP = lCurrentHP - 1;

		PVOID			pvPacket		= NULL;

		m_pagsmFactors->UpdateCharPoint(&pcsCharacter->m_csFactor, &pvPacket, (lCurrentHP - lCostHP), 0, 0);

		if (pvPacket)
		{
			m_pagsmCharacter->SendPacketFactor(pvPacket, pcsCharacter, PACKET_PRIORITY_4);

			m_pagpmFactors->m_csPacket.FreePacket(pvPacket);
		}
	}

	return TRUE;
}

//		EndDefenseSkill
//	Functions
//		- pcsSkill 의 skill end effect type에 따른 함수를 호출해 실제 스킬 effect를 적용한다.
//	Arguments
//		- pcsSkill	: 처리할 AgpdSkill data pointer
//		- csTarget : target base
//	Return value
//		- BOOL : check result
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::EndDefenseSkill(AgpdSkill *pcsSkill, ApBase* pcsTarget)
{
	if (!pcsSkill || !pcsTarget)
		return FALSE;

	AgsdSkill			*pcsAgsdSkill	= GetADSkill(pcsSkill);
	if (!pcsAgsdSkill) return FALSE;

	AgpdSkillAttachData	*pcsAttachData	= m_pagpmSkill->GetAttachSkillData(pcsTarget);
	if (!pcsAttachData) return FALSE;

	if (pcsAgsdSkill->m_nMeleeReflectPoint != 0)
	{
		pcsAttachData->m_nMeleeReflectPoint -= pcsAgsdSkill->m_nMeleeReflectPoint;
		if (pcsAttachData->m_nMeleeReflectPoint <= 0 && pcsAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_REFLECT_MELEE_ATTACK)
		{
			pcsAttachData->m_nDefenseTypeEffect -= AGPMSKILL_EFFECT_REFLECT_MELEE_ATTACK;
		}
	}

	if (pcsAgsdSkill->m_nMagicReflectPoint != 0)
	{
		pcsAttachData->m_nMagicReflectPoint -= pcsAgsdSkill->m_nMagicReflectPoint;
		if (pcsAttachData->m_nMagicReflectPoint <= 0 && pcsAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_REFLECT_MAGIC_ATTACK)
		{
			pcsAttachData->m_nDefenseTypeEffect -= AGPMSKILL_EFFECT_REFLECT_MAGIC_ATTACK;
		}
	}

	if (pcsAgsdSkill->m_nMeleeDefensePoint != 0)
	{
		pcsAttachData->m_nMeleeDefensePoint -= pcsAgsdSkill->m_nMeleeDefensePoint;
		if (pcsAttachData->m_nMeleeDefensePoint <= 0 && pcsAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_DEFENSE_MELEE_ATTACK)
		{
			pcsAttachData->m_nDefenseTypeEffect -= AGPMSKILL_EFFECT_DEFENSE_MELEE_ATTACK;
		}
	}

	if (pcsAgsdSkill->m_nMagicDefensePoint != 0)
	{
		pcsAttachData->m_nMagicDefensePoint -= pcsAgsdSkill->m_nMagicDefensePoint;
		if (pcsAttachData->m_nMagicDefensePoint <= 0 && pcsAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_DEFENSE_MAGIC_ATTACK)
		{
			pcsAttachData->m_nDefenseTypeEffect -= AGPMSKILL_EFFECT_DEFENSE_MAGIC_ATTACK;
		}
	}

	if (pcsAgsdSkill->m_lDamageShield != 0)
	{
		pcsAttachData->m_lDamageShield -= pcsAgsdSkill->m_lDamageShield;
		if (pcsAttachData->m_lDamageShield <= 0 && pcsAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_REFLECT_DAMAGE_SHIELD)
		{
			pcsAttachData->m_nDefenseTypeEffect -= AGPMSKILL_EFFECT_REFLECT_DAMAGE_SHIELD;
		}
	}

	return TRUE;
}

BOOL AgsmSkill::RestoreCombatSkillArg(AgpdSkill *pcsSkill, ApBase* pcsTarget)
{
	if (!pcsSkill || !pcsTarget)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	AgsdSkill			*pcsAgsdSkill	= GetADSkill(pcsSkill);
	if (!pcsAgsdSkill) return FALSE;

	AgpdSkillAttachData	*pcsAttachData	= m_pagpmSkill->GetAttachSkillData(pcsTarget);
	if(!pcsAttachData)
		return FALSE;

	AgsdCharacter* pcsAgsdCharacter = m_pagsmCharacter->GetADCharacter((AgpdCharacter*)pcsTarget);
	if(!pcsAgsdCharacter)
		return FALSE;

	// restore magic critical strike factors
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lMagicCriticalProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicCriticalProbability -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lMagicCriticalProbability;
		if (pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicCriticalProbability < 0)
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicCriticalProbability = 0;
	}
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lMagicDamageAdjustRate > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicDamageAdjustRate -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lMagicDamageAdjustRate;
		if (pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicDamageAdjustRate <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicDamageAdjustRate = 0;
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicCriticalProbability = 0;
			if (pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_MAGIC_ATTACK_CRITICAL)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT_MAGIC_ATTACK_CRITICAL;
		}
	}

	// restore melee critical strike factors
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lMeleeCriticalProbability != 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lMeleeCriticalProbability -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lMeleeCriticalProbability;
	}
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lMeleeDamageAdjustRate != 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lMeleeDamageAdjustRate -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lMeleeDamageAdjustRate;
		if (pcsAttachData->m_stBuffedSkillCombatEffectArg.lMeleeDamageAdjustRate == 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lMeleeDamageAdjustRate = 0;
			if (pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_MELEE_ATTACK_CRITICAL)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT_MELEE_ATTACK_CRITICAL;
		}
	}

	// restore melee attack death strike factor
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDeathProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDeathProbability -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDeathProbability;
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDeathAmount -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDeathAmount;
		if (pcsAttachData->m_stBuffedSkillCombatEffectArg.lDeathProbability <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDeathProbability = 0;
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDeathAmount = 0;
			if (pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_MELEE_ATTACK_DEATH)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT_MELEE_ATTACK_DEATH;
		}

		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDeathProbability = 0;
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDeathAmount = 0;
	}

	// restore counter attack factor
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lCounterAttackProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lCounterAttackProbability -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lCounterAttackProbability;
		if (pcsAttachData->m_stBuffedSkillCombatEffectArg.lCounterAttackProbability <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lCounterAttackProbability = 0;
			if (pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_DEFENSE_COUNTER_ATTACK)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT_DEFENSE_COUNTER_ATTACK;
		}
	}

	// restore weapon damage type
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lWeaponDamageType != 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lWeaponDamageType = AGPMSKILL_WEAPON_DAMAGE_TYPE_NORMAL;
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lWeaponDamageType = AGPMSKILL_WEAPON_DAMAGE_TYPE_NORMAL;

		if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT] & AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT_TYPE2)
			pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT] -= AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT_TYPE2;

		if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT] & AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT_TYPE3)
			pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT] -= AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT_TYPE3;

		if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT] <= 0)
		{
			pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_UPDATE_COMBAT_POINT] = 0;
			pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_UPDATE_COMBAT_POINT;
		}
	}

	// restore convert damage to hp
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0] > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0]  -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0];
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[0] -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[0];
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0] <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0] = 0;
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[0] = 0;
			//if(pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_CONVERT_DAMAGE_TO_HP)
			//	pcsAttachData->m_ullBuffedSkillCombatEffect2 -= AGPMSKILL_EFFECT2_CONVERT_DAMAGE_TO_HP;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] & AGPMSKILL_EFFECT_DETAIL_CONVERT_DAMAGE_TO_HP)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] -= AGPMSKILL_EFFECT_DETAIL_CONVERT_DAMAGE_TO_HP;

			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] = 0;
				pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_CONVERT;
			}
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0] = 0;
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[0] = 0;
	}

	// restore convert damage to mp
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[0] > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[0]  -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[0];
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[0] -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[0];
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[0] <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[0] = 0;
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[0] = 0;
			//if(pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_CONVERT_DAMAGE_TO_MP)
			//	pcsAttachData->m_ullBuffedSkillCombatEffect2 -= AGPMSKILL_EFFECT2_CONVERT_DAMAGE_TO_MP;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] & AGPMSKILL_EFFECT_DETAIL_CONVERT_DAMAGE_TO_MP)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] -= AGPMSKILL_EFFECT_DETAIL_CONVERT_DAMAGE_TO_MP;

			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] = 0;
				pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_CONVERT;
			}
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[0] = 0;
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[0] = 0;
	}

	// 2005.07.12. steeple
	// restore convert attack damage to hp
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[1] > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[1]  -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[1];
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[1] -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[1];
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0] <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[1] = 0;
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[1] = 0;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] & AGPMSKILL_EFFECT_DETAIL_CONVERT_ATK_DAMAGE_TO_HP)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] -= AGPMSKILL_EFFECT_DETAIL_CONVERT_ATK_DAMAGE_TO_HP;

			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] = 0;
				pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_CONVERT;
			}
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[1] = 0;
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[1] = 0;
	}

	// 2005.07.12. steeple
	// restore convert attack damage to mp
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[1] > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[1]  -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[1];
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[1] -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[1];
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[1] <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[1] = 0;
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[1] = 0;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] & AGPMSKILL_EFFECT_DETAIL_CONVERT_ATK_DAMAGE_TO_MP)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] -= AGPMSKILL_EFFECT_DETAIL_CONVERT_ATK_DAMAGE_TO_MP;

			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] = 0;
				pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_CONVERT;
			}
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[1] = 0;
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[1] = 0;
	}

	// restore charge
	// Damage 가 세팅되어 있으면 걸려있는 것이다. 2005.01.25. steeple
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lChargeDamage > 0)
	{
		// Charge, MeleeComboInCharge 를 걍 0 으로 밀어버린다.
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lChargeLevel = AGPMSKILL_CHARGE_NONE;
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lChargeDamage = 0;
		pcsAttachData->m_lMeleeComboInCharge = 0;

		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lChargeLevel = AGPMSKILL_CHARGE_NONE;
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lChargeDamage = 0;

		if(pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_CHARGE)
			pcsAttachData->m_ullBuffedSkillCombatEffect2 -= AGPMSKILL_EFFECT2_CHARGE;
	}

	// restore skill level up
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lSkillLevelUpPoint > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lSkillLevelUpPoint -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lSkillLevelUpPoint;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lSkillLevelUpPoint <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lSkillLevelUpPoint = 0;
			if(pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_SKILL_LEVELUP)
				pcsAttachData->m_ullBuffedSkillCombatEffect2 -= AGPMSKILL_EFFECT2_SKILL_LEVELUP;
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lSkillLevelUpPoint = 0;

		// 2007.02.06. steeple
		// Modified Skill Level 을 다시 계산해준다.
		m_pagpmSkill->UpdateModifiedSkillLevel(pcsTarget);
	}

	// restore char level up - 2007.07.07. steeple
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lCharLevelUpPoint > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lCharLevelUpPoint -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lCharLevelUpPoint;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lCharLevelUpPoint <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lCharLevelUpPoint = 0;
			if(pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_SKILL_LEVELUP)
				pcsAttachData->m_ullBuffedSkillCombatEffect2 -= AGPMSKILL_EFFECT2_SKILL_LEVELUP;
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lCharLevelUpPoint = 0;

		// PC 라면 UpdateCharLevel 불러준다.
		if(m_pagpmCharacter->IsPC((AgpdCharacter*)pcsTarget))
		{
			// 나중에 필요할 때 작업한다.
			//
			//
		}
	}

	// restore damage reflect
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability;
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectAmount -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectAmount;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability = 0;
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectAmount = 0;
			if(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_REFLECT_RECEIVED_DAMAGE)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT_REFLECT_RECEIVED_DAMAGE;
		}

		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability = 0;
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectAmount = 0;
	}

	// restore heroic damage reflect
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicAmount -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicAmount;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicProbability = 0;
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicAmount = 0;
			if(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_REFLECT_RECEIVED_DAMAGE)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT_REFLECT_RECEIVED_DAMAGE;
		}

		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicProbability = 0;
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicAmount = 0;
	}

	// restore damage ignore
	if (pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability = 0;
			if(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_REFLECT_DAMAGE_SHIELD)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT_REFLECT_DAMAGE_SHIELD;
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability = 0;
	}

	// restore action on action type3
	if(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lActionOnActionType3Count > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lActionOnActionType3Count -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lActionOnActionType3Count;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lActionOnActionType3Count <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lActionOnActionType3Count = 0;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE3)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] -= AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE3;

			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] = 0;
				pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_ACTOIN_ON_ACTION;
			}
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lActionOnActionType3Count = 0;
	}

	// restore action on action type4
	if(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lActionOnActionType4Count > 0)
	{
		pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] -= AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE4;
		if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] <= 0)
		{
			pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] = 0;
			pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_ACTOIN_ON_ACTION;
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lActionOnActionType4Count = 0;
	}

	// restore increase max summons count
	if(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lIncreaseSummonsNum > 0)
	{
		// 빼준다.
		if(m_pagpmSummons)
			m_pagpmSummons->AddMaxSummonsCount((AgpdCharacter*)pcsTarget, -(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lIncreaseSummonsNum));
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lIncreaseSummonsNum = 0;

		// Max Count 패킷 보내준다.
		if(m_pagpmSummons && m_pagsmSummons)
			m_pagsmSummons->SendSetMaxSummons((AgpdCharacter*)pcsTarget, m_pagpmSummons->GetMaxSummonsCount((AgpdCharacter*)pcsTarget));
	}

	// restore summons type 5 - 2005.09.18. steeple
	if(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lSummonsType5Count > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lSummonsType5Count -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lSummonsType5Count;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lSummonsType5Count <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lSummonsType5Count = 0;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE5)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_SUMMONS] -= AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE5;

			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_SUMMONS] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_SUMMONS] = 0;
				pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_SUMMONS;
			}
		}
	}

	// restore divide attr 2007.06.26. steeple
	if(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDivideAttrRate > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideAttrRate -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDivideAttrRate;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideAttrRate <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideAttrRate = 0;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_DIVIDE] & AGPMSKILL_EFFECT_DETAIL_DIVIDE_ATTR)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_DIVIDE] -= AGPMSKILL_EFFECT_DETAIL_DIVIDE_ATTR;

			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_DIVIDE] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_DIVIDE] = 0;
				pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_DIVIDE;
			}
		}
	}

	// restore divide normal 2007.06.26. steeple
	if(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDivideNormalRate > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideNormalRate -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDivideNormalRate;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideNormalRate <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDivideNormalRate = 0;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_DIVIDE] & AGPMSKILL_EFFECT_DETAIL_DIVIDE_NORMAL)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_DIVIDE] -= AGPMSKILL_EFFECT_DETAIL_DIVIDE_NORMAL;

			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_DIVIDE] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_DIVIDE] = 0;
				pcsAttachData->m_ullBuffedSkillCombatEffect2 &= ~AGPMSKILL_EFFECT2_DIVIDE;
			}
		}
	}

	// restore ignore physical defense 2008.06.17. iluvs
	if(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lIgnorePhysicalDefence > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lIgnorePhysicalDefence -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lIgnorePhysicalDefence;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lIgnorePhysicalDefence <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lIgnorePhysicalDefence = 0;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] & AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK_IGNORE_PHYSICAL)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] -= AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK_IGNORE_PHYSICAL;
			
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] = 0;
			}
		}
	}

	// restore ignore attribute defense 2008.06.17. iluvs
	if(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lIgnoreAttributeDefence > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lIgnoreAttributeDefence -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lIgnoreAttributeDefence;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lIgnoreAttributeDefence <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lIgnoreAttributeDefence = 0;
			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] & AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK_IGNORE_ATTRIBUTE)
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] -= AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK_IGNORE_ATTRIBUTE;

			if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] <= 0)
			{
				pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] = 0;
			}
		}
	}

	// restore defense critical attack 2008.06.17. iluvs
	if(pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDefenceCriticalAttack > 0)
	{
		pcsAttachData->m_stBuffedSkillCombatEffectArg.lDefenceCriticalAttack -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDefenceCriticalAttack;
		if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDefenceCriticalAttack <= 0)
		{
			pcsAttachData->m_stBuffedSkillCombatEffectArg.lDefenceCriticalAttack = 0;
			if(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_CRITICAL_DEFENCE_PERCENT)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT_CRITICAL_DEFENCE_PERCENT;
		}
		pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lDefenceCriticalAttack = 0;
	}

	// restore normal attack invincible - 2007.07.04. steeple
	if(pcsAgsdSkill->m_stInvincibleInfo.lNormalATKProbability != 0)
	{
		// Special Status 는 시간이 지나면 없어진다.
		pcsAgsdCharacter->m_stInvincibleInfo.lNormalATKProbability -= pcsAgsdSkill->m_stInvincibleInfo.lNormalATKProbability;
	}

	// restore skill attack invincible - 2007.07.04. steeple
	if(pcsAgsdSkill->m_stInvincibleInfo.lSkillATKProbability != 0)
	{
		// Speical Status 는 시간이 지나면 없어진다.
		pcsAgsdCharacter->m_stInvincibleInfo.lSkillATKProbability -= pcsAgsdSkill->m_stInvincibleInfo.lSkillATKProbability;
	}

	// restore stun invincible - 2007.07.04. steeple
	if(pcsAgsdSkill->m_stInvincibleInfo.lStunProbability != 0)
	{
		// Speical Status 는 시간이 지나면 없어진다.
		pcsAgsdCharacter->m_stInvincibleInfo.lStunProbability -= pcsAgsdSkill->m_stInvincibleInfo.lStunProbability;
	}

	// restore slow invincible - 2007.07.04. steeple
	if(pcsAgsdSkill->m_stInvincibleInfo.lSlowProbability != 0)
	{
		// Speical Status 는 시간이 지나면 없어진다.
		pcsAgsdCharacter->m_stInvincibleInfo.lSlowProbability -= pcsAgsdSkill->m_stInvincibleInfo.lSlowProbability;
	}

	// restore attr invincible - 2007.07.10. steeple
	if(pcsAgsdSkill->m_stInvincibleInfo.lAttrProbability != 0)
	{
		// Speical Status 는 시간이 지나면 없어진다.
		pcsAgsdCharacter->m_stInvincibleInfo.lAttrProbability -= pcsAgsdSkill->m_stInvincibleInfo.lAttrProbability;
	}

	return TRUE;
}

BOOL AgsmSkill::RestoreSkillFactorArg(AgpdSkill *pcsSkill, ApBase* pcsTarget)
{
	if (!pcsSkill || !pcsTarget)
		return FALSE;

	AgsdSkill			*pcsAgsdSkill	= GetADSkill(pcsSkill);
	if (!pcsAgsdSkill) return FALSE;

	AgpdSkillAttachData	*pcsAttachData	= m_pagpmSkill->GetAttachSkillData(pcsTarget);
	if (!pcsAttachData) return FALSE;

	// restore cast time factors
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCastTimeProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTimeProbability -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCastTimeProbability;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTimeProbability < 0)
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTimeProbability = 0;
	}
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCastTime > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTime -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCastTime;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTime <= 0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTime				= 0;
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTimeProbability	= 0;
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_CAST_TIME)
				pcsAttachData->m_ullBuffedSkillFactorEffect -= AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_CAST_TIME;
		}
	}

	// 2005.07.09. steeple
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lDurationTime > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lDurationTime -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lDurationTime;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lDurationTime <= 0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lDurationTime			= 0;
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_DURATION_TIME)
				pcsAttachData->m_ullBuffedSkillFactorEffect -= AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_DURATION_TIME;
		}
	}

	// restore adjust range factors
	/*
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lMagicSkillRangeProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lMagicSkillRangeProbability -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lMagicSkillRangeProbability;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lMagicSkillRangeProbability < 0)
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lMagicSkillRangeProbability = 0;
	}
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lMagicSkillRange > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lMagicSkillRange -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lMagicSkillRange;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lMagicSkillRange <= 0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lMagicSkillRange				= 0;
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lMagicSkillRangeProbability	= 0;
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_RANGE)
				pcsAttachData->m_ullBuffedSkillFactorEffect -= AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_RANGE;
		}
	}
	*/

	// restore lRangeAdjustPercent
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lRangeAdjustPercent > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lRangeAdjustPercent -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lRangeAdjustPercent;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lRangeAdjustPercent <= 0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lRangeAdjustPercent				= 0;
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_RANGE)
				pcsAttachData->m_ullBuffedSkillFactorEffect -= AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_RANGE;
		}
	}

	// restore cost hp factors
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostHPProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostHPProbability -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostHPProbability;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostHPProbability < 0)
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostHPProbability = 0;
	}
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostHP > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostHP -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostHP;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostHP <= 0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostHP				= 0;
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostHPProbability	= 0;
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_COST_HP)
				pcsAttachData->m_ullBuffedSkillFactorEffect -= AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_COST_HP;
		}
	}

	// restore cost mp factors
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostMPProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMPProbability -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostMPProbability;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMPProbability < 0)
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMPProbability = 0;
		pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostMPProbability = 0;
	}
	// 2005.04.28. steeple
	// 옵티멈 스킬 때문에 lCostMP 약간 변화 생김. 현재 음수값으로 세팅되어 있어서 이런식으로 체크를 해야 한다.
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostMP < 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMP -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostMP;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMP > 0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMP				= 0;
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMPProbability	= 0;
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_COST_MP)
				pcsAttachData->m_ullBuffedSkillFactorEffect -= AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_COST_MP;
		}
		pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostMP = 0;
	}

	// restore cost sp factors
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostSPProbability > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostSPProbability -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostSPProbability;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostSPProbability < 0)
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostSPProbability = 0;
	}
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostSP > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostSP -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lCostSP;
		if (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostSP <= 0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostSP				= 0;
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostSPProbability	= 0;
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_COST_SP)
				pcsAttachData->m_ullBuffedSkillFactorEffect -= AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_COST_SP;
		}
	}

	// restore hp regen
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lHPRegen > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lHPRegen -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lHPRegen;
		if(pcsAttachData->m_stBuffedSkillFactorEffectArg.lHPRegen <=0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lHPRegen = 0;
			if(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT2_REGEN_HP)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT2_REGEN_HP;
		}
		pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lHPRegen = 0;
	}

	// restore mp regen
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lMPRegen > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lMPRegen -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lMPRegen;
		if(pcsAttachData->m_stBuffedSkillFactorEffectArg.lMPRegen <=0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lMPRegen = 0;
			if(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT2_REGEN_MP)
				pcsAttachData->m_ullBuffedSkillCombatEffect -= AGPMSKILL_EFFECT2_REGEN_MP;
		}
		pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lMPRegen = 0;
	}

	// restore bonus exp
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusExpRate > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusExpRate -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusExpRate;
		m_pagpmCharacter->SubGameBonusExp((AgpdCharacter*)pcsTarget, pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusExpRate);
		if(pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusExpRate <=0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusExpRate = 0;
		}
		pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusExpRate = 0;
	}

	// restore bonus money
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusMoneyRate > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusMoneyRate -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusMoneyRate;
		m_pagpmCharacter->SubGameBonusMoney((AgpdCharacter*)pcsTarget, pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusMoneyRate);
		if(pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusMoneyRate <=0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusMoneyRate = 0;
		}
		pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusMoneyRate = 0;
	}

	// restore game bonus
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropRate > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropRate -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropRate;
		m_pagpmCharacter->SubGameBonusDropRate((AgpdCharacter*)pcsTarget, pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropRate);
		if(pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropRate <=0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropRate = 0;
		}
		pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropRate = 0;
	}

	// restore game bonus2
	if (pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropRate2 > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropRate2 -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropRate2;
		m_pagpmCharacter->SubGameBonusDropRate2((AgpdCharacter*)pcsTarget, pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropRate2);
		if(pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropRate2 <=0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropRate2 = 0;
		}
		pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropRate2 = 0;
	}

	// restore charisma drop rate
	if(pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropCharismaRate > 0)
	{
		pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropCharismaRate -= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropCharismaRate;
		m_pagpmCharacter->SubGameBonusCharismaRate((AgpdCharacter*)pcsTarget, pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropCharismaRate);
		if(pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropCharismaRate <= 0)
		{
			pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropCharismaRate = 0;
		}
		pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.lBonusDropCharismaRate = 0;
	}

	return TRUE;
}

// 2007.03.30. steeple
// 스킬이 없어지면서 Summons 도 없애주는 함수
BOOL AgsmSkill::RestoreSummons(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;

	// 소환수랑 Fixed 만 한다. Tame 은 하지 않는다.
	if(m_pagpmSkill->IsSummonsSkill(pcsSkillTemplate) || m_pagpmSkill->IsFixedSkill(pcsSkillTemplate))
	{
		INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			return FALSE;

		m_pagsmSummons->RemoveSummonsByTID((AgpdCharacter*)pcsTarget, (INT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SUMMONS_TID][lSkillLevel]);
	}

	return TRUE;
}

// 2007.11.13. steeple
// Union Type1 을 없애준다.
BOOL AgsmSkill::RestoreUnionType1(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = static_cast<AgpdSkillTemplate*>(pcsSkill->m_pcsTemplate);
	if(m_pagpmSkill->IsSkillUnionType1(pcsSkillTemplate) == FALSE)
		return FALSE;

	AgsdSkillADBase* pcsAgsdADBase = GetADBase(pcsTarget);
	if(!pcsAgsdADBase)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	// UnionType1 이 해제 될 때 캐릭터에 되어 있는 CurrentCount 를 줄여줘야 한다.
	UnionControlArray::iterator iter = std::find(pcsAgsdADBase->m_UnionControlArray.m_astUnionControlInfo.begin(),
												pcsAgsdADBase->m_UnionControlArray.m_astUnionControlInfo.end(),
												pcsAgsdSkill->m_stUnionInfo.m_lUnionID);
	if(iter != pcsAgsdADBase->m_UnionControlArray.m_astUnionControlInfo.end())
	{
		--iter->m_lCurrentCount;
	}

	pcsAgsdSkill->m_stUnionInfo.m_lUnionID = 0;

	return TRUE;
}

// 2007.11.13. steeple
// Union Type2 을 없애준다.
BOOL AgsmSkill::RestoreUnionType2(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = static_cast<AgpdSkillTemplate*>(pcsSkill->m_pcsTemplate);
	if(m_pagpmSkill->IsSkillUnionType2(pcsSkillTemplate) == FALSE)
		return FALSE;

	AgsdSkillADBase* pcsAgsdADBase = GetADBase(pcsTarget);
	if(!pcsAgsdADBase)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	// UnionType1 이 해제 될 때 캐릭터에 되어 있는 CurrentCount 를 줄여줘야 한다.
	UnionControlArray::iterator iter = std::find(pcsAgsdADBase->m_UnionControlArray.m_astUnionControlInfo.begin(),
												pcsAgsdADBase->m_UnionControlArray.m_astUnionControlInfo.end(),
												pcsAgsdSkill->m_stUnionInfo.m_lUnionID);
	if(iter != pcsAgsdADBase->m_UnionControlArray.m_astUnionControlInfo.end())
	{
		// 기존에 Union Max Control 치가 있다면 더해주고 끝이다.
		iter->m_lMaxCount -= pcsAgsdSkill->m_stUnionInfo.m_lMaxCount;
	}
	
	pcsAgsdSkill->m_stUnionInfo.m_lUnionID = 0;
	pcsAgsdSkill->m_stUnionInfo.m_lMaxCount = 0;

	return TRUE;
}

// 2007.11.14. steeple
// 스킬 개별로 올려준 레벨을 원래대로 한다.
BOOL AgsmSkill::ResotreSpecificLevelUp(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsTarget)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = static_cast<AgpdSkillTemplate*>(pcsSkill->m_pcsTemplate);

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_LEVELUP_TID; ++i)
	{
		if(pcsAgsdSkill->m_alLevelUpSkillTID[i] == 0)
			break;

		AgpdSkill* pcsOwnSkill = m_pagpmSkill->GetSkillByTID(static_cast<AgpdCharacter*>(pcsTarget), pcsAgsdSkill->m_alLevelUpSkillTID[i]);
		if(pcsOwnSkill)
		{
			--pcsOwnSkill->m_lModifiedLevel;

			// 패킷을 보낸다.
			SendSpecificSkillLevel(pcsOwnSkill);

			// Passive Skill 이면 다시 캐스팅한다.
			if(m_pagpmSkill->IsPassiveSkill(pcsOwnSkill))
			{
				EndPassiveSkill(pcsOwnSkill);
				CastPassiveSkill(pcsOwnSkill);
			}
		}
	}

	pcsAgsdSkill->m_alLevelUpSkillTID.MemSetAll();

	return TRUE;
}

BOOL AgsmSkill::RestoreDisturbState(AgpdSkill *pcsSkill, ApBase *pcsTarget)
{
	if(NULL == pcsSkill || NULL == pcsTarget || NULL == pcsSkill->m_pcsTemplate)
		return FALSE;

	AgpdSkillTemplate *pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(NULL == pcsSkillTemplate)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
	{
		if(pcsSkillTemplate->m_lEffectType2[i] & AGPMSKILL_EFFECT2_DISTURB_CHARACTER_ACTION)
		{
			// Check Attack
			if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION] & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_ATTACK)
			{
				if(((AgpdCharacter*)pcsTarget)->m_unDisturbAction & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_ATTACK)
					((AgpdCharacter*)pcsTarget)->m_unDisturbAction -= AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_ATTACK;			
			}
			else if(pcsSkillTemplate->m_nProcessIntervalEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION] & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_ATTACK)
			{
				if(((AgpdCharacter*)pcsTarget)->m_unDisturbAction & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_ATTACK)
					((AgpdCharacter*)pcsTarget)->m_unDisturbAction -= AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_ATTACK;			
			}

			// Check Move
			if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION] & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_MOVE)
			{
				if(((AgpdCharacter*)pcsTarget)->m_unDisturbAction & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_MOVE)
					((AgpdCharacter*)pcsTarget)->m_unDisturbAction -= AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_MOVE;			
			}
			else if(pcsSkillTemplate->m_nProcessIntervalEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION] & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_MOVE)
			{
				if(((AgpdCharacter*)pcsTarget)->m_unDisturbAction & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_MOVE)
					((AgpdCharacter*)pcsTarget)->m_unDisturbAction -= AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_MOVE;			
			}

			// Check Use Item
			if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION] & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_USE_ITEM)
			{
				if(((AgpdCharacter*)pcsTarget)->m_unDisturbAction & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_USE_ITEM)
					((AgpdCharacter*)pcsTarget)->m_unDisturbAction -= AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_USE_ITEM;			
			}
			else if(pcsSkillTemplate->m_nProcessIntervalEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION] & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_USE_ITEM)
			{
				if(((AgpdCharacter*)pcsTarget)->m_unDisturbAction & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_USE_ITEM)
					((AgpdCharacter*)pcsTarget)->m_unDisturbAction -= AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_USE_ITEM;
			}

			// Check Skill
			if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION] & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_SKILL)
			{
				if(((AgpdCharacter*)pcsTarget)->m_unDisturbAction & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_SKILL)
					((AgpdCharacter*)pcsTarget)->m_unDisturbAction -= AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_SKILL;			
			}
			else if(pcsSkillTemplate->m_nProcessIntervalEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION] & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_SKILL)
			{
				if(((AgpdCharacter*)pcsTarget)->m_unDisturbAction & AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_SKILL)
					((AgpdCharacter*)pcsTarget)->m_unDisturbAction -= AGPMSKILL_EFFECT_DETAIL_DISTURB_CHARACTER_ACTION_SKILL;
			}
		}
	}

	return TRUE;
}

BOOL AgsmSkill::RestoreOriginalStatus(AgpdSkill *pcsSkill, ApBase *pcsTarget)
{
	if (!pcsSkill || !pcsTarget || !pcsSkill->m_pcsTemplate)
		return FALSE;

	//RestoreModifiedCharLevel(pcsTarget, pcsSkill);

	EndDefenseSkill(pcsSkill, pcsTarget);

	RestoreCombatSkillArg(pcsSkill, pcsTarget);
	RestoreSkillFactorArg(pcsSkill, pcsTarget);

	// 2007.03.30. steeple
	RestoreSummons(pcsSkill, pcsTarget);

	// 2007.11.13. steeple
	RestoreUnionType1(pcsSkill, pcsTarget);
	RestoreUnionType2(pcsSkill, pcsTarget);

	// 2007.11.14. steeple
	ResotreSpecificLevelUp(pcsSkill, pcsTarget);

	RestoreDisturbState(pcsSkill, pcsTarget);

	if (pcsTarget->m_eType == APBASE_TYPE_CHARACTER)
	{
		CalcSkillOptionIgnoreStatus(pcsSkill, (AgpdCharacter*)pcsTarget, FALSE);

		// 2004.08.26. steeple
		// 2004.12.15. steeple 주석처리.
		// SpecialStatus 는 Idle 에서 시간 단위로 처리하게 되므로 스킬이 없어진다고 없애버리면 안된다.
		//RestoreSpecialStatus(pcsSkill, pcsTarget);
		
		int i = 0;

		// AGPMSKILL_EFFECT_LIFE_PROTECTION
		for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
			if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_EFFECT_LIFE_PROTECTION)
			{
				AgsdCharacter	*pcsAgsdCharacter	= m_pagsmCharacter->GetADCharacter((AgpdCharacter *) pcsTarget);

				pcsAgsdCharacter->m_bResurrectWhenDie	= FALSE;
				pcsAgsdCharacter->m_lResurrectHP		= 0;

				break;
			}
	
		// AGPMSKILL_EFFECT_TRANSFORM_TARGET
		if(m_pagpmSkill->IsTransformSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
			m_pagpmCharacter->RestoreTransformCharacter((AgpdCharacter*) pcsTarget);

		// AGPMSKILL_EFFECT_DMG_ADJUST
		for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
			if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_EFFECT_DMG_ADJUST)
			{
				AgsdSkillADBase	*pcsADBase		= GetADBase(pcsTarget);
				if (!pcsADBase)
					return FALSE;

				pcsADBase->m_lDamageAdjustRate	= 0;

				break;
			}
	}

	return TRUE;
}

BOOL AgsmSkill::RestoreSpecialStatus(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	if(pcsTarget->m_eType != APBASE_TYPE_CHARACTER)
		return FALSE;

	int i = 0;
	
	// AGPMSKILL_SPCEIAL_STATUS_STUN
	for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_SPECIAL_STATUS_STUN)
		{
			m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*) pcsTarget, AGPDCHAR_SPECIAL_STATUS_STUN);
			break;
		}

	// AGPMSKILL_SPCEIAL_STATUS_FREEZE
	for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_SPECIAL_STATUS_FREEZE)
		{
			m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*) pcsTarget, AGPDCHAR_SPECIAL_STATUS_FREEZE);
			break;
		}

	// AGPMSKILL_SPCEIAL_STATUS_SLOW
	for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_SPECIAL_STATUS_SLOW)
		{
			m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*) pcsTarget, AGPDCHAR_SPECIAL_STATUS_SLOW);
			break;
		}

	// AGPMSKILL_SPCEIAL_STATUS_INVINCIBLE
	for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_SPECIAL_STATUS_INVINCIBLE)
		{
			m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*) pcsTarget, AGPDCHAR_SPECIAL_STATUS_INVINCIBLE);
			break;
		}

	// AGPMSKILL_SPCEIAL_STATUS_ATTRIBUTE_INVINCIBLE
	for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_SPECIAL_STATUS_ATTRIBUTE_INVINCIBLE)
		{
			m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*) pcsTarget, AGPDCHAR_SPECIAL_STATUS_ATTRIBUTE_INVINCIBLE);
			break;
		}

	// AGPMSKILL_SPECIAL_STATUS_NOT_ADD_AGRO
	for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_SPECIAL_STATUS_NOT_ADD_AGRO)
		{
			m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*) pcsTarget, AGPDCHAR_SPECIAL_STATUS_NOT_ADD_AGRO);
			break;
		}

	// AGPMSKILL_SPECIAL_STATUS_HIDE_AGRO
	for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_SPECIAL_STATUS_HIDE_AGRO)
		{
			m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*) pcsTarget, AGPDCHAR_SPECIAL_STATUS_HIDE_AGRO);
			break;
		}

	// AGPMSKILL_SPECIAL_STATUS_STUN_PROTECT
	for (i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[i] & AGPMSKILL_SPECIAL_STATUS_STUN_PROTECT)
		{
			m_pagpmCharacter->UpdateUnsetSpecialStatus((AgpdCharacter*) pcsTarget, AGPDCHAR_SPECIAL_STATUS_STUN_PROTECT);
			break;
		}

	return TRUE;
}

BOOL AgsmSkill::ApplyUpdateFactorItem(AgpdSkill* pcsSkill, ApBase* pcsTarget, INT16 nIndex, BOOL bFirst, BOOL bAdd, BOOL bTimeOut)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	// Update Factor Item 스킬인지 아닌지 확인한다.
	if(!(pcsSkillTemplate->m_lProcessIntervalEffectType[nIndex] & AGPMSKILL_EFFECT_UPDATE_FACTOR_ITEM)
		&&
		!(pcsSkillTemplate->m_lEffectType[nIndex] & AGPMSKILL_EFFECT_UPDATE_FACTOR_ITEM))
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsTarget);
	if(!pcsAttachData)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	//if(bFirst && bAdd &&
	//	pcsSkillTemplate->m_stConditionArg[nIndex].lArg1 >= 1 &&
	//	pcsSkillTemplate->m_stConditionArg[nIndex].lArg1 <= 13)
	//{
	//	// 올려질 Factor 를 계산한다.
	//	// pcsAgsdSkill 의 Factor 에 저장된다.
	//	CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);
	//}

	// 2004.12.15. steeple. 리턴값이 중요해졌음
	// 성공적으로 Factor 를 올리거나 내려줬으면 TRUE 로 리턴한다.
	BOOL bResult = FALSE;

	//m_eMissedReason 을 바꿔주어 장착 템이 없을경우 miss메세지가 뜨지않도록 수정
	pcsAgsdSkill->m_eMissedReason = AGSDSKILL_MISSED_REASON_TARGET_NUM_ZERO;

	switch(pcsSkillTemplate->m_stConditionArg[nIndex].lArg1)
	{
		case 1:	// 한 손 검류에 데미지 추가
		{
			if(bAdd)	// 더해주기
			{
				// 한 손 검을 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipOneHandSword((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[0] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[0] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				// 한 손 검을 들고 있지 않아야 한다. - 2005.06.
				if((m_pagpmItem->IsEquipOneHandSword((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[0])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[0] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 2:	// 한 손 도끼류에 데미지 추가
		{
			if(bAdd)	// 더해주기
			{
				// 한 손 도끼를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipOneHandAxe((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[1] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[1] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipOneHandAxe((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[1])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[1] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 3:	// 한,양 손 둔기류에 데미지 추가 (2004.12.15. steeple 변경)
		{
			if(bAdd)	// 더해주기
			{
				// 한,양 손 둔기류를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipBlunt((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[2] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[2] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipBlunt((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[2])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[2] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 4:	// 양손 검, 할버드류에 데미지 추가 (2004.12.15. steeple 변경)
		{
			if(bAdd)	// 더해주기
			{
				// 양손 검, 할버드류를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipTwoHandSlash((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[3] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[3] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipTwoHandSlash((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[3])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[3] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 5:	// 방패류 의 블록율 추가
		{
			if(bAdd)	// 더해주기
			{
				// 방패류를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipShield((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[4] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 로 옮긴다. 2006.11.21. steeple
					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					//// 얘는 Factor 가 아니라, CombatEffect 에 있다.
					//pcsAttachData->m_stBuffedSkillCombatEffectArg.lBlockRate += (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_BLOCK_POINT][lSkillLevel];
					//pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lBlockRate = (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_BLOCK_POINT][lSkillLevel];
					
					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[4] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipShield((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[4])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					//// 얘는 Factor 가 아니라, CombatEffect 에 있다.
					//pcsAttachData->m_stBuffedSkillCombatEffectArg.lBlockRate -= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg.lBlockRate;

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[4] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 6:	// 활에 데미지 추가
		{
			if(bAdd)	// 더해주기
			{
				// 활을 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipBow((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[5] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[5] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipBow((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[5])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[5] = FALSE;

					bResult = TRUE;
				}
			}
			
			break;
		}

		case 7:	// 석궁에 데미지 추가
		{
			if(bAdd)	// 더해주기
			{
				// 석궁을 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipCrossBow((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[6] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[6] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipCrossBow((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[6])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[6] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 8:	// 무기 사정거리 증가
		{
			if(bAdd)	// 더해주기
			{
				// 무기를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipWeapon((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[7] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[7] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipWeapon((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[7])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[7] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 9:	// 방패에 정령속성 저항력 추가
		{
			if(bAdd)	// 더해주기
			{
				// 방패류를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipShield((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[8] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[8] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipShield((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[8])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[8] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 10:	// 스태프 데미지 추가
		{
			if(bAdd)	// 더해주기
			{
				// 스태프를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipStaff((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[9] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[9] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipStaff((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[9])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[9] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 11:	// 완드 데미지 추가
		{
			if(bAdd)	// 더해주기
			{
				// 완드를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipWand((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[10] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[10] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipWand((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[10])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[10] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 12:	// 글레이브 (카타리야) 에 데미지 추가.
		{
			if(bAdd)	// 더해주기
			{
				// 글레이브(카타리야)를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipKatariya((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[11] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[11] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipKatariya((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[11])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[11] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 13:	// 루프(차크람) 에 데미지 추가
		{
			if(bAdd)	// 더해주기
			{
				// 루프(챠크람)를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipChakram((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[12] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[12] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipChakram((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[12])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[12] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 14:	// 쌍수 무기 전용
		{
			if(bAdd)	// 더해주기
			{
				// 루프(챠크람)를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipTwoHandBlunt((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[13] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[13] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipTwoHandBlunt((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[13])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[13] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}

		case 15:	// 오른손 무기 전용. 왼손은 비어 있어야 한다.
		{
			if(bAdd)	// 더해주기
			{
				// 루프(챠크람)를 들고 있고 Factor 가 더해지지 않은 상태라면 올려준다.
				if(m_pagpmItem->IsEquipRightHandOnly((AgpdCharacter*)pcsTarget) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[14] == FALSE)
				{
					// 아이템이 있을 때만 팩터 계산하게끔 변경. 2007.07.08. steeple
					if(bFirst)
						CalcUpdateFactorItem(pcsSkill, pcsTarget, TRUE);

					// Factor 를 올려준다.
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 더했다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[14] = TRUE;

					bResult = TRUE;
				}
			}
			else	// 빼주기
			{
				if((m_pagpmItem->IsEquipRightHandOnly((AgpdCharacter*)pcsTarget) == FALSE || bTimeOut) &&
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[14])
				{
					// Factor 를 빼주고
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, FALSE, FALSE);
					m_pagpmFactors->CalcFactor(&((AgpdCharacter*)pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, FALSE, FALSE);

					m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter*)pcsTarget, TRUE);	// 뒤에 TRUE 로 주면, 패킷도 보낸다.

					// Factor 를 빼주었다고 설정해준다.
					pcsAgsdSkill->m_stBuffedSkillFactorEffectArg.bAddedUpdateFactorItem[14] = FALSE;

					bResult = TRUE;
				}
			}

			break;
		}
	}

	return bResult;
}

// 2004.09.15. steeple.
// 특정 Item 에 따라 Factor 를 올려줘야 하는 놈들은 따로 계산한다.
BOOL AgsmSkill::CalcUpdateFactorItem(AgpdSkill* pcsSkill, ApBase* pcsTarget, BOOL bInit /* = TRUE */)
{
	if(!pcsSkill || !pcsTarget)
		return FALSE;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate	= (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return FALSE;

	AgpdFactorDIRT* pcsFactorDIRT = (AgpdFactorDIRT*)m_pagpmFactors->GetFactor(&pcsSkill->m_csFactor, AGPD_FACTORS_TYPE_DIRT);
	if (!pcsFactorDIRT)
		return FALSE;

	if(bInit)
	{
		m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPoint);
		m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPercent);
	}

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	BOOL bCheckConnection = CheckConnectionInfo(pcsSkill, pcsTarget);	// 2007.10.25. steeple

	for(int i = AGPMSKILL_CONST_POINT_START + 1; i < AGPMSKILL_CONST_POINT_END; ++i)
	{
		// '0' 인지 아닌지를 검사해서 '0'이 아니면 target의 factor point에 더한다.
		if(pcsSkillTemplate->m_fUsedConstFactor[i][lSkillLevel] != 0 || pcsSkillTemplate->m_fUsedConstFactor2[i][lSkillLevel] != 0)
		{
			if (!SetUpdateFactorItemPoint(pcsAgsdSkill))
				continue;

			AgpdFactorCharStatus* pcsFactorTemp = (AgpdFactorCharStatus*)m_pagpmFactors->SetFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPoint, NULL, g_alFactorTable[i][0]);
			if(!pcsFactorTemp)
				continue;

			if(g_alFactorTable[i][0] == AGPD_FACTORS_TYPE_DAMAGE || g_alFactorTable[i][0] == AGPD_FACTORS_TYPE_DEFENSE)
			{
				FLOAT	fPrevValue	= 0.0f;
				FLOAT	fConstValue = pcsSkillTemplate->m_fUsedConstFactor[i][lSkillLevel];
				if(bCheckConnection)
					fConstValue += pcsSkillTemplate->m_fUsedConstFactor2[i][lSkillLevel];

				m_pagpmFactors->GetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPoint, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[i][0], g_alFactorTable[i][1], g_alFactorTable[i][2]);
				m_pagpmFactors->SetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPoint, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[i][0], g_alFactorTable[i][1], g_alFactorTable[i][2]);
			}
			else
			{
				FLOAT	fPrevValue	= 0.0f;
				FLOAT	fConstValue = pcsSkillTemplate->m_fUsedConstFactor[i][lSkillLevel];
				if(bCheckConnection)
					fConstValue += pcsSkillTemplate->m_fUsedConstFactor2[i][lSkillLevel];

				m_pagpmFactors->GetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPoint, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[i][0], g_alFactorTable[i][1]);

				// AttackRange 는 Factor 에서 cm 단위로 저장된다. Skill_Const 에서는 m 단위로 온다. 2006.03.28. steeple
				if (g_alFactorTable[i][1] == AGPD_FACTORS_ATTACK_TYPE_ATTACKRANGE)
					m_pagpmFactors->SetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPoint, fPrevValue + (fConstValue * 100.0f), (eAgpdFactorsType) g_alFactorTable[i][0], g_alFactorTable[i][1]);
				else
					m_pagpmFactors->SetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPoint, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[i][0], g_alFactorTable[i][1]);
			}
		}
	}

	for(int i = AGPMSKILL_CONST_PERCENT_START + 1; i < AGPMSKILL_CONST_PERCENT_END; ++i)
	{
		// '0' 인지 아닌지를 검사해서 '0'이 아니면 target의 factor percent에 더한다.
		if(pcsSkillTemplate->m_fUsedConstFactor[i][lSkillLevel] != 0)
		{
			if (!SetUpdateFactorItemPercent(pcsAgsdSkill))
				continue;

			AgpdFactorCharStatus* pcsFactorTemp = (AgpdFactorCharStatus*)m_pagpmFactors->SetFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPercent, NULL, g_alFactorTable[i][0]);
			if(!pcsFactorTemp)
				continue;

			if(g_alFactorTable[i][0] == AGPD_FACTORS_TYPE_DAMAGE || g_alFactorTable[i][0] == AGPD_FACTORS_TYPE_DEFENSE)
			{
				FLOAT	fPrevValue	= 0.0f;
				FLOAT	fConstValue = pcsSkillTemplate->m_fUsedConstFactor[i][lSkillLevel];
				if(bCheckConnection)
					fConstValue += pcsSkillTemplate->m_fUsedConstFactor2[i][lSkillLevel];

				m_pagpmFactors->GetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPercent, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[i][0], g_alFactorTable[i][1], g_alFactorTable[i][2]);
				m_pagpmFactors->SetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPercent, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[i][0], g_alFactorTable[i][1], g_alFactorTable[i][2]);
			}
			else
			{
				FLOAT	fPrevValue	= 0.0f;
				FLOAT	fConstValue = pcsSkillTemplate->m_fUsedConstFactor[i][lSkillLevel];
				if(bCheckConnection)
					fConstValue += pcsSkillTemplate->m_fUsedConstFactor2[i][lSkillLevel];

				m_pagpmFactors->GetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPercent, &fPrevValue, (eAgpdFactorsType) g_alFactorTable[i][0], g_alFactorTable[i][1]);
				m_pagpmFactors->SetValue(pcsAgsdSkill->m_pcsUpdateFactorItemPercent, fPrevValue + fConstValue, (eAgpdFactorsType) g_alFactorTable[i][0], g_alFactorTable[i][1]);
			}
		}
	}

	return TRUE;
}


BOOL AgsmSkill::PreCheckProductSkill(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if (!pcsSkill || !pcsTarget || APBASE_TYPE_CHARACTER != pcsTarget->m_eType || !pcsSkill->m_pcsTemplate)
		return FALSE;

	PVOID pvBuffer[2];
	pvBuffer[0] = pcsSkill;
	pvBuffer[1] = pcsTarget;
	BOOL bResult = FALSE;

	EnumCallback(AGSMSKILL_CB_PRODUCT_SKILL, pvBuffer, &bResult);

	return bResult;
}


INT32 AgsmSkill::GetConstDamageA(AgpdSkill *pcsSkill, ApBase* pcsTarget)
{
	if (!pcsSkill)
		return 0;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return 0;

	AgpdFactorDIRT	*pcsFactorDIRT = (AgpdFactorDIRT *) m_pagpmFactors->GetFactor(&pcsSkill->m_csFactor, AGPD_FACTORS_TYPE_DIRT);
	if (!pcsFactorDIRT)
		return 0;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return 0;

	INT32	lDamageA = (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DAMAGE_A][lSkillLevel];

	if(CheckConnectionInfo(pcsSkill, pcsTarget))
		lDamageA += (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_DAMAGE_A][lSkillLevel];

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PHYSICAL_DMG		||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_MAGIC	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_FIRE	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_EARTH	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_AIR		||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_WATER)
	{
		// 10%씩 상승

		// 식이 잘못된거 같다.
		//
		//
		//
		//

		//lDamageA -= (lDamageA * 0.05) * (20 - pcsFactorDIRT->lValue[AGPD_FACTORS_DIRT_TYPE_INTENSITY]);

		//
		//	고쳐야 하는데 이고 뭐 어디서 보고 코딩한건지 기억이 안난다. ㅡ.ㅡ 
		//
		//
		//
		//
		//
	}

	return lDamageA;
}

INT32 AgsmSkill::GetRealSkillLevel(AgpdSkill *pcsSkill)
{
	return m_pagpmSkill->GetSkillLevel(pcsSkill);
}

INT32 AgsmSkill::GetModifiedSkillLevel(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate)
		return 0;

	INT32 lSkillRealLevel = GetRealSkillLevel(pcsSkill);
	if(!lSkillRealLevel)
		return 0;

	// Heroic Skill은 Modifed된 Skill Level을 돌려보내서는 안된다.
	if(m_pagpmSkill->IsHeroicSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
		return lSkillRealLevel;

	// CloneObject (ex. Buff 형 스킬) 의 레벨은 이미 세팅되어 있다. 2005.09.19. steeple
	if(pcsSkill->m_bCloneObject)
		return lSkillRealLevel;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsSkill->m_pcsBase);
	if(!pcsAttachData)
		return lSkillRealLevel;

	// 2005.05.12.
	// Skill Level Up 을 시켜주는 스킬의 레벨을 구하는 거라면 (ex. 엑스퍼트)
	// 변경된 레벨 값이 아닌 기본 값으로 리턴해준다.
	// 이거 만약에 문제 생기면 Recast Delay 처리 하는 부분을 고쳐야 한다.
	//for(INT16 nIndex = 0; nIndex < AGPMSKILL_MAX_SKILL_CONDITION; nIndex++)
	//{
	//	if(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lProcessIntervalEffectType2[nIndex] & AGPMSKILL_EFFECT2_SKILL_LEVELUP ||
	//		((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lEffectType2[nIndex] & AGPMSKILL_EFFECT2_SKILL_LEVELUP)
	//		return lSkillRealLevel;
	//}

	// 2005.05.12. steeple
	// 업그레이드 할 수 없는 스킬이라면 RealLevel 을 리턴한다.
	if(!m_pagpmEventSkillMaster->IsUpgradable(pcsSkill))
		return lSkillRealLevel;

	// 2007.02.12. steeple
	// 아크로드 스킬은 RealLevel 리턴.
	if(m_pagpmSkill->IsArchlordSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
		return lSkillRealLevel;

	// 2007.02.06. steeple
	// 이제 AttachData 에 모두 통합하였다.
	INT32 lModifiedSkillLevel = lSkillRealLevel +
								pcsAttachData->m_lModifiedSkillLevel +
								m_pagpmItem->GefEffectedSkillPlusLevel((AgpdCharacter*)pcsSkill->m_pcsBase, pcsSkill->m_pcsTemplate->m_lID) +
								pcsSkill->m_lModifiedLevel;		// 다른 스킬로 인해서 올라간 레벨


	// SkillLevel 이 엄한 숫자가 리턴되지 않도록 한다.
	if(lModifiedSkillLevel < 1)
	{
		/*char strCharBuff[256] = { 0, };
		sprintf_s(strCharBuff, sizeof(strCharBuff), "[%s][%d] : %s(%s)", __FUNCTION__, __LINE__,
				((AgpdCharacter*)pcsSkill->m_pcsBase)->m_szID, ((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_szName);
		AuLogFile_s("LOG\\SkillLevelError", strCharBuff);*/

		return 0;
	}
	else if(lModifiedSkillLevel >= AGPMSKILL_MAX_SKILL_CAP)
	{
		/*char strCharBuff[256] = { 0, };
		sprintf_s(strCharBuff, sizeof(strCharBuff), "[%s][%d] : %s(%s)", __FUNCTION__, __LINE__,
				((AgpdCharacter*)pcsSkill->m_pcsBase)->m_szID, ((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_szName);
		AuLogFile_s("LOG\\SkillLevelError", strCharBuff);*/

		return (AGPMSKILL_MAX_SKILL_CAP - 1);
	}

	return lModifiedSkillLevel;
}

BOOL AgsmSkill::SetModifiedSkillLevel(AgpdSkill *pcsSkill)
{
	/*
	if (!pcsSkill)
		return FALSE;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return FALSE;

	AgpdFactorDIRT	*pcsFactorDIRT = (AgpdFactorDIRT *) m_pagpmFactors->GetFactor(&pcsSkill->m_csFactor, AGPD_FACTORS_TYPE_DIRT);
	if (!pcsFactorDIRT)
		return FALSE;

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_BUFF ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_DEBUFF)
	{
		AgsdSkill	*pcsAgsdSkill = GetADSkill(pcsSkill);
		if (!pcsAgsdSkill)
			return FALSE;

		pcsAgsdSkill->m_lModifySkillLevel = 20 - pcsFactorDIRT->lValue[AGPD_FACTORS_DIRT_TYPE_INTENSITY];

		pcsSkill->m_lModifiedSkillLevel += 20 - pcsAgsdSkill->m_lModifySkillLevel;
	}
	*/

	return TRUE;
}

INT32 AgsmSkill::RestoreModifiedSkillLevel(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	/*
	AgsdSkill	*pcsAgsdSkill = GetADSkill(pcsSkill);
	if (!pcsAgsdSkill)
		return FALSE;

	pcsSkill->m_lModifiedSkillLevel -= pcsAgsdSkill->m_lModifySkillLevel;
	*/

	return TRUE;
}

// 2007.12.18. steeple
// 특정 스킬의 레벨이 올라가 있는 지 버프 리스트에서 체크한다.
// 문엘프의 머스커레이드 시리즈가 이와 관련되어 있다.
INT32 AgsmSkill::GetSpecificLevelUpSkillFromBuffedList(ApBase* pcsBase, INT32 lSkillTID)
{
	if(!pcsBase || lSkillTID < 1)
		return 0;

	// 버프된 스킬들중 pcsSkill과 템플릿이 같은 넘이 있는지 검사한다.
	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return 0;

	AgpdSkill* pcsBuffedSkill = NULL;
	INT32 lModifiedSkillLevel = 0;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate)
			break;

		pcsBuffedSkill = pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill;
		if(!(m_pagpmSkill->GetSkillLevelUpType((AgpdSkillTemplate*)pcsBuffedSkill->m_pcsTemplate) & AGPMSKILL_EFFECT_DETAIL_SKILL_LEVELUP_TYPE3))
			continue;

		AgsdSkill* pcsAgsdBuffedSkill = GetADSkill(pcsBuffedSkill);
		if(!pcsAgsdBuffedSkill)
			continue;

		ApSafeArray<INT32, AGPMSKILL_MAX_SKILL_LEVELUP_TID>::iterator iter = std::find(pcsAgsdBuffedSkill->m_alLevelUpSkillTID.begin(),
																						pcsAgsdBuffedSkill->m_alLevelUpSkillTID.end(),
																						lSkillTID);
		if(iter != pcsAgsdBuffedSkill->m_alLevelUpSkillTID.end())
			++lModifiedSkillLevel;
	}

	return lModifiedSkillLevel;
}

INT32 AgsmSkill::GetRealCharLevel(ApBase *pcsBase)
{
	if (!pcsBase)
		return 0;

	AgpdFactor	*pcsFactor = GetResultFactor(pcsBase);
	if (!pcsFactor)
		return 0;

	INT32	lLevel	= m_pagpmFactors->GetLevel(pcsFactor);

	return lLevel;
}

INT32 AgsmSkill::GetModifiedCharLevel(ApBase *pcsBase)
{
	if (!pcsBase)
		return 0;

	AgpdSkillAttachData	*pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return 0;

	return (GetRealCharLevel(pcsBase) + pcsAttachData->m_lModifyCharLevel);
}

BOOL AgsmSkill::SetModifiedCharLevel(ApBase *pcsBase, AgpdSkill *pcsSkill)
{
	/*
	if (!pcsBase || !pcsSkill)
		return FALSE;

	AgpdSkillAttachData	*pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return FALSE;

	//pcsAttachData->m_lModifyCharLevel = pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_MODIFY_CHAR_LEVEL][GetRealSkillLevel(pcsSkill)];
	pcsAttachData->m_lModifyCharLevel = 0;
	pcsAttachData->m_lModifyCharLevel += pcsAttachData->m_lModifyCharLevel;
	*/

	return TRUE;
}

BOOL AgsmSkill::RestoreModifiedCharLevel(ApBase *pcsBase, AgpdSkill *pcsSkill)
{
	/*
	if (!pcsBase || !pcsSkill)
		return FALSE;

	AgpdSkillAttachData	*pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

	AgsdSkill			*pcsAgsdSkill = GetADSkill(pcsSkill);
	if (!pcsAgsdSkill)
		return FALSE;

	pcsAttachData->m_lModifyCharLevel -= pcsAgsdSkill->m_lModifyCharLevel;
	*/

	return TRUE;
}

// 2004.12.29. steeple
// 4차 클로즈 베타 때 새로 바뀌게 되는 공식이다.
// 일반 Skill Damage 를 구해서 리턴해준다.
INT32 AgsmSkill::CalcNormalSkillDamage(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(!pcsSkill || !pcsSkill->m_pcsBase || !pcsTarget)
		return 0;

	if (pcsTarget->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgsdCharacter	*pcsAgsdTarget	= m_pagsmCharacter->GetADCharacter((AgpdCharacter *) pcsTarget);
		if (pcsAgsdTarget && pcsAgsdTarget->m_bIsMaxDefence)
			return 1;
		if (pcsAgsdTarget && pcsAgsdTarget->m_bIsSuperMan)
			return 0;
	}

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return 0;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return 0;

	// 2005.04.28. steeple
	// Scroll 일 때는 그냥 0 으로 내보낸다.
	if(pcsAgsdSkill->m_lSkillScrollIID != AP_INVALID_IID)
		return 0;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return 0;

	INT32 lAttackLevel = m_pagpmCharacter->GetLevel((AgpdCharacter*)pcsSkill->m_pcsBase);
	INT32 lTargetLevel = m_pagpmCharacter->GetLevel((AgpdCharacter*)pcsTarget);

	INT32 lNormalAttackDamage = m_pagpmCombat->CalcPhysicalAttackForSkill((AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTarget, TRUE);
	if(lNormalAttackDamage < 0)
		lNormalAttackDamage	= 0;

	// 2004.12.29. steeple 공식 변경
	INT32 lDamage = 0;
	INT32 lDamageA = (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DAMAGE_A][lSkillLevel];
	INT32 lDamageAPercent = (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DAMAGE_A_PERCENT][lSkillLevel];

	// 2007.10.24. steeple
	// 추가로 해줘야 하는 건지 확인.
	if(CheckConnectionInfo(pcsSkill, pcsTarget))
	{
		lDamageA += (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_DAMAGE_A][lSkillLevel];
		lDamageAPercent += (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_DAMAGE_A_PERCENT][lSkillLevel];
	}

	if(m_pagpmCharacter->IsPC((AgpdCharacter*)pcsSkill->m_pcsBase))
	{
		// PvP 에서는 데미지 덜 감소한다. 2005.08.02. steeple
		if(m_pagpmCharacter->IsPC((AgpdCharacter*)pcsTarget))
		{
			lDamage = (INT32) ( (lNormalAttackDamage + (lNormalAttackDamage * lDamageAPercent / 100.0) + lDamageA)
						* ((100 - ((lAttackLevel - lTargetLevel > 0) ? 0 : (abs(lAttackLevel - lTargetLevel) * 1))) / 100.0));
		}
		else
		{
			lDamage = (INT32) ( (lNormalAttackDamage + (lNormalAttackDamage * lDamageAPercent / 100.0) + lDamageA)
						* ((100 - ((lAttackLevel - lTargetLevel > 0) ? 0 : (abs(lAttackLevel - lTargetLevel) * 5))) / 100.0));
		}
	}
	else
	{
		lDamage = (INT32) ( (lNormalAttackDamage + (lNormalAttackDamage * lDamageAPercent / 100.0) + lDamageA)
					* ((100 - ((lAttackLevel - lTargetLevel > 0) ? 0 : (abs(lAttackLevel - lTargetLevel) * 1))) / 100.0));
	}

	// 방어력 구하기
	FLOAT fPhysicalDefense = m_pagpmCombat->GetPhysicalDefense((AgpdCharacter*)pcsTarget);
	FLOAT fPhysicalResistance = m_pagpmCombat->GetPhysicalResistance((AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTarget);
	FLOAT fHeroicDefense = m_pagpmCombat->GetHeroicDefense((AgpdCharacter*)pcsTarget);

	// 2005.11.08. steeple
	// 공식 약간 변경. Defense 를 먼저 뺀 후 Resistance 를 적용시킨다.
	// 저항력까지 적용한 공식에다가 최종 Heroic Defense를 까준다.
	FLOAT fFinalDamage = (FLOAT)lDamage - fPhysicalDefense;
	FLOAT fResi = 1.0f - (FLOAT)(fPhysicalResistance / 100.0f);
	fFinalDamage *= fResi;

	fFinalDamage -= fHeroicDefense;

	// 공성 몬스터에 대한 데미지 보정
	stAgsmCombatAttackResult	stAttackResult;
	memset(&stAttackResult, 0, sizeof(stAttackResult));
	stAttackResult.pAttackChar	= (AgpdCharacter *) pcsSkill->m_pcsBase;
	stAttackResult.pTargetChar	= (AgpdCharacter *) pcsTarget;
	stAttackResult.nDamage		= (INT32) fFinalDamage;

	EnumCallback(AGSMSKILL_CB_DAMAGE_ADJUST_SIEGEWAR, &stAttackResult, pcsSkill);

	fFinalDamage	= (FLOAT) stAttackResult.nDamage;

	return (INT32)fFinalDamage;

	//// 2005.03.07. steeple
	//// 물리 저항력 적용
	//lDamage = (INT32)(((FLOAT)lDamage) * (1.0f - (FLOAT)(fPhysicalResistance / 100.0f)));

	//return (INT32)((FLOAT)lDamage - fPhysicalDefense);
}

// 2004.12.29. steeple 공식변경
// 4차 클로즈 베타에 맞게 끔 변경했다.
INT32 AgsmSkill::CalcSpiritSkillDamage(AgpdSkill* pcsSkill, ApBase* pcsTarget, AgpdFactorAttribute* pstAttrDamage)
{
	if(!pcsSkill || !pcsSkill->m_pcsBase || !pcsTarget || !pstAttrDamage)
		return 0;

	if (pcsTarget->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgsdCharacter	*pcsAgsdTarget	= m_pagsmCharacter->GetADCharacter((AgpdCharacter *) pcsTarget);
		if (pcsAgsdTarget && pcsAgsdTarget->m_bIsMaxDefence)
			return 1;
		if (pcsAgsdTarget && pcsAgsdTarget->m_bIsSuperMan)
			return 0;
	}

	if(m_pagsmCombat->IsAttrInvincible((AgpdCharacter*)pcsTarget))
		return 0;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return 0;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return 0;

	INT32 lAttackLevel = m_pagpmCharacter->GetLevel((AgpdCharacter*)pcsSkill->m_pcsBase);
	INT32 lTargetLevel = m_pagpmCharacter->GetLevel((AgpdCharacter*)pcsTarget);

	INT32 lDamage = 0;
	INT32 lSpiritStoneDamage = 0;
	INT32 lSpiritConstIndex = -1;
	INT32 lSpiritSkillDamage = 0;
	INT32 lSpiritSkillDamagePercent = 0;

	FLOAT fAttrDamage = 0.0f;
	FLOAT fAttrDefense = 0.0f;

	BOOL bCheckConnection = CheckConnectionInfo(pcsSkill, pcsTarget);	// 2007.10.25. steeple

	// 정령 속성을 돌면서 계산해준다.
	for(int i = AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC; i <= AGPD_FACTORS_ATTRIBUTE_TYPE_ICE; i++)
	{
		//switch(i)
		//{
		//	case AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC:
		//	case AGPD_FACTORS_ATTRIBUTE_TYPE_WATER:
		//	case AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE:
		//	case AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH:
		//	case AGPD_FACTORS_ATTRIBUTE_TYPE_AIR:
				lSpiritStoneDamage = m_pagpmCombat->CalcFirstSpiritAttack((AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTarget, i, FALSE, FALSE, TRUE);
		//		break;

		//	case AGPD_FACTORS_ATTRIBUTE_TYPE_POISON:
		//	case AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING:
		//	case AGPD_FACTORS_ATTRIBUTE_TYPE_ICE:
		//		lSpiritStoneDamage = m_pagpmCombat->CalcSecondSpiritAttack((AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTarget, i, FALSE, FALSE, TRUE);
		//		break;
		//}

		// Factor 로 Const 의 Index 를 구한다.
		lSpiritConstIndex = GetSpiritConstIndexByFactor((AgpdFactorAttributeType)i);
		if(lSpiritConstIndex < AGPMSKILL_CONST_DAMAGE_MAGIC || 
			lSpiritConstIndex > AGPMSKILL_CONST_DAMAGE_THUNDER)
			continue;

		lSpiritSkillDamage = (INT32)pcsSkillTemplate->m_fUsedConstFactor[lSpiritConstIndex][lSkillLevel];
		lSpiritSkillDamagePercent = (INT32)pcsSkillTemplate->m_fUsedConstFactor[lSpiritConstIndex + AGPMSKILL_CONST_DAMAGE_MAGIC_PERCENT - AGPMSKILL_CONST_DAMAGE_MAGIC][lSkillLevel];

		// 2007.10.24. steeple
		// 추가로 해줘야 하는 건지 확인.
		if(bCheckConnection)
		{
			lSpiritSkillDamage += (INT32)pcsSkillTemplate->m_fUsedConstFactor2[lSpiritConstIndex][lSkillLevel];
			lSpiritSkillDamagePercent += (INT32)pcsSkillTemplate->m_fUsedConstFactor2[lSpiritConstIndex + AGPMSKILL_CONST_DAMAGE_MAGIC_PERCENT - AGPMSKILL_CONST_DAMAGE_MAGIC][lSkillLevel];
		}

		// 공격력 공식
		if(m_pagpmCharacter->IsPC((AgpdCharacter*)pcsSkill->m_pcsBase))
		{
			// PvP 에서는 데미지 덜 감소한다. 2005.08.02. steeple
			if(m_pagpmCharacter->IsPC((AgpdCharacter*)pcsTarget))
			{
				fAttrDamage = (FLOAT) ((lSpiritStoneDamage + (lSpiritStoneDamage * lSpiritSkillDamagePercent / 100.0) + lSpiritSkillDamage)
								* ((100 - ((lAttackLevel - lTargetLevel) > 0 ? 0 : (abs(lAttackLevel - lTargetLevel) * 1))) / 100.0));
			}
			else
			{
				fAttrDamage = (FLOAT) ((lSpiritStoneDamage + (lSpiritStoneDamage * lSpiritSkillDamagePercent / 100.0) + lSpiritSkillDamage)
								* ((100 - ((lAttackLevel - lTargetLevel) > 0 ? 0 : (abs(lAttackLevel - lTargetLevel) * 5))) / 100.0));
			}
		}
		else
		{
			fAttrDamage = (FLOAT) ((lSpiritStoneDamage + (lSpiritStoneDamage * lSpiritSkillDamagePercent / 100.0) + lSpiritSkillDamage)
							* ((100 - ((lAttackLevel - lTargetLevel) > 0 ? 0 : (abs(lAttackLevel - lTargetLevel) * 1))) / 100.0));
		}

		// 방어력
		if(m_pagpmCharacter->IsPC((AgpdCharacter*)pcsTarget))
		{
			fAttrDefense = (FLOAT)m_pagpmCombat->CalcFirstSpiritDefense((AgpdCharacter*)pcsTarget, i);
		}
		else
		{
			m_pagpmFactors->GetValue(&((AgpdCharacter*)pcsTarget)->m_csFactor, &fAttrDefense, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DEFENSE, AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT, i);
		}

		// 최종 데미지 구했음
		pstAttrDamage->lValue[i] = (INT32)(fAttrDamage * (100.0f - fAttrDefense) / 100.0f * 2.0f);
		if(pstAttrDamage->lValue[i] <= 0)
			pstAttrDamage->lValue[i] = 1;

		lDamage += pstAttrDamage->lValue[i];
	}

	// 공성 몬스터에 대한 데미지 보정
	stAgsmCombatAttackResult	stAttackResult;
	memset(&stAttackResult, 0, sizeof(stAttackResult));
	stAttackResult.pAttackChar	= (AgpdCharacter *) pcsSkill->m_pcsBase;
	stAttackResult.pTargetChar	= (AgpdCharacter *) pcsTarget;
	stAttackResult.nDamage		= (INT32) lDamage;

	EnumCallback(AGSMSKILL_CB_DAMAGE_ADJUST_SIEGEWAR, &stAttackResult, pcsSkill);

	lDamage	= stAttackResult.nDamage;

	return lDamage;
}

// CalcHeroicSkillDamage : Calculate Heroic Skill Damage
// Calculate Rule has Penalty about level gap and heroic damage - heroic defense
INT32 AgsmSkill::CalcNormalHeroicSkillDamage(AgpdSkill* pcsSkill, ApBase* pcsTarget)
{
	if(NULL == pcsSkill || NULL == pcsTarget)
		return 0;

	if(pcsTarget->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgsdCharacter *pcsAgsdTarget = m_pagsmCharacter->GetADCharacter((AgpdCharacter*)pcsTarget);
		if (pcsAgsdTarget && pcsAgsdTarget->m_bIsMaxDefence)
			return 1;
		if(pcsAgsdTarget && pcsAgsdTarget->m_bIsSuperMan)
			return 0;
	}

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(NULL == pcsSkillTemplate)
		return 0;

	AgsdSkill *pcsAgsdSkill = GetADSkill(pcsSkill);
	if(NULL == pcsAgsdSkill)
		return 0;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(0 == lSkillLevel)
		return 0;

	BOOL bCheckConnection = CheckConnectionInfo(pcsSkill, pcsTarget);

	INT32 lAttackLevel = m_pagpmCharacter->GetLevel((AgpdCharacter*)pcsSkill->m_pcsBase);
	INT32 lTargetLevel = m_pagpmCharacter->GetLevel((AgpdCharacter*)pcsTarget);

	INT32 lFinalHeroicDamage = m_pagpmCombat->CalcHeroicAttackForSkill((AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTarget, TRUE);

	// Heroic 관련 업그레이드값이 있는지 확인한다.
	INT32 lDamage = 0;
	INT32 lDamageHeroic		   = (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DAMAGE_HEROIC][lSkillLevel];
	INT32 lDamageHeroicPercent = (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DAMAGE_HEROIC_PERCENT][lSkillLevel];
	if(bCheckConnection)
	{
		lDamageHeroic			+= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_DAMAGE_HEROIC][lSkillLevel];
		lDamageHeroicPercent	+= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_DAMAGE_HEROIC_PERCENT][lSkillLevel];
	}

	
	// 공식 : NormalHeroic damage + (Heroic damage * Damage Percent) + Add HeroicDamage
	lDamage = (INT32)( (lFinalHeroicDamage + (lFinalHeroicDamage * lDamageHeroicPercent / 100.0) + lDamageHeroic)
		* ((100 - ((lAttackLevel - lTargetLevel > 0) ? 0 : (abs(lAttackLevel - lTargetLevel) * 1))) / 100.0));

	// Target의 방어력 구하기
	FLOAT fHeroicDefense	= m_pagpmCombat->GetHeroicDefense((AgpdCharacter*)pcsTarget);
	FLOAT fHeroicResistance = m_pagpmCombat->GetHeroicResistanceByAttackerHeroicClass((AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTarget);

	// 최종 데미지 구하기
	FLOAT fFinalDamage		= (FLOAT)lDamage - fHeroicDefense;
	FLOAT fResi				= 1.0f - (FLOAT)(fHeroicResistance / 100.0f);
	fFinalDamage		   *= fResi; 

	return (INT32)fFinalDamage;
}

// 2004.12.29. steeple
// AgpdFactors.h 에 있는 정령속성의 인덱스로 Skill Const 에 있는 놈을 리턴해준다.
INT32 AgsmSkill::GetSpiritConstIndexByFactor(AgpdFactorAttributeType eAttributeType)
{
	INT32 lSkillConstIndex = -1;

	switch(eAttributeType)
	{
		case AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC:
			lSkillConstIndex = AGPMSKILL_CONST_DAMAGE_MAGIC;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_WATER:
			lSkillConstIndex = AGPMSKILL_CONST_DAMAGE_WATER;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE:
			lSkillConstIndex = AGPMSKILL_CONST_DAMAGE_FIRE;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH:
			lSkillConstIndex = AGPMSKILL_CONST_DAMAGE_EARTH;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_AIR:
			lSkillConstIndex = AGPMSKILL_CONST_DAMAGE_AIR;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_POISON:
			lSkillConstIndex = AGPMSKILL_CONST_DAMAGE_POISON;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING:
			lSkillConstIndex = AGPMSKILL_CONST_DAMAGE_THUNDER;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_ICE:
			lSkillConstIndex = AGPMSKILL_CONST_DAMAGE_ICE;
			break;
		default:
			break;
	}

	return lSkillConstIndex;
}

// 2005.01.15. steeple
// AgpdFactors.h 에 있는 정령속성의 인덱스로 Skill Const 에 있는 DOT 놈을 리턴해준다.
INT32 AgsmSkill::GetSpiritDOTConstIndexByFactor(AgpdFactorAttributeType eAttributeType)
{
	INT32 lSkillConstIndex = -1;

	switch(eAttributeType)
	{
		case AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC:
			lSkillConstIndex = AGPMSKILL_CONST_DOT_DMG_MAGIC;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_WATER:
			lSkillConstIndex = AGPMSKILL_CONST_DOT_DMG_WATER;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE:
			lSkillConstIndex = AGPMSKILL_CONST_DOT_DMG_FIRE;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH:
			lSkillConstIndex = AGPMSKILL_CONST_DOT_DMG_EARTH;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_AIR:
			lSkillConstIndex = AGPMSKILL_CONST_DOT_DMG_AIR;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_POISON:
			lSkillConstIndex = AGPMSKILL_CONST_DOT_DMG_POISON;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING:
			lSkillConstIndex = AGPMSKILL_CONST_DOT_DMG_THUNDER;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_ICE:
			lSkillConstIndex = AGPMSKILL_CONST_DOT_DMG_ICE;
			break;
		default:
			break;
	}

	return lSkillConstIndex;
}

// 2005.07.07. steeple
// AgpdFactors.h 에 있는 정령속성의 인덱스로 Skill Const 에 있는 EA(Effect Area) 놈을 리턴해준다.
INT32 AgsmSkill::GetSpiritEAConstIndexByFactor(AgpdFactorAttributeType eAttributeType)
{
	INT32 lSkillConstIndex = -1;

	switch(eAttributeType)
	{
		case AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL:
			lSkillConstIndex = AGPMSKILL_CONST_EA_DMG_PHYSICAL;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC:
			lSkillConstIndex = AGPMSKILL_CONST_EA_DMG_MAGIC;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_WATER:
			lSkillConstIndex = AGPMSKILL_CONST_EA_DMG_WATER;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE:
			lSkillConstIndex = AGPMSKILL_CONST_EA_DMG_FIRE;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH:
			lSkillConstIndex = AGPMSKILL_CONST_EA_DMG_EARTH;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_AIR:
			lSkillConstIndex = AGPMSKILL_CONST_EA_DMG_AIR;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_POISON:
			lSkillConstIndex = AGPMSKILL_CONST_EA_DMG_POISON;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING:
			lSkillConstIndex = AGPMSKILL_CONST_EA_DMG_THUNDER;
			break;
		case AGPD_FACTORS_ATTRIBUTE_TYPE_ICE:
			lSkillConstIndex = AGPMSKILL_CONST_EA_DMG_ICE;
			break;
		default:
			break;
	}

	return lSkillConstIndex;
}

// 2005.09.18. steeple
// 소환수가 소환될 때 해줘야 할 것 중에서 SummonsType5 관련을 주인의 버프된 스킬에서 체크해서 소환수에게 걸어준다.
BOOL AgsmSkill::ProcessSummonsType5InBuffedList(ApBase* pcsOwner, ApBase* pcsSummons)
{
	if(!pcsOwner || !pcsSummons)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsOwner);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsOwner);
	if(!pcsADBase)
		return FALSE;

	// 버프된 스킬이 있는지 검사한다.
	if(pcsAttachData->m_astBuffSkillList[0].lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = NULL;

	for(INT32 i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			break;

		pcsSkillTemplate = (AgpdSkillTemplate*)pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate;
		if(!pcsSkillTemplate)
			continue;

		// Summons Type5 인지 쳌흐
		if((pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_SUMMONS) &&
			(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE5))
		{
			INT32 lSkillLevel = GetModifiedSkillLevel(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);
			if(!lSkillLevel)
				continue;

			INT32 lAdditionalSkillTID = pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel);

			if(!lAdditionalSkillTID)
				continue;

			CastSkill(pcsOwner,
						lAdditionalSkillTID,
						m_pagpmSkill->GetSkillLevel(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill),
						pcsSummons);
		}
	}

	return TRUE;
}

// 2005.09.18. steeple
// 소환수가 소환될 때 해줘야 할 것 중에서 SummonsType5 관련을 주인의 패시브 스킬에서 체크해서 소환수에게 걸어준다.
BOOL AgsmSkill::ProcessSummonsType5InPassiveList(ApBase* pcsOwner, ApBase* pcsSummons)
{
	if(!pcsOwner || !pcsSummons)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsOwner);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsOwner);
	if(!pcsADBase)
		return FALSE;

	for(INT32 i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if(pcsAttachData->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill* pcsSkill = m_pagpmSkill->GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
		if(!pcsSkill)
			pcsSkill = pcsAttachData->m_apcsUsePassiveSkill[i];

		if(!pcsSkill)
			continue;

		AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
		if(!pcsSkillTemplate)
			continue;

		// Summons Type5 인지 쳌흐
		if((pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_SUMMONS) &&
			(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE5))
		{
			INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
			if(!lSkillLevel)
				continue;

			INT32 lAdditionalSkillTID = pcsSkillTemplate->GetAdditionalSkillTID(lSkillLevel);

			if(!lAdditionalSkillTID)
				continue;

			CastSkill(pcsOwner,
						lAdditionalSkillTID,
						m_pagpmSkill->GetSkillLevel(pcsSkill),
						pcsSummons);
		}
	}

	return TRUE;
}

BOOL AgsmSkill::LearnArchlordSkill(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	//if(!m_pagpmArchlord->IsArchlord(pcsCharacter->m_szID))
	//	return TRUE;
	if(m_szArchlord.Compare(pcsCharacter->m_szID) != COMPARE_EQUAL)
		return TRUE;

	ArchlordSkillList& vcArchlordSkill = m_pagpmEventSkillMaster->GetArchlordSkillList();
	ArchlordSkillIter iter = vcArchlordSkill.begin();
	while(iter != vcArchlordSkill.end())
	{
		AgpdSkill* pcsSkill = m_pagpmSkill->LearnSkill(pcsCharacter, *iter, 1);	// 레벨은 1
		if(pcsSkill)
		{
			m_vcArchlordSkill.push_back(pcsSkill);

			// Passive Skill 이면 Cast 해준다.
			if(m_pagpmSkill->IsPassiveSkill(pcsSkill))
				CastPassiveSkill(pcsSkill);
		}

		++iter;
	}

	return TRUE;
}

BOOL AgsmSkill::RemoveArchlordSkill(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	//if(!m_pagpmArchlord->IsArchlord(pcsCharacter->m_szID))
	//	return TRUE;
	if(m_szArchlord.Compare(pcsCharacter->m_szID) != COMPARE_EQUAL)
		return TRUE;

	vector<AgpdSkill*>::iterator iter = m_vcArchlordSkill.begin();
	while(iter != m_vcArchlordSkill.end())
	{
		AgpdSkill* pcsSkill = static_cast<AgpdSkill*>(*iter);
		if(!pcsSkill)
			break;

		// 스킬을 종료시킨다.
		if(m_pagpmSkill->IsPassiveSkill(pcsSkill))
			EndPassiveSkill(pcsSkill);
		else
			EndBuffedSkill(pcsCharacter, pcsSkill);

		m_pagpmEventSkillMaster->EnumCallback(AGPMEVENT_SKILL_CB_END_SKILL, pcsCharacter, pcsSkill);
		m_pagpmEventSkillMaster->EnumCallback(AGPMEVENT_SKILL_CB_REMOVE_SKILL, pcsSkill, pcsCharacter);
		m_pagpmSkill->RemoveOwnSkillList(pcsSkill);
		m_pagpmSkill->RemoveSkill(pcsSkill->m_lID);

		++iter;
	}

	m_vcArchlordSkill.clear();

	return TRUE;
}

// 2006.12.18. steeple
BOOL AgsmSkill::BuffRaceSkill(INT32 lRace)
{
	if(lRace <= AURACE_TYPE_NONE || lRace >= AURACE_TYPE_MAX)
		return FALSE;

	// 종족전이 끝났을 때, 승자 쪽 종족에게 승리에 해당하는 스킬을 걸어준다.
	INT32 lIndex = 0;
	for(AgpdCharacter* pcsCharacter = m_pagpmCharacter->GetCharSequence(&lIndex);
		pcsCharacter;
		pcsCharacter = m_pagpmCharacter->GetCharSequence(&lIndex))
	{
		if(!pcsCharacter)
			continue;

		if(!m_pagpmCharacter->IsPC(pcsCharacter))
			continue;

		AuAutoLock csLock(pcsCharacter->m_Mutex);
		if(!csLock.Result())
			continue;

		if(lRace != m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor))
			continue;

		BuffRaceSkill(pcsCharacter);
	}

	return TRUE;
}

// 2006.12.18. steeple
BOOL AgsmSkill::BuffRaceSkill(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	if(!pcsCharacter->m_bIsActiveData ||
		!pcsCharacter->m_bIsAddMap ||
		pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return FALSE;

	if(_tcslen(pcsCharacter->m_szID) == 0)
		return FALSE;

	RaceSkillList& vtRaceSkillList = m_pagpmEventSkillMaster->GetRaceSkillList();
	RaceSkillIter iter = vtRaceSkillList.begin();
	while(iter != vtRaceSkillList.end())
	{
		if(CastSkill(static_cast<ApBase*>(pcsCharacter),
					*iter,
					1,
					static_cast<ApBase*>(NULL)))			// Level 은 1 로 해준다.
		{
			// Lens Stone 이펙트 넣어준다.
			SendAdditionalEffect(pcsCharacter,
								AGPMSKILL_ADDITIONAL_EFFECT_LENS_STONE,
								0,
								m_pagsmCharacter->GetCharDPNID(pcsCharacter));
		}

		++iter;
	}

	return TRUE;
}

BOOL AgsmSkill::EndRaceSkill(INT32 lRace)
{
	if(lRace <= AURACE_TYPE_NONE || lRace >= AURACE_TYPE_MAX)
		return FALSE;

	RaceSkillList& vtRaceSkillList = m_pagpmEventSkillMaster->GetRaceSkillList();

	INT32 lIndex = 0;
	for(AgpdCharacter* pcsCharacter = m_pagpmCharacter->GetCharSequence(&lIndex);
		pcsCharacter;
		pcsCharacter = m_pagpmCharacter->GetCharSequence(&lIndex))
	{
		if(!pcsCharacter)
			continue;

		AuAutoLock csLock(pcsCharacter->m_Mutex);
		if(!csLock.Result())
			continue;

		if(!m_pagpmCharacter->IsPC(pcsCharacter))
			continue;

		if(_tcslen(pcsCharacter->m_szID) == 0)
			continue;

		if(lRace != m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor))
			continue;

		RaceSkillIter iter = vtRaceSkillList.begin();
		while(iter != vtRaceSkillList.end())
		{
			EndBuffedSkillByTID(static_cast<ApBase*>(pcsCharacter), *iter);
			++iter;
		}
	}

	return TRUE;
}

// 2006.12.19. steeple
// 옵션에 스턴있는 지 체크해서 사용해준다.
BOOL AgsmSkill::ProcessOptionStun(AgpdCharacter* pcsAttacker, AgpdCharacter* pcsTarget)
{
	if(!pcsAttacker || !pcsTarget)
		return FALSE;

	AgsdCharacter* pcsAgsdAttacker = m_pagsmCharacter->GetADCharacter(pcsAttacker);
	if(!pcsAgsdAttacker)
		return FALSE;

	if(pcsAgsdAttacker->m_stOptionSkillData.m_lStunRate <= 0)
		return FALSE;

	// 먼저 공격자의 스턴 확률을 체크한다.
	if(pcsAgsdAttacker->m_stOptionSkillData.m_lStunRate < m_csRandom.randInt(100))
		return FALSE;

	// 2004.12.16. steeple
	// 방어자가 Stun Protect 상태이면 Stun 안 걸린다.
	if(pcsTarget->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_STUN_PROTECT)
		return FALSE;

	// 다른 모듈에서 특수상태와 관련된 적용해야할 값이 있는지 가져온다.

	UINT64	ulSpecialStatus		= (UINT64)	AGPDCHAR_SPECIAL_STATUS_STUN;
	UINT32	ulAdjustDuration	= 0;
	INT32	lAdjustProbability	= 0;

	PVOID	pvBuffer[4];
	pvBuffer[0]	= (PVOID)	&ulSpecialStatus;
	pvBuffer[1]	= (PVOID)	NULL;	//pcsSkill;
	pvBuffer[2]	= (PVOID)	&ulAdjustDuration;
	pvBuffer[3]	= (PVOID)	&lAdjustProbability;

	EnumCallback(AGSMSKILL_CB_CHECK_SPECIAL_STATUS_FACTOR, pcsTarget, pvBuffer);

	if (lAdjustProbability > 0)
	{
		// 스턴 실패...
		if (m_csRandom.randInt(100) < lAdjustProbability)
			return FALSE;
	}

	UINT32	ulStunDuration	= pcsAgsdAttacker->m_stOptionSkillData.m_ulStunTime;
	if (ulStunDuration == 0)
		ulStunDuration	= 3000;			// 기본은 3초로 한다.

	ulStunDuration -= ulAdjustDuration;
	m_pagsmCharacter->SetSpecialStatusTime(pcsTarget, AGPDCHAR_SPECIAL_STATUS_STUN, ulStunDuration);

	return TRUE;
}

// 2007.07.04. steeple
// 공격하는 데미지의 일부를 즉시 자신의 HP 로 전환한다.
BOOL AgsmSkill::ProcessDamageToHPInstant(AgpdSkill* pcsSkill, ApBase* pcsTarget, INT32 lDamage)
{
	if(!pcsSkill || !pcsSkill->m_pcsBase || !pcsTarget)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	if(!(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_CONVERT] & AGPMSKILL_EFFECT_DETAIL_CONVERT_ATK_DAMAGE_TO_HP_INSTANT))
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	INT32 lChangeValue = (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DAMAGE_CONVERT_HP][lSkillLevel];
	
	// 2007.10.25. steeple
	if(CheckConnectionInfo(pcsSkill, pcsTarget))
		lChangeValue += (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_DAMAGE_CONVERT_HP][lSkillLevel];

	if(lChangeValue == 0)
		return FALSE;

	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pcsSkill->m_pcsBase;

	// 현재 HP, MAX HP 를 얻는다.
	INT32 lCurrentHP, lMaxHP;
	lCurrentHP = lMaxHP = 0;

	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lCurrentHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);
	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lMaxHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_HP);

	lCurrentHP += (INT32)((double)lDamage * (double)lChangeValue / 100);
	if(lCurrentHP > lMaxHP)
		lCurrentHP = lMaxHP;

	// 다시 세팅
	PVOID pvPacket = NULL;
	m_pagsmFactors->UpdateCharPoint(&pcsCharacter->m_csFactor, &pvPacket, lCurrentHP, 0, 0);

	// 변경 했다면 패킷 보내버린다.
	if(pvPacket)
	{
		m_pagsmCharacter->SendPacketFactor(pvPacket, pcsCharacter, PACKET_PRIORITY_4);
		m_pagpmCharacter->m_csPacket.FreePacket(pvPacket);
	}

	return TRUE;
}

// 2007.07.20. steeple
// 스킬의 CoolTime 을 초기화 해준다. 즉, Endtime, RecastDelay 를 초기화 해주는 것임.
BOOL AgsmSkill::ProcessInitCoolTime(AgpdCharacter* pcsCharacter, INT32 lSkillTID)
{
	if(!pcsCharacter || !lSkillTID)
		return FALSE;

	// 해당하는 스킬을 가지고 있어야 초기화를 한다. 없을 수도 있으니 return TRUE
	AgpdSkill* pcsSkill = m_pagpmSkill->GetSkillByTID(pcsCharacter, lSkillTID);
	if(!pcsSkill)
		return TRUE;

	m_pagpmSkill->InitSkillCoolTime(pcsSkill);
	SendInitCoolTime((ApBase*)pcsCharacter, lSkillTID);

	return TRUE;
}

BOOL AgsmSkill::CBInitSkill(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill	*pThis		= (AgsmSkill *)	pClass;
	AgpdSkill	*pcsSkill	= (AgpdSkill *)	pData;

	if (pcsSkill->m_pcsBase && pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		if (pThis->m_pAgsmServerManager->GetThisServerType() != AGSMSERVER_TYPE_GAME_SERVER)
			return TRUE;

		if (!((AgpdCharacter *) pcsSkill->m_pcsBase)->m_bIsAddMap)
			return TRUE;
	}

	//pThis->m_pagpmSkill->UpdateSkillPoint(pcsSkill, 1);

	if (pcsSkill->m_pcsBase && pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgsdCharacter *pcsAgsdCharacter = pThis->m_pagsmCharacter->GetADCharacter((AgpdCharacter *) pcsSkill->m_pcsBase);
		if (!pcsAgsdCharacter)
			return FALSE;

		// dpnid가 세팅되어 있으면 주인한테 이 스킬 정보를 보내준다.
		if (pcsAgsdCharacter->m_dpnidCharacter != 0)
		{
			INT16	nPacketLength = 0;
			PVOID	pvPacket = pThis->m_pagpmSkill->MakePacketSkill(pcsSkill, &nPacketLength);

			BOOL	bRetval = pThis->SendPacket(pvPacket, nPacketLength, pcsAgsdCharacter->m_dpnidCharacter);

			pThis->m_pagpmSkill->m_csPacket.FreePacket(pvPacket);

			// 이 주인넘이 스킬을 새로 얻은것이다. DB에 저장한다.

			/*

			if (!pThis->StreamInsertDB(pcsSkill, NULL, NULL))
				return FALSE;

			*/

			//return bRetval;
		}
	}

	return TRUE;
}

BOOL AgsmSkill::CBRemoveSkill(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill	*pThis		= (AgsmSkill *)	pClass;
	AgpdSkill	*pcsSkill	= (AgpdSkill *)	pData;

	if (pcsSkill->m_pcsBase && pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		if (pThis->m_pAgsmServerManager->GetThisServerType() != AGSMSERVER_TYPE_GAME_SERVER)
			return TRUE;

		if (!((AgpdCharacter *) pcsSkill->m_pcsBase)->m_bIsAddMap)
			return TRUE;
	}

	return pThis->EndSkill(pcsSkill);
}

BOOL AgsmSkill::CBActionSkill(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmSkill::CBActionSkill");

	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill			*pThis			= (AgsmSkill *)				pClass;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)			pData;
	AgpdCharacterAction	*pstAction		= (AgpdCharacterAction *)	pCustData;

	ApBase			*pcsBase		= (ApBase *) pstAction->m_lUserData[1];
	if (!pcsBase)
		return FALSE;

	BOOL			bResult			= FALSE;

	AgpdSkill		*pcsSkill		= pThis->m_pagpmSkill->GetSkill((INT32)pstAction->m_lUserData[0]);
	if (!pcsSkill)
	{
		pcsSkill	= pThis->GetTempSkill(pcsBase, (INT32)pstAction->m_lUserData[2]);
		if(pcsSkill)
		{
			pcsSkill->m_bForceAttack = (BOOL) pstAction->m_bForceAction;
			bResult	= pThis->CastSkill(pcsSkill, (BOOL) pstAction->m_lUserData[4]); 
		}
	}
	else
	{
		//if (!pcsSkill || !pcsSkill->m_Mutex.WLock())
		if (!pcsSkill->m_pcsBase || pcsSkill->m_pcsBase != pcsBase)
			return FALSE;

		if (pThis->m_pagpmSkill->IsPassiveSkill(pcsSkill))
		{
			// passive skill은 사용자가 임의로 casting 할 수 없다.
			/////////////////////////////////////////////////////////////////////
			//pcsSkill->m_Mutex.Release();
			return FALSE;
		}

		pcsSkill->m_csTargetBase.m_eType	= pstAction->m_csTargetBase.m_eType;
		pcsSkill->m_csTargetBase.m_lID		= pstAction->m_csTargetBase.m_lID;

		pcsSkill->m_bForceAttack			= (BOOL) pstAction->m_bForceAction;

		bResult	= pThis->CastSkill(pcsSkill);
	}
	
	//pcsSkill->m_Mutex.Release();

	return bResult;
}

BOOL AgsmSkill::CBCreateCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	/*
	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	// default skill이 있으면 구걸 생성해준다.

	if (!pcsCharacter->m_pcsCharacterTemplate)
		return FALSE;

	AgpdSkillTemplateAttachData *pcsTemplateAttachData = pThis->m_pagpmSkill->GetAttachSkillTemplateData(pcsCharacter->m_pcsCharacterTemplate);
	if (!pcsTemplateAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_DEFAULT_SKILL; i++)
	{
		if (pcsTemplateAttachData->m_alDefaultTID[i] == AP_INVALID_SKILLID)
			break;

		// default 스킬을 생성해준다.
	}
	*/

	return TRUE;
}

BOOL AgsmSkill::CBRemoveCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	AgpdSkillAttachData	*pcsSkillAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase *) pcsCharacter);
	if (!pcsSkillAttachData)
		return FALSE;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBRemoveCharacter"));

	// character가 event에 등록되어 있는지 보고 있으면 삭제한다.
	if (pcsSkillAttachData->m_alUseSkillID[0] != AP_INVALID_SKILLID)
	{
		ApdEvent *pcsEvent = pThis->m_papmEventManager->GetEvent((ApBase *) pcsCharacter, APDEVENT_FUNCTION_SKILL);
		if (pcsEvent)
			pThis->m_papmEventManager->RemoveEvent(pcsEvent);
	}

	// 현재 사용하고 있는 스킬들을 EndSkill 한다.
	for (int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		AgpdSkill	*pcsSkill = pThis->m_pagpmSkill->GetSkill(pcsSkillAttachData->m_alUseSkillID[i]);
		if (!pcsSkill)
			continue;

		pThis->EndSkill(pcsSkill);
	}

	pThis->EndAllBuffedSkill((ApBase *) pcsCharacter, FALSE);

	pThis->EndAllPassiveSkill((ApBase *) pcsCharacter);

	// 아크로드 일지도 모르니 한번 불러준다. 2006.10.24. steeple
	pThis->RemoveArchlordSkill(pcsCharacter);

	return TRUE;
}

BOOL AgsmSkill::CBUpdateCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	UINT32			ulClockCount	= PtrToUint(pCustData);

	if (!pThis->m_pagsmCharacter->IsIdleProcessTime(pcsCharacter, AGSDCHAR_IDLE_TYPE_SKILL, ulClockCount))
		return TRUE;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBUpdateCharacter"));

	pThis->m_pagsmCharacter->ResetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_SKILL);
	pThis->m_pagsmCharacter->SetProcessTime(pcsCharacter, AGSDCHAR_IDLE_TYPE_SKILL, ulClockCount);

	pThis->ProcessBuffedSkill((ApBase *) pcsCharacter, ulClockCount);

	AgpdSkillAttachData	*pcsAttachData	= pThis->m_pagpmSkill->GetAttachSkillData((ApBase *) pcsCharacter);
	if (pcsAttachData)
	{
		// 버프된 스킬이 있는지 검사한다.
		if (pcsAttachData->m_astBuffSkillList[0].lSkillID != AP_INVALID_SKILLID)
			pThis->m_pagsmCharacter->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_SKILL, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);
	}

	// 마스터리에 Instant specialize가 있었는지 보고 있었다면 끝나는 시간이 되었는지 살펴본다.
//	AgpdSkillMastery	*pcsSkillMastery	= pThis->m_pagpmSkill->GetMastery((ApBase *) pcsCharacter);
//	if (!pcsSkillMastery)
//		return FALSE;
//
//	BOOL	bIsSpecialized	= FALSE;
//
//	for (int i = 0; i < AGPMSKILL_MAX_MASTERY; ++i)
//	{
//		if (pcsSkillMastery->cInstantSpecialization == AGPMSKILL_MASTERY_SPECIALIZED_NONE)
//			continue;
//
//		bIsSpecialized	= TRUE;
//
//		for (int j = 0; j < AGPMSKILL_MAX_INSTANT_SPECIALIZE_IN_MASTERY; ++j)
//		{
//			if (pcsSkillMastery->cInstantSpecializedType[j] != AGPMSKILL_MASTERY_SPECIALIZED_NONE)
//			{
//				if (pcsSkillMastery->ulEndInstantSpecializedTime[j] <= ulClockCount)
//				{
//					// 마스터리에서 특화된 넘들중 이넘이 시간이 다되었다. 없애야한다.
//					pThis->m_pagpmSkill->MasteryUnSpecialize((ApBase *) pcsCharacter, i, (eAgpmSkillMasterySpecializedType) pcsSkillMastery->cInstantSpecializedType[j]);
//				}
//			}
//		}
//	}
//
//	if (bIsSpecialized)
//		pThis->m_pagsmCharacter->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_SKILL, AGSDCHAR_IDLE_INTERVAL_TEN_SECONDS);

	return TRUE;
}

BOOL AgsmSkill::CBEnterGameWorld(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill			*pThis				= (AgsmSkill *)			pClass;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)		pData;

	pThis->EnterGameWorld(pcsCharacter);	

	return TRUE;
}

BOOL AgsmSkill::EnterGameWorld(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	/*AgsdSkillADBase* pcsADBase = GetADBase((ApBase *) pcsCharacter);
	if (!pcsADBase)
		return FALSE;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill = m_pagpmSkill->CreateSkill();
		if (!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			return FALSE;
	}*/

	// 현재 이넘이 갖고 있는 Passive Skill을 모두 Cast 시켜준다.
	//////////////////////////////////////////////////////////////
	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData((ApBase *) pcsCharacter);
	if (!pcsAttachData)
		return FALSE;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill* pcsSkill = m_pagpmSkill->GetSkill(pcsAttachData->m_alSkillID[i]);
		if (!pcsSkill)
			continue;

		if (m_pagpmSkill->IsPassiveSkill(pcsSkill))
		{
			CastPassiveSkill(pcsSkill);
		}
	}

	// 아크로드일지도 모르니 한번 불러준다. 2006.10.24. steeple
	if(m_pagpmArchlord->IsArchlord(pcsCharacter->m_szID))
	{
		m_szArchlord.SetText(pcsCharacter->m_szID);
		LearnArchlordSkill(pcsCharacter);
	}

	return TRUE;
}

BOOL AgsmSkill::CBReceiveAction(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill				*pThis			= (AgsmSkill *)				pClass;
	pstAgpmSkillActionData	pstActionData	= (pstAgpmSkillActionData)	pData;

	if(!pstActionData)
		return FALSE;

	AgpdSkill				*pcsSkill		= pThis->m_pagpmSkill->GetSkill(pstActionData->lSkillID);
	if (!pcsSkill)
		return FALSE;

	if (pcsSkill->m_pcsBase != pstActionData->pcsOwnerBase)
		return FALSE;

	pcsSkill->m_csTargetBase.m_eType	= pstActionData->csTargetBase[0].m_eType;
	pcsSkill->m_csTargetBase.m_lID		= pstActionData->csTargetBase[0].m_lID;
	pcsSkill->m_lTargetUnion			= pstActionData->lTargetUnion;
	pcsSkill->m_posTarget				= pstActionData->csTargetPos;
	pcsSkill->m_lTargetHitRange			= pstActionData->lTargetHitRange;
	pcsSkill->m_bForceAttack			= pstActionData->bForceAttack;

	BOOL	bResult = FALSE;
	switch(pstActionData->nActionType)
	{
		case AGPMSKILL_ACTION_START_CAST_SKILL:
			{
				bResult	= pThis->CastSkill(pcsSkill);
			} break;
		case AGPMSKILL_ACTION_CANCEL_CAST_SKILL:
			{
				bResult	= pThis->CancelSkill(pcsSkill);
			} break;
		default:
			{
			} break;
	}

	return bResult;
}

BOOL AgsmSkill::CBReceiveRealHit(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	ApBase* pcsBase = (ApBase*)pData;
	AgsmSkill* pThis = (AgsmSkill*)pClass;
	INT32* plSkillID = (INT32*)pCustData;

	AgpdSkill* pcsSkill = pThis->m_pagpmSkill->GetSkill(*plSkillID);
	if(!pcsSkill)
		return FALSE;

	if(pcsSkill->m_pcsBase != pcsBase)
		return FALSE;

	BOOL bResult = FALSE;
	if(pThis->IsSetTwicePacketSkill(pcsSkill))
		bResult = pThis->ApplySkillTwicePacket(pcsSkill, NULL);

	return bResult;
}

BOOL AgsmSkill::CBMeleeAttack(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmSkill::CBMeleeAttack");

	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill					*pThis				= (AgsmSkill *)					pClass;
	pstAgsmCombatAttackResult	pstAttackResult		= (pstAgsmCombatAttackResult)	pData;
	INT32						*plReflectDamage	= (INT32 *) pCustData;

	INT32						lDamage				= pstAttackResult->nDamage;

	if (pstAttackResult->nDamage < 0)
		return FALSE;

	AgpdSkillAttachData	*pcsAttackAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase *) pstAttackResult->pAttackChar);
	AgpdSkillAttachData	*pcsTargetAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase *) pstAttackResult->pTargetChar);

	if (pcsTargetAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_DEFENSE_MELEE_ATTACK)
	{
		// damage를 방어한다.
		if (lDamage <= pcsTargetAttachData->m_nMeleeDefensePoint)
		{
			pcsTargetAttachData->m_nMeleeDefensePoint -= lDamage;
			lDamage = 0;
		}
		else
		{
			lDamage -= pcsTargetAttachData->m_nMeleeDefensePoint;
			pcsTargetAttachData->m_nMeleeDefensePoint = 0;
		}
	}

	// 2004.09.21. steeple 변경
	// 반격 데미지를 계산한다.
	if(pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability > 0 ||
	   pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicProbability > 0)
	{
		INT32 lRandNumber = pThis->m_csRandom.randInt(100);
		if(pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability >= lRandNumber ||
		   pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicProbability >= lRandNumber)
		{
			*plReflectDamage += (INT32)(pstAttackResult->nDamage * pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectAmount / 100.0f);
			if(pstAttackResult->nHeroicDamage > 0)
			{
				*plReflectDamage += (INT32)(pstAttackResult->nHeroicDamage * pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamageReflectHeroicAmount / 100.0f);
			}
			pstAttackResult->ulCheckResult |= AGSMCOMBAT_CHECKRESULT_REFLECT_DAMAGE;

			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
			AgsdSkillADBase	*pcsAgsdSkillADBase	= pThis->GetADBase((ApBase *) pstAttackResult->pTargetChar);

			for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
			{
				if (pcsTargetAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
					!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
					break;

				AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

				if (pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability >= lRandNumber)
				{
					pstAttackResult->lSkillTID	= pcsTargetAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
					break;
				}

				lRandNumber	-= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageReflectProbability;
			}
			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
		}
	}

	// 2004.09.21. steeple 변경
	// 데미지를 무시하는 지 체크한다.
	if(pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability > 0)
	{
		INT32 lRandNumber = pThis->m_csRandom.randInt(100);
		if(pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability >= lRandNumber)
		{
			lDamage = 0;
			pstAttackResult->ulCheckResult |= AGSMCOMBAT_CHECKRESULT_REDUCE_DAMAGE;

			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
			AgsdSkillADBase	*pcsAgsdSkillADBase	= pThis->GetADBase((ApBase *) pstAttackResult->pTargetChar);

			for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
			{
				if (pcsTargetAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
					!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
					break;

				AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

				if (pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability >= lRandNumber)
				{
					pstAttackResult->lSkillTID	= pcsTargetAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
					break;
				}

				lRandNumber	-= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamageIgnoreProbability;
			}
			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
		}
	}

	// 2004.09.21. steeple 변경
	// 반격 데미지를 계산한다.
	//if (pcsTargetAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_REFLECT_MELEE_ATTACK)
	//{
	//	// 받은 물리 데미지 만큼을 반격한다.
	//	if (pcsTargetAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_REFLECT_RECEIVED_DAMAGE)
	//	{
	//		*plReflectDamage += pstAttackResult->nDamage;
	//	}
	//	else if (pcsTargetAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_REFLECT_DAMAGE_SHIELD)
	//	{
	//		if (pcsTargetAttachData->m_lDamageShield > pstAttackResult->nDamage)
	//			*plReflectDamage += pstAttackResult->nDamage;
	//		else
	//			*plReflectDamage += pcsTargetAttachData->m_lDamageShield;
	//	}
	//	// 계산된 반격포인트 한도 내에서 반격한다.
	//	else
	//	{
	//		if (pstAttackResult->nDamage <= pcsTargetAttachData->m_nMeleeReflectPoint)
	//		{
	//			*plReflectDamage += pstAttackResult->nDamage;
	//			pcsTargetAttachData->m_nMeleeReflectPoint -= pstAttackResult->nDamage;
	//		}
	//		else
	//		{
	//			*plReflectDamage += pcsTargetAttachData->m_nMeleeReflectPoint;
	//			pcsTargetAttachData->m_nMeleeReflectPoint = 0;
	//		}
	//	}
	//}

	pstAttackResult->nDamage = (INT32) lDamage;

	return TRUE;
}

BOOL AgsmSkill::CBMagicAttack(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill					*pThis				= (AgsmSkill *)					pClass;
	pstAgsmCombatAttackResult	pstAttackResult		= (pstAgsmCombatAttackResult)	pData;
	INT32						*plReflectDamage	= (INT32 *) pCustData;

	INT32						lDamage				= pstAttackResult->nDamage;

	if (pstAttackResult->nDamage < 0)
		return FALSE;

	AgpdSkillAttachData	*pcsAttackAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase *) pstAttackResult->pAttackChar);
	AgpdSkillAttachData	*pcsTargetAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase *) pstAttackResult->pTargetChar);

	if (pcsTargetAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_DEFENSE_MAGIC_ATTACK)
	{
		// damage를 방어한다.
		if (lDamage <= pcsTargetAttachData->m_nMagicDefensePoint)
		{
			pcsTargetAttachData->m_nMagicDefensePoint -= lDamage;
			lDamage = 0;
		}
		else
		{
			lDamage -= pcsTargetAttachData->m_nMagicDefensePoint;
			pcsTargetAttachData->m_nMagicDefensePoint = 0;
		}
	}

	// 반격 데미지를 계산한다.
	if (pcsTargetAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_REFLECT_MAGIC_ATTACK)
	{
		// 받은 물리 데미지 만큼을 반격한다.
		if (pcsTargetAttachData->m_nDefenseTypeEffect & AGPMSKILL_EFFECT_REFLECT_RECEIVED_DAMAGE)
		{
			*plReflectDamage = pstAttackResult->nDamage;
		}
		// 계산된 반격포인트 한도 내에서 반격한다.
		else
		{
			if (pstAttackResult->nDamage <= pcsTargetAttachData->m_nMagicReflectPoint)
			{
				*plReflectDamage = pstAttackResult->nDamage;
				pcsTargetAttachData->m_nMagicReflectPoint -= pstAttackResult->nDamage;
			}
			else
			{
				*plReflectDamage = pcsTargetAttachData->m_nMagicReflectPoint;
				pcsTargetAttachData->m_nMagicReflectPoint = 0;
			}
		}
	}

	pstAttackResult->nDamage = (INT32) lDamage;

	return TRUE;
}

BOOL AgsmSkill::CBPreCheckCombat(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill					*pThis				= (AgsmSkill *)					pClass;
	pstAgsmCombatAttackResult	pstCombatResult		= (pstAgsmCombatAttackResult)	pData;

	if (!pstCombatResult->pTargetChar)
		return FALSE;

	// 공격자가 투명이면 풀어준다. 2005.10.07. steeple
	if(pstCombatResult->pAttackChar && pThis->m_pagpmCharacter->IsStatusTransparent(pstCombatResult->pAttackChar))
		pThis->m_pagpmCharacter->UpdateUnsetSpecialStatus(pstCombatResult->pAttackChar, AGPDCHAR_SPECIAL_STATUS_TRANSPARENT);

	AgpdSkillAttachData			*pcsAttachData				= NULL;
	AgpdSkillAttachData			*pcsAttackerAttachData		= NULL;

	AgsdCharacter				*pcsAttackAgsdCharacter		= pThis->m_pagsmCharacter->GetADCharacter(pstCombatResult->pAttackChar);
	if(NULL == pcsAttackAgsdCharacter)
		return FALSE;

	// 피공격자에 대한 처리
	//////////////////////////////////////////////////////////////////////////////
	pcsAttachData				= pThis->m_pagpmSkill->GetAttachSkillData(pstCombatResult->pTargetChar);
	if (!pcsAttachData)
		return FALSE;

	// 공격자에 대한 처리
	//////////////////////////////////////////////////////////////////////////////
	pcsAttackerAttachData		= pThis->m_pagpmSkill->GetAttachSkillData(pstCombatResult->pAttackChar);
	if(!pcsAttackerAttachData)
		return FALSE;


	// process counter attack
	if ((pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_DEFENSE_MELEE_ATTACK) &&
		(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_DEFENSE_COUNTER_ATTACK))
	{
		// 먼저 확률을 계산한다.
		INT32	lRandomNumber	= pThis->m_csRandom.randInt(100);
		if (lRandomNumber <= pcsAttachData->m_stBuffedSkillCombatEffectArg.lCounterAttackProbability)
		{
			// 확률 체크 
			pstCombatResult->ulCheckResult	|=	AGSMCOMBAT_CHECKRESULT_FAILED_ATTACK;
			pstCombatResult->ulCheckResult	|=	AGSMCOMBAT_CHECKRESULT_MELEE_COUNTERATTACK;

			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
			AgsdSkillADBase	*pcsAgsdSkillADBase	= pThis->GetADBase((ApBase *) pstCombatResult->pTargetChar);

			for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
			{
				if (pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
					!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
					break;

				AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

				if (pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lCounterAttackProbability >= lRandomNumber)
				{
					pstCombatResult->lSkillTID	= pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
					break;
				}

				lRandomNumber	-= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lCounterAttackProbability;
			}
			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
		}
	}

	// process ignore physical attack.
	// 2008.06.11. iluvs
	if (pcsAttackerAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] & AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK_IGNORE_PHYSICAL
		|| pcsAttackAgsdCharacter->m_stOptionSkillData.m_lIgnorePhysicalDefenseRate > 0) 
	{
		// 먼저 확률을 계산한다. (BuffredSkill + ItemOption에 따른 Skill효과)
		INT32	lRandomNumber	= pThis->m_csRandom.randInt(100);
		if (lRandomNumber <= pcsAttackerAttachData->m_stBuffedSkillCombatEffectArg.lIgnorePhysicalDefence + pcsAttackAgsdCharacter->m_stOptionSkillData.m_lIgnorePhysicalDefenseRate)
		{
			pstCombatResult->ulCheckResult	|=	AGSMCOMBAT_CHECKRESULT_IGNORE_PHYSICAL_DEFENCE;
		}		
	}

	// process ignore Attribute attack.
	// 2008.06.11. iluvs
	if (pcsAttackerAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK] & AGPMSKILL_EFFECT_DETAIL_REFLECT_MELEE_ATK_IGNORE_ATTRIBUTE
		|| pcsAttackAgsdCharacter->m_stOptionSkillData.m_lIgnoreAttributeDefenseRate) 
	{
		// 먼저 확률을 계산한다. (BuffredSkill + ItemOption에 따른 Skill효과)
		INT32	lRandomNumber	= pThis->m_csRandom.randInt(100);
		if (lRandomNumber <= pcsAttackerAttachData->m_stBuffedSkillCombatEffectArg.lIgnoreAttributeDefence + pcsAttackAgsdCharacter->m_stOptionSkillData.m_lIgnoreAttributeDefenseRate)
		{
			pstCombatResult->ulCheckResult	|=	AGSMCOMBAT_CHECKRESULT_IGNORE_ATTRIBUTE_DEFENCE;
		}		
	}


	// 2005.12.22. steeple
	// Evade 체크는 바뀌었다. Combat 쪽에서 하게끔 변경.
	//
	//
	// process evade
	//else if ((pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_DEFENSE_MELEE_ATTACK) &&
	//		 (pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_DEFENSE_EVADE))
	//{
	//	// 2005.03.27. steeple
	//	// 손에 뭘 들고 있는 지 체크해서 Evade 를 적용할 지 Dodge 를 적용할 지 계사한다.
	//	INT32 lEvadeRate = 0;
	//	BOOL bDodge = FALSE;
	//	if(pThis->m_pagpmItem->IsEquipStaff(pstCombatResult->pAttackChar) ||
	//		pThis->m_pagpmItem->IsEquipWand(pstCombatResult->pAttackChar) ||
	//		pThis->m_pagpmItem->IsEquipBow(pstCombatResult->pAttackChar) ||
	//		pThis->m_pagpmItem->IsEquipCrossBow(pstCombatResult->pAttackChar) ||
	//		pThis->m_pagpmItem->IsEquipKatariya(pstCombatResult->pAttackChar) ||
	//		pThis->m_pagpmItem->IsEquipChakram(pstCombatResult->pAttackChar))
	//	{
	//		lEvadeRate = pcsAttachData->m_stBuffedSkillCombatEffectArg.lDodgeProbability;	// Dodge 적용
	//		bDodge = TRUE;
	//	}
	//	else
	//		lEvadeRate = pcsAttachData->m_stBuffedSkillCombatEffectArg.lEvadeProbability;	// Evade 적용
	//	
	//	// 먼저 확률을 계산한다.
	//	INT32	lRandomNumber	= pThis->m_csRandom.randInt(100);
	//	if (lRandomNumber <= lEvadeRate && lEvadeRate > 0)
	//	{
	//		// 확률 체크 OK
	//		pstCombatResult->ulCheckResult	|=	AGSMCOMBAT_CHECKRESULT_FAILED_ATTACK;
	//		pstCombatResult->ulCheckResult	|=	AGSMCOMBAT_CHECKRESULT_EVADE_ATTACK;

	//		// Buffed Skill 중에서 이 스킬을 찾아낸다.
	//		/////////////////////////////////////////////////////////
	//		AgsdSkillADBase	*pcsAgsdSkillADBase	= pThis->GetADBase((ApBase *) pstCombatResult->pTargetChar);

	//		for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
	//		{
	//			if (pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
	//				!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
	//				break;

	//			AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

	//			if ((!bDodge && pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lEvadeProbability >= lRandomNumber) ||
	//				bDodge && pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDodgeProbability >= lRandomNumber)
	//			{
	//				pstCombatResult->lSkillTID	= pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
	//				break;
	//			}

	//			if(!bDodge)
	//				lRandomNumber	-= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lEvadeProbability;
	//			else
	//				lRandomNumber	-= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDodgeProbability;
	//		}
	//		// Buffed Skill 중에서 이 스킬을 찾아낸다.
	//		/////////////////////////////////////////////////////////
	//	}
	//}

	return TRUE;
}

BOOL AgsmSkill::CBPostCheckCombat(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill					*pThis				= (AgsmSkill *)					pClass;
	pstAgsmCombatAttackResult	pstCombatResult		= (pstAgsmCombatAttackResult)	pData;

	if (!pstCombatResult->pAttackChar)
		return FALSE;

	AgpdSkillAttachData			*pcsAttachData		= NULL;
	AgpdSkillAttachData			*pcsDamageAttachData= NULL;

	// 공격자에 대한 처리
	//////////////////////////////////////////////////////////////////////////////
	pcsAttachData				= pThis->m_pagpmSkill->GetAttachSkillData(pstCombatResult->pAttackChar);
	if (!pcsAttachData)
		return FALSE;

	// 피공격자에 대한 처리
	//////////////////////////////////////////////////////////////////////////////
	pcsDamageAttachData			= pThis->m_pagpmSkill->GetAttachSkillData(pstCombatResult->pTargetChar);
	if (!pcsDamageAttachData)
		return FALSE;

	AgsdCharacter* pAgsdAttacker = pThis->m_pagsmCharacter->GetADCharacter(pstCombatResult->pAttackChar);
	AgsdCharacter* pAgsdTarget	 = pThis->m_pagsmCharacter->GetADCharacter(pstCombatResult->pTargetChar);
	if(!pAgsdAttacker || !pAgsdTarget)
		return FALSE;

	BOOL bDamageAdded = FALSE;

	// Penalty For level Gap
	INT32 lPenalty		= pThis->m_pagpmCombat->CalcDefPenaltyLevelGap(pstCombatResult->pAttackChar, pstCombatResult->pTargetChar);
	INT32 lAtkPenalty	= pThis->m_pagpmCombat->CalcAtkPenaltyLevelGap(pstCombatResult->pAttackChar, pstCombatResult->pTargetChar);

	// 2005.05.17. steeple
	// Death Strike 변경
	if(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_MELEE_ATTACK_DEATH)
	{
		// check death strike
		INT32	lDeathRandomNumber	= pThis->m_csRandom.randInt(100);
		if (lDeathRandomNumber < pcsAttachData->m_stBuffedSkillCombatEffectArg.lDeathProbability)
		{
			bDamageAdded = TRUE;

			// death strike 발동
			pstCombatResult->ulCheckResult	|=	AGSMCOMBAT_CHECKRESULT_MELEE_DEATH_STRIKE;

			// 2005.05.17. steeple
			// 기존 Damage 에다가 타겟의 현재 HP 의 해당 % 만큼을 더해준다.
			INT32	lTargetCurrentHP	= 0;
			pThis->m_pagpmFactors->GetValue(&pstCombatResult->pTargetChar->m_csFactor, &lTargetCurrentHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);

			INT32 lOriginalDamage = pstCombatResult->nDamage;
			if (lTargetCurrentHP > 0 && lTargetCurrentHP > pstCombatResult->nDamage)
				pstCombatResult->nDamage += (INT32)((FLOAT)(lTargetCurrentHP * pcsAttachData->m_stBuffedSkillCombatEffectArg.lDeathAmount) / 100.0f);

			// 2005.05.19. steeple
			// 원래 데미지의 5배를 넘어가면 5배로 고정
			if((FLOAT)pstCombatResult->nDamage > (FLOAT)lOriginalDamage * 4.9f)
				pstCombatResult->nDamage = (INT32)((FLOAT)lOriginalDamage * 4.9f + 0.5f);

			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
			AgsdSkillADBase	*pcsAgsdSkillADBase	= pThis->GetADBase((ApBase *) pstCombatResult->pTargetChar);

			BOOL bFound = FALSE;
			for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
			{
				if (pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
					!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
					break;

				AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

				if (pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDeathProbability >= lDeathRandomNumber)
				{
					pstCombatResult->lSkillTID	= pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
					bFound = TRUE;
					break;
				}

				lDeathRandomNumber	-= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDeathProbability;
			}

			// 2005.05.17. steeple
			if(!bFound)
			{
				// Passive Skill 에서도 돌아본다.
				AgpdSkill* pcsActionSkill = pThis->m_pagpmSkill->GetSkillByAction(pstCombatResult->pAttackChar,
																AGPDCHAR_ACTION_RESULT_TYPE_DEATH_STRIKE);
				if(pcsActionSkill)
				{
					pstCombatResult->lSkillTID = ((AgpdSkillTemplate*)pcsActionSkill->m_pcsTemplate)->m_lID;
				}
			}
			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
		}
	}

	// critical strike
	if (!bDamageAdded &&
		(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_MELEE_ATTACK_CRITICAL 
			|| pAgsdAttacker->m_stOptionSkillData.m_lCritical > 0))
	{
		// 먼저 확률을 계산한다.
		INT32	lRandomNumber	= pThis->m_csRandom.randInt(100);
		if (lRandomNumber <= pcsAttachData->m_stBuffedSkillCombatEffectArg.lMeleeCriticalProbability 
								+ pAgsdAttacker->m_stOptionSkillData.m_lCriticalRate
								- lAtkPenalty)
		{
			bDamageAdded = TRUE;

			// critical strike 발동
			pstCombatResult->ulCheckResult	|=	AGSMCOMBAT_CHECKRESULT_MELEE_CRITICAL_STRIKE;

			// damage를 정해진 값만큼 증감시킨다.
			INT32 nAddDamageRatio = pcsAttachData->m_stBuffedSkillCombatEffectArg.lMeleeDamageAdjustRate 
									+ pAgsdAttacker->m_stOptionSkillData.m_lCritical;
			if(nAddDamageRatio > 0)
				pstCombatResult->nDamage = (INT32) (pstCombatResult->nDamage * (nAddDamageRatio/100.0));

			// Critical 방어가 있으면 그만큼 Target의 데미지를 줄여준다.
			if(pcsDamageAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_CRITICAL_DEFENCE_PERCENT
				|| pAgsdTarget->m_stOptionSkillData.m_lCriticalDefenseRate > 0)
			{
				pstCombatResult->nDamage	= (INT32) (pstCombatResult->nDamage 
					- (pstCombatResult->nDamage 
					* (( pThis->m_pagsmCombat->GetCriticalDefenceRate(pstCombatResult->pAttackChar, pstCombatResult->pTargetChar)
					- lPenalty) / 100.0)));
			}

			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
			AgsdSkillADBase	*pcsAgsdSkillADBase	= pThis->GetADBase((ApBase *) pstCombatResult->pTargetChar);

			for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
			{
				if (pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
					!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
					break;

				AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

				if (pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lMeleeCriticalProbability >= lRandomNumber)
				{
					pstCombatResult->lSkillTID	= pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
					break;
				}

				lRandomNumber	-= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lMeleeCriticalProbability;
			}
			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
		}
	}

	// 이도 저도 아니고 걍 일반 공격이 들어오게 되었다.
	if(!bDamageAdded)
	{
		AgsdSkillADBase	*pcsADBase	= pThis->GetADBase((ApBase *) pstCombatResult->pAttackChar);
		if (!pcsADBase)
			return FALSE;

		if (pcsADBase->m_lDamageAdjustRate > 0)
		{
			pstCombatResult->nDamage	= (INT32) (pstCombatResult->nDamage * pcsADBase->m_lDamageAdjustRate);
		}
	}

	// 2005.07.12. steeple
	// 공격 히트시 히트 데미지를 HP 로 변환하는 지 체크
	if((pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_CONVERT) &&
		(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] & AGPMSKILL_EFFECT_DETAIL_CONVERT_ATK_DAMAGE_TO_HP))
	{
		// 확률 계산
		INT32 lRandomNumber = pThis->m_csRandom.randInt(100);
		if(lRandomNumber <= pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[1] && lRandomNumber < 80)
		{
			pstCombatResult->ulCheckResult |= AGSMCOMBAT_CHECKRESULT_CONVERT_ATK_DAMAGE_TO_HP;
			
			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
			AgsdSkillADBase* pcsAgsdSkillADBase	= pThis->GetADBase((ApBase*)pstCombatResult->pAttackChar);

			for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
			{
				if (pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
					!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
					break;

				AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

				if (pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[1] >= lRandomNumber)
				{
					pstCombatResult->lSkillTID = pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
					break;
				}

				lRandomNumber -= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[1];
			}
			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
		}
	}

	// 2005.07.12. steeple
	// 공격 히트시 히트 데미지를 MP 로 변환하는 지 체크
	if((pcsAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_CONVERT) &&
		(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] & AGPMSKILL_EFFECT_DETAIL_CONVERT_ATK_DAMAGE_TO_MP))
	{
		// 확률 계산
		INT32 lRandomNumber = pThis->m_csRandom.randInt(100);
		if(lRandomNumber <= pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[1] && lRandomNumber < 80)
		{
			pstCombatResult->ulCheckResult |= AGSMCOMBAT_CHECKRESULT_CONVERT_ATK_DAMAGE_TO_MP;
			
			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
			AgsdSkillADBase* pcsAgsdSkillADBase	= pThis->GetADBase((ApBase*)pstCombatResult->pAttackChar);

			for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
			{
				if (pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
					!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
					break;

				AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

				if (pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[1] >= lRandomNumber)
				{
					pstCombatResult->lSkillTID = pcsAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
					break;
				}

				lRandomNumber -= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[1];
			}
			// Buffed Skill 중에서 이 스킬을 찾아낸다.
			/////////////////////////////////////////////////////////
		}
	}

	// Option Stun 체크 2006.12.19. steeple
	pThis->ProcessOptionStun(pstCombatResult->pAttackChar, pstCombatResult->pTargetChar);

	// Charge 를 확인해서 처리해준다. 2005.01.25. stepele 변경.
	//pThis->ProcessChargeAttack(pstCombatResult);

	// 공격자의 상태가 NOT_ADD_AGRO 일때 공격시에 풀린다.
	if(pstCombatResult->pAttackChar->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_NOT_ADD_AGRO)
		pThis->m_pagpmCharacter->UpdateUnsetSpecialStatus(pstCombatResult->pAttackChar, AGPDCHAR_SPECIAL_STATUS_NOT_ADD_AGRO);

	// 공격자의 상태가 HIDE_AGRO 일때 공격시에 풀린다.
	if(pstCombatResult->pAttackChar->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_HIDE_AGRO)
		pThis->m_pagpmCharacter->UpdateUnsetSpecialStatus(pstCombatResult->pAttackChar, AGPDCHAR_SPECIAL_STATUS_HIDE_AGRO);

	// 2005.12.14. steeple. 공격자의 렌즈 스톤 처리.
	if(pThis->m_pagpmSkill->IsStatusActionOnActionType4((ApBase*)pstCombatResult->pAttackChar))
		pstCombatResult->ulCheckResult |= AGSMCOMBAT_CHECKRESULT_LENS_STONE;

	// Target 에 대한 처리 - 2004.11.04. steeple
	AgpdSkillAttachData* pcsTargetAttachData = pThis->m_pagpmSkill->GetAttachSkillData(pstCombatResult->pTargetChar);
	if(pcsTargetAttachData)
	{
		AgsdCharacter* pAgsdTarget = pThis->m_pagsmCharacter->GetADCharacter(pstCombatResult->pTargetChar);

		// 2004.11.04. steeple
		// PreCheckCombat 에 있던 것을 PostCheckCombat 으로 이동
		// convert damage to hp
		if(((pcsTargetAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_CONVERT) &&
			(pcsTargetAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] & AGPMSKILL_EFFECT_DETAIL_CONVERT_DAMAGE_TO_HP))
			||
			(pAgsdTarget->m_stOptionSkillData.m_lDamageConvertHPRate > 0))
		{

			// 확률 계산
			INT32 lRandomNumber = pThis->m_csRandom.randInt(100);
			INT32 lProbability = pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0] + pAgsdTarget->m_stOptionSkillData.m_lDamageConvertHPRate - lPenalty;
			if(lProbability > 80)
				lProbability = 80;

			if(lRandomNumber <= lProbability)
			{
				pstCombatResult->ulCheckResult |= AGSMCOMBAT_CHECKRESULT_CONVERT_DAMAGE_TO_HP;
				
				// Buffed Skill 중에서 이 스킬을 찾아낸다.
				/////////////////////////////////////////////////////////
				AgsdSkillADBase	*pcsAgsdSkillADBase	= pThis->GetADBase((ApBase *) pstCombatResult->pTargetChar);

				for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
				{
					if (pcsTargetAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
						!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
						break;

					AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

					if (pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0] >= lRandomNumber)
					{
						pstCombatResult->lSkillTID	= pcsTargetAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
						break;
					}

					lRandomNumber	-= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2HPProbability[0];
				}
				// Buffed Skill 중에서 이 스킬을 찾아낸다.
				/////////////////////////////////////////////////////////
			}
		}

		// 2004.11.04. steeple
		// PreCheckCombat 에 있던 것을 PostCheckCombat 으로 이동
		// convert damage to mp
		if((pcsTargetAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_CONVERT) &&
			(pcsTargetAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_CONVERT] & AGPMSKILL_EFFECT_DETAIL_CONVERT_DAMAGE_TO_MP))
		{
			// 확률 계산
			INT32 lRandomNumber = pThis->m_csRandom.randInt(100);
			if(lRandomNumber <= pcsTargetAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[0] - lPenalty && lRandomNumber < 80)
			{
				pstCombatResult->ulCheckResult |= AGSMCOMBAT_CHECKRESULT_CONVERT_DAMAGE_TO_MP;

				// Buffed Skill 중에서 이 스킬을 찾아낸다.
				/////////////////////////////////////////////////////////
				AgsdSkillADBase	*pcsAgsdSkillADBase	= pThis->GetADBase((ApBase *) pstCombatResult->pTargetChar);

				for (int nBuffedIndex = 0; nBuffedIndex < AGPMSKILL_MAX_SKILL_BUFF; ++nBuffedIndex)
				{
					if (pcsTargetAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID == AP_INVALID_SKILLID ||
						!pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill)
						break;

					AgsdSkill	*pcsBuffedAgsdSkill	= pThis->GetADSkill(pcsAgsdSkillADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_pcsTempSkill);

					if (pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[0] >= lRandomNumber)
					{
						pstCombatResult->lSkillTID	= pcsTargetAttachData->m_astBuffSkillList[nBuffedIndex].lSkillTID;
						break;
					}

					lRandomNumber	-= pcsBuffedAgsdSkill->m_stBuffedSkillCombatEffectArg.lDamage2MPProbability[0];
				}
				// Buffed Skill 중에서 이 스킬을 찾아낸다.
				/////////////////////////////////////////////////////////
			}
		}

		// 2005.07.18. steeple
		// Action on Action Type3 체크
		if((pcsTargetAttachData->m_ullBuffedSkillCombatEffect2 & AGPMSKILL_EFFECT2_ACTOIN_ON_ACTION) &&
			(pcsTargetAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE3))
		{
			pThis->ProcessActionOnActionType3(pstCombatResult);
		}

		// 2007.06.27. steeple
		// Divide Damage 처리한다.
		pThis->ProcessDivideDamage(pstCombatResult);
	}

	// 2005.07.17. steeple
	// 다 처리하고나서 ActionPassive 를 해준다.
	pThis->ProcessActionPassiveAttack(pstCombatResult);

	return TRUE;
}

BOOL AgsmSkill::CBPreCheckCast(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill			*pThis			= (AgsmSkill *)			pClass;
	PVOID				*pvBuffer		= (PVOID *)				pData;

	AgpdSkill			*pcsSkill		= (AgpdSkill *)			pvBuffer[0];
	ApBase				*pcsTargetBase	= (ApBase *)			pvBuffer[1];
	UINT32				*pulCheckResult	= (UINT32 *)			pvBuffer[2];

	if (!pcsSkill || !pcsTargetBase || !pulCheckResult)
		return FALSE;

	///////////////////////////////////////////////////////////////////////////////////////////
	// 스킬을 받는 놈 검사를 한다. 2005.03.09. steeple
	AgpdSkillAttachData	*pcsTargetAttachData = pThis->m_pagpmSkill->GetAttachSkillData(pcsTargetBase);
	if(pcsTargetAttachData && pcsSkill->m_pcsTemplate)
	{
		INT32 lSkillLevel = pThis->GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			return FALSE;

		if(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_TYPE2][lSkillLevel]
			&& !pThis->m_pagpmSkill->IsSummonsSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
		{
			INT32 lPenalty = pThis->m_pagpmCombat->CalcDefPenaltyLevelGap((AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTargetBase);

			// Skill Block 가능한지 체크
			INT32 lSkillBlockRate = pThis->m_pagpmCombat->GetSkillBlockRate((AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTargetBase) - lPenalty;
			INT32 lRandom = pThis->m_csRandom.randInt(100);
			if(lRandom <= lSkillBlockRate)
			{
				// Result 에 BLOCK 을 세팅해준다.
				*pulCheckResult |= AGPDCHAR_ACTION_RESULT_TYPE_ATTACK_BLOCK;	
			}
		}
	}

	// 2005.12.22. steeple
	// Evade 체크는 바뀌었다.
	//
	//
	// process magic skill evade
	//if ((pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_DEFENSE_MAGIC_ATTACK) &&
	//	(pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_DEFENSE_EVADE))
	//{
	//	// check that this skill attribute is AGPMSKILL_SKILL_ATTR_TYPE_MAGIC type
	//	//////////////////////////////////////////////////////////////////////////
	//	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_nAttribute & AGPMSKILL_SKILL_ATTR_TYPE_MAGIC)
	//	{
	//		// 먼저 확률을 계산한다.
	//		INT32	lRandomNumber	= pThis->m_csRandom.randInt(100);
	//		if (lRandomNumber <= pcsAttachData->m_stBuffedSkillCombatEffectArg.lEvadeProbability)
	//		{
	//			// 확률 체크 OK
	//			*pulCheckResult	|=	AGSMSKILL_CHECKRESULT_FAILED_CAST;
	//			*pulCheckResult	|=	AGSMSKILL_CHECKRESULT_EVADE_SKILL;
	//		}
	//	}
	//}

	return TRUE;
}

BOOL AgsmSkill::CBPostCheckCast(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill			*pThis			= (AgsmSkill *)			pClass;
	PVOID				*pvBuffer		= (PVOID *)			pData;

	AgpdSkill			*pcsSkill		= (AgpdSkill *)			pvBuffer[0];
	ApBase				*pcsTargetBase	= (ApBase *)			pvBuffer[1];
	UINT32				*pulCheckResult	= (UINT32 *)			pvBuffer[2];

	if (!pcsSkill || !pcsSkill->m_pcsBase || !pcsTargetBase || !pulCheckResult)
		return FALSE;

	// 스킬을 시연하는 넘에 대해 검사한다.
	//////////////////////////////////////////////////////////////////////////////////
	AgpdSkillAttachData	*pcsAttachData	= pThis->m_pagpmSkill->GetAttachSkillData(pcsSkill->m_pcsBase);
	if (!pcsAttachData)
		return FALSE;

	// process spell critical strike
	if (pcsAttachData->m_ullBuffedSkillCombatEffect & AGPMSKILL_EFFECT_MAGIC_ATTACK_CRITICAL)
	{
		// magic skill damage를 주는 스킬인지 검사한다.
		//	check that skill effect type is AGPMSKILL_EFFECT_MAGIC_ATTACK_DMG
		/////////////////////////////////////////////////////////////////////
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[0] & AGPMSKILL_EFFECT_MAGIC_ATTACK_DMG)
		{
			// 먼저 확률을 계산한다.
			INT32	lRandomNumber	= pThis->m_csRandom.randInt(100);
			if (lRandomNumber <= pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicCriticalProbability)
			{
				// critical strike 발동
				*pulCheckResult	|=	AGSMSKILL_CHECKRESULT_SPELL_CRITICAL_STRIKE;

				// damage를 정해진 값만큼 증감시킨다.
				//
				AgsdSkill	*pcsAgsdSkill		= pThis->GetADSkill(pcsSkill);

				if (!pThis->SetUpdateFactorResult(pcsAgsdSkill))
				{
					*pulCheckResult = 0;
					return FALSE;
				}

				AgpdFactor	*pcsFactorResult	= (AgpdFactor *) pThis->m_pagpmFactors->SetFactor(pcsAgsdSkill->m_pcsUpdateFactorResult, NULL, AGPD_FACTORS_TYPE_RESULT);
				if (!pcsFactorResult)
				{
					*pulCheckResult = 0;
					return FALSE;
				}

				// 현재 Damage가 없는 경우 걍 넘어간다.
				AgpdFactorCharPoint *pcsFactorCharPoint = (AgpdFactorCharPoint *) pThis->m_pagpmFactors->GetFactor(pcsFactorResult, AGPD_FACTORS_TYPE_CHAR_POINT);
				if (!pcsFactorCharPoint)
				{
					*pulCheckResult = 0;
					return FALSE;
				}

				pcsFactorCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] *= (INT32) (pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicDamageAdjustRate / 100.0);
				pcsFactorCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_DMG_NORMAL] *= (INT32) (pcsAttachData->m_stBuffedSkillCombatEffectArg.lMagicDamageAdjustRate / 100.0);
			}
		}
	}

	/*
	///////////////////////////////////////////////////////////////////////////////////////////
	// 스킬을 받는 놈 검사를 한다. 2005.03.09. steeple
	AgpdSkillAttachData	*pcsTargetAttachData = pThis->m_pagpmSkill->GetAttachSkillData(pcsTargetBase);
	if(pcsTargetAttachData && pcsSkill->m_pcsTemplate)
	{
		INT32 lSkillLevel = pThis->GetModifiedSkillLevel(pcsSkill);
		if(!lSkillLevel)
			return FALSE;

		if(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_TYPE2][lSkillLevel])
		{
			INT32 lPenalty = pThis->m_pagpmCombat->CalcDefPenaltyLevelGap((AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTargetBase);

			// Skill Block 가능한지 체크
			INT32 lSkillBlockRate = pThis->m_pagpmCombat->GetSkillBlockRate((AgpdCharacter*)pcsTargetBase) - lPenalty;
			INT32 lRandom = pThis->m_csRandom.randInt(100);
			if(lRandom <= lSkillBlockRate)
			{
				// Result 에 BLOCK 을 세팅해준다.
				*pulCheckResult |= AGPDCHAR_ACTION_RESULT_TYPE_ATTACK_BLOCK;

				pThis->SendCastResult(pcsSkill, pcsSkill->m_pcsBase, pcsTargetBase, NULL, AGPMSKILL_ACTION_BLOCK_CAST_SKILL, FALSE, TRUE);			
			}
		}
	}
	*/

	return TRUE;
}

// 2004.09.17. steeple
// Melee Attack 이 끝나고 이것을 체크해준다.
BOOL AgsmSkill::CBConvertDamage(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	pstAgsmCombatAttackResult pstAttackResult = (pstAgsmCombatAttackResult)pData;
	AgsmSkill* pThis = (AgsmSkill*)pClass;

	if(!pstAttackResult->pTargetChar)
		return FALSE;

	if(pstAttackResult->nDamage == 0)
		return FALSE;

	// Target 의 Attach Data 를 얻는다.
	AgpdSkillAttachData* pcsAttachData = pThis->m_pagpmSkill->GetAttachSkillData(pstAttackResult->pTargetChar);
	if(!pcsAttachData)
		return FALSE;

	AgsdCharacter* pcsAgsdTarget = pThis->m_pagsmCharacter->GetADCharacter(pstAttackResult->pTargetChar);
	if(!pcsAgsdTarget)
		return FALSE;

	// 일단 변동 되는 값 들을 미리 구해놓고...
	INT32 lCurrentTargetHP = 0;
	INT32 lCurrentTargetMP = 0;
	INT32 lMaxTargetHP = 0;
	INT32 lMaxTargetMP = 0;

	pThis->m_pagpmFactors->GetValue(&pstAttackResult->pTargetChar->m_csFactor, &lCurrentTargetHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);
	pThis->m_pagpmFactors->GetValue(&pstAttackResult->pTargetChar->m_csFactor, &lCurrentTargetMP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_MP);
	pThis->m_pagpmFactors->GetValue(&pstAttackResult->pTargetChar->m_csFactor, &lMaxTargetHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_HP);
	pThis->m_pagpmFactors->GetValue(&pstAttackResult->pTargetChar->m_csFactor, &lMaxTargetMP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_MP);

	// 죽었으면 나간다.
	if(lCurrentTargetHP == 0)
		return TRUE;

	BOOL bResult = FALSE;
	PVOID pvPacket = NULL;
	
	if(pstAttackResult->ulCheckResult & AGSMCOMBAT_CHECKRESULT_CONVERT_DAMAGE_TO_HP)
	{
		bResult = TRUE;

		lCurrentTargetHP += (INT32)(pstAttackResult->nDamage *
									(pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[0] + pcsAgsdTarget->m_stOptionSkillData.m_lDamageConvertHP) /
									100.0f);
		if(lCurrentTargetHP > lMaxTargetHP)
			lCurrentTargetHP = lMaxTargetHP;

		pThis->SendAdditionalEffect(pstAttackResult->pTargetChar, AGPMSKILL_ADDITIONAL_EFFECT_CONVERT_HP);
	}

	if(pstAttackResult->ulCheckResult & AGSMCOMBAT_CHECKRESULT_CONVERT_DAMAGE_TO_MP)
	{
		bResult = TRUE;

		lCurrentTargetMP += (INT32)(pstAttackResult->nDamage * pcsAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[0] / 100.0f);
		if(lCurrentTargetMP > lMaxTargetMP)
			lCurrentTargetMP = lMaxTargetMP;

		pThis->SendAdditionalEffect(pstAttackResult->pTargetChar, AGPMSKILL_ADDITIONAL_EFFECT_CONVERT_MP);
	}

	if(bResult)
		pThis->m_pagsmFactors->UpdateCharPoint(&pstAttackResult->pTargetChar->m_csFactor, &pvPacket, lCurrentTargetHP, lCurrentTargetMP, 0);

	// 변경 했다면 패킷 보내버린다.
	if(pvPacket)
	{
		pThis->m_pagsmCharacter->SendPacketFactor(pvPacket, pstAttackResult->pTargetChar, PACKET_PRIORITY_4);
		pThis->m_pagpmCharacter->m_csPacket.FreePacket(pvPacket);
	}

	// 2005.07.12. steeple
	// Attacker 에 대한 처리.
	if(!pstAttackResult->pAttackChar)
		return TRUE;

	INT32 lCurrentAttackerHP = 0, lCurrentAttackerMP = 0;
	INT32 lMaxAttackerHP = 0, lMaxAttackerMP = 0;

	pThis->m_pagpmFactors->GetValue(&pstAttackResult->pAttackChar->m_csFactor, &lCurrentAttackerHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);
	pThis->m_pagpmFactors->GetValue(&pstAttackResult->pAttackChar->m_csFactor, &lCurrentAttackerMP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_MP);
	pThis->m_pagpmFactors->GetValue(&pstAttackResult->pAttackChar->m_csFactor, &lMaxAttackerHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_HP);
	pThis->m_pagpmFactors->GetValue(&pstAttackResult->pAttackChar->m_csFactor, &lMaxAttackerMP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_MP);

	// 죽었으면 나간다.
	if(lCurrentAttackerHP == 0)
		return TRUE;

	BOOL bResult2 = FALSE;
	PVOID pvPacket2 = NULL;
	
	// Attacker 의 Attach Data 를 얻는다.
	AgpdSkillAttachData* pcsAttackerAttachData = pThis->m_pagpmSkill->GetAttachSkillData(pstAttackResult->pAttackChar);
	if(!pcsAttackerAttachData)
		return FALSE;

	if(pstAttackResult->ulCheckResult & AGSMCOMBAT_CHECKRESULT_CONVERT_ATK_DAMAGE_TO_HP)
	{
		bResult2 = TRUE;

		lCurrentAttackerHP += (INT32)(pstAttackResult->nDamage * pcsAttackerAttachData->m_stBuffedSkillCombatEffectArg.lDamage2HPAmount[1] / 100.0f);
		if(lCurrentAttackerHP > lMaxAttackerHP)
			lCurrentAttackerHP = lMaxAttackerHP;

		pThis->SendAdditionalEffect(pstAttackResult->pAttackChar, AGPMSKILL_ADDITIONAL_EFFECT_ABSORB_HP);
	}

	if(pstAttackResult->ulCheckResult & AGSMCOMBAT_CHECKRESULT_CONVERT_ATK_DAMAGE_TO_MP)
	{
		bResult2 = TRUE;

		lCurrentAttackerMP += (INT32)(pstAttackResult->nDamage * pcsAttackerAttachData->m_stBuffedSkillCombatEffectArg.lDamage2MPAmount[1] / 100.0f);
		if(lCurrentAttackerMP > lMaxAttackerMP)
			lCurrentAttackerMP = lMaxAttackerMP;

		pThis->SendAdditionalEffect(pstAttackResult->pAttackChar, AGPMSKILL_ADDITIONAL_EFFECT_ABSORB_MP);
	}

	if(bResult2)
		pThis->m_pagsmFactors->UpdateCharPoint(&pstAttackResult->pAttackChar->m_csFactor, &pvPacket2, lCurrentAttackerHP, lCurrentAttackerMP, 0);

	// 변경 했다면 패킷 보내버린다.
	if(pvPacket2)
	{
		pThis->m_pagsmCharacter->SendPacketFactor(pvPacket2, pstAttackResult->pAttackChar, PACKET_PRIORITY_4);
		pThis->m_pagpmCharacter->m_csPacket.FreePacket(pvPacket2);
	}

	return TRUE;
}

// 2005.12.14. steeple
// 이제 이 콜백 쓰지 않는다. -0-;;
//
// 2005.12.13. steeple
// AgsmCombat 에서 ApplyDamage 후 불린다.
BOOL AgsmSkill::CBAttackFromCombat(PVOID pData, PVOID pClass, PVOID pCustData)
{
	//if(!pData || !pClass)
	//	return FALSE;

	//pstAgsmCombatAttackResult pstAttackResult = (pstAgsmCombatAttackResult)pData;
	//AgsmSkill* pThis = (AgsmSkill*)pClass;

	//if(!pstAttackResult->pAttackChar || !pstAttackResult->pTargetChar)
	//	return FALSE;

	//AgpdSkillAttachData* pcsAttachData = pThis->m_pagpmSkill->GetAttachSkillData(pstAttackResult->pAttackChar);
	//if(!pcsAttachData)
	//	return FALSE;

	//// Action On Action Type4 가 걸려 있는 상태라면 타겟에게 Additional Effect 뿌린다.
	//if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE4)
	//	pThis->SendAdditionalEffect((ApBase*)pstAttackResult->pTargetChar, AGPMSKILL_ADDITIONAL_EFFECT_LENS_STONE);

	return TRUE;
}


/*
BOOL AgsmSkill::CBDamageAdjust(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill					*pThis			= (AgsmSkill *)					pClass;
	pstAgsmCombatAttackResult	pstAttackResult	= (pstAgsmCombatAttackResult)	pData;
	INT16						*pnDamage		= (INT16 *)						pCustData;

	if (!pstAttackResult->pAttackChar)
		return FALSE;

	AgsdSkillADBase	*pcsADBase	= pThis->GetADBase((ApBase *) pstAttackResult->pAttackChar);
	if (!pcsADBase)
		return FALSE;

	if (pcsADBase->m_lDamageAdjustRate > 0)
	{
		*pnDamage	= (INT16) (pstAttackResult->nDamage * pcsADBase->m_lDamageAdjustRate);
	}

	return TRUE;
}
*/

BOOL AgsmSkill::CBUpdateSkillPoint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)	pClass;
	AgpdSkill		*pcsSkill		= (AgpdSkill *)	pData;
	INT32			lUpdatePoint	= *(INT32 *)	pCustData;

	// passive skill 인 경우 스킬 레벨이 바뀌었으니 처리해줘야 한다. (스킬을 다시 시전한다.)
	if (pThis->m_pagpmSkill->IsPassiveSkill(pcsSkill))
	{
		pThis->EndPassiveSkill(pcsSkill);

		pThis->CastPassiveSkill(pcsSkill);
		//JK_패시브스킬 표시 오류
		///////////////////////////
//		UINT32	dpnid = pThis->m_pagsmCharacter->GetCharDPNID((AgpdCharacter *) pcsSkill->m_pcsBase);
//		if(dpnid != 0)
//		{
//			INT16	nPacketLength = 0;
//			PVOID	pvPacket = pThis->m_pagpmSkill->MakePacketSkill(pcsSkill, &nPacketLength);

//			BOOL	bRetval = pThis->SendPacket(pvPacket, nPacketLength, dpnid);

//			pThis->m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
//		}
		//////////////////////////////
	}

	if (pcsSkill->m_pcsBase && pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		if (pThis->m_pAgsmServerManager->GetThisServerType() != AGSMSERVER_TYPE_GAME_SERVER)
			return TRUE;

		if (!((AgpdCharacter *) pcsSkill->m_pcsBase)->m_bIsAddMap)
			return TRUE;
	}

	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		INT16	nPacketLength = 0;
		PVOID	pvPacket = pThis->m_pagpmSkill->MakePacketSkillDIRT(pcsSkill, &nPacketLength);
		if (!pvPacket || nPacketLength < 1)
			return FALSE;

		UINT32	dpnid = pThis->m_pagsmCharacter->GetCharDPNID((AgpdCharacter *) pcsSkill->m_pcsBase);
		if (dpnid != 0)
		{
			BOOL	bSendResult = pThis->SendPacket(pvPacket, nPacketLength, dpnid);

			pThis->m_pagpmSkill->m_csPacket.FreePacket(pvPacket);

			/*
			// DB에 저장한다.
			if (pThis->m_pAgsmServerManager->GetThisServerType() == AGSMSERVER_TYPE_GAME_SERVER)
				pThis->SaveSkillDIRT(pcsSkill);
			*/

			return bSendResult;
		}

		pThis->m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
	}
	
	return FALSE;
}

BOOL AgsmSkill::CBUpdateDIRTPoint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)	pClass;
	AgpdSkill		*pcsSkill		= (AgpdSkill *)	pData;

	if (pcsSkill->m_pcsBase && pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		if (pThis->m_pAgsmServerManager->GetThisServerType() != AGSMSERVER_TYPE_GAME_SERVER)
			return TRUE;

		if (!((AgpdCharacter *) pcsSkill->m_pcsBase)->m_bIsAddMap)
			return TRUE;
	}

	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		INT16	nPacketLength = 0;
		PVOID	pvPacket = pThis->m_pagpmSkill->MakePacketSkillDIRT(pcsSkill, &nPacketLength);
		if (!pvPacket || nPacketLength < 1)
			return FALSE;

		UINT32	dpnid = pThis->m_pagsmCharacter->GetCharDPNID((AgpdCharacter *) pcsSkill->m_pcsBase);
		if (dpnid != 0)
		{
			BOOL	bSendResult = pThis->SendPacket(pvPacket, nPacketLength, dpnid);

			pThis->m_pagpmSkill->m_csPacket.FreePacket(pvPacket);

			/*
			// DB에 저장한다.
			if (pThis->m_pagsmServerManager->GetThisServerType() == AGSMSERVER_TYPE_GAME_SERVER)
				pThis->SaveSkillDIRT(pcsSkill);
			*/

			return bSendResult;
		}

		pThis->m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
	}
	
	return FALSE;
}

BOOL AgsmSkill::CBEquipItem(PVOID pData, PVOID pClass, PVOID pCustData)
{
	// 2007.07.08. steeple
	// Passive 에서 아이템에 따라 적용 여부가 결정되는 스킬이 생겼다.
	// 이를 처리해준다.

	if(!pData || !pClass)
		return FALSE;

	AgpdItem* pcsItem = (AgpdItem*)pData;
	AgsmSkill* pThis = (AgsmSkill*)pClass;

	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pcsItem->m_pcsCharacter;
	if(!pcsCharacter)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return TRUE;

	AgsdSkillADBase* pcsADBase = pThis->GetADBase((ApBase*)pcsCharacter);
	if(!pcsADBase)
		return TRUE;

	//// 버프된 스킬이 있는지 검사한다.
	//if(pcsAttachData->m_astBuffSkillList[0].lSkillID == AP_INVALID_SKILLID)
	//	return TRUE;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID)
			break;

		if(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
		{
			for(int j = 0; j < AGPMSKILL_MAX_SKILL_CONDITION; ++j)
			{
				// Item 상태를 검사해서, Buff 된 스킬 중에 Factor 를 올려야 되는 게 있으면 올려준다.
				pThis->ApplyUpdateFactorItem(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill, pcsCharacter, j, FALSE, TRUE);

				// Item 상태를 검사해서, Buff 된 스킬 중에 Factor 를 올려 놓은 게 있으면 빼준다.
				if(pThis->ApplyUpdateFactorItem(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill, pcsCharacter, 0, FALSE, FALSE, FALSE))
				{
					// 2004.12.15. steeple
					// Factor 를 빼주는 Skill 이었고, 성공적으로 빼주었다면 Buff 해제한다.
					pThis->EndBuffedSkill((ApBase*)pcsCharacter, i, TRUE, _T("CBUnEquipItem"));
					i--;	// 이거 2005.08.25. 에 넣었다. 왜 빠져 있는 지 모르겠음..
					continue;
				}
			}
		}
	}

	// 2007.07.08. steeple
	// Passive 도 해준다.
	for(int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if(pcsAttachData->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill* pcsPassiveSkill = pThis->m_pagpmSkill->GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
		if(pcsPassiveSkill)
		{
			pThis->ApplyUpdateFactorItem(pcsPassiveSkill, pcsCharacter, 0, FALSE, TRUE, FALSE);
			pThis->ApplyUpdateFactorItem(pcsPassiveSkill, pcsCharacter, 0, FALSE, FALSE, FALSE);
		}
	}

	return TRUE;
}

BOOL AgsmSkill::CBUnEquipItem(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgpdItem* pcsItem = (AgpdItem*)pData;
	AgsmSkill* pThis = (AgsmSkill*)pClass;

	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pcsItem->m_pcsCharacter;
	if(!pcsCharacter)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return TRUE;

	AgsdSkillADBase* pcsADBase = pThis->GetADBase((ApBase*)pcsCharacter);
	if(!pcsADBase)
		return TRUE;

	//// 버프된 스킬이 있는지 검사한다.
	//if(pcsAttachData->m_astBuffSkillList[0].lSkillID == AP_INVALID_SKILLID)
	//	return TRUE;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID)
			break;

		// 2005.08.25. 약간 수정.
		if(!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			break;

		// Item 상태를 검사해서, Buff 된 스킬 중에 Factor 를 올려 놓은 게 있으면 빼준다.
		if(pThis->ApplyUpdateFactorItem(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill, pcsCharacter, 0, FALSE, FALSE, FALSE))
		{
			// 2004.12.15. steeple
			// Factor 를 빼주는 Skill 이었고, 성공적으로 빼주었다면 Buff 해제한다.
			pThis->EndBuffedSkill((ApBase*)pcsCharacter, i, TRUE, _T("CBUnEquipItem"));
			i--;	// 이거 2005.08.25. 에 넣었다. 왜 빠져 있는 지 모르겠음..
			continue;
		}
	}

	// 2007.07.08. steeple
	// Passive 도 해준다.
	for(int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if(pcsAttachData->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill* pcsPassiveSkill = pThis->m_pagpmSkill->GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
		if(pcsPassiveSkill)
		{
			pThis->ApplyUpdateFactorItem(pcsPassiveSkill, pcsCharacter, 0, FALSE, FALSE, FALSE);
		}
	}

	return TRUE;
}

BOOL AgsmSkill::SendCastSkill(AgpdSkill *pcsSkill, INT16 nActionType, INT32* palTargetCID, INT16 nLenTargetCID, AgpdCharacter **apcsCharacter, INT16 nNumTarget)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate)
		return FALSE;

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_nAttribute & AGPMSKILL_SKILL_ATTR_NOT_SEND_CAST)
		return TRUE;

	AuPOS	*pposBase = m_papmEventManager->GetBasePos(pcsSkill->m_pcsBase, NULL);
	if (!pposBase)
		return FALSE;

	/*
	ApWorldSector *pcsSector = m_papmMap->GetSector(pposBase->x, pposBase->z);
	if (!pcsSector)
		return FALSE;
	*/

	AgsmAOICell	*pcsCell	= m_pagsmAOIFilter->m_csAOIMap.GetCell(pposBase);
	if (!pcsCell)
		return FALSE;

	INT32 lRegionIndex = -1;
	if(pcsSkill->m_pcsBase && pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		lRegionIndex = m_pagpmCharacter->GetRealRegionIndex((AgpdCharacter*)pcsSkill->m_pcsBase);
	}

	//// 2007.02.13. steeple
	//// INT_PTR 8byte >> INT 4byte
	//ApSafeArray<INT32, AGSMSKILL_MAX_TARGET_NUM> alTargetCID;
	//if(palTargetCID )
	//{
	//	for(int i = 0; i < nLenTargetCID / sizeof(INT32); ++i)
	//	{
	//		alTargetCID[i] = (INT32)palTargetCID[i];
	//	}
	//}

	INT16	nPacketLength	= 0;
	INT32	lSkillLevel		= GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	// 2005.12.4. steeple
	UINT32 ulCastDelay = m_pagpmSkill->GetCastDelay((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate, lSkillLevel);
	INT32 ulAdjustCastDelay = (INT32)((FLOAT)(ulCastDelay) * (FLOAT)m_pagpmSkill->GetAdjustCastDelay((AgpdCharacter*)pcsSkill->m_pcsBase) / 100.0f);
	ulCastDelay += ulAdjustCastDelay;

	UINT32 ulRecastDelay = m_pagpmSkill->GetRecastDelay((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate, lSkillLevel);
	INT32 lAdjustDelay = (INT32)((FLOAT)(ulRecastDelay) * (FLOAT)m_pagpmSkill->GetAdjustRecastDelay((AgpdCharacter*)pcsSkill->m_pcsBase) / 100.0f);
	ulRecastDelay += lAdjustDelay;

	PVOID	pvPacket = m_pagpmSkill->MakeCastPacket(nActionType, &pcsSkill->m_lID, &pcsSkill->m_pcsTemplate->m_lID, pcsSkill->m_pcsBase, lSkillLevel,
							(ApBase *) &pcsSkill->m_csTargetBase, &pcsSkill->m_posTarget, pcsSkill->m_bForceAttack,
							ulCastDelay, pcsSkill->m_ulEndTime - pcsSkill->m_ulStartTime,
							ulRecastDelay, (CHAR*)palTargetCID, nLenTargetCID, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL	bRetval = TRUE;

	if (palTargetCID && nLenTargetCID > 0)
	{
		UINT32	ulExceptNIDs[5];
		INT32	lNumNID		= 0;

		ZeroMemory(ulExceptNIDs, sizeof(UINT32) * 5);

		if (apcsCharacter && nNumTarget > 0)
		{
			for (int i = 0; i < nNumTarget; ++i)
			{
				if (apcsCharacter[i] &&
					apcsCharacter[i]->m_eType == APBASE_TYPE_CHARACTER)
				{
					ulExceptNIDs[lNumNID]	= m_pagsmCharacter->GetCharDPNID(apcsCharacter[i]);
					if (ulExceptNIDs[lNumNID] != 0)
					{
						m_pagpmSkill->m_csPacket.SetCID(pvPacket, nPacketLength, apcsCharacter[i]->m_lID);
						SendPacket(pvPacket, nPacketLength, ulExceptNIDs[lNumNID], PACKET_PRIORITY_5);

						++lNumNID;

						if (lNumNID >= 5)
							break;
					}
				}
			}
		}

		if (!m_pagsmAOIFilter->SendPacketNearExceptNIDs(pvPacket, nPacketLength, pcsCell, lRegionIndex, ulExceptNIDs, lNumNID, PACKET_PRIORITY_5))
		{
			m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
			bRetval = FALSE;
		}
	}
	else
	{
		m_pagpmSkill->m_csPacket.SetCID(pvPacket, nPacketLength, pcsSkill->m_pcsBase->m_lID);

		if (!m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, pcsCell, lRegionIndex, PACKET_PRIORITY_5))
		{
			m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
			bRetval = FALSE;
		}
	}

	m_pagpmSkill->m_csPacket.FreePacket(pvPacket);

	return bRetval;
}

BOOL AgsmSkill::SendMissCastSkill(AgpdSkill *pcsSkill, ApBase *pcsTargetBase, UINT32 ulAdditionalEffect)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || !pcsSkill->m_pcsTemplate)
		return FALSE;

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_nAttribute & AGPMSKILL_SKILL_ATTR_NOT_SEND_CAST)
		return TRUE;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= m_pagpmSkill->MakeCastResultPacket(AGPMSKILL_ACTION_MISS_CAST_SKILL,
																 &pcsSkill->m_lID,
																 &pcsSkill->m_pcsTemplate->m_lID,
																 pcsSkill->m_pcsBase,
																 GetModifiedSkillLevel(pcsSkill),
																 pcsTargetBase,
																 NULL,
																 &nPacketLength,
																 0,
																 ulAdditionalEffect);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
		SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID((AgpdCharacter *) pcsSkill->m_pcsBase));

	if (pcsTargetBase && pcsTargetBase->m_eType == APBASE_TYPE_CHARACTER)
		SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID((AgpdCharacter *) pcsTargetBase));

	/*
	if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
		m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, ((AgpdCharacter *) pcsSkill->m_pcsBase)->m_stPos);
	*/

	m_pagpmSkill->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

//	스킬 처리 함수들.. 여기서 각종 계산을 다 처리한다.
///////////////////////////////////////////////////////////////////////////////

BOOL AgsmSkill::IsValidSkillCast(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate)
		return FALSE;

	AgpdFactorDIRT			*pcsFactorDIRT = (AgpdFactorDIRT *) m_pagpmFactors->GetFactor(&pcsSkill->m_csFactor, AGPD_FACTORS_TYPE_DIRT);
	if (!pcsFactorDIRT)
		return FALSE;

	// 스킬 레벨이 1 이상인지 검사한다. (0인넘은 당연히 사용할 수 없다)
	if (pcsFactorDIRT->lValue[AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL] < 1)
		return FALSE;

	// Force Attack이 세팅된경우 같은 유니온 멤버인지.. PK가 허용되는 지역인지 체크한다.
	//
	//

	return TRUE;
}

AgpdFactor* AgsmSkill::GetResultFactor(ApBase *pcsBase)
{
	if (!pcsBase)
		return NULL;

	switch (pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			return (AgpdFactor *) m_pagpmFactors->GetFactor(&((AgpdCharacter *) pcsBase)->m_csFactor, AGPD_FACTORS_TYPE_RESULT);
		}
		break;

	case APBASE_TYPE_ITEM:
		{
			if (!((AgpdItem *) pcsBase)->m_pcsCharacter)
				return NULL;

			return (AgpdFactor *) m_pagpmFactors->GetFactor(&((AgpdItem *) pcsBase)->m_pcsCharacter->m_csFactor, AGPD_FACTORS_TYPE_RESULT);
		}
		break;
	}

	return NULL;
}

AgpdFactor* AgsmSkill::GetOwnerFactorResult(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase)
		return NULL;

	return GetResultFactor(pcsSkill->m_pcsBase);
}

/*
AgpdFactor*	AgsmSkill::GetTargetFactorResult(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTargetBase)
		return NULL;

	return GetResultFactor(pcsSkill->m_pcsTargetBase);
}
*/

//		UpdateSkillFactor
//	Functions
//		- skill 에서 계산되어 나온 factor (point, percent, result factor)를 pcsTarget의 factor로 업데이트 시킨다.
//			1. point, percent 는 point, percent factor에 업데이트 시키고 CalcResultFactor()를 호출한다.
//			2. result 는 result factor에 업데이트 시키고 넘어온 패킷을 pcsTarget으로 보낸다.
//			3. hit history 에 저장한다.
//	Arguments
//		- pcsSkill	: skill data pointer
//		- pcsTarget	: target base pointer
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::UpdateSkillFactor(AgpdSkill *pcsSkill, ApBase *pcsTarget, BOOL bIsFactorNotQueueing, BOOL bProcessInterval)
{
	PROFILE("AgsmSkill::UpdateSkillFactor");

	if (!pcsSkill || !pcsTarget)
		return FALSE;

	AgsdSkill	*pcsAgsdSkill = GetADSkill(pcsSkill);
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsTarget->m_eType == APBASE_TYPE_CHARACTER &&
		// 2006.11.28. steeple
		// 한번만 팩터 적용되는 스킬인지 체크한다.
		!(bProcessInterval && CheckOnceAffected((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
		)
	{

		// hp, mp, sp를 올려주는넘들이면 걍 올려주고 끝나는거니까 point나 percent에 있는것들을 계산해서 result로 넘긴다.
		//////////////////////////////////////////////////////////////////////////
		AgpdFactorCharPoint	*pcsPointCharPoint = (AgpdFactorCharPoint *) m_pagpmFactors->GetFactor(pcsAgsdSkill->m_pcsUpdateFactorPoint, AGPD_FACTORS_TYPE_CHAR_POINT);
		if (pcsPointCharPoint)
		{
			if (pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] != 0 ||
				pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] != 0 ||
				pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] != 0)
			{
				if (SetUpdateFactorResult(pcsAgsdSkill))
				{
					// 2005.06.02. steeple. Result 팩터 를 안구하고 그냥 팩터에 넣는 닥질을 해결.
					AgpdFactor* pcsFactorResult	= (AgpdFactor*)m_pagpmFactors->SetFactor(pcsAgsdSkill->m_pcsUpdateFactorResult, NULL, AGPD_FACTORS_TYPE_RESULT);
					if(pcsFactorResult)
					{
						AgpdFactorCharPoint	*pcsResultCharPoint = (AgpdFactorCharPoint *) m_pagpmFactors->SetFactor(pcsFactorResult, NULL, AGPD_FACTORS_TYPE_CHAR_POINT);
						if (pcsResultCharPoint)
						{
							pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] += pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP];
							pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] += pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP];
							pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] += pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP];

							// DMG Normal 에다가도 더해준다. 2005.06.02. steeple
							pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_DMG_NORMAL] += pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP];
						}
					}
				}

				pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] = 0;
				pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] = 0;
				pcsPointCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] = 0;
			}
		}

		AgpdFactorCharPoint	*pcsPercentCharPoint = (AgpdFactorCharPoint *) m_pagpmFactors->GetFactor(pcsAgsdSkill->m_pcsUpdateFactorPercent, AGPD_FACTORS_TYPE_CHAR_POINT);
		if (pcsPercentCharPoint)
		{
			if (pcsPercentCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] != 0 ||
				pcsPercentCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] != 0 ||
				pcsPercentCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] != 0)
			{
				if (SetUpdateFactorResult(pcsAgsdSkill))
				{
					// 2005.06.02. steeple. Result 팩터 를 안구하고 그냥 팩터에 넣는 닥질을 해결.
					AgpdFactor* pcsFactorResult	= (AgpdFactor*)m_pagpmFactors->SetFactor(pcsAgsdSkill->m_pcsUpdateFactorResult, NULL, AGPD_FACTORS_TYPE_RESULT);
					if(pcsFactorResult)
					{
						AgpdFactorCharPoint	*pcsResultCharPoint = (AgpdFactorCharPoint *) m_pagpmFactors->SetFactor(pcsFactorResult, NULL, AGPD_FACTORS_TYPE_CHAR_POINT);
						if (pcsResultCharPoint)
						{
							// 2007.07.30. steeple
							// Target 의 기본값으로 계산하지 않고, Result 값으로 계산하여야 한다.
							INT32 lMaxHP, lMaxMP, lMaxSP;
							lMaxHP = lMaxMP = lMaxSP = 0;

							m_pagpmFactors->GetValue(&((AgpdCharacter*)pcsTarget)->m_csFactor, &lMaxHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_HP);
							m_pagpmFactors->GetValue(&((AgpdCharacter*)pcsTarget)->m_csFactor, &lMaxMP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_MP);
							m_pagpmFactors->GetValue(&((AgpdCharacter*)pcsTarget)->m_csFactor, &lMaxSP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_SP);

							INT32 lDamage = (INT32)((FLOAT)(lMaxHP * (FLOAT)pcsPercentCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP]) / 100.0f);

							// 초특급 임시대박 꽁수 코드. 2005.06.09. steeple
							if(lDamage < -500)
							{
								INT32 lRandDamage = m_csRandom.randInt(100);
								lDamage = -400 + (-lRandDamage);
							}

							pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] += lDamage;
							pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] += (lMaxMP * pcsPercentCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP]) / 100;
							pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] += (lMaxSP * pcsPercentCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP]) / 100;

							if(pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] > lMaxHP)
								pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] = lMaxHP;
							if(pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] > lMaxMP)
								pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] = lMaxMP;
							if(pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] > lMaxSP)
								pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] = lMaxSP;

							// DMG Normal 에다가도 더해준다. 2005.06.02. steeple
							pcsResultCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_DMG_NORMAL] += lDamage;
						}
					}
				}

				pcsPercentCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] = 0;
				pcsPercentCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] = 0;
				pcsPercentCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] = 0;
			}
		}

		// point, percent를 업데이트 하고 CalcResultFactor()를 호출해준다.
		//////////////////////////////////////////////////////////////////////////

		// 2005.02.04. steeple
		// ProcessInterval 이 FALSE 일 때만 한다.
		if(!bProcessInterval)
		{
			m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorPoint, FALSE, FALSE, TRUE, FALSE);
			m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorPercent, FALSE, FALSE, TRUE, FALSE);

			m_pagsmCharacter->ReCalcCharacterResultFactors((AgpdCharacter *) pcsTarget, TRUE);
		}

		// result factor를 업데이트 하고 넘어온 factor packet을 보낸다.
		//////////////////////////////////////////////////////////////////////////
		/*
		AgpdFactor	*pcsFactorUpdateResult = (AgpdFactor *) m_pagpmFactors->GetFactor(&pcsAgsdSkill->m_csUpdateFactorResult, AGPD_FACTORS_TYPE_RESULT);
		if (pcsFactorUpdateResult)
		{
			INT32	lTotalDamage	= 0;
			m_pagpmFactors->GetValue(pcsFactorUpdateResult, &lTotalDamage, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);
			m_pagpmFactors->SetValue(pcsFactorUpdateResult, lTotalDamage, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_DMG_NORMAL);
		}
		*/

		ApBase	*pcsOwnerBase	= NULL;
		if (pcsSkill->m_bCloneObject &&
			pcsSkill->m_csBase.m_eType == APBASE_TYPE_CHARACTER)
			pcsOwnerBase	= (ApBase *) m_pagpmCharacter->GetCharacterLock(pcsSkill->m_csBase.m_lID);
		else
			pcsOwnerBase	= pcsSkill->m_pcsBase;

		INT32	lHP			= 0;
		PVOID	pvPacketFactor = m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsTarget)->m_csFactor, pcsAgsdSkill->m_pcsUpdateFactorResult, FALSE, TRUE, TRUE, FALSE);
		if (pvPacketFactor)
		{
			BOOL	bIsSyncHP	= FALSE;

			m_pagpmFactors->GetValue(pcsAgsdSkill->m_pcsUpdateFactorResult, &lHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);

			if (lHP != 0)
				bIsSyncHP	= TRUE;

			// 2005.10.06. steeple
			if (lHP > 0)
			{
				// 이러면 데미지가 들어간 게 아니라 Heal 을 해준 것이다. 이럴 땐 따로 보내자 -0-;
				INT32 lCurrentHP = 0;
				m_pagpmFactors->GetValue(&((AgpdCharacter*)pcsTarget)->m_csFactor, &lCurrentHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);

				// 다시 세팅
				PVOID pvPacket = NULL;
				m_pagsmFactors->UpdateCharPoint(&((AgpdCharacter*)pcsTarget)->m_csFactor, &pvPacket, lCurrentHP, 0, 0);

				// 변경 했다면 패킷 보내버린다.
				if(pvPacket)
				{
					m_pagsmCharacter->SendPacketFactor(pvPacket, (AgpdCharacter*)pcsTarget, PACKET_PRIORITY_4);
					m_pagpmCharacter->m_csPacket.FreePacket(pvPacket);
				}
			}

			// 2005.04.01. steeple
			// 오직 DOT Skill 이면서 첫타일때는 싱크 시키지 않는다.
			if(!bProcessInterval && ((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lEffectType[0] == AGPMSKILL_EFFECT_DOT_DAMAGE)
			{
				bIsFactorNotQueueing = TRUE;
				bIsSyncHP = FALSE;
			}

			// 2007.07.04. steeple
			// 데미지를 주면서 동시에 자신에게 HP 가 차는 스킬 처리한다.
			ProcessDamageToHPInstant(pcsSkill, pcsTarget, -lHP);

			UINT32 ulAdditionalEffect = 0;
			if(m_pagpmSkill->IsStatusActionOnActionType4(pcsSkill->m_pcsBase) &&
				m_pagpmSkill->IsSubCashItemStackCountOnSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
				ulAdditionalEffect |= AGPDCHAR_ADDITIONAL_EFFECT_LENS_STONE;
			
			SendCastResult(pcsSkill, pcsOwnerBase, pcsTarget, pvPacketFactor, AGPMSKILL_ACTION_CAST_SKILL_RESULT, bIsFactorNotQueueing, bIsSyncHP, ulAdditionalEffect);
			//m_pagsmCombat->SendAttackResult((AgpdCharacter *) pcsSkill->m_pcsBase, (AgpdCharacter *) pcsTarget, pvPacketFactor, AGPDCHAR_ACTION_RESULT_TYPE_CAST_SKILL);
			//m_pagsmCharacter->SendPacketFactor(pvPacketFactor, (AgpdCharacter *) pcsTarget);

			m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);
		}
		else
		{
			// 2005.12.15. steeple
			if(m_pagpmSkill->IsStatusActionOnActionType4(pcsSkill->m_pcsBase) &&
				m_pagpmSkill->IsSubCashItemStackCountOnSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
				SendAdditionalEffect(pcsTarget, AGPMSKILL_ADDITIONAL_EFFECT_LENS_STONE, 0, m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase));
		}

		m_pagsmFactors->ResetResultFactorDamageInfo(pcsAgsdSkill->m_pcsUpdateFactorPoint);
		m_pagsmFactors->ResetResultFactorDamageInfo(pcsAgsdSkill->m_pcsUpdateFactorPercent);
		m_pagsmFactors->ResetResultFactorDamageInfo(pcsAgsdSkill->m_pcsUpdateFactorResult);
		m_pagsmFactors->ResetResultFactorDamageInfo(&((AgpdCharacter *) pcsTarget)->m_csFactor);


		// 2007.07.10. steeple
		// 혹시라도 Max값을 초과했다면 재조정해준다.
		m_pagsmCharacter->AdjustExceedPoint((AgpdCharacter*)pcsTarget);

		// hit history에 저장한다.
		//////////////////////////////////////////////////////////////////////////
		/*
		m_pagsmCharacter->AddHistory((AgpdCharacter *) pcsTarget, pcsSkill->m_pcsBase, &pcsAgsdSkill->m_csUpdateFactorPoint, GetClockCount());

		for (int i = 0; i < AGPD_FACTORS_MAX_TYPE; ++i)
		{
			if (pcsAgsdSkill->m_csUpdateFactorPercent.m_pvFactor[i])
				break;
		}

		if (i != AGPD_FACTORS_MAX_TYPE)
		{
			AgpdFactor	csCalcFactor;

			if (m_pagpmFactors->CalcResultFactor(&csCalcFactor, &((AgpdCharacter *) pcsTarget)->m_csFactor, &((AgpdCharacter *) pcsTarget)->m_csFactorPoint, &pcsAgsdSkill->m_csUpdateFactorPercent))
			{
				m_pagsmCharacter->AddHistory((AgpdCharacter *) pcsTarget, pcsSkill->m_pcsBase, &csCalcFactor, GetClockCount());
			}

			m_pagpmFactors->DestroyFactor(&csCalcFactor);
		}
		*/

		AgpdFactor	*pcsFactorUpdateResult = (AgpdFactor *) m_pagpmFactors->GetFactor(pcsAgsdSkill->m_pcsUpdateFactorResult, AGPD_FACTORS_TYPE_RESULT);
		if (pcsFactorUpdateResult)
		{
			if(m_pagpmCharacter && pcsTarget && pcsOwnerBase && m_pagpmCharacter->IsMonster((AgpdCharacter *)pcsTarget) && 
				(m_pagpmCharacter->IsStatusTame((AgpdCharacter *)pcsOwnerBase) || m_pagpmCharacter->IsStatusSummoner((AgpdCharacter *)pcsOwnerBase)))
			{
				INT32 lOwnerCID = m_pagpmSummons->GetOwnerCID((AgpdCharacter *)pcsOwnerBase);
				if(0 != lOwnerCID)
				{
					AgpdCharacter* pcsCharacter = m_pagpmCharacter->GetCharacterLock(lOwnerCID);
					if(NULL != pcsCharacter)
					{
						m_pagsmCharacter->AddHistory((AgpdCharacter *) pcsTarget, pcsCharacter, pcsFactorUpdateResult, GetClockCount());

						pcsCharacter->m_Mutex.Release();
					}
				}
			}

			if(m_pagpmSkill->IsNotAddAgroAtCast((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) == FALSE || lHP < 0)
				m_pagsmCharacter->AddAgroPoint((AgpdCharacter *) pcsTarget, pcsOwnerBase, pcsFactorUpdateResult );
		}

		//if (!m_pagpmSkill->IsPassiveSkill(pcsSkill))
		//{
		//	// callback function (AGSMSKILL_CB_ATTACK) 을 호출해준다.
		//	//////////////////////////////////////////////////////////////////////////
		//	stAgsmCombatAttackResult	stAttackResult;

		//	if (pcsSkill->m_bCloneObject)
		//		stAttackResult.pAttackChar	= (AgpdCharacter *) pcsOwnerBase;
		//	else
		//		stAttackResult.pAttackChar	= (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER) ? (AgpdCharacter *) pcsSkill->m_pcsBase : NULL;

		//	stAttackResult.pTargetChar	= (AgpdCharacter *) pcsTarget;
		//	stAttackResult.nDamage		= 0;

		//	EnumCallback(AGSMSKILL_CB_ATTACK, &stAttackResult, pcsSkill);
		//}

		if (pcsOwnerBase && pcsSkill->m_bCloneObject)
			pcsOwnerBase->m_Mutex.Release();
	}

	return TRUE;
}

//		RestoreSkillFactor
//	Functions
//		- skill 에서 업데이트 했던 point, percent 의 값들을 이전 값들로 되돌린다. (스킬이 끝났다)
//			1. point, percent 는 되돌리고 ReCalcResultFactor()를 호출한다.
//	Arguments
//		- pcsSkill		: skill data pointer
//		- csTarget		: target base pointer
//		- nBuffedIndex	: buffed list에 있는 index
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::RestoreSkillFactor(AgpdSkill *pcsSkill, ApBase *pcsTarget, INT16 nBuffedIndex)
{
	PROFILE("AgsmSkill::RestoreSkillFactor");

	if (!pcsSkill || !pcsTarget || nBuffedIndex >= AGPMSKILL_MAX_AFFECTED_BASE)
		return FALSE;

	AgsdSkill	*pcsAgsdSkill = GetADSkill(pcsSkill);
	if (!pcsAgsdSkill)
		return FALSE;

	switch (pcsTarget->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			// point, percent를 되돌리고 CalcResultFactor()를 호출해준다.
			//////////////////////////////////////////////////////////////////////////
			if (nBuffedIndex < 0)
			{
				m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorPoint, FALSE, FALSE, FALSE, FALSE);
				m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorPercent, FALSE, FALSE, FALSE, FALSE);
			}
			else
			{
				/*
				m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsTarget)->m_csFactorPoint, pcsAgsdSkill->m_csModifyFactorPoint + nBuffedIndex, FALSE, FALSE, FALSE);
				m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsTarget)->m_csFactorPercent, pcsAgsdSkill->m_csModifyFactorPercent + nBuffedIndex, FALSE, FALSE, FALSE);
				*/
			}

			m_pagsmCharacter->ReCalcCharacterResultFactors(((AgpdCharacter *) pcsTarget), TRUE);


			// 착용하고 있던 아이템에 따라 팩터 적용 여부가 결정되는 스킬도 Restore 한다.
			for(int i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
			{
				ApplyUpdateFactorItem(pcsSkill, pcsTarget, i, FALSE, FALSE);
			}
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

/*
BOOL AgsmSkill::RestoreSkillFactor(ApBase *pcsBase, INT16 nBuffedIndex)
{
	if (!pcsBase || nBuffedIndex < 0 || nBuffedIndex >= AGPMSKILL_MAX_SKILL_BUFF)
		return FALSE;

	// point, percent를 되돌리고 CalcResultFactor()를 호출해준다.
	//////////////////////////////////////////////////////////////////////////

	switch (pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			AgsdSkillADBase	*pcsADBase	= GetADBase(pcsBase);
			if (!pcsADBase)
				return FALSE;

			m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsBase)->m_csFactorPoint, &pcsADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_csModifyFactorPoint, FALSE, FALSE, FALSE);
			m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsBase)->m_csFactorPercent, &pcsADBase->m_csBuffedSkillProcessInfo[nBuffedIndex].m_csModifyFactorPercent, FALSE, FALSE, FALSE);

			return m_pagsmCharacter->ReCalcCharacterResultFactors(((AgpdCharacter *) pcsBase), TRUE);
		}
		break;

	default:
		return FALSE;
		break;
	}

	return FALSE;
}
*/

//		ProcessReflectDamage
//	Functions
//		- 반격 데미지를 처리한다.
//	Arguments
//		- pcsAttackBase	: 반격자 베이쑤
//		- pcsTargetBase	: 대상 베이쑤
//		- lDamage		: 반격 데미지
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::ProcessReflectDamage(ApBase *pcsAttackBase, ApBase *pcsTargetBase, INT32 lDamage)
{
	if (!pcsAttackBase || !pcsTargetBase)
		return FALSE;

	if (lDamage < 1)
		return TRUE;

	AgpdFactor	csFactor;

	m_pagpmFactors->InitFactor(&csFactor);

	AgpdFactor	*pcsFactorResult = (AgpdFactor *) m_pagpmFactors->SetFactor(&csFactor, NULL, AGPD_FACTORS_TYPE_RESULT);
	if (!pcsFactorResult)
	{
		m_pagpmFactors->DestroyFactor(&csFactor);
		return FALSE;
	}

	AgpdFactorCharPoint *pcsFactorCharPoint = (AgpdFactorCharPoint *) m_pagpmFactors->InitCalcFactor(pcsFactorResult, AGPD_FACTORS_TYPE_CHAR_POINT);
	if (!pcsFactorCharPoint)
	{
		m_pagpmFactors->DestroyFactor(&csFactor);

		return FALSE;
	}

	pcsFactorCharPoint->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] -= lDamage;

	if (pcsTargetBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		// result factor를 업데이트 하고 넘어온 factor packet을 보낸다.
		//////////////////////////////////////////////////////////////////////////
		PVOID	pvPacketFactor = m_pagpmFactors->CalcFactor(&((AgpdCharacter *) pcsTargetBase)->m_csFactor, &csFactor, TRUE, TRUE, TRUE, FALSE);
		if (pvPacketFactor)
		{
			m_pagsmCharacter->SendPacketFactor(pvPacketFactor, (AgpdCharacter *) pcsTargetBase, PACKET_PRIORITY_4);

			m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);
		}

		// hit history에 저장한다.
		//////////////////////////////////////////////////////////////////////////
		m_pagsmCharacter->AddAgroPoint((AgpdCharacter *) pcsTargetBase, pcsAttackBase, pcsFactorResult );

		/*
		// callback function (AGSMSKILL_CB_ATTACK) 을 호출해준다.
		//////////////////////////////////////////////////////////////////////////
		stAgsmCombatAttackResult	stAttackResult;

		stAttackResult.pAttackChar	= (pcsAttackBase->m_eType == APBASE_TYPE_CHARACTER) ? (AgpdCharacter *) pcsAttackBase : NULL;
		stAttackResult.pTargetChar	= (AgpdCharacter *) pcsTargetBase;
		stAttackResult.nDamage		= 0;

		EnumCallback(AGSMSKILL_CB_ATTACK, &stAttackResult, NULL);
		*/
	}

	m_pagpmFactors->DestroyFactor(&csFactor);

	return TRUE;
}

BOOL AgsmSkill::SendUpdateBuffedList(ApBase *pcsBase, INT32 lTID, INT32 lCasterTID, BOOL bAdd, UINT32 ulEndTimeMSec, UINT8 ucChargeLevel, UINT32 ulDPNID, UINT32 ulExpiredTime)
{
	if (!pcsBase || lTID == AP_INVALID_SKILLID)
		return FALSE;

	INT16	nPacketLength = 0;
	PVOID	pvPacket = m_pagpmSkill->MakePacketBuffedList(pcsBase, lTID, lCasterTID, bAdd, ulEndTimeMSec, ucChargeLevel, &nPacketLength, ulExpiredTime);

	if (pvPacket && nPacketLength > 0)
	{
		BOOL	bRetval	= FALSE;

		if (pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		{
			if (ulDPNID != 0)
				bRetval = SendPacket(pvPacket, nPacketLength, ulDPNID);
		}
		else
		{
			m_pagpmSkill->m_csPacket.SetCID(pvPacket, nPacketLength, pcsBase->m_lID);

//			bRetval = m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, ((AgpdCharacter *) pcsBase)->m_stPos, PACKET_PRIORITY_5);
			bRetval &= m_pagsmAOIFilter->SendPacketNearExceptSelf(pvPacket, nPacketLength, ((AgpdCharacter *) pcsBase)->m_stPos,
																	m_pagpmCharacter->GetRealRegionIndex((AgpdCharacter*)pcsBase),
																	m_pagsmCharacter->GetCharDPNID((AgpdCharacter *) pcsBase),
																	PACKET_PRIORITY_5);
			bRetval &= SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID((AgpdCharacter *) pcsBase));

			bRetval &= m_pagsmParty->SendPacketToFarPartyMember((AgpdCharacter *) pcsBase, pvPacket, nPacketLength, PACKET_PRIORITY_5);
		}

		m_pagpmSkill->m_csPacket.FreePacket(pvPacket);

		return bRetval;
	}

	return FALSE;
}

BOOL AgsmSkill::SendAllBuffedSkill(ApBase *pcsBase, UINT32 ulNID, BOOL bGroupNID, UINT32 ulSelfNID)
{
	if (!pcsBase || ulNID == 0)
		return FALSE;
	
	PROFILE("AgsmSkill:SendAllBuffedSkill");

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= NULL;
	
	{
		PROFILE("AgsmSkill::SendAllBuffedSkill - MakePacket");
		pvPacket	= m_pagpmSkill->MakePacketAllBuffedList(pcsBase, &nPacketLength);
	}

	if (!pvPacket || nPacketLength < 0)
		return TRUE;

	BOOL	bSendResult	= FALSE;

	m_pagpmSkill->m_csPacket.SetCID(pvPacket, nPacketLength, pcsBase->m_lID);

	{
		PROFILE("AgsmSkill::SendAllBuffedSkill - SendPacket");
		if (bGroupNID)
		{
			if (ulSelfNID != 0)
				bSendResult	= m_pagsmAOIFilter->SendPacketGroupExceptSelf(pvPacket, nPacketLength, ulNID, ulSelfNID, PACKET_PRIORITY_5);
			else
				bSendResult	= m_pagsmAOIFilter->SendPacketGroup(pvPacket, nPacketLength, ulNID, PACKET_PRIORITY_5);
		}
		else
			bSendResult = SendPacket(pvPacket, nPacketLength, ulNID, PACKET_PRIORITY_5);
	}

	m_pagpmSkill->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

UINT32 AgsmSkill::GetBaseDPNID(ApBase *pcsBase)
{
	if (!pcsBase)
		return 0;

	switch (pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			AgsdCharacter *pcsAgsdCharacter = m_pagsmCharacter->GetADCharacter((AgpdCharacter *) pcsBase);
			if (pcsAgsdCharacter)
				return pcsAgsdCharacter->m_dpnidCharacter;
		}
		break;
	}

	return 0;
}

ApBase* AgsmSkill::GetBaseLock(ApBase *pcsBase)
{
	if (!pcsBase)
		return NULL;

	switch (pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			return (ApBase *) m_pagpmCharacter->GetCharacterLock(pcsBase->m_lID);
		}
		break;
	}

	return NULL;
}

ApBase* AgsmSkill::GetBase(ApBase *pcsBase)
{
	if (!pcsBase)
		return NULL;

	switch (pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			return (ApBase *) m_pagpmCharacter->GetCharacter(pcsBase->m_lID);
		}
		break;
	}

	return NULL;
}

BOOL AgsmSkill::SetCallbackMeleeAttackCheck(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_MELEE_ATTACK_CHECK, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackMagicAttackCheck(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_MAGIC_ATTACK_CHECK, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackPreCheckCast(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_PRE_CHECK_CAST, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackPostCheckCast(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_POST_CHECK_CAST, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackAttack(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_ATTACK, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackInsertSkillMasteryToDB(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_INSERT_SKILL_MASTERY_TO_DB, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackUpdateSkillMasteryToDB(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_UPDATE_SKILL_MASTERY_TO_DB, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackDeleteSkillMasteryToDB(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_DELETE_SKILL_MASTERY_TO_DB, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackParseSkillTreeString(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_DB_PARSE_SKILL_TREE_STIRNG, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackParseSkillComposeString(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_DB_PARSE_SKILL_COMPOSE_STIRNG, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackCheckSpecialStatusFactor(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_CHECK_SPECIAL_STATUS_FACTOR, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackCheckPoisonStatusFactor(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_CHECK_POISON_STATUS_FACTOR, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackDamageAdjustSiegeWar(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_DAMAGE_ADJUST_SIEGEWAR, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackCastSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_CAST_SKILL, pfCallback, pClass);
}

/*
BOOL AgsmSkill::SetCallbackDBStreamInsert(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_DB_STREAM_INSERT, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackDBStreamDelete(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_DB_STREAM_DELETE, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackDBStreamSelect(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_DB_STREAM_SELECT, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackDBStreamUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_DB_STREAM_UPDATE, pfCallback, pClass);
}
*/

BOOL AgsmSkill::SetCallbackGetNewSkillID(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_GET_NEW_SKILLID, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackProductSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_PRODUCT_SKILL, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackAffectedSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_AFFECTED_SKILL, pfCallback, pClass);
}

BOOL AgsmSkill::SetCallbackRide(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMSKILL_CB_RIDE, pfCallback, pClass);
}

/*
// get DB Query String.
BOOL AgsmSkill::GetInsertQuery( AgpdSkill *pcsSkill, char *pstrQuery )
{
	if (!pcsSkill || !pstrQuery)
		return FALSE;

	AgsdSkill		*pcsAgsdSkill = GetADSkill(pcsSkill);

	sprintf(pstrQuery, "INSERT INTO SKILL (ID,TNAME,CNAME) VALUES( %I64d,'%s','%s')",
									pcsAgsdSkill->m_ullDBSkillID,
									((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szName,
									((AgpdCharacter *) pcsSkill->m_pcsBase)->m_szID);

	return TRUE;
}

BOOL AgsmSkill::GetSelectQuery( char *szCharName, char *pstrQuery )
{
	if (!szCharName || !pstrQuery)
		return FALSE;

	sprintf(pstrQuery, "SELECT ");
	sprintf(pstrQuery + strlen(pstrQuery), "ID,TNAME ");
	sprintf(pstrQuery + strlen(pstrQuery), "FROM SKILL WHERE CNAME='%s'", szCharName);

	return TRUE;
}

BOOL AgsmSkill::GetUpdateQuery( AgpdSkill *pcsSkill, char *szQuery )
{
	if (!pcsSkill || !szQuery)
		return FALSE;

	AgsdSkill		*pcsAgsdSkill	= GetADSkill(pcsSkill);

	sprintf(szQuery, "UPDATE SKILL SET TNAME='%s',CNAME='%s' WHERE ID=%8d",
									((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szName,
									((AgpdCharacter *) pcsSkill->m_pcsBase)->m_szID,
									pcsAgsdSkill->m_ullDBSkillID);

	return TRUE;
}

BOOL AgsmSkill::GetDeleteQueryByOwner( char *szCharName, char *pstrQuery )
{
	if (!szCharName || !pstrQuery)
		return FALSE;

	sprintf(pstrQuery, "DELETE FROM SKILL WHERE CNAME='%s'", szCharName );

	return TRUE;
}

BOOL AgsmSkill::GetDeleteQueryByDBID( AgpdSkill *pcsSkill, char *pstrQuery )
{
	if (!pcsSkill || !pstrQuery)
		return FALSE;

	sprintf(pstrQuery, "DELETE FROM SKILL WHERE CNAME='%s'", ((AgpdCharacter *) pcsSkill->m_pcsBase)->m_szID);

	return TRUE;
}

AgpdSkill *AgsmSkill::GetSelectResult( COLEDB	*pcOLEDB )
{
	if (!pcOLEDB)
		return NULL;

	AgpdSkill	*pcsSkill	= NULL;

	INT16	nIndex = 0;

	CHAR	*szBuffer = NULL;

	UINT64	ulDBID	= 0;
	if ((szBuffer = pcOLEDB->GetQueryResult(nIndex++)) != NULL)
		ulDBID = (UINT64) _atoi64(szBuffer);

	CHAR	szTName[AGSMDB_MAX_NAME];
	ZeroMemory(szTName, sizeof(CHAR) * AGSMDB_MAX_NAME);
	if ((szBuffer = pcOLEDB->GetQueryResult(nIndex++)) != NULL)
		strcpy(szTName, szBuffer);

	if (ulDBID && szTName[0])
	{
		AgpdSkillTemplate	*pcsTemplate = m_pagpmSkill->GetSkillTemplate(szTName);
		if (pcsTemplate)
		{
			INT32	lSkillID = 0;
			EnumCallback(AGSMSKILL_CB_GET_NEW_SKILLID, &lSkillID, NULL);

			pcsSkill = m_pagpmSkill->AddSkill(lSkillID, pcsTemplate->m_lID);
			if (pcsSkill)
			{
				AgsdSkill	*pcsAgsdSkill	= GetADSkill(pcsSkill);
				if (pcsAgsdSkill)
					pcsAgsdSkill->m_ullDBSkillID = ulDBID;
			}
		}
		else
			return NULL;
	}
	else
		return NULL;

	return pcsSkill;
}

BOOL AgsmSkill::StreamInsertDB(AgpdSkill *pcsSkill, ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_pcsBase || pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return FALSE;

	pstAgsmDBOperationArg	pstArg = (pstAgsmDBOperationArg) GlobalAlloc(GMEM_FIXED, sizeof(stAgsmDBOperationArg));
	if (!pstArg)	return FALSE;

	ZeroMemory(pstArg, sizeof(stAgsmDBOperationArg));

	pstArg->nDataType	= AGSMDB_DATATYPE_SKILL;
	pstArg->nOperation	= AGSMDB_OPERATION_INSERT;
	pstArg->lID			= pcsSkill->m_lID;
	pstArg->pfCallback	= pfCallback;
	pstArg->pClass		= pClass;

	if (!GetInsertQuery(pcsSkill, pstArg->szQuery))
	{
		GlobalFree(pstArg);
		return FALSE;
	}

	BOOL	bResult	= FALSE;

	bResult = m_pagsmDBStream->ExecuteQuery(pstArg, __FILE__, __LINE__ );

	if (!bResult)
		GlobalFree(pstArg);

	return bResult;
}

BOOL AgsmSkill::StreamSelectDB(CHAR *szCharName, ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	if (!szCharName || !szCharName[0])
		return FALSE;

	pstAgsmDBOperationArg	pstArg = (pstAgsmDBOperationArg) GlobalAlloc(GMEM_FIXED, sizeof(stAgsmDBOperationArg));
	if (!pstArg)	return FALSE;

	ZeroMemory(pstArg, sizeof(stAgsmDBOperationArg));

	pstArg->nDataType	= AGSMDB_DATATYPE_SKILL;
	pstArg->nOperation	= AGSMDB_OPERATION_SELECT;
	pstArg->lID			= AP_INVALID_SKILLID;
	pstArg->pfCallback	= pfCallback;
	pstArg->pClass		= pClass;

	if (!GetSelectQuery(szCharName, pstArg->szQuery))
	{
		GlobalFree(pstArg);
		return FALSE;
	}

	BOOL	bResult	= FALSE;

	bResult = m_pagsmDBStream->ExecuteQuery(pstArg, __FILE__, __LINE__ );

	if (!bResult)
		GlobalFree(pstArg);

	return bResult;
}

BOOL AgsmSkill::StreamUpdateDB(AgpdSkill *pcsSkill, ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_pcsBase || pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return FALSE;

	pstAgsmDBOperationArg	pstArg = (pstAgsmDBOperationArg) GlobalAlloc(GMEM_FIXED, sizeof(stAgsmDBOperationArg));
	if (!pstArg)	return FALSE;

	ZeroMemory(pstArg, sizeof(stAgsmDBOperationArg));

	pstArg->nDataType	= AGSMDB_DATATYPE_SKILL;
	pstArg->nOperation	= AGSMDB_OPERATION_UPDATE;
	pstArg->lID			= pcsSkill->m_lID;
	pstArg->pfCallback	= pfCallback;
	pstArg->pClass		= pClass;

	if (!GetUpdateQuery(pcsSkill, pstArg->szQuery))
	{
		GlobalFree(pstArg);
		return FALSE;
	}

	BOOL	bResult	= FALSE;

	bResult = m_pagsmDBStream->ExecuteQuery(pstArg, __FILE__, __LINE__ );

	if (!bResult)
		GlobalFree(pstArg);

	return bResult;
}

BOOL AgsmSkill::StreamDeleteDB(AgpdSkill *pcsSkill, ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_pcsBase || pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return FALSE;

	pstAgsmDBOperationArg	pstArg = (pstAgsmDBOperationArg) GlobalAlloc(GMEM_FIXED, sizeof(stAgsmDBOperationArg));
	if (!pstArg)	return FALSE;

	ZeroMemory(pstArg, sizeof(stAgsmDBOperationArg));

	pstArg->nDataType	= AGSMDB_DATATYPE_SKILL;
	pstArg->nOperation	= AGSMDB_OPERATION_DELETE;
	pstArg->lID			= pcsSkill->m_lID;
	pstArg->pfCallback	= pfCallback;
	pstArg->pClass		= pClass;

	CHAR	*szQuery = pstArg->szQuery;

	if (!GetDeleteQueryByDBID(pcsSkill, pstArg->szQuery))
	{
		GlobalFree(pstArg);
		return FALSE;
	}

	BOOL	bResult	= FALSE;

	bResult = m_pagsmDBStream->ExecuteQuery(pstArg, __FILE__, __LINE__ );

	if (!bResult)
		GlobalFree(pstArg);

	return bResult;
}

BOOL AgsmSkill::SaveSkillDIRT(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	AgsdSkill	*pcsAgsdSkill	= GetADSkill(pcsSkill);

	return m_pagsmFactors->StreamUpdateDBSkillPoint(&pcsSkill->m_csFactor, pcsSkill, NULL, pcsAgsdSkill->m_ullDBSkillID, NULL, NULL);
}

BOOL AgsmSkill::CBSaveCharacterDB(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill				*pThis			= (AgsmSkill *)				pClass;
	AgpdCharacter			*pcsCharacter	= (AgpdCharacter *)			pData;
	BOOL					*pbSave			= (BOOL *)					pCustData;

	// pcsCharacter가 소유하고 있는 스킬에 대한 정보를 저장한다.

	AgpdSkillAttachData		*pcsSkillAttachData	= pThis->m_pagpmSkill->GetAttachSkillData((ApBase *) pcsCharacter);
	if (!pcsSkillAttachData)
	{
		*pbSave = FALSE;
		return FALSE;
	}

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (pcsSkillAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
			continue;

		AgpdSkill	*pcsSkill = pThis->m_pagpmSkill->GetSkill(pcsSkillAttachData->m_alSkillID[i]);
		if (pcsSkill)
		{
			if (!pThis->StreamUpdateDB(pcsSkill, NULL, NULL))
			{
				*pbSave = FALSE;
				continue;
			}

			AgsdServer	*pcsServer	= pThis->m_pagsmServerManager->GetThisServer();
			if (!pcsServer)
				return FALSE;

			AgsdSkill	*pcsAgsdSkill	= pThis->GetADSkill(pcsSkill);

			// character factor를 DB에 저장한다
			if (!pThis->m_pagsmFactors->StreamUpdateDB(&pcsSkill->m_csFactor, 
												AGSMDB_DATATYPE_FACTOR_SKILL,
												(ApBase *) pcsSkill,
												NULL,
												pcsAgsdSkill->m_ullDBSkillID,
												0,
												NULL,
												NULL))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL AgsmSkill::CBStreamDB(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill				*pThis			= (AgsmSkill *)				pClass;
	COLEDB					*pcOLEDB		= (COLEDB *)				pData;
	pstAgsmDBOperationArg	pstOperationArg	= (pstAgsmDBOperationArg)	pCustData;

	if (pstOperationArg->nDataType != AGSMDB_DATATYPE_SKILL)
		return FALSE;

	switch (pstOperationArg->nOperation) {
	case AGSMDB_OPERATION_INSERT:
		{
			if (pstOperationArg->pfCallback && pstOperationArg->pClass)
				pstOperationArg->pfCallback(&pstOperationArg->lID, pstOperationArg->pClass, &pstOperationArg->bSuccess);
		}
		break;

	case AGSMDB_OPERATION_SELECT:
		{
			if (pstOperationArg->bSuccess)
			{
				INT16	nIndex = 0;

				CHAR	*szBuffer = NULL;

				UINT64	ulDBID	= 0;
				if ((szBuffer = pcOLEDB->GetQueryResult(nIndex++)) != NULL)
					ulDBID = (UINT64) _atoi64(szBuffer);

				CHAR	szTName[AGSMDB_MAX_NAME];
				ZeroMemory(szTName, sizeof(CHAR) * AGSMDB_MAX_NAME);
				if ((szBuffer = pcOLEDB->GetQueryResult(nIndex++)) != NULL)
					strcpy(szTName, szBuffer);

				if (ulDBID && szTName[0])
				{
					AgpdSkillTemplate	*pcsTemplate = pThis->m_pagpmSkill->GetSkillTemplate(szTName);
					if (pcsTemplate)
					{
						INT32	lSkillID = 0;
						pThis->EnumCallback(AGSMSKILL_CB_GET_NEW_SKILLID, &lSkillID, NULL);

						AgpdSkill	*pcsSkill = pThis->m_pagpmSkill->AddSkill(lSkillID, pcsTemplate->m_lID);
						if (pcsSkill)
						{
							AgsdSkill	*pcsAgsdSkill	= pThis->GetADSkill(pcsSkill);
							if (pcsAgsdSkill)
								pcsAgsdSkill->m_ullDBSkillID = ulDBID;
						}
					}
					else
						pstOperationArg->bSuccess = FALSE;
				}
				else
					pstOperationArg->bSuccess = FALSE;
			}
				
			if (pstOperationArg->pfCallback && pstOperationArg->pClass)
				pstOperationArg->pfCallback(&pstOperationArg->lID, pstOperationArg->pClass, &pstOperationArg->bSuccess);
		}
		break;

	case AGSMDB_OPERATION_UPDATE:
		{
			if (pstOperationArg->pfCallback && pstOperationArg->pClass)
				pstOperationArg->pfCallback(&pstOperationArg->lID, pstOperationArg->pClass, &pstOperationArg->bSuccess);
		}
		break;

	case AGSMDB_OPERATION_DELETE:
		{
			if (pstOperationArg->pfCallback && pstOperationArg->pClass)
				pstOperationArg->pfCallback(&pstOperationArg->lID, pstOperationArg->pClass, &pstOperationArg->bSuccess);
		}
		break;

	default:
		return FALSE;
		break;
	}

	return TRUE;
}
*/

BOOL AgsmSkill::CBSendCharacterViewInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	PVOID			*ppvBuffer		= (PVOID *)			pCustData;

	UINT32			ulNID			= PtrToUint(ppvBuffer[0]);
	BOOL			bGroupNID		= PtrToInt(ppvBuffer[1]);
	BOOL			bIsExceptSelf	= PtrToInt(ppvBuffer[2]);

	return pThis->SendAllBuffedSkill((ApBase *) pcsCharacter, ulNID, bGroupNID, (bIsExceptSelf) ? pThis->m_pagsmCharacter->GetCharDPNID(pcsCharacter) : 0);
}

BOOL AgsmSkill::CBSendCharacterAllInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	UINT32			ulNID			= *(UINT32 *)		pCustData;

#ifndef	__NEW_MASTERY__

	INT16	nPacketLength			= 0;
	PVOID	pvPacketAttachSkillData	= pThis->m_pagpmSkill->MakePacketAttachData((ApBase *) pcsCharacter, &nPacketLength, FALSE);

	if (nPacketLength > 0 && pvPacketAttachSkillData)
	{
		pThis->SendPacket(pvPacketAttachSkillData, nPacketLength, ulNID);

		pThis->m_pagpmSkill->m_csPacket.FreePacket(pvPacketAttachSkillData);
	}

#endif	//__NEW_MASTERY__

	/*
	PVOID	pvPacketMastery			= pThis->m_pagpmSkill->MakePacketAddMastery((ApBase *) pcsCharacter, &nPacketLength);
	if (nPacketLength > 0 && pvPacketMastery)
	{
		pThis->SendPacket(pvPacketMastery, nPacketLength, ulNID);

		pThis->m_pagpmSkill->m_csPacket.FreePacket(pvPacketMastery);
	}
	*/

	return pThis->SendSkillAll(pcsCharacter, ulNID);
}

BOOL AgsmSkill::CBSendCharacterNewID(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	UINT32			ulNID			= *(UINT32 *)		pCustData;

	return pThis->SendPacketDBData(pcsCharacter, ulNID);
}

BOOL AgsmSkill::CBSendCharacterAllServerInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	UINT32			*pulNID			= (UINT32 *)		pCustData;

	pThis->SendEquipSkill(pcsCharacter, pulNID[0]);

	return TRUE;
}

BOOL AgsmSkill::CBReCalcFactor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	AgsdSkillADBase	*pcsADBase		= pThis->GetADBase((ApBase *) pcsCharacter);
	if (!pcsADBase)
		return FALSE;

	int i = 0;
	for (i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			break;

		AgsdSkill	*pcsAgsdSkill	= pThis->GetADSkill(pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);

		pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorPoint, FALSE, FALSE, TRUE, FALSE);
		pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
		pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorPercent, FALSE, FALSE, TRUE, FALSE);
		pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);
	}

	AgpdSkillAttachData *pcsAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase *) pcsCharacter);
	if (!pcsAttachData)
		return FALSE;

	for (i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if (pcsAttachData->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
		{
			break;
		}

		AgpdSkill	*pcsPassiveSkill	= pThis->m_pagpmSkill->GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
		if (pcsPassiveSkill)
		{
			AgsdSkill	*pcsAgsdSkill	= pThis->GetADSkill(pcsPassiveSkill);

			pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorPoint, FALSE, FALSE, TRUE, FALSE);
			pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, FALSE, FALSE, TRUE, FALSE);
			pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorPercent, FALSE, FALSE, TRUE, FALSE);
			pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, FALSE, FALSE, TRUE, FALSE);
		}
	}

	return TRUE;
}

// 2005.08.25. steeple
// 변신물약 사용시 콜백
BOOL AgsmSkill::CBTransformStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pClass || !pData)
		return FALSE;

	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdCharacter* pcsCharacter	= (AgpdCharacter*)pData;

	// 2005.12.28. steeple
	// 디버프만 남기고 다 꺼버린다.
	pThis->EndAllBuffedSkillExceptDebuff((ApBase*)pcsCharacter);

	// 사용되지 않아야 하는 유료 아이템 중지
	pThis->m_pagsmItem->UnUseAllCashItem(pcsCharacter, TRUE, AGSDITEM_PAUSE_REASON_TRANSFORM);

	// 사용해야 하는 유료아이템(스킬) Use
	pThis->m_pagsmItem->UseAllEnableCashItem(pcsCharacter, TRUE);

	return TRUE;
}

// 2006.01.06. steeple
// 변신물약 끝난 후 콜백
BOOL AgsmSkill::CBTransformRestore(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pClass || !pData)
		return FALSE;

	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pData);
	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);

	// 일시정지 된 캐쉬아이템 스킬스크롤을 다시 시작해준다.
	pThis->m_pagsmItem->UsePausedCashItem(pcsCharacter, AGSDITEM_PAUSE_REASON_TRANSFORM);
	pThis->RecastSaveSkill(pcsCharacter);
	
	return TRUE;
}

BOOL AgsmSkill::CBEvolution(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pClass || !pData)
		return FALSE;

	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgsmSkill* pThis			= (AgsmSkill*)pClass;

	pThis->EndBuffedSkillOnEvolution((ApBase*)pcsCharacter);

	// 사용이 멈춰야 하는 유료아이템 중지
	//pThis->m_pagsmItem->UnUseAllCashItem(pcsCharacter, TRUE, AGSDITEM_PAUSE_REASON_EVOLUTION);

	// 사용중인 유료아이템 Use
	//pThis->m_pagsmItem->UseAllEnableCashItem(pcsCharacter, TRUE);
	
	return TRUE;
}

BOOL AgsmSkill::CBRestoreEvolution(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pClass || !pData)
		return FALSE;

	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgsmSkill* pThis			= (AgsmSkill*)pClass;

	pThis->EndBuffedSkillOnEvolution((ApBase*)pcsCharacter);

	// 사용이 멈춰야 하는 유료아이템 중지
	//pThis->m_pagsmItem->UnUseAllCashItem(pcsCharacter, TRUE, AGSDITEM_PAUSE_REASON_EVOLUTION);

	// 사용중인 유료아이템 Use
	//pThis->m_pagsmItem->UseAllEnableCashItem(pcsCharacter, TRUE);
	
	return TRUE;
}

// 2005.09.18. steeple
// 소환 성공 후 콜백 from AgsmSummons
BOOL AgsmSkill::CBNewSummons(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdCharacter* pcsSummons = static_cast<AgpdCharacter*>(pData);
	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdCharacter* pcsOwner = static_cast<AgpdCharacter*>(pCustData);

	// Owner 가 Summons 에게 뭔가 해줄 것이 있으면 해준다.
	AgpdSkillAttachData* pcsAttachData = pThis->m_pagpmSkill->GetAttachSkillData(pcsOwner);
	if(!pcsAttachData)
		return FALSE;

	// SummonsType5 를 체크해본다.
	if(pcsAttachData->m_stBuffedSkillCombatEffectArg.lSummonsType5Count > 0)
	{
		pThis->ProcessSummonsType5InBuffedList(pcsOwner, pcsSummons);
		pThis->ProcessSummonsType5InPassiveList(pcsOwner, pcsSummons);
	}

	return TRUE;
}

// 2005.10.10. steeple
BOOL AgsmSkill::CBEndSummons(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdCharacter* pcsSummons = static_cast<AgpdCharacter*>(pData);
	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdCharacter* pcsOwner = static_cast<AgpdCharacter*>(pCustData);

	// Owner 의 Summons List 에 같은 TID 의 Summons 이 없을 때만 한다.
	if(pThis->m_pagpmSummons->GetSummonsTIDCount(pcsOwner, pcsSummons->m_pcsCharacterTemplate->m_lID) != 0)
		return TRUE;

	// Owner 의 Summons 관련 버프중에서 Summons 의 TID 를 들고 있는 놈을 없앤다.
	pThis->EndSummonsBuffedSkill(pcsOwner, TRUE, pcsSummons->m_pcsCharacterTemplate->m_lID);

	return TRUE;
}

// 2005.10.08. steeple
// 테이밍이 끝난 후 콜백. (시간으로 끝났건, 소유주가 로그아웃 해서 끝났건 다 불림.)
BOOL AgsmSkill::CBEndTaming(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgpdCharacter* pcsTame = static_cast<AgpdCharacter*>(pData);
	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);

	// Taming 에 관련한 스킬을 없애뿐진다.
	pThis->EndTameBuffedSkill(pcsTame, TRUE);

	return TRUE;
}

// 2005.10.10. steeple
BOOL AgsmSkill::CBEndFixed(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdCharacter* pcsFixed = static_cast<AgpdCharacter*>(pData);
	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdCharacter* pcsOwner = static_cast<AgpdCharacter*>(pCustData);

	// Owner 의 Fixed List 에 같은 TID 의 Fixed 가 없을 때만 한다.
	if(pThis->m_pagpmSummons->GetFixedTIDCount(pcsOwner, pcsFixed->m_pcsCharacterTemplate->m_lID) != 0)
		return TRUE;

	// Owner 의 Summons 관련 버프중에서 Fixed 의 TID 를 들고 있는 놈을 없앤다.
	pThis->EndSummonsBuffedSkill(pcsOwner, TRUE, pcsFixed->m_pcsCharacterTemplate->m_lID);

	return TRUE;
}

// 2005.10.10. steeple
// 원래 UpdateSpecialStatus 는 AgsmCharacter 에서만 했었는데, 이곳에서도 필요한 처리가 있어서 여기도 등록했다.
BOOL AgsmSkill::CBUpdateSpecialStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdCharacter* pcsCharacter= (AgpdCharacter*)pData;
	AgsmSkill* pThis = (AgsmSkill*)pClass;
//	UINT64* pulOldStatus = (UINT64*)pCustData;

	PVOID *pStatus		= (PVOID *) pCustData;

	UINT64* pulOldStatus = (UINT64 *)pStatus[0];
	INT32*  pReserved    = (INT32 *)pStatus[1];

	// 업데이트를 했는데 같다.
	if(*pulOldStatus == pcsCharacter->m_ulSpecialStatus)
		return TRUE;

	// 2005.12.28. steeple
	// 투명이 되었을 때 스킬 및 캐쉬아이템 관련 처리를 해준다.
	if(!(*pulOldStatus & AGPDCHAR_SPECIAL_STATUS_TRANSPARENT) &&
		(pcsCharacter->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_TRANSPARENT))
	{
		// 디버프만 남기고 버프는 다 꺼버린다.
		pThis->EndAllBuffedSkillExceptDebuff(pcsCharacter, TRUE);

		// 사용이 멈춰야 하는 유료아이템 중지
		pThis->m_pagsmItem->UnUseAllCashItem(pcsCharacter, TRUE, AGSDITEM_PAUSE_REASON_INVISIBLE);

		// 사용중인 유료아이템 Use
		pThis->m_pagsmItem->UseAllEnableCashItem(pcsCharacter, TRUE);

		// 소환수들에게도 투명 걸어준다.
		pThis->m_pagsmSummons->UpdateTransparentToSummons(pcsCharacter, TRUE);
	}

	// 투명이 풀린 거라면, 투명 관련한 스킬 버프를 꺼준다.
	if((*pulOldStatus & AGPDCHAR_SPECIAL_STATUS_TRANSPARENT) &&
		!(pcsCharacter->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_TRANSPARENT))
	{
		pThis->EndTransparentBuffedSkill(pcsCharacter, TRUE);

		// 2006.01.06. steeple
		// 일시 정지된 캐쉬 아이템 스킬 스크롤을 사용한다.
		pThis->m_pagsmItem->UsePausedCashItem(pcsCharacter, AGSDITEM_PAUSE_REASON_INVISIBLE);
		pThis->RecastSaveSkill(pcsCharacter);

		// 소환수들 투명을 빼준다.
		pThis->m_pagsmSummons->UpdateTransparentToSummons(pcsCharacter, FALSE);
	}

	// 무장해제 관련 것이 풀린거라면, 무기레벨을 다시 정상적으로 돌려놓는다.
	if((*pulOldStatus & AGPDCHAR_SPECIAL_STATUS_DISARMAMENT) &&
		!(pcsCharacter->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_DISARMAMENT))
	{
		AgpdItem *pcsEquipItem = NULL;

		if(pcsCharacter->m_ulSpecialStatusReserved == (INT32)AGPMITEM_PART_HAND_RIGHT)
		{
			pcsEquipItem = pThis->m_pagpmItem->GetEquipWeapon(pcsCharacter);

			// 빠진 Part를 다시 초기화 시켜준다.
			if(pcsCharacter->m_ulSpecialStatusReserved != 0)
				pcsCharacter->m_ulSpecialStatusReserved = 0;

			if(!pcsEquipItem)
				return TRUE;
		}
		else
		{
			pcsEquipItem = pThis->m_pagpmItem->GetEquipSlotItem(pcsCharacter, (AgpmItemPart)pcsCharacter->m_ulSpecialStatusReserved);

			// 빠진 Part를 다시 초기화 시켜준다.
			if(pcsCharacter->m_ulSpecialStatusReserved != 0)
				pcsCharacter->m_ulSpecialStatusReserved = 0;

			if(NULL == pcsEquipItem)
				return TRUE;
		}

		if(pThis->m_pagpmItem->GetWeaponType(pcsEquipItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_RAPIER)
		{
			AgpdItem* pcsLeftItem = pThis->m_pagpmItem->GetEquipSlotItem(pcsCharacter, AGPMITEM_PART_HAND_LEFT);
			if(pcsLeftItem && pcsLeftItem->m_pcsItemTemplate && pcsLeftItem->m_lID != pcsEquipItem->m_lID &&
				pThis->m_pagpmItem->GetWeaponType(pcsLeftItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_DAGGER)
			{
				if (pcsLeftItem->m_lStatusFlag & AGPMITEM_STATUS_DISARMAMENT)
				{
					pcsLeftItem->m_lStatusFlag ^= AGPMITEM_STATUS_DISARMAMENT;

					pThis->m_pagpmItem->EnumCallback(ITEM_CB_ID_CHAR_EQUIP, pcsLeftItem, pcsCharacter);
				}
			}
		}

		if (pcsEquipItem->m_lStatusFlag & AGPMITEM_STATUS_DISARMAMENT)
		{
			pcsEquipItem->m_lStatusFlag ^= AGPMITEM_STATUS_DISARMAMENT;

			pThis->m_pagpmItem->EnumCallback(ITEM_CB_ID_CHAR_EQUIP, pcsEquipItem, pcsCharacter);
		}
	}

	return TRUE;
}

// 2005.12.09. steeple
// 캐쉬 아이템 사용을 중지 시켰다.
BOOL AgsmSkill::CBUnUseCashItem(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgpdItem* pcsItem = static_cast<AgpdItem*>(pData);
	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);

	if(!pcsItem->m_pcsItemTemplate)
		return FALSE;

	// 여기에 아바타 아이템도 추가. 2008.01.25. steeple
	// 캐쉬 아이템이고, 유저블이고, 스킬 스크롤이라면
	if(IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType) &&
		(pThis->m_pagpmItem->IsAvatarItem(pcsItem->m_pcsItemTemplate) ||
		(pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
		((AgpdItemTemplateUsable*)pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_SKILL_SCROLL))
		)
	{
		pThis->EndCashItemBuffedSkill((ApBase*)pcsItem->m_pcsCharacter,
									((AgpdItemTemplateUsableSkillScroll*)pcsItem->m_pcsItemTemplate)->m_lSkillTID);
	}
	else if ( pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
		((AgpdItemTemplateUsable*)pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_JANUS_TALISMAN)
	{
		BOOL bResult = FALSE;
		bResult = pThis->EndCashItemBuffedSkill((ApBase*)pcsItem->m_pcsCharacter,
									((AgpdItemTemplateUsableJanusTalisman*)pcsItem->m_pcsItemTemplate)->m_lSkillTID);
		if (bResult)
		{
			pThis->m_pagsmItem->SetCharacterJanusTypeFromItem(pcsItem->m_pcsCharacter, AGPMITEM_USABLE_JANUS_TALISMAN_TYPE_NONE);
		}
	}

	return TRUE;
}

// 2006.01.05. steeple
// 스킬 스크롤이 가능한지 아이템 사용하기 전 단계에서 미리 체크한다.
BOOL AgsmSkill::CBPreCheckEnableSkillScroll(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdItem* pcsItem = static_cast<AgpdItem*>(pData);
	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pCustData);

	BOOL bResult = pThis->IsEnableSkillByBuffedList(pcsItem, (ApBase*)pcsCharacter);

	if(!bResult)
	{
		// System Message 보낸다. 2006.01.05. steeple. 임시 땜빵
		if(pThis->m_pagpmPvP)
			pThis->m_pagpmPvP->ProcessSystemMessage(AGPMPVP_SYSTEM_CODE_SKILL_CANNOT_APPLY_EFFECT, NULL, NULL, 0, 0, pcsCharacter);
	}

	return bResult;
}

// 2006.10.17. steeple
// 아크로드가 되었을 때 불리는 콜백.
// 아크로드용 스킬을 배우게 해준다.
BOOL AgsmSkill::CBSetArchlord(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	CHAR* szCharName = static_cast<CHAR*>(pData);

	// 기존아크로드와 같다면 할 필요 없다.
	if(pThis->m_szArchlord.IsEmpty() == FALSE &&
		pThis->m_szArchlord.Compare(szCharName) == COMPARE_EQUAL)
		return FALSE;

	// 다르다면 비워준다.
	CBCancelArchlord(szCharName, pThis, NULL);

	pThis->m_szArchlord.SetText(szCharName);

	AgpdCharacter* pcsCharacter = pThis->m_pagpmCharacter->GetCharacter(szCharName);
	if(pcsCharacter)
	{
		if (pcsCharacter->m_Mutex.WLock())
		{
			pThis->LearnArchlordSkill(pcsCharacter);
			pcsCharacter->m_Mutex.Release();
		}
	}

	return TRUE;
}

BOOL AgsmSkill::CBCancelArchlord(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pClass)
		return FALSE;

	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	CHAR* szCharName = static_cast<CHAR*>(pData);

	AgpdCharacter* pcsCharacter = pThis->m_pagpmCharacter->GetCharacter(szCharName);
	if(pcsCharacter)
	{
		if (pcsCharacter->m_Mutex.WLock())
		{
			pThis->RemoveArchlordSkill(pcsCharacter);
			pcsCharacter->m_Mutex.Release();
		}
	}

	pThis->m_szArchlord.Clear();

	return TRUE;
}

// 2007.02.06. steeple
BOOL AgsmSkill::CBModifiedSkillLevel(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pData);

	// Modified Skill Level 을 새로 계산해서 보내준다.
	AgpdSkillAttachData* pcsAttachData = pThis->m_pagpmSkill->GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return FALSE;

	AgsdCharacter *pcsAgsdCharacter = pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);
	if(!pcsAgsdCharacter)
		return FALSE;

	pcsAttachData->m_lModifiedSkillLevel = pcsAttachData->m_stBuffedSkillCombatEffectArg.lSkillLevelUpPoint +
											pcsAgsdCharacter->m_stOptionSkillData.m_lSkillLevelUp;

	pThis->SendModifiedSkillLevel((ApBase*)pcsCharacter);

	// 2007.02.13. steeple
	// Passive Skill 은 다시 캐스팅 해준다.
	pThis->ReCastAllPassiveSkill((ApBase*)pcsCharacter);

	return TRUE;
}

// 2008.07.28. iluvs
BOOL AgsmSkill::CBGetModifiedSkillLevel(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdSkill* pcsSkill = static_cast<AgpdSkill*>(pData);
	INT32 lModifiedLevel = *static_cast<INT32*>(pCustData);

	lModifiedLevel = pThis->GetModifiedSkillLevel(pcsSkill);

	*static_cast<INT32*>(pCustData) = lModifiedLevel;

	return TRUE;
}

// 2008.01.28. steeple
BOOL AgsmSkill::CBEndBuffedSkillByItem(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pData);
	AgpdItem* pcsItem = static_cast<AgpdItem*>(pCustData);

	// 해당 아이템의 TID 로 Buff 된 스킬을 꺼준다. Cash 템 끄는 것과 거의 비슷.
	pThis->EndBuffedSkillByTID(pcsCharacter, ((AgpdItemTemplateUsableSkillScroll*)pcsItem->m_pcsItemTemplate)->m_lSkillTID);

	return TRUE;
}

// 2008.04.07. steeple
BOOL AgsmSkill::CBUseItemReverseOrb(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pData);
	AgpdItem* pcsItem = static_cast<AgpdItem*>(pCustData);

	pThis->RecastSaveSkill(pcsCharacter);
	
	// 2008.04.08. steeple
	// 디버프 스킬 뺐다. -_-; 투명스킬 쓰면 날아가는 거 때문에;;
	return TRUE;

	// 리버스 오브를 썼을 때의 상황에 맞는 컨디션 스킬 불러준다.
	INT32 lConditionType = AGPMSKILL_CONDITION2_START_ON_RESURRECTION;
		lConditionType |= AGPMSKILL_CONDITION2_RELY_ON_REGION;
		lConditionType |= AGPMSKILL_CONDITION2_DONT_USE_BY_CASH;
	pThis->CastCondition2Skill(pcsCharacter, lConditionType, IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType));

	return TRUE;
}

BOOL AgsmSkill::SendSkillAll(AgpdCharacter *pcsCharacter, UINT32 ulDPNID)
{
	if (!pcsCharacter || ulDPNID == 0)
		return FALSE;

	AgpdSkillAttachData	*pcsSkillAttachData	= (AgpdSkillAttachData *) m_pagpmSkill->GetAttachSkillData((ApBase *) pcsCharacter);
	if (!pcsSkillAttachData)
		return FALSE;

//#ifdef	__NEW_MASTERY__
//
//	SendMasteryInfo((ApBase *) pcsCharacter, ulDPNID);
//
//#else
//
//	// send mastery information
//	INT16	nPacketLength	= 0;
//	PVOID	pvPacket		= m_pagpmSkill->MakePacketAddMastery((ApBase *) pcsCharacter, &nPacketLength);
//
//	if (pvPacket && nPacketLength > 0)
//	{
//		SendPacket(pvPacket, nPacketLength, ulDPNID);
//
//		m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
//	}
//
//#endif	//__NEW_MASTERY__

	// send skill information
	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (!pcsSkillAttachData->m_alSkillID[i])
			break;

		AgpdSkill	*pcsSkill = m_pagpmSkill->GetSkill(pcsSkillAttachData->m_alSkillID[i]);
		if (!pcsSkill)
			continue;

		INT16	nPacketLength	= 0;
		PVOID	pvPacket		= m_pagpmSkill->MakePacketSkill(pcsSkill, &nPacketLength);
		
		if (pvPacket)
		{
			SendPacket(pvPacket, nPacketLength, ulDPNID);

			m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
		}
	}
	
	return TRUE;
}
//JK_패시브스킬 표시 오류
BOOL AgsmSkill::SendPassiveSkillAll(AgpdCharacter *pcsCharacter, UINT32 ulDPNID)
{
	if (!pcsCharacter || ulDPNID == 0)
		return FALSE;

	AgpdSkillAttachData	*pcsSkillAttachData	= (AgpdSkillAttachData *) m_pagpmSkill->GetAttachSkillData((ApBase *) pcsCharacter);
	if (!pcsSkillAttachData)
		return FALSE;

	// send skill information
	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (!pcsSkillAttachData->m_alSkillID[i])
			break;

		AgpdSkill	*pcsSkill = m_pagpmSkill->GetSkill(pcsSkillAttachData->m_alSkillID[i]);
		if (!pcsSkill)
			continue;

		if(!m_pagpmSkill->IsPassiveSkill(pcsSkill))
			continue;

		INT16	nPacketLength	= 0;
		PVOID	pvPacket		= m_pagpmSkill->MakePacketSkill(pcsSkill, &nPacketLength);

		if (pvPacket)
		{
			SendPacket(pvPacket, nPacketLength, ulDPNID);

			m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
		}
	}

	return TRUE;
}

BOOL AgsmSkill::SendPacketDBData(AgpdCharacter *pcsCharacter, UINT32 ulNID)
{
	if (!pcsCharacter || ulNID == 0)
		return FALSE;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= MakePacketAddDBData((ApBase *) pcsCharacter, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL	bSendResult		= SendPacket(pvPacket, nPacketLength, ulNID);

	m_csPacket.FreePacket(pvPacket);

	return	bSendResult;
}

BOOL AgsmSkill::SendEquipSkill(AgpdCharacter* pcsCharacter, UINT32 ulNID)
{
	if (!pcsCharacter || ulNID == 0)
		return FALSE;

	AgsdSkillADBase	*pcsADBase	= GetADBase((ApBase*)pcsCharacter);
	if (!pcsADBase)
		return FALSE;

	if(pcsADBase->m_csEquipSkillArray.empty())
		return FALSE;

	INT32 nIndex = 0;
	PACKET_AGSMSKILL_EQUIPLIST pPacketEquipSkill(pcsCharacter->m_lID);

	for(vector<AgsdEquipSkill>::iterator it = pcsADBase->m_csEquipSkillArray.begin(); it != pcsADBase->m_csEquipSkillArray.end(); ++it)
	{
		pPacketEquipSkill.m_csEquipSkillArray[nIndex++] = (*it);

		if(nIndex >= AGPMSKILL_MAX_SKILL_BUFF) break;
	}

	pPacketEquipSkill.nSkillCount = nIndex;

	AgsEngine::GetInstance()->SendPacket(pPacketEquipSkill, ulNID);

	return TRUE;
}

PVOID AgsmSkill::MakePacketAgsdSkill(AgpdSkill *pcsSkill, INT16 *pnPacketLength, BOOL bLogin)
{
	if (!pcsSkill || !pnPacketLength)
		return NULL;

	AgsdSkill	*pcsAgsdSkill	= GetADSkill(pcsSkill);
	if (!pcsAgsdSkill)
		return NULL;

	// 지금 패킷이 좀 잘못되어있다. 고로.. 일단은 NULL을 리턴한다

	return NULL;

	PVOID	pvPacket = NULL;

	if (!bLogin)
	{
		PVOID	pvPacketUpdateFactorResult	= m_pagpmFactors->MakePacketFactors(pcsAgsdSkill->m_pcsUpdateFactorResult);
		PVOID	pvPacketUpdateFactorPoint	= m_pagpmFactors->MakePacketFactors(pcsAgsdSkill->m_pcsUpdateFactorPoint);
		PVOID	pvPacketUpdateFactorPercent	= m_pagpmFactors->MakePacketFactors(pcsAgsdSkill->m_pcsUpdateFactorPercent);

		/*
		PVOID	pvPacketModifyFactorPoint[AGPMSKILL_MAX_AFFECTED_BASE];
		PVOID	pvPacketModifyFactorPercent[AGPMSKILL_MAX_AFFECTED_BASE];

		ZeroMemory(pvPacketModifyFactorPoint, sizeof(PVOID) * AGPMSKILL_MAX_AFFECTED_BASE);
		ZeroMemory(pvPacketModifyFactorPercent, sizeof(PVOID) * AGPMSKILL_MAX_AFFECTED_BASE);

		for (int i = 0; i < AGPMSKILL_MAX_AFFECTED_BASE; ++i)
		{
			pvPacketModifyFactorPoint[i] = m_pagpmFactors->MakePacketFactors(&pcsAgsdSkill->m_csModifyFactorPoint[i]);
			pvPacketModifyFactorPercent[i] = m_pagpmFactors->MakePacketFactors(&pcsAgsdSkill->m_csModifyFactorPercent[i]);
		}
		*/

		INT16	nLenCombatEffectArg		= sizeof(AgpdSkillBuffedCombatEffectArg);
		INT16	nLenFactorEffectArg		= sizeof(AgpdSkillBuffedFactorEffectArg);

		INT8	cOperation	= AGSMSKILL_PACKET_OPERATION_UPDATE_AGSD_SKILL;
		pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMSKILL_PACKET_TYPE,
										&cOperation,
										&pcsSkill->m_lID,
										pvPacketUpdateFactorResult,
										pvPacketUpdateFactorPoint,
										pvPacketUpdateFactorPercent,
										//pvPacketModifyFactorPoint,
										//pvPacketModifyFactorPercent,
										&pcsAgsdSkill->m_lModifySkillLevel,
										&pcsAgsdSkill->m_lModifyCharLevel,
										&pcsAgsdSkill->m_nMeleeDefensePoint,
										&pcsAgsdSkill->m_nMeleeReflectPoint,
										&pcsAgsdSkill->m_nMagicDefensePoint,
										&pcsAgsdSkill->m_nMagicReflectPoint,
										&pcsAgsdSkill->m_lDamageShield,
										&pcsAgsdSkill->m_stBuffedSkillCombatEffectArg, &nLenCombatEffectArg,
										&pcsAgsdSkill->m_stBuffedSkillFactorEffectArg, &nLenFactorEffectArg,
										NULL,
										NULL
										);

		if (pvPacketUpdateFactorResult)
			m_csPacket.FreePacket(pvPacketUpdateFactorResult);
		if (pvPacketUpdateFactorPoint)
			m_csPacket.FreePacket(pvPacketUpdateFactorPoint);
		if (pvPacketUpdateFactorPercent)
			m_csPacket.FreePacket(pvPacketUpdateFactorPercent);

		/*
		for (i = 0; i < AGPMSKILL_MAX_AFFECTED_BASE; ++i)
		{
			if (pvPacketModifyFactorPoint[i])
				m_csPacket.FreePacket(pvPacketModifyFactorPoint[i]);
			if (pvPacketModifyFactorPercent[i])
				m_csPacket.FreePacket(pvPacketModifyFactorPercent[i]);
		}
		*/
	}
	else
	{
		INT8	cOperation	= AGSMSKILL_PACKET_OPERATION_UPDATE_AGSD_SKILL;
		pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMSKILL_PACKET_TYPE,
										&cOperation,
										&pcsSkill->m_lID,
										NULL,
										NULL,
										NULL,
										//NULL,
										//NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL,
										NULL
										);
	}

	return pvPacket;
}

PVOID AgsmSkill::MakePacketBuffedSkillProcessInfo(AgsdSkillProcessInfo *pcsSkillProcessInfo)
{
	if (!pcsSkillProcessInfo)
		return NULL;

	PVOID	pvPacketTempSkill	= MakePacketTempSkillInfo(pcsSkillProcessInfo->m_pcsTempSkill);

	INT8	cBaseType		= (INT8) pcsSkillProcessInfo->m_csSkillOwner.m_eType;
	PVOID	pvPacket		= m_csPacketBuffedSkillProcessInfo.MakePacket(FALSE, NULL, 0,
																		  &cBaseType,
																		  pcsSkillProcessInfo->m_csSkillOwner.m_lID,
																		  pcsSkillProcessInfo->m_lMasteryPoint,
																		  pvPacketTempSkill);

	if (pvPacketTempSkill)
		m_csPacketTempSkillInfo.FreePacket(pvPacketTempSkill);

	return pvPacket;
}

PVOID AgsmSkill::MakePacketTempSkillInfo(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return NULL;

	AgsdSkill	*pcsAgsdSkill	= GetADSkill(pcsSkill);
	if (!pcsAgsdSkill)
		return NULL;

	PVOID	pvPacketModifyFactorPoint	= m_pagpmFactors->MakePacketFactors(pcsAgsdSkill->m_pcsUpdateFactorPoint);
	PVOID	pvPacketModifyFactorPercent	= m_pagpmFactors->MakePacketFactors(pcsAgsdSkill->m_pcsUpdateFactorPercent);

	INT32	lSkillLevel			= GetModifiedSkillLevel(pcsSkill);

	UINT32	ulClockCount		= GetClockCount();

	PVOID	pvPacket	= m_csPacketTempSkillInfo.MakePacket(FALSE, NULL, 0,
															 &pcsSkill->m_lID,
															 &lSkillLevel,
															 &pcsSkill->m_ulEndTime,
															 &pcsSkill->m_ulInterval,
															 &pcsSkill->m_ulNextProcessTime,
															 pvPacketModifyFactorPoint,
															 pvPacketModifyFactorPercent,
															 &pcsAgsdSkill->m_nMeleeDefensePoint,
															 &pcsAgsdSkill->m_nMeleeReflectPoint,
															 &pcsAgsdSkill->m_nMagicDefensePoint,
															 &pcsAgsdSkill->m_nMagicReflectPoint,
															 &ulClockCount);

	if (pvPacketModifyFactorPoint)
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketModifyFactorPoint);
	if (pvPacketModifyFactorPercent)
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketModifyFactorPercent);

	return pvPacket;
}

PVOID AgsmSkill::MakePacketAddDBData(ApBase *pcsBase, INT16 *pnPacketLength)
{
	if (!pcsBase || !pnPacketLength)
		return NULL;

	AgsdSkillADBase		*pcsADBase		= GetADBase(pcsBase);

	if (!pcsADBase->m_pstMasteryInfo)
		return NULL;

//	PVOID	pvPacketMasteryData[AGPMSKILL_MAX_MASTERY];
//
//	ZeroMemory(pvPacketMasteryData, sizeof(PVOID) * AGPMSKILL_MAX_MASTERY);
//
//	for (int i = 0; i < AGPMSKILL_MAX_MASTERY; ++i)
//	{
//		if (pcsADBase->m_pstMasteryInfo[i].lTotalSP == 0)
//			continue;
//
//		INT8	cMasteryIndex	= (INT8) pcsADBase->m_pstMasteryInfo[i].lMasteryIndex;
//		UINT8	ucInputSP		= (UINT8) pcsADBase->m_pstMasteryInfo[i].lTotalSP;
//
//		INT16	nSkillTreeLength	= 0;
//		if (pcsADBase->m_pstMasteryInfo[i].szSkillTree)
//			nSkillTreeLength	= strlen(pcsADBase->m_pstMasteryInfo[i].szSkillTree);
//
//		INT16	nSpecializeLength	= 0;
//		if (pcsADBase->m_pstMasteryInfo[i].szSpecialize)
//			nSpecializeLength	= strlen(pcsADBase->m_pstMasteryInfo[i].szSpecialize);
//
//		if (nSkillTreeLength > 0)
//		{
//			pvPacketMasteryData[i] = m_csPacketMasteryDBData.MakePacket(FALSE, NULL, 0,
//											&cMasteryIndex,
//											&ucInputSP,
//											pcsADBase->m_pstMasteryInfo[i].szSkillTree, &nSkillTreeLength,
//											pcsADBase->m_pstMasteryInfo[i].szSpecialize, &nSpecializeLength);
//		}
//		else
//		{
//			pvPacketMasteryData[i] = m_csPacketMasteryDBData.MakePacket(FALSE, NULL, 0,
//											&cMasteryIndex,
//											&ucInputSP,
//											NULL,
//											pcsADBase->m_pstMasteryInfo[i].szSpecialize, &nSpecializeLength);
//		}
//	}
//
//	PVOID	pvPacketDBData	= m_csPacketDBData.MakePacket(FALSE, NULL, 0,
//											&pcsBase->m_lID,
//											pvPacketMasteryData[0],
//											pvPacketMasteryData[1],
//											pvPacketMasteryData[2],
//											pvPacketMasteryData[3],
//											pvPacketMasteryData[4],
//											pvPacketMasteryData[5]);
//
//	for (i = 0; i < AGPMSKILL_MAX_MASTERY; ++i)
//	{
//		if (pvPacketMasteryData[i])
//			m_csPacketMasteryDBData.FreePacket(pvPacketMasteryData[i]);
//	}

	PVOID	pvPacketMasteryData	= NULL;

	INT16	nSkillTreeLength	= 0;
	if (pcsADBase->m_pstMasteryInfo->szSkillTree)
		nSkillTreeLength	= (INT16)strlen(pcsADBase->m_pstMasteryInfo->szSkillTree);

	INT16	nSpecializeLength	= 0;
	if (pcsADBase->m_pstMasteryInfo->szSpecialize)
		nSpecializeLength	= (INT16)strlen(pcsADBase->m_pstMasteryInfo->szSpecialize);

	INT16	nProductComposeLength	= 0;
	if (pcsADBase->m_pstMasteryInfo->szProductCompose)
		nProductComposeLength	= (INT16)strlen(pcsADBase->m_pstMasteryInfo->szProductCompose);	

	if (nSkillTreeLength > 0)
	{
		pvPacketMasteryData	= m_csPacketMasteryDBData.MakePacket(FALSE, NULL, 0,
										pcsADBase->m_pstMasteryInfo->szSkillTree, &nSkillTreeLength,
										pcsADBase->m_pstMasteryInfo->szSpecialize, &nSpecializeLength,
										pcsADBase->m_pstMasteryInfo->szProductCompose, &nProductComposeLength);
	}
	else
	{
		pvPacketMasteryData	= m_csPacketMasteryDBData.MakePacket(FALSE, NULL, 0,
										NULL,
										pcsADBase->m_pstMasteryInfo->szSpecialize, &nSpecializeLength,
										pcsADBase->m_pstMasteryInfo->szProductCompose, &nProductComposeLength);
	}

	if (!pvPacketMasteryData)
		return NULL;

	PVOID	pvPacketDBData	= m_csPacketDBData.MakePacket(FALSE, NULL, 0,
											&pcsBase->m_lID,
											pvPacketMasteryData);

	if (pvPacketMasteryData)
		m_csPacketMasteryDBData.FreePacket(pvPacketMasteryData);

	if (!pvPacketDBData)
		return NULL;

	INT8	cOperation	= AGSMSKILL_PACKET_OPERATION_ADD_DB_DATA;

	PVOID	pvPacket	= m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMSKILL_PACKET_TYPE,
												&cOperation,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												NULL,
												pvPacketDBData);

	m_csPacketDBData.FreePacket(pvPacketDBData);

	return pvPacket;
}

BOOL AgsmSkill::OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)
{
	if (!pvPacket ||
		nSize <= 0 ||
		ulNID == 0)
		return FALSE;

	if (!pstCheckArg->bReceivedFromServer)
		return FALSE;

	INT8		cOperation						= (-1);
	INT32		lSkillID						= 0;
	PVOID		pvPacketUpdateFactorResult		= NULL;
	PVOID		pvPacketUpdateFactorPoint		= NULL;
	PVOID		pvPacketUpdateFactorPercent		= NULL;
	//PVOID		pvPacketModifyFactorPoint[AGPMSKILL_MAX_AFFECTED_BASE];
	//PVOID		pvPacketModifyFactorPercent[AGPMSKILL_MAX_AFFECTED_BASE];
	INT32		lModifySkillLevel				= 0;
	INT32		lModifyCharLevel				= 0;
	INT32		lMeleeDefensePoint				= 0;
	INT32		lMeleeReflectPoint				= 0;
	INT32		lMagicDefensePoint				= 0;
	INT32		lMagicReflectPoint				= 0;
	INT32		lDamageShield					= 0;

	CHAR		*szCombatEffectArg				= NULL;
	INT16		nCombatEffectArgLength			= 0;

	CHAR		*szFactorEffectArg				= NULL;
	INT16		nFactorEffectArgLength			= 0;

	PVOID		pvPacketAttachBaseData			= NULL;
	PVOID		pvPacketDBData					= NULL;

	//ZeroMemory(pvPacketModifyFactorPoint, sizeof(PVOID) * AGPMSKILL_MAX_AFFECTED_BASE);
	//ZeroMemory(pvPacketModifyFactorPercent, sizeof(PVOID) * AGPMSKILL_MAX_AFFECTED_BASE);

	m_csPacket.GetField(TRUE, pvPacket, nSize,
						&cOperation,
						&lSkillID,
						&pvPacketUpdateFactorResult,
						&pvPacketUpdateFactorPoint,
						&pvPacketUpdateFactorPercent,
						//pvPacketModifyFactorPoint,
						//pvPacketModifyFactorPercent,
						&lModifySkillLevel,
						&lModifyCharLevel,
						&lMeleeDefensePoint,
						&lMeleeReflectPoint,
						&lMagicDefensePoint,
						&lMagicReflectPoint,
						&lDamageShield,
						&szCombatEffectArg, &nCombatEffectArgLength,
						&szFactorEffectArg, &nFactorEffectArgLength,
						&pvPacketAttachBaseData,
						&pvPacketDBData);

	switch (cOperation) {
	case AGSMSKILL_PACKET_OPERATION_UPDATE_AGSD_SKILL:
		{
			AgpdSkill	*pcsSkill	= m_pagpmSkill->GetSkillLock(lSkillID);
			if (!pcsSkill)
				return FALSE;

			if (pcsSkill->m_pcsBase && pcsSkill->m_pcsBase->m_Mutex.WLock())
			{
				pcsSkill->m_Mutex.Release();
				return FALSE;
			}

			// 받은 데이타를 업데이트 한다.
			AgsdSkill	*pcsAgsdSkill = GetADSkill(pcsSkill);
			if (pcsAgsdSkill)
			{
				if (pvPacketUpdateFactorResult && SetUpdateFactorResult(pcsAgsdSkill))
					m_pagpmFactors->ReflectPacket(pcsAgsdSkill->m_pcsUpdateFactorResult, pvPacketUpdateFactorResult, 0);
				if (pvPacketUpdateFactorPoint && SetUpdateFactorPoint(pcsAgsdSkill))
					m_pagpmFactors->ReflectPacket(pcsAgsdSkill->m_pcsUpdateFactorPoint, pvPacketUpdateFactorPoint, 0);
				if (pvPacketUpdateFactorPercent && SetUpdateFactorPercent(pcsAgsdSkill))
					m_pagpmFactors->ReflectPacket(pcsAgsdSkill->m_pcsUpdateFactorPercent, pvPacketUpdateFactorPercent, 0);

				/*
				if (pvPacketModifyFactorPoint)
				{
					for (int i = 0; i < AGPMSKILL_MAX_AFFECTED_BASE; ++i)
					{
						if (!pvPacketModifyFactorPoint[i])
							break;

						m_pagpmFactors->ReflectPacket(&pcsAgsdSkill->m_csModifyFactorPoint[i], pvPacketModifyFactorPoint[i], 0);
					}
				}
				if (pvPacketModifyFactorPercent)
				{
					for (int i = 0; i < AGPMSKILL_MAX_AFFECTED_BASE; ++i)
					{
						if (!pvPacketModifyFactorPercent[i])
							break;

						m_pagpmFactors->ReflectPacket(&pcsAgsdSkill->m_csModifyFactorPercent[i], pvPacketModifyFactorPercent[i], 0);
					}
				}
				*/

				if (lModifySkillLevel > 0)
					pcsAgsdSkill->m_lModifySkillLevel = lModifySkillLevel;
				if (lModifyCharLevel > 0)
					pcsAgsdSkill->m_lModifyCharLevel = lModifyCharLevel;
				if (lMeleeDefensePoint > 0)
					pcsAgsdSkill->m_nMeleeDefensePoint = lMeleeDefensePoint;
				if (lMeleeReflectPoint > 0)
					pcsAgsdSkill->m_nMeleeReflectPoint = lMeleeReflectPoint;
				if (lMagicDefensePoint > 0)
					pcsAgsdSkill->m_nMagicDefensePoint = lMagicDefensePoint;
				if (lMagicReflectPoint > 0)
					pcsAgsdSkill->m_nMagicReflectPoint = lMagicReflectPoint;
			}

			if (pcsSkill->m_pcsBase)
				pcsSkill->m_pcsBase->m_Mutex.Release();

			pcsSkill->m_Mutex.Release();
		}
		break;

	case AGSMSKILL_PACKET_OPERATION_UPDATE_ATTACH_BASE_DATA:
		{
			if (!pvPacketAttachBaseData)
				return FALSE;

//			PVOID	pvPacketBuffedSkillProcessInfo[AGPMSKILL_MAX_SKILL_BUFF];
//			ZeroMemory(pvPacketBuffedSkillProcessInfo, sizeof(PVOID) * AGPMSKILL_MAX_SKILL_BUFF);

			ApSafeArray<PVOID, AGPMSKILL_MAX_SKILL_BUFF>	pvPacketBuffedSkillProcessInfo;
			pvPacketBuffedSkillProcessInfo.MemSetAll();

			PVOID	pvPacketTargetBase					= NULL;
			INT32	lDamageAdjustRate					= (-1);

			m_csPacketAttachBaseData.GetField(FALSE, pvPacketAttachBaseData, 0, 
											  &pvPacketBuffedSkillProcessInfo[0],
											  &pvPacketTargetBase,
											  &lDamageAdjustRate);

			if (!pvPacketTargetBase)
				return FALSE;

//			if (!m_pagpmSkill->m_csAdminSkill.GlobalWLock())
//				return FALSE;

			ApBase *pcsBase = m_pagpmSkill->GetBaseFromPacket(pvPacketTargetBase);
			if (!pcsBase || !pcsBase->m_Mutex.WLock())
			{
//				m_pagpmSkill->m_csAdminSkill.GlobalRelease();
				return FALSE;
			}

//			if (!m_pagpmSkill->m_csAdminSkill.GlobalRelease())
//			{
//				pcsBase->m_Mutex.Release();
//				return FALSE;
//			}

			AgsdSkillADBase	*pcsADBase		= GetADBase(pcsBase);
			if (lDamageAdjustRate > (-1))
				pcsADBase->m_lDamageAdjustRate	= lDamageAdjustRate;

			for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
			{
				if (!pvPacketBuffedSkillProcessInfo[i])
					break;

				INT8	cBaseType						= (-1);
				INT32	lBaseID							= (-1);
				INT32	lMasteryPoint					= (-1);
				PVOID	pvPacketTempSkill				= NULL;

				m_csPacketBuffedSkillProcessInfo.GetField(FALSE, pvPacketBuffedSkillProcessInfo[i], 0,
														  &cBaseType,
														  &lBaseID,
														  &lMasteryPoint,
														  &pvPacketTempSkill);

				AgsdSkillADBase	*pcsSkillADBase			= GetADBase(pcsBase);

				if (cBaseType != (-1) && lBaseID != (-1) && pvPacketTempSkill)
				{
					pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_csSkillOwner.m_eType	= (ApBaseType) cBaseType;
					pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_csSkillOwner.m_lID		= lBaseID;

					pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_lMasteryPoint			= lMasteryPoint;


					INT32		lSkillTemplateID			= 0;
					INT32		lSkillLevel					= 0;
					PVOID		pvPacketModifyFactorPoint	= NULL;
					PVOID		pvPacketModifyFactorPercent	= NULL;

					AgsdSkill	*pcsAgsdSkill				= GetADSkill(pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill);

					UINT32		ulReceivedClockCount		= 0;
					UINT32		ulCurrentClockCount			= GetClockCount();

					m_csPacketTempSkillInfo.GetField(FALSE, pvPacketTempSkill, 0,
													 &pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_lID,
													 &lSkillTemplateID,
													 &lSkillLevel,
													 &pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulEndTime,
													 &pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulInterval,
													 &pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulNextProcessTime,
													 &pvPacketModifyFactorPoint,
													 &pvPacketModifyFactorPercent,
													 &pcsAgsdSkill->m_nMeleeDefensePoint,
													 &pcsAgsdSkill->m_nMeleeReflectPoint,
													 &pcsAgsdSkill->m_nMagicDefensePoint,
													 &pcsAgsdSkill->m_nMagicReflectPoint,
													 &ulReceivedClockCount);

					if (pvPacketModifyFactorPoint && SetUpdateFactorPoint(pcsAgsdSkill))
						m_pagpmFactors->ReflectPacket(pcsAgsdSkill->m_pcsUpdateFactorPoint, pvPacketModifyFactorPoint, 0);
					if (pvPacketModifyFactorPercent && SetUpdateFactorPercent(pcsAgsdSkill))
						m_pagpmFactors->ReflectPacket(pcsAgsdSkill->m_pcsUpdateFactorPercent, pvPacketModifyFactorPercent, 0);

					pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_pcsTemplate	= m_pagpmSkill->GetSkillTemplate(lSkillTemplateID);

					m_pagpmFactors->SetValue(&pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_csFactor, lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

					pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulEndTime = ulCurrentClockCount + (pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulEndTime - ulReceivedClockCount);
					pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulInterval = ulCurrentClockCount + (pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulInterval - ulReceivedClockCount);
					pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulNextProcessTime = ulCurrentClockCount + (pcsSkillADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill->m_ulNextProcessTime - ulReceivedClockCount);
				}
			}

			pcsBase->m_Mutex.Release();
		}
		break;

	case AGSMSKILL_PACKET_OPERATION_ADD_DB_DATA:
		{
			if (!pvPacketDBData)
				return FALSE;

			INT32	lCID	= AP_INVALID_CID;

			PVOID	pvPacketMasteryDBData	= NULL;

			m_csPacketDBData.GetField(FALSE, pvPacketDBData, 0,
									&lCID,
									&pvPacketMasteryDBData);

			if (!pvPacketMasteryDBData)
				return FALSE;

			AgpdCharacter	*pcsCharacter	= m_pagpmCharacter->GetCharacterLock(lCID);
			if (!pcsCharacter)
				return FALSE;

			CHAR	*pszSkillTree		= NULL;
			CHAR	szszSkillTree[AGSMSKILL_MAX_SKILLTREE_LENGTH+1];
			INT16	nSkillTreeLength	= 0;

			CHAR	*szSpecialize		= NULL;
			INT16	nSpecializeLength	= 0;
			
			CHAR	*pszProductCompose	= NULL;
			CHAR	szProductCompose[AGSMSKILL_MAX_SKILLTREE_LENGTH+1];
			INT16	nProductCompose		= 0;
			

			m_csPacketMasteryDBData.GetField(FALSE, pvPacketMasteryDBData, 0,
											&pszSkillTree, &nSkillTreeLength,
											&szSpecialize, &nSpecializeLength,
											&pszProductCompose, &nProductCompose);

			if (nSkillTreeLength > 0)
				{
				ZeroMemory(szszSkillTree, sizeof(szszSkillTree));
				strncpy(szszSkillTree, pszSkillTree, nSkillTreeLength);
				EnumCallback(AGSMSKILL_DB_PARSE_SKILL_TREE_STIRNG, pcsCharacter, szszSkillTree);
				}
			
			if (nProductCompose > 0)
				{
				ZeroMemory(szProductCompose, sizeof(szProductCompose));
				strncpy(szProductCompose, pszProductCompose, min(nProductCompose, AGSMSKILL_MAX_SKILLTREE_LENGTH));
				EnumCallback(AGSMSKILL_DB_PARSE_SKILL_COMPOSE_STIRNG, pcsCharacter, szProductCompose);
				}

			//DecodingMasterySpecialize((ApBase *) pcsCharacter, (INT32) cMasteryIndex, szSpecialize);

			//m_pagpmCharacter->UpdateSkillPoint(pcsCharacter, -lTotalInputSP);

			pcsCharacter->m_Mutex.Release();
		}
		break;
	case AGSMSKILL_PACKET_OPERATION_EQIP_LIST:
		{
			PACKET_AGSMSKILL_EQUIPLIST* pPacket = (PACKET_AGSMSKILL_EQUIPLIST*)pvPacket;

			AgpdCharacter	*pcsCharacter	= m_pagpmCharacter->GetCharacterLock(pPacket->lCID);
			if (!pcsCharacter)
				return FALSE;

			AgsdSkillADBase	*pcsAgsdSkillADBase	= GetADBase((ApBase *)pcsCharacter);
			if(!pcsAgsdSkillADBase)
				return FALSE;

			if(!pcsAgsdSkillADBase->m_csEquipSkillArray.empty())
				pcsAgsdSkillADBase->m_csEquipSkillArray.clear();

			for(INT32 i = 0; i < pPacket->nSkillCount; ++i)
			{
				pcsAgsdSkillADBase->m_csEquipSkillArray.push_back(pPacket->m_csEquipSkillArray[i]);
			}
		} break;
	default:
		return FALSE;
		break;
	}


	return TRUE;
}

BOOL AgsmSkill::FreeSkillEffect(AgpdSkill *pcsSkill, ApBase *pcsBase)
{
	if (!pcsSkill || !pcsBase)
		return FALSE;

	/*
	for (int i = 0; i < AGPMSKILL_MAX_AFFECTED_BASE; ++i)
	{
		if (pcsSkill->m_acsAffectedBase[i].m_eType	== pcsBase->m_eType &&
			pcsSkill->m_acsAffectedBase[i].m_lID	== pcsBase->m_lID)
			break;
	}

	if (i == AGPMSKILL_MAX_AFFECTED_BASE)
		return FALSE;

	EndSkillEffect(pcsSkill, pcsBase);

	RestoreSkillFactor(pcsSkill, pcsBase, i);

	AgpdSkillAttachData	*pcsAttachData = m_pagpmSkill->GetAttachSkillData(pcsBase);
	if (pcsAttachData)
	{
		INT32	lBuffedListIndex	= m_pagpmSkill->RemoveBuffedList(pcsAttachData->m_astBuffSkillList, pcsSkill->m_pcsTemplate->m_lID);
	}

	// 버프 없어진다고 알려준다.
	SendUpdateBuffedList(pcsBase, pcsSkill->m_pcsTemplate->m_lID, FALSE, GetBaseDPNID(pcsBase));

	pcsSkill->m_acsAffectedBase[i].m_lID		= 0;
	*/

	return TRUE;
}

BOOL AgsmSkill::FreeSkillEffect(AgpdSkill *pcsSkill, ApBase *pcsBase, INT32 lBuffedListIndex)
{
	PROFILE("AgsmSkill::FreeSkillEffect");

	if (!pcsSkill ||
		!pcsBase ||
		lBuffedListIndex < 0 ||
		lBuffedListIndex >= AGPMSKILL_MAX_SKILL_BUFF)
		return FALSE;

	pstAgpmSkillBuffList	pcsBuffedList = m_pagpmSkill->GetBuffedList(pcsBase);
	if (!pcsBuffedList)
		return FALSE;

	// 2004.09.23. steeple.
	// EndSkillEffect 에서 불러주는 건데, EndSkillEffect 를 아무데서도 안부르길래 여기로 옮겨옴.
	// pcsSkill->m_pcsTemplate 이 NULL 일 수도 있다. 왜지 -_-?
	if(pcsSkill->m_pcsTemplate)
	{
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEndEffectType & AGPMSKILL_ENDSKILL_CONSUME_HP)
			EndSkillEffectConsumeHP(pcsSkill, pcsBase);
	}

	if (pcsBuffedList[lBuffedListIndex].lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	if (!RestoreOriginalStatus(pcsSkill, pcsBase))
		return FALSE;

	if (!RestoreSkillFactor(pcsSkill, pcsBase))
		return FALSE;

	// 버프 없어진다고 알려준다.
	return SendUpdateBuffedList(pcsBase, pcsBuffedList[lBuffedListIndex].lSkillTID, 0, FALSE, 0, 0, GetBaseDPNID(pcsBase));
}

INT32 AgsmSkill::GetSpellCount(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate)
		return 0;

	AgpdSkillTemplate	*pcsTemplate	= (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

	if (pcsTemplate->m_lEffectType[0] | AGPMSKILL_EFFECT_ADD_SPELLCOUNT)
	{
		if (pcsTemplate->m_lEffectType[0] | AGPMSKILL_EFFECT_ADD_SPELLCOUNT_MASTERY_POINT)
		{
			return GetModifiedSkillLevel(pcsSkill);
		}
	}

	return 0;
}

INT32 AgsmSkill::AddSpellCount(AgpdSkill *pcsSkill, ApBase *pcsBase, INT32 lSpellCount)
{
	if (!pcsSkill || !pcsBase || lSpellCount)
		return 0;

	pstAgpmSkillBuffList	pcsBuffList = m_pagpmSkill->GetBuffedList(pcsBase);
	if (!pcsBuffList)
		return 0;

	int i = 0;
	for (i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (pcsBuffList[i].lSkillID == pcsSkill->m_lID)
			break;
	}

	if (i == AGPMSKILL_MAX_SKILL_BUFF)
		return 0;

	pcsBuffList[i].lSpellCount += lSpellCount;

	return pcsBuffList[i].lSpellCount;
}

INT32 AgsmSkill::ReduceSpellCount(ApBase *pcsBase, INT32 lSpellCount, INT32 *plDispelSkillIndex, AgpdSkill *pcsSkill)
{
	if (!pcsBase || !plDispelSkillIndex)
		return 0;

	pstAgpmSkillBuffList	pcsBuffList = m_pagpmSkill->GetBuffedList(pcsBase);
	if (!pcsBuffList)
		return 0;

	int	i = 0;

	if (pcsSkill)
	{
		for (i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
		{
			if (pcsBuffList[i].lSkillID == pcsSkill->m_lID)
				break;
		}

		if (i == AGPMSKILL_MAX_SKILL_BUFF)
			return 0;
	}
	else
	{
		for (i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
		{
			if (pcsBuffList[i].lSpellCount > 0)
				break;
		}

		if (i == AGPMSKILL_MAX_SKILL_BUFF)
			return 0;
	}

	if (pcsBuffList[i].lSpellCount <= 0)
		return 0;

	pcsBuffList[i].lSpellCount -= lSpellCount;
	if (pcsBuffList[i].lSpellCount <= 0)
	{
		pcsBuffList[i].lSpellCount = 0;

		// spell count가 0이 되었다. 이 스킬의 효과가 다되었다.
		*plDispelSkillIndex	= i;
	}

	return pcsBuffList[i].lSpellCount;
}

BOOL AgsmSkill::SetSkillProcessInfo(AgpdSkill *pcsSkill, AgsdSkillProcessInfo *pcsSkillProcessInfo)
{
	if (!pcsSkill || !pcsSkillProcessInfo /*|| !pcsSkillProcessInfo->m_pcsTempSkill*/)
		return FALSE;

	AgsdSkill	*pcsAgsdSkill									= GetADSkill(pcsSkill);
	if(!pcsAgsdSkill)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	if(!pcsSkillProcessInfo->m_pcsTempSkill)
	{
		AgpdSkill* pcsSkil = m_pagpmSkill->CreateSkill();
		if(!pcsSkill)
			return FALSE;

		pcsSkillProcessInfo->m_pcsTempSkill = pcsSkil;
	}

	pcsSkillProcessInfo->m_csSkillOwner.m_eType					= pcsSkill->m_pcsBase->m_eType;
	pcsSkillProcessInfo->m_csSkillOwner.m_lID					= pcsSkill->m_pcsBase->m_lID;

	pcsSkillProcessInfo->m_lMasteryPoint						= lSkillLevel;

	/*
	pcsSkillProcessInfo->m_lSkillID								= pcsSkill->m_lID;
	pcsSkillProcessInfo->m_pcsTemplate							= (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

	pcsSkillProcessInfo->m_lSkillLevel							= m_pagpmSkill->GetSkillLevel(pcsSkill);

	pcsSkillProcessInfo->m_tmEnd								= pcsSkill->m_ulEndTime;
	pcsSkillProcessInfo->m_tmInterval							= pcsSkill->m_ulInterval;
	pcsSkillProcessInfo->m_tmNextProcess						= pcsSkill->m_ulNextProcessTime;

	m_pagpmFactors->CopyFactor(&pcsSkillProcessInfo->m_csModifyFactorPoint, &pcsAgsdSkill->m_csUpdateFactorPoint, TRUE);
	m_pagpmFactors->CopyFactor(&pcsSkillProcessInfo->m_csModifyFactorPercent, &pcsAgsdSkill->m_csUpdateFactorPercent, TRUE);
	*/


	pcsSkillProcessInfo->m_pcsTempSkill->m_lID					= pcsSkill->m_lID;
	pcsSkillProcessInfo->m_pcsTempSkill->m_eType				= pcsSkill->m_eType;

	// 이거 관리가 넘 안되서 걍 뺐다.
//	pcsSkillProcessInfo->m_pcsTempSkill->m_pcsBase				= (ApBase *) &pcsSkillProcessInfo->m_csSkillOwner;
	pcsSkillProcessInfo->m_pcsTempSkill->m_csBase				= pcsSkillProcessInfo->m_csSkillOwner;

	pcsSkillProcessInfo->m_pcsTempSkill->m_pcsTemplate			= pcsSkill->m_pcsTemplate;

	pcsSkillProcessInfo->m_pcsTempSkill->m_ulEndTime			= pcsSkill->m_ulEndTime;
	pcsSkillProcessInfo->m_pcsTempSkill->m_ulInterval			= pcsSkill->m_ulInterval;
	pcsSkillProcessInfo->m_pcsTempSkill->m_ulNextProcessTime	= pcsSkill->m_ulNextProcessTime;
	pcsSkillProcessInfo->m_pcsTempSkill->m_lMasteryIndex		= pcsSkill->m_lMasteryIndex;

	pcsSkillProcessInfo->m_pcsTempSkill->m_bCloneObject			= TRUE;

	m_pagpmFactors->SetValue(&pcsSkillProcessInfo->m_pcsTempSkill->m_csFactor, lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

	AgsdSkill	*pcsTempAgsdSkill								= GetADSkill(pcsSkillProcessInfo->m_pcsTempSkill);

	if (pcsAgsdSkill->m_pcsUpdateFactorPoint && SetUpdateFactorPoint(pcsTempAgsdSkill))
		m_pagpmFactors->CopyFactor(pcsTempAgsdSkill->m_pcsUpdateFactorPoint, pcsAgsdSkill->m_pcsUpdateFactorPoint, TRUE);
	if (pcsAgsdSkill->m_pcsUpdateFactorPercent && SetUpdateFactorPercent(pcsTempAgsdSkill))
		m_pagpmFactors->CopyFactor(pcsTempAgsdSkill->m_pcsUpdateFactorPercent, pcsAgsdSkill->m_pcsUpdateFactorPercent, TRUE);

	if (pcsAgsdSkill->m_pcsUpdateFactorItemPoint && SetUpdateFactorItemPoint(pcsTempAgsdSkill))
		m_pagpmFactors->CopyFactor(pcsTempAgsdSkill->m_pcsUpdateFactorItemPoint, pcsAgsdSkill->m_pcsUpdateFactorItemPoint, TRUE);
	if (pcsAgsdSkill->m_pcsUpdateFactorItemPercent && SetUpdateFactorItemPercent(pcsTempAgsdSkill))
		m_pagpmFactors->CopyFactor(pcsTempAgsdSkill->m_pcsUpdateFactorItemPercent, pcsAgsdSkill->m_pcsUpdateFactorItemPercent, TRUE);

	pcsTempAgsdSkill->m_nMagicDefensePoint						= pcsAgsdSkill->m_nMagicDefensePoint;
	pcsTempAgsdSkill->m_nMagicReflectPoint						= pcsAgsdSkill->m_nMagicReflectPoint;
	pcsTempAgsdSkill->m_nMeleeDefensePoint						= pcsAgsdSkill->m_nMeleeDefensePoint;
	pcsTempAgsdSkill->m_nMeleeReflectPoint						= pcsAgsdSkill->m_nMeleeReflectPoint;

	pcsTempAgsdSkill->m_stBuffedSkillCombatEffectArg			= pcsAgsdSkill->m_stBuffedSkillCombatEffectArg;
	pcsTempAgsdSkill->m_stBuffedSkillFactorEffectArg			= pcsAgsdSkill->m_stBuffedSkillFactorEffectArg;

	pcsTempAgsdSkill->m_lSkillScrollIID							= pcsAgsdSkill->m_lSkillScrollIID;	// 2006.01.05. steeple

	pcsTempAgsdSkill->m_nAffectedDOTCount						= pcsAgsdSkill->m_nAffectedDOTCount;
	pcsTempAgsdSkill->m_nTotalDOTCount							= pcsAgsdSkill->m_nTotalDOTCount;

	pcsTempAgsdSkill->m_stInvincibleInfo						= pcsAgsdSkill->m_stInvincibleInfo;

	pcsTempAgsdSkill->m_stConnectionInfo						= pcsAgsdSkill->m_stConnectionInfo;
	pcsTempAgsdSkill->m_stUnionInfo								= pcsAgsdSkill->m_stUnionInfo;

	pcsTempAgsdSkill->m_alLevelUpSkillTID.MemCopy(0, pcsAgsdSkill->m_alLevelUpSkillTID.begin(), AGPMSKILL_MAX_SKILL_LEVELUP_TID);

	return TRUE;
}

//		UseSkill
//	Functions
//		- 스킬을 사용한다.
//			pcsOwner란 넘이 pcsTarget을 대상으로 이름이 szSkillName인 스킬을 사용한다.
//	Arguments
//		- szSkillName	: 사용할 스킬 템플릿 이름
//		- pcsOwner		: 스킬을 사용하는 넘 포인터
//		- pcsTarget		: 스킬의 대상이 되는 넘 (스킬의 대상이 자기 자신인 경우 이값은 무시된다.)
//	Return value
//		- BOOL : 스킬 사용 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmSkill::UseSkill(CHAR *szSkillName, ApBase *pcsOwner, ApBase *pcsTarget)
{
	if (!szSkillName || !szSkillName[0] || !pcsOwner)
		return FALSE;

	return TRUE;
}

BOOL AgsmSkill::SendCastResult(AgpdSkill *pcsSkill, ApBase *pcsBase, ApBase *pcsTarget, PVOID pvPacketFactor, eAgpmSkillActionType eResultType, BOOL bIsFactorNotQueueing, BOOL bIsSyncHP, UINT32 ulAdditionalEffect)
{
	if (!pcsBase || !pcsTarget)
		return FALSE;

	INT16	nPacketLength	= 0;

	PVOID	pvPacket		= m_pagpmSkill->MakeCastResultPacket(eResultType, &pcsSkill->m_lID, &pcsSkill->m_pcsTemplate->m_lID, pcsBase, GetModifiedSkillLevel(pcsSkill), pcsTarget, pvPacketFactor, &nPacketLength, bIsFactorNotQueueing, ulAdditionalEffect);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	UINT32	ulAttackDPNID	= 0;
	UINT32	ulTargetDPNID	= 0;

	// pcsBase와 pcsTarget에 결과를 보낸다.
	if (pcsBase->m_eType == APBASE_TYPE_CHARACTER &&
		pcsBase->m_lID != pcsTarget->m_lID)
	{
//		if (pcsSkill->m_bCloneObject)
//		{
//			ulAttackDPNID	= m_pagsmCharacter->GetCharDPNID(m_pagpmCharacter->GetCharacter(pcsBase->m_lID));
//		}
//		else
		{
			ulAttackDPNID	= m_pagsmCharacter->GetCharDPNID((AgpdCharacter *) pcsBase);
		}

		if (ulAttackDPNID != 0)
			SendPacket(pvPacket, nPacketLength, ulAttackDPNID);
	}

	if (pcsTarget->m_eType == APBASE_TYPE_CHARACTER)
	{
		ulTargetDPNID	= m_pagsmCharacter->GetCharDPNID((AgpdCharacter *) pcsTarget);

		if (ulTargetDPNID != 0)
			SendPacket(pvPacket, nPacketLength, ulTargetDPNID);
	}

	m_pagpmSkill->m_csPacket.FreePacket(pvPacket);

	if (bIsSyncHP)
	{
		PVOID	pvPacketHP	= m_pagpmFactors->MakePacketFactorsCharHP(&((AgpdCharacter *) pcsTarget)->m_csFactor);

		nPacketLength		= 0;
		PVOID	pvPacket	= m_pagsmCharacter->MakePacketFactor((AgpdCharacter *) pcsTarget, pvPacketHP, &nPacketLength);

		if (pvPacketHP)
			m_pagpmFactors->m_csPacket.FreePacket(pvPacketHP);

		if (pvPacket)
		{
			UINT32	ulExceptNIDs[2];
			INT32	lNumNID	= 0;

			if (ulAttackDPNID != 0)
				ulExceptNIDs[lNumNID++]	= ulAttackDPNID;
			if (ulTargetDPNID != 0)
				ulExceptNIDs[lNumNID++]	= ulTargetDPNID;

			m_pagpmSkill->m_csPacket.SetCID(pvPacket, nPacketLength, pcsBase->m_lID);

			m_pagsmAOIFilter->SendPacketNearExceptNIDs(pvPacket, nPacketLength, ((AgpdCharacter *) pcsTarget)->m_stPos,
														m_pagpmCharacter->GetRealRegionIndex((AgpdCharacter*)pcsTarget),
														ulExceptNIDs, lNumNID, PACKET_PRIORITY_5);

			m_pagsmCharacter->m_csPacket.FreePacket(pvPacket);
		}
	}

	/*
	// 주변 넘들한테 보낸다.
	if (pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, ((AgpdCharacter *) pcsBase)->m_stPos);
	}
	*/

	return TRUE;
}

// 2005.07.07. steeple
// Additional Effect 를 보낸다.
BOOL AgsmSkill::SendAdditionalEffect(ApBase* pcsBase, INT32 lEffectID, INT32 lTargetCID, UINT32 ulNID)
{
	if(!pcsBase)
		return FALSE;

	// 일단 캐릭터일때만 처리하자.
	if(pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return FALSE;

	INT16 nPacketLength = 0;
	PVOID pvPacket = m_pagpmSkill->MakePacketAdditionalEffect(&lEffectID, pcsBase, &lTargetCID, &nPacketLength);
	if(!pvPacket)
		return FALSE;

	m_pagpmSkill->m_csPacket.SetCID(pvPacket, nPacketLength, pcsBase->m_lID);
	BOOL bResult = FALSE;
	if(ulNID != 0)
		bResult = SendPacket(pvPacket, nPacketLength, ulNID);
	else
		bResult = m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, ((AgpdCharacter*)pcsBase)->m_stPos, PACKET_PRIORITY_5);

	m_pagpmSkill->m_csPacket.FreePacket(pvPacket);

	return bResult;
}

// 2007.02.06. steeple
// Modified Skill Level
BOOL AgsmSkill::SendModifiedSkillLevel(ApBase* pcsBase)
{
	if(!pcsBase)
		return FALSE;

	INT16 nPacketLength = 0;
	PVOID pvPacket = m_pagpmSkill->MakePacketModifiedSkillLevel(pcsBase, &nPacketLength);
	if(!pvPacket || nPacketLength < 1)
		return FALSE;

	m_pagpmSkill->m_csPacket.SetCID(pvPacket, nPacketLength, pcsBase->m_lID);
	BOOL bResult = SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsBase));

	m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
	return bResult;
}

// 2007.07.20. steeple
// Send Init CoolTime
BOOL AgsmSkill::SendInitCoolTime(ApBase* pcsBase, INT32 lSkillTID)
{
	if(!pcsBase)
		return FALSE;

	INT16 nPacketLength = 0;
	PVOID pvPacket = m_pagpmSkill->MakePacketInitCoolTime(pcsBase, &nPacketLength, lSkillTID);
	if(!pvPacket || nPacketLength < 1)
		return FALSE;

	m_pagpmSkill->m_csPacket.SetCID(pvPacket, nPacketLength, pcsBase->m_lID);
	BOOL bResult = SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsBase));

	m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
	return bResult;
}

// 2007.11.14. steeple
// Send Specific Skill Level
BOOL AgsmSkill::SendSpecificSkillLevel(AgpdSkill* pcsSkill)
{
	if(!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	INT16 nPacketLength = 0;
	PVOID pvPacket = m_pagpmSkill->MakePacketSpecificSkillLevel(pcsSkill, &nPacketLength);
	if(!pvPacket || nPacketLength < 1)
		return FALSE;

	m_pagpmSkill->m_csPacket.SetCID(pvPacket, nPacketLength, pcsSkill->m_pcsBase->m_lID);
	BOOL bResult = SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID((AgpdCharacter*)pcsSkill->m_pcsBase));

	m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
	return bResult;
}

//BOOL AgsmSkill::SendMasteryRollbackResult(ApBase *pcsBase, BOOL bResult, UINT32 ulNID)
//{
//	if (!pcsBase)
//		return FALSE;
//
//	eAgpmSkillPacketOperation	eResult;
//
//	if (bResult)
//		eResult	= AGPMSKILL_PACKET_OPERATION_MASTERY_ROLLBACK_SUCCESS;
//	else
//		eResult	= AGPMSKILL_PACKET_OPERATION_MASTERY_ROLLBACK_FAIL;
//
//	INT16	nPacketLength	= 0;
//	PVOID	pvPacket		= m_pagpmSkill->MakePacketMasteryRollbackResult(pcsBase, &nPacketLength, eResult);
//
//	if (!pvPacket || nPacketLength < 1)
//		return FALSE;
//
//	BOOL	bSendResult	= SendPacket(pvPacket, nPacketLength, ulNID);
//
//	m_pagpmSkill->m_csPacket.FreePacket(pvPacket);
//
//	return bSendResult;
//}

BOOL AgsmSkill::CBUpdateAllToDB(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmSkill		*pThis			= (AgsmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBUpdateAllToDB"));

//	for (int i = 0; i < AGPMSKILL_MAX_MASTERY; ++i)
//	{
//		pThis->SendRelayUpdate((ApBase *) pcsCharacter, i);
//	}

	pThis->SendRelayUpdate((ApBase *) pcsCharacter);

	return TRUE;
}

//BOOL AgsmSkill::SendRelayUpdate(ApBase *pcsBase, INT32 lMasteryIndex)
//{
//	if (!pcsBase ||
//		lMasteryIndex < 0 ||
//		lMasteryIndex >= AGPMSKILL_MAX_MASTERY)
//		return FALSE;
//
//	return EnumCallback(AGSMSKILL_CB_UPDATE_SKILL_MASTERY_TO_DB, pcsBase, (PVOID) lMasteryIndex);		
//}
//
//BOOL AgsmSkill::SendRelayInsert(ApBase *pcsBase, INT32 lMasteryIndex)
//{
//	if (!pcsBase ||
//		lMasteryIndex < 0 ||
//		lMasteryIndex >= AGPMSKILL_MAX_MASTERY)
//		return FALSE;
//
//	return EnumCallback(AGSMSKILL_CB_INSERT_SKILL_MASTERY_TO_DB, pcsBase, (PVOID) lMasteryIndex);		
//}
//
//BOOL AgsmSkill::SendRelayDelete(ApBase *pcsBase, INT32 lMasteryIndex)
//{
//	if (!pcsBase ||
//		lMasteryIndex < 0 ||
//		lMasteryIndex >= AGPMSKILL_MAX_MASTERY)
//		return FALSE;
//
//	return EnumCallback(AGSMSKILL_CB_DELETE_SKILL_MASTERY_TO_DB, pcsBase, (PVOID) lMasteryIndex);		
//}

BOOL AgsmSkill::SendRelayUpdate(ApBase *pcsBase)
{
	if (!pcsBase)
		return FALSE;

	return EnumCallback(AGSMSKILL_CB_UPDATE_SKILL_MASTERY_TO_DB, pcsBase, NULL);		
}

BOOL AgsmSkill::SendRelayInsert(ApBase *pcsBase)
{
	if (!pcsBase)
		return FALSE;

	return EnumCallback(AGSMSKILL_CB_INSERT_SKILL_MASTERY_TO_DB, pcsBase, NULL);		
}

BOOL AgsmSkill::SendRelayDelete(ApBase *pcsBase)
{
	if (!pcsBase)
		return FALSE;

	return EnumCallback(AGSMSKILL_CB_DELETE_SKILL_MASTERY_TO_DB, pcsBase, NULL);
}

BOOL AgsmSkill::SetParamInsertQuery5(AuDatabase2 *pDatabase, AgpdCharacter *pcsCharacter, CHAR *pstrSkillSeq)
{
	if (!pDatabase || !pcsCharacter)
		return FALSE;

	pDatabase->SetParam(0, m_pagsmCharacter->GetRealCharName(pcsCharacter->m_szID), sizeof(pcsCharacter->m_szID));
	pDatabase->SetParam(1, (pstrSkillSeq && pstrSkillSeq[0]) ? pstrSkillSeq : _T(""),
						 (pstrSkillSeq && pstrSkillSeq[0]) ? sizeof(CHAR) * ((INT32)strlen(pstrSkillSeq)+1) : sizeof(CHAR));
	pDatabase->SetParam(2, _T(""), sizeof(CHAR));
	
	if (AUDATABASE2_QR_SUCCESS == pDatabase->ExecuteQuery())
		return TRUE;

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// Log - 2004.05.06. steeple
/*
BOOL AgsmSkill::WriteSkillUseLog(AgpdSkill* pcsSkill)
{
	if(!m_pagpmLog)
		return TRUE;
	
	if(!pcsSkill ||!pcsSkill->m_pcsBase || !pcsSkill->m_pcsTemplate)
		return FALSE;

	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pcsSkill->m_pcsBase;
	if(!pcsCharacter || !m_pagpmCharacter->IsPC(pcsCharacter))
		return FALSE;

	INT32 lLevel = 0;
	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);
	
	return m_pagpmLog->WriteLog_SkillUse(pcsCharacter->m_szID,
																	((AgpdCharacterTemplate*)pcsCharacter->m_pcsCharacterTemplate)->m_lID,
																	lLevel,
																	((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lID,
																	1);
}
*/

BOOL AgsmSkill::SetUpdateFactorPoint(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorPoint)
		return TRUE;

	pcsAgsdSkill->m_pcsUpdateFactorPoint	= m_pagpmFactors->AllocAgpdFactor(m_lPointFactorTypeIndex);

	if (!pcsAgsdSkill->m_pcsUpdateFactorPoint)
		return FALSE;

	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorPoint);

	pcsAgsdSkill->m_pcsUpdateFactorPoint->m_bPoint	= TRUE;

	return TRUE;
}

BOOL AgsmSkill::SetUpdateFactorPercent(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorPercent)
		return TRUE;

	pcsAgsdSkill->m_pcsUpdateFactorPercent	= m_pagpmFactors->AllocAgpdFactor(m_lPercentFactorTypeIndex);

	if (!pcsAgsdSkill->m_pcsUpdateFactorPercent)
		return FALSE;

	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorPercent);

	pcsAgsdSkill->m_pcsUpdateFactorPercent->m_bPoint	= FALSE;

	return TRUE;
}

BOOL AgsmSkill::SetUpdateFactorResult(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorResult)
		return TRUE;

	pcsAgsdSkill->m_pcsUpdateFactorResult	= m_pagpmFactors->AllocAgpdFactor(m_lResultFactorTypeIndex);

	if (!pcsAgsdSkill->m_pcsUpdateFactorResult)
		return FALSE;

	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorResult);

	return TRUE;
}

BOOL AgsmSkill::SetUpdateFactorItemPoint(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorItemPoint)
		return TRUE;

	pcsAgsdSkill->m_pcsUpdateFactorItemPoint	= m_pagpmFactors->AllocAgpdFactor(m_lItemPointFactorTypeIndex);

	if (!pcsAgsdSkill->m_pcsUpdateFactorItemPoint)
		return FALSE;

	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPoint);

	pcsAgsdSkill->m_pcsUpdateFactorItemPoint->m_bPoint	= TRUE;

	return TRUE;
}

BOOL AgsmSkill::SetUpdateFactorItemPercent(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorItemPercent)
		return TRUE;

	pcsAgsdSkill->m_pcsUpdateFactorItemPercent	= m_pagpmFactors->AllocAgpdFactor(m_lItemPercentFactorTypeIndex);

	if (!pcsAgsdSkill->m_pcsUpdateFactorItemPercent)
		return FALSE;

	m_pagpmFactors->InitFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPercent);

	pcsAgsdSkill->m_pcsUpdateFactorItemPercent->m_bPoint	= FALSE;

	return TRUE;
}

BOOL AgsmSkill::DestroyUpdateFactorPoint(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorPoint)
	{
		m_pagpmFactors->DestroyFactor(pcsAgsdSkill->m_pcsUpdateFactorPoint);

		m_pagpmFactors->FreeAgpdFactor(pcsAgsdSkill->m_pcsUpdateFactorPoint, m_lPointFactorTypeIndex);

		pcsAgsdSkill->m_pcsUpdateFactorPoint	= NULL;
	}

	return TRUE;
}

BOOL AgsmSkill::DestroyUpdateFactorPercent(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorPercent)
	{
		m_pagpmFactors->DestroyFactor(pcsAgsdSkill->m_pcsUpdateFactorPercent);

		m_pagpmFactors->FreeAgpdFactor(pcsAgsdSkill->m_pcsUpdateFactorPercent, m_lPercentFactorTypeIndex);

		pcsAgsdSkill->m_pcsUpdateFactorPercent	= NULL;
	}

	return TRUE;
}

BOOL AgsmSkill::DestroyUpdateFactorResult(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorResult)
	{
		m_pagpmFactors->DestroyFactor(pcsAgsdSkill->m_pcsUpdateFactorResult);

		m_pagpmFactors->FreeAgpdFactor(pcsAgsdSkill->m_pcsUpdateFactorResult, m_lResultFactorTypeIndex);

		pcsAgsdSkill->m_pcsUpdateFactorResult	= NULL;
	}

	return TRUE;
}

BOOL AgsmSkill::DestroyUpdateFactorItemPoint(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorItemPoint)
	{
		m_pagpmFactors->DestroyFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPoint);

		m_pagpmFactors->FreeAgpdFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPoint, m_lItemPointFactorTypeIndex);

		pcsAgsdSkill->m_pcsUpdateFactorItemPoint	= NULL;
	}

	return TRUE;
}

BOOL AgsmSkill::DestroyUpdateFactorItemPercent(AgsdSkill *pcsAgsdSkill)
{
	if (!pcsAgsdSkill)
		return FALSE;

	if (pcsAgsdSkill->m_pcsUpdateFactorItemPercent)
	{
		m_pagpmFactors->DestroyFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPercent);

		m_pagpmFactors->FreeAgpdFactor(pcsAgsdSkill->m_pcsUpdateFactorItemPercent, m_lItemPercentFactorTypeIndex);

		pcsAgsdSkill->m_pcsUpdateFactorItemPercent	= NULL;
	}

	return TRUE;
}

// 2007.11.14. steeple
// 투명이 풀렸다고 세팅해 놓는다.
BOOL AgsmSkill::SetReleaseTransparent(AgpdSkill* pcsSkill)
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_pcsBase)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsSkill->m_pcsBase);
	if(!pcsADBase)
		return FALSE;

	pcsADBase->m_ulLastReleasedTransparentClock = GetClockCount();
	pcsADBase->m_lReleaseTransparentSkillTID = pcsSkill->m_pcsTemplate->m_lID;

	return TRUE;
}

// 2007.11.14. steeple
// 투명이 해당 스킬 때문에 풀렸는 지 체크한다. 시간 체크도 해서 3초가 넘었다면 아닌 것으로 간주한다.
BOOL AgsmSkill::IsReleaseTransparentSkillTID(ApBase* pcsBase, INT32 lSkillTID)
{
	if(!pcsBase || lSkillTID < 1)
		return FALSE;

	AgsdSkillADBase* pcsADBase = GetADBase(pcsBase);
	if(!pcsADBase)
		return FALSE;

	if(lSkillTID == pcsADBase->m_lReleaseTransparentSkillTID)
	{
		UINT32 ulClockCount = GetClockCount();
		if(ulClockCount - pcsADBase->m_ulLastReleasedTransparentClock <= 3000)
			return TRUE;
	}

	return FALSE;
}

// 2008.01.28. steeple
// PC Bang Item 스킬인지 체크
double AgsmSkill::GetPCBangItemSkillBonusRate(AgpdSkill* pcsSkill)
{
	if(!m_pagpmBillInfo || m_pagpmBillInfo->IsPCBang((AgpdCharacter*)pcsSkill->m_pcsBase) == FALSE)
		return 0;

	if(!pcsSkill)
		return 0;

	AgsdSkill* pcsAgsdSkill = GetADSkill(pcsSkill);
	if(!pcsAgsdSkill || !pcsAgsdSkill->m_lSkillScrollIID)
		return 0;

	AgpdItem* pcsItem = m_pagpmItem->GetItem(pcsAgsdSkill->m_lSkillScrollIID);
	if(!pcsItem || !pcsItem->m_pcsItemTemplate)
		return 0;

	if(pcsItem->m_pcsItemTemplate->m_lIsUseOnlyPCBang & AGPMITEM_PCBANG_TYPE_DOUBLE_EFFECT)
		return 2;

	return 0;
}

void AgsmSkill::RecastSaveSkill(AgpdCharacter* pcsCharacter)
{
	//////////////////////////////////////////////////////////////////////////
	// save skill - arycoat 2008.7.
	if(!pcsCharacter) return;
	ApBase* pcsBase = (ApBase*)pcsCharacter;
	ApBase* pcsTargetBase = (ApBase*)pcsCharacter;
	AgsdSkillADBase	*pcsAgsdSkillADBase	= GetADBase(pcsBase);
	if(!pcsAgsdSkillADBase) return;
	if(pcsAgsdSkillADBase->m_csEquipSkillArray.empty()) return;

	CEquipSkillArray EquipSkillArrayBack = pcsAgsdSkillADBase->m_csEquipSkillArray;

	for(CEquipSkillArray::iterator it = EquipSkillArrayBack.begin(); it != EquipSkillArrayBack.end(); ++it)
	{
		INT32 lSkillTID = it->SkillTID;
		AgpdSkillTemplate	*pcsSkillTemplate	= m_pagpmSkill->GetSkillTemplate(lSkillTID);
		if (!pcsSkillTemplate) continue;

		AgsdCastSkill pCastSkill;
		pCastSkill.pcsBase = pcsBase;
		pCastSkill.pcsTargetBase = pcsBase;
		pCastSkill.lSkillTID = lSkillTID;
		pCastSkill.lSkillLevel = 1;

		pCastSkill.m_pSaveSkillData.eStep	= SAVESKILL_RECASTING;
		pCastSkill.m_pSaveSkillData.m_pSaveData = (*it);

		if(it->Attribute & AGPMSKILL_RANGE2_TARGET_PARTY)
		{
			AgpdParty* pcsParty = m_pagpmParty->GetParty(pcsCharacter);

			if(!pcsParty)
			{
				pCastSkill.m_pSaveSkillData.m_pSaveData.ExpireDate = 0;
				pCastSkill.m_pSaveSkillData.m_pSaveData.RemainTime = 0;
			}
		}

		CastSkill(pCastSkill);
	}
}

void AgsmSkill::RemoveSaveSkill(AgpdCharacter* pcsCharacter, UINT64 Attribute)
{
	if(!pcsCharacter || !Attribute)
		return;

	AgsdSkillADBase	*pcsAgsdSkillADBase	= GetADBase(pcsCharacter);
	if(!pcsAgsdSkillADBase)
		return;

	if(pcsAgsdSkillADBase->m_csEquipSkillArray.empty())
		return;

	for(CEquipSkillArray::iterator it = pcsAgsdSkillADBase->m_csEquipSkillArray.begin(); it != pcsAgsdSkillADBase->m_csEquipSkillArray.end(); ++it)
	{
		if(!(it->Attribute & Attribute)) continue;
	
		it->ExpireDate = 0;
		it->RemainTime = 0;

		AgpdSkill* pcsSkill = FindBuffedSkill(pcsCharacter, it->SkillTID);
		if(!pcsSkill) continue;

		pcsSkill->m_ulEndTime = 0;
	}
}

BOOL AgsmSkill::ProcessSkillSave(AgpdSkill* pcsSkill, AgpdCharacter* pcsTarget, INT16 Operation)
{
	if(!pcsSkill || !pcsTarget || Operation == 0)
		return FALSE;

	PACKET_AGSMSKILL_RELAY_SKILLSAVE pPacketRelay(pcsTarget->m_szID);
	pPacketRelay.nOperation = Operation;
	pPacketRelay.m_pSaveData = pcsSkill->m_pSaveSkillData.m_pSaveData;

	AgsdServer* pRelayServer = m_pAgsmServerManager->GetRelayServer();
	if (!pRelayServer || !pRelayServer->m_bIsConnected)
		return FALSE;

	AgsEngine::GetInstance()->SendPacket(pPacketRelay, pRelayServer->m_dpnidServer);

	AgsdSkillADBase	*pcsAgsdSkillADBase	= GetADBase(pcsTarget);
	if(!pcsAgsdSkillADBase)
		return FALSE;

	switch(Operation)
	{
		case AGSMDATABASE_OPERATION_INSERT:
			{
				pcsAgsdSkillADBase->m_csEquipSkillArray.push_back(pcsSkill->m_pSaveSkillData.m_pSaveData);
			} break;
		case AGSMDATABASE_OPERATION_DELETE:
			{
				CEquipSkillArray::iterator it;
				it = pcsAgsdSkillADBase->m_csEquipSkillArray.begin();
				while( it != pcsAgsdSkillADBase->m_csEquipSkillArray.end() )
				{
					if( it->SkillTID == pcsSkill->m_pcsTemplate->m_lID )
						it = pcsAgsdSkillADBase->m_csEquipSkillArray.erase(it);
					else
						++it;
				}

				pcsSkill->m_pSaveSkillData.Clear();
			} break;
	}

	return TRUE;
}

AgpdSkill* AgsmSkill::FindBuffedSkill(AgpdCharacter *pcsCharacter, INT32 lSkillTID)
{
	if (!pcsCharacter || !lSkillTID)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkillADBase		*pcsADBase		= GetADBase((ApBase*)pcsCharacter);
	if (!pcsADBase)
		return FALSE;

	// 버프된 스킬이 있는지 검사한다.
	if (pcsAttachData->m_astBuffSkillList[0].lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			break;

		AgpdSkill* pSkill = pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill;
		if(!pSkill || !pSkill->m_pcsTemplate)
			break;

		if(pSkill->m_pcsTemplate->m_lID == lSkillTID)
			return pSkill;
	}

	return NULL;
}

BOOL AgsmSkill::CheckEnableEvolution(AgpdCharacter* pcsCharacter, AgpdSkill* pcsSkill)
{
	if(!pcsCharacter || !pcsSkill)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	
	INT32 lSkillLevel	= GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	INT32 lCreatureTID	= m_pagpmSkill->GetRandomCreatureTID(pcsSkillTemplate, lSkillLevel);

	return m_pagpmEventSkillMaster->m_vcSkillMasteryEvolution.CheckEnableEvolution(pcsCharacter->m_lTID1, lCreatureTID);
}

void AgsmSkill::RecastSaveEvolutionSkill(AgpdCharacter* pcsCharacter)
{
	//////////////////////////////////////////////////////////////////////////
	// save skill - arycoat 2008.7.
	if(!pcsCharacter) 
		return;

	ApBase* pcsBase			= (ApBase*)pcsCharacter;
	ApBase* pcsTargetBase	= (ApBase*)pcsCharacter;
	AgsdSkillADBase	*pcsAgsdSkillADBase	= GetADBase(pcsBase);
	if(!pcsAgsdSkillADBase) 
		return;
	
	if(pcsAgsdSkillADBase->m_csEquipSkillArray.empty()) 
		return;

	CEquipSkillArray EquipSkillArrayBack = pcsAgsdSkillADBase->m_csEquipSkillArray;

	for(CEquipSkillArray::iterator it = EquipSkillArrayBack.begin(); it != EquipSkillArrayBack.end(); ++it)
	{
		INT32 lSkillTID = it->SkillTID;
		AgpdSkillTemplate	*pcsSkillTemplate	= m_pagpmSkill->GetSkillTemplate(lSkillTID);
		if (!pcsSkillTemplate) continue;

		if(!m_pagpmSkill->IsEvolutionSkill(pcsSkillTemplate))
			continue;

		AgsdCastSkill pCastSkill;
		pCastSkill.pcsBase			= pcsBase;
		pCastSkill.pcsTargetBase	= pcsBase;
		pCastSkill.lSkillTID		= lSkillTID;
		pCastSkill.lSkillLevel		= 1;

		pCastSkill.m_pSaveSkillData.eStep	= SAVESKILL_RECASTING;
		pCastSkill.m_pSaveSkillData.m_pSaveData = (*it);

		CastSkill(pCastSkill);
	}
}

BOOL AgsmSkill::RemoveBuffedSkillEvolution(AgpdCharacter* pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = m_pagpmSkill->GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return FALSE;

	AgsdSkillADBase		*pcsADBase		= GetADBase((ApBase*)pcsCharacter);
	if (!pcsADBase)
		return FALSE;

	// 버프된 스킬이 있는지 검사한다.
	if (pcsAttachData->m_astBuffSkillList[0].lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID ||
			!pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill)
			break;

		AgpdSkill* pcsSkill = pcsADBase->m_csBuffedSkillProcessInfo[i].m_pcsTempSkill;
		if(!pcsSkill)
			break;

		if(m_pagpmSkill->IsEvolutionSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
			return m_pagsmSkillEffect->ProcessSkillEffectTransformTargetType5Cancel(pcsSkill, pcsCharacter);
	}

	return FALSE;
}

BOOL AgsmSkill::EndUpdateCharacterConfig(AgpdCharacter* pcsCharacter, AgpdSkill* pcsSkill)
{
	if(NULL == pcsCharacter || NULL == pcsSkill)
		return FALSE;

	UINT64 lSpecialStatus = (UINT64)((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lSpecialStatus;
	if(lSpecialStatus & AGPMSKILL_SPECIAL_STATUS_DISABLE_CHATTING)
	{
		if(pcsCharacter->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_DISABLE_CHATTING)
		{
			m_pagpmCharacter->UpdateUnsetSpecialStatus(pcsCharacter, AGPDCHAR_SPECIAL_STATUS_DISABLE_CHATTING);
		}
	}

	return TRUE;
}

BOOL AgsmSkill::CBRegionChange(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmSkill*		pThis			= (AgsmSkill*)pClass;
	AgpdCharacter*	pcsCharacter	= (AgpdCharacter*)pData;
	
	if(NULL == pcsCharacter)
		return FALSE;

	if(pThis->m_papmMap->CheckRegionPerculiarity(pcsCharacter->m_nBindingRegionIndex, APMMAP_PECULIARITY_PRESERVE_BUFF) == APMMAP_PECULIARITY_RETURN_DISABLE_USE)
	{
		pThis->EndAllBuffedSkillExceptTitleSkill((ApBase*)pcsCharacter);
	}

	return TRUE;
}

BOOL AgsmSkill::CBPreCheckEnableJanusTalisman( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdItem* pcsItem = static_cast<AgpdItem*>(pData);
	AgsmSkill* pThis = static_cast<AgsmSkill*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pCustData);

	BOOL bResult = pThis->IsEnableSkillByBuffedList(pcsItem, (ApBase*)pcsCharacter);

	if(!bResult)
	{
		// System Message 보낸다. 2006.01.05. steeple. 임시 땜빵
		if(pThis->m_pagpmPvP)
			pThis->m_pagpmPvP->ProcessSystemMessage(AGPMPVP_SYSTEM_CODE_SKILL_CANNOT_APPLY_EFFECT, NULL, NULL, 0, 0, pcsCharacter);
	}

	return bResult;
}

BOOL AgsmSkill::CheckSpecialStatusRate( AgpdSkill* pcsSkill )
{
	if(!pcsSkill || !pcsSkill->m_pcsTemplate)
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	if(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SPECIAL_STATUS_RATE][lSkillLevel] == 0 ||
		pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SPECIAL_STATUS_RATE][lSkillLevel] == 100)
		return TRUE;

	INT16 nRandomNumber = m_csRandom.randInt(100);
	if(nRandomNumber <= pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SPECIAL_STATUS_RATE][lSkillLevel])
		return TRUE;

	return FALSE;
}

BOOL AgsmSkill::CalcSkillOptionIgnoreStatus( AgpdSkill* pcsSkill, AgpdCharacter* pcsCharacter, BOOL bAdd )
{
	if ( !pcsSkill || !pcsSkill->m_pcsTemplate || !pcsCharacter || !pcsCharacter->m_pcsCharacterTemplate )
		return FALSE;

	AgpdSkillTemplate* pcsSkillTemplate = (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate;
	if(!pcsSkillTemplate)
		return FALSE;

	//////////////////////////////////////////////////////////////////////////
	INT32 lSkillLevel = GetModifiedSkillLevel(pcsSkill);
	if(!lSkillLevel)
		return FALSE;

	AgpdCharacterIgnoreStatus stIgnoreStatus;

	if ( bAdd )
	{
		stIgnoreStatus.m_lIgnorePhysicalAttackResist	+= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_PHYSICAL_ATTACK_RESIST][lSkillLevel];
		stIgnoreStatus.m_lIgnoreBlockRate				+= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_BLOCK_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreSkillBlockRate			+= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_SKILL_BLOCK_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreEvadeRate				+= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_EVADE_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreDodgeRate				+= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_DODGE_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreCriticalDefenseRate		+= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_CRITICAL_DEFENSE_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreStunDefenseRate			+= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_STUN_DEFENSE_RATE][lSkillLevel];
	}
	else
	{
		stIgnoreStatus.m_lIgnorePhysicalAttackResist	-= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_PHYSICAL_ATTACK_RESIST][lSkillLevel];
		stIgnoreStatus.m_lIgnoreBlockRate				-= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_BLOCK_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreSkillBlockRate			-= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_SKILL_BLOCK_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreEvadeRate				-= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_EVADE_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreDodgeRate				-= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_DODGE_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreCriticalDefenseRate		-= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_CRITICAL_DEFENSE_RATE][lSkillLevel];
		stIgnoreStatus.m_lIgnoreStunDefenseRate			-= (INT32)pcsSkillTemplate->m_fUsedConstFactor2[AGPMSKILL_CONST_IGNORE_STUN_DEFENSE_RATE][lSkillLevel];
	}

	m_pagsmCharacter->CalcCharacterIgnoreStatus(pcsCharacter, &stIgnoreStatus, TRUE);

	return TRUE;
}

