// EnBitmap.cpp: implementation of the CEgBitmap class (c) daniel godson 2002.
//
// credits: Peter Hendrix's CPicture implementation for the original IPicture code 
//          Yves Maurer's GDIRotate implementation for the idea of working directly on 32 bit representations of bitmaps 
//          Karl Lager's 'A Fast Algorithm for Rotating Bitmaps' 
// 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EgBitmap.h"
#include <math.h>
#include <afxpriv.h>
#include "common.h"
#include "fixed.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const int HIMETRIC_INCH	= 2540;

enum 
{
	FT_BMP,
	FT_ICO,
	FT_JPG,
	FT_GIF,

	FT_UNKNOWN
};

/*
//将3D坐标转换为2D坐标 
//这个好象是叫做斜等测平行投影   
void   Transform3DTo2D(f2dPoint   *pt2d,   const   f3dPoint   *pt3d,   const   int   &n) 
{ 
for(int   i=0;i <n;i++) 
{ 
pt2d[i].x=pt3d[i].x   -   pt3d[i].z   /sqrt(2); 
pt2d[i].y=pt3d[i].y   -   pt3d[i].z   /sqrt(2); 
} 
} 
//用于坐标旋转变化的矩阵，只要给这个矩阵赋值就可以做相应的变换 
dPoint   *   TranslatePoint(f3dPoint   *pt,   double   matrix[][4],   const   int   &n) 
{ 
for(int   i=0;i <n;i++) 
{ 
double   a[4]   =   {pt[i].x,pt[i].y,pt[i].z,1}; 
pt[i].x   =   a[0]*matrix[0][0]+a[1]*matrix[1][0]+a[2]*matrix[2][0]+a[3]*matrix[3][0]; 
pt[i].y   =   a[0]*matrix[0][1]+a[1]*matrix[1][1]+a[2]*matrix[2][1]+a[3]*matrix[3][1]; 
pt[i].z   =   a[0]*matrix[0][2]+a[1]*matrix[1][2]+a[2]*matrix[2][2]+a[3]*matrix[3][2]; 
} 
return   pt; 
} 

//平移 
ClearMatrix();     //清空矩阵 
matrix[0][0]=1; 
matrix[1][1]=1; 
matrix[2][2]=1; 
matrix[3][3]=1; 
matrix[3][1]=10;//X轴正方向移动10个单位 
//错切 
ClearMatrix(); 
matrix[0][0]=1; 
matrix[1][1]=1; 
matrix[2][2]=1; 
matrix[3][3]=1; 
matrix[1][0]=0.5; 
matrix[2][0]=2; 
//旋转   绕Z轴旋转   每次转30度 
ClearMatrix(); 
matrix[0][0]=cos(30*Pi/180); 
matrix[0][1]=sin(30*Pi/180); 
matrix[1][0]=-sin(30*Pi/180); 
matrix[1][1]=cos(30*Pi/180); 
matrix[2][2]=1; 
matrix[3][3]=1; 
//反射 
ClearMatrix(); 
matrix[0][0]=-1; 
matrix[1][1]=1; 
matrix[2][2]=1; 
matrix[3][3]=1; 
//按比例变换 
ClearMatrix(); 
matrix[0][0]=0.8; 
matrix[1][1]=0.6; 
matrix[2][2]=0.5; 
matrix[3][3]=1; 

3D坐标转换为2D坐标大概应该以上的步骤   
还有一些基本的变换操作

*/

// This function converts the given bitmap to a DFB.
// Returns true if the conversion took place,
// false if the conversion either unneeded or unavailable
bool ConvertToDFB(HBITMAP& hBitmap)
{
  bool bConverted = false;
  BITMAP stBitmap;
  if (GetObject(hBitmap, sizeof(stBitmap), &stBitmap) && stBitmap.bmBits)
  {
    // that is a DIB. Now we attempt to create
    // a DFB with the same sizes, and with the pixel
    // format of the display (to omit conversions
    // every time we draw it).
    HDC hScreen = GetDC(NULL);
    if (hScreen)
    {
      HBITMAP hDfb = 
              CreateCompatibleBitmap(hScreen, 
      stBitmap.bmWidth, stBitmap.bmHeight);
      if (hDfb)
      {
        // now let's ensure what we've created is a DFB.
        if (GetObject(hDfb, sizeof(stBitmap), 
                   &stBitmap) && !stBitmap.bmBits)
        {
          // ok, we're lucky. Now we have
          // to transfer the image to the DFB.
          HDC hMemSrc = CreateCompatibleDC(NULL);
          if (hMemSrc)
          {
            HGDIOBJ hOldSrc = SelectObject(hMemSrc, hBitmap);
            if (hOldSrc)
            {
              HDC hMemDst = CreateCompatibleDC(NULL);
              if (hMemDst)
              {
                HGDIOBJ hOldDst = SelectObject(hMemDst, hDfb);
                if (hOldDst)
                {
                  // transfer the image using BitBlt
                  // function. It will probably end in the
                  // call to driver's DrvCopyBits function.
                  if (BitBlt(hMemDst, 0, 0, 
                        stBitmap.bmWidth, stBitmap.bmHeight, 
                        hMemSrc, 0, 0, SRCCOPY))
                    bConverted = true; // success

                  VERIFY(SelectObject(hMemDst, hOldDst));
                }
                VERIFY(DeleteDC(hMemDst));
              }
              VERIFY(SelectObject(hMemSrc, hOldSrc));
            }
            VERIFY(DeleteDC(hMemSrc));
          }
        }

        if (bConverted)
        {
          VERIFY(DeleteObject(hBitmap)); // it's no longer needed
          hBitmap = hDfb;
        }
        else
          VERIFY(DeleteObject(hDfb));
      }
      ReleaseDC(NULL, hScreen);
    }
  }
  return bConverted;
}

