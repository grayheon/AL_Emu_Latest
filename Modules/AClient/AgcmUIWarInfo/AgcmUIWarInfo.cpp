#pragma once
#include "AgcmUIWarInfo.h"
#include "AuTimeStamp.h"


AgcmUIWarInfo::AgcmUIWarInfo()
{
	SetModuleName("AgcmUIWarInfo");

	m_pcsAgcmUIManager2 = NULL;
	
	m_lEventOpenWarInfoUI = -1;
	m_lEventCloseWarInfoUI = -1;
	m_pcsStepMessageEdit = NULL;
	m_pcsCenterMessageEdit = NULL;
	m_pcsTimeMessageEdit = NULL;

	m_lEventGetStepMessageEditControl = -1;
	m_lEventGetCenterMessageControl = -1;
	m_lEventGetTimeEditControl = -1;
	

	m_lCharacterWinCount = 0;
	m_lCharacterLoseCount = 0;
	m_lRaceWinCount = 0;
	m_lRaceLoseCount =0;

	m_bIsOpenUI = FALSE;

}

AgcmUIWarInfo::~AgcmUIWarInfo()
{

}

BOOL AgcmUIWarInfo::OnAddModule()
{
	m_pcsAgcmUIManager2	= ( AgcmUIManager2* )GetModule( "AgcmUIManager2" );
	if(!m_pcsAgcmUIManager2) return FALSE;


	if (!AddEvent())
		return FALSE;

	if (!AddFunction())
		return FALSE;

	if (!AddUserData())
		return FALSE;

	if (!AddDisplay())
		return FALSE;

	m_pWarInfoWindow = m_pcsAgcmUIManager2->GetUI("UI_WarInfo");
	if(!m_pWarInfoWindow) return FALSE;


	return TRUE;
}


BOOL AgcmUIWarInfo::OnDestroy()
{
	return TRUE;
}

BOOL AgcmUIWarInfo::OnInit()
{
	
	return TRUE;
}

BOOL AgcmUIWarInfo::OnIdle(UINT32 ulClockCount)
{
	//if(m_cUpdatedTime != 0 && m_lTime < 10)
	//{
	//	UINT32 uiCurrentTime = AuTimeStamp::GetCurrentTimeStamp();

	//	UINT32 uiTimeDiff = (uiCurrentTime - m_uiLastReceiveTime)/60;

	//	if(uiTimeDiff != m_lTime)
	//	{
	//		m_lTime = uiTimeDiff;
	//		SetTimeStep(m_cUpdatedTime - m_lTime);
	//	}
	//}


	return TRUE;
}

BOOL AgcmUIWarInfo::AddEvent()
{

	m_lEventOpenWarInfoUI	= m_pcsAgcmUIManager2->AddEvent("WarInfoUIOpen");
	if (m_lEventOpenWarInfoUI < 0)
		return FALSE;
	m_lEventCloseWarInfoUI	= m_pcsAgcmUIManager2->AddEvent("WarInfoUIClose");
	if (m_lEventCloseWarInfoUI < 0)
		return FALSE;

	m_lEventGetStepMessageEditControl = m_pcsAgcmUIManager2->AddEvent("WarInfoGetStepMessageEditControl");
	if (m_lEventGetStepMessageEditControl < 0)
		return FALSE;
	m_lEventGetCenterMessageControl	= m_pcsAgcmUIManager2->AddEvent("WarInfoGetCenterMessageEditControl");
	if (m_lEventGetCenterMessageControl < 0)
		return FALSE;

	m_lEventGetTimeEditControl = m_pcsAgcmUIManager2->AddEvent("WarInfoGetTimeEditControl");
	if (m_lEventGetTimeEditControl < 0)
		return FALSE;

	return TRUE;
}

BOOL AgcmUIWarInfo::AddFunction()
{
	if (!m_pcsAgcmUIManager2->AddFunction(this, "WarInfoGetStepMessageEditControl", CBGetStepMessageEditControl, 1, "WarInfo Step Message Edit Control"))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "WarInfoGetCenterMessageEditControl", CBGetCenterMessageEditControl, 1, "WarInfo Center Message Edit Control"))
		return FALSE;

	if (!m_pcsAgcmUIManager2->AddFunction(this, "WarInfoGetTimeEditControl", CBGetTimeEditControl, 1, "WarInfo Get Time Edit Control"))
		return FALSE;

	return TRUE;
}

