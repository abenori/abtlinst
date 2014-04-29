#pragma once
#include <atlwin.h>
#include "resource.h"

class DlgTLAdvSet :
	public CDialogImpl<DlgTLAdvSet>
{
public:
	DlgTLAdvSet(void){}
	~DlgTLAdvSet(void){}

	enum {IDD = IDD_TLADVSET};

	BEGIN_MSG_MAP_EX(DlgTLAdvSet)
        MSG_WM_INITDIALOG(OnInitDialog);
        COMMAND_ID_HANDLER_EX(IDOK, OnOK);
        COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel);
		COMMAND_ID_HANDLER_EX(IDC_SANSYO_TEXMFLOCAL,OnSansyoTEXMFLOCAL);
		COMMAND_ID_HANDLER_EX(IDC_SANSYO_TEXMFSYSVAR,OnSansyoTEXMFSYSVAR);
		COMMAND_ID_HANDLER_EX(IDC_SANSYO_TEXMFSYSCONFIG,OnSansyoTEXMFSYSCONFIG);
		COMMAND_ID_HANDLER_EX(IDC_SANSYO_TEXMFHOME,OnSansyoTEXMFHOME);
		COMMAND_ID_HANDLER_EX(IDC_CHKDIRINDEP,OnChkDirIndep);
    END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnOK(UINT uNotifyCode, int nID, HWND hWndCtl);
	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl);
	void OnSansyoTEXMFLOCAL(UINT uNotifyCode, int nID, HWND hWndCtl){OnSansyo(IDC_TEXMFLOCAL);}
	void OnSansyoTEXMFSYSVAR(UINT uNotifyCode, int nID, HWND hWndCtl){OnSansyo(IDC_TEXMFSYSVAR);}
	void OnSansyoTEXMFSYSCONFIG(UINT uNotifyCode, int nID, HWND hWndCtl){OnSansyo(IDC_TEXMFSYSCONFIG);}
	void OnSansyoTEXMFHOME(UINT uNotifyCode, int nID, HWND hWndCtl){OnSansyo(IDC_TEXMFHOME);}
	void OnChkDirIndep(UINT uNotifyCode, int nID, HWND hWndCtl){SetEnabled();}


private:
	void SetEnabled();
	void OnSansyo(int editid);
	void SetButtons();

};

