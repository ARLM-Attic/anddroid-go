/*************************************************************************
eGUI Copyright (c) 2007 Xiao Wang Yang 
Author: Xiao Wang Yang
Email: sureone@gmail.com   http://code.google.com/p/easygui
------------------------------------------
Revision History:
2007-11-10	Xiao Wang Yang		
	V0.1 BaseLine
2007-11-11	Xiao Wang Yang		
	1.  Add a new feature for supporting transparent wiget
2007-11-17 Xiao Wang Yang
	1.  Add a animate engine in charge of the animation management.
	2.  增加了一个CFrameSurface, 该Surface只是个内存影像,每次绘图之前即(BeginPaint)须Create
		一个FrameSurface,当绘图结束时候即(EndPaint)再将该FrameSurface添加到MyCDC管理的
		FrameSurface列表。然后由AnimateEngine每隔一定的时间将FrameSurface列表BitBlt到实际
		的屏幕CDC中，完成绘图的显示。	
2007-12-05 Xiao Wang Yang
	1. 在设置背景时需要考虑的运动过程中涉及的背景区域是否在当前view port之内，超出
	当前view port的部分的背景是不需要重绘，因为我们根本就不会在当前的view port之外
	绘图。
2008-01-10 Xiao Wang Yang
	
	1. 给每个AnimateThread设置了一个分层的结构,及AnimateThread所包含的Frame可以处于不同的layer,
	即不同的时间断，其通过AnimateFrame的m_nFrameLayer来区别，从运行的时间上来说，m_nFrameLayer最
	小的Frame最先运行，当具有相同m_nFrameLayer的Frame接受以后，再运行m_nFrameLayer大些的Frame.
	如果在创建时没有指定m_nFrameLayer,则系统自动按照从小到大的顺序分配，即所有的Frame处于不同的layer，
	也即将会一个接着一个的运行

	2.增加自动Scroll view的功能
*************************************************************************/
#include "animatecontroller.h"
#include "Widget.h"
#include "RectOP.h"
#include "math.h"
#include "Widget.h"
#include "Container.h"
CAnimateEngine g_animateEngine;

DLL_EXP BOOL StartAnimateEngine(int freq)
{
	g_animateEngine.Init(freq);
	return TRUE;
}
CAnimateThread * DLL_EXP  CreateNewAnimateThread(ModelListener* pListener)
{
	g_animateEngine.AddAnimateListener(pListener);
	return g_animateEngine.AddNewAnimateThread();

}

void DLL_EXP  CreateNewFrame(CAnimateThread* pThread,CMemoryFrameSurface* pSurface,CAnimateParam* pParam)
{
		g_animateEngine.AddNewFrame(pParam->pThread,pParam->pSurface,(CAnimateParam*)pParam);
}

void DLL_EXP StartThread(CAnimateThread* pThread)
{	
	g_animateEngine.StartThread(pThread);
}

bool DLL_EXP AddNewFrame(CAnimateParam* pParam)
{
	CWidget* pWidget=(CWidget*)( pParam->pWidget);
	if(pWidget!=NULL)
	{
		if(pWidget->IsAnimating()==true)
		{
			return false;
		}else
		{
			pWidget->SetAnimatingFlag(true);
		}
	}
	g_animateEngine.AddNewFrame(pParam->pThread,pParam->pSurface,pParam);
	return true;
}





CAnimateFrame::CAnimateFrame(CMemoryFrameSurface* pSurface, CAnimateParam* pParam){
	this->m_pMemorySurface = pSurface;
	this->m_pAnimateParamter = pParam;
	m_pNext=NULL;
		m_pPrev=NULL;
		m_bStop=true;
		this->m_nFrameLayer=pParam->m_nAnimateFrameLayer ;
};
CAnimateEngine::~CAnimateEngine()
{
		CAnimateThread* pItem = (CAnimateThread*)(this->m_pHeadOfAnmateThreads);
		CAnimateThread* pDelete;
		while(pItem!=0)
		{
			pDelete = pItem;				
			pItem=pItem->m_pNext;
			delete pDelete;
		}
		::DeleteCriticalSection(&m_CriticalSection);
}
void CAnimateEngine::Init(int freq)
{
	this->m_animateListener.pListenerData = (this);
	this->m_animateListener.pfnListener = HandleAnimateInteractionEvent;
	this->m_animateListener.bEnableEventHandle=true;
	AttachTimerListener(*(this));
	StartMMTimer(freq);
}
void CAnimateEngine::AddRefreshArea(WRect* pRect)
{
	if(this->m_rectRefresh.dx==0)
	{
		m_rectRefresh=*pRect;
	}
	else
	{
		WRect rc=m_rectRefresh;
		UnionRect(&m_rectRefresh,&rc,pRect);
	}
}
void CAnimateEngine::HandleAnimateInteractionEvent(void *pUserData, ModelEvent *pEvent)
{
	CAnimateEngine* pEngine = (CAnimateEngine*)pUserData;
	switch (pEvent->evCode)
	{
	case WIDGET_EVENT_ANIMATE_THREAD_FINISH:
		{
			EVENT_ANIMATE_THREAD_FINISH_T* pEvt = (EVENT_ANIMATE_THREAD_FINISH_T*)(pEvent->dwParam);
					
			break;
		}
	}
			
}

	 CAnimateEngine::CAnimateEngine()
	{
		m_pHeadOfAnmateThreads=NULL;
		m_pTailOfAnmateThreads=NULL;
		m_nAnimateThreads=0;
		::InitializeCriticalSection(&m_CriticalSection);

	};
extern void RefreshScreenSurface(WRect* pRect=NULL);
void CAnimateEngine::Update(void* userData)
{
		this->m_rectRefresh.dx=0;
		 ::EnterCriticalSection(&m_CriticalSection);
		CAnimateThread* pItem = (CAnimateThread*)(this->m_pHeadOfAnmateThreads);
		CAnimateThread* pDelete = NULL;
		while(pItem!=0)
		{

			if(pItem->IsDelete()==TRUE)
			{
				if(pItem->m_pPrev!=NULL)
				{
					pItem->m_pPrev->m_pNext=pItem->m_pNext;
				}
				else
				{
					this->m_pHeadOfAnmateThreads=pItem->m_pNext;
				}

				if(pItem->m_pNext!=NULL)
				{
					pItem->m_pNext->m_pPrev=pItem->m_pPrev;
				}
				else
				{
					this->m_pTailOfAnmateThreads=pItem->m_pPrev;
				}		
				this->m_nAnimateThreads--;
				pDelete = pItem;
				pItem=pItem->m_pNext;
				delete pDelete;
				continue;
			}

			if(m_nAnimateThreads==0)
			{
				m_pTailOfAnmateThreads=NULL;
				m_pHeadOfAnmateThreads=NULL;

			}
			//Thread stop flag is false , call the pthread->update
			if(pItem->IsStop()==false)
			{
				pItem->Update(userData);
			}
			pItem=pItem->m_pNext;

		}
		 ::LeaveCriticalSection(&m_CriticalSection);

		
		 RefreshScreenSurface(&m_rectRefresh );

}




