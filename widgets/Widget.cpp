/*************************************************************************
eGUI Copyright (c) 2007 Xiao Wang Yang 
Author: Xiao Wang Yang
Email: sureone@gmail.com   http://code.google.com/p/easygui
------------------------------------------
Revision History:
2007-12-10			
	V0.1 BaseLine
2007-12-11			
	1.  Add a new feature for supporting transparent wiget
2007-12-13
	1. Fix the bug of transparent widget display error.
	The solution is we need draw the transparent in a independed,clear bitmap and buffer it,
	instead of draw it in the currently screen surface, then when we need to draw it, we just 
	Alphabend which from buffer to screen surface.
	So I use two function BeginDrawInMemory and EndDrawInMemory to set a flag to let CDC
	to draw in a new, clear CBitmap. Then use GetMemBitmap to buffer it.
2008-1-1
	1. Widget变形问题

*************************************************************************/

#include "widget.h"
#include "container.h"
#include "DisplayManage.h"
#include "rectop.h"
#include "math.h"
#include "global_event_define.h"
CWidget::CWidget(void)
: m_nAlpha(0)
{
	m_pContainer=0;
	m_strID=0;
	m_nAlpha=0;
	m_bGhost=false;

	m_pWidgetConstraint=0;
	pNext=0;
	pPrev=0;
	SETAEERECT(&m_rectPosition,0,0,0,0);

	m_pDisplayManage=0;
	m_clrBackGround=0xFFFFFF;
	m_bVisible=true;
	m_bLastTempRect=false;

	m_bExtendBoundary=true;

	m_bRedraw=false;

	m_widgetType=WIDGET_TYPE_GENERAL;

	m_bMouseLButtonHold=false;
	m_bNeedDraw=false;
	m_pRelationWidget=0;
	m_pGhostWidget=NULL;



	m_bContentUpdated=true;
	m_bEnableMouseMove=false;

	m_pMemBmp=0;
	m_bEnableBmpCache=false;
	m_bBackGround=FALSE;
	m_pPayLoad=0;

	m_pTempMemBmp=0;
	m_nAlpha=255;
	m_bTopMost=FALSE;
	this->m_bTransparentWidget=true;
	m_viewPort.dx=0;
	m_nKeyScrollXStep=1;
	m_nKeyScrollYStep=1;
	m_bEnableMouseEvent=true;
	this->InitListener();
	m_bAnimating=false;
	m_pAnimateBmp=NULL;
	m_fZoom=1.0;
	m_bExcludeMe=false;
	m_pOrigMemBmp;
	m_fViewZ=10;
	m_lastRect.dx=0;
	m_fRotateAngle=0.0;
	m_pt3DOrig.z=0;
	m_bReflect=false;
	m_fReflectRatio=0.5;

	m_nTransfromType=TRANS_NONE;
	m_nLayerLevel=0;
	this->m_nIgnoreDrawLayer=-1;
	m_pSavedMemorySurfaceBmp=NULL;

	//当子widget超过view rect是否继续paint接下的child widget
	m_bContinueWhileOutView=true;


}



CWidget::~CWidget(void)
{
	if(this->m_strID!=0)
	{
		EG_FREE(this->m_strID);
	}

		if(this->m_pMemBmp!=0)
	{
		delete m_pMemBmp;
		m_pMemBmp=0;
	}
	if(this->m_pAnimateBmp!=NULL)
	{
		delete m_pAnimateBmp;

	}

	if(this->m_pSavedMemorySurfaceBmp!=NULL)
	{
		delete m_pSavedMemorySurfaceBmp;
	}
}
	
	void CWidget::GetViewPort(WRect* pRect)
	{
		if(m_viewPort.dx==0)
		{
			this->GetRect(pRect);
			WRect parentRc;
			CContainerWidget* pContainer = this->GetContainer();
			if(pContainer!=NULL)
			{
				pContainer->GetRect(&parentRc);
				if(pRect->dx+pRect->x>parentRc.dx)
				{
					pRect->dx-=(pRect->dx+pRect->x-parentRc.dx);
				}
				if(pRect->dy+pRect->y>parentRc.dy)
				{
					pRect->dy-=(pRect->dy+pRect->y-parentRc.dy);
				}
			}			
			pRect->x=0;
			pRect->y=0;



		}else
		{
			*pRect=m_viewPort;
		}

	}

