#include "AgpmEventGuide.h"
#include "AgcmUIGuideMessage.h"
#include <sstream>
#include "mapped_memory.h"
#include "AuMD5Encrypt.h"
#include "AgpmCharacter.h"
#include "AgcmCharacter.h"
#include "AgcmUIManager2.h"
#include "AgpmFactors.h"
//#include "AgcUIQuestText.h"
#include "AgcmUIControl.h"
#include "AgcmEventManager.h"
#include "AgcmSound.h"

//-----------------------------------------------------------------------
//
const INT32 GUIDE_TEXT_BUFFER_SIZE = 2000;
static char * _guideUIName = "UI_GuideMessage";

//-----------------------------------------------------------------------
//

AgcmUIGuideMessage::AgcmUIGuideMessage()
	: agpmCharacter_(0)
	, agcmCharacter_(0)
	, uiManager_(0)
//	, questText_(new AgcUIQuestText)
	, guideWindow_(0)
	, openButton_(0)
	, textBox_(0)
	, scroll_(0)
	, selMsg_( closedMessages_.end() ) 
	, selObject_(0)
	, objectMsg_(objectGuideMessages_.end())
{
	SetModuleName("AgcmUIGuideMessage");
	EnableIdle(TRUE);
	SetMaxIdleEvent( 10 );
}

//-----------------------------------------------------------------------
//

AgcmUIGuideMessage::~AgcmUIGuideMessage()
{}

//-----------------------------------------------------------------------
//

AgcmUIGuideMessage::cache_ptr AgcmUIGuideMessage::LoadFile( char const * filename, bool decrypt )
{
	if( !filename || strlen(filename) == 0 )
		return cache_ptr();

	BOOL	bResult = TRUE;
	size_t cache_size;
	cache_ptr cache;

	CHAR	szPath[ MAX_PATH ];
	sprintf_s( szPath, MAX_PATH, "%s", filename);

	AuPackingManager* pPackingManager = AuPackingManager::GetSingletonPtr();
	if( !pPackingManager )
		bResult = FALSE;

	ApdFile pdFile;
	if( !pPackingManager->OpenFile( szPath, &pdFile ) )
	{
		bResult = FALSE;
	}
	cache_size = pPackingManager->GetFileSize( &pdFile );
	if( cache_size <= 0 )
	{
		pPackingManager->CloseFile( &pdFile );
		bResult = FALSE;
	}

	char* pBuffer = new char[ cache_size ];
	int nReadSize = pPackingManager->ReadFile( pBuffer, cache_size, &pdFile );
	pPackingManager->CloseFile( &pdFile );

	if( nReadSize != cache_size )
	{
		delete [] pBuffer;
		pBuffer = NULL;
		bResult = FALSE;
	}
	if( bResult )
	{
		cache = cache_ptr( new char[ cache_size + 1 ] );
		memcpy_s( cache.get(), cache_size, pBuffer, cache_size );
	}
	else
	{
		umtl::mapped_file file( filename );

		umtl::mapped_memory fileMem = umtl::mapped_memory::access( file, 0, file.size() );

		cache_size = fileMem.size();
		
		cache =  cache_ptr( new char[ cache_size + 1 ] );

		memcpy_s( cache.get(), cache_size, fileMem.get(), fileMem.size() );	
	}

	if( decrypt )
	{
		AuMD5Encrypt md5;
		md5.DecryptString(MD5_HASH_KEY_STRING, cache.get(), cache_size);
	}

	cache.get()[ cache_size ] = 0;

	return cache;
}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::LoadLevelUpGuideMessage( bool decrypt )
{
	levelUpMessageDic_.clear();

	cache_ptr cache = LoadFile( "ini/LevelUpGuideMessage.txt", decrypt );

	std::stringstream str_stream( cache.get() );

	std::string line;
	std::string attr;

	std::getline( str_stream, line ); // 첫번째 라인은 무시

	while( std::getline( str_stream, line ) )
	{
		std::stringstream line_stream( line );

		// 첫번째는 종족
		std::getline( line_stream, attr, '\t' );
		AuRaceType race = GetRace( attr.c_str() );

		// 두번째는 클래스
		std::getline( line_stream, attr, '\t' );
		AuCharClassType charClass = GetClass( attr.c_str() );

		// 세번째는 레벨
		std::getline( line_stream, attr, '\t' );
		int level = atoi( attr.c_str() );

		// 네번째는 메세지
		std::getline( line_stream, attr, '\n' );

		if( !attr.empty() )
			attr.erase( attr.size() - 1 );

		GuideMessage & lvMsg = levelUpMessageDic_[race][charClass][level];

		lvMsg.id = level;
		lvMsg.msg = attr;
	}
}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::LoadObjectGuideMessage( bool decrypt )
{
	objectGuideMessages_.clear();

	cache_ptr cache = LoadFile( "ini/ObjectGuideMessage.txt", decrypt );

	std::stringstream str_stream( cache.get() );

	std::string line;
	std::string attr;

	std::getline( str_stream, line ); // 첫번째 라인은 무시

	while( std::getline( str_stream, line ) )
	{
		std::stringstream line_stream( line );

		// 첫번째는 아이디
		std::getline( line_stream, attr, '\t' );
		int id = atoi( attr.c_str() );

		//두번째 배경이미지
		std::getline( line_stream, attr, '\t' );

		size_t bgStart = attr.find( "bg(" );
		int iBackImage = 0;
		if( bgStart == 0 && attr.size() > 4 )
		{
			size_t bgEnd = attr.find( ")" );

			if( bgEnd != std::string::npos )
			{
				iBackImage = atoi( attr.substr( bgStart+3, bgEnd-(bgStart+3) ).c_str() );
			}
		}


		// 두번째는 메세지
		std::getline( line_stream, attr, '\n' );

		if( !attr.empty() && attr[ attr.size() - 1 ] == '\r' )
			attr.erase( attr.size() - 1 );

		GuideMessage & lvMsg = objectGuideMessages_[id];

		lvMsg.id = id;
		lvMsg.iBackGroundImageId = iBackImage;
		lvMsg.msg = attr;
	}
}

