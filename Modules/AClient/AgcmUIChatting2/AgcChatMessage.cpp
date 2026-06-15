#include "AgcChatMessage.h"
#include "AgcChatManager.h"
#include "AgcmFont.h"

#include "AgcmUIChatting2.h"
#include "AcUIEventButton.h"

AgcChatMessage::MapTextFormat	AgcChatMessage::m_mapTextFormat;

AgcChatMessage::AgcChatMessage( VOID )
{
	m_nMaxTextLength	=	0;
	m_nMaxTextLine		=	0;
	m_bTextBoardMessage	=	FALSE;
	m_unMessageType		=	CHAT_MESSAGE_NORMAL;
	m_bEnable			=	TRUE;

	m_pManager			=	NULL;
	m_nFontType			=	0;

#ifdef _AREA_GLOBAL_
	lineCheckByWord_ = true;
#else
	lineCheckByWord_ = false;
#endif

	m_pcsAgcmFont = 0;
}

AgcChatMessage::~AgcChatMessage( VOID )
{
	Clear();
}

BOOL	AgcChatMessage::Initialize( VOID )
{
	return TRUE;
}

VOID	AgcChatMessage::Clear( VOID )
{
	ClearMessage();
}

//-----------------------------------------------------------------------
//

int AgcChatMessage::SliceLineByWord( std::string & str, int totalWidth, size_t count )
{
	if( !m_pcsAgcmFont )
		m_pcsAgcmFont = static_cast< AgcmFont* >(g_pEngine->GetModule( "AgcmFont" ));

	if( str.empty() || GetTextLength() <= 0 )
		return 0;

	size_t spaceIdx = str.rfind( " " );

	if( spaceIdx != std::string::npos )
	{
		str = str.substr( 0, spaceIdx );
	}

	int width = m_pcsAgcmFont->GetTextExtent( 0, (char*)str.c_str(), (int)(str.length()) );

	if( (GetTextLength() - totalWidth) >= width )
		return (int)str.length();

	str = str.substr( 0, str.length() - 1 );

	return SliceLineByWord( str, totalWidth, count );
}

//-----------------------------------------------------------------------
//

stOriginalMessageInfo*	AgcChatMessage::AddMessage( CHAR* pSenderName , CHAR* pMessage , DWORD dwCustomColor /* = 0 */ , UINT32 ulCharType )
{
	if( !pMessage )
		return NULL;

	CHAR		szBuffer [ MAX_PATH * 2 ];
	ZeroMemory( szBuffer , MAX_PATH*2 );

	DWORD		dwCreateTickCount	=	GetTickCount();

	stChatTextFormat*	pTextFormat	=	GetTextFormat( m_unMessageType );
	if( !pTextFormat )
		return FALSE;

	// 문자열 조합
	_StringCombination( szBuffer , MAX_PATH*2 , pTextFormat , pSenderName , pMessage );

	DWORD		dwColor	=	dwCustomColor;
	if( !dwColor )
		dwColor	= pTextFormat->m_dwTextColor;

	string	strNoEventString;

	// 메세지를 파싱해서 저장한다
	_PushAndParseString( szBuffer , pSenderName , pMessage , dwColor , dwCreateTickCount , FALSE , TRUE , &strNoEventString , ulCharType );

	// 오리지날 메세지를 저장해둔다
	stOriginalMessageInfo	stMessageInfo;
	stMessageInfo.m_dwCustomColor	=	dwCustomColor;
	stMessageInfo.m_strMessage		=	pMessage;
	stMessageInfo.m_strNoEventMessage = strNoEventString;
	stMessageInfo.m_ulCharType		=	ulCharType;
	
	if( pSenderName )
		stMessageInfo.m_strSenderName	=	pSenderName;

	stMessageInfo.m_dwCreateTick	=	dwCreateTickCount;
	m_dequeOriginalMessage.push_back( stMessageInfo);

	// 메세지 최대 갯수를 초과하면 지운다
	_OverFlowMessageDelete();

	return &m_dequeOriginalMessage[ m_dequeOriginalMessage.size()-1 ];
}

