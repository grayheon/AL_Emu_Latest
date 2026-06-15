#pragma once
#include "AgsEngine.h"
#include "AgpmWarInfo.h"
#include "AgpmCharacter.h"
#include "AgsmCharacter.h"

class AgpdCharacterWarPVPData
{
private:
	LONG m_lWinCount;
	LONG m_lLoseCount;

	
public:
	AgpdCharacterWarPVPData()
		: m_lWinCount(0), m_lLoseCount(0)
	{

	}
	virtual ~AgpdCharacterWarPVPData(){}

	VOID Init()
	{
		m_lWinCount = 0;
		m_lLoseCount = 0;
	}

	VOID AddWinCount()
	{
		InterlockedIncrement(&m_lWinCount);
	}

	VOID AddLoseCount()
	{
		InterlockedIncrement(&m_lLoseCount);
	}

	LONG GetWinCount()
	{
		return m_lWinCount;
	}

	LONG GetLoseCount()
	{
		return m_lLoseCount;
	}
};

class AgsmWarInfo : public AgsModule
{
private:
	AgpmWarInfo			*m_pcsAgpmWarInfo;
	AgpmCharacter		*m_pAgpmCharacter;
	AgsmCharacter		*m_pAgsmCharacter;

	INT8				m_cLastWarType;
	INT8				m_cLastWarStep;
	INT8				m_cLastMessageStep;
	INT8				m_cLastAddInfoFlag;
	INT8				m_cLastRemainTime;

	INT32				m_nIndexADCharPvPData;

	AgpdRaceScore			m_arRaceScore[AURACE_TYPE_MAX];


public:
	AgsmWarInfo();
	virtual ~AgsmWarInfo();

	BOOL				OnAddModule();
	BOOL				OnInit();
	BOOL				OnDestroy();

	static BOOL			CBOperationRequestWarState(PVOID pData, PVOID pClass, PVOID pCustData);

	BOOL				SendWarInfo(INT8 cWarType, INT8 cWarStep, INT8 cMessageStep);
	BOOL				SendWarInfoToCharacter(AgpdCharacter *pcsCharacter, INT8 cWarType, INT8 cWarStep, INT8 cMessageStep);

	BOOL				SendTimeStep(INT8 cWarType, INT8 cTime);
	BOOL				SendTimeStepToCharacter(INT8 cWarType, AgpdCharacter *pcsCharacter, INT8 cTime);

	BOOL				SendAddInfo(INT8 cWarType, INT8 cAddInfoFlag, INT8 cUpdateNum);

	BOOL				SendLastInfoForChar(AgpdCharacter *pcsCharacter);

	BOOL				StartWar(INT8 cWarType);

	static BOOL			ConstructorPvPData(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL			DestructorPvPData(PVOID pData, PVOID pClass, PVOID pCustData);

	AgpdCharacterWarPVPData*	GetCharacterData(AgpdCharacter* pcsCharacter);

	BOOL				ProcessPvPResult(INT8 cWarType, AgpdCharacter *pcsWinCharacter, AgpdCharacter* pcsLoseCharacter);

	BOOL				InitRaceScore();
	BOOL				IncreaseRaceWinScore(AuRaceType eRaceType);
	BOOL				IncreaseRaceLoseScore(AuRaceType eRaceType);

	BOOL				SendRacePvPInfoToAll(INT8 cWarType);

	BOOL				InitADCharacterPvPInfo();

	INT8				GetLastRemainTime(){return m_cLastRemainTime;}

	BOOL				SendEndWarInfo(INT8 cWarType);
	inline VOID			SetTimeStep(INT8 cTime){m_cLastRemainTime = cTime;}

	BOOL				SendEndWarInfoToCharacter(INT8 cWarType, AgpdCharacter *pcsCharacter);
};