#include "stdafx.h"
#include "nkf.h"
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "Setting.h"
#include "Applications.h"

extern Setting g_Setting;
extern Applications g_Applications;


void nkf::ReadSetting(){
	m_filedir = g_Setting.ToAbsolutePath(g_Setting.ReadSysINI(_T("nkf"),_T("FileDir")));
	if(m_filedir == _T(""))m_filedir = g_Setting.ToAbsolutePath(g_Setting.ReadSysINI(_T("abtlinst"),_T("DownloadDir")));
	if(m_filedir != _T("") && m_filedir[m_filedir.length() - 1] != '\\')m_filedir += _T("\\");

	m_filereg = g_Setting.ReadSysINI(_T("nkf"),_T("Reg"));

	if(g_Setting.ReadINI(_T("nkf"),_T("install")) != _T("0"))m_doinst = true;
	else m_doinst = false;
#ifndef INSTALL_FROM_LOCAL
	m_url = g_Setting.ReadINI(_T("nkf"),_T("URL"));
#endif
}

void nkf::WriteSetting(){
	g_Setting.WriteINI(_T("nkf"),_T("install"),m_doinst ? _T("1") : _T("0"));
}

bool nkf::Download(HWND hwnd,MSGFUNC &msgfunc){
	msgfunc.msg(_T("nkf �̃t�@�C����T��\n"));
	std::wregex reg;
	try{reg.assign(m_filereg,std::regex_constants::icase);}
	catch(...){
		msgfunc.detail(_T("nkf �̔����Ɏ��s\n"));
		g_Setting.Log(_T("nkf �𔭌����鐳�K�\�����s���ł��D"));
		return false;
	}
	boost::filesystem::directory_iterator end;
	try{
		for(boost::filesystem::directory_iterator ite(m_filedir) ; ite != end ; ++ite){
			boost::filesystem::path p(*ite);
			if(std::regex_match(p.filename().wstring(),reg)){
				m_file = p.filename().wstring();
				break;
			}
		}
	}catch(...){
		g_Setting.Log(_T("nkf ��ۑ����Ă���t�H���_��������܂���ł����D"));
		return false;
	}
	if(m_file == _T("")){
		msgfunc.detail(_T("nkf �̔����Ɏ��s\n"));
		g_Setting.Log(_T("nkf ��������܂���ł����D"));
		return false;
	}else return true;
}

bool nkf::Install(HWND hwnd,MSGFUNC &msgfunc){
	if(!m_doinst){
		g_Setting.Log(_T("nkf ���C���X�g�[�����܂���D"));
		return true;
	}else g_Setting.Log(_T("nkf �̃C���X�g�[�������s�D"));
	ablib::string dir = g_Applications.m_TeXLive.GetVar(_T("TEXDIR"));
	if(dir == _T("")){
		g_Setting.Log(_T("nkf �̃C���X�g�[�����FTEXDIR �̎擾�Ɏ��s���܂����D"));
		return false;
	}
	if(dir != _T("") && dir[dir.length() - 1] != '\\')dir += _T("\\");
	dir += _T("bin\\win32\\");
	::CreateDirectoryReflex(dir.c_str());
	msgfunc.msg(_T("nkf �̃C���X�g�[��\n"));
	if(::CopyFile((m_filedir + m_file).c_str(),(dir + _T("nkf.exe")).c_str(),TRUE) == FALSE){
		g_Setting.Log(_T("nkf �̃R�s�[�Ɏ��s�DGetLastError = ") + boost::lexical_cast<ablib::string>(::GetLastError()));
		msgfunc.detail(_T("nkf �̃C���X�g�[���Ɏ��s�D\n"));
		return false;
	}else{
		msgfunc.detail(_T("nkf ���C���X�g�[�����܂����D\n"));
		return true;
	}
}

