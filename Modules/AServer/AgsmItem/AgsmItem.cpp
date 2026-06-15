/******************************************************************************
Module:  AgsmItem.cpp
Notices: Copyright (c) NHN Studio 2002 netong
Purpose: 
Last Update: 2002. 07. 29
******************************************************************************/

#include "AgsmItem.h"
#include "ApPacket.h"
#include "ApAutoLockCharacter.h"
#include "AuTimeStamp.h"
#include "AgsmGuild.h"
#include "AgsmTitle.h"
#include "AgsmItemManager.h"
#include "AgpmBattleGround.h"

#define OVER_FIVE_HUNDRED_THOSAND_ITEM_COUNT		"Over500000Item.log"

AgsmItem::AgsmItem()
{
	SetModuleName("AgsmItem");

	EnableIdle2(TRUE);

	SetModuleType(APMODULE_TYPE_SERVER);

	SetPacketType(AGSMITEM_PACKET_TYPE);

	m_csPacket.SetFlagLength(sizeof(INT8));
	m_csPacket.SetFieldType(
							AUTYPE_PACKET,			1,			// AgsdItem Data
							AUTYPE_PACKET,			1,			// AgsdItemADChar data
							AUTYPE_UINT32,			1,			// current clock count
							AUTYPE_MEMORY_BLOCK,	1,			// AgpdItem Packet
							AUTYPE_UINT64,			1,			// DBIID
							AUTYPE_MEMORY_BLOCK,	1,			// AgpdItemConvert Packet
							AUTYPE_END,				0
							);

	m_csPacketData.SetFlagLength(sizeof(INT8));
	m_csPacketData.SetFieldType(
							AUTYPE_INT32,			1,			// item id
							AUTYPE_UINT64,			1,			// m_ullDBIID
							AUTYPE_UINT32,			1,			// m_ulOwnTime
							AUTYPE_INT32,			1,			// m_lUseItemRemain
							AUTYPE_UINT32,			1,			// m_ulNextUseItemTime
							AUTYPE_UINT32,			1,			// m_ulDropTime
							AUTYPE_PACKET,			1,			// m_csFirstLooterBase packet
							AUTYPE_INT8,			1,			// m_bIsNeedInsertDB
							AUTYPE_END,				0
							);

	m_csPacketADChar.SetFlagLength(sizeof(INT8));
	m_csPacketADChar.SetFieldType(
							AUTYPE_INT32,			1,			// character id
							AUTYPE_INT32,			AGPMITEM_MAX_USE_ITEM,		// m_pcsUseItem 의 item id
							AUTYPE_INT32,			1,			// m_lNumUseItem
							AUTYPE_UINT32,			1,			// m_ulUseHPPotionTime
							AUTYPE_UINT32,			1,			// m_ulUseMPPotionTime
							AUTYPE_UINT32,			1,			// m_ulUseSPPotionTime
							AUTYPE_UINT32,			1,			// m_ulUseReverseOrbTime
							AUTYPE_UINT32,			1,			// m_ulUseTransformTime
							AUTYPE_END,				0
							);

//	2005.04.21	Start By SungHoon
	m_csPacketPartyItem.SetFlagLength(sizeof(INT8));
	m_csPacketPartyItem.SetFieldType(
							AUTYPE_INT32,          1,				// Item Template ID
							AUTYPE_INT32,		   1,				// Item Owner CID
							AUTYPE_INT32,		   1,				// Item Count
							AUTYPE_INT8,		   1,				// # of physical convert
							AUTYPE_INT8,		   1,				// # of socket
							AUTYPE_INT8,		   1,				// # of converted socket
							AUTYPE_INT8,		   1,				// option Count
		                    AUTYPE_END,            0
							);
//	2005.04.21	Finish By SungHoon
	m_ulNextProcessRemoveFieldItemMSec	= 0;

	m_pagpmLog = NULL;

	m_pagpmBattleGround = NULL;

	m_bCreate500000ItemCountLog = FALSE;
}

AgsmItem::~AgsmItem()
{
}