void CAnimateEngine::AddAnimateListener(ModelListener* listener)
{
	this->m_animateModel.AddListener(listener);
}

	void CAnimateThread::AddNewAnimateFrame(CMemoryFrameSurface* pSurface,CAnimateParam* pParam)
	{
		CAnimateFrame* pFrame = new CAnimateFrame(pSurface,pParam);
		//defautly all frames will be run one after one if user not set the frame layer explictly.
		if(pFrame->m_nFrameLayer ==0)
		{
			pFrame->m_nFrameLayer=++m_nMaxFrameLayers;
		}
		else
		{
			if(m_nMaxFrameLayers<pFrame->m_nFrameLayer)
			{
				m_nMaxFrameLayers=pFrame->m_nFrameLayer;

			}
		}
		//Record the fame layer in the animate paramter, then other frames can use this layer to put into.
		pParam->m_nAnimateFrameLayer=pFrame->m_nFrameLayer;
		switch (pParam->type )
		{
		case ANIMATE_SLIP:
			{
				CAnimateSlipController* pController = new CAnimateSlipController();
				pFrame->m_pController = pController;
				pController->m_pMemorySurface=pSurface;
				pSurface->SetUpdated(false);

				break;

			}
		case ANIMATE_FLIP:
			{
				CAnimateFlipController* pController = new CAnimateFlipController();
				pFrame->m_pController = pController;
				pController->m_pMemorySurface=pSurface;
				pSurface->SetUpdated(false);

				break;

			}
		case ANIMATE_ZOOM:
			{
				CAnimateZoomController* pController = new CAnimateZoomController();
				pFrame->m_pController = pController;
				pController->m_pMemorySurface=pSurface;

				break;

			}
		case ANIMATE_MOVE:
			{
				CAnimateMoveController* pController = new CAnimateMoveController();
				pFrame->m_pController = pController;
				pController->m_pMemorySurface=pSurface;

				break;

			}
		case ANIMATE_TICK_TICK:
			{
				CAnimateTickTick* pController = new CAnimateTickTick();
				pFrame->m_pController = pController;
				pController->m_pMemorySurface=pSurface;

				break;

			}
		case ANIMATE_SIN_MOVE_WIDGET:
			{
				CAnimateSinMoveWidgetController* pController = new CAnimateSinMoveWidgetController();
				pFrame->m_pController = pController;
				pController->m_pMemorySurface=NULL;

				break;

			}
		case ANIMATE_LINE_MOVE_WIDGET:
			{
				CAnimateLineMoveWidgetController* pController = new CAnimateLineMoveWidgetController();
				pFrame->m_pController = pController;
				pController->m_pMemorySurface=NULL;

				break;

			}
		case ANIMATE_3D_ROTATE_WIDGET:
			{
				CAnimate3DRotateWidgetController* pController = new CAnimate3DRotateWidgetController();
				pFrame->m_pController = pController;
				pController->m_pMemorySurface=NULL;

				break;

			}
		}
		if(this->m_pTailOfFrames==0)
		{
			m_pTailOfFrames=m_pHeadOfFrames=pFrame;
			m_pTailOfFrames->m_pNext=NULL;
			m_pTailOfFrames->m_pPrev=NULL;
		}
		else
		{
			this->m_pTailOfFrames->m_pNext=pFrame;
			pFrame->m_pPrev=this->m_pTailOfFrames;
			this->m_pTailOfFrames=pFrame;
		}
		this->m_nAnimateFrames++;
		pFrame->AddListener(&m_animateListener);
		pFrame->m_pController->SetParam(pParam);
	}
	void  CAnimateEngine::StartThread(CAnimateThread* pThread)
	{	
		if(pThread!=NULL)
		{
			pThread->Start();
		}
	}
	void CAnimateEngine::AddNewFrame(CAnimateThread* pThread,CMemoryFrameSurface* pSurface,CAnimateParam* pParam)
	{
		::EnterCriticalSection(&m_CriticalSection);
		pThread->AddNewAnimateFrame(pSurface,pParam);
		::LeaveCriticalSection(&m_CriticalSection);  
	}
	
	CAnimateThread* CAnimateEngine::AddNewAnimateThread()
	{
		CAnimateThread* pThread = new CAnimateThread();
		 ::EnterCriticalSection(&m_CriticalSection);
		if(this->m_pTailOfAnmateThreads==0)
		{
			m_pTailOfAnmateThreads=m_pHeadOfAnmateThreads=pThread;
			m_pTailOfAnmateThreads->m_pNext=NULL;
			m_pTailOfAnmateThreads->m_pPrev=NULL;
		}
		else
		{
			this->m_pTailOfAnmateThreads->m_pNext=pThread;
			pThread->m_pPrev=this->m_pTailOfAnmateThreads;
			this->m_pTailOfAnmateThreads=pThread;
		}
		::LeaveCriticalSection(&m_CriticalSection);   
		this->m_nAnimateThreads++;
		pThread->AddListener(&m_animateListener);
		return pThread;
	}


	BOOL CAnimateEngine::RemoveAnimateThread(CAnimateThread* pThread)
	{

		 ::EnterCriticalSection(&m_CriticalSection);
		CAnimateThread* pItem = (CAnimateThread*)(this->m_pHeadOfAnmateThreads);
		int idx=0;
		while(pItem!=0)
		{
			if(pItem==pThread)
			{
				if(pItem->m_pPrev!=NULL)
				{
					pItem->m_pPrev->m_pNext=pItem->m_pNext;
				}
				else
				{
					this->m_pHeadOfAnmateThreads=pItem->m_pNext;
				}

				if(pItem->m_pNext!=NULL)
				{
					pItem->m_pNext->m_pPrev=pItem->m_pPrev;
				}
				else
				{
					this->m_pTailOfAnmateThreads=pItem->m_pPrev;
				}		
				this->m_nAnimateThreads--;
			    ::LeaveCriticalSection(&m_CriticalSection);   
				return true;
			}
			pItem=pItem->m_pNext;

		}
		 ::LeaveCriticalSection(&m_CriticalSection);   
		return false;
		
	}
