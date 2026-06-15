#include <stdio.h>
#include "AgpmItem.h"
#include "AgpdItem.h"

/******************************************************************************
Module:  AgpmItem.cpp
Notices: Copyright (c) NHN Studio 2002 Ashulam
Purpose: 
Last Update: 2003. 1. 8
******************************************************************************/

#include <stdio.h>
#include "AgpmItem.h"
#include "ApMemoryTracker.h"

AgpdItemGridResult AgpmItem::Insert(AgpdItem *pcsItem, AgpdGrid *pcsGrid, INT32 lLayer, INT32 lRow, INT32 lColumn)
{
	if (!pcsItem || !pcsGrid)
		return AGPDITEM_INSERT_FAIL;

	if (lLayer >= 0 && lRow < 0 && lColumn < 0)
	{
		INT16	nNewRow	= 0;
		INT16	nNewColumn	= 0;

		if (!m_pagpmGrid->AddItem(pcsGrid, pcsItem->m_pcsGridItem))
			return AGPDITEM_INSERT_FAIL;

		if (!m_pagpmGrid->AddToLayer(pcsGrid, lLayer, pcsItem->m_pcsGridItem, 1, 1, &nNewRow, &nNewColumn))
		{
			m_pagpmGrid->DeleteItem(pcsGrid, pcsItem->m_pcsGridItem);
			return AGPDITEM_INSERT_IS_FULL;
		}

		lRow	= nNewRow;
		lColumn	= nNewColumn;
	}
	else
	{
		// 해당 위치가 비어 있는지 본다.
		if (!m_pagpmGrid->IsEmptyGrid(pcsGrid, lLayer, lRow, lColumn, 1, 1))
			return AGPDITEM_INSERT_NOT_EMPTY;

		if (!m_pagpmGrid->AddItem(pcsGrid, pcsItem->m_pcsGridItem))
			return AGPDITEM_INSERT_FAIL;

		if (!m_pagpmGrid->Add(pcsGrid, lLayer, lRow, lColumn, pcsItem->m_pcsGridItem, 1, 1))
			return AGPDITEM_INSERT_FAIL;
	}

	pcsItem->m_anGridPos[AGPDITEM_GRID_POS_TAB]		= lLayer;
	pcsItem->m_anGridPos[AGPDITEM_GRID_POS_ROW]		= lRow;
	pcsItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN]	= lColumn;

	return AGPDITEM_INSERT_SUCCESS;
}

AgpdItemGridResult AgpmItem::InsertStackMerge(AgpdItem *pcsItem, AgpdGrid *pcsGrid, INT32 lLayer, INT32 lRow, INT32 lColumn)
{
	if (!pcsItem || !pcsGrid)
		return AGPDITEM_INSERT_FAIL;

	AgpdItemGridResult	eResult	= Insert(pcsItem, pcsGrid, lLayer, lRow, lColumn);

	if (eResult == AGPDITEM_INSERT_NOT_EMPTY &&
		pcsItem->m_pcsItemTemplate->m_bStackable)
	{
		// 같은 TID를 갖고 있는 놈인지 본다.
		AgpdGridItem	*pcsExistGridItem	= m_pagpmGrid->GetItem(pcsGrid, lLayer, lRow, lColumn);
		//JK_서버크래쉬보완
		if(!pcsExistGridItem || !pcsExistGridItem->GetParentBase())
			return AGPDITEM_INSERT_FAIL;

		AgpdItem* pdItem = (AgpdItem*)pcsExistGridItem->GetParentBase();

		if(NULL == pcsExistGridItem || NULL == pdItem)
			return AGPDITEM_INSERT_FAIL;
		
		// 둘다 퀘스트 아이템이 아닌지 체크
		// 둘다 퀘템인경우엔 머지 가능
		if ( !( IsQuestItem(pdItem) && IsQuestItem(pcsItem) ) )
		{
			// 둘중에 한놈만 퀘템인지 체크
			if (!pcsExistGridItem || !pdItem || IsQuestItem(pdItem) || IsQuestItem(pcsItem))
				return AGPDITEM_INSERT_FAIL;
		}

		if (pcsExistGridItem == pcsItem->m_pcsGridItem)
			return AGPDITEM_INSERT_FAIL;

		if (pcsExistGridItem->m_lItemTID != pcsItem->m_pcsItemTemplate->m_lID)
			return AGPDITEM_INSERT_NOT_EMPTY;

		if (GetBoundType(GetItem(pcsExistGridItem)) != GetBoundType(pcsItem))
			return AGPDITEM_INSERT_NOT_EMPTY;

		// 합쳐서 max stack count 보다 작은지 본다.
		// 크면 실패다.
		AgpdItem	*pcsExistItem	= (AgpdItem *) pcsExistGridItem->GetParentBase();

		if (pcsExistItem->m_nCount + pcsItem->m_nCount > pcsItem->m_pcsItemTemplate->m_lMaxStackableCount)
			return AGPDITEM_INSERT_MAX_STACKCOUNT_OVER;

		AddItemStackCount(pcsExistItem, pcsItem->m_nCount);

		return AGPDITEM_INSERT_SUCCESS_STACKCOUNT_ZERO;
	}

	return eResult;
}

