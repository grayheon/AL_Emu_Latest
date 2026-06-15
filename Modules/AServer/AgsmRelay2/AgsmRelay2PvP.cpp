#include "AgsmRelay2.h"
#include "AgspPvP.h"
#include "AgpmPvP.h"
#include "AgsmPvP.h"
#include "AgpdPvP.h"

AgsdRelay2PvP::AgsdRelay2PvP(UINT32 ulNID, INT16 Operation)
{
	m_ulNID = ulNID;
	m_nOperation = Operation;
	m_eOperation = AGSMDATABASE_OPERATION_EXECUTE;
}


AgsdRelay2PvPLoad::AgsdRelay2PvPLoad(UINT32 ulNID, INT16 Operation)
{
	m_ulNID = ulNID;
	m_nOperation = Operation;
	m_eOperation = AGSMDATABASE_OPERATION_EXECUTE;
}

void AgsdRelay2PvP::Release()
{
	delete this;
}

void AgsdRelay2PvPLoad::Release()
{
	delete this;
}

BOOL AgsdRelay2PvP::SetParamExecute(AuStatement* pStatement)
{
	if (NULL == pStatement)
		return FALSE;

	INT32 i=0;

	pStatement->SetParam(i++, m_szCharID);
	pStatement->SetParam(i++, &m_lTotalWin);
	pStatement->SetParam(i++, &m_lTotalLose);
	pStatement->SetParam(i++, &m_lVSHumanWin);
	pStatement->SetParam(i++, &m_lVSHumanLose);
	pStatement->SetParam(i++, &m_lVSOrcWin);
	pStatement->SetParam(i++, &m_lVSOrcLose);
	pStatement->SetParam(i++, &m_lVSMoonElfWin);
	pStatement->SetParam(i++, &m_lVSMoonElfLose);
	pStatement->SetParam(i++, &m_lVSScionWin);
	pStatement->SetParam(i++, &m_lVSScionLose);

	return TRUE;
}

BOOL AgsdRelay2PvPLoad::SetParamExecute(AuStatement* pStatement)
{
	if (NULL == pStatement)
		return FALSE;

	pStatement->SetParam(0, m_szCharID);

	return TRUE;
}

BOOL AgsmRelay2::OnParamPvP(PACKET_HEADER *pvPacket, UINT32 ulNID)
{
	if (NULL == pvPacket)
		return FALSE;

	PACKET_AGSP_PVP *ppPacket = (PACKET_AGSP_PVP *) pvPacket;

	if (ppPacket->m_nOperation == AGSPPVP_PACKET_TYPE_UPDATE)
	{
		PACKET_AGSP_PVP_RESULT *pPacket = (PACKET_AGSP_PVP_RESULT *) ppPacket;
		
		AgsdRelay2PvP *pcsPvPUpdate = new AgsdRelay2PvP(ulNID, pPacket->m_nOperation);

		if (pPacket->m_strCharID)
		{
			strncpy_s(pcsPvPUpdate->m_szCharID, AGPACHARACTER_MAX_ID_STRING, pPacket->m_strCharID, _TRUNCATE);

			pcsPvPUpdate->m_lTotalWin		= pPacket->m_lTotalWin;
			pcsPvPUpdate->m_lTotalLose		= pPacket->m_lTotalLose;
			pcsPvPUpdate->m_lVSHumanWin		= pPacket->m_lVSHumanWin;
			pcsPvPUpdate->m_lVSHumanLose	= pPacket->m_lVSHumanLose;
			pcsPvPUpdate->m_lVSOrcWin		= pPacket->m_lVSOrcWin;
			pcsPvPUpdate->m_lVSOrcLose		= pPacket->m_lVSOrcLose;
			pcsPvPUpdate->m_lVSMoonElfWin	= pPacket->m_lVSMoonElfWin;
			pcsPvPUpdate->m_lVSMoonElfLose	= pPacket->m_lVSMoonElfLose;
			pcsPvPUpdate->m_lVSScionWin		= pPacket->m_lVSScionWin;
			pcsPvPUpdate->m_lVSScionLose	= pPacket->m_lVSScionLose;

			AgsdQueryWithParam* pQuery = new AgsdQueryWithParam;
			pQuery->m_nIndex = AGSMRELAY_QUERY_PVP_UPDATE;
			pQuery->m_pParam = pcsPvPUpdate;

			m_pAgsmDatabasePool->Execute(pQuery);
		}
	}

	else if (ppPacket->m_nOperation == AGSPPVP_PACKET_TYPE_LOAD_REQUEST)
	{
		PACKET_AGSP_PVP_RESULT_REQUEST *pPacket = (PACKET_AGSP_PVP_RESULT_REQUEST *) ppPacket;

		AgsdRelay2PvPLoad *pcsPvPLoad = new AgsdRelay2PvPLoad(ulNID, pPacket->m_nOperation);

		if (pPacket->m_strCharID)
		{
			strncpy_s(pcsPvPLoad->m_szCharID, AGPACHARACTER_MAX_ID_STRING, pPacket->m_strCharID, _TRUNCATE);

			AgsdQueryWithParam* pQuery = new AgsdQueryWithParam;
			pQuery->m_nIndex = AGSMRELAY_QUERY_PVP_LOAD;
			pQuery->m_pParam = pcsPvPLoad;

			pQuery->SetCallback(AgsmRelay2::CBOperationResultLoadPvPResultSuccess, AgsmRelay2::CBOperationResultLoadPvPResultFail, this, pcsPvPLoad);

			m_pAgsmDatabasePool->Execute(pQuery);
		}
	}

	else if (ppPacket->m_nOperation == AGSPPVP_PACKET_TYPE_LOAD_ANSWER)
	{
		PACKET_AGSP_PVP_RESULT_ANSWER *pPacket = (PACKET_AGSP_PVP_RESULT_ANSWER *) ppPacket;

		if (pPacket->m_eResultType == AGSPPVP_PACKET_RESULT_TYPE_SUCCESS)
		{
			AgpdCharacter *pcsCharacter = m_pAgpmCharacter->GetCharacter(pPacket->m_strCharID);
			if (pcsCharacter)
			{
				AuAutoLock Lock(pcsCharacter->m_Mutex);
				if (!Lock.Result())
					return FALSE;

				AgpdPvPADChar *pcsADPvPChar = m_pAgpmPvP->GetADCharacter(pcsCharacter);
				if (pcsADPvPChar)
				{
					pcsADPvPChar->m_lWin											= pPacket->m_lTotalWin;
					pcsADPvPChar->m_lLose											= pPacket->m_lTotalLose;
					pcsADPvPChar->m_arrRaceScore[AURACE_TYPE_HUMAN].m_lWin			= pPacket->m_lVSHumanWin;
					pcsADPvPChar->m_arrRaceScore[AURACE_TYPE_HUMAN].m_lLose			= pPacket->m_lVSHumanLose;
					pcsADPvPChar->m_arrRaceScore[AURACE_TYPE_ORC].m_lWin			= pPacket->m_lVSOrcWin;
					pcsADPvPChar->m_arrRaceScore[AURACE_TYPE_ORC].m_lLose			= pPacket->m_lVSOrcLose;
					pcsADPvPChar->m_arrRaceScore[AURACE_TYPE_MOONELF].m_lWin		= pPacket->m_lVSMoonElfWin;
					pcsADPvPChar->m_arrRaceScore[AURACE_TYPE_MOONELF].m_lLose		= pPacket->m_lVSMoonElfLose;
					pcsADPvPChar->m_arrRaceScore[AURACE_TYPE_DRAGONSCION].m_lWin	= pPacket->m_lVSScionWin;
					pcsADPvPChar->m_arrRaceScore[AURACE_TYPE_DRAGONSCION].m_lLose	= pPacket->m_lVSScionLose;
				}

				m_pAgsmPvP->SendPvPInfo(pcsCharacter);
			}
		}
	}

	return TRUE;
}

