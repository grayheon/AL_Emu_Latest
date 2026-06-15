#pragma once

#include "AgcModule.h"
#include "ApMemory.h"
#include <list>
#include <map>
#include <queue>

#define		MAX_JOURNALINFO_COUNT	100
#define		MAX_JOURNAL_VIEW_COUNT	7


enum	eJournalType
{
	QUEST_JOURNAL_MISSION	=	0x00000001	,
	QUEST_JOURNAL_NORMAL	=	0x00000002	,

	QUEST_JOURNAL_ALL		=	QUEST_JOURNAL_MISSION | QUEST_JOURNAL_NORMAL	,
};

class AgpmQuest;
class AgcmCharacter;
class AgpdQuest;
class AgpdCharacter;
class AgpdCurrentQuest;
class AgcmQuest;
class AgcmUIQuest2;

struct	stJournalInfo	:	public	ApMemory< stJournalInfo , MAX_JOURNALINFO_COUNT >
{
	eJournalType	m_eType;
	DWORD			m_dwViewStartTime;

	INT				m_nQuestTemplateID;
	INT				m_nParam1;
	INT				m_nParam2;

	AgpdCurrentQuest*	m_pQuestInfo;

	BOOL			m_bJournalView;

	string			m_strQuestName;
	FLOAT			m_fAlpha;

	stJournalInfo( VOID )
	{
		m_eType				=	QUEST_JOURNAL_NORMAL;
		m_nQuestTemplateID	=	0;
		m_bJournalView		=	FALSE;
		m_dwViewStartTime	=	0;
		m_pQuestInfo		=	NULL;
		m_fAlpha			=	1.0f;
	}
};

struct	stPosInfo 
{
	INT			m_nPosX;
	INT			m_nPosY;

	string		m_strTooltip;
};

#define		ListPosInfo		std::list< stPosInfo >		

struct stJournalData
{
	string			strJournalText;
	ListPosInfo		m_listPosInfo;
};

class AgcmQuestJournal
	:	public	AgcModule
{
private:
	typedef	std::multimap< INT , stJournalInfo* >				MapQuestJournal;
	typedef	std::multimap< INT , stJournalInfo* >::iterator		MapQuestJournalIter;	

	typedef std::map< INT , stJournalData* >					MapQuestJournalText;
	typedef std::map< INT , stJournalData* >::iterator			MapQuestJournalTextIter;

	// 저장해 놓고 셀프캐릭터가 들어오면 처리될 데이타 구조
	struct	stProcessData 
	{
		eJournalType	eType;
		INT				nQuestTemplateID;
		string			strQuestName;
	};

	typedef	std::queue< stProcessData >							QueueProcessData;

public:
	AgcmQuestJournal								( VOID );
	virtual ~AgcmQuestJournal						( VOID );

	BOOL 					OnInit					( VOID );
	BOOL 					OnAddModule				( VOID );
	BOOL 					OnIdle					( UINT32 ulClockCount );	
	BOOL 					OnDestroy				( VOID );

	BOOL					ReadJournalText			( CONST string&	strFileName, BOOL bIsEncrypted );

	// Journal Clear
	VOID					Clear					( BOOL bSendPacket = FALSE );

	stJournalInfo*			AddQuestJournal			( CONST string&	strQuestName , eJournalType eType , INT nQuestTemplateID );

	BOOL					DelQuestJournal			( CONST string& strQuestName , BOOL bSendPacket = FALSE );
	BOOL					DelQuestJournal			( INT	nQuestTemplateID , BOOL bSendPacket = FALSE );

	stJournalInfo*			GetQuestJournal			( CONST string& strQuestName );
	stJournalInfo*			GetQuestJournal			( INT nIndex , eJournalType	eType = QUEST_JOURNAL_ALL );

	BOOL					MakeJournalText			( INT	nQuestTemplateID , CHAR* pTextBuffer , INT nSize );
	INT						GetMapPosition			( INT	nQuestTemplateID , INT& nPosX , INT& nPosY );

	BOOL					IsQuestJournal			( INT nQuestTemplateID );

	INT						GetQuestCount			( eJournalType eType = QUEST_JOURNAL_ALL );

	static BOOL				CBSetSelfCharacter		( PVOID pData , PVOID pClass , PVOID pCustData );
	static BOOL				CBReleaseSelfCharacter	( PVOID pData , PVOID pClass , PVOID pCustData );
	static BOOL				CBInitQuest				( PVOID pData , PVOID pClass , PVOID pCustData );	

	BOOL					SetCallbackRefresh		( ApModuleDefaultCallBack pfCallback , PVOID pClass );

protected:
	CONST CHAR*				_GetQuestJournalText	( INT nQuestTemplateID );
	VOID					_QueueProcess			( VOID );


	MapQuestJournal						m_mapQuestJournal;
	MapQuestJournalText					m_mapQuestJournalText;

	AgpdQuest*							m_pcsSelfQuest;
	AgpdCharacter*						m_pcsSelfCharacter;

	AgpmQuest*							m_pcsAgpmQuest;
	AgcmCharacter*						m_pcsAgcmCharacter;

	AgcmUIQuest2*						m_pcsAgcmUIQuest2;
	AgcmQuest*							m_pcsAgcmQuest;

	QueueProcessData					m_queueProcessData;
};
