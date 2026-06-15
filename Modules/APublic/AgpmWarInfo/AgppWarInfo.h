#pragma once

#define MAX_WARINFO_FACTOR_LENGTH 32
#pragma pack(1)

struct PACKET_AGPP_WAR_INFO : public PACKET_HEADER
{
	CHAR	Flag1;

	INT8	pcOperation;

	INT8	cWarType;

	PACKET_AGPP_WAR_INFO(INT8 _cWarType)
		: pcOperation(0), Flag1(1)
	{
		cType			= AGPM_WARINFO_PACKET_TYPE;
		unPacketLength	= (UINT16)sizeof(PACKET_AGPP_WAR_INFO);
		cWarType = _cWarType;
	}
};

struct PACKET_WAR_INFO_STATE : public PACKET_AGPP_WAR_INFO
{
	INT8	cWarStep;
	INT8	cMessageStep;
	PACKET_WAR_INFO_STATE(INT8 _cWarType, INT8 _cWarStep, INT8 _cMessageStep) : PACKET_AGPP_WAR_INFO(_cWarType)
	{
		pcOperation = AGPMWARINFO_PACKET_UPDATE_WAR_STATE;
		cWarStep = _cWarStep;
		cMessageStep = _cMessageStep;
		unPacketLength = (UINT16)sizeof(PACKET_WAR_INFO_STATE);
	}
};


struct PACKET_WAR_INFO_TIME :  public PACKET_AGPP_WAR_INFO
{
	INT8 cRemainMin;
	PACKET_WAR_INFO_TIME(INT8 _cWarType, INT8 _cRemainMin) : PACKET_AGPP_WAR_INFO(_cWarType)
	{
		unPacketLength	= (UINT16)sizeof(PACKET_WAR_INFO_TIME);
		pcOperation = AGPMWARINFO_PACKET_UPDATE_TIME;
		cRemainMin = _cRemainMin;
	}
};


struct PACKET_WAR_INFO_ADD :  public PACKET_AGPP_WAR_INFO
{
	INT8 cAddInfoFlag;
	INT8 cUpdateNum;
	PACKET_WAR_INFO_ADD(INT8 _cWarType, INT8 _cAddInfoFlag, INT8 _cUpdateNum) : PACKET_AGPP_WAR_INFO(_cWarType)
	{
		unPacketLength	= (UINT16)sizeof(PACKET_WAR_INFO_ADD);
		pcOperation = AGPMWARINFO_PACKET_UPDATE_ADD_INFO;
		cAddInfoFlag = _cAddInfoFlag;
		cUpdateNum = _cUpdateNum;
	}
};

struct PACKET_WAR_INFO_CHARACTER_PVP_INFO : public PACKET_AGPP_WAR_INFO
{
	INT32 lWinCount;
	INT32 lLoseCount;

	PACKET_WAR_INFO_CHARACTER_PVP_INFO(INT8 _cWarType, INT32 _lWinCount, INT32 _lLoseCount) : PACKET_AGPP_WAR_INFO(_cWarType)
	{
		unPacketLength = (UINT16)sizeof(PACKET_WAR_INFO_CHARACTER_PVP_INFO);
		pcOperation = AGPMWARINFO_PACKET_CHARACTER_PVP_INFO;
		lWinCount = _lWinCount;
		lLoseCount = _lLoseCount;	
	}

};

struct PACKET_WAR_INFO_RACE_PVP_INFO : public PACKET_AGPP_WAR_INFO
{
	INT32 lWinCount;
	INT32 lLoseCount;

	PACKET_WAR_INFO_RACE_PVP_INFO(INT8 _cWarType, INT32 _lWinCount, INT32 _lLoseCount) : PACKET_AGPP_WAR_INFO(_cWarType)
	{
		unPacketLength = (UINT16)sizeof(PACKET_WAR_INFO_RACE_PVP_INFO);
		pcOperation = AGPMWARINFO_PACKET_RACE_PVP_INFO;
		lWinCount = _lWinCount;
		lLoseCount = _lLoseCount;	
	}
};


struct PACKET_WAR_INFO_WAR_END_INFO : public PACKET_AGPP_WAR_INFO
{
	PACKET_WAR_INFO_WAR_END_INFO(INT8 _cWarType) : PACKET_AGPP_WAR_INFO(_cWarType)
	{
		unPacketLength = (UINT16)sizeof(PACKET_WAR_INFO_WAR_END_INFO);
		pcOperation = AGPMWARINFO_PACKET_END_WAR;
	}
};

struct PACKET_WAR_INFO_FACTOR : public PACKET_AGPP_WAR_INFO
{
	CHAR szFactor1[MAX_WARINFO_FACTOR_LENGTH + 1];
	CHAR szFactor2[MAX_WARINFO_FACTOR_LENGTH + 1];
	CHAR szFactor3[MAX_WARINFO_FACTOR_LENGTH + 1];
	PACKET_WAR_INFO_FACTOR(INT8 _cWarType, CHAR *_szFactor1, CHAR *_szFactor2, CHAR *_szFactor3) : PACKET_AGPP_WAR_INFO(_cWarType)
	{
		ZeroMemory(szFactor1, MAX_WARINFO_FACTOR_LENGTH + 1);
		ZeroMemory(szFactor2, MAX_WARINFO_FACTOR_LENGTH + 1);
		ZeroMemory(szFactor3, MAX_WARINFO_FACTOR_LENGTH + 1);
		if(_szFactor1) strncpy(szFactor1, _szFactor1, MAX_WARINFO_FACTOR_LENGTH);
		if(_szFactor2) strncpy(szFactor2, _szFactor2, MAX_WARINFO_FACTOR_LENGTH);
		if(_szFactor3) strncpy(szFactor3, _szFactor3, MAX_WARINFO_FACTOR_LENGTH);

		unPacketLength = (UINT16)sizeof(PACKET_WAR_INFO_FACTOR);
		pcOperation = AGPMWARINFO_PACKET_FACTOR;
	}
};



#pragma pack()