//将Widget围绕Y轴旋转
//如果angle=0,而pt_orig的z坐标不为0,则可以实现远近的效果(放大或缩小)

void CWidget::RotateY(WPoint* pt_orig,float angle)
{
	Set3DOrig(pt_orig->x,pt_orig->y,pt_orig->z);

	SetRotateAngle(angle);

	this->m_nTransfromType=TRANS_ROTATE_Y;
}

void CWidget::GetTransformRect(WRect* pRect)
{
	if(TRANS_NONE==m_nTransfromType)
	{
		GetViewPort(pRect);
		this->ClientToScreen(pRect);
		
	}
	else if(TRANS_ROTATE_Y==m_nTransfromType)
	{

		WRect rcScreen;
		GetViewPort(&rcScreen);
		this->ClientToScreen(&rcScreen);

		CBmpTransformRotateY rotateY;
		rotateY.angle=this->m_fRotateAngle;
		rotateY.view_z=this->m_fViewZ;
		rotateY.bReflect=m_bReflect;
		rotateY.fReflectRatio=m_fReflectRatio;
		rotateY.orig=this->m_pt3DOrig;
		rotateY.m_rectOrig = rcScreen;
		
		rotateY.CalDestRect(pRect);
		rotateY.AddReflectSize(pRect);
		pRect->x+=rcScreen.x;
		pRect->y+=rcScreen.y;

	}
}


