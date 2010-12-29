#include "StaticTextWidget.h"
int CStaticTextWidget::CustDraw(CFrameSurface* pSurface)
	{
		
		WRect rc;
		GetRectAsScreen(&rc);
		//TRACE("(%d,%d,%d,%d)\n",rc.x,rc.y,rc.dx,rc.dy);
		pSurface->FillRect(&rc,this->m_clrBackGround);
		

		pSurface->MoveTo(rc.x,rc.y+rc.dy-1);
		pSurface->LineTo(rc.x+rc.dx-1,rc.y+rc.dy-1,0x0);		
		
		rc.x++;
		rc.dx-=2;
		rc.dy-=2;
		rc.y++;

		
		uint8* str=(uint8*)(m_strText.GetBuffer(0));
		int len=m_strText.GetLength();
		int wlen=len+1;
		uint16* wstr=(uint16*)EG_MALLOC(wlen*sizeof(uint16));


		wlen=str2wstr(str,len,wstr,wlen);


		//wchar_t   pText[]=L"HÄãºÃ";
		//wlen=wstrlen(pText);

		pSurface->DrawText(wstr,wlen,
			&rc,DT_CENTER|DT_WORDBREAK);
		EG_FREE(wstr);


		return 0;

	}