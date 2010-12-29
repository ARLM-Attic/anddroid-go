#include "messagewidget.h"

CMessageWidget::CMessageWidget(void)
{

}

CMessageWidget::~CMessageWidget(void)
{
}
int CMessageWidget::CustDraw(CFrameSurface* pSurface)
{

	CImageWidget::CustDraw(pSurface);
	WRect rc;
	GetRectAsScreen(&rc);


	uint8* str=(uint8*)(m_str.GetBuffer(0));
	int len=m_str.GetLength();
	int wlen=len+1;
	uint16* wstr=(uint16*)EG_MALLOC(wlen*sizeof(uint16));


	str2wstr(str,len,wstr,wlen);


	
	pSurface->DrawText(wstr,len,
			&rc,DT_CENTER|DT_WORDBREAK);
	return 0;

}