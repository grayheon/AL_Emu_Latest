#include "AgcmUIChatting2.h"
#include "CChattingCommander.h"

#include "AuXmlParser.h"
#include "AgcmTextBoardMng.h"
#include "AgcmCamera2.h"
#include "AgcuCamMode.h"
#include "AgcmTitle.h"
#include "AgcmUIGuild.h"
#include "AgpmAreaChatting.h"
#include "AgcmUIChannel.h"
#include "AgcmUIPartyOption.h"

#include "AgcmUINotice.h"

#include "AgpmQuest.h"
#include "AgcmUIQuestJournal.h"
#include "AgcmUIQuest2.h"
#include "AgpmEventNPCDialog.h"
#include "AgcmUIGuideMessage.h"

#if (defined(_BIN_EXEC_)||defined(USE_MFC))
BOOL	CChatCmd::m_bIsEncrypted = FALSE;
#else
BOOL	CChatCmd::m_bIsEncrypted = TRUE;
#endif

CChatCmd::CChatCmd( void )
{
}

CChatCmd::~CChatCmd( void )
{
}

BOOL CChatCmd::OnRegisterCmdCallBack( void )
{
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return FALSE;

	// 채팅창 테스트용 명령어
	_AddCmdCallBack( "CallBack_OnTestEditChatting", CallBack_OnTestEditChatting );

	// 금칙어 수동추가 명령어
	_AddCmdCallBack( "CallBack_OnAddTextFilter", CallBack_OnAddTextFilter );

	// 소셜 애니메이션 명령어
	_AddCmdCallBack( "CallBack_OnSocialAnimation", CallBack_OnSocialAnimation );

	// 퀘스트 리로드
	_AddCmdCallBack( "CallBack_OnReloadQuest" , CallBack_OnReloadQuest );

	// 플래그 이모티콘 명령어
	_AddCmdCallBack( "CallBack_OnFlagImoticon", CallBack_OnFlagImoticon );

	// 지평선 그리기 명령어
	_AddCmdCallBack( "CallBack_OnDrawOcclusion", CallBack_OnDrawOcclusion );

	// 타이틀정보 다시읽기 명령어
	_AddCmdCallBack( "CallBack_OnReloadTitle", CallBack_OnReloadTitle );

	// 카메라 모드변경 명령어
	_AddCmdCallBack( "CallBack_OnChangeCameraMode", CallBack_OnChangeCameraMode );

	// 모듈 리포팅 명령어
	_AddCmdCallBack( "CallBack_OnReportModule", CallBack_OnReportModule );

	// 메모리 검사 명령어
	_AddCmdCallBack( "CallBack_OnCheckMemory", CallBack_OnCheckMemory );

	// 길드 관련 명령어
	_AddCmdCallBack( "CallBack_OnGuildBattleRequest", CallBack_OnGuildBattleRequest );
	_AddCmdCallBack( "CallBack_OnGuildBattleCancel", CallBack_OnGuildBattleCancel );
	_AddCmdCallBack( "CallBack_OnGuildBattleSurrender", CallBack_OnGuildBattleSurrender );
	_AddCmdCallBack( "CallBack_OnGuildLeaveOut", CallBack_OnGuildLeaveOut );

	// 퍼스펙티브 값 수정
	_AddCmdCallBack( "CallBack_OnChangePerspective", CallBack_OnChangePerspective );

	// 와이드 모드로 변경
	_AddCmdCallBack( "CallBack_OnChangeWideScreen", CallBack_OnChangeWideScreen );

	// 사운드 효과 변경
	_AddCmdCallBack( "CallBack_OnChangeSoundEffect", CallBack_OnChangeSoundEffect );

	// 루아 스크립트 실행
	_AddCmdCallBack( "CallBack_OnScriptLua", CallBack_OnScriptLua );

	// FPS 보기
	_AddCmdCallBack( "CallBack_OnViewFPS", CallBack_OnViewFPS );

	// 3D Sound Test.. 테스트용이었던것 같으니 이젠 없어도 될듯하지만.. 일단 냅둔다..
	_AddCmdCallBack( "CallBack_OnTest3DSound", CallBack_OnTest3DSound );

	// 게임 내 시간 변경
	_AddCmdCallBack( "CallBack_OnChangeTime", CallBack_OnChangeTime );

	// MatD3DFX 켜기 / 끄기
	_AddCmdCallBack( "CallBack_OnToggleMatD3DFX", CallBack_OnToggleMatD3DFX );

	// 파티 초대
	_AddCmdCallBack( "CallBack_OnInviteParty", CallBack_OnInviteParty );

	// 외치기
	_AddCmdCallBack( "CallBack_OnSendMsgShout", CallBack_OnSendMsgShout );

	// 채널 들어가기 / 나가기
	_AddCmdCallBack( "CallBack_OnSendMsgJoin", CallBack_OnSendMsgJoin );
	_AddCmdCallBack( "CallBack_OnSendMsgLeave", CallBack_OnSendMsgLeave );	

	// 좌표이동
	_AddCmdCallBack( "CallBack_OnMoveToPosition", CallBack_OnMoveToPosition );	

	// 공지사항 매크로
	_AddCmdCallBack( "CallBack_OnNoticeMessage", CallBack_OnNoticeMessage );	
	_AddCmdCallBack( "CallBack_OnNoticeMessageTest", CallBack_OnNoticeMessageTest );	

	// 캐릭터 OptionFlag 조작 명령어
	_AddCmdCallBack( "CallBack_OnRejectTradeOn", CallBack_OnRejectTradeOn );	
	_AddCmdCallBack( "CallBack_OnRejectTradeOff", CallBack_OnRejectTradeOff );	
	_AddCmdCallBack( "CallBack_OnRejectPartyOn", CallBack_OnRejectPartyOn );	
	_AddCmdCallBack( "CallBack_OnRejectPartyOff", CallBack_OnRejectPartyOff );	
	_AddCmdCallBack( "CallBack_OnRejectGuildOn", CallBack_OnRejectGuildOn );	
	_AddCmdCallBack( "CallBack_OnRejectGuildOff", CallBack_OnRejectGuildOff );	
	_AddCmdCallBack( "CallBack_OnRejectGuildBattleOn", CallBack_OnRejectGuildBattleOn );	
	_AddCmdCallBack( "CallBack_OnRejectGuildBattleOff", CallBack_OnRejectGuildBattleOff );	
	_AddCmdCallBack( "CallBack_OnRejectGuildRelationOn", CallBack_OnRejectGuildRelationOn );	
	_AddCmdCallBack( "CallBack_OnRejectGuildRelationOff", CallBack_OnRejectGuildRelationOff );	
	_AddCmdCallBack( "CallBack_OnRejectBattleOn", CallBack_OnRejectBattleOn );	
	_AddCmdCallBack( "CallBack_OnRejectBattleOff", CallBack_OnRejectBattleOff );	
	_AddCmdCallBack( "CallBack_OnRejectBuddyOn", CallBack_OnRejectBuddyOn );	
	_AddCmdCallBack( "CallBack_OnRejectBuddyOff", CallBack_OnRejectBuddyOff );	

	// 카메라 세팅 리로드 명령어 (/reloadcam)
	_AddCmdCallBack( "CallBack_OnReloadCamSetting", CallBack_OnReloadCamSetting );

	// NPC Dialog 텍스트 리로드 (/reloadnpcdialog)
// 	_AddCmdCallBack( "CallBack_OnReloadNPCDialog", CallBack_OnReloadNPCDialog );

	// guid message 리로드 (/reloadguide)
	_AddCmdCallBack( "CallBack_OnReloadGuide", CallBack_OnReloadGuide );

	// worldmap.ini 리로드(/rwm)
	_AddCmdCallBack( "CallBack_OnReloadWorldmap", CallBack_OnReloadWorldmap);

	return TRUE;
}

