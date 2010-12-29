
#include ".\rectop.h"

CRectOp::CRectOp(void)
{
}

CRectOp::~CRectOp(void)
{
}

bool CRectOp::BottomOverTop(WRect* rc1, WRect* rc2)
{
	return ((rc1->y+rc1->dy)<(rc2->y));

}

bool CRectOp::TopLowBottom(WRect* rc1, WRect* rc2)
{
	return ((rc1->y)>(rc2->y+rc2->dy));

}


bool CRectOp::RightBeforeLeft(WRect* rc1, WRect* rc2)
{
	return ((rc1->x+rc1->dx)<(rc2->x));

}

bool CRectOp::LeftAfterRight(WRect* rc1, WRect* rc2)
{
	return ((rc1->x)>(rc2->x+rc2->dx));

}


bool CRectOp::TopLeft(WRect* rc1, WRect* rc2)
{
	WPoint pt1;
	WPoint pt2;
	CENTER(pt1,*rc1);
	CENTER(pt2,*rc2);

	if(pt1.x<pt2.x && pt1.y<pt2.y)
	{
		return true;
	}
	return false;

}
bool CRectOp::TopRight(WRect* rc1, WRect* rc2)
{
		WPoint pt1;
	WPoint pt2;
	CENTER(pt1,*rc1);
	CENTER(pt2,*rc2);

	if(pt1.x>pt2.x && pt1.y<pt2.y)
	{
		return true;
	}
	return false;

}
bool CRectOp::BottomLeft(WRect* rc1, WRect* rc2)
{
		WPoint pt1;
	WPoint pt2;
	CENTER(pt1,*rc1);
	CENTER(pt2,*rc2);

	if(pt1.x<pt2.x && pt1.y>pt2.y)
	{
		return true;
	}
	return false;


}
bool CRectOp::BottomRight(WRect* rc1, WRect* rc2)
{
		WPoint pt1;
	WPoint pt2;
	CENTER(pt1,*rc1);
	CENTER(pt2,*rc2);

	if(pt1.x>pt2.x && pt1.y>pt2.y)
	{
		return true;
	}
	return false;

}



bool CRectOp::Top(WRect* rc1, WRect* rc2)
{
	WPoint pt1;
	WPoint pt2;
	CENTER(pt1,*rc1);
	CENTER(pt2,*rc2);

	if(pt1.x==pt2.x && pt1.y<pt2.y)
	{
		return true;
	}
	return false;

}
bool CRectOp::Right(WRect* rc1, WRect* rc2)
{
		WPoint pt1;
	WPoint pt2;
	CENTER(pt1,*rc1);
	CENTER(pt2,*rc2);

	if(pt1.x>pt2.x && pt1.y==pt2.y)
	{
		return true;
	}
	return false;

}
bool CRectOp::Left(WRect* rc1, WRect* rc2)
{
		WPoint pt1;
	WPoint pt2;
	CENTER(pt1,*rc1);
	CENTER(pt2,*rc2);

	if(pt1.x<pt2.x && pt1.y==pt2.y)
	{
		return true;
	}
	return false;


}
bool CRectOp::Bottom(WRect* rc1, WRect* rc2)
{
		WPoint pt1;
	WPoint pt2;
	CENTER(pt1,*rc1);
	CENTER(pt2,*rc2);

	if(pt1.x==pt2.x && pt1.y>pt2.y)
	{
		return true;
	}
	return false;

}


bool CRectOp::IsPtIn(int x, int y, WRect* pRect)
{
	if( (x>=pRect->x && (x<=(pRect->x+pRect->dx))) &&
		(y>=pRect->y && (y<=(pRect->y+pRect->dy))) )

	{
		return true;
	}
	return false;
}

bool CRectOp::IsPtIn(WPoint* pt, WRect* pRect)
{
	return IsPtIn(pt->x,pt->y,pRect);
}

WRect* CRectOp::IsRectContain(WRect* rc1,WRect* rc2)
{
	int xl1=rc1->x;
	int yl1=rc1->y;

	int xr1=rc1->dx+rc1->x;
	int yr1=rc1->dy+rc1->y;

	int xl2=rc2->x;
	int yl2=rc2->y;

	int xr2=rc2->dx+rc2->x;
	int yr2=rc2->dy+rc2->y;


	if(xl2>=xl1 && yl2>=yl1 && xr2<=xr1 && yr2<=yr1)
	{
		return rc1;
	}
	
	if(xl1>=xl2 && yl1>=yl2 && xr1<=xr2 && yr1<=yr2) 

	{
		return rc2;
	}
	return NULL;


}


int CRectOp::ConvertToClientCoord(CWidget* pWidget, WPoint* pt)
{

	WRect rcMe;
	pWidget->GetRect(&rcMe);
	pt->x-=rcMe.x;
	pt->y-=rcMe.y;
	return 0;
}


