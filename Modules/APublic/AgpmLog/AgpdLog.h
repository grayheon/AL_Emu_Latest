/*==============================================================================

	AgpdLog.h
		(C) NHN - ArchLord Development Team
		steeple, 2004. 04. 26.

==============================================================================*/

#ifndef _AGPDLOG_H_
	#define _AGPDLOG_H_

/****************************************/
/*		The Definition of Constants		*/
/****************************************/
//
#define AGPDLOG_MAX_ACTCODE					10
#define AGPDLOG_MAX_IPADDR_STRING			15
#define AGPDLOG_MAX_ACCOUNTID				32
#define AGPDLOG_MAX_ITEM_CONVERT_STRING		128
#define AGPDLOG_MAX_ITEM_FULL_OPTION		128
#define AGPDLOG_MAX_ITEM_OPTION				32
#define AGPDLOG_MAX_STR_ID					16
#define AGPDLOG_MAX_DESCRIPTION				64
#define AGPDLOG_MAX_WORLD_NAME				20

#define AGPDLOG_MAX_GAMEID					32
#define AGPDLOG_MAX_PCROOM_CRM_CODE			32
#define AGPDLOG_MAX_GRADE					1


//
//	==========	Category	==========
//
const enum eAGPDLOG_CATEGORY
	{
	AGPDLOG_CATEGORY_PLAY = 0,
	AGPDLOG_CATEGORY_ITEM,
	AGPDLOG_CATEGORY_ETC,
	AGPDLOG_CATEGORY_GHELD,
	AGPDLOG_CATEGORY_CASHITEM,
	AGPDLOG_CATEGORY_PCROOM,
	AGPDLOG_CATEGORY_WEBZENAUTH,
	AGPDLOG_CATEGORY_MAX,
	};


//
//	==========	Play	==========
//
const enum eAGPDLOGTYPE_PLAY
	{
	AGPDLOGTYPE_PLAY_GAMEIN = 0,
	AGPDLOGTYPE_PLAY_GAMEOUT,
	AGPDLOGTYPE_PLAY_TICK,
	AGPDLOGTYPE_PLAY_LEVELUP,
	AGPDLOGTYPE_PLAY_GMEDITLEVEL,
	AGPDLOGTYPE_PLAY_GMEDITEXP,
	AGPDLOGTYPE_PLAY_GMEDITDB,
	AGPDLOGTYPE_PLAY_GETEXP,
	AGPDLOGTYPE_PLAY_MAX,
	};


