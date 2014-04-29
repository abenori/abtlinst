#include "stdafx.h"
#include <tuple>
#include <system/Process.h>
#include <system/FindFile.h>
#include <system/File.h>
#include <boost/lexical_cast.hpp>
#include <regex>
#include <fstream>
#include "Func.h"
#include "TeXLive.h"
#include "Network.h"
#include "Setting.h"
#include "ziptool.h"
#include "DownloadDlg.h"
#include "Ghostscript.h"
#include "Registory.h"
#include "Path.h"
#include "DlgList.h"

#ifndef INSTALL_FROM_LOCAL
extern Network g_Network;
#endif
extern Setting g_Setting;
extern Ghostscript g_Ghostscript;
extern Path g_Path;

TeXLive::TeXLive() : m_year(0){
	SetVar(_T("abnr_dir_indep"), _T("0"));
	SetVar(_T("abnr_use_texlivedir_prefix"),_T("1"));
	SetVar(_T("option_w32_multi_user"),_T("1"));
}

void TeXLive::ReadSetting(){
	if(g_Setting.ReadINI(_T("TeXLive"),_T("install")) != _T("0"))m_doinst = true;
	else m_doinst = false;
	
	std::vector<ablib::string> keys = g_Setting.INIKeys(_T("TeXLive_var"));
	for(auto key : keys){
		ablib::string val = g_Setting.ReadINI(_T("TeXLive_var"),key);
		if(val != _T(""))SetVar(key,val);
	}
	m_filedir = g_Setting.ToAbsolutePath(g_Setting.ReadSysINI(_T("TeXLive"),_T("FileDir")));
	if(m_filedir != _T("") && m_filedir[m_filedir.length() - 1] != '\\')m_filedir += _T("\\");
}

void TeXLive::WriteSetting(){
	g_Setting.WriteINI(_T("TeXLive"),_T("install"),m_doinst ? _T("1") : _T("0"));
	for(auto &pair : m_var)g_Setting.WriteINI(_T("TeXLive_var"),pair.first,pair.second);
}

