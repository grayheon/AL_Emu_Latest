#pragma once
#include "agcmodule.h"
#include "AgcdUIManager2.h"
#include "AgpdShrineBattle.h"
#include "AGUIDrawMark.h"

class AgpmShrineBattle;
class AgpmCharacter;
class AgcmCharacter;
class AgcmEventEffect;
class TiXmlNode;

#define SHRINE_RACENAME_LENGTH 64
#define SHRINE_MESSAGE_LENGTH 512

#define SHRINEBATTLEWINDOWNAME_MAX "UI_ShrineBattle_Info_Maximum"
#define SHRINEBATTLEWINDOWNAME_MINI "UI_ShrineBattle_Info_Minimum"

class AgcmShrine;
class AcUIShrineRender : public AcUIBase  
{
public:
	AcUIShrineRender():m_AgcmShrine(NULL){}
	virtual ~AcUIShrineRender(){}

	virtual	void 			OnWindowRender		( VOID );
	void						Set(AgcmShrine* pUI ){ m_AgcmShrine = pUI; }
private:
	AgcmShrine* m_AgcmShrine;
};

class AgcmShrine :public AgcModule
{
public:
	struct S_SHRINE_MSG_RACE_INFO
	{
		INT32 iRaceTID;
		char szRaceName[SHRINE_RACENAME_LENGTH];
		DWORD dwRaceColor;

		S_SHRINE_MSG_RACE_INFO() : iRaceTID(0), dwRaceColor(0)
		{ memset( szRaceName, 0, SHRINE_RACENAME_LENGTH * sizeof(char) ); }
	};

	struct S_SHRINE_MSG_MESSAGE
	{
		enum E_MSG_SHOW_TYPE
		{
			E_MSG_SHOW_ALL = 0,			//메인 메세지와 채팅창에 모두
			E_MSG_SHOW_MAIN,			//메인 메세지만
			E_MSG_SHOW_CHAT,			//채팅창에만
		};

		// 쉬라인메시지 표시되는 대상
		enum E_MSG_SHOW_TARGET
		{
			E_SHRINE_MSG_TARGET_UNKNOWN			= 0,
			E_SHRINE_MSG_TARGET_IN_SHRINE		= 1 << 0,	//쉬라인 참가자
			E_SHRINE_MSG_TARGET_NOT_IN_SHRINE	= 1 << 1,	//쉬라인 미참가자
			E_SHRINE_MSG_TARGET_SAME_RACE		= 1 << 2,	//같은 종족
			E_SHRINE_MSG_TARGET_OTHER_RACE		= 1 << 3,	//다른 종족
		};

		INT32				iMsgTID;
		E_MSG_SHOW_TYPE		eShowType;
		E_MSG_SHOW_TARGET	eShowTarget;
		DWORD				dwColor;
		char				szMessage[SHRINE_MESSAGE_LENGTH];
		
		S_SHRINE_MSG_MESSAGE() : iMsgTID(0), eShowType(E_MSG_SHOW_ALL), eShowTarget(E_SHRINE_MSG_TARGET_UNKNOWN), dwColor(0xffffff00)
		{memset( szMessage, 0, SHRINE_MESSAGE_LENGTH * sizeof(char) );  }
	};

	struct S_SHRINE_OBJECT_INFO
	{
		INT32 iTID;
		INT32 iAccTime;
		INT32 iPrevStatus;
		string strOccupyPCName;
		BOOL  bReadyToStart;
		BOOL bFirstShow;

		S_SHRINE_OBJECT_INFO():iTID(0),iAccTime(-1),iPrevStatus(-1),bReadyToStart(FALSE),bFirstShow(FALSE) { }
		void reset(){ iTID=0;iAccTime=-1;iPrevStatus=-1;bReadyToStart = FALSE; bFirstShow = FALSE; }
	};

