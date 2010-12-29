
#include "constraintcontainer.h"
typedef short int16;
typedef unsigned short uint16;

CConstraintContainer::CConstraintContainer(void)
{

}

CConstraintContainer::~CConstraintContainer(void)
{

}



int CConstraintContainer::SetConstraint(CWidget* pWidget, const WidgetConstraint* pWC, CWidget* pWidgetBefore)
{
	if(pWidget->m_pWidgetConstraint!=0)
	{
		if(Constraint_equals(((const WidgetConstraint*)(pWidget->m_pWidgetConstraint)),pWC))
		{
			return 0;
		}
		else
		{
			
			*(( WidgetConstraint*)(pWidget->m_pWidgetConstraint)) = *pWC;

			DoLayout(this,0);
		}
	}else
	{
		pWidget->m_pWidgetConstraint = (WidgetConstraint*)EG_MALLOC(sizeof(WidgetConstraint));
		*(( WidgetConstraint*)(pWidget->m_pWidgetConstraint)) = *pWC;

		DoLayout(pWidget,0);



	}
	return 0;
}


int CConstraintContainer::DoLayout(CWidget* pNewWidget,WRect* pInvRect)
{
	


	bool bFindNewWidget=false;
	CWidget* pWidget=this->m_pWidgetHead;
	while(pWidget!=0)
	{
			WRect rcWidget;
   

			if(pWidget==pNewWidget)
			{
				
				bFindNewWidget=true;
			}
			if(bFindNewWidget==true)
			{
				LayoutWidget(pWidget);
				pWidget->m_bRedraw=true;
			}
		
		    pWidget  =  pWidget->pNext;
	}

	 pWidget=this->m_pWidgetHead;
	 WRect rcOut;
	 SETAEERECT(&rcOut,0,0,0,0);
	while(pWidget!=0)
	{
			WRect rcWidget;
			WRect rcNext;
			pWidget->GetRect(&rcWidget);
			
			if(pWidget->m_bRedraw==true)
			{
				if(rcOut.dx==0)
				{
					CLONE_RECT(&rcWidget,&rcOut);

				}else
				{
					UnionRect(&rcOut,&rcOut,&rcWidget);
				}
				pWidget->m_bRedraw=false;

			}
			pWidget  =  pWidget->pNext;						
	}

	ExtendBoundary(&rcOut);

	InvalidateClientArea(&rcOut);

	




	return 0;
}

static int16 Constraint_valueFor(uint16 type, int16 offset, CWidget* pWidget,WRect* pRcContainer)
{
   int16 result;

   WRect rectPosition;
   switch(type) {
   case CONSTRAINT_PARENT_LEFT:
      result = offset;
      break;
   case CONSTRAINT_PARENT_RIGHT:
      result = pRcContainer->dx + offset;
      break;
   case CONSTRAINT_PARENT_TOP:
      result = offset;
      break;
   case CONSTRAINT_PARENT_BOTTOM:
      result = pRcContainer->dy + offset;
      break;
   case CONSTRAINT_PARENT_WIDTH:
      result = (pRcContainer->dx * offset) / 100;
      break;
   case CONSTRAINT_PARENT_HEIGHT:
      result = (pRcContainer->dy * offset) / 100;
      break;
   case CONSTRAINT_PREV_LEFT:
	   if(pWidget->pPrev!=0)
	   {
		   pWidget->pPrev->GetRect(&rectPosition);
			result = rectPosition.x + offset;
	   }else
	   {
		   result=offset;
	   }
      break;
   case CONSTRAINT_PREV_RIGHT:
      
	   


	  if(pWidget->pPrev!=0)
	   {

		   pWidget->pPrev->GetRect(&rectPosition);
			result = rectPosition.x + rectPosition.dx + offset;
	   }else
	   {
		   result = offset;
	   }
      break;
   case CONSTRAINT_PREV_TOP:
	     if(pWidget->pPrev!=0)
	   {
		   pWidget->pPrev->GetRect(&rectPosition);
			result = rectPosition.y + offset;
	   }else
	   {
		   result=offset;
	   }
      
      break;
   case CONSTRAINT_PREV_BOTTOM:
	 if(pWidget->pPrev!=0)
	   {
		    pWidget->pPrev->GetRect(&rectPosition);
			 result = rectPosition.y +rectPosition.dy + offset;
	   }else
	   {
		   result = offset;
	   }
     
      break;            
   case CONSTRAINT_PARENT_HCENTER:
      result = (pRcContainer->dx)/2 + offset;
      break;
   case CONSTRAINT_PARENT_VCENTER:
      result = (pRcContainer->dy)/2 + offset;
      break;      
   default:
      result = 0;
   }

   return result;
}

int CConstraintContainer::LayoutWidget(CWidget* pWidget)
{

	WRect wn;
	WRect rcContainer;
	pWidget->m_pContainer->GetRect(&rcContainer);

   int16 width, height;

	Constraint* c =& ((WidgetConstraint*)pWidget->m_pWidgetConstraint)->constraint ;

   wn.x = Constraint_valueFor(c->left.type, c->left.offset, pWidget, &(rcContainer));
   wn.y = Constraint_valueFor(c->top.type, c->top.offset, pWidget, &(rcContainer));
   wn.dx = Constraint_valueFor(c->right.type, c->right.offset, pWidget, &(rcContainer));
   wn.dy = Constraint_valueFor(c->bottom.type, c->bottom.offset, pWidget, &(rcContainer));
   // Check for some constraints based on existing/preferred size

   WRect rcOld;
   pWidget->GetRect(&rcOld);

  
   // we define WIDGET_CENTER_SIZE_TO_FIT to mean "centre using the preferred
   // extent of the widget"
   if (c->left.type == CONSTRAINT_PARENT_HCENTER && c->right.type == CONSTRAINT_PARENT_HCENTER)
   {
		wn.dx = rcOld.dx;
		wn.x=(rcContainer.dx-rcOld.dx)/2;   
   }   

   // and in the y direction
   if (c->bottom.type == CONSTRAINT_KEEP_WIDGET_EXTENT )
   {
      wn.dy=rcOld.dy;
   }
 if (c->right.type == CONSTRAINT_KEEP_WIDGET_EXTENT )
   {
	   wn.dx=rcOld.dx;
   
   }
   
   if (c->top.type == CONSTRAINT_PARENT_VCENTER || c->bottom.type == CONSTRAINT_PARENT_VCENTER)
   {
		wn.dy = rcOld.dy;
		wn.y=(rcContainer.dy-rcOld.dy)/2;   
   }

   pWidget->SetRect(&wn,true);
   pWidget->m_bRedraw=true;


  


	return 0;
}

int CConstraintContainer::ExtendBoundary(WRect* wn)
{

	if(m_bExtendBoundary==true)
	{
		WRect rcParent;
		GetRect(&rcParent);
		   
		if(wn->dx+wn->x>rcParent.dx)
		{
			rcParent.dx=wn->dx+wn->x;		   
		}
		if(wn->dy+wn->y>rcParent.dy)
		{
			rcParent.dy=wn->dy+wn->y;		   
		}

		SetRect(&rcParent,true);
	}
	
	return 0;
}
