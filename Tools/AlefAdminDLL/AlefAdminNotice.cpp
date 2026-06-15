// AlefAdminNotice.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "AlefAdminDLL.h"
#include "AlefAdminNotice.h"
#include ".\alefadminnotice.h"
#include "AlefAdminManager.h"
#include "AuStrTable.h"
// #include "AgcmChatting2.h"
#include "AgcmUIChatting2.h"


// AlefAdminNotice

IMPLEMENT_DYNCREATE(AlefAdminNotice, CFormView)

AlefAdminNotice::AlefAdminNotice()
	: CFormView(AlefAdminNotice::IDD)
{
	m_lCurrentIndex = 0;
	m_dwLastSendNoticeTick = 0;
	m_ePlayStatus = NOTICE_STOP;
}

AlefAdminNotice::~AlefAdminNotice()
{
}

void AlefAdminNotice::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_E_NOTICE_TEXT, m_csNoticeTextEdit);
	DDX_Control(pDX, IDC_B_SEND, m_csSendBtn);
	DDX_Control(pDX, IDC_B_PLAY, m_csPlayBtn);
	DDX_Control(pDX, IDC_B_PAUSE, m_csPauseBtn);
	DDX_Control(pDX, IDC_B_STOP, m_csStopBtn);
	DDX_Control(pDX, IDC_B_HELP, m_csHelpBtn);
	DDX_Control(pDX, IDC_B_SAVE, m_csSaveBtn);
	DDX_Control(pDX, IDC_B_SAVE_AS, m_csSaveAsBtn);
	DDX_Control(pDX, IDC_B_LOAD, m_csLoadBtn);
	DDX_Control(pDX, IDC_L_NOTICE, m_csNoticeTextList);
	DDX_Control(pDX, IDC_E_NOTICE, m_csNoticeEdit);
	DDX_Control(pDX, IDC_B_DELETE, m_csDeleteBtn);
}

BEGIN_MESSAGE_MAP(AlefAdminNotice, CFormView)
	ON_BN_CLICKED(IDC_B_SEND, OnBnClickedBSend)
	ON_BN_CLICKED(IDC_B_LOAD, OnBnClickedBLoad)
	ON_BN_CLICKED(IDC_B_DELETE, OnBnClickedBDelete)
	ON_LBN_DBLCLK(IDC_L_NOTICE, OnLbnDblclkLNotice)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_B_PLAY, OnBnClickedBPlay)
	ON_BN_CLICKED(IDC_B_PAUSE, OnBnClickedBPause)
	ON_BN_CLICKED(IDC_B_STOP, OnBnClickedBStop)
	ON_BN_CLICKED(IDC_B_SAVE, OnBnClickedBSave)
	ON_BN_CLICKED(IDC_B_SAVE_AS, OnBnClickedBSaveAs)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// AlefAdminNotice 진단입니다.

#ifdef _DEBUG
void AlefAdminNotice::AssertValid() const
{
	CFormView::AssertValid();
}

void AlefAdminNotice::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


// AlefAdminNotice 메시지 처리기입니다.

void AlefAdminNotice::OnInitialUpdate()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	CFormView::OnInitialUpdate();

	InitControls();
	InitNotice();
	m_ePlayStatus = NOTICE_STOP;

	// Timer 발동
	SetTimer(0, 500, NULL);	// 0.5 초에 한번씩
}

BOOL AlefAdminNotice::InitControls()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	DWORD dwStyle = BS_XT_SEMIFLAT | BS_XT_SHOWFOCUS | BS_XT_HILITEPRESSED;

	m_csSendBtn.SetXButtonStyle(dwStyle);
	m_csPlayBtn.SetXButtonStyle(dwStyle);
	m_csPauseBtn.SetXButtonStyle(dwStyle);
	m_csStopBtn.SetXButtonStyle(dwStyle);
	m_csHelpBtn.SetXButtonStyle(dwStyle);
	m_csSaveBtn.SetXButtonStyle(dwStyle);
	m_csSaveAsBtn.SetXButtonStyle(dwStyle);
	m_csLoadBtn.SetXButtonStyle(dwStyle);
	m_csDeleteBtn.SetXButtonStyle(dwStyle);

	m_csNoticeEdit.SetLimitText(MAX_NOTICE_STRING);

	return TRUE;
}