bool TeXLive::Download(HWND hwnd,MSGFUNC &msgfunc){
	m_abort = false;
	m_downstate = false;
	if(m_filedir == _T(""))m_filedir = g_Setting.GetDownloadDir() + _T("install-tl\\");
	DEBUGSTRING(_T("TeXLive::Download m_filedir = %s"),m_filedir.c_str());
// INSTALL_FROM_LOCALな時でも初期化は必ずする．
	msgfunc.msg(_T("TeX Live インストーラの初期化中……\n"));
#ifndef INSTALL_FROM_LOCAL
	if(!m_doinst)return true;
	std::vector<Network::FileData> fds;
	if(!g_Network.GetFileData(fds,m_url,msgfunc))return false;
	
	for(const Network::FileData &i : fds){
		if(i.file == _T("install-tl.zip")){
			m_file = i;
			break;
		}
	}
	if(m_file.file == _T("")){
		msgfunc.detail(_T("TeX Live の情報取得に失敗\n"));
		return false;
	}
	
	ablib::system::Time local(ablib::system::File::GetLastWriteTime(g_Setting.GetDownloadDir() + _T("install-tl.zip")));
	local += ablib::system::Time(0,0,1);
	if(local > m_file.time){
		msgfunc.detail(_T("install-tl.zip は更新されていません．以前のものを使います．\n"));
	}else{
		DownloadFileData dfd;
		dfd.url = m_url + _T("/install-tl.zip");
		dfd.size = m_file.size;
		dfd.localdir = g_Setting.GetDownloadDir();
		std::vector<DownloadFileData> dfds;
		dfds.push_back(dfd);
		msgfunc.detail(_T("install-tl.zip をダウンロード\n"));
		::CreateDirectoryReflex(g_Setting.GetDownloadDir().c_str());
		if(m_download.Download(dfds,msgfunc) != DownloadDlg::success){
			msgfunc.detail(_T("install-tl.zip のダウンロードに失敗\n"));
			return false;
		}
		ablib::system::File::DeleteFiles(g_Setting.GetDownloadDir() + _T("install-tl"));
	}
#endif

	if(!ablib::system::File::IsExist(m_filedir.c_str())){
		ziptool zip;
		msgfunc.detail(_T("install-tl.zip を解凍\n"));
		if(!zip.DeCompresDlg(hwnd,g_Setting.GetDownloadDir() + _T("install-tl.zip"),m_filedir)){
			msgfunc.detail(_T("install-tl.zip の解凍に失敗しました．\n"));
			g_Setting.Log(_T("install-tl が見つからなかったか，install-tl.zip の解凍に失敗しました．"));
			return false;
		}
	}
	
	if(ablib::system::File::IsExist(m_filedir + _T("install-tl.bat"))){
		m_workdir = m_filedir;
	}else{
		ablib::system::FindFile ff;
		WIN32_FIND_DATA wfd;
		if(ff.Begin(m_filedir + _T("\\*.*"))){
			if(ff.Next(&wfd)){
				m_workdir = m_filedir + _T("\\") + wfd.cFileName;
			}
		}
	}
	if(m_workdir != _T("") && m_workdir[m_workdir.length() - 1] != '\\')m_workdir += _T("\\");
	
	CloseProcess();
	
	GeneratePerlScript(m_workdir + _T("tlpkg\\installer\\install-menu-abnr.pl"));
	
	TCHAR buf[4096];
	::ExpandEnvironmentStrings((m_workdir + _T("tlpkg\\tlperl\\bin;%PATH%")).c_str(),buf,4095);
	::SetEnvironmentVariable(_T("PATH"),buf);
	::SetEnvironmentVariable(_T("TEXLIVE_INSTALL_PREFIX"),g_Setting.GetInstallDir().c_str());
	
	::SetEnvironmentVariable(_T("PERL5LIB"),(m_workdir + _T("tlpkg\\tlperl\\lib")).c_str());
	::SetEnvironmentVariable(_T("PERL5OPT"),_T(""));
	::SetEnvironmentVariable(_T("PERLIO"),_T(""));
	::SetEnvironmentVariable(_T("PERLIO_DEBUG"),_T(""));
	::SetEnvironmentVariable(_T("PERLLIB"),_T(""));
	::SetEnvironmentVariable(_T("PERL5DB"),_T(""));
	::SetEnvironmentVariable(_T("PERL5DB_THREADED"),_T(""));
	::SetEnvironmentVariable(_T("PERL5SHELL"),_T(""));
	::SetEnvironmentVariable(_T("PERL_ALLOW_NON_IFS_LSP"),_T(""));
	::SetEnvironmentVariable(_T("PERL_DEBUG_MSTATS"),_T(""));
	::SetEnvironmentVariable(_T("PERL_DESTRUCT_LEVEL"),_T(""));
	::SetEnvironmentVariable(_T("PERL_DL_NONLAZY"),_T(""));
	::SetEnvironmentVariable(_T("PERL_ENCODING"),_T(""));
	::SetEnvironmentVariable(_T("PERL_HASH_SEED"),_T(""));
	::SetEnvironmentVariable(_T("PERL_HASH_SEED_DEBUG"),_T(""));
	::SetEnvironmentVariable(_T("PERL_ROOT"),_T(""));
	::SetEnvironmentVariable(_T("PERL_SIGNALS"),_T(""));
	::SetEnvironmentVariable(_T("PERL_UNICODE"),_T(""));
	
	::DeleteFile((m_workdir + _T("installation.profile")).c_str());
#ifdef INSTALL_FROM_LOCAL
	::DeleteFile((g_Setting.GetInstallDir() + _T("2013\\tlpkg\\texlive.tlpdb")).c_str());
#endif

	/*
	if(ablib::system::File::IsExist(g_Setting.GetInstallDir())){
		ablib::string str = _T("デフォルトでインストール先に設定されている\n") + g_Setting.GetInstallDir() + _T("\nが存在しています．削除しても良いですか？\nもし正しくインストールされた TeX Live ならば，「いいえ」を押した後インストーラを終了し，TeX Live のアンインストーラにより削除してください．");
		if(::MessageBox(hwnd,str.c_str(),_T("abtlinst"),MB_YESNO) == IDYES){
			ablib::system::DeleteFiles(g_Setting.GetInstallDir());
		}
	}
	*/

	ablib::system::Process::ProcessStartInfo si;
	si.FileName = _T("");
	si.Arguments = _T("perl install-tl -gui abnr");
	if(m_repository != _T(""))si.Arguments += _T(" -repository ") + m_repository;
	
	si.WorkingDirectory = m_workdir;
	si.RedirectStandardError = true;
	si.RedirectStandardInput = true;
	si.RedirectStandardOutput = true;
	m_process.StartInfo(si);
	if(!m_process.Start()){
		DWORD d = ablib::system::GetLastError();
		msgfunc.detail(_T("TeX Live のインストーラの実行に失敗．\n"));
		g_Setting.Log(_T("TeX Live のインストーラの実行に失敗：GetLastError = ") + boost::lexical_cast<ablib::string>(d));
		return false;
	}
	
	ablib::string msgfromprocess,str;
	ReadFromProcess([&msgfunc,&str,&msgfromprocess,this](const ablib::basic_string<char> &s){
		if(s.find("message from abnr") == 0)return true;
		if(s.find("ABORTED INSTALLATION FOUND") == 0){m_process.Write("N\n");return false;}
		if(s.find("Do you want to continue") == 0)return false;
		str = ablib::TransCode::FromChar(s);
		msgfunc.detail(str);
		msgfromprocess += str;
		return false;
	});
	if(m_process.HasExited()){
		g_Setting.Log(_T("install-tl を動かしましたがエラーが発生しました．"));
		g_Setting.Log(msgfromprocess);
		return false;
	}
	if(m_abort)msgfunc.detail(_T("キャンセル\n"));
	try{m_year = boost::lexical_cast<int>(ExecCommand("getyear"));}catch(boost::bad_lexical_cast e){m_year = 0;}

	if(GetVar(_T("collection-texworks")) == _T(""))SetVar(_T("collection-texworks"),_T("1"));
	UpdateTeXDirs();
	if(!m_abort)msgfunc.detail(_T("TeX Live のインストールの準備ができました．\n"));
	m_downstate = true;
	return true;
}

