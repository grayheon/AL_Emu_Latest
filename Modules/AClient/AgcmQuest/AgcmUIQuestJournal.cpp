#include "AgcmUIQuestJournal.h"
#include "AgcmUIManager2.h"

#include "AgpmQuest.h"
#include "AgcUIQuestText.h"
#include "AgcmUIQuest2.h"

#define		JOURNAL_SIZE_X				200
#define		DEFAULT_JOURNAL_SIZE_Y		1000

#define		MAX_JOURNAL_TEXT			2048

#define		JOURNAL_TIME				10000
#define		JOURNAL_FADE_INTERVAL		1000

#define		JOURNAL_INTERVAL			5

#define		QUEST_MAP_BUTTON_NORMAL			"QuestJ_Btn_Map_a.png"
#define		QUEST_MAP_BUTTON_ONMOUSE		"QuestJ_Btn_Map_b.png"
#define		QUEST_MAP_BUTTON_CLICK			"QuestJ_Btn_Map_c.png"
#define		QUEST_MAP_BUTTON_DISABLE		"QuestJ_Btn_Map_d.png"

#define		QUEST_MAIN_BUTTON_NORMAL		"QuestJ_Btn_Win_a.png"
#define		QUEST_MAIN_BUTTON_ONMOUSE		"QuestJ_Btn_Win_b.png"
#define		QUEST_MAIN_BUTTON_CLICK			"QuestJ_Btn_Win_c.png"
#define		QUEST_MAIN_BUTTON_DISABLE		"QuestJ_Btn_Win_d.png"

#define		QUEST_EXIT_BUTTON_NORMAL		"QuestJ_Btn_X_a.png"
#define		QUEST_EXIT_BUTTON_ONMOUSE		"QuestJ_Btn_X_b.png"
#define		QUEST_EXIT_BUTTON_CLICK			"QuestJ_Btn_X_c.png"
#define		QUEST_EXIT_BUTTON_DISABLE		"QuestJ_Btn_X_d.png"

#define		QUEST_JOURNAL_IMAGE_TOP			"QuestJ_Base_a.png"
#define		QUEST_JOURNAL_IMAGE_CENTER		"QuestJ_Base_b.png"
#define		QUEST_JOURNAL_IMAGE_BOTTOM		"QuestJ_Base_c.png"

//-----------------------------------------------------------------------
//

JournalText::JournalText()
: questText_( new AgcUIQuestText )
, uiBase_( new AcUIBase )
{
	questText_->Init();
}

//-----------------------------------------------------------------------
//

JournalText::~JournalText()
{
	DEF_SAFEDELETE(questText_);
	DEF_SAFEDELETE(uiBase_);
}

//-----------------------------------------------------------------------
//

void JournalText::init( float * alpha /* = 0 */ )
{
	uiBase_->m_Property.bUseInput = FALSE;
	uiBase_->m_lFontType = 1;
	uiBase_->m_pfAlpha = alpha;

	questText_->Init();
}

//-----------------------------------------------------------------------
//

int JournalText::refresh( char const * msg, int questTID )
{
	return questText_->RefreshTextList( msg, uiBase_, questTID );
}

//-----------------------------------------------------------------------
//

void JournalText::alpha( float * alpha )
{
	questText_->SetAlpha( alpha );
}

//-----------------------------------------------------------------------
//

AgcmUIQuestJournal::AgcmUIQuestJournal( VOID )
:m_nUIBuffDefaultX(0)
,m_nUIBuffDefaultY(0)
,m_bIsInitialized(FALSE)
{
	SetModuleName("AgcmUIQuestJournal");
	EnableIdle( TRUE );

	m_pJournalUI	=	NULL;

	for( INT i = 0 ; i < MAX_JOURNAL_VIEW_COUNT ; ++i )
	{
		m_pJournalChildUI[i]	=	NULL;
		m_pJournalButton1[i]	=	NULL;
		m_pJournalButton2[i]	=	NULL;
		m_pJournalButton3[i]	=	NULL;

		m_pJournalText[i]		=	NULL;
	}

}

