#include "AgcmUISiegeWar.h"
#include "AgcmUIGuild.h"
#include "AuTimeStamp.h"
#include "AgcmTextBoardMng.h"
#include "AgcChatManager.h"
//#define	__TEST_SERVER_ONLY__	1


namespace {	// anonymous namespace for local use
	void RemoveTag(char* org, int orgLength, char* dest, int destLength)
	{
		bool insideTag = false;
		if (orgLength >= destLength) orgLength = destLength - 1;

		memset(dest, 0, destLength);

		int index = 0;
		for (int i = 0; i < orgLength; ++i)
		{
			if (org[i] == '<') insideTag = true;
			if (org[i] == '>') {
				insideTag = false;
				continue;
			}

			if (insideTag) continue;

			dest[index++] = org[i];
		}
	}

	char* FindNameWithoutJob( char* str )
	{
		size_t len = strnlen( str, 64 );
		size_t pos = strcspn( str, ">" );

		if ( pos == len ) return str;

		// '>'을 가르키고 있으니 다음으로
		++pos;

		while ( str[pos] == ' ' )
			++pos;

		return str + pos;
	}
}

AgcmUISiegeWar::AgcmUISiegeWar()
{
	SetModuleName("AgcmUISiegeWar");

	EnableIdle(TRUE);

	m_pcsEvent					= NULL;
	m_pcsSiegeWar				= NULL;

	m_pcsUserDataSiegeWar		= NULL;
	m_pcsUserDataDateCombo		= NULL;
	m_pcsUserDataHourCombo		= NULL;

	m_pcsUserDataAttackList		= NULL;
	m_pcsUserDataAttackCurrentPage	= NULL;

	m_pcsUserDataDefenseList		= NULL;
	m_pcsUserDataDefenseCurrentPage	= NULL;

	m_pcsUserDataCatapultGrid		= NULL;
	m_pcsUserDataAtkResTowerGrid	= NULL;

	m_pcsUserDataActiveCatapultRepairButton	= NULL;
	m_pcsUserDataActiveAtkResTowerRepairButton	= NULL;

	m_pcsUserDataActiveCatapultActiveButton	= NULL;
	m_pcsUserDataActiveAtkResTowerActiveButton	= NULL;

	m_pcsUserDataActiveAttackApplButton	= NULL;
	m_pcsUserDataUseRepairBar	= NULL;

	m_pcsUserDataUseCharName	= NULL;

	m_pcsMainUIDescEdit			= NULL;
	m_pcsMainUISiegeWarTimeEdit	= NULL;

	m_pcsAttackHelpEdit			= NULL;
	m_pcsAttackSiegeWarTimeEdit	= NULL;

	m_pcsDefenseHelpEdit		= NULL;
	m_pcsDefenseGuildInfoEdit	= NULL;

	m_pcsCatapultDescEdit		= NULL;
	m_pcsAtkResTowerDescEdit	= NULL;

	m_pcsDateCombo				= NULL;
	m_pcsHourCombo				= NULL;

	ZeroMemory(m_szDateCombo, sizeof(m_szDateCombo));
	ZeroMemory(m_szHourCombo, sizeof(m_szHourCombo));

	ZeroMemory(m_astAttackApplGuildList, sizeof(m_astAttackApplGuildList));
	ZeroMemory(m_astDefenseApplGuildList, sizeof(m_astDefenseApplGuildList));

	m_lAttackStartPage				= 0;
	m_lAttackCurrentPage			= 0;
	m_lAttackMaxPage				= 0;

	m_lDefenseStartPage				= 0;
	m_lDefenseCurrentPage			= 0;
	m_lDefenseMaxPage				= 0;

	ZeroMemory(m_aszDateComboPointer, sizeof(m_aszDateComboPointer));
	ZeroMemory(m_aszHourComboPointer, sizeof(m_aszHourComboPointer));

	for (int i = 0; i < AGCMUISIEGEWAR_MAX_SELECT_DATE; ++i)
		m_aszDateComboPointer[i]	= m_szDateCombo[i];
	for (int i = 0; i < AGCMUISIEGEWAR_MAX_SELECT_HOUR; ++i)
		m_aszHourComboPointer[i]	= m_szHourCombo[i];

	ZeroMemory(m_apstAttackApplGuildListPointer, sizeof(m_apstAttackApplGuildListPointer));
	ZeroMemory(m_apstDefenseApplGuildListPointer, sizeof(m_apstDefenseApplGuildListPointer));

	for (int i = 0; i < AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE; ++i)
		m_apstAttackApplGuildListPointer[i]		= &m_astAttackApplGuildList[i];
	for (int i = 0; i < AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE; ++i)
		m_apstDefenseApplGuildListPointer[i]	= &m_astDefenseApplGuildList[i];

	m_lSelectedDefenseGuildList	= (-1);

	ZeroMemory(m_astAttackGuildList, sizeof(m_astAttackGuildList));
	ZeroMemory(m_astDefenseGuildList, sizeof(m_astDefenseGuildList));

	m_bIsActiveCatapultRepairButton	= FALSE;
	m_bIsActiveAtkResTowerRepairButton	= FALSE;
	m_bIsActiveCatapultActiveButton	= FALSE;
	m_bIsActiveAtkResTowerActiveButton	= FALSE;

	m_bIsActiveAttackApplButton	= FALSE;

	m_pcsTarget	= NULL;

	m_pcsCatapultGrid		= NULL;
	m_pcsAtkResTowerGrid	= NULL;

	m_lCooldownIndex		= 0;

	m_pcsSiegeWarInfoEdit	= NULL;
	m_pcsRemainTimeEdit		= NULL;

	m_lSiegeWarInfoScroll	= 0;

	ZeroMemory(m_szUseCharName, sizeof(m_szUseCharName));

	m_lRepairStartTimeMSec	= 0;
	m_lRepairDurationMSec	= 0;

	m_bIsOpenedRepairBarUI	= FALSE;

	m_bOpenUI				= FALSE;
	ZeroMemory(&m_stOpenUIPos, sizeof(m_stOpenUIPos));

	m_bInfoOpenUI			= FALSE;

	memset( m_strMessage, 0, sizeof( CHAR ) * 1024 );
	//m_nCurrLine = 0;
	//m_nViewLineCount = 0;
	//m_nMaxLine = 0;
}

AgcmUISiegeWar::~AgcmUISiegeWar()
{
}

BOOL AgcmUISiegeWar::OnAddModule()
{
	m_pcsApmMap					= (ApmMap *)				GetModule("ApmMap");
	m_pcsApmEventManager		= (ApmEventManager *)		GetModule("ApmEventManager");
	m_pcsAgpmSiegeWar			= (AgpmSiegeWar *)			GetModule("AgpmSiegeWar");
	m_pcsAgpmEventSiegeWarNPC	= (AgpmEventSiegeWarNPC *)	GetModule("AgpmEventSiegeWarNPC");
	m_pcsAgpmGuild				= (AgpmGuild *)				GetModule("AgpmGuild");
	m_pcsAgpmAdmin				= (AgpmAdmin *)				GetModule("AgpmAdmin");
	m_pcsAgpmCharacter			= (AgpmCharacter *)			GetModule("AgpmCharacter");
	m_pcsAgcmCharacter			= (AgcmCharacter *)			GetModule("AgcmCharacter");
	m_pcsAgcmSiegeWar			= (AgcmSiegeWar *)			GetModule("AgcmSiegeWar");
	m_pcsAgcmEventSiegeWarNPC	= (AgcmEventSiegeWarNPC *)	GetModule("AgcmEventSiegeWarNPC");
	m_pcsAgcmUIManager2			= (AgcmUIManager2 *)		GetModule("AgcmUIManager2");
	m_pcsAgpmGrid				= (AgpmGrid *)				GetModule("AgpmGrid");
	m_pcsAgpmItem				= (AgpmItem *)				GetModule("AgpmItem");
	m_pcsAgcmUICharacter		= (AgcmUICharacter *)		GetModule("AgcmUICharacter");
	m_pcsAgcmUICooldown			= (AgcmUICooldown *)		GetModule("AgcmUICooldown");
	m_pcsAgcmUIItem				= (AgcmUIItem *)			GetModule("AgcmUIItem");
	m_pcsAgcmTextBoard			= (AgcmTextBoardMng *)		GetModule("AgcmTextBoardMng");
	m_pcsAgcmUIControl			= (AgcmUIControl *)			GetModule("AgcmUIControl");

	if (!m_pcsApmMap ||
		!m_pcsApmEventManager ||
		!m_pcsAgpmSiegeWar ||
		!m_pcsAgpmEventSiegeWarNPC ||
		!m_pcsAgpmGuild ||
		//!m_pcsAgpmAdmin ||
		!m_pcsAgcmCharacter ||
		!m_pcsAgcmSiegeWar ||
		!m_pcsAgcmEventSiegeWarNPC ||
		!m_pcsAgcmUIManager2 ||
		!m_pcsAgpmGrid ||
		!m_pcsAgpmItem ||
		!m_pcsAgcmUICooldown ||
		!m_pcsAgcmUIItem ||
		!m_pcsAgcmTextBoard ||
		!m_pcsAgcmUIControl ||
		!m_pcsAgpmCharacter ||
		!m_pcsAgcmUICharacter)
		return FALSE;

	if (!m_pcsAgpmSiegeWar->SetCallbackUpdateCastleInfo(CBUpdateCastleInfo, this))
		return FALSE;
	if (!m_pcsAgpmSiegeWar->SetCallbackReceiveResult(CBReceiveResult, this))
		return FALSE;
	if (!m_pcsAgpmSiegeWar->SetCallbackReceiveAttackApplGuildList(CBReceiveAttackApplGuildList, this))
		return FALSE;
	if (!m_pcsAgpmSiegeWar->SetCallbackReceiveDefenseApplGuildList(CBReceiveDefenseApplGuildList, this))
		return FALSE;
	/*if (!m_pcsAgpmSiegeWar->SetCallbackReceiveAttackGuildList(CBReceiveAttackGuildList, this))
		return FALSE;*/
	/*if (!m_pcsAgpmSiegeWar->SetCallbackReceiveDefenseGuildList(CBReceiveDefenseGuildList, this))
		return FALSE;*/
	if (!m_pcsAgpmSiegeWar->SetCallbackResponseUseAttackObject(CBResponseUseAttackObject, this))
		return FALSE;
	if (!m_pcsAgpmSiegeWar->SetCallbackAddItemResult(CBAddItemResult, this))
		return FALSE;
	if (!m_pcsAgpmSiegeWar->SetCallbackStatusInfo(CBStatusInfo, this))
		return FALSE;
	if (!m_pcsAgpmSiegeWar->SetCallbackSyncMessage(CBSyncMessage, this))
		return FALSE;
	if (!m_pcsAgpmSiegeWar->SetCallbackUpdateCastleInfo(CBUpdateStatus, this))
		return FALSE;

	if (!m_pcsAgpmEventSiegeWarNPC->SetCallbackEventGrant(CBEventGrant, this))
		return FALSE;

	if (!m_pcsAgcmCharacter->SetCallbackSetSelfCharacter(CBSetSelfCharacter, this))
		return FALSE;
	if (!m_pcsAgcmCharacter->SetCallbackSelfUpdatePosition(CBSelfUpdatePosition, this))
		return FALSE;

	if (!m_pcsAgcmUIItem->SetCallbackRemoveSiegeWarGrid(CBRemoveSiegeWarGrid, this))
		return FALSE;

	if (!m_pcsAgpmItem->SetCallbackRemove(CBRemoveItem, this))
		return FALSE;

	if (!m_pcsAgpmCharacter->SetCallbackUpdateActionStatus(CBUpdateActionStatus, this))
		return FALSE;
	if (!m_pcsAgpmCharacter->SetCallbackUpdateSpecialStatus(CBUpdateActionStatus, this))
		return FALSE;
	if( !m_pcsAgpmCharacter->SetCallbackBindingRegionChange(CBChangeCharacterBindingRegion, this))
		return FALSE;

// 	if (!m_pcsAgcmChatting2->SetCallbackInputSiegeWarMessage(CBInputSiegeWarMessage, this))
// 		return FALSE;

	if (!m_pcsAgcmUICharacter->SetCallbackSetTargetCharacter(CBSetTargetCharacter, this))
		return FALSE;
	if (!m_pcsAgcmUICharacter->SetCallbackResetTargetCharacter(CBResetTargetCharacter, this))
		return FALSE;

	if (!AddEvent())
		return FALSE;

	if (!AddFunction())
		return FALSE;

	if (!AddUserData())
		return FALSE;

	if (!AddDisplay())
		return FALSE;

	if (!AddBoolean())
		return FALSE;

	m_lCooldownIndex	= m_pcsAgcmUICooldown->RegisterCooldown(CBEndCooldown, this);
	if (m_lCooldownIndex < 0)
		return FALSE;

	return TRUE;
}

BOOL AgcmUISiegeWar::OnInit()
{
	return TRUE;
}

BOOL AgcmUISiegeWar::OnDestroy()
{
	m_listMessageChat.clear();
	return TRUE;
}

BOOL AgcmUISiegeWar::OnIdle(UINT32 ulClockCount)
{
	static DWORD dwTime1 = 0;
	static DWORD dwTime2 = timeGetTime();

	dwTime1 = timeGetTime();
	DWORD dwDelta = dwTime1 - dwTime2;

	SetRemainTimeEdit(ulClockCount);

	if (m_bIsOpenedRepairBarUI && m_pcsAgcmUICharacter->GetTargetCharacter())
	{
		// 수리 시간을 세팅한다.
		AgpdSiegeWarADCharacter	*pcsAttachData	= m_pcsAgpmSiegeWar->GetAttachCharacterData(m_pcsAgcmUICharacter->GetTargetCharacter());

		m_lRepairDurationMSec	= pcsAttachData->ucRepairDuration * 60 * 1000;
		m_lRepairStartTimeMSec	= pcsAttachData->m_ulStartRepairTimeMSec;

		m_pcsAgcmUIManager2->RefreshUserData(m_pcsUserDataUseRepairBar, TRUE);
	}

	m_dwPrintNextTimeChat += dwDelta;
	if(m_listMessageChat.size() != 0)
	{
		if(m_dwPrintNextTimeChat > AGCMUISIEGEWAR_PRINT_MESSAGE_TIME)
		{
			CHAR strMessage[ 1024 ];
			strcpy(strMessage, m_listMessageChat.begin()->c_str());
			m_dwPrintNextTimeChat = 0;
			SetInfoEdit(strMessage , 0xFFFFFFFF );
			SetTextEffect(strMessage);
			m_listMessageChat.pop_front();
		}
	}

	dwTime2 = dwTime1;

	return TRUE;
}

