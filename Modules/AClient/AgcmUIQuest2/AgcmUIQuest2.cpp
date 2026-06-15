// AgcmUIQuest2.cpp: implementation of the AgcmUIQuest2 class.
//
//////////////////////////////////////////////////////////////////////

#include "AgcmUIQuest2.h"
#include "AuStrTable.h"
#include "AgcmTextBoardMng.h"
#include "AgcChatManager.h"
#include "AgcUIQuestText.h"
#include "AgcmMinimap.h"
#include "AgcmQuestJournal.h"
#include "AgcmUIQuestJournal.h"
#include "AcUIWorldmap.h"
#include "AgcmUINotice.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Events
#define AGCMUI_EVENT_QUEST_ACCEPT_OPEN2		"QuestAccept_Open2"
#define AGCMUI_EVENT_QUEST_MAIN_OPEN2		"QuestMain_Open2"
#define AGCMUI_EVENT_QUEST_ACCEPT_CLOSE		"QuestAccept_Close2"
#define AGCMUI_EVENT_QUEST_CANCEL_DLG_OPEN	"QuestCancelDlg_Open"
#define AGCMUI_EVENT_QUEST_DONT_HAVE		"QuestDontHave_Message"
#define AGCMUI_EVENT_QUEST_START			"QuestStart_Message"
#define AGCMUI_EVENT_QUEST_MAIN_INVENTORY_FULL	"QuestInventoryMainFull_Message"	// main inventory
#define AGCMUI_EVEMT_QUEST_QUEST_INVENTORY_FULL	"QuestInventoryQuestFull_Message"	// quest inventory
#define AGCMUI_EVENT_QUEST_COMPLETE			"QuestComplete_Message"
#define AGCMUI_EVENT_QUEST_FULL				"QuestFull_Message"
#define AGCMUI_SOUND_QUEST_START			"QuestStart_Sound"
#define AGCMUI_SOUND_QUEST_FAIL				"QuestFail_Sound"
#define AGCMUI_EVENT_QUEST_COUNT			"QuestCount"
#define AGCMUI_EVENT_QUEST_AUTOSTART		"Quest_AutoStart"
#define AGCMUI_EVENT_QUEST_AUTOEND			"Quest_AutoEnd"

// Display
#define AGCMUI_DISPLAY_QUEST_ACCEPT			"QuestAccept_Display2"
#define AGCMUI_DISPLAY_QUEST_ACCEPT_TEXT	"QuestAcceptText_Display2"
#define AGCMUI_DISPLAY_QUEST_ACCEPT_LEFT_BUTTON "QusetAcceptLeftButton_Display2"
#define AGCMUI_DISPLAY_QUEST_MAIN_TEXT		"QuestMainText_Display2"
#define AGCMUI_DISPLAY_QUEST_MAIN_CANCEL	"QuestMainCancel_Display2"	// Message
#define AGCMUI_DISPLAY_QUEST_MAIN_TREE		"QuestMainTree_Display2"
#define	AGCMUI_DISPLAY_QUEST_MAIN_CHECK		"QuestMainTreeCheck_Display2"
#define AGCMUI_DISPLAY_QUEST_START			"QuestStart_Display2"		// Message
#define AGCMUI_DISPLAY_QUEST_COMPLETE		"QuestComplete_Display2"
#define AGCMUI_DISPLAY_QUEST_COUNT			"QuestCount_Display2"

// Userdata
#define AGCMUI_USERDATA_ACCEPT_LIST			"QuestAcceptList_UserData2"
#define AGCMUI_USERDATA_ACCEPT_TEXT			"QuestAcceptText_UserData2"
#define AGCMUI_USERDATA_ACCEPT_LEFT_BUTTON	"QuestAcceptLeftButton_UserData2"
#define AGCMUI_USERDATA_ACCEPT_LEFT_BUTTON_ENABLE "QuestAcceptLeftButtonEnable_UserData2"
#define AGCMUI_USERDATA_MAIN_TEXT			"QuestMainText_UserData2"
#define AGCMUI_USERDATA_MAIN_QUSET_CANCEL	"QuestMainCancel_UserData2"
#define AGCMUI_USERDATA_MAIN_TREE			"QuestMainTree_UserData2"
#define AGCMUI_USERDATA_MAIN_TREE_DEPTH		"QuestMainTreeDepth_UserData2"
#define AGCMUI_USERDATA_QUEST_START			"QuestStart_UserData2"
#define AGCMUI_USERDATA_QUEST_COMPLETE		"QuestComplete_UserData2"
#define AGCMUI_USERDATA_QUEST_COUNT			"QuestCount_UserData2"

// Functions
#define AGCMUI_FUNCTION_ACCEPT_CLICK		"QuestAcceptList_Click2"
#define AGCMUI_FUNCTION_ACCEPT_LEFT_BUTTON_CLICK	"QuestAcceptLeftButton_CLick2"
#define AGCMUI_FUNCTION_BUTTON_CANCEL		"QuestCancel_Button2"
#define AGCMUI_FUNCTION_MAIN_CLICK			"QuestMainLit_Cick2"
#define AGCMUI_FUNCTION_MAIN_HOT_KEY_OPEN	"QuestMainHoeKey_Open2"

#define AGCMUI_FUNCTION_CHECK_JOURNAL		"QuestMain_CheckJournal"

#define AGCMUI_FUNCTION_JOURNAL_CLOSE		"QuestMain_Journal_Close"
#define AGCMUI_FUNCTION_QUEST_SHARE			"QuestMain_Share"
#define AGCMUI_FUNCTION_QUEST_DEL			"QuestMain_Del"
#define AGCMUI_FUNCTION_MAP_OPEN			"QuestMain_MapOpen"

#define AGCMUI_AUTOSTART_ICON				"auto_quest_icon"

#define	AGCMUI_MAIN_QUEST_UI_NAME				"QuestMain_UI"
#define	AGCMUI_MAIN_TREE_CONTROL_NAME			"CONTROL_QM_TREE"
#define	AGCMUI_MAIN_EDIT_CONTROL_NAME			"CONTROL_QM_TEXT"
#define	AGCMUI_MAIN_TREE_SCROLL_NAME			"CONTROL_QM_TREE_SCROLL"
#define	AGCMUI_MAIN_EDIT_SCROLL_NAME			"CONTROL_QM_TEXT_SCROLL"

#define AGCMUI_ACCEPT_QUEST_UI_NAME				"QuestAccept_UI"
#define AGCMUI_ACCEPT_LIST_CONTROL_NAME			"CONTROL_QA_TREE"
#define AGCMUI_ACCEPT_LIST_SCROLL_NAME			"CONTROL_QA_TREE_SCROLL"
#define AGCMUI_ACCEPT_TEXT_SCROLL_NAME			"CONTROL_QA_CONDITION_SCROLL"

// const value
const INT32 TEXT_BUFFER_SIZE = 2000;
const INT32 AGCMUI_QUEST_NOTIFICATION_DELAY	= 5000;

// auto quest alert duration
const INT32 AGCMUIQUEST_AUTOQUEST_ALERT_DURATION	= 10000;
//const INT32 INVALID_INDEX = -1;

// font color
const UINT32 COLOR_QUEST_NEW		= 0xFFF0FF38;
const UINT32 COLOR_QUEST_INCOMPLETE = 0xFFFFFFFF;//
const UINT32 COLOR_QUEST_COMPLETE	= 0xFF9C9C9C;//
const UINT32 COLOR_QUEST_SCENARIO	= 0xFFDBB625;//
const UINT32 COLOR_QUEST_UNKNOWN	= 0xFFDDDDDD;
const UINT32 COLOR_QUEST_DEFAULT	= 0xFFFFFFFF;//
const UINT32 COLOR_QUEST_AREA		= 0xFF62AAB5;//
const UINT32 COLOR_QUEST_CONDITION	= 0xFF8DCAE0;

AgcmUIQuest2::AgcmUIQuest2()
: m_pcsAgcmMinimap(NULL)
, mainQuestText_(NULL)
, acceptQuestText_(NULL)
, m_pcsAgcmUINotice(NULL)
{
	SetModuleName("AgcmUIQuest2");
	EnableIdle(TRUE);

	m_pcsAgpmQuest					= NULL;
	m_pcsAgcmQuest					= NULL;
	m_pcsApmEventManager			= NULL;
	m_pcsAgcmUIManager2				= NULL;
	m_pcsAgcmUIControl				= NULL;
	m_pcsAgcmEventQuest				= NULL;
	m_pcsAgcmCharacter				= NULL;
	m_pcsAgcmEventEffect			= NULL;
	m_pcsAgcmTextBoard				= NULL;
	m_pcsAgpmCharacter				= NULL;
	m_pcsAgpmFactors				= NULL;
	m_pcsAgpmGrid					= NULL;
	m_pcsAgcmUICharacter			= NULL;
	m_pcsAgcmUICooldown				= NULL;
	m_pcsAgcmSkill					= NULL;
	m_pcsAgpmSkill					= NULL;
	m_pcsAgcmUIMain					= NULL;


	// Refresh Controls
	m_pstUDAcceptList				= NULL;
	m_pstUDAcceptText				= NULL;
	m_pstUDAcceptLeftButton			= NULL;
	m_pstUDAcceptLeftButtonEnable	= NULL;
	m_pstUDMainText					= NULL;
	m_pstUDMainQuestCancel			= NULL;
	m_pstUDMainTree					= NULL;
	m_pstUDMainTreeDepth			= NULL;
	m_pstUDQuestStart				= NULL;
	m_pstUDQuestComplete			= NULL;

	m_lEventAcceptUIClose			= 0;
	m_lEventAcceptUIOpen			= 0;
	m_lEventMainUIOpen				= 0;
	m_lEventQuestCancel				= 0;
	m_lMessageBoxQuestTID			= 0;
	m_lMessageQuestDontHave			= 0;
	m_lMessageQuestStart			= 0;
	m_lMessageMainInventoryFull		= 0;
	m_lMessageQuestInventoryFull	= 0;
	m_lMessageQuestComplete			= 0;
	m_lMessageQuestFull				= 0;
	m_lClickIDAcceptUI				= 0;
	m_lClickIDMainUI				= INVALID_INDEX;
	
	m_bIsQuestUIOpen				= FALSE;
	m_lMainQuestTreeCount			= 0;
	m_bAcceptLeftButtonEnable		= FALSE;
	m_ulNotificationCloseTime		= 0;
	m_lAcceptListCount				= 0;
	m_lLastCompleteTID				= INVALID_INDEX;
	m_bHold							= FALSE;

	m_lCooldownID					= AGCMUICOOLDOWN_INVALID_ID;

	ZeroMemory(&m_stOpenPos, sizeof(m_stOpenPos));

	m_lNumAutoQuest					= -1;
	m_ulAlertEndTime				= -1;

	m_pcsAutoStartIcon				= NULL;
	m_bOpenAutoStart				= FALSE;

//	SetTestData();
}

AgcmUIQuest2::~AgcmUIQuest2()
{

}

//void AgcmUIQuest2::SetTestData()
//{
//	// set Accept Data 
//	m_AcceptList[0].eStatus = ACCEPTUI_STATUS_NEW;
//	m_AcceptList[0].lQuestTID = 1;
//	m_AcceptList[1].eStatus = ACCEPTUI_STATUS_INCOMPLETE;
//	m_AcceptList[1].lQuestTID = 2;
//	m_AcceptList[2].eStatus = ACCEPTUI_STATUS_COMPLETE;
//	m_AcceptList[2].lQuestTID = 3;
//
//	m_lAcceptListCount = 3;
//	
//	// set Current Quet Data
//	m_MyQuest.AddNewQuest(1);
//	m_MyQuest.AddNewQuest(4);
//}

BOOL AgcmUIQuest2::OnAddModule()
{
	m_pcsAgpmQuest			= (AgpmQuest*)GetModule("AgpmQuest");
	m_pcsAgcmQuest			= (AgcmQuest*)GetModule("AgcmQuest");
	m_pcsAgcmCharacter		= (AgcmCharacter*)GetModule("AgcmCharacter");
	m_pcsAgcmEventQuest		= (AgcmEventQuest*)GetModule("AgcmEventQuest");
	m_pcsApmEventManager	= (ApmEventManager*)GetModule("ApmEventManager");
	m_pcsAgcmUIManager2		= (AgcmUIManager2*)GetModule("AgcmUIManager2");
	m_pcsAgcmUIControl		= (AgcmUIControl*)GetModule("AgcmUIControl");
	m_pcsAgcmEventEffect	= (AgcmEventEffect*)GetModule("AgcmEventEffect");
	m_pcsAgcmTextBoard		= (AgcmTextBoardMng*)GetModule("AgcmTextBoardMng");
	m_pcsAgpmCharacter		= (AgpmCharacter*)GetModule("AgpmCharacter");
	m_pcsAgpmFactors		= (AgpmFactors*)GetModule("AgpmFactors");
	m_pcsAgpmGrid			= (AgpmGrid*)GetModule("AgpmGrid");
	m_pcsAgpmItem			= (AgpmItem*)GetModule("AgpmItem");
	m_pcsApmMap				= (ApmMap*)GetModule("ApmMap");
	m_pcsAgcmUICharacter	= (AgcmUICharacter*)GetModule("AgcmUICharacter");
	m_pcsAgcmObject			= (AgcmObject*)GetModule("AgcmObject");
	m_pcsApmObject			= (ApmObject*)GetModule("ApmObject");
	m_pcsAgcmUICooldown		= (AgcmUICooldown*)GetModule("AgcmUICooldown");
	m_pcsAgpmSkill			= (AgpmSkill*)GetModule("AgpmSkill");
	m_pcsAgcmSkill			= (AgcmSkill*)GetModule("AgcmSkill");
	m_pcsAgcmUIMain			= (AgcmUIMain*)GetModule("AgcmUIMain");

	if (m_pcsAgcmUICooldown)
	{
		m_lCooldownID = m_pcsAgcmUICooldown->RegisterCooldown(CBAfterCooldown, this);
		if (AGCMUICOOLDOWN_INVALID_ID >= m_lCooldownID)
			return FALSE;
	}

	if (!m_pcsAgpmQuest || !m_pcsAgcmQuest || !m_pcsAgcmCharacter || !m_pcsAgcmEventQuest || !m_pcsApmEventManager ||
		!m_pcsAgcmEventEffect || !m_pcsAgcmTextBoard || !m_pcsAgpmCharacter || !m_pcsAgpmFactors ||
		!m_pcsAgpmItem || !m_pcsApmMap || !m_pcsAgcmSkill)
		return FALSE;

	if (!m_pcsAgcmEventQuest->SetCallbackGrant(CBQuestGrant, this))
		return FALSE;
	
	if (!m_pcsApmEventManager->SetCallbackAddEvent(CBQuestAddEvent, this))
		return FALSE;

	if (m_pcsAgcmObject && !m_pcsAgcmObject->SetCallbackInitObject(CBInitBase, this))
		return FALSE;

	if (!m_pcsAgcmCharacter->SetCallbackInitCharacter(CBInitBase, this))
		return FALSE;

	if (!m_pcsAgcmCharacter->SetCallbackSelfUpdatePosition(CBSelfUpdatePosition, this))
		return FALSE;

	if (!m_pcsAgpmCharacter->SetCallbackUpdateLevel(CBRefreshAll, this))
		return FALSE;

	if (!m_pcsAgcmCharacter->SetCallbackSetSelfCharacter(CBRefreshAll, this))
		return FALSE;

	if (!m_pcsAgpmQuest->SetCallbackRequireQuestResult(CBRequireQuestResult, this))
		return FALSE;

	if (!m_pcsAgpmQuest->SetCallbackCompleteQuestResult(CBCompleteQuestResult, this))
		return FALSE;

	if (!m_pcsAgpmQuest->SetCallbackCancelQuestResult(CBCancelQuestResult, this))
		return FALSE;

	if (!m_pcsAgpmQuest->SetCallbackInventoryFull(CBInventoryFull, this))
		return FALSE;

	if (!m_pcsAgpmQuest->SetCallbackQuestInventoryFull(CBQuestInventoryFull, this))
		return FALSE;

	if (!m_pcsAgpmQuest->SetCallbackUpdateQuest(CBUpdateQuest, this))
		return FALSE;

	if (!m_pcsAgpmItem->SetCallbackInventory(CBAddItemInventory, this))
		return FALSE;

	if (!m_pcsAgpmItem->SetCallbackRemoveFromInventory(CBRemoveItemInventory, this))
		return FALSE;

	if (!m_pcsAgpmItem->SetCallbackUpdateStackCount(CBQuestItemNotice, this))
		return FALSE;

	if (!AddEvent())
		return FALSE;

	if (!AddFunction())
		return FALSE;

	if (!AddUserData())
		return FALSE;
	
	if (!AddDisplay())
		return FALSE;
	
	if (m_pcsAgpmGrid)
		m_pcsAutoStartIcon = m_pcsAgpmGrid->CreateGridItem();

	return TRUE;
}

BOOL AgcmUIQuest2::OnInit()
{
	if (!m_pcsAgcmUIManager2->AddWindow(&m_csNotification))
	{
//		::MessageBox(NULL, "AgcmUIQuest::OnInit() - 쀍!", "햏햏", MB_OK);
		// UI가 필요없는 툴에서 쓰일 경우 문제가 된다. (BOB, 181004)
//		return FALSE;
	}

	m_csNotification.ShowWindow(FALSE);

// 	m_pcsAgcmItem			=	static_cast< AgcmItem* >( GetModule("AgcmItem") );
	m_pcsAgcmQuestJournal	= (AgcmQuestJournal*)GetModule( "AgcmQuestJournal" );
	m_pcsAgcmUIQuestJournal	= (AgcmUIQuestJournal*)GetModule( "AgcmUIQuestJournal" );

	m_pcsAgcmMinimap		= static_cast< AgcmMinimap* >( GetModule( "AgcmMinimap" ) );

	m_pcsAgcmUINotice		= static_cast<AgcmUINotice*>(GetModule("AgcmUINotice"));

	mainQuestText_			= new AgcUIQuestText;
	acceptQuestText_		= new AgcUIQuestText;

	mainQuestText_->Init();
	acceptQuestText_->Init();

	return TRUE;
}

BOOL AgcmUIQuest2::OnDestroy()
{
	DEF_SAFEDELETE(mainQuestText_);
	DEF_SAFEDELETE(acceptQuestText_);

	return TRUE;
}

