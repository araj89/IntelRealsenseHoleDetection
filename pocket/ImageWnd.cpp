// ImageWnd.cpp : implementation file
//

#include "stdafx.h"
#include "pocket.h"
#include "ImageWnd.h"


// CImageWnd

IMPLEMENT_DYNAMIC(CImageWnd, CWnd)

CImageWnd::CImageWnd()
{
	m_hParentWnd = NULL;

	m_bStart = TRUE;
	m_bAllocated = FALSE;
}

CImageWnd::~CImageWnd()
{
}


BEGIN_MESSAGE_MAP(CImageWnd, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CImageWnd message handlers




void CImageWnd::OnPaint()
{
	CPaintDC dc(this);

	GetClientRect(&m_Rect);

	if (m_bStart)
	{
		m_dc.CreateCompatibleDC(&dc);
		m_Back.CreateCompatibleBitmap(&dc, 1600, 900);
		m_dc.SelectObject(&m_Back);
		m_dcMemory.CreateCompatibleDC(&m_dc);

		m_bStart = FALSE;
	}

	DrawContent();

	dc.BitBlt(0, 0, m_Rect.Width(), m_Rect.Height(), &m_dc, 0, 0, SRCCOPY);
}

void CImageWnd::DrawContent()
{
	if (m_bAllocated) {
		m_dc.FillSolidRect(0, 0, m_Rect.Width(), m_Rect.Height(), RGB(20, 20, 20));
		m_CxImg.Draw(m_dc, GetValidRect(m_Rect, (double)m_CxImg.GetHeight() / m_CxImg.GetWidth()));
		return;
	}

	m_dc.FillSolidRect(0, 0, m_Rect.Width(), m_Rect.Height(), RGB(40, 40, 40));

#if 0
	CPen pen, pen1;
	if (1)
	{
		pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	}
	else {
		pen.CreatePen(PS_SOLID, 1, RGB(20, 20, 20));
	}

	pen1.CreatePen(PS_SOLID, 1, RGB(85, 85, 85));

	CPen *oldPen = m_dc.SelectObject(&pen);
	m_dc.MoveTo(0, 0);
	m_dc.LineTo(0, m_Rect.Height() - 1);
	m_dc.LineTo(m_Rect.Width() - 1, m_Rect.Height() - 1);
	m_dc.LineTo(m_Rect.Width() - 1, 0);
	m_dc.LineTo(0, 0);

	m_dc.SelectObject(&pen1);
	m_dc.MoveTo(1, 1);
	m_dc.LineTo(m_Rect.Width() - 1, 1);

	m_dc.SelectObject(&oldPen);
#endif

	int pos = 40;
	CString caption;
	caption.Format(_T("    No Image    "));
	m_dc.SetTextColor(RGB(0, 0, 0));
	m_dc.SetBkMode(TRANSPARENT);
	m_dc.DrawText(caption, CRect(m_Rect.Width() / 2 - pos, m_Rect.Height() / 2, m_Rect.Width(), m_Rect.Height()), NULL);
	m_dc.SetTextColor(RGB(160, 160, 160));
	m_dc.DrawText(caption, CRect(m_Rect.Width() / 2 - pos + 1, m_Rect.Height() / 2 + 1, m_Rect.Width(), m_Rect.Height()), NULL);
}

BOOL CImageWnd::DrawImg(CString filename)
{
	BOOL bSuccess =  TRUE;
	if (!m_CxImg.Load(filename, CXIMAGE_FORMAT_UNKNOWN))
		bSuccess = FALSE;
	
	m_bAllocated = bSuccess;
	Invalidate(FALSE);

	return bSuccess;
}

CRect CImageWnd::GetValidRect(CRect rtDst, double dRateHW)
{
	int ntempW = rtDst.Width();
	int ntempH = rtDst.Height();
	double dtRateHW = (double)ntempH / (double)ntempW;

	CRect rtSrc;
	if (dtRateHW >= dRateHW)
	{
		ntempW = rtDst.Width();
		ntempH = (int)ntempW * dRateHW;
		rtSrc = rtDst;

		double diff = rtDst.Height() - ntempH;
		rtSrc.top += (int)(diff / 2);		// adjust top position
		rtSrc.bottom -= (int)(diff / 2);	// adjust bottom position

	}
	else
	{
		ntempH = rtDst.Height();
		ntempW = (int)ntempH / dRateHW;
		rtSrc = rtDst;

		double diff = rtDst.Width() - ntempW;
		rtSrc.left += (int)(diff / 2);	// adjust top position
		rtSrc.right -= (int)(diff / 2);	// adjust bottom position
	}

	return rtSrc;
}