void CWidget::DrawRotateY(CFrameSurface* pSurface)
{

	    bool bReflect=true;
		float fReflectRatio=0.5;
		
		
		WRect rcScreen;
		GetRectAsScreen(&rcScreen);

		

		CBmpTransformRotateY bmpRotatyTrans;
		bmpRotatyTrans.angle=this->m_fRotateAngle;
		bmpRotatyTrans.view_z=this->m_fViewZ;
		bmpRotatyTrans.bReflect=m_bReflect;
		bmpRotatyTrans.fReflectRatio=m_fReflectRatio;
		bmpRotatyTrans.orig=this->m_pt3DOrig;
		bmpRotatyTrans.EnableMatrixCache(true);


		

		//Get Orig Surface Bitmap
		CMemBitmap* pBmpMe=NULL;

		if(m_pSavedMemorySurfaceBmp==NULL)
		{
			CMemoryFrameSurface* pSurfaceMe = this->CreateMemoryFrameSurface();
			m_pSavedMemorySurfaceBmp=pSurfaceMe->CopySurface(&rcScreen);
			delete pSurfaceMe;
		}
		pBmpMe=m_pSavedMemorySurfaceBmp;
		bmpRotatyTrans.m_rectOrig=rcScreen;

		
		WRect rcDest;
		bmpRotatyTrans.CalDestRect(&rcDest);
		bmpRotatyTrans.AddReflectSize(&rcDest);

		rcDest.x+=rcScreen.x;
		rcDest.y+=rcScreen.y;


		
					
		WRect paintRect;
		paintRect=rcDest;
		
		//获取背景，旋转的过程即用源位图的旋转后的数据去填充背景。

		DWORD dwTick = GetTickCount();
		this->GetContainer()->m_nIgnoreDrawLayer=this->m_nLayerLevel;
		CMemoryFrameSurface* pBKSurface = this->GetContainer()->CreateMemoryFrameSurface();
		this->GetContainer()->m_nIgnoreDrawLayer=-1;
		CMemBitmap* pBKBmp=pBKSurface->CopySurface(&paintRect);
		delete pBKSurface;	
		//TRACE ("Get BackGround BMP took %d seconds\n",  (GetTickCount() - dwTick)  );

		ASSERT(pBKBmp->GetBMP()->m_hObject!=NULL);
		
		//为RotateY设置背景
		pBmpMe->GetBMP()->SetBackGround(
			pBKBmp->GetBMP(),
			0,//abs(unionRect.x-origRect.x),
			0//abs(unionRect.y-origRect.y)
			);
		dwTick = GetTickCount();
		pBmpMe->GetBMP()->RotateY(&bmpRotatyTrans);		
		//TRACE ("RotateY took %d seconds\n",  (GetTickCount() - dwTick)  );


		
		


		bmpRotatyTrans.m_rectPaint = rcDest;
		dwTick = GetTickCount();
		CEgBitmap* pCacheBmp = bmpRotatyTrans.GenerateBmp();
		//TRACE ("GenerateBmp took %d seconds\n",  (GetTickCount() - dwTick) );
		WRect oldClip;
		pSurface->GetClipRect(&oldClip);
		pSurface->SetClipRect(&rcDest);
		pSurface->DrawPureBmp(pCacheBmp,&rcDest);
		pSurface->SetClipRect(&oldClip);
		pCacheBmp->DeleteObject();

		delete pBKBmp;
		delete pCacheBmp;
		


		//
}
int CWidget::Set3DOrig(int x,int y,int z)
{
	this->m_pt3DOrig.x=x;
	this->m_pt3DOrig.y=y;
	this->m_pt3DOrig.z=z;
	return 0;
}
int CWidget::SetRotateAngle(float angle)
{
	this->m_fRotateAngle=angle;	

	return 0;
}
void CWidget::EnableBmpCache(bool bFlag)
{
	this->m_bEnableBmpCache=bFlag;
}
bool CWidget::IsGhost()
{
	return m_bGhost;
}
bool CWidget::IsAnimating()
{
	return this->m_bAnimating;
}
void CWidget::SetAnimatingFlag(bool bFlag)
{
	this->m_bAnimating=bFlag;
}
//the rect is a screen relative; 
int CWidget::InvalidateRectAsScreen(WRect* rectScreen,CFrameSurface* pSurface)
{
	CWidget* pContainer=this->GetContainer();
	if(pContainer!=NULL)
	{
		pContainer->InvalidateRectAsScreen(rectScreen,pSurface);
	}
	else
	{
		if(GetDisplayManage()->IsDisplayReady()== false)
		{
			return -1;
		}
		if(pSurface==NULL)
			pSurface = GetDisplayManage()->BeginPaint(rectScreen);
		pSurface->SetClipRect(rectScreen);	
		// Surface‘s view port use screen coordinate
		pSurface->SetViewPort(rectScreen);
		this->Draw(pSurface);
		if(pSurface->IsMemorySurface()==false)
			GetDisplayManage()->EndPaint(pSurface);
	}
	return 0;


}
int CWidget::CustDraw(CFrameSurface* pSurface)
{
	return 0;
}
void CWidget::RestoreToOrig()
{
	
	this->m_nTransfromType=TRANS_NONE;
	this->m_fRotateAngle=0;
	this->m_pt3DOrig.z=0;
	this->SetRect(&(this->m_rectOrig ));
}
void CWidget::SetAnimateBmp(CMemBitmap* pBmp)
{

	if(this->m_pAnimateBmp !=NULL)
	{
		delete m_pAnimateBmp;
		m_pAnimateBmp=NULL;
	}
	m_pAnimateBmp=pBmp;
	WRect rc;
	pBmp->GetRect(&rc);
	this->GetContainer()->ScreenToClient(&rc);
	this->SetRect(&rc);
}
void CWidget::GetLastRect(WRect* pRect)
{
	*pRect=this->m_lastRect;
}
void CWidget::SetLastRect(WRect* pRect)
{
	this->m_lastRect=*pRect;
}
void CWidget::ConvertPPointTo2DPoint(float& xp,float& yp)
{
		WRect rc;
		GetRectAsScreen(&rc);
		float x2d,y2d;

		xp-=rc.x;
		yp-=rc.y;

		xp-=this->m_pt3DOrig.x;
		yp-=this->m_pt3DOrig.y;


		if(this->m_fRotateAngle!=0)
		{			
			float cosA=cos(this->m_fRotateAngle*(PI/180));
			float sinA=sin(this->m_fRotateAngle*(PI/180));
			double zo_zview=this->m_pt3DOrig.z/this->m_fViewZ;
			x2d=(1.0-zo_zview)/(sinA/m_fViewZ+cosA/xp);
			y2d=yp*(1.0-(x2d*sinA)/m_fViewZ-zo_zview);
		}
		else
		{
			double zo_zview=m_fViewZ/(m_fViewZ-m_pt3DOrig.z);
			x2d=xp/(zo_zview);
			y2d=yp/(zo_zview);
		}
		x2d+=this->m_pt3DOrig.x;
		y2d+=this->m_pt3DOrig.y;
		x2d+=rc.x;
		y2d+=rc.y;
		xp=x2d;
		yp=y2d;
}
void CWidget::GetVisibleRect(WRect* pRect)
{
	this->GetTransformRect(pRect);
}
bool CWidget::TestPoint(WPoint* pt_2d)
{


	WRect rcPaint;
	this->GetVisibleRect (&rcPaint);

	float xp=pt_2d->x;
	float yp=pt_2d->y;
	//ConvertPPointTo2DPoint(xp,yp);
	//WRect rc;
	//this->GetRectAsScreen(&rc);
	return CRectOp::IsPtIn(xp,yp,&rcPaint);

}
bool CWidget::TestRectIntersect(WRect* rect_2d)
{

	return false;
}

