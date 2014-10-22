// abtlinst_patch.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
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

	if(texmfcnf)str += _T("�Etexmf.cnf �̒ǋL\n");
	if(ipaex)str += _T("�E�f�t�H���g�̃t�H���g�� ipaex �ɕύX\n");
	if(TeXworks)str += _T("�ETeXworks �̐ݒ����{��I�ɂ���\n");
	if(tex2img)str += _T("�ETeX2img ��ݒ肷��\n");

	if(str == _T("")){
		::MessageBox(NULL,_T("�������s���܂���D"),_T("abtlinst_patch"),MB_OK);
		return 0;
	}
	str = _T("TeX Live �ւ̐ݒ���ēx�s���܂��D��̓I�ɂ�\n\n") + str + _T("\n���s���܂��D��낵���ł����H");

	int r = ::MessageBox(NULL,str.c_str(),_T("abtlinst_patch"),MB_YESNO);
	if(r == IDNO){
		::MessageBox(NULL,_T("�ݒ���s�킸�I�����܂��D"),_T("abtlinst_patch"),0);
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
		if(error_msg != _T(""))error_msg += _T("\n");
		::MessageBox(NULL,(_T("����I�����܂����D") + error_msg).c_str(),_T("abtlinst_patch"),0);
		::FreeConsole();
		return 0;
	}else{
		::MessageBox(NULL,(_T("�G���[���������܂����D\n") + error_msg).c_str(),_T("abtlinst_patch"),0);
		::FreeConsole();
		return -1;
	}
}


