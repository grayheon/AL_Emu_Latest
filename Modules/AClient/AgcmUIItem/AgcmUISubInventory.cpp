//
// AgcmUISubInventory.cpp
// 2008.05.14. steeple
// NHN Games. Archlord Programming Team. TaeHee, Kim
// 

#include "AgcmUIItem.h"
#include "AuXmlParser.h"
#include "AgcChatManager.h"

void stWorldAutoPickUpInfo::AddCharacter( char* pCharacterName, BOOL bAuto )
{
	if( !pCharacterName || strlen( pCharacterName ) <= 0 ) return;

	stCharacterAutoPickUpInfo* pCharacterpInfo = m_mapCharacters.Get( pCharacterName );
	if( !pCharacterpInfo )
	{
		stCharacterAutoPickUpInfo NewInfo;
		NewInfo.m_strCharacterName = pCharacterName;
		NewInfo.m_bAuto = bAuto;
		m_mapCharacters.Add( NewInfo.m_strCharacterName, NewInfo );

#ifdef _DEBUG
		char strDebug[ 256 ] = { 0, };
		sprintf( strDebug, "Add Character to AutoPickUp = %s, Auto = %s\n", pCharacterName, bAuto ? "true" : "false" );
		OutputDebugString( strDebug );
#endif
	}
}

void stWorldAutoPickUpInfo::UseCharacter( char* pCharacterName )
{
	if( !pCharacterName || strlen( pCharacterName ) <= 0 ) return;

	stCharacterAutoPickUpInfo* pCharacterpInfo = m_mapCharacters.Get( pCharacterName );
	if( pCharacterpInfo )
	{
		m_strCurrCharacterName = pCharacterName;

#ifdef _DEBUG
		char strDebug[ 256 ] = { 0, };
		sprintf( strDebug, "Use Character to AutoPickUp = %s\n", pCharacterName );
		OutputDebugString( strDebug );
#endif
	}
}

void stWorldAutoPickUpInfo::DeleteCharacter( char* pCharacterName )
{
	if( !pCharacterName || strlen( pCharacterName ) <= 0 ) return;
	m_mapCharacters.Delete( pCharacterName );

#ifdef _DEBUG
		char strDebug[ 256 ] = { 0, };
		sprintf( strDebug, "Delete Character to AutoPickUp = %s\n", pCharacterName );
		OutputDebugString( strDebug );
#endif
}

stCharacterAutoPickUpInfo* stWorldAutoPickUpInfo::GetCharacter( char* pCharacterName )
{
	if( !pCharacterName || strlen( pCharacterName ) <= 0 )
	{
		return m_mapCharacters.Get( m_strCurrCharacterName );
	}

	return m_mapCharacters.Get( pCharacterName );
}

void stAutoPickUpSettings::LoadFromFile( char* pFileName )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return;

	TiXmlDocument Doc;
	if( !Doc.LoadFile( pFileName ) ) return;

	TiXmlNode* pNodeAutoPickUp = Doc.FirstChild( "AutoPickUp" );
	if( !pNodeAutoPickUp ) return;
	
	TiXmlNode* pNodeWorld = pNodeAutoPickUp->FirstChild( "World" );
	while( pNodeWorld )
	{
		stWorldAutoPickUpInfo NewWorld;
		NewWorld.m_strWorldName = pNodeWorld->ToElement()->Attribute( "Name" );
		
		TiXmlNode* pNodeCharacter = pNodeWorld->FirstChild( "Character" );
		while( pNodeCharacter )
		{
			stCharacterAutoPickUpInfo NewCharacter;

			NewCharacter.m_strCharacterName = pNodeCharacter->ToElement()->Attribute( "Name" );
			NewCharacter.m_bAuto = atoi( pNodeCharacter->ToElement()->Attribute( "IsAuto" ) );

			NewWorld.m_mapCharacters.Add( NewCharacter.m_strCharacterName, NewCharacter );
			pNodeCharacter = pNodeCharacter->NextSibling();
		}

		m_mapWorlds.Add( NewWorld.m_strWorldName, NewWorld );
		pNodeWorld = pNodeWorld->NextSibling();
	}
}