int CWidget::Draw(CFrameSurface* pSurface)
{
	WRect rc;
	WRect rcScreen;

	//该标志表示如果在构造Memory Frame Surface时将自己排除在外,通常用在
	//为了得到某个Widget的背景时
	if(pSurface->IsMemorySurface()==TRUE && m_bExcludeMe==true)
	{
		return 0;
	}
	if(this->m_bVisible==FALSE && pSurface->IsMemorySurface()==FALSE) return 0;
	{
			GetRectAsScreen(&rcScreen);
	}

	if(this->m_nTransfromType==TRANS_ROTATE_Y && pSurface->IsMemorySurface()==FALSE)
	{

		DWORD dwTick = GetTickCount();
		this->DrawRotateY(pSurface);
		//TRACE ("CWidget::DrawRotateY took % d seconds\n", 	 (GetTickCount() - dwTick) );
			

		return 0;
	}


	

	if(this->m_bAnimating ==true && this->m_pAnimateBmp !=NULL && pSurface->IsMemorySurface()==FALSE)
	{
		GetRectAsScreen(&rcScreen);
		WRect rcClip;
		pSurface->GetClipRect(&rcClip);
//		m_pMemBmp->m_rect=rcScreen;		
		pSurface->DrawBmp(this->m_pAnimateBmp,&rcClip);
		return 0;
	}

	if(m_bContentUpdated ==true || m_bEnableBmpCache==false)
	{

		if(this->m_fZoom!=1.0)
		{


			CEgBitmap* pSrc=NULL;
			if(this->m_pAnimateBmp!=NULL)
			{
				pSrc=this->m_pAnimateBmp->GetBMP();
				pSurface->DrawStretchBmp(pSrc,&rcScreen);
			}else
			{
				WRect rcSaved=this->m_rectPosition;
				this->m_rectPosition=m_rectOrig;
				CFrameSurface* pMemSurface = this->CreateWidgetSurface(NULL);
				CustDraw(pMemSurface);
				this->m_rectPosition=rcSaved;		
				CEgBitmap* pSrc=pMemSurface->GetSurfaceMemBitmap()->GetBMP();			
				//pSrc->Stretch(m_fZoom,m_fZoom);
				pSurface->DrawStretchBmp(pSrc,&rcScreen);
				delete pMemSurface;
			}

			
		}
		else
		{
			CustDraw(pSurface);


		}		
		if(this->m_pMemBmp!=0)
		{
			delete m_pMemBmp;
			m_pMemBmp=0;
		}
		if(m_bEnableBmpCache==true)
		{			
			this->m_pMemBmp=pSurface->GetMemBitmap(&rcScreen);		
			m_bContentUpdated=false;
		}
	}
	//not need redraw, exampel, we just move the widget form one place to another.
	else if(m_bEnableBmpCache==true)
	{
		GetRectAsScreen(&rcScreen);
		WRect rcClip;
		pSurface->GetClipRect(&rcClip);

//		m_pMemBmp->m_rect=rcScreen;		
		pSurface->DrawBmp(this->m_pMemBmp,&rcClip);
		//pSurface->BitBlt(this->m_pMemBmp,&rcClip);
	}





	
	return 0;
}
int CWidget::InvalidateContent(CFrameSurface* pMemorySurface)
{
	WRect rcView;
	//invalidate the entire client area in view port;
	this->m_bContentUpdated=TRUE;
	this->GetVisibleRect(&rcView);
	this->InvalidateRectAsScreen(&rcView,pMemorySurface);
	
	return 0;
}
 int CWidget::Move(WRect* pRect)
 {

	return 0;
 }
