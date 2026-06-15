#ifndef __CLASS_SYSTEM_MESSAGE_MANAGER_H__
#define __CLASS_SYSTEM_MESSAGE_MANAGER_H__

#include "AgpmSystemMessage.h"
#include "ContainerUtil.h"
#include "CChattingCommander.h"
#include <string>

enum eSystemMsgUseType
{
	MsgUseType_Message = 0,
	MegUseType_Format,
};

struct stSystemMsgEntry
{
	eAgpmSystemMessageCode											m_eCode;
	eSystemMsgUseType												m_eUseType;

	std::string														m_strString;
	ContainerMap< int, stCmdParamEntry >							m_mapParam;

	DWORD															m_dwColor;

	stSystemMsgEntry( void )
	{
		m_eCode = ( eAgpmSystemMessageCode)-1;
		m_eUseType = MsgUseType_Message;
		m_strString = "";
		m_dwColor = 0xFFFFFFFF;
	}
};

class CSystemMsgManager
{
private :
	ContainerMap< eAgpmSystemMessageCode, stSystemMsgEntry >		m_mapSystemMsg;
	char															m_strBuffer[ 256 ];

public :
	CSystemMsgManager( void );
	virtual ~CSystemMsgManager( void );

public :
	BOOL					OnRegisterSystemMsgCallBack				( void );
	BOOL					OnReceiveSystemMessage					( AgpdSystemMessage* pMsg );

	BOOL					OnMsgDlgModal							( char* pMsg );
	BOOL					OnMsgDlgModalless						( char* pMsg );
	BOOL					NoticeMessage							( CONST string&	strMessage );

private :
	BOOL					_LoadSystemMsgFormat					( char* pFileName );
	BOOL					_AddSystemMessageEntry					( void* pNode );
	void					_AddSystemMessageEntry					( eAgpmSystemMessageCode eCode, eSystemMsgUseType eUseType, char* pString = NULL );
	std::string				_MakeSystemMessage						( AgpdSystemMessage* pMsg );
	DWORD					_GetSystemMessageColor					( AgpdSystemMessage* pMsg );
	DWORD					_MakeColorValue							( const char* pColorR, const char* pColorG, const char* pColorB );

public :
	static BOOL				CallBack_OnSystemMessage				( void* pData, void* pClass, void* pCustData );
};




#endif