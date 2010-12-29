#include "stdafx.h"
#include ".\ghostwidget.h"
#include "imagewidget.h"
#include "global_event_define.h"
CGhostWidget::CGhostWidget(void)
{
	this->m_pAttachedWidget=0;
	m_nDistanceWithReal=0;
	m_bMirror=false;
	m_bHori=true;
	m_bGhost=true;
	this->EnableBmpCache(true);
}

CGhostWidget::~CGhostWidget(void)
{

}
void CGhostWidget::AttachWidget(CWidget* pWidget)
{
	m_pAttachedWidget=pWidget;
	pWidget->GetEventModel()->AddListener(this->GetEventListener());
	pWidget->SetGhost(this);
	pWidget->AddGarbage(this);

	SetMyRect();
}
void CGhostWidget::SetMyRect()
{

		WRect rc;
		m_pAttachedWidget->GetRect(&rc);
		WRect rcOld;
		
		if(m_bMirror==false)
		{

	
			rc.y=rc.y+rc.dy+m_nDistanceWithReal;
			
		}
		else
		{
			if(m_bHori==true)
			{
				int offset_to_mirror=this->m_nMirrorPos-rc.y-rc.dy;
				rc.y=this->m_nMirrorPos+offset_to_mirror;
			
			}

		}
		this->GetRectAsScreen(&rcOld);
		this->SetRect(&rc);
		if(rcOld.dx==0)
		{
			return;
		}
		this->GetRectAsScreen(&rc);
		WRect rcU;
		UnionRect(&rcU,&rcOld,&rc);
		this->GetContainer()->InvalidateRectAsScreen(&rcU);
}
int CGhostWidget::CustDraw(CFrameSurface* pSurface)
{
	if(this->m_pAttachedWidget!=NULL)
	{		
		WRect rcScreen;
		this->GetRectAsScreen(&rcScreen);

		CMemoryFrameSurface* pMemSurface=this->m_pAttachedWidget->CreateMemoryFrameSurface();

		CMemBitmap* pBG=pSurface->CopySurface(&rcScreen);
		//pMemSurface->GetSurfaceMemBitmap()->m_pCDC->SelectObject((HGDIOBJ)NULL);
		CEgBitmap* pSrc=pMemSurface->GetSurfaceMemBitmap()->GetBMP();

		pSrc->DaoYin(pBG->GetBMP());

		//Since this memory surface is a copy of attached widget, we need
		//Move it to the actual paint position(ghost's position);
		pMemSurface->SetRect(&rcScreen);
		
		pSurface->DrawBmp(pMemSurface->GetSurfaceMemBitmap(),&rcScreen);

		delete pMemSurface;
		delete pBG;

	}
	return 0;
}
void CGhostWidget::HandleModelEvent(ModelEvent* pEvent)
{
	CWidget* pW=(CWidget*)(pEvent->dwParam );
	if(pEvent->evCode==WIDGET_EVENT_POSITION_CHANGED)
	{

		SetMyRect();

	}
}


CTiShengWidget::CTiShengWidget(void)
{
	this->m_pAttachedWidget=0;
}

CTiShengWidget::~CTiShengWidget()
{

}

void CTiShengWidget::AttachWidget(CWidget* pWidget)
{
	m_pAttachedWidget=pWidget;
	pWidget->GetEventModel()->AddListener(this->GetEventListener());
	pWidget->AddGarbage(this);
}