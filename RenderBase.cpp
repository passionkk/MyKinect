#include "StdAfx.h"
#include "RenderBase.h"
#include <math.h>

// intensity shift table to generate different render colors for different tracked players
const BYTE CRenderBase::m_intensityShiftR[] = {0, 2, 0, 2, 0, 0, 2};
const BYTE CRenderBase::m_intensityShiftG[] = {0, 2, 2, 0, 2, 0, 0};
const BYTE CRenderBase::m_intensityShiftB[] = {0, 0, 2, 2, 0, 2, 0};


CRenderBase::CRenderBase(void)
{
	m_pD2DFactory = NULL;
	m_pRenderTarget = NULL;
	m_pDWriteFactory = NULL;
	m_pTextFormat = NULL;
}


CRenderBase::~CRenderBase(void)
{
}

HRESULT CRenderBase::Initlize(HWND hWnd, float windowWidth, float windowHeight)
{
	HRESULT hr = S_OK;
	if (m_pD2DFactory == NULL)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	}

	if (NULL == m_pRenderTarget)
	{
		D2D1_SIZE_U size = D2D1::SizeU( (int)windowWidth, (int)windowHeight );
		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
		rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

		// Create a Hwnd render target, in order to render to the window set in initialize
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties(hWnd, size), 
			&m_pRenderTarget);

		if ( FAILED(hr) )
		{
			TRACE(_T("Couldn't create Direct2D render target!"));
			return hr;
		}

		if (FAILED(hr))
		{
			SafeRelease(m_pRenderTarget);
			return hr;
		}
	}
	
	if (NULL == m_pDWriteFactory)
	{
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED, 
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
			);
	}

	if (NULL == m_pTextFormat)
	{
		hr = m_pDWriteFactory->CreateTextFormat(
			L"Gabriola",                   // Font family name
			NULL,                          // Font collection(NULL sets it to the system font collection)
			DWRITE_FONT_WEIGHT_REGULAR,    // Weight
			DWRITE_FONT_STYLE_NORMAL,      // Style
			DWRITE_FONT_STRETCH_NORMAL,    // Stretch
			35.0f,                         // Size    
			L"en-us",                      // Local
			&m_pTextFormat                 // Pointer to recieve the created object
			);
	}

	m_hWndShow = hWnd;
	return hr;
}

void CRenderBase::UnInitlize()
{
	SafeRelease(m_pRenderTarget);
	SafeRelease(m_pD2DFactory);
}

void CRenderBase::InitDepthColorTable()
{
	// Get the min and max reliable depth
	USHORT minReliableDepth = (m_bNearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
	USHORT maxReliableDepth = (m_bNearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

	ZeroMemory(m_depthColorTable, sizeof(m_depthColorTable));

	// Set color for unknown depth
	m_depthColorTable[0][UNKNOWN_DEPTH] = UNKNOWN_DEPTH_COLOR;

	// Fill in the "near" portion of the table with solid color
	for (int depth = UNKNOWN_DEPTH + 1; depth < minReliableDepth; depth++)
	{
		m_depthColorTable[0][depth] = TOO_NEAR_COLOR;
	}

	// Fill in the "far" portion of the table with solid color
	for (int depth = maxReliableDepth + 1; depth <= USHRT_MAX; depth++)
	{
		m_depthColorTable[0][depth] = TOO_FAR_COLOR;
	}

	for (USHORT depth = minReliableDepth; depth <= maxReliableDepth; depth++)
	{
		BYTE intensity = GetIntensity(depth);

		for (int index = 0; index <= MAX_PLAYER_INDEX; index++)
		{
			BYTE r = intensity >> m_intensityShiftR[index];
			BYTE g = intensity >> m_intensityShiftG[index];
			BYTE b = intensity >> m_intensityShiftB[index];
			SetColor(&m_depthColorTable[index][depth], r, g, b);
		}
	}
}

BYTE CRenderBase::GetIntensity(int depth)
{
	// Validate arguments
	if (depth < MIN_DEPTH || depth > MAX_DEPTH)
	{
		return UCHAR_MAX;
	}

	// Use a logarithmic scale that shows more detail for nearer depths.
	// The constants in this formula were chosen such that values between
	// MIN_DEPTH and MAX_DEPTH will map to the full range of possible
	// byte values.
	return (BYTE)(~(BYTE)min(UCHAR_MAX, log((double)(depth - MIN_DEPTH) / 500.0f + 1) * 74));
}

void CRenderBase::SetColor(UINT* pColor, BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
	if (!pColor)
		return;

	BYTE* c = (BYTE*)pColor;
	c[COLOR_INDEX_RED]   = red;
	c[COLOR_INDEX_GREEN] = green;
	c[COLOR_INDEX_BLUE]  = blue;
	c[COLOR_INDEX_ALPHA] = alpha;
}