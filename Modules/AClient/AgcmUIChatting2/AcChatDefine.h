#pragma once

#include "ApMemory.h"
#include "AgcUIManager.h"

#include <list>
#include <map>

#define		CHATTING_MESSAGE_LIFETIME				30000
#define		MESSAGEBOX_MAX_ENTRY					400
#define		MESSAGEBOX_CONTROL_COUNT_MAX			(MESSAGEBOX_MAX_ENTRY*5)
#define		MESSAGE_COUNT_MAX						100
#define		MAX_LISTBOX_STRING						335
#define		MAX_VIEW_LIST_ITEM						10
#define		MAX_TAB_BUTTON							8

#define		MIN_CHAT_SIZE_X							200
#define		MIN_CHAT_SIZE_Y							200

#define		MAX_CHAT_SIZE_X							700
#define		MAX_CHAT_SIZE_Y							700

#define		DRAG_START_DISTANCE						30

#define		CHAT_FONT_HEIGHT						20

#define		SCROLL_SIZE_X							17

enum
{
	CHAT_MESSAGE_NORMAL		=	1 << 0	,		//	일반 메세지
	CHAT_MESSAGE_GUILD		=	1 << 1	,		//	길드 메세지
	CHAT_MESSAGE_PARTY		=	1 << 2	,		//	파티 메세지	
	CHAT_MESSAGE_WHISPER	=	1 << 3	,		//	귓속말
	CHAT_MESSAGE_SYSTEM		=	1 << 4	,		//	시스템 메세지
	CHAT_MESSAGE_NOTICE		=	1 << 5	,		//	공지사항
	CHAT_MESSAGE_QUEST		=	1 << 6	,		//	퀘스트 메세지
	CHAT_MESSAGE_SIEGE		=	1 << 7	,		//	공성 메세지
	CHAT_MESSAGE_ARCHLORD	=	1 << 8 ,		//	아크로드 메시지
	CHAT_MESSAGE_SHOUT		=	1 << 9 ,		//	외치기
	CHAT_MESSAGE_RACE		=	1 << 10 ,		//	종족나팔
	CHAT_MESSAGE_ELEMENTAL	=	1 << 11 ,		//	정령의 외침(세계외침)

	CHAT_MESSAGE_COUNT		=	12		,
};

#define		CHAT_MESSAGE_ALL	CHAT_MESSAGE_NORMAL | CHAT_MESSAGE_GUILD | CHAT_MESSAGE_PARTY | \
	CHAT_MESSAGE_WHISPER | CHAT_MESSAGE_SYSTEM | CHAT_MESSAGE_SYSTEM | \
	CHAT_MESSAGE_NOTICE | CHAT_MESSAGE_QUEST | CHAT_MESSAGE_SIEGE | \
	CHAT_MESSAGE_ARCHLORD | CHAT_MESSAGE_SHOUT | CHAT_MESSAGE_RACE | CHAT_MESSAGE_ELEMENTAL

enum
{
#ifdef _AREA_GLOBAL_
	FONT_SIZE_12			=	2	,
	FONT_SIZE_13			=	0	,
	FONT_SIZE_14			=	9	,
	FONT_SIZE_15			=	4	,
#else
	FONT_SIZE_11			=	2	,
	FONT_SIZE_12			=	1	,
	FONT_SIZE_13			=	0	,
	FONT_SIZE_14			=	9	,
	FONT_SIZE_15			=	4	,
#endif
};

enum eEventType
{
	EVENT_TYPE_NONE		,	
	EVENT_TYPE_ITEM		,	//	아이템 정보 이벤트
	EVENT_TYPE_MAP_POS	,	//	맵 정보 이벤트
	EVENT_TYPE_WHISPER	,	//	귓속말 이벤트
};

enum eDockingEventType
{
	EVENT_DOCKING_TYPE_COMBINE		,	//	합쳐진다
	EVENT_DOCKING_TYPE_SEPARATION	,	//	분리된다

	EVENT_DOCKING_TYPE_IMAGE_HIDE	,	//	이미지들이 점점 사라진다
	EVENT_DOCKING_TYPE_IMAGE_SHOW	,	//	이미지들이 점점 보여진다
};

enum eTabOperation
{
	E_TAB_OPTION_ADD		,		//	탭 생성 작업을 진행중이다.
	E_TAB_OPTION_EDIT		,		//	탭 정보 수정을 진행중이다.
};

// 나중에 갱신할때 사용( 사이즈 줄이고 늘리고 )
struct	stOriginalMessageInfo
{
	string					m_strSenderName;
	string					m_strMessage;

	string					m_strNoEventMessage;

	DWORD					m_dwCustomColor;
	DWORD					m_dwCreateTick;

