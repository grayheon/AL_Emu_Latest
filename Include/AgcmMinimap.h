// AgcmMinimap.h: interface for the AgcmMinimap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AGCMMINIMAP_H__6CE9B6F1_B8DA_42EA_B617_75088D546595__INCLUDED_)
#define AFX_AGCMMINIMAP_H__6CE9B6F1_B8DA_42EA_B617_75088D546595__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AgcModule.h"
#include "AgcmCharacter.h"
#include "AgcmUIControl.h"
#include "AgcmUIManager2.h"
#include "AgpmParty.h"

#include "AcUIMinimap.h"
//#include "AcUIWorldmap.h"
#include "AcUIRaider.h"
#include "AgcmFont.h"
#include "ApmMap.h"
#include "AgpmBattleGround.h"

#include "AgcmUIChatting2.h"

#include <vector>
#include <list>

#if _MSC_VER < 1300
#ifdef	_DEBUG
#pragma comment ( lib , "AgcmMinimapD" )
#else
#pragma comment ( lib , "AgcmMinimap" )
#endif
#endif

#define	MAX_MINIMAP_TEXTURE_BUFFER	30
#define	AGCMMAP_MINIMAP_FILE_FORMAT		"MAP%04d%c"
// Division Index & Part Index
#define AGCMMAP_MINIMAP_FILE_CURSOR		"Main_Minimap_Cursor"
#define AGCMMAP_WORLDMAP_TEST			"wmap%04d"
#define AGCMMAP_MEMBERPOSITION			"mempos"
#define AGCMMAP_PCPOSITION				"pcpos"

#define AGCMMAP_REGISTER_POSITION		"Main_Minimap_Pos_Siege1"

#define AGCMMAP_NPC_NORMAL				"Main_Minimap_Pos_NPC0"
#define AGCMMAP_NPC_QUESTINCOMPLETE		"Main_Minimap_Pos_NPC1"
#define AGCMMAP_NPC_QUESTNEW			"Main_Minimap_Pos_NPC2"

#define AGCMMAP_SIEGE_NORMAL				"Main_Minimap_Pos_Siege0"
#define AGCMMAP_SIEGE_DESTROIED				"Main_Minimap_Pos_Siege1"


#define	AGCMMAP_MINIMAP_UI_FOLDER		"texture\\UI\\"

#define	AGCMMAP_MINIMAP_FOLDER			"texture\\minimap\\"

#define AGCMMAP_WORLDMAP_FOLDER_DEFAULT	"texture\\worldmap\\"
#define AGCMMAP_WORLDMAP_FOLDER			"texture\\worldmap\\type%02d\\"

#define	AGCMMAP_MINIMAP_POINT_TEXT_LENGTH	64
#define	AGCMMAP_MINIMAP_POINT_MAX_COUNT		256


enum
{
	AGCMMINIMAP_CB_MAP_POS_REGISTER		,
};

struct stMapRegisterInfo 
{
	INT				m_nID;
	string			m_strName;
	RwV2d			m_vPosition;
	DWORD			m_dwInterval;			//	한번 깜빡거리는데 필요한 시간
	DWORD			m_dwDurationTime;		//	몇초간 깜박거릴것인지 
	DWORD			m_dwStartTime;			//	시작 시간
};


extern	FLOAT	__sRate;
extern	FLOAT	__GetDivisionStartX	( INT32 nDivision , INT32 nPartIndex );
extern	FLOAT	__GetDivisionStartZ	( INT32 nDivision , INT32 nPartIndex );
extern  INT32	__GetRaiderPos		( FLOAT fPos );

class AgcmMinimap : public AgcModule  
{
private:
	typedef			list< stMapRegisterInfo >				ListMapRegisterInfo;
	typedef			list< stMapRegisterInfo >::iterator		ListMapRegisterInfoIter;

public:
	AgcmUIManager2	*	m_pcsAgcmUIManager2	;
	AgpmParty		*	m_pcsAgpmParty		;
	AgcmResourceLoader*	m_pcsAgcmResourceLoader;
	AgcmFont		*	m_pcsAgcmFont		;
	ApmMap			*	m_pcsApmMap			;
	AgpmCharacter	*	m_pcsAgpmCharacter	;
	AgpmPvP			*	m_pcsAgpmPVP		;
	AgcmCharacter	*	m_pcsAgcmCharacter	;
	AgpmGuild		*	m_pcsAgpmGuild		;
	AgpmBattleGround*	m_pcsAgpmBattleGround;
	AgcmUIChatting2*	m_pcsAgcmUIChatting2;