BOOL CChatCmd::OnLoadChatCmd( char* pFileName, BOOL bIsEncrypt )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return FALSE;

	AuXmlParser Doc;
	if( !Doc.LoadXMLFile( pFileName ) )
		return FALSE;

	TiXmlNode* pRootNode = Doc.FirstChild( "ChattingCommands" );
	if( !pRootNode ) return FALSE;

	TiXmlNode* pNode = pRootNode->FirstChild( "ChatCmd" );
	while( pNode )
	{
		_AddChatCmd( pNode );
		pNode = pNode->NextSibling();
	}

	Doc.Clear();	
	return TRUE;
}

BOOL CChatCmd::OnParseCommand( char* pCmdString )
{
	if( !pCmdString || strlen( pCmdString ) <= 0 )
		return FALSE;

	if( pCmdString[ 0 ] != '/' )
		return FALSE;

	char strCmd[ 64 ] = { 0, };
	char strParam[ 256 ] = { 0, };

	char* pSeperator = strstr( pCmdString, " " );
	if( NULL == pSeperator )
		strcpy_s( strCmd, sizeof( char ) * 64, &pCmdString[ 1 ] );
	else
	{
		memcpy_s( strCmd, sizeof( char ) * 64, &pCmdString[ 1 ], pSeperator - &pCmdString[ 1 ] );
		strcpy_s( strParam, sizeof( char ) * 256, pSeperator + 1 );
	}

	if( strlen( strCmd ) <= 0 ) 
		return FALSE;

	stCmdDataEntry* pCmdData = m_mapCmd.Get( strCmd );
	if( !pCmdData ) 
		return FALSE;

	if( !pCmdData->m_fnCallBack ) 
		return FALSE;

	_ParseParamValue( pCmdData, strParam );
	_RunCmdCallBack( pCmdData );

	return TRUE;
}