	UINT32					m_ulCharType;
};

struct	stControlInfo
	:	public	ApMemory< stControlInfo , MESSAGEBOX_CONTROL_COUNT_MAX >
{
	AcUIBase*				m_pcsUIBase;				//	컨트롤
	eEventType				m_eType;					//	이벤트 타입

	PVOID					m_pData1;					//	Item이벤트에서 CID로 사용 , Map이벤트에서 X좌표로 사용
	PVOID					m_pData2;					//	Map이벤트에서 Z좌표로 사용
	PVOID					m_pData3;					//	Map이벤트에서 ID로 사용

	stControlInfo( VOID )
	{
		m_pcsUIBase		=	NULL;
		m_pData1		=	NULL;
		m_pData2		=	NULL;
		m_pData3		=	NULL;

		m_eType			=	EVENT_TYPE_NONE;
	}

};

#define		ListUIControl			std::list< stControlInfo* >
#define		ListUIControlIter		std::list< stControlInfo* >::iterator

struct	stChatMsgEntry
	:	public ApMemory< stChatMsgEntry , MESSAGEBOX_MAX_ENTRY >
{
	BOOL						m_bNewLine;				//	TRUE가 되면 줄바꿈이 된 라인이다.

	UINT						m_eType;
	DWORD						m_dwCustomColor;

	std::string					m_strSenderName;
	std::string					m_strMessage;
	ListUIControl				m_listText;

	std::string					m_strString;

	DWORD						m_dwCreateTick;

	stChatMsgEntry( VOID )
	{
		m_eType			=	CHAT_MESSAGE_NORMAL;
		m_dwCustomColor	=	0;
		m_bNewLine		=	FALSE;
		m_dwCreateTick	=	0;

		m_listText.clear();
	}
};

struct	stChatUsingMsgEntry 
{
	stChatMsgEntry*		m_pEntry;

	stChatUsingMsgEntry( VOID )
	{
		m_pEntry	=	NULL;
	}

	stChatUsingMsgEntry( stChatMsgEntry* pEntry )
	{
		m_pEntry	=	pEntry;
	}

	bool	operator ()	( const stChatUsingMsgEntry& LeftEntry , const stChatUsingMsgEntry& RightEntry )	const 
	{
		return LeftEntry.m_pEntry->m_dwCreateTick < RightEntry.m_pEntry->m_dwCreateTick;
	}

	bool	operator <	( const stChatUsingMsgEntry& rEntry ) const
	{
		return (m_pEntry->m_dwCreateTick < rEntry.m_pEntry->m_dwCreateTick);
	}
};

struct	stChatTextFormat 
{
	BOOL						m_bIsUseHeader;
	BOOL						m_bIsUseSenderName;

	std::string					m_strHeader;
	std::string					m_strFormat;

	DWORD						m_dwTextColor;

	stChatTextFormat( VOID )
	{
		m_bIsUseHeader			=	FALSE;
		m_bIsUseSenderName		=	TRUE;

		m_dwTextColor			=	0xffffffff;
	}

};

struct	stDockingEvent 
{
	eDockingEventType	m_eEventType;

	DWORD			m_dwStartTime;
	DWORD			m_dwDuration;

	PVOID			m_pDataSrc;
	PVOID			m_pDataDst;

	FLOAT			m_fAlpha;

	stDockingEvent()
	{
		m_eEventType	=	EVENT_DOCKING_TYPE_COMBINE;

		m_dwDuration	=	0;
		m_dwStartTime	=	0;

		m_pDataDst		=	NULL;
		m_pDataSrc		=	NULL;
		m_fAlpha		=	NULL;
	}
};

struct	stChatWindowInfo
{
	INT				m_nIndex;
	RwV2d			m_vPosition;
	RwV2d			m_vSize;

	UINT			m_unFontSize;

	BOOL			m_bTabEnable;


	string			m_strTabName[ MAX_TAB_BUTTON ];		//	탭 이름
	UINT			m_ulTabType[ MAX_TAB_BUTTON ];		//	탭이 가진 메세지 종류
	BOOL			m_bPrimary[ MAX_TAB_BUTTON ];		//	이 옵션을 가지면 수정도 안되고 지워지지도 않는다.

	stChatWindowInfo()
	{
		m_nIndex			=	0;
		m_unFontSize		=	0;
		m_bTabEnable		=	0;

		m_vPosition.x		=	0;
		m_vPosition.y		=	0;
		m_vSize.x			=	0;
		m_vSize.y			=	0;

		ZeroMemory( m_ulTabType , sizeof(UINT)*MAX_TAB_BUTTON );
		ZeroMemory( m_bPrimary , sizeof(BOOL)*MAX_TAB_BUTTON );
	}

};