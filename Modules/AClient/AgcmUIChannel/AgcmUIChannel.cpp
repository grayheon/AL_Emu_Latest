// -----------------------------------------------------------------------------
//                                _    _ _____  _____ _                            _                      
//     /\                        | |  | |_   _|/ ____| |                          | |                     
//    /  \    __ _  ___ _ __ ___ | |  | | | | | |    | |__   __ _ _ __  _ __   ___| |     ___ _ __  _ __  
//   / /\ \  / _` |/ __| '_ ` _ \| |  | | | | | |    | '_ \ / _` | '_ \| '_ \ / _ \ |    / __| '_ \| '_ \ 
//  / ____ \| (_| | (__| | | | | | |__| |_| |_| |____| | | | (_| | | | | | | |  __/ | _ | (__| |_) | |_) |
// /_/    \_\\__, |\___|_| |_| |_|\____/|_____|\_____|_| |_|\__,_|_| |_|_| |_|\___|_|(_) \___| .__/| .__/ 
//            __/ |                                                                          | |   | |    
//           |___/                                                                           |_|   |_|    
//
// 
//
// -----------------------------------------------------------------------------
// Originally created on 11/18/2005 by Nonstopdj
//
// Copyright 2005, NHN Games, Inc., all rights reserved.
// -----------------------------------------------------------------------------

#include "AgcmUIChannel.h"
#include "AgcmUIConsole.h"

#include "AuStrTable.h"
#include "AgcChatManager.h"

//. UI Common Message Define.
#define UI_MESSAGE_CHANNEL_NEED_PASSWORD				"Channel_Need_Password"
#define UI_MESSAGE_CHANNEL_JOIN_MESSAGE					"Channel_Join_Message"
#define UI_MESSAGE_CHANNEL_LEAVE_MESSAGE				"Channel_Leave_Message"
#define UI_MESSAGE_CHANNEL_CREATE_MESSAGE				"Channel_Create_Message"


// -----------------------------------------------------------------------------
BOOL AgcmUIChannel::m_sbCloneUI = TRUE;
BOOL AgcmUIChannel::m_bisWhisperBtnEnable = FALSE;

AgcmUIChannel::AgcmUIChannel()
{
	m_pcsAgcmUIChannel	= NULL;
	m_pcsAgpmChannel	= NULL;
	m_pcsAgcmCharacter	= NULL;
	m_pcsAgpmCharacter	= NULL;
	m_pcsAgcmUIMain		= NULL;
	m_pcsAgcmUIManager2 = NULL;
	m_pcsAgpmChatting	= NULL;
	m_pcsAgcmUIChatting2 = NULL;

	m_pstDummyUDChatList= NULL;
	m_pstDummyUDUserList= NULL;

	m_pstUDPublicRoom	= NULL;
	m_pstUDPrivateRoom	= NULL;
	m_pstUDGuildRoom	= NULL;		//. 대화방 만들기의 길드방 옵션 UD

	m_bIsPublicRoom		= TRUE;		//. 공개방 옵션상태
	m_bIsPrivateRoom	= FALSE;	//. 비밀방 옵션상태
	m_bIsGuildRoom		= FALSE;	//. 길드방 옵션상태
	m_bIsCreateRoom		= FALSE;

	m_iEventCreateChannel	= 0;
	m_iEventJoinChannel		= 0;
	m_iEventPasswordOpen	= 0;
	m_iEventPasswordClose	= 0;
	m_iEventInviteOpenUI	= 0;
	m_iEventInviteCloseUI	= 0;
	m_iEventInviteSendUI	= 0;
	m_iEventFindUserUI		= 0;
	m_iEventWhisperBtnStatus = 0;

	m_iEventAlreadyExist	= 0;			//. Message Code Event
	m_iEventNotExistChannel	= 0;
	m_iEventNotExistCharacter = 0;
	m_iEvnetFullChannel		= 0;
	m_iEventNotGuildMaster	= 0;
	m_iEventNotGuildMember	= 0;
	m_iEventAlreadyGuildChannel = 0;
	m_iEventInvalidPassword	= 0;
	m_iEventAlreadyJoin	= 0;

	m_eChannelType		= AGPDCHANNEL_TYPE_PUBLIC;

	for (INT32 i = 0; i < AGPDCHANNEL_MAX_JOIN; ++i)
	{
		m_arRoomWidget[i].InitUserIndex();
		m_arAgcdUI[i] = NULL;
	}

	// module initialize
	SetModuleName("AgcmUIChannel");
	EnableIdle2(FALSE);
}

// -----------------------------------------------------------------------------

AgcmUIChannel::~AgcmUIChannel()
{
	for(INT32 i = 0; i < AGPDCHANNEL_MAX_JOIN; ++i)
	{
		m_pcsAgcmUIManager2->RemoveUI(m_arAgcdUI[i]);
		m_arAgcdUI[i] = NULL;
	}
}

//.
BOOL AgcmUIChannel::OnAddModule()
{
	m_pcsAgcmUIChannel	= (AgcmUIChannel*)GetModule("AgcmUIChannel");
	m_pcsAgpmChannel	= (AgpmChannel*)GetModule("AgpmChannel");
	m_pcsAgcmCharacter	= (AgcmCharacter*)GetModule("AgcmCharacter");
	m_pcsAgpmCharacter	= (AgpmCharacter*)GetModule("AgpmCharacter");
	m_pcsAgcmUIMain		= (AgcmUIMain*)GetModule("AgcmUIMain");
	m_pcsAgcmUIManager2 = (AgcmUIManager2*)GetModule("AgcmUIManager2");
	m_pcsAgpmChatting	= (AgpmChatting*)GetModule("AgpmChatting");
	m_pcsAgcmUIChatting2 = (AgcmUIChatting2*)GetModule("AgcmUIChatting2");

	if (!m_pcsAgpmChannel || !m_pcsAgcmCharacter || !m_pcsAgcmUIManager2 || !m_pcsAgpmChatting)
		return FALSE;

	if (!m_pcsAgpmChannel->SetCallbackJoin(CBJoin, this))
		return FALSE;

	if (!m_pcsAgpmChannel->SetCallbackChatMsg(CBChatMsg, this))
		return FALSE;

	if (!m_pcsAgpmChannel->SetCallbackLeave(CBLeave, this))
		return FALSE;

	if (!m_pcsAgpmChannel->SetCallbackAddUser(CBAddUser, this))
		return FALSE;

	if (!m_pcsAgpmChannel->SetCallbackRemoveUser(CBRemoveUser, this))
		return FALSE;

	if (!m_pcsAgpmChannel->SetCallbackPassword(CBPassword, this))
		return FALSE;

	if (!m_pcsAgpmChannel->SetCallbackFindUser(CBFindUserResult, this))
		return FALSE;

	if (!m_pcsAgpmChannel->SetCallbackInvitation(CBInvitation, this))
		return FALSE;

	if (!m_pcsAgpmChannel->SetCallbackMsgCode(CBMsgCode, this))
		return FALSE;

	if (!m_pcsAgcmCharacter->SetCallbackSetSelfCharacter(CBCloneUI, this))
		return FALSE;

	if (!m_pcsAgcmCharacter->SetCallbackReleaseSelfCharacter(CBCloneUIClear, this))
		return FALSE;

	if (!AddEvent())
		return FALSE;

	if(!AddBoolean())
		return FALSE;

	if (!AddFunction())
		return FALSE;

	if (!AddUserData())
		return FALSE;
	
	if (!AddDisplay())
		return FALSE;


	return TRUE;
}

