
// GUI_MFC_HeapSortingDlg.h: 헤더 파일
//

#pragma once
#include <gdiplus.h>
#include "CPacketHeap.h"
#pragma comment(lib, "gdiplus.lib")

// CGUIMFCHeapSortingDlg 대화 상자
class CGUIMFCHeapSortingDlg : public CDialogEx
{
// 생성입니다.
public:
	CGUIMFCHeapSortingDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GUI_MFC_HEAPSORTING_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()



public:
	// 함수 선언
	void DrawCharacter(CDC* pDC, int x, int y, int state, int frame);
	void DrawPingGraph(CDC* pDC);

private:
	// 변수 선언(캐릭터)
	Gdiplus::Image* m_pCharSprite = nullptr;	// 캐릭터 스프라이트 이미지
	Gdiplus::Image* m_pSprites[6];				// 이미지 객체들을 담을 배열
	int m_curState = 0;							// 현재 캐릭터 상태

	// 변수 선언 Network
	int m_latencyHistory[200] = { 0, };			// 핑 데이터 저장용 배열
	int m_historyCount = 0;						// 현재까지 쌓인 데이터 개수
	int m_curFrame = 0;							// 현재 애니메이션 프레임
	PacketHeap m_packetHeap;
};
