//
// AgcmUIChargingInventory.cpp
// 2012.06.28. Bischoff
// Webzen. Archlord Programming Team. Jungyoon Choi
// 

#include "AgcmUIItem.h"
#include "AuXmlParser.h"
#include "AgcChatManager.h"

// 같은 그리드 내에서 이동했음
BOOL AgcmUIItem::CBChargingInventoryAMoveItem(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIItem *pThis = (AgcmUIItem *) pClass;
	AgcdUIControl *pcsControl = (AgcdUIControl *) pData1;
	AgpdGridSelectInfo *	pstGridSelectInfo;
	AgpdGrid *				pstGrid;
	AcUIGrid *				pcsUIGrid;

	BOOL					bResult;

	bResult = FALSE;

	// 현재 개조중이라면 암짓도 하면 안된다.
	if (pThis->m_bIsProcessConvert)
		return TRUE;

	//원래 위치에서 이동된 Layer, Row, Column로 아이템을 옮긴다는 패킷을 보낸다.
	if( !pcsControl || (pcsControl->m_lType != AcUIBase::TYPE_GRID) )
		return bResult;

	pcsUIGrid = (AcUIGrid *) pcsControl->m_pcsBase;

	pstGridSelectInfo = pcsUIGrid->GetDragDropMessageInfo();
	if (!pstGridSelectInfo)
		return bResult;

	pstGrid = pThis->m_pcsAgcmUIManager2->GetControlGrid(pcsControl);

	if (pstGrid)
	{
		INT32			lItemID;
		AgpdItem		*pcsAgpdItem;

		lItemID = pstGridSelectInfo->pGridItem->m_lItemID;

		pcsAgpdItem = pThis->m_pcsAgpmItem->GetItem( lItemID );

		if( pcsAgpdItem != NULL )
		{
			// 인벤에서 확장인벤으로 옮겨지는 경우
			if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_INVENTORY )
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo(pcsAgpdItem->m_lID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn);
			}
			// 서브인벤에서 확장인벤으로 옮겨지는 경우
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_SUB_INVENTORY )
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo(pcsAgpdItem->m_lID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn);
			}
			// 확장인벤에서 확장인벤으로 옮겨지는 경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_CHARGING_INVENTORY )
			{
				// 같은 자리인지 살펴본다.
				// 같은 자리가 아닌경우만 서버로 보낸다.
				if (pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB] == pcsUIGrid->m_lNowLayer &&
					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW] == pstGridSelectInfo->lGridRow &&
					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN] == pstGridSelectInfo->lGridColumn)
				{
					return TRUE;
				}

				// JNY 2005.6.28
				// GetAsyncKeyState는 Release 버젼에서 문제가 생겨서 g_pEngine->IsShiftDown()로 수정
				AgcdUIHotkeyType eHotKeyType;
				if (FALSE == g_pEngine->IsShiftDown())
					eHotKeyType = AGCDUI_HOTKEY_TYPE_NORMAL;
				else
					eHotKeyType = AGCDUI_HOTKEY_TYPE_SHIFT;

				BOOL	bProcess	= FALSE;

				if (eHotKeyType == AGCDUI_HOTKEY_TYPE_SHIFT)
				{
					// 빈자리인지 본다.
					if (pThis->m_pcsAgpmGrid->IsEmptyGrid(pstGrid, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn, 1, 1) &&
						pcsAgpdItem->m_pcsItemTemplate->m_bStackable)
					{
						pThis->m_pcsAgpdSplitItem	= pcsAgpdItem;

						pThis->m_lSplitItemTargetLayer		= pcsUIGrid->m_lNowLayer;
						pThis->m_lSplitItemTargetRow		= pstGridSelectInfo->lGridRow;
						pThis->m_lSplitItemTargetColumn		= pstGridSelectInfo->lGridColumn;

						pThis->m_pcsAgcmUISplitItem->SetCondition(SplitItem_Item, pThis->CBSplitItemInventory, pThis, 1, pcsAgpdItem->m_nCount, 0, 0, AGPDITEM_STATUS_CHARGING_INVENTORY);

						pThis->m_pcsAgcmUISplitItem->OpenSplitItemUI();

						bProcess	= TRUE;
					}
				}

				if (!bProcess)
					bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );
			}
			//Equip에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_EQUIP )
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );
			}
			//Bank에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_BANK )
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );
			}
			//Trade Grid에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_TRADE_GRID )
			{
				bResult = pThis->m_pcsAgcmPrivateTrade->SendTradeRemoveItemFromTradeGrid( pcsAgpdItem->m_ulCID, lItemID, 
																					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB],
																					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW],
																					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN],
																					pcsUIGrid->m_lNowLayer, 
																					pstGridSelectInfo->lGridRow, 
																					pstGridSelectInfo->lGridColumn,
																					AGPDITEM_STATUS_CHARGING_INVENTORY);
																				
			}
			//SalesBox에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_SALESBOX_GRID )
			{
				if( pcsAgpdItem )
				{
					pThis->m_pcsCancelItemInfo = pcsAgpdItem;

					pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_lSalesBoxCancelMessageBox );
				}
			}
			//NPC 거래창에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_NPC_TRADE )
			{
				pThis->m_lNPCTradeBuyItemID = lItemID;
				pThis->m_nNPCTradeBuyStatus = AGPDITEM_STATUS_CHARGING_INVENTORY;

				if (pThis->m_pcsAgpmGrid->IsEmptyGrid(pstGrid, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn, 1, 1))
				{
					pThis->m_lNPCTradeBuyLayer	= pcsUIGrid->m_lNowLayer;
					pThis->m_lNPCTradeBuyRow	= pstGridSelectInfo->lGridRow;
					pThis->m_lNPCTradeBuyColumn	= pstGridSelectInfo->lGridColumn;
				}

				//bResult = pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lNPCTradeMSGConfirmBuy);
				bResult = pThis->CBUINPCTradeMSGConfirmBuy((PVOID) NULL, (PVOID) pThis, (PVOID) NULL);
			}
			// from guild warehouse
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_GUILD_WAREHOUSE)
			{
				INT32 lStatus = (INT32)AGPDITEM_STATUS_CHARGING_INVENTORY;

				// toss to AgcmUIGuildWarehouse
				PVOID pvBuffer[4];
				pvBuffer[0] = &pcsUIGrid->m_lNowLayer;
				pvBuffer[1] = &pstGridSelectInfo->lGridRow;
				pvBuffer[2] = &pstGridSelectInfo->lGridColumn;
				pvBuffer[3] = &lStatus;
				pThis->EnumCallback(AGCMUIITEM_CB_MOVED_FROM_GUILDWAREHOUSE, pcsAgpdItem, pvBuffer);
			}
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_SIEGEWAR_OBJECT)
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );

				pThis->EnumCallback(AGCMUIITEM_CB_REMOVE_SIEGEWAR_GRID, pcsAgpdItem, NULL);
			}
			else if (pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_CASH_INVENTORY)
			{
				AgpdItem	*pcsAgpdExistItem = pThis->m_pcsAgpmItem->GetItem(pThis->m_pcsAgpmGrid->GetItem(pstGrid, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn));
				if (pcsAgpdExistItem)
				{
					if (AGPMITEM_TYPE_USABLE == pcsAgpdItem->m_pcsItemTemplate->m_nType)
					{
						if (AGPMITEM_USABLE_TYPE_RUNE == ((AgpdItemTemplateUsable *) pcsAgpdItem->m_pcsItemTemplate)->m_nUsableItemType)	// 기원석인 경우만 기원석 개조를 위해 허용된다.
							pThis->m_pcsAgcmItemConvert->SendCheckCashRuneConvert(pThis->m_pcsAgcmCharacter->GetSelfCID(), pcsAgpdExistItem->m_lID, lItemID);
						else if (AGPMITEM_USABLE_TYPE_SOCKET_INIT == ((AgpdItemTemplateUsable *) pcsAgpdItem->m_pcsItemTemplate)->m_nUsableItemType)		// 소켓 초기화를 위해 허용
						{
							// check initializable
							if (FALSE == pThis->m_pcsAgpmItemConvert->IsSocketInitializable(pcsAgpdExistItem))
							{
								AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_ITEM_NOT_SOCKET_INITIALIZABLE));
								return FALSE;
							}

							// confirm
							if (IDOK != pThis->m_pcsAgcmUIManager2->ActionMessageOKCancelDialog(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_ITEM_SOCKET_INIT_CONFIRM)))
								return FALSE;
							
							bResult = pThis->m_pcsAgcmItemConvert->SendSocketInitialize(pThis->m_pcsAgcmCharacter->GetSelfCID(), pcsAgpdExistItem->m_lID, lItemID);
						}
					}
				}
			}
		}
	}

	return bResult;
}