//.
BOOL AgcmUIChannel::OnInit()
{
	AS_REGISTER_TYPE_BEGIN(AgcmUIChannel, AgcmUIChannel);
		AS_REGISTER_METHOD0(void, CreateChannel);
		AS_REGISTER_METHOD0(void, JoinChannel);
		AS_REGISTER_METHOD0(void, FindUser);
	AS_REGISTER_TYPE_END;

	return TRUE;
}

//.
BOOL AgcmUIChannel::OnDestroy()
{
	return TRUE;
}

//.
BOOL AgcmUIChannel::OnIdle(UINT32 ulClockCount)
{
	return TRUE;
}

//.
BOOL AgcmUIChannel::AddEvent()
{
	AGCMUI_CHECK_RETURN(m_iEventCreateChannel, m_pcsAgcmUIManager2->AddEvent("CreateChannel_Open"));
	AGCMUI_CHECK_RETURN(m_iEventJoinChannel, m_pcsAgcmUIManager2->AddEvent("JoinChannel_Close"));
	AGCMUI_CHECK_RETURN(m_iEventPasswordOpen, m_pcsAgcmUIManager2->AddEvent("QueryPassword_Open"));
	AGCMUI_CHECK_RETURN(m_iEventPasswordClose, m_pcsAgcmUIManager2->AddEvent("QueryPassword_Close"));
	AGCMUI_CHECK_RETURN(m_iEventInviteOpenUI, m_pcsAgcmUIManager2->AddEvent("InviteChannel_Open"));
	AGCMUI_CHECK_RETURN(m_iEventInviteCloseUI, m_pcsAgcmUIManager2->AddEvent("InviteChannel_Close"));
	AGCMUI_CHECK_RETURN(m_iEventInviteSendUI, m_pcsAgcmUIManager2->AddEvent("InviteSend_Open"));
	AGCMUI_CHECK_RETURN(m_iEventFindUserUI, m_pcsAgcmUIManager2->AddEvent("FindUser_Open"));
	AGCMUI_CHECK_RETURN(m_iEventWhisperBtnStatus, m_pcsAgcmUIManager2->AddEvent("WhisperBtn_Status"));

	AGCMUI_CHECK_RETURN(m_iEventAlreadyExist, m_pcsAgcmUIManager2->AddEvent("ChannelAlreadyExist_MsgUI"));
	AGCMUI_CHECK_RETURN(m_iEventNotExistChannel, m_pcsAgcmUIManager2->AddEvent("ChannelNotExist_MsgUI"));
	AGCMUI_CHECK_RETURN(m_iEventNotExistCharacter, m_pcsAgcmUIManager2->AddEvent("NotExistCharacter_MsgUI"));
	AGCMUI_CHECK_RETURN(m_iEvnetFullChannel, m_pcsAgcmUIManager2->AddEvent("ChannelCountFull_MsgUI"));
	AGCMUI_CHECK_RETURN(m_iEventNotGuildMaster, m_pcsAgcmUIManager2->AddEvent("NotGuildMaster_MsgUI"));
	AGCMUI_CHECK_RETURN(m_iEventNotGuildMember, m_pcsAgcmUIManager2->AddEvent("NOtGuildMember_MsgUI"));
	AGCMUI_CHECK_RETURN(m_iEventAlreadyGuildChannel, m_pcsAgcmUIManager2->AddEvent("GuildChannelAlreadyExist_MsgUI"));
	AGCMUI_CHECK_RETURN(m_iEventInvalidPassword, m_pcsAgcmUIManager2->AddEvent("InvalidPassword_MsgUI"));

	return TRUE;
}

