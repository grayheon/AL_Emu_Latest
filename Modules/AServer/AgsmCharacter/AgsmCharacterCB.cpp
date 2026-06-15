#include "AgsmCharacter.h"
#include "AgpmArchlord.h"

#include "AgsmCogpas.h"
#include "AgsmGameholic.h"
#include "AgpmBattleGround.h"
#include "AgppCharacter.h"
#include "AgspCharacter.h"

BOOL AgsmCharacter::CBAddCell(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter		*pThis				= (AgsmCharacter *) pClass;
	AgsmAOICell			*pcsCell			= (AgsmAOICell *)	pData;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)	pCustData;

	AgsdCharacter		*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);
	if (!pcsAgsdCharacter)
		return FALSE;

	pcsAgsdCharacter->m_pcsPrevCell			= NULL;
	pcsAgsdCharacter->m_pcsCurrentCell		= pcsCell;
	ZeroMemory(&pcsAgsdCharacter->m_stPrevPos, sizeof(pcsAgsdCharacter->m_stPrevPos));

	return TRUE;
}

BOOL AgsmCharacter::CBRemoveCell(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBRemoveCell");

	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter		*pThis			= (AgsmCharacter *) pClass;
	AgsmAOICell			*pcsCell		= (AgsmAOICell *)	pData;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)	pCustData;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBRemoveCell"));

	AgsdCharacter		*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);

	pcsAgsdCharacter->m_pcsPrevCell		= NULL;
	pcsAgsdCharacter->m_pcsCurrentCell	= NULL;
	ZeroMemory(&pcsAgsdCharacter->m_stPrevPos, sizeof(pcsAgsdCharacter->m_stPrevPos));

	// game world 에서 나가는 상황이다. character와 연관된 모든 데이타를 DB에 저장한다.
	//
	//

	if (pThis->m_pcsAgpmCharacter->IsPC(pcsCharacter) &&
		pThis->m_pAgsmServerManager2->GetThisServerType() == AGSMSERVER_TYPE_GAME_SERVER &&
		!pcsAgsdCharacter->m_bIsNotLogout)
	{
		pThis->BackupCharacterData(pcsCharacter);
		pThis->EnumCallback(AGSMCHARACTER_CB_UPDATE_ALL_TO_DB, pcsCharacter, NULL);

		pThis->EnumCallback(AGSMCHARACTER_CB_EXIT_GAME_WORLD, pcsCharacter, NULL);
	}

	return TRUE;
}

BOOL AgsmCharacter::CBMoveCell(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter		*pThis		= (AgsmCharacter *) pClass;
	AgpdCharacter		*pCharacter	= (AgpdCharacter *)	pCustData;
	PVOID				*ppvBuffer	= (PVOID *)			pData;

	AgsmAOICell			*pcsNewCell	= (AgsmAOICell *)	ppvBuffer[0];
	AgsmAOICell			*pcsOldCell	= (AgsmAOICell *)	ppvBuffer[1];
	AuPOS				*pcsPrevPos	= (AuPOS *)			ppvBuffer[2];

	AgsdCharacter	*pcsAgsdCharacter = pThis->GetADCharacter(pCharacter);
	if (!pcsAgsdCharacter)
		return FALSE;

	pcsAgsdCharacter->m_bMoveCell = TRUE;
	pcsAgsdCharacter->m_pcsPrevCell = pcsOldCell;
	pcsAgsdCharacter->m_pcsCurrentCell = pcsNewCell;

	pcsAgsdCharacter->m_stPrevPos	= *pcsPrevPos;

	return TRUE;
}

BOOL AgsmCharacter::CBMoveCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBMoveCharacter");

	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter*	pThis				= (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter	= (AgpdCharacter *) pData;
	AgsdCharacter*	pcsAgsdCharacter	= pThis->GetADCharacter(pcsAgpdCharacter);
	BOOL			bNoSendSelf			= (BOOL)pCustData;

	if (pcsAgsdCharacter->m_ulPrevSendMoveTime == 0)
		pcsAgsdCharacter->m_ulPrevSendMoveTime = pThis->m_pcsAgpmCharacter->m_ulPrevClockCount;

	/*
	if (pcsAgpdCharacter->m_bMove && !pcsAgpdCharacter->m_bSync &&
		pcsAgsdCharacter->m_ulPrevSendMoveTime + pThis->m_ulSendMoveDelay > pThis->m_pcsAgpmCharacter->m_ulPrevClockCount)
		return TRUE;
	*/

	if (pcsAgpdCharacter->m_bMoveFollow)
	{
		if (pcsAgpdCharacter->m_ulSyncMoveTime + AGSMCHARACTER_SYNC_MOVE_FOLLOW_INTERVAL > pThis->GetClockCount())
			return TRUE;

		pcsAgpdCharacter->m_ulSyncMoveTime	= pThis->GetClockCount();
	}
	else
	{
		if (pcsAgpdCharacter->m_ulSyncMoveTime == 0)
			pcsAgpdCharacter->m_ulSyncMoveTime = pThis->GetClockCount();
	}

	pThis->SendPacketCharPosition(pcsAgpdCharacter, !bNoSendSelf );

	return TRUE;
}

BOOL AgsmCharacter::CBStopCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBStopCharacter");

	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter*	pThis				= (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter	= (AgpdCharacter *) pData;
	AgsdCharacter*	pcsAgsdCharacter	= pThis->GetADCharacter(pcsAgpdCharacter);
	BOOL			bNoSendSelf			= (BOOL)pCustData;

	//if (pCustData)
	//{
		pcsAgsdCharacter->m_ulPrevSendMoveTime = 0;

		if (pcsAgpdCharacter->m_bSync/* ||
			pcsAgpdCharacter->m_bWasMoveFollow*/)
			pThis->SendPacketCharPosition(pcsAgpdCharacter , !bNoSendSelf );
		//CBUpdatePosition(pData, pClass, pCustData);
	//}

	return TRUE;
}

BOOL AgsmCharacter::CBUpdatePosition(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBUpdatePosition");

	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;
	AgsdCharacter*	pcsAgsdCharacter = pThis->GetADCharacter(pcsAgpdCharacter);
	INT8			cOperation = AGPMCHAR_PACKET_OPERATION_UPDATE;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBUpdatePosition"));

	// 캐릭터가 움직이는 경우 일정 주기로 움직임 싱크를 맞춰준다.
// 	if (pcsAgpdCharacter->m_bMove && !pcsAgpdCharacter->m_bMoveFollow)
// 	{
// 		PROFILE("AgsmCharacter::SyncMoveInterval");
// 
// 		if (pcsAgpdCharacter->m_ulSyncMoveTime + AGSMCHARACTER_SYNC_MOVE_INTERVAL < pThis->GetClockCount())
// 		{
// 			pcsAgpdCharacter->m_ulSyncMoveTime	= pThis->GetClockCount();
// 			pThis->SendPacketCharPosition(pcsAgpdCharacter);
// 		}
// 	}
// 	else if (pcsAgpdCharacter->m_bSync)
	if( pcsAgpdCharacter->m_bSync )
	{
		PROFILE("AgsmCharacter::SyncMove");

		pThis->SendPacketCharPosition(pcsAgpdCharacter);
	}

	// 섹터를 옮기는 경우 처리를 한다.
	if (pcsAgsdCharacter->m_bMoveCell)
	{
		// 2006.07.05. steeple
		// 기본값은 기존의 코드에 있는 것을 그대로 사용하였다.
		const INT32 MAX_CELL_ROW_COUNT		= 128;
		const INT32 MAX_CELL_COL_COUNT		= MAX_CELL_ROW_COUNT;
		const INT32 MAX_CELL_COUNT			= MAX_CELL_ROW_COUNT * MAX_CELL_COL_COUNT;
		const INT32 DEFAULT_CHAR_RANGE		= 5;
		const INT32 DEFAULT_NPC_RANGE		= 9;

		// forward range
		INT32 FORWARD_NEAR_CELL_COUNT	= pThis->m_pagsmAOIFilter->GetNearCellCountByRegion(pThis->m_pcsAgpmCharacter->GetRealRegionIndex(pcsAgpdCharacter));
		INT32 FORWARD_CHAR_RANGE		= FORWARD_NEAR_CELL_COUNT * 2 + 1 > DEFAULT_CHAR_RANGE	? FORWARD_NEAR_CELL_COUNT * 2 + 1 : DEFAULT_CHAR_RANGE;
		INT32 FORWARD_NPC_RANGE			= FORWARD_NEAR_CELL_COUNT * 2 + 1 > DEFAULT_NPC_RANGE	? FORWARD_NEAR_CELL_COUNT * 2 + 1 : DEFAULT_NPC_RANGE;

		// 값 보정.
		if (FORWARD_CHAR_RANGE > MAX_CELL_ROW_COUNT)
			FORWARD_CHAR_RANGE = MAX_CELL_ROW_COUNT;
		if (FORWARD_NPC_RANGE > MAX_CELL_ROW_COUNT)
			FORWARD_NPC_RANGE = MAX_CELL_ROW_COUNT;

		INT32 FORWARD_AFFECTED_CELL_COUNT = FORWARD_CHAR_RANGE > FORWARD_NPC_RANGE ? FORWARD_CHAR_RANGE * FORWARD_CHAR_RANGE : FORWARD_NPC_RANGE * FORWARD_NPC_RANGE;

		// backward range
		INT32 BACKWARD_NEAR_CELL_COUNT	= pThis->m_pagsmAOIFilter->GetNearCellCountByRegion(pThis->m_papmMap->GetRegion(pcsAgsdCharacter->m_stPrevPos.x, pcsAgsdCharacter->m_stPrevPos.z));
		INT32 BACKWARD_CHAR_RANGE		= BACKWARD_NEAR_CELL_COUNT * 2 + 1 > DEFAULT_CHAR_RANGE	? BACKWARD_NEAR_CELL_COUNT * 2 + 1 : DEFAULT_CHAR_RANGE;
		INT32 BACKWARD_NPC_RANGE		= BACKWARD_NEAR_CELL_COUNT * 2 + 1 > DEFAULT_NPC_RANGE	? BACKWARD_NEAR_CELL_COUNT * 2 + 1 : DEFAULT_NPC_RANGE;

		// 값 보정.
		if (BACKWARD_CHAR_RANGE > MAX_CELL_ROW_COUNT)
			BACKWARD_CHAR_RANGE = MAX_CELL_ROW_COUNT;
		if (BACKWARD_NPC_RANGE > MAX_CELL_ROW_COUNT)
			BACKWARD_NPC_RANGE = MAX_CELL_ROW_COUNT;

		INT32 BACKWARD_AFFECTED_CELL_COUNT = BACKWARD_CHAR_RANGE > BACKWARD_NPC_RANGE ? BACKWARD_CHAR_RANGE * BACKWARD_CHAR_RANGE : BACKWARD_NPC_RANGE * BACKWARD_NPC_RANGE;

		INT32 AFFECTED_CELL_COUNT	= (FORWARD_AFFECTED_CELL_COUNT > BACKWARD_AFFECTED_CELL_COUNT) ? FORWARD_AFFECTED_CELL_COUNT : BACKWARD_AFFECTED_CELL_COUNT;

		// 2008.05.07. steeple
		// Heap 생성으로 변경
		AgsmAOICell** paForwardCell		= static_cast<AgsmAOICell**>(pThis->m_csHeap.Alloc(sizeof(AgsmAOICell*) * AFFECTED_CELL_COUNT));
		AgsmAOICell** paBackwardCell	= static_cast<AgsmAOICell**>(pThis->m_csHeap.Alloc(sizeof(AgsmAOICell*) * AFFECTED_CELL_COUNT));

		//AgsmAOICell		**paForwardCell		= new AgsmAOICell*[AFFECTED_CELL_COUNT];
		//AgsmAOICell		**paBackwardCell	= new AgsmAOICell*[AFFECTED_CELL_COUNT];

		//ZeroMemory(paForwardCell, sizeof(AgsmAOICell *) * AFFECTED_CELL_COUNT);
		//ZeroMemory(paBackwardCell, sizeof(AgsmAOICell *) * AFFECTED_CELL_COUNT);

		//PC인 경우만 처리해준다. 몹인경우 들어오면 안된다.
		// 무조건 버퍼링 한다.
//		if( pThis->m_pcsAgpmCharacter->IsPC(pcsAgpdCharacter) )
//		{
//			INT32	lNumAroundCharacter	= pThis->m_pagsmAOIFilter->GetNumAroundCharacter(pcsAgsdCharacter->m_pcsCurrentCell);
//
//			if (lNumAroundCharacter > AGSM_CHARACTER_NUM_START_PACKET_BUFFERING)
//				pThis->ActiveSendBuffer(pcsAgsdCharacter->m_dpnidCharacter);
//			else
//				pThis->DisableSendBuffer(pcsAgsdCharacter->m_dpnidCharacter);
//		}

		{
			PROFILE("AgsmCharacter::GetForwardCell");

			// 이동 방향을 보고 새로 데이타 싱크를 맞추어야 하는 섹터를 구한다.
			if (!pThis->GetForwardCell(pcsAgsdCharacter->m_pcsPrevCell, pcsAgsdCharacter->m_pcsCurrentCell, paForwardCell, paBackwardCell, FORWARD_CHAR_RANGE, BACKWARD_CHAR_RANGE))
			{
				pThis->m_csHeap.Free(paForwardCell);
				pThis->m_csHeap.Free(paBackwardCell);
				//delete [] paBackwardCell;
				//delete [] paForwardCell;

				return FALSE;
			}
		}

		{
			PROFILE("AgsmCharacter::ProcessRemoveCell");

			for (int i = 0; i < AFFECTED_CELL_COUNT; i++)
			{
				if (!paBackwardCell[i])
					continue;

				if (pcsAgsdCharacter->m_dpnidCharacter)
					pThis->SendCellRemoveToChar(paBackwardCell[i], pcsAgpdCharacter);
				pThis->SendCharRemoveToCell(pcsAgpdCharacter, paBackwardCell[i], FALSE);
			}
		}

		{
			PROFILE("AgsmCharacter::ProcessAddCell");

			for (int i = 0; i < AFFECTED_CELL_COUNT; i++)
			{
				if (!paForwardCell[i])
					continue;

				if (pcsAgsdCharacter->m_dpnidCharacter)
					pThis->SendCellToChar(paForwardCell[i], pcsAgpdCharacter);
				pThis->SendCharToCell(pcsAgpdCharacter, paForwardCell[i]);

			}
		}

		ZeroMemory(paForwardCell, sizeof(AgsmAOICell *) * AFFECTED_CELL_COUNT);
		ZeroMemory(paBackwardCell, sizeof(AgsmAOICell *) * AFFECTED_CELL_COUNT);

		{
			PROFILE("AgsmCharacter::GetForwardCell (for NPC)");

			// 이동 방향을 보고 새로 데이타 싱크를 맞추어야 하는 섹터를 구한다.
			if (!pThis->GetForwardCell(pcsAgsdCharacter->m_pcsPrevCell, pcsAgsdCharacter->m_pcsCurrentCell, paForwardCell, paBackwardCell, FORWARD_NPC_RANGE, BACKWARD_NPC_RANGE))
			{
				pThis->m_csHeap.Free(paForwardCell);
				pThis->m_csHeap.Free(paBackwardCell);
				//delete [] paBackwardCell;
				//delete [] paForwardCell;

				return FALSE;
			}
		}

		{
			PROFILE("AgsmCharacter::ProcessRemoveNPCCell");

			for (int i = 0; i < AFFECTED_CELL_COUNT; i++)
			{
				if (!paBackwardCell[i])
					continue;

				if (pcsAgsdCharacter->m_dpnidCharacter)
					pThis->SendCellNPCRemoveToChar(paBackwardCell[i], pcsAgpdCharacter);
			}
		}

		{
			PROFILE("AgsmCharacter::ProcessAddNPCCell");

			for (int i = 0; i < AFFECTED_CELL_COUNT; i++)
			{
				if (!paForwardCell[i])
					continue;

				if (pcsAgsdCharacter->m_dpnidCharacter)
					pThis->SendCellNPCToChar(paForwardCell[i], pcsAgpdCharacter);

			}
		}

		pThis->m_csHeap.Free(paForwardCell);
		pThis->m_csHeap.Free(paBackwardCell);
		//delete [] paBackwardCell;
		//delete [] paForwardCell;

		pcsAgsdCharacter->m_bMoveCell = FALSE;
	}

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateMoney(PVOID pData, PVOID pClass, PVOID pCustData)
{
	BOOL			bResult;

	bResult = FALSE;

//	if (!pData || !pClass || !pCustData)
	if (!pData || !pClass)
		return bResult;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;
	AgsdCharacter*	pcsAgsdCharacter = pThis->GetADCharacter(pcsAgpdCharacter);

	if (pcsAgsdCharacter->m_dpnidCharacter == 0)
		return TRUE;

	INT8	cOperation = AGPMCHAR_PACKET_OPERATION_UPDATE;
	INT16	nPacketLength;
	INT64	llMoney;

	if( pThis->m_pcsAgpmCharacter->GetMoney( pcsAgpdCharacter, &llMoney ) )
	{
		PVOID pvPacketMove = NULL;

		PVOID pvPacket = pThis->m_pcsAgpmCharacter->m_csPacket.MakePacket(TRUE, &nPacketLength, AGPMCHARACTER_PACKET_TYPE, 
																		  &cOperation,							// Operation
																		  &pcsAgpdCharacter->m_lID,				// Character ID
																		  NULL,									// Character Template ID
																		  NULL,									// Game ID
																		  NULL,									// Character Status
																		  NULL,									// Move Packet
																		  NULL,									// Action Packet
																		  NULL,									// Factor Packet
																		  &llMoney,								// llMoney
																		  NULL,									// bank money
																		  NULL,									// cash
																		  NULL,									// character action status
																		  NULL,									// character criminal status
																		  NULL,									// attacker id (정당방위 설정에 필요)
																		  NULL,									// 새로 생성되서 맵에 들어간넘인지 여부
																		  NULL,									// region index
																		  NULL,									// social action index
																		  NULL,									// special status
																		  NULL,									// is transform status
																		  NULL,									// skill initialization text
																		  NULL,									// face index
																		  NULL,									// hair index
																		  NULL,									// Option Flag
																		  NULL,									// bank size
																		  NULL,									// event status flag
																		  NULL,									// remained criminal status time
																		  NULL,									// remained murderer point time
																		  NULL,									// nick name
																		  NULL,									// gameguard
																		  NULL									// last killed time in battlesquare
																		  ,NULL
																		  );

		if( pvPacket != NULL )
		{
			if( pThis->SendPacket(pvPacket, nPacketLength, pcsAgsdCharacter->m_dpnidCharacter) )
			{
				bResult = TRUE;
			}

			pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);
		}
	}

	AgsdServer	*pcsServer	= pThis->m_pAgsmServerManager2->GetThisServer();

	if (pcsServer && (eAGSMSERVER_TYPE) pcsServer->m_cType == AGSMSERVER_TYPE_GAME_SERVER &&
		pcsServer->m_lServerID == pcsAgsdCharacter->m_ulServerID)
	{
		if (abs((long) pcsAgsdCharacter->m_llPrevUpdateMoney - (long) pcsAgpdCharacter->m_llMoney) > 250)
			pThis->BackupCharacterData(pcsAgpdCharacter);
	}

	return bResult;
}

