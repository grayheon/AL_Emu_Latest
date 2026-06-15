#include "AgcmUINotice.h"
#include "AgcmUIManager2.h"
#include "AgcmFont.h"
#include "AgcmUIChatting2.h"

#include "AgcmUIConsole.h"

#include "AuStrTable.h"

#define		DEFAULT_NOTICE_MESSAGE_SIZE_X		800
#define		DEFAULT_NOTICE_MESSAGE_SIZE_Y		25

#define		DEFAULT_NOTICE_MESSAGE_POS_X		0.5f
#define		DEFAULT_NOTICE_MESSAGE_POS_Y		0.2f

AgcmUINotice::AgcmUINotice( VOID )
{
	SetModuleName( "AgcmUINotice" );
	EnableIdle( TRUE );

	for( INT i = 0 ; i < MAX_NOTICE_MESSAGE_VIEW ; ++i )
	{
		m_arrpUIBase[ i ]	=	NULL;
	}

	m_pcsAgcmUIManager2		=	NULL;
	m_pcsAgcmFont			=	NULL;
	m_bStartMacro			=	FALSE;
	m_bMacroTest			=	FALSE;
}

AgcmUINotice::~AgcmUINotice( VOID )
{

}

BOOL	AgcmUINotice::OnInit( VOID )
{
	m_pcsAgcmUIManager2	=	static_cast< AgcmUIManager2* >( GetModule( "AgcmUIManager2" ) );
	m_pcsAgcmFont		=	static_cast< AgcmFont* >( GetModule( "AgcmFont" ) );
	m_pcsAgcmUIChatting2=	static_cast< AgcmUIChatting2* >( GetModule( "AgcmUIChatting2" ) );

	for( INT i = 0 ; i < MAX_NOTICE_MESSAGE_VIEW ; ++i )
	{
		m_arrpUIBase[i]	=	new AcUIBase;
		m_arrpUIBase[i]->m_Property.bTopmost	=	TRUE;
		m_arrpUIBase[i]->m_Property.bUseInput	=	FALSE;

		m_pcsAgcmUIManager2->AddWindow( m_arrpUIBase[i] );

		m_arrpUIBase[i]->ShowWindow( TRUE );
	}

	AS_REGISTER_TYPE_BEGIN(AgcmUINotice, AgcmUINotice);
	AS_REGISTER_METHOD0(void, DBG_Message);
	AS_REGISTER_TYPE_END;

	return TRUE;
}

BOOL	AgcmUINotice::OnAddModule( VOID )
{
	return TRUE;
}

BOOL	AgcmUINotice::OnIdle( UINT32 ulClockCount )
{
	BOOL								bRefresh	=	FALSE;
	ListNoticeOriginalString::iterator	Iter		=	m_listOriginal.begin();
	for( ; Iter != m_listOriginal.end() ; )
	{
		// Fade
		if( (*Iter)->m_dwStartTime+(NOTICE_MESSAGE_LIFE_TIME-NOTICE_MESSAGE_FADE_TIME) < ulClockCount )
		{
			DWORD dwRest	=	ulClockCount - ( (*Iter)->m_dwStartTime+(NOTICE_MESSAGE_LIFE_TIME-NOTICE_MESSAGE_FADE_TIME) );

			(*Iter)->m_fAlpha	= 1.0f - ( (FLOAT)dwRest / (FLOAT)NOTICE_MESSAGE_FADE_TIME );
			if( (*Iter)->m_fAlpha < 0 )
				(*Iter)->m_fAlpha	=	0;

		}

		// 지운다
		if( (*Iter)->m_dwStartTime+NOTICE_MESSAGE_LIFE_TIME < ulClockCount )
		{
			DEF_SAFEDELETE( (*Iter) );
			m_listOriginal.erase( Iter++ );	
			continue;
		}

		++Iter;
	}


	if( m_bStartMacro )
	{
		Iter	=	m_listMacroMessage.begin();
		for( ; Iter != m_listMacroMessage.end() ; ++Iter )
		{
			if( (*Iter)->m_dwStartTime < ulClockCount )
			{
				// Test는 그냥 찍어주기만 한다.
				if( m_bMacroTest )
				{
					AddMessage( (*Iter)->m_strString );
				}
				else
				{
					CHAR		szMessage[ MAX_PATH*3 ];
					ZeroMemory( szMessage , MAX_PATH*3 );

					sprintf_s( szMessage , MAX_PATH*3 , "%s %s" , ClientStr().GetStr(STI_CHAT_NOTICE_LOCALIZE) , (*Iter)->m_strString.c_str() );

					m_pcsAgcmUIChatting2->OnSendMessage( AGPDCHATTING_TYPE_NORMAL , szMessage , strlen(szMessage) , NULL , FALSE );
				}

				DEF_SAFEDELETE( (*Iter) );
				m_listMacroMessage.erase( Iter++ );
				break;
			}
		}

		if( !m_listMacroMessage.size() )
			m_bStartMacro	=	FALSE;
	}


	RefreshMessage();

	return TRUE;
}

