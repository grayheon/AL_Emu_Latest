#include "AgcmUIChatting2.h"
#include "AgcmUIManager2.h"



AgcmUIChatting2::AgcmUIChatting2( void )
{
	SetModuleName( "AgcmUIChatting2" );
	EnableIdle( TRUE );

	m_dwTimeLastSendAreaMsg = 0;
}

AgcmUIChatting2::~AgcmUIChatting2( void )
{
}

BOOL AgcmUIChatting2::OnInit( void )
{
	OnInitEditChatting();
	OnRegisterUIEvent();

	InitMenu();

	return OnInitOptionFlagController();
}

BOOL AgcmUIChatting2::OnIdle( UINT32 ulClockCount )
{
	ChatIdle( ulClockCount );

	return TRUE;
}

BOOL AgcmUIChatting2::OnDestroy( void )
{
	OnDestroyEditChatting();
	return TRUE;
}

BOOL AgcmUIChatting2::OnAddModule( void )
{
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	AgcmCharacter* pcmCharacter = ( AgcmCharacter* )g_pEngine->GetModule( "AgcmCharacter" );
	if( !pcmUIManager || !pcmCharacter ) return FALSE;

	OnAddCustomControl();
	OnReadyBGMTitle();

	if( !pcmCharacter->SetCallbackSetSelfCharacter( CBSetSelfCharacter, this ) ) return FALSE;

	if( !pcmUIManager->AddFunction( this, "Chat_CheckNCloseInput", CBCheckNCloseInput, 5, "Edit Control", "Edit Control", "Edit Control", "Edit Control", "Edit Control" ) ) return FALSE;

	if( !pcmUIManager->AddFunction( this, "Chat_SendWhisper", CBSendWhisper, 0 ) ) return FALSE;

	pcmUIManager->AddFunction( this , "Chat_Active_Input" , CBActiveChatInput , 0 );

	if( !pcmUIManager->AddFunction( this, "SyncPosBtn_Click", CBClickSyncPosBtn, 0)) return FALSE;
	if( !pcmUIManager->AddFunction( this, "EscapeConfirmOk_Click", CBClickEscapeConfirmOk, 0)) return FALSE;

	return TRUE;
}

BOOL AgcmUIChatting2::OnSendMessage( AgpdChattingType eType, char* pMessage, int nLength, char* pTargetID , BOOL bParse )
{
	if( !pMessage || strlen( pMessage ) <= 0 || nLength <= 0 ) 
		return FALSE;

	AgcmCharacter*	pcmCharacter	=	static_cast< AgcmCharacter*>( GetModule( "AgcmCharacter" ) );
	AgpmChatting*	ppmChatting		=	static_cast< AgpmChatting* >( GetModule( "AgpmChatting" ) );
	if( !pcmCharacter || !ppmChatting ) 
		return FALSE;

	AgpdCharacter* ppdSelfCharacter = pcmCharacter->GetSelfCharacter();
	if( !ppdSelfCharacter ) return FALSE;

	// 메세지 길이 제한 체크
	short nPacketLength = 0;
	if( nLength > 128 )
	{
		nLength	= 128;
	}

	// 명령어인 경우 로컬에서 처리한다.

	if( bParse )
	{
		if( OnParseCommand( pMessage ) ) 
	return TRUE;
	}

	// 명령문이 아니라면 필터링을 거친다.
	OnFiltering( pMessage );

	void* pvPacket = ppmChatting->MakePacketChatting( ppdSelfCharacter->m_lID, eType, NULL, eType == AGPDCHATTING_TYPE_WHISPER ? pTargetID : NULL, pMessage, nLength, &nPacketLength );
	if( !pvPacket || nPacketLength <= 0 ) return FALSE;

	BOOL bSendResult = SendPacket(pvPacket, nPacketLength);
	ppmChatting->m_csPacket.FreePacket(pvPacket);

	// 귓속말의 경우 마지막 귓속말 대상을 저장해둔다.
	if( ( eType == AGPDCHATTING_TYPE_WHISPER || eType == AGPDCHATTING_TYPE_WHISPER2 ) && pTargetID )
	{
		SetAutoReplyWhisperTarget( pTargetID );
	}

	return bSendResult;
}

BOOL AgcmUIChatting2::IsEnableSendAreaChatting( DWORD dwTime )
{
	return m_dwTimeLastSendAreaMsg + 5000 < dwTime ? TRUE : FALSE;
}

BOOL AgcmUIChatting2::UpdateTimeLastSendAreaMsg( DWORD dwTime )
{
	m_dwTimeLastSendAreaMsg = dwTime;
	return TRUE;
}

BOOL AgcmUIChatting2::CBSetSelfCharacter( void* pData, void* pClass, void* pCustData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )pClass;
	if( !pcmUIChatting ) return FALSE;

	AgpdCharacter*		pcsCharacter	=	static_cast< AgpdCharacter* >(pData);
	if( !pcsCharacter)
		return FALSE;

	pcmUIChatting->OnCollectControls();

	CHAR	szChatFileFullPath[ MAX_PATH ];
	sprintf_s( szChatFileFullPath , MAX_PATH , "CUSTOMDATA\\%s.SYJ" , pcsCharacter->m_szID );

	pcmUIChatting->LoadBaseFile( "INI\\ChatSetting.SYJ" );
	pcmUIChatting->LoadChatFile( szChatFileFullPath );

	pcmUIChatting->ReCalcChatInputPosition();

	return TRUE;
}

