#include "ViewManager.h"
	CViewManager::CViewManager(CContainerWidget* pContainer){
		ASSERT(pContainer!=NULL);
		m_pContainer=pContainer;
		m_pCurActiveView=NULL;
	}
	CViewManager::~CViewManager()
	{
		m_viewList.clear();
	}
	bool CViewManager::AddView(CViewContainer* pView,unsigned int viewIndex)
	{
		ASSERT(pView!=NULL);
		ASSERT(viewIndex!=INVALID_VIEW_INDEX);
		m_viewList.add(pView);
		pView->SetViewIndex(viewIndex);
		m_pContainer->InsertWidget(pView);
		pView->OnInitialUpdate();
		return true;
	}
	bool CViewManager::RemoveView(CViewContainer* pView){
		m_viewList.remove(pView);
		m_pContainer->RemoveWidget(pView);
		return true;
	}
	bool CViewManager::RemoveView(unsigned int viewIndex){

		CObjItem* pItem=NULL;
		while(true)
		{
			pItem=m_viewList.Next(pItem);
			if(pItem!=NULL)
			{
				CViewContainer* pView=(CViewContainer*)(pItem->m_pData);
				if(pView->GetViewIndex()==viewIndex)
				{
					RemoveView(pView);
					break;
				}
			}else
			{
				return false;
			}			
		}
		return true;
	}	
	bool CViewManager::SwitchView(CViewContainer* pView){

		if(m_pCurActiveView!=NULL)
		{
			m_pCurActiveView->DisableMouse();
			m_pCurActiveView->OnViewSwitchOut();
	

		}
		m_pCurActiveView=pView;
		if(m_pCurActiveView!=NULL)
		{
			m_pCurActiveView->EnableMouse();
			m_pCurActiveView->OnViewSwitchIn();
	
		}
		return true;
	}
	bool CViewManager::SwitchView(unsigned int viewIndex){
		return true;
	}

	unsigned int CViewManager::GetActiveViewIndex()
	{
		if(m_pCurActiveView!=NULL)
		{
			return m_pCurActiveView->GetViewIndex();
		}
		return INVALID_VIEW_INDEX;
	}
	CViewContainer* CViewManager::GetActiveView()
	{
		return m_pCurActiveView;
	}




	CViewContainer::CViewContainer()
	{
	}
	CViewContainer::~CViewContainer(){
	}
	void CViewContainer::OnInitialUpdate(){
	}
	void CViewContainer::OnViewSwitchOut(){
		this->SetVisible(false);
	}
	void CViewContainer::OnViewSwitchIn(){
		this->SetVisible(true);
	}
	void CViewContainer::SetViewIndex(unsigned int index){
		this->m_nViewIndex=index;
	}
	unsigned int CViewContainer::GetViewIndex(){
		return this->m_nViewIndex;
	}