BOOL AgcmUIItem::CBChargingInventoryAMoveEndItem(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	BOOL			bResult;

	bResult = FALSE;

	if (!pClass || !pData1)
		return bResult;

	AgcmUIItem			*pThis				= (AgcmUIItem *)	pClass;

	AgcdUIControl		*pcsControl			= (AgcdUIControl *) pData1;

	AcUIGrid			*pcsUIGrid			= (AcUIGrid *) pcsControl->m_pcsBase;
	if (!pcsUIGrid)
		return bResult;

	AgpdGridSelectInfo	*pstGridSelectInfo	= pcsUIGrid->GetGridItemClickInfo();
	if (!pstGridSelectInfo || !pstGridSelectInfo->pGridItem)
		return bResult;

	if (pThis->m_pcsAgcmUIManager2->IsMainWindow(pstGridSelectInfo->pTargetWindow))
	{
		// 오키. 이건 땅바닥에 아템을 버려야 하는 경우이다.
		pThis->m_lInventoryItemID = pstGridSelectInfo->pGridItem->m_lItemID;

		bResult = pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_lInventoryMSGDropItem );
	}

	return bResult;
}

BOOL AgcmUIItem::CBChargingInventoryUIUpdate(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIItem *pThis = (AgcmUIItem *) pClass;
	BOOL			bResult;

	bResult = FALSE;

	if( pThis )
	{
		if( pThis->m_bUIOpenChargingInventoryA || pThis->m_bUIOpenChargingInventoryB)
		{
			pThis->CloseChargingInventory();

			if( pThis->m_bUIOpenNPCTrade )
			{
				bResult = pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_lNPCTradeCloseUI );
				pThis->m_bUIOpenNPCTrade = FALSE;
			}

			// 창이 닫힐 때 보물상자 개방 진행 중이면 취소한다, kelovon, 20050920
			pThis->m_pcsAgcmItem->CancelCoolDown();
		}
		else
		{
			pThis->OpenChargingInventory();
		}
	}

	return bResult;
}

