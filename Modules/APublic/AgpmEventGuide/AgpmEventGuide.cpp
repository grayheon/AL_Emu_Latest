#include "AgpmEventGuide.h"
#include "ApModuleStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AgpmEventGuide::AgpmEventGuide()
{
	SetModuleName("AgpmEventGuide");

	/*
	SetPacketType(AGPMEVENT_GUIDE_PACKET_TYPE);

	m_csPacket.SetFlagLength(sizeof(INT8));
	m_csPacket.SetFieldType(
		AUTYPE_INT8,		1,			// Operation
		AUTYPE_PACKET,		1,			// Event Base Packet
		AUTYPE_INT32,		1,			// CID
		AUTYPE_INT32,		1,			// Object Range
		AUTYPE_END,			0);

	m_csPacketEventData.SetFlagLength(sizeof(INT8));
	m_csPacketEventData.SetFieldType(
		AUTYPE_INT32,	1,	// Quest Group ID
		AUTYPE_END,		0
		);
		*/

	m_pcsAgpmCharacter = NULL;
	m_pcsApmEventManager = NULL;
}

//-----------------------------------------------------------------------
//

AgpmEventGuide::~AgpmEventGuide() {}

//-----------------------------------------------------------------------
//

BOOL AgpmEventGuide::OnAddModule()
{
	m_pcsAgpmCharacter = (AgpmCharacter*)GetModule("AgpmCharacter");
	m_pcsApmEventManager = (ApmEventManager*)GetModule("ApmEventManager");

	if(!m_pcsAgpmCharacter || !m_pcsApmEventManager)
		return FALSE;

	if (!m_pcsApmEventManager->RegisterEvent(APDEVENT_FUNCTION_GUIDE, CBEventConstructor, CBEventDestructor, NULL, CBStreamWriteEvent, CBStreamReadEvent, this))
		return FALSE;

	//if (!m_pcsApmEventManager->RegisterPacketFunction(CBEmbeddedMakeEventPacket, CBEmbeddedReceiveEventPacket, this, APDEVENT_FUNCTION_GUIDE))
	//	return FALSE;

	return TRUE;
}

