#pragma once

#include "AcUIButton.h"
#include "AcUIMenu.h"

typedef BOOL (*fnEventCallback)( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );

class AcUIEventButton
	:	public AcUIButton
{
public:
	AcUIEventButton								( VOID );
	virtual ~AcUIEventButton					( VOID );

	VOID			SetCallbackClickEvent		( PVOID pClass , fnEventCallback	EventCallback );
	VOID			SetCallbackDragEvent		( PVOID pClass , fnEventCallback	EventCallback );

	VOID			SetCallbackSetFocusEvent	( PVOID pClass , fnEventCallback	EventCallback );
	VOID			SetCallbackKillFocusEvent	( PVOID pClass , fnEventCallback	EventCallback );

	virtual BOOL	OnLButtonDown				( RsMouseStatus *ms	);	
	virtual BOOL	OnLButtonUp					( RsMouseStatus *ms );
	virtual BOOL	OnRButtonDown				( RsMouseStatus *ms );
	virtual BOOL	OnRButtonUp					( RsMouseStatus *ms );
	virtual BOOL	OnMouseMove					( RsMouseStatus *ms );

	virtual VOID	OnSetFocus					( VOID );
	virtual VOID	OnKillFocus					( VOID );

	virtual VOID	SetFocus					( VOID );
	virtual VOID	KillFocus					( VOID );

	virtual VOID	OnWindowRender				( VOID );

	RwV2d&			GetDragStartPos				( VOID )	{	return m_vStartDrag;	}
	PVOID			GetTempData					( VOID )	{	return m_pvTempData;	}
	PVOID			GetTempData2					( VOID ) {  return m_pvTempData2;}

	VOID			SetData						( PVOID pData1 , PVOID pData2 )	{	m_pData1	=	pData1;	m_pData2	=	pData2;	}
	VOID			SetDragData					( PVOID pData1 , PVOID pData2 )	{	m_pDragData1=	pData1; m_pDragData2=	pData2;	}
	VOID			SetFocusData				( PVOID pData1 , PVOID pData2 )	{	m_pFocusData1=	pData1;	m_pFocusData2=	pData2;	}

	VOID			SetTempData					( PVOID pData )					{	m_pvTempData=	pData;	}
	VOID			SetTempData2					( PVOID pData )					{	m_pvTempData2=	pData;	}

	VOID			SetCustomRender				( BOOL bEnable )				{	m_bCustomRender	=	bEnable;	}
	VOID			SetCustomLineColor			( DWORD	dwColor )				{	m_dwLineColor	=	dwColor;	}
	VOID			SetCustomBGColor			( DWORD dwColor )				{	m_dwBackgroundColor	=	dwColor;}

	VOID			SetLastClickButton			( AcUIEventButton*	pButton = NULL );

	VOID			SetRButtonClickMenu			( BOOL bEnable );

	VOID			AddMenuEvent				(PVOID pClass , CONST string& strName , PVOID pData1 , PVOID pData2 , fnMenuEvent pfnEvent );
	VOID			EditMenuEventName			( CONST string& strMenuName , CONST string& strChangeName );
	VOID			ClearMenuEvent				( VOID );

	VOID			DestroyButtonMenu			( VOID );

protected:
	VOID			_DrawCustomOnMouse			( VOID );
	VOID			_DrawCustomOnClick			( VOID );

	fnEventCallback				m_fnClickEvent;
	PVOID						m_pClass;
	PVOID						m_pData1;
	PVOID						m_pData2;

	fnEventCallback				m_fnDragEvent;
	PVOID						m_pDragClass;
	PVOID						m_pDragData1;
	PVOID						m_pDragData2;

	fnEventCallback				m_fnFocusEvent;
	PVOID						m_pFocusClass;
	PVOID						m_pFocusData1;
	PVOID						m_pFocusData2;

	fnEventCallback				m_fnKillFocusEvent;
	PVOID						m_pKillFocusClass;
	PVOID						m_pKillFocusData1;
	PVOID						m_pKillFocusData2;


	PVOID						m_pvTempData;
	PVOID						m_pvTempData2;

	RwV2d						m_vStartDrag;

	BOOL						m_bDragStart;
	BOOL						m_bCustomRender;
	BOOL						m_bMenu;

	DWORD						m_dwLineColor;
	DWORD						m_dwBackgroundColor;

	AcUIMenu*					m_pUIMenu;
	static	AcUIEventButton*	m_pLastClickDown;
};