VOID	AgcmUINotice::DBG_Message( VOID )
{
	AddMessage( "<Hff0000ff>명<Hff00ff00>균<Hffff0000>짱<Hffff00ff> 이에욤!!!!!" );
}

BOOL	AgcmUINotice::OnDestroy( VOID )
{
	for( INT i = 0 ; i < MAX_NOTICE_MESSAGE_VIEW ; ++i )
	{
		m_pcsAgcmUIManager2->RemoveWindow( m_arrpUIBase[i] , TRUE );
		DEF_SAFEDELETE( m_arrpUIBase[i]	);
	}

	return TRUE;
}

BOOL	AgcmUINotice::AddMessage( CONST string& strString )
{
	stNoticeMessage*	pMessage	=	new stNoticeMessage;

	pMessage->m_strString	=	strString;
	pMessage->m_dwStartTime	=	GetClockCount();
	pMessage->m_fAlpha		=	1.0f;

	m_listOriginal.push_back( pMessage );

	if( m_listOriginal.size() > 5 )
	{
		DEF_SAFEDELETE( (*m_listOriginal.begin()) );
		m_listOriginal.erase( m_listOriginal.begin() );
	}

	RefreshMessage();
	return TRUE;
}

VOID	AgcmUINotice::RefreshMessage( VOID )
{
	_RefreshPosition();
	_ClearChildWindow();

	ListNoticeOriginalString::iterator	Iter	=	m_listOriginal.begin();
	for( INT i = 0 ; Iter != m_listOriginal.end() ; ++Iter , ++i )
	{
		m_arrpUIBase[i]->m_pfAlpha	=	&(*Iter)->m_fAlpha;
		_ParseMessage( (*Iter)->m_strString , m_arrpUIBase[i] );
	}
}

VOID	AgcmUINotice::_RefreshPosition( VOID )
{

	INT		nPositionX	=	(INT)((float)m_pcsAgcmUIManager2->m_lWindowWidth	*	DEFAULT_NOTICE_MESSAGE_POS_X);
	INT		nPositionY	=	(INT)((float)m_pcsAgcmUIManager2->m_lWindowHeight	*	DEFAULT_NOTICE_MESSAGE_POS_Y);

	nPositionX	-=	(DEFAULT_NOTICE_MESSAGE_SIZE_X/2);

	for( INT i = 0 ; i < MAX_NOTICE_MESSAGE_VIEW ; ++i )
	{
		if( m_arrpUIBase[i] )
		{
			m_arrpUIBase[i]->MoveWindow( nPositionX , nPositionY+(i*DEFAULT_NOTICE_MESSAGE_SIZE_Y) , DEFAULT_NOTICE_MESSAGE_SIZE_X , DEFAULT_NOTICE_MESSAGE_SIZE_Y );
		}
	}

}

VOID	AgcmUINotice::_ClearChildWindow( VOID )
{
	for( INT i = 0 ; i < MAX_NOTICE_MESSAGE_VIEW ; ++i )
	{
		if( m_arrpUIBase[i] )
		{
			m_arrpUIBase[i]->AllDeleteChild( TRUE , TRUE );
			m_arrpUIBase[i]->UpdateChildWindow();
		}
	}
}

