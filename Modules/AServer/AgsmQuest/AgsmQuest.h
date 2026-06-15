// AgsmQuest.h: interface for the AgsmQuest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AGSMQUEST_H__29A6F2A8_8D6B_4345_A173_015F8C7C8754__INCLUDED_)
#define AFX_AGSMQUEST_H__29A6F2A8_8D6B_4345_A173_015F8C7C8754__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AgsEngine.h"
#include "AgpmQuest.h"
#include "AgpmLog.h"
#include "AgsmCharacter.h"
#include "AgsmDeath.h"
#include "AgpmItem.h"
#include "AgsmItem.h"
#include "AgsmItemManager.h"
#include "AgpmSkill.h"
#include "AgsmChatting.h"
#include "AgpmItemConvert.h"
#include "AgsmItemConvert.h"
#include "AgsmParty.h"
#include "AuPacket.h"
#include "AuDatabase2.h"

#include "AgpmTitle.h"
#include "AgsmTitle.h"

#if _MSC_VER < 1300
#ifdef	_DEBUG
#pragma comment ( lib , "AgsmQuestD" )
#else
#pragma comment ( lib , "AgsmQuest" )
#endif
#endif

// Quest는 실시간 업데이트가 이루어 지기 때문에 Logout시에 특별한 작업은 필요하지 않다.
enum AgsmQuestDBOperation
{
	AGSMQUEST_DB_CURRENTQUEST_NONE = 0,
	AGSMQUEST_DB_CURRENTQUEST_INSERT,	// 진행중인 퀘스트 추가
	AGSMQUEST_DB_CURRENTQUEST_UPDATE,	// 진행중인 퀘스트 업데이트
	AGSMQUEST_DB_CURRENTQUEST_DELETE,	// 진행중인 퀘스트 완료/취소로 인한 삭제
	AGSMQUEST_DB_DATA_FLAG_UPDATE,		// Flag 데이터 업데이트
	AGSMQUEST_DB_CURRENT_QUEST_JOURNAL_REGISTER, //저널 등록여부 바꿈.
};

enum AgsmQuestCB
{
	AGSMQUEST_CB_CURRENT_QUEST = 0,		// AgpdCurrentQuest 클래스를 pData로 사용
	AGSMQUEST_CB_FLAG,					// AgpdQuest 클래스를 pData로 사용
};

enum AgsmQuestDeathResult
{
	AGSMQUEST_DEATH_NONE	= 0,
	AGSMQUEST_DEATH_FAIL,				// 오류가 있는 데이터 혹은 NULL값을 리턴한 경우
	AGSMQUEST_DEATH_ITEM_ENOUGH,		// 아이템을 충족
	AGSMQUEST_DEATH_MONSTER_INCREMENT,	// 몬스터 카운터 증가
	AGSMQUEST_DEATH_MONSTER_ENOUGH,		// 몬스터 카운터 완료
	AGSMQUEST_DEATH_ITEM_CREATE,		// 아이템 생성
};

class AgsmQuest : public AgsModule
{
public:
	AgpmQuest		*m_pcsAgpmQuest;
	AgpmLog			*m_pcsAgpmLog;
	AgsmCharacter	*m_pcsAgsmCharacter;
	AgpmCharacter	*m_pcsAgpmCharacter;
	AgsmDeath		*m_pcsAgsmDeath;
	AgpmItem		*m_pcsAgpmItem;
	AgsmItem		*m_pcsAgsmItem;
	AgsmItemManager *m_pcsAgsmItemManager;
	AgpmGrid		*m_pcsAgpmGrid;
	AgpmSkill		*m_pcsAgpmSkill;
	AgsmChatting	*m_pcsAgsmChatting;
	AgpmItemConvert	*m_pcsAgpmItemConvert;
	AgsmItemConvert	*m_pcsAgsmItemConvert;
	AgsmParty		*m_pcsAgsmParty;
	AgpmTitle		*m_pcsAgpmTitle;
	AgsmTitle		*m_pcsAgsmTitle;

	MTRand			m_csRand;

private:
	BOOL SetQuestBlock(BYTE *szDestBlock, CHAR *szSrcBlock);

public:
	AgsmQuest();
	virtual ~AgsmQuest();

	BOOL OnAddModule();

	BOOL OnInit();
	BOOL OnDestroy();
	BOOL OnIdle(UINT32 ulClockCount);

