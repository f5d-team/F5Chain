#include "stdafx.h"
#include "fbase.h"
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

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

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

///////////////////////////////////////////////////////////////////////////////
// String Utility Functions
size_t __StrLen(LPCTSTR str)
{
#ifdef _UNICODE
	return (str == NULL) ? 0 : wcslen(str);
#else
	return (str == NULL) ? 0 : strlen(str);
#endif
}

LPCTSTR __StrInc(LPCTSTR str)
{
#ifdef _UNICODE
	return str+1; 
#else
	if (*str & 0x80) return str+2; else return str+1;
#endif
}

LPCTSTR __StrDec(LPCTSTR str)
{
#ifdef _UNICODE
	return str-1; 
#else
	if (*(str-2) & 0x80) return str-2; else return str-1;
#endif
}

int __StrToInt(LPCTSTR str)
{
#ifdef _UNICODE
	return _wtoi(str);
#else
	return atoi(str);
#endif
}

int __IsDigit(TCHAR c)
{
#ifdef _UNICODE
	return iswdigit(c);
#else
	return isdigit(c);
#endif
}

int __StrnCmp(LPCTSTR str1, LPCTSTR str2, size_t count)
{
#ifdef _UNICODE
	return wcsncmp(str1, str2, count);
#else
	return strncmp(str1, str2, count);
#endif
}

// Boyer-Moore String search algorithm
int __bad_char(LPCTSTR str, TCHAR ch, int pos)
{
	for (int i=pos; i>=0; i--) if (str[i] == ch) return i;
	return -1;
}

int __good_suffix(LPCTSTR str, int pos)
{
	int i, j, len;
	len = (int)__StrLen(str);
	for (i=len-2; i>=(len-pos-2); i--)
	{
		for(j=0; j<(len-pos-1); j++) { if (str[i-j] != str[len-j-1]) break; }
		if (j>=(len-pos-1)) return i-(len-pos)+1;
	}
	return -1;
}

int __StrSearch(LPCTSTR str1, LPCTSTR str2)
{
	int i, j, len1, len2;
	len1 = (int)__StrLen(str1);
	len2 = (int)__StrLen(str2);
	for (i=0; i<=(len1-len2);)
	{
		for (j=len2-1; j>=0 && str2[j] == str1[i+j]; j--);
		if (j < 0) return i;
		i = i + j - min(__bad_char(str2, str1[i+j], j), __good_suffix(str2, j));
	}
	return -1;
}


///////////////////////////////////////////////////////////////////////////////
// Implementation
fTagString* fString::CreateTagString(int nLen)
{
	// workaround for CODESONAR
	int nSize = sizeof(fTagString) + (nLen+1)*sizeof(TCHAR);
	if ((nSize - sizeof(fTagString))/(nLen+1) != sizeof(TCHAR)) return NULL;

	fTagString *pTagString = (fTagString*)new char[nSize];
	pTagString->nLenData = nLen;
	pTagString->nLenAlloc = nLen;
	m_pData = pTagString->GetData();
	m_pData[nLen] = '\0';				// null-terminator
	return pTagString;
}


fTagString* fString::GetTagString() const
{
	return (fTagString*)((char*)m_pData - sizeof(fTagString));
}

bool fString::Alloc(int nLen)
{
	fTagString *pTagString = GetTagString();

	int nLenAllocNew;
		 if (nLen == 0)		nLenAllocNew = 0;
	else if (nLen < 64)		nLenAllocNew = 64;
	else if (nLen < 128)	nLenAllocNew = 128;
	else if (nLen < 256)	nLenAllocNew = 256;
	else if (nLen < 512)	nLenAllocNew = 512;
	else					nLenAllocNew = nLen+1;

	if (nLenAllocNew > pTagString->nLenAlloc)
	{
		delete []((char*)pTagString);
		pTagString = CreateTagString(nLenAllocNew);
	}

	pTagString->nLenData = nLen;
	m_pData[nLen] = '\0';
	return true;
}

void fString::CopyFrom(const fString& strSrc)
{
	if (!Alloc(strSrc.GetLength())) return;
	memcpy(m_pData, strSrc.m_pData, strSrc.GetLength() * sizeof(TCHAR));
}

