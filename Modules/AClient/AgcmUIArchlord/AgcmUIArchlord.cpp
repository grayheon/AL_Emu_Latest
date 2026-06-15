#include "AgcmUIArchlord.h"
#include "AgpmArchlord.h"
#include "AgcmArchlord.h"
#include "AgcmUIManager2.h"
#include "AuStrTable.h"
#include "AgcChatManager.h"

AgcmUIArchlord::AgcmUIArchlord( void )
{
	SetModuleName( "AgcmUIArchlord" );
	EnableIdle( FALSE );

	m_pcsAgcmUIManager2 = NULL;
	m_pcsAgpmArchlord = NULL;
	m_pcsAgcmArchlord = NULL;
	m_lEventNoticeNewArchlord = 0;
}

AgcmUIArchlord::~AgcmUIArchlord( void )
{
}

BOOL AgcmUIArchlord::OnAddModule( void )
{
	m_pcsAgpmArchlord	= ( AgpmArchlord* )GetModule( "AgpmArchlord" );
	m_pcsAgcmArchlord	= ( AgcmArchlord* )GetModule( "AgcmArchlord" );
	m_pcsAgcmUIManager2 = ( AgcmUIManager2* )GetModule( "AgcmUIManager2" );

	if( !m_pcsAgpmArchlord || !m_pcsAgcmArchlord || !m_pcsAgcmUIManager2 ) return FALSE;

	if( !m_pcsAgpmArchlord->SetCallbackOperationSetArchlord(	CBSetArchlord,		this ) ) return FALSE;
	if( !m_pcsAgpmArchlord->SetCallbackOperationCancelArchlord( CBCancelArchlord,	this ) ) return FALSE;
	if( !m_pcsAgpmArchlord->SetCallbackOperationSetGuard(		CBSetGuard,			this ) ) return FALSE;
	if( !m_pcsAgpmArchlord->SetCallbackOperationCancelGuard(	CBCancelGuard,		this ) ) return FALSE;
	if( !m_pcsAgpmArchlord->SetCallbackOperationGuardInfo(		CBGuardInfo,		this ) ) return FALSE;
	if( !m_pcsAgpmArchlord->SetCallbackOperationCurrentStep(	CBCurrentStep,		this ) ) return FALSE;
	if( !m_pcsAgpmArchlord->SetCallbackOperationMessageId(		CBMessageId,		this ) ) return FALSE;
	if( !m_pcsAgpmArchlord->SetCallbackEventGrant(				CBGrant,			this ) ) return FALSE;
	if( !m_pcsAgpmArchlord->SetCallbackEventNoticeNewArchlord(  CBPlaySetNewArchlordSound, this)) return FALSE;
	
	if(!AddEvent())
		return FALSE;

	return TRUE;
}

BOOL AgcmUIArchlord::CBGuardInfo( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pClass || !pData )	return FALSE;

	AgcmUIArchlord* pThis = ( AgcmUIArchlord* )pClass;

	INT32 lCurrentGuard = *( INT32* )pData;
	INT32 lMaxGuard = *( INT32* )pCustData;

	ApString< 256 > text;
	text.Format( ClientStr().GetStr( STI_GUARD_INFO ), lCurrentGuard, lMaxGuard );

	return pThis->SetTextEffect( text.GetBuffer() );
}

BOOL AgcmUIArchlord::CBSetArchlord( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pClass || !pData )	return FALSE;

	AgcmUIArchlord* pThis = ( AgcmUIArchlord* )pClass;
	CHAR* szID = ( CHAR* )pData;

	ApString< 256 > text;
	text.Format( ClientStr().GetStr( STI_ARCHLORD_SET_GUARD ), szID );

	return pThis->SetTextEffect( text.GetBuffer() );
}

BOOL AgcmUIArchlord::CBCancelArchlord( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pClass || !pData )	return FALSE;

	AgcmUIArchlord* pThis = ( AgcmUIArchlord* )pClass;
	CHAR* szID = ( CHAR* )pData;

	ApString< 256 > text;
	text.Format( ClientStr().GetStr( STI_ARCHLORD_CANCEL_GUARD ), szID );

	return pThis->SetTextEffect( text.GetBuffer() );
}

BOOL AgcmUIArchlord::CBSetGuard( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pClass || !pData )	return FALSE;

	AgcmUIArchlord* pThis = ( AgcmUIArchlord* )pClass;
	CHAR cResult = *( CHAR* )pData;

	ApString< 256 > text;
	if( cResult )
	{
		text.SetText( ClientStr().GetStr( STI_GUARD_SET_SUCCESS ) );
	}
	else
	{
		text.SetText( ClientStr().GetStr( STI_GUARD_SET_FAIL ) );
	}

	return pThis->SetTextEffect( text.GetBuffer() );
}

BOOL AgcmUIArchlord::CBCancelGuard( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pClass || !pData )	return FALSE;

	AgcmUIArchlord* pThis = ( AgcmUIArchlord* )pClass;
	CHAR cResult = *( CHAR* )pData;

	ApString< 256 > text;
	if( cResult )
	{
		text.SetText( ClientStr().GetStr( STI_GUARD_CANCEL_SUCCESS ) );
	}
	else
	{
		text.SetText( ClientStr().GetStr( STI_GUARD_CANCEL_FAIL ) );
	}

	return pThis->SetTextEffect( text.GetBuffer() );
}