stChatMsgEntry*		AgcChatMessage::GetMessage( INT nIndex )
{
	INT		nCount	=	(INT)m_dequeMessage.size();

	if( nCount <= nIndex )
		return NULL;

	return m_dequeMessage[nIndex];
}

INT		AgcChatMessage::GetMessageCount( VOID )
{
	INT		nCount	=	(INT)m_dequeMessage.size();

	return nCount;
}

VOID	AgcChatMessage::ClearMessage( VOID )
{
	_ClearEntryMessage();
	ClearOriginalMessage();
}

VOID	AgcChatMessage::RefreshMessageEntry( VOID )
{
	// 우선 파싱 된 메세지를 다 지운다
	_ClearEntryMessage();

	// 새로 파싱을 한다
	INT	nCount	=	(INT)m_dequeOriginalMessage.size();
	for( INT i = 0 ; i < nCount ; ++i )
	{
		CHAR		szBuffer [ MAX_PATH * 2 ];
		ZeroMemory( szBuffer , MAX_PATH*2 );

		stChatTextFormat*	pTextFormat	=	GetTextFormat( m_unMessageType );
		if( !pTextFormat )
			return;

		// 문자열 조합
		_StringCombination( szBuffer , MAX_PATH*2 , pTextFormat , m_dequeOriginalMessage[i].m_strSenderName.c_str() , m_dequeOriginalMessage[i].m_strMessage.c_str() );

		DWORD		dwColor	=	m_dequeOriginalMessage[i].m_dwCustomColor;
		if( !dwColor )
			dwColor	= pTextFormat->m_dwTextColor;

		CHAR*	pSenderName	=	NULL;
		if( m_dequeOriginalMessage[i].m_strSenderName.length() )
			pSenderName	=	(CHAR*)m_dequeOriginalMessage[i].m_strSenderName.c_str();

		// 메세지를 파싱해서 저장한다
		_PushAndParseString( szBuffer , 
							pSenderName,
							(CHAR*)m_dequeOriginalMessage[i].m_strMessage.c_str() , 
							dwColor , m_dequeOriginalMessage[i].m_dwCreateTick , 0 , 0 , 0 , m_dequeOriginalMessage[i].m_ulCharType
							);
	}
}

VOID	AgcChatMessage::SetTextFormat( UINT unMessageType , CONST CHAR* szFormat , CONST CHAR* szHeader , BOOL bUseHeader , BOOL bUseSenderName , DWORD dwTextColor )
{
	MapTextFormat::iterator	FormatIter	=	m_mapTextFormat.find( unMessageType );
	if( FormatIter == m_mapTextFormat.end() )
	{
		stChatTextFormat		TextFormat;

		TextFormat.m_bIsUseHeader		=	bUseHeader;
		TextFormat.m_bIsUseSenderName	=	bUseSenderName;
		TextFormat.m_dwTextColor		=	dwTextColor;
		if( szHeader )
			TextFormat.m_strHeader			=	szHeader;
		if( szFormat )
			TextFormat.m_strFormat			=	szFormat;

		m_mapTextFormat.insert( make_pair(unMessageType , TextFormat) );
	}
}

stChatTextFormat*	AgcChatMessage::GetTextFormat( UINT unMessageType )
{
	stChatTextFormat*		pChatTextFormat	=	NULL;
	MapTextFormat::iterator	FormatIter		=	m_mapTextFormat.find( unMessageType );

	if( FormatIter != m_mapTextFormat.end() )
	{
		pChatTextFormat	=	&FormatIter->second;
	}
	
	return pChatTextFormat;
}


VOID	AgcChatMessage::_StringCombination( OUT CHAR* szBuffer , INT nMaxLength , stChatTextFormat* pChatTextFormat , CONST CHAR* szName , CONST CHAR* szMessage )
{
	if( !szBuffer || !pChatTextFormat )
		return;

	if( pChatTextFormat->m_bIsUseHeader )
	{
		if( pChatTextFormat->m_bIsUseSenderName )
			sprintf_s( szBuffer , nMaxLength , pChatTextFormat->m_strFormat.c_str() , pChatTextFormat->m_strHeader.c_str() , szName , szMessage );
		else
			sprintf_s( szBuffer , nMaxLength , pChatTextFormat->m_strFormat.c_str() , pChatTextFormat->m_strHeader.c_str() , szMessage );
	}

	else
	{
		if( pChatTextFormat->m_bIsUseSenderName )
			sprintf_s( szBuffer , nMaxLength , pChatTextFormat->m_strFormat.c_str() , szName , szMessage );
		else
			sprintf_s( szBuffer , nMaxLength , pChatTextFormat->m_strFormat.c_str() , szMessage );
	}
}