// This function converts the given bitmap to a DIB.
// Returns true if the conversion took place,
// false if the conversion either unneeded or unavailable
bool ConvertToDIB(HBITMAP& hBitmap)
{
  bool bConverted = false;
  BITMAP stBitmap;
  if (GetObject(hBitmap, sizeof(stBitmap), 
            &stBitmap) && !stBitmap.bmBits)
  {
    // that is a DFB. Now we attempt to create
    // a DIB with the same sizes and pixel format.
    HDC hScreen = GetDC(NULL);
    if (hScreen)
    {
      union {
        BITMAPINFO stBitmapInfo;
        BYTE pReserveSpace[sizeof(BITMAPINFO) 
                     + 0xFF * sizeof(RGBQUAD)];
      };
      ZeroMemory(pReserveSpace, sizeof(pReserveSpace));
      stBitmapInfo.bmiHeader.biSize = sizeof(stBitmapInfo.bmiHeader);
      stBitmapInfo.bmiHeader.biWidth = stBitmap.bmWidth;
      stBitmapInfo.bmiHeader.biHeight = stBitmap.bmHeight;
      stBitmapInfo.bmiHeader.biPlanes = 1;
      stBitmapInfo.bmiHeader.biBitCount = stBitmap.bmBitsPixel;
      stBitmapInfo.bmiHeader.biCompression = BI_RGB;

      if (stBitmap.bmBitsPixel <= 8)
      {
        stBitmapInfo.bmiHeader.biClrUsed = 
                        1 << stBitmap.bmBitsPixel;
        // This image is paletted-managed.
        // Hence we have to synthesize its palette.
      }
      stBitmapInfo.bmiHeader.biClrImportant = 
                       stBitmapInfo.bmiHeader.biClrUsed;

      PVOID pBits;
      HBITMAP hDib = CreateDIBSection(hScreen, 
        &stBitmapInfo, DIB_RGB_COLORS, &pBits, NULL, 0);

      if (hDib)
      {
        // ok, we're lucky. Now we have
        // to transfer the image to the DFB.
        HDC hMemSrc = CreateCompatibleDC(NULL);
        if (hMemSrc)
        {
          HGDIOBJ hOldSrc = SelectObject(hMemSrc, hBitmap);
          if (hOldSrc)
          {
            HDC hMemDst = CreateCompatibleDC(NULL);
            if (hMemDst)
            {
              HGDIOBJ hOldDst = SelectObject(hMemDst, hDib);
              if (hOldDst)
              {
                if (stBitmap.bmBitsPixel <= 8)
                {
                  // take the DFB's palette and set it to our DIB
                  HPALETTE hPalette = 
                    (HPALETTE) GetCurrentObject(hMemSrc, OBJ_PAL);
                  if (hPalette)
                  {
                    PALETTEENTRY pPaletteEntries[0x100];
                    UINT nEntries = GetPaletteEntries(hPalette, 
                                    0, stBitmapInfo.bmiHeader.biClrUsed, 
                                    pPaletteEntries);
                    if (nEntries)
                    {
                      ASSERT(nEntries <= 0x100);
                      for (UINT nIndex = 0; nIndex < nEntries; nIndex++)
                        pPaletteEntries[nEntries].peFlags = 0;
                      VERIFY(SetDIBColorTable(hMemDst, 0, 
                        nEntries, (RGBQUAD*) pPaletteEntries) == nEntries);

                    }
                  }
                }

                // transfer the image using BitBlt function.
                // It will probably end in the
                // call to driver's DrvCopyBits function.
                if (BitBlt(hMemDst, 0, 0, stBitmap.bmWidth, 
                      stBitmap.bmHeight, hMemSrc, 0, 0, SRCCOPY))
                  bConverted = true; // success

                VERIFY(SelectObject(hMemDst, hOldDst));
              }
              VERIFY(DeleteDC(hMemDst));
            }
            VERIFY(SelectObject(hMemSrc, hOldSrc));
          }
          VERIFY(DeleteDC(hMemSrc));
        }

        if (bConverted)
        {
          VERIFY(DeleteObject(hBitmap)); // it's no longer needed
          hBitmap = hDib;
        }
        else
          VERIFY(DeleteObject(hDib));
      }
      ReleaseDC(NULL, hScreen);
    }
  }
  return bConverted;
}
///////////////////////////////////////////////////////////////////////

C32BitImageProcessor::C32BitImageProcessor(BOOL bEnableWeighting) : m_bWeightingEnabled(bEnableWeighting)
{
}

C32BitImageProcessor::~C32BitImageProcessor()
{
}

CSize C32BitImageProcessor::CalcDestSize(CSize sizeSrc) 
{ 
	return sizeSrc; // default
}

BOOL C32BitImageProcessor::ProcessPixels(RGBX* pSrcPixels, CSize sizeSrc, RGBX* pDestPixels, CSize sizeDest)
{ 
	CopyMemory(pDestPixels, pSrcPixels, sizeDest.cx * 4 * sizeDest.cy); // default
	return TRUE;
}
 
// C32BitImageProcessor::CalcWeightedColor(...) is inlined in EnBitmap.h

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEgBitmap::CEgBitmap(COLORREF crBkgnd) : m_crBkgnd(crBkgnd)
{

	m_pSavedBmp=NULL;
	this->m_pBackGroundBmp=NULL;
	this->m_nOrigX=0;
	this->m_nOrigY=0;
}

CEgBitmap::~CEgBitmap()
{

}

BOOL CEgBitmap::LoadImage(UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst, COLORREF crBack)
{
	ASSERT(m_hObject == NULL);      // only attach once, detach on destroy

	if (m_hObject != NULL)
		return FALSE;

	return Attach(LoadImageResource(uIDRes, szResourceType, hInst, crBack == -1 ? m_crBkgnd : crBack));
}

BOOL CEgBitmap::LoadImage(LPCTSTR szImagePath, COLORREF crBack)
{
	ASSERT(m_hObject == NULL);      // only attach once, detach on destroy

	if (m_hObject != NULL)
		return FALSE;

	return Attach(LoadImageFile(szImagePath, crBack == -1 ? m_crBkgnd : crBack));
}

