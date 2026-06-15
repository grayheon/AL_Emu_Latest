#pragma once

#include "AgcModule.h"
#include "AcUIBase.h"

#define		MAX_NOTICE_MESSAGE_VIEW			5
#define		NOTICE_MESSAGE_LIFE_TIME		8000
#define		NOTICE_MESSAGE_FADE_TIME		3000

class AgcmUIManager2;
class AgcmFont;
class AgcmUIChatting2;

class AgcmUINotice 
	: 	public AgcModule 
{
private:
	struct	stNoticeMessage 
	{
		DWORD		m_dwStartTime;
		FLOAT		m_fAlpha;

		string		m_strString;
	};

	typedef		std::list< stNoticeMessage* >			ListNoticeOriginalString;
	
public:
	AgcmUINotice			( VOID );
	virtual ~AgcmUINotice	( VOID );

	VOID				DBG_Message		( VOID );

	VOID				NoticeMacro		( CONST string&	strXMLFile , BOOL bTest = FALSE );

	virtual BOOL		OnAddModule		( VOID );
	virtual BOOL		OnInit			( VOID );
	virtual BOOL		OnIdle			( UINT32 ulClockCount );
	virtual BOOL		OnDestroy		( VOID );

	BOOL				AddMessage		( CONST string&	strString );

	VOID				RefreshMessage	( VOID );

private:
	VOID				_RefreshPosition	( VOID );
	VOID				_ClearChildWindow	( VOID );
	VOID				_ParseMessage		( CONST string&	strString , AcUIBase*	pUIBase );
	DWORD				_HexStringToDWORD	( CONST CHAR* szHexString );

	ListNoticeOriginalString		m_listOriginal;
	ListNoticeOriginalString		m_listMacroMessage;

	// 현재 매크로가 돌아가는 중이면 TRUE
	BOOL							m_bStartMacro;
	BOOL							m_bMacroTest;

	AcUIBase*						m_arrpUIBase	[ MAX_NOTICE_MESSAGE_VIEW ];

	AgcmUIManager2*					m_pcsAgcmUIManager2;
	AgcmFont*						m_pcsAgcmFont;
	AgcmUIChatting2*				m_pcsAgcmUIChatting2;
};