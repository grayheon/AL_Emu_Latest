#pragma once

#include "AcChatDefine.h"
#include "AgcChatMessage.h"
#include <map>
#include <string>

class AgcChatTab
{
private:

public:
	AgcChatTab								( VOID );
	~AgcChatTab								( VOID );

	VOID					Clear			( VOID );

	// Set
	VOID					SetTabName		( CONST string&	strTabName )	{ m_strTabName	=	strTabName;	}
	VOID					SetTabFlag		( UINT ulMsgType )				{ m_ulChatFlag	=	ulMsgType;	}
	VOID					SetPrimary		( BOOL	bPrimary )				{ m_bPrimary	=	bPrimary;	}

	// Get
	string&					GetTabName		( VOID )						{ return m_strTabName;			}
	UINT					GetTabType		( VOID )						{ return m_ulChatFlag;			}
	
	BOOL					IsPrimary		( VOID )						{ return m_bPrimary;			}
	BOOL					IsUsableType	( UINT ulMsgType );
	
private:
	string					m_strTabName;			//	탭 이름
	UINT					m_ulChatFlag;			//	탭에 설정된 대화창
	BOOL					m_bPrimary;				//	Primary 속성을 가지면 지워지지도 않고 수정도 안된다.
};
