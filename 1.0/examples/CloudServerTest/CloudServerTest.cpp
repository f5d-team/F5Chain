///////////////////////////////////////////////////////////////////////////////
// Developed by Curiosity laboratory 2018

#include "stdafx.h"
#include <afx.h>
#include <afxdlgs.h>
#include <Windows.h>
#include <assert.h>
#include "fcore.h"
#include <conio.h>
#include <iostream>

using namespace F5Chain;
using namespace F5Chain::FCORE;
using namespace std;

class fServerTest:public fService
{
public:
	fServerTest(){};
	virtual ~fServerTest(){};
};

fServerTest* pClient = NULL;

BOOL WINAPI ConsoleHandler(DWORD msgType)
{
	if (msgType == CTRL_CLOSE_EVENT)
	{

		printf("Close console window!\n");
		return TRUE;
	} 
	return FALSE;
}


int _tmain(int argc, _TCHAR* argv[])
{
	SetConsoleCtrlHandler(ConsoleHandler,TRUE);
	AfxSetResourceHandle(GetModuleHandle(NULL));


	return 0;
}