BOOL AgcmUIChannel::AddBoolean()
{
	if (!m_pcsAgcmUIManager2->AddBoolean(this, "ChangeWhisperBtn_onoff", CBWhiperBtnStatus, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	return TRUE;
}

//.
BOOL AgcmUIChannel::AddFunction()
{
	if (!m_pcsAgcmUIManager2->AddFunction(this, "CreateChannelOption_Clear", CBCreateRoomClear, 0))
		return FALSE;

	//. 2005. 12. 1. Nonstopdj
	//. EditBox를 하나 갖는 UI에서 사용하는 초기화 function
	if (!m_pcsAgcmUIManager2->AddFunction(this, "UIEditBox_Init", CBEditBoxClear, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "FindUser_Init", CBFindUserClear, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "InviteChannelReceive_Init", CBInviteRoomInit, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "InvitationUI_Open", CBInviteOpen, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "InviteChannelSend", CBInviteSend, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "InviteJoinChannel_Click", CBInviteJoinRoom, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "FindUser_Click", CBFindUser, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SendWhisper_Click", CBSendWhisper, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "CreateChannel", CBCreateRoom, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "CreateChannelOption_PuplicCLick", CBCheckPulicOptionClick, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "CreateChannelOption_PrivateCLick", CBCheckPrivateOptionClick, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "CreateChannelOption_GuildCLick", CBCheckGuildOptionClick, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "ChannelSend_ChatMessage", CBEnterChatMessage, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "Channeljoin_click", CBJoinRoom, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "ChannelLeave_click", CBLeaveRoom, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "PasswordOK_click", CBQueryPassword, 0))
		return FALSE;

	return TRUE;
}

//.
BOOL AgcmUIChannel::AddUserData()
{
	//. 대화방 dummy UD
	if (!(m_pstDummyUDChatList = m_pcsAgcmUIManager2->AddUserData("DummyChatList_UD", &m_iDummyChatListIndex, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32)))
		return FALSE;
	if (!(m_pstDummyUDUserList = m_pcsAgcmUIManager2->AddUserData("DummyUserList_UD", &m_iDummyUserListIndex, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32)))
		return FALSE;

	if(!(m_pstWhiperBtnData = m_pcsAgcmUIManager2->AddUserData("WhisperBtnStatus_UD", &m_iWhisperBtnStatus, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32)))
		return TRUE;

	//. 최대 5개의 대화방에 따른 RoomWidget(UD 2쌍)설정하기
	TCHAR strUDName[128];
	for(int nCount = 0; nCount < AGPDCHANNEL_MAX_JOIN; nCount++)
	{	
		wsprintf(strUDName, "ChatList%d_UD", nCount);
		if (!(m_arRoomWidget[nCount].m_pstUDChatList = m_pcsAgcmUIManager2->AddUserData(strUDName, 
																			&m_arRoomWidget[nCount].m_IChatList, 
																			sizeof(INT32), 
																			0, //AGPDCHANNEL_MAX_CHAT_LINE
																			AGCDUI_USERDATA_TYPE_INT32)))
			return FALSE;

		wsprintf(strUDName, "UserList%d_UD", nCount);
		if (!(m_arRoomWidget[nCount].m_pstUDUserList = m_pcsAgcmUIManager2->AddUserData(strUDName, 
																			&m_arRoomWidget[nCount].m_arUser[0], 
																			sizeof(INT32), 
																			AGPDCHANNEL_MEMBER_GUILD, //. MAX User수는 Guild채팅 인원.
																			AGCDUI_USERDATA_TYPE_INT32)))
			return FALSE;
	}


	//. 대화방 만들기 세가지옵션 UD
	if (!(m_pstUDPublicRoom = m_pcsAgcmUIManager2->AddUserData("CreateRoomPublicOption_UD", &m_bIsPublicRoom, sizeof(INT), 1, AGCDUI_USERDATA_TYPE_BOOL)))
		return FALSE;
	if (!(m_pstUDPrivateRoom = m_pcsAgcmUIManager2->AddUserData("CreateRoomPrivateOption_UD", &m_bIsPrivateRoom, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_BOOL)))
		return FALSE;
	if (!(m_pstUDGuildRoom = m_pcsAgcmUIManager2->AddUserData("CreateRoomGuildOption_UD", &m_bIsGuildRoom, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_BOOL)))
		return FALSE;

	return TRUE;
}

//.
BOOL AgcmUIChannel::AddDisplay()
{
	//. 
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "ChatList_Display", 0, CBDisplayChatList, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "UserList_Display", 0, CBDisplayUserList, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "ChannelTitle_Display", 0, CBDisplayChannelTitle, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	return TRUE;
}

//.
AgpdChannel*	AgcmUIChannel::CreateChannelData(AgpdChannelParam *pChannelParam)
{
	if(!pChannelParam)
		return NULL;

	AgpdChannel* pcsChannel = m_pcsAgpmChannel->CreateChannelData();

	pcsChannel->Initialize(pChannelParam->m_eType);
	pcsChannel->m_strName.SetText(pChannelParam->m_strName);
	pcsChannel->m_strPassword.SetText(pChannelParam->m_strPassword);

	//. UserList구성하기
	TCHAR* pszList = (TCHAR*)pChannelParam->m_pvUserList;
	INT32 ILength = (INT32)_tcslen(pszList);
	INT32 IStartPos = 0;

	for(INT32 nIndex = 0; nIndex < ILength; nIndex++ )
	{
		if(!_tcsncmp( ";", &pszList[nIndex], 1))
		//if(pszList[nIndex] == ';');.
		{
			TCHAR pszBuffer[AGPDCHARACTER_NAME_LENGTH];
			memset(pszBuffer, 0, sizeof(TCHAR)*AGPDCHARACTER_NAME_LENGTH);
			_tcsncpy(pszBuffer, &pszList[IStartPos], nIndex - IStartPos);

			ApStrCharacterName pName;
			pName.SetText(pszBuffer);
			
			//. 유저리스트에 삽입
			pcsChannel->m_UserList.push_back(pName);
			IStartPos = nIndex + 1; //. ';' 다음 주소부터 유저이름 시작~
		}
	}

	//. Admin Channel(hash)에 등록.
	m_pcsAgpmChannel->m_csAdminChannel.Add(pChannelParam->m_strName, pcsChannel);

	return pcsChannel;
}

////////////////////////////////////////////////////////////////////////////////////
//. Refresh function
//. 대화방만들기 옵션 Refresh
BOOL AgcmUIChannel::RefreshCreateRoomOption()
{
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDPublicRoom) &&
			m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDPrivateRoom) &&
			m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDGuildRoom);;
}
//. 모든 대화방 Refresh
BOOL AgcmUIChannel::RefreshAllChatRoom()
{
	return TRUE;
}

//. 해당 인덱스의 대화방만 Refresh
BOOL AgcmUIChannel::RefreshChatRoom(INT32 nIndex)
{
	return 	m_pcsAgcmUIManager2->SetUserDataRefresh(m_arRoomWidget[nIndex].m_pstUDChatList) &&
			m_pcsAgcmUIManager2->SetUserDataRefresh(m_arRoomWidget[nIndex].m_pstUDUserList );
}

//. 해당 인덱스의 유저리스트만 Refresh
BOOL AgcmUIChannel::RefreshChatRoomUserList(INT32 nIndex)
{
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_arRoomWidget[nIndex].m_pstUDUserList );
}