int CWidget::Move(int dx,int dy)
{

	return 0;
}

int CWidget::SetBackgroundColor(int color)
{
	m_clrBackGround=color;
	return 0;
}
void CWidget::SetPayLoad(unsigned int pPayLoad)
{
	m_pPayLoad=pPayLoad;
}
	unsigned int CWidget::GetPayLoad()
	{
		return m_pPayLoad;
	}
void CWidget::Zoom(float fZoom)
{
	CWidget* pGhost = this->GetGhost();
	if(pGhost!=NULL)
	{
		pGhost->Zoom(fZoom);
	}
	float oldZoom=m_fZoom;
	m_fZoom=fZoom;
	WRect rcOld;
	GetRect(&rcOld);
	float cx=rcOld.dx;
	float cy=rcOld.dy;

	cx=fZoom*cx;
	cy=fZoom*cy;

	WRect rc;
	rc.dx=cx;
	rc.dy=cy;
	rc.x=m_rectOrig.x-(cx-m_rectOrig.dx)/2;
	rc.y=m_rectOrig.y-(cy-m_rectOrig.dy)/2;
	SetRect(&rc);
	if(oldZoom > fZoom)
	{
		rc=rcOld;
	}

	CContainerWidget* pParent=GetContainer();
	ASSERT(pParent!=NULL);
	pParent->ClientToScreen(&rc);
	pParent->InvalidateRectAsScreen(&rc);	
}


void CWidget::SetRect(int x,int y,int dx,int dy)
{
	WRect rc;
	rc.x=x;
	rc.y=y;
	rc.dx=dx;
	rc.dy=dy;
	SetRect(&rc);
}
int CWidget::SetRect(WRect* pRect,bool bNoDraw){

	//Not ZOOM and Animating

		m_lastRect=*pRect;
		
		if(this->m_fZoom==1.0)
		{
			m_rectOrig=*pRect;
		}


		if(pRect->dx==m_rectPosition.dx && pRect->dy==m_rectPosition.dy)
		{
			CWidget* pCont=GetContainer();
			if(pCont!=NULL);
			WRect rc=*pRect;
			pCont->ClientToScreen(&rc);
			
			if(this->m_pMemBmp!=NULL)
			{
				this->m_pMemBmp->SetRect(&rc);
			}
		}else
		{
			if(this->m_pMemBmp!=NULL)
			{
				this->m_bContentUpdated=true;
				delete m_pMemBmp;
				m_pMemBmp=NULL;
			}

		}
		CLONE_RECT(pRect,&m_rectPosition);
		this->SendModelEvent(WIDGET_EVENT_POSITION_CHANGED,int(this));

		


		
		return 0;
};
int CWidget::ExtendBoundary(WRect* wn)
{
	return 0;
}

int CWidget::InvalidateClientArea(WRect* pRect)
{
	WRect rcView;
	this->GetViewPort(&rcView);
	
	if(pRect==NULL)
	{
		pRect=&rcView;				
	}
	this->ClientToScreen(&rcView);
	this->InvalidateRectAsScreen(&rcView,0);
	return 0;
}

WIDGET_TYPE_T CWidget::GetWidgetType(void)
{
	return m_widgetType;
}
int CWidget::ScreenToClient(int* x,int* y,int* z)
{
	int xMe,yMe,zMe;
	xMe=yMe=zMe=0;
	ClientToScreen(&xMe,&yMe,&zMe);
	(*x)-=xMe;
	(*y)-=yMe;

	return 0;
}
//Convert client cordinate to absolutely screen cordinate
//考虑view port 属性
//当 viewport向下移动时，client内容向上移动，即屏幕坐标减小

