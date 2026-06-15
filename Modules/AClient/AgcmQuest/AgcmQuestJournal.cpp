#include "AgcmQuestJournal.h"
#include "AgpmQuest.h"
#include "AgcmCharacter.h"

#include "AgcmQuest.h"
#include "AgcmUIQuest2.h"

#define	 AGCMQUEST_JOURNAL_CB_REFRESH		5

AgcmQuestJournal::AgcmQuestJournal( VOID )
{
	SetModuleName("AgcmQuestJournal");

	m_pcsAgpmQuest		=	NULL;
	m_pcsAgcmCharacter	=	NULL;

	m_pcsAgcmUIQuest2	=	NULL;

	m_pcsSelfCharacter	=	NULL;
	m_pcsSelfQuest		=	NULL;
}

AgcmQuestJournal::~AgcmQuestJournal( VOID )
{
	Clear();
}

BOOL	AgcmQuestJournal::OnInit( VOID )
{
	m_pcsAgpmQuest		=	static_cast< AgpmQuest* >( GetModule( "AgpmQuest" ) );
	ASSERT( m_pcsAgpmQuest );

	m_pcsAgcmCharacter	=	static_cast< AgcmCharacter* >( GetModule( "AgcmCharacter" ) );
	ASSERT( m_pcsAgcmCharacter );

	m_pcsAgcmUIQuest2	=	static_cast< AgcmUIQuest2* >( GetModule( "AgcmUIQuest2" ) );
	ASSERT( m_pcsAgcmUIQuest2 );

	m_pcsAgcmQuest		=	static_cast< AgcmQuest* >( GetModule( "AgcmQuest" ) );
	ASSERT( m_pcsAgcmQuest );

	m_pcsAgcmCharacter->SetCallbackSetSelfCharacter( CBSetSelfCharacter , this );
	m_pcsAgcmCharacter->SetCallbackReleaseSelfCharacter( CBReleaseSelfCharacter , this );
	m_pcsAgpmQuest->SetCallbackInitQuest( CBInitQuest , this );

	return TRUE;
}

BOOL	AgcmQuestJournal::OnAddModule( VOID )
{
	return TRUE;
}

BOOL	AgcmQuestJournal::OnIdle( UINT32 ulClockCount )
{
	return TRUE;
}

BOOL	AgcmQuestJournal::OnDestroy( VOID )
{
	Clear();

	return TRUE;
}

VOID	AgcmQuestJournal::Clear( BOOL bSendPacket )
{
	AgpdCharacter*	pcsSelfCharacter	=	m_pcsAgcmCharacter->GetSelfCharacter();


	MapQuestJournalIter	Iter	=	m_mapQuestJournal.begin();
	for( ; Iter != m_mapQuestJournal.end() ; ++Iter )
	{
		if( bSendPacket && pcsSelfCharacter )
		{
			m_pcsAgcmQuest->SendPacketQuestJournalRegisterChange( Iter->second->m_nQuestTemplateID , pcsSelfCharacter->GetID() , FALSE );
		}

		DEF_SAFEDELETE( Iter->second );
	}
	m_mapQuestJournal.clear();

	EnumCallback( AGCMQUEST_JOURNAL_CB_REFRESH , NULL , NULL );
}

