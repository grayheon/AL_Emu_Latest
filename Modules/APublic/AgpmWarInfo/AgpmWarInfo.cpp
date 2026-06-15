#include "AgpmWarInfo.h"



AgpmWarInfo::AgpmWarInfo()
{
	SetModuleName("AgpmWarInfo");
	SetPacketType(AGPM_WARINFO_PACKET_TYPE);

	m_csPacket.SetFlagLength(sizeof(INT8));

	m_csPacket.SetFieldType(
		AUTYPE_INT8,			1,			// operation
		AUTYPE_INT8,			1,			// WarInfoType
		AUTYPE_INT8,			1,			// WarInfoStep
		AUTYPE_INT8,            1,			// WarAddInfo
		AUTYPE_END,				0
		);


}



AgpmWarInfo::~AgpmWarInfo()
{

}

BOOL AgpmWarInfo::OnAddModule()
{
	return TRUE;
}


BOOL AgpmWarInfo::OnInit()
{
	return TRUE;
}

BOOL AgpmWarInfo::OnDestroy()
{
	return TRUE;
}

BOOL AgpmWarInfo::OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)
{
	PROFILE("AgpmWarInfo::OnReceive");

	if(!pvPacket || nSize < 1)
		return FALSE;

	INT8 cOperation = -1;
	INT8 cWarInfoType = -1;
	INT8 cWarInfoStep = -1;
	INT8 cWarAddInfo = 0;

	m_csPacket.GetField(TRUE, pvPacket, nSize,
		&cOperation
		,&cWarInfoType
		,&cWarInfoStep
		,cWarAddInfo
		);

	if(cOperation < 0 || cOperation >= AGPMWARINFO_PACKET_MAX) return TRUE;

	EnumCallback(cOperation, pvPacket, NULL);

	return TRUE;
}



BOOL AgpmWarInfo::SetCallbackUpdateWarState(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMWARINFO_PACKET_UPDATE_WAR_STATE, pfCallback, pClass);
}

BOOL AgpmWarInfo::SetCallbackRequestWarInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMWARINFO_PACKET_REQUEST_WARINFO, pfCallback, pClass);
}

BOOL AgpmWarInfo::SetCallbackUpdateTimeInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMWARINFO_PACKET_UPDATE_TIME, pfCallback, pClass);
}


BOOL AgpmWarInfo::SetCallbackUpdateAddInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMWARINFO_PACKET_UPDATE_ADD_INFO, pfCallback, pClass);
}

BOOL AgpmWarInfo::SetCallbackUpdateCharacterPvPInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMWARINFO_PACKET_CHARACTER_PVP_INFO, pfCallback, pClass);
}

BOOL AgpmWarInfo::SetCallbackUpdateRacePvPInfo(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMWARINFO_PACKET_RACE_PVP_INFO, pfCallback, pClass);
}

BOOL AgpmWarInfo::SetCallbackEndWar(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMWARINFO_PACKET_END_WAR, pfCallback, pClass);
}

BOOL AgpmWarInfo::SetCallbackFactor(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMWARINFO_PACKET_FACTOR, pfCallback, pClass);
}
