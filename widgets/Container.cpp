/*************************************************************************
eGUI Copyright (c) 2007 Xiao Wang Yang 
Author: Xiao Wang Yang
Email: sureone@gmail.com   http://code.google.com/p/easygui
------------------------------------------
Revision History:
2007-11-10			
	V0.1 BaseLine
2007-11-11			
	1.  Add a Z_ORDER property for each Widget, by which the widget will drawed by container as this order, 
		the samlles order will draw first, so the smaller z_order widget will be overlapped by bigger z-order 
		widget.
	2. Change the ClipRect to screen coord relative
2007-11-17
	1. Clip Rect can't be changed duringt the draw, if must be changed, should be restore to old clip rect after
		draw operation finished.
2007-12-10
	1. 添加m_viewPort视图窗口属性, 该属性是指对某个child widget来说,只有落在其父container视图窗口
		中的部分才会被绘制. m_viewPort定义为WRect的类型,使用屏幕绝对坐标系.
2007-12-11.
	1. Container View Scroll Requirment
		ASSUMPATION:
		- Each Container have a m_rect to save its location, which uses relative coordinate.
		- Each Container have a m_viewPort to save its view port, which uses screen absoulte coordinate.
		- In the currently draw function, Since the CDC use the screen coordinate to paint, should first convert 
		  the child widget's m_rect to a screen coordinate by ClientToScreen.
	    - 修改ClientToScreen函数,增加对View Port的支持,
		  例如： 某Container 位于
		  

*************************************************************************/
#include "common.h"
#include "container.h"
#include "DisplayManage.h"
#include "rectop.h"
#define INITIAL_ARRAY_SIZE 10
CContainerWidget::CContainerWidget(void)
{
	m_pWidgetHead=0;
	m_pWidgetTail=0;
	m_widgetType=WIDGET_TYPE_CONTAINER;
	m_pRelationWidget=0;
	m_nWidgtCount=0;
	draw_on_outside=false;
	this->m_bEnableBmpCache=false;

}

