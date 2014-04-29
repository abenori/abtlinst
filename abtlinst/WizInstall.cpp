#include "stdafx.h"
#include "WizInstall.h"
#include <functional>
#include <atlstr.h>
#include <atlctrls.h>
#include <future>
#include "Applications.h"
#include "DownloadDlg.h"

extern Applications g_Applications;

BOOL WizInstall::OnSetActive(){
	m_abort = false;
	CRichEditCtrl(this->GetDlgItem(IDC_MSG)).SetWindowText(_T(""));
	SetWizardButtons(PSWIZB_DISABLEDFINISH);
/*
	if(m_thread != NULL)::TerminateThread(m_thread,-1);
	m_thread = reinterpret_cast<HANDLE>(::_beginthread([](void *d){
		static_cast<WizInstall*>(d)->Install();
	},0,this));
	if(m_thread == NULL)return FALSE;
*/
	auto result = std::async(std::launch::async,std::mem_fn(&WizInstall::Install),this);
	return TRUE;
}

BOOL WizInstall::OnInitDialog(CWindow focuswnd,LPARAM lParam){
//	CRichEditCtrl(this->GetDlgItem(IDC_MSG)).SetBackgroundColor(::GetSysColor(COLOR_3DFACE));
	CEdit(this->GetDlgItem(IDC_DETAIL)).SetLimitText(UINT_MAX);
	CEdit(this->GetDlgItem(IDC_MSG)).SetLimitText(UINT_MAX);
	CProgressBarCtrl progress_bar(this->GetDlgItem(IDC_PROGRESS));

	progress_bar.SetRange(0,32768);
	return TRUE;
}

void WizInstall::Install(){
	CProgressBarCtrl progress_bar(this->GetDlgItem(IDC_PROGRESS));
	MSGFUNC msgfunc;
	msgfunc.msg = std::bind(&WizInstall::WriteMsg,this,std::placeholders::_1);
	msgfunc.detail = std::bind(&WizInstall::WriteDetail,this,std::placeholders::_1);
//	if(!m_abort)g_Ghostscript.Install(this->m_hWnd,msgfunc);
/*
	if(!g_TeXLive.GetDoInst() || !g_TeXLive.GetDownloadState()){
		if(g_abnrSetting.GetDoInst())progress_bar.SetPos(16384);
		else progress_bar.SetPos(16384);
	}
*/

	if(!m_abort)g_Applications.m_TeXLive.Install(this->m_hWnd,msgfunc);
	if(!m_abort)g_Applications.m_TeX2img.Install(this->m_hWnd,msgfunc);
	if(!m_abort)g_Applications.m_ImageMagick.Install(this->m_hWnd,msgfunc);
	if(!m_abort)g_Applications.m_nkf.Install(this->m_hWnd,msgfunc);
	if(!m_abort)g_Applications.m_abnrSetting.Install(this->m_hWnd,msgfunc);
	if(!m_abort)progress_bar.SetPos(32768);

	if(m_abort)WriteMsg(_T("キャンセルされました．\n"));
	else WriteMsg(_T("インストールが完了しました．\n"));
	WriteDetail(_T("インストールの終了\n"));
	SetWizardButtons(PSWIZB_FINISH);
}


BOOL WizInstall::OnCommand(UINT codeNotify, int id, HWND hwndCtl){
	switch(id){
	case IDCANCEL:
	{
		LOCK l(this);
		m_abort = true;
		g_Applications.Abort();
		return TRUE;
	}
	default:
		return FALSE;
	}
}
