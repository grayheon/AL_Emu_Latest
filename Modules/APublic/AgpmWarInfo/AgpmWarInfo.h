#pragma once
#include "ApBase.h"
#include "AuPacket.h"
#include "ApModule.h"

enum eAgpmWarInfoType
{
	SiegeWar = 0,
	BattleGround,
	ShrineBattle,
	ArchlordBattle,
	WarInfoTypeMax
};

typedef enum _eAgpmWarInfoPacketType
{
	AGPMWARINFO_PACKET_UPDATE_WAR_STATE = 0,
	AGPMWARINFO_PACKET_REQUEST_WARINFO,
	AGPMWARINFO_PACKET_UPDATE_TIME,
	AGPMWARINFO_PACKET_UPDATE_ADD_INFO,
	AGPMWARINFO_PACKET_CHARACTER_PVP_INFO,
	AGPMWARINFO_PACKET_RACE_PVP_INFO,
	AGPMWARINFO_PACKET_END_WAR,
	AGPMWARINFO_PACKET_FACTOR,
	AGPMWARINFO_PACKET_MAX
};


struct AgpdRaceScore
{
	LONG		m_lWinScore;
	LONG		m_lLoseScore;

	AgpdRaceScore()
		: m_lWinScore(0), m_lLoseScore(0)
	{

	}

	BOOL IncreaseWinScore()
	{
		InterlockedIncrement(&m_lWinScore);
		return TRUE;
	}

	BOOL IncreaseLoseScore()
	{
		InterlockedIncrement(&m_lLoseScore);
		return TRUE;
	}
};


class AgpmWarInfo : public ApModule
{
public:
	AgpmWarInfo();
	~AgpmWarInfo();

	BOOL					OnAddModule();
	BOOL					OnInit();
	BOOL					OnDestroy();

	BOOL				    OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg);


	BOOL					SetCallbackUpdateWarState(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL					SetCallbackRequestWarInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL					SetCallbackUpdateTimeInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL					SetCallbackUpdateAddInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass);

	BOOL					SetCallbackUpdateCharacterPvPInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL					SetCallbackUpdateRacePvPInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass);

	BOOL					SetCallbackEndWar(ApModuleDefaultCallBack pfCallback, PVOID pClass);

	BOOL					SetCallbackFactor(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	

	
private:
	AuPacket				m_csPacket;



};