BOOL	AgcChatMessage::_PushAndParseString( CHAR* szBuffer , char* pSenderName, char* pMessage , DWORD dwCustomColor , DWORD dwCreateTick , BOOL bNewLine /* = FALSE */ , BOOL bNewMessage , string* pstrNoEventString , UINT ulCharType )
{
	if( !szBuffer )
		return FALSE;

	//	문자열 파싱 및 삽입 작업

	string			strTemp;
	BOOL			bEnd			=	TRUE;
	BOOL			bExit			=	TRUE;
	BOOL			bForceNextLine		=	FALSE;
	stChatMsgEntry*	pMessageEntry	=	new stChatMsgEntry;
	INT				nStartPos		=	0;
	INT				nLastPos		=	0;
	INT				nStringWidth	=	0;
	INT				nNextLine		=	0;
	INT				nTotalStringWidth	=	0;

	if( !m_pcsAgcmFont )
		m_pcsAgcmFont = static_cast< AgcmFont* >(g_pEngine->GetModule( "AgcmFont" ));

	pMessageEntry->m_eType			=	m_unMessageType;

	if( pSenderName )
		pMessageEntry->m_strSenderName	=	pSenderName;
	pMessageEntry->m_strMessage		=	pMessage;
	pMessageEntry->m_dwCustomColor	=	dwCustomColor;
	pMessageEntry->m_dwCreateTick	=	dwCreateTick;

	pMessageEntry->m_bNewLine		=	bNewLine;

	for( INT i = 0 ; bExit ; ++i )
	{
		strTemp	+=	szBuffer[i];
		if( szBuffer[i] == '<' || szBuffer[i] == '[' )
			nStartPos	=	i;

		if( szBuffer[i] == '>' || szBuffer[i] == ']' || szBuffer[i] == 0 || szBuffer[i+1] == '<' || szBuffer[i+1] == '[' )
		{	
			if( szBuffer[i] == 0 )
				bExit = FALSE;

			nLastPos	=	i;

			if( szBuffer[nLastPos] < 0 )
				++nLastPos;

			// 이건 컬러다( 컬러 값만 바꿔주자 )
			if( szBuffer[nStartPos] == '<' && szBuffer[nStartPos+1] == 'H' )
			{
				size_t	nstringPos	=	strTemp.find( ">" );
				string	strText		=	strTemp.substr( 2 , (nstringPos-2) );

				dwCustomColor		=	_HexStringToDWORD( strText.c_str() );
				pMessageEntry->m_dwCustomColor	=	dwCustomColor;

				nStartPos			=	i+1;
				strTemp.clear();

				continue;
			}

			string		strEventName;
			string		strControlName;

			stControlInfo*		pControlInfo	=	new stControlInfo;
			pControlInfo->m_pcsUIBase			=	new AcUIEventButton;
			pControlInfo->m_eType				=	EVENT_TYPE_NONE;

			if( m_unMessageType != CHAT_MESSAGE_SYSTEM )
				pControlInfo->m_eType				=	_CalcEventType( strTemp , pSenderName , pControlInfo , strEventName );

			if( pControlInfo->m_eType != EVENT_TYPE_NONE )
			{
				strControlName	=	szBuffer[nStartPos];
				strControlName	+=	strEventName;
				strControlName	+=	szBuffer[i];

				_AddClickAction( pControlInfo->m_pcsUIBase , pMessageEntry , pControlInfo->m_eType );
			}

			else
			{
				strControlName	=	strTemp;
			}

			nStringWidth = m_pcsAgcmFont->GetTextExtent( m_nFontType , (CHAR*)strControlName.c_str() , strlen( strControlName.c_str() ) );

			// 이벤트가 창 크기보다 커진다면 이름을 대체한다.
			// 아이템 , 좌표 <-- 이렇게 
			if( GetTextLength() <= nStringWidth )
			{
				switch( pControlInfo->m_eType )
				{
				case EVENT_TYPE_ITEM:
					strControlName	=	"<ITEM>";
					nStringWidth = m_pcsAgcmFont->GetTextExtent( m_nFontType , (CHAR*)strControlName.c_str() , strlen( strControlName.c_str() ) );
					break;
				case EVENT_TYPE_MAP_POS:
					strControlName	=	"<POS>";
					nStringWidth = m_pcsAgcmFont->GetTextExtent( m_nFontType , (CHAR*)strControlName.c_str() , strlen( strControlName.c_str() ) );
					break;
				}
			}

			// 이벤트는 문자열 크기 초과하면 작업을 중지하고 다음줄로 넘겨야 된다
			if( GetTextLength() < nTotalStringWidth+nStringWidth )
			{
				if( pControlInfo->m_eType != EVENT_TYPE_NONE )
				{
					bEnd		=	FALSE;
					nNextLine	=	nStartPos;
					break;
				}

				// 일반 문자열은 초과하면 가능한 줄까지 찍고 넘겨야 된다
				else
				{
					bEnd	=	FALSE;

					if( lineCheckByWord_ ) // 단어단위로 라인체크
					{
						//int beforeLength = (int)strControlName.length();

						int width = SliceLineByWord( strControlName, nTotalStringWidth );

						//int afterLength = (int)strControlName.length();

						//nLastPos -= (beforeLength - afterLength);
						nLastPos = nStartPos + width;

						if( nLastPos < 0 )
							nLastPos = 0;

						if( width < 1 )
						{
							bForceNextLine = TRUE;
							//--nLastPos;
							++nLastPos;
						}
						else
						{
							nStringWidth = m_pcsAgcmFont->GetTextExtent( 0 , (CHAR*)strControlName.c_str() , (int)strControlName.length() );
						}

						if( szBuffer[nLastPos] == ' ' )
							++nLastPos;

						nNextLine = nLastPos;
					}
					else
					{
						BOOL	bTwobyte	=	FALSE;
						INT		nLength		=	strControlName.length()-1;

						for( INT k = 1 ; ; ++k )
						{

							if( !bTwobyte )
							{
								if( nLength >= (k+1) )
								{
									// 2바이트 문자 처리
									if( strControlName[nLength-(k+1)] < 0 &&
										strControlName[nLength-k] < 0 )
									{
										//nOffset		=	1;
										bTwobyte	=	TRUE;

										continue;
									}
								}
							}

							bTwobyte	=	FALSE;

							INT	nWidth	=	m_pcsAgcmFont->GetTextExtent( 0 , (CHAR*)strControlName.c_str() , strlen( strControlName.c_str() )-k );
							if( nWidth+nTotalStringWidth <= GetTextLength() )
							{
								INT		nOffset		=	0;

								// 짤라서 찍어줄게 있을경우에는 찍어주고
								if( nWidth )
								{
									strControlName	=	strControlName.substr( 0 , strControlName.length()-(k+1) );
									nLastPos		-=	(k);
									nStringWidth	=	m_pcsAgcmFont->GetTextExtent( 0 , (CHAR*)strControlName.c_str() , strlen( strControlName.c_str() ) );
								}

								// 짤라 찍을 문자열이 없을 경우에는 바로 다음 라인으로 넘긴다
								else
								{
									nLastPos		-=	(k-1);
									bForceNextLine	=	TRUE;
								}

								nNextLine		=	nLastPos;
								break;

							}
						}
					}
				}
			}

			if( !bForceNextLine )
			{
				pControlInfo->m_pcsUIBase->SetStaticStringExt( (CHAR*)strControlName.c_str() , 1.0f , m_nFontType , pMessageEntry->m_dwCustomColor , true , ACUIBASE_HALIGN_CENTER , ACUIBASE_VALIGN_CENTER );
				pControlInfo->m_pcsUIBase->MoveWindow( nTotalStringWidth , 0 , pControlInfo->m_pcsUIBase->GetStringWidth() , 19 );

				pMessageEntry->m_listText.push_back( pControlInfo );

				nTotalStringWidth	+=	pControlInfo->m_pcsUIBase->GetStringWidth();
				nStartPos			=	i+1;

				if( pControlInfo->m_eType != EVENT_TYPE_WHISPER )
				{
					if( pstrNoEventString )
					{
						//(*pstrNoEventString )	+=	strControlName;
						*pstrNoEventString = *pstrNoEventString + " " + strControlName;
					}
				}
			}

			strTemp.clear();

			if( !bEnd )
				break;

		}
	}

	if( bNewMessage)
		PushNewMessage( (INT)m_dequeMessage.size() );

	m_dequeMessage.push_back( pMessageEntry );

	// 문자열이 길어서 다음문자열에 추가해야 된다.
	if( !bEnd )
		_PushAndParseString( &szBuffer[nNextLine] , pSenderName , pMessage , dwCustomColor , dwCreateTick , TRUE , bNewMessage , pstrNoEventString , ulCharType );

	return TRUE;
}

