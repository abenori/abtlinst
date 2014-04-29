#include "stdafx.h"
#include <atlstr.h>
#include <future>
#include <vector>
#include <functional>
#include "DlgInitInstaller.h"
#include "Applications.h"
#include "Setting.h"

#ifdef INSTALL_FROM_LOCAL

extern Applications g_Applications;
extern Setting g_Seting;


BOOL DlgInitInstaller::OnInitDialog(CWindow wndFocus,LPARAM lInitParam){
	m_abort = false;
	auto result = std::async(std::launch::async,std::mem_fn(&DlgInitInstaller::InitInstaller),this);
	return TRUE;
}

bool DlgInitInstaller::InitInstaller(){
	MSGFUNC msgfunc;
	msgfunc.msg = std::bind(&DlgInitInstaller::WriteMsg,this,std::placeholders::_1);
	msgfunc.detail = std::bind(&DlgInitInstaller::WriteDetail,this,std::placeholders::_1);
	std::unique_lock<std::mutex> l(m_mutex);
	std::vector<std::function<bool ()>> funcs;
	funcs.push_back(std::bind(&TeXLive::Download,&g_Applications.m_TeXLive,this->m_hWnd,msgfunc));
	funcs.push_back(std::bind(&TeX2img::Download,&g_Applications.m_TeX2img,this->m_hWnd,msgfunc));
	funcs.push_back(std::bind(&ImageMagick::Download,&g_Applications.m_ImageMagick,this->m_hWnd,msgfunc));
	funcs.push_back(std::bind(&nkf::Download,&g_Applications.m_nkf,this->m_hWnd,msgfunc));
	for(auto func : funcs){
		if(!m_abort){
			l.unlock();
			if(!func()){
				this->EndDialog(FALSE);
				return false;
			}
			l.lock();
		}
	}

	if(m_abort){
		l.unlock();
		WriteMsg(_T("キャンセルされました．\n"));
		this->EndDialog(FALSE);
		return false;
	}else{
		l.unlock();
		WriteMsg(_T("インストールの準備が完了しました．\n"));
		this->EndDialog(TRUE);
		return true;
	}
}


void DlgInitInstaller::OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl){
	{std::unique_lock<std::mutex> l(m_mutex);m_abort = true;}
	g_Applications.Abort();
}


#endif // INSTALL_FROM_LOCAL

