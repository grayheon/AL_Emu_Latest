#include "AuIniPackingManager.h"
#include "AuPackingManager.h"

BOOL	AuINIPackingManager::FromFile( int lIndex, BOOL bDecryption)
{
	FILE* pFile;
	int lMaxCharacter = AUINIMANAGER_MAX_NAME+AUINIMANAGER_MAX_KEYVALUE+1;	
	char* szDummyPtr;
	int i = 0;
	bool bSuccessKey = false, bSuccessValue = false;
	int	lFileSectionNum		= 0					;		//Section이 몇개인가. 
	int	lFileKeyNum[AUINIMANAGER_MAX_SECTIONNUM];		//각 Section당 Key가 몇개씩인가 
	int lStartPos = 0;
	int lEndPos = 0;
	__int64 lCurPos = 0;

	ZeroMemory(lFileKeyNum, sizeof(lFileKeyNum));

	int nDummyLen;	
	char* pSrcReadData = NULL;
	int nTotalDataSize = 0;
	
	if( m_pFileBuffer )
	{
		delete [] m_pFileBuffer;
		m_pFileBuffer = NULL;
	}

	//File 열기 
	if ( (pFile = fopen( m_strPathName.c_str() , "rb" )) == NULL )	//File Open
	{
		CHAR	szPath[ MAX_PATH ];
		sprintf_s( szPath, MAX_PATH, "%s", m_strPathName.c_str());

		AuPackingManager* pPackingManager = AuPackingManager::GetSingletonPtr();
		if( !pPackingManager ) return 0;

		ApdFile pdFile;
		if( !pPackingManager->OpenFile( szPath, &pdFile ) )
		{
			return 0;
		}
		int iBufferSize = pPackingManager->GetFileSize( &pdFile );
		if( iBufferSize <= 0 )
		{
			pPackingManager->CloseFile( &pdFile );
			return 0;
		}
	
		m_pFileBuffer = new char[ iBufferSize ];
		int nReadSize = pPackingManager->ReadFile( m_pFileBuffer, iBufferSize, &pdFile );
		pPackingManager->CloseFile( &pdFile );

		if( nReadSize != iBufferSize )
		{
			ZeroMemory( m_pFileBuffer, iBufferSize );
			return 0;
		}

		if ( GetType() & AUINIMANAGER_TYPE_PART_INDEX )
		{
			if ( !ReadPartIndices( m_pFileBuffer ) || lIndex >= m_lPartNum )
			{
				return FALSE;
			}

			lStartPos = m_plPartIndices[lIndex];

			if ( lIndex + 1 < m_lPartNum)
				lEndPos = m_plPartIndices[lIndex + 1];
		}
		// Parn님 작업
		if( lEndPos == 0 )
		{
			lEndPos = iBufferSize;
		}

		if ( lEndPos <= lStartPos )
		{
			ZeroMemory( m_pFileBuffer, iBufferSize );
			return FALSE;
		}

		nTotalDataSize = (lEndPos - lStartPos);
		pSrcReadData = new char[nTotalDataSize];
		if ( !pSrcReadData )
		{
			ZeroMemory( m_pFileBuffer, iBufferSize );
			return FALSE;
		}
		
		memcpy_s( pSrcReadData, nTotalDataSize, m_pFileBuffer+lStartPos, nTotalDataSize);
		//ZeroMemory( m_pFileBuffer, iBufferSize );
	}
	else
	{
		// Parn님 작업 Part Index type일 경우는 앞에서 index를 읽어서 그 부분으로 넘어가서 읽는다.
		if ( GetType() & AUINIMANAGER_TYPE_PART_INDEX )
		{
			if ( !ReadPartIndices( pFile ) || lIndex >= m_lPartNum )
			{
				fclose( pFile );
				return FALSE;
			}

			lStartPos = m_plPartIndices[lIndex];

			if ( fseek( pFile, lStartPos, SEEK_SET) )
			{
				fclose( pFile );
				return FALSE;
			}

			if ( lIndex + 1 < m_lPartNum)
				lEndPos = m_plPartIndices[lIndex + 1];
		}
			// Parn님 작업

		if ( lEndPos == 0 )
		{
			fseek(pFile, 0, SEEK_END);
			lEndPos = ftell(pFile);
			fseek(pFile, lStartPos, SEEK_SET);
		}

		if ( lEndPos <= lStartPos )
		{
			fclose(pFile);
			return FALSE;
		}

		nTotalDataSize = (lEndPos - lStartPos);
		pSrcReadData = new char[nTotalDataSize];
		if ( !pSrcReadData )
		{
			fclose(pFile);
			return FALSE;
		}

		fread(pSrcReadData, nTotalDataSize, 1, pFile);
		fclose(pFile);
	}

	static DebugValueString	_stValue( "no file" , "Ini Access File = %s" );
	_stValue	= m_strPathName.c_str();

	ParseMemory( pSrcReadData, nTotalDataSize, bDecryption );

// 	ASSERT(pSrcReadData);
// 	delete [] pSrcReadData;

	if( lSectionNum == 0 )	return FALSE;

	return FALSE;
}