BOOL AgsmCharacter::CBUpdateBankMoney(PVOID pData, PVOID pClass, PVOID pCustData)
{
	BOOL			bResult;

	bResult = FALSE;

//	if (!pData || !pClass || !pCustData)
	if (!pData || !pClass)
		return bResult;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;
	AgsdCharacter*	pcsAgsdCharacter = pThis->GetADCharacter(pcsAgpdCharacter);

	if (pcsAgsdCharacter->m_dpnidCharacter == 0)
		return TRUE;

	INT8	cOperation = AGPMCHAR_PACKET_OPERATION_UPDATE;
	INT16	nPacketLength;

	PVOID pvPacket = pThis->m_pcsAgpmCharacter->m_csPacket.MakePacket(TRUE, &nPacketLength, AGPMCHARACTER_PACKET_TYPE, 
																	  &cOperation,									// Operation
																	  &pcsAgpdCharacter->m_lID,						// Character ID
																	  NULL,											// Character Template ID
																	  NULL,											// Game ID
																	  NULL,											// Character Status
																	  NULL,											// Move Packet
																	  NULL,											// Action Packet
																	  NULL,											// Factor Packet
																	  NULL,											// llMoney
																	  &pcsAgpdCharacter->m_llBankMoney,				// bank money
																	  NULL,											// cash
																	  NULL,											// character action status
																	  NULL,											// character criminal status
																	  NULL,											// attacker id (정당방위 설정에 필요)
																	  NULL,											// 새로 생성되서 맵에 들어간넘인지 여부
																	  NULL,											// region index
																	  NULL,											// social action index
																	  NULL,											// special status
																	  NULL,											// is transform status
																	  NULL,											// skill initialization text
																	  NULL,											// face index
																	  NULL,											// hair index
																	  NULL,											// Option Flag
																	  NULL,											// bank size
																	  NULL,											// event status flag
																	  NULL,											// remained criminal status time
																	  NULL,											// remained murderer point time
																	  NULL,											// nick name
																	  NULL,											// gameguard
																	  NULL											// last killed time in battlesquare
																	  ,NULL
																	  );

	if( pvPacket != NULL )
	{
		if( pThis->SendPacket(pvPacket, nPacketLength, pcsAgsdCharacter->m_dpnidCharacter) )
		{
			bResult = TRUE;
		}

		pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);
	}

	AgsdServer	*pcsServer	= pThis->m_pAgsmServerManager2->GetThisServer();

	if (pcsServer && (eAGSMSERVER_TYPE) pcsServer->m_cType == AGSMSERVER_TYPE_GAME_SERVER &&
		pcsServer->m_lServerID == pcsAgsdCharacter->m_ulServerID)
		pThis->BackupCharacterData(pcsAgpdCharacter);

	return bResult;
}

BOOL AgsmCharacter::CBUpdateCash(PVOID pData, PVOID pClass, PVOID pCustData)
{
	BOOL			bResult;

	bResult = FALSE;

//	if (!pData || !pClass || !pCustData)
	if (!pData || !pClass)
		return bResult;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;
	AgsdCharacter*	pcsAgsdCharacter = pThis->GetADCharacter(pcsAgpdCharacter);

	if (pcsAgsdCharacter->m_dpnidCharacter == 0)
		return TRUE;

	INT8	cOperation = AGPMCHAR_PACKET_OPERATION_UPDATE;
	INT16	nPacketLength;

	PVOID pvPacket = pThis->m_pcsAgpmCharacter->m_csPacket.MakePacket(TRUE, &nPacketLength, AGPMCHARACTER_PACKET_TYPE, 
																	  &cOperation,								// Operation
																	  &pcsAgpdCharacter->m_lID,					// Character ID
																	  NULL,										// Character Template ID
																	  NULL,										// Game ID
																	  NULL,										// Character Status
																	  NULL,										// Move Packet
																	  NULL,										// Action Packet
																	  NULL,										// Factor Packet
																	  NULL,										// llMoney
																	  NULL,										// bank money
																	  &pcsAgpdCharacter->m_llCash,				// cash
																	  NULL,										// character action status
																	  NULL,										// character criminal status
																	  NULL,										// attacker id (정당방위 설정에 필요)
																	  NULL,										// 새로 생성되서 맵에 들어간넘인지 여부
																	  NULL,										// region index
																	  NULL,										// social action index
																	  NULL,										// special status
																	  NULL,										// is transform status
																	  NULL,										// skill initialization text
																	  NULL,										// face index
																	  NULL,										// hair index
																	  NULL,										// Option Flag
																	  NULL,										// bank size
																	  NULL,										// event status flag
																	  NULL,										// remained criminal status time
																	  NULL,										// remained murderer point time
																	  NULL,										// nick name
																	  NULL,										// gameguard
																	  NULL										// last killed time in battlesquare
																	  ,NULL
																	  );

	if( pvPacket != NULL )
	{
		if( pThis->SendPacket(pvPacket, nPacketLength, pcsAgsdCharacter->m_dpnidCharacter) )
		{
			bResult = TRUE;
		}

		pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);
	}

	/*
	AgsdServer	*pcsServer	= pThis->m_pAgsmServerManager2->GetThisServer();

	if (pcsServer && (eAGSMSERVER_TYPE) pcsServer->m_cType == AGSMSERVER_TYPE_GAME_SERVER &&
		pcsServer->m_lServerID == pcsAgsdCharacter->m_ulServerID)
		pThis->BackupCharacterData(pcsAgpdCharacter);
	*/

	return bResult;
}

BOOL AgsmCharacter::CBMoveBankMoney(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;

	if (pCustData)
	{
		INT32 lMoney = *(INT32 *) pCustData;	
		if (lMoney > 0)
		{
			pThis->WriteGheldLog(AGPDLOGTYPE_GHELD_BANK_IN, pcsAgpdCharacter, lMoney);
		}
		else if (lMoney < 0)
		{
			pThis->WriteGheldLog(AGPDLOGTYPE_GHELD_BANK_OUT, pcsAgpdCharacter, abs(lMoney));
		}
	}
	return TRUE;
}

