
// pocket.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CpocketApp:
// See pocket.cpp for the implementation of this class
//

class CpocketApp : public CWinApp
{
public:
	CpocketApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CpocketApp theApp;