BOOL AgcmUIChannel::RefreshWhisperBtnStatus()
{
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstWhiperBtnData);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//. send packet
//. chat rooom create packet
BOOL AgcmUIChannel::SendCreateChannel(TCHAR* szRoomName, TCHAR* szPassword)
{
	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();

	ASSERT(NULL != pcsCharacter);
	if (NULL == pcsCharacter)
		return FALSE;

	INT16 nPacketLength = 0;

	PVOID pvPacket= m_pcsAgpmChannel->MakePacketCreateChannel(&nPacketLength, pcsCharacter->m_lID, 
																szRoomName, 
																szPassword, 
																m_eChannelType);	//. 옵션 버튼 클릭시 미리 설정된 값으로 send
	if (NULL == pvPacket)
	{
		m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

//. chatting message packet
BOOL AgcmUIChannel::SendPacketChatMsg(TCHAR *szChannelName, TCHAR *szChatMsg)
{
	if( !szChannelName || !szChatMsg )
		return FALSE;
	m_pcsAgcmUIChatting2->OnFiltering(szChatMsg);

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();

	ASSERT(NULL != pcsCharacter);
	if (NULL == pcsCharacter)
		return FALSE;

	INT16 nPacketLength = 0;

	PVOID pvPacket= m_pcsAgpmChannel->MakePacketChatMsg(&nPacketLength, pcsCharacter->m_lID, 
																szChannelName, 
																pcsCharacter->m_szID, 
																szChatMsg);
	if (NULL == pvPacket)
	{
		m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

//. Join channel packet
BOOL AgcmUIChannel::SendPacketJoinChannel(TCHAR *szChannelName)
{
	if( !szChannelName )
		return TRUE;

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();

	ASSERT(NULL != pcsCharacter);
	if (NULL == pcsCharacter)
		return FALSE;

	INT16 nPacketLength = 0;

	PVOID pvPacket= m_pcsAgpmChannel->MakePacketJoinChannel(&nPacketLength, pcsCharacter->m_lID, 
																szChannelName, 
																AGPDCHANNEL_TYPE_NONE, 
																NULL, 
																0);
	if (NULL == pvPacket)
	{
		m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

//. Leave Channel Packet
BOOL AgcmUIChannel::SendPacketLeaveChannel(TCHAR *szChannelName)
{
	if( !szChannelName )
		return TRUE;

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();

	ASSERT(NULL != pcsCharacter);
	if (NULL == pcsCharacter)
		return FALSE;

	INT16 nPacketLength = 0;

	PVOID pvPacket= m_pcsAgpmChannel->MakePacketLeaveChannel(&nPacketLength, pcsCharacter->m_lID, szChannelName);

	if (NULL == pvPacket)
	{
		m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

//. Password packet
BOOL AgcmUIChannel::SendPacketPassword(TCHAR *szChannelName, TCHAR *szPassword)
{
	if( !szChannelName || !szPassword)
		return TRUE;

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();

	ASSERT(NULL != pcsCharacter);
	if (NULL == pcsCharacter)
		return FALSE;

	INT16 nPacketLength = 0;

	PVOID pvPacket= m_pcsAgpmChannel->MakePacketPassword(&nPacketLength, pcsCharacter->m_lID, szChannelName, szPassword );

	if (NULL == pvPacket)
	{
		m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

//. invitation packet
BOOL AgcmUIChannel::SendPacketInvitation(TCHAR *szChannelName, EnumChannelType eType, TCHAR *szDestUser)
{
	if( !szChannelName || !szDestUser)
		return TRUE;

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();

	ASSERT(NULL != pcsCharacter);
	if (NULL == pcsCharacter)
		return FALSE;

	INT16 nPacketLength = 0;

	PVOID pvPacket= m_pcsAgpmChannel->MakePacketInvitation(&nPacketLength, pcsCharacter->m_lID, szChannelName, eType, szDestUser );

	if (NULL == pvPacket)
	{
		m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

//. Find User packet
BOOL AgcmUIChannel::SendPacketFindUser(CHAR *szFindUserName)
{
	if( !szFindUserName)
		return TRUE;

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();

	ASSERT(NULL != pcsCharacter);
	if (NULL == pcsCharacter)
		return FALSE;

	INT16 nPacketLength = 0;

	PVOID pvPacket= m_pcsAgpmChannel->MakePacketFindUser(&nPacketLength, pcsCharacter->m_lID, szFindUserName );

	if (NULL == pvPacket)
	{
		m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);
		return FALSE;
	}

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmChannel->m_csPacket.FreePacket(pvPacket);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//. callback functions
//.
BOOL AgcmUIChannel::CBCloneUI(PVOID pData, PVOID pClass, PVOID pCustData)
{
	//. SetSelfCharacter되는 시점에서 대화방UI를 복사해놓는다.
	if (!pClass)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter *	pcsCharacter = (AgpdCharacter *) pData;
	AgcdCharacter *	pstCharacter = pThis->m_pcsAgcmCharacter->GetCharacterData(pcsCharacter);

	if (pstCharacter->m_bTransforming)
		return TRUE;

	if(pThis->m_sbCloneUI)
	{
		//. UI를 복사해 놓는다.
		AgcdUI* pOriginal = pThis->m_pcsAgcmUIManager2->GetUI("UI_ChatRoom");
		if(!pOriginal)
			return FALSE;

		TCHAR strUIName[128];
		for(INT32 nCount = 0; nCount < AGPDCHANNEL_MAX_JOIN; nCount++)
		{
			AgcdUI* pClone = NULL;
			AgcdUIControl* pControl;

			wsprintf(strUIName, "UI_ChatRoom%d", nCount);
			pClone = pThis->m_pcsAgcmUIManager2->CopyUI(pOriginal, strUIName);
			
			//. 2005. 12. 20. Nonstopdj
			//. ESC키 자동 닫힘을 막음.
			pClone->m_bAutoClose = FALSE;

			INT32 lIndex = 0;
			for (pControl = pThis->m_pcsAgcmUIManager2->GetSequenceControl(pClone, &lIndex); pControl; pControl = pThis->m_pcsAgcmUIManager2->GetSequenceControl(pClone, &lIndex))
			{
				if (pControl->m_pstUserData == pThis->m_pstDummyUDChatList)
				{
					//. 대화리스트 UD설정
					pControl->m_pstUserData = pThis->m_arRoomWidget[nCount].m_pstUDChatList;
					pThis->m_pcsAgcmUIManager2->CountUserData(pThis->m_arRoomWidget[nCount].m_pstUDChatList, pControl);
				}
				else if (pControl->m_pstUserData == pThis->m_pstDummyUDUserList)
				{
					//. 유저리스트 UD설정
					pControl->m_pstUserData = pThis->m_arRoomWidget[nCount].m_pstUDUserList;
					pThis->m_pcsAgcmUIManager2->CountUserData(pThis->m_arRoomWidget[nCount].m_pstUDUserList, pControl);
				}
			}

			pThis->m_pcsAgcmUIManager2->SetUIData(pClone, nCount);
			pThis->m_pcsAgcmUIManager2->AddUI(pClone);
			pThis->m_arAgcdUI[nCount] = pClone;
		}
	}

	return TRUE;
}


//.
BOOL AgcmUIChannel::CBDestroyChannel(PVOID pData, PVOID pClass, PVOID pCustData)
{
	return TRUE;
}

//.
BOOL AgcmUIChannel::CBJoin(PVOID pData, PVOID pClass, PVOID pCustData)
{
	//. todo : 대화방을 복사해서 만들고 그것과 연결되는 UD를 가지고 Refresh한다.
	//.		   대화방정보(m_arRoom)의 UD를 이용하여 업데이트(메세지, 유저출입등)시 Refresh.
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpdChannelParam *pChannelParam = (AgpdChannelParam *)pCustData;

	return pThis->OperationJoinChannel(pcsCharacter, pThis->CreateChannelData(pChannelParam) );
}

//.
BOOL AgcmUIChannel::CBLeave(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpdChannelParam *pChannelParam = (AgpdChannelParam *)pCustData;

	AgpdChannel* pcsRemoveChannel = pThis->m_pcsAgpmChannel->m_csAdminChannel.Get(pChannelParam->m_strName);

	if(!pcsRemoveChannel)
		return FALSE;

	return pThis->OperationCloseChannel(pcsRemoveChannel);
}

//.
BOOL AgcmUIChannel::CBInvitation(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpdChannelParam *pChannelParam = (AgpdChannelParam *)pCustData;

	pThis->m_eChannelType = pChannelParam->m_eType;
	pThis->m_strChannelName.SetText(pChannelParam->m_strName.GetBuffer());			//. 현재 입장하려는 방의 제목저장. (초대포함)
	pThis->m_strInviteCharName.SetText(pChannelParam->m_strUserName.GetBuffer());	//.	현재 초대한 사람의 이름 저장.

	//. 기존에 열려있을 지 모를 대화방 초대창을 닫고 새로 연다.
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_iEventInviteCloseUI);
	return pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_iEventInviteOpenUI);
}

//. 
BOOL AgcmUIChannel::CBPassword(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpdChannelParam *pChannelParam = (AgpdChannelParam *)pCustData;

	//. 입장하려는 방의 제목을 저장한 후. 기존에 열려있을지 모르는 Password입력창을
	//. 닫고 다시 연다.
	pThis->m_strChannelName.SetText(pChannelParam->m_strName.GetBuffer());

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_iEventPasswordClose);
	return pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_iEventPasswordOpen);
}

//. 채팅메세지 callback
BOOL AgcmUIChannel::CBChatMsg(PVOID pData, PVOID pClass, PVOID pCustData)
{
	//. 채팅메세지 출력형태로 가공하여 m_arRoomWidget[].m_pcsMessage저장하기
	//. refresh -> Display - Editbox->SetText로 업데이트.
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpdChannelParam *pChannelParam = (AgpdChannelParam *)pCustData;

	AgpdChannel *pcsChannel = pThis->m_pcsAgpmChannel->m_csAdminChannel.Get(pChannelParam->m_strName);

	if(!pcsChannel)
		return FALSE;

	return pThis->OperationChatMsg(pcsChannel, 
								   pChannelParam, 
								  _tcscmp((TCHAR*)pcsCharacter->m_szID, pChannelParam->m_strUserName.GetBuffer()));
}

//. 유저찾기 결과 콜백
BOOL AgcmUIChannel::CBFindUserResult(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpdChannelParam *pChannelParam = (AgpdChannelParam *)pCustData;
	
	//. 이곳에 들어오면 친구를 찾은 것임.
	pThis->m_strInviteCharName.SetText(pChannelParam->m_strUserName.GetBuffer());

	//. 버튼의 상태는 Enable.
	return pThis->OperationFindUser(TRUE);
}

//. Enter Channel
BOOL AgcmUIChannel::CBAddUser(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpdChannelParam *pChannelParam = (AgpdChannelParam *)pCustData;

	AgpdChannel *pcsChannel = pThis->m_pcsAgpmChannel->m_csAdminChannel.Get(pChannelParam->m_strName);

	if(!pcsChannel)
		return FALSE;

	//. 자기자신을 제외한 입장/퇴장 시스템 메세지
	if( _tcscmp( (TCHAR*)pcsCharacter->m_szID, pChannelParam->m_strUserName.GetBuffer()))
	{
		ApString<96> strTemp;
		strTemp.Format(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_CHANNEL_JOIN_MESSAGE), pChannelParam->m_strUserName.GetBuffer());
		AgcChatManager::OnAddSystemMessage(strTemp);
	}

	return pThis->OperationAddUser(pcsChannel, pChannelParam);
}

//. Leave Channel
BOOL AgcmUIChannel::CBRemoveUser(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpdChannelParam *pChannelParam = (AgpdChannelParam *)pCustData;

	AgpdChannel *pcsChannel = pThis->m_pcsAgpmChannel->m_csAdminChannel.Get(pChannelParam->m_strName);

	if(!pcsChannel )
		return FALSE;

	//. 자기자신을 제외한 입장/퇴장 시스템 메세지
	if( _tcscmp( (TCHAR*)pcsCharacter->m_szID, pChannelParam->m_strUserName.GetBuffer()))
	{
		ApString<96> strTemp;
		strTemp.Format(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_CHANNEL_LEAVE_MESSAGE), pChannelParam->m_strUserName.GetBuffer());
		AgcChatManager::OnAddSystemMessage(strTemp);
	}

	return pThis->OperationRemoveUser(pcsChannel, pChannelParam);
}


//. 정보/오류 메세지코드 throw Event.
BOOL AgcmUIChannel::CBMsgCode(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData || !pCustData)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpdChannelParam *pChannelParam = (AgpdChannelParam *)pCustData;
	
	int nEventMsgCode = 0;
	switch(pChannelParam->m_eMsgCode)
	{
		case AGPMCHANNEL_MSG_CODE_ALREADY_EXIST:		// 이미 존재하는 채널
			nEventMsgCode = pThis->m_iEventAlreadyExist;
			break;
		case AGPMCHANNEL_MSG_CODE_NOT_EXIST_CHANNEL:	// 존재하지 않는 채널
			nEventMsgCode = pThis->m_iEventNotExistChannel;
			break;
		case AGPMCHANNEL_MSG_CODE_NOT_EXIST_CHARACTER:	// 찾는 캐릭터가 존재하지 않거나 Off-line
			nEventMsgCode = pThis->m_iEventNotExistCharacter;
			break;
		case AGPMCHANNEL_MSG_CODE_FULL_CHANNEL:			// 동시에 입장 가능한 채널을 가득찼다.
			nEventMsgCode = pThis->m_iEvnetFullChannel;
			break;
		case AGPMCHANNEL_MSG_CODE_NOT_GUILDMASTER:		// 길드 마스터가 아니다.
			nEventMsgCode = pThis->m_iEventNotGuildMaster;
			break;
		case AGPMCHANNEL_MSG_CODE_NOT_GUILDMEMBER:		// 길드 멤버가 아니다.
			nEventMsgCode = pThis->m_iEventNotGuildMember;
			break;
		case AGPMCHANNEL_MSG_CODE_ALREADY_GUILDCHANNEL:	// 이미 길드 채널을 만들었다.
			nEventMsgCode = pThis->m_iEventAlreadyGuildChannel;
			break;
		case AGPMCHANNEL_MSG_CODE_INVALID_PASSWORD:		// 패스워드가 유효하지 않다.
			nEventMsgCode = pThis->m_iEventInvalidPassword;
			break;
		case AGPMCHANNEL_MSG_CODE_ALREADY_JOIN:
			AgcChatManager::OnAddSystemMessage(ClientStr().GetStr(STI_ALREADYCHANNEL_JOIN));
			break;
	}

	return pThis->m_pcsAgcmUIManager2->ThrowEvent(nEventMsgCode);
}

//. AgcdUIData Clear.
BOOL AgcmUIChannel::CBCloneUIClear(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass)
		return FALSE;

	AgcmUIChannel *	pThis = (AgcmUIChannel*) pClass;
	AgpdCharacter *	pcsCharacter = (AgpdCharacter *) pData;
	AgcdCharacter *	pstCharacter = pThis->m_pcsAgcmCharacter->GetCharacterData(pcsCharacter);

	if (pstCharacter->m_bTransforming)
		return TRUE;

	for (INT32 i = 0; i < AGPDCHANNEL_MAX_JOIN; ++i)
	{
		pThis->m_arRoomWidget[i].InitUserIndex();
		pThis->m_pcsAgcmUIManager2->RemoveUI(pThis->m_arAgcdUI[i]);

		pThis->m_arAgcdUI[i] = NULL;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//. Operation
//. 채널입장처리.
BOOL AgcmUIChannel::OperationJoinChannel(AgpdCharacter *pcsCharacter, AgpdChannel *pcsChannel)
{
	//. Close상태를 UI에 해당하는 AgcdUI를 찾고 그 ID로 저장된 방정보에
	//. pcsChannel을 설정하고. UserData로 Refresh한다.

	for( int nCount = 0; nCount < AGPDCHANNEL_MAX_JOIN; nCount++)
	{
		if( AGCDUI_STATUS_ADDED == m_arAgcdUI[nCount]->m_eStatus)
		{
			//. 기존에 설정되어 있을지도 모를 문자열등을 클리어 한다.
			m_arRoomWidget[nCount].clear();
			m_arRoomWidget[nCount].m_pcsChannel = pcsChannel;

			//. 유저명수만큼 Display함수를 호출하기 위해
			m_arRoomWidget[nCount].m_pstUDUserList->m_stUserData.m_lCount = pcsChannel->m_UserList.size();
			
			//. 해당창열기
			m_pcsAgcmUIManager2->OpenUI(m_arAgcdUI[nCount]);

			//. 2006. 1. 6. Nonstopdj
			//. 대화방개설 로그남기기.
			if(m_bIsCreateRoom)
			{
				ApString<256> strTemp;
				strTemp.Format(m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_CHANNEL_CREATE_MESSAGE), pcsChannel->m_strName.GetBuffer());
				AgcChatManager::OnAddSystemMessage(strTemp);

				m_bIsCreateRoom = FALSE;
			}

			return RefreshChatRoom(nCount);
		}
	}

	//. error : 닫혀있는 방이 하나도 없으므로. 5개초과의 방조인을 요청하거나 
	//.			m_arAgcdUI Data가 이상.
	return FALSE;
}

//. 채팅메세지처리.
BOOL AgcmUIChannel::OperationChatMsg(AgpdChannel* pcsChannel, AgpdChannelParam *pChannelParam, INT32 IsSelf)
{
	for( int nCount = 0; nCount < AGPDCHANNEL_MAX_JOIN; nCount++)
	{
		if(m_arRoomWidget[nCount].m_pcsChannel == pcsChannel)
		{
			//. Chatting Messesage Add.
			m_arRoomWidget[nCount].AddChatMsg(pChannelParam, (IsSelf) ?  OTHERCHATCOLOR : SELFCHATCOLOR );

			return RefreshChatRoom(nCount);
		}
	}

	return FALSE;
}

BOOL AgcmUIChannel::OperationFindUser(BOOL bResult)
{
	(bResult) ? m_bisWhisperBtnEnable = TRUE :  m_bisWhisperBtnEnable = FALSE;
	return RefreshWhisperBtnStatus();
}

//. 채널나가기.
BOOL AgcmUIChannel::OperationCloseChannel(AgpdChannel* pChannel)
{
	for( int nCount = 0; nCount < AGPDCHANNEL_MAX_JOIN; nCount++)
	{
		//. 2006. 7. 14. Nonstopdj
		//. 두 채널이상 들어갔을경우 앞채널에서 나오면
		//. m_arRoomWidget[nCount].m_pcsChannel는 NULL이므로
		//. NULL체크가 필요하다.
		if( m_arRoomWidget[nCount].m_pcsChannel && m_arRoomWidget[nCount].m_pcsChannel->m_strName == pChannel->m_strName)
		{
			//. HashData Remove
			m_pcsAgpmChannel->m_csAdminChannel.Remove(pChannel->m_strName);
			//. 저장되어있던 방정보를 Clear
			m_pcsAgpmChannel->DestroyChannelData(m_arRoomWidget[nCount].m_pcsChannel);

			m_arRoomWidget[nCount].clear();

			//. UI를 닫는다.
			return m_pcsAgcmUIManager2->CloseUI(m_arAgcdUI[nCount]);
		}
	}

	return FALSE;
}

//. User추가.
BOOL AgcmUIChannel::OperationAddUser(AgpdChannel* pChannel, AgpdChannelParam *pChannelParam)
{
	for( int nCount = 0; nCount < AGPDCHANNEL_MAX_JOIN; nCount++)
	{
		if(m_arRoomWidget[nCount].m_pcsChannel && m_arRoomWidget[nCount].m_pcsChannel == pChannel)
		{
			m_arRoomWidget[nCount].m_pcsChannel->m_UserList.push_back(pChannelParam->m_strUserName);
			m_arRoomWidget[nCount].m_pstUDUserList->m_stUserData.m_lCount = m_arRoomWidget[nCount].m_pcsChannel->m_UserList.size();

			//. 유저리스트만 Refresh한다.
			return RefreshChatRoomUserList(nCount);
		}
	}

	return FALSE;
}

//. User제거.
BOOL AgcmUIChannel::OperationRemoveUser(AgpdChannel* pChannel, AgpdChannelParam *pChannelParam)
{
	for( int nCount = 0; nCount < AGPDCHANNEL_MAX_JOIN; nCount++)
	{
		if(m_arRoomWidget[nCount].m_pcsChannel && m_arRoomWidget[nCount].m_pcsChannel == pChannel)
		{
			//. 해당이름의 유저를 찾는다.
			std::list<ApStrCharacterName>::iterator Iter = std::find(m_arRoomWidget[nCount].m_pcsChannel->m_UserList.begin(), 
														m_arRoomWidget[nCount].m_pcsChannel->m_UserList.end(), pChannelParam->m_strUserName);

			if (Iter != m_arRoomWidget[nCount].m_pcsChannel->m_UserList.end())
			{
				//. 해당유저를 지운다.
				m_arRoomWidget[nCount].m_pcsChannel->m_UserList.erase(Iter);
				m_arRoomWidget[nCount].m_pstUDUserList->m_stUserData.m_lCount = m_arRoomWidget[nCount].m_pcsChannel->m_UserList.size();
			}

			//. 유저리스트만 Refresh한다.
			return RefreshChatRoomUserList(nCount);
		}
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////////////////
//. display
//. 대화내용 Display
BOOL AgcmUIChannel::CBDisplayChatList(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	//. 설정한 UI Data는 Index이므로..
	int nIndex = pThis->m_pcsAgcmUIManager2->GetUIData(pcsSourceControl->m_pcsParentUI);

	if( nIndex < 0 && nIndex > AGPDCHANNEL_MAX_JOIN)
		return FALSE;

	AcUIEdit	*pcsEdit	= (AcUIEdit *)pcsSourceControl->m_pcsBase;

	//. 미리 저장된 문자열로 SetText한다.
	if(pcsEdit)
	{
		pcsEdit->SetText(pThis->m_arRoomWidget[nIndex].m_pcsMessage.GetBuffer());

		//. 무조건 스크롤max값으로 지정하여 글이 추가될 시 무조건 가장 아래로 설정해버림.
		if(pcsEdit->GetVScroll())
			pcsEdit->GetVScroll()->SetScrollValue(1.0f);
	}
	else
		return FALSE;

	return TRUE;
}

//. 대화방내 유저목록 Display
BOOL AgcmUIChannel::CBDisplayUserList(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || eType != AGCDUI_USERDATA_TYPE_INT32)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	INT32 Index = *(INT32*)pData;

	//. 설정한 UI Data는 Index이므로..
	int nIndex = pThis->m_pcsAgcmUIManager2->GetUIData(pcsSourceControl->m_pcsParentUI);

	if( nIndex < 0 && nIndex > AGPDCHANNEL_MAX_JOIN)
		return FALSE;

	//. 한명씩 Draw하므로 현재 Index를 이용하여 슬롯의 유저를 구한다음 szDisplay로 copy한다.
	if(NULL != pThis->m_arRoomWidget[nIndex].m_pcsChannel &&
		( INT32 ) pThis->m_arRoomWidget[nIndex].m_pcsChannel->m_UserList.size() > pcsSourceControl->m_lUserDataIndex)
	{
		std::list<ApStrCharacterName>::iterator Iter = pThis->m_arRoomWidget[nIndex].m_pcsChannel->m_UserList.begin();

		for(int nCount = 0; nCount < Index; nCount++ )
			Iter++;

		strncpy(szDisplay, (*Iter).GetBuffer(), AGPDCHARACTER_NAME_LENGTH);
	}

	return TRUE;
}

//. 대화방제목 Display
BOOL AgcmUIChannel::CBDisplayChannelTitle(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || eType != AGCDUI_USERDATA_TYPE_INT32)
		return FALSE;
	
	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	//. 설정한 UI Data는 Index이므로..
	int nIndex = pThis->m_pcsAgcmUIManager2->GetUIData(pcsSourceControl->m_pcsParentUI);

	if(pThis->m_arRoomWidget[nIndex].m_pcsChannel)
	{
		strncpy(szDisplay, pThis->m_arRoomWidget[nIndex].m_pcsChannel->m_strName.GetBuffer(),
			pThis->m_arRoomWidget[nIndex].m_pcsChannel->m_strName.GetBufferLength());
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////
//. functions
//. 대화방 초기화
BOOL AgcmUIChannel::CBCreateRoomClear(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1 || !pData2 )
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	//. 대화방 만들기 기본 옵션은 공개방
	pThis->m_bIsPublicRoom		= TRUE;
	pThis->m_bIsPrivateRoom		= FALSE;
	pThis->m_bIsGuildRoom		= FALSE;
	pThis->m_eChannelType		= AGPDCHANNEL_TYPE_PUBLIC;		 //. 기본옵션은 공개방
	pThis->RefreshCreateRoomOption();

	AgcdUIControl* pUIRoomName = (AgcdUIControl*)pData1;
	AgcdUIControl* pUIPassword = (AgcdUIControl*)pData2;
	AcUIEdit* pUIRoomNameEdit = (AcUIEdit*)pUIRoomName->m_pcsBase;
	AcUIEdit* pUIPasswordEdit = (AcUIEdit*)pUIPassword->m_pcsBase;

	pUIRoomNameEdit->SetText("");
	pUIPasswordEdit->SetText("");
	pUIRoomNameEdit->SetMeActiveEdit();

	return TRUE;
}

//. 초대받은 채널의 정보표시를 위해 초기화.
BOOL AgcmUIChannel::CBInviteRoomInit(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgcdUIControl* pUIEditControl = (AgcdUIControl*)pData1;
	AcUIEdit* pUIEdit = (AcUIEdit*)pUIEditControl->m_pcsBase;

	//. todo EditBox에 저장된 초대발신캐릭터이름과 방제목으로 SetText
	TCHAR pBuffer[256];
	TCHAR pChanneTypeName[32];

	//. channel 타입에 따른 방속성문자열설정
	if(AGPDCHANNEL_TYPE_PUBLIC == pThis->m_eChannelType)
		_stprintf(pChanneTypeName, "%s", ClientStr().GetStr(STI_PUBLIC_CHANNEL) );
	else if(AGPDCHANNEL_TYPE_PRIVATE == pThis->m_eChannelType)
		_stprintf(pChanneTypeName, "%s", ClientStr().GetStr(STI_PRIVATE_CHANNEL) );
	else if(AGPDCHANNEL_TYPE_GUILD == pThis->m_eChannelType)
		_stprintf(pChanneTypeName, "%s", ClientStr().GetStr(STI_GUILD_CHANNEL) );

	_stprintf(pBuffer, ClientStr().GetStr(STI_CHANNEL_INVITE),
		pThis->m_strInviteCharName.GetBuffer(),
		pThis->m_strChannelName.GetBuffer(),
		ROOMTYPECOLOR,
		pChanneTypeName,
		OTHERCHATCOLOR);

	return pUIEdit->SetText(pBuffer);
}

//. EditBox를 첫번째 인자로 넣어 초기화하고 에디트박스에 포커스를 맞춘다.  ; 대화방 입장, 대화방 초대, 캐릭터찾기, 비밀번호 입력에 쓰임
BOOL AgcmUIChannel::CBEditBoxClear(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgcdUIControl* pUIEditControl = (AgcdUIControl*)pData1;

	AcUIEdit* pUIEdit = (AcUIEdit*)pUIEditControl->m_pcsBase;
	pUIEdit->SetText("");
	pUIEdit->SetMeActiveEdit();

	return TRUE;
}

//. Find User UI를 위한 Clear
BOOL AgcmUIChannel::CBFindUserClear(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	AgcdUIControl* pUIEditControl = (AgcdUIControl*)pData1;

	//. 기존에 찾았던 ID는 Clear하고 focus를 맞춤.
	AcUIEdit* pUIEdit = (AcUIEdit*)pUIEditControl->m_pcsBase;
	pUIEdit->SetText("");
	pUIEdit->SetMeActiveEdit();

	//. 열때 마다 귓말버튼은 disable시킨다.
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pstWhiperBtnData);

	return TRUE;
}


//***************************************************************************************************
// Function Name	: bool CBCreateRoom(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
//
// Last Modified 	: 2005. 11. 22	Nonstopdj
// First Modified	: 2005. 11. 22	Nonstopdj
//
// Function			: UI_CreateChatRoom에서 대화방 만들기버튼을 클릭했을 시의 호출되는 callback
//
// Inputs			: 
//
// Outputs			: 
//
// Returns			: Boolean
//***************************************************************************************************
BOOL AgcmUIChannel::CBCreateRoom(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl || !pData1)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	AgpdCharacter *pcsCharacter = pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if (NULL == pcsCharacter)
		return FALSE;

	AgcdUIControl* pUIRoomName = (AgcdUIControl*)pData1;
	AgcdUIControl* pUIPassword = (AgcdUIControl*)pData2;
	
	if((AcUIBase::TYPE_EDIT != pUIRoomName->m_lType) && (AcUIBase::TYPE_EDIT != pUIPassword->m_lType))
		return FALSE;

	AcUIEdit* pUIRoomNameEdit = (AcUIEdit*)pUIRoomName->m_pcsBase;
	AcUIEdit* pUIPasswordEdit = (AcUIEdit*)pUIPassword->m_pcsBase;

	if(strlen(pUIRoomNameEdit->GetText()) < ApStrChannelName::eBufferLength)
	{
		AgpdChannel channel;
		channel.Initialize(pThis->m_eChannelType);
		channel.m_strName.SetText(pUIRoomNameEdit->GetText());

		if(pThis->m_eChannelType == AGPDCHANNEL_TYPE_PRIVATE )
		{
			if( strlen(pUIPasswordEdit->GetText()) < ApStrChannelName::eBufferLength && strlen(pUIPasswordEdit->GetText()) > 0)
				pThis->SendCreateChannel((TCHAR*)pUIRoomNameEdit->GetText(), (TCHAR*)pUIPasswordEdit->GetText());
			else
			{
				//. todo: 시스템 메세지
				//. '비밀번호를 입력해주세요.'
				AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_CHANNEL_NEED_PASSWORD));
				return FALSE;
			}
		}
		else
		{
			if(pThis->SendCreateChannel((TCHAR*)pUIRoomNameEdit->GetText(), NULL))
				pThis->m_bIsCreateRoom = TRUE; //. 대화방에 입장한 것인지 대화방을 만든것인지 판별한다.
		}
		
	}
	else
	{
		//. todo : 시스템메세지
		//. '대화방제목을 입력하세요'
		return FALSE;
	}

	//. error : 에디트박스의 문자열길이가 설정한 값보다 크다.
	//.			툴에서 수정요.

	//. EditBox Clear.
	pUIRoomNameEdit->SetText("");
	pUIPasswordEdit->SetText("");

	return TRUE;
}

//. 공개방 옵션 체크
BOOL AgcmUIChannel::CBCheckPulicOptionClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (NULL == pClass)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	pThis->m_bIsPublicRoom		= TRUE;
	pThis->m_bIsPrivateRoom		= FALSE;
	pThis->m_bIsGuildRoom		= FALSE;

	pThis->m_eChannelType = AGPDCHANNEL_TYPE_PUBLIC;

	return pThis->RefreshCreateRoomOption();
}

//. 비밀방 옵션 체크
BOOL AgcmUIChannel::CBCheckPrivateOptionClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (NULL == pClass)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	pThis->m_bIsPublicRoom		= FALSE;
	pThis->m_bIsPrivateRoom		= TRUE;
	pThis->m_bIsGuildRoom		= FALSE;

	pThis->m_eChannelType = AGPDCHANNEL_TYPE_PRIVATE;

	return pThis->RefreshCreateRoomOption();
}