CContainerWidget::~CContainerWidget(void)
{

}
void CContainerWidget::RecalPaintWidgetList(CObjList& z_order_list,WRect* pRect,CFrameSurface* pSurface)
{
	WRect rcClip;
	if(pRect==NULL)
	{
		pRect=&rcClip;
		this->GetVisibleRect(pRect);
	}
	else
	{
		rcClip=*pRect;
	}
	if (ISRECTEMPTY(pRect)) {
      return ;
    }
//按照z_order进行排序
	//按照z_order进行排序

	CWidget* pWidget=this->m_pWidgetHead;

	while(pWidget!=NULL)
	{
			WRect rcOut;
			WRect rcWidget;
			WRect rcScreen;
			WRect rcViewPort;
 
			if(pSurface!=NULL)
			{
				if(pWidget->m_nLayerLevel == this->m_nIgnoreDrawLayer && pSurface->IsMemorySurface()==true)
				{
					pWidget=pWidget->pNext;
					continue;
				}
				
				if(pWidget->m_bVisible==false  && pSurface->IsMemorySurface()==false)
				{
					pWidget=pWidget->pNext;
					continue;
				}
			}


			pWidget->GetVisibleRect(&rcScreen);
			WRect rcParentViewPort;			
			pWidget->m_pContainer->GetVisibleRect (&rcParentViewPort);


			//Draw the widget only while widget rect is intersected with clip rect;
			
			if(IntersectRect(&rcOut, &rcScreen, &rcClip)==true)
			{

				if(draw_on_outside==false && pWidget->m_pContainer !=NULL)
				{
	//添加m_viewPort视图窗口属性, 该属性是指对某个child widget来说,只有落在其父container视图窗口
	//	中的部分才会被绘制. m_viewPort定义为WRect的类型,使用相对坐标系.

					//pWidget->m_pContainer->ClientToScreen(&rcParentViewPort);						
					WRect rcInt;
					if(IntersectRect(&rcInt,&rcOut,&rcParentViewPort)==false){
	//当发现某个child widget已经超出当前view port时,调用该函数来决定要不要继续绘制其他的widget.
    //对于某些顺序排列的container如CListWidget来说可以降低绘制开销.
						if(IsContinueWhileOutView(pWidget)==false) 
							break;
						pWidget=pWidget->pNext;
						continue;
					}
					CLONE_RECT(&rcInt,&rcOut);
				}

				pWidget->m_rectCurDrawClip=rcOut;

			}
			else
			{
				if(IsContinueWhileOutView(pWidget)==false ) 
						break;
				pWidget=pWidget->pNext;
				continue;
			}

			

			CObjItem* pTaileItem=z_order_list.m_pTail;
			if(pTaileItem==NULL)
			{
				z_order_list.add(pWidget);
			}
			else
			{
				CWidget* p=(CWidget*)(pTaileItem->m_pData);
				if(pWidget->m_z_order >=p->m_z_order)
				{
					z_order_list.add(pWidget);					
				}else
				{
					CObjItem* pItem = z_order_list.m_pHead ;
					CObjItem* pNewItem=new CObjItem(pWidget);
					while(pItem!=NULL)
					{
						p=(CWidget*)(pItem->m_pData);
						
						if(pWidget->m_z_order <=p->m_z_order)
						{
							pNewItem->m_pNextObj = pItem;
							pNewItem->m_pPrevObj=pItem->m_pPrevObj;
							pItem->m_pPrevObj=pNewItem;
							if(pNewItem->m_pPrevObj!=NULL)
							{
								pNewItem->m_pPrevObj->m_pNextObj=pNewItem;
							}
							else
							{
								z_order_list.m_pHead = pNewItem;

							}
							z_order_list.m_nCount++;
							break;
						}
						pItem=pItem->m_pNextObj;
					}
				}
				
			}


			pWidget=pWidget->pNext;

			



   }
}
int CContainerWidget::CustDraw(CFrameSurface* pSurface)
{
	WRect rcClip;
	WRect rcClient;
	// Get the Clip Rect as the take-care-area
	// Note the Clip Rect is relateive to the client coordinates 
	if(pSurface==0) return 0;
	pSurface->GetClipRect(&rcClip);       
	if (ISRECTEMPTY(&rcClip)) {
      return 0;
    }


	CObjList z_order_list;

	this->RecalPaintWidgetList(z_order_list,&rcClip,pSurface);

	CWidget* pWidget=NULL;
	CObjItem* pObjItem=z_order_list.m_pHead;
	while(pObjItem!=NULL)
	{

			WRect rcScreen;

			pWidget=(CWidget*)(pObjItem->m_pData);
			WRect clipOld;
			pSurface->GetClipRect(&clipOld);
			pSurface->SetClipRect(&(pWidget->m_rectCurDrawClip));
			pWidget->GetVisibleRect(&rcScreen);
				//Intersect the child rect with the parent rect, if it is outside of the parent's client area,
				//Don't need to draw them
			

				//tranverse the coordinates to the child widget's client corrdinates


			int old_alpha = pSurface->GetAlpha();
			pSurface->SetAlpha(pWidget->GetAlpha());

			//对于透明Widget,如果当前不是在Memory Surface上绘图,
			//不可以在原来的Surface位图上绘制,这样会破坏背景,
			//我们需要新建一个新的内存位图暂时替换Surface原有的位图,在该Widget绘制
			//结束后恢复原始位图.
			CMemBitmap* pSavedOld=pSurface->GetSurfaceBitmap();
			if(pWidget->GetAlpha()<255 && pSurface->IsMemorySurface()==false)
			{					
//					pSurface->SetAlpha(200);
				CMemBitmap* pAlphaMemBmp = new CMemBitmap(pSurface->GetScreenCDC(),&rcScreen);
				pSurface->SetSurfaceBitmap(pAlphaMemBmp);			
				pSurface->AddGarbage(pAlphaMemBmp);
			}

			pWidget->Draw(pSurface);	
			pSurface->SetSurfaceBitmap(pSavedOld);
			pSurface->SetAlpha(old_alpha);
			pSurface->SetClipRect(&clipOld);

			pObjItem=pObjItem->m_pNextObj;
	}



	z_order_list.clear();


	//Draw myself



	//pCanvas->EndPaint();

	return 0;
}

