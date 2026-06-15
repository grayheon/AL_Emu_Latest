/******************************************************************************
Module:  AgsmParty.cpp
Notices: Copyright (c) NHN Studio 2002 netong
Purpose: 
Last Update: 2003. 02. 07
******************************************************************************/

#include "AgsmParty.h"

#include "ApLockManager.h"
#include "ApAutoLockCharacter.h"
#include "AuMath.h"
#include "AgpmBattleGround.h"
#include "AgsmSkill.h"
#include "AgpmEpicZone.h"

AgsmParty::AgsmParty()
{
	SetModuleName("AgsmParty");

	SetPacketType(AGSMPARTY_PACKET_TYPE);

	m_csPacket.SetFlagLength(sizeof(INT8));
	m_csPacket.SetFieldType(
							AUTYPE_INT8,		1,		// operation
							AUTYPE_INT32,		1,		// operator id
							AUTYPE_INT32,		1,		// target id
							AUTYPE_CHAR,		AGPACHARACTER_MAX_ID_STRING + 1,
							AUTYPE_END,			0
							);

	EnableIdle2(TRUE);

	m_pagpmSiegeWar = NULL;
	m_pagpmBattleGround		= NULL;
	m_pagsmSystemMessage	= NULL;

	m_ulNextProcessTimeMSec	= 0;
}

AgsmParty::~AgsmParty()
{
}

