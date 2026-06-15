#if !defined(_AGSM_SHRINEBATTLE)
#define _AGSM_SHRINEBATTLE

#include "AgpmShrineBattle.h"
#include "AgsEngine.h"
#include "AgsmWarInfo.h"
#include "AgpmEventTeleport.h"

#include "AuXmlParser.h"

class AgpmCharacter;
class AgsmCharacter;
class AgpmSkill;
class AgsmSkill;
class AgsmAreaChatting;
class ApmMap;
class AgpmFactors;
class ApmEventManager;
class AgsmDeath;
class AgpmEventSpawn;
class AgsmEventSpawn;
class AgsmSystemMessage;
class AgpmItem;
class AgsmItem;
class AgsmItemManager;
class AgpmBattleGround;
class AgsmBattleGround;
class AgsmPvP;
class AgpmPvP;

const int AGSM_MAX_SHRINEPOINT_CHARACTER =	500;

const int AGSM_SHRINE_READY_TO_STARTTIME =	3;//10;			// 분단위 쉬라인 시작 전 시간 

#define RACE_PVP_SEND_TERM_MIN	3 

#define	LOG_ERROR_AGSM_SHRINE_BATTLE				"log\\ShrineBattleError.log"

enum AgsmShrineRewardType
{
	AGSM_SHRINE_REWARD_NONE = 0	,
	AGSM_SHRINE_REWARD_COMMON	,
	AGSM_SHRINE_REWARD_WIN_RACE	,
	AGSM_SHRINE_REWARD_CARVING	,
	AGSM_SHRINE_REWARD_MAX,
};

class AgsmShrineBattle : public AgsModule
{
public:
	AgpmShrineBattle		*m_pAgpmShrineBattle;
	AgpmCharacter			*m_pAgpmCharacter;
	AgsmCharacter			*m_pAgsmCharacter;
	AgpmSkill				*m_pAgpmSkill;
	AgsmSkill				*m_pAgsmSkill;
	AgsmAreaChatting		*m_pAgsmAreaChatting;
	ApmMap					*m_pApmMap;
	AgpmFactors				*m_pAgpmFactors;
	ApmEventManager			*m_pAgpmEventManager;
	AgsmDeath				*m_pAgsmDeath;
	AgpmEventSpawn			*m_pAgpmEventSpawn;
	AgsmEventSpawn			*m_pAgsmEventSpawn;
	AgsmSystemMessage		*m_pAgsmSystemMessage;
	AgpmItem				*m_pAgpmItem;
	AgsmItem				*m_pAgsmItem;
	AgsmItemManager			*m_pAgsmItemManager;
	AgpmBattleGround		*m_pAgpmBattleGround;
	AgsmBattleGround		*m_pAgsmBattleGround;
	AgsmPvP					*m_pAgsmPvP;
	AgpmEventBinding		*m_pAgpmEventBinding;

	AgsmWarInfo				*m_pAgsmWarInfo;
	AgpmEventTeleport		*m_pAgpmEventTeleport;
	AgpmPvP					*m_pAgpmPvP;

	UINT32					m_ulCheckClockCount;
	UINT32					m_ulPreCheckClockCount;
	UINT32					m_ulLastRacePvPSendTime;

	MTRand					m_csRandom;
	BOOL					m_bAllowMove;					// 쉬라인 지역으로 소환가능한지 여부 (ReadyToStart때부터는 열어줘야한다.)

	
private:

	BOOL					m_bLoadComplete;

public:
	AgsmShrineBattle();
	virtual ~AgsmShrineBattle();

	/************************************************************************/
	/* About xml                                                            */
	/************************************************************************/
	AuXmlNode*				GetRootNode();

	BOOL OnAddModule();
	BOOL OnInit();
	BOOL OnDestroy();

	BOOL OnIdle(UINT32 ulClockCount);

	BOOL ProcessShrineBattle(UINT32 ulClockCount);

	BOOL CloseCheck(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount);
	BOOL SendRemainTimeCheck(UINT32 ulClockCount);

	BOOL OnStatusOpen(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount);
	BOOL OnStatusTryImprint(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount);
	BOOL OnStatusImprint(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount);
	BOOL OnStatusTryOccupy(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount);
	BOOL OnStatusOccupy(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount);
	BOOL OnStatusEnd(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount);
	BOOL OnStatusNormal(AgpdShrinePoint* pstShrinePoint, UINT32 ulClockCount);

	BOOL OpenAllShrine();
	BOOL OpenAllShirineByCommand();
	BOOL CloseAllShrineByCommand();

	BOOL ShrineBattleRewardProcess(AgpdShrinePoint* pstShrinePoint);
	BOOL ProcessRewardCharismaPoint(AgpdShrinePoint* pstShrinePoint);
	BOOL StatusChange(AgpdShrinePoint* pstShrinePoint, INT32 nStatus, UINT32 ulClockCount, BOOL bSendMessage = TRUE);

	BOOL SendPacketShrineBattleInformationToCharacter(AgpdShrinePoint* pstShrinePoint, AgpdCharacter *pcsCharacter);
	BOOL SendPacketShrineBattleInformationToRegion(AgpdShrinePoint* pstShrinePoint);
	BOOL SendPacketShrineBattleInformationToWorld(AgpdShrinePoint* pstShrinePoint);
	BOOL SendPacketShrineBattleReadyToStartToWorld(INT32 lRestMinute, AgpdShrinePoint* pstShrinePoint);