HBITMAP CEgBitmap::LoadImageFile(LPCTSTR szImagePath, COLORREF crBack)
{
	int nType = GetFileType(szImagePath);

	switch (nType)
	{
		case FT_BMP:
			// the reason for this is that i suspect it is more efficient to load
			// bmps this way since it avoids creating device contexts etc that the 
			// IPicture methods requires. that method however is still valuable
			// since it handles other image types and transparency
			return (HBITMAP)::LoadImage(NULL, szImagePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		case FT_UNKNOWN:
			return NULL;

		default: // all the rest
		{
			USES_CONVERSION;
			IPicture* pPicture = NULL;
			
			HBITMAP hbm = NULL;
			HRESULT hr = OleLoadPicturePath(T2OLE(szImagePath), NULL, 0, crBack, IID_IPicture, (LPVOID*)&pPicture);
					
			if (pPicture)
			{
				hbm = ExtractBitmap(pPicture, crBack);
				pPicture->Release();
			}

			return hbm;
		}
	}

	return NULL; // can't get here
}

HBITMAP CEgBitmap::LoadImageResource(UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst, COLORREF crBack)
{
	BYTE* pBuff = NULL;
	int nSize = 0;
	HBITMAP hbm = NULL;

	// first call is to get buffer size
	if (GetResource(MAKEINTRESOURCE(uIDRes), szResourceType, hInst, 0, nSize))
	{
		if (nSize > 0)
		{
			pBuff = new BYTE[nSize];
			
			// this loads it
			if (GetResource(MAKEINTRESOURCE(uIDRes), szResourceType, hInst, pBuff, nSize))
			{
				IPicture* pPicture = LoadFromBuffer(pBuff, nSize);

				if (pPicture)
				{
					hbm = ExtractBitmap(pPicture, crBack);
					pPicture->Release();
				}
			}
			
			delete [] pBuff;
		}
	}

	return hbm;
}

IPicture* CEgBitmap::LoadFromBuffer(BYTE* pBuff, int nSize)
{
	bool bResult = false;

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, nSize);
	void* pData = GlobalLock(hGlobal);
	memcpy(pData, pBuff, nSize);
	GlobalUnlock(hGlobal);

	IStream* pStream = NULL;
	IPicture* pPicture = NULL;

	if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK)
	{
		HRESULT hr = OleLoadPicture(pStream, nSize, FALSE, IID_IPicture, (LPVOID *)&pPicture);
		pStream->Release();
	}

	return pPicture; // caller releases
}

BOOL CEgBitmap::GetResource(LPCTSTR lpName, LPCTSTR lpType, HMODULE hInst, void* pResource, int& nBufSize)
{ 
	HRSRC		hResInfo;
	HANDLE		hRes;
	LPSTR		lpRes	= NULL; 
	int			nLen	= 0;
	bool		bResult	= FALSE;

	// Find the resource
	hResInfo = FindResource(hInst, lpName, lpType);

	if (hResInfo == NULL) 
		return false;

	// Load the resource
	hRes = LoadResource(hInst, hResInfo);

	if (hRes == NULL) 
		return false;

	// Lock the resource
	lpRes = (char*)LockResource(hRes);

	if (lpRes != NULL)
	{ 
		if (pResource == NULL)
		{
			nBufSize = SizeofResource(hInst, hResInfo);
			bResult = true;
		}
		else
		{
			if (nBufSize >= (int)SizeofResource(hInst, hResInfo))
			{
				memcpy(pResource, lpRes, nBufSize);
				bResult = true;
			}
		} 

		UnlockResource(hRes);  
	}

	// Free the resource
	FreeResource(hRes);

	return bResult;
}
HBITMAP CEgBitmap::CreateBitmapMask(COLORREF crTransparent,int x,int y,int width,int height)
{

    HDC hdcMem, hdcMem2;
	CDC dcMem,dcMem2;

    HBITMAP hbmMask;
    BITMAP bm;

    // Create monochrome (1 bit) mask bitmap.  


	this->GetBitmap(&bm);    
	if(width==0)
	{
		width==bm.bmWidth;
		height=bm.bmHeight;
	}
	hbmMask = ::CreateBitmap(width, height, 1, 1, NULL);

    // Get some HDCs that are compatible with the display driver

    dcMem.CreateCompatibleDC(0);
    dcMem2.CreateCompatibleDC(0);

	dcMem.SelectObject(this);
	dcMem2.SelectObject(hbmMask);

    // Set the background colour of the colour image to the colour
    // you want to be transparent.
	if(crTransparent==-1)
	{
		dcMem.SetBkColor(dcMem.GetPixel(x,y));
	}
	else
	{
		dcMem.SetBkColor(crTransparent);
	}


    // Copy the bits from the colour image to the B+W mask... everything
    // with the background colour ends up white while everythig else ends up
    // black...Just what we wanted.

    dcMem2.BitBlt(0, 0, width, height, &dcMem, x, y, SRCCOPY);

    // Take our new mask and use it to turn the transparent colour in our
    // original colour image to black so the transparency effect will
    // work right.
    dcMem.BitBlt( 0, 0, width, height, &dcMem2, 0, 0, SRCINVERT);

    // Clean up.

	dcMem.DeleteDC();
	dcMem2.DeleteDC();

    return hbmMask;

}
HBITMAP CEgBitmap::ExtractBitmap(IPicture* pPicture, COLORREF crBack)
{
	ASSERT(pPicture);

	if (!pPicture)
		return NULL;

	CBitmap bmMem;
	CDC dcMem;
	CDC* pDC = CWnd::GetDesktopWindow()->GetDC();

	if (dcMem.CreateCompatibleDC(pDC))
	{
		long hmWidth;
		long hmHeight;

		pPicture->get_Width(&hmWidth);
		pPicture->get_Height(&hmHeight);
		
		int nWidth	= MulDiv(hmWidth, pDC->GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);
		int nHeight	= MulDiv(hmHeight, pDC->GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);

		if (bmMem.CreateCompatibleBitmap(pDC, nWidth, nHeight))
		{
			CBitmap* pOldBM = dcMem.SelectObject(&bmMem);

			if (crBack != -1)
				dcMem.FillSolidRect(0, 0, nWidth, nHeight, crBack);
			
			HRESULT hr = pPicture->Render(dcMem, 0, 0, nWidth, nHeight, 0, hmHeight, hmWidth, -hmHeight, NULL);
			dcMem.SelectObject(pOldBM);
		}
	}

	CWnd::GetDesktopWindow()->ReleaseDC(pDC);

	return (HBITMAP)bmMem.Detach();
}

int CEgBitmap::GetFileType(LPCTSTR szImagePath)
{
	CString sPath(szImagePath);
	sPath.MakeUpper();

	if (sPath.Find(".BMP") > 0)
		return FT_BMP;

	else if (sPath.Find(".ICO") > 0)
		return FT_ICO;

	else if (sPath.Find(".JPG") > 0 || sPath.Find(".JPEG") > 0)
		return FT_JPG;

	else if (sPath.Find(".GIF") > 0)
		return FT_GIF;

	// else
	return FT_UNKNOWN;
}