BOOL AgsmParty::OnAddModule()
{
	m_papmMap				= (ApmMap *) GetModule("ApmMap");
	m_pagpmFactors			= (AgpmFactors *) GetModule("AgpmFactors");
	m_pagpmParty			= (AgpmParty *) GetModule("AgpmParty");
	m_pagpmCharacter		= (AgpmCharacter *) GetModule("AgpmCharacter");
	m_pagpmSiegeWar			= (AgpmSiegeWar *) GetModule("AgpmSiegeWar");
	m_pagpmDropItem2		= (AgpmDropItem2 *) GetModule("AgpmDropItem2");
	m_pagpmItemConvert		= (AgpmItemConvert *) GetModule("AgpmItemConvert");
	m_papmEventManager		= (ApmEventManager *) GetModule("ApmEventManager");

	m_pagsmAOIFilter		= (AgsmAOIFilter *) GetModule("AgsmAOIFilter");
	m_pAgsmServerManager	= (AgsmServerManager *) GetModule("AgsmServerManager2");
	m_pagsmFactors			= (AgsmFactors *) GetModule("AgsmFactors");
	m_pagsmCharacter		= (AgsmCharacter *) GetModule("AgsmCharacter");
	m_pagsmZoning			= (AgsmZoning *) GetModule("AgsmZoning");
//	m_pagsmItem				= (AgsmItem *) GetModule("AgsmItem");// OnInit 으로 위치 이동

	if (!m_papmMap ||
		!m_pagpmFactors || 
		!m_pagpmParty || 
		!m_pagpmCharacter || 
		!m_pagpmSiegeWar ||
		!m_pagpmDropItem2 ||
		!m_pagpmItemConvert ||
		!m_papmEventManager ||
		!m_pagsmAOIFilter ||
		!m_pAgsmServerManager || 
		!m_pagsmFactors || 
		!m_pagsmCharacter ||
		!m_pagsmZoning)
		return FALSE;

	/*
	m_nIndexADParty = m_pagpmParty->AttachPartyData(this, sizeof(AgsdParty), ConAgsdParty, DesAgsdParty);
	if (m_nIndexADParty < 0)
		return FALSE;
	*/

	if (!m_pagpmFactors->SetCallbackUpdateFactorParty(CBUpdateFactorParty, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackUpdateStatus(CBUpdateActionStatus, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackRemoveChar(CBRemoveCharacter, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackTransformStatus(CBTransform, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackRestoreTransform(CBTransform, this))
		return FALSE;

	/*
	if (!m_papmMap->SetCallbackMoveChar(CBMoveSector, this))
		return FALSE;
	*/

	if (!m_pagsmAOIFilter->SetCallbackMoveChar(CBMoveCell, this))
		return FALSE;

	if (!m_pagpmParty->SetCBCreateParty(CBCreateParty, this))
		return FALSE;

	if (!m_pagpmParty->SetCBRemoveParty(CBRemoveParty, this))
		return FALSE;
	if (!m_pagpmParty->SetCBRemoveID(CBRemovePartyID, this))
		return FALSE;

	if (!m_pagpmParty->SetCBAddMember(CBAddMember, this))
		return FALSE;

	if (!m_pagpmParty->SetCBRemoveMember(CBRemoveMember, this))
		return FALSE;

	if (!m_pagpmParty->SetCBChangeLeader(CBChangeLeader, this))
		return FALSE;

	if (!m_pagpmParty->SetCBAddParty(CBAddParty, this))
		return FALSE;

	if (!m_pagpmParty->SetCBUpdateFactor(CBUpdateFactor, this))
		return FALSE;

	if (!m_pagpmParty->SetCBSyncRemoveCharacter(CBSyncCharacterRemove, this))
		return FALSE;

	if (!m_pagpmParty->SetCBCharLevelUp(CBCharLevelUp, this))
		return FALSE;

	if (!m_pagpmParty->SetCBResetEffectArea(CBResetEffectFactor, this))
		return FALSE;

	if (!m_pagpmParty->SetCBUpdateExpType(CBUpdateExpType, this))
		return FALSE;

	if (!m_pagpmParty->SetCBUpdatePartyFactor(CBUpdatePartyFactor, this))
		return FALSE;

	if (!m_pagpmParty->SetCBUpdateItemDivision(CBUpdateItemDivision, this))		// 2005.04.19 By SungHoon
		return FALSE;

	if (!m_pagpmDropItem2->SetCallbackGetBonusOfPartyMember(CBGetBonusOfPartyMember, this))
		return FALSE;

	if (!m_pagsmZoning->SetCallbackZoningPassControl(CBZoningPassControl, this))
		return FALSE;

	if (!m_pagsmCharacter->SetCallbackUpdateControlServer(CBUpdateControlServer, this))
		return FALSE;

	if (!m_pagsmCharacter->SetCallbackSyncCharacterActions(CBSyncCharacterActions, this))
		return FALSE;


#ifdef	AGSMPARTY_UPDATE_PARTY_FACTOR
	if (!m_pagsmCharacter->SetCallbackReCalcFactor(CBReCalcFactor, this))
		return FALSE;
#endif

//	2005.10.24. By SungHoon
	if (!m_pagpmCharacter->SetCallbackUpdateMurdererPoint(CBUpdateMurdererPoint, this))
		return FALSE;

//	if (!m_pagsmItem->SetCallbackPreCheckEnableJanusTalisman(CBPreCheckEnableJanusTalisman, this))
//		return FALSE;

	return TRUE;
}

BOOL AgsmParty::OnInit()
{
	m_pagpmBattleGround		= (AgpmBattleGround*)GetModule("AgpmBattleGround");
	m_pagsmSkill			= (AgsmSkill*)GetModule("AgsmSkill");
	m_pagpmSummons			= (AgpmSummons*)GetModule("AgpmSummons");
	m_pagsmSystemMessage	= (AgsmSystemMessage*)GetModule("AgsmSystemMessage");
	m_pagpmEpicZone			= (AgpmEpicZone*)GetModule("AgpmEpicZone");
	m_pagsmItem				= (AgsmItem *) GetModule("AgsmItem");

	
	if(!m_pagpmCharacter->SetCallbackBindingRegionChange(CBRegionChange, this))
		return FALSE;
	
	if(!m_pagpmBattleGround || !m_pagsmSkill
		|| !m_pagsmSystemMessage || !m_pagsmItem
		)
		return FALSE;

	if (!m_pagsmItem->SetCallbackPreCheckEnableJanusTalisman(CBPreCheckEnableJanusTalisman, this))
			return FALSE;
		
	return TRUE;
}

BOOL AgsmParty::OnDestroy()
{
	return TRUE;
}

BOOL AgsmParty::OnIdle2(UINT32 ulClockCount)
{
	//STOPWATCH2(GetModuleName(), _T("OnIdle2"));

	if (m_ulNextProcessTimeMSec <= ulClockCount)
	{
		INT32	lIndex	= 0;

		AgpdParty	*pcsParty	= m_pagpmParty->GetPartySequence(&lIndex);
		while (pcsParty)
		{
			if (pcsParty->m_Mutex.WLock())
			{
				m_pagpmParty->SetEffectArea(pcsParty);

				pcsParty->m_Mutex.Release();
			}

			pcsParty	= m_pagpmParty->GetPartySequence(&lIndex);
		}

		m_ulNextProcessTimeMSec	= ulClockCount + AGSMPARTY_PROCESS_IDLE_INTERVAL;
	}

	return TRUE;
}

/*
	2005.04.19	By SungHoon
	파티 멤버로 초대할떄
*/
BOOL AgsmParty::InvitePartyMember(  UINT32 ulNID, INT32 lOperatorID, INT32 lTargetID, CHAR *szTargetID )
{
	if (lOperatorID == AP_INVALID_CID || lOperatorID == lTargetID || (lTargetID == AP_INVALID_CID && szTargetID == NULL))
	{
		SendPacketFailed(lOperatorID, ulNID);
		return FALSE;
	}

	AgpdCharacter	*pcsOperator = m_pagpmCharacter->GetCharacterLock(lOperatorID);
	if (!pcsOperator || m_pagsmCharacter->GetCharDPNID(pcsOperator) == 0)
	{
		SendPacketFailed(lOperatorID, ulNID);
		return FALSE;
	}

	// RegionPeculiarity Check
	if(m_papmMap->CheckRegionPerculiarity(pcsOperator->m_nBindingRegionIndex, APMMAP_PECULIARITY_PARTY) == APMMAP_PECULIARITY_RETURN_DISABLE_USE)
	{
		SendPacketFailed(lOperatorID, ulNID);
		return FALSE;
	}

	INT32 lOperatorMurdererLevel = m_pagpmCharacter->GetMurdererLevel(pcsOperator);
	if (lOperatorMurdererLevel >= AGPMCHAR_MURDERER_LEVEL1_POINT)		//	MurdererPoint가 높으면 파티 못만듬
	{
		SendPacketInviteFailed(lOperatorID,
								lTargetID,
								AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_MURDERER_OPERATOR,
								ulNID);
		pcsOperator->m_Mutex.Release();
		return FALSE;
	}


	
	AgpdPartyADChar	*pcsOperatorPartyADChar = m_pagpmParty->GetADCharacter(pcsOperator);

	INT32	lPartyID	= pcsOperatorPartyADChar->lPID;

	pcsOperator->m_Mutex.Release();

	AgpdCharacter	*pcsTargetChar = NULL;

	if (lTargetID == AP_INVALID_CID) 
		pcsTargetChar = m_pagpmCharacter->GetCharacterLock(szTargetID);
	else 
		pcsTargetChar = m_pagpmCharacter->GetCharacterLock(lTargetID);
	
	if (!pcsTargetChar)		// 2005.04.21. By SungHoon
	{
		// 실패했다고 알려준다.
		if (szTargetID == NULL) SendPacketFailed(lOperatorID, ulNID);
		else SendPacketInviteFailed(lOperatorID, szTargetID, AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_NO_LOGIN_MEMBER, ulNID);

		return FALSE;
	}

	// 배틀 스퀘어만 체크하던것을 위험전투 지역인경우는 모두 파티 안되게 함으로 바꿈. supertj@20100413
	// 파티신청자와 파티대상자가 다른 지역에 있고 둘중에 어느 한명이 위험전투 지역에 있을때는 파티를 못하게 한다.
	ApmMap::RegionTemplate	*pcsRegionTemplateMe = m_papmMap->GetTemplate(pcsOperator->m_nBindingRegionIndex);	//파티신청자 지역정보.
	ApmMap::RegionTemplate	*pcsRegionTemplateTarget = m_papmMap->GetTemplate(pcsTargetChar->m_nBindingRegionIndex);	//파티대상자 지역정보.

	if (pcsRegionTemplateMe && pcsRegionTemplateTarget)
	{
		if(pcsRegionTemplateMe->ti.stType.uSafetyType != pcsRegionTemplateTarget->ti.stType.uSafetyType) //다른지역인지 체크
		{
			if( pcsRegionTemplateMe->ti.stType.uSafetyType == ApmMap::ST_DANGER ||	//위험전투지역에 있는지 체크
				pcsRegionTemplateTarget->ti.stType.uSafetyType == ApmMap::ST_DANGER )
			{
				SendPacketFailed(lOperatorID, ulNID);
				pcsTargetChar->m_Mutex.Release();
				return FALSE;
			}
		}
	}

	// RegionPeculiarity Check
	if(m_papmMap->CheckRegionPerculiarity(pcsTargetChar->m_nBindingRegionIndex, APMMAP_PECULIARITY_PARTY) == APMMAP_PECULIARITY_RETURN_DISABLE_USE)
	{
		SendPacketFailed(lOperatorID, ulNID);
		pcsTargetChar->m_Mutex.Release();
		return FALSE;
	}
	
	INT32 lTargetMurdererLevel = m_pagpmCharacter->GetMurdererLevel(pcsTargetChar);
	if (lTargetMurdererLevel >= AGPMCHAR_MURDERER_LEVEL1_POINT)
	{
		SendPacketInviteFailed(lOperatorID,
								pcsTargetChar->m_szID,
								AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_MURDERER_TARGET,
								ulNID);
		pcsTargetChar->m_Mutex.Release();
		return FALSE;
	}

	if (m_pagpmCharacter->IsOptionFlag(pcsTargetChar,AGPDCHAR_OPTION_REFUSE_PARTY_IN) == TRUE)	//	2005.05.31. 거부인지 확인한다.
	{
		SendPacketInviteFailed(lOperatorID, pcsTargetChar->m_szID, AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_REFUSE, ulNID);
		pcsTargetChar->m_Mutex.Release();
		return FALSE;
	}

	// Ban list에서 초대 차단인지 확인한다.
	if (FALSE == EnumCallback(AGSMPARTY_CB_BUDDY_CHECK, pcsOperator, pcsTargetChar))
	{
		SendPacketInviteFailed(lOperatorID, pcsTargetChar->m_szID, AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_REFUSE, ulNID);
		pcsTargetChar->m_Mutex.Release();
		return FALSE;
	}

	if (m_pagsmCharacter->GetCharDPNID(pcsTargetChar) == 0)
	{
		// 실패했다고 알려준다.
		SendPacketFailed(lOperatorID, ulNID);
		pcsTargetChar->m_Mutex.Release();
		return FALSE;
	}
	
	BOOL bOperator = m_pagpmBattleGround->IsInBattleGround(pcsOperator);
	BOOL bTarget = m_pagpmBattleGround->IsInBattleGround(pcsTargetChar);
	if(	(bOperator && bTarget && m_pagpmCharacter->IsSameRace(pcsOperator, pcsTargetChar) == FALSE)
		|| (bOperator && !bTarget) || (!bOperator && bTarget) )
	{
		SendPacketFailed(lOperatorID, ulNID);
		pcsOperator->m_Mutex.Release();
		return FALSE;
	}

	bOperator = m_pagpmEpicZone->IsInEpicZone(pcsOperator);
	bTarget = m_pagpmEpicZone->IsInEpicZone(pcsTargetChar);
	if((bOperator && !bTarget) || (!bOperator && bTarget))
	{
		SendPacketFailed(lOperatorID, ulNID);
		pcsOperator->m_Mutex.Release();
		return FALSE;
	}

	AgpdPartyADChar *pcsTargetPartyADChar = m_pagpmParty->GetADCharacter(pcsTargetChar);

	if (pcsTargetPartyADChar->lPID != AP_INVALID_PARTYID)
	{
		SendPacketInviteFailed(lOperatorID,
								lTargetID,
								AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_ALREADY_OTHER_PARTY_MEMBER,
								ulNID);
		pcsTargetChar->m_Mutex.Release();
		return FALSE;
	}

	AgpdParty	*pcsParty = NULL;

	if (lPartyID != AP_INVALID_PARTYID)
	{
		pcsParty = m_pagpmParty->GetPartyLock(pcsOperatorPartyADChar->lPID);
		if (!pcsParty)
		{
			pcsTargetChar->m_Mutex.Release();
			SendPacketFailed(lOperatorID, ulNID);
			return FALSE;
		}
	}
	
	if (pcsParty)
	{
		// 파티에 추가할 수 있는지 검사한다.
		// 초대한 캐릭터가 파티 리더인지 검사한다.
		if (pcsParty->m_lMemberListID[0] != lOperatorID)
		{
			SendPacketInviteFailed(lOperatorID,
									lTargetID,
									AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_ALREADY_OTHER_PARTY_MEMBER,
									ulNID);
			pcsTargetChar->m_Mutex.Release();
			pcsParty->m_Mutex.Release();
			return FALSE;
		}
		else if (!IsAddMember(pcsParty, pcsTargetChar))
		{
			SendPacketInviteFailed(lOperatorID,
									lTargetID,
									AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_PARTY_MEMBER_IS_FULL,
									ulNID);
			pcsTargetChar->m_Mutex.Release();
			pcsParty->m_Mutex.Release();
			return FALSE;
		}
		else if (m_pagpmParty->IsMember(pcsParty, pcsTargetChar->m_lID))
		{
			// 이미 가입되어져 있는 멤버다.
			SendPacketInviteFailed(lOperatorID,
									lTargetID,
									AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_ALREADY_PARTY_MEMBER,
									ulNID);
			pcsTargetChar->m_Mutex.Release();
			pcsParty->m_Mutex.Release();
			return FALSE;
		}
/*		else			// 레벨 체크는 없앤다. By SungHoon 2005.06.08
		{
//	레벨 체크 해야 한다.

			INT32	lMemberLevel = m_pagpmCharacter->GetLevel(pcsTargetChar);
			INT32	lLevelOffset = AGPMPARTY_AVAILABLE_LEVEL_DIFF - (pcsParty->m_lHighestMemberLevel - pcsParty->m_lLowerMemberLevel);
			if (lLevelOffset < 0 || lLevelOffset > AGPMPARTY_AVAILABLE_LEVEL_DIFF) lLevelOffset = AGPMPARTY_AVAILABLE_LEVEL_DIFF;
			if ((lMemberLevel + lLevelOffset < pcsParty->m_lLowerMemberLevel) ||
				(lMemberLevel > pcsParty->m_lHighestMemberLevel + lLevelOffset))
			{
				SendPacketInviteFailed(lOperatorID,
										lTargetID,
										AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_LEVEL_DIFF,
										ulNID);
				pcsTargetChar->m_Mutex.Release();
				pcsParty->m_Mutex.Release();
				return FALSE;
			}
		}
*/
	}
/*	else			// 레벨 체크는 없앤다. By SungHoon 2005.06.08
	{
//	레벨 체크 해야 한다.
		INT32	lLevelOffset = abs( m_pagpmCharacter->GetLevel(pcsOperator) - m_pagpmCharacter->GetLevel(pcsTargetChar) );
		if ( AGPMPARTY_AVAILABLE_LEVEL_DIFF <= lLevelOffset)
		{
			SendPacketInviteFailed(lOperatorID,
									lTargetID,
									AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_LEVEL_DIFF,
									ulNID);
			pcsTargetChar->m_Mutex.Release();
			return FALSE;
		}
	}
*/
	AgsdCharacter *pcsAgsdTargetChar = m_pagsmCharacter->GetADCharacter(pcsTargetChar);

	// target character에게 초대한다는 정보를 보내준다.
	SendPacketInvite(lOperatorID, pcsTargetChar->m_lID, pcsOperator->m_szID, pcsAgsdTargetChar->m_dpnidCharacter);


	if (pcsParty)
		pcsParty->m_Mutex.Release();

	pcsTargetChar->m_Mutex.Release();

	return TRUE;
}


BOOL AgsmParty::OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)
{
	if (!pvPacket || nSize == 0)
		return FALSE;

	INT8		cOperation	= (-1);
	INT32		lOperatorID	= AP_INVALID_CID;
	INT32		lTargetID	= AP_INVALID_CID;
	CHAR		*szTargetID = NULL;

	m_csPacket.GetField(TRUE, pvPacket, nSize, 
						&cOperation,
						&lOperatorID,
						&lTargetID,
						&szTargetID );

	switch (cOperation) {
	case AGSMPARTY_PACKET_OPERATION_INVITE:
		{
			return ( InvitePartyMember( ulNID, lOperatorID, lTargetID, szTargetID ) );
		}
		break;

	case AGSMPARTY_PACKET_OPERATION_REJECT:
		{
			if (lOperatorID == AP_INVALID_CID || lTargetID == AP_INVALID_CID)
			{
				SendPacketFailed(lOperatorID, ulNID);
				return FALSE;
			}

			AgpdCharacter *pcsTargetChar = m_pagpmCharacter->GetCharacterLock(lTargetID);
			if (!pcsTargetChar)
			{
				SendPacketFailed(lOperatorID, ulNID);
				return FALSE;
			}

			AgsdCharacter *pcsAgsdTargetChar = m_pagsmCharacter->GetADCharacter(pcsTargetChar);

			// 패킷을 보내준다. (target 에게로...)
			SendPacketReject(lOperatorID, lTargetID, pcsAgsdTargetChar->m_dpnidCharacter);

			pcsTargetChar->m_Mutex.Release();
		}
		break;

	case AGSMPARTY_PACKET_OPERATION_INVITE_ACCEPT:
		{
			if (lOperatorID == AP_INVALID_CID || lTargetID == AP_INVALID_CID || lOperatorID == lTargetID)
			{
				SendPacketFailed(lTargetID, ulNID);
				return FALSE;
			}

			// lTargetID가 초대한 넘이고 lOperatorID가 초대를 받은 넘이다.
			///////////////////////////////////////////////////////////////////////

			AgpdCharacter *pcsTargetChar = m_pagpmCharacter->GetCharacterLock(lTargetID);
			if (!pcsTargetChar)
			{
				return FALSE;
			}
			
			if (m_pagsmCharacter->GetCharDPNID(pcsTargetChar) == 0)
			{
				pcsTargetChar->m_Mutex.Release();
				return FALSE;
			}

			AgpdPartyADChar	*pcsTargetPartyADChar = m_pagpmParty->GetADCharacter(pcsTargetChar);

			INT32	lPartyID		= pcsTargetPartyADChar->lPID;
			UINT32	ulTargetDPNID	= m_pagsmCharacter->GetCharDPNID(pcsTargetChar);

			/* 아래서 계속한다. Parn
			if (lPartyID != AP_INVALID_PARTYID)
				pcsTargetChar->m_Mutex.Release();
			*/

			AgpdCharacter *pcsOperator = m_pagpmCharacter->GetCharacterLock(lOperatorID);
			if (!pcsOperator || m_pagsmCharacter->GetCharDPNID(pcsOperator) == 0)
			{
				pcsTargetChar->m_Mutex.Release();

				if (pcsOperator)
					pcsOperator->m_Mutex.Release();

				return FALSE;
			}

			// 타 종족끼리 배틀 그라운드에 있을경우 파티 불가. , 20090203 absenty
			BOOL bOperator = m_pagpmBattleGround->IsInBattleGround(pcsOperator);
			BOOL bTarget = m_pagpmBattleGround->IsInBattleGround(pcsTargetChar);
			if(	(bOperator && bTarget && m_pagpmCharacter->IsSameRace(pcsOperator, pcsTargetChar) == FALSE)
				|| (bOperator && !bTarget) || (!bOperator && bTarget) )
			{
				SendPacketFailed(lOperatorID, ulNID);
				pcsOperator->m_Mutex.Release();
				return FALSE;
			}

			bOperator = m_pagpmEpicZone->IsInEpicZone(pcsOperator);
			bTarget = m_pagpmEpicZone->IsInEpicZone(pcsTargetChar);
			if((bOperator && !bTarget) || (!bOperator && bTarget))
			{
				SendPacketFailed(lOperatorID, ulNID);
				pcsOperator->m_Mutex.Release();
				return FALSE;
			}

			//AgsdCharacter *pcsAgsdTargetChar = m_pagsmCharacter->GetADCharacter(pcsTargetChar);
			
			AgpdPartyADChar	*pcsOperatorPartyADChar = m_pagpmParty->GetADCharacter(pcsOperator);

			AgpdParty	*pcsParty = NULL;
			if (lPartyID != AP_INVALID_PARTYID)
			{
				pcsParty = m_pagpmParty->GetPartyLock(lPartyID);
				if (!pcsParty)
				{
					pcsTargetChar->m_Mutex.Release();
					pcsOperator->m_Mutex.Release();
					SendPacketFailed(lTargetID, ulTargetDPNID);

					return FALSE;
				}
			}

			// 야누스의 부적 상태면 파티에 들어갈수 없다
			if (pcsOperator->m_eJanusType != AGPDCHAR_JANUS_TALISMAN_NONE
				|| pcsTargetChar->m_eJanusType != AGPDCHAR_JANUS_TALISMAN_NONE )
			{
				SendPacketFailed(lOperatorID, ulNID);

				SendPacketInviteFailed(lOperatorID,
					lTargetID,
					AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_USING_JANUS_TALISMAN,
					ulNID);

				if (pcsParty)
					pcsParty->m_Mutex.Release();

				pcsOperator->m_Mutex.Release();

				pcsTargetChar->m_Mutex.Release();

				return FALSE;
			}

			if (pcsOperatorPartyADChar->lPID != AP_INVALID_PARTYID)
			{
				SendPacketFailed(lOperatorID, ulNID);
				//SendPacketFailed(lTargetID, ulTargetDPNID);

				SendPacketInviteFailed(lOperatorID,
									   lTargetID,
									   AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_ALREADY_OTHER_PARTY_MEMBER,
									   ulNID);

				if (pcsParty)
					pcsParty->m_Mutex.Release();

				pcsOperator->m_Mutex.Release();

				pcsTargetChar->m_Mutex.Release();

				return FALSE;
			}

			if (pcsParty)
			{
				// 파티에 추가할 수 있는지 검사한다.
				// 초대한 캐릭터가 파티 리더인지 검사한다.
				if (pcsParty->m_lMemberListID[0] != lTargetID)
				{
					SendPacketInviteFailed(lTargetID,
										   lOperatorID,
										   AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_ALREADY_OTHER_PARTY_MEMBER,
										   ulTargetDPNID);
					SendPacketFailed(lOperatorID, ulNID);
					pcsParty->m_Mutex.Release();
					pcsOperator->m_Mutex.Release();

					pcsTargetChar->m_Mutex.Release();

					return FALSE;
				}
				else if (!IsAddMember(pcsParty, pcsOperator))
				{
					SendPacketInviteFailed(lTargetID,
										   lOperatorID,
										   AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_PARTY_MEMBER_IS_FULL,
										   ulTargetDPNID);
					SendPacketFailed(lOperatorID, ulNID);
					pcsParty->m_Mutex.Release();
					pcsOperator->m_Mutex.Release();

					pcsTargetChar->m_Mutex.Release();

					return FALSE;
				}
				else if (m_pagpmParty->IsMember(pcsParty, pcsOperator->m_lID))
				{
					// 이미 가입되어져 있는 멤버다.
					SendPacketInviteFailed(lTargetID,
										   lOperatorID,
										   AGSMPARTY_PACKET_OPERATION_INVITE_FAILED_ALREADY_PARTY_MEMBER,
										   ulTargetDPNID);
					SendPacketFailed(lOperatorID, ulNID);

					pcsParty->m_Mutex.Release();
					pcsOperator->m_Mutex.Release();

					pcsTargetChar->m_Mutex.Release();

					return FALSE;
				}

				/*
				// 파티에 추가할 수 있는지 검사한다.
				// 초대한 캐릭터가 파티 리더인지 검사한다.
				if (pcsParty->m_lMemberListID[0] != lTargetID || !IsAddMember(pcsParty, pcsOperator) || m_pagpmParty->IsMember(pcsParty, pcsOperator->m_lID))
				{
					SendPacketFailed(lOperatorID, ulNID);
					SendPacketFailed(lTargetID, ulTargetDPNID);

					pcsParty->m_Mutex.Release();
					pcsOperator->m_Mutex.Release();
					if (lPartyID == AP_INVALID_PARTYID)
						pcsTargetChar->m_Mutex.Release();
					return FALSE;
				}
				*/
			}

			// 파티를 맹근다. 혹은 파티에 추가한다.

			if (!pcsParty)		// 파티가 없다. 새로 생성한다.
			{
				pcsParty = m_pagpmParty->CreateParty(m_csGenerateID.GetID(), pcsTargetChar, pcsOperator);
				if (!pcsParty)
				{
					SendPacketFailed(lOperatorID, ulNID);
					SendPacketFailed(lTargetID, ulTargetDPNID);
					pcsOperator->m_Mutex.Release();

					pcsTargetChar->m_Mutex.Release();

					return FALSE;
				}
				else
					pcsParty = NULL;
			}
			else														// 이미 존재하는 파티다. 걍 target 만 파티에 추가한다.
			{
				if (!m_pagpmParty->AddMember(pcsParty, pcsOperator))
				{
					SendPacketFailed(lOperatorID, ulNID);
					SendPacketFailed(lTargetID, ulTargetDPNID);
					pcsParty->m_Mutex.Release();
					pcsOperator->m_Mutex.Release();

					pcsTargetChar->m_Mutex.Release();

					return FALSE;
				}
			}

			// 패킷을 보내준다. (target 에게로...)

			SendPacketAccept(lOperatorID, lTargetID, ulTargetDPNID);

			if (pcsParty)
				pcsParty->m_Mutex.Release();

			pcsOperator->m_Mutex.Release();

			pcsTargetChar->m_Mutex.Release();
		}
		break;

	case AGSMPARTY_PACKET_OPERATION_LEAVE:
		{
			return ProcessRemoveMember(lOperatorID);
		}
		break;

	case AGSMPARTY_PACKET_OPERATION_BANISH:
		{
			if (lOperatorID == AP_INVALID_CID || lTargetID == AP_INVALID_CID || lOperatorID == lTargetID)
			{
				SendPacketFailed(lOperatorID, ulNID);
				return FALSE;
			}

			AgpdCharacter *pcsOperator = m_pagpmCharacter->GetCharacterLock(lOperatorID);
			if (!pcsOperator)
			{
				SendPacketFailed(lOperatorID, ulNID);
				return FALSE;
			}

			AgpdPartyADChar	*pcsOperatorPartyADChar = m_pagpmParty->GetADCharacter(pcsOperator);

			INT32	lPartyID	= pcsOperatorPartyADChar->lPID;

			pcsOperator->m_Mutex.Release();

			AgpdParty	*pcsParty = m_pagpmParty->GetPartyLock(lPartyID);
			if (!pcsParty || !m_pagpmParty->IsMember(pcsParty, lTargetID))
			{
				SendPacketFailed(lOperatorID, ulNID);
				return FALSE;
			}

			// lOperator가 파티 리더인지 검사한다. (리더만 추방할 수 있다.)
			if (pcsParty->m_lMemberListID[0] != lOperatorID)
			{
				pcsParty->m_Mutex.Release();
				SendPacketFailed(lOperatorID, ulNID);
				return FALSE;
			}

			AgpdCharacter *pcsTargetChar = m_pagpmCharacter->GetCharacterLock(lTargetID);
			if (!pcsTargetChar)
			{
				pcsParty->m_Mutex.Release();
				SendPacketFailed(lOperatorID, ulNID);
				return FALSE;
			}

			AgsdCharacter *pcsAgsdTargetChar = m_pagsmCharacter->GetADCharacter(pcsTargetChar);

			// 추방한다는 패킷을 target으로 보낸다.
			SendPacketBanish(lOperatorID, lTargetID, pcsAgsdTargetChar->m_dpnidCharacter);

			// 파티에서 뺀다.
			BOOL	bDestroyParty = FALSE;

			m_pagpmParty->RemoveMember(pcsParty, lTargetID, &bDestroyParty);

			pcsTargetChar->m_Mutex.Release();

			if (!bDestroyParty)
				pcsParty->m_Mutex.Release();
		}
		break;
	case AGSMPARTY_PACKET_OPERATION_DELEGATION_LEADER :
		{
			DelegationLeader(ulNID, lOperatorID,lTargetID);
		}
		break;

	case AGSMPARTY_PACKET_OPERATION_ACCEPT_RECALL:
		{
			OnReceiveAcceptRecall(lOperatorID, lTargetID);
		}
		break;

	case AGSMPARTY_PACKET_OPERATION_REJECT_RECALL:
		{
			OnReceiveRejectRecall(lOperatorID, lTargetID);
		}
		break;
	}

	return TRUE;
}

//		IsAddMember
//	Functions
//		- 파티에 멤버를 추가할 수 있는지 검사한다.
//			1. 파티가 꽉찼는지 검사한다.
//			2. 추가하려는 멤버가 이미 다른 파티에 들어 있는지 검사한다.
//	Arguments
//		- pcsPartyLeader : 파티장
//		- pcsMember : 추가하려는 파티원
//	Return value
//		- BOOL : success or fail
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmParty::IsAddMember(AgpdCharacter *pcsPartyLeader, AgpdCharacter *pcsMember)
{
	if (!pcsPartyLeader || !pcsMember)
		return FALSE;

	AgpdPartyADChar *pcsLeaderPartyADChar = m_pagpmParty->GetADCharacter(pcsPartyLeader);

	if (pcsLeaderPartyADChar->pcsParty)
		return IsAddMember(pcsLeaderPartyADChar->pcsParty, pcsMember);
	else
		return IsAddMember(m_pagpmParty->GetParty(pcsLeaderPartyADChar->lPID), pcsMember);

	return TRUE;
}

BOOL AgsmParty::IsAddMember(AgpdParty *pcsParty, AgpdCharacter *pcsMember)
{
	if (!pcsParty || !pcsMember)
		return FALSE;

	if (m_pagpmParty->IsFull(pcsParty))
		return FALSE;									// 파티가 이미 포화상태다.

	AgpdPartyADChar	*pcsMemberPartyADChar = m_pagpmParty->GetADCharacter(pcsMember);

	// 멤버로 추가하려는 넘이 이미 파티에 들어있다.
	if (pcsMemberPartyADChar->lPID != AP_INVALID_PARTYID)
	{
		return FALSE;
	}
	
	AgpdCharacter* pcsLeader = m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[0]);
	if(m_pagpmBattleGround->IsAttackable(pcsLeader, pcsMember)) return FALSE;

	return TRUE;
}

BOOL AgsmParty::ProcessRemoveMember(INT32 lOperatorID)
{
	if (lOperatorID == AP_INVALID_CID)
		return FALSE;

	// 파티에서 뺀다.

	AgpdCharacter *pcsOperator = m_pagpmCharacter->GetCharacter(lOperatorID);
	if (!pcsOperator) return FALSE;
	AuAutoLock pLock2(pcsOperator->m_Mutex);

	AgpdPartyADChar	*pcsPartyADChar = m_pagpmParty->GetADCharacter(pcsOperator);
	if(!pcsPartyADChar) return FALSE;

	AgpdParty *pcsParty = m_pagpmParty->GetPartyLock(pcsPartyADChar->lPID);
	if (!pcsParty) return FALSE;
	AuAutoLock pLock(pcsParty->m_Mutex);

	BOOL	bDestroyParty = FALSE;
	m_pagpmParty->RemoveMember(pcsParty, pcsOperator->m_lID, &bDestroyParty);

	return TRUE;
}

BOOL AgsmParty::SendPacketInvite(INT32 lOperator, INT32 lTarget, CHAR *szTargetID, DPNID dpnid)
{
	INT16	nPacketLength;

	PVOID	pvPacket = MakePacketInvite(lOperator, lTarget, szTargetID, &nPacketLength);
	if (!pvPacket)
		return FALSE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid))
	{
//		TRACEFILE("AgsmParty::SendPacketInvite() failed");
		m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgsmParty::SendPacketReject(INT32 lOperator, INT32 lTarget, DPNID dpnid)
{
	INT16	nPacketLength;

	PVOID	pvPacket = MakePacketReject(lOperator, lTarget, &nPacketLength);
	if (!pvPacket)
		return FALSE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid))
	{
//		TRACEFILE("AgsmParty::SendPacketReject() failed");
		m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgsmParty::SendPacketAccept(INT32 lOperator, INT32 lTarget, DPNID dpnid)
{
	INT16	nPacketLength;

	PVOID	pvPacket = MakePacketAccept(lOperator, lTarget, &nPacketLength);
	if (!pvPacket)
		return FALSE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid))
	{
//		TRACEFILE("AgsmParty::SendPacketAccept() failed");
		m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgsmParty::SendPacketLeave(INT32 lOperator, DPNID dpnid)
{
	INT16	nPacketLength;

	PVOID	pvPacket = MakePacketLeave(lOperator, &nPacketLength);
	if (!pvPacket)
		return FALSE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid))
	{
//		TRACEFILE("AgsmParty::SendPacketLeave() failed");
		m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgsmParty::SendPacketBanish(INT32 lOperator, INT32 lTarget, DPNID dpnid)
{
	INT16	nPacketLength;

	PVOID	pvPacket = MakePacketBanish(lOperator, lTarget, &nPacketLength);
	if (!pvPacket)
		return FALSE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid))
	{
//		TRACEFILE("AgsmParty::SendPacketBanish() failed");
		m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgsmParty::SendPacketInviteFailed(INT32 lOperator, INT32 lTarget, eAgsmPartyPacketOperation eFailReason, DPNID dpnid)
{
	INT16	nPacketLength;

	PVOID	pvPacket = MakePacketInviteFailed(lOperator, lTarget, eFailReason, &nPacketLength);
	if (!pvPacket)
		return FALSE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid))
	{
//		TRACEFILE("AgsmParty::SendPacketInviteFailed() failed");
		m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgsmParty::SendPacketInviteFailed(INT32 lOperator, CHAR *szTargetID, eAgsmPartyPacketOperation eFailReason, DPNID dpnid)
{
	INT16	nPacketLength;

	PVOID	pvPacket = MakePacketInviteFailed(lOperator, szTargetID, eFailReason, &nPacketLength);
	if (!pvPacket)
		return FALSE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid))
	{
//		TRACEFILE("AgsmParty::SendPacketInviteFailed() failed");
		m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgsmParty::SendPacketFailed(INT32 lOperator, DPNID dpnid)
{
	INT16	nPacketLength;

	PVOID	pvPacket = MakePacketFailed(lOperator, &nPacketLength);
	if (!pvPacket)
		return FALSE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid))
	{
//		TRACEFILE("AgsmParty::SendPacketFailed() failed");
		m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

/*
	2005.05.02	By SungHoon
	파티장 위임에 실패했다는 패킷을 위임을 요청한 플레이어에게 보낸다.
*/
BOOL AgsmParty::SendDelegationLeaderFailed(INT32 lOperator, DPNID dpnid)
{
	INT16	nPacketLength;

	PVOID	pvPacket = MakePacketDelegationLeaderFailed(lOperator, &nPacketLength);
	if (!pvPacket)
		return FALSE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid))
	{
		m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgsmParty::SendPacketRequestRecall(INT32 lOperator, UINT32 ulNID)
{
	if (ulNID == 0)
		return FALSE;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= MakePacketRequestRecall(lOperator, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL	bSendResult		= SendPacket(pvPacket, nPacketLength, ulNID);

	m_csPacket.FreePacket(pvPacket);

	return	bSendResult;
}

BOOL AgsmParty::SendPacketAcceptRecall(INT32 lOperator, INT32 lTargetID, UINT32 ulNID)
{
	if (ulNID == 0)
		return FALSE;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= MakePacketAcceptRecall(lOperator, lTargetID, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL	bSendResult		= SendPacket(pvPacket, nPacketLength, ulNID);

	m_csPacket.FreePacket(pvPacket);

	return	bSendResult;
}

BOOL AgsmParty::SendPacketRejectRecall(INT32 lOperator, INT32 lTargetID, UINT32 ulNID)
{
	if (ulNID == 0)
		return FALSE;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= MakePacketRejectRecall(lOperator, lTargetID, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL	bSendResult		= SendPacket(pvPacket, nPacketLength, ulNID);

	m_csPacket.FreePacket(pvPacket);

	return	bSendResult;
}

PVOID AgsmParty::MakePacketInvite(INT32 lOperator, INT32 lTarget, CHAR *szTargetID, INT16 *pnPacketLength)
{
	INT8	cOperation = AGSMPARTY_PACKET_OPERATION_INVITE;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												&lTarget,
												szTargetID);

	return pvPacket;
}

PVOID AgsmParty::MakePacketReject(INT32 lOperator, INT32 lTarget, INT16 *pnPacketLength)
{
	INT8	cOperation = AGSMPARTY_PACKET_OPERATION_REJECT;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												&lTarget,
												NULL);

	return pvPacket;
}

PVOID AgsmParty::MakePacketAccept(INT32 lOperator, INT32 lTarget, INT16 *pnPacketLength)
{
	INT8	cOperation = AGSMPARTY_PACKET_OPERATION_INVITE_ACCEPT;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												&lTarget,
												NULL);

	return pvPacket;
}

PVOID AgsmParty::MakePacketLeave(INT32 lOperator, INT16 *pnPacketLength)
{
	INT8	cOperation = AGSMPARTY_PACKET_OPERATION_LEAVE;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												NULL,
												NULL);

	return pvPacket;
}

PVOID AgsmParty::MakePacketBanish(INT32 lOperator, INT32 lTarget, INT16 *pnPacketLength)
{
	INT8	cOperation = AGSMPARTY_PACKET_OPERATION_BANISH;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												&lTarget,
												NULL);

	return pvPacket;
}

