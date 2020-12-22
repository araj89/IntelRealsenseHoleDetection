
// pocketDlg.h : header file
//

#pragma once
#include "ImageWnd.h"
#include "afxwin.h"


// CpocketDlg dialog
class CpocketDlg : public CDialogEx
{
// Construction
public:
	CpocketDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_POCKET_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnAddFile();
	afx_msg void OnBnClickedBtnAddFolder();
	afx_msg void OnBnClickedBtnRemove();
	afx_msg void OnBnClickedBtnDetect();
	afx_msg void OnDblclkListImg();
	DECLARE_MESSAGE_MAP()

public:

	BOOL				WndInit();
	BOOL				DrawImg(CString filename);
	BOOL				DrawSelectedImg(int selIdx);
	void				Detect(int selIdx);

	CImageWnd			m_ImgWnd;
	CRect				m_ImgWndRect;
	
	CListBox			m_ImgListCtl;
	BOOL				m_bDetectStart;
	
};
