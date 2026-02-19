
// GUI_MFC_HeapSorting.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// CGUIMFCHeapSortingApp:
// 이 클래스의 구현에 대해서는 GUI_MFC_HeapSorting.cpp을(를) 참조하세요.
//

class CGUIMFCHeapSortingApp : public CWinApp
{
private:
	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;
public:
	CGUIMFCHeapSortingApp();

// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.

	DECLARE_MESSAGE_MAP()
};

extern CGUIMFCHeapSortingApp theApp;


