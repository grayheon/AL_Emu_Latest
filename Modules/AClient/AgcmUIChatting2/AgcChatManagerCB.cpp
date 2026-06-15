#include "AgcChatManager.h"

#include "AgpmChatting.h"
#include "AgcmResourceLoader.h"
#include "AgcmUIChatting2.h"

#include "AgcUIChatMessage.h"
#include "AgcChatMessage.h"

BOOL AgcChatManager::CallBack_RecvMsgChatting( void* pData, void* pClass, void* pCustData )
{
	AgcChatManager*		pManager	= static_cast< AgcChatManager* >(pClass);
	AgpdChatData*		pChatData	= static_cast< AgpdChatData* >(pData);
	BOOL				bParse		= (BOOL)pCustData;
	DWORD				dwColor		= 0;
	if( !pManager || !pChatData ) 
		return FALSE;

	// 유효하지 않은 메세지이거나 명령코드면 패스
	if( !pChatData->szMessage || strlen( pChatData->szMessage ) <= 0 || pChatData->szMessage[ 0 ] == '/' ) 
		return TRUE;

	// 메세지 길이만큼만 처리하기 위해 해당 위치에서 끊어준다.
	pChatData->szMessage[ pChatData->lMessageLength ] = '\0';

	// 메세지랑 캐릭터 이름이 합쳐진 메세지다. 파싱 작업을 해줘야 한다.
	if( bParse )
	{
		pManager->_ParseMessageData( pChatData );

		dwColor	=	0xffa55252;
	}

	AgcmResourceLoader*	pLoader		=	static_cast< AgcmResourceLoader* >(g_pEngine->GetModule( "AgcmResourceLoader" ));
	if(!pLoader)
		return FALSE;

	switch( pChatData->eChatType )
	{
	case AGPDCHATTING_TYPE_NORMAL :
		{
			dwColor = pLoader->GetColor("NORMAL_CHAT");
			pManager->OnAddMessage( CHAT_MESSAGE_NORMAL , pChatData->szSenderName, pChatData->szMessage , dwColor , pChatData->lSenderID , pChatData->ulCharType );
		}
		break;

	case AGPDCHATTING_TYPE_GUILD :
		{
			dwColor = pLoader->GetColor("GUILD_CHAT");
			pManager->OnAddMessage( CHAT_MESSAGE_GUILD, pChatData->szSenderName, pChatData->szMessage , dwColor , pChatData->lSenderID , pChatData->ulCharType );
		}
		break;
	case AGPDCHATTING_TYPE_GUILD_JOINT :
		{
			dwColor = pLoader->GetColor("GUILDJOINT_CHAT");
			pManager->OnAddMessage( CHAT_MESSAGE_GUILD, pChatData->szSenderName, pChatData->szMessage , dwColor , pChatData->lSenderID , pChatData->ulCharType );
		}
		break;
	case AGPDCHATTING_TYPE_GUILD_NOTICE :
		{
			dwColor = pLoader->GetColor("GUILDNOTICE_CHAT");
			pManager->OnAddMessage( CHAT_MESSAGE_GUILD, pChatData->szSenderName, pChatData->szMessage , dwColor , pChatData->lSenderID , pChatData->ulCharType );
		}
		break;

	case AGPDCHATTING_TYPE_PARTY :
		{
			dwColor = pLoader->GetColor("PARTY_CHAT");
			pManager->OnAddMessage( CHAT_MESSAGE_PARTY, pChatData->szSenderName, pChatData->szMessage , dwColor , pChatData->lSenderID, pChatData->ulCharType );
		}
		break;

	case AGPDCHATTING_TYPE_WHISPER :
	case AGPDCHATTING_TYPE_WHISPER2 :
		{
			dwColor = pLoader->GetColor("WHISPER_SEND");
			pManager->OnAddMessage( CHAT_MESSAGE_WHISPER, pChatData->szSenderName, pChatData->szMessage , dwColor , pChatData->lSenderID , pChatData->ulCharType, pChatData->szTargetName );
		}
		break;

	case AGPDCHATTING_TYPE_SYSTEM_LEVEL1 :
	case AGPDCHATTING_TYPE_SYSTEM_LEVEL2 :
	case AGPDCHATTING_TYPE_SYSTEM_LEVEL3 :
		{
			pManager->OnAddMessage( CHAT_MESSAGE_SYSTEM , pChatData->szSenderName, pChatData->szMessage , 0 , pChatData->lSenderID , pChatData->ulCharType );
		}
		break;

// 	case AGPDCHATTING_TYPE_SYSTEM_LEVEL2:
// 		{
// 			pManager->OnAddMessage( CHAT_MESSAGE_SYSTEM2 , pChatData->szSenderName , pChatData->szMessage , 0 , pChatData->lSenderID , pChatData->ulCharType );
// 		}
// 		break;
// 
// 	case AGPDCHATTING_TYPE_SYSTEM_LEVEL3:
// 		{
// 			pManager->OnAddMessage( CHAT_MESSAGE_SYSTEM3 , pChatData->szSenderName , pChatData->szMessage , 0 , pChatData->lSenderID , pChatData->ulCharType );
// 		}
// 		break;

	case AGPDCHATTING_TYPE_NOTICE_LEVEL1 :
	case AGPDCHATTING_TYPE_NOTICE_LEVEL2 :
	case AGPDCHATTING_TYPE_WHOLE_WORLD :
		{
			dwColor = pLoader->GetColor("NOTICE_CHAT1");
			pManager->OnAddMessage( CHAT_MESSAGE_NOTICE, pChatData->szSenderName, pChatData->szMessage , dwColor , pChatData->lSenderID , pChatData->ulCharType );
		}
		break;

	case AGPDCHATTING_TYPE_SIEGEWARINFO :
		{
			dwColor = pLoader->GetColor("SIEGEWARINFO_CHAT");
			pManager->OnAddMessage( CHAT_MESSAGE_SIEGE, pChatData->szSenderName, pChatData->szMessage , dwColor , pChatData->lSenderID , pChatData->ulCharType );
		}
		break;

	case AGPDCHATTING_TYPE_ARCHLORD :
		{
			dwColor = pLoader->GetColor("Yellow");
			pManager->OnAddMessage( CHAT_MESSAGE_ARCHLORD, pChatData->szSenderName, pChatData->szMessage, dwColor, pChatData->lSenderID, pChatData->ulCharType );
		}
		break;
	case AGPDCHATTING_TYPE_WORD_BALLOON:
		{
			dwColor = pLoader->GetColor("WORD_BALLON");
			pManager->OnAddMessage( CHAT_MESSAGE_SHOUT, pChatData->szSenderName, pChatData->szMessage, dwColor, pChatData->lSenderID, pChatData->ulCharType );
		}
		break;
	case AGPDCHATTING_TYPE_RACE:
		{
			dwColor = pLoader->GetColor("RACE_TRUMPET");
			pManager->OnAddMessage( CHAT_MESSAGE_RACE, NULL, pChatData->szMessage, dwColor, pChatData->lSenderID, pChatData->ulCharType );
		}
		break;
	case AGPDCHATTING_TYPE_ALL:
		{
			dwColor = pLoader->GetColor("Yellow");
			pManager->OnAddMessage( CHAT_MESSAGE_ELEMENTAL, NULL, pChatData->szMessage, dwColor, pChatData->lSenderID, pChatData->ulCharType );
		}
		break;
	}
	
	return TRUE;
}