BOOL AgcmUIItem::CBChargingInventoryAOpenInvenLayer(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIItem *		pThis = (AgcmUIItem *) pClass;
	AgcdUIControl *		pcsGridControl = (AgcdUIControl *) pData1;
	AgcdUIUserData *	pstUserData = NULL;

	if (pcsGridControl)
	{
		pstUserData = pThis->m_pcsAgcmUIManager2->GetControlUserData(pcsSourceControl);
		if (pstUserData && pstUserData->m_lSelectedIndex != -1)
			((AcUIGrid *) pcsGridControl->m_pcsBase)->SetNowLayer(pstUserData->m_lSelectedIndex);
	}

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh( pThis->m_pcsChargingInventoryA );
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh( pThis->m_pcsChargingInventoryALayerIndex );

	return TRUE;
}

BOOL AgcmUIItem::CBDragDropItemToChargingInventoryA(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass ||
		!pcsSourceControl ||
		!pcsSourceControl->m_pcsBase)
		return FALSE;

	AgcmUIItem			*pThis				= (AgcmUIItem *)			pClass;
	AcUIButton			*pcsUIButton		= (AcUIButton *)			pcsSourceControl->m_pcsBase;

	AcUIGridItem		*pcsDragDropItem	= pcsUIButton->GetDragDropGridItem();
	if (!pcsDragDropItem || !pcsDragDropItem->m_ppdGridItem)
		return FALSE;

	AgpdGridItem		*pcsGridItem		= pcsDragDropItem->m_ppdGridItem;
	if (pcsGridItem->m_eType != AGPDGRID_ITEM_TYPE_ITEM ||
		pcsGridItem->m_lItemID == AP_INVALID_IID)
		return FALSE;

	INT32				lLayerIndex			= pcsSourceControl->m_lUserDataIndex;

	AgpdItem			*pcsAgpdItem		= pThis->m_pcsAgpmItem->GetItem(pcsGridItem);
	if (pcsAgpdItem &&
		pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_TRADE_GRID)
	{
		return FALSE;
	}

	return pThis->m_pcsAgcmItem->SendChargingInventoryInfo(pcsGridItem->m_lItemID, pThis->m_pcsAgcmCharacter->GetSelfCID(), lLayerIndex);
}

