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

	fService::fService()
	{
		m_pIOS = NULL;
		m_bAsynchronous = false;
		m_pAcceptor = NULL;
	}

	fService::~fService()
	{
		Clear();
		m_pIOS = NULL;
	}

	bool fService::Clear()
	{
		SAFE_DELETE(m_pAcceptor);
		return true;
	}

	bool fService::IsUsable(fString* pError)
	{
		if(!m_pIOS)
		{
			if(pError)	*pError = _FERROR_UNREGISTEDSERVICE_;
			return false;
		}
		return true;
	}


	bool fService::Listen(tcp::endpoint addr, bool bAsynchronous, fString* pError)
	{
		if(!Clear())					return false;
		if(!IsUsable(pError))		return false;

		m_Address = addr;
		m_bAsynchronous = bAsynchronous;

		m_pAcceptor = new tcp::acceptor(*m_pIOS,m_Address);

		// socket object
		fSocketSPtr pSocket(new tcp::socket(*m_pIOS));
		if(bAsynchronous)
		{
			// only error_code param at touched event, so use boost::bind to bind socket
			m_pAcceptor->async_accept(*pSocket,
				boost::bind(&fService::AcceptHandler, this, pSocket, _1));			
		}
		else
		{
			// wait client
			boost::system::error_code ec;
			m_pAcceptor->accept(*pSocket,ec);
			return OnAccepted(pSocket,ec);
		}
		return true;
	}

	bool fService::Connect(tcp::endpoint addr, bool bAsynchronous, fString* pError)
	{
		if(!Clear())				return false;
		if(!IsUsable(pError))	return false;

		m_Address = addr;
		m_bAsynchronous = bAsynchronous;

		// socket object
		fSocketSPtr pSocket(new tcp::socket(*m_pIOS));
		boost::system::error_code ec;
		if(bAsynchronous)
		{
			pSocket->async_connect(m_Address,
				boost::bind(&fService::ConnectHandler, this, pSocket, _1));	
		}
		else
		{
			pSocket->connect(m_Address,ec);
			return OnConnected(pSocket,ec);
		}
		return true;
	}

	bool fService::Register(boost::asio::io_service* pIOS, fString* pError)
	{
		if(!Clear())	return false;
		if(!pIOS)
		{
			if(pError)	*pError = _FERROR_INVALID_HANDLE_;
			return false;
		}

		m_pIOS = pIOS;
		return true;
	}
	
	bool fService::Read(fSocketSPtr pSocket, fBuffer* const pBuffers, fString* pError)
	{
		if(!IsUsable(pError))		return false;
		if(!pSocket)
		{
			if(pError)	*pError = _FERROR_INVALID_CONNECTION_;
			return false;
		}

		if(!pBuffers  || !pBuffers->GetDataPtr() || pBuffers->GetBufferSize() == 0)
		{
			if(pError)	*pError = _FERROR_INVALID_BUFFER_;
			return false;
		}

		if(pBuffers->m_ByteTransferred > pBuffers->GetBufferSize())
		{
			if(pError)	*pError = _FERROR_INVALID_DATA_;
			return false;
		}

		if(m_bAsynchronous)
		{			
			pSocket->async_receive(
				buffer(pBuffers->GetDataPtr(),pBuffers->GetBufferSize()),
				boost::bind(&fService::ReadHandler,
				this,
				pSocket,
				boost::asio::placeholders::error,
				pBuffers,
				boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			boost::asio::socket_base::message_flags msg_flags = 0;
			boost::system::error_code ec;
			pBuffers->m_ByteTransferred = pSocket->receive(
				buffer(pBuffers->GetDataPtr(),pBuffers->GetBufferSize()),
				msg_flags,
				ec);

			if(ec && pError)
			{
				*pError = boost::system::system_error(ec).what();
				return false;
			}
			return OnReadSomeOver(pSocket, pBuffers, ec);
		}
		return true;
	}

	bool fService::ReadSome(fSocketSPtr pSocket, fBuffer* const pBuffers, fString* pError)
	{
		if(!IsUsable(pError))		return false;
		if(!pSocket)
		{
			if(pError)	*pError = _FERROR_INVALID_CONNECTION_;
			return false;
		}
		
		if(!pBuffers  || !pBuffers->GetDataPtr() || pBuffers->GetBufferSize() == 0)
		{
			if(pError)	*pError = _FERROR_INVALID_BUFFER_;
			return false;
		}

		if(pBuffers->m_ByteTransferred > pBuffers->GetBufferSize())
		{
			if(pError)	*pError = _FERROR_INVALID_DATA_;
			return false;
		}
		char*	pBufferStart = pBuffers->GetDataPtr() + pBuffers->m_ByteTransferred;
		UINT	nBufferSize = pBuffers->GetBufferSize() - pBuffers->m_ByteTransferred;

		if(m_bAsynchronous)
		{			
			pSocket->async_read_some(
				buffer(pBufferStart,nBufferSize),
				boost::bind(&fService::ReadHandler,
							this,
							pSocket,
							boost::asio::placeholders::error,
							pBuffers,
							boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			boost::system::error_code ec;
			pBuffers->m_ByteTransferred += pSocket->read_some(
				buffer(pBufferStart,nBufferSize), ec);

			if(ec && pError)
			{
				*pError = boost::system::system_error(ec).what();
				return false;
			}
			return OnReadSomeOver(pSocket, pBuffers, ec);
		}
		return true;
	}

	bool fService::ReadSome(fSocketSPtr pSocket, fBufferSPtr pBuffers, fString* pError)
	{
		if(!IsUsable(pError))		return false;
		if(!pSocket)
		{
			if(pError)	*pError = _FERROR_INVALID_CONNECTION_;
			return false;
		}

		if(!pBuffers  || !pBuffers->GetDataPtr() || pBuffers->GetBufferSize() == 0)
		{
			if(pError)	*pError = _FERROR_INVALID_BUFFER_;
			return false;
		}


		if(pBuffers->m_ByteTransferred > pBuffers->GetBufferSize())
		{
			if(pError)	*pError = _FERROR_INVALID_DATA_;
			return false;
		}
		char*	pBufferStart = pBuffers->GetDataPtr() + pBuffers->m_ByteTransferred;
		UINT	nBufferSize = pBuffers->GetBufferSize() - pBuffers->m_ByteTransferred;

		if(m_bAsynchronous)
		{
			pSocket->async_read_some(
				buffer(pBufferStart,nBufferSize),
				boost::bind(&fService::ReadHandler1,
				this,
				pSocket,
				boost::asio::placeholders::error,
				pBuffers,
				boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			boost::system::error_code ec;
			pBuffers->m_ByteTransferred = pSocket->read_some(
				buffer(pBufferStart,nBufferSize), ec);

			if(ec && pError)
			{
				*pError = boost::system::system_error(ec).what();
				return false;
			}
			return OnReadSomeOver(pSocket, pBuffers, ec);
		}
		return true;
	}

	bool fService::WriteSome(fSocketSPtr pSocket, fBuffer* const pBuffers, fString* pError)
	{
		if(!IsUsable(pError))		return false;		
		if(!pSocket)
		{
			if(pError)	*pError = _FERROR_INVALID_CONNECTION_;
			return false;
		}

		if(!pBuffers  || !pBuffers->GetDataPtr() || pBuffers->GetBufferSize() == 0)
		{
			if(pError)	*pError = _FERROR_INVALID_BUFFER_;
			return false;
		}

		if(m_bAsynchronous)
		{
			pBuffers->m_ByteTransferred = 0;
			pSocket->async_write_some(
				buffer(pBuffers->GetDataPtr(),pBuffers->GetBufferSize()),
				boost::bind(&fService::WriteHandler,
							this,
							pSocket,
							boost::asio::placeholders::error,
							pBuffers,
							boost::asio::placeholders::bytes_transferred));			
		}
		else
		{
			boost::system::error_code ec;
			pBuffers->m_ByteTransferred = pSocket->write_some(
				buffer(pBuffers->GetDataPtr(),pBuffers->GetBufferSize()), ec);

			if(ec && pError)
			{
				*pError = boost::system::system_error(ec).what();
				return false;
			}
			return OnWriteSomeOver(pSocket, pBuffers, ec);
		}
		return true;
	}

	bool fService::WriteSome(fSocketSPtr pSocket, fBufferSPtr pBuffers, fString* pError)
	{
		if(!IsUsable(pError))		return false;		
		if(!pSocket)
		{
			if(pError)	*pError = _FERROR_INVALID_CONNECTION_;
			return false;
		}

		if(!pBuffers  || !pBuffers->GetDataPtr() || pBuffers->GetBufferSize() == 0)
		{
			if(pError)	*pError = _FERROR_INVALID_BUFFER_;
			return false;
		}

		if(m_bAsynchronous)
		{
			pBuffers->m_ByteTransferred = 0;
			pSocket->async_write_some(
				buffer(pBuffers->GetDataPtr(),pBuffers->GetBufferSize()),
				boost::bind(&fService::WriteHandler1,
				this,
				pSocket,
				boost::asio::placeholders::error,
				pBuffers,
				boost::asio::placeholders::bytes_transferred));			
		}
		else
		{
			boost::system::error_code ec;
			pBuffers->m_ByteTransferred = pSocket->write_some(
				buffer(pBuffers->GetDataPtr(),pBuffers->GetBufferSize()), ec);

			if(ec && pError)
			{
				*pError = boost::system::system_error(ec).what();
				return false;
			}
			return OnWriteSomeOver(pSocket, pBuffers, ec);
		}
		return true;
	}

	bool fService::AcceptHandler(fSocketSPtr pSocket, error_code ec)
	{
		if(!ec)
		{
			// socket object
			fSocketSPtr pNewSocket(new tcp::socket(*m_pIOS));
			if(m_bAsynchronous)
			{
				// only error_code param at touched event, so use boost::bind to bind socket
				m_pAcceptor->async_accept(*pNewSocket,
					boost::bind(&fService::AcceptHandler, this, pNewSocket, _1));			
			}
			else
			{
				// wait client
				boost::system::error_code ec;
				m_pAcceptor->accept(*pNewSocket,ec);
				return OnAccepted(pNewSocket,ec);
			}
		}
			

		return OnAccepted(pSocket,ec);
	}


	bool fService::ConnectHandler(fSocketSPtr pSocket, error_code ec)
	{
		return OnConnected(pSocket,ec);
	}

	bool fService::ReadHandler(fSocketSPtr pSocket, 
		const boost::system::error_code& ec, fBuffer* pBuffers, std::size_t bytes_transferred)
	{
		pBuffers->m_ByteTransferred += bytes_transferred;
		return OnReadSomeOver(pSocket, pBuffers, ec);
	}

	bool fService::ReadHandler1(fSocketSPtr pSocket, 
		const boost::system::error_code& ec, fBufferSPtr pBuffers, std::size_t bytes_transferred)
	{
		pBuffers->m_ByteTransferred += bytes_transferred;
		return OnReadSomeOver(pSocket, pBuffers, ec);
	}
	
	bool fService::WriteHandler(fSocketSPtr pSocket, 
		const boost::system::error_code& ec, fBuffer* pBuffers, std::size_t bytes_transferred)
	{
		pBuffers->m_ByteTransferred += bytes_transferred;
		return OnWriteSomeOver(pSocket, pBuffers, ec);
	}

	bool fService::WriteHandler1(fSocketSPtr pSocket, 
		const boost::system::error_code& ec, fBufferSPtr pBuffers, std::size_t bytes_transferred)
	{
		pBuffers->m_ByteTransferred += bytes_transferred;
		return OnWriteSomeOver(pSocket, pBuffers, ec);
	}

	bool fService::OnAccepted(fSocketSPtr pSocket, error_code ec)
	{
		return true;
	}

	bool fService::OnConnected(fSocketSPtr pSocket, error_code ec)
	{
		return true;
	}

	bool fService::OnReadSomeOver(fSocketSPtr pSocket, fBuffer* const pBuffers, error_code ec)
	{
		return true;
	}

	bool fService::OnReadSomeOver(fSocketSPtr pSocket, fBufferSPtr pBuffers, error_code ec)
	{
		return true;
	}

	bool fService::OnWriteSomeOver(fSocketSPtr pSocket, fBuffer* const pBuffers, error_code ec)
	{
		return true;
	}

	bool fService::OnWriteSomeOver(fSocketSPtr pSocket, fBufferSPtr pBuffers, error_code ec)
	{
		return true;
	}


	FCORE_IMPLEMENT_DYNAMIC(fService)


}	// end of namespace FCORE
}	// end of namespace FGBC