BOOL AlefAdminNotice::SendNotice(LPCTSTR szMessage)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(!szMessage || strlen(szMessage) < 1 || strlen(szMessage) > 240)
		return FALSE;

	CHAR szMsg[255];
	sprintf(szMsg, "%s %s", ClientStr().GetStr(STI_CHAT_NOTICE_ENG), szMessage);

	AgcmUIChatting2* pcmUIChatting = ( AgcmUIChatting2* )AlefAdminManager::Instance()->GetAdminModule()->GetModule( "AgcmUIChatting2" );
	pcmUIChatting->OnSendMessage(AGPDCHATTING_TYPE_NORMAL, szMsg, (INT32)strlen(szMsg), NULL);

	return TRUE;
}

// 풀 패스가 온다.
BOOL AlefAdminNotice::LoadFile(LPCTSTR szPathName)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(!szPathName || strlen(szPathName) == 0)
		return FALSE;

	CStdioFile csFileStream;
	if(!csFileStream.Open(szPathName, CFile::modeRead))
		return FALSE;

	UINT32 lSize = (UINT32)csFileStream.GetLength();
	if(lSize == 0)
		return FALSE;

	INT32 lLength = 0;
	CString szTotal = _T("");
	CString szLine = _T("");

	INT32 lIndex = 0;
	INT32 lDelay = -1;
	DWORD dwColor = 0;
	while(csFileStream.ReadString(szLine))
	{
		lLength = szLine.GetLength();
		if(lLength < 2)
			break;

		if(szLine.GetAt(lLength - 1) != '\r')
			szLine += "\r";

		szLine += "\n";
		szTotal += szLine;

		if(!CheckSetNoticeItem(szLine, &lDelay, &dwColor))
		{
			if(lDelay == -1 && lIndex > 0 && (INT32)m_vectorNoticeItem.size() >= lIndex)
			{
				// Config 가 안나오고 문장이 두줄 연속 나온경우
				m_vectorNoticeItem[lIndex].m_szNotice += " ";
				m_vectorNoticeItem[lIndex].m_szNotice += szLine.Left(szLine.GetLength() - 2);
			}
			else
			{
				CNoticeItem cNoticeItem;
				cNoticeItem.m_lDelay = lDelay;
				cNoticeItem.m_dwColor = dwColor;
				cNoticeItem.m_szNotice = szLine.Left(szLine.GetLength() - 2);	// "\r\n" 은 뺀다.

				m_vectorNoticeItem.push_back(cNoticeItem);

				lDelay = -1;
				dwColor = 0;
			}
		}

		lIndex++;
	}

	// 화면에 뿌린다.
	m_csNoticeTextEdit.SetWindowText((LPCTSTR)szTotal);

	csFileStream.Close();

	return TRUE;
}

// String 을 검사해서 Config 라면 포인터에 값을 넣어주고 return TRUE, 아니라면 그냥 return FALSE
BOOL AlefAdminNotice::CheckSetNoticeItem(CString& szLine, INT32* plDelay, DWORD* pdwColor)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(szLine.GetLength() == 0)
		return FALSE;

	// Config 라면 처음에 C| 로 시작한다. C 하구 |(\)
	if(szLine.Left(2).Compare(_T("C|")) != 0)
		return FALSE;

	INT32 lLength = szLine.GetLength();
	INT32 lStart = 2, lEnd = 0;
	for(INT32 lIndex = 2; lIndex < lLength; )
	{
		if(szLine.GetAt(lIndex) != '|')
		{
			lEnd = ++lIndex;
			continue;
		}

		CString szConfig = szLine.Mid(lStart, lEnd - lStart);
		if(szConfig.GetLength() < 3)
		{
			lStart = ++lIndex;
			continue;
		}

		switch(szConfig.GetAt(0))
		{
			case 'D':
			case 'd':
			{
				*plDelay = atoi((LPCTSTR)szConfig.Mid(2, szConfig.GetLength() - 2));
				break;
			}

			case 'C':
			case 'c':
			{
				*pdwColor = 0xFFFFFF;	// 현재 색은 지원하지 않는다.
				break;
			}
		}

		lStart = ++lIndex;
	}

	return TRUE;
}