BOOL AgcmUIWarInfo::AddDisplay()
{
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "Character_Win_Count", AGCMUI_WARINFO_DISPLAY_ID_CHARACTER_WIN_COUNT, CBDisplayPvPInfo, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "Character_Lose_Count", AGCMUI_WARINFO_DISPLAY_ID_CHARACTER_LOSE_COUNT, CBDisplayPvPInfo, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "Race_Win_Count", AGCMUI_WARINFO_DISPLAY_ID_RACE_WIN_COUNT, CBDisplayPvPInfo, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	if (!m_pcsAgcmUIManager2->AddDisplay(this, "Race_Lose_Count", AGCMUI_WARINFO_DISPLAY_ID_RACE_LOSE_COUNT, CBDisplayPvPInfo, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;



	return TRUE;
}

BOOL AgcmUIWarInfo::AddUserData()
{

	if (!(m_pcsCharacterWinCount = m_pcsAgcmUIManager2->AddUserData("Character_Win_Count", &m_lCharacterWinCount, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32)))
		return FALSE;
	if (!(m_pcsCharacterLoseCount = m_pcsAgcmUIManager2->AddUserData("Character_Lose_Count", &m_lCharacterLoseCount, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32)))
		return FALSE;
	if (!(m_pcsRaceWinCount = m_pcsAgcmUIManager2->AddUserData("Race_Win_Count", &m_lRaceWinCount, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32)))
		return FALSE;
	if (!(m_pcsRaceLoseCount = m_pcsAgcmUIManager2->AddUserData("Race_Lose_Count", &m_lRaceLoseCount, sizeof(INT32), 1, AGCDUI_USERDATA_TYPE_INT32)))
		return FALSE;


	return TRUE;
}

BOOL AgcmUIWarInfo::OpenWarInfo()
{
	m_bIsOpenUI = TRUE;
	m_pcsAgcmUIManager2->ThrowEvent( m_lEventOpenWarInfoUI );
	m_pcsAgcmUIManager2->ThrowEvent( m_lEventGetStepMessageEditControl );
	m_pcsAgcmUIManager2->ThrowEvent( m_lEventGetCenterMessageControl );
	m_pcsAgcmUIManager2->ThrowEvent( m_lEventGetTimeEditControl);


	return TRUE;
}

BOOL AgcmUIWarInfo::CloseWarInfo()
{
	m_bIsOpenUI = FALSE;
	m_pcsAgcmUIManager2->ThrowEvent( m_lEventCloseWarInfoUI );
	return TRUE;
}

BOOL AgcmUIWarInfo::SetStepMessage(const WarInfoDataMap &mapWarInfoData, INT8 cStep)
{
	if(!m_pcsStepMessageEdit) return FALSE;
	string szString;
	for(WarInfoDataMapConstIter itr = mapWarInfoData.begin(); itr != mapWarInfoData.end(); ++itr)
	{
		const stWarInfoData &stData = itr->second;
		szString += stData.szStepMessage;
		szString += "<\\n>";
	}

	m_pcsStepMessageEdit->SetLineDelimiter("<\\n>");
	m_pcsStepMessageEdit->SetText(szString.c_str());

	m_pcsStepMessageEdit->SetLookLine(cStep * 2);

	return TRUE;
}

BOOL AgcmUIWarInfo::UpdateCenterMessage(const stWarInfoData &stWarInfo, const WarInfoDataMap &mapWarInfoAddData, INT8 cCenterMessageStep, INT8 cAddInfoFlag, BOOL bIsViewLastLine)
{
	if(!m_pcsCenterMessageEdit) return FALSE;

	CHAR szTemp[200];
	sprintf(szTemp, stWarInfo.szCenterMessage[cCenterMessageStep].c_str(), m_szFactor1, m_szFactor2, m_szFactor3);

	string szString;
	szString = szTemp;
	szString += "<\\n>";
	DWORD	dwFlagMask = 0x01;

	for(WarInfoDataMapConstIter itr = mapWarInfoAddData.begin(); itr != mapWarInfoAddData.end(); ++itr, dwFlagMask <<= 1)
	{
		if(cAddInfoFlag & dwFlagMask)
		{
			const stWarInfoData &stData = itr->second;
			szString += stData.szCenterMessage[0];
			szString += "<\\n>";
		}
	}

	m_pcsCenterMessageEdit->SetLineDelimiter("<\\n>");
	m_pcsCenterMessageEdit->SetText(szString.c_str());

	m_pcsCenterMessageEdit->SetLookLine(0);


	if(bIsViewLastLine == (BOOL)TRUE)
	{
		m_pcsCenterMessageEdit->SetLookEndLine();
	}

	return TRUE;
}



BOOL AgcmUIWarInfo::CBGetTimeEditControl(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIWarInfo	*pThis	= (AgcmUIWarInfo *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsTimeMessageEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;

	return TRUE;
}

BOOL AgcmUIWarInfo::CBGetStepMessageEditControl(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIWarInfo	*pThis	= (AgcmUIWarInfo *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsStepMessageEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;
	pThis->m_pcsStepMessageEdit->SetIsScrollByMouse(FALSE);
	pThis->m_pcsStepMessageEdit->SetCanScroll(TRUE);


	return TRUE;

}

BOOL AgcmUIWarInfo::CBGetCenterMessageEditControl(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || !pData1)
		return FALSE;

	AgcmUIWarInfo	*pThis	= (AgcmUIWarInfo *)	pClass;
	AgcdUIControl	*pcsEditControl	= (AgcdUIControl *)	pData1;

	if (pcsEditControl->m_lType != AcUIBase::TYPE_EDIT)
		return FALSE;

	pThis->m_pcsCenterMessageEdit	= (AcUIEdit *) pcsEditControl->m_pcsBase;
	pThis->m_pcsCenterMessageEdit->SetCanScroll(TRUE);


	return TRUE;
}

BOOL AgcmUIWarInfo::UpdateTimeStep(const INT8 cTime)
{
	SetTimeStep(cTime);	
	return TRUE;
}


BOOL AgcmUIWarInfo::SetTimeStep(const INT8 cTime)
{
	if(!m_pcsTimeMessageEdit) return FALSE;

	CHAR szTime[32] ={0,};
	CHAR *szMin = m_pcsAgcmUIManager2->GetUIMessage("UiMsg_WarInfo_Min");
	if(szMin)
	{
		sprintf(szTime, "%d %s", cTime, szMin);
	}

	m_pcsTimeMessageEdit->SetText(szTime);

	return TRUE;
}

BOOL AgcmUIWarInfo::CBDisplayPvPInfo(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue)
{
	AgcmUIWarInfo *pThis = (AgcmUIWarInfo *) pClass;
	INT32		lPvPCount = *((INT32 *)pData);

	if ( !pThis || !pData || eType != AGCDUI_USERDATA_TYPE_INT32 || !szDisplay)
		return FALSE;

	sprintf(szDisplay, "%d", lPvPCount );

	return TRUE;
}


void AgcmUIWarInfo::SetPvPInfo(INT32 lCharacterWinCount, INT32 lCharacterLoseCount, INT32 lRaceWinCount, INT32 lRaceLoseCount)
{

	m_lCharacterWinCount = lCharacterWinCount;
	m_lCharacterLoseCount = lCharacterLoseCount;
	m_lRaceWinCount = lRaceWinCount;
	m_lRaceLoseCount = lRaceLoseCount;

	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsCharacterWinCount);
	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsCharacterLoseCount);
	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsRaceWinCount);
	m_pcsAgcmUIManager2->SetUserDataRefresh(m_pcsRaceLoseCount);
}

BOOL AgcmUIWarInfo::SetFactor(CHAR *szFactor1, CHAR *szFactor2, CHAR *szFactor3)
{
	if(!szFactor1 || !szFactor2 || !szFactor3)
		return FALSE;
	strncpy(m_szFactor1, szFactor1, MAX_WARINFO_FACTOR_LENGTH);
	strncpy(m_szFactor2, szFactor2, MAX_WARINFO_FACTOR_LENGTH);
	strncpy(m_szFactor3, szFactor3, MAX_WARINFO_FACTOR_LENGTH);
	return TRUE;
}
