///////////////////////////////////////////////////////////////////////////////
// Developed by Curiosity laboratory 2018

#pragma once
#include "fbase.h"

#ifdef FGBCCORE_DYNAMIC

#if defined(WIN64)
#include <afx.h>

#pragma warning(push,0)
#pragma warning(disable:4800)				// when include boost, to avoid display warning c4800 'int' forcing value to bool 'true' or 'false' (performance warning)
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#pragma warning(pop)

#undef WIN32
#define WIN64
#else ifdef WIN32
#pragma warning(push,0)
#pragma warning(disable:4800)
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#pragma warning(pop)
#endif

#else

#pragma warning(push,0)
#pragma warning(disable:4800)
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#pragma warning(pop)

#endif


namespace F5Chain
{
namespace FCORE
{

	using namespace boost::asio;
	using boost::system::error_code;
	using ip::tcp;


///////////////////////////////////////////////////////////////////////////////
// KNSError String
#define _FERROR_UNREGISTEDSERVICE_		_T("This Service not registed")
#define _FERROR_INVALID_HANDLE_			_T("Invalid io_service handle")
#define _FERROR_INVALID_CONNECTION_		_T("Invalid connection")
#define _FERROR_INVALID_BUFFER_			_T("Invalid buffer")
#define _FERROR_INVALID_DATA_			_T("Invalid data")

///////////////////////////////////////////////////////////////////////////////
// Protocol base
	class FCORE_API fBuffer
	{
	public:
		size_t		m_ByteTransferred;
	public:
		fBuffer();
		~fBuffer();

		virtual char*	GetDataPtr();
		virtual UINT	GetBufferSize();
	};

	typedef boost::shared_ptr<tcp::socket>	fSocketSPtr;
	typedef boost::shared_ptr<fBuffer>	fBufferSPtr;

///////////////////////////////////////////////////////////////////////////////
// Service base
	class FCORE_API fService
	{
		FCORE_DECLARE_DYNAMIC(fService)
	private:
		io_service*		m_pIOS;

		tcp::endpoint	m_Address;
		bool			m_bAsynchronous;

		tcp::acceptor*	m_pAcceptor;
	public:
		fService();
		~fService();

		// Call able
		bool Listen		(tcp::endpoint addr, bool bAsynchronous, fString* pError = NULL);
		bool Connect	(tcp::endpoint addr, bool bAsynchronous, fString* pError = NULL);

		// User controlled pointer
		bool Read		(fSocketSPtr pSocket, fBuffer* const pBuffers, fString* pError = NULL);//need test
		//bool Write		(KNSSocketSPtr pSocket, KNSBuffer* const pBuffers, KString* pError = nullptr);
		bool ReadSome	(fSocketSPtr pSocket, fBuffer* const pBuffers, fString* pError = NULL);
		bool WriteSome	(fSocketSPtr pSocket, fBuffer* const pBuffers, fString* pError = NULL);
		
		// Smart pointer
		//bool Read		(KNSSocketSPtr pSocket, KNSBufferSPtr pBuffers, KString* pError = nullptr);
		//bool Write		(KNSSocketSPtr pSocket, KNSBufferSPtr pBuffers, KString* pError = nullptr);
		bool ReadSome	(fSocketSPtr pSocket, fBufferSPtr pBuffers, fString* pError = NULL);
		bool WriteSome	(fSocketSPtr pSocket, fBufferSPtr pBuffers, fString* pError = NULL);
		
	public:
		bool Register	(boost::asio::io_service* pIOS, fString* pError = NULL);
	private:
		bool Clear();
		bool IsUsable(fString* pError = NULL);

		// Asynchronous event handles
		bool AcceptHandler	(fSocketSPtr pSocket, error_code ec);
		bool ConnectHandler	(fSocketSPtr pSocket, error_code ec);
		bool ReadHandler	(fSocketSPtr pSocket, const boost::system::error_code& ec, fBuffer* pBuffers, std::size_t bytes_transferred);
		bool WriteHandler	(fSocketSPtr pSocket, const boost::system::error_code& ec, fBuffer* pBuffers, std::size_t bytes_transferred);
		bool ReadHandler1	(fSocketSPtr pSocket, const boost::system::error_code& ec, fBufferSPtr pBuffers, std::size_t bytes_transferred);
		bool WriteHandler1	(fSocketSPtr pSocket, const boost::system::error_code& ec, fBufferSPtr pBuffers, std::size_t bytes_transferred);
	private:
		// Override able
		// Event
		virtual bool OnAccepted		(fSocketSPtr pSocket, error_code ec);
		virtual bool OnConnected	(fSocketSPtr pSocket, error_code ec);
		virtual bool OnReadSomeOver	(fSocketSPtr pSocket, fBuffer* const pBuffers, error_code ec);
		virtual bool OnReadSomeOver	(fSocketSPtr pSocket, fBufferSPtr pBuffers, error_code ec);
		virtual bool OnWriteSomeOver(fSocketSPtr pSocket, fBuffer* const pBuffers, error_code ec);
		virtual bool OnWriteSomeOver(fSocketSPtr pSocket, fBufferSPtr pBuffers, error_code ec);
	}; 
	typedef	fArray<fService*>	KNSServiceArray;

///////////////////////////////////////////////////////////////////////////////
// Cloud holder
	class FCORE_API fCloud : public fThread
	{
	private:
		boost::asio::io_service::work*	m_pWorker;
		boost::asio::io_service			m_IOS;
		KNSServiceArray					m_Services;
	public:
		fCloud();
		~fCloud();

		bool	RegisterService(fService* pService, fString* pError = NULL);

		bool	Run();
		bool	RunByBackThread();

		bool	Stop();
	private:
		void	Process();
	};

	
}
}