BOOL	AgcChatMessage::PushNewMessage( INT nIndex )
{

	m_dequeNewMessage.push_back( nIndex );

	return TRUE;
}

stChatMsgEntry*	AgcChatMessage::PopNewMessage( VOID )
{
	if( !m_dequeNewMessage.size() )
		return NULL;

	stChatMsgEntry*	pMsgEntry	=	GetMessage( m_dequeNewMessage[0] );

	m_dequeNewMessage.pop_front();

	return pMsgEntry;
}

eEventType	AgcChatMessage::_CalcEventType( const string& strString , CONST CHAR* pSenderName , stControlInfo* pControlInfo , OUT string& strEventName )
{
	if( pSenderName )
	{
		string	strTemp;

		strTemp	=	"[";
		strTemp +=	pSenderName;
		strTemp +=	"]";

		if( strString.find( strTemp ) != string::npos )
		{
			strEventName	=	pSenderName;
			return EVENT_TYPE_WHISPER;
		}
	}

	if( strString.find( "ITEM" ) != string::npos )
	{
		INT	nCIDStartPos	=	strString.find( "::" , 0 );
		nCIDStartPos += 2;

		INT	nCIDLastPos		=	strString.find( "::" , nCIDStartPos );
		nCIDLastPos	-= 1;

		INT	nNameStartPos	=	strString.find( "::" , nCIDLastPos+3 );
		nNameStartPos += 2;

		INT	nNameLastPos	=	strString.find( ">" , nNameStartPos );
		nNameLastPos -= 1;

		string	strCID	=	strString.substr( nCIDStartPos , (nCIDLastPos-nCIDStartPos)+1 );
		string	strName	=	strString.substr( (nCIDLastPos+3)+nNameStartPos-1 , nNameLastPos-((nCIDLastPos+3)+nNameStartPos)+2 );

		pControlInfo->m_pData1			=	(PVOID)atoi( strCID.c_str() );
		strEventName	=	strName;

		return EVENT_TYPE_ITEM;
	}

	if( strString.find( "MAPPOS" ) != string::npos )
	{

		INT nIDStartPos	=	strString.find( "::" , 0 );
		nIDStartPos += 2;

		INT nIDLastPos	=	strString.find( "::" , nIDStartPos );
		nIDLastPos -= 1;

		INT	nXStartPos	=	nIDLastPos+3;
		INT	nXLastPos		=	strString.find( "::" , nXStartPos );
		nXLastPos	-= 1;

		INT	nYStartPos	=	nXLastPos+3;
		INT	nYLastPos	=	strString.find( "::" , nYStartPos );
		nYLastPos -= 1;

		INT	nNameStartPos	=	nYLastPos+3;
		INT	nNameLastPos	=	strString.find( ">" , nNameStartPos );
		nNameLastPos -= 1;

		string	strID	=	strString.substr( nIDStartPos , nIDLastPos-nIDStartPos+1 );
		string	strX	=	strString.substr( nXStartPos , nXLastPos-nXStartPos+1 );
		string	strY	=	strString.substr( nYStartPos , nYLastPos-nYStartPos+1 );
		string	strName	=	strString.substr( nNameStartPos , nNameLastPos-nNameStartPos+1 );

		pControlInfo->m_pData1		=	(PVOID)atoi( strX.c_str() );
		pControlInfo->m_pData2		=	(PVOID)atoi( strY.c_str() );
		pControlInfo->m_pData3		=	(PVOID)atoi( strID.c_str() );
		strEventName				=	strName;

		return EVENT_TYPE_MAP_POS;
	}

	return EVENT_TYPE_NONE;
}