void fString::CopyFrom(TCHAR ch, int nRepeat)
{
	if (nRepeat <= 0) return;
	if (!Alloc(nRepeat)) return;

#ifdef _UNICODE
	for (int i=0; i<nRepeat; i++) m_pData[i] = ch;
#else
	memset(m_pData, ch, nRepeat);
#endif
}

void fString::CopyFrom(const char* strSrc)
{
	int nLen;

#ifdef _UNICODE
	_wsetlocale(LC_ALL, L"");				// this is essential to handle Korean character
	nLen = (strSrc == NULL) ? 0 : (int)mbstowcs(NULL, strSrc, 0);	// get wide-character length
	if (!Alloc(nLen)) return;
	if (nLen > 0) mbstowcs(m_pData, strSrc, nLen+1);
#else
	nLen = (strSrc == NULL) ? 0 : (int)strlen(strSrc);
	if (!Alloc(nLen)) return;
	if (nLen > 0) memcpy(m_pData, strSrc, nLen);
#endif
}

void fString::CopyFrom(const wchar_t* strSrc)
{
	int nLen;

#ifdef _UNICODE
	nLen = (strSrc == NULL) ? 0 : (int)wcslen(strSrc);
	if (!Alloc(nLen)) return;
	if (nLen > 0) memcpy(m_pData, strSrc, nLen*sizeof(TCHAR));
#else
	setlocale(LC_ALL, "");				// this is essential to handle Korean character	
	nLen = (strSrc == NULL) ? 0 : (int)wcstombs(NULL, strSrc, 0);	// get multibyte length
	if (!Alloc(nLen)) return;
	if (nLen > 0) wcstombs(m_pData, strSrc, nLen+1);
#endif
}

void fString::CopyFrom(const unsigned char* strSrc)
{
	int nLen = (strSrc == NULL) ? 0 : (int)strlen((const char*)strSrc);
	if (!Alloc(nLen)) return;

#ifdef _UNICODE
	_wsetlocale(LC_ALL, L"");	
	mbstowcs(m_pData, (const char*)strSrc, nLen+1);
#else
	memcpy(m_pData, (const char*)strSrc, nLen);
#endif
}

fString::fString()
{
	CreateTagString(0);
}

fString::fString(const fString& strSrc)
{
	CreateTagString(0);
	CopyFrom(strSrc);
}

fString::fString(TCHAR ch, int nRepeat)
{
	CreateTagString(0);
	CopyFrom(ch, nRepeat);
}

fString::fString(const char* strSrc)
{
	CreateTagString(0);
	CopyFrom(strSrc);
}

fString::fString(const wchar_t* strSrc)
{
	CreateTagString(0);
	CopyFrom(strSrc);
}

fString::fString(const unsigned char* strSrc)
{
	CreateTagString(0);
	CopyFrom(strSrc);
}

fString::~fString()
{
	fTagString *pTagString = GetTagString();
	if (pTagString != NULL) delete []((char*)pTagString);
}

int fString::GetLength() const
{
	fTagString *pTagString = GetTagString();
	return pTagString->nLenData;
}

int fString::GetLengthAlloc() const
{
	fTagString *pTagString = GetTagString();
	return pTagString->nLenAlloc;
}

int fString::SetLength(int nLenNew)
{
	if (nLenNew > GetLengthAlloc())		// expand
	{
		fString strNew;
		if (strNew.Alloc(nLenNew)) CopyFrom((const fString&)strNew);
	}
	else							// shrink
	{
		fTagString *pTagString = GetTagString();
		pTagString->nLenData = nLenNew;		// just change nLenData 
		m_pData[nLenNew] = '\0';			// set null terminater
	}
	return GetLength();
}

bool fString::IsEmpty() const
{
	return (m_pData[0] == '\0');
}

void fString::Empty()
{
	SetLength(0);	// just zeroize string length and set null terminator
}

LPTSTR fString::GetBuffer(int nMinBufLength)
{
	if (nMinBufLength <= 0) return m_pData;

	// alloc if current allocation size is smaller than requested size
	if (nMinBufLength > GetLengthAlloc()) Alloc(nMinBufLength);

	// set data size
	fTagString *pTagString = GetTagString();
	pTagString->nLenData = nMinBufLength;

	return m_pData;
}

LPTSTR fString::GetBuffer()
{
	return m_pData;
}

