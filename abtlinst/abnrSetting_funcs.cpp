#include "stdafx.h"
#include "abnrSetting_funcs.h"
#include <regex>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <system/FindFile.h>
#include <system/Process.h>
#include "Func.h"

namespace abnrSetting_funcs{

bool GetTeXLiveDir(ablib::string &tldir,std::function<void (const ablib::string&)> error){
	ablib::string tlstartmenu;
	int year;
	GetTeXLiveDirFromStartMenu(year,tldir,tlstartmenu);
	if(tldir != _T(""))return true;
	else{
		error(_T("TeX Live フォルダの取得に失敗．"));
		return false;
	}
}


bool kanji_config_updmap(
	const ablib::string &tldir,
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error,
	const ablib::string &font
){
	detail(_T("デフォルトの和文フォントを ") + font + _T(" に変更．\n"));
	ablib::system::Process process;
	ablib::system::Process::ProcessStartInfo si;
	si.FileName = _T("");
	si.Arguments =  tldir + _T("bin\\win32\\kanji-config-updmap-sys.exe ") + font;
	si.RedirectStandardOutput = si.RedirectStandardError = true;
	si.RedirectStandardInput = false;
	si.WorkingDirectory = tldir + _T("bin\\win32");
	process.StartInfo(si);
	if(!process.Start()){
		error(si.Arguments + _T(" の実行に失敗 : GetLastError = ") + boost::lexical_cast<ablib::string>(ablib::system::GetLastError()));
		return false;
	}
	while(!process.HasExited()){
		char buf[4096];
		process.WaitRead(buf,4095);
	}
	return true;
}

bool writetexmfcnf(
	const ablib::string &tldir,
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error
){
	detail(_T("TEXDIR 以下の texmf.cnf に追記します．\n"));

	if(tldir == _T("")){
		error(_T("TEXDIR が空です．texmf.cnf への追記に失敗しました．"));
		return false;
	}
	ablib::string texmfcnf = tldir + _T("texmf.cnf");
	
	std::wifstream ifs(texmfcnf);
	ablib::string buf;
	bool added = false;
	if(ifs){
		while(!ifs.eof() && !ifs.bad()){
			std::getline(ifs,buf);
			if(buf == _T("% added by abtlinst.exe")){
				added = true;
				error(_T("texmf.cnf には既に追記がなされているようです．"));
				break;
			}
		}
	}

	if(!added){
		std::wofstream ofs(texmfcnf,std::ios::out | std::ios::app);
		if(ofs){
#define RETTEXT(s) << _T(s) << std::endl
			ofs
				RETTEXT("% added by abtlinst.exe")
				RETTEXT("")
				RETTEXT("shell_escape_commands = \\")
				RETTEXT("bibtex,pbibtex,jbibtex,repstopdf,epspdf,extractbb,\\")
				RETTEXT("makeindex,mendex,mpost,pmpost,upmpost,kpsewhich")
				RETTEXT("")
				RETTEXT("guess_input_kanji_encoding = 1")
			;
#undef RETTEXT
			return true;
		}else{
			error(_T("ファイル [") + texmfcnf + _T("] を開けませんでした．"));
			return false;
		}
	}else return true;
}

void GenerateToolsINI(std::wofstream &wofs){
#define RETTEXT(s) << _T(s) << std::endl
	wofs
		RETTEXT("[001]")
		RETTEXT("name=pdfTeX")
		RETTEXT("program=pdftex.exe")
		RETTEXT("arguments=$synctexoption, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[002]")
		RETTEXT("name=pdfLaTeX")
		RETTEXT("program=pdflatex.exe")
		RETTEXT("arguments=$synctexoption, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[003]")
		RETTEXT("name=LuaTeX")
		RETTEXT("program=luatex.exe")
		RETTEXT("arguments=$synctexoption, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[004]")
		RETTEXT("name=LuaLaTeX")
		RETTEXT("program=lualatex.exe")
		RETTEXT("arguments=$synctexoption, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[005]")
		RETTEXT("name=XeTeX")
		RETTEXT("program=xetex.exe")
		RETTEXT("arguments=$synctexoption, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[006]")
		RETTEXT("name=XeLaTeX")
		RETTEXT("program=xelatex.exe")
		RETTEXT("arguments=$synctexoption, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[007]")
		RETTEXT("name=ConTeXt (LuaTeX)")
		RETTEXT("program=context.exe")
		RETTEXT("arguments=--synctex, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[008]")
		RETTEXT("name=ConTeXt (pdfTeX)")
		RETTEXT("program=texexec.exe")
		RETTEXT("arguments=--synctex, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[009]")
		RETTEXT("name=ConTeXt (XeTeX)")
		RETTEXT("program=texexec.exe")
		RETTEXT("arguments=--synctex, --xtx, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[010]")
		RETTEXT("name=BibTeX")
		RETTEXT("program=bibtex.exe")
		RETTEXT("arguments=$basename")
		RETTEXT("showPdf=false")
		RETTEXT("")
		RETTEXT("[011]")
		RETTEXT("name=MakeIndex")
		RETTEXT("program=makeindex.exe")
		RETTEXT("arguments=$basename")
		RETTEXT("showPdf=false")
		RETTEXT("")
		RETTEXT("[012]")
		RETTEXT("name=pLaTeX (ptex2pdf)")
		RETTEXT("program=ptex2pdf")
		RETTEXT("arguments=-l, -ot, -kanji=utf8 $synctexoption, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[013]")
		RETTEXT("name=upLaTeX (ptex2pdf)")
		RETTEXT("program=ptex2pdf")
		RETTEXT("arguments=-l, -u, -ot, -kanji=utf8 $synctexoption, $fullname")
		RETTEXT("showPdf=true")
		RETTEXT("")
		RETTEXT("[014]")
		RETTEXT("name=pBibTeX")
		RETTEXT("program=pbibtex.exe")
		RETTEXT("arguments=$basename")
		RETTEXT("showPdf=false")
		RETTEXT("")
		RETTEXT("[015]")
		RETTEXT("name=mendex")
		RETTEXT("program=mendex.exe")
		RETTEXT("arguments=$basename")
		RETTEXT("showPdf=false")
		;
#undef RETTEXT
}

bool TeXworksSetting(
	const ablib::string &tldir,
	std::function<void(const ablib::string&)> msg,
	std::function<void(const ablib::string&)> detail,
	std::function<void(const ablib::string&)> error
	){
	if (::GetFileAttributes((tldir + _T("tlpkg\\texworks")).c_str()) == -1){
		error(_T("TeXworks フォルダが見つかりません．"));
		return true;
	}

	//	msgfunc(_T("TeXworks の設定をいじります．\n"),false);

	/*
		FILE *fp = ::_tfopen((m_tldir + _T("bin\\win32\\pdfplatex.bat")).c_str(),_T("w"));
		if(!fp)return false;
		::_ftprintf(fp,_T("@echo off\nplatex -synctex=1 -jobname=\"%~n1\" -kanji=utf8 -guess-input-enc %1 && ^\ndvipdfmx \"%~n1\""));
		::fclose(fp);
		*/
	detail(_T("TeXworks の設定をあべのり的にします．\n"));
	//::SetEnvironmentVariable(_T("USERPROFILE"), _T("C:\\Users\\Abe_Noriyuki\\Desktop\\あべのり\\表示"));

	ablib::system::Process process;
	ablib::system::Process::ProcessStartInfo si;
	si.FileName = _T("");
	si.Arguments = tldir + _T("bin\\win32\\kpsewhich.exe -var-value=TW_INIPATH");
	si.RedirectStandardOutput = si.RedirectStandardError = true;
	si.RedirectStandardInput = false;
	si.WorkingDirectory = tldir + _T("bin\\win32");
	process.StartInfo(si);
	if (!process.Start()){
		error(_T("kpsewhich の起動に失敗しました．"));
		return false;
	}
	char buf[4096];
	ablib::string inipath;
	while (DWORD size = process.WaitRead(buf, 4095)){
		buf[size] = '\0';
		inipath += ablib::TransCode::FromChar(buf);
	}
	DEBUGSTRING(_T("abnrSetting::TeXWorksSetting kpsewhich -var-varlue=TW_INIPATH = [%s]"), inipath.c_str());
	inipath.replace(_T("\r"), _T(""));
	inipath.replace(_T("\n"), _T(""));
	std::wregex reg;
	try{ reg.assign(_T("^ *(.*?)( *)$")); }
	catch (...){ ASSERT(false); }
	inipath = std::regex_replace(inipath, reg, ablib::string(_T("$1")));
	inipath.replace(_T("/"), _T("\\"));
	DEBUGSTRING(_T("abnrSetting::TeXWorksSetting inipath = [%s]"), inipath.c_str());

	ablib::string tools_ini = inipath + _T("\\configuration\\tools.ini");
	//	if(::GetFileAttributes(tools_ini.c_str()) == -1){
	::CreateDirectoryReflex((inipath + _T("\\configuration\\")).c_str());
	{
		std::wofstream ofs(tools_ini);
		if (ofs){
			GenerateToolsINI(ofs);
			::CreateDirectoryReflex((inipath + _T("\\TUG\\")).c_str());
			if (::WritePrivateProfileString(_T("General"), _T("defaultEngine"), _T("pLaTeX (ptex2pdf)"), (inipath + _T("\\TUG\\TeXworks.ini")).c_str()) == FALSE){
				error(inipath + _T("\\TUG\\TeXworks.ini への書き込みに失敗：GetLastError = ") + boost::lexical_cast<ablib::string>(::GetLastError()));
			}
		}
		else{
			error(tools_ini + _T("が開けませんでした．"));
		}
	}
	//	}

	DWORD size = ::ExpandEnvironmentStrings(_T("%USERPROFILE%\\TeXworks\\configuration"), NULL, 0);
	TCHAR *b = new TCHAR[size + 10];
	if (::ExpandEnvironmentStrings(_T("%USERPROFILE%\\TeXworks\\configuration"),b, size + 1)){
		if (::GetFileAttributes(b) != -1){
			tools_ini = ablib::string(b) + _T("\\tools.ini");
			if (::GetFileAttributes(tools_ini.c_str()) == -1){
				detail(tools_ini + _T(" を生成します．"));
				{
					std::wofstream ofs(tools_ini);
					if (ofs)GenerateToolsINI(ofs);
					else error(tools_ini + _T(" が開けませんでした．"));
				}
			}else{
				detail(tools_ini + _T(" に追記します．"));
				int secnum = 1;
				ablib::string section;
				bool platex = true, uplatex = true, pbibtex = true, mendex = true;
				for (; secnum < 1000; ++secnum){
					section = boost::lexical_cast<ablib::string>(secnum);
					section = ablib::string(_T("000")).substr(section.length()) + section;
					TCHAR buf2[4096];
					::lstrcpy(buf2, _T(""));
					::GetPrivateProfileString(section.c_str(), _T("name"), _T(""), buf2, 4095, tools_ini.c_str());
					if (::lstrcmp(buf2, _T("")) == 0)break;
					else if (::lstrcmp(buf2, _T("pLaTeX (ptex2pdf)")) == 0)platex = false;
					else if (::lstrcmp(buf2, _T("upLaTeX (ptex2pdf)")) == 0)uplatex = false;
					else if (::lstrcmp(buf2, _T("pBibTeX")) == 0)pbibtex = false;
					else if (::lstrcmp(buf2, _T("mendex")) == 0)mendex = false;
				}
				--secnum;
				if (platex){
					++secnum;
					section = boost::lexical_cast<ablib::string>(secnum);
					section = ablib::string(_T("000")).substr(section.length()) + section;
					::WritePrivateProfileString(section.c_str(), _T("name"), _T("pLaTeX (ptex2pdf)"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("program"), _T("ptex2pdf"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("arguments"), _T("-l, -ot, -kanji=utf8 $synctexoption, $fullname"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("showPdf"), _T("true"), tools_ini.c_str());
				}
				if (uplatex){
					++secnum;
					section = boost::lexical_cast<ablib::string>(secnum);
					section = ablib::string(_T("000")).substr(section.length()) + section;
					::WritePrivateProfileString(section.c_str(), _T("name"), _T("upLaTeX (ptex2pdf)"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("program"), _T("ptex2pdf"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("arguments"), _T("-l, -u, -ot, -kanji=utf8 $synctexoption, $fullname"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("showPdf"), _T("true"), tools_ini.c_str());
				}
				if (pbibtex){
					++secnum;
					section = boost::lexical_cast<ablib::string>(secnum);
					section = ablib::string(_T("000")).substr(section.length()) + section;
					::WritePrivateProfileString(section.c_str(), _T("name"), _T("pBibTeX"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("program"), _T("pbibtex.exe"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("arguments"), _T("$basename"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("showPdf"), _T("false"), tools_ini.c_str());
				}
				if (mendex){
					++secnum;
					section = boost::lexical_cast<ablib::string>(secnum);
					section = ablib::string(_T("000")).substr(section.length()) + section;
					::WritePrivateProfileString(section.c_str(), _T("name"), _T("mendex"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("program"), _T("mendex.exe"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("arguments"), _T("$basename"), tools_ini.c_str());
					::WritePrivateProfileString(section.c_str(), _T("showPdf"), _T("false"), tools_ini.c_str());
				}
			}
		}
	}
	delete[] b;

	return true;
}


bool GetTeXLiveDirFromStartMenu(int &year,ablib::string &tldir,ablib::string &tlstartmenu){
	// スタートメニューフォルダを取得
//	const GUID csidls[] = {FOLDERID_CommonStartMenu,FOLDERID_StartMenu};
	int csidls[] = {CSIDL_COMMON_PROGRAMS,CSIDL_PROGRAMS};

//	for(const GUID csidl : csidls){
	for(int csidl : csidls){
	std::vector<ablib::string> results;
		std::vector<int> years;
		ablib::string startmenu;
//		PWSTR buf = NULL;
//		if(FAILED(::SHGetKnownFolderPath(csidl,0,NULL,&buf)))continue;
//		startmenu = buf;
//		::CoTaskMemFree(buf);

/*
		LPITEMIDLIST id;
		LPMALLOC mal;
		if(FAILED(::SHGetMalloc(&mal)))continue;
		if(FAILED(::SHGetSpecialFolderLocation(NULL,csidl,&id)))continue;
		TCHAR buf[4096];
		if(!::SHGetPathFromIDList(id,buf))continue;
		mal->Free(id);
		startmenu = buf;
*/
		TCHAR buf[MAX_PATH];
		if(
			FAILED(::SHGetFolderPath(NULL,csidl,NULL,SHGFP_TYPE_CURRENT,buf)) &&
			FAILED(::SHGetFolderPath(NULL,csidl,NULL,SHGFP_TYPE_DEFAULT,buf))
			)continue;
		startmenu = buf;
		if(startmenu != _T("") && startmenu[startmenu.length() - 1] != '\\')startmenu += _T("\\");
//		::MessageBox(NULL,startmenu.c_str(),NULL,0);
	

		// TeX Live [0-9]+を探す．（複数ある場合は年を見て最近のもの．）
		ablib::system::FindFile ff;
		std::wregex reg;
		try{reg.assign(_T("TeX Live ([0-9]+)"),std::regex::ECMAScript | std::regex::icase);}catch(...){ASSERT(false);}
		std::wsmatch match;
		WIN32_FIND_DATA wfd;
		ablib::string s;
		if(ff.Begin((startmenu + _T("*.*")).c_str())){
			while(ff.Next(&wfd)){
				s = wfd.cFileName;
				if(std::regex_search(s,match,reg)){
					results.push_back(wfd.cFileName);
					years.push_back(boost::lexical_cast<int>(match.str(1)));
				}
			}
		}
		if(results.empty())continue;
		std::vector<int>::iterator max = std::max_element(years.begin(),years.end());
		tlstartmenu = startmenu + _T("\\") + results[max - years.begin()] + _T("\\");
		if(year == 0)year = *max;

		DEBUGSTRING(_T("abnrSetting_funcs::GetTeXLiveDir : m_tlstatmenu = %s"),tlstartmenu.c_str());

		// その中のTeX Live Manager.lnkを読みに行く
		ablib::string tlmgr = tlstartmenu + _T("TeX Live Manager.lnk");

		IShellLink *isl;
		IPersistFile *ipf;
	
		::CoInitialize(NULL);
		if(FAILED(::CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&isl))){
			::CoUninitialize();
			continue;
		}
		if(FAILED(isl->QueryInterface(IID_IPersistFile,(void**)&ipf))){
			isl->Release();
			::CoUninitialize();
			continue;
		}


#ifdef UNICODE
		ablib::basic_string<wchar_t> lnk = tlmgr;
#else
		ablib::basic_string<wchar_t> lnk = ablib::TransCode::Trans(tlmgr);
#endif
		ipf->Load(lnk.c_str(),STGM_READ);
		isl->Resolve(NULL,SLR_UPDATE);

		WCHAR buf2[4096];
		isl->GetPath(buf2,4095,NULL,0);
#ifdef UNICODE
		ablib::string tlmgrpath = buf2;
#else
		ablib::string tlmgrpath = ablib::TransCode::Trans(buf2);
#endif
		DEBUGSTRING(_T("abnrSetting_funcs::GetTeXLiveDirFromStartMenu: tlmgrpath = %s"),tlmgrpath.c_str());

		ipf->Release();
		isl->Release();
		::CoUninitialize();

		// tlmgrpath = (TeX Liveのディレクトリ)\bin\win32\tlmgr-gui.exeのはず
		reg.assign(_T("^(.*)bin\\\\win32\\\\tlmgr-gui\\.exe"));
		if(!std::regex_search(tlmgrpath,match,reg))continue;
		tldir = match.str(1);
		return true;
	}
	return false;
}


}