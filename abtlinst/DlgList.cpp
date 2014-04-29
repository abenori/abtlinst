#include "stdafx.h"
#include "DlgList.h"
#include <atlstr.h>

bool DlgList::ShowList(HWND parent,const ablib::string &title,const ablib::string &msg,std::vector<ablib::string> &list){
	m_title = title;
	m_rv = list;
	m_msg = msg;
	int r = DoModal(parent);
	if(r){
		list = m_rv;
		return true;
	}else return false;
}


BOOL DlgList::OnInitDialog(CWindow wndFocus, LPARAM lInitParam){
	this->SetWindowText(m_title.c_str());
	CStatic(this->GetDlgItem(IDC_MSG)).SetWindowText(m_msg.c_str());
	CListViewCtrl list(this->GetDlgItem(IDC_LIST));
	list.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | list.GetExtendedListViewStyle());
	list.AddColumn(_T(""),0);
	RECT rect;
	list.GetWindowRect(&rect);
	list.SetColumnWidth(0,rect.right - rect.left - 10);
	for(std::vector<ablib::string>::size_type i = 0 ; i < m_rv.size() ; ++i){
		list.AddItem(i,0,m_rv[i].c_str());
		list.SetCheckState(i,TRUE);
	}
	m_rv.clear();
	return TRUE;
}

void DlgList::OnOK(UINT uNotifyCode, int nID, CWindow wndCtl){
	m_rv.clear();
	CListViewCtrl list(this->GetDlgItem(IDC_LIST));
	int count = list.GetItemCount();
	WTL::CString str;
	for(int i = 0  ; i < count ; ++i){
		if(list.GetCheckState(i)){
			list.GetItemText(i,0,str);
			m_rv.push_back(str.AllocSysString());
		}
	}
	this->EndDialog(TRUE);
}

void DlgList::OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl){
	m_rv.clear();
	this->EndDialog(FALSE);
}
