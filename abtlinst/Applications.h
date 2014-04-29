#pragma once
#include "TeXLive.h"
#include "abnrSetting.h"
#include "TeX2img.h"
#include "ImageMagick.h"
#include "nkf.h"

class Applications
{
public:
	Applications(void){}
	~Applications(void){}

	void Abort();
	void ReadSetting();
	void WriteSetting();

	TeXLive m_TeXLive;
	abnrSetting m_abnrSetting;
	TeX2img m_TeX2img;
	ImageMagick m_ImageMagick;
	nkf m_nkf;
};

