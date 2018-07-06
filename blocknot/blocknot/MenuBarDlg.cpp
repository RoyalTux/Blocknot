#include "MenuBarDlg.h"
#pragma warning(disable:4996)
OPENFILENAME ofn;
// ������������� ������������������� ��������� FINDMSGSTRING
UINT WM_FR = RegisterWindowMessage(FINDMSGSTRING);
//
int SizeDoc, MesRet;
//
char *BuferText; // max 9,53 �� (10 000 000 ����)
char Opffilt[256] = "��������� ����� *.txt\0*.txt\0��� ����� *.*\0*.*\0";
char Sffilt[256] = "*.*\0*.*\0";
char Opffile[256];
char Opffiletitle[256];
char Path[512];
char TitleWnd[1024];

//TCHAR *BuferText; // max 9,53 �� (10 000 000 ����)
//TCHAR Opffilt[256] = "��������� ����� *.txt\0*.txt\0��� ����� *.*\0*.*\0";
//TCHAR Sffilt[256] = "*.*\0*.*\0";
//TCHAR Opffile[256];
//TCHAR Opffiletitle[256];
//TCHAR Path[512];
//TCHAR TitleWnd[1024];
HANDLE hHeap;
HANDLE hFile;
DWORD nBytesRead;
LPWSTR currentFile = L"";
DWORD g_BytesTransferred = 0;

CMenuBarDlg* CMenuBarDlg::ptr = NULL;

HMENU hMenu, hMenuEn;

CMenuBarDlg::CMenuBarDlg(void)
{
	ptr = this;
	bShowStatusBar = TRUE;
	hFR = NULL;
	ZeroMemory(&fr, sizeof(fr));
}

void CMenuBarDlg::Cls_OnClose(HWND hwnd)
{
	exit();
}

BOOL CMenuBarDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	hDialog = hwnd;
	// ������� ���������� ���������� ����
	hEdit = GetDlgItem(hDialog, IDC_EDIT1);
	// �������� ������ ���������
	hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_TOOLTIPS | SBARS_SIZEGRIP, 0, hDialog, WM_USER);
	// �������� ���� �� �������� ����������
	hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU1));
	hMenuEn = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU2));
	//hFont = GetDlgItem(hDialog, ID_FRONT);
	// ����������� ���� � �������� ���� ����������
	SetMenu(hDialog, hMenu);
	return TRUE;
}

// ���������� ��������� WM_COMMAND ����� ������ ��� ������ ������ ����
void CMenuBarDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case ID_FIND:
		OnFind();
		break;
	case ID_REPLACE:
		OnReplace();
		break;
	case ID_CREATE:
		NewDoc();
	break;
	case ID_OPEN:
		OpenDoc();
		break;
	case ID_SAVE:
		SaveDoc();
		break;
	case ID_SAVE_AS:
		SaveAsDoc();
		break;
	case ID_FRONT:
		SetFont();
		break;
	case ID_ABOUT:
		MessageBox(ptr->hDialog, "������� v1.0\n����������� �������\n��� �2", "�������", MB_OK | MB_ICONINFORMATION);
		break;
	case ID_HELP:
		MessageBox(ptr->hDialog, "�� ����, ������� �� ���� �� �������...", "�������", MB_OK | MB_ICONINFORMATION);
		break;
	case ID_EXIT:
		exit();
		break;
	case ID_EN:
		SetMenu(hDialog, hMenuEn);
		SetWindowText(hwnd, "Notes");
		break;
	case ID_RUS:
		SetMenu(hDialog, hMenu);
		SetWindowText(hwnd, "�������");
		break;
	case ID_CANCEL:
		// ������� ��������� ��������
		SendMessage(hEdit, WM_UNDO, 0, 0);
		break;
	case ID_CUT:
		// ������ ���������� �������� ������ � ����� ������
		SendMessage(hEdit, WM_CUT, 0, 0);
		break;
	case ID_COPY:
		// ��������� ���������� �������� ������ � ����� ������
		SendMessage(hEdit, WM_COPY, 0, 0);
		break;
	case ID_PASTE:
		// ������� ����� � Edit Control �� ������ ������
		SendMessage(hEdit, WM_PASTE, 0, 0);
		break;
	case ID_DEL:
		// ������ ���������� �������� ������
		SendMessage(hEdit, WM_CLEAR, 0, 0);
		break;
	case ID_SELECTALL:
		// ������� ���� ����� � Edit Control
		SendMessage(hEdit, EM_SETSEL, 0, -1);
		break;
	case ID_STATUS_BAR:
		// ���� ���� ����� TRUE, �� ������ ��������� ����������
		if (bShowStatusBar)
		{
			// ������� ���������� �������� ����
			HMENU hMenu = GetMenu(hDialog);
			// ������ ������� � ������ ���� "������ ���������"
			CheckMenuItem(hMenu, ID_STATUS_BAR, MF_BYCOMMAND | MF_UNCHECKED);
			// ������ ������ ���������
			ShowWindow(hStatus, SW_HIDE);
		}
		else
		{
			// ������� ���������� �������� ����
			HMENU hMenu = GetMenu(hDialog);
			// ��������� ������� �� ������ ���� "������ ���������"
			CheckMenuItem(hMenu, ID_STATUS_BAR, MF_BYCOMMAND | MF_CHECKED);
			// ��������� ������ ���������
			ShowWindow(hStatus, SW_SHOW);
		}
		bShowStatusBar = !bShowStatusBar;
	}
}