BOOL AgsmCharacter::CBUpdateStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBUpdateStatus");

	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;
	AgsdCharacter*	pcsAgsdCharacter = pThis->GetADCharacter(pcsAgpdCharacter);
	INT16*			nOldStatus = (INT16 *) pCustData;

	if (*nOldStatus == pcsAgpdCharacter->m_unCurrentStatus)
		return TRUE;

	/*
	if (*nOldStatus == AGPDCHAR_STATUS_LOGOUT && pcsAgsdCharacter->m_dpnidCharacter == 0)
		return TRUE;
	*/

	INT8	cOperation = AGPMCHAR_PACKET_OPERATION_UPDATE;
	INT16	nPacketLength;


	PVOID pvPacketMove = NULL;

	/*
	INT8	cMoveFlag;

	cMoveFlag = AGPMCHARACTER_MOVE_FLAG_STOP;

	if (*nOldStatus != AGPDCHAR_STATUS_SELECT_CHAR && pcsAgpdCharacter->m_unCurrentStatus != AGPDCHAR_STATUS_SELECT_CHAR)
	{
		pvPacketMove = pThis->m_pcsAgpmCharacter->m_csPacketMove.MakePacket(FALSE, &nPacketLength, 0, 
																	&pcsAgpdCharacter->m_stPos,
																	&pcsAgpdCharacter->m_stPos,
																	&pcsAgpdCharacter->m_fTurnX,
																	&pcsAgpdCharacter->m_fTurnY,
																	&cMoveFlag);
	}
	*/

	PVOID pvPacket = pThis->m_pcsAgpmCharacter->m_csPacket.MakePacket(TRUE, &nPacketLength, AGPMCHARACTER_PACKET_TYPE, 
																	  &cOperation,										// Operation
																	  &pcsAgpdCharacter->m_lID,							// Character ID
																	  NULL,												// Character Template ID
																	  NULL,												// Game ID
																	  &pcsAgpdCharacter->m_unCurrentStatus,				// Character Status
																	  pvPacketMove,										// Move Packet
																	  NULL,												// Action Packet
																	  NULL,												// Factor Packet
																	  NULL,												// llMoney
																	  NULL,												// bank money
																	  NULL,												// cash
																	  NULL,												// character action status
																	  NULL,												// character criminal status
																	  NULL,												// attacker id (정당방위 설정에 필요)
																	  NULL,												// 새로 생성되서 맵에 들어간넘인지 여부
																	  NULL,												// region index
																	  NULL,												// social action index
																	  NULL,												// special status
																	  NULL,												// is transform status
																	  NULL,												// skill initialization text
																	  NULL,												// face index
																	  NULL,												// hair index
																	  NULL,												// Option Flag
																	  NULL,												// bank size
																	  NULL,												// event status flag
																	  NULL,												// remained criminal status time
																	  NULL,												// remained murderer point time
																	  NULL,												// nick name
																	  NULL,												// gameguard
																	  NULL												// last killed time in battlesquare
																	  ,NULL
																	  );

	if (pvPacketMove)
		pThis->m_pcsAgpmCharacter->m_csPacketMove.FreePacket(pvPacketMove);

	if (!pvPacket)
		return FALSE;

	pThis->m_pcsAgpmCharacter->m_csPacket.SetCID(pvPacket, nPacketLength, pcsAgpdCharacter->m_lID);

	// 맵에 들어가 있으면 SendPacketNear, 안들어 있으면 SendPacket 으로 보낸다.
	if (*nOldStatus != AGPDCHAR_STATUS_IN_GAME_WORLD)
	{
		if (pcsAgsdCharacter->m_dpnidCharacter != 0 && !pThis->SendPacket(pvPacket, nPacketLength, pcsAgsdCharacter->m_dpnidCharacter, PACKET_PRIORITY_3))
		{
			pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);

			return FALSE;
		}
	}
	else
	{
		if (!pThis->m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, pcsAgpdCharacter->m_stPos, PACKET_PRIORITY_3))
		{
			pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);

			return FALSE;
		}
	}

	pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateActionStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBUpdateActionStatus");

	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;
	INT16*			nOldStatus = (INT16 *) pCustData;

	// 2004.08.25. steeple
	//if (pcsAgpdCharacter->m_unActionStatus != AGPDCHAR_STATUS_STUN &&
	if (*nOldStatus == pcsAgpdCharacter->m_unActionStatus ||
		!pcsAgpdCharacter->m_bIsAddMap)
		return TRUE;

	AgsdCharacter*	pcsAgsdCharacter = pThis->GetADCharacter(pcsAgpdCharacter);
	pcsAgsdCharacter->m_ulLastUpdateActionStatusTime = pThis->GetClockCount();

	BOOL	bResult = TRUE;

	// 바뀐 상황중 중요한 넘들만 보낸다.
	//
	/*
	if (pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD ||
		pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_ATTACK ||
		pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_READY_SKILL ||
		pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_TRADE ||
		pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_PARTY ||
		(pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_NORMAL && *nOldStatus != AGPDCHAR_STATUS_MOVE))
	{
	*/
	if (pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_MOVE ||
		pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_PREDEAD)
		return bResult;

	if ((pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_NORMAL ||
		pcsAgpdCharacter->m_unActionStatus == AGPDCHAR_STATUS_NOT_ACTION) &&
		*nOldStatus == AGPDCHAR_STATUS_MOVE)
		return bResult;

	INT8	cOperation = AGPMCHAR_PACKET_OPERATION_UPDATE;
	INT16	nPacketLength;

	PVOID pvPacket = pThis->m_pcsAgpmCharacter->m_csPacket.MakePacket(TRUE, &nPacketLength, AGPMCHARACTER_PACKET_TYPE, 
																	  &cOperation,										// Operation
																	  &pcsAgpdCharacter->m_lID,							// Character ID
																	  NULL,												// Character Template ID
																	  NULL,												// Game ID
																	  NULL,												// Character Status
																	  NULL,												// Move Packet
																	  NULL,												// Action Packet
																	  NULL,												// Factor Packet
																	  NULL,												// llMoney
																	  NULL,												// bank money
																	  NULL,												// cash
																	  &pcsAgpdCharacter->m_unActionStatus,				// character action status
																	  NULL,												// character criminal status
																	  NULL,												// attacker id (정당방위 설정에 필요)
																	  NULL,												// 새로 생성되서 맵에 들어간넘인지 여부
																	  NULL,												// region index
																	  NULL,												// social action index
																	  NULL,												// special status
																	  NULL,												// is transform status
																	  NULL,												// skill initialization text
																	  NULL,												// face index
																	  NULL,												// hair index
																	  NULL,												// Option Flag
																	  NULL,												// bank size
																	  NULL,												// event status flag
																	  NULL,												// remained criminal status time
																	  NULL,												// remained murderer point time
																	  NULL,												// nick name
																	  NULL,												// gameguard
																	  NULL												// last killed time in battlesquare
																	  ,NULL
																	  );

	if (!pvPacket)
		return FALSE;

	pThis->m_pcsAgpmCharacter->m_csPacket.SetCID(pvPacket, nPacketLength, pcsAgpdCharacter->m_lID);

	bResult = pThis->m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, pcsAgpdCharacter->m_stPos, PACKET_PRIORITY_3);

	pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);

	return bResult;
}

BOOL AgsmCharacter::CBUpdateSpecialStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdCharacter* pcsCharacter= (AgpdCharacter*)pData;
	AgsmCharacter* pThis = (AgsmCharacter*)pClass;
	//UINT64* pulOldStatus = (UINT64*)pCustData;

	PVOID *pStatus		= (PVOID *) pCustData;

	UINT64* pulOldStatus = (UINT64 *)pStatus[0];
	INT32*  pReserved    = (INT32 *)pStatus[1];

	// 업데이트를 했는데 같다.
	if(*pulOldStatus == pcsCharacter->m_ulSpecialStatus)
		return TRUE;

	AgsdCharacter* pcsAgsdCharacter = pThis->GetADCharacter(pcsCharacter);
	if(!pcsAgsdCharacter)
		return FALSE;

	// 2006.01.09. steeple
	// 몇가지 처리.

	// 일시 무적 상태가 되었다.
	if(!(*pulOldStatus & AGPDCHAR_SPECIAL_STATUS_INVINCIBLE) &&
		(pcsCharacter->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_INVINCIBLE))
	{
		pThis->m_pcsAgpmPvP->ProcessSystemMessage(AGPMPVP_SYSTEM_CODE_START_INVINCIBLE, NULL, NULL, 0, 0, pcsCharacter);
	}

	if((*pulOldStatus & AGPDCHAR_SPECIAL_STATUS_INVINCIBLE) &&
		!(pcsCharacter->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_INVINCIBLE))
	{
		pThis->m_pcsAgpmPvP->ProcessSystemMessage(AGPMPVP_SYSTEM_CODE_END_INVINCIBLE, NULL, NULL, 0, 0, pcsCharacter);
	}

	// 2008.02.27. steeple
	// Restore SpecialStatus Time
	pThis->RestoreSpecialStatusTime(pcsCharacter, *pulOldStatus);


	// 대략 패킷을 보내준다....
	
	INT8	cOperation = AGPMCHAR_PACKET_OPERATION_UPDATE;
	INT16	nPacketLength;

	PVOID pvPacket = pThis->m_pcsAgpmCharacter->m_csPacket.MakePacket(TRUE, &nPacketLength, AGPMCHARACTER_PACKET_TYPE, 
																	  &cOperation,											// Operation
																	  &pcsCharacter->m_lID,									// Character ID
																	  NULL,													// Character Template ID
																	  NULL,													// Game ID
																	  NULL,													// Character Status
																	  NULL,													// Move Packet
																	  NULL,													// Action Packet
																	  NULL,													// Factor Packet
																	  NULL,													// llMoney
																	  NULL,													// bank money
																	  NULL,													// cash
																	  NULL,													// character action status
																	  NULL,													// character criminal status
																	  NULL,													// attacker id (정당방위 설정에 필요)
																	  NULL,													// 새로 생성되서 맵에 들어간넘인지 여부
																	  NULL,													// region index
																	  NULL,													// social action index
																	  &pcsCharacter->m_ulSpecialStatus,						// special status
																	  NULL,													// is transform status
																	  NULL,													// skill initialization text
																	  NULL,													// face index
																	  NULL,													// hair index
																	  pReserved ? pReserved : NULL,											// Option Flag
																	  NULL,													// bank size
																	  NULL,													// event status flag
																	  NULL,													// remained criminal status time
																	  NULL,													// remained murderer point time
																	  NULL,													// nick name
																	  NULL,													// gameguard
																	  NULL													// last killed time in battlesquare
																	  ,NULL
																	  );

	if (!pvPacket)
		return FALSE;

	pThis->m_pcsAgpmCharacter->m_csPacket.SetCID(pvPacket, nPacketLength, pcsCharacter->m_lID);

	BOOL	bSendResult = pThis->m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, pcsCharacter->m_stPos, PACKET_PRIORITY_3);

	pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);

	return bSendResult;
}

BOOL AgsmCharacter::SendPacketUpdateEventStatusFlag(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	INT8	cOperation = AGPMCHAR_PACKET_OPERATION_UPDATE;
	INT16	nPacketLength;

	PVOID pvPacket = m_pcsAgpmCharacter->m_csPacket.MakePacket(TRUE, &nPacketLength, AGPMCHARACTER_PACKET_TYPE, 
																	  &cOperation,											// Operation
																	  &pcsCharacter->m_lID,									// Character ID
																	  NULL,													// Character Template ID
																	  NULL,													// Game ID
																	  NULL,													// Character Status
																	  NULL,													// Move Packet
																	  NULL,													// Action Packet
																	  NULL,													// Factor Packet
																	  NULL,													// llMoney
																	  NULL,													// bank money
																	  NULL,													// cash
																	  NULL,													// character action status
																	  NULL,													// character criminal status
																	  NULL,													// attacker id (정당방위 설정에 필요)
																	  NULL,													// 새로 생성되서 맵에 들어간넘인지 여부
																	  NULL,													// region index
																	  NULL,													// social action index
																	  NULL,													// special status
																	  NULL,													// is transform status
																	  NULL,													// skill initialization text
																	  NULL,													// face index
																	  NULL,													// hair index
																	  NULL,													// Option Flag
																	  NULL,													// bank size
																	  &pcsCharacter->m_unEventStatusFlag,					// event status flag
																	  NULL,													// remained criminal status time
																	  NULL,													// remained murderer point time
																	  NULL,													// nick name
																	  NULL,													// gameguard
																	  NULL													// last killed time in battlesquare
																	  ,NULL
																	  );

	if (!pvPacket)
		return FALSE;

	m_pcsAgpmCharacter->m_csPacket.SetCID(pvPacket, nPacketLength, pcsCharacter->m_lID);

	BOOL	bSendResult = m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, pcsCharacter->m_stPos, PACKET_PRIORITY_3);

	m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);

	return bSendResult;
}

