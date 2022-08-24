#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define POINT_COUNT 6
#define POINT_OFFSET 0.30
#define SPEEDUP_ANGLE 0.80
#define RING_WIDTH 128
#define X_POS 128
#define Y_POS 128
#define POINT_WIDTH 8.0

TCHAR szClassName[] = TEXT("Window");

template<class Interface> inline void SafeRelease(Interface** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL) {
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = NULL;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static D2D1_POINT_2F point[POINT_COUNT];
	static double dAnimation[POINT_COUNT];
	static ID2D1Factory* m_pD2DFactory;
	static IDWriteFactory* m_pDWriteFactory;
	static ID2D1HwndRenderTarget* m_pRenderTarget;
	static ID2D1SolidColorBrush* m_pBlackBrush;
	switch (msg)
	{
	case WM_CREATE:
		{
			HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
			if (SUCCEEDED(hr))
				hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
			if (FAILED(hr))
				return -1;
			for (int i = 0; i < POINT_COUNT; i++) {
				dAnimation[i] = 1.4 * M_PI * i / POINT_COUNT;
				while (dAnimation[i] > 2.0 * M_PI) dAnimation[i] -= 2.0 * M_PI;
				point[i].x = (FLOAT)(X_POS + (RING_WIDTH / 2.0) + (RING_WIDTH / 2.0) * cos(dAnimation[i] + i * POINT_OFFSET - SPEEDUP_ANGLE));
				point[i].y = (FLOAT)(Y_POS + (RING_WIDTH / 2.0) + (RING_WIDTH / 2.0) * sin(dAnimation[i] + i * POINT_OFFSET - SPEEDUP_ANGLE));
			}
		}
		SetTimer(hWnd, 0x1234, 10, 0);
		break;
	case WM_TIMER:
		{
			for (int i = 0; i < POINT_COUNT; i++) {
				dAnimation[i] += (sin(dAnimation[i]) + 1.2) / 10.0;
				while (dAnimation[i] > 2.0 * M_PI) dAnimation[i] -= 2.0 * M_PI;
				point[i].x = (FLOAT)(X_POS + (RING_WIDTH / 2.0) + (RING_WIDTH / 2.0) * cos(dAnimation[i] + i * POINT_OFFSET - SPEEDUP_ANGLE));
				point[i].y = (FLOAT)(Y_POS + (RING_WIDTH / 2.0) + (RING_WIDTH / 2.0) * sin(dAnimation[i] + i * POINT_OFFSET - SPEEDUP_ANGLE));
			}
			InvalidateRect(hWnd, 0, 0);
		}
		break;
	case WM_PAINT:
		{
			HRESULT hr = S_OK;
			if (!m_pRenderTarget) {
				RECT rect;
				GetClientRect(hWnd, &rect);
				D2D1_SIZE_U size = D2D1::SizeU(rect.right, rect.bottom);
				hr = m_pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &m_pRenderTarget);
				if (SUCCEEDED(hr))
					hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);
			}
			if (SUCCEEDED(hr)) {
				static const WCHAR sc_helloWorld[] = L"Hello, World!";
				D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();
				m_pRenderTarget->BeginDraw();
				m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
				m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
				D2D1_ELLIPSE ellipse;
				for (int i = 0; i < POINT_COUNT; i++) {
					ellipse.point.x = (FLOAT)point[i].x;
					ellipse.point.y = (FLOAT)point[i].y;
					ellipse.radiusX = POINT_WIDTH;
					ellipse.radiusY = POINT_WIDTH;
					m_pRenderTarget->FillEllipse(ellipse, m_pBlackBrush);
				}
				hr = m_pRenderTarget->EndDraw();
				if (hr == D2DERR_RECREATE_TARGET) {
					hr = S_OK;
					SafeRelease(&m_pRenderTarget);
					SafeRelease(&m_pBlackBrush);
				}
			}
		}
		ValidateRect(hWnd, NULL);
		break;
	case WM_DISPLAYCHANGE:
		InvalidateRect(hWnd, 0, 0);
		break;
	case WM_SIZE:
		if (m_pRenderTarget) {
			D2D1_SIZE_U size = { LOWORD(lParam), HIWORD(lParam) };
			m_pRenderTarget->Resize(size);
		}
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 0x1234);
		SafeRelease(&m_pD2DFactory);
		SafeRelease(&m_pDWriteFactory);
		SafeRelease(&m_pRenderTarget);
		SafeRelease(&m_pBlackBrush);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
