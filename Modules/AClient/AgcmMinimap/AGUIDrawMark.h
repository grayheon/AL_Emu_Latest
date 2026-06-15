// AcUIWorldmap.h: interface for the AcUIWorldmap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_AGUIDRAWMARK_)
#define _AGUIDRAWMARK_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "AgcModule.h"
#include "AgcdUIManager2.h"
#include "AuXmlParser.h"

class AgcdUI;
class AgcmFont;
class AgpdCharacter;

class AGUIDrawMark
{
public:
	

	enum E_MARKINFO_TYPE
	{
		//월드맵
		E_TYPE_SELF = 0,
		E_TYPE_PARTY,
		E_TYPE_X_MARK,
		//--------------------------------------- 3단계 맵에서만 보여지는 것들
		E_TYPE_NPC,
		E_TYPE_NPC_QUEST_NEW,
		E_TYPE_NPC_QUEST_COMPLETE,
		E_TYPE_PC,
		E_TYPE_SIEGE,
		E_TYPE_SIEGE_DESTROY,

		
		//쉬라인
		E_TYPE_SHRINE_BATTLE,
		E_TYPE_SHRINE_DISABLE,
		E_TYPE_SHRINE_ENABLE,
		E_TYPE_SHRINE_OCCUPATION,
		E_TYPE_SHRINE_HUMN,
		E_TYPE_SHRINE_ORC,
		E_TYPE_SHRINE_MOONELF,
		E_TYPE_SHRINE_DRAGONSION,

	};
	
	struct S_TEXTURE_INFO
	{
		RwTexture *pTexture;
		INT32 iIndex;
	};

	struct S_MARK_INFO
	{
		AgpdCharacter*			pChar;
		INT32							iPosX;
		INT32							iPosY;
		INT32							iID;
		INT32							iCurTexIndex;
		INT32							iGlitterCount;
		INT32							iGroup;

		float							fIntervalTime;
		float							fCurTime;
		BOOL							bGlitter;
		BOOL							bTempShow;			//임시로 한번 보여주고 창이 닫히면 없애주는 처리

		E_MARKINFO_TYPE									eType;

		char							strComment[MAX_PATH];
		char							szRegionTexture[MAX_PATH];
		char							szRegionRoadTexture[MAX_PATH];

		S_MARK_INFO():
		pChar(NULL)
		,iPosX(0)
		,iPosY(0)
		,iCurTexIndex(0)
		,iGroup(0)
		,fIntervalTime(0.f)
		,fCurTime(0.f)
		,iID(-1)
		,eType(E_TYPE_SELF)
		,iGlitterCount(0)
		,bGlitter(FALSE)
		,bTempShow(FALSE)
		{
			memset( strComment, 0, MAX_PATH * sizeof( char ) );
			memset( szRegionTexture, 0, MAX_PATH * sizeof( char ) );
			memset( szRegionRoadTexture, 0, MAX_PATH * sizeof( char ) );
		}

		S_MARK_INFO& operator =( const S_MARK_INFO& info )
		{
			iPosX = info.iPosX;
			iPosY = info.iPosY;
			iCurTexIndex = info.iCurTexIndex;
			iGroup = info.iGroup;
			fIntervalTime = info.fIntervalTime;
			fCurTime = info.fCurTime;
			iID = info.iID;
			eType = info.eType;
			iGlitterCount = info.iGlitterCount;
			bGlitter = info.bGlitter;
			bTempShow = info.bTempShow;

			memcpy( strComment, info.strComment, MAX_PATH * sizeof( char ) );
			memcpy( szRegionTexture, info.szRegionTexture, MAX_PATH * sizeof( char ) );
			memcpy( szRegionRoadTexture, info.szRegionRoadTexture, MAX_PATH * sizeof( char ) );
		}
	};

	struct S_MAP_AREA
	{
		INT32 SX, SZ, EX, EZ;
		S_MAP_AREA():SX(0),SZ(0),EX(0),EZ(0){};
		S_MAP_AREA( INT32 X, INT32 Z, INT32 X2, INT32 Z2 )
		{
			SX = X; SZ = Z; EX = X2; EZ = Z2;
		}
	};