BOOL AgsmCharacter::CBCheckAllBlock(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis	= (AgsmCharacter *)	pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	BOOL			*pbIsAllBlock	= (BOOL *)			pCustData;

	if (!*pbIsAllBlock)
	{
        *pbIsAllBlock	|= pThis->IsAuctionBlocked(pcsCharacter);
	}

	/*AgsdCharacter* pagsdCharacter = pThis->GetADCharacter(pcsCharacter);
	if(pagsdCharacter)
	{
		*pbIsAllBlock	|= pagsdCharacter->m_bIsTeleportBlock;
	}*/

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateCriminalStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter				*pThis				= (AgsmCharacter *)		pClass;
	AgpdCharacter				*pcsCharacter		= (AgpdCharacter *)		pData;
	AgpdCharacterCriminalStatus	*peOldStatus		= (AgpdCharacterCriminalStatus *)	pCustData;

	AgsdCharacter				*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);

	switch (pcsCharacter->m_unCriminalStatus) {
	case AGPDCHAR_CRIMINAL_STATUS_INNOCENT:
		{
		}
		break;

	case AGPDCHAR_CRIMINAL_STATUS_CRIMINAL_FLAGGED:
		{
			pcsCharacter->m_lLastUpdateCriminalFlagTime	= pThis->m_pcsAgpmCharacter->SetLastCriminalTime(pcsCharacter, AGPMCHARACTER_CRIMINAL_PRESERVE_TIME);
		}
		break;
	}

	// 기존 상태와 다르거나 선공의 경우(시간 update) 보내준다.
	if (pcsCharacter->m_unCriminalStatus != (INT8) *peOldStatus
		|| AGPDCHAR_CRIMINAL_STATUS_CRIMINAL_FLAGGED == pcsCharacter->m_unCriminalStatus)
	{
		// 패킷을 보내줘야 한다.
		pThis->SendPacketNearCharCriminalStatus(pcsCharacter);
	}

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateFactor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBUpdateFactor");

	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;

	pThis->ReCalcCharacterResultFactors(pcsAgpdCharacter);

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateLevel(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBUpdateLevel");

	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsCharacter = (AgpdCharacter *) pData;
	INT32			lChangeLevel = *(INT32 *) pCustData;

	AgsdCharacter*	pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);
	if (!pcsAgsdCharacter) return FALSE;

	//INT32	lLevelMaxExp = 0;

	// level up point를 초기화한다.
	//pcsAgsdCharacter->m_lReceivedSkillPoint	= 0;

	INT32	lCharLevel			= pThis->m_pcsAgpmCharacter->GetLevel(pcsCharacter);
	lCharLevel += lChangeLevel;

	if (lCharLevel < 1 || lCharLevel > AGPMCHAR_MAX_LEVEL)
		return FALSE;

	INT32 lCharismaPoint = pThis->m_pcsAgpmCharacter->GetCharismaPoint(pcsCharacter);

	// 이 캐릭터의 template를 가져다가 레벨에 맞는 펙터로 업뎃한다.

	// character factor들을 초기화한다.
	//pThis->m_pagpmFactors->DestroyFactor(&pcsCharacter->m_csFactor);
	//pThis->m_pagpmFactors->DestroyFactor(&pcsCharacter->m_csFactorPoint);
	//pThis->m_pagpmFactors->DestroyFactor(&pcsCharacter->m_csFactorPercent);

	AgpdCharacterTemplate	*pcsTemplate	= NULL;
	
	if (pcsCharacter->m_bIsTrasform)
	{
		pcsTemplate	= pThis->m_pcsAgpmCharacter->GetCharacterTemplate(pcsCharacter->m_lOriginalTID);
		pThis->m_pagpmFactors->CopyFactorNotZero(&pcsCharacter->m_csFactorOriginal, &pcsTemplate->m_csLevelFactor[lCharLevel]);

		// 2008.03.11. steeple
		// Item 으로 된 것이라면 처리를 다르게 해줘야 한다.
		AgpdItemTemplate* pcsItemTemplate = pThis->m_pagpmItem->GetItemTemplate(pcsCharacter->m_lTransformItemTID);
		if(pcsItemTemplate && (pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE) &&
			(((AgpdItemTemplateUsable*)pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_TRANSFORM)
			)
		{
			// Factor 를 copy 했던 녀석은 해당 TID 의 놈으로 다시 카피해주고, Level 만 세팅하면 되는데,
			// Add 했던 녀석은 원 캐릭의 다음 레벨로 세팅해 준 다음, EffectFactor 를 Add 해준다.
			if(pcsCharacter->m_bCopyTransformFactor)
			{
				AgpdCharacterTemplate* pcsTransformCharacterTemplate = pcsCharacter->m_pcsCharacterTemplate;
				pThis->m_pagpmFactors->CopyFactorNotNull(&pcsCharacter->m_csFactor, &pcsTransformCharacterTemplate->m_csFactor);
			}
			else
			{
				pThis->m_pagpmFactors->CopyFactorNotZero(&pcsCharacter->m_csFactor, &pcsTemplate->m_csLevelFactor[lCharLevel]);
				pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactor, &((AgpdItemTemplateUsableTransform*)pcsItemTemplate)->m_csEffectFactor, TRUE, FALSE, TRUE);
			}
		}

		pThis->m_pagpmFactors->SetValue(&pcsCharacter->m_csFactor, lCharLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);
	}
	else
	{
		pcsTemplate	= pcsCharacter->m_pcsCharacterTemplate;
		pThis->m_pagpmFactors->CopyFactorNotZero(&pcsCharacter->m_csFactor, &pcsTemplate->m_csLevelFactor[lCharLevel]);
	}

	//pThis->m_pagpmFactors->CopyFactorNotZero(&pcsCharacter->m_csFactor, &pcsTemplate->m_csLevelFactor[lCharLevel]);

	pThis->m_pagpmFactors->SetValue(&pcsCharacter->m_csFactor, lCharismaPoint, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_CHA);

	//pThis->m_pagpmFactors->SetValue(&pcsCharacter->m_csFactor, lCharLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);

	// level up 하는데 필요한 exp가 얼마인지 세팅한다. 즉.. maxexp를 세팅한다.
	//pThis->m_pagpmFactors->SetValue(&pcsCharacter->m_csFactor, pThis->m_pcsAgpmCharacter->GetLevelUpExp(lCharLevel), AGPD_FACTORS_TYPE_CHAR_POINT_MAX, AGPD_FACTORS_CHARPOINTMAX_TYPE_EXP);

	pThis->m_pagsmFactors->ResetCharMaxPoint(&pcsCharacter->m_csFactor);

	pThis->ReCalcCharacterResultFactors(pcsCharacter, TRUE);

	pThis->m_pcsAgpmSkill->AdjustSkillPoint(pcsCharacter);

	pThis->m_pcsAgpmSkill->AdjustHeroicPoint(pcsCharacter);

	PVOID	pvPacketFactor = pThis->m_pagsmFactors->SetCharPointFull(&pcsCharacter->m_csFactor);

	// level up 했단 정보를 보내준다.
	if (lChangeLevel > 0)
		pThis->SendPacketLevelUp(pcsCharacter, pvPacketFactor);

	if (pvPacketFactor)
		pThis->m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);

	/*
	// DB에 저장한다.
	/////////////////////////////////////////////////////////
	if (pThis->m_pAgsmServerManager2->GetThisServerType() == AGSMSERVER_TYPE_GAME_SERVER)
		pThis->SaveCharacterLevel(pcsCharacter);
	*/

	/*
	pThis->EnumCallback(AGSMCHARACTER_CB_UPDATE_CHARACTER_TO_DB, pcsCharacter, NULL);
	*/

	AgsdServer	*pcsServer	= pThis->m_pAgsmServerManager2->GetThisServer();

	if (pcsServer && (eAGSMSERVER_TYPE) pcsServer->m_cType == AGSMSERVER_TYPE_GAME_SERVER &&
		pcsServer->m_lServerID == pcsAgsdCharacter->m_ulServerID)
		pThis->BackupCharacterData(pcsCharacter);
	/*
	if (pcsServer && pcsServer->m_pcsTemplate && 
		(eAGSMSERVER_TYPE) ((AgsdServerTemplate *) pcsServer->m_pcsTemplate)->m_cType == AGSMSERVER_TYPE_GAME_SERVER &&
		pcsServer->m_lServerID == pcsAgsdCharacter->m_ulServerID)
		pThis->BackupCharacterData(pcsCharacter);
	*/

	return TRUE;
}

BOOL AgsmCharacter::CBInitCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBInitCharacter");

	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBInitCharacter"));

	{
		AgsdCharacter *pAgsdCharacter	= pThis->GetADCharacter(pcsAgpdCharacter);

		pAgsdCharacter->m_bMoveCell			= FALSE;
		pAgsdCharacter->m_pcsPrevCell		= NULL;
		pAgsdCharacter->m_pcsCurrentCell	= NULL;
		ZeroMemory(&pAgsdCharacter->m_stPrevPos, sizeof(pAgsdCharacter->m_stPrevPos));

		pAgsdCharacter->m_ulNextAttackTime	= 0;
		pAgsdCharacter->m_ulNextSkillTime	= 0;

		pAgsdCharacter->m_ulLastUpdateActionStatusTime	= 0;

		pAgsdCharacter->m_ulHistoryDuration = AGSD_CHARACTER_HISTORY_DEFAULT_DURATION;
		pAgsdCharacter->m_fHistoryRange = AGSD_CHARACTER_HISTORY_DEFAULT_RANGE;

		pAgsdCharacter->m_ulReleaseStunStatusTime			= 0;
		pAgsdCharacter->m_ulReleaseFreezeStatusTime			= 0;
		pAgsdCharacter->m_ulReleaseSlowStatusTime			= 0;
		pAgsdCharacter->m_ulReleaseInvincibleStatusTime		= 0;
		pAgsdCharacter->m_ulReleaseAttributeInvincibleStatusTime = 0;
		pAgsdCharacter->m_ulReleaseNotAddAgroStatusTime		= 0;
		pAgsdCharacter->m_ulReleaseHideAgroStatusTime		= 0;
		pAgsdCharacter->m_ulReleaseStunProtectStatusTime	= 0;
		pAgsdCharacter->m_ulReleaseTransparentTime			= 0;
		pAgsdCharacter->m_ulReleasePvPInvincibleTime		= 0;
		pAgsdCharacter->m_ulReleaseHaltStatusTime			= 0;
		pAgsdCharacter->m_ulReleaseDisArmamentTime			= 0;

		pAgsdCharacter->m_lSlowStatusValue					= 0;

		pAgsdCharacter->m_eIdleProcessInterval[AGSDCHAR_IDLE_TYPE_TOTAL]	= AGSDCHAR_IDLE_INTERVAL_NEXT_TIME;

		pAgsdCharacter->m_ulReserveTimeForDestroy	= 0;

		pAgsdCharacter->m_bRemoveByPeriod = FALSE;
		pAgsdCharacter->m_DetailInfo.clear();

		pAgsdCharacter->m_TargetInfoArray.m_arrTargetInfo.clear();
	}

	AgpdFactor*	pcsFactorResult = (AgpdFactor *) pThis->m_pagpmFactors->GetFactor(&pcsAgpdCharacter->m_csFactor, AGPD_FACTORS_TYPE_RESULT);
	if (pcsFactorResult)
	{
		pThis->m_pagpmFactors->InitFactor(pcsFactorResult);
	}

	if (pThis->m_pcsAgpmCharacter->IsPC(pcsAgpdCharacter))
		pThis->m_pagsmFactors->ResetCharMaxPoint(&pcsAgpdCharacter->m_csFactor);

	pThis->ReCalcCharacterFactors(pcsAgpdCharacter, FALSE);

	/*
	PVOID pvPacketFactor = pThis->m_pagsmFactors->SetCharPoint(&pcsAgpdCharacter->m_csFactor);
	if (pvPacketFactor)
	{
		pThis->SendPacketFactor(pvPacketFactor, pcsAgpdCharacter);

		pThis->m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);
	}
	*/

	pThis->CheckRecvCharList(pcsAgpdCharacter->m_lID);

	if (pcsAgpdCharacter->m_unCurrentStatus == AGPDCHAR_STATUS_IN_LOGIN_PROCESS)
		pThis->SetWaitOperation(pcsAgpdCharacter, AGSMCHARACTER_WAIT_OPERATION_ENTER_GAME_WORLD);

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBUpdateCharacter");

	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter*	pThis = (AgsmCharacter *) pClass;
	AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;
	AgpdCharacter*	pcsSourceCharacter;
	AgsdCharacter*	pstAgsdCharacter = pThis->GetADCharacter(pcsAgpdCharacter);
	AgsdCharacterHistoryEntry *	pstEntry;
	UINT32			ulClockCount = PtrToUint(pCustData);
	INT32			lIndex;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBUpdateCharacter"));

	if (!pThis->IsIdleProcessTime(pcsAgpdCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, ulClockCount))
		return TRUE;

	pThis->ResetIdleInterval(pcsAgpdCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER);
	pThis->SetProcessTime(pcsAgpdCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, ulClockCount);

	pThis->SetIdleInterval(pcsAgpdCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, AGSDCHAR_IDLE_INTERVAL_TEN_SECONDS);

