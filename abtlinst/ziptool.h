#ifndef __ZIP_H__
#define __ZIP_H__

#include <functional>
#include <zlib/zip.h>
#include "resource.h"

/*
* zipの解凍を担当．エンジンは勿論zlib
*/

class ziptool : public CDialogImpl<ziptool>{
public:
	enum {IDD = IDD_ZIPDLG};

	ziptool() : CDialogImpl<ziptool>(){m_zip.SetOutputMessageFunction(std::bind(&ziptool::WriteMsg,this,std::placeholders::_1));}
	~ziptool(){}
	static bool DeCompress(const TCHAR *fname,const TCHAR *dir){zlib::Zip z;return z.DeCompress(fname,dir);}
	static bool DeCompress(const ablib::string &fname,const ablib::string &dir){return DeCompress(fname.c_str(),dir.c_str());}
//	static bool Compress(const TCHAR *dir);
//	static bool Compress(const ablib::string &dir){return Compress(dir.c_str());}
	bool DeCompresDlg(HWND parent,const TCHAR *fname,const TCHAR *dir);
	bool DeCompresDlg(HWND parent,const ablib::string &fname,const ablib::string &dir){return DeCompresDlg(parent,fname.c_str(),dir.c_str());}

	BEGIN_MSG_MAP(ziptool)
		MSG_WM_INITDIALOG(OnInitDialog);
		MSG_WM_COMMAND(OnCommand);
	END_MSG_MAP()

private:
	enum todo{
		decompress,
		compress
	};
	int m_todo;
	bool m_iscancel;
	HANDLE m_Thread;
	ablib::string m_file,m_dir;
	zlib::Zip m_zip;

	static bool CreateDirectoryReflex(const ablib::string &strPath);

	bool StartDeCompress();
	bool StartCompress();

	bool WriteMsg(const TCHAR *msg);
	bool CheckCancel();

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	BOOL OnCommand(UINT codeNotify, int id, HWND hwndCtl);
//	INT_PTR OnOtherMessage(ablib::window::Dialog *wnd,UINT msg,WPARAM wParam,LPARAM lParam);
};
#endif //__ZIP_H__