	// 마지막 이동한 케릭터 포지션..
	AcUIMinimap		*	m_pclMinimapWindow1	;
	AcUIRaider		*	m_pclRaiderWindow	;

	//AcUIWorldmap	*	m_pclWorldWindow		;
	RwTexture		*	m_pDirectionCursor	;	//방향커서..


	INT32				m_nWorldMap			;
	RwTexture		*	m_pWorldMap			;	//월드맵..
	RwTexture		*	m_pPcPos			;	// 자기위치..
	RwTexture		*	m_pMemberPos		;	//파티맴버 포지션.

	RwTexture		*	m_pNpcNormal		;	//파티맴버 포지션.
	RwTexture		*	m_pNpcQuestNew		;	//파티맴버 포지션.
	RwTexture		*	m_pNpcQuestIncomplete;	//파티맴버 포지션.

	RwTexture		*	m_pSiegeNormal		;
	RwTexture		*	m_pSiegeDestroied	;

	RwTexture		*	m_pRegisterPosTexture;

	AuPOS				m_stSelfCharacterPos	;
	AgcdUIUserData	*	m_pstUDPosition		;	// Self Character Position
	AgpdCharacter	*	m_pcsSelfCharacter	;

	INT32				m_lEventPlusButton	;
	INT32				m_lEventMinusButton	;

	INT32				m_lEventOpenMiniMapUI	;
	INT32				m_lEventCloseMiniMapUI	;

	INT32				m_lEventOpenRaiderUI	;
	INT32				m_lEventCloseRaiderUI	;

	INT32				m_lEventMinimapForceClose	;
	INT32				m_lEventMinimapYouCanOpen	;

	INT32				m_lEventOpenWorldMapUI		;
	INT32				m_lEventWorldMapNoItem		;
	INT32				m_lEventCloseWorldMapUI		;

	INT32				m_lEventMinimapON;
	INT32				m_lEventMinimapOFF;

	BOOL				m_bIsOpenMiniMap	;
	BOOL				m_bIsOpenRaider		;
	bool				m_bShowAllCharacter	;

	BOOL				m_bEnemyGuild;
	BOOL				m_bMurderer;
	BOOL				m_bCriminal;

	BOOL				m_bMinimapON;


	ListMapRegisterInfo	m_listMapRegisterInfo;

	AgcdUIUserData*		m_pstMMRegionNameUD;
	INT32				m_lDummy;

	AgcmMinimap();
	virtual ~AgcmMinimap();

public:
	// Operation
	// 미니맵 텍스쳐 관리.
	struct	MinimapStructure
	{
		INT32			nIndex		;	// DivisionIndex
		INT32			nPart		;	// Part Index
		RwTexture	*	pTexture	;
		UINT32			uLastAccess	;

		//		MinimapStructure():nIndex( -1 ),pTexture( NULL ) {}
		MinimapStructure()
		{
			nIndex		= -1	;
			nPart		= -1	;
			pTexture	= NULL	;
			uLastAccess	= 0		;
		}
	};

	MinimapStructure	m_pMinimapTextureBuffer[ MAX_MINIMAP_TEXTURE_BUFFER ];

	unsigned char m_padding[132];

	void				InitMinimapTextureBuffer	();
	RwTexture	*		GetMinimapTexture			( INT32 nDivisionIndex , INT32 nPartIndex );
	RwTexture	*		GetMinimapDummyTexture		();
	void				FlushMinimapTexture			();

	RwTexture	*		GetCursorTexture			();
	RwTexture	*		GetWorldMapTexture			( int nIndex );
	RwTexture	*		GetMemberPosTexture			();
	RwTexture	*		GetPcPosTexture				();

	RwTexture	*		GetNpcNormalTexture			();
	RwTexture	*		GetNpcQuestNewTexture		();
	RwTexture	*		GetNpcQuestIncompleteTexture();

	RwTexture	*		GetTextureSiegeNormal		();
	RwTexture	*		GetTextureSiegeDestroied	();

	RwTexture	*		GetRegisterPosTexture		();

	// Message
	virtual BOOL		OnAddModule					( VOID );
	virtual BOOL		OnDestroy					( VOID );
	virtual BOOL		OnInit						( VOID );

	BOOL				OpenDlgAndAddPos			( RwV2d*	pvPosition );