BOOL	AgcChatManager::CallBack_WhisperOffline( PVOID pData, PVOID pClass, PVOID pCustData )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	CHAR*							pTargetID	=	(CHAR*)pCustData;
	CHAR							szMessage[ MAX_PATH ];

	ZeroMemory( szMessage , MAX_PATH );

	if( !pTargetID || !strlen(pTargetID)  )
		return FALSE;

	sprintf_s( szMessage , MAX_PATH , "%s %s" , pTargetID , pThis->m_pcsAgcmUIManager2->GetUIMessage( "Chat_Offline_Whisper_Target_Msg" ) );

	pThis->OnAddSystemMessage( szMessage );

	return TRUE;
}

BOOL	AgcChatManager::CAllBack_WhisperBlock( PVOID pData, PVOID pClass, PVOID pCustData )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	CHAR*						pTargetID	=	(CHAR*)pCustData;
	CHAR						szMessage[MAX_PATH];

	ZeroMemory( szMessage, MAX_PATH );

	if( !pTargetID || !strlen(pTargetID) )
		return FALSE;

	sprintf_s( szMessage , MAX_PATH , "%s %s" , pTargetID , pThis->m_pcsAgcmUIManager2->GetUIMessage( "Chat_Block_Whisper_Msg" )  );

	pThis->OnAddSystemMessage( szMessage );

	return TRUE;
}