	struct S_SHRINE_SORT_INFO
	{
		INT32 iShrinePointID;
		DWORD dwOpenTickTime;
		S_SHRINE_SORT_INFO():iShrinePointID(0), dwOpenTickTime(0){}
	};

	struct S_SHRINE_MSG_NODE
	{
		std::string strMessage;
		DWORD		strColor;
		S_SHRINE_MSG_NODE():strColor(0xffffff00){}
	};
public:
	friend class AcUIShrineRender;

	AgcmShrine();
	virtual ~AgcmShrine();

	virtual BOOL								OnAddModule();
	virtual BOOL								AddFunction();
	virtual BOOL								AddDisplay();
	virtual BOOL								AddUserData();
	virtual BOOL								AddEvent();
	virtual	BOOL								OnDestroy();

	virtual BOOL								OnInit();
	BOOL			OnIdle				( UINT32 ulClockCount	);

	//click shrine object
	BOOL ClickShrine( AgpdCharacter *pcsSelfChar, AgpdCharacter* pObject );

	//Set TapControl
	BOOL SetTapControl();

	//Enter Game mode 
	void EnterGameMode();

	inline	BOOL	IsOpenedAnyBattleWindow()	{return m_bIsOpenMaximizedWindow || m_bIsOpenMinimizedWindow;}
	inline	BOOL	IsOpenedMaxWindow()			{return m_bIsOpenMaximizedWindow;}
	inline	BOOL	IsOpenedMiniWindow()		{return m_bIsOpenMinimizedWindow;}

	BOOL	CloseAllBattleWindow();
	BOOL	CheckBattleWindow();
	BOOL	MakeShrineMessageByMessageCode(eAgpmShrineBattleMessageCode eCode, PVOID pData = NULL);
 	BOOL	SelfPrintMessageChat(std::string strMessage);
//	BOOL	SelfPrintMessageChat(S_SHRINE_MSG_NODE* nodeShrineMsg);
// 	BOOL	SelfPrintMessageCenter(std::string strMessage);
	BOOL	SelfPrintMessageCenter(S_SHRINE_MSG_NODE nodeShrineMsg);
	//////////////////////////////////////////////////////////////////////////
	//AgpmShrine call back function
	//////////////////////////////////////////////////////////////////////////
	//time bar ON/OFF
	static BOOL CBShrineTimeToggleShow(PVOID pData, PVOID pClass, PVOID pCustData); 
	//update shrine status 
	static BOOL CBShrinePointStatusUpdate(PVOID pData, PVOID pClass, PVOID pCustData);
	//update shrine info
	static BOOL CBShrinePointUpdateInfo(PVOID pData, PVOID pClass, PVOID pCustData);
	//all update shrine info
	static BOOL CBAllShrinePointUpdateInfo(PVOID pData, PVOID pClass, PVOID pCustData);
		
	//imprint result
	static BOOL CBResultImprint(PVOID pData, PVOID pClass, PVOID pCustData);

	//notify Shrine Open
	static BOOL CBReadyToStart(PVOID pData, PVOID pClass, PVOID pCustData);

	//shrine message
	static BOOL CBShrineMessage(PVOID pData, PVOID pClass, PVOID pCustData);

	//shrine PvP Record
	static BOOL CBShrinePvPRecord(PVOID pData, PVOID pClass, PVOID pCustData);

	//shrine Race Record
	static BOOL CBShrineRaceRecord(PVOID pData, PVOID pClass, PVOID pCustData);

