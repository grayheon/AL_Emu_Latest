#include "AgcChatManager.h"

#include "AgcmUIManager2.h"
#include "AgpmChatting.h"
#include "AgpmAreaChatting.h"

#include "AgcmUIChatting2.h"
#include "AgcmMinimap.h"
#include "AgcmUIItem.h"
#include "AgcmItem.h"
#include "AgcmCharacter.h"

#include "AgcmTextBoardMng.h"

#include "AgcmUINotice.h"

#include "AgcUIChatMessage.h"
#include "AgcChatMessage.h"

#include "AuStrTable.h"

AgcChatManager::AgcChatManager( void )
{
	m_bIsReady				=	FALSE;
	m_pUIWhisperEventMap	=	NULL;
	m_bIsNeedSave			=	TRUE;
	m_bIsIdleStart			=	FALSE;

	m_eTabOperation			=	E_TAB_OPTION_ADD;
	m_pOperationChatUI		=	NULL;
	m_pOperationChatTab		=	NULL;
	m_ulOperationFlag		=	0;

	m_lTabOptionAddOpenEvent	=	0;
	m_lTabOptionEditOpenEvent	=	0;
	m_lTabOptionUIClose			=	0;

	m_pcsDisplayUserData		=	NULL;
	m_pcsAllDisplayUserData		=	NULL;

	ZeroMemory( m_arrTabFlag , sizeof(m_arrTabFlag) );
	
}

AgcChatManager::~AgcChatManager( void )
{
	OnClearFilter( TRUE );
	OnClearFilter( FALSE );
}

AgcUIChatMessage*	AgcChatManager::CreateUIChatMessage( VOID )
{
	AgcUIChatMessage*	pUIChatMessage	=	new AgcUIChatMessage;
	if( !pUIChatMessage )
		return NULL;

	AgcdUI*		pUI	=		m_pcsAgcmUIManager2->CreateUI();
	if( !pUI )
	{
		DEF_SAFEDELETE( pUIChatMessage );
		return NULL;
	}

	pUIChatMessage->SetIndex( m_listUIChatMessage.size() );

	static INT	nIndex	=	0;

	sprintf_s( pUI->m_szUIName , AGCDUIMANAGER2_MAX_NAME , "ChatUI_%d" , nIndex++ );

	pUI->m_pcsUIWindow->m_Property.bMovable	=	FALSE;
	pUI->m_pcsUIWindow->m_Property.bUseInput=	FALSE;
	pUI->m_bAutoClose						=	FALSE;

	m_pcsAgcmUIManager2->AddUI( pUI );

	pUIChatMessage->SetManager( this );
	pUIChatMessage->Initialize( pUI );

	pUI->m_pcsUIWindow->SetDockingCallback( this , pUIChatMessage , NULL , CBDockingCallback );
	pUI->m_pcsUIWindow->SetDockingPreCallback( this , pUIChatMessage , NULL , CBDockingPreEvent );
	pUI->m_pcsUIWindow->SetRenderRGBAColor( 0x50000000 );

	return pUIChatMessage;
}

BOOL	AgcChatManager::AddUIChatMessage( AgcUIChatMessage* pUIChatMessage )
{
	ListUIChatMessageIter	Iter	=	find( m_listUIChatMessage.begin() , m_listUIChatMessage.end() , pUIChatMessage );
	if( Iter != m_listUIChatMessage.end() )
		return FALSE;

	m_listUIChatMessage.push_back( pUIChatMessage );

	return TRUE;
}

BOOL	AgcChatManager::DestroyUIChatMessage( AgcUIChatMessage* pUIChatMessage )
{
	ListUIChatMessageIter	Iter	=	find( m_listUIChatMessage.begin() , m_listUIChatMessage.end() , pUIChatMessage );
	if( Iter == m_listUIChatMessage.end() )
		return FALSE;

	// 리스트에서 지우고
	m_listUIChatMessage.erase( Iter );

	// Event List에 들어있는지 확인해서 지운다.
	ListDockingEventIter	EventIter	=	m_listDockingEvent.begin();
	for( ; EventIter != m_listDockingEvent.end() ; )
	{
		if( (*EventIter).m_pDataSrc == pUIChatMessage || (*EventIter).m_pDataDst == pUIChatMessage )
		{
			m_listDockingEvent.erase( EventIter++ );
		}
		else
		{
			++EventIter;
		}
	}

	// 실제로 인스턴트를 삭제한다.
	DEF_SAFEDELETE( pUIChatMessage );
	

	return TRUE;
}

