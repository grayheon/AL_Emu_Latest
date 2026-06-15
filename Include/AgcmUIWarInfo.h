#pragma once
#include "AgcModule.h"
#include "AgcmUiManager2.h"
#include "AgppWarinfo.h"


struct stWarInfoData
{
	string szStepMessage;
	string szCenterMessage[3];
	stWarInfoData()
	{
		szStepMessage.clear();
		szCenterMessage[0].clear();
		szCenterMessage[1].clear();
		szCenterMessage[2].clear();
	}
};

typedef std::map<INT32, stWarInfoData>			WarInfoDataMap;
typedef std::map<INT32, stWarInfoData>::const_iterator	WarInfoDataMapConstIter;

class AgcmUIWarInfo : public AgcModule
{
private:
	AgcmUIManager2	*m_pcsAgcmUIManager2;
	
	AgcdUI			*m_pWarInfoWindow;

	INT32			m_lEventOpenWarInfoUI;
	INT32			m_lEventCloseWarInfoUI;

	INT32			m_lEventGetStepMessageEditControl;
	INT32			m_lEventGetCenterMessageControl;
	INT32			m_lEventGetTimeEditControl;


	AcUIEdit		*m_pcsStepMessageEdit;
	AcUIEdit		*m_pcsCenterMessageEdit;
	AcUIEdit		*m_pcsTimeMessageEdit;


	AgcdUIUserData *m_pcsCharacterWinCount;
	AgcdUIUserData *m_pcsCharacterLoseCount;
	AgcdUIUserData *m_pcsRaceWinCount;
	AgcdUIUserData *m_pcsRaceLoseCount;

	INT32			m_lCharacterWinCount;
	INT32			m_lCharacterLoseCount;
	INT32			m_lRaceWinCount;
	INT32			m_lRaceLoseCount;

	CHAR			m_szFactor1[MAX_WARINFO_FACTOR_LENGTH+1];
	CHAR			m_szFactor2[MAX_WARINFO_FACTOR_LENGTH+1];
	CHAR			m_szFactor3[MAX_WARINFO_FACTOR_LENGTH+1];

	BOOL			m_bIsOpenUI;
	
	enum _AgcmUIWarInfo_Display_ID
	{
		AGCMUI_WARINFO_DISPLAY_ID_CHARACTER_WIN_COUNT = 0,
		AGCMUI_WARINFO_DISPLAY_ID_CHARACTER_LOSE_COUNT,
		AGCMUI_WARINFO_DISPLAY_ID_RACE_WIN_COUNT,
		AGCMUI_WARINFO_DISPLAY_ID_RACE_LOSE_COUNT,
	};


public:

	AgcmUIWarInfo( void );
	~AgcmUIWarInfo( void );

	BOOL			OnAddModule							( void );
	BOOL			OnDestroy							( void );
	BOOL			OnInit								( void );
	BOOL			OnIdle(UINT32 ulClockCount);


	BOOL			AddEvent							( void );
	BOOL			AddFunction							( void );
	BOOL			AddDisplay							( void );
	BOOL			AddUserData							( void );


	BOOL			OpenWarInfo();
	BOOL			CloseWarInfo();
	BOOL			SetStepMessage(const WarInfoDataMap &mapWarInfoData, INT8 cStep);
	BOOL			UpdateCenterMessage(const stWarInfoData &stWarInfo, const WarInfoDataMap &mapWarInfoAddData, INT8 cCenterMessageStep, INT8 cAddInfoFlag, BOOL bIsViewLastLine = FALSE);
	BOOL			UpdateTimeStep(const INT8 time);
	BOOL			SetLineAddInfo(INT8 cNewAddInfo);

	BOOL			SetTimeStep(const INT8 cTime);


	static BOOL		CBGetStepMessageEditControl(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);
	static BOOL		CBGetCenterMessageEditControl(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);
	static BOOL		CBGetTimeEditControl(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);

	
	static BOOL		CBDisplayPvPInfo(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue);

	void			SetPvPInfo(INT32 lCharacterWinCount, INT32 lCharacterLoseCount, INT32 lRaceWinCount, INT32 lRaceLoseCount);

	BOOL			SetFactor(CHAR *szFactor1, CHAR *szFactor2, CHAR *szFactor3);

	BOOL			IsWarInfoUIOpen(){return m_bIsOpenUI;}
};