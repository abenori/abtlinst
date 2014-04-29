#pragma once
#include <atlwin.h>
#include <thread>
#include <mutex>
#include "resource.h"
#include "func.h"

#ifdef INSTALL_FROM_LOCAL

class DlgInitInstaller :
	public CDialogImpl<DlgInitInstaller>
{
public:
	enum {IDD = IDD_INITINSTALLER};

	DlgInitInstaller(void){}
	~DlgInitInstaller(void){}
	BEGIN_MSG_MAP(DlgInitInstaller)
		 MSG_WM_INITDIALOG(OnInitDialog)
		 COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()
	
private:
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl);

	bool InitInstaller();
//	std::thread m_thread;
	bool m_abort;
	void WriteMsg(const ablib::string &m){::WriteToEditCtrl(this->GetDlgItem(IDC_MSG),m);}
	void WriteDetail(const ablib::string &m){::WriteToEditCtrl(this->GetDlgItem(IDC_DETAIL),m);}
	std::mutex m_mutex;


};

#endif // INSTALL_FROM_LOCAL