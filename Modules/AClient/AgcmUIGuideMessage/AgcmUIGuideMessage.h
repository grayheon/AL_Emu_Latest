#pragma once

#include "AgcModule.h"
#include "AgcmUIManager2.h"
#include <map>
#include <string>
#include <memory>

class AgpmCharacter;
class AgcmCharacter;
class AgcmUIManager2;
class AgpmFactors;
//class AgcUIQuestText;
class AgcdUIControl;
class AcUIButton;
class AgcdUI;
class AgcmEventManager;
class AgpmEventGuide;
class ApdObject;

class AgcmUIGuideMessage : public AgcModule
{
public:
	struct GuideMessage
	{
		int id;
		int iBackGroundImageId;
		std::string msg;

		GuideMessage() : id(0), iBackGroundImageId(0) {}
		GuideMessage( int id, std::string & msg ) : id(id), msg(msg) {}
	};

	typedef std::map< int, GuideMessage >											GuideMessages;
	typedef std::map< AuRaceType, std::map< AuCharClassType, GuideMessages > >		LevelUpMessageDic;
	typedef std::auto_ptr<char>														cache_ptr;

	AgcmUIGuideMessage();
	virtual ~AgcmUIGuideMessage();

	
	virtual BOOL					OnAddModule();
	virtual BOOL					OnInit();

	void							LoadLevelUpGuideMessage(bool decrypt);
	void							LoadObjectGuideMessage(bool decrypt);

	void							InitMessage();

	GuideMessages &					GetObjectGuideMessages() { return objectGuideMessages_; }

	void							HideGuideButton();


private:
	BOOL							AddFunctions();
	BOOL							AddEvents();
	BOOL							AddUserDatas();
	BOOL							AddDisplay();

	cache_ptr						LoadFile( char const * filename, bool decrypt );

	void							UpdateMessage();

	int								GetSelfCharacterLevel();
	AuRaceType						GetSelfRace();
	AuCharClassType					GetSelfClass();

	AuRaceType						GetRace( char const * str );
	AuCharClassType					GetClass( char const * str );

	void							OpenLevelUpGuideUI();
	void							ShowLevelUpGuideUI();
	void							OpenObjectGuideUI( ApdObject * object, int msgID );
	void							CloseUI();

	void							SelectMessage();
	void							PrevMessage();
	void							NextMessage();

	bool							CheckObjectRange();

	static BOOL						CallBack_SelectCharacter(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL						CallBack_CharacterLevelup(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL						CallBack_Close(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);
	static BOOL						CallBack_Open(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);
	static BOOL						CallBack_Hide(INT32 lCID, PVOID pClass, UINT32 ulClockCount, PVOID pvData);
	static BOOL						CallBack_NextMsg(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);
	static BOOL						CallBack_PrevMsg(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl);
	static BOOL						CallBack_RequestObjectGuide(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL						CallBack_CharacterMove( PVOID pData, PVOID pClass, PVOID pCustData );


	static BOOL						CBDisplayGuideMainText(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl);


	// 레벨업 가이드 메세지 전체 목록
	LevelUpMessageDic levelUpMessageDic_;
	// 오브젝트 가이드 메세지 전체 목록
	GuideMessages objectGuideMessages_;
	// 이미 봤던 레벨업 메세지
	GuideMessages closedMessages_;

	AgpmCharacter *		agpmCharacter_;
	AgcmCharacter *		agcmCharacter_;
	AgcmUIManager2 *	uiManager_;
	AgpmFactors *		agpmFactors_;
	AgcmEventManager *	agcmEventManager_;
	AgpmEventGuide *	agpmEventGuide_;

//	typedef std::auto_ptr< AgcUIQuestText > QuestTextPtr;
//	QuestTextPtr questText_;

	AgcdUI * guideWindow_;
	AcUIButton * openButton_;
	AgcdUIControl * textBox_;
	AgcdUIControl * scroll_;

	GuideMessages::iterator selMsg_;
	GuideMessages::iterator objectMsg_;

	ApdObject * selObject_;

	AgcdUIUserData		*m_pstUDGuideText;
};