BOOL CChatCmd::_AddChatCmd( void* pXmlNode )
{
	TiXmlNode* pNode = ( TiXmlNode* )pXmlNode;
	if( !pNode ) 
		return FALSE;

	char* pCmdString = ( char* )pNode->ToElement()->Attribute( "CmdString" );
	if( !pCmdString || strlen( pCmdString ) <= 0 ) 
		return FALSE;

	char* pCmdFunc = ( char* )pNode->ToElement()->Attribute( "CmdFunc" );
	if( !pCmdFunc || strlen( pCmdFunc ) <= 0 ) 
		return FALSE;

	stCmdDataEntry* pEntry = m_mapCmd.Get( pCmdString );
	if( pEntry ) 
		return FALSE;

	stCmdDataEntry NewEntry;

	NewEntry.m_strCmd = pCmdString;
	NewEntry.m_fnCallBack = _GetCmdCallBack( pCmdFunc );

	TiXmlNode* pNodeParam = pNode->FirstChild( "Param" );
	while( pNodeParam )
	{
		stCmdParamEntry NewParam;

		NewParam.m_nIndex = NewEntry.m_mapParam.GetSize();
		NewParam.m_eType = _ParseParamType( ( char* )pNodeParam->ToElement()->Attribute( "ParamType" ) );

		char* pDefaultValue = ( char* )pNodeParam->ToElement()->Attribute( "DefaultValue" );
		if( pDefaultValue && strlen( pDefaultValue ) > 0 )
		{
			switch( NewParam.m_eType )
			{
			case ParamType_Integer :
				NewParam.m_nParam = atoi( pDefaultValue );
				break;
			case ParamType_Integer64 :
				NewParam.m_n64Param = _atoi64( pDefaultValue );
				break;
			case ParamType_Float :
				NewParam.m_fParam = ( float )atof( pDefaultValue );
				break;
			case ParamType_Boolean :
				NewParam.m_bParam = strcmp( pDefaultValue, "true" ) == 0 || strcmp( pDefaultValue, "TRUE" ) == 0 ? TRUE : FALSE;
				break;
			case ParamType_String :
				NewParam.m_strParam = pDefaultValue;
				break;
			}
		}

		NewEntry.m_mapParam.Add( NewParam.m_nIndex, NewParam );
		pNodeParam = pNodeParam->NextSibling();
	}

	m_mapCmd.Add( pCmdString, NewEntry );
	return TRUE;
}

void CChatCmd::_AddCmdCallBack( char* pCallBackName, fnCallBack_ChatCmd fnCallBack )
{
	if( !pCallBackName || strlen( pCallBackName ) <= 0 ) return;

	stCmdCallBackEntry* pEntry = m_mapCallBack.Get( pCallBackName );
	if( pEntry ) return;

	stCmdCallBackEntry NewEntry;

	NewEntry.m_strCallBackName = pCallBackName;
	NewEntry.m_fnCallBack = fnCallBack;

	m_mapCallBack.Add( pCallBackName, NewEntry );
}

fnCallBack_ChatCmd CChatCmd::_GetCmdCallBack( char* pCallBackName )
{
	stCmdCallBackEntry* pEntry = m_mapCallBack.Get( pCallBackName );
	if( !pEntry ) 
		return NULL;

	return pEntry->m_fnCallBack;
}

eCmdParamType CChatCmd::_ParseParamType( char* pTypeName )
{
	if( !pTypeName || strlen( pTypeName ) <= 0 ) 
		return ParamType_UnKnown;

	if( strcmp( pTypeName, "Integer64" ) == 0 ) 
		return ParamType_Integer64;

	if( strcmp( pTypeName, "Integer" ) == 0 ) 
		return ParamType_Integer;

	if( strcmp( pTypeName, "Float" ) == 0 ) 
		return ParamType_Float;

	if( strcmp( pTypeName, "Boolean" ) == 0 ) 
		return ParamType_Boolean;

	if( strcmp( pTypeName, "String" ) == 0 ) 
		return ParamType_String;

	return ParamType_UnKnown;
}

void CChatCmd::_ParseParamValue( stCmdDataEntry* pCmdData, char* pParamString )
{
	if( !pCmdData || !pParamString || strlen( pParamString ) <= 0 ) return;

	char strParam[ 64 ] = { 0, };
	int nCount = 0;
	int nCopyCount = 0;
	int nParamCount = pCmdData->m_mapParam.GetSize();
	int nParsedCount = 0;

	while( nParsedCount < nParamCount )
	{
		if( pParamString[ nCount ] != ' ' && pParamString[ nCount ] != '\0' && pParamString[ nCount ] != '\n' )
		{
			strParam[ nCopyCount ] = pParamString[ nCount ];
			nCopyCount++;
		}
		else
		{
			stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( nParsedCount );
			if( pParam )
			{
				switch( pParam->m_eType )
				{
				case ParamType_Integer :
					pParam->m_nParam = atoi( strParam );
					break;
				case ParamType_Integer64 :
					pParam->m_nParam = atoi( strParam );
					break;
				case ParamType_Float :
					pParam->m_fParam = ( float )atof( strParam );
					break;
				case ParamType_Boolean :
					pParam->m_bParam = strcmp( strParam, "true" ) == 0 || strcmp( strParam, "TRUE" ) == 0 ? TRUE : FALSE;
					break;
				case ParamType_String :
					pParam->m_strParam = strParam;
					break;
				}

				nParsedCount++;
			}

			memset( strParam, 0, sizeof( char ) * 64 );
			nCopyCount = 0;
		}

		nCount++;
	}
}