BOOL AgcmUIArchlord::CBCurrentStep( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pClass || !pData )	return FALSE;

	AgcmUIArchlord* pThis = ( AgcmUIArchlord* )pClass;
	AgpmArchlordStep eStep = *( AgpmArchlordStep* )pData;

	ApString< 256 > text;

	// TODO : 수정 필요!!
	switch( eStep )
	{
	case AGPMARCHLORD_STEP_DUNGEON :	text.SetText( ClientStr().GetStr( STI_START_DUNGEON		) );		break;
	case AGPMARCHLORD_STEP_SIEGEWAR :	text.SetText( ClientStr().GetStr( STI_START_LANSPHERE	) );		break;
	case AGPMARCHLORD_STEP_ARCHLORD :	text.SetText( ClientStr().GetStr( STI_ARCHLORD_BATTLE	) );		break;
	case AGPMARCHLORD_STEP_END:			text.SetText( ClientStr().GetStr( STI_ARCHLORD_END		) );		break;
	};

	pThis->SetTextEffect( text.GetBuffer() );
	AgcChatManager::OnAddSystemMessage( text.GetBuffer() );
	return TRUE;
}

BOOL AgcmUIArchlord::CBMessageId( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pClass || !pData )	return FALSE;

	AgcmUIArchlord* pThis = ( AgcmUIArchlord* )pClass;
	INT32 lMessageId = *( INT32* )pData;

	switch( ( AgpmArchlordMessageId )lMessageId )
	{
	case AGPMARCHLORD_MESSAGE_NOT_ENOUGH_INVENTORY :
		AgcChatManager::OnAddSystemMessage( ClientStr().GetStr( STI_NOT_ENOUGH_INVENTORY ) );
		break;
	case AGPMARCHLORD_MESSAGE_DONT_SET_GUARD :
		AgcChatManager::OnAddSystemMessage( ClientStr().GetStr( STI_DONT_SET_GUARD ) );
		break;
	case AGPMARCHLORD_MESSAGE_DONT_CANCEL_GUARD	:
		AgcChatManager::OnAddSystemMessage( ClientStr().GetStr( STI_DONT_CANCEL_GUARD ) );
		break;
	case AGPMARCHLORD_MESSAGE_NEED_ARCHON_ITEM :
		AgcChatManager::OnAddSystemMessage( ClientStr().GetStr( STI_NEED_ARCHON_ITEM ) );
		break;
	case AGPMARCHLORD_MESSAGE_GUILD_MASTER_ITEM :
		AgcChatManager::OnAddSystemMessage( ClientStr().GetStr( STI_ONLY_GUILD_MASTER_ITEM ) );
		break;
	default :
		ASSERT( !"message id error" );
		break;
	};

	return TRUE;
}

BOOL AgcmUIArchlord::CBGrant( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pCustData || !pClass )	return FALSE;

	AgcmUIArchlord* pThis = ( AgcmUIArchlord* )pClass;
	AgpdCharacter* pcsCharacter	= ( AgpdCharacter* )pData;

	// 머지.. 하는 일이 없네? ㅡ.ㅡ;;
	return TRUE;
}


BOOL AgcmUIArchlord::CBPlaySetNewArchlordSound( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if(!pClass) return FALSE;

	AgcmUIArchlord* pThis = ( AgcmUIArchlord* )pClass;

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventNoticeNewArchlord);

	return TRUE;	
}

BOOL AgcmUIArchlord::SetTextEffect( CHAR *pszString )
{
	if( !pszString || strlen( pszString ) <= 0 ) return FALSE;

	static INT32 lFont1		= 3;
	static INT32 lDuration	= 5000;
	static FLOAT fScale1	= 1.f;
	static UINT32 uColor	= 0xff00ff00;
	static INT32 lOffsetY1	= INT32( m_pcsAgcmUIManager2->m_csEffectWindow.h * 0.19f );

	m_pcsAgcmUIManager2->m_csEffectWindow.SetEffectSubText( "", lFont1, lOffsetY1, FALSE, 0x00, fScale1 );
	m_pcsAgcmUIManager2->m_csEffectWindow.SetEffectAboveText( "", lFont1, lOffsetY1, FALSE, 0x00, fScale1 );
	m_pcsAgcmUIManager2->m_csEffectWindow.SetEffectText( pszString,	lFont1,	lOffsetY1, FALSE, uColor, fScale1, lDuration, TRUE, TRUE );
	m_pcsAgcmUIManager2->m_csEffectWindow.SetEffectBG("RegionInfo.png", (FLOAT)lOffsetY1);
	return TRUE;
}

BOOL AgcmUIArchlord::AddEvent()
{
	m_lEventNoticeNewArchlord = m_pcsAgcmUIManager2->AddEvent("Notice_New_Archlord");
	if (m_lEventNoticeNewArchlord < 0)
		return FALSE;
	return TRUE;

}