BOOL AlefAdminNotice::SaveFile(LPCTSTR szPathName)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(!szPathName || strlen(szPathName) == 0)
		return FALSE;

	// 파일을 연다.
	BOOL bOpened = FALSE;

	CStdioFile csFileStream;
	CFileFind cFileFind;
	if(cFileFind.FindFile(szPathName))
		bOpened = csFileStream.Open((LPCTSTR)szPathName, CFile::modeReadWrite | CFile::shareDenyRead | CFile::shareDenyWrite);
	else
		bOpened = csFileStream.Open((LPCTSTR)szPathName, CFile::modeCreate | CFile::modeReadWrite);

	if(!bOpened)
		return FALSE;

	CString szNotice = _T("");
	m_csNoticeTextEdit.GetWindowText(szNotice);

	csFileStream.SetLength(szNotice.GetLength());
	csFileStream.SeekToBegin();
	csFileStream.WriteString((LPCTSTR)szNotice);
	csFileStream.Flush();

	csFileStream.Close();

	return TRUE;
}

BOOL AlefAdminNotice::InitNotice()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	// 먼저 화면을 지우고
	m_csNoticeTextEdit.SetWindowText(_T(""));

	m_vectorNoticeItem.clear();
	m_dwLastSendNoticeTick = 0;

	return TRUE;
}

BOOL AlefAdminNotice::ProcessNotice()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(m_vectorNoticeItem.empty())
		return FALSE;

	// 현재 상태가 Play 가 아니면 나간다.
	if(m_ePlayStatus != NOTICE_PLAY)
		return FALSE;

	// 끝까지 재생을 다 했다.
	if(m_lCurrentIndex >= (INT32)m_vectorNoticeItem.size())
	{
		OnBnClickedBStop();
				
		m_lCurrentIndex = 0;	// 현재 인덱스를 0 으로 해준다.

		m_csNoticeTextEdit.SetSel(-1, 0);	// 선택된 거 지운다.

		return TRUE;
	}
	
	DWORD dwTickCount = ::GetTickCount();
	if(m_dwLastSendNoticeTick == 0)
		m_dwLastSendNoticeTick = dwTickCount;

	if((INT32)(dwTickCount - m_dwLastSendNoticeTick) < m_vectorNoticeItem[m_lCurrentIndex].m_lDelay * 1000)
		return FALSE;

	// 다음꺼 보낼 시간이 됬다.
	m_dwLastSendNoticeTick = dwTickCount;

	// 공지사항 입력창에 세팅해주고
	m_csNoticeEdit.SetWindowText((LPCTSTR)m_vectorNoticeItem[m_lCurrentIndex].m_szNotice);
	OnBnClickedBSend();

	// 현재 어느 위치인지 표시해준다.
	SetCurNotice();

	m_lCurrentIndex++;

	return TRUE;
}

BOOL AlefAdminNotice::SetCurNotice()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(m_vectorNoticeItem.empty())
		return FALSE;

	// 원래 있던거 비운다.
	m_csNoticeTextEdit.SetSel(-1, 0);

	// 고쳐진게 많아서 원래 선택되었던 게 전체를 넘어가 있다면
	if(m_lCurrentIndex * 2 >= m_csNoticeTextEdit.GetLineCount())
		m_lCurrentIndex = 0;	// 맨 처음으로 돌려준다.

	INT32 lStartIndex = -1;
	INT32 lEndIndex = 0;
	INT32 lCount = 0;

	CString szNotice = _T("");
	m_csNoticeTextEdit.GetWindowText(szNotice);

	// 현재 위치까지 찾아간다.
	for(INT32 iIndex = 0; iIndex < szNotice.GetLength(); iIndex++)
	{
		if(szNotice.GetAt(iIndex) != '\n')
			continue;

		lCount++;	// 몇번째 줄인지 센다.
		
		if(m_lCurrentIndex * 2 <= lCount)
		{
			lStartIndex = iIndex + 1;

			// 첫번째 줄이라면 한번
			if(lCount == 1)
			{
				lEndIndex = szNotice.Find('\n', lStartIndex);
			}
			else
			{
				// 두번 돈다.
				lEndIndex = szNotice.Find('\n', lStartIndex);
				if(lEndIndex == -1)
					lEndIndex = szNotice.GetLength();
				else if(lEndIndex + 1 < szNotice.GetLength())
				{
					lStartIndex = lEndIndex + 1;
					lEndIndex = szNotice.Find('\n', lEndIndex + 1);
				}
			}

			if(lEndIndex == -1)
				lEndIndex = szNotice.GetLength();

			break;
		}
	}

	m_csNoticeTextEdit.SetSel(lStartIndex, lEndIndex);

	return TRUE;
}