VOID	AgcChatManager::ChatIdle( DWORD dwTick )
{
	if( !m_bIsIdleStart )
		return;

	// 분가 요청을 한 메세지를 처리한다
	ListUIChatMessageIter	UIIter	=	m_listUIChatMessage.begin();
	for( ; UIIter != m_listUIChatMessage.end() ; ++UIIter )
	{
		if( !(*UIIter)->IsUsing() )
			continue;

		AgcChatMessage*	pMessage	=	(*UIIter)->GetSeparationTab();
		(*UIIter)->SetSeparationTab( NULL );
		if( pMessage )
		{
			stDockingEvent stEvent;

			stEvent.m_eEventType	=	EVENT_DOCKING_TYPE_SEPARATION;
			stEvent.m_dwStartTime	=	g_pEngine->GetClockCount();
			stEvent.m_dwDuration	=	500;
			stEvent.m_pDataSrc		=	(*UIIter);
			stEvent.m_pDataDst		=	SetEmptyUIData( pMessage );

			AddDockingEvent( stEvent );
		}

		RwV2d&	vPos		=	(*UIIter)->GetUIPos();
		RwV2d&	vSize		=	(*UIIter)->GetUISize();
		RwV2d&	vMousePos	=	m_pcsAgcmUIManager2->m_v2dCurMousePos;
		AgcdUI*	pUI			=	(*UIIter)->GetUI();
		BOOL	bEnd		=	FALSE;

		if( vPos.x	<=	vMousePos.x && (vPos.x+vSize.x) >= vMousePos.x )
		{
			if( vPos.y <= vMousePos.y && (vPos.y+vSize.y) >= vMousePos.y )
			{
				if( !(*UIIter)->GetShowEvent() )
				{
					stDockingEvent stEvent;

					stEvent.m_eEventType	=	EVENT_DOCKING_TYPE_IMAGE_SHOW;
					stEvent.m_dwStartTime	=	g_pEngine->GetClockCount();
					stEvent.m_dwDuration	=	500;
					stEvent.m_pDataSrc		=	(*UIIter);
					stEvent.m_pDataDst		=	NULL;
					stEvent.m_fAlpha		=	(*UIIter)->GetAlpha();

					(*UIIter)->SetShowEvent( TRUE );
					(*UIIter)->SetHideEvent( FALSE );

					AddDockingEvent( stEvent );
				}

				bEnd	=	TRUE;
			}
		}

		if( !bEnd )
		{
			if( !(*UIIter)->GetHideEvent() )
			{
				stDockingEvent stEvent;

				stEvent.m_eEventType	=	EVENT_DOCKING_TYPE_IMAGE_HIDE;
				stEvent.m_dwStartTime	=	g_pEngine->GetClockCount();
				stEvent.m_dwDuration	=	500;
				stEvent.m_pDataSrc		=	(*UIIter);
				stEvent.m_pDataDst		=	NULL;
				stEvent.m_fAlpha		=	(*UIIter)->GetAlpha();

				(*UIIter)->SetHideEvent( TRUE );
				(*UIIter)->SetShowEvent( FALSE );

				AddDockingEvent( stEvent );
			}
			
		}

	}


	ListDockingEventIter	Iter	=	m_listDockingEvent.begin();
	for( ; Iter != m_listDockingEvent.end() ; )
	{
		AgcUIChatMessage*	pUIChatSrc	=	static_cast< AgcUIChatMessage* >(Iter->m_pDataSrc);
		AgcUIChatMessage*	pUIChatDst	=	static_cast< AgcUIChatMessage* >(Iter->m_pDataDst);
		if( !pUIChatSrc )
		{
			++Iter;
			continue;
		}

		if( Iter->m_eEventType == EVENT_DOCKING_TYPE_COMBINE  )
		{
			AgcUIWindow*		pWindow		=	pUIChatSrc->GetUI()->m_pcsUIWindow;
			AgcUIWindow*		pDstWindow	=	pUIChatDst->GetUI()->m_pcsUIWindow;
			if( !pWindow || !pDstWindow )
			{	
				++Iter;
				continue;
			}

			if( dwTick <= Iter->m_dwStartTime )
			{
				++Iter;
				continue;
			}

			DWORD				dwRest	=	dwTick	-	Iter->m_dwStartTime;
			FLOAT				fRest	=	((FLOAT)dwRest / Iter->m_dwDuration);
			INT					nHalfX	=	pDstWindow->w / 2;
			INT					nHalfY	=	pDstWindow->h / 2;

			pUIChatSrc->DetachScroll();
			pUIChatSrc->DetachResize();
			pUIChatSrc->DetachMove();
			pUIChatSrc->DetachSetup();
			pUIChatSrc->HideTabButton();

			pWindow->MoveWindow(	(INT32)(pDstWindow->x + (nHalfX * fRest)) , 
									(INT32)(pDstWindow->y + (nHalfY * fRest)) , 
									(INT32)(pDstWindow->w - (pDstWindow->w * fRest) ), 
									(INT32)(pDstWindow->h - (pDstWindow->h * fRest)) 
								);

			if( dwRest > Iter->m_dwDuration )
			{
				// UI를 합친다
				_CombineDockingUI( pUIChatDst , pUIChatSrc );
				_DockingPreEventEnd( pUIChatDst );

				m_listDockingEvent.erase( Iter++ );

				pUIChatDst->SetDockingEvent( FALSE );
				pUIChatSrc->SetDockingEvent( FALSE );

				// 합쳐진 놈은 Idle이 끝난후에 지워버린다.
				m_listDeleteUI.push_back( pUIChatSrc );

				continue;
			}
		}

		else if( Iter->m_eEventType == EVENT_DOCKING_TYPE_SEPARATION )
		{
			AgcUIWindow*		pWindow		=	pUIChatDst->GetUI()->m_pcsUIWindow;
			AgcUIWindow*		pSrcWindow	=	pUIChatSrc->GetUI()->m_pcsUIWindow;
			if( !pWindow || !pSrcWindow )
			{
				++Iter;
				continue;
			}

			if( dwTick <= Iter->m_dwStartTime )
			{
				++Iter;
				continue;
			}

			INT					nStartPosX	=	pSrcWindow->x;
			INT					nStartPosY	=	pSrcWindow->y;

			INT					nPosX		=	(m_pcsAgcmUIManager2->m_lWindowWidth / 2) - (MAX_LISTBOX_STRING / 2 );
			INT					nPosY		=	(m_pcsAgcmUIManager2->m_lWindowHeight / 2) - (MAX_LISTBOX_STRING / 2 );
			DWORD				dwRest		=	dwTick	-	Iter->m_dwStartTime;
			FLOAT				fRest		=	((FLOAT)dwRest / Iter->m_dwDuration);

			INT					nRestX		=	nStartPosX - nPosX;
			INT					nRestY		=	nStartPosY - nPosY;

			pWindow->MoveWindow(	(INT32)(nStartPosX - (nRestX * fRest)) , 
									(INT32)(nStartPosY - (nRestY * fRest)) , 
									(INT32)(MAX_LISTBOX_STRING * fRest) , 
									(INT32)(MAX_LISTBOX_STRING * fRest )
									);

			if( dwRest > Iter->m_dwDuration )
			{
				m_listDockingEvent.erase( Iter++ );
				continue;
			}
		}

		else if( Iter->m_eEventType == EVENT_DOCKING_TYPE_IMAGE_SHOW )
		{
			AgcUIWindow*		pWindow	=	pUIChatSrc->GetUI()->m_pcsUIWindow;
			if( !pWindow )
			{
				++Iter;
				continue;
			}

			if( dwTick <= Iter->m_dwStartTime )
			{
				++Iter;
				continue;
			}

			FLOAT				fOffset		=	(FLOAT)Iter->m_fAlpha;
			DWORD				dwRest		=	dwTick	-	Iter->m_dwStartTime;
			FLOAT				fRest		=	((FLOAT)dwRest / Iter->m_dwDuration);
			FLOAT				fData		=	fOffset + fRest;

			if( dwRest > Iter->m_dwDuration || fData >= 1.0f )
			{
				pUIChatSrc->SetAlpha( 1.0f );
				pUIChatSrc->SetHideEvent( FALSE );

				m_listDockingEvent.erase( Iter++ );
				continue;
			}

			pUIChatSrc->SetAlpha( fData );

		}

		else if( Iter->m_eEventType == EVENT_DOCKING_TYPE_IMAGE_HIDE )
		{
			AgcUIWindow*		pWindow	=	pUIChatSrc->GetUI()->m_pcsUIWindow;
			if( !pWindow )
			{
				++Iter;
				continue;
			}

			if( dwTick <= Iter->m_dwStartTime )
			{
				++Iter;
				continue;
			}

			FLOAT				fOffset		=	(FLOAT)Iter->m_fAlpha;
			DWORD				dwRest		=	dwTick	-	Iter->m_dwStartTime;
			FLOAT				fRest		=	((FLOAT)dwRest / Iter->m_dwDuration);
			FLOAT				fData		=	fOffset - fRest;
			
			if( dwRest > Iter->m_dwDuration || fData <= 0 )
			{
				pUIChatSrc->SetAlpha( 0.0f );
				pUIChatSrc->SetShowEvent( FALSE );

				m_listDockingEvent.erase( Iter++ );
				continue;
			}

			pUIChatSrc->SetAlpha( fData );
		}

		++Iter;
	}

	// 지워야될 UI를 지운다.
	ListUIChatMessageIter	DelIter	=	m_listDeleteUI.begin();
	for( ; DelIter != m_listDeleteUI.end() ; ++DelIter )
	{
		DestroyUIChatMessage( (*DelIter) );
	}

	m_listDeleteUI.clear();

	// 파일 저장이 필요한지 체크해서 저장한다
	static DWORD	dwLastTick	=	dwTick;

	if( dwLastTick+10000 < dwTick )
	{
		if( IsNeedSave() && m_pcsAgcmCharacter->GetSelfCharacter() )
		{
			CHAR	szPath[ MAX_PATH ];
			sprintf_s( szPath , MAX_PATH , "CUSTOMDATA\\%s.SYJ" , m_pcsAgcmCharacter->GetSelfCharacter()->m_szID );

			SaveChatFile( szPath );

			m_bIsNeedSave	=	FALSE;
		}

		dwLastTick	=	dwTick;
	}
}

VOID	AgcChatManager::LoadBaseFile( CONST string& strFileName )
{
	AuXmlParser	Document;

	if( !Document.LoadXMLFile( strFileName ) )
		return;

	m_listChatWindowBase.clear();

	TiXmlNode*	pRootNode	=	Document.FirstChild( "ChatBase" );
	if( !pRootNode )
		return;

	TiXmlNode*	pCreateChat	=	pRootNode->FirstChild( "CreateChat" );
	for( ; pCreateChat ; pCreateChat = pCreateChat->NextSibling( "CreateChat" ) )
	{
		stChatWindowInfo	stChatInfo;

		CONST CHAR*		pCreateChatIndex	=	pCreateChat->ToElement()->Attribute( "Index" );
		if( pCreateChatIndex )
		{
			stChatInfo.m_nIndex	=	atoi( pCreateChatIndex );
		}

		TiXmlNode*	pPositionNode	=	pCreateChat->FirstChild( "Position" );
		if( pPositionNode )
		{
			CONST	CHAR*	pPositionX	=	pPositionNode->ToElement()->Attribute( "X" );
			if( pPositionX )
			{
				stChatInfo.m_vPosition.x	=	(RwReal)(m_pcsAgcmUIManager2->m_lWindowWidth * atof( pPositionX ));
			}

			CONST	CHAR*	pPositionY	=	pPositionNode->ToElement()->Attribute( "Y" );
			if( pPositionY )
			{
				stChatInfo.m_vPosition.y	=	(RwReal)(m_pcsAgcmUIManager2->m_lWindowHeight * atof( pPositionY ));
			}
		}

		TiXmlNode*	pSizeNode		=	pCreateChat->FirstChild( "Size" );
		if( pSizeNode )
		{
			CONST	CHAR*	pSizeWidth	=	pSizeNode->ToElement()->Attribute( "Width" );
			if( pSizeWidth )
			{
				stChatInfo.m_vSize.x	=	(RwReal)atoi(pSizeWidth);
			}

			CONST	CHAR*	pSizeHeight	=	pSizeNode->ToElement()->Attribute( "Height" );
			if( pSizeHeight )
			{
				stChatInfo.m_vSize.y	=	(RwReal)atoi(pSizeHeight);
			}
		}

		TiXmlNode*	pOptionNode		=	pCreateChat->FirstChild( "Option" );
		if( pOptionNode )
		{
			CONST	CHAR*	pFontSize	=	pOptionNode->ToElement()->Attribute( "Font" );
			if( pFontSize )
			{
				stChatInfo.m_unFontSize	=	(UINT)atoi(pFontSize);
			}

			CONST	CHAR*	pTabEnable	=	pOptionNode->ToElement()->Attribute( "TabEnable" );
			if( pTabEnable )
			{
				stChatInfo.m_bTabEnable	=	(BOOL)atoi(pTabEnable);
			}
		}

		TiXmlNode*	pTabNode		=	pCreateChat->FirstChild( "Tab" );
		for( INT i = 0 ; pTabNode && i < MAX_TAB_BUTTON ; pTabNode = pTabNode->NextSibling( "Tab" ) , ++i ) 
		{
			CONST	CHAR*	pTabName	=	pTabNode->ToElement()->Attribute( "Name" );
			if( pTabName )
			{
				stChatInfo.m_strTabName[i]	=	pTabName;
			}

			CONST	CHAR*	pTabMsgType	=	pTabNode->ToElement()->Attribute( "MsgType"  );
			if( pTabMsgType )
			{
				stChatInfo.m_ulTabType[i]	=	_ParseMessageText( pTabMsgType );
			}

			stChatInfo.m_bPrimary[i]	=	TRUE;		// 베이스 파일에서 읽어온 탭은 무조건 Primary 속성을 가진다.
		}

		m_listChatWindowBase.push_back( stChatInfo );
	}
	
	Document.Clear();
}