BOOL CEgBitmap::ShearImage(int nHorz, int nVert, BOOL bEnableWeighting)
{
	return TRUE;
}

BOOL CEgBitmap::GrayImage()
{
	return TRUE;
}

BOOL CEgBitmap::BlurImage(int nAmount)
{
	return TRUE;
}

BOOL CEgBitmap::SharpenImage(int nAmount)
{
	return TRUE;
}

BOOL CEgBitmap::ResizeImage(double dFactor)
{
	return TRUE;
}

BOOL CEgBitmap::FlipImage(BOOL bHorz, BOOL bVert)
{
	return TRUE;
}

BOOL CEgBitmap::NegateImage()
{
	return TRUE;
}

BOOL CEgBitmap::ReplaceColor(COLORREF crFrom, COLORREF crTo)
{
	return TRUE;
}

BOOL CEgBitmap::RotateImage(int nDegrees, BOOL bEnableWeighting)
{
	return TRUE;
}

/*
双线性灰度级插值算法

四点确定一个平面函数，属于过约束问题；
问题描述：单位正方形顶点已知，求正方形内任一点的f(x,y)值


假设输出图像的宽度为W，高度为H；
输入图像的宽度为w高度为h，要将输入图像的尺度拉伸或压缩变换至输出图像的尺度；
按照线性插值的方法，将输入图像的宽度方向分为W等份，高度方向分为H等份；
那么输出图像中任意一点（x，y）的灰度值就应该由输入图像中四点（a，b）、（a+1，b）、（a，b+1）和（a+1，b+1）的灰度值来确定。其中a和b的值分别为

反向坐标变换
a=x*(w/W) 
b=y*(h/H)

f(x,y) = (b+1-y)f(x,b)+(y-b)f(x,b+1)
其中
f(x,b+1)=(x-a)f(a+1,b+1)+(a+1-x)f(a,b+1)
f(x,b)=(x-a)f(a+1,b)+(a+1-x)f(a,b)


详细部骤：
假设输出图像像素坐标是x,y,输入图像的宽度为w高度为h，假设输出图像的宽度为W，高度为H
求输入图像的坐标
Xa=x*w/W;
Ya=y*h/H;

1。 f(xa-1,ya) = 


*/
BOOL CEgBitmap:: Stretch(double sx,double sy)
{

	bool bAlphaGhost=false;
	float ghostBeiShu=0.8;

	BITMAP BM,BM2;
	if (!GetBitmap(&BM))
		return FALSE;
	RGBX* pSrcPixels = GetDIBits32();
	
	CSize sizeSrc(BM.bmWidth, BM.bmHeight);
	CSize sizeDest;
	
	sizeDest.cx=sizeSrc.cx*sx;
	sizeDest.cy=sizeSrc.cy*sy;

	CSize sizeInner;

	sizeInner.cx=sizeSrc.cx*sx*ghostBeiShu;
	sizeInner.cy=sizeSrc.cy*sy*ghostBeiShu;

	int ghost_offset_x=(-sizeInner.cx+sizeDest.cx)/2;
	int ghost_offset_y=(-sizeInner.cy+sizeDest.cy)/2;

	int offsetX,offsetY;
	int cx,cy;
	CEgBitmap* pBkBmp =this->GetBackGround(offsetX,offsetY);
	
	RGBX* pDestPixels ;
	if(pBkBmp!=NULL)
	{
		pBkBmp->GetBitmap(&BM2);
		pDestPixels = pBkBmp->GetDIBits32();

	}else
	{
		pDestPixels = new RGBX[sizeDest.cx * sizeDest.cy];
	}


	double xb1,xb,w,W,h,H;
	xb1=0.0;
	xb=0.0;
	w=sizeSrc.cx;
	h=sizeSrc.cy;
	W=sizeDest.cx;
	H=sizeDest.cx;
	int a,b;
	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;
	int nAlgo=3;
	for (int nX = 0; nX < sizeDest.cx; nX++)
	{
		for (int nY = 0; nY < sizeDest.cy; nY++)
		{
			//二次插值算法
#if(0)
			a=nX*(w/W);
			b=nY*(h/H);
			if(nAlgo==1)
			{
				xb1=(nX-a)*pSrcPixels[(b+1) * sizeSrc.cx + a+1].btRed
					+(a+1-nX)*pSrcPixels[(b+1) * sizeSrc.cx + a].btRed;
				xb=(nX-a)*pSrcPixels[(b) * sizeSrc.cx + a+1].btRed
					+(a+1-nX)*pSrcPixels[(b) * sizeSrc.cx + a].btRed;
				pDestPixels[nY*sizeDest.cx+nX].btRed=xb*(b+1-nY)+(nY-b)*xb1;

				xb1=(nX-a)*pSrcPixels[(b+1) * sizeSrc.cx + a+1].btBlue
					+(a+1-nX)*pSrcPixels[(b+1) * sizeSrc.cx + a].btBlue;
				xb=(nX-a)*pSrcPixels[(b) * sizeSrc.cx + a+1].btBlue
					+(a+1-nX)*pSrcPixels[(b) * sizeSrc.cx + a].btBlue;
				pDestPixels[nY*sizeDest.cx+nX].btBlue=xb*(b+1-nY)+(nY-b)*xb1;


				xb1=(nX-a)*pSrcPixels[(b+1) * sizeSrc.cx + a+1].btGreen 
					+(a+1-nX)*pSrcPixels[(b+1) * sizeSrc.cx + a].btGreen;
				xb=(nX-a)*pSrcPixels[(b) * sizeSrc.cx + a+1].btGreen
					+(a+1-nX)*pSrcPixels[(b) * sizeSrc.cx + a].btGreen;
				pDestPixels[nY*sizeDest.cx+nX].btGreen=xb*(b+1-nY)+(nY-b)*xb1;
			}

			//相邻取值算法
			if(nAlgo==2)
			{
				pDestPixels[nY*sizeDest.cx+nX].btRed=pSrcPixels[b*sizeSrc.cx+a].btRed;
				pDestPixels[nY*sizeDest.cx+nX].btGreen=pSrcPixels[b*sizeSrc.cx+a].btGreen;
				pDestPixels[nY*sizeDest.cx+nX].btBlue=pSrcPixels[b*sizeSrc.cx+a].btBlue;
			}
#endif
			//齐次坐标变换算法
			//Homogeneous coordinates
			if(nAlgo==3)
			{
				


				if(bAlphaGhost==true)
				{
					//由目标坐标求元坐标
					double fx=nX;
					double fy=nY;
					int a=(1/sx)*fx;
					int b=(1/sy)*fy;
					int dest_idx= (nY+offsetY)*BM2.bmWidth+nX+offsetX;
					int src_idx= b*sizeSrc.cx+a;
					pDestPixels[dest_idx].btRed=pSrcPixels[src_idx].btRed;
					pDestPixels[dest_idx].btGreen=pSrcPixels[src_idx].btGreen;
					pDestPixels[dest_idx].btBlue=pSrcPixels[src_idx].btBlue;	



					int innerX;
					int innerY;
					innerX=nX-ghost_offset_x;
					innerY=nY-ghost_offset_y;
					fx=innerX;
					fy=innerY;

					if(innerX>=0 && innerX<sizeInner.cx && innerY>=0 && innerY<sizeInner.cy)
					{
						float ghost_alpha = 50;
						a=(1/(sx*ghostBeiShu))*fx;
						b=(1/(sy*ghostBeiShu))*fy;
						
						dest_idx= (innerY+offsetY+ghost_offset_y)*BM2.bmWidth+innerX+offsetX+ghost_offset_x;
						src_idx= b*sizeSrc.cx+a;
	
						
						pDestPixels[dest_idx].btRed=pSrcPixels[src_idx].btRed*(1.0-ghost_alpha/255.0)
							+pDestPixels[dest_idx].btRed*(ghost_alpha/255.0);
						pDestPixels[dest_idx].btGreen=pSrcPixels[src_idx].btGreen*(1.0-ghost_alpha/255.0)
							+pDestPixels[dest_idx].btGreen*(ghost_alpha/255.0);
						pDestPixels[dest_idx].btBlue=pSrcPixels[src_idx].btBlue*(1.0-ghost_alpha/255.0)
							+pDestPixels[dest_idx].btBlue*(ghost_alpha/255.0);	
							
					}



				}else			
				{
					//由目标坐标求元坐标
					double fx=nX;
					double fy=nY;
					int a=(1/sx)*fx;
					int b=(1/sy)*fy;
					int dest_idx= (nY+offsetY)*BM2.bmWidth+nX+offsetX;
					int src_idx= b*sizeSrc.cx+a;
					pDestPixels[dest_idx].btRed=pSrcPixels[src_idx].btRed;
					pDestPixels[dest_idx].btGreen=pSrcPixels[src_idx].btGreen;
					pDestPixels[dest_idx].btBlue=pSrcPixels[src_idx].btBlue;
				}
			}

		}
	}
	TRACE ("CImageAlpha::ProcessPixels took %0.03f seconds\n", 
			 (GetTickCount() - dwTick) / 1000.0f);


	if (bRes)
	{
		// set the bits
		HDC hdc = GetDC(NULL);
		HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc,BM2.bmWidth,BM2.bmHeight);

		if (hbmSrc)
		{
			BITMAPINFO bi;

			if (PrepareBitmapInfo32(bi, hbmSrc))
			{
				if (SetDIBits(hdc, hbmSrc, 0, BM2.bmHeight, pDestPixels, &bi, DIB_RGB_COLORS))
				{
					// delete the bitmap and attach new
					DeleteObject();
					bRes = Attach(hbmSrc);
				}
			}

			::ReleaseDC(NULL, hdc);

			if (!bRes)
				::DeleteObject(hbmSrc);
		}
	}

	delete [] pSrcPixels;
	delete [] pDestPixels;

	return TRUE;


}

