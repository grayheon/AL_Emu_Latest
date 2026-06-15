#include "MyException.h"
#include "MyEngine.h"

#include "AcuDeviceCheck.h"
#include "CSLog.h"
#include "AgcmRegistryManager.h"

const DWORD CMyException::s_dwOption = /*GSTSO_PARAMS |*/GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
MyEngine* CMyException::m_pEngine = NULL;
LPTOP_LEVEL_EXCEPTION_FILTER CMyException::m_pExceptionFilter = NULL;

CMyException::CMyException( MyEngine* pEngine )
{
	m_pEngine			= pEngine;
	m_pExceptionFilter	= SetUnhandledExceptionFilter( ExceptionHandler );
}

CMyException::~CMyException()
{
	if( m_pExceptionFilter )
	{
		SetUnhandledExceptionFilter( m_pExceptionFilter );
		m_pExceptionFilter = NULL;
	}
}

// 디버그 정보추가
extern UINT32 g_uDebugLastRemoveCharacterTID;		// AgcmCharacter.cpp
extern UINT32 g_uRemoveUpdateInfoFromClump2Error;	// AgcmRender.cpp
extern int	g_DebugHashKey;							// AgcmRender.cpp

extern DWORD g_DebugHashTableAtomic[R_HASH_SIZE];
extern DWORD g_DebugHashTableTexaddr[R_HASH_SIZE];
extern DWORD g_DebugHashTableSibling[R_HASH_SIZE];
extern DWORD g_DebugHashTableNext[R_HASH_SIZE];
LONG __stdcall CMyException::ExceptionHandler( EXCEPTION_POINTERS* pExPtrs )
{
	// 우선 모드 돌려놓기
	((MyEngine*)m_pEngine)->m_pMainWindow->ReturnScreenMode();

	// some network-related clean-ups
	INT16	nSize;
	PVOID	pvPacket = m_pEngine->MakeSPClientExit(&nSize);
	if ( pvPacket )
	{
		m_pEngine->SendPacket(pvPacket, nSize);
		m_pEngine->FreeSystemPacket(pvPacket);
	}

	m_pEngine->m_SocketManager.DisconnectAll();

	// additional log for graphics card
	char buf[65536];
	sprintf_s(buf, 65536, "Graphics card: %s\r\nDriver version: %d.%d.%d.%d\r\nVendor ID: %d\r\nDevice ID: %d\r\n", 
					AcuDeviceCheck::m_strDeviceName,
					AcuDeviceCheck::m_iProduct,
					AcuDeviceCheck::m_iVersion,
					AcuDeviceCheck::m_iSubVersion,
					AcuDeviceCheck::m_iBuild,
					AcuDeviceCheck::m_iVendorID,
					AcuDeviceCheck::m_iDeviceID);

	//@{ Jaewon 20050211
	// Self character infos
	char buf2[512];
	buf2[0] = '\0';
	sprintf_s(buf2, 512, "\r\nPlayer character:\r\n" );
	strcat_s(buf, 65536, buf2);
	if(g_pcsAgcmCharacter)
	{
		__try
		{
			AgpdCharacter *pAgpdCharacter = g_pcsAgcmCharacter->GetSelfCharacter();
			if( pAgpdCharacter )
			{
				sprintf_s(buf2, 512, 	"m_szID = %s\r\n"
								"m_stPos = (%f,%f,%f)\r\n"
								"m_unActionStatus = %d\r\n"
								"m_bIsTrasform = %s\r\n"					,
								pAgpdCharacter->m_szID						,
								pAgpdCharacter->m_stPos.x					,
								pAgpdCharacter->m_stPos.y					,
								pAgpdCharacter->m_stPos.z					,
								pAgpdCharacter->m_unActionStatus			,
								pAgpdCharacter->m_bIsTrasform?"TRUE":"FALSE");

			}
			else
			{
				sprintf_s( buf2, 512, "No Self Character Info\r\n" );
			}
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			sprintf_s(buf2, 512, "\r\nException encountered during play character info logging.\r\n");
		}

		buf2[511] = '\0';

		strncat_s(buf, 65536, buf2, sizeof(buf) - strlen(buf) - 1);

		sprintf_s( buf2, 511, "LastRemoveCharacterTID =%d\r\n", g_uDebugLastRemoveCharacterTID );
		strncat_s( buf, 65536, buf2, sizeof(buf) - strlen(buf) - 1);
		sprintf_s( buf2, 511, "RemoveUpdateInfoFromClump2Error = %d\r\n", g_uRemoveUpdateInfoFromClump2Error );
		strncat_s( buf, 65536, buf2, sizeof(buf) - strlen(buf) - 1 );
		sprintf_s( buf2, 511, "Last referenced hash key = %d\r\n", g_DebugHashKey );
		strncat_s( buf, 65536, buf2, sizeof(buf) - strlen(buf) - 1 );
		for(int i=0 ; i<7 ; ++i)
		{
			sprintf_s( buf2, 511, "%d : atomic = 0x%x, textaddr = 0x%x, sibling = 0x%x, next = 0x%x\r\n", i, g_DebugHashTableAtomic[i], g_DebugHashTableTexaddr[i], g_DebugHashTableSibling[i], g_DebugHashTableNext[i]);
			strncat_s( buf, 65536, buf2, sizeof(buf) - strlen(buf) - 1 );
		}		

		DebugValueManager* pManager = DebugValueManager::GetInstance();

		for(unsigned int i = 0 ; i < pManager->vecValue.size(); ++i)
		{
			pManager->vecValue[i]->LogString(buf2);
			strcat_s( buf2, 511, "\r\n" ); // 개행 문자 추가.
			strncat_s( buf, 65536, buf2, sizeof(buf) - strlen(buf) - 1 );
		}

		CSLogManager::Print( buf );		// 추가정보.. 콜스텍 정보 있으면 추가.
	}

	DWORD	dwClientVersion = 0;

	/*
	HKEY	hRegKey;
#ifdef _AREA_GLOBAL_
	if( !RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGIKEY_ARCHLORD, 0, KEY_READ, &hRegKey ) )
#else
	if( !RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\ArchLord", 0, KEY_READ, &hRegKey ) )
#endif
	{
		
		DWORD iDataLen = sizeof( dwClientVersion );
		DWORD iType;
		RegQueryValueEx( hRegKey, "Version", 0, &iType, (unsigned char*)&dwClientVersion, &iDataLen );
	}
	*/

	dwClientVersion = AgcmRegistryManager::GetProperty<DWORD>( REGIKEY_VERSION );

	sprintf_s( buf2, 512, "\r\nClient Version: %d",dwClientVersion );
	strcat_s( buf, 65536, buf2 );

	// 서비스 지역 정보 추가.
	sprintf_s( buf2, 512, "\r\nService Area: %d\r\n", g_eServiceArea );
	strcat_s( buf, 65536, buf2 );

	return RecordExceptionInfo( pExPtrs, "AlefClient", buf, &AddExceptionHandler );
}

