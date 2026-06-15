#include "CSystemMsgManager.h"
#include "AgcChatManager.h"

#include "AgcmUIManager2.h"

#include "AgcmUINotice.h"
// #include "AuKoreanTest.h"

CSystemMsgManager::CSystemMsgManager( void )
{
	memset( m_strBuffer, 0, sizeof( char ) * 256 );
}

CSystemMsgManager::~CSystemMsgManager( void )
{
}

BOOL CSystemMsgManager::OnRegisterSystemMsgCallBack( void )
{
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return FALSE;

	AgpmSystemMessage::GetSingleTon().SetCallbackSystemMessage( CallBack_OnSystemMessage, this );

	if( !_LoadSystemMsgFormat( "INI\\SystemMsgFormat.xml" ) )
		return FALSE;

	return TRUE;
}

BOOL CSystemMsgManager::OnReceiveSystemMessage( AgpdSystemMessage* pMsg )
{
	if( !pMsg ) return FALSE;

	std::string strMsg = _MakeSystemMessage( pMsg );
	DWORD dwColor = _GetSystemMessageColor( pMsg );

	switch( pMsg->m_lOperation )
	{
	case 0 :	AgcChatManager::OnAddSystemMessage( ( char* )strMsg.c_str(), dwColor );	break;
	case 1 :	OnMsgDlgModal( ( char* )strMsg.c_str() );									break;
	case 2 :	OnMsgDlgModalless( ( char* )strMsg.c_str() );								break;
	case 3 :	break;
	case 4 :	NoticeMessage( strMsg );													break;
	}

	return TRUE;
}

BOOL CSystemMsgManager::OnMsgDlgModal( char* pMsg )
{
	if( !pMsg || strlen( pMsg ) <= 0 ) return FALSE;

	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return FALSE;

	pcmUIManager->ActionMessageOKDialog( pMsg );
	return TRUE;
}

BOOL CSystemMsgManager::OnMsgDlgModalless( char* pMsg )
{
	if( !pMsg || strlen( pMsg ) <= 0 ) return FALSE;

	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return FALSE;

	pcmUIManager->ActionMessageOKDialog2( pMsg );
	return TRUE;
}

BOOL CSystemMsgManager::NoticeMessage( CONST string& strMessage )
{
	if( !strMessage.length() )
		return FALSE;

	AgcmUINotice*	pcmUINotice	=	static_cast< AgcmUINotice* >( g_pEngine->GetModule( "AgcmUINotice" ) );
	if( !pcmUINotice )
		return FALSE;

	pcmUINotice->AddMessage( strMessage );

	return TRUE;
}

BOOL CSystemMsgManager::_LoadSystemMsgFormat( char* pFileName )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return FALSE;

	AuXmlParser Doc;
	if( !Doc.LoadXMLFile( pFileName ) )
		return FALSE;

	TiXmlNode* pNodeRoot = Doc.FirstChild( "SystemMsgFormat" );
	if( !pNodeRoot ) return FALSE;

	TiXmlNode* pNode = pNodeRoot->FirstChild( "Msg" );
	while( pNode )
	{
		_AddSystemMessageEntry( pNode );
		pNode = pNode->NextSibling();
	}

	Doc.Clear();	
	return TRUE;
}

