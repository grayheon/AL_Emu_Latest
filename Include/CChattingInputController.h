#ifndef __CLASS_CHATTING_INPUT_CONTROLLER_H__
#define __CLASS_CHATTING_INPUT_CONTROLLER_H__



#include "AcUIChattingEdit.h"
#include "AgcdUIManager2.h"
#include <string>

#include "AcUIMenu.h"

class CChattingInputController
{
private :
	AcUIChattingEdit								m_EditChatting;
	int												m_nEventOnShow;
	std::string										m_strAutoReplyTargetName;

	AcUIMenu										m_csInputMenu;

	DWORD											m_dwCurrentStatusColor;
	string											m_strStatus;

	AgcdUIUserData*									m_pcsStatusUserData;

	INT32											m_DummyData;

public :
	CChattingInputController( void );
	virtual ~CChattingInputController( void );

public :
	BOOL				OnInitEditChatting			( void );
	BOOL				OnDestroyEditChatting		( void );

	VOID				InitMenu					( VOID );

	BOOL				OnAddCustomControl			( void );

	BOOL				OnSetChattingMessage		( char* pMsg );
	BOOL				OnAddChattingMessage		( CHAR* pMsg , DWORD dwColor = 0xffffffff);

	BOOL				OnShowEditChatting			( void );

	VOID				ReCalcChatInputPosition		( void );

	VOID				ActiveChatInput				( VOID );

	BOOL				OnReadyStatusEvent			( VOID );
	BOOL				OnReadyAutoReplyWhisper		( void );
	BOOL				OnReadyAutoReplyWhisper		( char* pTargetName );
	BOOL				OnReadyItemRegister			( CONST CHAR*	pItemName   , INT lID );
	BOOL				OnReadyMapPosRegister		( CONST CHAR*	pMapPosName , INT nID , INT nX , INT nY );

	VOID				OpenInputStatusMenu			( INT nX , INT nY );
	VOID				ChangeInputStatus			( ChattingEditInputType eInputType , BOOL bRefresh = TRUE );

	string				GetToID						( CONST CHAR* pMessage );

public :
	void				SetAutoReplyWhisperTarget	( char* pTargetName ) { m_strAutoReplyTargetName = pTargetName ? pTargetName : ""; }

public :
	static BOOL			CallBack_OnInputStart		( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl );
	static BOOL			CallBack_OnInputEnd			( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl );
	static BOOL			CallBack_OnInputEndPrivate	( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl );
	static BOOL			CallBack_OnAutoReplyWhisper	( void* pClass, void* pData1, void* pData2, void* pData3, void* pData4, void* pData5, ApBase* pTarget, AgcdUIControl* pcdSourceControl );

	// 입력창 클릭 이벤트
	static BOOL			CallBack_OnInputStatusClick	( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase* pTarget , AgcdUIControl* pcdSourceControl );

	// 입력창 상태 디스플레이
	static BOOL			Callback_DisplayStatus		( PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl* pcsSourceControl );

	// Menu CB
	static BOOL			CBMenuStatusChange			( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pMenu );

	static BOOL			CallBack_StatusRefresh		( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 );

private:
	
};



#endif