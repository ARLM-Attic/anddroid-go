/*
	2007-12-4 List Widget Implement 
*/
#include "listwidget.h"
#include "animate.h"

bool CListWidget::Scroll(int dx,int dy)
	{

		WRect listViewPort;
		this->m_listContainer.GetViewPort(&listViewPort);
		listViewPort.x+=dx;
		listViewPort.y+=dy;
		if(this->m_listContainer.SetViewPort(&listViewPort)==false)
		{
			return false;
		}
		this->InvalidateContent();
		return true;
	}





	 bool CListWidget::IsContinueWhileOutView()
	{
		return false;
	}
	 int CListWidget::SetRect(WRect* pRect,bool bNoDraw){
		CContainerWidget::SetRect(pRect,bNoDraw);
		WRect rcList;
		CLONE_RECT(pRect,&rcList);
		rcList.x=0;
		rcList.y=0;
		m_listContainer.SetRect(&rcList);
		m_listContainer.SetViewPort(&rcList);
		m_bkWidget.SetRect(&rcList);
		
		m_bkWidget.SetBackgroundColor(m_clrBackGround);
		return 0;
	}
	void CListWidget::HandleModelEvent(ModelEvent *pEvent)
	{
		CListWidget* me = this;
		switch (pEvent->evCode)
		{
		case WIDGET_EVENT_ANIMATE_THREAD_FINISH:
			{
				EVENT_ANIMATE_THREAD_FINISH_T* pEvt = (EVENT_ANIMATE_THREAD_FINISH_T*)(pEvent->dwParam);

				/*
				CAnimateParam* pAnimateParam=(CAnimateParam*)(pEvt->pAnimateParam);

				if(pAnimateParam->type==ANIMATE_MOVE)
				{
					CMoveAnimateParam* pMove=(CMoveAnimateParam*)pAnimateParam;
					CWidget* pWidget = (CWidget*)(pMove->pWidget);
					if(pWidget!=NULL)
					{
						WRect rc;
						pWidget->GetRect(&rc);
						rc.x=pMove->x2;
						rc.y=pMove->y2;
						CWidget* pContainer=pWidget->GetContainer();
						ASSERT(pContainer!=NULL);
						pContainer->ScreenToClient(&rc);
						pWidget->SetRect(&rc);
						pWidget->SetVisible(true);
						pWidget->InvalidateContent(0);
					}
				//	free(pMove);
				}
				*/
				
				break;
			}
		}
	}
	 BOOL CListWidget::RemoveItem(CWidget* pItem)
	{
		if(pItem==NULL) return false;
		CWidget* pNext = pItem->pNext;
		WRect rc;
		pItem->GetRectAsScreen(&rc);
		
		/*获取背景*/
		if(m_pBkSurface==NULL)
		{
			m_pBkSurface=this->m_bkWidget.CreateMemoryFrameSurface();		
		}
		WRect rcBk;
		m_pBkSurface->GetRect(&rcBk);
		CMemoryFrameSurface* pSurface = pItem->CreateMemoryFrameSurface();
		CAnimateThread* pThread = CreateNewAnimateThread(this->GetEventListener());

		//设置背景
		pSurface->AddBackGround(&rcBk,m_pBkSurface->m_pSurfaceMemBitmap);		
		CMoveAnimateParam* move_param=new CMoveAnimateParam();
		move_param->x1=rc.x;
		move_param->y1=rc.y;
		move_param->x2=rc.x+rc.dx;
		move_param->y2=rc.y;
		move_param->nAcceler=10;
		move_param->nFadeOption=0;
		move_param->nSpeed=1;
		move_param->pSurface=pSurface;
		move_param->pThread=pThread;
		move_param->pWidget=NULL;
		AddNewFrame(move_param);
		
		
		m_listContainer.RemoveWidget(pItem);

		StartThread(pThread);

		while(pNext!=NULL)
		{
			WRect rcM;
			pNext->GetRectAsScreen(&rcM);

			CMemoryFrameSurface* pSurface = pNext->CreateMemoryFrameSurface();
			pSurface->AddBackGround(&rcBk,m_pBkSurface->m_pSurfaceMemBitmap);
			CAnimateThread* pThread = CreateNewAnimateThread(this->GetEventListener());

			CTickTickAnimateParam* tick_param=new CTickTickAnimateParam();
			tick_param->nTicks=10;
			tick_param->pSurface=NULL;
			tick_param->pThread=pThread;
			tick_param->pWidget=pNext;
			
			AddNewFrame(tick_param);

			CMoveAnimateParam* move_param= new CMoveAnimateParam();
			move_param->x1=rcM.x;
			move_param->y1=rcM.y;
			move_param->x2=rcM.x;
			move_param->y2=rcM.y-rc.dy-SEPERATE_LINE_DY;
			move_param->nAcceler=0.2;
			move_param->nFadeOption=0;
			move_param->nSpeed=1;
			move_param->pSurface=pSurface;
			move_param->pThread=pThread;
			move_param->pWidget=pNext;
			AddNewFrame(move_param);

			StartThread(pThread);
			pNext=pNext->pNext;
		}
		
		return TRUE;
	}

	 BOOL CListWidget::RemoveItem(int nIndex)
	{
		CWidget* pW=m_listContainer.GetWidget(nIndex);
		if(pW==NULL) return FALSE;
		RemoveItem(pW);
		return TRUE;
	}

	 BOOL CListWidget::AddItem(CWidget* pItem,int nIndex)
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
			rc.x=rcPrev.x;
			rc.y=rcPrev.y+rcPrev.dy+SEPERATE_LINE_DY;
			rc.dx=rcPrev.dx;
		}
		else
		{
			rc.x=0;
			rc.y=0;
			rc.dx=rcList.dx;
		}
		pItem->SetRect(&rc);
		//如果Item超出listContainer的边界，扩大listContainer的边界
		if(rcList.dy<rc.y+rc.dy)
		{
			rcList.dy=rc.y+rc.dy;
			m_listContainer.SetRect(&rcList);
		}
		m_nItem++;
		return TRUE;
	}

	 CWidget* CListWidget::GetItem(WPoint* pt)
	{
		WPoint ptTest=*pt;
		return m_listContainer.GetWidget(&ptTest);
		
	}

	void CListWidget::DockFloatWidget()
	{
		this->m_listContainer.RemoveWidget(this->m_pFloatItem);

		///Animate Docking the last widget
		if(this->m_pLastUnderWidget!=NULL)
		{
				WRect rcFloat;
				WRect rcCur;
				m_pFloatItem->GetRect(&rcFloat);
				int dock_x,dock_y;
				this->m_pLastUnderWidget->GetRect(&rcCur);
				GetDockPosition(m_pLastUnderWidget,dock_x,dock_y);

				
				if(rcCur.y>rcFloat.y)
				{
					dock_y+=rcFloat.dy;
				}
				m_pLastUnderWidget->GetRect(&rcFloat);

				if(rcFloat.y==dock_y)
				{
					return;
				}
				WRect rcDock;
				rcDock=rcFloat;
				rcDock.y=dock_y;
				
				//m_pLastUnderWidget->SetRect(&rcDock);
				
				if(this->m_bAnimateEffect == true)
				{
			
					CAnimateThread* pThread = CreateNewAnimateThread(this->GetEventListener());					
					pThread = CreateNewAnimateThread(this->GetEventListener());
					CLineMoveWidgetAnimateParam* move_param= new CLineMoveWidgetAnimateParam();
						
					move_param->x1=rcFloat.x;
					move_param->y1=rcFloat.y;
					move_param->x2=rcFloat.x;
					move_param->y2=dock_y;

					this->m_listContainer.ClientToScreen(&(move_param->x1),&(move_param->y1));
					this->m_listContainer.ClientToScreen(&(move_param->x2),&(move_param->y2));
					move_param->nAcceler=0.2;
					move_param->nFadeOption=0;
					move_param->nSpeed=1;
					move_param->pSurface=NULL;
					move_param->pThread=pThread;
					move_param->pWidget=m_pLastUnderWidget;
					AddNewFrame(move_param);

					StartThread(pThread);
				}
		}


		WRect rcFloat;
		m_pFloatItem->GetRect(&rcFloat);

		CWidget* pWidget=this->m_listContainer.m_pWidgetHead  ;
		WRect rc;
		int idx=0;
		bool bFound=false;
		while(pWidget!=0)
		{
				
			pWidget->GetRect(&rc);

			if(rcFloat.y<=rc.y)
			{
				bFound=true;
				this->m_listContainer.InsertWidget((idx),m_pFloatItem);
				break;
			}
			
			pWidget=pWidget->pNext  ;
			idx++;			
		}
		if(bFound==false)
		//dock to tail
		{
			this->m_listContainer.InsertWidget((-1),m_pFloatItem);
		}

		////Animate docking
		{
				int dock_x,dock_y;
				this->GetDockPosition(m_pFloatItem,dock_x,dock_y);
				WRect rcDock;
				rcDock=rcFloat;
				rcDock.y=dock_y;
				
				//m_pFloatItem->SetRect(&rcDock);
				

				if(this->m_bAnimateEffect == true)
				{
					CAnimateThread* pThread = CreateNewAnimateThread(this->GetEventListener());					
					pThread = CreateNewAnimateThread(this->GetEventListener());
					CLineMoveWidgetAnimateParam* move_param= new CLineMoveWidgetAnimateParam();
						
					move_param->x1=rcFloat.x;
					move_param->y1=rcFloat.y;
					move_param->x2=rcFloat.x;
					move_param->y2=dock_y;

					this->m_listContainer.ClientToScreen(&(move_param->x1),&(move_param->y1));
					this->m_listContainer.ClientToScreen(&(move_param->x2),&(move_param->y2));
					move_param->nAcceler=1;
					move_param->nFadeOption=0;
					move_param->nSpeed=1;
					move_param->pSurface=NULL;
					move_param->pThread=pThread;
					move_param->pWidget=m_pFloatItem;
					AddNewFrame(move_param);

					StartThread(pThread);
				}
				else
				{
					this->m_pFloatItem->SetAlpha(255);
					InvalidateContent();
				}
		}



	}

	void CListWidget::GetDockPosition(CWidget* pW,int& x,int& y)
	{
		if(pW->pPrev!=0)
		{
			WRect rc;
			pW->pPrev->GetRect(&rc);
			int dock_y,dock_x;
            
			rc.y+=(rc.dy+SEPERATE_LINE_DY);
			WRect rcMe;
			pW->GetRect(&rcMe);
			y=rc.y;
			x=rc.x;
			

		}
		else
		{
			WRect rcMe;
			pW->GetRect(&rcMe);
			y=0;
			x=rcMe.x;
			
		}
	}
	
	void CListWidget::HandleFloatItemMove(WPoint& pt)
	{
				int dy=pt.y-this->m_ptMouseOld.y;					
				WRect rcItem;
				WRect rcOld;
				m_pFloatItem->GetRect(&rcItem);
				rcOld=rcItem;
				rcItem.y+=dy;

				if(rcItem.y<=0)
				{
					rcItem.y=0;
					dy=rcItem.y-rcOld.y;
				}

				
				
				WRect rcInvalid;
				UnionRect(&rcInvalid,&rcItem,&rcOld);

				WPoint ptTestTop,ptTestBottom;			
				ptTestBottom.x=rcOld.x;
				ptTestBottom.y=rcOld.y+dy+rcOld.dy;
	
				ptTestTop.x=rcOld.x;
				ptTestTop.y=rcOld.y+dy;

				m_listContainer.ClientToScreen(&(ptTestTop.x),&(ptTestTop.y));
				m_listContainer.ClientToScreen(&(ptTestBottom.x),&(ptTestBottom.y));

				CWidget* pUnderWidgetTop=NULL;
				CWidget* pUnderWidgetBot=NULL;

				pUnderWidgetTop=m_listContainer.GetWidget(&ptTestTop,m_pFloatItem);
				pUnderWidgetBot=m_listContainer.GetWidget(&ptTestBottom,m_pFloatItem);


				//Move the float Item
				m_pFloatItem->SetRect(&rcItem);

				//Move Under Widget
				WRect rcUnder;
				CWidget* pUnderWidget=pUnderWidgetTop;
				if(pUnderWidgetBot!=NULL)
				{
					pUnderWidget=pUnderWidgetBot;

				}

				int dock_x,dock_y;
				if(this->m_pLastUnderWidget!=NULL && m_pLastUnderWidget!=pUnderWidget )
				{
					WRect rc;
					m_pLastUnderWidget->GetRect(&rc);
					GetDockPosition(m_pLastUnderWidget,dock_x,dock_y);

					if(dock_y>rcItem.y)
					{
						dock_y+=rcItem.dy;
					}

					if(rc.y!=dock_y)
					{
						rc.y=dock_y;
						m_pLastUnderWidget->SetRect(&rc);
						WRect rcInvalid1=rcInvalid;
						UnionRect(&rcInvalid,&rcInvalid1,&rc);
					}
				}

				if(pUnderWidget!=NULL)
				{
					pUnderWidget->GetRect(&rcUnder);
					WRect rcOld = rcUnder;
					rcUnder.y-=dy;


					/*
					GetDockPosition(pUnderWidget,dock_x,dock_y);
					if(dy>0 && rcUnder.y<dock_y)
					{
						rcUnder.y=dock_y;
					}
					if(dy<0)
					{
						dock_y+=rcItem.dy;
						if(rcUnder.y>dock_y)
							rcUnder.y=dock_y;
					}
					*/
					pUnderWidget->SetRect(&rcUnder);
					WRect rcInvalid2;
					UnionRect(&rcInvalid2,&rcOld,&rcUnder);
					WRect rcInvalid1=rcInvalid;
					UnionRect(&rcInvalid,&rcInvalid1,&rcInvalid2);
				}
				//this->m_listContainer.InvalidateClientArea(&rcInvalid);
				this->m_listContainer.InvalidateContent(0);
				m_pLastUnderWidget=pUnderWidget;

				//TRACE(" update (y=%d,dy=%d)\n",rcInvalid.y,rcInvalid.dy);
	}
	

	 CWidget* CListWidget::GetSelectedItem()
	{
		return this->m_pSelectedItem;
	}
	 void CListWidget::HandleItemSelected(CWidget* pItem)
	{
		if(m_pLastSelectedItem!=m_pSelectedItem && m_pLastSelectedItem!=NULL)
		{						


		}
		//m_pSelectedItem->SetBackgroundColor(0xFF0000);
		//m_pSelectedItem->InvalidateContent(0);
		m_pLastSelectedItem=m_pSelectedItem;		

	}
	 bool CListWidget::HandleKeyDown(unsigned int nChar,unsigned int nRepCnt,unsigned int nFlags)
	{
		return true;
	}

	 bool CListWidget::HandleMouseEvent(int evCode, WPoint* ptScreen, int param2)
	{

		WPoint ptMouse;
		ptMouse=*ptScreen;

		if(evCode==MOUSE_LBUTTON_UP)
		{

			this->m_pSelectedItem=this->m_listContainer.GetWidget(&ptMouse);
			if(m_pSelectedItem!=NULL)
			{
				HandleItemSelected(m_pSelectedItem);
			}
			if(m_pFloatItem!=NULL)
			{
				DockFloatWidget();
				m_pFloatItem->SetAlpha(255);
				m_pLastUnderWidget=NULL;
			}
			m_pFloatItem=NULL;
			if(this->m_bMouseLButtonHold==true)
			{
				this->m_bMouseLButtonHold=false;
				return false;
			}
		}

		if(evCode==MOUSE_LBUTTON_DOWN)
		{

			if(this->m_bMouseLButtonHold==true)
			{
				this->m_bMouseLButtonHold=false;
				return false;
			}
			else
			{
				m_bMouseLButtonHold=true;
				this->m_ptMouseOld = ptMouse;

			}
		}

		if(evCode==MOUSE_MOVE)
		{
			
			if(m_bMouseLButtonHold==true && this->m_bEnableFloatItem ==true)
			{
				if(this->m_pFloatItem==NULL)
				{
					
					m_pFloatItem=GetItem(&ptMouse);
					if(m_pFloatItem!=NULL)
					{
						//将FloatItem设置为透明(alpha=FLOAT_ITEM_ALPHA)
						m_pFloatItem->SetAlpha(FLOAT_ITEM_ALPHA);
						//先从容器中将FloatItem删除，然后再将其以Top Most Order添加到容器中
						this->m_listContainer.RemoveWidget(this->m_pFloatItem);
						this->m_listContainer.InsertWidget(this->m_pFloatItem,Z_ORDER_TOPMOST);
					}
				}
				if(m_pFloatItem!=NULL)
				{									
					if(m_maskWidget.m_bVisible==true)
					{
						this->m_maskWidget.SetVisible(false);
						this->m_maskWidget.InvalidateContent(0);
					}
					HandleFloatItemMove(ptMouse);											
				}
				
			}
			if(m_bMouseLButtonHold==false)
			{
				CWidget* pOverWidget=this->m_listContainer.GetWidget(&ptMouse);
				CWidget* pOld=this->m_maskWidget.GetMaskedWidget();
				if(pOverWidget!=pOld)
				{
					this->m_maskWidget.Mask(pOverWidget,MASK_ALPHA);
					if(pOld!=NULL)
					{
						pOld->InvalidateContent(0);
					}
				}
				

				

			}


			

		}
		this->m_ptMouseOld = ptMouse;
	}