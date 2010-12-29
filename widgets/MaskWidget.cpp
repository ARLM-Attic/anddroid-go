#include "CMaskWidget.h"
CMaskWidget::CMaskWidget(void){
		m_pMaskedWidget=NULL;
	};
	void CMaskWidget::Mask(CWidget* pWidget,int nAlpha){		

		if(pWidget==NULL)
		{
			this->SetVisible(false);
			return;
		}
		else
		{
			this->SetVisible(true);
		}
		this->SetAlpha(nAlpha);
		m_pMaskedWidget=pWidget;	
		WRect rc;
		pWidget->GetRectAsScreen(&rc);

		this->GetContainer()->ScreenToClient(&rc);
		this->SetRect(&rc);
		this->InvalidateContent(0);


	};
	CWidget* CMaskWidget::GetMaskedWidget()
	{
		return m_pMaskedWidget;
	}

		int CMaskWidget::CustDraw(CFrameSurface* pSurface)
	{
		if(m_pMaskedWidget==NULL) return 0;
		
		WRect rc;
		GetRectAsScreen(&rc);
		//TRACE("(%d,%d,%d,%d)\n",rc.x,rc.y,rc.dx,rc.dy);
		pSurface->FillRect(&rc,this->m_clrBackGround);

		return 0;
	}