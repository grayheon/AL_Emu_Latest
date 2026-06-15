#include "AgcmUIEventGacha.h"
#include "AuStrTable.h"

INT32	g_nDummy = 0;
extern INT32	g_nRotateCount;


AgcmUIEventGacha::AgcmUIEventGacha():
	m_pcsApmEventManager	( NULL ),
	m_pcsAgcmUIManager2		( NULL ),
	m_pcsAgcmEventGacha		( NULL ),
	m_lEventGachaOpen		( -1 ),
	m_lEventGachaError		( -1 ),
	m_lEventGachaRoll		( -1 ),
	m_lEventGachaClose		( -1 ),
	m_bOpenGachaUI			( FALSE )
{
	SetModuleName("AgcmUIEventGacha");
}


LuaGlue	LG_LoadGachaSetting( lua_State *L )
{
	AuLua * pLua = AuLua::GetPointer();
	const char *pStr = pLua->GetStringArgument( 1 , "RollTable.txt" );

	AgcmUIEventGacha	* pcsAgcmUIEventGacha = ( AgcmUIEventGacha * )g_pEngine->GetModule( "AgcmUIEventGacha" );

	if( pcsAgcmUIEventGacha->m_cUIGacha.LoadGachaSetting( pStr ) )
	{
		g_pEngine->LuaErrorMessage( "Gacha Rolling Table Load Succeeded" );
	}
	else
	{
		g_pEngine->LuaErrorMessage( "Gacha Rolling Table Load Failed" );
	}
	
	LUA_RETURN( TRUE )
}

LuaGlue	LG_GachaRollCount( lua_State *L )
{
	AuLua * pLua = AuLua::GetPointer();
	INT32	nCount = ( INT32 ) pLua->GetNumberArgument( 1 , 7 );

	g_nRotateCount = nCount;

	g_pEngine->LuaErrorMessage( "Gacha Roll Count Changed" );
	
	LUA_RETURN( TRUE )
}

LuaGlue	LG_GachaTestRoll( lua_State *L )
{
	AuLua * pLua = AuLua::GetPointer();
	AgcmUIEventGacha	* pcsAgcmUIEventGacha = ( AgcmUIEventGacha * )g_pEngine->GetModule( "AgcmUIEventGacha" );

	if( pcsAgcmUIEventGacha->m_cUIGacha.m_vecSlot.size() ) 
	{
		MTRand	mtRnd;
		INT32	nRandomTID = pcsAgcmUIEventGacha->m_cUIGacha.m_vecSlot[ mtRnd.randInt( pcsAgcmUIEventGacha->m_cUIGacha.m_vecSlot.size() ) ].nTID;
		pcsAgcmUIEventGacha->m_cUIGacha.StartRoll( nRandomTID );
	}
	else
	{
		g_pEngine->LuaErrorMessage( "Gacha is not initialized" );
	}

	LUA_RETURN( TRUE )
}

void	AgcmUIEventGacha::OnLuaInitialize	( AuLua * pLua			)
{
	luaDef MVLuaGlue[] = 
	{
		{"LoadGachaSetting"		,	LG_LoadGachaSetting	},
		{"GachaTestRoll"		,	LG_GachaTestRoll	},
		{"GachaRollCount"		,	LG_GachaRollCount	},
		{NULL					,	NULL				},
	};

	for(int i=0; MVLuaGlue[i].name; i++)
	{
		pLua->AddFunction(MVLuaGlue[i].name, MVLuaGlue[i].func);
	}
}


AgcmUIEventGacha::~AgcmUIEventGacha()
{
}

BOOL	AgcmUIEventGacha::OnAddModule()
{
	m_pcsApmEventManager	= ( ApmEventManager	 * )	GetModule( "ApmEventManager" );
	m_pcsAgcmUIManager2		= ( AgcmUIManager2	 * )	GetModule( "AgcmUIManager2" );
	m_pcsAgcmEventGacha		= ( AgcmEventGacha	 * )	GetModule( "AgcmEventGacha" );

	if( !m_pcsApmEventManager || !m_pcsAgcmUIManager2 || !m_pcsAgcmEventGacha )
		return FALSE;

	if( !AddEvent() )
		return FALSE;

	AgcmCharacter * pcsAgcmCharacter = ( AgcmCharacter * ) GetModule( "AgcmCharacter" );
	if( pcsAgcmCharacter )
	{
		pcsAgcmCharacter->SetCallbackSelfUpdatePosition( CBSelfCharacterUpdatePosition , this );
	}

	return TRUE;
}