DWORD AgcChatManager::GetTextColor( char* pColorName )
{
	if( !pColorName || strlen( pColorName ) <= 0 ) return 0xFFFFFFFF;

	AgcmResourceLoader* pcmResourceLoader = ( AgcmResourceLoader* )g_pEngine->GetModule( "AgcmResourceLoader" );
	if( !pcmResourceLoader ) return 0xFFFFFFFF;

	return pcmResourceLoader->GetColor( pColorName );
}

BOOL AgcChatManager::OnAddChattingMessage( UINT eType, AgpdChatData* ppdData )
{
	if( !ppdData ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	pcmUIChatting->OnAddMessage( eType , ppdData->szSenderName, ppdData->szMessage );
	return TRUE;
}

BOOL AgcChatManager::OnAddSystemMessage( char* pMessage, DWORD dwCustomColor )
{
	if( !pMessage || strlen( pMessage ) <= 0 ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) 
		return FALSE;

	pcmUIChatting->OnAddMessage( CHAT_MESSAGE_SYSTEM, "SYSTEM", pMessage, dwCustomColor );
	return TRUE;
}

BOOL AgcChatManager::OnAddNoticeMessage( char* pMessage )
{
	if( !pMessage || strlen( pMessage ) <= 0 ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	pcmUIChatting->OnAddMessage( CHAT_MESSAGE_NOTICE, "NOTICE", pMessage );
	return TRUE;
}

BOOL AgcChatManager::OnAddQuestMessage( char* pMessage )
{
	if( !pMessage || strlen( pMessage ) <= 0 ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	pcmUIChatting->OnAddMessage( CHAT_MESSAGE_QUEST, "QUEST", pMessage );
	return TRUE;
}

BOOL AgcChatManager::OnAddShrineMessage( char* pMessage )
{
	if( !pMessage || strlen( pMessage ) <= 0 ) return FALSE;

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )g_pEngine->GetModule( "AgcmUIChatting2" );
	if( !pcmUIChatting ) return FALSE;

	pcmUIChatting->OnAddMessage( CHAT_MESSAGE_SIEGE, "SIEGE", pMessage );
	return TRUE;
}

BOOL	AgcChatManager::CBDragCallback( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcChatManager*		pThis			=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	PVOID*		ppvData	=	(PVOID*)pData1;

	AgcChatTab*			pChatTab		=	static_cast< AgcChatTab* >( ppvData[0] );
	if( !pChatTab )
		return FALSE;

	AgcUIChatMessage*	pUIChatMessage	=	static_cast< AgcUIChatMessage* >( ppvData[1] );
	if( !pUIChatMessage )
		return FALSE;

	RwV2d*				pOffsetPos		=	static_cast< RwV2d* >(pData2);
	if( !pOffsetPos )
		return FALSE;

	AcUIEventButton*	pEventButton	=	static_cast< AcUIEventButton* >(pButton);
	if( !pEventButton )
		return FALSE;

	INT					nDragX				=	(INT)pEventButton->GetDragStartPos().x;
	INT					nDragY				=	(INT)pEventButton->GetDragStartPos().y;

	pEventButton->SetModal();
	pEventButton->ClientToScreen( &nDragX , &nDragY );

	INT		nDistnace	=	(INT)sqrt( pow( pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.x - nDragX , 2 ) + pow( pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.y - nDragY , 2 ) );
	INT		nCount		=	pUIChatMessage->GetTabCount();

	// 창이 여러개 붙어있고... 일정 범위를 벗어났다면.. 분리 시킨다
	if( nDistnace > DRAG_START_DISTANCE && nCount > 1 )

	{
		pEventButton->ReleaseModal();

		AgcUIChatMessage*	pNewUIChat	=	pThis->_CopyUIChatMessage( pUIChatMessage );
		ASSERT( pNewUIChat );

		// 매니저 리스트에 등록한다.
		pThis->AddUIChatMessage( pNewUIChat );

		// 선택된 탭을 추가한다.
		pNewUIChat->AddTab( pChatTab->GetTabName() , pChatTab->GetTabType() , pChatTab->IsPrimary() );

		// 기존에 붙어 있던건 삭제한다.
		pUIChatMessage->DelTab( pChatTab->GetTabName() );
				
		pThis->ShowChatUI( pNewUIChat );

		pUIChatMessage->RefreshTabButton();
		pNewUIChat->RefreshTabButton();

		// 도킹 이벤트가 물려 있다면 지운다
		pThis->_DockingPreEventEnd( pUIChatMessage );

		pNewUIChat->SetLastButton( 0 );
		pNewUIChat->MoveChatWindow( (INT)(pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.x - pOffsetPos->x) , 
										(INT)(pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.y - pOffsetPos->y)	);

		pThis->m_bIsNeedSave	=	TRUE;

	}

	// 해당 위치로 Move
	else if( nCount == 1 )
	{
		
		pUIChatMessage->MoveChatWindow( (int)(pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.x - pOffsetPos->x), 
										(int)(pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.y - pOffsetPos->y)	);

		pThis->m_bIsNeedSave	=	TRUE;
	}
	
	return TRUE;
}

BOOL	AgcChatManager::CBEventCallback( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcChatManager*	pThis	=	static_cast<AgcChatManager*>(pClass);
	if( !pThis )
		return FALSE;

	stChatMsgEntry*	pMsg	=	static_cast< stChatMsgEntry* >(pData1);
	if( !pMsg )
		return FALSE;

	string			strBuffer;
	eEventType		eType	=	EVENT_TYPE_NONE;
	INT				nTemp1	=	0;
	INT				nTemp2	=	0;
	INT				nTemp3	=	0;

	ListUIControlIter	Iter	=	pMsg->m_listText.begin();
	for( ; Iter != pMsg->m_listText.end() ; ++Iter )
	{
		if( (*Iter)->m_pcsUIBase	==	pButton )
		{
			strBuffer	=	(*Iter)->m_pcsUIBase->m_szStaticString;
			eType		=	(*Iter)->m_eType;

			nTemp1	=	(INT)(*Iter)->m_pData1;
			nTemp2	=	(INT)(*Iter)->m_pData2;
			nTemp3	=	(INT)(*Iter)->m_pData3;

			break;
		}
	}

	switch( eType )
	{
	case EVENT_TYPE_ITEM:		pThis->_ItemEvent( nTemp1 );											break;
	case EVENT_TYPE_MAP_POS:	pThis->_MapPosEvent( strBuffer.c_str() , nTemp1 , nTemp2 , nTemp3 );	break;
	case EVENT_TYPE_WHISPER:	pThis->_WhisperEvent( strBuffer.c_str() );								break;
	}

	return TRUE;
}

VOID	AgcChatManager::CBDockingCallback( PVOID pClass , PVOID pData1 , PVOID pData2 )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	AgcUIChatMessage*	pSrcUI	=	static_cast< AgcUIChatMessage* >(pData1);
	if( !pThis || !pSrcUI )
		return;

	// 도킹 시작
	INT					nMinLength	=	999999;
	AgcUIChatMessage*	pDstUI		=	NULL;

	ListUIChatMessageIter	Iter	=	pThis->m_listUIChatMessage.begin();
	// 도킹이 가능한지 체크
	for( ; Iter != pThis->m_listUIChatMessage.end() ; ++Iter )
	{
		INT		nLength	=	0;
		// 도킹전 이벤트가 적용 되어 있을수도 있으니 다 종료 시킨다
		pThis->_DockingPreEventEnd( (*Iter) );

		if( (*Iter)->IsUsing() && (*Iter) != pSrcUI )
		{
			// 도킹이 가능한 위치에 있는지 체크
			if( pThis->_DockingCheck( pSrcUI , (*Iter) , nLength ) )
			{
				if( nMinLength > nLength )
				{
					nMinLength	=	nLength;
					pDstUI		=	(*Iter);
				}
			}
		}
	}

	if( pDstUI )
	{
		stDockingEvent stEvent;

		stEvent.m_eEventType	=	EVENT_DOCKING_TYPE_COMBINE;
		stEvent.m_dwStartTime	=	g_pEngine->GetClockCount();
		stEvent.m_dwDuration	=	300;
		stEvent.m_pDataSrc		=	pSrcUI;
		stEvent.m_pDataDst		=	pDstUI;

		if( pSrcUI->GetDockingEvent() || pDstUI->GetDockingEvent() )
			return;

		pSrcUI->SetDockingEvent( TRUE );
		pDstUI->SetDockingEvent( TRUE );
		
		pSrcUI->AllDeleteListItem();

		pThis->AddDockingEvent( stEvent );

	}

}

VOID	AgcChatManager::CBDockingPreEvent( PVOID pClass , PVOID pData1 , PVOID pData2 )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	AgcUIChatMessage*	pSrcUI	=	static_cast< AgcUIChatMessage* >(pData1);
	if( !pThis || !pData1 )
		return;

	INT					nMinLength	=	999999;
	AgcUIChatMessage*	pDstUI		=	NULL;

	ListUIChatMessageIter	Iter	=	pThis->m_listUIChatMessage.begin();
	// 도킹이 가능한지 체크
	for( ; Iter != pThis->m_listUIChatMessage.end() ; ++Iter )
	{
		INT		nLength	=	0;
		if( (*Iter)->IsUsing() && (*Iter) != pSrcUI )
		{
			pThis->_DockingPreEventEnd( (*Iter) );

			// 도킹이 가능한 위치에 있는지 체크
			if( pThis->_DockingCheck( pSrcUI , (*Iter) , nLength ) )
			{
				if( nMinLength > nLength  )
				{
					pDstUI		=	(*Iter);
					nMinLength	=	nLength;
				}
			}
		}
	}

	if( pDstUI )
		pThis->_DockingPreEventStart( pDstUI );
}


