#include "CChattingInputController.h"

#include "AgcmUIChatting2.h"
#include "AuStrTable.h"


CChattingInputController::CChattingInputController( void )
{
	m_strAutoReplyTargetName = "";
	m_nEventOnShow = -1;

	m_pcsStatusUserData		=	NULL;
}

CChattingInputController::~CChattingInputController( void )
{
}

BOOL CChattingInputController::OnInitEditChatting( void )
{
	m_EditChatting.m_pInputInfoTex = RwTextureRead( "Chat_InputInfo.png", NULL );
	
	if( m_EditChatting.m_pInputInfoTex )
		RwTextureSetAddressing( m_EditChatting.m_pInputInfoTex , rwTEXTUREADDRESSCLAMP);

	m_EditChatting.m_hKeyboardLayOut = GetKeyboardLayout( 0 );
	return TRUE;
}

BOOL CChattingInputController::OnDestroyEditChatting( void )
{
	if( m_EditChatting.m_pInputInfoTex )
	{
		RwTextureDestroy( m_EditChatting.m_pInputInfoTex );
		m_EditChatting.m_pInputInfoTex = NULL;
	}

	return TRUE;
}

BOOL CChattingInputController::OnAddCustomControl( void )
{
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return FALSE;

	m_nEventOnShow = pcmUIManager->AddEvent( "Chat_InputNormalUIOpen" );
	if( m_nEventOnShow < 0 ) return FALSE;

	pcmUIManager->AddCustomControl( "ChattingEdit", &m_EditChatting );

	m_EditChatting.SetCallbackRefresh( this , CallBack_StatusRefresh );

	if( !pcmUIManager->AddFunction( this, "Chat_InputStart", CallBack_OnInputStart, 1, "Input Control" ) ) return FALSE;
	if( !pcmUIManager->AddFunction( this, "Chat_InputEndMessage", CallBack_OnInputEnd, 0 ) ) return FALSE;
	if( !pcmUIManager->AddFunction( this, "Chat_InputEndPrivateMessage", CallBack_OnInputEndPrivate, 1, "TargetIDControl" ) )	return FALSE;
	if( !pcmUIManager->AddFunction( this, "Chat_ReplyImmediate", CallBack_OnAutoReplyWhisper, 0 ) )	return FALSE;

	pcmUIManager->AddFunction( this , "Chat_InputStatusClick" , CallBack_OnInputStatusClick , 0 );

	pcmUIManager->AddDisplay( this , "Chat_DisplayStatus" , 0 , Callback_DisplayStatus , AGCDUI_USERDATA_TYPE_INT32 );
	m_pcsStatusUserData	=	pcmUIManager->AddUserData( "Chat_InputUserData" , &m_DummyData , sizeof(INT32) , 1 , AGCDUI_USERDATA_TYPE_INT32 );

	return TRUE;
}

VOID	CChattingInputController::InitMenu( VOID )
{
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return;

	m_csInputMenu.Initialize( E_MENU_DRAW_TEXTURE );

	// 상태 변경 팝업메뉴
	pcmUIManager->AddWindow( &m_csInputMenu );
	m_csInputMenu.ShowWindow( FALSE );

	// 메뉴 등록
	stMenuEvent		MenuEvent;
	MenuEvent.m_bEnable		=	TRUE;
	MenuEvent.m_eEventType	=	E_MENU_EVENT_FUNCTION;
	MenuEvent.m_pClass		=	this;
	MenuEvent.m_pvData		=	CBMenuStatusChange;
	MenuEvent.m_pData1		=	(PVOID)CHAT_INPUT_NORMAL;
	MenuEvent.m_strMenuName	=	ClientStr().GetStr( STI_INPUT_STATUS_NORMAL );
	m_csInputMenu.AddMenuEvent( MenuEvent );

	MenuEvent.m_pData1		=	(PVOID)CHAT_INPUT_PARTY;
	MenuEvent.m_strMenuName	=	ClientStr().GetStr( STI_INPUT_STATUS_PARTY );
	m_csInputMenu.AddMenuEvent( MenuEvent );

	MenuEvent.m_pData1		=	(PVOID)CHAT_INPUT_GUILD;
	MenuEvent.m_strMenuName	=	ClientStr().GetStr( STI_INPUT_STATUS_GUILD );
	m_csInputMenu.AddMenuEvent( MenuEvent );

	MenuEvent.m_pData1		=	(PVOID)CHAT_INPUT_SHOUT;
	MenuEvent.m_strMenuName	=	ClientStr().GetStr( STI_INPUT_STATUS_SHOUT );
	m_csInputMenu.AddMenuEvent( MenuEvent );

	MenuEvent.m_pData1		=	(PVOID)CHAT_INPUT_REPLY;
	MenuEvent.m_strMenuName	=	ClientStr().GetStr( STI_INPUT_STATUS_REPLY );
	m_csInputMenu.AddMenuEvent( MenuEvent );

	m_strStatus				=	ClientStr().GetStr( STI_INPUT_STATUS_NORMAL );
	m_dwCurrentStatusColor	=	0xffffffff;
}

