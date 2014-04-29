#include "stdafx.h"
#include <vector>
#include <regex>
#include <boost/lexical_cast.hpp>
#include <system/Process.h>
#include <system/FindFile.h>
#include <ablib/Pathname.h>
#include <ablib/IsKanji.h>
#include <system/File.h>
#include <boost/filesystem.hpp>
#include "Ghostscript.h"
#include "Network.h"
#include "Path.h"
#include "Registory.h"
#include "Setting.h"
#include "Func.h"

#ifndef INSTALL_FROM_LOCAL
extern Network g_Network;
#endif
extern Path g_Path;
extern Setting g_Setting;

void Ghostscript::ReadSetting(){
	if(g_Setting.ReadINI(_T("Ghostscript"),_T("install")) != _T("0"))m_doinst = true;
	else m_doinst = false;
}

void Ghostscript::WriteSetting(){
	g_Setting.WriteINI(_T("Ghostscript"),_T("install"),m_doinst ? _T("1") : _T("0"));
#ifndef INSTALL_FROM_LOCAL
	g_Setting.WriteINI(_T("Ghostscript"),_T("URL"),m_url);
#endif
}

bool Ghostscript::Download(HWND hwnd,MSGFUNC &msgfunc){
	m_abort = false;
	m_downloadresult = false;

	msgfunc.msg(_T("Ghostscriptのインストールの準備中……\n"));

	std::vector<Network::FileData> fds;
#ifndef INSTALL_FROM_LOCAL
	if(!m_doinst)return true;
	if(!g_Network.GetFileData(fds,m_url,msgfunc))return false;
#else
	ablib::system::FindFile ff;
	ff.Begin(g_Setting.GetDownloadDir() + _T("*.*"));
	WIN32_FIND_DATA wfd;
	while(ff.Next(&wfd)){
		Network::FileData fd;
		fd.file = wfd.cFileName;
		fd.time = ablib::system::Time(wfd.ftLastWriteTime);
		fd.size = wfd.nFileSizeLow;
		fds.push_back(fd);
	}
#endif

	std::wregex reg(g_Setting.ReadSysINI(_T("GS"),_T("Reg")));
	std::vector<Network::FileData> gsfds;

	for(auto &i : fds){
//		::OutputDebugString((i.file + _T("\n")).c_str());
		if(std::regex_match(i.file,reg))gsfds.push_back(i);
	}
	if(gsfds.empty()){
		msgfunc.detail(_T("Ghostscript のインストーラが見つかりませんでした．\n"));
		g_Setting.Log(_T("Ghostscript のインストーラがみつかりませんでした．"));
		return false;
	}

	reg.assign(_T("[0-9]+"));
	std::wsmatch reg_results;
	std::vector<unsigned int> version;
	for(auto &i : gsfds){
		if(std::regex_search(i.file,reg_results,reg))version.push_back(boost::lexical_cast<int>(reg_results.str()));
		else version.push_back(0);
	}
	std::vector<unsigned int>::iterator max = std::max_element(version.begin(),version.end());
	if(max == version.end()){
		m_file = gsfds[0];
		if(!version.empty())m_version = version[0];
		else m_version = 0;
	}else{
		m_file = gsfds[max - version.begin()];
		m_version = *max;
	}

#ifndef INSTALL_FROM_LOCAL
	ablib::system::Time local = ablib::system::File::GetLastWriteTime(g_Setting.GetDownloadDir() + m_file.file);
	if(local + ablib::system::Time(0,0,1) > m_file.time){
		msgfunc.detail(_T("ローカルの Ghostscript のインストーラが最新のようです．ダウンロードしません．\n"));
	}else{
		DownloadFileData dfd;
		dfd.url = m_url + _T("/") + m_file.file;
		dfd.size = m_file.size;
		dfd.localdir = g_Setting.GetDownloadDir();
		std::vector<DownloadFileData> dfds;
		dfds.push_back(dfd);
		::CreateDirectoryReflex(g_Setting.GetDownloadDir().c_str());
		if(m_download.Download(dfds,msgfunc) != DownloadDlg::success){
			msgfunc.detail(_T("Ghostscript のインストーラのダウンロードに失敗\n"));
			return false;
		}
	}
#endif // INSTALL_FROM_LOCAL
	m_downloadresult = true;
	return true;
}


