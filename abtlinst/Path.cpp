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
	// 今のPATH取得．
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
	//	m_msg += _T("環境変数の追加……");
	//	WriteMsg(m_msg);
	//	g_Setting.Log(_T("環境変数PATHの追加開始"));
	bool rv = true;
	
	std::vector<ablib::string> addpath,delpath;
	// だぶってる分を消去
	delpath.resize(m_deletepath.size());
	auto ite = unique_copy_nonsort(m_deletepath.begin(),m_deletepath.end(),delpath.begin(),[](const ablib::string &i,const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;});
	delpath.erase(ite,delpath.end());

	addpath.resize(m_addpath.size());
	ite = unique_copy_nonsort(m_addpath.begin(),m_addpath.end(),addpath.begin(),[](const ablib::string &i,const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;});
	addpath.erase(ite,addpath.end());

	// 消して入れるをすると順番が狂うので，あらかじめ入れることが決まっているやつは消す方からなくしておく．
	// 「あえて」順番を狂わせるようにした．
/*	ite = std::remove_if(delpath.begin(),delpath.end(),[&addpath](const ablib::string &i){
		return std::find_if(addpath.begin(),addpath.end(),[&i](const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;}) != addpath.end();
	});
	delpath.erase(ite,delpath.end());
*/
	std::vector<ablib::string> paths;
	if(!GetPath(paths)){
//		m_msg += _T("失敗\r\n");WriteMsg(m_msg);
		rv = false;
		g_Setting.Log(_TR("環境変数追加失敗"));
		return false;
	}
	
	bool change = false;
//	g_Setting.Log(_T("現在のPATH文字列"));
//	g_Setting.Log(nowpath);
//	削除するPATHを消して行く．
	ite = std::remove_if(paths.begin(),paths.end(),[&delpath,&change](const ablib::string &i){
		if(std::find_if(delpath.begin(),delpath.end(),[&i](const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;}) != delpath.end()){
			g_Setting.Log(ablib::string::sprintf(_TR("環境変数PATHから「%s」を削除"),i.c_str()));
			change = true;
			return true;
		}else return false;
	});
	paths.erase(ite,paths.end());

	// 入れるべきPATHを追加．
	for(ablib::string &i : addpath){
		if(std::find_if(paths.begin(),paths.end(),[&i](const ablib::string &j){return CheckSameFile(i.c_str(),j.c_str()) ? true : false;}) == paths.end()){
			change = true;
			paths.push_back(i);
			g_Setting.Log(ablib::string::sprintf(_TR("環境変数PATHに「%s」を追加"),i.c_str()));
		}
	}
	
	if(change){
		// PATH文字列生成
		ablib::string path;
		for(ablib::string &i : paths)path += i + _T(";");
		// 環境変数はREG_EXPAND_SZ型．mkasaさんthanks！
		if(!Registory::WriteSZ(HKEY_LOCAL_MACHINE,RegPath,_T("PATH"),path.c_str(),true)){
			g_Setting.Log(_TR("レジストリへの書き込み失敗"));
//			m_msg += _T("失敗\r\n");WriteMsg(m_msg);
			rv = false;
			g_Setting.Log(_TR("環境変数の書き換えに失敗"));
		}
//		m_msg += _T("成功\r\n");
//		WriteMsg(m_msg);
		DWORD ret;
		::SendMessageTimeout(HWND_BROADCAST,WM_SETTINGCHANGE,0,reinterpret_cast<LPARAM>(_T("Environment")),SMTO_ABORTIFHUNG,5000,&ret);
		::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
	}else{
		g_Setting.Log(_TR("環境変数PATHをいじる必要はありませんでした"));
//		m_msg += _T("不要\r\n");
//		WriteMsg(m_msg);
	}
	
//	g_Setting.Log(_T("環境変数PATHの追加終了"));
	return rv;
}