void AlefAdminNotice::OnBnClickedBSend()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	CHAR szMessage[255];
	memset(szMessage, 0, sizeof(szMessage));

	m_csNoticeEdit.GetWindowText(szMessage, 255);

	SendNotice(szMessage);
}


void AlefAdminNotice::OnBnClickedBLoad()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	CHAR szFilters[]= "Files (*.txt)|*.txt|All Files (*.*)|*.*||";

	CFileDialog cFileDlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, this);

	//CHAR* szFilePathBuf = new CHAR[MAX_NOTICE_TEXT_LIST * MAX_PATH];
    cFileDlg.GetOFN().Flags &= ~OFN_ENABLESIZING;
	cFileDlg.GetOFN().Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	//cFileDlg.GetOFN().lStructSize = sizeof(OPENFILENAME) + 12;
    cFileDlg.GetOFN().lpstrTitle = _T("Load Notice File(s)");
    //cFileDlg.GetOFN().lpstrFile = szFilePathBuf;
    //cFileDlg.GetOFN().nMaxFile = MAX_NOTICE_TEXT_LIST * MAX_PATH;

	if(cFileDlg.DoModal() != IDOK)
		return;

	// 중복은 신경쓰지 않고 일단 밀어넣는다.
	int iIndex = 0;
	POSITION pos = cFileDlg.GetStartPosition();
	while(pos)
	{
		CString szPathName = cFileDlg.GetNextPathName(pos);
		iIndex = szPathName.ReverseFind('\\');
		CString szFileName = szPathName.Mid(iIndex + 1);

		m_vectorNoticeFile.push_back(szPathName);
		m_csNoticeTextList.AddString((LPCTSTR)szFileName);
	}

	//delete [] szFilePathBuf;
}

void AlefAdminNotice::OnBnClickedBDelete()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	int iCurSel = m_csNoticeTextList.GetCurSel();
	if(iCurSel == CB_ERR || iCurSel < 0)
		return;

	m_csNoticeTextList.DeleteString(iCurSel);

	if(iCurSel + 1 >= (INT32)m_vectorNoticeFile.size())
		return;
	
	// Vector 에서도 지운다.
	vector<CString>::iterator iterData = m_vectorNoticeFile.begin();
	for(int i = 0; i < iCurSel && iterData != m_vectorNoticeFile.end(); i++)
		iterData++;

	m_vectorNoticeFile.erase(iterData);
}

void AlefAdminNotice::OnLbnDblclkLNotice()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	int iCurSel = m_csNoticeTextList.GetCurSel();
	if(iCurSel == CB_ERR || iCurSel < 0 || iCurSel >= (INT32)m_vectorNoticeFile.size())
		return;

	InitNotice();
	LoadFile((LPCTSTR)m_vectorNoticeFile[iCurSel]);

	// 파일이름도 세팅해준다.
	SetDlgItemText(IDC_E_NOTICE_FILENAME, (LPCTSTR)m_vectorNoticeFile[iCurSel]);
}

void AlefAdminNotice::OnDestroy()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	CFormView::OnDestroy();

	KillTimer(0);

	m_vectorNoticeFile.clear();
	InitNotice();
}

