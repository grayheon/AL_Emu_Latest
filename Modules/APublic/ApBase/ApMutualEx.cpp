/******************************************************************************
Module:	Optex.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/

#include "ApDefine.h"
#include "ApMutualEx.h"
#include "ApLockManager.h"
#include <assert.h>

#pragma message ( "Service Area Start" ) 
#ifdef _AREA_CHINA_
	#pragma message ( "China" ) 
	const ApServiceArea	g_eServiceArea	= AP_SERVICE_AREA_CHINA;
#endif

#ifdef _AREA_WESTERN_
	#pragma message ( "Western" ) 
	const ApServiceArea	g_eServiceArea	= AP_SERVICE_AREA_WESTERN;
#endif
		
#ifdef _AREA_JAPAN_
	#pragma message ( "Japan" ) 
	const ApServiceArea	g_eServiceArea	= AP_SERVICE_AREA_JAPAN;
#endif

#ifdef _AREA_KOREA_
	#pragma message ( "Korea" ) 
	const ApServiceArea	g_eServiceArea	= AP_SERVICE_AREA_KOREA;
#endif

#ifdef _AREA_GLOBAL_
	#pragma message ( "Global" ) 
	const ApServiceArea	g_eServiceArea	= AP_SERVICE_AREA_GLOBAL;
	ApChannelingService g_eChannelingService = AP_SERVICE_CHANNELING_ORIGINAL_PATH;
#endif

#ifdef _AREA_TAIWAN_
	#pragma message ( "Taiwan" ) 
	const ApServiceArea	g_eServiceArea	= AP_SERVICE_AREA_TAIWAN;
#endif

#pragma message ( "Service Area End" ) 

INT32 g_lLocalServer = 0;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ApMutualEx::ApMutualEx()
{
	m_bInit	= TRUE;
	m_bRemoveLock	= FALSE;
	m_pvParent	= NULL;

	m_bNotUseLockManager	= FALSE;

	if(FALSE == InitializeCriticalSectionAndSpinCount(&m_csCriticalSection, 4000))
	{
		m_bInit = FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////////
VOID ApMutualEx::Init(PVOID pvParent)
{
	if (m_bInit)
		return;

	m_bInit	= TRUE;
	m_pvParent	= pvParent;
	m_bRemoveLock = FALSE;

	m_bNotUseLockManager	= FALSE;

	if(FALSE == InitializeCriticalSectionAndSpinCount(&m_csCriticalSection, 4000))
	{
		m_bInit = FALSE;
	}
}

VOID ApMutualEx::Destroy()
{
	if (!m_bInit)
		return;

	// CriticalSection인 경우 EnterCriticalSection()을 위해 기다리는 넘들은 그 타겟 객체가 없어져도 인식을 하지 못한다.
	// 없어졌는지도 모르고 계속 기다리는 CriticalSection 땜시 아래같은 처리를 해줘야 한다.
	// 안그럼 데드락 걸린다.

	// 1. Lock()하고 m_bRemoveLock을 세팅한다. (m_bRemoveLock이 세팅되어 있으면 WLock()에서 무조건 FALSE가 리턴된다.)
	// 2. UnLock() 한다. (이유는 이 객체를 락하기 위해 기다리는 넘들에게 모두 기회를 m_bRemoveLock를 보고 바로 FALSE를 리턴해 버릴 수 있는 기회를 주기 위함이다.)
	// 3. Lock() 한다. (위에 기다리던 놈들이 모두 FALSE를 리턴할때까지 기다리기 위함이다.)
	// 4. 이제 안전하다. 없앤다.

	if (!m_bRemoveLock)
	{
		if (m_bNotUseLockManager)
			EnterCriticalSection(&m_csCriticalSection);
		else
			ApLockManager::Instance()->InsertLock( this );

		m_bRemoveLock = TRUE;
	}

	if (m_bNotUseLockManager)
		LeaveCriticalSection(&m_csCriticalSection);
	else
		ApLockManager::Instance()->RemoveUnlock( this );

	if (m_bNotUseLockManager)
		EnterCriticalSection(&m_csCriticalSection);
	else
		ApLockManager::Instance()->InsertLock( this );

	if (m_bNotUseLockManager)
		LeaveCriticalSection(&m_csCriticalSection);
	else
		ApLockManager::Instance()->RemoveUnlock( this );

	if (!m_bNotUseLockManager)
		ApLockManager::Instance()->SafeRemoveUnlock( this );

	DeleteCriticalSection(&m_csCriticalSection);

	m_bInit = FALSE;	
}


ApMutualEx::~ApMutualEx() 
{
	if(TRUE == m_bInit)
		Destroy();
}

BOOL ApMutualEx::RLock()
{
	return WLock();
}

BOOL ApMutualEx::WLock() {
	if (!m_bInit || m_bRemoveLock)
		return FALSE;

	if (m_bNotUseLockManager)
		EnterCriticalSection(&m_csCriticalSection);
	else
	{
		if(!ApLockManager::Instance()->InsertLock( this ))
			return FALSE;
	}

	if (m_bRemoveLock)
	{
		Release(TRUE);
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////


BOOL ApMutualEx::RemoveLock()
{
	if (!WLock())
		return FALSE;
	
	m_bRemoveLock = TRUE;

	return TRUE;
}

VOID ApMutualEx::ResetRemoveLock()
{
	m_bRemoveLock = FALSE;
}

///////////////////////////////////////////////////////////////////////////////


BOOL ApMutualEx::Release(BOOL bForce) {
	if (!m_bInit || (m_bRemoveLock && !bForce)) return FALSE;

	if (m_bNotUseLockManager)
		LeaveCriticalSection(&m_csCriticalSection);
	else
		ApLockManager::Instance()->RemoveUnlock( this );

//	LeaveCriticalSection(&m_csCriticalSection);

// 마고자 (04-05-23 오전 6:09:51) : 98에서는 이 정보가 믿을 수 없어요..
// CRITICAL_SECTION 이 윈98에서는 ulong 이라 , 아래와 같은 정보를 사용할수없어요~

//	if (m_csCriticalSection.RecursionCount < 0 && m_csCriticalSection.LockCount < 0)
//	{
//		char buffer[256] = {0,};
//		sprintf(buffer, "\nRecursionCount : %d, LockCount : %d", m_csCriticalSection.RecursionCount, m_csCriticalSection.LockCount);
//		OutputDebugString(buffer);
//		DebugBreak();
//	}

	return TRUE;
}

BOOL ApMutualEx::SafeRelease()
{
	if (!m_bInit)	return FALSE;

	if (!m_bNotUseLockManager)
		ApLockManager::Instance()->SafeRemoveUnlock( this );

	return TRUE;
}

BOOL ApMutualEx::SetNotUseLockManager()
{
	m_bNotUseLockManager	= TRUE;

	return TRUE;
}

//////////////////////////////// End of File //////////////////////////////////
