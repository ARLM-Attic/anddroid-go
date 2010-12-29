
#include "umlwidget.h"

CUMLWidget::CUMLWidget(void)
{
}

CUMLWidget::~CUMLWidget(void)
{
}



int CUMLWidget::SetModel(CModel* pModel)
{
	m_pModel=pModel;
	return 0;
}

CModel* CUMLWidget::GetModel(void)
{
	return m_pModel;
}










int CUMLWidget::IsFocus(void)
{
	return  m_bFocus;
	return 0;
}
