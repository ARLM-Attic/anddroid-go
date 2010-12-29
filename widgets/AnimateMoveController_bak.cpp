
#include "animatemovecontroller.h"
#include "DisplayManage.h"
#define ANIMATE_STEPS		5
#define TOP_OFFSET		20
CAnimateMoveController::CAnimateMoveController(void)
{
	m_xMath=0;
	m_curX=0;
	m_nAlpha=0;

}

CAnimateMoveController::~CAnimateMoveController(void)
{
}

int CAnimateMoveController::Start(void* pUserData)
{
	CAnimateController::Start();
	m_nAlpha=0;
	
	return 0;
}

int CAnimateMoveController::Update(void)
{
	CAnimateController::Update();
	//CWidget* pWidget=(CWidget*)m_pUserData;
	if(m_bStarted==false) return 0;

//	NextStep();
	return 0;
}
#if(0)
int CAnimateMoveController::NextStepSin()
{
	CWidget* pWidget=(CWidget*)m_pUserData;

	if(m_nAnimateDelay>0)
	{
		m_nAnimateDelay--;
		//still need send notify though I have none change happen.
		this->m_animateModel.NotifyEvent(ANIMATE_WIDGET_UPDATE_READY,(int)(m_pUserData));
		return 0;
	}

	{

		if(m_bStarted==false)
		{
			//still need send notify though I have none change happen.
			this->m_animateModel.NotifyEvent(ANIMATE_WIDGET_UPDATE_READY,(int)(m_pUserData));
			return 0;
		}
	}


	{

		//float PI	= 3.1415926;

		float xOrig = m_initX;
		float yOrig	= m_initY;

		//conver to standard XY cordinates

		WRect rcW;
		pWidget->GetRect(&rcW);

		float xTar=m_maxX;
		float yTar=m_maxY;
		float dxTar=rcW.dx;
		float dyTar=rcW.dy;
		

		xTar-=xOrig;
		yTar-=yOrig;

		
		float fa=1;

		float ff=(PI/2)/(11/10*abs(xTar));

		float fp=0;

		
		if(xTar<0 && yTar<0)
		{
			m_curX-=abs(xTar)/ANIMATE_STEPS;
			fp=PI;
		
		}
		if(xTar>0 && yTar<0)
		{
			m_curX+=abs(xTar)/ANIMATE_STEPS;
			fp=-PI;
		
		}

		m_curY=fa*sin(ff*m_curX+fp);
		m_curY*=(TOP_OFFSET+abs(yTar));

		rcW.x=m_curX+xOrig;
		rcW.y=m_curY+yOrig;

		if(m_t==ANIMATE_STEPS)
		{
			rcW.x=m_maxX;
			rcW.y=m_maxY;
			if(yTar<0)
			{
				m_nAlpha=225;
			}
			else
			{
				m_nAlpha=0;
			}
			this->Stop();
		}

		//pWidget->SetAlpha(m_nAlpha);

		pWidget->SetRect(&rcW,true);

	
		//set flag indicate the widget is updated;
		pWidget->m_nAnimateFlag=ANIMATE_MOVE;
		//Notify listener for the further operation,
		//For example, in the CAnimateGridContainer, which we use the event to refresh the client area.
		this->m_animateModel.NotifyEvent(ANIMATE_WIDGET_UPDATE_READY,(int)(m_pUserData));


	}

}


int CAnimateMoveController::NextStep()
{
	CWidget* pWidget=(CWidget*)m_pUserData;

	if(m_nAnimateDelay>0)
	{
		m_nAnimateDelay--;
		//still need send notify though I have none change happen.
		this->m_animateModel.NotifyEvent(ANIMATE_WIDGET_UPDATE_READY,(int)(m_pUserData));
		return 0;
	}

	{

		if(m_bStarted==false)
		{
			//still need send notify though I have none change happen.
			this->m_animateModel.NotifyEvent(ANIMATE_WIDGET_UPDATE_READY,(int)(m_pUserData));
			return 0;
		}
	}


	//Calculate the move sin math function
	{

		//float PI	= 3.1415926;

		float xOrig = m_initX;
		float yOrig	= m_initY;

		//conver to standard XY cordinates

		WRect rcW;
		pWidget->GetRect(&rcW);

		float xTar=m_maxX;
		float yTar=m_maxY;
		float dxTar=rcW.dx;
		float dyTar=rcW.dy;
		

		xTar-=xOrig;
		yTar-=yOrig;



		float k;
		float b;
		float k2;
		float b2;

		if(xTar!=0)
		{
		
			if(yTar<0)
			{
				k=(yTar+TOP_OFFSET)/(xTar/2);
				b=0;
				k2=(TOP_OFFSET)/(xTar/2);
				b2=yTar-k2*xTar;
				
			}else
			{
				k=(-TOP_OFFSET)/(xTar/2);
				b=0;

				k2=(yTar+TOP_OFFSET)/(xTar/2);
				b2=yTar-k2*xTar;
				if(m_nAlpha==0)
				{
					m_nAlpha=255;
				}
				
			}

			if(xTar<0 )
			{
				m_curX-=abs(xTar)/ANIMATE_STEPS;

				
		
			}
			if(xTar>0)
			{
				m_curX+=abs(xTar)/ANIMATE_STEPS;
			
			}


			if(abs(m_curX)>abs(xTar)/2)
			{
				k=k2;
				b=b2;
			}

			m_curY=k*m_curX+b;

		}
		//xTar==0;
		else
		{
			
				m_curX=xTar;
				if(yTar>0)
				{
					m_curY+=yTar/ANIMATE_STEPS;
				}else
				{
					m_curY-=yTar/ANIMATE_STEPS;
				}
		}

			if(yTar<0)
			{
				m_nAlpha+=180/ANIMATE_STEPS;
			}else
			{
				if(m_nAlpha==0)
				{
					m_nAlpha=255;
				}
				m_nAlpha-=180/ANIMATE_STEPS;
			
			}



		





		rcW.x=m_curX+xOrig;
		rcW.y=m_curY+yOrig;

		
	
		
		if(m_t==ANIMATE_STEPS)
		{
			rcW.x=m_maxX;
			rcW.y=m_maxY;
			if(yTar<0)
			{
				m_nAlpha=225;
			}
			else
			{
				m_nAlpha=0;
			}
			this->Stop();
		}

		pWidget->SetAlpha(m_nAlpha);

	
		pWidget->SetRect(&rcW,true);

	
		//set flag indicate the widget is updated;
		pWidget->m_nAnimateFlag=ANIMATE_MOVE;
		//Notify listener for the further operation,
		//For example, in the CAnimateGridContainer, which we use the event to refresh the client area.
		this->m_animateModel.NotifyEvent(ANIMATE_WIDGET_UPDATE_READY,(int)(m_pUserData));

	//	pWidget->m_nAnimateFlag=ANIMATE_NO_CHECK;

	}
	

	return 0;
}
#endif