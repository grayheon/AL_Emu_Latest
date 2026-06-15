#ifndef __CLASS_TEXT_FILTER_H__
#define __CLASS_TEXT_FILTER_H__

#include "ContainerUtil.h"
#include "AcDefine.h"

#include <string>

#include <map>


struct stTextFilterEntry
{
	char											m_strFilter[ 256 ];
	int												m_nTextLegnth;

	stTextFilterEntry( void )
	{
		memset( m_strFilter, 0, sizeof( char ) * 256 );
		m_nTextLegnth = 0;
	}
};

class CTextFilter
{
private :
	typedef	std::map< std::string , stTextFilterEntry >				MapTextFilterEntry;
	typedef std::map< std::string , stTextFilterEntry >::iterator	MapTextFilterEntryIter;

public :
	CTextFilter( VOID );
	virtual ~CTextFilter( VOID );

public :
	BOOL			OnLoadFilterFile				( char* pFileName, BOOL bIsEncrypt );

	BOOL			OnAddFilter						( char* pFilterText, BOOL bIsCustom, BOOL bIsSave = FALSE );
	BOOL			OnRemoveFilter					( char* pFilterText, BOOL bIsCustom );
	BOOL			OnClearFilter					( BOOL bIsCustom );

	BOOL			OnSaveFilterFileCustom			( char* pFileName, char* pFilter );
	BOOL			OnFiltering						( char* pString );

private :
	BOOL			_LoadCustomFilterFile			( char* pFileName );

	MapTextFilterEntry								m_mapFilter;
	MapTextFilterEntry								m_mapFilterCustom;

	char											m_strBlindText[ 6 ];



};




#endif