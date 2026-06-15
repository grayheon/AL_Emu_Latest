// AgpaGuild.h
// (C) NHN - ArchLord Development Team
// steeple, 2004. 06. 15.

#ifndef _AGPAGUILD_H_
#define _AGPAGUILD_H_

#include "ApBase.h"
#include "ApAdmin.h"
#include "AgpdGuild.h"
#include "ApSort.h"

//////////////////////////////////////////////////////////////////////////
// AgpdGuild Class 를 관리한다.


class AgpaGuild : public ApAdmin
{
public:
	static const int GUILD_MASTER_VISIT_MAX_TERM = 7 * 24 * 60 * 60;

	AgpaGuild()				{		}
	virtual ~AgpaGuild()	{		}

	AgpdGuild*	GetGuild(CHAR* szGuildID);
	AgpdGuild*	AddGuild(AgpdGuild* pcsGuild);
	BOOL		RemoveGuild(CHAR* szGuildID);

	BOOL		SortGuildMemberDesc( BOOL bPriorityMember );
	BOOL		IsGuildMasterVisitMaxTermOver(AgpdGuild *pcsGuild);

	ApSort< AgpdGuild* >	m_ApGuildSortList;

public:
	struct CompareGuildMemberDesc
	{
		template < class T >
		bool operator()(const T& a, const T& b)
		{
			return a->m_pMemberList->GetObjectCount() > b->m_pMemberList->GetObjectCount();
		};
	};

	struct CompareGuildPointDesc
	{
		template < class T >
		bool operator()(const T& a, const T& b)
		{
			return a->m_lGuildPoint > b->m_lGuildPoint;
		};
	};
};

#endif //_AGPAGUILD_H_