#include "CBGMTitleViewer.h"
#include "AgcChatManager.h"


#include "AgcmUIManager2.h"
#include "AgcmEventNature.h"
#include "AuStrTable.h"

#define	MAX_FILENAME	64

CBGMTitleViewer::CBGMTitleViewer( void )
{
	m_strCurrBGMFileName = "";
	m_pUserData = NULL;
}

CBGMTitleViewer::~CBGMTitleViewer( void )
{
}

BOOL CBGMTitleViewer::OnReadyBGMTitle( void )
{
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	AgcmEventNature* pcmEventNature = ( AgcmEventNature* )g_pEngine->GetModule( "AgcmEventNature" );
	if( !pcmUIManager || !pcmEventNature ) return FALSE;

	if( !pcmUIManager->AddDisplay( this, "Chat_BGMTitle", 0, CallBack_DisplayBGMTitle, AGCDUI_USERDATA_TYPE_INT32 ) ) return FALSE;

	m_pUserData = pcmUIManager->AddUserData( "Chat_BGMTitle", NULL,	0, 0, AGCDUI_USERDATA_TYPE_INT32 );
	if( !m_pUserData ) return FALSE;

	if( !pcmEventNature->SetCallbackBGMUpdate( CallBack_UpdateBGMTitle, this ) ) return FALSE;

	_AddBGMTitle( "MT_001.MP3",		ClientStr().GetStr( STI_AMUNESIA ) );
	_AddBGMTitle( "HTT_001.MP3",	ClientStr().GetStr( STI_THOULAN ) );
	_AddBGMTitle( "HTT_002.MP3",	ClientStr().GetStr( STI_DELPARASS ) );
	_AddBGMTitle( "HFT_001.MP3",	ClientStr().GetStr( STI_ELAN ) );
	_AddBGMTitle( "HFT_002.MP3",	ClientStr().GetStr( STI_JUNGLE ) );
	_AddBGMTitle( "HFT_003.MP3",	ClientStr().GetStr( STI_AMUNESIA ) );
	_AddBGMTitle( "HFT_004.MP3",	ClientStr().GetStr( STI_ELORR ) );
	_AddBGMTitle( "HFT_005.MP3",	ClientStr().GetStr( STI_MEMBREATHEN ) );
	_AddBGMTitle( "HFT_006.MP3",	ClientStr().GetStr( STI_SHINEWOOD ) );
	_AddBGMTitle( "HFT_007.MP3",	ClientStr().GetStr( STI_HARIEL ) );
	_AddBGMTitle( "HFT_008.MP3",	ClientStr().GetStr( STI_LIMELITE ) );
	_AddBGMTitle( "OTT_001.MP3",	ClientStr().GetStr( STI_THRILGARD ) );
	_AddBGMTitle( "OTT_002.MP3",	ClientStr().GetStr( STI_KUN ) );
	_AddBGMTitle( "OFT_001.MP3",	ClientStr().GetStr( STI_WHISTLE ) );
	_AddBGMTitle( "OFT_002.MP3",	ClientStr().GetStr( STI_CIESELVA ) );
	_AddBGMTitle( "OFT_003.MP3",	ClientStr().GetStr( STI_XAILOK ) );
	_AddBGMTitle( "OFT_004.MP3",	ClientStr().GetStr( STI_EDGEOROCK ) );
	_AddBGMTitle( "OFT_005.MP3",	ClientStr().GetStr( STI_SANTUS ) );
	_AddBGMTitle( "OFT_006.MP3",	ClientStr().GetStr( STI_HELINK ) );
	_AddBGMTitle( "OFT_007.MP3",	ClientStr().GetStr( STI_GRIMROAR ) );
	_AddBGMTitle( "OFT_008.MP3",	ClientStr().GetStr( STI_SERENDO ) );
	_AddBGMTitle( "BT_001.MP3",		ClientStr().GetStr( STI_BATTLE ) );
	_AddBGMTitle( "NFT_001.MP3",	ClientStr().GetStr( STI_WINDRILL ) );
	_AddBGMTitle( "NFT_002.MP3",	ClientStr().GetStr( STI_GORGOB ) );
	_AddBGMTitle( "NFT_003.MP3",	ClientStr().GetStr( STI_KUSKUT ) );
	_AddBGMTitle( "NFT_004.MP3",	ClientStr().GetStr( STI_ELKA ) );
	_AddBGMTitle( "NFT_005.MP3",	ClientStr().GetStr( STI_SHAMLUK ) );
	_AddBGMTitle( "NFT_006.MP3",	ClientStr().GetStr( STI_NUK ) );
	_AddBGMTitle( "NFT_007.MP3",	ClientStr().GetStr( STI_RAZMARQ ) );

	ReadXMLData("ini\\client\\LoginBGM.xml");

	return TRUE;
}