BOOL AgcmUISiegeWar::AddEvent()
{
	m_lEventOpenSiegeWarMainUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenMainUI");
	if (m_lEventOpenSiegeWarMainUI < 0)
		return FALSE;
	m_lEventOpenSetTimeUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenSetTimeUI");
	if (m_lEventOpenSetTimeUI < 0)
		return FALSE;
	m_lEventOpenApplicationDefenseUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenApplicationDefenseUI");
	if (m_lEventOpenApplicationDefenseUI < 0)
		return FALSE;
	m_lEventOpenSelectDefenseGuildUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenSelectDefenseGuildUI");
	if (m_lEventOpenSelectDefenseGuildUI < 0)
		return FALSE;
	m_lEventOpenApplicationAttackUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenApplicationAttackUI");
	if (m_lEventOpenApplicationAttackUI < 0)
		return FALSE;
	m_lEventCloseSiegeWarUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarCloseUI");
	if (m_lEventCloseSiegeWarUI < 0)
		return FALSE;

	m_lEventGetMainUIDescEdit	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetMainUIDescEdit");
	if (m_lEventGetMainUIDescEdit < 0)
		return FALSE;
	m_lEventGetMainUISiegeWarTimeEdit	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetMainUISiegeWarTimeEdit");
	if (m_lEventGetMainUISiegeWarTimeEdit < 0)
		return FALSE;
	m_lEventGetAttackHelpEdit	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetAttackHelpEdit");
	if (m_lEventGetAttackHelpEdit < 0)
		return FALSE;
	m_lEventGetAttackSiegeWarTimeEdit	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetAttackSiegeWarTimeEdit");
	if (m_lEventGetAttackSiegeWarTimeEdit < 0)
		return FALSE;
	m_lEventGetDefenseHelpEdit	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetDefenseHelpEdit");
	if (m_lEventGetDefenseHelpEdit < 0)
		return FALSE;
	m_lEventGetDefenseGuidInfoEdit	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetDefenseGuidInfoEdit");
	if (m_lEventGetDefenseGuidInfoEdit < 0)
		return FALSE;

	m_lEventGetDateCombo	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetDateCombo");
	if (m_lEventGetDateCombo < 0)
		return FALSE;
	m_lEventGetHourCombo	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetHourCombo");
	if (m_lEventGetHourCombo < 0)
		return FALSE;

	m_lEventRequireCastleMasterMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarRequireCastleMaster");
	if (m_lEventRequireCastleMasterMessage < 0)
		return FALSE;
	m_lEventRequireGuildMasterMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarRequireGuildMaster");
	if (m_lEventRequireGuildMasterMessage < 0)
		return FALSE;
	m_lEventRequireGuildConditionMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarRequireGuildCondition");
	if (m_lEventRequireGuildConditionMessage < 0)
		return FALSE;
	m_lEventAlreadyApplGuildMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarAlreadyApplGuild");
	if (m_lEventAlreadyApplGuildMessage < 0)
		return FALSE;
	m_lEventCannotSetSiegeWarTimeMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarCannotSetSiegeWarTime");
	if (m_lEventCannotSetSiegeWarTimeMessage < 0)
		return FALSE;
	m_lEventCannotApplSiegeWarMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarCannotApplSiegeWar");
	if (m_lEventCannotApplSiegeWarMessage < 0)
		return FALSE;
	m_lEventDefenseApplSuccessMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarDefenseApplSuccess");
	if (m_lEventDefenseApplSuccessMessage < 0)
		return FALSE;
	m_lEventAttackApplSuccessMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarAttackApplSuccess");
	if (m_lEventAttackApplSuccessMessage < 0)
		return FALSE;
	m_lEventSetDefenseGuildSuccessMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarDefenseGuildSuccess");
	if (m_lEventSetDefenseGuildSuccessMessage < 0)
		return FALSE;
	m_lEventSetNextSiegeWarTimeSuccessMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarNextSiegeWarTimeSuccess");
	if (m_lEventSetNextSiegeWarTimeSuccessMessage < 0)
		return FALSE;
	m_lEventNotEnoughMoneyMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarNotEnoughMoney");
	if (m_lEventNotEnoughMoneyMessage < 0)
		return FALSE;
	m_lEventNoMoreRepairMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarNoMoreRepair");
	if (m_lEventNoMoreRepairMessage < 0)
		return FALSE;

	m_lEventSelectAttackPageSlot[0]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectAttackPage1");
	if (m_lEventSelectAttackPageSlot[0] < 0)
		return FALSE;
	m_lEventSelectAttackPageSlot[1]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectAttackPage2");
	if (m_lEventSelectAttackPageSlot[1] < 0)
		return FALSE;
	m_lEventSelectAttackPageSlot[2]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectAttackPage3");
	if (m_lEventSelectAttackPageSlot[2] < 0)
		return FALSE;
	m_lEventSelectAttackPageSlot[3]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectAttackPage4");
	if (m_lEventSelectAttackPageSlot[3] < 0)
		return FALSE;
	m_lEventSelectAttackPageSlot[4]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectAttackPage5");
	if (m_lEventSelectAttackPageSlot[4] < 0)
		return FALSE;
	m_lEventSelectAttackPageSlot[5]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectAttackPage6");
	if (m_lEventSelectAttackPageSlot[5] < 0)
		return FALSE;
	m_lEventSelectAttackPageSlot[6]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectAttackPage7");
	if (m_lEventSelectAttackPageSlot[6] < 0)
		return FALSE;
	m_lEventSelectAttackPageSlot[7]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectAttackPage8");
	if (m_lEventSelectAttackPageSlot[7] < 0)
		return FALSE;
	m_lEventSelectAttackPageSlot[8]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectAttackPage9");
	if (m_lEventSelectAttackPageSlot[8] < 0)
		return FALSE;

	m_lEventSelectDefensePageSlot[0]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectDefensePage1");
	if (m_lEventSelectDefensePageSlot[0] < 0)
		return FALSE;
	m_lEventSelectDefensePageSlot[1]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectDefensePage2");
	if (m_lEventSelectDefensePageSlot[1] < 0)
		return FALSE;
	m_lEventSelectDefensePageSlot[2]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectDefensePage3");
	if (m_lEventSelectDefensePageSlot[2] < 0)
		return FALSE;
	m_lEventSelectDefensePageSlot[3]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectDefensePage4");
	if (m_lEventSelectDefensePageSlot[3] < 0)
		return FALSE;
	m_lEventSelectDefensePageSlot[4]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectDefensePage5");
	if (m_lEventSelectDefensePageSlot[4] < 0)
		return FALSE;
	m_lEventSelectDefensePageSlot[5]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectDefensePage6");
	if (m_lEventSelectDefensePageSlot[5] < 0)
		return FALSE;
	m_lEventSelectDefensePageSlot[6]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectDefensePage7");
	if (m_lEventSelectDefensePageSlot[6] < 0)
		return FALSE;
	m_lEventSelectDefensePageSlot[7]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectDefensePage8");
	if (m_lEventSelectDefensePageSlot[7] < 0)
		return FALSE;
	m_lEventSelectDefensePageSlot[8]	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectDefensePage9");
	if (m_lEventSelectDefensePageSlot[8] < 0)
		return FALSE;

	m_lEventGetCatapultEdit	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetCatapultEdit");
	if (m_lEventGetCatapultEdit < 0)
		return FALSE;
	m_lEventGetAtkResTowerEdit	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetAtkResTowerEdit");
	if (m_lEventGetAtkResTowerEdit < 0)
		return FALSE;

	m_lEventOpenCatapultUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenCatapultUI");
	if (m_lEventOpenCatapultUI < 0)
		return FALSE;
	m_lEventOpenAtkResTowerUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenAtkResTowerUI");
	if (m_lEventOpenAtkResTowerUI < 0)
		return FALSE;
	m_lEventCloseCatapultUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarCloseCatapultUI");
	if (m_lEventCloseCatapultUI < 0)
		return FALSE;
	m_lEventCloseAtkResTowerUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarCloseAtkResTowerUI");
	if (m_lEventCloseAtkResTowerUI < 0)
		return FALSE;

	m_lEventSelectedDefenseGuild	= m_pcsAgcmUIManager2->AddEvent("SiegeWarSelectedDefenseGuild");
	if (m_lEventSelectedDefenseGuild < 0)
		return FALSE;
	m_lEventDisableDefenseGuild	= m_pcsAgcmUIManager2->AddEvent("SiegeWarDisableDefenseGuild");
	if (m_lEventDisableDefenseGuild < 0)
		return FALSE;
	m_lEventNormalDefenseGuild	= m_pcsAgcmUIManager2->AddEvent("SiegeWarNormalDefenseGuild");
	if (m_lEventNormalDefenseGuild < 0)
		return FALSE;

	m_lEventOpenCarveMessageUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenCarveMessageUI");
	if (m_lEventOpenCarveMessageUI < 0)
		return FALSE;
	m_lEventCloseCarveMessageUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarCloseCarveMessageUI");
	if (m_lEventCloseCarveMessageUI < 0)
		return FALSE;

	//m_lEventOpenSiegeWarInfoUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenInfoUI");
	//if (m_lEventOpenSiegeWarInfoUI < 0)
	//	return FALSE;
	//m_lEventCloseSiegeWarInfoUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarCloseInfoUI");
	//if (m_lEventCloseSiegeWarInfoUI < 0)
	//	return FALSE;

	m_lEventGetSiegeWarInfoEditControl	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetInfoEditControl");
	if (m_lEventGetSiegeWarInfoEditControl < 0)
		return FALSE;
	m_lEventGetRemainTimeEditControl	= m_pcsAgcmUIManager2->AddEvent("SiegeWarGetRemainTimeEditControl");
	if (m_lEventGetRemainTimeEditControl < 0)
		return FALSE;

	m_lEventUseCharNameMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarUseCharNameMessage");
	if (m_lEventUseCharNameMessage < 0)
		return FALSE;
	m_lEventRepairCharNameMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarRepairCharNameMessage");
	if (m_lEventRepairCharNameMessage < 0)
		return FALSE;

	m_lEventOpenRepairBarUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarOpenRepairBarUI");
	if (m_lEventOpenRepairBarUI < 0)
		return FALSE;
	m_lEventCloseRepairBarUI	= m_pcsAgcmUIManager2->AddEvent("SiegeWarCloseRepairBarUI");
	if (m_lEventCloseRepairBarUI < 0)
		return FALSE;

	m_lEventConfirmApplGuildMessage	= m_pcsAgcmUIManager2->AddEvent("SiegeWarConfirmApplGuildMessage");
	if (m_lEventConfirmApplGuildMessage < 0)
		return FALSE;

	m_lEventStartSiegeWar = m_pcsAgcmUIManager2->AddEvent("SiegeWar_Start");
	if (m_lEventStartSiegeWar < 0)
		return FALSE;

	m_lEventCloseSiegeWar = m_pcsAgcmUIManager2->AddEvent("SiegeWar_Close");
	if (m_lEventCloseSiegeWar < 0)
		return FALSE;

	return TRUE;
}

BOOL AgcmUISiegeWar::AddFunction()
{
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetMainUIDescEdit", CBGetMainUIDescEdit, 1, "edit control"))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetMainUIWarTimeEdit", CBGetMainUISiegeWarTimeEdit, 1, "edit control"))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetAttackHelpEdit", CBGetAttackHelpEdit, 1, "edit control"))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetAttackSiegeWarTimeEdit", CBGetAttackSiegeWarTimeEdit, 1, "edit control"))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetDefenseHelpEdit", CBGetDefenseHelpEdit, 1, "edit control"))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetDefenseGuildInfoEdit", CBGetDefenseGuildInfoEdit, 1, "edit control"))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetDateCombo", CBGetDateCombo, 1, "date combo control"))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetHourCombo", CBGetHourCombo, 1, "hour combo control"))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarOpenSiegeWarTimeSetUI", CBOpenSiegeWarTimeSetUI, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarOpenAttackApplicationUI", CBOpenAttackApplicationUI, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarOpenDefenseApplicationUI", CBOpenDefenseApplicationUI, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarConfirmSiegeWarTime", CBConfirmSiegeWarTime, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarSelectDefenseGuild", CBSelectDefenseGuild, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarApplicationDefense", CBApplicationDefense, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarCancelDefense", CBCancelDefense, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarApplicationAttack", CBApplicationAttack, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarAttackPageLeft", CBAttackPageLeft, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarAttackPageRight", CBAttackPageRight, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarAttackGoPage", CBAttackGoPage, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarDefensePageLeft", CBDefensePageLeft, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarDefensekPageRight", CBDefensePageRight, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarDefenseGoPage", CBDefenseGoPage, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarRegisterDefenseGuild", CBRegisterDefenseGuild, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarDefenseGuildListSelect", CBDefenseGuildListSelect, 1, "list user data"))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetCatapultDescEdit", CBGetCatapultDescEdit, 1, "Catapult desc edit"))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetAtkResTowerDescEdit", CBGetAtkResTowerDescEdit, 1, "AtkResTower desc edit"))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarRepairCatapult", CBRepairCatapult, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarRepairAtkResTower", CBRepairAtkResTower, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarActiveCatapult", CBActiveCatapult, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarActiveAtkResTower", CBActiveAtkResTower, 0))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarCatapultDragDrop", CBCatapultDragDrop, 0))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarAtkResTowerDragDrop", CBAtkResTowerDragDrop, 0))
		return FALSE;

	/*if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetInfoEditControl", CBGetSiegeWarInfoEditControl, 1, "SiegeWarInfo Edit Control"))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarGetRemainTimeEditControl", CBGetRemainTimeEditControl, 1, "RemainTime Edit Control"))
		return FALSE;*/

	//if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarInfoScrollUp", CBSiegeWarInfoScrollUp, 0))
	//	return FALSE;
	//if (!m_pcsAgcmUIManager2->AddFunction(this, "SiegeWarInfoScrollDown", CBSiegeWarInfoScrollDown, 0))
	//	return FALSE;

	return TRUE;
}

BOOL AgcmUISiegeWar::AddUserData()
{
	m_pcsUserDataSiegeWar		= m_pcsAgcmUIManager2->AddUserData("SiegeWarInfo",
																	m_pcsSiegeWar,
																	sizeof(AgpdSiegeWar *),
																	1,
																	AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pcsUserDataSiegeWar)
		return FALSE;

	m_pcsUserDataDateCombo		= m_pcsAgcmUIManager2->AddUserData("SiegeWarDateCombo",
																	m_aszDateComboPointer,
																	sizeof(CHAR *),
																	AGCMUISIEGEWAR_MAX_SELECT_DATE,
																	AGCDUI_USERDATA_TYPE_STRING);
	if (!m_pcsUserDataDateCombo)
		return FALSE;

	m_pcsUserDataHourCombo		= m_pcsAgcmUIManager2->AddUserData("SiegeWarHourCombo",
																	m_aszHourComboPointer,
																	sizeof(CHAR *),
																	AGCMUISIEGEWAR_MAX_SELECT_HOUR,
																	AGCDUI_USERDATA_TYPE_STRING);
	if (!m_pcsUserDataHourCombo)
		return FALSE;

	m_pcsUserDataAttackList		= m_pcsAgcmUIManager2->AddUserData("SiegeWarAttackGuildList",
																	m_apstAttackApplGuildListPointer,
																	sizeof(AtttackGuildList *),
																	AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE,
																	AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pcsUserDataAttackList)
		return FALSE;

	m_pcsUserDataDefenseList		= m_pcsAgcmUIManager2->AddUserData("SiegeWarDefenseGuildList",
																	m_apstDefenseApplGuildListPointer,
																	sizeof(AtttackGuildList *),
																	AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE,
																	AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pcsUserDataDefenseList)
		return FALSE;

	m_pcsUserDataAttackCurrentPage	= m_pcsAgcmUIManager2->AddUserData("SiegeWarAttackCurrentPage",
																	&m_lAttackCurrentPage,
																	sizeof(INT32),
																	1,
																	AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pcsUserDataAttackCurrentPage)
		return FALSE;

	m_pcsUserDataDefenseCurrentPage	= m_pcsAgcmUIManager2->AddUserData("SiegeWarDefenseCurrentPage",
																	&m_lDefenseCurrentPage,
																	sizeof(INT32),
																	1,
																	AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pcsUserDataDefenseCurrentPage)
		return FALSE;

	m_pcsUserDataCatapultGrid	= m_pcsAgcmUIManager2->AddUserData("SiegeWarCatapultGrid",
																	m_pcsCatapultGrid,
																	sizeof(AgpdGrid),
																	1,
																	AGCDUI_USERDATA_TYPE_GRID);
	if (!m_pcsUserDataCatapultGrid)
		return FALSE;

	m_pcsUserDataAtkResTowerGrid	= m_pcsAgcmUIManager2->AddUserData("SiegeWarAtkResTowerGrid",
																	m_pcsAtkResTowerGrid,
																	sizeof(AgpdGrid),
																	1,
																	AGCDUI_USERDATA_TYPE_GRID);
	if (!m_pcsUserDataAtkResTowerGrid)
		return FALSE;



	m_pcsUserDataActiveCatapultRepairButton			= m_pcsAgcmUIManager2->AddUserData("SiegeWarActiveCatapultRepairButton",
																	&m_bIsActiveCatapultRepairButton,
																	sizeof(INT32),
																	1,
																	AGCDUI_USERDATA_TYPE_BOOL);
	if (!m_pcsUserDataActiveCatapultRepairButton)
		return FALSE;

	m_pcsUserDataActiveAtkResTowerRepairButton		= m_pcsAgcmUIManager2->AddUserData("SiegeWarActiveAtkResTowerRepairButton",
																	&m_bIsActiveAtkResTowerRepairButton,
																	sizeof(INT32),
																	1,
																	AGCDUI_USERDATA_TYPE_BOOL);
	if (!m_pcsUserDataActiveAtkResTowerRepairButton)
		return FALSE;

	m_pcsUserDataActiveCatapultActiveButton			= m_pcsAgcmUIManager2->AddUserData("SiegeWarActiveCatapultActiveButton",
																	&m_bIsActiveCatapultActiveButton,
																	sizeof(INT32),
																	1,
																	AGCDUI_USERDATA_TYPE_BOOL);
	if (!m_pcsUserDataActiveCatapultActiveButton)
		return FALSE;

	m_pcsUserDataActiveAtkResTowerActiveButton		= m_pcsAgcmUIManager2->AddUserData("SiegeWarActiveAtkResTowerActiveButton",
																	&m_bIsActiveAtkResTowerActiveButton,
																	sizeof(INT32),
																	1,
																	AGCDUI_USERDATA_TYPE_BOOL);
	if (!m_pcsUserDataActiveAtkResTowerActiveButton)
		return FALSE;

	m_pcsUserDataActiveAttackApplButton		= m_pcsAgcmUIManager2->AddUserData("SiegeWarActiveAttackApplButton",
																	&m_bIsActiveAttackApplButton,
																	sizeof(INT32),
																	1,
																	AGCDUI_USERDATA_TYPE_BOOL);
	if (!m_pcsUserDataActiveAttackApplButton)
		return FALSE;

	m_pcsUserDataUseCharName		= m_pcsAgcmUIManager2->AddUserData("SiegeWarUseCharName",
																	&m_lUseCharName,
																	sizeof(INT32),
																	1,
																	AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pcsUserDataUseCharName)
		return FALSE;

	m_pcsUserDataUseRepairBar		= m_pcsAgcmUIManager2->AddUserData("SiegeWarRepairBar",
																	&m_lRepairBar,
																	sizeof(INT32),
																	1,
																	AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pcsUserDataUseRepairBar)
		return FALSE;

	return TRUE;
}

BOOL AgcmUISiegeWar::AddDisplay()
{
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarNPCName", 1, CBDisplayNPCName, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarAttackGuildName", 1, CBDisplayAttackGuildName, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarAttackGuildMasterName", 1, CBDisplayAttackGuildMasterName, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarAttackGuildPoint", 1, CBDisplayAttackGuildPoint, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarDefenseGuildName", 1, CBDisplayDefenseGuildName, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarDefenseGuildMasterName", 1, CBDisplayDefenseGuildMasterName, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarDefenseGuildPoint", 1, CBDisplayDefenseGuildPoint, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarAttackPageNumber", 0, CBDisplayAttackPageNumber, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarDefensePageNumber", 0, CBDisplayDefensePageNumber, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarComboDate", 0, CBDisplayComboDate, AGCDUI_USERDATA_TYPE_STRING))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarComboHour", 0, CBDisplayComboHour, AGCDUI_USERDATA_TYPE_STRING))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarUseCharName", 0, CBDisplayUseCharacterName, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarRepairBarMax", 0, CBDisplayRepairBarMax, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "SiegeWarRepairBarCurrent", 0, CBDisplayRepairBarCurrent, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;

	return TRUE;
}