AgcmUIQuestJournal::~AgcmUIQuestJournal( VOID )
{

}

BOOL	AgcmUIQuestJournal::OnInit( VOID )
{
	m_pcsAgcmUIManager2		=	static_cast< AgcmUIManager2* >( GetModule( "AgcmUIManager2" ) );
	m_pcsAgcmQuestJournal	=	static_cast< AgcmQuestJournal* >( GetModule( "AgcmQuestJournal" ) );
	m_pcsAgpmQuest			=	static_cast< AgpmQuest* >( GetModule( "AgpmQuest" ) );
	m_pcsAgcmUIQuest2		=	static_cast< AgcmUIQuest2* >( GetModule( "AgcmUIQuest2" ) );

	m_pJournalUI	=	m_pcsAgcmUIManager2->CreateUI();
	sprintf_s( m_pJournalUI->m_szUIName , AGCDUIMANAGER2_MAX_NAME , "QuestJournalUI" );
	m_pcsAgcmUIManager2->AddUI( m_pJournalUI );

	m_pJournalUI->m_pcsUIWindow->MoveWindow( m_pcsAgcmUIManager2->m_lWindowWidth - (JOURNAL_SIZE_X+7) , 200  , JOURNAL_SIZE_X , DEFAULT_JOURNAL_SIZE_Y );
	m_pJournalUI->m_pcsUIWindow->m_Property.bMovable	=	FALSE;
	m_pJournalUI->m_pcsUIWindow->m_Property.bUseInput	=	FALSE;
	m_pJournalUI->m_bAutoClose							=	FALSE;

	AcUIJournal::SetReadImage( QUEST_JOURNAL_IMAGE_TOP , E_JOURNAL_IMAGE_TOP );
	AcUIJournal::SetReadImage( QUEST_JOURNAL_IMAGE_CENTER , E_JOURNAL_IMAGE_CENTER );
	AcUIJournal::SetReadImage( QUEST_JOURNAL_IMAGE_BOTTOM , E_JOURNAL_IMAGE_BOTTOM );

	for( INT i = 0 ; i < MAX_JOURNAL_VIEW_COUNT ; ++i )
	{
		m_pJournalChildUI[i]	=	new AcUIJournal;
		m_pJournalTempChildUI[i]=	new AcUIJournal;

		m_pJournalButton1[i]	=	new	AcUIEventButton;
		m_pJournalButton2[i]	=	new AcUIEventButton;
		m_pJournalButton3[i]	=	new AcUIEventButton;

		// 버튼별 이벤트를 붙여준다
		m_pJournalButton1[i]->SetCallbackClickEvent( this , CBClickJournalButton1 );
		m_pJournalButton2[i]->SetCallbackClickEvent( this , CBClickJournalButton2 );
		m_pJournalButton3[i]->SetCallbackClickEvent( this , CBClickJournalButton3 );

		m_pJournalButton1[i]->SetData( (PVOID)i , NULL );
		m_pJournalButton2[i]->SetData( (PVOID)i , NULL );
		m_pJournalButton3[i]->SetData( (PVOID)i , NULL );

		m_pJournalText[i]		=	new JournalText;
		m_pJournalTempText[i]	=	new JournalText;

		m_pJournalChildUI[i]->m_Property.bVisible		=	FALSE;
		m_pJournalTempChildUI[i]->m_Property.bVisible	=	FALSE;

		m_pJournalText[i]->init();
		m_pJournalTempText[i]->init( &m_fJournalTempAlpha[i] );

		m_pJournalChildUI[i]->AddChild( m_pJournalText[i]->uiBase_ );
		m_pJournalChildUI[i]->AddChild( m_pJournalButton1[i] );
		m_pJournalChildUI[i]->AddChild( m_pJournalButton3[i] );

		m_pJournalTempChildUI[i]->AddChild( m_pJournalTempText[i]->uiBase_ );

		m_pJournalChildUI[i]->MoveWindow( 0 , 0 , 200 , 70 );
		m_pJournalTempChildUI[i]->MoveWindow( 0 , 0 , 200 , 70 );
		m_pJournalText[i]->uiBase_->MoveWindow( 0 , 0 , 200 , 70 );
		m_pJournalTempText[i]->uiBase_->MoveWindow( 0 , 0 , 200 , 70 );

		m_pJournalUI->m_pcsUIWindow->AddChild( m_pJournalChildUI[i] );
		m_pJournalUI->m_pcsUIWindow->AddChild( m_pJournalTempChildUI[i] );

		INT		nOffset		=	m_pJournalChildUI[i]->w - 2;
		m_pJournalButton3[i]->AddButtonImage( QUEST_EXIT_BUTTON_NORMAL	, ACUIBUTTON_MODE_NORMAL );
		m_pJournalButton3[i]->AddButtonImage( QUEST_EXIT_BUTTON_ONMOUSE	, ACUIBUTTON_MODE_ONMOUSE );
		m_pJournalButton3[i]->AddButtonImage( QUEST_EXIT_BUTTON_CLICK	, ACUIBUTTON_MODE_CLICK );
		m_pJournalButton3[i]->AddButtonImage( QUEST_EXIT_BUTTON_DISABLE	, ACUIBUTTON_MODE_DISABLE );

		nOffset	= (INT)(nOffset - m_pJournalButton3[i]->m_fTextureWidth);
		m_pJournalButton3[i]->MoveWindow( nOffset  , 1 , (INT32)m_pJournalButton3[i]->m_fTextureWidth , (INT32)m_pJournalButton3[i]->m_fTextureHeight );

		m_pJournalButton1[i]->AddButtonImage( QUEST_MAIN_BUTTON_NORMAL	, ACUIBUTTON_MODE_NORMAL );
		m_pJournalButton1[i]->AddButtonImage( QUEST_MAIN_BUTTON_ONMOUSE	, ACUIBUTTON_MODE_ONMOUSE );
		m_pJournalButton1[i]->AddButtonImage( QUEST_MAIN_BUTTON_CLICK	, ACUIBUTTON_MODE_CLICK );
		m_pJournalButton1[i]->AddButtonImage( QUEST_MAIN_BUTTON_DISABLE	, ACUIBUTTON_MODE_DISABLE );

		nOffset -=	2;
		nOffset = (INT)(nOffset - m_pJournalButton1[i]->m_fTextureWidth );
		m_pJournalButton1[i]->MoveWindow( nOffset , 1 , (INT32)m_pJournalButton1[i]->m_fTextureWidth , (INT32)m_pJournalButton1[i]->m_fTextureHeight );

		m_pJournalTempChildUI[i]->m_pfAlpha	=	&m_fJournalTempAlpha[i];
	}

	m_pcsAgcmQuestJournal->SetCallbackRefresh( CBRefreshJournalInfo , this );
	m_pcsAgpmQuest->SetCallbackUpdateQuest(CBRefreshJournalInfo, this);

	return TRUE;
}

