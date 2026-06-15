#include "AgsmSessionMgr.h"

#pragma warning(disable:4251) // http://msdn2.microsoft.com/ko-kr/library/esew7y1w.aspx

// class AgsmSessionMgr Implemetation
AgsmSessionMgr::AgsmSessionMgr()
{
	InitializeCriticalSection(&m_csSTQ);
	InitializeCriticalSection(&m_csNTS);
	InitializeCriticalSection(&m_csATQ);
}

AgsmSessionMgr::~AgsmSessionMgr()
{
	DeleteCriticalSection(&m_csSTQ);
	DeleteCriticalSection(&m_csNTS);
	DeleteCriticalSection(&m_csATQ);
}

void AgsmSessionMgr::NewSessionToQi(unsigned session, AgsmLoginQueueInfo* lqi)
{
	Lock lock(m_csSTQ);
	m_stq[session] = lqi;
}

void AgsmSessionMgr::NewNidToSession(unsigned nid, CSessionPtr* csp)
{
	Lock lock(m_csNTS);
	m_nts[nid] = csp;
}

void AgsmSessionMgr::NewAccountToQi(char* account, AgsmLoginQueueInfo* lqi)
{
	Lock lock(m_csATQ);
	m_atq[account] = lqi;
}

// nid로 CSessionPtr을 얻자
CSessionPtr* AgsmSessionMgr::GetSessionByNid(unsigned nid)
{
	Lock lock(m_csNTS);

	NidToSession::iterator iter = m_nts.find(nid);
	return (iter == m_nts.end()) ? 0 : iter->second;
}

// sid로 QueueInfo*를 얻자
AgsmLoginQueueInfo* AgsmSessionMgr::GetQiBySession(unsigned sid)
{
	Lock lock(m_csSTQ);

	SessionToQi::iterator iter = m_stq.find(sid);
	return (iter == m_stq.end()) ? 0 : iter->second;
}

// Account로 QueueInfo*를 얻자
AgsmLoginQueueInfo* AgsmSessionMgr::GetQiByAccount(const char* account)
{
	Lock lock(m_csATQ);

	AccountToQi::iterator iter = m_atq.find(account);
	return (iter == m_atq.end()) ? 0 : iter->second;
}

void AgsmSessionMgr::RemoveSession(unsigned sid)
{
	Lock lock(m_csSTQ);
	m_stq.erase(sid);
}

void AgsmSessionMgr::RemoveNid(unsigned nid)
{
	Lock lock(m_csNTS);
	m_nts.erase(nid);
}

void AgsmSessionMgr::RemoveAccount(char* account)
{
	Lock lock(m_csATQ);
	m_atq.erase(account);
}