AgpdItemGridResult AgpmItem::AutoInsert(AgpdItem *pcsItem, AgpdGrid *pcsGrid)
{
	if (!pcsItem || !pcsGrid)
		return AGPDITEM_INSERT_FAIL;

	// 비어있는 그리드가 있는지 본다.
	INT16	nLayer	= 0;
	INT16	nRow	= 0;
	INT16	nColumn	= 0;

	if (!m_pagpmGrid->GetEmptyGrid(pcsGrid, &nLayer, &nRow, &nColumn, 1, 1))
		return AGPDITEM_INSERT_IS_FULL;

	if (!m_pagpmGrid->AddItem(pcsGrid, pcsItem->m_pcsGridItem))
		return AGPDITEM_INSERT_FAIL;

	if (!m_pagpmGrid->Add(pcsGrid, nLayer, nRow, nColumn, pcsItem->m_pcsGridItem, 1, 1))
		return AGPDITEM_INSERT_FAIL;

	pcsItem->m_anGridPos[AGPDITEM_GRID_POS_TAB]		= nLayer;
	pcsItem->m_anGridPos[AGPDITEM_GRID_POS_ROW]		= nRow;
	pcsItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN]	= nColumn;

	return AGPDITEM_INSERT_SUCCESS;
}

AgpdItemGridResult AgpmItem::AutoInsertStackMerge(AgpdItem *pcsItem, AgpdGrid *pcsGrid)
{
	if (!pcsItem || !pcsGrid)
		return AGPDITEM_INSERT_FAIL;

	if (!pcsItem->m_pcsItemTemplate->m_bStackable)
		return AutoInsert(pcsItem, pcsGrid);

	// 같은 TID를 갖는 놈들을 모두 가져와서 pcsItem의 스택 갯수를 모두 받아줄 수 있는지 검사한다.
	INT32	lStackBuffer	= 0;
	INT32	lIndex			= 0;

	AgpdItem	*pcsExistItem	= NULL;

	AgpdGridItem	*pcsExistGridItem	= m_pagpmGrid->GetItemByTemplate(lIndex, pcsGrid, pcsItem->m_pcsGridItem->m_eType, pcsItem->m_pcsGridItem->m_lItemTID);
	while (pcsExistGridItem && pcsExistGridItem->GetParentBase() && pcsExistGridItem->GetParentBase()->m_eType == APBASE_TYPE_ITEM)
	{
		pcsExistItem	= (AgpdItem *) pcsExistGridItem->GetParentBase();
		if (!pcsExistItem || !pcsExistItem->m_pcsItemTemplate)
		{
			pcsExistGridItem	= m_pagpmGrid->GetItemByTemplate(lIndex, pcsGrid, pcsItem->m_pcsGridItem->m_eType, pcsItem->m_pcsGridItem->m_lItemTID);
			continue;
		}

		if (GetBoundType(pcsItem) == GetBoundType(pcsExistItem))
			lStackBuffer	+= pcsExistItem->m_pcsItemTemplate->m_lMaxStackableCount - pcsExistItem->m_nCount;

		if (lStackBuffer >= pcsItem->m_nCount)
			break;

		pcsExistGridItem	= m_pagpmGrid->GetItemByTemplate(lIndex, pcsGrid, pcsItem->m_pcsGridItem->m_eType, pcsItem->m_pcsGridItem->m_lItemTID);
	}

	INT16	nLayer	= 0;
	INT16	nRow	= 0;
	INT16	nColumn	= 0;

	if (lStackBuffer < pcsItem->m_nCount &&
		!m_pagpmGrid->GetEmptyGrid(pcsGrid, &nLayer, &nRow, &nColumn, 1, 1))
		return AGPDITEM_INSERT_MAX_STACKCOUNT_OVER;

	// 오키, 다시한번 모두 가져와서,, 스택 카운트를 업데이트 시켜준다.
	lIndex		= 0;
	INT32	lStackCount	= pcsItem->m_nCount;

	pcsExistGridItem	= m_pagpmGrid->GetItemByTemplate(lIndex, pcsGrid, pcsItem->m_pcsGridItem->m_eType, pcsItem->m_pcsGridItem->m_lItemTID);
	while (pcsExistGridItem && pcsExistGridItem->GetParentBase() && pcsExistGridItem->GetParentBase()->m_eType == APBASE_TYPE_ITEM)
	{
		pcsExistItem	= (AgpdItem *) pcsExistGridItem->GetParentBase();
		if (!pcsExistItem || !pcsExistItem->m_pcsItemTemplate)
		{
			pcsExistGridItem	= m_pagpmGrid->GetItemByTemplate(lIndex, pcsGrid, pcsItem->m_pcsGridItem->m_eType, pcsItem->m_pcsGridItem->m_lItemTID);
			continue;
		}

		if( IsQuestItem(pcsExistItem) )
		{
			pcsExistGridItem	= m_pagpmGrid->GetItemByTemplate(lIndex, pcsGrid, pcsItem->m_pcsGridItem->m_eType, pcsItem->m_pcsGridItem->m_lItemTID);
			continue;
		}
		if (GetBoundType(pcsExistItem) == GetBoundType(pcsItem))
		{
			INT32	lOriginalStackCount	= pcsExistItem->m_nCount;

			if (lStackCount > pcsExistItem->m_pcsItemTemplate->m_lMaxStackableCount - pcsExistItem->m_nCount)
				AddItemStackCount(pcsExistItem, pcsExistItem->m_pcsItemTemplate->m_lMaxStackableCount - pcsExistItem->m_nCount);
			else
			{
				AddItemStackCount(pcsExistItem, lStackCount);
				lStackCount	= 0;
				break;
			}

			lStackCount -= pcsExistItem->m_pcsItemTemplate->m_lMaxStackableCount - lOriginalStackCount;
		}

		pcsExistGridItem	= m_pagpmGrid->GetItemByTemplate(lIndex, pcsGrid, pcsItem->m_pcsGridItem->m_eType, pcsItem->m_pcsGridItem->m_lItemTID);
	}

	if (lStackCount > 0)
	{
		SubItemStackCount(pcsItem, pcsItem->m_nCount - lStackCount);
		return AutoInsert(pcsItem, pcsGrid);
	}
	else
	{
		return AGPDITEM_INSERT_SUCCESS_STACKCOUNT_ZERO;
	}

	return AGPDITEM_INSERT_SUCCESS;
}

