#include "stdafx.h"
#include <boost/lexical_cast.hpp>
#include <regex>
#include "Network.h"
#include "ftpaccess.h"
#include "httpaccess.h"

#ifndef INSTALL_FROM_LOCAL

bool Network::GetFileData(std::vector<Network::FileData> &rv,const ablib::string &url,MSGFUNC &msgfunc){
	bool http;
	ablib::string s,p;
	if(url.substr(0,7) == _T("http://")){
		http = true;
		s = url.substr(7);
	}else{
		http = false;
		s = url.substr(6);
	}
	ablib::string::size_type r = s.find(_T("/"));
	if(r != ablib::string::npos){
		p = s.substr(r + 1);
		s.erase(r);
	}
	if(http)return GetFileDataHTTP(rv,s,p,msgfunc);
	else return GetFileDataFTP(rv,s,p,msgfunc);
}

bool Network::GetFileDataFTP(std::vector<Network::FileData> &rv,const ablib::string &server,const ablib::string &path,MSGFUNC &msgfunc){
	FtpAccess ftp(server.c_str(),_T("anonymous"),_T("a@b"));
	msgfunc.detail(ablib::string::sprintf(_TR("%s�ɐڑ�\n"),server.c_str()));
	if(!ftp.Connect()){
		msgfunc.detail(_TR("���s"));
		return false;
	}
	if(!ftp.cd(path.c_str())){
		msgfunc.detail(_TR("���s"));
		return false;
	}
	std::vector<FtpAccess::FileInfo> fis;
	if(!ftp.ls(_T("*.*"),fis)){
		msgfunc.detail(_TR("���s"));
		return false;
	}
	ftp.Close();
	FileData fd;
	for(auto &i : fis){
		fd.file = i.name;fd.size = i.size;fd.time = i.time;
		rv.push_back(fd);
	}
	return true;
}

