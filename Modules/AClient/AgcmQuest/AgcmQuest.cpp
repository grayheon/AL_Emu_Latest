// AgcmQuest.cpp: implementation of the AgcmQuest class.
//
//////////////////////////////////////////////////////////////////////

#include "AgcmQuest.h"
#include "AgcmCharacter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AgcmQuest::AgcmQuest()
{
	SetModuleName("AgcmQuest");
	m_pcsAgpmQuest = NULL;
}

AgcmQuest::~AgcmQuest()
{

}

BOOL AgcmQuest::OnInit()
{
	return TRUE;
}

BOOL AgcmQuest::OnAddModule()
{
	m_pcsAgpmQuest = (AgpmQuest*)GetModule("AgpmQuest");

	if (!m_pcsAgpmQuest) return FALSE;

	return TRUE;
}

BOOL AgcmQuest::OnIdle(UINT32 ulClockCount)
{
	return TRUE;
}

BOOL AgcmQuest::OnDestroy()
{
	return TRUE;
}

BOOL AgcmQuest::SendPacketRequireQuest(INT32 lQuestTID, ApdEvent* pEvent, INT32 lCID)
{
	INT16 nPacketLength = 0;
	PVOID pvPacket = m_pcsAgpmQuest->MakePacketRequireQuest(&nPacketLength, pEvent, lCID, lQuestTID);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL bResult =  SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmQuest->m_csPacket.FreePacket(pvPacket);

	return bResult;	
}

BOOL AgcmQuest::SendPacketCompleteQuest(INT32 lQuestTID, ApdEvent* pEvent, INT32 lCID , INT nSelectItemIndex )
{
	INT16 nPacketLength = 0;
	PVOID pvPacket = m_pcsAgpmQuest->MakePacketQuestComplete(&nPacketLength, pEvent, lCID, lQuestTID , nSelectItemIndex );

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmQuest->m_csPacket.FreePacket(pvPacket);

	return bResult;
}

BOOL AgcmQuest::SendPacketQuestCancel(INT32 lQuestTID, INT32 lCID)
{
	INT16 nPacketLength = 0;
	PVOID pvPacket = m_pcsAgpmQuest->MakePacketQuestCancel(&nPacketLength, lCID, lQuestTID);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmQuest->m_csPacket.FreePacket(pvPacket);

	return bResult;
}

BOOL AgcmQuest::SendPacketCheckPoint(INT32 lQuestTID, ApdEvent* pEvent, INT32 lCID, INT32 lCheckPointIndex)
{
	INT16 nPacketLength = 0;
	PVOID pvPacket = m_pcsAgpmQuest->MakePacketRequireCheckPoint(&nPacketLength, lCID, pEvent, lQuestTID, lCheckPointIndex);

	if (!pvPacket || nPacketLength < 1)
		return FALSE;

	BOOL bResult = SendPacket(pvPacket, nPacketLength);
	m_pcsAgpmQuest->m_csPacket.FreePacket(pvPacket);

	return bResult;
}

BOOL AgcmQuest::SendPacketQuestJournalRegisterChange( INT lQuestTID , INT32 lCID , BOOL bJournalRegister )
{
	INT16		nPacketLength	=	0;
	PVOID		pvPacket		=	m_pcsAgpmQuest->MakePacketQuestRegisterJournal( &nPacketLength , lCID , lQuestTID , bJournalRegister );

	if( !pvPacket || nPacketLength < 1 )
		return FALSE;

	BOOL	bSendResult	=	SendPacket( pvPacket , nPacketLength );
	m_pcsAgpmQuest->m_csPacket.FreePacket( pvPacket );

	return bSendResult;
}