VOID	AgcChatManager::LoadChatFile( CONST string& strFileName )
{
	// 해당 파일이 없다면 기본 세팅을 한다
	AuXmlParser	Document;

	CreateDirectory("CUSTOMDATA", NULL);

	if( Document.LoadXMLFile( strFileName ) )
	{
		AllClearChatUI();

		TiXmlNode*	pRootNode	=	Document.FirstChild( "ChatBase" );
		if( !pRootNode )
			return;

		TiXmlNode*	pCreateChat	=	pRootNode->FirstChild( "CreateChat" );
		for( ; pCreateChat ; pCreateChat = pCreateChat->NextSibling( "CreateChat" ) )
		{
			stChatWindowInfo	stChatInfo;

			CONST		CHAR*		pChatIndex	=	pCreateChat->ToElement()->Attribute( "Index"  );
			if( pChatIndex )
			{
				stChatInfo.m_nIndex	=	atoi(pChatIndex);
			}


			TiXmlNode*	pPositionNode	=	pCreateChat->FirstChild( "Position" );
			if( pPositionNode )
			{
				CONST	CHAR*	pPositionX	=	pPositionNode->ToElement()->Attribute( "X" );
				if( pPositionX )
				{
					stChatInfo.m_vPosition.x	=	(RwReal)(m_pcsAgcmUIManager2->m_lWindowWidth * atof( pPositionX ));
				}

				CONST	CHAR*	pPositionY	=	pPositionNode->ToElement()->Attribute( "Y" );
				if( pPositionY )
				{
					stChatInfo.m_vPosition.y	=	(RwReal)(m_pcsAgcmUIManager2->m_lWindowHeight * atof( pPositionY ));
				}
			}

			TiXmlNode*	pSizeNode		=	pCreateChat->FirstChild( "Size" );
			if( pSizeNode )
			{
				CONST	CHAR*	pSizeWidth	=	pSizeNode->ToElement()->Attribute( "Width" );
				if( pSizeWidth )
				{
					stChatInfo.m_vSize.x	=	(RwReal)atoi(pSizeWidth);
				}

				CONST	CHAR*	pSizeHeight	=	pSizeNode->ToElement()->Attribute( "Height" );
				if( pSizeHeight )
				{
					stChatInfo.m_vSize.y	=	(RwReal)atoi(pSizeHeight);
				}
			}

			TiXmlNode*	pOptionNode		=	pCreateChat->FirstChild( "Option" );
			if( pOptionNode )
			{
				CONST	CHAR*	pFontSize	=	pOptionNode->ToElement()->Attribute( "Font" ); 
				if( pFontSize )
				{
					stChatInfo.m_unFontSize	=	(UINT)atoi(pFontSize);
				}

				CONST	CHAR*	pTabEnable	=	pOptionNode->ToElement()->Attribute( "TabEnable" );
				if( pTabEnable )
				{
					stChatInfo.m_bTabEnable	=	(BOOL)atoi(pTabEnable);
				}
			}

			TiXmlNode*	pTabNode		=	pCreateChat->FirstChild( "Tab" );
			for( INT i = 0  ; pTabNode && i < MAX_TAB_BUTTON ; pTabNode = pTabNode->NextSibling( "Tab" ) , ++i )
			{
				CONST	CHAR*	pTabName		=	pTabNode->ToElement()->Attribute( "Name" );
				if( pTabName )
				{
					stChatInfo.m_strTabName[i]	=	pTabName;
				}

				CONST	CHAR*	pMsgType		=	pTabNode->ToElement()->Attribute( "MsgType" );
				if( pMsgType )
				{
					stChatInfo.m_ulTabType[i]		=	_ParseMessageText( pMsgType );
				}
			}

			_LoadChatSetting( stChatInfo );
		}
		
	}
	else
	{
		AllClearChatUI();
	}

	_DefaultChatSetting();

	m_bIsIdleStart	=	TRUE;
}

VOID	AgcChatManager::SaveChatFile( CONST string& strFileName )
{
	AuXmlDocument	Document;
	TiXmlElement	RootNode( "ChatBase" );

	ListUIChatMessageIter	Iter	=	m_listUIChatMessage.begin();
	for( INT i = 0 ; Iter != m_listUIChatMessage.end() ; ++Iter , ++i )
	{
		if( !(*Iter)->IsUsing() )
			continue;

		RwV2d&	vPos	=	(*Iter)->GetUIPos();
		RwV2d&	vSize	=	(*Iter)->GetUISize();

		TiXmlElement	BaseNode( "CreateChat" );
		TiXmlElement	PosNode( "Position" );
		TiXmlElement	SizeNode( "Size" );
		TiXmlElement	OptionNode( "Option" );

		CHAR	szValue[ 50 ];
		
		sprintf_s( szValue , 50 , "%f" , vPos.x / m_pcsAgcmUIManager2->m_lWindowWidth );
		PosNode.SetAttribute( "X" , szValue );

		sprintf_s( szValue , 50 , "%f" , vPos.y / m_pcsAgcmUIManager2->m_lWindowHeight);
		PosNode.SetAttribute( "Y" , szValue  );

		SizeNode.SetAttribute( "Width" , (INT)vSize.x );
		SizeNode.SetAttribute( "Height" , (INT)vSize.y );

		OptionNode.SetAttribute( "Font" , (*Iter)->GetFontSize() );
		OptionNode.SetAttribute( "TabEnable" , (*Iter)->GetTabInput() );

		BaseNode.InsertEndChild( PosNode );
		BaseNode.InsertEndChild( SizeNode );
		BaseNode.InsertEndChild( OptionNode );

		BaseNode.SetAttribute( "Index" , i );

		INT		nCount		=	(*Iter)->GetTabCount();
		for( INT i = 0 ; i < nCount ; ++i )
		{
			TiXmlElement	TabNode( "Tab" );

			string			strTabType;
			string			strTabName;

			AgcChatTab*		pChatTab	=	(*Iter)->GetTab( i );
			if( !pChatTab )
				continue;

			// Primary 속성은 저장하지 않는다.
			if( pChatTab->IsPrimary() )
				continue;

			UINT			ulMsgType	=	pChatTab->GetTabType();

			strTabName	=	pChatTab->GetTabName();
			for( INT k = 0 ; k < CHAT_MESSAGE_COUNT ; ++k )
			{
				CHAR		szValue[ 20 ];
				ZeroMemory( szValue , 20 );

				if( ulMsgType & (1<<k))
				{
					itoa( k , szValue , 10 );
					strTabType	+=	szValue;
					strTabType += "#";
				}
			}

			TabNode.SetAttribute( "Name" , strTabName );
			TabNode.SetAttribute( "MsgType" , strTabType );

			BaseNode.InsertEndChild( TabNode );
		}
	
		RootNode.InsertEndChild( BaseNode );
	}

	Document.InsertEndChild( RootNode );
	Document.SaveFile( strFileName );
}

