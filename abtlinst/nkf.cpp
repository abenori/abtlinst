#include "stdafx.h"
#include "nkf.h"
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "Setting.h"
#include "Applications.h"

extern Setting g_Setting;
extern Applications g_Applications;


void nkf::ReadSetting(){
	m_filedir = g_Setting.ToAbsolutePath(g_Setting.ReadSysINI(_T("nkf"),_T("FileDir")));
	if(m_filedir == _T(""))m_filedir = g_Setting.ToAbsolutePath(g_Setting.ReadSysINI(_T("abtlinst"),_T("DownloadDir")));
	if(m_filedir != _T("") && m_filedir[m_filedir.length() - 1] != '\\')m_filedir += _T("\\");

	m_filereg = g_Setting.ReadSysINI(_T("nkf"),_T("Reg"));

	if(g_Setting.ReadINI(_T("nkf"),_T("install")) != _T("0"))m_doinst = true;
	else m_doinst = false;
#ifndef INSTALL_FROM_LOCAL
	m_url = g_Setting.ReadINI(_T("nkf"),_T("URL"));
#endif
}

void nkf::WriteSetting(){
	g_Setting.WriteINI(_T("nkf"),_T("install"),m_doinst ? _T("1") : _T("0"));
}

bool nkf::Download(HWND hwnd,MSGFUNC &msgfunc){
	msgfunc.msg(_T("nkf のファイルを探索\n"));
	std::wregex reg;
	try{reg.assign(m_filereg,std::regex_constants::icase);}
	catch(...){
		msgfunc.detail(_T("nkf の発見に失敗\n"));
		g_Setting.Log(_T("nkf を発見する正規表現が不正です．"));
		return false;
	}
	boost::filesystem::directory_iterator end;
	try{
		for(boost::filesystem::directory_iterator ite(m_filedir) ; ite != end ; ++ite){
			boost::filesystem::path p(*ite);
			if(std::regex_match(p.filename().wstring(),reg)){
				m_file = p.filename().wstring();
				break;
			}
		}
	}catch(...){
		g_Setting.Log(_T("nkf を保存しているフォルダが見つかりませんでした．"));
		return false;
	}
	if(m_file == _T("")){
		msgfunc.detail(_T("nkf の発見に失敗\n"));
		g_Setting.Log(_T("nkf が見つかりませんでした．"));
		return false;
	}else return true;
}

bool nkf::Install(HWND hwnd,MSGFUNC &msgfunc){
	if(!m_doinst){
		g_Setting.Log(_T("nkf をインストールしません．"));
		return true;
	}else g_Setting.Log(_T("nkf のインストールを実行．"));
	ablib::string dir = g_Applications.m_TeXLive.GetVar(_T("TEXDIR"));
	if(dir == _T("")){
		g_Setting.Log(_T("nkf のインストール中：TEXDIR の取得に失敗しました．"));
		return false;
	}
	if(dir != _T("") && dir[dir.length() - 1] != '\\')dir += _T("\\");
	dir += _T("bin\\win32\\");
	::CreateDirectoryReflex(dir.c_str());
	msgfunc.msg(_T("nkf のインストール\n"));
	if(::CopyFile((m_filedir + m_file).c_str(),(dir + _T("nkf.exe")).c_str(),TRUE) == FALSE){
		g_Setting.Log(_T("nkf のコピーに失敗．GetLastError = ") + boost::lexical_cast<ablib::string>(::GetLastError()));
		msgfunc.detail(_T("nkf のインストールに失敗．\n"));
		return false;
	}else{
		msgfunc.detail(_T("nkf をインストールしました．\n"));
		return true;
	}
}

