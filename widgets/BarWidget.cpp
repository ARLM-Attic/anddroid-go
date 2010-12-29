#include "animate.h"
#include "BarWidget.h"
#include "GhostWidget.h"
CBarWidget::CBarWidget(void)
{
		m_nItem=0;		
		m_nSeparateDx=10;
		m_nOffsetTop=10;
		m_bEnableGhost=true;
		m_pFocusItem=NULL;
		
}

CBarWidget::~CBarWidget(void)
{
	
}

int CBarWidget::CustDraw(CFrameSurface* pSurface)
{
	return CContainerWidget::CustDraw(pSurface);
}



bool CBarWidget::AddItem(CWidget* pItem,int nIndex)
{
		WRect rc;
		WRect rcList;
		m_listContainer.GetRect(&rcList);
		pItem->GetRect(&rc);
		CWidget* pAfterWidget = m_listContainer.InsertWidget(nIndex,pItem);
		if(pAfterWidget==NULL)
		{
			return FALSE;
		}
		CWidget* pPrevWidget;
		pPrevWidget=pItem->pPrev;

		if(pPrevWidget!=NULL)
		{
			WRect rcPrev;
			pPrevWidget->GetRect(&rcPrev);
			rc.x=rcPrev.x+rcPrev.dx+m_nSeparateDx;
			rc.y=rcPrev.y;
			rc.dy=rcPrev.dy;
		}
		else
		{
			rc.x=m_nSeparateDx;
			rc.y=m_nOffsetTop;			
		}
		pItem->SetRect(&rc);
		//如果Item超出listContainer的边界，扩大listContainer的边界
		if(rcList.dx<rc.x+rc.dx)
		{
			rcList.dx=rc.x+rc.dx;
			m_listContainer.SetRect(&rcList);
		}
		m_nItem++;


		itemList.add(pItem);
		if(this->m_bEnableGhost==true)
		{
			CGhostWidget* ghost=new CGhostWidget();
			m_listContainer.GetRectAsScreen(&rcList);
			int nMirrorPos=rc.y+rc.dy+1;
			ghost->SetMirrorPos(nMirrorPos);
			ghost->AttachWidget(pItem);
			this->m_listContainer.InsertWidget(0,ghost);

			ghost->m_bContinueWhileOutView=true;
			pItem->m_bContinueWhileOutView = true;
		}


		return TRUE;		
}
void CBarWidget::HandleModelEvent(ModelEvent* pEvent)
{
		CBarWidget* me = this;
		switch (pEvent->evCode)
		{
		case WIDGET_EVENT_ANIMATE_THREAD_FINISH:
			{
				EVENT_ANIMATE_THREAD_FINISH_T* pEvt = (EVENT_ANIMATE_THREAD_FINISH_T*)(pEvent->dwParam);

			
				
				break;
			}
		}
}


bool CBarWidget::HandleMouseEvent(int evCode, WPoint* ptScreen, int param2)
{

	WPoint ptMouse;
	ptMouse=*ptScreen;

	if(evCode==MOUSE_LBUTTON_UP)
	{
		this->m_pLastItem=m_pSelectedItem;
		m_pSelectedItem=this->m_pFocusItem;
		if(m_pLastItem!=this->m_pSelectedItem)
		{
			this->HandleItemSelected(m_pLastItem,this->m_pSelectedItem);
		}
	}

	if(evCode==MOUSE_MOVE)
	{
		CWidget* pOld=this->m_pFocusItem;
		this->m_pFocusItem=this->m_listContainer.GetWidget(&ptMouse);

	
		

		if(m_pFocusItem!=NULL && pOld!=this->m_pFocusItem )
		{
			if(this->m_pFocusItem->IsGhost()==true)
			{
				this->m_pFocusItem=NULL;
			}else
			{
					if(this->m_pFocusItem->IsAnimating()==false)
					{
					

						CAnimateThread* pThread = CreateNewAnimateThread(this->GetEventListener());
						CSinMoveWidgetAnimateParam* move_param=new CSinMoveWidgetAnimateParam();


						
						move_param->pWidget=m_pFocusItem;
						move_param->fEndAngle=180;
						move_param->fExtent=m_nOffsetTop;
						move_param->fStartAngle=0;
						move_param->nMoveFlag ='Y';

						move_param->nAcceler=0;
						move_param->nFadeOption=0;
						move_param->nSpeed=6;
						move_param->pSurface=NULL;
						move_param->pThread=pThread;

						
						if(AddNewFrame(move_param)==true)
						{
							StartThread(pThread);
						}
					
					}

			}
		}



	

	}
	return true;
}