	BOOL				AddMapPosition				( CONST string&	strName , INT nID , RwV2d* pvPosition , INT iRegionIndex = -1, BOOL bTempShow = FALSE );
	BOOL				AddMapPosition				( CONST string&	strName , INT nID , INT nX , INT nY , INT iRegionIndex = -1, BOOL bTempShow = FALSE );

	BOOL				DestroyMapPosition			( CONST string&	strName );
	BOOL				DestroyMapPosition			( INT nID );

	stMapRegisterInfo*	GetMapPosition				( INT nID );
	stMapRegisterInfo*	GetMapPosition				( CONST string& strName );
	stMapRegisterInfo*	GetMapPositionIndex			( INT nIndex );

	INT					GetMapPositionCount			( VOID )		{	return (INT)m_listMapRegisterInfo.size();	}

	VOID				ToggleCameraView			( VOID );
	VOID				ToggleMinimapSize			( VOID );

	VOID				ClearMapPosition			( VOID );

	BOOL				GetMinimapON				( VOID )	{	return m_bMinimapON;	}

	// SetCallback
	BOOL				SetCallbackRegisterMapPos	( ApModuleDefaultCallBack pfCallback , PVOID pClass );

	virtual void OnLuaInitialize( AuLua * pLua );

	static BOOL	CBSelfCharacterPositionCallback	( PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL	CBSetSelfCharacter				( PVOID pData, PVOID pClass, PVOID pCustData);

	static BOOL	CBDisplayPos					( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue);

	static BOOL	CBDisplayRegionName				( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl* pcsSourceControl );

	static BOOL	CBClickPlusButton				( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);
	static BOOL CBClickMinusButton				( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);

	static BOOL	CBClickCameraView				( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);

	static BOOL CBClickMapToggle				( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);

	static BOOL	CBToggleMiniMapUI				( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);
	static BOOL	CBToggleRaiderUI				( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);
	static BOOL	CBToggleWorldMapUI				( PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);

	static BOOL	CBBindingRegionChange			( PVOID pData, PVOID pClass, PVOID pCustData				);
	static BOOL	CBUseMapItem					( PVOID pData, PVOID pClass, PVOID pCustData				);

public:
	// 미니맵 포인트 정보.
	struct MPInfo // MinimapPointInfo
	{
		enum TYPE
		{
			MPI_NPC				,
			MPI_QUESTNEW		, // ? 표 
			MPI_QUESTINCOMPLETE	, // ! 표
			MPI_SIEGENORMAL		,
			MPI_SIEGEDESTROIED	,
			MPI_NONE	
		};

		INT32	nType;
		char	strText[ AGCMMAP_MINIMAP_POINT_TEXT_LENGTH ];
		AuPOS	pos;
		INT32	nControlGroupID;
		INT32	nTextureWidth;

		BOOL	bDisabled	;
		BOOL	bRendered	;


		MPInfo() : bDisabled( FALSE ), nControlGroupID( -1 ) , bRendered ( FALSE ) , nTextureWidth(0)
		{
		}
	};

	std::vector< MPInfo >	m_arrayMPInfo;	// 계배열을 저장
	MPInfo *	GetMPArray() { return m_arrayMPInfo.size() >= 1 ? &m_arrayMPInfo[ 0 ] : NULL; }
	INT32		GetMPCount() { return (INT32)m_arrayMPInfo.size(); }

protected:
	MPInfo *	GetMPInfo( INT32 nIndex ) { return &m_arrayMPInfo[ nIndex ]; }
	INT32		GetDisabledIndex();
public:

	//MinimapPointInfo	*	m_pPointList;
	//INT32					m_nPointList;
	// 좀더 나은 데이타 구조를 정해야지..

	INT32				AddPoint	( INT32 nType , char * strText , AuPOS * pPos , INT32 nControlGroupID = -1 );
	// return : Added point index
	// if error occurs , -1 will be returned

	BOOL				RemovePoint	( INT32 nIndex );
	MPInfo *			GetPointInfo( INT32 nIndex );
	INT32				GetPoint( char * strText );
	INT32				RemovePointGroup( INT32 nControlGroupID );
	// 리턴은 사라진 갯수.

	BOOL				UpdatePointPosition( INT32 nIndex , AuPOS * pPos );	
	// 지정된 점 위치 갱신..

	bool				ToggleShowAllCharacter( bool bUse );
};


#endif // !defined(AFX_AGCMMINIMAP_H__6CE9B6F1_B8DA_42EA_B617_75088D546595__INCLUDED_)