BOOL	AgcmUIEventGacha::AddEvent()
{
	m_lEventGachaOpen = m_pcsAgcmUIManager2->AddEvent("Gacha_OpenUI");
	if (m_lEventGachaOpen < 0)
		return FALSE;
	m_lEventGachaError = m_pcsAgcmUIManager2->AddEvent("Gacha_Error");
	if (m_lEventGachaError < 0)
		return FALSE;
	m_lEventGachaRoll = m_pcsAgcmUIManager2->AddEvent("Gacha_Roll");
	if (m_lEventGachaRoll < 0)
		return FALSE;
	m_lEventGachaClose = m_pcsAgcmUIManager2->AddEvent("Gacha_CloseUI");
	if (m_lEventGachaClose < 0)
		return FALSE;

	m_pcsAgcmEventGacha->SetCallbackOpenGachaUI	( CBOpenGachaUI		, this );
	m_pcsAgcmEventGacha->SetCallbackGachaRoll	( CBReceiveResult	, this );

	if (!m_pcsAgcmUIManager2->AddCustomControl("GachaWindow", &m_cUIGacha))
		return FALSE;

	m_cUIGacha.m_pcsAgcmUIEventGacha = this;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "Gacha_Roll", CBGachaRoll, 1 ))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "Gacha_Close", CBGachaClose, 1 ))
		return FALSE;
	m_pcsUDNotice	= m_pcsAgcmUIManager2->AddUserData("Gacha_TextData", &g_nDummy , sizeof INT32 , 1 , AGCDUI_USERDATA_TYPE_STRING );
	m_pcsUDItemName	= m_pcsAgcmUIManager2->AddUserData("Gacha_ItemName", &g_nDummy , sizeof INT32 , 1 , AGCDUI_USERDATA_TYPE_STRING );
	if(!m_pcsUDNotice)
		return FALSE;
	if(!m_pcsUDItemName)
		return FALSE;

	if(!m_pcsAgcmUIManager2->AddDisplay(this, "Gacha_Display_Notice", 0, CBDisplayGachaMessage, GACHA_MESSAGE_LENGTH ))
		return FALSE;
	if(!m_pcsAgcmUIManager2->AddDisplay(this, "Gacha_Display_Item", 0, CBDisplayGachaItem, GACHA_MESSAGE_LENGTH ))
		return FALSE;

	return TRUE;
}

void	AgcmUIEventGacha::ShowErrorReport( AgpmEventGacha::ERROR_CODE eErrorCode )
{
	switch( eErrorCode )
	{
	case 		AgpmEventGacha::EC_NOERROR					:
		break;
	case 		AgpmEventGacha::EC_NOT_ENOUGH_ITEM			:
		m_pcsAgcmUIManager2->ActionMessageOKDialog( ClientStr().GetStr(STI_GACHA_EC_NOT_ENOUGH_ITEM) );
		break;
	case 		AgpmEventGacha::EC_NOT_ENOUGH_MONEY			:
		m_pcsAgcmUIManager2->ActionMessageOKDialog( ClientStr().GetStr(STI_GACHA_EC_NOT_ENOUGH_MONEY) );
		break;
	case 		AgpmEventGacha::EC_NOT_ENOUGH_INVENTORY		:
		m_pcsAgcmUIManager2->ActionMessageOKDialog( ClientStr().GetStr(STI_GACHA_EC_NOT_ENOUGH_INVENTORY) );
		break;
	case 		AgpmEventGacha::EC_NOT_ENOUGH_CHARISMA	:
		m_pcsAgcmUIManager2->ActionMessageOKDialog( ClientStr().GetStr(STI_GACHA_EC_NOT_ENOUGH_CHARISMA) );
		break;
	case 		AgpmEventGacha::EC_GACHA_ERROR				:
		m_pcsAgcmUIManager2->ActionMessageOKDialog( ClientStr().GetStr(STI_GACHA_EC_NOT_ETC_ERROR) );
		break;
	case 		AgpmEventGacha::EC_LEVELLIMIT				:
		m_pcsAgcmUIManager2->ActionMessageOKDialog( ClientStr().GetStr(STI_GACHA_EC_LEVEL_LIMIT) );
		break;
	}
}

