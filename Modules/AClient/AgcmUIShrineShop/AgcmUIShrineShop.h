#pragma once


#include "AgcModule.h"

#include "AgcmUIManager2.h"

class AgcmCharacter;
class AgcmItem;
class AgpmItem;
class AgcmUIItem;
class AgcmEventNPCTrade;
class AgcmUIMain;


class	AgcmUIShrineShop
	:	public	AgcModule
{
public:
	AgcmUIShrineShop			( VOID );
	~AgcmUIShrineShop			( VOID );

	virtual	BOOL				OnInit					( VOID );

	// UI창이 열렸을경우 처리
	VOID						UserInputMessageProc	( RsEvent event , PVOID param  );

	// 거래에 필요한 아이템 정보를 세팅한다.
	BOOL						SetBuyItem				( VOID );

	// 아이템 구입 처리
	BOOL						ItemBuy					( VOID );

	// 아이템 구입할건지 Confirm창
	static BOOL					CBOpenBuyConfirmUI		( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase *pcsTarget , AgcdUIControl *pcsSourceControl );

	// Confirm창 Close시에 불린다.
	static BOOL					CBCloseBuyConfirmUI		( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase *pcsTarget , AgcdUIControl *pcsSourceControl );

	// 아이템 구입 버튼을 Click
	static BOOL 				CBItemBuyClick			( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 , PVOID pData4 , PVOID pData5 , ApBase *pcsTarget , AgcdUIControl *pcsSourceControl );
	

	// 아이템 이름 및 이미지 표시를 갱신해 주는 Display 함수
	static BOOL					CBItemName				( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl );
	static BOOL					CBItemImage				( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl );
	static BOOL					CBItemCount				( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl );

	// 아이템 구입시 필요한 요구사항이 갱신되는 Display 함수
	static BOOL					CBItemCharismaPrice		( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl );
	static BOOL					CBItemShrineCoinPrice	( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl );
	static BOOL					CBItemGheldPrice		( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl );

	// 아이템 구입시 잔액 표시를 갱신시켜주는 Display 함수
	static BOOL					CBItemCharismaRest		( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl );
	static BOOL					CBItemShrineCoinRest	( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl );
	static BOOL					CBItemGheldRest			( PVOID pClass , PVOID pData , AgcdUIDataType eType , INT32 lID , CHAR *szDisplay , INT32 *plValue , AgcdUIControl *pcsSourceControl );

	static BOOL					CBKeydownESC			( PVOID pData, PVOID pClass, PVOID pCustData );


private:
	AgcmUIManager2*				m_pcsAgcmUIManager2;
	AgcmCharacter*				m_pcsAgcmCharacter;
	AgpmCharacter*				m_pcsAgpmCharacter;
	AgcmEventNPCTrade*			m_pcsAgcmEventNPCTrade;

	AgcmUIItem*					m_pcsAgcmUIItem;
	AgpmItem*					m_pcsAgpmItem;
	AgcmItem*					m_pcsAgcmItem;

	AgcmUIMain*					m_pcsAgcmUIMain;

	AgcdUIUserData*				m_pcsItemInfoUserData;

	INT32						m_nShrineBuyConfirmUIClose;

	// 구입하기 위해서 선택한 아이템
	AgpdItem*					m_pcsSelectItem;
	INT							m_nTradeBuyStatus;
	INT							m_nTradeBuyLayer;
	INT							m_nTradeBuyRow;
	INT							m_nTradeBuyColumn;

	BOOL						m_bOpenBuyUI;


};