void AlefAdminNotice::OnTimer(UINT nIDEvent)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(nIDEvent == 0)
	{
		ProcessNotice();
	}

	CFormView::OnTimer(nIDEvent);
}

void AlefAdminNotice::OnBnClickedBPlay()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(m_vectorNoticeItem.empty())
		return;

	// 각 버튼 상태를 바꿔주고
	m_csPlayBtn.SetCheck(BST_CHECKED);
	m_csPlayBtn.SetStateX(TRUE);
	m_csPauseBtn.SetCheck(BST_UNCHECKED);
	m_csPauseBtn.SetStateX(FALSE);

	// 현재 상태를 Play 로 바꾼다.
	m_ePlayStatus = NOTICE_PLAY;
}

void AlefAdminNotice::OnBnClickedBPause()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(m_vectorNoticeItem.empty())
		return;

	if(m_ePlayStatus == NOTICE_PLAY)
	{
		m_csPlayBtn.SetCheck(BST_UNCHECKED);
		m_csPlayBtn.SetStateX(FALSE);
		m_csPauseBtn.SetCheck(BST_CHECKED);
		m_csPauseBtn.SetStateX(TRUE);

		m_ePlayStatus = NOTICE_PAUSE;
	}
	else if(m_ePlayStatus == NOTICE_PAUSE)
	{
		m_csPlayBtn.SetCheck(BST_CHECKED);
		m_csPlayBtn.SetStateX(TRUE);
		m_csPauseBtn.SetCheck(BST_UNCHECKED);
		m_csPauseBtn.SetStateX(FALSE);

		m_ePlayStatus = NOTICE_PLAY;
	}
}

void AlefAdminNotice::OnBnClickedBStop()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(m_vectorNoticeItem.empty())
		return;

	m_csPlayBtn.SetCheck(BST_UNCHECKED);
	m_csPlayBtn.SetStateX(FALSE);
	m_csPauseBtn.SetCheck(BST_UNCHECKED);
	m_csPauseBtn.SetStateX(FALSE);

	m_ePlayStatus = NOTICE_STOP;

	m_lCurrentIndex = 0;
}

void AlefAdminNotice::OnBnClickedBSave()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	CString szPathName = _T("");
	GetDlgItemText(IDC_E_NOTICE_FILENAME, szPathName);

	SaveFile((LPCTSTR)szPathName);

	// 기존 Index 를 백업하고
	INT32 lCurrentIndex = m_lCurrentIndex;

	// 다시 로드해준다.
	InitNotice();
	LoadFile((LPCTSTR)szPathName);

	m_lCurrentIndex = lCurrentIndex;
	SetCurNotice();
}

void AlefAdminNotice::OnBnClickedBSaveAs()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	CHAR szFilters[]= "Files (*.txt)|*.txt|All Files (*.*)|*.*||";

	CFileDialog cFileDlg(FALSE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, this);
    cFileDlg.GetOFN().lpstrTitle = _T("Save As Notice File");

	if(cFileDlg.DoModal() != IDOK)
		return;

	CString szPathName = cFileDlg.GetPathName();
	SaveFile((LPCTSTR)szPathName);

	// 새로 변경된 내용으로 적용할 것인지 확인한다.
	if(AfxMessageBox(IDS_NOTICE_ASK_APPLY, MB_OKCANCEL) == IDOK)
	{
		// 다시 로드해준다.
		InitNotice();
		LoadFile((LPCTSTR)szPathName);

		// 파일이름도 세팅해준다.
		SetDlgItemText(IDC_E_NOTICE_FILENAME, (LPCTSTR)szPathName);
	}
}

int AlefAdminNotice::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	if(CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL AlefAdminNotice::OnEraseBkgnd(CDC* pDC)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	UNREFERENCED_PARAMETER(pDC);	
	return TRUE;
	//return CFormView::OnEraseBkgnd(pDC);
}

void AlefAdminNotice::OnPaint()
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CFormView::OnPaint()을(를) 호출하지 마십시오.

	CRect r;
	GetClientRect(&r);
	CXTMemDC memDC(&dc, r);
	
	CFormView::DefWindowProc(WM_PAINT, (WPARAM)memDC.m_hDC, 0);
}
