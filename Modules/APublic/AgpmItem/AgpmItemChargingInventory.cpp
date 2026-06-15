#include "AgpmItem.h"

// ************************************************************************/
// define
//
//--------------------------------------------------------------------------

#define CHARGING_INVENTORY_TYPE_A_LAYER			1
#define CHARGING_INVENTORY_TYPE_A_COLUMN		4
#define CHARGING_INVENTORY_TYPE_A_ROW			4
#define CHARGING_INVENTORY_TYPE_B_LAYER			1
#define CHARGING_INVENTORY_TYPE_B_COLUMN		4
#define CHARGING_INVENTORY_TYPE_B_ROW			8

BOOL AgpmItem::SetCallbackChargingInventoryCheckAdd(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_CHARGING_INVENTORY_CHECK_ADD, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackChargingInventoryAdd(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_CHARGING_INVENTORY_ADD, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackChargingInventoryUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_CHARGING_INVENTORY_UPDATE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackChargingInventoryRemove(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_CHARGING_INVENTORY_REMOVE, pfCallback, pClass);
}


AgpdGrid* AgpmItem::GetChargingInventory(INT32 lCID)
{
	AgpdCharacter* pcsCharacter = m_pagpmCharacter->GetCharacter(lCID);
	if(!pcsCharacter)
		return NULL;

	return GetChargingInventory(pcsCharacter);
}

AgpdGrid* AgpmItem::GetChargingInventory(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return NULL;

	AgpdItemADChar* pcsAgpdItemADChar = GetADCharacter(pcsCharacter);
	if(pcsAgpdItemADChar)
		return &pcsAgpdItemADChar->m_csChargingInventoryGrid;

	return NULL;
}

BOOL AgpmItem::AddItemCheckToChargingInventory(INT32 lCID, AgpdItem *pcsAgpdItem, INT32 lLayer, INT32 lRow, INT32 lColumn, BOOL bStackMerge)
{
	return AddItemCheckToChargingInventory(m_pagpmCharacter->GetCharacter(lCID), pcsAgpdItem, lLayer, lRow, lColumn, bStackMerge);
}

BOOL AgpmItem::AddItemCheckToChargingInventory(AgpdCharacter *pcsCharacter, AgpdItem *pcsAgpdItem, INT32 lLayer, INT32 lRow, INT32 lColumn, BOOL bStackMerge)
{
	// 아직은 미구현
	return TRUE;
}

BOOL AgpmItem::AddItemToChargingInventory(INT32 lCID, AgpdItem *pcsAgpdItem, INT32 lLayer, INT32 lRow, INT32 lColumn, BOOL bStackMerge)
{
	return AddItemToChargingInventory(m_pagpmCharacter->GetCharacter(lCID), pcsAgpdItem, lLayer, lRow, lColumn, bStackMerge);
}

BOOL AgpmItem::AddItemToChargingInventory(AgpdCharacter *pcsCharacter, AgpdItem *pcsAgpdItem, INT32 lLayer, INT32 lRow, INT32 lColumn, BOOL bStackMerge)
{
	if(!pcsCharacter || !pcsAgpdItem)
		return FALSE;

	if(!CheckUpdateStatus(pcsAgpdItem, AGPDITEM_STATUS_CHARGING_INVENTORY, lLayer, lRow, lColumn))
		return FALSE;

	if(IS_CASH_ITEM(pcsAgpdItem->m_pcsItemTemplate->m_eCashItemType))
	{
		return AddItemToCashInventory(pcsCharacter, pcsAgpdItem);
	}

	// Cannot insert money to a sub inventory
	if(((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_nType == AGPMITEM_TYPE_OTHER &&
		((AgpdItemTemplateOther *) pcsAgpdItem->m_pcsItemTemplate)->m_eOtherItemType == AGPMITEM_OTHER_TYPE_MONEY)
	{
		return FALSE;
	}

	if (lLayer < 0 && lRow < 0 && lColumn < 0)
		return AddItemToChargingInventory(pcsCharacter, pcsAgpdItem, bStackMerge);

	AgpdItemGridResult	eResult	= AGPDITEM_INSERT_FAIL;

	if(bStackMerge)
		eResult	= InsertStackMerge(pcsAgpdItem, GetChargingInventory(pcsCharacter), lLayer, lRow, lColumn);
	else
		eResult	= Insert(pcsAgpdItem, GetChargingInventory(pcsCharacter), lLayer, lRow, lColumn);

	return AddItemToChargingInventoryResult(eResult,
		pcsAgpdItem,
		pcsCharacter);
}

BOOL AgpmItem::AddItemToChargingInventory(AgpdCharacter *pcsCharacter, AgpdItem *pcsAgpdItem, BOOL bIsMergeStack)
{
	if (!pcsCharacter || !pcsAgpdItem)
		return FALSE;

	if (!CheckUpdateStatus(pcsAgpdItem, AGPDITEM_STATUS_CHARGING_INVENTORY, -1, -1, -1))
		return FALSE;

	if (IS_CASH_ITEM(pcsAgpdItem->m_pcsItemTemplate->m_eCashItemType))
	{
		return AddItemToCashInventory(pcsCharacter, pcsAgpdItem);
	}

	// Cannot insert money to a sub inventory
	if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_nType == AGPMITEM_TYPE_OTHER &&
		((AgpdItemTemplateOther *) pcsAgpdItem->m_pcsItemTemplate)->m_eOtherItemType == AGPMITEM_OTHER_TYPE_MONEY)
	{
		return FALSE;
	}

	AgpdItemGridResult	eResult	= AGPDITEM_INSERT_FAIL;

	if (bIsMergeStack)
		eResult	= AutoInsertStackMerge(pcsAgpdItem, GetChargingInventory(pcsCharacter));
	else
		eResult	= AutoInsert(pcsAgpdItem, GetChargingInventory(pcsCharacter));

	return AddItemToChargingInventoryResult(eResult,
		pcsAgpdItem,
		pcsCharacter);
}

BOOL AgpmItem::AddItemToChargingInventoryResult(AgpdItemGridResult eResult, AgpdItem *pcsAgpdItem, AgpdCharacter *pcsCharacter)
{
	if (!pcsAgpdItem || !pcsCharacter)
		return FALSE;

	AgpdItemADChar	*pcsAttachData	= GetADCharacter(pcsCharacter);

	INT32	lStackCount	= pcsAgpdItem->m_nCount;

	if (eResult == AGPDITEM_INSERT_SUCCESS)
	{
		// 이전 상태를 Release 시키고 현재 상태를 Charging Inventory로 바꾼다.
		RemoveStatus(pcsAgpdItem, AGPDITEM_STATUS_CHARGING_INVENTORY);

		pcsAgpdItem->m_eStatus	= AGPDITEM_STATUS_CHARGING_INVENTORY;

		ChangeItemOwner(pcsAgpdItem, pcsCharacter);

		// 만약 화살류의 아템이라면 카운트를 계산한다.
		if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetArrowTID())
		{
			pcsAttachData->m_lNumArrowCount += lStackCount;
		}
		else if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetBoltTID())
		{
			pcsAttachData->m_lNumBoltCount += lStackCount;
		}

		EnumCallback(ITEM_CB_ID_CHARGING_INVENTORY_ADD, pcsAgpdItem, NULL);

		pcsAgpdItem->m_anPrevGridPos	= pcsAgpdItem->m_anGridPos;

		return TRUE;
	}
	else if (eResult == AGPDITEM_INSERT_SUCCESS_STACKCOUNT_ZERO)
	{
		// 만약 화살류의 아템이라면 카운트를 계산한다.
		if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetArrowTID())
		{
			pcsAttachData->m_lNumArrowCount += lStackCount;
		}
		else if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetBoltTID())
		{
			pcsAttachData->m_lNumBoltCount += lStackCount;
		}

		ZeroMemory(pcsAgpdItem->m_szDeleteReason, sizeof(CHAR) * (AGPMITEM_MAX_DELETE_REASON + 1));
		strncpy(pcsAgpdItem->m_szDeleteReason, "Inventory:stackcount == 0", AGPMITEM_MAX_DELETE_REASON);
		//만들어진 아이템을 없앤다.
		RemoveItem( pcsAgpdItem, TRUE );

		return TRUE;
	}

	return FALSE;
}