BOOL	AgcChatMessage::_AddClickAction( AcUIBase* pcsBase , stChatMsgEntry* pMsg , eEventType eEvent )
{
	AcUIEventButton*	pButton	=	dynamic_cast< AcUIEventButton* >(pcsBase);
	if( !pButton )
		return FALSE;

	pButton->SetCustomRender( TRUE );
	pButton->SetCallbackClickEvent( m_pManager , AgcChatManager::CBEventCallback );
	pButton->SetData( pMsg , NULL );

	switch( eEvent )
	{
	case EVENT_TYPE_ITEM:
		pButton->SetCustomBGColor( 0x400d4ef9);
		pButton->SetCustomLineColor( 0x400d4ef9 );
		break;
	case EVENT_TYPE_MAP_POS:
		pButton->SetCustomBGColor( 0x40e43251 );
		pButton->SetCustomLineColor( 0x40e43251 );
		break;
	case EVENT_TYPE_WHISPER:
		pButton->SetCustomBGColor( 0x406084d7 );
		pButton->SetCustomLineColor( 0xee6084d7 );
		break;
	}

	return TRUE;
}

VOID	AgcChatMessage::_OverFlowMessageDelete( VOID )
{
	INT nEraseCount	=	m_dequeMessage.size()	-	GetTextLine();
	if( nEraseCount > 0 )
	{
		for( INT i = 0 ; i < nEraseCount ; ++i )
		{
			stChatMsgEntry*	pEntry	=	m_dequeMessage[0];
			if( pEntry )
			{
				ListUIControlIter	EntryIter	=	pEntry->m_listText.begin();
				for( ; EntryIter != pEntry->m_listText.end() ; ++EntryIter )
				{
					DEF_SAFEDELETE( (*EntryIter)->m_pcsUIBase );
					DEF_SAFEDELETE( (*EntryIter) );
				}
			}

			DEF_SAFEDELETE( pEntry );
			m_dequeMessage.pop_front();
		}
	}

	nEraseCount	=	m_dequeOriginalMessage.size()	-	GetTextLine();
	if( nEraseCount > 0 )
	{
		for( INT i = 0 ; i < nEraseCount ; ++i )
		{
			m_dequeOriginalMessage.pop_front();
		}
	}
}

VOID	AgcChatMessage::_ClearEntryMessage( VOID )
{	
	INT	nCount	=	(INT)m_dequeMessage.size();
	for( INT i = 0 ; i < nCount ; ++i )
	{
		stChatMsgEntry*	pEntry	=	m_dequeMessage[i];
		if( pEntry )
		{
			ListUIControlIter	EntryIter	=	pEntry->m_listText.begin();
			for( ; EntryIter != pEntry->m_listText.end() ; ++EntryIter )
			{
				DEF_SAFEDELETE( (*EntryIter)->m_pcsUIBase );
				DEF_SAFEDELETE( (*EntryIter) );
			}
		}
		DEF_SAFEDELETE( pEntry );
	}

	m_dequeMessage.clear();
}

VOID	AgcChatMessage::ClearOriginalMessage( VOID )
{
	m_dequeOriginalMessage.clear();
}

VOID	AgcChatMessage::SetFontType( INT nFontType , BOOL bRefresh )
{
	m_nFontType		=	nFontType;
	if( bRefresh )
	{
		RefreshMessageEntry();
	}
}

DWORD	AgcChatMessage::_HexStringToDWORD( const char* szHexString )
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