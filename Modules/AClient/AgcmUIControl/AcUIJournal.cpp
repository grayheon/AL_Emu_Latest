#include "AcUIJournal.h"

RwTexture*	AcUIJournal::m_pBackTexture[ E_JOURNAL_IMAGE_COUNT ] = { NULL , NULL , NULL };
BOOL		AcUIJournal::m_bTextureLoad	=	FALSE;

AcUIJournal::AcUIJournal( VOID )
{
}

AcUIJournal::~AcUIJournal( VOID )
{
}

VOID	AcUIJournal::SetReadImage( CONST string& strFilename , eJournalImage eType )
{
	if( m_pBackTexture[ eType ] )
	{
		RwTextureDestroy( m_pBackTexture[eType] );
		m_pBackTexture[eType]	=	NULL;
	}

	m_pBackTexture[eType]	=	RwTextureRead( strFilename.c_str() , NULL );

	if( m_pBackTexture[eType] )
	{
		RwTextureSetFilterMode( m_pBackTexture[eType] , rwFILTERNEAREST );
		RwTextureSetAddressing( m_pBackTexture[eType] , rwTEXTUREADDRESSCLAMP );
	}
}

VOID	AcUIJournal::ClearImage( VOID )
{
	for( INT i = 0 ; i < E_JOURNAL_IMAGE_COUNT ; ++i )
	{
		if( m_pBackTexture[i] )
		{
			RwTextureDestroy( m_pBackTexture[i] );
			m_pBackTexture[i]	=	NULL;
		}
	}
}

VOID	AcUIJournal::OnWindowRender( VOID )
{
	INT	nStartPosX	=	0;
	INT	nStartPosY	=	0;

	INT	nOffsetY	=	0;

	INT	nRestY		=	this->h;

	ClientToScreen( &nStartPosX , &nStartPosY );

	UINT8 ualpha		=	255;

	if( m_pfAlpha )
		ualpha	*= 	(UINT8)(*m_pfAlpha);

	// Top Image ·»´õ
	if( m_pBackTexture[ E_JOURNAL_IMAGE_TOP] )
	{
		INT	nTopTextureWidth	=	RwRasterGetWidth( RwTextureGetRaster(m_pBackTexture[ E_JOURNAL_IMAGE_TOP ]) );
		INT	nTopTextureHeight	=	RwRasterGetHeight( RwTextureGetRaster(m_pBackTexture[ E_JOURNAL_IMAGE_TOP ]) );

		g_pEngine->DrawIm2D( m_pBackTexture[ E_JOURNAL_IMAGE_TOP ] , (float)nStartPosX , (float)nStartPosY , (float)nTopTextureWidth , (float)nTopTextureHeight , 0.f , 0.f , 1.f , 1.f , -1 , ualpha  );

		nOffsetY	+=	nTopTextureHeight;
		nRestY		-=	nTopTextureHeight;
	}

	// Bottom Image ·»´õ
	if( m_pBackTexture[ E_JOURNAL_IMAGE_BOTTOM ] )
	{
		INT	nBottomTextureWidth		=	RwRasterGetWidth( RwTextureGetRaster(m_pBackTexture[E_JOURNAL_IMAGE_BOTTOM]) );
		INT	nBottomTextureHeight	=	RwRasterGetHeight( RwTextureGetRaster(m_pBackTexture[E_JOURNAL_IMAGE_BOTTOM]) );

		g_pEngine->DrawIm2D( m_pBackTexture[ E_JOURNAL_IMAGE_BOTTOM ] , 
								(float)nStartPosX , 
								(float)(nStartPosY+(h-nBottomTextureHeight) ), 
								(float)nBottomTextureWidth , 
								(float)nBottomTextureHeight , 0.f , 0.f , 1.f , 1.f , -1 , ualpha  );

		nRestY		-=	nBottomTextureHeight;
	}

	// Center Image ·»´õ
	if( m_pBackTexture[ E_JOURNAL_IMAGE_CENTER ] )
	{
		INT	nCenterTextureWidth		=	RwRasterGetWidth( RwTextureGetRaster(m_pBackTexture[E_JOURNAL_IMAGE_CENTER] ) );
		INT	nCenterTextureHeight	=	RwRasterGetHeight( RwTextureGetRaster(m_pBackTexture[E_JOURNAL_IMAGE_CENTER] ) );

		while( nRestY >= 0 )
		{
			INT		nDrawHeight	=	0;
			nRestY	-=	nCenterTextureHeight;
			if( nRestY >= 0 )
				nDrawHeight	=	nCenterTextureHeight;

			else
				nDrawHeight =	nCenterTextureHeight + nRestY;

			g_pEngine->DrawIm2D( m_pBackTexture[ E_JOURNAL_IMAGE_CENTER ] , 
									(float)nStartPosX , 
									(float)(nStartPosY+nOffsetY ), 
									(float)nCenterTextureWidth , 
									(float)nDrawHeight , 0 , 0 , 1 , 1 , -1 , ualpha );

			nOffsetY	+=	nDrawHeight;
		}
	}
}