BOOL AgpmItem::UpdateItemInChargingInventory(INT32 lCID, AgpdItem *pcsAgpdItem, INT32 lLayer, INT32 lRow, INT32 lColumn)
{
	return UpdateItemInChargingInventory(m_pagpmCharacter->GetCharacter(lCID), pcsAgpdItem, lLayer, lRow, lColumn);
}

BOOL AgpmItem::UpdateItemInChargingInventory(AgpdCharacter *pcsCharacter, AgpdItem *pcsAgpdItem, INT32 lLayer, INT32 lRow, INT32 lColumn)
{
	if (!pcsCharacter || !pcsAgpdItem)
		return FALSE;

	if (!CheckUpdateStatus(pcsAgpdItem, AGPDITEM_STATUS_CHARGING_INVENTORY, lLayer, lRow, lColumn))
		return FALSE;

	if (pcsAgpdItem->m_eStatus != AGPDITEM_STATUS_CHARGING_INVENTORY)
		return AddItemToChargingInventory(pcsCharacter, pcsAgpdItem, lLayer, lRow, lColumn);

	INT32	lPrevLayer	= pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB];
	INT32	lPrevRow	= pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW];
	INT32	lPrevColumn	= pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN];

	AgpdItemADChar	*pcsAttachData	= GetADCharacter(pcsCharacter);

	AgpdItemGridResult	eResult	= InsertStackMerge(pcsAgpdItem, &pcsAttachData->m_csChargingInventoryGrid, lLayer, lRow, lColumn);

	if (eResult == AGPDITEM_INSERT_SUCCESS)
	{
		m_pagpmGrid->Clear(&pcsAttachData->m_csChargingInventoryGrid, lPrevLayer, lPrevRow, lPrevColumn, 1, 1);
		m_pagpmGrid->DeleteItem(&pcsAttachData->m_csChargingInventoryGrid, pcsAgpdItem->m_pcsGridItem);

		// Admin Client 를 위해서 Update 되었을 때도 불러준다.
		EnumCallback(ITEM_CB_ID_CHAR_ADD_INVENTORY_FOR_ADMIN, pcsAgpdItem, NULL);

		BOOL	bIsUpdate	= TRUE;

		EnumCallback(ITEM_CB_ID_CHARGING_INVENTORY_ADD, pcsAgpdItem, &bIsUpdate);

		pcsAgpdItem->m_anPrevGridPos	= pcsAgpdItem->m_anGridPos;

		return TRUE;
	}
	else if (eResult == AGPDITEM_INSERT_SUCCESS_STACKCOUNT_ZERO)
	{
		ZeroMemory(pcsAgpdItem->m_szDeleteReason, sizeof(CHAR) * (AGPMITEM_MAX_DELETE_REASON + 1));
		strncpy(pcsAgpdItem->m_szDeleteReason, "SubInventory:stackcount == 0", AGPMITEM_MAX_DELETE_REASON);
		//만들어진 아이템을 없앤다.
		RemoveItem( pcsAgpdItem, TRUE );

		return  TRUE;
	}
	else if (eResult == AGPDITEM_INSERT_NOT_EMPTY)
	{
		AgpdItem	*pcsAgpdExistItem = GetItem(m_pagpmGrid->GetItem(&pcsAttachData->m_csChargingInventoryGrid, lLayer, lRow, lColumn));
		if (!pcsAgpdExistItem)
			return FALSE;

		// 둘중 한넘이 정령석인경우 개조 처리를 한다.
		//		pcsAgpdExistItem 위에 pcsAgpdItem을 얻어놓은 경우이다.
		//		이때 pcsAgpdItem 이 정령석인경우 개조가 가능한지 여부를 판단해 그 뒤 과정을 처리한다.
		//////////////////////////////////////////////////////////////////////////
		EnumCallback(ITEM_CB_ID_REQUEST_SPIRITSTONE_CONVERT, pcsAgpdExistItem, pcsAgpdItem);

		return FALSE;
	}
	else
		return FALSE;

	return TRUE;
}