BOOL CChatCmd::_RunCmdCallBack( stCmdDataEntry* pCmdData )
{
	if( !pCmdData || !pCmdData->m_fnCallBack ) return FALSE;
	return pCmdData->m_fnCallBack( pCmdData );
}

BOOL CChatCmd::CallBack_OnTestEditChatting( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam1 = pCmdData->m_mapParam.GetByIndex( 0 );
	stCmdParamEntry* pParam2 = pCmdData->m_mapParam.GetByIndex( 1 );
	stCmdParamEntry* pParam3 = pCmdData->m_mapParam.GetByIndex( 2 );
	if( !pParam1 || !pParam2 || !pParam3 ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	DWORD dwColor = 0xFFFFFFFF;
	if( strcmp( pParam3->m_strParam.c_str(), "Red" ) == 0 )
	{
		dwColor = 0xFFFF0000;
	}
	else if( strcmp( pParam3->m_strParam.c_str(), "Green" ) == 0 )
	{
		dwColor = 0xFF00FF00;
	}
	else if( strcmp( pParam3->m_strParam.c_str(), "Blue" ) == 0 )
	{
		dwColor = 0xFF0000FF;
	}
	else
	{
		dwColor = 0xFFFFFFFF;
	}

	pcmUIChatting->OnAddMessage( CHAT_MESSAGE_NORMAL , "Test", ( char* )pParam2->m_strParam.c_str(), dwColor );

	return TRUE;
}

BOOL CChatCmd::CallBack_OnAddTextFilter( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	return pcmUIChatting->OnAddFilter( ( char* )pParam->m_strParam.c_str(), TRUE, TRUE );
}

BOOL CChatCmd::CallBack_OnSocialAnimation( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AgcmCharacter* pcmCharacter = ( AgcmCharacter* )g_pEngine->GetModule( "AgcmCharacter" );
	if( !pcmCharacter ) return FALSE;

	return pcmCharacter->SendSocialAnimation( pParam->m_nParam );
}

BOOL CChatCmd::CallBack_OnFlagImoticon( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AgcmTextBoardMng* pcmTextBoardMng = ( AgcmTextBoardMng* )g_pEngine->GetModule( "AgcmTextBoardMng" );
	if( !pcmTextBoardMng ) return FALSE;

#ifdef _DEBUG
	if( strcmp( pParam->m_strParam.c_str(), "reload" ) == 0 )
	{
		return pcmTextBoardMng->ReloadFlag( "INI\\FlagImoticon.xml" );
	}
#endif

	return TRUE;
}

BOOL CChatCmd::CallBack_OnDrawOcclusion( void* pData )
{
	AgcmRender* pcmRender = ( AgcmRender* )g_pEngine->GetModule( "AgcmRender" );
	if( !pcmRender ) return FALSE;

	pcmRender->m_bDrawOcLineDebug = !pcmRender->m_bDrawOcLineDebug;
	return TRUE;
}

BOOL CChatCmd::CallBack_OnReloadTitle( void* pData )
{
	AgcmTitle* pcmTitle = ( AgcmTitle* )g_pEngine->GetModule( "AgcmTitle" );
	if( !pcmTitle ) return FALSE;

	pcmTitle->OnLoadTitleSetting( "INI\\TitleSetting.xml" );
	return TRUE;
}

BOOL CChatCmd::CallBack_OnChangeCameraMode( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AgcmCamera2* pcmCamera = ( AgcmCamera2* )g_pEngine->GetModule( "AgcmCamera2" );
	if( !pcmCamera ) return FALSE;

	AgcuCamMode::bGetInst().bChageMode( *pcmCamera, ( AgcuCamMode::eMode )pParam->m_nParam );
	return TRUE;
}

BOOL CChatCmd::CallBack_OnReportModule( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	ApModuleManager* pModuleManager = ( ApModuleManager* )pcmUIChatting->GetModuleManager();
	if( !pModuleManager ) return FALSE;

	char* pParamString = ( char* )pParam->m_strParam.c_str();
	if( !pParamString || strlen( pParamString ) <= 0 ) return FALSE;

	return strcmp( pParamString, "all" ) == 0 ? pModuleManager->ReportAll() : pModuleManager->ReportModule( pParamString );
}

BOOL CChatCmd::CallBack_OnCheckMemory( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	if( _CrtCheckMemory() )
	{
		pcmUIChatting->OnAddMessage( CHAT_MESSAGE_SYSTEM , "System", "메모리 OK~"  );
	}
	else
	{
		pcmUIChatting->OnAddMessage( CHAT_MESSAGE_SYSTEM , "System", "메모리에 문제 있음!"  );
	}

	return TRUE;
}

BOOL CChatCmd::CallBack_OnGuildBattleRequest( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam1 = pCmdData->m_mapParam.GetByIndex( 0 );
	stCmdParamEntry* pParam2 = pCmdData->m_mapParam.GetByIndex( 0 );
	stCmdParamEntry* pParam3 = pCmdData->m_mapParam.GetByIndex( 0 );
	stCmdParamEntry* pParam4 = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam1 || !pParam2 || !pParam3 || !pParam4 ) return FALSE;

	AgcmUIGuild* pcmUIGuild = ( AgcmUIGuild* )g_pEngine->GetModule( "AgcmUIGuild" );
	if( !pcmUIGuild ) return FALSE;

	return pcmUIGuild->OpenBattleRequestConfirmUI( ( char* )pParam1->m_strParam.c_str(), pParam2->m_nParam, pParam3->m_nParam, pParam4->m_nParam );
}