//	if (pThis->m_ulPrevHistoryRefreshTime + pThis->m_ulHistoryRefreshDelay > ulClockCount)
//		return TRUE;

	for (lIndex = 0; lIndex < pstAgsdCharacter->m_stHistory.m_lEntryNum; ++lIndex)
	{
		pstEntry = &pstAgsdCharacter->m_stHistory.m_astEntry[lIndex];

		if (pstEntry->m_ulLastHitTime + pstAgsdCharacter->m_ulHistoryDuration <= ulClockCount)
		{
//			pThis->RemoveHistory(pcsAgpdCharacter, &pstEntry->m_csSource);
//			--lIndex;
			continue;
		}

		if (pstAgsdCharacter->m_stHistory.m_astEntry[lIndex].m_csSource.m_eType == APBASE_TYPE_CHARACTER)
		{
			pcsSourceCharacter = pThis->m_pcsAgpmCharacter->GetCharacter(pstEntry->m_csSource.m_lID);
			if (!pcsSourceCharacter)
			{
				if (pThis->RemoveHistory(pcsAgpdCharacter, (ApBase *) &pstEntry->m_csSource))
					--lIndex;
				continue;
			}

			if (pcsSourceCharacter->m_unCurrentStatus != AGPDCHAR_STATUS_IN_GAME_WORLD)
			{
				if (pThis->RemoveHistory(pcsAgpdCharacter, (ApBase *) &pstEntry->m_csSource))
					--lIndex;
				continue;
			}
			/*
			if (pcsSourceCharacter->m_unActionStatus != AGPDCHAR_STATUS_NORMAL)
			{
				pThis->RemoveHistory(pcsAgpdCharacter, &pstEntry->m_csSource);
				--lIndex;
				continue;
			}
			*/

			if (AUPOS_DISTANCE_XZ(pcsAgpdCharacter->m_stPos, pcsSourceCharacter->m_stPos) > pstAgsdCharacter->m_fHistoryRange)
			{
				if (pThis->RemoveHistory(pcsAgpdCharacter, (ApBase *) &pstEntry->m_csSource))
					--lIndex;
				continue;
			}
		}
	}

	if (pstAgsdCharacter->m_stHistory.m_lEntryNum > 0)
		pThis->SetIdleInterval(pcsAgpdCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);

	// 2005.04.27. steeple
	// Party History 에 대해선 없길래 추가했음.
	// Party 는 시간만 체크해서 시간 지나면 없애주자.

	for (lIndex = 0; lIndex < pstAgsdCharacter->m_stHistoryParty.m_lEntryNum; ++lIndex)
	{
		pstEntry = &pstAgsdCharacter->m_stHistoryParty.m_astEntry[lIndex];

		if (pstEntry->m_ulLastHitTime + pstAgsdCharacter->m_ulHistoryDuration <= ulClockCount)
		{
//			pThis->RemoveHistory(pcsAgpdCharacter, &pstEntry->m_csSource);
//			--lIndex;
			continue;
		}

		if (pstEntry->m_csSource.m_eType == APBASE_TYPE_PARTY)
		{
			AgpdParty* pcsParty = pThis->m_pcsAgpmParty->GetParty(pstEntry->m_csSource.m_lID);
			if (!pcsParty)
			{
				if (pThis->RemovePartyHistory(pcsAgpdCharacter, (ApBase *) &pstEntry->m_csSource))
					--lIndex;
				continue;
			}
		}
	}

	if (pstAgsdCharacter->m_stHistoryParty.m_lEntryNum > 0)
		pThis->SetIdleInterval(pcsAgpdCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);
	
	/*
	if (pThis->m_pcsAgpmCharacter->IsMukza(pcsAgpdCharacter))
	{
		if ((ulClockCount - pstAgsdCharacter->m_ulLastUpdateMukzaPointTime) > pstAgsdCharacter->m_ulRemainUpdateMukzaPointTime)
		{
			pThis->m_pcsAgpmCharacter->UpdateMukzaPoint(pcsAgpdCharacter, pThis->m_pcsAgpmCharacter->GetMukzaPoint(pcsAgpdCharacter) - 1);
		}
	}
	*/

	// Special Status 를 체크한다.
	pThis->CheckSpecialStatusIdleTime(pcsAgpdCharacter, ulClockCount);

	if (pThis->m_pcsAgpmCharacter->IsPC(pcsAgpdCharacter))
	{
		// 악당포인트 소멸가능시간
		
		// 선공상태 조정. by laki
		if (pcsAgpdCharacter->m_unCriminalStatus == AGPDCHAR_CRIMINAL_STATUS_CRIMINAL_FLAGGED)
		{
			if (0 == pThis->m_pcsAgpmCharacter->GetRemainedCriminalTime(pcsAgpdCharacter))
			{
				pThis->m_pcsAgpmCharacter->UpdateCriminalStatus(pcsAgpdCharacter, AGPDCHAR_CRIMINAL_STATUS_INNOCENT);
			}
		}

		// 주기적으로 DB에 저장하는 내용을 저장한다.
		///////////////////////////////////////////////////////
		AgsdServer	*pcsServer	= pThis->m_pAgsmServerManager2->GetThisServer();

		if (pcsServer && (eAGSMSERVER_TYPE) pcsServer->m_cType == AGSMSERVER_TYPE_GAME_SERVER &&
			pcsServer->m_lServerID == pstAgsdCharacter->m_ulServerID &&
			pstAgsdCharacter->m_ulNextBackupCharacterDataTime <= ulClockCount &&
			pstAgsdCharacter->m_dpnidCharacter)
		{
			pThis->BackupCharacterData(pcsAgpdCharacter);
		}
		/*
		if (pcsServer && pcsServer->m_pcsTemplate && 
			(eAGSMSERVER_TYPE) ((AgsdServerTemplate *) pcsServer->m_pcsTemplate)->m_cType == AGSMSERVER_TYPE_GAME_SERVER &&
			pcsServer->m_lServerID == pstAgsdCharacter->m_ulServerID &&
			pstAgsdCharacter->m_ulNextBackupCharacterDataTime <= ulClockCount)
		{
			pThis->BackupCharacterData(pcsAgpdCharacter);
		}
		*/
	}

	// 변신 중이라면 Timeout이 세팅되어 있는지 보고 되어 있다면 체크한다.
	if (pcsAgpdCharacter->m_bIsTrasform)
	{
		if (pstAgsdCharacter->m_ulTimeOutTransform > 0 &&
			pstAgsdCharacter->m_ulTimeOutTransform <= ulClockCount)
		{
			pThis->m_pcsAgpmCharacter->RestoreTransformCharacter(pcsAgpdCharacter);

			pThis->ResetTransformTimeout(pcsAgpdCharacter);
		}
		else
		{
			pThis->SetIdleInterval(pcsAgpdCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);
		}
	}

	// 결제한 유저만 게임을 지속할수 있다. 1시간 마다 검사한다
	// 북미도 아이템 유료화로 변하면서 주석처리 by 성경
	//if ( AP_SERVICE_AREA_WESTERN == g_eServiceArea && pstAgsdCharacter->m_ulTimeOutPaid <= ulClockCount )
	//{
	//	//AgpdCharacter*	pcsAgpdCharacter = (AgpdCharacter *) pData;
	//	// 결제 정보 확인
	//	if ( g_cogpas.AuthenticateUser( pcsAgpdCharacter->m_szID, false) )
	//	{
	//		pThis->SetPaidTimeout(pcsAgpdCharacter, AGSDCHAR_IDLE_INTERVAL_ONE_HOUR);
	//	}
	//	else
	//	{
	//		UINT32 ulNID = pThis->GetCharDPNID( pcsAgpdCharacter );
	//		INT32 lCID = pcsAgpdCharacter->m_lID;

	//		if(ulNID != 0)
	//			pThis->DestroyClient(ulNID);
	//		else if(lCID != 0)
	//			pThis->m_pcsAgpmCharacter->RemoveCharacter(lCID);
	//		else
	//			pThis->m_pcsAgpmCharacter->RemoveCharacter( pcsAgpdCharacter->m_szID );

	//	}
	//}

	// 공격한 타겟 리스트 검사한다. 2005.11.29. steeple
	if(pThis->CheckTargetInfoIdle(pcsAgpdCharacter, ulClockCount) > 0)
		pThis->SetIdleInterval(pcsAgpdCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateMurdererPoint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis				= (AgsmCharacter *) pClass;
	AgpdCharacter	*pcsCharacter		= (AgpdCharacter *)	pData;
	INT32			lDiff				= *((INT32 *) pCustData);

	AgsdCharacter	*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);

	// 포인트 증가시, 악당 1레벨 이상시에만 남은시간 업데이트
	if (lDiff > 0 &&
		pThis->m_pcsAgpmCharacter->GetMurdererLevel(pcsCharacter) >= AGPMCHAR_MURDERER_LEVEL1_POINT
		)
	{
		pThis->m_pcsAgpmCharacter->SetLastMurdererTime(pcsCharacter, AGPMCHARACTER_MURDERER_PRESERVE_TIME);
	}

	return pThis->SendPacketMurdererPoint(pcsCharacter);
}

BOOL AgsmCharacter::CBUpdateMukzaPoint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter	*pThis				= (AgsmCharacter *) pClass;
	AgpdCharacter	*pcsCharacter		= (AgpdCharacter *)	pData;

	AgsdCharacter	*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);

	pcsAgsdCharacter->m_ulLastUpdateMukzaPointTime = pThis->GetClockCount();
	pcsAgsdCharacter->m_ulRemainUpdateMukzaPointTime = AGSMCHARACTER_UPDATE_MUKZA_INTERVAL;

	return pThis->SendPacketMukzaPoint(pcsCharacter);
}

BOOL AgsmCharacter::CBUpdateCharismaPoint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis				= (AgsmCharacter *) pClass;
	AgpdCharacter	*pcsCharacter		= (AgpdCharacter *)	pData;
	INT32			lDiff				= *((INT32 *) pCustData);

	AgsdCharacter	*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);

	//return pThis->SendPacketCharismaPoint(pcsCharacter);	// 이건 클라이언트에서 값이 업데이트 안됨.
	return pThis->SendPacketMurdererPoint(pcsCharacter);	// 이걸로 해야 됨. 아마도 클라문제 일껄~... arycoat - 2008.7
}

BOOL AgsmCharacter::CBGetDPNID(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *) pData;
	AgsmCharacter	*pThis			= (AgsmCharacter *) pClass;
	UINT_PTR		*pulDPNID		= (UINT_PTR *)		pCustData;

	AgsdCharacter	*pcsAgsdCharacter = pThis->GetADCharacter(pcsCharacter);

	if(!pcsAgsdCharacter)
		return FALSE;

	*pulDPNID = pcsAgsdCharacter->m_dpnidCharacter;

	return TRUE;
}

BOOL AgsmCharacter::CBTransformAppear(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)		pData;
	INT32			lTID			= *(INT32 *)			pCustData;

	pThis->ResetTransformTimeout(pcsCharacter);

	pThis->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);

	return pThis->SendPacketTransformAppear(pcsCharacter, lTID);
}

BOOL AgsmCharacter::CBTransformStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)		pData;

	pcsCharacter->m_bIsTrasform		= TRUE;

	// status가 변경되었다. Result Factor를 전체 다 다시 계산해야 한다.
	pThis->ReCalcCharacterResultFactors(pcsCharacter);

	pThis->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);

	return TRUE;
}

BOOL AgsmCharacter::CBRestoreTransform(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)		pData;

	// status가 변경되었다. Result Factor를 전체 다 다시 계산해야 한다.
	pThis->ReCalcCharacterResultFactors(pcsCharacter);

	return pThis->SendPacketRestoreTransformAppear(pcsCharacter);
}

BOOL AgsmCharacter::CBEvolution(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)		pData;
	AgpdCharacterTemplate* pcsTemplate = (AgpdCharacterTemplate*) pCustData;

	if(!pcsCharacter || !pcsTemplate)
		return FALSE;

	pThis->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);
	pThis->SendPacketEvolutionAppear(pcsCharacter, pcsTemplate->m_lID);

	INT32 lLevel = pThis->m_pcsAgpmCharacter->GetLevel(pcsCharacter);
	AgpdFactor* pcsResultFactor = (AgpdFactor*)pThis->m_pagpmFactors->GetFactor(&pcsCharacter->m_csFactor, AGPD_FACTORS_TYPE_RESULT);

	pThis->m_pagpmFactors->CopyFactorNotZero(&pcsCharacter->m_csFactor, &pcsTemplate->m_csLevelFactor[lLevel]);
	pThis->m_pagpmFactors->CopyFactor(&pcsCharacter->m_csFactor, &pcsTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_CHAR_TYPE, TRUE);
	pThis->m_pagpmFactors->CopyFactor(pcsResultFactor, &pcsTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_CHAR_TYPE, TRUE);
	pThis->ReCalcCharacterFactors(pcsCharacter);
	
	return TRUE;
}

BOOL AgsmCharacter::CBRestoreEvolution(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)		pData;
	AgpdCharacterTemplate* pcsTemplate = (AgpdCharacterTemplate*) pCustData;

	if(!pcsCharacter || !pcsTemplate)
		return FALSE;

	pThis->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_CHARACTER, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);
	pThis->SendPacketRestoreEvolutionAppear(pcsCharacter, pcsTemplate->m_lID);

	INT32 lLevel = pThis->m_pcsAgpmCharacter->GetLevel(pcsCharacter);
	AgpdFactor* pcsResultFactor = (AgpdFactor*)pThis->m_pagpmFactors->GetFactor(&pcsCharacter->m_csFactor, AGPD_FACTORS_TYPE_RESULT);

	pThis->m_pagpmFactors->CopyFactorNotZero(&pcsCharacter->m_csFactor, &pcsTemplate->m_csLevelFactor[lLevel]);
	pThis->m_pagpmFactors->CopyFactor(&pcsCharacter->m_csFactor, &pcsTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_CHAR_TYPE, TRUE);
	pThis->m_pagpmFactors->CopyFactor(pcsResultFactor, &pcsTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_CHAR_TYPE, TRUE);
	pThis->ReCalcCharacterFactors(pcsCharacter);

	return TRUE;
}

BOOL AgsmCharacter::CBCancelTransform(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)		pData;

	// 2005.10.24. steeple
	// 남이 건 Transform 을 자기가 캔슬할 수는 없다.
	if(pcsCharacter->m_lTransformedByCID != 0 && pcsCharacter->m_lTransformedByCID != pcsCharacter->m_lID)
		return FALSE;

	pThis->m_pcsAgpmCharacter->RestoreTransformCharacter(pcsCharacter);

	pThis->ResetTransformTimeout(pcsCharacter);

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateSkillPoint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBUpdateSkillPoin");

	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)		pData;
	INT32			lSkillPoint		= *(INT32 *)			pCustData;

	/*
	// DB에 저장한다.
	////////////////////////////////////////////////////////////
	if (pThis->m_pAgsmServerManager2->GetThisServerType() == AGSMSERVER_TYPE_GAME_SERVER)
		pThis->SaveCharacterSkillPoint(pcsCharacter);
	*/

	return pThis->ReCalcCharacterResultFactors(pcsCharacter, TRUE);
}