// ���������� ��������� WM_SIZE ����� ������ ��� ��������� �������� �������� ����
// ���� ��� ������������/�������������� �������� ����
void CMenuBarDlg::Cls_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	RECT rect1, rect2;
	// ������� ���������� ���������� ������� �������� ����
	GetClientRect(hDialog, &rect1);
	// ������� ���������� ������������ ������ ������ ���������
	SendMessage(hStatus, SB_GETRECT, 0, (LPARAM)&rect2);
	// ��������� ����� ������� ���������� ����
	MoveWindow(hEdit, rect1.left, rect1.top, rect1.right, rect1.bottom - (rect2.bottom - rect2.top), 1);
	// ��������� ������ ������ ���������, 
	// ������ ������ ���������� ������� �������� ����
	SendMessage(hStatus, WM_SIZE, 0, 0);
}

// ���������� WM_INITMENUPOPUP ����� ������ ��������������� 
// ����� ������������ ������������ ����
void CMenuBarDlg::Cls_OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu)
{
	if (item == 0) // �������������� ����� ���� "������"
	{
		// ������� ������� ��������� ������
		DWORD dwPosition = SendMessage(hEdit, EM_GETSEL, 0, 0);
		WORD wBeginPosition = LOWORD(dwPosition);
		WORD wEndPosition = HIWORD(dwPosition);

		if (wEndPosition != wBeginPosition) // ������� �� �����?
		{
			// ���� ������� ���������� �����, 
			// �� ������� ������������ ������ ���� "����������", "��������" � "�������"
			EnableMenuItem(hMenu, ID_COPY, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, ID_CUT, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, ID_DEL, MF_BYCOMMAND | MF_ENABLED);
		}
		else
		{
			// ���� ����������� ���������� �����, 
			// �� ������� ������������ ������ ���� "����������", "��������" � "�������"
			EnableMenuItem(hMenu, ID_COPY, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, ID_CUT, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, ID_DEL, MF_BYCOMMAND | MF_GRAYED);
		}

		if (IsClipboardFormatAvailable(CF_TEXT)) // ������� �� ����� � ������ ������?
			// ���� ������� ����� � ������ ������, 
			// �� ������� ����������� ����� ���� "��������"
			EnableMenuItem(hMenu, ID_PASTE, MF_BYCOMMAND | MF_ENABLED);
		else
			// ���� ����������� ����� � ������ ������, 
			// �� ������� ����������� ����� ���� "��������"
			EnableMenuItem(hMenu, ID_PASTE, MF_BYCOMMAND | MF_GRAYED);

		// ���������� �� ����������� ������ ���������� ��������?
		if (SendMessage(hEdit, EM_CANUNDO, 0, 0))
			// ���� ���������� ����������� ������ ���������� ��������,
			// �� ������� ����������� ����� ���� "��������"
			EnableMenuItem(hMenu, ID_UNDO, MF_BYCOMMAND | MF_ENABLED);
		else
			// ���� ����������� ����������� ������ ���������� ��������,
			// �� ������� ����������� ����� ���� "��������"
			EnableMenuItem(hMenu, ID_UNDO, MF_BYCOMMAND | MF_GRAYED);

		// ��������� ����� ������ � Edit Control
		int length = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
		// ������� �� ���� ����� � Edit Control?
		if (length != wEndPosition - wBeginPosition)
			//���� �� ���� ����� ������� � Edit Control,
			// �� ������� ����������� ����� ���� "�������� ��"
			EnableMenuItem(hMenu, ID_SELECTALL, MF_BYCOMMAND | MF_ENABLED);
		else
			// ���� ������� ���� ����� � Edit Control,
			// �� ������� ����������� ����� ���� "�������� ��"
			EnableMenuItem(hMenu, ID_SELECTALL, MF_BYCOMMAND | MF_GRAYED);
	}
}