/*
BOOL AgpmEventGuide::OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)
{
	if(!pvPacket || nSize < 1)
		return FALSE;

	INT8 cOperation = -1;
	PVOID pvPacketEventBase = NULL;
	INT32 lCID = -1;
	INT32 lRange = -1;

	m_csPacket.GetField(TRUE, pvPacket, nSize,
		&cOperation,
		&pvPacketEventBase,
		&lCID,
		&lRange);

	if(!pvPacketEventBase)
		return FALSE;

	ApdEvent* pcsEvent = m_pcsApmEventManager->GetEventFromBasePacket(pvPacketEventBase);
	if(!pcsEvent)
		return FALSE;

	if(pcsEvent->m_eFunction != APDEVENT_FUNCTION_QUEST)
		return FALSE;

	AgpdCharacter* pcsCharacter = m_pcsAgpmCharacter->GetCharacterLock(lCID);
	if(!pcsCharacter)
		return FALSE;

	if (!pstCheckArg->bReceivedFromServer && m_pcsAgpmCharacter->IsAllBlockStatus(pcsCharacter))
	{
		pcsCharacter->m_Mutex.Release();
		return FALSE;
	}

	switch(cOperation)
	{
	case AGPMEVENT_QUEST_PACKET_REQUEST:
		OnOperationRequest(pcsEvent, pcsCharacter, lRange);
		break;

	case AGPMEVENT_QUEST_PACKET_GRANT:
		OnOperationGrant(pcsEvent, pcsCharacter);
		break;
	}

	pcsCharacter->m_Mutex.Release();

	return TRUE;
}

BOOL AgpmEventGuide::OnOperationRequest(ApdEvent* pcsEvent, AgpdCharacter* pcsCharacter, INT32 lRange)
{
	if(!pcsEvent || !pcsCharacter)
		return FALSE;

	AuPOS stTargetPos;
	memset(&stTargetPos, 0, sizeof(stTargetPos));

	INT32 lMinRange = MIN(lRange, AGPMEVENT_GUIDE_MAX_USE_RANGE);
	if (lMinRange <= 0) lMinRange = AGPMEVENT_GUIDE_MAX_USE_RANGE;

	if(m_pcsApmEventManager->CheckValidRange(pcsEvent, &pcsCharacter->m_stPos, lMinRange, &stTargetPos))
	{
		pcsCharacter->m_stNextAction.m_eActionType = AGPDCHAR_ACTION_TYPE_NONE;

		if(pcsCharacter->m_bMove)
			m_pcsAgpmCharacter->StopCharacter(pcsCharacter, NULL);

		EnumCallback(AGPMEVENT_QUEST_CB_REQUEST, pcsEvent, pcsCharacter);
	}
	else
	{
		// stTargetPos 까지 이동
		pcsCharacter->m_stNextAction.m_bForceAction = FALSE;
		pcsCharacter->m_stNextAction.m_eActionType = AGPDCHAR_ACTION_TYPE_EVENT_QUEST;
		pcsCharacter->m_stNextAction.m_csTargetBase.m_eType = pcsEvent->m_pcsSource->m_eType;
		pcsCharacter->m_stNextAction.m_csTargetBase.m_lID = pcsEvent->m_pcsSource->m_lID;

		m_pcsAgpmCharacter->MoveCharacter(pcsCharacter, &stTargetPos, MD_NODIRECTION, FALSE, TRUE, FALSE, TRUE, FALSE);
	}

	return TRUE;
}

BOOL AgpmEventGuide::OnOperationGrant(ApdEvent* pcsEvent, AgpdCharacter* pcsCharacter)
{
	if(!pcsEvent || !pcsCharacter)
		return FALSE;

	EnumCallback(AGPMEVENT_QUEST_CB_GRANT, pcsEvent, pcsCharacter);

	return TRUE;
}

BOOL AgpmEventGuide::SetCallbackRequest(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMEVENT_GUIDE_CB_REQUEST, pfCallback, pClass);
}

BOOL AgpmEventGuide::SetCallbackGrant(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMEVENT_QUEST_CB_GRANT, pfCallback, pClass);
}

PVOID AgpmEventGuide::MakePacketEventRequest(ApdEvent *pcsEvent, INT32 lCID, INT16 *pnPacketLength, INT32 lRange)
{
	if(!pcsEvent || !pnPacketLength || lCID == AP_INVALID_CID)
		return NULL;

	INT8 cOperation = AGPMEVENT_QUEST_PACKET_REQUEST;

	PVOID pvPacketBase = m_pcsApmEventManager->MakeBasePacket(pcsEvent);
	if(!pvPacketBase)
		return NULL;

	PVOID pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMEVENT_QUEST_PACKET_TYPE,
		&cOperation,
		pvPacketBase,
		&lCID,
		&lRange);

	m_csPacket.FreePacket(pvPacketBase);

	return pvPacket;
}

PVOID AgpmEventGuide::MakePacketEventGrant(ApdEvent *pcsEvent, INT32 lCID, INT16 *pnPacketLength)
{
	if(!pcsEvent || !pnPacketLength || lCID == AP_INVALID_CID)
		return NULL;

	INT8 cOperation = AGPMEVENT_QUEST_PACKET_GRANT;

	PVOID pvPacketBase = m_pcsApmEventManager->MakeBasePacket(pcsEvent);
	if(!pvPacketBase)
		return NULL;

	PVOID pvPacket = m_csPacket.MakePacket(TRUE, pnPacketLength, AGPMEVENT_QUEST_PACKET_TYPE,
		&cOperation,
		pvPacketBase,
		&lCID,
		0);

	m_csPacket.FreePacket(pvPacketBase);

	return pvPacket;

	return 0;
}

BOOL AgpmEventGuide::CBActionGuide(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpdCharacter* pcsCharacter = (AgpdCharacter*)pData;
	AgpmEventGuide* pThis = (AgpmEventGuide*)pClass;
	AgpdCharacterAction* pstAction = (AgpdCharacterAction*)pCustData;

	if (pThis->m_pcsAgpmCharacter->IsAllBlockStatus(pcsCharacter))
		return FALSE;

	pcsCharacter->m_stNextAction.m_eActionType = AGPDCHAR_ACTION_TYPE_NONE;

	if(pcsCharacter->m_bMove)
		pThis->m_pcsAgpmCharacter->StopCharacter(pcsCharacter, NULL);

	ApdEvent* pcsEvent = pThis->m_pcsApmEventManager->GetEvent(pstAction->m_csTargetBase.m_eType, pstAction->m_csTargetBase.m_lID, APDEVENT_FUNCTION_GUIDE);

	pThis->EnumCallback(AGPMEVENT_GUIDE_CB_REQUEST, pcsEvent, pcsCharacter);

	return TRUE;
}

*/

BOOL AgpmEventGuide::CBStreamWriteEvent(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	ApdEvent		*pcsEvent			= (ApdEvent *)			pData;
	AgpmEventGuide	*pThis				= (AgpmEventGuide *)	pClass;
	ApModuleStream	*pStream			= (ApModuleStream *)	pCustData;

	AgpdGuideEventAttachData	*pcsAttachData		= (AgpdGuideEventAttachData *)	pcsEvent->m_pvData;
	if (!pcsAttachData)
		return FALSE;

	if (!pStream->WriteValue(AGPMEVENT_GUIDE_STREAM_NAME_GROUP_ID, (INT32) pcsAttachData->msgID))
	{
		OutputDebugString("AgpmEventSkillMaster::CBStreamWriteEvent() Error (1) !!!\n");
		return FALSE;
	}

	if (!pStream->WriteValue(AGPMEVENT_GUIDE_STREAM_NAME_EVENT_END, 0))
		return FALSE;

	return TRUE;
}