void TeXLive::ReadFromProcess(std::function<bool (const ablib::basic_string<char>&)> func,int timeout,int sleeptime){
	ablib::basic_string<char> output;
	char buf[4096];
	DWORD be = ::GetTickCount();
	while(true){
		if(timeout >= 0 && ::GetTickCount() - be > timeout)return;
		if(timeout >= 0)buf[m_process.Read(buf,4095)] = '\0';
		else buf[m_process.WaitRead(buf,4095)] = '\0';
		if(!buf[0]){
			if(m_process.HasExited())return;
			if(sleeptime)::Sleep(sleeptime);
		}
		const char *p = buf;
/*
		OutputDebugStringA("TeXLive::ReadFromProcess: processから：[");
		OutputDebugStringA(buf);
		OutputDebugStringA("]\n");
*/
		while(true){
			int i;
			for(i = 0 ; p[i] != '\0' ; ++i){
				if(p[i] == '\n' || p[i] == '\r'){
					output.append(p,i);
					break;
				}
			}
			if(p[i] == '\0'){
				output += p;
				break;
			}
			p += (i + 1);
			if(*(p - 1) == '\r' && *p == '\n')++p;
			output += "\n";
			if(func(output))return;
			if(m_abort)return;
			output = "";
		}
	}
}


void TeXLive::CloseProcess(){
	if(!m_process.HasExited()){
		m_process.Write("quit\n");
		m_process.WaitForExit(100);
		if(!m_process.HasExited()){
			char buf[3] = {3/*Ctrl + C*/,'\n','\0'};
			m_process.Write(buf);
			m_process.WaitForExit(100);
			if(!m_process.HasExited())m_process.KillChilds();
		}
	}
}


struct INSTALLFUNC{
	MSGFUNC f;
	int warn_hook_len,info_hook_len,install_package_hook_len;
	static const char *warn_hook,*info_hook,*install_package_hook;
	std::regex installing_reg;
	std::smatch match;
	CProgressBarCtrl progress_bar;
	bool install_done;
	std::vector<ablib::string> &warnings;
	INSTALLFUNC(MSGFUNC &fu,HWND h_prog,std::vector<ablib::string> &w) : f(fu),progress_bar(h_prog),install_done(false),warnings(w),
		warn_hook_len(::lstrlenA(warn_hook)),
		info_hook_len(::lstrlenA(info_hook)),
		install_package_hook_len(::lstrlenA(install_package_hook)),
		installing_reg("^info_hook by abnr: Installing \\[([0-9]+)/([0-9]+), time/total: ([0-9:?]+)/([0-9:?]+)\\]: ([^ ]+)"){}