BOOL	AgcmUIEventGacha::CBOpenGachaUI(PVOID pData, PVOID pClass, PVOID pCustData)
{
	AgcmUIEventGacha	* pThis = ( AgcmUIEventGacha * ) pClass;
	AgpmEventGacha::GachaInfo	*pstGachaInfo = ( AgpmEventGacha::GachaInfo * ) pData ;
	ApBase *pGenerator	=(ApBase *) pCustData;

	pThis->m_stGachaInfo = *pstGachaInfo;

	pThis->UpdateControlNotice();
	pThis->UpdateControlItem();

	// 이미 떠있으면 막아라.
	if( pThis->m_bOpenGachaUI && pThis->m_cUIGacha.IsRollingNow() ) return FALSE;

	/*
	vector<INT32> vecTID;
	vecTID.push_back( 3034 );
	vecTID.push_back( 4624 );
	vecTID.push_back( 4625 );
	vecTID.push_back( 11 );
	vecTID.push_back( 1965 );
	vecTID.push_back( 18 );
	vecTID.push_back( 488 );
	vecTID.push_back( 1895 );
	vecTID.push_back( 51 );
	vecTID.push_back( 4001 );
	*/


	// 일단 가챠 창은 띄우는게 좋겠다.
	// 에러 체크는 하지 않음..
	AgpmEventGacha::ERROR_CODE eErrorCode = ( AgpmEventGacha::ERROR_CODE ) pstGachaInfo->lResult;

	switch( eErrorCode )
	{
	case 		AgpmEventGacha::EC_NOERROR					:
	case 		AgpmEventGacha::EC_NOT_ENOUGH_ITEM			:
	case 		AgpmEventGacha::EC_NOT_ENOUGH_MONEY			:
	case 		AgpmEventGacha::EC_NOT_ENOUGH_CHARISMA		:
	case 		AgpmEventGacha::EC_NOT_ENOUGH_INVENTORY		:
		// 돈이 없어도 인벤이 없어도
		// 일단 창은 보임
		pThis->m_cUIGacha.SetSlotItems( &pstGachaInfo->vecTID );
		pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_lEventGachaOpen );
		pThis->m_bOpenGachaUI	= TRUE;
		pThis->m_pcsAgcmEventGacha->SetGachaWindowOpen( FALSE );

		// 포지션을 저장해둠..
		switch( pstGachaInfo->pcsEvent->m_pcsSource->m_eType )
		{
		case APBASE_TYPE_OBJECT:
			{
				ApdObject	* pcsApdObject = ( ApdObject * ) pstGachaInfo->pcsEvent->m_pcsSource;
				pThis->m_posGachaOpenPosition = pcsApdObject->m_stPosition;
			}
			break;
		case APBASE_TYPE_CHARACTER:
			{
				AgpdCharacter	* pcsAgpdCharacter = ( AgpdCharacter * ) pstGachaInfo->pcsEvent->m_pcsSource;
				pThis->m_posGachaOpenPosition = pcsAgpdCharacter->m_stPos;
			}
			break;
		default:
			// 어라라라..
			ASSERT(!"오브젝트와 케릭터 이외의 녀석에 가챠가 붙어있따" );
			break;
		}

		break;

	case 		AgpmEventGacha::EC_GACHA_ERROR				:
	case 		AgpmEventGacha::EC_LEVELLIMIT				:
		pThis->ShowErrorReport( eErrorCode );
		break;
	}
	return TRUE;
}

BOOL	AgcmUIEventGacha::CBReceiveResult(PVOID pData, PVOID pClass, PVOID pCustData)
{
	AgcmUIEventGacha	* pThis = ( AgcmUIEventGacha * ) pClass;

	//pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_lEventGachaRoll );

	AgpmEventGacha::ERROR_CODE	eError	= ( AgpmEventGacha::ERROR_CODE ) PtrToInt( pData );
	INT32	nTID	= PtrToInt( pCustData );

	switch( eError )
	{
	case 		AgpmEventGacha::EC_NOERROR					:
		pThis->m_cUIGacha.StartRoll( nTID );
		break;
	default:
		pThis->ShowErrorReport( eError );
		break;
	}
		/*
	가챠 디버그 코드
	MTRand	mtRnd;
	INT32	nRandomTID = pThis->m_cUIGacha.m_vecSlot[ mtRnd.randInt( pThis->m_cUIGacha.m_vecSlot.size() ) ].nTID;

	pThis->m_cUIGacha.StartRoll( nRandomTID );
	*/

	return TRUE;
}

