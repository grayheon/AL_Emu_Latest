#include "AgcmWarInfo.h"
#include "AuExcelTxtLib.h"

#define AGCMWARINFO_STREAM_NAME_TYPE "TYPE"
#define AGCMWARINFO_STREAM_NAME_STEP "STEP"
#define AGCMWARINFO_STREAM_NAME_STEP_MESSAGE "STEP MESSAGE"
#define AGCMWARINFO_STREAM_NAME_CENTER_MESSAGE1 "CENTER MESSAGE1"
#define AGCMWARINFO_STREAM_NAME_CENTER_MESSAGE2 "CENTER MESSAGE2"
#define AGCMWARINFO_STREAM_NAME_CENTER_MESSAGE3 "CENTER MESSAGE3"



static const CHAR *g_aszWarInfoTypeName[WarInfoTypeMax * 2] =
{
	"SiegeWar",
	"BattleGround",
	"ShrineBattle",
	"ArchlordBattle",
};


BOOL AgcmWarInfo::ReadStreamWarInfo( CHAR *szFile, BOOL bDecryption )
{
	if (!szFile || !szFile[0])
		return FALSE;

	AuExcelTxtLib	csExcelTxtLib;

	if (!csExcelTxtLib.OpenExcelFile(szFile, TRUE, bDecryption))
		return FALSE;

	INT16			nRow			= csExcelTxtLib.GetRow();
	INT16			nColumn			= csExcelTxtLib.GetColumn();
	CHAR szError[256];

	stWarInfoData tempWarInfoData;


	if (nRow < 1)
		return FALSE;

	for (int i = 1; i < nRow; ++i)
	{
		INT32 nType = -1;
		INT32 nStep = -1;

		stWarInfoData tempWarInfoData;

		for (int j = 0; j < nColumn; ++j)
		{
			CHAR	*szValue		= csExcelTxtLib.GetData(j, i);
			if (!szValue)
				continue;

			CHAR	*szColumnName	= csExcelTxtLib.GetData(j, 0);
			if (!szColumnName)
				continue;


			if(strcmp(szColumnName, AGCMWARINFO_STREAM_NAME_TYPE) == 0)
			{
				for(int k = 0; k < WarInfoTypeMax; ++k)
				{
					if (strcmp(szValue, g_aszWarInfoTypeName[k]) == 0)
					{
						nType = k;
						break;
					}
				}
				continue;
			}



			if(strcmp(szColumnName, AGCMWARINFO_STREAM_NAME_STEP) == 0)
			{
				nStep = atoi(szValue);
			}
			else if(strcmp(szColumnName, AGCMWARINFO_STREAM_NAME_STEP_MESSAGE) == 0)
			{
				tempWarInfoData.szStepMessage = szValue;
			}
			else if(strcmp(szColumnName, AGCMWARINFO_STREAM_NAME_CENTER_MESSAGE1) == 0)
			{
				tempWarInfoData.szCenterMessage[0] = szValue;
			}
			else if(strcmp(szColumnName, AGCMWARINFO_STREAM_NAME_CENTER_MESSAGE2) == 0)
			{
				tempWarInfoData.szCenterMessage[1] = szValue;
			}
			else if(strcmp(szColumnName, AGCMWARINFO_STREAM_NAME_CENTER_MESSAGE3) == 0)
			{
				tempWarInfoData.szCenterMessage[2] = szValue;
			}

		}

		if(nType < 0 || nType >= WarInfoTypeMax)
		{
			sprintf(szError, "ERROR:[WarInfoData.txt] %d줄 TYPE이 잘못되었습니다!", i + 1);
			OutputDebugString(szError);
			continue;
		}
		if(nStep < 0)
		{
			sprintf(szError, "ERROR:[WarInfoData.txt] %d줄 Step이 잘못되었습니다!", i + 1);

			OutputDebugString(szError);
			continue;
		}
		if(nStep < 100)
		{
			--nStep;
			if(m_mapWarInfoData[nType].find(nStep) == m_mapWarInfoData[nType].end())
			{
				m_mapWarInfoData[nType].insert(pair<INT32, stWarInfoData>(nStep, tempWarInfoData));
			}
		}
		else
		{
			nStep -= 100;
			--nStep;
			if(m_mapWarAddMessageData[nType].find(nStep) == m_mapWarAddMessageData[nType].end())
			{
				m_mapWarAddMessageData[nType].insert(pair<INT32, stWarInfoData>(nStep, tempWarInfoData));
			}
		}
	}
	return TRUE;
}