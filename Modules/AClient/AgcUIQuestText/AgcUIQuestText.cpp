#include <algorithm>
#include <cassert>

#include "AgpmItem.h"
#include "AgcmItem.h"
#include "AcUIBase.h"
#include "AgcmMinimap.h"
#include "AcUIWorldmap.h"
#include "AgcmQuestJournal.h"
#include "AgcmUIQuestJournal.h"
#include "AgcmUINotice.h"
#include "AcUIToolTip.h"
#include "AgcmUIManager2.h"
#include "AgcmUIITem.h"
#include "AcUIEventButton.h"
#include "AcUIToolTip.h"
#include "AgcmCharacter.h"
#include "AgcmFont.h"
#include "AcUIScroll.h"
#include "AcUIToolTip.h"
#include "AgcEngine.h"

#include "AgcmUIQuest2.h"

#include "../Include/AgcUIQuestText.h"

#include <vector>

extern AgcEngine * g_pEngine;

//-----------------------------------------------------------------------
//

AgcUIQuestText::AgcUIQuestText()
	: ownerUI_(0)
	, questTextUI_( new AcUIBase )
	, textScrollUI_(0)
	, maxScroll_(0)
	, agcmFont_(0)
#ifdef _AREA_GLOBAL_
	, lineCheckByWord_(true)
#else
	, lineCheckByWord_(false)
#endif
{
	questTextUI_->m_Property.bClipping = TRUE;
}

//-----------------------------------------------------------------------
//

AgcUIQuestText::~AgcUIQuestText()
{
	ClearTextList(TRUE);

	if( ownerUI_ )
		ownerUI_->DeleteChild( questTextUI_ );

	if( questTextUI_ )
	{
		delete questTextUI_;
		questTextUI_ = NULL;
	}
}

//-----------------------------------------------------------------------
//

