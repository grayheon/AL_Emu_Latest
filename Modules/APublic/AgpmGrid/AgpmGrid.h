/******************************************************************************
Module:  AgpdGrid.h
Notices: Copyright (c) NHN Studio 2003 Ashulam
Purpose: 
Last Update: 2003. 09. 4
******************************************************************************/

#ifndef __AGPMGRID_H__
#define __AGPMGRID_H__

#include "AgpdGrid.h"
#include "ApMemoryPool.h"

#if _MSC_VER < 1300
#ifdef	_DEBUG
#pragma comment ( lib , "AgpmGridD" )
#else
#pragma comment ( lib , "AgpmGrid" )
#endif
#endif

enum 
{
	AGPMGRID_DATA_GRIDITEM		=	0
};

class AgpmGrid : public ApModule 
{
private:
	// Memory Pool
	ApMemoryPool m_BankGridItem;
	ApMemoryPool m_BankLockTable;

	ApMemoryPool m_InventoryGridItem;
	ApMemoryPool m_InventoryLockTable;

	ApMemoryPool m_EquipGridItem;
	ApMemoryPool m_EquipLockTable;

	ApMemoryPool m_TradeGridItem;
	ApMemoryPool m_TradeLockTable;

	ApMemoryPool m_SalesGridItem;
	ApMemoryPool m_SalesLockTable;

	ApMemoryPool m_NPCTradeGridItem;
	ApMemoryPool m_NPCTradeLockTable;

	ApMemoryPool m_QuestGridItem;
	ApMemoryPool m_QuestLockTable;

//	2005.11.16. By SungHoon
	ApMemoryPool m_CashInventoryGridItem;
	ApMemoryPool m_CashInventoryLockTable;

	ApMemoryPool m_GuildWarehouseGridItem;
	ApMemoryPool m_GuildWarehouseLockTable;	

	ApMemoryPool m_SubInventoryGridItem;
	ApMemoryPool m_SubInventoryLockTable;

	ApMemoryPool m_ChargingTypeInventoryGridItem;
	ApMemoryPool m_ChargingInventoryLockTable;

	BOOL m_bUseMemoryPool;

	BOOL CreateByMemoryPool(EnumGridType eGridType, INT32 nLayer, INT32 nRow, INT32 nColumn, AgpdGrid *pcsAgpdGrid, 
							ApMemoryPool* pPoolGridItem, ApMemoryPool* pPoolLockTable);
	BOOL DeleteByMemoryPool(AgpdGrid *pcsAgpdGrid, ApMemoryPool* pPoolGridItem, ApMemoryPool* pPoolLockTable);

public:

	//현재 ItemGrid에 들어있는 Item의 IDList. 이거 나중에 BST같은걸루 바꿔야한다. 검색이 느림.
//	INT64			m_llItemList[AGPMITEM_MAX_ITEM_OWN];

	AgpmGrid();
	~AgpmGrid();

	AgpdGridItem*	CreateGridItem()	;
	BOOL			DeleteGridItem(AgpdGridItem *pcsGridItem)	;

	void InitGridMemoryPool(INT32 lPoolCount);
	BOOL Init( AgpdGrid *pcsAgpdGrid, EnumGridType eGridType );
	BOOL Init( AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn );

	AgpdGridItem *GetItem( AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn );
	AgpdGridItem *GetItem( AgpdGrid *pcsAgpdGrid, INT32 lIndex );

	INT16 GetLayer( AgpdGrid *pcsAgpdGrid );
	INT16 GetRow(AgpdGrid *pcsAgpdGrid );
	INT16 GetColumn( AgpdGrid *pcsAgpdGrid );
	INT32 GetItemCount( AgpdGrid *pcsAgpdGrid );

	INT16 GetLayerByIndex( AgpdGrid *pcsAgpdGrid, INT32 lIndex );
	INT16 GetRowByIndex( AgpdGrid *pcsAgpdGrid, INT32 lIndex );
	INT16 GetColumnByIndex( AgpdGrid *pcsAgpdGrid, INT32 lIndex );

	INT32 GetCountByTemplate(AgpdGrid *pcsAgpdGrid, AgpdGridItemType eType, INT32 lItemTID);