BOOL AgcmUISiegeWar::AddBoolean()
{
	if (!m_pcsAgcmUIManager2->AddBoolean(this, "SiegeWarActiveCatapultRepairButton", CBIsActiveCatapultRepairButton, AGCDUI_USERDATA_TYPE_BOOL))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddBoolean(this, "SiegeWarActiveCatapultActiveButton", CBIsActiveCatapultActiveButton, AGCDUI_USERDATA_TYPE_BOOL))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddBoolean(this, "SiegeWarActiveAtkResTowerRepairButton", CBIsActiveAtkResTowerRepairButton, AGCDUI_USERDATA_TYPE_BOOL))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddBoolean(this, "SiegeWarActiveAtkResTowerActiveButton", CBIsActiveAtkResTowerActiveButton, AGCDUI_USERDATA_TYPE_BOOL))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddBoolean(this, "SiegeWarActiveAttackApplButton", CBIsActiveAttackApplButton, AGCDUI_USERDATA_TYPE_BOOL))
		return FALSE;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBUpdateCastleInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis			= (AgcmUISiegeWar *)	pClass;
	AgpdSiegeWar	*pcsSiegeWar	= (AgpdSiegeWar *)		pData;

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataSiegeWar);
	return pThis->SetMainUIEdit();
}

BOOL AgcmUISiegeWar::CBReceiveResult(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUISiegeWar	*pThis			= (AgcmUISiegeWar *)	pClass;
	AgpdSiegeWar	*pcsSiegeWar	= (AgpdSiegeWar *)		pData;
	INT8			cResult			= *(INT8 *)				pCustData;

	switch (cResult)
	{
	case AGPMSIEGE_RESULT_NEED_CASTLE_MASTER:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireCastleMasterMessage);
		break;

	case AGPMSIEGE_RESULT_NEED_GUILD_MASTER:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireGuildMasterMessage);
		AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ONLY_GUILD_MASTER));
		break;

	case AGPMSIEGE_RESULT_NOTENOUGH_GUILD_CONDITION:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireGuildConditionMessage);
		break;

	case AGPMSIEGE_RESULT_ALREADY_APPLICATION:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventAlreadyApplGuildMessage);
		break;

	case AGPMSIEGE_RESULT_DEFENSE_APPL_SUCCESS:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventDefenseApplSuccessMessage);
		break;

	case AGPMSIEGE_RESULT_ATTACK_APPL_SUCCESS:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventAttackApplSuccessMessage);
		break;

	case AGPMSIEGE_RESULT_SET_DEFENSE_GUILD_SUCCESS:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventSetDefenseGuildSuccessMessage);
		break;

	case AGPMSIEGE_RESULT_SET_NEXT_SIEGE_WAR_TIME_SUCCESS:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventSetNextSiegeWarTimeSuccessMessage);
		break;

	case AGPMSIEGE_RESULT_START_CARVE_A_SEAL:
		// 각인이 시작된다.
		// 바 UI를 띄워주고, 각인이 시작되었다고 알려준다.
		pThis->m_pcsAgcmUICooldown->StartCooldown(pThis->m_lCooldownIndex, pcsSiegeWar->m_csTemplate.m_ulNeedTimeForCarveMSec);
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenCarveMessageUI);
		AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_START_CARVING));
		break;

	case AGPMSIEGE_RESULT_CANCEL_CARVE_A_SEAL:
		// 각인이 취소되었다.
		// 바 UI를 닫고, 취소되었다는 메시지를 날려준다.
		pThis->m_pcsAgcmUICooldown->CancelCooldown(pThis->m_lCooldownIndex);
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseCarveMessageUI);
		AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_CANCEL_CARVING));
		break;

	case AGPMSIEGE_RESULT_END_CARVE_A_SEAL:
		// 각인이 끝났다.
		// 각인이 완료되었다고 메시지를 날려준다.
		AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_END_CARVING));
		break;

	case AGPMSIEGE_RESULT_ALREADY_ACTIVE_OBJECT:
		AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ALREADY_ACTIVE));
		break;

	case AGPMSIEGE_RESULT_NOTENOUGH_MONEY:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventNotEnoughMoneyMessage);
		break;

	case AGPMSIEGE_RESULT_NO_MORE_REPAIR:
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventNoMoreRepairMessage);
		break;

	case AGPMSIEGE_RESULT_ALREADY_USED_ATTACK_OBJECT:
		AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ALREADY_USE));
		break;

	case AGPMSIEGE_RESULT_ALREADY_REPAIRED_ATTACK_OBJECT:
		AgcChatManager::OnAddSystemMessage(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ALREADY_REPAIR));
		break;
		
	case AGPMSIEGE_RESULT_REGISTRATION_EXCEEDED:
		pThis->m_pcsAgcmUIManager2->ActionMessageOKDialog(pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_REACHED_LIMITATION));
		break;
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::CBReceiveAttackApplGuildList(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	PVOID			*ppvGuildListPacket	= (PVOID *) pData;
	PVOID			*ppvBuffer	= (PVOID *)	pCustData;

	INT16	*pnPage		= (INT16 *)	ppvBuffer[0];
	INT16	*pnTotal	= (INT16 *)	ppvBuffer[1];

	if (!ppvGuildListPacket || !pnPage || !pnTotal)
		return FALSE;

	pThis->m_lAttackCurrentPage	= *pnPage;
	pThis->m_lAttackMaxPage		= *pnTotal;

	ZeroMemory(pThis->m_astAttackApplGuildList, sizeof(pThis->m_astAttackApplGuildList));

	for (int i = 0; i < AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE; ++i)
	{
		if (!ppvGuildListPacket[i])
			break;

		CHAR	*pszGuildName			= NULL;
		CHAR	*pszGuildMasterID		= NULL;

		UINT16	unGuildNameLength		= 0;
		UINT16	unGuildMasterIDLength	= 0;

		pThis->m_pcsAgpmSiegeWar->m_csPacketGuildInfo.GetField(FALSE, ppvGuildListPacket[i], 0,
											&pszGuildName, &unGuildNameLength,
											&pszGuildMasterID, &unGuildMasterIDLength,
											&pThis->m_astAttackApplGuildList[i].ulGuildPoint);

		if (pszGuildName && unGuildNameLength > 0 && unGuildNameLength <= AGPMGUILD_MAX_GUILD_ID_LENGTH)
			CopyMemory(pThis->m_astAttackApplGuildList[i].szGuildName, pszGuildName, sizeof(CHAR) * unGuildNameLength);
		if (pszGuildMasterID && unGuildMasterIDLength > 0 && unGuildMasterIDLength <= AGPDCHARACTER_MAX_ID_LENGTH)
			CopyMemory(pThis->m_astAttackApplGuildList[i].szGuildMasterName, pszGuildMasterID, sizeof(CHAR) * unGuildMasterIDLength);
	}

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataAttackCurrentPage);
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataAttackList);

	if (pThis->m_pcsSiegeWar && pThis->m_pcsSiegeWar->m_eCurrentStatus == AGPD_SIEGE_WAR_STATUS_PROCLAIM_WAR)
		pThis->m_bIsActiveAttackApplButton	= TRUE;
	else
		pThis->m_bIsActiveAttackApplButton	= FALSE;

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataActiveAttackApplButton);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBReceiveDefenseApplGuildList(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	PVOID			*ppvGuildListPacket	= (PVOID *) pData;
	PVOID			*ppvBuffer	= (PVOID *)	pCustData;

	INT16	*pnPage		= (INT16 *)	ppvBuffer[0];
	INT16	*pnTotal	= (INT16 *)	ppvBuffer[1];

	if (!ppvGuildListPacket || !pnPage || !pnTotal)
		return FALSE;

	pThis->m_lDefenseCurrentPage	= *pnPage;
	pThis->m_lDefenseMaxPage		= *pnTotal;

	ZeroMemory(pThis->m_astDefenseApplGuildList, sizeof(pThis->m_astDefenseApplGuildList));

	for (int i = 0; i < AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE; ++i)
	{
		if (!ppvGuildListPacket[i])
			break;

		CHAR	*pszGuildName			= NULL;
		CHAR	*pszGuildMasterID		= NULL;

		UINT16	unGuildNameLength		= 0;
		UINT16	unGuildMasterIDLength	= 0;

		pThis->m_pcsAgpmSiegeWar->m_csPacketGuildInfo.GetField(FALSE, ppvGuildListPacket[i], 0,
											&pszGuildName, &unGuildNameLength,
											&pszGuildMasterID, &unGuildMasterIDLength,
											&pThis->m_astDefenseApplGuildList[i].ulGuildPoint);

		if (pszGuildName && unGuildNameLength > 0 && unGuildNameLength <= AGPMGUILD_MAX_GUILD_ID_LENGTH)
			CopyMemory(pThis->m_astDefenseApplGuildList[i].szGuildName, pszGuildName, sizeof(CHAR) * unGuildNameLength);
		if (pszGuildMasterID && unGuildMasterIDLength > 0 && unGuildMasterIDLength <= AGPDCHARACTER_MAX_ID_LENGTH)
			CopyMemory(pThis->m_astDefenseApplGuildList[i].szGuildMasterName, pszGuildMasterID, sizeof(CHAR) * unGuildMasterIDLength);
	}

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataDefenseList);
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataDefenseCurrentPage);

	pThis->SetSelectedDefenseGuild();

	return TRUE;
}

BOOL AgcmUISiegeWar::CBReceiveAttackGuildList(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUISiegeWar	*pThis				= (AgcmUISiegeWar *)	pClass;
	PVOID			*ppvGuildListPacket	= (PVOID *)				pData;
	AgpdSiegeWar	*pcsSiegeWar		= (AgpdSiegeWar *)		pCustData;

	//if (pThis->m_pcsAgpmSiegeWar->IsAttackGuild(pThis->m_pcsAgcmCharacter->GetSelfCharacter(), pcsSiegeWar))
	//{
	//	if (pThis->m_pcsAgpmSiegeWar->IsStarted(pcsSiegeWar) &&
	//		pcsSiegeWar == pThis->m_pcsAgpmSiegeWar->GetSiegeWarInfo(pThis->m_pcsAgcmCharacter->GetSelfCharacter()))
	//	{
	//		if (!pThis->m_pcsSiegeWarInfoEdit)
	//		{
	//			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetSiegeWarInfoEditControl);
	//			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetRemainTimeEditControl);
	//		}

	//		//pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenSiegeWarInfoUI);
	//	}
	//}

	if (!pThis->m_pcsSiegeWar)
		return FALSE;

	ZeroMemory(pThis->m_astAttackGuildList, sizeof(pThis->m_astAttackGuildList));

	for (int i = 0; i < AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE; ++i)
	{
		if (!ppvGuildListPacket[i])
			break;

		CHAR	*pszGuildName			= NULL;
		CHAR	*pszGuildMasterID		= NULL;

		UINT16	unGuildNameLength		= 0;
		UINT16	unGuildMasterIDLength	= 0;

		pThis->m_pcsAgpmSiegeWar->m_csPacketGuildInfo.GetField(FALSE, ppvGuildListPacket[i], 0,
											&pszGuildName, &unGuildNameLength,
											&pszGuildMasterID, &unGuildMasterIDLength,
											&pThis->m_astAttackGuildList[i].ulGuildPoint);

		if (pszGuildName && unGuildNameLength > 0 && unGuildNameLength <= AGPMGUILD_MAX_GUILD_ID_LENGTH)
			CopyMemory(pThis->m_astAttackGuildList[i].szGuildName, pszGuildName, sizeof(CHAR) * unGuildNameLength);
		if (pszGuildMasterID && unGuildMasterIDLength > 0 && unGuildMasterIDLength <= AGPDCHARACTER_MAX_ID_LENGTH)
			CopyMemory(pThis->m_astAttackGuildList[i].szGuildMasterName, pszGuildMasterID, sizeof(CHAR) * unGuildMasterIDLength);
	}

	//INT32 lMode = 0;
	//pThis->EnumCallback(AGCMUISIEGEWAR_CB_RECEIVE_ATTACK_GUILD_LIST, NULL, &lMode);

	for (int i = 0; i < AGPMSIEGEWAR_TOTAL_ATTACK_GUILD; ++i)
		pThis->m_pcsAgcmTextBoard->PvPRefreshSiege(pThis->m_astAttackGuildList[i].szGuildName);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBReceiveDefenseGuildList(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUISiegeWar	*pThis				= (AgcmUISiegeWar *)	pClass;
	PVOID			*ppvGuildListPacket	= (PVOID *)				pData;
	AgpdSiegeWar	*pcsSiegeWar		= (AgpdSiegeWar *)		pCustData;

	//if (pThis->m_pcsAgpmSiegeWar->IsDefenseGuild(pThis->m_pcsAgcmCharacter->GetSelfCharacter(), pcsSiegeWar))
	//{
	//	if (pThis->m_pcsAgpmSiegeWar->IsStarted(pcsSiegeWar) &&
	//		pcsSiegeWar == pThis->m_pcsAgpmSiegeWar->GetSiegeWarInfo(pThis->m_pcsAgcmCharacter->GetSelfCharacter()))
	//	{
	//		if (!pThis->m_pcsSiegeWarInfoEdit)
	//		{
	//			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetSiegeWarInfoEditControl);
	//			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetRemainTimeEditControl);
	//		}

	//		//pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenSiegeWarInfoUI);
	//	}
	//}

	if (!pThis->m_pcsSiegeWar)
		return FALSE;

	ZeroMemory(pThis->m_astDefenseGuildList, sizeof(pThis->m_astDefenseGuildList));

	for (int i = 0; i < AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE; ++i)
	{
		if (!ppvGuildListPacket[i])
			break;

		CHAR	*pszGuildName			= NULL;
		CHAR	*pszGuildMasterID		= NULL;

		UINT16	unGuildNameLength		= 0;
		UINT16	unGuildMasterIDLength	= 0;

		pThis->m_pcsAgpmSiegeWar->m_csPacketGuildInfo.GetField(FALSE, ppvGuildListPacket[i], 0,
											&pszGuildName, &unGuildNameLength,
											&pszGuildMasterID, &unGuildMasterIDLength,
											&pThis->m_astDefenseGuildList[i].ulGuildPoint);

		if (pszGuildName && unGuildNameLength > 0 && unGuildNameLength <= AGPMGUILD_MAX_GUILD_ID_LENGTH)
			CopyMemory(pThis->m_astDefenseGuildList[i].szGuildName, pszGuildName, sizeof(CHAR) * unGuildNameLength);
		if (pszGuildMasterID && unGuildMasterIDLength > 0 && unGuildMasterIDLength <= AGPDCHARACTER_MAX_ID_LENGTH)
			CopyMemory(pThis->m_astDefenseGuildList[i].szGuildMasterName, pszGuildMasterID, sizeof(CHAR) * unGuildMasterIDLength);
	}

	pThis->SetSiegeWarDefenseGuildInfoEdit();

	//INT32 lMode = 1;
	//pThis->EnumCallback(AGCMUISIEGEWAR_CB_RECEIVE_ENEMY_GUILD_LIST, NULL, &lMode);

	for (int i = 0; i < AGPMSIEGEWAR_TOTAL_ATTACK_GUILD; ++i)
		pThis->m_pcsAgcmTextBoard->PvPRefreshSiege(pThis->m_astAttackGuildList[i].szGuildName);

	pThis->SetSelectedDefenseGuild();

	return TRUE;
}

BOOL AgcmUISiegeWar::OpenCatapultUI(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdSiegeWarADCharacter	*pcsAttachData	= m_pcsAgpmSiegeWar->GetAttachCharacterData(pcsCharacter);

	if (m_pcsCatapultDescEdit)
	{
		m_pcsCatapultDescEdit->ClearText();
		m_pcsCatapultDescEdit->SetLineDelimiter("\n");

		CHAR	szBuffer[1024];
		ZeroMemory(szBuffer, sizeof(szBuffer));

		CHAR	szRepairCount[32];
		CHAR	szRepairCost[32];
		CHAR	szRepairDuration[32];

		ZeroMemory(szRepairCount, sizeof(szRepairCount));
		ZeroMemory(szRepairCost, sizeof(szRepairCost));
		ZeroMemory(szRepairDuration, sizeof(szRepairDuration));

		// 필요하다면 <Cxxxxxxxx>로 칼라를 넣는다.
		sprintf(szRepairCount, "%d", pcsAttachData->ucRepairCount);
		sprintf(szRepairCost, "%d", pcsAttachData->lRepairCost);
		sprintf(szRepairDuration, "%d", pcsAttachData->ucRepairDuration);

		sprintf(szBuffer,
				m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_CATAPULT_DESC),
				szRepairCount,
				szRepairCost,
				szRepairDuration);

		m_pcsCatapultDescEdit->SetText(szBuffer);
	}

	m_pcsCatapultGrid				= pcsAttachData->m_pcsAttackObjectGrid;

	m_pcsUserDataCatapultGrid->m_stUserData.m_pvData	= m_pcsCatapultGrid;

	if (!m_pcsAgpmCharacter->IsUseCharacter(pcsCharacter) && m_pcsAgpmSiegeWar->CheckValidUseAttackObject(m_pcsAgcmCharacter->GetSelfCharacter(), pcsCharacter) == AGPMSIEGE_RESULT_SUCCESS)
		m_bIsActiveCatapultActiveButton	= TRUE;
	else
		m_bIsActiveCatapultActiveButton	= FALSE;

	if (!m_pcsAgpmCharacter->IsRepairCharacter(pcsCharacter) && m_pcsAgpmSiegeWar->CheckValidRepairAttackObject(m_pcsAgcmCharacter->GetSelfCharacter(), pcsCharacter) == AGPMSIEGE_RESULT_SUCCESS)
		m_bIsActiveCatapultRepairButton	= TRUE;
	else
		m_bIsActiveCatapultRepairButton	= FALSE;

	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsUserDataCatapultGrid);
	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsUserDataActiveCatapultRepairButton);
	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsUserDataActiveCatapultActiveButton);

	m_stOpenUIPos	= m_pcsAgcmCharacter->GetSelfCharacter()->m_stPos;
	m_bOpenUI		= TRUE;

	return m_pcsAgcmUIManager2->ThrowEvent(m_lEventOpenCatapultUI);
}

