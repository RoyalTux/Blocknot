#include "MenuBarDlg.h"
#pragma warning(disable:4996)
OPENFILENAME ofn;
// идентификатор зарегистрированного сообщения FINDMSGSTRING
UINT WM_FR = RegisterWindowMessage(FINDMSGSTRING);
//
int SizeDoc, MesRet;
//
char *BuferText; // max 9,53 МБ (10 000 000 байт)
char Opffilt[256] = "Текстовые файлы *.txt\0*.txt\0Все файлы *.*\0*.*\0";
char Sffilt[256] = "*.*\0*.*\0";
char Opffile[256];
char Opffiletitle[256];
char Path[512];
char TitleWnd[1024];

//TCHAR *BuferText; // max 9,53 МБ (10 000 000 байт)
//TCHAR Opffilt[256] = "Текстовые файлы *.txt\0*.txt\0Все файлы *.*\0*.*\0";
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
	// Получим дескриптор текстового поля
	hEdit = GetDlgItem(hDialog, IDC_EDIT1);
	// Создадим строку состояния
	hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_TOOLTIPS | SBARS_SIZEGRIP, 0, hDialog, WM_USER);
	// Загрузим меню из ресурсов приложения
	hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU1));
	hMenuEn = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU2));
	//hFont = GetDlgItem(hDialog, ID_FRONT);
	// Присоединим меню к главному окну приложения
	SetMenu(hDialog, hMenu);
	return TRUE;
}

// Обработчик сообщения WM_COMMAND будет вызван при выборе пункта меню
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
		MessageBox(ptr->hDialog, "Блокнот v1.0\nЩебетовский Дмитрий\nЕКО П2", "Справка", MB_OK | MB_ICONINFORMATION);
		break;
	case ID_HELP:
		MessageBox(ptr->hDialog, "По идее, переход на сайт со спракой...", "Справка", MB_OK | MB_ICONINFORMATION);
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
		SetWindowText(hwnd, "Блокнот");
		break;
	case ID_CANCEL:
		// Отменим последнее действие
		SendMessage(hEdit, WM_UNDO, 0, 0);
		break;
	case ID_CUT:
		// Удалим выделенный фрагмент текста в буфер обмена
		SendMessage(hEdit, WM_CUT, 0, 0);
		break;
	case ID_COPY:
		// Скопируем выделенный фрагмент текста в буфер обмена
		SendMessage(hEdit, WM_COPY, 0, 0);
		break;
	case ID_PASTE:
		// Вставим текст в Edit Control из буфера обмена
		SendMessage(hEdit, WM_PASTE, 0, 0);
		break;
	case ID_DEL:
		// Удалим выделенный фрагмент текста
		SendMessage(hEdit, WM_CLEAR, 0, 0);
		break;
	case ID_SELECTALL:
		// Выделим весь текст в Edit Control
		SendMessage(hEdit, EM_SETSEL, 0, -1);
		break;
	case ID_STATUS_BAR:
		// Если флаг равен TRUE, то строка состояния отображена
		if (bShowStatusBar)
		{
			// Получим дескриптор главного меню
			HMENU hMenu = GetMenu(hDialog);
			// Снимем отметку с пункта меню "Строка состояния"
			CheckMenuItem(hMenu, ID_STATUS_BAR, MF_BYCOMMAND | MF_UNCHECKED);
			// Скроем строку состояния
			ShowWindow(hStatus, SW_HIDE);
		}
		else
		{
			// Получим дескриптор главного меню
			HMENU hMenu = GetMenu(hDialog);
			// Установим отметку на пункте меню "Строка состояния"
			CheckMenuItem(hMenu, ID_STATUS_BAR, MF_BYCOMMAND | MF_CHECKED);
			// Отобразим строку состояния
			ShowWindow(hStatus, SW_SHOW);
		}
		bShowStatusBar = !bShowStatusBar;
	}
}

// Обработчик сообщения WM_SIZE будет вызван при изменении размеров главного окна
// либо при сворачивании/восстановлении главного окна
void CMenuBarDlg::Cls_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	RECT rect1, rect2;
	// Получим координаты клиентской области главного окна
	GetClientRect(hDialog, &rect1);
	// Получим координаты единственной секции строки состояния
	SendMessage(hStatus, SB_GETRECT, 0, (LPARAM)&rect2);
	// Установим новые размеры текстового поля
	MoveWindow(hEdit, rect1.left, rect1.top, rect1.right, rect1.bottom - (rect2.bottom - rect2.top), 1);
	// Установим размер строки состояния, 
	// равный ширине клиентской области главного окна
	SendMessage(hStatus, WM_SIZE, 0, 0);
}

