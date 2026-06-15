#include "COptionFlagController.h"
#include "AgcChatManager.h"


#include "AgpmCharacter.h"
#include "AgcmCharacter.h"
#include "AgcmUIManager2.h"



COptionFlagController::COptionFlagController( void )
{
}

COptionFlagController::~COptionFlagController( void )
{
}

BOOL COptionFlagController::OnInitOptionFlagController( void )
{
	_AddOptionFlagMsg( AGPDCHAR_OPTION_REFUSE_TRADE,			"Option_Trade_Off", 	"Option_Trade_On", 		"Negative_Text_Color", "Positive_Text_Color" );
	_AddOptionFlagMsg( AGPDCHAR_OPTION_REFUSE_PARTY_IN,			"Option_Party_Off", 	"Option_Party_On", 		"Negative_Text_Color", "Positive_Text_Color" );
	_AddOptionFlagMsg( AGPDCHAR_OPTION_REFUSE_GUILD_IN,			"Option_Guild_Off", 	"Option_Guild_On", 		"Negative_Text_Color", "Positive_Text_Color" );
	_AddOptionFlagMsg( AGPDCHAR_OPTION_REFUSE_GUILD_BATTLE, 	"Option_GBattle_Off",	"Option_GBattle_On",	"Negative_Text_Color", "Positive_Text_Color" );
	_AddOptionFlagMsg( AGPDCHAR_OPTION_REFUSE_BATTLE,			"Option_Battle_Off",	"Option_Battle_On",		"Negative_Text_Color", "Positive_Text_Color" );
	_AddOptionFlagMsg( AGPDCHAR_OPTION_REFUSE_GUILD_RELATION,	"Option_GRelation_Off", "Option_GRelation_On",	"Negative_Text_Color", "Positive_Text_Color" );
	_AddOptionFlagMsg( AGPDCHAR_OPTION_REFUSE_BUDDY,			"Option_Buddy_Off",		"Option_Buddy_On",		"Negative_Text_Color", "Positive_Text_Color" );

	return TRUE;
}

BOOL COptionFlagController::OnChangeOptionFlag( int nFlag )
{
	AgcmCharacter* pcmCharacter = ( AgcmCharacter* )g_pEngine->GetModule( "AgcmCharacter" );
	if( !pcmCharacter ) return FALSE;

	AgpdCharacter* ppdSelfCharacter = pcmCharacter->GetSelfCharacter();
	if( !ppdSelfCharacter ) return FALSE;

	int nMyFlag = ppdSelfCharacter->m_lOptionFlag;

	_CheckOptionFlag( nMyFlag, nFlag, AGPDCHAR_OPTION_REFUSE_TRADE );
	_CheckOptionFlag( nMyFlag, nFlag, AGPDCHAR_OPTION_REFUSE_PARTY_IN );
	_CheckOptionFlag( nMyFlag, nFlag, AGPDCHAR_OPTION_REFUSE_GUILD_IN );
	_CheckOptionFlag( nMyFlag, nFlag, AGPDCHAR_OPTION_REFUSE_GUILD_BATTLE );
	_CheckOptionFlag( nMyFlag, nFlag, AGPDCHAR_OPTION_REFUSE_GUILD_RELATION );
	_CheckOptionFlag( nMyFlag, nFlag, AGPDCHAR_OPTION_REFUSE_BATTLE );
	_CheckOptionFlag( nMyFlag, nFlag, AGPDCHAR_OPTION_REFUSE_BUDDY );

	return TRUE;
}

BOOL COptionFlagController::OnRequestChangeOptionFlag( AgpdCharacterOptionFlag eFlag, BOOL bIsSet )
{
	AgpmCharacter* ppmCharacter = ( AgpmCharacter* )g_pEngine->GetModule( "AgpmCharacter" );
	AgcmCharacter* pcmCharacter = ( AgcmCharacter* )g_pEngine->GetModule( "AgcmCharacter" );
	if( !ppmCharacter || !pcmCharacter ) return FALSE;

	AgpdCharacter* ppdSelfCharacter = pcmCharacter->GetSelfCharacter();
	if( !ppdSelfCharacter ) return FALSE;

	int nFlag = bIsSet ? ppmCharacter->SetOptionFlag( ppdSelfCharacter, eFlag ) : ppmCharacter->UnsetOptionFlag( ppdSelfCharacter, eFlag );
	return pcmCharacter->SendPacketOptionFlag( ppdSelfCharacter, nFlag );
}

void COptionFlagController::_AddOptionFlagMsg( AgpdCharacterOptionFlag eFlag, char* pMsgON, char* pMsgOFF, char* pColorON, char* pColorOFF )
{
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return;

	stOptionFlagMsgEntry* pEntry = m_mapOptionFlagMsg.Get( eFlag );
	if( pEntry ) return;

	stOptionFlagMsgEntry NewEntry;

	if( pMsgON && strlen( pMsgON ) > 0 )
	{
		char* pMsg = pcmUIManager->GetUIMessage( pMsgON );
		NewEntry.m_MsgON.m_strMessage = pMsg ? pMsg : "";
	}

	if( pMsgOFF && strlen( pMsgOFF ) > 0 )
	{
		char* pMsg = pcmUIManager->GetUIMessage( pMsgOFF );
		NewEntry.m_MsgOFF.m_strMessage = pMsg ? pMsg : "";
	}

	if( pColorON && strlen( pColorON ) > 0 )
	{
		char* pColor = pcmUIManager->GetUIMessage( pColorON );
		NewEntry.m_MsgON.m_dwColor = pColor ? ( DWORD )atoi( pColor ) : 0xFFFFFFFF;
	}

	if( pColorOFF && strlen( pColorOFF ) > 0 )
	{
		char* pColor = pcmUIManager->GetUIMessage( pColorOFF );
		NewEntry.m_MsgOFF.m_dwColor = pColor ? ( DWORD )atoi( pColor ) : 0xFFFFFFFF;
	}

	m_mapOptionFlagMsg.Add( eFlag, NewEntry );
}

void COptionFlagController::_CheckOptionFlag( int nPrev, int nCurr, AgpdCharacterOptionFlag eFlag )
{
	if( ( nPrev & eFlag ) != ( nCurr & eFlag ) )
	{
		_SendMsgChangeOptionFlag( eFlag, nCurr & eFlag ? TRUE : FALSE );
	}
}

void COptionFlagController::_SendMsgChangeOptionFlag( AgpdCharacterOptionFlag eFlag, BOOL bIsOn )
{
	stOptionFlagMsgEntry* pEntry = m_mapOptionFlagMsg.Get( eFlag );
	if( !pEntry ) return;

	stMsgDataEntry* pData = bIsOn ? &pEntry->m_MsgON : &pEntry->m_MsgOFF;
	AgcChatManager::OnAddSystemMessage( ( char* )pData->m_strMessage.c_str(), pData->m_dwColor );
}