BOOL AgcmUIItem::OpenChargingInventory()
{
	BOOL	bResult = FALSE;

	AgpdCharacter	*pcsSelfCharacter	= m_pcsAgcmCharacter->GetSelfCharacter();
	if( pcsSelfCharacter )
	{
		// Self Character의 스테이트를 확인한다.
		if( pcsSelfCharacter->IsDead() )
		{
			// do nothing..
		}
		else if( m_pcsAgpmItem->IsEnableChargingInventory(pcsSelfCharacter) == FALSE )
		{
			CHAR szErrorMessage[128] = {0,};
			sprintf(szErrorMessage, "%s", m_pcsAgcmUIManager2->GetUIMessage("HaveToBuyChargingBag"));
			g_pEngine->MessageDialog(NULL, szErrorMessage);
		}
		else
		{
			// Enable a charging inventory A one more time
			switch(m_eChargingBagType)
			{
			case AGPMITEM_USABLE_CHARGING_BAG_TYPE_A:
				bResult = m_pcsAgcmUIManager2->ThrowEvent( m_lChargingInventoryAUIOpen );
				m_bUIOpenChargingInventoryA = TRUE;
				EnableChargingInventoryA();
				break;
			case AGPMITEM_USABLE_CHARGING_BAG_TYPE_B:
				bResult = m_pcsAgcmUIManager2->ThrowEvent( m_lChargingInventoryBUIOpen );
				m_bUIOpenChargingInventoryB = TRUE;
				EnableChargingInventoryB();
				break;
			}
		}
	}

	return bResult;
}

BOOL AgcmUIItem::CloseChargingInventory()
{
	m_pcsAgcmUIManager2->ThrowEvent( m_lChargingInventoryAUIClose );
	m_bUIOpenChargingInventoryA = FALSE;
	
	m_pcsAgcmUIManager2->ThrowEvent( m_lChargingInventoryBUIClose );
	m_bUIOpenChargingInventoryB = FALSE;
	
	return TRUE;
}

void AgcmUIItem::EnableChargingInventoryA()
{
	if(!m_pcsUIChargingInventoryA)
		m_pcsUIChargingInventoryA = m_pcsAgcmUIManager2->GetUI("UI_ExpandedInventoryA");

	if(m_pcsUIChargingInventoryA)
	{
		DisableChargingInventory();
		m_pcsUIChargingInventoryA->m_pcsUIWindow->SetActive(TRUE);
	}
}

void AgcmUIItem::DisableChargingInventory()
{
	if(!m_pcsUIChargingInventoryA)
		m_pcsUIChargingInventoryA = m_pcsAgcmUIManager2->GetUI("UI_ExpandedInventoryA");

	if(m_pcsUIChargingInventoryA)
	{
		m_pcsUIChargingInventoryA->m_pcsUIWindow->SetActive(FALSE);
	}

	if(!m_pcsUIChargingInventoryB)
		m_pcsUIChargingInventoryB = m_pcsAgcmUIManager2->GetUI("UI_ExpandedInventoryB");

	if(m_pcsUIChargingInventoryB)
	{
		m_pcsUIChargingInventoryB->m_pcsUIWindow->SetActive(FALSE);
	}
}

