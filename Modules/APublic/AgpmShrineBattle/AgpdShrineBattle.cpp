#include "AgpdShrineBattle.h"

AgpdShrineBattle::AgpdShrineBattle()
{
	m_vtShrinePoint.clear();

	for(int i = 0; i < AURACE_TYPE_MAX; i++)
		memset(m_szRaceName[i], 0, AGPDSHRINEBATTLE_RACE_NAME_MAX_LENGTH);

	for(int i = 0; i < AGPMSHRINEBATTLE_MESSAGE_CODE_MAX; i++)
		memset(m_szMessage[i], 0, AGPDSHRINEBATTLE_MESSAGE_MAX_LENGTH);

	m_bLoad = FALSE;

	m_lMaxRegionIndex = -1;
	m_lMinRegionIndex = -1;
}

IterShrineRegion AgpdShrineBattle::FindRegionByShrinePointID(INT32 nShrinePointID)
{
	IterShrineRegion iter;

	for(iter = m_vtShrineRegion.begin(); iter != m_vtShrineRegion.end(); ++iter)
	{
		if((*iter).m_nShrinePointID == nShrinePointID)
			return iter;
	}

	return NULL;
}

IterShrineRegion AgpdShrineBattle::FindRegionByRegionIndex(INT32 nRegionIndex)
{
	IterShrineRegion iter;

	for(iter = m_vtShrineRegion.begin(); iter != m_vtShrineRegion.end(); ++iter)
	{
		if((*iter).m_nRegionIndex == nRegionIndex)
			return iter;
	}

	return NULL;
}

IterShrinePoint AgpdShrineBattle::FindByShrinePointID(INT32 nShrinePointID)
{
	IterShrinePoint iter;

	for(iter = m_vtShrinePoint.begin(); iter != m_vtShrinePoint.end(); ++iter)
	{
		if((*iter).m_nShrinePointID == nShrinePointID)
			return iter;
	}

	return NULL;
}