	int Str2Time(const ablib::basic_string<char> &s){
		static ablib::basic_string<char> str;
		ablib::string::size_type rb,re = s.length() - 1;
		int time[4] = {0,0,0,0};int i = 0;
		while((rb = s.rfind(':',re)) != ablib::string::npos){
			str.assign(s,rb + 1,re - rb);
			re = rb - 1;
			try{time[i] = boost::lexical_cast<int>(str);}catch(...){}
			++i;
			if(i == 3)break;
		}
		str.assign(s,0,re + 1);
		try{time[i] = boost::lexical_cast<int>(str);}
		catch(...){}
		
		return time[0] + time[1]*60 + time[2]*(60*60) + time[3]*(60*60*24);
	}

	ablib::string Time2Str(int sec){
		if(sec == 0)return _T("0 秒 ");
		int day = sec/(60*60*24);
		sec -= day * (60*60*24);
		int hour = sec/(60*60);
		sec -= hour * (60*60);
		int min = sec/60;
		sec -= min*60;
		return (
			(day == 0 ? ablib::string(_T("")) : boost::lexical_cast<ablib::string>(day) + _T(" 日 ")) + 
			(hour == 0 ? ablib::string(_T("")) : boost::lexical_cast<ablib::string>(hour) + _T(" 時間 ")) + 
			(min == 0 ? ablib::string(_T("")) : boost::lexical_cast<ablib::string>(min) + _T(" 分 ")) + 
			(sec == 0 ? ablib::string(_T("")) : boost::lexical_cast<ablib::string>(sec) + _T(" 秒 "))
		);
	}
	
	bool operator()(const ablib::basic_string<char> &s){
/*
//		if(s != "\n"){l
			::OutputDebugString(_T("（TeX Live のインストーラから）"));
			::OutputDebugStringA(s.c_str());
//		}
*/
		if(s.compare(0,warn_hook_len,warn_hook,warn_hook_len) == 0)return Warn_hook(s);
		else if(s.compare(0,info_hook_len,info_hook,info_hook_len) == 0)return Info_hook(s);
		else if(s.compare(0,install_package_hook_len,install_package_hook,install_package_hook_len) == 0)return Install_Package_hook(s);
		else return Nohook(s);
	}

	bool Warn_hook(const ablib::basic_string<char> &s){
		warnings.push_back(ablib::TransCode::FromChar(s));
		return false;
	}

	bool Nohook(const ablib::basic_string<char> &s){
		return false;
	}

	bool Install_Package_hook(const ablib::basic_string<char> &s){
		static ablib::string str;
		if(install_done){
			str = ablib::TransCode::FromChar(s);
			str.erase(0,install_package_hook_len);
			f.detail(str);
		}
		return false;
	}

	bool Info_hook(const ablib::basic_string<char> &s){
		static ablib::string str;
		if(install_done){
			str = ablib::TransCode::FromChar(s);
			str.erase(0,info_hook_len);
			f.detail(str);
		}else{
			if(std::regex_search(s,match,installing_reg)){
				if(match.str(4) == "" || match.str(4)[0] == '?'){
					f.msg(_T("TeX Live のインストール\n"));
					f.detail(_T("( ") 
						+ ablib::TransCode::FromChar(match.str(1))
						+ _T(" / ")
						+ ablib::TransCode::FromChar(match.str(2))
						+ _T(" ) : ")
						+ ablib::TransCode::FromChar(match.str(5))
						+ _T("\n"));
				}else{
					int done_time = Str2Time(match.str(3));
					int total_time = Str2Time(match.str(4));
					if(total_time != 0)progress_bar.SetPos((done_time*32768)/total_time);
					f.msg(_T("TeX Live のインストール 残り ") + Time2Str(total_time - done_time) + _T("\n"));
					f.detail(
						_T("( ")
						+ ablib::TransCode::FromChar(match.str(1))
						+ _T(" / ")
						+ ablib::TransCode::FromChar(match.str(2))
						+ _T(" ) : ")
						+ ablib::TransCode::FromChar(match.str(5))
						+ _T(" をインストール中\n"));
				}
			}
			if(s.find("Time used for installing the packages") != ablib::string::npos){
				install_done = true;
				f.msg(_T("TeX Live のインストール：最後の調整を行っています．\n"));
				progress_bar.ModifyStyle(0,PBS_MARQUEE);
				progress_bar.SetMarquee(TRUE);
			}
		}
		return false;
	}
};
const char *INSTALLFUNC::warn_hook = "warn_hook by abnr: ";
const char *INSTALLFUNC::info_hook = "info_hook by abnr: ";
const char *INSTALLFUNC::install_package_hook = "install_package_hook by abnr: ";

