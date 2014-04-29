#pragma once
#include "resource.h"

class WizFirst :
	public CPropertyPageImpl<WizFirst>
{
public:
	enum {IDD = IDD_WIZFIRST};
	WizFirst(ATL::_U_STRINGorID title =reinterpret_cast<LPCTSTR>(NULL)) : CPropertyPageImpl<WizFirst>(title){}
	~WizFirst(void){}

	BEGIN_MSG_MAP(WizFirst)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus,LPARAM lInitParam);
};