BOOL CChatCmd::CallBack_OnGuildBattleCancel( void* pData )
{
	AgcmUIGuild* pcmUIGuild = ( AgcmUIGuild* )g_pEngine->GetModule( "AgcmUIGuild" );
	if( !pcmUIGuild ) return FALSE;

	return pcmUIGuild->OpenBattleCancelRequestConfirmUI();
}

BOOL CChatCmd::CallBack_OnGuildBattleSurrender( void* pData )
{
	AgcmUIGuild* pcmUIGuild = ( AgcmUIGuild* )g_pEngine->GetModule( "AgcmUIGuild" );
	if( !pcmUIGuild ) return FALSE;

	return pcmUIGuild->OpenBattleWithdrawConfirmUI();
}

BOOL CChatCmd::CallBack_OnGuildLeaveOut( void* pData )
{
	AgcmUIGuild* pcmUIGuild = ( AgcmUIGuild* )g_pEngine->GetModule( "AgcmUIGuild" );
	if( !pcmUIGuild ) return FALSE;

	return pcmUIGuild->LeaveByCommand();
}

BOOL CChatCmd::CallBack_OnChangePerspective( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	BOOL bIsWideMode = g_pEngine->GetWideScreen();
	g_pEngine->SetProjection( pParam->m_fParam );

	AgcmUIChatting2* pcmUIChatting2 = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( pcmUIChatting2 )
	{
		char strMessage[ 64 ] = { 0, };
		sprintf_s( strMessage, sizeof( char ) * 64, "퍼스펙티브 값 변경 ( %3.3f )", pParam->m_fParam );
		pcmUIChatting2->OnAddMessage( CHAT_MESSAGE_SYSTEM , "System", strMessage  );
	}

	return TRUE;
}

BOOL CChatCmd::CallBack_OnChangeWideScreen( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	BOOL bIsWideMode = g_pEngine->GetWideScreen();
	g_pEngine->SetWideScreen( !bIsWideMode );

	AgcmUIChatting2* pcmUIChatting2 = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( pcmUIChatting2 )
	{
		char* pMsg = bIsWideMode ? "와이드 스크린 모드로 변경" : "일반 스크린 모드로 변경";
		pcmUIChatting2->OnAddMessage( CHAT_MESSAGE_SYSTEM , "System", pMsg );
	}

	return TRUE;
}

BOOL CChatCmd::CallBack_OnChangeSoundEffect( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AgcmSound* pcmSound = ( AgcmSound* )g_pEngine->GetModule( "AgcmSound" );
	if( !pcmSound ) return FALSE;

	pcmSound->SetRoom( ( eSoundEffectRoomType )pParam->m_nParam );
	return TRUE;
}

BOOL CChatCmd::CallBack_OnScriptLua( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AuLua* pLua = AuLua::GetPointer();
	if( !pLua ) return FALSE;

	// 루아파일은 엔진에서 실행
	if( strstr( pParam->m_strParam.c_str(), "." ) )
	{
		g_pEngine->Lua_RunScript( pParam->m_strParam.c_str(), FALSE );
	}
	else
	{
		pLua->RunString( pParam->m_strParam.c_str() );
	}

	return TRUE;
}

BOOL CChatCmd::CallBack_OnViewFPS( void* pData )
{
	if( !g_pEngine ) return FALSE;
	g_pEngine->m_bDrawFPS = !g_pEngine->m_bDrawFPS;
	return TRUE;
}

BOOL CChatCmd::CallBack_OnTest3DSound( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AgcmSound* pcmSound = ( AgcmSound* )g_pEngine->GetModule( "AgcmSound" );
	if( !pcmSound ) return FALSE;

	pcmSound->SetRoom( ( eSoundEffectRoomType )pParam->m_nParam );
	return TRUE;
}