LPTSTR fString::GetBuffer() const
{
	return m_pData;
}

fString& fString::operator=(const fString& strSrc)
{
	CopyFrom(strSrc);
	return *this;
}

fString& fString::operator=(TCHAR ch)
{
	CopyFrom(ch);
	return *this;
}

fString& fString::operator=(const char* strSrc)
{
	CopyFrom(strSrc);
	return *this;
}

fString& fString::operator=(const wchar_t* strSrc)
{
	CopyFrom(strSrc);
	return *this;
}

fString& fString::operator=(const unsigned char* strSrc)
{
	CopyFrom(strSrc);
	return *this;
}

fString& fString::operator+=(const fString& str)
{
	// calc new size
	int nLenDataNew = GetLength() + str.GetLength();

	if (GetLengthAlloc() > nLenDataNew)			// if my buffer is big enough to contain me and str
	{
		memcpy(m_pData + GetLength(), str.m_pData, str.GetLength()*sizeof(TCHAR));	// copy after me
		SetLength(nLenDataNew);		// update length
	}
	else
	{
		fString strNew;
		if (strNew.Alloc(nLenDataNew))
		{
			memcpy(strNew.m_pData, m_pData, GetLength()*sizeof(TCHAR));							// copy my data
			memcpy(strNew.m_pData + GetLength(), str.m_pData, str.GetLength()*sizeof(TCHAR));	// copy str data
			CopyFrom((const fString&)strNew);
		}
	}
	return *this;
}

fString& fString::operator+=(TCHAR ch)
{
	*this += fString(ch);
	return *this;
}

fString& fString::operator+=(LPCTSTR str)
{
	*this += fString(str);
	return *this;
}

fString operator+(const fString& str1, const fString& str2)
{
	fString strOut;
	strOut.Alloc(str1.GetLength() + str2.GetLength());
	strOut.CopyFrom(str1);
	strOut += str2;
	return strOut;
}

fString operator+(const fString& str, TCHAR ch)
{
	fString strOut;
	strOut.Alloc(str.GetLength() + 1);
	strOut.CopyFrom(str);
	strOut += ch;
	return strOut;
}

fString operator+(TCHAR ch, const fString& str)
{
	fString strOut;
	strOut.Alloc(1 + str.GetLength());
	strOut.CopyFrom(ch);
	strOut += str;
	return strOut;
}

fString operator+(const fString& str1, LPCTSTR str2)
{
	fString strOut;
	strOut.Alloc(str1.GetLength() + (int)__StrLen(str2));
	strOut.CopyFrom(str1);
	strOut += str2;
	return strOut;
}

fString operator+(LPCTSTR str1, const fString& str2)
{
	fString strOut;
	strOut.Alloc((int)__StrLen(str1) + str2.GetLength());
	strOut.CopyFrom(str1);
	strOut += str2;
	return strOut;
}

int fString::Compare(LPCTSTR str) const
{
#ifdef _UNICODE
	_wsetlocale(LC_ALL, L"");
	return wcscmp(m_pData, str);
#else
	setlocale(LC_ALL, "");
	return strcmp(m_pData, str);
#endif
}

int fString::CompareNoCase(LPCTSTR str) const
{
#ifdef _UNICODE
	_wsetlocale(LC_ALL, L"");
	return _wcsicmp(m_pData, str);
#else
	setlocale(LC_ALL, "");
	return _stricmp(m_pData, str);
#endif
}

int fString::Collate(LPCTSTR str) const
{
#ifdef _UNICODE
	_wsetlocale(LC_ALL, L"");
	return wcscoll(m_pData, str);
#else
	setlocale(LC_ALL, "");
	return strcoll(m_pData, str);
#endif
}

int fString::CollateNoCase(LPCTSTR str) const
{
#ifdef _UNICODE
	_wsetlocale(LC_ALL, L"");
	return _wcsicoll(m_pData, str);
#else
	setlocale(LC_ALL, "");
	return _stricoll(m_pData, str);
#endif
}

fString fString::Mid(int nFirst) const
{
	return Mid(nFirst, GetLength() - nFirst);
}

