// AcUIMinimap.h: interface for the AcUIMinimap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACUIMINIMAP_H__BB23B70C_035A_4A27_AD02_ADE2A23C00F3__INCLUDED_)
#define AFX_ACUIMINIMAP_H__BB23B70C_035A_4A27_AD02_ADE2A23C00F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AcUIBase.h"

class AgcmMinimap;
class AgpmCharacter;

#define	ACUIMINIMAP_TEXTURE_SIZE	237

#define ACUIMINIMAP_MAP_SIZE		193

#define ACUIMINIMAP_NEXT_DURATION	60

class AcUIMinimap : public AcUIBase  
{
public:
	AcUIMinimap				( VOID );
	virtual ~AcUIMinimap	( VOID );

	INT32				m_nXPos				;	// 좌표를 픽셀값으로 가지고 있음..
	INT32				m_nZPos				;

	UINT32				m_uLastChangeTime	;
	FLOAT				m_fCurrentTurnY		;

	AgcmMinimap	*		m_pAgcmMinimap		;
	AgpmCharacter *		m_pAgpmCharacter	;

	INT32				m_nDivisionIndex	;
	INT32				m_nPartIndex		;

	FLOAT				m_fStartX			;
	FLOAT				m_fStartZ			;
	FLOAT				m_fWidth			;

	RsMouseStatus		m_prevMouseState	;

public:

	// 모듈 포인터 지정해둠.
	void					SetMinimapModule	( AgcmMinimap * pMinimapModule )		{ m_pAgcmMinimap	= pMinimapModule;	}
	void					SetCharacterModule	( AgpmCharacter * pCharacterModule)		{ m_pAgpmCharacter	= pCharacterModule;	}
	void					PositionUpdate		( VOID );

	// 오퍼레이션

	BOOL					RenderMinimap		( VOID );
	VOID					RenderETC			( VOID );
	VOID					RenderPartyMember	( VOID );

	FLOAT					MapEnlargement		( VOID );
	FLOAT					MapReduction		( VOID );

	VOID					RenderRegisterPos	( VOID );

	VOID					RenderDisplayRegisterPopup ( VOID );
	VOID					RenderDisplayPopup	( VOID );
	VOID					RenderCursor		( VOID );
	RwTexture *				RenderDivision		( INT32 nDivisionIdnex , INT32 nPartIndex );
	// 렌더한다.

	// 메시지처리.
	virtual BOOL 			OnInit				( VOID );
	virtual BOOL			OnIdle				( UINT32 ulClockCount );
	virtual	void 			OnWindowRender		( VOID );
	virtual void 			OnClose				( VOID );

	virtual	BOOL 			OnMouseMove			( RsMouseStatus *ms	);
	virtual	BOOL 			OnLButtonDown		( RsMouseStatus *ms	);
	virtual	BOOL 			OnRButtonUp			( RsMouseStatus *ms	);
	virtual BOOL			OnRButtonDblClk		( RsMouseStatus *ms );

	INT32					GetMMPos			( FLOAT fPos );
	INT32					GetMMPosINV			( INT32 nPos );

	INT32					GetTextureSize		( VOID );

	BOOL					DestroyMapPosition	( RwV2d*	pvPosition );

private:
	BOOL					_RenderCameraCreate	( VOID );
	VOID					_RenderCameraClear	( VOID );

	RwCamera*				m_pRenderCamera;
	RwTexture*				m_pRenderTexture;
	RwTexture*				m_pMaskTex;

	INT						m_lDisplayCount;
	FLOAT					m_fMapSizeOffset;
};

#endif // !defined(AFX_ACUIMINIMAP_H__BB23B70C_035A_4A27_AD02_ADE2A23C00F3__INCLUDED_)