BOOL	AgcmQuest::ReadXMLData( CONST string& strFileName )
{
	AuXmlParser	parser;

	if( !parser.LoadXMLFile( strFileName ) )
		return FALSE;

	AuXmlNode*	pNode	=	parser.FirstChild( "Accept" );
	if( pNode )
	{
		AuXmlNode*	pNewNode	=	pNode->FirstChild( "NEW" );
		DWORD	dwColor;

		if( pNewNode )
		{
			dwColor	=	0xff<<24;
			dwColor	+=	(atoi( pNewNode->ToElement()->Attribute( "R" ) ) << 16);
			dwColor +=	(atoi( pNewNode->ToElement()->Attribute( "G" ) ) << 8);
			dwColor +=	atoi( pNewNode->ToElement()->Attribute( "B" ) );

			m_arrdwAcceptColor[ QUEST_ACCEPT_NEW_COLOR ]	=	dwColor;
		}

		AuXmlNode*	pIngNode	=	pNode->FirstChild( "ING" );
		if( pIngNode )
		{
			dwColor	=	0xff<<24;
			dwColor	+=	(atoi( pIngNode->ToElement()->Attribute( "R" ) ) << 16);
			dwColor	+=	(atoi( pIngNode->ToElement()->Attribute( "G" ) ) << 8);
			dwColor	+=	atoi( pIngNode->ToElement()->Attribute( "B" ) );

			m_arrdwAcceptColor[ QUEST_ACCEPT_ING_COLOR ]	=	dwColor;
		}

		AuXmlNode*	pCompleteNode	=	pNode->FirstChild( "COMPLETE" );
		if( pCompleteNode )
		{
			dwColor	=	0xff<<24;
			dwColor	+=	(atoi( pCompleteNode->ToElement()->Attribute( "R" ) ) << 16);
			dwColor	+=	(atoi( pCompleteNode->ToElement()->Attribute( "G" ) ) << 8);
			dwColor +=	atoi( pCompleteNode->ToElement()->Attribute( "B" ) );

			m_arrdwAcceptColor[ QUEST_ACCEPT_COMPLETE_COLOR ]	=	dwColor;
		}
	}

	pNode	=	parser.FirstChild( "Journal" );
	if( pNode )
	{
		DWORD		dwColor			=	0;
		AuXmlNode*	pMissionNode	=	pNode->FirstChild( "Mission" );
		if( pMissionNode )
		{
			dwColor	=	0xff<<24;
			dwColor	+=	(atoi( pMissionNode->ToElement()->Attribute( "R" ) ) << 16);
			dwColor	+=	(atoi( pMissionNode->ToElement()->Attribute( "G" ) ) << 8);
			dwColor +=	atoi( pMissionNode->ToElement()->Attribute( "B" ) );

			m_arrdwJournalColor[ QUEST_JOURNAL_MISSION_COLOR ]	=	dwColor;
		}

		AuXmlNode*	pEpicNode		=	pNode->FirstChild( "Epic" );
		if( pEpicNode )
		{
			dwColor	=	0xff<<24;
			dwColor	+=	(atoi( pEpicNode->ToElement()->Attribute( "R" ) ) << 16);
			dwColor	+=	(atoi( pEpicNode->ToElement()->Attribute( "G" ) ) << 8);
			dwColor +=	atoi( pEpicNode->ToElement()->Attribute( "B" ) );

			m_arrdwJournalColor[ QUEST_JOURNAL_EPIC_COLOR ]		=	dwColor;
		}
	}

	pNode	=	parser.FirstChild( "Main" );
	if( pNode )
	{
		AuXmlNode*	pColorNode	=	pNode->FirstChild( "Color" );
		for( ; pColorNode ; pColorNode	=	pColorNode->NextSibling("Color") )
		{
			stQuestNameMain	QuestNameColor;

			QuestNameColor.m_nInterval	=	atoi( pColorNode->ToElement()->Attribute( "Interval" ) );

			QuestNameColor.m_dwColor	=	0xff<<24;
			QuestNameColor.m_dwColor	+=	(atoi( pColorNode->ToElement()->Attribute( "R" ) ) << 16);
			QuestNameColor.m_dwColor	+=	(atoi( pColorNode->ToElement()->Attribute( "G" ) ) << 8);
			QuestNameColor.m_dwColor	+=	atoi( pColorNode->ToElement()->Attribute( "B" ) );

			m_listQuestNameMain.push_back( QuestNameColor );
		}

		AuXmlNode*	pScenarioColorNode	=	pNode->FirstChild( "EPICSCENARIO" );
		if( pScenarioColorNode )
		{
			DWORD		dwScenarioColor	=	0xff << 24;

			dwScenarioColor +=	(atoi(pScenarioColorNode->ToElement()->Attribute( "R" )) <<16 );
			dwScenarioColor +=	(atoi(pScenarioColorNode->ToElement()->Attribute( "G" )) <<8 );
			dwScenarioColor +=	atoi(pScenarioColorNode->ToElement()->Attribute( "B" ));

			m_arrdwCustomColor[ QUEST_CUSTOM_EPIC_SCENARIO_COLOR ]	=	dwScenarioColor;
		}

		AuXmlNode*	pMissionScenarioColorNode	=	pNode->FirstChild( "MISSIONSCENARIO" );
		if( pMissionScenarioColorNode )
		{
			DWORD		dwScenarioColor	=	0xff << 24;

			dwScenarioColor +=	(atoi(pMissionScenarioColorNode->ToElement()->Attribute( "R" )) <<16 );
			dwScenarioColor +=	(atoi(pMissionScenarioColorNode->ToElement()->Attribute( "G" )) <<8 );
			dwScenarioColor +=	atoi(pMissionScenarioColorNode->ToElement()->Attribute( "B" ));

			m_arrdwCustomColor[ QUEST_CUSTOM_MISSION_SCENARIO_COLOR ]	=	dwScenarioColor;
		}

		AuXmlNode*	pAreaColorNode	=	pNode->FirstChild( "AREA" );
		if( pAreaColorNode )
		{
			DWORD		dwScenarioColor	=	0xff << 24;

			dwScenarioColor +=	(atoi(pAreaColorNode->ToElement()->Attribute( "R" )) <<16 );
			dwScenarioColor +=	(atoi(pAreaColorNode->ToElement()->Attribute( "G" )) <<8 );
			dwScenarioColor +=	atoi(pAreaColorNode->ToElement()->Attribute( "B" ));

			m_arrdwCustomColor[ QUEST_CUSTOM_AREA_COLOR ]	=	dwScenarioColor;
		}
	}

	return TRUE;
}