// Обработчик WM_INITMENUPOPUP будет вызван непосредственно 
// перед активизацией всплывающего меню
void CMenuBarDlg::Cls_OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu)
{
	if (item == 0) // Активизируется пункт меню "Правка"
	{
		// Получим границы выделения текста
		DWORD dwPosition = SendMessage(hEdit, EM_GETSEL, 0, 0);
		WORD wBeginPosition = LOWORD(dwPosition);
		WORD wEndPosition = HIWORD(dwPosition);

		if (wEndPosition != wBeginPosition) // Выделен ли текст?
		{
			// Если имеется выделенный текст, 
			// то сделаем разрешёнными пункты меню "Копировать", "Вырезать" и "Удалить"
			EnableMenuItem(hMenu, ID_COPY, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, ID_CUT, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, ID_DEL, MF_BYCOMMAND | MF_ENABLED);
		}
		else
		{
			// Если отсутствует выделенный текст, 
			// то сделаем недоступными пункты меню "Копировать", "Вырезать" и "Удалить"
			EnableMenuItem(hMenu, ID_COPY, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, ID_CUT, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, ID_DEL, MF_BYCOMMAND | MF_GRAYED);
		}

		if (IsClipboardFormatAvailable(CF_TEXT)) // Имеется ли текст в буфере обмена?
			// Если имеется текст в буфере обмена, 
			// то сделаем разрешённым пункт меню "Вставить"
			EnableMenuItem(hMenu, ID_PASTE, MF_BYCOMMAND | MF_ENABLED);
		else
			// Если отсутствует текст в буфере обмена, 
			// то сделаем недоступным пункт меню "Вставить"
			EnableMenuItem(hMenu, ID_PASTE, MF_BYCOMMAND | MF_GRAYED);

		// Существует ли возможность отмены последнего действия?
		if (SendMessage(hEdit, EM_CANUNDO, 0, 0))
			// Если существует возможность отмены последнего действия,
			// то сделаем разрешённым пункт меню "Отменить"
			EnableMenuItem(hMenu, ID_UNDO, MF_BYCOMMAND | MF_ENABLED);
		else
			// Если отсутствует возможность отмены последнего действия,
			// то сделаем недоступным пункт меню "Отменить"
			EnableMenuItem(hMenu, ID_UNDO, MF_BYCOMMAND | MF_GRAYED);

		// Определим длину текста в Edit Control
		int length = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
		// Выделен ли весь текст в Edit Control?
		if (length != wEndPosition - wBeginPosition)
			//Если не весь текст выделен в Edit Control,
			// то сделаем разрешённым пункт меню "Выделить всё"
			EnableMenuItem(hMenu, ID_SELECTALL, MF_BYCOMMAND | MF_ENABLED);
		else
			// Если выделен весь текст в Edit Control,
			// то сделаем недоступным пункт меню "Выделить всё"
			EnableMenuItem(hMenu, ID_SELECTALL, MF_BYCOMMAND | MF_GRAYED);
	}
}

