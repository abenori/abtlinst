#pragma once
#include <atldlgs.h>
#include "resource.h"

class WizTLSet :
	public CPropertyPageImpl<WizTLSet>
{
public:
#ifdef INSTALL_FROM_LOCAL
	enum {IDD = IDD_WIZAPPSSET};
#else
	enum {IDD = IDD_TLSET};
#endif
	WizTLSet(){}
	~WizTLSet(){}

	BEGIN_MSG_MAP_EX(WizTLSet)
		MSG_WM_COMMAND(OnCommand);
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify);
	END_MSG_MAP()

	BOOL OnSetActive();
	BOOL OnKillActive();
	BOOL OnWizardBack();
	BOOL OnCommand(UINT codeNotify, int id, HWND hwndCtl);

private:
	void SetEnabled();
	void SetTeXLiveVars();
};