BOOL	AuINIPackingManager::FromMemory( BOOL bDecryption)
{
	int	 lMaxCharacter = AUINIMANAGER_MAX_NAME + AUINIMANAGER_MAX_KEYVALUE + 1;

	char szDummy[AUINIMANAGER_MAX_NAME + AUINIMANAGER_MAX_KEYVALUE + 1];

	int	lFileSectionNum								= 0		;		//Section이 몇개인가. 

	int	lFileKeyNum[AUINIMANAGER_MAX_SECTIONNUM]	= {0,}	;		//각 Section당 Key가 몇개씩인가 

	int		i = 0;
	int		lStartPos = 0;
	int		lEndPos	= 0;
	__int64	lCurPos = 0;

	size_t iOffsetBytes = 0;
	size_t iOffset		= 0;

	//File 열기
	FILE*		file = NULL;
	char*		pBuffer;
	size_t		iFileSize;
	bool		bFileReadResult;

	bFileReadResult = true;
	pBuffer = NULL;
	iFileSize = 0;

	file = fopen( m_strPathName.c_str(), "rb" );
	if( !file )
	{
		CHAR	szPath[ MAX_PATH ];
		sprintf_s( szPath, MAX_PATH, "%s", m_strPathName.c_str());

		AuPackingManager* pPackingManager = AuPackingManager::GetSingletonPtr();
		if( !pPackingManager ) return 0;

		ApdFile pdFile;
		if( !pPackingManager->OpenFile( szPath, &pdFile ) )
		{
			return 0;
		}

		iFileSize = pPackingManager->GetFileSize( &pdFile );
		if( iFileSize <= 0 )
		{
			pPackingManager->CloseFile( &pdFile );
			return 0;
		}

		pBuffer = new char[ iFileSize ];
		int nReadSize = pPackingManager->ReadFile( pBuffer, iFileSize, &pdFile );
		pPackingManager->CloseFile( &pdFile );

		if( nReadSize != iFileSize )
		{
			delete [] pBuffer;
			pBuffer = NULL;
			return 0;
		}
	}
	else
	{
		fseek( file, 0, SEEK_END );
		iFileSize = ftell( file );
		fseek( file, 0, SEEK_SET );

		if( iFileSize )
		{
			pBuffer = new char[iFileSize + 1];

			//@{ 2006/11/22 burumal
			memset(pBuffer, 0, iFileSize + 1);
			//@}

			if( fread( pBuffer, 1, iFileSize, file ) != iFileSize )
			{
				bFileReadResult = false;
			}
		}
		else
		{
			bFileReadResult = false;
		}

		fclose( file );

		if ( bFileReadResult == false )
			return FALSE;
	}

	ParseMemory( pBuffer, iFileSize, bDecryption );

	if( lSectionNum == 0 )
		return FALSE;

	return TRUE;
}