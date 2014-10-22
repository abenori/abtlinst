#include "stdafx.h"
#include "Setting.h"
#include <vector>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include "Func.h"
#include "Version.h"

/** 初期値
* InstallDir: <sysdir>:\texlive
* DownloadDir: <InstallDir>\download
*/

Setting::Setting(){
	DWORD size = ::GetTempPath(0,NULL);
	TCHAR *buf = new TCHAR[size + 1];
	::GetTempPath(size,buf);
	m_tmpdir = AddLastYen(buf);
	delete[] buf;
	// 長いパスにする（気分の問題）
	size = ::GetLongPathName(m_tmpdir.c_str(),NULL,0);
	buf = new TCHAR[size + 1];
	::GetLongPathName(m_tmpdir.c_str(),buf,size);
	m_tmpdir = AddLastYen(buf);
	delete[] buf;

	TCHAR file[4096];
	::GetModuleFileName(NULL,file,4096);
	m_ini = file;
	m_ini.erase(m_ini.rfind(_T(".")));
	m_sysini = m_ini + _T("_sys.ini");

	m_ini += _T(".ini");

}

void Setting::ReadSetting(){
	if(ReadINI(_T("abtlinst"),_T("setpath")) != _T("0"))m_setpath = true;
	else m_setpath = true;

	m_installdir = AddLastYen(ReadINI(_T("abtlinst"),_T("InstallDir")));
	if(m_installdir == _T("")){
		TCHAR buf2[128];::GetSystemDirectory(buf2,127);
		m_installdir = ablib::string(buf2[0]) + _T(":\\texlive\\");
	}
	m_installdir = ToAbsolutePath(m_installdir);
	
	m_downloaddir = AddLastYen(ReadSysINI(_T("abtlinst"),_T("DownloadDir")));
	if(m_downloaddir == _T("")){
		m_downloaddir = m_installdir + _T("download\\");
		m_downloaddir_from_ini = false;
	}else m_downloaddir_from_ini = true;
	m_downloaddir = ToAbsolutePath(m_downloaddir);

	int logtype = 0;
	try{logtype = boost::lexical_cast<int>(ReadSysINI(_T("abtlinst"),_T("LogFile")));}catch(...){}
	InitLogFileName(logtype);
}

void Setting::WriteSetting(){
	WriteINI(_T("abtlinst"),_T("setpath"),m_setpath ? _T("1") : _T("0"));
	WriteINI(_T("abtlinst"),_T("InstallDir"),m_installdir);
}

void Setting::GenerateSysINI(){
	if(::GetFileAttributes(m_sysini.c_str()) != -1)return;
	HANDLE fh = ::CreateFile(m_sysini.c_str(),GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh != INVALID_HANDLE_VALUE){
#define RETTEXT(s) _T(s) _T("\r\n")
		const TCHAR sysini_def[] = 
			RETTEXT("[TeXLive]")
			RETTEXT("URLlist0=http://ftp.kddilabs.jp/CTAN/systems/texlive/tlnet/")
			RETTEXT("URLlist1=http://ossm.utm.my/tex-archive/systems/texlive/tlnet/")
			RETTEXT("URLlist2=http://ftp.jaist.ac.jp/pub/CTAN/systems/texlive/tlnet/")
			RETTEXT("URLlist3=ftp://ftp.riken.go.jp/pub/tex-archive/systems/texlive/tlnet/")
			RETTEXT("[GS]")
			RETTEXT("URLlist0=http://www.ring.gr.jp/pub/text/TeX/ptex-win32/gs/")
			RETTEXT("URLlist1=http://www.dnsbalance.ring.gr.jp/pub/text/TeX/ptex-win32/gs/")
			RETTEXT("URLlist2=http://core.ring.gr.jp/pub/text/TeX/ptex-win32/gs/")
			RETTEXT("URLlist3=ftp://ftp.ring.gr.jp/pub/text/TeX/ptex-win32/gs/")
			RETTEXT("URLlist4=ftp://ftp.dnsbalance.ring.gr.jp/pub/text/TeX/ptex-win32/gs/")
			RETTEXT("URLlist5=ftp://core.ring.gr.jp/pub/text/TeX/ptex-win32/gs/")
			RETTEXT("URLlist6=ftp://akagi.ms.u-tokyo.ac.jp/pub/TeX/win32-gs/")
			RETTEXT("Reg=gs[0-9]*w32full(-gpl)?\\.exe")
			;
#undef RETTEXT
		DWORD size;
		::WriteFile(fh,sysini_def,sizeof(TCHAR)*::lstrlen(sysini_def),&size,NULL);
		::CloseHandle(fh);
	}
}