bool Network::GetFileDataHTTP(std::vector<Network::FileData> &rv,const ablib::string &server,const ablib::string &path,MSGFUNC &msgfunc){
	ablib::string proxy;
	if(m_proxy.proxy != _T("") && m_proxy.port != 0){
		proxy = m_proxy.proxy;
		proxy += _T(":") + boost::lexical_cast<ablib::string>(m_proxy.port);
	}
	HttpAccess http(server.c_str(),proxy.c_str());
	msgfunc.detail(ablib::string::sprintf(_TR("%s�ɐڑ�\n"),server.c_str()));
	if(!http.Connect()){
		msgfunc.detail(_TR("���s\n"));
		return false;
	}
	// �f�B���N�g���ꗗ���擾�D
	std::vector<BYTE> data;
	if(!http.SendGET(path.c_str())){
//		msgfunc(_TR("GET���N�G�X�g�̑��M�Ɏ��s"));
		msgfunc.detail(_TR("���s\n"));
		return false;
	}

	data.reserve(http.content_length());
	if(!http.data(data)){
//		msgfunc(_TR("�f�[�^�̓ǂݍ��݂Ɏ��s"));
		msgfunc.detail(_TR("���s\n"));
		return false;
	}

	int a = http.status_code();
	data.push_back('\0');
	http.Close();
	ablib::string str = ablib::TransCode::FromChar(reinterpret_cast<char*>(&data[0]));
//	::OutputDebugString(str.c_str());
	
	// 1: �t�@�C�����C2: ���C3: ���C4: �N�C5: ���C6: ���C7: �T�C�Y
	std::wregex reg1,reg2;
	try{
		reg1.assign(_T("href[ \\t]*=[ \\t]*\"([^\\r\\n\"]+)\"[^\\n\\r]*?([0-9]*)-([A-Za-z]*)-([0-9]*)[ \\t]*([0-9]*):([0-9]*)[^\\n\\r]*?([0-9\\.]*[KMB])"));
		reg2.assign(_T("href[ \\t]*=[ \\t]*\"([^\\r\\n\"]+)\"[^\\n\\r]*?([0-9]*)-([0-9]*)-([0-9]*)[ \\t]*([0-9]*):([0-9]*)[^\\n\\r]*?([0-9\\.]*[KMB])"));
	}catch(...){ASSERT(false);}
//	DEBUGSTRING(_T("%s"), reg1.Match(_T("href=\"context.tar.bz2\">context.tar.bz2</a>            15-Aug-2009 10:07   17M  ")) ? _T("true") : _T("false"));
//	DEBUGSTRING(_T("%s"), reg2.Match("href="context.tar.xz">context.tar.xz</a>            2013-02-26 15:20   10M") ? "true" : "false");
	ablib::string::size_type p = 0;
	std::wsmatch reg1_res,reg2_res;
	while(true){
		bool reg1matched = std::regex_search(str,reg1_res,reg1);
		bool reg2matched = std::regex_search(str,reg2_res,reg1);
		if(!reg1matched && !reg2matched)break;
		FileData fd;
		ablib::string day,year,hour,min;
		DWORD month;
		if(reg1matched && reg1_res.size() >= 7){
			fd.file = reg1_res.str(1);
			fd.size = FromStrSize2IntSize(reg1_res.str(7));
			day = reg1_res.str(2);
			year = reg1_res.str(4);
			hour = reg1_res.str(5);
			min = reg1_res.str(6);
			month = FromStrMonth2IntMonth(reg1_res.str(3));
			str = reg1_res.suffix().str();
		}else if(reg2matched && reg2_res.size() >= 7){
			fd.file = reg2_res.str(1);
			fd.size = FromStrSize2IntSize(reg2_res.str(7));
			day = reg2_res.str(4);
			year = reg2_res.str(2);
			hour = reg2_res.str(5);
			min = reg2_res.str(6);
			month = boost::lexical_cast<int>(reg2_res.str(3));
			str = reg2_res.suffix().str();
		}else{
			if(reg1matched)str = reg1_res.suffix().str();
			else str = reg2_res.suffix().str();
			continue;
		}

		ablib::system::Time time(
			boost::lexical_cast<int>(year),
			month,
			boost::lexical_cast<int>(day),
			boost::lexical_cast<int>(hour),
			boost::lexical_cast<int>(min));
		fd.time = time;
//		DEBUGSTRING(_T("�t�@�C�����X�g�쐬�Ffile = %s, �T�C�Y = %d, %d�N%d��%d��%d��%d��"),fd.file.c_str(),fd.size,fd.time.year(),fd.time.month(),fd.time.day(),fd.time.hour(),fd.time.minute());
		rv.push_back(fd);
	}
	return true;
}

DWORD Network::FromStrMonth2IntMonth(const ablib::string &month){
	ablib::string m = month;m.tolower();
	if(m == _T("jan"))return 1;
	else if(m == _T("feb"))return 2;
	else if(m == _T("mar"))return 3;
	else if(m == _T("apr"))return 4;
	else if(m == _T("may"))return 5;
	else if(m == _T("jun"))return 6;
	else if(m == _T("jul"))return 7;
	else if(m == _T("aug"))return 8;
	else if(m == _T("sep"))return 9;
	else if(m == _T("oct"))return 10;
	else if(m == _T("nov"))return 11;
	else if(m == _T("dec"))return 12;
	else return -1;
}

int Network::FromStrSize2IntSize(const ablib::string &s){
	if(s == _T(""))return 0;
	TCHAR c = s[s.length() - 1];
	double size;
	if(!(L'0' <= c && c <= L'9'))size = ::_tstof(s.c_str());
	else size = ::_tstof(s.substr(0,s.length() - 1).c_str());
	switch(c){
	case L'M':
		size = size*1024*1024;
		break;
	case L'K':
		size = size*1024;
		break;
	default:
		break;
	}
	return static_cast<int>(size);
}

#endif // INSTALL_FROM_LOCAL