BOOL CEgBitmap::ZoomIn(float beishu)
{
	double sx=beishu;
	double sy=beishu;
	this->Stretch(sx,sy);
	return TRUE;
}
BOOL CEgBitmap::ZoomOut(float beishu)
{
	double sx=beishu;
	double sy=beishu;
	this->Stretch(sx,sy);
	return TRUE;
}

void CEgBitmap::SetOrig(int x,int y)
{
	m_nOrigX=x;
	m_nOrigY=y;
}
//当平面围绕y轴旋转时，在3维空间中只有y坐标不会变化，
//将平面投影到x,y平面时，需要确定观察点的位置及观察者的z坐标，然后再作计算。

//正变换求目标图Size
/*
	x3d = x2d*cosA;
	y3d = y2d;
	z3d = -x2d*sinA;

	Zview/(zView+x2d*sinA)=Xp/x3d;
	Zview/(zView+x2d*sinA)=Yp/y3d;

=>
	Zview*x3d=Xp*Zview+Xp*x2d*sinA;
	Zview*x2d*cosA-Xp*x2d*sina=xp*zview;
=>
	x2d=(xp*zview)/(zview*cosa-xp*sina)
	y2d=y3d=Yp*(zview+x2d*sina)/zview

	xp=(zview*x2d*cosa)/(zview+x2d*sina);
	yp=y2d*zview/(zview+x2d*sina)
*/