BOOL AgcmUIQuest2::OnIdle(UINT32 ulClockCount)
{
	PROFILE("AgcmUIQuest2::OnIdle");

	if (m_ulNotificationCloseTime <= ulClockCount)
	{
		m_csNotification.ShowWindow(FALSE);
	}

	if (m_ulAlertEndTime != -1 && m_ulAlertEndTime <= ulClockCount)
	{
		m_pcsAgcmUIManager2->ThrowEvent(m_lEventQuestAutoEnd);
		m_ulAlertEndTime = -1;
	}

	return TRUE;
}

BOOL AgcmUIQuest2::AddEvent()
{
	AGCMUI_CHECK_RETURN(m_lEventAcceptUIOpen, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_ACCEPT_OPEN2));
	AGCMUI_CHECK_RETURN(m_lEventMainUIOpen, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_MAIN_OPEN2));
	AGCMUI_CHECK_RETURN(m_lEventAcceptUIClose, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_ACCEPT_CLOSE));
	AGCMUI_CHECK_RETURN(m_lMessageQuestDontHave, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_DONT_HAVE));
	AGCMUI_CHECK_RETURN(m_lMessageQuestStart, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_START));
	AGCMUI_CHECK_RETURN(m_lMessageMainInventoryFull, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_MAIN_INVENTORY_FULL));
	AGCMUI_CHECK_RETURN(m_lMessageQuestInventoryFull, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVEMT_QUEST_QUEST_INVENTORY_FULL));
	AGCMUI_CHECK_RETURN(m_lMessageQuestComplete, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_COMPLETE));
	AGCMUI_CHECK_RETURN(m_lMessageQuestFull, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_FULL));
	AGCMUI_CHECK_RETURN(m_lSoundQuestStart, m_pcsAgcmUIManager2->AddEvent(AGCMUI_SOUND_QUEST_START));
	AGCMUI_CHECK_RETURN(m_lSoundQuestFail, m_pcsAgcmUIManager2->AddEvent(AGCMUI_SOUND_QUEST_FAIL));
	AGCMUI_CHECK_RETURN(m_lEventQuestCount, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_COUNT));
	AGCMUI_CHECK_RETURN(m_lEventQuestAutoStart, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_AUTOSTART));
	AGCMUI_CHECK_RETURN(m_lEventQuestAutoEnd, m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_AUTOEND));

	return TRUE;
}

BOOL AgcmUIQuest2::AddFunction()
{
	if (!m_pcsAgcmUIManager2->AddFunction(this, AGCMUI_FUNCTION_ACCEPT_CLICK, CBButtonQuestAcceptListClick, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, AGCMUI_FUNCTION_BUTTON_CANCEL, CBCancelDialogOpen, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, AGCMUI_FUNCTION_ACCEPT_LEFT_BUTTON_CLICK, CBButtonAcceptLeftClick, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, AGCMUI_FUNCTION_MAIN_CLICK, CBButtonQuestMainListClick, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, AGCMUI_FUNCTION_MAIN_HOT_KEY_OPEN, CBMainUIHotKeyOpen, 0))
		return FALSE;

	if(!m_pcsAgcmUIManager2->AddFunction( this , AGCMUI_FUNCTION_CHECK_JOURNAL , CBCheckJournal , 0 ))
		return FALSE;

	if(!m_pcsAgcmUIManager2->AddFunction( this , AGCMUI_FUNCTION_JOURNAL_CLOSE	, CBClickJournalClose , 0 ))
		return FALSE;

// 	if(!m_pcsAgcmUIManager2->AddFunction( this , AGCMUI_FUNCTION_QUEST_SHARE	, CBClickQuestShare , 0 ))
// 		return FALSE;

	if(!m_pcsAgcmUIManager2->AddFunction( this , AGCMUI_FUNCTION_QUEST_DEL		, CBClickQuestDel , 0 ))
		return FALSE;

	if(!m_pcsAgcmUIManager2->AddFunction( this , AGCMUI_FUNCTION_MAP_OPEN		, CBClickMapOpen , 0 ))
		return FALSE;

	return TRUE;
}

BOOL AgcmUIQuest2::AddUserData()
{
	m_pstUDAcceptList = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_ACCEPT_LIST, &m_AcceptList[0], sizeof(AgcdUIQuestList), ACCEPTLIST_MAX_COUNT, AGCDUI_USERDATA_TYPE_STRING);
	if (!m_pstUDAcceptList)
		return FALSE;

	m_pstUDAcceptText = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_ACCEPT_TEXT, NULL, 0, 0, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDAcceptText)
		return FALSE;

	m_pstUDAcceptLeftButton = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_ACCEPT_LEFT_BUTTON, NULL, 0, 0, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDAcceptLeftButton)
		return FALSE;

	m_pstUDAcceptLeftButtonEnable = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_ACCEPT_LEFT_BUTTON_ENABLE, &m_bAcceptLeftButtonEnable, sizeof(m_bAcceptLeftButtonEnable), 1, AGCDUI_USERDATA_TYPE_BOOL);
	if (!m_pstUDAcceptLeftButtonEnable)
		return FALSE;

	m_pstUDMainText = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_MAIN_TEXT, NULL, 0, 0, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDMainText)
		return FALSE;

	m_pstUDMainQuestCancel = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_MAIN_QUSET_CANCEL, NULL, 0, 0, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDMainQuestCancel)
		return FALSE;

	m_pstUDMainTree = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_MAIN_TREE, &m_lMainQuestIDList[0], sizeof(INT32), QUESTMAIN_MAX_COUNT, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDMainTree)
		return FALSE;

	m_pstUDMainTreeDepth = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_MAIN_TREE_DEPTH, &m_lMainQuestIDListDepth[0], sizeof(INT32), QUESTMAIN_MAX_COUNT, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDMainTreeDepth)
		return FALSE;

	m_pstUDQuestStart = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_QUEST_START, NULL, 0, 0, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDQuestStart)
		return FALSE;

	m_pstUDQuestComplete = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_QUEST_COMPLETE, NULL, 0, 0, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDQuestComplete)
		return FALSE;

	m_pstUDQuestCount = m_pcsAgcmUIManager2->AddUserData(AGCMUI_USERDATA_QUEST_COUNT, NULL, 0, 0, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDQuestCount)
		return FALSE;

	return TRUE;
}

BOOL AgcmUIQuest2::AddDisplay()
{
	if (!m_pcsAgcmUIManager2->AddDisplay(this, AGCMUI_DISPLAY_QUEST_ACCEPT, 0, CBDisplayQuestAcceptList, AGCDUI_USERDATA_TYPE_STRING))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, AGCMUI_DISPLAY_QUEST_ACCEPT_TEXT, 0, CBDisplayQuestAcceptText, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, AGCMUI_DISPLAY_QUEST_ACCEPT_LEFT_BUTTON, 0, CBDisplayQuestAcceptLeftButton, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, AGCMUI_DISPLAY_QUEST_MAIN_TEXT, 0, CBDisplayQuestMainText, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, AGCMUI_DISPLAY_QUEST_MAIN_CANCEL, 0, CBDisplayQuestMainCancel, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, AGCMUI_DISPLAY_QUEST_MAIN_TREE, 0, CBDisplayQuestMainTree, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if(!m_pcsAgcmUIManager2->AddDisplay( this , AGCMUI_DISPLAY_QUEST_MAIN_CHECK , 0 , CBDisplayQuestMainTreeCheck , AGCDUI_USERDATA_TYPE_INT32 ))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, AGCMUI_DISPLAY_QUEST_START, 0, CBDisplayQuestStart, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, AGCMUI_DISPLAY_QUEST_COMPLETE, 0, CBDisplayQuestComplete, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, AGCMUI_DISPLAY_QUEST_COUNT, 0, CBDispalyQuestCount, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	m_lEventQuestCancel = m_pcsAgcmUIManager2->AddEvent(AGCMUI_EVENT_QUEST_CANCEL_DLG_OPEN, CBReturnCancelQuest, this);
	if (m_lEventQuestCancel < 0)
		return FALSE;

	return TRUE;
}

BOOL AgcmUIQuest2::RefreshAcceptList()
{
	m_pstUDAcceptList->m_stUserData.m_lCount = m_lAcceptListCount;
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDAcceptList);
}

BOOL AgcmUIQuest2::OpenWorldMap( INT nRegionIndex )
{
	m_pcsAgcmUIWorldmap->OpenWorldMap( nRegionIndex );
	return TRUE;
}

BOOL AgcmUIQuest2::RefreshAcceptListAutoStart()
{
	INT32	lListCount;

	MakeAcceptListAutoStart(&m_AcceptListAutoStart[0], &lListCount);
	if (m_pcsAgcmUIMain)
	{
		if (lListCount && !m_bOpenAutoStart)
		{
			CHAR *			szTextureName = m_pcsAgcmUIControl->GetCustomTexture(AGCMUI_AUTOSTART_ICON);
			RwTexture **	ppTexture = m_pcsAgcmUIControl->GetAttachGridItemTextureData(m_pcsAutoStartIcon);

			if (!ppTexture || !szTextureName)
				return FALSE;

			if (!*ppTexture)
				*ppTexture = RwTextureRead(szTextureName, NULL);
				
			m_pcsAutoStartIcon->SetTooltip( "Self Quest" );

			m_pcsAgcmUIMain->AddSystemMessageGridItem(m_pcsAutoStartIcon, CBClickAutoStartQuest, this);

			m_bOpenAutoStart = TRUE;
		}
		else if (!lListCount && m_bOpenAutoStart)
		{
			m_pcsAgcmUIMain->RemoveSystemMessageGridItem(m_pcsAutoStartIcon);

			m_bOpenAutoStart = FALSE;
		}
	}

	return TRUE;
}
//퀘스트 저널관련이당
VOID AgcmUIQuest2::RefreshQuestUI()
{
	RefreshMainTree();
	RefreshMainText();
// 	RefreshMainCompensation();
// 
	m_pcsAgcmUIQuestJournal->RefreshJournalInfo();
	m_pcsAgcmUIQuestJournal->RefreshJournalPosition();

}

BOOL AgcmUIQuest2::CBClickAutoStartQuest(PVOID pData, PVOID pClass, PVOID pCustData)
{
	AgcmUIQuest2 *	pThis = (AgcmUIQuest2 *) pClass;

	pThis->MakeAcceptListAutoStart(&pThis->m_AcceptList[0], &pThis->m_lAcceptListCount);
	pThis->RefreshAcceptList();
	pThis->RefreshAcceptText();
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventAcceptUIOpen);
	//pThis->m_stOpenPos = pAgpdCharacter->m_stPos;
	pThis->m_bIsQuestUIOpen	= TRUE;

	return TRUE;
}

BOOL AgcmUIQuest2::UpdateQuestMainTree()
{
	INT32 lIndex;
	AgpdQuestTemplate *pQuestTemplate, *pSequenceQuestTemplate;
	m_lMainQuestTreeCount = 0;

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsCharacter)
		return FALSE;

	AgpdQuest *pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsCharacter);
	if (!pAgpdQuest)
		return FALSE;

	vector<INT32> RegionArray;
	// 지역 리스트를 먼저 모은다.
	for (AgpdQuest::IterQuest iter = pAgpdQuest->m_vtQuest.begin(); iter != pAgpdQuest->m_vtQuest.end(); ++iter)
	{
		BOOL bFind = FALSE;
		pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get((*iter).lQuestID);
		if (!pQuestTemplate) 
			continue;

		for (vector<INT32>::iterator iterReg = RegionArray.begin(); iterReg != RegionArray.end(); ++iterReg)
		{
			if ((*iterReg == pQuestTemplate->m_QuestInfo.lLocation) && (pQuestTemplate->m_QuestInfo.eQuestCategory == AGPDQUEST_SYSTEM_TYPE_NORMAL))
				bFind = TRUE;
		}
        
		// 같은게 없으면 하나 추가~
		if (!bFind && (pQuestTemplate->m_QuestInfo.eQuestCategory == AGPDQUEST_SYSTEM_TYPE_NORMAL))
			RegionArray.push_back(pQuestTemplate->m_QuestInfo.lLocation);
	}

	// 먼저 시나리오 퀘스트가 아닌 일반 퀘스트부터 지역명에 맞게 정렬한다.
	for (vector<INT32>::iterator iterReg = RegionArray.begin(); iterReg != RegionArray.end(); ++iterReg)
	{
		// 지역 이름 삽입
		m_lMainQuestIDList[m_lMainQuestTreeCount]		= *iterReg;
		m_lMainQuestIDListDepth[m_lMainQuestTreeCount]	= TREE_ROOT;
		m_eMainQuestIDStatus[m_lMainQuestTreeCount]		= MAINUI_AREA;
		++m_lMainQuestTreeCount;

		for (AgpdQuest::IterQuest iter = pAgpdQuest->m_vtQuest.begin(); iter != pAgpdQuest->m_vtQuest.end(); ++iter)
		{ 
			pSequenceQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get((*iter).lQuestID);
			if ((!pSequenceQuestTemplate) || (m_lMainQuestTreeCount >= QUESTMAIN_MAX_COUNT))
				continue;

			if (AGPDQUEST_SYSTEM_TYPE_NORMAL != pSequenceQuestTemplate->m_QuestInfo.eQuestCategory)
				continue;
			
			if (pSequenceQuestTemplate->m_QuestInfo.lLocation == *iterReg)
			{
				m_lMainQuestIDList[m_lMainQuestTreeCount]		= pSequenceQuestTemplate->m_lID;
				m_lMainQuestIDListDepth[m_lMainQuestTreeCount]	= TREE_DEPTH_1;
				if (m_pcsAgpmQuest->EvaluationCompleteCondition(pcsCharacter, pSequenceQuestTemplate->m_lID))
					m_eMainQuestIDStatus[m_lMainQuestTreeCount]		= MAINUI_COMPLETE;
				else
					m_eMainQuestIDStatus[m_lMainQuestTreeCount]		= MAINUI_CURRENT;
				++m_lMainQuestTreeCount;
			}
		}
	}

	for (AgpdQuest::IterQuest iter = pAgpdQuest->m_vtQuest.begin(); iter != pAgpdQuest->m_vtQuest.end(); ++iter)
	{
		pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get((*iter).lQuestID);
		if ((!pQuestTemplate) || (m_lMainQuestTreeCount >= QUESTMAIN_MAX_COUNT))
			continue;

		//if (AGPDQUEST_SYSTEM_TYPE_SCENARIO != pQuestTemplate->m_QuestInfo.eQuestCategory)
		//{
		//	m_lMainQuestIDList[m_lMainQuestTreeCount]		= pQuestTemplate->m_lID;
		//	m_lMainQuestIDListDepth[m_lMainQuestTreeCount]	= TREE_ROOT;
		//	m_eMainQuestIDStatus[m_lMainQuestTreeCount]		= MAINUI_CURRENT;
		//	++m_lMainQuestTreeCount;
		//	continue;
		//}

		if ((AGPDQUEST_SYSTEM_TYPE_SCENARIO != pQuestTemplate->m_QuestInfo.eQuestCategory) && 
			(AGPDQUEST_SYSTEM_TYPE_EPIC != pQuestTemplate->m_QuestInfo.eQuestCategory))
			continue;

		ASSERT(strlen(pQuestTemplate->m_QuestInfo.szScenarioName) > 0);

		// 시나리오 이름 삽입
		m_lMainQuestIDList[m_lMainQuestTreeCount]		= pQuestTemplate->m_lID;
		m_lMainQuestIDListDepth[m_lMainQuestTreeCount]	= TREE_ROOT;
		m_eMainQuestIDStatus[m_lMainQuestTreeCount]		= MAINUI_SCENARIO;
		++m_lMainQuestTreeCount;

		// 시나리오에 해당하는 퀘스트 리스트 삽입
		lIndex = 0;
		for (pSequenceQuestTemplate = m_pcsAgpmQuest->GetTemplateSequence(&lIndex); pSequenceQuestTemplate; pSequenceQuestTemplate = m_pcsAgpmQuest->GetTemplateSequence(&lIndex))
		{
			if (pQuestTemplate->m_StartCondition.lRace)
			{
				if (pSequenceQuestTemplate->m_StartCondition.lRace != m_pcsAgpmFactors->GetRace(&pcsCharacter->m_csFactor))
					continue;
			}

			if (pQuestTemplate->m_StartCondition.lClass)
			{
				if (pSequenceQuestTemplate->m_StartCondition.lClass != m_pcsAgpmFactors->GetClass(&pcsCharacter->m_csFactor))
					continue;
			}

			// 같은 시나리오 인지 확인
			if (0 == strcmp(pQuestTemplate->m_QuestInfo.szScenarioName, pSequenceQuestTemplate->m_QuestInfo.szScenarioName))
			{
				if (pQuestTemplate->m_lID == pSequenceQuestTemplate->m_lID)
				{
					// 현재 진행중인 퀘스트
					m_lMainQuestIDList[m_lMainQuestTreeCount]		= pSequenceQuestTemplate->m_lID;
					m_lMainQuestIDListDepth[m_lMainQuestTreeCount]	= TREE_DEPTH_1;
					if (m_pcsAgpmQuest->EvaluationCompleteCondition(pcsCharacter, pSequenceQuestTemplate->m_lID))
						m_eMainQuestIDStatus[m_lMainQuestTreeCount]		= MAINUI_COMPLETE;
					else
						m_eMainQuestIDStatus[m_lMainQuestTreeCount]		= MAINUI_CURRENT;
					++m_lMainQuestTreeCount;
				}
				else
				{
					// TODO : 이거는 실제 m_lID로 완료된 퀘스트 인지 Flag를 이용하여 확인해야 한다.
					if (pQuestTemplate->m_lID > pSequenceQuestTemplate->m_lID)
					{
						m_lMainQuestIDList[m_lMainQuestTreeCount]		= pSequenceQuestTemplate->m_lID;
						m_lMainQuestIDListDepth[m_lMainQuestTreeCount]	= TREE_DEPTH_1;
						m_eMainQuestIDStatus[m_lMainQuestTreeCount]		= MAINUI_SCENARIO_COMPLETE;
						++m_lMainQuestTreeCount;
					}
					else
					{
						m_lMainQuestIDList[m_lMainQuestTreeCount]		= pSequenceQuestTemplate->m_lID;
						m_lMainQuestIDListDepth[m_lMainQuestTreeCount]	= TREE_DEPTH_1;
						m_eMainQuestIDStatus[m_lMainQuestTreeCount]		= MAINUI_UNKNOWN;
						++m_lMainQuestTreeCount;
					}
				}
			}
		}		
	}
	
	m_pstUDMainTree->m_stUserData.m_lCount = m_lMainQuestTreeCount;

	return TRUE;
}