//
//	==========	Item	==========
//
const enum eAGPDLOGTYPE_ITEM
	{
	AGPDLOGTYPE_ITEM_NPC_BUY = 0,
	AGPDLOGTYPE_ITEM_NPC_SELL,
	AGPDLOGTYPE_ITEM_TRADE_GIVE,		// 개인 거래 준거
	AGPDLOGTYPE_ITEM_TRADE_TAKE,		// 개인 거래 받은거
	AGPDLOGTYPE_ITEM_BOARD_SELL,		// 거래 게시판 등록
	AGPDLOGTYPE_ITEM_BOARD_BUY,			// 거래 게시판 구매
	AGPDLOGTYPE_ITEM_BOARD_SOLD,		// 거래 게시판 구매 더미
	AGPDLOGTYPE_ITEM_BOARD_CANCEL,		// 거래 게시판 취소
	AGPDLOGTYPE_ITEM_BOARD_CONFIRM,		// 거래 게시판 확인
	AGPDLOGTYPE_ITEM_PICK,
	AGPDLOGTYPE_ITEM_DEST,
	AGPDLOGTYPE_ITEM_USE,
	AGPDLOGTYPE_ITEM_REPAIR,			// 수리
	AGPDLOGTYPE_ITEM_SM1_SC,			// 강화 성공
	AGPDLOGTYPE_ITEM_SM1_NC,			// 강화 실패(변화 없음)
	AGPDLOGTYPE_ITEM_SM1_IN,			// 강화 초기화
	AGPDLOGTYPE_ITEM_SM1_DT,			// 강화 아이템 파괴
	AGPDLOGTYPE_ITEM_SM2_SC,			// 정령석 성공
	AGPDLOGTYPE_ITEM_SM2_FL,			// 정령석 실패
	AGPDLOGTYPE_ITEM_SM3_SC,			// 기원석 성공
	AGPDLOGTYPE_ITEM_SM3_NC,			// 기원석 실패(변화 없음)
	AGPDLOGTYPE_ITEM_SM3_IN,			// 기원석 소켓 초기화
	AGPDLOGTYPE_ITEM_SM3_II,			// 기원석 아이템 초기화
	AGPDLOGTYPE_ITEM_SM3_DT,			// 기원석 아이템 파괴
	AGPDLOGTYPE_ITEM_QUEST_REWARD,		// 퀘스트 보상(받았다)
	AGPDLOGTYPE_ITEM_QUEST_PAY,			// 퀘스트 거시기(줬다)
	AGPDLOGTYPE_ITEM_PDSKILL_REWARD,	// 생산스킬 보상
	AGPDLOGTYPE_ITEM_PDSKILL_PAY,		// 생산 재료
	AGPDLOGTYPE_ITEM_REFINE_REWARD,		// 정제 결과
	AGPDLOGTYPE_ITEM_REFINE_PAY,		// 정제 재료
	AGPDLOGTYPE_ITEM_BANK_IN,			// 뱅크에 넣었다.
	AGPDLOGTYPE_ITEM_BANK_OUT,			// 뱅크에서 뺐다.
	AGPDLOGTYPE_ITEM_BANK_LOGOUT,		// 로그 아웃시 뱅크 내역
	AGPDLOGTYPE_ITEM_PARTY_DISTRIBUTE,	// 파티 아이템 분배
	AGPDLOGTYPE_ITEM_EVENT_GIVE,		// 이벤트 아이템 증정
	AGPDLOGTYPE_ITEM_GM_MAKE,			// GM이 생성.
	AGPDLOGTYPE_ITEM_GM_DESTROY,		// GM이 삭제
	AGPDLOGTYPE_ITEM_GM_ITSM1,			// GM 축성제강화
	AGPDLOGTYPE_ITEM_GM_ITSM2,			// GM 정령석강화
	AGPDLOGTYPE_ITEM_GM_ITSM3,			// GM 기원석강화
	AGPDLOGTYPE_ITEM_GM_ITSM4,			// GM 고리추가
	AGPDLOGTYPE_ITEM_GM_ITSM5,			// GM 옵션추가
	AGPDLOGTYPE_ITEM_MAIL_ATTACH,		// 메일에 첨부
	AGPDLOGTYPE_ITEM_MAIL_READ,			// 메일에 첨부된거 확인
	AGPDLOGTYPE_ITEM_USE_START,			// 캐쉬아이템 사용 시작.
	AGPDLOGTYPE_ITEM_DROP,
	AGPDLOGTYPE_ITEM_GWARE_IN,			// 길드창고넣기
	AGPDLOGTYPE_ITEM_GWARE_OUT,			// 길드창고빼기
	AGPDLOGTYPE_ITEM_SOCKET_INIT,		// 소켓 초기화
	AGPDLOGTYPE_ITEM_NPC_GIVE,		// NPC Manager에게 받았다
	AGPDLOGTYPE_ITEM_NPC_ROB,			// NPC Manager에게 주었다
	AGPDLOGTYPE_ITEM_USE_STOP,			//캐쉬아이템 사용 중지.
	AGPDLOGTYPE_ITEM_USE_END,			//캐쉬아이템 사용 끝.
	AGPDLOGTYPE_ITEM_COUPON_GIVE,		// Coupon에 의한 아이템 지급
	AGPDLOGTYPE_ITEM_SHRINE_GIVE,		//쉬라인 Give Item //JK_쉬라인머지
	AGPDLOGTYPE_ITEM_MAX,
	};


