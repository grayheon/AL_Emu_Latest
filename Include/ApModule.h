/******************************************************************************
Module:  ApModule.h
Notices: Copyright (c) 2002 netong
Purpose: 
Last Update: 2002. 04. 11
******************************************************************************/

#if !defined(__APMODULE_H__)
#define __APMODULE_H__

#include "ApBase.h"
#include "ApPacket.h"
#include "ApArray.h"
#include "ApErrorCode.h"
#include "ApMutualEx.h"
#include "ApMemory.h"
#include "ApMemoryPool.h"

#include "AuProfileManager.h"

#pragma warning( disable : 4786 )
#pragma warning ( disable : 4819 )
#pragma comment( lib , "winmm" )

// ApModule
///////////////////////////////////////////////////////////////////////////////
const int APMODULE_MAX_MODULE_NAME						= 40;
const int APMODULE_MAX_MODULE_NUMBER					= 120;
const int APMODULE_MAX_CALLBACK_POINT					= 130;
const int APMODULE_MAX_CALLBACK_NUMBER					= 130;
const int APMODULE_MAX_MODULE_PRIMITIVE_DATA_SIZE		= 1024;
const int APMODULE_MAX_UPDATE_FUNC_NUMBER				= 20;

const int APMODULE_MAX_MODULEPACKET_TYPE				= 10;

/*
// Debug Mode일 때만 Guard Page를 사용
#ifdef _DEBUG
#define	REGISTER_MODULE(_ClassPtr, _Class)						\
	{															\
		BOOL	bRet;											\
		ApGuardArray* pTemp = NULL;								\
		VERIFY( pTemp = new ApGuardArray( sizeof(_Class) ) );	\
		_ClassPtr = new (pTemp->GetValidPointer()) _Class;		\
		VERIFY( bRet = _ClassPtr->AddModule( this, TRUE ) );	\
		if( ! bRet  )											\
		{														\
			delete _ClassPtr;									\
			return FALSE;										\
		}														\
	}

#else
*/
#define	REGISTER_MODULE(_ClassPtr, _Class)				\
	{													\
		BOOL	bRet;									\
		_ClassPtr = new _Class;							\
		VERIFY( _ClassPtr );							\
		bRet = _ClassPtr->AddModule( this, TRUE );		\
		VERIFY( bRet );									\
		if( ! bRet  )									\
		{												\
			delete _ClassPtr;							\
			return FALSE;								\
		}												\
	}

#define	REGISTER_MODULE_SINGLETON(_Class)				\
	{													\
		BOOL	bRet;									\
		bRet = _Class::GetInstance().AddModule( this, FALSE );	\
		VERIFY( bRet );									\
		if( ! bRet  )									\
		{												\
			return FALSE;								\
		}												\
	}
//#endif

#define APMODULE_TYPE_PUBLIC	1
#define APMODULE_TYPE_CLIENT	2
#define APMODULE_TYPE_SERVER	3

#define APMODULE_ALIGN_4BYTE(size)		((size) % 4 ? ((size) + (4 - ((size) % 4))):(size))

typedef enum _eApModuleSendType {
	APMODULE_SENDPACKET_PLAYER					= 0,
	APMODULE_SENDPACKET_ALL_PLAYERS,
	APMODULE_SENDPACKET_ALL_SERVERS
} eApModuleSendType;

class ApModuleStream;
class ApModule;

typedef BOOL	(*ApModuleDataCallBack) (PVOID pData, PVOID pClass, INT16 nDataIndex);
typedef BOOL	(*ApModuleDefaultCallBack) (PVOID pData, PVOID pClass, PVOID pCustData);
typedef BOOL	(*ApModuleUpdateCallBack) (PVOID pClass);
typedef BOOL	(*ApModuleStreamReadCallBack) (PVOID pData, ApModule *pClass, ApModuleStream *pclStream);
typedef BOOL	(*ApModuleStreamWriteCallBack) (PVOID pData, ApModule *pClass, ApModuleStream *pclStream);

// stMODULEIDLEEVENT
//		- 모듈을 등록할때 사용되는 구조체
///////////////////////////////////////////////////////////////////////////////
typedef struct _stMODULEIDLEEVENT {
	UINT32		ulClockCount;					// 이 이벤트를 처리해야할 시간
	INT32		lCID;							// 처리해야할 캐릭터
	PVOID		pClass;							// 이벤트를 처리할 클래스
	PVOID		pvData;
	BOOL		(*pfnProcessFunc) (INT32 lCID, PVOID pClass, UINT32 ulClockCount, PVOID pvData);		// 이벤트 처리 함수

	_stMODULEIDLEEVENT*		next;
} stMODULEIDLEEVENT, *pstMODULEIDLEEVENT;

