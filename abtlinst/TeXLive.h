#pragma once
#include <vector>
#include <system/Process.h>
#include <map>
#include "Network.h"
#include "MainWnd.h"
#include "DownloadDlg.h"

class TeXLive
{
public:
	TeXLive();
	~TeXLive(){m_process.KillChilds();}
#ifdef INSTALL_FROM_LOCAL
	void Set(bool doinst){m_doinst = doinst;}
#else
	void Set(bool doinst,const ablib::string &url,const ablib::string &repository){m_doinst = doinst;m_url = url;m_repository = repository;}
#endif
// インストーラ内の$var{***}に対応．
// インストーラに設定は生かさず，基本的にはabtlinst内で保持する
// インストール直前にインストーラに渡す．
	void SetVar(const ablib::string &varname,const ablib::string &val){m_var[varname] = val;}
	ablib::string GetVar(const ablib::string &varname);
	bool Download(HWND hwnd,MSGFUNC &msgfunc);
	bool Install(HWND hwnd,MSGFUNC &msgfunc);
	void Abort(){
		m_abort = true;
#ifndef INSTALL_FROM_LOCAL
		m_download.Abort();
#endif
	}
	int GetYear();
	std::vector<ablib::string> GetSchemes();
	void UpdateTeXDirs();
	bool DoInst() const{return m_doinst;}
	bool GetDownloadState() const{return m_downstate;}

	void ReadSetting();
	void WriteSetting();

private:
	std::map<ablib::string,ablib::string> m_var;
	ablib::string m_url,m_repository;
	ablib::string m_workdir,m_filedir;
	bool m_doinst;
	bool m_abort;
	bool m_downstate;

	ablib::system::Process m_process;
	void CloseProcess();
#ifndef INSTALL_FROM_LOCAL
	Network::FileData m_file;
	DownloadDlg m_download;
#endif

	void ReadFromProcess(std::function<bool (const ablib::basic_string<char>&)> func,int timeout = -1,int sleeptime = 0);
	void GeneratePerlScript(const ablib::string &file);
//	void CheckPath(HWND hwnd);
	ablib::string ExecCommand(const ablib::basic_string<char> &cmd);
	int m_year;
};