void CMenuBarDlg::Cls_OnMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags)
{
	if (flags & MF_POPUP) // ��������, �������� �� ���������� ����� ���� ���������� ����������� �������?
	{
		// ���������� ����� ���� �������� ���������� ����������� �������
		SendMessage(hStatus, SB_SETTEXT, 0, 0); // ������� ����� �� ������ ���������
	}
	else
	{
		// ���������� ����� ���� �������� �������� ������� (����� ���� "�������")
		TCHAR buf[200];
		// ������� ���������� �������� ���������� ����������
		HINSTANCE hInstance = GetModuleHandle(NULL);
		// ������� ������ �� ������� �����, ������������� � �������� ����������
		// ��� ���� ������������� ����������� ������ ������ ������������� �������������� ����������� ������ ����
		LoadString(hInstance, item, buf, 200);
		// ������� � ������ ��������� ����������� �������, ��������������� ����������� ������ ����
		SendMessage(hStatus, SB_SETTEXT, 0, LPARAM(buf));
	}
}

BOOL CALLBACK CMenuBarDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
		HANDLE_MSG(hwnd, WM_SIZE, ptr->Cls_OnSize);
		HANDLE_MSG(hwnd, WM_INITMENUPOPUP, ptr->Cls_OnInitMenuPopup);
		HANDLE_MSG(hwnd, WM_MENUSELECT, ptr->Cls_OnMenuSelect);
	}
	// ��������� ���������, ���������� �� ������������ ����������� ����
	if (message == WM_FR)
		ptr->MessageFromFindReplace();
	return FALSE;
}

void CMenuBarDlg::MemAlloc(int size)
{
	BuferText = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
	if (BuferText == 0)
	{
		MessageBox(ptr->hDialog, "������ � ����", "", MB_ICONERROR);
		ExitProcess(0);
	}
}

void CMenuBarDlg::MemFree()
{
	HeapFree(GetProcessHeap(), 0, BuferText);
}

void CMenuBarDlg::NewDoc()
{
	RtlZeroMemory(Path, sizeof(Path));
	SetWindowText(hEdit, NULL);
	SendMessage(hEdit, EM_SETMODIFY, 0, 0);
	SendMessage(hEdit, EM_EMPTYUNDOBUFFER, 0, 0);
	SetFocus(hEdit);
}

