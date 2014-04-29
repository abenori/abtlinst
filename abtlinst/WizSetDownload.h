#pragma once
#include "atldlgs.h"
#include "resource.h"

class WizSetDownload :
	public CPropertyPageImpl<WizSetDownload>
{
public:
	enum {IDD = IDD_WIZSETDOWNLOAD};

	WizSetDownload() : CPropertyPageImpl<WizSetDownload>(){}
	~WizSetDownload(void){}

	BEGIN_MSG_MAP_EX(WizSetDownload)
		MSG_WM_INITDIALOG(OnInitDialog);
		MSG_WM_COMMAND(OnCommand);
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify);
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow focuswnd,LPARAM lParam);
	BOOL OnCommand(UINT codeNotify, int id, HWND hwndCtl);
	BOOL OnKillActive();

private:
	void SetEnables();
};

