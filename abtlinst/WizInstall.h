#pragma once
#include <atldlgs.h>
#include <ablib/Lock.h>
#include "resource.h"
#include "func.h"

class WizInstall :
	public CPropertyPageImpl<WizInstall>,ablib::Lock
{
public:
	enum {IDD = IDD_WIZINSTALL};
	WizInstall() : m_thread(NULL),m_abort(false){}
	~WizInstall(){}

	BEGIN_MSG_MAP_EX(WizInstall)
		MSG_WM_INITDIALOG(OnInitDialog);
		MSG_WM_COMMAND(OnCommand);
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify);
	END_MSG_MAP()

	BOOL OnSetActive();
	BOOL OnInitDialog(CWindow focuswnd,LPARAM lParam);
	BOOL OnCommand(UINT codeNotify, int id, HWND hwndCtl);
private:
	void WriteMsg(const ablib::string &msg){if(this->IsWindow())::WriteToEditCtrl(this->GetDlgItem(IDC_MSG),msg);}
	void WriteDetail(const ablib::string &msg){if(this->IsWindow())::WriteToEditCtrl(this->GetDlgItem(IDC_DETAIL),msg);}
	
	void Install();
	HANDLE m_thread;
	bool m_abort;
};