BOOL AgcChatManager::OnRegisterUIEvent( void )
{
	m_pcsAgcmUIManager2		=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	m_pcsAgcmUIChatting2	=	static_cast< AgcmUIChatting2* >( g_pEngine->GetModule( "AgcmUIChatting2" ) );
	m_pcsAgcmMinimap		=	static_cast< AgcmMinimap* >( g_pEngine->GetModule( "AgcmMinimap" ) );
	m_pcsAgcmUIItem			=	static_cast< AgcmUIItem* >( g_pEngine->GetModule( "AgcmUIItem" ) );
	m_pcsAgcmItem			=	static_cast< AgcmItem* >( g_pEngine->GetModule( "AgcmItem" ) );
	m_pcsAgcmCharacter		=	static_cast< AgcmCharacter* >( g_pEngine->GetModule( "AgcmCharacter" ) );
	m_pcsAgcmTextBoardMng	=	static_cast< AgcmTextBoardMng* >( g_pEngine->GetModule( "AgcmTextBoardMng" ) );
	m_pcsAgcmUINotice		=	static_cast< AgcmUINotice* >( g_pEngine->GetModule( "AgcmUINotice" ) );
	m_pcsAgpmCharacter		=	static_cast< AgpmCharacter* >( g_pEngine->GetModule( "AgpmCharacter" ) );

	return OnRegisterCallBack();
}

BOOL AgcChatManager::OnRegisterCallBack( void )
{
	AgpmChatting*		ppmChatting		= static_cast< AgpmChatting* >( g_pEngine->GetModule( "AgpmChatting" ) );
	AgpmAreaChatting*	ppmAreaChatting = static_cast< AgpmAreaChatting* >( g_pEngine->GetModule( "AgpmAreaChatting" ) );
	AgcmUIManager2*		pcmUIManager	= static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !ppmChatting || !ppmAreaChatting || !pcmUIManager ) 
		return FALSE;

	// 모든 종류의 채팅메세지를 하나의 콜백으로 통합처리한다. ㅡ.ㅡ.. 나눌 이유가 없자나 사실..
	if( !ppmChatting->SetCallbackRecvNormalMsg( CallBack_RecvMsgChatting, this ) ) return FALSE;
	if( !ppmChatting->SetCallbackRecvEmphasisMsg( CallBack_RecvMsgChatting, this ) ) return FALSE;
	if( !ppmAreaChatting->SetCallbackRecvSendMessage( CallBack_RecvMsgChatting, this ) ) return FALSE;
	if( !ppmChatting->SetCallbackRecvWholeWorldMsg( CallBack_RecvMsgChatting, this ) ) return FALSE;
	if( !ppmChatting->SetCallbackRecvWordBalloonMsg( CallBack_RecvMsgChatting, this ) ) return FALSE;
	
	if( !ppmChatting->SetCallbackRecvGuildMsg( CallBack_RecvMsgChatting, this ) ) return FALSE;
	if( !ppmChatting->SetCallbackRecvGuildJointMsg( CallBack_RecvMsgChatting, this ) ) return FALSE;

	if( !ppmChatting->SetCallbackRecvPartyMsg( CallBack_RecvMsgChatting, this ) ) return FALSE;

	if( !ppmChatting->SetCallbackRecvSystemLevel1Msg( CallBack_RecvMsgChatting, this ) ) return FALSE;
	if( !ppmChatting->SetCallbackRecvSystemLevel2Msg( CallBack_RecvMsgChatting, this ) ) return FALSE;
	if( !ppmChatting->SetCallbackRecvSystemLevel3Msg( CallBack_RecvMsgChatting, this ) ) return FALSE;
	if( !pcmUIManager->SetCallbackActionMessageChat( CallBack_RecvMsgChatting, this ) ) return FALSE;

	if( !ppmChatting->SetCallbackRecvWhisperMsg( CallBack_RecvMsgChatting, this ) ) return FALSE;
	if( !ppmChatting->SetCallbackRecvReplyBlockWhisper( CAllBack_WhisperBlock, this ) ) return FALSE;
	if( !ppmChatting->SetCallbackRecvOfflineWhisperTarget( CallBack_WhisperOffline, this ) ) return FALSE;

	pcmUIManager->AddFunction( this , "Chat_OptionOK" , CBOptionOK , 0 );
	pcmUIManager->AddFunction( this , "Chat_FlagCheck" , CBTabOptionFlagCheck , 0 );
	pcmUIManager->AddFunction( this , "Chat_FlagAllCheck" , CBTabAllOptionFlagCheck , 0 );

	pcmUIManager->AddFunction( this , "Chat_InitOpenAddTab" , CBInitOpenAddTab , 0 );
	pcmUIManager->AddFunction( this , "Chat_InitOpenEditTab" , CBInitOpenEditTab , 0 );

	pcmUIManager->AddDisplay( this , "Chat_CheckDisplay" , 0 , CBDisplayCheck , AGCDUI_USERDATA_TYPE_STRING );
	pcmUIManager->AddDisplay( this , "Chat_NameDisplay" , 0 , CBDisplayName , AGCDUI_USERDATA_TYPE_STRING );
	pcmUIManager->AddDisplay( this , "Chat_ColorDisplay" , 0 , CBDisplayColor , AGCDUI_USERDATA_TYPE_STRING );

	pcmUIManager->AddDisplay( this , "Chat_AllCheckDisplay" , 0 , CBDisplayAllCheck , AGCDUI_USERDATA_TYPE_STRING );
	

	m_pcsDisplayUserData	=	pcmUIManager->AddUserData( "Chat_DisplayUserData" , &m_arrTabFlag[0] , sizeof(BOOL) , CHAT_MESSAGE_COUNT , AGCDUI_USERDATA_TYPE_STRING );
	m_pcsAllDisplayUserData	=	pcmUIManager->AddUserData( "Chat_AllDisplayUserData" , &m_Dummy , sizeof(INT) , 1 , AGCDUI_USERDATA_TYPE_STRING );

	m_lTabOptionAddOpenEvent	= pcmUIManager->AddEvent("TabOptionAdd_OpenUI");
	m_lTabOptionEditOpenEvent	= pcmUIManager->AddEvent("TabOptionEdit_OpenUI");
	m_lTabOptionUIClose			= pcmUIManager->AddEvent("TabOption_CloseUI" );

	return TRUE;
}

BOOL AgcChatManager::OnRegisterTextFormat( void )
{
	// 채팅명령어 로딩
	BOOL	bIsEncrypted = TRUE;
#ifdef _BIN_EXEC_
	bIsEncrypted = FALSE;
#endif
	if( !OnRegisterCmdCallBack() ) 
		return FALSE;

	if( !OnLoadChatCmd( "INI\\ChatCmd.xml", bIsEncrypted ) ) 
		return FALSE;

	if( !OnRegisterSystemMsgCallBack() ) 
		return FALSE;

	return TRUE;
}

