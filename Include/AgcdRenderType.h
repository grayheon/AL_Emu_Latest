#ifndef __AGCD_RENDER_TYPE_H__
#define __AGCD_RENDER_TYPE_H__

#include <windows.h>
#include <vector>

using namespace std;

#define D_AGCD_CLUMP_RENDER_TYPE_DATA_CUST_DATA_NONE	-1

class AgcdClumpRenderTypeData
{
public:
	//INT32	*m_plRenderType;
	//INT32	*m_plCustData;
	vector< INT32 >	m_vecRenderType;
	vector< INT32 >	m_vecCustData;

	AgcdClumpRenderTypeData();
	virtual ~AgcdClumpRenderTypeData();

	void	Alloc( int nSize );
	void	MemsetRenderType( INT32 nValue );
	void	MemsetCustData( INT32 nValue );
	void	Clear();
};

class AgcdClumpRenderType
{
public:
	INT32						m_lSetCount;
	AgcdClumpRenderTypeData		m_csRenderType;

	INT32						m_lCBCount;
	INT32						*m_plCustData;

	AgcdClumpRenderType()
	{
		m_lCBCount		= 0;
		m_lSetCount		= 0;
		m_plCustData	= NULL;
	}
};

#endif