BOOL CChatCmd::CallBack_OnChangeTime( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AgcmEventNature* pcmEventNature = ( AgcmEventNature* )g_pEngine->GetModule( "AgcmEventNature" );
	if( !pcmEventNature ) return FALSE;

	char strMsg[ 256 ] = { 0, };
	sprintf_s( strMsg, sizeof( char ) * 256, "%d시로 시간 변경합니다~", pParam->m_nParam );
	AgcChatManager::OnAddSystemMessage( strMsg );

	if( pParam->m_nParam < 0 )
	{
		pParam->m_nParam = 0;
	}
	else if( pParam->m_nParam > 23 )
	{
		pParam->m_nParam = 23;
	}

	AgcmEventNature::__SetTime( pcmEventNature, pParam->m_nParam, 1 );
	return TRUE;
}

BOOL CChatCmd::CallBack_OnToggleMatD3DFX( void* pData )
{
	AgcmRender* pcmRender = ( AgcmRender* )g_pEngine->GetModule( "AgcmRender" );
	if( !pcmRender ) return FALSE;

	if( pcmRender->IsMatD3DFxDisabled() )
	{
		pcmRender->EnableMatD3DFx();
		AgcChatManager::OnAddSystemMessage( "enable matd3dfx." );
	}
	else
	{
		pcmRender->DisableMatD3DFx();
		AgcChatManager::OnAddSystemMessage( "disable matd3dfx." );
	}

	return TRUE;
}

BOOL CChatCmd::CallBack_OnInviteParty( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam || strlen( pParam->m_strParam.c_str() ) <= 0 ) return FALSE;

	AgpmParty* ppmParty = ( AgpmParty* )g_pEngine->GetModule( "AgpmParty" );
	AgcmParty* pcmParty = ( AgcmParty* )g_pEngine->GetModule( "AgcmParty" );
	AgcmCharacter* pcmCharacter = ( AgcmCharacter* )g_pEngine->GetModule( "AgcmCharacter" );
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !ppmParty || !pcmParty || !pcmCharacter || !pcmUIManager ) return FALSE;

	// 일단 내 캐릭터가 있어야겠고
	AgpdCharacter* ppdSelfCharacter = pcmCharacter->GetSelfCharacter();
	if( !ppdSelfCharacter )	return FALSE;

	// 타겟 플레이어가 나 자신이 아니어야 하면서..
	if( strcmp( pParam->m_strParam.c_str(), ppdSelfCharacter->m_szID ) == 0 )
	{
		char* pErrMsg = pcmUIManager->GetUIMessage( "Party_Invite_Failed_Self" );
		if( pErrMsg && strlen( pErrMsg ) > 0 )
		{
			AgcChatManager::OnAddSystemMessage( pErrMsg );
		}

		return FALSE;
	}

	// 내가 파티에 가입된 상태 이라면
	AgpdParty* ppdParty = ppmParty->GetParty( ppdSelfCharacter );
	if( ppdParty )
	{
		// 내가 파장이어야 가능
		int nPartyLeaderCID = ppmParty->GetLeaderCID( ppdParty );
		if( nPartyLeaderCID != ppdSelfCharacter->m_lID )
		{
			char* pErrMsg = pcmUIManager->GetUIMessage( "Party_Invite_Failed_No_Leader" );
			if( pErrMsg && strlen( pErrMsg ) > 0 )
			{
				AgcChatManager::OnAddSystemMessage( pErrMsg );
				return FALSE;
			}
		}
	}


	// 파티 초대 메세지 송신
	//pcmParty->SendPacketInviteByCharID( ppdSelfCharacter->m_lID, ( char* )pParam->m_strParam.c_str() ); 

	AgcmUIPartyOption* pOption = (AgcmUIPartyOption*)g_pEngine->GetModule( "AgcmUIPartyOption" );
	if( NULL == pOption  )
		return FALSE;

	pOption->OpenPartyOption( ( char* )pParam->m_strParam.c_str() );

	return TRUE;
}

