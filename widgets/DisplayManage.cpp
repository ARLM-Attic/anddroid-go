/*************************************************************************
eGUI Copyright (c) 2007 Xiao Wang Yang 
Author: Xiao Wang Yang
Email: sureone@gmail.com   http://code.google.com/p/easygui
------------------------------------------
Revision History:
2007-11-10			
	V0.1 BaseLine
2007-11-11			
	1.  Add a new feature for supporting transparent wiget
2007-11-14
	1.  Fix the bug of draw transparent img, for each img should map to two mask, one mask for forground,
		one mask for background, during draw, we first copy the img souce bmp into a temp DC, then mask it
		with background mask , then 
	2. Format Draw
	
		Format of mutil_bmp_draw_cmd
		direction,num,x,y,w,h,repeat_num

		direction = the dierction of draw , H means horizon, V mean vertical
		num = the num of "x,y,w,h,repeat_num"
		x = the x_offset in the resource bmp
		y = the y_offset in the resource bmp
		w = the width of bmp slot
		h = the hight of bmp slot
		repeat_num = draw repeat times
2007-11-17
	1.  Add a animate engine in charge of the animation management.
	2.  增加了一个CFrameSurface, 该Surface只是个内存影像,每次绘图之前即(BeginPaint)须Create
		一个FrameSurface,当绘图结束时候即(EndPaint)再将该FrameSurface添加到CDisplayManage管理的
		FrameSurface列表。然后由AnimateEngine每隔一定的时间将FrameSurface列表BitBlt到实际
		的屏幕CDC中，完成绘图的显示。	
2007-12-10
	1. 在FrameSurface被添加到Surface列表中时，直接调用FlushSurface函数来提高绘图相应速度。
	2. 对于透明Widget,不可以在原来的Surface位图上绘制,这样会破坏背景,
		我们需要新建一个新的内存位图暂时替换Surface原有的位图,在该Widget绘制
		结束后恢复原始位图.

*************************************************************************/

#include "DisplayManage.h"

#include "memdc.h"
#include "RectOp.h"
#include "Container.h"
#include "drawhtml.h"

CMemBitmap::CMemBitmap(CDC* pScreen,WRect* pRect){
	m_rect.dx=m_rect.dy=0;
	m_pBmp= new CEgBitmap();
	//TRACE(" A new CDC allocated = %x \n",(int)(m_pCDC));


	//if(m_pBmp->CreateCompatibleBitmap(pScreen,pRect->dx,pRect->dy)==FALSE)
	{
		HDC hdc = GetDC(NULL);		
		HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc, pRect->dx,pRect->dy);

		m_pBmp->Attach(hbmSrc);
		::ReleaseDC(NULL, hdc);
	}


	//ASSERT(m_pBmp->m_hObject!=NULL);
	CLONE_RECT(pRect,(&m_rect));
	m_pScreen=pScreen;

	m_nAlphaValue=255;
	m_pNext=NULL;
	m_pPrev=NULL;
	m_bRemoveFlag=false;		
	this->m_pPaintDC=NULL;
};
CDC* CMemBitmap::CreateMemDC()
{

	CDC* pDC=m_pPaintDC;

	if((int)pDC==0x0101C4E0)
	{
		TRACE("Got it\n");
	}

	if(m_pPaintDC!=NULL && m_bPaindDcRelease==true)
	{
		m_bPaindDcRelease=false;
		pDC->SelectObject(this->GetBMP());
		return pDC;
	}

	if(m_pPaintDC==NULL)
	{

		m_pPaintDC=new CDC();
		pDC=m_pPaintDC;
		m_bPaindDcRelease=false;		
		

	}else
	{	
			pDC=new CDC();
	}
	

	//TRACE("=MEM= allocate CDC @ %x\n",(unsigned int)pDC);
	pDC->CreateCompatibleDC(this->m_pScreen);

	//pDC->SetBkColor(TRANSPARENT);
	
	pDC->SelectObject(this->GetBMP());
	

	return pDC;
}

void CMemBitmap::CopyBitmap(CMemBitmap* pSrcBmp)
{
	CDC* pDCSrc = pSrcBmp->CreateMemDC();
	CDC* pDCTar=this->CreateMemDC();

	WRect rc;
	GetRect(&rc);
	pSrcBmp->ScreenToClient(&rc);
	pDCTar->BitBlt(0,0,rc.dx,rc.dy,pDCSrc,rc.x,rc.y,SRCCOPY);


	pSrcBmp->ReleaseMemDC(pDCSrc);
	this->ReleaseMemDC(pDCTar);


}
void CMemBitmap::ReleaseMemDC(CDC* pDC)
{

	pDC->SelectObject((HGDIOBJ)NULL);
	if(pDC==m_pPaintDC)
	{
		m_bPaindDcRelease=true;

	}

	if(pDC!=m_pPaintDC)
	{
		pDC->DeleteDC();
		delete pDC;
		//TRACE("=MEM= free CDC @ %x\n",(unsigned int)pDC);
	}


}
bool IsDirectScreenDraw()
{
	return true;
};
CWidgetResource::~CWidgetResource()
{
#ifdef USE_CXIMAGE
#else
	CBitmap* bmp = (CBitmap*)(this->resource);
	bmp->DeleteObject();
	delete bmp;

	if(this->bmp_mask1!=NULL)
	{
			CBitmap* bmp = (CBitmap*)(this->bmp_mask1);
			bmp->DeleteObject();
			delete bmp;

	}
	if(this->bmp_mask2!=NULL)
	{
			CBitmap* bmp = (CBitmap*)(this->bmp_mask2);
			bmp->DeleteObject();
			delete bmp;

	}
#endif
}
CWidgetResource* CDisplayManage::GetResource(char* res_id)
{

	CWidgetResource* pItem=this->m_pResourceHead ;

	while(pItem!=0)
	{
		if(pItem->isMe(res_id)==TRUE)
		{
			return pItem;
			
			break;
		}
		pItem=pItem->pNext;

	}

	return NULL;
}
CWidgetResource* CDisplayManage::AddResource(char* res_id,void* resource,void* bmp_mask1,void* bmp_mask2)
{

	CWidgetResource* p = new CWidgetResource(res_id,resource);
	p->bmp_mask1=bmp_mask1;
	p->bmp_mask2=bmp_mask2;
	p->resource=resource;
	p->pNext=NULL;
	p->pPrev=NULL;

		if(this->m_pResourceTail==0)
		{
			m_pResourceTail=m_pResourceHead=p;
			m_pResourceTail->pNext=NULL;
			m_pResourceTail->pPrev=NULL;
		}
		else
		{
			this->m_pResourceTail->pNext=p;
			p->pPrev=this->m_pResourceTail;
			this->m_pResourceTail=p;
		}
		this->m_nResourceCnt++;




	return p;
}



  void OUT_AREA_RECT(int& x1,int& y1,int& dx,int& dy,int h,int w) 
  {

	  int fx1=x1;
	  int fy1=y1;
	  h--;
	  w--;

	  int fx2=fx1+dx;
	  int fy2=fy1+dy;


	  	fx1=MAX(fx1,0);
		fx2=MAX(fx2,0);
		
		fx1=MIN(fx1,w);
		fx2=MIN(fx2,w);

		fy1=MAX(fy1,0);
		fy1=MIN(fy1,h);
		fy2=MAX(fy2,0);
		fy2=MIN(fy2,h);


		dx=abs(fx2-fx1);
		dy=abs(fy2-fy1);

	  x1=fx1;
	  y1=fy1;

	//	dx=MIN(x1+dx,w-x1);
	//	dy=MIN(x1+dy,h-y1);
	  
  }

void OUT_AREA_LINE(int& x1,int& y1,int& x2,int& y2,int h,int w) 
  {

	float fx1=x1;
	float fx2=x2;
	float fy1=y1;
	float fy2=y2;
	float k,b;

	float fx0,fxh,fy0,fyw;
    h--;
	w--;
	//straight line

	if(fx1==fx2 || fy1==fy2)
	{
		fx1=MAX(fx1,0);
		fx2=MAX(fx2,0);
		
		fx1=MIN(fx1,w);
		fx2=MIN(fx2,w);

		fy1=MAX(fy1,0);
		fy1=MIN(fy1,h);
		fy2=MAX(fy2,0);
		fy2=MIN(fy2,h);
	}

	
	//xie xian
	if(fx1!=fx2 && fy1!=fy2)
	{
		k=(fy2-fy1)/(fx1-fx2);
		b=fy2-k*fx2;
		fy0=b;
		fyw=k*w+b;

		fx0=(-b)/k;
		fxh=(h-b)/k;

		if(fx1<0)
		{
			fx1=0;
			fy1=fy0;
		}

		if(fx2<0)
		{
			fx2=0;
			fy2=fy0;
		}

		
		if(fx1>w)
		{
			fx1=w;
			fy1=fyw;
		}

		if(fx2>w)
		{
			fx2=0;
			fy2=fyw;
		}

		if(fy1<0)
		{
			fy1=0;
			fx1=fx0;
		}

		if(fy2<0)
		{
			fy2=0;
			fx2=fx0;
		}

		if(fy1>h)
		{
			fy1=h;
			fx1=fxh;
		}

		if(fy2>h)
		{
			fy2=h;
			fx2=fxh;
		}


		

	}


	if(fx1<0 || fx2<0 || fy1<0 || fy2 <0)
	{
		fx1++;
	}	
	x1=fx1;
	x2=fx2;
	y1=fy1;
	y2=fy2;

  }

