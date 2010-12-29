
#include "RectOp.h"
#include ".\appledockbarcontainerwidget.h"
#include "GhostWidget.h"
#define	LEFT_OFFSET		30
#define TOP_OFFSET			30
#define BOTTOM_OFFSET	30
#define RIGHT_OFFSET		30

#define FOCUS_OFFSET	30

static int last_x=0;

static void animate_listener_cb(void *pUserData, ModelEvent *pEvent)
{
	CWidget* pfWidget=(CWidget*)(pEvent->dwParam);
	CAppleDockBarContainerWidget* pContainer=(CAppleDockBarContainerWidget*)pUserData;
	if(pfWidget->m_nAnimateFlag==ANIMATE_MOVE)
	{
		//have update
		pfWidget->m_nAnimateFlag=ANIMATE_UPDATED;
	}else
	{
		//no update
		pfWidget->m_nAnimateFlag=ANIMATE_CHECKED;

	}

	//go through all the child widgets, check if need refresh the client area;

	CWidget* pWidget=pContainer->m_pWidgetHead;

	bool bAnimateUpdated=false;
	while(pWidget!=0)
	{
		if(pWidget->m_nAnimateFlag==ANIMATE_NO_CHECK)
		{		
			return;
		}
		if(pWidget->m_nAnimateFlag==ANIMATE_UPDATED)
		{					
			bAnimateUpdated=true;
		}
	

		pWidget=pWidget->pNext;
		
	}

	if(bAnimateUpdated==true)
	{
		pContainer->InvalidateContent(0);

		//Reset the flag
		CWidget* pWidget=pContainer->m_pWidgetHead;


		while(pWidget!=0)
		{
			pWidget->m_nAnimateFlag=ANIMATE_NO_CHECK;
		

			pWidget=pWidget->pNext;
			
		}
	}
}

CAppleDockBarContainerWidget::CAppleDockBarContainerWidget(void)
{
	this->animateListener.pfnListener=animate_listener_cb;
	this->animateListener.pListenerData = this;
	this->animateListener.pNext=0;
	this->animateListener.pPrev=0;
	this->animateListener.pCancelData=0;
	this->animateListener.pfnCancel=0;
	m_bUpdated=false;
}

CAppleDockBarContainerWidget::~CAppleDockBarContainerWidget(void)
{
}


int CAppleDockBarContainerWidget::InsertWidget(CWidget* pWidget,CWidget* pWidgetBefore)
{

	CContainerWidget::InsertWidget(pWidget,Z_ORDER_NORMAL);

	pWidget->AddAnimateListener(&(this->animateListener));

	WRect rcW;
	pWidget->GetRect(&rcW);

	int dx=this->m_nWidgtCount * rcW.dx+LEFT_OFFSET+RIGHT_OFFSET+FOCUS_OFFSET*4;
	int dy=rcW.dy*2+TOP_OFFSET+BOTTOM_OFFSET;

	m_rect.dx=dx;
	m_rect.dy=dy;

	m_nInitDx=rcW.dx;
	m_nInitDy=rcW.dy;

	this->SetRect(&m_rect,true);

	int index = GetWidgetIndex(pWidget);

	rcW.x=LEFT_OFFSET+rcW.dx*index;
	rcW.y=TOP_OFFSET;

	pWidget->SetRect(&rcW,true);

	CGhostWidget* ghost = new CGhostWidget();
	pWidget->AttachGost(ghost);

	return 0;
}



int CAppleDockBarContainerWidget::HandleEvent(int evCode, int param1, int param2)
{

	
	if(IS_MOUSE_EVENT(evCode)==true)
	{
		WPoint pt;
		WPoint pt2;
		pt=*( (WPoint*)param1);
		pt2=pt;
		CRectOp::ConvertToClientCoord(this,&pt);

		WRect rcMe;
		GetRect(&rcMe);
		rcMe.x=0;
		rcMe.y=0;



			

		if(CRectOp::IsPtIn(&pt,&rcMe)==false)
		{
				if(m_bUpdated==true)
				{
					Restore();
					m_bUpdated=false;
				}


		
				return false;
		}

		if(abs(last_x-pt.x)<=2){
			last_x=pt.x;
			return true;
		}

		m_bUpdated=true;



		

		CWidget* pFocusWidget=0;

		CWidget* pWidget=this->m_pWidgetHead;

		float range=m_nInitDx*5;
		float fx=pt.x;
		float ff=3.1415926/range;
		float p=3.1415926/2-fx*ff;
		float fdy=0;
		float fa=FOCUS_OFFSET;
		
		
		int idx=0;
		int prev_right=0;
		while(pWidget!=0)
		{
			WRect rc;
			pWidget->GetRect(&rc);

			WPoint cPt;
			cPt.x=rc.x+rc.dx/2;
			cPt.y=rc.y+rc.dy/2;

			float curFx=cPt.x;
			float absFx=abs(curFx-fx);
			fdy=0;

/*
			
			if((pt.y<rc.y || pt.y>rc.y+rc.dy))
			{
				float yy=abs(pt.y-cPt.y)-rc.dy/2;

				



				if(yy>=fa){

					fa=0;

					last_x=pt.x;
				//	return true;
				}else
				{
					curFx=asin((fa-yy)/fa)/ff-p;
				}				
			}
*/
			if(absFx<range/2)
			{
				//TRACE("(%d,%d),(%d,%d)\n",pt.x,pt.y,cPt.x,cPt.y);
				fdy=fa*sin(ff*curFx+p);
			}

			if(prev_right==0)
			{
				rc.x=LEFT_OFFSET;
			}
			else
			{
				rc.x=prev_right;

			}
			rc.dx=m_nInitDx+fdy;
			rc.dy=m_nInitDy+fdy;
			rc.y=TOP_OFFSET-fdy;
			prev_right=rc.x+rc.dx;
			

//			TRACE("offset=%d\n",prev_offset);
			
/*
			if(cPt.x>pt.x)
			{
				rc.x=LEFT_OFFSET+m_nInitDx*idx-fdy;
			}
*/
			if(CRectOp::IsPtIn(&pt,&rc)==true)
			{
				pFocusWidget=pWidget;

			}

			pWidget->SetRect(&rc,true);

			
			pWidget=pWidget->pNext;
			idx++;
		}

		

		while(pWidget!=0)
		{
			WRect rc;
			pWidget->GetRect(&rc);
			
			
			pWidget=pWidget->pNext;
		}

		if(evCode==MOUSE_LBUTTON_UP && pFocusWidget!=0)
		{
			this->eventModel.NotifyEvent(evCode,(int)(pFocusWidget));
		}

		this->InvalidateContent(0);

		last_x=pt.x;






		

	//	CWidget::HandleEvent(evCode,(int)(&pt2),param2);
	}

	
	
	return false;

}

int CAppleDockBarContainerWidget::Restore(void)
{
		CWidget* pWidget=this->m_pWidgetHead;

	
		int prev_offset=0;
		while(pWidget!=0)
		{
			WRect rc;
			pWidget->GetRect(&rc);

	
			rc.x+=prev_offset;
			prev_offset+=m_nInitDx-rc.dx;
			
			rc.dx=m_nInitDx;
			rc.dy=m_nInitDy;
			rc.y=TOP_OFFSET;

			pWidget->SetRect(&rc,true);

			
			pWidget=pWidget->pNext;
			
		}
		this->InvalidateContent(0);

	return 0;
}
