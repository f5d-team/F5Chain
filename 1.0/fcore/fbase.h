///////////////////////////////////////////////////////////////////////////////
// Developed by Curiosity laboratory 2018

#pragma once
#define FGBC_OS_WIN

#define _F5C_LIB_NAME_(PROJECT)		PROJECT ## ".lib"
#define _F5C_DLL_NAME_(PROJECT)		PROJECT ## ".dll"

///////////////////////////////////////////////////////////////////////////////
//
#ifndef F5C_DYNAMIC
#define FCORE_API  
#pragma comment(lib,_F5C_LIB_NAME_("fcore"))
#else	// DLL
#ifdef FGBCCORE_EXPORTS
#define FCORE_API __declspec(dllexport)
#else
#define FCORE_API __declspec(dllimport)
#ifndef F5CCORE_INGORE_AUTO_LINK
#pragma comment(lib,_F5C_LIB_NAME_("fcore"))
#endif
#endif	//FGBCCORE_EXPORTS
#endif	//F5C_DYNAMIC
//
///////////////////////////////////////////////////////////////////////////////

// assertion definition
#ifdef _DEBUG
#define KASSERT(f)				assert(f)
#define KVERIFY(f)				KASSERT(f)
#define KASSERT_VALID(pOb)		((void)0)			// not-yet implemented
#else
#define KASSERT(f)				((void)0) 
#define KVERIFY(f)				((void)(f))
#define KASSERT_VALID(pOb)		((void)0)
#endif



#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#define FCORE_DECLARE_DYNAMIC(class_name)	\
public:	\
	virtual void Release();	\

#define FCORE_IMPLEMENT_DYNAMIC(class_name) \
	void class_name::Release() \
{ \
	delete (class_name*)this; \
} \


namespace F5Chain
{
namespace FCORE
{


	///////////////////////////////////////////////////////////////////////////////
	// Critical Section
	class FCORE_API fCS
	{
	protected:
#ifdef FGBC_OS_WIN
		CRITICAL_SECTION m_cs;
#endif
#ifdef FGBC_OS_UNIX
		pthread_mutex	m_pm;
#endif
	public:
		fCS();
		~fCS();

		void Enter();
		void Leave();
	};

	class FCORE_API fLock
	{
	private:
		fCS*	m_pcs;
	public:
		fLock(fCS& cs);
		~fLock();
	};

	//////////////////////////////////////////////////////////////////////////////
	// KString
	struct fTagString
	{
		int		nLenData;		// length of data
		int		nLenAlloc;		// length of allocation
		LPTSTR	GetData()		{ return (LPTSTR)(this+1); };	// after this kTagString structure, string should be followed
	};

	class FCORE_API fString
	{
	public:
		TCHAR*	m_pData;

	protected:
		fTagString* CreateTagString(int nLen);
		fTagString* GetTagString() const;
		bool Alloc(int nLen);
		void CopyFrom(const fString& strSrc);
		void CopyFrom(TCHAR ch, int nRepeat = 1);
		void CopyFrom(const char* strSrc);		
		void CopyFrom(const wchar_t* strSrc);		
		void CopyFrom(const unsigned char* strSrc);
	public:
		fString();
		fString(const fString& strSrc);
		fString(TCHAR ch, int nRepeat = 1);		// from a single character
		fString(const char* strSrc);			// from an ANSI string
		fString(const wchar_t* strSrc);			// from an UNICODE string
		fString(const unsigned char* strSrc);	// from unsigned characters
		~fString();

		int		GetLength() const;
		int		GetLengthAlloc() const;
		int		SetLength(int nLenNew);
		bool	IsEmpty() const;
		void	Empty();
		TCHAR	GetAt(int nIndex) const			{ KASSERT(nIndex >= 0 && nIndex < GetLength()); return m_pData[nIndex]; };	// return single character at zero-based index
		TCHAR	operator[](int nIndex) const	{ KASSERT(nIndex >= 0 && nIndex < GetLength()); return m_pData[nIndex]; };	// return single character at zero-based index
		void	SetAt(int nIndex, TCHAR ch)		{ KASSERT(nIndex >= 0 && nIndex < GetLength()); m_pData[nIndex] = ch; };		// set a single character at zero-based index
		LPTSTR	GetBuffer(int nMinBufLength); 		// get pointer to modifiable buffer at least as long as nMinBufLength
		LPTSTR	GetBuffer();
		LPTSTR	GetBuffer() const;
		operator LPCTSTR() const				{ return m_pData; };
		operator LPTSTR() const					{ return m_pData; };