//. 길드방 옵션체크
BOOL AgcmUIChannel::CBCheckGuildOptionClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (NULL == pClass)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	pThis->m_bIsPublicRoom		= FALSE;
	pThis->m_bIsPrivateRoom		= FALSE;
	pThis->m_bIsGuildRoom		= TRUE;
	pThis->m_eChannelType = AGPDCHANNEL_TYPE_GUILD;

	return pThis->RefreshCreateRoomOption();
}

//. 대화방 다이얼로그에서 메세지 입력 후 Enter을 입력했을 경우.
BOOL AgcmUIChannel::CBEnterChatMessage(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
		return FALSE;

	if (AcUIBase::TYPE_EDIT != pcsSourceControl->m_lType)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	//. 설정한 UI Data는 Index이므로..
	int nIndex = pThis->m_pcsAgcmUIManager2->GetUIData(pcsSourceControl->m_pcsParentUI);

	if( nIndex < 0 && nIndex > AGPDCHANNEL_MAX_JOIN)
		return FALSE;

	AcUIEdit* pUIEdit = (AcUIEdit*)pcsSourceControl->m_pcsBase;
	CHAR* szDelimiter = pThis->m_pcsAgcmUIManager2->GetLineDelimiter();
	pUIEdit->SetLineDelimiter(szDelimiter);

	//. chatting 메세지 패킷 전송
	//. EditBox의 입력된 문자열은 최대 120으로 툴에서 지정해놓았음.
	
	//. 2005. 12. 15. Nonstopdj.
	//. 엔터입력후에도 자동으로 포커스를 유지시키는 옵션을 삽입함. by 이종석PD님.
	if(strlen(pUIEdit->GetText()) > 0)
	{
		pThis->SendPacketChatMsg(pThis->m_arRoomWidget[nIndex].m_pcsChannel->m_strName, (TCHAR*)pUIEdit->GetText());
		pUIEdit->SetText("");		//. 이전에 입력했던 내용지우기
	}

	return TRUE;
}