BOOL AgsmCharacter::CBAddAttackerToList(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter		*pThis			= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)		pData;
	INT32				lAttackerID		= *(INT32 *)			pCustData;

	return pThis->SendAddAttackerID(pcsCharacter, lAttackerID, pThis->GetCharDPNID(pcsCharacter));
}

BOOL AgsmCharacter::CBRemoveChar(PVOID pData, PVOID pClass, PVOID pCustData)
{
	PROFILE("AgsmCharacter::CBRemoveChar");

	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter		*pThis			= (AgsmCharacter *) pClass;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)	pData;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBRemoveChar"));

	//pcsCharacter->m_ulRemoveTimeMSec	= pThis->GetClockCount();

	if( pThis->m_pcsAgpmCharacter->IsPC(pcsCharacter) )
	{
		pThis->RemoveListRemoveChar(pcsCharacter);
		pThis->m_csAdminCheckRecvChar.RemoveObject(pcsCharacter->m_lID);

		//pc라면 AgsmLogin모듈로 게임서버 나갔다는 사실을 로그인서버로 알린다.
		if (!pThis->m_pcsAgpmCharacter->IsReturnToLoginServerStatus(pcsCharacter))
			pThis->EnumCallback( AGSMCHARACTER_CB_DISCONNECT_FROM_GAMESERVER, pcsCharacter, NULL );

		INT32	lClass	= pThis->m_pagpmFactors->GetClass(&pcsCharacter->m_pcsCharacterTemplate->m_csFactor);
		INT32	lRace	= pThis->m_pagpmFactors->GetRace(&pcsCharacter->m_pcsCharacterTemplate->m_csFactor);

		if (lRace > AURACE_TYPE_NONE && lRace < AURACE_TYPE_MAX &&
			lClass > AUCHARCLASS_TYPE_NONE && lClass < AUCHARCLASS_TYPE_MAX)
		{
			InterlockedDecrement(&pThis->m_alNumPlayerByClass[lRace][lClass]);
		}
	}

	AgsdServer			*pcsThisServer	= pThis->m_pAgsmServerManager2->GetThisServer();
	
	if (!pcsThisServer)
		return FALSE;
	/*
	if (!pcsThisServer || !pcsThisServer->m_pcsTemplate)
		return FALSE;
	*/

	switch ((eAGSMSERVER_TYPE) pcsThisServer->m_cType) {
	//switch ((eAGSMSERVER_TYPE) ((AgsdServerTemplate *) pcsThisServer->m_pcsTemplate)->m_cType) {

	case AGSMSERVER_TYPE_LOGIN_SERVER:
		{
			return pThis->SendPacketCharRemove(pcsCharacter, pThis->GetCharDPNID(pcsCharacter), FALSE);
		}
		break;

	case AGSMSERVER_TYPE_GAME_SERVER:
		{
			if (pThis->m_pcsAgpmCharacter->IsPC(pcsCharacter))
			{
				AgsdCharacter	*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);
				if (pcsAgsdCharacter &&
					pcsAgsdCharacter->m_ulServerID == pcsThisServer->m_lServerID)
				{
					// 일본 릴리즈일때만 해당하지만 NULL로 세팅되어 있으니 상관없다.
					//delete pcsCharacter->m_pGameguard;

#ifdef _AREA_CHINA_
					g_agsmGameholic.Offline( pcsAgsdCharacter->m_szAccountID,
											   pcsCharacter->m_szID,
											   pcsAgsdCharacter->m_strIPAddress.begin() );
#endif

					AgsdServer	*pcsRelayServer	= pThis->m_pAgsmServerManager2->GetRelayServer();
					if (pcsRelayServer)
					{
						if (pcsAgsdCharacter->m_bIsNotLogout)
							pThis->BackupCharacterData(pcsCharacter);
						
						pThis->SendPacketCharRemove(pcsCharacter, pcsRelayServer->m_dpnidServer, FALSE);
					}
				}
			}
		}
		break;
	}

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateFactorView(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter		*pThis			= (AgsmCharacter *)		pClass;
	AgpdFactor			*pcsFactor		= (AgpdFactor *)		pData;

	AgpdFactorOwner		*pcsFactorOwner	= (AgpdFactorOwner *) pThis->m_pagpmFactors->GetFactor(pcsFactor, AGPD_FACTORS_TYPE_OWNER);
	if (!pcsFactorOwner || pcsFactorOwner->lValue[AGPD_FACTORS_OWNER_TYPE_ID] == AP_INVALID_CID || !pcsFactorOwner->lValue[AGPD_FACTORS_OWNER_TYPE_OWNER])
		return TRUE;

	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pcsFactorOwner->lValue[AGPD_FACTORS_OWNER_TYPE_OWNER];

	if(!pThis->m_pcsAgpmCharacter->GetCharacter(pcsCharacter->m_szID))
		return FALSE;

	return pThis->SendPacketUpdateFactorView(pcsCharacter);
}

BOOL AgsmCharacter::CBUpdateFactorMovement(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter		*pThis			= (AgsmCharacter *)		pClass;
	AgpdFactor			*pcsFactor		= (AgpdFactor *)		pData;

	AgpdFactorOwner		*pcsFactorOwner	= (AgpdFactorOwner *) pThis->m_pagpmFactors->GetFactor(pcsFactor, AGPD_FACTORS_TYPE_OWNER);
	if (!pcsFactorOwner || pcsFactorOwner->lValue[AGPD_FACTORS_OWNER_TYPE_ID] == AP_INVALID_CID || !pcsFactorOwner->lValue[AGPD_FACTORS_OWNER_TYPE_OWNER])
		return TRUE;

	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pcsFactorOwner->lValue[AGPD_FACTORS_OWNER_TYPE_OWNER];

	if(!pThis->m_pcsAgpmCharacter->GetCharacter(pcsCharacter->m_szID))
		return FALSE;

	if (pcsCharacter->m_bMove)
	{
		return pThis->m_pcsAgpmCharacter->MoveCharacter(pcsCharacter,
														&pcsCharacter->m_stDestinationPos,
														pcsCharacter->m_eMoveDirection,
														pcsCharacter->m_bPathFinding,
														pcsCharacter->m_bMoveFast,
														pcsCharacter->m_bHorizontal);
	}

	return TRUE;
}

BOOL AgsmCharacter::CBBackupCharacterData(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter		*pThis			= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)		pData;

	return pThis->BackupCharacterData(pcsCharacter);
}

// 2005.11.30. steeple
// 그동안 주석이었는데 다시 풀었3~~~~~~~~~~~~~
BOOL AgsmCharacter::CBPayActionCost(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)	pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	INT16			nActionType		= *(INT16 *)		pCustData;

	switch ((AgpdCharacterActionType) nActionType) {
	case AGPDCHAR_ACTION_TYPE_ATTACK:
		{
			INT32		lClass	= (INT32) AUCHARCLASS_TYPE_NONE;
			pThis->m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lClass, AGPD_FACTORS_TYPE_CHAR_TYPE, AGPD_FACTORS_CHARTYPE_TYPE_CLASS);

			// 마법사인 경우 MP를 소모시켜 준다.
			// 무기를 들고 있는경우에만 소모시킨다.
			if (lClass == AUCHARCLASS_TYPE_MAGE &&
				!pcsCharacter->m_bIsTrasform)
			{
				AgpdItem	*pcsWeaponItem	= pThis->m_pagpmItem->GetEquipWeapon(pcsCharacter);
				if (pcsWeaponItem)
				{
					INT32	lCostMP		= pThis->m_pcsAgpmCharacter->GetMeleeActionCostMP(pcsCharacter);
					if (lCostMP < 0)
						return FALSE;
					else if (lCostMP == 0)
						return TRUE;

					AgpdFactor			csUpdateFactor;
					pThis->m_pagpmFactors->InitFactor(&csUpdateFactor);

					AgpdFactor			*pcsUpdateFactorResult = (AgpdFactor *) pThis->m_pagpmFactors->SetFactor(&csUpdateFactor, NULL, AGPD_FACTORS_TYPE_RESULT);
					if (!pcsUpdateFactorResult)
						return FALSE;

					AgpdFactorCharPoint *pcsFactorCharPoint = (AgpdFactorCharPoint *) pThis->m_pagpmFactors->InitCalcFactor(pcsUpdateFactorResult, AGPD_FACTORS_TYPE_CHAR_POINT);
					if (!pcsFactorCharPoint)
						return FALSE;

					pThis->m_pagpmFactors->SetValue(&csUpdateFactor, (-lCostMP), AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_MP);

					PVOID pvPacketFactor = pThis->m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactor, &csUpdateFactor, TRUE, TRUE, TRUE, FALSE);
					if (pvPacketFactor)
					{
						pThis->SendPacketFactor(pvPacketFactor, pcsCharacter, PACKET_PRIORITY_3);

						pThis->m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);
					}

					pThis->m_pagpmFactors->DestroyFactor(&csUpdateFactor);
				}
			}
		}
		break;
	}

	return TRUE;
}

BOOL AgsmCharacter::CBResetNID(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter		*pThis				= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)		pData;

	AgsdCharacter		*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);

	pcsAgsdCharacter->m_dpnidCharacter		= 0;

	return TRUE;
}

BOOL AgsmCharacter::CBAccountRemoveCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter		*pThis				= (AgsmCharacter *)		pClass;
	INT32				lCID				= *(INT32 *)			pData;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBAccountRemoveCharacter"));

	AgpdCharacter		*pcsCharacter		= pThis->m_pcsAgpmCharacter->GetCharacter(lCID);
	if (!pcsCharacter)
		return FALSE;

	//if (pThis->m_pcsAgpmCharacter->IsNPC(pcsCharacter) || pcsCharacter->m_bIsProtectedNPC)
	//	AuLogFile("RemoveNPC.log", "Removed by CBAccountRemoveCharacter()\n");

	AgsdCharacter		*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);

	if (pcsAgsdCharacter->m_bIsNewCID)
		pThis->m_pcsAgpmCharacter->RemoveCharacter(pcsAgsdCharacter->m_lOldCID);
	else
		pThis->m_pcsAgpmCharacter->RemoveCharacter(lCID);

	return TRUE;
}

BOOL AgsmCharacter::CBIsPlayerCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter		*pThis					= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter			= (AgpdCharacter *)		pData;
	BOOL				*pbIsPlayerCharacter	= (BOOL *)				pCustData;

	AgsdCharacter		*pcsAgsdCharacter		= pThis->GetADCharacter(pcsCharacter);

	if (pcsAgsdCharacter->m_dpnidCharacter == 0 &&
		!pThis->m_pcsAgpmCharacter->IsStatusSummoner(pcsCharacter) &&
		!pThis->m_pcsAgpmCharacter->IsStatusTame(pcsCharacter))
		*pbIsPlayerCharacter	= FALSE;
	else
		*pbIsPlayerCharacter	= TRUE;

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateRegionIndex(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter		*pThis					= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter			= (AgpdCharacter *)		pData;
	INT16				nPrevRegionIndex		= *(INT16*)				pCustData;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBUpdateRegionIndex"));

	ApmMap::RegionTemplate	*pcsRegionTemplate		= pThis->m_papmMap->GetTemplate(pcsCharacter->m_nBindingRegionIndex);
	ApmMap::RegionTemplate	*pcsPrevRegionTemplate	= pThis->m_papmMap->GetTemplate(nPrevRegionIndex);

	if (pThis->m_pcsAgpmEventBinding)
	{
		PROFILE("AgsmCharacter::Update Last Binding Town");
		
		if (pcsRegionTemplate)
		{
			// binding event가 붙어 있는 region 인지 본다.
			if (pThis->m_pcsAgpmEventBinding->GetBindingByTown(pcsRegionTemplate->pStrName, AGPDBINDING_TYPE_RESURRECTION))
			{
				switch(pcsCharacter->m_nBindingRegionIndex)
				{
				case REGIONINDEX_BATTLEGROUND_MOONELF_CAMP:
					{
						if(pThis->m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor) == AURACE_TYPE_MOONELF)
							pcsCharacter->m_nLastExistBindingIndex	= pcsCharacter->m_nBindingRegionIndex;	
					}
					break;
				case REGIONINDEX_BATTLEGROUND_ORC_CAMP:
					{
						if(pThis->m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor) == AURACE_TYPE_ORC)
							pcsCharacter->m_nLastExistBindingIndex	= pcsCharacter->m_nBindingRegionIndex;	
					}
					break;
				case REGIONINDEX_BATTLEGROUND_HUMAN_CAMP:
					{
						if(pThis->m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor) == AURACE_TYPE_HUMAN)
							pcsCharacter->m_nLastExistBindingIndex	= pcsCharacter->m_nBindingRegionIndex;	
					}
					break;
				case REGIONINDEX_BATTLEGROUND_DRAGONSCION_CAMP:
					{
						if(pThis->m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor) == AURACE_TYPE_DRAGONSCION)
							pcsCharacter->m_nLastExistBindingIndex	= pcsCharacter->m_nBindingRegionIndex;	
					}
					break;
				default:
					{
						pcsCharacter->m_nLastExistBindingIndex	= pcsCharacter->m_nBindingRegionIndex;
					}
					break;
				}
			}
		}
	}