BOOL AgcmUISiegeWar::OpenAtkResTowerUI(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return FALSE;

	AgpdSiegeWarADCharacter	*pcsAttachData	= m_pcsAgpmSiegeWar->GetAttachCharacterData(pcsCharacter);

	if (m_pcsAtkResTowerDescEdit)
	{
		m_pcsAtkResTowerDescEdit->ClearText();
		m_pcsAtkResTowerDescEdit->SetLineDelimiter("\n");

		CHAR	szBuffer[1024];
		ZeroMemory(szBuffer, sizeof(szBuffer));

		CHAR	szRepairCount[32];
		CHAR	szRepairCost[32];
		CHAR	szRepairDuration[32];

		ZeroMemory(szRepairCount, sizeof(szRepairCount));
		ZeroMemory(szRepairCost, sizeof(szRepairCost));
		ZeroMemory(szRepairDuration, sizeof(szRepairDuration));

		// 필요하다면 <Cxxxxxxxx>로 칼라를 넣는다.
		sprintf(szRepairCount, "%d", pcsAttachData->ucRepairCount);
		sprintf(szRepairCost, "%d", pcsAttachData->lRepairCost);
		sprintf(szRepairDuration, "%d", pcsAttachData->ucRepairDuration);

		sprintf(szBuffer,
				m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_ATK_RES_TOWER_DESC),
				szRepairCount,
				szRepairCost,
				szRepairDuration);

		m_pcsAtkResTowerDescEdit->SetText(szBuffer);
	}

	m_pcsAtkResTowerGrid				= pcsAttachData->m_pcsAttackObjectGrid;

	m_pcsUserDataAtkResTowerGrid->m_stUserData.m_pvData	= m_pcsAtkResTowerGrid;

	if (!m_pcsAgpmCharacter->IsUseCharacter(pcsCharacter) && m_pcsAgpmSiegeWar->CheckValidUseAttackObject(m_pcsAgcmCharacter->GetSelfCharacter(), pcsCharacter) == AGPMSIEGE_RESULT_SUCCESS)
		m_bIsActiveAtkResTowerActiveButton	= TRUE;
	else
		m_bIsActiveAtkResTowerActiveButton	= FALSE;

	if (!m_pcsAgpmCharacter->IsRepairCharacter(pcsCharacter) && m_pcsAgpmSiegeWar->CheckValidRepairAttackObject(m_pcsAgcmCharacter->GetSelfCharacter(), pcsCharacter) == AGPMSIEGE_RESULT_SUCCESS)
		m_bIsActiveAtkResTowerRepairButton	= TRUE;
	else
		m_bIsActiveAtkResTowerRepairButton	= FALSE;

	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsUserDataAtkResTowerGrid);
	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsUserDataActiveAtkResTowerRepairButton);
	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsUserDataActiveAtkResTowerActiveButton);

	m_stOpenUIPos	= m_pcsAgcmCharacter->GetSelfCharacter()->m_stPos;
	m_bOpenUI		= TRUE;

	return m_pcsAgcmUIManager2->ThrowEvent(m_lEventOpenAtkResTowerUI);
}

BOOL AgcmUISiegeWar::CBResponseUseAttackObject(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgpdCharacter	*pcsTarget	= (AgpdCharacter *)	pData;
	INT8			*pcResult	= (INT8 *)			pCustData;

	if (*pcResult != AGPMSIEGE_RESULT_SUCCESS &&
		*pcResult != AGPMSIEGE_RESULT_OPEN_ATTACK_OBJECT_EVENT)
	{
		// 경고 메시지를 띄워줘야 하나?
		//
		//
		//

		return TRUE;
	}

	pThis->m_pcsTarget	= pcsTarget;

	if (*pcResult == AGPMSIEGE_RESULT_OPEN_ATTACK_OBJECT_EVENT)
	{
		// pcsTarget의 공성타입에 따라 해당되는 UI를 띄워준다.

		AgpdSiegeWarMonsterType	eMonsterType	= pThis->m_pcsAgpmSiegeWar->GetSiegeWarMonsterType(pcsTarget);

		switch (eMonsterType) {
			case AGPD_SIEGE_MONSTER_CATAPULT:
				{
					pThis->OpenCatapultUI(pcsTarget);
				}
				break;

			case AGPD_SIEGE_MONSTER_ATTACK_RESURRECTION_TOWER:
				{
					pThis->OpenAtkResTowerUI(pcsTarget);
				}
				break;
		}
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::CBAddItemResult(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	//AgpdItem		*pItem  = (AgpdItem*) pData[2];


	if (pThis->m_pcsAgpmSiegeWar->CheckValidUseAttackObject(pThis->m_pcsAgcmCharacter->GetSelfCharacter(), pThis->m_pcsTarget) == AGPMSIEGE_RESULT_SUCCESS)
	{
		pThis->m_bIsActiveCatapultActiveButton	= TRUE;
		pThis->m_bIsActiveAtkResTowerActiveButton	= TRUE;
	}
	else
	{
		pThis->m_bIsActiveCatapultActiveButton	= FALSE;
		pThis->m_bIsActiveAtkResTowerActiveButton	= FALSE;
	}

	//m_pcsAgpmGrid->AddItem(pThis->m_pcsUserDataCatapultGrid, pcsItem->m_pcsGridItem);

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataActiveCatapultActiveButton);
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataActiveAtkResTowerActiveButton);

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataCatapultGrid);
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataAtkResTowerGrid);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBStatusInfo(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	PVOID			pvPacketStatusInfo	= (PVOID)	pData;

	UINT8	ucStatusType			= 0;

	CHAR	*pszCastleName			= NULL;
	CHAR	*pszGuildName			= NULL;
	CHAR	*pszGuildMasterName		= NULL;

	UINT16	unCastleNameLength		= 0;
	UINT16	unGuildNameLength		= 0;
	UINT16	unGuildMasterNameLength	= 0;

	UINT8	ucObjectType			= AGPD_SIEGE_MONSTER_TYPE_NONE;
	UINT32	ulTimeSec				= 0;
	UINT8	ucActiveCount			= 0;

	pThis->m_pcsAgpmSiegeWar->m_csPacketStatusInfo.GetField(FALSE, pvPacketStatusInfo, 0,
						&ucStatusType,
						&pszCastleName, &unCastleNameLength,
						&pszGuildName, &unGuildNameLength,
						&pszGuildMasterName, &unGuildMasterNameLength,
						&ucObjectType,
						&ulTimeSec,
						&ucActiveCount);

	CHAR	szCastleName[AGPMSIEGEWAR_MAX_CASTLE_NAME + 1];
	CHAR	szGuildName[AGPMGUILD_MAX_GUILD_ID_LENGTH + 1];
	CHAR	szGuildMasterName[AGPDCHARACTER_MAX_ID_LENGTH + 1];
	
	ZeroMemory(szCastleName, sizeof(szCastleName));
	ZeroMemory(szGuildName, sizeof(szGuildName));
	ZeroMemory(szGuildMasterName, sizeof(szGuildMasterName));

	if (szCastleName && unCastleNameLength > 0 && unCastleNameLength <= AGPMSIEGEWAR_MAX_CASTLE_NAME)
		CopyMemory(szCastleName, pszCastleName, unCastleNameLength);
	if (szGuildName && unGuildNameLength > 0 && unGuildNameLength <= AGPMGUILD_MAX_GUILD_ID_LENGTH)
		CopyMemory(szGuildName, pszGuildName, unGuildNameLength);
	if (szGuildMasterName && unGuildMasterNameLength > 0 && unGuildMasterNameLength <= AGPDCHARACTER_MAX_ID_LENGTH)
		CopyMemory(szGuildMasterName, pszGuildMasterName, unGuildMasterNameLength);

	AgpdSiegeWar	*pcsSiegeWar	= pThis->m_pcsAgpmSiegeWar->GetSiegeWarInfo(szCastleName);
	if( !pcsSiegeWar ) return FALSE;

	CHAR szBuffer[256] = {0, };

	switch ((AgpmSiegeStatusType) ucStatusType) {
		case AGPMSIEGE_STATUS_START:
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventStartSiegeWar);
			sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_START), szCastleName);
			pcsSiegeWar->m_ulSiegeWarStartTimeMSec	= pThis->GetClockCount();
			break;

		case AGPMSIEGE_STATUS_REMAIN_TIME_TO_START:
			break;

		case AGPMSIEGE_STATUS_REMAIN_TIME_TO_TERMINATE:
			break;

		case AGPMSIEGE_STATUS_DESTROY_OBJECT:
			switch ((AgpdSiegeWarMonsterType) ucObjectType) {
				case AGPD_SIEGE_MONSTER_GUARD_TOWER:
					// %s성 %s 파괴, %s destroyed for %s castle
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_GUARD_TOWER));
					else
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), szCastleName, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_GUARD_TOWER));
					break;
				case AGPD_SIEGE_MONSTER_INNER_GATE:
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_INNER_GATE));
					else
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), szCastleName, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_INNER_GATE));
					break;
				case AGPD_SIEGE_MONSTER_OUTER_GATE:
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_OUTER_GATE));
					else
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), szCastleName, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_OUTER_GATE));
					break;
				case AGPD_SIEGE_MONSTER_LIFE_TOWER:
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_LIFE_TOWER));
					else
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), szCastleName, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_LIFE_TOWER));
					break;
				case AGPD_SIEGE_MONSTER_DEFENSE_RESURRECTION_TOWER:
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_DEFENSE_RES_TOWER));
					else
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), szCastleName, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_DEFENSE_RES_TOWER));
					break;
				case AGPD_SIEGE_MONSTER_ATTACK_RESURRECTION_TOWER:
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_ATTACK_RES_TOWER));
					else
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), szCastleName, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_ATTACK_RES_TOWER));
					break;
				case AGPD_SIEGE_MONSTER_CATAPULT:
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_CATAPULT));
					else
						sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DESTROY_OBJECT), szCastleName, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_OBJECT_CATAPULT));
					break;
			}
			break;

		case AGPMSIEGE_STATUS_RELEASE_THRONE:
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				strncpy_s( szBuffer, 256, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_RELEASE_THRONE), _TRUNCATE );
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_RELEASE_THRONE), szCastleName);
			break;

		case AGPMSIEGE_STATUS_ACTIVE_ARCHON_EYE:
			if (ucActiveCount == 1)
			{
				if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
					strncpy_s( szBuffer, 256, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_ACTIVE_ARCHON_RIGHT_EYE), _TRUNCATE );
				else
					sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_ACTIVE_ARCHON_RIGHT_EYE), szCastleName);
			}
			else if (ucActiveCount == 0)
			{
				if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
					strncpy_s( szBuffer, 256, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_ACTIVE_ARCHON_LEFT_EYE), _TRUNCATE );
				else
					sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_ACTIVE_ARCHON_LEFT_EYE), szCastleName);
			}
			break;

		case AGPMSIEGE_STATUS_FAILED_ACTIVE_ARCHON_EYE:
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				strncpy_s( szBuffer, 256, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_FAILED_ACTIVE_ARCHON_EYE), _TRUNCATE );
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_FAILED_ACTIVE_ARCHON_EYE), szCastleName);
			break;

		case AGPMSIEGE_STATUS_START_CARVE_GUILD:
			if (pcsSiegeWar)
			{
				pThis->m_pcsAgcmUICooldown->StartCooldown(pThis->m_lCooldownIndex, pcsSiegeWar->m_csTemplate.m_ulNeedTimeForCarveMSec);
				pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenCarveMessageUI);
			}
			// %s성 %s 길드 각인 시작, %s guild in %s castle begins stamping
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_START_CARVE_GUILD), szGuildName);
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_START_CARVE_GUILD), szCastleName, szGuildName);
			break;

		case AGPMSIEGE_STATUS_CANCEL_CARVE_GUILD:
			pThis->m_pcsAgcmUICooldown->CancelCooldown(pThis->m_lCooldownIndex);
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseCarveMessageUI);
			// %s성 %s 길드 각인 취소, %s guild in %s castle cancels stamping
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_CANCEL_CARVE_GUILD), szGuildName);
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_CANCEL_CARVE_GUILD), szCastleName, szGuildName);
			break;

		case AGPMSIEGE_STATUS_NEW_CARVE_GUILD:
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_NEW_CARVE_GUILD), szGuildName);
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_NEW_CARVE_GUILD), szCastleName, szGuildName);

			if (pThis->m_pcsAgpmSiegeWar->IsArchlordCastle(pcsSiegeWar))
			{
				// 5분 후 최종전 시작. 경과 시간을 조정할 수 없으니 시작 시간을 조정한다.
				pcsSiegeWar->m_ulSiegeWarStartTimeMSec = pThis->GetClockCount() - pThis->m_pcsAgpmSiegeWar->GetDurationMSec(pcsSiegeWar) + AGPMSIEGEWAR_ARCHLORD_2STEP_WAIT_INTERVAL;
				sprintf(szBuffer + strlen(szBuffer), ". %s", pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_STOP_BATTLE_UNTIL_END));
			}
			break;

		case AGPMSIEGE_STATUS_TERMINATE:
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseSiegeWar);
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_TERMINATE));
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_TERMINATE), szCastleName);
			break;

		case AGPMSIEGE_STATUS_ACTIVE_LEFT_ATTACK_RES_TOWER:
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				strncpy_s( szBuffer, 256, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_ACTIVE_LEFT_ATTACK_RES_TOWER), _TRUNCATE );
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_ACTIVE_LEFT_ATTACK_RES_TOWER), szCastleName);
			break;

		case AGPMSIEGE_STATUS_ACTIVE_RIGHT_ATTACK_RES_TOWER:
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				strncpy_s( szBuffer, 256, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_ACTIVE_RIGHT_ATTACK_RES_TOWER), _TRUNCATE );
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_ACTIVE_RIGHT_ATTACK_RES_TOWER), szCastleName);
			break;

		case AGPMSIEGE_STATUS_DISABLE_LEFT_ATTACK_RES_TOWER:
			sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DISABLE_LEFT_ATTACK_RES_TOWER), szCastleName);
			break;

		case AGPMSIEGE_STATUS_DISABLE_RIGHT_ATTACK_RES_TOWER:
			sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_DISABLE_RIGHT_ATTACK_RES_TOWER), szCastleName);
			break;

		case AGPMSIEGE_STATUS_REPAIR_START_LEFT_ATTACK_RES_TOWER:
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
			{
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_START_REPAIR),
							  pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_LEFT),
							  pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ATKRESTOWER));
			}
			else
			{
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_START_REPAIR),
							  szCastleName,
							  pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_LEFT),
							  pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ATKRESTOWER));
			}
			break;

		case AGPMSIEGE_STATUS_REPAIR_START_RIGHT_ATTACK_RES_TOWER:
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL )
			{
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_START_REPAIR),
							  pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_RIGHT),
							  pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ATKRESTOWER));
			}
			else
			{
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_START_REPAIR),
							  szCastleName,
							  pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_RIGHT),
							  pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ATKRESTOWER));
			}
			break;
		case AGPMSIEGE_STATUS_SET_CASTLE_OWNER:
			sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_SET_CASTLE_OWNER), szGuildName, szGuildMasterName, szCastleName);
			break;

		case AGPMSIEGE_STATUS_ARCHON_EYE_ALL_OPEN:
		
			if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL )
				strncpy_s( szBuffer, 256, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_OPEN_ALL_EYES), _TRUNCATE );
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_OPEN_ALL_EYES), szCastleName);

			break;

	}


	if (strlen(szBuffer) > 0)
	{
		pThis->_SelfPrintShrineMessageChat(szBuffer);
		AgcChatManager::OnAddSystemMessage(szBuffer);
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::CBSyncMessage(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis				= (AgcmUISiegeWar *)	pClass;
	PVOID			pvPacketSyncMessage	= (PVOID)				pData;

	UINT8	ucMessageType	= 0;
	UINT16	unTime			= 0;

	pThis->m_pcsAgpmSiegeWar->m_csPacketSyncMessage.GetField(FALSE, pvPacketSyncMessage, 0,
					&ucMessageType,
					&unTime);

	CHAR	szBuffer[128];
	ZeroMemory(szBuffer, sizeof(szBuffer));

	switch ((AgpmSiegeMessage) ucMessageType) {
		case AGPMSIEGE_MESSAGE_SYNC_START_TIME:
			if (unTime > 0)
			{
				if (unTime >= 60)
				{
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, "%s %d%s", pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_START_TIME), unTime / 60, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_MINUTE));
					else
						sprintf(szBuffer, "%d%s%s", unTime / 60, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_MINUTE), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_START_TIME));
				}
				else
				{
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, "%s %d%s", pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_START_TIME), unTime, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_SECOND));
					else
						sprintf(szBuffer, "%d%s%s", unTime, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_SECOND), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_START_TIME));
				}
			}
			break;

		case AGPMSIEGE_MESSAGE_SYNC_END_TIME:
			if (unTime > 0)
			{

				if (unTime >= 60)
				{
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, "%s %d%s", pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_END_TIME), unTime / 60, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_MINUTE));
					else
						sprintf(szBuffer, "%d%s%s", unTime / 60, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_MINUTE), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_END_TIME));
				}
				else
				{
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, "%s %d%s", pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_END_TIME), unTime, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_SECOND));
					else
						sprintf(szBuffer, "%d%s%s", unTime, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_SECOND), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_END_TIME));
				}

				AgpdSiegeWar	*pcsSiegeWar	= pThis->m_pcsAgpmSiegeWar->GetSiegeWarInfo(pThis->m_pcsAgcmCharacter->GetSelfCharacter());
				if (pcsSiegeWar)
				{
#ifdef	__TEST_SERVER_ONLY__
					pcsSiegeWar->m_ulSiegeWarStartTimeMSec	= pThis->GetClockCount() - (3600000 - unTime * 1000);
#else	
					pcsSiegeWar->m_ulSiegeWarStartTimeMSec	= pThis->GetClockCount() - (pThis->m_pcsAgpmSiegeWar->GetDurationMSec(pcsSiegeWar) - unTime * 1000);
#endif	//__TEST_SERVER_ONLY__
				}
			}
			break;
		case AGPMSIEGE_MESSAGE_SYNC_START_ARCHLORDBATTLE_TIME:
			if(unTime > 0)
			{
				if (unTime >= 60)
				{
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, "%s %d%s", pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ARCHLORDBATTLE_START_TIME), unTime / 60, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_MINUTE));
					else
						sprintf(szBuffer, "%d%s%s", unTime / 60, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_MINUTE), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ARCHLORDBATTLE_START_TIME));
				}
				else
				{
					if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
						sprintf(szBuffer, "%s %d%s", pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ARCHLORDBATTLE_START_TIME), unTime, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_SECOND));
					else
						sprintf(szBuffer, "%d%s%s", unTime, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_SECOND), pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MESSAGE_ARCHLORDBATTLE_START_TIME));
				}

				AgpdSiegeWar	*pcsSiegeWar	= pThis->m_pcsAgpmSiegeWar->GetSiegeWarInfo(pThis->m_pcsAgcmCharacter->GetSelfCharacter());
				if (pcsSiegeWar)
				{
#ifdef	__TEST_SERVER_ONLY__
					pcsSiegeWar->m_ulSiegeWarStartTimeMSec	= pThis->GetClockCount() - (3600000 - unTime * 1000);
#else	
					pcsSiegeWar->m_ulSiegeWarStartTimeMSec	= pThis->GetClockCount() - (pThis->m_pcsAgpmSiegeWar->GetDurationMSec(pcsSiegeWar) - unTime * 1000);
#endif	//__TEST_SERVER_ONLY__
				}
			}
			break;
	}

	if (strlen(szBuffer) > 0)
	{
		pThis->SetTextEffect(szBuffer);
		AgcChatManager::OnAddSystemMessage(szBuffer);
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::CBUpdateStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUISiegeWar	*pThis			= (AgcmUISiegeWar *)	pClass;
	AgpdSiegeWar	*pcsSiegeWar	= (AgpdSiegeWar *)		pData;
	AgpdSiegeWarStatus	eOldStatus	= (AgpdSiegeWarStatus) (*(INT32 *) pCustData);

	//pThis->UpdateSiegeWarInfoUI( pcsSiegeWar, TRUE );

	if (eOldStatus == pcsSiegeWar->m_eCurrentStatus)
		return TRUE;

	CHAR	szBuffer[256];
	ZeroMemory(szBuffer, sizeof(szBuffer));

	switch (pcsSiegeWar->m_eCurrentStatus) {
		case AGPD_SIEGE_WAR_STATUS_OPEN_EYES:
			/*if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
				strncpy_s( szBuffer, 256, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_OPEN_ALL_EYES), _TRUNCATE );
			else
				sprintf(szBuffer, pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_STATUS_OPEN_ALL_EYES), pcsSiegeWar->m_strCastleName.GetBuffer());*/
			break;

		case AGPD_SIEGE_WAR_STATUS_TIME_OVER:
			pThis->m_pcsAgcmUICooldown->CancelCooldown(pThis->m_lCooldownIndex);
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseCarveMessageUI);
			break;
			
		case AGPD_SIEGE_WAR_STATUS_ARCHLORD_BATTLE:		// 결정전 3단계 시작시 시작시간 조정.
			pcsSiegeWar->m_ulSiegeWarStartTimeMSec	= pThis->GetClockCount() - (pThis->m_pcsAgpmSiegeWar->GetDurationMSec(pcsSiegeWar) * 1000);
			break;
	}

	if (strlen(szBuffer) > 0)
	{
		pThis->SetTextEffect(szBuffer);
		AgcChatManager::OnAddSystemMessage(szBuffer);
	}

	pThis->m_bIsActiveAttackApplButton = AGPD_SIEGE_WAR_STATUS_PROCLAIM_WAR == pcsSiegeWar->m_eCurrentStatus ? TRUE : FALSE;

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataActiveAttackApplButton);

	return TRUE;
}

