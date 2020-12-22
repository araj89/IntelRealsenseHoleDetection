#pragma once
#include "ximage\ximage.h"


// CImageWnd

class CImageWnd : public CWnd
{
	DECLARE_DYNAMIC(CImageWnd)

public:
	CImageWnd();
	virtual ~CImageWnd();

	enum { IDD = IDD_IMG_WND };

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();

	void				DrawContent();
	BOOL				DrawImg(CString filename);
	CRect				GetValidRect(CRect rtDst, double dRateHW);

	HWND				m_hParentWnd;

	CRect				m_Rect;
	CDC					m_dc;
	CDC					m_dcMemory;

	CBitmap				m_Back;
	CxImage				m_CxImg;

	BOOL				m_bStart;
	BOOL				m_bAllocated;
};