/*
AgpdItemGridResult AgpmItem::Update(AgpdItem *pcsItem, AgpdGrid *pcsGrid, INT32 lLayer, INT32 lRow, INT32 lColumn)
{
	if (!pcsItem || !pcsGrid)
		return AGPDITEM_UPDATE_FAIL;

	// 해당 위치가 비어 있는지 본다.
	if (!m_pagpmGrid->IsEmptyGrid(pcsGrid, lLayer, lRow, lColumn, 1, 1))
		return AGPDITEM_UPDATE_NOT_EMPTY;

	if (!m_pagpmGrid->AddItem(pcsGrid, pcsItem->m_pcsGridItem))
		return AGPDITEM_UPDATE_FAIL;

	if (!m_pagpmGrid->Add(pcsGrid, lLayer, lRow, lColumn, pcsItem->m_pcsGridItem, 1, 1))
		return AGPDITEM_UPDATE_FAIL;

	return AGPDITEM_UPDATE_SUCCESS;
}

AgpdItemGridResult AgpmItem::UpdateStackMerge(AgpdItem *pcsItem, AgpdGrid *pcsGrid, INT32 lLayer, INT32 lRow, INT32 lColumn)
{
	if (!pcsItem || !pcsGrid)
		return AGPDITEM_UPDATE_FAIL;

	AgpdItemGridResult	eResult	= Update(pcsItem, pcsGrid, lLayer, lRow, lColumn);

	if (eResult == AGPDITEM_UPDATE_NOT_EMPTY &&
		pcsItem->m_pcsItemTemplate->m_bStackable)
	{
		// 같은 TID를 갖고 있는 놈인지 본다.
		AgpdGridItem	*pcsExistGridItem	= m_pagpmGrid->GetItem(pcsGrid, lLayer, lRow, lColumn);
		if (!pcsExistGridItem || !pcsExistGridItem->GetParentBase())
			return AGPDITEM_UPDATE_FAIL;

		if (pcsExistGridItem == pcsItem->m_pcsGridItem)
			return AGPDITEM_UPDATE_FAIL;

		if (pcsExistGridItem->m_lItemTID != pcsItem->m_pcsItemTemplate->m_lID)
			return AGPDITEM_UPDATE_NOT_EMPTY;

		// 합쳐서 max stack count 보다 작은지 본다.
		// 크면 실패다.
		AgpdItem	*pcsExistItem	= (AgpdItem *) pcsExistGridItem->GetParentBase();

		if (pcsExistItem->m_nCount + pcsItem->m_nCount > pcsItem->m_pcsItemTemplate->m_lMaxStackableCount)
			return AGPDITEM_INSERT_MAX_STACKCOUNT_OVER;

		AddItemStackCount(pcsExistItem, pcsItem->m_nCount);

		return AGPDITEM_INSERT_SUCCESS_STACKCOUNT_ZERO;
	}

	return eResult;
}
*/


