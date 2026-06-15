/******************************************************************************
Module:  AgpmItem.cpp
Notices: Copyright (c) NHN Studio 2002 netong
Purpose: 
Last Update: 2002. 04. 16
******************************************************************************/

#include <stdio.h>
#include "AgpmItem.h"
#include "ApMemoryTracker.h"
#include <AgppItem.h>

#include "AuZpackLoader.h"
#include "AuMD5Encrypt.h"
#include <set>

UINT32	g_ulReserveItemData		= 8000;	// msec

// Durability가 있는 장비
const INT32	AgpmItem::s_lBeDurabilityEquipItem[AGPM_DURABILITY_EQUIP_PART_MAX] = 
{
	AGPMITEM_PART_HEAD,					//	3
	AGPMITEM_PART_BODY,					//	2
	AGPMITEM_PART_LEGS,					//	6
	AGPMITEM_PART_HANDS,				//	4
	AGPMITEM_PART_FOOT,					//	7
	AGPMITEM_PART_HAND_LEFT,			//	9
	AGPMITEM_PART_HAND_RIGHT,			//	10					
};


void AgpmItem::InitTradeBlockItems()
{
	tradeBlockItems_.clear();

	static wchar_t * packName = L"ini.zp";
	static char const * filename = "ini\\tradeBlockItems.txt";

	bool isZpackStream = true;

	size_t size = 0;

	char * filestream = (char*)GetZpackStream( packName, filename, size );

	// 없으면 그냥 읽음
	if( !filestream )
	{
		isZpackStream = false;

		FILE * file = fopen( filename, "rb" );

		if( file )
		{
			fseek( file, 0, SEEK_END );
			size = ftell( file );
			fseek( file, 0, SEEK_SET );

			if( size )
			{
				filestream = new char[size + 1];

				if( fread( filestream, 1, size, file ) != size )
				{

					bool isEncrypt = false;

					// 암호화 되어있으면
					#if !defined( _BIN_EXEC_ ) && !defined( _WIN64 )
					isEncrypt = true;
					#endif

					if( isEncrypt )
					{
						AuMD5Encrypt MD5;

						if (!MD5.DecryptString(MD5_HASH_KEY_STRING, filestream, (unsigned long)size))
						{
							delete [] filestream;
							filestream = 0;
						}
					}

					if( filestream )
						filestream[size] = 0;

					fclose( file );
				}
			}
		}
	}

	if( filestream )
	{
		std::istringstream itemstream( filestream );

		char buf[1024];

		while( itemstream.getline( buf, sizeof(buf) ) )
		{
			int itemId = atoi( buf );

			if( tradeBlockItems_.count( itemId ) == 0 )
				tradeBlockItems_.insert( itemId );
		}

		if( isZpackStream )
			ReleaseZpackStream( packName, filestream );
		else
			delete [] filestream;
	}
}

bool AgpmItem::IsTradeBlockItem( AgpdItem * item )
{
	return item && (tradeBlockItems_.count( item->m_lTID ) > 0);
}

INT32 AgpmItem::GetTemplateDataSize()
{
	INT32	lTemplateSize = 0;

	if(sizeof(AgpdItemTemplateEquipWeapon) >= sizeof(AgpdItemTemplateUsableSkillBook))
	{
		if(sizeof(AgpdItemTemplateEquipWeapon) >= sizeof(AgpdItemTemplateOtherEtc))
			lTemplateSize = sizeof(AgpdItemTemplateEquipWeapon);
		else
			lTemplateSize = sizeof(AgpdItemTemplateOtherEtc);
	}
	else
	{
		if(sizeof(AgpdItemTemplateUsableSkillBook) >= sizeof(AgpdItemTemplateOtherEtc))
			lTemplateSize = sizeof(AgpdItemTemplateUsableSkillBook);
		else
			lTemplateSize = sizeof(AgpdItemTemplateOtherEtc);
	}

	return lTemplateSize;
}

AgpmItem::AgpmItem()
{
	SetModuleName("AgpmItem");

	EnableIdle2(TRUE);

	// setting module data
	SetModuleData(sizeof(AgpdItem));

	// BOB님 수정(171203)
	SetModuleData(GetTemplateDataSize(), AGPMITEM_DATA_TYPE_TEMPLATE);
	SetModuleData(sizeof(AgpdItemOptionTemplate), AGPMITEM_DATA_TYPE_OPTION_TEMPLATE);

	SetPacketType(AGPMITEM_PACKET_TYPE);

	m_csPacket.Initialize();

	m_csPacketField.SetFlagLength(sizeof(INT8));
	m_csPacketField.SetFieldType(
		                         AUTYPE_POS,       1, // position
		                         AUTYPE_END,       0
								 );

	m_csPacketInventory.SetFlagLength(sizeof(INT8));
	m_csPacketInventory.SetFieldType(
									 AUTYPE_INT16, 1, // Inventory tab(몇번째 인벤토리냐?)
									 AUTYPE_INT16, 1, // Inventory row
									 AUTYPE_INT16, 1, // Inventory column
		                             AUTYPE_END,   0
									 );

	m_csPacketBank.SetFlagLength(sizeof(INT8));
	m_csPacketBank.SetFieldType(
									 AUTYPE_INT16, 1, // Bank tab(몇번째 뱅크냐?)
									 AUTYPE_INT16, 1, // Bank row
									 AUTYPE_INT16, 1, // Bank column
		                             AUTYPE_END,   0
									 );
	
	m_csPacketQuest.SetFlagLength(sizeof(INT8));
	m_csPacketQuest.SetFieldType(
									 AUTYPE_INT16, 1, // tab
									 AUTYPE_INT16, 1, // row
									 AUTYPE_INT16, 1, // column
		                             AUTYPE_END,   0
								);

	m_csPacketEquip.SetFlagLength(sizeof(INT8));
	m_csPacketEquip.SetFieldType(
								    AUTYPE_END,       0
								 );

	m_csPacketConvert.SetFlagLength(sizeof(INT16));
	m_csPacketConvert.SetFieldType(
									AUTYPE_INT8,		1,						// convert operation & result
									AUTYPE_INT32,		1,						// spirit stone id

									// whole convert history information
									/////////////////////////////////////////////////////////
									AUTYPE_INT8,		1,						// total convert level
									AUTYPE_FLOAT,		1,						// convert constant
									AUTYPE_INT8,		AGPMITEM_MAX_CONVERT,	// bUseSpiritStone
									AUTYPE_INT32,		AGPMITEM_MAX_CONVERT,	// lSpiritStoneTID
									AUTYPE_INT8,		AGPMITEM_MAX_CONVERT,	// lRuneAttributeType
									AUTYPE_INT8,		AGPMITEM_MAX_CONVERT,	// lRuneAttributeValue
									AUTYPE_PACKET,		AGPMITEM_MAX_CONVERT,	// converted spirit stone factor point
									AUTYPE_PACKET,		AGPMITEM_MAX_CONVERT,	// converted spirit stone factor percent

									// add convert history
									/////////////////////////////////////////////////////////
									AUTYPE_INT8,		1,						// bUseSpiritStone
									AUTYPE_INT32,		1,						// lSpiritStoneTID
									AUTYPE_INT8,		1,						// lRuneAttributeType
									AUTYPE_INT8,		1,						// lRuneAttributeValue
									AUTYPE_PACKET,		1,						// converted spirit stone factor point
									AUTYPE_PACKET,		1,						// converted spirit stone factor percent

									AUTYPE_END,			0
									);

	m_csPacketEgo.SetFlagLength(sizeof(INT8));
	m_csPacketEgo.SetFieldType(
									AUTYPE_INT8,		1,		// ego item operation & result
									AUTYPE_INT32,		1,		// soul cube id
									AUTYPE_INT32,		1,		// soul character id
									AUTYPE_INT32,		1,		// target item id
									AUTYPE_END,			0
									);

	m_csPacketOption.SetFlagLength(sizeof(INT8));
	m_csPacketOption.SetFieldType(
									AUTYPE_UINT16,		1,		// option tid 1
									AUTYPE_UINT16,		1,		// option tid 2
									AUTYPE_UINT16,		1,		// option tid 3
									AUTYPE_UINT16,		1,		// option tid 4
									AUTYPE_UINT16,		1,		// option tid 5
									AUTYPE_END,			0
									);

	m_csPacketSkillPlus.SetFlagLength(sizeof(INT8));
	m_csPacketSkillPlus.SetFieldType(
									AUTYPE_UINT16,		1,		// skill tid 1
									AUTYPE_UINT16,		1,		// skill tid 2
									AUTYPE_UINT16,		1,		// skill tid 3
									AUTYPE_END,			0
									);

	m_csPacketCashInformaion.SetFlagLength(sizeof(INT8));
	m_csPacketCashInformaion.SetFieldType(
									AUTYPE_INT8,		1,		//	inuse
									AUTYPE_INT64,		1,		//	RemainTime
									AUTYPE_UINT32,		1,		//	ExpireTime
									AUTYPE_INT32,		1,		//	EableTrade
									AUTYPE_INT64,		1,		//	StaminaRemainTime
									AUTYPE_END,			0
									);

	m_csPacketAutoPickItem.SetFlagLength(sizeof(INT8));
	m_csPacketAutoPickItem.SetFieldType(
								  AUTYPE_INT16,			1,		// On / Off
								  AUTYPE_END,			0
								  );

	m_csPacketExtra.SetFlagLength(sizeof(INT8));
	m_csPacketExtra.SetFieldType(	AUTYPE_CHAR, sizeof(PACKET_AGPMITEM_EXTRA),
									AUTYPE_END,			0);
								

	m_lItemMoneyTID	= AP_INVALID_IID;
	m_lItemArrowTID	= AP_INVALID_IID;
	m_lItemBoltTID	= AP_INVALID_IID;

	//memset( m_lEquipItemTable, -1, sizeof(INT32)*AGPMITEM_PART_NUM );
	//m_lEquipItemTable.MemSet(0, AGPMITEM_PART_NUM);

	for (int i = 0; i < AGPMITEM_PART_NUM; ++i)
		m_lEquipItemTable[i]	= (-1);

	m_lEquipItemTable[ AGPMITEM_PART_ACCESSORY_NECKLACE	] = 0	;
	m_lEquipItemTable[ AGPMITEM_PART_HEAD				] = 1	;
	m_lEquipItemTable[ AGPMITEM_PART_HAND_RIGHT			] = 2	;
	m_lEquipItemTable[ AGPMITEM_PART_ACCESSORY_RING1	] = 3	;
	m_lEquipItemTable[ AGPMITEM_PART_HANDS				] = 4	;
	m_lEquipItemTable[ AGPMITEM_PART_BODY				] = 5	;
	m_lEquipItemTable[ AGPMITEM_PART_HAND_LEFT			] = 6	;
	m_lEquipItemTable[ AGPMITEM_PART_ACCESSORY_RING2	] = 7	;
	m_lEquipItemTable[ AGPMITEM_PART_FOOT				] = 8	;
	m_lEquipItemTable[ AGPMITEM_PART_LEGS				] = 9	;
	m_lEquipItemTable[ AGPMITEM_PART_LANCER				] = 10	;
	m_lEquipItemTable[ AGPMITEM_PART_RIDE				] = 11	;
	m_lEquipItemTable[ AGPMITEM_PART_ARMS				] = 12	;
	m_lEquipItemTable[ AGPMITEM_PART_ARMS2				] = 13	;

	m_lEquipItemTable[ AGPMITEM_PART_V_ACCESSORY_NECKLACE	] = 14	;
	m_lEquipItemTable[ AGPMITEM_PART_V_HEAD					] = 15	;
	m_lEquipItemTable[ AGPMITEM_PART_V_HAND_RIGHT			] = 16	;
	m_lEquipItemTable[ AGPMITEM_PART_V_ACCESSORY_RING1		] = 17	;
	m_lEquipItemTable[ AGPMITEM_PART_V_HANDS				] = 18	;
	m_lEquipItemTable[ AGPMITEM_PART_V_BODY					] = 19	;
	m_lEquipItemTable[ AGPMITEM_PART_V_HAND_LEFT			] = 20	;
	m_lEquipItemTable[ AGPMITEM_PART_V_ACCESSORY_RING2		] = 21	;
	m_lEquipItemTable[ AGPMITEM_PART_V_FOOT					] = 22	;
	m_lEquipItemTable[ AGPMITEM_PART_V_LEGS					] = 23	;
	m_lEquipItemTable[ AGPMITEM_PART_V_LANCER				] = 24	;
	m_lEquipItemTable[ AGPMITEM_PART_V_RIDE					] = 25	;
	m_lEquipItemTable[ AGPMITEM_PART_V_ARMS					] = 26	;
	m_lEquipItemTable[ AGPMITEM_PART_V_ARMS2				] = 27	;

	m_ulPrevRemoveClockCount			= 0;

	m_bIsRemovePolearm					= FALSE;

	m_lItemHumanSkullTID				= 0;
	m_lItemOrcSkullTID					= 0;
	for(int s = 0; s < AGPMITEM_MAX_SKULL_LEVEL; s++)
		m_alSkullTID[s] = 0;

	m_lItemCatalystTID					= 0;
	m_lItemLuckyScrollTID				= 0;

	m_lItemReverseOrbTID				= 0;

	m_lItemChattingEmphasisTID				= 0;

	ZeroMemory(m_lItemSkillBookTID, sizeof(INT32) * AURACE_TYPE_MAX * AUCHARCLASS_TYPE_MAX);

	m_bIsCompareTemplate				= FALSE;

	for (int i = 0; i < AGPMITEM_BANK_MAX_LAYER + 1; ++i)
		m_llBankSlotPrice[i] = -1;

	m_pagpmSystemMessage				= NULL;
}

AgpmItem::~AgpmItem()
{
}

BOOL AgpmItem::OnAddModule()
{
	m_pagpmGrid = (AgpmGrid *) GetModule("AgpmGrid");
	m_papmMap = (ApmMap *) GetModule("ApmMap");
	m_pagpmCharacter = (AgpmCharacter *) GetModule("AgpmCharacter");
	m_pagpmFactors = (AgpmFactors *) GetModule("AgpmFactors");
	m_pagpmBillInfo = (AgpmBillInfo *) GetModule("AgpmBillInfo");
	m_pagpmSystemMessage = (AgpmSystemMessage *) GetModule("AgpmSystemMessage");

	
	if (!m_papmMap || !m_pagpmCharacter || !m_pagpmGrid || !m_pagpmBillInfo)
	{
		OutputDebugString("AgpmItem::OnAddModule() Error (1) !!!\n");
		return FALSE;
	}
	

	// setting attached data(m_nKeeping) in character module
	if (m_pagpmCharacter)
	{
		m_nIndexADCharacter = m_pagpmCharacter->AttachCharacterData(this, sizeof(AgpdItemADChar), ConAgpdItemADChar, DesAgpdItemADChar);
		m_nIndexADCharacterTemplate = m_pagpmCharacter->AttachCharacterTemplateData(this, sizeof(AgpdItemADCharTemplate), ConAgpdItemADCharTemplate, DesAgpdItemADCharTemplate);

		if (m_nIndexADCharacter < 0 || m_nIndexADCharacterTemplate < 0)
		{
			OutputDebugString("AgpmItem::OnAddModule() Error (2) !!!\n");
			return FALSE;
		}

		if (!m_pagpmCharacter->AddStreamCallback(AGPMCHAR_DATA_TYPE_TEMPLATE, CharTemplateReadCB, CharTemplateWriteCB, this))
		{
			OutputDebugString("AgpmItem::OnAddModule() Error (3) !!!\n");
			return FALSE;
		}

		if (!m_pagpmCharacter->SetCallbackStreamReadImportData(CharacterImportDataReadCB, this))
		{
			OutputDebugString("AgpmItem::OnAddModule() Error (333) !!!\n");
			return FALSE;
		}

		if (!m_pagpmCharacter->SetCallbackRemoveChar(CBRemoveChar, this))
		{
			OutputDebugString("AgpmItem::OnAddModule() Error (6) !!!\n");
			return FALSE;
		}

		if (!m_pagpmCharacter->SetCallbackResetMonster(CBRemoveChar, this))
		{
			OutputDebugString("AgpmItem::OnAddModule() Error (69) !!!\n");
			return FALSE;
		}

		if (!m_pagpmCharacter->SetCallbackActionPickupItem(CBPickupItem, this))
		{
			OutputDebugString("AgpmItem::OnAddModule() Error (666) !!!\n");
			return FALSE;
		}

		if (!m_pagpmCharacter->SetCallbackCheckActionRequirement(CBCheckActionRequirement, this))
			return FALSE;

		/*
		if (!m_pagpmCharacter->SetCallbackReCalcFactor(CBReCalcFactor, this))
		{
			OutputDebugString("AgpmItem::OnAddModule() Error (7) !!!\n");
			return FALSE;
		}
		*/

		if (!m_pagpmCharacter->SetCallbackPayActionCost(CBPayActionCost, this))
			return FALSE;

		if (!m_pagpmCharacter->SetCallbackGetItemLancer(CBGetItemLancer, this))
			return FALSE;

		if (!m_pagpmCharacter->SetCallbackInitTemplateDefaultValue(CBInitTemplateDefaultValue, this))
			return FALSE;
	}

	if (!AddStreamCallback(AGPMITEM_DATA_TYPE_TEMPLATE, TemplateReadCB, TemplateWriteCB, this))
	{
		OutputDebugString("AgpmItem::OnAddModule() Error (4) !!!\n");
		return FALSE;
	}

	if (!AddStreamCallback(AGPMITEM_DATA_TYPE_ITEM, ItemReadCB, ItemWriteCB, this))
	{
		OutputDebugString("AgpmItem::OnAddModule() Error (5) !!!\n");
		return FALSE;
	}

	return TRUE;
}

BOOL AgpmItem::OnInit()
{
	BOOL	bRetVal;

	// initialize data size (item, item template data)
	bRetVal = csItemAdmin.InitializeObject(sizeof(AgpdItem *), csItemAdmin.GetCount());
	bRetVal &= m_csAdminItemRemove.InitializeObject(sizeof(AgpdItem *), m_csAdminItemRemove.GetCount());

	if (bRetVal == FALSE)
	{
		// initialize failed... error return
		OutputDebugString("AgpmItem::OnInit() Error (1) !!!\n");
		return FALSE;
	}

	InitTradeBlockItems();

	return bRetVal;
}

BOOL AgpmItem::OnDestroy()
{
	INT32	lIndex;
	AgpdItem				*pcsItem;

	lIndex = 0;
	for (pcsItem = GetItemSequence(&lIndex); pcsItem; pcsItem = GetItemSequence(&lIndex))
	{
		// Modify 090602 Bob Jung.-DestroyItem 막았음.
		RemoveItem(pcsItem->m_lID);
		//DestroyItem(pcsItem);
	}

	lIndex = 0;
	AgpdItem			**ppcsItem	= (AgpdItem **) m_csAdminItemRemove.GetObjectSequence(&lIndex);
	while (ppcsItem && *ppcsItem)
	{
		DestroyItem(*ppcsItem);
		ppcsItem	= (AgpdItem **) m_csAdminItemRemove.GetObjectSequence(&lIndex);
	}

	DestroyAllTemplate();

	for(AgpaItemOptionTemplate::iterator it = csOptionTemplateAdmin.begin(); it != csOptionTemplateAdmin.end(); ++it)
	{
		if (it->second)
			DestroyOptionTemplate(it->second);
	}

	csItemAdmin.RemoveObjectAll();
	csTemplateAdmin.clear();
	csOptionTemplateAdmin.clear();

	return TRUE;
}

BOOL AgpmItem::OnIdle2(UINT32 ulClockCount)
{
	PROFILE("AgpmItem::OnIdle2");

	//STOPWATCH2(GetModuleName(), _T("OnIdle2"));

	return ProcessRemove(ulClockCount);
}

BOOL AgpmItem::OnValid(CHAR* szData, INT16 nSize)
{
	return TRUE;
}

INT32 AgpmItem::GetEquipIndexFromTable( INT32 lPart )
{
	if( (0 <= lPart) && (lPart < AGPMITEM_PART_NUM ) )
	{
		return m_lEquipItemTable[lPart];
	}

	return (-1);
}

AgpdItem* AgpmItem::CreateItem()
{
	AgpdItem* pcsItem = (AgpdItem *) CreateModuleData(AGPMITEM_DATA_TYPE_ITEM);

	if ( pcsItem && m_pagpmGrid )
	{
		pcsItem->m_Mutex.Init((PVOID) pcsItem);
		pcsItem->m_eType = APBASE_TYPE_ITEM;
		pcsItem->m_eStatus = AGPDITEM_STATUS_NOTSETTING;
		pcsItem->m_eNewStatus = AGPDITEM_STATUS_NOTSETTING;

		if (m_pagpmFactors)
		{
			m_pagpmFactors->InitFactor(&pcsItem->m_csFactor);
			m_pagpmFactors->InitFactor(&pcsItem->m_csFactorPercent);
			m_pagpmFactors->InitFactor(&pcsItem->m_csRestrictFactor);
		}

		pcsItem->m_csFactor.m_bPoint			= TRUE;
		pcsItem->m_csFactorPercent.m_bPoint		= FALSE;

		// 아템 개조 초기화
//		pcsItem->m_stConvertHistory.lConvertLevel		= 0;
//		pcsItem->m_stConvertHistory.fConvertConstant	= 1.25;
//
//		if (m_pagpmFactors)
//		{
//			for (int i = 0; i < AGPMITEM_MAX_CONVERT; ++i)
//			{
//				m_pagpmFactors->InitFactor(&pcsItem->m_stConvertHistory.csFactorHistory[i]);
//				m_pagpmFactors->InitFactor(&pcsItem->m_stConvertHistory.csFactorPercentHistory[i]);
//
//				pcsItem->m_stConvertHistory.csFactorHistory[i].m_bPoint = TRUE;
//				pcsItem->m_stConvertHistory.csFactorPercentHistory[i].m_bPoint = FALSE;
//			}
//		}

		ZeroMemory(&pcsItem->m_szSoulMasterID, sizeof(CHAR) * (AGPACHARACTER_MAX_ID_STRING + 1));

		//pcsItem->m_pcsGridItem = new AgpdGridItem;
		pcsItem->m_pcsGridItem = m_pagpmGrid->CreateGridItem();
		if (pcsItem->m_pcsGridItem)
			pcsItem->m_pcsGridItem->SetParentBase((ApBase *) pcsItem);
		
		pcsItem->m_pstSkullInfo = NULL;

		pcsItem->m_ulRemoveTimeMSec	= 0;

		//pcsItem->m_lDeleteReason	= 0;

		ZeroMemory(pcsItem->m_szDeleteReason, sizeof(CHAR) * (AGPMITEM_MAX_DELETE_REASON + 1));


		pcsItem->m_nDimension = 0;

		pcsItem->m_aunSkillPlusTID.MemSetAll();

		pcsItem->m_nInUseItem = AGPDITEM_CASH_ITEM_UNUSE;
		pcsItem->m_lRemainTime = 0;
		pcsItem->m_lExpireTime = 0;
		pcsItem->m_llStaminaRemainTime = 0;

		pcsItem->m_pcsGuildWarehouseBase	= NULL;
	}

	return pcsItem;
}

BOOL AgpmItem::DestroyItem(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	if (m_pagpmFactors)
	{
		m_pagpmFactors->DestroyFactor(&pcsItem->m_csFactor);
		m_pagpmFactors->DestroyFactor(&pcsItem->m_csFactorPercent);
		m_pagpmFactors->DestroyFactor(&pcsItem->m_csRestrictFactor);

//		for (int i = 0; i < pcsItem->m_stConvertHistory.lConvertLevel; ++i)
//		{
//			m_pagpmFactors->DestroyFactor(&pcsItem->m_stConvertHistory.csFactorHistory[i]);
//			m_pagpmFactors->DestroyFactor(&pcsItem->m_stConvertHistory.csFactorPercentHistory[i]);
//		}
	}

	if (pcsItem->m_pstSkullInfo)
		GlobalFree(pcsItem->m_pstSkullInfo);

	if (pcsItem->m_pcsGridItem)
		m_pagpmGrid->DeleteGridItem(pcsItem->m_pcsGridItem);

	pcsItem->m_Mutex.Destroy();

	return DestroyModuleData((PVOID) pcsItem, AGPMITEM_DATA_TYPE_ITEM);
}

BOOL AgpmItem::CreateCopyItem( AgpdItem *pcAgpdItem, INT16 eStatus, INT32 lIID, INT32 lTID, INT32 lCID, INT16 nLayer, INT16 nRow, INT16 nColumn )
{
	BOOL			bResult;

	bResult = FALSE;

	if( pcAgpdItem != NULL )
	{
		if (m_pagpmCharacter)
		{
			pcAgpdItem->m_pcsCharacter = m_pagpmCharacter->GetCharacter(lCID);

			pcAgpdItem->m_eStatus = eStatus;
			pcAgpdItem->m_lID= lIID;
			pcAgpdItem->m_lTID = lTID;
			pcAgpdItem->m_ulCID = lCID;
			pcAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB] = nLayer;
			pcAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW] = nRow;
			pcAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN] = nColumn;

			bResult = TRUE;
		}
	}

	return bResult;
}

AgpdItemTemplate* AgpmItem::CreateTemplate()
{
	AgpdItemTemplate* pcsTemplate = (AgpdItemTemplate *) CreateModuleData(AGPMITEM_DATA_TYPE_TEMPLATE);

	if (pcsTemplate)
	{
		pcsTemplate->m_Mutex.Init((PVOID) pcsTemplate);
		pcsTemplate->m_eType = APBASE_TYPE_ITEM_TEMPLATE;

		pcsTemplate->m_nSizeInInventory[AGPDITEMTEMPLATESIZE_WIDTH] = 1;
		pcsTemplate->m_nSizeInInventory[AGPDITEMTEMPLATESIZE_HEIGHT] = 1;

		ZeroMemory(&pcsTemplate->m_csFactor, sizeof(AgpdFactor));
		if (m_pagpmFactors)
		{
			m_pagpmFactors->InitFactor(&pcsTemplate->m_csFactor);
			m_pagpmFactors->InitFactor(&pcsTemplate->m_csRestrictFactor);
		}

		pcsTemplate->m_csFactor.m_bPoint = TRUE;

		/*
		// usable item template setting
		AgpdItemTemplateUsable	*pcsTemplateUsable = (AgpdItemTemplateUsable *) pcsTemplate;

		ZeroMemory(pcsTemplateUsable->m_szSpellName, sizeof(CHAR) * AGPMSKILL_MAX_SKILL_NAME);

		if (m_pagpmFactors)
			m_pagpmFactors->InitFactor(&pcsTemplateUsable->m_csEffectFactor);

		pcsTemplateUsable->m_lEffectActivityTimeMsec	= 0;
		// usable item template setting
		*/

		pcsTemplate->m_pcsGridItem					= m_pagpmGrid->CreateGridItem();
//		pcsTemplate->m_pcsGridItemDurabilityZero	= m_pagpmGrid->CreateGridItem();
//		pcsTemplate->m_pcsGridItemDurabilityUnder5	= m_pagpmGrid->CreateGridItem();

		if (pcsTemplate->m_pcsGridItem)
			pcsTemplate->m_pcsGridItem->SetParentBase((ApBase *) pcsTemplate);

		pcsTemplate->m_lFirstCategory = 0;
		pcsTemplate->m_lSecondCategory = 0;
		pcsTemplate->m_bFreeDuration = FALSE;

		pcsTemplate->m_eCashItemType = AGPMITEM_CASH_ITEM_TYPE_NONE;
		pcsTemplate->m_eCashItemUseType = AGPMITEM_CASH_ITEM_USE_TYPE_UNUSABLE;

		pcsTemplate->m_alOptionTID.MemSetAll();
		pcsTemplate->m_alLinkID.MemSetAll();
		pcsTemplate->m_aunSkillPlusTID.MemSetAll();
		
		pcsTemplate->m_bEnableGamble	= FALSE	;
		pcsTemplate->m_nQuestGroup		= 0		;
		pcsTemplate->m_nGachaType		= 0		;

		pcsTemplate->m_llStaminaCure = 0;
		pcsTemplate->m_llStaminaRemainTime = 0;

		pcsTemplate->m_vClassifyID.clear();
	}

	return pcsTemplate;
}

BOOL AgpmItem::DestroyAllTemplate()
{
	for(AgpaItemTemplate::iterator it = csTemplateAdmin.begin(); it != csTemplateAdmin.end(); ++it)
	{
		if(!DestroyTemplate(it->second))
			return FALSE;
	}

	return TRUE;
}

BOOL AgpmItem::DestroyTemplate(AgpdItemTemplate *pcsTemplate)
{
	if (!pcsTemplate)
		return FALSE;

	if (m_pagpmFactors)
	{
		m_pagpmFactors->DestroyFactor(&pcsTemplate->m_csFactor);
		m_pagpmFactors->DestroyFactor(&pcsTemplate->m_csRestrictFactor);

		if (pcsTemplate->m_nType == AGPMITEM_TYPE_USABLE)
		{
			// usable item template setting
			AgpdItemTemplateUsable	*pcsTemplateUsable = (AgpdItemTemplateUsable *) pcsTemplate;

			m_pagpmFactors->DestroyFactor(&pcsTemplateUsable->m_csEffectFactor);
			// usable item template setting
		}
	}

	if (pcsTemplate->m_pcsGridItem)
		m_pagpmGrid->DeleteGridItem(pcsTemplate->m_pcsGridItem);

//	if (pcsTemplate->m_pcsGridItemDurabilityZero)
//		m_pagpmGrid->DeleteGridItem(pcsTemplate->m_pcsGridItemDurabilityZero);
//
//	if (pcsTemplate->m_pcsGridItemDurabilityUnder5)
//		m_pagpmGrid->DeleteGridItem(pcsTemplate->m_pcsGridItemDurabilityUnder5);

	if (pcsTemplate->m_pcsGridItemSmall)
		m_pagpmGrid->DeleteGridItem(pcsTemplate->m_pcsGridItemSmall);

	// BOB추가(241103)
	csTemplateAdmin.erase(pcsTemplate->m_lID);

	pcsTemplate->m_Mutex.Destroy();

	return DestroyModuleData((PVOID) pcsTemplate, AGPMITEM_DATA_TYPE_TEMPLATE);
}

AgpdItemOptionTemplate* AgpmItem::CreateOptionTemplate()
{
	AgpdItemOptionTemplate	*pcsOptionTemplate	= (AgpdItemOptionTemplate *) CreateModuleData(AGPMITEM_DATA_TYPE_OPTION_TEMPLATE);
	if (!pcsOptionTemplate)
		return NULL;

	if (m_pagpmFactors)
	{
		m_pagpmFactors->InitFactor(&pcsOptionTemplate->m_csFactor);
		m_pagpmFactors->InitFactor(&pcsOptionTemplate->m_csFactorPercent);
		m_pagpmFactors->InitFactor(&pcsOptionTemplate->m_csSkillFactor);
		m_pagpmFactors->InitFactor(&pcsOptionTemplate->m_csSkillFactorPercent);
	}

	return pcsOptionTemplate;
}

BOOL AgpmItem::DestroyOptionTemplate(AgpdItemOptionTemplate *pcsOptionTemplate)
{
	if (!pcsOptionTemplate)
		return FALSE;

	if (m_pagpmFactors)
	{
		m_pagpmFactors->DestroyFactor(&pcsOptionTemplate->m_csFactor);
		m_pagpmFactors->DestroyFactor(&pcsOptionTemplate->m_csFactorPercent);
		m_pagpmFactors->DestroyFactor(&pcsOptionTemplate->m_csSkillFactor);
		m_pagpmFactors->DestroyFactor(&pcsOptionTemplate->m_csSkillFactorPercent);
	}

	return DestroyModuleData((PVOID) pcsOptionTemplate, AGPMITEM_DATA_TYPE_OPTION_TEMPLATE);;
}

AgpdGrid *AgpmItem::GetInventory( INT32 lCID )
{
	AgpdGrid	*pcsAgpdGrid = NULL;
	AgpdCharacter	*pcsAgpdCharacter = NULL;
	
	if (m_pagpmCharacter)
		pcsAgpdCharacter = m_pagpmCharacter->GetCharacter(lCID);

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csInventoryGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetInventory( AgpdCharacter *pcsAgpdCharacter )
{
	AgpdGrid	*pcsAgpdGrid = NULL;

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csInventoryGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetEquipGrid( INT32 lCID )
{
	AgpdGrid	*pcsAgpdGrid = NULL;
	AgpdCharacter	*pcsAgpdCharacter = NULL;
	
	if (m_pagpmCharacter)
		pcsAgpdCharacter = m_pagpmCharacter->GetCharacter(lCID);

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csEquipGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetEquipGrid( AgpdCharacter *pcsAgpdCharacter )
{
	AgpdGrid	*pcsAgpdGrid = NULL;

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csEquipGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetBank( INT32 lCID )
{
	AgpdGrid	*pcsAgpdGrid = NULL;
	AgpdCharacter	*pcsAgpdCharacter = NULL;
	
	if (m_pagpmCharacter)
		pcsAgpdCharacter = m_pagpmCharacter->GetCharacter(lCID);

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csBankGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetBank( AgpdCharacter *pcsAgpdCharacter )
{
	AgpdGrid	*pcsAgpdGrid = NULL;

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csBankGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetTradeGrid( INT32 lCID )
{
	AgpdGrid	*pcsAgpdGrid = NULL;
	AgpdCharacter	*pcsAgpdCharacter = NULL;
	
	if (m_pagpmCharacter)
		pcsAgpdCharacter = m_pagpmCharacter->GetCharacter(lCID);

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csTradeGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetTradeGrid( AgpdCharacter *pcsAgpdCharacter )
{
	AgpdGrid	*pcsAgpdGrid = NULL;

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csTradeGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetClientTradeGrid( INT32 lCID )
{
	AgpdGrid	*pcsAgpdGrid = NULL;
	AgpdCharacter	*pcsAgpdCharacter = NULL;
	
	if (m_pagpmCharacter)
		pcsAgpdCharacter = m_pagpmCharacter->GetCharacter(lCID);

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csClientTradeGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetClientTradeGrid( AgpdCharacter *pcsAgpdCharacter )
{
	AgpdGrid	*pcsAgpdGrid = NULL;

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csClientTradeGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetSalesBox( INT32 lCID )
{
	AgpdGrid	*pcsAgpdGrid = NULL;
	AgpdCharacter	*pcsAgpdCharacter = NULL;
	
	if (m_pagpmCharacter)
		pcsAgpdCharacter = m_pagpmCharacter->GetCharacter(lCID);

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csSalesBoxGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetSalesBox( AgpdCharacter *pcsAgpdCharacter )
{
	AgpdGrid	*pcsAgpdGrid = NULL;

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csSalesBoxGrid;
		}
	}

	return pcsAgpdGrid;
}

AgpdGrid *AgpmItem::GetQuestGrid( INT32 lCID )
{
	AgpdGrid	*pcsAgpdGrid = NULL;
	AgpdCharacter	*pcsAgpdCharacter = NULL;
	
	if (m_pagpmCharacter)
		pcsAgpdCharacter = m_pagpmCharacter->GetCharacter(lCID);

	if( !pcsAgpdCharacter ) return NULL;

	return GetQuestGrid(pcsAgpdCharacter);
}

AgpdGrid *AgpmItem::GetQuestGrid( AgpdCharacter *pcsAgpdCharacter )
{
	AgpdGrid	*pcsAgpdGrid = NULL;

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csQuestGrid;
		}
	}

	return pcsAgpdGrid;
}

/*
	2005.11.16. By SungHoon
	해당 lCID캐릭터의 캐쉬인벤토리를 가져온다.
*/
AgpdGrid *AgpmItem::GetCashInventoryGrid( INT32 lCID )
{
	AgpdCharacter	*pcsAgpdCharacter = NULL;
	
	if (m_pagpmCharacter)
		pcsAgpdCharacter = m_pagpmCharacter->GetCharacter(lCID);

	if( !pcsAgpdCharacter ) return NULL;

	return GetCashInventoryGrid(pcsAgpdCharacter);
}

/*
	2005.11.16. By SungHoon
	해당 pcsAgpdCharacter 캐릭터의 캐쉬인벤토리를 가져온다.
*/
AgpdGrid *AgpmItem::GetCashInventoryGrid( AgpdCharacter *pcsAgpdCharacter )
{
	AgpdGrid	*pcsAgpdGrid = NULL;

	if( pcsAgpdCharacter )
	{
		AgpdItemADChar *pcsAgpdItemADChar = GetADCharacter(pcsAgpdCharacter);

		if( pcsAgpdItemADChar != NULL )
		{
			pcsAgpdGrid = &pcsAgpdItemADChar->m_csCashInventoryGrid;
		}
	}

	return pcsAgpdGrid;
}

BOOL AgpmItem::ConAgpdItemADChar(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmItem *pThis	= (AgpmItem *) pClass;
	AgpdItemADChar *pAttachedData = pThis->GetADCharacter((AgpdCharacter *) pData);

	// 여기서 데이타 세팅
	pAttachedData->m_nNumItem		= 0;

//	ZeroMemory(pAttachedData->m_pvItemData, sizeof(PVOID) * AGPMITEM_MAX_ITEM_OWN);

//	ZeroMemory(pAttachedData->m_lEquipSlot, sizeof(INT64) * AGPMITEM_MAX_EQUIP_SLOT);

	pThis->m_pagpmGrid->Init(&pAttachedData->m_csInventoryGrid, AGPDGRID_TYPE_INVENTORY );

	pThis->m_pagpmGrid->Init(&pAttachedData->m_csEquipGrid, AGPDGRID_TYPE_EQUIP );

	pThis->m_pagpmGrid->Init(&pAttachedData->m_csBankGrid, AGPDGRID_TYPE_BANK );

	pThis->m_pagpmGrid->Init(&pAttachedData->m_csSalesBoxGrid, AGPDGRID_TYPE_SALES );

	pThis->m_pagpmGrid->Init(&pAttachedData->m_csTradeGrid, AGPDGRID_TYPE_TRADEBOX );

	pThis->m_pagpmGrid->Init(&pAttachedData->m_csClientTradeGrid, AGPDGRID_TYPE_TRADEBOX );

	pThis->m_pagpmGrid->Init(&pAttachedData->m_csQuestGrid, AGPDGRID_TYPE_QUEST);

//	2005.11.16. By SungHoon
	pThis->m_pagpmGrid->Init(&pAttachedData->m_csCashInventoryGrid, AGPDGRID_TYPE_CASH_INVENTORY);

	pThis->m_pagpmGrid->Init(&pAttachedData->m_csSubInventoryGrid, AGPDGRID_TYPE_SUB_INVENTORY);	// 2008.05.08. steeple

	pThis->m_pagpmGrid->Init(&pAttachedData->m_csChargingInventoryGrid, AGPDGRID_TYPE_CHARGING_INVENTORY);
	
	pAttachedData->m_csInventoryGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;
	pAttachedData->m_csEquipGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;
	pAttachedData->m_csTradeGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;
	pAttachedData->m_csClientTradeGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;
	pAttachedData->m_csSalesBoxGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;
	pAttachedData->m_csBankGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;
	pAttachedData->m_csQuestGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;
//	2005.11.16. By SungHoon
	pAttachedData->m_csCashInventoryGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;
	pAttachedData->m_csSubInventoryGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;
	pAttachedData->m_csChargingInventoryGrid.m_lGridType = AGPDGRID_ITEM_TYPE_ITEM;

	//거래관련 스탯을 Reset
	pThis->ResetTradeStats(pAttachedData);

	pAttachedData->m_lArrowIDInInventory	= AP_INVALID_IID;
	pAttachedData->m_lBoltIDInInventory		= AP_INVALID_IID;

	pAttachedData->m_lHPPotionIDInInventory	= AP_INVALID_IID;
	pAttachedData->m_lMPPotionIDInInventory	= AP_INVALID_IID;
	pAttachedData->m_lSPPotionIDInInventory	= AP_INVALID_IID;

	pAttachedData->m_lNumArrowCount			= 0;
	pAttachedData->m_lNumBoltCount			= 0;

	pAttachedData->m_ulUseHPPotionTime		= 0;
	pAttachedData->m_ulUseMPPotionTime		= 0;
	pAttachedData->m_ulUseSPPotionTime		= 0;

	pAttachedData->m_ulUseReverseOrbTime	= 0;
	pAttachedData->m_ulUseTransformTime		= 0;

	pAttachedData->m_ulUseJumpTime			= 0;

	pAttachedData->m_CooldownInfo.m_pBases	= new AgpdItemCooldownArray::CooldownBaseVector;	// 2008.02.14. steeple

	pAttachedData->m_bUseChagingInventory	= FALSE;
	pAttachedData->m_eChargingBagType		= AGPMITEM_USABLE_CHARGING_BAG_TYPE_NONE;

	return TRUE;
}

BOOL AgpmItem::DesAgpdItemADChar(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgpmItem *pThis	= (AgpmItem *) pClass;
	AgpdItemADChar *pAttachedData = pThis->GetADCharacter((AgpdCharacter *) pData);

	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csInventoryGrid);
	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csEquipGrid);
	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csBankGrid);
	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csSalesBoxGrid);
	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csTradeGrid);
	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csClientTradeGrid);
	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csQuestGrid);
//	2005.11.16. By SungHoon
	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csCashInventoryGrid);
	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csSubInventoryGrid);
	pThis->m_pagpmGrid->Remove(&pAttachedData->m_csChargingInventoryGrid);

	if(pAttachedData->m_CooldownInfo.m_pBases)
	{
		pAttachedData->m_CooldownInfo.m_pBases->clear();
		delete pAttachedData->m_CooldownInfo.m_pBases;
	}

	return TRUE;
}

BOOL AgpmItem::ConAgpdItemADCharTemplate(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmItem *pThis	= (AgpmItem *) pClass;
	AgpdItemADCharTemplate *pAttachedData = pThis->GetADCharacterTemplate((AgpdCharacterTemplate *) pData);

	new (pAttachedData) AgpdItemADCharTemplate;

	//ZeroMemory(pAttachedData->m_lDefaultEquipITID, sizeof(INT32) * AGPMITEM_PART_NUM);

	return TRUE;
}

BOOL AgpmItem::DesAgpdItemADCharTemplate(PVOID pData, PVOID pClass, PVOID pCustData)
{
	return TRUE;
}

BOOL AgpmItem::SetCallbackInit(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_INIT, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackAdd(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_ADD, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackNewItemToClient(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_ADD_CLIENT_TRADE_GRID, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackRemove(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_REMOVE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackRemoveID(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_REMOVE_ID, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackDelete(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_DELETE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUpdateFactor(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_FACTOR, pfCallback, pClass);
}

//Trade Grid에 뭔가 들어오거나 나갈때~
BOOL AgpmItem::SetCallbackUpdateTradeGrid(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_TRADEGRID, pfCallback, pClass);
}

//거래자의 Trade Grid에 뭔가 들어오거나 나갈때~
BOOL AgpmItem::SetCallbackUpdateTargetTradeGrid(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_TARGET_TRADEGRID, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackChangeItemOwner(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_CHANGE_ITEM_OWNER, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUseItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_USE_ITEM, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUseItemByTID(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_USE_ITEM_BY_TID, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUseItemFailedByTID(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_USE_ITEM_FAILED_BY_TID, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUseItemSuccess(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_USE_ITEM_SUCCESS, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUpdateReturnTeleportStatus(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_RETURN_TELEPORT_STATUS, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUseReturnScroll(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_USE_RETURN_SCROLL, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUseReturnScrollResultFailed(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_USE_RETURN_SCROLL_RESULT_FAILED, pfCallback, pClass);
}

//BOOL AgpmItem::SetCallbackAskReallyConvertItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(ITEM_CB_ID_ASK_REALLY_CONVERT_ITEM, pfCallback, pClass);
//}

//BOOL AgpmItem::SetCallbackConvertItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(ITEM_CB_ID_CONVERT_ITEM, pfCallback, pClass);
//}

BOOL AgpmItem::SetCallbackCheckPickupItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_CHECK_CAN_PICKUP_ITEM, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackPickupItemMoney(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_PICKUP_ITEM_MONEY, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackPickupItemResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_PICKUP_ITEM_RESULT, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackIconSlotInput(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_ITEM_SLOT_SOUND, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUIUpdateInventory(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UI_UPDATE_INVENTORY, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUIUpdateEquip(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UI_UPDATE_EQUIP, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUIUpdateTrade(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UI_UPDATE_TRADE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUIUpdateTradeClient(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UI_UPDATE_TRADE_CLIENT, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUIUpdateBank(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UI_UPDATE_BANK, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUIUpdateSubInventory(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UI_UPDATE_SUB_INVENTORY, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUIUpdateChargingInventory(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UI_UPDATE_CHARGING_INVENTORY, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUIUpdateTradeOption(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UI_UPDATE_TRADE_OPTION , pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUIUpdateSalesBox(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UI_UPDATE_SALESBOX, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUpdateStackCount(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_STACK_COUNT, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUpdateRequireLevel(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_REQUIRE_LEVEL, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackReceiveItemData(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_RECEIVE_ITEM_DATA, pfCallback, pClass);
}

//BOOL AgpmItem::SetCallbackReceiveConvertData(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(ITEM_CB_ID_RECEIVE_CONVERT_DATA, pfCallback, pClass);
//}

BOOL AgpmItem::SetCallbackReadTemplate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_READ_TEMPLATE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackStreamReadImportDataErrorReport(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_STREAM_READ_IMPORT_DATA_ERROR_REPORT, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackStreamReadOptionTable(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_STREAM_READ_OPTION_TABLE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackRemoveInventoryGrid(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_REMOVE_INVENTORY_GRID, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackSalesBoxRemoveUIUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_SALESBOX_REMOVE_UI_UPDATE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackDropMoneyToField(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_DROP_MONEY_TO_FIELD, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackAddItemToMap(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_ADD_ITEM_TO_MAP, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackRemoveItemFromMap(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_REMOVE_ITEM_FROM_MAP, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackRequestSpiritStoneConvert(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_REQUEST_SPIRITSTONE_CONVERT, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackAddItemToBank(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_ADD_ITEM_TO_BANK, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackRemoveItemFromBank(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_REMOVE_ITEM_FROM_BANK, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackAddItemToGuildWarehouse(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_ADD_ITEM_TO_GUILD_WAREHOUSE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackRemoveItemFromGuildWarehouse(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_REMOVE_ITEM_FROM_GUILD_WAREHOUSE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackSplitItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_SPLIT_ITEM, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackCheckJoinItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_CHECK_JOIN_ITEM, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUpdateReuseTimeForReverseOrb(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_REUSETIME_FOR_REVERSE_ORB, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUpdateReuseTimeForTransform(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_REUSETIME_FOR_TRANSFORM, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackInitReuseTimeForTransform(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_INIT_REUSETIME_FOR_TRANSFORM, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUpdateItemStatusFlag(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_ITEM_STATUS_FLAG, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackStreamReadImportData(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_STREAM_READ_IMPORT_DATA, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackInventoryQuestAdd(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_ADD_QUEST, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackInventoryQuestUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_QUEST, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackInventoryQuestRemove(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_REMOVE_QUEST, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUnuseItem(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UNUSE_ITEM, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUseItemResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_USE_ITEM_RESULT, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackRemoveGuildWarehouse(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_REMOVE_GUILD_WAREHOUSE_GRID, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackAddItemOption(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_ADD_ITEM_OPTION, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackUpdateCooldown(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_UPDATE_COOLDOWN, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackChangeAutoPickUp(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_CHANGE_AUTOPICK_ITEM, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackAddItemExtraData(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ID_ADD_ITEM_EXTRADATA, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackItemInformation(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ITEM_INFORMATION, pfCallback, pClass);
}
BOOL AgpmItem::SetCallbackAddGachaItemUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ADD_GACHA_ITEM_UPDATE, pfCallback, pClass);
}

BOOL AgpmItem::SetCallbackItemInformationResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(ITEM_CB_ITEM_INFORMATION_RESULT, pfCallback, pClass);
}

//		AddItem
//	Functions
//		- 새로운 아이템 데이타를 추가한다.
//	Arguments
//		- lIID : 추가할 아이템 아뒤
//		- lTID : 추가할 아이템 TID
//	Return value
//		- AgpdItem * : 추가한 아이템 포인터
///////////////////////////////////////////////////////////////////////////////
AgpdItem* AgpmItem::AddItem(INT32 lIID, INT32 lTID, INT32 lCount, BOOL bAddtoAdmin)
{
	AgpdItemTemplate *pcsItemTemplate = GetItemTemplate(lTID);
	if (!pcsItemTemplate)
	{
		//OutputDebugString("AgpmItem::AddItem() Error (1) !!!\n");
		return NULL;
	}

	//스태커블 아이템인데 count가 0보다 작거나 최대 숫자보다 크다?
	if( pcsItemTemplate->m_bStackable == TRUE )
	{
		if( (lCount < 0)/* && ( pcsItemTemplate->m_lMaxStackableCount < lCount )*/ )
		{
			return NULL;
		}
	}

	AgpdItem *pcsItem = CreateItem();
	if (!pcsItem)
	{
		OutputDebugString("AgpmItem::AddItem() Error (2) !!!\n");
		return NULL;
	}

	pcsItem->m_lID = lIID;
	pcsItem->m_lTID = lTID;

	if (pcsItemTemplate->m_bStackable)
	{	
		if (pcsItemTemplate->m_lMaxStackableCount < lCount)
			pcsItem->m_nCount	= pcsItemTemplate->m_lMaxStackableCount;
		else
			pcsItem->m_nCount	= lCount;
	}
	else
		pcsItem->m_nCount	= 0;

	pcsItem->m_pcsItemTemplate = pcsItemTemplate;

	/*
	if (pcsItemTemplate->m_nType == AGPMITEM_TYPE_EQUIP &&
		(((AgpdItemTemplateEquip *) pcsItemTemplate)->m_nKind == AGPMITEM_EQUIP_KIND_RING ||
		 ((AgpdItemTemplateEquip *) pcsItemTemplate)->m_nKind == AGPMITEM_EQUIP_KIND_NECKLACE))
	{
		pcsItem->m_csFactor.m_bPoint			= FALSE;
		pcsItem->m_csFactorPercent.m_bPoint		= TRUE;

		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactorPercent, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_CHAR_STATUS);
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactorPercent, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_CHAR_POINT);
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactorPercent, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_CHAR_POINT_MAX);
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactorPercent, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_CHAR_POINT_RECOVERY_RATE);
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactorPercent, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_DAMAGE);
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactorPercent, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_DEFENSE);
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactorPercent, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_ATTACK);

		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactor, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_CHAR_TYPE);
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactor, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_ITEM);
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactor, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_DIRT);
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactor, &pcsItemTemplate->m_csFactor, FALSE, FALSE, AGPD_FACTORS_TYPE_PRICE);
	}
	else
	*/
	{
		m_pagpmFactors->CopyFactor(&pcsItem->m_csFactor, &pcsItemTemplate->m_csFactor, FALSE);
	}

	m_pagpmFactors->CopyFactor(&pcsItem->m_csRestrictFactor, &pcsItemTemplate->m_csRestrictFactor, FALSE);

	pcsItem->m_lStatusFlag	= pcsItemTemplate->m_lStatusFlag;

	//Admin에 넣을지를 본다.
	if( bAddtoAdmin )
	{
		if (!csItemAdmin.AddItem(pcsItem))
		{
			DestroyItem(pcsItem);

			return NULL;
		}
	}

	m_pagpmFactors->SetOwnerFactor(&pcsItem->m_csFactor, pcsItem->m_lID, pcsItem);

	EnumCallback(ITEM_CB_ID_ADD, (PVOID)(pcsItem), NULL);

	return pcsItem;
}

AgpdItem* AgpmItem::AddItem(AgpdItem *pcsItem)
{
	if (pcsItem->m_lID == AP_INVALID_IID || pcsItem->m_lTID == AP_INVALID_IID)
		return NULL;

	if (!csItemAdmin.AddItem(pcsItem))
	{
		return NULL;
	}

	return pcsItem;
}

BOOL AgpmItem::InitItem(AgpdItem *pcsItem)
{
	pcsItem->m_pcsGridItem->m_eType = AGPDGRID_ITEM_TYPE_ITEM;
	pcsItem->m_pcsGridItem->m_lItemID = pcsItem->m_lID;
	pcsItem->m_pcsGridItem->m_lItemTID = pcsItem->m_lTID;
	pcsItem->m_pcsGridItem->m_bMoveable = TRUE				;

	pcsItem->m_pcsGuildWarehouseBase	= NULL;

	// 이 아템 추가와 관련한 콜백 함수들을 호출해준다.
	EnumCallback(ITEM_CB_ID_INIT, pcsItem, pcsItem->m_pcsItemTemplate);

	return TRUE;
}

/******************************************************************************
* Purpose : 아이템의 상태만 날린다.
*
* 111002. Bob Jung
******************************************************************************/
/*
BOOL AgpmItem::ReleaseItem(INT32 lIID)
{
	return ReleaseItem(GetItem(lIID));
}

BOOL AgpmItem::ReleaseItem(AgpdItem *pcsAgpdItem, AgpdItemStatus eNewStatus)
{
	switch(pcsAgpdItem->m_eStatus)
	{
	case AGPDITEM_STATUS_FIELD:
		{
			if(!RemoveItemFromField(pcsAgpdItem, TRUE))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (1) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_INVENTORY:
		{
			if(!RemoveItemFromInventory(pcsAgpdItem->m_ulCID, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (2) !!!\n");
				return FALSE;
			}
			else
			{
				EnumCallback( ITEM_CB_ID_CHAR_REMOVE_INVENTORY, (PVOID)pcsAgpdItem, (PVOID) eNewStatus );
			}
		}
		break;

	case AGPDITEM_STATUS_TRADE_GRID:
		{
			if(!RemoveItemFromTradeGrid(pcsAgpdItem->m_ulCID, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (2) !!!\n");
				return FALSE;
			}
			else
			{
				EnumCallback( ITEM_CB_ID_UPDATE_TRADEGRID, (PVOID)pcsAgpdItem, (PVOID) eNewStatus );
			}
		}
		break;

	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
		{
			if(!RemoveItemFromClientTradeGrid(pcsAgpdItem->m_ulCID, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (2) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_EQUIP:
		{
			if(!UnEquipItem(pcsAgpdItem->m_ulCID, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (3) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_BANK:
		{
			AgpdCharacter *	pcsAgpdCharacter = NULL;
			
			if (m_pagpmCharacter)
				pcsAgpdCharacter = m_pagpmCharacter->GetCharacter(pcsAgpdItem->m_ulCID);

			if (!pcsAgpdCharacter)
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (10) !!!\n");
				return FALSE;
			}

			// 현재 Bank에 Item을 뺄 수 있는지 확인
			if (!EnumCallback(ITEM_CB_ID_CHAR_CHECK_BANK, pcsAgpdItem, pcsAgpdCharacter))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (11) !!!\n");
				return FALSE;
			}

			if (!RemoveItemFromBank(pcsAgpdCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (12) !!!\n");
				return FALSE;
			}

			EnumCallback(ITEM_CB_ID_CHAR_REMOVE_BANK, pcsAgpdItem, (PVOID) eNewStatus);
		}
		break;

	case AGPDITEM_STATUS_SALESBOX_GRID:
		{
			if(!RemoveItemFromSalesBox(pcsAgpdItem->m_ulCID, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (13) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_QUEST:
		{
			if (!RemoveItemFromQuest(pcsAgpdItem->m_ulCID, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (14) !!!\n");
				return FALSE;
			}
		}
		break;

	default:
		{
		}
		break;
	}

	return TRUE;
}
*/

//		RemoveItem
//	Functions
//		- 아이템을 삭제한다. (필요하다면 아이템 모듈에 붙여 관리하는 데이타에서도 삭제한다.)
//	Arguments
//		- lIID : 삭제할 아이템
//	Return value
//		- BOOL : 삭제 성공 여부
//  Modify
//      - 311002 Bob Jung.- Destroy하기 전에 ItemAdmin을 먼저 날린다.
///////////////////////////////////////////////////////////////////////////////
BOOL AgpmItem::RemoveItem(INT32 lIID, BOOL bDelete, BOOL bRemoveLock, BOOL bDestroyModuleData)
{
	AgpdItem	*pcsAgpdItem	= NULL;

	//STOPWATCH2(GetModuleName(), _T("RemoveItem"));

	if (bRemoveLock)
	{
//		if (!csItemAdmin.GlobalWLock())
//			return FALSE;

		pcsAgpdItem = GetItem(lIID);
		if(!pcsAgpdItem || !pcsAgpdItem->m_Mutex.RemoveLock())
		{
//			csItemAdmin.GlobalRelease();
			OutputDebugString("AgpmItem::RemoveItem() Error (1) !!!\n");
			return FALSE;
		}

//		if (!csItemAdmin.GlobalRelease())
//		{
//			pcsAgpdItem->m_Mutex.Release();
//			return FALSE;
//		}
	}
	else
	{
		pcsAgpdItem = GetItem(lIID);
		if (!pcsAgpdItem)
			return FALSE;
	}

	if (bDelete)
	{
//		if (strlen(pcsAgpdItem->m_szDeleteReason) <= 0)
//			ASSERT(!"AgpmItem::RemoveItem()에서 잘못된 아이템 삭제가 일어났습니다. 알려지지 않은 이유로 DB에서 아이템을 삭제하려 합니다.");

		EnumCallback(ITEM_CB_ID_DELETE, pcsAgpdItem, NULL);
	}

	EnumCallback(ITEM_CB_ID_REMOVE, pcsAgpdItem, NULL);

	// 120104-BOB님, RELEASE해 줄 필요가 없지 않나?
	//ReleaseItem( pcsAgpdItem );
	RemoveStatus(pcsAgpdItem, AGPDITEM_STATUS_NONE);

	csItemAdmin.RemoveItem(pcsAgpdItem->m_lID);

	EnumCallback(ITEM_CB_ID_REMOVE_ID, pcsAgpdItem, NULL);

	//pcsAgpdItem->m_Mutex.Destroy();
	pcsAgpdItem->m_Mutex.SafeRelease();

	if (m_csAdminItemRemove.GetCount() > 0 && !bDestroyModuleData)
		return AddRemoveItem(pcsAgpdItem);
	else
	{
//		TRACEFILE(ALEF_ERROR_FILENAME,"AgpmItem::RemoveItem() Remove Pool Full !!!\n"); 
		return DestroyItem(pcsAgpdItem);
	}
}

BOOL AgpmItem::RemoveItem(AgpdItem *pcsAgpdItem, BOOL bDelete, BOOL bDestroyModuleData)
{
	if (!pcsAgpdItem)
		return FALSE;

	INT32	lIID	= pcsAgpdItem->m_lID;

	return RemoveItem(lIID, bDelete, FALSE, bDestroyModuleData);
}

BOOL AgpmItem::RemoveAllItems()
{
	INT32	lIndex	= 0;

	AgpdItem	*pcsItem	= GetItemSequence(&lIndex);
	while (pcsItem)
	{
		RemoveItem(pcsItem);

		pcsItem	= GetItemSequence(&lIndex);
	}

	return TRUE;
}

AgpdItem* AgpmItem::GetItem(INT32 lIID)
{
	return csItemAdmin.GetItem(lIID);
}

AgpdItem* AgpmItem::GetItemLock(INT32 lIID)
{
	AgpdItem *pcsItem = GetItem(lIID);
	if (!pcsItem)
		return NULL;

	if (!pcsItem->m_Mutex.WLock())
		return NULL;

	return pcsItem;
}

AgpdItemTemplate* AgpmItem::GetItemTemplate(INT32 lTID)
{
	AgpaItemTemplate::iterator it = csTemplateAdmin.find(lTID);
	return (it != csTemplateAdmin.end()) ? it->second : NULL;
}

AgpdItemTemplate* AgpmItem::GetItemTemplate(CHAR *szTName)
{
	if (!szTName)
		return NULL;

	for(AgpaItemTemplate::iterator it = csTemplateAdmin.begin(); it != csTemplateAdmin.end(); ++it)
	{
		AgpdItemTemplate* pcsTemplate = it->second;

		if(!strcmp(szTName, pcsTemplate->m_szName))
			return pcsTemplate;
	}
	
	return NULL;
}

AgpdItemTemplate* AgpmItem::GetItemTemplateUseIID(INT32 lIID)
{
	AgpdItem *pItem = csItemAdmin.GetItem(lIID);

	if (pItem != NULL)
	{
		if (pItem->m_pcsItemTemplate)
			return (AgpdItemTemplate *) pItem->m_pcsItemTemplate;
		else
			return GetItemTemplate(pItem->m_lTID);
	}

	return NULL;
}

//		GetADCharacter
//	Functions
//		- character module 에 붙인 데이타를 가져온다.
//	Arguments
//		- pData : 붙인 데이타
//	Return value
//		- AgpdItemADChar * : character module에 붙인 데이타
///////////////////////////////////////////////////////////////////////////////
AgpdItemADChar* AgpmItem::GetADCharacter(AgpdCharacter *pData)
{
	if (m_pagpmCharacter)
		return (AgpdItemADChar *) m_pagpmCharacter->GetAttachedModuleData(m_nIndexADCharacter, (PVOID) pData);

	return NULL;
}

AgpdItemADCharTemplate* AgpmItem::GetADCharacterTemplate(AgpdCharacterTemplate *pData)
{
	if (m_pagpmCharacter)
		return (AgpdItemADCharTemplate *) m_pagpmCharacter->GetAttachedModuleData(m_nIndexADCharacterTemplate, (PVOID) pData);

	return NULL;
}

BOOL AgpmItem::SetMaxItem(INT32 nCount)
{
	return csItemAdmin.SetCount(nCount);
}

BOOL AgpmItem::SetMaxItemRemove(INT32 nCount)
{
	return m_csAdminItemRemove.SetCount(nCount);
}

// SetAttachedModuleData() wrap functions
INT16 AgpmItem::AttachItemData(PVOID pClass, INT32 nDataSize, ApModuleDefaultCallBack pfConstructor, ApModuleDefaultCallBack pfDestructor)
{
	return SetAttachedModuleData(pClass, AGPMITEM_DATA_TYPE_ITEM, nDataSize, pfConstructor, pfDestructor);
}

INT16 AgpmItem::AttachItemTemplateData(PVOID pClass, INT32 nDataSize, ApModuleDefaultCallBack pfConstructor, ApModuleDefaultCallBack pfDestructor)
{
	return SetAttachedModuleData(pClass, AGPMITEM_DATA_TYPE_TEMPLATE, nDataSize, pfConstructor, pfDestructor);
}

AgpdItemTemplate* AgpmItem::AddItemTemplate(INT32 lTID)
{
	AgpdItemTemplate *pTemplate = CreateTemplate();
	if (!pTemplate)
	{
		OutputDebugString("AgpmItem::AddItemTemplate() Error (1) !!!\n");
		return NULL;
	}

	if (pTemplate->m_pcsGridItem)
	{
		pTemplate->m_pcsGridItem->m_eType = AGPDGRID_ITEM_TYPE_ITEM;
		pTemplate->m_pcsGridItem->m_lItemTID = lTID;
	}
		

	pTemplate->m_lID = lTID;
	
	return (csTemplateAdmin.insert(AgpaItemTemplate::value_type(lTID, pTemplate)).second == TRUE) ? pTemplate : NULL;
}

AgpdItem* AgpmItem::GetItemSequence(INT32 *plIndex)
{
	AgpdItem **ppcsItem = (AgpdItem **) csItemAdmin.GetObjectSequence(plIndex);

	if (!ppcsItem)
	{
		return NULL;
	}

	return  *ppcsItem;
}

AgpdItemOptionTemplate* AgpmItem::AddItemOptionTemplate(INT32 lTID)
{
	AgpdItemOptionTemplate *pcsOptionTemplate	= CreateOptionTemplate();
	if (!pcsOptionTemplate)
	{
		OutputDebugString("AgpmItem::AddItemOptionTemplate() Error (1) !!!\n");
		return NULL;
	}

	pcsOptionTemplate->m_lID = lTID;

	return (csOptionTemplateAdmin.insert(AgpaItemOptionTemplate::value_type(lTID, pcsOptionTemplate)).second) ? pcsOptionTemplate : NULL;
}

AgpdItemOptionTemplate* AgpmItem::GetItemOptionTemplate(INT32 lTID)
{
	AgpaItemOptionTemplate::iterator it = csOptionTemplateAdmin.find(lTID);

	return (it != csOptionTemplateAdmin.end()) ? it->second : NULL;
}

BOOL AgpmItem::CBRemoveChar(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
	{
		OutputDebugString("AgpmItem::CBRemoveChar() Error (1) !!!\n");
		return FALSE;
	}

	AgpmItem		*pThis			= (AgpmItem *) pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *) pData;

	AgpdItemADChar	*pcsADChar		= pThis->GetADCharacter(pcsCharacter);

	//STOPWATCH2(pThis->GetModuleName(), _T("CBRemoveChar"));

	int i = 0;
	for (i = 0; i < pcsADChar->m_csEquipGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csEquipGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csEquipGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_EQUIP)
			continue;

		pThis->RemoveItem(pcsADChar->m_csEquipGrid.m_ppcGridData[i]->m_lItemID); // Modify 111002 Bob Jung.-RemoveItem 파라미터 변경에 따른 수정.
	}
	
	for (i = 0; i < pcsADChar->m_csInventoryGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csInventoryGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csInventoryGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_INVENTORY)
			continue;

		pThis->RemoveItem(pcsADChar->m_csInventoryGrid.m_ppcGridData[i]->m_lItemID); // Modify 111002 Bob Jung.-RemoveItem 파라미터 변경에 따른 수정.
	}

	for (i = 0; i < pcsADChar->m_csBankGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csBankGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csBankGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_BANK)
			continue;

		pThis->RemoveItem(pcsADChar->m_csBankGrid.m_ppcGridData[i]->m_lItemID); // Modify 111002 Bob Jung.-RemoveItem 파라미터 변경에 따른 수정.
	}

	for (i = 0; i < pcsADChar->m_csSalesBoxGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csSalesBoxGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csSalesBoxGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_SALESBOX_GRID)
			continue;

		pThis->RemoveItem(pcsADChar->m_csSalesBoxGrid.m_ppcGridData[i]->m_lItemID); // Modify 111002 Bob Jung.-RemoveItem 파라미터 변경에 따른 수정.
	}

	for (i = 0; i < pcsADChar->m_csTradeGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csTradeGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csTradeGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_TRADE_GRID)
			continue;

		pThis->RemoveItem(pcsADChar->m_csTradeGrid.m_ppcGridData[i]->m_lItemID); // Modify 111002 Bob Jung.-RemoveItem 파라미터 변경에 따른 수정.
	}

	for (i = 0; i < pcsADChar->m_csClientTradeGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csClientTradeGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csClientTradeGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_CLIENT_TRADE_GRID)
			continue;

		pThis->RemoveItem(pcsADChar->m_csClientTradeGrid.m_ppcGridData[i]->m_lItemID); // Modify 111002 Bob Jung.-RemoveItem 파라미터 변경에 따른 수정.
	}

	for (i = 0; i < pcsADChar->m_csQuestGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csQuestGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csQuestGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_QUEST)
			continue;

		pThis->RemoveItem(pcsADChar->m_csQuestGrid.m_ppcGridData[i]->m_lItemID); // Modify 111002 Bob Jung.-RemoveItem 파라미터 변경에 따른 수정.
	}

//	2004.11.16. By SungHoon
//	캐쉬 인벤도 정리한다.
	for (i = 0; i < pcsADChar->m_csCashInventoryGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csCashInventoryGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csCashInventoryGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_CASH_INVENTORY)
			continue;

		pThis->RemoveItem(pcsADChar->m_csCashInventoryGrid.m_ppcGridData[i]->m_lItemID);
	}

	for (i = 0; i < pcsADChar->m_csSubInventoryGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csSubInventoryGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csSubInventoryGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_SUB_INVENTORY)
			continue;

		pThis->RemoveItem(pcsADChar->m_csSubInventoryGrid.m_ppcGridData[i]->m_lItemID);
	}


	for (i = 0; i < pcsADChar->m_csChargingInventoryGrid.m_lGridCount; ++i)
	{
		if (pcsADChar->m_csChargingInventoryGrid.m_ppcGridData[i] == NULL)
			continue;

		AgpdItem *pcsItem = pThis->GetItem(pcsADChar->m_csChargingInventoryGrid.m_ppcGridData[i]->m_lItemID);
		if(NULL == pcsItem)
			continue;

		if(pcsItem->m_eStatus != AGPDITEM_STATUS_CHARGING_INVENTORY)
			continue;

		pThis->RemoveItem(pcsADChar->m_csChargingInventoryGrid.m_ppcGridData[i]->m_lItemID);		
	}

	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csEquipGrid);
	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csInventoryGrid);
	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csBankGrid);
	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csSalesBoxGrid);
	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csTradeGrid);
	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csClientTradeGrid);
	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csQuestGrid);
	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csCashInventoryGrid);
	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csSubInventoryGrid);
	pThis->m_pagpmGrid->Reset(&pcsADChar->m_csChargingInventoryGrid);

	return TRUE;
}

BOOL AgpmItem::ProcessRefreshUI( INT32 lCID, INT16 cOldStatus, INT16 cCurrentStatus )
{
	if( cOldStatus == cCurrentStatus )
	{
		if( cCurrentStatus == AGPDITEM_STATUS_INVENTORY )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_INVENTORY, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_EQUIP )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_EQUIP, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_TRADE_GRID )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_TRADE, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_CLIENT_TRADE_GRID )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_TRADE_CLIENT, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_BANK )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_BANK, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_SALESBOX_GRID )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_SALESBOX, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_CASH_INVENTORY)			//	2005.11.16. By SungHoon
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_CASH_INVENTORY, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_SUB_INVENTORY )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_SUB_INVENTORY, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_CHARGING_INVENTORY )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_CHARGING_INVENTORY, &lCID, NULL );
		}
	}
	else
	{
		if( cOldStatus == AGPDITEM_STATUS_INVENTORY )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_INVENTORY, &lCID, NULL );
		}
		else if( cOldStatus == AGPDITEM_STATUS_EQUIP )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_EQUIP, &lCID, NULL );
		}
		else if( cOldStatus == AGPDITEM_STATUS_TRADE_GRID )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_TRADE, &lCID, NULL );
		}
		else if( cOldStatus == AGPDITEM_STATUS_CLIENT_TRADE_GRID )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_TRADE_CLIENT, &lCID, NULL );
		}
		else if( cOldStatus == AGPDITEM_STATUS_BANK )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_BANK, &lCID, NULL );
		}
		else if( cOldStatus == AGPDITEM_STATUS_SALESBOX_GRID )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_SALESBOX, &lCID, NULL );
		}
		else if( cOldStatus == AGPDITEM_STATUS_CASH_INVENTORY)		//	2005.11.16. By SungHoon
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_CASH_INVENTORY, &lCID, NULL );
		}
		else if( cOldStatus == AGPDITEM_STATUS_SUB_INVENTORY )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_SUB_INVENTORY, &lCID, NULL );
		}
		else if( cOldStatus == AGPDITEM_STATUS_CHARGING_INVENTORY )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_CHARGING_INVENTORY, &lCID, NULL );
		}

		if( cCurrentStatus == AGPDITEM_STATUS_INVENTORY )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_INVENTORY, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_EQUIP )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_EQUIP, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_TRADE_GRID )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_TRADE, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_CLIENT_TRADE_GRID )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_TRADE_CLIENT, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_BANK )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_BANK, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_SALESBOX_GRID )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_SALESBOX, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_CASH_INVENTORY)		//	2005.11.16. By SungHoon
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_CASH_INVENTORY, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_SUB_INVENTORY )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_SUB_INVENTORY, &lCID, NULL );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_CHARGING_INVENTORY )
		{
			EnumCallback( ITEM_CB_ID_UI_UPDATE_CHARGING_INVENTORY, &lCID, NULL );
		}
	}

	return TRUE;
}

BOOL AgpmItem::ProcessRefreshCount( INT32 lCID, AgpdItem *pcsAgpdItem )
{
	AgpdCharacter		*pcsAgpdCharacter;
	BOOL				bResult;

	bResult = FALSE;

	pcsAgpdCharacter = m_pagpmCharacter->GetCharacter( lCID );

	if( pcsAgpdCharacter )
		bResult = ProcessRefreshCount( pcsAgpdCharacter, pcsAgpdItem );

	return bResult;
}

BOOL AgpmItem::ProcessRefreshCount( AgpdCharacter *pcsAgpdCharacter, AgpdItem *pcsAgpdItem )
{
	BOOL			bResult;

	bResult = FALSE;

	if( pcsAgpdItem ) 
	{
		AgpdGridItem		*pcsAgpdGridItem;
		INT16			nLayer, nRow, nColumn;

		nLayer = pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_TAB];
		nRow = pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_ROW];
		nColumn = pcsAgpdItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN];

		/*
		pcsAgpdItemADChar = GetADCharacter( pcsAgpdCharacter );
		cCurrentStatus = pcsAgpdItem->m_eStatus;

		if( cCurrentStatus == AGPDITEM_STATUS_INVENTORY )
		{
			pcsAgpdGridItem = m_pagpmGrid->GetItem( &pcsAgpdItemADChar->m_csInventoryGrid, nLayer, nRow, nColumn );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_EQUIP )
		{
			pcsAgpdGridItem = m_pagpmGrid->GetItem( &pcsAgpdItemADChar->m_csEquipGrid, nLayer, nRow, nColumn );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_TRADE_GRID )
		{
			pcsAgpdGridItem = m_pagpmGrid->GetItem( &pcsAgpdItemADChar->m_csTradeGrid, nLayer, nRow, nColumn );
		}
		else if( cCurrentStatus == AGPDITEM_STATUS_CLIENT_TRADE_GRID )
		{
			pcsAgpdGridItem = m_pagpmGrid->GetItem( &pcsAgpdItemADChar->m_csClientTradeGrid, nLayer, nRow, nColumn );
		}
		*/

		pcsAgpdGridItem	= pcsAgpdItem->m_pcsGridItem;

		if( pcsAgpdGridItem && pcsAgpdItem->m_pcsItemTemplate )
		{
			char			strLeftString[AGPDGRIDITEM_BOTTOM_STRING_LENGTH + 1];
			char			strRightString[AGPDGRIDITEM_BOTTOM_STRING_LENGTH + 1];

			memset( strLeftString, 0, sizeof(strLeftString) );
			memset( strRightString, 0, sizeof(strRightString) );

			//StackableItem 인지학인한다.
			if( ((AgpdItemTemplate *)pcsAgpdItem->m_pcsItemTemplate)->m_bStackable )
			{
				itoa( pcsAgpdItem->m_nCount, strRightString, 10 );
			}

//			if (pcsAgpdItem->m_stConvertHistory.lConvertLevel > 0)
//				itoa(pcsAgpdItem->m_stConvertHistory.lConvertLevel, strLeftString, 10);

			pcsAgpdGridItem->SetGridDisplayData( strLeftString, strRightString );
		}
	}	

	return bResult;
}

/******************************************************************************
* Purpose : Receive packet.
*
* 091602. Bob Jung
******************************************************************************/
BOOL AgpmItem::OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)
{
	PROFILE("AgpmItem::OnReceive");

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
	UINT32		ulReuseTImeForReverseOrb	= 0;
	INT32		lStatusFlag	= (-1);
	PVOID		pOption		= NULL;
	PVOID		pSkillPlus	= NULL;
	UINT32		ulReuseTimeForTransform		= 0;
	PVOID		pCashInformation = NULL;
	PVOID		pExtraInformation = NULL;

	m_csPacket.GetField(TRUE, pvPacket, nSize,  
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
						&ulReuseTImeForReverseOrb,
						&lStatusFlag,
						&pOption,
						&pSkillPlus,
						&ulReuseTimeForTransform,
						&pCashInformation,
						&pExtraInformation);

	//TRACE("Item Packet : %d,%d,%d,%d\n", lIID, lTID, lCID, cOperation);

	if (!pstCheckArg->bReceivedFromServer &&
		pstCheckArg->lSocketOwnerID != AP_INVALID_CID &&
		pstCheckArg->lSocketOwnerID != lCID)
		return FALSE;

	AgpdCharacter	*pcsCharacter	= m_pagpmCharacter->GetCharacterLock(lCID);
	if (!pstCheckArg->bReceivedFromServer && m_pagpmCharacter->IsAllBlockStatus(pcsCharacter))
	{
		pcsCharacter->m_Mutex.Release();
		return FALSE;
	}

	BOOL	bRetval = FALSE;

	switch(cOperation)
	{
	case AGPMITEM_PACKET_OPERATION_ADD:
		{
			if (pstCheckArg->bReceivedFromServer)
				bRetval = OnOperationAdd(pstCheckArg->bReceivedFromServer, cStatus, lIID, lTID, pcsCharacter, lItemCount, pField, pInventory, pBank, pEquip, pFactor, pFactorPercent, pRestrictFactor, pConvert, pQuest, lSkillTID, lStatusFlag, pOption, pSkillPlus, pCashInformation, pExtraInformation);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_REMOVE:
		{
			if (pstCheckArg->bReceivedFromServer)
				bRetval = OnOperationRemove(cStatus, lIID, lTID, pcsCharacter, pField, pInventory, pBank, pEquip, pFactor, pFactorPercent, pQuest);		
		}
		break;

	case AGPMITEM_PACKET_OPERATION_UPDATE:
		{
			bRetval = OnOperationUpdate(pstCheckArg->bReceivedFromServer, cStatus, lIID, lTID, pcsCharacter, lItemCount, pField, pInventory, pBank, pEquip, pFactor, pFactorPercent, pRestrictFactor, pConvert, pQuest, lSkillTID, lStatusFlag, pOption, pSkillPlus, pCashInformation, pExtraInformation);

			EnumCallback(ITEM_CB_ID_RECEIVE_ITEM_DATA, &lIID, NULL);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_USE_ITEM:
		{
			bRetval = OnOperationUseItem(cStatus, lIID, lTID, pcsCharacter, pField, pInventory, pBank, pEquip, pFactor, pFactorPercent, lTargetID);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_USE_ITEM_BY_TID:
		{
			bRetval	= OnOperationUseItemByTID(lTID, pcsCharacter, lTargetID, ulReuseTImeForReverseOrb);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_USE_ITEM_FAILED_BY_TID:
		{
			bRetval	= OnOperationUseItemFailedByTID(lTID, pcsCharacter, lTargetID);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_PICKUP_ITEM:
		{
			bRetval = OnOperationPickupItem(cStatus, lIID, pcsCharacter);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_PICKUP_ITEM_RESULT:
		{
			bRetval = OnOperationPickupItemResult(cStatus, pcsCharacter, lIID, lTID, lItemCount);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_STACK_ITEM:
		{
			bRetval = OnOperationStackItem( cStatus, lIID, pcsCharacter, pInventory, pBank, pQuest );
		}
		break;

	case AGPMITEM_PACKET_OPERATION_DROP_MONEY:
		{
			return TRUE;

			// 은행에서 자꾸 아템을 버린다. 아예 막자.
			//bRetval = OnOperationDropMoney( pcsCharacter, lItemCount );
		}
		break;

	case AGPMITEM_PACKET_OPERATION_USE_RETURN_SCROLL:
		{
			bRetval = OnOperationUseReturnScroll(pcsCharacter);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_USE_RETURN_SCROLL_FAILED:
		{
			bRetval = OnOperationUseReturnScrollResultFailed(pcsCharacter);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_ENABLE_RETURN_SCROLL:
		{
			bRetval	= OnOperationUpdateReturnScrollStatus(pcsCharacter, TRUE);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_DISABLE_RETURN_SCROLL:
		{
			bRetval	= OnOperationUpdateReturnScrollStatus(pcsCharacter, FALSE);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_CANCEL_RETURN_SCROLL:
		{
			bRetval = OnOperationCancelReturnScroll(pcsCharacter);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_USE_ITEM_SUCCESS:
		{
			bRetval = OnOperationUseItemSuccess(pcsCharacter, lIID);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_SPLIT_ITEM:
		{
			bRetval	= OnOperationSplitItem(pcsCharacter, lIID, lItemCount, cStatus, pField, pInventory);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_UPDATE_REUSE_TIME_FOR_REVERSE_ORB:
		{
			bRetval = UpdateReverseOrbReuseTime(pcsCharacter, ulReuseTImeForReverseOrb);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_UPDATE_REUSE_TIME_FOR_TRANSFORM:
		{
			bRetval = UpdateTransformReuseTime(pcsCharacter, ulReuseTimeForTransform);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_INIT_TIME_FOR_TRANSFORM:
		{
			bRetval = InitTransformReuseTime(pcsCharacter);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_REQUEST_DESTROY_ITEM:
		{
			bRetval = OnOperationRequestDestroyItem(pcsCharacter, lIID);
		}
		break;

	case AGPMITEM_PACKET_OPERATION_REQUEST_BUY_BANK_SLOT:
		{
			bRetval = OnOperationRequestBuyBankSlot(pcsCharacter);
		}
		break;
	case AGPMITEM_PACKET_OPERATION_UNUSE_ITEM :
		{
			bRetval = OnOperationUnuseItem(cStatus, lIID, lTID, pcsCharacter);
		}
		break;
	case AGPMITEM_PACKET_OPERATION_PAUSE_ITEM:
		{
			bRetval = OnOperationPauseItem(cStatus, lIID, lTID, pcsCharacter);
		}
		break;
	case AGPMITEM_PACKET_OPERATION_UPDATE_ITEM_USE_TIME:
		{
			bRetval = OnOperationUpdateItemUseTime(pcsCharacter,lIID, pCashInformation);
		}
		break;
	case AGPMITEM_PACKET_OPERATION_USE_ITEM_RESULT:
		{
			if (pstCheckArg->bReceivedFromServer)
				bRetval = OnOperationUseItemResult(pcsCharacter, (AgpmItemUseResult) cStatus);
		}
		break;
	case AGPMITEM_PACKET_OPERATION_UPDATE_COOLDOWN:
		{
			bRetval = OnOperationUpdateCooldown(pcsCharacter, lTID, ulReuseTImeForReverseOrb, lItemCount);
		}
		break;
	case AGPMITEM_PACKET_OPERATION_UPDATE_STAMINA_REMAIN_TIME:
		{
			bRetval = OnOperationUpdateStaminaRemainTime(pcsCharacter, lIID, pCashInformation);
		}
		break;
	case AGPMITEM_PACKET_OPERATION_CHANGE_AUTOPICK_ITEM:
		{
			bRetval = OnOperationChangeAutoPickUpItem(pcsCharacter, lIID, pOption);
		}
		break;
	case AGPMITEM_PACKET_OPERATION_ITEMINFORMATION:
		{
			bRetval = OnOperationItemInformation(pcsCharacter, lIID, lTID, lTargetID);
		}
		break;
	case AGPMITEM_PACKET_OPERATION_ITEMINFORMATION_RESULT:
		{
			bRetval = OnOperationItemInformationResult(pstCheckArg->bReceivedFromServer, cStatus, lIID, lTID, lCID, lItemCount, pField, pInventory, pBank, pEquip, pFactor, pFactorPercent, pRestrictFactor, pConvert, pQuest, lSkillTID, lStatusFlag, pOption, pSkillPlus, pCashInformation, pExtraInformation, lTargetID);
		}
		break;
	default:
		{
			OutputDebugString("AgpmCharacter::OnReceive() Error (19) !!!\n");
			bRetval = FALSE;
		}
		break;
	}

	if (pcsCharacter)
		pcsCharacter->m_Mutex.Release();

	return bRetval;
}

BOOL AgpmItem::OnOperationAdd(BOOL bReceivedFromServer, INT8 cStatus, INT32 lIID, INT32 lTID, AgpdCharacter *pcsCharacter, INT32 lItemCount, PVOID pField, PVOID pInventory, PVOID pBank, PVOID pEquip, PVOID pFactor, PVOID pFactorPercent, PVOID pRestrictFactor, PVOID pConvert, PVOID pQuest, INT32 lSkillTID, INT32 lStatusFlag, PVOID pOption, PVOID pSkillPlus, PVOID pvCashInformation, PVOID pvExtraInformation)
{
	AgpdItem   *pstAgpdItem;

	INT16       nIdx, nRow, nCol;

	if((!lIID) || (!lTID))
	{
		OutputDebugString("AgpmItem::OnReceive() Error (1) !!!\n");
		return FALSE; 
	}

	pstAgpdItem = AddItem(lIID, lTID, (lItemCount < 0) ? 0 : lItemCount );

	if(!pstAgpdItem)
	{
		pstAgpdItem = GetItem(lIID);
		if (!pstAgpdItem)
		{
			OutputDebugString("AgpmItem::OnReceive() Error - AddItem()도 GetItem()도 모두 실패 !!!\n");
			return FALSE;
		}

		// 자기 자신이 마블주문서를 올린거라면
		// 그리드에 추가하고 종료
		if( cStatus == AGPDITEM_STATUS_TRADE_OPTION_GRID)
		{
			_AddPrivateTradeItem( lIID , pcsCharacter );
		}

		if (m_bIsCompareTemplate && pstAgpdItem->m_pcsItemTemplate->m_lID != lTID)
		{
			if (!RemoveItem(pstAgpdItem))
				return FALSE;

			pstAgpdItem	= AddItem(lIID, lTID, (lItemCount < 0) ? 0 : lItemCount);
			if (!pstAgpdItem)
				return FALSE;
		}
		else
		{
			if (!OnOperationUpdate(bReceivedFromServer, cStatus, lIID, lTID, pcsCharacter, lItemCount, pField, pInventory, pBank, pEquip, pFactor, pFactorPercent, pRestrictFactor, pConvert, pQuest, lSkillTID, lStatusFlag, pOption, pSkillPlus, pvCashInformation, pvExtraInformation))
			{
				OutputDebugString("AgpmItem::OnReceive() Error (2) !!!\n");
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
	}

	if (pcsCharacter)
	{
		pstAgpdItem->m_ulCID = pcsCharacter->m_lID;
		pstAgpdItem->m_pcsCharacter = pcsCharacter;
	}

	if (m_pagpmFactors)
	{
		if (pFactor)
		{
			m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csFactor, pFactor, 0);
		}

		if (pFactorPercent)
			m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csFactorPercent, pFactorPercent, 0);

		if (pRestrictFactor)
			m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csRestrictFactor, pRestrictFactor, 0);

		if (pFactor || pFactorPercent || pRestrictFactor)
			EnumCallback(ITEM_CB_ID_UPDATE_FACTOR, pstAgpdItem, NULL);
	}

	pstAgpdItem->m_eNewStatus = cStatus;

	pstAgpdItem->m_lSkillTID	= lSkillTID;

	if (lStatusFlag != (-1))
		pstAgpdItem->m_lStatusFlag	= lStatusFlag;

	if (pOption)
	{
		pstAgpdItem->m_aunOptionTID.MemSetAll();
		pstAgpdItem->m_apcsOptionTemplate.MemSetAll();

		pstAgpdItem->m_aunOptionTIDRune.MemSetAll();
		pstAgpdItem->m_apcsOptionTemplateRune.MemSetAll();

		m_csPacketOption.GetField(FALSE, pOption, 0,
									&pstAgpdItem->m_aunOptionTID[0],
									&pstAgpdItem->m_aunOptionTID[1],
									&pstAgpdItem->m_aunOptionTID[2],
									&pstAgpdItem->m_aunOptionTID[3],
									&pstAgpdItem->m_aunOptionTID[4]);

		pstAgpdItem->m_apcsOptionTemplate[0]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[0]);
		pstAgpdItem->m_apcsOptionTemplate[1]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[1]);
		pstAgpdItem->m_apcsOptionTemplate[2]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[2]);
		pstAgpdItem->m_apcsOptionTemplate[3]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[3]);
		pstAgpdItem->m_apcsOptionTemplate[4]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[4]);
	}

	// 2007.02.05. steeple
	if (pSkillPlus)
	{
		pstAgpdItem->m_aunSkillPlusTID.MemSetAll();

		m_csPacketSkillPlus.GetField(FALSE, pSkillPlus, 0,
									&pstAgpdItem->m_aunSkillPlusTID[0],
									&pstAgpdItem->m_aunSkillPlusTID[1],
									&pstAgpdItem->m_aunSkillPlusTID[2]);
	}


	//	2005.11.30. By SungHoon
	if (pvCashInformation)
	{
		m_csPacketCashInformaion.GetField(FALSE, pvCashInformation, 0,
											&pstAgpdItem->m_nInUseItem, 
											&pstAgpdItem->m_lRemainTime,
											&pstAgpdItem->m_lExpireTime,
											&pstAgpdItem->m_lCashItemUseCount,
											&pstAgpdItem->m_llStaminaRemainTime);
	}

	if(pvExtraInformation)
	{
		PVOID strBuffer = 0;
		m_csPacketExtra.GetField(FALSE, pvExtraInformation, 0, &strBuffer);

		PACKET_AGPMITEM_EXTRA* pPacketExtra = (PACKET_AGPMITEM_EXTRA*)strBuffer;

		switch(pPacketExtra->Type)
		{
			case 1: // Seal Data
				{
					AgpdSealData pSealData;
					memcpy(&pSealData, &pPacketExtra->ExtraData.SealData, sizeof(AgpdSealData));

					EnumCallback( ITEM_CB_ID_ADD_ITEM_EXTRADATA, pstAgpdItem, &pSealData );
				} break;

			default :
				EnumCallback( ITEM_CB_ID_ADD_ITEM_EXTRADATA, pstAgpdItem, NULL );
				break;
		}
	}
	else
	{
		EnumCallback( ITEM_CB_ID_ADD_ITEM_EXTRADATA, pstAgpdItem, NULL );
	}

	if(!InitItem(pstAgpdItem))
	{
		OutputDebugString("AgpmItem::OnReceive() Error (3) !!!\n");

		return FALSE;
	}

	switch(cStatus)
	{
	case AGPDITEM_STATUS_FIELD:
		{
			ParseFieldPacket(pField, &pstAgpdItem->m_posItem);

			if(!AddItemToField(pstAgpdItem))
			{
				OutputDebugString("AgpmItem::OnReceive() Error (4) !!!\n");

				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_INVENTORY:
		{
			BOOL			bResult;

			if (!pcsCharacter)
				return FALSE;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				{
					if( AddItemToInventory(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
						ProcessRefreshCount( pcsCharacter, pstAgpdItem );
					}
					else
					{
						return FALSE;
					}
				}
			}
		}
		break;

	case AGPDITEM_STATUS_SUB_INVENTORY:
		{
			BOOL			bResult;

			if (!pcsCharacter)
				return FALSE;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				{
					if( AddItemToSubInventory(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
						ProcessRefreshCount( pcsCharacter, pstAgpdItem );
					}
					else
					{
						return FALSE;
					}
				}
			}
		}
		break;

	case AGPDITEM_STATUS_CHARGING_INVENTORY:
		{
			BOOL			bResult;

			if (!pcsCharacter)
				return FALSE;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				{
					if( AddItemToChargingInventory(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
						ProcessRefreshCount( pcsCharacter, pstAgpdItem );
					}
					else
					{
						return FALSE;
					}
				}
			}
		}
		break;

	case AGPDITEM_STATUS_BANK:
		{
			BOOL			bResult;

			if (!pcsCharacter)
				return FALSE;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pBank, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				{
					if( AddItemToBank(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
						ProcessRefreshCount( pcsCharacter, pstAgpdItem );
					}
					else
					{
						return FALSE;
					}
				}
			}
		}
		break;

	case AGPDITEM_STATUS_GUILD_WAREHOUSE:
		{
			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
				ProcessRefreshCount( pcsCharacter, pstAgpdItem );

				PVOID pvBuffer[3];
				pvBuffer[0] = &nIdx;
				pvBuffer[1] = &nRow;
				pvBuffer[2] = &nCol;
				EnumCallback(ITEM_CB_ID_ADD_ITEM_TO_GUILD_WAREHOUSE, pstAgpdItem, pvBuffer);
				ProcessRefreshCount( pcsCharacter, pstAgpdItem );
			}
		}
		break;

	case AGPDITEM_STATUS_QUEST:
		{
			BOOL			bResult;

			if (!pcsCharacter)
				return FALSE;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseQuestPacket(pQuest, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				if( AddItemToQuest(pcsCharacter, pstAgpdItem, &nIdx, &nRow, &nCol) )
				{
//						ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
//						ProcessRefreshCount( pcsCharacter, pstAgpdItem );
				}
				else
				{
					return FALSE;
				}
			}
		}
		break;

	case AGPDITEM_STATUS_TRADE_GRID:
		{
			if (!pcsCharacter)
				return FALSE;

			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				{
					if( AddItemToTradeGrid(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
						ProcessRefreshCount( pcsCharacter, pstAgpdItem );
					}
					else
					{
						return FALSE;
					}
				}
			}
		}
		break;

	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
		{
			if (!pcsCharacter)
				return FALSE;

			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				{
					if( AddItemToClientTradeGrid(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
						ProcessRefreshCount( pcsCharacter, pstAgpdItem );
					}
					else
					{
						return FALSE;
					}
				}
			}
		}
		break;

	case AGPDITEM_STATUS_SALESBOX_GRID:
		{
			if (!pcsCharacter)
				return FALSE;

			//ParseEquipPacket(pEquip, &lCID);

			if( ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol) )
			{
				if( AddItemToSalesBox( pcsCharacter, pstAgpdItem, nIdx, nRow, nCol ) )
				{
					ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
					ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );
				}
				else
				{
					return FALSE;
				}
			}
		}
		break;


	//Layer, Row, Column정보를 보내기는 하지만 실제로 쓰지는 않는다. 나중에 무결성체크용으로 사용할지도.....
	//여하간 우선은 패킷에 포함되서 날라오기는한다. -ashulam 추가-
	case AGPDITEM_STATUS_NPC_TRADE:
		{
			;
		}
		break;

	case AGPDITEM_STATUS_EQUIP:
		{
			if (!pcsCharacter)
				return FALSE;

			//ParseEquipPacket(pEquip);

			// 2007.11.07. steeple
			// 마지막에 Login Flag 를 bReceivedFromServer 로 주었다. 클라이언트에서는 이렇게 처리하게끔.
			if(EquipItem(pcsCharacter, pstAgpdItem, bReceivedFromServer ? FALSE : TRUE, bReceivedFromServer))
			{
				ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
				ProcessRefreshCount( pcsCharacter, pstAgpdItem );
			}
			else
			{
				OutputDebugString("AgpmItem::OnReceive() Error (6) !!!\n");

				return FALSE;
			}
		}
		break;
//	2005.11.16. By SungHoon 캐쉬 인벤토리에 추가한다.
	case AGPDITEM_STATUS_CASH_INVENTORY:
		{
			if (!pcsCharacter)
				return FALSE;
			if( !AddItemToCashInventory(pcsCharacter, pstAgpdItem) ) return FALSE;

			ProcessRefreshUI( pstAgpdItem->m_ulCID, cStatus, cStatus );
			ProcessRefreshCount( pcsCharacter, pstAgpdItem );
		}
		break;
	case AGPDITEM_STATUS_NOTSETTING:
		break;

	case AGPDITEM_STATUS_TRADE_OPTION_GRID:
		_AddPrivateTradeItem( lIID , pcsCharacter );
		break;

	default:
		{
			OutputDebugString("AgpmItem::OnReceive() Error (7) !!!\n");

			return FALSE;
		}
		break;
	}

	return TRUE;
}

BOOL AgpmItem::OnOperationRemove(INT8 cStatus, INT32 lIID, INT32 lTID, AgpdCharacter *pcsCharacter, PVOID pField, PVOID pInventory, PVOID pBank, PVOID pEquip, PVOID pFactor, PVOID pFactorPercent, PVOID pQuest)
{
	INT16		eStatus;

	if(!lIID)
	{
		OutputDebugString("AgpmItem::OnReceive() Error (30) !!!\n");
		return FALSE;
	}

	//pstAgpdItem = GetItemLock(lIID);
	AgpdItem *pstAgpdItem = GetItem(lIID);
	if(!pstAgpdItem)
	{
		// 뜰 수 있는 상황이 많다. 예를 들면, 해당 아이테을 가진 케릭터가 Remove됐을때.
//		OutputDebugString("AgpmItem::OnReceive() Error (31) !!!\n");
		return TRUE;
	}
	else
	{
		eStatus = pstAgpdItem->m_eStatus;
	}

	//ReleaseItem(pstAgpdItem);
	RemoveStatus(pstAgpdItem, AGPDITEM_STATUS_NONE);

//	임시코드	By SungHoon
//	실제로 지워 줘야 한다.
	if (eStatus == AGPDITEM_STATUS_INVENTORY)
	{
		RemoveItemFromCashInventory(pcsCharacter, pstAgpdItem->m_lTID);
		ProcessRefreshUI( pstAgpdItem->m_ulCID, AGPDITEM_STATUS_CASH_INVENTORY, AGPDITEM_STATUS_CASH_INVENTORY );
	}
//	여기까지
	else if (eStatus == AGPDITEM_STATUS_GUILD_WAREHOUSE)
	{
		EnumCallback(ITEM_CB_ID_REMOVE_ITEM_FROM_GUILD_WAREHOUSE, pstAgpdItem, NULL);
	}

	//아이템을 없앴으니 UI에서 지워주는 콜백 호출
	ProcessRefreshUI( pstAgpdItem->m_ulCID, eStatus, eStatus );
	ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );

	//pstAgpdItem->m_Mutex.Release();

	// Mutex Release는 DestroyItemData 내에서 이루어진다.
	return RemoveItem(lIID);
}

BOOL AgpmItem::OnOperationUpdate(BOOL bReceivedFromServer, INT8 cStatus, INT32 lIID, INT32 lTID, AgpdCharacter *pcsCharacter, INT32 lItemCount, PVOID pField, PVOID pInventory, PVOID pBank, PVOID pEquip, PVOID pFactor, PVOID pFactorPercent, PVOID pRestrictFactor, PVOID pConvert, PVOID pQuest, INT32 lSkillTID, INT32 lStatusFlag, PVOID pOption, PVOID pSkillPlus, PVOID pCashInformation, PVOID pvExtraInformation)
{
	if(!lIID)
	{
		OutputDebugString("AgpmItem::OnReceive() Error (10) !!!\n");
		return FALSE;
	}

	//AgpdItem *pstAgpdItem = GetItemLock(lIID);
	AgpdItem *pstAgpdItem = GetItem(lIID);

	if(!pstAgpdItem)
	{
		if (lTID)
		{
//		이게 문제임 Update 패킷에선 아이템정보가 전부다 내려오지 않음 여기서 강제로 Cash아이템을 만들수는 없음
//		2005.11.31 By SungHoon
			return OnOperationAdd(bReceivedFromServer, cStatus, lIID, lTID, pcsCharacter, lItemCount, pField, pInventory, pBank, pEquip, pFactor, pFactorPercent, pRestrictFactor, pConvert, pQuest, lSkillTID, lStatusFlag, pOption, pSkillPlus, pCashInformation, pvExtraInformation);
		}

		TRACE("AgpmItem::OnReceive() Error (11) !!! status=%d, id=%d\n", cStatus, lIID);
		return FALSE;
	}
/*	// 원래 없던 코드임 - CID 변경되는 경우 적용되지 않아 추가 - for PrivateTrade - 정현호 , 성연준
	else
	{
		if ( lCID != 0 )
			pstAgpdItem->m_ulCID = lCID;
	}*/

	BOOL	bRetval	= OnOperationUpdate(bReceivedFromServer, cStatus, pstAgpdItem, lTID, pcsCharacter, lItemCount, pField, pInventory, pBank, pEquip, pFactor, pFactorPercent, pRestrictFactor, pConvert, pQuest, lSkillTID, lStatusFlag, pOption, pSkillPlus, pCashInformation, pvExtraInformation);

	//pstAgpdItem->m_Mutex.Release();

	return bRetval;
}

BOOL AgpmItem::OnOperationUpdate(BOOL bReceivedFromServer, INT8 cStatus, AgpdItem *pstAgpdItem, INT32 lTID, AgpdCharacter *pcsCharacter, INT32 lItemCount, PVOID pField, PVOID pInventory, PVOID pBank, PVOID pEquip, PVOID pFactor, PVOID pFactorPercent, PVOID pRestrictFactor, PVOID pConvert, PVOID pQuest, INT32 lSkillTID, INT32 lStatusFlag, PVOID pOption, PVOID pSkillPlus, PVOID pCashInformation, PVOID pvExtraInformation)
{
	if (!pstAgpdItem)
		return FALSE;

	if (!bReceivedFromServer &&
		(pstAgpdItem->m_eStatus == AGPDITEM_STATUS_SALESBOX_GRID ||
		 pstAgpdItem->m_eStatus == AGPDITEM_STATUS_SALESBOX_BACKOUT ||
		 pstAgpdItem->m_eStatus == AGPDITEM_STATUS_GUILD_WAREHOUSE))
		return FALSE;

	// 1:1 거래 중 잠금 버튼이 눌린 이후론 Trade Grid에선 다른데로 못뺀다.
	if (!bReceivedFromServer &&
		pcsCharacter &&
		pstAgpdItem->m_eStatus == AGPDITEM_STATUS_TRADE_GRID &&
		cStatus != AGPDITEM_STATUS_TRADE_GRID)
	{
		AgpdItemADChar	*pcsAttachData	= GetADCharacter(pcsCharacter);
		if (pcsAttachData &&
			(pcsAttachData->m_lTradeStatus == AGPDITEM_TRADE_STATUS_LOCK ||
			 pcsAttachData->m_lTradeStatus == AGPDITEM_TRADE_STATUS_READY_TO_EXCHANGE))
		{
			return FALSE;
		}
	}

	// 클라이언트에선 맵에 추가되어 있는 상황이 아니면 아이템에 대해 어떤 행동도 할 수 없다.
	if (!bReceivedFromServer &&
		pstAgpdItem->m_pcsCharacter &&
		!pstAgpdItem->m_pcsCharacter->m_bIsAddMap)
		return FALSE;

	// 자기가 주인이 아닌 아이템을 어케 하려고 하는 놈이다. 불량스런 놈인 셈이다.
	if (!bReceivedFromServer &&
		pstAgpdItem->m_pcsCharacter &&
		pstAgpdItem->m_pcsCharacter != pcsCharacter)
	{
		return FALSE;
	}

	// Check possiblity to use a sub inventory 2008.06.24. steeple
	if (!bReceivedFromServer && 
		(pstAgpdItem->m_eStatus == AGPDITEM_STATUS_SUB_INVENTORY || cStatus == AGPDITEM_STATUS_SUB_INVENTORY) &&
		IsEnableSubInventory(pcsCharacter) == FALSE)
	{
		return FALSE;
	}


	// Check possibility to use a charging inventory 2012.07.03. Bischoff
	if (!bReceivedFromServer &&
		(pstAgpdItem->m_eStatus == AGPDITEM_STATUS_CHARGING_INVENTORY || cStatus == AGPDITEM_STATUS_CHARGING_INVENTORY) &&
		IsEnableChargingInventory(pcsCharacter) == FALSE)
	{
		return FALSE;
	}


	INT16       nIdx, nRow, nCol;
	INT8		cOldStatus;			//과거 위치를 저장했다가 갱신한다.

	AgpdItem	csBackupItem;
	csBackupItem.m_Mutex.Destroy();

	ZeroMemory(&csBackupItem, sizeof(AgpdItem));

	if(pField || pInventory || pBank || pQuest || pEquip || cStatus == AGPDITEM_STATUS_EQUIP)
	{
		if (!SaveItemStatus(pstAgpdItem, &csBackupItem))
			return FALSE;
	}

	if (bReceivedFromServer && m_pagpmFactors)
	{
		if (pFactor)
			m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csFactor, pFactor, 0);
		//m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csFactor, NULL, pFactor, 0);

		if (pFactorPercent)
			m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csFactorPercent, pFactor, 0);

		if (pRestrictFactor)
			m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csRestrictFactor, pRestrictFactor, 0);

		// 착용하고 있는 넘의 스탯이 바뀐경우 Factor 를 다쉬 계산해서 업데이트 시켜준다.
		if ((pstAgpdItem->m_eStatus == AGPDITEM_STATUS_EQUIP || pstAgpdItem->m_eStatus == AGPDITEM_STATUS_INVENTORY) &&
			(pFactor || pFactorPercent || pRestrictFactor))
			EnumCallback(ITEM_CB_ID_UPDATE_FACTOR, pstAgpdItem, NULL);
	}

	// Item을 Slot에 넣는 Sound를 Play위한 CB
	EnumCallback( ITEM_CB_ID_ITEM_SLOT_SOUND, pstAgpdItem, (PVOID)&cStatus );

	//Relase하기전에 저장한다. Update를 위해 저장한다.
	cOldStatus = (UINT8)pstAgpdItem->m_eStatus;

	if(pField || pInventory || pEquip || pBank || pQuest || cStatus == AGPDITEM_STATUS_EQUIP)
	{
		if (!CheckMovableItemBetweenGrid(pstAgpdItem, (AgpdItemStatus) cStatus))
			return FALSE;

		/*
		if (pstAgpdItem->m_eStatus != cStatus)
		{
			if(!ReleaseItem(pstAgpdItem)) // 전 상태를 해제한다.
			{
				OutputDebugString("AgpmItemashula::OnReceive() Error (12) !!!\n");
				return FALSE;
			}
		}
		*/
	}

	//Release하기전에 소유권을 바꾸면 안된다. Release하고 소유권이 바뀌어야한다. 2004.4.6 Ashulam
	if( bReceivedFromServer && pcsCharacter )
	{
		if (pstAgpdItem->m_ulCID != pcsCharacter->m_lID)
		{
			ChangeItemOwner(pstAgpdItem, pcsCharacter);
		}
	}

	if( bReceivedFromServer && lItemCount != (-1) )
	{
		SetItemStackCount(pstAgpdItem, lItemCount);
		ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );
	}

	if (bReceivedFromServer && lSkillTID != AP_INVALID_SKILLID)
		pstAgpdItem->m_lSkillTID	= lSkillTID;

	if (bReceivedFromServer && lStatusFlag != (-1))
	{
		pstAgpdItem->m_lStatusFlag	= lStatusFlag;
		UpdateItemStatusFlag(pstAgpdItem);
	}

	if (bReceivedFromServer && pOption)
	{
		pstAgpdItem->m_aunOptionTID.MemSetAll();
		pstAgpdItem->m_apcsOptionTemplate.MemSetAll();

		pstAgpdItem->m_aunOptionTIDRune.MemSetAll();
		pstAgpdItem->m_apcsOptionTemplateRune.MemSetAll();

		m_csPacketOption.GetField(FALSE, pOption, 0,
									&pstAgpdItem->m_aunOptionTID[0],
									&pstAgpdItem->m_aunOptionTID[1],
									&pstAgpdItem->m_aunOptionTID[2],
									&pstAgpdItem->m_aunOptionTID[3],
									&pstAgpdItem->m_aunOptionTID[4]);

		pstAgpdItem->m_apcsOptionTemplate[0]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[0]);
		pstAgpdItem->m_apcsOptionTemplate[1]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[1]);
		pstAgpdItem->m_apcsOptionTemplate[2]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[2]);
		pstAgpdItem->m_apcsOptionTemplate[3]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[3]);
		pstAgpdItem->m_apcsOptionTemplate[4]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[4]);
	}

	if (bReceivedFromServer && pSkillPlus)
	{
		pstAgpdItem->m_aunSkillPlusTID.MemSetAll();

		m_csPacketSkillPlus.GetField(FALSE, pSkillPlus, 0,
									&pstAgpdItem->m_aunSkillPlusTID[0],
									&pstAgpdItem->m_aunSkillPlusTID[1],
									&pstAgpdItem->m_aunSkillPlusTID[2]);
	}

	if( bReceivedFromServer && pvExtraInformation )
	{
		PVOID strBuffer = 0;
		m_csPacketExtra.GetField( FALSE, pvExtraInformation, 0, &strBuffer );

		PACKET_AGPMITEM_EXTRA* pPacketExtra = ( PACKET_AGPMITEM_EXTRA* )strBuffer;

		switch( pPacketExtra->Type )
		{
			case 1: // Seal Data
				{
					AgpdSealData pSealData;
					memcpy( &pSealData, &pPacketExtra->ExtraData.SealData, sizeof( AgpdSealData ) );

					EnumCallback( ITEM_CB_ID_ADD_ITEM_EXTRADATA, pstAgpdItem, &pSealData );
				} break;

			case 2:	//Bag Data
				{
					AgpdChargingBagData	ChargingBagData;
					memcpy( &ChargingBagData, &pPacketExtra->ExtraData.BagData, sizeof( AgpdChargingBagData ) );

				}break;

			default :
				EnumCallback( ITEM_CB_ID_ADD_ITEM_EXTRADATA, pstAgpdItem, NULL );
				break;
		}
	}
	else
	{
		EnumCallback( ITEM_CB_ID_ADD_ITEM_EXTRADATA, pstAgpdItem, NULL );
	}

	switch(cStatus)
	{
	case AGPDITEM_STATUS_FIELD:
		{
			if (!pField || !bReceivedFromServer)
				break;

			ParseFieldPacket(pField, &pstAgpdItem->m_posItem);
			
			if(!AddItemToField(pstAgpdItem))
			{
				//RestoreItemStatus(pstAgpdItem, &csBackupItem);

				OutputDebugString("AgpmItem::OnReceive() Error (13) !!!\n");
				return FALSE;
			}

			ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
			ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );
		}
		break;

	case AGPDITEM_STATUS_INVENTORY:
		{
			if (!pcsCharacter)
				return FALSE;

			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				if( UpdateItemInInventory(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
				{
					if (csBackupItem.m_eStatus == AGPDITEM_STATUS_EQUIP)
						EnumCallback(ITEM_CB_ID_REMOVE_FOR_NEAR_CHARACTER, pstAgpdItem, NULL);

					ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
					ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );

					if (cOldStatus == AGPDITEM_STATUS_BANK)
						EnumCallback(ITEM_CB_ID_REMOVE_ITEM_FROM_BANK, pstAgpdItem, NULL);
					else if (cOldStatus == AGPDITEM_STATUS_GUILD_WAREHOUSE)
						EnumCallback(ITEM_CB_ID_REMOVE_ITEM_FROM_GUILD_WAREHOUSE, pstAgpdItem, NULL);
				}
				else
				{
					return FALSE;
				}
			}

			ItemTimeUpdate( pstAgpdItem , pCashInformation );
		}
		break;

	case AGPDITEM_STATUS_SUB_INVENTORY:
		{
			if (!pcsCharacter)
				return FALSE;

			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				if( UpdateItemInSubInventory(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
				{
					if (csBackupItem.m_eStatus == AGPDITEM_STATUS_EQUIP)
						EnumCallback(ITEM_CB_ID_REMOVE_FOR_NEAR_CHARACTER, pstAgpdItem, NULL);

					ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
					ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );

					if (cOldStatus == AGPDITEM_STATUS_BANK)
						EnumCallback(ITEM_CB_ID_REMOVE_ITEM_FROM_BANK, pstAgpdItem, NULL);
					else if (cOldStatus == AGPDITEM_STATUS_GUILD_WAREHOUSE)
						EnumCallback(ITEM_CB_ID_REMOVE_ITEM_FROM_GUILD_WAREHOUSE, pstAgpdItem, NULL);
					else if (cOldStatus == AGPDITEM_STATUS_INVENTORY)
						EnumCallback(ITEM_CB_ID_CHAR_REMOVE_INVENTORY, pstAgpdItem, NULL);
				}
				else
				{
					return FALSE;
				}
			}

			ItemTimeUpdate( pstAgpdItem , pCashInformation );
		}
		break;

	case AGPDITEM_STATUS_CHARGING_INVENTORY:
		{
			BOOL			bResult;

			if (!pcsCharacter)
				return FALSE;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				{
					if( UpdateItemInChargingInventory(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						if (csBackupItem.m_eStatus == AGPDITEM_STATUS_EQUIP)
							EnumCallback(ITEM_CB_ID_REMOVE_FOR_NEAR_CHARACTER, pstAgpdItem, NULL);

						ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
						ProcessRefreshCount( pcsCharacter, pstAgpdItem );
					}
					else
					{
						return FALSE;
					}
				}
			}
		}
		break;

	case AGPDITEM_STATUS_TRADE_GRID:
		{
			if (!pcsCharacter)
				return FALSE;

			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				if( nIdx == -1 )
				{
					if( !AddItemToTradeGrid(pcsCharacter, pstAgpdItem, -1, -1, -1) )
					{
						//RestoreItemStatus(pstAgpdItem, &csBackupItem);
						return FALSE;
					}
				}
				else
				{
					//TradeGrid에 넣을때.
					if( AddItemToTradeGrid(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
						ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );
					}
					//
					else
					{
						//RestoreItemStatus(pstAgpdItem, &csBackupItem);
						return FALSE;
					}
				}
			}
		}
		break;

	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
		{
			if (!pcsCharacter)
				return FALSE;

			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				if( nIdx == -1 )
				{
					if( !AddItemToClientTradeGrid(pcsCharacter, pstAgpdItem, -1, -1, -1 ) )
					{
						//RestoreItemStatus(pstAgpdItem, &csBackupItem);
						return FALSE;
					}
				}
				else
				{
					if( AddItemToClientTradeGrid(pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
						ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );
					}
					else
					{
						//RestoreItemStatus(pstAgpdItem, &csBackupItem);
						return FALSE;
					}
				}
			}
		}
		break;

	case AGPDITEM_STATUS_BANK:
		{
			if (!pcsCharacter)
				return FALSE;

			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseBankPacket(pBank, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
/*				if( nIdx == -1 )
				{
					if( !UpdateBank( pstAgpdItem->m_ulCID, pstAgpdItem ) )
					{
						RestoreItemStatus(pstAgpdItem, &csBackupItem);
						return FALSE;
					}
				}
				else*/
				/*
				{
					if( !UpdateBank( pcsCharacter, pstAgpdItem, nIdx, nRow, nCol) )
					{
						RestoreItemStatus(pstAgpdItem, &csBackupItem);
						return FALSE;
					}
				}
				*/

				/*
				if( nIdx == -1 )
				{
				*/
					if( !UpdateItemInBank( pcsCharacter, pstAgpdItem, nIdx, nRow, nCol ) )
					{
						//RestoreItemStatus(pstAgpdItem, &csBackupItem);
						return FALSE;
					}

					ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
					ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );

					if (cOldStatus != AGPDITEM_STATUS_BANK)
						EnumCallback(ITEM_CB_ID_ADD_ITEM_TO_BANK, pstAgpdItem, NULL);
				/*
				}
				else
				{
					if( !AddItemToBank(pstAgpdItem->m_ulCID, pstAgpdItem, &nIdx, &nRow, &nCol) )
					{
						RestoreItemStatus(pstAgpdItem, &csBackupItem);
						return FALSE;
					}

					ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
					ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );
				}
				*/
			}
		}
		break;

	case AGPDITEM_STATUS_GUILD_WAREHOUSE:
		{
			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseBankPacket(pInventory, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				if (cOldStatus != AGPDITEM_STATUS_GUILD_WAREHOUSE)
				{
					RemoveStatus( pstAgpdItem, AGPDITEM_STATUS_GUILD_WAREHOUSE );
					PVOID pvBuffer[3];
					pvBuffer[0] = &nIdx;
					pvBuffer[1] = &nRow;
					pvBuffer[2] = &nCol;
					EnumCallback(ITEM_CB_ID_ADD_ITEM_TO_GUILD_WAREHOUSE, pstAgpdItem, pvBuffer);
				}

				ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
				ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );

			}
		}
		break;

	case AGPDITEM_STATUS_QUEST:
		{
			if (!pcsCharacter)
				return FALSE;

			BOOL			bResult;

			nIdx = -1;
			nRow = -1;
			nCol = -1;

			bResult = ParseQuestPacket(pQuest, &nIdx, &nRow, &nCol);

			if( bResult == FALSE )
			{
				break;
			}
			else
			{
				if( !UpdateQuest( pcsCharacter, pstAgpdItem, nIdx, nRow, nCol ) )
				{
					//RestoreItemStatus(pstAgpdItem, &csBackupItem);
					return FALSE;
				}

//				ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
//				ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );

//				if (cOldStatus != AGPDITEM_STATUS_QUEST)
//					EnumCallback(ITEM_CB_ID_ADD_ITEM_TO_BANK, pstAgpdItem, NULL);
			}			
		}
		break;

	case AGPDITEM_STATUS_SALESBOX_GRID:
		{


			if (!pcsCharacter)
				return FALSE;

			//ParseEquipPacket(pEquip, &lCID);

			if( ParseInventoryPacket(pInventory, &nIdx, &nRow, &nCol) )
			{
				if( AddItemToSalesBox( pcsCharacter, pstAgpdItem, nIdx, nRow, nCol ) )
				{
					ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
					ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );
				}
				else
				{
					//RestoreItemStatus(pstAgpdItem, &csBackupItem);
					return FALSE;
				}
			}
		}
		break;

	case AGPDITEM_STATUS_EQUIP:
		{
			if (!pcsCharacter)
				return FALSE;

			//ParseEquipPacket(pEquip, &lCID);

			// 2007.11.07. steeple
			// 마지막에 Login Flag 를 bReceivedFromServer 로 주었다. 클라이언트에서는 이렇게 처리하게끔.
			if(EquipItem(pcsCharacter, pstAgpdItem, bReceivedFromServer ? FALSE : TRUE , bReceivedFromServer))
			{
				ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
				ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );
			}
			else
			{
				//RestoreItemStatus(pstAgpdItem, &csBackupItem);
				OutputDebugString("AgpmItem::OnReceive() Error (17) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_NONE:
		{
			if (pstAgpdItem->m_eStatus != AGPDITEM_STATUS_NONE)
				//ReleaseItem(pstAgpdItem);
				RemoveStatus(pstAgpdItem, AGPDITEM_STATUS_NONE);
		}
		break;

	case AGPDITEM_STATUS_CASH_INVENTORY:
		{
			ItemTimeUpdate( pstAgpdItem , pCashInformation );

			if (!CheckUpdateStatus(pstAgpdItem, AGPDITEM_STATUS_CASH_INVENTORY, 0, 0, 0))
				return FALSE;

			if (pstAgpdItem->m_eStatus != AGPDITEM_STATUS_CASH_INVENTORY)
			{
				AddItemToCashInventory(pcsCharacter, pstAgpdItem);
			}

			ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
			ProcessRefreshCount( pstAgpdItem->m_ulCID, pstAgpdItem );
		}
		break;

	case AGPDITEM_STATUS_NOTSETTING:
	case AGPDITEM_STATUS_TEMP:
	case AGPDITEM_STATUS_NUM:
	case AGPDITEM_STATUS_SALESBOX_BACKOUT:
	case AGPDITEM_STATUS_NPC_TRADE:
	case AGPDITEM_STATUS_RECEIPE:
		{
			;	// NOP
		}
		break;

	case AGPDITEM_STATUS_SIEGEWAR_OBJECT:
		{
			RemoveStatus(pstAgpdItem, AGPDITEM_STATUS_SIEGEWAR_OBJECT);

			pstAgpdItem->m_eStatus	= AGPDITEM_STATUS_SIEGEWAR_OBJECT;

			ProcessRefreshUI( pstAgpdItem->m_ulCID, cOldStatus, cStatus );
		}
		break;

	default:
		{
			OutputDebugString("AgpmCharacter::OnReceive() Error (18) !!!\n");
			return FALSE;
		}
		break;
	}

//	if (pConvert)
//	{
//		OnOperationConvertItem(bReceivedFromServer, cStatus, pstAgpdItem->m_lID, pConvert, NULL);
//	}

	return TRUE;
}

BOOL AgpmItem::OnOperationUseItem(INT8 cStatus, INT32 lIID, INT32 lTID, AgpdCharacter *pcsCharacter, PVOID pField, PVOID pInventory, PVOID pBank, PVOID pEquip, PVOID pFactor, PVOID pFactorPercent, INT32 lTargetID)
{
	if(!lIID || !pcsCharacter)
	{
		OutputDebugString("AgpmItem::OnReceive() Error (30) !!!\n");
		return FALSE;
	}

			
	//AgpdItem	*pcsItem			= GetItemLock(lIID);
	AgpdItem	*pcsItem			= GetItem(lIID);
	if (!pcsItem) return FALSE;
	if (pcsItem->m_eStatus != AGPDITEM_STATUS_INVENTORY &&
		pcsItem->m_eStatus != AGPDITEM_STATUS_CASH_INVENTORY &&
		pcsItem->m_eStatus != AGPDITEM_STATUS_SUB_INVENTORY &&
		pcsItem->m_eStatus != AGPDITEM_STATUS_CHARGING_INVENTORY )
		return FALSE;

	if (!pcsItem->m_pcsItemTemplate) return FALSE;

	// Before Process Using Item Packet Check Using item disturbing
	if(m_pagpmCharacter->CheckEnableActionCharacter(pcsCharacter, AGPDCHAR_DISTURB_ACTION_USE_ITEM) == FALSE)
	{
		//아이템 사용 못하게되는 히로익에 걸렸을경우에도 보호귀환문서는 사용 할수 있어야 한다.
		if( (((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType != AGPMITEM_USABLE_TYPE_TELEPORT_SCROLL) ||
			( AGPMITEM_USABLE_TELEPORT_SCROLL_GO_TOWN_NOW != ((AgpdItemTemplateUsableTeleportScroll *) pcsItem->m_pcsItemTemplate)->m_eTeleportScrollType ) )
				return FALSE;
	}


	// 2005.09.29. steeple
	// 소유주 체크
    if(pcsCharacter && pcsItem->m_pcsCharacter)
	{
		if(pcsCharacter->m_lID != pcsItem->m_pcsCharacter->m_lID)
			return FALSE;
	}

	// Check possiblity to use a sub inventory 2008.06.24. steeple
	if (pcsItem->m_eStatus == AGPDITEM_STATUS_SUB_INVENTORY && IsEnableSubInventory(pcsCharacter) == FALSE)
	{
		return FALSE;
	}

	// Check possibility to use a charging inventory 2012.07.03. Bischoff
	if (pcsItem->m_eStatus == AGPDITEM_STATUS_CHARGING_INVENTORY && IsEnableChargingInventory(pcsCharacter) == FALSE)
	{
		return FALSE;
	}

	AgpdCharacter	*pcsTargetChar	= NULL;
	
	if (lTargetID != AP_INVALID_IID &&
		pcsCharacter->m_lID != lTargetID)
	{
		pcsTargetChar	= m_pagpmCharacter->GetCharacterLock(lTargetID);
		if (pcsCharacter->m_Mutex.IsRemoveLock())
		{
			if (pcsTargetChar)
				pcsTargetChar->m_Mutex.Release();
			return FALSE;
		}
	}
	else
		pcsTargetChar	= pcsCharacter;
	//AgpdItem	*pcsTargetItem		= GetItemLock(lTargetID);
//	AgpdItem	*pcsTargetItem		= GetItem(lTargetID);

	if (m_pagpmCharacter->IsAllBlockStatus(pcsTargetChar))
	{
		if (pcsTargetChar && pcsTargetChar != pcsCharacter)
			pcsTargetChar->m_Mutex.Release();

		return FALSE;
	}

	// target 이 있는 경우 현재는 아이템 개조밖에 없다. 고로.. 아템 개조와 사용으로 나눠서 함수를 호출해준다.
	//
	BOOL	bRetval = TRUE;

//	if (pcsTargetItem)
//		bRetval = ConvertItem(pcsTargetItem, pcsItem);
//	else
		bRetval = UseItem(pcsItem, pcsTargetChar);

	//pcsItem->m_Mutex.Release();
	//if (pcsTargetItem)
	//{
	//	pcsTargetItem->m_Mutex.Release();
	//}

	if (pcsTargetChar && pcsTargetChar != pcsCharacter)
		pcsTargetChar->m_Mutex.Release();

	return bRetval;
}

BOOL AgpmItem::OnOperationUseItemByTID(INT32 lTID, AgpdCharacter *pcsCharacter, INT32 lTargetID, UINT32 ulUseInterval)
{
	if (lTID == AP_INVALID_IID ||
		!pcsCharacter)
		return FALSE;

	AgpdCharacter	*pcsTargetChar	= NULL;
	
	if (lTargetID != AP_INVALID_IID &&
		pcsCharacter->m_lID != lTargetID)
		pcsTargetChar	= m_pagpmCharacter->GetCharacterLock(lTargetID);
	else
		pcsTargetChar	= pcsCharacter;

	if (m_pagpmCharacter->IsAllBlockStatus(pcsTargetChar))
	{
		if (pcsTargetChar && pcsTargetChar != pcsCharacter)
			pcsTargetChar->m_Mutex.Release();

		return FALSE;
	}

	PVOID	pvBuffer[2];
	pvBuffer[0]	= IntToPtr(lTID);
	pvBuffer[1]	= (PVOID) pcsTargetChar;

	EnumCallback(ITEM_CB_ID_USE_ITEM_BY_TID, pcsCharacter, pvBuffer);

	// Cooldown 함수 한번 불러준다. 2008.02.15. steeple
	OnOperationUpdateCooldown(pcsCharacter, lTID, ulUseInterval, 0);

	if (pcsTargetChar && pcsTargetChar != pcsCharacter)
		pcsTargetChar->m_Mutex.Release();

	return TRUE;
}

BOOL AgpmItem::OnOperationUseItemFailedByTID(INT32 lTID, AgpdCharacter *pcsCharacter, INT32 lTargetID)
{
	if (lTID == AP_INVALID_IID ||
		!pcsCharacter)
		return FALSE;

	AgpdCharacter	*pcsTargetChar	= NULL;
	
	if (lTargetID != AP_INVALID_IID &&
		pcsCharacter->m_lID != lTargetID)
		pcsTargetChar	= m_pagpmCharacter->GetCharacterLock(lTargetID);
	else
		pcsTargetChar	= pcsCharacter;

	PVOID	pvBuffer[2];
	pvBuffer[0]	= IntToPtr(lTID);
	pvBuffer[1]	= (PVOID) pcsTargetChar;

	EnumCallback(ITEM_CB_ID_USE_ITEM_FAILED_BY_TID, pcsCharacter, pvBuffer);

	if (pcsTargetChar && pcsTargetChar != pcsCharacter)
		pcsTargetChar->m_Mutex.Release();

	return TRUE;
}

BOOL AgpmItem::OnOperationUpdateReturnScrollStatus(AgpdCharacter *pcsCharacter, BOOL bIsEnable)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return FALSE;

	if (bIsEnable)
	{
		pcsItemADChar->m_bUseReturnTeleportScroll	= TRUE;
	}
	else
	{
		pcsItemADChar->m_bUseReturnTeleportScroll	= FALSE;
	}

	return EnumCallback(ITEM_CB_ID_UPDATE_RETURN_TELEPORT_STATUS, pcsCharacter, NULL);
}

BOOL AgpmItem::OnOperationCancelReturnScroll(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return FALSE;

	pcsItemADChar->m_bUseReturnTeleportScroll	= FALSE;

	return TRUE;
}

BOOL AgpmItem::OnOperationUseReturnScroll(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar		*pcsItemADChar		= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return FALSE;

	if (!pcsItemADChar->m_bUseReturnTeleportScroll)
		return FALSE;

	return EnumCallback(ITEM_CB_ID_USE_RETURN_SCROLL, pcsCharacter, NULL);
}

BOOL AgpmItem::OnOperationUseReturnScrollResultFailed(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	return EnumCallback(ITEM_CB_ID_USE_RETURN_SCROLL_RESULT_FAILED, pcsCharacter, NULL);
}

BOOL AgpmItem::OnOperationPickupItem(INT8 cStatus, INT32 lIID, AgpdCharacter *pcsCharacter)
{
	if (lIID == AP_INVALID_IID || !pcsCharacter)
		return FALSE;

	// 땅에 있는 아템을 집어들은 경우다.
	//AgpdItem		*pcsItem		= GetItemLock(lIID);

	AgpdItem		*pcsItem		= GetItemLock(lIID);
	if (!pcsItem)
	{
		return FALSE;
	}

	BOOL			bIsRemoveItem	= FALSE;

	// 집어들 수 있는 거리안에 있는지 살펴본다.
	AuPOS	stMovePos;
	if (!m_pagpmFactors->IsInRange(&pcsCharacter->m_stPos, &pcsItem->m_posItem, AGPMITEM_MAX_PICKUP_ITEM_DISTANCE, 0, &stMovePos))
	{
		// 너무 멀면 집지 않는다.
		FLOAT				fx = pcsCharacter->m_stPos.x - stMovePos.x;
		FLOAT				fz = pcsCharacter->m_stPos.z - stMovePos.z;

		FLOAT				fDistance = (FLOAT) sqrt(fx * fx + fz * fz);

		if (fDistance > (FLOAT) (AGPMITEM_MAX_PICKUP_ITEM_DISTANCE * 5))
		{
			pcsItem->m_Mutex.Release();
			return TRUE;
		}

		pcsCharacter->m_stNextAction.m_eActionType			= AGPDCHAR_ACTION_TYPE_PICKUP_ITEM;
		pcsCharacter->m_stNextAction.m_csTargetBase.m_eType	= pcsItem->m_eType;
		pcsCharacter->m_stNextAction.m_csTargetBase.m_lID	= pcsItem->m_lID;

		pcsItem->m_Mutex.Release();

		return m_pagpmCharacter->MoveCharacter(pcsCharacter, &stMovePos, MD_NODIRECTION, FALSE, TRUE, FALSE, TRUE);
	}
	else
	{
		pcsCharacter->m_stNextAction.m_eActionType	= AGPDCHAR_ACTION_TYPE_NONE;

		if (pcsCharacter->m_bMove)
			m_pagpmCharacter->StopCharacter(pcsCharacter, NULL);

		PickupItem(pcsItem, pcsCharacter->m_lID, &bIsRemoveItem);
	}

	if (!bIsRemoveItem)
		pcsItem->m_Mutex.Release();

	return TRUE;
}

BOOL AgpmItem::OnOperationPickupItemResult(INT8 cStatus, AgpdCharacter *pcsCharacter, INT32 lIID, INT32 lTID, INT32 lItemCount)
{
	if (lTID == AP_INVALID_IID)
		return FALSE;

	AgpdItemTemplate	*pcsItemTemplate	= GetItemTemplate(lTID);
	if (!pcsItemTemplate)
		return FALSE;

	PVOID	pvBuffer[5];
	pvBuffer[0]	= (PVOID) pcsItemTemplate;
	pvBuffer[1]	= IntToPtr(lItemCount);
	pvBuffer[2] = IntToPtr(lIID);
	pvBuffer[3] = NULL;
	pvBuffer[4] = NULL;

	EnumCallback(ITEM_CB_ID_PICKUP_ITEM_RESULT, &cStatus, pvBuffer);

	return TRUE;
}

BOOL AgpmItem::OnOperationStackItem( INT8 cStatus, INT32 lIID, AgpdCharacter *pcsAgpdCharacter, void *pInventory, void *pBank, void *pQuest )
{
	if (!pcsAgpdCharacter)
		return FALSE;

	BOOL	bResult = FALSE;

	//AgpdItem	*pcsAgpdItem	= GetItemLock(lIID);
	AgpdItem	*pcsAgpdItem	= GetItem(lIID);
	if (!pcsAgpdItem)
		return FALSE;

	//인벤에서 Stack이라면...
	if( cStatus == AGPDITEM_STATUS_INVENTORY )
	{
		if( (pInventory != NULL))
		{
			INT16			nLayer, nRow, nColumn;

			if( ParseInventoryPacket( pInventory, &nLayer, &nRow, &nColumn) )
			{
				bResult = UpdateItemInInventory( pcsAgpdCharacter, pcsAgpdItem, nLayer, nRow, nColumn );
			}
		}
	}
	//Bank에서 Stack이라면....
	else if(cStatus == AGPDITEM_STATUS_BANK )
	{
		if( (pBank != NULL))
		{
			INT16			nLayer, nRow, nColumn;

			if( ParseInventoryPacket( pBank, &nLayer, &nRow, &nColumn) )
			{
				bResult = UpdateItemInBank( pcsAgpdCharacter, pcsAgpdItem, nLayer, nRow, nColumn );
			}
		}
	}
	// stack in a sub inventory
	else if(cStatus == AGPDITEM_STATUS_SUB_INVENTORY)
	{
		if( (pInventory != NULL) && IsEnableSubInventory(pcsAgpdCharacter) )
		{
			INT16			nLayer, nRow, nColumn;

			if( ParseInventoryPacket( pInventory, &nLayer, &nRow, &nColumn) )
			{
				bResult = UpdateItemInSubInventory( pcsAgpdCharacter, pcsAgpdItem, nLayer, nRow, nColumn );
			}
		}
	}
	// stack in a charging inventory
	else if(cStatus == AGPDITEM_STATUS_CHARGING_INVENTORY)
	{
		if( (pInventory != NULL) && IsEnableChargingInventory(pcsAgpdCharacter) )
		{
			INT16			nLayer, nRow, nColumn;

			if( ParseInventoryPacket( pInventory, &nLayer, &nRow, &nColumn) )
			{
				bResult = UpdateItemInChargingInventory( pcsAgpdCharacter, pcsAgpdItem, nLayer, nRow, nColumn );
			}
		}
	}

	return bResult;
}

BOOL AgpmItem::OnOperationDropMoney( AgpdCharacter *pcsAgpdCharacter, INT32 lItemCount )
{
	if (!pcsAgpdCharacter)
		return FALSE;

	return EnumCallback( ITEM_CB_ID_DROP_MONEY_TO_FIELD, pcsAgpdCharacter, &lItemCount );
}

BOOL AgpmItem::OnOperationUseItemSuccess(AgpdCharacter *pcsCharacter, INT32 lIID)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItem	*pcsItem	= GetItem(lIID);
	if (!pcsItem)
		return FALSE;

	return EnumCallback(ITEM_CB_ID_USE_ITEM_SUCCESS, pcsCharacter, pcsItem);
}

BOOL AgpmItem::OnOperationSplitItem(AgpdCharacter *pcsCharacter, INT32 lIID, INT32 lSplitStackCount, INT8 cStatus, PVOID pField, PVOID pInventory)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItem	*pcsItem	= GetItem(lIID);
	if (!pcsItem)
		return FALSE;

	if (cStatus == (INT8) AGPDITEM_STATUS_FIELD)
	{
		if (!pField)
			return FALSE;

		AuPOS	stTargetPos	= {0,0,0};

		ParseFieldPacket(pField, &stTargetPos);

		if (!SplitItem(pcsItem, lSplitStackCount, &stTargetPos))
			return FALSE;
	}
	else
	{
		if (!pInventory)
			return FALSE;

		INT16	nLayer	= -1;
		INT16	nRow	= -1;
		INT16	nColumn	= -1;

		ParseInventoryPacket(pInventory, &nLayer, &nRow, &nColumn);

		if (!SplitItem(pcsItem, lSplitStackCount, (AgpdItemStatus) cStatus, nLayer, nRow, nColumn))
			return FALSE;
	}

	return TRUE;
}

BOOL AgpmItem::OnOperationRequestDestroyItem(AgpdCharacter *pcsCharacter, INT32 lIID)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItem	*pcsItem	= GetItem(lIID);
	if (!pcsItem 
		|| !pcsItem->m_pcsCharacter 
		|| pcsItem->m_pcsCharacter != pcsCharacter 
		|| (pcsItem->m_eStatus != AGPDITEM_STATUS_INVENTORY && pcsItem->m_eStatus != AGPDITEM_STATUS_CASH_INVENTORY && pcsItem->m_eStatus != AGPDITEM_STATUS_SUB_INVENTORY && pcsItem->m_eStatus != AGPDITEM_STATUS_CHARGING_INVENTORY )
		|| pcsItem->m_nInUseItem != AGPDITEM_CASH_ITEM_UNUSE)
		return FALSE;

	ZeroMemory(pcsItem->m_szDeleteReason, sizeof(pcsItem->m_szDeleteReason));
	sprintf(pcsItem->m_szDeleteReason, "%s", "Throw wastebasket");

	return RemoveItem(pcsItem, TRUE);
}

BOOL AgpmItem::UpdateReverseOrbReuseTime(AgpdCharacter *pcsCharacter, UINT32 ulReuseTime)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar	*pcsAttachItemData		= GetADCharacter(pcsCharacter);

// 	AgpdItemTemplate	*pcsItemTemplate	= GetItemTemplate(1054);
// 	if (pcsItemTemplate)
// 		pcsAttachItemData->m_ulUseReverseOrbTime	= GetClockCount() - (((AgpdItemTemplateUsable *) pcsItemTemplate)->m_ulUseInterval - ulReuseTime);

	pcsAttachItemData->m_ulUseReverseOrbTime	= GetClockCount() - ( AGPMITEM_REVERSE_ORB_REUSE_INTERVAL - ulReuseTime);

	EnumCallback(ITEM_CB_ID_UPDATE_REUSETIME_FOR_REVERSE_ORB, pcsCharacter, NULL);

	return TRUE;
}

BOOL AgpmItem::UpdateTransformReuseTime(AgpdCharacter *pcsCharacter, UINT32 ulReuseTime)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar	*pcsAttachItemData		= GetADCharacter(pcsCharacter);

	pcsAttachItemData->m_ulUseTransformTime	= GetClockCount() - (AGPMITEM_TRANSFORM_RECAST_TIME - ulReuseTime);

	EnumCallback(ITEM_CB_ID_UPDATE_REUSETIME_FOR_TRANSFORM, pcsCharacter, NULL);

	return TRUE;
}

// 2006.12.14. steeple
BOOL AgpmItem::InitTransformReuseTime(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar	*pcsAttachItemData		= GetADCharacter(pcsCharacter);

	pcsAttachItemData->m_ulUseTransformTime	= 0;

	EnumCallback(ITEM_CB_ID_INIT_REUSETIME_FOR_TRANSFORM, pcsCharacter, NULL);

	return TRUE;
}

// 2008.02.12. steeple
// 인벤토리와, 캐쉬 인벤토리에서 현재 쿨다운이 진행중인 녀석이 있는지 체크한다.
BOOL AgpmItem::IsProgressingCooldown(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return FALSE;

	AgpdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	// 현재 진행중인 cooldown 이 있으면 return TRUE
	return !pcsItemADChar->m_CooldownInfo.m_pBases->empty();
}

// 2008.02.12. steeple
// 쿨다운이 진행 중인 모든 아이템의 쿨다운의 시간을 차감해준다.
INT32 AgpmItem::ProcessAllItemCooldown(AgpdCharacter* pcsCharacter, UINT32 ulElapsedClockCount)
{
	if(!pcsCharacter)
		return 0;

	AgpdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if(!pcsItemADChar || pcsItemADChar->m_CooldownInfo.m_pBases->empty())
		return 0;

	INT32 lCount = 0;

	AgpdItemCooldownArray::CooldownBaseIter iter = pcsItemADChar->m_CooldownInfo.m_pBases->begin();
	while(iter != pcsItemADChar->m_CooldownInfo.m_pBases->end())
	{
		AgpdItemTemplate* pcsItemTemplate = GetItemTemplate(iter->m_lTID);
		if(!pcsItemTemplate)
		{
			++iter;
			continue;
		}

		// 캐쉬템, 피씨방 인 애들은 상황을 체크해준다.
		if(iter->m_bPause == FALSE &&
			IS_CASH_ITEM(pcsItemTemplate->m_eCashItemType) &&
			((pcsItemTemplate->m_lIsUseOnlyPCBang & AGPMITEM_PCBANG_TYPE_USE_ONLY) || (pcsItemTemplate->m_lIsUseOnlyPCBang & AGPMITEM_PCBANG_TYPE_USE_ONLY_GPREMIUM))
			)
		{
			AgpdGridItem* pcsGridItem = GetCashItemByTID(pcsCharacter, pcsItemTemplate->m_lID);
			if(pcsGridItem)
			{
				AgpdItem* pcsItem = GetItem(pcsGridItem);
				
				// 사용중이 아닌 경우, 유지시간이 다 완료되기 전에 끝난 경우에는 Pause 로 처리해야 한다.
				if(pcsItem && pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_UNUSE &&
					CheckReuseIntervalWithCooldown(pcsItem, iter->m_ulRemainTime, ulElapsedClockCount) == FALSE)
				{
					iter->m_bPause = TRUE;
				}
			}
		}

		// 정지중인 녀석들은 그냥 콜백만 부른다.
		if(iter->m_bPause)
		{
			CallCooldownCallback(pcsCharacter, *iter);
			++iter;
			continue;
		}

		++lCount;

		if(iter->m_ulRemainTime <= ulElapsedClockCount)
		{
			iter->m_ulRemainTime = 0;
			CallCooldownCallback(pcsCharacter, *iter);
			iter = pcsItemADChar->m_CooldownInfo.m_pBases->erase(iter);
			continue;
		}

		iter->m_ulRemainTime -= ulElapsedClockCount;
		CallCooldownCallback(pcsCharacter, *iter);

		++iter;
	}

	return lCount;
}

// 2008.02.13. steeple
BOOL AgpmItem::SetCooldownByTID(AgpdCharacter* pcsCharacter, INT32 lTID, UINT32 ulRemainTime, BOOL bPause)
{
	if(!pcsCharacter || lTID < 1)
		return FALSE;

	AgpdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	AgpdItemCooldownArray::CooldownBaseIter iter = std::find(
													pcsItemADChar->m_CooldownInfo.m_pBases->begin(),
													pcsItemADChar->m_CooldownInfo.m_pBases->end(),
													lTID);

	// ulRemainTime 이 0 으로 온 경우에는 쿨다운 벡터에서 지워준다.
	if(ulRemainTime == 0)
	{
		if(iter != pcsItemADChar->m_CooldownInfo.m_pBases->end())
			pcsItemADChar->m_CooldownInfo.m_pBases->erase(iter);

		return TRUE;
	}

	// 여기서 부터는 업데이트나 새로 추가하는 것임.
	if(iter != pcsItemADChar->m_CooldownInfo.m_pBases->end())
	{
		iter->m_ulRemainTime = ulRemainTime;
		iter->m_bPause = bPause;
	}
	else
	{
		AgpdItemCooldownBase csBase;
		csBase.m_lTID = lTID;
		csBase.m_ulRemainTime = ulRemainTime;
		csBase.m_bPause = bPause;

		pcsItemADChar->m_CooldownInfo.m_pBases->push_back(csBase);
	}

	return TRUE;
}

// 2008.02.20. steeple
BOOL AgpmItem::SetCooldownPause(AgpdCharacter* pcsCharacter, INT32 lTID, BOOL bPause)
{
	if(!pcsCharacter || lTID < 1)
		return FALSE;

	AgpdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	AgpdItemCooldownArray::CooldownBaseIter iter = std::find(
													pcsItemADChar->m_CooldownInfo.m_pBases->begin(),
													pcsItemADChar->m_CooldownInfo.m_pBases->end(),
													lTID);

	if(iter != pcsItemADChar->m_CooldownInfo.m_pBases->end())
	{
		iter->m_bPause = bPause;
	}

	return TRUE;
}

// 2008.02.14. steeple
UINT32 AgpmItem::GetRemainTimeByTID(AgpdCharacter* pcsCharacter, INT32 lTID)
{
	if(!pcsCharacter || lTID < 1)
		return 0;

	AgpdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return 0;

	AgpdItemCooldownArray::CooldownBaseIter iter = std::find(
													pcsItemADChar->m_CooldownInfo.m_pBases->begin(),
													pcsItemADChar->m_CooldownInfo.m_pBases->end(),
													lTID);
	if(iter != pcsItemADChar->m_CooldownInfo.m_pBases->end())
		return iter->m_ulRemainTime;

	return 0;
}

// 2008.02.21. steeple
BOOL AgpmItem::GetCooldownPause(AgpdCharacter* pcsCharacter, INT32 lTID)
{
	if(!pcsCharacter || lTID < 1)
		return FALSE;

	AgpdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return FALSE;

	AgpdItemCooldownArray::CooldownBaseIter iter = std::find(
													pcsItemADChar->m_CooldownInfo.m_pBases->begin(),
													pcsItemADChar->m_CooldownInfo.m_pBases->end(),
													lTID);
	if(iter != pcsItemADChar->m_CooldownInfo.m_pBases->end())
		return iter->m_bPause;

	return FALSE;
}

// 2008.02.21. steeple
AgpdItemCooldownBase AgpmItem::GetCooldownBase(AgpdCharacter* pcsCharacter, INT32 lTID)
{
	AgpdItemCooldownBase stBase;

	if(!pcsCharacter || lTID < 1)
		return stBase;

	AgpdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if(!pcsItemADChar)
		return stBase;

	AgpdItemCooldownArray::CooldownBaseIter iter = std::find(
													pcsItemADChar->m_CooldownInfo.m_pBases->begin(),
													pcsItemADChar->m_CooldownInfo.m_pBases->end(),
													lTID);
	if(iter != pcsItemADChar->m_CooldownInfo.m_pBases->end())
		return *iter;

	return stBase;
}

// 2008.02.15. steeple
BOOL AgpmItem::CallCooldownCallback(AgpdCharacter* pcsCharacter, AgpdItemCooldownBase stCooldownBase)
{
	if(!pcsCharacter || stCooldownBase.m_lTID < 1)
		return FALSE;

	EnumCallback(ITEM_CB_ID_UPDATE_COOLDOWN, pcsCharacter, &stCooldownBase);

	return TRUE;
}


/*
BOOL AgpmItem::CBReCalcFactor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmItem		*pThis			= (AgpmItem *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	AgpdItemADChar	*pcsItemADChar	= pThis->GetADCharacter(pcsCharacter);

	AgpdFactor		*pcsResultFactor	= (AgpdFactor *) pThis->m_pagpmFactors->GetFactor(&pcsCharacter->m_csFactor, AGPM_FACTORS_TYPE_RESULT);

	for (int i = 0; i < AGPMITEM_PART_NUM; i++)
	{
		if (pcsItemADChar->m_lEquipSlot[i] == AP_INVALID_IID)
			continue;

		AgpdItem	*pcsItem = pThis->GetItem(pcsItemADChar->m_lEquipSlot[i]);
		if (!pcsItem)
			continue;

		pThis->m_pagpmFactors->AddResultFactor(pcsResultFactor, &pcsItem->m_csFactor);
	}

	return TRUE;
}
*/

//		MakePacketItem
//	Functions
//		- pcsItem에 대한 정보(detail info)를 패킷으로 만들어 리턴한다.
//			주의) 리턴된 패킷은 사용후 꼬옥~~~ FreePacket() 해줘야 한다.
//	Arguments
//		- pcsItem : 아템 데이타 포인터
//		- pnPacketLength : 만들어진 패킷 길이
//		- eStatus : 스테이터스를 바꿔서 보내고 싶을때 따로 추가한다.(1:1거래할때 필요함)
//	Return value
//		- PVOID : 만든 패킷 포인터
///////////////////////////////////////////////////////////////////////////////
PVOID AgpmItem::MakePacketItem(AgpdItem *pcsItem, INT16	*pnPacketLength )
{
	if (!pcsItem)
		return NULL;

	INT8	nOperation = AGPMITEM_PACKET_OPERATION_ADD;

	PVOID pvFieldPacket = NULL;
	PVOID pvInventoryPacket = NULL;
	PVOID pvBankPacket = NULL;
	PVOID pvEquipPacket = NULL;
	PVOID pvPacketFactor = NULL;
	PVOID pvPacketFactorPercent = NULL;
	PVOID pvPacketRestrictFactor = NULL;
	PVOID pvPacketQuest	= NULL;
	PVOID pvPacketCashInformation = NULL;
	PVOID pvPacketExtra = NULL;

	switch (pcsItem->m_eStatus) {
	case AGPDITEM_STATUS_FIELD:
		pvFieldPacket = m_csPacketField.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE,
												&pcsItem->m_posItem
												);
		break;

	//인벤, 거래창의 위치정보를 나타내는 패킷은 같다.
	case AGPDITEM_STATUS_INVENTORY:
	case AGPDITEM_STATUS_TRADE_GRID:
	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
	case AGPDITEM_STATUS_NPC_TRADE:
	case AGPDITEM_STATUS_SALESBOX_GRID:
	case AGPDITEM_STATUS_GUILD_WAREHOUSE:
	case AGPDITEM_STATUS_SUB_INVENTORY:
	case AGPDITEM_STATUS_CHARGING_INVENTORY:
		pvInventoryPacket = m_csPacketInventory.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE, 
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_TAB],
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_ROW],
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN]);
		break;
	case AGPDITEM_STATUS_BANK:
		pvBankPacket = m_csPacketBank.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE, 
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_TAB],
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_ROW],
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN]);
		break;

	case AGPDITEM_STATUS_QUEST:
		pvPacketQuest = m_csPacketQuest.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE, 
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_TAB],
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_ROW],
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN]);

	case AGPDITEM_STATUS_EQUIP:
		pvEquipPacket = m_csPacketEquip.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE
												);

	case AGPDITEM_STATUS_TRADE_OPTION_GRID:


		break;
	}

	if (m_pagpmFactors)
	{
		pvPacketFactor = m_pagpmFactors->MakePacketFactorDiffer(&pcsItem->m_csFactor, &((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_csFactor);
		pvPacketFactorPercent = m_pagpmFactors->MakePacketFactors(&pcsItem->m_csFactorPercent);
		pvPacketRestrictFactor = m_pagpmFactors->MakePacketFactorDiffer(&pcsItem->m_csRestrictFactor, &((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_csRestrictFactor);
	}

	PVOID	pvPacketConvert = NULL;
//	if (pcsItem->m_stConvertHistory.lConvertLevel > 0)
//	{
//		pvPacketConvert = MakePacketItemConvertOnly(pcsItem, NULL);
//	}

	PVOID	pvPacketOption	= NULL;

	pvPacketOption	= m_csPacketOption.MakePacket(FALSE, pnPacketLength, 0,
													(pcsItem->m_aunOptionTID[0] != 0) ? &pcsItem->m_aunOptionTID[0] : NULL,
													(pcsItem->m_aunOptionTID[1] != 0) ? &pcsItem->m_aunOptionTID[1] : NULL,
													(pcsItem->m_aunOptionTID[2] != 0) ? &pcsItem->m_aunOptionTID[2] : NULL,
													(pcsItem->m_aunOptionTID[3] != 0) ? &pcsItem->m_aunOptionTID[3] : NULL,
													(pcsItem->m_aunOptionTID[4] != 0) ? &pcsItem->m_aunOptionTID[4] : NULL);

	PVOID	pvPacketSkillPlus = NULL;

	pvPacketSkillPlus = m_csPacketSkillPlus.MakePacket(FALSE, pnPacketLength, 0,
													(pcsItem->m_aunSkillPlusTID[0] != 0) ? &pcsItem->m_aunSkillPlusTID[0] : NULL,
													(pcsItem->m_aunSkillPlusTID[1] != 0) ? &pcsItem->m_aunSkillPlusTID[1] : NULL,
													(pcsItem->m_aunSkillPlusTID[2] != 0) ? &pcsItem->m_aunSkillPlusTID[2] : NULL);

	if(pcsItem->m_eStatus == AGPDITEM_STATUS_CASH_INVENTORY || pcsItem->m_lExpireTime > 0)
	{
		pvPacketCashInformation = m_csPacketCashInformaion.MakePacket(FALSE, pnPacketLength, 0,
																&pcsItem->m_nInUseItem, &pcsItem->m_lRemainTime, 
																&pcsItem->m_lExpireTime ,&pcsItem->m_lCashItemUseCount, 
																&pcsItem->m_llStaminaRemainTime);
	}

	pvPacketExtra = MakePacketExtra(pcsItem, pnPacketLength);

	INT8 eStatus = (INT8)pcsItem->m_eStatus;

	PVOID pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
												  &nOperation,				//Operation
												  &eStatus,		//Status
												  &pcsItem->m_lID,			//ItemID
												  &pcsItem->m_lTID,			//ItemTemplateID
												  (pcsItem->m_ulCID != AP_INVALID_CID) ? &pcsItem->m_ulCID : NULL,		//ItemOwnerID
												  (pcsItem->m_nCount) ? &pcsItem->m_nCount : NULL,		//ItemCount
												  pvFieldPacket,			//Field
												  pvInventoryPacket,		//Inventory
												  pvBankPacket,				//Bank
												  pvEquipPacket,			//Equip
												  pvPacketFactor,			//Factors
												  pvPacketFactorPercent,
												  NULL,						//TargetItemID
												  pvPacketConvert,
												  pvPacketRestrictFactor,
												  NULL,
												  pvPacketQuest,			// Quest
												  (pcsItem->m_lSkillTID != AP_INVALID_SKILLID) ? &pcsItem->m_lSkillTID : NULL,
												  NULL,						// reuse time for reverse orb
												  &pcsItem->m_lStatusFlag,
												  pvPacketOption,
												  pvPacketSkillPlus,
												  NULL,
												  pvPacketCashInformation,
												  pvPacketExtra
												  );

	if (m_pagpmFactors)
	{
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactorPercent);
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketRestrictFactor);
	}

	m_csPacketField.FreePacket(pvFieldPacket);
	m_csPacketInventory.FreePacket(pvInventoryPacket);
	m_csPacketEquip.FreePacket(pvEquipPacket);
	m_csPacketBank.FreePacket(pvBankPacket);
	m_csPacketQuest.FreePacket(pvPacketQuest);

	if (pvPacketOption)
		m_csPacketOption.FreePacket(pvPacketOption);

//	if (pvPacketConvert)
//		m_csPacketConvert.FreePacket(pvPacketConvert);
	if (pvPacketCashInformation)
		m_csPacketCashInformaion.FreePacket(pvPacketCashInformation);

	if(pvPacketExtra)
		m_csPacketExtra.FreePacket(pvPacketExtra);

	return pvPacket;
}

PVOID AgpmItem::MakePacketItemInforMation(INT32 lIID, INT32 lTID, INT32 lCID , INT32 lType, INT16 *pnPacketLength)
{
	INT8	nOperation = AGPMITEM_PACKET_OPERATION_ITEMINFORMATION;

	PVOID pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
		&nOperation,				//Operation
		NULL,		//Status
		&lIID,			//ItemID
		&lTID,			//ItemTemplateID
		&lCID,		//ItemOwnerID
		NULL,		//ItemCount
		NULL,			//Field
		NULL,		//Inventory
		NULL,				//Bank
		NULL,			//Equip
		NULL,			//Factors
		NULL,
		&lType,						//TargetItemID
		NULL,
		NULL,
		NULL,
		NULL,			// Quest
		NULL,
		NULL,						// reuse time for reverse orb
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
		);

	return pvPacket;

}

PVOID AgpmItem::MakePacketItemInforMationResult(AgpdItem *pcsItem, INT32 lType, INT16 *pnPacketLength)
{
	if (!pcsItem)
		return NULL;

	INT8	nOperation = AGPMITEM_PACKET_OPERATION_ITEMINFORMATION_RESULT;

	PVOID pvFieldPacket = NULL;
	PVOID pvInventoryPacket = NULL;
	PVOID pvBankPacket = NULL;
	PVOID pvEquipPacket = NULL;
	PVOID pvPacketFactor = NULL;
	PVOID pvPacketFactorPercent = NULL;
	PVOID pvPacketRestrictFactor = NULL;
	PVOID pvPacketQuest	= NULL;
	PVOID pvPacketCashInformation = NULL;
	PVOID pvPacketExtra = NULL;

	switch (pcsItem->m_eStatus) {
	case AGPDITEM_STATUS_FIELD:
		pvFieldPacket = m_csPacketField.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE,
			&pcsItem->m_posItem
			);
		break;

		//인벤, 거래창의 위치정보를 나타내는 패킷은 같다.
	case AGPDITEM_STATUS_INVENTORY:
	case AGPDITEM_STATUS_TRADE_GRID:
	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
	case AGPDITEM_STATUS_NPC_TRADE:
	case AGPDITEM_STATUS_SALESBOX_GRID:
	case AGPDITEM_STATUS_GUILD_WAREHOUSE:
	case AGPDITEM_STATUS_SUB_INVENTORY:
	case AGPDITEM_STATUS_CHARGING_INVENTORY:
		//	case AGPDITEM_STATUS_UNSEEN_INVENTORY:
		pvInventoryPacket = m_csPacketInventory.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE, 
			&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_TAB],
			&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_ROW],
			&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN]);
		break;
	case AGPDITEM_STATUS_BANK:
		pvBankPacket = m_csPacketBank.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE, 
			&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_TAB],
			&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_ROW],
			&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN]);
		break;

	case AGPDITEM_STATUS_QUEST:
		pvPacketQuest = m_csPacketQuest.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE, 
			&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_TAB],
			&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_ROW],
			&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN]);

	case AGPDITEM_STATUS_EQUIP:
		pvEquipPacket = m_csPacketEquip.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE
			);

	case AGPDITEM_STATUS_TRADE_OPTION_GRID:


		break;
	}

	if (m_pagpmFactors)
	{
		pvPacketFactor = m_pagpmFactors->MakePacketFactorDiffer(&pcsItem->m_csFactor, &((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_csFactor);
		pvPacketFactorPercent = m_pagpmFactors->MakePacketFactors(&pcsItem->m_csFactorPercent);
		pvPacketRestrictFactor = m_pagpmFactors->MakePacketFactorDiffer(&pcsItem->m_csRestrictFactor, &((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_csRestrictFactor);
	}

	PVOID	pvPacketConvert = NULL;
	//	if (pcsItem->m_stConvertHistory.lConvertLevel > 0)
	//	{
	//		pvPacketConvert = MakePacketItemConvertOnly(pcsItem, NULL);
	//	}

	PVOID	pvPacketOption	= NULL;

	pvPacketOption	= m_csPacketOption.MakePacket(FALSE, pnPacketLength, 0,
		(pcsItem->m_aunOptionTID[0] != 0) ? &pcsItem->m_aunOptionTID[0] : NULL,
		(pcsItem->m_aunOptionTID[1] != 0) ? &pcsItem->m_aunOptionTID[1] : NULL,
		(pcsItem->m_aunOptionTID[2] != 0) ? &pcsItem->m_aunOptionTID[2] : NULL,
		(pcsItem->m_aunOptionTID[3] != 0) ? &pcsItem->m_aunOptionTID[3] : NULL,
		(pcsItem->m_aunOptionTID[4] != 0) ? &pcsItem->m_aunOptionTID[4] : NULL);

	PVOID	pvPacketSkillPlus = NULL;

	pvPacketSkillPlus = m_csPacketSkillPlus.MakePacket(FALSE, pnPacketLength, 0,
		(pcsItem->m_aunSkillPlusTID[0] != 0) ? &pcsItem->m_aunSkillPlusTID[0] : NULL,
		(pcsItem->m_aunSkillPlusTID[1] != 0) ? &pcsItem->m_aunSkillPlusTID[1] : NULL,
		(pcsItem->m_aunSkillPlusTID[2] != 0) ? &pcsItem->m_aunSkillPlusTID[2] : NULL);

	if(pcsItem->m_eStatus == AGPDITEM_STATUS_CASH_INVENTORY || pcsItem->m_lExpireTime > 0)
	{
		pvPacketCashInformation = m_csPacketCashInformaion.MakePacket(FALSE, pnPacketLength, 0,
			&pcsItem->m_nInUseItem, &pcsItem->m_lRemainTime, 
			&pcsItem->m_lExpireTime ,&pcsItem->m_lCashItemUseCount, 
			&pcsItem->m_llStaminaRemainTime);
	}

	pvPacketExtra = MakePacketExtra(pcsItem, pnPacketLength);

	INT8 eStatus = (INT8)pcsItem->m_eStatus;

	PVOID pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
		&nOperation,				//Operation
		&eStatus,		//Status
		&pcsItem->m_lID,			//ItemID
		&pcsItem->m_lTID,			//ItemTemplateID
		(pcsItem->m_ulCID != AP_INVALID_CID) ? &pcsItem->m_ulCID : NULL,		//ItemOwnerID
		(pcsItem->m_nCount) ? &pcsItem->m_nCount : NULL,		//ItemCount
		pvFieldPacket,			//Field
		pvInventoryPacket,		//Inventory
		pvBankPacket,				//Bank
		pvEquipPacket,			//Equip
		pvPacketFactor,			//Factors
		pvPacketFactorPercent,
		&lType,						//TargetItemID// tooltipType
		pvPacketConvert,
		pvPacketRestrictFactor,
		NULL,
		pvPacketQuest,			// Quest
		(pcsItem->m_lSkillTID != AP_INVALID_SKILLID) ? &pcsItem->m_lSkillTID : NULL,
		NULL,						// reuse time for reverse orb
		&pcsItem->m_lStatusFlag,
		pvPacketOption,
		pvPacketSkillPlus,
		NULL,
		pvPacketCashInformation,
		pvPacketExtra
		);

	if (m_pagpmFactors)
	{
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactorPercent);
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketRestrictFactor);
	}

	m_csPacketField.FreePacket(pvFieldPacket);
	m_csPacketInventory.FreePacket(pvInventoryPacket);
	m_csPacketEquip.FreePacket(pvEquipPacket);
	m_csPacketBank.FreePacket(pvBankPacket);
	m_csPacketQuest.FreePacket(pvPacketQuest);

	if (pvPacketOption)
		m_csPacketOption.FreePacket(pvPacketOption);

	//	if (pvPacketConvert)
	//		m_csPacketConvert.FreePacket(pvPacketConvert);
	if (pvPacketCashInformation)
		m_csPacketCashInformaion.FreePacket(pvPacketCashInformation);

	if(pvPacketExtra)
		m_csPacketExtra.FreePacket(pvPacketExtra);

	return pvPacket;
}

PVOID AgpmItem::MakePacketItemInforMationResult(AgpdItemTemplate *pcsItemTemplate, INT32 lType, INT16 *pnPacketLength)
{
	if (!pcsItemTemplate)
		return NULL;

	INT8	nOperation = AGPMITEM_PACKET_OPERATION_ITEMINFORMATION_RESULT;

	PVOID pvPacketFactor = NULL;
	PVOID pvPacketRestrictFactor = NULL;

	if (m_pagpmFactors)
	{
		pvPacketFactor = m_pagpmFactors->MakePacketFactorDiffer(&pcsItemTemplate->m_csFactor, &pcsItemTemplate->m_csFactor);
		pvPacketRestrictFactor = m_pagpmFactors->MakePacketFactorDiffer(&pcsItemTemplate->m_csRestrictFactor, &pcsItemTemplate->m_csRestrictFactor);
	}

	PVOID pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
		&nOperation,				//Operation
		NULL,		//Status
		NULL,			//ItemID
		&pcsItemTemplate->m_lID,			//ItemTemplateID
		NULL,		//ItemOwnerID
		NULL,		//ItemCount
		NULL,			//Field
		NULL,			//Inventory
		NULL,				//Bank
		NULL,			//Equip
		pvPacketFactor,			//Factors
		NULL,
		&lType,						//TargetItemID /// tooltipType
		NULL,
		pvPacketRestrictFactor,
		NULL,
		NULL,			// Quest
		NULL,
		NULL,						// reuse time for reverse orb
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
		);

	if (m_pagpmFactors)
	{
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketRestrictFactor);
	}

	return pvPacket;
}

//		MakePacketItemView
//	Functions
//		- item 의 겉모습에 영향을 주는 정보를 패킷으로 만든다.
//			주의) 만들어진 패킷은 사용후 꼬옥 FreePacket 시켜줘야 한다.
//	Arguments
//		- pcsItem : 패킷을 만들 아템 데이타
//		- pnPacketLength : 패킷 길이
//	Return value
//		- PVOID : 만든 패킷 포인터
///////////////////////////////////////////////////////////////////////////////
PVOID AgpmItem::MakePacketItemView(AgpdItem *pcsItem, INT16 *pnPacketLength)
{
	if (!pcsItem || !pnPacketLength)
		return NULL;

	INT8	nOperation = AGPMITEM_PACKET_OPERATION_ADD;

	PVOID pvFieldPacket = NULL;
	PVOID pvInventoryPacket = NULL;
	PVOID pvEquipPacket = NULL;
	PVOID pvPacketCashInformation = NULL;
	PVOID pvPacketExtra = NULL;

	switch (pcsItem->m_eStatus) {
	case AGPDITEM_STATUS_FIELD:
		pvFieldPacket = m_csPacketField.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE,
												&pcsItem->m_posItem
												);
		break;

	//인벤, 거래창의 위치정보를 나타내는 패킷은 같다.
	case AGPDITEM_STATUS_INVENTORY:
	case AGPDITEM_STATUS_SUB_INVENTORY:
	case AGPDITEM_STATUS_TRADE_GRID:
	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
	case AGPDITEM_STATUS_CHARGING_INVENTORY:
		pvInventoryPacket = m_csPacketInventory.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE, 
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_TAB],
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_ROW],
												&pcsItem->m_anGridPos[AGPDITEM_GRID_POS_COLUMN] );
		break;

	case AGPDITEM_STATUS_EQUIP:
		pvEquipPacket = m_csPacketEquip.MakePacket(FALSE, pnPacketLength, AGPMITEM_PACKET_TYPE
												);

		break;
	}

	PVOID	pvPacketFactor			= NULL;
	PVOID	pvPacketRestrictFactor	= NULL;

	/*
	if (m_pagpmFactors)
	{
		pvPacketFactor			= m_pagpmFactors->MakePacketFactorDiffer(&pcsItem->m_csFactor, &((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_csFactor);
		//pvPacketFactor			= m_pagpmFactors->MakePacketItem(&pcsItem->m_csFactor);
		pvPacketRestrictFactor	= m_pagpmFactors->MakePacketFactorDiffer(&pcsItem->m_csRestrictFactor, &((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_csRestrictFactor);
		//pvPacketRestrictFactor	= m_pagpmFactors->MakePacketItemRestrict(&pcsItem->m_csRestrictFactor);
	}
	*/

	PVOID	pvPacketConvert = NULL;
//	if (pcsItem->m_stConvertHistory.lConvertLevel > 0)
//	{
//		pvPacketConvert = MakePacketItemConvertOnly(pcsItem, NULL);
//	}

	PVOID	pvPacketOption	= NULL;

	pvPacketOption	= m_csPacketOption.MakePacket(FALSE, pnPacketLength, 0,
													(pcsItem->m_aunOptionTID[0] != 0) ? &pcsItem->m_aunOptionTID[0] : NULL,
													(pcsItem->m_aunOptionTID[1] != 0) ? &pcsItem->m_aunOptionTID[1] : NULL,
													(pcsItem->m_aunOptionTID[2] != 0) ? &pcsItem->m_aunOptionTID[2] : NULL,
													(pcsItem->m_aunOptionTID[3] != 0) ? &pcsItem->m_aunOptionTID[3] : NULL,
													(pcsItem->m_aunOptionTID[4] != 0) ? &pcsItem->m_aunOptionTID[4] : NULL);

	//PVOID	pvPacketSkillPlus = NULL;
	//if (pcsItem->m_aunSkillPlusTID[0] != 0)
	//{
	//	pvPacketSkillPlus = m_csPacketSkillPlus.MakePacket(FALSE, pnPacketLength, 0,
	//													&pcsItem->m_aunSkillPlusTID[0],
	//													(pcsItem->m_aunSkillPlusTID[1] != 0) ? &pcsItem->m_aunSkillPlusTID[1] : NULL,
	//													(pcsItem->m_aunSkillPlusTID[2] != 0) ? &pcsItem->m_aunSkillPlusTID[2] : NULL);
	//}

	if(pcsItem->m_eStatus == AGPDITEM_STATUS_CASH_INVENTORY || pcsItem->m_lExpireTime > 0)
	{
		pvPacketCashInformation = m_csPacketCashInformaion.MakePacket(FALSE, pnPacketLength, 0,
																&pcsItem->m_nInUseItem, &pcsItem->m_lRemainTime, 
																&pcsItem->m_lExpireTime ,&pcsItem->m_lCashItemUseCount, 
																&pcsItem->m_llStaminaRemainTime);
	}

	pvPacketExtra = MakePacketExtra(pcsItem, pnPacketLength);

	INT8 eStatus = (INT8)pcsItem->m_eStatus;

	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
											  &nOperation,				//Operation
											  &eStatus,		//Status
											  &pcsItem->m_lID,			//ItemID
											  &pcsItem->m_lTID,			//ItemTemplate
											  (pcsItem->m_ulCID != AP_INVALID_CID) ? &pcsItem->m_ulCID : NULL,		//ItemOwnerID
											  (pcsItem->m_nCount) ? &pcsItem->m_nCount : NULL,		//ItemCount
											  pvFieldPacket,			//Field
											  pvInventoryPacket,		//Inventory
											  NULL,						//Bank
											  pvEquipPacket,			//Equip
											  pvPacketFactor,			//Factors
											  NULL,
											  NULL,						//TargetItemID
											  pvPacketConvert,
											  pvPacketRestrictFactor,
											  NULL,
											  NULL,						// Quest
											  (pcsItem->m_lSkillTID != AP_INVALID_SKILLID) ? &pcsItem->m_lSkillTID : NULL,
											  NULL,						// reuse time for reverse orb
											  &pcsItem->m_lStatusFlag,
											  pvPacketOption,
											  NULL,						//pvPacketSkillPlus,
											  NULL,
											  pvPacketCashInformation,
											  pvPacketExtra
											  );

	m_csPacketField.FreePacket(pvFieldPacket);
	m_csPacketInventory.FreePacket(pvInventoryPacket);
	m_csPacketEquip.FreePacket(pvEquipPacket);

	if (m_pagpmFactors)
	{
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketFactor);
		m_pagpmFactors->m_csPacket.FreePacket(pvPacketRestrictFactor);
	}

	if (pvPacketConvert)
		m_csPacketConvert.FreePacket(pvPacketConvert);

	if (pvPacketOption)
		m_csPacketOption.FreePacket(pvPacketOption);

	if(pvPacketExtra)
		m_csPacketExtra.FreePacket(pvPacketExtra);

	return pvPacket;

//	INT8	cOperation = AGPMITEM_PACKET_OPERATION_ADD;
//
//	PVOID	pvPacketField			= NULL;
//	PVOID	pvPacketInventory		= NULL;
//	PVOID	pvPacketBank			= NULL;
//	PVOID	pvPacketEquip			= NULL;
//	PVOID	pvPacketFactor			= NULL;
//	PVOID	pvPacketFactorPercent	= NULL;
//	PVOID	pvPacketRestrictFactor	= NULL;
//	PVOID	pvPacketQuest			= NULL;
//
//	switch (pcsItem->m_eStatus) {
//	case AGPDITEM_STATUS_FIELD:
//		{
//			pvPacketField = m_csPacketField.MakePacket(FALSE, pnPacketLength, 0,
//														&pcsItem->m_posItem);
//		}
//		break;
//
//	case AGPDITEM_STATUS_INVENTORY:
//		{
//			pvPacketInventory = m_csPacketInventory.MakePacket(FALSE, pnPacketLength, 0,
//														&pcsItem->m_anGridPos[0],
//														&pcsItem->m_anGridPos[1],
//														&pcsItem->m_anGridPos[2]);
//		}
//		break;
//
//	case AGPDITEM_STATUS_TRADE_GRID:
//		{
//			pvPacketInventory = m_csPacketInventory.MakePacket(FALSE, pnPacketLength, 0,
//														&pcsItem->m_anGridPos[0],
//														&pcsItem->m_anGridPos[1],
//														&pcsItem->m_anGridPos[2]);
//		}
//		break;
//
//	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
//		{
//			pvPacketInventory = m_csPacketInventory.MakePacket(FALSE, pnPacketLength, 0,
//														&pcsItem->m_anGridPos[0],
//														&pcsItem->m_anGridPos[1],
//														&pcsItem->m_anGridPos[2]);
//		}
//		break;
//
//	case AGPDITEM_STATUS_BANK:
//		{
//			pvPacketBank = m_csPacketBank.MakePacket(FALSE, pnPacketLength, 0,
//														&pcsItem->m_anGridPos[0],
//														&pcsItem->m_anGridPos[1],
//														&pcsItem->m_anGridPos[2]);
//		}
//		break;
//
//	case AGPDITEM_STATUS_QUEST:
//		{
//			pvPacketQuest = m_csPacketQuest.MakePacket(FALSE, pnPacketLength, 0,
//														&pcsItem->m_anGridPos[0],
//														&pcsItem->m_anGridPos[1],
//														&pcsItem->m_anGridPos[2]);
//		}
//		break;
//
//	case AGPDITEM_STATUS_EQUIP:
//		{
//			pvPacketEquip = m_csPacketEquip.MakePacket(FALSE, pnPacketLength, 0
//														);
//		}
//		break;
//	}
//
//	/*
//	pvPacketFactor			= m_pagpmFactors->MakePacketFactors(&pcsItem->m_csFactor);
//	pvPacketFactorPercent	= m_pagpmFactors->MakePacketFactors(&pcsItem->m_csFactorPercent);
//	pvPacketRestrictFactor	= m_pagpmFactors->MakePacketFactors(&pcsItem->m_csRestrictFactor);
//	*/
//
//	PVOID	pvPacketConvert = NULL;
//	/*
//	if (pcsItem->m_stConvertHistory.lConvertLevel > 0)
//	{
//		pvPacketConvert = MakePacketItemConvertOnly(pcsItem, NULL);
//	}
//	*/
//
//	PVOID	pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
//														&cOperation,			//Operation
//														&pcsItem->m_eStatus,	//Status
//														&pcsItem->m_lID,		//ItemID
//														&pcsItem->m_lTID,		//ItemTemplateID
//														(pcsItem->m_ulCID != AP_INVALID_CID) ? &pcsItem->m_ulCID : NULL,		//ItemOwnerID
//														(pcsItem->m_nCount) ? &pcsItem->m_nCount : NULL,		//ItemCount
//														&pvPacketField,			//Field
//														&pvPacketInventory,		//Inventory
//														&pvPacketBank,			//Bank
//														&pvPacketEquip,			//Equip
//														&pvPacketFactor,		//Factors
//														&pvPacketFactorPercent,
//														NULL,					//Target ItemID
//														&pvPacketConvert,
//														&pvPacketRestrictFactor,
//														NULL,					//Ego
//														&pvPacketQuest,			//Quest
//														(pcsItem->m_lSkillTID != 0) ? &pcsItem->m_lSkillTID : NULL
//														);
//
//	if (pvPacketField)
//		m_csPacketField.FreePacket(pvPacketField);
//	if (pvPacketInventory)
//		m_csPacketInventory.FreePacket(pvPacketInventory);
//	if (pvPacketBank)
//		m_csPacketBank.FreePacket(pvPacketBank);
//	if (pvPacketEquip)
//		m_csPacketBank.FreePacket(pvPacketBank);
//	if (pvPacketFactor)
//		m_csPacket.FreePacket(pvPacketFactor);
//	if (pvPacketFactorPercent)
//		m_csPacket.FreePacket(pvPacketFactorPercent);
//	if (pvPacketConvert)
//		m_csPacketConvert.FreePacket(pvPacketConvert);
//	if (pvPacketRestrictFactor)
//		m_csPacket.FreePacket(pvPacketRestrictFactor);
//	if (pvPacketQuest)
//		m_csPacketQuest.FreePacket(pvPacketQuest);
//
//	return pvPacket;
}

//		MakePacketItemUse
//	Functions
//		- item을 사용한다. 사용한다는 패킷을 만든다.
//	Arguments
//		- pcsItem : 패킷을 만들 아템 데이타
//		- pnPacketLength : 패킷 길이
//	Return value
//		- PVOID : 만든 패킷 포인터
///////////////////////////////////////////////////////////////////////////////
PVOID AgpmItem::MakePacketItemUse(AgpdItem *pcsItem, INT16 *pnPacketLength, INT32 lTargetID)
{
	if (!pcsItem || !pnPacketLength)
		return NULL;

	INT8	cOperation = AGPMITEM_PACKET_OPERATION_USE_ITEM;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
														&cOperation,						//Operation
														NULL,								//Status
														&pcsItem->m_lID,					//ItemID
														NULL,								//ItemTemplateID
														&pcsItem->m_ulCID,					//ItemOwnerID
														NULL,								//ItemCount
														NULL,								//Field
														NULL,								//Inventory
														NULL,								//Bank
														NULL,								//Equip
														NULL,								//Factors
														NULL,
														((lTargetID) ? &lTargetID : NULL),	//TargetItemID
														NULL,
														NULL,
														NULL,
														NULL,								// Quest
														NULL,
														NULL,								// reuse time for reverse orb
														NULL,
														NULL,
														NULL,								//pvPacketSkillPlus,
														NULL,
														NULL,								// CashInformation
														NULL
														);
}

PVOID AgpmItem::MakePacketItemPickup(AgpdItem *pcsItem, INT32 lOwnerID, INT16 *pnPacketLength)
{
	if (!pcsItem || !pnPacketLength || lOwnerID == AP_INVALID_CID)
		return NULL;

	INT8	cOperation = AGPMITEM_PACKET_OPERATION_PICKUP_ITEM;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
														&cOperation,		//Operation
														NULL,				//Status
														&pcsItem->m_lID,	//ItemID
														NULL,				//ItemTemplateID
														&lOwnerID,			//ItemOwnerID
														NULL,				//ItemCount
														NULL,				//Field
														NULL,				//Inventory
														NULL,				//Bank
														NULL,				//Equip
														NULL,				//Factors
														NULL,
														NULL,				//TagetItemID
														NULL,
														NULL,
														NULL,
														NULL,				//Quest
														NULL,
														NULL,				// reuse time for reverse orb
														NULL,
														NULL,
														NULL,				//pvPacketSkillPlus,
														NULL,
														NULL,				// CashInformation
														NULL
														);
}

PVOID AgpmItem::MakePacketItemPickupResult(INT8 cResult, INT32 lIID, INT32 lTID, INT32 lItemCount, INT16 *pnPacketLength)
{
	if (lTID == AP_INVALID_IID || !pnPacketLength)
		return NULL;

	INT8	cOperation	= AGPMITEM_PACKET_OPERATION_PICKUP_ITEM_RESULT;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
														&cOperation,
														&cResult,
														&lIID,
														&lTID,
														NULL,
														(lItemCount > 0) ? &lItemCount : NULL,
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
														NULL,		// Quest
														NULL,
														NULL,		// reuse time for reverse orb
														NULL,
														NULL,
														NULL,		//pvPacketSkillPlus,
														NULL,
														NULL
														);
}

PVOID AgpmItem::MakePacketDropMoneyToField( INT32 lCID, INT32 lMoneyCount, INT16 *pnPacketLength )
{
	if( lMoneyCount <= 0 || lCID == AP_INVALID_IID )
		return NULL;

	INT8	cOperation	= AGPMITEM_PACKET_OPERATION_DROP_MONEY;

	PVOID	pvPacket	= m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
												&cOperation,
												NULL,
												NULL,
												NULL,
												&lCID,
												&lMoneyCount,
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
												NULL,	// Quest
												NULL,
												NULL,	// reuse time for reverse orb
												NULL,
												NULL,
												NULL,	//pvPacketSkillPlus,
												NULL,
												NULL,	// CashInformation
												NULL
												);

	return pvPacket;
}

PVOID AgpmItem::MakePacketSplitItem(AgpdItem *pcsItem, INT32 lSplitStackCount, AgpdItemStatus eTargetStatus, INT16 nLayer, INT16 nRow, INT16 nColumn, INT16 *pnPacketLength)
{
	if (!pcsItem || !pnPacketLength || lSplitStackCount < 1 || pcsItem->m_nCount <= lSplitStackCount)
		return NULL;

	PVOID	pvPacketInven	= m_csPacketInventory.MakePacket(FALSE, pnPacketLength, 0,
													&nLayer,
													&nRow,
													&nColumn);
	if (!pvPacketInven)
		return NULL;

	INT8	cOperation		= AGPMITEM_PACKET_OPERATION_SPLIT_ITEM;
	INT8	cStatus			= (INT8) eTargetStatus;

	PVOID	pvPacket		= m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
													&cOperation,
													&cStatus,
													&pcsItem->m_lID,
													NULL,
													&pcsItem->m_ulCID,
													&lSplitStackCount,
													NULL,
													pvPacketInven,
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
													NULL,	// reuse time for reverse orb
													NULL,
													NULL,
													NULL,	//pvPacketSkillPlus,
													NULL,
													NULL,	// CashInformation
													NULL
													);

	m_csPacketInventory.FreePacket(pvPacketInven);

	return pvPacket;
}

PVOID AgpmItem::MakePacketSplitItem(AgpdItem *pcsItem, INT32 lSplitStackCount, AuPOS *pstDestPos, INT16 *pnPacketLength)
{
	if (!pcsItem || !pnPacketLength || lSplitStackCount < 1 || pcsItem->m_nCount <= lSplitStackCount)
		return NULL;

	PVOID	pvPacketField	= m_csPacketField.MakePacket(FALSE, pnPacketLength, 0,
													pstDestPos);

	if (!pvPacketField)
		return NULL;

	INT8	cOperation		= AGPMITEM_PACKET_OPERATION_SPLIT_ITEM;
	INT8	cStatus			= (INT8) AGPDITEM_STATUS_FIELD;

	PVOID	pvPacket		= m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
													&cOperation,
													&cStatus,
													&pcsItem->m_lID,
													NULL,
													&pcsItem->m_ulCID,
													&lSplitStackCount,
													pvPacketField,
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
													NULL,	// reuse time for reverse orb
													NULL,
													NULL,
													NULL,	//pvPacketSkillPlus,
													NULL,
													NULL,	// CashInformation
													NULL
													);

	m_csPacketField.FreePacket(pvPacketField);

	return pvPacket;
}

PVOID AgpmItem::MakePacketUpdateReuseTimeForReverseOrb(INT32 lCID, UINT32 ulReuseTIme, INT16 *pnPacketLength)
{
	if (lCID == AP_INVALID_CID)
		return NULL;

	INT8	cOperation		= AGPMITEM_PACKET_OPERATION_UPDATE_REUSE_TIME_FOR_REVERSE_ORB;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
								&cOperation,
								NULL,
								NULL,
								NULL,
								&lCID,
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
								&ulReuseTIme,	// reuse time for reverse orb
								NULL,
								NULL,
								NULL,			//pvPacketSkillPlus,
								NULL,
								NULL,			// CashInformation
								NULL
								);
}

PVOID AgpmItem::MakePacketUpdateReuseTimeForTransform(INT32 lCID, UINT32 ulReuseTIme, INT16 *pnPacketLength)
{
	if (lCID == AP_INVALID_CID)
		return NULL;

	INT8	cOperation		= AGPMITEM_PACKET_OPERATION_UPDATE_REUSE_TIME_FOR_TRANSFORM;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
								&cOperation,
								NULL,
								NULL,
								NULL,
								&lCID,
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
								NULL,			// reuse time for reverse orb
								NULL,
								NULL,
								NULL,			//pvPacketSkillPlus,
								&ulReuseTIme,
								NULL,			// CashInformation
								NULL
								);
}

PVOID AgpmItem::MakePacketInitReuseTimeForTransform(INT32 lCID, INT16 *pnPacketLength)
{
	if (lCID == AP_INVALID_CID)
		return NULL;

	INT8	cOperation		= AGPMITEM_PACKET_OPERATION_INIT_TIME_FOR_TRANSFORM;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
								&cOperation,
								NULL,
								NULL,
								NULL,
								&lCID,
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
								NULL,	// reuse time for reverse orb
								NULL,
								NULL,
								NULL,	//pvPacketSkillPlus,
								NULL,
								NULL,	// CashInformation
								NULL
								);
}

PVOID AgpmItem::MakePacketRequestDestroyItem(INT32 lCID, INT32 lIID, INT16 *pnPacketLength)
{
	if (lCID == AP_INVALID_CID ||
		lIID == AP_INVALID_IID ||
		!pnPacketLength)
		return NULL;

	INT8	cOperation		= AGPMITEM_PACKET_OPERATION_REQUEST_DESTROY_ITEM;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
								&cOperation,
								NULL,
								&lIID,
								NULL,
								&lCID,
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
								NULL,	// reuse time for reverse orb
								NULL,
								NULL,
								NULL,	//pvPacketSkillPlus,
								NULL,
								NULL,	// CashInformation
								NULL
								);
}

PVOID AgpmItem::MakePacketRequestBuyBankSlot(INT32 lCID, INT16 *pnPacketLength)
{
	if (lCID == AP_INVALID_CID ||
		!pnPacketLength)
		return NULL;

	INT8	cOperation		= AGPMITEM_PACKET_OPERATION_REQUEST_BUY_BANK_SLOT;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
								&cOperation,
								NULL,
								NULL,
								NULL,
								&lCID,
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
								NULL,	// reuse time for reverse orb
								NULL,
								NULL,
								NULL,	//pvPacketSkillPlus,
								NULL,
								NULL,	// CashInformation
								NULL
								);
}

/*
	2005.11.30. By SungHoon
	아이템 사용중지 패킷을 만든다.
*/
PVOID AgpmItem::MakePacketItemUnuse(AgpdItem *pcsItem, INT16 *pnPacketLength)
{
	if (!pcsItem || !pnPacketLength)
		return NULL;

	INT8	cOperation = AGPMITEM_PACKET_OPERATION_UNUSE_ITEM;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
														&cOperation,						//Operation
														NULL,								//Status
														&pcsItem->m_lID,					//ItemID
														NULL,								//ItemTemplateID
														&pcsItem->m_ulCID,					//ItemOwnerID
														NULL,								//ItemCount
														NULL,								//Field
														NULL,								//Inventory
														NULL,								//Bank
														NULL,								//Equip
														NULL,								//Factors
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,								// Quest
														NULL,
														NULL,								// reuse time for reverse orb
														NULL,
														NULL,
														NULL,								//pvPacketSkillPlus,
														NULL,
														NULL,								// CashInformation
														NULL
														);
}

/*
	2005.11.30. By SungHoon
	아이템 사용중지 패킷을 만든다.
*/
PVOID AgpmItem::MakePacketItemUnuseFailed(AgpdItem *pcsItem, INT16 *pnPacketLength)
{
	if (!pcsItem || !pnPacketLength)
		return NULL;

	INT8	cOperation = AGPMITEM_PACKET_OPERATION_UNUSE_ITEM_FAILED;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
														&cOperation,						//Operation
														NULL,								//Status
														&pcsItem->m_lID,					//ItemID
														NULL,								//ItemTemplateID
														&pcsItem->m_ulCID,					//ItemOwnerID
														NULL,								//ItemCount
														NULL,								//Field
														NULL,								//Inventory
														NULL,								//Bank
														NULL,								//Equip
														NULL,								//Factors
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,								// Quest
														NULL,
														NULL,								// reuse time for reverse orb
														NULL,
														NULL,
														NULL,								//pvPacketSkillPlus,
														NULL,
														NULL,								// CashInformation
														NULL
														);
}

// 2006.01.08. steeple
PVOID AgpmItem::MakePacketItemPause(AgpdItem* pcsItem, INT16* pnPacketLength)
{
	if(!pcsItem || !pnPacketLength)
		return NULL;

	INT8	cOperation = AGPMITEM_PACKET_OPERATION_PAUSE_ITEM;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
														&cOperation,						//Operation
														NULL,								//Status
														&pcsItem->m_lID,					//ItemID
														NULL,								//ItemTemplateID
														&pcsItem->m_ulCID,					//ItemOwnerID
														NULL,								//ItemCount
														NULL,								//Field
														NULL,								//Inventory
														NULL,								//Bank
														NULL,								//Equip
														NULL,								//Factors
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,								// Quest
														NULL,
														NULL,								// reuse time for reverse orb
														NULL,
														NULL,
														NULL,								//pvPacketSkillPlus,
														NULL,
														NULL,								// CashInformation
														NULL
														);
}

PVOID AgpmItem::MakePacketItemUpdateItemUseTime(AgpdItem *pcsItem, INT16 *pnPacketLength)
{
	if (!pcsItem || !pnPacketLength)
		return NULL;

	PVOID pvPacketCashInformation = m_csPacketCashInformaion.MakePacket(FALSE, pnPacketLength, 0,
										&pcsItem->m_nInUseItem, &pcsItem->m_lRemainTime, &pcsItem->m_lExpireTime, &pcsItem->m_lCashItemUseCount, &pcsItem->m_llStaminaRemainTime);

	if (!pvPacketCashInformation)
		return NULL;

	INT8 cOperation = AGPMITEM_PACKET_OPERATION_UPDATE_ITEM_USE_TIME;

	PVOID pvPacket =  m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
														&cOperation,						//Operation
														NULL,								//Status
														&pcsItem->m_lID,					//ItemID
														NULL,								//ItemTemplateID
														&pcsItem->m_ulCID,					//ItemOwnerID
														NULL,								//ItemCount
														NULL,								//Field
														NULL,								//Inventory
														NULL,								//Bank
														NULL,								//Equip
														NULL,								//Factors
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,								// Quest
														NULL,
														NULL,								// reuse time for reverse orb
														NULL,
														NULL,
														NULL,								//pvPacketSkillPlus,
														NULL,
														pvPacketCashInformation,				// CashInformation
														NULL
														);

	m_csPacketCashInformaion.FreePacket(pvPacketCashInformation);

	return pvPacket;
}

PVOID AgpmItem::MakePacketItemUpdateItemStaminaRemainTime(AgpdItem *pcsItem, INT16 *pnPacketLength)
{
	if (!pcsItem || !pnPacketLength)
		return NULL;

	PVOID pvPacketCashInformation = m_csPacketCashInformaion.MakePacket(FALSE, pnPacketLength, 0,
										NULL, NULL, NULL, NULL, &pcsItem->m_llStaminaRemainTime);

	if (!pvPacketCashInformation)
		return NULL;

	INT8 cOperation = AGPMITEM_PACKET_OPERATION_UPDATE_STAMINA_REMAIN_TIME;

	PVOID pvPacket =  m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
														&cOperation,						//Operation
														NULL,								//Status
														&pcsItem->m_lID,					//ItemID
														NULL,								//ItemTemplateID
														&pcsItem->m_ulCID,					//ItemOwnerID
														NULL,								//ItemCount
														NULL,								//Field
														NULL,								//Inventory
														NULL,								//Bank
														NULL,								//Equip
														NULL,								//Factors
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,								// Quest
														NULL,
														NULL,								// reuse time for reverse orb
														NULL,
														NULL,
														NULL,								//pvPacketSkillPlus,
														NULL,
														pvPacketCashInformation,				// CashInformation
														NULL
														);

	m_csPacketCashInformaion.FreePacket(pvPacketCashInformation);

	return pvPacket;
}

// 2008.02.15. steeple
PVOID AgpmItem::MakePacketItemUpdateCooldown(INT32 lOwnerCID, AgpdItemCooldownBase stCooldownBase, INT16* pnPacketLength)
{
	if(lOwnerCID < 1 || stCooldownBase.m_lTID < 1 || !pnPacketLength)
		return NULL;

	INT8 cOperation = AGPMITEM_PACKET_OPERATION_UPDATE_COOLDOWN;

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
														&cOperation,						//Operation
														NULL,								//Status
														NULL,								//ItemID
														&stCooldownBase.m_lTID,				//ItemTemplateID
														&lOwnerCID,							//ItemOwnerID
														&stCooldownBase.m_bPause,			//ItemCount
														NULL,								//Field
														NULL,								//Inventory
														NULL,								//Bank
														NULL,								//Equip
														NULL,								//Factors
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,
														NULL,								// Quest
														NULL,
														&stCooldownBase.m_ulRemainTime,		// reuse time for reverse orb
														NULL,
														NULL,
														NULL,								//pvPacketSkillPlus,
														NULL,
														NULL,								// CashInformation
														NULL
														);
}

PVOID AgpmItem::MakePacketChangeAutoPickItem(INT32 lCID, INT32 llID, INT16 *lSwitch, INT16 *pnPacketLength)
{
	if (lCID == AP_INVALID_CID ||
		llID == AP_INVALID_IID ||
		!pnPacketLength ||
		!(*lSwitch == 1 || *lSwitch == 0))
		return NULL;

	INT8 cOperation = AGPMITEM_PACKET_OPERATION_CHANGE_AUTOPICK_ITEM;

	PVOID pvPacketAutoPickItem = m_csPacketAutoPickItem.MakePacket(FALSE, pnPacketLength, 0, lSwitch);

	return m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMITEM_PACKET_TYPE,
															&cOperation,
															NULL,
															&llID,
															NULL,
															&lCID,
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
															NULL,
															pvPacketAutoPickItem,	// Option Packet
															NULL,	
															NULL,
															NULL,
															NULL
															);


}

PVOID AgpmItem::MakePacketExtra(AgpdItem* pcsItem, INT16 *pnPacketLength)
{
	if(!pcsItem || !pcsItem->m_pcsItemTemplate)
		return NULL;

	if( pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE )
	{
		PACKET_AGPMITEM_EXTRA pPacketExtra;

		EnumCallback(ITEM_CB_ID_GET_EXTRADATA, pcsItem, &pPacketExtra);

		if(pPacketExtra.Type)
		{
			return m_csPacketExtra.MakePacket(FALSE, pnPacketLength, 0, &pPacketExtra);
		}
	}

	return NULL;
}

//		ChangeItemOwner
//	Functions
//		- item의 주인을 바꾼다.
//	Arguments
//		- lIID			:
//		- ulNewOwnerID	:
//	Return value
//		- BOOL			: 성공 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgpmItem::ChangeItemOwner(INT32 lIID, INT32 ulNewOwnerID)
{
	return ChangeItemOwner(GetItem(lIID), m_pagpmCharacter->GetCharacter(ulNewOwnerID));
}

BOOL AgpmItem::ChangeItemOwner(AgpdItem *pcsAgpdItem, AgpdCharacter *pcsAgpdCharacter)
{
	if (!pcsAgpdItem)
		return FALSE;

	INT32	lPrevOwner			= pcsAgpdItem->m_ulCID;
	if(m_pagpmCharacter->IsPC(pcsAgpdItem->m_pcsCharacter))
		pcsAgpdItem->m_lPrevOwner = lPrevOwner;

	if (pcsAgpdCharacter)
	{
		pcsAgpdItem->m_ulCID		= pcsAgpdCharacter->m_lID;
		pcsAgpdItem->m_pcsCharacter	= pcsAgpdCharacter;

		if (lPrevOwner == pcsAgpdCharacter->m_lID)
			return TRUE;
	}
	else
	{
		pcsAgpdItem->m_ulCID		= AP_INVALID_CID;
		pcsAgpdItem->m_pcsCharacter	= NULL;

		if (lPrevOwner == AP_INVALID_CID)
			return TRUE;
	}

	if (!IsBoundOnOwner(pcsAgpdItem) && E_AGPMITEM_BIND_ON_ACQUIRE == GetBoundType(pcsAgpdItem))
		SetBoundOnOwner(pcsAgpdItem, pcsAgpdCharacter);

	// 아템이 돈이라면 걍 넘어간다.
	if (GetMoneyTID() != ((AgpdItemTemplate *) pcsAgpdItem->m_pcsItemTemplate)->m_lID)
	{
		EnumCallback(ITEM_CB_ID_CHANGE_ITEM_OWNER, pcsAgpdItem, &lPrevOwner);
	}

	return TRUE;
}

BOOL AgpmItem::LockAdminItem()
{
//	return csItemAdmin.m_Mutex.WLock();
	return TRUE;
}

BOOL AgpmItem::ReleaseAdminItem()
{
//	return csItemAdmin.m_Mutex.Release();
	return TRUE;
}

BOOL AgpmItem::HasItemTemplate(AgpdCharacter *pcsCharacter, INT32 lITID)
{
	AgpdItemADChar *	pstItemADChar = GetADCharacter(pcsCharacter);

	if (m_pagpmGrid->IsExistItemTemplate(&pstItemADChar->m_csInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM, lITID))
		return TRUE;

	if (m_pagpmGrid->IsExistItemTemplate(&pstItemADChar->m_csEquipGrid, AGPDGRID_ITEM_TYPE_ITEM, lITID))
		return TRUE;

//	2005.11.16. By SungHoon
//	캐쉬인벤도 검사해본다.
	if (m_pagpmGrid->IsExistItemTemplate(&pstItemADChar->m_csCashInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM, lITID))
		return TRUE;

	return FALSE;
}

BOOL AgpmItem::CharacterForAllItems(AgpdCharacter *pcsCharacter, ApModuleDefaultCallBack pfnCallback, PVOID pClass)
{
	AgpdItemADChar *	pstItemADChar = GetADCharacter(pcsCharacter);
	AgpdItem *			pcsItem;
	AgpdGridItem *		pstGridItem;
	INT32				lIndex;

	for(lIndex = 0; lIndex < pstItemADChar->m_csInventoryGrid.m_lGridCount; ++lIndex)
	{
		pstGridItem = pstItemADChar->m_csInventoryGrid.m_ppcGridData[lIndex];
		if(pstGridItem)
		{
			pcsItem	= GetItem(pstGridItem);

			if (pcsItem && !pfnCallback(pcsItem, pClass, pcsCharacter))
				return FALSE;
		}
	}

	for (lIndex = 0; lIndex < AGPMITEM_PART_NUM; ++lIndex)
	{
		pstGridItem = GetEquipItem(pcsCharacter, GetEquipIndexFromTable(lIndex));
		if (pstGridItem)
		{
			pcsItem	= GetItem(pstGridItem);

			if (pcsItem && !pfnCallback(pcsItem, pClass, pcsCharacter))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL AgpmItem::UseItem(INT32 lIID, AgpdCharacter *pcsTargetChar)
{
	return UseItem(GetItem(lIID), pcsTargetChar);
}

BOOL AgpmItem::UseItem(AgpdItem *pcsItem, AgpdCharacter *pcsTargetChar)
{
	if (!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	return EnumCallback(ITEM_CB_ID_USE_ITEM, pcsItem, pcsTargetChar);
}

BOOL AgpmItem::CheckUseValidArea(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	if (pcsItem->m_pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE)
		return FALSE;

	if (((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_POTION)
	{
		AgpmItemUsablePotionType2	eType2 = ((AgpdItemTemplateUsablePotion *) pcsItem->m_pcsItemTemplate)->m_ePotionType2;
		
		if(CheckAreaUsePotion(pcsItem, eType2) == FALSE)
			return FALSE;
	}

	return TRUE;
}

BOOL AgpmItem::CheckAreaUsePotion(AgpdItem *pcsItem, AgpmItemUsablePotionType2 ePotionType2)
{
	if(NULL == pcsItem)
		return FALSE;

	ApmMap::RegionTemplate *pRegionTemplate	= m_papmMap->GetTemplate(m_papmMap->GetRegion(pcsItem->m_pcsCharacter->m_stPos.x, pcsItem->m_pcsCharacter->m_stPos.z));
	if(NULL == pRegionTemplate)
		return FALSE;

	// 아이템 타입을 구하고
	AgpmItemUsablePotionType ePotionType = ((AgpdItemTemplateUsablePotion *) pcsItem->m_pcsItemTemplate)->m_ePotionType;

	BOOL bUse = FALSE;

	if(ePotionType == AGPMITEM_USABLE_POTION_TYPE_HP || ePotionType == AGPMITEM_USABLE_POTION_TYPE_BOTH)
	{
		// 일반물약
		if(ePotionType2 == AGPMITEM_USABLE_POTION_TYPE2_NORMAL)
		{
			// 지금 있는 지역이 일반물약 가능지역 표시가 있다면 사용가능
			if(pRegionTemplate->ti.stType.bItemPotion) bUse = TRUE;
		}
		// 정령계물약
		else if(ePotionType2 == AGPMITEM_USABLE_POTION_TYPE2_HOT)
		{
			// 지금 있는 지역이 뜨거운포션 가능지역 표시가 있다면 사용가능
			if(pRegionTemplate->ti.stType.bItemPotion2) bUse = TRUE;
		}
		// 길드물약
		else if(ePotionType2 == AGPMITEM_USABLE_POTION_TYPE2_GUILD)
		{
			// 지금 있는 지역이 길드경기포션 가능지역 표시가 있다면 사용가능
			if(pRegionTemplate->ti.stType.bItemPotionGuild) bUse = TRUE;
		}
		// 에픽물약
		else if(ePotionType2 == AGPMITEM_USABLE_POTION_TYPE2_VITAL)
		{
			// 지금 있는 지역이 활력포션 가능지역 표시가 있다면 사용가능
			if(pRegionTemplate->ti.stType.bVitalPotion) bUse = TRUE;
		}
		// 리커버물약
		else if(ePotionType2 == AGPMITEM_USABLE_POTION_TYPE2_RECOVERY)
		{
			// 지금 있는 지역이 리커버포션 가능지역 표시가 있다면 사용가능
			if(pRegionTemplate->ti.stType.bRecoveryPotion) bUse = TRUE;
		}
		// 큐어물약
		else if(ePotionType2 == AGPMITEM_USABLE_POTION_TYPE2_CURE)
		{
			// 지금 있는 지역이 큐어포션 가능지역 표시가 있다면 사용가능
			if(pRegionTemplate->ti.stType.bCurePotion) bUse = TRUE;
		}
		else if(ePotionType2 == AGPMITEM_USABLE_POTION_TYPE2_SHRINE)
		{
			// 지금 있는 지역이 쉬라인포션 가능지역 표시가 있다면 사용가능
			if(pRegionTemplate->ti.stType.bShrinePotion) bUse = TRUE;
		}
	}
	else
	{
		bUse = TRUE;
	}

	return bUse;
}

INT32 AgpmItem::GetMoneyTID()
{
	return m_lItemMoneyTID;
}

INT32 AgpmItem::GetArrowTID()
{
	return m_lItemArrowTID;
}

INT32 AgpmItem::GetBoltTID()
{
	return m_lItemBoltTID;
}

INT32 AgpmItem::GetHumanSkullTID()
{
	return m_lItemHumanSkullTID;
}

BOOL AgpmItem::SetHumanSkullTID(INT32 lSkullTID)
{
	m_lItemHumanSkullTID	= lSkullTID;

	return TRUE;
}

INT32 AgpmItem::GetOrcSkullTID()
{
	return m_lItemOrcSkullTID;
}

BOOL AgpmItem::SetOrcSkullTID(INT32 lSkullTID)
{
	m_lItemOrcSkullTID	= lSkullTID;

	return TRUE;
}

INT32 AgpmItem::GetSkullTID(INT32 lSkullLevel)
{
	if(lSkullLevel < 0 || lSkullLevel >= AGPMITEM_MAX_SKULL_LEVEL)
		return 0;

	return m_alSkullTID[lSkullLevel];
}

BOOL AgpmItem::SetSkullTID(INT32 lSkullTID, INT32 lSkullLevel)
{
	if(lSkullLevel < 0 || lSkullLevel >= AGPMITEM_MAX_SKULL_LEVEL)
		return 0;

	m_alSkullTID[lSkullLevel] = lSkullTID;

	return TRUE;
}

INT32 AgpmItem::GetCatalystTID()
{
	return m_lItemCatalystTID;
}

BOOL AgpmItem::SetCatalystTID(INT32 lCatalystTID)
{
	m_lItemCatalystTID	= lCatalystTID;

	return TRUE;
}

INT32 AgpmItem::GetLuckyScrollTID()
{
	return m_lItemLuckyScrollTID;
}

BOOL AgpmItem::SetLuckyScrollTID(INT32 lLuckyScrollTID)
{
	m_lItemLuckyScrollTID	= lLuckyScrollTID;

	return TRUE;
}

INT32 AgpmItem::GetChattingEmphasisTID()
{
	return m_lItemChattingEmphasisTID;
}

BOOL AgpmItem::SetChattingEmphasisTID(INT32 lEmphasisTID)
{
	m_lItemChattingEmphasisTID = lEmphasisTID;

	return TRUE;
}

INT32 AgpmItem::GetSkillBookTID(AuRaceType eRaceType, AuCharClassType eClassType)
{
	if (eRaceType <= AURACE_TYPE_NONE || eRaceType >= AURACE_TYPE_MAX ||
		eClassType <= AUCHARCLASS_TYPE_NONE || eClassType >= AUCHARCLASS_TYPE_MAX)
		return 0;

	return m_lItemSkillBookTID[eRaceType][eClassType];
}

BOOL AgpmItem::SetSkillBookTID(AuRaceType eRaceType, AuCharClassType eClassType, INT32 lSkillBookTID)
{
	if (eRaceType <= AURACE_TYPE_NONE || eRaceType >= AURACE_TYPE_MAX ||
		eClassType <= AUCHARCLASS_TYPE_NONE || eClassType >= AUCHARCLASS_TYPE_MAX)
		return FALSE;

	m_lItemSkillBookTID[eRaceType][eClassType]	= lSkillBookTID;

	return TRUE;
}

INT32 AgpmItem::GetTotalArrowCountInInventory(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return 0;

	AgpdItem *pcsArrow = GetArrowItemFromInventory(pcsCharacter);
	if(NULL == pcsArrow)
		return 0;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);

	return pcsItemADChar->m_lNumArrowCount;
}

INT32 AgpmItem::GetTotalBoltCountInInventory(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return 0;

	AgpdItem *pcsBolt = GetBoltItemFromInventory(pcsCharacter);
	if(NULL == pcsBolt)
		return 0;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);

	return pcsItemADChar->m_lNumBoltCount;
}

BOOL AgpmItem::SetItemMoney(AgpdItem *pcsItem, INT32 lMoney)
{
	if (!pcsItem || lMoney < 1)
		return FALSE;

	AgpdFactorPrice	*pcsFactorPrice = (AgpdFactorPrice *) m_pagpmFactors->SetFactors(&pcsItem->m_csFactor, NULL, AGPD_FACTORS_TYPE_PRICE);
	if (!pcsFactorPrice)
		return FALSE;

	m_pagpmFactors->SetValue(&pcsItem->m_csFactor, lMoney, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_MONEY_LOW);

	return TRUE;
}

INT32 AgpmItem::GetItemMoney(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return 0;

	//INT32	lMoney = 0;

	//m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lMoney, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_MONEY_LOW);

	return pcsItem->m_nCount;
}

BOOL AgpmItem::SetItemPrice(AgpdItem *pcsItem, INT64 llPrice)
{
	if (!pcsItem)
		return FALSE;

	INT32	lHigh	= 0;
	INT32	lLow	= 0;

	CopyMemory(&lHigh, &llPrice, sizeof(INT32));
	CopyMemory(&lLow, ((INT32 *) &llPrice) + 1, sizeof(INT32));

	m_pagpmFactors->SetValue(&pcsItem->m_csFactor, lHigh, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_MONEY_HIGH);
	m_pagpmFactors->SetValue(&pcsItem->m_csFactor, lLow, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_MONEY_LOW);

	return TRUE;
}

INT64 AgpmItem::GetItemPrice(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return (-1);

	INT64	llPrice	= 0;

	INT32	lHigh	= 0;
	INT32	lLow	= 0;

	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lHigh, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_MONEY_HIGH);
	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lLow, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_MONEY_LOW);

	CopyMemory(&llPrice, &lHigh, sizeof(INT32));
	CopyMemory(((INT32 *) &llPrice) + 1, &lLow, sizeof(INT32));

	return llPrice;
}

BOOL	AgpmItem::SetNPCTradeGheld( AgpdItem* pcsItem , INT32 nGheldPrice )
{
	if( !pcsItem )
		return FALSE;

	AgpdFactorPrice	*pcsFactorPrice = (AgpdFactorPrice *) m_pagpmFactors->SetFactors( &pcsItem->m_csFactor, NULL, AGPD_FACTORS_TYPE_PRICE );
	if (!pcsFactorPrice)
		return FALSE;

	m_pagpmFactors->SetValue(&pcsItem->m_csFactor, nGheldPrice, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_NPC_PRICE );

	return TRUE;
}

BOOL	AgpmItem::SetNPCTradeCharismaPoint( AgpdItem* pcsItem , INT32 nCharismaPoint )
{
	if( !pcsItem )
		return FALSE;

	AgpdFactorPrice	*pcsFactorPrice = (AgpdFactorPrice *) m_pagpmFactors->SetFactors( &pcsItem->m_csFactor, NULL, AGPD_FACTORS_TYPE_PRICE );
	if (!pcsFactorPrice)
		return FALSE;

	m_pagpmFactors->SetValue(&pcsItem->m_csFactor, nCharismaPoint, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_CHRISMA_PRICE );

	return TRUE;
}

BOOL	AgpmItem::SetNPCTradeShrineCoin( AgpdItem* pcsItem , INT32 nShrineCoin )
{
	if( !pcsItem )
		return FALSE;

	AgpdFactorPrice	*pcsFactorPrice = (AgpdFactorPrice *) m_pagpmFactors->SetFactors( &pcsItem->m_csFactor, NULL, AGPD_FACTORS_TYPE_PRICE );
	if (!pcsFactorPrice)
		return FALSE;

	m_pagpmFactors->SetValue(&pcsItem->m_csFactor, nShrineCoin, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_SHRINE_COIN );

	return TRUE;
}


float AgpmItem::GetNPCPrice(AgpdItem *pcsItem)
{
/*	INT32	lPrice = 0;

	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lPrice, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_PRICE);

	return lPrice;*/
	float	fItemPrice = -1.0f;

	if (!pcsItem)
		return fItemPrice;

	PVOID	pvItemPrice = m_pagpmFactors->GetFactor( &pcsItem->m_csFactor, AGPD_FACTORS_TYPE_PRICE );

	if( pvItemPrice ) 
		fItemPrice = *((float *)pvItemPrice);

	return fItemPrice;
}

float AgpmItem::GetPCPrice(AgpdItem *pcsItem)
{
/*	INT32	lPrice = 0;

	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lPrice, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_PRICE);

	return lPrice;*/
	float	fItemPrice = -1.0f;

	if (!pcsItem)
		return fItemPrice;

	PVOID	pvItemPrice = m_pagpmFactors->GetFactor( &pcsItem->m_csFactor, AGPD_FACTORS_TYPE_PRICE );

	if( pvItemPrice ) 
		fItemPrice = *((float *)&(((AgpdFactorPrice *) pvItemPrice)->lValue[AGPD_FACTORS_PRICE_TYPE_PC_PRICE]));

	return fItemPrice;
}
//JK_쉬라인상점
INT32 AgpmItem::GetCharismaPrice(AgpdItem *pcsItem)
{
	INT32 lChrismaPrice = 0;

	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lChrismaPrice, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_CHRISMA_PRICE);

	return lChrismaPrice;
}

//JK_쉬라인상점
INT32 AgpmItem::GetShrineCoinPrice(AgpdItem *pcsItem)
{
	INT32 lShrineCoinPrice = 0;

	m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lShrineCoinPrice, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_SHRINE_COIN);

	return lShrineCoinPrice;
}

INT32	AgpmItem::GetItemRank( AgpdItem* pcsItem )
{
	if( !pcsItem )
		return -1;

	INT32	nItemRank	= 0;
	m_pagpmFactors->GetValue( &pcsItem->m_csFactor, &nItemRank, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_RANK );

	return nItemRank;
}

AgpdItem* AgpmItem::AddItemMoney(INT32 lIID, INT32 lMoney, AuPOS pos)
{
	AgpdItem *pcsItem = AddItem(lIID, GetMoneyTID(), lMoney );
	if (pcsItem)
	{
		//m_pagpmFactors->SetValue(&pcsItem->m_csFactor, lMoney, AGPD_FACTORS_TYPE_PRICE, AGPD_FACTORS_PRICE_TYPE_MONEY_LOW);

		pcsItem->m_posItem = pos;

		InitItem(pcsItem);

		AddItemToField(pcsItem);
	}

	return pcsItem;
}

BOOL AgpmItem::CheckUseItem(AgpdCharacter *pcsCharacter, AgpdItem *pcsItem, BOOL bExceptLevel, BOOL bLogin/* = FALSE*/)
{
	if (!pcsCharacter || !pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	// ReverseOrb류 아이템을 사용 가능한 지역인지 아닌지 확인한다.
	if(IsReverseOrbTypeItem(pcsItem) == TRUE)
	{
		ApmMap::RegionTemplate *pTemplate = m_papmMap->GetTemplate(pcsCharacter->m_nBindingRegionIndex);
		if(NULL != pTemplate)
		{
			if(pTemplate->ti.stType.bItemResurrect == FALSE)
				return FALSE;
		}
	}

	// pcsItem을 pcsCharacter가 사용(입던지,, 사용하던지,,)할 수 있는지 검사한다.
	if (m_pagpmCharacter->IsPC(pcsCharacter))		// PC Character만 체크한다. 몬스터 같은 경우 체크하지 않는다.
	{
		// class와 race, gender가 맞는지 검사한다.
		//
		INT32	lItemClass = m_pagpmFactors->GetClass(&pcsItem->m_csRestrictFactor);
		INT32	lCharClass = m_pagpmFactors->GetClass(&pcsCharacter->m_csFactor);

		if (lItemClass > 0 &&
			lItemClass != (INT32) AGPM_FACTORS_UPDATE_FACTOR_INIT_VALUE && 
			!m_pagpmFactors->CheckClass((AuCharClassType)lCharClass, &pcsItem->m_csRestrictFactor))
			return FALSE;

		INT32	lItemRace = m_pagpmFactors->GetRace(&pcsItem->m_csRestrictFactor);
		INT32	lCharRace = m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor);

		if (lItemRace > 0 &&
			lItemRace != (INT32) AGPM_FACTORS_UPDATE_FACTOR_INIT_VALUE && 
			!m_pagpmFactors->CheckRace((AuRaceType) lCharRace, &pcsItem->m_csRestrictFactor))
			return FALSE;

		INT32	lItemGender = m_pagpmFactors->GetGender(&pcsItem->m_csRestrictFactor);
		INT32	lCharGender = m_pagpmFactors->GetGender(&pcsCharacter->m_csFactor);

		if (lItemGender > 0 &&
			lItemGender != (INT32) AGPM_FACTORS_UPDATE_FACTOR_INIT_VALUE && 
			lItemGender != lCharGender)
			return FALSE;

		// level이 맞는지 검사한다.
		//

		INT32	lItemLevel = m_pagpmFactors->GetLevel(&pcsItem->m_csRestrictFactor);
		INT32	lCharLevel = m_pagpmCharacter->GetLevel(pcsCharacter);

		if(FALSE == bExceptLevel)
		{
			if (lItemLevel > 0 &&
			lItemLevel != (INT32) AGPM_FACTORS_UPDATE_FACTOR_INIT_VALUE && 
			lItemLevel > lCharLevel)
			return FALSE;

			// Max Level 쪽도 처리 2008.01.29. steeple
			if(pcsItem->m_pcsItemTemplate->m_lLimitedLevel != 0 &&
			lCharLevel > pcsItem->m_pcsItemTemplate->m_lLimitedLevel)
			return FALSE;
		}

		// 탈것의 경우 로그인 서버에서는 리전을 몰라서 체크로직을 제대로 처리하지 못한다
		// 탈것을 타고 로그아웃을 했다면, 재접속시 그 리전은 탈것을 이용가능한 리전이기 때문에 
		// 로그인 할때 체크하지 않고 바로 태워준다
		if ( FALSE == bLogin )
		{
			//탈것의 경우 Region에서 타기 가능한 상태를 체크한다.
			if(((AgpdItemTemplateEquip*)pcsItem->m_pcsItemTemplate)->m_nKind == AGPMITEM_EQUIP_KIND_RIDE)
			{
				INT32 RegionIndex = m_papmMap->GetRegion(pcsCharacter->m_stPos.x, pcsCharacter->m_stPos.z);
				ApmMap::RegionTemplate* pRegion = m_papmMap->GetTemplate(RegionIndex);

				ASSERT(NULL != pRegion);
				if (NULL == pRegion)
					return FALSE;

				if (FALSE == pRegion->ti.stType.bRidable)
					return FALSE;
			}
		}

		// 아이템 상태가 Disarmament 인지 확인한다.
		if (pcsItem->m_lStatusFlag & AGPMITEM_STATUS_DISARMAMENT)
			return FALSE;
	}

	return TRUE;
}

BOOL AgpmItem::CheckUseItem(AgpdCharacter *pcsCharacter, AgpdItemTemplate *pcsItemTemplate)
{
	if (!pcsCharacter || !pcsItemTemplate)
		return FALSE;

	// pcsItem을 pcsCharacter가 사용(입던지,, 사용하던지,,)할 수 있는지 검사한다.

	// class와 race, gender가 맞는지 검사한다.
	//
	INT32	lItemClass = m_pagpmFactors->GetClass(&pcsItemTemplate->m_csRestrictFactor);
	INT32	lCharClass = m_pagpmFactors->GetClass(&pcsCharacter->m_csFactor);

	if (lItemClass > 0 &&
		lItemClass != (INT32) AGPM_FACTORS_UPDATE_FACTOR_INIT_VALUE && 
		!m_pagpmFactors->CheckClass((AuCharClassType)lCharClass, &pcsItemTemplate->m_csRestrictFactor))
		return FALSE;

	INT32	lItemRace = m_pagpmFactors->GetRace(&pcsItemTemplate->m_csRestrictFactor);
	INT32	lCharRace = m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor);

	if (lItemRace > 0 &&
		lItemRace != (INT32) AGPM_FACTORS_UPDATE_FACTOR_INIT_VALUE && 
		!m_pagpmFactors->CheckRace((AuRaceType) lCharRace, &pcsItemTemplate->m_csRestrictFactor))
		return FALSE;

	INT32	lItemGender = m_pagpmFactors->GetGender(&pcsItemTemplate->m_csRestrictFactor);
	INT32	lCharGender = m_pagpmFactors->GetGender(&pcsCharacter->m_csFactor);

	if (lItemGender > 0 &&
		lItemGender != (INT32) AGPM_FACTORS_UPDATE_FACTOR_INIT_VALUE && 
		lItemGender != lCharGender)
		return FALSE;

	return TRUE;
}

BOOL AgpmItem::SaveItemStatus(AgpdItem *pcsItem, AgpdItem *pcsBuffer)
{
	if (!pcsItem || !pcsBuffer)
		return FALSE;

	pcsBuffer->m_eStatus		= pcsItem->m_eStatus;
	//CopyMemory(pcsBuffer->m_anGridPos, pcsItem->m_anGridPos, sizeof(INT16) * AGPDITEM_GRID_POS_NUM);
	pcsBuffer->m_anGridPos.MemCopy(0, &pcsItem->m_anGridPos[0], AGPDITEM_GRID_POS_NUM);
	pcsBuffer->m_posItem		= pcsItem->m_posItem;
	pcsBuffer->m_ulCID			= pcsItem->m_ulCID;
	pcsBuffer->m_lID			= pcsItem->m_lID;

	return TRUE; 
}

BOOL AgpmItem::RestoreItemStatus(AgpdItem *pcsItem, AgpdItem *pcsBuffer)
{
	if (!pcsItem || !pcsBuffer)
		return FALSE;

	pcsItem->m_posItem = pcsBuffer->m_posItem;
	//CopyMemory(pcsItem->m_anGridPos, pcsBuffer->m_anGridPos, sizeof(INT16) * AGPDITEM_GRID_POS_NUM);
	pcsItem->m_anGridPos.MemCopy(0, &pcsBuffer->m_anGridPos[0], AGPDITEM_GRID_POS_NUM);

	pcsItem->m_pcsCharacter		= NULL;
	pcsItem->m_ulCID			= pcsBuffer->m_ulCID;
	pcsItem->m_pcsCharacter		= m_pagpmCharacter->GetCharacter(pcsBuffer->m_ulCID);
	pcsItem->m_lID				= pcsBuffer->m_lID;

	switch (pcsBuffer->m_eStatus) {
	case AGPDITEM_STATUS_FIELD:
		AddItemToField(pcsItem);
		break;

	case AGPDITEM_STATUS_INVENTORY:
		if (pcsItem->m_pcsCharacter)
			AddItemToInventory(pcsItem->m_pcsCharacter, pcsItem, pcsItem->m_anGridPos[0], pcsItem->m_anGridPos[1], pcsItem->m_anGridPos[2]);
		break;

	case AGPDITEM_STATUS_EQUIP:
		if (pcsItem->m_pcsCharacter)
			EquipItem(pcsItem->m_pcsCharacter, GetADCharacter(pcsItem->m_pcsCharacter), pcsItem);
		break;

	case AGPDITEM_STATUS_BANK:
		if (pcsItem->m_pcsCharacter)
			AddItemToBank(pcsItem->m_pcsCharacter, pcsItem, pcsItem->m_anGridPos[0], pcsItem->m_anGridPos[1], pcsItem->m_anGridPos[2]);
		break;

	case AGPDITEM_STATUS_SUB_INVENTORY:
		if (pcsItem->m_pcsCharacter)
			AddItemToSubInventory(pcsItem->m_pcsCharacter, pcsItem, pcsItem->m_anGridPos[0], pcsItem->m_anGridPos[1], pcsItem->m_anGridPos[2]);
		break;

	case AGPDITEM_STATUS_CHARGING_INVENTORY:
		if (pcsItem->m_pcsCharacter)
			AddItemToChargingInventory(pcsItem->m_pcsCharacter, pcsItem, pcsItem->m_anGridPos[0], pcsItem->m_anGridPos[1], pcsItem->m_anGridPos[2]);
		break;

	default:
		break;
	}

	return TRUE;
}

BOOL AgpmItem::SetItemStackCount(AgpdItem *pcsItem, INT32 lCount, BOOL bIsSaveToDB)
{
	if (!pcsItem || lCount < 0)
		return FALSE;

	if (pcsItem->m_nCount == lCount)
		return TRUE;

	if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lMaxStackableCount < lCount)
		lCount	= ((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lMaxStackableCount;

	INT32	lPrevCount	= pcsItem->m_nCount;

	pcsItem->m_nCount = lCount;

	PVOID	pvBuffer[2];
	pvBuffer[0]	= (PVOID)	&lPrevCount;
	pvBuffer[1]	= IntToPtr(bIsSaveToDB);

	EnumCallback(ITEM_CB_ID_UPDATE_STACK_COUNT, pcsItem, pvBuffer);

	if (pcsItem->m_pcsCharacter &&
		pcsItem->m_eStatus == AGPDITEM_STATUS_INVENTORY)
	{
		// 만약 화살류의 아템이라면 카운트를 계산한다.
		if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lID == GetArrowTID())
		{
			AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsItem->m_pcsCharacter);

			pcsAgpdItemADChar->m_lNumArrowCount += lCount - lPrevCount;
		}
		else if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lID == GetBoltTID())
		{
			AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsItem->m_pcsCharacter);

			pcsAgpdItemADChar->m_lNumBoltCount += lCount - lPrevCount;
		}
	}

	return TRUE;
}

INT32 AgpmItem::GetItemStackCount(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	return pcsItem->m_nCount;
}

INT32 AgpmItem::AddItemStackCount(AgpdItem *pcsItem, INT32 lAddCount, BOOL bIsSaveToDB)
{
	if (!pcsItem)
		return (-1);

	if (lAddCount == 0)
		return pcsItem->m_nCount;

	INT32	lPrevCount	= pcsItem->m_nCount;

	pcsItem->m_nCount += lAddCount;

	if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lMaxStackableCount < pcsItem->m_nCount)
		pcsItem->m_nCount	= ((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lMaxStackableCount;

	PVOID	pvBuffer[2];
	pvBuffer[0]	= (PVOID)	&lPrevCount;
	pvBuffer[1]	= IntToPtr(bIsSaveToDB);

	EnumCallback(ITEM_CB_ID_UPDATE_STACK_COUNT, pcsItem, pvBuffer);

	if (pcsItem->m_pcsCharacter &&
		pcsItem->m_eStatus == AGPDITEM_STATUS_INVENTORY)
	{
		// 만약 화살류의 아템이라면 카운트를 계산한다.
		if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lID == GetArrowTID())
		{
			AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsItem->m_pcsCharacter);

			pcsAgpdItemADChar->m_lNumArrowCount += lAddCount;
		}
		else if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lID == GetBoltTID())
		{
			AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsItem->m_pcsCharacter);

			pcsAgpdItemADChar->m_lNumBoltCount += lAddCount;
		}
	}

	return pcsItem->m_nCount;
}

INT32 AgpmItem::SubItemStackCount(AgpdItem *pcsItem, INT32 lSubCount, BOOL bIsSaveToDB)
{
	if (!pcsItem || lSubCount == 0)
		return (-1);

	if (lSubCount == 0)
		return pcsItem->m_nCount;

	INT32	lPrevCount	= pcsItem->m_nCount;

	pcsItem->m_nCount -= lSubCount;
	if (pcsItem->m_nCount < 0)
		pcsItem->m_nCount = 0;

	PVOID	pvBuffer[2];
	pvBuffer[0]	= (PVOID)	&lPrevCount;
	pvBuffer[1]	= IntToPtr(bIsSaveToDB);

	EnumCallback(ITEM_CB_ID_UPDATE_STACK_COUNT, pcsItem, pvBuffer);

	if (pcsItem->m_pcsCharacter &&
		pcsItem->m_eStatus == AGPDITEM_STATUS_INVENTORY)
	{
		// 만약 화살류의 아템이라면 카운트를 계산한다.
		if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lID == GetArrowTID())
		{
			AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsItem->m_pcsCharacter);

			pcsAgpdItemADChar->m_lNumArrowCount -= lSubCount;
		}
		else if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lID == GetBoltTID())
		{
			AgpdItemADChar	*pcsAgpdItemADChar	= GetADCharacter(pcsItem->m_pcsCharacter);

			pcsAgpdItemADChar->m_lNumBoltCount -= lSubCount;
		}
	}

	// modified by bin. for auction
	if (pcsItem->m_nCount == 0 && AGPDITEM_STATUS_SALESBOX_GRID != pcsItem->m_eStatus)
	{
		ZeroMemory(pcsItem->m_szDeleteReason, sizeof(CHAR) * (AGPMITEM_MAX_DELETE_REASON + 1));
		strncpy(pcsItem->m_szDeleteReason, "stackcount == 0", AGPMITEM_MAX_DELETE_REASON);

		RemoveItem(pcsItem, TRUE);
		return 0;
	}

	return pcsItem->m_nCount;
}

INT32 AgpmItem::GetItemDurabilityPercent(AgpdItem *pcsAgpdItem)
{
	if (!pcsAgpdItem || !pcsAgpdItem->m_pcsItemTemplate)
		return (-1);

	INT32	lMaxTemplateItemDurability	= 0;
	m_pagpmFactors->GetValue(&pcsAgpdItem->m_pcsItemTemplate->m_csFactor, &lMaxTemplateItemDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);

	if (lMaxTemplateItemDurability <= 0)
		return (-1);

	INT32	lMaxItemDurability		= 0;
	INT32	lCurrentItemDurability	= 0;

	m_pagpmFactors->GetValue(&pcsAgpdItem->m_csFactor, &lCurrentItemDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);
	m_pagpmFactors->GetValue(&pcsAgpdItem->m_csFactor, &lMaxItemDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);

	if (lMaxItemDurability == 0 || lCurrentItemDurability == 0)
		return 0;

	FLOAT	tempPercent = (lCurrentItemDurability / (FLOAT) lMaxItemDurability * 100);
	if(tempPercent < 1.0f && tempPercent > 0.0001f)
		return 1;

	return (INT32) (lCurrentItemDurability / (FLOAT) lMaxItemDurability * 100);
}

// 2006.02.08. steeple
INT32 AgpmItem::GetItemDurabilityCurrent(AgpdItem *pcsAgpdItem)
{
	if(!pcsAgpdItem)
		return 0;

	INT32	lCurrentItemDurability	= 0;
	m_pagpmFactors->GetValue(&pcsAgpdItem->m_csFactor, &lCurrentItemDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);

	return lCurrentItemDurability;
}

// 2006.03.25. steeple
INT32 AgpmItem::GetItemDurabilityMax(AgpdItem* pcsAgpdItem)
{
	if(!pcsAgpdItem)
		return 0;

	INT32	lMaxItemDurability		= 0;
	m_pagpmFactors->GetValue(&pcsAgpdItem->m_csFactor, &lMaxItemDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);

	return lMaxItemDurability;
}

// 2006.02.08. steeple
INT32 AgpmItem::GetItemDurabilityMax(AgpdItemTemplate* pcsItemTemplate)
{
	if(!pcsItemTemplate)
		return 0;

	INT32	lMaxItemDurability		= 0;
	m_pagpmFactors->GetValue(&pcsItemTemplate->m_csFactor, &lMaxItemDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);

	return lMaxItemDurability;
}

BOOL AgpmItem::PickupItem(AgpdItem *pcsItem, INT32 lCID, BOOL *pbIsRemoveItem)
{
	if (!pcsItem || lCID == AP_INVALID_CID || !pbIsRemoveItem)
		return FALSE;

	// 현재 이 캐릭터가 아템을 집어들 수 있는 상태인지 검사한다.
	AgpdCharacter	*pcsCharacter	= m_pagpmCharacter->GetCharacter(lCID);
	if (!pcsCharacter)
		return FALSE;

	if (pcsCharacter->m_unActionStatus != AGPDCHAR_STATUS_NOT_ACTION &&
		pcsCharacter->m_unActionStatus != AGPDCHAR_STATUS_MOVE &&
		pcsCharacter->m_unActionStatus != AGPDCHAR_STATUS_NORMAL)
		return FALSE;

	// check bound type
	if (!CheckBoundType(pcsItem, pcsCharacter))
		return FALSE;

	// 선 루팅권이 세팅되어 있는지 살펴본다.
	BOOL	bCanPickup	= TRUE;

	*pbIsRemoveItem	= FALSE;

	stAgpmItemCheckPickupItem	stCheckPickupItem;
	stCheckPickupItem.pcsItem	= pcsItem;
	stCheckPickupItem.pcsCharacter	= pcsCharacter;

	INT8				cPickupResult		= 0;
	AgpdItemTemplate	*pcsItemTemplate	= (AgpdItemTemplate *) pcsItem->m_pcsItemTemplate;

	PVOID				pvBuffer[5];
	pvBuffer[0]			= (PVOID) pcsItemTemplate;
	pvBuffer[2]			= IntToPtr(lCID);

	if (pcsItemTemplate->m_lID != GetMoneyTID())
	{
		pvBuffer[3]			= (PVOID) pcsItem;
		pvBuffer[4]			= (PVOID) pcsItem;
	}
	else
	{
		pvBuffer[3]			= NULL;
		pvBuffer[4]			= NULL;
	}

	INT32	lIID		= pcsItem->m_lID;

	// check 한다.
	EnumCallback(ITEM_CB_ID_CHECK_CAN_PICKUP_ITEM, &stCheckPickupItem, &bCanPickup);

	pvBuffer[1]			= IntToPtr(stCheckPickupItem.lStackCount);

	AgpdItem	*pcsPickupItem	= NULL;

	if (!bCanPickup)	// pick up 할 수 없다.
	{
		cPickupResult			= AGPMITEM_PACKET_PICKUP_ITEM_RESULT_FAIL;
		EnumCallback(ITEM_CB_ID_PICKUP_ITEM_RESULT, &cPickupResult, pvBuffer);
		return FALSE;
	}
	else
	{
		/*
		// 인벤토리에 넣는다.
		BOOL bRetval = AddItemToInventory(lCID, pcsItem);
		*/

		pcsPickupItem	= GetItem(lIID);

		if (pcsPickupItem)
		{
			RemoveItemFromField(pcsItem, TRUE, FALSE);

			if ((GetBoundType(pcsItem) == E_AGPMITEM_BIND_ON_ACQUIRE) ||
				(GetBoundType(pcsItem) == E_AGPMITEM_BIND_ON_GUILDMASTER))
			{
				SetBoundOnOwner(pcsItem, pcsCharacter);
			}
		}

		pvBuffer[3]	= pcsPickupItem;

		cPickupResult			= AGPMITEM_PACKET_PICKUP_ITEM_RESULT_SUCCESS;
		EnumCallback(ITEM_CB_ID_PICKUP_ITEM_RESULT, &cPickupResult, pvBuffer);
	}

	if (!pcsPickupItem)
		*pbIsRemoveItem = TRUE;

	return TRUE;
}

BOOL AgpmItem::CBPickupItem(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgpmItem			*pThis			= (AgpmItem *)				pClass;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)			pData;
	AgpdCharacterAction	*pcsActionData	= (AgpdCharacterAction *)	pCustData;

	if (pThis->m_pagpmCharacter->IsAllBlockStatus(pcsCharacter))
		return FALSE;

	AgpdItem			*pcsItem		= pThis->GetItemLock(pcsActionData->m_csTargetBase.m_lID);
	if (!pcsItem)
	{
		return FALSE;
	}

	BOOL				bIsRemoveItem	= FALSE;

	BOOL	bResult	= pThis->PickupItem(pcsItem, pcsCharacter->m_lID, &bIsRemoveItem);

	if (!bIsRemoveItem)
		pcsItem->m_Mutex.Release();

	return bResult;
}

BOOL AgpmItem::CBCheckActionRequirement(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgpmItem						*pThis			= (AgpmItem *)						pClass;
	AgpdCharacter					*pcsCharacter	= (AgpdCharacter *)					pData;
	AgpdCharacterActionResultType	*peActionResult	= (AgpdCharacterActionResultType *)	pCustData;

	if (pThis->m_pagpmCharacter->IsAllBlockStatus(pcsCharacter))
		return FALSE;

	// 궁수인경우 인벤토리에 활통(화살)이 있어야 한다.
	///////////////////////////////////////////////////////////////////////
	INT32	lClass	= pThis->m_pagpmFactors->GetClass(&pcsCharacter->m_csFactor);

	if (lClass == AUCHARCLASS_TYPE_RANGER &&
		!pcsCharacter->m_bIsTrasform)
	{
		AgpdItem	*pcsWeaponItem	= pThis->GetEquipWeapon(pcsCharacter);
		if (pcsWeaponItem)
		{
			AgpdItemTemplate	*pcsWeaponTemplate	= (AgpdItemTemplate *) pcsWeaponItem->m_pcsItemTemplate;
			if (pcsWeaponTemplate)
			{
				switch (((AgpdItemTemplateEquipWeapon *) pcsWeaponTemplate)->m_nWeaponType) {
				case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_BOW:
					{
						AgpdItem		*pcsItemArrow		= pThis->GetArrowItemFromInventory(pcsCharacter);
						if (!pcsItemArrow)
						{
							*peActionResult	= AGPDCHAR_ACTION_RESULT_TYPE_NOT_ENOUGH_ARROW;
							return FALSE;
						}
					}
					break;

				case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_CROSSBOW:
					{
						AgpdItem		*pcsItemBolt		= pThis->GetBoltItemFromInventory(pcsCharacter);
						if (!pcsItemBolt)
						{
							*peActionResult = AGPDCHAR_ACTION_RESULT_TYPE_NOT_ENOUGH_BOLT;
							return FALSE;
						}
					}
					break;
				}
			}
		}
	}

	// 2005.11.30. steeple
	// 아래 주석을 풀어버림~
	if (lClass == AUCHARCLASS_TYPE_MAGE &&
		!pcsCharacter->m_bIsTrasform)
	{
		AgpdItem	*pcsWeaponItem	= pThis->GetEquipWeapon(pcsCharacter);
		if (pcsWeaponItem)
		{
			INT32	lCost	= pThis->m_pagpmCharacter->GetMeleeActionCostMP(pcsCharacter);
			if (lCost < 0)
			{
				*peActionResult	= AGPDCHAR_ACTION_RESULT_TYPE_NOT_ENOUGH_MP;
				return FALSE;
			}
			else if (lCost == 0)
				return TRUE;

			INT32	lCurrentMP	= 0;

			pThis->m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lCurrentMP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_MP);

			if (lCurrentMP < lCost)
			{
				*peActionResult	= AGPDCHAR_ACTION_RESULT_TYPE_NOT_ENOUGH_MP;
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL AgpmItem::CBPayActionCost(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgpmItem		*pThis			= (AgpmItem *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	INT16			nActionType		= *(INT16 *)		pCustData;

	switch ((AgpdCharacterActionType) nActionType) {
	case AGPDCHAR_ACTION_TYPE_ATTACK:
		{
			// 궁수인경우 화살(혹은 볼트)을 하나 줄여야 한다.
			/////////////////////////////////////////////////////
			INT32	lClass	= pThis->m_pagpmFactors->GetClass(&pcsCharacter->m_csFactor);

			if (lClass == AUCHARCLASS_TYPE_RANGER &&
				!pcsCharacter->m_bIsTrasform)
			{
				AgpdItem		*pcsWeaponItem	= pThis->GetEquipWeapon(pcsCharacter);
				if (pcsWeaponItem)
				{
					AgpdItemADChar	*pcsItemADChar	= pThis->GetADCharacter(pcsCharacter);
					if (!pcsItemADChar)
						return FALSE;

					AgpdItemTemplate	*pcsWeaponTemplate	= (AgpdItemTemplate *) pcsWeaponItem->m_pcsItemTemplate;
					if (pcsWeaponTemplate)
					{
						switch (((AgpdItemTemplateEquipWeapon *) pcsWeaponTemplate)->m_nWeaponType) {
						case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_BOW:
							{
								return pThis->SubItemStackCount(pThis->GetItem(pcsItemADChar->m_lArrowIDInInventory), 1);
							}
							break;

						case AGPMITEM_EQUIP_WEAPON_TYPE_TWO_HAND_CROSSBOW:
							{
								return pThis->SubItemStackCount(pThis->GetItem(pcsItemADChar->m_lBoltIDInInventory), 1);
							}
							break;
						}
					}
				}
			}
		}
		break;
	}

	return TRUE;
}

AgpdItem* AgpmItem::GetArrowItemFromInventory(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return FALSE;

	AgpdItem	*pcsItemArrow	= NULL;

	if (pcsItemADChar->m_lArrowIDInInventory != AP_INVALID_IID)
	{
		pcsItemArrow	= GetItem(pcsItemADChar->m_lArrowIDInInventory);
		if (pcsItemArrow)
		{
			if (pcsItemArrow->m_eStatus != AGPDITEM_STATUS_INVENTORY ||
				pcsItemArrow->m_ulCID != pcsCharacter->m_lID)
				pcsItemArrow = NULL;
		}
	}

	if (!pcsItemArrow)
	{
		pcsItemArrow = GetInventoryItemByTID(pcsCharacter, GetArrowTID());
		if (pcsItemArrow)
			pcsItemADChar->m_lArrowIDInInventory	= pcsItemArrow->m_lID;
	}

	return pcsItemArrow;
}

AgpdItem* AgpmItem::GetBoltItemFromInventory(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return FALSE;

	AgpdItem	*pcsItemBolt	= NULL;

	if (pcsItemADChar->m_lBoltIDInInventory != AP_INVALID_IID)
	{
		pcsItemBolt	= GetItem(pcsItemADChar->m_lBoltIDInInventory);
		if (pcsItemBolt)
		{
			if (pcsItemBolt->m_eStatus != AGPDITEM_STATUS_INVENTORY ||
				pcsItemBolt->m_ulCID != pcsCharacter->m_lID)
				pcsItemBolt = NULL;
		}
	}

	if (!pcsItemBolt)
	{
		pcsItemBolt = GetInventoryItemByTID(pcsCharacter, GetBoltTID());
		if (pcsItemBolt)
			pcsItemADChar->m_lBoltIDInInventory	= pcsItemBolt->m_lID;
	}

	return pcsItemBolt;
}

AgpdItem* AgpmItem::GetHPPotionInInventory(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return NULL;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return NULL;

	AgpdItem	*pcsHPPotion		= NULL;

	if (pcsItemADChar->m_lHPPotionIDInInventory != AP_INVALID_IID)
		pcsHPPotion	= GetItem(pcsItemADChar->m_lHPPotionIDInInventory);

	if (!pcsHPPotion)
	{
		// 인벤토리 전체를 뒤져 찾아온다.
		pcsHPPotion	= GetInventoryPotionItem(pcsCharacter, AGPMITEM_USABLE_POTION_TYPE_HP);
		if (pcsHPPotion)
			pcsItemADChar->m_lHPPotionIDInInventory	= pcsHPPotion->m_lID;
	}

	return pcsHPPotion;
}

AgpdItem* AgpmItem::GetMPPotionInInventory(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return NULL;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return NULL;

	AgpdItem	*pcsMPPotion		= NULL;

	if (pcsItemADChar->m_lMPPotionIDInInventory != AP_INVALID_IID)
		pcsMPPotion	= GetItem(pcsItemADChar->m_lMPPotionIDInInventory);

	if (!pcsMPPotion)
	{
		// 인벤토리 전체를 뒤져 찾아온다.
		pcsMPPotion	= GetInventoryPotionItem(pcsCharacter, AGPMITEM_USABLE_POTION_TYPE_MP);
		if (pcsMPPotion)
			pcsItemADChar->m_lMPPotionIDInInventory	= pcsMPPotion->m_lID;
	}

	return pcsMPPotion;
}

AgpdItem* AgpmItem::GetSPPotionInInventory(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return NULL;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return NULL;

	AgpdItem	*pcsSPPotion		= NULL;

	if (pcsItemADChar->m_lSPPotionIDInInventory != AP_INVALID_IID)
		pcsSPPotion	= GetItem(pcsItemADChar->m_lSPPotionIDInInventory);

	if (!pcsSPPotion)
	{
		// 인벤토리 전체를 뒤져 찾아온다.
		pcsSPPotion	= GetInventoryPotionItem(pcsCharacter, AGPMITEM_USABLE_POTION_TYPE_SP);
		if (pcsSPPotion)
			pcsItemADChar->m_lSPPotionIDInInventory	= pcsSPPotion->m_lID;
	}

	return pcsSPPotion;
}

BOOL AgpmItem::ProcessRemove(UINT32 ulClockCount)
{
	if (m_ulPrevRemoveClockCount + AGPMITEM_PROCESS_REMOVE_INTERVAL > ulClockCount)
		return TRUE;

	INT32 lIndex = 0;

	AgpdItem **ppcsItem = (AgpdItem **)m_csAdminItemRemove.GetObjectSequence(&lIndex);

	while (ppcsItem && *ppcsItem)
	{
		AgpdItem *pcsItem = *ppcsItem;

		if (pcsItem->m_ulRemoveTimeMSec + g_ulReserveItemData < ulClockCount)
		{
			m_csAdminItemRemove.RemoveObject((INT_PTR) pcsItem);
			lIndex = 0;

			DestroyItem(pcsItem);
		}

		ppcsItem = (AgpdItem **) m_csAdminItemRemove.GetObjectSequence(&lIndex);
	}

	m_ulPrevRemoveClockCount = ulClockCount;

	return TRUE;
}

BOOL AgpmItem::AddRemoveItem(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	pcsItem->m_ulRemoveTimeMSec	= GetClockCount();

	if (!m_csAdminItemRemove.AddObject(&pcsItem, (INT_PTR) pcsItem))
	{
		/*
		AgpdItem	**ppcsItem = (AgpdItem **) m_csAdminItemRemove.GetObject((INT32) pcsItem);
		if (ppcsItem && *ppcsItem)
		{
			//ASSERT(!"이러면 죽을지도 몰라요~!!!!!, AgpmItem::AddRemoveItem()");

			m_csAdminItemRemove.RemoveObject((INT32) (*ppcsItem));

			DestroyItem(*ppcsItem);

			if (m_csAdminItemRemove.AddObject(&pcsItem, (INT32) pcsItem))
				return TRUE;
		}

		return FALSE;
		*/
	}

	return TRUE;
}

BOOL AgpmItem::CheckMovableItemBetweenGrid(AgpdItem *pcsItem, AgpdItemStatus eNewStatus)
{
	if (!pcsItem)
		return FALSE;

	switch (pcsItem->m_eStatus) {
	case AGPDITEM_STATUS_NPC_TRADE:
		{
			if (eNewStatus != AGPDITEM_STATUS_NPC_TRADE)
				return FALSE;
		}
		break;

	case AGPDITEM_STATUS_INVENTORY:
		{
			if ((eNewStatus == AGPDITEM_STATUS_BANK) && !IsNotBound(pcsItem))
				return FALSE;
		}
		break;
	}

	return TRUE;
}

BOOL AgpmItem::RequireLevelUp(AgpdItem *pcsItem, INT32 lLevel)
{
	if (!pcsItem)
		return FALSE;

	if (lLevel == 0)
		return TRUE;

	INT32	lPrevRequireLevel	= 0;

	m_pagpmFactors->GetValue(&pcsItem->m_csRestrictFactor, &lPrevRequireLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);

	INT32	lNewRequireLevel	= lPrevRequireLevel + lLevel;
	if (lNewRequireLevel > AGPMCHAR_MAX_LEVEL)
		lNewRequireLevel	= AGPMCHAR_MAX_LEVEL;

	m_pagpmFactors->SetValue(&pcsItem->m_csRestrictFactor, lNewRequireLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);

	EnumCallback(ITEM_CB_ID_UPDATE_REQUIRE_LEVEL, pcsItem, IntToPtr(lPrevRequireLevel));

	return TRUE;
}

BOOL AgpmItem::RequireLevelDown(AgpdItem *pcsItem, INT32 lLevel)
{
	if (!pcsItem)
		return FALSE;

	if (lLevel == 0)
		return TRUE;

	INT32	lPrevRequireLevel	= 0;

	m_pagpmFactors->GetValue(&pcsItem->m_csRestrictFactor, &lPrevRequireLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);

	INT32	lNewRequireLevel	= lPrevRequireLevel - lLevel;
	if (lNewRequireLevel < 1)
		lNewRequireLevel	= 1;

	m_pagpmFactors->SetValue(&pcsItem->m_csRestrictFactor, lNewRequireLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);

	EnumCallback(ITEM_CB_ID_UPDATE_REQUIRE_LEVEL, pcsItem, IntToPtr(lPrevRequireLevel));

	return TRUE;
}

BOOL AgpmItem::SetRemovePolearm()
{
	m_bIsRemovePolearm	= TRUE;

	return TRUE;
}

AgpdItem* AgpmItem::SplitItem(AgpdItem *pcsItem, INT32 lSplitStackCount, AgpdItemStatus eTargetStatus, INT32 lLayer, INT32 lRow, INT32 lColumn)
{
	if (!pcsItem ||
		!pcsItem->m_pcsCharacter ||
		lSplitStackCount == 0 ||
		pcsItem->m_nCount <= lSplitStackCount ||
		!pcsItem->m_pcsItemTemplate ||
		!((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_bStackable)
		return NULL;

	AgpdGrid	*pcsTargetGrid	= NULL;

	AgpdItemADChar	*pcsAttachData	= GetADCharacter(pcsItem->m_pcsCharacter);

	// Trade 상태에서 TradeGrid가 아닌 다른 곳으로 Split 시도시 실패;
	if (pcsAttachData->m_lTradeStatus != AGPDITEM_TRADE_STATUS_NONE &&
		eTargetStatus != AGPDITEM_STATUS_TRADE_GRID)
		return NULL;

	if (pcsItem->m_eStatus != AGPDITEM_STATUS_INVENTORY && pcsItem->m_eStatus != AGPDITEM_STATUS_SUB_INVENTORY && pcsItem->m_eStatus != AGPDITEM_STATUS_CHARGING_INVENTORY)
		return NULL;

	// 2007.02.27. steeple
	// 귀속아이템은 나눈 놈도 귀속으로 해준다.
	BOOL bIsBoundOnOwner = IsBoundOnOwner(pcsItem);

	// eTargetStatus에 빈 공간이 있는지 체크한다.
	switch (eTargetStatus) {
	case AGPDITEM_STATUS_INVENTORY:
		pcsTargetGrid	= &pcsAttachData->m_csInventoryGrid;
		break;

	case AGPDITEM_STATUS_TRADE_GRID:
		pcsTargetGrid	= &pcsAttachData->m_csTradeGrid;
		break;

	case AGPDITEM_STATUS_SUB_INVENTORY:
		pcsTargetGrid	= &pcsAttachData->m_csSubInventoryGrid;
		break;

	case AGPDITEM_STATUS_CHARGING_INVENTORY:
		pcsTargetGrid	= &pcsAttachData->m_csChargingInventoryGrid;
		break;

	default :
		return NULL;
	}

	if (m_pagpmGrid->IsFullGrid(pcsTargetGrid))
		return NULL;

	AgpdItem	*pcsNewItem	= NULL;

	PVOID		pvBuffer[3];
	pvBuffer[0]		= IntToPtr(lSplitStackCount);
	pvBuffer[1]		= (PVOID) &pcsNewItem;
	pvBuffer[2]		= (PVOID) &eTargetStatus;

	EnumCallback(ITEM_CB_ID_SPLIT_ITEM, pcsItem, pvBuffer);

	if (!pcsNewItem)
		return NULL;

	INT16	nLayer	= -1;
	INT16	nRow	= -1;
	INT16	nColumn	= -1;

	if (m_pagpmGrid->IsEmptyGrid(pcsTargetGrid, lLayer, lRow, lColumn, 1, 1))
	{
		nLayer		= (INT16) lLayer;
		nRow		= (INT16) lRow;
		nColumn		= (INT16) lColumn;
	}

	pcsNewItem->m_eStatus = AGPDITEM_STATUS_NOTSETTING;

	// 2007.02.26. steeple
	if(bIsBoundOnOwner)
	{
		SetBoundType(pcsNewItem, GetBoundType(pcsItem));
		SetBoundOnOwner(pcsNewItem, pcsItem->m_pcsCharacter);
	}

	switch (eTargetStatus) {
	case AGPDITEM_STATUS_INVENTORY:
		AddItemToInventory(pcsItem->m_pcsCharacter, pcsNewItem, nLayer, nRow, nColumn);
		break;

	case AGPDITEM_STATUS_TRADE_GRID:
		AddItemToTradeGrid(pcsItem->m_pcsCharacter, pcsNewItem, nLayer, nRow, nColumn);
		break;

	case AGPDITEM_STATUS_SUB_INVENTORY:
		AddItemToSubInventory(pcsItem->m_pcsCharacter, pcsNewItem, nLayer, nRow, nColumn);
		break;
	
	case AGPDITEM_STATUS_CHARGING_INVENTORY:
		AddItemToChargingInventory(pcsItem->m_pcsCharacter, pcsNewItem, nLayer, nRow, nColumn);
		break;
	}

	return pcsNewItem;
}

AgpdItem* AgpmItem::SplitItem(AgpdItem *pcsItem, INT32 lSplitStackCount, AuPOS *pstDestPos)
{
	if (!pcsItem ||
		!pcsItem->m_pcsCharacter ||
		lSplitStackCount == 0 ||
		!pstDestPos ||
		pcsItem->m_nCount <= lSplitStackCount ||
		!pcsItem->m_pcsItemTemplate ||
		!((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_bStackable)
		return NULL;

	// 2007.02.27. steeple
	// 귀속아이템은 나눈 놈도 귀속으로 해준다.
	BOOL bIsBoundOnOwner = IsBoundOnOwner(pcsItem);

	AgpdItem	*pcsNewItem	= NULL;

	PVOID		pvBuffer[2];
	pvBuffer[0]		= IntToPtr(lSplitStackCount);
	pvBuffer[1]		= (PVOID) &pcsNewItem;

	EnumCallback(ITEM_CB_ID_SPLIT_ITEM, pcsItem, pvBuffer);

	if (!pcsNewItem)
		return NULL;

	// 2007.02.26. steeple
	if(bIsBoundOnOwner)
	{
		SetBoundType(pcsNewItem, GetBoundType(pcsItem));
		SetBoundOnOwner(pcsNewItem, pcsItem->m_pcsCharacter);
	}

	AddItemToField(pcsNewItem);

	return pcsNewItem;
}

BOOL AgpmItem::CheckJoinItem(AgpdItem *pcsItem1, AgpdItem *pcsItem2)
{
	if (!pcsItem1 ||
		!pcsItem1->m_pcsItemTemplate ||
		!pcsItem2 ||
		!pcsItem2->m_pcsItemTemplate)
		return FALSE;

	if (!((AgpdItemTemplate *) pcsItem1->m_pcsItemTemplate)->m_bStackable ||
		!((AgpdItemTemplate *) pcsItem2->m_pcsItemTemplate)->m_bStackable)
		return FALSE;

	// 스킬 북 같은 경우 등등.. 조건에 따라 합칠지 안합칠지 여부를 따져봐야 한다.
	// 콜벡 불러준다.

	BOOL	bCheckJoinItem	= TRUE;

	PVOID	pvBuffer[3];
	pvBuffer[0]	=	(PVOID)	pcsItem1;
	pvBuffer[1]	=	(PVOID)	pcsItem2;
	pvBuffer[2]	=	(PVOID)	&bCheckJoinItem;

	EnumCallback(ITEM_CB_ID_CHECK_JOIN_ITEM, pvBuffer, NULL);

	return bCheckJoinItem;
}

AgpdItem* AgpmItem::GetItem(AgpdGridItem *pcsGridItem)
{
	if (!pcsGridItem)
		return NULL;

	if (pcsGridItem->GetParentBase() && ((AgpdItem *) pcsGridItem->GetParentBase())->m_pcsItemTemplate)
		return (AgpdItem *) pcsGridItem->GetParentBase();

	return GetItem(pcsGridItem->m_lItemID);
}

AgpdItem* AgpmItem::GetItemLock(AgpdGridItem *pcsGridItem)
{
	AgpdItem	*pcsItem	= GetItem(pcsGridItem);
	if (pcsItem && pcsItem->m_Mutex.WLock())
		return pcsItem;

	return NULL;
}

BOOL AgpmItem::SetBoundType(AgpdItemTemplate *pcsItemTemplate, AgpmItemBoundTypes eBoundType)
{
	if (!pcsItemTemplate)
		return FALSE;

	pcsItemTemplate->m_lStatusFlag &= ~AGPMITEM_BIND_ON_ACQUIRE; 
	pcsItemTemplate->m_lStatusFlag &= ~AGPMITEM_BIND_ON_EQUIP;
	pcsItemTemplate->m_lStatusFlag &= ~AGPMITEM_BIND_ON_USE;

	if (eBoundType == E_AGPMITEM_BIND_ON_ACQUIRE)
		pcsItemTemplate->m_lStatusFlag |= AGPMITEM_BIND_ON_ACQUIRE;
	else if (eBoundType == E_AGPMITEM_BIND_ON_EQUIP)
		pcsItemTemplate->m_lStatusFlag |= AGPMITEM_BIND_ON_EQUIP;
	else if (eBoundType == E_AGPMITEM_BIND_ON_USE)
		pcsItemTemplate->m_lStatusFlag |= AGPMITEM_BIND_ON_USE;

	return TRUE;
}

AgpmItemBoundTypes AgpmItem::GetBoundType(AgpdItemTemplate *pcsItemTemplate)
{
	if (!pcsItemTemplate)
		return E_AGPMITEM_NOT_BOUND;

	if (pcsItemTemplate->m_lStatusFlag & AGPMITEM_BIND_ON_ACQUIRE)
		return E_AGPMITEM_BIND_ON_ACQUIRE;
	else if (pcsItemTemplate->m_lStatusFlag & AGPMITEM_BIND_ON_EQUIP)
		return E_AGPMITEM_BIND_ON_EQUIP;
	else if (pcsItemTemplate->m_lStatusFlag & AGPMITEM_BIND_ON_USE)
		return E_AGPMITEM_BIND_ON_USE;
	
	return E_AGPMITEM_NOT_BOUND;
}

BOOL AgpmItem::SetBoundType(AgpdItem *pcsItem, AgpmItemBoundTypes eBoundType)
{
	if (!pcsItem)
		return FALSE;

	if (pcsItem->m_lStatusFlag & AGPMITEM_BOUND_ON_OWNER)
		return FALSE;

	pcsItem->m_lStatusFlag &= ~AGPMITEM_BIND_ON_ACQUIRE; 
	pcsItem->m_lStatusFlag &= ~AGPMITEM_BIND_ON_EQUIP;
	pcsItem->m_lStatusFlag &= ~AGPMITEM_BIND_ON_USE;

	if (eBoundType == E_AGPMITEM_BIND_ON_ACQUIRE)
		pcsItem->m_lStatusFlag |= AGPMITEM_BIND_ON_ACQUIRE;
	else if (eBoundType == E_AGPMITEM_BIND_ON_EQUIP)
		pcsItem->m_lStatusFlag |= AGPMITEM_BIND_ON_EQUIP;
	else if (eBoundType == E_AGPMITEM_BIND_ON_USE)
		pcsItem->m_lStatusFlag |= AGPMITEM_BIND_ON_USE;

	UpdateItemStatusFlag(pcsItem);

	return TRUE;
}

AgpmItemBoundTypes AgpmItem::GetBoundType(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return E_AGPMITEM_NOT_BOUND;

	if (pcsItem->m_lStatusFlag & AGPMITEM_BIND_ON_ACQUIRE)
		return E_AGPMITEM_BIND_ON_ACQUIRE;
	else if (pcsItem->m_lStatusFlag & AGPMITEM_BIND_ON_EQUIP)
		return E_AGPMITEM_BIND_ON_EQUIP;
	else if (pcsItem->m_lStatusFlag & AGPMITEM_BIND_ON_USE)
		return E_AGPMITEM_BIND_ON_USE;
	
	return E_AGPMITEM_NOT_BOUND;
}

BOOL		AgpmItem::SetShrineType(AgpdItem *pcsItem)
{
	if( !pcsItem )
		return FALSE;

	pcsItem->m_lStatusFlag |= AGPMITEM_BOUND_SHRINE_ITEM;

	return TRUE;
}

BOOL		AgpmItem::SetShrineType(AgpdItemTemplate *pcsItemTemplate)
{
	if( !pcsItemTemplate )
		return FALSE;

	pcsItemTemplate->m_lStatusFlag	|=	AGPMITEM_BOUND_SHRINE_ITEM;
	return TRUE;
}

BOOL		AgpmItem::IsShrineType(AgpdItem* pcsItem)
{
	if( !pcsItem )
		return FALSE;

	return	(pcsItem->m_lStatusFlag & AGPMITEM_BOUND_SHRINE_ITEM);
}

BOOL		AgpmItem::IsShrineType(AgpdItemTemplate* pcsItemTemplate)
{
	if( !pcsItemTemplate )
		return FALSE;

	return	(pcsItemTemplate->m_lStatusFlag & AGPMITEM_BOUND_SHRINE_ITEM);
}

BOOL AgpmItem::IsNotBound(AgpdItem *pcsItem)
{
	AgpmItemBoundTypes	eBoundType	= GetBoundType(pcsItem);

	if (eBoundType == E_AGPMITEM_NOT_BOUND)
		return TRUE;

	return FALSE;
}

BOOL AgpmItem::CheckBoundType(AgpdItem *pcsItem, AgpdCharacter *pcsCharacter)
{
	if (!pcsItem || !pcsCharacter)
		return FALSE;

	AgpmItemBoundTypes	eBoundType	= GetBoundType(pcsItem);

	if (eBoundType == E_AGPMITEM_NOT_BOUND)
		return TRUE;
	else
	{
		if (pcsItem->m_lStatusFlag & AGPMITEM_BOUND_ON_OWNER)
		{
			if (pcsItem->m_pcsCharacter)
			{
				if (pcsItem->m_pcsCharacter == pcsCharacter)
					return TRUE;
				else
					return FALSE;
			}
			else
			{
				if (pcsItem->m_pcsBoundOwner == pcsCharacter)
					return TRUE;
				else
					return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL AgpmItem::IsBoundOnOwner(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	if (pcsItem->m_lStatusFlag & AGPMITEM_BOUND_ON_OWNER)
		return TRUE;

	return FALSE;
}

BOOL AgpmItem::IsEventItem( AgpdItem* pcsItem )
{
	if( !pcsItem )
		return FALSE;

	if( pcsItem->m_pcsItemTemplate->m_bIsEventItem )
		return TRUE;

	return FALSE;
}

BOOL AgpmItem::SetBoundOnOwner(AgpdItem *pcsItem, AgpdCharacter *pcsCharacter)
{
	if (!pcsItem)
		return FALSE;

	pcsItem->m_lStatusFlag		|= AGPMITEM_BOUND_ON_OWNER;
	pcsItem->m_pcsBoundOwner	= pcsCharacter;

	UpdateItemStatusFlag(pcsItem);

	return TRUE;
}

BOOL AgpmItem::UpdateItemStatusFlag(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	EnumCallback(ITEM_CB_ID_UPDATE_ITEM_STATUS_FLAG, pcsItem, NULL);

	return TRUE;
}

BOOL AgpmItem::SetQuestType(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	pcsItem->m_lStatusFlag		|= AGPMITEM_STATUS_QUEST;

	UpdateItemStatusFlag(pcsItem);

	return TRUE;
}

BOOL AgpmItem::ResetQuestType(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	pcsItem->m_lStatusFlag		^= AGPMITEM_STATUS_QUEST;

	UpdateItemStatusFlag(pcsItem);

	return TRUE;
}

BOOL AgpmItem::IsQuestItem(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	return (pcsItem->m_lStatusFlag & AGPMITEM_STATUS_QUEST);
}

void AgpmItem::GetNearPosition(AuPOS *pcsSrcPos, AuPOS *pcsDestPos)
{
	ASSERT(pcsSrcPos);
	ASSERT(pcsDestPos);

	*pcsDestPos = *pcsSrcPos;

	INT32	lRandomX	= m_csRandom.randInt(100);
	INT32	lRandomZ	= m_csRandom.randInt(100);

	if (lRandomX > 50)
		pcsDestPos->x	+= lRandomX;
	else
		pcsDestPos->x	-= (lRandomX + 50);

	if (lRandomZ > 50)
		pcsDestPos->z	+= lRandomZ;
	else
		pcsDestPos->z	-= (lRandomZ + 50);
}

BOOL AgpmItem::IsProperPart(AgpdItemTemplate *pcsItemTemplate, AgpdItemOptionTemplate *pcsItemOptionTemplate)
{
	if (!pcsItemTemplate || !pcsItemOptionTemplate)
		return FALSE;

	if (pcsItemTemplate->m_nType != AGPMITEM_TYPE_EQUIP)
		return FALSE;

	switch (((AgpdItemTemplateEquip *) pcsItemTemplate)->m_nKind) {
		case AGPMITEM_EQUIP_KIND_WEAPON:
			{
				return pcsItemOptionTemplate->m_ulSetPart & AGPDITEM_OPTION_SET_TYPE_WEAPON;
			}
			break;

		case AGPMITEM_EQUIP_KIND_SHIELD:
			{
				return pcsItemOptionTemplate->m_ulSetPart & AGPDITEM_OPTION_SET_TYPE_SHIELD;
			}
			break;

		case AGPMITEM_EQUIP_KIND_RING:
			{
				return pcsItemOptionTemplate->m_ulSetPart & AGPDITEM_OPTION_SET_TYPE_RING;
			}
			break;

		case AGPMITEM_EQUIP_KIND_NECKLACE:
			{
				return pcsItemOptionTemplate->m_ulSetPart & AGPDITEM_OPTION_SET_TYPE_NECKLACE;
			}
			break;

		case AGPMITEM_EQUIP_KIND_ARMOUR:
			{
				switch (((AgpdItemTemplateEquip *) pcsItemTemplate)->m_nPart) {
					case AGPMITEM_PART_BODY:
						{
							return pcsItemOptionTemplate->m_ulSetPart & AGPDITEM_OPTION_SET_TYPE_BODY;
						}
						break;

					case AGPMITEM_PART_HEAD:
						{
							return pcsItemOptionTemplate->m_ulSetPart & AGPDITEM_OPTION_SET_TYPE_HEAD;
						}
						break;

					case AGPMITEM_PART_ARMS:
						{
						}
						break;

					case AGPMITEM_PART_HANDS:
						{
							return pcsItemOptionTemplate->m_ulSetPart & AGPDITEM_OPTION_SET_TYPE_HANDS;
						}
						break;

					case AGPMITEM_PART_LEGS:
						{
							return pcsItemOptionTemplate->m_ulSetPart & AGPDITEM_OPTION_SET_TYPE_LEGS;
						}
						break;

					case AGPMITEM_PART_FOOT:
						{
							return pcsItemOptionTemplate->m_ulSetPart & AGPDITEM_OPTION_SET_TYPE_FOOTS;
						}
						break;
				}
			}
			break;
	}

	return FALSE;
}

BOOL AgpmItem::IsAlreadySetType(AgpdItem *pcsItem, AgpdItemOptionTemplate *pcsItemOptionTemplate)
{
	if (!pcsItem || !pcsItemOptionTemplate)
		return TRUE;

	for (int i = 0; i < AGPDITEM_OPTION_MAX_NUM; ++i)
	{
		if (!pcsItem->m_apcsOptionTemplate[i])
			break;

		if (pcsItem->m_apcsOptionTemplate[i]->m_lType == pcsItemOptionTemplate->m_lType)
			return TRUE;
	}

	return FALSE;
}

BOOL AgpmItem::AddItemOption(AgpdItem *pcsItem, INT32 lItemOptionTID, BOOL bIsValidCheck)
{
	if (!pcsItem)
		return FALSE;

	return AddItemOption(pcsItem, GetItemOptionTemplate(lItemOptionTID), bIsValidCheck);
}

BOOL AgpmItem::AddItemOption(AgpdItem *pcsItem, AgpdItemOptionTemplate *pcsItemOptionTemplate, BOOL bIsValidCheck)
{
	if (!pcsItem || !pcsItemOptionTemplate)
		return FALSE;

	if (bIsValidCheck)
	{
		if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lMaxOptionNum > AGPDITEM_OPTION_MAX_NUM)
			((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lMaxOptionNum	= AGPDITEM_OPTION_MAX_NUM;

		if (((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lMaxOptionNum <= 0 ||
			pcsItem->m_aunOptionTID[((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_lMaxOptionNum - 1] != 0)
			return FALSE;

		if (IsAlreadySetType(pcsItem, pcsItemOptionTemplate))
			return FALSE;

		// 2007.01.31. steeple
		// 옵션이 날라갔다.....
		if (!IsProperPart((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate, pcsItemOptionTemplate))
			return FALSE;
	}

	int i = 0;
	for (i = 0; i < AGPDITEM_OPTION_MAX_NUM; ++i)
	{
		if (pcsItem->m_aunOptionTID[i] == 0)
			break;
	}

	if (i == AGPDITEM_OPTION_MAX_NUM)
		return FALSE;

	pcsItem->m_aunOptionTID[i]			= pcsItemOptionTemplate->m_lID;
	pcsItem->m_apcsOptionTemplate[i]	= pcsItemOptionTemplate;

	m_pagpmFactors->CalcFactor(&pcsItem->m_csFactor, &pcsItemOptionTemplate->m_csFactor, TRUE, FALSE, TRUE);
	m_pagpmFactors->CalcFactor(&pcsItem->m_csFactorPercent, &pcsItemOptionTemplate->m_csFactorPercent, TRUE, FALSE, TRUE);

	EnumCallback(ITEM_CB_ID_ADD_ITEM_OPTION, pcsItem, pcsItemOptionTemplate);

	return TRUE;
}

BOOL AgpmItem::AddRefineItemOption(AgpdItem *pcsItem, AgpdItemOptionTemplate *pcsItemOptionTemplate)
{
    if (!pcsItem || !pcsItemOptionTemplate)
        return FALSE;

    if (IsAlreadySetType(pcsItem, pcsItemOptionTemplate))
        return FALSE;

    // 2007.01.31. steeple
    // 옵션이 날라갔다.....
    if (!IsProperPart((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate, pcsItemOptionTemplate))
        return FALSE;

    int i = 0;
    for (i = 0; i < AGPDITEM_OPTION_MAX_NUM; ++i)
    {
        if (pcsItem->m_aunOptionTID[i] == 0)
            break;
    }

    if (i == AGPDITEM_OPTION_MAX_NUM)
        return FALSE;

    pcsItem->m_aunOptionTID[i]			= pcsItemOptionTemplate->m_lID;
    pcsItem->m_apcsOptionTemplate[i]	= pcsItemOptionTemplate;

#ifdef _DEBUG
		char szOptionLog[256]={0,};
		sprintf(szOptionLog,"OptID[%d] OptName[%s]\n", pcsItemOptionTemplate->m_lID, pcsItemOptionTemplate->m_szDescription);
		OutputDebugString(szOptionLog);
#endif

    m_pagpmFactors->CalcFactor(&pcsItem->m_csFactor, &pcsItemOptionTemplate->m_csFactor, TRUE, FALSE, TRUE);
    m_pagpmFactors->CalcFactor(&pcsItem->m_csFactorPercent, &pcsItemOptionTemplate->m_csFactorPercent, TRUE, FALSE, TRUE);

    EnumCallback(ITEM_CB_ID_ADD_ITEM_OPTION, pcsItem, pcsItemOptionTemplate);

    return TRUE;
}

BOOL AgpmItem::CalcItemOptionFactor(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	for (int i = 0; i < AGPDITEM_OPTION_MAX_NUM; ++i)
	{
		if (pcsItem->m_aunOptionTID[i] == 0 )
			break;

		if ( pcsItem->m_apcsOptionTemplate[i] == NULL )
		{
			// 마고자: 일단 에러내고 죽지 않게 수정.
			ASSERT(!"옵션 템플릿 아이디가 있지만 템플릿이 널이다." );
			continue;
		}

		m_pagpmFactors->CalcFactor(&pcsItem->m_csFactor, &pcsItem->m_apcsOptionTemplate[i]->m_csFactor, TRUE, FALSE, TRUE);
		m_pagpmFactors->CalcFactor(&pcsItem->m_csFactorPercent, &pcsItem->m_apcsOptionTemplate[i]->m_csFactorPercent, TRUE, FALSE, TRUE);

		EnumCallback(ITEM_CB_ID_ADD_ITEM_OPTION, pcsItem, pcsItem->m_apcsOptionTemplate[i]);
	}

	return TRUE;
}

// 2007.02.05. steeple
// Skill Plus
BOOL AgpmItem::AddItemSkillPlus(AgpdItem* pcsItem, INT32 lSkillPlusTID)
{
	if(!pcsItem)
		return FALSE;

	int i = 0;
	for(i; i < AGPMITEM_MAX_SKILL_PLUS_EFFECT; ++i)
	{
		if(pcsItem->m_aunSkillPlusTID[i] == 0)
			break;
	}

	if(i == AGPMITEM_MAX_SKILL_PLUS_EFFECT)
		return FALSE;

	pcsItem->m_aunSkillPlusTID[i] = lSkillPlusTID;

	return TRUE;
}

// 2007.02.08. steeple
// Return skill plus count is equiped per skill tid
INT32 AgpmItem::GefEffectedSkillPlusLevel(AgpdCharacter* pcsCharacter, INT32 lSkillTID)
{
	if(!pcsCharacter || !lSkillTID)
		return 0;

	AgpdGridItem* pcsGridItem = NULL;
	AgpdItem* pcsItem = NULL;
	INT32 lLevel = 0;

	for(INT32 lPart = AGPMITEM_PART_NONE + 1; lPart < AGPMITEM_PART_NUM; ++lPart)
	{
		pcsGridItem = GetEquipItem(pcsCharacter, lPart);
		if(pcsGridItem)
		{
			pcsItem = GetItem(pcsGridItem);
			if(pcsItem && pcsItem->m_pcsItemTemplate)
			{
				// 내구도가 0인것은 equip 상태라도 무시한다.
				INT32	lDurability		= 0;
				INT32	lTemplateMaxDurability	= 0;
				m_pagpmFactors->GetValue(&pcsItem->m_csFactor, &lDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_DURABILITY);
				m_pagpmFactors->GetValue(&((AgpdItemTemplate *) pcsItem->m_pcsItemTemplate)->m_csFactor, &lTemplateMaxDurability, AGPD_FACTORS_TYPE_ITEM, AGPD_FACTORS_ITEM_TYPE_MAX_DURABILITY);

				// Max 레벨 제한 체크도 들어간다. 2008.01.28. steeple
				INT32 lCharLevel	= m_pagpmCharacter->GetLevel(pcsCharacter);
				INT32 lLimitedLevel	= pcsItem->m_pcsItemTemplate->m_lLimitedLevel;
				INT32 lMinLevel		= m_pagpmFactors->GetLevel(&pcsItem->m_csRestrictFactor);

				if ((lTemplateMaxDurability == 0 || lDurability >= 1) && 
					m_pagpmCharacter->IsPC(pcsCharacter) && 
					(lLimitedLevel == 0 || lLimitedLevel >= lCharLevel) &&	// 최대 레벨 초과
					(lMinLevel <= lCharLevel) &&							// 최소 레벨 미만.
					CheckUseItem(pcsCharacter, pcsItem))
				{
					lLevel += (INT32)std::count(pcsItem->m_aunSkillPlusTID.begin(), pcsItem->m_aunSkillPlusTID.end(), lSkillTID);
				}
			}
		}
	}

	return lLevel;
}

/*
BOOL AgpmItem::CheckAddItem(AgpdItem *pcsItem, AgpdItemStatus eTargetStatus, INT32 lLayer, INT32 lRow, INT32 lColumn, BOOL bIsStackMerge)
{
	if (!pcsItem)
		return FALSE;

	switch (eTargetStatus) {
		case AGPDITEM_STATUS_INVENTORY:
			return CheckAddItemToInventory(pcsItem, AGPDITEM_STATUS_INVENTORY, lLayer, lRow, lColumn, bIsStackMerge);
			break;

		case AGPDITEM_STATUS_EQUIP:
			return CheckAddItemToEquip(pcsItem, AGPDITEM_STATUS_EQUIP, lLayer, lRow, lColumn, bIsStackMerge);
			break;

		case AGPDITEM_STATUS_TRADE_GRID:
			return CheckAddItemToTrade(pcsItem, AGPDITEM_STATUS_TRADE_GRID, lLayer, lRow, lColumn, bIsStackMerge);
			break;

		case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
			return CheckAddItemToClientTrade(pcsItem, AGPDITEM_STATUS_CLIENT_TRADE_GRID, lLayer, lRow, lColumn, bIsStackMerge);
			break;

		case AGPDITEM_STATUS_SALESBOX_GRID:
			return CheckAddItemToSalesBox(pcsItem, AGPDITEM_STATUS_SALESBOX_GRID, lLayer, lRow, lColumn, bIsStackMerge);
			break;

		case AGPDITEM_STATUS_SALESBOX_BACKOUT:
			return CheckAddItemToSalesBoxBackout(pcsItem, AGPDITEM_STATUS_SALESBOX_BACKOUT, lLayer, lRow, lColumn, bIsStackMerge);
			break;

		case AGPDITEM_STATUS_BANK:
			return CheckAddItemToBank(pcsItem, AGPDITEM_STATUS_BANK, lLayer, lRow, lColumn, bIsStackMerge);
			break;

		case AGPDITEM_STATUS_QUEST:
			return CheckAddItemToQuest(pcsItem, AGPDITEM_STATUS_QUEST, lLayer, lRow, lColumn, bIsStackMerge);
			break;

		case AGPDITEM_STATUS_RECEIPE:
			break;
	}

	return TRUE;
}
*/

BOOL AgpmItem::RemoveStatus(INT32 lIID, AgpdItemStatus eNewStatus)
{
	return RemoveStatus(GetItem(lIID), eNewStatus);
}

BOOL AgpmItem::RemoveStatus(AgpdItem *pcsAgpdItem, AgpdItemStatus eNewStatus)
{
	if (!pcsAgpdItem)
		return FALSE;

	switch(pcsAgpdItem->m_eStatus)
	{
	case AGPDITEM_STATUS_FIELD:
		{
			if(!RemoveItemFromField(pcsAgpdItem, TRUE))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (1) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_INVENTORY:
		{
			if(!RemoveItemFromInventory(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (2) !!!\n");
				return FALSE;
			}
			else
			{
				EnumCallback( ITEM_CB_ID_CHAR_REMOVE_INVENTORY, (PVOID)pcsAgpdItem, (PVOID) eNewStatus );
			}
		}
		break;

	case AGPDITEM_STATUS_TRADE_GRID:
		{
			if(!RemoveItemFromTradeGrid(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (2) !!!\n");
				return FALSE;
			}
			else
			{
				EnumCallback( ITEM_CB_ID_UPDATE_TRADEGRID, (PVOID)pcsAgpdItem, (PVOID) eNewStatus );
			}
		}
		break;

	case AGPDITEM_STATUS_CLIENT_TRADE_GRID:
		{
			if(!RemoveItemFromClientTradeGrid(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (2) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_EQUIP:
		{
			if(!UnEquipItem(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem, (AgpdItemStatus)eNewStatus))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (3) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_BANK:
		{
			// 현재 Bank에 Item을 뺄 수 있는지 확인
			if (!EnumCallback(ITEM_CB_ID_CHAR_CHECK_BANK, pcsAgpdItem, pcsAgpdItem->m_pcsCharacter))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (11) !!!\n");
				return FALSE;
			}

			if (!RemoveItemFromBank(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (12) !!!\n");
				return FALSE;
			}

			EnumCallback(ITEM_CB_ID_CHAR_REMOVE_BANK, pcsAgpdItem, (PVOID) eNewStatus);
		}
		break;

	case AGPDITEM_STATUS_SALESBOX_GRID:
		{
			if(!RemoveItemFromSalesBox(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (13) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_QUEST:
		{
			if (!RemoveItemFromQuest(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (14) !!!\n");
				return FALSE;
			}
		}
		break;

		//	2005.11.16. By SungHoon
	case AGPDITEM_STATUS_CASH_INVENTORY :
		{
			//if ( NULL == pcsAgpdItem ) return FALSE;
			if (!RemoveItemFromCashInventory(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (15) !!!\n");
				return FALSE;
			}
		}
		break;

	case AGPDITEM_STATUS_GUILD_WAREHOUSE:
		{
			EnumCallback(ITEM_CB_ID_REMOVE_GUILD_WAREHOUSE_GRID, pcsAgpdItem, NULL);
		}
		break;

	case AGPDITEM_STATUS_SUB_INVENTORY:
		{
			if(!RemoveItemFromSubInventory(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (2) !!!\n");
				return FALSE;
			}
			else
			{
				EnumCallback( ITEM_CB_ID_SUB_INVENTORY_REMOVE, (PVOID)pcsAgpdItem, (PVOID) eNewStatus );
			}
		}
		break;

	case AGPDITEM_STATUS_CHARGING_INVENTORY:
		{
			if(!RemoveItemFromChargingInventory(pcsAgpdItem->m_pcsCharacter, pcsAgpdItem))
			{
				OutputDebugString("AgpmItem::ReleaseItem() Error (2) !!!\n");
				return FALSE;
			}
			else
			{
				EnumCallback( ITEM_CB_ID_CHARGING_INVENTORY_REMOVE, (PVOID)pcsAgpdItem, (PVOID)eNewStatus );
			}
		}
		break;

	default:
		{
		}
		break;
	}
	return TRUE;
}

BOOL AgpmItem::CheckUpdateStatus(AgpdItem *pcsItem, AgpdItemStatus eStatus, INT32 lLayer, INT32 lRow, INT32 lColumn)
{
	if (!pcsItem)
		return FALSE;

	if (pcsItem->m_pcsCharacter &&
		(pcsItem->m_pcsCharacter->m_bIsTrasform || pcsItem->m_pcsCharacter->m_bIsEvolution) &&
		pcsItem->m_eStatus == AGPDITEM_STATUS_EQUIP && 
		((AgpdItemTemplateEquip*)pcsItem->m_pcsItemTemplate)->m_nKind != AGPMITEM_EQUIP_KIND_RIDE &&
		!CheckUseItem(pcsItem->m_pcsCharacter, pcsItem))
		return FALSE;

	// ChargingInventory 경우에는 사용가능한 Layer, Row, Column인지 확인해야한다.
	AgpdCharacter *pcsCharacter = pcsItem->m_pcsCharacter;
	if (pcsCharacter)
	{
		AgpdItemADChar *pcsItemADChar = GetADCharacter(pcsCharacter);
		if (pcsItemADChar)
		{
			if (pcsCharacter->m_unCurrentStatus == AGPDCHAR_STATUS_IN_GAME_WORLD)
			{
				if (eStatus == AGPDITEM_STATUS_CHARGING_INVENTORY)
				{
					// 아이템을 Update하려는데 ChargingInventory를 사용하지 않으면 실패한다.
					if (!pcsItemADChar->m_bUseChagingInventory)
						return FALSE;

					if (lLayer > GetChargingInvectoryLayer(pcsItem->m_pcsCharacter) - 1 || 
						lRow > GetChargingInvectoryRow(pcsItem->m_pcsCharacter) - 1		||
						lColumn > GetChargingInvectoryColumn(pcsItem->m_pcsCharacter) - 1)
						return FALSE;
				}
			}
		}
	}
	
	// 현재 아이템의 상태가 Disarmament 상태이면 Status변경이 불가다.
	if (pcsItem->m_lStatusFlag & AGPMITEM_STATUS_DISARMAMENT)
		return FALSE;

	return TRUE;
}

BOOL AgpmItem::SetCompareTemplate()
{
	m_bIsCompareTemplate	= TRUE;

	return TRUE;
}

BOOL AgpmItem::IsFirstLooterOnly(AgpdItemTemplate *pcsTemplate)
{
	if (AGPMITEM_TYPE_OTHER == pcsTemplate->m_nType &&
		AGPMITEM_OTHER_TYPE_FIRST_LOOTER_ONLY == ((AgpdItemTemplateOther *) pcsTemplate)->m_eOtherItemType
		)
		return TRUE;
	
	return FALSE;
}

/******************************************************************************
******************************************************************************/
/*
	2005.11.30. By SungHoon
	캐쉬 아이템 사용중지를 요청할 경우 불리워 진다.
*/
BOOL AgpmItem::OnOperationUnuseItem(INT8 cStatus, INT32 lIID, INT32 lTID, AgpdCharacter *pcsCharacter)
{
	if(!lIID)
	{
		OutputDebugString("AgpmItem::OnReceive() Error (30) !!!\n");
		return FALSE;
	}

	AgpdItem *pcsItem = GetItem(lIID);
	if (!pcsItem)
		return FALSE;

	return (UnuseItem(pcsCharacter, pcsItem));
}

BOOL AgpmItem::OnOperationPauseItem(INT8 cStatus, INT32 lIID, INT32 lTID, AgpdCharacter* pcsCharacter)
{
	if(!lIID)
		return FALSE;

	AgpdItem *pcsItem = GetItem(lIID);
	if(!pcsItem)
		return FALSE;

	return PauseCashItemTimer(pcsItem);
}

BOOL AgpmItem::OnOperationUpdateItemUseTime(AgpdCharacter *pcsCharacter, INT32 lIID, PVOID pCashInformation)
{
	if(!lIID || !pcsCharacter)
	{
		return FALSE;
	}

	AgpdItem *pcsItem = GetItem(lIID);
	if (!pcsItem)
	{
		return FALSE;
	}

	if (pCashInformation)
	{
		m_csPacketCashInformaion.GetField(FALSE, pCashInformation, 0,
			&pcsItem->m_nInUseItem, 
			&pcsItem->m_lRemainTime,
			&pcsItem->m_lExpireTime,
			&pcsItem->m_lCashItemUseCount,
			&pcsItem->m_llStaminaRemainTime);
	}

	ProcessRefreshUI(0, AGPDITEM_STATUS_CASH_INVENTORY, AGPDITEM_STATUS_CASH_INVENTORY);

	return TRUE;
}

BOOL AgpmItem::OnOperationUseItemResult(AgpdCharacter *pcsCharacter, AgpmItemUseResult eResult)
{
	return EnumCallback(ITEM_CB_ID_USE_ITEM_RESULT, pcsCharacter, &eResult);
}

BOOL AgpmItem::OnOperationUpdateCooldown(AgpdCharacter* pcsCharacter, INT32 lTID, UINT32 ulRemainTime, BOOL bPause)
{
	if(!pcsCharacter || lTID < 1)
		return FALSE;

	BOOL bPrevPause = GetCooldownPause(pcsCharacter, lTID);

	SetCooldownByTID(pcsCharacter, lTID, ulRemainTime, bPause);

	AgpdItemCooldownBase stCooldownBase;
	stCooldownBase.m_lTID = lTID;
	stCooldownBase.m_ulRemainTime = ulRemainTime;
	stCooldownBase.m_bPause = bPause;
	stCooldownBase.m_bPrevPause = bPrevPause;

	CallCooldownCallback(pcsCharacter, stCooldownBase);

	return TRUE;
}

BOOL AgpmItem::OnOperationUpdateStaminaRemainTime(AgpdCharacter *pcsCharacter, INT32 lIID, PVOID pCashInformation)
{
	if(!lIID || !pcsCharacter)
	{
		return FALSE;
	}

	AgpdItem *pcsItem = GetItem(lIID);
	if (!pcsItem)
	{
		return FALSE;
	}

	if (pCashInformation)
	{
		m_csPacketCashInformaion.GetField(FALSE, pCashInformation, 0,
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			&pcsItem->m_llStaminaRemainTime);
	}

	ProcessRefreshUI(0, AGPDITEM_STATUS_CASH_INVENTORY, AGPDITEM_STATUS_CASH_INVENTORY);

	return TRUE;
}

BOOL AgpmItem::OnOperationChangeAutoPickUpItem(AgpdCharacter *pcsCharacter, INT32 llID, PVOID pAutoPickUpItem)
{
	if(!pcsCharacter || llID == AP_INVALID_IID || !pAutoPickUpItem)
		return FALSE;

	AgpdItem *pcsItem = GetItem(llID);
	if (!pcsItem)
	{
		return FALSE;
	}

	INT16 lSwitch = -1;
	
	m_csPacketAutoPickItem.GetField(FALSE, pAutoPickUpItem, 0, &lSwitch);

	EnumCallback(ITEM_CB_ID_CHANGE_AUTOPICK_ITEM, pcsItem, &lSwitch);

	return TRUE;
}

/*
	2005.11.30. By SungHoon
	캐쉬 아이템사용중지한다.
*/
BOOL AgpmItem::UnuseItem(AgpdCharacter *pcsCharacter, INT32 lIID)
{
	AgpdItem *pcsItem = GetItem(lIID);
	return ( UnuseItem(pcsCharacter, pcsItem) );
}

/*
	2005.11.30. By SungHoon
	캐쉬 아이템사용중지한다.
*/
BOOL AgpmItem::UnuseItem(AgpdCharacter *pcsCharacter, AgpdItem *pcsItem)
{
	if (!pcsItem) return FALSE;
	// 2005.09.29. steeple
	// 소유주 체크
    if(pcsCharacter && pcsItem->m_pcsCharacter)
	{
		if(pcsCharacter->m_lID != pcsItem->m_pcsCharacter->m_lID)
			return FALSE;
	}

	EnumCallback(ITEM_CB_ID_UNUSE_ITEM, pcsItem, NULL );

	return TRUE;

}

/*
	2005.11.30. By SungHoon
	캐쉬 아이템의 사용시간을 시작한다.(시간체크)
*/
BOOL AgpmItem::StartCashItemTimer(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsCharacter)
		return FALSE;

	if (!IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType)) return FALSE;

	if (pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_INUSE) return FALSE;
	
	if (pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_UNUSE && pcsItem->m_lCashItemUseCount < AGPDITEM_CASH_ITEM_MAX_USABLE_COUNT)
		pcsItem->m_lCashItemUseCount++;

	if ( pcsItem->m_pcsItemTemplate->m_eCashItemUseType == AGPMITEM_CASH_ITEM_USE_TYPE_CONTINUOUS)
		pcsItem->m_nInUseItem = AGPDITEM_CASH_ITEM_INUSE;

	// check a stamina remain time 2008.06.13. steeple
	SetInitialStamina(pcsItem);

	EnumCallback(ITEM_CB_ID_CHAR_USE_CASH_ITEM, pcsItem, NULL);

	return TRUE;
}

/*
	2005.11.30. By SungHoon
	캐쉬 아이템의 사용시간을 종료한다.(시간체크)
*/
BOOL AgpmItem::StopCashItemTimer(AgpdItem *pcsItem)
{
	if (!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	if (!IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType)) return FALSE;

	if (pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_UNUSE) return FALSE;
	pcsItem->m_nInUseItem = AGPDITEM_CASH_ITEM_UNUSE;

	EnumCallback(ITEM_CB_ID_CHAR_UNUSE_CASH_ITEM, pcsItem, NULL);

	return TRUE;
}

// 2006.01.08. steeple
// 캐쉬아이템 일시 정지
BOOL AgpmItem::PauseCashItemTimer(AgpdItem* pcsItem)
{
	if(!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	if(!IS_CASH_ITEM(pcsItem->m_pcsItemTemplate->m_eCashItemType))
		return FALSE;

	if(pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_UNUSE)
		return FALSE;

	pcsItem->m_nInUseItem = AGPDITEM_CASH_ITEM_PAUSE;

	EnumCallback(ITEM_CB_ID_CHAR_PAUSE_CASH_ITEM, pcsItem, NULL);

	return TRUE;
}

// 2008.06.24. steeple
// Set an initial stamina value
BOOL AgpmItem::SetInitialStamina(AgpdItem* pcsItem)
{
	if(!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	// check a stamina remain time 2008.06.13. steeple
	if(pcsItem->m_pcsItemTemplate->m_llStaminaRemainTime != 0 &&
		(pcsItem->m_llStaminaRemainTime == 0 || pcsItem->m_llStaminaRemainTime == pcsItem->m_pcsItemTemplate->m_llStaminaRemainTime)
		)
	{
		INT32 lPetTID = 0;
		EnumCallback(ITEM_CB_ID_GET_PET_TID_BY_ITEM_FROM_SKILL, pcsItem->m_pcsItemTemplate, &lPetTID);
		if(lPetTID)
		{
			AgpdCharacterTemplate* pcsCharacterTemplate = m_pagpmCharacter->GetCharacterTemplate(lPetTID);
			if(pcsCharacterTemplate)
				pcsItem->m_llStaminaRemainTime = (INT64)pcsCharacterTemplate->m_lStartStaminaPoint * (INT64)(1000);
		}

		if(pcsItem->m_llStaminaRemainTime == 0 || pcsItem->m_llStaminaRemainTime == pcsItem->m_pcsItemTemplate->m_llStaminaRemainTime)
			pcsItem->m_llStaminaRemainTime = pcsItem->m_pcsItemTemplate->m_llStaminaRemainTime - 1;
	}

	return TRUE;
}

// 2008.06.25. steeple
// return whether the item uses a stamina value
BOOL AgpmItem::IsUsingStamina(AgpdItemTemplate* pcsItemTemplate)
{
	if(!pcsItemTemplate)
		return FALSE;

	if(pcsItemTemplate->m_eCashItemType == AGPMITEM_CASH_ITEM_TYPE_STAMINA && pcsItemTemplate->m_llStaminaRemainTime != 0)
		return TRUE;

	return FALSE;
}

// 2007.02.27. steeple
// 강화축성제가 두개가 되어서 TID 하나만으로 하면 안된다. 인벤 돌면서 타입비교 후 가져오게끔 변경.
// 우선순위는 귀속 > 일반
AgpdItem* AgpmItem::GetExistCatalyst(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return NULL;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return NULL;

	AgpdItem* pcsCatalyst = NULL;

	for (int i = 0; i < pcsItemADChar->m_csInventoryGrid.m_nLayer; ++i)
	{
		for (int j = 0; j < pcsItemADChar->m_csInventoryGrid.m_nColumn; ++j)
		{
			for (int k = 0; k < pcsItemADChar->m_csInventoryGrid.m_nRow; ++k)
			{
				AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csInventoryGrid, i, k, j);
				if (pcsGridItem)
				{
					AgpdItem	*pcsItem	= GetItem(pcsGridItem);
					if (pcsItem && pcsItem->m_pcsItemTemplate &&
						pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
						((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_CONVERT_CATALYST)
					{
						// 2007.02.27. steeple
						// 귀속이라면 바로 리턴
						if(IsBoundOnOwner(pcsItem))
							return pcsItem;
						else
							pcsCatalyst = pcsItem;
					}
				}
			}
		}
	}

	if(IsEnableSubInventory(pcsCharacter))
	{		
		for (int i = 0; i < pcsItemADChar->m_csSubInventoryGrid.m_nLayer; ++i)
		{
			for (int j = 0; j < pcsItemADChar->m_csSubInventoryGrid.m_nColumn; ++j)
			{
				for (int k = 0; k < pcsItemADChar->m_csSubInventoryGrid.m_nRow; ++k)
				{
					AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csSubInventoryGrid, i, k, j);
					if (pcsGridItem)
					{
						AgpdItem	*pcsItem	= GetItem(pcsGridItem);
						if (pcsItem && pcsItem->m_pcsItemTemplate &&
							pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
							((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_CONVERT_CATALYST)
						{
							// 2007.02.27. steeple
							// 귀속이라면 바로 리턴
							if(IsBoundOnOwner(pcsItem))
								return pcsItem;
							else
								pcsCatalyst = pcsItem;
						}
					}
				}
			}
		}
	}

	if (IsEnableChargingInventory(pcsCharacter))
	{
		INT32 lLayer	= GetChargingInvectoryLayer(pcsCharacter);
		INT32 lColumn	= GetChargingInvectoryColumn(pcsCharacter);
		INT32 lRow		= GetChargingInvectoryRow(pcsCharacter);

		for (int i = 0; i < lLayer; ++i)
		{
			for (int j = 0; j < lColumn; ++j)
			{
				for (int k = 0; k < lRow; ++k)
				{
					AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csChargingInventoryGrid, i, k, j);
					if (pcsGridItem)
					{
						AgpdItem	*pcsItem	= GetItem(pcsGridItem);
						if (pcsItem && pcsItem->m_pcsItemTemplate &&
							pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
							((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_CONVERT_CATALYST)
						{
							// 2007.02.27. steeple
							// 귀속이라면 바로 리턴
							if(IsBoundOnOwner(pcsItem))
								return pcsItem;
							else
								pcsCatalyst = pcsItem;
						}
					}
				}
			}
		}
	}


	return pcsCatalyst;
}

AgpdItem* AgpmItem::GetExistReverseOrb(AgpdCharacter *pcsCharacter, BOOL bIncludeCash)
{
	if (!pcsCharacter)
		return NULL;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return NULL;

	// 2006.08.17. steeple
	// ReverseOrb 쿨타임 버그때문에.
	if(bIncludeCash)
	{
		for (int i = 0; i < pcsItemADChar->m_csCashInventoryGrid.m_nRow; i++)
		{
			AgpdGridItem *pcsGridItem = m_pagpmGrid->GetItem(&pcsItemADChar->m_csCashInventoryGrid, 0, i, 0 );
			if (pcsGridItem)
			{
				AgpdItem	*pcsItem	= GetItem(pcsGridItem);
				if (pcsItem && pcsItem->m_pcsItemTemplate &&
					pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
					((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_REVERSE_ORB)
				{
					// Level 체크 들어간다. 2008.01.29. steeple
					INT32 lRestrictLevel = 0;
					m_pagpmFactors->GetValue(&pcsItem->m_csRestrictFactor, &lRestrictLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);
					if(lRestrictLevel > 0 || pcsItem->m_pcsItemTemplate->m_lLimitedLevel != 0)
					{
						INT32 lCharLevel = m_pagpmCharacter->GetLevel(pcsCharacter);
						if(lCharLevel < lRestrictLevel ||
							(pcsItem->m_pcsItemTemplate->m_lLimitedLevel != 0 && lCharLevel > pcsItem->m_pcsItemTemplate->m_lLimitedLevel))
							continue;
					}

					return pcsItem;
				}
			}
		}
	}

	AgpdItem* pcsReverseOrb = NULL;

	for (int i = 0; i < pcsItemADChar->m_csInventoryGrid.m_nLayer; ++i)
	{
		for (int k = 0; k < pcsItemADChar->m_csInventoryGrid.m_nRow; ++k)
		{
			for (int j = 0; j < pcsItemADChar->m_csInventoryGrid.m_nColumn; ++j)
			{
				AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csInventoryGrid, i, k, j);
				if (pcsGridItem)
				{
					AgpdItem	*pcsItem	= GetItem(pcsGridItem);
					if (pcsItem && pcsItem->m_pcsItemTemplate &&
						pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
						((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_REVERSE_ORB)
					{
						// Level 체크 들어간다. 2008.01.29. steeple
						INT32 lRestrictLevel = 0;
						m_pagpmFactors->GetValue(&pcsItem->m_csRestrictFactor, &lRestrictLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);
						if(lRestrictLevel > 0 || pcsItem->m_pcsItemTemplate->m_lLimitedLevel != 0)
						{
							INT32 lCharLevel = m_pagpmCharacter->GetLevel(pcsCharacter);
							if(lCharLevel < lRestrictLevel ||
								(pcsItem->m_pcsItemTemplate->m_lLimitedLevel != 0 && lCharLevel > pcsItem->m_pcsItemTemplate->m_lLimitedLevel))
								continue;
						}

						// 2007.02.27. steeple
						// 귀속이라면 바로 리턴
						if(IsBoundOnOwner(pcsItem))
							return pcsItem;
						else
							pcsReverseOrb = pcsItem;
					}
				}
			}
		}
	}

	if(GetUsingCashPetItemInvolveSubInventory(pcsCharacter))
	{

		for (int i = 0; i < pcsItemADChar->m_csSubInventoryGrid.m_nLayer; ++i)
		{
			for (int k = 0; k < pcsItemADChar->m_csSubInventoryGrid.m_nRow; ++k)
			{
				for (int j = 0; j < pcsItemADChar->m_csSubInventoryGrid.m_nColumn; ++j)
				{
					AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csSubInventoryGrid, i, k, j);
					if (pcsGridItem)
					{
						AgpdItem	*pcsItem	= GetItem(pcsGridItem);
						if (pcsItem && pcsItem->m_pcsItemTemplate &&
							pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
							((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_REVERSE_ORB)
						{
							// Level 체크 들어간다. 2008.01.29. steeple
							INT32 lRestrictLevel = 0;
							m_pagpmFactors->GetValue(&pcsItem->m_csRestrictFactor, &lRestrictLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);
							if(lRestrictLevel > 0 || pcsItem->m_pcsItemTemplate->m_lLimitedLevel != 0)
							{
								INT32 lCharLevel = m_pagpmCharacter->GetLevel(pcsCharacter);
								if(lCharLevel < lRestrictLevel ||
									(pcsItem->m_pcsItemTemplate->m_lLimitedLevel != 0 && lCharLevel > pcsItem->m_pcsItemTemplate->m_lLimitedLevel))
									continue;
							}

							// 2007.02.27. steeple
							// 귀속이라면 바로 리턴
							if(IsBoundOnOwner(pcsItem))
								return pcsItem;
							else
								pcsReverseOrb = pcsItem;
						}
					}
				}
			}
		}
	}

	if(IsEnableChargingInventory(pcsCharacter))
	{
		INT32 lLayer	= GetChargingInvectoryLayer(pcsCharacter);
		INT32 lColumn	= GetChargingInvectoryColumn(pcsCharacter);
		INT32 lRow		= GetChargingInvectoryRow(pcsCharacter);

		for (int i = 0; i < lLayer; ++i)
		{
			for (int k = 0; k < lRow; ++k)
			{
				for (int j = 0; j < lColumn; ++j)
				{
					AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csChargingInventoryGrid, i, k, j);
					if (pcsGridItem)
					{
						AgpdItem	*pcsItem	= GetItem(pcsGridItem);
						if (pcsItem && pcsItem->m_pcsItemTemplate &&
							pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
							((AgpdItemTemplateUsable *) pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_REVERSE_ORB)
						{
							// Level 체크 들어간다. 2008.01.29. steeple
							INT32 lRestrictLevel = 0;
							m_pagpmFactors->GetValue(&pcsItem->m_csRestrictFactor, &lRestrictLevel, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL);
							if(lRestrictLevel > 0 || pcsItem->m_pcsItemTemplate->m_lLimitedLevel != 0)
							{
								INT32 lCharLevel = m_pagpmCharacter->GetLevel(pcsCharacter);
								if(lCharLevel < lRestrictLevel ||
									(pcsItem->m_pcsItemTemplate->m_lLimitedLevel != 0 && lCharLevel > pcsItem->m_pcsItemTemplate->m_lLimitedLevel))
									continue;
							}

							// 2007.02.27. steeple
							// 귀속이라면 바로 리턴
							if(IsBoundOnOwner(pcsItem))
								return pcsItem;
							else
								pcsReverseOrb = pcsItem;
						}
					}
				}
			}
		}
	}


	return pcsReverseOrb;
}

// 2009.03.05. iluvs
AgpdItem* AgpmItem::GetExistPrivateTradeOptionItem(AgpdCharacter *pcsCharacter, INT32 lIID)
{
	if(pcsCharacter == NULL)
		return NULL;

	AgpdItemADChar *pcsItemADChar = GetADCharacter(pcsCharacter);
	if(pcsItemADChar == NULL)
		return NULL;

	AgpdGridItem	*pcsGridItem;

	BOOL	bResult = TRUE;
	INT32	i = 0;
	for (pcsGridItem = m_pagpmGrid->GetItemSequence(&pcsItemADChar->m_csCashInventoryGrid, &i);
		pcsGridItem;
		pcsGridItem = m_pagpmGrid->GetItemSequence(&pcsItemADChar->m_csCashInventoryGrid, &i))
	{
		AgpdItem	*pcsItem = GetItem(pcsGridItem);
		if(pcsItem && pcsItem->m_pcsItemTemplate &&
			pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE &&
			((AgpdItemTemplateUsable*)pcsItem->m_pcsItemTemplate)->m_nUsableItemType == AGPMITEM_USABLE_TYPE_PRIVATE_TRADE_OPTION)
		{
			// 이 안에 들어온 조건이면 마블 주문서이다.
			if(pcsItem->m_lID == lIID)
				return pcsItem;
		}
	}

	return NULL;
}

// 2006.12.15. steeple
AgpdItemOptionTemplate* AgpmItem::GetFitLinkTemplate(INT32 lLinkID, INT32 lLevel)
{
	if(!lLinkID)
		return NULL;

	// 여기까지 왔으면 옵션 템플릿 돌면서 찾아본다.
	for(AgpaItemOptionTemplate::iterator it = csOptionTemplateAdmin.begin(); it != csOptionTemplateAdmin.end(); ++it)
	{
		AgpdItemOptionTemplate* pcsOptionTemplate = it->second;

		if(pcsOptionTemplate->m_lLinkID == lLinkID &&
			pcsOptionTemplate->m_lLevelMin <= lLevel &&
			pcsOptionTemplate->m_lLevelMax >= lLevel)
			return pcsOptionTemplate;
	}

	return NULL;
}

void AgpmItem::Report(FILE *fp)
{
	if (!fp)
		return;

	fprintf(fp, "Template Admin : %d/%d\n", csTemplateAdmin.size(), csTemplateAdmin.size());
	fprintf(fp, "Item Admin     : %d/%d\n", csItemAdmin.GetObjectCount(), csItemAdmin.GetCount());
}

AgpdAvatarSetItem		*AgpmItem::GetAvatarSetTemplate( INT32 nTID )
{
	for( vector< AgpdAvatarSetItem >::iterator iter = m_vecAvatarSetItem.begin();
		iter != m_vecAvatarSetItem.end();
		iter ++ )
	{
		AgpdAvatarSetItem * pSet = &*iter;

		if( pSet->nTID == nTID )
		{
			return pSet;
		}
	}

	return NULL;
}

AgpdAvatarSetItem* AgpmItem::GetAvatarSetTemplateByBase(INT32 lBaseTID)
{
	if(lBaseTID < 1)
		return NULL;

	IterAvatarSetItem iter = m_vecAvatarSetItem.begin();
	while(iter != m_vecAvatarSetItem.end())
	{
		if(lBaseTID == iter->lBaseTID)
			return &*iter;

		++iter;
	}
	
	return NULL;
}


BOOL					AgpmItem::Debug_EquipAvatarSet( INT32 lCID, INT32 nTID , BOOL bCheckUseItem )
{
	AgpdCharacter * pCharacter = m_pagpmCharacter->GetCharacter( lCID );	
	return Debug_EquipAvatarSet( pCharacter , nTID , bCheckUseItem );
}

BOOL					AgpmItem::Debug_EquipAvatarSet(AgpdCharacter *pcsCharacter, INT32 nTID, BOOL bCheckUseItem )
{
	AgpdAvatarSetItem * pSetItem = GetAvatarSetTemplate( nTID );
	if( !pSetItem )
	{
		// 템플릿이 없네요?-_-;
		return FALSE;
	}

	// 각 템플릿이 제대로 되어 있는지 검사
	// 각 아이템이 들어갈 파트가 비어있는지 검사
	// 각 아이템을 착용할수 있는지 속성검사 ( 성별 , 종족 , 레벨 )

	static	INT32 _snDebugOffset = 0x0FFF0000;

	for( vector< INT32 >::iterator iter = pSetItem->vecItemTIDs.begin() ;
		iter != pSetItem->vecItemTIDs.end();
		iter++ )
	{
		INT32	nItemTID = *iter;

		AgpdItem * pstAgpdItem = AddItem( _snDebugOffset++ , nItemTID , 1);

		if( !pstAgpdItem )
		{
			return FALSE;
		}

		if(!InitItem(pstAgpdItem))
		{
			return FALSE;
		}

		// 포인터 강제설정.
		pstAgpdItem->m_pcsCharacter	= pcsCharacter;

		if( EquipItem( pcsCharacter , pstAgpdItem , FALSE) )
		{
			// do nothing..
		}
		else
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

BOOL					AgpmItem::Debug_UnEquipAvatarSet(INT32 lCID, INT32 nTID , BOOL bEquipDefaultItem )
{
	AgpdCharacter * pCharacter = m_pagpmCharacter->GetCharacter( lCID );	
	return Debug_UnEquipAvatarSet( pCharacter , nTID , bEquipDefaultItem );
}

BOOL					AgpmItem::Debug_UnEquipAvatarSet(AgpdCharacter *pcsCharacter, INT32 nTID , BOOL bEquipDefaultItem )
{
	AgpdAvatarSetItem * pSetItem = GetAvatarSetTemplate( nTID );
	if( !pSetItem )
	{
		// 템플릿이 없네요?-_-;
		return FALSE;
	}

	for( vector< INT32 >::iterator iter = pSetItem->vecItemTIDs.begin() ;
		iter != pSetItem->vecItemTIDs.end();
		iter++ )
	{
		INT32	nItemTID = *iter;

		AgpdItemTemplate		* pTemplate			= GetItemTemplate( nItemTID );
		AgpdItemTemplateEquip	* pTemplateEquip	= ( AgpdItemTemplateEquip * ) pTemplate;

		if( pTemplate )
		{
			AgpmItemPart	ePart = pTemplateEquip->GetAvatarPartIndex();

			AgpdGridItem	* pcsItemGrid	= GetEquipItem( pcsCharacter , ePart );
			AgpdItem		* pcsItem		= pcsItemGrid ? GetItem( pcsItemGrid ) : NULL;

			if( pcsItem )
			{
				UnEquipItem( pcsCharacter , pcsItem );
				DestroyItem( pcsItem );
			}			
		}
		else
		{
			// 이거 뭔가 이상한데
			// 에러를 엇다가 내야하는겨.
		}
	}

	return TRUE;
}

INT32					AgpmItem::GetEquipAvatarSet( INT32 lCID )
{
	AgpdCharacter * pCharacter = m_pagpmCharacter->GetCharacter( lCID );	
	return GetEquipAvatarSet( pCharacter );
}

INT32					AgpmItem::GetEquipAvatarSet( AgpdCharacter *pcsAgpdCharacter )
{
	// 메이저 파트들 돌아다니면서
	// 처음나온 아바타 아이템의 템플릿 아이디를 조사해서 리턴해줌.

	for( INT32 ePart = ( AgpmItemPart ) AGPMITEM_PART_V_BODY ; ePart < ( AgpmItemPart )AGPMITEM_PART_V_ALL ; ePart ++ )
	{
		AgpdGridItem	* pcsItemGrid	= GetEquipItem( pcsAgpdCharacter , ( AgpmItemPart ) ePart );
		AgpdItem		* pcsItem		= pcsItemGrid ? GetItem( pcsItemGrid ) : NULL;
		if( pcsItem )
		{
			for( vector< AgpdAvatarSetItem >::iterator iter = m_vecAvatarSetItem.begin() ;
				iter != m_vecAvatarSetItem.end();
				iter++)
			{
				AgpdAvatarSetItem * pSetItem = &*iter;

				for( vector< INT32 >::iterator iterTID = pSetItem->vecItemTIDs.begin();
					iterTID != pSetItem->vecItemTIDs.end();
					iterTID++ )
				{
					INT32 nTID = * iterTID;
					if( nTID == pcsItem->m_pcsItemTemplate->m_lID )
					{
						return pSetItem->nTID;
					}
				}

			}
		}
	}

	// 없네..
	return -1;
}

// 2007.08.03. steeple
BOOL AgpmItem::IsAvatarItem(AgpdItemTemplate* pcsItemTemplate)
{
	if(!pcsItemTemplate)
		return FALSE;

	if(pcsItemTemplate->m_nType == AGPMITEM_TYPE_EQUIP)
	{
		AgpdItemTemplateEquip* pcsTemplateEquip = static_cast<AgpdItemTemplateEquip*>(pcsItemTemplate);
		return pcsTemplateEquip->IsAvatarEquip();
	}
	else if(pcsItemTemplate->m_nType == AGPMITEM_TYPE_USABLE)
	{
		AgpdItemTemplateUsable* pcsTemplateUsable = static_cast<AgpdItemTemplateUsable*>(pcsItemTemplate);
		if(pcsTemplateUsable->m_nUsableItemType == AGPMITEM_USABLE_TYPE_AVATAR)
			return TRUE;
	}

	return FALSE;
}

// 2007.08.03. steeple
BOOL AgpmItem::IsBaseAvatarItem(AgpdItemTemplate* pcsItemTemplate)
{
	if(!pcsItemTemplate)
		return FALSE;

	IterAvatarSetItem iter = m_vecAvatarSetItem.begin();
	while(iter != m_vecAvatarSetItem.end())
	{
		if(pcsItemTemplate->m_lID == iter->lBaseTID)
			return TRUE;

		++iter;
	}

	return FALSE;
}

// 2007.08.06. steeple
BOOL AgpmItem::IsEnableEquipAvatarItem(AgpdItem* pcsItem)
{
	if(!pcsItem || !pcsItem->m_pcsItemTemplate || !pcsItem->m_pcsCharacter)
		return FALSE;

	if(IsBaseAvatarItem(pcsItem->m_pcsItemTemplate))
	{
		AgpdAvatarSetItem* pSet = GetAvatarSetTemplateByBase(pcsItem->m_pcsItemTemplate->m_lID);
		if(!pSet)
			return FALSE;

		// 하위 아이템들을 모두 돌면서 착용할 수 있는 지 본다.
		vector<INT32>::iterator iter = pSet->vecItemTIDs.begin();
		while(iter != pSet->vecItemTIDs.end())
		{
			AgpdItemTemplate* pcsItemTemplate = GetItemTemplate(*iter);
			if(!pcsItemTemplate || pcsItemTemplate->m_lID == pcsItem->m_pcsItemTemplate->m_lID)		// Base 는 입지 않는다.
				break;

			if(!IsEnableEquipAvatarItem(pcsItem->m_pcsCharacter, pcsItemTemplate))
				return FALSE;

			++iter;
		}

		return TRUE;
	}
	else
	{
		return IsEnableEquipAvatarItem(pcsItem->m_pcsCharacter, pcsItem->m_pcsItemTemplate);
	}

	return FALSE;
}

// 2007.08.06. steeple
BOOL AgpmItem::IsEnableEquipAvatarItem(AgpdCharacter* pcsCharacter, AgpdItemTemplate* pcsItemTemplate)
{
	if(!pcsCharacter || !pcsItemTemplate)
		return FALSE;

	if(pcsItemTemplate->m_nType != AGPMITEM_TYPE_EQUIP)
		return FALSE;

	AgpdItemTemplateEquip* pcsEquipItemTemplate = static_cast<AgpdItemTemplateEquip*>(pcsItemTemplate);

	// 해당 파트에 아이템을 착용하고 있으면 FALSE
	AgpdGridItem* pcsEquipGridItem = GetEquipItem(pcsCharacter, pcsEquipItemTemplate->m_nPart);
	if(pcsEquipGridItem)
	{
		AgpdItem* pcsItem = GetItem(pcsEquipGridItem);
		if(pcsItem)
			return FALSE;
	}

	return TRUE;
}

BOOL					AgpmItem::StreamReadAvatarSet(CHAR *szFile, BOOL bDecryption)
{
	int					nNumKeys;
	int					nNumSections;

	m_vecAvatarSetItem.clear();

	AuIniManagerA		csIniFile;

	csIniFile.SetMode( (AuIniManagerMode) APMODULE_STREAM_MODE_NAME_OVERWRITE );

	csIniFile.SetPath( szFile );

	if( ! csIniFile.ReadFile(0 , bDecryption) ) return FALSE;

	nNumSections = csIniFile.GetNumSection();

	const char strName	[]	= "name"	;
	const char strtid	[]	= "tid"		;

	// 각 Section에 대해서...
	for (INT32 nSection = 0; nSection < nNumSections; nSection ++)
	{
		// Section Name은 TID 이다.
		INT32 lTID		= atoi( csIniFile.GetSectionName( nSection ) );
		nNumKeys	= csIniFile.GetNumKeys( nSection );

		AgpdAvatarSetItem	stSet;
		stSet.nTID = lTID;

		BOOL	bGetName	= FALSE;
		BOOL	bGetTID		= FALSE;

		for( INT32 nKey = 0; nKey < nNumKeys ; nKey++ )
		{
			const char * pKey = csIniFile.GetKeyName( nSection , nKey );

			// 그냥 앞에 3글자만 검사..
			if( !bGetName && !strncmp( pKey , strName , 3 ) )
			{
				bGetName = TRUE;
				strncpy( stSet.strName , csIniFile.GetValue( nSection , nKey , "" ) , AGPMITEM_MAX_ITEM_NAME );
			}
			else if( !strncmp( pKey , strtid , 3 ) )
			{
				INT32	nItemTID = atoi( csIniFile.GetValue( nSection , nKey , "0" ) );

				if( nItemTID != 0 && stSet.vecItemTIDs.size() < 10 ) // 10은 대충 넣은 숫자 -_- 보통 5개 이하일듯 
				{
					bGetTID = TRUE;
					stSet.vecItemTIDs.push_back( nItemTID );
				}

				if(_tcslen(pKey) >= 4 && pKey[3] == '0')
					stSet.lBaseTID = nItemTID;
			}
		}

		if( bGetName && bGetTID )
		{
			m_vecAvatarSetItem.push_back( stSet );
		}
	}

	return TRUE;
}

eAgpmItemSectionType AgpmItem::GetItemSectionNum(AgpdItem* pcsItem)
{
	if(NULL == pcsItem)
		return AGPMITEM_SECTION_TYPE_NONE;

	return pcsItem->m_pcsItemTemplate->m_eItemSectionType;
}

BOOL AgpmItem::IsEnableEquipItemInMyRegion(AgpdCharacter* pcsCharacter, AgpdItem* pcsItem)
{
	if(NULL == pcsCharacter || NULL == pcsItem)
		return FALSE;

	// -1이 들어있으면 Model Tool에서 사용하는 거다
	if( pcsCharacter->m_nBindingRegionIndex == -1 )
		return TRUE;

	ApmMap::RegionTemplate *pRegionTemplate = m_papmMap->GetTemplate(pcsCharacter->m_nBindingRegionIndex);
	if(NULL == pRegionTemplate)
		return FALSE;

	// 아이템의 Section Type이 해당 Region의 사용불가 아이템 Section Type에 해당된다면 Equip이 불가능하다.
	if(pcsItem->m_pcsItemTemplate->m_eItemSectionType & (eAgpmItemSectionType)pRegionTemplate->nUnableItemSectionNum)
		return FALSE;

	return TRUE;
}

// 2007.11.02. steeple
INT32 AgpmItem::GetWeaponType(AgpdItemTemplate* pcsItemTemplate)
{
	if(!pcsItemTemplate)
		return -1;

	if(((AgpdItemTemplateEquip*)pcsItemTemplate)->m_nKind != AGPMITEM_EQUIP_KIND_WEAPON)
		return -1;

	return ((AgpdItemTemplateEquipWeapon*)pcsItemTemplate)->m_nWeaponType;
}

BOOL AgpmItem::IsReverseOrbTypeItem(AgpdItem *pcsItem)
{
	if(NULL == pcsItem || NULL == pcsItem->m_pcsItemTemplate)
		return FALSE;

	if (((AgpdItemTemplateUsable *)(pcsItem->m_pcsItemTemplate))->m_nUsableItemType == AGPMITEM_USABLE_TYPE_REVERSE_ORB)
		return TRUE;

	return FALSE;
}

// 2008.02.01. steeple
// 해당 status grid 에 있는 TID 아이템 모든 개수를 리턴한다.
INT32 AgpmItem::GetItemTotalCountFromGrid(AgpdCharacter* pcsCharacter, INT32 lItemTID, AgpdItemStatus eStatus)
{
	if(!pcsCharacter || lItemTID < 1)
		return 0;
	
	AgpdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return 0;

	AgpdGrid* pcsGrid = NULL;
	switch(eStatus)
	{
		case AGPDITEM_STATUS_INVENTORY:
			pcsGrid = &pcsItemADChar->m_csInventoryGrid;
			break;

		case AGPDITEM_STATUS_EQUIP:
			pcsGrid = &pcsItemADChar->m_csEquipGrid;
			break;

		case AGPDITEM_STATUS_CASH_INVENTORY:
			pcsGrid = &pcsItemADChar->m_csCashInventoryGrid;
			break;

		case AGPDITEM_STATUS_SUB_INVENTORY:
			pcsGrid = &pcsItemADChar->m_csSubInventoryGrid;
			break;

		case AGPDITEM_STATUS_CHARGING_INVENTORY:
			pcsGrid = &pcsItemADChar->m_csChargingInventoryGrid;
			break;

		default:
			break;
	}

	if(!pcsGrid)
		return 0;

	AgpdGridItem* pcsGridItem = NULL;
	INT32 lCount, lIndex;
	lCount = lIndex = 0;

	for(pcsGridItem = m_pagpmGrid->GetItemSequence(pcsGrid, &lIndex);
		pcsGridItem;
		pcsGridItem = m_pagpmGrid->GetItemSequence(pcsGrid, &lIndex))
	{
		AgpdItem* pcsItem = GetItem(pcsGridItem);
		if(!pcsItem || !pcsItem->m_pcsItemTemplate || pcsItem->m_pcsItemTemplate->m_lID != lItemTID)
			continue;

		if(pcsItem->m_pcsItemTemplate->m_bStackable)
			lCount += pcsItem->m_nCount;
		else
			++lCount;
	}

	return lCount;
}

// 2008.02.01. steeple
// 해당 status grid 에 있는 TID 아이템을 lCount 만큼 지운다.
BOOL  AgpmItem::RemoveItemCompleteFromGrid(AgpdCharacter* pcsCharacter, INT32 lItemTID, INT32 lCount, AgpdItemStatus eStatus)
{
	if(!pcsCharacter || lItemTID < 1)
		return FALSE;
	
	AgpdItemADChar* pcsItemADChar = GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return FALSE;

	AgpdGrid* pcsGrid = NULL;
	switch(eStatus)
	{
		case AGPDITEM_STATUS_INVENTORY:
			pcsGrid = &pcsItemADChar->m_csInventoryGrid;
			break;

		case AGPDITEM_STATUS_EQUIP:
			pcsGrid = &pcsItemADChar->m_csEquipGrid;
			break;

		case AGPDITEM_STATUS_CASH_INVENTORY:
			pcsGrid = &pcsItemADChar->m_csCashInventoryGrid;
			break;

		case AGPDITEM_STATUS_SUB_INVENTORY:
			pcsGrid = &pcsItemADChar->m_csSubInventoryGrid;
			break;

		case AGPDITEM_STATUS_CHARGING_INVENTORY:
			pcsGrid = &pcsItemADChar->m_csChargingInventoryGrid;
			break;


		default:
			break;
	}

	if(!pcsGrid)
		return FALSE;

	INT32 lCountInGrid = GetItemTotalCountFromGrid(pcsCharacter, lItemTID, eStatus);
	if(lCountInGrid < lCount)
		return FALSE;

	AgpdGridItem* pcsGridItem = NULL;

	INT32 lIndex = 0;
	INT32 lRemained = lCount;
	INT32 lRemove = 0;

	for(pcsGridItem = m_pagpmGrid->GetItemSequence(pcsGrid, &lIndex);
		pcsGridItem;
		pcsGridItem = m_pagpmGrid->GetItemSequence(pcsGrid, &lIndex))
	{
		AgpdItem* pcsItem = GetItem(pcsGridItem);
		if(!pcsItem || !pcsItem->m_pcsItemTemplate || pcsItem->m_pcsItemTemplate->m_lID != lItemTID)
			continue;

		if (pcsItem->m_pcsItemTemplate->m_bStackable)
			lRemove = min(pcsItem->m_nCount, lRemained);
		else
			lRemove = 1;
		
		SubItemStackCount(pcsItem, lRemove);
		lRemained -= lRemove;
		
		if(0 >= lRemained)
			return TRUE;
	}

	return FALSE;
}

// 2008.03.18. steeple
// On/Off 가능한 Expired Time 을 가진 아이템을 Off 시켜도 시간은 흘러간다.
BOOL AgpmItem::IsContinuousOffItem(AgpdItemTemplate* pcsItemTemplate)
{
	if(!pcsItemTemplate)
		return FALSE;

	return pcsItemTemplate->m_bContinuousOff;
}

// 2008.04.02. steeple
UINT32 AgpmItem::GetReuseInterval(AgpdItemTemplate* pcsItemTemplate)
{
	if(!pcsItemTemplate || pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE)
		return 0;

	return ((AgpdItemTemplateUsable*)pcsItemTemplate)->m_ulUseInterval + ((AgpdItemTemplateUsable*)pcsItemTemplate)->m_lEffectActivityTimeMsec * ((AgpdItemTemplateUsable*)pcsItemTemplate)->m_lEffectApplyCount;
}

// 2008.04.02. steeple
// ReuseTime 과 Duration 과 RemainTime 을 비교해서 Pause 인건지, 완전 끝난건지 확인시켜준다.
// 맨 뒤의 ElapsedClockCount 는 idle latency 를 처리하기 위해서이다.
//
// Return Value : TRUE(이 아이템은 완전 정지), FALSE (이 아이템은 Pause 상태)
BOOL AgpmItem::CheckReuseIntervalWithCooldown(AgpdItem* pcsItem, UINT32 ulRemainTime, UINT32 ulElapsedClockCount)
{
	UINT32 ulReuseInterval = GetReuseInterval(pcsItem->m_pcsItemTemplate);
	if(ulReuseInterval == 0)
		return TRUE;

	INT64 llDuration = pcsItem->m_pcsItemTemplate->m_lRemainTime;
	if((INT64)(ulReuseInterval) < llDuration)
		return TRUE;

	INT64 lValue = ((INT64)ulReuseInterval) - (llDuration) - ((INT64)ulRemainTime);// - ((INT64)ulElapsedClockCount * 2);
	if(lValue >= 0)
		return TRUE;
	else
		return FALSE;
}

// 현재 이 아이템이 캐릭터가 사용중인지 아닌지 여부
BOOL AgpmItem::IsUsingItem(AgpdItem* pcsItem)
{
	if(!pcsItem || !pcsItem->m_pcsItemTemplate)
		return FALSE;

	if(pcsItem->m_nInUseItem == AGPDITEM_CASH_ITEM_INUSE)
		return TRUE;

	return FALSE;		
}

BOOL AgpmItem::MakeAndSendPacketEquipInfo(INT32 lIID, INT32 lCID)
{
	PVOID	pvPacket;
	PVOID	pvEquipPacket;
	INT16	nSize;
	INT8	cOperation = AGPMITEM_PACKET_OPERATION_UPDATE;
	INT8	CStatus    = AGPDITEM_STATUS_EQUIP;

	pvEquipPacket  = m_csPacketEquip.MakePacket(FALSE, &nSize, AGPMITEM_PACKET_TYPE);

	if(!pvEquipPacket)
	{
		char strCharBuff[256] = { 0, };
		sprintf_s(strCharBuff, sizeof(strCharBuff), "AgpmItem::MakeAndSendPacketEquipInfo() Error (1) !!!\n");
		AuLogFile_s(ALEF_ERROR_FILENAME, strCharBuff);

		return FALSE;
	}

	pvPacket		   = m_csPacket.MakePacket(TRUE, &nSize, AGPMITEM_PACKET_TYPE,
												&cOperation,
												&CStatus,
												&lIID,
												NULL,
												&lCID,
												NULL,
												NULL,
												NULL,
												NULL,
												pvEquipPacket,
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
												NULL);

	m_csPacket.FreePacket(pvEquipPacket);

	if(!pvPacket)
	{
		char strCharBuff[256] = { 0, };
		sprintf_s(strCharBuff, sizeof(strCharBuff), "AgpmItem::MakeAndSendPacketEquipInfo() Error (2) !!!\n");
		AuLogFile_s(ALEF_ERROR_FILENAME, strCharBuff);

		return FALSE;
	}

	if(!SendPacket(pvPacket, nSize))
	{
		m_csPacket.FreePacket(pvPacket);
		char strCharBuff[256] = { 0, };
		sprintf_s(strCharBuff, sizeof(strCharBuff), "AgpmItem::MakeAndSendPacketEquipInfo() Error (3) !!!\n");
		AuLogFile_s(ALEF_ERROR_FILENAME, strCharBuff);

		return FALSE;
	}

	m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

BOOL AgpmItem::_AddPrivateTradeItem( DWORD dwIID , AgpdCharacter* pcsCharacter )
{
	return EnumCallback( ITEM_CB_ID_UI_UPDATE_TRADE_OPTION , (PVOID)((ULONG_PTR)dwIID), (PVOID)pcsCharacter );
}

VOID	AgpmItem::ItemTimeUpdate( AgpdItem*	pAgpdItem , PVOID	pCashInfomation )
{
	if (pCashInfomation)
	{
		m_csPacketCashInformaion.GetField(FALSE, pCashInfomation, 0,
			&pAgpdItem->m_nInUseItem, 
			&pAgpdItem->m_lRemainTime,
			&pAgpdItem->m_lExpireTime,
			&pAgpdItem->m_lCashItemUseCount,
			&pAgpdItem->m_llStaminaRemainTime);
	}
}

BOOL AgpmItem::IsEquipUnableItemInEpicZone(AgpdCharacter* pcsCharacter)
{
	if(NULL == pcsCharacter)
		return FALSE;

	BOOL bEquipUnableItem = FALSE;

	for(INT32 lPart = (INT32)AGPMITEM_PART_BODY; lPart < (INT32)AGPMITEM_PART_NUM; ++lPart)
	{
		AgpdGridItem* pcsGridItem = GetEquipItem(pcsCharacter, lPart);
		if(NULL == pcsGridItem)
			continue;

		AgpdItem *pcsItem = GetItem(pcsGridItem);
		if(NULL == pcsItem)
			continue;

		if(AGPMITEM_SECTION_TYPE_UNABLE_USE_EPIC_ZONE == GetItemSectionNum(pcsItem))
		{
			bEquipUnableItem = TRUE;
			break;
		}
	}

	return bEquipUnableItem;
}

BOOL AgpmItem::AdjustDragonScionWeaponFactor(AgpdCharacter *pcsCharacter, AgpdItem *pcsItem, AgpdFactor *pcsFactorPoint, AgpdFactor *pcsFactorPercent, BOOL bTransform)
{
	if(NULL == pcsCharacter || NULL == pcsItem || NULL == pcsFactorPoint || NULL == pcsFactorPercent)
		return FALSE;

	m_pagpmFactors->CopyFactor(pcsFactorPoint, &pcsItem->m_csFactor, TRUE);
	m_pagpmFactors->CopyFactor(pcsFactorPercent, &pcsItem->m_csFactorPercent, TRUE);

	AgpdItem* pcsItemL		= GetEquipSlotItem(pcsCharacter, AGPMITEM_PART_HAND_LEFT);
	AgpdItem* pcsItemR		= GetEquipSlotItem(pcsCharacter, AGPMITEM_PART_HAND_RIGHT);

	if(!bTransform)
	{
		if(GetWeaponType(pcsItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_ZENON)
		{
			if(pcsItemL && (CheckUseItem(pcsCharacter, pcsItemL) || bTransform == FALSE) && 
				GetWeaponType(pcsItemL->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_CHARON)
			{
				m_pagpmFactors->SetValue(pcsFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
				m_pagpmFactors->SetValue(pcsFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
			}
		}

	}

	if(!bTransform)
	{
		if(GetWeaponType(pcsItem->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_CHARON)
		{
			if(pcsItemR && (CheckUseItem(pcsCharacter, pcsItemR) || bTransform == FALSE) && 
				GetWeaponType(pcsItemR->m_pcsItemTemplate) == AGPMITEM_EQUIP_WEAPON_TYPE_ONE_HAND_ZENON)
			{
				m_pagpmFactors->SetValue(pcsFactorPoint, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
				m_pagpmFactors->SetValue(pcsFactorPercent, 0, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SPEED);
			}
		}
	}

	return TRUE;
}

BOOL AgpmItem::StreamReadClassfyID( AgpdItemTemplate* pcsItemTemplate, AuExcelLib& csExcelTxtLib, INT32 lCurCol, INT32 lCurRow )
{
	if(!pcsItemTemplate)
		return FALSE;

	CHAR* szBuffer = NULL;
	CHAR* szToken = NULL;
	CHAR szSeps[] = ";";

	szBuffer = csExcelTxtLib.GetData(lCurCol, lCurRow);
	if(!szBuffer || !strlen(szBuffer))
		return FALSE;

	INT32 lIndex = 0;
	// 문자열 형식은 xxx;xxx;xxx 이런식임
	szToken = strtok(szBuffer, szSeps);
	while(szToken)
	{
		INT32 lValue = atoi(szToken);
		pcsItemTemplate->m_vClassifyID.push_back(lValue);
		if(lIndex >= AGPDITEM_LINK_MAX_NUM)
			break;

		szToken = strtok(NULL, szSeps);
	}

	return TRUE;
}

BOOL AgpmItem::IsSameClassfyID( AgpdItemTemplate* pcsItemTemplate, AgpdItemTemplate* pcsGridItemTemplate )
{
	if (!pcsItemTemplate || !pcsGridItemTemplate)
		return FALSE;

	vector<INT32>::iterator iterItem;
	vector<INT32>::iterator iterGridItem;

	for (iterItem = pcsItemTemplate->m_vClassifyID.begin(); iterItem != pcsItemTemplate->m_vClassifyID.end(); ++iterItem)
	{
		for (iterGridItem = pcsGridItemTemplate->m_vClassifyID.begin(); iterGridItem != pcsGridItemTemplate->m_vClassifyID.end(); ++iterGridItem)
		{
			if (*iterItem == *iterGridItem)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL AgpmItem::OnOperationItemInformation( AgpdCharacter* pcsCharacter, INT32 llID, INT32 lTID, INT32 lType )
{
	if(!pcsCharacter)
		return FALSE;

	AgpdItem *pcsItem = GetItem(llID);
	AgpdItemTemplate *pcsItemTemplate = GetItemTemplate(lTID);
	if(!pcsItem && !pcsItemTemplate)
		return FALSE;

	PVOID pvData[3];
	pvData[0] = pcsItem;
	pvData[1] = pcsItemTemplate;
	pvData[2] = &lType;

	EnumCallback(ITEM_CB_ITEM_INFORMATION, pcsCharacter, pvData);

	return TRUE;
}

BOOL AgpmItem::OnOperationItemInformationResult( BOOL bReceivedFromServer, INT8 cStatus, INT32 lIID, INT32 lTID, INT32 lCID, INT32 lItemCount, PVOID pField, PVOID pInventory, PVOID pBank, PVOID pEquip, PVOID pFactor, PVOID pFactorPercent, PVOID pRestrictFactor, PVOID pConvert, PVOID pQuest, INT32 lSkillTID, INT32 lStatusFlag, PVOID pOption, PVOID pSkillPlus, PVOID pvCashInformation, PVOID pvExtraInformation, INT32 tooltipType )
{
	AgpdItem   *pstAgpdItem;
	//BOOL		bNotIID		=	FALSE;
	if((!lIID) && (!lTID))
		return FALSE; 

	if( !lIID  )
	{
		// 없으면 그냥 임시로 만듬
		lIID	=	830530 + lTID ;
		//bNotIID	=	TRUE;
	}

	pstAgpdItem = AddItem(lIID, lTID, (lItemCount < 0) ? 0 : lItemCount , FALSE );

	if(!pstAgpdItem)
		return FALSE;

	if (m_pagpmFactors)
	{
		if (pFactor)
		{
			m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csFactor, pFactor, 0);
		}

		if (pFactorPercent)
			m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csFactorPercent, pFactorPercent, 0);

		if (pRestrictFactor)
			m_pagpmFactors->ReflectPacket(&pstAgpdItem->m_csRestrictFactor, pRestrictFactor, 0);

		if (pFactor || pFactorPercent || pRestrictFactor)
			EnumCallback(ITEM_CB_ID_UPDATE_FACTOR, pstAgpdItem, NULL);
	}

	pstAgpdItem->m_eNewStatus = cStatus;

	pstAgpdItem->m_lSkillTID	= lSkillTID;

	if (lStatusFlag != (-1))
		pstAgpdItem->m_lStatusFlag	= lStatusFlag;

	if (pOption)
	{
		pstAgpdItem->m_aunOptionTID.MemSetAll();
		pstAgpdItem->m_apcsOptionTemplate.MemSetAll();

		pstAgpdItem->m_aunOptionTIDRune.MemSetAll();

		m_csPacketOption.GetField(FALSE, pOption, 0,
			&pstAgpdItem->m_aunOptionTID[0],
			&pstAgpdItem->m_aunOptionTID[1],
			&pstAgpdItem->m_aunOptionTID[2],
			&pstAgpdItem->m_aunOptionTID[3],
			&pstAgpdItem->m_aunOptionTID[4]);

		pstAgpdItem->m_apcsOptionTemplate[0]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[0]);
		pstAgpdItem->m_apcsOptionTemplate[1]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[1]);
		pstAgpdItem->m_apcsOptionTemplate[2]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[2]);
		pstAgpdItem->m_apcsOptionTemplate[3]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[3]);
		pstAgpdItem->m_apcsOptionTemplate[4]	= GetItemOptionTemplate((INT32) pstAgpdItem->m_aunOptionTID[4]);
	}

	if (pSkillPlus)
	{
		pstAgpdItem->m_aunSkillPlusTID.MemSetAll();

		m_csPacketSkillPlus.GetField(FALSE, pSkillPlus, 0,
			&pstAgpdItem->m_aunSkillPlusTID[0],
			&pstAgpdItem->m_aunSkillPlusTID[1],
			&pstAgpdItem->m_aunSkillPlusTID[2]);
	}


	if (pvCashInformation)
	{
		m_csPacketCashInformaion.GetField(FALSE, pvCashInformation, 0,
			&pstAgpdItem->m_nInUseItem, 
			&pstAgpdItem->m_lRemainTime,
			&pstAgpdItem->m_lExpireTime,
			&pstAgpdItem->m_lCashItemUseCount,
			&pstAgpdItem->m_llStaminaRemainTime);
	}

	if(pvExtraInformation)
	{
		PVOID strBuffer = 0;
		m_csPacketExtra.GetField(FALSE, pvExtraInformation, 0, &strBuffer);

		PACKET_AGPMITEM_EXTRA* pPacketExtra = (PACKET_AGPMITEM_EXTRA*)strBuffer;

		switch(pPacketExtra->Type)
		{
		case 1: // Seal Data
			{
				AgpdSealData pSealData;
				memcpy(&pSealData, &pPacketExtra->ExtraData.SealData, sizeof(AgpdSealData));

				EnumCallback( ITEM_CB_ID_ADD_ITEM_EXTRADATA, pstAgpdItem, &pSealData );
			} break;

		default :
			EnumCallback( ITEM_CB_ID_ADD_ITEM_EXTRADATA, pstAgpdItem, NULL );
			break;
		}
	}
	else
	{
		EnumCallback( ITEM_CB_ID_ADD_ITEM_EXTRADATA, pstAgpdItem, NULL );
	}

	if(!InitItem(pstAgpdItem))
	{
		OutputDebugString("AgpmItem::OnReceive() Error (3) !!!\n");

		return FALSE;
	}

	EnumCallback(ITEM_CB_ITEM_INFORMATION_RESULT, pstAgpdItem, (PVOID)((ULONG_PTR)tooltipType));

	return TRUE;
}

#define		SHRINECOIN_TID		21044
INT32	AgpmItem::GetCharShrineCoinCount( AgpdCharacter* pcsCharacter )
{
	if( !pcsCharacter )
		return 0;

	AgpdItemADChar	*pcsItemADChar	= GetADCharacter(pcsCharacter);
	if (!pcsItemADChar)
		return NULL;

	AgpdItemTemplate	*pcsItemTemplate	=	NULL;
	AgpdItem			*pcsItem			=	NULL;
	INT					nCount				=	0;

	for (int i = 0; i < pcsItemADChar->m_csInventoryGrid.m_nLayer; ++i)
	{
		for (int j = 0; j < pcsItemADChar->m_csInventoryGrid.m_nColumn; ++j)
		{
			for (int k = 0; k < pcsItemADChar->m_csInventoryGrid.m_nRow; ++k)
			{
				AgpdGridItem	*pcsGridItem	= m_pagpmGrid->GetItem(&pcsItemADChar->m_csInventoryGrid, i, k, j);
				if (pcsGridItem)
				{
					pcsItem		= GetItem(pcsGridItem);
					if (pcsItem && pcsItem->m_pcsItemTemplate)
					{
						if( pcsItem->m_pcsItemTemplate->m_nType == AGPMITEM_TYPE_OTHER && ((AgpdItemTemplateOther*)pcsItem->m_pcsItemTemplate)->m_eOtherItemType == AGPMITEM_OTHER_TYPE_SHRINE_COIN )
							nCount +=	pcsItem->m_nCount;
					}
				}
			}
		}
	}

	return nCount;
}
BOOL AgpmItem::StreamReadGachaDropPointTable( CHAR *szFile, BOOL bDecryption )
{
	if (!szFile || !szFile[0])
		return FALSE;

	AuExcelTxtLib	csExcelTxtLib;

	if(!csExcelTxtLib.OpenExcelFile(szFile, TRUE, bDecryption))
	{
		TRACE("AgpmItem::StreamReadTransformData() Error (OpenExcelFile) !!!\n");
		csExcelTxtLib.CloseFile();
		return FALSE;
	}

	INT32	lNumRow			= csExcelTxtLib.GetRow();
	INT32	lNumColumn		= csExcelTxtLib.GetColumn();

	CHAR	*szColumnName	= NULL;
	CHAR	*szValue		= NULL;

	for (int i = 1; i < lNumRow; ++i)
	{
		szValue	= csExcelTxtLib.GetData(0, i);
		if (!szValue)
			continue;

		AgpdItemTemplate *pcsTemplate	= GetItemTemplate(atoi(szValue));
		if (!pcsTemplate)
		{
			ASSERT("AgpmItem::StreamReadGachaDropPointTable() Error : pcsTemplate is NULL\n");
			continue;
		}

		for (int j = 1; j < lNumColumn; ++j)
		{
			szColumnName	= csExcelTxtLib.GetData(j, 0);
			szValue			= csExcelTxtLib.GetData(j, i);

			if (!szColumnName || !szValue)
				continue;

			if (strncmp(szColumnName, AGPMITEM_GACHA_DROP_POINT_EXP, strlen(AGPMITEM_GACHA_DROP_POINT_EXP)) == 0)
			{
				pcsTemplate->m_stGachaPoint.m_nPointEXP				= atoi(szValue);
			}
			else if (strncmp(szColumnName, AGPMITEM_GACHA_DROP_POINT_GHELD, strlen(AGPMITEM_GACHA_DROP_POINT_GHELD)) == 0)
			{
				pcsTemplate->m_stGachaPoint.m_nPointGheld			= atoi(szValue);
			}
			else if (strncmp(szColumnName, AGPMITEM_GACHA_DROP_POINT_CHARISMA, strlen(AGPMITEM_GACHA_DROP_POINT_CHARISMA)) == 0)
			{
				pcsTemplate->m_stGachaPoint.m_nPointCharismaPoint	= atoi(szValue);
			}
			else if (strncmp(szColumnName, AGPMITEM_GACHA_DROP_PERCENT_EXP, strlen(AGPMITEM_GACHA_DROP_PERCENT_EXP)) == 0)
			{
				pcsTemplate->m_stGachaPoint.m_nPercentEXP			= atoi(szValue);
			}
			else if (strncmp(szColumnName, AGPMITEM_GACHA_DROP_PERCENT_GHELD, strlen(AGPMITEM_GACHA_DROP_PERCENT_GHELD)) == 0)
			{
				pcsTemplate->m_stGachaPoint.m_nPercentGheld			= atoi(szValue);
			}
			else if (strncmp(szColumnName, AGPMITEM_GACHA_DROP_PERCENT_CHARISMA, strlen(AGPMITEM_GACHA_DROP_PERCENT_CHARISMA)) == 0)
			{
				pcsTemplate->m_stGachaPoint.m_nPercentCharismaPoint	= atoi(szValue);
			}
		}
	}

	csExcelTxtLib.CloseFile();

	return TRUE;
}
