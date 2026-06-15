#include "AcUIChatListBox.h"

AcUIChatListBox::AcUIChatListBox( VOID )
{
	m_bFocusWindow		=	FALSE;
	m_bDockingWindow	=	FALSE;
	m_dwUpdateDelay		=	0;
	m_dwBackgroundColor	=	0xffffffff;
	m_dwUpdateLastTime	=	0;
	m_ulMaxLine			=	0;
}

AcUIChatListBox::~AcUIChatListBox( VOID )
{
}

VOID	AcUIChatListBox::InitChatListBox( VOID )
{
	this->SetListScroll( &m_csUIScroll );

	//m_csUIScroll.SetScrollButton()
}

VOID	AcUIChatListBox::OnWindowRender( VOID )
{
	// Focus가 있을때만 살짝 이미지를 깔아준다
	if( m_bFocusWindow )
	{
		g_pEngine->Draw2DPanel( this->x , this->y , this->w , this->h , m_dwBackgroundColor );
	}

	AcUIList::OnWindowRender();
}

BOOL	AcUIChatListBox::OnIdle( UINT32 ulClockCount )
{
	if( (m_dwUpdateLastTime+m_dwUpdateDelay) < ulClockCount )
	{
		_UpdateListBox();

		m_dwUpdateLastTime	=	ulClockCount;
	}

	return TRUE;
}

VOID	AcUIChatListBox::OnSetFocus( VOID )
{
	m_bFocusWindow	=	TRUE;

	AcUIList::OnSetFocus();
}

VOID	AcUIChatListBox::OnKillFocus( VOID )
{
	m_bFocusWindow	=	FALSE;

	AcUIList::OnKillFocus();
}

BOOL	AcUIChatListBox::OnMouseMove( RsMouseStatus *ms )
{
	return AcUIList::OnMouseMove( ms );
}

BOOL	AcUIChatListBox::OnLButtonDown( RsMouseStatus *ms )
{
	return AcUIList::OnLButtonDown( ms );
}

BOOL	AcUIChatListBox::OnLButtonUp( RsMouseStatus *ms )
{
	return AcUIList::OnLButtonUp( ms );
}

VOID AcUIChatListBox::SetMoveWindow( BOOL bMoveWindow )
{
	m_Property.bUseInput	=	bMoveWindow;
	m_Property.bMovable		=	bMoveWindow;
}

VOID AcUIChatListBox::SetDockingWindow( BOOL bDocking )
{
	m_bDockingWindow	=	bDocking;
}
VOID AcUIChatListBox::_CalcListScroll( VOID )
{
	INT		nCount	=	static_cast< INT >( m_mapMessageInstance.size() );
	INT		nSize	=	nCount-m_ulMaxLine;

	if( nSize	< 0 )
		nSize	=	0;

	m_csUIScroll.m_fScrollUnit	=	1.0f / nSize;

}