BOOL AgsmItem::OnAddModule()
{
	m_pagpmGrid = (AgpmGrid *) GetModule("AgpmGrid");

	m_papmMap = (ApmMap *) GetModule("ApmMap");
	m_pagpmFactors = (AgpmFactors *) GetModule("AgpmFactors");
	m_pagpmCharacter = (AgpmCharacter *) GetModule("AgpmCharacter");
	m_pagpmItem	= (AgpmItem *) GetModule("AgpmItem");
	m_pagpmItemConvert = (AgpmItemConvert *) GetModule("AgpmItemConvert");
	m_pagpmDropItem2 = (AgpmDropItem2 *) GetModule("AgpmDropItem2");
	m_pagpmSkill = (AgpmSkill *) GetModule("AgpmSkill");
	m_pagpmEventSkillMaster = (AgpmEventSkillMaster *) GetModule("AgpmEventSkillMaster");
	m_pagpmParty = (AgpmParty *) GetModule("AgpmParty");
	m_papmEventManager = (ApmEventManager *) GetModule("ApmEventManager");
	m_pagpmEventBinding = (AgpmEventBinding *) GetModule("AgpmEventBinding");
	m_pagpmPvP = (AgpmPvP *) GetModule("AgpmPvP");
	m_pagpmLog = (AgpmLog *) GetModule("AgpmLog");
	m_pagpmOptimizedPacket2 = (AgpmOptimizedPacket2 *) GetModule("AgpmOptimizedPacket2");
	m_pagpmBillInfo = (AgpmBillInfo *) GetModule("AgpmBillInfo");
	m_pagpmSiegeWar = (AgpmSiegeWar *) GetModule("AgpmSiegeWar");
	m_pagpmSummons = (AgpmSummons *) GetModule("AgpmSummons");
	m_pagsmSystemMessage = (AgsmSystemMessage *) GetModule("AgsmSystemMessage");
	m_pagsmFactors = (AgsmFactors *) GetModule("AgsmFactors");
	m_pagsmAOIFilter = (AgsmAOIFilter *) GetModule("AgsmAOIFilter");
	m_pagsmCharacter = (AgsmCharacter *) GetModule("AgsmCharacter");
	m_pAgsmServerManager = (AgsmServerManager *) GetModule( "AgsmServerManager2" );
	m_pagsmZoning = (AgsmZoning *) GetModule("AgsmZoning");
	m_pagsmParty = (AgsmParty *) GetModule("AgsmParty");
	m_pagpmArchlord = (AgpmArchlord *) GetModule("AgpmArchlord");
	
	if (!m_papmMap || !m_pagpmCharacter || !m_pagpmItem || !m_pagpmItemConvert || !m_pagpmDropItem2 ||
		!m_pagpmSkill || !m_pagpmEventSkillMaster || !m_pagsmFactors || !m_pagsmAOIFilter || !m_pagsmCharacter)
		return FALSE;

	if (!m_pagpmParty ||
		!m_pagsmZoning ||
		//!m_pagpmItemLog ||
		!m_pagsmParty ||
		!m_papmEventManager ||
		!m_pagpmEventBinding ||
		!m_pagpmBillInfo ||
		!m_pagpmSiegeWar ||
		!m_pagpmSummons ||
		!m_pagpmArchlord)
		return FALSE;

//	if (!m_pagpmItem)
//		return FALSE;

	/*
	if (m_papmMap && !m_papmMap->SetCallbackAddItem(CallbackAddSector, this))
		return FALSE;
	if (m_papmMap && !m_papmMap->SetCallbackRemoveItem(CallbackRemoveSector, this))
		return FALSE;
	*/

	if (!m_pagsmAOIFilter->SetCallbackAddItem(CallbackAddCell, this))
		return FALSE;
	if (!m_pagsmAOIFilter->SetCallbackRemoveItem(CallbackRemoveCell, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackUpdateChar(CallbackUpdateCharacter, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackUpdateLevel(CallbackCharLevelUp, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackBindingRegionChange(CBUpdateRegionIndex, this))
		return FALSE;

	/*
	if (m_pagpmCharacter && !m_pagpmCharacter->SetCallbackInitChar(CallbackAddChar, this))
		return FALSE;
	if (m_pagpmCharacter && !m_pagpmCharacter->SetCallbackRemoveChar(CallbackRemoveChar, this))
		return FALSE;
	*/

	if (!m_pagpmItem->SetCallbackInit(CallbackInit, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackNewItemToClient(CallbackNewItemToClient, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackEquip(CallbackEquip, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackUnEquip(CallbackUnEquip, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackRemoveForNearCharacter(CallbackRemoveForNearCharacter, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackInventory(CallbackAddInventory, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackInventoryQuestAdd(CallbackAddItemToQuest, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackRemoveFromInventory( CallbackRemoveInventory, this) )
		return FALSE;

	if (!m_pagpmItem->SetCallbackSubInventoryAdd(CallbackAddInventory, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackChargingInventoryAdd(CallbackAddInventory, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackTradeGrid(CallbackUpdateTradeGrid, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackSalesBox(CallbackUpdateSalesBoxGrid, this))
		return FALSE;

	//if (!m_pagpmItem->SetCallbackUpdateFactor(CallbackUpdateFactor, this))
	//	return FALSE;

	// 20051201, kelovon
	if (!m_pagpmItem->SetCallbackUseCashItem(CBUseCashItem, this))		// ITEM_CB_ID_CHAR_USE_CASH_ITEM
		return FALSE;
	if (!m_pagpmItem->SetCallbackUnUseCashItem(CBUnUseCashItem, this))	// ITEM_CB_ID_CHAR_UNUSE_CASH_ITEM
		return FALSE;

	//if (!m_pagpmItem->SetCallbackDelete(CallbackDeleteItem, this))
	//	return FALSE;

//	if (!m_pagpmItem->SetCallbackAskReallyConvertItem(CallbackAskReallyConvertItem, this))
//		return FALSE;
//	if (!m_pagpmItem->SetCallbackConvertItem(CallbackConvertItem, this))
//		return FALSE;
//	if (!m_pagpmItem->SetCallbackAddConvertHistory(CallbackAddConvertHistory, this))
//		return FALSE;
//	if (!m_pagpmItem->SetCallbackRemoveConvertHistory(CallbackRemoveConvertHistory, this))
//		return FALSE;

	if (!m_pagpmItem->SetCallbackCheckPickupItem(CallbackCheckPickupItem, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackPickupItemMoney(CallbackPickupItemMoney, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackPickupItemResult(CallbackPickupItemResult, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackChangeItemOwner(CallbackChangeOwner, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackBank(CallbackUpdateBank, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackRemoveBank(CallbackRemoveBank, this))
		return FALSE;
	
	if (!m_pagpmItem->SetCallbackQuest(CallbackUpdateQuest, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackUseItem(CallbackUseItem, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackUseReturnScroll(CallbackUseReturnScroll, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackRemove(CallbackRemoveItem, this))
		return FALSE;

//	2005.11.30. By SungHoon
	if (!m_pagpmItem->SetCallbackUnuseItem(CallbackUnuseItem, this))	// ITEM_CB_ID_UNUSE_ITEM
		return FALSE;
//	if (!m_pagpmItem->SetCallbackUpdateEgoExp(CallbackUpdateEgoExp, this))
//		return FALSE;
//	if (!m_pagpmItem->SetCallbackUpdateEgoLevel(CallbackUpdateEgoLevel, this))
//		return FALSE;
//	if (!m_pagpmItem->SetCallbackPutSoulIntoCube(CallbackPutSoulIntoCube, this))
//		return FALSE;
//	if (!m_pagpmItem->SetCallbackUseSoulCube(CallbackUseSoulCube, this))
//		return FALSE;

	if (!m_pagpmItem->SetCallbackUpdateStackCount(CallbackUpdateStackCount, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackUpdateReuseTimeForReverseOrb(CallbackUpdateReverseOrbReuseTime, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackUpdateReuseTimeForTransform(CallbackUpdateTransformReuseTime, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackInitReuseTimeForTransform(CallbackInitTransformReuseTime, this))
		return FALSE;

	if (!m_pagpmItem->SetCallbackUpdateItemStatusFlag(CallbackUpdateItemStatusFlag, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackUpdateCooldown(CBUpdateCooldown, this))
		return FALSE;

	if (!m_pagpmItemConvert->SetCallbackConvertAsDrop(CBConvertAsDrop, this))
		return FALSE;

	//if (!m_pagpmItem->SetCallbackField(CallbackAddField, this))
	//	return FALSE;

	/*
	if (!m_pagsmAOIFilter->SetCallbackSendItem(CallbackSendItemView, this))
		return FALSE;
	if (!m_pagsmAOIFilter->SetCallbackSendCharEquipItem(CallbackSendCharEquipItem, this))
		return FALSE;
	*/

	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSendEquipItem(CallbackSendCharEquipItem, this))
		return FALSE;
	if (m_pagsmZoning && !m_pagsmZoning->SetCallbackSyncAddChar(CallbackSyncAddChar, this))
		return FALSE;
	if (m_pagsmZoning && !m_pagsmZoning->SetCallbackZoningStart(CallbackZoningStart, this))
		return FALSE;
	if (m_pagsmZoning && !m_pagsmZoning->SetCallbackZoningPassControl(CallbackZoningPassControl, this))
		return FALSE;
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackReCalcFactor(CallbackReCalcFactor, this))
		return FALSE;
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSendCharacterAllInfo(CallbackSendCharacterAllInfo, this))
		return FALSE;
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSendCharacterNewID(CallbackSendCharacterNewID, this))
		return FALSE;
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSendCharacterAllServerInfo(CallbackSendCharacterAllServerInfo, this))
		return FALSE;

	/*
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSendSectorInfo(CallbackSendSectorInfo, this))
		return FALSE;
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSendSectorRemoveInfo(CallbackSendSectorRemoveInfo, this))
		return FALSE;
	*/
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSendCellInfo(CallbackSendCellInfo, this))
		return FALSE;
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSendCellRemoveInfo(CallbackSendCellRemoveInfo, this))
		return FALSE;
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackAllUpdate(CallbackUpdateAllToDB, this))
		return FALSE;

	if (m_pagpmCharacter && !m_pagpmCharacter->SetCallbackRestoreTransform(CallbackRestoreTransform, this))
		return FALSE;
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackAdjustTransformFactor(CallbackAdjustTransformFactor, this))
		return FALSE;
	//if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackEnterGameworld(CallbackEnterGameWorld, this))		// AGSMCHARACTER_CB_ENTER_GAMEWORLD, 20051201, kelovon
	//	return FALSE;
//	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackApplyBonusFactor(CallbackApplyBonusFactor, this))
//		return FALSE;

	/*
	if (m_pagsmDBStream && !m_pagsmDBStream->SetCallbackConnectDB(CallbackConnectDB, this))
		return FALSE;
	if (m_pagsmDBStream && !m_pagsmDBStream->SetCallbackDisconnectDB(CallbackDisconnectDB, this))
		return FALSE;
	if (m_pagsmDBStream && !m_pagsmDBStream->SetCallbackItemResult(CBStreamDB, this))
		return FALSE;
	*/

	/*
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSaveCharacterDB(CBSaveCharacterDB, this))
		return FALSE;
	*/

	/*
	// register data backup callback functions
	//////////////////////////////////////////////////////////////////
	if (m_pagpmItem && !m_pagpmItem->SetCallbackChangeItemOwner(CBBackupItemBasicData, this))
		return FALSE;
	if (m_pagpmItem && !m_pagpmItem->SetCallbackAddConvertHistory(CBBackupItemAddConvertHistoryData, this))
		return FALSE;
	if (m_pagpmItem && !m_pagpmItem->SetCallbackRemoveConvertHistory(CBBackupItemRemoveConvertHistoryData, this))
		return FALSE;
	*/

	/*
	if (m_pagsmCharacter && !m_pagsmCharacter->SetCallbackSendAllDBData(CallbackSendAllDBData, this))
		return FALSE;
	*/

	// cash inventory 관련, 20051124, kelovon
	if (m_pagpmItem && !m_pagpmItem->SetCallbackAddItemCashInventory(CallbackAddCashInventory, this))	// ITEM_CB_ID_CHAR_ADD_ITEM_CASH_INVENTORY
		return FALSE;

	if (m_pagpmItem && !m_pagpmItem->SetCallbackAddItemOption(CBAddItemOption, this))
		return FALSE;

	if(!m_pagpmItem->SetCallbackItemInformation(CBItemInformation, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackEvolution(CBEvolution, this))
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackRestoreEvolution(CBEvolution, this))
		return FALSE;

	// setting attached data(m_nKeeping) in character module
	m_nIndexADItem = m_pagpmItem->AttachItemData(this, sizeof(AgsdItem), ConAgsdItem, DesAgsdItem);
	if (m_nIndexADItem < 0)
		return FALSE;

	m_nIndexADChar = m_pagpmCharacter->AttachCharacterData(this, sizeof(AgsdItemADChar), ConAgsdItemADChar, DesAgsdItemADChar);
	if (m_nIndexADChar < 0)
		return FALSE;

	m_bFirst = TRUE;

	return TRUE;
}

BOOL AgsmItem::OnInit()
{
	m_csAdminFieldItem.InitializeObject(sizeof(INT32), m_csAdminFieldItem.GetCount());
	m_csAdminDBIDItem.InitializeObject(sizeof(UINT64), m_csAdminDBIDItem.GetCount());

	m_pagsmTitle		= (AgsmTitle *) GetModule("AgsmTitle");
	m_pagsmItemManager	= (AgsmItemManager *) GetModule("AgsmItemManager");
	m_pagpmShrineBattle = (AgpmShrineBattle*) GetModule("AgpmShrineBattle");

	if(!m_pagpmShrineBattle) return FALSE;

	return TRUE;
}

BOOL AgsmItem::OnDestroy()
{
	return TRUE;
}

BOOL AgsmItem::OnIdle2(UINT32 ulClockCount)
{
	PROFILE("AgsmItem::OnIdle2");

	//STOPWATCH2(GetModuleName(), _T("OnIdle2"));

	if (ulClockCount >= m_ulNextProcessRemoveFieldItemMSec)
	{
		m_ulNextProcessRemoveFieldItemMSec = ulClockCount + AGSMITEM_PROCESS_REMOVE_FIELD_ITEM_INTERVAL;

		// 2007.12.27. steeple
		// 아이템 개수가 50만개 넘어가면 찍어준다.
		if(m_pagpmItem->csItemAdmin.GetObjectCount() > 500000 && m_bCreate500000ItemCountLog == FALSE)
		{
			m_bCreate500000ItemCountLog = TRUE;
			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "Over 500000 Item Count\n");
			AuLogFile_s(OVER_FIVE_HUNDRED_THOSAND_ITEM_COUNT, strCharBuff);
		}

		// m_csAdminFieldItem 에서 하나씩 꺼내서 최대 유지 시간이 지났는지.. 아직 필드에 있는 넘인지 살펴본다.

		INT32	lIndex	= 0;
		INT32	*plFieldIID	= (INT32 *) m_csAdminFieldItem.GetObjectSequence(&lIndex);
		while (plFieldIID && *plFieldIID != AP_INVALID_IID)
		{
			AgpdItem	*pcsFieldItem	= m_pagpmItem->GetItemLock(*plFieldIID);
			if (pcsFieldItem)
			{
				AgpdItemStatus	eStatus		= (AgpdItemStatus) pcsFieldItem->m_eStatus;

				pcsFieldItem->m_Mutex.Release();

				if (eStatus == AGPDITEM_STATUS_FIELD)
				{
					// m_ulDropTime 이후 AGSMITEM_MAX_PRESERVE_ITEM_FIELD_TIME 만큼 지났는지 본다.
					AgsdItem	*pcsAgsdItemField	= GetADItem(pcsFieldItem);
					UINT32	ulMaxPreserveItemFieldTime	= AGSMITEM_MAX_PRESERVE_ITEM_FIELD_TIME;
					if (pcsFieldItem->m_pcsItemTemplate->m_lID == 4608)
						ulMaxPreserveItemFieldTime	= 10 * 60 * 1000;
					if (pcsAgsdItemField->m_ulDropTime + ulMaxPreserveItemFieldTime <= ulClockCount)
					{
						// 오키 이넘은 삭제한다.
						ZeroMemory(pcsFieldItem->m_szDeleteReason, sizeof(CHAR) * (AGPMITEM_MAX_DELETE_REASON + 1));
						strncpy(pcsFieldItem->m_szDeleteReason, "Timeout from field", AGPMITEM_MAX_DELETE_REASON);
						m_pagpmItem->RemoveItem(pcsFieldItem, TRUE);
					}
				}
			}

			plFieldIID	= (INT32 *) m_csAdminFieldItem.GetObjectSequence(&lIndex);
		}
	}

	return TRUE;
}

void AgsmItem::Report(FILE *fp)
{
	if (!fp)
		return;

	fprintf(fp, "Field Item Admin : %d/%d\n", m_csAdminFieldItem.GetObjectCount(), m_csAdminFieldItem.GetCount());
	fprintf(fp, "Item Seq Admin   : %d/%d\n", m_csAdminDBIDItem.GetObjectCount(), m_csAdminDBIDItem.GetCount());
	fprintf(fp, "Unadded Item     : %d\n", m_lUnaddedItemCount);
}

BOOL AgsmItem::OnValid(CHAR* pszData, INT16 nSize)
{
	return TRUE;
}

UINT64 AgsmItem::GetDBIID(INT32 lIID, BOOL bIsSetNewDBID)
{
	AgpdItem	*pItem = m_pagpmItem->GetItem(lIID);
	if (pItem == NULL)
		return AP_INVALID_IID;

	return GetDBIID(pItem, bIsSetNewDBID);
}

UINT64 AgsmItem::GetDBIID(AgpdItem *pItem, BOOL bIsSetNewDBID)
{
	if (pItem == NULL)
		return AP_INVALID_IID;

	AgsdItem	*pAgsdItem = (AgsdItem *) GetADItem(pItem);
	if (pAgsdItem == NULL)
		return AP_INVALID_IID;
		
	if (pAgsdItem->m_ullDBIID == 0 &&
		bIsSetNewDBID)
	{
		EnumCallback(AGSMITEM_CB_GET_NEW_DBID, &pAgsdItem->m_ullDBIID, NULL);
	}

	return pAgsdItem->m_ullDBIID;
}

INT16 AgsmItem::SetDBIID(INT32 lIID, UINT64 ullDBIID)
{
	AgpdItem	*pItem = m_pagpmItem->GetItem(lIID);
	if (pItem == NULL)
		return FALSE;

	return SetDBIID(pItem, ullDBIID);
}

INT16 AgsmItem::SetDBIID(AgpdItem *pItem, UINT64 ullDBIID)
{
	if (pItem == NULL)
		return FALSE;

	AgsdItem *pAgsdItem = (AgsdItem *) GetADItem(pItem);

	return TRUE;
}

AgsdItem* AgsmItem::GetADItem(PVOID pData)
{
	if (!pData)
		return NULL;

	return (AgsdItem *) m_pagpmItem->GetAttachedModuleData(m_nIndexADItem, pData);
}

AgsdItemADChar* AgsmItem::GetADCharacter(PVOID pData)
{
	if (!pData)
		return NULL;

	return (AgsdItemADChar *) m_pagpmCharacter->GetAttachedModuleData(m_nIndexADChar, pData);
}

BOOL AgsmItem::ConAgsdItem(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmItem *pThis						= (AgsmItem *) pClass;
	AgpdItem *pcsItem					= (AgpdItem *) pData;
	AgsdItem *pcsAgsdItem				= pThis->GetADItem(pcsItem);

	pcsAgsdItem->m_ulDropTime			= 0;
	ZeroMemory(&pcsAgsdItem->m_csFirstLooterBase, sizeof(ApBaseExLock));

	pcsAgsdItem->m_lUseItemRemain		= 0;
	pcsAgsdItem->m_ulNextUseItemTime	= 0;
	pcsAgsdItem->m_ulOwnTime			= 0;

	pcsAgsdItem->m_bIsNeedInsertDB		= FALSE;

	pcsAgsdItem->m_lPrevSaveStackCount	= 0;

	pcsAgsdItem->m_ePauseReason			= AGSDITEM_PAUSE_REASON_NONE;
	pcsAgsdItem->m_ulPausedClock		= 0;

	//pcsAgsdItem->m_bUseOnEnterGameWorld = 0;

	return TRUE;
}

BOOL AgsmItem::DesAgsdItem(PVOID pData, PVOID pClass, PVOID pCustData)
{
	return TRUE;
}

BOOL AgsmItem::ConAgsdItemADChar(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmItem		*pThis			= (AgsmItem *)		pClass;
	AgsdItemADChar	*pAttachedData	= pThis->GetADCharacter((AgpdCharacter *) pData);

	pAttachedData->m_lUseItemID.MemSetAll();
	pAttachedData->m_lNumUseItem		= 0;
	
	pAttachedData->m_bUseTimeLimitItem	= FALSE;
	pAttachedData->m_ulTimeLimitItemCheckClock = 0;

	pAttachedData->m_pvPacketEquipItem.MemSetAll();
	pAttachedData->m_lPacketEquipItemLength.MemSetAll();

	return TRUE;
}

BOOL AgsmItem::DesAgsdItemADChar(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgsmItem		*pThis			= (AgsmItem *)		pClass;
	AgsdItemADChar	*pAttachedData	= pThis->GetADCharacter((AgpdCharacter *) pData);

	for (int i = 0; i < AGPMITEM_PART_NUM; ++i)
	{
		if (pAttachedData->m_pvPacketEquipItem[i])
		{
			pThis->m_csPacket.FreeStaticPacket(pAttachedData->m_pvPacketEquipItem[i]);

			pAttachedData->m_pvPacketEquipItem[i]		= NULL;
			pAttachedData->m_lPacketEquipItemLength[i]	= 0;
		}
	}

	return TRUE;
}

/*
BOOL AgsmItem::InitServer(UINT32 ulStartValue, UINT32 ulServerFlag, INT16 nSizeServerFlag)
{
	return m_csGenerateID.Initialize(ulStartValue, ulServerFlag, nSizeServerFlag);
}

BOOL AgsmItem::InitItemDBIDServer(UINT64 ullStartValue, UINT64 ullServerFlag, INT16 nSizeServerFlag)
{
	return m_csGenerateID64.Initialize(ullStartValue, ullServerFlag, nSizeServerFlag);
}

INT32 AgsmItem::GenerateIID()
{
	return m_csGenerateID.GetID();
}

INT64 AgsmItem::GenerateDBID()
{
	return m_csGenerateID64.GetID();
}
*/

//		LoadItem
//	Functions
//		- 캐릭터를 로드한경우 그넘이 가지고 있는 아템들도 로드한다.
//	Arguments
//		- pcsCharacter : 아템 데이타를 로드할 캐릭터 데이타.
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmItem::LoadItem(AgpdCharacter *pcsCharacter)
{
	/*
	if (!pcsCharacter)
		return FALSE;

	// DB에서 가져오는 기능 말고 걍 TID를 보고 디폴트 값으로 공통모듈에 아템을 추가한다.
	// 차후에 DB 연결이 되면 그때 여기 루틴을 바꾼다. 일단은 걍 기본 값으로 채워넣기

	// 일단은 걍 디폴트 아템을 가져온다.
	AgpdItemADCharTemplate* pcsADCharTemplate = m_pagpmItem->GetADCharacterTemplate((AgpdCharacterTemplate *) pcsCharacter->m_pcsCharacterTemplate);
	if (!pcsADCharTemplate)
		return FALSE;

	for (int i = 0; i < AGPMITEM_PART_NUM; i++)
	{
		//GenerateIID();
		//pcsADCharTemplate->m_lDefaultEquipITID[i];

		if (!pcsADCharTemplate->m_lDefaultEquipITID[i])
			continue;

		AgpdItem *pcsItem = m_pagpmItem->AddItem(GenerateIID(), pcsADCharTemplate->m_lDefaultEquipITID[i]);
		if (!pcsItem)
			continue;

		// 여기서 아템 속성 세팅
		//
		pcsItem->m_ulCID = pcsCharacter->m_lID;

		//m_pagsmFactors->LoadFactor(pcsItem->m_llDBIID, pcsItem->m_csFactor);
		//if (m_pagsmFactors)
		//	m_pagsmFactors->LoadFactor((UINT64) pcsItem->m_lID, &pcsItem->m_csFactor);			// 테스트용

		if (!m_pagpmItem->InitItem(pcsItem))
			continue;

		if (!m_pagpmItem->EquipItem(pcsCharacter, pcsItem, FALSE))
		{
			WriteLog(AS_LOG_DEBUG, "AgsmItem : failed EquipItem()");

			return FALSE;
		}

		// 여기서 아템이 추가되었다는 정보를 클라이언트로 보낸다.
		if (!SendPacketItem(pcsItem, pcsCharacter->m_ulNID, APMODULE_SENDPACKET_PLAYER))
			return FALSE;
	}
	*/

	return TRUE;
}

/*
//		SyncAddChar
//	Functions
//		- 캐릭터가 맵에 추가되었다. 이 캐릭터 주변에 떨어져 있는 (Field에 있는) 아템 정보들을 보내준다.
//	Arguments
//		- pcsCharacter : 추가된 캐릭터
//	Return value
//		- BOOL : 성공 여부 
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmItem::SyncAddChar(AgpdCharacter *pcsCharacter)
{
	PROFILE("AgsmItem::SyncAddChar");

	if (!pcsCharacter)
		return FALSE;

	ApWorldSector	*		pSector = NULL;
	ApWorldSector::IdPos *	pItem;
	
	if (m_papmMap)
		pSector = m_papmMap->GetSector(pcsCharacter->m_stPos);

	if (!pSector)
		return FALSE;

	AgsdCharacter	*pcsAgsdCharacter = m_pagsmCharacter->GetADCharacter(pcsCharacter);

	AgpdItem	*pcsItem	= NULL;

	if (pcsAgsdCharacter->m_dpnidCharacter)
	{
		// 캐릭터가 있는 섹터를 포함해서 주변8개 섹터안에 있는 필드 아템들의 정보를 보내준다.
		for (int i = (int) pSector->GetIndexX() - 1; i <= (int) pSector->GetIndexX() + 1; i++)
		{
			for (int j = (int) pSector->GetIndexZ() - 1; j <= (int) pSector->GetIndexZ() + 1; j++)
			{
				for (pItem = pSector->m_pItems; pItem; pItem = pItem->pNext)
				{
					AgpdItem	*pcsItem = m_pagpmItem->GetItem(pItem->id);
					if (!pcsItem)
						continue;

					SendPacketItemView(pcsItem, pcsAgsdCharacter->m_dpnidCharacter);
				}
			}
		}
	}

	return TRUE;
}
*/

//		SyncAddChar
//	Functions
//		- 캐릭터가 맵에 추가되었다. 이 캐릭터 주변에 떨어져 있는 (Field에 있는) 아템 정보들을 보내준다.
//	Arguments
//		- pcsCharacter : 추가된 캐릭터
//	Return value
//		- BOOL : 성공 여부 
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmItem::SyncAddChar(AgpdCharacter *pcsCharacter)
{
	PROFILE("AgsmItem::SyncAddChar");

	if (!pcsCharacter)
		return FALSE;

	AgsmAOICell		*pcsCell		= m_pagsmAOIFilter->m_csAOIMap.GetCell(&pcsCharacter->m_stPos);
	if (!pcsCell)
		return FALSE;
	
	AgsmCellUnit	*pcsCellUnit	= NULL;

	AgsdCharacter	*pcsAgsdCharacter = m_pagsmCharacter->GetADCharacter(pcsCharacter);

	AgpdItem	*pcsItem	= NULL;

	AgsmAOICell		*pcsAroundCell	= NULL;

	INT32	lRegionIndex	= pcsCharacter->m_nBindingRegionIndex;

	if(lRegionIndex < 0)
		lRegionIndex = m_papmMap->GetRegion(pcsCharacter->m_stPos.x, pcsCharacter->m_stPos.z);

	// 2006.07.05. steeple
	// 패킷 보내야할 주위의 셀 개수를 구하는 함수를 만들었다.
	INT32	lNearCellCount = m_pagsmAOIFilter->GetNearCellCountByRegion(lRegionIndex);

	if (pcsAgsdCharacter->m_dpnidCharacter)
	{
		// 캐릭터가 있는 섹터를 포함해서 주변8개 섹터안에 있는 필드 아템들의 정보를 보내준다.
		for (int i = (int) pcsCell->GetIndexX() - lNearCellCount; i <= (int) pcsCell->GetIndexX() + lNearCellCount; i++)
		{
			for (int j = (int) pcsCell->GetIndexZ() - lNearCellCount; j <= (int) pcsCell->GetIndexZ() + lNearCellCount; j++)
			{
				pcsAroundCell	= m_pagsmAOIFilter->m_csAOIMap.GetCell(i, j);
				if (!pcsAroundCell)
					continue;

				pcsAroundCell->m_csRWLock.LockReader();

				for (pcsCellUnit = pcsCell->GetItemUnit(); pcsCellUnit; pcsCellUnit = pcsAroundCell->GetNext(pcsCellUnit))
				{
					pcsItem		= (AgpdItem *) pcsCellUnit->lID;
					if (pcsItem)
					{
						SendPacketItemView(pcsItem, pcsAgsdCharacter->m_dpnidCharacter);
					}
				}

				pcsAroundCell->m_csRWLock.UnlockReader();
			}
		}
	}

	return TRUE;
}

BOOL AgsmItem::SetParamInsertQuery5(AuDatabase2 *pDatabase, AgpdItem *pcsItem )
	{
	if (!pDatabase || !pcsItem)
		return FALSE;

	AgsdItem		*pcsAgsdItem		= GetADItem(pcsItem);

	AgsdCharacter	*pcsAgsdCharacter	= NULL;
	
	if (pcsItem->m_pcsCharacter)
		pcsAgsdCharacter	= m_pagsmCharacter->GetADCharacter(pcsItem->m_pcsCharacter);

	CHAR	szPosition[65];
	AuPOS	stPos;

	ZeroMemory(szPosition, sizeof(szPosition));
	if (pcsItem->m_eStatus == AGPDITEM_STATUS_FIELD)
		stPos	= pcsItem->m_posItem;
	else
	{
		stPos.x	= (FLOAT) pcsItem->m_anGridPos[0];
		stPos.y	= (FLOAT) pcsItem->m_anGridPos[1];
		stPos.z	= (FLOAT) pcsItem->m_anGridPos[2];
	}

	m_pagsmCharacter->EncodingPosition(&stPos, szPosition, 64);

	INT32	lDurability	= 0;
	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);
	INT32	lMaxDurability	= 0;
	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lMaxDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);

	CHAR				szDBID[21];
	ZeroMemory(szDBID, sizeof(szDBID));
	_i64toa(pcsAgsdItem->m_ullDBIID, szDBID, 10);

	INT16 nParam = 0;
	pDatabase->SetParam(nParam++, szDBID, sizeof(szDBID));
	pDatabase->SetParam(nParam++, pcsAgsdCharacter->m_szAccountID, sizeof(pcsAgsdCharacter->m_szAccountID));
	pDatabase->SetParam(nParam++, (pcsItem->m_pcsCharacter) ? m_pagsmCharacter->GetRealCharName(pcsItem->m_pcsCharacter->m_szID) : "",
					     (pcsItem->m_pcsCharacter) ? sizeof(pcsItem->m_pcsCharacter->m_szID) : sizeof(CHAR));
	pDatabase->SetParam(nParam++, &pcsItem->m_pcsItemTemplate->m_lID);
	pDatabase->SetParam(nParam++, &pcsItem->m_nCount);
	pDatabase->SetParam(nParam++, &pcsItem->m_eStatus);
	pDatabase->SetParam(nParam++, szPosition, sizeof(szPosition));
	pDatabase->SetParam(nParam++, &lDurability);
	pDatabase->SetParam(nParam++, &lMaxDurability);
	pDatabase->SetParam(nParam++, &pcsItem->m_lStatusFlag);
	pDatabase->SetParam(nParam++, (UINT32*)&pcsItem->m_lRemainTime);
	CHAR szExpireDate[23];
	ZeroMemory(szExpireDate, sizeof(szExpireDate));
	AuTimeStamp::ConvertTimeStampToOracleTime(pcsItem->m_lExpireTime, szExpireDate, sizeof(szExpireDate));
	pDatabase->SetParam(nParam++, szExpireDate, sizeof(szExpireDate));
	pDatabase->SetParam(nParam++, (UINT32*)&pcsItem->m_llStaminaRemainTime);

	if (AUDATABASE2_QR_SUCCESS == pDatabase->ExecuteQuery())
		return TRUE;

	return FALSE;
}

BOOL AgsmItem::SetCallbackGetNewIID(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_GET_NEW_IID, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackGetNewDBID(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_GET_NEW_DBID, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackGetNewCashItemBuyID(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_GET_NEW_CASHITEMBUYID, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackCreateNewItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_CREATE_NEW_ITEM, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackUpdateItemDurability(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_UPDATE_ITEM_DURABILITY, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackSendItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_SEND_ITEM, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackSendItemView(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_SEND_ITEM_VIEW, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackUseItemSkillBook(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_USE_ITEM_SKILLBOOK, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackUseItemSkillScroll(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_USE_ITEM_SKILLSCROLL, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackUseItemSkillRollbackScroll(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_USE_ITEM_SKILLROLLBACKSCROLL, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackUseItemReverseOrb(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_USE_ITEM_REVERSE_ORB, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackUseItemTransform(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_USE_ITEM_TRANSFORM, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackUseItemLotteryBox(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_USE_ITEM_LOTTERYBOX, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackItemInsert(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_INSERT_ITEM_TO_DB, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackItemUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_UPDATE_ITEM_TO_DB, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackItemDelete(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_DELETE_ITEM_TO_DB, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackPickupQuestValidCheck(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_USE_ITEM_PICKUP_CHECK_QUEST_VALID, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackUseTeleportScroll(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_USE_TELEPORT_SCROLL, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackCheckReturnPosition(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_CHECK_RETURN_POSITION, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackRideMount(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_RIDE_MOUNT, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackRideDisMount(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_RIDE_DISMOUNT, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackPreCheckEnableSkillScroll(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_PRE_CHECK_ENABLE_SKILLSCROLL, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackPreCheckEnableJanusTalisman(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_PRE_CHECK_ENABLE_JANUS_TALISMAN, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackUseItemJanusTalisman(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_USE_ITEM_JANUS_TALISMAN, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackItemConvertHistoryInsert(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_INSERT_CONVERT_HISTORY_TO_DB, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackCashItemBuyListInsert(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_INSERT_CASHITEMBUYLIST, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackCashItemBuyListUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_UPDATE_CASHITEMBUYLIST, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackCashItemBuyListUpdate2(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_UPDATE_CASHITEMBUYLIST2, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackPickupCheckItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_PICKUP_CHECK_ITEM, pfCallback, pClass);
}

BOOL AgsmItem::SetCallbackEndBuffedSkillByItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGSMITEM_CB_END_BUFFED_SKILL_BY_ITEM, pfCallback, pClass);
}

//BOOL AgsmItem::SetCallbackItemConvertHistoryDelete(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGSMITEM_CB_REMOVE_CONVERT_HISTORY_FROM_DB, pfCallback, pClass);
//}

BOOL AgsmItem::AddItemToUseList(AgpdItem *pcsItem)
{
	PROFILE("AgsmItem::AddItemToUseList");

	if (!pcsItem)
		return FALSE;

	if (!pcsItem->m_pcsCharacter)
	{
		pcsItem->m_pcsCharacter = m_pagpmCharacter->GetCharacter(pcsItem->m_ulCID);
		if (!pcsItem->m_pcsCharacter)
			return FALSE;
	}

	// pcsItem의 owner의 아템 사용 리스트에 pcsItem을 추가한다.

	AgsdItemADChar	*pcsAttachCharacter = GetADCharacter(pcsItem->m_pcsCharacter);
	if (!pcsAttachCharacter)
		return FALSE;

	// 리스트 풀이다.
	if (pcsAttachCharacter->m_lNumUseItem >= AGPMITEM_MAX_USE_ITEM ||
		pcsAttachCharacter->m_lNumUseItem < 0)
		return FALSE;

	pcsAttachCharacter->m_lUseItemID[pcsAttachCharacter->m_lNumUseItem++] = pcsItem->m_lID;

	// 소모성 아템인경우.. 즉, 한번쓰면 없어지는 아템인 경우는 사용 후 인벤토리에서 뺀다.
	//	(클라이언트에선 없어진것처럼 보인다. 실제 아템 삭제는 사용이 다 된후 이루어진다.)
	//
	if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_nType == AGPMITEM_TYPE_USABLE &&
		!(((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_bStackable &&
		  pcsItem->m_nCount > 0))
	{
		//m_pagpmItem->ReleaseItem(pcsItem);
		m_pagpmItem->RemoveStatus(pcsItem, AGPDITEM_STATUS_NONE);

		if (m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter))
		{
			INT16	nPacketLength = 0;
			PVOID	pvPacket = MakePacketItemUpdateStatus(pcsItem, &nPacketLength);

			if (pvPacket && nPacketLength > 0)
			{
				SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter));

				m_pagpmItem->m_csPacket.FreePacket(pvPacket);
			}
		}
	}

	m_pagsmCharacter->SetIdleInterval(pcsItem->m_pcsCharacter, AGSDCHAR_IDLE_TYPE_ITEM, AGSDCHAR_IDLE_INTERVAL_NEXT_TIME);

	return TRUE;
}

BOOL AgsmItem::RemoveItemFromUseList(AgpdItem *pcsItem, BOOL bOnlyRemoveList)
{
	PROFILE("AgsmItem::RemoveItemFromUseList");

	if (!pcsItem)
		return FALSE;

	//if (!pcsItem->m_pcsCharacter)
	//{
	//	pcsItem->m_pcsCharacter = m_pagpmCharacter->GetCharacter(pcsItem->m_ulCID);
	//	if (!pcsItem->m_pcsCharacter)
	//		return FALSE;
	//}

	// 2007.12.13. steeple
	// Owner 를 한 번 더 구한다.
	AgpdCharacter* pcsOwner = m_pagpmCharacter->GetCharacter(pcsItem->m_ulCID);
	if(!pcsOwner || pcsOwner->m_bIsReadyRemove)
	{
		char strCharBuff[256] = { 0, };
		sprintf_s(strCharBuff, sizeof(strCharBuff), "RemoveItemFromUseList(...), pcsOwner is null, ItemTID : %d\n", pcsItem->m_lTID);
		AuLogFile_s("LOG\\ItemError.log", strCharBuff);
		
		return FALSE;
	}

	// pcsItem의 owner의 아템 사용 리스트에서 pcsItem을 삭제한다.

	AgsdItemADChar	*pcsAttachCharacter = GetADCharacter(pcsOwner);
	if (!pcsAttachCharacter)
		return FALSE;

	ASSERT(pcsAttachCharacter->m_lNumUseItem <= AGPMITEM_MAX_USE_ITEM);

	// 리스트에서 pcsItem을 검색한다.
	for (int i = 0; i < pcsAttachCharacter->m_lNumUseItem; ++i)
	{
		if (pcsAttachCharacter->m_lUseItemID[i] == pcsItem->m_lID)
		{
			if (!bOnlyRemoveList)
			{
				//사용한 아이템의 성능에 해당하는 값의 반대로 값을 돌려야한다면 돌린다.
				if(	((AgpdItemTemplate *)pcsItem->m_pcsItemTemplate)->m_nType == AGPMITEM_TYPE_USABLE )
				{
					AgpdItemTemplateUsable	*pcsTemplateUsable = (AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate;

	//				if( pcsTemplateUsable->m_nUsableItemType == AGPMITEM_USABLE_TYPE_POTION )
	//				{
	//					//템플릿ID를 보자~
	//					if( pcsTemplateUsable->m_lID == 777 )
	//					{
	//						int lOriginMinDMG, lOriginMaxDMG;
	//						int lMinDMG, lMaxDMG;
	//						int lBuffMinDMG, lBuffMaxDMG;
	//
	//						m_pagpmFactors->GetValue(&pcsItem->m_pcsCharacter->m_csFactorPoint, &lOriginMinDMG, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DAMAGE, AGPD_FACTORS_DAMAGE_TYPE_MIN );
	//						m_pagpmFactors->GetValue(&pcsItem->m_pcsCharacter->m_csFactorPoint, &lOriginMaxDMG, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DAMAGE, AGPD_FACTORS_DAMAGE_TYPE_MIN );
	//
	//						m_pagpmFactors->GetValue(&pcsTemplateUsable->m_csEffectFactor, &lBuffMinDMG, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DAMAGE, AGPD_FACTORS_DAMAGE_TYPE_MIN, AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL );
	//						m_pagpmFactors->GetValue(&pcsTemplateUsable->m_csEffectFactor, &lBuffMaxDMG, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DAMAGE, AGPD_FACTORS_DAMAGE_TYPE_MAX, AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL );
	//
	//						//물약의 효과치를 돌려놓는다.
	//						lMinDMG = lOriginMinDMG - lBuffMinDMG;
	//						lMaxDMG = lOriginMaxDMG - lBuffMaxDMG;
	//
	//						//원래대로 돌려놓는다.
	//						m_pagpmFactors->SetValue(&pcsItem->m_pcsCharacter->m_csFactorPoint, lMinDMG, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DAMAGE, AGPD_FACTORS_DAMAGE_TYPE_MIN );
	//						m_pagpmFactors->SetValue(&pcsItem->m_pcsCharacter->m_csFactorPoint, lMaxDMG, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DAMAGE, AGPD_FACTORS_DAMAGE_TYPE_MAX );
	//
	//						m_pagsmCharacter->ReCalcCharacterFactors( pcsItem->m_pcsCharacter, FALSE );
	//					}
	//					else if( pcsTemplateUsable->m_lID == 888 )
	//					{
	//						int lOriginDefense;
	//						int lDefense;
	//						int lBuffDef;
	//
	//						m_pagpmFactors->GetValue(&pcsItem->m_pcsCharacter->m_csFactorPoint, &lOriginDefense, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DEFENSE, AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT );
	//						m_pagpmFactors->GetValue(&pcsTemplateUsable->m_csEffectFactor, &lBuffDef, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DEFENSE, AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT );
	//
	//						//물약의 효과치를 돌려놓는다.
	//						lDefense = lOriginDefense - lBuffDef;
	//
	//						//원래대로 돌려놓는다.
	//						m_pagpmFactors->SetValue(&pcsItem->m_pcsCharacter->m_csFactorPoint, lDefense, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_DEFENSE, AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT );
	//
	//						m_pagsmCharacter->ReCalcCharacterFactors( pcsItem->m_pcsCharacter, FALSE );
	//					}
	//					else if( pcsTemplateUsable->m_lID == 999 )
	//					{
	//						int	lOriginAttackSpeed;
	//						int	lAttackSpeed;
	//						int lBuffAttackSpeed;
	//
	//						m_pagpmFactors->GetValue(&pcsItem->m_pcsCharacter->m_csFactorPoint, &lOriginAttackSpeed, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED );
	//						m_pagpmFactors->GetValue(&pcsTemplateUsable->m_csEffectFactor, &lBuffAttackSpeed, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED );
	//
	//						//물약의 효과치를 돌려놓는다.
	//						lAttackSpeed = lOriginAttackSpeed - lBuffAttackSpeed;
	//
	//						//원래대로 돌려놓는다.
	//						m_pagpmFactors->SetValue(&pcsItem->m_pcsCharacter->m_csFactorPoint, lAttackSpeed, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED );
	//
	//						m_pagsmCharacter->ReCalcCharacterFactors( pcsItem->m_pcsCharacter, FALSE );
	//					}
	//				}
				}


				// 소모성 아템인경우.. 즉, 한번쓰면 없어지는 아템인 경우는 사용 후 아템을 삭제한다.
				//	(Remove가 아니고 Delete해야 한다.)
				if (pcsItem->m_pcsItemTemplate &&
					((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_nType == AGPMITEM_TYPE_USABLE)
				{
					if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_bStackable)
					{
						m_pagpmItem->SubItemStackCount(pcsItem, 1);
						/*
						if (m_pagpmItem->SubItemStackCount(pcsAttachCharacter->m_pcsUseItem[i], 1) <= 0)
						{
							pcsAttachCharacter->m_pcsUseItem[i]->m_lDeleteReason = 15;
							m_pagpmItem->RemoveItem(pcsAttachCharacter->m_pcsUseItem[i]->m_lID, TRUE);
						}
						*/
					}
					else
					{
						ZeroMemory(pcsItem->m_szDeleteReason, sizeof(CHAR) * (AGPMITEM_MAX_DELETE_REASON + 1));
						strncpy(pcsItem->m_szDeleteReason, "Use item", AGPMITEM_MAX_DELETE_REASON);

						m_pagpmItem->RemoveItem(pcsItem->m_lID, TRUE);
					}
				}
			}

			// 리스트에서 지우고 m_lNumUseItem을 하나 감소시킨다.
			//CopyMemory(pcsAttachCharacter->m_lUseItemID + i, pcsAttachCharacter->m_lUseItemID + i + 1, sizeof(INT32) * (pcsAttachCharacter->m_lNumUseItem - i - 1));
			if (pcsAttachCharacter->m_lNumUseItem > 0)
				pcsAttachCharacter->m_lUseItemID.MemCopy(i, &pcsAttachCharacter->m_lUseItemID[i + 1], pcsAttachCharacter->m_lNumUseItem - i - 1);
			pcsAttachCharacter->m_lUseItemID[pcsAttachCharacter->m_lNumUseItem--] = AP_INVALID_IID;

			--i;
		}
	}

	return TRUE;
}

BOOL AgsmItem::UseItem(AgpdItem *pcsItem, BOOL bIsFirstUse, AgpdCharacter *pcsTargetChar)
{
	PROFILE("AgsmItem::UseItem");

	//STOPWATCH2(GetModuleName(), _T("UseItem"));

	if (!pcsItem) return FALSE;
	if (pcsItem->m_eStatus != AGPDITEM_STATUS_INVENTORY &&
		pcsItem->m_eStatus != AGPDITEM_STATUS_SUB_INVENTORY &&
		pcsItem->m_eStatus != AGPDITEM_STATUS_CASH_INVENTORY)
		return FALSE;

	if (pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_INUSE)
		return FALSE;

	// pcsItem이 사용되었다. 음... 타입에 따라 잘 처리해준다.

	// 먼저 character가 제대로 있는지 본다.
	if (!pcsItem->m_pcsCharacter)
	{
		pcsItem->m_pcsCharacter = m_pagpmCharacter->GetCharacter(pcsItem->m_ulCID);
		if (!pcsItem->m_pcsCharacter)
			return FALSE;
	}

	AgpdItemTemplate	*pcsTemplate	= (AgpdItemTemplate *) pcsItem->m_pcsItemTemplate;
	if (!pcsTemplate)
		return FALSE;

	// 감옥이라면 사용불가 2007.05.04. steeple
	if(m_pagpmCharacter->IsInJail(pcsItem->m_pcsCharacter))
		return FALSE;

	//JK_거래중금지
	if( pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_TRADE )
	{
		m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS);
		return FALSE;
	}

	// 이미 사용중인 동일 classify_id가 있으면 사용 불가
	if (IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType)
		&& m_pagpmItem->IsCharacterUsingCashItemOfSameClassifyIDorTID(pcsItem->m_pcsCharacter, pcsItem))
	{
		m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_CASH_ITEM_SAME_KIND_ALREADY_IN_USE);
		return FALSE;
	}

	// 별다른 메시지 안줘도 되겠찌?? 2006.06.22. steeple
	if(!CheckUsable(pcsItem, bIsFirstUse, pcsTargetChar))
		return FALSE;

	BOOL bRet = FALSE;

	switch (pcsTemplate->m_nType) {
	case AGPMITEM_TYPE_USABLE:
		{
			AgpdItemTemplateUsable	*pcsTemplateUsable = (AgpdItemTemplateUsable *) pcsTemplate;

			AgsdItem	*pcsAgsdItem = GetADItem(pcsItem);
			if (!pcsAgsdItem)
				return FALSE;

			AgpdItemADChar	*pcsAgpdItemADChar = m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);
			if (!pcsAgpdItemADChar)
				return FALSE;

			// use interval 이 세팅되어 있는 경우 그 시간이 지났는지 체크해본다. (물약의 경우만 여기서 체크)
			if (bIsFirstUse && pcsTemplateUsable->m_ulUseInterval && pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT])
			{
				if (((AgpdFactorCharPoint *) pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT])->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] > 0)
				{
					if (pcsAgpdItemADChar->m_ulUseHPPotionTime + pcsTemplateUsable->m_ulUseInterval + pcsTemplateUsable->m_lEffectActivityTimeMsec * pcsTemplateUsable->m_lEffectApplyCount > GetClockCount())
						return FALSE;
				}
				else if (((AgpdFactorCharPoint *) pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT])->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] > 0)
				{
					if (pcsAgpdItemADChar->m_ulUseMPPotionTime + pcsTemplateUsable->m_ulUseInterval + pcsTemplateUsable->m_lEffectActivityTimeMsec * pcsTemplateUsable->m_lEffectApplyCount > GetClockCount())
						return FALSE;
				}
				else if (((AgpdFactorCharPoint *) pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT])->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] > 0)
				{
					if (pcsAgpdItemADChar->m_ulUseSPPotionTime + pcsTemplateUsable->m_ulUseInterval + pcsTemplateUsable->m_lEffectActivityTimeMsec * pcsTemplateUsable->m_lEffectApplyCount > GetClockCount())
						return FALSE;
				}
			}

			switch (pcsTemplateUsable->m_nUsableItemType) {
			case AGPMITEM_USABLE_TYPE_POTION:
				{
					// potion을 사용했다. 처리해준다.
					bRet = UseItemPotion(pcsItem, bIsFirstUse);
				}
				break;

			case AGPMITEM_USABLE_TYPE_TELEPORT_SCROLL:
				{
					bRet = UseItemTeleportScroll(pcsItem);
				}
				break;

			case AGPMITEM_USABLE_TYPE_TRANSFORM:
				{
					bRet = UseItemTransform(pcsItem);
				}
				break;

			case AGPMITEM_USABLE_TYPE_SKILL_BOOK:
				{
					bRet = UseItemSkillBook(pcsItem);
				}
				break;

			case AGPMITEM_USABLE_TYPE_SKILL_SCROLL:
				{
					bRet = UseItemSkillScroll(pcsItem, pcsTargetChar);
				}
				break;

			case AGPMITEM_USABLE_TYPE_SKILLROLLBACK_SCROLL:
				{
					bRet = UseItemSkillRollbackScroll(pcsItem);
				}
				break;

			case AGPMITEM_USABLE_TYPE_REVERSE_ORB:
				{
					bRet = UseItemReverseOrb(pcsItem);
				}
				break;

			case AGPMITEM_USABLE_TYPE_RECALL_PARTY:
				{
					bRet = UseItemRecallParty(pcsItem);
				}
				break;
			case AGPMITEM_USABLE_TYPE_LOTTERY_BOX:
				{
					bRet = UseItemLotteryBox(pcsItem);
				}
				break;
			case AGPMITEM_USABLE_TYPE_LOTTERY_KEY:
				{
					bRet = UseItemLotteryKey(pcsItem);
				}
				break;
			case AGPMITEM_USABLE_TYPE_ADD_BANK_SLOT:
				{
					bRet = UseItemAddBankSlot(pcsItem);
				}
				break;
			case AGPMITEM_USABLE_TYPE_ANTI_DROP :
				{
					bRet = TRUE;
				}
				break;
			case AGPMITEM_USABLE_TYPE_USE_EFFECT :
				{
					bRet = UseItemEffect(pcsItem);
				}
				break;
			case AGPMITEM_USABLE_TYPE_CHATTING :
				{
					bRet = UseItemChatting(pcsItem);
				}
				break;
			case AGPMITEM_USABLE_TYPE_JUMP :
				{
					bRet = TRUE;
				}
				break;
			case AGPMITEM_USABLE_TYPE_LUCKY_SCROLL :
				{
					bRet = TRUE;
				}
				break;
			case AGPMITEM_USABLE_TYPE_NICK :
				{
					bRet = TRUE;
				}
				break;
			case AGPMITEM_USABLE_TYPE_SKILL_INIT :
				{
					bRet = UseItemSkillInitialize(pcsItem);
				}
				break;
			case AGPMITEM_USABLE_TYPE_ANY_SUNDRY  :
				{
					bRet = TRUE;
				}
				break;
			case AGPMITEM_USABLE_TYPE_ANY_BANK :
				{
					bRet = TRUE;
				}
				break;
			case AGPMITEM_USABLE_TYPE_AVATAR:
				{
					bRet = UseItemAvatar(pcsItem);
				}
				break;
			case AGPMITEM_USABLE_TYPE_PET_FOOD:
				{
					bRet = TRUE;
					WriteUseStartLog(pcsItem->m_pcsCharacter->m_lID, pcsItem, pcsItem->m_nCount);
				}
				break;
			case AGPMITEM_USABLE_TYPE_PRIVATE_TRADE_OPTION:
				{
					bRet = TRUE;
				}
				break;
			case AGPMITEM_USABLE_TYPE_JANUS_TALISMAN:
				{
					bRet = UseItemJanusTalisman(pcsItem, pcsTargetChar);
				}
				break;
			case AGPMITEM_USABLE_TYPE_CHARGING_BAG:
				{
					bRet = TRUE;
				}
				break;

			default:
				break;
			}
		}
		break;

	case AGPMITEM_TYPE_EQUIP:
		{
			AgpdItemTemplateEquip* pcsTemplateEquip = static_cast<AgpdItemTemplateEquip*>(pcsTemplate);

			if(pcsTemplateEquip->IsAvatarEquip())
			{
				bRet = UseItemAvatar(pcsItem);
			}
		}
		break;
	}

	if (bRet && IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType))
	{
		//	캐쉬 아이템인지 체크후 사용중이 아니면 사용중으로 돌린다.
		m_pagpmItem->StartCashItemTimer(pcsItem);
	}

	return bRet;
}

BOOL AgsmItem::UseItemPotion(AgpdItem *pcsItem, BOOL bIsFirstUse)
{
	if (!pcsItem)
		return FALSE;

	// potion을 사용했다. 처리해준다.

	// 캐릭터가 죽은경운 사용할 수 없다.
	if (pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return FALSE;

	AgsdItem	*pcsAgsdItem = GetADItem(pcsItem);
	if (!pcsAgsdItem)
		return FALSE;

	// 아직 처리 못한 단계가 있다면 나간다. 2008.04.15. steeple
	if(bIsFirstUse && pcsAgsdItem->m_lUseItemRemain != 0)
		return FALSE;

	AgpdItemADChar	*pcsAgpdItemADChar = m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);
	if (!pcsAgpdItemADChar)
		return FALSE;

	AgpdItemTemplateUsable	*pcsTemplateUsable = (AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate;

	// 해당 포인트가 꽉 차있는지 확인해본다.
	// pvPacketFactor가 NULL이면 업데이트할 내용이 없는거다.
	// 물약을 빨 수 없다.

	// effect factor를 effect activity time 의 1 / m_lEffectApplyCount 시간마다 아템 주인 factor에 적용한다.
	PVOID	pvPacketFactor	= NULL;

	FLOAT	fApplyPercent	= (FLOAT) pcsTemplateUsable->m_lEffectApplyCount;
	if (pcsAgsdItem->m_lUseItemRemain > 0)
		fApplyPercent	= (FLOAT) pcsAgsdItem->m_lUseItemRemain;

	fApplyPercent	/= 10.0f;

	// 한번에 다 채워주는 Potion 이다.
	if(pcsTemplateUsable->m_lEffectApplyCount == 0)
		fApplyPercent = 1.0f;

	if (((AgpdItemTemplateUsablePotion *) pcsTemplateUsable)->m_bIsPercentPotion)
		pvPacketFactor	= m_pagsmFactors->AddCharPointByPercent(pcsItem->m_pcsCharacter, pcsItem, FALSE, fApplyPercent);
	else
		pvPacketFactor	= m_pagsmFactors->AddCharPoint(pcsItem->m_pcsCharacter, pcsItem, FALSE, fApplyPercent);

	if (!pvPacketFactor)
	{
		if (pcsAgsdItem->m_lUseItemRemain > 0)
		{
			pcsAgsdItem->m_lUseItemRemain = 0;
			RemoveItemFromUseList(pcsItem);
		}

		return FALSE;
	}

	m_pagsmCharacter->SendPacketFactor(pvPacketFactor, pcsItem->m_pcsCharacter, PACKET_PRIORITY_3);

	m_csPacket.FreePacket(pvPacketFactor);

	if(bIsFirstUse)
	//if (pcsAgsdItem->m_lUseItemRemain == 0)
	{
		// 처음 사용인경우
		if ((AgpdFactorCharPoint *) pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT] &&
			((AgpdFactorCharPoint *) pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT])->lValue[AGPD_FACTORS_CHARPOINT_TYPE_HP] > 0)
		{
			pcsAgpdItemADChar->m_ulUseHPPotionTime = GetClockCount();
		}
		else if ((AgpdFactorCharPoint *) pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT] &&
			((AgpdFactorCharPoint *) pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT])->lValue[AGPD_FACTORS_CHARPOINT_TYPE_MP] > 0)
		{
			pcsAgpdItemADChar->m_ulUseMPPotionTime = GetClockCount();
		}
		else if ((AgpdFactorCharPoint *) pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT] &&
			((AgpdFactorCharPoint *) pcsTemplateUsable->m_csEffectFactor.m_pvFactor[AGPD_FACTORS_TYPE_CHAR_POINT])->lValue[AGPD_FACTORS_CHARPOINT_TYPE_SP] > 0)
		{
			pcsAgpdItemADChar->m_ulUseSPPotionTime = GetClockCount();
		}

		SendPacketUseItemSuccess(pcsItem->m_pcsCharacter, pcsItem);

		if(pcsTemplateUsable->m_lEffectApplyCount > 0)
		{
			// 일단 클라이언트에 이 아템이 없어진다고 알려준다.
			// 서버에선 다 사용하고 나중에 없앤다.
			if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_bStackable)
			{
				// 스택 카운터가 하나 줄어든다고 알려준다.
				if (pcsItem->m_nCount > 1)
				{
					SendPacketItemStackCount(pcsItem, pcsItem->m_nCount - 1, m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter));
				}
				else
				{
					SendPacketItemRemove(pcsItem->m_lID, m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter));
				}
			}
			else
			{
				SendPacketItemRemove(pcsItem->m_lID, m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter));
			}

			AddItemToUseList(pcsItem);
			pcsAgsdItem->m_lUseItemRemain = pcsTemplateUsable->m_lEffectApplyCount - 1;
		}
		else
		{
			// 2007.04.04. steeple
			// 한번에 다 채워주는 Potion 이다.
			//
			if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_bStackable)
				m_pagpmItem->SubItemStackCount(pcsItem, 1);
			else
			{
				ZeroMemory(pcsItem->m_szDeleteReason, sizeof(CHAR) * (AGPMITEM_MAX_DELETE_REASON + 1));
				strncpy(pcsItem->m_szDeleteReason, "Use item", AGPMITEM_MAX_DELETE_REASON);

				m_pagpmItem->RemoveItem(pcsItem->m_lID, TRUE);
			}
		}

		// Log - 2004.05.04. steeple
		WriteUseLog(pcsItem->m_pcsCharacter->m_lID, pcsItem);
	}
	else
	{
		// 여러번 사용한 경우

		--pcsAgsdItem->m_lUseItemRemain;

		if (pcsAgsdItem->m_lUseItemRemain == 0)
			return RemoveItemFromUseList(pcsItem);
	}

	pcsAgsdItem->m_ulNextUseItemTime = GetClockCount() + pcsTemplateUsable->m_lEffectActivityTimeMsec;

	return TRUE;
}

BOOL AgsmItem::UseItemTeleportScroll(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	AgpmItemUsableTeleportScrollType eScrollType = ((AgpdItemTemplateUsableTeleportScroll *) pcsItem->m_pcsItemTemplate)->m_eTeleportScrollType;

	// 캐릭터가 죽은경운 사용할 수 없다.
	if (pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return FALSE;
	//JK_거래중금지
	if( pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_TRADE )	
		return FALSE;

	if (m_pagpmCharacter->IsAllBlockStatus(pcsItem->m_pcsCharacter))
		return FALSE;

	AgsdCharacter *pagsdCharacter = m_pagsmCharacter->GetADCharacter(pcsItem->m_pcsCharacter);
	if (NULL == pagsdCharacter)
		return FALSE;
	
	if (eScrollType == AGPMITEM_USABLE_TELEPORT_SCROLL_DESTINATION)
	{
		AuPOS stTeleportPos = ((AgpdItemTemplateUsableTeleportScroll*)pcsItem->m_pcsItemTemplate)->m_stDestination;
		AuPOS stCharacterPos = pcsItem->m_pcsCharacter->m_stPos;

		if( m_papmMap->GetTargetPositionLevelLimit( stTeleportPos ) 
			&& m_papmMap->GetTargetPositionLevelLimit( stCharacterPos ) )
		{
			if(m_pagsmSystemMessage)
				m_pagsmSystemMessage->SendSystemMessage( pcsItem->m_pcsCharacter , AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS );
			
			return FALSE;
		}
	}

	// 현재 변신중인지 본다.
	if (m_pagpmArchlord->IsArchlord(pcsItem->m_pcsCharacter->m_szID) )
	{
		if (eScrollType == AGPMITEM_USABLE_TELEPORT_SCROLL_DESTINATION)
		{
			// 아크로드이고 목적지가 레벨제한 지역이면 못가요.
			AuPOS stTeleportPos = ((AgpdItemTemplateUsableTeleportScroll*)pcsItem->m_pcsItemTemplate)->m_stDestination;

			if( m_papmMap->GetTargetPositionLevelLimit( stTeleportPos ) )
				return FALSE;
		}
	}
	else
	{
		if( pcsItem->m_pcsCharacter->m_bIsTrasform )
			return FALSE;
	}

	if (eScrollType != AGPMITEM_USABLE_TELEPORT_SCROLL_GO_TOWN_NOW)
	{
		if( m_pagpmCharacter->IsCombatMode(pcsItem->m_pcsCharacter) 
			|| (pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_MOVE)
			|| (pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_ATTACK)
			|| (pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_TRADE)
			|| (m_pagpmCharacter->IsActionBlockCondition(pcsItem->m_pcsCharacter))
			)
		{
			if(m_pagsmSystemMessage)
				m_pagsmSystemMessage->SendSystemMessage( pcsItem->m_pcsCharacter , AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS );

			return FALSE;
		}
	}

	// 베틀 스퀘어 안에 있는지 체크해본다. 안에 있으면 사용 못한다.
	if (eScrollType != AGPMITEM_USABLE_TELEPORT_SCROLL_GO_TOWN_NOW 
		&& m_pagpmCharacter->IsInPvPDanger(pcsItem->m_pcsCharacter))
	{
		if(m_pagpmPvP)
			m_pagpmPvP->ProcessSystemMessage(AGPMPVP_SYSTEM_CODE_CANNOT_USE_TELEPORT, NULL, NULL, 0, 0, pcsItem->m_pcsCharacter);

		return FALSE;
	}

	// 카오틱프런티어에서는 모든 이동문서 사용불가 - arycoat 2009.02.23
	if (m_papmMap->GetTargetPositionLevelLimit(pcsItem->m_pcsCharacter->m_stPos))
	{
		if(m_pagsmSystemMessage)
			m_pagsmSystemMessage->SendSystemMessage( pcsItem->m_pcsCharacter , AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_STATUS );

		return FALSE;
	}

	// 레벨 제한 지역 인지를 확인한다.
	// 왕복문서만 
	if( eScrollType == AGPMITEM_USABLE_TELEPORT_SCROLL_RETURN_TOWN )
	{
		// 여기서 사용이 가능한지 플래그 확인
		AgpdCharacter	*pcsCharacter = pcsItem->m_pcsCharacter;
		ApmMap::RegionTemplate*	pTemplate = m_papmMap->GetTemplate( pcsCharacter->m_nBindingRegionIndex );

		if( pTemplate && !pTemplate->ti.stType.bItemWangbok)
		{
			if(m_pagsmSystemMessage)
				m_pagsmSystemMessage->SendSystemMessage( pcsCharacter , AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_WANGBOK_SCROLL );
			return FALSE;
		}
	}

	// 이미 되돌아가야 하는 마을에 있는지 본다.
	if (eScrollType != AGPMITEM_USABLE_TELEPORT_SCROLL_DESTINATION &&
		eScrollType != AGPMITEM_USABLE_TELEPORT_SCROLL_GO_TOWN &&
		pcsItem->m_pcsCharacter->m_nBindingRegionIndex == pcsItem->m_pcsCharacter->m_nLastExistBindingIndex)
		return FALSE;

	// 텔레포트 중이라면 사용할 수 없다.
	if (!pcsItem->m_pcsCharacter->m_bIsAddMap)
		return FALSE;

	// 사용가능한 아이템인지 본다. 2008.04.01. steeple 추가.
	if (!m_pagpmItem->CheckUseItem(pcsItem->m_pcsCharacter, pcsItem))
		return FALSE;

	//////////////////////////////////////////////////////////////////////////
	//
	WriteUseLog(pcsItem->m_pcsCharacter->m_lID, pcsItem);
	m_pagpmItem->SubItemStackCount(pcsItem, 1);

	SendPacketUseItemSuccess(pcsItem->m_pcsCharacter, pcsItem);

	//////////////////////////////////////////////////////////////////////////
	//
	m_pagpmCharacter->SetActionBlockTime(pcsItem->m_pcsCharacter, 3000);
	pagsdCharacter->m_bIsTeleportBlock = TRUE;

	AuPOS				stTeleportPos;
	if (!m_pagpmEventBinding->GetBindingPositionForResurrection(pcsItem->m_pcsCharacter, &stTeleportPos))
	{
		// 실패할 수 없는 코드지만 만약 실패하면 걍 제자리에있는다. ㅡ.ㅡ
		// 이런경운 있음 안된다.
		stTeleportPos	= pcsItem->m_pcsCharacter->m_stPos;
	}

	if (eScrollType == AGPMITEM_USABLE_TELEPORT_SCROLL_RETURN_TOWN)
	{
		AgpdItemADChar	*pcsItemADChar	= m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);

		pcsItemADChar->m_bUseReturnTeleportScroll	= TRUE;
		pcsItemADChar->m_stReturnPosition			= pcsItem->m_pcsCharacter->m_stPos;
	}
	else if (eScrollType == AGPMITEM_USABLE_TELEPORT_SCROLL_DESTINATION)
	{
		stTeleportPos = ((AgpdItemTemplateUsableTeleportScroll*)pcsItem->m_pcsItemTemplate)->m_stDestination;
	}

	pcsItem->m_pcsCharacter->m_stNextAction.m_eActionType = AGPDCHAR_ACTION_TYPE_NONE;

	if (pcsItem->m_pcsCharacter->m_bMove)
		m_pagpmCharacter->StopCharacter(pcsItem->m_pcsCharacter, NULL);

	EnumCallback(AGSMITEM_CB_USE_TELEPORT_SCROLL, pcsItem->m_pcsCharacter, &stTeleportPos);

	/*
	m_pagpmCharacter->UpdatePosition(pcsItem->m_pcsCharacter, &stTeleportPos, FALSE, TRUE);
	*/

	if (eScrollType == AGPMITEM_USABLE_TELEPORT_SCROLL_RETURN_TOWN)
		SendPacketUpdateReturnScrollStatus(pcsItem->m_pcsCharacter, TRUE);

	pagsdCharacter->m_bIsTeleportBlock = FALSE;

	return TRUE;
}

BOOL AgsmItem::UseItemTransform(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	// 글로벌 변신물약사용시 주변캐릭터 넉백 현상 / 공성오브젝트비비기문제 / 등으로 임시로 사용못하게 막음 [ 2011-07-25 silvermoo ]
	if ( AP_SERVICE_AREA_GLOBAL == g_eServiceArea )
	{
		INT16	nPacketLength	= 0;
		PVOID	pvPacket		= MakePacketUseItemResult(pcsItem->m_pcsCharacter, AGPMITEM_USE_RESULT_CANNOT_USE_TRANSFORM_POTION, &nPacketLength);

		if (pvPacket && nPacketLength > sizeof(PACKET_HEADER))
			SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter));

		return FALSE;
	}

	// 변신 아템이다.

	// 죽은넘은 변신할 수 없다.
	if (pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return FALSE;

	// 현재 변신중인지 본다.
	if (pcsItem->m_pcsCharacter->m_bIsTrasform)
		return FALSE;

	// 사용가능한 아이템인지 본다.
	if (!m_pagpmItem->CheckUseItem(pcsItem->m_pcsCharacter, pcsItem))
		return FALSE;

	// 포탈 등의 로딩창이 뜨는 이동 중에는 변신물약 못 쓴다. 2008.04.08. steeple
	if(m_pagsmCharacter->IsNotLogoutStatus(pcsItem->m_pcsCharacter))
		return FALSE;
	// 쉬라인 전투리전에서는 사용못한다.내부 원형 안에서.
	if(m_pagpmShrineBattle->IsInShrineRegion(pcsItem->m_pcsCharacter))
		return FALSE;

	// 레벨 제한 지역에서는 변신물약을 사용할 수 없다.
	switch( m_pagpmCharacter->GetCharacterLevelLimit( pcsItem->m_pcsCharacter ) )
	{
	case	AgpmCharacter::LLS_NONE	:	// 블럭 없음.. 노멀 상황
		break;
	default:
	case	AgpmCharacter::LLS_LIMITED	:	// 레벨 제한걸림.. 걸린 레벨보다 낮음. 경험치&레벨업 정상획득
	case	AgpmCharacter::LLS_BLOCKED	:	// 레벨 제한 걸림.. 경험치는 얻으나 레벨은 못올라감.
	case	AgpmCharacter::LLS_DOWNED	:	// 레벨 제한걸림 .. 레벨이 다운되어 있음.
		return FALSE;
	}

	// check reusable time
	AgpdItemADChar	*pcsAttachItemData	= m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);

	UINT32	ulClockCount	= GetClockCount();

	if (pcsAttachItemData->m_ulUseTransformTime > 0 &&
		pcsAttachItemData->m_ulUseTransformTime + AGPMITEM_TRANSFORM_RECAST_TIME > ulClockCount)
		return FALSE;

	AgpdItemTemplateUsableTransform	*pcsTemplateTransform	= (AgpdItemTemplateUsableTransform *) pcsItem->m_pcsItemTemplate;

	INT32	lCharacterTID	= pcsTemplateTransform->m_lTargetTID;
	AgpdCharacterTemplate* pcsTransformCharacterTemplate = m_pagpmCharacter->GetCharacterTemplate(lCharacterTID);

	if (m_pagpmCharacter->IsCreature(pcsTransformCharacterTemplate) &&
		m_pagpmSiegeWar->IsStarted(m_pagpmSiegeWar->GetSiegeWarInfo(pcsItem->m_pcsCharacter)))
	{
		INT16	nPacketLength	= 0;
		PVOID	pvPacket		= MakePacketUseItemResult(pcsItem->m_pcsCharacter, AGPMITEM_USE_RESULT_CANNOT_TRANSFORM_CREATURE, &nPacketLength);

		if (pvPacket && nPacketLength > sizeof(PACKET_HEADER))
			SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter));

		return FALSE;
	}

	AgsdItemADChar	*pcsAgsdItemADChar = GetADCharacter(pcsItem->m_pcsCharacter);
	if (!pcsAgsdItemADChar)
		return FALSE;

	// Item TID 저장해 놓는다. 2008.03.11. steeple
	pcsItem->m_pcsCharacter->m_lTransformItemTID = pcsItem->m_pcsItemTemplate->m_lID;

	pcsAgsdItemADChar->m_lAddMaxHP	= pcsTemplateTransform->m_lAddMaxHP;
	pcsAgsdItemADChar->m_lAddMaxMP	= pcsTemplateTransform->m_lAddMaxMP;

	m_pagpmCharacter->TransformCharacter(pcsItem->m_pcsCharacter, AGPMCHAR_TRANSFORM_TYPE_APPEAR_STATUS_ALL, m_pagpmCharacter->GetCharacterTemplate(lCharacterTID), &pcsTemplateTransform->m_csEffectFactor);
	pcsItem->m_pcsCharacter->m_bCopyTransformFactor = FALSE;

	if (pcsTemplateTransform->m_ulDuration > 0)
		m_pagsmCharacter->SetTransformTimeout(pcsItem->m_pcsCharacter, pcsTemplateTransform->m_ulDuration);

	SendPacketUseItemSuccess(pcsItem->m_pcsCharacter, pcsItem);

	m_pagpmItem->UpdateTransformReuseTime(pcsItem->m_pcsCharacter, AGPMITEM_TRANSFORM_RECAST_TIME);

	EnumCallback(AGSMITEM_CB_USE_ITEM_TRANSFORM, pcsItem->m_pcsCharacter, pcsItem);

	// Log - 2004.05.04. steeple
	WriteUseLog(pcsItem->m_pcsCharacter->m_lID, pcsItem);

	m_pagpmItem->SubItemStackCount(pcsItem, 1);

	/*
	strncpy(pcsItem->m_szDeleteReason, "변신 아템 사용", AGPMITEM_MAX_DELETE_REASON);

	m_pagpmItem->RemoveItem(pcsItem, TRUE);
	*/
	return TRUE;
}

BOOL AgsmItem::UseItemSkillBook(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	if (EnumCallback(AGSMITEM_CB_USE_ITEM_SKILLBOOK, pcsItem, NULL))
	{
		m_pagpmItem->SubItemStackCount(pcsItem, 1);
	}
	else
		return FALSE;

	return TRUE;
}

BOOL AgsmItem::UseItemSkillScroll(AgpdItem *pcsItem, AgpdCharacter *pcsTargetChar, BOOL bIsWriteLog)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	if(pcsItem->m_pcsCharacter->m_unCurrentStatus != AGPDCHAR_STATUS_IN_GAME_WORLD)
		return FALSE;

	// 사용가능한 아이템인지 본다.
	if (!m_pagpmItem->CheckUseItem(pcsItem->m_pcsCharacter, pcsItem))
		return FALSE;

	// 2005.12.26. steeple
	// 캐쉬 아이템인 경우, 탈것을 타도 가능한 거라면 통과이다.
	if(pcsItem->m_pcsCharacter->m_bRidable ||
		!m_pagpmArchlord->IsArchlord(pcsItem->m_pcsCharacter->m_szID) && pcsItem->m_pcsCharacter->m_bIsTrasform ||
	   m_pagpmCharacter->IsStatusTransparent(pcsItem->m_pcsCharacter))
	{
		if(IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType) && pcsItem->m_pcsItemTemplate->m_bEnableOnRide)
		{}
		else
		{
			return FALSE;
		}
	}

	if (!pcsTargetChar)
		pcsTargetChar	= pcsItem->m_pcsCharacter;

	if (pcsItem->m_pcsCharacter &&
		pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return FALSE;

	// 2006.01.05. steeple
	// 캐쉬 아이템이 이미 적용중인 버프등의 영향으로 인해서 적용이 안되야 하는 지 검사한다.
	if (EnumCallback(AGSMITEM_CB_PRE_CHECK_ENABLE_SKILLSCROLL, pcsItem, pcsTargetChar) == FALSE)
	{
		return FALSE;
	}

	if (EnumCallback(AGSMITEM_CB_USE_ITEM_SKILLSCROLL, pcsItem, pcsTargetChar))
	{
		//m_pagpmItem->SubItemStackCount(pcsItem, 1);

		if (bIsWriteLog && pcsItem->m_pcsCharacter)
		{
			if (IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType))
				WriteUseStartLog(pcsItem->m_pcsCharacter->m_lID, pcsItem, pcsItem->m_nCount);
			else
				WriteUseLog(pcsItem->m_pcsCharacter->m_lID, pcsItem);
		}
	}
	else
		return FALSE;

	return TRUE;
}

BOOL AgsmItem::UseItemSkillRollbackScroll(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	return EnumCallback(AGSMITEM_CB_USE_ITEM_SKILLROLLBACKSCROLL, pcsItem, NULL);
}

BOOL AgsmItem::UseItemReverseOrb(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	// check owner(character) action status
	if (pcsItem->m_pcsCharacter->m_unActionStatus != AGPDCHAR_STATUS_DEAD)
		return FALSE;

	// 아크로드는 사용 못한다. 2007.05.23. steeple
	if (m_pagpmArchlord->IsArchlord(pcsItem->m_pcsCharacter->m_szID))
		return FALSE;

	// 레벨 제한 넣었다. 2008.01.29. steeple
	if (m_pagpmItem->CheckUseItem(pcsItem->m_pcsCharacter, pcsItem) == FALSE)
		return FALSE;

	// 공성지역에서 사용 못한다
	if (m_pagpmSiegeWar->GetSiegeWarInfo(pcsItem->m_pcsCharacter))
		return FALSE;

	// check reusable time
	AgpdItemADChar	*pcsAttachItemData	= m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);

	UINT32	ulClockCount	= GetClockCount();

	if (pcsAttachItemData->m_ulUseReverseOrbTime > 0 &&
		pcsAttachItemData->m_ulUseReverseOrbTime + ((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_ulUseInterval > ulClockCount)
	{
		//시간에 관련된 Error처리 Client에 알려준다
		INT16	nPacketLength	= 0;
		PVOID	pvPacket		= MakePacketUseItemResult(pcsItem->m_pcsCharacter, AGPMITEM_USE_RESULT_FAILED_REVERSEORB, &nPacketLength);

		if ( pvPacket && nPacketLength > sizeof(PACKET_HEADER) )
			SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter));

		return FALSE;
	}

	m_pagpmBattleGround = (AgpmBattleGround*)GetModule("AgpmBattleGround");
	if (!m_pagpmBattleGround )
		return FALSE;

	// 베틀 스퀘어 안에 있는지 체크해본다. 안에 있으면 사용 못한다.
	// 위험 전투지역이라도 캐쉬템은 사용가능 -arycoat 2009.6.11
	if (!IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType) && m_pagpmCharacter->IsInPvPDanger(pcsItem->m_pcsCharacter)
		&& !m_pagpmBattleGround->IsInBattleGround(pcsItem->m_pcsCharacter) )
		return FALSE;

	BOOL bIsTransform = pcsItem->m_pcsCharacter->m_bIsTrasform;

	// use item
	{
		// Criminal Flag가 세팅되어 있다면 없애준다.
		/*
		if (pcsItem->m_pcsCharacter->m_unCriminalStatus == AGPDCHAR_CRIMINAL_STATUS_CRIMINAL_FLAGGED)
		{
			m_pagpmCharacter->UpdateCriminalStatus(pcsItem->m_pcsCharacter, AGPDCHAR_CRIMINAL_STATUS_INNOCENT);
		}
		*/

		// 변신중이면 원래로 되돌린다.
		if (pcsItem->m_pcsCharacter->m_bIsTrasform)
			m_pagpmCharacter->RestoreTransformCharacter(pcsItem->m_pcsCharacter);

		// 지정되어 있는 위치에서 부활시킨다.
		// 위치정보, 캐릭터 Factor 등등의 데이타를 업데이트 한다.
		//
		PVOID pvPacketFactor	= NULL;

		// 2007.03.15. steeple
		// 일반 아이템도 무적 걸어준다.
		// 2006.01.09. steeple
		// 캐쉬 아이템이라면, 일시 무적을 걸어준다.
		//if(IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType))
			m_pagsmCharacter->SetResurrectionOrbInvincible(pcsItem->m_pcsCharacter);

		if (!m_pagsmFactors->Resurrection(&pcsItem->m_pcsCharacter->m_csFactor, &pvPacketFactor))
			return FALSE;

		// 다시 살아났다는 패킷을 보낸다.
		BOOL bSendResult = m_pagsmCharacter->SendPacketFactor(pvPacketFactor, pcsItem->m_pcsCharacter, PACKET_PRIORITY_3);

		if (pvPacketFactor)
			m_pagpmCharacter->m_csPacket.FreePacket(pvPacketFactor);

		// update character status
		m_pagpmCharacter->UpdateActionStatus(pcsItem->m_pcsCharacter, AGPDCHAR_STATUS_NORMAL);
	}

	if (((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_ulUseInterval > 0)
		m_pagpmItem->UpdateReverseOrbReuseTime(pcsItem->m_pcsCharacter, ((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_ulUseInterval);

	EnumCallback(AGSMITEM_CB_USE_ITEM_REVERSE_ORB, pcsItem->m_pcsCharacter, pcsItem);

	// 2006.01.09. steeple
	UseAllEnableCashItem(pcsItem->m_pcsCharacter,
						FALSE,
						AGSDITEM_PAUSE_REASON_NONE,
						bIsTransform);	// 부활 전 상태가 변신 중이었다면, '일시 정지' 상태의 아이템도 사용해준다.

	WriteUseLog(pcsItem->m_pcsCharacter->m_lID, pcsItem);

	m_pagpmItem->SubItemStackCount(pcsItem, 1);

	return TRUE;
}

class	PartyError
{
public:
	PartyError( AgsmSystemMessage * pstAgsmSystemMessage ) : m_pcsAgsmSystemMessage( pstAgsmSystemMessage ) {}

	void	operator()( AgsmParty::RecallPartyError	eError , AgpdCharacter * pcsCharacter , AgpdCharacter * pcsBadMember )
	{
		switch( eError )
		{
		case AgsmParty::DEAD			:
			m_pcsAgsmSystemMessage->SendSystemMessage( pcsCharacter , AGPMSYSTEMMESSAGE_CODE_PARTY_PORTAL_ERROR_DEAD_MEMBER , -1 , -1 , pcsBadMember->m_szID ); break;
			m_pcsAgsmSystemMessage->SendSystemMessage( pcsBadMember , AGPMSYSTEMMESSAGE_CODE_PARTY_PORTAL_ERROR_DEAD_MEMBER , -1 , -1 , pcsBadMember->m_szID ); break;
		case AgsmParty::INSECRETDUNGEON	:
			m_pcsAgsmSystemMessage->SendSystemMessage( pcsCharacter , AGPMSYSTEMMESSAGE_CODE_PARTY_PORTAL_ERROR_IN_SECRET_DUNGEON , -1 , -1 , pcsBadMember->m_szID ); break;
			m_pcsAgsmSystemMessage->SendSystemMessage( pcsBadMember , AGPMSYSTEMMESSAGE_CODE_PARTY_PORTAL_ERROR_IN_SECRET_DUNGEON , -1 , -1 , pcsBadMember->m_szID ); break;
		case AgsmParty::INSIEGE			:
			m_pcsAgsmSystemMessage->SendSystemMessage( pcsCharacter , AGPMSYSTEMMESSAGE_CODE_PARTY_PORTAL_ERROR_IN_SIEGE_WAR , -1 , -1 , pcsBadMember->m_szID ); break;
			m_pcsAgsmSystemMessage->SendSystemMessage( pcsBadMember , AGPMSYSTEMMESSAGE_CODE_PARTY_PORTAL_ERROR_IN_SIEGE_WAR , -1 , -1 , pcsBadMember->m_szID ); break;
		case AgsmParty::LACKOFLEVEL		:
			m_pcsAgsmSystemMessage->SendSystemMessage( pcsCharacter , AGPMSYSTEMMESSAGE_CODE_PARTY_PORTAL_ERROR_LACK_OF_LEVEL , -1 , -1 , pcsBadMember->m_szID ); break;
			m_pcsAgsmSystemMessage->SendSystemMessage( pcsBadMember , AGPMSYSTEMMESSAGE_CODE_PARTY_PORTAL_ERROR_LACK_OF_LEVEL , -1 , -1 , pcsBadMember->m_szID ); break;
		}
	}

private:
	AgsmSystemMessage	*m_pcsAgsmSystemMessage;
};

BOOL AgsmItem::UseItemRecallParty(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	// check owner(character) action status
	if (pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return TRUE;

	// 2008.01.03. steeple
	// 비밀의 던젼 안에서는 사용 못한다.
	if(m_pagpmSiegeWar->IsInSecretDungeon(pcsItem->m_pcsCharacter))
		return TRUE;

	AgpdPartyADChar	*pcsAttachPartyData	= m_pagpmParty->GetADCharacter((PVOID) pcsItem->m_pcsCharacter);
	if (!pcsAttachPartyData->pcsParty)
		return FALSE;

	// 공성중일때는 공성지역내로의 소환이 불가능하다.
	AgpdSiegeWar	*pcsSiegeWar	= m_pagpmSiegeWar->GetSiegeWarInfo(pcsItem->m_pcsCharacter);
	if (pcsSiegeWar && m_pagpmSiegeWar->IsStarted(pcsSiegeWar))
	{
		INT16	nPacketLength	= 0;
		PVOID	pvPacket		= MakePacketUseItemResult(pcsItem->m_pcsCharacter, AGPMITEM_USE_RESULT_FAILED_BY_SIEGEWAR, &nPacketLength);

		if (pvPacket && nPacketLength > sizeof(PACKET_HEADER))
			SendPacket(pvPacket, nPacketLength, m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter));

		return TRUE;
	}

	/*
	AgpdBillInfo	*pcsAttachBillData	= m_pagpmBillInfo->GetADCharacter(pcsItem->m_pcsCharacter);
	if (!pcsAttachBillData->m_bIsPCRoom)
		return FALSE;
	*/

	if( !m_pagsmParty->CheckPartyMemberForRecallParty( pcsItem->m_pcsCharacter , PartyError( m_pagsmSystemMessage ) ) )
	{
		if( m_pagsmParty->RecallPartyMember(pcsItem->m_pcsCharacter) )
		{
			WriteUseLog(pcsItem->m_pcsCharacter->m_lID, pcsItem);
			m_pagpmItem->SubItemStackCount(pcsItem, 1);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

// LotteryBox 2005.9.2 kelovon
BOOL AgsmItem::UseItemLotteryBox(AgpdItem *pcsItem, BOOL bKeyCheck)
{
	AgpdCharacter	*pcsCharacter			= (AgpdCharacter*)pcsItem->m_pcsCharacter;
	AgpdGridItem	*pcsGridItemLotteryKey	= NULL;
	AgpdItem		*pcsAgpmItemLotteryKey	= NULL;

	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	// check owner(character) action status
	if (pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return TRUE;

	if(m_pagpmItem->m_pagpmGrid->IsFullGrid(m_pagpmItem->GetInventory(pcsCharacter))
		|| m_pagpmItem->m_pagpmGrid->IsFullGrid(m_pagpmItem->GetCashInventoryGrid(pcsCharacter)))
	{
		m_pagsmSystemMessage->SendSystemMessage(pcsCharacter, AGPMSYSTEMMESSAGE_CODE_NOT_ENOUGH_INVENTORY);
		return FALSE;
	}

	if ( TRUE == bKeyCheck )
	{
		// 상자 열쇠 확인
		if (pcsItem->m_pcsItemTemplate->m_nSubType == AGPMITEM_USABLE_LOTTERY_TYPE_GOLD
			|| pcsItem->m_pcsItemTemplate->m_nSubType == AGPMITEM_USABLE_LOTTERY_TYPE_SILVER
			|| pcsItem->m_pcsItemTemplate->m_nSubType == AGPMITEM_USABLE_LOTTERY_TYPE_BRONZE
			|| pcsItem->m_pcsItemTemplate->m_nSubType == AGPMITEM_USABLE_LOTTERY_TYPE_PLATINUM)
		{
			pcsGridItemLotteryKey
				= m_pagpmItem->GetCashItemUsableByType(pcsCharacter, AGPMITEM_USABLE_TYPE_LOTTERY_KEY, pcsItem->m_pcsItemTemplate->m_nSubType);
			if (pcsGridItemLotteryKey == NULL)
			{
				if (pcsItem->m_pcsItemTemplate->m_nSubType == AGPMITEM_USABLE_LOTTERY_TYPE_GOLD)
					m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_LOTTERYBOX_NEEDS_GOLD_KEY);
				else if (pcsItem->m_pcsItemTemplate->m_nSubType == AGPMITEM_USABLE_LOTTERY_TYPE_SILVER)
					m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_LOTTERYBOX_NEEDS_SILVER_KEY);
				else if (pcsItem->m_pcsItemTemplate->m_nSubType == AGPMITEM_USABLE_LOTTERY_TYPE_BRONZE)
					m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_LOTTERYBOX_NEEDS_BRONZE_KEY);
				else if (pcsItem->m_pcsItemTemplate->m_nSubType == AGPMITEM_USABLE_LOTTERY_TYPE_PLATINUM)
					m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_LOTTERYBOX_NEEDS_PLATINUM_KEY);

				return FALSE;
			}

			pcsAgpmItemLotteryKey = m_pagpmItem->GetItem(pcsGridItemLotteryKey);
			if (pcsAgpmItemLotteryKey == NULL)
			{
				return FALSE;
			}
		}
	}

	int nProb = m_csRandomNumber.randInt(9999);

	MapLotteryBoxToItems::iterator itr = m_pagpmItem->m_mapLotteryBoxToItems.find(pcsItem->m_pcsItemTemplate->m_lID);
	if (m_pagpmItem->m_mapLotteryBoxToItems.end() == itr)
	{
		return FALSE;
	}

	vector<AgpdLotteryItemInfo> &vectorLotteryItemInfo = itr->second;

	for (int i = 0; i < vectorLotteryItemInfo.size(); i++)
	{
		AgpdLotteryItemInfo *pInfo = &vectorLotteryItemInfo[i];

		if (nProb < pInfo->nPercent)
		{
			int nCount = m_csRandomNumber.randInt(pInfo->nMaxStackCount-pInfo->nMinStackCount + 1) + pInfo->nMinStackCount;

			// 아이템 생성
			AgpdItem* pcsPotItem = CreateItem(pInfo->nPotItemTID, pcsCharacter, nCount);
			if (!pcsPotItem)
			{
				return FALSE;
			}

			// PP Card로 구매한 아이템에서 나온 모든 아이템도 PPCARD속성을 부여해 줘야한다.
			if(pcsItem->m_lStatusFlag & AGPMITEM_STATUS_CASH_PPCARD)
			{
				pcsPotItem->m_lStatusFlag |= AGPMITEM_STATUS_CASH_PPCARD;
			}

			// write log
			WritePickupLog(pcsCharacter->m_lID, pcsPotItem, nCount);

			// 삽입
			if(!m_pagpmItem->AddItemToInventory(pcsCharacter, pcsPotItem))
			{
				m_pagsmSystemMessage->SendSystemMessage(pcsCharacter, AGPMSYSTEMMESSAGE_CODE_NOT_ENOUGH_INVENTORY);
				return FALSE;
			}

			// 아이템 삭제 메시지용 아이템 이름 복사
			char szTempItemName[AGPMITEM_MAX_ITEM_NAME+1];
			strncpy(szTempItemName, pcsItem->m_pcsItemTemplate->m_szName, AGPMITEM_MAX_ITEM_NAME);	// system messasge용으로 복사

			// write log
			WriteUseLog(pcsCharacter->m_lID, pcsItem);

			// 아이템 삭제
			m_pagpmItem->SubItemStackCount(pcsItem, 1);


			// 상자 개방 메시지 전송
			m_pagsmSystemMessage->SendSystemMessage(pcsCharacter,
							AGPMSYSTEMMESSAGE_CODE_LOTTERY_ITEM_TO_POT_ITEM,
							nCount, -1,
							szTempItemName, pcsPotItem->m_pcsItemTemplate->m_szName);

			if (NULL != pcsAgpmItemLotteryKey)
			{
				// write log, 열쇠 아이템 삭제
				WriteUseLog(pcsCharacter->m_lID, pcsAgpmItemLotteryKey);

				// 열쇠 아이템 삭제
				++pcsAgpmItemLotteryKey->m_lCashItemUseCount;
				BOOL bRet = m_pagpmItem->SubItemStackCount(pcsAgpmItemLotteryKey, 1);
				if (!bRet)
				{
					return FALSE;
				}
			}

			break;
		}
	}

	return TRUE;
}

BOOL AgsmItem::UseItemLotteryKey(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
	{
		return FALSE;
	}

	// check owner(character) action status
	if (pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
	{
		return FALSE;
	}

	m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_LOTTERY_KEY_NOT_USABLE);

	return FALSE;
}

BOOL AgsmItem::UseChargingInventory(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter || !pcsItem->m_pcsItemTemplate)
	{
		return FALSE;
	}

	AgpdItemTemplateUsable	*pcsTemplateUsable = (AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate;
	if (pcsTemplateUsable->m_nUsableItemType != AGPMITEM_USABLE_TYPE_CHARGING_BAG)
		return FALSE;

	AgpdItemADChar *pAttachedData = m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);
	if( !pAttachedData )
		return FALSE;

	// 이미 사용하고 있으면 실패
	if (pAttachedData->m_bUseChagingInventory)
		return FALSE;

	switch(pcsItem->m_pcsItemTemplate->m_nSubType)
	{
	case AGPMITEM_USABLE_CHARGING_BAG_TYPE_A:
		{
			pAttachedData->m_eChargingBagType = AGPMITEM_USABLE_CHARGING_BAG_TYPE_A;
		} break;

	case AGPMITEM_USABLE_CHARGING_BAG_TYPE_B:
		{
			pAttachedData->m_eChargingBagType = AGPMITEM_USABLE_CHARGING_BAG_TYPE_B;
		} break;
	}

	pAttachedData->m_bUseChagingInventory = TRUE;

	SendPacketUseItemSuccess(pcsItem->m_pcsCharacter, pcsItem);

	return TRUE;
}

AgpdItem* AgsmItem::GetItemAreaChattingRace(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return NULL;

	// 필요한 아템을 찾아 있으면 스택카운트를 하나 줄인다.

	// 캐시 인벤에서 찾는다.
	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return NULL;

	for(int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nRow; i++)
	{
		AgpdItem* pcsItem = m_pagpmItem->GetItem(m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, 0, i, 0 ));
		if(!pcsItem)
			continue;

		if (pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
			((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_AREA_CHATTING &&
			((AgpdItemTemplateUsableAreaChatting *) pcsItem->m_pcsItemTemplate)->m_eAreaChattingType == AGPMITEM_USABLE_AREA_CHATTING_TYPE_RACE)
		{
			return pcsItem;
		}
	}

	return NULL;
}

// 2006.06.01. steeple
BOOL AgsmItem::UseItemAreaChattingRace(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	// 필요한 아템을 찾아 있으면 스택카운트를 하나 줄인다.
	AgpdItem	*pcsItem	= GetItemAreaChattingRace(pcsCharacter);
	if (!pcsItem)
		return FALSE;

	// 헌것으로 변했음
	++pcsItem->m_lCashItemUseCount;

	if (m_pagpmItem->SubItemStackCount(pcsItem, 1, TRUE) >= 0)
		return TRUE;

	return FALSE;
}

AgpdItem* AgsmItem::GetItemAreaChattingAll(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return NULL;

	// 필요한 아템을 찾아 있으면 스택카운트를 하나 줄인다.

	// 캐시 인벤에서 찾는다.
	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return NULL;

	for(int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nRow; i++)
	{
		AgpdItem* pcsItem = m_pagpmItem->GetItem(m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, 0, i, 0 ));
		if(!pcsItem)
			continue;

		if (pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
			((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_AREA_CHATTING &&
			((AgpdItemTemplateUsableAreaChatting *) pcsItem->m_pcsItemTemplate)->m_eAreaChattingType == AGPMITEM_USABLE_AREA_CHATTING_TYPE_ALL)
		{
			return pcsItem;
		}
	}

	return NULL;
}

BOOL AgsmItem::UseItemAreaChattingAll(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	// 필요한 아템을 찾아 있으면 스택카운트를 하나 줄인다.
	AgpdItem	*pcsItem	= GetItemAreaChattingAll(pcsCharacter);
	if (!pcsItem)
		return FALSE;

	// 헌것으로 변했음
	++pcsItem->m_lCashItemUseCount;

	if (m_pagpmItem->SubItemStackCount(pcsItem, 1, TRUE) >= 0)
		return TRUE;

	return FALSE;
}

AgpdItem* AgsmItem::GetItemAreaChattingGlobal(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return NULL;

	// 필요한 아템을 찾아 있으면 스택카운트를 하나 줄인다.

	// 캐시 인벤에서 찾는다.
	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return NULL;

	for(int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nRow; i++)
	{
		AgpdItem* pcsItem = m_pagpmItem->GetItem(m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, 0, i, 0 ));
		if(!pcsItem)
			continue;

		if (pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
			((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_AREA_CHATTING &&
			((AgpdItemTemplateUsableAreaChatting *) pcsItem->m_pcsItemTemplate)->m_eAreaChattingType == AGPMITEM_USABLE_AREA_CHATTING_TYPE_GLOBAL)
		{
			return pcsItem;
		}
	}

	return NULL;
}

BOOL AgsmItem::UseItemAreaChattingGlobal(AgpdCharacter *pcsCharacter)
{
	AgpdItem	*pcsItem	= GetItemAreaChattingGlobal(pcsCharacter);
	if (!pcsItem)
		return FALSE;

	// 헌것으로 변했음
	++pcsItem->m_lCashItemUseCount;

	if (m_pagpmItem->SubItemStackCount(pcsItem, 1, TRUE) >= 0)
		return TRUE;

	return FALSE;
}

BOOL AgsmItem::UseItemAddBankSlot(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;
	if (g_eServiceArea != AP_SERVICE_AREA_CHINA)
	{
		// 2007.09.18. steeple
		// 2번슬롯 까지 열려 있지 않으면 아이템 사용 못하게 한다.
		if (pcsItem->m_pcsCharacter->m_cBankSize < 1)
		{
			if(m_pagsmSystemMessage)
				m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_BANK_MONEY_FIRST);

			return FALSE;
		}
	}

	// 장궈말고 다른 지역은 3번 슬롯까지만 가능하다.
	if (g_eServiceArea != AP_SERVICE_AREA_CHINA && g_eServiceArea != AP_SERVICE_AREA_GLOBAL)
	{
		if (pcsItem->m_pcsCharacter->m_cBankSize + 1 >= AGPMITEM_BANK_MAX_LAYER - 1)
		{
			if(m_pagsmSystemMessage)
				m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_BANK_SLOT_IS_MAX);

			return FALSE;
		}
	}

	// 확장할 뱅크 슬롯이 있는지 확인해본다.
	if (!m_pagpmItem->IsBuyBankSlot(pcsItem->m_pcsCharacter))
	{
		// 확장할 슬롯이 없다. 클라이언트로 보내준다.
		return FALSE;
	}

	if (!IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType) &&
		pcsItem->m_pcsCharacter->m_cBankSize >= 1)
	{
		// 3번째 슬롯부터는 캐시 아템으로만 확장할 수 있다.		
		return FALSE;
	}

	// 확장 가능하다면 확장해주고 아이템을 소모시킨다.
	if (!m_pagpmCharacter->UpdateBankSize(pcsItem->m_pcsCharacter, pcsItem->m_pcsCharacter->m_cBankSize + 1))
	{
		return FALSE;
	}

	// 성공했다는 메시지도 보내준다.
	if(m_pagsmSystemMessage)
		m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_BANK_ADD_SLOT_SUCCESS);

	return m_pagpmItem->SubItemStackCount(pcsItem, 1, TRUE);;
}

BOOL AgsmItem::UseItemEffect(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	if (m_pagpmItem->SubItemStackCount(pcsItem, 1, TRUE) < 0)
		return FALSE;

	return m_pagsmCharacter->SendPacketCharUseEffect(pcsItem->m_pcsCharacter,
										(AgpdCharacterAdditionalEffect) ((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nSubType,
										pcsItem->m_pcsItemTemplate->m_lExtraType);
}

BOOL AgsmItem::UseItemChatting(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	UINT32 ulClockCount = GetClockCount();
	AgpdItemADChar* pAgpdItemADChar = m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);
	if(!pAgpdItemADChar)
		return FALSE;

	if (pAgpdItemADChar->m_ulUseChattingEmphasis > 0 &&
		pAgpdItemADChar->m_ulUseChattingEmphasis + ((AgpdItemTemplateUsable*) pcsItem->m_pcsItemTemplate)->m_ulUseInterval > ulClockCount)
	{
		if(m_pagsmSystemMessage)
			m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_CANNOT_USE_BY_REUSE_INTERVAL);
		return FALSE;
	}

	pAgpdItemADChar->m_ulUseChattingEmphasis = ulClockCount;

	if (m_pagpmItem->SubItemStackCount(pcsItem, 1, TRUE) < 0)
		return FALSE;

	return TRUE;
}

BOOL AgsmItem::UseItemSkillInitialize(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	// at least 1 or more skill
	if (0 >= m_pagpmEventSkillMaster->GetOwnSkillCount(pcsItem->m_pcsCharacter))
	{
		if(m_pagsmSystemMessage)
			m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_CANNOT_INITIALIZE_SKILL);
		return FALSE;
	}

	if( m_pagpmFactors->GetRace( &pcsItem->m_pcsCharacter->m_csFactor ) == AURACE_TYPE_DRAGONSCION )
	{
		if( m_pagpmFactors->GetClass( &pcsItem->m_pcsCharacter->m_csFactor ) != AUCHARCLASS_TYPE_SCION )
		{
			return FALSE;
		}
	}

	if (!m_pagpmEventSkillMaster->InitializeSkillTree(pcsItem->m_pcsCharacter, TRUE, TRUE)) // Admin TRUE, Cash TRUE
		return FALSE;

	WriteUseLog(pcsItem->m_pcsCharacter->m_lID, pcsItem);

	m_pagpmItem->SubItemStackCount(pcsItem, 1, TRUE);

	return TRUE;
}

// 2007.08.03. steeple
BOOL AgsmItem::UseItemAvatar(AgpdItem* pcsItem)
{
	if(!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	if(!m_pagpmItem->CheckUseItem(pcsItem->m_pcsCharacter, pcsItem))
		return FALSE;

	if(m_pagpmItem->IsEnableEquipAvatarItem(pcsItem))
	{
		// SkillTID 가 세팅되어 있다면, 스킬을 써준다. 스킬 스크롤 함수를 그냥 타본다.
		if(((AgpdItemTemplateUsableSkillScroll*)pcsItem->m_pcsItemTemplate)->m_lSkillTID)
		{
			UseItemSkillScroll(pcsItem, pcsItem->m_pcsCharacter, TRUE);
		}

		if(m_pagpmItem->IsBaseAvatarItem(pcsItem->m_pcsItemTemplate))
		{
			// AvatarSet 모두 만들어서 입혀준다.
			AgpdAvatarSetItem* pAvatarSet = m_pagpmItem->GetAvatarSetTemplateByBase(pcsItem->m_pcsItemTemplate->m_lID);
			if(pAvatarSet)
			{
				vector<INT32>::iterator iter = pAvatarSet->vecItemTIDs.begin();
				while(iter != pAvatarSet->vecItemTIDs.end())
				{
					// Base 아이템은 입지 않는다.
					if(*iter == pcsItem->m_pcsItemTemplate->m_lID)
					{
						++iter;
						continue;
					}

					// DB 아이디는 만들필요 없다.
					AgpdItem* pcsAvatarItem = CreateItem(*iter, pcsItem->m_pcsCharacter, 1, FALSE, 0);
					if(!pcsAvatarItem)
					{
						++iter;
						continue;
					}

					// 아이템을 입힌다.
					if(!m_pagpmItem->EquipItem(pcsItem->m_pcsCharacter, pcsAvatarItem))
					{
						// 못입으면 지워뿐진다.
						m_pagpmItem->RemoveItem(pcsAvatarItem, TRUE);
					}

					++iter;
				}
			}
		}
		else
		{
			// 그냥 얘만 입히면 된다.
			m_pagpmItem->EquipItem(pcsItem->m_pcsCharacter, pcsItem);
		}
	}
	
	return TRUE;
}

// 2009.03.12. iluvs
// 개인거래 옵션 아이템 사용
BOOL AgsmItem::UseItemPrivateTradeOption(AgpdItem* pcsItem)
{
	if(pcsItem == NULL || pcsItem->m_pcsCharacter == NULL)
		return FALSE;

	// 가져옴 김에 타입값 한번 더 검사해준다.
	if(pcsItem->m_eStatus != AGPDITEM_STATUS_TRADE_OPTION_GRID || pcsItem->m_pcsItemTemplate == NULL ||
		pcsItem->m_pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE || 
		((AgpdItemTemplateUsable*)pcsItem->m_pcsItemTemplate)->m_nUsableItemType != AGPMITEM_USABLE_TYPE_PRIVATE_TRADE_OPTION)
	{
		return FALSE;
	}

	AgpdItemADChar *pcsItemADCharacter = m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);
	if(pcsItemADCharacter == NULL)
		return FALSE;

	BOOL bCorrect = FALSE;

	for(int i=0; i<AGPDITEM_PRIVATE_TRADE_OPTION_MAX; i++)
	{
		if(pcsItemADCharacter->m_stTradeOptionItem[i].m_lItemIID != 0)
		{
			if(pcsItem->m_lID == pcsItemADCharacter->m_stTradeOptionItem[i].m_lItemIID)
			{
				pcsItemADCharacter->m_stTradeOptionItem[i].m_lItemIID = 0;
				pcsItemADCharacter->m_stTradeOptionItem[i].m_lItemTID = 0;
				pcsItemADCharacter->m_stTradeOptionItem[i].m_bOnGrid = FALSE;
				pcsItemADCharacter->m_stTradeOptionItem[i].m_bUsed = FALSE;
				pcsItem->m_eStatus = AGPDITEM_STATUS_CASH_INVENTORY;
				bCorrect = TRUE;
				break;
			}
		}
	}

	if(bCorrect)
	{
		INT32 nCount = m_pagpmItem->SubItemStackCount(pcsItem, 1, TRUE);
		if(nCount == -1)
			return FALSE;
	}

	return TRUE;
}

BOOL AgsmItem::UpdateItemDurability(INT32 lIID, INT32 lDurability, INT32 lNewMaxDurability)
{
	return UpdateItemDurability(m_pagpmItem->GetItem(lIID), lDurability, lNewMaxDurability);
}

//		UpdateItemDurability
//	Functions
//		- pcsItem의 durability를 조정한다.
//			1. lDurability 만큼 조정한다.
//			2. 아템 주인이 PC인경우 주인에게 알려준다.
//	Arguments
//		- pcsItem : item data pointer
//		- lDurability : 변경할 내구도
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmItem::UpdateItemDurability(AgpdItem *pcsItem, INT32 lDurability, INT32 lNewMaxDurability)
{
	PROFILE("AgsmItem::UpdateItemDurability");

	if (!pcsItem || (lDurability == 0 && lNewMaxDurability == 0))
		return FALSE;

	AgpdFactor		csUpdateFactor;
	m_pagpmFactors->InitFactor(&csUpdateFactor);

	// pcsItem->m_csFactor 의 ItemDurability 값에 lDurability를 더한다.
	AgpdFactorItem	*pcsFactorItem = (AgpdFactorItem *) m_pagpmFactors->InitCalcFactor(&csUpdateFactor, AGPD_FACTORS_TYPE_ITEM);
	if (!pcsFactorItem)
	{
		m_pagpmFactors->DestroyFactor(&csUpdateFactor);
		return FALSE;
	}

	m_pagpmFactors->SetValue(&csUpdateFactor, lDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);

	// 아이템의 맥스내구도 감소
	// ItemNowDurability ≤ (ItemMaxDurability / 10) 일 경우, 내구도가 감소되면서 아이템의 최대내구도 수치도 감소된다.

	INT32	lMaxDurability	= 0;
	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lMaxDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);

	INT32	lCurrentDurability	= 0;
	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lCurrentDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);

	// lNewMaxDurability 가 0 일때 아래 처리. 0 이 아니면 강제 세팅. 2006.03.26. steeple
	if(lNewMaxDurability == 0)
	{
		if ((FLOAT) (lCurrentDurability + lDurability) <= (lMaxDurability / 10.0f))
			m_pagpmFactors->SetValue(&csUpdateFactor, -(AGSMITEM_REDUCE_DURABILITY), AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);
	}
	else if(lNewMaxDurability > 0)
	{
		m_pagpmFactors->SetValue(&csUpdateFactor, lNewMaxDurability - lMaxDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);
	}

	BOOL	bSendResult	= TRUE;

	if (m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter))
	{
		// 업데이트 시키고 변경된 값을 아템 주인에게 보내준다.
		PVOID	pvPacketFactor = m_pagpmFactors->CalcFactor(&pcsItem->m_csFactor, &csUpdateFactor, TRUE, TRUE, TRUE, FALSE);
		if (!pvPacketFactor)
		{
			m_pagpmFactors->DestroyFactor(&csUpdateFactor);

			return FALSE;
		}

		bSendResult = SendPacketItemFactor(pcsItem, pvPacketFactor, NULL, m_pagsmCharacter->GetCharDPNID(pcsItem->m_pcsCharacter));

		m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);
	}
	else
	{
		m_pagpmFactors->CalcFactor(&pcsItem->m_csFactor, &csUpdateFactor, TRUE, FALSE, TRUE, FALSE);
	}

	m_pagpmFactors->DestroyFactor(&csUpdateFactor);

	// 내구도가 0인것은 equip 상태라도 무시한다.
	// 장비중인 아이템만 캐릭터 능력치를 조절한다
	if (lCurrentDurability + lDurability < 1 && pcsItem->m_pcsCharacter && pcsItem->m_eStatus == AGPDITEM_STATUS_EQUIP)
	{
		if ( m_pagpmFactors->GetRace( &pcsItem->m_pcsCharacter->m_csFactor ) == AURACE_TYPE_DRAGONSCION )
		{
			AgpdItem* pcsItemL		= m_pagpmItem->GetEquipSlotItem(pcsItem->m_pcsCharacter, AGPMITEM_PART_HAND_LEFT);
			AgpdItem* pcsItemR		= m_pagpmItem->GetEquipSlotItem(pcsItem->m_pcsCharacter, AGPMITEM_PART_HAND_RIGHT);

			if (pcsItemL && pcsItemR)
			{
				AgpdFactor UpdateFactorPoint;
				AgpdFactor UpdateFactorPercent;

				m_pagpmFactors->InitFactor(&UpdateFactorPoint);
				m_pagpmFactors->InitFactor(&UpdateFactorPercent);

				m_pagpmFactors->CopyFactor(&UpdateFactorPoint, &pcsItem->m_csFactor, TRUE);
				m_pagpmFactors->CopyFactor(&UpdateFactorPercent, &pcsItem->m_csFactorPercent, TRUE);

				if ( m_pagpmItem->GetWeaponType(pcsItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_ZENON )
				{
					INT32 lLeftDurability = -1;
					m_pagpmFactors->GetValue(&pcsItemL->m_csFactor, &lLeftDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);

					if (0 < lLeftDurability)
					{
						// 오비터일때는 0셋팅 하면 안댐
						INT32		lClass	= (INT32) AUCHARCLASS_TYPE_NONE;
						m_pagpmFactors->GetValue(&pcsItem->m_pcsCharacter->m_csFactor, &lClass, AGPD_FACTORS_TYPE_CHAR_TYPE, AGPD_FACTORS_CHARTYPE_TYPE_CLASS);

						if (lClass != AUCHARCLASS_TYPE_RANGER) // 드래곤시온 오비터체크
						{
							m_pagpmFactors->SetValue(&UpdateFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
							m_pagpmFactors->SetValue(&UpdateFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
						}

						m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPoint, &UpdateFactorPoint, FALSE, FALSE, FALSE, FALSE);
						m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPercent, &UpdateFactorPercent, FALSE, FALSE, FALSE, FALSE);
					}
					else if (0 == lLeftDurability)
					{
						if (pcsItem->m_pcsCharacter->m_bIsEvolution)
						{
							INT32		lClass	= (INT32) AUCHARCLASS_TYPE_NONE;
							m_pagpmFactors->GetValue(&pcsItem->m_pcsCharacter->m_csFactor, &lClass, AGPD_FACTORS_TYPE_CHAR_TYPE, AGPD_FACTORS_CHARTYPE_TYPE_CLASS);
							if (lClass == AUCHARCLASS_TYPE_MAGE) // 드래곤시온 서머너체크
							{
								m_pagpmFactors->SetValue(&UpdateFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
								m_pagpmFactors->SetValue(&UpdateFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
							}

							m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPoint, &UpdateFactorPoint, FALSE, FALSE, FALSE, FALSE);
							m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPercent, &UpdateFactorPercent, FALSE, FALSE, FALSE, FALSE);
						}
						else
						{
							m_pagpmFactors->SetValue(&UpdateFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
							m_pagpmFactors->SetValue(&UpdateFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);

							m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPoint, &UpdateFactorPoint, FALSE, FALSE, FALSE, FALSE);
							m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPercent, &UpdateFactorPercent, FALSE, FALSE, FALSE, FALSE);
						}
					}
				}
				else if ( m_pagpmItem->GetWeaponType(pcsItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_CHARON)
				{
					INT32 lRightDurability = -1;
					m_pagpmFactors->GetValue(&pcsItemR->m_csFactor, &lRightDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);

					if (0 < lRightDurability)
					{
						// 슬레이어일때는 0셋팅 하면 안댐
						INT32		lClass	= (INT32) AUCHARCLASS_TYPE_NONE;
						m_pagpmFactors->GetValue(&pcsItem->m_pcsCharacter->m_csFactor, &lClass, AGPD_FACTORS_TYPE_CHAR_TYPE, AGPD_FACTORS_CHARTYPE_TYPE_CLASS);

						if (lClass != AUCHARCLASS_TYPE_KNIGHT) // 드래곤시온 슬레이어체크
						{
							m_pagpmFactors->SetValue(&UpdateFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
							m_pagpmFactors->SetValue(&UpdateFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
						}

						m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPoint, &UpdateFactorPoint, FALSE, FALSE, FALSE, FALSE);
						m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPercent, &UpdateFactorPercent, FALSE, FALSE, FALSE, FALSE);
					}
					else if (0 == lRightDurability)
					{
						if (pcsItem->m_pcsCharacter->m_bIsEvolution)
						{
							INT32		lClass	= (INT32) AUCHARCLASS_TYPE_NONE;
							m_pagpmFactors->GetValue(&pcsItem->m_pcsCharacter->m_csFactor, &lClass, AGPD_FACTORS_TYPE_CHAR_TYPE, AGPD_FACTORS_CHARTYPE_TYPE_CLASS);
							if (lClass == AUCHARCLASS_TYPE_MAGE) // 드래곤시온 서머너체크
							{
								m_pagpmFactors->SetValue(&UpdateFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
								m_pagpmFactors->SetValue(&UpdateFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
							}

							m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPoint, &UpdateFactorPoint, FALSE, FALSE, FALSE, FALSE);
							m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPercent, &UpdateFactorPercent, FALSE, FALSE, FALSE, FALSE);
						}
						else
						{
							m_pagpmFactors->SetValue(&UpdateFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
							m_pagpmFactors->SetValue(&UpdateFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);

							m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPoint, &UpdateFactorPoint, FALSE, FALSE, FALSE, FALSE);
							m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPercent, &UpdateFactorPercent, FALSE, FALSE, FALSE, FALSE);
						}
					}
				}
			}
			else
			{
				m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPoint, &pcsItem->m_csFactor, FALSE, FALSE, FALSE, FALSE);
				m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPercent, &pcsItem->m_csFactorPercent, FALSE, FALSE, FALSE, FALSE);
			}
		}
		else
		{
			m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPoint, &pcsItem->m_csFactor, FALSE, FALSE, FALSE, FALSE);
			m_pagpmFactors->CalcFactor(&pcsItem->m_pcsCharacter->m_csFactorPercent, &pcsItem->m_csFactorPercent, FALSE, FALSE, FALSE, FALSE);
		}
		
		// 내구도 0됐을때 세트아이템 능력치도 줄인다.
		CalcItemOptionSkillData(pcsItem, FALSE);

		CalcItemOptionIgnoreStatus(pcsItem->m_pcsCharacter, pcsItem, FALSE);

		AdjustRecalcFactor(pcsItem->m_pcsCharacter, pcsItem, FALSE);

		m_pagsmCharacter->ReCalcCharacterResultFactors(pcsItem->m_pcsCharacter);
	}

	EnumCallback(AGSMITEM_CB_UPDATE_ITEM_DURABILITY, pcsItem, &lDurability);

	return bSendResult;
}

