#pragma once

#include <vector>
#include "Network.h"
#include "MainWnd.h"
#include "DownloadDlg.h"

class Ghostscript
{
public:
	Ghostscript() : m_installresult(false),m_downloadresult(false){}
	~Ghostscript(){}
#ifdef INSTALL_FROM_LOCAL
	void Set(bool doinst){m_doinst = doinst;}
#else
	void Set(bool doinst,const ablib::string &url){m_doinst = doinst;m_url = url;}
#endif
	bool Download(HWND hwnd,MSGFUNC &msgfunc);
	bool GetInfo(HWND hwnd,MSGFUNC &msgfunc);
	std::vector<DownloadFileData> GetDownloadData();
	bool Install(HWND hwnd,MSGFUNC &msgfunc);
	bool GetDoInst() const{return m_doinst;}
	ablib::string GetGSDir();
	void Abort(){
		m_abort = true;
#ifndef INSTALL_FROM_LOCAL
		m_download.Abort();
#endif
	}

	void ReadSetting();
	void WriteSetting();
	bool InstallResult(){return m_installresult;}

private:
	ablib::string ExecToPath(const ablib::string &exec);
	bool CheckandSetTmp(const TCHAR *dir);
	bool CheckandSetTmpEnv(const TCHAR *env,const TCHAR *dir);

#ifndef INSTALL_FROM_LOCAL
	ablib::string m_url;
	DownloadDlg m_download;
#endif
	Network::FileData m_file;
	ablib::string m_gsdir;
	bool m_abort;

	int m_version;

	bool m_doinst,m_downloadresult,m_installresult;
};