/*
BOOL AgpmItem::AddItemToGrid(AgpdGrid *pcsGrid, INT32 lCheckAddCB, INT32 lUpdateCB, INT32 lItemStatus, AgpdCharacter *pcsAgpdCharacter, AgpdItem *pcsAgpdItem, INT16 *pnGridIdx, INT16 *pnGridRow, INT16 *pnGridCol )
{
	BOOL				bAutoInsert;

	if (!pcsAgpdCharacter || !pcsAgpdItem)
		return FALSE;

	//전부 NULL이면 AutoInsert이다.
	if( !pnGridIdx && !pnGridRow && !pnGridCol )
	{
		bAutoInsert = TRUE;
	}
	//전부 NULL이 아니면 정해진 위치에 넣는다.
	else if( pnGridIdx && pnGridRow && pnGridCol )
	{
		if (*pnGridIdx == (-1) && *pnGridRow == (-1) && *pnGridCol == (-1))
			bAutoInsert	= TRUE;
		else
			bAutoInsert = FALSE;
	}
	//그것도 아니라.... 이건 메모리뻑이군..
	else
	{
		return FALSE;
	}


	// 현재 Grid에 Item을 넣을 수 있는지 확인
	if (lCheckAddCB != -1 && !EnumCallback(lCheckAddCB, pcsAgpdItem, pcsAgpdCharacter))
	{
		OutputDebugString("AgpmItem::AddItemToGrid() Error (11) !!!\n");
		return FALSE;
	}

	AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);
	if(!pcsAgpdItemADChar)
	{
		OutputDebugString("AgpmItem::AddItemToGrid() Error (2) !!!\n");
		return FALSE;
	}

	if( m_pagpmGrid->IsExistItem(pcsGrid, AGPDGRID_ITEM_TYPE_ITEM, pcsAgpdItem->m_lID ) )
	{ // 이미 뱅크내에 이 아템이 존재한다.
		OutputDebugString("AgpmItem::AddItemToGrid() Error (4) !!!\n");
		return FALSE;
	}

	AgpdItemTemplate *pTemplate	= (AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate;

	//존재하는 템플릿인가?
	if( pTemplate == NULL )
	{
		return FALSE;
	}

	BOOL		bResult;

	bResult = m_pagpmGrid->AddItem(pcsGrid, pcsAgpdItem->m_pcsGridItem );

	if( bResult == TRUE )
	{
		INT16			nWidth, nHeight;
		INT16			nLayer, nRow, nColumn;

		nWidth = pTemplate->m_nSizeInInventory[AGPDITEMTEMPLATESIZE_WIDTH];
		nHeight = pTemplate->m_nSizeInInventory[AGPDITEMTEMPLATESIZE_HEIGHT];

		if( bAutoInsert == FALSE )
		{
			if( !m_pagpmGrid->Add(pcsGrid, *pnGridIdx, *pnGridRow, *pnGridCol, pcsAgpdItem->m_pcsGridItem, nWidth, nHeight ) )
			{
				return FALSE;
			}

			nLayer = *pnGridIdx;
			nRow = *pnGridRow;
			nColumn = *pnGridCol;
		}
		else //if( bAutoInsert == TRUE )
		{
			if( !m_pagpmGrid->Add(pcsGrid, 0, 0, 0, &nLayer, &nRow, &nColumn, pcsAgpdItem->m_pcsGridItem, nWidth, nHeight ) )
			{
				return FALSE;
			}
		}

		//여기까지 왔으면 잘 넣었다는 이야기이다. 아이템 상태를 set한다.
		pcsAgpdItem->m_eStatus = lItemStatus;
		// 아이템의 뱅크 위치 정보를 기록한다.
		pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB]    = nLayer;
		pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW]    = nRow;
		pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN] = nColumn;

		if (pcsAgpdCharacter)
		{
			ChangeItemOwner(pcsAgpdItem, pcsAgpdCharacter);
		}

		if (lUpdateCB != -1)
			EnumCallback(lUpdateCB, pcsAgpdItem, NULL);
	}

	return bResult;
}
*/