//
//	==========	ETC	==========
//
const enum eAGPDLOGTYPE_ETC
	{
	AGPDLOGTYPE_ETC_SKILL_BUY = 0,
	AGPDLOGTYPE_ETC_PDSKILL_BUY,
	AGPDLOGTYPE_ETC_SKILL_UPGRADE,
	AGPDLOGTYPE_ETC_QUEST_ACCEPT,
	AGPDLOGTYPE_ETC_QUEST_CANCEL,
	AGPDLOGTYPE_ETC_QUEST_COMPLETE,
	AGPDLOGTYPE_ETC_GUILD_CREATE,
	AGPDLOGTYPE_ETC_GUILD_REMOVE,
	AGPDLOGTYPE_ETC_GUILD_IN,
	AGPDLOGTYPE_ETC_GUILD_OUT,
	AGPDLOGTYPE_ETC_GUILD_BATTLE_REQ,
	AGPDLOGTYPE_ETC_GUILD_BATTLE_ACP,
	AGPDLOGTYPE_ETC_SKILL_INIT,
	AGPDLOGTYPE_ETC_CHAR_CUSTOMIZE,
	AGPDLOGTYPE_ETC_BANK_EXPANSION,
	AGPDLOGTYPE_ETC_DEATH,
	AGPDLOGTYPE_ETC_GUILD_EXPANSION,
	AGPDLOGTYPE_ETC_GUILD_RENAME,
	AGPDLOGTYPE_ETC_TELEPORT,
	AGPDLOGTYPE_ETC_GUILD_BUY_MARK,
	AGPDLOGTYPE_ETC_REMISSION,
	AGPDLOGTYPE_ETC_MURDERER_UP,
	AGPDLOGTYPE_ETC_WC_KILL,
	AGPDLOGTYPE_ETC_WC_DEAD,
	AGPDLOGTYPE_ETC_WC_ARRANGE,
	AGPDLOGTYPE_ETC_KILL,
	AGPDLOGTYPE_ETC_TAX_TRANSFER,
	AGPDLOGTYPE_ETC_TAX_UPDATE,
	AGPDLOGTYPE_ETC_CHARISMA_UP,
	AGPDLOGTYPE_ETC_TAX_BACKUP,
	AGPDLOGTYPE_ETC_RENAME,
	AGPDLOGTYPE_ETC_SEAL,
	AGPDLOGTYPE_ETC_RELEASESEAL,
	AGPDLOGTYPE_ETC_NOCHAT,
	AGPDLOGTYPE_ETC_TITLEQUEST_ADD,
	AGPDLOGTYPE_ETC_TITLEQUEST_COMPLETE,
	AGPDLOGTYPE_ETC_TITLE_ADD,
	AGPDLOGTYPE_ETC_TITLE_USE,
	AGPDLOGTYPE_ETC_TITLE_DELETE,
	AGPDLOGTYPE_ETC_TITLE_ADD_BY_GM,
	AGPDLOGTYPE_ETC_TITLE_DELETE_BY_GM,
	AGPDLOGTYPE_ETC_TITLE_EDIT_BY_GM,
	AGPDLOGTYPE_ETC_TITLEQUEST_ADD_BY_GM,
	AGPDLOGTYPE_ETC_TITLEQUEST_EDIT_BY_GM,
	AGPDLOGTYPE_ETC_TITLEQUEST_DELETE_BY_GM,
	AGPDLOGTYPE_ETC_TITLEQUEST_COMPLETE_BY_GM,
	AGPDLOGTYPE_ETC_CHARISMA_UP_BY_GM,
	AGPDLOGTYPE_ETC_MAIL_WRITE,
	AGPDLOGTYPE_ETC_MAIL_DELETE,
	AGPDLOGTYPE_ETC_BATTLEGROUND_TELEPORT,
	AGPDLOGTYPE_ETC_MAX,
	};


//
//	==========	Gheld	==========
//
const enum eAGPDLOGTYPE_GHELD
	{
	AGPDLOGTYPE_GHELD_PICK = 0,
	AGPDLOGTYPE_GHELD_BANK_IN,
	AGPDLOGTYPE_GHELD_BANK_OUT,
	AGPDLOGTYPE_GHELD_TRADE_GIVE,
	AGPDLOGTYPE_GHELD_TRADE_TAKE,
	AGPDLOGTYPE_GHELD_REFINE_PAY,
	AGPDLOGTYPE_GHELD_QUEST_REWARD,
	AGPDLOGTYPE_GHELD_EVENT_GIVE,
	AGPDLOGTYPE_GHELD_GMEDIT,
	AGPDLOGTYPE_GHELD_GWARE_IN,
	AGPDLOGTYPE_GHELD_GWARE_OUT,
	AGPDLOGTYPE_GHELD_MAX,
	};




/********************************************/
/*		The Definition of AgpdLog class		*/
/********************************************/
//
//	==========	Base	==========
//
class AgpdLog
	{
	public:
		INT8	m_cLogType;

		// Base Data
		INT32	m_lTimeStamp;
		INT32	m_lServerID;
		CHAR	m_szIP[AGPDLOG_MAX_IPADDR_STRING+1];
		CHAR	m_szAccountID[AGPDLOG_MAX_ACCOUNTID+1];
		CHAR	m_szWorld[AGPDLOG_MAX_WORLD_NAME+1];
		CHAR	m_szCharID[AGPDLOG_MAX_STR_ID+1];
		INT32	m_lCharTID;
		INT32	m_lCharLevel;
		INT64	m_llCharExp;
		INT64	m_llGheldInven;
		INT64	m_llGheldBank;

	public:
		AgpdLog();
		virtual ~AgpdLog();

		inline void		Init();
		inline void		SetType(INT8 cType);
		inline INT32	SetTimeStamp(INT32 lTimeStamp = 0);

		inline void		SetBasicData(AgpdLog* pcsAgpdLog);
		inline void		SetBasicData(INT32 lTimeStamp, CHAR *pszIP, CHAR *pszAccountID, CHAR *pszWorld,
									CHAR *pszCharID, INT32 lCharTID, INT32 lCharLevel,
									INT64 llCharExp, INT64 llGheldInven, INT64 llGheldBank);
	};