BOOL CEgBitmap:: GetRotateYDestSize(int& x1,int& y1,int& x2,int& y2,double degree_y,double view_z)
{
	double cosA=0;
	double sinA=0;
	if(degree_y!=0)
	{
		cosA=cos((degree_y/180.0)*PI);
		sinA=sin((degree_y/180.0)*PI);
	}
	BITMAP BM,BM2;
	if (!GetBitmap(&BM))
		return FALSE;

	double xo=BM.bmWidth/2;
	double yo=BM.bmHeight/2;

	double x2d=-xo;
	double y2d=-yo;
	double xp1,yp1;
	double xmax,ymax;
	double xmin,ymin;
	xp1=(view_z*x2d*cosA)/(view_z+x2d*sinA);
	yp1=(y2d*view_z)/(view_z+x2d*sinA);
	xmax=(xp1);
	ymax=(yp1);
	xmin=(xp1);
	ymin=(yp1);

	x2d=xo;
	y2d=yo;
	xp1=(view_z*x2d*cosA)/(view_z+x2d*sinA);
	yp1=(y2d*view_z)/(view_z+x2d*sinA);
	xmax=MAX(xmax,(xp1));
	ymax=MAX(ymax,(yp1));
	xmin=MIN(xmin,(xp1));
	ymin=MIN(ymin,(yp1));



	x2d=-xo;
	y2d=yo;
	xp1=(view_z*x2d*cosA)/(view_z+x2d*sinA);
	yp1=(y2d*view_z)/(view_z+x2d*sinA);
	xmax=MAX(xmax,(xp1));
	ymax=MAX(ymax,(yp1));
	xmin=MIN(xmin,(xp1));
	ymin=MIN(ymin,(yp1));
	x2d=xo;
	y2d=-yo;
	xp1=(view_z*x2d*cosA)/(view_z+x2d*sinA);
	yp1=(y2d*view_z)/(view_z+x2d*sinA);
	xmax=MAX(xmax,(xp1));
	ymax=MAX(ymax,(yp1));
	xmin=MIN(xmin,(xp1));
	ymin=MIN(ymin,(yp1));

	x1=xmin;
	y1=ymin;
	x2=xmax;
	y2=ymax;
}

//反向坐标变换求元图坐标
/*
	x3d = x2d*cosA;
	y3d = y2d;
	z3d = -x2d*sinA;

	Zview/(zView+x2d*sinA)=Xp/x3d;
	Zview/(zView+x2d*sinA)=Yp/y3d;

=>
	Zview*x3d=Xp*Zview+Xp*x2d*sinA;
	Zview*x2d*cosA-Xp*x2d*sina=xp*zview;
=>
	x2d=(xp*zview)/(zview*cosa-xp*sina)
	y2d=y3d=Yp*(zview+x2d*sina)/zview
*/

BOOL CEgBitmap:: Rotate_y(double degree_y,double view_z,int nAlpha)
{

	double cosA=0;
	double sinA=0;	
	if(degree_y!=0)
	{
		cosA=cos((degree_y/180.0)*PI);
		sinA=sin((degree_y/180.0)*PI);
	}

	BITMAP BM,BM2;
	if (!GetBitmap(&BM))
		return FALSE;
	RGBX* pSrcPixels = GetDIBits32();
	
	
	CSize sizeSrc(BM.bmWidth, BM.bmHeight);


	int x1,x2,y1,y2;
	GetRotateYDestSize(x1,y1,x2,y2,degree_y,view_z);



	int cy=y2-y1;
	int cx=x2-x1;
	int offsetX,offsetY;
	CEgBitmap* pBkBmp =this->GetBackGround(offsetX,offsetY);
	RGBX* pDestPixels ;
	if(pBkBmp!=NULL)
	{
		pBkBmp->GetBitmap(&BM2);
		pDestPixels = pBkBmp->GetDIBits32();

	}else
	{
		pDestPixels = new RGBX[cx * cy];
	}

	//Fill(pDestPixels,sizeDest,0xFF);
	int a,b;
	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;
	int nAlgo=3;
	double x2d,y2d;
	double Vz=view_z;
	double x3d,y3d,z3d;
	int dest_idx;
	int src_idx;

	//反向坐标变换


	for (int Xp =x1; Xp <x2; Xp++)
	{
		for (int Yp =y1; Yp <y2; Yp++)
		{
			if(degree_y!=0)
			{				

		

				x2d=(Vz*Xp)/(cosA*Vz-sinA*Xp);		
				y2d=((Vz+x2d*sinA)/Vz)*Yp;
				//y2d=(1+(x2d*sinA/(Vz)))*Yp;
			
			
				if(x2d>sizeSrc.cx/2 || y2d>sizeSrc.cy/2 || x2d<-sizeSrc.cx/2 || y2d<-sizeSrc.cy/2)
				{
					continue;
				}

				y2d+=sizeSrc.cy/2;
				x2d+=sizeSrc.cx/2;
				dest_idx=(int)((Yp-y1+offsetY)*BM2.bmWidth)+(int)(Xp-x1+offsetX);
				src_idx=(int)y2d*sizeSrc.cx+(int)x2d;



				if(nAlpha==255)
				{
					pDestPixels[dest_idx].btRed=pSrcPixels[src_idx].btRed;
					pDestPixels[dest_idx].btGreen=pSrcPixels[src_idx].btGreen;
					pDestPixels[dest_idx].btBlue=pSrcPixels[src_idx].btBlue;			
				}else
				{
					pDestPixels[dest_idx].btRed=pDestPixels[dest_idx].btRed*(1.0-nAlpha/255.0)+pSrcPixels[src_idx].btRed*(nAlpha/255.0);
					pDestPixels[dest_idx].btGreen=pDestPixels[dest_idx].btGreen*(1.0-nAlpha/255.0)+
						pSrcPixels[src_idx].btGreen*(nAlpha/255.0);
					pDestPixels[dest_idx].btBlue=pDestPixels[dest_idx].btBlue*(1.0-nAlpha/255.0)+pSrcPixels[src_idx].btBlue*(nAlpha/255.0);	
				}
			}

		}
	}
	TRACE ("CImageAlpha::ProcessPixels took %0.03f seconds\n", 
			 (GetTickCount() - dwTick) / 1000.0f);


	if (bRes)
	{
		// set the bits
		HDC hdc = GetDC(NULL);
		
		HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc, BM2.bmWidth, BM2.bmHeight);

		if (hbmSrc)
		{
			BITMAPINFO bi;

			if (PrepareBitmapInfo32(bi, hbmSrc))
			{
				if (SetDIBits(hdc, hbmSrc, 0, BM2.bmHeight, pDestPixels, &bi, DIB_RGB_COLORS))
				{
					// delete the bitmap and attach new
					DeleteObject();
					bRes = Attach(hbmSrc);
				}
			}

			::ReleaseDC(NULL, hdc);

			if (!bRes)
				::DeleteObject(hbmSrc);
		}
	}

	delete [] pSrcPixels;
	delete [] pDestPixels;

	return TRUE;


}

