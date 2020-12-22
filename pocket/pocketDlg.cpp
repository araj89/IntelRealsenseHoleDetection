
// pocketDlg.cpp : implementation file
//

#include "stdafx.h"
#include "pocket.h"
#include "pocketDlg.h"
#include "afxdialogex.h"
#include "detect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern int g_nScreenW;
extern int g_nScreenH;

// CpocketDlg dialog

CpocketDlg::CpocketDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CpocketDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ImgWndRect = CRect(0, 0, 500, 300);
	m_bDetectStart = FALSE;
}

void CpocketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_IMG, m_ImgListCtl);
}

BEGIN_MESSAGE_MAP(CpocketDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_BN_CLICKED(IDCANCEL, &CpocketDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CpocketDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_ADD_FILE, &CpocketDlg::OnBnClickedBtnAddFile)
	ON_BN_CLICKED(IDC_BTN_REMOVE, &CpocketDlg::OnBnClickedBtnRemove)
	ON_LBN_DBLCLK(IDC_LIST_IMG, &CpocketDlg::OnDblclkListImg)
	ON_BN_CLICKED(IDC_BTN_DETECT, &CpocketDlg::OnBnClickedBtnDetect)
	ON_BN_CLICKED(IDC_BTN_ADD_FOLDER, &CpocketDlg::OnBnClickedBtnAddFolder)
END_MESSAGE_MAP()


// CpocketDlg message handlers

BOOL CpocketDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	if (!WndInit())	return FALSE;
	
#if 0
	HObject Image;
	read_image(&Image, "D:/Project/D415/sample-image/00003 (1).jpg");

	HTuple width, height;
	get_image_size(Image, &width, &height);

	HWindow w(0, 0, width, height);
	Image.DispObj(w);
	w.Click();

	HObject EdgeAmplitude, Edges, EdgesExtended;
	sobel_amp(Image, &EdgeAmplitude, "thin_sum_abs", 3);
	EdgeAmplitude.DispObj(w);
	w.Click();
	threshold(EdgeAmplitude, &Edges, 30, 255);
	Edges.DispObj(w);
	w.Click();
	close_edges(Edges, EdgeAmplitude, &EdgesExtended, 1);
	w.SetColor("red");
	EdgesExtended.DispObj(w);
	w.Click();
#endif

//	action();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CpocketDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CpocketDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CpocketDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	if (m_ImgWnd.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, m_ImgWndRect, this, 111) == -1)
	{
		return -1;
	}

	return 0;
}

BOOL CpocketDlg::WndInit()
{

	//. SetWindowPos(NULL, 0, 0, g_nScreenW / 2, g_nScreenH / 2, SWP_SHOWWINDOW);

	m_ImgWnd.MoveWindow(510, 8, 720, 460);

	return TRUE;
}


void CpocketDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CpocketDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CpocketDlg::OnBnClickedBtnAddFile()
{
	CString str;
	BOOL isOpen = TRUE;

	TCHAR szFile[0x100] = { 0, };
	GetModuleFileName(NULL, szFile, 0x100);

	CString defaultDir = szFile;
	defaultDir = defaultDir.Left(defaultDir.ReverseFind('\\'));
	CString fileName = _T("");
	CString filter = _T("txt(*.txt)|*.txt|*(*.*)|*.*|");
	CFileDialog openFileDlg(isOpen, NULL, defaultDir + "\\" + fileName, OFN_READONLY, filter, NULL);
	INT_PTR result = openFileDlg.DoModal();
	if (result == IDOK)
	{
		fileName = openFileDlg.GetPathName();
#if 0
		fileName.Replace(L"\\", L"/");
		switch (openFileDlg.m_ofn.nFilterIndex)
		{
		case 1:
			filter = L"bmp";
			break;
		case 2:
			filter = L"jpg";
		}

		USES_CONVERSION;
		char *file = T2A(fileName);
		char *filterCH = T2A(filter);
#endif
		CT2A tmp(fileName);
		char *img_path = txt2jpg(tmp.m_psz);
		CString cstr_tmp(img_path);

		if (!DrawImg(cstr_tmp))
			return;

		m_ImgListCtl.AddString(fileName);
		m_ImgListCtl.SetCurSel(m_ImgListCtl.GetCount() - 1);
	}
}

void CpocketDlg::OnBnClickedBtnRemove()
{
	int selIdx = m_ImgListCtl.GetCurSel();

	if (selIdx == -1) {
		AfxMessageBox(_T("Select image to delete correctly."));
		return;
	}

	m_ImgListCtl.DeleteString(selIdx);

	if (m_ImgListCtl.GetCount() == 0)
		DrawImg(_T(""));
	else
	{
		m_ImgListCtl.SetCurSel(0);
		OnDblclkListImg();
	}

	AfxMessageBox(_T("Deleted Successfully."));
}

void CpocketDlg::OnBnClickedBtnDetect()
{
	int selIdx = m_ImgListCtl.GetCurSel();

	if (selIdx == -1) {
		AfxMessageBox(_T("Select image to detect correctly."));
		return;
	}

	Detect(selIdx);
	m_bDetectStart = TRUE;
}


void CpocketDlg::OnDblclkListImg()
{
	DrawSelectedImg(m_ImgListCtl.GetCurSel());

	//if (m_bDetectStart)
	//	Detect(m_ImgListCtl.GetCurSel());
}

BOOL CpocketDlg::DrawImg(CString filename)
{
	return m_ImgWnd.DrawImg(filename);
}

BOOL CpocketDlg::DrawSelectedImg(int selIdx)
{

	CString filename;
	m_ImgListCtl.GetText(selIdx, filename);
	
	CT2A tmp(filename);
	char *img_path = txt2jpg(tmp.m_psz);
	CString cstr_tmp(img_path);

	return DrawImg(cstr_tmp);
}

void CpocketDlg::Detect(int selIdx)
{
	
	CString filename;
	m_ImgListCtl.GetText(selIdx, filename);

	CT2A pps(filename);
	char* file = pps.m_psz;
	
	char *res_img_file = action(file);
	CString tmp(res_img_file);
	DrawImg(tmp);
}

void CpocketDlg::OnBnClickedBtnAddFolder()
{
	CFolderPickerDialog *dlg = new CFolderPickerDialog;

	if (dlg->DoModal() == IDOK) {
		CString folderPath = dlg->GetPathName();

		if (folderPath.Right(1) != "\\")
			folderPath += "\\";

		folderPath += "*.txt";

		CFileFind search;
		BOOL bFound = search.FindFile(folderPath);

		while (bFound) {
			bFound = search.FindNextFile();

			if (search.IsDots())
				continue;

			if (search.IsDirectory())
				continue;

// 			if (!DrawImg(search.GetFilePath()))
// 				continue;

			m_ImgListCtl.AddString(search.GetFilePath());
			m_ImgListCtl.SetCurSel(m_ImgListCtl.GetCount() - 1);
		}
	}

	return;
}
