/*
eGUI Copyright (c) 2007 Xiao Wang Yang 
Author: Xiao Wang Yang
Email: sureone@gmail.com   http://code.google.com/p/easygui
*/
#include "ImageWidget.h"

CImageWidget::CImageWidget(void)
{
	memset(m_pImageFile,0,sizeof(m_pImageFile));
	m_pLabel=0;
	m_imageHandle=0;
	
	this->m_widgetType=WIDGET_TYPE_IMAGE;
	m_bTransparent=false;

}

CImageWidget::~CImageWidget(void)
{

}

int CImageWidget::CustDraw(CFrameSurface* pSurface)
{
	WRect rc;
	GetRectAsScreen(&rc);
	if(m_pImageFile[0]!=NULL)
	{
		if(m_imageHandle==0)
		{
			m_imageHandle=GetDisplayManage()->LoadImage(this->m_pImageFile,this->m_img_draw_cmd);
		}

		pSurface->DrawImage(m_imageHandle,&rc,false,m_img_draw_cmd);
	}
	return 0;
}
int CImageWidget::Draw(CFrameSurface* pSurface)
{



	CWidget::Draw(pSurface);


	return 0;
}