//拽物函数变换
// g(x)=2/(exp(x)+exp(-x));
void CEgBitmap::TractrixX(float y1,float y2)
{


	BITMAP BM,BM2;
	if (!GetBitmap(&BM))
		return ;
	RGBX* pSrcPixels = GetDIBits32();
	
	CSize sizeSrc(BM.bmWidth, BM.bmHeight);
	CSize sizeDest;
	


	int offsetX,offsetY;
	int cx,cy;
	CEgBitmap* pBkBmp =this->GetBackGround(offsetX,offsetY);
	cx=sizeSrc.cx;
	cy=sizeSrc.cy;
	RGBX* pDestPixels ;
	if(pBkBmp!=NULL)
	{
		pBkBmp->GetBitmap(&BM2);
		pDestPixels = pBkBmp->GetDIBits32();

	}else
	{
		offsetX=0;
		offsetY=0;
		BM2.bmWidth=BM.bmWidth;
		BM2.bmHeight=BM.bmHeight;
		pDestPixels = new RGBX[cx * cy];
	}


	
	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;
	float dy_max=(y2-y1)/2;
	float y_o = y1+(y2-y1)/2;
	float a=10;
	float b=dy_max;
	for (int nX = 0; nX < cx; nX++)
	{
		for (int nY = 0; nY < cy; nY++)
		{
			
					int dx=0;
					int dest_idx,src_idx;
					src_idx=nY*cx+nX;
				

					dx=a/(exp((nY-y_o)/(b))+exp((y_o-nY)/(b)));
				

					if(dx>=0)
					{
						dx=nX-dx;
					}
					if(dx>=0 && dx<cx)
					{
						dest_idx=(nY+offsetY)*cx+dx+offsetX;
						pDestPixels[dest_idx].btRed=pSrcPixels[src_idx].btRed;
						pDestPixels[dest_idx].btGreen=pSrcPixels[src_idx].btGreen;
						pDestPixels[dest_idx].btBlue=pSrcPixels[src_idx].btBlue;		
					}
				
		
		}
	}
	TRACE ("CImageAlpha::ProcessPixels took %0.03f seconds\n", 
			 (GetTickCount() - dwTick) / 1000.0f);


	if (bRes)
	{
		// set the bits
		HDC hdc = GetDC(NULL);
		HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc,BM2.bmWidth,BM2.bmHeight);

		if (hbmSrc)
		{
			BITMAPINFO bi;

			if (PrepareBitmapInfo32(bi, hbmSrc))
			{
				if (SetDIBits(hdc, hbmSrc, 0, BM2.bmHeight, pDestPixels, &bi, DIB_RGB_COLORS))
				{
					// delete the bitmap and attach new
					DeleteObject();
					bRes = Attach(hbmSrc);
				}
			}

			::ReleaseDC(NULL, hdc);

			if (!bRes)
				::DeleteObject(hbmSrc);
		}
	}

	delete [] pSrcPixels;
	delete [] pDestPixels;

	return ;
}
BOOL CEgBitmap:: AlphaBlend(double nAlpha)
{
	BITMAP BM,BM2;
	if (!GetBitmap(&BM))
		return FALSE;
	CSize sizeSrc(BM.bmWidth, BM.bmHeight);
	int offsetX,offsetY;
	offsetX=offsetY=0;
	CEgBitmap* pBkBmp =this->GetBackGround(offsetX,offsetY);
	if(pBkBmp==NULL) return FALSE;
	pBkBmp->GetBitmap(&BM2);
	CSize sizeDest(BM2.bmWidth, BM2.bmHeight);
	// prepare src and dest bits
	RGBX* pSrcPixels = GetDIBits32();
	RGBX* pDestPixels = pBkBmp->GetDIBits32();

	ASSERT(pDestPixels!=NULL);
	ASSERT(pSrcPixels!=NULL);

	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;

	for (int nX = 0; nX < sizeSrc.cx; nX++)
	{
		for (int nY = 0; nY < sizeSrc.cy; nY++)
		{
			if((nY+offsetY) >= sizeDest.cy) break;
			if((nX+offsetX) >= sizeDest.cx) break;
			pSrcPixels[nY * sizeSrc.cx + nX].btBlue  = pDestPixels[(nY+offsetY) * sizeDest.cx + nX+offsetX].btBlue*(1.0-nAlpha/255.0)+ 
				pSrcPixels[nY * sizeSrc.cx + nX].btBlue*(nAlpha/255.0);
			pSrcPixels[nY * sizeSrc.cx + nX].btRed  = pDestPixels[(nY+offsetY) * sizeDest.cx + nX+offsetX].btRed*(1.0-nAlpha/255.0)+ 
				pSrcPixels[nY * sizeSrc.cx + nX].btRed*(nAlpha/255.0);
			pSrcPixels[nY * sizeSrc.cx + nX].btGreen   = pDestPixels[(nY+offsetY) * sizeDest.cx + nX+offsetX].btGreen*(1.0-nAlpha/255.0)+ 
				pSrcPixels[nY * sizeSrc.cx + nX].btGreen*(nAlpha/255.0);
		}
	}


	TRACE ("CImageAlpha::ProcessPixels took %0.03f seconds\n", 
			 (GetTickCount() - dwTick) / 1000.0f);


	if (bRes)
	{
		// set the bits
		HDC hdc = GetDC(NULL);
		HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc, sizeSrc.cx, sizeSrc.cy);

		if (hbmSrc)
		{
			BITMAPINFO bi;

			if (PrepareBitmapInfo32(bi, hbmSrc))
			{
				if (SetDIBits(hdc, hbmSrc, 0, sizeSrc.cy, pSrcPixels, &bi, DIB_RGB_COLORS))
				{
					// delete the bitmap and attach new
					DeleteObject();
					bRes = Attach(hbmSrc);
				}
			}

			::ReleaseDC(NULL, hdc);

			if (!bRes)
				::DeleteObject(hbmSrc);
		}
	}

	delete [] pSrcPixels;
	delete [] pDestPixels;

}