int CWidget::ClientToScreen(int* x, int* y,int* z)
{
	CWidget* pParent= this->m_pContainer;
	WRect myPosition;
	GetRect(&myPosition);
	WRect myViewPort;
	GetViewPort(&myViewPort);
	
	//减去viewport的位置
	(*x)-=myViewPort.x;
	(*y)-=myViewPort.y;

	//换算成父container的client坐标
	(*x)+=myPosition.x;
	(*y)+=myPosition.y;

	if(z!=NULL)
	{
		(*z)+=myPosition.z;
	}

	if(pParent!=NULL)
	{
		pParent->ClientToScreen(x,y,z);
	}
	
	return 0;
}

int CWidget::InvalidateScreenArea(WRect* pRect)
{
	this->InvalidateRectAsScreen(pRect);
	return 0;
}

int CWidget::ClientToScreen(WRect* rc)
{
	return ClientToScreen(&(rc->x),&(rc->y),&(rc->z));
}

int CWidget::ScreenToClient(WRect* rc)
{
	return ScreenToClient(&(rc->x),&(rc->y),&(rc->z));
}

int CWidget::ClientToScreen(WPoint* pt)
{
	return ClientToScreen(&(pt->x),&(pt->y),&(pt->z));
}
void CWidget::EnableMouse()
{
	this->m_bEnableMouseEvent=true;
}
void CWidget::DisableMouse()
{
	this->m_bEnableMouseEvent=false;
}

bool CWidget::HandleMouseEvent(int evCode, WPoint* pPoint, int param2)
{

	if(m_bEnableMouseMove==false)
	{
		return true;
	}
		WPoint ptMouse=*pPoint;
		if(evCode==MOUSE_LBUTTON_UP)
		{
			this->m_bMouseLButtonHold=false;
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
				

			}
		}

		if(evCode==MOUSE_MOVE)
		{
			
			if(m_bMouseLButtonHold==true && this->m_bEnableMouseMove ==true)
			{
				int dy=ptMouse.y-this->m_ptMouseOld.y;
				int dx=ptMouse.x-this->m_ptMouseOld.x;

				WRect rcOld;
				this->GetRect(&rcOld);	
				WRect rcNew=rcOld;
				rcNew.x+=dx;
				rcNew.y+=dy;
				this->SetRect(&rcNew);
				CWidget* pContainer=this->GetContainer();
				if(pContainer!=NULL)
				{
					GetContainer()->ClientToScreen(&rcOld);
					GetContainer()->ClientToScreen(&rcNew);
					WRect rcUnion;
					IntersectRect(&rcUnion,&rcOld,&rcNew);
					GetContainer()->InvalidateClientArea(&rcUnion);
				}


				

			}
		}

		this->m_ptMouseOld = ptMouse;

};
int CWidget::HandleEvent(int evCode, int param1, int param2){


	if(IS_KEY_EVENT(evCode)==true)
	{
		WKEY_EVENT_T key_event = *((WKEY_EVENT_T*)(param1));
		HandleKeyEvent(evCode,&key_event,param2);
	}

	if(IS_MOUSE_EVENT(evCode)==true)
	{

		if(m_bEnableMouseEvent==false)
		{
			return -1;
		}
		WPoint pt = *((WPoint*)param1);


		//判断鼠标是否落在Widget的View Port之内
		WPoint ptClient = pt;
		ScreenToClient(&(ptClient.x),&(ptClient.y));
		WRect viewPort;
		GetViewPort(&viewPort);

		if(CRectOp::IsPtIn(&ptClient,&viewPort)==false)
		{
			return -1;
		}

		HandleMouseEvent(evCode,&pt,param2);

	}


	return 0;






}

int CWidget::GainFocus()
{
	m_bFocus=true;
	if(this->m_pContainer!=0)
	{
		this->m_pContainer->OnChildGainFocus(this);
	}
	return 0;
}
#if 1
void CWidget::UpdateMove(void* userData)
{
	

}
#endif