BOOL AgpmItem::RemoveItemFromGrid(AgpdGrid *pcsGrid, INT32 lCheckRemoveCB, AgpdCharacter *pcsAgpdCharacter, AgpdItem *pcsAgpdItem)
{
	if (!pcsAgpdCharacter || !pcsAgpdItem)
		return FALSE;

	// 현재 Grid에 Item을 뺄 수 있는지 확인
	if (lCheckRemoveCB != -1 && !EnumCallback(lCheckRemoveCB, pcsAgpdItem, pcsAgpdCharacter))
	{
		OutputDebugString("AgpmItem::AddItemToGrid() Error (11) !!!\n");
		return FALSE;
	}

	AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);
	if(!pcsAgpdItemADChar)
	{
		OutputDebugString("AgpmItem::RemoveItemFromGrid() Error (2) !!!\n");
		return FALSE;
	}

	if(!pcsAgpdItem)
	{
		OutputDebugString("AgpmItem::RemoveItemFromGrid() Error (3) !!!\n");
		return FALSE;
	}

	if( m_pagpmGrid->IsExistItem(pcsGrid, AGPDGRID_ITEM_TYPE_ITEM, pcsAgpdItem->m_lID ) == FALSE )
	{ 
		OutputDebugString("AgpmItem::RemoveItemFromGrid() Error (4) !!!\n");
		return FALSE; // 뱅크에 없는 넘이다. 삭제 할 수 없당. 개념 없는 놈이군.
	}

	BOOL			bResult;

	bResult = m_pagpmGrid->DeleteItem(pcsGrid, pcsAgpdItem->m_pcsGridItem );

	if( bResult == TRUE )
	{
		AgpdItemTemplate *pTemplate	= (AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate;

		//존재하는 템플릿인가?
		if( pTemplate == NULL )
		{
			return FALSE;
		}

		INT16			nWidth, nHeight;

		nWidth = pTemplate->m_nSizeInInventory[AGPDITEMTEMPLATESIZE_WIDTH];
		nHeight = pTemplate->m_nSizeInInventory[AGPDITEMTEMPLATESIZE_HEIGHT];

		bResult = m_pagpmGrid->Clear(pcsGrid,
												pcsAgpdItem->m_anPrevGridPos[AGPDITEM_GRID_POS_TAB],
												pcsAgpdItem->m_anPrevGridPos[AGPDITEM_GRID_POS_ROW],
												pcsAgpdItem->m_anPrevGridPos[AGPDITEM_GRID_POS_COLUMN],
												nWidth,
												nHeight );

		if( bResult == TRUE )
		{
			pcsAgpdItem->m_eStatus = AGPDITEM_STATUS_NONE;
		}
	}

	return bResult;
}