BOOL AgsmRelay2::CBOperationResultLoadPvPResultSuccess(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(NULL == pData || NULL == pClass || NULL == pCustData)
		return FALSE;

	AgsmRelay2 *pThis = (AgsmRelay2 *) pClass;
	AgsdRelay2PvPLoad *pAgsdRelay2 = (AgsdRelay2PvPLoad *) pCustData;
	AuRowset *pRowset = (AuRowset *) pData;

	for (UINT32 i=0; i < pRowset->GetRowCount(); ++i)
	{
		if (pAgsdRelay2->m_szCharID)
		{
			INT32 lTotalWin			= atoi((CHAR*)pRowset->Get(i, 1));
			INT32 lTotalLose		= atoi((CHAR*)pRowset->Get(i, 2));
			INT32 lVSHumanWin		= atoi((CHAR*)pRowset->Get(i, 3));
			INT32 lVSHumanLose		= atoi((CHAR*)pRowset->Get(i, 4));
			INT32 lVSOrcWin			= atoi((CHAR*)pRowset->Get(i, 5));
			INT32 lVSOrcLose		= atoi((CHAR*)pRowset->Get(i, 6));
			INT32 lVSMoonElfWin		= atoi((CHAR*)pRowset->Get(i, 7));
			INT32 lVSMoonElfLose	= atoi((CHAR*)pRowset->Get(i, 8));
			INT32 lVSScionWin		= atoi((CHAR*)pRowset->Get(i, 9));
			INT32 lVSScionLose		= atoi((CHAR*)pRowset->Get(i, 10));

			PACKET_AGSP_PVP_RESULT_ANSWER stPacket(AGSPPVP_PACKET_RESULT_TYPE_SUCCESS, pAgsdRelay2->m_szCharID, lTotalWin, lTotalLose, lVSHumanWin, lVSHumanLose,
				lVSOrcWin, lVSOrcLose, lVSMoonElfWin, lVSMoonElfLose, lVSScionWin, lVSScionLose);

			AgsEngine::GetInstance()->SendPacket(stPacket, pAgsdRelay2->m_ulNID);
		}
	}

	pAgsdRelay2->Release();

	return TRUE;
}

BOOL AgsmRelay2::CBOperationResultLoadPvPResultFail(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(NULL == pData || NULL == pClass || NULL == pCustData)
		return FALSE;

	AgsmRelay2 *pThis = (AgsmRelay2 *) pClass;
	AgsdRelay2PvPLoad *pAgsdRelay2 = (AgsdRelay2PvPLoad *) pCustData;
	AuRowset *pRowset = (AuRowset *) pData;

	if (pAgsdRelay2->m_szCharID)
	{
		INT32  lTempResult = -1;

		PACKET_AGSP_PVP_RESULT_ANSWER stPacket(AGSPPVP_PACKET_RESULT_TYPE_FAIL, pAgsdRelay2->m_szCharID, lTempResult, lTempResult, lTempResult, lTempResult,
			lTempResult, lTempResult, lTempResult, lTempResult, lTempResult, lTempResult);

		AgsEngine::GetInstance()->SendPacket(stPacket, pAgsdRelay2->m_ulNID);
	}

	pAgsdRelay2->Release();

	return TRUE;
}