BOOL AgcmQuestJournal::ReadJournalText( CONST string& strFileName, BOOL bIsEncrypted )
{
	AuExcelTxtLib	csExcelTxtLib;

	if (!csExcelTxtLib.OpenExcelFile( strFileName.c_str() , TRUE, bIsEncrypted) )
		return FALSE;

	INT32			lRow			= csExcelTxtLib.GetRow();
	INT32			lColumn			= csExcelTxtLib.GetColumn();
	string			strTemp;
	string			strTemp2;

	if (lRow <= 0 || lColumn <= 0)
		return FALSE;

	for( INT k = 2 ; k < lRow ; ++k )
	{
		INT				nQuestTID		=	csExcelTxtLib.GetDataToInt( 0 , k );
		stJournalData*	pJournalData	=	NULL;
		BOOL			bAdd			=	FALSE;

		MapQuestJournalTextIter	Iter		=	m_mapQuestJournalText.find( nQuestTID );
		if( Iter != m_mapQuestJournalText.end() )
		{
			pJournalData	=	Iter->second;
		}

		else
		{
			pJournalData	=	new stJournalData;
			bAdd			=	TRUE;
		}

		CHAR*	pData	=	csExcelTxtLib.GetData( 1 , k );
		if( pData )
			pJournalData->strJournalText	=	pData;

		else
			pJournalData->strJournalText	=	"";


		if( bAdd )
			m_mapQuestJournalText.insert( make_pair( nQuestTID , pJournalData ) );
	}


	return TRUE;
}

stJournalInfo*	AgcmQuestJournal::AddQuestJournal( CONST string& strQuestName , eJournalType eType , INT nQuestTemplateID )
{
	if( !strQuestName.length() )
		return NULL;

	// 셀프캐릭터가 들어오면 처리하도록 다른 곳으로 빼 놓는다
	if( !m_pcsSelfQuest )
	{
		stProcessData	stData;

		stData.eType			=	eType;
		stData.nQuestTemplateID	=	nQuestTemplateID;
		stData.strQuestName		=	strQuestName;

		m_queueProcessData.push( stData );

		return NULL;
	}

	AgpdCurrentQuest*	pQuestInfo	=	m_pcsSelfQuest->GetQuest( nQuestTemplateID );
	if( !pQuestInfo )
		return NULL;

	AgpdQuestTemplate* pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get( nQuestTemplateID );
	if( !pQuestTemplate )
		return NULL;

	INT	nStartLevel	=	pQuestTemplate->m_StartCondition.lLevel;

	if(IsQuestJournal(nQuestTemplateID))	// 이미 저널에 추가된 퀘스트는 추가하지 않습니다. 2011.11.18 Bischoff
	{
		return NULL;
	}

	stJournalInfo*		pJournalInfo	=	new stJournalInfo;
	ASSERT( pJournalInfo );

	pJournalInfo->m_strQuestName		=	strQuestName;
	pJournalInfo->m_eType				=	eType;
	pJournalInfo->m_dwViewStartTime		=	g_pEngine->GetTickCount();

	pJournalInfo->m_nQuestTemplateID	=	nQuestTemplateID;
	pJournalInfo->m_pQuestInfo			=	pQuestInfo;


	if( GetQuestCount() >= MAX_JOURNAL_VIEW_COUNT )
	{
		EnumCallback( AGCMQUEST_JOURNAL_CB_REFRESH , pJournalInfo , (PVOID)TRUE );
		return NULL;
	}

	pQuestInfo->bRegisterJournal		=	TRUE;

	m_mapQuestJournal.insert( make_pair( nStartLevel , pJournalInfo ) );

	EnumCallback( AGCMQUEST_JOURNAL_CB_REFRESH , NULL , NULL );

	return pJournalInfo;
	
}

BOOL	AgcmQuestJournal::DelQuestJournal( CONST string& strQuestName , BOOL bSendPacket )
{
	AgpdCharacter*	pSelfCharacter	=	m_pcsAgcmCharacter->GetSelfCharacter();

	MapQuestJournalIter	Iter	=	m_mapQuestJournal.begin();
	for( ; Iter != m_mapQuestJournal.end() ; ++Iter )
	{
		stJournalInfo*	pInfo	=	Iter->second;
		if( pInfo->m_strQuestName	== strQuestName  )
		{
			AgpdCurrentQuest*	pQuestInfo	=	m_pcsSelfQuest->GetQuest( pInfo->m_nQuestTemplateID );
			if( pQuestInfo )
			{
				pQuestInfo->bRegisterJournal	=	FALSE;
			}

			if( bSendPacket && pSelfCharacter )
			{
				m_pcsAgcmQuest->SendPacketQuestJournalRegisterChange( pInfo->m_nQuestTemplateID , pSelfCharacter->GetID() , FALSE );
			}

			DEF_SAFEDELETE( pInfo );
			m_mapQuestJournal.erase( Iter );

			EnumCallback( AGCMQUEST_JOURNAL_CB_REFRESH , NULL , NULL );
			return TRUE;
		}
	}

	

	return FALSE;
}

