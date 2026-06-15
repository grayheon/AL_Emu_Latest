#if !defined(AGPM_SHRINE_BATTLE)
#define AGPM_SHRINE_BATTLE

#include "ApBase.h"
#include "ApModule.h"
#include "AgpmCharacter.h"
#include "AgpmSkill.h"
#include "AgpmAI2.h"
#include "AgpdShrineBattle.h"
#include "AgppShrineBattle.h"
#include <AuXmlParser.h>
#include "AgpmWarInfo.h"

class AgpmBattleGround;

typedef enum AGPM_SHRINEBATTLE_CALLBACK
{
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_INFORMATION_REQUEST	= 0,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_INFORMATION,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_INFORMATION_TOTAL,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_TIMEBAR_STATE,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_STATUS_UPDATE,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_TRYIMPRINT,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_TRYIMPRINT_RESULT,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_MESSAGE_CODE,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_JOIN_ANSWER,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_READY_TO_START,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_PVP_RECORD,
	AGPMSHRINEBATTLE_CALLBACK_SHRINEPOINT_RACE_RECORD,
};

enum	eShrineRegionIndex
{
	REGIONINDEX_SHRINEBATTLE_LIGHTNING		= 236,
	REGIONINDEX_SHRINEBATTLE_SIRIPUS			,
	REGIONINDEX_SHRINEBATTLE_WATER				,
	REGIONINDEX_SHRINEBATTLE_LAVA				,
	REGIONINDEX_SHRINEBATTLE_WIND				,
	REGIONINDEX_SHRINEBATTLE_POISON				,
	REGIONINDEX_SHRINEBATTLE_FIRE				,
	REGIONINDEX_SHRINEBATTLE_SPIRIT				,

	REGIONINDEX_SHRINEBATTLE_LIGHTNING_HUMAN	,
	REGIONINDEX_SHRINEBATTLE_LIGHTNING_ORC		,
	REGIONINDEX_SHRINEBATTLE_LIGHTNING_MOONELF	,
	REGIONINDEX_SHRINEBATTLE_LIGHTNING_SCION	,

	REGIONINDEX_SHRINEBATTLE_SIRIPUS_HUMAN		,
	REGIONINDEX_SHRINEBATTLE_SIRIPUS_ORC		,
	REGIONINDEX_SHRINEBATTLE_SIRIPUS_MOONELF	,
	REGIONINDEX_SHRINEBATTLE_SIRIPUS_SCION		,

	REGIONINDEX_SHRINEBATTLE_WATER_HUMAN		,
	REGIONINDEX_SHRINEBATTLE_WATER_ORC			,
	REGIONINDEX_SHRINEBATTLE_WATER_MOONELF		,
	REGIONINDEX_SHRINEBATTLE_WATER_SCION		,
	
	REGIONINDEX_SHRINEBATTLE_LAVA_HUMAN			,
	REGIONINDEX_SHRINEBATTLE_LAVA_ORC			,
	REGIONINDEX_SHRINEBATTLE_LAVA_MOONELF		,
	REGIONINDEX_SHRINEBATTLE_LAVA_SCION			,

	REGIONINDEX_SHRINEBATTLE_WIND_HUMAN			,
	REGIONINDEX_SHRINEBATTLE_WIND_ORC			,
	REGIONINDEX_SHRINEBATTLE_WIND_MOONELF		,
	REGIONINDEX_SHRINEBATTLE_WIND_SCION			,
	
	REGIONINDEX_SHRINEBATTLE_POISON_HUMAN		,
	REGIONINDEX_SHRINEBATTLE_POISON_ORC			,
	REGIONINDEX_SHRINEBATTLE_POISON_MOONELF		,
	REGIONINDEX_SHRINEBATTLE_POISON_SCION		,
	
	REGIONINDEX_SHRINEBATTLE_FIRE_HUMAN			,
	REGIONINDEX_SHRINEBATTLE_FIRE_ORC			,
	REGIONINDEX_SHRINEBATTLE_FIRE_MOONELF		,
	REGIONINDEX_SHRINEBATTLE_FIRE_SCION			,
	
	REGIONINDEX_SHRINEBATTLE_SPIRIT_HUMAN		,
	REGIONINDEX_SHRINEBATTLE_SPIRIT_ORC			,
	REGIONINDEX_SHRINEBATTLE_SPIRIT_MOONELF		,
	REGIONINDEX_SHRINEBATTLE_SPIRIT_SCION		,