int TeXLive::GetYear(){
	if(m_year != 0)return m_year;
	ablib::string year = ExecCommand("getyear");
	try{m_year = boost::lexical_cast<int>(year);}
	catch(boost::bad_lexical_cast e){g_Setting.Log(_T("getyear = ") + year + _T(", e.what() = ") + ablib::TransCode::FromChar(e.what()));}
	return m_year;
}

bool TeXLive::Install(HWND hwnd,MSGFUNC &msgfunc){
	m_abort = false;
	if(!m_doinst){
		g_Setting.Log(_T("TeX Live のインストールは行いません．"));
		return true;
	}else g_Setting.Log(_T("TeX Live のインストールを実行．"));
	if(m_process.HasExited())return false;
	CProgressBarCtrl(::GetDlgItem(hwnd,IDC_PROGRESS)).SetPos(0);

	msgfunc.msg(_T("TeX Live のインストール"));

//	CheckPath(hwnd);
	
	::CreateDirectoryReflex(GetVar(_T("TEXDIR")).c_str());
	::CreateDirectoryReflex(GetVar(_T("TEXMFLOCAL")).c_str());
	::CreateDirectoryReflex(GetVar(_T("TEXMFSYSVAR")).c_str());
	::CreateDirectoryReflex(GetVar(_T("TEXMFSYSCONFIG")).c_str());
	
	SetVar(_T("option_path"),g_Setting.SetPath() ? __T("1") : _T("0"));

	m_process.Write(("setvar selected_scheme " + ablib::TransCode::ToChar(GetVar(_T("selected_scheme"))) + "\n").c_str());
	m_process.Write("setscheme\n");

	ablib::string key,val;
//	if(g_Ghostscript.InstallResult())SetVar(_T("tlgs.win32"),_T("0"));
	DEBUGSTRING(_T("TeX Live インストールを開始"));
	g_Setting.Log(_T("パラメータ一覧："));
	for(auto &i : m_var){
		m_process.Write(("setvar " + ablib::TransCode::ToChar(i.first) + " " + ablib::TransCode::ToChar(i.second) + "\n").c_str());
		DEBUGSTRING(_T("$var{%s} = %s"),i.first.c_str(),i.second.c_str());
		g_Setting.Log(i.first + _T(" = ") + i.second);

	}
	m_process.Write("install\n");
	
	std::vector<ablib::string> warnings;
	{
		INSTALLFUNC install_func(msgfunc,::GetDlgItem(hwnd,IDC_PROGRESS),warnings);
		ReadFromProcess(install_func);
	}
	if(m_abort)CloseProcess();
	
	if(m_abort)msgfunc.detail(_T("キャンセルされました．\n"));
	else msgfunc.detail(_T("TeX Live のインストールが終了しました．\n"));
	if(!m_abort && ::IsWindow(hwnd)){
		CProgressBarCtrl(::GetDlgItem(hwnd,IDC_PROGRESS)).ModifyStyle(PBS_MARQUEE,0);
		CProgressBarCtrl(::GetDlgItem(hwnd,IDC_PROGRESS)).SetMarquee(FALSE);
		CProgressBarCtrl(::GetDlgItem(hwnd,IDC_PROGRESS)).SetPos(32768);
	}

	if(!warnings.empty()){
		ablib::string str = _T("TeX Live のインストール中に以下の警告が出ました．\n");
		for(const auto &s : warnings)str += _T("\n") + s;
		::MessageBox(hwnd,str.c_str(),_T("TeX Live のインストール"),0);
		g_Setting.Log(str);
	}

	return true;
}