fString fString::Mid(int nFirst, int nCount) const
{
	int nLenData = GetLength();
	if (nFirst == 0 && nCount == nLenData) return *this;

	if (nFirst < 0) nFirst = 0;
	if (nCount < 0) nCount = 0;
	if (nFirst > (nLenData-1)) nFirst = nLenData-1;
	if ((nFirst + nCount) > nLenData) nCount = nLenData - nFirst;

	KASSERT(nFirst >= 0);
	KASSERT((nFirst + nCount) <= nLenData);

	fString strOut;
	strOut.Alloc(nCount);
	memcpy(strOut.m_pData, m_pData + nFirst, nCount*sizeof(TCHAR));
	return strOut;
}

fString fString::Left(int nCount) const
{
	if (nCount < 0) nCount = 0;
	if (nCount >= GetLength()) return *this;

	fString strOut;
	strOut.Alloc(nCount);
	memcpy(strOut.m_pData, m_pData, nCount*sizeof(TCHAR));
	return strOut;
}

fString fString::Right(int nCount) const
{
	if (nCount < 0) nCount = 0;
	if (nCount >= GetLength()) return *this;

	fString strOut;
	strOut.Alloc(nCount);
	memcpy(strOut.m_pData, m_pData + (GetLength()-nCount), nCount*sizeof(TCHAR));
	return strOut;
}

void fString::MakeUpper()
{
#ifdef _UNICODE
	_wsetlocale(LC_ALL, L"");
	_wcsupr(m_pData);
#else
	setlocale(LC_ALL, "");
	_strupr(m_pData);
#endif
}

void fString::MakeLower()
{
#ifdef _UNICODE
	_wsetlocale(LC_ALL, L"");
	_wcslwr(m_pData);
#else
	setlocale(LC_ALL, "");
	_strlwr(m_pData);
#endif
}

void fString::MakeReverse()
{
#ifdef _UNICODE
	_wsetlocale(LC_ALL, L"");
	_wcsrev(m_pData);
#else
	setlocale(LC_ALL, "");
//	_strrev(m_pData);			-- looks don't work for korean
	fString strTemp;
	int nLenData = GetLength();
	if (strTemp.Alloc(nLenData))
	{
		int i=0;
		while (i<nLenData)
		{
			if (m_pData[i] & 0x80)
			{
				strTemp.m_pData[nLenData-1-i-1] = m_pData[i];
				strTemp.m_pData[nLenData-1-i] = m_pData[i+1];
				i+=2;
			}
			else
			{
				strTemp.m_pData[nLenData-1-i] = m_pData[i];
				i++;
			}
		}
		memcpy(m_pData, strTemp.m_pData, nLenData);
	}
#endif
}

void fString::TrimLeft()
{
	int nLenData = GetLength();
	if (nLenData == 0) return;

	// find first non-space character
	int i = 0;
	while (i < nLenData)
	{
#ifdef _UNICODE
		if (m_pData[i] != L' ') break;
#else
		if (m_pData[i] != ' ') break;
#endif
		i++;
	}
	if (i == 0) return;

	nLenData = nLenData-i;		// new string length
	if (nLenData > 0) memmove(m_pData, m_pData + i, nLenData*sizeof(TCHAR));
	SetLength(nLenData);		// update length
}

void fString::TrimRight()
{
	int nLenData = GetLength();
	if (nLenData == 0) return;

	// find first non-space character from right
	int i = nLenData-1;
	while (i >= 0)
	{
#ifdef _UNICODE
		if (m_pData[i] != L' ') break;
#else
		if (m_pData[i] != ' ') break;
#endif
		i--;
	}
	if (i == nLenData-1) return;

	nLenData = i+1;			// new string length
	SetLength(nLenData);	// update length
}

void fString::TrimLeft(TCHAR ch)
{
	int nLenData = GetLength();
	if (nLenData == 0) return;

	// find first non-ch character
	int i = 0;
	while (i < nLenData)
	{
		if (m_pData[i] != ch) break;
		i++;
	}
	if (i == 0) return;

	nLenData = nLenData-i;		// new string length
	if (nLenData > 0) memmove(m_pData, m_pData + i, nLenData*sizeof(TCHAR));
	SetLength(nLenData);		// update length
}

void fString::TrimRight(TCHAR ch)
{
	int nLenData = GetLength();
	if (nLenData == 0) return;

	// find first non-ch character from right
	int i = nLenData-1;
	while(i >= 0)
	{
		if (m_pData[i] != ch) break;
		i--;
	}
	if (i == nLenData-1) return;

	nLenData = i+1;			// new string length
	SetLength(nLenData);	// update length
}