BOOL	AgcmUIEventGacha::CBGachaClose	(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIEventGacha	* pThis = ( AgcmUIEventGacha * ) pClass;

	if( pThis->m_cUIGacha.IsRollingNow() ) return FALSE;

	pThis->m_bOpenGachaUI	= FALSE;

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGachaClose);

	pThis->m_pcsAgcmEventGacha->SetGachaWindowOpen( FALSE );


	return TRUE;
}

BOOL	AgcmUIEventGacha::CBGachaRoll	(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIEventGacha	* pThis = ( AgcmUIEventGacha * ) pClass;

	if( pThis->m_cUIGacha.IsRollingNow() ) return FALSE;

	/*
	가챠 디버그 코드
	MTRand	mtRnd;
	INT32	nRandomTID = pThis->m_cUIGacha.m_vecSlot[ mtRnd.randInt( pThis->m_cUIGacha.m_vecSlot.size() ) ].nTID;

	pThis->m_cUIGacha.StartRoll( nRandomTID );
	*/

	AgcmCharacter * pcsAgcmCharacter = ( AgcmCharacter * ) g_pEngine->GetModule( "AgcmCharacter" );

	if( !pcsAgcmCharacter ) return FALSE; 

	AgpdCharacter * pcsSelfCharacter = pcsAgcmCharacter->GetSelfCharacter();

	if( !pcsSelfCharacter ) return FALSE; 

	INT16 nPacketLength = 0;

	AgpmEventGacha * pcsAgpmEventGacha = ( AgpmEventGacha * ) g_pEngine->GetModule( "AgpmEventGacha" );

	PVOID pvPacket = pcsAgpmEventGacha->MakePacketEventGacha( 
		pThis->m_stGachaInfo.pcsEvent , 
		pcsSelfCharacter->m_lID,
		&nPacketLength );

	if( pvPacket )
	{
		BOOL bResult = pcsAgpmEventGacha->SendPacket(pvPacket, nPacketLength);
		pcsAgpmEventGacha->m_csPacket.FreePacket(pvPacket);
		return TRUE;
	}
	else
	{
		// 패킷 생성실패.
		return FALSE;
	}
}

void	AgcmUIEventGacha::UpdateControlNotice()
{
	m_pcsUDNotice->m_bUpdateList = TRUE;
	m_pcsAgcmUIManager2->RefreshUserData(m_pcsUDNotice, TRUE);
}

void	AgcmUIEventGacha::UpdateControlItem()
{
	m_pcsUDItemName->m_bUpdateList = TRUE;
	m_pcsAgcmUIManager2->RefreshUserData(m_pcsUDItemName, TRUE);
}

