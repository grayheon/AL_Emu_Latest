// AcUIWorldmap.h: interface for the AcUIWorldmap class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "AgcModule.h"
#include "AgcdUIManager2.h"
#include "AuXmlParser.h"
#include "AGUIDrawMark.h"

class AgcmUIManager2;
class AgcdUIControl;
class AgcdUI;
class ApmMap;
class AgpdCharacter;
class AgcmFont;
class AgpmParty;
class AgpmCharacter;
class AgcmUIChatting2;

//////////////////////////////////////////////////////////////////////////
namespace WORLD_MAP
{
	static std::vector< string > vecGroupName;		
	static BOOL LoadWorldName()
	{
		AuXmlParser XMLDocument;
		if( !XMLDocument.LoadXMLFile( "INI\\WORLDMAPNAME.XML" ) )
		{
			MessageBox(0, "Can not open file[ WORLDMAPNAME.XML ]","", MB_OK );
			return FALSE;
		}

		AuXmlElement * pWorld = XMLDocument.FirstChildElement("World");
		if( pWorld )
		{
			vecGroupName.push_back( pWorld->GetText() );
			AuXmlElement* pContinent = pWorld->FirstChildElement("Camp");
			while( pContinent )
			{
				vecGroupName.push_back( pContinent->GetText() );
				AuXmlElement* pRegion = pContinent->FirstChildElement("Region");
				while( pRegion )
				{
					vecGroupName.push_back( pRegion->GetText() );
					pRegion = pRegion->NextSiblingElement("Region");
				}
				pContinent = pContinent->NextSiblingElement("Camp");
			}
		}

		return TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////
class AgcmUIWorldmap;
class AcUIWorldmapRender : public AcUIBase  
{
public:
	AcUIWorldmapRender():m_pAgcmUIWorldmap(NULL){}
	virtual ~AcUIWorldmapRender(){}

	virtual	void 			OnWindowRender		( VOID );
	void						Set(AgcmUIWorldmap* pUI ){ m_pAgcmUIWorldmap = pUI; }
private:
	AgcmUIWorldmap* m_pAgcmUIWorldmap;
};

class AgcmUIWorldmap : public   AgcModule
{
public:

	AgcmUIWorldmap();
	virtual ~AgcmUIWorldmap();

	friend class AcUIWorldmapRender;

	virtual BOOL					OnAddModule();
	virtual BOOL					OnInit();
	virtual BOOL					OnDestroy();
	virtual BOOL					OnIdle(UINT32 ulClockCount);

	virtual BOOL					AddEvent();
	virtual BOOL					AddFunction();
	virtual BOOL					AddDisplay();
	virtual BOOL					AddUserData();
	virtual BOOL					AddBoolean();

	BOOL							SetGrouping();
	BOOL							AllHideControl();
	BOOL							ShowControl( int iGroup );
	BOOL							AddPoint( const char* pComment, RwV2d vPos, AGUIDrawMark::E_MARKINFO_TYPE eType , INT iRegionIndex = -1, BOOL bTempShow = FALSE );
	BOOL							RemovePoint( const char* pComment, RwV2d vPos );
	BOOL							RemovePoint( int iID );
	void							DeleteAllMark(){ m_DrawMark.DeleteAll(); }

	BOOL							OpenWorldMap	( INT nRegionTemplateIndex = -1 );
	BOOL							CloseWorldMap	();

	//////////////////////////////////////////////////////////////////////////
	//CALLBACK FUNCTION
	//////////////////////////////////////////////////////////////////////////
	static BOOL						CBWorldMapToggle		(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL						CBRightClick				(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL						CBLeftClick					(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL						CBRoadMapToggle		(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL						CBMapEventPos			(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl );
	static BOOL						CBBindingRegionChange(PVOID	pData,PVOID	pClass,PVOID	pCustData);

	//////////////////////////////////////////////////////////////////////////
	//DISPLAY FUNCTION
	//////////////////////////////////////////////////////////////////////////
	static BOOL						CBDisplayWorldName( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );
	static BOOL						CBDisplayRegionName( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl );

private:
	BOOL							_ChangeMap( INT32 iWorldMapIndex );
	int								_GetGroupIndex( const char* strGroupName );
	//////////////////////////////////////////////////////////////////////////
	// stataic function
	//////////////////////////////////////////////////////////////////////////
	static void						S_ChangeMap	(AgcmUIWorldmap* pThis, AgcdUI* pWorlMap, const char* pszGroupName  );
	static void						S_Close			(AgcmUIWorldmap* pThis, AgcdUI* pWorlMap);

	AgcmUIManager2*													m_pcsUIMgr;
	ApmMap*															m_pcsApmMap;
	AgpmCharacter*													m_pcsAgpmChar;
	AgpdCharacter*													m_pAgpdSelfChar;
	AgcdUI*																m_pUIWorldMap;
	AgpmParty*														m_pcsParty;		
	AgcmUIChatting2*												m_pcsAgcmUIChatting2;

	std::vector< std::vector<AgcdUIControl*> >						m_vecCtrlGroup;

	AgcdUIUserData*													m_pUDWorldName;
	AgcdUIUserData*													m_pUDRegionName;

	int																		m_iCurGroup;

	BOOL																	m_bChecking;
	BOOL																	m_bShowRoadMap;
	BOOL																	m_bShowWorldMap;

	AcUIWorldmapRender											m_WorldMapRender;
	AGUIDrawMark														m_DrawMark;

	INT32															m_nEventPaperSound;

	// 성주 정보
public :
	enum AuCastleOwnerType
	{
		CastleOwner_Invalid							= 0,
		CastleOwner_Human,
		CastleOwner_Orc,
		CastleOwner_MoonElf,
		CastleOwner_DragonScion,
		CastleOwner_Archlord,
	};

	enum AuCastleInfoType
	{
		CastleInfo_UnKnown							= 0,
		CastleInfo_OwnerGuildName,
		CastleInfo_GuildMasterName,
		CastleInfo_GuildMark,
	};
private :
	float													m_fAlpha;
	float													m_fMinimumAlphaLimit;

	INT32													m_nEventGetSlideBar;

	INT32													m_nEventGetBtnGuildMarkHuman;
	INT32													m_nEventGetBtnGuildMarkHuman2;
	INT32													m_nEventGetBtnGuildMarkHuman3;

	INT32													m_nEventGetBtnGuildMarkOrc;
	INT32													m_nEventGetBtnGuildMarkOrc2;
	INT32													m_nEventGetBtnGuildMarkOrc3;

	INT32													m_nEventGetBtnGuildMarkMoonElf;
	INT32													m_nEventGetBtnGuildMarkMoonElf2;
	INT32													m_nEventGetBtnGuildMarkMoonElf3;

	INT32													m_nEventGetBtnGuildMarkDragonSion;
	INT32													m_nEventGetBtnGuildMarkDragonSion2;
	INT32													m_nEventGetBtnGuildMarkDragonSion3;

	INT32													m_nEventGetBtnGuildMarkArchlord;
	INT32													m_nEventGetBtnGuildMarkArchlord2;
	INT32													m_nEventGetBtnGuildMarkArchlord3;

	AcUIScroll*												m_pScrollAlpha;

	AcUIButton*												m_pCastleGuildMarkHuman;
	AcUIButton*												m_pCastleGuildMarkHuman2;
	AcUIButton*												m_pCastleGuildMarkHuman3;

	AcUIButton*												m_pCastleGuildMarkOrc;
	AcUIButton*												m_pCastleGuildMarkOrc2;
	AcUIButton*												m_pCastleGuildMarkOrc3;

	AcUIButton*												m_pCastleGuildMarkMoonElf;
	AcUIButton*												m_pCastleGuildMarkMoonElf2;
	AcUIButton*												m_pCastleGuildMarkMoonElf3;

	AcUIButton*												m_pCastleGuildMarkDragonSion;
	AcUIButton*												m_pCastleGuildMarkDragonSion2;
	AcUIButton*												m_pCastleGuildMarkDragonSion3;

	AcUIButton*												m_pCastleGuildMarkArchlord;
	AcUIButton*												m_pCastleGuildMarkArchlord2;
	AcUIButton*												m_pCastleGuildMarkArchlord3;

	AgcdUIUserData*											m_pcdUIUserData;
	CHAR													m_strDisplayText[ 32 ];
	BOOL													m_bViewCastleInfo;

public:
	void				OnUpdateCastleInfo					( AuCastleOwnerType eCastleType, AuCastleInfoType eInfoType, void* pData );
	void				OnUpdateGuildMarkTexture			( AuCastleOwnerType eCastleType );
	void				RenderButton						( AcUIButton* pBtn, void* pTexture );

	void				OnGetControls						( VOID );

	void*				GetCastleInfomation					( AuCastleOwnerType eCastleType );
	void*				GetCastleOwnerGuild					( AuCastleOwnerType eCastleType );
	CHAR*				GetCastleOwnerGuildName				( AuCastleOwnerType eCastleType );
	CHAR*				GetCastleOwnerGuildMasterName		( AuCastleOwnerType eCastleType );
	void*				GetCastleOwnerGuildMarkBG			( AuCastleOwnerType eCastleType );
	void*				GetCastleOwnerGuildMarkPattern		( AuCastleOwnerType eCastleType );
	void*				GetCastleOwnerGuildMarkSimbol		( AuCastleOwnerType eCastleType );
	
	static BOOL			CB_GetBtnCastleHuman				( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleHuman2				( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleHuman3				( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );

	static BOOL			CB_GetBtnCastleOrc					( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleOrc2					( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleOrc3					( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );

	static BOOL			CB_GetBtnCastleMoonElf				( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleMoonElf2				( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleMoonElf3				( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );

	static BOOL			CB_GetBtnCastleDragonSion			( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleDragonSion2			( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleDragonSion3			( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );

	static BOOL			CB_GetBtnCastleArchlord				( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleArchlord2			( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );
	static BOOL			CB_GetBtnCastleArchlord3			( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pControl );

	static BOOL			CB_OnDisplayOwnerGuildHuman			( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );
	static BOOL			CB_OnDisplayOwnerNameHuman			( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );

	static BOOL			CB_OnDisplayOwnerGuildOrc			( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );
	static BOOL			CB_OnDisplayOwnerNameOrc			( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );

	static BOOL			CB_OnDisplayOwnerGuildMoonElf		( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );
	static BOOL			CB_OnDisplayOwnerNameMoonElf		( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );

	static BOOL			CB_OnDisplayOwnerGuildDragonScion	( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );
	static BOOL			CB_OnDisplayOwnerNameDragonScion	( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );

	static BOOL			CB_OnDisplayOwnerGuildArchlord		( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );
	static BOOL			CB_OnDisplayOwnerNameArchlord		( void* pClass, void* pData, AgcdUIDataType eType, INT32 nID, CHAR* pDisplay, INT32* pValue );

};