//
//	==========	Play	==========
//
class AgpdLog_Play : public AgpdLog
	{
	public:
		static CHAR m_szAct[AGPDLOGTYPE_PLAY_MAX][AGPDLOG_MAX_ACTCODE+1];
		
	public:
		INT8	m_cFlag;		// eAGPDLOGTYPE_PLAY

		INT32	m_lCharLevelFrom;
		INT32	m_lPartyTime;
		INT32	m_lSoloTime;
		INT32	m_lKillPC;
		INT32	m_lKillMonster;
		INT32	m_lDeadByPC;
		INT32	m_lDeadByMonster;

	public:
		AgpdLog_Play();
		~AgpdLog_Play();

		inline void	Init();
	};


//
//	==========	Item	==========
//
class AgpdLog_Item : public AgpdLog
	{
	public:
		static CHAR m_szAct[AGPDLOGTYPE_ITEM_MAX][AGPDLOG_MAX_ACTCODE+1];
		
	public:
		INT8	m_cFlag;			// eAGPDLOGTYPE_ITEM
		UINT64	m_ullItemDBID;
		INT32	m_lItemTID;
		INT32	m_lItemQty;
		CHAR	m_szConvert[AGPDLOG_MAX_ITEM_CONVERT_STRING+1];
		CHAR	m_szOption[AGPDLOG_MAX_ITEM_FULL_OPTION+1];
		INT32	m_lGheld;
		CHAR	m_szCharID2[AGPDLOG_MAX_STR_ID+1];
		INT32	m_lInUse;
		INT32	m_lUseCount;
		INT64	m_lRemainTime;
		INT32	m_lExpireDate;
		INT64	m_llStaminaRemainTime;
		CHAR	m_szDescription[AGPDLOG_MAX_DESCRIPTION+1];
		
	public:
		AgpdLog_Item();
		~AgpdLog_Item();
		
		inline void	Init();
	};


//
//	==========	ETC	==========
//
class AgpdLog_ETC : public AgpdLog
	{
	public:
		static CHAR m_szAct[AGPDLOGTYPE_ETC_MAX][AGPDLOG_MAX_ACTCODE+1];
		
	public:
		INT8	m_cFlag;	// eAGPDLOGTYPE_ETC
		INT32	m_lNumID;
		CHAR	m_szStrID[AGPDLOG_MAX_STR_ID+1];
		CHAR	m_szDescription[AGPDLOG_MAX_DESCRIPTION+1];
		INT32	m_lGheld;
		CHAR	m_szCharID2[AGPDLOG_MAX_STR_ID+1];

	public:
		AgpdLog_ETC();
		~AgpdLog_ETC();
		
		inline void	Init();
	};


//
//	==========	Gheld	==========
//
class AgpdLog_Gheld : public AgpdLog
	{
	public:
		static CHAR m_szAct[AGPDLOGTYPE_GHELD_MAX][AGPDLOG_MAX_ACTCODE+1];
		
	public:
		INT8	m_cFlag;	// eAGPDLOGTYPE_GHELD
		INT32	m_lGheldChange;
		CHAR	m_szCharID2[AGPDLOG_MAX_STR_ID+1];
		
	public:
		AgpdLog_Gheld();
		~AgpdLog_Gheld();
		
		inline void	Init();
	};


//
//	==========	PC Room	==========
//
// 2007.11.06. steeple
class AgpdLog_PCRoom : public AgpdLog
	{
	public:
		CHAR		m_szGameID[AGPDLOG_MAX_GAMEID+1];
		UINT32		m_ulLoginTimeStamp;
		UINT32		m_ulLogoutTimeStamp;
		UINT32		m_ulPlayTime;
		INT32		m_lLoginLevel;
		CHAR		m_szCRMCode[AGPDLOG_MAX_PCROOM_CRM_CODE+1];
		CHAR		m_szGrade[AGPDLOG_MAX_GRADE+1];

	public:
		AgpdLog_PCRoom();
		~AgpdLog_PCRoom();
		
		inline void	Init();
	};

// Global service 용 로긴 로그아웃 로그데이터 090804 by supertj
class AgpdLog_Global: public AgpdLog
{
public:
	CHAR		m_szGameID[AGPDLOG_MAX_GAMEID+1];
	UINT32		m_uiPlayTime;
	UINT32		m_uiLoginTime;
	UINT32		m_uiLogoutTime;
public:
	AgpdLog_Global()
	{
		m_cLogType = AGPDLOG_CATEGORY_WEBZENAUTH;
		ZeroMemory(m_szGameID, sizeof(m_szGameID));
		m_uiPlayTime = 0;
		m_uiLoginTime = 0;
		m_uiLogoutTime = 0;
	};
	~AgpdLog_Global(){};
};

#endif // _AGPDLOG_H_