VOID	AgcmUIQuestJournal::OpenJournalUI( VOID )
{
	m_pcsAgcmUIManager2->OpenUI( m_pJournalUI );
}

VOID	AgcmUIQuestJournal::RefreshJournalInfo( VOID )
{
	_ClearTempText();

	MapJournalInfo::iterator	Iter	=	m_mapTempJournalInfo.begin();
	for( INT i = 0 ; Iter != m_mapTempJournalInfo.end() && i < MAX_JOURNAL_VIEW_COUNT ; ++Iter , ++i )
	{
		stJournalInfo*	pInfo	=	Iter->second;
		_RefreshTempText( pInfo , i );
	}

	BOOL	bIsExistFloatingJournal = FALSE;

	for( INT i = 0 ; i < MAX_JOURNAL_VIEW_COUNT ; ++i )
	{
		stJournalInfo*	pInfo	=	m_pcsAgcmQuestJournal->GetQuestJournal( i );
		bIsExistFloatingJournal |= _RefreshText( pInfo , i );
	}

	AgcdUI* pAlarmUI = m_pcsAgcmUIManager2->GetUI("UI_Alarm_Grid");
	if(pAlarmUI)
	{
		if(!m_bIsInitialized)
		{
			m_nUIBuffDefaultX = pAlarmUI->m_pcsUIWindow->x;
			m_nUIBuffDefaultY = pAlarmUI->m_pcsUIWindow->y;
			m_bIsInitialized = TRUE;
		}
		if(bIsExistFloatingJournal)
		{
			pAlarmUI->m_pcsUIWindow->MoveWindow(m_nUIBuffDefaultX - JOURNAL_SIZE_X - 7, m_nUIBuffDefaultY);
		}
		else
		{
			pAlarmUI->m_pcsUIWindow->MoveWindow(m_nUIBuffDefaultX, m_nUIBuffDefaultY);
		}
	}
}

