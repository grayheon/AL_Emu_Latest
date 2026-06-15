#pragma once
#include "AgcModule.h"
#include "AgpmWarInfo.h"
#include "AgcmUIWarInfo.h"
#include <map>

#define WAR_INFO_CLOSE_TIME 30


class AgcmWarInfo : public AgcModule
{
private:
	AgpmWarInfo *m_pcsAgpmWarInfo;
	AgcmUIWarInfo *m_pcsAgcmUIWarInfo;

	WarInfoDataMap m_mapWarInfoData[WarInfoTypeMax];
	WarInfoDataMap m_mapWarAddMessageData[WarInfoTypeMax];

	INT8				m_cLastWarType;
	INT8				m_cLastWarStep;
	INT8				m_cLastMessageStep;
	INT8				m_cLastAddInfoFlag;
	INT8				m_cLastRemainTime;


	DWORD				m_EndTime;
	BOOL				m_bWarEnd;

	INT32				m_lCharacterWinCount;
	INT32				m_lCharacterLoseCount;
	INT32				m_lRaceWinCount;
	INT32				m_lRaceLoseCount;
	
public:

	AgcmWarInfo();
	virtual ~AgcmWarInfo();	

	// 필수 함수
	BOOL					OnInit();
	BOOL					OnAddModule();
	BOOL					OnIdle(UINT32 ulClockCount);
	BOOL					OnDestroy();




	BOOL					ReadStreamWarInfo( CHAR *szFile, BOOL bDecryption );

	static BOOL				CBOperationUpdateWarState(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL				CBOperationUpdateTimeStep(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL				CBOperationUpdateAddInfo(PVOID pData, PVOID pClass, PVOID pCustData);

	static BOOL				CBOperationUpdateCharacterPvPInfo(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL				CBOperationUpdateRacePvPInfo(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL				CBOperationEndWar(PVOID pData, PVOID pClass, PVOID pCustData);

	static BOOL				CBOperationUpdateFactor(PVOID pData, PVOID pClass, PVOID pCustData);

	BOOL					EndWar();
	BOOL					StartWar();

	BOOL					UpdatePvPInfo();

};