PVOID AgsmParty::MakePacketInviteFailed(INT32 lOperator, CHAR *szTargetID, eAgsmPartyPacketOperation eFailReason, INT16 *pnPacketLength)
{
	INT8	cOperation = eFailReason;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												NULL,
												szTargetID);

	return pvPacket;
}

PVOID AgsmParty::MakePacketInviteFailed(INT32 lOperator, INT32 lTarget, eAgsmPartyPacketOperation eFailReason, INT16 *pnPacketLength)
{
	INT8	cOperation = eFailReason;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												&lTarget,
												NULL);

	return pvPacket;
}

PVOID AgsmParty::MakePacketFailed(INT32 lOperator, INT16 *pnPacketLength)
{
	INT8	cOperation = AGSMPARTY_PACKET_OPERATION_FAILED;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												NULL,
												NULL);

	return pvPacket;
}

/*
	2005.05.02	By SungHoon
	파티장 위임성공 패킷을 만든다.
*/
PVOID AgsmParty::MakePacketDelegationLeader(INT32 lOperator, INT32 lTargetID, INT16 *pnPacketLength)
{
	INT8	cOperation = AGSMPARTY_PACKET_OPERATION_DELEGATION_LEADER;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												&lTargetID,
												NULL);

	return pvPacket;
}

/*
	2005.05.02	By SungHoon
	파티장 위임실패 패킷을 만든다.
*/
PVOID AgsmParty::MakePacketDelegationLeaderFailed(INT32 lOperator, INT16 *pnPacketLength)
{
	INT8	cOperation = AGSMPARTY_PACKET_OPERATION_DELEGATION_LEADER_FAILED;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												NULL,
												NULL);

	return pvPacket;
}

PVOID AgsmParty::MakePacketRequestRecall(INT32 lOperator, INT16 *pnPacketLength)
{
	if (lOperator == AP_INVALID_CID || !pnPacketLength)
		return NULL;

	INT8	cOperation	= AGSMPARTY_PACKET_OPERATION_REQUEST_RECALL;

	return	m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												NULL,
												NULL);
}

PVOID AgsmParty::MakePacketAcceptRecall(INT32 lOperator, INT32 lTargetID, INT16 *pnPacketLength)
{
	if (lOperator == AP_INVALID_CID || lTargetID == AP_INVALID_CID || !pnPacketLength)
		return NULL;

	INT8	cOperation	= AGSMPARTY_PACKET_OPERATION_ACCEPT_RECALL;

	return	m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												&lTargetID,
												NULL);
}

PVOID AgsmParty::MakePacketRejectRecall(INT32 lOperator, INT32 lTargetID, INT16 *pnPacketLength)
{
	if (lOperator == AP_INVALID_CID || lTargetID == AP_INVALID_CID || !pnPacketLength)
		return NULL;

	INT8	cOperation	= AGSMPARTY_PACKET_OPERATION_REJECT_RECALL;

	return	m_csPacket.MakePacket(TRUE, pnPacketLength, AGSMPARTY_PACKET_TYPE,
												&cOperation,
												&lOperator,
												&lTargetID,
												NULL);
}

BOOL AgsmParty::InitServer(UINT32 ulStartValue, UINT32 ulServerFlag, INT16 nSizeServerFlag, INT32 lRemoveIDQueueSize)
{
	return m_csGenerateID.Initialize(ulStartValue, ulServerFlag, nSizeServerFlag, lRemoveIDQueueSize, TRUE);
}

/*
AgsdParty* AgsmParty::GetADParty(AgpdParty *pcsParty)
{
	if (pcsParty)
		return (AgsdParty *) m_pagpmParty->GetAttachedModuleData(m_nIndexADParty, (PVOID) pcsParty);

	return NULL;
}
*/

BOOL AgsmParty::CBCreateParty(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmParty *pThis	= (AgsmParty *) pClass;
	AgpdParty *pcsParty	= (AgpdParty *) pData;

	pThis->m_pagpmParty->EnumCallback(AGPMPARTY_CB_ADD_MEMBER, pcsParty, pThis->m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[0]));

	/*
	AgsdParty *pcsAgsdParty = pThis->GetADParty(pcsParty);

	pcsAgsdParty->m_dpnidPartyServer = pThis->m_pagsmAOIFilter->CreateGroup(AGPMPARTY_MAX_PARTY_MEMBER);

	// 파티 그룹을 만들고 그룹안에 현재 파티원들을 등록시킨다.
	pcsAgsdParty->m_dpnidParty = pThis->m_pagsmAOIFilter->CreateGroup(AGPMPARTY_MAX_PARTY_MEMBER);
	*/
/*
	pThis->SendPartyInfo(pcsParty);

	for (int i = 0; i < pcsParty->m_nCurrentMember; i++)
	{
		if (!pcsParty->m_pcsMemberList[i])
			return FALSE;//continue;

		AgsdCharacter	*pcsAgsdCharacter = pThis->m_pagsmCharacter->GetADCharacter(pcsParty->m_pcsMemberList[i]);
		if (!pcsAgsdCharacter)
			return FALSE;//continue;

		//pThis->m_pagsmAOIFilter->AddPlayerToGroup(pcsAgsdParty->m_dpnidParty, pcsAgsdCharacter->m_dpnidCharacter);

		AgpdPartyADChar	*pcsAttachData	= pThis->m_pagpmParty->GetADCharacter(pcsParty->m_pcsMemberList[i]);

		pThis->m_pagpmParty->CalcPartyFactor(pcsParty->m_pcsMemberList[0], pcsParty->m_pcsMemberList[i], &pcsAttachData->m_csFactorPoint);

#ifdef	AGSMPARTY_UPDATE_PARTY_FACTOR
		if (pcsParty->m_bIsInEffectArea[i])
			pThis->m_pagsmCharacter->ReCalcCharacterFactors(pcsParty->m_pcsMemberList[i], TRUE);
#endif	//AGSMPARTY_UPDATE_PARTY_FACTOR
	}

	for (i = 0; i < pcsParty->m_nCurrentMember - 1; ++i)
	{
		pThis->SyncMemberHPMax(pcsParty, pcsParty->m_pcsMemberList[i], TRUE);
	}

*/
	return TRUE;
}

