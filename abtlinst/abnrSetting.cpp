#include "stdafx.h"
#include <system/FindFile.h>
#include <vector>
#include <regex>
#include <boost/lexical_cast.hpp>
#include <system/Process.h>
#include <fstream>
#include <functional>
#include <boost/algorithm/string.hpp>
#include "Registory.h"
#include "abnrSetting.h"
#include "Ghostscript.h"
#include "TeXLive.h"
#include "Setting.h"
#include "abnrSetting_funcs.h"
#include "Applications.h"

//extern Ghostscript g_Ghostscript;
extern Applications g_Applications;
extern Setting g_Setting;

void abnrSetting::ReadSetting(){
	if(g_Setting.ReadINI(_T("abnrSetting"),_T("install")) != _T("0"))m_doinst = true;
	else m_doinst = false;
}

void abnrSetting::WriteSetting(){
	g_Setting.WriteINI(_T("abnrSetting"),_T("install"),m_doinst ? _T("1") : _T("0"));
}

bool abnrSetting::Install(HWND hwnd,MSGFUNC &msgfunc){
	if(!m_doinst)return true;
	msgfunc.msg(_T("あべのり設定をします．\n"));
	g_Setting.Log(_T("あべのり設定を実行．"));
	GetTeXLiveDir();
	if(m_tldir == _T("")){
//		msgfunc(_TR("TeX Liveのフォルダ取得に失敗\n"),false);
		g_Setting.Log(_T("あべのり設定中：TeX Live フォルダ取得に失敗しました．"));
		return false;
	}

	std::vector<std::function<bool (HWND,MSGFUNC&)>> funcs;
	funcs.push_back(std::bind(&abnrSetting::TeXworksSetting,this,std::placeholders::_1,std::placeholders::_2));
	funcs.push_back(std::bind(&abnrSetting::writetexmfcnf,this,std::placeholders::_1,std::placeholders::_2));
	funcs.push_back(std::bind(&abnrSetting::kanji_config_updmap,this,std::placeholders::_1,std::placeholders::_2,_T("ipaex")));

	bool rv = true;
	CProgressBarCtrl progress_bar(::GetDlgItem(hwnd,IDC_PROGRESS));
	static const int PROGRESSBAR_MAX = 32768;
	for(std::vector<std::function<bool (HWND,MSGFUNC&)>>::size_type i = 0 ; i < funcs.size() ; ++i){
		rv = (rv && funcs[i](hwnd,msgfunc));
		progress_bar.SetPos((i + 1)*PROGRESSBAR_MAX / funcs.size());
	}

//	msgfunc(rv ? _T("完了") : _T("失敗"),false);
//	msgfunc(_T("しました．\n"),false);
	return rv;
}

bool abnrSetting::TeXworksSetting(HWND hwnd,MSGFUNC &msgfunc){
	return abnrSetting_funcs::TeXworksSetting(m_tldir,msgfunc.msg,msgfunc.detail,std::bind(&Setting::Log,&g_Setting,std::placeholders::_1));
}


bool abnrSetting::writetexmfcnf(HWND hwnd,MSGFUNC &msgfunc){
	return abnrSetting_funcs::writetexmfcnf(m_tldir,msgfunc.msg,msgfunc.detail,std::bind(&Setting::Log,&g_Setting,std::placeholders::_1));
}
/*
bool abnrSetting::PatchTorunscript(HWND hwnd,MSGFUNC &msgfunc){
	return abnrSetting_funcs::PatchTorunscript(m_tldir,msgfunc.msg,msgfunc.detail,std::bind(&Setting::Log,&g_Setting,std::placeholders::_1));
}
*/
bool abnrSetting::kanji_config_updmap(HWND hwnd,MSGFUNC &msgfunc,const ablib::string &font){
	return abnrSetting_funcs::kanji_config_updmap(m_tldir,msgfunc.msg,msgfunc.detail,std::bind(&Setting::Log,&g_Setting,std::placeholders::_1),font);
}

void abnrSetting::GetTeXLiveDir(){
	if(m_tldir != _T(""))return;
	if(m_year == 0)m_year = g_Applications.m_TeXLive.GetYear();
	if(m_year != 0){
		m_tldir = g_Applications.m_TeXLive.GetVar(_T("TEXDIR"));
		if(m_tldir != _T("") && m_tldir[m_tldir.length() - 1] != '\\')m_tldir += _T("\\");
		DEBUGSTRING(_T("abnrSetting::GetTeXLiveDir : TeX Liveディレクトリとして$var{TEXDIR}を利用：m_tldir = %s"),m_tldir.c_str());
		if(m_tlstartmenu != _T("") && m_tldir != _T(""))return;
	}
	abnrSetting_funcs::GetTeXLiveDirFromStartMenu(m_year,m_tldir,m_tlstartmenu);
}