void fString::TrimLeft(LPCTSTR str)
{
	int nLenData = GetLength();
	if (nLenData == 0) return;

	int nLenStr = (int)__StrLen(str);
	if (nLenStr == 0) return;

	// find matching position from left
	int i = 0;
	while (i < nLenData)
	{
		if (i >= nLenStr) break;
		if (m_pData[i] != str[i]) break;
		i++;
	}
	if (i == 0) return;

	nLenData = nLenData-i;		// new string length
	if (nLenData > 0) memmove(m_pData, m_pData + i, nLenData*sizeof(TCHAR));
	SetLength(nLenData);		// update length
}

void fString::TrimRight(LPCTSTR str)
{
	int nLenData = GetLength();
	if (nLenData == 0) return;

	int nLenStr = (int)__StrLen(str);
	if (nLenStr == 0) return;

	// find matching position from right
	int i = nLenData-1;
	int j = nLenStr-1;
	while(i >= 0)
	{
		if (j < 0) break;
		if (m_pData[i] != str[j]) break;
		i--;
		j--;
	}
	if (i == nLenData-1) return;

	nLenData = i+1;			// new string length
	SetLength(nLenData);	// update length
}

int fString::Replace(TCHAR chOld, TCHAR chNew)
{
	int nCount = 0;
	int nLenData = GetLength();
	if (nLenData == 0) return nCount;

	for (int i=0; i<nLenData; i++)
	{
		if (m_pData[i] == chOld) { m_pData[i] = chNew; nCount++; }
	}
	return nCount;
}

int fString::Replace(LPCTSTR strOld, LPCTSTR strNew)
{
	int nCount = 0;
	int nLenData = GetLength();
	if (nLenData == 0) return nCount;

	int nLenOld = (int)__StrLen(strOld);
	int nLenNew = (int)__StrLen(strNew);
	if (nLenOld == 0) return nCount;

	fString strOut, strSub;
	int nPos, nStart, nLenSub;
	nStart = 0;
	do
	{
		nPos = Find(strOld, nStart);
		if (nPos != -1)
		{
			nLenSub = nPos - nStart;
			if (!strSub.Alloc(nLenSub)) return nCount;
			memcpy(strSub.m_pData, m_pData + nStart, nLenSub*sizeof(TCHAR));

			// add sub region
			strOut += strSub;

			// add new string
			strOut += strNew;

			// new start point of searching
			nStart = nPos + nLenOld;

			nCount++;
		}
		else
		{
			// add last sub region
			nLenSub = nLenData - nStart;
			if (nLenSub > 0)
			{
				if (!strSub.Alloc(nLenSub)) return nCount;
				memcpy(strSub.m_pData, m_pData + nStart, nLenSub*sizeof(TCHAR));
				strOut += strSub;
			}
		}
	}
	while (nPos != -1);
	if (nCount > 0) CopyFrom(strOut);

	return nCount;
}

int fString::Remove(TCHAR ch)
{
	int nCount = 0;
	int nLenData = GetLength();
	if (nLenData == 0) return nCount;

	fString strOut, strSub;
	int nPos, nStart, nLenSub;
	nStart = 0;
	do
	{
		nPos = Find(ch, nStart);
		if (nPos != -1)
		{
			nLenSub = nPos - nStart;
			if (!strSub.Alloc(nLenSub)) return nCount;
			memcpy(strSub.m_pData, m_pData + nStart, nLenSub*sizeof(TCHAR));

			// add sub region
			strOut += strSub;

			// new start point of searching
			nStart = nPos + 1;

			nCount++;
		}
		else
		{
			// add last sub region
			nLenSub = nLenData - nStart;
			if (nLenSub > 0)
			{
				if (!strSub.Alloc(nLenSub)) return nCount;
				memcpy(strSub.m_pData, m_pData + nStart, nLenSub*sizeof(TCHAR));
				strOut += strSub;
			}
		}
	}
	while (nPos != -1);
	if (nCount > 0) CopyFrom(strOut);

	return nCount;
}