ablib::string Setting::Read(const ablib::string &file,const ablib::string &sec,const ablib::string &key) const{
	TCHAR buf[4096];
	::GetPrivateProfileString(sec.c_str(),key.c_str(),_T(""),buf,4095,file.c_str());
	return buf;
}

 void Setting::Write(const ablib::string &file,const ablib::string &sec,const ablib::string &key,const ablib::string &val){
	 ::WritePrivateProfileString(sec.c_str(),key.c_str(),val.c_str(),file.c_str());
}

std::vector<ablib::string> Setting::INIKeys(const ablib::string &sec){
	return Keys(m_ini,sec);
}

std::vector<ablib::string> Setting::Keys(const ablib::string &ini,const ablib::string &sec){
	TCHAR keys[65537];
	::GetPrivateProfileString(sec.c_str(),NULL,_T(""),keys,65535,ini.c_str());
	std::vector<ablib::string> rv;
	const TCHAR *p = keys;
	while(*p){
		rv.push_back(p);
		p += ::lstrlen(p) + 1;
	}
	return rv;
}

void Setting::InitLogFileName(int logtype){
	TCHAR file[4096];
	::GetModuleFileName(NULL,file,4096);
	boost::filesystem::path me(file);
	switch(logtype){
	case 1:
	// インストールディレクトリ
		m_logfile = m_installdir + me.stem().wstring() + _T("_log.txt");
		break;
	case 2:
	// 保存フォルダ
		m_logfile = m_downloaddir + me.stem().wstring() + _T("_log.txt");
		break;
	case 3:
	// デスクトップ
	{
		LPITEMIDLIST id;
		LPMALLOC mal;
		if(!FAILED(::SHGetMalloc(&mal)) && !FAILED(::SHGetSpecialFolderLocation(NULL,CSIDL_DESKTOP,&id))){
			::SHGetPathFromIDList(id,file);
			mal->Free(id);
			m_logfile = ablib::string(file) + _T("\\") + me.stem().wstring() + _T("_log.txt");
		}else{
			m_logfile = me.parent_path().wstring() + _T("\\") + me.stem().wstring() + _T("_log.txt");
		}
		break;
	}
	case 4:
	// システムドライブ直下
		::GetSystemDirectory(file,4095);file[1] = '\0';
		m_logfile = ablib::string(file) + _T("\\") + me.stem().wstring() + _T("_log.txt");
		break;
	default:
	// .exeと同じ
		m_logfile = me.parent_path().wstring() + _T("\\") + me.stem().wstring() + _T("_log.txt");
		break;
	}

	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	Log(_T(""));
	Log(_T("------------------------------------------"));
	Log(_T("abtlinst: Version ") + Version() + _T(" build : ") + VersionDate() + _T(" ") + VersionTime());
	Log(_T("起動時刻：") + boost::posix_time::to_simple_wstring(now));
//	Log(boost::posix_time::to_simple_wstring(now));
}

void Setting::Log(const ablib::string &log){
	if(::GetFileAttributes(m_logfile.c_str()) == -1){
		boost::filesystem::path l(m_logfile);
		::CreateDirectoryReflex(l.parent_path().wstring().c_str());
	}
	static std::wofstream ofs;
	if(!ofs){
		ofs.imbue(std::locale("Japanese",std::locale::ctype));
		ofs.open(m_logfile,std::ios::out | std::ios::app);
	}
	if(ofs){
		ofs << log << std::endl;
	}
#ifndef NDEBUG
	::OutputDebugString(log.c_str());::OutputDebugString(_T("\n"));
#endif
	return;
}

ablib::string Setting::ToAbsolutePath(const ablib::string &path){
	if(path == _T(""))return _T("");
	TCHAR file[4096] = {0};
	::GetModuleFileName(NULL,file,4096);
	boost::filesystem::path me(file);
	
	return boost::filesystem::absolute(path,me.parent_path()).wstring();
}
