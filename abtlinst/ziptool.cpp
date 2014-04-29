#include "stdafx.h"
#include "ziptool.h"
#include <process.h>
#include <window/dialog.h>
#include <ablib/IsKanji.h>
#include <zlib/zip.h>
#include <ablib/Pathname.h>
#include "resource.h"


bool ziptool::CheckCancel(){
//	return GetWindow()->SendMsg(WM_ISCANCEL,0,0) != FALSE;
	return false;
}


bool ziptool::DeCompresDlg(HWND parent,const TCHAR *fname,const TCHAR *dir){
	m_file = fname;
	m_dir = dir;
	m_todo = decompress;
	return DoModal(parent) ? true : false;
}


BOOL ziptool::OnInitDialog(CWindow focuswnd,LPARAM lParam){
	ablib::Pathname path(m_file);
	this->SetWindowText(ablib::string::sprintf(_TR("%s ‚ð‰ð“€’†cc"),(path.basename() + path.filetype()).c_str()).c_str());
	CStatic(this->GetDlgItem(IDC_DIR)).SetWindowText(m_dir.c_str());
	m_Thread = reinterpret_cast<HANDLE>(::_beginthread([](void *d){
		bool rv = false;
		ziptool *me = static_cast<ziptool *>(d);
		if(me->m_todo == decompress)rv = me->StartDeCompress();
		me->EndDialog(rv ? TRUE : FALSE);
	},0,this));
	if(m_Thread == NULL)this->EndDialog(FALSE);
	return true;
}
/*
INT_PTR ziptool::OnOtherMessage(ablib::window::Dialog *wnd,UINT msg,WPARAM wParam,LPARAM lParam){
	switch(msg){
	case WM_MESSAGE:
		ablib::window::Edit(wnd,IDC_FILE).SetText(reinterpret_cast<TCHAR *>(lParam));
		return TRUE;
	default:
		return FALSE;
	}
}
*/

BOOL ziptool::OnCommand(UINT codeNotify, int id, HWND hwndCtl){
	switch(id){
	case IDCANCEL:
		m_zip.Stop();
		return true;
	default:
		return false;
	}
}

bool ziptool::StartDeCompress(){
	return m_zip.DeCompress(m_file.c_str(),m_dir.c_str());
}


bool ziptool::WriteMsg(const TCHAR *msg){
	CEdit(this->GetDlgItem(IDC_FILE)).SetWindowText(msg);
//	this->SendMessage(WM_MESSAGE,0,reinterpret_cast<LPARAM>(const_cast<TCHAR*>(msg)));
	return true;
}


