#include "stdafx.h"
#include "ImageMagick.h"
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "Setting.h"
#include "Applications.h"

extern Setting g_Setting;
extern Applications g_Applications;


void ImageMagick::ReadSetting(){
	m_filedir = g_Setting.ToAbsolutePath(g_Setting.ReadSysINI(_T("ImageMagick"),_T("FileDir")));
	if(m_filedir == _T(""))m_filedir = g_Setting.ToAbsolutePath(g_Setting.ReadSysINI(_T("abtlinst"),_T("DownloadDir")));
	if(m_filedir != _T("") && m_filedir[m_filedir.length() - 1] != '\\')m_filedir += _T("\\");

	m_filereg = g_Setting.ReadSysINI(_T("ImageMagick"),_T("Reg"));

	if(g_Setting.ReadINI(_T("ImageMagick"),_T("install")) != _T("0"))m_doinst = true;
	else m_doinst = false;
#ifndef INSTALL_FROM_LOCAL
	m_url = g_Setting.ReadINI(_T("ImageMagick"),_T("URL"));
#endif
}

void ImageMagick::WriteSetting(){
	g_Setting.WriteINI(_T("ImageMagick"),_T("install"),m_doinst ? _T("1") : _T("0"));
}

bool ImageMagick::Download(HWND hwnd,MSGFUNC &msgfunc){
	std::wregex reg;
	msgfunc.detail(_T("ImageMagick のファイルを探索\n"));
	try{reg.assign(m_filereg,std::regex_constants::icase);}
	catch(...){
		msgfunc.detail(_T("ImageMagick の発見に失敗\n"));
		g_Setting.Log(_T("ImageMagick を発見する正規表現が不正です．"));
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
	}
	catch(...){
		g_Setting.Log(_T("ImageMagick を保存しているフォルダが見つかりませんでした．"));
		return false;
	}
	if(m_file == _T("")){
		msgfunc.detail(_T("ImageMagick の発見に失敗\n"));
		g_Setting.Log(_T("ImageMagick が見つかりませんでした．"));
		return false;
	}else return true;
}

bool ImageMagick::Install(HWND hwnd,MSGFUNC &msgfunc){
	if(!m_doinst){
		g_Setting.Log(_T("ImageMagick のインストールはしません．"));
		return true;
	}else g_Setting.Log(_T("ImageMagick のインストールを実行．"));
	ablib::system::Process process;
	ablib::system::Process::ProcessStartInfo si;
	msgfunc.msg(_T("ImageMagick のインストール\n"));
	msgfunc.detail(_T("ImageMagick のインストール\n"));
	si.Arguments = _T("\"") + m_filedir + m_file + _T("\"");
	si.RedirectStandardError = si.RedirectStandardInput = si.RedirectStandardOutput = false;
	si.ShowState = si.show;
	si.WorkingDirectory = m_filedir;
	process.StartInfo(si);
	if(!process.Start()){
		g_Setting.Log(_T("ImageMagick の起動に失敗：GetLastErro = ") + boost::lexical_cast<ablib::string>(ablib::system::GetLastError()));
		return false;
	}else{
		process.WaitForExit();
		return true;
	}
}