void CMenuBarDlg::OpenDoc()
{
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = ptr->hDialog;
	ofn.lpstrFilter = Opffilt;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = Opffile;
	ofn.nMaxFile = sizeof(Opffile);
	//ofn.lpstrFileTitle = Opffiletitle;
	//ofn.nMaxFileTitle = sizeof(Opffiletitle);
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetOpenFileName(&ofn))
	{
		hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, FILE_READ_DATA, NULL, OPEN_EXISTING, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(ptr->hDialog, "���������� ������� ����", "������", MB_ICONERROR);
			return;
		}
		SizeDoc = GetFileSize(hFile, NULL);
		SizeDoc++;
		if (SizeDoc>10000000)
		{
			CloseHandle(hFile);
			MessageBox(ptr->hDialog, "������ ������������ ����� ��������� ������", "������", MB_ICONERROR);
			return;
		}
		MemAlloc(SizeDoc);
		ReadFile(hFile, BuferText, SizeDoc, &nBytesRead, NULL);
		BuferText[nBytesRead] = 0x00;
		CloseHandle(hFile);
		SetWindowText(hEdit, BuferText);
		SetFocus(hEdit);
		MemFree();
	}
}

void CMenuBarDlg::SaveDoc()
{
	if (Path[0] == 0)
	{
		SaveAsDoc();
		return;
	}
	SaveDocS();
}

void CMenuBarDlg::SaveDocS()
{
	SizeDoc = GetWindowTextLength(hEdit);
	SizeDoc++;
	MemAlloc(SizeDoc);
	SizeDoc = GetWindowText(hEdit, BuferText, SizeDoc);
	hFile = CreateFile(Opffile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(ptr->hDialog, "���������� ��������� ����", "������", MB_ICONERROR);
		return;
	}
	WriteFile(hFile, BuferText, SizeDoc, &nBytesRead, NULL);
	CloseHandle(hFile);
	MemFree();
	SendMessage(hEdit, EM_SETMODIFY, 0, 0);
	SendMessage(hEdit, EM_EMPTYUNDOBUFFER, 0, 0);
}

void CMenuBarDlg::SaveAsDoc()
{
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = ptr->hDialog;
	ofn.lpstrFilter = Sffilt;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = Opffile;
	ofn.nMaxFile = sizeof(Opffile);
	//ofn.lpstrFileTitle = Opffiletitle;
	//ofn.nMaxFileTitle = sizeof(Opffiletitle);
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetSaveFileName(&ofn))
	{
		SaveDocS();
	}
}

void CMenuBarDlg::SetFont()
{
	CHOOSEFONT chfnt;
	LOGFONT lf;
	chfnt.lStructSize = sizeof(CHOOSEFONT);
	chfnt.hwndOwner = ptr->hDialog;
	chfnt.lpLogFont = &lf;
	chfnt.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	if (ChooseFont(&chfnt))
	{
		hFont = CreateFontIndirect(chfnt.lpLogFont);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

	}
}

void CMenuBarDlg::exit()
{
	if (SendMessage(hEdit, EM_GETMODIFY, 0, 0))
	{
		MesRet = MessageBox(ptr->hDialog, "����� ��� �������\n��������� ���������?", "", MB_YESNOCANCEL | MB_ICONINFORMATION);
		switch (MesRet)
		{
		case IDYES:
			SaveDoc();
			PostQuitMessage(0);
			break;
		case IDNO:
			PostQuitMessage(0);
			break;
		case IDCANCEL:
			return;
			break;
		}
	}
	DestroyWindow(ptr->hDialog);
	PostQuitMessage(0);
}

void CMenuBarDlg::OnFind()
{
	// ��������, ������� �� ���� ������
	if (hFR)
	{
		//������������ ���� ������
		SetForegroundWindow(hFR);
		return;
	}
	// �������� ��������� FINDREPLACE
	ZeroMemory(&fr, sizeof(fr));
	DWORD start, end;
	// ������� ���� �����, ����������� � ��������� ���� �����
	GetWindowText(hEdit, alltext, 65536);
	// ������� ������� ��������� ��������� ������
	SendMessage(hEdit, EM_GETSEL, WPARAM(&start), LPARAM(&end));
	// ��������� � ����� ���������� �������� ������
	_tcsncpy(bufFind, alltext + start, end - start);
	bufFind[end - start] = TEXT('\0');
	fr.lStructSize = sizeof(fr);
	// ������� ������ �������� �����-����������
	fr.hwndOwner = hDialog;
	// ��������� �� �����, ���������� ������ ��� ������
	fr.lpstrFindWhat = bufFind;
	fr.wFindWhatLen = 100;
	// ����� �� �������� ��������� ������� � ������ �� ����� ���������
	fr.Flags = FR_DOWN;
	// ���������� ������ �����
	hFR = FindText(&fr);
}