/*
BOOL AgpmItem::UpdateGrid(AgpdGrid *pcsGrid, INT32 lCheckAddCB, INT32 lUpdateCB, INT32 lItemStatus, AgpdCharacter *pcsAgpdCharacter, AgpdItem *pcsAgpdItem, INT16 nLayer, INT16 nRow, INT16 nColumn)
{
	if (!pcsAgpdCharacter || !pcsAgpdItem)
		return FALSE;

	AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);
	if(!pcsAgpdItemADChar)
	{
		OutputDebugString("AgpmItem::UpdateGrid() Error (2) !!!\n");
		return FALSE;
	}

	if(!pcsAgpdItem)
	{
		OutputDebugString("AgpmItem::UpdateGrid() Error (3) !!!\n");
		return FALSE;
	}

	AgpdItemTemplate *pTemplate	= (AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate;

	//존재하는 템플릿인가?
	if( pTemplate == NULL )
	{
		return FALSE;
	}

	BOOL			bResult;
	BOOL			bExist;
	INT16			nOriginLayer, nOriginRow, nOriginColumn;
	AgpdGridItem *	pcsGridItem;

	bResult = FALSE;
	bExist = FALSE;
	nOriginLayer = pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB];
	nOriginRow = pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW];
	nOriginColumn = pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN];

	if (nLayer == (-1))
	{
		//그냥 Relase시킨다.
		ReleaseItem( pcsAgpdItem, (AgpdItemStatus) lItemStatus );

		return AddItemToGrid( pcsGrid, lCheckAddCB, lUpdateCB, lItemStatus, pcsAgpdCharacter, pcsAgpdItem, NULL, NULL, NULL );
	}
	else if (nRow == (-1) && nColumn == (-1))
		return MoveGridLayer(pcsGrid, lCheckAddCB, lUpdateCB, lItemStatus, pcsAgpdCharacter, pcsAgpdItem, nLayer);

	pcsGridItem = m_pagpmGrid->GetItem(pcsGrid, nLayer, nRow, nColumn );

	if( pcsGridItem != NULL)
	{
		bExist = TRUE;
	}

	//비어있는경우
	if( bExist == FALSE )
	{
		//그냥 Relase시킨다.
		ReleaseItem( pcsAgpdItem, (AgpdItemStatus) lItemStatus );

		bResult = AddItemToGrid( pcsGrid, lCheckAddCB, lUpdateCB, lItemStatus, pcsAgpdCharacter, pcsAgpdItem, &nLayer, &nRow, &nColumn );

		if (bResult &&
			pcsAgpdItem->m_pcsCharacter &&
			pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_INVENTORY)
		{
			// 만약 화살류의 아템이라면 카운트를 계산한다.
			if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetArrowTID())
			{
				AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsAgpdItem->m_pcsCharacter);

				pcsAgpdItemADChar->m_lNumArrowCount += pcsAgpdItem->m_nCount;
			}
			else if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetBoltTID())
			{
				AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsAgpdItem->m_pcsCharacter);

				pcsAgpdItemADChar->m_lNumBoltCount += pcsAgpdItem->m_nCount;
			}
		}
	}
	//뭔가 있는 경우. 같은 TID인지 확인한다.
	else //if( bExist == TRUE )
	{
		// 이미 들어있는 넘이 똑같은 넘인지 본다.
		// 똑같은 놈이라면 암짓도 하면 안된다. (by netong)
		if (pcsGridItem->m_lItemID == pcsAgpdItem->m_lID)
			return TRUE;

		if( pcsGridItem->m_lItemTID == pcsAgpdItem->m_lTID )
		{
			AgpdItemTemplate	*pExistTemplate;
			AgpdItem			*pcsAgpdExistItem;

			//아이템 포인터를 얻어낸다.
			pcsAgpdExistItem = GetItem( pcsGridItem );
			
			//템플릿을 얻어낸다.
			pExistTemplate = NULL;
			if (pcsAgpdExistItem)
				pExistTemplate	= (AgpdItemTemplate *) pcsAgpdExistItem->m_pcsItemTemplate;

			if( (pExistTemplate != NULL) && (pcsAgpdExistItem != NULL) )
			{
				//스태커블 아이템인지 확인한다.
				if( CheckJoinItem(pcsAgpdExistItem, pcsAgpdItem) )
				//if( pExistTemplate->m_bStackable )
				{
					//두 스택아이템을 합했을때!! 스택제한값을 넘기지 않은경우~
					if( (pcsAgpdItem->m_nCount + pcsAgpdExistItem->m_nCount) <= pExistTemplate->m_lMaxStackableCount )
					{
						AddItemStackCount(pcsAgpdExistItem, pcsAgpdItem->m_nCount);

						ZeroMemory(pcsAgpdItem->m_szDeleteReason, sizeof(CHAR) * (AGPMITEM_MAX_DELETE_REASON + 1));
						strncpy(pcsAgpdItem->m_szDeleteReason, "Grid:다른아템과합체", AGPMITEM_MAX_DELETE_REASON);
						//클라이언트에 AgpdItem을 지우고.
						RemoveItem( pcsAgpdItem->m_lID, TRUE );	//RemoveItem안에서 SendRemovePacket를 보냄.
															//RemoveItem에서 ReleaseItem을 함.
						
						//AgpdExistItem을 update한다.
						//EnumCallback(lUpdateCB, pcsAgpdExistItem, NULL); //Add에서 Update를 호출한다.

						bResult = TRUE;
					}
					else
					{
						if (m_pagpmGrid->IsFullGrid(pcsGrid))
							return FALSE;

						//pcsAgpdItem은 OnOperationUpdate에서 Release된 상태에서 들어왔기 때문에 다시 AddItemToGrid가 호출되어야한다.
						if (lItemStatus != pcsAgpdItem->m_eStatus)
							if (!AddItemToGrid(pcsGrid, lCheckAddCB, lUpdateCB, lItemStatus, pcsAgpdCharacter, pcsAgpdItem, NULL, NULL, NULL))
								return FALSE;

						{
							SetItemStackCount(pcsAgpdItem, (pcsAgpdItem->m_nCount + pcsAgpdExistItem->m_nCount) - pExistTemplate->m_lMaxStackableCount);
							SetItemStackCount(pcsAgpdExistItem, pExistTemplate->m_lMaxStackableCount);

							//클라이언트에 AgpdItem, AgpdExistItem을 모두 update한다.
							//EnumCallback(lUpdateCB, pcsAgpdItem, NULL); //Add에서 Update를 호출한다.
							//EnumCallback(lUpdateCB, pcsAgpdExistItem, NULL); //Add에서 Update를 호출한다.

							bResult = TRUE;
						}
					}
				}
				else
				{
					if (lItemStatus != pcsAgpdItem->m_eStatus)
						if (AddItemToGrid( pcsGrid, lCheckAddCB, lUpdateCB, lItemStatus, pcsAgpdCharacter, pcsAgpdItem, NULL, NULL, NULL ) )
							bResult	= TRUE;
				}
			}
		}

		else
		{
			//	added by netong
			//////////////////////////////////////////////////////////////////////////

			// 2개의 아이템이 다른 종류이다.

			AgpdItem	*pcsAgpdExistItem = GetItem( pcsGridItem );
			if (!pcsAgpdExistItem)
				return FALSE;

			// 개조는 Inventory 내에서만 가능하다. 둘다 Inventory 안에 있는지 본다.
			if (pcsAgpdExistItem->m_eStatus != AGPDITEM_STATUS_INVENTORY ||
				pcsAgpdItem->m_eStatus != AGPDITEM_STATUS_INVENTORY)
				return FALSE;

			// 둘중 한넘이 정령석인경우 개조 처리를 한다.
			//		pcsAgpdExistItem 위에 pcsAgpdItem을 얻어놓은 경우이다.
			//		이때 pcsAgpdItem 이 정령석인경우 개조가 가능한지 여부를 판단해 그 뒤 과정을 처리한다.
			//////////////////////////////////////////////////////////////////////////
			EnumCallback(ITEM_CB_ID_REQUEST_SPIRITSTONE_CONVERT, pcsAgpdExistItem, pcsAgpdItem);
			//bResult = CheckConvertRequest(pcsAgpdExistItem, pcsAgpdItem);

			// bResult가 TRUE 인 경우 pcsAgpdExistItem, pcsAgpdItem의 위치를 바꿔선 안된다.
			// 기존 위치를 그대로 유지시킨다.
			if (bResult)
			{

			}

			//	added by netong
			//////////////////////////////////////////////////////////////////////////
		}
	}

	return bResult;
}

BOOL AgpmItem::MoveGridLayer(AgpdGrid *pcsGrid, INT32 lCheckAddCB, INT32 lUpdateCB, INT32 lItemStatus, AgpdCharacter *pcsCharacter, AgpdItem *pcsAgpdItem, INT16 nLayer)
{
	if (!pcsGrid || !pcsCharacter || !pcsAgpdItem)
		return FALSE;

	// 현재 Grid에 Item을 넣을 수 있는지 확인
	if (lCheckAddCB != -1 && !EnumCallback(lCheckAddCB, pcsAgpdItem, pcsCharacter))
	{
		OutputDebugString("AgpmItem::MoveGridLayer() Error (11) !!!\n");
		return FALSE;
	}

	INT16	nNewRow		= 0;
	INT16	nNewColumn	= 0;

	// 한번 넣어보고 실제 들어가면 그때 제대로 넣는다.
	if (m_pagpmGrid->AddItem(pcsGrid, pcsAgpdItem->m_pcsGridItem))
	{
		if (m_pagpmGrid->AddToLayer(pcsGrid, nLayer, pcsAgpdItem->m_pcsGridItem, 1, 1, &nNewRow, &nNewColumn))
		{
			m_pagpmGrid->Clear(pcsGrid, nLayer, nNewRow, nNewColumn, 1, 1);
			m_pagpmGrid->DeleteItem(pcsGrid, pcsAgpdItem->m_pcsGridItem);

			ReleaseItem( pcsAgpdItem );

			m_pagpmGrid->AddItem(pcsGrid, pcsAgpdItem->m_pcsGridItem);
			m_pagpmGrid->AddToLayer(pcsGrid, nLayer, pcsAgpdItem->m_pcsGridItem, 1, 1, &nNewRow, &nNewColumn);

			pcsAgpdItem->m_eStatus	= lItemStatus;

			pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB]    = nLayer;
			pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW]    = nNewRow;
			pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN] = nNewColumn;

			if (pcsAgpdItem->m_pcsCharacter &&
				pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_INVENTORY)
			{
				// 만약 화살류의 아템이라면 카운트를 계산한다.
				if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetArrowTID())
				{
					AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsAgpdItem->m_pcsCharacter);

					pcsAgpdItemADChar->m_lNumArrowCount += pcsAgpdItem->m_nCount;
				}
				else if (((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID == GetBoltTID())
				{
					AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsAgpdItem->m_pcsCharacter);

					pcsAgpdItemADChar->m_lNumBoltCount += pcsAgpdItem->m_nCount;
				}
			}

			EnumCallback(lUpdateCB, pcsAgpdItem, NULL);

			return TRUE;
		}
		else
			m_pagpmGrid->DeleteItem(pcsGrid, pcsAgpdItem->m_pcsGridItem);
	}

	return FALSE;
}
*/