extern void InitSymEng();
extern void CleanupSymEng();
const char* CMyException::AddExceptionHandler( EXCEPTION_POINTERS* pExPtrs )
{
	// additional stack trace, but this needs the release of pdb
	static char buf[10240];
	const UINT32 limit = sizeof(buf)/sizeof(buf[0])-1;
	buf[limit] = '\0';
	strcpy_s(buf, 10240, "Debug log:\r\n");
	strncat_s(buf, 10240, MD_GetErrorMessage(), limit - strlen(buf));
	strncat_s(buf, 10240, "\r\n", limit-strlen(buf));

	char tmp[1025];
	tmp[1024] = '\0';

	strncat_s(buf, 10240, "\r\n", limit - strlen(buf));
	sprintf_s(tmp, 1025, "Crash Address:\r\n");
	strncat_s(buf, 10240, tmp, limit - strlen(buf));
	sprintf_s(tmp, 1025, "%08x\r\n\r\n" , pExPtrs->ExceptionRecord->ExceptionAddress );
	strncat_s(buf, 10240, tmp, limit - strlen(buf));

	sprintf_s(tmp, 1025, "Stack trace:\r\n");
	strncat_s(buf, 10240, tmp, limit - strlen(buf));

	InitSymEng();

	const char *szBuff = GetFirstStackTraceString( s_dwOption, pExPtrs );
	do
	{
		_snprintf_s( tmp, 1025, sizeof(tmp)/sizeof(tmp[0])-1, "%s\r\n", szBuff );
		strncat_s( buf, 10240, tmp, limit-strlen(buf) );
		szBuff = GetNextStackTraceString( s_dwOption, pExPtrs );
	}
	while(szBuff);
	
	CleanupSymEng();

	return buf;
}