CMemBitmap* CWidget::GetBitMap()
{
	return NULL;
}




int CWidget::SetDisplayTarget(bool bEnable)
{

	return 0;
}

int CWidget::SetAlpha(int nAlpha)
{
	this->m_nAlpha = nAlpha;
	if(nAlpha<255)
	{
		this->m_bTransparentWidget=true;
	}
	return 0;
}

int CWidget::SetVisible(bool visible)
{
	this->m_bVisible=visible;
	return 0;
}
CMemoryFrameSurface* CWidget:: CreateWidgetSurface(WRect* rcScreen)
{
			WRect rcMe;
			if(rcScreen==NULL)
			{
				GetRectAsScreen(&rcMe);			
			}
			else
			{
				rcMe=*rcScreen;
			}
		CDisplayManage* pDM = this->GetDisplayManage();

		if(pDM!=NULL)
		{

			CMemoryFrameSurface* pSurface = pDM->BeginMemoryPaint(&rcMe);
			/*Set view port*/			
			WRect rcView;
			if(m_pContainer!=NULL)
			{
				m_pContainer->GetViewPort(&rcView);

				//Since pSurface use screen coordinate for view port but widget use client coordinate
				//here must convert the widget's view port to screen
				
				m_pContainer->ClientToScreen(&rcView);
				pSurface->SetViewPort(&rcView);
			}
			pSurface->SetWidget(this);			
			pSurface->SetAlpha(255); // set no alpha
			pSurface->SetClipRect(&rcMe);
			return pSurface;
		}
		return NULL;
}

CMemoryFrameSurface* CWidget:: CreateMemoryFrameSurface(WRect* pRectOnly,bool bExcludMe)
	{

		if(GetDisplayManage()->IsDisplayReady()==false)
		{
			return NULL;
		}
		WRect rcMe;
		if(pRectOnly==NULL)
		{
			GetRectAsScreen(&rcMe);			
		}
		else
		{
			rcMe=*pRectOnly;
			this->ClientToScreen(&rcMe);
		}






		CDisplayManage* pDM = this->GetDisplayManage();

		if(pDM!=NULL)
		{
			
			//对于透明Widget来说，在生成MemoryFrameSurface时是无需在意Alpha属性的，
			//因为Alpha属性是相对于背景来说的，而MemoryFrameSurface是没有背景的，所以
			//也无从谈起其的Alpha属性。所以在生成MemorySurface之前将其Alpha设为255，在生成
			//结束后在设置Alpha的正确值用来将此Surface贴到背景时使用。
			CMemoryFrameSurface* pSurface = pDM->BeginMemoryPaint(&rcMe);
			/*Set view port*/			
			WRect rcView;
			if(m_pContainer!=NULL)
			{
				m_pContainer->GetViewPort(&rcView);

				//Since pSurface use screen coordinate for view port but widget use client coordinate
				//here must convert the widget's view port to screen
				
				m_pContainer->ClientToScreen(&rcView);
				pSurface->SetViewPort(&rcView);
			}

			pSurface->SetWidget(this);
			
			pSurface->SetAlpha(255); // set no alpha
			pSurface->SetClipRect(&rcMe);
			//Start Draw from rootContainer
			//this->InvalidateContent(pSurface);
			//Start Draw myself only
			this->Draw(pSurface);
			pSurface->SetAlpha(this->GetAlpha()); //set correct alpha, which 
			pDM->EndMemoryPaint(pSurface);
			return pSurface;
		}

	}

	 CWidget* CWidget::GetRootContainer()
	{
		CWidget* pRoot=this;
		CWidget* pTemp=this;
		while(true)
		{
			pRoot=pTemp;
			pTemp = pTemp->GetContainer();
			if(pTemp==NULL)
			{
				return pRoot;
			}						
		}
	}
	bool CWidget::HandleKeyEvent(int evCode, WKEY_EVENT_T* key_event, int param2)
	{
		switch(evCode)
		{

		case KEY_EVENT_DOWN:
			{				
				HandleKeyDown(key_event->key_code,key_event->rpt_cnt,key_event->flag);
			}
			break;
		}
		return true;
	};