BOOL CSystemMsgManager::_AddSystemMessageEntry( void* pNode )
{
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return FALSE;

	TiXmlNode* pXmlNode = ( TiXmlNode* )pNode;
	if( !pXmlNode ) return FALSE;

	const char* pCode = pXmlNode->ToElement()->Attribute( "Code" );
	if( !pCode || strlen( pCode ) <= 0 ) return FALSE;

	const char* pType = pXmlNode->ToElement()->Attribute( "Type" );
	if( !pType || strlen( pType ) <= 0 ) return FALSE;

	const char* pColorR = pXmlNode->ToElement()->Attribute( "ColorR" );
	if( !pColorR || strlen( pColorR ) <= 0 ) return FALSE;

	const char* pColorG = pXmlNode->ToElement()->Attribute( "ColorG" );
	if( !pColorG || strlen( pColorG ) <= 0 ) return FALSE;

	const char* pColorB = pXmlNode->ToElement()->Attribute( "ColorB" );
	if( !pColorB || strlen( pColorB ) <= 0 ) return FALSE;

	char*		pUIMsg	=	NULL;
	const char* pString = pXmlNode->ToElement()->Attribute( "UiMessage" );
	if( pString )
	{
		pUIMsg = pcmUIManager->GetUIMessage( ( char* )pString );
	}

	stSystemMsgEntry NewEntry;

	NewEntry.m_eCode = ( eAgpmSystemMessageCode )atoi( pCode );

	if( pUIMsg )
		NewEntry.m_strString = pUIMsg;

	NewEntry.m_eUseType = ( eSystemMsgUseType )atoi( pType );
	NewEntry.m_dwColor = _MakeColorValue( pColorR, pColorG, pColorB );

	TiXmlNode* pParam = pXmlNode->FirstChild( "Param" );
	while( pParam )
	{
		const char* pParamType = pParam->ToElement()->Attribute( "Type" );
		const char* pIndex = pParam->ToElement()->Attribute( "Index" );
		if( pParamType && strlen( pParamType ) > 0 && pIndex && strlen( pIndex ) > 0 )
		{
			stCmdParamEntry NewParam;

			if( strcmp( pParamType, "String" ) == 0 )
			{
				NewParam.m_eType = ParamType_String;
			}
			else if( strcmp( pParamType, "Integer" ) == 0 )
			{
				NewParam.m_eType = ParamType_Integer;
			}
			else if( strcmp( pParamType, "Integer64" ) == 0 )
			{
				NewParam.m_eType = ParamType_Integer64;
			}

			NewParam.m_nIndex = atoi( pIndex );
			NewEntry.m_mapParam.Add( NewEntry.m_mapParam.GetSize(), NewParam );
		}

		pParam = pParam->NextSibling();
	}


	m_mapSystemMsg.Add( NewEntry.m_eCode, NewEntry );
	return TRUE;
}

void CSystemMsgManager::_AddSystemMessageEntry( eAgpmSystemMessageCode eCode, eSystemMsgUseType eUseType, char* pString )
{
	if( !pString || strlen( pString ) <= 0 ) return;

	stSystemMsgEntry* pEntry = m_mapSystemMsg.Get( eCode );
	if( pEntry ) return;

	stSystemMsgEntry NewEntry;

	NewEntry.m_eCode = eCode;
	NewEntry.m_eUseType = eUseType;
	NewEntry.m_strString = pString;

	m_mapSystemMsg.Add( eCode, NewEntry );
}

