// NetTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "NetTest.h"
#include "NetTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNetTestDlg �Ի���




CNetTestDlg::CNetTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNetTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNetTestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CNetTestDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CNetTestDlg::OnBnClickedOk)
	ON_WM_CREATE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CNetTestDlg ��Ϣ�������

BOOL CNetTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	_CrtSetBreakAlloc(149);

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CNetTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CNetTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CNetTestDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_serversocket.SetServerIp("127.0.0.1");
	m_serversocket.SetServerPort(9100);
	m_ServerScketStatue = m_serversocket.Initialize();


}

void CNetTestDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
  if (m_ServerScketStatue) 
	m_serversocket.Finalization();
	OnOK();
}

int CNetTestDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_ServerScketStatue = false;
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������

	return 0;
}

void CNetTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	
	CDialog::OnTimer(nIDEvent);
}