CDisplayManage* g_pDisplayManage=NULL;
void RefreshScreenSurface(WRect* pRefreshRect=NULL)
{
	if(g_pDisplayManage==NULL) return;
	g_pDisplayManage->FlushSurface();
	if(pRefreshRect!=NULL && pRefreshRect->dx>0)
	{
		g_pDisplayManage->InvalidateScreenRectFromDisplay(pRefreshRect);
	}
}
void DisplayAddSurface(CFrameSurface* pSurface)
{
	if(g_pDisplayManage==NULL) return;

		 DWORD  curThreadId = GetCurrentThreadId();
		//TRACE("Current Thread ID = %d\n", curThreadId);
	g_pDisplayManage->AddFrameSurface(pSurface);

	
}
CDisplayManage::CDisplayManage(void)
{
}
CDisplayManage::CDisplayManage(HWND hWnd,int cx,int cy)

{


		//Create the Font
	LOGFONT lf = {0};
	lf.lfHeight = -13;
	lf.lfWeight = FW_NORMAL;
	
	_tcscpy(lf.lfFaceName,"Tahoma");
	m_font.CreateFontIndirect(&lf);

	m_pHeadOfFrameSurfaces=NULL;
	m_pTailOfFrameSurfaces=NULL;
	m_nFrameSurfaces=0;
	::InitializeCriticalSection(&m_CriticalSection);

		g_pDisplayManage=this;
		CDC* pDC=new CDC();
		pDC->Attach(GetWindowDC(hWnd));
		m_nBackGroundColor=RGB(255,255,255);

		m_pScreen=pDC;
		m_pScreen->SelectObject(&m_font);
		

		m_pScreenBuffer=new CScreenBuffer(pDC,cx,cy);
		m_pScreenBuffer->SetFont(&m_font);
		this->m_pResourceHead=NULL;
		this->m_pResourceTail=NULL;
		this->m_nResourceCnt=0;
		m_bDisplayReady=false;

#ifdef USE_FREETYPE
		//Initialize the free type libaray and default free type font face.
		bool error = FT_Init_FreeType( &freetype_library );
	    if ( error )
		{
		    TRACE("... an error occurred during library initialization ...\n");
		}

		error = FT_New_Face( freetype_library,
							"D:\\code\\wq_project\\svn_src\\build-inter\\GameClient\\resource\\arial.ttf",
							0,
							&default_ft_face );
		if ( error == FT_Err_Unknown_File_Format )
		{
			TRACE("... the font file could be opened and read, but it appears\n");

			TRACE("...that its font format is unsupported\n");
		}
		else if ( error )
		{
			TRACE("... another error code means that the font file could not\n");
			TRACE("... be opened or read, or simply that it is broken...\n");
		}
#if(0)
		error = FT_Select_Charmap(
            default_ft_face,               /* target face object */
             FT_ENCODING_GB2312); /* encoding           */
	    if ( error )
		{
		    TRACE("... an error occurred during FT_Select_Charmap ...\n");
		}
#endif

#endif

	
}
#ifdef USE_FREETYPE
bool CFrameSurface::FT_draw_str(const w_char* pStr,int count,WRect* pRc)
{

	FT_Face& face = (this->m_pDisplayManage->default_ft_face); 
	int error ;

	error = FT_Set_Char_Size(
            face,    /* handle to face object           */
            0,       /* char_width in 1/64th of points  */
            0,   /* char_height in 1/64th of points */
            300,     /* horizontal device resolution    */
            300 );   /* vertical device resolution      */

	if(error!=0)
	{

		TRACE("FT_Set_Char_Size error\n");
	}



	error = FT_Set_Pixel_Sizes(
            face,   /* handle to face object */
            0,      /* pixel_width           */
            32 );   /* pixel_height          */

	if(error!=0)
	{

		TRACE("FT_Set_Pixel_Sizes error\n");
	}

	FT_Matrix     matrix; 
	float angle         = ( 25.0 / 360 ) * PI * 2;

	matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

	FT_Vector     pen;

	pen.x=0;
	pen.y=0;



	
	CEgBitmap bmp;
	bmp.CreateCompatibleBitmap(this->GetScreenCDC(),pRc->dx,pRc->dy);

	WRect rcT=*pRc;
	FT_GlyphSlot slot = face->glyph;
	for(int i=0;i<count;i++)
	{
		/* set transformation */
	   // FT_Set_Transform( face, &matrix, &pen );
		unsigned long charCode=(unsigned long)(*(pStr++));

	
		/* load glyph image into the slot (erase previous one) */
	    //error = FT_Load_Char( face,charCode, FT_LOAD_RENDER );
		FT_UInt glyph_index = FT_Get_Char_Index( face, charCode );

		if(glyph_index==0)
		{
			TRACE("undefined character code\n");
		}


		FT_Int32  load_flags = FT_LOAD_DEFAULT ;
		error = FT_Load_Glyph(
				face,          /* handle to face object */
				glyph_index,   /* glyph index           */
				load_flags );  /* load flags, see below */

		if(error!=0)
		{
			TRACE("FT_Load_Glyph error\n");
		}


		FT_Render_Mode  render_mode=FT_RENDER_MODE_NORMAL;
		error = FT_Render_Glyph( face->glyph,   /* glyph slot  */
								render_mode ); /* render mode */

		if(error!=0)
		{
			TRACE("FT_Render_Glyph error\n");
		}

		if(error==0)
		{
				bmp.RenderFTGlyph(
					&slot->bitmap,
					slot->bitmap_left,
					rcT.dy-slot->bitmap_top, pen);
		}		

		 /* increment pen position */
		 pen.x += slot->advance.x/64;
		 pen.y += slot->advance.y/64;

	}
	this->DrawPureBmp(&bmp,pRc);
	bmp.DeleteObject();

	return true;
}
#endif

void CFrameSurface::DrawPureBmp(CEgBitmap* pBmp,WRect* pRect)
{	

	CDC* pBmpDC = new CDC();
	pBmpDC->CreateCompatibleDC(this->GetScreenCDC());	
	pBmpDC->SelectObject(pBmp);
	
	WRect wrc=*pRect;
	CDC* surfaceCDC=this->m_pSurfaceMemBitmap->CreateMemDC();
	this->m_pSurfaceMemBitmap->ScreenToClient(&wrc);
	ASSERT(surfaceCDC->BitBlt(wrc.x,wrc.y,wrc.dx,wrc.dy,pBmpDC,0,0,SRCCOPY)==TRUE);
	m_pSurfaceMemBitmap->ReleaseMemDC(surfaceCDC);
	pBmpDC->DeleteDC();
	this->AddDirtyRect(pRect);
	delete pBmpDC;
	
}
	void CDisplayManage::SetDisplayReady(bool bReady)
	{
		m_bDisplayReady=bReady;
	}
	bool CDisplayManage::IsDisplayReady()
	{
		return m_bDisplayReady;
	}
CDisplayManage::~CDisplayManage(void)

{
	g_pDisplayManage=NULL;
#ifdef USE_FREETYPE
	FT_Done_Face(default_ft_face );
	FT_Done_FreeType(freetype_library);
#endif

	CWidgetResource* pRes = (CWidgetResource*)(this->m_pResourceHead );		
	while(pRes!=0)
	{
		CWidgetResource* pDelete = pRes;
		pRes=pRes->pNext;
		delete pDelete;

	}
	CFrameSurface* pItem = (CFrameSurface*)(this->m_pHeadOfFrameSurfaces);		
	while(pItem!=0)
	{
		CFrameSurface* pDelete = pItem;
		pItem=pItem->m_pNext;
		delete pDelete;

	}

	if(m_pScreen!=NULL)
	{
		m_pScreen->DeleteDC();
		delete m_pScreen;
	}

	if(m_pScreenBuffer!=NULL)
	{
		delete m_pScreenBuffer;
	}



}
void CFrameSurface::DrawText(const uint16* pString,   // address of string to draw
                    int     nCount,     // string length, in characters
                    WRect* pRc,     // address of structure with formatting dimensions
                    UINT    uFormat     // text-drawing flags
					)
{

#if 0
#ifndef USE_FREETYPE 
			CDC* pSurfaceCDC=this->m_pSurfaceMemBitmap->CreateMemDC();
			WRect wrc=*pRc;
			this->m_pSurfaceMemBitmap->ScreenToClient(&wrc);
			CRect rc;
			WRECT_TO_CRECT(&wrc,(&rc));
			pSurfaceCDC->SetBkMode(TRANSPARENT);
			pSurfaceCDC->SelectObject(m_pScreenBuffer->GetFont());

			DrawHTML(
                    pSurfaceCDC->m_hDC ,        // handle of device context
                    pString,   // address of string to draw
                        nCount,     // string length, in characters
                        (LPRECT)(&rc),     // address of structure with formatting dimensions
                        uFormat     // text-drawing flags
					);
			WRect rcClip;
			this->GetClipRect(&rcClip);
			WRect rcDirty;
			IntersectRect(&rcDirty,&rcClip,pRc);
				
			AddDirtyRect(&rcDirty);
			m_pSurfaceMemBitmap->ReleaseMemDC(pSurfaceCDC);
#else

			FT_draw_str(pString,nCount,pRc);

			
#endif
#endif


			
}

void CDisplayManage::GetClipRect(  WRect* rc){
	CFrameSurface* pSurface = this->GetCurFrameSurface();
	pSurface->GetClipRect(rc);
};

void CDisplayManage::SetClipRect( WRect* rc){
	CFrameSurface* pSurface = this->GetCurFrameSurface();
	pSurface->SetClipRect(rc);
};
void CFrameSurface::GetClipRect(  WRect* rc){
	CLONE_RECT(&m_clipRect,rc);
};

void CFrameSurface::SetClipRect( WRect* rc){
	CLONE_RECT(rc,&m_clipRect);
};


