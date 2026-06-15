#pragma once

#include "AcUIList.h"
#include <map>

// 채팅 시스템에서 사용될 ListBox UI
// 탭 버튼이 추가되며... 도킹 기능이 들어간다

class AcUIChatListBox
	:	public	AcUIList
{
private:
	typedef std::map< DWORD , string >				MapMessageInstance;
	typedef std::map< DWORD , string >::iterator	MapMessageInstanceIter;

public:
	AcUIChatListBox			( VOID );
	~AcUIChatListBox		( VOID );

	VOID					InitChatListBox					( VOID );

	// 특정 메세지들을 가지고 있는 메세지 인스턴스를 추가한다
	BOOL					AddMessageInstance				( VOID );
	
	// 삭제
	BOOL					DestroyMessageInstance			( VOID );

	VOID					ClearMessageInstance			( VOID );

	VOID					SetBackgroundColor				( DWORD	dwColor )	{	m_dwBackgroundColor	=	dwColor;	}
	VOID					SetUpdateDelay					( DWORD	dwDelay )	{	m_dwUpdateDelay		=	dwDelay;	}

	VOID					SetDockingWindow				( BOOL	bDocking );
	VOID					SetMoveWindow					( BOOL bMoveWindow );

	virtual BOOL			OnIdle							( UINT32 ulClockCount );

	virtual VOID			OnWindowRender					( VOID );
	virtual VOID			OnSetFocus						( VOID );
	virtual VOID			OnKillFocus						( VOID );

	virtual BOOL			OnMouseMove						( RsMouseStatus *ms );

	virtual BOOL			OnLButtonDown					( RsMouseStatus *ms );
	virtual BOOL			OnLButtonUp						( RsMouseStatus *ms );

protected:
	VOID					_UpdateListBox					( VOID );
	VOID					_CalcListScroll					( VOID );

	BOOL					m_bFocusWindow;			//	포커스를 받았는지 체크하기 위해 사용한다
	BOOL					m_bDockingWindow;		//	도킹 윈도우인지 확인한다

	DWORD					m_dwBackgroundColor;	//	뒷 배경에 그리게 될 색깔

	DWORD					m_dwUpdateDelay;		//	퍼포먼스 향상을 위해 딜레이 타임을 두고 갱신한다
	DWORD					m_dwUpdateLastTime;		//	마지막 업데이트 받은 시간

	UINT					m_ulMaxLine;			//	현재 표시할 수 있는 라인 수

	AcUIScroll				m_csUIScroll;			//	Scroll
	AcUIScrollButton		m_csUIScrollButton;		//	Scroll Button
	AcUIButton				m_csUIScrollUpButton;	//	Scroll Up Button
	AcUIButton				m_csUIScrollDownButton;	//	Scroll Down Button

	MapMessageInstance		m_mapMessageInstance;

};
