#pragma once

#include "AgpmPvP.h"
#include "AgsmPvP.h"
#include "AgsdRelay2.h"

/************************************************************************/
/* AgspPvP : GameServer <-> RelayServer °£ Packet                       */
/************************************************************************/

enum eAgspPvPPacketType
{
	AGSPPVP_PACKET_TYPE_NONE =0						,
	AGSPPVP_PACKET_TYPE_UPDATE						,
	AGSPPVP_PACKET_TYPE_LOAD_REQUEST				,
	AGSPPVP_PACKET_TYPE_LOAD_ANSWER					,
	AGSPPVP_PACKET_TYPE_MAX				
};

enum eAgspPvPPacketResult
{
	AGSPPVP_PACKET_RESULT_TYPE_NONE	= 0				,
	AGSPPVP_PACKET_RESULT_TYPE_SUCCESS				,
	AGSPPVP_PACKET_RESULT_TYPE_FAIL					,
	AGSPPVP_PACKET_RESULT_TYPE_MAX
};

struct PACKET_AGSP_PVP : public PACKET_HEADER
{
	CHAR		m_szFlag1;

	INT16		m_nParam;
	INT16		m_nOperation;

	PACKET_AGSP_PVP()
		: m_szFlag1(1)
	{
		cType			= AGSMRELAY_PACKET_TYPE;
		m_nParam		= AGSMRELAY_PARAM_PVP;
		unPacketLength	= sizeof(PACKET_AGSP_PVP);
	}
};

struct PACKET_AGSP_PVP_RESULT : public PACKET_AGSP_PVP
{
	CHAR		m_strCharID[AGPACHARACTER_MAX_ID_STRING + 1];

	INT32		m_lTotalWin;
	INT32		m_lTotalLose;
	INT32		m_lVSHumanWin;
	INT32		m_lVSHumanLose;
	INT32		m_lVSOrcWin;
	INT32		m_lVSOrcLose;
	INT32		m_lVSMoonElfWin;
	INT32		m_lVSMoonElfLose;
	INT32		m_lVSScionWin;
	INT32		m_lVSScionLose;

	PACKET_AGSP_PVP_RESULT() {}

	PACKET_AGSP_PVP_RESULT(CHAR* pszID, INT32& lTotalWin, INT32& lTotalLose, INT32& lVSHumanWin, INT32& lVSHumanLose,
						   INT32& lVSOrcWin, INT32& lVSOrcLose, INT32& lMoonElfWin, INT32& lMoonElfLose, INT32& lScionWin, INT32& lScionLose)
	{
		m_nOperation	 = AGSPPVP_PACKET_TYPE_UPDATE;

		m_lTotalWin		 = lTotalWin;
		m_lTotalLose	 = lTotalLose;
		m_lVSHumanWin	 = lVSHumanWin;
		m_lVSHumanLose	 = lVSHumanLose;
		m_lVSOrcWin		 = lVSOrcWin;
		m_lVSOrcLose	 = lVSOrcLose;
		m_lVSMoonElfWin	 = lMoonElfWin;
		m_lVSMoonElfLose = lMoonElfLose;
		m_lVSScionWin	 = lScionWin;
		m_lVSScionLose	 = lScionLose;

		unPacketLength	 = sizeof(PACKET_AGSP_PVP_RESULT);

		strncpy_s(m_strCharID, AGPACHARACTER_MAX_ID_STRING, pszID, _TRUNCATE);
	}
};

struct PACKET_AGSP_PVP_RESULT_ANSWER : public PACKET_AGSP_PVP_RESULT
{
	eAgspPvPPacketResult	m_eResultType;

	PACKET_AGSP_PVP_RESULT_ANSWER(eAgspPvPPacketResult eType, CHAR* pszID, INT32& lTotalWin, INT32& lTotalLose, INT32& lVSHumanWin, INT32& lVSHumanLose,
								  INT32& lVSOrcWin, INT32& lVSOrcLose, INT32& lMoonElfWin, INT32& lMoonElfLose, INT32& lScionWin, INT32& lScionLose)
	{
		m_nOperation	 = AGSPPVP_PACKET_TYPE_LOAD_ANSWER;

		m_eResultType	 = eType;

		m_lTotalWin		 = lTotalWin;
		m_lTotalLose	 = lTotalLose;
		m_lVSHumanWin	 = lVSHumanWin;
		m_lVSHumanLose	 = lVSHumanLose;
		m_lVSOrcWin		 = lVSOrcWin;
		m_lVSOrcLose	 = lVSOrcLose;
		m_lVSMoonElfWin	 = lMoonElfWin;
		m_lVSMoonElfLose = lMoonElfLose;
		m_lVSScionWin	 = lScionWin;
		m_lVSScionLose	 = lScionLose;

		unPacketLength	 = sizeof(PACKET_AGSP_PVP_RESULT_ANSWER);

		strncpy_s(m_strCharID, AGPACHARACTER_MAX_ID_STRING, pszID, _TRUNCATE);
	}
};

struct PACKET_AGSP_PVP_RESULT_REQUEST : public PACKET_AGSP_PVP
{
	CHAR		m_strCharID[AGPACHARACTER_MAX_ID_STRING + 1];

	PACKET_AGSP_PVP_RESULT_REQUEST(CHAR* pszCharID)
	{
		m_nOperation = AGSPPVP_PACKET_TYPE_LOAD_REQUEST;

		unPacketLength = sizeof(PACKET_AGSP_PVP_RESULT_REQUEST);

		strncpy_s(m_strCharID, AGPACHARACTER_MAX_ID_STRING, pszCharID, _TRUNCATE);
	}
};