//#define DISABLE_MEM_DC
void CFrameSurface::MergeDirtyRect()
{
	this->m_pDirtyTable->MergeNeighborRect();
}
CFrameSurface* CDisplayManage::BeginPaint(WRect* pPaintRc)
{
			CFrameSurface* pSurface=CreateNewOfflineSurface(pPaintRc);
			pSurface->BeginPaint();
			
			return pSurface;

}
void CDisplayManage::EndPaint(CFrameSurface* pSurface)
{		
		
		pSurface->EndPaint();
		pSurface->MergeDirtyRect();
		this->AddFrameSurface(pSurface);
		if(IsDirectScreenDraw()==true)
		{			
			g_pDisplayManage->FlushSurface();
		}

}

CDC* CFrameSurface::GetScreenCDC()
{
	return this->m_pScreenBuffer->GetScreenCDC();
}

CMemoryFrameSurface* CDisplayManage::BeginMemoryPaint(WRect* pPaintRc)
{
			CMemoryFrameSurface* pSurface=CreateNewMemorySurface(pPaintRc);
			pSurface->BeginPaint();
			return pSurface;

}
void CDisplayManage::EndMemoryPaint(CMemoryFrameSurface* pSurface)
{		
		pSurface->EndPaint();
		pSurface->MergeDirtyRect();
		//this->AddFrameSurface(pSurface);
}


#ifdef USE_CDC


#endif
	int CFrameSurface::MoveTo(int x, int y)
	{
		ptOld.x=x;
		ptOld.y=y;



		this->m_pSurfaceMemBitmap->ScreenToClient(x,y);

		
		CDC* pSurfaceDC = m_pSurfaceMemBitmap->CreateMemDC();
			pSurfaceDC->MoveTo(x,y);
		m_pSurfaceMemBitmap->ReleaseMemDC(pSurfaceDC);

		return 0;
	}


	int CFrameSurface::LineTo(int xo, int yo,int color)
	{


		CDC* pSurfaceDC = m_pSurfaceMemBitmap->CreateMemDC();
		//int c=SDL_MapRGB(screen->format, (color>>16)&0xFF,(color>>8)&0xFF,(color)&0xFF);

	    int oldx=ptOld.x;
		int oldy=ptOld.y;
		int x=xo;
		int y=yo;

		this->m_pSurfaceMemBitmap->ScreenToClient(oldx,oldy);
		this->m_pSurfaceMemBitmap->ScreenToClient(x,y);



		CPen pen;
		pen.CreatePen(PS_SOLID,1,color);
		CPen* oldpen=pSurfaceDC->SelectObject(&pen);
		pSurfaceDC->MoveTo(oldx,oldy);
		pSurfaceDC->LineTo(x,y);
		pSurfaceDC->SelectObject(oldpen);
		pen.DeleteObject();


		WRect rc;
		rc.x=MIN(ptOld.x,xo);
		rc.y=MIN(ptOld.y,yo);
		rc.dx=MAX(ptOld.x,xo)-MIN(ptOld.x,xo)+1;
		rc.dy=MAX(ptOld.y,yo)-MIN(ptOld.y,yo)+1;		
		WRect rcClip;
		this->GetClipRect(&rcClip);
		WRect rcDirty;
		IntersectRect(&rcDirty,&rcClip,&rc);
			
	   AddDirtyRect(&rcDirty);

	    
		ptOld.x=xo;
		ptOld.y=yo;
		m_pSurfaceMemBitmap->ReleaseMemDC(pSurfaceDC);
		return 0;
	}

	int CFrameSurface::FillRect(WRect* pRect, int color)
	{




		CDC* pSurfaceDC = m_pSurfaceMemBitmap->CreateMemDC();
		WRect rc=*(pRect);
		this->m_pSurfaceMemBitmap->ScreenToClient(&rc);
	
		pSurfaceDC->FillSolidRect(rc.x,rc.y,rc.dx,rc.dy,color);
		

		WRect rcClip;
		this->GetClipRect(&rcClip);
		WRect rcDirty;
		IntersectRect(&rcDirty,&rcClip,pRect);
			
	   AddDirtyRect(&rcDirty);
	   m_pSurfaceMemBitmap->ReleaseMemDC(pSurfaceDC);



		return 0;
	}

	/*
		Format of mutil_bmp_draw_cmd
		direction,num,x,y,w,h,repeat_num

		direction = the dierction of draw , H means horizon, V mean vertical
		num = the num of "x,y,w,h,repeat_num"
		x = the x_offset in the resource bmp
		y = the y_offset in the resource bmp
		w = the width of bmp slot
		h = the hight of bmp slot
	*/
	typedef struct{
		int x;
		int y;
		int w;
		int h;
		int repeat_num;
	}BATCH_BMP_DRAW_CMD;

	void CFrameSurface::DrawImage(IMAGE_HANDLE handle,WRect* pFullImageRect,bool reverse,char* batch_bmp_draw_cmd)
	{
		
		WRect drawRc;
		WRect clipRc;
		WRect fullRc;
		CLONE_RECT(pFullImageRect,(&fullRc));

		GetClipRect(&clipRc);
		
		if(IntersectRect(&drawRc,&fullRc,&clipRc)==false)
		{
			return;
		}

		
		int num_cmd;
		char draw_direction;				
		char value[16];
		memset(value,0,sizeof(value));
		char* p1=batch_bmp_draw_cmd+2;
		char* p2=NULL;

		bool bMasked=false;
		bool bSelfMask=false;
		if(strstr(batch_bmp_draw_cmd,"BMP_MASK:")>0)
		{
			bMasked=true;
		}

		if(strstr(batch_bmp_draw_cmd,"SELF_MASK")>0)
		{
			bSelfMask=true;
		}

		draw_direction=batch_bmp_draw_cmd[0];	
		p2= strchr(p1,',');
		strncpy(value,p1,(p2-p1));
		num_cmd=atoi(value);
		
	
		
		if(num_cmd>0)
		{
			int i =0;

			while(i<num_cmd)
			{
				BATCH_BMP_DRAW_CMD cmd;
				//get x
				p1=p2+1;
				p2=	strchr(p1,',');
				memset(value,0,sizeof(value));
				strncpy(value,p1,(p2-p1));
				cmd.x=atoi(value);

				//get y
				p1=p2+1;
				p2=	strchr(p1,',');
				memset(value,0,sizeof(value));
				strncpy(value,p1,(p2-p1));
				cmd.y=atoi(value);

				//get w
				p1=p2+1;
				p2=	strchr(p1,',');
				memset(value,0,sizeof(value));
				strncpy(value,p1,(p2-p1));
				cmd.w=atoi(value);

				//get h
				p1=p2+1;
				p2=	strchr(p1,',');
				memset(value,0,sizeof(value));
				strncpy(value,p1,(p2-p1));
				cmd.h=atoi(value);

				//get repeat_num
				p1=p2+1;
				p2=	strchr(p1,',');
				memset(value,0,sizeof(value));
				strncpy(value,p1,(p2-p1));
				cmd.repeat_num=atoi(value);

				while(cmd.repeat_num>0)
				{
					
					fullRc.dx=cmd.w;
					fullRc.dy=cmd.h;

					WRect curRc;

					if(IntersectRect(&curRc,&fullRc,&drawRc)==true)
					{
						DrawImage( handle,&fullRc,&curRc,bMasked,bSelfMask,reverse,cmd.x,cmd.y,cmd.w,cmd.h);
					}
					 if(draw_direction=='H')
						fullRc.x+=cmd.w;					
					if(draw_direction=='V')
						fullRc.y+=cmd.h;
					 cmd.repeat_num--;

				}
				i++;
			}
		}	
	}


	// CtestbmpView 绘制


	// Frame Surface 的Size与Clip Rect一样大也就是以Clip Rect的左上点为坐标原点
	// 

	void CFrameSurface::DrawSelfMaskBmp(CDC* pTargetDC,CEgBitmap* pSrcBmp,int sx,int sy,int sw,int sh,int tx,int ty,int tw,int th)
	{
		HBITMAP hMask = pSrcBmp->CreateBitmapMask(-1,sx,sy,sw,sh);

		CDC dcMem;
		dcMem.CreateCompatibleDC(this->GetScreenCDC());
		dcMem.SelectObject(hMask);

		pTargetDC->BitBlt(tx,ty,sw,sh,&dcMem,0,0,SRCAND);


		CDC dcMem2;
		dcMem2.CreateCompatibleDC(this->GetScreenCDC());
		dcMem2.SelectObject(pSrcBmp);

		pTargetDC->BitBlt(tx,ty,sw,sh,&dcMem2,0,0,SRCPAINT);
		dcMem.DeleteDC();
		dcMem2.DeleteDC();
		::DeleteObject(hMask);





	}
	void CFrameSurface::DrawImage(IMAGE_HANDLE handle,WRect* fullRc,WRect* inClipRc,bool bMasked,bool bSelfMask,bool reverse,int x,int y, int w,int h)
	{
		static int cntDraw=0;

		CDC* pSurfaceDC=this->m_pSurfaceMemBitmap->CreateMemDC();

		CWidgetResource* resource=(CWidgetResource*)(handle);
		CEgBitmap* imgBmp=(CEgBitmap*)(resource->resource );
		CEgBitmap* maskBmp1=(CEgBitmap*)(resource->bmp_mask1);
				
		WRect surfaceRect;
	
		this->GetRect(&surfaceRect);

		int offset_x=MAX(fullRc->x,inClipRc->x)-MIN(fullRc->x,inClipRc->x);
		int offset_y=MAX(fullRc->y,inClipRc->y)-MIN(fullRc->y,inClipRc->y);

		//转换坐标，以Surface Rect 的左上为原点
		int ox = inClipRc->x;
		int oy = inClipRc->y;

		this->m_pSurfaceMemBitmap->ScreenToClient(ox,oy);

		CDC* targetCDC=pSurfaceDC;
		
		int alpha = GetAlpha();		


		if(bMasked == false)
		{

			if(bSelfMask==true)
			{
				DrawSelfMaskBmp(targetCDC,imgBmp,x+offset_x,y+offset_y,inClipRc->dx,inClipRc->dy,ox,oy,inClipRc->dx,inClipRc->dy);
			}else
			{
					
				CDC bmDC;
				bmDC.CreateCompatibleDC(this->GetScreenCDC());
				CBitmap *pOldbmp = bmDC.SelectObject(imgBmp);
				//targetCDC->FillSolidRect(ox,oy,inClipRc->dx,inClipRc->dy,RGB(0xFF,0xFF,0xFF));
				
				//Here found a ASSERT failed during 2008-01-09
			  ASSERT(ox+inClipRc->dx<=this->m_pSurfaceMemBitmap->m_rect.x+m_pSurfaceMemBitmap->m_rect.dx);
	ASSERT(oy+inClipRc->dy<=this->m_pSurfaceMemBitmap->m_rect.y+m_pSurfaceMemBitmap->m_rect.dy);

				targetCDC->BitBlt(ox,oy,inClipRc->dx,inClipRc->dy,&bmDC,x+offset_x,y+offset_y,SRCCOPY);
				bmDC.DeleteDC();
			//	targetCDC->SelectObject(pOldbmp);
				
			}

			
		}
		else
		{
			
				CDC* pScreen = this->m_pScreenBuffer->GetScreenCDC();
				CDC bmDC;
				bmDC.CreateCompatibleDC(pScreen);

				
				CDC maskDC;
				maskDC.CreateCompatibleDC(pScreen);			
				CBitmap *pOldbmp = bmDC.SelectObject(imgBmp);
				maskDC.SelectObject(maskBmp1);
						
				CDC tempDC;
				tempDC.CreateCompatibleDC(pScreen);

				CBitmap pTempBmp;
				pTempBmp.CreateCompatibleBitmap(pScreen,w,h);

				tempDC.SelectObject(&pTempBmp);	

				pOldbmp = bmDC.SelectObject(imgBmp);

				//copy source bmp to tempDC
				tempDC.BitBlt(0,0,w,h,&bmDC,x,y,SRCCOPY);
				
				//check if mask2 exist
				CBitmap* maskBmp2=(CBitmap*)(resource->bmp_mask2);
				if(maskBmp2!=NULL)
				{
						CDC maskDC2;
						maskDC2.CreateCompatibleDC(pScreen);				
						maskDC2.SelectObject(maskBmp2);

						//AND source bmp and mask2 bmp to hide background
						tempDC.BitBlt(0,0,w,h,&maskDC2,0,0,SRCAND);
						maskDC2.DeleteDC();
				}


		





	
				targetCDC->BitBlt(ox,oy,inClipRc->dx,inClipRc->dy,&maskDC,offset_x,offset_y,SRCAND);
				//(screen AND mask) OR bmp
				targetCDC->BitBlt(ox,oy,inClipRc->dx,inClipRc->dy,&tempDC,offset_x,offset_y,SRCPAINT);	
				
				{
					//targetCDC->BitBlt(ox,oy,inClipRc->dx,inClipRc->dy,&pTempTargetCDC,0,0,SRCCOPY);
				}
			
				pTempBmp.DeleteObject();

				bmDC.DeleteDC();
				maskDC.DeleteDC();
				tempDC.DeleteDC();

		}

			

		cntDraw++;

		AddDirtyRect(inClipRc);
		this->m_pSurfaceMemBitmap->ReleaseMemDC(pSurfaceDC);
	}


	/* draw_cmd format
		BMP_MASK:num:x,y,w,h,
	*/
	IMAGE_HANDLE CDisplayManage::LoadImage(char* fn,char* draw_cmd)
	{
		IMAGE_HANDLE imgHandle;

			CWidgetResource* p=this->GetResource(fn);
		
			if(p==NULL)
			{
				
				HBITMAP hBmp = (HBITMAP)::LoadImage(NULL,fn,
								IMAGE_BITMAP,0,0,
								LR_LOADFROMFILE|LR_CREATEDIBSECTION);

				CBitmap* bmp = new CBitmap();
				bmp->Attach(hBmp);

				p = this->AddResource(fn,bmp,NULL,NULL);
				p->bmp_mask1=NULL;
				p->bmp_mask2=NULL;

				BITMAP bm;
				bmp->GetObject(sizeof(BITMAP),&bm);
				p->h=bm.bmHeight;
				p->w=bm.bmWidth;

			int mask_num;
			bool bMaskLoad = false;
					
			char value[16];
			int mask_x,mask_y,mask_w,mask_h;
			
			char* p1=strstr(draw_cmd,"BMP_MASK:");
			

				if(p1>0)
				{
					p1+=strlen("BMP_MASK:");
					
					bMaskLoad=true;

					char* p2=NULL;

					memset(value,0,sizeof(value));
					p2= strchr(p1,',');
					strncpy(value,p1,(p2-p1));
					mask_num=atoi(value);

					int cnt_mask=0;

					while(cnt_mask<mask_num)
					{
						p1=p2+1;
						memset(value,0,sizeof(value));
						p2= strchr(p1,',');
						strncpy(value,p1,(p2-p1));
						mask_x=atoi(value);

						p1=p2+1;
						memset(value,0,sizeof(value));
						p2= strchr(p1,',');
						strncpy(value,p1,(p2-p1));
						mask_y=atoi(value);

						p1=p2+1;
						memset(value,0,sizeof(value));
						p2= strchr(p1,',');
						strncpy(value,p1,(p2-p1));
						mask_w=atoi(value);

						p1=p2+1;
						memset(value,0,sizeof(value));
						p2= strchr(p1,',');
						strncpy(value,p1,(p2-p1));
						mask_h=atoi(value);


						CDC dcSrc;
						CDC dcMask;
						CBitmap* maskBmp = new CBitmap();
						maskBmp->CreateCompatibleBitmap(m_pScreen,mask_w,mask_h);
						dcSrc.CreateCompatibleDC(m_pScreen);
						dcMask.CreateCompatibleDC(m_pScreen);
						dcSrc.SelectObject(bmp);
						dcMask.SelectObject(maskBmp);
						dcMask.BitBlt(0,0,mask_w,mask_h,&dcSrc,mask_x,mask_y,SRCCOPY);
						if(cnt_mask==0)
							p->bmp_mask1=maskBmp;
						else
							p->bmp_mask2=maskBmp;

						dcSrc.DeleteDC();
						dcMask.DeleteDC();		


						cnt_mask++;
					}
				}		
			}	
			return (IMAGE_HANDLE)p;

	}


