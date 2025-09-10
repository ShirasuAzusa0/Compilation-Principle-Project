
// Compilation_principle_experiment2Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Compilation_principle_experiment2.h"
#include "Compilation_principle_experiment2Dlg.h"
#include "afxdialogex.h"
#include "DFAbuilder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CCompilationprincipleexperiment2Dlg 对话框



CCompilationprincipleexperiment2Dlg::CCompilationprincipleexperiment2Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMPILATION_PRINCIPLE_EXPERIMENT2_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCompilationprincipleexperiment2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_REGULAR, m_REGULAR);
	DDX_Control(pDX, IDC_EDIT_PROGRAM, m_PROGRAM);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_STATUS);
	DDX_Control(pDX, IDC_PIC_NFAandDFA, m_PIC);
	DDX_Control(pDX, IDC_EDIT_PATH, m_Path);
}

BEGIN_MESSAGE_MAP(CCompilationprincipleexperiment2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_toNFAandDFA, &CCompilationprincipleexperiment2Dlg::OnBnClickedButtontonfaanddfa)
	ON_BN_CLICKED(IDC_BUTTON_MINDFA, &CCompilationprincipleexperiment2Dlg::OnBnClickedButtonMindfa)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE, &CCompilationprincipleexperiment2Dlg::OnBnClickedButtonGenerate)
	ON_BN_CLICKED(IDC_BUTTON_NFA, &CCompilationprincipleexperiment2Dlg::OnBnClickedButtonNfa)
	ON_BN_CLICKED(IDC_BUTTON_DFA, &CCompilationprincipleexperiment2Dlg::OnBnClickedButtonDfa)
END_MESSAGE_MAP()

// 创建初始化DFAbuilder实例
DFAbuilder builder;

// 正则表达式容器（处理多行）
vector<string> regex;

// 正则表达式容器（仅前字符名）
vector<string> frontparts;

// 正则表达式容器（纯粹表达式内容）
vector<string> backparts;

// 存放接受状态的容器
vector<string> endstates;

// 存放非接受状态的容器
vector<string> notendstates;

// 词法分析程序暂存string变量
string code;


// CCompilationprincipleexperiment2Dlg 消息处理程序

BOOL CCompilationprincipleexperiment2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCompilationprincipleexperiment2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCompilationprincipleexperiment2Dlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCompilationprincipleexperiment2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCompilationprincipleexperiment2Dlg::OnBnClickedButtontonfaanddfa()
{
	// TODO: 在此添加控件通知处理程序代码
	// 正则表达式字符串获取
	CString c_regular;
	m_REGULAR.GetWindowTextW(c_regular);
	string regular = CW2A(c_regular);

	bool dfaflag = false;

	regex.clear();
	frontparts.clear();
	backparts.clear();
	endstates.clear();

	// 正则表达式字符串处理（考虑多行和单行的情况）
	string subre = "";
	for (size_t i = 0; i < regular.size(); ++i)
	{
		subre += string(1, regular[i]);
		if (regular[i + 1] == '\r' && regular[i + 2] == '\n' && i + 2 < regular.size())
		{
			regex.push_back(subre);
			i += 2;
			subre = "";
		}
	}
	regex.push_back(subre);
	
	// 针对每行正则表达式，按等号前后划分，前半截只取单词名，后半截取正则表达式
	for (string item : regex)
	{
		int pos = item.find('=');
		string fronttemp = item.substr(0, pos);
		string backtemp = item.substr(pos + 1);
		string frontpart = "";
		for (char c : fronttemp)
		{
			if (!isalpha(c) && c != '_') break;
			else if (c == '_')
			{
				dfaflag = true;
				continue;
			}
			frontpart += c;
		}
		frontparts.push_back(frontpart);
		backparts.push_back(backtemp);
	}

	// 处理多行正则表达式（每行单独处理）
	for (size_t i = 0; i < regex.size(); ++i)
	{
		string regular = regex[i];
		string re = regular.substr(regular.find('=') + 1);
		vector <DFAState> dfastates;
		vector <DFATransition> dfatransitions;
		set<string> nfa_Initial_StateSet;
		bool flag = false;
		// 考虑当前正则表达式的符号是否为前若干行表达式对应的单词名
		for (string str : frontparts)
			if (re.find(str) != -1)
			{
				flag = true;
				break;
			}
		
		string processed, postfix;
		re = builder.opChange(re);
		processed = builder.addConcatenation(re, frontparts);
		
		postfix = builder.InfixtoPostfix(processed);
		
		Elem NFA_elem = builder.buildNFA(postfix, frontparts);
		builder.buildDFA(NFA_elem, dfastates, dfatransitions);
		generateDotFile_NFA(NFA_elem, frontparts[i]);
		if(dfaflag) generateDotFile_DFA(endstates, dfastates, dfatransitions, NFA_elem, frontparts[i]);
	}

	vector <DFAState> dfastates;
	vector <DFATransition> dfatransitions;
	string total_regex = regexReplace(frontparts, backparts, backparts.back());
	total_regex = builder.opChange(total_regex);
	string processed = builder.addConcatenation(total_regex, frontparts);
	string postfix = builder.InfixtoPostfix(processed);
	Elem NFA_elem = builder.buildNFA(postfix, frontparts);
	builder.buildDFA(NFA_elem, dfastates, dfatransitions);
	DFAbuilder::minimizeDFA(endstates, notendstates, dfatransitions);
	code = code_generate(frontparts.back(),endstates, dfatransitions);

	m_STATUS.SetWindowTextW(L"有穷自动机的图示已生成完成！bmp格式图可以直接通过对应按钮打开查看，另外也可以在本地打开svg格式图");
}

