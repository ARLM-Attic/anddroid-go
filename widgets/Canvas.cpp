
#include "canvas.h"



CCanvas::CCanvas(void)
{
	m_pDC=0;

}

CCanvas::~CCanvas(void)
{
	if(this->m_pDC!=0)
	{
		delete m_pDC;
	}

}

int CCanvas::GetClipRect(CFrameSurface* pSurface, WRect* pRect)
{
	pSurface->GetClipRect(pRect);
	return 0;
}

int CCanvas::SetClipRect(CFrameSurface* pSurface, WRect* pRect)
{
	pSurface->SetClipRect(pRect);
	return 0;
}

int CCanvas::SetMyCDC(MyCDC* pDC)
{
	m_pDC=pDC;
	//pDC->SetROP2(R2_XORPEN );
	return 0;
}


int CCanvas::SetCDC(HWND hwnd)
{
	CDC* pDC=new CDC();
	pDC->Attach(GetWindowDC(hwnd));
	m_pDC=new MyCDC();
	 m_pDC->m_pScreen=pDC;
	
	return 0;
}

MyCDC* CCanvas::GetMyCDC(void)
{

	return m_pDC;
}

int CCanvas::DrawRectangle(CFrameSurface* pSurface, WRect* pRect,int color,char* widgetID)
{

	//pDC->Rectangle(pRect->x,pRect->y,pRect->x+pRect->dx,pRect->y+pRect->dy);
	pSurface->MoveTo(pRect->x,pRect->y);
	pSurface->LineTo(pRect->x,pRect->y+pRect->dy,color);
	pSurface->LineTo(pRect->x+pRect->dx,pRect->y+pRect->dy,color);
	pSurface->LineTo(pRect->x+pRect->dx,pRect->y,color);
	pSurface->LineTo(pRect->x,pRect->y,color);

	return 0;
}

int CCanvas::FillRect(CFrameSurface* pSurface, WRect* pRect, int color,char* widgetID)
{

	pSurface->FillSolidRect(pRect,color);

	return 0;
}



CFrameSurface* CCanvas::BeginPaint(WRect* pClipRect)
{
	return m_pDC->CreateFrameSurface(pClipRect);
}

int CCanvas::EndPaint(CFrameSurface* pSurface)
{
	
	m_pDC->EndPaint(pSurface);

	return 0;
}


int CCanvas::DrawLine(CFrameSurface* pSurface,int x1, int y1, int x2, int y2,char* widgetID)
{

	pSurface->MoveTo(x1,y1);
	pSurface->LineTo(x2,y2,0x555555);

	return 0;
}

CMemBitmap* CCanvas::NewMemBitmap(CFrameSurface* pSurface,int w,int h)
{

	return pSurface->NewMemBitmap(w,h);
	
}

CMemBitmap* CCanvas::GetMemBitmap(CFrameSurface* pSurface,WRect* pRect,int nAlpha)
{

	MyCDC* pDC=GetMyCDC();
	CMemBitmap* pMB = pSurface->GetMemBitmap(pRect);
	pMB->SetAlpha(nAlpha);
	return pMB;
}



void CCanvas::BitBlt(CFrameSurface* pSurface,CMemBitmap* pMemBmp,WRect* rcDraw)
{
	 pSurface->BitBlt(pMemBmp,rcDraw);
}