		// overloaded assignment
		fString& operator=(const fString& strSrc);
		fString& operator=(TCHAR ch);
		fString& operator=(const char* strSrc);
		fString& operator=(const wchar_t* strSrc);
		fString& operator=(const unsigned char* strSrc);

		// string concatenation
		fString& operator+=(const fString& str);
		fString& operator+=(TCHAR ch);
		fString& operator+=(LPCTSTR str);

		friend FCORE_API fString operator+(const fString& str1, const fString& str2);
		friend FCORE_API fString operator+(const fString& str, TCHAR ch);
		friend FCORE_API fString operator+(TCHAR ch, const fString& str);
		friend FCORE_API fString operator+(const fString& str1, LPCTSTR str2);
		friend FCORE_API fString operator+(LPCTSTR str1, const fString& str2);

		// string comparison
		int Compare(LPCTSTR str) const;
		int CompareNoCase(LPCTSTR str) const;
		int Collate(LPCTSTR str) const;
		int CollateNoCase(LPCTSTR str) const;

		// simple sub-string extraction
		fString Mid(int nFirst) const;
		fString Mid(int nFirst, int nCount) const;
		fString Left(int nCount) const;
		fString Right(int nCount) const;

		// upper/lower/reverse conversion
		void MakeUpper();
		void MakeLower();
		void MakeReverse();

		// trimming whitespace
		void TrimLeft();
		void TrimRight();

		// trimming anything
		void TrimLeft(TCHAR ch);
		void TrimRight(TCHAR ch);
		void TrimLeft(LPCTSTR str);
		void TrimRight(LPCTSTR str);

		// advanced manipulation
		int Replace(TCHAR chOld, TCHAR chNew);
		int Replace(LPCTSTR strOld, LPCTSTR strNew);
		int Remove(TCHAR ch);
		int Insert(int nIndex, TCHAR ch);
		int Insert(int nIndex, LPCTSTR str);
		int Delete(int nIndex, int nCount = 1);

		// searching
		int Find(TCHAR ch, int nStart = 0) const;
		int Find(LPCTSTR str, int nStart = 0) const;
		int FindOneOf(LPCTSTR str) const;
		int ReverseFind(TCHAR ch) const;


		// formatting
		void __cdecl Format(LPCTSTR strFormat, ...);
		void FormatV(LPCTSTR lpszFormat, va_list argList);
	};

	// Compare helpers
	bool FCORE_API operator==(const fString& s1, const fString& s2);
	bool FCORE_API operator==(const fString& s1, LPCTSTR s2);
	bool FCORE_API operator==(const fString& s1, LPTSTR s2);
	bool FCORE_API operator==(LPCTSTR s1, const fString& s2);
	bool FCORE_API operator==(LPCSTR s1, const fString& s2);

	bool FCORE_API operator!=(const fString& s1, const fString& s2);
	bool FCORE_API operator!=(const fString& s1, LPCTSTR s2);
	bool FCORE_API operator!=(const fString& s1, LPTSTR s2);
	bool FCORE_API operator!=(LPCTSTR s1, const fString& s2);
	bool FCORE_API operator!=(LPTSTR s1, const fString& s2);

	bool FCORE_API operator<(const fString& s1, const fString& s2);
	bool FCORE_API operator<(const fString& s1, LPCTSTR s2);
	bool FCORE_API operator<(const fString& s1, LPTSTR s2);
	bool FCORE_API operator<(LPCTSTR s1, const fString& s2);
	bool FCORE_API operator<(LPTSTR s1, const fString& s2);