VOID	AgcmUIQuestJournal::RefreshJournalPosition( VOID )
{
	INT		nOffsetY	=	4;

	MapJournalInfo::iterator	Iter	=	m_mapTempJournalInfo.begin();
	for( INT i = 0 ; Iter != m_mapTempJournalInfo.end() && i < MAX_JOURNAL_VIEW_COUNT ; ++Iter , ++i )
	{
		INT		nWidth	=	m_pJournalTempChildUI[i]->w;
		INT		nHeight	=	m_pJournalTempText[i]->uiBase_->h+4;

		m_pJournalTempChildUI[i]->MoveWindow( 4 , nOffsetY , nWidth , nHeight );

		nOffsetY	+=	nHeight;
		nOffsetY	+=	JOURNAL_INTERVAL;
	}

	for( INT i = 0 ; i < MAX_JOURNAL_VIEW_COUNT ; ++i )
	{
		INT		nWidth	=	m_pJournalChildUI[i]->w;
		INT		nHeight	=	m_pJournalText[i]->uiBase_->h+4;

		m_pJournalChildUI[i]->MoveWindow( 4 , nOffsetY , nWidth , nHeight );

		nOffsetY	+=	nHeight;
		nOffsetY	+=	JOURNAL_INTERVAL;
	}

}

VOID	AgcmUIQuestJournal::RefreshTempJournalAlpha( VOID )
{
	MapJournalInfo::iterator	Iter	=	m_mapTempJournalInfo.begin();
	for( INT i = 0 ; Iter != m_mapTempJournalInfo.end() && i < MAX_JOURNAL_VIEW_COUNT ; ++Iter , ++i )
	{
		m_fJournalTempAlpha[i]	=	Iter->second->m_fAlpha;
	}
}

BOOL	AgcmUIQuestJournal::OnAddModule( VOID )
{
	return TRUE;
}

BOOL	AgcmUIQuestJournal::OnIdle( UINT32 ulClockCount )
{
	MapJournalInfo::iterator	Iter	=	m_mapTempJournalInfo.begin();
	BOOL						bErase	=	FALSE;

	for( INT i = 0 ; Iter != m_mapTempJournalInfo.end() ; ++i )
	{
		// 지워준다
		if( (Iter->first + JOURNAL_TIME) < ulClockCount )
		{
			m_mapTempJournalInfo.erase( Iter++ );
			bErase	=	TRUE;
			continue;
		}

		else
		{
			DWORD	dwRest			=	(Iter->first + JOURNAL_TIME) - ulClockCount;
			DWORD	dwIntervalRest	=	(dwRest % JOURNAL_FADE_INTERVAL);
			DWORD	dwHalfInterval	=	JOURNAL_FADE_INTERVAL / 2;

			// 점점 안보인다
			if( dwIntervalRest > dwHalfInterval )
			{
				Iter->second->m_fAlpha	=	1.0f - ( (FLOAT)dwIntervalRest / (FLOAT)JOURNAL_FADE_INTERVAL );
			}

			// 점점 보인다
			else
			{
				Iter->second->m_fAlpha	=	(FLOAT)dwIntervalRest / (FLOAT)dwHalfInterval;
			}
		}

		++Iter;
	}

	// 하나라도 지워진게 있으면 다시 업데이트 해준다
	if( bErase )
	{
		RefreshJournalInfo();
		RefreshJournalPosition();
	}

	// 임시 저널의 알파를 업데이트 해준다
	RefreshTempJournalAlpha();

	return TRUE;
}