BOOL AgsmParty::CBRemoveParty(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmParty *pThis	= (AgsmParty *) pClass;
	AgpdParty *pcsParty	= (AgpdParty *) pData;

#ifdef	AGSMPARTY_UPDATE_PARTY_FACTOR
	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
	{
		AgpdCharacter	*pcsCharacter	= pThis->m_pagpmCharacter->GetCharacterLock(pcsParty->m_lMemberListID[i]);
		if (pcsCharacter)
		{
//	파티 추가능력치 제거 By SungHoon 2005.06.28
			pThis->RecalcPartyMemberHP(pcsParty, pcsCharacter, FALSE);
			pThis->m_pagsmSkill->RemoveSaveSkill(pcsCharacter, AGPMSKILL_RANGE2_TARGET_PARTY);
			pcsCharacter->m_Mutex.Release();
//			pThis->m_pagpmFactors->DestroyFactor(&pcsAttachData->m_csFactorPoint);

//			pThis->m_pagsmCharacter->ReCalcCharacterFactors(pcsParty->m_pcsMemberList[i], TRUE);
		}
	}
#endif	//AGSMPARTY_UPDATE_PARTY_FACTOR

	//AgsdParty *pcsAgsdParty = pThis->GetADParty(pcsParty);

	// 파티가 삭제 되었다는 정보를 먼저 파티원들에게 보낸다.
	if (pThis->SendPartyRemove(pcsParty))
		return FALSE;

	/*
	// 파티 DPNID Group을 삭제한다.
	pThis->m_pagsmAOIFilter->DestroyGroup(pcsAgsdParty->m_dpnidParty);

	pThis->m_pagsmAOIFilter->DestroyGroup(pcsAgsdParty->m_dpnidPartyServer);
	*/

	pThis->RemoveFarMemberView(pcsParty);		//	전체 파티를 검색해서 서로 멀리 있을 경우 시야에서 뺀다.  By SungHoon

	// Log 관련. Remove Party 할 때 멤버 돌면서 체크해준다.
	for(INT32 j = 0; j < pcsParty->m_nCurrentMember; j++)
	{
		AgpdCharacter	*pcsCharacter	= pThis->m_pagpmCharacter->GetCharacterLock(pcsParty->m_lMemberListID[j]);
		if (pcsCharacter)
		{
			pThis->m_pagsmCharacter->StopPartyPlay(pcsCharacter);
			pThis->m_pagsmSkill->RemoveSaveSkill(pcsCharacter, AGPMSKILL_RANGE2_TARGET_PARTY);
			pcsCharacter->m_Mutex.Release();
		}
	}

	return TRUE;
}

BOOL AgsmParty::CBRemovePartyID(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdParty		*pcsParty		= (AgpdParty *)		pData;

	pThis->m_csGenerateID.AddRemoveID(pcsParty->m_lID);

	return TRUE;
}

BOOL AgsmParty::CBAddMember(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgsmParty *pThis				= (AgsmParty *) pClass;
	AgpdParty *pcsParty				= (AgpdParty *) pData;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *) pCustData;
	AgsdCharacter *pcsAgsdCharacter	= pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);

	if (pcsParty->m_nCurrentMember < 2) return FALSE;

	// 추가된 멤버에게 이 파티 정보를 보낸다.
	pThis->SendPartyMemberInfo(pcsParty, pcsAgsdCharacter->m_dpnidCharacter, pcsCharacter->m_lID);
	if (!pThis->SendPartyInfo(pcsParty, pcsAgsdCharacter->m_dpnidCharacter))
		return FALSE;

	// 추가된 멤버에 대한 정보를 보낸다.
	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
	{
		AgpdCharacter	*pcsMember	= pThis->m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[i]);
		if (!pcsMember)
			continue;

		if (pcsCharacter != pcsMember)
			pThis->m_pagsmCharacter->SendPacketCharView(pcsCharacter, pThis->m_pagsmCharacter->GetCharDPNID(pcsMember), FALSE);
		pThis->SendMemberAdd(pcsParty, pcsCharacter, pThis->m_pagsmCharacter->GetCharDPNID(pcsMember));
	}

	pThis->RecalcPartyMemberHP(pcsParty, pcsCharacter, TRUE);

	pThis->SyncMemberHPMax(pcsParty, pcsCharacter, TRUE);

	// 새로 추가된 멤버 DPNID를 파티 그룹에 추가시킨다.
	//pThis->m_pagsmAOIFilter->AddPlayerToGroup(pcsAgsdParty->m_dpnidParty, pcsAgsdCharacter->m_dpnidCharacter);

	AgsdServer *pcsThisServer = pThis->m_pAgsmServerManager->GetThisServer();
	if (!pcsThisServer)
		return FALSE;

	// 이 캐릭터에서 컨트롤하는 캐릭터라면 다른 서버에도 멤버로 추가된 사실을 알려준다.
	/*
	if (pcsAgsdCharacter->m_ulServerID == pcsThisServer->m_lServerID)
		if (!pThis->SendMemberAdd(pcsParty, pcsCharacter, pcsAgsdParty->m_dpnidPartyServer, TRUE))
			return FALSE;
	*/

	pThis->m_pagsmCharacter->StartPartyPlay(pcsCharacter);

	return TRUE;
}

BOOL AgsmParty::CBRemoveMember(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgsmParty *pThis				= (AgsmParty *) pClass;
	AgpdParty *pcsParty				= (AgpdParty *) pData;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *) pCustData;

//	파티 추가능력치 제거 By SungHoon 2005.06.28
	pThis->RecalcPartyMemberHP(pcsParty, pcsCharacter, FALSE);
	pThis->m_pagsmSkill->RemoveSaveSkill(pcsCharacter, AGPMSKILL_RANGE2_TARGET_PARTY);

//	AgsdParty *pcsAgsdParty			= pThis->GetADParty(pcsParty);
	//AgsdCharacter *pcsAgsdCharacter	= pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);

	// 파티에서 나간 멤버에 대한 정보를 보낸다.
	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
	{
		pThis->SendMemberRemove(pcsParty, pcsCharacter, pThis->m_pagsmCharacter->GetCharDPNID(pcsParty->m_lMemberListID[i]));
	}

	pThis->SendMemberRemove(pcsParty, pcsCharacter, pThis->m_pagsmCharacter->GetCharDPNID(pcsCharacter));

	// 나간 멤버 DPNID를 파티 그룹에서 뺀다.
	//pThis->m_pagsmAOIFilter->RemovePlayerFromGroup(pcsAgsdParty->m_dpnidParty, pcsAgsdCharacter->m_dpnidCharacter);

	/*
	AgsdServer *pcsThisServer = pThis->m_pAgsmServerManager->GetThisServer();
	if (!pcsThisServer)
		return FALSE;

	// 이 캐릭터에서 컨트롤하는 캐릭터라면 다른 서버에도 파티에서 나간 사실을 알려준다.
	if (pcsAgsdCharacter->m_ulServerID == pcsThisServer->m_lServerID)
		if (!pThis->SendMemberRemove(pcsParty, pcsCharacter, pcsAgsdParty->m_dpnidPartyServer, TRUE))
			return FALSE;
	*/

	pThis->m_pagsmCharacter->StopPartyPlay(pcsCharacter);

	pThis->RemoveFarMemberView(pcsParty, pcsCharacter->m_lID);		//	파티에서 나간 멤버가 멀리 있을 경우 처리 By SungHoon
	return TRUE;
}

BOOL AgsmParty::CBChangeLeader(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmParty *pThis	= (AgsmParty *) pClass;
	AgpdParty *pcsParty	= (AgpdParty *) pData;

//	AgsdParty *pcsAgsdParty = pThis->GetADParty(pcsParty);

	AgsdServer *pcsThisServer = pThis->m_pAgsmServerManager->GetThisServer();
	if (!pcsThisServer)
		return FALSE;

	// 일단 파티원들의 factor를 업데이트한다. (이 서버에서 관리하는 넘만..)
	for (int i = 0; i < pcsParty->m_nCurrentMember; i++)
	{
		if (pcsParty->m_lMemberListID[i] == AP_INVALID_CID)
			continue;

		AgpdCharacter	*pcsMember	= pThis->m_pagpmCharacter->GetCharacterLock(pcsParty->m_lMemberListID[i]);
		if (!pcsMember)
			continue;

		AgsdCharacter	*pcsAgsdMember = pThis->m_pagsmCharacter->GetADCharacter(pcsMember);
		if (!pcsAgsdMember || pcsAgsdMember->m_ulServerID != pcsThisServer->m_lServerID)
		{
			pcsMember->m_Mutex.Release();
			continue;
		}

//	2005.06.28 By SungHoon 파티 팩터 다시 계산
		pThis->RecalcPartyMemberHP(pcsParty,pcsMember, TRUE);

		pcsMember->m_Mutex.Release();
	}

	// 새로운 리더가 이서버에서 관리하는 넘이면 다른 서버들로 변경된 파티 펙터를 보내준다.
//	if (pcsParty->m_pcsMemberList[0])
//	{
//		AgsdCharacter *pcsAgsdLeader = pThis->m_pagsmCharacter->GetADCharacter(pcsParty->m_pcsMemberList[0]);
//		if (pcsAgsdLeader)
//		{
//			if (pcsAgsdLeader->m_ulServerID == pcsThisServer->m_lServerID)
//			{
//				PVOID pvPacketFactor = pThis->m_pagsmFactors->MakePacketSyncParty(&pcsParty->m_csFactorPoint);
//				if (pvPacketFactor)
//				{
//					pThis->m_pagsmCharacter->SendPacketFactor(pvPacketFactor, pcsParty->m_pcsMemberList[0], pcsAgsdParty->m_dpnidPartyServer, TRUE);
//
//					pThis->m_csPacket.FreePacket(pvPacketFactor);
//				}
//			}
//		}
//	}

	return TRUE;
}

BOOL AgsmParty::CBUpdateActionStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	AgpdPartyADChar	*pcsPartyADChar = pThis->m_pagpmParty->GetADCharacter(pcsCharacter);

	// 파티에 들어있는 넘인지 검사한다.
	if (pcsPartyADChar->lPID == AP_INVALID_PARTYID)
		return FALSE;

	// 파티 데이타가 세팅 안되어 있다면 세팅하고...
	if (!pcsPartyADChar->pcsParty)
		pcsPartyADChar->pcsParty = pThis->m_pagpmParty->GetParty(pcsPartyADChar->lPID);

	if (!pcsPartyADChar->pcsParty)
		return FALSE;

//	AgsdParty *pcsAgsdParty = pThis->GetADParty(pcsPartyADChar->pcsParty);

	/*
	// 같은 서버내에 있는 파트원들에게 보내준다.
	pThis->m_pagsmCharacter->SendPacketCharStatus(pcsCharacter, pcsAgsdParty->m_dpnidParty);
	*/

	/*
	AgsdCharacter	*pcsAgsdCharacter = pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);

	AgsdServer *pcsThisServer = pThis->m_pAgsmServerManager->GetThisServer();
	if (pcsThisServer)
	{
		// 이서버가 컨트롤하는 넘인지 살펴본다.
		// 이서버가 컨트롤하는 넘이면 다른 서버들에 있는 파티원들에게 싱크를 맞춰줄지를 결정하고
		// 컨트롤 하는 넘이 아니라면 걍 넘어간다.
		if (pcsAgsdCharacter->m_ulServerID == pcsThisServer->m_lServerID)
		{
			// 싱크 맞춰준다. 즉, 파티원이 있는 다른 서버들에게 알려준다.
			pThis->m_pagsmCharacter->SendPacketCharStatus(pcsCharacter, pcsAgsdParty->m_dpnidPartyServer, TRUE);
		}
	}
	*/

	return TRUE;
}

/*
BOOL AgsmParty::CBMoveSector(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	ApWorldSector	*pcsSector		= (ApWorldSector *)	pData;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pCustData;

	AgpdPartyADChar	*pcsPartyADChar = pThis->m_pagpmParty->GetADCharacter(pcsCharacter);

	// 파티에 들어있는 넘인지 검사한다.
	if (pcsPartyADChar && pcsPartyADChar->lPID == AP_INVALID_PARTYID)
		return TRUE;

	if (pcsPartyADChar && !pcsPartyADChar->pcsParty)
		pcsPartyADChar->pcsParty = pThis->m_pagpmParty->GetParty(pcsPartyADChar->lPID);

	if (pcsPartyADChar && pcsPartyADChar->pcsParty == NULL)
		return FALSE;

	AgsdParty	*pcsAgsdParty	= pThis->GetADParty(pcsPartyADChar->pcsParty);
	if (!pcsAgsdParty)
		return FALSE;

	AgsdCharacter	*pcsAgsdCharacter = pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);

	AgsdServer *pcsThisServer = pThis->m_pagsmServerManager->GetThisServer();
	if (pcsThisServer)
	{
		// 이서버가 컨트롤하는 넘인지 살펴본다.
		// 이서버가 컨트롤하는 넘이면 다른 서버들에 있는 파티원들에게 싱크를 맞춰줄지를 결정하고
		// 컨트롤 하는 넘이 아니라면 걍 넘어간다.
		if (pcsAgsdCharacter->m_ulServerID == pcsThisServer->m_lServerID)
		{
			// 싱크 맞춰준다. 즉, 파티원이 있는 다른 서버들에게 알려준다.
			pThis->m_pagsmCharacter->SendPacketCharPosition(pcsCharacter, pcsAgsdParty->m_dpnidPartyServer, TRUE);
		}
	}

	return TRUE;
}
*/

BOOL AgsmParty::CBMoveCell(PVOID pData, PVOID pClass, PVOID pCustData)
{
	/*
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	ApWorldSector	*pcsSector		= (ApWorldSector *)	pData;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pCustData;

	AgpdPartyADChar	*pcsPartyADChar = pThis->m_pagpmParty->GetADCharacter(pcsCharacter);

	// 파티에 들어있는 넘인지 검사한다.
	if (pcsPartyADChar && pcsPartyADChar->lPID == AP_INVALID_PARTYID)
		return TRUE;

	if (pcsPartyADChar && !pcsPartyADChar->pcsParty)
		pcsPartyADChar->pcsParty = pThis->m_pagpmParty->GetParty(pcsPartyADChar->lPID);

	if (pcsPartyADChar && pcsPartyADChar->pcsParty == NULL)
		return FALSE;

	AgsdParty	*pcsAgsdParty	= pThis->GetADParty(pcsPartyADChar->pcsParty);
	if (!pcsAgsdParty)
		return FALSE;

	AgsdCharacter	*pcsAgsdCharacter = pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);

	AgsdServer *pcsThisServer = pThis->m_pagsmServerManager->GetThisServer();
	if (pcsThisServer)
	{
		// 이서버가 컨트롤하는 넘인지 살펴본다.
		// 이서버가 컨트롤하는 넘이면 다른 서버들에 있는 파티원들에게 싱크를 맞춰줄지를 결정하고
		// 컨트롤 하는 넘이 아니라면 걍 넘어간다.
		if (pcsAgsdCharacter->m_ulServerID == pcsThisServer->m_lServerID)
		{
			// 싱크 맞춰준다. 즉, 파티원이 있는 다른 서버들에게 알려준다.
			pThis->m_pagsmCharacter->SendPacketCharPosition(pcsCharacter, pcsAgsdParty->m_dpnidPartyServer, TRUE);
		}
	}
	*/

	return TRUE;
}

BOOL AgsmParty::CBRegionChange(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty*		pThis			= (AgsmParty*)pClass;
	AgpdCharacter*	pcsCharacter	= (AgpdCharacter*)pData;
	INT16 nPrevRegionIndex			= *(INT16*)pCustData;	// NULL 로 올때는 ADD_CHARACTER_TO_MAP 에서 불린것임.
	
	AgpdParty	*pcsParty	= pThis->m_pagpmParty->GetParty(pcsCharacter);
	if (!pcsParty)
		return FALSE;
	
	BOOL bPrevRegion = pThis->m_pagpmBattleGround->IsBattleGround(nPrevRegionIndex);
	BOOL bCurrRegion = pThis->m_pagpmBattleGround->IsBattleGround(pcsCharacter->m_nBindingRegionIndex);

	if( (bPrevRegion && !bCurrRegion) || (!bPrevRegion && bCurrRegion))
		return pThis->ProcessRemoveMember(pcsCharacter->m_lID);

	bPrevRegion = pThis->m_pagpmEpicZone->IsEpicZone(nPrevRegionIndex);
	bCurrRegion = pThis->m_pagpmEpicZone->IsEpicZone(pcsCharacter->m_nBindingRegionIndex);

	if((bPrevRegion && !bCurrRegion) || (!bPrevRegion && bCurrRegion))
		return pThis->ProcessRemoveMember(pcsCharacter->m_lID);

	// Check RegionPeculiarity
	if(pThis->m_papmMap->CheckRegionPerculiarity(pcsCharacter->m_nBindingRegionIndex, APMMAP_PECULIARITY_PARTY) == APMMAP_PECULIARITY_RETURN_DISABLE_USE)
	{
		return pThis->ProcessRemoveMember(pcsCharacter->m_lID);
	}

	return FALSE;
}