	BOOL SendPacketShrineBattleStatusChangeToWorld(AgpdShrinePoint* pstShrinePoint, INT32 nPreStatus);

	BOOL SendPacketShrineBattleFullInformationToCharacter(AgpdCharacter *pcsCharacter);
	BOOL SendPacketShrineBattleFullInformationToWorld();
	BOOL SendPacketShrineBattleRacePvPInformationToWorld();

	BOOL SendPacketShrineBattleImprintResult(AgpdShrinePoint* pstShrinePoint, AgpdCharacter *pcsCharacter, INT32 nPrevStatus, BOOL bResult);

	BOOL SendPacketShrineBattleMessageCodeToCharacter(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid, AgpdCharacter* pcsCharacter);
	BOOL SendPacketShrineBattleMessageCodeToRace(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid, INT32 nToRaceID);
	BOOL SendPacketShrineBattleMessageCodeToRegion(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid, INT32 nRegionIndex);
	BOOL SendPacketShrineBattleMessageCodeToShrineArea(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid);
	BOOL SendPacketShrineBattleMessageCodeToWorld(INT32 eMessageCode, INT32 nShrinePointID, INT32 nRaceID, INT32 nPointCid);

	BOOL SendPacketShrineTimebarStatusToRegion(AgpdShrinePoint* pstShrinePoint, BOOL bAppear, INT32 nRegionIndex);
	BOOL SendPacketShrineTimebarStatusToCharacter(AgpdShrinePoint* pstShrinePoint, BOOL bAppear, AgpdCharacter *pcsCharacter);

	BOOL SendPacketInShrineRegion(PACKET_HEADER& pPacket, PACKET_PRIORITY ePriority, INT16 nFlag);

	static BOOL CBAddCharacterToMap(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBRemoveCharacter(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBRegionChange(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBShrineBattleInformationRequest(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBShrineBattleTryImprint(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBDeath(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBMoveCharacter(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBSpawnCharacter(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBShrineBattleJoinAnswer(PVOID pData, PVOID pClass, PVOID pCustData);

	static BOOL CBEnterGameWorld(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBTeleportLoading(PVOID pData, PVOID pClass, PVOID pCustData);


	static BOOL	OpenShrineDungeonTimer(INT32 lCID, PVOID pClass, UINT32 uDelay, PVOID pvData);		

	//JK_쉬라인 추가
	static	BOOL PartyIsOverGoHome(INT32 lCID, PVOID pClass, UINT32 uDelay, PVOID pvData);



	BOOL RegionChange(AgpdCharacter *pcsCharacter, INT16 nPrevRegionIndex);
	BOOL RemoveCharacter(AgpdCharacter *pcsCharacter);
	BOOL ShrineBattleTryImprint(AgpdCharacter *pcsCharacter, AgpdShrinePoint* pstShrinePoint);
	BOOL Death(AgpdCharacter *pcsCharacter, ApBase* pcsAttackerBase);
	BOOL Move(AgpdCharacter *pcsCharacter);


	BOOL SpawnShrineObject(AgpdShrinePoint* pstShrinePoint);
	BOOL SettingShrineObject(AgpdCharacter* pcsShrineCharacter);

	BOOL ShuffleShrineObjectSetting();
	BOOL ShuffleShrineObjectSetting(AgpdShrinePoint *pstShrinePoint);

	IterShrinePoint FindShrinePointInCharacter(AgpdCharacter* pcsCharacter);
	IterShrinePoint FindShrinePointSameRegion(AgpdCharacter* pcsCharacter);

	/************************************************************************/
	/* Global Renewel                                                       */
	/************************************************************************/

	BOOL				OpenShrine(AgpdShrinePoint *pcsShrinePoint);
	BOOL				ApplyReward(AgpdCharacter *pcsCharacter, AgpdCharacter *pcsShrineCharacter, AgsmShrineRewardType eType);

	BOOL				OpenShirineByCommand(INT32 lShrinePointID);
	AgpdCharacter*		GetShrineObjectInRegion(INT32 lRegionIndex);

	BOOL				OpenShrineDungeon();
	BOOL				EndShrineDungeon();

	BOOL				PvPDeath(AgpdCharacter *pWinCharacter, AgpdCharacter *pLoseCharacter);

	INT32				GetPlayerCountPerRace(AgpdShrineRegion* pcsShrineRegion, INT32 nRace );


	BOOL				CheckRacePvPSend(UINT32 ulClockCount);

	BOOL				IsShrineBattleOpenDay();
		
	BOOL				SendShrineWarInfo(AgpdShrinePoint *pcsShrinePoint, INT8 cWarStep, INT8 cMessageStep);
	BOOL				SendEndWar(AgpdShrinePoint *pcsShrinePoint);


	BOOL				SendPacketInShrineRegion(AgpdShrinePoint *pcsShrinePoint, PACKET_HEADER& pPacket);

	BOOL				SendPacketRacePvPInfoToShrineRegion(AgpdShrinePoint *pcsShrinePoint);
	BOOL				SendPacketToShrineAll(PACKET_HEADER& pPacket);

	BOOL				SendPacketRemainTime(AgpdShrinePoint *pstShrinePoint, INT8 cTime);

};

#endif