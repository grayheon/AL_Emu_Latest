#include "AgcChatTab.h"

AgcChatTab::AgcChatTab( VOID )
{
	m_ulChatFlag	=	0;
	m_bPrimary		=	FALSE;
}

AgcChatTab::~AgcChatTab( VOID )
{
	
}

BOOL	AgcChatTab::IsUsableType( UINT ulMsgType )
{
	if( m_ulChatFlag & ulMsgType )
		return TRUE;

	return FALSE;
}