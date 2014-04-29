#include "stdafx.h"
#include "DownloadDlg.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "Network.h"

#ifndef INSTALL_FROM_LOCAL
extern Network g_Network;

DownloadDlg::Result DownloadDlg::Download(const std::vector<DownloadFileData> &dfds,MSGFUNC &msgfunc){
	m_abort = false;
	m_msgfunc = msgfunc;
	m_downlist.clear();
	DownloadList dl;
	for(auto &i : dfds){
//	std::for_each(dfds.begin(),dfds.end(),[&dl,this](const DownloadFileData &i){
		dl.filedata = i;
		ablib::string::size_type r = dl.filedata.url.rfind(_T("/"));
		ASSERT(r != ablib::string::npos);
		dl.name = dl.filedata.url.substr(r+1);
		m_downlist.push_back(dl);
	}
	struct{
		DWORD dwAccessType;
		const char *lpszProxy;
		const char *lpszProxyBypass;
	}ipi;
	Network::ProxyData proxy = g_Network.GetProxy();
	if(proxy.port != 0 && proxy.proxy != _T("")){
		ablib::basic_string<char> serv = ablib::TransCode::ToChar(proxy.proxy + _T(":") + boost::lexical_cast<ablib::string>(proxy.port));
		ipi.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
		ipi.lpszProxy = serv.c_str();
		ipi.lpszProxyBypass = "<local>";
		if(::UrlMkSetSessionOption(INTERNET_OPTION_PROXY,&ipi,sizeof(ipi),0) != S_OK){
			return fail;
		}
	}

	m_ikutumeka = 0;
	for(m_ite = m_downlist.begin() ; m_ite != m_downlist.end() ; ++m_ite){
		++m_ikutumeka;
		msgfunc.detail(m_ite->name);
		::DeleteUrlCacheEntry(m_ite->filedata.url.c_str());
		if(::URLDownloadToFile(NULL,m_ite->filedata.url.c_str(),(m_ite->filedata.localdir + _T("\\") + m_ite->name).c_str(),0,this) != S_OK){
			msgfunc.detail(_T("\n"));
			::DeleteFile((m_ite->filedata.localdir + _T("\\") + m_ite->name).c_str());
			return fail;
		}
		msgfunc.detail(_T("\n"));
		m_alreadydownsize += m_ite->filedata.size;
	}
	return success;
}


STDMETHODIMP DownloadDlg::OnProgress(ULONG ulProgress,ULONG ulProgressMax,ULONG ulStatusCode,LPCWSTR szStatusText){
	TCHAR msg[1024];
	if(m_abort)return E_ABORT;

	ULONG maxsize = ulProgressMax;
	if(ulProgressMax == 0)maxsize = m_ite->filedata.size;
	
	ulProgress = min(ulProgress,maxsize);
	::wsprintf(msg,_T("%s : %ld/%ld"),m_ite->name.c_str(),ulProgress,maxsize);
	m_msgfunc.detail(msg);

	return S_OK;
}

STDMETHODIMP DownloadDlg::QueryInterface(REFIID riid,void **ppv){
	*ppv = NULL;
	AddRef();
	if(IsEqualIID(riid,IID_IUnknown) || IsEqualIID(riid,IID_IBindStatusCallback)){
		*ppv = static_cast<IBindStatusCallback*>(this);
		return NOERROR;
	}else{
		Release();
		return E_NOINTERFACE;
	}
}

#endif // INSTALL_FROM_LOCAL