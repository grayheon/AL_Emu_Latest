#include "AcUIButtonEx.h"

AcUIButtonEx::AcUIButtonEx()
{
	m_nType = TYPE_BUTTON_EX;
}

AcUIButtonEx::~AcUIButtonEx()
{
}

BOOL AcUIButtonEx::HitTest( INT32 x , INT32 y	)
{
	if( AgcWindow::HitTest( x,y) == FALSE )
		return FALSE;
	
	// 인단 현재 Texture를 가져온다 
	RwTexture* pTexture = m_stRenderInfo.lRenderID ? m_csTextureList.GetImage_ID( m_stRenderInfo.lRenderID ) : NULL;
	if( NULL == pTexture )
		return TRUE;

	RwRaster *pRaster = RwTextureGetRaster( pTexture );
	if( NULL == pRaster )
		return FALSE;

	BYTE*  pSrc = (BYTE*) RwRasterLock(pRaster,0,rwRASTERLOCKREAD);
	if( NULL == pSrc  )
		return FALSE;

	int iAlphaBit = 0;
	int iPixelSize = 0;
	
	switch( RwRasterGetFormat( pRaster) &0x0f00 )
	{
	case rwRASTERFORMAT1555:			iAlphaBit = 1; iPixelSize = 2; break;
	case rwRASTERFORMAT4444: 		iAlphaBit = 4; iPixelSize = 2; break;
	case rwRASTERFORMAT8888: 		iAlphaBit = 8; iPixelSize = 4; break;
	default:RwRasterUnlock( pRaster ); return FALSE;
	}
	

	int		SrcPitch = RwRasterGetStride(pRaster);
	int		posx = 0 , posy = 0 ;
	ClientToScreen( &posx , &posy );
	int		iOffset = SrcPitch*(y-posy) + (x-posx)*iPixelSize;
	BYTE byColor = 0;
	if( pRaster->width * pRaster->height * iPixelSize> iOffset )
	{
		//텍스쳐 타입이 tif인 경우는 최상의 비트에 알파값이
		//byColor = (BYTE)pSrc[iOffset];
		
		//png타입은 최하의 비트에 알파값이 있다.
		byColor = (BYTE)pSrc[iOffset+3];

	}

	RwRasterUnlock( pRaster );

	if( byColor > 0 )
	{
		//해당 컬러에 알파값이 있는지 알아 본다.
		switch( iAlphaBit )
		{
		case 1: return ((byColor & 0x80) )? TRUE : FALSE;		/**<16 bits - 1 bit alpha, 5 bits red, green and blue */
		case 4: return ((byColor & 0xf0)  )? TRUE : FALSE;		/**<16 bits - 4 bits per component */
		case 8: return ((byColor & 0xff)  )? TRUE : FALSE;			/**<32 bits - 8 bits per component */
		}
	}
	return FALSE;
}