BOOL CEgBitmap:: DaoYin(CEgBitmap* pBackGround,bool bX)
{
	BITMAP BM,BM2;
	if (!GetBitmap(&BM))
		return FALSE;

	if (!pBackGround->GetBitmap(&BM2))
		return FALSE;

	RGBX* pSrcPixels = GetDIBits32();
	RGBX* pDestPixels = pBackGround->GetDIBits32();
	ASSERT(pDestPixels!=NULL);
	ASSERT(pSrcPixels!=NULL);

	DWORD dwTick = GetTickCount();
	BOOL bRes = TRUE;

	int cx2=BM2.bmWidth;
	int cy2=BM2.bmHeight;
	//fixed nAlpha=itofx(100);
	//fixed dAlpha=Divfx(nAlpha,itofx(cy2));
	float nAlpha=80;
	float dAlpha=(nAlpha/cy2);
	for (int nY = 0; nY < cy2; nY++)
	{
		
		nAlpha-=(dAlpha);



		if(nAlpha<0)
		{
			nAlpha=0;
		}
		
		float alpha1=(nAlpha/255.0);
		float alpha2=(1.0-nAlpha/255.0);
		for (int nX = 0; nX <cx2 ; nX++)
		{

			int srcX=nX;
			int srcY=BM.bmHeight-nY-1;
			

			pDestPixels[nY * cx2 + nX].btBlue  = pSrcPixels[srcY*BM.bmWidth+srcX].btBlue*alpha1+ 
																	pDestPixels[nY * cx2 + nX].btBlue*alpha2;
			pDestPixels[nY * cx2 + nX].btGreen  = pSrcPixels[srcY*BM.bmWidth+srcX].btGreen*alpha1+ 
																	pDestPixels[nY * cx2 + nX].btGreen*alpha2;
			pDestPixels[nY * cx2 + nX].btRed   = pSrcPixels[srcY*BM.bmWidth+srcX].btRed*alpha1+ 
																	pDestPixels[nY * cx2 + nX].btRed*alpha2;
	
		}
	}

TRACE ("CImageAlpha::ProcessPixels took %0.03f seconds\n", 
			 (GetTickCount() - dwTick) / 1000.0f);


	{
		// set the bits
		HDC hdc = GetDC(NULL);
		HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc, cx2, cy2);

		if (hbmSrc)
		{
			BITMAPINFO bi;

			if (PrepareBitmapInfo32(bi, hbmSrc))
			{
				if (SetDIBits(hdc, hbmSrc, 0, cy2, pDestPixels, &bi, DIB_RGB_COLORS))
				{
					// delete the bitmap and attach new
					DeleteObject();
					Attach(hbmSrc);
				}
			}

			::ReleaseDC(NULL, hdc);

			if (!bRes)
				::DeleteObject(hbmSrc);
		}
	}

	delete [] pSrcPixels;
	delete [] pDestPixels;

}

RGBX* CEgBitmap::GetDIBits32()
{
	BITMAPINFO bi;

	int nHeight = PrepareBitmapInfo32(bi);
	
	if (!nHeight)
		return FALSE;

	BYTE* pBits = (BYTE*)new BYTE[bi.bmiHeader.biSizeImage];
	HDC hdc = GetDC(NULL);

	if (!GetDIBits(hdc, (HBITMAP)GetSafeHandle(), 0, nHeight, pBits, &bi, DIB_RGB_COLORS))
	{
		delete pBits;
		pBits = NULL;
	}

	::ReleaseDC(NULL, hdc);

	return (RGBX*)pBits;
}

BOOL CEgBitmap::PrepareBitmapInfo32(BITMAPINFO& bi, HBITMAP hBitmap)
{
	if (!hBitmap)
		hBitmap = (HBITMAP)GetSafeHandle();

	BITMAP BM;

	if (!::GetObject(hBitmap, sizeof(BM), &BM))
		return FALSE;

	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = BM.bmWidth;
	bi.bmiHeader.biHeight = -BM.bmHeight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32; // 32 bit
	bi.bmiHeader.biCompression = BI_RGB; // 32 bit
	bi.bmiHeader.biSizeImage = BM.bmWidth * 4 * BM.bmHeight; // 32 bit
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	return BM.bmHeight;
}

BOOL CEgBitmap::CopyImage(HBITMAP hBitmap)
{
	ASSERT (hBitmap);
	
	if (!hBitmap)
		return FALSE;
	
	BITMAPINFO bi;
	int nHeight = PrepareBitmapInfo32(bi, hBitmap);

	if (!nHeight)
		return FALSE;

	BYTE* pBits = (BYTE*)new BYTE[bi.bmiHeader.biSizeImage];
	HDC hdc = GetDC(NULL);
	BOOL bRes = FALSE;

	if (GetDIBits(hdc, hBitmap, 0, nHeight, pBits, &bi, DIB_RGB_COLORS))
	{
		int nWidth = bi.bmiHeader.biSizeImage / (nHeight * 4);

		HBITMAP hbmDest = ::CreateCompatibleBitmap(hdc, nWidth, nHeight);

		if (hbmDest)
		{
			if (SetDIBits(hdc, hbmDest, 0, nHeight, pBits, &bi, DIB_RGB_COLORS))
			{
				DeleteObject();
				bRes = Attach(hbmDest);
			}
		}
	}

	::ReleaseDC(NULL, hdc);
	delete [] pBits;

	return bRes;
}

BOOL CEgBitmap::CopyImage(CBitmap* pBitmap)
{
	if (!pBitmap)
		return FALSE;

	return CopyImage((HBITMAP)pBitmap->GetSafeHandle());
}

BOOL CEgBitmap::Fill(RGBX* pPixels, CSize size, COLORREF color)
{
	if (!pPixels)
		return FALSE;

	if (color == -1 || color == RGB(255, 255, 255))
		FillMemory(pPixels, size.cx * 4 * size.cy, 255); // white

	else if (color == 0)
		FillMemory(pPixels, size.cx * 4 * size.cy, 0); // black

	else
	{
		// fill the first line with the color
		RGBX* pLine = &pPixels[0];
		int nSize = 1;

		pLine[0] = RGBX(color);

		while (1)
		{
			if (nSize > size.cx)
				break;

			// else
			int nAmount = min(size.cx - nSize, nSize) * 4;

			CopyMemory(&pLine[nSize], pLine, nAmount);
			nSize *= 2;
		}

		// use that line to fill the rest of the block
		int nRow = 1;

		while (1)
		{
			if (nRow > size.cy)
				break;

			// else
			int nAmount = min(size.cy - nRow, nRow) * size.cx * 4;

			CopyMemory(&pPixels[nRow * size.cx], pPixels, nAmount);
			nRow *= 2;
		}
	}

	return TRUE;
}
