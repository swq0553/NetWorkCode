// NetTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "NetTest.h"
#include "NetTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNetTestDlg 对话框




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


// CNetTestDlg 消息处理程序

BOOL CNetTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	_CrtSetBreakAlloc(149);

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNetTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CNetTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CNetTestDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_serversocket.SetServerIp("127.0.0.1");
	m_serversocket.SetServerPort(9100);
	m_ServerScketStatue = m_serversocket.Initialize();


}

void CNetTestDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
  if (m_ServerScketStatue) 
	m_serversocket.Finalization();
	OnOK();
}

int CNetTestDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_ServerScketStatue = false;
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}

void CNetTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	
	CDialog::OnTimer(nIDEvent);
}
