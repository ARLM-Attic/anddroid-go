
#include "relationwidget.h"
#include "Container.h"
#include "rectop.h"
#include "DisplayManage.h"

CRelationWidget::CRelationWidget(void)
{
	m_widgetType=WIDGET_TYPE_RELATION;
	m_pSrcWidget=0;
	m_pTargetWidget=0;
	ptSrcOld.x=ptTargetOld.x=-1;
	ptSrcOld.y=ptTargetOld.y=-1;
	m_bAutoHook=true;
}

CRelationWidget::~CRelationWidget(void)
{
}

int CRelationWidget::GetConnectPoint(int x_src, int y_src, int x_tar, int y_tar)
{
	WRect rc_src;
	WRect rc_tar;
	WPoint pt_c_src;
	WPoint pt_c_tar;
	this->m_pSrcWidget->GetRect(&rc_src);
	this->m_pTargetWidget->GetRect(&rc_tar);
	CENTER(pt_c_src,rc_src);
	CENTER(pt_c_tar,rc_tar);

    return 0;
}


int CRelationWidget::HandleEvent(int evCode, int param1, int param2)
{
#if(0)
	if(IS_MOUSE_EVENT(evCode)==true)
	{
		//Means relationship have created
		//we need not take anymore
		if(m_pSrcWidget!=0 && m_pTargetWidget!=0)
		{
			return false;
		}

	}
	
	if(evCode==MOUSE_LBUTTON_DOWN)
	{
		WPoint pt;
		pt=*((WPoint*)param1);
		this->m_pSrcWidget=FindWidgetByPoint(&pt);
		if(m_pSrcWidget!=0)
		{
			ptSrcOld.x=ptTargetOld.x=0;
			ptSrcOld.y=ptTargetOld.y=0;
			return true;
		}
	}


	if(evCode==MOUSE_MOVE)
	{

		WPoint ptSrc;
		if(this->m_pSrcWidget!=0)
		{
			CENTER(ptSrc,m_pSrcWidget->m_rect);
			ptSrc.x-=m_pSrcWidget->m_rect.x;
			ptSrc.y-=m_pSrcWidget->m_rect.y;

			m_pSrcWidget->ClientToScreen(&(ptSrc.x),&(ptSrc.y));

			WPoint pt;
			pt=*((WPoint*)param1);
			pt.x-=m_rect.x;
			pt.y-=m_rect.y;

			this->ClientToScreen(&(pt.x),&(pt.y));

			CCanvas* pCanvas;
	
			pCanvas=m_pContainer->GetCanvas();
	
			if(pCanvas!=0)
			{
		

				if(!(ptSrcOld.x==ptTargetOld.x &&
					ptSrcOld.y==ptTargetOld.y ))
				{

					//xor draw to remove the old line
					pCanvas->DrawLine(ptSrcOld.x,ptSrcOld.y,ptTargetOld.x,ptTargetOld.y);
				
				}
				pCanvas->DrawLine(ptSrc.x,ptSrc.y,pt.x,pt.y);

			
				ptSrcOld=ptSrc;
				ptTargetOld=pt;
			}

			return true;

		}


	}


	if(evCode==MOUSE_LBUTTON_UP)
	{
		
		CCanvas* pCanvas;
		pCanvas=m_pContainer->GetCanvas();
		WPoint pt;
		pt=*((WPoint*)param1);
		this->m_pTargetWidget=FindWidgetByPoint(&pt);

		if(m_pSrcWidget!=0 && m_pTargetWidget!=0)
		{

			m_pSrcWidget->m_pRelationWidget=this;
			m_pTargetWidget->m_pRelationWidget=this;
			if(pCanvas!=0)
			{
				WPoint ptSrc;
				WPoint ptTar;
				CENTER(ptSrc,m_pSrcWidget->m_rect);
				ptSrc.x-=m_pSrcWidget->m_rect.x;
				ptSrc.y-=m_pSrcWidget->m_rect.y;

				m_pSrcWidget->ClientToScreen(&(ptSrc.x),&(ptSrc.y));

				CENTER(ptTar,m_pTargetWidget->m_rect);
				ptTar.x-=m_pTargetWidget->m_rect.x;
				ptTar.y-=m_pTargetWidget->m_rect.y;

				m_pTargetWidget->ClientToScreen(&(ptTar.x),&(ptTar.y));


				pCanvas->DrawLine(ptSrcOld.x,ptSrcOld.y,ptTargetOld.x,ptTargetOld.y);

				pCanvas->DrawLine(ptSrc.x,ptSrc.y,ptTar.x,ptTar.y);

			}
		//	m_pSrcWidget=0;
		//	m_pTargetWidget=0;
			return true;
		}

		if(m_pSrcWidget==0)
		{
			m_pSrcWidget=0;
			m_pTargetWidget=0;
			return false;
		}

		if(m_pTargetWidget==0)
		{
			m_pSrcWidget=0;
			m_pTargetWidget=0;
			if(pCanvas!=0)
			{

			}
			return true;
		}		
	}
#endif
	return false;
}

int CRelationWidget::Draw(CCanvas* pCanvas, int screen_x, int screen_y)
{
#if(0)
		if(m_pSrcWidget!=0 && m_pTargetWidget!=0)
		{

			if(pCanvas!=0)
			{
				WPoint ptSrc;
				WPoint ptTar;
				CENTER(ptSrc,m_pSrcWidget->m_rect);
				ptSrc.x-=m_pSrcWidget->m_rect.x;
				ptSrc.y-=m_pSrcWidget->m_rect.y;

				m_pSrcWidget->ClientToScreen(&(ptSrc.x),&(ptSrc.y));

				CENTER(ptTar,m_pTargetWidget->m_rect);
				ptTar.x-=m_pTargetWidget->m_rect.x;
				ptTar.y-=m_pTargetWidget->m_rect.y;

				m_pTargetWidget->ClientToScreen(&(ptTar.x),&(ptTar.y));

			

				
				
			//	pCanvas->set_draw_mode(MODE_XOR);

		

				
			//	pCanvas->DrawLine(ptSrcOld.x,ptSrcOld.y,ptTargetOld.x,ptTargetOld.y,this->m_strID );
				pCanvas->DrawLine(ptSrc.x,ptSrc.y,ptTar.x,ptTar.y,this->m_strID);

			
			//	pCanvas->restore_draw_mode();
				

				ptSrcOld=ptSrc;
				ptTargetOld=ptTar;


			}		
		}
#endif
	return 0;
}
CWidget* CRelationWidget::FindWidgetByPoint(WPoint* pt)
{
	    CWidget* pWidget=this->m_pContainer->m_pWidgetTail ;
		while(pWidget!=0)
		{

			WRect rc;
			pWidget->GetRect(&rc);
			if(pWidget->GetWidgetType()==WIDGET_TYPE_GENERAL)
			{
				if(CRectOp::IsPtIn(pt,&rc)==true)
				{
					if(pWidget!=this->m_pSrcWidget&& pWidget!=this->m_pTargetWidget)
					{
						return pWidget;
					}
				}
			}
			pWidget=pWidget->pPrev;
			
		}	
	return 0;
}