// 2006.06.20. steeple
// pcsCharacter 의 길드가 공성전 중이고, pcsTarget 의 길드가 같은 편일때만 return TRUE
// 아니면 return FALSE
BOOL AgcmUISiegeWar::CBIsFriendGuild(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUISiegeWar*	pThis = static_cast<AgcmUISiegeWar*>(pClass);
	AgpdCharacter*	pcsCharacter = static_cast<AgpdCharacter*>(pData);
	AgpdCharacter*	pcsTarget = static_cast<AgpdCharacter*>(pCustData);

	CHAR* szGuildID1 = pThis->m_pcsAgpmGuild->GetJoinedGuildID(pcsCharacter);
	CHAR* szGuildID2 = pThis->m_pcsAgpmGuild->GetJoinedGuildID(pcsTarget);
	if(!szGuildID1 || !szGuildID2 || strlen(szGuildID1) < 1 || strlen(szGuildID2) < 1)
		return FALSE;

	if((pThis->IsInAttackGuild(szGuildID1) && pThis->IsInAttackGuild(szGuildID2)) ||
		(pThis->IsInDefenseGuild(szGuildID1) && pThis->IsInDefenseGuild(szGuildID2)))
		return TRUE;

	return FALSE;
}

// 2006.06.20. steeple
// pcsCharacter 의 길드가 공성전 중이고, pcsTarget 의 길드가 적일때만 return TRUE
// 아니면 return FALSE
BOOL AgcmUISiegeWar::CBIsEnemyGuild(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgcmUISiegeWar*	pThis = static_cast<AgcmUISiegeWar*>(pClass);
	AgpdCharacter*	pcsCharacter = static_cast<AgpdCharacter*>(pData);
	AgpdCharacter*	pcsTarget = static_cast<AgpdCharacter*>(pCustData);

	CHAR* szGuildID1 = pThis->m_pcsAgpmGuild->GetJoinedGuildID(pcsCharacter);
	CHAR* szGuildID2 = pThis->m_pcsAgpmGuild->GetJoinedGuildID(pcsTarget);
	if(!szGuildID1 || !szGuildID2 || strlen(szGuildID1) < 1 || strlen(szGuildID2) < 1)
		return FALSE;

	if((pThis->IsInAttackGuild(szGuildID1) && pThis->IsInDefenseGuild(szGuildID2)) ||
		(pThis->IsInDefenseGuild(szGuildID1) && pThis->IsInAttackGuild(szGuildID2)))
		return TRUE;

	return FALSE;
}

BOOL AgcmUISiegeWar::CBEventGrant(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	ApdEvent		*pcsEvent	= (ApdEvent *)			pData;
	AgcmUISiegeWar	*pThis		= (AgcmUISiegeWar *)	pClass;

	pThis->m_pcsEvent		= pcsEvent;

	pThis->m_pcsSiegeWar	= pThis->m_pcsAgpmEventSiegeWarNPC->GetSiegeWarInfo(pcsEvent);
	if (!pThis->m_pcsSiegeWar)
		return FALSE;

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataSiegeWar);
	pThis->SetMainUIEdit();

	pThis->m_stOpenUIPos	= pThis->m_pcsAgcmCharacter->GetSelfCharacter()->m_stPos;
	pThis->m_bOpenUI	= TRUE;

	// UI를 열어준다.
	/////////////////////////////////////////////////////////////////////////
	return pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenSiegeWarMainUI);
}

BOOL AgcmUISiegeWar::CBSetSelfCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	pThis->m_pcsEvent		= NULL;
	pThis->m_pcsSiegeWar	= NULL;

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetMainUIDescEdit);
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetMainUISiegeWarTimeEdit);

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetAttackHelpEdit);
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetAttackSiegeWarTimeEdit);

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetDefenseHelpEdit);
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetDefenseGuidInfoEdit);

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetDateCombo);
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetHourCombo);

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetCatapultEdit);
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventGetAtkResTowerEdit);

	pThis->m_pcsAgcmSiegeWar->SetCatapultUseTexture(pThis->m_pcsAgcmUIControl->m_pCatapultUse);
	pThis->m_pcsAgcmSiegeWar->SetCatapultRepairTexture(pThis->m_pcsAgcmUIControl->m_pCatapultRepair);

	pThis->m_pcsAgcmSiegeWar->SetAtkResTowerUseTexture(pThis->m_pcsAgcmUIControl->m_pAtkResTowerUse);
	pThis->m_pcsAgcmSiegeWar->SetAtkResTowerRepairTexture(pThis->m_pcsAgcmUIControl->m_pAtkResTowerRepair);

	pThis->m_pcsAgcmSiegeWar->SetCastleOwnerTexture(pThis->m_pcsAgcmUIControl->m_pCastleOwnerHuman, AGPMSIEGEWAR_HUMAN_CASTLE_INDEX);
	pThis->m_pcsAgcmSiegeWar->SetCastleOwnerTexture(pThis->m_pcsAgcmUIControl->m_pCastleOwnerOrc, AGPMSIEGEWAR_ORC_CASTLE_INDEX);
	pThis->m_pcsAgcmSiegeWar->SetCastleOwnerTexture(pThis->m_pcsAgcmUIControl->m_pCastleOwnerMoonelf, AGPMSIEGEWAR_MOONELF_CASTLE_INDEX);
	pThis->m_pcsAgcmSiegeWar->SetCastleOwnerTexture(pThis->m_pcsAgcmUIControl->m_pCastleOwnerDragonScion, AGPMSIEGEWAR_DRAGONSCION_CASTLE_INDEX);

	//pThis->UpdateSiegeWarInfoUI();

	return TRUE;
}

BOOL AgcmUISiegeWar::CBSelfUpdatePosition(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUISiegeWar			*pThis				= (AgcmUISiegeWar *)		pClass;
	AgpdCharacter			*pcsCharacter		= (AgpdCharacter *)			pData;

	if (!pThis->m_bOpenUI)
		return TRUE;

	FLOAT	fDistance = AUPOS_DISTANCE_XZ(pcsCharacter->m_stPos, pThis->m_stOpenUIPos);

	if ((INT32) fDistance < 150)
		return TRUE;

	pThis->m_bOpenUI	= FALSE;

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseSiegeWarUI);
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseCatapultUI);
	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseAtkResTowerUI);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBRemoveSiegeWarGrid(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgpdItem	*pcsItem	= (AgpdItem *)	pData;

	if (pThis->IsUsableItemForCatapult(pcsItem) &&
		pThis->m_pcsCatapultGrid)
	{
		pThis->m_pcsAgpmGrid->RemoveItem(pThis->m_pcsCatapultGrid, pcsItem->m_pcsGridItem);

		// 포탄이 0개면 활성화 버튼을 disable 시키자
		if ( !pThis->m_pcsCatapultGrid->m_lItemCount )
		{
			pThis->m_bIsActiveCatapultActiveButton	= FALSE;
			pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataActiveCatapultActiveButton);
		}

		// 포탄 UI에서 포탄을 제거했음
		pThis->m_pcsAgcmSiegeWar->SendRemoveItemToAttackObject(pThis->m_pcsTarget, pcsItem);
	}
	if (pThis->IsUsableItemForAtkResTower(pcsItem) &&
		pThis->m_pcsAtkResTowerGrid)
	{
		pThis->m_pcsAgpmGrid->RemoveItem(pThis->m_pcsAtkResTowerGrid, pcsItem->m_pcsGridItem);
	}

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataCatapultGrid);
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataAtkResTowerGrid);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBRemoveItem(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgpdItem		*pcsItem	= (AgpdItem *)	pData;

	if (pcsItem->m_pcsCharacter == pThis->m_pcsAgcmCharacter->GetSelfCharacter())
	{
		if (pThis->IsUsableItemForCatapult(pcsItem) &&
			pThis->m_pcsCatapultGrid)
		{
			// m_pcsCatapultGrid 가 쓰레기 값인 경우가 있다. 일단 죽지 않도록 예외처리를 추가함
			if( pThis->m_pcsCatapultGrid->m_lGridCount < Siegewar_AttackObject_Grid::max_count )
			{
				pThis->m_pcsAgpmGrid->RemoveItem(pThis->m_pcsCatapultGrid, pcsItem->m_pcsGridItem);
				// 포탄이 0개면 활성화 버튼을 disable 시키자
				if ( !pThis->m_pcsCatapultGrid->m_lItemCount )
				{
					pThis->m_bIsActiveCatapultActiveButton	= FALSE;
					pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataActiveCatapultActiveButton);
				}
			}
		}
		if (pThis->IsUsableItemForAtkResTower(pcsItem) &&
			pThis->m_pcsAtkResTowerGrid)
		{
			// m_pcsAtkResTowerGrid 가 쓰레기 값인 경우가 있다. 일단 죽지 않도록 예외처리를 추가함
			if( pThis->m_pcsAtkResTowerGrid->m_lGridCount < Siegewar_AttackObject_Grid::max_count )
				pThis->m_pcsAgpmGrid->RemoveItem(pThis->m_pcsAtkResTowerGrid, pcsItem->m_pcsGridItem);
		}

		pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataCatapultGrid);
		pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataAtkResTowerGrid);
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::CBUpdateActionStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	AgpdSiegeWarMonsterType	eMonsterType	= pThis->m_pcsAgpmSiegeWar->GetSiegeWarMonsterType(pcsCharacter);

	if (eMonsterType != AGPD_SIEGE_MONSTER_ATTACK_RESURRECTION_TOWER &&
		eMonsterType != AGPD_SIEGE_MONSTER_CATAPULT)
		return TRUE;

	if (pcsCharacter == pThis->m_pcsAgcmUICharacter->GetTargetCharacter())
	{
		if (pThis->m_pcsAgpmCharacter->IsRepairCharacter(pcsCharacter))
		{
			if (!pThis->m_bIsOpenedRepairBarUI)
			{
				pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenRepairBarUI);

				pThis->m_bIsOpenedRepairBarUI	= TRUE;
			}
		}
		else
		{
			if (pThis->m_bIsOpenedRepairBarUI)
			{
				pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseRepairBarUI);

				pThis->m_bIsOpenedRepairBarUI	= FALSE;
			}
		}
	}

	if (pThis->m_pcsAgpmSiegeWar->CheckValidRepairAttackObject(pThis->m_pcsAgcmCharacter->GetSelfCharacter(), pThis->m_pcsTarget) == AGPMSIEGE_RESULT_SUCCESS)
	{
		pThis->m_bIsActiveCatapultRepairButton		= TRUE;
		pThis->m_bIsActiveAtkResTowerRepairButton	= TRUE;
	}
	else
	{
		pThis->m_bIsActiveCatapultRepairButton		= FALSE;
		pThis->m_bIsActiveAtkResTowerRepairButton	= FALSE;
	}

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataActiveCatapultRepairButton);
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataActiveAtkResTowerRepairButton);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBChangeCharacterBindingRegion(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis			= (AgcmUISiegeWar *)	pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)		pData;

	//pThis->UpdateSiegeWarInfoUI( NULL, TRUE );

	return TRUE;
}

BOOL AgcmUISiegeWar::CBEndCooldown(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseCarveMessageUI);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBInputSiegeWarMessage(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	return pThis->SetSiegeWarInfoEdit();
}