ablib::string TeXLive::GetVar(const ablib::string &varname){
	if(m_var.find(varname) != m_var.end())return m_var[varname];
	m_var[varname] = ExecCommand("getvar " + ablib::TransCode::ToChar(varname));
	return m_var[varname];
}

void TeXLive::UpdateTeXDirs(){
	// TeXLiveのインストール先を全体のインストール先と連動させるとき
	if(GetVar(_T("abnr_use_texlivedir_prefix")) != _T("0")){
		ablib::string year = boost::lexical_cast<ablib::string>(GetYear());
		SetVar(_T("TEXDIR"),g_Setting.GetInstallDir() + year);
	}
	ablib::string texdir = GetVar(_T("TEXDIR"));
	if(texdir != _T("") && texdir[texdir.length() - 1] != '\\')texdir += _T("\\");
	// 各種ディレクトリを個別に設定しないとき
	if(GetVar(_T("abnr_dir_indep")) != _T("1")){
		if(GetVar(_T("abnr_use_texlivedir_prefix")) == _T("1")){
			SetVar(_T("TEXMFLOCAL"),g_Setting.GetInstallDir() + _T("texmf-local"));
		}else{
			SetVar(_T("TEXMFLOCAL"),texdir + _T("texmf-local"));
		}
		SetVar(_T("TEXMFSYSVAR"),texdir + _T("texmf-var"));
		SetVar(_T("TEXMFSYSCONFIG"),texdir + _T("texmf-config"));
		SetVar(_T("TEXMFHOME"),_T("~/texmf"));
	}
}

ablib::string TeXLive::ExecCommand(const ablib::basic_string<char> &cmd){
	ablib::string output = _T("");
	if(!m_process.HasExited()){
		m_process.Write((cmd + "\n").c_str());
		ReadFromProcess([&output](const ablib::basic_string<char> &s){
			output = ablib::TransCode::FromChar(s);
			if(output != _T(""))output.erase(output.length() - 1);
			return true;
		},2000);
		DEBUGSTRING(_T("TeXLive::ExecCommand : [%s] の返事は [%s]"),
			ablib::TransCode::FromChar(cmd).c_str(),
			output.c_str());
		return output;
/*
		DWORD be = ::GetTickCount();
		char buf[4096];
		while(true){
			if(::GetTickCount() - be > 2000)return _T("");
			buf[m_process.Read(buf,4095)] = '\0';
			for(int i = 0 ; buf[i] != '\0' ; ++i){
				if(buf[i] == '\r' || buf[i] == '\n'){
					output.append(buf,buf + i);
					return ablib::TransCode::FromChar(output);
				}
			}
			output += buf;
		}
*/
	}else return _T("");
}

std::vector<ablib::string> TeXLive::GetSchemes(){
	std::vector<ablib::string> rv;
	m_process.Write("getschemes\n");

	ReadFromProcess([&rv](const ablib::basic_string<char> &s){
		if(s == "\n")return true;
		ablib::string str = ablib::TransCode::FromChar(s);
		if(str == _T(""))return true;
		rv.push_back(str.erase(str.length() - 1));
		return false;
	},2000);
	return rv;
}




