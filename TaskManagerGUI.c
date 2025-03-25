#include <Windows.h>
#include <strsafe.h>
#include <psapi.h>
#include <stdio.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")

HWND ghwnd = NULL;
static int iVScrollPosition; // global scroll bar variable
unsigned long processCount;	 // count of processes
static int cxChar, cyChar;
FILE* file = NULL;

// Function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void mainDisplay(void);
void ErrorExit(LPCTSTR lpszFunction);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{

	file = fopen("log.txt", "w");
	if (file == NULL)
	{
		MessageBox(NULL, TEXT("Log file cannot be created!"), TEXT("Error"), MB_OK | MB_ICONERROR);
		exit(-1);
	}

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("Task_Manager_Class"); 		// Window's Class name
	TCHAR szTitleName[] = TEXT("Task Manager");			// Application's Title name

	// Initializing members of WNDCLASSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szClassName;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpszMenuName = NULL;

	RegisterClassEx(&wndclass);

	// Create window in memory
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		szClassName,
		szTitleName,
		WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
			WS_VSCROLL,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(HWND)NULL,
		(HMENU)NULL,
		hInstance,
		(LPVOID)NULL);

	ghwnd = hwnd;

	// Show the window
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	// Message loop
	// BOOL getMessage =
	// GetMessage(&msg, NULL, 0, 0);
	// while (GetMessage(&msg, NULL, 0, 0))
	// {
	// 	TranslateMessage(&msg);
	// 	DispatchMessage(&msg);
	// }
	
	int bdone = FALSE;
	
	while (bdone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bdone = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			mainDisplay();
		}
	}

	return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	#define MAX_CHARS 48

	static int cxScreen, cyScreen;

	TEXTMETRIC tm;
	HDC hdc = NULL;

	switch (iMsg)
	{

		case WM_CREATE:
		{
			hdc = GetDC(hwnd);
			GetTextMetrics(hdc, &tm);
			ReleaseDC(hwnd, hdc);
			cxChar = tm.tmAveCharWidth;
			cyChar = tm.tmHeight + tm.tmExternalLeading;

			SetScrollRange(hwnd, SB_VERT, 0, processCount - 1, FALSE);
			SetScrollPos(hwnd, SB_VERT, iVScrollPosition, TRUE);

			break;
		}

		case WM_SIZE:
		{
			cxScreen = LOWORD(lParam);
			cyScreen = HIWORD(lParam);
			break;
		}

		case WM_VSCROLL:
			switch (LOWORD(wParam))
			{
			case SB_LINEUP:
				iVScrollPosition -= 1;
				break;

			case SB_PAGEUP:
				iVScrollPosition -= cyScreen / cyChar;
				break;

			case SB_LINEDOWN:
				iVScrollPosition += 1;
				break;

			case SB_PAGEDOWN:
				iVScrollPosition += cyScreen / cyChar;
				break;

			case SB_THUMBPOSITION:
				iVScrollPosition = HIWORD(wParam);
				break;
			}

			iVScrollPosition = max(0, min(iVScrollPosition, processCount - 1));
			if (iVScrollPosition != GetScrollPos(hwnd, SB_VERT))
			{
				SetScrollPos(hwnd, SB_VERT, iVScrollPosition, FALSE);
				InvalidateRect(hwnd, NULL, TRUE);
			}

			break;

		case WM_KEYDOWN:
		{
			switch (LOWORD(wParam))
			{
				case VK_ESCAPE:	// quit on Esc
				{
					DestroyWindow(hwnd);
					break;
				}
			}
			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}

		default:
		{
			break;
		}
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ErrorExit(LPCTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
									  (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
					LocalSize(lpDisplayBuf) / sizeof(TCHAR),
					TEXT("%s failed with error %d: %s"),
					lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}

void mainDisplay(void)
{
	HDC hdc;

	TCHAR nameColomn[] = TEXT("Name\n\n");
	TCHAR pidColomn[] = TEXT("PID");

	int nameY = 30;

	hdc = GetDC(ghwnd);
	SetTextColor(hdc, RGB(255, 0, 0)); // set text color to red
	SetBkColor(hdc, RGB(0, 0, 0));	   // set background color to black

	/********** LOGIC *************/

	int i;
	int pidY = 30;
	char pidArray[2000];

	unsigned long processIdArray[2000];
	unsigned long returnSize = 0;

	BOOL status;
	status = EnumProcesses(processIdArray, sizeof(processIdArray), &returnSize);

	if (status == 0) // errrror...
	{
		fprintf(file, "EnumProcesses() failed!\n");
		exit(-1);
	}

	processCount = returnSize / sizeof(unsigned long);

	HMODULE hmod;
	HANDLE hprocess;
	unsigned long epmSize;
	int returnValue = 0;

	char lpBaseName[8192];
	DWORD Namefilereturn;

	/* jo parent scroll nhi hot to parent name and pid show kar */
	if (iVScrollPosition == 0)
	{
		TextOut(hdc, 10, 10, pidColomn, strlen(pidColomn));
		TextOut(hdc, 110, 10, nameColomn, strlen(nameColomn));
	}

	SetTextColor(hdc, RGB(255, 255, 255)); // set text color to white

	unsigned long foundNamePids[2000];
	int mainProcessCount = 0;
	for (int i = 0; i < processCount; i++)
	{
		hprocess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIdArray[i]);

		// pid printf
		sprintf(pidArray, "%ld\0", processIdArray[i]);

		if (NULL != hprocess)
		{
			returnValue = EnumProcessModules(hprocess, &hmod, sizeof(hmod), &epmSize);
			// ErrorExit(TEXT("EnumProcessModules"));

			if (returnValue != 0)
			{
				GetModuleBaseName(hprocess, hmod, lpBaseName, 8192);
				// fprintf(file, "Name: %d, %d, %d -> %d\n", mainProcessCount, iVScrollPosition, cyChar, (mainProcessCount - iVScrollPosition) * cyChar);
				
				TextOut(hdc, 10, (mainProcessCount - iVScrollPosition) * cyChar + 30, pidArray, strlen(pidArray));
				TextOut(hdc, 100, (mainProcessCount - iVScrollPosition) * cyChar + 30, lpBaseName, strlen(lpBaseName));
				mainProcessCount = mainProcessCount + 1;
			}
		}
	}

	ReleaseDC(ghwnd, hdc);
}
