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

///////////////////////////////////////////////////////////////////////////////
// fThread
	ULONG PASCAL fThreadFunc(void *pParam)
	{
		fThread *pThread = (fThread*)pParam;
		HANDLE pEventWaits[2] = {pThread->m_hEventRun, pThread->m_hEventQuit};
		while (1)
		{
			DWORD dwResult = ::WaitForMultipleObjects(2, pEventWaits, FALSE, INFINITE);
			if (dwResult == WAIT_OBJECT_0)
			{
				pThread->Process();
			}
			else if (dwResult == WAIT_OBJECT_0+1)
			{
				break;
			}
		}
		return 1;
	};

	fThread::fThread()
	{
		m_hThread = NULL;
		m_dwThreadID = 0;
		m_hEventRun = NULL;
		m_hEventQuit = NULL;
		m_bQuit = false;
		m_bStop = false;
		m_nPercent = 0;
	}

	fThread::~fThread()
	{
		Quit();
	}
	
#ifdef FGBC_OS_WIN
	HANDLE fThread::Create(bool bSuspended)
	{
		Quit();

		m_hEventRun = ::CreateEvent(NULL, FALSE, FALSE, NULL);		// auto-reset event
		m_hEventQuit = ::CreateEvent(NULL, FALSE, FALSE, NULL);		// auto-reset event

		m_hThread = ::CreateThread(NULL, 0, fThreadFunc, this, bSuspended ? CREATE_SUSPENDED : 0, &m_dwThreadID);
		m_bQuit = false;
		m_bStop = false;

		return m_hThread;
	}

	bool fThread::Run()
	{
		if (!IsValid()) return false;
		m_bQuit = false;
		m_bStop = false;
		::ResumeThread(m_hThread);
		return (::SetEvent(m_hEventRun) == TRUE);		
	}
	
	bool fThread::Stop()
	{
		//if (!IsValid()) return false;
		m_bStop = true;
		return true;
	}

	bool fThread::Quit()
	{
		if (!IsValid()) return false;

		m_bQuit = true;
		m_bStop = true;

		::ResumeThread(m_hThread);
		if (::SetEvent(m_hEventQuit) == FALSE) return false;

		::WaitForSingleObject(m_hThread, INFINITE); 
		::TerminateThread(m_hThread, 0);

		::CloseHandle(m_hThread); 
		::CloseHandle(m_hEventRun);
		::CloseHandle(m_hEventQuit);
		m_hThread = NULL; 
		m_hEventRun = NULL;
		m_hEventQuit = NULL;
		m_bQuit = false;
		m_bStop = false;

		return true;
	}

#endif
#ifdef FGBC_OS_UNIX
	int fThread::Create(bool bSuspended)
	{
		Quit();
		m_hThread = pthread_create((pthread_t*)&m_dwThreadID, NULL, fThreadFunc, this);
		return m_hThread;
	}

	bool fThread::Run()
	{
		return false;
	}

	bool fThread::Quit()
	{
		return false; 
	}

	bool fThread::WaitToRun()
	{
		return false;
	}
	}
#endif

///////////////////////////////////////////////////////////////////////////////
// FParallelProcess
	typedef struct tagParamPP
	{
		fParallelProcess *pPP;
		int nIdx;
	} XParamPP;

	ULONG PASCAL fParallelProcessFunc(void *pParam)
	{
		XParamPP *pParamPP = (XParamPP*)pParam;

		fParallelProcess *pPP = pParamPP->pPP;
		int nIdx = pParamPP->nIdx;

		HANDLE pEventWaits[2] = {pPP->m_pEventRun[nIdx], pPP->m_pEventQuit[nIdx]};
		while (1)
		{
			DWORD dwResult = ::WaitForMultipleObjects(2, pEventWaits, FALSE, INFINITE);
			if (dwResult == WAIT_OBJECT_0)
			{
				pPP->Process(nIdx);
				::SetEvent(pPP->m_pEventDone[nIdx]);		// set finished flag
			}
			else if (dwResult == WAIT_OBJECT_0+1)
			{
				break;
			}
		}
		return 1;
	};

	fParallelProcess::fParallelProcess()
	{
		m_pThread = NULL;
		m_pThreadID = NULL;
		m_pParam = NULL;

		m_bQuit = false;
		m_bStop = false;
		m_pPercent = NULL;
		m_nSize = 0;

		m_pEventRun = NULL;
		m_pEventQuit = NULL;
		m_pEventDone = NULL;
	}

	fParallelProcess::~fParallelProcess()
	{
		Quit();
	}

	void fParallelProcess::Free()
	{
		if (m_pThread != NULL)
		{
			for (int i=0; i<m_nSize; i++)
				::CloseHandle(m_pThread[i]);
			delete []m_pThread;
		}

		if (m_pThreadID != NULL) delete []m_pThreadID;
		
		if (m_pParam != NULL) delete []m_pParam;
		
		if (m_pPercent != NULL) delete []m_pPercent;
		
		if (m_pEventRun != NULL)
		{
			for (int i=0; i<m_nSize; i++)
				::CloseHandle(m_pEventRun[i]);
			delete []m_pEventRun;				
		}
		
		if (m_pEventQuit != NULL)
		{
			for (int i=0; i<m_nSize; i++)
				::CloseHandle(m_pEventQuit[i]);
			delete []m_pEventQuit;				
		}
		
		if (m_pEventDone != NULL)
		{
			for (int i=0; i<m_nSize; i++)
				::CloseHandle(m_pEventDone[i]);
			delete []m_pEventDone;				
		}
		
		m_pThread = NULL;
		m_pThreadID = NULL;
		m_pParam = NULL;
		m_pPercent = NULL;
		m_pEventRun = NULL;
		m_pEventQuit = NULL;
		m_pEventDone = NULL;

		m_bQuit = false;
		m_bStop = false;
		m_nSize = 0;
	}

	bool fParallelProcess::Alloc(int nSize)
	{
		if (nSize == FTHREAD_NUM_CPU)
		{
			SYSTEM_INFO si;
			GetSystemInfo(&si);
			nSize = si.dwNumberOfProcessors;
		}

		Free();
		if (nSize <= 0)
		{
			m_nSize = nSize;
		}
		else 
		{
			m_pThread = new HANDLE[nSize];					if (m_pThread == NULL) return false;
			m_pThreadID = new DWORD[nSize];					if (m_pThreadID == NULL) return false;
			m_pParam = new XParamPP[nSize];					if (m_pParam == NULL) return false;
			m_pPercent = new int[nSize];					if (m_pPercent == NULL) return false;
			m_pEventRun = new HANDLE[nSize];				if (m_pEventRun == NULL) return false;
			m_pEventQuit = new HANDLE[nSize];				if (m_pEventQuit == NULL) return false;
			m_pEventDone = new HANDLE[nSize];				if (m_pEventDone == NULL) return false;

			m_nSize = nSize;
		}
		return true;
	}
	
