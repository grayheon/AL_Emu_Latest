#ifndef _AGPM_EPIC_ZONE_H
#define _AGPM_EPIC_ZONE_H

#include "ApModule.h"

enum eEpicZoneRegionIndex
{
	// 여기에 EpicZone에 해당되는 Region들이 들어가게 된다.
	REGIONINDEX_EPICZONE_HELL_OF_FLAME			= 180,
	REGIONINDEX_EPICZONE_VEIN_OF_LIFE			= 181,
	REGIONINDEX_EPICZONE_MOUNTAINS_BLADESTORM	= 182,
	REGIONINDEX_EPICZONE_HEAVEN_HELL			= 183,
};

class AgpdCharacter;
class AgpmCharacter;

class AgpmEpicZone : public ApModule
{
private:
	AgpmCharacter*		m_pcsAgpmCharacter;

public:
	AgpmEpicZone();
	virtual ~AgpmEpicZone();

	BOOL	OnInit();

	BOOL	IsEpicZone(INT16 nBindingRegionindex);
	BOOL	IsInEpicZone(AgpdCharacter* pcsCharacter);
	BOOL	IsEpicBossZone(INT16 nBindingRegionindex);
	BOOL	IsInEpicBossZone(AgpdCharacter* pcsCharacter);
};

#endif