CMemBitmap* CFrameSurface::GetSurfaceMemBitmap()
{
	return this->m_pSurfaceMemBitmap;
}

CMemBitmap* CFrameSurface::GetMemBitmap(WRect* pRect)
{

		CDC* pScreen=m_pScreenBuffer->GetScreenCDC();
		CMemBitmap* pMemBmp = new CMemBitmap(pScreen,pRect);
		CDC* surfaceCDC=this->m_pSurfaceMemBitmap->CreateMemDC();
		CDC* pBmpDC=pMemBmp->CreateMemDC();
		WRect wrc=*pRect;
		this->m_pSurfaceMemBitmap->ScreenToClient(&wrc);
		pBmpDC->BitBlt(0,0,pRect->dx,pRect->dy,surfaceCDC,wrc.x,wrc.y,SRCCOPY);	
		pMemBmp->ReleaseMemDC(pBmpDC);
		m_pSurfaceMemBitmap->ReleaseMemDC(surfaceCDC);

		return pMemBmp;
	
	
}
void CFrameSurface::DrawStretchBmp(CEgBitmap* pBmp,WRect* pRect)
{	
	CDC dc;
	dc.CreateCompatibleDC(this->GetScreenCDC());
	dc.SelectObject(pBmp);
	
	CDC* surfaceCDC=this->m_pSurfaceMemBitmap->CreateMemDC();
	WRect wrc=*pRect;
	this->m_pSurfaceMemBitmap->ScreenToClient(&wrc);
	//surfaceCDC->BitBlt(wrc.x,wrc.y,wrc.dx,wrc.dx,&dc,0,0,SRCCOPY);


	BITMAP BM;
	pBmp->GetBitmap(&BM);
	surfaceCDC->StretchBlt(wrc.x,wrc.y,wrc.dx,wrc.dx,&dc,0,0,BM.bmWidth,BM.bmHeight,SRCCOPY);

	m_pSurfaceMemBitmap->ReleaseMemDC(surfaceCDC);
	this->AddDirtyRect(pRect);
	dc.DeleteDC();
}
void CMemBitmap::GetRect(WRect* rc)
	{
		CLONE_RECT(&m_rect,rc);
	}

	void CMemBitmap::SetRect(WRect* rc)
	{
		CLONE_RECT(rc,&m_rect);
	}
