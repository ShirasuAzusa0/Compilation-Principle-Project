
// Compilation_principle_experiment2Dlg.h: 头文件
//

#pragma once


// CCompilationprincipleexperiment2Dlg 对话框
class CCompilationprincipleexperiment2Dlg : public CDialogEx
{
// 构造
public:
	CCompilationprincipleexperiment2Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMPILATION_PRINCIPLE_EXPERIMENT2_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtontonfaanddfa();
	afx_msg void OnBnClickedButtonMindfa();
	afx_msg void OnBnClickedButtonGenerate();
	// 正则表达式输入框变量
	CEdit m_REGULAR;
	// 词法分析程序显示栏变量
	CEdit m_PROGRAM;
	// 状态栏变量
	CEdit m_STATUS;
	// NFA、DFA、最小化DFA图显示框变量
	CStatic m_PIC;
	afx_msg void OnBnClickedButtonNfa();
	afx_msg void OnBnClickedButtonDfa();
	CEdit m_Path;
};