//. 대화방 초대(Send)
BOOL AgcmUIChannel::CBInviteOpen(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;
	
	//. 설정한 UI Data는 Index이므로..
	int nIndex = pThis->m_pcsAgcmUIManager2->GetUIData(pcsSourceControl->m_pcsParentUI);

	//. 초대 ID UI창에서 사용하기 위한 방이름과 방속성 임시 저장
	pThis->m_strChannelName.SetText(pThis->m_arRoomWidget[nIndex].m_pcsChannel->m_strName.GetBuffer());
	pThis->m_eChannelType = pThis->m_arRoomWidget[nIndex].m_pcsChannel->GetChannelType();

	return 	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_iEventInviteSendUI);
}

//. 초대 메세지 보내기
BOOL AgcmUIChannel::CBInviteSend(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	AgcdUIControl* pUIEditCtrl = (AgcdUIControl*)pData1;
	
	if(AcUIBase::TYPE_EDIT != pUIEditCtrl->m_lType)
		return FALSE;

	AcUIEdit* pUIEdit = (AcUIEdit*)pUIEditCtrl->m_pcsBase;

	return pThis->SendPacketInvitation(pThis->m_strChannelName.GetBuffer(), pThis->m_eChannelType, (TCHAR*)pUIEdit->GetText());
}

