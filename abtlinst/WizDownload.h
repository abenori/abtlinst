#pragma once

#include <atldlgs.h>
#include "resource.h"
#include <ablib/Lock.h>
#include "Func.h"

class WizDownload :
	public CPropertyPageImpl<WizDownload>, ablib::Lock
{
public:
	enum {IDD = IDD_WIZDOWNLOAD};
	WizDownload(){}
	~WizDownload(){}

	BEGIN_MSG_MAP_EX(WizDownload)
		MSG_WM_COMMAND(OnCommand);
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify);
	END_MSG_MAP()

	BOOL OnSetActive();
	BOOL OnCommand(UINT codeNotify, int id, HWND hwndCtl);
	BOOL OnWizardNext();

private:
	HANDLE m_thread,m_event;
	void Download();

	void WriteMsg(const ablib::string &msg){::WriteToEditCtrl(this->GetDlgItem(IDC_MSG),msg);}
	void WriteDetail(const ablib::string &msg){::WriteToEditCtrl(this->GetDlgItem(IDC_DETAIL),msg);}
	bool m_abort;
	
};

