#include "AuExcelTxtLib.h"
#include "ApModuleStream.h"
#include "CTextFilter.h"
#include "AuRandomNumber.h"



CTextFilter::CTextFilter( void )
{
	m_strBlindText[ 0 ] = '@';
	m_strBlindText[ 1 ] = '#';
	m_strBlindText[ 2 ] = '$';
	m_strBlindText[ 3 ] = '*';
	m_strBlindText[ 4 ] = '&';
	m_strBlindText[ 5 ] = '\0';
}

CTextFilter::~CTextFilter( void )
{
}

BOOL CTextFilter::OnLoadFilterFile( char* pFileName, BOOL bIsEncrypt )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return FALSE;
	if( !bIsEncrypt ) return _LoadCustomFilterFile( pFileName );

	AuExcelTxtLib ExcelFile;
	if( !ExcelFile.OpenExcelFile( pFileName, TRUE, bIsEncrypt ) )
	{
		ExcelFile.CloseFile();
		return FALSE;
	}

	int nRowCount = ExcelFile.GetRow();
	for( int nCount = 0 ; nCount < nRowCount ; nCount++ )
	{
		char* pFilterText = ExcelFile.GetData( 0, nCount );
		if( pFilterText && strlen( pFilterText ) > 0 )
		{
			_strlwr( pFilterText );
			OnAddFilter( pFilterText, FALSE );
		}
	}

	ExcelFile.CloseFile();
	return TRUE;
}

BOOL CTextFilter::OnAddFilter( char* pFilterText, BOOL bIsCustom, BOOL bIsSave )
{
	if( !pFilterText || strlen( pFilterText ) <= 0 ) 
		return FALSE;

	stTextFilterEntry*	pEntry	=	NULL;
	if( bIsCustom )
	{
		MapTextFilterEntryIter	Iter	=	m_mapFilterCustom.find( pFilterText );
		if( Iter != m_mapFilterCustom.end() )
			pEntry	=	&Iter->second;
	}
	else
	{
		MapTextFilterEntryIter	Iter	=	m_mapFilter.find( pFilterText );
		if( Iter != m_mapFilter.end() )
			pEntry	=	&Iter->second;
	}

	if( !pEntry )
	{
		stTextFilterEntry NewFilter;

		strcpy_s( NewFilter.m_strFilter, sizeof( char ) * 256, pFilterText );
		NewFilter.m_nTextLegnth = strlen( NewFilter.m_strFilter );

		if( bIsCustom )
			m_mapFilterCustom.insert( make_pair( pFilterText , NewFilter ) );
		else
			m_mapFilter.insert( make_pair( pFilterText , NewFilter ) );

		if( bIsCustom && bIsSave )
		{
			OnSaveFilterFileCustom( "INI\\CustomTextFilter.txt", pFilterText );
		}
	}

	return TRUE;
}

BOOL CTextFilter::OnRemoveFilter( char* pFilterText, BOOL bIsCustom )
{
	if( !pFilterText || strlen( pFilterText ) <= 0 ) return FALSE;

	if( bIsCustom )
	{
		MapTextFilterEntryIter	Iter	=	m_mapFilterCustom.find( pFilterText );
		if( Iter != m_mapFilterCustom.end() )
			m_mapFilterCustom.erase( Iter );

	}
	else
	{
		MapTextFilterEntryIter	Iter	=	m_mapFilter.find( pFilterText );
		if( Iter != m_mapFilterCustom.end() )
			m_mapFilter.erase( Iter );
	}

	return TRUE;
}

BOOL CTextFilter::OnClearFilter( BOOL bIsCustom )
{
	if( bIsCustom )
		m_mapFilterCustom.clear();
	else
		m_mapFilter.clear();

	return TRUE;
}

BOOL CTextFilter::OnSaveFilterFileCustom( char* pFileName, char* pFilter )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return FALSE;
	if( !pFilter || strlen( pFilter ) <= 0 ) return FALSE;

	FILE* pFile = fopen( pFileName, "a+" );
	if( !pFile )
	{
		// 파일이 없으면 새로 만든다.
		pFile = fopen( pFileName, "wt" );
		if( !pFile ) return FALSE;

		fclose( pFile );
		pFile = fopen( pFileName, "a+" );
		if( !pFile ) return FALSE;
	}
	
	fseek( pFile, 0, SEEK_END );
	fwrite( pFilter, strlen( pFilter ), 1, pFile );
	fwrite( "\n", 1, 1, pFile );
	fclose( pFile );

	return TRUE;
}