BOOL AgcmUIQuest2::CBDisplayQuestAcceptList(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || eType != AGCDUI_USERDATA_TYPE_STRING)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;

	AgpdQuestTemplate *pQuest = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(
									pThis->m_AcceptList[pcsSourceControl->m_lUserDataIndex].lQuestTID);

	if (!pQuest) return FALSE;

	switch (pThis->m_AcceptList[pcsSourceControl->m_lUserDataIndex].eStatus)
	{
	case ACCEPTUI_STATUS_NEW:			
		{
			sprintf(szDisplay, " [%s] %s%d %s", ClientStr().GetStr(STI_POSSIBLE), ClientStr().GetStr(STI_LEVEL),
				pQuest->m_StartCondition.lLevel, pQuest->m_QuestInfo.szName);	
			pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor = COLOR_QUEST_NEW;
		}
		break;

	case ACCEPTUI_STATUS_INCOMPLETE_GRANT:
	case ACCEPTUI_STATUS_INCOMPLETE:	
		{
			sprintf(szDisplay, " [%s] %s%d %s", ClientStr().GetStr(STI_ING), ClientStr().GetStr(STI_LEVEL),
				pQuest->m_StartCondition.lLevel, pQuest->m_QuestInfo.szName);
			pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor = COLOR_QUEST_INCOMPLETE;
		}
		break;

	case ACCEPTUI_STATUS_COMPLETE:		
		{
			sprintf(szDisplay, " [%s] %s%d %s", ClientStr().GetStr(STI_COMPLETE), ClientStr().GetStr(STI_LEVEL),
				pQuest->m_StartCondition.lLevel, pQuest->m_QuestInfo.szName);
			pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor = COLOR_QUEST_COMPLETE;
		}
		break;
	}
	
	return TRUE;
}

BOOL AgcmUIQuest2::CBButtonQuestAcceptListClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;

	if (pThis->m_lAcceptListCount < pcsSourceControl->m_lUserDataIndex) return FALSE;

	pThis->m_lClickIDAcceptUI = pcsSourceControl->m_lUserDataIndex;

	switch(pThis->m_AcceptList[pThis->m_lClickIDAcceptUI].eStatus)
	{
	case ACCEPTUI_STATUS_NEW:
	case ACCEPTUI_STATUS_COMPLETE:
		pThis->RefreshAcceptLeftButtonEnable(TRUE);
		break;

	case ACCEPTUI_STATUS_INCOMPLETE_GRANT:
	case ACCEPTUI_STATUS_INCOMPLETE:
		pThis->RefreshAcceptLeftButtonEnable(FALSE);
		break;
	}

	pThis->RefreshAcceptLeftButton();
	return pThis->RefreshAcceptText();
}

BOOL AgcmUIQuest2::RefreshAcceptText()
{
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDAcceptText);
}

BOOL AgcmUIQuest2::RefreshMainText()
{
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDMainText);
}

BOOL AgcmUIQuest2::RefreshMainTree()
{
	if (!UpdateQuestMainTree()) return FALSE;
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDMainTree);
}

BOOL AgcmUIQuest2::RefreshMainCount()
{
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDQuestCount);
}

BOOL AgcmUIQuest2::RefreshAcceptLeftButton()
{
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDAcceptLeftButton);
}

BOOL AgcmUIQuest2::RefreshAcceptLeftButtonEnable(BOOL bEnable)
{
	m_bAcceptLeftButtonEnable = bEnable;
	return m_pcsAgcmUIManager2->SetUserDataRefresh(m_pstUDAcceptLeftButtonEnable);
}

BOOL AgcmUIQuest2::CBDisplayQuestAcceptText(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
// 	if (!pClass || eType != AGCDUI_USERDATA_TYPE_INT32)
// 		return FALSE;
// 
// 	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
// 
// 	if (pcsSourceControl->m_lType != AcUIBase::TYPE_EDIT) return FALSE;
// 	AcUIEdit* pUIEdit = (AcUIEdit *) pcsSourceControl->m_pcsBase;
// 
// 	if (INVALID_INDEX == pThis->m_lClickIDAcceptUI && INVALID_INDEX == pThis->m_lLastCompleteTID)
// 	{
// 		pUIEdit->SetText("");
// 		return FALSE;
// 	}
// 
// 	AgpdQuestTemplate* pQuestTemplate;
// 	if (INVALID_INDEX == pThis->m_lLastCompleteTID)
// 		pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pThis->m_AcceptList[pThis->m_lClickIDAcceptUI].lQuestTID);
// 	else
// 		pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pThis->m_lLastCompleteTID);
// 
// 	if (!pQuestTemplate) 
// 		return FALSE;
// 
// 	pUIEdit->SetLineDelimiter("</n>");
// 
// 	ApSafeArray<CHAR, TEXT_BUFFER_SIZE> szBuffer;
// 
// 	pThis->ComposeAcceptText(pQuestTemplate, &szBuffer[0], TEXT_BUFFER_SIZE);
// 
// 	pUIEdit->SetText(&szBuffer[0]);
	AgcmUIQuest2* pThis = static_cast<AgcmUIQuest2*> (pClass);
	if(!pThis)
		return FALSE;

	AcUIBase*		pUIBase	=	static_cast< AcUIBase* >(pcsSourceControl->m_pcsBase);
	if( !pUIBase )
		return FALSE;

	pThis->SetScrollButton();
	pThis->_RefreshTextList( E_QUEST_TEXT_ACCEPT , pUIBase , TRUE );

	return TRUE;
}

BOOL AgcmUIQuest2::CBDisplayQuestMainText(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || eType != AGCDUI_USERDATA_TYPE_INT32)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	CHAR szBuffer[TEXT_BUFFER_SIZE] = {0,};

	AcUIBase*		pUIBase	=	static_cast< AcUIBase* >(pcsSourceControl->m_pcsBase);
	if( !pUIBase )
		return FALSE;

	if ( INVALID_INDEX == pThis->m_lClickIDMainUI)
		return FALSE;

	AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pThis->m_lMainQuestIDList[pThis->m_lClickIDMainUI]);

	if (!pQuestTemplate)
		return FALSE;

	pThis->SetScrollButton();
	pThis->_RefreshTextList( E_QUEST_TEXT_MAIN , pUIBase , TRUE );

// 	if (pcsSourceControl->m_lType != AcUIBase::TYPE_EDIT) return FALSE;
// 	AcUIEdit* pUIEdit = (AcUIEdit *) pcsSourceControl->m_pcsBase;
// 
// 	if (INVALID_INDEX == pThis->m_lClickIDMainUI)
// 	{
// 		pUIEdit->SetText("");
// 		return FALSE;
// 	}
// 
// 	AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pThis->m_lMainQuestIDList[pThis->m_lClickIDMainUI]);
// 	if (!pQuestTemplate || !pThis->m_lMainQuestTreeCount)
// 	{
// 		pUIEdit->SetText(szBuffer);
// 		return FALSE;
// 	}
// 
// 	pUIEdit->SetLineDelimiter("</n>");
// 
// 	pThis->ComposeMainText(pQuestTemplate, szBuffer, TEXT_BUFFER_SIZE);
// 
// 	pUIEdit->SetText(szBuffer);

	return TRUE;
}

BOOL AgcmUIQuest2::CBDisplayQuestMainCancel(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || eType != AGCDUI_USERDATA_TYPE_INT32)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pThis->m_lMessageBoxQuestTID);
	if (!pQuestTemplate) return FALSE;

	sprintf(szDisplay, ClientStr().GetStr(STI_QUEST_CANCEL), pQuestTemplate->m_QuestInfo.szName);

	return TRUE;
}

BOOL AgcmUIQuest2::CBDisplayQuestStart(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || eType != AGCDUI_USERDATA_TYPE_INT32)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pThis->m_lMessageBoxQuestTID);
	if (!pQuestTemplate) return FALSE;

	sprintf(szDisplay, ClientStr().GetStr(STI_TAKE_QUEST), pQuestTemplate->m_QuestInfo.szName);

	return TRUE;
}

BOOL AgcmUIQuest2::CBDisplayQuestComplete(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || eType != AGCDUI_USERDATA_TYPE_INT32)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pThis->m_lMessageBoxQuestTID);

	if (!pQuestTemplate) return FALSE;

	sprintf(szDisplay, ClientStr().GetStr(STI_QUEST_COMPLETE), pQuestTemplate->m_QuestInfo.szName);

	return TRUE;
}

BOOL AgcmUIQuest2::CBDisplayQuestAcceptLeftButton(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || eType != AGCDUI_USERDATA_TYPE_INT32)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	if (INVALID_INDEX == pThis->m_lClickIDAcceptUI)
	{
		sprintf(szDisplay, ClientStr().GetStr(STI_COMPLETE));
		return TRUE;
	}

	switch(pThis->m_AcceptList[pThis->m_lClickIDAcceptUI].eStatus)
	{
	case ACCEPTUI_STATUS_NEW:
		sprintf(szDisplay, ClientStr().GetStr(STI_ACCEPT));	break;

	case ACCEPTUI_STATUS_INCOMPLETE:
	case ACCEPTUI_STATUS_COMPLETE:
		sprintf(szDisplay, ClientStr().GetStr(STI_COMPLETE));	break;

	default:
		sprintf(szDisplay, ClientStr().GetStr(STI_COMPLETE));	break;
	}

	return TRUE;
}

BOOL AgcmUIQuest2::CBDisplayQuestMainTreeCheck(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;

	if (pThis->m_lMainQuestTreeCount < pcsSourceControl->m_lUserDataIndex)
		return FALSE;

	pcsSourceControl->m_pcsBase->m_Property.bVisible	=	FALSE;

	if( pThis->m_eMainQuestIDStatus[ pcsSourceControl->m_lUserDataIndex] == MAINUI_SCENARIO ||
		pThis->m_eMainQuestIDStatus[ pcsSourceControl->m_lUserDataIndex] == MAINUI_SCENARIO_COMPLETE ||
		pThis->m_eMainQuestIDStatus[ pcsSourceControl->m_lUserDataIndex] == MAINUI_UNKNOWN	)
	{
		return FALSE;
	}

	INT	nQuestID	=	pThis->m_lMainQuestIDList[ pcsSourceControl->m_lUserDataIndex ];

	switch(pThis->m_eMainQuestIDStatus[pcsSourceControl->m_lUserDataIndex])
	{
	case MAINUI_CURRENT:
	case MAINUI_COMPLETE:
		{
			pcsSourceControl->m_pcsBase->m_Property.bVisible	=	TRUE;

			((AcUIButton*)pcsSourceControl->m_pcsBase)->SetButtonImage( 1 , ACUIBUTTON_MODE_NORMAL );
			((AcUIButton*)pcsSourceControl->m_pcsBase)->SetButtonImage( 1 , ACUIBUTTON_MODE_ONMOUSE );
			((AcUIButton*)pcsSourceControl->m_pcsBase)->SetButtonImage( 1 , ACUIBUTTON_MODE_CLICK );
			((AcUIButton*)pcsSourceControl->m_pcsBase)->SetButtonImage( 1 , ACUIBUTTON_MODE_DISABLE );

			if( pThis->m_pcsAgcmQuestJournal->IsQuestJournal( nQuestID ) )
			{
				((AcUIButton*)pcsSourceControl->m_pcsBase)->SetButtonImage( 2 , ACUIBUTTON_MODE_NORMAL );
				((AcUIButton*)pcsSourceControl->m_pcsBase)->SetButtonImage( 2 , ACUIBUTTON_MODE_ONMOUSE );
				((AcUIButton*)pcsSourceControl->m_pcsBase)->SetButtonImage( 2 , ACUIBUTTON_MODE_CLICK );
				((AcUIButton*)pcsSourceControl->m_pcsBase)->SetButtonImage( 2 , ACUIBUTTON_MODE_DISABLE );
			}

		}
		break;
	};

	return TRUE;
}

BOOL AgcmUIQuest2::CBDisplayQuestMainTree(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if ((!pClass) || (eType != AGCDUI_USERDATA_TYPE_INT32) || (!pcsSourceControl))
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;

	if (pThis->m_lMainQuestTreeCount < pcsSourceControl->m_lUserDataIndex)
		return FALSE;

	INT32 lQuestID = pThis->m_lMainQuestIDList[pcsSourceControl->m_lUserDataIndex];

	switch(pThis->m_eMainQuestIDStatus[pcsSourceControl->m_lUserDataIndex])
	{
	case MAINUI_CURRENT:	
		{
			AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestID);
			if (!pQuestTemplate) return FALSE;

			pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor = COLOR_QUEST_INCOMPLETE;
			sprintf(szDisplay, "%s%d %s (%s)", ClientStr().GetStr(STI_LEVEL), pQuestTemplate->m_StartCondition.lLevel,
				ClientStr().GetStr(STI_ING), pQuestTemplate->m_QuestInfo.szName); 
		} 
		break;

	case MAINUI_COMPLETE:	
		{
			AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestID);
			if (!pQuestTemplate) return FALSE;

			pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor = COLOR_QUEST_COMPLETE;
			sprintf(szDisplay, "%s%d %s (%s)", ClientStr().GetStr(STI_LEVEL), pQuestTemplate->m_StartCondition.lLevel,
				ClientStr().GetStr(STI_COMPLETE), pQuestTemplate->m_QuestInfo.szName); 
		}
		break;

	case MAINUI_UNKNOWN:	
		{
			AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestID);
			if (!pQuestTemplate) return FALSE;

			pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor = COLOR_QUEST_UNKNOWN;
			sprintf(szDisplay, "???"); 
		}
		break;

	case MAINUI_SCENARIO:	
		{
			AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestID);
			if (!pQuestTemplate) return FALSE;

			pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor = COLOR_QUEST_SCENARIO;
			sprintf(szDisplay, "%s", pQuestTemplate->m_QuestInfo.szScenarioName); 
		}
		break;

	case MAINUI_SCENARIO_COMPLETE:
		{
			AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestID);
			if (!pQuestTemplate) return FALSE;

			pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor = COLOR_QUEST_COMPLETE;
			sprintf(szDisplay, "%s", pQuestTemplate->m_QuestInfo.szName); 
		}
		break;

	case MAINUI_AREA:
		{
			ApmMap::RegionTemplate *pTemplate = pThis->m_pcsApmMap->GetTemplate(lQuestID);
			if (pTemplate)
			{
				pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor = COLOR_QUEST_AREA;
				sprintf(szDisplay, "%s", pTemplate->pStrName);
			}
		}
		break;
	};

	return TRUE;
}

BOOL AgcmUIQuest2::CBButtonAcceptLeftClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;

	AgpdCharacter* pCharacter = pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pCharacter) 
		return FALSE;

	AgpdQuest* pcsAgpdQuest = pThis->m_pcsAgpmQuest->GetAttachAgpdQuestData(pCharacter);
	if (!pcsAgpdQuest)
		return FALSE;

	if (INVALID_INDEX == pThis->m_lClickIDAcceptUI)
		return FALSE;

	AgpdQuestTemplate* pcsApgdQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(
																	pThis->m_AcceptList[pThis->m_lClickIDAcceptUI].lQuestTID);
	if (!pcsApgdQuestTemplate) 
		return FALSE;

	switch(pThis->m_AcceptList[pThis->m_lClickIDAcceptUI].eStatus)
	{
	// 수락 버튼을 눌렀을때 진행 되어야 할 일
	case ACCEPTUI_STATUS_NEW:
		{
			if (pcsAgpdQuest->m_vtQuest.size() >= AGPDQUEST_CURRENT_ID_MAX)
			{
				pThis->m_lMessageBoxQuestTID = pThis->m_AcceptList[pThis->m_lClickIDAcceptUI].lQuestTID;
				pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lMessageQuestFull);

				CHAR szBuffer[256];
				ZeroMemory(szBuffer, 256);
				INT32 lSize = sprintf(szBuffer, "<HFFF00000>[%s] %s<HFFFFFFFF>", pcsApgdQuestTemplate->m_QuestInfo.szName, ClientStr().GetStr(STI_NO_MORE_QUEST));
				ASSERT(lSize < 256);
				SystemMessage.ProcessSystemMessage(szBuffer);
			}

			return pThis->m_pcsAgcmQuest->SendPacketRequireQuest(pThis->m_AcceptList[pThis->m_lClickIDAcceptUI].lQuestTID, 
												pThis->m_pcsAgcmEventQuest->m_pcsLastGrantEvent, pCharacter->m_lID);
		}
		break;

	// 완료 버튼을 눌렀을때 진행 되어야 할 일
	case ACCEPTUI_STATUS_COMPLETE:
		{
			AgpdItemADChar* pcsAgpdItemADChar = pThis->m_pcsAgpmQuest->m_pcsAgpmItem->GetADCharacter(pCharacter);
			if (!pcsAgpdItemADChar) 
				return FALSE;

			INT32 lResultItemCount = 0;
			if (pcsApgdQuestTemplate->m_Result.Item1.pItemTemplate) 
				++lResultItemCount;
			if (pcsApgdQuestTemplate->m_Result.Item2.pItemTemplate)
				++lResultItemCount;
			if (pcsApgdQuestTemplate->m_Result.Item3.pItemTemplate)
				++lResultItemCount;
			if (pcsApgdQuestTemplate->m_Result.Item4.pItemTemplate)
				++lResultItemCount;

			if ((pcsAgpdItemADChar->m_csInventoryGrid.m_lGridCount - pcsAgpdItemADChar->m_csInventoryGrid.m_lItemCount) < lResultItemCount)
			{
				pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lMessageMainInventoryFull);
				return FALSE;
			}
			
			if( pcsApgdQuestTemplate->m_Result.Selectableitem[0].pItemTemplate && pThis->acceptQuestText_->SelectedButtonIdx() == -1 )
			{
				CHAR	szMessage[ MAX_PATH ];
				sprintf_s( szMessage , MAX_PATH , "%s" , pThis->m_pcsAgcmUIManager2->GetUIMessage( "QuestSelectItem" ) );
				pThis->Notice( szMessage );
				return FALSE;
			}

			// 서버에 완료 요청을 보낸다.
			pThis->m_pcsAgcmQuest->SendPacketCompleteQuest(pThis->m_AcceptList[pThis->m_lClickIDAcceptUI].lQuestTID, 
													pThis->m_pcsAgcmEventQuest->m_pcsLastGrantEvent, pCharacter->m_lID, pThis->acceptQuestText_->SelectedButtonIdx());
		}
		break;
	}

	return TRUE;
}