BOOL		AgcChatManager::CBScrollRefresh( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 )
{
	AgcChatManager*		pThis			=	static_cast< AgcChatManager* >(pClass);
	if( !pClass )
		return FALSE;

	AgcUIChatMessage*	pUIChatMessage	=	static_cast< AgcUIChatMessage* >(pData1);
	if( !pUIChatMessage )
		return FALSE;

	pUIChatMessage->Refresh();

	return TRUE;
}

BOOL		AgcChatManager::CBResizeEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcChatManager*	pThis			=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	PVOID*	ppvData	=	(PVOID*)pData1;

	AgcUIChatMessage*	pUIChatMessage	=	static_cast< AgcUIChatMessage* >(ppvData[0]);
	if( !pUIChatMessage )
		return FALSE;

	INT		nSizeX	=	(INT)pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.x;
	INT		nSizeY	=	(INT)pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.y;
	RwV2d&	vPos	=	pUIChatMessage->GetUIPos();

	nSizeX	=	nSizeX	-	(INT)vPos.x;
	nSizeY	=	nSizeY	-	(INT)vPos.y;

	if( nSizeX < MIN_CHAT_SIZE_X )
		nSizeX	=	MIN_CHAT_SIZE_X;

	//else if( nSizeX > MAX_CHAT_SIZE_X )
	//	nSizeX	=	MAX_CHAT_SIZE_X;

	if( nSizeY < MIN_CHAT_SIZE_Y )
		nSizeY	=	MIN_CHAT_SIZE_Y;

	//else if( nSizeY > MAX_CHAT_SIZE_Y )
	//	nSizeY	=	MAX_CHAT_SIZE_Y;

	pUIChatMessage->MoveChatWindow( (INT32)vPos.x  , (INT32)vPos.y , nSizeX , nSizeY );

	pUIChatMessage->SetListItemMaxCount();
	pUIChatMessage->Refresh( TRUE );

	pThis->m_bIsNeedSave	=	TRUE;

	return TRUE;
}

