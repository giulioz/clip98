#include <iostream>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

const char* wndClassName = "ClipboardExchanger";
HANDLE hComm;
HWND nextClipboardViewer = NULL;
bool justReadt = false;

void SetupSerialPort(HWND hwnd) {
	hComm = CreateFileA("\\\\.\\COM1",
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);

	if (hComm == INVALID_HANDLE_VALUE) {
		MessageBoxEx(hwnd, "Error opening serial port", "Error", MB_OK, NULL);
		exit(1);
	}

	DCB dcb = {0};
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	if (SetCommState(hComm, &dcb) == FALSE) {
		MessageBoxEx(hwnd, "Error setting up serial port", "Error", MB_OK, NULL);
		exit(1);
	}

	COMMTIMEOUTS timeouts = {0};
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	SetCommTimeouts(hComm, &timeouts);
}

void SendClipboard(HWND hwnd) {
	OpenClipboard(hwnd);
	HANDLE hData = GetClipboardData(CF_TEXT);
	char* pszText = static_cast<char*>(GlobalLock(hData));
	std::string text(pszText);
	GlobalUnlock(hData);
	CloseClipboard();

	if (text.size() > 0 && !justReadt) {
		unsigned long written;
		WriteFile(hComm, text.c_str(), text.size(), &written, NULL);
		WriteFile(hComm, "\f", 1, &written, NULL);
	}

	justReadt = false;
}

void RecieveData(HWND hwnd) {
	char readData;
	std::vector<char> chars;
	unsigned long nBytesRead = 0;
	do {
		ReadFile(hComm, &readData, sizeof(readData), &nBytesRead, NULL);
		if (nBytesRead > 0) {
			chars.push_back(readData);
		}
	} while (nBytesRead > 0);

	if (chars.size() > 0) {
		chars.push_back(0);
		unsigned long length = chars.size();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, length);
		char* ptr = (char*)GlobalLock(hMem);
		std::copy(chars.begin(), chars.end(), ptr);
		GlobalUnlock(hMem);
		OpenClipboard(hwnd);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();

		justReadt = true;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		nextClipboardViewer = SetClipboardViewer(hwnd);
		return 0;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		CloseHandle(hComm);
		ChangeClipboardChain(hwnd, nextClipboardViewer);
		PostQuitMessage(0);
		return 0;

	case WM_CHANGECBCHAIN:
		if((HWND) wParam == nextClipboardViewer){
			nextClipboardViewer = (HWND) lParam;
		} else if (nextClipboardViewer != NULL) {
			SendMessage(nextClipboardViewer, msg, wParam, lParam);
		}
		return 0;


	case WM_DRAWCLIPBOARD:
		SendClipboard(hwnd);
		if (nextClipboardViewer != NULL) {
			SendMessage(nextClipboardViewer, msg, wParam, lParam);
		}
		return 0;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
			LPSTR lpCmdLine, int nShowCmd) {
	WNDCLASSEX wc = {0};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszClassName = wndClassName;
	RegisterClassEx(&wc);

	HWND hwnd;
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		wndClassName,
		"ClipboardExchanger",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
		NULL, NULL, hInstance, NULL);

	SetupSerialPort(hwnd);

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	MSG msg = {0};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			RecieveData(hwnd);
		}
	}

	return msg.wParam;
}