	bool FCORE_API operator>(const fString& s1, const fString& s2);
	bool FCORE_API operator>(const fString& s1, LPCTSTR s2);
	bool FCORE_API operator>(const fString& s1, LPTSTR s2);
	bool FCORE_API operator>(LPCTSTR s1, const fString& s2);
	bool FCORE_API operator>(LPTSTR s1, const fString& s2);

	bool FCORE_API operator<=(const fString& s1, const fString& s2);
	bool FCORE_API operator<=(const fString& s1, LPCTSTR s2);
	bool FCORE_API operator<=(const fString& s1, LPTSTR s2);
	bool FCORE_API operator<=(LPCTSTR s1, const fString& s2);
	bool FCORE_API operator<=(LPTSTR s1, const fString& s2);

	bool FCORE_API operator>=(const fString& s1, const fString& s2);
	bool FCORE_API operator>=(const fString& s1, LPCTSTR s2);
	bool FCORE_API operator>=(const fString& s1, LPTSTR s2);
	bool FCORE_API operator>=(LPCTSTR s1, const fString& s2);
	bool FCORE_API operator>=(LPTSTR s1, const fString& s2);


	///////////////////////////////////////////////////////////////////////////////
	// Thread
	class FCORE_API fThread
	{
	protected:
		HANDLE	m_hThread;
		DWORD	m_dwThreadID;
		bool	m_bQuit;
		bool	m_bStop;
		int		m_nPercent;
	public:
		HANDLE	m_hEventRun;
		HANDLE	m_hEventQuit;

	public:
		fThread();
		virtual ~fThread();
		HANDLE	Create(bool bSuspended = false);
		bool	Run();
		bool	Stop();
		bool	Quit();
		bool	IsValid()			{ return (m_hThread != NULL); };
		DWORD	GetThreadID()		{ return m_dwThreadID; };
		bool	IsTimeToExit()		{ return m_bQuit || m_bStop; };

		// pure virtual function for processing
		virtual void Process() = 0;

		// message functions
		virtual void OnMessagePercent(int nPercent) { m_nPercent = nPercent; };
		virtual int	GetPercent()					{ return m_nPercent; };
	};
	const long FTHREAD_NUM_CPU = -1;


	class FCORE_API fParallelProcess
	{
	protected:
		HANDLE*	m_pThread;
		DWORD*	m_pThreadID;
		void*	m_pParam;
		bool	m_bQuit;
		bool	m_bStop;
		int*	m_pPercent;
		int		m_nSize;

		void	Free();
		bool	Alloc(int nSize);
	public:
		HANDLE*	m_pEventRun;
		HANDLE*	m_pEventQuit;
		HANDLE*	m_pEventDone;

	public:
		fParallelProcess();
		virtual ~fParallelProcess();
		void	Create(int nSize, bool bSuspended = false);
		bool	Run();
		bool	Stop();
		bool	Quit();
		bool	IsValid(int nIdx)			{ return (nIdx < m_nSize) && (m_pThread[nIdx] != NULL); };
		DWORD	GetThreadID(int nIdx)		{ if (nIdx >= m_nSize) return 0; else return m_pThreadID[nIdx]; };
		int		GetNumThreads()				{ return m_nSize; };
		bool	IsTimeToExit()				{ return m_bQuit || m_bStop; };

		// pure virtual function for processing
		virtual void Process(int nIdx) = 0;

		// message functions
		virtual void OnMessagePercent(int nIdx, int nPercent)	{ if (nIdx < m_nSize) m_pPercent[nIdx] = nPercent; };
		virtual int	GetPercent(int nIdx)						{ if (nIdx >= m_nSize) return 0; else return m_pPercent[nIdx]; };
		virtual int	GetPercent();			// total percent
	};

	////////////////////////////////////////////////////////////////////////////////
	// Generic Object Handling
	template<class TYPE>
	void ConstructObjects(TYPE* pElements, int nCount)
	{
		memset((void*)pElements, 0, nCount * sizeof(TYPE));
		for (; nCount--; pElements++) ::new((void*)pElements) TYPE;		// call the constructor(s) by "placement new" operator
	};

	template<class TYPE>
	void DestructObjects(TYPE* pElements, int nCount)
	{
		for (; nCount--; pElements++) pElements->~TYPE();				// call the destructor(s)
	};