BOOL	AgcmUIQuestJournal::OnDestroy( VOID )
{
	return TRUE;
}

BOOL	AgcmUIQuestJournal::_RefreshText( stJournalInfo* pJournalInfo , INT nIndex )
{
	if( !pJournalInfo )
	{
		m_pJournalChildUI[nIndex]->m_Property.bVisible	=	FALSE;
		return FALSE;
	}

	m_pJournalChildUI[nIndex]->m_Property.bVisible	=	TRUE;

	CHAR	szJournalBuffer[ MAX_JOURNAL_TEXT ];
	ZeroMemory( szJournalBuffer , MAX_JOURNAL_TEXT );

	m_pcsAgcmQuestJournal->MakeJournalText( pJournalInfo->m_nQuestTemplateID , szJournalBuffer , MAX_JOURNAL_TEXT );

	JournalText * journalText = m_pJournalText[nIndex]; 

	int height = journalText->refresh( szJournalBuffer, pJournalInfo->m_nQuestTemplateID );

	journalText->uiBase_->MoveWindow( 2 , 2 , m_pJournalChildUI[nIndex]->w - 4 , height );

	return TRUE;
}

VOID	AgcmUIQuestJournal::_ClearTempText( VOID )
{
	for( INT i = 0 ; i < MAX_JOURNAL_VIEW_COUNT ; ++i )
	{
		m_pJournalTempChildUI[i]->m_Property.bVisible	=	FALSE;
		m_pJournalTempText[i]->refresh( "" );
	}
}

BOOL	AgcmUIQuestJournal::_RefreshTempText( stJournalInfo* pJournalInfo , INT nIndex )
{
	if( !pJournalInfo )
		return FALSE;

	m_pJournalTempChildUI[nIndex]->m_Property.bVisible = TRUE;

	CHAR	szJournalBuffer[ MAX_JOURNAL_TEXT ];
	ZeroMemory( szJournalBuffer , MAX_JOURNAL_TEXT );

	m_pcsAgcmQuestJournal->MakeJournalText( pJournalInfo->m_nQuestTemplateID , szJournalBuffer , MAX_JOURNAL_TEXT );

	JournalText * journalText = m_pJournalTempText[nIndex];

	int height = journalText->refresh( szJournalBuffer );

	journalText->alpha( &m_fJournalTempAlpha[nIndex] );

	journalText->uiBase_->MoveWindow( 2 , 2 , m_pJournalTempChildUI[nIndex]->w - 4 , height );

	return TRUE;
}


// 통합 커뮤니티 창을 열어준다
BOOL	AgcmUIQuestJournal::CBClickJournalButton1( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcmUIQuestJournal*	pThis	=	static_cast< AgcmUIQuestJournal* >(pClass);
	if( !pThis )
		return FALSE;

	return pThis->m_pcsAgcmUIQuest2->OpenMainQuest();
}

// 전체지도를 열어준다( 위치표시 )
BOOL	AgcmUIQuestJournal::CBClickJournalButton2( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcmUIQuestJournal*	pThis	=	static_cast< AgcmUIQuestJournal* >(pClass);
	if( !pThis )
		return FALSE;

	INT					nIndex	=	(INT)pData1;
	stJournalInfo*	pInfo	=	pThis->m_pcsAgcmQuestJournal->GetQuestJournal( nIndex );
	if( pInfo )
	{
		pThis->m_pcsAgcmUIQuest2->OpenWorldMap();
	}

	return TRUE;
}