//	if (pThis->m_pcsAgpmCharacter->IsPC(pcsCharacter))
	{
		// 마을인 경우 해당 마을의 모든 NPC 정보 전송, 2005.6.27 by kelovon
		if (pcsRegionTemplate != NULL
			&& pcsRegionTemplate->ti.stType.uFieldType == ApmMap::FT_TOWN)
		{
			pThis->SendRegionNPCToChar(pThis->m_pcsAgpmCharacter->GetRealRegionIndex(pcsCharacter), pcsCharacter);
		}

		// 이전 region이 마을인 경우 해당 마을의 모든 NPC 정보 전송, 2005.6.27 by kelovon
		if (pcsPrevRegionTemplate != NULL
			&& pcsPrevRegionTemplate->ti.stType.uFieldType == ApmMap::FT_TOWN)
		{
			pThis->SendRegionNPCRemoveToChar(nPrevRegionIndex, pcsCharacter);
		}
	}

	if (pThis->m_pcsAgpmCharacter->IsPC(pcsCharacter))
	{
		if( pcsPrevRegionTemplate && pcsPrevRegionTemplate->nLevelLimit )
		{
			pThis->ReleaseCharacterLevelLimit( pcsCharacter );
		}

		if( pcsRegionTemplate && pcsRegionTemplate->nLevelLimit )
		{
			INT32	lLevel	= pcsRegionTemplate->nLevelLimit;
			if (lLevel > 0 && lLevel < AGPMCHAR_MAX_LEVEL)
				pThis->SetCharacterLevelLimit(pcsCharacter, lLevel);
		}
	}

	//return pThis->SendPacketUpdateRegionIndex(pcsCharacter);
	return TRUE;
}

BOOL AgsmCharacter::CBSocialAnimation(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter		*pThis					= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter			= (AgpdCharacter *)		pData;
	UINT8				ucSocialAnimation		= *(UINT8 *)			pCustData;

	if (FALSE == pThis->m_pcsAgpmArchlord->IsArchlord(pcsCharacter->m_szID))
	{
		if (TRUE == pcsCharacter->m_bIsTrasform)
			return FALSE;
	}

	if (TRUE == pcsCharacter->m_bRidable)
		return FALSE;

	return pThis->SendPacketSocialAnimation(pcsCharacter, ucSocialAnimation);
}

BOOL AgsmCharacter::CBCheckActionTargetAttack(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter		*pThis				= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)		pData;
	PVOID				*ppvBuffer			= (PVOID *)				pCustData;

	BOOL				*pbIsValidAttack	= (BOOL *)				ppvBuffer[2];

	*pbIsValidAttack	= pThis->m_pcsAgpmCharacter->CheckVaildNormalAttackTarget(pcsCharacter, (AgpdCharacter *) ppvBuffer[0], PtrToInt(ppvBuffer[1]));

	return TRUE;
}

BOOL AgsmCharacter::CBCheckProcessIdle(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter		*pThis				= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)		pData;
	UINT32				*pulClockCount		= (UINT32 *)			pCustData;

	return pThis->CheckProcessIdle(pcsCharacter, *pulClockCount);
}

BOOL AgsmCharacter::CBReleaseActionMove(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter		*pThis				= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)		pData;

	if (pThis->m_pagpmOptimizedPacket2)
	{
		INT16	nPacketLength	= 0;
		PVOID	pvPacket		= pThis->m_pagpmOptimizedPacket2->MakePacketReleaseMoveAction(pcsCharacter, &nPacketLength);

		pThis->m_pcsAgpmCharacter->m_csPacket.SetCID(pvPacket, nPacketLength, pcsCharacter->m_lID);

		pThis->SendPacket(pvPacket, nPacketLength, pThis->GetCharDPNID(pcsCharacter));

		pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);
	}

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateCustomizeIndex(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter		*pThis				= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)		pData;

	// 2005.10.10. steeple
	// 투명이면 투명 부터 풀어준다.
	if(pThis->m_pcsAgpmCharacter->IsStatusTransparent(pcsCharacter) == TRUE)
		pThis->m_pcsAgpmCharacter->UpdateUnsetSpecialStatus(pcsCharacter, AGPDCHAR_SPECIAL_STATUS_TRANSPARENT);

	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= pThis->MakePacketUpdateCustomize(pcsCharacter, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	pThis->m_pcsAgpmCharacter->m_csPacket.SetCID(pvPacket, nPacketLength, pcsCharacter->m_lID);

	BOOL	bSendResult	= pThis->m_pagsmAOIFilter->SendPacketNear(pvPacket, nPacketLength, pcsCharacter->m_stPos, PACKET_PRIORITY_3);

	pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);

	pThis->BackupCharacterData(pcsCharacter);

	return	bSendResult;
}

/*
	2005.05.31 By SungHoon
	거부 마스크가 변경될 경우 불린다.
*/
BOOL AgsmCharacter::CBUpdateOptionFlag( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter		*pThis				= (AgsmCharacter *)		pClass;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)		pData;
	AgsdCharacter		*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);
	INT32				lOptionFlag			= *(INT32		 *)		pCustData;

	if (!pcsAgsdCharacter) return FALSE;

//	if (pThis->m_pcsAgpmCharacter->IsOptionFlag(lOptionFlag ,AGPDCHAR_OPTION_REFUSE_GUILD_BATTLE) 
//		!= pThis->m_pcsAgpmCharacter->IsOptionFlag(pcsCharacter, AGPDCHAR_OPTION_REFUSE_GUILD_BATTLE))	//	길드전 관련 명령이면
	{
		INT32 lResult = 0;
		pThis->EnumCallback(AGSMCHARACTER_CB_IS_GUILD_MASTER, pcsCharacter, &lResult);
		if (lResult != 1) lOptionFlag = pThis->m_pcsAgpmCharacter->UnsetOptionFlag(lOptionFlag, AGPDCHAR_OPTION_REFUSE_GUILD_BATTLE);
	}
//	if (pThis->m_pcsAgpmCharacter->IsOptionFlag(lOptionFlag ,AGPDCHAR_OPTION_REFUSE_GUILD_IN) 
//		!= pThis->m_pcsAgpmCharacter->IsOptionFlag(pcsCharacter, AGPDCHAR_OPTION_REFUSE_GUILD_IN))	//	길드초대 관련 명령이면
	{
		BOOL lResult = 0;
		pThis->EnumCallback(AGSMCHARACTER_CB_IS_GUILD_MASTER, pcsCharacter, &lResult);
		if (lResult != 0) lOptionFlag = pThis->m_pcsAgpmCharacter->UnsetOptionFlag(lOptionFlag, AGPDCHAR_OPTION_REFUSE_GUILD_IN);
	}

	// 왠지 위의 코딩을 따라가야 할 거 같다. -_-; 2007.07.12. steeple
	{
		BOOL lResult = 0;
		pThis->EnumCallback(AGSMCHARACTER_CB_IS_GUILD_MASTER, pcsCharacter, &lResult);
		if (lResult != 1) lOptionFlag = pThis->m_pcsAgpmCharacter->UnsetOptionFlag(lOptionFlag, AGPDCHAR_OPTION_REFUSE_GUILD_RELATION);
	}


	INT16	nPacketLength	= 0;
	PVOID	pvPacket		= pThis->m_pcsAgpmCharacter->MakePacketOptionFlag(pcsCharacter, lOptionFlag, &nPacketLength);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	pThis->SendPacket(pvPacket, nPacketLength, pcsAgsdCharacter->m_dpnidCharacter);
	pThis->m_pcsAgpmCharacter->m_csPacket.FreePacket(pvPacket);

	pcsCharacter->m_lOptionFlag = lOptionFlag;

	return TRUE;
}

BOOL AgsmCharacter::CBUpdateBankSize(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)	pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	INT8			cPrevBankSize	= *(INT8 *)			pCustData;

	pThis->WriteBankExpLog(pcsCharacter);

	return pThis->SendPacketUpdateBankSize(pcsCharacter, pThis->GetCharDPNID(pcsCharacter));
}

BOOL AgsmCharacter::CBBlockByPenalty(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)	pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	eAgpmCharacterPenalty	ePenalty = *((eAgpmCharacterPenalty *) pCustData);

	if (AGPMCHAR_PENALTY_NONE >= ePenalty || ePenalty >= AGPMCHAR_PENALTY_MAX)
		return FALSE;	
	
	return pThis->SendPakcetBlockByPenalty(pcsCharacter, (INT32) ePenalty, pThis->GetCharDPNID(pcsCharacter));
}

BOOL AgsmCharacter::CBGetCurrentUser(PVOID pData, PVOID pClass, PVOID pCustData)
{
	INT32* plCurrentUserCount = (INT32*)pData;
	AgsmCharacter* pThis = (AgsmCharacter*)pClass;

	if(!plCurrentUserCount || !pThis)
		return FALSE;

	*plCurrentUserCount = pThis->GetNumOfPlayers();
	return TRUE;
}

BOOL AgsmCharacter::CBAddStaticCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)	pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	ApmMap::RegionTemplate *pstRegionTemplate = pThis->m_papmMap->GetTemplate(pcsCharacter->m_nBindingRegionIndex);
	if (!pstRegionTemplate || pstRegionTemplate->ti.stType.uFieldType != ApmMap::FT_TOWN)
		return TRUE;

	vector<string>::iterator iter	= find(pThis->m_vectorStaticCharacterName.begin(), pThis->m_vectorStaticCharacterName.end(), pcsCharacter->m_szID);
	if (iter == pThis->m_vectorStaticCharacterName.end())
		pThis->m_vectorStaticCharacterName.push_back(pcsCharacter->m_szID);

	AgsdCharacter	*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);
	pcsAgsdCharacter->m_lOriginalRegionIndex	= (INT32) pcsCharacter->m_nBindingRegionIndex;

	return TRUE;
}

BOOL AgsmCharacter::CBRemoveProtectedNPC(PVOID pData, PVOID pClass, PVOID pCustData)
{
	return TRUE;

	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter	*pThis			= (AgsmCharacter *)	pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	UINT32			*pulNID			= (UINT32 *)		pCustData;

	char strCharBuff[256] = { 0, };
	sprintf_s(strCharBuff, sizeof(strCharBuff), "	socket id : %d\n", *pulNID);
	AuLogFile_s("LOG\\RemoveNPC.log", strCharBuff);

	PlayerData	*pPlayerData	= pThis->GetPlayerDataForID(*pulNID);
	if (!pPlayerData)
	{
		char strCharBuff2[256] = { 0, };
		sprintf_s(strCharBuff2, sizeof(strCharBuff2), "	PlayerData is NULL,,,,\n");
		AuLogFile_s("LOG\\RemoveNPC.log", strCharBuff2);
	}

	CHAR	*pszIPAddress	= pThis->GetPlayerIPAddress(*pulNID);
	if (!pszIPAddress || strlen(pszIPAddress))
	{
		char strCharBuff2[256] = { 0, };
		sprintf_s(strCharBuff2, sizeof(strCharBuff2), "	Remote IP address is NULL,,,,\n");
		AuLogFile_s("LOG\\RemoveNPC.log", strCharBuff2);
	}

	if (pszIPAddress && strlen(pszIPAddress))
	{
		char strCharBuff2[256] = { 0, };
		sprintf_s(strCharBuff2, sizeof(strCharBuff2), "	Remote IP address : %s\n", pszIPAddress);
		AuLogFile_s("LOG\\RemoveNPC.log", strCharBuff2);
	}

	if (pPlayerData)
	{
		char strCharBuff2[256] = { 0, };
		sprintf_s(strCharBuff2, sizeof(strCharBuff2), "	PlayerData.lID	: %d\n", pPlayerData->lID);
		AuLogFile_s("LOG\\RemoveNPC.log", strCharBuff2);

		AgsdServer2	*pcsServer	= pThis->m_pAgsmServerManager2->GetServer(pPlayerData->lID);
		if (!pcsServer)
		{
			char strCharBuff3[256] = { 0, };
			sprintf_s(strCharBuff3, sizeof(strCharBuff3), "	GetServer() returned NULL\n");
			AuLogFile_s("LOG\\RemoveNPC.log", strCharBuff3);

			AgpdCharacter	*pcsRemoveChar	= pThis->m_pcsAgpmCharacter->GetCharacter(pPlayerData->lID);
			if (pcsRemoveChar)
			{
				char strCharBuff4[256] = { 0, };
				sprintf_s(strCharBuff4, sizeof(strCharBuff4), "	character name : %s, account name : %s\n",
													pcsRemoveChar->m_szID,
													(pThis->GetAccountID(pcsRemoveChar)) ? pThis->GetAccountID(pcsRemoveChar) : "");
				AuLogFile_s("LOG\\RemoveNPC.log", strCharBuff4);
			}
			else
			{
				char strCharBuff4[256] = { 0, };
				sprintf_s(strCharBuff4, sizeof(strCharBuff4), "	GetCharacter() returned NULL\n");
				AuLogFile_s("LOG\\RemoveNPC.log", strCharBuff4);
			}
		}
		else
		{
			char strCharBuff3[256] = { 0, };
			sprintf_s(strCharBuff3, sizeof(strCharBuff3), "	server name : %s, server type : %d, server address : %s\n",
													pcsServer->m_szWorld,
													pcsServer->m_cType,
													pcsServer->m_szIP);
			AuLogFile_s("LOG\\RemoveNPC.log", strCharBuff3);
		}
	}

	return TRUE;
}