	BOOL IsExistItem( AgpdGrid *pcsAgpdGrid, AgpdGridItemType eType, INT32 lItemID, INT32 *plIndex = NULL );
	BOOL IsExistItemTemplate( AgpdGrid *pcsAgpdGrid, AgpdGridItemType eType, INT32 lItemTID );
	AgpdGridItem* GetItemByTemplate( AgpdGrid *pcsAgpdGrid, AgpdGridItemType eType, INT32 lItemTID );
	AgpdGridItem* GetItemByTemplate( INT32 &lIndex, AgpdGrid *pcsAgpdGrid, AgpdGridItemType eType, INT32 lItemTID );
	AgpdGridItem* GetQuestItemByTemplate( AgpdGrid *pcsAgpdGrid, AgpdGridItemType eType, INT32 lItemTID );
	AgpdGridItem* GetItemByIndexAndTID( INT32 &Index, AgpdGrid *pcsAgpdGrid, AgpdGridItemType eType, INT32 lItemTID );
	BOOL AddItem( AgpdGrid *pcsGrid, AgpdGridItem *pcsGridItem );
	BOOL DeleteItem( AgpdGrid *pcsGrid, AgpdGridItem *pcsGridItem );

	BOOL IsInGrid( AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn, INT16 nHeight, INT16 nWidth );
	BOOL IsMovalbeGrid( AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn, AgpdGridItem *pcsGridItem, INT16 nHeight, INT16 nWidth );

	BOOL GetEmptyGrid( AgpdGrid *pcsAgpdGrid, INT16 *pnLayer, INT16 *pnRow, INT16 *pnColumn, INT16 nHeight, INT16 nWidth );
	BOOL GetEmptyGridWithStartPos( AgpdGrid *pcsAgpdGrid, INT16 nStartLayer, INT16 nStartRow, INT16 nStartColumn, INT16 *pnLayer, INT16 *pnRow, INT16 *pnColumn, INT16 nHeight, INT16 nWidth );
	BOOL IsEmptyGrid( AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn, INT16 nHeight, INT16 nWidth );
	INT32 GetEmpyGridCount(AgpdGrid* pcsAgpdGrid);

	// Grid가 가득차 있는지 확인
	BOOL IsFullGrid( AgpdGrid *pcsAgpdGrid );

	BOOL Add( AgpdGrid *pcsAgpdGrid, AgpdGridItem *pcsGridItem, INT16 nHeight, INT16 nWidth );
	BOOL Add( AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn, AgpdGridItem *pcsGridItem, INT16 nHeight, INT16 nWidth );
	BOOL Add( AgpdGrid *pcsAgpdGrid, INT16 StartLayer, INT16 StartRow, INT16 StartColumn, INT16 *nLayer, INT16 *nRow, INT16 *nColumn, AgpdGridItem *pcsGridItem, INT16 nHeight, INT16 nWidth );
	BOOL AddToLayer( AgpdGrid *pcsAgpdGrid, INT16 nLayer, AgpdGridItem *pcsGridItem, INT16 nHeight, INT16 nWidth, INT16 *nRow = NULL, INT16 *nColumn = NULL);
	BOOL Clear( AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn, INT16 nHeight, INT16 nWidth );
	BOOL RemoveItem(AgpdGrid *pcsAgpdGrid, AgpdGridItem *pcsGridItem);
	BOOL RemoveItemAndFillFirst(AgpdGrid *pcsAgpdGrid, AgpdGridItem *pcsGridItem);
//	BOOL Update( AgpdItem *pcsAgpdItem, INT16 nLayer, INT16 nRow, INT16 nColumn, INT64 llItemID, INT16 nHeight, INT16 nWidth );
//	BOOL Update( INT16 nOriginLayer, INT16 nOriginRow, INT16 nOriginColumn, INT16 nLayer, INT16 nRow, INT16 nColumn, INT64 llItemID, INT16 nHeight, INT16 nWidth );

	BOOL Remove( AgpdGrid *pcsAgpdGrid );

	VOID UpdateMaxRowExistItem(AgpdGrid *pcsAgpdGrid);

	AgpdGridItem *	GetItemSequence(AgpdGrid *pcsAgpdGrid, INT32 *plIndex);

	VOID Reset(AgpdGrid *pcsAgpdGrid);

	INT16 AttachGridItemTextureData(PVOID pClass, INT32 nDataSize, ApModuleDefaultCallBack pfConstructor, ApModuleDefaultCallBack pfDestructor);

	BOOL IsLocked(AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn);
	BOOL LockItem(AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn, INT16 nHeight, INT16 nWidth);
	BOOL ReleaseItem(AgpdGrid *pcsAgpdGrid, INT16 nLayer, INT16 nRow, INT16 nColumn, INT16 nHeight, INT16 nWidth);

	BOOL CheckEnoughItem(AgpdGrid *pcsAgpdGrid, INT32 lItemTID, INT32 lCount);	// ItemTemplate이 lCount만큼 해당 grid에 있는지 확인하는 루틴
};

#endif
