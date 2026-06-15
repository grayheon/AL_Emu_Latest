#ifndef __CLASS_OPTION_FLAG_CONTROLLER_H__
#define __CLASS_OPTION_FLAG_CONTROLLER_H__




#include "ContainerUtil.h"
#include <string>

#include "AgpdCharacter.h"



struct stMsgDataEntry
{
	std::string														m_strMessage;
	DWORD															m_dwColor;

	stMsgDataEntry( void )
	{
		m_strMessage = "";
		m_dwColor = 0xFFFFFFFF;
	}
};

struct stOptionFlagMsgEntry
{
	stMsgDataEntry													m_MsgON;
	stMsgDataEntry													m_MsgOFF;

	stOptionFlagMsgEntry( void )
	{
	}
};

class COptionFlagController
{
private :
	ContainerMap< AgpdCharacterOptionFlag, stOptionFlagMsgEntry >	m_mapOptionFlagMsg;

public :
	COptionFlagController( void );
	virtual ~COptionFlagController( void );

public :
	BOOL				OnInitOptionFlagController					( void );
	BOOL				OnChangeOptionFlag							( int nFlag );
	BOOL				OnRequestChangeOptionFlag					( AgpdCharacterOptionFlag eFlag, BOOL bIsSet );

private :
	void				_AddOptionFlagMsg							( AgpdCharacterOptionFlag eFlag, char* pMsgON, char* pMsgOFF, char* pColorON, char* pColorOFF );
	void				_CheckOptionFlag							( int nPrev, int nCurr, AgpdCharacterOptionFlag eFlag );
	void				_SendMsgChangeOptionFlag					( AgpdCharacterOptionFlag eFlag, BOOL bIsOn );
};



#endif