BOOL AgsmParty::CBSyncCharacterActions(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	PVOID			*pvBuffer		= (PVOID *)			pCustData;

	if (pcsCharacter->m_bSync)
	{
		AgpdParty	*pcsParty	= pThis->m_pagpmParty->GetParty(pcsCharacter);
		if (!pcsParty)
			return FALSE;

		return pThis->SendPacketToPartyExceptOne(pcsParty, pvBuffer[0], (INT16) pvBuffer[1], pcsCharacter->m_lID, PACKET_PRIORITY_4);
	}
	else
	{
		return pThis->SendPacketToFarPartyMember(pcsCharacter, pvBuffer[0], (INT16) pvBuffer[1], PACKET_PRIORITY_4);
	}

	return TRUE;
}

BOOL AgsmParty::CBSyncCharacterRemove(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdParty		*pcsParty		= (AgpdParty *)		pData;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pCustData;

//	AgsdParty		*pcsAgsdParty	= pThis->GetADParty(pcsParty);

	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
		pThis->m_pagsmCharacter->SendPacketCharRemove(pcsCharacter->m_lID, pThis->m_pagsmCharacter->GetCharDPNID(pcsParty->m_lMemberListID[i]), FALSE);

	return TRUE;

	/*
	INT16			nPacketLength	= 0;
	PVOID			pvPacket		= pThis->m_pagsmCharacter->MakePacketCharRemove(pcsCharacter->m_lID, &nPacketLength);
	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL			bSendResult		= pThis->SendPacketToFarPartyMember(pcsCharacter, pvPacket, nPacketLength);

	pThis->m_pagpmCharacter->m_csPacket.FreePacket(pvPacket);

	return			bSendResult;
	*/
}

BOOL AgsmParty::CBUpdateFactorParty(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdFactor		*pcsFactor		= (AgpdFactor *)	pData;

	AgpdFactorOwner	*pcsFactorOwner	= (AgpdFactorOwner *) pThis->m_pagpmFactors->GetFactor(pcsFactor, AGPD_FACTORS_TYPE_OWNER);
	if (!pcsFactorOwner || pcsFactorOwner->lValue[AGPD_FACTORS_OWNER_TYPE_ID] == AP_INVALID_CID || !pcsFactorOwner->lValue[AGPD_FACTORS_OWNER_TYPE_OWNER])
		return TRUE;

	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pcsFactorOwner->lValue[AGPD_FACTORS_OWNER_TYPE_OWNER];

	if(!pThis->m_pagpmCharacter->GetCharacter(pcsCharacter->m_szID))
		return FALSE;

	AgpdPartyADChar	*pcsPartyADChar = pThis->m_pagpmParty->GetADCharacter(pcsCharacter);

	if(!pcsPartyADChar)
		return FALSE;

	// 파티에 들어있는 넘인지 검사한다.
	if (pcsPartyADChar->lPID == AP_INVALID_PARTYID)
		return FALSE;

	// 파티 데이타가 세팅 안되어 있다면 세팅하고...
	if (!pcsPartyADChar->pcsParty)
		pcsPartyADChar->pcsParty = pThis->m_pagpmParty->GetParty(pcsPartyADChar->lPID);

	if (!pcsPartyADChar->pcsParty)
		return FALSE;

	pThis->SyncMemberHP(pcsPartyADChar->pcsParty, pcsCharacter, FALSE);

	/*
	AgsdParty *pcsAgsdParty = pThis->GetADParty(pcsPartyADChar->pcsParty);

	// 파티원간 싱크를 맞출 패킷 데이타를 얻어온다.
	PVOID pvPacketFactor = pThis->m_pagsmFactors->MakePacketSyncParty(&pcsCharacter->m_csFactor);

	if (pvPacketFactor)
	{
		// 파티원들에게 보낸다.
		pThis->m_pagsmCharacter->SendPacketFactor(pvPacketFactor, pcsCharacter, pcsAgsdParty->m_dpnidParty);

		AgsdCharacter	*pcsAgsdCharacter = pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);

		AgsdServer *pcsThisServer = pThis->m_pagsmServerManager->GetThisServer();
		if (pcsThisServer)
		{
			// 이서버가 컨트롤하는 넘인지 살펴본다.
			// 이서버가 컨트롤하는 넘이면 다른 서버들에 있는 파티원들에게 싱크를 맞춰줄지를 결정하고
			// 컨트롤 하는 넘이 아니라면 걍 넘어간다.
			if (pcsAgsdCharacter->m_ulServerID == pcsThisServer->m_lServerID)
			{
				// 싱크 맞춰준다. 즉, 파티원이 있는 다른 서버들에게 알려준다.
				pThis->m_pagsmCharacter->SendPacketFactor(pvPacketFactor, pcsCharacter, pcsAgsdParty->m_dpnidPartyServer, TRUE);
			}
		}

		pThis->m_pagpmCharacter->m_csPacket.FreePacket(pvPacketFactor);
	}
	*/

	return TRUE;
}

BOOL AgsmParty::CBZoningPassControl(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	// 조닝하는경우 파티 정보를 조닝하는 서버에 보낼필요가 있는거 검사
	// 필요에 따라 파티 정보를 보낸다. (파티 서버 그룹에 추가한다.)

	AgsmParty			*pThis			= (AgsmParty *)				pClass;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)			pData;
	AgsdServer			*pcsServer		= (AgsdServer *)			pCustData;

	AgsdCharacter		*pcsAgsdServer	= pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);

	AgpdPartyADChar	*pcsPartyADChar = pThis->m_pagpmParty->GetADCharacter(pcsCharacter);

	if (pcsPartyADChar->lPID == AP_INVALID_PARTYID)
		return FALSE;			// 파티원이 아니다. 가입되어 있는 파티가 없다.

	if (!pcsPartyADChar->pcsParty)
		pcsPartyADChar->pcsParty = pThis->m_pagpmParty->GetParty(pcsPartyADChar->lPID);

	if (!pcsPartyADChar->pcsParty)
		return FALSE;

	/*
	AgpdParty	*pcsParty = pcsPartyADChar->pcsParty;

	// pcsServer에 파티 멤버가 몇명이나 있는지 가져온다.
	INT16	nZoneServerMember = pThis->GetPartyMemberInServer(pcsParty, pcsServer->m_lServerID);
	if (nZoneServerMember < 0)
		return FALSE;

	if (nZoneServerMember == 0)		// 이 멤버가 처음으로 pcsServer에 조닝했다. 고로... 파티 데이타를 보내준다.
	{
//		AgsdParty	*pcsAgsdParty = pThis->GetADParty(pcsParty);

//		pThis->m_pagsmAOIFilter->AddPlayerToGroup(pcsAgsdParty->m_dpnidPartyServer, pcsServer->m_dpnidServer);

		if (!pThis->SendPartyInfo(pcsParty, pcsServer->m_dpnidServer, FALSE))
			return FALSE;
	}
	*/

	return TRUE;
}

BOOL AgsmParty::CBUpdateControlServer(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty		*pThis					= (AgsmParty *)		pClass;
	AgpdCharacter	*pcsCharacter			= (AgpdCharacter *)	pData;
	INT32			*plPrevControlServer	= (INT32 *)			pCustData;

	AgpdPartyADChar	*pcsAgpdPartyADChar		= pThis->m_pagpmParty->GetADCharacter(pcsCharacter);

	// 파티에 가입한 사람인지 검사한다.
	if (pcsAgpdPartyADChar->lPID == AP_INVALID_PARTYID)
		return TRUE;

	if (!pcsAgpdPartyADChar->pcsParty)
		pcsAgpdPartyADChar->pcsParty = pThis->m_pagpmParty->GetParty(pcsAgpdPartyADChar->lPID);

	if (!pcsAgpdPartyADChar->pcsParty)
		return FALSE;

	AgpdParty	*pcsParty = pcsAgpdPartyADChar->pcsParty;

	AgsdCharacter	*pcsAgsdCharacter = pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);

	AgsdServer	*pcsServer = pThis->m_pAgsmServerManager->GetServer(pcsAgsdCharacter->m_ulServerID);
	if (!pcsServer)
		return FALSE;

	/*
	// 새로 조닝해 들어간 서버에 대한 그룹핑 세팅을 한다.
	INT16	nNumMember = pThis->GetPartyMemberInServer(pcsParty, pcsAgsdCharacter->m_ulServerID);
	if (nNumMember < 0)
		return FALSE;

	if (nNumMember == 1)	// 이 멤버가 처음으로 pcsServer에 조닝했다. 고로... 파티 서버 그룹핑을 한다.
	{
		AgsdParty	*pcsAgsdParty = pThis->GetADParty(pcsParty);

		pThis->m_pagsmAOIFilter->AddPlayerToGroup(pcsAgsdParty->m_dpnidPartyServer, pcsServer->m_dpnidServer);
	}

	// 이전 컨트롤 서버에 대한 그룹핑 세팅을 한다.
	nNumMember = pThis->GetPartyMemberInServer(pcsParty, *plPrevControlServer);
	if (nNumMember < 0)
		return FALSE;

	if (nNumMember == 0)	// 이 멤버 말고는 더이상 남아있는 멤버가 없다. 고로.. 그룹에서 뺀다.
	{
		AgsdParty	*pcsAgsdParty = pThis->GetADParty(pcsParty);

		pThis->m_pagsmAOIFilter->RemovePlayerFromGroup(pcsAgsdParty->m_dpnidPartyServer, pcsServer->m_dpnidServer);
	}
	*/

	return TRUE;
}

BOOL AgsmParty::CBReCalcFactor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	AgpdParty		*pcsParty		= pThis->m_pagpmParty->GetParty(pcsCharacter);
	if (pcsParty)
	{
		int i = 0;
		for (i = 0; i < AGPMPARTY_MAX_PARTY_MEMBER; ++i)
		{
			if (pcsParty->m_lMemberListID[i] == pcsCharacter->m_lID)
				break;
		}

		if (i == AGPMPARTY_MAX_PARTY_MEMBER)
			return TRUE;

		if (pcsParty->m_bIsInEffectArea[i])
		{
			AgpdPartyADChar	*pcsAttachData	= pThis->m_pagpmParty->GetADCharacter(pcsCharacter);

			pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPoint, &pcsAttachData->m_csFactorPoint, TRUE, FALSE, TRUE, FALSE);
		}
	}

	return TRUE;
}

BOOL AgsmParty::CBResetEffectFactor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdParty		*pcsParty		= (AgpdParty *)		pData;
	BOOL			*pbOldList		= (BOOL *)			pCustData;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= pThis->m_pagpmParty->MakePacketEffectArea(pcsParty, pbOldList, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	//AgsdParty		*pcsAgsdParty	= pThis->GetADParty(pcsParty);

	BOOL	bSendResult	= TRUE;
	
	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
		bSendResult &= pThis->SendPacket(pvPacket, nPacketLength, pThis->m_pagsmCharacter->GetCharDPNID(pcsParty->m_lMemberListID[i]), PACKET_PRIORITY_4);

	pThis->m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	return	bSendResult;
}

BOOL AgsmParty::CBUpdateExpType(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdParty		*pcsParty		= (AgpdParty *)		pData;
	INT32			*pMemberID		= (INT32	 *)		pCustData;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= pThis->m_pagpmParty->MakePacketExpDivision(pcsParty, *pMemberID, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	//AgsdParty		*pcsAgsdParty	= pThis->GetADParty(pcsParty);

	BOOL	bSendResult	= TRUE;
	
	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
		bSendResult &= pThis->SendPacket(pvPacket, nPacketLength, pThis->m_pagsmCharacter->GetCharDPNID(pcsParty->m_lMemberListID[i]), PACKET_PRIORITY_4);

	pThis->m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	return	bSendResult;
}

BOOL AgsmParty::CBUpdatePartyFactor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	PVOID			*ppvBuffer		= (PVOID *)			pCustData;

	/*
	if (!ppvBuffer[0] ||
		!ppvBuffer[1] ||
		!ppvBuffer[2] ||
		!ppvBuffer[3] ||
		!ppvBuffer[4] ||
		!ppvBuffer[5] ||
		!ppvBuffer[6] ||
		!ppvBuffer[7])
		return FALSE;
	*/

	INT32	lOriginalDamage		= PtrToInt(ppvBuffer[0]);
	INT32	lOriginalDefense	= PtrToInt(ppvBuffer[1]);
	INT32	lOriginalMaxHP		= PtrToInt(ppvBuffer[2]);
	INT32	lOriginalMaxMP		= PtrToInt(ppvBuffer[3]);

	INT32	lNewDamage			= PtrToInt(ppvBuffer[4]);
	INT32	lNewDefense			= PtrToInt(ppvBuffer[5]);
	INT32	lNewMaxHP			= PtrToInt(ppvBuffer[6]);
	INT32	lNewMaxMP			= PtrToInt(ppvBuffer[7]);

/*	if (lNewDamage > 0 && lNewDamage != lOriginalDamage ||
		lNewDefense > 0 && lNewDefense != lOriginalDefense ||
		lNewMaxHP > 0 && lNewMaxHP != lOriginalMaxHP ||
		lNewMaxMP > 0 && lNewMaxMP != lOriginalMaxMP)
	{
		// 패킷을 보내야 한다.
		return pThis->SendPartyBonusStats(pcsCharacter);
	}

	return TRUE;
*/
	return pThis->SendPartyBonusStats(pcsCharacter);
}

BOOL AgsmParty::CBRemoveCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	AgpdPartyADChar	*pcsPartyADChar = pThis->m_pagpmParty->GetADCharacter(pcsCharacter);

	if (pcsPartyADChar->lPID == AP_INVALID_PARTYID)
		return TRUE;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBRemoveCharacter"));

	if (!pcsPartyADChar->pcsParty)
		pcsPartyADChar->pcsParty = pThis->m_pagpmParty->GetParty(pcsPartyADChar->lPID);

	if (!pcsPartyADChar->pcsParty)
		return FALSE;

	//AgsdParty		*pcsAgsdParty	= pThis->GetADParty(pcsPartyADChar->pcsParty);

	for (int i = 0; i < pcsPartyADChar->pcsParty->m_nCurrentMember; ++i)
		pThis->m_pagsmCharacter->SendPacketCharRemove(pcsCharacter, pThis->m_pagsmCharacter->GetCharDPNID(pcsPartyADChar->pcsParty->m_lMemberListID[i]), FALSE);

	return TRUE;
}