#ifdef FGBC_OS_WIN
	void fParallelProcess::Create(int nSize, bool bSuspended)
	{
		Quit();
		Alloc(nSize);

		if (m_nSize > 0)
		{
			XParamPP* pParam = (XParamPP*)m_pParam;

			for (int i=0; i<m_nSize; i++)
			{
				m_pEventRun[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);		// auto-reset event
				m_pEventQuit[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);		// auto-reset event
				m_pEventDone[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);		// auto-reset event
			}

			for (int i=0; i<m_nSize; i++)
			{
				pParam[i].pPP = this;
				pParam[i].nIdx = i;

				m_pThread[i] = ::CreateThread(NULL, 0, fParallelProcessFunc, (void*)(&pParam[i]), bSuspended ? CREATE_SUSPENDED : 0, &(m_pThreadID[i]));
				::SetThreadAffinityMask(m_pThread[i], 1<<i);
				m_pPercent[i] = 0;
			}
		}

		m_bQuit = false;
		m_bStop = false;
	}

	bool fParallelProcess::Run()
	{
		int i;

		m_bQuit = false;
		m_bStop = false;

		if (m_nSize == 0)
		{
			Process(0);
			return true;
		}
		else
		{
			for (i=0; i<m_nSize; i++)
			{
				if (!IsValid(i)) return false;

				::ResumeThread(m_pThread[i]);
				::SetEvent(m_pEventRun[i]);
			}

			// wait all done-flag
			::WaitForMultipleObjects(m_nSize, m_pEventDone, TRUE, INFINITE);
			return true;
		}
	}

	bool fParallelProcess::Stop()
	{
		m_bStop = true;
		return true;
	}

	bool fParallelProcess::Quit()
	{
		int i;

		m_bQuit = true;
		m_bStop = true;

		if (m_nSize > 0)
		{
			for (i=0; i<m_nSize; i++)
			{
				if (!IsValid(i)) continue;

				::ResumeThread(m_pThread[i]);
				::SetEvent(m_pEventQuit[i]);

				::WaitForSingleObject(m_pThread[i], INFINITE); 
				::TerminateThread(m_pThread[i], 0);

				::CloseHandle(m_pThread[i]); 
				::CloseHandle(m_pEventRun[i]);
				::CloseHandle(m_pEventQuit[i]);
				::CloseHandle(m_pEventDone[i]);

				m_pThread[i] = NULL; 
				m_pEventRun[i] = NULL;
				m_pEventQuit[i] = NULL;
				m_pEventDone[i] = NULL;
			}

			m_bQuit = false;
			m_bStop = false;
		}

		Free();
		return true;
	}
#endif

	int fParallelProcess::GetPercent()
	{
		int nSum;
		double total;
		if (m_nSize <= 0) return 0;

		total = m_nSize*100;
		nSum = 0;
		for (int i=0; i<m_nSize; i++) nSum += m_pPercent[i];

		return (int)(nSum/total*100);
	}

#ifdef FGBC_OS_UNIX
	void fParallelProcess::Create(int nSize, bool bSuspended)
	{
		Quit();
		Alloc(nSize);

		if (m_nSize > 0)
		{
			XParamPP* pParam = (XParamPP*)m_pParam;

			for (int i=0; i<m_nSize; i++)
			{
				m_pEventRun[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);		// auto-reset event
				m_pEventQuit[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);		// auto-reset event
				m_pEventDone[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);		// auto-reset event

				pParam[i].pThread = this;
				pParam[i].nIdx = i;

				m_pThread[i] = pthread_create((pthread_t*)&m_pThreadID[i], NULL, fParallelProcessFunc, &pParam[i]);
				::SetThreadAffinityMask(m_pThread[i], 1<<i);
				m_pPercent[i] = 0;
			}
		}
	}

	bool fParallelProcess::Run()
	{
		return false;
	}

	bool fParallelProcess::Stop()
	{
		return false;
	}

	bool fParallelProcess::Quit()
	{
		return false;
	}
#endif


}	// end of namespace FCORE
}	// end of namespace FGBC