bool Ghostscript::Install(HWND hwnd,MSGFUNC &msgfunc){
	m_installresult = false;
	if(!m_doinst)return true;
	if(!m_downloadresult)return false;
	ablib::string file = g_Setting.GetDownloadDir() + m_file.file;
	boost::filesystem::path path(file);
	CProgressBarCtrl progress_bar(::GetDlgItem(hwnd,IDC_PROGRESS));
	static const int PROGRESSBAR_MAX = 32768;

	if(file.length() < 4 || file.substr(file.length() - 4) != _T(".exe")){
		::MessageBox(hwnd,_TR("古い Ghostscript を発見しました．このバージョンには対応していません．"),_T("abtlinst"),0);
		return false;
	}
	ablib::system::Process process;
	ablib::system::Process::ProcessStartInfo s;

	ablib::string gsdir;
	gsdir = g_Setting.GetInstallDir() + _T("gs");
	if(m_version != 0){
		ablib::string ver = boost::lexical_cast<ablib::string>(m_version);

		// 後ろから二つおきにピリオドを挿入
		ablib::string::size_type start = (ver.length() % 2);
		int len = ver.length();
		for(ablib::string::size_type i = 0 ; i < len/2 ; ++i){
			ver.insert(start + 3*i,_T("."));
		}
		gsdir += _T("\\gs") + ver;
	}
	
	s.Arguments = _T("\"") + file + _T("\" /S /D=") + gsdir;
//	s.Arguments = _T("\"") + file + _T("\" /D=") + g_Setting.GetInstallDir() + _T("gs");
//	s.Arg = _T("\"") + file + _T("\"");
	s.FileName = _T("");
	s.WorkingDirectory = path.parent_path().wstring();
	s.RedirectStandardError = false;
	s.RedirectStandardOutput = false;
	s.RedirectStandardInput = false;
	s.ShowState = ablib::system::Process::ProcessStartInfo::show;
	s.CreateNoWindow = false;
	process.StartInfo(s);
	CheckandSetTmp(g_Setting.GetTmpDir().c_str());
	msgfunc.msg(_T("Ghostscript のインストール\n"));
	msgfunc.detail(_T("Ghostscript をインストール中\n"));
	if(!process.Start()){
		g_Setting.Log(file + _T(" の起動に失敗"));
		return false;
	}else{
		process.WaitForExit();
	}
	
	progress_bar.SetPos(PROGRESSBAR_MAX/2);
	gsdir += _T("\\");

//	ablib::string gsdir = GetGSDir();
	if(g_Setting.SetPath()){
		if(gsdir != _T("")){
			g_Path.AddPath(gsdir + _T("bin"));
			g_Path.AddPath(gsdir + _T("lib"));
		}else g_Setting.Log(_TR("Ghostscript のインストールパス取得に失敗したので，PATH の設定を行いません"));
	}

	msgfunc.detail(_T("cidfmap の生成\n"));
	LPITEMIDLIST id;
	LPMALLOC mal;
	TCHAR fontdir[4096];
	if(
		FAILED(::SHGetMalloc(&mal)) ||
		FAILED(::SHGetSpecialFolderLocation(NULL,CSIDL_FONTS,&id)) ||
		!::SHGetPathFromIDList(id,fontdir)
	){
		g_Setting.Log(_T("cidfmap の生成に失敗（フォントフォルダの取得に失敗）．"));
		return false;
	}
		
	mal->Free(id);
	if(fontdir[::lstrlen(fontdir) - 1] != '\\')::lstrcat(fontdir,_T("\\"));
	
	ablib::string arg = ablib::string(_T("-q -dBATCH -sFONTDIR=")) + fontdir + _T(" -sCIDFMAP=") + gsdir + _T("lib\\cidfmap") + _T(" ") + gsdir + _T("lib\\mkcidfm.ps");
	arg.replace(_T("\\"),_T("/"));
	s.Arguments = _T("\"") + gsdir + _T("\\bin\\gswin32c.exe\" ") + arg;
	s.WorkingDirectory = gsdir + _T("\\bin");
	process.StartInfo(s);
	if(!process.Start()){
		g_Setting.Log(s.Arguments + _T(" の実行に失敗"));
		return false;
	}else{
		process.WaitForExit();
	}
	progress_bar.SetPos(PROGRESSBAR_MAX);
	
	m_installresult = true;
	return true;
}