/*
	2005.04.19	By SungHoon
	아이템 분배 방식이 변경 요청을 파티장이 요구했을 경우 처리한다.
*/
BOOL AgsmParty::CBUpdateItemDivision(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdParty		*pcsParty		= (AgpdParty *)		pData;
	INT32			*pMemberID		= (INT32	 *)		pCustData;

	if (pcsParty == NULL) return FALSE;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= pThis->m_pagpmParty->MakePacketItemDivision(pcsParty, *pMemberID, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	//AgsdParty		*pcsAgsdParty	= pThis->GetADParty(pcsParty);

	BOOL	bSendResult	= TRUE;

	pThis->SendPacketToParty(pcsParty, pvPacket, nPacketLength, PACKET_PRIORITY_4);

	pThis->m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	return	bSendResult;
}

/*
	2005.05.02	By SungHoon
	파티장 위임을 요청했을 경우 불리워진다.
*/
BOOL AgsmParty::DelegationLeader(UINT32 ulNID, INT32 lOperator, INT32 lTarget )
{
	AgpdCharacter *pcsOperator = m_pagpmCharacter->GetCharacterLock(lOperator);
	if(!pcsOperator)
		return FALSE;

	AgpdParty *pcsParty = m_pagpmParty->GetPartyLock(pcsOperator);
	if (!pcsParty)
	{
		pcsOperator->m_Mutex.Release();
		return FALSE;
	}

	if (lOperator != m_pagpmParty->GetLeaderCID(pcsParty))
	{
		SendDelegationLeaderFailed(lOperator, ulNID);
		pcsOperator->m_Mutex.Release();
		pcsParty->m_Mutex.Release();
		return FALSE;
	}
	pcsOperator->m_Mutex.Release();
	if (m_pagpmParty->DelegationLeader(pcsParty, lOperator, lTarget) == TRUE)
	{
		INT16	nPacketLength	= 0;
		PVOID	pvPacket		= MakePacketDelegationLeader(lOperator, lTarget, &nPacketLength);
		if (!pvPacket || nPacketLength < 1)
		{
			pcsParty->m_Mutex.Release();
			return FALSE;
		}
		SendPacketToParty( pcsParty, pvPacket, nPacketLength, PACKET_PRIORITY_4);
		m_pagpmParty->m_csPacket.FreePacket(pvPacket);
	}
	pcsParty->m_Mutex.Release();

	return TRUE;
}
//	Finish 2005.05.02	By Sunghoon

BOOL AgsmParty::SendPacketToParty(INT32 lPartyID, PVOID pvPacket, INT16 nPacketLength, PACKET_PRIORITY ePriority)
{
	return SendPacketToParty(m_pagpmParty->GetParty(lPartyID), pvPacket, nPacketLength, ePriority);
}

BOOL AgsmParty::SendPacketToParty(AgpdParty *pcsParty, PVOID pvPacket, INT16 nPacketLength, PACKET_PRIORITY ePriority)
{
	if (!pcsParty || !pvPacket || !nPacketLength)
		return FALSE;

	//AgsdParty	*pcsAgsdParty	= GetADParty(pcsParty);

	//if (pcsAgsdParty)
	{
		for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
			SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsParty->m_lMemberListID[i]), ePriority);
		//m_pagsmAOIFilter->SendPacketGroup(pvPacket, nPacketLength, pcsAgsdParty->m_dpnidPartyServer, ePriority);
	}

	return TRUE;
}

BOOL AgsmParty::SendPacketToParty(AgpdCharacter *pcsCharacter, PVOID pvPacket, INT16 nPacketLength, PACKET_PRIORITY ePriority)
{
	if (!pcsCharacter || !pvPacket || !nPacketLength)
		return FALSE;

	AgpdPartyADChar *pcsPartyADChar = m_pagpmParty->GetADCharacter(pcsCharacter);

	if (pcsPartyADChar)
	{
		if (!pcsPartyADChar->pcsParty)
			pcsPartyADChar->pcsParty = m_pagpmParty->GetParty(pcsPartyADChar->lPID);

		return SendPacketToParty(pcsPartyADChar->pcsParty, pvPacket, nPacketLength, ePriority);
	}

	return FALSE;
}

BOOL AgsmParty::SendPacketToPartyExceptOne(AgpdParty *pcsParty, PVOID pvPacket, INT16 nPacketLength, INT32 lExceptMemberID, PACKET_PRIORITY ePriority)
{
	if (!pcsParty || !pvPacket || nPacketLength < 1 || lExceptMemberID == AP_INVALID_CID)
		return FALSE;

	BOOL	bSendResult	= TRUE;

	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
	{
		if (pcsParty->m_lMemberListID[i] == lExceptMemberID)
			continue;

		bSendResult	&= SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsParty->m_lMemberListID[i]), ePriority);
	}

	return bSendResult;
}

BOOL AgsmParty::SendPacketToNearPartyMember(AgpdCharacter *pcsCharacter, PVOID pvPacket, INT16 nPacketLength, PACKET_PRIORITY ePriority)
{
	if (!pcsCharacter || !pvPacket || nPacketLength < 1)
		return FALSE;

	// 현재는 같은 서버 내에 있는 넘들한테 보내는 방법밖에 없다. ㅡ.ㅡ 이걸 그룹을 다시 더 지정해야 하나... ㅡ.ㅡ

	// 파티 가입 여부를 따져서 파티한테 보내던지 걍 저넘한테 보내던지 한다.

	AgpdParty	*pcsParty	= m_pagpmParty->GetParty(pcsCharacter);

	if (pcsParty)
	{
		//AgsdParty	*pcsAgsdParty	= GetADParty(pcsParty);

		for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
			SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsParty->m_lMemberListID[i]), ePriority);

		return TRUE;
	}
	else
	{
		return SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsCharacter), ePriority);
	}

	return TRUE;
}

BOOL AgsmParty::SendPacketToFarPartyMember(AgpdCharacter *pcsCharacter, PVOID pvPacket, INT16 nPacketLength, PACKET_PRIORITY ePriority)
{
	if (!pcsCharacter || !pvPacket || nPacketLength < 1)
		return FALSE;

	ApSafeArray<INT32, AGPMPARTY_MAX_PARTY_MEMBER>		alMemberListBuffer;
	alMemberListBuffer.MemSetAll();

	AgpdParty	*pcsParty	= m_pagpmParty->GetPartyLock(pcsCharacter);
	if (!pcsParty)
		return FALSE;

	INT32	nCurrentMember	= pcsParty->m_nCurrentMember;
	alMemberListBuffer.MemCopy(0, &pcsParty->m_lMemberListID[0], pcsParty->m_nCurrentMember);

	pcsParty->m_Mutex.Release();

	for (int i = 0; i < nCurrentMember; ++i)
	{
		AgpdCharacter	*pcsMember	= m_pagpmCharacter->GetCharacterLock(alMemberListBuffer[i]);
		if (!pcsMember)
			continue;

		INT32	lDistance = CheckValidDistance(pcsMember, pcsCharacter);

		if (lDistance > 1)		// 거리가 2 이상이면 시야내에 없는거다.
			SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsMember), ePriority);

		pcsMember->m_Mutex.Release();
	}

	return TRUE;
}

BOOL AgsmParty::SendPartyMemberInfo(AgpdParty *pcsParty, DPNID dpnid, INT32 lExceptMemberID)
{
	if (!pcsParty || dpnid == 0)
		return FALSE;

	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
	{
		if (pcsParty->m_lMemberListID[i] == lExceptMemberID)
			continue;

		m_pagsmCharacter->SendPacketCharView(m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[i]), dpnid, FALSE);
	}

	return TRUE;
}

BOOL AgsmParty::SendPartyInfo(AgpdParty *pcsParty, DPNID dpnid)
{
	if (!pcsParty)
		return FALSE;

	INT16	nPacketLength = 0;
	PVOID	pvPacket = m_pagpmParty->MakePacketParty(pcsParty, &nPacketLength);

	if (!pvPacket)
		return FALSE;

	BOOL	bRetval = TRUE;

	if (!SendPacket(pvPacket, nPacketLength, dpnid, PACKET_PRIORITY_2))
		bRetval = FALSE;

	m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	return bRetval;
}

BOOL AgsmParty::SendPartyInfo(AgpdParty *pcsParty)
{
	if (!pcsParty)
		return FALSE;

	INT16	nPacketLength = 0;
	PVOID	pvPacket = m_pagpmParty->MakePacketParty(pcsParty, &nPacketLength);

	if (!pvPacket)
		return FALSE;

	BOOL	bRetval = TRUE;

	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
	{
		if (!SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsParty->m_lMemberListID[i]), PACKET_PRIORITY_2))
			bRetval = FALSE;
	}

	m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	return bRetval;
}

BOOL AgsmParty::SendPartyRemove(AgpdParty *pcsParty)
{
	if (!pcsParty)
		return FALSE;

	INT16	nPacketLength = 0;
	PVOID	pvPacket = m_pagpmParty->MakePacketRemoveParty(pcsParty->m_lID, &nPacketLength);

	if (!pvPacket)
		return FALSE;

	BOOL	bRetval = TRUE;

	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
	{
		if (!SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsParty->m_lMemberListID[i]), PACKET_PRIORITY_2))
			bRetval = FALSE;
	}

	m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	return bRetval;
}

BOOL AgsmParty::SendMemberAdd(AgpdParty *pcsParty, AgpdCharacter *pcsCharacter, DPNID dpnid)
{
	if (!pcsParty)
		return FALSE;

	BOOL	bIsInEffectArea	= FALSE;

	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
	{
		if (pcsParty->m_lMemberListID[i] == pcsCharacter->m_lID)
		{
			bIsInEffectArea	= pcsParty->m_bIsInEffectArea[i];
			break;
		}
	}

	INT16	nPacketLength = 0;
	PVOID	pvPacket = m_pagpmParty->MakePacketAddMember(pcsParty, pcsCharacter->m_lID, &nPacketLength);

	if (!pvPacket)
		return FALSE;

	BOOL	bRetval = SendPacket(pvPacket, nPacketLength, dpnid, PACKET_PRIORITY_2);

	m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	return bRetval;
}

BOOL AgsmParty::SendMemberRemove(AgpdParty *pcsParty, AgpdCharacter *pcsCharacter, DPNID dpnid)
{
	if (!pcsParty)
		return FALSE;

	INT16	nPacketLength = 0;
	PVOID	pvPacket = m_pagpmParty->MakePacketRemoveMember(pcsParty->m_lID, pcsCharacter->m_lID, &nPacketLength);

	if (!pvPacket)
		return FALSE;

	BOOL	bRetval = SendPacket(pvPacket, nPacketLength, dpnid, PACKET_PRIORITY_2);

	m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	return bRetval;
}

BOOL AgsmParty::SyncMemberHP(AgpdParty *pcsParty, AgpdCharacter *pcsCharacter, BOOL bIsMemberAdd)
{
	if (!pcsParty || !pcsCharacter)
		return FALSE;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= m_pagpmParty->MakePacketSyncMemberHP(pcsCharacter, &nPacketLength);
	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL	bSendResult	= SendPacketToPartyExceptOne(pcsParty, pvPacket, nPacketLength, pcsCharacter->m_lID, PACKET_PRIORITY_4);

	m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	if (bIsMemberAdd)
	{
		for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
		{
			if (pcsParty->m_lMemberListID[i] == pcsCharacter->m_lID)
				continue;

			pvPacket	= m_pagpmParty->MakePacketSyncMemberHP(m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[i]), &nPacketLength);
			if (!pvPacket || nPacketLength < 1)
				continue;

			SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsCharacter));

			m_pagpmParty->m_csPacket.FreePacket(pvPacket);
		}
	}

	return bSendResult;
}

BOOL AgsmParty::SyncMemberHPMax(AgpdParty *pcsParty, AgpdCharacter *pcsCharacter, BOOL bIsMemberAdd)
{
	if (!pcsParty || !pcsCharacter)
		return FALSE;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= m_pagpmParty->MakePacketSyncMemberHPMax(pcsCharacter, &nPacketLength);
	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL	bSendResult	= SendPacketToPartyExceptOne(pcsParty, pvPacket, nPacketLength, pcsCharacter->m_lID, PACKET_PRIORITY_4);

	m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	if (bIsMemberAdd)
	{
		for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
		{
			if (pcsParty->m_lMemberListID[i] == pcsCharacter->m_lID)
				continue;

			pvPacket	= m_pagpmParty->MakePacketSyncMemberHPMax(m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[i]), &nPacketLength);
			if (!pvPacket || nPacketLength < 1)
				continue;

			SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsCharacter));

			m_pagpmParty->m_csPacket.FreePacket(pvPacket);
		}
	}

	return bSendResult;
}

BOOL AgsmParty::SendPartyBonusStats(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= m_pagpmParty->MakePacketBonusStatus(pcsCharacter, &nPacketLength);
	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL	bSendResult		= SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsCharacter));

	m_pagpmParty->m_csPacket.FreePacket(pvPacket);

	return	bSendResult;
}

/*
BOOL AgsmParty::ConAgsdParty(PVOID pData, PVOID pClass, PVOID pCustData)
{
	AgsmParty *pThis = (AgsmParty *) pClass;

	AgsdParty *pcsAgsdParty = pThis->GetADParty((AgpdParty *) pData);

	pcsAgsdParty->m_dpnidParty			= ASDP_INVALID_DPNID;
	pcsAgsdParty->m_dpnidPartyServer	= ASDP_INVALID_DPNID;

	return TRUE;
}

BOOL AgsmParty::DesAgsdParty(PVOID pData, PVOID pClass, PVOID pCustData)
{
	return TRUE;
}
*/

INT16 AgsmParty::GetPartyMemberInServer(AgpdParty *pcsParty, INT32 lServerID)
{
	if (!pcsParty || lServerID == AP_INVALID_SERVERID)
		return (-1);

	INT16	nMember = 0;
	for (int i = 0; i < pcsParty->m_nCurrentMember; i++)
	{
		AgpdCharacter	*pcsMember = m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[i]);
		if (!pcsMember)
			continue;

		AgsdCharacter	*pcsAgsdMember = m_pagsmCharacter->GetADCharacter(pcsMember);
		if (!pcsAgsdMember)
			continue;

		if (pcsAgsdMember->m_ulServerID == lServerID)
			nMember++;
	}

	return nMember;
}

BOOL AgsmParty::CBAddParty(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmParty	*pThis		= (AgsmParty *) pClass;
	AgpdParty	*pcsParty	= (AgpdParty *) pData;

	// 파티가 새로 추가되었다. (패킷으로 받았다.)
	// 이 경우엔 기존 파티 멤버들이 있는 서버에 대한 그룹핑을 한다.

//	AgsdParty	*pcsAgsdParty	= pThis->GetADParty(pcsParty);
//	if (!pcsAgsdParty)
//		return FALSE;

	AgsdServer	*pcsThisServer = pThis->m_pAgsmServerManager->GetThisServer();
	if (!pcsThisServer)
		return FALSE;

	for (int i = 0; i < pcsParty->m_nCurrentMember; i++)
	{
		AgpdCharacter	*pcsMember	= pThis->m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[i]);
		if (!pcsMember)
			continue;

		AgsdCharacter	*pcsAgsdMember = pThis->m_pagsmCharacter->GetADCharacter(pcsMember);
		if (!pcsAgsdMember)
			continue;

		if (pcsThisServer->m_lServerID == pcsAgsdMember->m_ulServerID)
			continue;

		AgsdServer		*pcsServer = pThis->m_pAgsmServerManager->GetServer(pcsAgsdMember->m_ulServerID);
		if (!pcsServer)
			continue;

		//pThis->m_pagsmAOIFilter->AddPlayerToGroup(pcsAgsdParty->m_dpnidPartyServer, pcsServer->m_dpnidServer);
	}

	return TRUE;
}

BOOL AgsmParty::CBUpdateFactor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmParty	*pThis		= (AgsmParty *) pClass;
	AgpdParty	*pcsParty	= (AgpdParty *)	pData;

	AgsdServer	*pcsThisServer = pThis->m_pAgsmServerManager->GetThisServer();
	if (!pcsThisServer)
		return FALSE;

	// 업데이트 펙터 패킷을 받았다. 이서버에서 관리하는 멤버들 펙터를 다쉬 계산해준다.
	for (int i = 0; i < pcsParty->m_nCurrentMember; i++)
	{
		AgpdCharacter	*pcsMember	= pThis->m_pagpmCharacter->GetCharacterLock(pcsParty->m_lMemberListID[i]);
		if (!pcsMember)
			continue;

		AgsdCharacter	*pcsAgsdMember = pThis->m_pagsmCharacter->GetADCharacter(pcsMember);
		if (!pcsAgsdMember)
		{
			pcsMember->m_Mutex.Release();
			continue;
		}

#ifdef	AGSMPARTY_UPDATE_PARTY_FACTOR
		if (pcsAgsdMember->m_ulServerID == pcsThisServer->m_lServerID)
			pThis->m_pagsmCharacter->ReCalcCharacterFactors(pcsMember);
#endif	//AGSMPARTY_UPDATE_PARTY_FACTOR

		pcsMember->m_Mutex.Release();
	}

	return TRUE;
}