void CFrameSurface::DrawBmp(CMemBitmap* pBmp,WRect* pRect)
{	


	CDC* pBmpDC=pBmp->CreateMemDC();
	WRect rc;
	pBmp->GetRect(&rc);


	WRect wrc;
	IntersectRect(&wrc,&rc,pRect);

	CDC* surfaceCDC=this->m_pSurfaceMemBitmap->CreateMemDC();

	WRect rcSurface;
	WRect rcDest;
	this->GetRect(&rcSurface);
	IntersectRect(&rcDest,&wrc,&rcSurface);
	this->m_pSurfaceMemBitmap->ScreenToClient(&rcDest);

	ASSERT(rcDest.x+rcDest.dx<=this->m_pSurfaceMemBitmap->m_rect.x+m_pSurfaceMemBitmap->m_rect.dx);
	ASSERT(rcDest.y+rcDest.dy<=this->m_pSurfaceMemBitmap->m_rect.y+m_pSurfaceMemBitmap->m_rect.dy);

	ASSERT(surfaceCDC->BitBlt(rcDest.x,rcDest.y,rcDest.dx,rcDest.dy,pBmpDC,0,0,SRCCOPY)==TRUE);




	m_pSurfaceMemBitmap->ReleaseMemDC(surfaceCDC);

	pBmp->ReleaseMemDC(pBmpDC);
	this->AddDirtyRect(&rcDest);
	
}
void CFrameSurface::BitBlt(CMemBitmap* pMemBmp,WRect* rcDraw)
{

}

CFrameSurface::CFrameSurface(CScreenBuffer* pScreenBuffer,WRect* pSurfaceRect)
	{
		m_pScreenBuffer=pScreenBuffer;
		m_pDirtyTable= new CDirtyTable();;
		m_pNext=NULL;
		m_pPrev=NULL;
		m_bFinishPaint=false;
		ptOld.x=0;
		ptOld.y=0;
		m_bUpdated=false;
		m_bOffScreenDraw=false;
		m_bMemoryDraw=false;

		m_bMemorySurface=false;
		m_nBitBltXOffset =0 ;
		m_nBitBltYOffset =0;
		m_bDestroy=false;
		m_bRemove=false;
		m_nAlpha=255;
		m_pHeadOfBackGroundList=NULL;
		m_pTailOfBackGroundList=NULL;
		m_nBackGround=0;


		this->m_pSurfaceMemBitmap=new CMemBitmap(m_pScreenBuffer->GetScreenCDC(),pSurfaceRect);			

//		m_pSurfaceMemBitmap->m_pCDC->SetBkMode(TRANSPARENT);			
		SetRect(pSurfaceRect);
		m_bViewPort=false;


	}