//		ReduceDurability
//	Functions
//		- pcsItem의 durability를 조정한다.(줄인다)
//			1. CheckItemDurability()로 줄여야 하는지를 체크한다.
//			2. 줄여야 한다면 정해진만큼 줄인다.
//	Arguments
//		- pcsItem : item data pointer
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmItem::ReduceDurability(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	if (!pcsItem->m_pcsCharacter)
	{
		if (pcsItem->m_ulCID == AP_INVALID_CID)
			return FALSE;

		pcsItem->m_pcsCharacter = m_pagpmCharacter->GetCharacter(pcsItem->m_ulCID);
		if (!pcsItem->m_pcsCharacter)
			return FALSE;
	}

	// 2007.11.03. steeple
	// Rapier 이고 왼손에 Dagger 가 있다면 둘 중 하나만 랜덤하게 줄인다.
	if(m_pagpmItem->GetWeaponType(pcsItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_RAPIER)
	{
		AgpdItem* pcsLeftItem = m_pagpmItem->GetEquipSlotItem(pcsItem->m_pcsCharacter, AGPMITEM_PART_HAND_LEFT);
		if(pcsLeftItem && pcsLeftItem->m_pcsItemTemplate &&
			m_pagpmItem->GetWeaponType(pcsLeftItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_DAGGER)
		{
			if(CheckItemDurability(pcsLeftItem))
				return UpdateItemDurability(pcsLeftItem, -(AGSMITEM_REDUCE_DURABILITY));
		}
	}
	else if(m_pagpmItem->GetWeaponType(pcsItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_ZENON 
		|| m_pagpmItem->GetWeaponType(pcsItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_ZENON)
	{
		AgpdItem* pcsLeftItem = m_pagpmItem->GetEquipSlotItem(pcsItem->m_pcsCharacter, AGPMITEM_PART_HAND_LEFT);
		if(pcsLeftItem && pcsLeftItem->m_pcsItemTemplate &&
			(m_pagpmItem->GetWeaponType(pcsLeftItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_CHARON
			|| m_pagpmItem->GetWeaponType(pcsLeftItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_CHARON)
			)
		{
			if(CheckItemDurability(pcsLeftItem))
				return UpdateItemDurability(pcsLeftItem, -(AGSMITEM_REDUCE_DURABILITY));
		}
	}

	if (!CheckItemDurability(pcsItem))
		return TRUE;

	// AGSMITEM_REDUCE_DURABILITY만큼 줄인다.
	return UpdateItemDurability(pcsItem, -(AGSMITEM_REDUCE_DURABILITY));
}

//		ReduceArmourDurability
//	Functions
//		- pcsCharacter의 Equip Item 중 하나를 택해 그 아이템의 durability를 조정한다.(줄인다)
//			1. Equip Part중 하나를 정해진 확률에 따라 구한다.
//			2. 정해진 파트에 아템이 착용되어 있다면 그 아템을 ReduceWeaponDurability()에 넘겨준다.
//	Arguments
//		- pcsCharacter : AgpdCharacter data pointer
//	Return value
//		- BOOL : 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmItem::ReduceArmourDurability(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter || !m_pagpmCharacter->IsPC(pcsCharacter))
		return FALSE;

	INT32			lItemID;
	INT32			lRandomNumber	= 0;

	/*
	INT32			lCharacterClass		= (INT32) AUCHARCLASS_TYPE_NONE;
	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lCharacterClass, AGPD_FACTORS_TYPE_CHAR_TYPE, AGPD_FACTORS_CHARTYPE_TYPE_CLASS);

	switch ((AuCharClassType) lCharacterClass) {
	case AUCHARCLASS_TYPE_KNIGHT:
		{
			lRandomNumber	= m_csRandomNumber.randInt(120);
		}
		break;

	case AUCHARCLASS_TYPE_MAGE:
	case AUCHARCLASS_TYPE_RANGER:
	case AUCHARCLASS_TYPE_MONK:
		{
			lRandomNumber	= m_csRandomNumber.randInt(100);
		}
		break;
	}
	*/

	lRandomNumber	= m_csRandomNumber.randInt(108) + 1;

	// 방어구 파트를 구한다.
	lItemID = 0;

	if (lRandomNumber <= 25)
	{
		return ReduceDurability(m_pagpmItem->GetEquipSlotItem( pcsCharacter, AGPMITEM_PART_BODY ));
	}
	else if (lRandomNumber <= 46)
	{
		return ReduceDurability(m_pagpmItem->GetEquipSlotItem( pcsCharacter, AGPMITEM_PART_LEGS ));
	}
	else if (lRandomNumber <= 62)
	{
		return ReduceDurability(m_pagpmItem->GetEquipSlotItem( pcsCharacter, AGPMITEM_PART_HEAD ));
	}
	else if (lRandomNumber <= 76)
	{
		return ReduceDurability(m_pagpmItem->GetEquipSlotItem( pcsCharacter, AGPMITEM_PART_HANDS ));
	}
	else if (lRandomNumber <= 89)
	{
		return ReduceDurability(m_pagpmItem->GetEquipSlotItem( pcsCharacter, AGPMITEM_PART_FOOT ));
	}
//	else if (lRandomNumber <= 100)
//	{
//		return ReduceDurability(m_pagpmItem->GetEquipSlotItem( pcsCharacter, AGPMITEM_PART_HANDS ));
//	}
	else
	{
		AgpdItem	*pcsLeftEquip	= m_pagpmItem->GetEquipSlotItem( pcsCharacter, AGPMITEM_PART_HAND_LEFT );
		if (pcsLeftEquip &&
			((AgpdItemTemplateEquip *) pcsLeftEquip->m_pcsItemTemplate)->m_nKind == AGPMITEM_EQUIP_KIND_SHIELD)
		{
			return ReduceDurability(pcsLeftEquip);
		}
	}

	return TRUE;
}

//		CheckItemDurability
//	Functions
//		- pcsItem의 현재 내구도를 보고 내구도를 줄여야 하는지 체크한다.
//			(1 / 현재 내구도 * 4) 의 확률이다.
//	Arguments
//		- pcsItem : AgpdItem Data Pointer
//	Return value
//		- BOOL : 줄여야 하는지 여부 (TRUE면 줄이고 FALSE면 냅둔다.
///////////////////////////////////////////////////////////////////////////////
BOOL AgsmItem::CheckItemDurability(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	// 아크로드 일 때는 내구도 안 줄인다. 2007.04.27. steeple
	if (m_pagpmCharacter->IsArchlord(pcsItem->m_pcsCharacter))
		return FALSE;

	INT32	lCurrentDurability = 0;

	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lCurrentDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);
	
	if (lCurrentDurability == 0)
		return FALSE;

	FLOAT fTypeValue = GetReduceTypeValue(pcsItem);
	if(fTypeValue == 0)
		return FALSE;

	if (m_csRandomNumber.rand(lCurrentDurability) <= (lCurrentDurability * 0.02 / fTypeValue))
		return TRUE;

	/*
	if (m_csRandomNumber.randInt(lCurrentDurability * 4) == lCurrentDurability)
		return TRUE;
	*/

	return FALSE;
}

FLOAT AgsmItem::GetReduceTypeValue(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsItemTemplate)
		return 0;

	if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_nType != AGPMITEM_TYPE_EQUIP)
		return 0;

	switch (((AgpdItemTemplateEquip *) pcsItem->m_pcsItemTemplate)->m_nKind) {
	case AGPMITEM_EQUIP_KIND_ARMOUR:
		{
			switch (((AgpdItemTemplateEquip *) pcsItem->m_pcsItemTemplate)->m_nPart) {
			case AGPMITEM_PART_BODY:
				return (FLOAT) 1.4;
				break;

			case AGPMITEM_PART_LEGS:
				return (FLOAT) 1.2;
				break;

			case AGPMITEM_PART_HEAD:
				return (FLOAT) 1.1;
				break;

			case AGPMITEM_PART_ARMS:
				return (FLOAT) 1.0;
				break;

			case AGPMITEM_PART_FOOT:
				return (FLOAT) 1.1;
				break;

			case AGPMITEM_PART_HANDS:
				return (FLOAT) 1.2;
				break;
			}
		}
		break;

	case AGPMITEM_EQUIP_KIND_SHIELD:
		return (FLOAT) 1.3;
		break;

	case AGPMITEM_EQUIP_KIND_WEAPON:
		{
			switch (((AgpdItemTemplateEquipWeapon *) pcsItem->m_pcsItemTemplate)->m_nWeaponType) {
			case AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_SWORD:
				return (FLOAT) 1;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_AXE:
				return (FLOAT) 1.2;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_MACE:
				return (FLOAT) 1.3;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_HAMMER:
				return (FLOAT) 1.5;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_SWORD:
				return (FLOAT) 1.2;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_HAMMER:
				return (FLOAT) 1.2;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_POLEARM:
				return (FLOAT) 1.4;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_SCYTHE:
				return (FLOAT) 1.4;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_CLAW:
				return (FLOAT) 1.1;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_WING:
				return (FLOAT) 1.2;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_LANCE:
				return (FLOAT) 1;
				break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_RAPIER:
			case AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_DAGGER:
				{
					return (FLOAT) 0.8;
				} break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_CHARON:
			case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_CHARON:
				{
					return (FLOAT) 0.6;
				} break;

			case AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_ZENON:
			case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_ZENON:
				{
					return (FLOAT) 0.6;
				} break;

			default:
				return (FLOAT) 1;
				break;
			}
		}
		break;
	}

	return (FLOAT) 0;
}

BOOL AgsmItem::OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)
{
	if (!pvPacket ||
		nSize <= 0 ||
		ulNID == 0)
		return FALSE;

	if (!pstCheckArg->bReceivedFromServer)
		return FALSE;

	PVOID		pvPacketData			= NULL;
	PVOID		pvPacketADChar			= NULL;
	UINT32		ulReceivedClockCount	= 0;
	UINT32		ulCurrentClockCount		= GetClockCount();

	CHAR		*szAgpdItemPacket		= NULL;
	UINT16		unAgpdItemPacketSize	= 0;

	UINT64		ullDBID					= 0;

	CHAR		*szAgpdItemConvertPacket	= NULL;
	UINT16		unAgpdItemConvertPacketSize	= 0;

	m_csPacket.GetField(TRUE, pvPacket, nSize,
							&pvPacketData,
							&pvPacketADChar,
							&ulReceivedClockCount,
							&szAgpdItemPacket, &unAgpdItemPacketSize,
							&ullDBID,
							&szAgpdItemConvertPacket, &unAgpdItemConvertPacketSize);

	if (pvPacketData)
	{
		INT32		lIID					= AP_INVALID_IID;
		UINT64		ullDBIID				= 0;
		UINT32		ulOwnTime				= 0;
		INT32		lUseItemRemain			= 0;
		UINT32		ulNextUseItemTime		= 0;
		UINT32		ulDropTime				= 0;
		PVOID		pvPacketFirstLooter		= NULL;
		BOOL		bIsNeedInsertDB			= FALSE;

		m_csPacketData.GetField(FALSE, pvPacketData, 0,
							&lIID,
							&ullDBIID,
							&ulOwnTime,
							&lUseItemRemain,
							&ulNextUseItemTime,
							&ulDropTime,
							&pvPacketFirstLooter,
							&bIsNeedInsertDB);

		AgpdItem	*pcsItem	= m_pagpmItem->GetItemLock(lIID);
		if (pcsItem)
		{
			// 2005.05.10. steeple
			if(!AddDBIDAdmin(ullDBIID))
			{
				char strCharBuff[256] = { 0, };
				sprintf_s(strCharBuff, sizeof(strCharBuff), "Item DBID Error :%I64d (TID : %d)\n", ullDBID, pcsItem->m_lTID);
				AuLogFile_s("LOG\\ItemDBIDErr.log", strCharBuff);
				
				// DBID Admin 에 이미 아이템이 있다.
				AgsdItem* pcsAgsdItem = GetADItem(pcsItem);
				if(pcsAgsdItem)
					pcsAgsdItem->m_ullDBIID = 0;

				pcsItem->m_Mutex.Release();
				m_pagpmItem->RemoveItem(lIID, FALSE);
				return FALSE;
			}

			AgpdCharacter	*pcsCharacter = m_pagpmCharacter->GetCharacterLock(pcsItem->m_ulCID);

			AgsdItem	*pcsAgsdItem	= GetADItem(pcsItem);
			
			pcsAgsdItem->m_ullDBIID				= ullDBIID;
			pcsAgsdItem->m_ulOwnTime			= ulCurrentClockCount + (ulOwnTime - ulReceivedClockCount);
			pcsAgsdItem->m_lUseItemRemain		= lUseItemRemain;
			pcsAgsdItem->m_ulNextUseItemTime	= ulCurrentClockCount + (ulNextUseItemTime - ulReceivedClockCount);
			pcsAgsdItem->m_ulDropTime			= ulCurrentClockCount + (ulDropTime - ulReceivedClockCount);
			pcsAgsdItem->m_bIsNeedInsertDB		= bIsNeedInsertDB;

			if (pvPacketFirstLooter)
			{
				INT8	cType	= 0;
				INT32	lID		= 0;

				m_pagsmCharacter->m_csPacketBase.GetField(FALSE, pvPacketFirstLooter, 0,
															&cType,
															&lID);

				pcsAgsdItem->m_csFirstLooterBase.m_eType = (ApBaseType) cType;
				pcsAgsdItem->m_csFirstLooterBase.m_lID = lID;
			}

			if (pcsCharacter)
				pcsCharacter->m_Mutex.Release();

			pcsItem->m_Mutex.Release();
		}

		EnumCallback(AGSMITEM_CB_RECEIVE_ITEM_DATA, &lIID, NULL);
	}

	if (pvPacketADChar)
	{
		INT32		lCID					= AP_INVALID_CID;
//		INT32		alUseItem[AGPMITEM_MAX_USE_ITEM];
		INT32		lNumUseItem				= 0;
		UINT32		ulUseHPPotionTime		= 0;
		UINT32		ulUseMPPotionTime		= 0;
		UINT32		ulUseSPPotionTime		= 0;
		UINT32		ulUseReverseOrbTime		= 0;
		UINT32		ulUseTransformTime		= 0;

//		ZeroMemory(alUseItem, sizeof(INT32) * AGPMITEM_MAX_USE_ITEM);

		ApSafeArray<INT32, AGPMITEM_MAX_USE_ITEM>	alUseItem;
		alUseItem.MemSetAll();

		m_csPacketADChar.GetField(FALSE, pvPacketADChar, 0,
							&lCID,
							&alUseItem[0],
							&lNumUseItem,
							&ulUseHPPotionTime,
							&ulUseMPPotionTime,
							&ulUseSPPotionTime,
							&ulUseReverseOrbTime,
							&ulUseTransformTime);

		AgpdCharacter	*pcsCharacter	= m_pagpmCharacter->GetCharacterLock(lCID);
		if (pcsCharacter)
		{
			AgpdItemADChar	*pcsAgpdItemADChar	= m_pagpmItem->GetADCharacter(pcsCharacter);
			AgsdItemADChar	*pcsItemADChar		= GetADCharacter(pcsCharacter);

			pcsItemADChar->m_lNumUseItem		= lNumUseItem;

			if (ulUseHPPotionTime > 0)
				pcsAgpdItemADChar->m_ulUseHPPotionTime	= ulCurrentClockCount + (ulUseHPPotionTime - ulReceivedClockCount);
			if (ulUseMPPotionTime > 0)
				pcsAgpdItemADChar->m_ulUseMPPotionTime	= ulCurrentClockCount + (ulUseMPPotionTime - ulReceivedClockCount);
			if (ulUseSPPotionTime > 0)
				pcsAgpdItemADChar->m_ulUseSPPotionTime	= ulCurrentClockCount + (ulUseSPPotionTime - ulReceivedClockCount);

			for (int i = 0; i < AGPMITEM_MAX_USE_ITEM; ++i)
			{
				pcsItemADChar->m_lUseItemID[i] = alUseItem[i];
			}

//			m_pagpmItem->UpdateReverseOrbReuseTime(pcsCharacter, ulUseReverseOrbTime);
//			m_pagpmItem->UpdateTransformReuseTime(pcsCharacter, ulUseTransformTime);

			pcsCharacter->m_Mutex.Release();
		}
	}
	
	if (szAgpdItemPacket && unAgpdItemPacketSize > 0)
	{
		INT8		cOperation = -1;
		INT8        cStatus    = -2;
		INT32		lIID       =  0;
		INT32		lTID       =  0;
		INT32       lCID       =  0;
		INT32		lItemCount = -1;
		PVOID       pField		= NULL;
		PVOID       pInventory	= NULL;
		PVOID		pBank		= NULL;
		PVOID       pEquip		= NULL;
		PVOID		pFactor		= NULL;
		PVOID		pFactorPercent	= NULL;
		INT32		lTargetID	= 0;
		PVOID		pConvert	= NULL;
		PVOID		pRestrictFactor	= NULL;
		PVOID		pEgo		= NULL;
		PVOID		pQuest		= NULL;
		INT32		lSkillTID	= 0;
		UINT32		ulReuseTimeForReverseOrb	= 0;
		INT32		lStatusFlag	= (-1);
		PVOID		pOption		= NULL;
		PVOID		pSkillPlus	= NULL;
		UINT32		ulReuseTimeForTransform		= 0;
		PVOID		pvPacketCashInformation	= NULL;
		PVOID		pvPacketExtra = NULL;

		m_pagpmItem->m_csPacket.GetField(TRUE, (PVOID) szAgpdItemPacket, unAgpdItemPacketSize,  
							&cOperation,
							&cStatus,
							&lIID,
							&lTID,
							&lCID,
							&lItemCount,
							&pField,
							&pInventory,
							&pBank,
							&pEquip,
							&pFactor,
							&pFactorPercent,
							&lTargetID,
							&pConvert,
							&pRestrictFactor,
							&pEgo,
							&pQuest,
							&lSkillTID,
							&ulReuseTimeForReverseOrb,
							&lStatusFlag,
							&pOption,
							&pSkillPlus,
							&ulReuseTimeForTransform,
							&pvPacketCashInformation,
							&pvPacketExtra);

		AgpdCharacter	*pcsCharacter	= m_pagpmCharacter->GetCharacterLock(lCID);

		INT32	lNewIID = 0;
		EnumCallback(AGSMITEM_CB_GET_NEW_IID, &lNewIID, NULL);

		m_pagpmItem->OnOperationAdd(TRUE, cStatus, lNewIID, lTID, pcsCharacter, lItemCount, pField, pInventory, pBank, pEquip, pFactor, pFactorPercent, pRestrictFactor, pConvert, pQuest, lSkillTID, lStatusFlag, pOption, pSkillPlus, pvPacketCashInformation, pvPacketExtra);

		AgpdItem	*pcsItem		= m_pagpmItem->GetItem(lNewIID);
		if (pcsItem)
		{
			INT32	lDurability	= 0;
			INT32	lMaxDurability = 0;
			m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);
			m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lMaxDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);

			m_pagpmFactors->InitFactor(&pcsItem->m_csFactor);
			m_pagpmFactors->InitFactor(&pcsItem->m_csFactorPercent);
			m_pagpmFactors->InitFactor(&pcsItem->m_csRestrictFactor);

			m_pagpmFactors->CopyFactor(&pcsItem->m_csFactor, &pcsItem->m_pcsItemTemplate->m_csFactor, FALSE);
			m_pagpmFactors->CopyFactor(&pcsItem->m_csRestrictFactor, &pcsItem->m_pcsItemTemplate->m_csRestrictFactor, FALSE);

			m_pagpmItem->CalcItemOptionFactor(pcsItem);

			m_pagpmFactors->SetValue(&pcsItem->m_csFactor, lDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);
			m_pagpmFactors->SetValue(&pcsItem->m_csFactor, lMaxDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);

			// stackable 아이템이 아니었다가 stackable로 된 경우 DB에 0으로 저장되어 있던 놈을 1로 바꾼다.
			if (pcsItem->m_pcsItemTemplate->m_bStackable &&
				pcsItem->m_nCount == 0 &&
				(pcsItem->m_eStatus == AGPDITEM_STATUS_INVENTORY ||
				pcsItem->m_eStatus == AGPDITEM_STATUS_BANK))
				pcsItem->m_nCount = 1;

			AgsdItem	*pcsAgsdItem	= GetADItem(pcsItem);

			// 2005.05.10. steeple
			if(!AddDBIDAdmin(ullDBID))
			{
				char strCharBuff[256] = { 0, };
				sprintf_s(strCharBuff, sizeof(strCharBuff), "Item DBID Error :%I64d (TID : %d)\n", ullDBID, pcsItem->m_lTID);
				AuLogFile_s("LOG\\ItemDBIDErr.log", strCharBuff);
				
				// DBID Admin 에 이미 아이템이 있다.
				AgsdItem* pcsAgsdItem = GetADItem(pcsItem);
				if(pcsAgsdItem)
					pcsAgsdItem->m_ullDBIID = 0;

				m_pagpmItem->RemoveItem(lNewIID, FALSE);
				return FALSE;
			}

			pcsAgsdItem->m_ullDBIID		= ullDBID;

			if (szAgpdItemConvertPacket &&
				unAgpdItemConvertPacketSize > 0)
			{
				m_pagpmItemConvert->DispatchAddPacket(pcsCharacter, pcsItem, (PVOID) szAgpdItemConvertPacket, unAgpdItemConvertPacketSize);

				m_pagpmItemConvert->CalcConvertFactor(pcsItem);

				/*if(pcsItem->m_eStatus == AGPDITEM_STATUS_EQUIP)
				{
					AgpdItemConvertADItem* pcsAttachItemData = m_pagpmItemConvert->GetADItem(pcsItem);
					AgpdItemConvertADChar* pcsAttachCharData = m_pagpmItemConvert->GetADChar(pcsItem->m_pcsCharacter);

					if(!pcsAttachCharData || !pcsAttachItemData)
						return FALSE;

					pcsAttachCharData->m_stOptionSkillData += pcsAttachItemData->m_stOptionSkillData;
				}*/
			}
		}

		else
		{
			::InterlockedIncrement(&m_lUnaddedItemCount);
		
			CHAR szFile[MAX_PATH];
			SYSTEMTIME st;
			GetLocalTime(&st);

			sprintf(szFile, "LOG\\AgsmItem-%04d%02d%02d.log", st.wYear, st.wMonth, st.wDay);
			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "[%02d:%02d:%02d]"
						"Status[%d], ID[%d], TID[%d], Owner[%s], No[%d]\n",
						st.wHour, st.wMinute, st.wSecond,
						cStatus, lNewIID, lTID, pcsCharacter ? pcsCharacter->m_szID : "", lItemCount);
			AuLogFile_s(szFile, strCharBuff);
		}
		if (pcsCharacter)
			pcsCharacter->m_Mutex.Release();
	}

	return TRUE;
}

