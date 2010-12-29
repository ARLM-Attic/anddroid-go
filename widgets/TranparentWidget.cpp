/*************************************************************************
Easy-Go Server 
Author: Sureone Yang
Email: EasyGo_Owner@yahoo.com
------------------------------------------
Revision History:
2007-12-10			
	V0.1 BaseLine
2007-12-11			
	1.  Add a new feature for supporting transparent wiget

*************************************************************************/
#include "tranparentwidget.h"

CTransparentWidget::CTransparentWidget(void)
{
	m_nAlphaValue=255;
}

CTransparentWidget::~CTransparentWidget(void)
{
}
void CTransparentWidget::SetAlphaValue(int v)
{
	m_nAlphaValue=v;
}
int CTransparentWidget::Draw(CCanvas* pCanvas, int screen_x, int screen_y)
{
	WRect rc;
	m_pCanvas=pCanvas;
	WRect rcScreen;

	{
			GetRect(&rc);
			rc.x=0;
			rc.y=0;
			ClientToScreen(&rc);
			rcScreen=rc;
		}
	// For transparent widget we cann't use cache, before the background maybe changed
/*
	if(m_bContentUpdated )
	{
*/
		if(this->m_pGhostWidget!=0)
		{
			this->m_pGhostWidget->CustDraw(pCanvas,screen_x,screen_y);
		}
		
		if(this->m_pMemBmp!=0)
		{
			delete m_pMemBmp;
			m_pMemBmp=0;
		}
		
		this->m_pMemBmp = pCanvas->StartTransparentDraw(&rcScreen);
		CustDraw(pCanvas,screen_x,screen_y);
		pCanvas->FinishTransparentDraw();
		pCanvas->AlphaBitBlt(this->m_pMemBmp,&rcScreen,m_nAlphaValue);

		
		m_bContentUpdated=false;
/*
	}
	//not need redraw, exampel, we just move the widget form one place to another.
	else
	{
		GetRect(&rc);
		rc.x=0;
		rc.y=0;
		ClientToScreen(&rc);
		rcScreen=rc;

		WRect rcClip;
		pCanvas->GetClipRect(&rcClip);

//		m_pMemBmp->m_rect=rcScreen;
		pCanvas->AlphaBitBlt(this->m_pMemBmp,&rcClip,m_nAlphaValue);
	}



*/

	
	return 0;
}