//insert widget before index
//return after widget
CWidget* CContainerWidget::InsertWidget(int index,CWidget* pIWidget)
{
	CWidget* pWidget=this->m_pWidgetHead;
	int idx=0;
	CWidget* pBefore=NULL;
	if(index==-1)
	{
		if(this->m_pWidgetTail!=NULL)
		{
			this->m_pWidgetTail->pNext=pIWidget;
			pIWidget->pPrev=m_pWidgetTail;
		}
		else
		{
			this->m_pWidgetHead=pIWidget;
		}
		m_pWidgetTail=pIWidget;
		pBefore= pIWidget;
	}
	else
	{
		while(pWidget!=0)
		{
			if(idx==index)
			{
				pIWidget->pNext=pWidget;
				if(pWidget->pPrev==NULL)
				{
					m_pWidgetHead=pIWidget;
				}
				else
				{
					pWidget->pPrev->pNext=pIWidget;
					pIWidget->pPrev=pWidget->pPrev;
				}
				pWidget->pPrev=pIWidget;
				pBefore= pWidget;
				break;
			}
			pWidget=pWidget->pNext;
			idx++;
		}
	}
	if(pBefore!=NULL)
	{
		pIWidget->SetContainer(this);
		pIWidget->SetDisplayManage(this->GetDisplayManage());
		m_nWidgtCount++;
	}

	pIWidget->m_nLayerLevel=this->m_nLayerLevel+1;
	

	return pBefore;
}
void CContainerWidget::TopMost(CWidget* pWidget)
{

	ASSERT(pWidget!=NULL);
	RemoveWidget(pWidget);
	InsertWidget(pWidget,Z_ORDER_TOPMOST);

}
BOOL CContainerWidget::InsertWidget(CWidget* pWidget,Z_ORDER_T z_order)
{
	if(z_order==Z_ORDER_TOPMOST || z_order==Z_ORDER_NORMAL)
	{				
			pWidget->m_z_order = z_order+this->m_nWidgtCount;
					
	}else if(z_order == Z_ORDER_BACKGROUND)
	{
			pWidget->m_z_order=Z_ORDER_BACKGROUND-this->m_nWidgtCount ;
	}		
	else
	{
		pWidget->m_z_order=z_order;
	}
	if(m_pWidgetHead==0)
	{
		m_pWidgetHead=m_pWidgetTail=pWidget;
		pWidget->pNext=NULL;
		pWidget->pPrev=NULL;
	}
	else
	{
		m_pWidgetTail->pNext=pWidget;
		pWidget->pPrev=this->m_pWidgetTail;
		m_pWidgetTail=pWidget;
		pWidget->pNext=NULL;
	}

	pWidget->SetContainer(this);
	pWidget->SetDisplayManage(this->GetDisplayManage());
	m_nWidgtCount++;
	pWidget->m_nLayerLevel=this->m_nLayerLevel+1;
	
	return 0;
}

	 void CContainerWidget::SetDisplayManage(CDisplayManage* pDisplayManage)
	{

		m_pDisplayManage=pDisplayManage;
		CWidget* pWidget=this->m_pWidgetHead;
		int idx=0;
		while(pWidget!=0)
		{
			pWidget->SetDisplayManage(pDisplayManage);
			pWidget=pWidget->pNext;
			idx++;
		}
		
	}
int CContainerWidget::RemoveWidget(CWidget* pfWidget)
{
	CWidget* pWidget=this->m_pWidgetHead;
	WRect rcClip;
	int idx=0;
	while(pWidget!=0)
	{
		if(pWidget==pfWidget)
		{
			if(pWidget->pPrev!=NULL)
			{
				pWidget->pPrev->pNext=pWidget->pNext;
			}
			else
			{
				this->m_pWidgetHead=pWidget->pNext;
			}

			if(pWidget->pNext!=NULL)
			{
				pWidget->pNext->pPrev=pWidget->pPrev;
			}
			else
			{
				this->m_pWidgetTail=pWidget->pPrev;
			}

			this->m_nWidgtCount--;
			break;
		}
		pWidget=pWidget->pNext;
		idx++;
	}
	pfWidget->pNext=NULL;
	pfWidget->pPrev=NULL;
	return 0;
}

