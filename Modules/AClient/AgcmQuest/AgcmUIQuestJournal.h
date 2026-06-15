#pragma once

#include "AgcModule.h"
#include <map>

#include "AgcmQuestJournal.h"
#include "AcUIEventButton.h"

#include "AcUIEdit.h"

#include "AcUIJournal.h"

#include <memory>

//-----------------------------------------------------------------------
//

class AgcmUIManager2;
class AgcdUI;
class AgpmQuest;
class AgcmUIQuest2;
class AgcUIQuestText;

//-----------------------------------------------------------------------
//

class JournalText
{
public:
	AgcUIQuestText* questText_;
	AcUIBase* uiBase_;

	JournalText();
	~JournalText();

	void init( float * alpha = 0 );
	int refresh( char const * msg, int questTID = 0 );
	void alpha( float * alpha );

private:
	JournalText( JournalText const & other );
	void operator=( JournalText const & other );
};

//-----------------------------------------------------------------------
//

class AgcmUIQuestJournal
	:	public	AgcModule
{
private:
	typedef	std::multimap< UINT , stJournalInfo* >		MapJournalInfo;

public:
	AgcmUIQuestJournal									( VOID );
	~AgcmUIQuestJournal									( VOID );

	BOOL						OnInit					( VOID );
	BOOL						OnAddModule				( VOID );
	BOOL						OnIdle					( UINT32 ulClockCount );
	BOOL						OnDestroy				( VOID );

	BOOL						LoadSettingData			( CONST string&	strFileName );

	VOID						OpenJournalUI			( VOID );

	// 현재 보여지고 있는 Journal을 모두 안 보이게 빼버린다.
	VOID						ClearJournal			( VOID );

	// 현재 표시되야 하는 저널을 갱신한다.
	VOID						RefreshJournalInfo		( VOID );

	// 저널들을 정렬 시킨다
	VOID						RefreshJournalPosition	( VOID );

	VOID						RefreshTempJournalAlpha	( VOID );

	static BOOL					CBRefreshJournalInfo	( PVOID pData , PVOID pClass , PVOID pCustData );

	static BOOL					CBClickJournalButton1	( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL					CBClickJournalButton2	( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL					CBClickJournalButton3	( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );

	static BOOL					CBJournalPositionLinkEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );

protected:
	// RefreshJournalInfo에서 사용

	BOOL						_RefreshText			( stJournalInfo*	pJournalInfo , INT nIndex );
	BOOL						_RefreshTempText		( stJournalInfo*	pJournalInfo , INT nIndex );
	VOID						_ClearTempText			( VOID );

	MapJournalInfo				m_mapTempJournalInfo;		//	꽉차서 못 들어간 저널들( 금방 없어진다 )

	AgcmQuestJournal*			m_pcsAgcmQuestJournal;
	AgcmUIManager2*				m_pcsAgcmUIManager2;
	AgpmQuest*					m_pcsAgpmQuest;
	AgcmUIQuest2*				m_pcsAgcmUIQuest2;

	AgcdUI*						m_pJournalUI;

	AcUIJournal*				m_pJournalChildUI	[ MAX_JOURNAL_VIEW_COUNT ];
	JournalText*				m_pJournalText		[ MAX_JOURNAL_VIEW_COUNT ];

	AcUIJournal*				m_pJournalTempChildUI	[ MAX_JOURNAL_VIEW_COUNT ];
	JournalText*				m_pJournalTempText		[ MAX_JOURNAL_VIEW_COUNT ];
	FLOAT						m_fJournalTempAlpha		[ MAX_JOURNAL_VIEW_COUNT ];

	AcUIEventButton*			m_pJournalButton1	[ MAX_JOURNAL_VIEW_COUNT ];
	AcUIEventButton*			m_pJournalButton2	[ MAX_JOURNAL_VIEW_COUNT ];
	AcUIEventButton*			m_pJournalButton3	[ MAX_JOURNAL_VIEW_COUNT ];

	INT32						m_nUIBuffDefaultX, m_nUIBuffDefaultY;		// 버프리스트의 초기위치
	BOOL						m_bIsInitialized;
};

