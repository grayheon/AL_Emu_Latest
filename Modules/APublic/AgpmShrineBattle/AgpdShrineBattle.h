// 쉬라인 공방전.
// 대규모 인원제한이 없는 24시간 상시 전투 가능한 시스템
// 랜스피어성을 중심으로 8개의 포인트가 존재.
// 포인트 주변의 최다 종족의 지속 데미지로 점령자가 결정됨
// 점령시 5분간 점령 불가, 포인트에 따른 버프(무한) 수여, 
// 종족 포인트 추가 획득 보너스

// 각 포인트를 구분할 data를 정의 해야한다.

// AgpdShrinePoint
// pointID, vector -> buffSkillTid, IsOccupation(기본 비점령), OccupationRace(기본 비점령)
// OccupationDate(년월일시분초), BattleDelayTime(점령후 재 점령 가능 시간차이 기본 5분)
// HealthPoint, PointCoordinate(포인트 중심 좌표), PointSize(중심으로 부터 좌우 상하 m, 직사각형)


// AgpdShrineBattle
// AgpdShrinePoint PointList, OccupationPointNumberByRace(각 종족별로 점령한 포인트 숫자)
// 

#if !defined(__AGPDSHIRINEBATTLE_H__)
#define  __AGPDSHIRINEBATTLE_H__

#include "ApDefine.h"
#include "ApBase.h"
#include "AgpdCharacter.h"
#include "AgpmWarInfo.h"

#include <vector>

const INT32 AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER	=	10;
const INT32 AGPDSHRINEBATTLE_POINT_MAX_BUFF_SKILL_NUM	=	5;
const INT32 AGPDSHRINEBATTLE_POINT_MAX_NAME_LENGTH		=	30;
const INT32 AGPDSHRINEBATTLE_RACE_NAME_MAX_LENGTH		=	30;
const INT32	AGPDSHRINEBATTLE_MESSAGE_MAX_LENGTH			=	255;

const INT32 AGPDSHRINEBATTLE_REWARDPOINT_CHARISMA_OCCUPY_CHARACTER		= 100;	// 최종각인자 카리스마 보상
const INT32 AGPDSHRINEBATTLE_REWARDPOINT_CHARISMA_WINRACE				= 100;	// 승리진영 카리스마 보상
const INT32 AGPDSHRINEBATTLE_REWARDPOINT_CHARISMA_LOSERACE				= 100;	// 패배진영 카리스마 보상

const INT32 AGPDSHRINE_IMPRINT_DISTANCE					=	8;					// 각인체와 각인자의 거리 단위는 m입니다.

const INT32 MAX_SHRINE_HAS_REGEION_NUM = 6;




const enum eAGPMSHRINEBATTLE_DATA_EXCEL_COLUMN
{
	AGPMSHRINEBATTLE_EXCEL_COLUMN_POINTID	= 0,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_POINT_STRING_NAME,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_STANDARD_START_TIME,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_INTERVAL_TIME,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_BATTLETIME_LIMIT,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_IMPRINTTIME_LIMIT,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_OCCUPYTIME_LIMIT,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_LEVEL_MIN,		//JK_쉬라인추가
	AGPMSHRINEBATTLE_EXCEL_COLUMN_LEVEL_MAX,
};

const enum eAGPMSHRINEBATTLE_REGIONDATA_EXCEL_COLUMN
{
	AGPMSHRINEBATTLE_EXCEL_COLUMN_REGIONINDEX	=0,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_FIXSHRINEREGION,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_FIXSHRINEPOINTID,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_HUMAN_BASE_POINTX,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_HUMAN_BASE_POINTZ,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_ORC_BASE_POINTX,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_ORC_BASE_POINTZ,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_MOONELF_BASE_POINTX,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_MOONELF_BASE_POINTZ,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_SCION_BASE_POINTX,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_SCION_BASE_POINTZ,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION1,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION2,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION3,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION4,
	AGPMSHRINEBATTLE_EXCEL_COLUMN_OUTSIDEREGION5,
};

enum AgpdShrineBattleStatus
{
	AGPDSHRINEBATTLE_STATUS_NORMAL		= 0,			// 아무것도 아닌상태이다
	AGPDSHRINEBATTLE_STATUS_OPEN,
	AGPDSHRINEBATTLE_STATUS_TRYIMPRINT,
	AGPDSHRINEBATTLE_STATUS_IMPRINT,
	AGPDSHRINEBATTLE_STATUS_TRYOCCUPY,
	AGPDSHRINEBATTLE_STATUS_OCCUPY,
	AGPDSHRINEBATTLE_STATUS_END,
};