BOOL	AgcmQuestJournal::DelQuestJournal( INT nQuestTemplateID , BOOL bSendPacket )
{
	AgpdCharacter*	pSelfCharacter	=	m_pcsAgcmCharacter->GetSelfCharacter();

	MapQuestJournalIter	Iter	=	m_mapQuestJournal.begin();
	for( ; Iter != m_mapQuestJournal.end() ; ++Iter )
	{
		stJournalInfo*	pInfo	=	Iter->second;
		if( pInfo->m_nQuestTemplateID == nQuestTemplateID )
		{
			AgpdCurrentQuest*	pQuestInfo	=	m_pcsSelfQuest->GetQuest( pInfo->m_nQuestTemplateID );
			if( pQuestInfo )
			{
				pQuestInfo->bRegisterJournal	=	FALSE;
			}

			if( bSendPacket && pSelfCharacter )
			{
				m_pcsAgcmQuest->SendPacketQuestJournalRegisterChange( pInfo->m_nQuestTemplateID , pSelfCharacter->GetID() , FALSE );
			}


			DEF_SAFEDELETE( pInfo );
			m_mapQuestJournal.erase( Iter );
			EnumCallback( AGCMQUEST_JOURNAL_CB_REFRESH , NULL , NULL );
			return TRUE;
		}
	}

	return FALSE;
}

INT		AgcmQuestJournal::GetQuestCount( eJournalType eType /* = QUEST_JOURNAL_NORMAL | QUEST_JOURNAL_MISSION */ )
{
	INT		nCount	=	0;
	
	MapQuestJournalIter	Iter	=	m_mapQuestJournal.begin();
	for( ; Iter != m_mapQuestJournal.end() ; ++Iter )
	{
		stJournalInfo*	pInfo	=	Iter->second;
		if( pInfo->m_eType & eType )
		{
			++nCount;
		}
	}

	return nCount;
}

stJournalInfo*	AgcmQuestJournal::GetQuestJournal( INT nIndex , eJournalType eType /* = QUEST_JOURNAL_NORMAL | QUEST_JOURNAL_MISSION */ )
{

	INT						nCount	=	0;
	MapQuestJournalIter		Iter	=	m_mapQuestJournal.begin();
	for( ; Iter != m_mapQuestJournal.end() ; ++Iter )
	{
		stJournalInfo*	pInfo	=	Iter->second;
		if( pInfo->m_eType & eType )
		{
			if( nCount == nIndex )
				return pInfo;

			++nCount;
		}
	}

	return NULL;
}

stJournalInfo*	AgcmQuestJournal::GetQuestJournal( CONST string& strQuestName )
{

	MapQuestJournalIter	Iter	=	m_mapQuestJournal.begin();
	for( ; Iter != m_mapQuestJournal.end() ; ++Iter )
	{
		stJournalInfo*	pInfo	=	Iter->second;
		if( pInfo->m_strQuestName	== strQuestName )
			return pInfo;
	}

	return NULL;
}

BOOL	AgcmQuestJournal::IsQuestJournal( INT nQuestTemplateID )
{
	MapQuestJournalIter	Iter	=	m_mapQuestJournal.begin();
	for( ; Iter != m_mapQuestJournal.end() ; ++Iter )
	{
		stJournalInfo*	pInfo	=	Iter->second;
		if( pInfo->m_nQuestTemplateID == nQuestTemplateID )
			return TRUE;
	}

	return FALSE;
}