BOOL CTextFilter::OnFiltering( char* pString )
{
	if( !pString || strlen( pString ) <= 0 ) return FALSE;

	// 첫글자가 '/' 로 시작하는 명령문인 경우 그 뒷글자부터 검사한다.
	char strBuffer[ 256 ] = { 0, };
	bool bIsInstruction = false;
	int i = 0;

	MTRand	mtRnd;

	if(pString[ 0 ] == '/')
		bIsInstruction = true;

	strcpy_s( strBuffer, sizeof( char ) * 256, bIsInstruction ? pString + 1 : pString );

	// 귓속말이면 케릭터 명에 들어간 금칙어는 허용한다.
	if(bIsInstruction)
	{
		char strInstruction[32] = {0, };
		char strParam1[32] = {0, };
		char strParam2[256] = {0, };

		if(strBuffer[0] == 'w' || strBuffer[0] == 'W')
		{
			char strTemp[ 256 ] = { 0, };
			int	nChecker(0);
			strcpy_s( strTemp, sizeof( char ) * 256, strBuffer );

			while(nChecker<2)
			{
				if(strTemp[i++] == ' ')
					++nChecker;
			}

			memset(strBuffer, '\0', sizeof( char ) * 256);
			strcpy_s(strBuffer, sizeof( char ) * 256, strTemp + i);
		}
		
		
	}

	_strlwr( strBuffer );
	int nBufferLength = strlen( strBuffer );

	MapTextFilterEntryIter	Iter	=	m_mapFilter.begin();
	for( ; Iter != m_mapFilter.end() ; ++Iter )
	{
		stTextFilterEntry*	pEntry	=	&Iter->second;
		if( pEntry && strlen(pEntry->m_strFilter) )
		{
			for( int nBufferCount = 0 ; nBufferCount < nBufferLength ; ++nBufferCount )
			{
				if( !strncmp( pEntry->m_strFilter, &strBuffer[ nBufferCount ], pEntry->m_nTextLegnth ) )
				{
					for( int nBlindCount = 0 ; nBlindCount < pEntry->m_nTextLegnth ; nBlindCount++ )
					{
						pString[ nBufferCount + nBlindCount + i + (bIsInstruction?1:0) ] = m_strBlindText[ mtRnd.randInt(4) ];
					}

					nBufferCount += pEntry->m_nTextLegnth;
				}
				else if( strBuffer[nBufferCount] < 0 ) // 2바이트 문자는 2칸씩 이동
					++nBufferCount;
			}
		}
	}

	Iter	=	m_mapFilterCustom.begin();
	for( ; Iter != m_mapFilterCustom.end() ; ++Iter )
	{
		stTextFilterEntry* pFilter = &Iter->second;
		if( pFilter && strlen( pFilter->m_strFilter ) )
		{
			for( int nBufferCount = 0 ; nBufferCount < nBufferLength ; ++nBufferCount )
			{
				if( !strncmp( pFilter->m_strFilter, &strBuffer[ nBufferCount ], pFilter->m_nTextLegnth ) )
				{
					for( int nBlindCount = 0 ; nBlindCount < pFilter->m_nTextLegnth ; nBlindCount++ )
					{
						pString[ nBufferCount + nBlindCount + i + (bIsInstruction?1:0) ] = m_strBlindText[ mtRnd.randInt(4) ];
					}

					nBufferCount += pFilter->m_nTextLegnth;
				}
				else if( strBuffer[nBufferCount] < 0 ) // 2바이트 문자는 2칸씩 이동
					++nBufferCount;
			}
		}
	}

	return TRUE;
}

BOOL CTextFilter::_LoadCustomFilterFile( char* pFileName )
{
	if( !pFileName || strlen( pFileName ) <= 0 ) return FALSE;

	FILE* pFile = fopen( pFileName, "rt" );
	if( !pFile ) return FALSE;
	
	fseek( pFile, 0, SEEK_END );
	int nFileSize = ftell( pFile );
	if( nFileSize <= 0 )
	{
		fclose( pFile );
		return FALSE;
	}

	char* pBuffer = new char[ nFileSize ];
	memset( pBuffer, 0, sizeof( char ) * nFileSize );

	fseek( pFile, 0, SEEK_SET );
	fread( pBuffer, nFileSize, 1, pFile );
	fclose( pFile );

	int nIndex = 0;
	while( pBuffer[ nIndex ] != '\0' )
	{
		char strBuffer[ 256 ] = { 0, };
		int nCopyCount = 0;

		while( pBuffer[ nIndex ] != '\n' && pBuffer[ nIndex ] != '\0' )
		{
			strBuffer[ nCopyCount ] = pBuffer[ nIndex ];

			nCopyCount++;
			nIndex++;
		}

		OnAddFilter( strBuffer, TRUE );
		if( pBuffer[ nIndex ] != '\0' )
		{
			nIndex++;
		}
	}

	delete[] pBuffer;
	pBuffer = NULL;

	return TRUE;
}