BOOL AgpmItem::LockGrid(AgpdCharacter *pcsCharacter, AgpdItemStatus eLockGrid, INT16 *pnLayer, INT16 *pnRow, INT16 *pnColumn)
{
	if (!pcsCharacter || !pnLayer || !pnRow || !pnColumn)
		return FALSE;

	BOOL	bSearchEmptyGrid	= FALSE;

	if (*pnLayer < 0 || *pnRow < 0 || *pnColumn < 0)
		bSearchEmptyGrid	= TRUE;

	AgpdGrid	*pcsGrid	= NULL;

	switch (eLockGrid) {
	case AGPDITEM_STATUS_INVENTORY:
		pcsGrid	= GetInventory(pcsCharacter);
		break;

	case AGPDITEM_STATUS_TRADE_GRID:
		pcsGrid	= GetTradeGrid(pcsCharacter);
		break;

	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
		pcsGrid	= GetClientTradeGrid(pcsCharacter);
		break;

	case AGPDITEM_STATUS_SALESBOX_GRID:
		pcsGrid	= GetSalesBox(pcsCharacter);
		break;

	case AGPDITEM_STATUS_BANK:
		pcsGrid	= GetBank(pcsCharacter);
		break;
	}

	if (!pcsGrid)
		return FALSE;

	if (!m_pagpmGrid->GetEmptyGridWithStartPos(pcsGrid, 0, 0, 0, pnLayer, pnRow, pnColumn, 1, 1))
		return FALSE;

	return m_pagpmGrid->LockItem(pcsGrid, *pnLayer, *pnRow, *pnColumn, 1, 1);
}

BOOL AgpmItem::ReleaseGrid(AgpdCharacter *pcsCharacter, AgpdItemStatus eLockGrid, INT16 nLayer, INT16 nRow, INT16 nColumn)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdGrid	*pcsGrid	= NULL;

	switch (eLockGrid) {
	case AGPDITEM_STATUS_INVENTORY:
		pcsGrid	= GetInventory(pcsCharacter);
		break;

	case AGPDITEM_STATUS_TRADE_GRID:
		pcsGrid	= GetTradeGrid(pcsCharacter);
		break;

	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
		pcsGrid	= GetClientTradeGrid(pcsCharacter);
		break;

	case AGPDITEM_STATUS_SALESBOX_GRID:
		pcsGrid	= GetSalesBox(pcsCharacter);
		break;

	case AGPDITEM_STATUS_BANK:
		pcsGrid	= GetBank(pcsCharacter);
		break;
	}

	if (!pcsGrid)
		return FALSE;

	return m_pagpmGrid->ReleaseItem(pcsGrid, nLayer, nRow, nColumn, 1, 1);
}