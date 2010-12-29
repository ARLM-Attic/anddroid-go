/*************************************************************************
eGUI/easyGUI Copyright (c) 2007-2008 Xiao Wang Yang 
Author: Xiao Wang Yang
Email: sureone@gmail.com   
Project Site: http://code.google.com/p/easygui
------------------------------------------
Revision History:
2008-01-01			
	C3DListWidget is a apple style picture list, implement picture widget rotate, move in 3D space. 
*************************************************************************/

#include "animate.h"
#include "3DListWidget.h"
#include "GhostWidget.h"
#include "CommonListWidget.h"
C3DListWidget::C3DListWidget(void)
{
		m_nItem=0;		
		m_nSeparateDx=6;
		m_nOffsetTop=10;
		m_bEnableGhost=true;
		m_pFocusItem=NULL;
		m_nItemAngle=40;		
		//Ralate to widget width
		this->m_nItemRotateOrigXRatio = 1;
		this->m_nItemViewZRatio = 3;
		m_nItemRotateOrigZRatio=0;
}

C3DListWidget::~C3DListWidget(void)
{
	
}

int C3DListWidget::CustDraw(CFrameSurface* pSurface)
{
	return CContainerWidget::CustDraw(pSurface);
}





bool C3DListWidget::HandleKeyDown(unsigned int nChar,unsigned int nRepCnt,unsigned int nFlags)
{
	return true;

}

void C3DListWidget::ReCalLayout(bool bInitial)
{
	WRect rc;
	CWidget* pWidget=m_listContainer.m_pWidgetHead;
	if(pWidget==NULL)
	{
		return;
	}
	pWidget->GetRect(&rc);

	WPoint pt_orig;
	float view_z=rc.dy*m_nItemViewZRatio;
	

		
	if(this->m_pFocusItem==NULL)
	{
		pWidget=m_listContainer.m_pWidgetHead;		
		while(pWidget!=NULL)
		{
			pWidget->m_fViewZ=view_z;
			pWidget->RotateY(&pt_orig,-this->m_nItemAngle );
			pWidget=pWidget->pNext;
		}
	}
	else
	{
		pt_orig.x=-rc.dx*this->m_nItemRotateOrigXRatio ;
		pt_orig.y=rc.dy/2;
		pt_orig.z=-rc.dy*this->m_nItemRotateOrigZRatio;
		//focus
		pWidget=this->m_pFocusItem;
		if(bInitial==true)
			pWidget->RotateY(&pt_orig,0);

		
		pWidget->m_z_order=Z_ORDER_TOPMOST;
		pWidget->m_fViewZ=view_z;

		Z_ORDER_T z_order = Z_ORDER_NORMAL;
		pWidget=pWidget->pPrev;
		//left side

		pt_orig.x=-rc.dx*this->m_nItemRotateOrigXRatio ;
		pt_orig.y=rc.dy/2;
		pt_orig.z=-rc.dy*this->m_nItemRotateOrigZRatio;

		while(pWidget!=NULL)
		{
			pWidget->m_fViewZ=view_z;
			pWidget->m_z_order = z_order--;
			if(pWidget!=m_pLastItem || bInitial==true)
				pWidget->RotateY(&pt_orig,-this->m_nItemAngle );
			pWidget=pWidget->pPrev;
		}

		pWidget=this->m_pFocusItem;
		pWidget=pWidget->pNext;
		z_order = Z_ORDER_NORMAL+1;

		pt_orig.x=rc.dx+rc.dx*this->m_nItemRotateOrigXRatio ;
		pt_orig.y=rc.dy/2;
		pt_orig.z=-rc.dy*this->m_nItemRotateOrigZRatio;

		//right side
		while(pWidget!=NULL)
		{
			pWidget->m_fViewZ=view_z;
			pWidget->m_z_order = z_order--;
			if(pWidget!=m_pLastItem || bInitial==true)
				pWidget->RotateY(&pt_orig,this->m_nItemAngle );
			pWidget=pWidget->pNext;
		}
	}
	
}


bool C3DListWidget::AddItem(CWidget* pItem,int nIndex)
{
		WRect rc;
		WRect rcList;
		m_listContainer.GetRect(&rcList);
		pItem->GetRect(&rc);
		CWidget* pAfterWidget = m_listContainer.InsertWidget(nIndex,pItem);
		pItem->m_bReflect=true;
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
			rc.x=rcPrev.x+rcPrev.dx/m_nSeparateDx;
			rc.y=rcPrev.y;
			rc.dy=rcPrev.dy;
		}
		else
		{
			rc.x=rc.dx;
			rc.y=m_nOffsetTop;		
			
		}
		pItem->SetRect(&rc);
		//如果Item超出listContainer的边界，扩大listContainer的边界
		if(rcList.dx<rc.x-rc.dx*2)
		{
			rcList.dx=rc.x+rc.dx*2;
			m_listContainer.SetRect(&rcList);
		}
		m_nItem++;


		itemList.add(pItem);
	

		return TRUE;		
}


void C3DListWidget::HandleModelEvent(ModelEvent* pEvent)
{
		C3DListWidget* me = this;
		switch (pEvent->evCode)
		{
		case WIDGET_EVENT_ANIMATE_THREAD_FINISH:
			{
				EVENT_ANIMATE_THREAD_FINISH_T* pEvt = (EVENT_ANIMATE_THREAD_FINISH_T*)(pEvent->dwParam);

				
			
				
				break;
			}
		}
}

