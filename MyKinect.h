
// MyKinect.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMyKinectApp:
// �йش����ʵ�֣������ MyKinect.cpp
//

class CMyKinectApp : public CWinApp
{
public:
	CMyKinectApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMyKinectApp theApp;