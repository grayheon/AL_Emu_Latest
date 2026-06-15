#include "AgcmUIShrineShop.h"

#include "AgcmCharacter.h"

#include "AgpmItem.h"
#include "AgcmItem.h"

#include "AgcmUIItem.h"

#include "AgcmEventNPCTrade.h"

#include "AgcmUIMain.h"

AgcmUIShrineShop::AgcmUIShrineShop( VOID )
{
	SetModuleName( "AgcmUIShrineShop" );

	m_pcsAgcmUIManager2			=	NULL;
	m_pcsAgcmCharacter			=	NULL;
	m_pcsAgpmCharacter			=	NULL;
	m_pcsAgpmItem				=	NULL;
	m_pcsAgcmItem				=	NULL;
	m_pcsAgcmUIItem				=	NULL;
	m_pcsAgcmEventNPCTrade		=	NULL;
	
	m_pcsItemInfoUserData		=	NULL;

	m_pcsSelectItem				=	NULL;
	m_nTradeBuyStatus			=	-1;
	m_nTradeBuyLayer			=	-1;
	m_nTradeBuyRow				=	-1;
	m_nTradeBuyColumn			=	-1;

	m_pcsAgcmUIMain				=	NULL;

	m_bOpenBuyUI				=	FALSE;


}

AgcmUIShrineShop::~AgcmUIShrineShop( VOID )
{

}

BOOL	AgcmUIShrineShop::OnInit( VOID )
{
	m_pcsAgcmUIManager2	=	dynamic_cast< AgcmUIManager2* >( GetModule( "AgcmUIManager2" ) );
	if( !m_pcsAgcmUIManager2 )
		return FALSE;

	m_pcsAgcmCharacter	=	dynamic_cast< AgcmCharacter* >( GetModule( "AgcmCharacter" ) );
	if( !m_pcsAgcmCharacter )
		return FALSE;

	m_pcsAgpmItem		=	dynamic_cast< AgpmItem* >( GetModule( "AgpmItem" ) );
	if( !m_pcsAgpmItem )
		return FALSE;

	m_pcsAgcmItem		=	dynamic_cast< AgcmItem* >( GetModule( "AgcmItem" ) );
	if( !m_pcsAgcmItem )
		return FALSE;

	m_pcsAgpmCharacter	=	dynamic_cast< AgpmCharacter* >( GetModule( "AgpmCharacter" ) );
	if( !m_pcsAgpmCharacter )
		return FALSE;

	m_pcsAgcmUIItem		=	dynamic_cast< AgcmUIItem* >( GetModule( "AgcmUIItem" ) );
	if( !m_pcsAgcmUIItem )
		return FALSE;

	m_pcsAgcmEventNPCTrade	=	dynamic_cast< AgcmEventNPCTrade* >( GetModule( "AgcmEventNPCTrade" ) );
	if( !m_pcsAgcmEventNPCTrade )
		return FALSE;

	m_pcsAgcmUIMain		=	dynamic_cast< AgcmUIMain* >( GetModule( "AgcmUIMain" ) );
	if( !m_pcsAgcmUIMain )
		return FALSE;


	m_pcsAgcmUIMain->SetCallbackKeydownESC(CBKeydownESC, this);


	// UserData -----------
	m_pcsItemInfoUserData		=	m_pcsAgcmUIManager2->AddUserData( "Shrine_ComfirmUI_Refresh" , NULL , 0 , 0 , AGCDUI_USERDATA_TYPE_INT32 );

	// Event -----------
	m_nShrineBuyConfirmUIClose	=	m_pcsAgcmUIManager2->AddEvent( "Shrine_ItemBuyConfirmClose_Event2" );


	// Function -----------
	m_pcsAgcmUIManager2->AddFunction( this , "Shrine_ItemBuyComfirmUI_Open" , CBOpenBuyConfirmUI , 0 );
	m_pcsAgcmUIManager2->AddFunction( this , "Shrine_ItemBuyComfirmUI_Close" , CBCloseBuyConfirmUI , 0 );
	m_pcsAgcmUIManager2->AddFunction( this , "Shrine_ItemBuyClick" , CBItemBuyClick , 0 );

	
	// Display ------------
	m_pcsAgcmUIManager2->AddDisplay( this , "Shrine_ItemName_Display" , 0 , CBItemName , AGCDUI_USERDATA_TYPE_INT32 );
	m_pcsAgcmUIManager2->AddDisplay( this , "Shrine_ItemImage_Display" , 0 , CBItemImage , AGCDUI_USERDATA_TYPE_INT32 );
	m_pcsAgcmUIManager2->AddDisplay( this , "Shrine_ItemCount_Display" , 0 , CBItemCount , AGCDUI_USERDATA_TYPE_INT32 );

	m_pcsAgcmUIManager2->AddDisplay( this , "Shrine_ItemCharismaPrice_Display" , 0 , CBItemCharismaPrice , AGCDUI_USERDATA_TYPE_INT32 );
	m_pcsAgcmUIManager2->AddDisplay( this , "Shrine_ItemShrineCoinPrice_Display" , 0 , CBItemShrineCoinPrice , AGCDUI_USERDATA_TYPE_INT32 );
	m_pcsAgcmUIManager2->AddDisplay( this , "Shrine_ItemGheldPrice_Display" , 0 , CBItemGheldPrice , AGCDUI_USERDATA_TYPE_INT32 );

	m_pcsAgcmUIManager2->AddDisplay( this , "Shrine_ItemCharismaRest_Display" , 0 , CBItemCharismaRest , AGCDUI_USERDATA_TYPE_INT32 );
	m_pcsAgcmUIManager2->AddDisplay( this , "Shrine_ItemShrineCoinRest_Display" , 0 , CBItemShrineCoinRest , AGCDUI_USERDATA_TYPE_INT32 );
	m_pcsAgcmUIManager2->AddDisplay( this , "Shrine_ItemGheldRest_Display" , 0 , CBItemGheldRest , AGCDUI_USERDATA_TYPE_INT32 );


	return TRUE;
}