	template<class TYPE>
	void CopyObjects(TYPE* pDest, const TYPE* pSrc, int nCount)
	{
		while (nCount--) *pDest++ = *pSrc++;							// element-copy using assignment
	};

	//////////////////////////////////////////////////////////////////////////////
	// Array
	template<class TYPE>
	class fArray
	{
	protected:
		TYPE*		m_pData;
		int			m_nSize;
		int			m_nMaxSize;
		int			m_nGrowBy;

	public:
		fArray(const fArray& src)	{ m_pData = NULL; m_nSize = m_nMaxSize = m_nGrowBy = 0; Copy(src); };
	public:
		fArray()			{ m_pData = NULL; m_nSize = m_nMaxSize = m_nGrowBy = 0; };
		virtual ~fArray()	{ if (m_pData != NULL) { delete[] (BYTE*)m_pData; } };


		int	 GetSize() const						{ return m_nSize; };
		int	 GetUpperBound() const					{ return m_nSize-1; };
		void SetSize(int nNewSize, int nGrowBy = -1)
		{
			KASSERT(nNewSize >= 0);

			if (nGrowBy != -1) m_nGrowBy = nGrowBy;
			if (nNewSize == 0)
			{
				if (m_pData != NULL)
				{
					DestructObjects<TYPE>(m_pData, m_nSize);
					delete[] (BYTE*)m_pData;
					m_pData = NULL;
				}
				m_nSize = m_nMaxSize = 0;
				return;
			}

			if (m_pData == NULL)
			{
				int nAlloc = nNewSize * sizeof(TYPE);
				if (nAlloc / nNewSize == sizeof(TYPE))
				{
					m_pData = (TYPE*) new BYTE[nAlloc];
					ConstructObjects<TYPE>(m_pData, nNewSize);
					m_nSize = m_nMaxSize = nNewSize;
				}
				return;
			}

			if (nNewSize <= m_nMaxSize)
			{
				if (nNewSize > m_nSize)	ConstructObjects<TYPE>(m_pData+m_nSize, nNewSize-m_nSize);		// call constructor from (m_nSize) to (nNewSize-1)
				else if (m_nSize > nNewSize)	DestructObjects<TYPE>(m_pData+nNewSize, m_nSize-nNewSize);		// call destructor from (nNewSize) to (m_nSize-1)
				m_nSize = nNewSize;
			}
			else
			{
				int nGrowBy, nNewMax, nAlloc;

				nGrowBy = m_nGrowBy;
				if (nGrowBy == 0)
				{
					nGrowBy = m_nSize / 8;
					if (nGrowBy < 4) nGrowBy = 4;
					if (nGrowBy > 1024) nGrowBy = 1024;
				}
				if (nNewSize < m_nMaxSize + nGrowBy)	nNewMax = m_nMaxSize + nGrowBy;		// granularity
				else									nNewMax = nNewSize;					// no slush

				nAlloc = nNewMax * sizeof(TYPE);
				if (nAlloc / nNewMax == sizeof(TYPE))
				{
					TYPE* pNewData = (TYPE*) new BYTE[nAlloc];
					memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));		// copy current data to new buffer

					// call constructor from (m_nSize) to (nNewSize-1)
					ConstructObjects<TYPE>(pNewData+m_nSize, nNewSize-m_nSize);	

					// delete current data buffer and make pointing new buffer (note: no destructor called)
					delete[] (BYTE*)m_pData;
					m_pData = pNewData;
					m_nSize = nNewSize;
					m_nMaxSize = nNewMax;
				}
			}
		};

		void  FreeExtra()
		{
			if (m_nSize != m_nMaxSize)
			{
				TYPE* pNewData = NULL;
				if (m_nSize != 0)
				{
					pNewData = (TYPE*) new BYTE[m_nSize * sizeof(TYPE)];
					memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));	// copy new current data to new buffer
				}