enum AgpdShrineBattleSetOccupation
{
	AGPDSHRINEBATTLE_SETOCCUPATION_INVALID_RACE_NUMBER = 0,
	AGPDSHRINEBATTLE_SETOCCUPATION_INVALID_SHRINEPOINT_NUMBER,
	AGPDSHRINEBATTLE_SETOCCUPATION_ALREADY_SAMERACE_OCCUPIED,
	AGPDSHRINEBATTLE_SETOCCUPATION_SUCCESS,
};

typedef enum _eAgpmShrineBattleMessageCode
{
	AGPMSHRINEBATTLE_MESSAGE_CODE_NONE					= 0,
	AGPMSHRINEBATTLE_MESSAGE_CODE_OPEN_REMAIN_10MIN		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_OPEN_REMAIN_5MIN		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_ICON_ACTIVATED		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_OPEN_REMAIN_1MIN		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_START					,
	AGPMSHRINEBATTLE_MESSAGE_CODE_OPEN					,	//1 기존 MessageTID값
	AGPMSHRINEBATTLE_MESSAGE_CODE_OCCUPY_DURING_X_MIN	,
	AGPMSHRINEBATTLE_MESSAGE_CODE_CANNOT_USE_ICON		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT			,	//2
	AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_IMPRINT_FAIL		,	//8
	AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_OCCUPY			,	//4
	AGPMSHRINEBATTLE_MESSAGE_CODE_IMPRINT				,	//3
	AGPMSHRINEBATTLE_MESSAGE_CODE_COULD_BE_CANCEL		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_PROTECT_OCCUPIER		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_DEFEAT_OCCUPIER		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_TRY_OCCUPY_FAIL		,	//9
	AGPMSHRINEBATTLE_MESSAGE_CODE_VICTORY_MESSAGE		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_GET_CHARISMA			,
	AGPMSHRINEBATTLE_MESSAGE_CODE_GET_SHRINE_COIN		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_CLOSE					,	//6
	AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH				,
	AGPMSHRINEBATTLE_MESSAGE_CODE_OCCUPYIED_BY_RACE		,
	AGPMSHRINEBATTLE_MESSAGE_CODE_GET_DUNGEON_TICKET	,
	AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH_REMAIN_10MIN	,
	AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH_REMAIN_5MIN	,
	AGPMSHRINEBATTLE_MESSAGE_CODE_FINISH2				,
	AGPMSHRINEBATTLE_MESSAGE_CODE_CAN_USE_DUNGEON		,
	
//	AGPMSHRINEBATTLE_MESSAGE_CODE_OCCUPYING,		//5
//	AGPMSHRINEBATTLE_MESSAGE_CODE_ALL_OPEN,			//7
	
	AGPMSHRINEBATTLE_MESSAGE_CODE_MAX,
}eAgpmShrineBattleMessageCode;

class AgpdShrineRegion
{
public:
	INT32	m_nRegionIndex;
	INT32	m_nShrinePointID;				// 거기에 박혀있는 쉬라인 포인트 ID
	BOOL	m_bFixShrineRegion;

	AuPOS   m_stHumanBasePoint;
	AuPOS   m_stOrcBasePoint;
	AuPOS	m_stMoonElfBasePoint;
	AuPOS	m_stScionBasePoint;

	INT32	m_nHumanPlayerCount;	//JK_종족별 참여인원
	INT32	m_nOrcPlayerCount;
	INT32	m_nMoonElfPlayerCount;
	INT32	m_nScionPlayerCount;

	INT32	m_arrayRegionIndex[MAX_SHRINE_HAS_REGEION_NUM];
	AgpdRaceScore m_arrayRaceScore[AURACE_TYPE_MAX];

	UINT32	m_ulLastSendTimeRemainTime;

public:
	AgpdShrineRegion()
	{
		m_nRegionIndex = -1;
		m_nShrinePointID = -1;
		ZeroMemory(&m_stHumanBasePoint, sizeof(m_stHumanBasePoint));
		ZeroMemory(&m_stOrcBasePoint, sizeof(m_stOrcBasePoint));
		ZeroMemory(&m_stMoonElfBasePoint, sizeof(m_stMoonElfBasePoint));
		ZeroMemory(&m_stScionBasePoint, sizeof(m_stScionBasePoint));
		for(int i = 0; i < MAX_SHRINE_HAS_REGEION_NUM; ++i) m_arrayRegionIndex[i] = 0;
		m_nHumanPlayerCount = 0;
		m_nOrcPlayerCount = 0;
		m_nMoonElfPlayerCount = 0;
		m_nScionPlayerCount = 0;
		m_ulLastSendTimeRemainTime = 0;

	}