BOOL	AgcmUIShrineShop::CBOpenBuyConfirmUI( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase *pcsTarget , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	AgcdUI*					pUI		=	static_cast< AgcdUI* >(pData1);
	if( !pUI )
		return FALSE;

	// 아이템 세팅이 성공시에만 UI를 오픈한다.
	if( pThis->SetBuyItem() )
	{
		pThis->m_pcsAgcmUIManager2->OpenUI( pUI );
		pThis->m_pcsAgcmUIManager2->SetUserDataRefresh( pThis->m_pcsItemInfoUserData );

		pUI->m_pcsUIWindow->x	=	(pThis->m_pcsAgcmUIManager2->m_lWindowWidth / 2) - (pUI->m_pcsUIWindow->w / 2);
		pUI->m_pcsUIWindow->y	=	(pThis->m_pcsAgcmUIManager2->m_lWindowHeight / 2) - (pUI->m_pcsUIWindow->h / 2);

		pThis->m_bOpenBuyUI			=	TRUE;
	}
	
		
	return TRUE;
}

BOOL	AgcmUIShrineShop::CBCloseBuyConfirmUI( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase *pcsTarget , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->m_bOpenBuyUI			=	FALSE;

	return TRUE;

}


BOOL	AgcmUIShrineShop::CBItemBuyClick( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase *pcsTarget , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis		=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	AgpdCharacter*	pcsSelfCharacter	=	pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if( !pcsSelfCharacter )
		return FALSE;

	// Comfirm 창을 닫는다.
	pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_nShrineBuyConfirmUIClose );

	// 아이템 구입 처리를 한다.
	return pThis->ItemBuy();
}


BOOL	AgcmUIShrineShop::CBItemName( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	if( !pThis->m_pcsSelectItem || !pThis->m_pcsSelectItem->m_pcsItemTemplate )
		return FALSE;

	sprintf( szDisplay , "%s" , pThis->m_pcsSelectItem->m_pcsItemTemplate->GetName() );

	return TRUE;
}

BOOL	AgcmUIShrineShop::CBItemCount( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	szDisplay[0]	=	NULL;

	if( !pThis->m_pcsSelectItem || !pThis->m_pcsSelectItem->m_pcsItemTemplate )
		return FALSE;

	if( pThis->m_pcsSelectItem->m_nCount )
	{
		sprintf( szDisplay , "%d" , pThis->m_pcsSelectItem->m_nCount );
	}

	return TRUE;
}

BOOL	AgcmUIShrineShop::CBItemImage( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	if( !pcsSourceControl || !pcsSourceControl->m_pcsBase )
		return FALSE;

	pcsSourceControl->m_pcsBase->DestroyImage();

	if( !pThis->m_pcsSelectItem || !pThis->m_pcsSelectItem->m_pcsItemTemplate )
		return FALSE;
	
	AgcdItemTemplate*	pcdItemTemplate	=	pThis->m_pcsAgcmItem->GetTemplateData( pThis->m_pcsSelectItem->m_pcsItemTemplate );
	if( !pcdItemTemplate )
		return FALSE;

	char szTextureName[MAX_PATH];
	ZeroMemory(szTextureName, MAX_PATH);
#ifndef _BIN_EXEC_
	pcdItemTemplate->GetTextureName(szTextureName);
#else
	sprintf(szTextureName, "%s", pcdItemTemplate->GetFileNameTexture());
#endif
	INT		nImageID	=	pcsSourceControl->m_pcsBase->AddImage( szTextureName );
	pcsSourceControl->m_pcsBase->SetDefaultRenderTexture( nImageID );

	return TRUE;
}

