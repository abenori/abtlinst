#include "stdafx.h"
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <system/Process.h>
#include "Func.h"
#include "TeX2img.h"
#include "TeX2img_func.h"
#include "Setting.h"
#include "Applications.h"

extern Setting g_Setting;
extern Applications g_Applications;

void TeX2img::ReadSetting(){
	m_filedir = g_Setting.ToAbsolutePath(g_Setting.ReadSysINI(_T("TeX2img"),_T("FileDir")));
	if(m_filedir == _T(""))m_filedir = g_Setting.ToAbsolutePath(g_Setting.ReadSysINI(_T("abtlinst"),_T("DownloadDir")));
	if(m_filedir != _T("") && m_filedir[m_filedir.length() - 1] != '\\')m_filedir += _T("\\");

	m_filereg = g_Setting.ReadSysINI(_T("TeX2img"),_T("Reg"));

	if(g_Setting.ReadINI(_T("TeX2img"),_T("install")) != _T("0"))m_doinst = true;
	else m_doinst = false;
#ifndef INSTALL_FROM_LOCAL
	m_url = g_Setting.ReadINI(_T("TeX2img"),_T("URL"));
#endif
}

void TeX2img::WriteSetting(){
	g_Setting.WriteINI(_T("TeX2img"),_T("install"),m_doinst ? _T("1") : _T("0"));
}

bool TeX2img::Download(HWND hwnd,MSGFUNC &msgfunc){
	msgfunc.msg(_T("TeX2img �̃t�@�C����T��\n"));
	std::wregex reg;
	try{reg.assign(m_filereg,std::regex_constants::icase);}
	catch(...){
		msgfunc.detail(_T("TeX2img �̔����Ɏ��s\n"));
		g_Setting.Log(_T("TeX2img �𔭌����鐳�K�\�����s���ł��D"));
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
	}
	catch(...){
		g_Setting.Log(_T("TeX2img ��ۑ����Ă���t�H���_��������܂���ł����D"));
		return false;
	}
	if(m_file == _T("")){
		msgfunc.detail(_T("TeX2img �̔����Ɏ��s\n"));
		g_Setting.Log(_T("TeX2img ��������܂���ł����D"));
		return false;
	}else return true;
}

bool TeX2img::Install(HWND hwnd,MSGFUNC &msgfunc){
	if(!m_doinst){
		g_Setting.Log(_T("TeX2img ���C���X�g�[�����܂���D"));
		return true;
	}else g_Setting.Log(_T("TeX2img �̃C���X�g�[�������s�D"));
	ablib::string dir = g_Applications.m_TeXLive.GetVar(_T("TEXDIR"));
	if(dir == _T("")){
		g_Setting.Log(_T("TeX2img �̃C���X�g�[�����FTEXDIR �̎擾�Ɏ��s���܂����D"));
		return false;
	}
	if(dir != _T("") && dir[dir.length() - 1] != '\\')dir += _T("\\");
	dir += _T("bin\\win32\\");
	::CreateDirectoryReflex(dir.c_str());
	msgfunc.msg(_T("TeX2img �̃C���X�g�[��\n"));
	if (
		!::CopyFile((m_filedir + m_file).c_str(), (dir + _T("TeX2img.exe")).c_str(), FALSE) ||
		!::CopyFile((m_filedir + m_file + _T(".config")).c_str(), (dir + _T("TeX2img.exe.config")).c_str(), FALSE) ||
		!::CopyFile((m_filedir + _T("Azuki.dll")).c_str(), (dir + _T("Azuki.dll")).c_str(), FALSE))
	{
		g_Setting.Log(_T("TeX2img�̃C���X�g�[���F�t�@�C���̃R�s�[�Ɏ��s�D"));
	}
	::CopyFile((m_filedir + _T("tex2imgc.exe")).c_str(), (dir + _T("TeX2imgc.exe")).c_str(), FALSE);
	::CopyFile((m_filedir + _T("TeX2imgc.exe.config")).c_str(), (dir + _T("TeX2imgc.exe.config")).c_str(), FALSE);
	//::CopyDirectory(m_filedir + _T("pstoedit"),dir + _T("pstoedit"));
	msgfunc.detail(_T("TeX2img ���C���X�g�[�����܂����D\n"));
	return TeX2img_funcs::Set(
		g_Applications.m_TeXLive.GetVar(_T("TEXDIR")) + _T("\\"),
		msgfunc.msg,msgfunc.detail,
		std::bind(&Setting::Log,&g_Setting,std::placeholders::_1));

}