void stAutoPickUpSettings::SaveToFile( char* pFileName )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return;

	// 해당파일이 있든없든 일단 지워버린다. 어차피 새로 만들어야 함..
	::DeleteFile( pFileName );

	TiXmlDocument Doc;

	TiXmlElement ElementAutoPickUp( "AutoPickUp" );
	Doc.InsertEndChild( ElementAutoPickUp );

	TiXmlNode* pNodeAutoPickUp = Doc.FirstChild( "AutoPickUp" );
	if( !pNodeAutoPickUp ) return;

	int nWorldCount = m_mapWorlds.GetSize();
	for( int nCount = 0 ; nCount < nWorldCount ; nCount++ )
	{
		stWorldAutoPickUpInfo* pWorldInfo = m_mapWorlds.GetByIndex( nCount );
		if( pWorldInfo )
		{
			TiXmlElement ElementWorld( "World" );
			ElementWorld.SetAttribute( "Name", pWorldInfo->m_strWorldName.c_str() );

			int nCharacterCount = pWorldInfo->m_mapCharacters.GetSize();
			for( int nCountCharacter = 0 ; nCountCharacter < nCharacterCount ; nCountCharacter++ )
			{
				stCharacterAutoPickUpInfo* pCharacterInfo = pWorldInfo->m_mapCharacters.GetByIndex( nCountCharacter );
				if( pCharacterInfo )
				{
					TiXmlElement ElementCharacter( "Character" );
					ElementCharacter.SetAttribute( "Name", pCharacterInfo->m_strCharacterName.c_str() );
					ElementCharacter.SetAttribute( "IsAuto", pCharacterInfo->m_bAuto );
					ElementWorld.InsertEndChild( ElementCharacter );
				}
			}

			pNodeAutoPickUp->ToElement()->InsertEndChild( ElementWorld );
		}
	}

	Doc.SaveFile( pFileName );
}

void stAutoPickUpSettings::AddWorld( char* pWorldName )
{
	stWorldAutoPickUpInfo* pWorldInfo = m_mapWorlds.Get( pWorldName );
	if( !pWorldInfo )
	{
		stWorldAutoPickUpInfo NewInfo;
		NewInfo.m_strWorldName = pWorldName;
		m_mapWorlds.Add( NewInfo.m_strWorldName, NewInfo );

#ifdef _DEBUG
		char strDebug[ 256 ] = { 0, };
		sprintf( strDebug, "Add World to AutoPickUp = %s\n", pWorldName );
		OutputDebugString( strDebug );
#endif
	}
}

void stAutoPickUpSettings::UseWorld( char* pWorldName )
{
	stWorldAutoPickUpInfo* pWorldInfo = m_mapWorlds.Get( pWorldName );
	if( pWorldInfo )
	{
		m_strCurrWorldName = pWorldName;

#ifdef _DEBUG
		char strDebug[ 256 ] = { 0, };
		sprintf( strDebug, "Use World to AutoPickUp = %s\n", pWorldName );
		OutputDebugString( strDebug );
#endif
	}
}

void stAutoPickUpSettings::DeleteWorld( char* pWorldName )
{
	m_mapWorlds.Delete( pWorldName );

#ifdef _DEBUG
		char strDebug[ 256 ] = { 0, };
		sprintf( strDebug, "Delete World to AutoPickUp = %s\n", pWorldName );
		OutputDebugString( strDebug );
#endif
}

stWorldAutoPickUpInfo* stAutoPickUpSettings::GetWorld( char* pWorldName )
{
	if( !pWorldName || strlen( pWorldName ) <= 0 )
	{
		return m_mapWorlds.Get( m_strCurrWorldName );
	}
	else
	{
		return m_mapWorlds.Get( pWorldName );
	}

	return NULL;
}

