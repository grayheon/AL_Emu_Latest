#pragma once

#include "ApDefine.h"
#include <list>
#include <map>
#include <memory>
#include <string>

//-----------------------------------------------------------------------
//

class AgpdItem;
class AgpmItem;
class AgcdUI;
class AcUIBase;
class AcUIScroll;
class AgcmMinimap;
class AgcmUICombine;
class AgcmUIWorldmap;
class AgcmQuestJournal;
class AgcmUIQuestJournal;
class AgcmUINotice;
class AgcmItem;
class AgcmUIItem;
class AgcmUIManager2;
class AgcmItem;
class AgcmUIItem;
class AgcmCharacter;
class AgcmFont;
class AgcUIScroll;
class AcUIToolTip;
class AgcmUIQuest2;

//-----------------------------------------------------------------------
//

enum	eQuestControlType
{
	E_QUEST_CONTROL_TEXT			,
	E_QUEST_CONTROL_ITEM			,
	E_QUEST_CONTROL_COLOR			,
	E_QUEST_CONTROL_POSITION_LINK	,
};

//-----------------------------------------------------------------------
//

struct QuestControlInfo
	:	ApMemory< QuestControlInfo , 1000 >
{
	AcUIBase*				m_pUIBase;
	eQuestControlType		m_eControlType;

	// 아이템은 컨트롤을 두개 사용한다...
	AcUIBase*				m_pUIButton;
	INT						m_nCount;
	INT						m_nUpgrade;
	INT						m_nSocket;
	INT						m_nTID;
	INT						m_nBoundType;
	BOOL					m_bSelect;

	INT						m_nPosX;
	INT						m_nPosY;
	INT						m_nRegionIndex;
};

//-----------------------------------------------------------------------
//

class AgcUIQuestText
{
public:
	AgcUIQuestText();
	virtual ~AgcUIQuestText();

	void															Init();
	VOID															ClearTextList( BOOL bDelData = FALSE );
	int																RefreshTextList( char const * msg, AcUIBase* pUIBase, int questTID = 0 );
	void															SetScrollControl( AcUIScroll * scrollUI );
	int																SelectedButtonIdx() { return selectButtonIdx_; }
	void															SetAlpha( float * alpha );
	void															SetLineCheckByWord( bool enable ) { lineCheckByWord_ = enable; }

protected:
	typedef std::list< QuestControlInfo* >							QuestTexts;
	typedef std::list< AcUIBase * >									SelectButtons;
	typedef std::map< int, AgpdItem * >								QuestItemInfos;
	typedef std::list< INT >										QuestPositionInfoIDs;

	AcUIBase *														ownerUI_;
	AgcmCharacter *													agcmCharacter_;
	QuestTexts														questTexts_;
	SelectButtons													selectButtons_;
	int																selectButtonIdx_;
	AcUIBase*														questTextUI_;
	AcUIScroll *													textScrollUI_;
	QuestItemInfos													questItemInfos_;
	AgcmMinimap	*													agcmMinimap_;
// 	AgcmUICombine *													agcmUIConbine_;
	AgcmUIWorldmap *												agcmUIWorldmap_;
	AgcmQuestJournal *												agcmQuestJournal_;
	AgcmUIQuestJournal *											agcmUIQuestJournal_;
// 	AgcmUINotice *													agcmUINotice_;
	AgpmItem *														agpmItem_;
	AgcmItem *														agcmItem_;
	AgcmUIItem *													agcmUIItem_;
	AgcmUIManager2 *												agcmUIManager_;
	AgcmFont *														agcmFont_;
	AgcmUIQuest2*													agcmUIQuest2_;

	int																maxScroll_;

	bool															lineCheckByWord_;

	static int const												_font_size = 18;
	static int const												_scroll_interval = 10;

	BOOL															AddQuestItemInfo( AgpdItem* pdItem );
	BOOL															DelQuestItemInfo( AgpdItem* pdItem );
	void															ClearQuestItemInfo();

	void															OpenQuestItemTooltip( AgpdItem* pdItem );
	void															CloseQuestItemTooltip();

	AgpdItem*														GetQuestItemInfo( int tid );
	VOID															ClearClickSelectButton( VOID );
	INT																MakeTextList(  CHAR const* szBuffer , DWORD dwColor , INT nHeight , INT nIndex, INT questTID );
	BOOL															AddClickSelectAction( AcUIBase* pcsBase  );
	BOOL															AddPositionLinkEvent( AcUIBase* pcsBase , INT nPosX , INT nPosY , INT nRegionIndex, INT questTID );
	BOOL															AddTooltipAction( AcUIBase* pcsBase , INT nTID );
	eQuestControlType												CalcEventType( std::string const & strString , QuestControlInfo* pControlInfo , std::string& strName  );
	DWORD															HexStringToDWORD( const char* szHexString );

	static BOOL														CBOpenQuestItemTooltip( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL														CBCloseQuestItemTooltip( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL														CBSelectButtonEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );	
	static BOOL														CBPositionLinkEvent( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pButton );
	static BOOL														CBQuestRegisterItem( PVOID pData , PVOID pClass , PVOID pCustData );
	static BOOL														CBRefreshTextScroll( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 );
	static BOOL														CBMouseWheel( PVOID pClass , PVOID pData1 , PVOID pData2 , PVOID pData3 );

	void operator=( AgcUIQuestText const & other );
	AgcUIQuestText( AgcUIQuestText const & other );

private:
	std::string														SliceLineByWord( std::string const & str, int totalWidth );
	INT32															InsertQuestControl( INT32 questTID, QuestControlInfo * pControlInfo, std::string const & strControlName, INT32 nTotalStringWidth, DWORD dwColor, INT32 nHeight, INT32 nStringWidth );
};

//-----------------------------------------------------------------------