BOOL AgcmUIQuest2::ComposeAcceptText(AgpdQuestTemplate *pQuestTemplate, char *pBuffer, INT32 lBufferSize)
{
	INT32 lIndex = 0, i = 0;
	AgpdCharacter *pcsAgpdCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsAgpdCharacter)
		return FALSE;

	lIndex = sprintf(pBuffer,	"</n><HFFB6FF38>\"%s\"<HFFFFFFFF></n>"		// 퀘스트 제목
								"%s</n>",										// 퀘스트 지역
								pQuestTemplate->m_QuestInfo.szName, pQuestTemplate->m_QuestInfo.szAreaInfo);		


	if (INVALID_INDEX != m_lLastCompleteTID)
	{
		ComposeAcceptTextComplete(pQuestTemplate, &pBuffer[lIndex], lBufferSize - lIndex);
		m_lLastCompleteTID = INVALID_INDEX;
		return TRUE;
	}

	switch (m_AcceptList[m_lClickIDAcceptUI].eStatus)
	{
	case ACCEPTUI_STATUS_NEW:			ComposeAcceptTextNew(pQuestTemplate, &pBuffer[lIndex], lBufferSize - lIndex);			break;
	case ACCEPTUI_STATUS_COMPLETE:		ComposeAcceptTextComplete(pQuestTemplate, &pBuffer[lIndex], lBufferSize - lIndex);		break;
	case ACCEPTUI_STATUS_INCOMPLETE_GRANT:
	case ACCEPTUI_STATUS_INCOMPLETE:	ComposeAcceptTextIncomplete(pQuestTemplate, &pBuffer[lIndex], lBufferSize - lIndex);	break;
	}

	m_lLastCompleteTID = INVALID_INDEX;

	return TRUE;
}

INT32 AgcmUIQuest2::ComposeAcceptTextNew(AgpdQuestTemplate *pQuestTemplate, char *pBuffer, INT32 lBufferSize)
{
	INT32 lIndex = 0;

	lIndex += sprintf(&pBuffer[lIndex], "</n><H%x>%s<H%x></n>", COLOR_QUEST_CONDITION, ClientStr().GetStr(STI_CONTENTS), COLOR_QUEST_DEFAULT);
	lIndex += sprintf(&pBuffer[lIndex], "%s</n>", pQuestTemplate->m_DialogText.szAcceptText);

	lIndex += sprintf(&pBuffer[lIndex], "</n><H%x>%s<H%x></n>", COLOR_QUEST_CONDITION, ClientStr().GetStr(STI_COMPLETE_CONDITION), COLOR_QUEST_DEFAULT);
	lIndex += ComposeAcceptSummaryText(&pBuffer[lIndex], lBufferSize - lIndex, pQuestTemplate);

	lIndex += sprintf(&pBuffer[lIndex], "</n><H%x>%s<H%x></n>", COLOR_QUEST_CONDITION, ClientStr().GetStr(STI_REWARD), COLOR_QUEST_DEFAULT);
	if (pQuestTemplate->m_Result.lMoney)
		lIndex += sprintf(&pBuffer[lIndex], "%d %s</n>", pQuestTemplate->m_Result.lMoney, ClientStr().GetStr(STI_GELD));

	if (pQuestTemplate->m_Result.lExp)
		lIndex += sprintf(&pBuffer[lIndex], "%s %d</n>", ClientStr().GetStr(STI_EXP), pQuestTemplate->m_Result.lExp);

	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item1, &pBuffer[lIndex], lIndex);
	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item2, &pBuffer[lIndex], lIndex);
	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item3, &pBuffer[lIndex], lIndex);
	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item4, &pBuffer[lIndex], lIndex);

	if( pQuestTemplate->m_Result.Selectableitem[0].pItemTemplate != NULL )
	{
		lIndex += sprintf( &pBuffer[ lIndex] , "</n></n><H%x>%s<H%x></n>" , COLOR_QUEST_CONDITION , ClientStr().GetStr(ST_QUEST_SELECT_ITEM) , COLOR_QUEST_DEFAULT );

		// 선택 보상 아이템
		for( INT i = 0 ; i < AGPDQUEST_MAX_RESULT_ITEM_SELECTABLE ; ++i )
			lIndex += ComposeAcceptItem( &pQuestTemplate->m_Result.Selectableitem[i] , &pBuffer[lIndex] , lIndex );
	}

	if (pQuestTemplate->m_Result.ProductSkill.pSkillTemplate)
		lIndex += sprintf(&pBuffer[lIndex], "%s (%d)", pQuestTemplate->m_Result.ProductSkill.pSkillTemplate->m_szName,
									pQuestTemplate->m_Result.ProductSkill.lExp);

	ASSERT(lIndex < lBufferSize);
	return lIndex;
}

INT32 AgcmUIQuest2::ComposeAcceptTextIncomplete(AgpdQuestTemplate *pQuestTemplate, char *pBuffer, INT32 lBufferSize)
{
	INT32 lIndex = 0;
	lIndex += sprintf(&pBuffer[lIndex], "</n><H%x>%s<H%x></n>", COLOR_QUEST_CONDITION, ClientStr().GetStr(STI_CONTENTS), COLOR_QUEST_DEFAULT);
	lIndex += sprintf(&pBuffer[lIndex], "%s</n>", pQuestTemplate->m_DialogText.szIncompleteText);

	ASSERT(lIndex < lBufferSize);
	return lIndex;
}

INT32 AgcmUIQuest2::ComposeAcceptTextComplete(AgpdQuestTemplate *pQuestTemplate, char *pBuffer, INT32 lBufferSize)
{
	INT32 lIndex = 0;
	lIndex += sprintf(&pBuffer[lIndex], "</n><H%x>%s<H%x></n>", COLOR_QUEST_CONDITION, ClientStr().GetStr(STI_CONTENTS), COLOR_QUEST_DEFAULT);
	lIndex += sprintf(&pBuffer[lIndex], "%s</n>", pQuestTemplate->m_DialogText.szCompleteText);

	lIndex += sprintf(&pBuffer[lIndex], "</n><H%x>%s<H%x></n>", COLOR_QUEST_CONDITION, ClientStr().GetStr(STI_REWARD), COLOR_QUEST_DEFAULT);
	if (pQuestTemplate->m_Result.lMoney)
		lIndex += sprintf(&pBuffer[lIndex], "%d %s</n>", pQuestTemplate->m_Result.lMoney, ClientStr().GetStr(STI_GELD));

	if (pQuestTemplate->m_Result.lExp)
		lIndex += sprintf(&pBuffer[lIndex], "%s %d</n>", ClientStr().GetStr(STI_EXP), pQuestTemplate->m_Result.lExp);

	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item1, &pBuffer[lIndex], lIndex);
	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item2, &pBuffer[lIndex], lIndex);
	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item3, &pBuffer[lIndex], lIndex);
	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item4, &pBuffer[lIndex], lIndex);

	if( pQuestTemplate->m_Result.Selectableitem[0].pItemTemplate != 0 )
	{

		lIndex += sprintf( &pBuffer[ lIndex] , "</n></n><H%x>%s<H%x></n>" , COLOR_QUEST_CONDITION , ClientStr().GetStr(ST_QUEST_SELECT_ITEM) , COLOR_QUEST_DEFAULT );

		// 선택 보상 아이템
		for( INT i = 0 ; i < AGPDQUEST_MAX_RESULT_ITEM_SELECTABLE ; ++i )
			lIndex += ComposeAcceptItem( &pQuestTemplate->m_Result.Selectableitem[i] , &pBuffer[lIndex] , lIndex , TRUE );
	}

	if (pQuestTemplate->m_Result.ProductSkill.pSkillTemplate)
		lIndex += sprintf(&pBuffer[lIndex], "%s (%d)", pQuestTemplate->m_Result.ProductSkill.pSkillTemplate->m_szName,
									pQuestTemplate->m_Result.ProductSkill.lExp);

	ASSERT(lIndex < lBufferSize);
	return lIndex;
}

INT32 AgcmUIQuest2::ComposeAcceptItem( AgpdElementItem *pItem, char* pBuffer, INT32 lBufferSize , BOOL bSelect /*= FALSE */ )
{
	AgpdItemTemplate*	pItemTemplate	=	pItem->pItemTemplate;

	INT32 lIndex = 0;
	if (pItemTemplate)
	{
		// 텍스처크기 , 아이템TID , 아이템갯수 , 소캣갯수 , 업그레이드여부 , 귀속타입
		lIndex	+=	sprintf( &pBuffer[lIndex] , "<ITEM::%d::%d::%d::%d::%d::%d>" , pItem->lCount , pItemTemplate->m_lID, pItem->lSocket , pItem->lUpgrade , pItem->m_eBoundType , bSelect );
	}
	return lIndex;
}

BOOL AgcmUIQuest2::ComposeMainText(AgpdQuestTemplate *pQuestTemplate, char *pBuffer, INT32 lBufferSize)
{
	INT32 lIndex = 0, i = 0;
	AgpdCharacter *pcsAgpdCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsAgpdCharacter)
		return FALSE;

	AgpdQuest *pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsAgpdCharacter);
	if (!pAgpdQuest)
		return FALSE;

	if (MAINUI_SCENARIO == m_eMainQuestIDStatus[m_lClickIDMainUI] ||
		MAINUI_UNKNOWN == m_eMainQuestIDStatus[m_lClickIDMainUI] ||
		MAINUI_AREA == m_eMainQuestIDStatus[m_lClickIDMainUI])
		return TRUE;

	lIndex = sprintf(pBuffer,	"</n><HFFB6FF38>\"%s\"<HFFFFFFFF></n>"			// 퀘스트 제목
								"%s</n></n>", pQuestTemplate->m_QuestInfo.szName, pQuestTemplate->m_QuestInfo.szAreaInfo);	// 퀘스트 지역

	lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s<H%x></n>", COLOR_QUEST_CONDITION, ClientStr().GetStr(STI_COMPLETE_CONDITION), COLOR_QUEST_DEFAULT);
	lIndex += ComposeMainSummaryText(&pBuffer[lIndex], lBufferSize - lIndex, pQuestTemplate, pAgpdQuest);

	lIndex += sprintf(&pBuffer[lIndex], "</n><H%x>%s<H%x></n>", COLOR_QUEST_CONDITION, ClientStr().GetStr(STI_REWARD), COLOR_QUEST_DEFAULT);

	if (pQuestTemplate->m_Result.lMoney)
		lIndex += sprintf(&pBuffer[lIndex], "%d %s</n>", pQuestTemplate->m_Result.lMoney, ClientStr().GetStr(STI_GELD));

	if (pQuestTemplate->m_Result.lExp)
		lIndex += sprintf(&pBuffer[lIndex], "%s %d</n>", ClientStr().GetStr(STI_EXP), pQuestTemplate->m_Result.lExp);

	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item1, &pBuffer[lIndex], lIndex);
	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item2, &pBuffer[lIndex], lIndex);
	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item3, &pBuffer[lIndex], lIndex);
	lIndex += ComposeAcceptItem(&pQuestTemplate->m_Result.Item4, &pBuffer[lIndex], lIndex);

	if (pQuestTemplate->m_Result.ProductSkill.pSkillTemplate)
		lIndex += sprintf(&pBuffer[lIndex], "%s (%d)", pQuestTemplate->m_Result.ProductSkill.pSkillTemplate->m_szName,
									pQuestTemplate->m_Result.ProductSkill.lExp);

	lIndex += sprintf(&pBuffer[lIndex], "</n><H%x>%s<H%x></n>", COLOR_QUEST_CONDITION, ClientStr().GetStr(STI_CONTENTS), COLOR_QUEST_DEFAULT);
	lIndex += sprintf(&pBuffer[lIndex], "%s</n>", pQuestTemplate->m_DialogText.szAcceptText);

	// 선택보상
	if( pQuestTemplate->m_Result.Selectableitem[0].pItemTemplate != NULL )
	{
		lIndex += sprintf( &pBuffer[ lIndex] , "</n></n><H%x>%s<H%x></n>" , COLOR_QUEST_CONDITION , ClientStr().GetStr(ST_QUEST_SELECT_ITEM) , COLOR_QUEST_DEFAULT );

		// 선택 보상 아이템
		for( INT i = 0 ; i < AGPDQUEST_MAX_RESULT_ITEM_SELECTABLE ; ++i )
			lIndex += ComposeAcceptItem( &pQuestTemplate->m_Result.Selectableitem[i] , &pBuffer[lIndex] , lIndex );
	}

	ASSERT(lIndex < lBufferSize);

	return TRUE;	
}

INT32 AgcmUIQuest2::ComposeMainSummaryText(CHAR* pBuffer, INT32 lBufferSize, AgpdQuestTemplate *pQuestTemplate, AgpdQuest *pAgpdQuest , BOOL bOnlyIncomplete , BOOL bOneSummary )
{
	INT32	lIndex	=	0;
	BOOL	bEnd	=	FALSE;
	AgpdQuestSummaryText *pSummaryText = &pQuestTemplate->m_SummaryText;

	AgpdCharacter *pcsAgpdCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsAgpdCharacter)
		return FALSE;

	AgpdItemADChar* pcsAgpdItemADChar = m_pcsAgpmQuest->m_pcsAgpmItem->GetADCharacter(pcsAgpdCharacter);
	if (!pcsAgpdItemADChar) 
		return FALSE;

	// 이 변수는 AGPDQUEST_SUMMARY_FACTOR_ITEM_COUNT 에서만 쓴다.
	INT	nItemCountIndex	=	0;
	INT32 lCount = 0;
	for (INT32 i = 0; i < AGPDQUEST_MAX_SUMMARY_TEXT_COUNT; ++i)
	{
		if (0 == strlen(pSummaryText->SummaryText[i].szSummaryText)) 
			continue;

		AgpdQuest::IterQuest iter = pAgpdQuest->FindByQuestID(pQuestTemplate->m_lID);
		if (iter == pAgpdQuest->m_vtQuest.end())
			continue;

		switch (pSummaryText->SummaryText[i].eFactor)
		{
		case AGPDQUEST_SUMMARY_FACTOR_LEVEL:
			if (m_pcsAgpmFactors->GetLevel(&pcsAgpdCharacter->m_csFactor) >= pQuestTemplate->m_CompleteCondition.lLevel)
			{
				if( !bOnlyIncomplete )
				{
					lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s</n><H%x>", 
						COLOR_QUEST_COMPLETE, pSummaryText->SummaryText[i].szSummaryText, COLOR_QUEST_DEFAULT);

					bEnd	=	TRUE;
				}
			}
			else
			{
				lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s</n><H%x>", 
					COLOR_QUEST_INCOMPLETE, pSummaryText->SummaryText[i].szSummaryText, COLOR_QUEST_DEFAULT);

				bEnd	=	TRUE;
			}
			break;

		case AGPDQUEST_SUMMARY_FACTOR_MONEY:
			if (pcsAgpdCharacter->m_llMoney >= pQuestTemplate->m_CompleteCondition.lMoney)
			{
				if( !bOnlyIncomplete )
				{
					lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s</n><H%x>",
						COLOR_QUEST_COMPLETE, pSummaryText->SummaryText[i].szSummaryText, COLOR_QUEST_DEFAULT);

					bEnd	=	TRUE;
				}
			}
			else
			{
				lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s</n><H%x>",
					COLOR_QUEST_INCOMPLETE, pSummaryText->SummaryText[i].szSummaryText, COLOR_QUEST_DEFAULT);

				bEnd	=	TRUE;
			}
			break;

		case AGPDQUEST_SUMMARY_FACTOR_ITEM_COUNT:
			if (!pQuestTemplate->m_CompleteCondition.Monster1.Item.pItemTemplate)
				continue;

			lCount = m_pcsAgpmGrid->GetCountByTemplate(&pcsAgpdItemADChar->m_csInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM,
				pQuestTemplate->m_CompleteCondition.Monster1.Item.pItemTemplate->m_lID);
			if (lCount >= pQuestTemplate->m_CompleteCondition.Monster1.Item.lCount)
			{
				if( !bOnlyIncomplete )
				{
					lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s(%d/%d)</n><H%x>", 
						COLOR_QUEST_COMPLETE, pSummaryText->SummaryText[i].szSummaryText, lCount, 
						pQuestTemplate->m_CompleteCondition.Monster1.Item.lCount, COLOR_QUEST_DEFAULT);

					bEnd	=	TRUE;
				}
			}
			else
			{
				lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s(%d/%d)</n><H%x>", 
					COLOR_QUEST_INCOMPLETE, pSummaryText->SummaryText[i].szSummaryText, lCount, 
					pQuestTemplate->m_CompleteCondition.Monster1.Item.lCount, COLOR_QUEST_DEFAULT);

				bEnd	=	TRUE;
			}
			break;

		case AGPDQUEST_SUMMARY_FACTOR_MONSTER1_COUNT:
			if ((*iter).lParam1 >= pQuestTemplate->m_CompleteCondition.Monster1.lCount)
			{
				if( !bOnlyIncomplete )
				{
					lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s(%d/%d)</n><H%x>", 
						COLOR_QUEST_COMPLETE, pSummaryText->SummaryText[i].szSummaryText,
						(*iter).lParam1, pQuestTemplate->m_CompleteCondition.Monster1.lCount, COLOR_QUEST_DEFAULT);
					bEnd	=	TRUE;
				}
			}
			else
			{
				lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s(%d/%d)</n><H%x>", 
					COLOR_QUEST_INCOMPLETE, pSummaryText->SummaryText[i].szSummaryText,
					(*iter).lParam1, pQuestTemplate->m_CompleteCondition.Monster1.lCount, COLOR_QUEST_DEFAULT);
				bEnd	=	TRUE;
			}
			break;

		case AGPDQUEST_SUMMARY_FACTOR_MONSTER2_COUNT:
			if ((*iter).lParam2 >= pQuestTemplate->m_CompleteCondition.Monster2.lCount)
			{
				if( !bOnlyIncomplete )
				{
					lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s(%d/%d)</n><H%x>", 
						COLOR_QUEST_COMPLETE, pSummaryText->SummaryText[i].szSummaryText,
						(*iter).lParam2, pQuestTemplate->m_CompleteCondition.Monster2.lCount, COLOR_QUEST_DEFAULT);
					bEnd	=	TRUE;
				}
			}
			else
			{
				lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s(%d/%d)</n><H%x>", 
					COLOR_QUEST_INCOMPLETE, pSummaryText->SummaryText[i].szSummaryText,
					(*iter).lParam2, pQuestTemplate->m_CompleteCondition.Monster2.lCount, COLOR_QUEST_DEFAULT);
				bEnd	=	TRUE;
			}
			break;

		case AGPDQUEST_SUMMARY_FACTOR_NORMAL_ITEM:
			ASSERT(pQuestTemplate->m_CompleteCondition.Item.pItemTemplate);
			lCount = m_pcsAgpmGrid->GetCountByTemplate(&pcsAgpdItemADChar->m_csInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM,
				pQuestTemplate->m_CompleteCondition.Item.pItemTemplate->m_lID);

			if (lCount >= pQuestTemplate->m_CompleteCondition.Item.lCount)
			{
				if( !bOnlyIncomplete )
				{
					lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s(%d/%d)</n><H%x>", 
						COLOR_QUEST_COMPLETE, pSummaryText->SummaryText[i].szSummaryText,
						lCount, pQuestTemplate->m_CompleteCondition.Item.lCount, COLOR_QUEST_DEFAULT);
					bEnd	=	TRUE;
				}
			}
			else
			{
				lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s(%d/%d)</n><H%x>", 
					COLOR_QUEST_INCOMPLETE, pSummaryText->SummaryText[i].szSummaryText, 
					lCount, pQuestTemplate->m_CompleteCondition.Item.lCount, COLOR_QUEST_DEFAULT);
				bEnd	=	TRUE;
			}
			break;

		case AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_1:
			lIndex += ComposeCheckPointText(AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_1, &pBuffer[lIndex], pQuestTemplate, pcsAgpdItemADChar, bOnlyIncomplete , &bEnd);
			break;

		case AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_2:
			lIndex += ComposeCheckPointText(AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_2, &pBuffer[lIndex], pQuestTemplate, pcsAgpdItemADChar, bOnlyIncomplete , &bEnd);
			break;

		case AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_3:
			lIndex += ComposeCheckPointText(AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_3, &pBuffer[lIndex], pQuestTemplate, pcsAgpdItemADChar, bOnlyIncomplete , &bEnd);
			break;

		case AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_4:
			lIndex += ComposeCheckPointText(AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_4, &pBuffer[lIndex], pQuestTemplate, pcsAgpdItemADChar, bOnlyIncomplete , &bEnd);
			break;

		case AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_5:
			lIndex += ComposeCheckPointText(AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_5, &pBuffer[lIndex], pQuestTemplate, pcsAgpdItemADChar, bOnlyIncomplete , &bEnd);
			break;

		case AGPDQUEST_SUMMARY_FACTOR_NONE:
			{
				lIndex += sprintf(&pBuffer[lIndex], "%s</n>", pSummaryText->SummaryText[i].szSummaryText);
			}
			break;
		}
		if( bOneSummary && bEnd )
			break;
	}

	return lIndex;
}