// stATTACHEDDATA
///////////////////////////////////////////////////////////////////////////////
typedef struct _stATTACHEDDATA {
	INT32	nDataSize;		// attached data size
	INT32	nDataOffset;	// attached data offset (in parent module)
	INT32	nDataIndex;		// parent module data index
	//PVOID	pParentModule;	// module pointer of attached data
	PVOID	pAttachedModule;	// 데이타를 붙인 모듈
} stATTACHEDDATA, *pstATTACHEDDATA;

typedef struct _DispatchArg {
	BOOL	bReceivedFromServer;
	INT32	lSocketOwnerID;
} DispatchArg;

// Add by gemani
typedef struct tag_CallBackData
{
	PVOID						pClass;
	ApModuleDefaultCallBack		pFunc;
	tag_CallBackData*			next;
}CallBackData;

typedef struct tag_PreProcessPacket
{
	ApModule*					pModule;
	tag_PreProcessPacket*		next;
}PreProcessPacket;

struct StreamData
{
	ApModuleStreamReadCallBack	pReadFunc;
	ApModuleStreamWriteCallBack pWriteFunc;
	ApModule*					pModule;

	StreamData*					next;
};

typedef enum
{
	APMODULE_STATUS_INIT	=	0,
	APMODULE_STATUS_ADDED,
	APMODULE_STATUS_READY,
	APMODULE_STATUS_PREDESTROYED,
	APMODULE_STATUS_DESTROYED,
} ApModuleStatus;


// ApModule Class
//	- Module Base Class
///////////////////////////////////////////////////////////////////////////////
class ApModule {
private:
	// Module type
	INT32			m_nModuleType;

	// 기본 적인 Module에 대한 정보들 (Name, Module Manager)
	CHAR			m_szName[APMODULE_MAX_MODULE_NAME];
	PVOID			m_pModuleManager;

	// Attach된 Data의 정보와 개수
	INT32			m_nAttachedDataNumber;		// 5마다 realloc
	stATTACHEDDATA*	m_stAttachedData;

	// Module이 관리하는 Data의 Size
	INT32			m_nDataSize[APMODULE_MAX_MODULE_NUMBER];

	// Attach된 Data의 Constructor와 Destructor
	CallBackData*	m_listConstructor[APMODULE_MAX_MODULE_NUMBER];
	CallBackData*	m_listDestructor[APMODULE_MAX_MODULE_NUMBER];

	// Callback으로 등록된 함수들과 개수
	CallBackData*	m_listCallback[APMODULE_MAX_CALLBACK_POINT];

	// 이 Module이 Packet을 받기 전에 전처리할 Module들
	PreProcessPacket*	m_listPreRecieveModule;

	ApMutualEx		m_MutexIdleEvent;
	ApArray<DONT_USE_AUTOLOCK>		m_csIdleEvent;
	INT16			m_nIdleEvent;

	ApMutualEx		m_MutexIdleEvent2;
	ApArray<DONT_USE_AUTOLOCK>		m_csIdleEvent2;
	INT16			m_nIdleEvent2;
	
	ApMutualEx		m_MutexIdleEvent3;
	ApArray<DONT_USE_AUTOLOCK>		m_csIdleEvent3;
	INT16			m_nIdleEvent3;
	
protected:
	// ApAdmin으로부터 Data가 Construction/Destruction 될때 넘겨 받기 위한 Callback (실제로 거의 쓰이지 않음)
	static BOOL			m_fConstructorCB(PVOID pData, PVOID pClass, INT16 nDataIndex);
	static BOOL			m_fDestructorCB(PVOID pData, PVOID pClass, INT16 nDataIndex);

public:
	// Module이 처리할 Packet Type
	ApModuleStatus	m_eModuleStatus;

	UINT32			m_ulPacketType[APMODULE_MAX_MODULEPACKET_TYPE];
	BOOL			m_bUseIdle;
	BOOL			m_bUseIdle2;
	BOOL			m_bUseIdle3;

	// Streaming에 필요한 Callback 함수들 (ApModuleStream이 사용한다.)
	StreamData*		m_listStream[APMODULE_MAX_MODULE_NUMBER];

	// 메모리 풀링 CreateModuleData의 성능 개선
	ApMemoryPool	m_csMemoryPool;