int fString::Insert(int nIndex, TCHAR ch)
{
	int nLenData = GetLength();
	if (nIndex < 0) nIndex = 0;
	if (nIndex >= nLenData) nIndex = nLenData;
	int nLenDataNew = nLenData + 1;

	if (GetLengthAlloc() > nLenDataNew)		// if have room for 1 char to be inserted
	{
		memmove(m_pData + nIndex + 1, m_pData + nIndex, (nLenData-nIndex)*sizeof(TCHAR));
		m_pData[nIndex] = ch;
		SetLength(nLenDataNew);		// update length
	}
	else
	{
		fString strOut;
		if (strOut.Alloc(nLenDataNew))
		{
			if (nIndex > 0) memcpy(strOut.m_pData, m_pData, nIndex*sizeof(TCHAR));
			strOut.m_pData[nIndex] = ch;
			if (nIndex < nLenData) memcpy(strOut.m_pData + nIndex + 1, m_pData + nIndex, (nLenData-nIndex)*sizeof(TCHAR));
			CopyFrom(strOut);
		}
	}

	return GetLength();
}

int fString::Insert(int nIndex, LPCTSTR str)
{
	int nLenData = GetLength();
	if (nIndex < 0) nIndex = 0;
	if (nIndex >= nLenData) nIndex = nLenData;
	
	int nLenStr, nLenDataNew;
	nLenStr = (int)__StrLen(str);
	if (nLenStr == 0) return GetLength();

	// output string length
	nLenDataNew = nLenData + nLenStr;

	if (GetLengthAlloc() > nLenDataNew)		// if have room for str to be inserted
	{
		memmove(m_pData + nIndex + nLenStr, m_pData + nIndex, (nLenData-nIndex)*sizeof(TCHAR));
		memcpy(m_pData + nIndex, str, nLenStr*sizeof(TCHAR));
		SetLength(nLenDataNew);		// update length
	}
	else
	{
		fString strOut;
		if (strOut.Alloc(nLenDataNew)) 
		{
			if (nIndex > 0) memcpy(strOut.m_pData, m_pData, nIndex*sizeof(TCHAR));
			memcpy(strOut.m_pData + nIndex, str, nLenStr*sizeof(TCHAR));
			if (nIndex < nLenData) memcpy(strOut.m_pData + nIndex + nLenStr, m_pData + nIndex, (nLenData-nIndex)*sizeof(TCHAR));
			CopyFrom(strOut);
		}
	}

	return GetLength();
}

int fString::Delete(int nIndex, int nCount)
{
	int nLenData = GetLength();
	if (nIndex < 0) nIndex = 0;
	if (nIndex >= nLenData) nIndex = nLenData;
	if ((nIndex + nCount) > nLenData) nCount = nLenData - nIndex;

	int nMove = nLenData - (nIndex + nCount);
	if (nMove > 0) memmove(m_pData + nIndex, m_pData + nIndex + nCount, nMove*sizeof(TCHAR));

	SetLength(nLenData - nCount); // update size
	return GetLength();
}

int fString::Find(TCHAR ch, int nStart) const
{
	int nLenData = GetLength();	
	if (nLenData == 0) return -1;
	if (nStart < 0 || nStart >= nLenData) return -1;
	for (int i=nStart; i<nLenData; i++)
	{
		if (m_pData[i] == ch) return i;
	}
	return -1;
}

int fString::Find(LPCTSTR str, int nStart) const
{
	int nLenData = GetLength();	
	if (nLenData == 0) return -1;
	if (nStart < 0 || nStart >= nLenData) return -1;

	int nLenStr = (int)__StrLen(str);
	if (nLenStr == 0 || nLenStr > nLenData) return -1;

	return __StrSearch(m_pData + nStart, str);
}

int fString::FindOneOf(LPCTSTR str) const
{
	int nLenData = GetLength();	
	if (nLenData == 0) return -1;

	int nLenStr = (int)__StrLen(str);
	if (nLenStr == 0 || nLenStr > nLenData) return -1;

	int i, j;
	for (i=0; i<nLenData; i++)
	{
		for (j=0; j<nLenStr; j++)
		{
			if (m_pData[i] == str[j]) return i;
		}
	}
	return -1;
}