//. 유저 찾기
BOOL AgcmUIChannel::CBFindUser(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	AgcdUIControl* pUIEditCtrl = (AgcdUIControl*)pData1;
	
	if(AcUIBase::TYPE_EDIT != pUIEditCtrl->m_lType)
		return FALSE;

	AcUIEdit* pUIEdit = (AcUIEdit*)pUIEditCtrl->m_pcsBase;

	return pThis->SendPacketFindUser((TCHAR*)pUIEdit->GetText());
}

//. 귓말보내기 버튼 상태바꾸기
BOOL AgcmUIChannel::CBWhiperBtnStatus(PVOID pClass, PVOID pData, AgcdUIDataType eType, AgcdUIControl *pcsSourceControl)
{
	//. static BOOL 이므로.
	if(m_bisWhisperBtnEnable)
	{
		//. 다음 상태를 위하여.
		m_bisWhisperBtnEnable = FALSE;
		return TRUE;
	}

	return FALSE;
}

//. 찾은 유저 귓말 보내기 
BOOL AgcmUIChannel::CBSendWhisper(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	//. to do: '/w 유저' -> 노말대화창에 Insert
	if (!pClass)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	ApString<64> strWhisperMsg;
	strWhisperMsg.Format(_T("%s %s "), ClientStr().GetStr(STI_CHAT_WHISPER_LOWER), pThis->m_strInviteCharName.GetBuffer());

	// Chat UI를 Open후 귓속말 메세지를 추가한다.
	pThis->m_pcsAgcmUIChatting2->OnShowEditChatting();
	pThis->m_pcsAgcmUIChatting2->OnSetChattingMessage( strWhisperMsg );
	return TRUE;
}


