#pragma once

#include <vector>
#include "resource.h"

class DlgList :
	public CDialogImpl<DlgList>
{
public:
	enum {IDD = IDD_DLGLIST};

	DlgList(){}
	~DlgList(){}
	bool ShowList(HWND parent,const ablib::string &title,const ablib::string &msg,std::vector<ablib::string> &list);

	BEGIN_MSG_MAP(DlgList)
		MSG_WM_INITDIALOG(OnInitDialog);
		COMMAND_ID_HANDLER_EX(IDOK,OnOK);
		COMMAND_ID_HANDLER_EX(IDCANCEL,OnCancel);
	END_MSG_MAP()

private:
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnOK(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl);
	std::vector<ablib::string> m_rv;
	ablib::string m_msg,m_title;

};

