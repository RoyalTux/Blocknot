#pragma once
#include "header.h"

class CMenuBarDlg
{
public:
	CMenuBarDlg(void);
public:
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	static CMenuBarDlg* ptr;
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnClose(HWND hwnd);
	void Cls_OnSize(HWND hwnd, UINT state, int cx, int cy);
	void Cls_OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu);
	void Cls_OnMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags);
	HWND hDialog, hStatus, hEdit;
	HFONT hFont = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Courier New"));
	BOOL bShowStatusBar;
	void MemAlloc(int size);
	void MemFree();
	void NewDoc();
	void OpenDoc();
	void SaveDocS();
	void SaveAsDoc();
	void SaveDoc();
	void SetFont();
	void exit();
	//
	void MessageFromFindReplace();
	void OnFind();
	void OnReplace();
	TCHAR bufFind[100], alltext[65536], bufReplace[100];
	HWND hFR;
	FINDREPLACE fr;
};