// 2007.08.07. steeple
// AgsmItemManager::CreateItem 을 부른다.
AgpdItem* AgsmItem::CreateItem(INT32 lTID, AgpdCharacter* pcsCharacter, INT32 lCount, BOOL bGenerateDBID, INT32 lSkillTID)
{
	if(lTID < 1)
		return NULL;

	AgpdItem* pcsItem = NULL;

	PVOID pvBuffer[6];
	pvBuffer[0]	= IntToPtr(lTID);			// INT32 lTID
	pvBuffer[1]	= (PVOID)pcsCharacter;		// AgpdCharacter *pcsCharacter
	pvBuffer[2]	= IntToPtr(lCount);			// INT32 lItemCount
	pvBuffer[3]	= IntToPtr(bGenerateDBID);	// BOOL bGenerateDBID
	pvBuffer[4]	= IntToPtr(lSkillTID);		// INT32 lSkillTID
	pvBuffer[5] = &pcsItem;					// AgpdItem** pcsItem, return value

	EnumCallback(AGSMITEM_CB_CREATE_NEW_ITEM, pvBuffer, NULL);

	return pcsItem;
}

BOOL AgsmItem::ProcessIdleRemoveFieldItem(INT32 lIID, PVOID pClass, UINT32 ulClockCount, PVOID pvData)
{
	PROFILE("AgsmItem::ProcessIdleRemoveFieldItem");

	if (!pClass || lIID == AP_INVALID_IID)
		return FALSE;

	AgsmItem		*pThis		= (AgsmItem *)	pClass;

	AgpdItem		*pcsItem	= pThis->m_pagpmItem->GetItemLock(lIID);
	if (!pcsItem)
		return FALSE;

	AgpdItemStatus	eStatus		= (AgpdItemStatus) pcsItem->m_eStatus;

	pcsItem->m_Mutex.Release();

	// 이 아템이 아직도 필드에 있는지 본다. 필드에 있다면 걍 없애버린다.
	if (eStatus == AGPDITEM_STATUS_FIELD)
		pThis->m_pagpmItem->RemoveItem(lIID, FALSE, TRUE);

	return TRUE;
}