// 같은 그리드 내에서 이동했음
BOOL AgcmUIItem::CBChargingInventoryBMoveItem(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIItem *pThis = (AgcmUIItem *) pClass;
	AgcdUIControl *pcsControl = (AgcdUIControl *) pData1;
	AgpdGridSelectInfo *	pstGridSelectInfo;
	AgpdGrid *				pstGrid;
	AcUIGrid *				pcsUIGrid;

	BOOL					bResult;

	bResult = FALSE;

	// 현재 개조중이라면 암짓도 하면 안된다.
	if (pThis->m_bIsProcessConvert)
		return TRUE;

	//원래 위치에서 이동된 Layer, Row, Column로 아이템을 옮긴다는 패킷을 보낸다.
	if( !pcsControl || (pcsControl->m_lType != AcUIBase::TYPE_GRID) )
		return bResult;

	pcsUIGrid = (AcUIGrid *) pcsControl->m_pcsBase;

	pstGridSelectInfo = pcsUIGrid->GetDragDropMessageInfo();
	if (!pstGridSelectInfo)
		return bResult;

	pstGrid = pThis->m_pcsAgcmUIManager2->GetControlGrid(pcsControl);

	if (pstGrid)
	{
		INT32			lItemID;
		AgpdItem		*pcsAgpdItem;

		lItemID = pstGridSelectInfo->pGridItem->m_lItemID;

		pcsAgpdItem = pThis->m_pcsAgpmItem->GetItem( lItemID );

		if( pcsAgpdItem != NULL )
		{
			// 인벤에서 확장인벤으로 옮겨지는 경우
			if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_INVENTORY )
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo(pcsAgpdItem->m_lID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn);
			}
			// 서브인벤에서 확장인벤으로 옮겨지는 경우
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_SUB_INVENTORY )
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo(pcsAgpdItem->m_lID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn);
			}
			// 확장인벤에서 확장인벤으로 옮겨지는 경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_CHARGING_INVENTORY )
			{
				// 같은 자리인지 살펴본다.
				// 같은 자리가 아닌경우만 서버로 보낸다.
				if (pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB] == pcsUIGrid->m_lNowLayer &&
					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW] == pstGridSelectInfo->lGridRow &&
					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN] == pstGridSelectInfo->lGridColumn)
				{
					return TRUE;
				}

				// JNY 2005.6.28
				// GetAsyncKeyState는 Release 버젼에서 문제가 생겨서 g_pEngine->IsShiftDown()로 수정
				AgcdUIHotkeyType eHotKeyType;
				if (FALSE == g_pEngine->IsShiftDown())
					eHotKeyType = AGCDUI_HOTKEY_TYPE_NORMAL;
				else
					eHotKeyType = AGCDUI_HOTKEY_TYPE_SHIFT;

				BOOL	bProcess	= FALSE;

				if (eHotKeyType == AGCDUI_HOTKEY_TYPE_SHIFT)
				{
					// 빈자리인지 본다.
					if (pThis->m_pcsAgpmGrid->IsEmptyGrid(pstGrid, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn, 1, 1) &&
						pcsAgpdItem->m_pcsItemTemplate->m_bStackable)
					{
						pThis->m_pcsAgpdSplitItem	= pcsAgpdItem;

						pThis->m_lSplitItemTargetLayer		= pcsUIGrid->m_lNowLayer;
						pThis->m_lSplitItemTargetRow		= pstGridSelectInfo->lGridRow;
						pThis->m_lSplitItemTargetColumn		= pstGridSelectInfo->lGridColumn;

						pThis->m_pcsAgcmUISplitItem->SetCondition(SplitItem_Item, pThis->CBSplitItemInventory, pThis, 1, pcsAgpdItem->m_nCount, 0, 0, AGPDITEM_STATUS_CHARGING_INVENTORY);

						pThis->m_pcsAgcmUISplitItem->OpenSplitItemUI();

						bProcess	= TRUE;
					}
				}

				if (!bProcess)
					bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );
			}
			//Equip에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_EQUIP )
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );
			}
			//Bank에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_BANK )
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );
			}
			//Trade Grid에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_TRADE_GRID )
			{
				bResult = pThis->m_pcsAgcmPrivateTrade->SendTradeRemoveItemFromTradeGrid( pcsAgpdItem->m_ulCID, lItemID, 
					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB],
					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW],
					pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN],
					pcsUIGrid->m_lNowLayer, 
					pstGridSelectInfo->lGridRow, 
					pstGridSelectInfo->lGridColumn,
					AGPDITEM_STATUS_CHARGING_INVENTORY);

			}
			//SalesBox에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_SALESBOX_GRID )
			{
				if( pcsAgpdItem )
				{
					pThis->m_pcsCancelItemInfo = pcsAgpdItem;

					pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_lSalesBoxCancelMessageBox );
				}
			}
			//NPC 거래창에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_NPC_TRADE )
			{
				pThis->m_lNPCTradeBuyItemID = lItemID;
				pThis->m_nNPCTradeBuyStatus = AGPDITEM_STATUS_CHARGING_INVENTORY;

				if (pThis->m_pcsAgpmGrid->IsEmptyGrid(pstGrid, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn, 1, 1))
				{
					pThis->m_lNPCTradeBuyLayer	= pcsUIGrid->m_lNowLayer;
					pThis->m_lNPCTradeBuyRow	= pstGridSelectInfo->lGridRow;
					pThis->m_lNPCTradeBuyColumn	= pstGridSelectInfo->lGridColumn;
				}

				//bResult = pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lNPCTradeMSGConfirmBuy);
				bResult = pThis->CBUINPCTradeMSGConfirmBuy((PVOID) NULL, (PVOID) pThis, (PVOID) NULL);
			}
			// from guild warehouse
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_GUILD_WAREHOUSE)
			{
				INT32 lStatus = (INT32)AGPDITEM_STATUS_CHARGING_INVENTORY;

				// toss to AgcmUIGuildWarehouse
				PVOID pvBuffer[4];
				pvBuffer[0] = &pcsUIGrid->m_lNowLayer;
				pvBuffer[1] = &pstGridSelectInfo->lGridRow;
				pvBuffer[2] = &pstGridSelectInfo->lGridColumn;
				pvBuffer[3] = &lStatus;
				pThis->EnumCallback(AGCMUIITEM_CB_MOVED_FROM_GUILDWAREHOUSE, pcsAgpdItem, pvBuffer);
			}
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_SIEGEWAR_OBJECT)
			{
				bResult = pThis->m_pcsAgcmItem->SendChargingInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );

				pThis->EnumCallback(AGCMUIITEM_CB_REMOVE_SIEGEWAR_GRID, pcsAgpdItem, NULL);
			}
			else if (pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_CASH_INVENTORY)
			{
				AgpdItem	*pcsAgpdExistItem = pThis->m_pcsAgpmItem->GetItem(pThis->m_pcsAgpmGrid->GetItem(pstGrid, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn));
				if (pcsAgpdExistItem)
				{
					if (AGPMITEM_TYPE_USABLE == pcsAgpdItem->m_pcsItemTemplate->m_nType)
					{
						if (AGPMITEM_USABLE_TYPE_RUNE == ((AgpdItemTemplateUsable *) pcsAgpdItem->m_pcsItemTemplate)->m_nUsableItemType)	// 기원석인 경우만 기원석 개조를 위해 허용된다.
							pThis->m_pcsAgcmItemConvert->SendCheckCashRuneConvert(pThis->m_pcsAgcmCharacter->GetSelfCID(), pcsAgpdExistItem->m_lID, lItemID);
						else if (AGPMITEM_USABLE_TYPE_SOCKET_INIT == ((AgpdItemTemplateUsable *) pcsAgpdItem->m_pcsItemTemplate)->m_nUsableItemType)		// 소켓 초기화를 위해 허용
						{
							// check initializable
							if (FALSE == pThis->m_pcsAgpmItemConvert->IsSocketInitializable(pcsAgpdExistItem))
							{
								AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_ITEM_NOT_SOCKET_INITIALIZABLE));
								return FALSE;
							}

							// confirm
							if (IDOK != pThis->m_pcsAgcmUIManager2->ActionMessageOKCancelDialog(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_ITEM_SOCKET_INIT_CONFIRM)))
								return FALSE;

							bResult = pThis->m_pcsAgcmItemConvert->SendSocketInitialize(pThis->m_pcsAgcmCharacter->GetSelfCID(), pcsAgpdExistItem->m_lID, lItemID);
						}
					}
				}
			}
		}
	}

	return bResult;
}

