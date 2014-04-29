#pragma warning(disable:4996)
#include "stdafx.h"
#pragma warning(default:4996)
#include "Path.h"
#include <system/System.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "Setting.h"
#include "func.h"
#include "Registory.h"

extern Setting g_Setting;


const TCHAR RegPath[] = _T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
bool Path::GetPath(std::vector<ablib::string> &path){
	// ����PATH�擾�D
	ablib::string p;
	if(!Registory::ReadSZ(HKEY_LOCAL_MACHINE,RegPath,_T("PATH"),p))return false;
	boost::algorithm::split(path,p,boost::algorithm::is_any_of(_T(";")));
	auto ite = std::remove(path.begin(),path.end(),_T(""));
	path.erase(ite,path.end());
	return true;
}

template <class InputIterator, class OutputIterator, class BinaryPredicate> OutputIterator Path::unique_copy_nonsort(InputIterator first, InputIterator last,OutputIterator result, BinaryPredicate pred){
	OutputIterator result_first = result;
	for(InputIterator i = first ; i != last ; ++i){
		if(std::find_if(result_first,result,std::bind(pred,*i,std::placeholders::_1)) == result){
			*result = *i;
			++result;
		}
	}
	return result;
}

bool Path::SetPath(){
	//	m_msg += _T("���ϐ��̒ǉ��c�c");
	//	WriteMsg(m_msg);
	//	g_Setting.Log(_T("���ϐ�PATH�̒ǉ��J�n"));
	bool rv = true;
	
	std::vector<ablib::string> addpath,delpath;
	// ���Ԃ��Ă镪������
	delpath.resize(m_deletepath.size());
	auto ite = unique_copy_nonsort(m_deletepath.begin(),m_deletepath.end(),delpath.begin(),[](const ablib::string &i,const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;});
	delpath.erase(ite,delpath.end());

	addpath.resize(m_addpath.size());
	ite = unique_copy_nonsort(m_addpath.begin(),m_addpath.end(),addpath.begin(),[](const ablib::string &i,const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;});
	addpath.erase(ite,addpath.end());

	// �����ē���������Ə��Ԃ������̂ŁC���炩���ߓ���邱�Ƃ����܂��Ă����͏���������Ȃ����Ă����D
	// �u�����āv���Ԃ����킹��悤�ɂ����D
/*	ite = std::remove_if(delpath.begin(),delpath.end(),[&addpath](const ablib::string &i){
		return std::find_if(addpath.begin(),addpath.end(),[&i](const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;}) != addpath.end();
	});
	delpath.erase(ite,delpath.end());
*/
	std::vector<ablib::string> paths;
	if(!GetPath(paths)){
//		m_msg += _T("���s\r\n");WriteMsg(m_msg);
		rv = false;
		g_Setting.Log(_TR("���ϐ��ǉ����s"));
		return false;
	}
	
	bool change = false;
//	g_Setting.Log(_T("���݂�PATH������"));
//	g_Setting.Log(nowpath);
//	�폜����PATH�������čs���D
	ite = std::remove_if(paths.begin(),paths.end(),[&delpath,&change](const ablib::string &i){
		if(std::find_if(delpath.begin(),delpath.end(),[&i](const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;}) != delpath.end()){
			g_Setting.Log(ablib::string::sprintf(_TR("���ϐ�PATH����u%s�v���폜"),i.c_str()));
			change = true;
			return true;
		}else return false;
	});
	paths.erase(ite,paths.end());

	// �����ׂ�PATH��ǉ��D
	for(ablib::string &i : addpath){
		if(std::find_if(paths.begin(),paths.end(),[&i](const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;}) == paths.end()){
			change = true;
			paths.push_back(i);
			g_Setting.Log(ablib::string::sprintf(_TR("���ϐ�PATH�Ɂu%s�v��ǉ�"),i.c_str()));
		}
	}
	
	if(change){
		// PATH�����񐶐�
		ablib::string path;
		for(ablib::string &i : paths)path += i + _T(";");
		// ���ϐ���REG_EXPAND_SZ�^�Dmkasa����thanks�I
		if(!Registory::WriteSZ(HKEY_LOCAL_MACHINE,RegPath,_T("PATH"),path.c_str(),true)){
			g_Setting.Log(_TR("���W�X�g���ւ̏������ݎ��s"));
//			m_msg += _T("���s\r\n");WriteMsg(m_msg);
			rv = false;
			g_Setting.Log(_TR("���ϐ��̏��������Ɏ��s"));
		}
//		m_msg += _T("����\r\n");
//		WriteMsg(m_msg);
		DWORD ret;
		::SendMessageTimeout(HWND_BROADCAST,WM_SETTINGCHANGE,0,reinterpret_cast<LPARAM>(_T("Environment")),SMTO_ABORTIFHUNG,5000,&ret);
		::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
	}else{
		g_Setting.Log(_TR("���ϐ�PATH��������K�v�͂���܂���ł���"));
//		m_msg += _T("�s�v\r\n");
//		WriteMsg(m_msg);
	}
	
//	g_Setting.Log(_T("���ϐ�PATH�̒ǉ��I��"));
	return rv;
}

