
// GUI_MFC_HeapSortingDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "GUI_MFC_HeapSorting.h"
#include "GUI_MFC_HeapSortingDlg.h"
#include "afxdialogex.h"
#include "CNetwork.h"
#include "resource.h"

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
public:
//	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
//	ON_WM_LBUTTONDOWN()
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
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CGUIMFCHeapSortingDlg 메시지 처리기

BOOL CGUIMFCHeapSortingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//리스트 컨트롤러 초기화
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PACKET);
	pList->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES); // 줄 긋기
	pList->InsertColumn(0, _T("Seq"), LVCFMT_LEFT, 80);
	pList->InsertColumn(1, _T("Type"), LVCFMT_LEFT, 80);
	pList->InsertColumn(2, _T("Frame"), LVCFMT_LEFT, 70);
	pList->InsertColumn(3, _T("Ping"), LVCFMT_LEFT, 100);
	//리스트 컨트롤러 초기화



	// 네트워크 초기화 및 서버 접속
	if (m_net.InitAndConnect(SERVER_IP, SERVER_PORT)) {
		printf("서버 접속 시도 중... (IP: %s, Port: %d)\n", SERVER_IP, SERVER_PORT);
	}

	// 캐릭터 이미지 로드
	CString fileNames[] = {
		_T("res\\Character\\Idle_KG.png"),		// IDLE (0)
		_T("res\\Character\\Move_KG.png"),		// MOVE (1)
		_T("res\\Character\\Attack_KG.png"),	// ATTACK (2)
		_T("res\\Character\\Hit_KG.png"),		// HIT (3)
		_T("res\\Character\\Guard_KG.png"),		// GUARD (4)
		_T("res\\Character\\Parry_KG.png"),		// PARRY (5)
		_T("res\\Character\\Idle2_KG.png")		// IDLE2 (6) == GUARD_IDLE
	};

	for (int i = 0; i < MAX_STATE; i++) {
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
	// 애니메이션 타이머
	SetTimer(1, 70, NULL);
	// 네트워크 로직 전용 타이머 
	SetTimer(2, 30, NULL);

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

		// 메모리 DC 배경 청소(흰색 바탕)
		memDC.FillSolidRect(&rc, RGB(255, 255, 255));

		//캐릭터 위치 조정 변수
		int centerX = rc.Width() / 2;
		int groundY = rc.Height() - CHARACTOR_HEIGHT; // 바닥에서 50px 정도 띄움

		// 메모리 DC에 그리기(내 캐릭터)
		DrawCharacter(&memDC, centerX - 100, groundY, m_curState, m_curFrame, false);
		// 상대 캐릭터 그리기 (서버가 준 프레임 정보 사용)
		DrawCharacter(&memDC, centerX, groundY, m_enemyState, m_enemyFrame, true);

		// 4. 완성된 메모리 도화지를 실제 화면(Picture Control)에 복사
		CDC* pControlDC = pImgView->GetDC();
		pControlDC->BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
		

		DrawPingGraph(&dc);			//핑 크래프 출력

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
void CGUIMFCHeapSortingDlg::DrawCharacter(CDC* pDC, int x, int y, int state, int frame, bool isEnemy)
{
	// 예외 처리 
	if (state < 0 || state > MAX_STATE - 1 || m_pSprites[state] == NULL) return;

	Gdiplus::Graphics graphics(pDC->GetSafeHdc());

	// 스프라이트 시트 내 프레임 위치 계산
	int frameWidth = 100;
	int frameHeight = 64;
	int srcX = frame * frameWidth;
	int srcY = 0;

	Gdiplus::ImageAttributes imgAtt;
	if (isEnemy) {
		// 빨간색 필터 적용 (ColorMatrix)
		Gdiplus::ColorMatrix clrMatrix = {
			1.2f, 0.0f, 0.0f, 0.0f, 0.0f, // Red 강조
			0.0f, 0.3f, 0.0f, 0.0f, 0.0f, // Green 축소
			0.0f, 0.0f, 0.3f, 0.0f, 0.0f, // Blue 축소
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Alpha
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};
		imgAtt.SetColorMatrix(&clrMatrix);

		// 좌우 반전 (캐릭터 중앙을 기준으로 뒤집기)
		graphics.TranslateTransform((float)x + 50.0f, (float)y + 32.0f);
		graphics.ScaleTransform(-1.0f, 1.0f);
		graphics.TranslateTransform(-(float)x - 50.0f, -(float)y - 32.0f);
	}


	// 이미지 그리기
	graphics.DrawImage(m_pSprites[state],
		Gdiplus::Rect(x, y, frameWidth, frameHeight),
		srcX, srcY, frameWidth, frameHeight,
		Gdiplus::UnitPixel, &imgAtt); // imgAtt 추가

	// 변환 초기화(다음 그리기 덮어쓰기 방기)
	graphics.ResetTransform();

	// 텍스트 출력 로직 
	const TCHAR* stateNames[] = { _T("IDLE"), _T("MOVE"), _T("ATTACK"), _T("HIT"), _T("GUARD"), _T("PARRY"), _T("GUARDING") };

	CString strStateName;
	strStateName.Format(_T("[%s]"), stateNames[state]);
	CString strPingValue;
	strPingValue.Format(_T("%d ms"), m_currentPing);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(isEnemy ? RGB(255, 0, 0) : RGB(0, 0, 0)); // 적군은 빨간색 텍스트

	pDC->SetTextAlign(TA_CENTER);
	pDC->TextOut(x + 50, y - 40, strStateName);
	pDC->TextOut(x + 50, y - 22, strPingValue);
	pDC->SetTextAlign(TA_LEFT);
}

// 2. 핑 그래프 그리기 함수
void CGUIMFCHeapSortingDlg::DrawPingGraph(CDC* pDC)
{
	// 1. Picture Control의 위치를 대화 상자 좌표 기준으로 가져오기
	CWnd* pPingWnd = GetDlgItem(IDC_PING_GRAPH);
	if (!pPingWnd) return;

	CRect rect;
	pPingWnd->GetWindowRect(&rect);      // 화면 기준 좌표
	ScreenToClient(&rect);              // 대화 상자 기준 좌표로 변환

	// 2. 배경 그리기
	pDC->FillSolidRect(&rect, RGB(30, 30, 30));

	// 3. 그래프 그리기 준비
	CPen pen(PS_SOLID, 1, RGB(0, 255, 255)); // 하늘색 선
	CPen* pOldPen = pDC->SelectObject(&pen);

	// 데이터가 2개 이상 있어야 선을 긋습니다.
	if (m_historyCount < 2) return;

	// 그래프 가로 간격 계산 (컨트롤 너비에 맞춰 자동 조절)
	float xStep = (float)rect.Width() / 199.0f;

	// 핑 최대 기준치
	float maxHeight = 300.0f;

	for (int i = 1; i < m_historyCount; i++) {
		// X 좌표 계산
		int x1 = rect.left + (int)((i - 1) * xStep);
		int x2 = rect.left + (int)(i * xStep);

		// Y 좌표 계산 (핑 수치가 높을수록 위로 가도록 계산)
		// m_latencyHistory[i]가 0이면 바닥(rect.bottom), 200이면 천장(rect.top)
		float val1 = (float)m_latencyHistory[i - 1];
		float val2 = (float)m_latencyHistory[i];

		if (val1 > maxHeight) val1 = maxHeight;
		if (val2 > maxHeight) val2 = maxHeight;

		int y1 = rect.bottom - (int)((val1 / maxHeight) * rect.Height());
		int y2 = rect.bottom - (int)((val2 / maxHeight) * rect.Height());

		pDC->MoveTo(x1, y1);
		pDC->LineTo(x2, y2);
	}

	pDC->SelectObject(pOldPen);

	// 현재 핑 수치 텍스트 표시
	CString strPing;
	strPing.Format(_T("%d ms"), m_latencyHistory[m_historyCount - 1]);
	pDC->SetTextColor(RGB(0, 255, 255));
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOutW(rect.left + 5, rect.top + 5, strPing);
}

void CGUIMFCHeapSortingDlg::AddPacketLog(const SIM_PACKET& pkt) {
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_PACKET);

	// 1. 시퀀스 번호를 첫 번째 칸(Item)으로 바로 삽입
	CString strSeq;
	strSeq.Format(_T("%d"), pkt.sequence);
	int nIndex = pList->InsertItem(pList->GetItemCount(), strSeq);

	// 2. 나머지 정보들 삽입
	CString strType, strFrame, strPing;
	strType.Format(_T("%d"), pkt.type);
	strFrame.Format(_T("%d"), pkt.curFrame);

	strPing.Format(_T("%d"), m_net.GetCurrentPing());

	pList->SetItemText(nIndex, 1, strType);
	pList->SetItemText(nIndex, 2, strFrame);
	pList->SetItemText(nIndex, 3, strPing);

	// 3. 자동 스크롤 및 개수 제한 (기존과 동일)
	pList->EnsureVisible(nIndex, FALSE);
	if (pList->GetItemCount() > 100) pList->DeleteItem(0);
}
void CGUIMFCHeapSortingDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) {
		m_curFrame++;
		// 현재 상태의 최대 프레임에 도달했을 때
		if (m_curFrame >= m_maxFrames[m_curState]) {
			if (m_curState == GUARD) {
				m_curState = 6; // 방패 들기 끝 -> 든 채로 대기(IDLE2)
				m_curFrame = 0;
			}
			else if (m_curState == ATTACK || m_curState == HIT || m_curState == PARRY) {
				// 동작이 끝났을 때 방어 키를 꾹 누르고 있었다면?
				if (m_isXPressed) {
					m_curState = 4; // 바로 방어 유지(IDLE2) 상태로 전이
					m_curFrame = 0;
				}
				else {
					m_curState = IDLE;
					m_curFrame = 0;
				}
			}
			else {
				m_curFrame = 0; // IDLE, MOVE, IDLE2는 무한 루프
			}
		}

		Invalidate(FALSE);
	}
	// --- [ID 2] 네트워크 통신 (30ms) ---
	else if (nIDEvent == 2) {
		// 1. 서버로 내 현재 상태 보고 (서버는 이걸 보고 판정함)
		m_net.SendPacket(m_curState, m_curFrame);

		// 2. 힙에서 서버 패킷 꺼내기 (상대방의 행동)
		SIM_PACKET enemyPkt;
		if (m_net.GetNextFrame(&enemyPkt, false)) {
			AddPacketLog(enemyPkt); // 리스트 컨트롤에 기록

			// --- 서버 로직과 동일한 상호작용 판정 시작 ---
			if (enemyPkt.type == ATTACK || enemyPkt.type == PARRY) {
				// [중복 판정 방지]
				// 마지막으로 맞았던 시퀀스(m_lastHitSeq)보다 현재 패킷의 시퀀스가 확실히 커야 함
				if (enemyPkt.sequence > m_lastHitSeq) {
					if (enemyPkt.curFrame >= 1 && enemyPkt.curFrame <= 3) {

						if (m_curState == GUARD) {
							m_curState = PARRY;
							m_curFrame = 0;
							// 패링 시에도 이 공격 시퀀스는 끝난 걸로 간주
							m_lastHitSeq = enemyPkt.sequence + (m_maxFrames[ATTACK] - enemyPkt.curFrame);
						}
						else if (m_curState == 6) { // IDLE2 (가드 유지)
							// 가드 중에는 상태 변화가 없으므로 무적 시퀀스 갱신만 해줌 (연타 방지)
							m_lastHitSeq = enemyPkt.sequence + (m_maxFrames[ATTACK] - enemyPkt.curFrame);
						}
						else if (m_curState != HIT && m_curState != PARRY) {
							m_curState = HIT;
							m_curFrame = 0;

							// [중요] 공격의 남은 프레임만큼 시퀀스를 '이미 처리됨'으로 간주해서 무시함
							// 서버가 30ms마다 패킷을 쏘니까, 공격 애니메이션이 끝날 때까지의 시퀀스를 계산
							int remainingFrames = m_maxFrames[ATTACK] - enemyPkt.curFrame;
							m_lastHitSeq = enemyPkt.sequence + remainingFrames;
						}
					}
				}
			}
			// --- 상호작용 판정 끝 ---

			// 상대방 캐릭터 외형 갱신
			m_enemyState = enemyPkt.type;
			m_enemyFrame = enemyPkt.curFrame;

			// 핑 기록.
			m_currentPing = m_net.GetCurrentPing();			//캐릭터 위 표현
			if (m_historyCount < 200) {						//핑 그래프 데이타 기록
				m_latencyHistory[m_historyCount++] = m_currentPing;
			}
			else {
				for (int i = 0; i < 199; i++) m_latencyHistory[i] = m_latencyHistory[i + 1];
				m_latencyHistory[199] = m_currentPing;
			}
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CGUIMFCHeapSortingDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'X' || nChar == 'x') m_isXPressed = true;

	if (m_curState == ATTACK || m_curState == HIT || m_curState == PARRY)
	{
		return;
	}

	switch (nChar)
	{
	case VK_LEFT:
	case VK_RIGHT:
		// 이동은 평시(IDLE)나 가드 대기(IDLE2) 상태에서만 가능하게 제한
		if (m_curState == IDLE || m_curState == 6 || m_curState == MOVE) {
			m_curState = MOVE;
		}
		break;
	case 'Z': // 공격
	case 'z':
		m_curState = ATTACK;
		m_curFrame = 0;
		break;
	case 'X': // 방어
	case 'x':
		// 이미 방어 중(GUARD나 IDLE2)이라면 다시 시작하지 않음
		if (m_curState != GUARD && m_curState != 6) {
			m_curState = GUARD;
			m_curFrame = 0;
		}
		break;
	}

	Invalidate(FALSE);
	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CGUIMFCHeapSortingDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// 키를 떼면 다시 IDLE로 복귀
	if (nChar == VK_LEFT || nChar == VK_RIGHT) {
		// 현재 걷고 있는 중일 때만 IDLE로 복귀
		if (m_curState == MOVE) {
			m_curState = IDLE;
			m_curFrame = 0;
		}
	}
	else if (nChar == 'X' || nChar == 'x') {
		m_isXPressed = false;
		// 현재 방어 중(방패 드는 중 또는 든 채 대기 중)일 때만 IDLE로 복귀
		if (m_curState == GUARD || m_curState == 6) {
			m_curState = IDLE;
			m_curFrame = 0;
		}
	}
	CDialogEx::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL CGUIMFCHeapSortingDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
	{
		// 1. 캐릭터 조작에 사용되는 키인지 먼저 확인
		if (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT ||
			pMsg->wParam == 'Z' || pMsg->wParam == 'X' ||
			pMsg->wParam == 'z' || pMsg->wParam == 'x')
		{
			// 이 키들은 포커스가 어디에 있든(에디트 박스 안이라도) 캐릭터가 가로챕니다.
			if (pMsg->message == WM_KEYDOWN) OnKeyDown((UINT)pMsg->wParam, 0, 0);
			else OnKeyUp((UINT)pMsg->wParam, 0, 0);

			return TRUE; // 여기서 TRUE를 리턴해야 슬라이더나 에디트 박스가 키를 못 훔쳐갑니다.
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CGUIMFCHeapSortingDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 포커스 강제 회수
	this->SetFocus();

	CDialogEx::OnLButtonDown(nFlags, point);
}