int fString::ReverseFind(TCHAR ch) const
{
	int nLenData = GetLength();	
	if (nLenData == 0) return -1;
	for (int i=nLenData-1; i>=0; i--)
	{
		if (m_pData[i] == ch) return i;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
// Formatting

#define TCHAR_ARG   TCHAR
#define WCHAR_ARG   wchar_t
#define CHAR_ARG    char

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000

void fString::FormatV(LPCTSTR lpszFormat, va_list argList)
{
	va_list argListSave = argList;

	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	for (LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = __StrInc(lpsz))
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(lpsz = __StrInc(lpsz)) == '%')
		{
			nMaxLen += (int)__StrLen(lpsz);
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz = __StrInc(lpsz))
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = _crt_va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
				*lpsz == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = __StrToInt(lpsz);
			for (; *lpsz != '\0' && __IsDigit(*lpsz); lpsz = __StrInc(lpsz))
				;
		}
		KASSERT(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz = __StrInc(lpsz);

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = _crt_va_arg(argList, int);
				lpsz = __StrInc(lpsz);
			}
			else
			{
				nPrecision = __StrToInt(lpsz);
				for (; *lpsz != '\0' && __IsDigit(*lpsz); lpsz = __StrInc(lpsz))
					;
			}
			KASSERT(nPrecision >= 0);
		}

		// should be on type modifier or specifier
		int nModifier = 0;
#ifdef _UNICODE
		if (__StrnCmp(lpsz, L"I64", 3) == 0)
#else
		if (__StrnCmp(lpsz, "I64", 3) == 0)