void stAutoPickUpSettings::AddCharacter( char* pCharacterName, BOOL bAuto )
{
	stWorldAutoPickUpInfo* pWorld = GetWorld();
	if( !pWorld ) return;
	pWorld->AddCharacter( pCharacterName, bAuto );
}

void stAutoPickUpSettings::UseCharacter( char* pCharacterName )
{
	stWorldAutoPickUpInfo* pWorld = GetWorld();
	if( !pWorld ) return;
	pWorld->UseCharacter( pCharacterName );
}

void stAutoPickUpSettings::DeleteCharacter( char* pCharacterName )
{
	stWorldAutoPickUpInfo* pWorld = GetWorld();
	if( !pWorld ) return;
	pWorld->DeleteCharacter( pCharacterName );
}

BOOL stAutoPickUpSettings::IsAutoPickUp( void )
{
	stWorldAutoPickUpInfo* pWorld = GetWorld();
	if( !pWorld ) return TRUE;

	stCharacterAutoPickUpInfo* pCharacter = pWorld->GetCharacter();
	if( !pCharacter ) return TRUE;

	return pCharacter->m_bAuto;
}

BOOL stAutoPickUpSettings::ToggleAutoPickUp( void )
{
	stWorldAutoPickUpInfo* pWorld = GetWorld();
	if( !pWorld ) return TRUE;

	stCharacterAutoPickUpInfo* pCharacter = pWorld->GetCharacter();
	if( !pCharacter ) return TRUE;

	pCharacter->m_bAuto = !pCharacter->m_bAuto;
	return pCharacter->m_bAuto;
}