void C3DListWidget::SetFocusItem(CWidget* pWidget)
{
	this->m_pFocusItem = pWidget;
}

void C3DListWidget::PlayItemSwitchAnimation(CWidget* last,CWidget* cur)
{

			if(last==NULL || cur==NULL)
			{
				return;
			}
			if(last==cur)
			{
				return;
			}


			
			CAnimateThread* pThread = CreateNewAnimateThread(this->GetEventListener());
			C3DRotateWidgetAnimateParam* animate_param=new C3DRotateWidgetAnimateParam();
			C3DRotateWidgetAnimateParam* animate_param_2=new C3DRotateWidgetAnimateParam();

			WRect last_rc;
			last->GetRect(&last_rc);
			WRect cur_rc;
			cur->GetRect(&cur_rc);

			//ReCalLayout(true);


			
			if(last_rc.x<cur_rc.x)
			{
				Scroll(last_rc.dx/this->m_nSeparateDx,0);
			}
			else
			{
				Scroll(-last_rc.dx/this->m_nSeparateDx,0);
			}
			this->InvalidateContent(0);
			ReCalLayout(false);

			
	
			//return;

			//Add a tick delay for let scroll painting.
			




		
			float nFrames=2;
			
			WPoint pt_orig;
			
			pt_orig.y=last_rc.dy/2;
			pt_orig.z=-last_rc.dy*this->m_nItemRotateOrigZRatio;

		
			animate_param->start_radian =0;		
			animate_param_2->max_radian=0;

			float r_speed=(float)m_nItemAngle/nFrames;		
			float r_acceler = (float)m_nItemAngle/(nFrames*4)-0.5;

			
			if(last_rc.x<cur_rc.x)
			{
				pt_orig.x=-last_rc.dx*this->m_nItemRotateOrigXRatio ;
				animate_param->max_radian  = - this->m_nItemAngle;
				animate_param->nAcceler=r_acceler;
				animate_param->nSpeed=-r_speed;
				animate_param->nOrigX = pt_orig.x;

				animate_param_2->start_radian = this->m_nItemAngle;
				animate_param_2->nAcceler=r_acceler;
				animate_param_2->nSpeed=-r_speed;
				animate_param_2->nOrigX=last_rc.dx+last_rc.dx*this->m_nItemRotateOrigXRatio ;

				
			}
			else
			{
				pt_orig.x=last_rc.dx+last_rc.dx*this->m_nItemRotateOrigXRatio ;
				animate_param->nAcceler=-r_acceler;
				animate_param->nSpeed=r_speed;
				animate_param->max_radian   =this->m_nItemAngle;
				animate_param->nOrigX = pt_orig.x;

				animate_param_2->start_radian = -this->m_nItemAngle;
				animate_param_2->nAcceler=-r_acceler;
				animate_param_2->nSpeed=r_speed;
				animate_param_2->nOrigX=-last_rc.dx*this->m_nItemRotateOrigXRatio ;
			}
			animate_param->nOrigY=pt_orig.y;
			animate_param->start_z=pt_orig.z;
			animate_param->max_z = pt_orig.z;
			animate_param->z_speed=0;
			animate_param->z_acceler=0;		
			animate_param->nFadeOption=0;
			animate_param->pSurface=NULL;
			animate_param->pThread=pThread;
			animate_param->pWidget=last;
			animate_param_2->bUpdateScreen=false;

			CTickTickAnimateParam* tick_param=new CTickTickAnimateParam();
			tick_param->nTicks=1;
			tick_param->pSurface=NULL;
			tick_param->pThread=pThread;
			tick_param->pWidget=NULL;
			
			AddNewFrame(tick_param);
			AddNewFrame(animate_param);
			
			

			//Make 2 frames in the same layer, means the two frames with same layer will be run at the same time.
			animate_param_2->m_nAnimateFrameLayer = animate_param->m_nAnimateFrameLayer;

			animate_param_2->nOrigY=pt_orig.y;
			animate_param_2->start_z=pt_orig.z;
			animate_param_2->max_z = pt_orig.z;
			animate_param_2->z_speed=0;
			animate_param_2->z_acceler=0;		
			animate_param_2->nFadeOption=0;
			animate_param_2->pSurface=NULL;
			animate_param_2->pThread=pThread;
			animate_param_2->pWidget=cur;
			animate_param_2->bUpdateScreen=true;

			AddNewFrame(animate_param_2);

			StartThread(pThread);





}
bool C3DListWidget::HandleMouseEvent(int evCode, WPoint* ptScreen, int param2)
{

	WPoint ptMouse;
	ptMouse=*ptScreen;

	if(evCode==MOUSE_LBUTTON_UP)
	{

		this->m_pLastItem=this->m_pFocusItem;
		m_pFocusItem=this->GetItem(&ptMouse);
		if(m_pFocusItem!=NULL && m_pLastItem!=m_pFocusItem)
		{
			PlayItemSwitchAnimation(m_pLastItem,m_pFocusItem);
			//m_pFocusItem->RestoreToOrig();
			//this->ReCalLayout();
			//this->InvalidateContent(0);
		}

		this->m_pLastItem=m_pSelectedItem;
		m_pSelectedItem=this->m_pFocusItem;
		this->HandleItemSelected(m_pLastItem,this->m_pSelectedItem);

	}

	if(evCode==MOUSE_MOVE)
	{

	}
	return true;
}