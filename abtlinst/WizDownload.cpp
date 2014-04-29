#include "stdafx.h"
#include "WizDownload.h"
#include <functional>
#include <atlstr.h>
#include <future>
#include "Applications.h"

extern Applications g_Applications;

BOOL WizDownload::OnSetActive(){
	m_abort = false;
	SetWizardButtons(0);
	auto result = std::async(std::launch::async,std::mem_fn(&WizDownload::Download),this);
/*
	m_thread = reinterpret_cast<HANDLE>(::_beginthread([](void *d){
		static_cast<WizDownload*>(d)->Download();
	},0,this));
*/
	return TRUE;
}

BOOL WizDownload::OnWizardNext(){
#ifndef INSTALL_FROM_LOCAL
	if(!g_TeXLive.GetDoInst())return IDD_WIZINSTALL;
#endif
	return FALSE;
}

void WizDownload::Download(){
	CEdit(this->GetDlgItem(IDC_MSG)).SetWindowText(_T(""));
	WriteMsg(_T("インストールの準備を行います．\nこれにはしばらく時間がかかることがあります．\n"));
	MSGFUNC msgfunc;
	msgfunc.msg = std::bind(&WizDownload::WriteMsg,this,std::placeholders::_1);
	msgfunc.detail = std::bind(&WizDownload::WriteDetail,this,std::placeholders::_1);
	if(!m_abort)g_Applications.m_TeXLive.Download(this->m_hWnd,msgfunc);
//	if(!m_abort)g_Ghostscript.Download(this->m_hWnd,msgfunc);

	if(m_abort){
		SetWizardButtons(PSWIZB_BACK);
		WriteMsg(_T("キャンセルされました．\n"));
	}else{
		SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
		WriteMsg(_T("インストールの準備が完了しました．\n"));
	}
}

BOOL WizDownload::OnCommand(UINT codeNotify, int id, HWND hwndCtl){
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