BOOL AgcmUISiegeWar::CBSetTargetCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass || !pData)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgpdCharacter	*pcsTarget	= (AgpdCharacter *)	pData;

	AgpdSiegeWarMonsterType	eMonsterType	= pThis->m_pcsAgpmSiegeWar->GetSiegeWarMonsterType(pcsTarget);
	if (eMonsterType == AGPD_SIEGE_MONSTER_TYPE_NONE ||
		!pThis->m_pcsAgpmCharacter->IsRepairCharacter(pcsTarget))
	{
		if (pThis->m_bIsOpenedRepairBarUI)
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseRepairBarUI);

		pThis->m_bIsOpenedRepairBarUI	= FALSE;
	}
	else
	{
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenRepairBarUI);

		pThis->m_bIsOpenedRepairBarUI	= TRUE;
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::CBResetTargetCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (pThis->m_bIsOpenedRepairBarUI)
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseRepairBarUI);

	pThis->m_bIsOpenedRepairBarUI	= FALSE;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetMainUIDescEdit(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsMainUIDescEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetMainUISiegeWarTimeEdit(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsMainUISiegeWarTimeEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetAttackHelpEdit(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsAttackHelpEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetAttackSiegeWarTimeEdit(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsAttackSiegeWarTimeEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetDefenseHelpEdit(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsDefenseHelpEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetDefenseGuildInfoEdit(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsDefenseGuildInfoEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetDateCombo(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsComboControl	= (AgcdUIControl *)	pData1;

	if (pcsComboControl->m_lType != AcUIBase::TYPE_COMBO)
		return FALSE;

	pThis->m_pcsDateCombo	= (AcUICombo *) pcsComboControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetHourCombo(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsComboControl	= (AgcdUIControl *)	pData1;

	if (pcsComboControl->m_lType != AcUIBase::TYPE_COMBO)
		return FALSE;

	pThis->m_pcsHourCombo	= (AcUICombo *) pcsComboControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBOpenSiegeWarTimeSetUI(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	// 성주인지 확인해본다.
	if (pThis->m_pcsSiegeWar->m_strOwnerGuildMasterName.GetLength() > 0)
	{
		if (pThis->m_pcsSiegeWar->m_strOwnerGuildMasterName.Compare(pThis->m_pcsAgcmCharacter->GetSelfCharacter()->m_szID) != COMPARE_EQUAL)
		{
			// 성주가 아니다. 아니라고 알려준다.
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireCastleMasterMessage);

			return TRUE;
		}
	}
	else
	{
		if (pThis->m_pcsAgpmAdmin && pThis->m_pcsAgpmAdmin->GetAdminLevel(pThis->m_pcsAgcmCharacter->GetSelfCharacter()) == 0)
		{
			// 성주가 아니다. 아니라고 알려준다.
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireCastleMasterMessage);

			return TRUE;
		}
	}

	if (pThis->m_pcsSiegeWar->m_eCurrentStatus != AGPD_SIEGE_WAR_STATUS_OFF)
	{
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCannotSetSiegeWarTimeMessage);

		return TRUE;
	}

	// 공성시간 입력 도움말과 공성전 시간을 출력해준다.
	///////////////////////////////////////////////////////////////
	pThis->SetSiegeWarTimeUIEdit();

	// 세팅할 수 있는 공성전 날짜와 시간을 콤보박스에 추가해준다.
	///////////////////////////////////////////////////////////////
	pThis->SetComboBoxData();

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataDateCombo);
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataHourCombo);

	return pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenSetTimeUI);
}

BOOL AgcmUISiegeWar::CBOpenAttackApplicationUI(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pThis->m_pcsSiegeWar)
		return FALSE;

	pThis->m_lAttackStartPage		= 1;
	pThis->m_lAttackCurrentPage		= 1;

	pThis->SetAttackGuildPage(pThis->m_lAttackStartPage, pThis->m_lAttackCurrentPage);

	pThis->m_pcsAgcmSiegeWar->SendRequestAttackApplGuildList(pThis->m_pcsSiegeWar, pThis->m_lAttackCurrentPage);

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataAttackCurrentPage);
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataAttackList);

	if (pThis->m_pcsSiegeWar->m_eCurrentStatus == AGPD_SIEGE_WAR_STATUS_PROCLAIM_WAR)
		pThis->m_bIsActiveAttackApplButton	= TRUE;
	else
		pThis->m_bIsActiveAttackApplButton	= FALSE;

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataActiveAttackApplButton);

	if (pThis->m_pcsSiegeWar->m_eCurrentStatus == AGPD_SIEGE_WAR_STATUS_READY)
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventConfirmApplGuildMessage);

	return pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenApplicationAttackUI);
}

BOOL AgcmUISiegeWar::CBOpenDefenseApplicationUI(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	pThis->m_pcsAgcmSiegeWar->SendRequestDefenseGuildList(pThis->m_pcsSiegeWar);

	pThis->SetSiegeWarDefenseHelpEdit();
	pThis->SetSiegeWarDefenseGuildInfoEdit();

	return pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenApplicationDefenseUI);
}

BOOL AgcmUISiegeWar::SetSiegeWarTimeEdit(AcUIEdit *pcsTimeEdit)
{
	if (!m_pcsSiegeWar || !pcsTimeEdit)
		return FALSE;

	pcsTimeEdit->ClearText();

	if (m_pcsSiegeWar->m_ullPrevSiegeWarTimeDate == 0 &&
		m_pcsSiegeWar->m_ullNextSiegeWarTimeDate == 0)
		return TRUE;

	pcsTimeEdit->SetLineDelimiter("\n");

	CHAR	szBuffer[256];
	ZeroMemory(szBuffer, sizeof(szBuffer));

	CHAR	szDate[AUTIMESTAMP_SIZE_TIME_STRING + 1];
	ZeroMemory(szDate, sizeof(szDate));

	UINT64	ullTime	= 0;

	if (m_pcsSiegeWar->m_eCurrentStatus == AGPD_SIEGE_WAR_STATUS_OFF)
		ullTime	= m_pcsSiegeWar->m_ullPrevSiegeWarTimeDate;
	else
		ullTime	= m_pcsSiegeWar->m_ullNextSiegeWarTimeDate;

	SYSTEMTIME	system_time;
	FILETIME	file_time;

	CopyMemory(&file_time, &ullTime, sizeof(UINT64));
	if (::FileTimeToSystemTime(&file_time, &system_time) == 0)
		return FALSE;

	if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
	{
		sprintf(szBuffer,
			"-%s\n %d/%d/%d %d:%02d(GST) %s",
				m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE),
				system_time.wMonth,
				system_time.wDay,
				system_time.wYear,
				system_time.wHour,
				system_time.wMinute,
				(m_pcsSiegeWar->m_eCurrentStatus == AGPD_SIEGE_WAR_STATUS_OFF) ? m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_COMPLETE) : m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_RESERVE));
	}
	else
	{
	sprintf(szBuffer,
			"%d%s %d%s %d%s %s%d%s %s",//"%s\n%d%s %d%s %d%s %s%d%s %s",
			//m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE),
			system_time.wYear,
			m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_YEAR),
			system_time.wMonth,
			m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_MONTH),
			system_time.wDay,
			m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_DAY),
			m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_PM),
			system_time.wHour - 12,
			m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_HOUR),
			(m_pcsSiegeWar->m_eCurrentStatus == AGPD_SIEGE_WAR_STATUS_OFF) ? m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_COMPLETE) : m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_RESERVE));
	}

	pcsTimeEdit->SetText(szBuffer);

	return TRUE;
}

BOOL AgcmUISiegeWar::SetMainUIEdit()
{
	if (!m_pcsSiegeWar)
		return FALSE;

	if (m_pcsMainUIDescEdit && m_pcsEvent && m_pcsEvent->m_pcsSource && m_pcsEvent->m_pcsSource->m_eType == APBASE_TYPE_CHARACTER)
	{
		m_pcsMainUIDescEdit->ClearText();

		m_pcsMainUIDescEdit->SetLineDelimiter("\n");

		CHAR	szBuffer[256];
		ZeroMemory(szBuffer, sizeof(szBuffer));


		if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
		{
			// 전령의 이름이 <Siege Commander> Garrison과 같은 형식인데 <>을 컬러 tag로 인식해
			// 이상한 색으로 보여줘서 괄호 사이의 문자를 삭제하는 함수를 만들었다.
			char name[128];
			RemoveTag(((AgpdCharacter *) m_pcsEvent->m_pcsSource)->m_szID,
					  strlen(((AgpdCharacter *) m_pcsEvent->m_pcsSource)->m_szID),
					  name, 128);

			sprintf(szBuffer,
					m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MAIN_TEXT), 
					name,
					m_pcsSiegeWar->m_strCastleName.GetBuffer()
					);
		}
		else
		{
			sprintf(szBuffer, m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_MAIN_TEXT), 
								((AgpdCharacter *) m_pcsEvent->m_pcsSource)->m_szID,
								m_pcsSiegeWar->m_strCastleName.GetBuffer(),
								m_pcsSiegeWar->m_strOwnerGuildName.GetBuffer(),
								m_pcsSiegeWar->m_strOwnerGuildMasterName.GetBuffer());
		}

		m_pcsMainUIDescEdit->SetText(szBuffer);
	}

	if (m_pcsMainUISiegeWarTimeEdit)
	{
		SetSiegeWarTimeEdit(m_pcsMainUISiegeWarTimeEdit);
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::SetSiegeWarTimeUIEdit()
{
	if (!m_pcsSiegeWar)
		return FALSE;

	if (m_pcsSiegeWar->m_ullPrevSiegeWarTimeDate == 0)
		return TRUE;

	if (m_pcsAttackHelpEdit)
	{
		m_pcsAttackHelpEdit->ClearText();
		m_pcsAttackHelpEdit->SetLineDelimiter("\n");

		CHAR	szBuffer[2048];
		ZeroMemory(szBuffer, sizeof(szBuffer));

		sprintf(szBuffer, "%s\n\n", m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_SET_TIME_HELP));

		UINT64	ullDifferTimeSec	= m_pcsSiegeWar->m_ullPrevSiegeWarTimeDate 
									+ m_pcsSiegeWar->m_csTemplate.m_ulSiegeWarDurationMSec * 10000
									+ m_pcsSiegeWar->m_csTemplate.m_ulProclaimTimeHour * (UINT64) 60 * 60 * 10000
									- AuTimeStamp2::GetCurrentTimeStamp();

		ullDifferTimeSec	= ullDifferTimeSec / 10000000;

		// 한시간 이내면 분단위, 이상이면 시간단위로 표시해준다.
		if (ullDifferTimeSec < 60 * 60)
		{
			sprintf(szBuffer + strlen(szBuffer), "%s : %d %s", m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_REMAIN_TIME),
															   ullDifferTimeSec / 60,
															   m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_MINUTE));
		}
		else
		{
			sprintf(szBuffer + strlen(szBuffer), "%s : %d %s", m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_REMAIN_TIME),
															   ullDifferTimeSec / (60 * 60),
															   m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_HOUR));
		}

		m_pcsAttackHelpEdit->SetText(szBuffer);
	}

	if (m_pcsAttackSiegeWarTimeEdit)
	{
		SetSiegeWarTimeEdit(m_pcsAttackSiegeWarTimeEdit);
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::SetSiegeWarDefenseHelpEdit()
{
	if (!m_pcsDefenseHelpEdit)
		return FALSE;

	m_pcsDefenseHelpEdit->SetLineDelimiter(m_pcsAgcmUIManager2->GetLineDelimiter());
	m_pcsDefenseHelpEdit->SetText(m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DEFENSE_GUILD_HELP));

	return TRUE;
}

BOOL AgcmUISiegeWar::SetSiegeWarDefenseGuildInfoEdit()
{
	if (!m_pcsSiegeWar || !m_pcsDefenseGuildInfoEdit)
		return FALSE;

	CHAR	szBuffer[256];
	ZeroMemory(szBuffer, sizeof(szBuffer));

	if (strlen(m_astDefenseGuildList[0].szGuildName) <= 0)
	{
		sprintf(szBuffer, "%s", m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_NOT_SELECTED_DEFENSE_GUILD));
	}
	else
	{
		sprintf(szBuffer, "%s", m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DEFENSE_GUILD));

		for (int i = 0; i < AGPMSIEGEWAR_TOTAL_DEFENSE_GUILD; ++i)
		{
			if (strlen(m_astDefenseGuildList[i].szGuildName) > 0)
			{
				sprintf(szBuffer + strlen(szBuffer), "\n%s, %s(%d)",
					m_astDefenseGuildList[i].szGuildName,
					m_astDefenseGuildList[i].szGuildMasterName,
					i + 1);
			}
			else
				break;
		}
	}

	m_pcsDefenseGuildInfoEdit->SetLineDelimiter("\n");
	m_pcsDefenseGuildInfoEdit->SetText(szBuffer);

	return TRUE;
}

BOOL AgcmUISiegeWar::SetSiegeWarInfoEdit()
{
// 	if (!m_pcsSiegeWarInfoEdit)
// 		return FALSE;
// 
// 	m_pcsSiegeWarInfoEdit->SetLineDelimiter("\n");
// 
// 	CHAR	szBuffer[18000];
// 	ZeroMemory(szBuffer, sizeof(szBuffer));
// 
// 	INT32	lMessageCount	= m_pcsAgcmChatting2->GetChatMessageCount(AGCMCHATTING_TYPE_SIEGEWARINFO);
// 	UINT32	ulColor = 0xffffffff;
// 
// 	for (int i = lMessageCount - m_lSiegeWarInfoScroll - 3; i < lMessageCount - m_lSiegeWarInfoScroll; ++i)
// 	{
// 		if (i < 0)
// 			continue;
// 
// 		CHAR	*szMessage			= m_pcsAgcmChatting2->GetChatMessage(AGCMCHATTING_TYPE_SIEGEWARINFO, i);
// 		if (!szMessage || !szMessage[0])
// 			continue;
// 
// 		if (strlen(szMessage) + strlen(szBuffer) >= 18000)
// 			break;
// 
// 		ulColor = m_pcsAgcmChatting2->GetChatMessageColor(AGCMCHATTING_TYPE_SIEGEWARINFO, i);
// 
// 		sprintf(szBuffer + strlen(szBuffer), "<C%d>%s\n", ulColor, szMessage);
// 	}
// 
// 	return _SetMessageText( szBuffer, ulColor );
	return TRUE;
	//return m_pcsSiegeWarInfoEdit->SetText(szBuffer);
}

BOOL AgcmUISiegeWar::SetRemainTimeEdit(UINT32 ulClockCount)
{
	if (!m_pcsRemainTimeEdit)
		return FALSE;

	CHAR	szBuffer[128];
	ZeroMemory(szBuffer, sizeof(szBuffer));

	AgpdSiegeWar	*pcsSiegeWar	= m_pcsAgpmSiegeWar->GetSiegeWarInfo(m_pcsAgcmCharacter->GetSelfCharacter());

	// 남은 시간 표시
	if (!m_pcsAgpmSiegeWar->IsStarted(pcsSiegeWar))
	{
		//sprintf(szBuffer, "%s", m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_NOT_STARTED));
		return FALSE;
	}
	else
	{
#ifdef	__TEST_SERVER_ONLY__
		INT32	lRemainTimeSec	= (pcsSiegeWar->m_ulSiegeWarStartTimeMSec + 3600000 - ulClockCount) / 1000;
#else
		INT32	lRemainTimeSec	= 0;
		lRemainTimeSec	= (pcsSiegeWar->m_ulSiegeWarStartTimeMSec + m_pcsAgpmSiegeWar->GetDurationMSec(pcsSiegeWar) - ulClockCount) / 1000;

#endif	//__TEST_SERVER_ONLY__
		if (lRemainTimeSec < 0)
			lRemainTimeSec	= 0;

		if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
		{
			sprintf(szBuffer, "%2d : %2d", lRemainTimeSec / 60,
										   lRemainTimeSec % 60);
		}
		else
		{
			sprintf(szBuffer, "%2d%s %2d%s", lRemainTimeSec / 60, m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_MINUTE),
											 lRemainTimeSec % 60, m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_SECOND));
		}
	}

	return m_pcsRemainTimeEdit->SetText(szBuffer);
}

