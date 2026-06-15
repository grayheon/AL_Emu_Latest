#pragma once

#include "ApmEventManager.h"
#include "AgpmCharacter.h"
#include "AuPacket.h"

#if _MSC_VER < 1300
#ifdef _DEBUG
#pragma comment(lib, "AgpmEventGuidetD.lib")
#else
#pragma comment(lib, "AgpmEventGuide.lib")
#endif
#endif

//#define	AGPMEVENT_GUIDE_MAX_USE_RANGE				1600
#define AGPMEVENT_GUIDE_STREAM_NAME_GROUP_ID		"GuideID"
#define AGPMEVENT_GUIDE_STREAM_NAME_EVENT_END		"GuideEnd"

class AgpdGuideEventAttachData {
public:
	INT32 msgID;
};

/*
typedef enum _eAgpmEventGuidePacketType
{
	AGPMEVENT_GUIDE_PACKET_REQUEST = 0,
	AGPMEVENT_GUIDE_PACKET_GRANT,
	AGPMEVENT_GUIDE_PACKET_MAX,
} eAgpmEventGuidePacketType;

typedef enum _eAgpmEventGuideCallback
{
	AGPMEVENT_GUIDE_CB_REQUEST = 0,
	AGPMEVENT_GUIDE_CB_GRANT,
	AGPMEVENT_GUIDE_CB_MAX,
} eAgpmEventGuideCallback;
*/

class AgpmEventGuide : public ApModule
{
public:
	AgpmCharacter* m_pcsAgpmCharacter;
	ApmEventManager* m_pcsApmEventManager;

	//AuPacket	m_csPacket;
	//AuPacket	m_csPacketEventData;

public:
	AgpmEventGuide();
	virtual ~AgpmEventGuide();

	BOOL OnAddModule();

	//BOOL OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg);

	// Event Data Stream
	static BOOL			CBStreamWriteEvent(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL			CBStreamReadEvent(PVOID pData, PVOID pClass, PVOID pCustData);

	static BOOL			CBEventConstructor(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL			CBEventDestructor(PVOID pData, PVOID pClass, PVOID pCustData);

	/*
	BOOL				OnOperationRequest(ApdEvent* pcsEvent, AgpdCharacter* pcsCharacter, INT32 lRange);
	BOOL				OnOperationGrant(ApdEvent* pcsEvent, AgpdCharacter* pcsCharacter);

	// Event Data
	PVOID MakePacketEventData(ApdEvent *pcsEvent);
	BOOL ParseEventDataPacket(ApdEvent *pcsEvent, PVOID pvPacketCustomData);
	PVOID MakePacketEventRequest(ApdEvent *pcsEvent, INT32 lCID, INT16 *pnPacketLength, INT32 lRange);
	PVOID MakePacketEventGrant(ApdEvent *pcsEvent, INT32 lCID, INT16 *pnPacketLength);
	// ApmEventManager Embedded Packet
	static BOOL CBEmbeddedMakeEventPacket(PVOID pData, PVOID pClass, PVOID pCustData);
	static BOOL CBEmbeddedReceiveEventPacket(PVOID pData, PVOID pClass, PVOID pCustData);
	*/

	BOOL SetCallbackRequest(ApModuleDefaultCallBack pfCallback, PVOID pClass);
	BOOL SetCallbackGrant(ApModuleDefaultCallBack pfCallback, PVOID pClass);

	static BOOL CBActionGuide(PVOID pData, PVOID pClass, PVOID pCustData);
};