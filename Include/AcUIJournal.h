#pragma once

#include "AcUIBase.h"
#include <string>
#include <map>
#include <list>

enum eJournalImage 
{
	E_JOURNAL_IMAGE_TOP		,
	E_JOURNAL_IMAGE_CENTER	,
	E_JOURNAL_IMAGE_BOTTOM	,

	E_JOURNAL_IMAGE_COUNT	,
};

class AcUIJournal 
	:	public AcUIBase
{
public:
	AcUIJournal										( VOID );
	virtual ~AcUIJournal							( VOID );

	virtual VOID			OnWindowRender			( VOID );

	static	VOID			SetReadImage			( CONST string& strFilename , eJournalImage eType );
	static	VOID			ClearImage				( VOID );

protected:
	static BOOL				m_bTextureLoad;
	static RwTexture*		m_pBackTexture[ E_JOURNAL_IMAGE_COUNT ];
};