BOOL	AgcChatManager::CBMoveEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcChatManager*	pThis			=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	PVOID*	ppvData	=	(PVOID*)pData1;

	AgcUIChatMessage*	pUIChatMessage	=	static_cast< AgcUIChatMessage* >(ppvData[0]);
	if( !pUIChatMessage )
		return FALSE;

	AcUIEventButton*	pEventButton	=	static_cast< AcUIEventButton* >(pButton);
	if( !pEventButton )
		return FALSE;

	INT		nPosX	=	(INT)(pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.x - pEventButton->GetDragStartPos().x);
	INT		nPosY	=	(INT)(pThis->m_pcsAgcmUIManager2->m_v2dCurMousePos.y - pEventButton->GetDragStartPos().y);
	
	pUIChatMessage->MoveChatWindow( nPosX , nPosY );

	pThis->m_bIsNeedSave	=	TRUE;

	return TRUE;
}

BOOL	AgcChatManager::CBSetupEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcChatManager*	pThis			=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	AgcUIChatMessage*	pUIChatMessage	=	static_cast< AgcUIChatMessage* >(pData1);
	if( !pUIChatMessage )
		return FALSE;

	pUIChatMessage->OpenSetupMenu();

	return TRUE;
}

BOOL	AgcChatManager::CBTabDeleteEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu )
{
	AgcChatManager*		pThis			=	static_cast< AgcChatManager* >(pClass);
	AgcChatTab*			pChatTab		=	static_cast< AgcChatTab* >(pData1);
	AgcUIChatMessage*	pUIChatMessage	=	static_cast< AgcUIChatMessage* >(pData2);
	if( !pThis || !pChatTab || !pUIChatMessage )
		return FALSE;

	pUIChatMessage->DelTab( pChatTab->GetTabName() );
	pUIChatMessage->Refresh( TRUE );

	INT	nCount	=	pUIChatMessage->GetTabCount();
	if( !nCount )
	{
		// 탭이 하나도 없으면 지울 목록에 넣는다.
		pThis->m_listDeleteUI.push_back( pUIChatMessage );

	}

	pThis->m_bIsNeedSave = TRUE;

	return TRUE;
}

