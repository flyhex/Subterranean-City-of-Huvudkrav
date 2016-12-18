#pragma once
#include <windows.h>
#include "GraphicsCore.h"
#include "Input.h"
#include "Game.h"
#include <io.h>
#include <fcntl.h>
//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE			hInst	= NULL;
HWND				hWnd	= NULL;
GraphicsCore		*core	= NULL;
Input				*input	= NULL;
Game				*game	= NULL;
ObjLoader			*objLoader = NULL;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT				initWindow( HINSTANCE hInstance, int nCmdShow );
LRESULT	CALLBACK	wndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
void				cleanUp();
void				createConsoleLog(const char *winTitle);

//--------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow )
{
	createConsoleLog("Output Console");

	if( FAILED( initWindow( hInstance, nCmdShow ) ) )
	{
		MessageBox(0, "Error initializing window!", 0, 0);
		return 0;
	}

	input = Input::getInstance();
	if( !input->init(&hInstance, &hWnd) )
	{
		MessageBox(0, "Error initializing input!", 0, 0);
		return 0;
	}

	core = GraphicsCore::getInstance();
	if( !core->init(&hWnd) )
		return 0;

	objLoader = ObjLoader::getInstance();


	game = new Game();

	__int64			currTimeStamp = 0, prevTimeStamp = 0, cntsPerSec = 0;
	QueryPerformanceFrequency( ( LARGE_INTEGER* )&cntsPerSec);
	double			dt = 0, time = 0;
	double			secsPerCnt = 1.0 / (double)cntsPerSec;

	QueryPerformanceCounter( ( LARGE_INTEGER* )&currTimeStamp);
	prevTimeStamp	= currTimeStamp;
	MSG msg			= { 0 };
	while( WM_QUIT != msg.message )
	{

		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			__int64 currTimeStamp = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
			double dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;	//time since last frame

			if(GetActiveWindow() == hWnd)
			{
				input->update();
				if(input->getKeyState()[DIK_ESCAPE] & 0x80)
					PostQuitMessage(0);
				game->update(dt);

				
				GraphicsCore::getInstance()->clearRenderTarget(.3f, 0.4f, .2f, 1.0f);
				GraphicsCore::getInstance()->clearMultipleRenderTarget(0.0f,0.0f,0.0f);

				game->draw();

				GraphicsCore::getInstance()->presentSwapChain();
			}

			prevTimeStamp = currTimeStamp;
		}
	}

	cleanUp();
	_CrtDumpMemoryLeaks();

	return (int) msg.wParam;
}

HRESULT initWindow( HINSTANCE hInstance, int nCmdShow )
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = wndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = 0;
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = "Huvudkrav";
	wcex.hIconSm        = 0;
	if( !RegisterClassEx(&wcex) )
		return E_FAIL;

	// Create window
	hInst				= hInstance; 
	RECT rc = { 0, 0, DEFAULTSCRNWIDTH, DEFAULTSCRNHEIGHT };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	
	if(!(hWnd = CreateWindow(	"Huvudkrav",
							"Subterranean City of Huvudkrav",
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							rc.right - rc.left,
							rc.bottom - rc.top,
							NULL,
							NULL,
							hInstance,
							NULL)))
	{
		return E_FAIL;
	}

	ShowWindow( hWnd, nCmdShow );
	
	return S_OK;
}

LRESULT CALLBACK wndProc( HWND _hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_PAINT:
		hdc = BeginPaint(_hWnd, &ps);
		EndPaint(_hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(_hWnd, message, wParam, lParam);
	}

	return 0;
}

void cleanUp()
{
	SAFE_DELETE(core);
	SAFE_DELETE(game);
	SAFE_DELETE(input);
	SAFE_DELETE(objLoader);
}

void createConsoleLog(const char *winTitle)
{
	AllocConsole();
	SetConsoleTitle(winTitle);

	int hConHandle;
	long lStdHandle;
	FILE fp;

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = *_fdopen( hConHandle, "w" );
	*stdout = fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = *_fdopen( hConHandle, "r" );
	*stdin = fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = *_fdopen( hConHandle, "w" );
	*stderr = fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
}