// 같은 그리드 내에서 이동했음
BOOL AgcmUIItem::CBSubInventoryMoveItem(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
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
			//필드에서 인벤으로 옮겨진경우.
			if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_FIELD )
			{
				bResult = pThis->m_pcsAgcmItem->PickupItem(pcsAgpdItem, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn);

				if( bResult )
				{
					bResult = pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lInventoryMSGGetItem);
				}
			}
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_INVENTORY )
			{
				bResult = pThis->m_pcsAgcmItem->SendSubInventoryInfo(pcsAgpdItem->m_lID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn);
			}
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_CHARGING_INVENTORY )
			{
				bResult = pThis->m_pcsAgcmItem->SendSubInventoryInfo(pcsAgpdItem->m_lID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn);
			}
			// 서브인벤에서 서브인벤으로 옮겨지는 경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_SUB_INVENTORY )
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

						pThis->m_pcsAgcmUISplitItem->SetCondition(SplitItem_Item, pThis->CBSplitItemInventory, pThis, 1, pcsAgpdItem->m_nCount, 0, 0, AGPDITEM_STATUS_SUB_INVENTORY);

						pThis->m_pcsAgcmUISplitItem->OpenSplitItemUI();

						bProcess	= TRUE;
					}
				}

				if (!bProcess)
					bResult = pThis->m_pcsAgcmItem->SendSubInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );
			}
			//Equip에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_EQUIP )
			{
				// 벗을 수 있는 아이템인지 한번 체크한다.
				if( !pThis->IsEnableUnEquipItem( pcsAgpdItem ) ) return FALSE;
				bResult = pThis->m_pcsAgcmItem->SendSubInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );
			}
			//Bank에서 인벤으로 옮겨진경우.
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_BANK )
			{
				bResult = pThis->m_pcsAgcmItem->SendSubInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );
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
																					AGPDITEM_STATUS_SUB_INVENTORY);
																				
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
				pThis->m_nNPCTradeBuyStatus = AGPDITEM_STATUS_SUB_INVENTORY;

				if (pThis->m_pcsAgpmGrid->IsEmptyGrid(pstGrid, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn, 1, 1))
				{
					pThis->m_lNPCTradeBuyLayer	= pcsUIGrid->m_lNowLayer;
					pThis->m_lNPCTradeBuyRow	= pstGridSelectInfo->lGridRow;
					pThis->m_lNPCTradeBuyColumn	= pstGridSelectInfo->lGridColumn;
				}

				bResult = pThis->CBUINPCTradeMSGConfirmBuy((PVOID) NULL, (PVOID) pThis, (PVOID) NULL);
			}
			// from guild warehouse
			else if( pcsAgpdItem->m_eStatus == AGPDITEM_STATUS_GUILD_WAREHOUSE)
			{
				INT32 lStatus = (INT32)AGPDITEM_STATUS_SUB_INVENTORY;

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
				bResult = pThis->m_pcsAgcmItem->SendSubInventoryInfo( lItemID, pcsAgpdItem->m_ulCID, pcsUIGrid->m_lNowLayer, pstGridSelectInfo->lGridRow, pstGridSelectInfo->lGridColumn );

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

BOOL AgcmUIItem::CBSubInventoryMoveEndItem(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
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

BOOL AgcmUIItem::CBSubInventoryUIUpdate(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIItem *pThis = (AgcmUIItem *) pClass;
	BOOL			bResult;

	bResult = FALSE;

	if( pThis )
	{
		if( pThis->m_bUIOpenSubInventory )
		{
			bResult = pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_lSubInventoryUIClose );
			pThis->m_bUIOpenSubInventory = FALSE;

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
			pThis->OpenSubInventory();
		}
	}

	return bResult;
}

BOOL AgcmUIItem::CBSubInventoryOpenInvenLayer(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
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

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh( pThis->m_pcsSubInventory );
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh( pThis->m_pcsSubInventoryLayerIndex );

	return TRUE;
}

BOOL AgcmUIItem::CBDragDropItemToSubInventory(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
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

	return pThis->m_pcsAgcmItem->SendSubInventoryInfo(pcsGridItem->m_lItemID, pThis->m_pcsAgcmCharacter->GetSelfCID(), lLayerIndex);
}

BOOL AgcmUIItem::CBToggleAutoGetDropItem( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl )
{
	if( !pClass ) return FALSE;
	AgcmUIItem* pThis = ( AgcmUIItem* )pClass;

	if( !pcsSourceControl || !pcsSourceControl->m_pcsBase ) return FALSE;
	AcUIButton* pBtn = ( AcUIButton* )pcsSourceControl->m_pcsBase;

	INT16 bAuto = ( INT16 )pThis->m_AutoPickUp.ToggleAutoPickUp();

	CHAR* pBtnKeyText = bAuto ? UI_MESSAGE_ID_ITEM_AUTO_PICK_GROUND_ITEM_ON : UI_MESSAGE_ID_ITEM_AUTO_PICK_GROUND_ITEM_OFF;
	if( pBtnKeyText )
	{
		CHAR* pBtnText = pThis->m_pcsAgcmUIManager2->GetUIMessage( pBtnKeyText );
		if( pBtnText )
		{
			pBtn->SetStaticString( pBtnText );
		}
	}
	
	// ToDo : 변경된 자동줍기 설정값을 서버로 전송
	AgpdCharacter *pcsSelfCharacter = pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if( pcsSelfCharacter == NULL ) return FALSE;

	AgpdItemADChar* pcsItemADChar = pThis->m_pcsAgpmItem->GetADCharacter(pcsSelfCharacter);
	if( !pcsItemADChar ) return FALSE;

	// Find pet is using.
	AgpdItem* pcsPetItem = NULL;
	INT32 lIndex = 0;
	for(AgpdGridItem* pcsGridItem = pThis->m_pcsAgpmGrid->GetItemSequence(&pcsItemADChar->m_csCashInventoryGrid, &lIndex);
		pcsGridItem;
		pcsGridItem = pThis->m_pcsAgpmGrid->GetItemSequence(&pcsItemADChar->m_csCashInventoryGrid, &lIndex))
	{
		AgpdItem* pcsItem = pThis->m_pcsAgpmItem->GetItem(pcsGridItem);
		if( !pThis->IsPetItem( pcsItem ) )
			continue;

		if(pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_INUSE)
		{
			pcsPetItem = pcsItem;
			break;
		}
	}

	if(pcsPetItem == NULL) return FALSE;
	return pThis->m_pcsAgcmItem->SendAutoPickItem( pcsPetItem->m_lID, pcsPetItem->m_ulCID, &bAuto );
}

BOOL AgcmUIItem::CBGetBtnAutoPickUp( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl )
{
	if( !pClass ) return FALSE;
	AgcmUIItem* pThis = ( AgcmUIItem* )pClass;

	AgcdUIControl* pControl = ( AgcdUIControl* )pData1;
	if( !pControl || !pControl->m_pcsBase ) return FALSE;

	pThis->m_pBtnAutoPickUp = ( AcUIButton* )pControl->m_pcsBase;
	pThis->InitializeAutoPickup();
	return TRUE;
}

BOOL AgcmUIItem::IsPetItem( AgpdItem* ppdItem )
{
	if( !ppdItem || !ppdItem->m_pcsItemTemplate ) return FALSE;
	if( ppdItem->m_pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE ) return FALSE;
	if( !m_pcsAgpmItem->IsUsingStamina( ppdItem->m_pcsItemTemplate ) ) return FALSE;
	return TRUE;
}

BOOL AgcmUIItem::InitializeAutoPickup( void )
{
	if( !m_pBtnAutoPickUp ) return FALSE;

	INT16 bAuto = ( INT16 )m_AutoPickUp.IsAutoPickUp();

	CHAR* pBtnKeyText = bAuto ? UI_MESSAGE_ID_ITEM_AUTO_PICK_GROUND_ITEM_ON : UI_MESSAGE_ID_ITEM_AUTO_PICK_GROUND_ITEM_OFF;
	if( pBtnKeyText )
	{
		CHAR* pBtnText = m_pcsAgcmUIManager2->GetUIMessage( pBtnKeyText );
		if( pBtnText )
		{
			m_pBtnAutoPickUp->SetStaticString( pBtnText );
		}
	}
	else
	{
		m_pBtnAutoPickUp->SetStaticString( "" );
	}

	return TRUE;
}

BOOL AgcmUIItem::OpenSubInventory()
{
	BOOL	bResult = FALSE;

	AgpdCharacter	*pcsSelfCharacter	= m_pcsAgcmCharacter->GetSelfCharacter();
	if( !m_bUIOpenSubInventory && pcsSelfCharacter )
	{
		// Self Character의 스테이트를 확인한다.
		if( pcsSelfCharacter->IsDead() || m_pcsAgpmItem->IsEnableSubInventory(pcsSelfCharacter) == FALSE )
		{
			// do nothing..
		}
		else
		{
			bResult = m_pcsAgcmUIManager2->ThrowEvent( m_lSubInventoryUIOpen );
			m_bUIOpenSubInventory = TRUE;

			// Enable a sub inventory one more time
			EnableSubInventory();
		}
	}

	return bResult;
}

BOOL AgcmUIItem::CloseSubInventory()
{
	m_pcsAgcmUIManager2->ThrowEvent( m_lSubInventoryUIClose );
	m_bUIOpenSubInventory = FALSE;

	return TRUE;
}

void AgcmUIItem::EnableSubInventory()
{
	if(!m_pcsUISubInventory)
		m_pcsUISubInventory = m_pcsAgcmUIManager2->GetUI("UI_SubInventory");

	if(m_pcsUISubInventory)
	{
		m_pcsUISubInventory->m_pcsUIWindow->SetActive(TRUE);
		m_pcsAgcmUIManager2->ThrowEvent( m_nEventGetBtnAutoPickUp );
	}
}

void AgcmUIItem::DisableSubInventory()
{
	if(!m_pcsUISubInventory)
		m_pcsUISubInventory = m_pcsAgcmUIManager2->GetUI("UI_SubInventory");

	if(m_pcsUISubInventory)
	{
		m_pcsUISubInventory->m_pcsUIWindow->SetActive(FALSE);
	}
}