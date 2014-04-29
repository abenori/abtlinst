#include "stdafx.h"
#include "WizSetDownload.h"
#include <boost/lexical_cast.hpp>
#include "Setting.h"
#include "TeXLive.h"
#include "abnrSetting.h"
#include "Network.h"
#include "Applications.h"

#ifndef INSTALL_FROM_LOCAL
extern Setting g_Setting;
extern Applications g_Applications;

extern Network g_Network;

BOOL WizSetDownload::OnInitDialog(CWindow focuswnd,LPARAM lParam){
	CButton(this->GetDlgItem(IDC_CHKGS)).SetCheck(g_Applications.m_Ghostscript.GetDoInst() ? TRUE : FALSE);
	CButton(this->GetDlgItem(IDC_CHKTL)).SetCheck(g_Applications.m_TeXLive.GetDoInst() ? TRUE : FALSE);
	CButton(this->GetDlgItem(IDC_PATH)).SetCheck(g_Setting.SetPath() ? TRUE : FALSE);
	CButton(this->GetDlgItem(IDC_CHKABNRSET)).SetCheck(g_Applications.m_abnrSetting.GetDoInst() ? TRUE : FALSE);

	CEdit(this->GetDlgItem(IDC_INSTALLDIR)).SetWindowText(g_Setting.GetInstallDir().c_str());

	ablib::string text;
	CComboBox tlcomb(this->GetDlgItem(IDC_TLURLLIST));
	for(int i = 0 ; ; ++i){
		text = g_Setting.ReadSysINI(_T("TeXLive"),_T("URLlist") + boost::lexical_cast<ablib::string>(i));
		if(text == _T(""))break;
		tlcomb.AddString(text.c_str());
	}
	tlcomb.SetCurSel(0);

	CComboBox gscomb(this->GetDlgItem(IDC_GSURLLIST));
	for(int i = 0 ; ; ++i){
		text = g_Setting.ReadSysINI(_T("GS"),_T("URLlist") + boost::lexical_cast<ablib::string>(i));
		if(text == _T(""))break;
		gscomb.AddString(text.c_str());
	}
	gscomb.SetCurSel(0);

	SetEnables();

	SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);
	return TRUE;
}

BOOL WizSetDownload::OnCommand(UINT codeNotify, int id, HWND hwndCtl){
	switch(id){
	case IDC_CHKGS:
	case IDC_CHKTL:
		SetEnables();
		return TRUE;
	case IDC_SANSYO_INSTALL:
	{
		CFolderDialog dl(this->m_hWnd,_T("インストールディレクトリを選択してください．"));
		if(dl.DoModal() == IDOK){
			CEdit(this->GetDlgItem(IDC_INSTALLDIR)).SetWindowText(dl.GetFolderPath());
		}
		return TRUE;
	}
	default:
		return FALSE;
	}
}


void WizSetDownload::SetEnables(){
	if(CButton(this->GetDlgItem(IDC_CHKGS)).GetCheck() == BST_CHECKED){
		CComboBox(this->GetDlgItem(IDC_GSURLLIST)).EnableWindow(TRUE);
	}else{
		CComboBox(this->GetDlgItem(IDC_GSURLLIST)).EnableWindow(FALSE);
	}

	if(CButton(this->GetDlgItem(IDC_CHKTL)).GetCheck()){
		CComboBox(this->GetDlgItem(IDC_TLURLLIST)).EnableWindow(TRUE);
		CEdit(this->GetDlgItem(IDC_REPOSITORY)).SetReadOnly(FALSE);
	}else{
		CComboBox(this->GetDlgItem(IDC_TLURLLIST)).EnableWindow(FALSE);
		CEdit(this->GetDlgItem(IDC_REPOSITORY)).SetReadOnly(TRUE);
	}
}

BOOL WizSetDownload::OnKillActive(){
	ablib::string url;
	TCHAR buf[4096];
	WTL::CString str;
	CComboBox combogs(this->GetDlgItem(IDC_GSURLLIST));
	combogs.GetLBText(combogs.GetCurSel(),str);
	url = static_cast<LPCTSTR>(str);

	g_Ghostscript.Set(CButton(this->GetDlgItem(IDC_CHKGS)).GetCheck() ? true : false,url);

	CComboBox combotl(this->GetDlgItem(IDC_TLURLLIST));
	combotl.GetLBText(combotl.GetCurSel(),str);
	url = static_cast<LPCTSTR>(str);

	ablib::string repository;
	CButton(this->GetDlgItem(IDC_REPOSITORY)).GetWindowText(buf,4095);
	repository = buf;
	g_TeXLive.Set(CButton(this->GetDlgItem(IDC_CHKTL)).GetCheck() ? true : false,url,repository);
	g_Setting.SetPath(CButton(this->GetDlgItem(IDC_PATH)).GetCheck() ? true : false);

	CEdit(this->GetDlgItem(IDC_INSTALLDIR)).GetWindowText(buf,4095);
	g_Setting.SetInstallDir(buf);

	CEdit(this->GetDlgItem(IDC_PROXY)).GetWindowText(buf,4095);
	ablib::string proxy = buf;
	CEdit(this->GetDlgItem(IDC_PROXYPORT)).GetWindowText(buf,4095);
	g_Network.SetProxy(proxy,boost::lexical_cast<int>(buf));

	g_Applications.m_abnrSetting.Set(CButton(this->GetDlgItem(IDC_CHKABNRSET)).GetCheck() ? true : false);

	return TRUE;
}
#endif // INSTALL_FROM_LOCAL
