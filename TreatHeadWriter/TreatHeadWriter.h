
// TreatHeadWriter.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTreatHeadWriterApp: 
// �йش����ʵ�֣������ TreatHeadWriter.cpp
//

class CTreatHeadWriterApp : public CWinApp
{
public:
	CTreatHeadWriterApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTreatHeadWriterApp theApp;