int CContainerWidget::DrawBoundary(int screen_x, int screen_y)
{

	return 0;
}

int CContainerWidget::HandleEvent(int evCode, int param1, int param2)
{
	if(CWidget::HandleEvent(evCode,param1,param2)==-1)
	{
		return 0;
	}
	CWidget* pWidget=this->m_pWidgetTail;
	while(pWidget!=0)
	{
		pWidget->HandleEvent(evCode,param1,param2);		
		pWidget=pWidget->pPrev;
	}	
	return 0;
}

int CContainerWidget::OnChildGainFocus(CWidget* pWidget)
{

	//if widget gain focus, place it at the topmost.
	if(pWidget!=this->m_pWidgetTail)
	{
		if(pWidget->pPrev!=0)
		{
			pWidget->pPrev->pNext=pWidget->pNext;
		}

		if(pWidget->pNext!=0)
		{
			
			pWidget->pNext->pPrev=pWidget->pPrev;

			if(pWidget==m_pWidgetHead)
			{
				m_pWidgetHead=pWidget->pNext;
			}
			
			
		}

		m_pWidgetTail->pNext=pWidget;
		pWidget->pPrev=m_pWidgetTail;
		m_pWidgetTail=pWidget;
		pWidget->pNext=0;
	}

	if(this->m_pContainer!=0)
		m_pContainer->OnChildGainFocus(this);

	return 0;
}

int CContainerWidget::GetWidgetIndex(CWidget* pfWidget)
{
	CWidget* pWidget=this->m_pWidgetHead;
	int idx=0;
	while(pWidget!=0)
	{
		if(pWidget==pfWidget)
		{
			return idx;
		}
		pWidget=pWidget->pNext;
		idx++;
	}
	return -1;
}

CWidget* CContainerWidget::GetWidget(int ifdx)
{
	CWidget* pWidget=this->m_pWidgetHead;
	int idx=0;
	while(pWidget!=0)
	{
		if(idx==ifdx)
		{
			return pWidget;
		}
		pWidget=pWidget->pNext;
		idx++;
	}
	return 0;
}


//使用屏幕坐标
CWidget*  CContainerWidget::GetWidget(WPoint* ptScreen,CWidget* pExcept)
{

	CWidget* pWidget=NULL;

	CObjList z_order_list;
	this->RecalPaintWidgetList(z_order_list);
	CObjItem* pItem=z_order_list.m_pTail;
	while(pItem!=NULL)
	{
		pWidget=(CWidget*)(pItem->m_pData);
		if(pWidget->TestPoint(ptScreen)==true  && pWidget!=pExcept)
		{

			return pWidget;
		}
		pItem=pItem->m_pPrevObj ;
	}

	z_order_list.clear();
	
	return NULL;
}

// 2007-12-06
//该函数用来在动画过程中计算绘制区域，基于如下几点需求
//1. 通常每个MemorySurface都对应于某个Widget
//2. 当对某个Widget的MemorySurface进行动画时,该动画不应该覆盖Z_ORDER大于其的Widget之上
//3. 所以每次AnimateEngine在绘制MemorySurface时，可以调用该函数去计算实际需要绘制的区域
int CContainerWidget::ReCalOverlapInvalidSurface(CMemoryFrameSurface* pSurface)
{
	int numRects=0;

	bool bFoundFlag=false;
	CWidget* pFWidget=(CWidget*)(pSurface->GetWidget());
	if(pFWidget==NULL) return 0;
	CWidget* pWidget=this->m_pWidgetHead;
	WRect rcExclude;
	WRect rcF;
	pFWidget->GetRect(&rcF);
	while(pWidget!=0)
	{
		if(bFoundFlag==true)
		{
			pWidget->GetRect(&rcExclude);

			
			//复杂方案
			
			WRect rcInter;
			if(IntersectRect(&rcInter,&rcExclude,&rcF)==true)
			{
				//Surface Use Screen Coordinate
				this->ClientToScreen(&rcExclude);
				pSurface->ReCalSurfaceByExcludeRect(&rcExclude);
			}
			


		}
		if(pWidget==pFWidget)
		{
			bFoundFlag=true;
		}
		pWidget=pWidget->pNext;		
	}	
	return numRects;
}