void CMenuBarDlg::Cls_OnMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags)
{
	if (flags & MF_POPUP) // Проверим, является ли выделенный пункт меню заголовком выпадающего подменю?
	{
		// Выделенный пункт меню является заголовком выпадающего подменю
		SendMessage(hStatus, SB_SETTEXT, 0, 0); // Убираем текст со строки состояния
	}
	else
	{
		// Выделенный пункт меню является конечным пунктом (пункт меню "команда")
		TCHAR buf[200];
		// Получим дескриптор текущего экземпляра приложения
		HINSTANCE hInstance = GetModuleHandle(NULL);
		// Зарузим строку из таблицы строк, расположенной в ресурсах приложения
		// При этом идентификатор загружаемой строки строго соответствует идентификатору выделенного пункта меню
		LoadString(hInstance, item, buf, 200);
		// Выводим в строку состояния контекстную справку, соответствующую выделенному пункту меню
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
	// обработка сообщений, посылаемых из немодального диалогового окна
	if (message == WM_FR)
		ptr->MessageFromFindReplace();
	return FALSE;
}

void CMenuBarDlg::MemAlloc(int size)
{
	BuferText = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
	if (BuferText == 0)
	{
		MessageBox(ptr->hDialog, "Ошибка в куче", "", MB_ICONERROR);
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
			MessageBox(ptr->hDialog, "Невозможно открыть файл", "Ошибка", MB_ICONERROR);
			return;
		}
		SizeDoc = GetFileSize(hFile, NULL);
		SizeDoc++;
		if (SizeDoc>10000000)
		{
			CloseHandle(hFile);
			MessageBox(ptr->hDialog, "Размер открываемого файла превышает предел", "Ошибка", MB_ICONERROR);
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
		MessageBox(ptr->hDialog, "Невозможно сохранить файл", "Ошибка", MB_ICONERROR);
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
		MesRet = MessageBox(ptr->hDialog, "Текст был изменен\nСохранить изменения?", "", MB_YESNOCANCEL | MB_ICONINFORMATION);
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
	// Проверим, открыто ли окно поиска
	if (hFR)
	{
		//Активизируем окно поиска
		SetForegroundWindow(hFR);
		return;
	}
	// обнуляем структуру FINDREPLACE
	ZeroMemory(&fr, sizeof(fr));
	DWORD start, end;
	// получим весь текст, находяшийся в текстовом поле ввода
	GetWindowText(hEdit, alltext, 65536);
	// получим границы выделения фрагмента текста
	SendMessage(hEdit, EM_GETSEL, WPARAM(&start), LPARAM(&end));
	// скопируем в буфер выделенный фрагмент текста
	_tcsncpy(bufFind, alltext + start, end - start);
	bufFind[end - start] = TEXT('\0');
	fr.lStructSize = sizeof(fr);
	// главный диалог является окном-владелецем
	fr.hwndOwner = hDialog;
	// указатель на буфер, содержащий строку для поиска
	fr.lpstrFindWhat = bufFind;
	fr.wFindWhatLen = 100;
	// поиск от текущего положения каретки в тексте до конца документа
	fr.Flags = FR_DOWN;
	// отображаем диалог Найти
	hFR = FindText(&fr);
}

void CMenuBarDlg::OnReplace()
{
	// Проверим, открыто ли окно замены
	if (hFR)
	{
		// Активизируем окно замены
		SetForegroundWindow(hFR);
		return;
	}
	// обнуляем структуру FINDREPLACE
	ZeroMemory(&fr, sizeof(fr));
	DWORD start, end;
	// обнуляем буфер, предназначенный для хранения замещающей строки 
	ZeroMemory(&bufReplace, sizeof(bufReplace));
	// получим весь текст, находяшийся в текстовом поле ввода
	GetWindowText(hEdit, alltext, 65536);
	// получим границы выделения фрагмента текста
	SendMessage(hEdit, EM_GETSEL, WPARAM(&start), LPARAM(&end));
	// скопируем в буфер выделенный фрагмент текста
	_tcsncpy(bufFind, alltext + start, end - start);
	bufFind[end - start] = TEXT('\0');
	fr.lStructSize = sizeof(fr);
	// главный диалог является окном-владелецем
	fr.hwndOwner = hDialog;
	// указатель на буфер, содержащий строку для поиска
	fr.lpstrFindWhat = bufFind;
	fr.wFindWhatLen = 100;
	// указатель на буфер, содержащий строку для замены
	fr.lpstrReplaceWith = bufReplace;
	fr.wReplaceWithLen = 100;
	// поиск от текущего положения каретки в тексте до конца документа
	fr.Flags = FR_DOWN;
	// отображаем диалог Заменить
	hFR = ReplaceText(&fr);
}

void CMenuBarDlg::MessageFromFindReplace()
{
	if (fr.Flags & FR_REPLACEALL)
		MessageBox(hDialog, TEXT("Нажата кнопка \"Заменить всё\""), TEXT("Поиск и замена"), MB_OK | MB_ICONINFORMATION);

	if (fr.Flags & FR_REPLACE)
	{
		MessageBox(hDialog, TEXT("Нажата кнопка \"Заменить\""), TEXT("Поиск и замена"), MB_OK | MB_ICONINFORMATION);
		// заменяем выделенный фрагмент текста на строку, находящуюся в буфере bufReplace
		SendMessage(hEdit, EM_REPLACESEL, WPARAM(TRUE), (LPARAM)bufReplace);
	}

	if (fr.Flags & FR_FINDNEXT)
	{
		MessageBox(hDialog, TEXT("Нажата кнопка \"Найти далее\""), TEXT("Поиск и замена"), MB_OK | MB_ICONINFORMATION);
		DWORD Start, End;
		// выполняем поиск искомой строки
		TCHAR * p = _tcsstr(alltext, bufFind);
		if (p)
		{
			Start = p - alltext;
			End = Start + _tcslen(bufFind);
			// выделяем найденную строку
			SendMessage(hEdit, EM_SETSEL, Start, End);
		}
	}

	if (fr.Flags&FR_DIALOGTERM)
	{
		hFR = NULL;
		MessageBox(hDialog, TEXT("Закрывается диалог поиска и замены!"), TEXT("Поиск и замена"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (fr.Flags & FR_MATCHCASE)
		MessageBox(hDialog, TEXT("Установлен флажок регистрозависимости"), TEXT("Поиск и замена"), MB_OK | MB_ICONINFORMATION);

	if (fr.Flags & FR_WHOLEWORD)
		MessageBox(hDialog, TEXT("Установлен флажок поиска слова целиком"), TEXT("Поиск и замена"), MB_OK | MB_ICONINFORMATION);

	if (fr.Flags & FR_DOWN)
		MessageBox(hDialog, TEXT("Выбран режим поиска в направлении вниз"), TEXT("Поиск и замена"), MB_OK | MB_ICONINFORMATION);
	else
		MessageBox(hDialog, TEXT("Выбран режим поиска в направлении вверх"), TEXT("Поиск и замена"), MB_OK | MB_ICONINFORMATION);

}
//@err,hr