#pragma once
#include <atlwin.h>
#include <vector>
#include <ablib/Lock.h>
#include "MainWnd.h"

#ifndef INSTALL_FROM_LOCAL

class DownloadDlg : public IBindStatusCallback{
public:

	DownloadDlg(){}
	~DownloadDlg(){}
	enum Result {cancel,fail,success};
	Result Download(const std::vector<DownloadFileData> &dfds,MSGFUNC &msgfunc);
	void Abort(){m_abort = true;}

private:
	bool m_abort;

	struct DownloadList{
		ablib::string name;
		DownloadFileData filedata;
	};
	std::vector<DownloadList> m_downlist;
	std::vector<DownloadList>::iterator m_ite;
	unsigned int m_alreadydownsize,m_totalsize;
	DWORD m_starttime,m_lasttime;
	int m_ikutumeka;
	MSGFUNC m_msgfunc;


	// ‚æ‚­‚í‚©‚Á‚Ä‚È‚¢
	ULONG m_cRef;
	STDMETHODIMP OnProgress(ULONG ulProgress,ULONG ulProgressMax,ULONG ulStatusCode,LPCWSTR szStatusText);
	STDMETHODIMP_(ULONG) AddRef()	{ return (++m_cRef); }
	STDMETHODIMP_(ULONG) Release()	{ return (m_cRef ? --m_cRef : 0L); }
	STDMETHODIMP GetBindInfo( DWORD*, BINDINFO* ) { return E_NOTIMPL; }
	STDMETHODIMP GetPriority( LONG* ) { return E_NOTIMPL; }
	STDMETHODIMP OnLowResource( DWORD ) { return E_NOTIMPL; }
	STDMETHODIMP OnDataAvailable( DWORD, DWORD, FORMATETC*, STGMEDIUM* ) { return E_NOTIMPL; }
	STDMETHODIMP OnObjectAvailable( REFIID, IUnknown* ) { return E_NOTIMPL; }
	STDMETHODIMP OnStartBinding( DWORD, IBinding* ) { return E_NOTIMPL; }
	STDMETHODIMP OnStopBinding( HRESULT, LPCWSTR ) { return E_NOTIMPL; }
	STDMETHODIMP QueryInterface( REFIID riid, void** ppv );

};

#endif // INSTALL_FROM_LOCAL