#pragma once

#include "AcUIBase.h"
#include <string>
#include <map>
#include <list>

enum	eMenuEventType
{
	E_MENU_EVENT_FUNCTION	,		//	Function 호출
	E_MENU_EVENT_SUB_MENU	,		//	Sub Menu 를 열어준다

	E_MENU_EVENT_COUNT		,
};

enum	eBackTexture
{
	E_MENU_BACK_TEXTURE_LEFT_TOP	,
	E_MENU_BACK_TEXTURE_RIGHT_TOP	,
	E_MENU_BACK_TEXTURE_TOP			,
	E_MENU_BACK_TEXTURE_LEFT		,
	E_MENU_BACK_TEXTURE_RIGHT		,
	E_MENU_BACK_TEXTURE_CENTER		,
	E_MENU_BACK_TEXTURE_BOTTOM		,
	E_MENU_BACK_TEXTURE_LEFT_BOTTOM	,
	E_MENU_BACK_TEXTURE_RIGHT_BOTTOM,
	E_MENU_BACK_TEXTURE_SELECT_LEFT		,
	E_MENU_BACK_TEXTURE_SELECT_CENTER	,
	E_MENU_BACK_TEXTURE_SELECT_RIGHT	,

	E_MENU_BACK_TEXTURE_COUNT		,
};

enum	eDrawType
{
	E_MENU_DRAW_RGB			,
	E_MENU_DRAW_TEXTURE		,
};


typedef BOOL (*fnMenuEvent)( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu );


struct	stMenuEvent 
{
	eMenuEventType			m_eEventType;

	string					m_strMenuName;			//	화면에 찍어주는 이름
	DWORD					m_dwFontColor;			//	폰트 컬러를 지정할 수 있다

	BOOL					m_bEnable;				//	TRUE가 들어가야 동작한다


	PVOID					m_pvData;				//	사용 할때는 캐스팅 해줘야 한다
	//	Callback Arg
	PVOID					m_pClass;
	PVOID					m_pData1;
	PVOID					m_pData2;

	stMenuEvent()
	{
		m_eEventType	=	E_MENU_EVENT_COUNT;
		m_pvData		=	NULL;
		m_pClass		=	NULL;
		m_pData1		=	NULL;
		m_pData2		=	NULL;

		m_bEnable		=	TRUE;

		m_dwFontColor	=	0xffffffff;
	}

};

class AcUIMenu :
	public AcUIBase
{
private:
	typedef		map< string , AcUIMenu* >					MapChildMenu;
	typedef		map< string , AcUIMenu* >::iterator			MapChildMenuIter;

	typedef		list< stMenuEvent >							ListMenuEvent;
	typedef		list< stMenuEvent >::iterator				ListMenuEventIter;

public:
	AcUIMenu										( VOID );
	virtual ~AcUIMenu								( VOID );

	VOID					Initialize				( eDrawType	eType = E_MENU_DRAW_RGB );
	VOID					Clear					( VOID );
	VOID					ClearEvent				( VOID );
	VOID					TextureClear			( VOID );


	VOID					SetBackgroundMode		( eDrawType eType )	{ m_eDrawType	=	eType; }
	virtual VOID			OnWindowRender			( VOID );
	virtual BOOL			OnLButtonDown			( RsMouseStatus *ms );
	virtual BOOL			OnLButtonUp				( RsMouseStatus *ms );
	virtual BOOL			OnLButtonDblClk			( RsMouseStatus *ms );
	virtual BOOL			OnMouseMove				( RsMouseStatus *ms );

	virtual BOOL			OnIdle					( UINT32 ulClockCount );

	virtual VOID			OnSetFocus				( VOID );
	virtual VOID			OnKillFocus				( VOID );

	// 이 컨트롤에 포커스를 준다
	VOID					SetFocusedWindow		( VOID )	{	g_pEngine->m_pFocusedWindow	=	this;	}

	// 기존 메뉴에 Event를 붙이는 작업
	// 반환값은 해당 메뉴 이벤트의 위치 인덱스
	INT						AddMenuEvent			( stMenuEvent&	rMenuEvent );

	stMenuEvent*			GetMenuEvent			( INT	nIndex );
	stMenuEvent*			GetMenuEvent			( CONST string&	strMenuName );

	BOOL					DelMenuEvent			( CONST	string&	strName );
	BOOL					DelMenuEvent			( INT	nIndex );

	// Child 윈도우를 등록하면 나중에 Event에서 Child Menu를 불러올 수 있다.
	AcUIMenu*				CreateChildMenu			( CONST	string&	strName );
	AcUIMenu*				GetChildMenu			( CONST string&	strName );

	BOOL					DeleteChildMenu			( AcUIMenu*&	rChildMenu	);
	BOOL					DeleteChildMenu			( CONST string&	strName );

	// FALSE가 들어가면 이벤트가 안 먹는다
	// 글자색도 회색으로 들어간다
	VOID					EnableEvent				( CONST	string&	strName , BOOL	bEnable );
	VOID					EnableEvent				( INT	nIndex , BOOL	bEnable );

	// 이 메뉴의 기본 이름을 설정한다.
	VOID					SetMenuName				( CONST string&	strName )	{	m_strMenuName	=	strName;	}
	string&					GetMenuName				( VOID )					{	return m_strMenuName;			}

	// 상위에 메뉴 이름이 보일지 안 보일지 정한다.
	VOID					VisibleMenuName			( BOOL	bVisible )			{	m_bVisibleName	=	bVisible;	}

	VOID					SetBackColor			( DWORD	dwColor	)			{	m_dwBackgroundColor	=	dwColor;	}

	VOID					SetEnableLastYPosition	( BOOL	bEnable )			{	m_bEnableLastYPos	=	bEnable;	}
	VOID					SetLastYPosition		( FLOAT	fPosY	)			{	m_fLastYPos			=	fPosY;		}
	VOID					SetDisableFontColor		(DWORD dwColor)				{	m_dwDisableFontColor = dwColor;}


protected:
	VOID					_RenderMenuText			( VOID );
	VOID					_RenderMenuBackground	( VOID );

	VOID					_ProcessEvent			( stMenuEvent*	pEvent );

	VOID					_SetBackImage			( CONST	string&	strTextureFileName , eBackTexture eTextureType );

	VOID					_DrawTextureCorner		( INT nPosX , INT nPosY , INT nWidth , INT nHeight );
	VOID					_DrawTextureBody		( INT nPosX , INT nPosY , INT nWidth , INT nHeight );
	VOID					_DrawTextureSelect		( INT nPosX , INT nPosY , INT nWidth , INT nHeight );

	// 현재 커서 위치에 있는 이벤트 인덱스를 찾는다
	INT						_GetCurrentEventIndex	( OUT INT& nEventX , OUT INT& nEventY  );


	ListMenuEvent			m_listMenuEvent;
	MapChildMenu			m_mapChildMenu;

	string					m_strMenuName;

	RwV2d					m_v2dCurrentMousePos;

	DWORD					m_dwBackgroundColor;
	DWORD					m_dwSelectColor;
	DWORD					m_dwLineColor;
	DWORD					m_dwDisableFontColor;

	INT						m_nLastMouseDownIndex;
	BOOL					m_bVisibleName;
	AcUIMenu*				m_pParentUIMenu;
	eDrawType				m_eDrawType;	//	Back Ground가 어떻게 그려질지 

	static BOOL				m_bTextureLoad;
	static RwTexture*		m_pBackTexture[ E_MENU_BACK_TEXTURE_COUNT ];


	BOOL					m_bEnableLastYPos;
	FLOAT					m_fLastYPos;
};