CFrameSurface::~CFrameSurface()
	{
		
		delete m_pSurfaceMemBitmap;
		if(this->m_pDirtyTable!=NULL)
		{
			delete m_pDirtyTable;
		}


	}
	void CFrameSurface::MergeBackGroundBmp()
	{
		CMemBitmap* pBig=NULL;
		CMemBitmap* pItem = this->m_pHeadOfBackGroundList;
		while(pItem!=NULL)
		{
			WRect* pItemRect = &(pItem->m_rect);
			if(pBig!=NULL)
			{				
				WRect* pBigRect = CRectOp::IsRectContain(&(pBig->m_rect),pItemRect);
				if(pBigRect!=NULL)
				{
					if(pBigRect==pItemRect)
					{
						pItem->m_bRemoveFlag=true;
					}
					else
					{
						pBig->m_bRemoveFlag=true;
					}
					pBig=pItem;
				}
			}else
			{
				pBig=pItem;
			}
			pItem=pItem->m_pNext;
		}

		//Remove some CMemBitmap  from list
		pItem = this->m_pHeadOfBackGroundList;
		while(pItem!=NULL)
		{
			if(pItem->m_bRemoveFlag == TRUE)
			{
					CMemBitmap* pDelete = pItem;
					if(pItem->m_pPrev!=NULL)
					{
						pItem->m_pPrev->m_pNext=pItem->m_pNext;
					}
					else
					{
						this->m_pHeadOfBackGroundList=pItem->m_pNext;
					}

					if(pItem->m_pNext!=NULL)
					{
						pItem->m_pNext->m_pPrev=pItem->m_pPrev;
					}
					else
					{
						this->m_pTailOfBackGroundList=pItem->m_pPrev;
					}		
					this->m_nBackGround--;
					pItem=pItem->m_pNext;
					//delete pDelete;
					continue;
			}
			pItem=pItem->m_pNext;
		}
	}
	CMemBitmap* CFrameSurface::CopySurface(WRect* pRect)
	{
		CDC* pScreen=m_pScreenBuffer->GetScreenCDC();

		CMemBitmap* pMemBmp = new CMemBitmap(pScreen,pRect);
		CDC* surfaceCDC=this->m_pSurfaceMemBitmap->CreateMemDC();
		CDC* pBmpDC=pMemBmp->CreateMemDC();
		WRect wrc=*pRect;
		this->m_pSurfaceMemBitmap->ScreenToClient(&wrc);

		pBmpDC->BitBlt(0,0,pRect->dx,pRect->dy,surfaceCDC,wrc.x,wrc.y,SRCCOPY);	
		pMemBmp->ReleaseMemDC(pBmpDC);
		m_pSurfaceMemBitmap->ReleaseMemDC(surfaceCDC);

		return pMemBmp;

	}
	CMemBitmap* CFrameSurface::AddBackGround(WRect* pRect,CMemBitmap* pMemBmp)
	{

		CDC* pScreen=m_pScreenBuffer->GetScreenCDC();
		if(pMemBmp==NULL)
		{
			pMemBmp = new CMemBitmap(pScreen,pRect);
			CDC* pDC=pMemBmp->CreateMemDC();
			pDC->BitBlt(0,0,pRect->dx,pRect->dy,pScreen,pRect->x,pRect->y,SRCCOPY);	
			pMemBmp->ReleaseMemDC(pDC);
		}
		if(this->m_pTailOfBackGroundList==0)
		{
			m_pTailOfBackGroundList=m_pHeadOfBackGroundList=pMemBmp;
			m_pTailOfBackGroundList->m_pNext=NULL;
			m_pTailOfBackGroundList->m_pPrev=NULL;
		}
		else
		{
			this->m_pTailOfBackGroundList->m_pNext=pMemBmp;
			pMemBmp->m_pPrev=this->m_pTailOfBackGroundList;
			this->m_pTailOfBackGroundList=pMemBmp;
		}
		this->m_nBackGround++;
		return pMemBmp;
	}

	CMemBitmap* CFrameSurface::GetBestFitBackGround(WRect* pRect)
	{
		CMemBitmap* pItem = this->m_pHeadOfBackGroundList;
		while(pItem!=NULL)
		{
			WRect* pItemRect = &(pItem->m_rect);		
			WRect* pBigRect = CRectOp::IsRectContain(pRect,pItemRect);
			if(pBigRect!=NULL)
			{
				if(pBigRect==pItemRect || CRectOp::IsRectSame(pRect,pItemRect))
				{
					CMemBitmap* pNewMemBitmap=new CMemBitmap(this->GetScreenCDC(),pRect);
					pNewMemBitmap->CopyBitmap(pItem);
					
					return pNewMemBitmap;
				}			
			}
			pItem=pItem->m_pNext;
		}
		return NULL;
	}
	void CFrameSurface::DrawNormalBmp(CDC* pCDCBmp,int tx,int ty,int w,int h,int sx,int sy,int sw,int sh)
	{


		// Create memory DC
		CDC* memDC = new CDC();
		CBitmap* memBmp= new CBitmap();
	
		memDC->CreateCompatibleDC(m_pScreenBuffer->GetScreenCDC());
		memBmp->CreateCompatibleBitmap(m_pScreenBuffer->GetScreenCDC(),w,h);		
		memDC->SelectObject(memBmp);

		
		memDC->DeleteDC();
		memBmp->DeleteObject();
		delete memDC;
		delete memBmp;

	}


	void CFrameSurface::DrawTransparentBmp(CMemBitmap* pMemBMP,int tx,int ty,int w,int h,int sx,int sy,int sw,int sh,int alpha)
	{
		BLENDFUNCTION m_bf;	
		m_bf.BlendOp = AC_SRC_OVER;
		m_bf.BlendFlags = 0;
		m_bf.SourceConstantAlpha = 0;
		m_bf.AlphaFormat = 0 ;
		m_bf.SourceConstantAlpha = alpha;

		CDC* pDC=m_pScreenBuffer->CreateBufferDC();
		CDC* pBmpDC=pMemBMP->CreateMemDC();
		pDC->AlphaBlend(tx,ty,w,h,pBmpDC ,sx,sy,sw,sh,m_bf);
		pMemBMP->ReleaseMemDC(pBmpDC);
		m_pScreenBuffer->ReleaseBufferDC(pDC);
		//m_pSurfaceMemBitmap->ReleaseMemDC(pSurfaceDC);

	}

	 void CFrameSurface::RemoveBackGroundDirtyRect()
	{


		SDL_Rect* pItem = this->m_pDirtyTable->m_pHeadOfRects;
		while(pItem!=NULL)
		{			
			if(pItem->IsBackGround()==true )
			{
					SDL_Rect* pDelete = pItem;
					if(pItem->m_pPrev!=NULL)
					{
						pItem->m_pPrev->m_pNext=pItem->m_pNext;
					}
					else
					{
						this->m_pDirtyTable->m_pHeadOfRects=pItem->m_pNext;
					}

					if(pItem->m_pNext!=NULL)
					{
						pItem->m_pNext->m_pPrev=pItem->m_pPrev;
					}
					else
					{
						this->m_pDirtyTable->m_pTailOfRects=pItem->m_pPrev;
					}		
					this->m_pDirtyTable->count--;
					pItem=pItem->m_pNext;
					delete pDelete;
			}else
			{
				pItem=pItem->m_pNext;
			}
		}

		if(this->m_pDirtyTable->count==0)
		{
			this->m_pDirtyTable->m_pHeadOfRects=NULL;
			this->m_pDirtyTable->m_pTailOfRects=NULL;
		}

	}


	void CFrameSurface::AddDirtyRect(WRect* pRect,int nAlpha,CMemBitmap* pMemBitmap,
		bool bHead,bool bPlusOffset,bool bBackGround)
	{
		SDL_Rect* rc= new SDL_Rect();
		WRECT_TO_SDLRECT((*pRect),(*rc));
		rc->nAlpha = nAlpha;
		rc->pMemBmp =pMemBitmap;
		rc->m_bPlusOffset=bPlusOffset;
		rc->m_bBackGround=bBackGround;
		
		if(nAlpha==-1)
		{
			rc->nAlpha=GetAlpha();
		}
		if(pMemBitmap==NULL)
		{
			rc->pMemBmp=this->m_pSurfaceMemBitmap ;
		}		
		if(bHead==true)
		{
			this->m_pDirtyTable->AddToHead(rc);
		}
		else
		{
			this->m_pDirtyTable->Add(rc);
		}
	}





	CMemoryFrameSurface* CDisplayManage::CreateNewMemorySurface(WRect* pSurfaceRc)
	{
		 DWORD  curThreadId = GetCurrentThreadId();
		//TRACE("Current Thread ID = %d\n", curThreadId);
		CMemoryFrameSurface * pSurface = new CMemoryFrameSurface(m_pScreenBuffer,pSurfaceRc);
		pSurface->m_nThreadID=curThreadId;
		pSurface->m_pDisplayManage=this;
		return pSurface;
	}
	//请注意CFrameSurface必须是Thread Safe,因为绘图操作可能由不同的Thread发起，所以
	//每个FrameSurface对每个Thread是独立的。
	CFrameSurface* CDisplayManage::CreateNewOfflineSurface(WRect* pSurfaceRc)
	{
		 DWORD  curThreadId = GetCurrentThreadId();
		//TRACE("Current Thread ID = %d\n", curThreadId);		
		 WRect rc;
		 rc.dx=m_pScreenBuffer->m_nScreenWidth ;
		 rc.dy=m_pScreenBuffer->m_nScreenHeight;
		 rc.x=0;
		 rc.y=0;
		 WRect rcOut;
		 IntersectRect(&rcOut,&rc,pSurfaceRc);
		 *pSurfaceRc=rcOut;
		CFrameSurface * pSurface = new CFrameSurface(m_pScreenBuffer,pSurfaceRc);
		pSurface->m_nThreadID=curThreadId;
		pSurface->m_pDisplayManage=this;
		return pSurface;
	}

	CFrameSurface* CDisplayManage::CreateFrameSurface(WRect* pSurfaceRc)
	{
		return GetCurrentThreadFrameSurface(false,pSurfaceRc);
	}
	CFrameSurface* CDisplayManage::GetCurFrameSurface()
	{
		CFrameSurface* pSurface;
		pSurface = GetCurrentThreadFrameSurface(true);
		return pSurface;
	}
	CFrameSurface* CDisplayManage::GetCurrentThreadFrameSurface(bool bMustHave,WRect* pSurfaceRc)
	{
		 DWORD  curThreadId = GetCurrentThreadId();
		 ::EnterCriticalSection(&m_CriticalSection);
		CFrameSurface* pItem = (CFrameSurface*)(this->m_pHeadOfFrameSurfaces);		
		while(pItem!=0)
		{
				if(pItem->m_nThreadID == curThreadId && pItem->IsPaintFinished()==false)
				{
					break;
				}
				pItem=pItem->m_pNext;			
		}		
		 ::LeaveCriticalSection(&m_CriticalSection);   

		 if(bMustHave == false)
		 {
			if(pItem==NULL)
			{
				pItem = CreateNewOfflineSurface(pSurfaceRc);
				AddFrameSurface(pItem);
			}
		}
		 pItem->m_pDisplayManage=this;

		 return pItem;

	}

	BOOL CDisplayManage::IsFrameSurfaceAlreadyIn(CFrameSurface* pSurface)
	{
		::EnterCriticalSection(&m_CriticalSection);
		CFrameSurface* pItem = (CFrameSurface*)(this->m_pHeadOfFrameSurfaces);
		BOOL bFound = false;
		while(pItem!=0)
		{			
			if(pItem==pSurface){
				bFound =true;
				break;
			}
			pItem=pItem->m_pNext;
		}
		::LeaveCriticalSection(&m_CriticalSection);  
		return bFound;

	}

	CFrameSurface* CDisplayManage::AddFrameSurface(CFrameSurface* pSurface)
	{
		if(IsFrameSurfaceAlreadyIn(pSurface)==TRUE)
		{
			return pSurface;
		}

		 ::EnterCriticalSection(&m_CriticalSection);
		if(this->m_pTailOfFrameSurfaces==0)
		{
			m_pTailOfFrameSurfaces=m_pHeadOfFrameSurfaces=pSurface;
			m_pTailOfFrameSurfaces->m_pNext=NULL;
			m_pTailOfFrameSurfaces->m_pPrev=NULL;
		}
		else
		{
			this->m_pTailOfFrameSurfaces->m_pNext=pSurface;
			pSurface->m_pPrev=this->m_pTailOfFrameSurfaces;
			this->m_pTailOfFrameSurfaces=pSurface;
		}
		this->m_nFrameSurfaces++;
	//	TRACE("Current Surface = %d\n",this->m_nFrameSurfaces );
		::LeaveCriticalSection(&m_CriticalSection);   
		


		return pSurface;
	}




	BOOL CDisplayManage::RemoveFrameSurface(CFrameSurface* pSurface)
	{

		 ::EnterCriticalSection(&m_CriticalSection);
		CFrameSurface* pItem = (CFrameSurface*)(this->m_pHeadOfFrameSurfaces);
		while(pItem!=0)
		{
			if(pItem==pSurface)
			{
				if(pItem->m_pPrev!=NULL)
				{
					pItem->m_pPrev->m_pNext=pItem->m_pNext;
				}
				else
				{
					this->m_pHeadOfFrameSurfaces=pItem->m_pNext;
				}

				if(pItem->m_pNext!=NULL)
				{
					pItem->m_pNext->m_pPrev=pItem->m_pPrev;
				}
				else
				{
					this->m_pTailOfFrameSurfaces=pItem->m_pPrev;
				}		
				this->m_nFrameSurfaces--;
			    ::LeaveCriticalSection(&m_CriticalSection);   
				return true;
			}
			pItem=pItem->m_pNext;

		}
		 ::LeaveCriticalSection(&m_CriticalSection);   
		return false;		
	}

	/*
	函数功能：Bitblt所有Paint Finished's的FrameSurface到屏幕位图完成显示
	*/
	BOOL CDisplayManage::FlushSurface()
	{

		 ::EnterCriticalSection(&m_CriticalSection);
		CFrameSurface* pItem = (CFrameSurface*)(this->m_pHeadOfFrameSurfaces);	
		if(this==NULL)
		{
			//TRACE("fuck");
		}
		if(this->m_nFrameSurfaces>0)
		{
			DWORD  curThreadId = GetCurrentThreadId();
			//TRACE("Current Thread ID = %d\n", curThreadId);
		}else
		{
			goto _exit;
		}
		

		while(pItem!=0)
		{
			if(pItem->IsUpdated()==true)
			{

				// 2007-12-06
				//该函数用来在动画过程中计算绘制区域，基于如下几点需求
				//1. 通常每个MemorySurface都对应于某个Widget
				//2. 当对某个Widget的MemorySurface进行动画时,该动画不应该覆盖Z_ORDER大于其的Widget之上
				//3. 所以每次AnimateEngine在绘制MemorySurface时，可以调用该函数去计算实际需要绘制的区域

				if(pItem->IsMemorySurface()==true)
				{
					CWidget* pRelatedWidget=(CWidget*)(pItem->GetWidget());
					if(pRelatedWidget!=NULL)
					{
						CContainerWidget* pContainer= pRelatedWidget->m_pContainer;
						if(pContainer!=NULL)
							pContainer->ReCalOverlapInvalidSurface((CMemoryFrameSurface*)pItem);
					}
				}

				//遍历每个Rect			
				SDL_Rect* pSDLRect = pItem->m_pDirtyTable->m_pHeadOfRects;
				while(pSDLRect!=NULL)
				{
					if(pSDLRect->GetDrawOp()==DRAW_OP_PAINT_NOT_THIS_TIME)
					{
						pSDLRect->SetDrawOp(DRAW_OP_ALWAYS_PAINT);
						pSDLRect=pSDLRect->m_pNext;
						continue;
					}

					
				




					//FrameSurface中Rect的位置是屏幕绝对位置
					//但Surfcae中位图数据保存的位置是相对与surface的Top Left点，因为surface可能远远小于屏幕大小，
					//没必要保存所有的屏幕位图数据，所以写回屏幕是要坐标进行转换

					
					WRect rcDraw;

					
					rcDraw.x=pSDLRect->x;
					rcDraw.y=pSDLRect->y;
					rcDraw.dx=pSDLRect->w;
					rcDraw.dy=pSDLRect->h;
					//如果定义了viewport，则绘图只应该发生在view port定义的Rect范围之内
					
					WRect rcView;
					if(pItem->GetViewPort(&rcView)==true)
					{
						WRect rc1;
						rc1.x=pSDLRect->x;
						rc1.y=pSDLRect->y;
						rc1.dx=pSDLRect->w;
						rc1.dy=pSDLRect->h;

						if(IntersectRect(&rcDraw,&rc1,&rcView)==false) 
						{
							pSDLRect=pSDLRect->m_pNext;
							continue;
						}						
					}
					
					int x = rcDraw.x;
					int y = rcDraw.y;
					int sx = rcDraw.x;
					int sy = rcDraw.y;


					 pSDLRect->pMemBmp->ScreenToClient(sx,sy);
					 /*
					if(pSDLRect->IsPlusOffset()==true)
					{
						int x_offset=0;
						int y_offset=0;
						pItem->GetBitBltOffset(x_offset,y_offset);	
						x+=x_offset;
						y+=y_offset;
					}
					*/

					int alpha = pItem->GetAlpha();
					//背景RECT不可应用Surface的Alpha属性，他通常是作为同一Surface中其他Alpha Rect的背景,
					//所以不可以使用AlphaBlend去绘制
					if((alpha==255 && pSDLRect->nAlpha ==255)||pSDLRect->IsBackGround()==true)
						//not transparent
					{

						//DrawNormalBmp(pMemDC,x,y,pSDLRect->w,pSDLRect->h,sx,sy,
						//	pSDLRect->w,pSDLRect->h);


						CDC* pBufferDC=m_pScreenBuffer->CreateBufferDC();
						CDC* pMemDC=pSDLRect->pMemBmp->CreateMemDC();
						
						ASSERT(x+rcDraw.dx<=m_pScreenBuffer->m_nScreenWidth );
						ASSERT(y+rcDraw.dy<=m_pScreenBuffer->m_nScreenHeight );

						pBufferDC->BitBlt(x,y,rcDraw.dx,rcDraw.dy,pMemDC,sx,sy,SRCCOPY);
						pSDLRect->pMemBmp->ReleaseMemDC(pMemDC);
						m_pScreenBuffer->ReleaseBufferDC(pBufferDC);
	
						WRect rcUpdate;
						rcUpdate.x=x;
						rcUpdate.y=y;
						rcUpdate.dx=rcDraw.dx;
						rcUpdate.dy=rcDraw.dy;
						m_pScreenBuffer->AddDirtyRect(&rcUpdate);

					}
					else
					{
						if(pSDLRect->nAlpha<255) 
						{
							alpha = pSDLRect->nAlpha;
						}

						pItem->DrawTransparentBmp(pSDLRect->pMemBmp,x,y,rcDraw.dx,rcDraw.dy,sx,sy,
							rcDraw.dx,rcDraw.dy,alpha);

						WRect rcUpdate;
						rcUpdate.x=x;
						rcUpdate.y=y;
						rcUpdate.dx=rcDraw.dx;
						rcUpdate.dy=rcDraw.dy;
						m_pScreenBuffer->AddDirtyRect(&rcUpdate);

					}

					if(pSDLRect->GetDrawOp()==DRAW_OP_PAINT_ONE_TIME)
					{
						SDL_Rect* pDelete = pSDLRect;
	
						if(pSDLRect->m_pPrev!=NULL)
						{
							pSDLRect->m_pPrev->m_pNext=pSDLRect->m_pNext;
						}
						else
						{
							pItem->m_pDirtyTable->m_pHeadOfRects=pSDLRect->m_pNext;
						}

						if(pSDLRect->m_pNext!=NULL)
						{
							pSDLRect->m_pNext->m_pPrev=pSDLRect->m_pPrev;
						}
						else
						{
							pItem->m_pDirtyTable->m_pTailOfRects=pSDLRect->m_pPrev;
						}		
						pItem->m_pDirtyTable->count--;

						if(pItem->m_pDirtyTable->count==0)
						{
							pItem->m_pDirtyTable->m_pTailOfRects=NULL;
							pItem->m_pDirtyTable->m_pHeadOfRects=NULL;			
						}
						pSDLRect=pSDLRect->m_pNext;
						delete pDelete;
						continue;
					}
					pSDLRect=pSDLRect->m_pNext;
				}
			}
			//Set Surface's Paint Finish flag to false after bitblt end.
			//绘制结束将surface的paint finish标志置为true
			pItem->SetPainFinish(true);

			//绘制结束将surface的update flag置为false
			pItem->SetUpdated(false);
			


			//检查 Remove Flag，如果为True将该Surface从 surface列表中删除
			CFrameSurface* pDelete =pItem;
			if(pItem->IsRemove()==TRUE)
			{												
				if(pItem->m_pPrev!=NULL)
				{
					pItem->m_pPrev->m_pNext=pItem->m_pNext;
				}
				else
				{
					this->m_pHeadOfFrameSurfaces=pItem->m_pNext;
				}

				if(pItem->m_pNext!=NULL)
				{
					pItem->m_pNext->m_pPrev=pItem->m_pPrev;
				}
				else
				{
					this->m_pTailOfFrameSurfaces=pItem->m_pPrev;
				}										
				this->m_nFrameSurfaces--;
			}
			
			pItem=pItem->m_pNext;

			if(this->m_nFrameSurfaces==0)
			{
				this->m_pTailOfFrameSurfaces=NULL;
				this->m_pHeadOfFrameSurfaces=NULL;				
			}

			//检查Destroy Flag
			//如果不是MemorySurface则直接将其销毁,否则保留，因为
			//MemorySurface一般是动画用的，下次动画可能还要用,留给
			//AnimateEngine to destroy
			if( pDelete->IsDestroy()==true)
			{
				delete pDelete;
			}					

			


		}



		if(this->m_nFrameSurfaces==0)
		{
			m_pTailOfFrameSurfaces=NULL;
			m_pHeadOfFrameSurfaces=NULL;
		}
		this->m_pScreenBuffer->FlushToScreen();
_exit:
		 ::LeaveCriticalSection(&m_CriticalSection);   


		return false;		
	}

	void CDirtyTable::MergeNeighborRect()
	{
		SDL_Rect* pItem = (SDL_Rect*)(this->m_pHeadOfRects);
		SDL_Rect* pMerge=NULL;
		int n = 0;
		while(pItem!=0)
		{		
			if(pMerge!=NULL)
			{
				bool bMerged = false;
				if(pMerge->pMemBmp ==pItem->pMemBmp && pMerge->nAlpha == pItem->nAlpha )
				{
					if( pMerge->x==pItem->x 
						&& 
						pMerge->w==pItem->w)
					{
						int y1 = pMerge->y;
						int y2 = pMerge->y+pMerge->h;
						int yy1 = pItem->y;
						int yy2 = pItem->h;

						if( (yy1>=y1 && yy1<=y2) ||
							(y1>=yy1 && y1<=yy2))
						{
							MergeRect(pItem,pMerge);
							n++;
							bMerged = true;
						}
					}else
					if( pMerge->y==pItem->y 
						&& 
						pMerge->h==pItem->h)
					{
						int x1 = pMerge->x;
						int x2 = pMerge->x+pMerge->w;
						int xx1 = pItem->x;
						int xx2 = pItem->w;

						if( (xx1>=x1 && xx1<=x2) ||
							(x1>=xx1 && x1<=xx2))
						{
							MergeRect(pItem,pMerge);
							n++;
							bMerged= true;
						}
					}
				}

				if(bMerged == false)
				{
					pMerge=pItem;
				}
				else
				{
					//删除当前的RECT
					SDL_Rect* pDelete = pItem;
					if(pItem->m_pPrev!=NULL)
					{
						pItem->m_pPrev->m_pNext=pItem->m_pNext;
					}
					else
					{
						this->m_pHeadOfRects=pItem->m_pNext;
					}

					if(pItem->m_pNext!=NULL)
					{
						pItem->m_pNext->m_pPrev=pItem->m_pPrev;
					}
					else
					{
						this->m_pTailOfRects=pItem->m_pPrev;
					}		
					this->count--;
					pItem=pItem->m_pNext;
					delete pDelete;
					continue;
				}
			}			
			else
			{
				pMerge=pItem;
			}
			pItem=pItem->m_pNext;
		}
	}
	void CFrameSurface::Aggregate()
	{
		//删除所有的同与surface membmp相同的rect
		//将surface作为一个整体加到dirty rect中去，而不是作为分隔的dirty rect
		SDL_Rect* pItem = this->m_pDirtyTable->m_pHeadOfRects;
		while(pItem!=NULL)
		{
			
			if(pItem->pMemBmp==this->m_pSurfaceMemBitmap )
			{
	//删除当前的RECT
					SDL_Rect* pDelete = pItem;
					if(pItem->m_pPrev!=NULL)
					{
						pItem->m_pPrev->m_pNext=pItem->m_pNext;
					}
					else
					{
						this->m_pDirtyTable->m_pHeadOfRects=pItem->m_pNext;
					}

					if(pItem->m_pNext!=NULL)
					{
						pItem->m_pNext->m_pPrev=pItem->m_pPrev;
					}
					else
					{
						this->m_pDirtyTable->m_pTailOfRects=pItem->m_pPrev;
					}		
					this->m_pDirtyTable->count--;
					pItem=pItem->m_pNext;
					delete pDelete;
			}else
			{
				pItem=pItem->m_pNext;
			}
		}

		this->AddDirtyRect(&(this->m_pSurfaceMemBitmap->m_rect));

	}
	void CFrameSurface::Save()
	{
			//m_pSurfaceMemBitmap->m_pBmp->Save(); 
			SDL_Rect* pSDLRect = this->m_pDirtyTable->m_pHeadOfRects;
			while(pSDLRect!=NULL)
			{
				
				if(pSDLRect->IsBackGround()==false)
					//not Background
				{
					pSDLRect->pMemBmp->m_pBmp->Save();
				}		
				pSDLRect=pSDLRect->m_pNext;
			}

	}
	void CFrameSurface::Restore()
	{
			//m_pSurfaceMemBitmap->m_pBmp->Restore();
			SDL_Rect* pSDLRect = this->m_pDirtyTable->m_pHeadOfRects;
			while(pSDLRect!=NULL)
			{
				
				if(pSDLRect->IsBackGround()==false)
					//not Background
				{
					pSDLRect->pMemBmp->m_pBmp->Restore();
				}		
				pSDLRect=pSDLRect->m_pNext;
			}

	}
	CMemBitmap* CFrameSurface::FilpSurface(float nRadian)
	{
			SDL_Rect* pSDLRect = this->m_pDirtyTable->m_pHeadOfRects;
			while(pSDLRect!=NULL)
			{
				
				if(pSDLRect->IsBackGround()==false)
					//not Background
				{
					pSDLRect->pMemBmp->FlipBmp(nRadian);	
					pSDLRect->w=pSDLRect->pMemBmp->m_rect.dx;
					pSDLRect->h=pSDLRect->pMemBmp->m_rect.dy;
				}		
				pSDLRect=pSDLRect->m_pNext;
			}
			return NULL;

	}

	void CMemBitmap::FlipBmp(int nRadian)
	{
		BITMAP BM;


		m_pBmp->GetBitmap(&BM);

		m_pBmp->ShearImage(nRadian,0,0);

		m_pBmp->GetBitmap(&BM);
		this->m_rect.dx=BM.bmWidth;
		this->m_rect.dy=BM.bmHeight;


	}

	void CFrameSurface::Stretch(float sx,float sy)
	{
			SDL_Rect* pSDLRect = this->m_pDirtyTable->m_pHeadOfRects;
			while(pSDLRect!=NULL)
			{
				
				if(pSDLRect->IsBackGround()==false)
					//not Background
				{
					pSDLRect->pMemBmp->m_pBmp->Stretch(sx,sy);

					BITMAP BM;
					pSDLRect->pMemBmp->m_pBmp->GetBitmap(&BM);
					pSDLRect->w=BM.bmWidth;
					pSDLRect->h=BM.bmHeight;


				}		
				pSDLRect=pSDLRect->m_pNext;
			}
	}

	void CFrameSurface::Rotate(float degrees,float view_z)
	{
			SDL_Rect* pSDLRect = this->m_pDirtyTable->m_pHeadOfRects;
			while(pSDLRect!=NULL)
			{
				
				if(pSDLRect->IsBackGround()==false)
					//not Background
				{

					pSDLRect->pMemBmp->m_pBmp->Rotate_y(degrees,view_z,255);

				}		
				pSDLRect=pSDLRect->m_pNext;
			}

	}

	void CFrameSurface::RemoveRect(CMemBitmap* pMemBitmap)
	{
			SDL_Rect* pItem = this->m_pDirtyTable->m_pHeadOfRects;
			while(pItem!=NULL)
			{
				
				if(pItem->pMemBmp == pMemBitmap)
					//not Background
				{

					SDL_Rect* pDelete = pItem;
					if(pItem->m_pPrev!=NULL)
					{
						pItem->m_pPrev->m_pNext=pItem->m_pNext;
					}
					else
					{
						this->m_pDirtyTable->m_pHeadOfRects=pItem->m_pNext;
					}

					if(pItem->m_pNext!=NULL)
					{
						pItem->m_pNext->m_pPrev=pItem->m_pPrev;
					}
					else
					{
						this->m_pDirtyTable->m_pTailOfRects=pItem->m_pPrev;
					}		
					this->m_pDirtyTable->count--;

					if(this->m_pDirtyTable->count==0)
					{
						this->m_pDirtyTable->m_pTailOfRects=NULL;
						this->m_pDirtyTable->m_pHeadOfRects=NULL;			
					}
					pItem=pItem->m_pNext;
					delete pDelete;

				}		
				else
				{
					pItem=pItem->m_pNext;
				}
			}
	}

	void CFrameSurface::Zoom(unsigned int bZoomIn,float beishu)
	{
			SDL_Rect* pSDLRect = this->m_pDirtyTable->m_pHeadOfRects;
			while(pSDLRect!=NULL)
			{
				
				if(pSDLRect->IsBackGround()==false)
					//not Background
				{

					if(bZoomIn==true)
					{
						pSDLRect->pMemBmp->m_pBmp->ZoomIn(beishu);
					}else
					{
						pSDLRect->pMemBmp->m_pBmp->ZoomOut(beishu);
					}


				}		
				pSDLRect=pSDLRect->m_pNext;
			}

	}

	void CFrameSurface::ReCalSurfaceByExcludeRect(WRect* pExcludeRect)
	{
			SDL_Rect* pItem = this->m_pDirtyTable->m_pHeadOfRects;
			int total_rects=0;
			while(pItem!=NULL)
			{				
				if(pItem->GetDrawOp()!=DRAW_OP_ALWAYS_PAINT &&
					pItem->GetDrawOp()!=DRAW_OP_PAINT_ONE_TIME)
				{
					pItem=pItem->m_pNext;
					continue;
				}
				WRect rc;
				rc.dx=pItem->w;
				rc.dy=pItem->h;
				rc.x=pItem->x;
				rc.y=pItem->y;

				WRect* pInvalidRects=NULL;
				int numRects=CRectOp::CalDirty(&rc,pExcludeRect,pInvalidRects);

				
				if(numRects<=0)
				{
					if(pItem->GetDrawOp()==DRAW_OP_PAINT_ONE_TIME)
					{
						SDL_Rect* pDelete = pItem;
						if(pItem->m_pPrev!=NULL)
						{
							pItem->m_pPrev->m_pNext=pItem->m_pNext;
						}
						else
						{
							this->m_pDirtyTable->m_pHeadOfRects=pItem->m_pNext;
						}

						if(pItem->m_pNext!=NULL)
						{
							pItem->m_pNext->m_pPrev=pItem->m_pPrev;						
						}
						else
						{
							this->m_pDirtyTable->m_pTailOfRects=pItem->m_pPrev;
						}				
						this->m_pDirtyTable->count--;
						if(this->m_pDirtyTable->count==0)
						{
							this->m_pDirtyTable->m_pTailOfRects=NULL;
							this->m_pDirtyTable->m_pHeadOfRects=NULL;			
						}
						pItem=pItem->m_pNext;
						delete pDelete;

					}else
					if(pItem->GetDrawOp()==DRAW_OP_ALWAYS_PAINT)
					{
						pItem->SetDrawOp(DRAW_OP_PAINT_NOT_THIS_TIME);
						pItem=pItem->m_pNext;	
					}


					
					
					continue;

				}
				total_rects+=numRects;
			
				WRect* pRc=pInvalidRects;

				SDL_Rect* pHead=NULL;
				SDL_Rect* pTail=NULL;
				bool bRemoveThisItem=false;

				//表明该Item是上次计算出来的未画区域,必须被删除
				if(pItem->GetDrawOp()==DRAW_OP_PAINT_ONE_TIME)
				{
					bRemoveThisItem=true;
				}
				//表明这是源图区域不可以删除
				if(pItem->GetDrawOp()==DRAW_OP_ALWAYS_PAINT)
				{
					pItem->SetDrawOp(DRAW_OP_PAINT_NOT_THIS_TIME);
				}
				
				while(numRects>0)
				{
					SDL_Rect* newSR=pItem->Clone();
					newSR->x=pRc->x;
					newSR->h=pRc->dy;
					newSR->y=pRc->y;
					newSR->w=pRc->dx;					
					if(pTail==NULL)
					{
						pHead=newSR;
					}
					else
					{
						pTail->m_pNext=newSR;
						newSR->m_pPrev=pTail;						
					}
					pTail=newSR;
					this->m_pDirtyTable->count++;
					newSR->SetDrawOp(DRAW_OP_PAINT_ONE_TIME);

					numRects--;
					pRc++;
					
				}						

				EG_FREE(pInvalidRects);

				if(bRemoveThisItem==false)
				{
					if(pItem->m_pNext!=NULL)
					{
						pItem->m_pNext->m_pPrev=pTail;
						pTail->m_pNext=pItem->m_pNext;					
					}
					else
					{
						this->m_pDirtyTable->m_pTailOfRects=pTail;
					}					
					pItem->m_pNext=pHead;
					pHead->m_pPrev=pItem;			

					pItem=pTail->m_pNext;				
				}
				else
				{
					SDL_Rect* pDelete = pItem;
					if(pItem->m_pPrev!=NULL)
					{
						pItem->m_pPrev->m_pNext=pHead;
						pTail->m_pNext=pItem->m_pNext;
					}
					else
					{
						this->m_pDirtyTable->m_pHeadOfRects=pHead;
					}

					if(pItem->m_pNext!=NULL)
					{
						pItem->m_pNext->m_pPrev=pTail;						
					}
					else
					{
						this->m_pDirtyTable->m_pTailOfRects=pTail;
					}
					pHead->m_pPrev=pItem->m_pPrev;

					this->m_pDirtyTable->count--;

					if(this->m_pDirtyTable->count==0)
					{
						this->m_pDirtyTable->m_pTailOfRects=NULL;
						this->m_pDirtyTable->m_pHeadOfRects=NULL;			
					}
					pItem=pTail->m_pNext;
					delete pDelete;
				}
				
		}
	}

	void CFrameSurface::Move(unsigned int x,unsigned y)
	{
			WRect rcCur;
			GetRect(&rcCur);

			int offset_x=x-rcCur.x;
			int offset_y=y-rcCur.y;

			rcCur.x=x;
			rcCur.y=y;
			SetRect(&rcCur);

			SDL_Rect* pSDLRect = this->m_pDirtyTable->m_pHeadOfRects;
			
			while(pSDLRect!=NULL)
			{				
				if(pSDLRect->IsBackGround()==false)
					//not Background
				{
					pSDLRect->x+=offset_x;
					pSDLRect->y+=offset_y;
				}		
				pSDLRect=pSDLRect->m_pNext;
			}
	}