	REGIONINDEX_SHRINEBATTLE_LIGHTNING_OUTSIDE	,
	REGIONINDEX_SHRINEBATTLE_SIRIPUS_OUTSIDE	,
	REGIONINDEX_SHRINEBATTLE_WATER_OUTSIDE		,
	REGIONINDEX_SHRINEBATTLE_LAVA_OUTSIDE		,
	REGIONINDEX_SHRINEBATTLE_WIND_OUTSIDE		,
	REGIONINDEX_SHRINEBATTLE_POISON_OUTSIDE		,
	REGIONINDEX_SHRINEBATTLE_FIRE_OUTSIDE		,
	REGIONINDEX_SHRINEBATTLE_SPIRIT_OUTSIDE		,

	REGIONINDEX_SHRINEBATTLE_MAX,
};

#define SHRINEBATTLEPLAYER_MAX_PER_RACE			5//15

struct AgpdShrineReward
{
	typedef vector<INT32>  SkillVector;

	struct ItemInfo
	{
		UINT32		m_ulCount;
		UINT32		m_ulTimeLimit;
		BOOL		m_bUseTimeLimit;

		ItemInfo()
		{
			m_ulCount = 0;
			m_ulTimeLimit = 0;
			m_bUseTimeLimit = FALSE;
		}
		
	};

	struct Reward
	{
		map<INT32, SkillVector>		m_mapSkillList;				// ShrineCharacteTID, SkillTid
		map<INT32, ItemInfo>		m_mapItem;					// ItemTid, ItemInfo

		INT32						m_nCharismaPoint;		// CharismaPoint

		Reward()
		{
			m_mapSkillList.clear();
			m_mapItem.clear();

			m_nCharismaPoint = 0;
		}
	};

	Reward					m_stCommon;	
	Reward					m_stWinRace;
	Reward					m_stCarvingCharacter;
};



class AgpmShrineBattle : public ApModule
{
public:
	AgpmCharacter*		m_pAgpmCharacter;
	AgpmSkill*			m_pAgpmSkill;
	AgpmBattleGround*	m_pAgpmBattleGround;

	AgpdShrineBattle	m_stShrineBattle;
	AuXmlDocument		m_XmlData;

	MTRand				m_csRand;

	AgpdShrineReward	m_stShrineReward;


public:
	AgpmShrineBattle();
	virtual ~AgpmShrineBattle();


	
	BOOL OnAddModule();
	BOOL OnInit();
	BOOL OnDestroy();

	BOOL LoadShrineBattleData();

	BOOL StreamReadShrineRegionData(CHAR* pszFile, BOOL bDecryption);
	BOOL StreamReadShrinePointData(CHAR* pszFile, BOOL bDecryption);
	BOOL StreamReadShrineMessageData(const string &pszName);

	BOOL ShuffleShrineRegion();
	BOOL RandomPlaceShrineRegion(AgpdShrinePoint *pstShrinePoint);

	BOOL IsInShrineRegion(AgpdCharacter *pcsCharacter);
	BOOL IsInShrineRegionByIndex(AgpdCharacter *pcsCharacter);
	BOOL IsInShrineRegionByIndex( UINT32 nRegionIndex);

	BOOL IsInShrineRegionPast(AgpdCharacter *pcsCharacter);
	BOOL IsInShrineRegionPast(UINT32 m_nPrevRegionIndex);

	BOOL IsInShrineRegion(INT32 lRegionIndex);

	IterShrinePoint FindShrinePointAreaInvalveCharacter(UINT32 nRegionIndex);
	BOOL IsShrineBattleOpenAtThisArea(UINT32 nRegionIndex);

	//JK_쉬라인추가 케릭별 부활위치
	UINT32 FindResurrectionIndexInShrineArea(AgpdCharacter *pcsCharacter );
	//JK_쉬라인추가 케릭터 접속시 바인딩 될 리전에서 부활위치를 찾는다.
	UINT32 FindResurrectionIndexInShrineArea(AgpdCharacter *pcsCharacter, UINT32 nBindingRegionIndex );


	BOOL IsInTryImprintRange(AgpdShrinePoint *pcsShrinePoint, AgpdCharacter *pcsCharacter);

	BOOL OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg);
	BOOL OnProcessShrinePointTimeBarUpdate(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointStatusUpdate(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointInformationRequest(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointInformation(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointInformationTotal(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointTryImprint(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointTryImprintResult(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointMessageCode(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointJoinAnswer(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointReadyToStart(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointPvPRecord(PACKET_AGPP_SHRINEBATTLE* pPacket);
	BOOL OnProcessShrinePointRaceRecord(PACKET_AGPP_SHRINEBATTLE* pPacket);

	BOOL SetCallBackShrinePointTimeBarUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointStatusUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointInformationRequest(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointInformation(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointInformationTotal(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointTryImprint(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointTryImprintResult(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointMessageCode(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointJoinAnswer(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointReadyToStart(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointPvPRecord(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallBackShrinePointRaceRecord(ApModuleDefaultCallBack pfCallback, PVOID pClass);
};

#endif