BOOL	AgcmUIShrineShop::CBItemCharismaPrice( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	if( !pThis->m_pcsSelectItem || !pThis->m_pcsSelectItem->m_pcsItemTemplate )
		return FALSE;

	INT32	nCharismaPrice	=	pThis->m_pcsAgpmItem->GetCharismaPrice( pThis->m_pcsSelectItem );
	if( pThis->m_pcsSelectItem->m_nCount > 0 )
	{
		nCharismaPrice	*= pThis->m_pcsSelectItem->m_nCount;
	}

	sprintf( szDisplay , "%d" , nCharismaPrice );


	return TRUE;
}

BOOL	AgcmUIShrineShop::CBItemShrineCoinPrice( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	if( !pThis->m_pcsSelectItem || !pThis->m_pcsSelectItem->m_pcsItemTemplate )
		return FALSE;

	INT32	nShrineCoin		=	pThis->m_pcsAgpmItem->GetShrineCoinPrice( pThis->m_pcsSelectItem );
	if( pThis->m_pcsSelectItem->m_nCount > 0 )
	{
		nShrineCoin	*= pThis->m_pcsSelectItem->m_nCount;
	}

	sprintf( szDisplay , "%d" , nShrineCoin );

	return TRUE;
}

BOOL	AgcmUIShrineShop::CBItemGheldPrice( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	if( !pThis->m_pcsSelectItem || !pThis->m_pcsSelectItem->m_pcsItemTemplate )
		return FALSE;

	INT		nGheld			=	(INT)pThis->m_pcsAgpmItem->GetNPCPrice( pThis->m_pcsSelectItem );

	AgpdCharacter *pcsCharacter = pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	INT32 lTaxRatio = pThis->m_pcsAgpmCharacter->GetTaxRatio(pcsCharacter);
	INT32 lTax = 0;
	if (lTaxRatio > 0)
	{
		lTax = (nGheld * lTaxRatio) / 100;
	}
	nGheld = nGheld + lTax;

	if( pThis->m_pcsSelectItem->m_nCount > 0 )
	{
		nGheld	*= pThis->m_pcsSelectItem->m_nCount;
	}

	sprintf( szDisplay , "%d" , nGheld );

	return TRUE;
}

BOOL	AgcmUIShrineShop::CBItemCharismaRest( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	if( !pcsSourceControl || !pcsSourceControl->m_pcsBase )
		return FALSE;

	AgpdCharacter*		pcsCharacter	=	pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if( !pcsCharacter )
		return FALSE;

	if( !pThis->m_pcsSelectItem || !pThis->m_pcsSelectItem->m_pcsItemTemplate )
		return FALSE;

	INT32	nItemCharismaPrice	=	0;
	INT32	nCharCharismaPoint	=	pThis->m_pcsAgpmCharacter->GetCharismaPoint( pcsCharacter );

	if( pThis->m_pcsSelectItem->m_nCount > 0 )
	{
		nItemCharismaPrice	=	pThis->m_pcsAgpmItem->GetCharismaPrice( pThis->m_pcsSelectItem ) * pThis->m_pcsSelectItem->m_nCount;
	}

	INT32	nResult				=	nCharCharismaPoint - nItemCharismaPrice;

	sprintf( szDisplay , "%d" , nResult );

	if( nResult >= 0 )
	{
		pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor	=	0xffffffff;
	}
	else
	{
		pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor	=	0xffff0000;
	}
	
	return TRUE;
}

BOOL	AgcmUIShrineShop::CBItemShrineCoinRest( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	if( !pcsSourceControl || !pcsSourceControl->m_pcsBase )
		return FALSE;

	AgpdCharacter*		pcsCharacter	=	pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if( !pcsCharacter )
		return FALSE;

	if( !pThis->m_pcsSelectItem || !pThis->m_pcsSelectItem->m_pcsItemTemplate )
		return FALSE;

	INT32	nItemShrineCoinPrice	=	0;
	INT32	nCharShrineCoinCount	=	pThis->m_pcsAgpmItem->GetCharShrineCoinCount( pcsCharacter );

	if( pThis->m_pcsSelectItem->m_nCount > 0 )
	{
		nItemShrineCoinPrice	=	pThis->m_pcsAgpmItem->GetShrineCoinPrice( pThis->m_pcsSelectItem ) * pThis->m_pcsSelectItem->m_nCount;
	}

	INT32	nResult				=	nCharShrineCoinCount - nItemShrineCoinPrice;

	sprintf( szDisplay , "%d" , nResult );

	if( nResult >= 0 )
	{
		pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor	=	0xffffffff;
	}
	else
	{
		pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor	=	0xffff0000;
	}

	return TRUE;
}