ablib::string Ghostscript::GetGSDir(){
	if(m_gsdir != _T(""))return m_gsdir;
	std::wregex reg;
	try{
		reg.assign(_T("(AFPL|GNU|GPL|ESP) Ghostscript (gs)?([0-9]*\\.[0-9]*)"));
	}catch(...){ASSERT(false);}
	std::vector<ablib::string> keys;
	if(!Registory::EnumKey(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),keys))return _T("");
	for(auto &key : keys){
		std::wsmatch reg_results;
		if(std::regex_match(key,reg_results,reg)){
			ablib::string val = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
			val += key;
			ablib::string str;
			if(!Registory::ReadSZ(HKEY_LOCAL_MACHINE,val.c_str(),_T("UninstallString"),str))return _T("");
			bool dq = false;
			ablib::string::size_type p = 0;
			str = ExecToPath(str);
			if(str != _T("")){
				m_gsdir = str;
				m_gsdir.erase(m_gsdir.rfind(_T("\\")));
				m_gsdir += _T("\\");
				if(::GetFileAttributes((m_gsdir + _T("bin\\gswin32.exe")).c_str()) == -1){
					m_gsdir += _T("gs");
					m_gsdir += reg_results[3];
					m_gsdir += _T("\\");
				}
				DEBUGSTRING(_T("Install::GetGSDir : GSDir = %s"),m_gsdir.c_str());
				return m_gsdir;
			}
		}
	}
	return m_gsdir;
}

ablib::string Ghostscript::ExecToPath(const ablib::string &exec){
	ablib::string target = exec;
	bool dq = false;
	ablib::string::size_type p = 0;
	for(;p < target.length() ; ++p){
		if(target[p] == '"')dq = !dq;
		else if(target[p] == ' ' && !dq){
			ablib::string c;
			c.assign(target,0,p);c.replace(_T("\""),_T(""));
			break;
//			if(::GetFileAttributes(c.c_str()) != -1)break;
//			else if(::GetFileAttributes((c + _T(".exe")).c_str()) != -1)break;
		}
#ifndef UNICODE
		else if(ablib::IsKanji(target[p]))++p;
#endif
	}
	target.erase(p);target.replace(_T("\""),_T(""));
/*
	if(::GetFileAttributes(target.c_str()) == -1){
		target += _T(".exe");
		if(::GetFileAttributes(target.c_str()) == -1)return _T("");
	}
*/
	return target;
}

bool Ghostscript::CheckandSetTmp(const TCHAR *dir){
	ablib::basic_string<char> dchar = ablib::TransCode::ToChar(dir);
	ablib::string d;
	for(ablib::string::size_type i = 0 ; i < dchar.length() ; ++i){
		if(ablib::IsKanji(dchar[i])){
			d = ablib::TransCode::FromChar(::getenv("windir"));
			d += _T("\\temp");
		}
	}
	bool a,b;
	a = CheckandSetTmpEnv(_T("TMP") ,d.c_str());
	b = CheckandSetTmpEnv(_T("TEMP"),d.c_str());
	if(a || b)::CreateDirectoryReflex(d.c_str());
	return (a || b);
}

bool Ghostscript::CheckandSetTmpEnv(const TCHAR *env,const TCHAR *dir){
	ablib::basic_string<char> var = ::getenv(ablib::TransCode::ToChar(env).c_str());
	for(ablib::string::size_type i = 0 ; i < var.length() ; ++i){
		if(ablib::IsKanji(var[i])){
			::SetEnvironmentVariable(env,dir);
			return true;
		}
	}
	return false;
}