BOOL AgpmItem::RemoveItemFromChargingInventory(INT32 lCID, AgpdItem *pcsAgpdItem)
{
	return RemoveItemFromChargingInventory(m_pagpmCharacter->GetCharacter(lCID), pcsAgpdItem);
}

BOOL AgpmItem::RemoveItemFromChargingInventory(AgpdCharacter *pcsAgpdCharacter, AgpdItem *pcsAgpdItem)
{
	if (!pcsAgpdCharacter || !pcsAgpdItem)
		return FALSE;

	AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);
	if(!pcsAgpdItemADChar)
	{
		OutputDebugString("AgpmItem::RemoveItemFromInventory() Error (2) !!!\n");
		return FALSE;
	}

	pcsAgpdItem->m_eStatus	= AGPDITEM_STATUS_NONE;

	// 만약 화살류의 아템이라면 카운트를 계산한다.
	if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetArrowTID())
	{
		pcsAgpdItemADChar->m_lNumArrowCount -= pcsAgpdItem->m_nCount;
	}
	else if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetBoltTID())
	{
		pcsAgpdItemADChar->m_lNumBoltCount -= pcsAgpdItem->m_nCount;
	}


	EnumCallback(ITEM_CB_ID_CHARGING_INVENTORY_REMOVE, (PVOID)pcsAgpdItem, NULL);

	return RemoveItemFromGrid(&pcsAgpdItemADChar->m_csChargingInventoryGrid, -1, pcsAgpdCharacter, pcsAgpdItem);
}