BOOL AgcChatManager::OnAddMessage( UINT eMsgType , char* pSenderName , char* pMessage , DWORD dwCustomColor , INT32 lSenderCID , UINT32 ulCharType, const char* pTargetName )
{
	if( !m_bIsReady )
	{
		if( !OnCollectControls() ) 
			return FALSE;
	}

	// 시스템 메세지는 필터링하지 않는다.
	if( (eMsgType & CHAT_MESSAGE_NORMAL) || (eMsgType & CHAT_MESSAGE_GUILD) || (eMsgType & CHAT_MESSAGE_PARTY) || (eMsgType & CHAT_MESSAGE_WHISPER) || (eMsgType & CHAT_MESSAGE_ARCHLORD) )
	{
		// 먼저 필터링을 거친다.
		if( !OnFiltering( pMessage ) ) 
			return FALSE;
	}

	stOriginalMessageInfo*	pMessageInfo	=	NULL;

	if( !(ulCharType & AGPMCHAR_TYPE_MONSTER) )
	{
		ListUIChatMessageIter	UIIter	=	m_listUIChatMessage.begin();
		for( ; UIIter != m_listUIChatMessage.end() ; ++UIIter )
		{
			// 해당 타입을 사용하고 있다면 메세지 추가
			if( (*UIIter) && (*UIIter)->IsUsableType( eMsgType) )
			{
				(*UIIter)->AllDeleteListItem();

				pMessageInfo	=		(*UIIter)->AddMessage( eMsgType , pSenderName , pMessage , dwCustomColor , ulCharType );
			}
		}
	}

	switch( eMsgType )
	{
	case CHAT_MESSAGE_NORMAL:
		{
			if( m_pcsAgcmTextBoardMng )
			{
				sBoardAttr	BoardInfo;

				BoardInfo.eType		=	AGCM_BOARD_TEXT;

				if( pMessageInfo)
					BoardInfo.String	=	pMessageInfo->m_strNoEventMessage;

				else if( pMessage && strlen(pMessage) )
					BoardInfo.String	=	pMessage;

				BoardInfo.Color		=	0xffffffff;
				BoardInfo.ViewCount	=	7000;
				BoardInfo.fType		=	0;

				// 몬스터 같은 경우에는 CID로 구분해야 한다.
				if( lSenderCID > 0 )
					m_pcsAgcmTextBoardMng->CreateBoard( lSenderCID , &BoardInfo );

				else if( pMessageInfo )
					m_pcsAgcmTextBoardMng->CreateBoard( (CHAR*)pMessageInfo->m_strSenderName.c_str() , &BoardInfo );
			}
		
		}
		break;

	case CHAT_MESSAGE_NOTICE:
		{
			if( m_pcsAgcmUINotice && pMessageInfo && !pMessageInfo->m_strSenderName.length() )
			{
				m_pcsAgcmUINotice->AddMessage( pMessageInfo->m_strMessage );
			}
		}
		break;

	case CHAT_MESSAGE_WHISPER:
		{
			if( m_pcsAgcmUIChatting2 )
			{
				if( pMessageInfo)
				{
					AgpdCharacter* pSelfCharacter = m_pcsAgcmCharacter->GetSelfCharacter();

					if( pTargetName && 0 != strlen(pTargetName)&& 0 != strcmp(pSelfCharacter->m_szID, pTargetName))
						m_pcsAgcmUIChatting2->SetLastWhisper( pTargetName );
					else
						m_pcsAgcmUIChatting2->SetLastWhisper( pMessageInfo->m_strSenderName.c_str() );
				}
			}
			
		}
		break;
	}
	

	return TRUE;
}

BOOL	AgcChatManager::AddDockingEvent( stDockingEvent& stEvent )
{
	if( stEvent.m_eEventType == EVENT_DOCKING_TYPE_IMAGE_SHOW || stEvent.m_eEventType == EVENT_DOCKING_TYPE_IMAGE_HIDE )
	{
		ListDockingEventIter	Iter	=	m_listDockingEvent.begin();
		for( ; Iter != m_listDockingEvent.end() ; )
		{
			if( (*Iter).m_pDataSrc == stEvent.m_pDataSrc )
			{
				if( (*Iter).m_eEventType == EVENT_DOCKING_TYPE_IMAGE_SHOW || (*Iter).m_eEventType == EVENT_DOCKING_TYPE_IMAGE_HIDE )
				{
					m_listDockingEvent.erase( Iter++ );
					continue;
				}
			}

			++Iter;
		}
	}

	m_listDockingEvent.push_back( stEvent );

	return TRUE;
}

BOOL AgcChatManager::OnCollectControls( void )
{
	if( m_bIsReady ) 
		return FALSE;

	// 텍스트 포맷 설정
	OnRegisterTextFormat();

	m_bIsReady = TRUE;

	return TRUE;
}

VOID	AgcChatManager::_ItemEvent( INT nID )
{
	if( !m_pcsAgcmUIManager2 || !nID || !m_pcsAgcmUIItem )
		return;

	AgpdItem*	pdItem	=	m_pcsAgcmUIItem->GetTooltipRegisterItem( nID );

	// 해당 아이템이 없다면 서버에 요청한다
	if( !pdItem )
	{
		AgpdCharacter*	pcsSelfCharacter	=	m_pcsAgcmCharacter->GetSelfCharacter();
		if( !pcsSelfCharacter )
			return;

		m_pcsAgcmUIItem->SendChatRegisterItemRequest( nID, 0, pcsSelfCharacter->GetID() );

		return;
	}

	// 있다면 그놈으로 한다
	m_pcsAgcmUIItem->OpenChattingItemTooltip( pdItem );
}

VOID	AgcChatManager::_MapPosEvent( CONST CHAR* szBuffer , INT nX , INT nY , INT nID )
{
	if( !m_pcsAgcmUIManager2 )
		return;

	INT		nStringIndex	=	0;
	CHAR	szString[ MAX_PATH ];
	ZeroMemory( szString , MAX_PATH );

	for( INT i = 0 ; szBuffer[i] != 0 ; ++i )
	{
		if( szBuffer[i] != '<' && szBuffer[i] != '>' )
			szString[ nStringIndex++ ]	=	szBuffer[i];
	}

	m_pcsAgcmMinimap->DestroyMapPosition( nID );
	m_pcsAgcmMinimap->AddMapPosition( szString , nID , nX , nY );
}

UINT	AgcChatManager::_ParseMessageText( CONST CHAR* pMessageText )
{
	if( !pMessageText )
		return 0;

	CHAR	szMessageText[ 500 ];
	CHAR*	pTemp			=	NULL;
	CHAR*	pToken			=	NULL;

	UINT	unMessageType	=	0;

	sprintf_s( szMessageText , 500 , "%s" , pMessageText );

	pToken	=	strtok_s( szMessageText , "#" , &pTemp );
	while( pToken )
	{
		
		unMessageType	+=	1 << atoi(pToken);
		pToken	=	strtok_s( NULL , "#" , &pTemp );
	}

	return unMessageType;

}

VOID	AgcChatManager::_LoadChatSetting( stChatWindowInfo& rInfo )
{

	AgcUIChatMessage*	pUIChatMessage	=	GetUIChatMessage( rInfo.m_nIndex );
	if( !pUIChatMessage )
	{
		pUIChatMessage	=	CreateUIChatMessage();
		if( !pUIChatMessage )
			return;

		pUIChatMessage->InitMessageInstance( this );
		AddUIChatMessage( pUIChatMessage );
	}

	BOOL	bResult	=	FALSE;

	for( INT i = 0 ; i < MAX_TAB_BUTTON ; ++i )
	{
		// 탭이 존재하는 경우
		if( rInfo.m_strTabName[i].length() )
		{
			pUIChatMessage->AddTab( rInfo.m_strTabName[i] , rInfo.m_ulTabType[i] , rInfo.m_bPrimary[i] );

			bResult	=	TRUE;
		}
	}

	pUIChatMessage->Refresh();

	// 탭이 존재하면 만들어준다.
	if( bResult )
	{
		pUIChatMessage->MoveChatWindow( (INT32)rInfo.m_vPosition.x , (INT32)rInfo.m_vPosition.y , (INT32)rInfo.m_vSize.x , (INT32)rInfo.m_vSize.y );
		pUIChatMessage->SetFontSize( rInfo.m_unFontSize );
		pUIChatMessage->SetTabInput( rInfo.m_bTabEnable );

		ShowChatUI( pUIChatMessage );
	}

	// 탭이 존재하지 않는다면 삭제한다.
	else
	{
		DestroyUIChatMessage( pUIChatMessage );
	}

}

VOID	AgcChatManager::_DefaultChatSetting( VOID )
{
	BOOL		bGetUIChat	=	TRUE;

	ListChatWindowInfoIter	Iter	=	m_listChatWindowBase.begin();
	for( ; Iter != m_listChatWindowBase.end() ; ++Iter )
	{
		AgcUIChatMessage*	pUIChatMessage	=	GetUIChatMessage( (*Iter).m_nIndex ); 
		if( !pUIChatMessage )
		{
			pUIChatMessage	=	CreateUIChatMessage();
			if( !pUIChatMessage)
				return;

			pUIChatMessage->InitMessageInstance( this );
			AddUIChatMessage( pUIChatMessage );

			bGetUIChat	=	FALSE;
		}

		BOOL	bResult	=	FALSE;

		for( INT i = 0 ; i < MAX_TAB_BUTTON ; ++i )
		{
			// 탭이 존재하는 경우
			if( Iter->m_strTabName[i].length() )
			{
				pUIChatMessage->AddTab( Iter->m_strTabName[i] , Iter->m_ulTabType[i] , Iter->m_bPrimary[i] );
				AgcChatTab*	pChatTab	=	pUIChatMessage->GetTab(0);
				if( pChatTab )
				{
					pUIChatMessage->SelectTab( pChatTab );
				}

				bResult	=	TRUE;
			}
		}

		pUIChatMessage->Refresh();
		
		if( bResult )
		{
			if( !bGetUIChat )
			{
				pUIChatMessage->MoveChatWindow( (INT)Iter->m_vPosition.x , (INT)Iter->m_vPosition.y , (INT)Iter->m_vSize.x , (INT)Iter->m_vSize.y );
				pUIChatMessage->SetFontSize( Iter->m_unFontSize );
				pUIChatMessage->SetTabInput( Iter->m_bTabEnable );

				ShowChatUI( pUIChatMessage );
			}
		}

		else
		{
			DestroyUIChatMessage( pUIChatMessage );
		}
	}

}