std::string CSystemMsgManager::_MakeSystemMessage( AgpdSystemMessage* pMsg )
{
	if( !pMsg ) 
		return "";
	if( pMsg->m_lCode == -1 ) 
		return pMsg->m_aszData[ 0 ];

	if( pMsg->m_lCode == AGPMSYSTEMMESSAGE_CODE_GENERAL_STRING )
		return pMsg->m_aszData[ 0 ];

	stSystemMsgEntry* pEntry = m_mapSystemMsg.Get( ( eAgpmSystemMessageCode )pMsg->m_lCode );
	if( !pEntry ) return "";

// 	bool	bIsKoreanTest = AuKoreanTest::IsKoreanTest();

	memset( m_strBuffer, 0, sizeof( char ) * 256 );
	switch( pEntry->m_eUseType )
	{
	case MsgUseType_Message :
		{
			strcpy_s( m_strBuffer, sizeof( char ) * 256, pEntry->m_strString.c_str() );
		}
		break;

	case MegUseType_Format :
		{
			const char* pFormat = pEntry->m_strString.c_str();
			if( pFormat && strlen( pFormat ) > 0 )
			{
				switch( pEntry->m_mapParam.GetSize() )
				{
				case 1 : 
					{
						stCmdParamEntry* pParam = pEntry->m_mapParam.GetByIndex( 0 );
						if( pParam && pParam->m_nIndex >= 0 && pParam->m_nIndex < 2 )
						{
							switch( pParam->m_eType )
							{
							case ParamType_Integer :
								sprintf_s( m_strBuffer, sizeof( char ) * 256, pFormat, pMsg->m_alData[ pParam->m_nIndex ] );
								break;
							case ParamType_Integer64 :
								sprintf_s( m_strBuffer, sizeof( char ) * 256, pFormat, (INT64)pMsg->m_alData[ pParam->m_nIndex ] );
								break;
							case ParamType_String :
								sprintf_s( m_strBuffer, sizeof( char ) * 256, pFormat, pMsg->m_aszData[ pParam->m_nIndex ] );
								break;
							}							
						}
					}
					break;

				case 3 : 
					{
						// AGPMSYSTEMMESSAGE_CODE_LOTTERY_ITEM_TO_POT_ITEM 인 경우에만.. 
						if( pEntry->m_eCode != AGPMSYSTEMMESSAGE_CODE_LOTTERY_ITEM_TO_POT_ITEM ) return "";

						stCmdParamEntry* pParam1 = pEntry->m_mapParam.GetByIndex( 0 );
						stCmdParamEntry* pParam2 = pEntry->m_mapParam.GetByIndex( 1 );
						stCmdParamEntry* pParam3 = pEntry->m_mapParam.GetByIndex( 2 );

						if( pParam1 && pParam2 && pParam3 )
						{
							if(AP_SERVICE_AREA_GLOBAL == g_eServiceArea )
								sprintf_s( m_strBuffer, sizeof( char ) * 256, pFormat,
								pMsg->m_alData[ pParam3->m_nIndex ], pMsg->m_aszData[ pParam2->m_nIndex ], pMsg->m_aszData[ pParam1->m_nIndex ]);
							else
								sprintf_s( m_strBuffer, sizeof( char ) * 256, pFormat,
								pMsg->m_aszData[ pParam1->m_nIndex ], pMsg->m_aszData[ pParam2->m_nIndex ], pMsg->m_alData[ pParam3->m_nIndex ] );
						}
					}
					break;
				}
			}
		}
		break;

	default :					return "";							break;
	};

	return m_strBuffer;
}

DWORD CSystemMsgManager::_GetSystemMessageColor( AgpdSystemMessage* pMsg )
{
	if( !pMsg || pMsg->m_lCode == -1 ) return pMsg->m_dwColor;

	stSystemMsgEntry* pEntry = m_mapSystemMsg.Get( ( eAgpmSystemMessageCode )pMsg->m_lCode );
	if( !pEntry ) return 0xFFFFFFFF;

	return pEntry->m_dwColor;
}

DWORD CSystemMsgManager::_MakeColorValue( const char* pColorR, const char* pColorG, const char* pColorB )
{
	int nRed = pColorR && strlen( pColorR ) > 0 ? atoi( pColorR ) : 0;
	int nGreen = pColorG && strlen( pColorG ) > 0 ? atoi( pColorG ) : 0;
	int nBlue = pColorB && strlen( pColorB ) > 0 ? atoi( pColorB ) : 0;
	return 0xff000000 | nRed << 16 | nGreen << 8 | nBlue;
}

BOOL CSystemMsgManager::CallBack_OnSystemMessage( void* pData, void* pClass, void* pCustData )
{
	AgpdSystemMessage* ppdSysMessage = ( AgpdSystemMessage* )pData;
	CSystemMsgManager* pManager = ( CSystemMsgManager* )pClass;
	if( !ppdSysMessage || !pManager ) return FALSE;

	return pManager->OnReceiveSystemMessage( ppdSysMessage );
}