	struct S_MAP_POS
	{
		INT32 x,y;
		S_MAP_POS():x(0),y(0){};
		S_MAP_POS( INT32 _x, INT32 _y ){ x = _x; y = _y; }
	};

	typedef std::list< S_TEXTURE_INFO >		LIST_TEX_INFO;
	typedef LIST_TEX_INFO::iterator			LIST_TEX_INFO_ITR;
	typedef std::list< S_MARK_INFO >			LIST_MARK_INFO;
	typedef LIST_MARK_INFO::iterator			LIST_MARK_INFO_ITR;
	
	AGUIDrawMark();
	~AGUIDrawMark();
	BOOL												Init				( AgcmFont* pAgcmFont);
	BOOL												AddTexture		( E_MARKINFO_TYPE eType, char* pszTextureName );

	BOOL												Add				( AgpdCharacter* pChar, int iGroup, E_MARKINFO_TYPE eType, BOOL bTempShow = FALSE );
	BOOL												Add				( INT32 iID, RwV2d vPos, char* pszComment, int iGroup, E_MARKINFO_TYPE eType , BOOL bTempShow = FALSE);
	BOOL												Update			( INT32 iID, RwV2d vPos, char* pszComment, int iGroup, E_MARKINFO_TYPE eType, BOOL bTempShow = FALSE);

	BOOL												Delete			( INT32 iID, E_MARKINFO_TYPE eType  );
	BOOL												Delete			( INT32 iID );
	void												DeleteAll			(){m_listMarkInfo.clear();};
	void												DeleteTempShowElements();

	AGUIDrawMark::LIST_MARK_INFO_ITR	Find				( INT32 iID );
	AGUIDrawMark::LIST_MARK_INFO_ITR	MarkListEnd		(){ return m_listMarkInfo.end();}
	INT32												MarkListSize	(){ return (INT32)m_listMarkInfo.size(); }

	AGUIDrawMark::S_MAP_POS				GetWorldPos	(S_MAP_POS& stPos);
	INT32												GetHitMark		(){ return m_iCurHitMark; }
	void												UpdateData		(std::vector< INT32 >& vecID , AGUIDrawMark::E_MARKINFO_TYPE eType );
	void												SetMapUI		( AgcdUI* pMapUI ){ m_pMapUI = pMapUI; }
	void												SetMapArea	(S_MAP_AREA stArea ){ m_stMapArea = stArea; }
	void												SetUIMatchingArea( S_MAP_AREA stArea ){m_stUIMatchingArea = stArea; }

	void												Render			(RwV2d vMousePos, int iCurGroup, BOOL bShowMap );

private:
	void												_SetInfo(S_MARK_INFO* pInfo, INT32 iID, RwV2d vPos, char* pszComment, int iGroup, E_MARKINFO_TYPE eType, BOOL bTempShow );
	S_MAP_POS									_GetMapPos	( S_MAP_POS& stPos );
	INT32												_LoadTexture	( char* pszName );
	AGUIDrawMark::LIST_TEX_INFO_ITR	_TextureListEnd(){ return m_listTextureInfo.end();}
	AGUIDrawMark::LIST_TEX_INFO_ITR	_FindTexture		( INT32 iID );

	AgcmFont*					m_pAgcmFont;
	AgcdUI*						m_pMapUI;

	S_MAP_AREA				m_stMapArea;								//현재 리젼 실제 영역
	S_MAP_AREA				m_stUIMatchingArea;

	LIST_TEX_INFO			m_listTextureInfo;					
	LIST_MARK_INFO			m_listMarkInfo;

	typedef vector<INT32>	VECTOR_TEXTUREID;
	typedef map<E_MARKINFO_TYPE, VECTOR_TEXTUREID> MAP_TEXTURE_GROUP;
	MAP_TEXTURE_GROUP	m_mapTextureID;

	INT32							m_iCurHitMark;
	INT32							m_iCreateCount;

	INT32		m_uiTime;
	INT32		m_uiTime2;
};


#endif 