VOID	AgcChatManager::_WhisperEvent( CONST CHAR* szBuffer )
{
	if( !m_pcsAgcmUIManager2 )
		return;

	INT		nStringIndex	=	0;
	CHAR	szString[ MAX_PATH ];
	ZeroMemory( szString , MAX_PATH );

	for( INT i = 0 ; szBuffer[i] != 0 ; ++i )
	{
		if( szBuffer[i] != '[' && szBuffer[i] != ']' )
			szString[ nStringIndex++ ]	=	szBuffer[i];
	}


	if( !m_pUIWhisperEventMap )
	{
		m_pUIWhisperEventMap	=	m_pcsAgcmUIManager2->GetEventMap( "Guild_Open_Chat_UI" );

	}

	if( m_pUIWhisperEventMap )
	{
		m_pcsAgcmUIManager2->ThrowEvent( m_pUIWhisperEventMap );
		m_pcsAgcmUIChatting2->OnReadyAutoReplyWhisper( szString );
		m_pcsAgcmUIChatting2->ActiveChatInput();
	}
}

AgcChatMessage*	AgcChatManager::GetChatMessage( UINT unMessageType )
{
	AgcChatMessage*				pChatMessage	=	NULL;
	MapChatMessage::iterator	Iter			=	m_mapChatMessage.find( unMessageType );
	if( Iter != m_mapChatMessage.end() )
	{
		pChatMessage	=	Iter->second;
	}

	return pChatMessage;
}

AgcUIChatMessage*	AgcChatManager::GetUIChatMessage( INT	nIndex )
{
	AgcUIChatMessage*			pUIChatMessage	=	NULL;
	ListUIChatMessage::iterator	Iter			=	m_listUIChatMessage.begin();

	for( ; Iter != m_listUIChatMessage.end() ; ++Iter )
	{
		if( (*Iter)->GetIndex() == nIndex )
		{
			pUIChatMessage	=	(*Iter);
			break;
		}
	}

	return pUIChatMessage;
}

BOOL		AgcChatManager::_DockingCheck( AgcUIChatMessage* pSrcUI , AgcUIChatMessage* pDstUI , OUT INT& nLength  )
{
	if( !pSrcUI || !pDstUI )
		return FALSE;

	RwV2d&	rSrcPos		=	pSrcUI->GetUIPos();
	RwV2d&	rSrcSize	=	pSrcUI->GetUISize();

	RwV2d&	rDstPos		=	pDstUI->GetUIPos();
	RwV2d&	rDstSize	=	pDstUI->GetUISize();

	RwV2d	vSrcCenter;
	vSrcCenter.x	=	rSrcPos.x + (rSrcSize.x / 2 );
	vSrcCenter.y	=	rSrcPos.y + (rSrcSize.y / 2 );

	RwV2d	vDstCenter;
	vDstCenter.x	=	rDstPos.x + (rDstSize.x / 2 );
	vDstCenter.y	=	rDstPos.y + (rDstSize.y / 2 );

	INT	nRestX		=	abs( (INT)(vSrcCenter.x - vDstCenter.x) );
	INT	nRestY		=	abs( (INT)(vSrcCenter.y - vDstCenter.y) );

	if( nRestX <= 100 && nRestY <= 100 )
	{
		nLength	=	(nRestX + nRestY) / 2;

		return TRUE;
	}
		
	return FALSE;
}

VOID		AgcChatManager::_CombineDockingUI( AgcUIChatMessage* pSrcUI , AgcUIChatMessage* pDstUI )
{
	if( !pSrcUI || !pDstUI )
		return;

	INT		nCount	=	pDstUI->GetTabCount();
	for( INT i = 0 ; i < nCount ; ++i )
	{
		AgcChatTab*		pChatTab			=	pDstUI->GetTab( i );
		if( pChatTab )
		{
			pSrcUI->AddTab( pChatTab->GetTabName() , pChatTab->GetTabType() , pChatTab->IsPrimary() );
		}
	}

	// 갱신
	pSrcUI->SetFontSize( pSrcUI->GetFontSize() , TRUE );
}

VOID		AgcChatManager::_DockingPreEventStart( AgcUIChatMessage* pDstUI )
{
	if( !pDstUI || !pDstUI->GetUI() )
		return;

	// 파랑
	pDstUI->GetUI()->m_pcsUIWindow->SetRenderRGBAColor( 0x500000ff );
}

VOID		AgcChatManager::_DockingPreEventEnd( AgcUIChatMessage* pDstUI )
{
	if( !pDstUI || !pDstUI->GetUI() )
		return;

	// 블랙
	pDstUI->GetUI()->m_pcsUIWindow->SetRenderRGBAColor( 0x50000000 );
}

VOID		AgcChatManager::ShowChatUI( AgcUIChatMessage* pUIChatMessage )
{
	if( !pUIChatMessage )
		return;

	AgcdUI*	pUI	=	pUIChatMessage->GetUI();
	if( pUI )
	{
		pUIChatMessage->AttachScroll();
		pUIChatMessage->AttachResize();
		pUIChatMessage->AttachMove();
		pUIChatMessage->AttachSetup();
		pUIChatMessage->ShowTabButton();

		pUIChatMessage->SetScrollValue( 1.0f );

		pUIChatMessage->Refresh( TRUE );

		m_pcsAgcmUIManager2->OpenUI( pUI , FALSE , FALSE , FALSE  );
	}
}

VOID		AgcChatManager::HideChatUI( AgcUIChatMessage* pUIChatMessage )
{
	if( !pUIChatMessage )
		return;

	AgcdUI*	pUI	=	pUIChatMessage->GetUI();
	if( pUI )
	{
		pUIChatMessage->DetachScroll();
		pUIChatMessage->DetachResize();
		pUIChatMessage->DetachMove();
		pUIChatMessage->DetachSetup();
		pUIChatMessage->HideTabButton();

		m_pcsAgcmUIManager2->CloseUI( pUI , FALSE , FALSE , FALSE , FALSE );
	}

}

VOID		AgcChatManager::AllForceDetailShow( VOID )
{
	ListUIChatMessageIter	Iter	=	m_listUIChatMessage.begin();
	for( ; Iter != m_listUIChatMessage.end() ; ++Iter )
	{
		(*Iter)->ForceDetailShow();
	}
}

VOID		AgcChatManager::AllForceDetailHide( VOID )
{
	ListUIChatMessageIter	Iter	=	m_listUIChatMessage.begin();
	for( ; Iter != m_listUIChatMessage.end() ; ++Iter )
	{
		(*Iter)->ForceDetailHide();
	}
}

VOID		AgcChatManager::AllShowChatUI( VOID )
{

	ListUIChatMessageIter	Iter	=	m_listUIChatMessage.begin();
	for( ; Iter != m_listUIChatMessage.end() ; ++Iter )
	{
		ShowChatUI( (*Iter) );
	}

}

VOID		AgcChatManager::AllHideChatUI( VOID )
{

	ListUIChatMessageIter	Iter	=	m_listUIChatMessage.begin();
	for( ; Iter != m_listUIChatMessage.end() ; ++Iter )
	{
		HideChatUI( (*Iter) );
	}

}

VOID		AgcChatManager::AllClearChatUI( VOID )
{
	ListUIChatMessageIter	Iter	=	m_listUIChatMessage.begin();
	for( ; Iter != m_listUIChatMessage.end() ; ++Iter )
	{
		ListDockingEventIter	EventIter	=	m_listDockingEvent.begin();
		for( ; EventIter != m_listDockingEvent.end() ; )
		{
			if( (*EventIter).m_pDataSrc == (*Iter) || (*EventIter).m_pDataDst == (*Iter) )
			{
				m_listDockingEvent.erase( EventIter++ );
			}
			else
			{
				++EventIter;
			}
		}
		// 실제로 인스턴트를 삭제한다.
		DEF_SAFEDELETE( (*Iter)  );
	}

	m_listUIChatMessage.clear();
}

