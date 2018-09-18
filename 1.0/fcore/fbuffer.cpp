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

	fBuffer::fBuffer()
	{
		m_ByteTransferred = 0;
	}

	fBuffer::~fBuffer()
	{

	}

	char* fBuffer::GetDataPtr()
	{
		return NULL;
	}

	UINT fBuffer::GetBufferSize()
	{
		return 0;
	}

}	// end of namespace FCORE
}	// end of namespace FGBC