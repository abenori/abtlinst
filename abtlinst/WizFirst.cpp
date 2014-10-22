#include "stdafx.h"
#include "WizFirst.h"
#include "Version.h"

BOOL WizFirst::OnInitDialog(CWindow wndFocus,LPARAM lInitParam){
	const TCHAR *msg = _TR("TeX Live\r\nTeX2img\r\nImageMagick\r\nnkf\r\nのインストールを行います．");

	ablib::string m = ablib::string(msg) + _T("\r\n\r\nabtlinst: ") + Version();
	
	CStatic text(this->GetDlgItem(IDC_FIRSTMSG));
	text.SetWindowText(m.c_str());
	SetWizardButtons(PSWIZB_NEXT);
	return TRUE;
}