	// Callback Functions
	static BOOL CBRequireQuest(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBRequireComplete(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBRequireCancel(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBMonsterDeath(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBChatQuestComplete(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBRequireCheckPoint(PVOID pData, PVOID pClass, PVOID pCustData);
	
	static BOOL CBQuestItemPickupValidCheck(PVOID pData, PVOID pClass, PVOID pCustData);

	static BOOL CBQuestRegisterJournal(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL SendQuestDataAtLogin(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL SendQuestDataAllInfo(PVOID pData, PVOID pClass, PVOID pCustData);

	BOOL SetCallbackDBCurrentQuest(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallbackDBFlag(ApModuleDefaultCallBack pfCallback, PVOID pClass);

	BOOL QuestItemPickupValidCheck(AgpdCharacter *pcsAgpdCharacter, AgpdItem *pcsAgpdItem);
	BOOL CheckPointWork(AgpdCharacter *pcsAgpdCharacter, AgpdQuestGroup *pQuestGroup, AgpdQuest *pcsAgpdQuest, INT32 lQuestTID, INT32 lCheckPointIndex);

	BOOL StartQuest(AgpdCharacter* pcsAgpdCharacter, AgpdQuestTemplate* pQuestTemplate);
	BOOL CompleteQuest(AgpdCharacter* pcsAgpdCharacter, AgpdQuestTemplate* pQuestTemplate, INT32 lSelectItemIndex = -1);

	BOOL RequireQuest(AgpdCharacter* pcsAgpdCharacter, ApdEvent* pcsEvent, INT32 lQuestTID);
	BOOL RequireComplete(AgpdCharacter* pcsAgpdCharacter, ApdEvent* pcsEvent, INT32 lQuestTID, INT32 lSelectItemIndex = -1);
	BOOL RequireCancel(AgpdCharacter* pcsAgpdCharacter, INT32 lQuestTID);
	BOOL RequireCheckPoint(AgpdCharacter* pcsAgpdCharacter, ApdEvent* pcsEvent, INT32 lQuestTID, INT32 lCheckPointIndex);

	BOOL QuestRegisterJournal(AgpdCharacter* pcsAgpdCharacter, INT32 lQuestTID, BOOL bRegisterJournal);

	BOOL QuestResult(AgpdCharacter* pcsAgpdCharacter, AgpdQuestTemplate* pcsQuestTemplate, INT32 lSelectItemIndex = -1);
	BOOL QuestResultMakeItem(AgpdElementItem *pItem, AgpdCharacter *pcsAgpdCharacter);

	BOOL MonsterDeathByCharacter(AgpdCharacter *pcsAttacker, AgpdCharacter *pcsMonster, AgpdParty *pcsParty = NULL, BOOL *pCreated = NULL);
	BOOL MonsterDeathByParty(AgpdParty *pcsParty, AgpdCharacter *pcsMonster);

	AgsmQuestDeathResult MonsterDeath(AgpdCharacter* pcsAttacker, AgpdCharacter* pcsMonster, AgpdElementMonster *pcsQuestMonster, 
									INT32 lCount, INT32 *pItemTID, AgpdParty *pcsParty, BOOL *pCreated);
	//AGSDQUEST_EXPAND_BLOCK
	AgsmQuestDeathResult MonsterDeathCheckCount(AgpdCharacter* pcsAttacker, AgpdCharacter* pcsMonster, AgpdElementMonster *pcsQuestMonster, 
											 INT32 lCount, INT32 *pItemTID, AgpdParty *pcsParty, BOOL *pCreated);
	AgsmQuestDeathResult MonsterDeathCheckItem(AgpdCharacter* pcsAttacker, AgpdCharacter* pcsMonster, AgpdElementMonster *pcsQuestMonster, 
											 INT32 lCount, INT32 *pItemTID, AgpdParty *pcsParty, BOOL *pCreated);
	// Packet Functions
	BOOL SendPacketQuestRequireResult(INT32 lCID, INT32 lQuestTID, BOOL bResult, AgpdQuest* pcsAgpdQuest);
	BOOL SendPacketQuestCompleteResult(INT32 lCID, INT32 lQuestTID, BOOL bResult);
	BOOL SendPacketQuestCancelResult(INT32 lCID, INT32 lQuestID, BOOL bResult);
	BOOL SendPacketInventoryFull(INT32 lCID, INT32 lQuestID);
	BOOL SendPacketQuestInventoryFull(INT32 lCID, INT32 lQuestID);
	BOOL SendPacketUpdateQuest(INT32 lCID, INT32 lQuestTID, INT32 lItemTID, AgpdCurrentQuest* pcsAgpdCurrentQuest);

	// Evaluation Functions
	BOOL EvaluationStartCondition(AgpdCharacter *pcsAgpdCharacter, INT32 lQuestTID);
	BOOL EvaluationCompleteCondition(AgpdCharacter *pcsAgpdCharacter, INT32 lQuestTID);

	// DB 
	BOOL GetSelectQueryCurrentQuest(CHAR* szCharName, CHAR *pstrWorldDBName, CHAR* pstrQuery, INT32 lQueryLength);
	BOOL GetSelectQueryFlag(CHAR* szCharName, CHAR *pstrWorldDBName, CHAR* pstrQuery, INT32 lQueryLength);
	BOOL GetSelectQueryCurrentQuest4(CHAR* pstrQuery, INT32 lQueryLength);
	BOOL GetSelectQueryFlag4(CHAR* pstrQuery, INT32 lQueryLength);

	BOOL GetSelectResultFlag5(AuDatabase2 *pDatabase, AgpdCharacter *pcsCharacter);
	BOOL GetSelectResultCurrentQuest5(AuDatabase2 *pDatabase, AgpdCharacter *pcsCharacter);

	// Test
	BOOL CompleteQuestByChatting(AgpdCharacter* pcsAgpdCharacter, INT32 lQuestTID);
};

#endif // !defined(AFX_AGSMQUEST_H__29A6F2A8_8D6B_4345_A173_015F8C7C8754__INCLUDED_)
