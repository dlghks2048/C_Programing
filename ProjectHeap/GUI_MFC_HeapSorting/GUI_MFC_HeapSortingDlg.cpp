
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
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CGUIMFCHeapSortingDlg 메시지 처리기

BOOL CGUIMFCHeapSortingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 캐릭터 이미지 로드
	CString fileNames[] = {
		_T("res\\Character\\Idle_KG.png"),   // IDLE (0)
		_T("res\\Character\\Move_KG.png"),   // MOVE (1)
		_T("res\\Character\\Attack_KG.png"), // ATTACK (2)
		_T("res\\Character\\Hit_KG.png"),    // HIT (3)
		_T("res\\Character\\Guard_KG.png"),  // GUARD (4)
		_T("res\\Character\\Parry_KG.png")   // PARRY (5)
	};

	for (int i = 0; i < 6; i++) {
		m_pSprites[i] = Gdiplus::Image::FromFile(fileNames[i]);
		if (m_pSprites[i]->GetLastStatus() == Gdiplus::Ok) {
			// 가로 길이를 100(한 프레임 너비)으로 나눠서 최대 프레임 계산
			m_maxFrames[i] = m_pSprites[i]->GetWidth() / 100;
		}

		// 이미지 로드 실패 시 디버그 출력 (출력창에서 확인 가능)
		if (m_pSprites[i]->GetLastStatus() != Gdiplus::Ok) {
			TRACE(_T("FAILED to load: %s\n"), fileNames[i]);
		}
		else {
			TRACE(_T("Sucsece to load: %s\n"), fileNames[i]);
		}
	}

	SetTimer(1, 100, NULL);

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
		CPaintDC dc(this);
		CWnd* pImgView = GetDlgItem(IDC_SIM_VIEW);
		if (!pImgView) return;

		CRect rc;
		pImgView->GetClientRect(&rc);

		// 1. 메모리 DC와 비트맵 생성 (임시 도화지)
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
		CBitmap* pOldBmp = memDC.SelectObject(&bmp);

		// 2. 메모리 DC 배경 청소
		memDC.FillSolidRect(&rc, RGB(255, 255, 255));

		// 3. 메모리 DC에 그리기
		DrawCharacter(&memDC, 0, 0, m_testState, m_curFrame);

		// 4. 완성된 메모리 도화지를 실제 화면(Picture Control)에 복사
		CDC* pControlDC = pImgView->GetDC();
		pControlDC->BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

		// 5. 정리
		pImgView->ReleaseDC(pControlDC);
		memDC.SelectObject(pOldBmp);
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CGUIMFCHeapSortingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 1. 캐릭터 그리기 함수
void CGUIMFCHeapSortingDlg::DrawCharacter(CDC* pDC, int x, int y, int state, int frame)
{
	// 예외 처리: 범위를 벗어나거나 이미지가 없는 경우
	if (state < 0 || state > 5 || m_pSprites[state] == NULL) return;

	Gdiplus::Graphics graphics(pDC->GetSafeHdc());

	// 각 파일이 1줄짜리 스프라이트 시트이므로 y는 항상 0
	int frameWidth = 100;
	int frameHeight = 64;
	int srcX = frame * frameWidth;
	int srcY = 0;

	graphics.DrawImage(m_pSprites[state],
		Gdiplus::Rect(x, y, frameWidth, frameHeight),
		srcX, srcY, frameWidth, frameHeight,
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

void CGUIMFCHeapSortingDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) {
		m_curFrame++;

		// 현재 상태(m_testState)의 최대 프레임 수를 사용
		if (m_curFrame >= m_maxFrames[m_testState]) {
			m_curFrame = 0;
			m_testState = (m_testState + 1) % 6;
		}
		Invalidate(FALSE);
	}
	CDialogEx::OnTimer(nIDEvent);
}
