#ifndef		_ACUILIST_H_
#define		_ACUILIST_H_

#include "AcUIBase.h"
#include "AuList.h"
#include "AcUIScroll.h"

#define ACUILIST_LIST_ITEM_STRING_LENGTH					16


// Command Message
enum 
{
	UICM_LIST_SCROLL										= UICM_BASE_MAX_MESSAGE,
	UICM_LIST_MAX_MESSAGE
};

// Command Message
enum 
{
	UICM_LISTITEM_MAX_MESSAGE								= UICM_BASE_MAX_MESSAGE	
};

class AcUIList;
class AcUIListItem;

typedef BOOL ( *AcUIListCallback )( AcUIList* pUIList, AcUIListItem* pUIListItem, PVOID pClass, PVOID pData, INT32* pHeight );

struct AcdUIListItemInfo  // -> 대략 ID개념이 필요함 
{
	CHAR													m_szListItemString[ ACUILIST_LIST_ITEM_STRING_LENGTH ]; 
};

class AcUIList;

/********************************************AcUIListItem**************************************************/
class AcUIListItem : public AcUIBase 
{
public:
	AcUIList*												m_pcsList;
	INT32													m_lItemIndex;
	INT32													m_lListItemWindowHeight;

	AcUIListItem( void )
	{
		m_nType = TYPE_LIST_ITEM;
		m_lListItemWindowHeight = 0;
		m_pcsList =	NULL;
	}

	virtual	VOID			OnWindowRender					( void );

private :
	RwTexture*				_GetTextureByID					( INT32 nTextureID );
};

/***********************************************AcUIList***********************************************/
class AcUIList : public AcUIBase 
{
public:
	AcUIList( void );
	virtual ~AcUIList( void );

public:
	AcUIScroll*												m_pcsScroll;				// List Control에 쓰일 Scroll

	AuList< AcdUIListItemInfo* >							m_listItemInfo;
	AcUIListItem**											m_ppListItem;

	AcUIListCallback										m_pfConstructorCallback;	// ListItem Window Costructor Callback Function 
	PVOID													m_paConstructorClass;		// Pointer of Class that has Constructor callback Fucntion
	PVOID													m_pConstructorData;			// Pointer of Constructor Data

	AcUIListCallback										m_pfDestructorCallback;		// ListItem Window Destructor Callback Function 
	PVOID													m_paDestructorClass;		// Pointer of Class that has Destructor callback Function 
	PVOID													m_pDestructorData;			// Pointer of Destructor Data

	AcUIListCallback										m_pfRefreshCallback;		// ListItem Window Refresh Callback Function 
	PVOID													m_paRefreshClass;			// Pointer of Class that has Refresh callback Function 
	PVOID													m_pRefreshData;				// Pointer of Refresh Data

	INT32													m_lUseListItemInfoStringIndex;	// 현재 사용하고 있는 ListItem Info String 의 Index

	BOOL													m_bStartAtBottom;			// 시작 Item을 밑에서부터 시작?

	INT32													m_lSelectedItemTextureID;	// 선택된 아이템 TextureID
	INT32													m_lSelectedIndex;			// 선택된 아이템

	RsMouseStatus											m_csMouseMoveStatus;
	
	// 설정 관련 
	INT32													m_lListItemWidth;
	INT32													m_lListItemHeight;			// 한 List Item의 Height 
	INT32													m_lListItemStartX;
	INT32													m_lListItemStartY;

	INT32													m_lTotalListItemNum;		// 총 List
	INT32													m_lVisibleListItemRow;		// 눈에 보일 ( 실제 존재하는 ) List Item Row의 수
	INT32													m_lCurrentExistListItemRow;	// m_lVisibleListItemRow만큼 있어야 하지만 Size등의 이유로 적게 존재할수도 있다 
	INT32													m_lListItemStartRow;		// 가장 처음 List Item Window의 Row

	INT32													m_lItemColumn;				// Item의 행
	INT32													m_lTotalItemRow;			// Item의 행에 따른 Row

	BOOL					DeleteAllListItemInfo			( void );

	BOOL					SetListItemWindowTotalNum		( INT32 lTotalNum );
	BOOL					SetListItemWindowVisibleRow		( INT32 lVisibleRow );
	VOID					SetListItemWindowStartRow		( INT32 lStartRow, BOOL bDestroy = FALSE, BOOL bChangeScroll = TRUE );			// 호출될때마다 update 개념 
	VOID					SetListItemWindowMoveInfo		( INT32 lItemWidth, INT32 lItemHeight, INT32 lItemStartX, INT32 lItemStartY );	// Window의 Move Info - 호출될 때 마다 Window들 위치 바뀜
	VOID					GoNextListPage					( void );
	VOID					GoPreListPage					( void );	

	// Call back 관련 
	VOID					SetCallbackConstructor			( AcUIListCallback pfConstructor, PVOID pClass,	PVOID pData );
	VOID					SetCallbackDestructor			( AcUIListCallback pfDestructor, PVOID pClass, PVOID pData );
	VOID					SetCallbackRefresh				( AcUIListCallback pfDestructor, PVOID pClass, PVOID pData );

	// Set ListItem Info 
	BOOL					SetListInfo_String				( CHAR* pszString );
	BOOL					DeleteListInfo_String			( CHAR* pszString );
		
	VOID					SetListItemColumn				( INT32 lColumn );
	VOID					SetSelectedTexture				( INT32 lTextureID );
	VOID					SelectItem						( INT32 lSelectedIndex );

	inline	VOID			SetListScroll					( AcUIScroll *pcsScroll )	{ m_pcsScroll		= pcsScroll;	}

	// virtual function overriding 
	virtual	VOID 			OnClose							( void );
	virtual	BOOL 			OnCommand						( INT32	nID, PVOID pParam );

private:
	AcUIListItem *			NewListItemWidnow				( INT32 lIndex, INT32 lHeightLeft, BOOL *pbFullHeight = NULL );	// List Item Window를 New하면서 AddChild, Constructor 등을 수행한다 
	BOOL					RefreshListItemWindow			( AcUIListItem *pListItem, INT32 lIndex, INT32 lHeightLeft, BOOL *pbFullHeight = NULL );	// List Item Window를 Refresh한다.
	BOOL					DeleteListItemWindow			( AcUIListItem* pListItem );
	BOOL					SetMemoryppListItem				( void );
	CHAR*					GetListItemInfoString			( INT32 lIndex );
	void					MoveListItemWindow				( void );

public:
	AcUIListItem*			GetListItemPointer_Index		( INT32 lIndex );

	virtual AcUIListItem *	OnNewListItem					( INT32 lIndex );
	virtual VOID			RefreshList						( void );
	virtual VOID			OnChangeTotalNum				( void ) { };
	virtual BOOL			IsValidListItem					( AcUIListItem *pListItem, INT32 lIndex	) {	return TRUE; }

	virtual VOID			UpdateStartRowByScroll			( void );
	virtual VOID			UpdateScroll					( void );
	virtual	BOOL			OnMouseWheel					( INT32	lDelta );
	virtual	BOOL			OnMouseMove						( RsMouseStatus *ms	);
	virtual	BOOL			OnLButtonUp						( RsMouseStatus *ms	);
	virtual	VOID			OnWindowRender					( void );
};

#endif // _ACUILIST_H_