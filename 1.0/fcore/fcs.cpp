#include "stdafx.h"
#include "fbase.h"

#ifdef FGBC_OS_WIN
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif
#endif
#ifdef FGBC_OS_UNIX
#endif


namespace F5Chain
{
namespace FCORE
{

#ifdef FGBC_OS_WIN
	fCS::fCS()
	{
		::InitializeCriticalSection(&m_cs);
	}

	fCS::~fCS()
	{
		::DeleteCriticalSection(&m_cs);
	}

	void fCS::Enter()
	{
		::EnterCriticalSection(&m_cs);
	}

	void fCS::Leave()
	{
		::LeaveCriticalSection(&m_cs);
	}
#endif
#ifdef FGBC_OS_UNIX
	fCS::fCS()
	{
		pthread_mutex_init(&m_pm, NULL);
	}

	fCS::~fCS()
	{
		pthread_mutex_destroy(&m_pm);
	}

	void fCS::Enter()
	{
		pthread_mutex_lock(&m_pm);
	}

	void fCS::Leave()
	{
		pthread_mutex_unlock(&m_pm);
	}
#endif



	fLock::fLock(fCS& cs)
	{
		m_pcs = &cs;
		m_pcs->Enter();
	}

	fLock::~fLock()
	{
		m_pcs->Leave();
	}


}	// end of namespace FCORE
}	// end of namespace FGBC