// 2004.07,22, steeple
BOOL AgsmParty::CBCharLevelUp(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdParty* pcsParty = (AgpdParty*)pData;
	AgsmParty* pThis = (AgsmParty*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pCustData;
	
//	AgsdParty *pcsAgsdParty = pThis->GetADParty(pcsParty);

	AgsdServer *pcsThisServer = pThis->m_pAgsmServerManager->GetThisServer();
	if (!pcsThisServer)
		return FALSE;

	INT16 nPacketLength	=	0;
	PVOID pvPacket		=	pThis->m_pagpmCharacter->MakePacketCharLevel(pcsCharacter, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	pThis->SendPacketToPartyExceptOne(pcsParty, pvPacket, nPacketLength, pcsCharacter->m_lID, PACKET_PRIORITY_4);
	pThis->m_pagpmCharacter->m_csPacket.FreePacket(pvPacket);


	// Level Up 한 사람이 마스터가 아니면 factor 업데이트 없음		2005.06.15 By SungHoon
	if(pcsParty->m_lMemberListID[0] != pcsCharacter->m_lID) return FALSE;	

	// 일단 파티원들의 factor를 업데이트한다. (이 서버에서 관리하는 넘만..)
	for (int i = 0; i < pcsParty->m_nCurrentMember; i++)
	{
		AgpdCharacter	*pcsMember	= pThis->m_pagpmCharacter->GetCharacterLock(pcsParty->m_lMemberListID[i]);
		if (!pcsMember)
			continue;

		AgsdCharacter	*pcsAgsdMember = pThis->m_pagsmCharacter->GetADCharacter(pcsMember);
		if (!pcsAgsdMember || pcsAgsdMember->m_ulServerID != pcsThisServer->m_lServerID)
		{
			pcsMember->m_Mutex.Release();
			continue;
		}

//	2005.06.28 By SungHoon 파티 팩터 다시 계산
		pThis->RecalcPartyMemberHP(pcsParty, pcsMember, TRUE);

		pcsMember->m_Mutex.Release();
	}

//	// 새로운 리더가 이서버에서 관리하는 넘이면 다른 서버들로 변경된 파티 펙터를 보내준다.
//	if (pcsParty->m_pcsMemberList[0])
//	{
//		AgsdCharacter *pcsAgsdLeader = pThis->m_pagsmCharacter->GetADCharacter(pcsParty->m_pcsMemberList[0]);
//		if (pcsAgsdLeader)
//		{
//			if (pcsAgsdLeader->m_ulServerID == pcsThisServer->m_lServerID)
//			{
//				PVOID pvPacketFactor = pThis->m_pagsmFactors->MakePacketSyncParty(&pcsParty->m_csFactorPoint);
//				if (pvPacketFactor)
//				{
//					pThis->m_pagsmCharacter->SendPacketFactor(pvPacketFactor, pcsParty->m_pcsMemberList[0], pcsAgsdParty->m_dpnidPartyServer, TRUE);
//
//					pThis->m_csPacket.FreePacket(pvPacketFactor);
//				}
//			}
//		}
//	}

	return TRUE;
}

INT32 AgsmParty::CheckValidDistance(AgpdCharacter *pcsPartyMember, AgpdCharacter *pcsBaseCharacter)
{
	if (!pcsPartyMember || !pcsBaseCharacter)
		return 100;

	AgsdCharacter	*pcsAgsdPartyMember		= m_pagsmCharacter->GetADCharacter(pcsPartyMember);
	AgsdCharacter	*pcsAgsdBaseCharacter	= m_pagsmCharacter->GetADCharacter(pcsBaseCharacter);

	/*
	INT32	lSubX	= abs(pcsAgsdPartyMember->m_pCurrentSector->GetIndexX() - pcsAgsdBaseCharacter->m_pCurrentSector->GetIndexX());
	INT32	lSubZ	= abs(pcsAgsdPartyMember->m_pCurrentSector->GetIndexZ() - pcsAgsdBaseCharacter->m_pCurrentSector->GetIndexZ());
	*/

	if (!pcsAgsdPartyMember->m_pcsCurrentCell ||
		!pcsAgsdBaseCharacter->m_pcsCurrentCell)
		return 100;

	INT32	lSubX	= abs(pcsAgsdPartyMember->m_pcsCurrentCell->GetIndexX() - pcsAgsdBaseCharacter->m_pcsCurrentCell->GetIndexX());
	INT32	lSubZ	= abs(pcsAgsdPartyMember->m_pcsCurrentCell->GetIndexZ() - pcsAgsdBaseCharacter->m_pcsCurrentCell->GetIndexZ());

	if (lSubX > lSubZ)
		return lSubX;

	return lSubZ;
}

//INT32 AgsmParty::GetNearMember(AgpdParty *pcsParty, AgpdCharacter **ppcsMemberBuffer, INT32 *plMemberTotalLevel)
//{
//	if (!pcsParty)
//		return 0;
//
//	*plMemberTotalLevel		= 0;
//
//	INT32	lNumNearMember	= 0;
//
//	// 시야내에 있는 파티원만 골라낸다.
//	// 기준위치는 pcsBase다.
//
//	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
//	{
//		if (!pcsParty->m_bIsInEffectArea[i])
//			continue;
//
//		// 시야내에 들어와있는 넘이다. 음.....
//
//		// 혹시 죽은넘이 아닌지 본다.
//		// 죽은넘은 암것도 안한다.
//
//		if (pcsParty->m_pcsMemberList[i]->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
//			continue;
//
//		if (ppcsMemberBuffer)
//		{
//			ppcsMemberBuffer[lNumNearMember] = pcsParty->m_pcsMemberList[i];
//		}
//
//		if (plMemberTotalLevel)
//		{
//			*plMemberTotalLevel += m_pagpmFactors->GetLevel(&pcsParty->m_pcsMemberList[i]->m_csFactor);
//		}
//
//		++lNumNearMember;
//	}
//
//	return lNumNearMember;
//}

INT32 AgsmParty::GetNearMember(AgpdParty *pcsParty, AgpdCharacter *pcsBase, AgpdCharacter **ppcsMemberBuffer, INT32 *plMemberTotalLevel)
{
	if (!pcsParty ||
		!pcsBase)
		return 0;

	*plMemberTotalLevel		= 0;

	INT32	lNumNearMember	= 0;

	// 시야내에 있는 파티원만 골라낸다.
	// 기준위치는 pcsBase다.

	for (int i = 0; i < pcsParty->m_nCurrentMember; ++i)
	{
		AgpdCharacter	*pcsMember	= m_pagpmCharacter->GetCharacter(pcsParty->m_lMemberListID[i]);
		if (!pcsMember)
			continue;

		if (IsNearMember(pcsBase, pcsMember) == FALSE) continue;

		if (ppcsMemberBuffer)
		{
			ppcsMemberBuffer[lNumNearMember] = pcsMember;
		}

		if (plMemberTotalLevel)
		{
			*plMemberTotalLevel += m_pagpmCharacter->GetLevel( pcsMember );
		}

		++lNumNearMember;
	}

	return lNumNearMember;
}

INT32 AgsmParty::GetPartyBonusDamage(ApBase *pcsBase, AgpdSkill *pcsSkill)
{
	return 0;

//	if (!pcsBase || pcsBase->m_eType != APBASE_TYPE_CHARACTER)
//		return 0;
//
//	pcsBase->m_Mutex.Release();
//
//	AgpdParty	*pcsParty	= m_pagpmParty->GetPartyLock((AgpdCharacter *) pcsBase);
//	if (!pcsParty)
//	{
//		pcsBase->m_Mutex.WLock();
//		return 0;
//	}
//
//	FLOAT	fLeaderCharisma	= 0.0f;
//	INT32	lLeaderClass	= 0;
//
//	AgpdCharacter	*pcsCharacter = pcsParty->m_pcsMemberList[0];
//	if (!pcsCharacter)
//	{
//		pcsParty->m_Mutex.Release();
//		pcsBase->m_Mutex.WLock();
//		return 0;
//	}
//
//	if (!m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lLeaderClass, AGPD_FACTORS_TYPE_CHAR_TYPE, AGPD_FACTORS_CHARTYPE_TYPE_CLASS))
//	{
//		pcsParty->m_Mutex.Release();
//		pcsBase->m_Mutex.WLock();
//		return 0;
//	}
//
//	AgpdFactor	*pcsResultFactor = (AgpdFactor *) m_pagpmFactors->GetFactor(&pcsCharacter->m_csFactor, AGPD_FACTORS_TYPE_RESULT);
//	if (!pcsResultFactor)
//	{
//		pcsParty->m_Mutex.Release();
//		pcsBase->m_Mutex.WLock();
//		return 0;
//	}
//
//	if (!m_pagpmFactors->GetValue(pcsResultFactor, &fLeaderCharisma, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_CHA))
//	{
//		pcsParty->m_Mutex.Release();
//		pcsBase->m_Mutex.WLock();
//		return 0;
//	}
//
//	pcsParty->m_Mutex.Release();
//
//	pcsBase->m_Mutex.WLock();
//
//
//	switch (lLeaderClass) {
//	case AUCHARCLASS_TYPE_KNIGHT:
//		{
//			if (pcsSkill)
//				return 0;
//
//			return (INT32) (fLeaderCharisma / 5.0f);
//		}
//		break;
//
//	case AUCHARCLASS_TYPE_MAGE/* 마법사 */:
//		{
//			if (!pcsSkill)
//				return 0;
//
//			if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lCostType & AGPMSKILL_COST_MP)
//				return (INT32) (fLeaderCharisma / 10.0f);
//
//			return 0;
//		}
//		break;
//
//	case AUCHARCLASS_TYPE_RANGER/* 궁수 */:
//		{
//			if (!pcsSkill)
//				return 0;
//
//			return  (INT32) (fLeaderCharisma / 10.0f);
//		}
//		break;
//
//		/*
//	case AUCHARCLASS_TYPE_MONK:
//		{
//			return lCharisma;
//		}
//		break;
//		*/
//
//	default:
//		return 0;
//		break;
//	}
//
//	return 0;
}

/*
	2005.04.21	By SungHoon
	현재 파티 중이고 파티가 순차적 획득이라면 강제로 아이템을 이번순서 멤버에게 준다.
*/
INT32 AgsmParty::GetCurrentGetItemPartyMember( ApBase *pcsCharacter , ApBase *pcsDropbase, BOOL bTemp )
{
	if (!pcsCharacter) return AP_INVALID_CID;

	AgpdParty		*pcsAgpdParty	= NULL;
	AgpdCharacter	*pcsDropCharacter = NULL;

	if (pcsCharacter->m_eType == APBASE_TYPE_CHARACTER)		pcsAgpdParty = m_pagpmParty->GetParty(( AgpdCharacter *)pcsCharacter);
	if (pcsCharacter->m_eType == APBASE_TYPE_PARTY)			pcsAgpdParty = (AgpdParty *)pcsCharacter;
	if (pcsDropbase->m_eType == APBASE_TYPE_CHARACTER)		pcsDropCharacter = ( AgpdCharacter *)pcsDropbase;

	if (pcsAgpdParty == NULL || pcsDropCharacter == NULL) return AP_INVALID_CID;
	
	switch (pcsAgpdParty->m_eDivisionItem)
	{
	case AGPMPARTY_DIVISION_ITEM_SEQUENCE :
			{
				ApSafeArray<AgpdCharacter*, AGPMPARTY_MAX_PARTY_MEMBER>	pcsNearMember;
				pcsNearMember.MemSetAll();
				INT32 lMemberTotalLevel = 0;

				INT32 lNumNearMember = GetNearMember(pcsAgpdParty, pcsDropCharacter, &pcsNearMember[0], &lMemberTotalLevel);

				for (INT32 i = 0; i < pcsAgpdParty->m_nCurrentMember; i++)
				{
					INT32	lGetItemCID	=	m_pagpmParty->GetNextGetItemMember( pcsAgpdParty, bTemp );

					for (int j = 0;j < lNumNearMember; j++)
					{
						if (lGetItemCID == pcsNearMember[j]->m_lID)
							return lGetItemCID;
					}
				}
			}
			break;
	case AGPMPARTY_DIVISION_ITEM_DAMAGE :
			{
				AgsdCharacterHistoryEntry* pcsEntry = m_pagsmCharacter->GetMostDamagerInParty(pcsDropCharacter, pcsAgpdParty->m_lID);
				if(pcsEntry && pcsEntry->m_csSource.m_eType == APBASE_TYPE_CHARACTER)
					return pcsEntry->m_csSource.m_lID;
			}
			break;
	}

	return AP_INVALID_CID;
}

BOOL AgsmParty::CBTransform(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgsmParty		*pThis			= (AgsmParty *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	AgpdPartyADChar	*pcsAttachData	= pThis->m_pagpmParty->GetADCharacter(pcsCharacter);

	if (pcsAttachData->pcsParty)
	{
		if (pcsAttachData->pcsParty->m_lMemberListID[0] == pcsCharacter->m_lID)
		{
			// if pcsCharacter is party leader, must be recalculate bonus factor of party leader
			//////////////////////////////////////////////////////////////////////////

			AgpdParty	*pcsParty	= pcsAttachData->pcsParty;

			for (int i = 0; i < pcsParty->m_nCurrentMember; i++)
			{
				AgpdCharacter	*pcsMember	= pThis->m_pagpmCharacter->GetCharacterLock(pcsParty->m_lMemberListID[i]);
				if (!pcsMember)
					continue;

				AgsdCharacter	*pcsAgsdMember = pThis->m_pagsmCharacter->GetADCharacter(pcsMember);
				if (!pcsAgsdMember)
				{
					pcsMember->m_Mutex.Release();
					continue;
				}

//	2005.06.28 By SungHoon 파티 팩터 다시 계산
				pThis->RecalcPartyMemberHP(pcsParty, pcsMember, TRUE);

				pcsMember->m_Mutex.Release();
			}
		}
		else
		{
//	2005.06.28 By SungHoon 파티 팩터 다시 계산
			AgpdParty	*pcsParty	= pcsAttachData->pcsParty;
			pThis->RecalcPartyMemberHP(pcsParty,pcsCharacter, TRUE);
		}
	}

	return TRUE;
}

BOOL AgsmParty::CBGetBonusOfPartyMember(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty* pThis = static_cast<AgsmParty*>(pClass);
	INT32* plRate = static_cast<INT32*>(pData);
	AgpdDropItemInfo2* pcsDropInfo = static_cast<AgpdDropItemInfo2*>(pCustData);

	INT32 lCID = pThis->GetCurrentGetItemPartyMember(pcsDropInfo->m_pcsFirstLooter, pcsDropInfo->m_pcsDropCharacter, TRUE);
	AgpdCharacter* pcsCharacter = pThis->m_pagpmCharacter->GetCharacter(lCID);
	if(pcsCharacter)
	{
		*plRate += pThis->m_pagpmCharacter->GetGameBonusDropRate(pcsCharacter);
	}

	return TRUE;
}

/*
	2005.05.25.	Bt SungHoon
	파티가 없어질 경우 파티에 소속된 모든 멤버를 검색해 서로 제거한다.
	1...2
	1...3  2...3 
	1...4  2...4  3...4
	.....
	서로간 검색이 다 된다.
*/
BOOL AgsmParty::RemoveFarMemberView(AgpdParty *pcsParty)
{
	if(pcsParty->m_nCurrentMember < 2) return FALSE;
	for (int i = 0; i < pcsParty->m_nCurrentMember - 1; ++i)
	{
		if (pcsParty->m_lMemberListID[i] != AP_INVALID_CID)
			RemoveFarMemberView(pcsParty, pcsParty->m_lMemberListID[i], i + 1);
	}
	return TRUE;

}

/*
	2005.05.25.	Bt SungHoon
	한 멤버가 나갈 경우 다른 파티 멤버와 비교해 멀리 있을 경우 제거한다.
*/
BOOL AgsmParty::RemoveFarMemberView(AgpdParty *pcsParty, INT32 lCID, int nStartPos )
{
	ApAutoLockCharacter LockChecker(m_pagpmCharacter, lCID);
	AgpdCharacter *pcsCharacter = LockChecker.GetCharacterLock();
	if (!pcsCharacter) return FALSE;

	DPNID CharacterDPNID = m_pagsmCharacter->GetCharDPNID(pcsCharacter);
	for (int i = nStartPos; i < pcsParty->m_nCurrentMember; ++i)
	{
		if(lCID == pcsParty->m_lMemberListID[i] || pcsParty->m_lMemberListID[i] == AP_INVALID_CID) continue;	//	같은 유저는 처리하지 않는다.

		ApAutoLockCharacter Lock(m_pagpmCharacter, pcsParty->m_lMemberListID[i]);
		AgpdCharacter	*pcsMember	= Lock.GetCharacterLock();
		if (!pcsMember || pcsMember == pcsCharacter) continue;

		INT32	lDistance = CheckValidDistance(pcsMember, pcsCharacter);
		if (lDistance > 2)		// 거리가 2 이상이면 시야내에 없는거다.
		{
			m_pagsmCharacter->SendPacketCharRemoveForView(pcsMember, CharacterDPNID, FALSE);
			m_pagsmCharacter->SendPacketCharRemoveForView(pcsCharacter, m_pagsmCharacter->GetCharDPNID(pcsMember), FALSE);
		}
	}
	return TRUE;
}

/*
	2005.06.07. By SungHoon
	pcsTarget이 pcsOperator로부터 해당 거리안에 유효한(죽지않은) 상태로 있는지 확인한다.
*/
BOOL AgsmParty::IsNearMember(AgpdCharacter *pcsOperator, AgpdCharacter *pcsTarget, FLOAT lDistance /*= 8000 */)
{
	AuPOS	stDelta;
	// 임시로 Y 축 검사를 하지 않는다. 이유는 DestinationPos의 Y축 값이 땅 밑에 있는경우 맵에서 그리로 내려가지 못하므로 무한히 목표까지 도달하지 못하고 이동한다.
	////////////////////////////////////////////////////////////////////////
	// stDelta는 현재 위치에서 목표 위치까지의 Vector
	AuMath::V3DSubtract(&stDelta, &pcsTarget->m_stPos, &pcsOperator->m_stPos);
	stDelta.y = 0;

	// fDiv는 그 거리
	FLOAT fDiv = AuMath::V3DLength(&stDelta);

	// 50M 안에 있는지 체크한다.
	if (fDiv > lDistance) return FALSE;

/*
	INT32	lDistance = CheckValidDistance(pcsParty->m_pcsMemberList[i], pcsBase);

	if (lDistance > 1)		// 거리가 2 이상이면 시야내에 없는거다.
		continue;
*/

	// 시야내에 들어와있는 넘이다. 음.....

	// 혹시 죽은넘이 아닌지 본다.
	// 죽은넘은 암것도 안한다.

	if (pcsTarget->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return FALSE;

	/*
	// 마을에 있는 놈도 뺀다.

	INT16	nBindingRegion	= m_papmMap->GetRegion(pcsTarget->m_stPos.x , pcsTarget->m_stPos.z);

	ApmMap::RegionTemplate	*pcsRegionTemplate	= m_papmMap->GetTemplate(nBindingRegion);
	if (pcsRegionTemplate &&
		pcsRegionTemplate->ti.stType.uSafetyType == ApmMap::ST_SAFE)
		return FALSE;
	*/

	return TRUE;
}


/*
	2005.06.29 By SungHoon
	파티팩터 삭제후 재계산 필요하면 업데이트 후 다시 계산하라고 함.(ㅡㅡ)
*/
BOOL AgsmParty::RecalcPartyMemberHP(AgpdParty *pcsParty, AgpdCharacter *pcsCharacter, BOOL bRecalc)
{
	if (!pcsParty || !pcsCharacter)
		return FALSE;

	AgpdPartyADChar	*pcsAttachData	= m_pagpmParty->GetADCharacter(pcsCharacter);
	m_pagpmFactors->DestroyFactor(&pcsAttachData->m_csFactorPoint);

	AgpdCharacter	*pcsLeader	= m_pagpmCharacter->GetCharacterLock(pcsParty->m_lMemberListID[0]);
	if (pcsLeader)
	{
		if (bRecalc)
			m_pagpmParty->CalcPartyFactor(pcsLeader, pcsCharacter, &pcsAttachData->m_csFactorPoint);

		pcsLeader->m_Mutex.Release();
	}

#ifdef	AGSMPARTY_UPDATE_PARTY_FACTOR
	m_pagsmCharacter->ReCalcCharacterFactors(pcsCharacter);
#endif	//AGSMPARTY_UPDATE_PARTY_FACTOR

	// 2005.06.30. steeple
	// 행여나 현재 HP 가 Max 를 넘어간 놈이 있을지도 모르니 다시 체크해본다.
	// 결국에 이렇게 할 걸 가지고 조낸 삽질했3
	INT32 lMaxHP = 0, lMaxMP = 0, lMaxSP = 0;
	INT32 lCurrentHP = 0, lCurrentMP = 0, lCurrentSP = 0;

	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lMaxHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_HP);
	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lMaxMP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_MP);
	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lMaxSP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_SP);

	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lCurrentHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);
	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lCurrentMP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_MP);
	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lCurrentSP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_SP);

	if(lCurrentHP > lMaxHP)
		lCurrentHP = lMaxHP;
	if(lCurrentMP > lMaxMP)
		lCurrentMP = lMaxMP;
	if(lCurrentSP > lMaxSP)
		lCurrentSP = lMaxSP;

	PVOID pvPacket = NULL;
	m_pagsmFactors->UpdateCharPoint(&pcsCharacter->m_csFactor, &pvPacket, lCurrentHP, lCurrentMP, lCurrentSP);
	if(pvPacket)
	{
		m_pagsmCharacter->SendPacketFactor(pvPacket, pcsCharacter, PACKET_PRIORITY_4);
		m_pagsmCharacter->m_csPacket.FreePacket(pvPacket);
	}

	return TRUE;
}