BOOL AgcmUISiegeWar::SetComboBoxData()
{
	if (!m_pcsSiegeWar ||
		!m_pcsDateCombo ||
		!m_pcsHourCombo)
		return FALSE;

	UINT64	ullNextSiegeWarTimeDate	= 0;
	if (m_pcsSiegeWar->m_ullPrevSiegeWarTimeDate > 0)
		ullNextSiegeWarTimeDate	= m_pcsAgpmSiegeWar->GetNextSiegeWarFirstTime(m_pcsSiegeWar);
	else
		ullNextSiegeWarTimeDate	= m_pcsAgpmSiegeWar->GetFirstSiegeWarTime(m_pcsSiegeWar);

	SYSTEMTIME	system_time;
	FILETIME	file_time;

	CopyMemory(&file_time, &ullNextSiegeWarTimeDate, sizeof(UINT64));
	if (::FileTimeToSystemTime(&file_time, &system_time) == 0)
		return FALSE;

	sprintf(m_szDateCombo[0], "%d%s %d%s %d%s", system_time.wYear,
												m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_YEAR),
												system_time.wMonth,
												m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_MONTH),
												system_time.wDay,
												m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_DAY));

	sprintf(m_szDateCombo[1], "%d%s %d%s %d%s", system_time.wYear,
												m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_YEAR),
												system_time.wMonth,
												m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_MONTH),
												system_time.wDay + 1,
												m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_DAY));

	for (int i = 0; i < AGCMUISIEGEWAR_MAX_SELECT_HOUR; ++i)
	{
		sprintf(m_szHourCombo[i], "%s %d%s", m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_PM),
											 AGPMSIEGEWAR_START_WAR_TIME + i,
										 	 m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_DATE_HOUR));
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayNPCName(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue)
{
	if (!pClass || !szDisplay)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	if (!pThis->m_pcsEvent ||
		!pThis->m_pcsEvent->m_pcsSource ||
		pThis->m_pcsEvent->m_pcsSource->m_eType != APBASE_TYPE_CHARACTER)
		return FALSE;

	if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
		sprintf(szDisplay, "%s", FindNameWithoutJob( ((AgpdCharacter *) pThis->m_pcsEvent->m_pcsSource)->m_szID ) );
	else
		sprintf(szDisplay, "%s %s", pThis->m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_ID_SIEGEWAR_PREFIX_NPC_NAME), ((AgpdCharacter *) pThis->m_pcsEvent->m_pcsSource)->m_szID);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayAttackGuildName(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData || !pcsSourceControl)
		return FALSE;

	AgcmUISiegeWar		*pThis			= (AgcmUISiegeWar *)	pClass;
	AtttackGuildList	**ppcsAttackGuild	= (AtttackGuildList **)	pData;

	if (ppcsAttackGuild && *ppcsAttackGuild && (*ppcsAttackGuild)->szGuildName)
		sprintf(szDisplay, "%s", (*ppcsAttackGuild)->szGuildName);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayAttackGuildMasterName(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData || !pcsSourceControl)
		return FALSE;

	AgcmUISiegeWar		*pThis			= (AgcmUISiegeWar *)	pClass;
	AtttackGuildList	**ppcsAttackGuild	= (AtttackGuildList **)	pData;

	if (ppcsAttackGuild && *ppcsAttackGuild && (*ppcsAttackGuild)->szGuildMasterName)
		sprintf(szDisplay, "%s", (*ppcsAttackGuild)->szGuildMasterName);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayAttackGuildPoint(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData || !pcsSourceControl)
		return FALSE;

	AgcmUISiegeWar		*pThis			= (AgcmUISiegeWar *)	pClass;
	AtttackGuildList	**ppcsAttackGuild	= (AtttackGuildList **)	pData;

	if (ppcsAttackGuild && *ppcsAttackGuild && (*ppcsAttackGuild)->szGuildName && strlen((*ppcsAttackGuild)->szGuildName) > 0)
		sprintf(szDisplay, "%d", (*ppcsAttackGuild)->ulGuildPoint);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayDefenseGuildName(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData || !pcsSourceControl)
		return FALSE;

	AgcmUISiegeWar		*pThis			= (AgcmUISiegeWar *)	pClass;
	AtttackGuildList	**ppcsAttackGuild	= (AtttackGuildList **)	pData;

	if (ppcsAttackGuild && *ppcsAttackGuild && (*ppcsAttackGuild)->szGuildName)
		sprintf(szDisplay, "%s", (*ppcsAttackGuild)->szGuildName);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayDefenseGuildMasterName(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData || !pcsSourceControl)
		return FALSE;

	AgcmUISiegeWar		*pThis			= (AgcmUISiegeWar *)	pClass;
	AtttackGuildList	**ppcsAttackGuild	= (AtttackGuildList **)	pData;

	if (ppcsAttackGuild && *ppcsAttackGuild && (*ppcsAttackGuild)->szGuildMasterName)
		sprintf(szDisplay, "%s", (*ppcsAttackGuild)->szGuildMasterName);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayDefenseGuildPoint(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData || !pcsSourceControl)
		return FALSE;

	AgcmUISiegeWar		*pThis			= (AgcmUISiegeWar *)	pClass;
	AtttackGuildList	**ppcsAttackGuild	= (AtttackGuildList **)	pData;

	if (ppcsAttackGuild && *ppcsAttackGuild && (*ppcsAttackGuild)->szGuildName && strlen((*ppcsAttackGuild)->szGuildName) > 0)
		sprintf(szDisplay, "%d", (*ppcsAttackGuild)->ulGuildPoint);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayAttackPageNumber(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (pThis->m_lAttackMaxPage <= 0)
		return FALSE;

	INT32	lBaseSlotPos	= 9 / 2 + 1;

	INT32	lStartSlotIndex	= lBaseSlotPos - (pThis->m_lAttackMaxPage - 1);

	if (pcsSourceControl->m_lUserDataIndex < lStartSlotIndex ||
		pcsSourceControl->m_lUserDataIndex > lStartSlotIndex + (pThis->m_lAttackMaxPage - 1) * 2)
		return TRUE;

	if ((pcsSourceControl->m_lUserDataIndex - lStartSlotIndex) % 2)
		return TRUE;

	_stprintf(szDisplay, "%d", pThis->m_lAttackStartPage + (pcsSourceControl->m_lUserDataIndex - lStartSlotIndex) / 2);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayDefensePageNumber(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pcsSourceControl)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (pThis->m_lDefenseMaxPage <= 0)
		return FALSE;

	INT32	lBaseSlotPos	= 9 / 2 + 1;

	INT32	lStartSlotIndex	= lBaseSlotPos - (pThis->m_lDefenseMaxPage - 1);

	if (pcsSourceControl->m_lUserDataIndex < lStartSlotIndex ||
		pcsSourceControl->m_lUserDataIndex > lStartSlotIndex + (pThis->m_lDefenseMaxPage - 1) * 2)
		return TRUE;

	if ((pcsSourceControl->m_lUserDataIndex - lStartSlotIndex) % 2)
		return TRUE;

	_stprintf(szDisplay, "%d", pThis->m_lDefenseStartPage + (pcsSourceControl->m_lUserDataIndex - lStartSlotIndex) / 2);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayComboDate(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData || !szDisplay)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	CHAR	*pszComboDate	= *(CHAR **)	pData;

	sprintf(szDisplay, "%s", pszComboDate);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayComboHour(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData || !szDisplay)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	CHAR	*pszComboHour	= *(CHAR **)	pData;

	sprintf(szDisplay, "%s", pszComboHour);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayUseCharacterName(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !szDisplay)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	sprintf(szDisplay, "%s", pThis->m_szUseCharName);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayRepairBarMax(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !szDisplay || !plValue)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	*plValue	= pThis->m_lRepairDurationMSec;

	sprintf(szDisplay, "%d", *plValue);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBDisplayRepairBarCurrent(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !szDisplay || !plValue)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	*plValue	= pThis->GetClockCount() - pThis->m_lRepairStartTimeMSec;
	if (*plValue > pThis->m_lRepairDurationMSec)
		*plValue	= pThis->m_lRepairDurationMSec;

	sprintf(szDisplay, "%d", *plValue);

	return TRUE;
}

BOOL AgcmUISiegeWar::CBIsActiveCatapultRepairButton(PVOID pClass, PVOID pData, AgcdUIDataType eType, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis			= (AgcmUISiegeWar *)	pClass;

	return pThis->m_bIsActiveCatapultRepairButton;
}

BOOL AgcmUISiegeWar::CBIsActiveCatapultActiveButton(PVOID pClass, PVOID pData, AgcdUIDataType eType, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis			= (AgcmUISiegeWar *)	pClass;

	return pThis->m_bIsActiveCatapultActiveButton;
}

BOOL AgcmUISiegeWar::CBIsActiveAtkResTowerRepairButton(PVOID pClass, PVOID pData, AgcdUIDataType eType, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis			= (AgcmUISiegeWar *)	pClass;

	return pThis->m_bIsActiveAtkResTowerRepairButton;
}

BOOL AgcmUISiegeWar::CBIsActiveAtkResTowerActiveButton(PVOID pClass, PVOID pData, AgcdUIDataType eType, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis			= (AgcmUISiegeWar *)	pClass;

	return pThis->m_bIsActiveAtkResTowerActiveButton;
}

BOOL AgcmUISiegeWar::CBIsActiveAttackApplButton(PVOID pClass, PVOID pData, AgcdUIDataType eType, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis			= (AgcmUISiegeWar *)	pClass;

	return pThis->m_bIsActiveAttackApplButton;
}

BOOL AgcmUISiegeWar::CBConfirmSiegeWarTime(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pThis->m_pcsDateCombo ||
		!pThis->m_pcsHourCombo)
		return FALSE;

	UINT64	ullNewSetTime	= 0;

	if (!pThis->GetComboTime(&ullNewSetTime))
		return FALSE;

	// 서버로 정해진 시간을 보낸다.
	////////////////////////////////////////////////////
	return pThis->m_pcsAgcmSiegeWar->SendNextSiegeWarTime(pThis->m_pcsSiegeWar, ullNewSetTime);
}

BOOL AgcmUISiegeWar::CBSelectDefenseGuild(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pThis->m_pcsSiegeWar)
		return FALSE;

	/*
	// 성주인지 확인해본다.
	if (!pThis->m_pcsAgpmSiegeWar->IsCastleOwner(pThis->m_pcsSiegeWar, pThis->m_pcsAgcmCharacter->GetSelfCharacter()->m_szID))
	{
		// 성주가 아니다. 아니라고 알려준다.
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireCastleMasterMessage);

		return TRUE;
	}
	*/

	pThis->m_lDefenseStartPage		= 1;
	pThis->m_lDefenseCurrentPage	= 1;

	pThis->SetDefenseGuildPage(pThis->m_lDefenseStartPage, pThis->m_lDefenseCurrentPage);

	pThis->m_pcsAgcmSiegeWar->SendRequestDefenseApplGuildList(pThis->m_pcsSiegeWar, pThis->m_lDefenseCurrentPage);

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventOpenSelectDefenseGuildUI);

	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataDefenseCurrentPage);
	pThis->m_pcsAgcmUIManager2->SetUserDataRefresh(pThis->m_pcsUserDataDefenseList);

	pThis->m_lSelectedDefenseGuildList	= (-1);

	return pThis->SetSelectedDefenseGuild();
}

BOOL AgcmUISiegeWar::CBApplicationDefense(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pThis->m_pcsSiegeWar)
		return FALSE;

	// 조건에 맞는 사람이 신청을 하는건지 확인한다.
	if (!pThis->m_pcsAgpmSiegeWar->CheckApplicationRequirement(pThis->m_pcsAgcmCharacter->GetSelfCharacter()))
	{
		if (!pThis->m_pcsAgpmGuild->IsMaster(pThis->m_pcsAgpmGuild->GetGuild(pThis->m_pcsAgcmCharacter->GetSelfCharacter()), pThis->m_pcsAgcmCharacter->GetSelfCharacter()->m_szID))
		{
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireGuildMasterMessage);
		}
		else if (!pThis->m_pcsAgpmSiegeWar->CheckGuildRequirement(pThis->m_pcsAgpmGuild->GetGuild(pThis->m_pcsAgcmCharacter->GetSelfCharacter())))
		{
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireGuildConditionMessage);
		}

		return TRUE;
	}

	pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventCloseSiegeWarUI);

	return pThis->m_pcsAgcmSiegeWar->SendDefenseApplication(pThis->m_pcsSiegeWar);
}

BOOL AgcmUISiegeWar::CBCancelDefense(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pThis->m_pcsSiegeWar)
		return FALSE;

	if (!pThis->m_pcsAgpmGuild->IsMaster(pThis->m_pcsAgpmGuild->GetGuild(pThis->m_pcsAgcmCharacter->GetSelfCharacter()), pThis->m_pcsAgcmCharacter->GetSelfCharacter()->m_szID))
	{
		pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireGuildMasterMessage);
		return TRUE;
	}

	// 신청한 놈인지 살펴본다.
	//
	//
	//

	return pThis->m_pcsAgcmSiegeWar->SendCancelDefenseApplication(pThis->m_pcsSiegeWar);
}

BOOL AgcmUISiegeWar::CBApplicationAttack(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pThis->m_pcsSiegeWar)
		return FALSE;

// 	// 받은 리스트 기준으로 AGPMSIEGEWAR_TOTAL_ATTACK_GUILD 만큼의 길드가 신청을 한 상태라면 메시지 표시
// 	if(m_nNumOfApplyGuilds >= AGPMSIEGEWAR_TOTAL_ATTACK_GUILD)
// 	{
// 		pThis->m_pcsAgcmUIManager2->ActionMessageOKDialog( "The number of Siege applicants is reached the limitation." );
// 		return FALSE;
// 	}

	// 조건에 맞는 사람이 신청을 하는건지 확인한다.
	if (!pThis->m_pcsAgpmSiegeWar->CheckApplicationRequirement(pThis->m_pcsAgcmCharacter->GetSelfCharacter()))
	{
		if (!pThis->m_pcsAgpmGuild->IsMaster(pThis->m_pcsAgpmGuild->GetGuild(pThis->m_pcsAgcmCharacter->GetSelfCharacter()), pThis->m_pcsAgcmCharacter->GetSelfCharacter()->m_szID))
		{
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireGuildMasterMessage);
		}
		else if (!pThis->m_pcsAgpmSiegeWar->CheckGuildRequirement(pThis->m_pcsAgpmGuild->GetGuild(pThis->m_pcsAgcmCharacter->GetSelfCharacter())))
		{
			pThis->m_pcsAgcmUIManager2->ThrowEvent(pThis->m_lEventRequireGuildConditionMessage);
		}

		return TRUE;
	}

	return pThis->m_pcsAgcmSiegeWar->SendAttackApplication(pThis->m_pcsSiegeWar);
}

BOOL AgcmUISiegeWar::CBAttackPageLeft(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	return pThis->SetAttackGuildPage((pThis->m_lAttackStartPage == 1 || pThis->m_lAttackCurrentPage - 1 >= pThis->m_lAttackStartPage) ? pThis->m_lAttackStartPage : pThis->m_lAttackStartPage - 1,
									 (pThis->m_lAttackCurrentPage == 1) ? pThis->m_lAttackCurrentPage : pThis->m_lAttackCurrentPage - 1);
}

BOOL AgcmUISiegeWar::CBAttackPageRight(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (pThis->m_lAttackMaxPage <= 0)
		return FALSE;

	return pThis->SetAttackGuildPage((pThis->m_lAttackCurrentPage == pThis->m_lAttackMaxPage || pThis->m_lAttackCurrentPage <= pThis->m_lAttackStartPage + 3) ? pThis->m_lAttackStartPage : pThis->m_lAttackStartPage + 1,
									 (pThis->m_lAttackCurrentPage == pThis->m_lAttackMaxPage) ? pThis->m_lAttackCurrentPage : pThis->m_lAttackCurrentPage + 1);
}

BOOL AgcmUISiegeWar::CBAttackGoPage(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pcsSourceControl->m_pcsBase->m_szStaticString ||
		_tcslen(pcsSourceControl->m_pcsBase->m_szStaticString) == 0)
		return TRUE;

	return pThis->SetAttackGuildPage(pThis->m_lAttackStartPage,
									 _ttoi(pcsSourceControl->m_pcsBase->m_szStaticString));
}

BOOL AgcmUISiegeWar::CBDefensePageLeft(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	return pThis->SetDefenseGuildPage((pThis->m_lDefenseStartPage == 1 || pThis->m_lDefenseCurrentPage - 1 >= pThis->m_lDefenseStartPage) ? pThis->m_lDefenseStartPage : pThis->m_lDefenseStartPage - 1,
									 (pThis->m_lDefenseCurrentPage == 1) ? pThis->m_lDefenseCurrentPage : pThis->m_lDefenseCurrentPage - 1);
}

BOOL AgcmUISiegeWar::CBDefensePageRight(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (pThis->m_lDefenseMaxPage <= 0)
		return FALSE;

	return pThis->SetDefenseGuildPage((pThis->m_lDefenseCurrentPage == pThis->m_lDefenseMaxPage || pThis->m_lDefenseCurrentPage <= pThis->m_lDefenseStartPage + 3) ? pThis->m_lDefenseStartPage : pThis->m_lDefenseStartPage + 1,
									 (pThis->m_lDefenseCurrentPage == pThis->m_lDefenseMaxPage) ? pThis->m_lDefenseCurrentPage : pThis->m_lDefenseCurrentPage + 1);
}

BOOL AgcmUISiegeWar::CBDefenseGoPage(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pcsSourceControl->m_pcsBase->m_szStaticString ||
		_tcslen(pcsSourceControl->m_pcsBase->m_szStaticString) == 0)
		return TRUE;

	return pThis->SetDefenseGuildPage(pThis->m_lDefenseStartPage,
									 _ttoi(pcsSourceControl->m_pcsBase->m_szStaticString));
}

BOOL AgcmUISiegeWar::CBDefenseGuildListSelect(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (strlen(pThis->m_astDefenseApplGuildList[pcsSourceControl->m_lUserDataIndex].szGuildName) <= 0)
		return TRUE;

	if (pThis->m_pcsSiegeWar->m_csDefenseGuild.GetObject(pThis->m_astDefenseApplGuildList[pcsSourceControl->m_lUserDataIndex].szGuildName))
		return TRUE;

	pThis->m_lSelectedDefenseGuildList	= pcsSourceControl->m_lUserDataIndex;

	return pThis->SetSelectedDefenseGuild();
}

BOOL AgcmUISiegeWar::CBGetCatapultDescEdit(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsCatapultDescEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetAtkResTowerDescEdit(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsAtkResTowerDescEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBRepairCatapult(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	return pThis->m_pcsAgcmSiegeWar->SendRequestRepairAttackObject(pThis->m_pcsTarget);
}

BOOL AgcmUISiegeWar::CBRepairAtkResTower(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	return pThis->m_pcsAgcmSiegeWar->SendRequestRepairAttackObject(pThis->m_pcsTarget);
}

BOOL AgcmUISiegeWar::CBActiveCatapult(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	return pThis->m_pcsAgcmSiegeWar->SendRequestUseAttackObject(pThis->m_pcsTarget);
}

BOOL AgcmUISiegeWar::CBActiveAtkResTower(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	return pThis->m_pcsAgcmSiegeWar->SendRequestUseAttackObject(pThis->m_pcsTarget);
}

BOOL AgcmUISiegeWar::CBCatapultDragDrop(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass ||
		!pcsSourceControl ||
		!pcsSourceControl->m_pcsBase)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pcsSourceControl || (pcsSourceControl->m_lType != AcUIBase::TYPE_GRID))
		return FALSE;

	AgpdGridSelectInfo	*pcsGridSelectInfo = ((AcUIGrid *) pcsSourceControl->m_pcsBase)->GetDragDropMessageInfo();
	if (!pcsGridSelectInfo)
		return FALSE;

	AgpdGrid	*pcsGrid	= pThis->m_pcsAgcmUIManager2->GetControlGrid(pcsSourceControl);
	if (!pcsGrid)
		return FALSE;

	AgpdItem	*pcsItem	= pThis->m_pcsAgpmItem->GetItem(pcsGridSelectInfo->pGridItem->m_lItemID);
	if (!pcsItem)
		return FALSE;

	if (pcsItem->m_eStatus != AGPDITEM_STATUS_INVENTORY)
		return FALSE;

	return pThis->m_pcsAgcmSiegeWar->SendAddItemToAttackObject(pThis->m_pcsTarget, pcsItem);
}

