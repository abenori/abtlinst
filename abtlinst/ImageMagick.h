#pragma once
class ImageMagick
{
public:
	ImageMagick(){}
	~ImageMagick(){}
#ifdef INSTALL_FROM_LOCAL
	void Set(bool doinst){m_doinst = doinst;}
#else
	void Set(bool doinst,const ablib::string &url){m_doinst = doinst;m_url = url;m_repository = repository;}
#endif
	bool Download(HWND hwnd,MSGFUNC &msgfunc);
	bool Install(HWND hwnd,MSGFUNC &msgfunc);
	void Abort(){
		m_abort = true;
#ifndef INSTALL_FROM_LOCAL
		m_download.Abort();
#endif
	}
	bool DoInst() const{return m_doinst;}
#ifndef INSTALL_FROM_LOCAL
	bool DownloadState() const{return m_downstate;}
#endif

	void ReadSetting();
	void WriteSetting();

private:
	bool m_doinst,m_abort;
#ifndef INSTALL_FROM_LOCAL
	ablib::string m_url;
#endif
	ablib::string m_filedir,m_filereg,m_file;
};