BOOL CChatCmd::CallBack_OnSendMsgShout( void* pData )
{
	return TRUE;

	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	int nMsgLength = ( int )strlen( pParam->m_strParam.c_str() );
	if( nMsgLength <= 0 ) return FALSE;

	AgpmAreaChatting* ppmAreaChatting = ( AgpmAreaChatting* )g_pEngine->GetModule( "AgpmAreaChatting" );
	AgcmUIChatting2* pcmUIChatting2 = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	AgcmCharacter* pcmCharacter = ( AgcmCharacter* )g_pEngine->GetModule( "AgcmCharacter" );
	if( !ppmAreaChatting || !pcmUIChatting2 || !pcmCharacter ) return FALSE;

	AgpdCharacter* ppdSelfCharacter = pcmCharacter->GetSelfCharacter();
	if( !ppdSelfCharacter ) return FALSE;

	// 아직 제한시간이 덜 되었다면 외치기 불가
	DWORD dwTime = ::timeGetTime();
	if( !pcmUIChatting2->IsEnableSendAreaChatting( dwTime ) ) return FALSE;

	short nPacketLength = 0;
	void* pPacket = ppmAreaChatting->MakePacketSendMessage( ppdSelfCharacter->m_lID, 0, 1, 0, ( char* )pParam->m_strParam.c_str(), nMsgLength, &nPacketLength );
	if( !pPacket || nPacketLength <= 0 ) return FALSE;

	BOOL bResult = ppmAreaChatting->SendPacket( pPacket, nPacketLength );
	ppmAreaChatting->m_csPacket.FreePacket( pPacket );

	pcmUIChatting2->UpdateTimeLastSendAreaMsg( dwTime );
	return bResult;
}

BOOL CChatCmd::CallBack_OnSendMsgJoin( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	int nMsgLength = ( int )strlen( pParam->m_strParam.c_str() );
	if( nMsgLength <= 0 ) return FALSE;

	AgcmUIChannel* pcmUIChannel = ( AgcmUIChannel* )g_pEngine->GetModule( "AgcmUIChannel" );
	if( !pcmUIChannel ) return FALSE;

	return pcmUIChannel->SendPacketJoinChannel( ( char* )pParam->m_strParam.c_str() );
}

BOOL CChatCmd::CallBack_OnSendMsgLeave( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	int nMsgLength = ( int )strlen( pParam->m_strParam.c_str() );
	if( nMsgLength <= 0 ) return FALSE;

	AgcmUIChannel* pcmUIChannel = ( AgcmUIChannel* )g_pEngine->GetModule( "AgcmUIChannel" );
	if( !pcmUIChannel ) return FALSE;

	return pcmUIChannel->SendPacketLeaveChannel( ( char* )pParam->m_strParam.c_str() );
}

BOOL CChatCmd::CallBack_OnMoveToPosition( void* pData )
{
	stCmdDataEntry* pCmdData = ( stCmdDataEntry* )pData;
	if( !pCmdData ) return FALSE;

	stCmdParamEntry* pParam = pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	// 입력된 문자열을 그대로 서버로 전송한다. 파싱은 서버에서 한다.
	char strBuffer[ 256 ] = { 0, };
	sprintf_s( strBuffer, sizeof( char ) * 256, "/move %s", pParam->m_strParam.c_str() );
	return pcmUIChatting->OnSendMessage( AGPDCHATTING_TYPE_NORMAL, strBuffer, strlen( strBuffer ), NULL , FALSE );
}

BOOL CChatCmd::CallBack_OnRejectTradeOn( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_TRADE, TRUE );
}

BOOL CChatCmd::CallBack_OnRejectTradeOff( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_TRADE, FALSE );
}

BOOL CChatCmd::CallBack_OnRejectPartyOn( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_PARTY_IN, TRUE );
}

BOOL CChatCmd::CallBack_OnRejectPartyOff( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_PARTY_IN, FALSE );
}

BOOL CChatCmd::CallBack_OnRejectGuildOn( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_GUILD_IN, TRUE );
}

BOOL CChatCmd::CallBack_OnRejectGuildOff( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_GUILD_IN, FALSE );
}

BOOL CChatCmd::CallBack_OnRejectGuildBattleOn( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_GUILD_BATTLE, TRUE );
}

BOOL CChatCmd::CallBack_OnRejectGuildBattleOff( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_GUILD_BATTLE, FALSE );
}

BOOL CChatCmd::CallBack_OnRejectGuildRelationOn( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_GUILD_RELATION, TRUE );
}

BOOL CChatCmd::CallBack_OnRejectGuildRelationOff( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_GUILD_RELATION, FALSE );
}

BOOL CChatCmd::CallBack_OnRejectBattleOn( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_BATTLE, TRUE );
}

BOOL CChatCmd::CallBack_OnRejectBattleOff( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_BATTLE, FALSE );
}

BOOL CChatCmd::CallBack_OnRejectBuddyOn( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_BUDDY, TRUE );
}

BOOL CChatCmd::CallBack_OnRejectBuddyOff( void* pData )
{
	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2 * )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;
	return pcmUIChatting->OnRequestChangeOptionFlag( AGPDCHAR_OPTION_REFUSE_BUDDY, FALSE );
}

// BOOL CChatCmd::CallBack_OnReloadNPCDialog( void * pData )
// {
// 	CHAR strFile[1024] = {0,};
// 	sprintf(strFile, "Ini\\%sNPCDialog.txt", g_pEngine->m_szLanguage);
// 
// 	AgpmEventNPCDialog * npcDialog = (AgpmEventNPCDialog *)g_pEngine->GetModule( "AgpmEventNPCDialog" );
// 
// 	return npcDialog ? npcDialog->LoadNPCDialogRes(strFile, TRUE) : FALSE;
// }

