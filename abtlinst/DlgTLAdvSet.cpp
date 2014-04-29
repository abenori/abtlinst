#include "stdafx.h"
#include <vector>
#include <atlstr.h>
#include "DlgTLAdvSet.h"
#include "Applications.h"

extern Applications g_Applications;


LRESULT DlgTLAdvSet::OnInitDialog(HWND hWnd, LPARAM lParam){
	std::vector<ablib::string> schemes = g_Applications.m_TeXLive.GetSchemes();
	int index = -1;
	int scheme_full_index = 0;
	for(std::vector<ablib::string>::size_type i = 0; i < schemes.size() ; ++i){
		CComboBox(this->GetDlgItem(IDC_SCHEMES)).AddString(schemes[i].c_str());
		if(schemes[i] == g_Applications.m_TeXLive.GetVar(_T("selected_scheme")))index = i;
		else if(schemes[i] == _T("scheme-full"))scheme_full_index = i;
	}
	if(index == -1)index = scheme_full_index;
	CComboBox(this->GetDlgItem(IDC_SCHEMES)).SetCurSel(index);

	CButton(this->GetDlgItem(IDC_CHKDIRINDEP)).SetCheck(g_Applications.m_TeXLive.GetVar(_T("abnr_dir_indep")) == _T("1") ? TRUE : FALSE);
	CEdit(this->GetDlgItem(IDC_TEXMFLOCAL)).SetWindowText(g_Applications.m_TeXLive.GetVar(_T("TEXMFLOCAL")).c_str());
	CEdit(this->GetDlgItem(IDC_TEXMFSYSVAR)).SetWindowText(g_Applications.m_TeXLive.GetVar(_T("TEXMFSYSVAR")).c_str());
	CEdit(this->GetDlgItem(IDC_TEXMFSYSCONFIG)).SetWindowText(g_Applications.m_TeXLive.GetVar(_T("TEXMFSYSCONFIG")).c_str());
	CEdit(this->GetDlgItem(IDC_TEXMFHOME)).SetWindowText(g_Applications.m_TeXLive.GetVar(_T("TEXMFHOME")).c_str());

	SetButtons();
#ifndef INSTALL_FROM_LOCAL
	CButton(this->GetDlgItem(IDC_UPDATEFROMINTERNET)).ShowWindow(SW_HIDE);
#endif

	SetEnabled();
	return TRUE;
}

void DlgTLAdvSet::SetButtons(){
#define SETBUTTON(id,varname) CButton(this->GetDlgItem(id)).SetCheck(g_Applications.m_TeXLive.GetVar(varname) == _T("0") ? FALSE : TRUE);
	SETBUTTON(IDC_WRITE18,_T("option_write18_restricted"));
	SETBUTTON(IDC_FORMATFILE,_T("option_fmt"));
	SETBUTTON(IDC_DOCTREE,_T("option_doc"));
	SETBUTTON(IDC_SOURCETREE,_T("option_src"));
	SETBUTTON(IDC_TEXWORKS,_T("collection-texworks"));
#ifdef INSTALL_FROM_LOCAL
//	SETBUTTON(IDC_UPDATEFROMINTERNET,_T("option_adjustrepo"));
#endif
#undef SETBUTTON
}

void DlgTLAdvSet::OnOK(UINT uNotifyCode, int nID, HWND hWndCtl){
	if(CButton(this->GetDlgItem(IDC_CHKDIRINDEP)).GetCheck()){
		g_Applications.m_TeXLive.SetVar(_T("abnr_dir_indep"),_T("1"));
		TCHAR buf[4096];
		CEdit(this->GetDlgItem(IDC_TEXMFLOCAL)).GetWindowText(buf,4095);
		g_Applications.m_TeXLive.SetVar(_T("TEXMFLOCAL"),buf);
		CEdit(this->GetDlgItem(IDC_TEXMFSYSVAR)).GetWindowText(buf,4095);
		g_Applications.m_TeXLive.SetVar(_T("TEXMFSYSVAR"),buf);
		CEdit(this->GetDlgItem(IDC_TEXMFSYSCONFIG)).GetWindowText(buf,4095);
		g_Applications.m_TeXLive.SetVar(_T("TEXMFSYSCONFIG"),buf);
		CEdit(this->GetDlgItem(IDC_TEXMFHOME)).GetWindowText(buf,4095);
		g_Applications.m_TeXLive.SetVar(_T("TEXMFHOME"),buf);
	}else{
		g_Applications.m_TeXLive.SetVar(_T("abnr_dir_indep"),_T("0"));
	}
	ATL::CString str;
	CComboBox(this->GetDlgItem(IDC_SCHEMES)).GetWindowText(str);
	g_Applications.m_TeXLive.SetVar(_T("selected_scheme"),ablib::string(str));

	g_Applications.m_TeXLive.UpdateTeXDirs();

#define SETVAR(id,varname) g_Applications.m_TeXLive.SetVar(varname,CButton(this->GetDlgItem(id)).GetCheck() ? _T("1") : _T("0"));
	SETVAR(IDC_WRITE18,_T("option_write18_restricted"));
	SETVAR(IDC_FORMATFILE,_T("option_fmt"));
	SETVAR(IDC_DOCTREE,_T("option_doc"));
	SETVAR(IDC_SOURCETREE,_T("option_src"));
	SETVAR(IDC_TEXWORKS,_T("collection-texworks"));
#ifdef INSTALL_FROM_LOCAL
//	SETVAR(IDC_UPDATEFROMINTERNET,_T("option_adjustrepo"));
#endif
#undef SETVAR
	EndDialog(TRUE);
	return;
}
void DlgTLAdvSet::OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl){
	EndDialog(FALSE);
	return;
}

void DlgTLAdvSet::SetEnabled(){
	BOOL checkstate = CButton(this->GetDlgItem(IDC_CHKDIRINDEP)).GetCheck();
	CEdit(this->GetDlgItem(IDC_TEXMFLOCAL)).SetReadOnly(!checkstate);
	CEdit(this->GetDlgItem(IDC_TEXMFSYSVAR)).SetReadOnly(!checkstate);
	CEdit(this->GetDlgItem(IDC_TEXMFSYSCONFIG)).SetReadOnly(!checkstate);
	CEdit(this->GetDlgItem(IDC_TEXMFHOME)).SetReadOnly(!checkstate);
	return;
}

void DlgTLAdvSet::OnSansyo(int editid){
	CFolderDialog dl(this->m_hWnd,_T("ディレクトリを選択してください．"));
	if(dl.DoModal() == IDOK){
		CEdit(this->GetDlgItem(editid)).SetWindowText(dl.GetFolderPath());
	}
}