DWORD	AgcmQuest::GetAcceptQuestNameColor( INT nQuestTemplateID )
{
	DWORD			dwColor			=	0;
	AgpdCharacter*	pSelfCharacter	=	m_pcsAgcmCharacter->GetSelfCharacter();

	if( pSelfCharacter )
	{
		if( m_pcsAgpmQuest->EvaluationCompleteCondition( pSelfCharacter , nQuestTemplateID ) )
		{
			return m_arrdwAcceptColor[ QUEST_ACCEPT_COMPLETE_COLOR ];
		}

		AgpdQuest*	pSelfQuest	=	m_pcsAgpmQuest->GetAttachAgpdQuestData( pSelfCharacter );
		if( pSelfQuest )
		{
			if( pSelfQuest->IsHaveQuest( nQuestTemplateID ) )
				return m_arrdwAcceptColor[ QUEST_ACCEPT_ING_COLOR ];

		}
	}

	return m_arrdwAcceptColor[ QUEST_ACCEPT_NEW_COLOR ];
}

DWORD	AgcmQuest::GetMainQuestNameColor( INT nQuestTemplateID )
{
	DWORD				dwColor			=	0;
	AgpdQuestTemplate*	pTemplate		=	m_pcsAgpmQuest->m_csQuestTemplate.Get( nQuestTemplateID );
	AgpdCharacter*		pSelfCharacter	=	m_pcsAgcmCharacter->GetSelfCharacter();
	if( pTemplate && pSelfCharacter )
	{
		INT32	nCharacterLevel	=	m_pcsAgpmFactors->GetLevel( &pSelfCharacter->m_csFactor );
		INT32	nQuestLevel		=	pTemplate->m_StartCondition.lLevel;
		INT32	nInterval		=	nCharacterLevel	- nQuestLevel;
		INT32	nTemp			=	9999;

		ListQuestNameMain::iterator	Iter	=	m_listQuestNameMain.begin();

		for( ; Iter != m_listQuestNameMain.end() ; ++Iter )
		{
			if( nInterval == Iter->m_nInterval )
			{
				dwColor	=	Iter->m_dwColor;
				break;
			}

			if( nTemp > abs(nInterval - Iter->m_nInterval) )
			{
				nTemp	=	nInterval-Iter->m_nInterval;
				dwColor	=	Iter->m_dwColor;
			}
		}

	}

	return dwColor;
}

DWORD	AgcmQuest::GetJournalQuestNameColor( INT nQuestTemplateID )
{
	DWORD			dwColor			=	0;

	AgpdQuestTemplate* pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(nQuestTemplateID);
	if (pQuestTemplate)
	{
		if( pQuestTemplate->m_QuestInfo.eQuestCategory	==	AGPDQUEST_SYSTEM_TYPE_SCENARIO )
		{
			return m_arrdwJournalColor[ QUEST_JOURNAL_MISSION_COLOR ];
		}
	}

	return m_arrdwJournalColor[ QUEST_JOURNAL_EPIC_COLOR ];
}

DWORD	AgcmQuest::GetCustomColor( eQuestCustomColor eCustomColor )
{
	return m_arrdwCustomColor[ eCustomColor ];
}