BOOL AgcmUIItem::CBChargingInventoryBMoveEndItem(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	BOOL			bResult;

	bResult = FALSE;

	if (!pClass || !pData1)
		return bResult;

	AgcmUIItem			*pThis				= (AgcmUIItem *)	pClass;

	AgcdUIControl		*pcsControl			= (AgcdUIControl *) pData1;

	AcUIGrid			*pcsUIGrid			= (AcUIGrid *) pcsControl->m_pcsBase;
	if (!pcsUIGrid)
		return bResult;

	AgpdGridSelectInfo	*pstGridSelectInfo	= pcsUIGrid->GetGridItemClickInfo();
	if (!pstGridSelectInfo || !pstGridSelectInfo->pGridItem)
		return bResult;

	if (pThis->m_pcsAgcmUIManager2->IsMainWindow(pstGridSelectInfo->pTargetWindow))
	{
		// 오키. 이건 땅바닥에 아템을 버려야 하는 경우이다.
		pThis->m_lInventoryItemID = pstGridSelectInfo->pGridItem->m_lItemID;

		bResult = pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_lInventoryMSGDropItem );
	}

	return bResult;
}

BOOL AgcmUIItem::CBChargingInventoryBOpenInvenLayer(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIItem *		pThis = (AgcmUIItem *) pClass;
	AgcdUIControl *		pcsGridControl = (AgcdUIControl *) pData1;
	AgcdUIUserData *	pstUserData = NULL;

	if (pcsGridControl)
	{
		pstUserData = pThis->m_pcsAgcmUIManager2->GetControlUserData(pcsSourceControl);
		if (pstUserData && pstUserData->m_lSelectedIndex != -1)
			((AcUIGrid *) pcsGridControl->m_pcsBase)->SetNowLayer(pstUserData->m_lSelectedIndex);
	}

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh( pThis->m_pcsChargingInventoryB );
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh( pThis->m_pcsChargingInventoryBLayerIndex );

	return TRUE;
}