// 저널을 닫는다
BOOL	AgcmUIQuestJournal::CBClickJournalButton3( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcmUIQuestJournal* pThis	=	static_cast< AgcmUIQuestJournal* >(pClass);
	if( !pThis )
		return FALSE;

	INT					nIndex	=	(INT)pData1;
	stJournalInfo*	pInfo	=	pThis->m_pcsAgcmQuestJournal->GetQuestJournal( nIndex );
	if( pInfo )
	{
		pThis->m_pcsAgcmQuestJournal->DelQuestJournal( pInfo->m_nQuestTemplateID , TRUE );
		pThis->m_pcsAgcmUIQuest2->RefreshMainTree();
	}

	return TRUE;
}

BOOL	AgcmUIQuestJournal::CBRefreshJournalInfo( PVOID pData , PVOID pClass , PVOID pCustData )
{
	AgcmUIQuestJournal*	pThis	=	static_cast< AgcmUIQuestJournal* >(pClass);
	if( !pThis )
		return FALSE;

	BOOL				bFailed	=	(BOOL)pCustData;

	// 이놈이 있으면 꽉 차서 등록이 안되는 놈인데..
	// 그냥 맨 위에 깜박깜박 해서 알려주기만 한다.
	if( bFailed )
	{
		stJournalInfo*		pInfo	=	static_cast< stJournalInfo* >(pData);
		if( pInfo )
		{
			MapJournalInfo::iterator	Iter	=	pThis->m_mapTempJournalInfo.begin();
			for( ; Iter != pThis->m_mapTempJournalInfo.end() ; ++Iter )
			{
				if( Iter->second->m_nQuestTemplateID == pInfo->m_nQuestTemplateID )
				{
					return TRUE;
				}
			}
			pThis->m_pcsAgcmUIQuest2->Notice( pThis->m_pcsAgcmUIManager2->GetUIMessage( "JournalMaxCount" ) );
			pThis->m_mapTempJournalInfo.insert( make_pair( pThis->GetClockCount() , pInfo) );
			pInfo->m_fAlpha	=	1.0f;
		}
	}

	pThis->RefreshJournalInfo();
	pThis->RefreshJournalPosition();

	return TRUE;
}

BOOL	AgcmUIQuestJournal::CBJournalPositionLinkEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcmUIQuestJournal*	pThis	=	static_cast< AgcmUIQuestJournal* >(pClass);
	if( !pThis  )
		return FALSE;

	AcUIEventButton*	pEventButton	=	static_cast< AcUIEventButton* >(pButton);
	if( !pEventButton )
		return FALSE;

	string		strName;

	if( pEventButton->m_szStaticString )
	{
		strName	=	pEventButton->m_szStaticString;
	}

	INT				nPosX			=	(INT)pData1;
	INT				nPosY			=	(INT)pData2;
	INT				nRegionIndex	=	(INT)pEventButton->GetTempData();
	INT				nQuestTID		=	(INT)pEventButton->GetTempData2();

	INT				nID				=	nPosX + nPosY;
	RwV2d			vPos;

	nID		+=	strName.length();
	nID		=	abs( nID );

	vPos.x		=	(RwReal)nPosX;
	vPos.y		=	(RwReal)nPosY;

	AgcmMinimap*	pcsAgcmMinimap	=	static_cast< AgcmMinimap* >( pThis->GetModule( "AgcmMinimap" ) );
	if( pcsAgcmMinimap )
	{
		//AgcmUIQuest2 *pQuest = (AgcmUIQuest2*)pThis->GetModule("AgcmUIQuest2");
		//if( pQuest )
		//	pQuest->AddQuestPositionInfo( nQuestTID, nID );

		pcsAgcmMinimap->DestroyMapPosition( nID );
		pcsAgcmMinimap->AddMapPosition( strName , nID , &vPos , nRegionIndex );
		pThis->m_pcsAgcmUIQuest2->OpenWorldMap( nRegionIndex );
	}



	return TRUE;
}