void AgcUIQuestText::Init()
{
	#define _getMoudle( module, member )					\
	member = (module *)g_pEngine->GetModule( #module );		\
	assert( member && ( "모듈을 찾을 수 없습니다 : " #module " - AgcUIQuestText::Init()" ) );

	_getMoudle( AgcmQuestJournal,		agcmQuestJournal_	);
	_getMoudle( AgcmUIQuestJournal,		agcmUIQuestJournal_ );
	_getMoudle( AgcmMinimap,			agcmMinimap_		);
// 	_getMoudle( AgcmUICombine,			agcmUIConbine_		);
	_getMoudle( AgcmUIWorldmap,			agcmUIWorldmap_		);
// 	_getMoudle( AgcmUINotice,			agcmUINotice_		);
	_getMoudle( AgpmItem,				agpmItem_			);
	_getMoudle( AgcmItem,				agcmItem_			);
	_getMoudle( AgcmUIItem,				agcmUIItem_			);
	_getMoudle( AgcmUIManager2,			agcmUIManager_		);
	_getMoudle( AgcmCharacter,			agcmCharacter_		);
	_getMoudle( AgcmFont,				agcmFont_			);
	_getMoudle( AgcmUIQuest2,			agcmUIQuest2_		);

	static bool registCallback = false;

	if( !registCallback )
	{
		registCallback = true;
		agpmItem_->SetCallbackItemInformationResult( CBQuestRegisterItem , this );
	}

	#undef _getMoudle
}

//-----------------------------------------------------------------------
//

void AgcUIQuestText::SetScrollControl( AcUIScroll * scrollCtrl )
{
	textScrollUI_ = scrollCtrl;

	//	Scroll을 세팅한다.
	if( textScrollUI_ )
	{		
		if( textScrollUI_ )
		{
			textScrollUI_->SetRefreshCallback( this, CBRefreshTextScroll , NULL , NULL , NULL );
			questTextUI_->SetMouseWheelCallBack( this, CBMouseWheel );
		}
	}
}

//-----------------------------------------------------------------------
//


BOOL AgcUIQuestText::AddQuestItemInfo( AgpdItem* pdItem )
{
	if( !pdItem && !pdItem->m_pcsItemTemplate )
		return FALSE;

	questItemInfos_[ pdItem->m_pcsItemTemplate->GetID() ] = pdItem;

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcUIQuestText::DelQuestItemInfo( AgpdItem* pdItem )
{
	if( !pdItem && !pdItem->m_pcsItemTemplate )
		return FALSE;
	
	QuestItemInfos::iterator iter = questItemInfos_.find( pdItem->m_pcsItemTemplate->GetID() );

	if( iter != questItemInfos_.end() )
	{
		questItemInfos_.erase( iter );
		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------
//

AgpdItem* AgcUIQuestText::GetQuestItemInfo( INT tid )
{
	QuestItemInfos::iterator iter = questItemInfos_.find( tid );

	return iter != questItemInfos_.end() ? iter->second : 0;
}

//-----------------------------------------------------------------------
//

void AgcUIQuestText::ClearQuestItemInfo()
{
	_create_functor__( AgcUIQuestText, 
		void, func, ( QuestItemInfos::value_type & itemInfo ), { this_->agpmItem_->DestroyItem( itemInfo.second ); }
	);

	std::for_each( questItemInfos_.begin(), questItemInfos_.end(), func );

	questItemInfos_.clear();
}

//-----------------------------------------------------------------------
//

BOOL AgcUIQuestText::CBOpenQuestItemTooltip( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcUIQuestText*	pThis	=	static_cast< AgcUIQuestText* >(pClass);
	if( !pThis )
		return FALSE;

	AgpdCharacter*	selfCharacter = pThis->agcmCharacter_->GetSelfCharacter();

	INT				tid	=	(INT)pData1;
	AgpdItem*		pdItem	=	pThis->GetQuestItemInfo( tid );

	if( pdItem )
	{
		pThis->OpenQuestItemTooltip( pdItem );
		return TRUE;
	}

	pThis->agcmItem_->SendTooltipRegisterItemRequest( 0 , tid , selfCharacter->GetID(), eAgpmItemTooltipType::quest_grid );

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcUIQuestText::CBCloseQuestItemTooltip( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcUIQuestText*	pThis = static_cast< AgcUIQuestText* >(pClass);
	if( !pThis )
		return FALSE;

	pThis->CloseQuestItemTooltip();

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL	AgcUIQuestText::CBRefreshTextScroll( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 )
{
	AgcUIQuestText*	pThis = static_cast< AgcUIQuestText* >(pClass);

	if( !pThis || !pThis->textScrollUI_ )
		return FALSE;

	FLOAT			fValue	=	pThis->textScrollUI_->GetScrollValue();

	if( pThis->maxScroll_ > 0 )
	{
		pThis->questTextUI_->MoveWindow( pThis->questTextUI_->x , (INT32)-(pThis->maxScroll_*fValue),
			pThis->questTextUI_->w , pThis->questTextUI_->h );
	}

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcUIQuestText::CBMouseWheel( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 )
{
	AgcUIQuestText* pThis	=	static_cast< AgcUIQuestText* >(pClass);
	if( !pThis )
		return FALSE;

	INT				nDelta	=	(INT)pData1;

	FLOAT			fValue	=	pThis->textScrollUI_->GetScrollValue();
	FLOAT			fResult	=	fValue + (pThis->textScrollUI_->m_fScrollUnit*(-nDelta/40));

	if( fResult < 0.00f )
		fResult	=	0;

	else if( fResult > 1.0f )
		fResult =	1.0f;

	pThis->textScrollUI_->SetScrollValue( fResult );
	pThis->questTextUI_->MoveWindow( pThis->questTextUI_->x , (INT32)-(pThis->maxScroll_*fResult),
		pThis->questTextUI_->w , pThis->questTextUI_->h	);

	return TRUE;
}

//-----------------------------------------------------------------------
//

VOID AgcUIQuestText::OpenQuestItemTooltip( AgpdItem* pdItem )
{
	AgpdCharacter*	selfCharacter	=	agcmCharacter_->GetSelfCharacter();
	if( !selfCharacter )
		return;

	agcmUIItem_->OpenToolTip( pdItem , (INT32)agcmUIManager_->m_v2dCurMousePos.x , (INT32)agcmUIManager_->m_v2dCurMousePos.y , FALSE , FALSE );
}

//-----------------------------------------------------------------------
//

BOOL AgcUIQuestText::AddTooltipAction( AcUIBase* pcsBase , INT nTID )
{
	AcUIEventButton * button = dynamic_cast< AcUIEventButton* >(pcsBase);

	if( !button )
		return FALSE;

	button->SetCallbackSetFocusEvent( this , CBOpenQuestItemTooltip );

	button->SetFocusData( (PVOID)nTID , NULL );

	button->SetCallbackKillFocusEvent( this, CBCloseQuestItemTooltip );

	return TRUE;
}

//-----------------------------------------------------------------------
//

namespace StringUtill
{
	typedef std::vector<std::string> SplitResult;

	void split( SplitResult & result, std::string str, std::string token )
	{
		for( size_t idx = str.find( token ); !str.empty(); idx = str.find( token ) )
		{
			std::string s = str.substr( 0, idx );

			if( !s.empty() )
				result.push_back( s );

			if( idx != std::string::npos )
				idx += token.length();

			str.erase( 0, idx );
		}
	}
}

//-----------------------------------------------------------------------
//

std::string AgcUIQuestText::SliceLineByWord( std::string const & str, int totalWidth )
{
	if( str.empty() || (questTextUI_->w) <= 0 )
		return "";

	std::string lineStr;

	int width = 0;

	size_t spaceIdx = str.find( " " );

	// 공백이 있는 문자는 단어단위로 나눠서 한 단어씩 붙여서 길이를 잰다.
	if( spaceIdx != std::string::npos )
	{
		typedef StringUtill::SplitResult SplitResult;

		SplitResult strings;

		StringUtill::split( strings, str, " " );

		if( strings.empty() )
			return 0;

		for( SplitResult::iterator iter = strings.begin(); iter != strings.end(); ++iter )
		{
			std::string temp = lineStr;

			if( iter != strings.begin() )
				temp += " ";
				
			temp += *iter;

			int tempWidth = agcmFont_->GetTextExtent( 0, (char*)temp.c_str(), (int)(temp.length()) );

			if( tempWidth + totalWidth >= questTextUI_->w )
			{
				// 단어 단위로 나눴지만 첫번째 단어가 ui사이즈를 넘어가는 경우
				if( iter == strings.begin() && totalWidth < 1 )
				{
					lineStr = temp;
					width = tempWidth;
				}

				break;
			}
			else
			{
				lineStr = temp;
				width = tempWidth;
			}
		}
	}
	else
	{
		lineStr = str;
		width = agcmFont_->GetTextExtent( 0, (char*)lineStr.c_str(), (int)(lineStr.length()) );

		if( totalWidth > 0 && (width + totalWidth > questTextUI_->w) )
			lineStr = "";
	}

	if( totalWidth < 1 )
	{
		// 조합된 스트링을 다시 ui사이즈에 맞게 뒤에서부터 한글자씩 뺀다.
		while( width + totalWidth >= questTextUI_->w )
		{
			if( lineStr.empty() )
				break;

			// 한글이라면 두글자를 뺀다.
			if( lineStr.length() > 1 && lineStr[ lineStr.length() - 1 ] < 0 && lineStr[ lineStr.length() - 2 ] < 0 )
				lineStr.erase( lineStr.length()-2, 2 );
			else
				lineStr.erase( lineStr.length()-1, 1 );

			width = agcmFont_->GetTextExtent( 0, (char*)lineStr.c_str(), (int)(lineStr.length()) );
		}
	}

	return lineStr;
}

//-----------------------------------------------------------------------
//

INT32 AgcUIQuestText::InsertQuestControl( INT32 questTID, QuestControlInfo * pControlInfo, std::string const & strControlName, INT32 nTotalStringWidth, DWORD dwColor, INT32 nHeight, INT32 nStringWidth )
{
	INT32 nControlHeight = _font_size;

	if( !pControlInfo )
		return nControlHeight;

	if( pControlInfo->m_eControlType == E_QUEST_CONTROL_TEXT )
	{
		pControlInfo->m_pUIBase->SetStaticStringExt( (CHAR*)strControlName.c_str() , 1.0f , 0 , dwColor , true , ACUIBASE_HALIGN_CENTER , ACUIBASE_VALIGN_CENTER );
		pControlInfo->m_pUIBase->MoveWindow( nTotalStringWidth , nHeight , nStringWidth , _font_size );
	}
	else if( pControlInfo->m_eControlType == E_QUEST_CONTROL_POSITION_LINK )
	{
		pControlInfo->m_pUIBase->SetStaticStringExt( (CHAR*)strControlName.c_str() , 1.0f , 0 , dwColor , true , ACUIBASE_HALIGN_CENTER , ACUIBASE_VALIGN_CENTER );
		pControlInfo->m_pUIBase->MoveWindow( nTotalStringWidth , nHeight , nStringWidth , _font_size );

		AddPositionLinkEvent( pControlInfo->m_pUIBase , pControlInfo->m_nPosX , pControlInfo->m_nPosY , pControlInfo->m_nRegionIndex, questTID );

		//INT				nID				=	pControlInfo->m_nPosX + pControlInfo->m_nPosY;
		//nID		+=	strControlName.length();
		//nID		=	abs( nID );
	}

	else if( pControlInfo->m_eControlType == E_QUEST_CONTROL_ITEM )
	{			
		pControlInfo->m_pUIBase->MoveWindow( nTotalStringWidth , nHeight , pControlInfo->m_pUIBase->w , pControlInfo->m_pUIBase->h  );

		AgpdItemTemplate*	pcsItemTemplate	= agpmItem_->GetItemTemplate( pControlInfo->m_nTID );
		if( pcsItemTemplate )
		{
			AcUIEventButton*	pButton	=	new AcUIEventButton;
			pControlInfo->m_pUIButton	=	pButton;
			pControlInfo->m_pUIButton->m_Property.bClipping	=	TRUE;
			pControlInfo->m_pUIBase->AddChild( pControlInfo->m_pUIButton );

			AgcdItemTemplate*	pcdItemTemplate	= agcmItem_->GetTemplateData( pcsItemTemplate);

			if( pcdItemTemplate )
			{
				agcmItem_->m_pcsAgcmResourceLoader->SetTexturePath( agcmItem_->m_szIconTexturePath );

#if (!defined(USE_MFC)&&defined(_BIN_EXEC_))
				pButton->AddButtonImage( pcdItemTemplate->GetFileNameTexture() , ACUIBUTTON_MODE_NORMAL );
				pButton->AddButtonImage( pcdItemTemplate->GetFileNameTexture() , ACUIBUTTON_MODE_ONMOUSE );
				pButton->AddButtonImage( pcdItemTemplate->GetFileNameTexture() , ACUIBUTTON_MODE_CLICK );
#elif (!defined(USE_MFC)&&!defined(_BIN_EXEC_))
				char szTextureName[MAX_PATH];
				ZeroMemory(szTextureName, MAX_PATH);
				pcdItemTemplate->GetTextureName(szTextureName);
				pButton->AddButtonImage( szTextureName , ACUIBUTTON_MODE_NORMAL );
				pButton->AddButtonImage( szTextureName , ACUIBUTTON_MODE_ONMOUSE );
				pButton->AddButtonImage( szTextureName , ACUIBUTTON_MODE_CLICK );
#endif
				pButton->MoveWindow( 3 , 3 , (INT32)pButton->m_fTextureWidth , (INT32)pButton->m_fTextureHeight );

				AddTooltipAction( pButton , pControlInfo->m_nTID );

				agcmItem_->m_pcsAgcmResourceLoader->SetTexturePath( (CHAR*)agcmUIManager_->GetTexturePath() );

				if( pControlInfo->m_bSelect )
				{
					pButton->SetButtonSelectImage( "Quest_ActiveBox.png" );
					pButton->SetIsSelected( FALSE );
					AddClickSelectAction( pButton );
				}

				if( pControlInfo->m_nCount )
				{
					CHAR	szCount[20];
					sprintf_s( szCount , "%d" , pControlInfo->m_nCount );

					pButton->SetStaticStringExt( szCount , 1.0f , 1 , 0xffff9999 , true , ACUIBASE_HALIGN_RIGHT , ACUIBASE_VALIGN_BOTTOM );
				}
			}
		}
		nControlHeight	=	pControlInfo->m_pUIBase->h;
	}

	questTexts_.push_back( pControlInfo);

	return nControlHeight;
}

//-----------------------------------------------------------------------
//

INT AgcUIQuestText::MakeTextList( CHAR const * szBuffer , DWORD dwColor , INT nHeight , INT nIndex, INT questTID )
{
	if( !szBuffer )
		return nHeight;

	string			strTemp;
	BOOL			bEnd				=	TRUE;
	BOOL			bExit				=	TRUE;
	BOOL			bControlItem		=	FALSE;
	BOOL			bForceNextLine		=	FALSE;
	INT				nStartPos			=	0;
	INT				nLastPos			=	0;
	INT				nStringWidth		=	0;
	INT				nNextLine			=	0;
	INT				nTotalStringWidth	=	0;
	INT				nLineY				=	0;
	INT				nControlHeight		=	_font_size;

	for( INT i = 0 ; bExit ; ++i )
	{
		strTemp	+=	szBuffer[i];

		if( szBuffer[i] == '<' )
			nStartPos	=	i;

		if( szBuffer[i] == '>' || szBuffer[i] == 0 || szBuffer[i+1] == '<' )
		{
			if( szBuffer[i] == 0 )
				bExit = FALSE;

			nLastPos	=	i;

			if( szBuffer[nLastPos] < 0 )
				++nLastPos;

			// 이건 컬러다( 컬러 값만 바꿔주자 )
			if( szBuffer[nStartPos] == '<' && szBuffer[nStartPos+1] == 'H' )
			{
				size_t	nstringPos	=	strTemp.find( ">" );
				string	strText		=	strTemp.substr( 2 , (nstringPos-2) );

				dwColor		=	HexStringToDWORD( strText.c_str() );

				nStartPos			=	i+1;
				strTemp.clear();

				continue;
			}

			// 이건 줄바꿈이다.
			if( szBuffer[nStartPos] == '<' && szBuffer[nStartPos+1] == '/' && szBuffer[nStartPos+2] == 'n' )
			{
				bEnd		=	FALSE;
				nNextLine	=	nLastPos + 1;
				break;
			}

			string		strControlName;
			INT			nSizeX			=	0;
			INT			nSizeY			=	0;
			INT			nCount			=	0;
			INT			nUpgrade		=	0;
			INT			nSocket			=	0;
			INT			nTID			=	0;
			INT			nBoundType		=	0;
			BOOL		bSelect			=	FALSE;

			string strName;

			//-----------------------------------------------------------------------
			// QuestControlInfo 생성

			QuestControlInfo * pControlInfo = new QuestControlInfo;

			pControlInfo->m_eControlType = CalcEventType( strTemp , pControlInfo , strName );

			if( pControlInfo->m_eControlType == E_QUEST_CONTROL_ITEM )
			{
				pControlInfo->m_pUIBase							=	new AcUIBase;
				pControlInfo->m_pUIBase->m_Property.bClipping	=	TRUE;

				INT			nTextureIndex	=	pControlInfo->m_pUIBase->AddImage( "Quest_Slot.png" );
				pControlInfo->m_pUIBase->SetDefaultRenderTexture( nTextureIndex );

				nStringWidth	= pControlInfo->m_pUIBase->w;
			}

			else if( pControlInfo->m_eControlType == E_QUEST_CONTROL_POSITION_LINK )
			{
				pControlInfo->m_pUIBase							=	new AcUIEventButton;
				pControlInfo->m_pUIBase->m_Property.bClipping	=	TRUE;
				strControlName	= strName;
				nStringWidth	= agcmFont_->GetTextExtent( 0 , (CHAR*)strControlName.c_str() , (INT32)strControlName.length() );

				agcmUIQuest2_->AddQuestPositionInfo( questTID, MapLinkPos( (float)pControlInfo->m_nPosX, (float)pControlInfo->m_nPosY ), strControlName );
			}
			else
			{
				pControlInfo->m_pUIBase							=	new AcUIBase;
				pControlInfo->m_pUIBase->m_Property.bClipping	=	TRUE;
				strControlName	= strTemp;
				nStringWidth	= agcmFont_->GetTextExtent( 0 , (CHAR*)strControlName.c_str() , (INT32)strControlName.length() );
			}

			bForceNextLine = FALSE;

			//-----------------------------------------------------------------------
			//

			// 이벤트 컨트롤이 부모 윈도우 사이즈보다 크다.
			if( questTextUI_->w < nTotalStringWidth+nStringWidth )
			{
				// 아이템컨트롤은 윈도우 사이즈보다 커도 그냥 출력한다.
				if( pControlInfo->m_eControlType == E_QUEST_CONTROL_ITEM )
				{
					nNextLine	=	nStartPos;
					bForceNextLine = TRUE;
				}
				// 퀘스트 텍스트에 링크의 문자열 길이가 컨트롤 윈도우 가로폭 보다 긴 상황이다.
				// 한 줄에 들어갈 수 있는 만큼씩 버튼을 여러개로 나눠서 생성한다.
				else if( pControlInfo->m_eControlType == E_QUEST_CONTROL_POSITION_LINK )
				{
					int posX = pControlInfo->m_nPosX;
					int posY = pControlInfo->m_nPosY;
					int regionIdx = pControlInfo->m_nRegionIndex;

					DEF_SAFEDELETE( pControlInfo->m_pUIBase );
					DEF_SAFEDELETE( pControlInfo );

					std::string orgControlText = strControlName;

					nStringWidth = 0;

					while( !orgControlText.empty() )
					{
						strControlName = SliceLineByWord( orgControlText, nTotalStringWidth + nStringWidth );

						if( strControlName.empty() )
						{
							if( nTotalStringWidth == 0 )
							{
								break;
							}
							else
							{
								nTotalStringWidth = 0;
								nHeight += _font_size;
								continue;
							}
						}

						nStringWidth = agcmFont_->GetTextExtent( 0 , (CHAR*)strControlName.c_str() , (int)strControlName.length() );

						QuestControlInfo * pControlInfo = new QuestControlInfo;
						pControlInfo->m_pUIBase = new AcUIEventButton;
						pControlInfo->m_pUIBase->m_Property.bClipping = TRUE;

						pControlInfo->m_nPosX = posX;
						pControlInfo->m_nPosY = posY;
						pControlInfo->m_nRegionIndex = regionIdx;
						pControlInfo->m_eControlType = E_QUEST_CONTROL_POSITION_LINK;
						CHAR	tempStr[1024] = {0,};
						ZeroMemory(tempStr, 1024);

						strcpy(tempStr, strControlName.c_str());
						InsertQuestControl( questTID, pControlInfo, strControlName, nTotalStringWidth, dwColor, nHeight, nStringWidth );

						nTotalStringWidth += nStringWidth;

						nStringWidth = 0;

						orgControlText.erase( 0, orgControlText.find( strControlName ) + strControlName.length() );
					}

					bEnd = TRUE;
					nNextLine = nLastPos;
					bForceNextLine = TRUE;
					strTemp.clear();
				}
				// 텍스트가 아니면 다음줄로 넘기거나 종료
				else if( pControlInfo->m_eControlType != E_QUEST_CONTROL_TEXT )
				{
					bEnd		=	FALSE;
					nNextLine	=	nStartPos;

					if( questTextUI_->w	< nStringWidth )
						bEnd = TRUE;

					if( pControlInfo )
					{
						DEF_SAFEDELETE( pControlInfo->m_pUIBase );
						DEF_SAFEDELETE( pControlInfo );
					}					
					
					break;
				}
				else
				{
					// 일반 문자열은 초과하면 가능한 줄까지 찍고 넘겨야 된다
					bEnd	=	FALSE;

					if( lineCheckByWord_ ) // 단어단위로 라인체크
					{
						if( !strControlName.empty() )
						{
							std::string orgText = strControlName;

							strControlName = SliceLineByWord( orgText, nTotalStringWidth );

							if( strControlName.empty() )
							{
								bForceNextLine = TRUE;
								nNextLine = nStartPos;
								break;
							}
							else
							{
								nLastPos -= orgText.length() - (orgText.find( strControlName ) + strControlName.length()) - 1;

								if( nLastPos < 0 )
									nLastPos = 0;

								nStringWidth = agcmFont_->GetTextExtent( 0 , (CHAR*)strControlName.c_str() , (int)strControlName.length() );
							}

							if( szBuffer[nLastPos] == ' ' )
							{
								++nLastPos;
							}
						}						

						nNextLine = nLastPos;
					}
					else
					{
						CHAR	tempStr[1024] = {0,};
						ZeroMemory(tempStr, 1024);

						strcpy(tempStr, strControlName.c_str());

						BOOL	bTwobyte	=	FALSE;
						INT		nLength		=	strControlName.length()-1;

						for( INT k = 1 ; ; ++k )
						{
							if( !bTwobyte )
							{
								if( nLength >= (k+1) )
								{
									// 2바이트 문자 처리
									if( strControlName[nLength-(k+1)] < 0 &&
										strControlName[nLength-k] < 0 )
									{
										//nOffset		=	1;
										bTwobyte	=	TRUE;

										continue;
									}
								}
							}

							bTwobyte	=	FALSE;

							INT	nWidth	=	agcmFont_->GetTextExtent( 0 , (CHAR*)strControlName.c_str() , strlen( strControlName.c_str() )-k );

							if( nWidth+nTotalStringWidth <= questTextUI_->w )
							{

								// 짤라서 찍어줄게 있을경우에는 찍어주고
								if( nWidth )
								{
									int tempLen = strControlName.length()-(k+1);
									strControlName	=	strControlName.substr( 0 , strControlName.length()-(k+1) );
									nLastPos		-=	(k);

									nStringWidth	=	agcmFont_->GetTextExtent( 0 , (CHAR*)strControlName.c_str() , strlen( strControlName.c_str() ) );
								}

								// 짤라 찍을 문자열이 없을 경우에는 바로 다음 라인으로 넘긴다
								else
								{
// 									nLastPos		-=	(k-1);
									nLastPos		-=	(k-1);
									bForceNextLine	=	TRUE;
								}

								nNextLine		=	nLastPos;
								break;
							}
						}	
					}
				}
			}

			if( !bForceNextLine )
			{
				//실제로 집어 넣기 전 결과물 체크
				int nlen = strControlName.length();
				int nByteCount = 0;
				for(int j=0 ; j<nlen ; ++j)
				{
					if(strControlName[j]<0)
					{
						++nByteCount;
						continue;
					}
				}
				if(nByteCount%2!=0)
				{
					strControlName = strControlName.substr( 0 , strControlName.length()-1);
					--nNextLine;
					--nLastPos;
				}

				int height = InsertQuestControl( questTID, pControlInfo, strControlName, nTotalStringWidth, dwColor, nHeight, nStringWidth );
				nControlHeight = nControlHeight > height ? nControlHeight : height;
				nTotalStringWidth	+=	nStringWidth;
				nStartPos			=	i+1;
			}

			strTemp.clear();

			if( !bEnd )
				break;
		}
	}
	INT		nReturnHeight	=	nHeight+nControlHeight;

	if( !bEnd )
	{
		//다음 줄을 만들기 전에 결과물 체크
		int nStrLength = strlen(szBuffer);
		int nByteCount = 0;

		for(int j = nNextLine ; j<nStrLength ; ++j)
		{
			if(szBuffer[j]<0)
			{
				++nByteCount;
				continue;
			}
			else
			{
				// 다음 1바이트를 만나면 테스트 종료
				break;
			}
		}
		if(nByteCount%2!=0)
		{
			//바이트 카운트가 홀수면 잘못 잘린거다.
			--nNextLine;
		}

		nReturnHeight	=	MakeTextList( &szBuffer[ nNextLine ] , dwColor , nHeight+nControlHeight , nIndex, questTID );
	}

	return nReturnHeight;
}

//-----------------------------------------------------------------------
//


BOOL AgcUIQuestText::CBSelectButtonEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcUIQuestText * pThis = static_cast< AgcUIQuestText* >(pClass);

	if( !pThis )
		return FALSE;

	bool select = false;

	int i=0;

	for( SelectButtons::iterator iter = pThis->selectButtons_.begin(); iter != pThis->selectButtons_.end(); ++iter )
	{
		select = false;

		if( *iter == pButton )
		{
			select = true;
			pThis->selectButtonIdx_ = i;
		}

		((AcUIEventButton*)*iter)->SetIsSelected( select );

		++i;
	}

	return TRUE;
}

//-----------------------------------------------------------------------
//
BOOL AgcUIQuestText::CBPositionLinkEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton )
{
	AgcUIQuestText*	pThis = static_cast< AgcUIQuestText* >(pClass);

	if( !pThis  )
		return FALSE;

	AcUIEventButton*	pEventButton	=	static_cast< AcUIEventButton* >(pButton);

	if( !pEventButton )
		return FALSE;

	INT				nPosX			=	(INT)pData1;
	INT				nPosY			=	(INT)pData2;
	INT				nRegionIndex	=	(INT)pEventButton->GetTempData();
	INT				nQuestTID		=	(INT)pEventButton->GetTempData2();

	INT				nID				=	nPosX + nPosY;
	RwV2d			vPos;

	vPos.x		=	(RwReal)nPosX;
	vPos.y		=	(RwReal)nPosY;

	std::string strName = pThis->agcmUIQuest2_->GetQuestPosName( nQuestTID, vPos );

	if( strName.empty() && pEventButton->m_szStaticString )
		strName	=	pEventButton->m_szStaticString;

	nID		+=	strName.length();
	nID		=	abs( nID );

	//pThis->agcmUIQuest2_->AddQuestPositionInfo( nQuestTID, nID );
	
	pThis->agcmMinimap_->DestroyMapPosition( nID );
	pThis->agcmMinimap_->AddMapPosition( strName , nID , &vPos , nRegionIndex );

	pThis->agcmUIWorldmap_->OpenWorldMap( nRegionIndex );

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcUIQuestText::CBQuestRegisterItem( PVOID pData , PVOID pClass , PVOID pCustData )
{
	AgcUIQuestText*	pThis = static_cast< AgcUIQuestText* >(pClass);

	if( !pThis )
		return FALSE;

	AgpdItem * pdItem = static_cast< AgpdItem* >(pData);

	if( !pdItem )
		return FALSE;

	BOOL bNotIID = (BOOL)pCustData;

	eAgpmItemTooltipType tooltipType = (INT32)( pCustData );

	if( tooltipType.check( eAgpmItemTooltipType::quest_grid ) )
	{
		pThis->AddQuestItemInfo( pdItem );
		pThis->OpenQuestItemTooltip( pdItem );
	}

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcUIQuestText::AddPositionLinkEvent( AcUIBase* pcsBase , INT nPosX , INT nPosY , INT nRegionIndex, INT questTID )
{
	AcUIEventButton* button	= dynamic_cast< AcUIEventButton* >( pcsBase );

	if( !button )
		return FALSE;

	button->SetCustomRender( TRUE );
	button->SetCallbackClickEvent( this, CBPositionLinkEvent );
	button->SetData( (PVOID)nPosX , (PVOID)nPosY );
	button->SetTempData( (PVOID)nRegionIndex );
	button->SetTempData2( (PVOID)questTID );

	button->SetCustomBGColor( 0x60ffffff );
	button->SetCustomLineColor( 0xaaffffff );

	return TRUE;
}

//-----------------------------------------------------------------------
//

int AgcUIQuestText::RefreshTextList( char const * msg, AcUIBase* pUIBase, int questTID )
{
	if( !pUIBase || !msg )
		return 0;

	if( ownerUI_ )
		ownerUI_->DeleteChild( questTextUI_);

	ownerUI_ = pUIBase;

	pUIBase->AddChild( questTextUI_);

	ClearTextList(TRUE);

	questTextUI_->MoveWindow( 0 , 0 , pUIBase->w , pUIBase->h );

	INT	nHeight	= MakeTextList( msg , 0xffffffff , 0 , 0, questTID );

	maxScroll_ = nHeight - pUIBase->h;

	questTextUI_->MoveWindow( 0 , 0 , pUIBase->w , nHeight );

	if( textScrollUI_ )
	{
		if( maxScroll_ > 0 )
			textScrollUI_->m_fScrollUnit = _scroll_interval / ((FLOAT)maxScroll_);
		else
			textScrollUI_->m_fScrollUnit = 0;

		textScrollUI_->SetScrollValue( 0.0f );
	}	

	if( questTextUI_)
	{
		_create_functor__( AgcUIQuestText,
			void, func, ( QuestControlInfo * info ),
			{
				if( info )
					this_->questTextUI_->AddChild( info->m_pUIBase );
			}
		);

		std::for_each( questTexts_.begin(), questTexts_.end(), func );
	}

	return nHeight;
}

//-----------------------------------------------------------------------
//

eQuestControlType AgcUIQuestText::CalcEventType( std::string const & strString , QuestControlInfo* pControlInfo , std::string& strName )
{
	if( strString.find( "ITEM" ) != string::npos )
	{

		INT	nCountStartPos	=	strString.find( "::" , 0 );
		nCountStartPos	+=	2;

		INT	nCountLastPos	=	strString.find( "::" , nCountStartPos );
		nCountLastPos	-=	1;

		INT	nTIDStartPos	=	nCountLastPos+3;
		INT	nTIDLastPos	=	strString.find( "::" , nTIDStartPos );
		nTIDLastPos -= 1;

		INT	nUpStartPos	=	nTIDLastPos+3;
		INT	nUpLastPos	=	strString.find( "::" , nUpStartPos );
		nUpLastPos -= 1;

		INT	nSocketStartPos	=	nUpLastPos+3;
		INT	nSocketLastPos	=	strString.find( "::" , nSocketStartPos );
		nSocketLastPos	-=	1;

		INT	nBoundStartPos	=	nSocketLastPos+3;
		INT	nBoundLastPos	=	strString.find( "::" , nBoundStartPos );
		nBoundLastPos	-=	1;

		INT	nSelectStartPos	=	nBoundLastPos+3;
		INT	nSelectLastPos	=	strString.find( ">" , nSelectStartPos );
		nSelectLastPos -= 1;

		string	strCount		=	strString.substr( nCountStartPos , nCountLastPos-nCountStartPos+1 );
		string	strTID			=	strString.substr( nTIDStartPos , nTIDLastPos-nTIDStartPos+1 );
		string	strUpgrade		=	strString.substr( nUpStartPos , nUpLastPos-nUpStartPos+1 );
		string	strSocket		=	strString.substr( nSocketStartPos , nSocketLastPos-nSocketStartPos+1 );
		string	strBound		=	strString.substr( nBoundStartPos , nBoundLastPos-nBoundStartPos+1 );
		string	strSelect		=	strString.substr( nSelectStartPos , nSelectLastPos-nSelectStartPos+1 );

		pControlInfo->m_nCount		=	atoi( strCount.c_str() );
		pControlInfo->m_nUpgrade	=	atoi( strUpgrade.c_str() );
		pControlInfo->m_nSocket		=	atoi( strSocket.c_str() );
		pControlInfo->m_nTID		=	atoi( strTID.c_str() );
		pControlInfo->m_nBoundType	=	atoi( strBound.c_str() );
		pControlInfo->m_bSelect		=	atoi( strSelect.c_str() );

		return E_QUEST_CONTROL_ITEM;
	}

	else if( strString.find( "POS" ) != string::npos )
	{
		INT	nRegionStartPos	=	strString.find( "::" , 0 );
		nRegionStartPos	+=	2;

		INT	nRegionLastPos	=	strString.find( "::" , nRegionStartPos );
		nRegionLastPos	-=	1;

		INT	nXStartPos	=	nRegionLastPos+3;
		INT	nXLastPos	=	strString.find( "::" , nXStartPos );
		nXLastPos	-=	1;

		INT	nYStartPos	=	nXLastPos+3;
		INT	nYLastPos	=	strString.find( "::" , nYStartPos );
		nYLastPos -= 1;

		INT	nNameStartPos	=	nYLastPos+3;
		INT	nNameLastPos	=	strString.find( ">" , nNameStartPos );
		nNameLastPos -= 1;

		string	strRegion	=	strString.substr( nRegionStartPos , nRegionLastPos-nRegionStartPos+1 );
		string	strPosX		=	strString.substr( nXStartPos , nXLastPos-nXStartPos+1 );
		string	strPosY		=	strString.substr( nYStartPos , nYLastPos-nYStartPos+1 );
		string	strPosName	=	strString.substr( nNameStartPos , nNameLastPos-nNameStartPos+1 );

		pControlInfo->m_nRegionIndex	=	atoi( strRegion.c_str() );
		pControlInfo->m_nPosX			=	atoi( strPosX.c_str() );
		pControlInfo->m_nPosY			=	atoi( strPosY.c_str() );
		strName							=	strPosName;

		return E_QUEST_CONTROL_POSITION_LINK;
	}

	return E_QUEST_CONTROL_TEXT;
}

//-----------------------------------------------------------------------
//

DWORD AgcUIQuestText::HexStringToDWORD( const char* szHexString )
{
	if( !szHexString )
		return 0;

	DWORD	dwValue		=	0;
	DWORD	dwTotal		=	0;
	DWORD	dwMulti		=	0;
	INT		nLength		=	strlen( szHexString );

	for( INT i = nLength-1 ; i >= 0 ; --i )
	{
		dwMulti		=	1;

		// Ascii Code 의 값을 빼서 숫자로 만든다
		if( szHexString[ i ] >= 'A' && szHexString[ i ] <= 'F' )
			dwValue		=	szHexString[ i ] - 55;

		else if( szHexString[ i ] >= 'a' && szHexString[ i ] <= 'f' )
			dwValue		=	szHexString[ i ] - 87;

		else if( szHexString[ i ] >= '0' && szHexString[ i ] <= '9' )
			dwValue		=	szHexString[ i ] - 48;

		// 승을 구해준다
		for( INT k = 0 ; k < nLength - 1 - i  ; ++k )
			dwMulti	*= 16;

		dwTotal +=	( dwValue * dwMulti );
	}

	return dwTotal;
}

//-----------------------------------------------------------------------
//

BOOL AgcUIQuestText::AddClickSelectAction( AcUIBase* pcsBase )
{
	AcUIEventButton* button	= dynamic_cast< AcUIEventButton* >(pcsBase);
	
	if( !button )
		return FALSE;

	button->SetCustomRender( TRUE );
	button->SetCallbackClickEvent( this, CBSelectButtonEvent );

	button->SetCustomBGColor( 0x30ffffff);
	button->SetCustomLineColor( 0x90ffffff );

	selectButtons_.push_back( button );

	return TRUE;
}

//-----------------------------------------------------------------------
//

VOID AgcUIQuestText::CloseQuestItemTooltip( VOID )
{
	if( agcmUIItem_ )
		agcmUIItem_->CloseToolTip();
}

//-----------------------------------------------------------------------
//

void AgcUIQuestText::ClearClickSelectButton()
{
	selectButtons_.clear();
	selectButtonIdx_ = -1;
}

//-----------------------------------------------------------------------
//

void AgcUIQuestText::ClearTextList( BOOL bDelData )
{
	ClearClickSelectButton();

	if( questTextUI_ )
	{
		questTextUI_->AllDeleteChild( FALSE , FALSE );
		questTextUI_->UpdateChildWindow();
	}

	if( bDelData )
	{
		_create_functor__( void,
			void, func, ( QuestControlInfo * questControlInfo ),
			{
				if( questControlInfo ) {
					DEF_SAFEDELETE( questControlInfo->m_pUIBase );
					delete questControlInfo;
				}
			}
		);

		std::for_each( questTexts_.begin(), questTexts_.end(), func );

		questTexts_.clear();
	}
}

//-----------------------------------------------------------------------
//

void AgcUIQuestText::SetAlpha( float * alpha )
{
	for( QuestTexts::iterator iter = questTexts_.begin(); iter != questTexts_.end(); ++iter )
	{
		if( *iter && (*iter)->m_pUIBase )
		{
			(*iter)->m_pUIBase->m_pfAlpha = alpha;
		}
	}
}

//-----------------------------------------------------------------------