#pragma once

#include "AuIniManager.h"

//-----------------------------------------------------------------------
//

class	AuINIPackingManager : public AuIniManagerA
{
private:
	char*	m_pFileBuffer;

public:
	AuINIPackingManager()	{ m_pFileBuffer = NULL; }
	~AuINIPackingManager()  
	{  
		if( m_pFileBuffer )
		{	
			delete [] m_pFileBuffer;
			m_pFileBuffer = NULL; 
		} 
	}

	BOOL	FromFile				( int lIndex, BOOL bDecryption);
	BOOL	FromMemory				( BOOL bDecryption);
};

//-----------------------------------------------------------------------