VOID		AgcChatManager::AllClearMessage( VOID )
{
	MapChatMessageIter	Iter	=	m_mapChatMessage.begin();
	for( ; Iter != m_mapChatMessage.end() ; ++Iter )
	{
		Iter->second->ClearOriginalMessage();
	}
}

AgcUIChatMessage*		AgcChatManager::SetEmptyUIData( AgcChatMessage* pChatMessage , AgcUIChatMessage* pUITarget )
{
	if( !pChatMessage )
		return NULL;

	AgcUIChatMessage*	pUIChatMessage	=	pUITarget;
	if( !pUIChatMessage )
		pUIChatMessage	=	GetEmptyUIData();

	if( !pUIChatMessage )
		return NULL;

	//pUIChatMessage->AddMessageInstance( pChatMessage );
	
	return NULL;
}

AgcUIChatMessage*		AgcChatManager::GetEmptyUIData( VOID )
{
	ListUIChatMessageIter	Iter	=	m_listUIChatMessage.begin();
	for( ; Iter != m_listUIChatMessage.end() ; ++Iter )
	{
		if( !(*Iter)->IsUsing() )
		{
			return (*Iter);
		}
	}

	return NULL;
}

AgcUIChatMessage*		AgcChatManager::_CopyUIChatMessage( AgcUIChatMessage* pUIChatMessage )
{
	if( !pUIChatMessage )
		return NULL;

	AgcUIChatMessage*	pUIDst	=	CreateUIChatMessage();
	if( !pUIDst )
		return NULL;

	if( pUIDst )
	{
		pUIDst->InitMessageInstance( this );
		pUIDst->Refresh();
	}

	RwV2d&	vSize	=	pUIChatMessage->GetUISize();
	RwV2d&	vPos	=	pUIChatMessage->GetUIPos();

	pUIDst->GetUI()->m_pcsUIWindow->MoveWindow( (INT32)vPos.x , (INT32)vPos.y , (INT32)vSize.x , (INT32)vSize.y );
	pUIDst->SetFontSize( pUIChatMessage->GetFontSize() , FALSE );
	pUIDst->SetTabInput( TRUE );

	return pUIDst;

}

BOOL		AgcChatManager::CBOptionOK( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl )
{
	AgcChatManager*		pThis			=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	AgcdUIControl*		pEditControl	=	static_cast< AgcdUIControl* >(pData1);
	if( !pEditControl )
		return FALSE;

	AgcmUIManager2*		pcmUIManager2	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !pcmUIManager2 )
		return FALSE;

	// 탭 이름이 없으면 실패다.
	CONST	CHAR*		pText			=	((AcUIEdit*)pEditControl->m_pcsBase)->GetText();
	if( !pText || !strlen(pText) )
	{
		CHAR*		pMessage	=	pcmUIManager2->GetUIMessage( "TabName_NotFound" );
		if( pMessage )
		{
			pcmUIManager2->ActionMessageOKDialog( pMessage );
		}
		
		return FALSE;
	}

	if( pThis->m_eTabOperation == E_TAB_OPTION_ADD )
	{
		// 중복된 탭 이름은 넣을 수 없다.
		ListUIChatMessageIter	Iter	=	pThis->m_listUIChatMessage.begin();
		for( ; Iter != pThis->m_listUIChatMessage.end() ; ++Iter )
		{
			INT		nTabCount	=	(*Iter)->GetTabCount();
			for( INT i = 0 ; i < nTabCount ; ++i )
			{
				AgcChatTab*		pTab	=	(*Iter)->GetTab(i);
				if( pTab )
				{
					if( pTab->GetTabName() == pText )
					{
						CHAR*		pMessage	=	pcmUIManager2->GetUIMessage( "TabName_Duplicate" );
						if( pMessage )
						{
							pcmUIManager2->ActionMessageOKDialog( pMessage );
						}

						return FALSE;
					}
				}
			}
		}
	}

	// 탭 옵션 처리 작업
	switch( pThis->m_eTabOperation )
	{
	case E_TAB_OPTION_ADD:
		pThis->_AddOptionTab( ((AcUIEdit*)pEditControl->m_pcsBase)->GetText() );
		break;

	case E_TAB_OPTION_EDIT:
		pThis->_EditOptionTab( ((AcUIEdit*)pEditControl->m_pcsBase)->GetText() );
		break;

	}

	if( pThis->m_pOperationChatUI )
	{
		pThis->m_pOperationChatUI->Refresh();
	}

	pThis->m_bIsNeedSave	=	TRUE;
	pcmUIManager2->ThrowEvent( pThis->m_lTabOptionUIClose );
	
	return TRUE;
}

BOOL		AgcChatManager::CBTabOptionFlagCheck( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->m_arrTabFlag[ pcsSourceControl->m_lUserDataIndex ]	=	!pThis->m_arrTabFlag[ pcsSourceControl->m_lUserDataIndex ];

	if( pThis->m_arrTabFlag[ pcsSourceControl->m_lUserDataIndex ] )
	{
		pcsSourceControl->m_pcsBase->SetStatus( pcsSourceControl->m_pcsBase->m_astStatus[0].m_lStatusID );
	}

	else
	{
		pcsSourceControl->m_pcsBase->SetStatus( pcsSourceControl->m_pcsBase->m_astStatus[1].m_lStatusID );
	}

	AgcmUIManager2*		pcmUIManager2	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !pcmUIManager2 )
		return FALSE;

	pcmUIManager2->SetUserDataRefresh( pThis->m_pcsAllDisplayUserData );

	return TRUE;
}

BOOL		AgcChatManager::CBTabAllOptionFlagCheck( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;


	BOOL	bTrue	=	FALSE;

	// 체크가 다 되어 있는지 체크한다.
	for( INT i = 0 ; i < CHAT_MESSAGE_COUNT ; ++i )
	{
		if( !pThis->m_arrTabFlag[i] )
		{
			bTrue	=	TRUE;
			break;
		}
	}

	for( INT i = 0 ; i < CHAT_MESSAGE_COUNT ; ++i )
	{
		// 하나 라도 체크가 안되어 있으면 모두 체크한다.
		if( bTrue )
			pThis->m_arrTabFlag[i]	=	TRUE;

		// 다 체크되어 있다면 뺀다
		else
			pThis->m_arrTabFlag[i]	=	FALSE;
		
	}

	AgcmUIManager2*		pcmUIManager2	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !pcmUIManager2 )
		return FALSE;

	pThis->m_pcsDisplayUserData->m_stUserData.m_lCount = CHAT_MESSAGE_COUNT;
	pcmUIManager2->SetUserDataRefresh( pThis->m_pcsDisplayUserData );
	pcmUIManager2->SetUserDataRefresh( pThis->m_pcsAllDisplayUserData );

	return TRUE;
}

BOOL		AgcChatManager::CBDisplayCheck( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	if( !pThis || !pcsSourceControl )
		return FALSE;

	if( pcsSourceControl->m_lUserDataIndex >= CHAT_MESSAGE_COUNT )
		return FALSE;

	if( pThis->m_arrTabFlag[ pcsSourceControl->m_lUserDataIndex ] )
	{
		pcsSourceControl->m_pcsBase->SetStatus( pcsSourceControl->m_pcsBase->m_astStatus[0].m_lStatusID );
	}

	else
	{
		pcsSourceControl->m_pcsBase->SetStatus( pcsSourceControl->m_pcsBase->m_astStatus[1].m_lStatusID );
	}

	return TRUE;
}

BOOL		AgcChatManager::CBDisplayAllCheck( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	if( !pThis || !pcsSourceControl )
		return FALSE;

	BOOL	bTrue	=	FALSE;

	// 체크가 다 되어 있는지 체크한다.
	for( INT i = 0 ; i < CHAT_MESSAGE_COUNT ; ++i )
	{
		if( !pThis->m_arrTabFlag[i] )
		{
			bTrue	=	TRUE;
			break;
		}
	}

	if( !bTrue )
		pcsSourceControl->m_pcsBase->SetStatus( pcsSourceControl->m_pcsBase->m_astStatus[0].m_lStatusID );

	else
		pcsSourceControl->m_pcsBase->SetStatus( pcsSourceControl->m_pcsBase->m_astStatus[1].m_lStatusID );


	return TRUE;
}

BOOL		AgcChatManager::CBDisplayName( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	string	strDisplayName	=	pThis->_GetDisplayName( pcsSourceControl->m_lUserDataIndex );
	DWORD		dwColor		=	pThis->_GetDisplayColor( pcsSourceControl->m_lUserDataIndex );

	pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor	=	dwColor;

	sprintf( szDisplay , "%s" , strDisplayName.c_str() );

	return TRUE;
}