				// delete current data buffer and make pointing new buffer (note: no destructor called)
				delete[] (BYTE*)m_pData;
				m_pData = pNewData;
				m_nMaxSize = m_nSize;
			}
		};
		void  RemoveAll(bool bRemainBuffer = false)	
		{ 
			if (bRemainBuffer)
			{
				if (m_nSize > 0 && m_pData != NULL) DestructObjects<TYPE>(m_pData, m_nSize);		// call destructor for all elements
				m_nSize = 0;
			}
			else SetSize(0, -1); 
		};

		TYPE  GetAt(int nIndex) const				{ KASSERT(nIndex >= 0 && nIndex < m_nSize); return m_pData[nIndex]; };
		void  SetAt(int nIndex, TYPE newElement)	{ KASSERT(nIndex >= 0 && nIndex < m_nSize); m_pData[nIndex] = newElement; };
		TYPE& ElementAt(int nIndex)					{ KASSERT(nIndex >= 0 && nIndex < m_nSize); return m_pData[nIndex]; };

		const TYPE*	GetData() const					{ return (const TYPE*)m_pData; };
		TYPE*		GetData()						{ return (TYPE*)m_pData; };

		void SetAtGrow(int nIndex, TYPE newElement) { KASSERT(nIndex >= 0); if (nIndex >= m_nSize) SetSize(nIndex+1, -1); if (m_pData != NULL) m_pData[nIndex] = newElement; };
		int	 Add(TYPE newElement)					{ int nIndex = m_nSize; SetAtGrow(nIndex, newElement); return nIndex; };
		int	 Append(const fArray& src)
		{
			KASSERT(this != &src);   // cannot append to itself
			int nOldSize = m_nSize;
			SetSize(m_nSize + src.m_nSize);
			CopyObjects<TYPE>(m_pData + nOldSize, src.m_pData, src.m_nSize);
			return nOldSize;
		};
		void Copy(const fArray& src) { KASSERT(this != &src); SetSize(src.m_nSize); CopyObjects<TYPE>(m_pData, src.m_pData, src.m_nSize); };

		void InsertAt(int nIndex, TYPE newElement, int nCount = 1)
		{
			KASSERT(nIndex >= 0);
			KASSERT(nCount > 0);

			if (nIndex >= m_nSize)
			{
				// adding after the end of the array
				SetSize(nIndex + nCount, -1);						// grow so nIndex is valid
			}
			else
			{
				int nSizeOld = m_nSize;
				SetSize(m_nSize + nCount, -1);														// grow it to new size
				DestructObjects<TYPE>(m_pData+nSizeOld, nCount);									// destroy initial data before copying over it
				memmove(m_pData+nIndex+nCount, m_pData+nIndex, (nSizeOld-nIndex) * sizeof(TYPE));	// shift old data up to fill gap
				ConstructObjects<TYPE>(m_pData+nIndex, nCount);										// re-init slots we copied from
			}

			// insert new value in the gap
			KASSERT(nIndex + nCount <= m_nSize);
			while (nCount--) m_pData[nIndex++] = newElement;
		};
		void RemoveAt(int nIndex, int nCount = 1)
		{
			KASSERT(nIndex >= 0);
			KASSERT(nCount >= 0);
			KASSERT(nIndex + nCount <= m_nSize);

			// just remove a range
			int nMoveCount = m_nSize - (nIndex + nCount);
			DestructObjects<TYPE>(&m_pData[nIndex], nCount);
			if (nMoveCount)
				memmove(&m_pData[nIndex], &m_pData[nIndex + nCount], nMoveCount * sizeof(TYPE));
			m_nSize -= nCount;
		};
		void InsertAt(int nStartIndex, fArray* pNewArray)
		{
			KASSERT(pNewArray != NULL);
			KASSERT_VALID(pNewArray);
			KASSERT(nStartIndex >= 0);

			if (pNewArray->GetSize() > 0)
			{
				InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
				for (int i=0; i<pNewArray->GetSize(); i++) SetAt(nStartIndex + i, pNewArray->GetAt(i));
			}
		};

		TYPE		operator[](int nIndex) const			{ return GetAt(nIndex); };
		TYPE&		operator[](int nIndex)					{ return ElementAt(nIndex); };

		fArray& operator=(const fArray& src) { Copy(src);	return*this; }
	};
}
}