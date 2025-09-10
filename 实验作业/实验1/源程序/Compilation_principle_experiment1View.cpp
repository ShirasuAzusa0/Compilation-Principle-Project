
// Compilation_principle_experiment1View.cpp: CCompilationprincipleexperiment1View 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Compilation_principle_experiment1.h"
#endif

#include "Compilation_principle_experiment1Doc.h"
#include "Compilation_principle_experiment1View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCompilationprincipleexperiment1View

IMPLEMENT_DYNCREATE(CCompilationprincipleexperiment1View, CView)

BEGIN_MESSAGE_MAP(CCompilationprincipleexperiment1View, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CCompilationprincipleexperiment1View 构造/析构

CCompilationprincipleexperiment1View::CCompilationprincipleexperiment1View() noexcept
{
	// TODO: 在此处添加构造代码

}

CCompilationprincipleexperiment1View::~CCompilationprincipleexperiment1View()
{
}

BOOL CCompilationprincipleexperiment1View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CCompilationprincipleexperiment1View 绘图

void CCompilationprincipleexperiment1View::OnDraw(CDC* /*pDC*/)
{
	CCompilationprincipleexperiment1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CCompilationprincipleexperiment1View 打印

BOOL CCompilationprincipleexperiment1View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CCompilationprincipleexperiment1View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CCompilationprincipleexperiment1View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CCompilationprincipleexperiment1View 诊断

#ifdef _DEBUG
void CCompilationprincipleexperiment1View::AssertValid() const
{
	CView::AssertValid();
}

void CCompilationprincipleexperiment1View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCompilationprincipleexperiment1Doc* CCompilationprincipleexperiment1View::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCompilationprincipleexperiment1Doc)));
	return (CCompilationprincipleexperiment1Doc*)m_pDocument;
}
#endif //_DEBUG


// CCompilationprincipleexperiment1View 消息处理程序
