#pragma once
#include "afxdialogex.h"


// CRustClassifierDlg 对话框

class CRustClassifierDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRustClassifierDlg)

public:
	CRustClassifierDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CRustClassifierDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLASSIFIER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_RustCode;
	CEdit m_Result;
	CComboBox m_CodeSelect;
	CEdit m_Status;
	afx_msg void OnBnClickedClassifier();
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