BOOL		AgcmQuestJournal::CBSetSelfCharacter( PVOID pData , PVOID pClass , PVOID pCustData )
{
	AgcmQuestJournal*	pThis	=	static_cast< AgcmQuestJournal* >(pClass);
	if( !pThis )
		return FALSE;

	AgpdCharacter*		pcsCharacter	=	static_cast< AgpdCharacter* >(pData);
	if( !pcsCharacter)
		return FALSE;

	pThis->m_pcsSelfCharacter	=	pcsCharacter;
	pThis->m_pcsSelfQuest		=	pThis->m_pcsAgpmQuest->GetAttachAgpdQuestData( pcsCharacter );

	pThis->_QueueProcess();

	return TRUE;
}

BOOL		AgcmQuestJournal::CBReleaseSelfCharacter( PVOID pData , PVOID pClass , PVOID pCustData )
{
	AgcmQuestJournal*	pThis	=	static_cast< AgcmQuestJournal* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->Clear();

	pThis->m_pcsSelfQuest		=	NULL;
	pThis->m_pcsSelfCharacter	=	NULL;

	return TRUE;
}

BOOL		AgcmQuestJournal::CBInitQuest( PVOID pData , PVOID pClass , PVOID pCustData )
{
	AgcmQuestJournal*	pThis	=	static_cast< AgcmQuestJournal* >(pClass);
	if( !pThis )
		return FALSE;

	INT					nQuestTID	=	(INT)pData;
	BOOL				bRegister	=	(BOOL)pCustData;
	if( bRegister )
	{
		AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(nQuestTID);
		if (!pQuestTemplate) 
			return FALSE;

		pThis->AddQuestJournal( pQuestTemplate->m_QuestInfo.szName , QUEST_JOURNAL_ALL , nQuestTID );
	}
	
	return TRUE;
}

BOOL		AgcmQuestJournal::SetCallbackRefresh( ApModuleDefaultCallBack pfCallback , PVOID pClass )
{
	return SetCallback( AGCMQUEST_JOURNAL_CB_REFRESH , pfCallback , pClass );
}


CONST	CHAR*	AgcmQuestJournal::_GetQuestJournalText( INT nQuestTemplateID )
{
	MapQuestJournalTextIter	Iter	=	m_mapQuestJournalText.find( nQuestTemplateID );
	if( Iter == m_mapQuestJournalText.end() )
		return NULL;

	return Iter->second->strJournalText.c_str();
}

BOOL	AgcmQuestJournal::MakeJournalText( INT nQuestTemplateID , CHAR* pTextBuffer , INT nSize )
{
	AgpdQuestTemplate*	pTemplate	=	m_pcsAgpmQuest->m_csQuestTemplate.Get( nQuestTemplateID );
	if( !pTemplate )
		return FALSE;

	INT		nIndex	=	0;
	DWORD	dwColor	=	m_pcsAgcmQuest->GetJournalQuestNameColor( nQuestTemplateID );

	// 퀘스트 이름
	nIndex	= sprintf_s(pTextBuffer , nSize-nIndex , "<H%x>%s<HFFFFFFFF></n>", dwColor , pTemplate->m_QuestInfo.szName );
	// 퀘스트 조건
	nIndex	+= m_pcsAgcmUIQuest2->ComposeMainSummaryText( &pTextBuffer[nIndex] , nSize-nIndex , pTemplate , m_pcsSelfQuest , TRUE , !pTemplate->m_QuestInfo.bIsCompleteSummary );

	CONST	CHAR*	szMessage	=	_GetQuestJournalText( nQuestTemplateID );
	if( szMessage && strlen(szMessage) )
	{
		// 퀘스트 설명
		nIndex +=	sprintf_s( &pTextBuffer[nIndex] , nSize-nIndex , "%s" , szMessage );
	}

	return TRUE;
}


VOID	AgcmQuestJournal::_QueueProcess( VOID )
{
	while( !m_queueProcessData.empty() )
	{
		stProcessData	stData	=	m_queueProcessData.front();

		AddQuestJournal( stData.strQuestName , stData.eType , stData.nQuestTemplateID );

		m_queueProcessData.pop();
	}

}