BOOL AgsmParty::RecallPartyMember(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdPartyADChar	*pcsAttachData	= m_pagpmParty->GetADCharacter(pcsCharacter);

	if (!pcsAttachData->pcsParty)
		return FALSE;

	ApmMap::RegionTemplate *pcsRegionTemplate = m_papmMap->GetTemplate(pcsCharacter->m_nBindingRegionIndex);
	if(NULL == pcsRegionTemplate)
		return FALSE;

	// 사용자가 감옥에 있거나 상담소에 있으면 사용할 수 없다.
	if(m_pagpmCharacter->IsInJail(pcsCharacter) || m_pagpmCharacter->IsInConsulationOffice(pcsCharacter) ||
	   pcsRegionTemplate->ti.stType.bRecallItem)
		return FALSE;

	// 비밀의 던젼 안, 렌스피어 지역에서는 사용 못한다.
	if(m_pagpmSiegeWar->IsInSecretDungeon(pcsCharacter))
		return FALSE;

	BOOL bResult = FALSE;

	for (int i = 0; i < pcsAttachData->pcsParty->m_nCurrentMember; ++i)
	{
		if (AgpdCharacter *pcsMember = m_pagpmCharacter->GetCharacterLock(pcsAttachData->pcsParty->m_lMemberListID[i]))
		{
			if (pcsMember != pcsCharacter) // 본인은 처리 안한다
			{
				// 2008.04.08. steeple
				// 변신중인 놈을 레벨제한 지역으로 부르지 못한다.
				// 탈것을 탄 상태로 레벨제한 지역으로 부르지 못한다.
				if(pcsMember->m_bIsTrasform && m_pagpmCharacter->GetCurrentRegionLevelLimit(pcsCharacter) != 0 ||
				   m_pagpmCharacter->IsRideOn(pcsMember) && m_pagpmCharacter->GetCurrentRegionLevelLimit(pcsCharacter) != 0)
				{
					pcsMember->m_Mutex.Release();
					continue;
				}

				//////////////////////////////////////////////////////////////////////////
				//
				AuPOS stTeleportPos = pcsMember->m_stPos;
				AuPOS stCharacterPos = pcsCharacter->m_stPos;

				if( m_papmMap->GetTargetPositionLevelLimit( stTeleportPos ) 
					&& m_papmMap->GetTargetPositionLevelLimit( stCharacterPos ) )
				{
					/*if(m_pagsmSystemMessage)
						m_pagsmSystemMessage->SendSystemMessage( pcsMember , AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS );*/
					
					pcsMember->m_Mutex.Release();
					continue;
				}

				// 해당 캐릭터가 비밀의 던젼에 있으면 안된다.
				if (pcsMember->m_unActionStatus != AGPDCHAR_STATUS_DEAD &&
					!m_pagpmSiegeWar->IsInSecretDungeon(pcsMember)
					&& pcsMember->m_nBindingRegionIndex != 81	// 렌스피어
					&& pcsMember->m_nBindingRegionIndex != 88	// 렌스피어
					&& !m_pagpmCharacter->IsInJail(pcsMember)	// 감옥
					&& !m_pagpmCharacter->IsInConsulationOffice(pcsMember))	//상담소
				{
					// 해당 캐릭터가 공성지역안에 있어도 안된다.
					AgpdSiegeWar* pcsSiegeWar = m_pagpmSiegeWar->GetSiegeWarInfo(pcsMember);
					if( !(pcsSiegeWar && m_pagpmSiegeWar->IsStarted(pcsSiegeWar)) )
					{
						SendPacketRequestRecall(pcsCharacter->m_lID, m_pagsmCharacter->GetCharDPNID(pcsMember));
						bResult = TRUE;
					}
				}
			}

			pcsMember->m_Mutex.Release();
		}
	}

	return bResult;
}

BOOL AgsmParty::OnReceiveAcceptRecall(INT32 lOperator, INT32 lTarget)
{
	AgpdCharacter* pcsCharacter = m_pagpmCharacter->GetCharacter(lOperator);
	if (!pcsCharacter)
		return FALSE;

	AuAutoLock pLock(pcsCharacter->m_Mutex);
	if(!pLock.Result())
		return FALSE;
	
	if (pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return FALSE;

	if (m_pagpmCharacter->IsAllBlockStatus(pcsCharacter))
		return FALSE;

	AgsdCharacter *pagsdCharacter = m_pagsmCharacter->GetADCharacter(pcsCharacter);
	if (NULL == pagsdCharacter)
		return FALSE;

	AgpdCharacter* pcsTarget = m_pagpmCharacter->GetCharacter(lTarget);
	if (!pcsTarget)
		return FALSE;

	AuAutoLock pLock2(pcsTarget->m_Mutex);
	if(!pLock2.Result())
		return FALSE;
	
	AgpdPartyADChar	*pcsOperatorAttachData	= m_pagpmParty->GetADCharacter(pcsCharacter);
	AgpdPartyADChar	*pcsTargetAttachData	= m_pagpmParty->GetADCharacter(pcsTarget);

	if(!pcsOperatorAttachData || !pcsTargetAttachData)
		return FALSE;

	if (pcsOperatorAttachData->lPID == pcsTargetAttachData->lPID)
	{
		AuPOS stTeleportPos = pcsTarget->m_stPos;
		AuPOS stCharacterPos = pcsCharacter->m_stPos;

		ApmMap::RegionTemplate	* pTemplateTarget		= m_papmMap->GetTemplate( m_papmMap->GetRegion( stTeleportPos.x , stTeleportPos.z ) );
		ApmMap::RegionTemplate	* pTemplateCharacter	= m_papmMap->GetTemplate( m_papmMap->GetRegion( stCharacterPos.x , stCharacterPos.z ) );

		if (pTemplateTarget && pTemplateCharacter)
		{
			// 사용할 수 없는 지역이라면 사용 못한다.
			if (pTemplateTarget->ti.stType.bRecallItem || pTemplateCharacter->ti.stType.bRecallItem)
			{
				if(m_pagsmSystemMessage)
					m_pagsmSystemMessage->SendSystemMessage( pcsCharacter , AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS );

				return FALSE;
			}
		}

		//JK_소환 신청후 카오틱으로 이동한 경우를 위해 수락할때 변신과 카오틱 여부 확인 
		if(pcsCharacter->m_bIsTrasform &&  m_papmMap->GetTargetPositionLevelLimit(stTeleportPos))
		{
			if(m_pagsmSystemMessage)
				m_pagsmSystemMessage->SendSystemMessage( pcsCharacter , AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS );
			return FALSE;
		}

		if( m_papmMap->GetTargetPositionLevelLimit( stTeleportPos ) 
			&& m_papmMap->GetTargetPositionLevelLimit( stCharacterPos ) )
		{
			if(m_pagsmSystemMessage)
				m_pagsmSystemMessage->SendSystemMessage( pcsCharacter , AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS );
			
			return FALSE;
		}

		if( m_pagpmCharacter->IsCombatMode(pcsCharacter) 
			|| (pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_MOVE)
			|| (pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_ATTACK)
			|| (pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_TRADE)
			|| (m_pagpmCharacter->IsActionBlockCondition(pcsCharacter))
			)
		{
			if(m_pagsmSystemMessage)
				m_pagsmSystemMessage->SendSystemMessage( pcsCharacter , AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS );

			return FALSE;
		}

		SendPacketAcceptRecall(lOperator, lTarget, m_pagsmCharacter->GetCharDPNID(pcsTarget));

		m_pagpmCharacter->SetActionBlockTime(pcsCharacter, 3000);
		pagsdCharacter->m_bIsTeleportBlock = TRUE;

		m_pagpmCharacter->StopCharacter(pcsCharacter, NULL);
		m_pagpmCharacter->UpdatePosition(pcsCharacter, &pcsTarget->m_stPos, FALSE, TRUE);
		m_pagpmSummons->UpdateAllSummonsPosToOwner(pcsCharacter);

		pagsdCharacter->m_bIsTeleportBlock = FALSE;
	}

	return TRUE;
}

BOOL AgsmParty::OnReceiveRejectRecall(INT32 lOperator, INT32 lTarget)
{
	AgpdCharacter	*pcsOperator	= m_pagpmCharacter->GetCharacterLock(lOperator);
	if (!pcsOperator)
		return FALSE;

	AgpdCharacter	*pcsTarget		= m_pagpmCharacter->GetCharacterLock(lTarget);
	if (!pcsTarget)
		return FALSE;

	AgpdPartyADChar	*pcsOperatorAttachData	= m_pagpmParty->GetADCharacter(pcsOperator);
	AgpdPartyADChar	*pcsTargetAttachData	= m_pagpmParty->GetADCharacter(pcsTarget);

	if (pcsOperatorAttachData->lPID == pcsTargetAttachData->lPID)
	{
		SendPacketRejectRecall(lOperator, lTarget, m_pagsmCharacter->GetCharDPNID(pcsTarget));
	}

	pcsOperator->m_Mutex.Release();
	pcsTarget->m_Mutex.Release();

	return TRUE;
}

BOOL AgsmParty::SetCallbackUseRecall(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMPARTY_CB_USE_RECALL, pfCallback, pClass);
}

BOOL AgsmParty::SetCallbackBuddyCheck(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMPARTY_CB_BUDDY_CHECK, pfCallback, pClass);
}

/*
	2005.10.24. By SungHoon
	Muderer Point 가 변할 경우 불리어서 상황에 따라 파티를 해체한다.
*/
BOOL AgsmParty::CBUpdateMurdererPoint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmParty	*pThis				= (AgsmParty *) pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	INT32			lDiff			= *((INT32 *) pCustData);

	INT32 lMurdererLevel = pThis->m_pagpmCharacter->GetMurdererLevel(pcsCharacter);
	if (lMurdererLevel >= AGPMCHAR_MURDERER_LEVEL1_POINT)		//	파티 탈퇴
	{
		AgpdParty *pcsParty = pThis->m_pagpmParty->GetPartyLock(pcsCharacter);
		if (!pcsParty) return FALSE;

		AgsdCharacter *pcsAgsdTargetChar = pThis->m_pagsmCharacter->GetADCharacter(pcsCharacter);
		pThis->SendPacketInviteFailed(pcsCharacter->m_lID,
								NULL,
								AGSMPARTY_PACKET_OPERATION_LEAVE_BY_MURDERER,
								pcsAgsdTargetChar->m_dpnidCharacter);

		BOOL	bDestroyParty = FALSE;

		pThis->m_pagpmParty->RemoveMember(pcsParty, pcsCharacter->m_lID, &bDestroyParty);

		if (!bDestroyParty)
			pcsParty->m_Mutex.Release();
	}
	return TRUE;
}

BOOL AgsmParty::CBPreCheckEnableJanusTalisman( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdItem* pcsItem = static_cast<AgpdItem*>(pData);
	AgsmParty* pThis = static_cast<AgsmParty*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pCustData);

	AgpdParty* pcsParty = pThis->m_pagpmParty->GetParty(pcsCharacter);
	if (pcsParty)
	{
		// 야누스의 부적은 파티가 있을때엔 사용할수 없다
		pThis->m_pagsmSystemMessage->SendSystemMessage(pThis->m_pagsmCharacter->GetCharDPNID(pcsCharacter),
			AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS);
		return FALSE;
	}

	return TRUE;
}