BOOL CBGMTitleViewer::OnUpdateBGMTitle( char* pFileName )
{
	AgcmSound* pcmSound = ( AgcmSound* )g_pEngine->GetModule( "AgcmSound" );
	if( !pcmSound ) return FALSE;
	if( pcmSound->IsSoundMuted( SoundType_BGM ) ) return FALSE;

	char* pTitleName = GetBGMTitle( pFileName );
	if( !pTitleName || strlen( pTitleName ) <= 0 ) return FALSE;

	m_strCurrBGMFileName = pFileName;

	char strBuffer[ 256 ] = { 0, };
	switch( g_eServiceArea )
	{
	case AP_SERVICE_AREA_KOREA :	sprintf_s( strBuffer, sizeof( char ) * 256, "♬ %s", pTitleName );		break;
	default :						sprintf_s( strBuffer, sizeof( char ) * 256, "BGM : %s", pTitleName );	break;
	}

	// 채팅창에 배경음 종류를 뿌려준다
	// AgcChatManager::OnAddSystemMessage( strBuffer );

	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return FALSE;

	pcmUIManager->m_csEffectBGMWindow.SetEffectSubText( strBuffer, 0, 5, 20, FALSE, 0xFFFFFFFF, 1.0f, 10000, TRUE );
	pcmUIManager->SetUserDataRefresh( m_pUserData );
	return TRUE;
}

char* CBGMTitleViewer::GetBGMTitle( char* pFileName )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return FALSE;

	stBGMTitleEntry* pEntry = m_mapBGM.Get( pFileName );
	if( !pEntry ) 
	{
		pEntry = m_mapLoginBGM.Get( pFileName );
		if( !pEntry )
			return NULL;
	}

	return ( char* )pEntry->m_strTitle.c_str();
}

char* CBGMTitleViewer::GetCurrentBGMTitle( void )
{
	return GetBGMTitle( ( char* )m_strCurrBGMFileName.c_str() );
}

void CBGMTitleViewer::_AddBGMTitle( const char* pFileName, const char* pTitleName )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return;
	if( !pTitleName || strlen( pTitleName ) <= 0 ) return;

	stBGMTitleEntry* pEntry = m_mapBGM.Get( pFileName );
	if( pEntry ) return;

	stBGMTitleEntry NewEntry;

	NewEntry.m_strTitle = pTitleName;
	NewEntry.m_strFileName = pFileName;

	m_mapBGM.Add( pFileName, NewEntry );
}

void CBGMTitleViewer::_AddLoginBGMTitle( const char* pFileName, const char* pTitleName )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return;
	if( !pTitleName || strlen( pTitleName ) <= 0 ) return;

	stBGMTitleEntry* pEntry = m_mapLoginBGM.Get( pFileName );
	if( pEntry ) return;

	stBGMTitleEntry NewEntry;

	NewEntry.m_strTitle = pTitleName;
	NewEntry.m_strFileName = pFileName;

	m_mapLoginBGM.Add( pFileName, NewEntry );
}

BOOL CBGMTitleViewer::CallBack_DisplayBGMTitle( void* pClass, void* pData, AgcdUIDataType eType, int nID, char* pDisplay, int* pValue, AgcdUIControl* pcdSourceControl )
{
	CBGMTitleViewer* pBGMTitleViewer = ( CBGMTitleViewer* )pClass;
	if( !pBGMTitleViewer ) return FALSE;
	if( !pcdSourceControl || pcdSourceControl->m_lType != AcUIBase::TYPE_EDIT ) return FALSE;

	AcUIEdit* pEdit = ( AcUIEdit* )pcdSourceControl->m_pcsBase;
	if( !pEdit ) return FALSE;

	char* pTitle = pBGMTitleViewer->GetCurrentBGMTitle();
	if( !pTitle || strlen( pTitle ) <= 0 ) return FALSE;
	
	char strBuffer[ 256 ] = { 0, };
	sprintf_s( strBuffer, sizeof( char ) * 256, "BGM : %s", pTitle );
	pEdit->SetText( strBuffer );

	return TRUE;
}

BOOL CBGMTitleViewer::CallBack_UpdateBGMTitle( void* pData, void* pClass, void* pCustData )
{
	CBGMTitleViewer* pBGMTitleViewer = ( CBGMTitleViewer* )pClass;
	char* pFileName = ( char* )pData;
	if( !pBGMTitleViewer || !pFileName || strlen( pFileName ) <= 0 ) return FALSE;

	return pBGMTitleViewer->OnUpdateBGMTitle( pFileName );
}

BOOL CBGMTitleViewer::ReadXMLData( CONST string& strFileName )
{
	AuXmlParser _Parser;
	int			_NumOfLoginBGM = 0;

	if(!_Parser.LoadXMLFile(strFileName))
		return FALSE;

	AuXmlNode* pNode = _Parser.FirstChild("LoginBGMInfo");

	if(pNode)
	{
		AuXmlElement *pElement = pNode->FirstChildElement("BGM");
		while(pElement)
		{
			char _strTitle[MAX_FILENAME];
			char _strFilename[MAX_FILENAME];

			strcpy(_strTitle, pElement->Attribute("title"));
			strcpy(_strFilename, pElement->Attribute("filename"));

			_AddLoginBGMTitle(_strFilename, _strTitle);

			pElement = pElement->NextSiblingElement("BGM");
			++_NumOfLoginBGM;
		}
	}
	
	return TRUE;
}