INT32 AgcmUIQuest2::ComposeCheckPointText(Enum_AGPDQUEST_SUMMARY_FACTOR eCheckPointFactor, char *pBuffer, AgpdQuestTemplate *pQuestTemplate, AgpdItemADChar* pcsAgpdItemADChar , BOOL bOnlyIncomplete , BOOL *pbOneSummary )
{
	INT32 lIndex = 0;
	INT32 lCheckPointIndex = eCheckPointFactor - AGPDQUEST_SUMMARY_FACTOR_CHECKPOINT_1;

	if (!pQuestTemplate->m_CheckPoint.CheckPointItem[lCheckPointIndex].pItemTemplate)
		return 0;

	ASSERT(pQuestTemplate->m_CheckPoint.CheckPointItem[lCheckPointIndex].pItemTemplate);
	INT32 lCount = m_pcsAgpmGrid->GetCountByTemplate(&pcsAgpdItemADChar->m_csInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM,
		pQuestTemplate->m_CheckPoint.CheckPointItem[lCheckPointIndex].pItemTemplate->m_lID);

	if (lCount >= pQuestTemplate->m_CheckPoint.CheckPointItem[lCheckPointIndex].lItemCount)
	{
		if( !bOnlyIncomplete )
		{
			lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s</n><H%x>", 
				COLOR_QUEST_COMPLETE, pQuestTemplate->m_SummaryText.SummaryText[lCheckPointIndex].szSummaryText,COLOR_QUEST_DEFAULT);

			if( pbOneSummary )
				*pbOneSummary	=	TRUE;
		}
	}
	else
	{
		lIndex += sprintf(&pBuffer[lIndex], "<H%x>%s</n><H%x>", 
			COLOR_QUEST_INCOMPLETE, pQuestTemplate->m_SummaryText.SummaryText[lCheckPointIndex].szSummaryText, COLOR_QUEST_DEFAULT);

		if( pbOneSummary )
			*pbOneSummary	=	TRUE;
	}

	return lIndex;
}

INT32 AgcmUIQuest2::ComposeAcceptSummaryText(CHAR* pBuffer, INT32 lBufferSize, AgpdQuestTemplate *pQuestTemplate)
{
	INT32 lIndex = 0;
	AgpdQuestSummaryText *pSummaryText = &pQuestTemplate->m_SummaryText;
	
	for (INT32 i = 0; i < AGPDQUEST_MAX_SUMMARY_TEXT_COUNT; ++i)
	{
		if (0 == strlen(pSummaryText->SummaryText[i].szSummaryText)) 
			continue;
		
		lIndex += sprintf(&pBuffer[lIndex], "%s</n>", pSummaryText->SummaryText[i].szSummaryText);
	}

	return lIndex;
}

BOOL AgcmUIQuest2::CBReturnCancelQuest(PVOID pClass, INT32 lTrueCancel, CHAR* szInputMessage)
{
	if (!pClass)
		return FALSE;

	AgcmUIQuest2* pThis = (AgcmUIQuest2*)pClass;
	if (lTrueCancel)
	{
		AgpdCharacter* pcsAgpdCharacter = pThis->m_pcsAgcmCharacter->GetSelfCharacter();
		if (!pcsAgpdCharacter)
			return FALSE;

		pThis->m_pcsAgcmQuest->SendPacketQuestCancel(pThis->m_lMainQuestIDList[pThis->m_lClickIDMainUI], pcsAgpdCharacter->m_lID);
	}

	return TRUE;
}

BOOL AgcmUIQuest2::CBCancelDialogOpen(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;

	//if ( (pThis->m_eMainQuestIDStatus[pThis->m_lClickIDMainUI] != MAINUI_CURRENT) || 
	//	 (pThis->m_eMainQuestIDStatus[pThis->m_lClickIDMainUI] != MAINUI_COMPLETE) )
	//{
	//	SystemMessage.ProcessSystemMessage("현재 진행중인 퀘스트가 아닙니다.");
	//	return FALSE;
	//}

	if (pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pThis->m_lMainQuestIDList[pThis->m_lClickIDMainUI]))
	{
		pThis->m_lMessageBoxQuestTID = pThis->m_lMainQuestIDList[pThis->m_lClickIDMainUI];
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventQuestCancel);
	}

	return TRUE;	
}

BOOL AgcmUIQuest2::CBMainUIHotKeyOpen(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	pThis->m_lClickIDAcceptUI = INVALID_INDEX;
//	pThis->m_lClickIDMainUI = INVALID_INDEX;
	pThis->RefreshMainTree();
	pThis->RefreshMainText();
	pThis->RefreshMainCount();
	
	return TRUE;
}

BOOL	AgcmUIQuest2::CBCheckJournal( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase* pcsTarget , AgcdUIControl* pcsSourceControl )
{
	AgcmUIQuest2*	pThis	=	static_cast< AgcmUIQuest2* >(pClass);
	if( !pThis )
		return FALSE;

	INT		nQuestID	=	0;

	if( !pcsSourceControl || !pcsSourceControl->m_pcsBase )
		return FALSE;

	nQuestID	=	pThis->m_lMainQuestIDList[ pcsSourceControl->m_lUserDataIndex ];

	AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(nQuestID);
	if (!pQuestTemplate) 
		return FALSE;

	AgpdCharacter*		pcsSelfCharacter	=	pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if( !pcsSelfCharacter )
		return FALSE;

	if( pThis->m_pcsAgcmQuestJournal->IsQuestJournal( nQuestID) )
	{
		pThis->m_pcsAgcmQuestJournal->DelQuestJournal( nQuestID , TRUE );

	}
	else
	{
		eJournalType	eJournal	=	QUEST_JOURNAL_NORMAL;

		if( pQuestTemplate->m_QuestInfo.eQuestCategory == AGPDQUEST_SYSTEM_TYPE_EPIC )
			eJournal	=	QUEST_JOURNAL_MISSION;

		if( pThis->m_pcsAgcmQuestJournal->AddQuestJournal( pQuestTemplate->m_QuestInfo.szName , eJournal , nQuestID) )
			pThis->m_pcsAgcmQuest->SendPacketQuestJournalRegisterChange( nQuestID , pcsSelfCharacter->GetID() , TRUE );
	}

	pThis->RefreshMainTree();

	return TRUE;
}

BOOL AgcmUIQuest2::CBClickJournalClose( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase* pcsTarget , AgcdUIControl* pcsSourceControl )
{
	// 저널을 모두 닫는다
	AgcmUIQuest2*	pThis	=	static_cast< AgcmUIQuest2* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->m_pcsAgcmQuestJournal->Clear( TRUE );
	pThis->RefreshMainTree();

	return TRUE;
}

BOOL AgcmUIQuest2::CBClickMapOpen( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase* pcsTarget , AgcdUIControl* pcsSourceControl )
{
	// 맵 오픈
	AgcmUIQuest2*	pThis	=	static_cast< AgcmUIQuest2* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->OpenWorldMap();

	return TRUE;
}

BOOL AgcmUIQuest2::CBClickQuestDel( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase* pcsTarget , AgcdUIControl* pcsSourceControl )
{
	// 퀘스트 삭제
	AgcmUIQuest2*	pThis	=	static_cast< AgcmUIQuest2* >(pClass);
	if( !pThis )
		return FALSE;

	if( pThis->m_lClickIDMainUI == INVALID_INDEX )
		return FALSE;

	AgpdCharacter*	pcsSelfCharacter	=	pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if( !pcsSelfCharacter )
		return FALSE;

	AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pThis->m_lMainQuestIDList[pThis->m_lClickIDMainUI]);
	if (!pQuestTemplate || !pThis->m_lMainQuestTreeCount)
		return FALSE;

	CHAR*	pUIMessage	=	pThis->m_pcsAgcmUIManager2->GetUIMessage( "QuestDeleteMessage" );
	if( pUIMessage)
	{
		CHAR	szMessage[ MAX_PATH*2 ];
		sprintf_s( szMessage , MAX_PATH*2 , pUIMessage  , pQuestTemplate->m_QuestInfo.szName );

		if( pThis->m_pcsAgcmUIManager2->ActionMessageOKCancelDialog( szMessage ) )
		{
			pThis->m_pcsAgcmQuest->SendPacketQuestCancel( pQuestTemplate->GetID() , pcsSelfCharacter->GetID() );
		}
	}

	return TRUE;
}

BOOL AgcmUIQuest2::CBButtonQuestMainListClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;

	if (pThis->m_lMainQuestTreeCount < pcsSourceControl->m_lUserDataIndex) return FALSE;

	pThis->m_lClickIDMainUI = pcsSourceControl->m_lUserDataIndex;
	return pThis->RefreshMainText();

	return TRUE;
}

BOOL AgcmUIQuest2::MakeAcceptList(AgcdUIQuestList *pAcceptList, INT32 *pListCount, ApdEvent *pApdEvent)
{
	if (!pApdEvent)
	{
		return MakeAcceptListAutoStart(pAcceptList, pListCount);
	}

	*pListCount = 0;

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsCharacter)
		return FALSE;
	
	AgpdQuestEventAttachData* pcsAttachData =  (AgpdQuestEventAttachData*)pApdEvent->m_pvData;
	if (!pcsAttachData)
		return FALSE;

	AgpdQuestGroup* pQuestGroup = m_pcsAgpmQuest->m_csQuestGroup.Get(pcsAttachData->lQuestGroupID);
	if (!pQuestGroup)
		return FALSE;

	AgpdQuest* pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsCharacter);
	if (!pAgpdQuest)
		return FALSE;

	list<INT32>::iterator iter;
	AgpdQuest::IterQuest iterQ;
	INT32 lIndex = 0;

	// 현재 캐릭터가 수행할수 있는 퀘스트 목록
	AgpdQuestTemplate* pQuestTemplate;
	for (iter = pQuestGroup->m_GrantQuest.begin(); iter != pQuestGroup->m_GrantQuest.end(); ++iter)
	{
		pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(*iter);
		if (!pQuestTemplate)
			continue;

		// 이미 완료한 퀘스트 이면 넘어간다.
		if ( pQuestTemplate->m_eQuestType != AGPDQUEST_TYPE_REPEAT &&
			pAgpdQuest->m_csFlag.GetValueByDBID(pQuestTemplate->m_lID))
			continue;

		if (*pListCount >= ACCEPTLIST_MAX_COUNT)
			break;

		if (m_pcsAgpmQuest->EvaluationStartCondition(pcsCharacter, *iter))
		{
			pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_NEW;
			pAcceptList[*pListCount].lQuestTID = *iter;

			iterQ = pAgpdQuest->FindByQuestID(*iter);

			if (iterQ != pAgpdQuest->m_vtQuest.end())
				pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_INCOMPLETE;	// 가지고 있는 퀘스트
			else
				pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_NEW;	// 가지고 있지 않은 퀘스트

			++(*pListCount);
		}
	}

	// 캐릭터가 진행중인 퀘스트 중에서 NPC가 종료할수 있는 퀘스트 인지 확인
	for (iter = pQuestGroup->m_EstimateQuest.begin(); iter != pQuestGroup->m_EstimateQuest.end(); ++iter)
	{
		if (*pListCount >= ACCEPTLIST_MAX_COUNT)
			break;

		iterQ = pAgpdQuest->FindByQuestID(*iter);
		if (iterQ != pAgpdQuest->m_vtQuest.end())
		{
			if (m_pcsAgpmQuest->EvaluationCompleteCondition(pcsCharacter, *iter))
			{
				BOOL bFind = FALSE;
				INT32 lIndex = -1;

				// 완료 가능한 퀘스트 라면 rVector에 있는지 확인하고 있다면 업데이트만 한다.
				for (INT32 i = 0; i < *pListCount; ++i)
				{
					if (*iter == pAcceptList[i].lQuestTID)
					{
						bFind = TRUE;
						lIndex = i;
					}
				}

				if (bFind)
				{
					pAcceptList[lIndex].eStatus = ACCEPTUI_STATUS_COMPLETE;
				}
				else
				{
					// 완료 가능한 퀘스트가 rVector에 없다면 새로 추가
					pAcceptList[*pListCount].lQuestTID = *iter;
					pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_COMPLETE;
					++(*pListCount);
				}
			}
			else
			{
				BOOL bFind = FALSE;
				INT32 lIndex = -1;

				// 완료 가능한 퀘스트 라면 rVector에 있는지 확인하고 있다면 업데이트만 한다.
				for (INT32 i = 0; i < *pListCount; ++i)
				{
					if (*iter == pAcceptList[i].lQuestTID)
					{
						bFind = TRUE;
						lIndex = i;
					}
				}

				if (bFind)
				{
					pAcceptList[lIndex].eStatus = ACCEPTUI_STATUS_INCOMPLETE_GRANT;
				}
				else
				{
					// 완료 가능한 퀘스트가 rVector에 없다면 새로 추가
					pAcceptList[*pListCount].lQuestTID = *iter;
					pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_INCOMPLETE_GRANT;
					++(*pListCount);
				}				
			}
		}
	}

	// 상태별로 정렬 (완료, 진행, 대기 순으로)
	INT32 lChangeIndex;
	EnumAcceptUIStatus eStatus;
	for (INT32 lIndex = 0; lIndex < *pListCount; ++lIndex)
	{
		eStatus = ACCEPTUI_STATUS_NONE;
		for (INT32 lSubIndex = lIndex; lSubIndex < *pListCount; ++lSubIndex)
		{
			if (pAcceptList[lSubIndex].eStatus > eStatus)
			{
				eStatus = pAcceptList[lSubIndex].eStatus;
				lChangeIndex = lSubIndex;
			}
		}

		if (ACCEPTUI_STATUS_NONE != eStatus)
		{
			AgcdUIQuestList stTemp = pAcceptList[lIndex];
			pAcceptList[lIndex] = pAcceptList[lChangeIndex];
			pAcceptList[lChangeIndex] = stTemp;
		}

	}

	return TRUE;
}