CAnimateThread::~CAnimateThread()
{
		CAnimateFrame* pItem = (CAnimateFrame*)(this->m_pHeadOfFrames);
		CAnimateFrame* pDelete;
		while(pItem!=0)
		{
			pDelete = pItem;				
			pItem=pItem->m_pNext;
			delete pDelete;
		}
		//TRACE("CAnimateThread Destroy\n");
}
CAnimateController::CAnimateController(void)
{
	m_bStarted=false;
	m_pMemorySurface=false;
	m_curAlpha=0;
	m_nSpeed=0;
	m_nAcceler=0;
	m_bStop=false;
}

CAnimateController::~CAnimateController(void)
{
}


int CAnimateController::Update(void)
{
	m_t++;
	return 0;
}

int CAnimateController::Stop(void)
{
	//m_pUserData->StopAnimate(this);
	m_bStarted=false;
	m_t=0;
	m_bStop=true;

	return 0;
}
void CAnimateController::SetMaxBackGround(WRect* pRect)
{
	this->m_rectMaxBK = *pRect;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////	Tick Tick Controller
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void CAnimateTickTick::Start()
{
	m_bStop=false;
	m_nCurTick=0;
}
 int CAnimateTickTick::Update(void)
{
	CTickTickAnimateParam* pParam = (CTickTickAnimateParam*)(this->m_pAnimateParam);
	m_nCurTick++;

	if(m_nCurTick>=pParam->nTicks )
	{
			EVENT_ANIMATE_FRAME_FINISH_T evt;
			evt.pAnimateParam=pParam;
			evt.pContorller=this;
			this->m_animateModel.NotifyEvent(WIDGET_EVENT_ANIMATE_FRAME_FINISH,(int)(&evt),sizeof(EVENT_ANIMATE_FRAME_FINISH_T));
			this->Stop();	
	}
	return 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////	Slip Controller
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void CAnimateSlipController::Start()
	{
		CAnimateController::Start();
		m_bStop=false;

		m_nCurDistance = 0;
		WRect backGroundRect;
		//备份当前FrameSurface运动范围内的屏幕背景，以便动画过程中使用
		//注意该备份只会做一次，不可能做第二次，因为第二次屏幕可能已经在动画过程中脏了。
		this->m_pMemorySurface->GetRect(&backGroundRect);
		CSlipAnimateParam* pParam = (CSlipAnimateParam*)(this->m_pAnimateParam);
		int cur_x_offset;
		int cur_y_offset;
		this->m_pMemorySurface->GetBitBltOffset(cur_x_offset,cur_y_offset);

		backGroundRect.x+=cur_x_offset;
		backGroundRect.y+=cur_y_offset;

		if(pParam->slip_type ==SLIP_DOWN)
		{
			backGroundRect.dy+=pParam->distance ;
		}
		if(pParam->slip_type ==SLIP_UP)
		{
			backGroundRect.dy+=pParam->distance ;
			backGroundRect.y-=pParam->distance ;
		}
		if(this->m_pMemorySurface->GetBestFitBackGround(&backGroundRect)==NULL)
		{
			CMemBitmap* pNewBk=this->m_pMemorySurface->AddBackGround(&backGroundRect);
			this->m_pMemorySurface->MergeBackGroundBmp();

			// will be destroyed when cotroller destroy
			this->AddGarbage(pNewBk);
		}
		
		//Add FrameSurface东DisplayManage, DisplayManage会在PainFinish为TRUE是将该Surface BitBlt到
		//屏幕。
		m_pMemorySurface->SetRemoveFlag(false);
		DisplayAddSurface(this->m_pMemorySurface);
		
		m_pMemorySurface->Save();		
	}

	//该函数由AnimateEngin按其时钟频率调用。
 int CAnimateSlipController::Update(void)
	{
		//very important
		CSlipAnimateParam* pParam = (CSlipAnimateParam*)(this->m_pAnimateParam);
		if(m_bStop==true) return 0;
		//pParam->nSpeed=pParam->nAcceler*m_t;

		int nSpeed = pParam->nSpeed+pParam->nAcceler*m_t;


		WRect invalidRect;
		this->m_pMemorySurface->GetRect(&invalidRect);
		
		int nCurXOffset ,nCurYOffset;
		this->m_pMemorySurface->GetBitBltOffset(nCurXOffset,nCurYOffset);
		int nLasYOffset = nCurYOffset;

		//calculate the invalid rect, 
		//计算当前的位移，以及Invalid Rect
		if(nSpeed<=1) nSpeed=1;


		m_nCurDistance+=nSpeed;

		int delta_speed=0;
		if(abs(m_nCurDistance)>=pParam->distance)
		{
			delta_speed = m_nCurDistance-pParam->distance ;
			m_bStop=true;
		}
		nSpeed -= delta_speed;

		if(pParam->slip_type ==SLIP_DOWN)
		{
			
			nCurYOffset +=nSpeed;
		
			invalidRect.y+=nLasYOffset;
			invalidRect.dy=nCurYOffset-nLasYOffset;
		}

		if(pParam->slip_type ==SLIP_UP)
		{
			nCurYOffset -=nSpeed;
			invalidRect.y=invalidRect.y+nLasYOffset+invalidRect.dy;
			invalidRect.dy=abs(nCurYOffset-nLasYOffset);
			invalidRect.y-=invalidRect.dy;
		}



		//Add a dirty rect with background,
		//即当前FrameSurface移出的部分需要用背景来重画，该Dirty Rect必须第一个被画，所以加到头部
		CMemBitmap* pMemBmp = this->m_pMemorySurface->GetBestFitBackGround(&invalidRect);
		ASSERT(pMemBmp!=NULL);
		this->m_pMemorySurface->RemoveBackGroundDirtyRect();		
		this->m_pMemorySurface->AddDirtyRect(&invalidRect,255,pMemBmp,true,false,true);



		if(abs(m_nCurDistance)>=pParam->distance)
		{
			m_bStop=true;
		}

		this->m_pMemorySurface->SetBitBltOffset(nCurXOffset,nCurYOffset);
		
		m_pMemorySurface->Restore();

		m_pMemorySurface->SetUpdated(true);		

		// Broadcast a Frame Finish Notification
		//通知监听者SlipFrame已经结束了
		//设置FrameSurface的删除标志为TRUE，DisplayManage会在下次访问该FrameSurface时从他的FrameSurface
		//列表中删除，不再访问。
		if(m_bStop==TRUE)
		{			
			EVENT_ANIMATE_FRAME_FINISH_T evt;
			evt.pAnimateParam=m_pAnimateParam;
			evt.pContorller =this;
			this->Stop();						
			//m_pMemorySurface->SetRemoveFlag(true);
			//m_pMemorySurface->SetDestroyFlag(true);
			this->m_animateModel.NotifyEvent(WIDGET_EVENT_ANIMATE_FRAME_FINISH,(int)(&evt),sizeof(EVENT_ANIMATE_FRAME_FINISH_T));
		}
		


	
		
		
		CAnimateController::Update();
		return 0;
		
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////		Flip Controller
///////////////////////////////////////////////////////////////////////////////////////////////////////
//在start函数中需要保存背景，应为在翻转过程中背景需要重画
//保存的背景大大小应等于动画过程中可能涉及的rect的所有背景,也就是说计算一个最大宽度和高度
//同时应为旋转应该围绕中心来旋转,所以中心坐标不会变化,就可以确定Rect的位置
void CAnimateFlipController::Start()
{
	CAnimateController::Start();
	CFlipAnimateParam* pParam = (CFlipAnimateParam*)(this->m_pAnimateParam);
	CWidget* pWidget=(CWidget*)(pParam->pWidget);
	this->m_nCurRadian=pParam->start_radian ;
	m_pMemorySurface->SetRemoveFlag(false);
	

	this->m_nOrigX=pParam->nOrigX;
	this->m_nOrigY=pParam->nOrigY;
	this->m_nOrigZ=pParam->nOrigZ;
	this->m_pMemorySurface->GetSurfaceBitmap()->GetBMP()->SetOrig(this->m_nOrigX,this->m_nOrigY,pParam->nOrigZ);
	m_pMemorySurface->GetRect(&m_origRect);

	//取得动画Widget的背景,
	//通过将Widget排除,重画Container来获得
	pWidget->m_bExcludeMe=true;	
	CMemoryFrameSurface* pBKSurface = pWidget->GetContainer()->CreateMemoryFrameSurface();
	pWidget->m_bExcludeMe=false;	
	pBKSurface->GetRect(&m_rectMaxBK);
	CMemBitmap* pBKBmp=pBKSurface->CopySurface(&m_rectMaxBK);
	delete pBKSurface;

	//保存当前的背景

	this->m_pMemorySurface->AddBackGround(&m_rectMaxBK,pBKBmp);

	m_lastRect=m_origRect;



	//扩大Surface
	this->m_pMemorySurface->SetRect(&m_rectMaxBK);
	this->m_pMemorySurface->SetViewPort(&m_rectMaxBK);
	m_pMemorySurface->Aggregate();
	m_pMemorySurface->Save();
	//DisplayAddSurface(this->m_pMemorySurface);	

	pWidget->SetAnimatingFlag(true);
		
	

}

//无需重画移出部分的背景,因为我已经在移动之前设置了一个最大范围的背景,目标图本身就包含背景,
//也就是说动画是在内存背景上绘制.
//在每次动画的时候我只需要计算目标图中动画部分得分位移

int CAnimateFlipController::Update(void)
{
	CAnimateController::Update();
	CFlipAnimateParam* pParam = (CFlipAnimateParam*)(this->m_pAnimateParam);
	float speed = pParam->nSpeed + pParam->nAcceler*m_t;


	this->m_nOrigZ+=20;

	if(this->m_nOrigZ>=-100) m_nOrigZ=-100;
	 
	CWidget* pWidget=(CWidget*)(pParam->pWidget);
	if(pParam->start_radian < pParam->max_radian)
	{
		if(speed<=0)
		{
			speed=abs(pParam->nAcceler);
		}
	}
	this->m_nCurRadian += speed;
	if(this->m_nCurRadian>=pParam->max_radian && speed>0)
	{
		speed-=(this->m_nCurRadian- pParam->max_radian);
		m_bStop=true;
		m_nCurRadian=pParam->max_radian;
	}

	if(this->m_nCurRadian<=pParam->max_radian && speed<0)
	{
		speed-=(this->m_nCurRadian- pParam->max_radian);
		m_bStop=true;
		m_nCurRadian=pParam->max_radian;
	}
/*
	float noDrawSin = abs(sin((float)m_nMaxNoDrawRadian*(PI/180)));
	float curSin=abs(sin((float)m_nCurRadian*(PI/180)));
	if(curSin>noDrawSin)
	{
		m_nCurRadian+=2*(90-m_nMaxNoDrawRadian);

	}

	if(this->m_nCurRadian>=pParam->max_radian)
	{
		m_nCurRadian=pParam->max_radian;
	}
	*/
	m_pMemorySurface->Restore();

	

	float x1,y1,x2,y2;
	
	this->m_pMemorySurface->GetSurfaceBitmap()->GetBMP()->SetOrig(this->m_nOrigX,this->m_nOrigY,this->m_nOrigZ);

	WRect paintRect;
	m_pMemorySurface->m_pSurfaceMemBitmap->m_pBmp->GetRotateYDestSize(x1,y1,x2,y2,
		m_nCurRadian,pParam->view_z);
	paintRect.dy=y2-y1+2;
	paintRect.dx=x2-x1+2;
	paintRect.x=m_origRect.x+x1-1;
	paintRect.y=m_origRect.y+y1-1;

	if(pParam->bEnableDaoYin==true)
	{
		paintRect.dy=paintRect.dy*(1+pParam->fDaoYinRatio)+1;

	}

	WRect unionRect;
	UnionRect(&unionRect,&paintRect,&m_lastRect);

    
	CMemBitmap* pMemBmp = this->m_pMemorySurface->GetBestFitBackGround(&(unionRect));
	
	if(pMemBmp!=NULL)
	{
	
		m_pMemorySurface->SetRect(&unionRect);
		
		if(pParam->bEnableDaoYin==true)
		{			
			m_pMemorySurface->m_pSurfaceMemBitmap->GetBMP()->m_bEnableReflect=true;
			m_pMemorySurface->m_pSurfaceMemBitmap->GetBMP()->m_fReflectRatio=pParam->fDaoYinRatio;
		}
		
		m_pMemorySurface->m_pSurfaceMemBitmap->GetBMP()->SetBackGround(pMemBmp->m_pBmp,
			abs(m_origRect.x-unionRect.x),
			abs(m_origRect.y-unionRect.y));	

		this->m_pMemorySurface->Rotate(this->m_nCurRadian,pParam->view_z );
		this->m_pMemorySurface->SetUpdated(true);

		CMemBitmap* pBmpForWidget = m_pMemorySurface->CopySurface(&unionRect);
		pWidget->SetAnimateBmp(pBmpForWidget);
		pWidget->InvalidateContent(0);

		

		m_lastRect=unionRect;

		delete pMemBmp;

		
	}
	if(m_bStop==TRUE)
	{			
		EVENT_ANIMATE_FRAME_FINISH_T evt;
		evt.pAnimateParam=m_pAnimateParam;
		evt.pContorller =this;
		this->Stop();						
		this->m_animateModel.NotifyEvent(WIDGET_EVENT_ANIMATE_FRAME_FINISH,(int)(&evt),sizeof(EVENT_ANIMATE_FRAME_FINISH_T));
		//pWidget->SetAnimatingFlag(false);
	}
		

	return 0;
}


void CAnimateZoomController::Start()
{
	CAnimateController::Start();
	CZoomAnimateParam* pParam = (CZoomAnimateParam*)(this->m_pAnimateParam);

	//计算占用的最大背景大小	
	WRect rc = this->m_pMemorySurface->m_pSurfaceMemBitmap->m_rect;
	WRect rcZoom;

	//保存Surface的原始大小，因为每次倍数计算都是在此基础上进行的
	m_origRect = rc;

	CRectOp::ZoomRect(&rc,&rcZoom,pParam->start_bei_shu);

	this->m_lastRect=rcZoom;

	//计算中心点，保持中心不变
	//只对放大的情况进行计算
	m_nCurBeiShu=1;
	if(pParam->bZoomIn==true)
	{
		
		CRectOp::ZoomRect(&rc,&rcZoom,pParam->nBeiShu);
	}
	
	if(this->m_pMemorySurface->GetBestFitBackGround(&rcZoom)==NULL)
	{
		CMemBitmap* pNewBk=this->m_pMemorySurface->AddBackGround(&rcZoom);
		this->m_pMemorySurface->MergeBackGroundBmp();

		// will be destroyed when cotroller destroy
		this->AddGarbage(pNewBk);
	}
	//聚合
	m_pMemorySurface->Aggregate();
	//保存原始位图
	m_pMemorySurface->Save();
	DisplayAddSurface(this->m_pMemorySurface);	
	
}


int CAnimateZoomController::Update(void)
{
	CAnimateController::Update();
	CZoomAnimateParam* pParam = (CZoomAnimateParam*)(this->m_pAnimateParam);
	//恢复原始位图
	m_pMemorySurface->Restore();

	float speed = pParam->nSpeed + pParam->nAcceler*m_t;
	
	this->m_nCurBeiShu += speed;
	if(this->m_nCurBeiShu>pParam->nBeiShu)
	{
		speed-=(this->m_nCurBeiShu- pParam->nBeiShu);
		this->m_nCurBeiShu = pParam->nBeiShu;
		m_bStop=true;
	}

	WRect rcBk;
	WRect rc = m_origRect;
	WRect rcZoom;



	float fBeiShu=pParam->start_bei_shu;

	//计算中心点，保持中心不变
	
	if(pParam->bZoomIn==true)
	{
		fBeiShu*=m_nCurBeiShu;
		CRectOp::ZoomRect(&rc,&rcZoom,fBeiShu);		
		rcBk=rcZoom;
	}
	else
	{
		fBeiShu/=m_nCurBeiShu;
		CRectOp::ZoomRect(&rc,&rcZoom,fBeiShu);
		rcBk=this->m_lastRect;
	}
	CMemBitmap* pBKMemBmp = m_pMemorySurface->GetBestFitBackGround(&rcBk);
	ASSERT(pBKMemBmp!=NULL);
	int tx,ty;
	tx=rcBk.x;
	ty=rcBk.y;
	pBKMemBmp->ScreenToClient(tx,ty);
	CEgBitmap* pCloneBmp = pBKMemBmp->CloneBmp(tx,ty,rcBk.dx,rcBk.dy);
	m_pMemorySurface->m_pSurfaceMemBitmap->m_pBmp->SetBackGround(pCloneBmp,rcZoom.x-rcBk.x,rcZoom.y-rcBk.y);
	
	m_pMemorySurface->Zoom(pParam->bZoomIn,fBeiShu);
	delete pCloneBmp;
	m_pMemorySurface->SetRect(&rcBk);
	m_pMemorySurface->RemoveRect(m_pMemorySurface->m_pSurfaceMemBitmap);
	m_pMemorySurface->AddDirtyRect(&rcBk);
	m_lastRect=rcZoom;

	m_pMemorySurface->SetUpdated(true);	
	if(m_bStop==TRUE)
	{			
		EVENT_ANIMATE_FRAME_FINISH_T evt;
		evt.pAnimateParam=m_pAnimateParam;
		evt.pContorller =this;
		this->Stop();						
		this->m_animateModel.NotifyEvent(WIDGET_EVENT_ANIMATE_FRAME_FINISH,(int)(&evt),sizeof(EVENT_ANIMATE_FRAME_FINISH_T));
	}
	

	return 0;
}


/**********************************************************************/
/*
	2007-12-02  ANIMATE MOVING Implementation  
	2007-12-03  Finished.
*/
/**********************************************************************/


void CAnimateMoveController::Start()
{
	CAnimateController::Start();
	CMoveAnimateParam* pParam = (CMoveAnimateParam*)(this->m_pAnimateParam);

	//计算占用的最大背景大小	
	this->m_pMemorySurface->GetRect(&m_origRect);
	
	WRect rcBk;

	rcBk.x=MIN(pParam->x1,pParam->x2);
	rcBk.y=MIN(pParam->y1,pParam->y2);
	rcBk.dx=MAX(pParam->x1,pParam->x2)+m_origRect.x+m_origRect.dx-rcBk.x;
	rcBk.dy=MAX(pParam->y1,pParam->y2)+m_origRect.y+m_origRect.dy-rcBk.y;

	WRect rcView;
	//在设置背景时需要考虑的运动过程中涉及的背景区域是否在当前view port之内，超出
	//当前view port的部分的背景是不需要重绘，因为我们根本就不会在当前的view port之外
	//绘图。
	//与View Port作交集计算，避免绘制当前View port之外的背景
	if(this->m_pMemorySurface->GetViewPort(&rcView)==true)
	{

		WRect rcTemp;
		if(IntersectRect(&rcTemp,&rcBk,&rcView)==false)
		{
			m_bStop=TRUE;
			return;
		}
		rcBk=rcTemp;
	}

	if(this->m_pMemorySurface->GetBestFitBackGround(&rcBk)==NULL)
	{
		CMemBitmap* pNewBk=this->m_pMemorySurface->AddBackGround(&rcBk);
		this->m_pMemorySurface->MergeBackGroundBmp();

		// will be destroyed when cotroller destroy
		this->AddGarbage(pNewBk);
	}
	//聚合
	m_pMemorySurface->Aggregate();
	DisplayAddSurface(this->m_pMemorySurface);	

	this->m_curX=pParam->x1;
	this->m_curY=pParam->y1;
	this->m_dirtyRect=m_origRect;
	this->m_dirtyRect.x=m_curX;
	this->m_dirtyRect.y=m_curY;
	
	
}


int CAnimateMoveController::Update(void)
{
	CAnimateController::Update();
	CMoveAnimateParam* pParam = (CMoveAnimateParam*)(this->m_pAnimateParam);
	//恢复原始位图
	//m_pMemorySurface->Restore();

	float speed = pParam->nSpeed + pParam->nAcceler*m_t;
	float offsetX,offsetY;

	if(pParam->x2>pParam->x1)
	{
		float k=(pParam->y2-pParam->y1)/(pParam->x2-pParam->x1);
		m_curX+=speed;
		m_curY+=k*speed;

		if(m_curX>pParam->x2)
		{
			m_curX=pParam->x2;
			m_bStop=true;
		}
	
	}
	else if(pParam->x2<pParam->x1)
	{
		float k=(pParam->y2-pParam->y1)/(pParam->x2-pParam->x1);
		m_curY+=k*speed;
		m_curX-=speed;
		if(m_curX<pParam->x2)
		{
			m_curX=pParam->x2;
			m_bStop=true;
		}
	}
	else if(pParam->x2==pParam->x1)
	{
		if(pParam->y2<pParam->y1)
		{
			m_curY-=speed;
			if(m_curY<pParam->y2)
			{
				m_curY=pParam->y2;
				m_bStop=true;
			}
		}
		if(pParam->y2>pParam->y1)
		{
			m_curY+=speed;
			if(m_curY>pParam->y2)
			{
				m_curY=pParam->y2;
				m_bStop=true;
			}
		}
	}

	//计算脏区，然后用背景重画
	WRect rcNext=this->m_origRect;
	rcNext.x=m_curX;
	rcNext.y=m_curY;

	WRect* pDirties;

	int nDirty = CRectOp::CalDirty(&m_dirtyRect,&rcNext,pDirties);
	this->m_pMemorySurface->RemoveBackGroundDirtyRect();	
	WRect* pRc=pDirties;
	while(nDirty>0)
	{	
		//在设置背景时需要考虑的运动过程中涉及的背景区域是否在当前view port之内，超出
		//当前view port的部分的背景是不需要重绘，因为我们根本就不会在当前的view port之外
		//绘图。
		//与View Port作交集计算，避免绘制当前View port之外的背景
		WRect rcView;
		if(this->m_pMemorySurface->GetViewPort(&rcView)==true)
		{
			WRect rcTemp;
			if(IntersectRect(&rcTemp,pRc,&rcView)==false)
			{
				pRc++;
				nDirty--;
				continue;
			}else
			{
				CLONE_RECT(&rcTemp,pRc);
			}
		}
		CMemBitmap* pMemBmp = this->m_pMemorySurface->GetBestFitBackGround(pRc);
		ASSERT(pMemBmp!=NULL);
		this->m_pMemorySurface->AddDirtyRect(pRc,255,pMemBmp,true,false,true);
		pRc++;
		nDirty--;
	}
	if(pDirties!=NULL)
	{
		EG_FREE(pDirties);
	}

	if(this->m_pMemorySurface->GetAlpha()<255)
	{
		WRect rcView;
		WRect rcTemp=rcNext;
		if(this->m_pMemorySurface->GetViewPort(&rcView)==true)
		{			
			if(IntersectRect(&rcTemp,&rcNext,&rcView)==false)
			{
				rcTemp=rcNext;
			}
		}
		CMemBitmap* pMemBmp = this->m_pMemorySurface->GetBestFitBackGround(&rcTemp);
		if(pMemBmp!=NULL)
		{
			this->m_pMemorySurface->AddDirtyRect(&rcTemp,255,pMemBmp,true,false,true);
		}
	}

	m_dirtyRect=rcNext;


	this->m_pMemorySurface->Move(m_curX,m_curY);

	this->m_pMemorySurface->SetUpdated(true);

	if(m_bStop==TRUE)
	{			
		EVENT_ANIMATE_FRAME_FINISH_T evt;
		evt.pAnimateParam=m_pAnimateParam;
		evt.pContorller =this;
		this->Stop();						
		this->m_animateModel.NotifyEvent(WIDGET_EVENT_ANIMATE_FRAME_FINISH,(int)(&evt),sizeof(EVENT_ANIMATE_FRAME_FINISH_T));
	}

	return 0;
}

void CAnimateSinMoveWidgetController::Start()
{
	CAnimateController::Start();
	CSinMoveWidgetAnimateParam* pParam = (CSinMoveWidgetAnimateParam*)(this->m_pAnimateParam);
	this->m_fCurAngle=pParam->fStartAngle;
	CWidget* pWidget=(CWidget*)(pParam->pWidget);
	ASSERT(pWidget!=NULL);	
	
	pWidget->GetRectAsScreen(&m_rectSrc);	
	

}
int CAnimateSinMoveWidgetController::Update(void)
{
	CAnimateController::Update();
	CSinMoveWidgetAnimateParam* pParam = (CSinMoveWidgetAnimateParam*)(this->m_pAnimateParam);
	this->m_fCurAngle=this->m_fCurAngle+pParam->nSpeed;
	if(this->m_fCurAngle>=pParam->fEndAngle)
	{
		this->m_fCurAngle=pParam->fEndAngle;
		m_bStop=true;
	}
	float sinA=pParam->fExtent*sin((this->m_fCurAngle/180)*PI);
	WRect rc;
	CWidget* pWidget=(CWidget*)(pParam->pWidget);
	pWidget->GetRectAsScreen(&rc);	
	ASSERT(pWidget!=NULL);
	WRect rcOld=rc;
	WRect rcU;		
	if(pParam->nMoveFlag=='Y')
	{		
		rc.y=m_rectSrc.y-sinA;;		
		UnionRect(&rcU,&rcOld,&rc);
		CContainerWidget* pContainer=pWidget->GetContainer();
		pContainer->ScreenToClient(&rc);		
		pWidget->SetRect(&rc);
		g_animateEngine.AddRefreshArea(&rcU);
		//pContainer->InvalidateRectAsScreen(&rcU);
	}
	if(pParam->nMoveFlag=='H')
	{		
		float fZoom=(float)(m_rectSrc.dy+sinA)/(float)m_rectSrc.dy;

		pWidget->Zoom(fZoom);

	}

	if(m_bStop==TRUE)
	{			
		EVENT_ANIMATE_FRAME_FINISH_T evt;
		evt.pAnimateParam=m_pAnimateParam;
		evt.pContorller =this;
		this->Stop();				
		pWidget->SetAnimatingFlag(false);
		this->m_animateModel.NotifyEvent(WIDGET_EVENT_ANIMATE_FRAME_FINISH,(int)(&evt),sizeof(EVENT_ANIMATE_FRAME_FINISH_T));

	}

	return 0;

}

void CAnimateLineMoveWidgetController::Start()
{
	CAnimateController::Start();
	CLineMoveWidgetAnimateParam* pParam = (CLineMoveWidgetAnimateParam*)(this->m_pAnimateParam);
	
	this->m_curX=pParam->x1;
	this->m_curY=pParam->y1;
	WRect rc;
	CWidget* pWidget=(CWidget*)(pParam->pWidget);
	pWidget->GetRectAsScreen(&rc);	
	rc.x=pParam->x1;
	rc.y=pParam->y1;
	pWidget->GetContainer()->ScreenToClient(&rc);
	pWidget->SetRect(&rc);

}
int CAnimateLineMoveWidgetController::Update(void)
{
	CAnimateController::Update();
	CLineMoveWidgetAnimateParam* pParam = (CLineMoveWidgetAnimateParam*)(this->m_pAnimateParam);


	float speed = pParam->nSpeed + pParam->nAcceler*m_t;
	float offsetX,offsetY;

	if(pParam->x2>pParam->x1)
	{
		float k=(pParam->y2-pParam->y1)/(pParam->x2-pParam->x1);
		m_curX+=speed;
		m_curY+=k*speed;

		if(m_curX>pParam->x2)
		{
			m_curX=pParam->x2;
			m_bStop=true;
		}
	
	}
	else if(pParam->x2<pParam->x1)
	{
		float k=(pParam->y2-pParam->y1)/(pParam->x2-pParam->x1);
		m_curY+=k*speed;
		m_curX-=speed;
		if(m_curX<pParam->x2)
		{
			m_curX=pParam->x2;
			m_bStop=true;
		}
	}
	else if(pParam->x2==pParam->x1)
	{
		if(pParam->y2<pParam->y1)
		{
			m_curY-=speed;
			if(m_curY<pParam->y2)
			{
				m_curY=pParam->y2;
				m_bStop=true;
			}
		}
		if(pParam->y2>pParam->y1)
		{
			m_curY+=speed;
			if(m_curY>pParam->y2)
			{
				m_curY=pParam->y2;
				m_bStop=true;
			}
		}
	}

	WRect rc;
	CWidget* pWidget=(CWidget*)(pParam->pWidget);
	pWidget->GetRectAsScreen(&rc);	
	ASSERT(pWidget!=NULL);
	WRect rcOld=rc;
	WRect rcU;		

	rc.y=m_curY;
	rc.x=m_curX;
	
	UnionRect(&rcU,&rcOld,&rc);
	CContainerWidget* pContainer=pWidget->GetContainer();
	pContainer->ScreenToClient(&rc);		
	pWidget->SetRect(&rc);
	pContainer->InvalidateRectAsScreen(&rcU);


	if(m_bStop==TRUE)
	{			
		EVENT_ANIMATE_FRAME_FINISH_T evt;
		evt.pAnimateParam=m_pAnimateParam;
		evt.pContorller =this;
		this->Stop();						
		this->m_animateModel.NotifyEvent(WIDGET_EVENT_ANIMATE_FRAME_FINISH,(int)(&evt),sizeof(EVENT_ANIMATE_FRAME_FINISH_T));
		pWidget->SetAnimatingFlag(false);
	}

	return 0;

}



void CAnimate3DRotateWidgetController::Start()
{
	CAnimateController::Start();
	C3DRotateWidgetAnimateParam* pParam = (C3DRotateWidgetAnimateParam*)(this->m_pAnimateParam);

	CWidget* pWidget=(CWidget*)(pParam->pWidget);
	cur_radian=pParam->start_radian;
	cur_z=pParam->start_z ;
}
int CAnimate3DRotateWidgetController::Update(void)
{
	CAnimateController::Update();
	C3DRotateWidgetAnimateParam* pParam = (C3DRotateWidgetAnimateParam*)(this->m_pAnimateParam);


	float rSpeed = pParam->nSpeed + pParam->nAcceler*m_t;
	float zSpeed = pParam->z_speed  + pParam->z_acceler*m_t;

	if(pParam->nSpeed>0 && pParam->nAcceler<0)
	{
		if(rSpeed<0)
		{
			rSpeed=2;
		}
	}

	
	if(pParam->nSpeed<0 && pParam->nAcceler>0)
	{
		if(rSpeed>0)
		{
			rSpeed=-2;
		}
	}
	cur_radian+=rSpeed;
	cur_z+= zSpeed;


	bool bRStop,bZStop;
	bRStop=bZStop=false;

	if(cur_radian>=pParam->max_radian && pParam->nSpeed >=0)
	{
		bRStop=true;
		cur_radian=pParam->max_radian;
	}

	if(cur_radian<=pParam->max_radian && pParam->nSpeed<0)
	{		
		bRStop=true;
		cur_radian=pParam->max_radian;
	}

	if(cur_z>=pParam->max_z  && pParam->z_speed>=0)
	{

			bZStop=true;

		cur_z=pParam->max_z;
	}


	if(cur_z<=pParam->max_z && pParam->z_speed<0)
	{

			bZStop=true;


		
		cur_z=pParam->max_z;
	}

	if(bZStop==true && bRStop==true)
	{
		cur_z=pParam->max_z;
		cur_radian=pParam->max_radian;
		m_bStop=true;
	}

	WPoint orig_pt;
	orig_pt.x=pParam->nOrigX;
	orig_pt.y=pParam->nOrigY;
	orig_pt.z=cur_z;

	CWidget* pWidget=(CWidget*)(pParam->pWidget);

	WRect rcPaint;
	pWidget->GetVisibleRect(&rcPaint);
	g_animateEngine.AddRefreshArea(&rcPaint);
	
	pWidget->RotateY(&orig_pt,cur_radian);
	pWidget->GetVisibleRect(&rcPaint);
	g_animateEngine.AddRefreshArea(&rcPaint);




	
	
	if(pParam->bUpdateScreen==true)
	{
DWORD dwTick = GetTickCount();				
		//pWidget->GetContainer()->InvalidateRectAsScreen(&rcPaint);
TRACE ("CBmpTransform::Cache Matrix transform took %0.03f seconds\n", 	 (GetTickCount() - dwTick) / 1000.0f);
	}


	if(m_bStop==TRUE)
	{			
		EVENT_ANIMATE_FRAME_FINISH_T evt;
		evt.pAnimateParam=m_pAnimateParam;
		evt.pContorller =this;
		this->Stop();						
		this->m_animateModel.NotifyEvent(WIDGET_EVENT_ANIMATE_FRAME_FINISH,(int)(&evt),sizeof(EVENT_ANIMATE_FRAME_FINISH_T));
		pWidget->SetAnimatingFlag(false);
	}

	return 0;

}


	CAnimateParam::CAnimateParam(){
		nSpeed=0;
		nFadeOption=0;
		pThread=NULL;
		pSurface=NULL;
		pWidget=NULL;
		bEnableDaoYin=false;
		bUpdateScreen=true;
		fDaoYinRatio=0.5;
		nAcceler=0;
		m_nAnimateFrameLayer=0;





	}
	CAnimateParam::~CAnimateParam()
	{
		if(this->pSurface!=NULL)
		{
			pSurface->SetRemoveFlag(true);				
			pSurface->SetDestroyFlag(true);
		}
	}


CAnimateThread::CAnimateThread()
	{

		m_pHeadOfFrames=NULL;
		m_pTailOfFrames=NULL;
		m_nAnimateFrames=0;
		m_pCurFrame=NULL;
		m_animateListener.pListenerData=this;
		m_animateListener.pfnListener=HandleAnimateInteractionEvent;
		this->m_animateListener.bEnableEventHandle=true;
		m_bStop = true;
		m_pNext=NULL;
		m_pPrev=NULL;
		m_bDelete = false;
		m_nCurFrameLayer=0;
		m_nMaxFrameLayers=0;

	}
	void CAnimateThread::DeleteMe()
	{
		m_bDelete =true;
	}
	BOOL CAnimateThread::IsDelete()
	{
		return m_bDelete;
	}



	void CAnimateThread::AddListener(ModelListener* pListener)
	{
		animateModel.AddListener(pListener);
	}

	 bool CAnimateThread::GetNextFrames()
	{
		m_nCurFrameLayer++;

		while(m_nCurFrameLayer<=this->m_nMaxFrameLayers)
		{			
			CAnimateFrame* pItem = (CAnimateFrame*)(this->m_pHeadOfFrames);		
			while(pItem!=0)
			{
				if(pItem->GetFrameLayer()==m_nCurFrameLayer)
				{
					m_curFrameList.add(pItem);
				}
				pItem=pItem->m_pNext;

			}
			if(m_curFrameList.m_nCount>0)
			{
				return true;
			}
			else
			{
				m_nCurFrameLayer++;

			}
		}
		return false;
	}

	void CAnimateThread::SetNextFrame(CAnimateFrame* pFrame)
	{
		m_pCurFrame=pFrame;
		pFrame->Start();
	}
	void CAnimateThread::HandleAnimateInteractionEvent(void *pUserData, ModelEvent *pEvent){
	}
	void CAnimateThread::Start()
	{
		this->m_nCurFrameLayer=0;
		if(m_pHeadOfFrames!=NULL && m_nMaxFrameLayers>0)
		{
			m_bStop=false;			
		}
	}

	void CAnimateThread::Stop()
	{
		m_bStop=true;
		
		EVENT_ANIMATE_THREAD_FINISH_T data;
		data.pThread=this;		
		this->animateModel.NotifyEvent(WIDGET_EVENT_ANIMATE_THREAD_FINISH,(int)(&data),sizeof(EVENT_ANIMATE_THREAD_FINISH_T));
		
		DeleteMe();
	}

	bool CAnimateThread::IsStop()
	{
		return m_bStop;
	}

	BOOL CAnimateThread::RemoveAnimateFrame(CAnimateFrame* pFrame)
	{
		CAnimateFrame* pItem = (CAnimateFrame*)(this->m_pHeadOfFrames);
		int idx=0;
		while(pItem!=0)
		{

			if(pItem==pFrame)
			{
				
				CAnimateFrame* pDelete = (CAnimateFrame*)pItem;

				if(pItem->m_pPrev!=NULL)
				{
					pItem->m_pPrev->m_pNext=pItem->m_pNext;
				}
				else
				{
					this->m_pHeadOfFrames=pItem->m_pNext;
				}

				if(pItem->m_pNext!=NULL)
				{
					pItem->m_pNext->m_pPrev=pItem->m_pPrev;
				}
				else
				{
					this->m_pTailOfFrames=pItem->m_pPrev;
				}		

				delete pDelete;

				this->m_nAnimateFrames--;
				return true;
			}
			pItem=pItem->m_pNext;

		

		}
		return false;
		
	}

	void CAnimateThread::Update(void* userData)
	{
		bool bStartNextLayerFrame=false;
		if(this->m_curFrameList.m_nCount==0)
		{
			if(this->GetNextFrames()==false)
			{
				this->Stop();
				return;
			}
			
			bStartNextLayerFrame=true;
			
		}
		if(this->m_curFrameList.m_nCount>0)
		{
			CObjItem* pItem = this->m_curFrameList.m_pHead;
			while(pItem!=NULL)
			{
				CAnimateFrame* pFrame = (CAnimateFrame*)(pItem->m_pData);
				if(bStartNextLayerFrame==true)
				{
					pFrame->Start();
				}
				//Delete the Frame if frame is stop
				if(pFrame->m_bStop==true)
				{
					CObjItem* pDelete = pItem;
					if(pItem->m_pPrevObj!=NULL)
					{
						pItem->m_pPrevObj->m_pNextObj =pItem->m_pNextObj ;
					}
					else
					{
						m_curFrameList.m_pHead=pItem->m_pNextObj;
					}

					if(pItem->m_pNextObj!=NULL)
					{
						pItem->m_pNextObj->m_pPrevObj =pItem->m_pPrevObj ;
					}
					else
					{
						m_curFrameList.m_pTail =pItem->m_pPrevObj ;
					}		
					m_curFrameList.m_nCount--;					
					pItem=pItem->m_pNextObj;
					this->RemoveAnimateFrame(pFrame);
					delete pDelete;
					continue;								
				}

				pFrame->Update(this);
				if(pFrame->m_pController->m_bStop==true)
				{
					pFrame->Stop();
				}			
				pItem=pItem->m_pNextObj;
			}			


		}	
	}