	// new 로 할당되었나?
	BOOL			m_bDynamicAllocated;

public:
	ApModule();
	virtual ~ApModule();

	BOOL		SetModuleType(INT16 nModuleType);
	INT16		GetModuleType();

	VOID		EnableIdle(BOOL bUseIdle);
	VOID		EnableIdle2(BOOL bUseIdle);
	VOID		EnableIdle3(BOOL bUseIdle);
	// RegisterModule 돼이 이전에 호출돼어져야함..

	// Module에 대한 기본 함수들
	BOOL		SetModuleName(CHAR* szName) { ZeroMemory(m_szName, sizeof(CHAR) * APMODULE_MAX_MODULE_NAME); strncpy(m_szName, szName, (APMODULE_MAX_MODULE_NAME - 1)); return TRUE; }
	ApModule*	GetModule(CHAR* szModuleName);
	CHAR*		GetModuleName();

	PVOID		GetModuleManager();

	// Module 등록
	BOOL		AddModule(PVOID pModuleManager, BOOL bDynamicAllocated = FALSE);

	// 일반 Callback 함수들
	BOOL		SetCallback(INT16 nCallbackID, ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL		EnumCallback(UINT32 nCallbackID, PVOID pData, PVOID pCustData);

	// Stream Callback 함수들
	BOOL		AddStreamCallback(INT16 nDataIndex, ApModuleStreamReadCallBack pfReadCallback, ApModuleStreamWriteCallBack pfWriteCallback, ApModule *pClass);

	// Idle 처리 함수들...
	BOOL		Idle(UINT32 ulClockCount);
	BOOL		AddIdleEvent(UINT32 ulClockCount, INT32 lCID, PVOID pClass, BOOL (*pfnProcessFunc) (INT32 lCID, PVOID pClass, UINT32 ulClockCount, PVOID pvData), PVOID pvData);
	BOOL		RemoveIdleEvent(INT32 lCID);
	pstMODULEIDLEEVENT	GetIdleEvent(UINT32 ulClockCount, INT32 *plIndex);
	BOOL		SetMaxIdleEvent(INT16 nCount);
	INT16		GetMaxIdleEvent();

	// Idle2 처리 함수들...
	BOOL		Idle2(UINT32 ulClockCount);
	BOOL		AddIdleEvent2(UINT32 ulClockCount, INT32 lCID, PVOID pClass, BOOL (*pfnProcessFunc) (INT32 lCID, PVOID pClass, UINT32 ulClockCount, PVOID pvData), PVOID pvData);
	BOOL		RemoveIdleEvent2(INT32 lCID);
	pstMODULEIDLEEVENT	GetIdleEvent2(UINT32 ulClockCount, INT32 *plIndex);
	BOOL		SetMaxIdleEvent2(INT16 nCount);
	INT16		GetMaxIdleEvent2();

	// Idle3 처리 함수들...
	BOOL		Idle3(UINT32 ulClockCount);
	BOOL		AddIdleEvent3(UINT32 ulClockCount, INT32 lCID, PVOID pClass, BOOL (*pfnProcessFunc) (INT32 lCID, PVOID pClass, UINT32 ulClockCount, PVOID pvData), PVOID pvData);
	BOOL		RemoveIdleEvent3(INT32 lCID);
	pstMODULEIDLEEVENT	GetIdleEvent3(UINT32 ulClockCount, INT32 *plIndex);
	BOOL		SetMaxIdleEvent3(INT16 nCount);
	INT16		GetMaxIdleEvent3();

	// Module Data에 관한 함수들

	// Module Data의 Size 함수들
	INT32		GetDataSize(INT16 nDataIndex = 0);
	INT32		AddDataSize(INT32 nDataSize, INT16 nDataIndex = 0);
	BOOL		SetModuleData(INT32 nSize, INT16 nDataIndex = 0);

	// Attached Data에 대한 함수들
	INT16		SetAttachedModuleData(PVOID pClass, INT16 nDataIndex, INT32 nDataSize, ApModuleDefaultCallBack pfConstructor = NULL, ApModuleDefaultCallBack pfDestructor = NULL);
	PVOID		GetAttachedModuleData(INT16 nIndex, PVOID pData);
	PVOID		GetParentModuleData(INT16 nIndex, PVOID pData);

	// Module Data를 생성 파괴
	template< class T >
	PVOID		CreateModuleData(INT16 nDataIndex)
	{
		if( nDataIndex >= APMODULE_MAX_MODULE_NUMBER || nDataIndex < 0 )
			return NULL;

		PVOID	pBuffer;
		INT32	nDataSize = GetDataSize(nDataIndex);

		if (nDataSize < 1)
			return NULL;

		// 메모리 풀링을 셋팅 하였다면 풀링을 사용한다.
		if ((-1 != m_csMemoryPool.GetTypeIndex()) && (0 == nDataIndex))
		{
			pBuffer = m_csMemoryPool.Alloc();
		}
		else
		{
			pBuffer = (PVOID) new BYTE[nDataSize];
			//ApMemoryTracker::GetInstance().AddInformation(pBuffer, GetModuleName());
		}

		if (!pBuffer)
			return NULL;

		ZeroMemory(pBuffer, nDataSize);

		new(pBuffer) T;

		EnumConstructor(nDataIndex, (CHAR*)pBuffer, NULL);

		return pBuffer;
	}
	PVOID		CreateModuleData(INT16 nDataIndex = 0, INT16 nCount = 1);
	BOOL		DestroyModuleData(PVOID pData, INT16 nDataIndex = 0);
	BOOL		EnumDestructor(INT16 nDataIndex, PVOID data, PVOID pCustData);
	BOOL		EnumConstructor(INT16 nDataIndex, PVOID data, PVOID pCustData);

	// Packet 처리에 관한 함수들
	BOOL		SetPacketType(UINT32 ulType);
	//UINT32		GetPacketType();
	BOOL		AddPreReceiveModule(ApModule *pcsModule);
	BOOL		EnumPreRecieveModule(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg);

	BOOL		SendPacket(PVOID pvPacket, INT16 nLength, UINT32 ulNID = 0, PACKET_PRIORITY ePriority = PACKET_PRIORITY_4, INT16 nFlag = APMODULE_SENDPACKET_PLAYER);

	BOOL		SetSelfCID(INT32 lCID);

	UINT32		GetClockCount();
	UINT32		GetPrevClockCount();

	BOOL		InitMemoryPool(INT32 lDataSize, INT32 lCount, CHAR*	szName = NULL);

	BOOL		ExistCallback(INT16 nCallbackID);

public:

	virtual BOOL	OnAddModule()	{ return TRUE; }
	virtual BOOL	OnInit()		{ return TRUE; }

	virtual BOOL	OnIdle(UINT32 ulClockCount) 
	{ 
		//@{ 2006/09/21 burumal
		UNREFERENCED_PARAMETER(ulClockCount);		
		//@}
		return TRUE; 
	}

	virtual BOOL	OnIdle2(UINT32 ulClockCount) 
	{ 
		//@{ 2006/09/21 burumal
		UNREFERENCED_PARAMETER(ulClockCount);		
		//@}
		return TRUE; 
	}

	virtual BOOL	OnIdle3(UINT32 ulClockCount) 
	{ 
		//@{ 2006/09/21 burumal
		UNREFERENCED_PARAMETER(ulClockCount);		
		//@}
		return TRUE; 
	}

	virtual BOOL	OnPreDestroy();
	virtual BOOL	OnDestroy();

	virtual BOOL	OnPreReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)	
	{ 
		//@{ 2006/09/21 burumal
		UNREFERENCED_PARAMETER(ulType);
		UNREFERENCED_PARAMETER(pvPacket);
		UNREFERENCED_PARAMETER(nSize);
		UNREFERENCED_PARAMETER(ulNID);
		UNREFERENCED_PARAMETER(pstCheckArg);
		//@}
		return TRUE; 
	}

	virtual BOOL	OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)	
	{ 
		//@{ 2006/09/21 burumal
		UNREFERENCED_PARAMETER(ulType);
		UNREFERENCED_PARAMETER(pvPacket);
		UNREFERENCED_PARAMETER(nSize);
		UNREFERENCED_PARAMETER(ulNID);
		UNREFERENCED_PARAMETER(pstCheckArg);
		//@}
		return TRUE; 
	}

	virtual BOOL	OnValid(CHAR* szData, INT16 nSize) 
	{ 
		//@{ 2006/09/21 burumal
		UNREFERENCED_PARAMETER(szData);
		UNREFERENCED_PARAMETER(nSize);
		//@}

		return TRUE; 
	}

	virtual void	Report(FILE *fp);
	virtual void	Report_Head(FILE *fp);

public:
	BOOL		Receive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg);

	void		ReleaseCallBackDataList(CallBackData* node);
	void		ReleaseStreamDataList(StreamData* node);
};

#endif //__APMODULE_H__