BOOL AgcmUIQuest2::MakeAcceptListAutoStart(AgcdUIQuestList *pAcceptList, INT32 *pListCount)
{
	*pListCount = 0;

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsCharacter)
		return FALSE;

	AgpdQuest* pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsCharacter);
	if (!pAgpdQuest)
		return FALSE;

	INT32				lIndex;
	AgpdQuestTemplate *	pQuestTemplate;
	AgpdQuest::IterQuest iterQ;

	lIndex = 0;
	for (pQuestTemplate = m_pcsAgpmQuest->GetTemplateSequence(&lIndex); pQuestTemplate; pQuestTemplate = m_pcsAgpmQuest->GetTemplateSequence(&lIndex))
	{
		if (pQuestTemplate->m_eQuestType != AGPDQUEST_TYPE_AUTOSTART)
			continue;

		// 이미 완료한 퀘스트 이면 넘어간다.
		if (pAgpdQuest->m_csFlag.GetValueByDBID(pQuestTemplate->m_lID))
			continue;

		if (*pListCount >= ACCEPTLIST_MAX_COUNT)
			break;

		if (m_pcsAgpmQuest->EvaluationStartCondition(pcsCharacter, pQuestTemplate->m_lID))
		{
			pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_NEW;
			pAcceptList[*pListCount].lQuestTID = pQuestTemplate->m_lID;

			iterQ = pAgpdQuest->FindByQuestID(pQuestTemplate->m_lID);

			if (iterQ != pAgpdQuest->m_vtQuest.end())
				pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_INCOMPLETE;	// 가지고 있는 퀘스트
			else
				pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_NEW;	// 가지고 있지 않은 퀘스트

			++(*pListCount);
		}

		iterQ = pAgpdQuest->FindByQuestID(pQuestTemplate->m_lID);
		if (iterQ != pAgpdQuest->m_vtQuest.end())
		{
			if (m_pcsAgpmQuest->EvaluationCompleteCondition(pcsCharacter, pQuestTemplate->m_lID))
			{
				BOOL bFind = FALSE;
				INT32 lIndex = -1;

				// 완료 가능한 퀘스트 라면 rVector에 있는지 확인하고 있다면 업데이트만 한다.
				for (INT32 i = 0; i < *pListCount; ++i)
				{
					if (pQuestTemplate->m_lID == pAcceptList[i].lQuestTID)
					{
						bFind = TRUE;
						lIndex = i;
					}
				}

				if (bFind)
				{
					pAcceptList[lIndex].eStatus = ACCEPTUI_STATUS_COMPLETE;
				}
				else
				{
					// 완료 가능한 퀘스트가 rVector에 없다면 새로 추가
					pAcceptList[*pListCount].lQuestTID = pQuestTemplate->m_lID;
					pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_COMPLETE;
					++(*pListCount);
				}
			}
			else
			{
				BOOL bFind = FALSE;
				INT32 lIndex = -1;

				// 완료 가능한 퀘스트 라면 rVector에 있는지 확인하고 있다면 업데이트만 한다.
				for (INT32 i = 0; i < *pListCount; ++i)
				{
					if (pQuestTemplate->m_lID == pAcceptList[i].lQuestTID)
					{
						bFind = TRUE;
						lIndex = i;
					}
				}

				if (bFind)
				{
					pAcceptList[lIndex].eStatus = ACCEPTUI_STATUS_INCOMPLETE_GRANT;
				}
				else
				{
					// 완료 가능한 퀘스트가 rVector에 없다면 새로 추가
					pAcceptList[*pListCount].lQuestTID = pQuestTemplate->m_lID;
					pAcceptList[*pListCount].eStatus = ACCEPTUI_STATUS_INCOMPLETE_GRANT;
					++(*pListCount);
				}				
			}
		}
	}

	// 상태별로 정렬 (완료, 진행, 대기 순으로)
	INT32 lChangeIndex;
	EnumAcceptUIStatus eStatus;
	for (lIndex = 0; lIndex < *pListCount; ++lIndex)
	{
		eStatus = ACCEPTUI_STATUS_NONE;
		for (INT32 lSubIndex = lIndex; lSubIndex < *pListCount; ++lSubIndex)
		{
			if (pAcceptList[lSubIndex].eStatus > eStatus)
			{
				eStatus = pAcceptList[lSubIndex].eStatus;
				lChangeIndex = lSubIndex;
			}
		}

		if (ACCEPTUI_STATUS_NONE != eStatus)
		{
			AgcdUIQuestList stTemp = pAcceptList[lIndex];
			pAcceptList[lIndex] = pAcceptList[lChangeIndex];
			pAcceptList[lChangeIndex] = stTemp;
		}
	}

	if (m_lNumAutoQuest != -1 && m_lNumAutoQuest != *pListCount)
	{
		m_pcsAgcmUIManager2->ThrowEvent(m_lEventQuestAutoStart);
		m_ulAlertEndTime	= GetTickCount() + AGCMUIQUEST_AUTOQUEST_ALERT_DURATION;
		m_lNumAutoQuest		= *pListCount;
	}

	return TRUE;
}

BOOL AgcmUIQuest2::CheckPointQuest(ApdEvent* pcsEvent)
{
	if (m_bHold) 
		return FALSE;

	AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsCharacter)
		return FALSE;

	AgpdItemADChar* pcsAgpdItemADChar = m_pcsAgpmQuest->m_pcsAgpmItem->GetADCharacter(pcsCharacter);
	if (!pcsAgpdItemADChar) 
		return FALSE;
	
	AgpdQuestEventAttachData* pcsAttachData =  (AgpdQuestEventAttachData*)pcsEvent->m_pvData;
	if (!pcsAttachData)
		return FALSE;

	AgpdQuestGroup* pQuestGroup = m_pcsAgpmQuest->m_csQuestGroup.Get(pcsAttachData->lQuestGroupID);
	if (!pQuestGroup)
		return FALSE;

	AgpdQuest* pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsCharacter);
	if (!pAgpdQuest)
		return FALSE;

	if (m_pcsAgpmCharacter->IsRideOn(pcsCharacter))
	{
		// 말을 타고 있는 상태이면 오브젝트 퀘스트를 진행할 수 없다.
		AgcChatManager::OnAddSystemMessage( ClientStr().GetStr(STI_CANT_WITH_HORSE) );
		return FALSE;
	}

	list< AgpdQuestCheckPoint >::iterator iter = pQuestGroup->m_CheckPoint.begin();
	
	while (iter != pQuestGroup->m_CheckPoint.end())
	{
		AgpdQuest::IterQuest iterQuest = pAgpdQuest->FindByQuestID(iter->lQuestTID);
		if (iterQuest != pAgpdQuest->m_vtQuest.end())
		{
			// 현재의 체크포인트를 실행할수 있는 퀘스트를 받은 상태다.
			if (m_pcsAgcmUICooldown && AGCMUICOOLDOWN_INVALID_ID != m_lCooldownID)
			{
				// 체크포인트 퀘스트를 시작하기 이전에 필요 아이템을 가지고 있다면 애니메이션을 진행할 필요가 없다.
				AgpdQuestTemplate *pcsQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(iter->lQuestTID);
				if (!pcsQuestTemplate)
					return FALSE;

				if (!pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].pItemTemplate)
					return FALSE;

				if (m_pcsAgpmGrid->CheckEnoughItem(&pcsAgpdItemADChar->m_csInventoryGrid, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].pItemTemplate->m_lID, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].lItemCount)||
					m_pcsAgpmGrid->CheckEnoughItem(&pcsAgpdItemADChar->m_csBankGrid, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].pItemTemplate->m_lID, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].lItemCount)||
					m_pcsAgpmGrid->CheckEnoughItem(&pcsAgpdItemADChar->m_csCashInventoryGrid, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].pItemTemplate->m_lID, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].lItemCount)||
					m_pcsAgpmGrid->CheckEnoughItem(&pcsAgpdItemADChar->m_csSubInventoryGrid, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].pItemTemplate->m_lID, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].lItemCount)||
					m_pcsAgpmGrid->CheckEnoughItem(&pcsAgpdItemADChar->m_csChargingInventoryGrid, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].pItemTemplate->m_lID, pcsQuestTemplate->m_CheckPoint.CheckPointItem[iter->lIndex].lItemCount))
				{
					AgcChatManager::OnAddSystemMessage(ClientStr().GetStr(STI_NO_MORE_ITEM));
				}
				else
				{
					// Cooldown이 성공해야 Animation도 실행한다.
					if (m_pcsAgcmUICooldown->StartCooldown(m_lCooldownID, 5000, ClientStr().GetStr(STI_QUEST), TRUE, NULL, AGCMUICOOLDOWN_EVENT_SOUND_QUSET))
					{
						m_pCheckPointEvent = pcsEvent;
						m_CheckPointData.lQuestTID = iter->lQuestTID;
						m_CheckPointData.lIndex = iter->lIndex;
						m_pcsAgpmCharacter->StopCharacter(pcsCharacter, NULL);
						StartAnimation(pcsCharacter, pcsEvent);
						m_bHold = TRUE;
						return TRUE;
					}
				}
			}

			break;
		}

		iter++;
	}

	return FALSE;
}

BOOL AgcmUIQuest2::StartAnimation(AgpdCharacter* pcsAgpdCharacter, ApdEvent* pApdEvent)
{
	AgcdCharacter *pcsAgcdCharacter								= m_pcsAgcmCharacter->GetCharacterData(pcsAgpdCharacter);
	AgpdSkillTemplate *pcsAgpdSkillTemplate						= m_pcsAgpmSkill->GetSkillTemplate("연금술");
	if (!pcsAgpdSkillTemplate)
		return FALSE;

	AgcdSkillAttachTemplateData	*pstAgcdSkillAttachTemplateData	= m_pcsAgcmSkill->GetAttachTemplateData((ApBase *)(pcsAgpdCharacter->m_pcsCharacterTemplate));
	if (!pstAgcdSkillAttachTemplateData)
		return FALSE;

	INT32 lUsableIndex	= m_pcsAgcmSkill->GetUsableSkillTNameIndex((ApBase *)(pcsAgpdCharacter->m_pcsCharacterTemplate), pcsAgpdSkillTemplate->m_lID);
	if (0 > lUsableIndex)
		return FALSE;

	m_pcsAgcmSkill->SetCurrentSkillUsableIndex(pcsAgpdCharacter, lUsableIndex);

	AuPOS tempPos;
	m_pcsAgpmCharacter->TurnCharacter(pcsAgpdCharacter, 0, 
					m_pcsAgpmCharacter->GetSelfCharacterTurnAngle(pcsAgpdCharacter, m_pcsApmEventManager->GetBasePos(pApdEvent, &tempPos)));

	FLOAT fAnimDuration;

	if (pstAgcdSkillAttachTemplateData->m_pacsSkillVisualInfo[lUsableIndex]->m_pacsAnimation[pcsAgcdCharacter->m_lCurAnimType2])
	{
		fAnimDuration = m_pcsAgcmCharacter->GetAnimDuration(pstAgcdSkillAttachTemplateData->m_pacsSkillVisualInfo[lUsableIndex]->m_pacsAnimation[pcsAgcdCharacter->m_lCurAnimType2]->m_pcsAnimation);
		m_pcsAgcmCharacter->StartAnimation(pcsAgpdCharacter, AGCMCHAR_ANIM_TYPE_SKILL, 
						pstAgcdSkillAttachTemplateData->m_pacsSkillVisualInfo[lUsableIndex]->m_pacsAnimation[pcsAgcdCharacter->m_lCurAnimType2]);
	}
	else if (pstAgcdSkillAttachTemplateData->m_pacsSkillVisualInfo[lUsableIndex]->m_pacsAnimation[AGCMCHAR_AT2_COMMON_DEFAULT])
	{
		fAnimDuration = m_pcsAgcmCharacter->GetAnimDuration(pstAgcdSkillAttachTemplateData->m_pacsSkillVisualInfo[lUsableIndex]->m_pacsAnimation[AGCMCHAR_AT2_COMMON_DEFAULT]->m_pcsAnimation);
		m_pcsAgcmCharacter->StartAnimation(pcsAgpdCharacter, AGCMCHAR_ANIM_TYPE_SKILL, 
						pstAgcdSkillAttachTemplateData->m_pacsSkillVisualInfo[lUsableIndex]->m_pacsAnimation[AGCMCHAR_AT2_COMMON_DEFAULT]);
	}

	m_pcsAgpmCharacter->SetActionBlockTime(pcsAgpdCharacter, (UINT32) (fAnimDuration * 1000), AGPDCHAR_ACTION_BLOCK_TYPE_SKILL);
	
	return TRUE;
}

BOOL AgcmUIQuest2::CBQuestGrant(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	AgpdCharacter *pAgpdCharacter = (AgpdCharacter*)pData;
	ApdEvent* pcsEvent = (ApdEvent*)pCustData;
	
	// CheckPoint기능이 있다면 그 기능을 우선적으로 처리한다.
	if (pThis->CheckPointQuest(pcsEvent))
		return TRUE;

	pThis->m_lClickIDAcceptUI = INVALID_INDEX;
	pThis->MakeAcceptList(&pThis->m_AcceptList[0], &pThis->m_lAcceptListCount, pThis->m_pcsAgcmEventQuest->m_pcsLastGrantEvent);
	
	switch (pThis->m_lAcceptListCount)
	{
	case 0:
		// 오브젝트가 아닌 대상에서만 진행 가능한 퀘스트가 없다는 메세지를 보여준다.
		if (APBASE_TYPE_OBJECT != pcsEvent->m_pcsSource->m_eType)
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lMessageQuestDontHave);
		break;

	default:
		// 퀘스트가 하나 이상이면 Accept UI를 보여준다.
		pThis->RefreshAcceptList();
		pThis->RefreshAcceptText();
		pThis->RefreshAcceptListAutoStart();
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventAcceptUIOpen);
		break;
	};

	pThis->m_stOpenPos = pAgpdCharacter->m_stPos;
	pThis->m_bIsQuestUIOpen	= TRUE;

	return TRUE;
}

BOOL AgcmUIQuest2::CBQuestAddEvent(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUIQuest2* pThis = (AgcmUIQuest2*)pClass;
	ApdEvent*	pApdEvent = (ApdEvent*)pData;
	ApBase*		pApBase = (ApBase*)pCustData;

	return pThis->SetCharacterEffect(pApBase, pApdEvent);
}

BOOL AgcmUIQuest2::SetCharacterEffect(ApBase* pApBase, ApdEvent* pApdEvent)
{
	if (APDEVENT_FUNCTION_QUEST == pApdEvent->m_eFunction)
	{
		// NPC가 퀘스트를 가지고 있다면 스크롤을 띄울지 여부를 판정
		ApSafeArray<AgcdUIQuestList, ACCEPTLIST_MAX_COUNT> TempList;
		INT32 lTempListCount = 0;

		MakeAcceptList(&TempList[0], &lTempListCount, pApdEvent);

		if (m_pcsAgcmEventEffect)
		{
			m_pcsAgcmEventEffect->RemoveCommonCharacterEffect(pApBase, E_COMMON_CHAR_TYPE_QUEST_NEW);
			m_pcsAgcmEventEffect->RemoveCommonCharacterEffect(pApBase, E_COMMON_CHAR_TYPE_QUEST_COMPLETE);
			m_pcsAgcmEventEffect->RemoveCommonCharacterEffect(pApBase, E_COMMON_CHAR_TYPE_QUEST_INCOMPLETE);

			m_pcsAgcmEventEffect->RemoveCommonCharacterEffect(pApBase, E_COMMON_CHAR_TYPE_GIFT_BOX);
			m_pcsAgcmEventEffect->RemoveCommonCharacterEffect(pApBase, E_COMMON_CHAR_TYPE_GIFT_FIRE);

			m_pcsAgcmUICharacter->RemoveBaseMinimapType(pApBase);
		}

		// 미완료 퀘스트가 있다면 표시해준다.
		for (INT32 i = 0; i < lTempListCount; ++i)
		{
			if (ACCEPTUI_STATUS_COMPLETE == TempList[i].eStatus)
			{
				if (m_pcsAgcmEventEffect)
					m_pcsAgcmEventEffect->SetCommonCharacterEffect(pApBase, E_COMMON_CHAR_TYPE_QUEST_COMPLETE);

				if (m_pcsAgcmUICharacter)
				{
					// 마고자 2005/08/12
					// 미니맵 포인트를 제거한후 사입하게 변경
					// 기존으론 이벤트가 없는경우 미니밉에 표시가 돼지 않았음.
					m_pcsAgcmUICharacter->RemoveBaseMinimapType(pApBase, TempList[i].lQuestTID);

					if (APBASE_TYPE_OBJECT == pApBase->m_eType)
					{
						AgpdQuestTemplate* pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(TempList[i].lQuestTID);
						if (!pQuestTemplate)
							continue;

						m_pcsAgcmUICharacter->AddBaseMinimapType(pApBase, AgcmMinimap::MPInfo::MPI_QUESTINCOMPLETE, pQuestTemplate->m_QuestInfo.szName, TempList[i].lQuestTID);
					}
					else
						m_pcsAgcmUICharacter->AddBaseMinimapType(pApBase, AgcmMinimap::MPInfo::MPI_QUESTINCOMPLETE, NULL, TempList[i].lQuestTID);
				}

				return TRUE;
			}
			else if (ACCEPTUI_STATUS_INCOMPLETE_GRANT == TempList[i].eStatus)
			{
				if (m_pcsAgcmEventEffect)
					m_pcsAgcmEventEffect->SetCommonCharacterEffect(pApBase, E_COMMON_CHAR_TYPE_QUEST_INCOMPLETE);

				if (m_pcsAgcmUICharacter)
				{
					// 마고자 2005/08/12
					// 미니맵 포인트를 제거한후 사입하게 변경
					// 기존으론 이벤트가 없는경우 미니밉에 표시가 돼지 않았음.
					m_pcsAgcmUICharacter->RemoveBaseMinimapType(pApBase, TempList[i].lQuestTID);
					m_pcsAgcmUICharacter->AddBaseMinimapType(pApBase, AgcmMinimap::MPInfo::MPI_QUESTINCOMPLETE, NULL, TempList[i].lQuestTID);
				}

				return TRUE;
			}
		}

		// 대기 퀘스트가 있다면 표시해준다.
		INT32 i;
		for (i = 0; i < lTempListCount; ++i)
		{
			if (ACCEPTUI_STATUS_NEW == TempList[i].eStatus)
			{
				if (m_pcsAgcmEventEffect)
					m_pcsAgcmEventEffect->SetCommonCharacterEffect(pApBase, E_COMMON_CHAR_TYPE_QUEST_NEW);

				if (m_pcsAgcmUICharacter)
				{
					// 마고자 2005/08/12
					// 미니맵 포인트를 제거한후 사입하게 변경
					// 기존으론 이벤트가 없는경우 미니밉에 표시가 돼지 않았음.
					m_pcsAgcmUICharacter->RemoveBaseMinimapType(pApBase, TempList[i].lQuestTID);
					m_pcsAgcmUICharacter->AddBaseMinimapType(pApBase, AgcmMinimap::MPInfo::MPI_QUESTNEW, NULL, TempList[i].lQuestTID);
				}

				return TRUE;
			}
		}

		// 없으면 다시 추가한다.

		AgpdCharacter *pcsCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
		
		AgpdQuestEventAttachData* pcsAttachData =  (AgpdQuestEventAttachData*)pApdEvent->m_pvData;

		AgpdQuestGroup* pQuestGroup = pcsAttachData ? m_pcsAgpmQuest->m_csQuestGroup.Get(pcsAttachData->lQuestGroupID) : NULL ;

		AgpdQuest* pAgpdQuest = pcsCharacter ? m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsCharacter) : NULL;
		if (!pAgpdQuest		||
			!pcsCharacter	||
			!pcsAttachData	||
			!pQuestGroup	)
		{
			// pQuestGroup이 NULL 인경우가 있따.
			// 이게 맞는건지 틀린건지 모르겠는데 그것때문에
			// 미니맵/월드맵에서 NPC의 위치가 표시가 돼고 있지 않는 경우가 있따.
			if (m_pcsAgcmUICharacter)
				if (pApBase->m_eType == APBASE_TYPE_CHARACTER && m_pcsAgpmCharacter->IsNPC((AgpdCharacter *) pApBase))
					m_pcsAgcmUICharacter->AddBaseMinimapType(pApBase, AgcmMinimap::MPInfo::MPI_NPC);
			return FALSE;
		}

		for (list< AgpdQuestCheckPoint >::iterator iter = pQuestGroup->m_CheckPoint.begin(); iter != pQuestGroup->m_CheckPoint.end(); iter++)
		{
			AgpdQuest::IterQuest iter2 = pAgpdQuest->FindByQuestID(iter->lQuestTID);
			if (iter2 != pAgpdQuest->m_vtQuest.end())
			{
				if (m_pcsAgcmEventEffect)
					m_pcsAgcmEventEffect->SetCommonCharacterEffect(pApBase, E_COMMON_CHAR_TYPE_QUEST_INCOMPLETE);

				if (m_pcsAgcmUICharacter)
				{
					if (APBASE_TYPE_OBJECT == pApBase->m_eType)
					{
						AgpdQuestTemplate* pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(iter->lQuestTID);
						if (!pQuestTemplate)
							continue;

						m_pcsAgcmUICharacter->AddBaseMinimapType(pApBase, AgcmMinimap::MPInfo::MPI_QUESTINCOMPLETE, pQuestTemplate->m_QuestInfo.szName, TempList[i].lQuestTID);
					}
					else
					{
						m_pcsAgcmUICharacter->AddBaseMinimapType(pApBase, AgcmMinimap::MPInfo::MPI_QUESTINCOMPLETE, NULL, TempList[i].lQuestTID);
					}
				}
					
			}
		}

		if (m_pcsAgcmUICharacter)
			if (pApBase->m_eType == APBASE_TYPE_CHARACTER && m_pcsAgpmCharacter->IsNPC((AgpdCharacter *) pApBase))
				m_pcsAgcmUICharacter->AddBaseMinimapType(pApBase, AgcmMinimap::MPInfo::MPI_NPC);
	}

	return TRUE;
}