//-----------------------------------------------------------------------
//

AuRaceType AgcmUIGuideMessage::GetRace( char const * str )
{
	if( stricmp( str, "human" ) == 0 )
		return AURACE_TYPE_HUMAN;

	if( stricmp( str, "orc" ) == 0 )
		return AURACE_TYPE_ORC;

	if( stricmp( str, "moonelf" ) == 0 )
		return AURACE_TYPE_MOONELF;

	if( stricmp( str, "dragonscion" ) == 0 )
		return AURACE_TYPE_DRAGONSCION;

	return AURACE_TYPE_NONE;
}

//-----------------------------------------------------------------------
//

AuCharClassType AgcmUIGuideMessage::GetClass( char const * str )
{
	if( stricmp( str, "knight" ) == 0 )
		return AUCHARCLASS_TYPE_KNIGHT;

	if( stricmp( str, "mage" ) == 0 )
		return AUCHARCLASS_TYPE_MAGE;

	if( stricmp( str, "ranger" ) == 0 )
		return AUCHARCLASS_TYPE_RANGER;

	if( stricmp( str, "scion" ) == 0 )
		return AUCHARCLASS_TYPE_SCION;

	return AUCHARCLASS_TYPE_NONE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::OnInit()
{
#if (defined(_BIN_EXEC_)||defined(USE_MFC))
	LoadLevelUpGuideMessage(false);
	LoadObjectGuideMessage(false);
#else 
	LoadLevelUpGuideMessage(true);
	LoadObjectGuideMessage(true);
#endif

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::OnAddModule()
{
	agcmCharacter_		=	(AgcmCharacter *)GetModule("AgcmCharacter");
	agpmCharacter_		=	(AgpmCharacter *)GetModule("AgpmCharacter");
	uiManager_			=	(AgcmUIManager2 *)GetModule("AgcmUIManager2");
	agpmFactors_		=	(AgpmFactors *)GetModule("AgpmFactors");
	agcmEventManager_	=	(AgcmEventManager *)GetModule("AgcmEventManager");

	if( !agcmCharacter_ || !agpmCharacter_ || !uiManager_ || !agpmFactors_ || !agcmEventManager_ )
		return FALSE;

	agcmCharacter_->SetCallbackSetSelfCharacter( CallBack_SelectCharacter, this );
	agpmCharacter_->SetCallbackUpdateLevel( CallBack_CharacterLevelup, this );
	agpmCharacter_->SetCallbackUpdatePosition( CallBack_CharacterMove, this );

	if (!AddEvents())
		return FALSE;
	if (!AddFunctions())
		return FALSE;
	if (!AddDisplay())
		return FALSE;
	if (!AddUserDatas())
		return FALSE;

	if(!agcmEventManager_->RegisterEventCallback(APDEVENT_FUNCTION_GUIDE, CallBack_RequestObjectGuide, this))
		return FALSE;

	return TRUE;
}


//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::InitMessage()
{
	int level = GetSelfCharacterLevel();

	if( level == 0 )
		return;

	guideWindow_ = uiManager_->GetUI( _guideUIName );

	AgcdUIControl *pControl = uiManager_->GetControl( uiManager_->GetUI("UI_SystemMessageGrid"), "GuideMainBtn" );
	if(pControl)
	{
		openButton_ = (AcUIButton *)pControl->m_pcsBase;
		openButton_->m_Property.bVisible = TRUE;
	}

	if( guideWindow_ )
	{
		guideWindow_->m_bMainUI = FALSE;
		textBox_ = uiManager_->GetControl( guideWindow_, "GuideMessage_TextBox" );
		scroll_ = uiManager_->GetControl( guideWindow_, "GuideMessage_Scroll" );
	}

	closedMessages_.clear();

	GuideMessages & curCharLvMsg = levelUpMessageDic_[ GetSelfRace() ][ GetSelfClass() ];
	GuideMessages::iterator first = curCharLvMsg.begin();
	GuideMessages::iterator last = curCharLvMsg.lower_bound( level+1 );

	closedMessages_.insert( first, last );

	selMsg_ = closedMessages_.empty() ? closedMessages_.end() : --closedMessages_.end();

	//questText_->Init();

	//if( guideWindow_ && guideWindow_->m_pcsUIWindow )
	//{
	//	//guideWindow_->m_pcsUIWindow->m_Property.bClipping = TRUE;

	//	if( scroll_ )
	//		questText_->SetScrollControl( (AcUIScroll*)scroll_->m_pcsBase );

	//	SelectMessage();

	//	CloseUI();
	//}
}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::UpdateMessage()
{
	if( !uiManager_ )
		return;

	int level = GetSelfCharacterLevel();

	size_t beforeCount = closedMessages_.size();

	GuideMessages & curCharLvMsg = levelUpMessageDic_[ GetSelfRace() ][ GetSelfClass() ];
	GuideMessages::iterator first = curCharLvMsg.begin();
	GuideMessages::iterator last = curCharLvMsg.lower_bound( level+1 );

	closedMessages_.insert( first, last );

	size_t afterCount = closedMessages_.size();

	if( afterCount > beforeCount )
	{
		if( openButton_ )
			openButton_->StartAnimation();

		if( !closedMessages_.empty() )
			selMsg_ = --closedMessages_.end();
		else
			selMsg_ = closedMessages_.end();

		SelectMessage();
		uiManager_->SetUserDataRefresh(m_pstUDGuideText);

		//ShowLevelUpGuideUI();
	}
	else
	{
		selMsg_ = closedMessages_.empty() ? closedMessages_.end() : --closedMessages_.end();
	}
}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::SelectMessage()
{
	//if( questText_.get() && textBox_ )
	//{
	//	questText_->RefreshTextList( selMsg_ == closedMessages_.end() ? "" : selMsg_->second.msg.c_str(), textBox_->m_pcsBase );
	//}
}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::PrevMessage()
{
	if( closedMessages_.empty() )
	{
		selMsg_ = closedMessages_.end();
	}
	else
	{
		if( selMsg_ == closedMessages_.end() )
			selMsg_ = closedMessages_.begin();
		else if( selMsg_ != closedMessages_.begin() )
			--selMsg_;
	}

	SelectMessage();
	uiManager_->SetUserDataRefresh(m_pstUDGuideText);

}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::NextMessage()
{
	if( closedMessages_.empty() )
	{
		selMsg_ = closedMessages_.end();
	}
	else
	{
		GuideMessages::iterator lastMsg = --closedMessages_.end();

		if( selMsg_ == closedMessages_.end() )
			selMsg_ = lastMsg;
		else if( selMsg_ != lastMsg )
			++selMsg_;
	}

	SelectMessage();
	uiManager_->SetUserDataRefresh(m_pstUDGuideText);
}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::OpenLevelUpGuideUI()
{
	if( guideWindow_ )
	{
		AgcUIWindow * window = guideWindow_->m_pcsUIWindow;

		if( window )
		{
			window->SetRednerTexture( (int)GetSelfRace() );
			window->MoveWindow( window->x, (uiManager_->m_lWindowHeight) - (window->h) - 64, window->w, window->h );
		}

		SelectMessage();
		uiManager_->SetUserDataRefresh(m_pstUDGuideText);

		uiManager_->OpenUI( guideWindow_,0,0,TRUE );

		AgcmSound * agcmSound = (AgcmSound*)g_pEngine->GetModule( "AgcmSound" );
		if( agcmSound )
			agcmSound->PlaySampleSound( "Sound\\UI\\U_WM_US.ogg" );

		selObject_ = 0;
	}
}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::OpenObjectGuideUI( ApdObject * object, int msgID)
{
	selObject_ = object;

	if( !CheckObjectRange() )
	{
		selObject_ = 0;
		return;
	}

	GuideMessages::iterator msg = objectGuideMessages_.find( msgID );

	if( msg == objectGuideMessages_.end() )
		return;


	objectMsg_ = msg;
	uiManager_->SetUserDataRefresh(m_pstUDGuideText);

	//if( questText_.get() )
	//{
	//	if( scroll_ )
	//		questText_->SetScrollControl( (AcUIScroll*)scroll_->m_pcsBase );

	//	if( textBox_ )
	//		questText_->RefreshTextList( msgPtr, textBox_->m_pcsBase );
	//}

	if( guideWindow_ )
	{
		AgcUIWindow * window = guideWindow_->m_pcsUIWindow;

		if( window )
		{
			window->SetRednerTexture( msg->second.iBackGroundImageId );
			window->MoveWindow( window->x, (uiManager_->m_lWindowHeight) - (window->h) - 64, window->w, window->h );
		}

		uiManager_->OpenUI( guideWindow_,0,0,TRUE );

		AgcmSound * agcmSound = (AgcmSound*)g_pEngine->GetModule( "AgcmSound" );
		if( agcmSound )
			agcmSound->PlaySampleSound( "Sound\\UI\\U_WM_US.WAV" );
	}
}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::ShowLevelUpGuideUI()
{
	OpenLevelUpGuideUI();

	INT32 eventTime = GetClockCount() + 2500;
	AddIdleEvent(eventTime, 0 , this, CallBack_Hide, 0);
}

//-----------------------------------------------------------------------
//

void AgcmUIGuideMessage::CloseUI()
{
	selObject_ = 0;

	if( !uiManager_ || !guideWindow_ )
		return;

	uiManager_->CloseUI( guideWindow_ );
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::CallBack_Hide(INT32 lCID, PVOID pClass, UINT32 ulClockCount, PVOID pvData)
{
	AgcmUIGuideMessage*	_this = static_cast< AgcmUIGuideMessage* >(pClass);

	if( !_this || !_this->uiManager_ || !_this->guideWindow_ )
		return FALSE;

	_this->CloseUI();

	return TRUE;
}

//-----------------------------------------------------------------------
//

int AgcmUIGuideMessage::GetSelfCharacterLevel()
{
	if( !agcmCharacter_ || !agpmFactors_ )
		return 0;

	AgpdCharacter * selfCharcter = agcmCharacter_->GetSelfCharacter();

	if( !selfCharcter )
		return 0;

	AgpdFactor	*factorResult = (AgpdFactor *) agpmFactors_->GetFactor(&selfCharcter->m_csFactor, AGPD_FACTORS_TYPE_RESULT);

	if( !factorResult )
		return 0;

	int level = 0;

	agpmFactors_->GetValue( factorResult, &level, AGPD_FACTORS_TYPE_CHAR_STATUS, AGPD_FACTORS_CHARSTATUS_TYPE_LEVEL );

	return level;
}

//-----------------------------------------------------------------------
//

AuRaceType AgcmUIGuideMessage::GetSelfRace()
{
	if( !agpmCharacter_ || !agcmCharacter_ )
		return AURACE_TYPE_NONE;

	return (AuRaceType)agpmCharacter_->GetCharacterRace( agcmCharacter_->GetSelfCharacter() ).detail.nRace;
}

//-----------------------------------------------------------------------
//

AuCharClassType AgcmUIGuideMessage::GetSelfClass()
{
	if( !agpmCharacter_ || !agcmCharacter_ )
		return AUCHARCLASS_TYPE_NONE;

	return (AuCharClassType)agpmCharacter_->GetCharacterRace( agcmCharacter_->GetSelfCharacter() ).detail.nClass;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::CallBack_SelectCharacter( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if(!pClass)
		return FALSE;

	((AgcmUIGuideMessage*)pClass)->InitMessage();

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::CallBack_CharacterLevelup( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if(!pClass || !pData)
		return FALSE;

	AgcmUIGuideMessage * _this = (AgcmUIGuideMessage*)pClass;
	AgpdCharacter * target = (AgpdCharacter *)pData;

	if( !_this->agcmCharacter_ || _this->agcmCharacter_->GetSelfCharacter() != target )
		return TRUE;

	_this->UpdateMessage();

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::CallBack_CharacterMove( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pClass || !pData )
		return FALSE;

	AgcmUIGuideMessage * _this = (AgcmUIGuideMessage*)pClass;
	AgpdCharacter * target = (AgpdCharacter *)pData;

	if( !_this->agcmCharacter_ || _this->agcmCharacter_->GetSelfCharacter() != target )
		return TRUE;

	if( _this->selObject_ && !_this->CheckObjectRange() )
	{
		_this->CloseUI();
	}

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::CallBack_Close(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	if (!pClass )
		return FALSE;

	AgcmUIGuideMessage	* _this	= (AgcmUIGuideMessage *) pClass;

	_this->CloseUI();

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::CallBack_NextMsg(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIGuideMessage	* _this	= (AgcmUIGuideMessage *) pClass;

	if( !_this )
		return FALSE;

	if( _this->selObject_ )
		return TRUE;

	_this->NextMessage();

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::CallBack_PrevMsg(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIGuideMessage	* _this	= (AgcmUIGuideMessage *) pClass;

	if( !_this )
		return FALSE;

	if( _this->selObject_ )
		return TRUE;

	_this->PrevMessage();

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::CallBack_Open(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase *pcsTarget, AgcdUIControl *pcsSourceControl)
{
	AgcmUIGuideMessage	* _this	= (AgcmUIGuideMessage *) pClass;

	if( !_this || !_this->guideWindow_ || !_this->guideWindow_->m_pcsUIWindow )
		return FALSE;

	if( _this->guideWindow_->m_pcsUIWindow->m_bOpened )
		_this->CloseUI();
	else
		_this->OpenLevelUpGuideUI();

	if( _this->openButton_ )
		_this->openButton_->StopAnimation();

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::CallBack_RequestObjectGuide( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if( !pData || !pClass )
		return FALSE;

	AgcmUIGuideMessage * _this = static_cast< AgcmUIGuideMessage * >( pClass );
	ApdEvent* event = (ApdEvent*)pData;

	if( !event->m_pvData || !event->m_pcsSource || !_this->agcmCharacter_ )
		return FALSE;

	_this->OpenObjectGuideUI( (ApdObject*)event->m_pcsSource, ((AgpdGuideEventAttachData*)event->m_pvData)->msgID );

	return TRUE;
}

//-----------------------------------------------------------------------
//

bool AgcmUIGuideMessage::CheckObjectRange()
{
	if( !selObject_ || !agcmCharacter_ )
		return false;

	AgpdCharacter * selfChar = agcmCharacter_->GetSelfCharacter();

	if( !selfChar )
		return false;

	AuPOS delta = selObject_->m_stPosition - selfChar->m_stPos;

	static const int _guide_max_use_length = 1600;

	return delta.Length() < _guide_max_use_length;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::AddFunctions()
{
	if( !uiManager_ )
		return FALSE;

	if (!uiManager_->AddFunction(this, "Open_GuideMessage", CallBack_Open, 0))
		return FALSE;

	if (!uiManager_->AddFunction(this, "Close_GuideMessage", CallBack_Close, 0))
		return FALSE;

	if( !uiManager_->AddFunction(this, "NextGuideMessage", CallBack_NextMsg, 0) )
		return FALSE;

	if( !uiManager_->AddFunction(this, "PrevGuideMessage", CallBack_PrevMsg, 0) )
		return FALSE;

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::AddEvents()
{

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::AddUserDatas()
{
	m_pstUDGuideText = uiManager_->AddUserData("GuideMessage_Data", NULL, 0, 0, AGCDUI_USERDATA_TYPE_INT32);
	if (!m_pstUDGuideText)
		return FALSE;

	return TRUE;
}

//-----------------------------------------------------------------------
//

BOOL AgcmUIGuideMessage::AddDisplay()
{
	if (!uiManager_->AddDisplay(this, "GuideMessage_Display", 0, CBDisplayGuideMainText, AGCDUI_USERDATA_TYPE_INT32))
		return FALSE;
	return TRUE;
}

//-----------------------------------------------------------------------

BOOL AgcmUIGuideMessage::CBDisplayGuideMainText(PVOID pClass, PVOID pData, AgcdUIDataType eType, INT32 lID, CHAR *szDisplay, INT32 *plValue, AgcdUIControl *pcsSourceControl)
{
	if (!pClass || eType != AGCDUI_USERDATA_TYPE_INT32)
		return FALSE;

	AgcmUIGuideMessage *pThis = (AgcmUIGuideMessage*)pClass;

	if (pcsSourceControl->m_lType != AcUIBase::TYPE_EDIT) return FALSE;

	AcUIEdit* pUIEdit = (AcUIEdit *) pcsSourceControl->m_pcsBase;

	pUIEdit->SetLineDelimiter("</n>");
	if(!pThis->selObject_)
	{
		pUIEdit->SetText(pThis->selMsg_ == pThis->closedMessages_.end() ? "" : pThis->selMsg_->second.msg.c_str());
	}
	else
	{
		pUIEdit->SetText(pThis->objectMsg_ == pThis->objectGuideMessages_.end() ? "" : pThis->objectMsg_->second.msg.c_str());
	}


	return TRUE;
}

void AgcmUIGuideMessage::HideGuideButton()
{
	if(openButton_) openButton_->m_Property.bVisible = FALSE;
	return;
}