	void InitRacePvPScore()
	{
		for(int i = 0; i < AURACE_TYPE_MAX; ++i)
		{
			m_arrayRaceScore[i].m_lWinScore = 0;
			m_arrayRaceScore[i].m_lLoseScore = 0;
		}
	}
};

typedef ApVector<AgpdShrineRegion, AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER> VectorShrineRegion;
typedef VectorShrineRegion::iterator IterShrineRegion;


class AgpdShrinePoint
{
public:
	INT32	m_nShrinePointID;
	char	m_szPointName[AGPDSHRINEBATTLE_POINT_MAX_NAME_LENGTH];
	char	m_szOccupyCharacterName[AGPDCHARACTER_MAX_ID_LENGTH + 1];

	UINT32	m_nStartHour;					// Shrine이 열려야하는 시작 시각 (시간단위 : ShrineTable에 Setting된 숫자)
	UINT32	m_nIntervalHour;				// Shrine이 열려야하는 시간 간격
	UINT32	m_nTimeLimitTime;				// Shrine Battle이 활성화 되는 시간 
	UINT32	m_nImprintTime;					// 각인을 해야하는 시간
	UINT32	m_nOccupyTime;					// 점령을 해야하는 시간
	
	INT32	m_nCurrentStatus;
	INT32	m_nOccupationRace;
	INT32	m_nTryOccupyRace;
	UINT32	m_nBattleOpenTime;				// 실제적으로 ShrineBattle이 시작된 시각
	UINT32	m_nImprintStartTime;			// 각인을 시작한 시각
	UINT32	m_nOccupyStartTime;				// 점령을 시작한 시각
	UINT32	m_nElapsedTime;					// 각 State 경과 시간
	UINT32	m_nRemainTime;					// 최종 마무리까지 남은 시간
	INT32	m_nOccupyCharacterCid;
	INT32	m_nLevelMax;					// 쉬라인 참가 레벨제한 max//JK_쉬라인 추가
	INT32	m_nLevelMin;					// 쉬라인 참가 레벨제한 min

	INT32	m_nShrineObjectCID;
	BOOL	m_bSpawnObject;
	BOOL	m_bTeleportToTown;				// 전투 끝나고 마을로 보냈는가?

public:
	AgpdShrinePoint()
	{
		m_nShrinePointID = 0;
		memset(m_szPointName, 0, AGPDSHRINEBATTLE_POINT_MAX_NAME_LENGTH);
		ZeroMemory(m_szOccupyCharacterName, sizeof(m_szOccupyCharacterName));

		m_nStartHour = 0;
		m_nIntervalHour = 0;
		m_nTimeLimitTime = 0;
		m_nBattleOpenTime = 0;
		m_nImprintTime = 0;
		m_nOccupyTime = 0;

		m_nCurrentStatus = AGPDSHRINEBATTLE_STATUS_NORMAL;
		m_nOccupationRace = AURACE_TYPE_NONE;
		m_nTryOccupyRace = AURACE_TYPE_NONE;
		m_nImprintStartTime = 0;
		m_nOccupyStartTime = 0;
		m_nElapsedTime = 0;

		m_nOccupyCharacterCid = AP_INVALID_CID;
		m_nShrineObjectCID	  = AP_INVALID_CID;

		m_nLevelMax = 0;
		m_nLevelMin = 0;

		m_bSpawnObject = FALSE;
		m_bTeleportToTown = FALSE;
	}
};

typedef ApVector<AgpdShrinePoint, AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER> VectorShrinePoint;
typedef VectorShrinePoint::iterator IterShrinePoint;

class AgpdShrineBattle : public ApBase
{
public:
	VectorShrineRegion	m_vtShrineRegion;
	VectorShrinePoint	m_vtShrinePoint;
	char				m_szRaceName[AURACE_TYPE_MAX][AGPDSHRINEBATTLE_RACE_NAME_MAX_LENGTH];
	char				m_szMessage[AGPMSHRINEBATTLE_MESSAGE_CODE_MAX][AGPDSHRINEBATTLE_MESSAGE_MAX_LENGTH];

	BOOL				m_bLoad;

	INT32				m_lMaxRegionIndex;
	INT32				m_lMinRegionIndex;
	
public:
	AgpdShrineBattle();

	IterShrineRegion FindRegionByShrinePointID(INT32 nShrinePointID);
	IterShrineRegion FindRegionByRegionIndex(INT32 nRegionIndex);
	IterShrinePoint FindByShrinePointID(INT32 nShrinePointID);
};

#endif