// AgcmQuest.h: interface for the AgcmQuest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AGCMQUEST_H__4DCE4603_C36E_402F_A583_43F482A93232__INCLUDED_)
#define AFX_AGCMQUEST_H__4DCE4603_C36E_402F_A583_43F482A93232__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AgpmQuest.h"
#include "AgcModule.h"

#if _MSC_VER < 1300
#ifdef	_DEBUG
#pragma comment ( lib , "AgcmQuestD" )
#else
#pragma comment ( lib , "AgcmQuest" )
#endif
#endif

enum
{
	QUEST_ACCEPT_NEW_COLOR		,
	QUEST_ACCEPT_ING_COLOR		,
	QUEST_ACCEPT_COMPLETE_COLOR	,

	QUEST_ACCEPT_COLOR_COUNT	,
};

enum
{
	QUEST_JOURNAL_MISSION_COLOR	,
	QUEST_JOURNAL_EPIC_COLOR	,

	QUEST_JOURNAL_COLOR_COUNT	,
};

enum	eQuestCustomColor
{
	QUEST_CUSTOM_EPIC_SCENARIO_COLOR	,
	QUEST_CUSTOM_MISSION_SCENARIO_COLOR ,
	QUEST_CUSTOM_AREA_COLOR		,
	QUEST_CUSTOM_COUNT			,
};

struct	stQuestNameMain 
{
	INT		m_nInterval;
	DWORD	m_dwColor;
};

class AgcmCharacter;

class AgcmQuest : public AgcModule
{
private:
	typedef	std::list< stQuestNameMain>		ListQuestNameMain;

public:
	AgpmQuest*	m_pcsAgpmQuest;
	AgpmFactors*		m_pcsAgpmFactors;

	AgcmCharacter*		m_pcsAgcmCharacter;

	DWORD				m_arrdwAcceptColor	[ QUEST_ACCEPT_COLOR_COUNT ];		//	수락 & 완룍 창 퀘스트 이름 색깔
	DWORD				m_arrdwJournalColor	[ QUEST_JOURNAL_COLOR_COUNT ];		//	저널 창 퀘스트 이름 색깔
	DWORD				m_arrdwCustomColor	[ QUEST_CUSTOM_COUNT ];				//	이것저것

	ListQuestNameMain	m_listQuestNameMain;									//	커뮤니티창 퀘스트 이름 색깔

public:
	AgcmQuest();
	virtual ~AgcmQuest();

	BOOL OnInit();
	BOOL OnAddModule();
	BOOL OnIdle(UINT32 ulClockCount);	
	BOOL OnDestroy();

	BOOL		ReadXMLData				( const string& strFileName );

	DWORD		GetAcceptQuestNameColor	( INT nQuestTemplateID );
	DWORD		GetMainQuestNameColor	( INT nQuestTemplateID );
	DWORD		GetJournalQuestNameColor( INT nQuestTemplateID );

	DWORD		GetCustomColor			( eQuestCustomColor eCustomColor );


	// Callback function
	BOOL SetCallbackNotifyNewQuest(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallbackNotifyIncompleteQuest(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallbackNotifyCompleteQuest(ApModuleDefaultCallBack pfCallback, PVOID pClass);

	// Send Packet
	BOOL SendPacketRequireQuest(INT32 lQuestTID, ApdEvent* pEvent, INT32 lCID);
	BOOL SendPacketCompleteQuest(INT32 lQuestTID, ApdEvent* pEvent, INT32 lCID , INT nSelectItemIndex = -1);
	BOOL SendPacketQuestCancel(INT32 lQuestTID, INT32 lCID);
	BOOL SendPacketCheckPoint(INT32 lQuestTID, ApdEvent* pEvent, INT32 lCID, INT32 lCheckPointIndex);

	BOOL	SendPacketQuestJournalRegisterChange( INT lQuestTID , INT32 lCID , BOOL bJournalRegister );
};

#endif // !defined(AFX_AGCMQUEST_H__4DCE4603_C36E_402F_A583_43F482A93232__INCLUDED_)