BOOL AgcmUISiegeWar::CBAtkResTowerDragDrop(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass ||
		!pcsSourceControl ||
		!pcsSourceControl->m_pcsBase)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (!pcsSourceControl || (pcsSourceControl->m_lType != AcUIBase::TYPE_GRID))
		return FALSE;

	AgpdGridSelectInfo	*pcsGridSelectInfo = ((AcUIGrid *) pcsSourceControl->m_pcsBase)->GetDragDropMessageInfo();
	if (!pcsGridSelectInfo)
		return FALSE;

	AgpdGrid	*pcsGrid	= pThis->m_pcsAgcmUIManager2->GetControlGrid(pcsSourceControl);
	if (!pcsGrid)
		return FALSE;

	AgpdItem	*pcsItem	= pThis->m_pcsAgpmItem->GetItem(pcsGridSelectInfo->pGridItem->m_lItemID);
	if (!pcsItem)
		return FALSE;

	if (pcsItem->m_eStatus != AGPDITEM_STATUS_INVENTORY)
		return FALSE;

	return pThis->m_pcsAgcmSiegeWar->SendAddItemToAttackObject(pThis->m_pcsTarget, pcsItem);
}

BOOL AgcmUISiegeWar::CBGetSiegeWarInfoEditControl(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsSiegeWarInfoEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBGetRemainTimeEditControl(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsRemainTimeEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUISiegeWar::CBSiegeWarInfoScrollUp(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	pThis->_SetMsgTextToEditBox( -1 );
	//pThis->m_pcsSiegeWarInfoEdit->OnMouseWheel(1);
	//pThis->m_pcsSiegeWarInfoEdit->SetText();
	/*
	INT32	lMessageCount	= pThis->m_pcsAgcmChatting2->GetChatMessageCount(AGCMCHATTING_TYPE_SIEGEWARINFO);

	if (lMessageCount - pThis->m_lSiegeWarInfoScroll - 3 <= 0)
		return TRUE;

	++pThis->m_lSiegeWarInfoScroll;

	pThis->SetSiegeWarInfoEdit();
	*/

	return TRUE;
}

BOOL AgcmUISiegeWar::CBSiegeWarInfoScrollDown(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass)
		return FALSE;

	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	pThis->_SetMsgTextToEditBox( 1 );
	//pThis->m_pcsSiegeWarInfoEdit->OnMouseWheel(-1);
	//pThis->m_pcsSiegeWarInfoEdit->SetText();
	/*
	if (pThis->m_lSiegeWarInfoScroll <= 0)
		return TRUE;

	--pThis->m_lSiegeWarInfoScroll;

	pThis->SetSiegeWarInfoEdit();
	*/

	return TRUE;
}

BOOL AgcmUISiegeWar::CBRegisterDefenseGuild(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUISiegeWar	*pThis	= (AgcmUISiegeWar *)	pClass;

	if (pThis->m_lSelectedDefenseGuildList < 0 ||
		pThis->m_lSelectedDefenseGuildList >= AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE)
		return FALSE;

	// 선정된 수성 길드를 서버로 전송한다.
	pThis->m_pcsAgcmSiegeWar->SendSelectDefenseGuild(pThis->m_pcsSiegeWar, pThis->m_astDefenseApplGuildList[pThis->m_lSelectedDefenseGuildList].szGuildName);

	return pThis->m_pcsAgcmSiegeWar->SendRequestDefenseGuildList(pThis->m_pcsSiegeWar);
}

BOOL AgcmUISiegeWar::SetAttackGuildPage(INT32 lStartPage, INT32 lSelectPage)
{
	if (lStartPage < 0 || lSelectPage < 0)
		return FALSE;

	m_lAttackStartPage	= lStartPage;
	m_lAttackCurrentPage	= lSelectPage;

	m_pcsAgcmUIManager2->RefreshUserData(m_pcsUserDataAttackCurrentPage, TRUE);

	INT32	lBaseSlotPos	= 9 / 2 + 1;
	INT32	lStartSlotIndex	= lBaseSlotPos - (m_lAttackMaxPage - 1);

	SetAttackSelectPageSlot(lStartSlotIndex + (m_lAttackCurrentPage - m_lAttackStartPage) * 2);

	m_pcsAgcmSiegeWar->SendRequestAttackApplGuildList(m_pcsSiegeWar, lSelectPage);

	return TRUE;
}

BOOL AgcmUISiegeWar::SetDefenseGuildPage(INT32 lStartPage, INT32 lSelectPage)
{
	if (lStartPage < 0 || lSelectPage < 0)
		return FALSE;

	m_lDefenseStartPage	= lStartPage;
	m_lDefenseCurrentPage	= lSelectPage;

	m_pcsAgcmUIManager2->RefreshUserData(m_pcsUserDataDefenseCurrentPage, TRUE);

	INT32	lBaseSlotPos	= 9 / 2 + 1;
	INT32	lStartSlotIndex	= lBaseSlotPos - (m_lDefenseMaxPage - 1);

	SetDefenseSelectPageSlot(lStartSlotIndex + (m_lDefenseCurrentPage - m_lDefenseStartPage) * 2);

	m_pcsAgcmSiegeWar->SendRequestDefenseApplGuildList(m_pcsSiegeWar, lSelectPage);

	return TRUE;
}

BOOL AgcmUISiegeWar::GetComboTime(UINT64 *pullTime)
{
	if (!pullTime || !m_pcsSiegeWar)
		return FALSE;

	INT32	lSelectedDate	= m_pcsDateCombo->GetSelectedIndex();
	INT32	lSelectedHour	= m_pcsHourCombo->GetSelectedIndex();

	UINT64	ullNextSiegeWarTimeDate	= 0;

	//if (m_pcsSiegeWar->m_ulPrevSiegeWarTimeDate > 0)
	//	ulNextSiegeWarTimeDate	= m_pcsSiegeWar->m_ulPrevSiegeWarTimeDate + 60 * 60 * 24 * m_pcsSiegeWar->m_csTemplate.m_ulSiegeWarIntervalDay;
	//else
	//	ulNextSiegeWarTimeDate	= AuTimeStamp::GetCurrentTimeStamp() + 60 * 60 * 24 * (m_pcsSiegeWar->m_csTemplate.m_ulSiegeWarIntervalDay - 7);

	if (m_pcsSiegeWar->m_ullPrevSiegeWarTimeDate > 0)
		ullNextSiegeWarTimeDate	= m_pcsAgpmSiegeWar->GetNextSiegeWarFirstTime(m_pcsSiegeWar);
	else
		ullNextSiegeWarTimeDate	= m_pcsAgpmSiegeWar->GetFirstSiegeWarTime(m_pcsSiegeWar);

	ullNextSiegeWarTimeDate	+= (UINT64) lSelectedDate * 60 * 60 * 24 * 10000000;

	SYSTEMTIME	system_time;
	FILETIME	file_time;

	CopyMemory(&file_time, &ullNextSiegeWarTimeDate, sizeof(UINT64));
	::FileTimeToSystemTime(&file_time, &system_time);

	system_time.wHour	= (AGPMSIEGEWAR_START_WAR_TIME + 12 + lSelectedHour);

	::SystemTimeToFileTime(&system_time, &file_time);
	CopyMemory(&ullNextSiegeWarTimeDate, &file_time, sizeof(UINT64));

	//if (newtime->tm_wday == 0)	// sunday
	//{
	//	ulNextSiegeWarTimeDate	-= (1 - lSelectedDate) * 60 * 60 * 24;
	//}
	//else
	//{
	//	ulNextSiegeWarTimeDate	+= lSelectedDate * 60 * 60 * 24;
	//}

	//ulNextSiegeWarTimeDate	+= (lSelectedHour - (newtime->tm_hour - (12 + AGPMSIEGEWAR_START_WAR_TIME))) * 60 * 60;

	*pullTime	= ullNextSiegeWarTimeDate;

	return TRUE;
}

BOOL AgcmUISiegeWar::SetAttackSelectPageSlot(INT32 lSelectPage)
{
	for (int i = 1; i <= 9; ++i)
	{
		if (i == lSelectPage)
		{
			m_pcsAgcmUIManager2->ThrowEvent(m_lEventSelectAttackPageSlot[i - 1], TRUE);
			break;
		}
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::SetDefenseSelectPageSlot(INT32 lSelectPage)
{
	for (int i = 1; i <= 9; ++i)
	{
		if (i == lSelectPage)
		{
			m_pcsAgcmUIManager2->ThrowEvent(m_lEventSelectDefensePageSlot[i - 1], TRUE);
			break;
		}
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::IsInAttackGuild(const CHAR* szGuildID)
{
	if(!szGuildID || strlen(szGuildID) < 1)
		return FALSE;

	for(int i = 0; i < AGPMSIEGEWAR_TOTAL_ATTACK_GUILD; ++i)
	{
		if(strcmp(m_astAttackGuildList[i].szGuildName, szGuildID) == 0)
			return TRUE;
	}

	return FALSE;
}

BOOL AgcmUISiegeWar::IsInDefenseGuild(const CHAR* szGuildID)
{
	if(!szGuildID || strlen(szGuildID) < 1)
		return FALSE;

	for(int i = 0; i < AGPMSIEGEWAR_TOTAL_DEFENSE_GUILD; ++i)
	{
		if(strcmp(m_astDefenseGuildList[i].szGuildName, szGuildID) == 0)
			return TRUE;
	}

	return FALSE;
}

BOOL AgcmUISiegeWar::SetCallbackReceiveAttackGuildList(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGCMUISIEGEWAR_CB_RECEIVE_ATTACK_GUILD_LIST, pfCallback, pClass);
}

BOOL AgcmUISiegeWar::SetCallbackReceiveEnemyGuildList(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGCMUISIEGEWAR_CB_RECEIVE_ENEMY_GUILD_LIST, pfCallback, pClass);
}

BOOL AgcmUISiegeWar::OpenSiegeWarInfo()
{
	//if( !m_bInfoOpenUI )
	//{
	//	//m_pcsAgcmUIManager2->ThrowEvent( m_lEventOpenSiegeWarInfoUI );
	//	m_pcsAgcmUIManager2->ThrowEvent( m_lEventGetSiegeWarInfoEditControl );
	//	m_pcsAgcmUIManager2->ThrowEvent( m_lEventGetRemainTimeEditControl );

	//	if( m_pcsSiegeWarInfoEdit )
	//	{
	//		m_pcsSiegeWarInfoEdit->SetLineDelimiter("\n");
	//	}

	//	m_bInfoOpenUI = TRUE;
	//}

	return TRUE;
}

void AgcmUISiegeWar::CloseSiegeWarInfo()
{
	//if( m_bInfoOpenUI )
	//{
	//	m_pcsAgcmUIManager2->ThrowEvent( m_lEventCloseSiegeWarInfoUI );
	//	m_bInfoOpenUI	= FALSE;
	//}
}

void AgcmUISiegeWar::SetRemainTime( UINT32 ulStart, UINT32 ulClockCount )
{
	if( !m_pcsRemainTimeEdit )		return;

	CHAR	szBuffer[128];
	ZeroMemory( szBuffer, sizeof(szBuffer) );

	INT32 lRemainTimeSec = (ulStart + ( 1000 * 60 * 60 ) - ulClockCount) / 1000;
	lRemainTimeSec = max( 0, lRemainTimeSec );

	if (g_eServiceArea == AP_SERVICE_AREA_GLOBAL)
		sprintf(szBuffer, "%2d : %2d", lRemainTimeSec / 60, lRemainTimeSec % 60);
	else
		sprintf(szBuffer, "%2d%s %2d%s", lRemainTimeSec / 60, m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_MINUTE), 
										 lRemainTimeSec % 60, m_pcsAgcmUIManager2->GetUIMessage(UI_MESSAGE_GUILD_TIME_UNIT_SECOND));
										 
	m_pcsRemainTimeEdit->SetText( szBuffer );
}

void AgcmUISiegeWar::SetInfoEdit( char* szText, UINT32 ulColor )
{
	if( !m_pcsSiegeWarInfoEdit ) return;
	_SetMessageText( szText, ulColor );
}

BOOL AgcmUISiegeWar::SetTextEffect(CHAR *pszString)
{
	if (!pszString || strlen(pszString) <= 0)
		return FALSE;

	static	INT32 lFont1	= 6;
	static	INT32 lDuration	= 5000;

	static	FLOAT fScale1	= 0.7f;

	static	UINT32 uColor	= 0xff00ff00;

	static	INT32 lOffsetY1	= INT32(m_pcsAgcmUIManager2->m_csEffectWindow.h * 0.19f);

	m_pcsAgcmUIManager2->m_csEffectWindow.SetEffectSubText("", lFont1, lOffsetY1, FALSE, 0x00, fScale1);
	m_pcsAgcmUIManager2->m_csEffectWindow.SetEffectAboveText("", lFont1, lOffsetY1, FALSE, 0x00, fScale1);

	m_pcsAgcmUIManager2->m_csEffectWindow.SetEffectText(
		pszString,
		lFont1,
		lOffsetY1,
		FALSE,
		uColor,
		fScale1,
		lDuration,
		TRUE,
		TRUE);

	return TRUE;
}

BOOL AgcmUISiegeWar::SetSelectedDefenseGuild()
{
	if (!m_pcsSiegeWar)
		return FALSE;

	for (int i = 0; i < AGPMSIEGEWAR_MAX_GUILD_LIST_PER_PAGE; ++i)
	{
		if (m_pcsSiegeWar->m_csDefenseGuild.GetObject(m_astDefenseApplGuildList[i].szGuildName))
			m_pcsAgcmUIManager2->ThrowEvent(m_lEventDisableDefenseGuild, i);
		else if (m_lSelectedDefenseGuildList == i)
			m_pcsAgcmUIManager2->ThrowEvent(m_lEventSelectedDefenseGuild, i);
		else
			m_pcsAgcmUIManager2->ThrowEvent(m_lEventNormalDefenseGuild, i);
	}

	return TRUE;
}

BOOL AgcmUISiegeWar::SetUseCharName(CHAR *pszCharName, BOOL bUse)
{
	if (!pszCharName)
		return FALSE;

	CopyMemory(m_szUseCharName, pszCharName, sizeof(m_szUseCharName) - 1);
	m_pcsAgcmUIManager2->RefreshUserData(m_pcsUserDataUseCharName);

	if (bUse)
		m_pcsAgcmUIManager2->ThrowEvent(m_lEventUseCharNameMessage);
	else
		m_pcsAgcmUIManager2->ThrowEvent(m_lEventRepairCharNameMessage);

	return TRUE;
}

BOOL AgcmUISiegeWar::IsUsableItemForCatapult(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	for (int i = 0; i < 8; ++i)
	{
		if (pcsItem->m_pcsItemTemplate->m_lID == m_pcsAgpmSiegeWar->m_acsSiegeWarObjectInfo[AGPD_SIEGE_MONSTER_CATAPULT].m_lNeedItemTIDforActive[i])
			return TRUE;
	}

	return FALSE;
}

BOOL AgcmUISiegeWar::IsUsableItemForAtkResTower(AgpdItem *pcsItem)
{
	if (!pcsItem)
		return FALSE;

	for (int i = 0; i < 8; ++i)
	{
		if (pcsItem->m_pcsItemTemplate->m_lID == m_pcsAgpmSiegeWar->m_acsSiegeWarObjectInfo[AGPD_SIEGE_MONSTER_ATTACK_RESURRECTION_TOWER].m_lNeedItemTIDforActive[i])
			return TRUE;
	}

	return FALSE;
}

BOOL	AgcmUISiegeWar::UpdateSiegeWarInfoUI( AgpdSiegeWar* pSiegeWarInfo, BOOL bBattleInfo )
{
	//if( bBattleInfo && !m_bInfoOpenUI )
	//	m_pcsAgcmUIManager2->ThrowEvent( m_lEventCloseSiegeWarInfoUI );

	//AgpdCharacter* pMy = m_pcsAgcmCharacter->GetSelfCharacter();
	//if( !pMy )		return FALSE;

	//AgpdSiegeWar* pInfo = m_pcsAgpmSiegeWar->GetSiegeWarInfo( pMy );
	//if( !pInfo )	return FALSE;
	//if( pSiegeWarInfo && pSiegeWarInfo != pInfo )	return FALSE;

	//if( !m_pcsAgpmSiegeWar->IsStarted( pInfo ) )	return FALSE;
	//if( m_pcsAgpmSiegeWar->IsAttackGuild( pMy, pInfo ) || m_pcsAgpmSiegeWar->IsDefenseGuild( pMy, pInfo ) )
	//{
	//	if( !m_pcsSiegeWarInfoEdit )
	//	{
	//		m_pcsAgcmUIManager2->ThrowEvent( m_lEventGetSiegeWarInfoEditControl );
	//		m_pcsAgcmUIManager2->ThrowEvent( m_lEventGetRemainTimeEditControl );
	//	}

	//	//m_pcsAgcmUIManager2->ThrowEvent( m_lEventOpenSiegeWarInfoUI );

	//	return TRUE;
	//}

	return FALSE;
}

BOOL AgcmUISiegeWar::_SetMessageText( CHAR* pText, UINT32 nColor )
{
	if( !pText ) return FALSE;

	//memset( m_strMessage, 0, sizeof( CHAR ) * 1024 );
	//strcpy( m_strMessage, pText );
	//m_nTextColor = nColor;

	//// 이상하게 이 알림메세지는 다른 에디트들과 달리 최초 한줄이 스크롤되지 않는다.
	//// 그래서 일부러 앞에 \n을 넣어서 한줄 더 내려주었다..
	//char strString[ 1024 ] = { 0, };
	//sprintf( strString, "\n<C%d>%s\n", m_nTextColor, m_strMessage );

	//m_pcsSiegeWarInfoEdit->SetLineDelimiter("\n");
	//m_pcsSiegeWarInfoEdit->SetText( strString );
	return TRUE;
}

BOOL AgcmUISiegeWar::_SetMsgTextToEditBox( INT32 nScrollValue )
{
	if(m_pcsSiegeWarInfoEdit)
		m_pcsSiegeWarInfoEdit->OnMouseWheel( nScrollValue * -1 );
	return TRUE;
}


BOOL AgcmUISiegeWar::_SelfPrintShrineMessageChat(std::string strMessage)
{
	m_listMessageChat.push_back(strMessage);
	return TRUE;
}