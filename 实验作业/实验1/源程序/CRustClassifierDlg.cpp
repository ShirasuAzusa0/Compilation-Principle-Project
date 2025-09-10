// CRustClassifierDlg.cpp: 实现文件
//

#include "pch.h"
#include "Compilation_principle_experiment1.h"
#include "afxdialogex.h"
#include "CRustClassifierDlg.h"
#include "CodeClassifier.h"

// 获取下拉列表选择数据
string combodata;

// 初始化分类器
CodeClassifier classifier;

// CRustClassifierDlg 对话框

IMPLEMENT_DYNAMIC(CRustClassifierDlg, CDialogEx)

CRustClassifierDlg::CRustClassifierDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLASSIFIER, pParent)
{

}

CRustClassifierDlg::~CRustClassifierDlg()
{
}

void CRustClassifierDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RUSTCODE, m_RustCode);
	DDX_Control(pDX, IDC_EDIT_RESULT, m_Result);
	DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_CodeSelect);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_Status);
}


BEGIN_MESSAGE_MAP(CRustClassifierDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CLASSIFIER, &CRustClassifierDlg::OnBnClickedClassifier)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CRustClassifierDlg 消息处理程序

void CRustClassifierDlg::OnBnClickedClassifier()
{
	// TODO: 在此添加控件通知处理程序代码
	CString c_code, c_combodata;
	m_RustCode.GetWindowTextW(c_code);
	string code = CW2A(c_code);

	// 获取下拉列表中的选项并保存到c_combodata中
	int option = m_CodeSelect.GetCurSel();
	m_CodeSelect.GetLBText(option, c_combodata);
	// 将CString类型的c_combodata转换为string类型的combodata
	combodata = string(CW2A(c_combodata.GetString()));
	classifier.SetType(combodata);
	string res = classifier.Classify(code);
	// 显示最终的单词分类结果
	if (res == "NULL")
	{
		m_Result.SetWindowTextW(L"Error, wrong language choice!");
		m_Status.SetWindowTextW(L"分类出错，语言选择错误！");
		return;
	}
	CString c_res(res.c_str());
	m_Result.SetWindowTextW(c_res);
	m_Status.SetWindowTextW(L"分类完成！");
}


void CRustClassifierDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//获取文件路径
	TCHAR szPath[MAX_PATH] = { 0 };
	DragQueryFile(hDropInfo, 0, szPath, MAX_PATH);
	//显示到控件上
	UpdateData(TRUE);

	CDialogEx::OnDropFiles(hDropInfo);
}
