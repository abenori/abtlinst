#ifndef __PATH_H__
#define __PATH_H__

#include <vector>
#include <functional>

class Path
{
public:
	Path(){}
	~Path(){}
	void AddPath(const ablib::string &path,bool last = false){if(last)m_addpath.push_back(path);else m_addpath.insert(m_addpath.begin(),path);}
	void DeletePath(const ablib::string &path){m_deletepath.push_back(path);}
	// DeltePath‚µ‚½‚Ì‚ğÁ‚µ‚ÄCAddPath‚µ‚½‚Ì‚ğÅŒã‚É“ü‚ê‚éD
	bool SetPath();
	static bool GetPath(std::vector<ablib::string> &path);
	template <class InputIterator, class OutputIterator, class BinaryPredicate> static OutputIterator unique_copy_nonsort(InputIterator first, InputIterator last,OutputIterator result, BinaryPredicate pred);

private:
	std::vector<ablib::string> m_addpath,m_deletepath;
};

#endif //__PATH_H__