VOID	AgcmUINotice::_ParseMessage( CONST string& strString , AcUIBase* pUIBase )
{
	if( !strString.length() || !pUIBase )
		return;

	INT			nOffset		=	0;
	DWORD		dwColor		=	0xffffffff;
	UINT32		unFontType	=	5;
	INT			nPosX		=	0;

	list< AcUIBase* >		listNoticeUI;

	while( TRUE )
	{
		size_t		stringPosLast	=	0;
		size_t		stringPosStart	=	strString.find( "<H" , nOffset );
		
		if( stringPosStart != string::npos)
		{
			if( stringPosStart != nOffset )
			{
				string	strSubString	=	strString.substr( nOffset , stringPosStart - nOffset );

				AcUIBase*	pTextUI		=	new AcUIBase;
				INT			nWidth		=	m_pcsAgcmFont->GetTextExtent( unFontType , (CHAR*)strSubString.c_str() , strSubString.length() );
				INT			nHeight		=	DEFAULT_NOTICE_MESSAGE_SIZE_Y;

				pTextUI->SetStaticStringExt( (CHAR*)strSubString.c_str() , 1 , unFontType , dwColor , true , ACUIBASE_HALIGN_CENTER , ACUIBASE_VALIGN_CENTER );
				pTextUI->MoveWindow( nPosX , 0 , nWidth , nHeight );

				pUIBase->AddChild( pTextUI );
				pTextUI->m_pfAlpha	=	pUIBase->m_pfAlpha;

				listNoticeUI.push_back( pTextUI );

				nPosX	+=	nWidth;
			}

			

			stringPosLast	=	strString.find( ">" , stringPosStart );
			string	strColor	=	strString.substr( stringPosStart+2 , stringPosLast - (stringPosStart+2) );

			dwColor	=	_HexStringToDWORD( strColor.c_str() );
			nOffset	=	stringPosLast+1;

			continue;
		}

		string	strSubString	=	strString.substr( nOffset );
		
		AcUIBase*	pTextUI		=	new AcUIBase;
		INT			nWidth	=	m_pcsAgcmFont->GetTextExtent( unFontType , (CHAR*)strSubString.c_str() , strSubString.length() );
		INT			nHeight	=	DEFAULT_NOTICE_MESSAGE_SIZE_Y;

		pTextUI->SetStaticStringExt( (CHAR*)strSubString.c_str() , 1 , unFontType , dwColor , true , ACUIBASE_HALIGN_CENTER , ACUIBASE_VALIGN_CENTER );
		pTextUI->MoveWindow( nPosX , 0 , nWidth , nHeight );

		pUIBase->AddChild( pTextUI );
		pTextUI->m_pfAlpha	=	pUIBase->m_pfAlpha;
		listNoticeUI.push_back( pTextUI );

		nPosX	+=	nWidth;

		break;
	}

	INT		nHalf	=	(DEFAULT_NOTICE_MESSAGE_SIZE_X	-	nPosX)/2;

	// 가운데 정렬 시킨다
	list< AcUIBase* >::iterator	UIIter	=	listNoticeUI.begin();
	for( ; UIIter != listNoticeUI.end() ; ++UIIter )
	{
		(*UIIter)->MoveWindow( (*UIIter)->x+nHalf , (*UIIter)->y , (*UIIter)->w , (*UIIter)->h );
	}

	pUIBase->UpdateChildWindow();
}

DWORD	AgcmUINotice::_HexStringToDWORD( const char* szHexString )
{
	if( !szHexString )
		return 0;

	DWORD	dwValue		=	0;
	DWORD	dwTotal		=	0;
	DWORD	dwMulti		=	0;
	INT		nLength		=	strlen( szHexString );

	for( INT i = nLength-1 ; i >= 0 ; --i )
	{
		dwMulti		=	1;

		// Ascii Code 의 값을 빼서 숫자로 만든다
		if( szHexString[ i ] >= 'A' && szHexString[ i ] <= 'F' )
			dwValue		=	szHexString[ i ] - 55;

		else if( szHexString[ i ] >= 'a' && szHexString[ i ] <= 'f' )
			dwValue		=	szHexString[ i ] - 87;

		else if( szHexString[ i ] >= '0' && szHexString[ i ] <= '9' )
			dwValue		=	szHexString[ i ] - 48;

		// 승을 구해준다
		for( INT k = 0 ; k < nLength - 1 - i  ; ++k )
			dwMulti	*= 16;

		dwTotal +=	( dwValue * dwMulti );
	}

	return dwTotal;
}

VOID		AgcmUINotice::NoticeMacro( CONST string& strXMLFile , BOOL bTest /* = FALSE */ )
{
	if( m_bStartMacro )
		return;

	AuXmlParser	parser;
	DWORD		dwClockCount	=	GetClockCount();

	if( parser.LoadXMLFile( strXMLFile.c_str() ) )
	{
		AuXmlNode*	pNode	=	parser.FirstChild( "MSG" );
		for( ; pNode  ; pNode = pNode->NextSibling( "MSG" ) )
		{
			stNoticeMessage*	pNoticeMSG	=	new stNoticeMessage;

			CONST	CHAR*	pNoticeInterval	=	pNode->ToElement()->Attribute( "INTERVAL" );
			CONST	CHAR*	pNoticeMessage	=	pNode->ToElement()->GetText();

			
			if( pNoticeInterval )
			{
				pNoticeMSG->m_dwStartTime	=	dwClockCount + atoi( pNoticeInterval );
				
				dwClockCount	=	pNoticeMSG->m_dwStartTime;
			}

			if( pNoticeMessage )
			{
				pNoticeMSG->m_strString	=	pNoticeMessage;
			}

			m_listMacroMessage.push_back( pNoticeMSG );
		}
	}
	
	if( m_listMacroMessage.size() )
		m_bStartMacro	=	TRUE;

	m_bMacroTest	=	bTest;
	
	parser.Clear();
}