/*
BOOL AgsmItem::BackupBasicItemData(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	AgsdServer		*pcsThisServer	= m_pagsmServerManager->GetThisServer();
	if (!pcsThisServer || !pcsThisServer->m_pcsTemplate)
		return FALSE;

	if ((eAGSMSERVER_TYPE) ((AgsdServerTemplate *) pcsThisServer->m_pcsTemplate)->m_cType == AGSMSERVER_TYPE_GAME_SERVER)
	{
		AgsdServer		*pcsRelayServer	= m_pagsmServerManager->GetRelayServer();
		if (pcsRelayServer && pcsRelayServer->m_dpnidServer)
		{
			return SendPacketBasicDBData(pcsItem, pcsRelayServer->m_dpnidServer);
		}
	}

	return TRUE;
}
*/

BOOL AgsmItem::SetMaxFieldItem(INT32 lCount)
{
	return m_csAdminFieldItem.SetCount(lCount);
}

BOOL AgsmItem::SetMaxDBIDItem(INT32 lCount)
{
	return m_csAdminDBIDItem.SetCount(lCount);
}

BOOL AgsmItem::AddDBIDAdmin(UINT64 ullDBID)
{
	// 2005.05.10. steeple
	// 버그 때문에 일단 리턴 TRUE
	return TRUE;

	if(ullDBID == 0L)
		return TRUE;

	CHAR szDBID[_I64_STR_LENGTH+1] = { 0, };
	_i64toa(ullDBID, szDBID, 10);
	return PtrToInt(m_csAdminDBIDItem.AddObject((PVOID)&ullDBID, szDBID));
}

BOOL AgsmItem::RemoveDBIDAdmin(UINT64 ullDBID)
{
	// 2005.05.10. steeple
	// 버그 때문에 일단 리턴 TRUE
	return TRUE;

	if(ullDBID == 0L)
		return FALSE;

	CHAR szDBID[_I64_STR_LENGTH+1] = { 0, };
	_i64toa(ullDBID, szDBID, 10);
	return (BOOL)m_csAdminDBIDItem.RemoveObject(szDBID);
}

//BOOL AgsmItem::MakeStringItemConvertHistory(AgpdItem *pcsItem, CHAR *szStringBuffer, INT32 lStringBufferSize)
//{
//	if (!pcsItem || !szStringBuffer || lStringBufferSize < 1)
//		return FALSE;
//
//	if (pcsItem->m_stConvertHistory.lConvertLevel < 1)
//		return TRUE;
//
//	ZeroMemory(szStringBuffer, sizeof(CHAR) * lStringBufferSize);
//
//	sprintf(szStringBuffer, "%.2f;", pcsItem->m_stConvertHistory.fConvertConstant);
//
//	INT32	lAttribute	= 0;
//	INT32	lValue		= 0;
//
//	for (int i = 0; i < pcsItem->m_stConvertHistory.lConvertLevel; ++i)
//	{
//		if (pcsItem->m_stConvertHistory.bUseSpiritStone[i])
//		{
//			lAttribute	= pcsItem->m_stConvertHistory.lTID[i];
//			lValue		= 0;
//		}
//		else
//		{
//			lAttribute	= (INT32) pcsItem->m_stConvertHistory.eRuneAttribute[i];
//			lValue		= pcsItem->m_stConvertHistory.lRuneAttributeValue[i];
//		}
//
//		sprintf(szStringBuffer + strlen(szStringBuffer), "%d,%d,%d;",
//								pcsItem->m_stConvertHistory.bUseSpiritStone[i],
//								lAttribute,
//								lValue);
//	}
//
//	return TRUE;
//}
//
//BOOL AgsmItem::ParseStringItemConvertHistory(AgpdItem *pcsItem, CHAR *szStringBuffer)
//{
//	if (!pcsItem || !szStringBuffer || !szStringBuffer[0])
//		return FALSE;
//
//	INT32	lStringBufferLength	= strlen(szStringBuffer);
//	INT32	lPrevStringPoint	= 0;
//
//	INT32	lIsUseSprintStone;
//	INT32	lAttribute;
//	INT32	lValue;
//
//	sscanf(szStringBuffer, "%f", &pcsItem->m_stConvertHistory.fConvertConstant);
//	for (int i = 0; i < lStringBufferLength; ++i)
//		if (szStringBuffer[i] == ';')
//			break;
//
//	if (i == lStringBufferLength)
//		return FALSE;
//
//	if (pcsItem->m_stConvertHistory.fConvertConstant < 2.6 ||
//		pcsItem->m_stConvertHistory.fConvertConstant > 3.0)
//		pcsItem->m_stConvertHistory.fConvertConstant	= 2.6 + m_csRandomNumber.randInt(40) / 100.0;
//
//	++i;
//
//	lPrevStringPoint	= i;
//
//	for ( ; i < lStringBufferLength; ++i)
//	{
//		if (szStringBuffer[i] == ';')
//		{
//			lIsUseSprintStone	= 0;
//			lAttribute			= 0;
//			lValue				= 0;
//
//			sscanf(szStringBuffer + lPrevStringPoint, "%d,%d,%d",
//								&lIsUseSprintStone,
//								&lAttribute,
//								&lValue);
//
//			pcsItem->m_stConvertHistory.bUseSpiritStone[pcsItem->m_stConvertHistory.lConvertLevel]			= (BOOL) lIsUseSprintStone;
//
//			if ((BOOL) lIsUseSprintStone)
//			{
//				pcsItem->m_stConvertHistory.lTID[pcsItem->m_stConvertHistory.lConvertLevel]					= lAttribute;
//
//				m_pagpmItem->RequireLevelUp(pcsItem, AGPMITEM_REQUIRE_LEVEL_UP_STEP);
//			}
//			else
//			{
//				pcsItem->m_stConvertHistory.eRuneAttribute[pcsItem->m_stConvertHistory.lConvertLevel]		= (AgpmItemRuneAttribute) lAttribute;
//				pcsItem->m_stConvertHistory.lRuneAttributeValue[pcsItem->m_stConvertHistory.lConvertLevel]	= lValue;
//
//				m_pagpmItem->SetRuneAttributeValue(&pcsItem->m_stConvertHistory.csFactorHistory[pcsItem->m_stConvertHistory.lConvertLevel], (AgpmItemRuneAttribute) lAttribute, lValue);
//			}
//
//			++pcsItem->m_stConvertHistory.lConvertLevel;
//
//			lPrevStringPoint = i + 1;
//		}
//	}
//
//	return TRUE;
//}

