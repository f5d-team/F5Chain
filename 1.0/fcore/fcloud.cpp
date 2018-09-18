#include "stdafx.h"
#include "fcore.h"

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

	fCloud::fCloud()		
	{
		m_pWorker = NULL;
	}

	fCloud::~fCloud()
	{
		Stop();
		for(int i=0;i<m_Services.GetSize();i++)
		{
			SAFE_RELEASE(m_Services[i]);
		}
	}
	
	bool fCloud::RegisterService(fService* pService, fString* pError)
	{
		if(!pService)
		{
			if(pError)	*pError = _FERROR_INVALID_HANDLE_;
			return false;
		}
		if(!pService->Register(&m_IOS,pError))
		{
			return false;
		}

		m_Services.Add(pService);

		return true;
	}




	bool fCloud::Run()
	{
		Stop();
		m_pWorker = new boost::asio::io_service::work(m_IOS);
		m_IOS.reset();
		m_IOS.run();
		return true;
	}

	bool fCloud::RunByBackThread()
	{
		Stop();
		m_pWorker = new boost::asio::io_service::work(m_IOS);
		fThread::Create(false);
		fThread::Run();
		return true;
	}

	bool fCloud::Stop()
	{
		SAFE_DELETE(m_pWorker);
		m_IOS.stop();
		fThread::Quit();
		m_IOS.stop();
		return true;
	}

	void fCloud::Process()
	{
		m_IOS.reset();
		while(!IsTimeToExit())
		{
			m_IOS.run();
			Sleep(1);
		}

	}

}
}