	//////////////////////////////////////////////////////////////////////////
	// ui call back function
	//////////////////////////////////////////////////////////////////////////
	/*---------
	function
	---------*/
	static BOOL CBOpenShrineInfo(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL CBShrineIconClick(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	
	//Shrine Battle Info
	static BOOL	CBShrineBattleInfoOpenUI(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL CBMaximizeShrineBattleInfo(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL CBMinimizeShrineBattleInfo(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );

	//tap Button 
	static BOOL CBShrineTapStatus(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL CBShrineTapScroe(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );

	//Info List Page Button
	static BOOL CBInfoListPrev(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL CBInfoListNext(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );

	/*---------
	display
	---------*/
	static BOOL CBShrineHPMAX( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBShrineHPCUR( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );

	static BOOL CBShrineHPDP( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );

	
	//new 
	static BOOL CBShrineUpdate( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBShrineOccHCount( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBShrineOccOCount( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );

	//Shrine Infomation Status
	static BOOL CBDPListStatus( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPListName( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPListShrineStatusBarCur( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPListShrineStatusBarMax( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPListShrineStatusText( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPListOccupyRace( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPListOccupyPCName( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPListRemainTime( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );

	//Shrine Info List Page
	static BOOL CBDPListPage( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );

	//Shrine Battle Info
	static BOOL CBDPMaxLefttime( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPMiniLefttime( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPMaxMyWin( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPMaxMyLose( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPMaxRaceWin( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL CBDPMaxRaceLose( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );

private:
	BOOL _LoadShrineMessage();
	DWORD _GetColorFromXml( TiXmlNode* pNode, DWORD dwDefaultColor = 0 );
	S_SHRINE_MSG_RACE_INFO* _GetMsgRaceInfo( INT32 iTID );
	S_SHRINE_MSG_MESSAGE* _GetMsgMessge( INT32 iTID );
	BOOL _CopyShrinInfo(  AgpdShrinePoint* pDestShrineInfo, AgpdShrinePoint* pSrcShrineInfo  );
	AgpdShrinePoint*	_GetShrineInfo( INT32 iShrineID );
	INT32 _GetShrineInfoIndex( INT32 iShrineID );
	INT32 _GetShrineInfoIndex( char* pszTmp, char* pszIconName );
	BOOL _ChangeShrineIcon(  INT32 iIndex );
	AGUIDrawMark::E_MARKINFO_TYPE _GetShrineStatus( AgpdShrinePoint* pShrineInfo );
	INT32 _GetShrineOccupateCount( INT32 iRace );
	void _ShowTimeBar( INT32 iShrineID, BOOL bShow, BOOL bImprint );
	BOOL _ChangeShrineObject( INT32 iShrineObjectTID , INT32 iCID, char* szObjectName );
	BOOL _ChangeTapUI( INT32 iTapIndex );
	void _SortShrineInfo();
	BOOL _IsJoinShrine( INT32 iIndex );
	BOOL _MessagePrintCheckerType( S_SHRINE_MSG_MESSAGE* pMessage, INT32 nTarget );
	BOOL _MessagePrintCheckerTargetPlace( S_SHRINE_MSG_MESSAGE* _pMessage, INT32 _nTarget, BOOL _IsInShrine );
	BOOL _MessagePrintCheckerTargetRace( S_SHRINE_MSG_MESSAGE* _pMessage, INT32 _nTarget, BOOL _IsSameRace );
	BOOL _SelfPrintShrineMessageChat(std::string strMessage);
	BOOL _SelfPrintShrineMessageCenter(S_SHRINE_MSG_NODE strMessage);
	BOOL _ReservePrintMessage(std::string, PVOID pData1 = NULL);

	AgcmUIManager2* m_pUIMgr;
	AgpmShrineBattle* m_pAgpmShrine;
	AgpmCharacter* m_pAgpmCharacter;
	AgcmCharacter* m_pAgcmCharacter;
	AgcmEventEffect* m_pAgcmEventEffect;

	//new shrine
	AgcdUIUserData* m_pUDShrineUpdate;
	INT32 m_iEventShrineUpdate;

	//Shrine Info List
	AgcdUIUserData* m_pUDShrineInfoList;

	//Shrine Info Page
	AgcdUIUserData* m_pUDShrineListPage;

	//shrine info
	AgpdShrinePoint	m_arrShrinPointInfo[AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER];
	//shrine sort info
	S_SHRINE_SORT_INFO m_arrShrineSortInfo[ AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER ];

	//Shrine icon Render
	AcUIShrineRender		m_ShrineRender;

	//shrine icon UI control
	vector<AgcdUIControl*>	m_vecShrineIconUI;
	AGUIDrawMark m_ShrineIcon;

	//shrine tap button control
	vector<AgcdUIControl*>	m_vecTapbutton;

	//shrine info UI
	AgcdUI* m_pUI;

	AgcdUIUserData* m_pUDShrineHP;
	AgcdUIUserData* m_pUDOccupationCount[AURACE_TYPE_MAX];

	AgcdUIUserData* m_pUDMaxTime;
	AgcdUIUserData* m_pUDMyWin;
	AgcdUIUserData* m_pUDMyLose;
	AgcdUIUserData* m_pUDRaceWin;
	AgcdUIUserData* m_pUDRaceLose;

	AgcdUIUserData* m_pUDMiniTime;

	INT32 m_iEventShrineHPShow;
	INT32 m_iEventShrineHPHide;
	INT32 m_iEventShrineBattleInfoOpen;
	INT32 m_iCurProcessShrine;
	INT32 m_iEventShrineOpenSound;
	INT32 m_iEventShrineCloseSound;

	typedef map<INT32, S_SHRINE_MSG_RACE_INFO*> MAP_MSG_RACEINFO;
	typedef MAP_MSG_RACEINFO::iterator MAP_MSG_RACEINFO_ITR;
	MAP_MSG_RACEINFO m_mapMsgRaceInfo;

	typedef map<INT32, S_SHRINE_MSG_MESSAGE*> MAP_MSG_MESSAGE;
	typedef MAP_MSG_MESSAGE::iterator MAP_MSG_MESSAGE_ITR;
	MAP_MSG_MESSAGE m_mapMsgMessage;

	INT32 m_iShrineHPPos;
	INT32 m_iShrineMsgPos;
	DWORD m_dwNormalMessageColor;
	DWORD m_dwStartMessageColor;
	DWORD m_dwCloseMessageColor;

	BOOL m_bStartImprint;
	INT32 m_iImprintProcessTime;
	INT32 m_iImprintAccTime;
	INT32 m_iImprintMaxTime;

	S_SHRINE_OBJECT_INFO	 m_arShrineObject[AGPDSHRINEBATTLE_MAX_SHRINE_POINT_NUMBER];

	CString m_strCurRace;
	CString m_strCurName;
	CString m_strCurAction;

	//Shrine Status Icon Offset
	RwV2d m_vIconOffset;

	DWORD m_dwRequestTime;

	INT32 m_iCurPage;
	INT32 m_iShrineEnableCount;

	// 쉬라인 전투 창 UI관련 내부변수
	BOOL	m_bIsOpenMaximizedWindow;	// 최대화 된 전투창
	BOOL	m_bIsOpenMinimizedWindow;	// 최소화 된 전투창

	INT32	m_nBattleInfoPosX;	// 창의 위치를 기억
	INT32	m_nBattleInfoPosY;

	INT32	m_nMyWin;
	INT32	m_nMyLose;
	INT32	m_nRaceWin;
	INT32	m_nRaceLose;

	INT32	m_nShrineBattleInfoTime;
	UINT32	m_nOpenedTime;
	DWORD	m_dwCheckFiveMin;

	DWORD	m_dwRefreshTime;
	DWORD	m_dwPrintNextTimeChat;
	DWORD	m_dwPrintNextTimeCenter;

	AgpdCharacter* m_pShrineCharacter;
	INT32	m_nOpenedShrinePointID;

	std::list<std::string> m_listShrineMessageChat;
	std::list<S_SHRINE_MSG_NODE> m_listShrineMessageCenter;

	AgcdUIControl *m_pUICtrlShrineButton;

public:
	BOOL	IsOtherRace(AgpdCharacter* pcdCharacter);
};
