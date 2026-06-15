#ifndef __CLASS_CHATTING_COMMANDER_H__
#define __CLASS_CHATTING_COMMANDER_H__

#include "ContainerUtil.h"
#include "AcDefine.h"

enum eCmdParamType
{
	ParamType_UnKnown = 0,
	ParamType_Integer,
	ParamType_Float,
	ParamType_Boolean,
	ParamType_String,
	ParamType_Integer64,
};

typedef BOOL ( *fnCallBack_ChatCmd )( void* pCmdData );

struct stCmdParamEntry
{
	int															m_nIndex;
	eCmdParamType												m_eType;

	int															m_nParam;
	INT64														m_n64Param;
	float														m_fParam;
	BOOL														m_bParam;
	std::string													m_strParam;

	stCmdParamEntry( void )
	{
		m_nIndex = 0;
		m_eType = ParamType_Integer;

		m_nParam = 0;
		m_n64Param = 0;
		m_fParam = 0;
		m_bParam = 0;
		m_strParam = "";
	}
};

struct stCmdDataEntry
{
	std::string													m_strCmd;
	fnCallBack_ChatCmd											m_fnCallBack;
	ContainerMap< int, stCmdParamEntry >						m_mapParam;

	stCmdDataEntry( void )
	{
		m_strCmd = "";
		m_fnCallBack = NULL;
	}
};

struct stCmdCallBackEntry
{
	std::string													m_strCallBackName;
	fnCallBack_ChatCmd											m_fnCallBack;

	stCmdCallBackEntry( void )
	{
		m_strCallBackName = "";
		m_fnCallBack = NULL;
	}
};

class CChatCmd
{
private :
	ContainerMap< std::string, stCmdDataEntry >					m_mapCmd;
	ContainerMap< std::string, stCmdCallBackEntry >				m_mapCallBack;
	
	static	BOOL												m_bIsEncrypted;

public :
	CChatCmd( void );
	virtual ~CChatCmd( void );

public :
	virtual BOOL			OnRegisterCmdCallBack				( void );
	virtual BOOL			OnLoadChatCmd						( char* pFileName, BOOL bIsEncrypt );
	virtual BOOL			OnParseCommand						( char* pCmdString );

private :
	BOOL					_AddChatCmd							( void* pXmlNode );

	void					_AddCmdCallBack						( char* pCallBackName, fnCallBack_ChatCmd fnCallBack );
	fnCallBack_ChatCmd		_GetCmdCallBack						( char* pCallBackName );

	eCmdParamType			_ParseParamType						( char* pTypeName );
	void					_ParseParamValue					( stCmdDataEntry* pCmdData, char* pParamString );
	BOOL					_RunCmdCallBack						( stCmdDataEntry* pCmdData );

public :
	static BOOL				CallBack_OnTestEditChatting			( void* pData );

	static BOOL				CallBack_OnAddTextFilter			( void* pData );
	static BOOL				CallBack_OnSocialAnimation			( void* pData );
	static BOOL				CallBack_OnFlagImoticon				( void* pData );
	static BOOL				CallBack_OnDrawOcclusion			( void* pData );
	static BOOL				CallBack_OnReloadTitle				( void* pData );
	static BOOL				CallBack_OnChangeCameraMode			( void* pData );
	static BOOL				CallBack_OnReportModule				( void* pData );
	static BOOL				CallBack_OnCheckMemory				( void* pData );

	static BOOL				CallBack_OnGuildBattleRequest		( void* pData );
	static BOOL				CallBack_OnGuildBattleCancel		( void* pData );
	static BOOL				CallBack_OnGuildBattleSurrender		( void* pData );
	static BOOL				CallBack_OnGuildLeaveOut			( void* pData );

	static BOOL				CallBack_OnChangePerspective		( void* pData );
	static BOOL				CallBack_OnChangeWideScreen			( void* pData );
	static BOOL				CallBack_OnChangeSoundEffect		( void* pData );
	static BOOL				CallBack_OnScriptLua				( void* pData );

	static BOOL				CallBack_OnViewFPS					( void* pData );
	static BOOL				CallBack_OnTest3DSound				( void* pData );
	static BOOL				CallBack_OnChangeTime				( void* pData );
	static BOOL				CallBack_OnToggleMatD3DFX			( void* pData );

	static BOOL				CallBack_OnNoticeMessage			( void* pData );
	static BOOL				CallBack_OnNoticeMessageTest		( void* pData );

	static BOOL				CallBack_OnInviteParty				( void* pData );
	static BOOL				CallBack_OnSendMsgShout				( void* pData );
	static BOOL				CallBack_OnSendMsgJoin				( void* pData );
	static BOOL				CallBack_OnSendMsgLeave				( void* pData );

	static BOOL				CallBack_OnMoveToPosition			( void* pData );

	static BOOL				CallBack_OnRejectTradeOn			( void* pData );
	static BOOL				CallBack_OnRejectTradeOff			( void* pData );

	static BOOL				CallBack_OnRejectPartyOn			( void* pData );
	static BOOL				CallBack_OnRejectPartyOff			( void* pData );

	static BOOL				CallBack_OnRejectGuildOn			( void* pData );
	static BOOL				CallBack_OnRejectGuildOff			( void* pData );

	static BOOL				CallBack_OnRejectGuildBattleOn		( void* pData );
	static BOOL				CallBack_OnRejectGuildBattleOff		( void* pData );

	static BOOL				CallBack_OnRejectGuildRelationOn	( void* pData );
	static BOOL				CallBack_OnRejectGuildRelationOff	( void* pData );

	static BOOL				CallBack_OnRejectBattleOn			( void* pData );
	static BOOL				CallBack_OnRejectBattleOff			( void* pData );

	static BOOL				CallBack_OnRejectBuddyOn			( void* pData );
	static BOOL				CallBack_OnRejectBuddyOff			( void* pData );

	static BOOL				CallBack_OnReloadCamSetting			( void* pData );
// 	static BOOL				CallBack_OnReloadNPCDialog			( void* pData );

	static BOOL				CallBack_OnReloadQuest				( void* pData );
	static BOOL				CallBack_OnReloadGuide				( void * pData );
	static BOOL				CallBack_OnReloadWorldmap			( void * pData );
};



#endif