BOOL AgcmUIQuest2::CBInitBase(PVOID pData, PVOID pClass, PVOID pCustData)
{
	AgcmUIQuest2		*pThis				= (AgcmUIQuest2 *)	pClass;
	ApBase				*pcsBase			= (ApBase *)	pData;
	ApdEvent			*pcsEvent;

	pcsEvent = pThis->m_pcsApmEventManager->GetEvent(pcsBase, APDEVENT_FUNCTION_QUEST);

	if (pcsEvent)
	{
		pThis->SetCharacterEffect(pcsBase, pcsEvent);
	}

	return TRUE;
}

BOOL AgcmUIQuest2::CBSelfUpdatePosition(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUIQuest2		*pThis				= (AgcmUIQuest2 *)	pClass;
	AgpdCharacter		*pcsCharacter		= (AgpdCharacter *)	pData;

	if (!pThis->m_bIsQuestUIOpen)
		return TRUE;

	FLOAT	fDistance = AUPOS_DISTANCE_XZ(pcsCharacter->m_stPos, pThis->m_stOpenPos);

	if ((INT32) fDistance < AGCMUIEVENTQUEST_CLOSE_UI_DISTANCE)
		return TRUE;

	// Accept 윈도우를 닫는다.
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventAcceptUIClose);

	pThis->m_bIsQuestUIOpen	= FALSE;

	return TRUE;
}

BOOL AgcmUIQuest2::CBRequireQuestResult(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	CBQuestParam *pParam = (CBQuestParam*)pData;

	INT32 lQuestID, lParam1, lParam2, lItemTID;
	CHAR* szNPCName = NULL;
	BOOL	bRegisterJournal = FALSE;

	pThis->m_pcsAgpmQuest->GetFieldEmbeddedPacket(pParam->pvPacket, lQuestID, lParam1, lParam2, lItemTID, szNPCName, bRegisterJournal);

	return pThis->RequireQuestResult(pParam->pcsAgpdCharacter, pParam->lQuestTID, pParam->bResult, szNPCName);
}

BOOL AgcmUIQuest2::RequireQuestResult(AgpdCharacter *pcsAgpdCharacter, INT32 lQuestTID, BOOL bResult, CHAR* szNPCName)
{
	if (!bResult) return FALSE;

	AgpdQuest *pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsAgpdCharacter);
	if (!pAgpdQuest)
		return FALSE;

	pAgpdQuest->AddNewQuest(lQuestTID);

	AgpdQuestTemplate *pTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestTID);
	if (!pTemplate)
		return FALSE;

	if( m_pcsAgcmQuestJournal->AddQuestJournal( pTemplate->m_QuestInfo.szName , QUEST_JOURNAL_NORMAL , lQuestTID) )
		m_pcsAgcmQuest->SendPacketQuestJournalRegisterChange( lQuestTID , pcsAgpdCharacter->GetID() , TRUE );

	RefreshMainTree();
	RefreshMainCount();

	CHAR szBuffer[128] = {0,};
	INT32 lSize = sprintf(szBuffer, ClientStr().GetStr(STI_TAKE_QUEST), pTemplate->m_QuestInfo.szName);
	ASSERT(lSize < 128);
	AgcChatManager::OnAddSystemMessage(szBuffer);

	// MessageBox용 Quest TID
	m_lMessageBoxQuestTID = lQuestTID;
	m_pcsAgcmUIManager2->ThrowEvent(m_lMessageQuestStart);
	m_pcsAgcmUIManager2->ThrowEvent(m_lSoundQuestStart);

	m_lClickIDAcceptUI = 0;
	MakeAcceptList(&m_AcceptList[0], &m_lAcceptListCount, m_pcsAgcmEventQuest->m_pcsLastGrantEvent);

	if (!m_lAcceptListCount)
		m_pcsAgcmUIManager2->ThrowEvent(m_lEventAcceptUIClose);

	RefreshAcceptList();
	RefreshAcceptListAutoStart();

	RefreshBaseEffect();
	RefreshAcceptLeftButtonEnable(FALSE);
	
	return FALSE;	
}

BOOL AgcmUIQuest2::RefreshBaseEffect()
{
	INT32			lIndex	= 0;
	AgpdCharacter	*pcsCharacter	= m_pcsAgpmCharacter->GetCharSequence(&lIndex);

	while (pcsCharacter)
	{
		ApdEvent* pApdEvent = m_pcsApmEventManager->GetEvent(pcsCharacter, APDEVENT_FUNCTION_QUEST);
		if (pApdEvent)
			SetCharacterEffect(pcsCharacter, pApdEvent);

		pcsCharacter = m_pcsAgpmCharacter->GetCharSequence(&lIndex);
	}

	if (m_pcsApmObject)
	{
		lIndex = 0;
		ApdObject		*pcsObject		= m_pcsApmObject->GetObjectSequence(&lIndex);
		while (pcsObject)
		{
			ApdEvent* pApdEvent = m_pcsApmEventManager->GetEvent(pcsObject, APDEVENT_FUNCTION_QUEST);
			if (pApdEvent)
				SetCharacterEffect(pcsObject, pApdEvent);

			pcsObject = m_pcsApmObject->GetObjectSequence(&lIndex);
		}
	}

	return TRUE;
}

BOOL AgcmUIQuest2::CBCompleteQuestResult(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	CBQuestParam *pParam = (CBQuestParam*)pData;

	return pThis->CompleteQuestResult(pParam->pcsAgpdCharacter, pParam->lQuestTID, pParam->bResult);
}

BOOL AgcmUIQuest2::CompleteQuestResult(AgpdCharacter *pcsAgpdCharacter, INT32 lQuestTID, BOOL bResult)
{
	if (bResult)
	{
		AgpdQuest* pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsAgpdCharacter);
		if (!pAgpdQuest)
			return FALSE;

		AgpdQuestTemplate* pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestTID);
		if (!pQuestTemplate)
			return FALSE;

		m_pcsAgcmQuestJournal->DelQuestJournal( lQuestTID );

		pAgpdQuest->m_csFlag.SetValueByDBID(pQuestTemplate->m_lID, TRUE);
		AgpdQuest::IterQuest iter = pAgpdQuest->FindByQuestID(lQuestTID);
		if (iter != pAgpdQuest->m_vtQuest.end())
			pAgpdQuest->m_vtQuest.erase(iter);

		RefreshMainTree();
		RefreshMainText();
		RefreshMainCount();

		if (m_pcsAgcmEventEffect)
			m_pcsAgcmEventEffect->SetCommonCharacterEffect(pcsAgpdCharacter, E_COMMON_CHAR_TYPE_QUEST_DONE);

		CHAR szText[128] = {0,};
		sprintf(szText, ClientStr().GetStr(STI_QUEST_COMPLETE2), pQuestTemplate->m_QuestInfo.szName);
		AgcChatManager::OnAddSystemMessage(szText);

		m_lMessageBoxQuestTID = lQuestTID;
		m_lLastCompleteTID = lQuestTID;
		m_lClickIDAcceptUI = INVALID_INDEX;
		m_pcsAgcmUIManager2->ThrowEvent(m_lMessageQuestComplete);

		ClearQuestPositionInfo( lQuestTID );

		// 퀘스트 보상 아이템을 찍어준다.
		AddMessageResult(&pQuestTemplate->m_Result.Item1);
		AddMessageResult(&pQuestTemplate->m_Result.Item2);
		AddMessageResult(&pQuestTemplate->m_Result.Item3);
		AddMessageResult(&pQuestTemplate->m_Result.Item4);

		// 퀘스트 보상 스킬을 찍어준다.
		AddMessageResult(&pQuestTemplate->m_Result.ProductSkill);

		// 퀘스트 보상 겔드와 경험치는 따로 패킷이 온다.
	}

	RefreshBaseEffect();

	if (m_pcsAgcmEventQuest->m_pcsLastGrantEvent)
		MakeAcceptList(&m_AcceptList[0], &m_lAcceptListCount, m_pcsAgcmEventQuest->m_pcsLastGrantEvent);

	RefreshAcceptList();
	RefreshAcceptText();
	RefreshAcceptListAutoStart();

	RefreshAcceptLeftButtonEnable(FALSE);

	return TRUE;
}

void AgcmUIQuest2::AddMessageResult(AgpdElementItem* pItem)
{
	CHAR szBuffer[256] = {0,};
	INT32 lSize = 0;

	if (pItem->pItemTemplate)
	{
		if (pItem->pItemTemplate->m_bStackable)
		{
			if (g_eServiceArea == AP_SERVICE_AREA_CHINA)
				lSize = sprintf(szBuffer, "%s %d %s %s",
								ClientStr().GetStr(STI_ITEM_GET),
								pItem->lCount,
								pItem->pItemTemplate->m_szName,
								ClientStr().GetStr(STI_ITEM_POST));
			else if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				lSize = sprintf(szBuffer, "%d %s %s",
								pItem->lCount,
								pItem->pItemTemplate->m_szName,
								ClientStr().GetStr(STI_ITEM_GET));
			else
				lSize = sprintf(szBuffer, "%s %s %d %s",
								pItem->pItemTemplate->m_szName,
								ClientStr().GetStr(STI_ITEM_POST),
								pItem->lCount,
								ClientStr().GetStr(STI_ITEM_GET));
		}
		else
		{
			lSize = sprintf(szBuffer, "%s %s", pItem->pItemTemplate->m_szName, ClientStr().GetStr(STI_GET_ITEM2));
		}

		ASSERT(lSize < 256);
		AgcChatManager::OnAddSystemMessage(szBuffer);
	}
}

void AgcmUIQuest2::AddMessageResult(AgpdElementSkill* pPRoductSkill)
{
	CHAR szBuffer[256] = {0,};
	INT32 lSize = 0;

	if (pPRoductSkill->pSkillTemplate)
	{
		lSize = sprintf(szBuffer, ClientStr().GetStr(STI_SKILL_LEARN2), pPRoductSkill->pSkillTemplate->m_szName);
		ASSERT(lSize < 256);
		AgcChatManager::OnAddSystemMessage(szBuffer);
	}
}

BOOL AgcmUIQuest2::CBCancelQuestResult(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	CBQuestParam *pParam = (CBQuestParam*)pData;

	// 퀘스트 삭제
	pThis->CancelQuestResult(pParam->pcsAgpdCharacter, pParam->lQuestTID, pParam->bResult);

	// 퀘스트 목록창의 트리 업데이트
	AgcmUIManager2* pcmUIManager = ( AgcmUIManager2* )g_pEngine->GetModule( "AgcmUIManager2" );
	if( !pcmUIManager ) return FALSE;

	AgcdUI* pcdUI = pcmUIManager->GetUI( "QuestMain_UI" );
	if( !pcdUI ) return FALSE;

	AgcdUIControl* pcdControl = pcmUIManager->GetControl( pcdUI, "CONTROL_10830" );
	if( !pcdControl || !pcdControl->m_pcsBase ) return FALSE;

	AcUITree* pTree = ( AcUITree* )pcdControl->m_pcsBase;
	pTree->OnMouseWheel( 0 );
	return TRUE;
}

BOOL AgcmUIQuest2::CancelQuestResult(AgpdCharacter *pcsAgpdCharacter, INT32 lQuestTID, BOOL bResult)
{
	if (!bResult) return FALSE;

	AgpdQuest* pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsAgpdCharacter);
	if (!pAgpdQuest)
		return FALSE;

	AgpdQuestTemplate* pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestTID);
	if (!pQuestTemplate)
		return FALSE;

	m_pcsAgcmQuestJournal->DelQuestJournal( lQuestTID );

	CHAR szText[128] = {0,};
	sprintf(szText, "<HFFCC0000>[%s] %s<HFFFFFFFF>", pQuestTemplate->m_QuestInfo.szName, ClientStr().GetStr(STI_REMOVE_QUEST));
	AgcChatManager::OnAddSystemMessage(szText);

	AgpdQuest::IterQuest iter = pAgpdQuest->FindByQuestID(lQuestTID);
	if (iter != pAgpdQuest->m_vtQuest.end())
	{
		pAgpdQuest->m_vtQuest.erase(iter);
	}

	m_pcsAgcmUIManager2->ThrowEvent(m_lSoundQuestFail);

	if (m_pcsAgcmEventQuest->m_pcsLastGrantEvent)
		MakeAcceptList(&m_AcceptList[0], &m_lAcceptListCount, m_pcsAgcmEventQuest->m_pcsLastGrantEvent);

	m_lMessageBoxQuestTID = lQuestTID;
	m_lClickIDMainUI = INVALID_INDEX;

	ClearQuestPositionInfo( lQuestTID );

	RefreshAcceptList();
	RefreshAcceptText();
	RefreshMainTree();
	RefreshMainCount();
	RefreshMainText();
	RefreshBaseEffect();
	RefreshAcceptListAutoStart();

	return TRUE;
}

BOOL AgcmUIQuest2::CBInventoryFull(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	CBQuestParam *pParam = (CBQuestParam*)pData;

	pThis->m_lMessageBoxQuestTID = pParam->lQuestTID;
	SystemMessage.ProcessSystemMessage(ClientStr().GetStr(STI_NO_SPACE_CANT_COMPLETE));
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lMessageMainInventoryFull);

	return TRUE;
}

BOOL AgcmUIQuest2::CBQuestInventoryFull(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	CBQuestParam *pParam = (CBQuestParam*)pData;

	AgpdQuestTemplate* pQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(pParam->lQuestTID);
	if (!pQuestTemplate)
		return FALSE;

	CHAR szText[128] = {0,};
	sprintf(szText, "<HFFCC0000>[%s] %s<HFFFFFFFF>", 
			pQuestTemplate->m_QuestInfo.szName,
			ClientStr().GetStr(STI_NO_MORE_QUEST2));
	SystemMessage.ProcessSystemMessage(szText);

	pThis->m_lMessageBoxQuestTID = pParam->lQuestTID;
	SystemMessage.ProcessSystemMessage(szText);
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lMessageQuestInventoryFull);

	return TRUE;
}

BOOL AgcmUIQuest2::CBUpdateQuest(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	CBQuestParam *pParam = (CBQuestParam*)pData;

	INT32 lQuestID, lParam1, lParam2, lItemTID;

	BOOL	bRegisterJournal	=	FALSE;
	CHAR*	szNPCName			=	NULL;

	pThis->m_pcsAgpmQuest->GetFieldEmbeddedPacket(pParam->pvPacket, lQuestID, lParam1, lParam2, lItemTID, szNPCName, bRegisterJournal);

	AgpdQuest *pAgpdQuest = pThis->m_pcsAgpmQuest->GetAttachAgpdQuestData(pParam->pcsAgpdCharacter);
	AgpdQuest::IterQuest iter = pAgpdQuest->FindByQuestID(lQuestID);
	if (iter != pAgpdQuest->m_vtQuest.end())
	{
		AgpdQuestTemplate* pcsAgpdQuestTemplate = pThis->m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestID);
		if (pcsAgpdQuestTemplate)
		{
			CHAR szQuestName[128] = {0,};
 			if (pcsAgpdQuestTemplate->m_CompleteCondition.Monster1.pMonsterTemplate) 
			{
				if ((*iter).lParam1 < lParam1)
				{
					sprintf(szQuestName, "%s %d/%d", pcsAgpdQuestTemplate->m_CompleteCondition.Monster1.pMonsterTemplate->m_szTName,
													lParam1,
													pcsAgpdQuestTemplate->m_CompleteCondition.Monster1.lCount);
					pThis->Notice(szQuestName);
				}
			}

			if ( pcsAgpdQuestTemplate->m_CompleteCondition.Monster2.pMonsterTemplate)
			{
				if ((*iter).lParam2 < lParam2)
				{
					sprintf(szQuestName, "%s %d/%d", pcsAgpdQuestTemplate->m_CompleteCondition.Monster2.pMonsterTemplate->m_szTName,
													lParam2,
													pcsAgpdQuestTemplate->m_CompleteCondition.Monster2.lCount);
					pThis->Notice(szQuestName);
				}
			}
		}

		(*iter).lParam1 = lParam1;
		(*iter).lParam2 = lParam2;
	}

	if (lItemTID)
	{
		pThis->NoticeQuestItem(lQuestID, lItemTID);
	}

	pThis->RefreshMainText();
	pThis->RefreshAcceptList();
	pThis->RefreshAcceptText();
	pThis->RefreshAcceptListAutoStart();

	return TRUE;
}

