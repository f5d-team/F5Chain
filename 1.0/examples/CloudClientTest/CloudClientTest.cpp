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

class fClientTest:public fService
{
public:
	fClientTest(){};
	virtual ~fClientTest(){};
};

fClientTest* pClient = NULL;

BOOL WINAPI ConsoleHandler(DWORD msgType)
{
	if (msgType == CTRL_CLOSE_EVENT)
	{
		SAFE_DELETE(pClient);
		printf("Close console window!\n");
		return TRUE;
	} 
	return FALSE;
}


int _tmain(int argc, _TCHAR* argv[])
{
	SetConsoleCtrlHandler(ConsoleHandler,TRUE);
	AfxSetResourceHandle(GetModuleHandle(NULL));

	cout<< "./ fCloudClientTest " << endl;
	fString strError = "";	

	pClient = new fClientTest();
	
	fCloud cloud;
	cloud.RegisterService(pClient);
	if(!cloud.RegisterService(pClient,&strError)){
		cout << strError << endl;
		goto end;
	}

	ip::tcp::endpoint AddrTarget = ip::tcp::endpoint(ip::address_v4::from_string("192.168.1.254"), 1000);
	if(!pClient->Connect(AddrTarget,true,&strError))
	{
		cout << strError << endl;
		SAFE_DELETE(pClient);
	}
end:
	cout<< endl<< "Press any key to exit" <<endl;
	int k = getch();
	//SAFE_DELETE(pClient);
	return 0;
}