#endif
		{
			lpsz += 3;
			nModifier = FORCE_INT64;
		}
		else
		{
			switch (*lpsz)
			{
			// modifiers that affect size
			case 'h':
				nModifier = FORCE_ANSI;
				lpsz = __StrInc(lpsz);
				break;
			case 'l':
				nModifier = FORCE_UNICODE;
				lpsz = __StrInc(lpsz);
				break;

			// modifiers that do not affect size
			case 'F':
			case 'N':
			case 'L':
				lpsz = __StrInc(lpsz);
				break;
			}
		}

		// now should be on specifier
		switch (*lpsz | nModifier)
		{
		// single characters
		case 'c':
		case 'C':
			nItemLen = 2;
			_crt_va_arg(argList, TCHAR_ARG);
			break;
		case 'c'|FORCE_ANSI:
		case 'C'|FORCE_ANSI:
			nItemLen = 2;
			_crt_va_arg(argList, CHAR_ARG);
			break;
		case 'c'|FORCE_UNICODE:
		case 'C'|FORCE_UNICODE:
			nItemLen = 2;
			_crt_va_arg(argList, WCHAR_ARG);
			break;

		// strings
		case 's':
			{
				LPCTSTR pstrNextArg = _crt_va_arg(argList, LPCTSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6;  // "(null)"
				else
				{
				   nItemLen = (int)strlen((const char*)pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
			}
			break;

		case 'S':
			{
				LPTSTR pstrNextArg = _crt_va_arg(argList, LPTSTR);
				if (pstrNextArg == NULL)
				   nItemLen = 6;  // "(null)"
				else
				{
				   nItemLen = (int)__StrLen(pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
			}
			break;

		case 's'|FORCE_ANSI:
		case 'S'|FORCE_ANSI:
			{
				const char* pstrNextArg = _crt_va_arg(argList, const char*);
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"
				else
				{
				   nItemLen = (int)strlen(pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
			}
			break;

		case 's'|FORCE_UNICODE:
		case 'S'|FORCE_UNICODE:
			{
				wchar_t* pstrNextArg = _crt_va_arg(argList, wchar_t*);
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"
				else
				{
				   nItemLen = (int)wcslen(pstrNextArg);
				   nItemLen = max(1, nItemLen);
				}
			}
			break;
		}

		// adjust nItemLen for strings
		if (nItemLen != 0)
		{
			if (nPrecision != 0) nItemLen = min(nItemLen, nPrecision);
			nItemLen = max(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
			// integers
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				if (nModifier & FORCE_INT64)
					_crt_va_arg(argList, __int64);
				else
					_crt_va_arg(argList, int);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			case 'e':
			case 'g':
			case 'G':
				_crt_va_arg(argList, double);
				nItemLen = 128;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			case 'f':
				_crt_va_arg(argList, double);
				nItemLen = 128; // width isn't truncated
				// 312 == strlen("-1+(309 zeroes).")
				// 309 zeroes == max precision of a double
				nItemLen = max(nItemLen, 312+nPrecision);
				break;

			case 'p':
				_crt_va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth+nPrecision);
				break;

			// no output
			case 'n':
				_crt_va_arg(argList, int*);
				break;

			default:
				KASSERT(false);  // unknown formatting option
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	if (Alloc(nMaxLen))
	{
#ifdef _UNICODE
		vswprintf(m_pData, lpszFormat, argListSave);
#else
		vsprintf(m_pData, lpszFormat, argListSave);
#endif
	}

	// set exact size of string
	SetLength(__StrLen(m_pData));

	_crt_va_end(argListSave);
}

// formatting (using wsprintf style formatting)
void __cdecl fString::Format(LPCTSTR lpszFormat, ...)
{
	va_list argList;
	_crt_va_start(argList, lpszFormat);
	FormatV(lpszFormat, argList);
	_crt_va_end(argList);
}

bool operator==(const fString& s1, const fString& s2)	{ return s1.Compare(s2) == 0; };
bool operator==(const fString& s1, LPCTSTR s2)			{ return s1.Compare(s2) == 0; };
bool operator==(const fString& s1, LPTSTR s2)			{ return s1.Compare(s2) == 0; };
bool operator==(LPCTSTR s1, const fString& s2)			{ return s2.Compare(s1) == 0; };
bool operator==(LPTSTR s1, const fString& s2)			{ return s2.Compare(s1) == 0; };

bool operator!=(const fString& s1, const fString& s2)	{ return s1.Compare(s2) != 0; };
bool operator!=(const fString& s1, LPCTSTR s2)			{ return s1.Compare(s2) != 0; };
bool operator!=(const fString& s1, LPTSTR s2)			{ return s1.Compare(s2) != 0; };
bool operator!=(LPCTSTR s1, const fString& s2)			{ return s2.Compare(s1) != 0; };
bool operator!=(LPTSTR s1, const fString& s2)			{ return s2.Compare(s1) != 0; };

bool operator<(const fString& s1, const fString& s2)	{ return s1.Compare(s2) < 0; };
bool operator<(const fString& s1, LPCTSTR s2)			{ return s1.Compare(s2) < 0; };
bool operator<(const fString& s1, LPTSTR s2)			{ return s1.Compare(s2) < 0; };
bool operator<(LPCTSTR s1, const fString& s2)			{ return s2.Compare(s1) > 0; };
bool operator<(LPTSTR s1, const fString& s2)			{ return s2.Compare(s1) > 0; };

bool operator>(const fString& s1, const fString& s2)	{ return s1.Compare(s2) > 0; };
bool operator>(const fString& s1, LPCTSTR s2)			{ return s1.Compare(s2) > 0; };
bool operator>(const fString& s1, LPTSTR s2)			{ return s1.Compare(s2) > 0; };
bool operator>(LPCTSTR s1, const fString& s2)			{ return s2.Compare(s1) < 0; };
bool operator>(LPTSTR s1, const fString& s2)			{ return s2.Compare(s1) < 0; };

bool operator<=(const fString& s1, const fString& s2)	{ return s1.Compare(s2) <= 0; };
bool operator<=(const fString& s1, LPCTSTR s2)			{ return s1.Compare(s2) <= 0; };
bool operator<=(const fString& s1, LPTSTR s2)			{ return s1.Compare(s2) <= 0; };
bool operator<=(LPCTSTR s1, const fString& s2)			{ return s2.Compare(s1) >= 0; };
bool operator<=(LPTSTR s1, const fString& s2)			{ return s2.Compare(s1) >= 0; };

bool operator>=(const fString& s1, const fString& s2)	{ return s1.Compare(s2) >= 0; };
bool operator>=(const fString& s1, LPCTSTR s2)			{ return s1.Compare(s2) >= 0; };
bool operator>=(const fString& s1, LPTSTR s2)			{ return s1.Compare(s2) >= 0; };
bool operator>=(LPCTSTR s1, const fString& s2)			{ return s2.Compare(s1) <= 0; };
bool operator>=(LPTSTR s1, const fString& s2)			{ return s2.Compare(s1) <= 0; };

}	// end of namespace FCORE
}	// end of namespace FGBC