BOOL		AgcmUIEventGacha::CBDisplayGachaMessage(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue , AgcdUIControl *pcsSourceControl)
{
	AgcmUIEventGacha	* pThis = ( AgcmUIEventGacha * ) pClass;
	if (pcsSourceControl->m_lType != AcUIBase::TYPE_EDIT) return FALSE;

	AcUIEdit* pUIEdit = (AcUIEdit *) pcsSourceControl->m_pcsBase;
	if (!pUIEdit) return FALSE;

	char	str[ 256 ] = {0,};
	if( pThis->m_stGachaInfo.lTID && pThis->m_stGachaInfo.lItemCount )
	{
		AgpmItem * pcsAgpmItem	= ( AgpmItem * ) g_pEngine->GetModule( "AgpmItem" );
		AgpdItemTemplate	* pcsAgpdItemTemplate	 = pcsAgpmItem->GetItemTemplate( pThis->m_stGachaInfo.lTID );
		ASSERT( pcsAgpdItemTemplate );		

#ifdef _AREA_GLOBAL_ // 어순이 다름.
		if(pThis->m_stGachaInfo.lMoney)
			sprintf( str , ClientStr().GetStr(STI_GACHA_DISPLAY_MESSAGE) ,
				pThis->m_stGachaInfo.lItemCount, pcsAgpdItemTemplate->m_szName, pThis->m_stGachaInfo.lMoney );
		else
			sprintf( str, ClientStr().GetStr(STI_GACHA_DISPLAY_MESSAGE4) , pThis->m_stGachaInfo.lItemCount, pcsAgpdItemTemplate->m_szName );
#else
		sprintf( str , ClientStr().GetStr(STI_GACHA_DISPLAY_MESSAGE) ,
			pcsAgpdItemTemplate->m_szName , pThis->m_stGachaInfo.lItemCount  , pThis->m_stGachaInfo.lMoney );
#endif
	}
	else
	{
		char	tmpStr[ 256 ] = {0,};
		if( pThis->m_stGachaInfo.lMoney != 0 )
			sprintf( str, ClientStr().GetStr(STI_GACHA_DISPLAY_MESSAGE2) , pThis->m_stGachaInfo.lMoney );
		if( pThis->m_stGachaInfo.lCharisma != 0 )
			sprintf( tmpStr, ClientStr().GetStr(STI_GACHA_DISPLAY_MESSAGE3), pThis->m_stGachaInfo.lCharisma );

		strcat(str, tmpStr);
	}

	pUIEdit->SetText( str );

	return TRUE;
}

BOOL		AgcmUIEventGacha::CBDisplayGachaItem(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue , AgcdUIControl *pcsSourceControl)
{
	AgcmUIEventGacha	* pThis = ( AgcmUIEventGacha * ) pClass;

	if (pcsSourceControl->m_lType != AcUIBase::TYPE_EDIT) return FALSE;

	AcUIEdit* pUIEdit = (AcUIEdit *) pcsSourceControl->m_pcsBase;
	if (!pUIEdit) return FALSE;

	pUIEdit->SetText( ( char * ) pThis->m_stringName.c_str() );

	return TRUE;
}

BOOL	AgcmUIEventGacha::OnRollEnd()	// 가챠 롤이 다 끝나고 결과가 나온경우
{
	INT32	nTID = m_cUIGacha.m_nResultTID;

	AgpmItem * pcsAgpmItem	= ( AgpmItem * ) g_pEngine->GetModule( "AgpmItem" );
	AgpdItemTemplate	* pcsAgpdItemTemplate	 = pcsAgpmItem->GetItemTemplate( nTID );
	ASSERT( pcsAgpdItemTemplate );

	{
		AgcmCharacter * pcsAgcmCharacter = ( AgcmCharacter * ) g_pEngine->GetModule( "AgcmCharacter" );
		AgpdCharacter * pcsSelfCharacter = pcsAgcmCharacter->GetSelfCharacter();
		if( pcsSelfCharacter ) 
			pcsSelfCharacter->m_ulSpecialStatus &= ~AGPDCHAR_SPECIAL_STATUS_DONT_MOVE;
	}

	if( pcsAgpdItemTemplate )
	{
		char	str[ 1024 ];
		sprintf( str , ClientStr().GetStr(STI_GACHA_DISPLAY_RESULT) , pcsAgpdItemTemplate->m_szName );
		m_pcsAgcmUIManager2->ActionMessageOKDialog( str );
	}

	m_pcsAgcmEventGacha->SetGachaWindowOpen( FALSE );

	return TRUE;
}

BOOL	AgcmUIEventGacha::CBSelfCharacterUpdatePosition(PVOID pData, PVOID pClass, PVOID pCustData)
{
	AgcmUIEventGacha	* pThis			= ( AgcmUIEventGacha * ) pClass;
	AgpdCharacter		* pcsCharacter	= (AgpdCharacter *)			pData;

	ASSERT( pThis );
	ASSERT( pcsCharacter );

	if (pThis->m_bOpenGachaUI)
	{
		FLOAT	fDistance = AUPOS_DISTANCE_XZ(pcsCharacter->m_stPos, pThis->m_posGachaOpenPosition);

		if ((INT32) fDistance < AGCMUIEVENTGACHA_CLOSE_UI_DISTANCE)
			return TRUE;

		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGachaClose);

		pThis->m_bOpenGachaUI	= FALSE;
	}

	return TRUE;
}