//. 방에 입장
BOOL AgcmUIChannel::CBJoinRoom(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	AgcdUIControl* pUIRoomName = (AgcdUIControl*)pData1;
	
	if(AcUIBase::TYPE_EDIT != pUIRoomName->m_lType)
		return FALSE;

	AcUIEdit* pUIRoomNameEdit = (AcUIEdit*)pUIRoomName->m_pcsBase;
	
	if(pThis->SendPacketJoinChannel((TCHAR*)pUIRoomNameEdit->GetText()))
		pThis->m_bIsCreateRoom = FALSE; //. 대화방에 입장한 것

	pUIRoomNameEdit->SetText("");

	return TRUE;
}

//. 초대에 응할 방에 입장
//. 초대콜백이 호출되면 호출한 방의 제목을 멤버변수 저장한 후 사용한다.
BOOL AgcmUIChannel::CBInviteJoinRoom(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
			return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	return pThis->SendPacketJoinChannel((TCHAR*)pThis->m_strChannelName.GetBuffer());
}

//. 대화방을 떠남
BOOL AgcmUIChannel::CBLeaveRoom(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
			return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	//. 설정한 UI Data는 Index이므로..
	int nIndex = pThis->m_pcsAgcmUIManager2->GetUIData(pcsSourceControl->m_pcsParentUI);

	return pThis->SendPacketLeaveChannel((TCHAR*)(pThis->m_arRoomWidget[nIndex].m_pcsChannel->m_strName.GetBuffer()));
}

//. 패스워드입력OK 클릭
BOOL AgcmUIChannel::CBQueryPassword(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIChannel* pThis = (AgcmUIChannel*)pClass;

	AgcdUIControl* pUIPassword = (AgcdUIControl*)pData1;

	if(AcUIBase::TYPE_EDIT != pUIPassword->m_lType)
		return FALSE;

	AcUIEdit* pUIPasswordEdit = (AcUIEdit*)pUIPassword->m_pcsBase;

	pThis->SendPacketPassword(pThis->m_strChannelName.GetBuffer(),  (TCHAR*)pUIPasswordEdit->GetText());
	pUIPasswordEdit->SetText("");	//. Edit박스내용 비우기

	return TRUE;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//. script
void  AgcmUIChannel::CreateChannel()
{
	m_pcsAgcmUIManager2->ThrowEvent(m_iEventCreateChannel);
}

void  AgcmUIChannel::JoinChannel()
{
	m_pcsAgcmUIManager2->ThrowEvent(m_iEventJoinChannel);
}
void  AgcmUIChannel::FindUser()
{
	m_pcsAgcmUIManager2->ThrowEvent(m_iEventFindUserUI);
}

// -----------------------------------------------------------------------------
// AgcmUIChannel.cpp - End of file
// -----------------------------------------------------------------------------