BOOL CChattingInputController::OnSetChattingMessage( char* pMsg )
{
	if( !pMsg )
		return FALSE;

	m_EditChatting.SetText( !pMsg || strlen( pMsg ) <= 0 ? "" : pMsg );
	m_EditChatting.UpdateEvent();

	//return OnShowEditChatting();

	return TRUE;
}

BOOL CChattingInputController::OnAddChattingMessage( CHAR* pMsg , DWORD dwColor )
{
	m_EditChatting.AddStringItem( pMsg , dwColor );

	string		strChatMessage	=	m_EditChatting.GetText();
	strChatMessage += pMsg;


	if( strChatMessage.length() < m_EditChatting.m_ulTextMaxLength )
	{
		m_EditChatting.SetText( (CHAR*)strChatMessage.c_str() );
		m_EditChatting.UpdateEvent();
		return OnShowEditChatting();
	}

	return TRUE;
}

VOID	CChattingInputController::ReCalcChatInputPosition( void )
{

	// 채팅 입력창을 다시 맞춰준다.
	AgcmUIManager2*	pcmUIManager	=	static_cast< AgcmUIManager2* >(g_pEngine->GetModule( "AgcmUIManager2" ));
	if( pcmUIManager )
	{
		AgcdUI*				pInputUI	=	pcmUIManager->GetUI( "Chat_Input_Normal" );
// 		AgcdUI*				pBarUI		=	pcmUIManager->GetUI( "UI_BarExp" );

		if( pInputUI /*&& pBarUI*/ )
		{
// 			AgcdUIControl*	pControl	=	pcmUIManager->GetControl( pBarUI , "CONTROL_BG" );
// 			if( pControl && pControl->m_pcsBase  )
// 			{
// 				INT	nX	=	0;
// 				INT	nY	=	0;

// 				pControl->m_pcsBase->ClientToScreen( &nX , &nY );
				pInputUI->m_pcsUIWindow->MoveWindow(  0 , pcmUIManager->m_lWindowHeight-115 );
// 			}
		}
	}

}

BOOL CChattingInputController::OnShowEditChatting( void )
{
	m_EditChatting.UpdateCurrentStatus();

	ActiveChatInput();

	return TRUE;
}

VOID CChattingInputController::ActiveChatInput( VOID )
{
	m_EditChatting.SetMeActiveEdit();
}

BOOL CChattingInputController::OnReadyStatusEvent( VOID )
{

	if( m_EditChatting.GetCurrentInputType() == CHAT_INPUT_WHISPER ||
		m_EditChatting.GetCurrentInputType() == CHAT_INPUT_REPLY )
	{
		return OnReadyAutoReplyWhisper();
	}

	return FALSE;
}

BOOL CChattingInputController::OnReadyAutoReplyWhisper( void )
{
	return OnReadyAutoReplyWhisper( ( char* )m_strAutoReplyTargetName.c_str() );
}

BOOL CChattingInputController::OnReadyAutoReplyWhisper( char* pTargetName )
{
	if( !pTargetName || strlen( pTargetName ) <= 0 ) return FALSE;

	char* pCmdString = ClientStr().GetStr( STI_CHAT_WHISPER_LOWER );
	if( !pCmdString || strlen( pCmdString ) <= 0 ) 
		return FALSE;

	char strBuffer[ 256 ];
	ZeroMemory( strBuffer , 256 );
	sprintf_s( strBuffer, sizeof( char ) * 256, "%s %s ", pCmdString, pTargetName );

	return OnSetChattingMessage( strBuffer );
}

BOOL CChattingInputController::OnReadyItemRegister( CONST CHAR* pItemName , INT lID )
{
	if( !pItemName || !lID )
		return FALSE;

	CHAR	strBuffer[ MAX_PATH ];
	ZeroMemory( strBuffer , MAX_PATH );

	// 아이템 포맷
	// <ITEM::ID::NAME>
	sprintf_s( strBuffer , MAX_PATH , "<ITEM::%d::%s>" , lID , pItemName );

	return OnAddChattingMessage( strBuffer );
}