void TeXLive::GeneratePerlScript(const ablib::string &file){
/*
	HANDLE fh = ::CreateFile(file.c_str(),GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(fh != INVALID_HANDLE_VALUE){
#define RETTEXT(s) _T(s) _T("\r\n")
		const TCHAR script[] = 
*/
	std::wofstream ofs(file);
	if(ofs){
#define RETTEXT(s) << _T(s) << std::endl
		ofs
			RETTEXT("$::run_menu = \\&run_menu_abnr;")
			RETTEXT("")
			RETTEXT("sub run_menu_abnr{")
			RETTEXT("	my (@args) = @_;")
			RETTEXT("	do_remote_init($::init_remote_needed);")
			RETTEXT("	my @schemes = schemes_ordered_for_presentation();")
			RETTEXT("	while(@args){")
			RETTEXT("		my $f = shift @args;")
			RETTEXT("		if($f =~ m/^-old-installation-found=(.*)$/){")
			RETTEXT("			my $dn = $1;")
			RETTEXT("			import_settings_from_old_tlpdb($dn);")
			RETTEXT("		}")
			RETTEXT("	}")
			RETTEXT("	push @::info_hook,sub{ print \"\\n\" . 'info_hook by abnr: ' . join(' ',@_) . \"\\n\";};")
			RETTEXT("	push @::warn_hook,sub{ print \"\\n\" . 'warn_hook by abnr: ' . join(' ',@_) . \"\\n\";};")
			RETTEXT("	push @::install_packages_hook,sub{ print \"\\n\" . 'install_package_hook by abnr: ' . join(' ',@_) . \"\\n\";};")
			RETTEXT("	print \"message from abnr: Preparation is finished\\n\";")
			RETTEXT("")
			RETTEXT("	while(1){")
			RETTEXT("		$input = <STDIN>;")
			RETTEXT("		chomp($input);")
			RETTEXT("		if($input eq \"install\"){")
			RETTEXT("			return $MENU_INSTALL;")
			RETTEXT("		}elsif($input eq \"quit\"){return $MENU_QUIT;}")
			RETTEXT("		elsif($input =~ /^setvar/){")
			RETTEXT("			@args = split(/\\s+/,$input);")
			RETTEXT("			$vars{$args[1]} = $args[2];")
			RETTEXT("		}elsif($input =~ /^getvar/){")
			RETTEXT("			@args = split(/\\s+/,$input);")
			RETTEXT("			print $vars{$args[1]},\"\\n\";")
			RETTEXT("		}elsif($input =~ /^getyear/){")
			RETTEXT("			print $texlive_release,\"\\n\";")
			RETTEXT("		}elsif($input =~ /^getschemes/){")
			RETTEXT("			print join(\"\\n\",@schemes);")
			RETTEXT("			print \"\\n\\n\";")
			RETTEXT("		}elsif($input =~ /^setscheme/){")
			RETTEXT("			select_scheme($vars{'selected_scheme'});")
			RETTEXT("		}")
			RETTEXT("	}")
			RETTEXT("	")
			RETTEXT("	return $MENU_INSTALL;")
			RETTEXT("}")
		;
#undef RETTEXT
/*
		DWORD size;
		::WriteFile(fh,script,sizeof(TCHAR)*::lstrlen(script),&size,NULL);
		::CloseHandle(fh);
*/
		}
}

/*
void TeXLive::CheckPath(HWND hwnd){
	std::vector<ablib::string> path;
	if(!Path::GetPath(path))return;
	auto ite = std::remove_if(path.begin(),path.end(),[this](const ablib::string &s){
		ablib::string str;
		DWORD size = ::ExpandEnvironmentStrings(s.c_str(),NULL,0);
		TCHAR *buf = new TCHAR[size + 1];
		if(::ExpandEnvironmentStrings(s.c_str(),buf,size + 1) != FALSE)str = buf;
		else str = s;
		delete[] buf;
		return (
			(::CheckSameFile(str.c_str(),(GetVar(_T("TEXDIR")) + _T("\\bin\\win32")).c_str()) != FALSE) || 
			(
				(::GetFileAttributes((str + _T("\\platex.exe")).c_str()) == -1) &&
				(::GetFileAttributes((str + _T("\\ps2pdf.bat")).c_str()) == -1) 
			)
		);
	});
	path.erase(ite,path.end());
	if(!path.empty()){
		DlgList dlg;
		if(dlg.ShowList(hwnd,
			_T("古い TeX のチェック"),
			_T("PATH にある以下のフォルダに TeX と関連するプログラムを発見しました．")
			_T("以前インストールしていた TeX が残っているのではないかと思われます．")
			_T("不都合を起こす可能性があるので，優先度を下げることを推奨します．")
			_T("優先度を下げるフォルダを選んで「OK」を押してください．"),
			path))
		{
			for(auto &p : path){
				g_Path.DeletePath(p);
				g_Path.AddPath(p,true);
			}
		}
	}
}
*/