AgpdItem* AgsmItem::GetItemByDBID(AgpdCharacter *pcsCharacter, UINT64 ullDBID)
{
	if (!pcsCharacter)
		return NULL;

	AgpdItemADChar	*pcsItemADChar	= m_pagpmItem->GetADCharacter(pcsCharacter);

	for (int i = 0; i < pcsItemADChar->m_csInventoryGrid.m_nLayer; ++i)
	{
		for (int j = 0; j < pcsItemADChar->m_csInventoryGrid.m_nColumn; ++j)
		{
			for (int k = 0; k < pcsItemADChar->m_csInventoryGrid.m_nRow; ++k)
			{
				AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csInventoryGrid, i, k, j);
				if (pcsGridItem)
				{
					AgpdItem	*pcsItem	= m_pagpmItem->GetItem(pcsGridItem);
					if (pcsItem)
					{
						AgsdItem	*pcsAgsdItem	= GetADItem(pcsItem);

						if (pcsAgsdItem->m_ullDBIID == ullDBID)
							return pcsItem;
					}
				}
			}
		}
	}

	for (int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nLayer; ++i)
	{
		for (int j = 0; j < pcsItemADChar->m_csCashInventoryGrid.m_nColumn; ++j)
		{
			for (int k = 0; k < pcsItemADChar->m_csCashInventoryGrid.m_nRow; ++k)
			{
				AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, i, k, j);
				if (pcsGridItem)
				{
					AgpdItem	*pcsItem	= m_pagpmItem->GetItem(pcsGridItem);
					if (pcsItem)
					{
						AgsdItem	*pcsAgsdItem	= GetADItem(pcsItem);

						if (pcsAgsdItem->m_ullDBIID == ullDBID)
							return pcsItem;
					}
				}
			}
		}
	}

	for (int i = 0; i < pcsItemADChar->m_csSalesBoxGrid.m_nLayer; ++i)
	{
		for (int j = 0; j < pcsItemADChar->m_csSalesBoxGrid.m_nColumn; ++j)
		{
			for (int k = 0; k < pcsItemADChar->m_csSalesBoxGrid.m_nRow; ++k)
			{
				AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csSalesBoxGrid, i, k, j);
				if (pcsGridItem)
				{
					AgpdItem	*pcsItem	= m_pagpmItem->GetItem(pcsGridItem);
					if (pcsItem)
					{
						AgsdItem	*pcsAgsdItem	= GetADItem(pcsItem);

						if (pcsAgsdItem->m_ullDBIID == ullDBID)
							return pcsItem;
					}
				}
			}
		}
	}

	for (int i = 0; i < pcsItemADChar->m_csEquipGrid.m_nLayer; ++i)
	{
		for (int j = 0; j < pcsItemADChar->m_csEquipGrid.m_nColumn; ++j)
		{
			for (int k = 0; k < pcsItemADChar->m_csEquipGrid.m_nRow; ++k)
			{
				AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csEquipGrid, i, k, j);
				if (pcsGridItem)
				{
					AgpdItem	*pcsItem	= m_pagpmItem->GetItem(pcsGridItem);
					if (pcsItem)
					{
						AgsdItem	*pcsAgsdItem	= GetADItem(pcsItem);

						if (pcsAgsdItem->m_ullDBIID == ullDBID)
							return pcsItem;
					}
				}
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// Log 관련 - 2004.045.04. steeple
BOOL AgsmItem::WriteItemLog(eAGPDLOGTYPE_ITEM eType, INT32 lCID, AgpdItem* pAgpdItem, INT32 lCount, INT32 lGheld, CHAR *pszTargetChar)
{
	if(!m_pagpmLog)
		return TRUE;

	if(!lCID || !pAgpdItem)
		return FALSE;

	AgpdItemTemplate* pAgpdItemTemplate = (AgpdItemTemplate*)pAgpdItem->m_pcsItemTemplate;
	if(!pAgpdItemTemplate)
		return FALSE;

	AgsdItem* pAgsdItem = GetADItem(pAgpdItem);
	if(!pAgsdItem)
		return FALSE;
	
	CHAR szConvert[AGPDLOG_MAX_ITEM_CONVERT_STRING+1];
	ZeroMemory(szConvert, sizeof(szConvert));
	AgpdItemConvertADItem *pAgpdItemConvertADItem = m_pagpmItemConvert->GetADItem(pAgpdItem);
	m_pagpmItemConvert->EncodeConvertHistory(pAgpdItemConvertADItem, szConvert, AGPDLOG_MAX_ITEM_CONVERT_STRING);

	CHAR szOption[AGPDLOG_MAX_ITEM_FULL_OPTION + 1];
	ZeroMemory(szOption, sizeof(szOption));

	CHAR szNormalOption[AGPDLOG_MAX_ITEM_OPTION+1];
	ZeroMemory(szNormalOption, sizeof(szNormalOption));
	EncodingOption(pAgpdItem, szNormalOption, AGPDLOG_MAX_ITEM_OPTION);

	//##########################
	CHAR szSkillPlus[AGPDLOG_MAX_ITEM_OPTION+1];
	ZeroMemory(szSkillPlus, sizeof(szSkillPlus));
	EncodingSkillPlus(pAgpdItem, szSkillPlus, AGPDLOG_MAX_ITEM_OPTION);	

	sprintf(szOption, "%s::%s", szNormalOption, szSkillPlus);

	AgpdCharacter* pAgpdCharacter = m_pagpmCharacter->GetCharacter(lCID);
	if(!pAgpdCharacter)
		return FALSE;

	if (FALSE == m_pagpmCharacter->IsPC(pAgpdCharacter))
		return FALSE;

	AgsdCharacter* pAgsdCharacter = m_pagsmCharacter->GetADCharacter(pAgpdCharacter);
	if(!pAgsdCharacter)
		return FALSE;
	
	INT32 lLevel = m_pagpmCharacter->GetLevel(pAgpdCharacter);
	INT64 llExp = m_pagpmCharacter->GetExp(pAgpdCharacter);

	CHAR szPosition[33];
	ZeroMemory(szPosition, sizeof(szPosition));
	m_pagsmCharacter->EncodingPosition(&pAgpdCharacter->m_stPos, szPosition, 32);

	BOOL bIsCashItem = IS_CASH_ITEM(pAgpdItem->m_pcsItemTemplate->m_eCashItemType);

	return m_pagpmLog->WriteLog_Item(bIsCashItem, eType, 0, &pAgsdCharacter->m_strIPAddress[0],
								pAgsdCharacter->m_szAccountID, pAgsdCharacter->m_szServerName, pAgpdCharacter->m_szID,
								((AgpdCharacterTemplate*)pAgpdCharacter->m_pcsCharacterTemplate)->m_lID,
								lLevel, llExp, pAgpdCharacter->m_llMoney, pAgpdCharacter->m_llBankMoney,
								pAgsdItem->m_ullDBIID, pAgpdItemTemplate->m_lID,
								lCount, szConvert, szOption, lGheld, pszTargetChar,
								pAgpdItem->m_nInUseItem, pAgpdItem->m_lCashItemUseCount,
								pAgpdItem->m_lRemainTime, pAgpdItem->m_lExpireTime, pAgpdItem->m_llStaminaRemainTime,
								m_pagpmItem->GetItemDurabilityCurrent(pAgpdItem),
								m_pagpmItem->GetItemDurabilityMax(pAgpdItem),
								szPosition
								);
}

BOOL AgsmItem::WritePickupLog(INT32 lCID, AgpdItem* pAgpdItem, INT32 lCount /* = 1 */)
{
	return WriteItemLog(AGPDLOGTYPE_ITEM_PICK, lCID, pAgpdItem, lCount);
}

BOOL AgsmItem::WriteDropLog(INT32 lCID, AgpdItem* pAgpdItem, INT32 lCount /* = 1 */)
{
	return WriteItemLog(AGPDLOGTYPE_ITEM_DROP, lCID, pAgpdItem, lCount);
}

BOOL AgsmItem::WriteUseLog(INT32 lCID, AgpdItem* pAgpdItem)
{
	return WriteItemLog(AGPDLOGTYPE_ITEM_USE, lCID, pAgpdItem, 1);
}

BOOL AgsmItem::WriteUseStartLog(INT32 lCID, AgpdItem* pcsItem, INT32 lCount)
{
	return WriteItemLog(AGPDLOGTYPE_ITEM_USE_START, lCID, pcsItem, lCount);
}

BOOL AgsmItem::AddItemToDB(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	AgsdCharacter	*pcsAgsdCharacter	= m_pagsmCharacter->GetADCharacter(pcsItem->m_pcsCharacter);
	if (!pcsAgsdCharacter)
		return FALSE;

	AgsdItem			*pcsAgsdItem	= (AgsdItem *)		GetADItem(pcsItem);

	// DB에 저장해야 하는지 함 알아본다.
	if (pcsItem->m_pcsCharacter &&
		pcsAgsdItem->m_bIsNeedInsertDB)
	{
		if (m_pagsmCharacter->GetAccountID(pcsItem->m_pcsCharacter))
		{
			AgsdServer		*pcsRelayServer	= m_pAgsmServerManager->GetRelayServer();
			if (pcsRelayServer && pcsRelayServer->m_dpnidServer != 0)
			{
				SendRelayInsert(pcsItem);

				pcsAgsdItem->m_bIsNeedInsertDB = FALSE;
			}
		}
	}
	else if (AGSMSERVER_TYPE_GAME_SERVER == m_pAgsmServerManager->GetThisServerType() &&
			pcsItem->m_pcsCharacter->m_unCurrentStatus == AGPDCHAR_STATUS_IN_GAME_WORLD &&
			m_pagsmCharacter->GetAccountID(pcsItem->m_pcsCharacter))
	{
		SendRelayUpdate(pcsItem);
	}


	return TRUE;
}

BOOL AgsmItem::EncodingOption(AgpdItem *pcsItem, CHAR *szBuffer, INT32 lBufferLength)
{
	if (!pcsItem || !szBuffer || lBufferLength < 32)
		return FALSE;

	ZeroMemory(szBuffer, sizeof(CHAR) * lBufferLength);

	sprintf(szBuffer, "%d,%d,%d,%d,%d", pcsItem->m_aunOptionTID[0],
										pcsItem->m_aunOptionTID[1],
										pcsItem->m_aunOptionTID[2],
										pcsItem->m_aunOptionTID[3],
										pcsItem->m_aunOptionTID[4]);

	return TRUE;
}

BOOL AgsmItem::DecodingOption(AgpdItem *pcsItem, CHAR *szBuffer, INT32 lBufferLength)
{
	if (!pcsItem || !szBuffer || !szBuffer[0] || lBufferLength < 1)
		return FALSE;

	pcsItem->m_aunOptionTID.MemSetAll();
	pcsItem->m_apcsOptionTemplate.MemSetAll();

	INT32	ucOptionTID[AGPDITEM_OPTION_MAX_NUM];
	ZeroMemory(ucOptionTID, sizeof(ucOptionTID));

	int	retval	= sscanf(szBuffer, "%d,%d,%d,%d,%d", &ucOptionTID[0],
													 &ucOptionTID[1],
													 &ucOptionTID[2],
													 &ucOptionTID[3],
													 &ucOptionTID[4]);

	if (retval <= 0 || retval == EOF)
		return FALSE;

	for (int i = 0; i < AGPDITEM_OPTION_MAX_NUM; ++i)
	{
		if (ucOptionTID[i] == 0)
			break;

		m_pagpmItem->AddItemOption(pcsItem, ucOptionTID[i], FALSE);
	}

	return TRUE;
}

BOOL AgsmItem::EncodingSkillPlus(AgpdItem* pcsItem, CHAR* szBuffer, INT32 lBufferLength)
{
	if(!pcsItem || !szBuffer || lBufferLength < 1)
		return FALSE;

	ZeroMemory(szBuffer, sizeof(CHAR) * lBufferLength);

	sprintf(szBuffer, "%d,%d,%d", pcsItem->m_aunSkillPlusTID[0],
									pcsItem->m_aunSkillPlusTID[1],
									pcsItem->m_aunSkillPlusTID[2]);
	return TRUE;
}

BOOL AgsmItem::DecodingSkillPlus(AgpdItem* pcsItem, CHAR* szBuffer, INT32 lBufferLength)
{
	if(!pcsItem || !szBuffer || !szBuffer[0] || lBufferLength < 1)
		return FALSE;

	pcsItem->m_aunSkillPlusTID.MemSetAll();
	
	INT32 ulSkillPlusTID[AGPMITEM_MAX_SKILL_PLUS_EFFECT];
	ZeroMemory(ulSkillPlusTID, sizeof(ulSkillPlusTID));

	int retval = sscanf(szBuffer, "%d,%d,%d", &ulSkillPlusTID[0],
												&ulSkillPlusTID[1],
												&ulSkillPlusTID[2]);

	if(retval <= 0 || retval == EOF)
		return FALSE;

	for(int i = 0; i < AGPMITEM_MAX_SKILL_PLUS_EFFECT; ++i)
	{
		if(ulSkillPlusTID[i] == 0)
			break;

		m_pagpmItem->AddItemSkillPlus(pcsItem, ulSkillPlusTID[i]);
	}

	return TRUE;
}

//	2005.04.22	Start By SungHoon
BOOL AgsmItem::AddItemToPartyMember(ApBase *pcsFirstLooter, ApBase *pcsDropCharacter, AgpdItem *pcsItem)
{
	INT32 lGetItemCID = m_pagsmParty->GetCurrentGetItemPartyMember(pcsFirstLooter, pcsDropCharacter);
	if (lGetItemCID == AP_INVALID_CID) return FALSE;

	ApAutoLockCharacter Lock(m_pagpmCharacter,lGetItemCID );
	AgpdCharacter *pAgpdCharacter = Lock.GetCharacterLock();
	if (pAgpdCharacter == NULL) return FALSE;

	//if (pcsItem->m_pcsItemTemplate->m_bIsUseOnlyPCBang &&
	//	!m_pagpmBillInfo->IsPCBang(pAgpdCharacter))
	//	return FALSE;

	if (m_pagpmItem->AddItemToInventory(pAgpdCharacter, pcsItem) != TRUE) 
	{
		// 캐릭터 인벤에 넣는게 실패했다면, 펫인벤토리에 넣을수 있는지 확인한다
		if ( m_pagpmItem->IsEnableSubInventory(pAgpdCharacter) )
		{
			if(m_pagpmItem->AddItemToSubInventory(pAgpdCharacter, pcsItem) != TRUE)
				// 펫인벤에 넣는게 실패했다면 아이템을 필드에 드랍해야 하는데 필드드랍은 AgsmDropItem2::CBDropItem에서 해준다. 그래서 그냥 리턴
				return FALSE;
		}
		else
			return FALSE;
	}

	// 파티분배 로그
	WriteItemLog(AGPDLOGTYPE_ITEM_PARTY_DISTRIBUTE, pAgpdCharacter->m_lID, pcsItem, pcsItem->m_nCount ? pcsItem->m_nCount : 1);

	if (m_pagpmItem->IsQuestItem(pcsItem))
	{
		if (E_AGPMITEM_BIND_ON_ACQUIRE == m_pagpmItem->GetBoundType(pcsItem))
		{
			m_pagpmItem->SetBoundOnOwner(pcsItem, pAgpdCharacter);
		}
	}

	SendPacketItem(pcsItem, m_pagsmCharacter->GetCharDPNID(pAgpdCharacter));

//	모든 파티멤버에게 해당 유저가 아이템을 획득했음을 알린다.
	SendItemToPartyMember(pAgpdCharacter, pcsItem);

	return TRUE;
}
//	2005.04.21	Finish By SungHoon

/*
	2005.11.30. By SungHoon
	실제로 아이템 사용중지가 가능한지 확인후 시간을 체크해야한다.
*/
BOOL AgsmItem::UnuseItem(AgpdItem *pcsItem, BOOL bEndEffect)
{
	if (!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	BOOL bUnuseResult = m_pagpmItem->StopCashItemTimer(pcsItem);

	if (bUnuseResult)
	{
		// Avatar Item 이면 처리해준다.
		UnuseAvatarItem(pcsItem);

		// Charging Bag Item 이면 처리해준다.
		UnuseChargingInventory(pcsItem);

		SendPacketUnuseItem(pcsItem);

		if (bEndEffect)
		{
			m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter,
				AGPMSYSTEMMESSAGE_CODE_CASH_ITEM_END_USING_ITEM, -1, -1, pcsItem->m_pcsItemTemplate->m_szName);
			WriteItemLog(AGPDLOGTYPE_ITEM_USE_END, pcsItem->m_pcsCharacter->m_lID, pcsItem, pcsItem->m_nCount);
		}
		else
		{
			m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter,
				AGPMSYSTEMMESSAGE_CODE_CASH_ITEM_STOP_USING_ITEM, -1, -1, pcsItem->m_pcsItemTemplate->m_szName);
			WriteItemLog(AGPDLOGTYPE_ITEM_USE_STOP, pcsItem->m_pcsCharacter->m_lID, pcsItem, pcsItem->m_nCount);
		}
	}
	else
	{
		SendPacketUnuseItemFailed(pcsItem);
	}

	return bUnuseResult;
}

// 2007.08.10. Steeple
BOOL AgsmItem::UnuseAvatarItem(AgpdItem* pcsItem)
{
	if(!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	if(m_pagpmItem->IsAvatarItem(pcsItem->m_pcsItemTemplate) == FALSE)
		return FALSE;

	if(m_pagpmItem->IsBaseAvatarItem(pcsItem->m_pcsItemTemplate))
	{
		// Base Avatar 이면 Equip Part 를 돌면서 다 UnEquip 및 Remove 해준다.
		AgpdAvatarSetItem* pAvatarSet = m_pagpmItem->GetAvatarSetTemplateByBase(pcsItem->m_pcsItemTemplate->m_lID);
		if(pAvatarSet)
		{
			for(INT32 lPart = (INT32)AGPMITEM_PART_V_BODY; lPart <= (INT32)AGPMITEM_PART_V_LANCER; ++lPart)
			{
				AgpdGridItem* pcsGridItem = m_pagpmItem->GetEquipItem(pcsItem->m_pcsCharacter, lPart);
				if(!pcsGridItem)
					continue;

				AgpdItem* pcsEquipItem = m_pagpmItem->GetItem(pcsGridItem);
				if(!pcsEquipItem || !pcsEquipItem->m_pcsItemTemplate)
					continue;

				// EquipItem 의 TID 가 Set 에 포함되어 있는 지 확인한다.
				vector<INT32>::iterator iter = std::find(pAvatarSet->vecItemTIDs.begin(),
														pAvatarSet->vecItemTIDs.end(),
														pcsEquipItem->m_pcsItemTemplate->m_lID);
				if(iter == pAvatarSet->vecItemTIDs.end())
					continue;

				// 이제 UnEquip 해준다.
				m_pagpmItem->UnEquipItem(pcsItem->m_pcsCharacter, pcsEquipItem);

				// Remove 도 한다.
				m_pagpmItem->RemoveItem(pcsEquipItem, FALSE);
			}
		}
	}
	else
	{
		// 그냥 UnEquip 만 하면 된다.
		m_pagpmItem->UnEquipItem(pcsItem->m_pcsCharacter, pcsItem);
	}

	return TRUE;
}

BOOL AgsmItem::UnuseChargingInventory(AgpdItem* pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter || !pcsItem->m_pcsItemTemplate)
	{
		return FALSE;
	}

	AgpdItemTemplateUsable	*pcsTemplateUsable = (AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate;
	if (pcsTemplateUsable->m_nUsableItemType != AGPMITEM_USABLE_TYPE_CHARGING_BAG)
		return FALSE;

	AgpdItemADChar *pAttachedData = m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);
	if( !pAttachedData )
		return FALSE;

	// 사용하고 있지 않음
	if (!pAttachedData->m_bUseChagingInventory)
		return FALSE;

	pAttachedData->m_bUseChagingInventory = FALSE;

	return TRUE;
}


// 2006.01.08. steeple
// 캐쉬 아이템을 일시 정지한다.
BOOL AgsmItem::PauseItem(AgpdItem* pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	BOOL bPauseItem = m_pagpmItem->PauseCashItemTimer(pcsItem);

	if(bPauseItem)
	{
		SendPacketPauseItem(pcsItem);

		m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter,
			AGPMSYSTEMMESSAGE_CODE_CASH_ITEM_PAUSE, -1, -1, pcsItem->m_pcsItemTemplate->m_szName);
	}

	return bPauseItem;
}

INT32 AgsmItem::UpdateAllInUseLimitTimeItem(AgpdCharacter *pcsCharacter, AgpdGrid m_csGrid, INT32 lElapsedTickCount, UINT32 lCurrentTimeStamp)
{
	INT32 lCount = 0;

	for(int i = 0; i < m_csGrid.m_nLayer; i++)
	{
		for(int j = 0; j < m_csGrid.m_nRow; j++)
		{
			for(int k = 0; k < m_csGrid.m_nColumn; k++)
			{
				AgpdGridItem *pcsGridItem = m_pagpmGrid->GetItem(&m_csGrid, i, j, k);

				if (pcsGridItem)
				{
					AgpdItem *pcsItem = m_pagpmItem->GetItem(pcsGridItem);

					if (!pcsItem)
					{
						continue;
					}

					AgsdItem* pcsAgsdItem = GetADItem(pcsItem);
					if(!pcsAgsdItem)
						continue;

					if (pcsItem->m_lExpireTime > 0)
					{
						if(lCurrentTimeStamp > pcsItem->m_lExpireTime)
						{
							m_pagsmSystemMessage->SendSystemMessage(pcsItem->m_pcsCharacter, AGPMSYSTEMMESSAGE_CODE_CASH_ITEM_END_USING_ITEM, -1, -1, pcsItem->m_pcsItemTemplate->m_szName);

							memset(pcsItem->m_szDeleteReason, 0, sizeof(pcsItem->m_szDeleteReason));
							strncpy(pcsItem->m_szDeleteReason, "LimitTimeExpired", AGPMITEM_MAX_DELETE_REASON);

							m_pagpmItem->RemoveItem(pcsItem->m_lID, TRUE);
						}
						else
						{
							++lCount;
						}
					}
				}
			}
		}
	}

	return lCount;
}

BOOL AgsmItem::UpdateAllInUseLimitTimeItemTime(AgpdCharacter *pcsCharacter, INT32 lElapsedTickCount, UINT32 lCurrentTimeStamp)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar *pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return FALSE;

	AgsdItemADChar* pcsAgsdItemADChar = GetADCharacter(pcsCharacter);
	if(!pcsAgsdItemADChar)
		return FALSE;

	INT32 lCount = 0;

	lCount += UpdateAllInUseLimitTimeItem(pcsCharacter, pcsItemADChar->m_csEquipGrid,			lElapsedTickCount, lCurrentTimeStamp);
	lCount += UpdateAllInUseLimitTimeItem(pcsCharacter, pcsItemADChar->m_csInventoryGrid,		lElapsedTickCount, lCurrentTimeStamp);
	lCount += UpdateAllInUseLimitTimeItem(pcsCharacter, pcsItemADChar->m_csBankGrid,			lElapsedTickCount, lCurrentTimeStamp);
	lCount += UpdateAllInUseLimitTimeItem(pcsCharacter, pcsItemADChar->m_csSubInventoryGrid,	lElapsedTickCount, lCurrentTimeStamp);
	lCount += UpdateAllInUseLimitTimeItem(pcsCharacter, pcsItemADChar->m_csChargingInventoryGrid, lElapsedTickCount, lCurrentTimeStamp);

	pcsAgsdItemADChar->m_bUseTimeLimitItem = (lCount > 0) ? TRUE : FALSE;

	return TRUE;
}

BOOL AgsmItem::UpdateAllInUseCashItemTime(AgpdCharacter *pcsCharacter, INT32 lElapsedTickCount, UINT32 lCurrentTimeStamp)
{
	if (!pcsCharacter)
	{
		return FALSE;
	}

	AgpdItemADChar *pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
	{
		return NULL;
	}

	//STOPWATCH2(GetModuleName(), _T("UpdateAllInUseCashItemTime"));

	for (int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nRow; i++)
	{
		AgpdGridItem *pcsGridItem = m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, 0, i, 0 );
		if (pcsGridItem)
		{
			AgpdItem *pcsItem = m_pagpmItem->GetItem(pcsGridItem);
			if (!pcsItem)
			{
				continue;
			}

			AgsdItem* pcsAgsdItem = GetADItem(pcsItem);
			if(!pcsAgsdItem)
				continue;

			AgpdItemTemplate *pcsItemTemplate = pcsItem->m_pcsItemTemplate;

			// 2006.01.08. steeple
			// 사용중인 아이템이 아니면 Skip.
			if (pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_UNUSE &&
				pcsItemTemplate->m_eCashItemType != AGPMITEM_CASH_ITEM_TYPE_STAMINA)
			{
				continue;
			}

			if (pcsItemTemplate->m_eCashItemType == AGPMITEM_CASH_ITEM_TYPE_PLAY_TIME)
			{
//#ifdef _DEBUG
//				CHAR szString[64];
//				sprintf(szString, "e:%d r:%d", lElapsedTickCount, pcsItem->m_lRemainTime);
//				m_pagsmSystemMessage->SendSystemDebugString(pcsCharacter, szString);
//#endif

				// 2006.01.08. steeple
				// 플레이 타임 계산에서는 일시 정지 된 건 하지 않는다.
				if(pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_PAUSE)
					continue;

				INT64 lPrevRemainTime = pcsItem->m_lRemainTime;

				pcsItem->m_lRemainTime -= lElapsedTickCount;
				if (pcsItem->m_lRemainTime <= 0) pcsItem->m_lRemainTime = 0;

				if (lPrevRemainTime > 60000)
				{
					// 분단위 변화 확인
					if (lPrevRemainTime/60000 != pcsItem->m_lRemainTime/60000)
					{
						// 패킷 전송
						SendPacketUpdateItemUseTime(pcsItem);

						SendRelayUpdate(pcsItem);
					}
				}
				else
				{
					// 초단위 변화 확인
					if (lPrevRemainTime/1000 != pcsItem->m_lRemainTime/1000)
					{
						// 패킷 전송
						SendPacketUpdateItemUseTime(pcsItem);
					}
				}

				if (pcsItem->m_lRemainTime <= 0)
				{
					// write log, 시작할 때 남겨야 할까? 마칠 때 남겨야 할까??? 20051202, kelovon
					//WriteUseLog(pcsCharacter->m_lID, pcsItem);	// 시작할 때 남겨야 할까? 마칠 때 남겨야 할까??? 20051202, kelovon

					// 사용 해제
					UnuseItem(pcsItem, TRUE);

					// 시간이 다 되서 끝난 녀석이라면 Cooldown 에서 Pause 를 FALSE 로 해준다. 2008.04.02. steeple
					m_pagpmItem->SetCooldownPause(pcsCharacter, pcsItem->m_pcsItemTemplate->m_lID, FALSE);

					// write log
					WriteUseLog(pcsCharacter->m_lID, pcsItem);

					// 아이템 삭제
					if (0 < m_pagpmItem->SubItemStackCount(pcsItem, 1))
					{
						// 개수가 남아있는 경우이므로 사용시간 reset
						pcsItem->m_lRemainTime = pcsItem->m_pcsItemTemplate->m_lRemainTime;

						// 정보 업데이트
						SendPacketUpdateItemUseTime(pcsItem);

						// update db
						SendRelayUpdate(pcsItem);
					}
				}
			}
			else if (pcsItemTemplate->m_eCashItemType == AGPMITEM_CASH_ITEM_TYPE_REAL_TIME)
			{
//				ApString<32> szTmpDate;
//				AuTimeStamp::ConvertTimeStampToOracleTime(pcsItem->m_lExpireTime, szTmpDate.GetBuffer(), szTmpDate.GetBufferLength());

				if (pcsItem->m_lExpireTime != 0 && lCurrentTimeStamp > pcsItem->m_lExpireTime)
				{
					// 사용 해제
					UnuseItem(pcsItem, TRUE);

					// 시간이 다 되서 끝난 녀석이라면 Cooldown 에서 Pause 를 FALSE 로 해준다. 2008.04.02. steeple
					m_pagpmItem->SetCooldownPause(pcsCharacter, pcsItem->m_pcsItemTemplate->m_lID, FALSE);

					// write log
					WriteUseLog(pcsCharacter->m_lID, pcsItem);

					// 아이템 삭제
					if (0 < m_pagpmItem->SubItemStackCount(pcsItem, 1))
					{
						// 개수가 남아있는 경우이므로 만료시간 reset
						pcsItem->m_lExpireTime = 0;

						// 정보 업데이트
						SendPacketUpdateItemUseTime(pcsItem);

						// update db
						SendRelayUpdate(pcsItem);
					}
				}
			}

			// Check a stamina 2008.06.09. steeple
			// Check only them have m_llStaminaRemainTime and have a lower m_llStaminaRemainTime value than one of template.
			if(pcsItemTemplate->m_eCashItemType == AGPMITEM_CASH_ITEM_TYPE_STAMINA &&
				pcsItemTemplate->m_llStaminaRemainTime != 0 &&
				pcsItem->m_llStaminaRemainTime < pcsItemTemplate->m_llStaminaRemainTime &&
				pcsItem->m_lCashItemUseCount > 0)
			{
				// If a pet eats a food automatically, nothing to do.
				if(CheckStatminaPet(pcsItem))
					continue;

				INT64 lPrevStaminaRemainTime = pcsItem->m_llStaminaRemainTime;

				pcsItem->m_llStaminaRemainTime -= lElapsedTickCount;
				if (pcsItem->m_llStaminaRemainTime <= 0) pcsItem->m_llStaminaRemainTime = 0;

				if (lPrevStaminaRemainTime > 60000)
				{
					// 분단위 변화 확인
					if (lPrevStaminaRemainTime/60000 != pcsItem->m_llStaminaRemainTime/60000)
					{
						// 패킷 전송
						SendPacketUpdateItemStaminaRemainTime(pcsItem);
						SendRelayUpdate(pcsItem);
					}
				}
				else
				{
					// 초단위 변화 확인
					if (lPrevStaminaRemainTime/1000 != pcsItem->m_llStaminaRemainTime/1000)
					{
						// 패킷 전송
						SendPacketUpdateItemStaminaRemainTime(pcsItem);
					}
				}

				if (pcsItem->m_llStaminaRemainTime <= 0)
				{
					// write log, 시작할 때 남겨야 할까? 마칠 때 남겨야 할까??? 20051202, kelovon
					//WriteUseLog(pcsCharacter->m_lID, pcsItem);	// 시작할 때 남겨야 할까? 마칠 때 남겨야 할까??? 20051202, kelovon

					// 사용 해제
					UnuseItem(pcsItem, TRUE);

					// 시간이 다 되서 끝난 녀석이라면 Cooldown 에서 Pause 를 FALSE 로 해준다. 2008.04.02. steeple
					m_pagpmItem->SetCooldownPause(pcsCharacter, pcsItem->m_pcsItemTemplate->m_lID, FALSE);

					// write log
					WriteUseLog(pcsCharacter->m_lID, pcsItem);

					// 아이템 삭제
					if (0 < m_pagpmItem->SubItemStackCount(pcsItem, 1))
					{
						// 개수가 남아있는 경우이므로 사용시간 reset
						m_pagpmItem->SetInitialStamina(pcsItem);

						// 정보 업데이트
						SendPacketUpdateItemStaminaRemainTime(pcsItem);

						// update db
						SendRelayUpdate(pcsItem);
					}
				}


			}	// Check a stamina
		}	// if(GridItem)
	}

	return FALSE;
}

