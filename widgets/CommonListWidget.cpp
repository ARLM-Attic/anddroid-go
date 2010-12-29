/*************************************************************************
eGUI/easyGUI Copyright (c) 2007-2008 Xiao Wang Yang 
Author: Xiao Wang Yang
Email: sureone@gmail.com   
Project Site: http://code.google.com/p/easygui
------------------------------------------
Revision History:
2008-01-09			
	CCommonListWidget
*************************************************************************/
#include "global_event_define.h"
#include "CommonListWidget.h"




CCommonListWidget::CCommonListWidget(void)
{
	m_pFocusItem=NULL;
	m_pLastItem=NULL;
	m_pSelectedItem=NULL;
	m_nItem=0;
	m_nSeparateDx=0;
	m_nOffsetTop=0;		
	InsertWidget(&m_listContainer);
	InsertWidget(&m_bkWidget,Z_ORDER_BACKGROUND);
}
bool CCommonListWidget::Scroll(int dx,int dy)
{
	WRect listViewPort;
	this->m_listContainer.GetViewPort(&listViewPort);
	listViewPort.x+=dx;
	listViewPort.y+=dy;
	if(this->m_listContainer.SetViewPort(&listViewPort)==false)
	{
		return false;
	}
	//this->InvalidateContent();
	return true;		
	
}
bool CCommonListWidget::IsContinueWhileOutView(CWidget* pWidget)
{
	WRect rcScreen;
	this->m_listContainer.GetVisibleRect(&rcScreen);
	WRect rcChild;
	pWidget->GetVisibleRect(&rcChild);
	if(rcChild.x>rcScreen.x+rcScreen.dx || rcChild.y>rcScreen.y+rcScreen.dy)
	{
		return false;
	}
	return true;
}
CWidget* CCommonListWidget::GetSelectedItem()
{
	return m_pFocusItem;
}
CCommonListWidget::~CCommonListWidget(void)
{
	
}
bool CCommonListWidget::AddItem(CWidget* pWidget,int nIndex)
{
	return true;
}
int CCommonListWidget::SetRect(WRect* pRect,bool bNoDraw)
{
	CContainerWidget::SetRect(pRect,bNoDraw);
	WRect rcList;
	CLONE_RECT(pRect,&rcList);
	rcList.x=0;
	rcList.y=0;
	m_listContainer.SetRect(&rcList);
	m_listContainer.SetViewPort(&rcList);

	m_bkWidget.SetRect(&rcList);		
	m_bkWidget.SetBackgroundColor(this->m_clrBackGround);
	return 0;		
}
void CCommonListWidget::HandleItemSelected(CWidget* pLastItem,CWidget* pCurItem)
{
	this->SendModelEvent(WIDGET_LIST_ITEM_SELECTED,(int)this);		
}
void CCommonListWidget::ReCalLayout(bool bInitial)
{
}
CWidget* CCommonListWidget::GetItem(WPoint* pt)
{
	WPoint ptTest=*pt;

	return m_listContainer.GetWidget(&ptTest);
	
}
