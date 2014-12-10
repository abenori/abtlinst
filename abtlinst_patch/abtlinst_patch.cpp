// abtlinst_patch.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <iostream>
#include <boost/program_options.hpp>
#include "../abtlinst/abnrSetting_funcs.h"
#include "../abtlinst/TeX2img_func.h"

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	int argc;
	TCHAR **argv = ::CommandLineToArgvW(::GetCommandLine(),&argc);
	bool TeXworks = true,ipaex = true,texmfcnf = true,tex2img = true;
	for(int i = 0 ; i < argc ; ++i){
		if(::lstrcmp(argv[i],_T("/noTeXworks")) == 0)TeXworks = false;
		else if(::lstrcmp(argv[i],_T("/noipaex")) == 0)ipaex = false;
		else if(::lstrcmp(argv[i],_T("/notexmfcnf")) == 0)texmfcnf = false;
		else if(::lstrcmp(argv[i],_T("/noTeX2img")) == 0)tex2img = false;
	}

	ablib::string str;

	if(texmfcnf)str += _T("・texmf.cnf の追記\n");
	if(ipaex)str += _T("・デフォルトのフォントを ipaex に変更\n");
	if(TeXworks)str += _T("・TeXworks の設定を日本語的にする\n");
	if(tex2img)str += _T("・TeX2img を設定する\n");

	if(str == _T("")){
		::MessageBox(NULL,_T("何も実行しません．"),_T("abtlinst_patch"),MB_OK);
		return 0;
	}
	str = _T("TeX Live への設定を再度行います．具体的には\n\n") + str + _T("\nを行います．よろしいですか？");

	int r = ::MessageBox(NULL,str.c_str(),_T("abtlinst_patch"),MB_YESNO);
	if(r == IDNO){
		::MessageBox(NULL,_T("設定を行わず終了します．"),_T("abtlinst_patch"),0);
		return 0;
	}

	::AllocConsole();
	FILE *fp;
	::_tfreopen_s(&fp,_T("CONOUT$"), _T("w"), stdout);

	ablib::string error_msg,detail_msg;
	std::function<void (const ablib::string &)> msg,detail,error;
	msg = [](const ablib::string &msg){::_tcprintf(_T("%s"),msg.c_str());};
	detail = [&detail_msg](const ablib::string &msg){detail_msg += msg + _T("\n"); ::_tcprintf(_T("%s"), msg.c_str()); };
	error = [&error_msg](const ablib::string &msg){error_msg += msg + _T("\n");};

	bool b = true;
	if(texmfcnf)b = (b && abnrSetting_funcs::writetexmfcnf(msg,detail,error));
	if(ipaex)b = (b && abnrSetting_funcs::kanji_config_updmap(msg,detail,error,_T("ipaex")));
	if(TeXworks)b = (b && abnrSetting_funcs::TeXworksSetting(msg,detail,error));
	if(tex2img)b = (b && TeX2img_funcs::Set(msg,detail,error));
	if(b){
		if(error_msg != _T(""))error_msg = _T("\n") + error_msg;
		::MessageBox(NULL,(_T("正常終了しました．") + error_msg).c_str(),_T("abtlinst_patch"),0);
		::FreeConsole();
		return 0;
	}else{
		::MessageBox(NULL,(_T("エラーが発生しました．\n") + error_msg).c_str(),_T("abtlinst_patch"),0);
		::FreeConsole();
		return -1;
	}
}