BOOL AgcmUIQuest2::CBRefreshAll(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUIQuest2 *	pThis = (AgcmUIQuest2*)pClass;
	AgpdCharacter *	pcsCharacter = (AgpdCharacter *) pData;

	if (pThis->m_pcsAgcmCharacter->GetSelfCharacter() == pcsCharacter)
	{
		pThis->m_bOpenAutoStart = FALSE;

		pThis->RefreshAcceptList();
		pThis->RefreshAcceptText();
		pThis->RefreshMainTree();
		pThis->RefreshMainCount();
		pThis->RefreshMainText();
		pThis->RefreshBaseEffect();
		pThis->RefreshAcceptListAutoStart();
	}

	return TRUE;
}

BOOL AgcmUIQuest2::NoticeQuestItem(INT32 lQuestTID, INT32 lItemTID)
{
	AgpdQuestTemplate *pcsQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(lQuestTID);
	if (!pcsQuestTemplate)
		return FALSE;

	AgpdCharacter *pcsAgpdCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsAgpdCharacter)
		return FALSE;

	AgpdItemADChar* pcsAgpdItemADChar = m_pcsAgpmQuest->m_pcsAgpmItem->GetADCharacter(pcsAgpdCharacter);
	if (!pcsAgpdItemADChar) 
		return FALSE;

	ApSafeArray<CHAR, 256> szBuffer;

	if (pcsQuestTemplate->m_CompleteCondition.Monster1.Item.pItemTemplate)
	{
		if (pcsQuestTemplate->m_CompleteCondition.Monster1.Item.pItemTemplate->m_lID == lItemTID)
		{
			INT32 lCount = m_pcsAgpmGrid->GetCountByTemplate(&pcsAgpdItemADChar->m_csInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM,
												pcsQuestTemplate->m_CompleteCondition.Monster1.Item.pItemTemplate->m_lID);

			sprintf(&szBuffer[0], "%s (%d/%d)", pcsQuestTemplate->m_CompleteCondition.Monster1.Item.pItemTemplate->m_szName,
						lCount, pcsQuestTemplate->m_CompleteCondition.Monster1.Item.lCount);

			Notice(&szBuffer[0]);

//			sprintf(&szBuffer[0], "<HFFFFFF00>%s 아이템을 획득하였습니다.<HFFFFFFFF>", 
//						pcsQuestTemplate->m_CompleteCondition.Monster1.Item.pItemTemplate->m_szName);
//			AddSystemMessage(&szBuffer[0]);
		}
	}

	if (pcsQuestTemplate->m_CompleteCondition.Monster2.Item.pItemTemplate)
	{
		if (pcsQuestTemplate->m_CompleteCondition.Monster2.Item.pItemTemplate->m_lID == lItemTID)
		{
			INT32 lCount = m_pcsAgpmGrid->GetCountByTemplate(&pcsAgpdItemADChar->m_csInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM,
												pcsQuestTemplate->m_CompleteCondition.Monster2.Item.pItemTemplate->m_lID);

			sprintf(&szBuffer[0], "%s (%d/%d)", pcsQuestTemplate->m_CompleteCondition.Monster2.Item.pItemTemplate->m_szName,
						lCount, pcsQuestTemplate->m_CompleteCondition.Monster2.Item.lCount);

			Notice(&szBuffer[0]);

//			sprintf(&szBuffer[0], "<HFFFFFF00>%s 아이템을 획득하였습니다.<HFFFFFFFF>", 
//						pcsQuestTemplate->m_CompleteCondition.Monster2.Item.pItemTemplate->m_szName);
//			AddSystemMessage(&szBuffer[0]);
		}
	}

	for (INT32 i = 0; i < AGPDQUEST_MAX_CHECKPOINT; ++i)
	{
		szBuffer.MemSetAll();
		if (!pcsQuestTemplate->m_CheckPoint.CheckPointItem[i].pItemTemplate) continue;

		if (pcsQuestTemplate->m_CheckPoint.CheckPointItem[i].pItemTemplate->m_lID == lItemTID)
		{
			INT32 lCount = m_pcsAgpmGrid->GetCountByTemplate(&pcsAgpdItemADChar->m_csInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM,
												pcsQuestTemplate->m_CheckPoint.CheckPointItem[i].pItemTemplate->m_lID);

			sprintf(&szBuffer[0], "%s (%d/%d)", pcsQuestTemplate->m_CheckPoint.CheckPointItem[i].pItemTemplate->m_szName,
					lCount, pcsQuestTemplate->m_CheckPoint.CheckPointItem[i].lItemCount);

			Notice(&szBuffer[0]);

//			sprintf(&szBuffer[0], "<HFFFFFF00>%s 아이템을 획득하였습니다.<HFFFFFFFF>", 
//						pcsQuestTemplate->m_CheckPoint.CheckPointItem[i].pItemTemplate->m_szName);
//			AddSystemMessage(&szBuffer[0]);
		}
	}

	return TRUE;
}

BOOL AgcmUIQuest2::Notice(CHAR *szMessage)
{
	m_pcsAgcmUINotice->AddMessage( szMessage );
	AgcChatManager::OnAddQuestMessage( szMessage );


	return TRUE;
}

BOOL AgcmUIQuest2::CBDispalyQuestCount(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;

	AgpdCharacter *pcsAgpdCharacter = pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsAgpdCharacter)
		return FALSE;

	AgpdQuest *pAgpdQuest = pThis->m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsAgpdCharacter);
	if (!pAgpdQuest)
		return FALSE;

	sprintf(szDisplay, "%s (%d%s)", ClientStr().GetStr(STI_QUEST_LIST), pAgpdQuest->m_vtQuest.size(), ClientStr().GetStr(STI_NUMBER_UNIT));

	return TRUE;
}

BOOL AgcmUIQuest2::CBAfterCooldown(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass)
		return FALSE;

	AgcmUIQuest2 *pThis = (AgcmUIQuest2*)pClass;
	pThis->m_bHold = FALSE;

	if (!pThis->m_pCheckPointEvent)
		return FALSE;

	AgpdCharacter *pcsAgpdCharacter = pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsAgpdCharacter)
		return FALSE;

	return pThis->m_pcsAgcmQuest->SendPacketCheckPoint(pThis->m_CheckPointData.lQuestTID, pThis->m_pCheckPointEvent, pcsAgpdCharacter->m_lID, 
												pThis->m_CheckPointData.lIndex);
}

BOOL AgcmUIQuest2::CBAddItemInventory(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	if (NULL != pCustData)
		return FALSE;

	AgcmUIQuest2::CBQuestItemNotice(pData, pClass, NULL);

	return TRUE;
}

BOOL AgcmUIQuest2::CBQuestItemNotice(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgcmUIQuest2 *pThis		= (AgcmUIQuest2*)pClass;
	AgpdItem *pcsAgpdItem	= (AgpdItem*)pData;

	// 노멀 아이템에서 확인을 해보고
	if( !pThis->NoticeNormalItem(pcsAgpdItem) )
	{
		// 없다면 퀘스트 아이템으로
		// 퀘스트 아이템이면 해당 퀘스트를 찾는다.
		INT32 lQuestTID = pThis->GetQuestTIDByItem(pcsAgpdItem);
		if (lQuestTID)
		{
			pThis->NoticeQuestItem(lQuestTID, ((AgpdItemTemplate*)pcsAgpdItem->m_pcsItemTemplate)->m_lID);
			pThis->RefreshMainTree();
			pThis->RefreshMainText();
// 			pThis->RefreshMainCompensation();

			pThis->m_pcsAgcmUIQuestJournal->RefreshJournalInfo();
			pThis->m_pcsAgcmUIQuestJournal->RefreshJournalPosition();
		}
	}

	return TRUE;
}

INT32 AgcmUIQuest2::GetQuestTIDByItem(AgpdItem* pcsAgpdItem)
{
	AgpdCharacter *pcsAgpdCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsAgpdCharacter)
		return FALSE;

	AgpdQuest *pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsAgpdCharacter);
	if (!pAgpdQuest) 
		return FALSE;

	AgpdQuestTemplate *pQuestTemplate;
	for(AgpdQuest::IterQuest iter = pAgpdQuest->m_vtQuest.begin(); iter != pAgpdQuest->m_vtQuest.end(); ++iter)
	{
		pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get((*iter).lQuestID);
		if (!pQuestTemplate) continue;

		if (pQuestTemplate->m_CompleteCondition.Monster1.Item.pItemTemplate)
		{
			if (pQuestTemplate->m_CompleteCondition.Monster1.Item.pItemTemplate->m_lID == 
				((AgpdItemTemplate*)pcsAgpdItem->m_pcsItemTemplate)->m_lID)
				return pQuestTemplate->m_lID;
		}

		if (pQuestTemplate->m_CompleteCondition.Monster2.Item.pItemTemplate)
		{
			if (pQuestTemplate->m_CompleteCondition.Monster2.Item.pItemTemplate->m_lID == 
				((AgpdItemTemplate*)pcsAgpdItem->m_pcsItemTemplate)->m_lID)
				return pQuestTemplate->m_lID;
		}

		for (INT32 i = 0; i < AGPDQUEST_MAX_CHECKPOINT; ++i)
		{
			if (!pQuestTemplate->m_CheckPoint.CheckPointItem[i].pItemTemplate) continue;

			if (pQuestTemplate->m_CheckPoint.CheckPointItem[i].pItemTemplate->m_lID ==
				((AgpdItemTemplate*)pcsAgpdItem->m_pcsItemTemplate)->m_lID)
				return pQuestTemplate->m_lID;
		}

	}

	return 0;
}

BOOL AgcmUIQuest2::NoticeNormalItem(AgpdItem *pcsAgpdItem)
{
	AgpdCharacter *pcsAgpdCharacter = m_pcsAgcmCharacter->GetSelfCharacter();
	if (!pcsAgpdCharacter)
		return FALSE;

	AgpdItemADChar* pcsAgpdItemADChar = m_pcsAgpmQuest->m_pcsAgpmItem->GetADCharacter(pcsAgpdCharacter);
	if (!pcsAgpdItemADChar) 
		return FALSE;

	AgpdQuest *pAgpdQuest = m_pcsAgpmQuest->GetAttachAgpdQuestData(pcsAgpdCharacter);
	if (!pAgpdQuest) 
		return FALSE;

	for (AgpdQuest::IterQuest iter = pAgpdQuest->m_vtQuest.begin(); iter != pAgpdQuest->m_vtQuest.end(); ++iter)
	{
		AgpdQuestTemplate *pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get(iter->lQuestID);
		if (!pQuestTemplate)
			continue;

		if (!pQuestTemplate->m_CompleteCondition.Item.pItemTemplate)
			continue;
		
		if (pQuestTemplate->m_CompleteCondition.Item.pItemTemplate->m_lID == ((AgpdItemTemplate*)pcsAgpdItem->m_pcsItemTemplate)->m_lID)
		{
			ApSafeArray<CHAR, 256> szBuffer;
			INT32 lCount = m_pcsAgpmGrid->GetCountByTemplate(&pcsAgpdItemADChar->m_csInventoryGrid, AGPDGRID_ITEM_TYPE_ITEM,
												pQuestTemplate->m_CompleteCondition.Item.pItemTemplate->m_lID);

			if (0 == lCount)
				continue;

			if(lCount > pQuestTemplate->m_CompleteCondition.Item.lCount)
				lCount = pQuestTemplate->m_CompleteCondition.Item.lCount;

			sprintf(&szBuffer[0], "%s (%d/%d)", pQuestTemplate->m_CompleteCondition.Item.pItemTemplate->m_szName,
						lCount, pQuestTemplate->m_CompleteCondition.Item.lCount);

			Notice(&szBuffer[0]);
			RefreshMainTree();
			RefreshMainText();

			m_pcsAgcmUIQuestJournal->RefreshJournalInfo();
			m_pcsAgcmUIQuestJournal->RefreshJournalPosition();

			return TRUE;
		}
	}

	return FALSE;
}

BOOL AgcmUIQuest2::AddQuestPositionInfo( INT nQuestID , MapLinkPos pos, std::string & name )
{
	std::string & orgPosName = m_qeustPositionsByID[ nQuestID ][pos];

	if( orgPosName.empty() )
		orgPosName = name;

	return TRUE;
}

BOOL AgcmUIQuest2::DelQuestPositionInfo( INT nQuestID , MapLinkPos pos )
{
	QuestPositionsByQuestID::iterator	Iter	=	m_qeustPositionsByID.find( nQuestID );
	if( Iter == m_qeustPositionsByID.end() )
	{
		return FALSE;
	}

	QuestPostions::iterator posIter = Iter->second.find( pos );

	if( posIter != Iter->second.end() )
	{
		Iter->second.erase( posIter );
		return TRUE;
	}

	return FALSE;
}

std::string AgcmUIQuest2::GetQuestPosName( INT nQuestID, MapLinkPos pos )
{
	return m_qeustPositionsByID[ nQuestID ][ pos ];
}

BOOL AgcmUIQuest2::ClearQuestPositionInfo( INT nQuestID )
{
	QuestPositionsByQuestID::iterator	Iter	=	m_qeustPositionsByID.find( nQuestID );
	if( Iter == m_qeustPositionsByID.end() )
	{
		return FALSE;
	}

	QuestPostions::iterator	PosIter	=	Iter->second.begin();
	for( ; PosIter != Iter->second.end() ; ++PosIter )
	{
		int nID = PosIter->first.GetNID( PosIter->second.length() );
		m_pcsAgcmMinimap->DestroyMapPosition( nID );
	}

	return TRUE;
}

VOID AgcmUIQuest2::_RefreshTextList( eQuestTextType eType , AcUIBase* pUIBase , BOOL bReParse /*= FALSE */ )
{
	if( !pUIBase )
		return;

	//pUIBase->AddChild( 0 );

	// 유효한 체크후에 퀘스트템플릿을 얻어온다
	AgpdQuestTemplate*	pQuestTemplate	= _CheckQuest( eType );
	if( !pQuestTemplate )
	{
		acceptQuestText_->ClearTextList( TRUE );
		return;
	}

	if( bReParse )
	{
		CHAR szBuffer[ TEXT_BUFFER_SIZE ] = {0,};

		switch( eType )
		{
		case E_QUEST_TEXT_MAIN:
			ComposeMainText( pQuestTemplate, szBuffer, TEXT_BUFFER_SIZE );
			mainQuestText_->RefreshTextList( szBuffer, pUIBase );
			break;
		case E_QUEST_TEXT_ACCEPT:
			ComposeAcceptText( pQuestTemplate , szBuffer , TEXT_BUFFER_SIZE );
			acceptQuestText_->RefreshTextList( szBuffer, pUIBase );
			break;
		}
	}
}

AgpdQuestTemplate* AgcmUIQuest2::_CheckQuest( eQuestTextType eType )
{
	AgpdQuestTemplate* pQuestTemplate	=	NULL;

	if( eType == E_QUEST_TEXT_MAIN )
	{
		if (INVALID_INDEX != m_lClickIDMainUI)
		{
			pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get( m_lMainQuestIDList[m_lClickIDMainUI] );
		}
	}

	else if( eType == E_QUEST_TEXT_ACCEPT )
	{
		if ( INVALID_INDEX != m_lClickIDAcceptUI )
		{
			pQuestTemplate = m_pcsAgpmQuest->m_csQuestTemplate.Get( m_AcceptList[m_lClickIDAcceptUI].lQuestTID );
		}
	}

	return pQuestTemplate;
}

INT32 AgcmUIQuest2::_GetTopQuestTID( INT nTID )
{
	AgpdQuestTemplate*	pTemplate	=	m_pcsAgpmQuest->m_csQuestTemplate.Get( nTID );
	if( !pTemplate )
		return 0;

	if( pTemplate->m_StartCondition.Quest.lBackupTID )
		return _GetTopQuestTID( pTemplate->m_StartCondition.Quest.lBackupTID );

	return pTemplate->GetID();
}

VOID AgcmUIQuest2::SetScrollButton( VOID )
{
	//	Scroll을 세팅한다.
	AgcdUI*	pQuestUI	=	m_pcsAgcmUIManager2->GetUI( AGCMUI_MAIN_QUEST_UI_NAME );
	if( pQuestUI )
	{	
		mainQuestText_->SetScrollControl( (AcUIScroll*)(m_pcsAgcmUIManager2->GetControl( pQuestUI , AGCMUI_MAIN_EDIT_SCROLL_NAME )->m_pcsBase) );
	}

	AgcdUI*	pAcceptQuestUI	=	m_pcsAgcmUIManager2->GetUI( AGCMUI_ACCEPT_QUEST_UI_NAME );
	if( pAcceptQuestUI )
	{
		AgcdUIControl* pTextScrollControl = m_pcsAgcmUIManager2->GetControl( pAcceptQuestUI , AGCMUI_ACCEPT_TEXT_SCROLL_NAME );
		if(pTextScrollControl)
			acceptQuestText_->SetScrollControl( (AcUIScroll*)pTextScrollControl->m_pcsBase);
	}
}

BOOL AgcmUIQuest2::CBRemoveItemInventory(PVOID pData, PVOID pClass, PVOID pCustData)
{
	AgcmUIQuest2*	pThis	=	static_cast< AgcmUIQuest2* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->RefreshMainText();

	pThis->m_pcsAgcmUIQuestJournal->RefreshJournalInfo();
	pThis->m_pcsAgcmUIQuestJournal->RefreshJournalPosition();

	return TRUE;
}

BOOL AgcmUIQuest2::OpenMainQuest( VOID )
{
	return m_pcsAgcmUIManager2->ThrowEvent(m_lEventMainUIOpen);
}