BOOL	AgcChatManager::CBTabEditEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu )
{
	AgcChatManager*		pThis			=	static_cast< AgcChatManager* >(pClass);
	AgcChatTab*			pChatTab		=	static_cast< AgcChatTab* >(pData1);
	AgcUIChatMessage*	pUIChatMessage	=	static_cast< AgcUIChatMessage* >(pData2);
	if( !pThis || !pChatTab || !pUIChatMessage )
		return FALSE;

	AgcmUIManager2*	pcmUIManager	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !pcmUIManager )
		return FALSE;

	pThis->m_pOperationChatUI	=	pUIChatMessage;
	pThis->m_pOperationChatTab	=	pChatTab;

	pcmUIManager->ThrowEvent( pThis->m_lTabOptionEditOpenEvent );

	return TRUE;
}


BOOL	AgcChatManager::CBAddTab( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu )
{
	AgcChatManager*		pThis			=	static_cast< AgcChatManager* >(pClass);
	AgcUIChatMessage*	pUIChatMessage	=	static_cast< AgcUIChatMessage* >(pData1);
	if( !pThis || !pUIChatMessage )
		return FALSE;

	AgcmUIManager2*	pcmUIManager	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !pcmUIManager )
		return FALSE;

	pThis->m_pOperationChatUI	=	pUIChatMessage;
	pThis->m_pOperationChatTab	=	NULL;

	pcmUIManager->ThrowEvent( pThis->m_lTabOptionAddOpenEvent );

	return TRUE;
}

VOID AgcChatManager::_ParseMessageData( AgpdChatData* pChatData )
{
	if( !pChatData )
		return;

	CHAR*		pNext		=	NULL;
	CHAR*		pToken1		=	NULL;
	CHAR*		pToken2		=	NULL;

	static CHAR	szMessage	[ MAX_PATH ];
	static CHAR szSender	[ MAX_PATH ];

	ZeroMemory( szMessage , MAX_PATH );
	ZeroMemory( szSender , MAX_PATH );

	pToken1	=	strtok_s( pChatData->szMessage , " : " , &pNext );
	pToken2	=	strtok_s( NULL , " : " , &pNext );

	if( pToken1 && pToken2 )
	{
		sprintf_s( szSender , MAX_PATH , "%s" , pToken1 );
		sprintf_s( szMessage , MAX_PATH , "%s" , pToken2 );

		pChatData->szSenderName		=	szSender;
		pChatData->szMessage		=	szMessage;
		pChatData->lMessageLength	=	strlen(szMessage);
	}
}