BOOL AgcmUIChatting2::CBActiveChatInput( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase* pTarget , AgcdUIControl* pcsSourceControl )
{
	AgcmUIChatting2*	pThis	=	static_cast< AgcmUIChatting2* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->ActiveChatInput();

	pThis->OnReadyStatusEvent();

	return TRUE;
}


BOOL AgcmUIChatting2::CBCheckNCloseInput( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl )
{
	AgcdUIControl* pcdUIControl = ( AgcdUIControl* )pData1;
	if( pcdUIControl && pcdUIControl->m_pcsBase )
	{
		AcUIEdit* pEdit = ( AcUIEdit* )pcdUIControl->m_pcsBase;
		if( pEdit->GetActiveEdit() ) return FALSE;
	}

	pcdUIControl = ( AgcdUIControl* )pData2;
	if( pcdUIControl && pcdUIControl->m_pcsBase )
	{
		AcUIEdit* pEdit = ( AcUIEdit* )pcdUIControl->m_pcsBase;
		if( pEdit->GetActiveEdit() ) return FALSE;
	}

	pcdUIControl = ( AgcdUIControl* )pData3;
	if( pcdUIControl && pcdUIControl->m_pcsBase )
	{
		AcUIEdit* pEdit = ( AcUIEdit* )pcdUIControl->m_pcsBase;
		if( pEdit->GetActiveEdit() ) return FALSE;
	}

	pcdUIControl = ( AgcdUIControl* )pData4;
	if( pcdUIControl && pcdUIControl->m_pcsBase )
	{
		AcUIEdit* pEdit = ( AcUIEdit* )pcdUIControl->m_pcsBase;
		if( pEdit->GetActiveEdit() ) return FALSE;
	}

	pcdUIControl = ( AgcdUIControl* )pData5;
	if( pcdUIControl && pcdUIControl->m_pcsBase )
	{
		AcUIEdit* pEdit = ( AcUIEdit* )pcdUIControl->m_pcsBase;
		if( pEdit->GetActiveEdit() ) return FALSE;
	}

	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( pcdSourceControl && pcmUIManager )
	{
		pcmUIManager->CloseUI( pcdSourceControl->m_pcsParentUI );
				}

	return TRUE;
}
BOOL AgcmUIChatting2::CBClickSyncPosBtn( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pTarget, AgcdUIControl* pcsSourceControl )
{
	if(!pClass)
		return FALSE;

	AgcmUIChatting2			*pThis				= (AgcmUIChatting2 *)	pClass;
	CHAR szCommand[] = "/syncpos";

	pThis->OnSendMessage(AGPDCHATTING_TYPE_NORMAL, szCommand, sizeof(szCommand), NULL, FALSE);
	
	return TRUE;
}

BOOL AgcmUIChatting2::CBClickEscapeConfirmOk( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pTarget, AgcdUIControl* pcsSourceControl )
{
	if(!pClass)
		return FALSE;

	AgcmUIChatting2			*pThis				= (AgcmUIChatting2 *)	pClass;
	CHAR szCommand[] = "/escape";

	pThis->OnSendMessage(AGPDCHATTING_TYPE_NORMAL, szCommand, sizeof(szCommand), NULL, FALSE);

	return TRUE;
}


BOOL AgcmUIChatting2::CBSendWhisper( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pTarget, AgcdUIControl* pcsSourceControl )
{
	if (!pClass)
		return FALSE;

	AgcmUIChatting2	*pThis	= (AgcmUIChatting2 *)	pClass;
	AgcmUIManager2			*pUIManager			= (AgcmUIManager2 *)	g_pEngine->GetModule("AgcmUIManager2");

	if (pcsSourceControl &&
		pUIManager->GetControlUserData(pcsSourceControl))
	{
		AgcdUIUserData *	pcsUserData = pUIManager->GetControlUserData(pcsSourceControl);
		if (pcsUserData->m_eType == AGCDUI_USERDATA_TYPE_CHARACTER)
		{
			AgpdCharacter *	pcsTargetCharacter = (AgpdCharacter *)		pcsSourceControl->m_pstUserData->m_stUserData.m_pvData;
			if (pcsTargetCharacter)
			{
				pThis->OnReadyAutoReplyWhisper(pcsTargetCharacter->m_szID);
				return TRUE;
			}
		}
	}

	if (pData1 && ((ApBase *) pData1)->m_eType == APBASE_TYPE_UI_CONTROL)
	{
		AgcdUIControl *	pcsControl = (AgcdUIControl *) pData1;

		if (pcsControl->m_pcsBase->m_nType == AcUIBase::TYPE_EDIT)
		{
			const CHAR *	szID = ((AcUIEdit *) pcsControl->m_pcsBase)->GetText();
			if (szID && strlen(szID) > 0)
			{
				pThis->OnReadyAutoReplyWhisper((CHAR *) szID);
				return TRUE;
			}
		}
	}

	return TRUE;
}



VOID	AgcmUIChatting2::SetLastWhisper( CONST CHAR *pMessage )
{
	if( !pMessage )
		return;

	SetAutoReplyWhisperTarget( (CHAR*)pMessage );
}