void CMenuBarDlg::OnReplace()
{
	// ��������, ������� �� ���� ������
	if (hFR)
	{
		// ������������ ���� ������
		SetForegroundWindow(hFR);
		return;
	}
	// �������� ��������� FINDREPLACE
	ZeroMemory(&fr, sizeof(fr));
	DWORD start, end;
	// �������� �����, ��������������� ��� �������� ���������� ������ 
	ZeroMemory(&bufReplace, sizeof(bufReplace));
	// ������� ���� �����, ����������� � ��������� ���� �����
	GetWindowText(hEdit, alltext, 65536);
	// ������� ������� ��������� ��������� ������
	SendMessage(hEdit, EM_GETSEL, WPARAM(&start), LPARAM(&end));
	// ��������� � ����� ���������� �������� ������
	_tcsncpy(bufFind, alltext + start, end - start);
	bufFind[end - start] = TEXT('\0');
	fr.lStructSize = sizeof(fr);
	// ������� ������ �������� �����-����������
	fr.hwndOwner = hDialog;
	// ��������� �� �����, ���������� ������ ��� ������
	fr.lpstrFindWhat = bufFind;
	fr.wFindWhatLen = 100;
	// ��������� �� �����, ���������� ������ ��� ������
	fr.lpstrReplaceWith = bufReplace;
	fr.wReplaceWithLen = 100;
	// ����� �� �������� ��������� ������� � ������ �� ����� ���������
	fr.Flags = FR_DOWN;
	// ���������� ������ ��������
	hFR = ReplaceText(&fr);
}

void CMenuBarDlg::MessageFromFindReplace()
{
	if (fr.Flags & FR_REPLACEALL)
		MessageBox(hDialog, TEXT("������ ������ \"�������� ��\""), TEXT("����� � ������"), MB_OK | MB_ICONINFORMATION);

	if (fr.Flags & FR_REPLACE)
	{
		MessageBox(hDialog, TEXT("������ ������ \"��������\""), TEXT("����� � ������"), MB_OK | MB_ICONINFORMATION);
		// �������� ���������� �������� ������ �� ������, ����������� � ������ bufReplace
		SendMessage(hEdit, EM_REPLACESEL, WPARAM(TRUE), (LPARAM)bufReplace);
	}

	if (fr.Flags & FR_FINDNEXT)
	{
		MessageBox(hDialog, TEXT("������ ������ \"����� �����\""), TEXT("����� � ������"), MB_OK | MB_ICONINFORMATION);
		DWORD Start, End;
		// ��������� ����� ������� ������
		TCHAR * p = _tcsstr(alltext, bufFind);
		if (p)
		{
			Start = p - alltext;
			End = Start + _tcslen(bufFind);
			// �������� ��������� ������
			SendMessage(hEdit, EM_SETSEL, Start, End);
		}
	}

	if (fr.Flags&FR_DIALOGTERM)
	{
		hFR = NULL;
		MessageBox(hDialog, TEXT("����������� ������ ������ � ������!"), TEXT("����� � ������"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (fr.Flags & FR_MATCHCASE)
		MessageBox(hDialog, TEXT("���������� ������ �������������������"), TEXT("����� � ������"), MB_OK | MB_ICONINFORMATION);

	if (fr.Flags & FR_WHOLEWORD)
		MessageBox(hDialog, TEXT("���������� ������ ������ ����� �������"), TEXT("����� � ������"), MB_OK | MB_ICONINFORMATION);

	if (fr.Flags & FR_DOWN)
		MessageBox(hDialog, TEXT("������ ����� ������ � ����������� ����"), TEXT("����� � ������"), MB_OK | MB_ICONINFORMATION);
	else
		MessageBox(hDialog, TEXT("������ ����� ������ � ����������� �����"), TEXT("����� � ������"), MB_OK | MB_ICONINFORMATION);

}
//@err,hr