BOOL AgsmCharacter::CheckStaticCharacter(INT32 lCID, PVOID pClass, UINT32 uDelay, PVOID pvData)
{
	if (!pClass)
		return FALSE;

	AgsmCharacter	*pThis	= (AgsmCharacter *)	pClass;

	//STOPWATCH2(pThis->GetModuleName(), _T("CheckStaticCharacter"));

	if (pThis->m_vectorStaticCharacterName.size())
	{
		for (vector<string>::iterator iter = pThis->m_vectorStaticCharacterName.begin(); 
			iter != pThis->m_vectorStaticCharacterName.end() ; iter++)
		{
			BOOL	bIsRemoveCharacter	= FALSE;
			BOOL	bIsRepairCharacter	= FALSE;

			AgpdCharacter	*pcsCharacter	= pThis->m_pcsAgpmCharacter->GetCharacterLock((CHAR *) iter->c_str());
			if (!pcsCharacter)
			{
				// 지워지면 안되는 놈인데 없어졌다. 옴마야,,, 로그를 남기자,, 그리고 다시 복구해준다.
				//AuLogFile("RemoveNPC.log", "Removed NPC Name : %s\n", iter->c_str());

				// 복구를 어케 해준다냐,,,
				bIsRepairCharacter	= TRUE;

				goto end;
			}

			if (!pThis->m_pcsAgpmCharacter->IsNPC(pcsCharacter) || !pcsCharacter->m_bIsProtectedNPC)
			{
				// 값이 바뀌었다. 저 값은 바뀌면 안되는 값이다.
				//AuLogFile("RemoveNPC.log", "NPC data was changed. m_ulCharType : %d, m_bIsProtectedNPC : %d, m_szID : %s\n", pcsCharacter->m_ulCharType, pcsCharacter->m_bIsProtectedNPC, pcsCharacter->m_szID);

				pcsCharacter->m_ulCharType			= 2;
				pcsCharacter->m_bIsProtectedNPC		= TRUE;
			}

			if (!pcsCharacter->m_bIsAddMap)
			{
				// 맵에 들어있는 놈이 아니네. 이럴수가,,, 이럴순 없다.
				//AuLogFile("RemoveNPC.log", "NPC removed from map. m_szID : %s\n", pcsCharacter->m_szID);

				// 다시 맵에 넣어준다.
				pThis->m_pcsAgpmCharacter->AddCharacterToMap(pcsCharacter);
			}

			{
				BOOL	bNPCListError	= FALSE;

				map<INT16, vector<AgpdCharacter*> >::iterator itr
					= pThis->m_pcsAgpmCharacter->m_mapTownToNPCList.find(pcsCharacter->m_nBindingRegionIndex);

				if (pThis->m_pcsAgpmCharacter->m_mapTownToNPCList.end() == itr)
				{
					bNPCListError	= TRUE;
				}
				else
				{
					vector<AgpdCharacter*> &tmpNPCVector = itr->second;
					int	i = 0;
					for (i = 0; i < tmpNPCVector.size(); i++)
					{
						if (pcsCharacter == tmpNPCVector[i])
							break;
					}

					if (i == tmpNPCVector.size())
					{
						bNPCListError	= TRUE;
					}
				}

				if (bNPCListError)
				{
					// 어케 이놈 바인딩 위치로 등록된 NPC가 없냐? 이건 nBindingRegionIndex가 변했다는 얘기인가?
					//AuLogFile("RemoveNPC.log", "NPC town list error. m_szID : %s\n", pcsCharacter->m_szID);

					// 현재 위치와 nBindingRegionIndex와 비교해본다.
					INT32	lCurrentRegionIndex	= pThis->m_papmMap->GetRegion(pcsCharacter->m_stPos.x, pcsCharacter->m_stPos.z);
					if (pcsCharacter->m_nBindingRegionIndex != lCurrentRegionIndex)
					{
						AgsdCharacter	*pcsAgsdCharacter	= pThis->GetADCharacter(pcsCharacter);

						//AuLogFile("RemoveNPC.log", "	Original Index : %d, Current Index : %d, m_nBindingRegionIndex : %d\n", pcsAgsdCharacter->m_lOriginalRegionIndex, lCurrentRegionIndex, pcsCharacter->m_nBindingRegionIndex);
					}

					// 이놈을 지우고 새로 추가해준다.
					bIsRemoveCharacter	= TRUE;
					bIsRepairCharacter	= TRUE;

					goto end;
				}
			}
end:
			if (pcsCharacter)
				pcsCharacter->m_Mutex.Release();

			if (bIsRemoveCharacter)
			{
				// 지운다.
				pThis->m_pcsAgpmCharacter->RemoveCharacter((CHAR *) iter->c_str());
			}

			if (bIsRepairCharacter)
			{
				// 새로 복구해준다.
				pThis->m_pcsAgpmCharacter->StreamReadOneStaticCharacter(".\\Ini\\NPC.ini", (CHAR *) iter->c_str());
			}
		}
	}

	pThis->AddTimer(1 * 60 * 1000, 0, pThis, CheckStaticCharacter, NULL);

	return TRUE;
}

BOOL AgsmCharacter::CBGetBonusDropRate(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter* pThis = static_cast<AgsmCharacter*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pData);
	INT32* plBonusDropRate = static_cast<INT32*>(pCustData);

	AgsdCharacter* pcsAgsdCharacter = pThis->GetADCharacter(pcsCharacter);
	if(!pcsAgsdCharacter)
		return FALSE;

	*plBonusDropRate += pcsAgsdCharacter->m_stOptionSkillData.m_lBonusDropRate;
	return TRUE;
}

BOOL AgsmCharacter::CBGetBonusDropRate2(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter* pThis = static_cast<AgsmCharacter*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pData);
	INT32* plBonusDropRate2 = static_cast<INT32*>(pCustData);

	AgsdCharacter* pcsAgsdCharacter = pThis->GetADCharacter(pcsCharacter);
	if(!pcsAgsdCharacter)
		return FALSE;

	*plBonusDropRate2 += pcsAgsdCharacter->m_stOptionSkillData.m_lBonusDropRate2;
	return TRUE;
}

BOOL AgsmCharacter::CBGetBonusMoneyRate(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter* pThis = static_cast<AgsmCharacter*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pData);
	INT32* plBonusMoneyRate = static_cast<INT32*>(pCustData);

	AgsdCharacter* pcsAgsdCharacter = pThis->GetADCharacter(pcsCharacter);
	if(!pcsAgsdCharacter)
		return FALSE;

	*plBonusMoneyRate += pcsAgsdCharacter->m_stOptionSkillData.m_lBonusMoney;

	return TRUE;
}

BOOL AgsmCharacter::CBGetBonusCharismaRate(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter* pThis = static_cast<AgsmCharacter*>(pClass);
	AgpdCharacter* pcsCharacter = static_cast<AgpdCharacter*>(pData);
	INT32* plBonusCharismaRate = static_cast<INT32*>(pCustData);

	AgsdCharacter* pcsAgsdCharacter = pThis->GetADCharacter(pcsCharacter);
	if(!pcsAgsdCharacter)
		return FALSE;

	*plBonusCharismaRate += pcsAgsdCharacter->m_stOptionSkillData.m_lBonusCharismaRate;

	return TRUE;
}

BOOL AgsmCharacter::CBChangeAutoPickItem(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgsmCharacter *pThis = static_cast<AgsmCharacter*>(pClass);
	AgpdItem *pcsItem	 = static_cast<AgpdItem*>(pData);
	INT16 *plSwitch		 = static_cast<INT16*>(pCustData);

	AgsdCharacter	*pcsAgsdCharacter	= pThis->GetADCharacter(pcsItem->m_pcsCharacter);

	if(!pcsAgsdCharacter)
		return FALSE;

	switch(*plSwitch)
	{
	case AUTOPICKUPON:
		pcsAgsdCharacter->m_bIsAutoPickupItem = TRUE;
		break;
	case AUTOPICKUPOFF:
		pcsAgsdCharacter->m_bIsAutoPickupItem = FALSE;
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL AgsmCharacter::CBRequestCoupon(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pClass || !pData)
		return FALSE;

	AgsmCharacter *pThis						= (AgsmCharacter *) pClass;
	PACKET_CHARACTER_COUPON_REQUEST *pvPacket	= (PACKET_CHARACTER_COUPON_REQUEST *)pData;

	AgpdCharacter *pcsCharacter = pThis->m_pcsAgpmCharacter->GetCharacter(pvPacket->m_lCID);
	if(NULL == pcsCharacter)
		return FALSE;

	AgsdCharacter *pcsAgsdCharacter = pThis->GetADCharacter(pcsCharacter);
	if(NULL == pcsAgsdCharacter)
		return FALSE;

	AgsdAccount *pcsAccount = pThis->m_pagsmAccountManager->GetAccount(pcsAgsdCharacter->m_szAccountID);
	if(NULL == pcsAccount)
		return FALSE;

	INT32 lCouponCount = pcsAccount->m_csCoupon.m_lNumCoupon;
	INT32 lSendCount = 0;

	PACKET_CHARACTER_COUPON_INFO stPacket;
	stPacket.m_lTotalCount = lCouponCount;

	for(int i=0; i < lCouponCount; ++i)
	{
		if(lSendCount == AGPPACCOUNT_MAX_COUPON_PACKET)
		{
			AgsEngine::GetInstance()->SendPacket(stPacket, pcsAgsdCharacter->m_dpnidCharacter);

			ZeroMemory(&stPacket.m_stCouponInfo, sizeof(stPacket.m_stCouponInfo));

			lSendCount = 0;
		}

		stPacket.m_stCouponInfo[lSendCount].m_lCouponType = pcsAccount->m_csCoupon.m_pcsCouponInfo[lSendCount].m_lCouponType;
		strncpy_s(stPacket.m_stCouponInfo[lSendCount].m_szCouponNo, AGPPACCOUNT_MAX_COUPONNO + 1, pcsAccount->m_csCoupon.m_pcsCouponInfo[lSendCount].m_szCouponNo, AGPPACCOUNT_MAX_COUPONNO + 1);

		
		++lSendCount;
	}

	if(lSendCount != 0)
	{
		AgsEngine::GetInstance()->SendPacket(stPacket, pcsAgsdCharacter->m_dpnidCharacter);
	}

	return TRUE;
}

BOOL AgsmCharacter::CBRequestCouponUse(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pClass || !pData)
		return FALSE;

	AgsmCharacter *pThis							= (AgsmCharacter *) pClass;
	PACKET_CHARACTER_REQUEST_COUPON_USE *pvPacket	= (PACKET_CHARACTER_REQUEST_COUPON_USE *)pData;

	AgpdCharacter *pcsCharacter = pThis->m_pcsAgpmCharacter->GetCharacter(pvPacket->m_lCID);
	if(NULL == pcsCharacter)
		return FALSE;

	AgsdCharacter *pcsAgsdCharacter = pThis->GetADCharacter(pcsCharacter);
	if(NULL == pcsAgsdCharacter)
		return FALSE;

	AgsdAccount *pcsAccount = pThis->m_pagsmAccountManager->GetAccount(pcsAgsdCharacter->m_szAccountID);
	if(NULL == pcsAccount)
		return FALSE;

	for(int i=0; i < pcsAccount->m_csCoupon.m_lNumCoupon; ++i)
	{
		// 요청한 쿠폰을 검색
		if( strcmp(pvPacket->m_stRequestCoupon.m_szCouponNo, pcsAccount->m_csCoupon.m_pcsCouponInfo[i].m_szCouponNo) == 0 )
		{
			// 사용할 수 있는 녀석인지 확인
			if(pcsAccount->m_csCoupon.m_pcsCouponInfo[i].m_bEnable)
			{
				PACKET_AGSP_CHARACTER_REQUEST_COUPON_USE stRequestPacket;

				strncpy_s(stRequestPacket.m_szCharName, AGSMACCOUNT_MAX_ACCOUNT_NAME, pcsCharacter->m_szID, _TRUNCATE);
				strncpy_s(stRequestPacket.stRequestCoupon.m_szCouponNo, AGPPACCOUNT_MAX_COUPONNO, pcsAccount->m_csCoupon.m_pcsCouponInfo[i].m_szCouponNo, _TRUNCATE);

				stRequestPacket.stRequestCoupon.m_lCouponType = pcsAccount->m_csCoupon.m_pcsCouponInfo[i].m_lCouponType;

				AgsdServer	*pcsRelayServer	= pThis->m_pAgsmServerManager2->GetRelayServer();
				if (pcsRelayServer)
				{
					AgsEngine::GetInstance()->SendPacket(stRequestPacket, pcsRelayServer->m_dpnidServer);
				}

				break;
			}
		}
	}

	return TRUE;
}

BOOL AgsmCharacter::CBRenameCharacterID(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData) 
		return FALSE;

	AgsmCharacter* pThis = (AgsmCharacter*)pClass;
	CHAR* szOldID = (CHAR*)pData;
	CHAR* szNewID = (CHAR*)pCustData;

	char strCharBuff[256] = { 0, };
	sprintf_s(strCharBuff, sizeof(strCharBuff), "ChangeName : OldCharID = [%s] NewCharID = [%s]", szOldID, szNewID);
	AuLogFile_s("Log\\EventSystem.log", strCharBuff);

	pThis->m_pagpmLog->WriteLog_ETC(	AGPDLOGTYPE_ETC_RENAME,
										0,
										NULL,
										NULL,
										NULL,
										szOldID,
										0,
										0,
										0,
										0,
										0,
										0,
										NULL,
										NULL,
										0,
										szNewID
										);

	return TRUE;
}