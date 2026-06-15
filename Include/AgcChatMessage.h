#pragma once

#include "AcChatDefine.h"
#include <map>

class AgcmFont;

class AgcChatMessage
{
private:
	typedef std::deque< stChatMsgEntry* >				DequeMsgEntry;			//	파싱된 메세지
	typedef std::deque< stOriginalMessageInfo >			DequeOriginalMsg;		//	오리지날 메세지

	typedef std::deque< INT >							DequeNewMessage;		//	새롭게 추가된 메세지

	typedef std::map< UINT , stChatTextFormat >			MapTextFormat;

public:
	AgcChatMessage								( VOID );
	~AgcChatMessage								( VOID );

	BOOL				Initialize				( VOID );
	VOID				Clear					( VOID );

	stOriginalMessageInfo*	AddMessage				( CHAR* pSenderName , CHAR* pMessage , DWORD dwCustomColor = 0 , UINT32 ulCharType = AGPMCHAR_TYPE_NONE );
	stChatMsgEntry*			GetMessage				( INT nIndex );
	VOID					ClearMessage			( VOID );

	VOID					ClearOriginalMessage	( VOID );

	BOOL				PushNewMessage			( INT nIndex );
	stChatMsgEntry*		PopNewMessage			( VOID );

	VOID				RefreshMessageEntry		( VOID );

	VOID				SetManager				( PVOID pManager	)		{	m_pManager				=	pManager;			}
	VOID				SetMessageType			( UINT unMessageType )		{	m_unMessageType			=	unMessageType;		}
	VOID				SetTextLength			( INT	nLength )			{	m_nMaxTextLength		=	nLength;			}
	VOID				SetTextLine				( INT	nLine	)			{	m_nMaxTextLine			=	nLine;				}
	VOID				SetEnable				( BOOL	bEnable )			{	m_bEnable				=	bEnable;			}

	VOID				SetTextBoardMessage		( BOOL	bTextBoardMessage )	{	m_bTextBoardMessage		=	bTextBoardMessage;	}

	VOID				SetName					( CONST string&	strName )	{	m_strName				=	strName;			}
	VOID				SetFontType				( INT nFontType , BOOL bRefresh = FALSE );


	INT					GetMessageCount			( VOID );

	BOOL				GetTextBoardMessage		( VOID )					{	return m_bTextBoardMessage;						}
	INT					GetTextLength			( VOID )					{	return m_nMaxTextLength;						}
	INT					GetTextLine				( VOID )					{	return m_nMaxTextLine;							}
	UINT				GetMessageType			( VOID )					{	return m_unMessageType;							}
	string				GetName					( VOID )					{	return m_strName;								}

	BOOL				IsEnable				( VOID )					{	return m_bEnable;								}


	static VOID					SetTextFormat	( UINT unMessageType , CONST CHAR* szFormat , CONST CHAR* szHeader , BOOL bUseHeader , BOOL bUseSenderName , DWORD dwTextColor );
	static stChatTextFormat*	GetTextFormat	( UINT unMessageType );

	static BOOL					CBEventCallback	( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );

private:
	VOID				_ClearEntryMessage		( VOID );

	VOID				_StringCombination		( OUT CHAR* szBuffer , INT nMaxLength , stChatTextFormat*	pChatTextFormat , CONST CHAR* szName , CONST CHAR* szMessage );
	BOOL				_PushAndParseString		( CHAR* szBuffer , char* pSenderName, char* pMessage  , DWORD dwCustomColor , DWORD dwCreateTick , BOOL bNewLine = FALSE , BOOL bNewMessage = FALSE , string* pstrNoEventString = NULL , UINT32 ulCharType = AGPMCHAR_TYPE_NONE );
	eEventType			_CalcEventType			( const string& strString , CONST CHAR* pSenderName , stControlInfo* pControlInfo , OUT string& strEventName );
	BOOL				_AddClickAction			( AcUIBase* pcsBase , stChatMsgEntry* pMsg , eEventType eEvent );

	VOID				_OverFlowMessageDelete	( VOID );

	VOID				_ItemEvent				( INT nID );
	VOID				_WhisperEvent			( CONST CHAR* szBuffer );
	VOID				_MapPosEvent			( CONST CHAR* szBuffer , INT nX , INT nY , INT nID );

	DWORD				_HexStringToDWORD		( const char* szHexString );

	int					SliceLineByWord		( std::string & str, int totalWidth, size_t count = 0 );

protected:
	INT						m_nMaxTextLength;		//	한줄에 표현 할 수 있는 최대 길이
	INT						m_nMaxTextLine;			//	최대 메세지 갯수
	UINT					m_unMessageType;		//	어떤 종류의 메세지를 보관하는 곳인가

	BOOL					m_bTextBoardMessage;	//	이 메세지는 텍스트보드에 찍혀야 되는지
	BOOL					m_bEnable;				//	활성화 / 비활성화

	string					m_strName;				//	이름
	INT						m_nFontType;			//	현재 사용 되는 폰트 타입

	PVOID					m_pManager;

	DequeMsgEntry			m_dequeMessage;				//	파싱 된 메세지
	DequeOriginalMsg		m_dequeOriginalMessage;		//	오리지날 메세지
	DequeNewMessage			m_dequeNewMessage;			//	추가된 메세지( AgcUIChatMessage에서 사용 )

	static MapTextFormat	m_mapTextFormat;

	bool					lineCheckByWord_;

	AgcmFont *				m_pcsAgcmFont;
};
