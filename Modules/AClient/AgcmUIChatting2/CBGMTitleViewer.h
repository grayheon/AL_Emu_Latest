#ifndef __CLASS_BGM_TITLE_VIEWER_H__
#define __CLASS_BGM_TITLE_VIEWER_H__




#include "ContainerUtil.h"
#include "AgcdUIManager2.h"
#include <string>




struct stBGMTitleEntry
{
	std::string											m_strTitle;
	std::string											m_strFileName;

	stBGMTitleEntry( void )
	{
		m_strTitle = "";
		m_strFileName = "";
	}
};

class CBGMTitleViewer
{
private :
	ContainerMap< std::string, stBGMTitleEntry >		m_mapBGM;
	ContainerMap< std::string, stBGMTitleEntry >		m_mapLoginBGM;
	int													m_nCurrBGM;

	AgcdUIUserData*										m_pUserData;
	std::string											m_strCurrBGMFileName;

public :
	CBGMTitleViewer( void );
	virtual ~CBGMTitleViewer( void );

public :
	BOOL			ReadXMLData							( CONST string& strFileName );

public :
	BOOL			OnReadyBGMTitle						( void );
	BOOL			OnUpdateBGMTitle					( char* pFileName );

public :
	char*			GetBGMTitle							( char* pFileName );
	char*			GetCurrentBGMTitle					( void );

private :
	void			_AddBGMTitle						( const char* pFileName, const char* pTitleName );
	void			_AddLoginBGMTitle					( const char* pFileName, const char* pTitleName );

public :
	static BOOL		CallBack_DisplayBGMTitle			( void* pClass, void* pData, AgcdUIDataType eType, int nID, char* pDisplay, int* pValue, AgcdUIControl* pcdSourceControl );
	static BOOL		CallBack_UpdateBGMTitle				( void* pData, void* pClass, void* pCustData );
};



#endif