// 2005.12.26. steeple
// 사용 가능한 캐쉬 아이템을 Use 시킨다.
// 보통 스킬스크롤 용 캐쉬 아이템이 사용되게 된다.
// bRideCheck == TRUE 일 때는, ItemTemplate->m_bEnableOnRide 를 확인한다.
BOOL AgsmItem::UseAllEnableCashItem(AgpdCharacter* pcsCharacter, BOOL bRideCheck, eAgsdItemPauseReason eReason, BOOL bIncludePause)
{
	//STOPWATCH2(GetModuleName(), _T("UseAllEnableCashItem"));

	if(!pcsCharacter)
		return FALSE;

	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	UINT32 ulClockCount = GetClockCount();

	for(int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nRow; i++)
	{
		AgpdGridItem* pcsGridItem = m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, 0, i, 0 );
		if(!pcsGridItem)
			continue;

		AgpdItem* pcsItem = m_pagpmItem->GetItem(pcsGridItem);
		if(!pcsItem)
			continue;

		AgsdItem* pcsAgsdItem = GetADItem(pcsItem);
		if(!pcsAgsdItem)
			continue;

		AgpdItemTemplate* pcsItemTemplate = pcsItem->m_pcsItemTemplate;

		if(!m_pagpmItem->IsAvatarItem(pcsItemTemplate) && pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE)
			continue;

		INT8 cPrevUsingStatus = pcsItem->m_nInUseItem;
		if(bIncludePause)
		{
			// 일시 정지인 놈도 사용해주어야 한다.
			if(cPrevUsingStatus == AGPDITEM_CASH_ITEM_UNUSE)
				continue;
		}
		else
		{
			// 사용중이 아니라면 패스
			if(cPrevUsingStatus != AGPDITEM_CASH_ITEM_INUSE)
				continue;
		}

		if(IS_CASH_ITEM(pcsItemTemplate->m_eCashItemType) == FALSE ||
			m_pagpmItem->IsCharacterUsingCashItemOfSameClassifyIDorTID(pcsCharacter, pcsItem))
			continue;

		// 탈 것 체크에서는 사용가능한 체크를 한번 더 해준다.
		if(bRideCheck && pcsItemTemplate->m_bEnableOnRide == FALSE)
			continue;

		// 2008.02.20. steeple
		// 사용중인데 PC 방에서만 해야 하는 놈들인데 유저가 PC 방이 아닌 경우에는 정지 시켜버린다.
		if(pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_INUSE &&
			(pcsItem->m_pcsItemTemplate->m_lIsUseOnlyPCBang & AGPMITEM_PCBANG_TYPE_USE_ONLY) &&
			m_pagpmBillInfo->IsPCBang(pcsCharacter) == FALSE)
		{
			m_pagpmItem->UnuseItem(pcsCharacter, pcsItem);

			// 쿨타임도 정지 시켜준다.
			m_pagpmItem->SetCooldownPause(pcsCharacter, pcsItem->m_pcsItemTemplate->m_lID, TRUE);

			continue;
		}

		// 사용할 수 있는 지 체크. 지역별, PC 방 모두. 2008.01.28. steeple
		if(CheckUsable(pcsItem) == FALSE)
			continue;

		if(m_pagpmItem->IsAvatarItem(pcsItemTemplate))
			UseItemAvatar(pcsItem);
		else if(((AgpdItemTemplateUsable*)pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_SKILL_SCROLL)
			UseItemSkillScroll(pcsItem, pcsCharacter, TRUE);
		else if(((AgpdItemTemplateUsable*)pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_JANUS_TALISMAN)
			UseItemJanusTalisman(pcsItem, pcsCharacter, TRUE);
		else if(((AgpdItemTemplateUsable*)pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_CHARGING_BAG)
			UseChargingInventory(pcsItem);

		if(cPrevUsingStatus == AGPDITEM_CASH_ITEM_PAUSE)
		{
			// 정지되었던 놈이라면 사용중으로 바꾼다.
			m_pagpmItem->StartCashItemTimer(pcsItem);

			// 정보 업데이트
			SendPacketUpdateItemUseTime(pcsItem);

			pcsAgsdItem->m_ePauseReason = AGSDITEM_PAUSE_REASON_NONE;
			pcsAgsdItem->m_ulPausedClock = 0;
		}
	}

	return TRUE;
}

// 2005.12.27. steeple
// 사용중인 캐쉬아이템을 중지 시킨다.
// RideCheck 가 TRUE 일 때는, 탈 것 타고 가능한 놈들은 UnUse 시키지 않는다.
BOOL AgsmItem::UnUseAllCashItem(AgpdCharacter* pcsCharacter, BOOL bRideCheck, eAgsdItemPauseReason eReason)
{
	//STOPWATCH2(GetModuleName(), _T("UnUseAllCashItem"));

	if(!pcsCharacter)
		return FALSE;

	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	UINT32 ulClockCount = GetClockCount();

	for(int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nRow; i++)
	{
		AgpdGridItem* pcsGridItem = m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, 0, i, 0 );
		if(!pcsGridItem)
			continue;

		AgpdItem* pcsItem = m_pagpmItem->GetItem(pcsGridItem);
		if(!pcsItem)
			continue;

		AgsdItem* pcsAgsdItem = GetADItem(pcsItem);
		if(!pcsAgsdItem)
			continue;

		AgpdItemTemplate* pcsItemTemplate = pcsItem->m_pcsItemTemplate;

		if(!pcsItem->m_nInUseItem || pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE || 
			pcsItemTemplate->m_eCashItemType == AGPMITEM_CASH_ITEM_TYPE_STAMINA)
			continue;

		//JK_아바타 투명후 사라지는 현상
		if(m_pagpmItem->IsAvatarItem(pcsItemTemplate))
			UnuseAvatarItem(pcsItem);


		// 탈 것 체크에서는 사용가능한 체크를 한번 더 해준다.
		if(bRideCheck && pcsItemTemplate->m_bEnableOnRide)
			continue;

		if(eReason == AGSDITEM_PAUSE_REASON_NONE)
            UnuseItem(pcsItem);
		else
		{
			PauseItem(pcsItem);

			pcsAgsdItem->m_ePauseReason = eReason;
			pcsAgsdItem->m_ulPausedClock = ulClockCount;
		}
	}

	return TRUE;
}

// 2006.01.06. steeple
BOOL AgsmItem::UsePausedCashItem(AgpdCharacter* pcsCharacter, eAgsdItemPauseReason eReason)
{
	//STOPWATCH2(GetModuleName(), _T("UsePausedCashItem"));

	if(!pcsCharacter)
		return FALSE;

	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	for(int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nRow; i++)
	{
		AgpdGridItem* pcsGridItem = m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, 0, i, 0 );
		if(!pcsGridItem)
			continue;

		AgpdItem* pcsItem = m_pagpmItem->GetItem(pcsGridItem);
		if(!pcsItem)
			continue;

		AgsdItem* pcsAgsdItem = GetADItem(pcsItem);
		if(!pcsAgsdItem)
			continue;

		AgpdItemTemplate* pcsItemTemplate = pcsItem->m_pcsItemTemplate;

		if(pcsItem->m_nInUseItem != AGPDITEM_CASH_ITEM_PAUSE ||
			pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE ||
			((AgpdItemTemplateUsable*)pcsItemTemplate)->m_nUsableItemType != AGPMITEM_USABLE_TYPE_SKILL_SCROLL ||
			m_pagpmItem->IsCharacterUsingCashItemOfSameClassifyIDorTID(pcsCharacter, pcsItem))
			continue;

		if(eReason == pcsAgsdItem->m_ePauseReason)
		{
			UseItem(pcsItem, FALSE, pcsCharacter);

			// 정보 업데이트
			SendPacketUpdateItemUseTime(pcsItem);

			pcsAgsdItem->m_ePauseReason = AGSDITEM_PAUSE_REASON_NONE;
			pcsAgsdItem->m_ulPausedClock = 0;
		}
	}

	return TRUE;
}

BOOL AgsmItem::GiveItem(AgpdCharacter *pcsCharacter, INT32 lItemTID, INT32 lCount, eAGPDLOGTYPE_ITEM eReason)
{
	if(NULL == pcsCharacter)
		return FALSE;

	AgpdItemTemplate *pcsItemTempate = m_pagpmItem->GetItemTemplate(lItemTID);
	if(NULL == pcsItemTempate)
		return FALSE;

	if(m_pagsmItemManager)
		return FALSE;

	AgpdItem* pcsItem = NULL;
	if (pcsItemTempate->m_bStackable)
		pcsItem = m_pagsmItemManager->CreateItem(pcsItemTempate->m_lID, pcsCharacter, lCount);
	else
		pcsItem = m_pagsmItemManager->CreateItem(pcsItemTempate->m_lID, pcsCharacter);

	if(!pcsItem)
		return FALSE;

	// 귀속아이템이라면 귀속과 주인을 세팅해준다. 2005.11.02. steeple
	if(m_pagpmItem->GetBoundType(pcsItem->m_pcsItemTemplate) == E_AGPMITEM_BIND_ON_ACQUIRE)
	{
		m_pagpmItem->SetBoundType(pcsItem, E_AGPMITEM_BIND_ON_ACQUIRE);
		m_pagpmItem->SetBoundOnOwner(pcsItem, pcsCharacter);
	}

	// Skill Plus 해준다. 2007.02.05. steeple
	ProcessItemSkillPlus(pcsItem, pcsCharacter);

	WriteItemLog(eReason, pcsCharacter->m_lID,
		pcsItem,
		pcsItem->m_nCount ? pcsItem->m_nCount : 1
		);

	BOOL bSuccess = m_pagpmItem->AddItemToInventory(pcsCharacter, pcsItem);

	SendPacketPickupItemResult(
		AGPMITEM_PACKET_PICKUP_ITEM_RESULT_SUCCESS, 
		(pcsItem) ? pcsItem->m_lID : AP_INVALID_IID, 
		pcsItemTempate->m_lID, 
		(pcsItem->m_nCount) ? pcsItem->m_nCount : 1, 
		m_pagsmCharacter->GetCharDPNID(pcsCharacter));

	return TRUE;
}

UINT64 AgsmItem::WriteCashItemBuyList(AgpdCharacter *pcsCharacter, stCashItemBuyList *pstList)
{
	if (NULL == pcsCharacter || NULL == pstList)
		return 0;

	pstList->m_ullBuyID = 0;
	EnumCallback(AGSMITEM_CB_GET_NEW_CASHITEMBUYID, &pstList->m_ullBuyID, NULL);
	if (0 == pstList->m_ullBuyID)
		return 0;
	
	ZeroMemory(pstList->m_szCharName, sizeof(pstList->m_szCharName));
	ZeroMemory(pstList->m_szAccName, sizeof(pstList->m_szAccName));
	ZeroMemory(pstList->m_szIP, sizeof(pstList->m_szIP));

	_tcsncpy(pstList->m_szCharName, pcsCharacter->m_szID, AGPDCHARACTER_MAX_ID_LENGTH);
	pstList->m_lCharTID = pcsCharacter->m_pcsCharacterTemplate->m_lID;
	pstList->m_lCharLevel = m_pagpmCharacter->GetLevel(pcsCharacter);
	
	AgsdCharacter *pAgsdCharacter = m_pagsmCharacter->GetADCharacter(pcsCharacter);
	if (NULL != pAgsdCharacter)
		{
		_tcsncpy(pstList->m_szAccName, pAgsdCharacter->m_szAccountID, AGSMACCOUNT_MAX_ACCOUNT_NAME);
		_tcsncpy(pstList->m_szIP, &pAgsdCharacter->m_strIPAddress[0], 14);
		}
	
	EnumCallback(AGSMITEM_CB_INSERT_CASHITEMBUYLIST, pstList, NULL);
	
	return pstList->m_ullBuyID;
}

UINT64 AgsmItem::WriteCashItemBuyList(stCashItemBuyList *pstList, CHAR *pszAccName, CHAR *pszCharName)
{
	if (NULL == pstList || NULL == pszAccName || NULL == pszCharName)
		return 0;

	pstList->m_ullBuyID = 0;
	EnumCallback(AGSMITEM_CB_GET_NEW_CASHITEMBUYID, &pstList->m_ullBuyID, NULL);
	if (0 == pstList->m_ullBuyID)
		return 0;
	
	ZeroMemory(pstList->m_szCharName, sizeof(pstList->m_szCharName));
	ZeroMemory(pstList->m_szAccName, sizeof(pstList->m_szAccName));
	ZeroMemory(pstList->m_szIP, sizeof(pstList->m_szIP));

	_tcsncpy(pstList->m_szCharName, pszCharName, AGPDCHARACTER_MAX_ID_LENGTH);
	_tcsncpy(pstList->m_szAccName, pszAccName, AGSMACCOUNT_MAX_ACCOUNT_NAME);
	pstList->m_lCharTID = 0;
	pstList->m_lCharLevel = 0;
	
	EnumCallback(AGSMITEM_CB_INSERT_CASHITEMBUYLIST, pstList, NULL);
	
	return pstList->m_ullBuyID;
}

BOOL AgsmItem::UpdateCashItemBuyList_Complete(UINT64 ullItemSeq, UINT64 ullOrderNo, UINT64 ullBuyID)
{
	if (0 == ullBuyID)
		return FALSE;

	stCashItemBuyList stList;
	ZeroMemory(&stList, sizeof(stList));
	stList.m_ullBuyID = ullBuyID;
	stList.m_ullItemSeq = ullItemSeq;
	stList.m_cStatus = AGSDITEM_CASHITEMBUYLIST_STATUS_COMPLETE;	
	stList.m_ullOrderNo = ullOrderNo;

	EnumCallback(AGSMITEM_CB_UPDATE_CASHITEMBUYLIST, &stList, NULL);
	return TRUE;
}

BOOL AgsmItem::UpdateCashItemBuyList_Complete2(UINT64 ullItemSeq, CHAR *pszOrderID, UINT64 ullBuyID)
{
	if (0 == ullBuyID)
		return FALSE;

	stCashItemBuyList stList;
	ZeroMemory(&stList, sizeof(stList));
	stList.m_ullBuyID = ullBuyID;
	stList.m_ullItemSeq = ullItemSeq;
	stList.m_cStatus = AGSDITEM_CASHITEMBUYLIST_STATUS_COMPLETE;	
	strcpy(stList.m_szOrderID, pszOrderID);

	EnumCallback(AGSMITEM_CB_UPDATE_CASHITEMBUYLIST, &stList, NULL);
	return TRUE;
}

BOOL AgsmItem::UpdateCashItemBuyList_Other(INT8 cStatus, UINT64 ullItemSeq)
{
	if (0 == ullItemSeq)
		return FALSE;

	stCashItemBuyList stList;
	ZeroMemory(&stList, sizeof(stList));
	stList.m_ullItemSeq = ullItemSeq;
	stList.m_cStatus = cStatus;	

	EnumCallback(AGSMITEM_CB_UPDATE_CASHITEMBUYLIST2, &stList, NULL);
	return TRUE;
}

// 2005.12.14. steeple
// AgsmCharManager 의 EnterGameWorld 함수 안에서 호출한다.
BOOL AgsmItem::EnterGameWorld(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	// PC 만 한다.
	if(m_pagpmCharacter->IsPC(pcsCharacter) == FALSE || pcsCharacter->m_szID[0] == '\0')
		return TRUE;

	AgpdItemADChar *pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return FALSE;

	AgsdItemADChar* pcsAgsdItemADChar = GetADCharacter(pcsCharacter);
	if(!pcsAgsdItemADChar)
		return FALSE;

	// 캐쉬 아이템 정렬을 함 해준다. 2006.01.05. steeple
	m_pagpmItem->ArrangeCashItemGridPos(pcsCharacter);

	// 캐시 인벤토리에 사용 중인 아이템이 있으면, idle 설정함
	BOOL bRet = m_pagpmItem->IsCharacterUsingCashItem(pcsCharacter);
	if (bRet)
	{
		// 자동으로 사용할 애들이 있으면 사용해준다.
		UseAllEnableCashItem(pcsCharacter,
							pcsCharacter->m_bRidable,
							pcsCharacter->m_bRidable ? AGSDITEM_PAUSE_REASON_RIDE : AGSDITEM_PAUSE_REASON_NONE);

		m_pagsmCharacter->SetProcessTime(pcsCharacter, AGSDCHAR_IDLE_TYPE_ITEM, GetClockCount());
		m_pagsmCharacter->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_ITEM, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);
	}

	// 인벤토리 기간제 아이템.
	UINT32 lCurrentTimeStamp = AuTimeStamp::GetCurrentTimeStamp();
	UpdateAllInUseLimitTimeItemTime(pcsCharacter, 0, lCurrentTimeStamp);

	if (IsCharacterUsingTimeLimitItem(pcsCharacter))
	{
		m_pagsmCharacter->SetProcessTime(pcsCharacter, AGSDCHAR_IDLE_TYPE_ITEM, GetClockCount());
		m_pagsmCharacter->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_ITEM, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);
	}

	// 2008.02.15. steeple
	// Cooldown 진행중이면 보내준다.
	if(m_pagpmItem->IsProgressingCooldown(pcsCharacter))
	{
		SendPacketAllCooldown(pcsCharacter);
		m_pagsmCharacter->SetIdleInterval(pcsCharacter, AGSDCHAR_IDLE_TYPE_ITEM, AGSDCHAR_IDLE_INTERVAL_TWO_SECONDS);
	}

	// 2006.12.27. steeple
	// 장착하고 있는 아이템의 스킬 옵션을 계산해준다.
	UpdateItemOptionSkillData(pcsCharacter, TRUE);
	m_pagsmCharacter->ReCalcCharacterFactors(pcsCharacter, TRUE);

	// 아크로드 스킬은 항상 1레벨 고정이라 스킬데이터가 바뀌지 않으니 계산하지 않는다
	// 홀리 브리스(패시브)스킬 관련 체력이 줄어드는 문제 수정
	if( FALSE == m_pagpmCharacter->IsArchlord(pcsCharacter) )
	{
		m_pagpmSkill->UpdateModifiedSkillLevel((ApBase*)pcsCharacter);	
	}

	return TRUE;
}