INT32 AgpmItem::GetChargingInvectoryLayer(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter) 
		return -1;

	AgpdItemADChar *pcsItemADChar = GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return -1;

	INT32	lInvenLayer = CHARGING_INVENTORY_TYPE_B_LAYER;

	AgpmItemUsableChargingBagType eType = pcsItemADChar->m_eChargingBagType;

	switch(eType)
	{
	case AGPMITEM_USABLE_CHARGING_BAG_TYPE_A:
		{
			lInvenLayer = CHARGING_INVENTORY_TYPE_A_LAYER;
		} break;

	case AGPMITEM_USABLE_CHARGING_BAG_TYPE_B:
		{
			lInvenLayer = CHARGING_INVENTORY_TYPE_B_LAYER;
		} break;
	}

	return lInvenLayer;
}

INT32 AgpmItem::GetChargingInvectoryRow(AgpdCharacter *pcsCharacter)
{
	if(!pcsCharacter)
		return -1;

	AgpdItemADChar *pcsItemADChar = GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return -1;

	INT32	lInvenRow = CHARGING_INVENTORY_TYPE_B_ROW;

	AgpmItemUsableChargingBagType eType = pcsItemADChar->m_eChargingBagType;

	switch(eType)
	{
	case AGPMITEM_USABLE_CHARGING_BAG_TYPE_A:
		{
			lInvenRow = CHARGING_INVENTORY_TYPE_A_ROW;
		} break;

	case AGPMITEM_USABLE_CHARGING_BAG_TYPE_B:
		{
			lInvenRow = CHARGING_INVENTORY_TYPE_B_ROW;
		} break;
	}

	return lInvenRow;
}

INT32 AgpmItem::GetChargingInvectoryColumn( AgpdCharacter *pcsCharacter )
{
	if(!pcsCharacter)
		return -1;

	AgpdItemADChar *pcsItemADChar = GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return -1;

	INT32	lInvenColumn = CHARGING_INVENTORY_TYPE_B_COLUMN;

	AgpmItemUsableChargingBagType eType = pcsItemADChar->m_eChargingBagType;

	switch(eType)
	{
	case AGPMITEM_USABLE_CHARGING_BAG_TYPE_A:
		{
			lInvenColumn = CHARGING_INVENTORY_TYPE_A_COLUMN;
		} break;

	case AGPMITEM_USABLE_CHARGING_BAG_TYPE_B:
		{
			lInvenColumn = CHARGING_INVENTORY_TYPE_B_COLUMN;
		} break;
	}

	return lInvenColumn;
}