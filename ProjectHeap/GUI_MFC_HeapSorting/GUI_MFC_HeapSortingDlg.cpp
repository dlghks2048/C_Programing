
// GUI_MFC_HeapSortingDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "GUI_MFC_HeapSorting.h"
#include "GUI_MFC_HeapSortingDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGUIMFCHeapSortingDlg 대화 상자



CGUIMFCHeapSortingDlg::CGUIMFCHeapSortingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GUI_MFC_HEAPSORTING_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGUIMFCHeapSortingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGUIMFCHeapSortingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CGUIMFCHeapSortingDlg 메시지 처리기

BOOL CGUIMFCHeapSortingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CGUIMFCHeapSortingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CGUIMFCHeapSortingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CGUIMFCHeapSortingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 1. 캐릭터 그리기 함수
void CGUIMFCHeapSortingDlg::DrawCharacter(CDC* pDC, int state, int frame)
{
	// m_pCharSprite는 헤더파일에 선언되어 있어야 합니다.
	if (m_pCharSprite == NULL) {
		COLORREF color = RGB(0, 255, 0); // IDLE
		if (state == 2) color = RGB(255, 0, 0); // ATTACK 
		if (state == 3) color = RGB(255, 255, 0); // PARRY

		pDC->FillSolidRect(50, 50, 100, 64, color);
		return;
	}

	Gdiplus::Graphics graphics(pDC->GetSafeHdc());
	int srcX = frame * 100;
	int srcY = state * 64;

	graphics.DrawImage(m_pCharSprite,
		Gdiplus::Rect(50, 50, 100, 64),
		srcX, srcY, 100, 64,
		Gdiplus::UnitPixel);
}

// 2. 핑 그래프 그리기 함수
void CGUIMFCHeapSortingDlg::DrawPingGraph(CDC* pDC)
{
	CRect rect;
	// 다이얼로그에 배치한 Picture Control의 ID를 사용합니다.
	GetDlgItem(IDC_PING_GRAPH)->GetClientRect(&rect);

	pDC->FillSolidRect(&rect, RGB(30, 30, 30)); // 배경색 (어두운 회색)

	CPen pen(PS_SOLID, 2, RGB(0, 255, 255)); // 하늘색 선
	CPen* pOldPen = pDC->SelectObject(&pen);

	// m_historyCount와 m_latencyHistory는 헤더에 선언되어야 함
	for (int i = 1; i < m_historyCount; i++) {
		int x1 = (i - 1) * 5;
		int x2 = i * 5;

		// 그래프가 Picture Control 영역을 벗어나지 않도록 계산
		int y1 = rect.Height() - (m_latencyHistory[i - 1] % rect.Height());
		int y2 = rect.Height() - (m_latencyHistory[i] % rect.Height());

		pDC->MoveTo(x1, y1);
		pDC->LineTo(x2, y2);
	}
	pDC->SelectObject(pOldPen);
}