BOOL AgcmUIItem::CBDragDropItemToChargingInventoryB(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass ||
		!pcsSourceControl ||
		!pcsSourceControl->m_pcsBase)
		return FALSE;

	AgcmUIItem			*pThis				= (AgcmUIItem *)			pClass;
	AcUIButton			*pcsUIButton		= (AcUIButton *)			pcsSourceControl->m_pcsBase;

	AcUIGridItem		*pcsDragDropItem	= pcsUIButton->GetDragDropGridItem();
	if (!pcsDragDropItem || !pcsDragDropItem->m_ppdGridItem)
		return FALSE;

	AgpdGridItem		*pcsGridItem		= pcsDragDropItem->m_ppdGridItem;
	if (pcsGridItem->m_eType != AGPDGRID_ITEM_TYPE_ITEM ||
		pcsGridItem->m_lItemID == AP_INVALID_IID)
		return FALSE;

	INT32				lLayerIndex			= pcsSourceControl->m_lUserDataIndex;

	AgpdItem			*pcsAgpdItem		= pThis->m_pcsAgpmItem->GetItem(pcsGridItem);
	if (pcsAgpdItem &&
		pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_TRADE_GRID)
	{
		return FALSE;
	}

	return pThis->m_pcsAgcmItem->SendChargingInventoryInfo(pcsGridItem->m_lItemID, pThis->m_pcsAgcmCharacter->GetSelfCID(), lLayerIndex);
}

void AgcmUIItem::EnableChargingInventoryB()
{
	if(!m_pcsUIChargingInventoryB)
		m_pcsUIChargingInventoryB = m_pcsAgcmUIManager2->GetUI("UI_ExpandedInventoryB");

	if(m_pcsUIChargingInventoryB)
	{
		DisableChargingInventory();
		m_pcsUIChargingInventoryB->m_pcsUIWindow->SetActive(TRUE);
	}
}