BOOL CChattingInputController::OnReadyMapPosRegister( CONST CHAR* pMapPosName , INT nID , INT nX , INT nY )
{
	if( !pMapPosName )
		return FALSE;

	const INT32 strSize = 306;	//mappos(256) + ID,x,y좌표(10*3) + 명령어(20)
	CHAR	strBuffer[ strSize ];
	ZeroMemory( strBuffer , strSize );

	// 맵포즈 포맷
	// <MAPPOS::ID::X::Y::NAME>
	sprintf_s( strBuffer , strSize , "<MAPPOS::%d::%d::%d::%s>" , nID , nX , nY , pMapPosName );

	return OnAddChattingMessage( strBuffer );
}

BOOL CChattingInputController::CallBack_OnInputStart( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl )
{
	AgcdUIControl* pcdInputControl = ( AgcdUIControl* )pData1;
	if( !pcdInputControl ) return FALSE;
	if( pcdInputControl->m_lType != AcUIBase::TYPE_EDIT ) return FALSE;

	AcUIEdit* pEdit = ( AcUIEdit* )pcdInputControl->m_pcsBase;
	if( !pEdit ) return FALSE;

	pEdit->SetMeActiveEdit();
	return TRUE;
}

BOOL CChattingInputController::CallBack_OnInputEnd( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl )
{
	if( !pcdSourceControl )
		return FALSE;

	CChattingInputController*	pController	=	static_cast< CChattingInputController* >(pClass);
	if( !pController )
		return FALSE;

	AcUIEdit* pEdit = ( AcUIEdit* )pcdSourceControl->m_pcsBase;
	if( !pEdit ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	const char* pMsg = pEdit->GetText();
	if( !pMsg || strlen( pMsg ) <= 0 ) 
		return FALSE;

	CHAR	szMessage[ MAX_PATH*2 ];

	switch( pController->m_EditChatting.GetCurrentInputType() )
	{
	case CHAT_INPUT_WHISPER:
	case CHAT_INPUT_REPLY:
	case CHAT_INPUT_NORMAL:
		sprintf_s( szMessage , MAX_PATH*2 , "%s" , pMsg );
		break;

	case CHAT_INPUT_PARTY:
		sprintf_s( szMessage , MAX_PATH*2 , "/p %s" , pMsg );
		break;
	case CHAT_INPUT_GUILD:
		sprintf_s( szMessage , MAX_PATH*2 , "/g %s" , pMsg );
		break;
	case CHAT_INPUT_SHOUT:
		sprintf_s( szMessage , MAX_PATH*2 , "/s %s" , pMsg );
		break;

	default:
		sprintf_s( szMessage , MAX_PATH*2 , "%s" , pMsg );
		break;
	}
	

	if( pcmUIChatting->OnSendMessage( AGPDCHATTING_TYPE_NORMAL, ( char* )szMessage, strlen( szMessage ), NULL ) )
	{
		pEdit->ClearText();
	}
	
	pEdit->SetMeActiveEdit();
	return TRUE;
}

BOOL CChattingInputController::CallBack_OnInputEndPrivate( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl )
{
	AgcdUIControl* pcdControl = ( AgcdUIControl* )pData1;
	if( !pcdControl || !pcdSourceControl ) return FALSE;
	if( pcdSourceControl->m_lType != AcUIBase::TYPE_EDIT || pcdControl->m_lType != AcUIBase::TYPE_EDIT ) return FALSE;

	AcUIEdit* pEditID = ( AcUIEdit* )pcdControl->m_pcsBase;
	AcUIEdit* pEditMsg = ( AcUIEdit* )pcdSourceControl->m_pcsBase;
	if( !pEditID || !pEditMsg ) return FALSE;

	const char* pTargetName = pEditID->GetText();
	const char* pMsg = pEditMsg->GetText();
	if( !pMsg || strlen( pMsg ) <= 0 ) return FALSE;
	if( !pTargetName || strlen( pTargetName ) <= 0 )
	{
		return CallBack_OnInputEnd( pClass, pData1, pData2, pData3, pData4, pData5, pTarget, pcdSourceControl );
	}

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	if( pcmUIChatting->OnSendMessage( AGPDCHATTING_TYPE_WHISPER, ( char* )pMsg, strlen( pMsg ), ( char* )pTargetName ) )
	{
		pEditMsg->ClearText();
		pEditMsg->SetMeActiveEdit();
	}

	return TRUE;
}

BOOL CChattingInputController::CallBack_OnAutoReplyWhisper( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl )
{
	CChattingInputController* pController = ( CChattingInputController* )pClass;
	if( !pController ) return FALSE;

	pController->OnReadyAutoReplyWhisper();
	return TRUE;
}

BOOL	CChattingInputController::CallBack_OnInputStatusClick( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase* pTarget , AgcdUIControl* pcdSourceControl )
{
	CChattingInputController*	pController	=	static_cast< CChattingInputController* >(pClass);
	if( !pController )
		return FALSE;

	INT		nX	=	0;
	INT		nY	=	0;

	pcdSourceControl->m_pcsBase->ClientToScreen( &nX , &nY );

	pController->OpenInputStatusMenu( nX , nY );

	return TRUE;
}

BOOL	CChattingInputController::Callback_DisplayStatus( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl* pcsSourceControl )
{
	CChattingInputController*	pController	=	static_cast< CChattingInputController* >(pClass);
	if( !pController )
		return FALSE;

	pcsSourceControl->m_pcsBase->SetStaticStringExt( (CHAR*)pController->m_strStatus.c_str() , 1 , 0 , pController->m_dwCurrentStatusColor , true , ACUIBASE_HALIGN_CENTER , ACUIBASE_VALIGN_CENTER );

	sprintf( szDisplay , "%s" , pController->m_strStatus.c_str() );

	return TRUE;
}

BOOL	CChattingInputController::CBMenuStatusChange( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu )
{
	CChattingInputController*	pController	=	static_cast< CChattingInputController* >(pClass);
	if( !pController )
		return FALSE;

	ChattingEditInputType	eInputType	=	(ChattingEditInputType)((INT)pData1);

	pController->m_EditChatting.ClearText();
	pController->ChangeInputStatus( eInputType );
	pController->ActiveChatInput();

	return TRUE;
}

BOOL	CChattingInputController::CallBack_StatusRefresh( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 )
{
	CChattingInputController*	pController	=	static_cast< CChattingInputController* >(pClass);
	if( !pController )
		return FALSE;

	ChattingEditInputType	eInputType	=	(ChattingEditInputType)((INT)pData1);

	pController->ChangeInputStatus( eInputType , FALSE );

	return TRUE;
}



VOID	CChattingInputController::ChangeInputStatus( ChattingEditInputType eInputType , BOOL bRefresh )
{

	INT		nIndex		=	0;
	DWORD	dwFontColor	=	0xffffffff;
	switch( eInputType )
	{
	case CHAT_INPUT_NORMAL:
		nIndex		=	STI_INPUT_STATUS_NORMAL;
		dwFontColor =	AgcChatManager::GetTextColor( "NORMAL_CHAT" );
		break;
	case CHAT_INPUT_PARTY:
		nIndex		=	STI_INPUT_STATUS_PARTY;
		dwFontColor =	AgcChatManager::GetTextColor( "PARTY_CHAT" );
		break;
	case CHAT_INPUT_GUILD:
		nIndex		=	STI_INPUT_STATUS_GUILD;
		dwFontColor =	AgcChatManager::GetTextColor( "GUILD_CHAT" );
		break;
	case CHAT_INPUT_SHOUT:
		nIndex		=	STI_INPUT_STATUS_SHOUT;
		dwFontColor =	AgcChatManager::GetTextColor( "WORD_BALLON" );
		break;

	case CHAT_INPUT_WHISPER:
		{
			nIndex		=	STI_INPUT_STATUS_REPLY;
			dwFontColor	=	AgcChatManager::GetTextColor( "WHISPER_CHAT" );
		}
		break;

	case CHAT_INPUT_REPLY:
		{
			nIndex		=	STI_INPUT_STATUS_REPLY;
			dwFontColor	=	AgcChatManager::GetTextColor( "WHISPER_CHAT" );
			OnReadyAutoReplyWhisper();
		}
		break;

	default:
		return;
	}

	m_dwCurrentStatusColor	=	dwFontColor;
	m_strStatus				=	ClientStr().GetStr(nIndex);

	if( bRefresh )
		m_EditChatting.SetCurrentInputType( eInputType );

	AgcmUIManager2*	pcmUIManager	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( pcmUIManager )
	{
		if( m_pcsStatusUserData )
			pcmUIManager->SetUserDataRefresh( m_pcsStatusUserData );
	}
}


VOID	CChattingInputController::OpenInputStatusMenu( INT nX , INT nY )
{

	m_csInputMenu.SetFocusedWindow();
	m_csInputMenu.MoveWindow( nX , 200 , 100 , 200);
	m_csInputMenu.SetEnableLastYPosition( TRUE );
	m_csInputMenu.SetLastYPosition( (FLOAT)nY );
	m_csInputMenu.ShowWindow( TRUE );
}


string	CChattingInputController::GetToID( CONST CHAR* pMessage )
{
	string		strReturn;

	if( pMessage )
	{
		CHAR*		pNext	=	NULL;

		if( strtok_s( (CHAR*)pMessage , " " , &pNext ) )
		{
			strReturn	=	strtok_s( NULL , " " , &pNext );
		}
	}

	return strReturn;
}