// 2006.06.22. steeple
BOOL AgsmItem::CheckUsable(AgpdItem* pcsItem, BOOL bIsFirstUse, AgpdCharacter *pcsTargetChar)
{
	if(!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	AgpdItemTemplate* pcsTemplate = (AgpdItemTemplate*)pcsItem->m_pcsItemTemplate;
	if(!pcsTemplate)
		return FALSE;

	if (!m_pagpmItem->CheckUseValidArea(pcsItem))
		return FALSE;

	// PC 방에서만 쓸 수 있는 놈인지 체크. 2008.01.28. steeple
	if((pcsTemplate->m_lIsUseOnlyPCBang & AGPMITEM_PCBANG_TYPE_USE_ONLY) &&
		m_pagpmBillInfo->IsPCBang(pcsItem->m_pcsCharacter) == FALSE)
		return FALSE;

	// T PC방에서만 쓸 수 있는 놈인지 체크
	if(pcsTemplate->m_lIsUseOnlyPCBang & AGPMITEM_PCBANG_TYPE_USE_ONLY_TPACK)
	{
		INT32 ulPCRoomType = 0;

		m_pagpmCharacter->EnumCallback(AGPMCHAR_CB_ID_CHECK_PCROOM_TYPE, pcsItem->m_pcsCharacter, &ulPCRoomType);
		if(!(ulPCRoomType & AGPDPCROOMTYPE_HANGAME_TPACK))
			return FALSE;
	}

	// GPREMIUM에서만 쓸 수 있는 놈인지 체크
	if(pcsTemplate->m_lIsUseOnlyPCBang & AGPMITEM_PCBANG_TYPE_USE_ONLY_GPREMIUM)
	{
		INT32 ulPCRoomType = 0;

		m_pagpmCharacter->EnumCallback(AGPMCHAR_CB_ID_CHECK_PCROOM_TYPE, pcsItem->m_pcsCharacter, &ulPCRoomType);
		if(!(ulPCRoomType & AGPDPCROOMTYPE_WEBZEN_GPREMIUM))
			return FALSE;
	}

	if(pcsTemplate->m_lUsingType == AGPMITEM_USING_TYPE_ALL)
		return TRUE;

	BOOL bCheck = TRUE;

	if(pcsTemplate->m_lUsingType & AGPMITEM_USING_TYPE_SIEGE_WAR_ONLY)
	{
		// 사용자가 공성전 상태가 아니라면 쓸 수 없다.
		AgpdSiegeWar* pcsSiegeWar = m_pagpmSiegeWar->GetSiegeWarInfoOfCharacter(pcsItem->m_pcsCharacter);
		if(!pcsSiegeWar)
			bCheck = FALSE;
		else if(pcsSiegeWar->m_eCurrentStatus < AGPD_SIEGE_WAR_STATUS_START || pcsSiegeWar->m_eCurrentStatus >= AGPD_SIEGE_WAR_STATUS_TIME_OVER)
			bCheck = FALSE;

		// 2006.08.28. steeple
		// 공성, 수성 전용 체크를 해준다.
		if((pcsTemplate->m_lUsingType & AGPMITEM_USING_TYPE_SIEGE_WAR_ATTACK_ONLY) &&
			!(pcsTemplate->m_lUsingType & AGPMITEM_USING_TYPE_SIEGE_WAR_DEFENSE_ONLY))
		{
			if(!m_pagpmSiegeWar->IsAttackGuild(pcsItem->m_pcsCharacter, pcsSiegeWar))
				bCheck = FALSE;
		}

		if(!(pcsTemplate->m_lUsingType & AGPMITEM_USING_TYPE_SIEGE_WAR_ATTACK_ONLY) &&
			(pcsTemplate->m_lUsingType & AGPMITEM_USING_TYPE_SIEGE_WAR_DEFENSE_ONLY))
		{
			if(!m_pagpmSiegeWar->IsDefenseGuild(pcsItem->m_pcsCharacter, pcsSiegeWar))
				bCheck = FALSE;
		}

		// guild master only item
		if (pcsItem->m_pcsItemTemplate->m_eBuyerType == AGPMITEM_BUYER_TYPE_GUILD_MASTER_ONLY)
		{
			AgsmGuild* pAgsmGuild = (AgsmGuild*)GetModule("AgsmGuild");
			if (FALSE == pAgsmGuild->IsGuildMaster(pcsItem->m_pcsCharacter))
				bCheck = FALSE;
		}
	}

	return bCheck;
}

// 2006.12.12. steeple
// 옵션에 의해서 스킬데이터가 바뀌는 것을 처리한다.
BOOL AgsmItem::CalcItemOptionSkillData(AgpdItem* pcsItem, BOOL bAdd, INT32 lPrevLevel, BOOL bEvolution)
{
	if(!pcsItem || !pcsItem->m_pcsItemTemplate || !pcsItem->m_pcsCharacter)
		return FALSE;

	//STOPWATCH2(GetModuleName(), _T("CalcItemOptionSkillData"));

	AgpdCharacter* pcsCharacter = pcsItem->m_pcsCharacter;
	AgsdCharacter* pcsAgsdCharacter = m_pagsmCharacter->GetADCharacter(pcsCharacter);
	if(!pcsAgsdCharacter)
		return FALSE;

	AgsdItem* pcsAgsdItem = GetADItem(pcsItem);
	if(!pcsAgsdItem)
		return FALSE;

	// 아크로드 스킬은 항상 1레벨 고정이라 스킬데이터가 바뀌지 않으니 계산하지 않는다
	// 홀리 브리스(패시브)스킬 관련 체력이 줄어드는 문제 수정
	if ( TRUE == m_pagpmCharacter->IsArchlord(pcsCharacter) )
		return TRUE;

	// Option Data 부터 처리해준다. 2007.05.04. steeple
	if(bAdd)
	{
		pcsAgsdCharacter->m_stOptionSkillData += pcsAgsdItem->m_stOptionSkillData;

		AgpdItemConvertADItem* pcsAttachItemData = m_pagpmItemConvert->GetADItem(pcsItem);
		if(pcsAttachItemData)
			pcsAgsdCharacter->m_stOptionSkillData += pcsAttachItemData->m_stOptionSkillData;
	}
	else
	{
		pcsAgsdCharacter->m_stOptionSkillData -= pcsAgsdItem->m_stOptionSkillData;
		
		AgpdItemConvertADItem* pcsAttachItemData = m_pagpmItemConvert->GetADItem(pcsItem);
		if(pcsAttachItemData)
			pcsAgsdCharacter->m_stOptionSkillData -= pcsAttachItemData->m_stOptionSkillData;
	}

	// 여기부터는 레벨에 따라 바뀌는 링크처리.

	INT32 lLevel = m_pagpmCharacter->GetLevel(pcsCharacter);

	// 레벨업 해서 새로 계산하러 들어온 경우라면 레벨을 이전 레벨로 맞춰준다.
	if(!bAdd && lPrevLevel)
		lLevel = lPrevLevel;

	AgpdItemOptionTemplate* pcsItemOptionTemplate = NULL;
	for(int i = 0; i < AGPDITEM_LINK_MAX_NUM; ++i)
	{
		if(pcsItem->m_pcsItemTemplate->m_alLinkID[i] == 0)
			break;

		// 레벨에 알맞는 링크를 새로 얻는다.
		pcsItemOptionTemplate = m_pagpmItem->GetFitLinkTemplate(pcsItem->m_pcsItemTemplate->m_alLinkID[i], lLevel);
		if(!pcsItemOptionTemplate)
			continue;

		if(bAdd)
		{
			// 진화로 인해서 재계산인 경우는 아이템 옵션값은 재계산 해줄 필요가 없다
			if(FALSE == bEvolution)
			{
				// 기본 Factor Data 계산
				m_pagpmFactors->CalcFactor(&pcsItem->m_csFactor, &pcsItemOptionTemplate->m_csFactor, FALSE, FALSE, TRUE);
				m_pagpmFactors->CalcFactor(&pcsItem->m_csFactorPercent, &pcsItemOptionTemplate->m_csFactorPercent, FALSE, FALSE, TRUE);

				// Factor Data 도 일단 Item 쪽에 계산해준다.
				m_pagpmFactors->CalcFactor(&pcsItem->m_csFactor, &pcsItemOptionTemplate->m_csSkillFactor, FALSE, FALSE, TRUE);
				m_pagpmFactors->CalcFactor(&pcsItem->m_csFactorPercent, &pcsItemOptionTemplate->m_csSkillFactorPercent, FALSE, FALSE, TRUE);
			}

			// Skill Data 계산
			pcsAgsdCharacter->m_stOptionSkillData += pcsItemOptionTemplate->m_stSkillData;
		}
		else
		{
			// 진화로 인해서 재계산인 경우는 아이템 옵션값은 재계산 해줄 필요가 없다
			if(FALSE == bEvolution)
			{
				// 기본 Factor Data 계산
				m_pagpmFactors->CalcFactor(&pcsItem->m_csFactor, &pcsItemOptionTemplate->m_csFactor, FALSE, FALSE, FALSE);
				m_pagpmFactors->CalcFactor(&pcsItem->m_csFactorPercent, &pcsItemOptionTemplate->m_csFactorPercent, FALSE, FALSE, FALSE);

				// Factor Data 도 일단 Item 쪽에 계산해준다.
				m_pagpmFactors->CalcFactor(&pcsItem->m_csFactor, &pcsItemOptionTemplate->m_csSkillFactor, FALSE, FALSE, FALSE);
				m_pagpmFactors->CalcFactor(&pcsItem->m_csFactorPercent, &pcsItemOptionTemplate->m_csSkillFactorPercent, FALSE, FALSE, FALSE);
			}

			// Skill Data 계산
			pcsAgsdCharacter->m_stOptionSkillData -= pcsItemOptionTemplate->m_stSkillData;
		}
	}

	// 2007.02.06. steeple
	// Modified Skill Level 을 다시 계산해준다.
	m_pagpmSkill->UpdateModifiedSkillLevel((ApBase*)pcsCharacter);

	return TRUE;
}

// 2006.12.26. steeple
// 보통 세트아이템을 장착한 상태에서 레벨업 하면 아래 처리를 해준다.
// bFirst 가 TRUE 인 경우는 EnterGameWorld 시 딱 한번이다.
BOOL AgsmItem::UpdateItemOptionSkillData(AgpdCharacter* pcsCharacter, BOOL bFirst, INT32 lPrevLevel, BOOL bEquip)
{
	if(!pcsCharacter)
		return FALSE;

	//STOPWATCH2(GetModuleName(), _T("UpdateItemOptionSkillData"));

	ApmMap::RegionTemplate* pcsPrevRegionTemplate = m_papmMap->GetTemplate(pcsCharacter->m_nBeforeRegionIndex);

	INT32 lCurrLevel = m_pagpmCharacter->GetLevel(pcsCharacter);
	if(bFirst)
		lPrevLevel = lCurrLevel;
	else if(!lPrevLevel)
		lPrevLevel = lCurrLevel - 1;

	INT32 lLevelDiff = lCurrLevel - lPrevLevel;

	BOOL bChanged = FALSE;
	BOOL bRide = FALSE;
	BOOL bCheckDragonScionWeapon = FALSE;
	BOOL bCheckDragonScionWeapon2 = FALSE;
	BOOL bCheckDragonScionWeaponAdd = FALSE;
	BOOL bTransform = FALSE;
	BOOL bCalcBoth = FALSE;
	for(INT32 lPart = (INT32)AGPMITEM_PART_BODY;
		lPart < (INT32)AGPMITEM_PART_NUM;
		++lPart)
	{
		BOOL	bAdd = FALSE;

		AgpdGridItem* pcsGridItem = m_pagpmItem->GetEquipItem(pcsCharacter, lPart);
		if(!pcsGridItem)
			continue;

		AgpdItem* pcsItem = m_pagpmItem->GetItem(pcsGridItem);
		if(!pcsItem || !pcsItem->m_pcsItemTemplate)
			continue;

		// 내구도 체크해야 한다. 으악~~ 2007.10.30. steeple
		INT32 lCurrentDurability = m_pagpmItem->GetItemDurabilityCurrent(pcsItem);
		INT32 lMaxTemplateDurability = m_pagpmItem->GetItemDurabilityMax(pcsItem->m_pcsItemTemplate);
		if(lCurrentDurability < 1 && lMaxTemplateDurability != 0)
			continue;

		/*if ((lLimitedLevel != 0 && lLimitedLevel < lCharLevel) ||	// 최대 레벨 초과
			(lMinLevel > lCharLevel) ||							// 최소 레벨 미만.
			m_pagpmItem->CheckUseItem(pcsCharacter, pcsItem) == FALSE)*/

		if(m_pagpmItem->CheckUseItem(pcsCharacter, pcsItem, TRUE) == FALSE)
		{
			continue;
		}

		bChanged = TRUE;

		if(lPart == AGPMITEM_PART_RIDE)
			bRide = TRUE;

		if(lPart == AGPMITEM_PART_LANCER && !bRide)
		{
			continue;
		}

		// 일단 팩터 계산을 한다.
		if(pcsPrevRegionTemplate && pcsPrevRegionTemplate->nLevelLimit != 0) 
		{
			INT32 lMinLevel = m_pagpmFactors->GetLevel(&pcsItem->m_csRestrictFactor);
			if(lMinLevel > pcsPrevRegionTemplate->nLevelLimit) 
			{
				//if(!pcsItem->m_pcsItemTemplate->m_alLinkID[0]) bAdd  = TRUE;	
				bAdd  = TRUE;
			}
		}

		if(!bFirst && !bAdd)
		{
			BOOL		bCalcFactor = TRUE;
			AgpdFactor	csItemUpdateFactorPoint;
			AgpdFactor	csItemUpdateFactorPercent;

			m_pagpmFactors->InitFactor(&csItemUpdateFactorPoint);
			m_pagpmFactors->InitFactor(&csItemUpdateFactorPercent);

			m_pagpmFactors->CopyFactor(&csItemUpdateFactorPoint, &pcsItem->m_csFactor, TRUE);
			m_pagpmFactors->CopyFactor(&csItemUpdateFactorPercent, &pcsItem->m_csFactorPercent, TRUE);

			// 드래곤 시온을 위한 코드
			if(lPart == AGPMITEM_PART_HAND_LEFT || lPart == AGPMITEM_PART_HAND_RIGHT)
			{
				AgpdItem* pcsItemL		= m_pagpmItem->GetEquipSlotItem(pcsCharacter, AGPMITEM_PART_HAND_LEFT);
				AgpdItem* pcsItemR		= m_pagpmItem->GetEquipSlotItem(pcsCharacter, AGPMITEM_PART_HAND_RIGHT);

				INT32 lMinLevelItemL	= 0;
				INT32 lMinLevelItemR	= 0;
				
				if(pcsItemL)
					lMinLevelItemL = m_pagpmFactors->GetLevel(&pcsItemL->m_csRestrictFactor);

				if(pcsItemR)
					lMinLevelItemR = m_pagpmFactors->GetLevel(&pcsItemR->m_csRestrictFactor);

				if(pcsItemL && pcsItemR)
				{
					if(m_pagpmItem->GetWeaponType(pcsItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_ZENON)
					{
						if(bCheckDragonScionWeapon == FALSE)
						{
							if((m_pagpmItem->CheckUseItem(pcsCharacter, pcsItemL) || bCheckDragonScionWeapon == FALSE) && 
								m_pagpmItem->GetWeaponType(pcsItemL->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_CHARON)
							{
								bCalcFactor = FALSE;
								bCheckDragonScionWeapon = TRUE;
							}
						}
					}

					if(m_pagpmItem->GetWeaponType(pcsItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_CHARON)
					{
						if(bCheckDragonScionWeapon == FALSE)
						{
							if((m_pagpmItem->CheckUseItem(pcsCharacter, pcsItemR) || bCheckDragonScionWeapon == FALSE) && 
								m_pagpmItem->GetWeaponType(pcsItemR->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_ZENON)
							{
								bCalcFactor = FALSE;
								bCheckDragonScionWeapon = TRUE;
							}
						}
					}

					// 카오틱에 입장할때
					if(pcsPrevRegionTemplate && pcsPrevRegionTemplate->nLevelLimit == 0)
					{
						bCalcBoth = TRUE;

						if(!bCalcFactor)
						{
							m_pagpmFactors->SetValue(&csItemUpdateFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
							m_pagpmFactors->SetValue(&csItemUpdateFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
						}
					}
					// 다시 복귀할때
					else
					{
						if(pcsPrevRegionTemplate && lMinLevelItemL <= pcsPrevRegionTemplate->nLevelLimit && 
						   lMinLevelItemR <= pcsPrevRegionTemplate->nLevelLimit)
						{
							bCalcBoth = TRUE;

							if(!bCalcFactor)
							{
								m_pagpmFactors->SetValue(&csItemUpdateFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
								m_pagpmFactors->SetValue(&csItemUpdateFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
							}
						}
					}
				}		
			}	

			m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPoint, &csItemUpdateFactorPoint, FALSE, FALSE, FALSE, FALSE);
			m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPercent, &csItemUpdateFactorPercent, FALSE, FALSE, FALSE, FALSE);

			// 일단 기존 레벨 꺼를 빼주고
	//		m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPoint, &pcsItem->m_csFactor, FALSE, FALSE, FALSE, FALSE);
	//		m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPercent, &pcsItem->m_csFactorPercent, FALSE, FALSE, FALSE, FALSE);

			CalcItemOptionSkillData(pcsItem, FALSE, lPrevLevel);

			CalcItemOptionIgnoreStatus(pcsCharacter, pcsItem, FALSE);

			if(bCalcBoth && bCalcFactor == FALSE)
				AdjustRecalcFactor(pcsCharacter, pcsItem, FALSE, bCheckDragonScionWeapon2, TRUE);
			else
				AdjustRecalcFactor(pcsCharacter, pcsItem, FALSE, TRUE);
			
			if(bCheckDragonScionWeapon2 == FALSE && bCalcFactor == FALSE)
				bCheckDragonScionWeapon2 = TRUE;
		}

		// 레벨 제한 걸린 아이템이라면 계산하지 않아야 한다. 2008.04.08. steeple
		INT32 lCharLevel	= m_pagpmCharacter->GetLevel(pcsCharacter);
		INT32 lLimitedLevel	= pcsItem->m_pcsItemTemplate->m_lLimitedLevel;
		INT32 lMinLevel		= m_pagpmFactors->GetLevel(&pcsItem->m_csRestrictFactor);

		if ((lLimitedLevel == 0 || lLimitedLevel >= lCharLevel) &&	// 최대 레벨 초과
			(lMinLevel <= lCharLevel) &&							// 최소 레벨 미만.
			m_pagpmItem->CheckUseItem(pcsCharacter, pcsItem))
		{
			// 새로운 레벨로 계산
			CalcItemOptionSkillData(pcsItem);

			CalcItemOptionIgnoreStatus(pcsCharacter, pcsItem, TRUE);

			AgpdFactor pcsUpdateFactor;
			AgpdFactor pcsUpdateFactorPercent;

			m_pagpmFactors->InitFactor(&pcsUpdateFactor);
			m_pagpmFactors->InitFactor(&pcsUpdateFactorPercent);

			m_pagpmFactors->CopyFactor(&pcsUpdateFactor, &pcsItem->m_csFactor, TRUE);
			m_pagpmFactors->CopyFactor(&pcsUpdateFactorPercent, &pcsItem->m_csFactorPercent, TRUE);

			if(lPart == AGPMITEM_PART_HAND_LEFT || lPart == AGPMITEM_PART_HAND_RIGHT)
			{
				AgpdItem* pcsItemL		= m_pagpmItem->GetEquipSlotItem(pcsCharacter, AGPMITEM_PART_HAND_LEFT);
				AgpdItem* pcsItemR		= m_pagpmItem->GetEquipSlotItem(pcsCharacter, AGPMITEM_PART_HAND_RIGHT);

				if(pcsItemL && pcsItemR)
				{
					if(m_pagpmItem->CheckUseItem(pcsCharacter, pcsItemL) && m_pagpmItem->CheckUseItem(pcsCharacter, pcsItemR))
					{
						m_pagpmItem->AdjustDragonScionWeaponFactor(pcsCharacter, pcsItem, &pcsUpdateFactor, &pcsUpdateFactorPercent, bTransform);

						if(bTransform == FALSE)
							bTransform = TRUE;
					}			
				}
			}

			m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPoint, &pcsUpdateFactor, FALSE, FALSE, TRUE, FALSE);
			m_pagpmFactors->CalcFactor(&pcsCharacter->m_csFactorPercent, &pcsUpdateFactorPercent, FALSE, FALSE, TRUE, FALSE);

			if(bTransform == TRUE)
				AdjustRecalcFactor(pcsCharacter, pcsItem, TRUE, bCheckDragonScionWeaponAdd);
			else
				AdjustRecalcFactor(pcsCharacter, pcsItem, TRUE, TRUE);
			
			if(bCheckDragonScionWeaponAdd == FALSE && bTransform == TRUE)
				bCheckDragonScionWeaponAdd = TRUE;
		}
	}

	return bChanged;
}

// 2007.02.05. steeple
BOOL AgsmItem::ProcessItemSkillPlus(AgpdItem* pcsItem, AgpdCharacter* pcsCharacter)
{
	if(!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	if(pcsItem->m_pcsItemTemplate->m_aunSkillPlusTID[0] == 0)
		return TRUE;

#ifdef _DEBUG
	//// Debug 일 때는 pcsCharacter 로 전체 Validation 검사를 해본다.

	//AgpdSkillTemplate* pcsSkillTemplate = NULL;

	//for(int i = 0; i < AGPMITEM_MAX_SKILL_PLUS; ++i)
	//{
	//	if(pcsItem->m_pcsItemTemplate->m_aunSkillPlusTID[i] == 0)
	//		break;

	//	pcsSkillTemplate = m_pagpmSkill->GetSkillTemplate(pcsItem->m_pcsItemTemplate->m_aunSkillPlusTID[i]);
	//	if(!pcsSkillTemplate)
	//		continue;

	//	// 클래스만 체크한다.
	//	if(!m_pagpmSkill->CheckLearnableSkill(pcsCharacter, pcsSkillTemplate->m_szName))
	//	{
	//		ASSERT(!"Skill Plus TID is invalid");
	//	}
	//}
#endif

	// 먼저 비운다.
	pcsItem->m_aunSkillPlusTID.MemSetAll();

	ApSafeArray<UINT16, AGPMITEM_MAX_SKILL_PLUS_EFFECT> aunEffectTID;
	aunEffectTID.MemSetAll();

	// 0 을 찾는다.
	ApSafeArray<UINT16, AGPMITEM_MAX_SKILL_PLUS>::iterator iter = std::find(pcsItem->m_pcsItemTemplate->m_aunSkillPlusTID.begin(),
																			pcsItem->m_pcsItemTemplate->m_aunSkillPlusTID.end(),
																			0);

	// 표본에서 랜덤하게 AGPMITEM_MAX_SKILL_PLUS_EFFECT 개수만큼 뽑는다.
	std::random_sample(pcsItem->m_pcsItemTemplate->m_aunSkillPlusTID.begin(),
						iter,		// 0 이 위치하기 직전까지만 표본을 뽑는다.
						aunEffectTID.begin(),
						aunEffectTID.end());

	// 한번 섞어준다.
	std::random_shuffle(aunEffectTID.begin(), aunEffectTID.end());

	INT32 lRandom = m_csRandomNumber.randInt(3);
	++lRandom;	// 1~3

	ASSERT(lRandom > 0 && lRandom < 4);

	for(int i = 0; i < lRandom; ++i)
		m_pagpmItem->AddItemSkillPlus(pcsItem, aunEffectTID[i]);
	
	return TRUE;
}

// 2008.01.28. steeple
// 캐릭터 레벨이 변할 때 Limited Level 제한이 있는 아이템은 내려주거나,
// 아이템의 효과를 없애주거나, 해당 아이템에 연결된 스킬을 종료 시켜준다.
BOOL AgsmItem::ProcessLimitedLevelItemOnLevelUp(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	// Equip Grid 를 돌면서 처리한다.
	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);

	if(pcsItemADChar == NULL)
		return FALSE;

	BOOL bAffected = FALSE;
	INT32 lCharLevel = m_pagpmCharacter->GetLevel(pcsCharacter);

	for(int i = 0; i < AGPMITEM_PART_NUM; ++i)
	{
		AgpdGridItem* pcsGridItem = m_pagpmItem->GetEquipItem(pcsCharacter, i);

		if(!pcsGridItem)
			continue;

		AgpdItem* pcsItem = m_pagpmItem->GetItem(pcsGridItem);
		if(!pcsItem || !pcsItem->m_pcsItemTemplate)
			continue;

		INT32 lLimitedLevel = pcsItem->m_pcsItemTemplate->m_lLimitedLevel;
		if(lLimitedLevel == 0 || lCharLevel <= lLimitedLevel)
			continue;

		bAffected = TRUE;

		// 현재(2008.01.28. steeple) 로써는 효과를 없애준다.
		// 일단, 해당 스킬을 꺼준다.
		// 아이템의 효과를 없애는 것은, 그냥 나중에 Recalc 해주면 된다.
		EnumCallback(AGSMITEM_CB_END_BUFFED_SKILL_BY_ITEM, pcsCharacter, pcsItem);
	}

	// Cash Inventory 도 돌린다.
	for(int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nRow; ++i)
	{
		AgpdGridItem* pcsGridItem = m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, 0, i, 0 );
		if(!pcsGridItem)
			continue;

		AgpdItem* pcsItem = m_pagpmItem->GetItem(pcsGridItem);
		if(!pcsItem)
			continue;

		if(!pcsItem->m_nInUseItem)
			continue;

		INT32 lLimitedLevel = pcsItem->m_pcsItemTemplate->m_lLimitedLevel;
		if(lLimitedLevel == 0 || lCharLevel <= lLimitedLevel)
			continue;

		bAffected = TRUE;

		// 아바타면 스킬만 꺼주고, 스킬 스크롤이면 캐쉬템을 정지 시켜준다.
		if(m_pagpmItem->IsAvatarItem(pcsItem->m_pcsItemTemplate))
		{
			// 현재(2008.01.28. steeple) 로써는 효과를 없애준다.
			// 일단, 해당 스킬을 꺼준다.
			// 아이템의 효과를 없애는 것은, 그냥 나중에 Recalc 해주면 된다.
			EnumCallback(AGSMITEM_CB_END_BUFFED_SKILL_BY_ITEM, pcsCharacter, pcsItem);
		}
		else if(((AgpdItemTemplateUsable*)pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_SKILL_SCROLL)
		{
			UnuseItem(pcsItem);
		}
	}

	return bAffected;
}

// 2008.02.15. steeple
// Cooldown 정보 인코딩
BOOL AgsmItem::EncodingCooldown(AgpdCharacter* pcsCharacter, CHAR* szBuffer, INT32 lBufferLength)
{
	if(!pcsCharacter || !szBuffer || lBufferLength < 1)
		return FALSE;

	ZeroMemory(szBuffer, sizeof(szBuffer));

	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	AgpdItemCooldownArray::CooldownBaseIter iter = pcsItemADChar->m_CooldownInfo.m_pBases->begin();
	while(iter != pcsItemADChar->m_CooldownInfo.m_pBases->end())
	{
		sprintf(szBuffer + strlen(szBuffer), "%d,%d:", iter->m_lTID, iter->m_ulRemainTime);

		if(strlen(szBuffer) + 16 > lBufferLength)
		{
			ASSERT(!"Exceed an encodingCooldown buffer!!");
			return TRUE;
		}

		++iter;
	}

	return TRUE;
}

// 2008.02.15. steeple
// Cooldown 정보 디코딩
BOOL AgsmItem::DecodingCooldown(AgpdCharacter* pcsCharacter, CHAR* szBuffer, INT32 lBufferLength)
{
	if(!pcsCharacter || !szBuffer || lBufferLength < 1)
		return FALSE;

	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	INT32 lDecodedLength = 0;
	while(lDecodedLength < lBufferLength)
	{
		INT32 lTID = 0;
		INT32 lRemainTime = 0;

		int j = 0;
		for(j = lDecodedLength; j < lBufferLength; ++j)
		{
			if(szBuffer[j] == ':')
				break;
		}

		if(j == lBufferLength)
			return TRUE;

		sscanf(szBuffer + lDecodedLength, "%d,%d:", &lTID, &lRemainTime);
		lDecodedLength	= j + 1;

		AgpdItemCooldownBase stBase;
		stBase.m_lTID = lTID;
		stBase.m_ulRemainTime = (UINT32)lRemainTime;

		pcsItemADChar->m_CooldownInfo.m_pBases->push_back(stBase);
	}

	return TRUE;
}

// 2008.06.19. steeple
// Check pet's stamina and feed a food when its stamina would reach the line.
BOOL AgsmItem::CheckStatminaPet(AgpdItem* pcsItem)
{
	if(!pcsItem || !pcsItem->m_pcsCharacter || pcsItem->m_pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE)
		return FALSE;

	// Exit if it isn't doing now.
	if(pcsItem->m_nInUseItem != AGPDITEM_CASH_ITEM_INUSE)
		return FALSE;

	// Find a pet.
	INT32 lPetTID = m_pagpmSkill->GetSummonsTIDByItem(pcsItem->m_pcsItemTemplate);
	INT32 lPetCID = m_pagpmSummons->GetSummonsCIDByTID(pcsItem->m_pcsCharacter, lPetTID);
	if(lPetCID == 0)
		return FALSE;

	AgpdCharacterTemplate* pcsPetTemplate = m_pagpmCharacter->GetCharacterTemplate(lPetTID);
	if(!pcsPetTemplate)
		return FALSE;

	// Compare pet's template stamina point with pcsItem's remaining stamina point.
	if(pcsItem->m_llStaminaRemainTime > pcsPetTemplate->m_lStaminaPoint * 1000)
		return FALSE;

	// Find a food (just in using)
	AgpdItemADChar* pcsItemADChar = m_pagpmItem->GetADCharacter(pcsItem->m_pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	AgpdItem* pcsFood = NULL;
	INT32 lIndex = 0;
	for(AgpdGridItem* pcsGridItem = m_pagpmGrid->GetItemSequence(&pcsItemADChar->m_csCashInventoryGrid, &lIndex);
		pcsGridItem;
		pcsGridItem = m_pagpmGrid->GetItemSequence(&pcsItemADChar->m_csCashInventoryGrid, &lIndex))
	{
		AgpdItem* pcsCashItem = m_pagpmItem->GetItem(pcsGridItem);
		if(!pcsCashItem || !pcsCashItem->m_pcsItemTemplate || pcsCashItem->m_nInUseItem != AGPDITEM_CASH_ITEM_INUSE)
			continue;

		// Pass an item having no stamina cure value.
		if(pcsCashItem->m_pcsItemTemplate->m_llStaminaCure == 0)
			continue;

		pcsFood = pcsCashItem;
		break;
	}

	if(!pcsFood)
		return FALSE;

	// Feed
	pcsItem->m_llStaminaRemainTime += pcsFood->m_pcsItemTemplate->m_llStaminaCure;
	if(pcsItem->m_llStaminaRemainTime >= pcsItem->m_pcsItemTemplate->m_llStaminaRemainTime)
		pcsItem->m_llStaminaRemainTime = pcsItem->m_pcsItemTemplate->m_llStaminaRemainTime - 1;

	// Send Packet
	SendPacketUpdateItemStaminaRemainTime(pcsItem);

	WriteUseLog(pcsItem->m_pcsCharacter->m_lID, pcsItem);
	// Remove a food item
	m_pagpmItem->SubItemStackCount(pcsFood, 1, TRUE);


	return TRUE;
}

BOOL AgsmItem::IsCharacterUsingTimeLimitItem(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgsdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return FALSE;

	return pcsItemADChar->m_bUseTimeLimitItem;
}

BOOL AgsmItem::UseItemJanusTalisman( AgpdItem *pcsItem, AgpdCharacter *pcsTargetChar, BOOL bIsWriteLog /*= TRUE*/ )
{
	if (!pcsItem || !pcsItem->m_pcsCharacter || !pcsItem->m_pcsItemTemplate )
		return FALSE;

	if(pcsItem->m_pcsCharacter->m_unCurrentStatus != AGPDCHAR_STATUS_IN_GAME_WORLD)
		return FALSE;

	// 사용가능한 아이템인지 본다.
	if (!m_pagpmItem->CheckUseItem(pcsItem->m_pcsCharacter, pcsItem))
		return FALSE;

	// 2005.12.26. steeple
	// 캐쉬 아이템인 경우, 탈것을 타도 가능한 거라면 통과이다.
	if(pcsItem->m_pcsCharacter->m_bRidable ||
		!m_pagpmArchlord->IsArchlord(pcsItem->m_pcsCharacter->m_szID) && pcsItem->m_pcsCharacter->m_bIsTrasform ||
		m_pagpmCharacter->IsStatusTransparent(pcsItem->m_pcsCharacter))
	{
		if(IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType) && pcsItem->m_pcsItemTemplate->m_bEnableOnRide)
		{}
		else
		{
			return FALSE;
		}
	}

	if (!pcsTargetChar)
		pcsTargetChar	= pcsItem->m_pcsCharacter;

	if (pcsItem->m_pcsCharacter &&
		pcsItem->m_pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
		return FALSE;

	// 2006.01.05. steeple
	// 캐쉬 아이템이 이미 적용중인 버프등의 영향으로 인해서 적용이 안되야 하는 지 검사한다.
	if (EnumCallback(AGSMITEM_CB_PRE_CHECK_ENABLE_JANUS_TALISMAN, pcsItem, pcsTargetChar) == FALSE)
	{
		return FALSE;
	}

	if (EnumCallback(AGSMITEM_CB_USE_ITEM_JANUS_TALISMAN, pcsItem, pcsTargetChar))
	{
		//m_pagpmItem->SubItemStackCount(pcsItem, 1);

		if (bIsWriteLog && pcsItem->m_pcsCharacter)
		{
			if (IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType))
				WriteUseStartLog(pcsItem->m_pcsCharacter->m_lID, pcsItem, pcsItem->m_nCount);
			else
				WriteUseLog(pcsItem->m_pcsCharacter->m_lID, pcsItem);
		}
	}
	else
		return FALSE;

	// 야누스 타입 설정
	SetCharacterJanusTypeFromItem( pcsItem->m_pcsCharacter, ((AgpdItemTemplateUsableJanusTalisman *) pcsItem->m_pcsItemTemplate)->m_eJanusTalismanType );
	
	return TRUE;
}

BOOL AgsmItem::SetCharacterJanusTypeFromItem( AgpdCharacter *pcsCharacter, AgpmItemUsableJanusTalismanType eType )
{
	if (!pcsCharacter)
		return FALSE;

	switch(eType)
	{
	case AGPMITEM_USABLE_JANUS_TALISMAN_TYPE_NONE:
		pcsCharacter->m_eJanusType = AGPDCHAR_JANUS_TALISMAN_NONE;
		break;
	case AGPMITEM_USABLE_JANUS_TALISMAN_TYPE_EXP:
		pcsCharacter->m_eJanusType = AGPDCHAR_JANUS_TALISMAN_EXP;
		break;
	case AGPMITEM_USABLE_JANUS_TALISMAN_TYPE_DROP:
		pcsCharacter->m_eJanusType = AGPDCHAR_JANUS_TALISMAN_DROP;
		break;
	case AGPMITEM_USABLE_JANUS_TALISMAN_TYPE_DROP2:
		pcsCharacter->m_eJanusType = AGPDCHAR_JANUS_TALISMAN_DROP2;
		break;
	case AGPMITEM_USABLE_JANUS_TALISMAN_TYPE_GHELD:
		pcsCharacter->m_eJanusType = AGPDCHAR_JANUS_TALISMAN_GHELD;
		break;
	case AGPMITEM_USABLE_JANUS_TALISMAN_TYPE_MAX:
		pcsCharacter->m_eJanusType = AGPDCHAR_JANUS_TALISMAN_MAX;
		break;
	}

	return TRUE;	
}

BOOL AgsmItem::AddItemToCharacterPointForGacha( AgpdCharacter *pcsCharacter, AgpdItem *pcsItem )
{
	if ( !pcsCharacter || !pcsItem || !pcsItem->m_pcsItemTemplate )
		return FALSE;

	if ( FALSE == pcsItem->m_pcsItemTemplate->m_bIsGachaPointType )
		return FALSE;

	BOOL	bResult = FALSE;

	stGachaPoint *pGachaPoint = &pcsItem->m_pcsItemTemplate->m_stGachaPoint;

	if ( 0 != pGachaPoint->m_nPointEXP )
	{
		bResult |= m_pagsmCharacter->AddExp( pcsCharacter, pGachaPoint->m_nPointEXP );
	}
	if ( 0 != pGachaPoint->m_nPointGheld )
	{
		bResult |= m_pagpmCharacter->AddMoney( pcsCharacter, pGachaPoint->m_nPointGheld );
	}
	if ( 0 != pGachaPoint->m_nPointCharismaPoint )
	{
		bResult |= m_pagpmCharacter->AddCharismaPoint( pcsCharacter, pGachaPoint->m_nPointCharismaPoint );
	}
	if ( 0 != pGachaPoint->m_nPercentEXP )
	{
		bResult |= m_pagsmCharacter->AddExpPercent( pcsCharacter, pGachaPoint->m_nPercentEXP );
	}
	if ( 0 != pGachaPoint->m_nPercentGheld )
	{
		bResult |= pcsCharacter->m_llMoney += (INT64)(pcsCharacter->m_llMoney * ( 1 + (FLOAT)pGachaPoint->m_nPercentGheld / 100 ));
	}
	if ( 0 != pGachaPoint->m_nPercentCharismaPoint )
	{
		INT32 nCharismaPoint = m_pagpmCharacter->GetCharismaPoint(pcsCharacter);

		nCharismaPoint += (INT32)(nCharismaPoint * ( 1 + (FLOAT)pGachaPoint->m_nPercentCharismaPoint / 100 ));
		bResult |= m_pagpmCharacter->UpdateCharismaPoint( pcsCharacter, nCharismaPoint );
	}

	return bResult;
}

BOOL AgsmItem::CalcItemOptionIgnoreStatus( AgpdCharacter *pcsCharacter, AgpdItem* pcsItem, BOOL bAdd )
{
	if (!pcsItem || !pcsItem->m_pcsItemTemplate || !pcsCharacter || !pcsCharacter->m_pcsCharacterTemplate)
		return FALSE;

	// Ignore Status 계산
	for (INT32 i = 0; i < AGPDITEM_OPTION_MAX_NUM; i++)
	{
		if ( pcsItem->m_aunOptionTID[i] == 0 )
			break;

		AgpdItemOptionTemplate*	pcsItemOptionTemplate = NULL;
		pcsItemOptionTemplate = m_pagpmItem->GetItemOptionTemplate(pcsItem->m_aunOptionTID[i]);
		if (!pcsItemOptionTemplate)
			continue;

		AgpdCharacterIgnoreStatus stIgnoreStatus;
		if (bAdd)
			stIgnoreStatus = stIgnoreStatus + pcsItemOptionTemplate->m_stSkillData.m_stIgnoreStatus;
		else
			stIgnoreStatus = stIgnoreStatus - pcsItemOptionTemplate->m_stSkillData.m_stIgnoreStatus;

		m_pagsmCharacter->CalcCharacterIgnoreStatus(pcsCharacter, &stIgnoreStatus);
	}

	return TRUE;
}