BOOL CChatCmd::CallBack_OnReloadGuide( void * pData )
{
	AgcmUIGuideMessage * guideMsg = (AgcmUIGuideMessage *)g_pEngine->GetModule( "AgcmUIGuideMessage" );

	if( guideMsg )
	{
		bool bIsEncrypted = m_bIsEncrypted != 0;
		guideMsg->LoadLevelUpGuideMessage(bIsEncrypted);
		guideMsg->LoadObjectGuideMessage(bIsEncrypted);
		guideMsg->InitMessage();
	}

	return TRUE;
}

BOOL CChatCmd::CallBack_OnReloadCamSetting( void* pData )
{
	AgcmCamera2*	pcmCamera		= (AgcmCamera2*)g_pEngine->GetModule("AgcmCamera2");
	AgcmCharacter*	pcmCharacter	= (AgcmCharacter*)g_pEngine->GetModule( "AgcmCharacter" );
	AgpmCharacter*	ppmCharacter	= (AgpmCharacter*)g_pEngine->GetModule( "AgpmCharacter" );

	if(!pcmCamera || !pcmCharacter || !ppmCharacter)
		return FALSE;

	AgpdCharacter*	pstAgpdCharacter	= pcmCharacter->GetSelfCharacter();

	if(!pstAgpdCharacter)
		return FALSE;

	if( pstAgpdCharacter->m_bRidable )	// 탈것에 타고 있는지 확인.
	{
		AgpdCharacter	*pstAgpdCharacterRide = ppmCharacter->GetCharacter( pstAgpdCharacter->m_nRideCID );

		if( pstAgpdCharacterRide )
		{
			pstAgpdCharacter = pstAgpdCharacterRide;
		}
	}

	AgcuCamMode::bGetInst().ReloadCamSetting();
	AgcuCamMode::bGetInst().SetCameraInfo( pcmCamera , pstAgpdCharacter );

	return TRUE;
}

BOOL	CChatCmd::CallBack_OnNoticeMessage( void* pData )
{
	AgcmUINotice*	pcmUINotice	=	static_cast< AgcmUINotice* >( g_pEngine->GetModule( "AgcmUINotice" ) );
	if( !pcmUINotice )
		return FALSE;

	stCmdDataEntry* pCmdData	=	static_cast< stCmdDataEntry* >(pData);
	if( !pCmdData ) 
		return FALSE;

	stCmdParamEntry* pParam		=	pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) 
		return FALSE;

	if( !pParam->m_strParam.length() )
		return FALSE;

	pcmUINotice->NoticeMacro( pParam->m_strParam.c_str() , FALSE);

	return TRUE;
}

BOOL	CChatCmd::CallBack_OnNoticeMessageTest( void* pData )
{
	AgcmUINotice*	pcmUINotice	=	static_cast< AgcmUINotice* >( g_pEngine->GetModule( "AgcmUINotice" ) );
	if( !pcmUINotice )
		return FALSE;

	stCmdDataEntry* pCmdData	=	static_cast< stCmdDataEntry* >(pData);
	if( !pCmdData ) 
		return FALSE;

	stCmdParamEntry* pParam		=	pCmdData->m_mapParam.GetByIndex( 0 );
	if( !pParam ) 
		return FALSE;

	if( !pParam->m_strParam.length() )
		return FALSE;

	pcmUINotice->NoticeMacro( pParam->m_strParam.c_str() , TRUE );

	return TRUE;
}

BOOL	CChatCmd::CallBack_OnReloadQuest( void* pData )
{
	//퀘스트 저널 관련부분
	AgpmQuest*			ppmQuest		=	static_cast< AgpmQuest* >( g_pEngine->GetModule( "AgpmQuest" ) );
	AgcmQuestJournal*	pcmQuestJournal	=	static_cast< AgcmQuestJournal* >( g_pEngine->GetModule( "AgcmQuestJournal" ) );
	AgcmUIQuest2*		pcmUIQuest2		=	static_cast< AgcmUIQuest2* >( g_pEngine->GetModule( "AgcmUIQuest2" ) );


	if( ppmQuest )
		ppmQuest->StreamReadTemplate( "Ini\\QUESTTEMPLATE.INI" , m_bIsEncrypted );

	if( pcmQuestJournal )
		pcmQuestJournal->ReadJournalText( "ini\\Quest_Aid_Data.txt", m_bIsEncrypted );

	if( pcmUIQuest2 )
		pcmUIQuest2->RefreshQuestUI();


	return TRUE;
}

BOOL CChatCmd::CallBack_OnReloadWorldmap( void * pData )
{
	ApmMap* ppmMap = static_cast<ApmMap*>(g_pEngine->GetModule("ApmMap"));
	if(!ppmMap)
		return FALSE;

	return ppmMap->ReloadWorldMap("ini\\worldmap.ini", m_bIsEncrypted);
}