BOOL	AgcmUIShrineShop::CBItemGheldRest( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	if( !pcsSourceControl || !pcsSourceControl->m_pcsBase )
		return FALSE;

	AgpdCharacter*		pcsCharacter	=	pThis->m_pcsAgcmCharacter->GetSelfCharacter();
	if( !pcsCharacter )
		return FALSE;

	if( !pThis->m_pcsSelectItem || !pThis->m_pcsSelectItem->m_pcsItemTemplate )
		return FALSE;

	INT32	nItemGheldPrice	=	0;
	INT64	nCharGheld		=	0;

	if( pThis->m_pcsSelectItem->m_nCount > 0 )
	{
		nItemGheldPrice	=	(INT32)(pThis->m_pcsAgpmItem->GetNPCPrice( pThis->m_pcsSelectItem ) * pThis->m_pcsSelectItem->m_nCount);
	}

	pThis->m_pcsAgpmCharacter->GetMoney( pcsCharacter , &nCharGheld );

	INT32	nResult				=	(INT32)(nCharGheld - nItemGheldPrice);

	sprintf( szDisplay , "%d" , nResult );

	if( nResult >= 0 )
	{
		pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor	=	0xffffffff;
	}
	else
	{
		pcsSourceControl->m_stDisplayMap.m_stFont.m_ulColor	=	0xffff0000;
	}

	return TRUE;
}

BOOL	AgcmUIShrineShop::SetBuyItem( VOID )
{
	INT		nItemID	=	m_pcsAgcmUIItem->GetBuyItemTID();

	m_nTradeBuyStatus	=	m_pcsAgcmUIItem->GetBuyItemStatus();
	m_nTradeBuyLayer	=	m_pcsAgcmUIItem->GetBuyItemLayer();
	m_nTradeBuyRow		=	m_pcsAgcmUIItem->GetBuyItemRow();
	m_nTradeBuyColumn	=	m_pcsAgcmUIItem->GetBuyItemColumn();

	if( !nItemID )
		return FALSE;

	m_pcsSelectItem	=	m_pcsAgpmItem->GetItem( nItemID );
	if( !m_pcsSelectItem )
		return FALSE;

	return TRUE;
}

BOOL	AgcmUIShrineShop::ItemBuy( VOID )
{
	BOOL			bResult	=	FALSE;

	if( !m_pcsSelectItem )
		return FALSE;

	AgpdCharacter*		pcsCharacter	=	m_pcsAgcmCharacter->GetSelfCharacter();
	if( !pcsCharacter )
		return FALSE;

	bResult	=	m_pcsAgcmEventNPCTrade->SendBuy(	pcsCharacter->GetID(), m_pcsSelectItem->m_lID,m_pcsSelectItem->m_nCount,
													(INT16)m_nTradeBuyStatus,
													(INT8)m_nTradeBuyLayer,
													(INT8)m_nTradeBuyRow,
													(INT8)m_nTradeBuyColumn
												);

	m_pcsSelectItem		= NULL;
	m_nTradeBuyStatus	= -1;
	m_nTradeBuyLayer	= -1;
	m_nTradeBuyRow		= -1;
	m_nTradeBuyColumn	= -1;


	return bResult;
}

VOID	AgcmUIShrineShop::UserInputMessageProc( RsEvent event , PVOID param )
{
	RsKeyStatus*	keyStatus	=	(RsKeyStatus*)param;

	// 키보드 입력 enter와 space를 받는다.
	if( m_pcsSelectItem && m_bOpenBuyUI && event == rsKEYDOWN )
	{
		if( keyStatus->keyCharCode == rsENTER || keyStatus->keyCharCode == rsPADENTER || keyStatus->keyCharCode == ' ' )
		{
			AgpdCharacter*	pcsSelfCharacter	=	m_pcsAgcmCharacter->GetSelfCharacter();
			if( !pcsSelfCharacter )
				return;

			// Comfirm 창을 닫는다.
			m_pcsAgcmUIManager2->ThrowEvent( m_nShrineBuyConfirmUIClose );
			m_bOpenBuyUI	=	FALSE;

			// 아이템 구입 처리를 한다.
			ItemBuy();
		}
	}

}

BOOL	AgcmUIShrineShop::CBKeydownESC( PVOID pData, PVOID pClass, PVOID pCustData )
{
	AgcmUIShrineShop*		pThis	=	static_cast< AgcmUIShrineShop* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->m_bOpenBuyUI	=	FALSE;

	pThis->m_pcsAgcmUIManager2->ThrowEvent( pThis->m_nShrineBuyConfirmUIClose );

	return TRUE;
}