void CCompilationprincipleexperiment2Dlg::OnBnClickedButtonNfa()
{
	// TODO: 在此添加控件通知处理程序代码
	// 显示nfa图
	m_PIC.ModifyStyle(0, SS_BITMAP);
	CBitmap bitmap;
	CImage img;
	CString c_filename;
	m_Path.GetWindowTextW(c_filename);
	if (img.Load(_T(".\\") + c_filename + _T(".bmp")) == S_OK)
	{
		HBITMAP hBmp = img.Detach();
		if (HBITMAP hOld = m_PIC.GetBitmap())
			DeleteObject(hOld);
		m_PIC.SetBitmap(hBmp);
		m_STATUS.SetWindowTextW(L"NFA图已显示！");
	}
	else
	{
		DWORD err = GetLastError();
		CString msg;
		msg.Format(_T("error: 文件不存在！"), err);
		AfxMessageBox(msg);
	}
}


void CCompilationprincipleexperiment2Dlg::OnBnClickedButtonDfa()
{
	// TODO: 在此添加控件通知处理程序代码
	// 显示dfa图
	m_PIC.ModifyStyle(0, SS_BITMAP);
	CBitmap bitmap;
	CImage img;
	CString c_filename;
	m_Path.GetWindowTextW(c_filename);
	if (img.Load(_T(".\\") + c_filename + _T(".bmp")) == S_OK)
	{
		HBITMAP hBmp = img.Detach();
		if (HBITMAP hOld = m_PIC.GetBitmap())
			DeleteObject(hOld);
		m_PIC.SetBitmap(hBmp);
		m_STATUS.SetWindowTextW(L"DFA图已显示！");
	}
	else
	{
		DWORD err = GetLastError();
		CString msg;
		msg.Format(_T("error: 文件不存在！"), err);
		AfxMessageBox(msg);
	}
}

void CCompilationprincipleexperiment2Dlg::OnBnClickedButtonMindfa()
{
	// TODO: 在此添加控件通知处理程序代码
	// 显示dfa图
	m_PIC.ModifyStyle(0, SS_BITMAP);
	CBitmap bitmap;
	CImage img;
	CString c_filename;
	m_Path.GetWindowTextW(c_filename);
	if (img.Load(_T(".\\") + c_filename + _T(".bmp")) == S_OK)
	{
		HBITMAP hBmp = img.Detach();
		if (HBITMAP hOld = m_PIC.GetBitmap())
			DeleteObject(hOld);
		m_PIC.SetBitmap(hBmp);
		m_STATUS.SetWindowTextW(L"最小化DFA图已显示！");
	}
	else
	{
		DWORD err = GetLastError();
		CString msg;
		msg.Format(_T("error: 文件不存在！"), err);
		AfxMessageBox(msg);
	}
}


void CCompilationprincipleexperiment2Dlg::OnBnClickedButtonGenerate()
{
	// TODO: 在此添加控件通知处理程序代码
	wstring wcode(code.begin(), code.end());
	m_PROGRAM.SetWindowTextW(wcode.c_str());
	m_STATUS.SetWindowTextW(L"词法分析程序已生成！");
}