BOOL		AgcChatManager::CBDisplayColor( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	pcsSourceControl->m_pcsBase->SetDefaultRenderType( UICM_BASE_RGBA_RENDER );
	pcsSourceControl->m_pcsBase->SetRenderRGBAColor( pThis->_GetDisplayColor( pcsSourceControl->m_lUserDataIndex ) );

	return TRUE;
}

VOID		AgcChatManager::_AddOptionTab( CONST CHAR*	szTabName )
{

	UINT	ulOperationFlag	=	0;
	for( INT i = 0 ; i < CHAT_MESSAGE_COUNT ; ++i )
	{
		if( m_arrTabFlag[i] )
		{
			ulOperationFlag |=	(1<<i);
		}
	}

	if( m_pOperationChatUI )
		m_pOperationChatUI->AddTab( szTabName , ulOperationFlag );
}

VOID		AgcChatManager::_EditOptionTab( CONST CHAR* szTabName )
{
	UINT	ulOperationFlag	=	0;
	for( INT i = 0 ; i < CHAT_MESSAGE_COUNT ; ++i )
	{
		if( m_arrTabFlag[i] )
		{
			ulOperationFlag |=	(1<<i);
		}
	}

	if( m_pOperationChatUI && m_pOperationChatTab )
	{
		m_pOperationChatTab->SetTabName( szTabName );
		m_pOperationChatTab->SetTabFlag( ulOperationFlag );
	}
}


string		AgcChatManager::_GetDisplayName( INT lIndex )
{

	string		strReturn;

	switch( 1<<lIndex )
	{
	case CHAT_MESSAGE_NORMAL:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_NORMAL);
		break;

	case CHAT_MESSAGE_GUILD:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_GUILD );
		break;

	case CHAT_MESSAGE_PARTY:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_PARTY);
		break;

	case CHAT_MESSAGE_WHISPER:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_WHISPER);
		break;

	case CHAT_MESSAGE_SYSTEM:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_SYSTEM);
		break;

// 	case CHAT_MESSAGE_SYSTEM2:
// 		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_SYSTEM2);
// 		break;
// 
// 	case CHAT_MESSAGE_SYSTEM3:
// 		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_SYSTEM3);
// 		break;

	case CHAT_MESSAGE_NOTICE:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_NOTICE);
		break;

	case CHAT_MESSAGE_QUEST:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_QUEST);
		break;

	case CHAT_MESSAGE_SIEGE:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_SIEGE);
		break;

// 	case CHAT_MESSAGE_DAMAGEFROM:
// 		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_DAMAGEFROM);
// 		break;
// 
// 	case CHAT_MESSAGE_DAMAGETO:
// 		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_DAMAGETO);
// 		break;

	case CHAT_MESSAGE_ARCHLORD:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_ARCHLORD);
		break;
	case CHAT_MESSAGE_SHOUT:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_SHOUT);
		break;
	case CHAT_MESSAGE_RACE:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_RACE);
		break;
	case CHAT_MESSAGE_ELEMENTAL:
		strReturn	=	ClientStr().GetStr(STI_TAB_BUTTON_NAME_ALL);
		break;
	}

	return strReturn;
}

DWORD	AgcChatManager::_GetDisplayColor( INT lIndex )
{
	DWORD		dwReturn = 0xffffffff;

	AgcmResourceLoader*		pLoader	=	static_cast< AgcmResourceLoader* >(g_pEngine->GetModule( "AgcmResourceLoader" ));
	if( !pLoader )
		return dwReturn;

	switch( 1<<lIndex )
	{
	case CHAT_MESSAGE_NORMAL:
		dwReturn	=	pLoader->GetColor( "NORMAL_CHAT" );
		break;

	case CHAT_MESSAGE_GUILD:
		dwReturn	=	pLoader->GetColor( "GUILD_CHAT" );
		break;

	case CHAT_MESSAGE_PARTY:
		dwReturn	=	pLoader->GetColor( "PARTY_CHAT" );
		break;

	case CHAT_MESSAGE_WHISPER:
		dwReturn	=	pLoader->GetColor( "WHISPER_SEND" );
		break;

	case CHAT_MESSAGE_SYSTEM:
		dwReturn	=	pLoader->GetColor( "SYSTEM_CHAT1" );
		break;

// 	case CHAT_MESSAGE_SYSTEM2:
// 		dwReturn	=	pLoader->GetColor( "SYSTEM_CHAT2" );
// 		break;
// 
// 	case CHAT_MESSAGE_SYSTEM3:
// 		dwReturn	=	pLoader->GetColor( "SYSTEM_CHAT3" );
// 		break;

	case CHAT_MESSAGE_NOTICE:
		dwReturn	=	pLoader->GetColor( "NOTICE_CHAT1" );
		break;

	case CHAT_MESSAGE_QUEST:
		dwReturn	=	pLoader->GetColor( "QUEST_CHAT" );
		break;

	case CHAT_MESSAGE_SIEGE:
		dwReturn	=	pLoader->GetColor( "SHRINE_CHAT" );
		break;

// 	case CHAT_MESSAGE_DAMAGEFROM:
// 		dwReturn	=	pLoader->GetColor( "DAMAGEFROM_CHAT" );
// 		break;
// 
// 	case CHAT_MESSAGE_DAMAGETO:
// 		dwReturn	=	pLoader->GetColor( "DAMAGETO_CHAT" );
// 		break;

	case CHAT_MESSAGE_ARCHLORD:
		dwReturn	=	pLoader->GetColor( "Yellow" );
		break;
	case CHAT_MESSAGE_SHOUT:
		dwReturn	=	pLoader->GetColor( "WORD_BALLON" );
		break;
	case CHAT_MESSAGE_RACE:
		dwReturn	=	pLoader->GetColor( "RACE_TRUMPET" );
		break;
	case CHAT_MESSAGE_ELEMENTAL:
		dwReturn	=	pLoader->GetColor( "Yellow" );
		break;

	}

	return dwReturn;

}

BOOL	AgcChatManager::CBInitOpenAddTab( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl )
{
	AgcChatManager*		pThis			=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	AgcdUIControl*		pEditControl	=	static_cast< AgcdUIControl* >(pData1);
	if( !pEditControl )
		return FALSE;

	AgcmUIManager2*		pcmUIManager2	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !pcmUIManager2 )
		return FALSE;

	((AcUIEdit*)pEditControl->m_pcsBase)->ClearText();

	ZeroMemory( pThis->m_arrTabFlag , sizeof(pThis->m_arrTabFlag) );

	pThis->m_pcsDisplayUserData->m_stUserData.m_lCount = CHAT_MESSAGE_COUNT;
	pcmUIManager2->SetUserDataRefresh( pThis->m_pcsDisplayUserData );

	pThis->m_eTabOperation	=	E_TAB_OPTION_ADD;

	return TRUE;
}

BOOL	AgcChatManager::CBInitOpenEditTab( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl )
{
	AgcChatManager*		pThis	=	static_cast< AgcChatManager* >(pClass);
	if( !pThis )
		return FALSE;

	AgcdUIControl*			pEditControl	=	static_cast< AgcdUIControl* >(pData1);
	if( !pEditControl )
		return FALSE;

	AgcmUIManager2*		pcmUIManager2	=	static_cast< AgcmUIManager2* >( g_pEngine->GetModule( "AgcmUIManager2" ) );
	if( !pcmUIManager2 )
		return FALSE;
	
	if( pThis->m_pOperationChatTab )
	{
		((AcUIEdit*)pEditControl->m_pcsBase)->SetText( (CHAR*)pThis->m_pOperationChatTab->GetTabName().c_str() );

		UINT	ulTabFlag	=	pThis->m_pOperationChatTab->GetTabType();

		for( INT i = 0 ; i < CHAT_MESSAGE_COUNT ; ++i )
		{
			if( ulTabFlag & 1<<i )
				pThis->m_arrTabFlag[i]	=	TRUE;
			else
				pThis->m_arrTabFlag[i]	=	FALSE;
		}
	}

	pThis->m_pcsDisplayUserData->m_stUserData.m_lCount = CHAT_MESSAGE_COUNT;
	pcmUIManager2->SetUserDataRefresh( pThis->m_pcsDisplayUserData );

	pThis->m_eTabOperation	=	E_TAB_OPTION_EDIT;

	return TRUE;
}