BOOL AgpmEventGuide::CBStreamReadEvent(PVOID pData, PVOID pClass, PVOID pCustData)
{
	ApdEvent		*pcsEvent			= (ApdEvent *)			pData;
	AgpmEventGuide	*pThis				= (AgpmEventGuide *)	pClass;
	ApModuleStream	*pStream			= (ApModuleStream *)	pCustData;

	AgpdGuideEventAttachData	*pcsAttachData		= (AgpdGuideEventAttachData *)	pcsEvent->m_pvData;
	if (!pcsAttachData)
		return FALSE;

	const CHAR					*szValueName		= NULL;

	while (pStream->ReadNextValue())
	{
		szValueName = pStream->GetValueName();

		if (!strcmp(szValueName, AGPMEVENT_GUIDE_STREAM_NAME_GROUP_ID))
		{
			INT32	lQuestGroupID	= 0;

			if (!pStream->GetValue(&lQuestGroupID))
			{
				TRACE("AgpmEventGuide::CBStreamReadEvent() GetRaceType Failed\n"); 
				ASSERT(!"AgpmEventGuide::CBStreamReadEvent() GetRaceType Failed");
			}

			pcsAttachData->msgID = lQuestGroupID;
		}
		else if (!strcmp(szValueName, AGPMEVENT_GUIDE_STREAM_NAME_EVENT_END))
			break;
	}

	return TRUE;
}

BOOL AgpmEventGuide::CBEventConstructor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmEventGuide	*pThis			= (AgpmEventGuide *)	pClass;
	ApdEvent		*pcsDestEvent	= (ApdEvent *)			pData;

	pcsDestEvent->m_pvData	= (PVOID) new BYTE[sizeof(AgpdGuideEventAttachData)];
	if (!pcsDestEvent->m_pvData)
		return FALSE;

	ZeroMemory(pcsDestEvent->m_pvData, sizeof(AgpdGuideEventAttachData));

	((AgpdGuideEventAttachData *) pcsDestEvent->m_pvData)->msgID = 0;

	return TRUE;	
}

BOOL AgpmEventGuide::CBEventDestructor(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmEventGuide	*pThis			= (AgpmEventGuide *)pClass;
	ApdEvent		*pcsDestEvent	= (ApdEvent *)pData;

	if (pcsDestEvent->m_pvData)
	{
		delete [] (BYTE*)pcsDestEvent->m_pvData;
		pcsDestEvent->m_pvData	= NULL;
	}

	return TRUE;
}

/*
PVOID AgpmEventGuide::MakePacketEventData(ApdEvent *pcsEvent)
{
	if (!pcsEvent || !pcsEvent->m_pvData)
		return NULL;

	AgpdQuestEventAttachData	*pcsAttachData	= (AgpdQuestEventAttachData *)	pcsEvent->m_pvData;

	INT32	lQuestGroupID	= (INT32) pcsAttachData->lQuestGroupID;

	return m_csPacketEventData.MakePacket(FALSE, NULL, 0,
		&lQuestGroupID);	
}

BOOL AgpmEventGuide::ParseEventDataPacket(ApdEvent *pcsEvent, PVOID pvPacketCustomData)
{
	if (!pcsEvent || !pcsEvent->m_pvData || !pvPacketCustomData)
		return FALSE;

	INT32 lQuestGroupID = 0;

	m_csPacketEventData.GetField(FALSE, pvPacketCustomData, 0,
		&lQuestGroupID);

	AgpdQuestEventAttachData	*pcsAttachData	= (AgpdQuestEventAttachData *)	pcsEvent->m_pvData;

	pcsAttachData->lQuestGroupID = lQuestGroupID;

	return TRUE;	
}

BOOL AgpmEventGuide::CBEmbeddedMakeEventPacket(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgpmEventGuide	*pThis					= (AgpmEventGuide *)	pClass;
	ApdEvent	*pcsEvent				= (ApdEvent *)	pData;
	PVOID		*ppvPacketCustomData	= (PVOID *)		pCustData;

	*ppvPacketCustomData	= pThis->MakePacketEventData(pcsEvent);	
	return TRUE;
}

BOOL AgpmEventGuide::CBEmbeddedReceiveEventPacket(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgpmEventGuide	*pThis					= (AgpmEventGuide *)	pClass;
	ApdEvent	*pcsEvent				= (ApdEvent *)	pData;
	PVOID		pvPacketCustomData		= (PVOID)		pCustData;

	return pThis->ParseEventDataPacket(pcsEvent, pvPacketCustomData);
}
*/