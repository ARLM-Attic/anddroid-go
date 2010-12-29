
#include "viewportcontinerwidget.h"

CViewPortContinerWidget::CViewPortContinerWidget(void)
{
	//Should disable this flag, for ViewPort is designed as Fix-Boundary Widget
	m_bExtendBoundary = false;
}

CViewPortContinerWidget::~CViewPortContinerWidget(void)
{
}

int CViewPortContinerWidget::SetWidget(CWidget* pWidget)
{
	InsertWidget(pWidget,Z_ORDER_NORMAL);
	m_pChildWidget=pWidget;
	return 0;
}

int CViewPortContinerWidget::MoveViewPort(int offset_x, int offset_y, int offset_dx, int offset_dy)
{
	//m_pChildWidget->Move(offset_x,offset_y);

	WRect rc;
	WRect rcMe;
	WRect rcOld;
	m_pChildWidget->GetRect(&rc);
	CLONE_RECT(&rc,&rcOld);
	
	GetRect(&rcMe);
	
	if(rc.dx<rcMe.dx){
		offset_x=0;
	}
	if(rc.dy<rcMe.dy){
		offset_y=0;
	}
	
    rc.x+=offset_x;
	rc.y+=offset_y;

	if(offset_x>0)
		rc.x=MIN(rc.x,0);
	if(offset_y>0)
		rc.y=MIN(rc.y,0);

	if(offset_x<0)
		rc.x=MAX(rcMe.dx-rc.dx,rc.x);
		
	if(offset_y<0)
		rc.y=MAX(rcMe.dy-rc.dy,rc.y);

	

	if(!ISSAMERECT(&rc,&rcOld))
	{
		m_pChildWidget->SetRect(&rc,true);
		this->InvalidateContent(0);
	}
	return 0;
}
