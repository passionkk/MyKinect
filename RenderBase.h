#pragma once

//Render Base Class
#include "NuiApi.h"
#include <d2d1.h>
#include <dwrite.h>

#define MAX_PLAYER_INDEX    6
#define NUI_SKELETON_POSITION_COUNT 20

enum WindowHwnd{
	ColorWindowHwnd = 0,	//颜色
	DepthWindowHwnd,		//深度
	FrontWindowHwnd,		//正视
	SideWindowHwnd,			//侧视
	TopWindowHwnd,			//俯视
	CountWindowHwnd			
};

//默认参数
static const int cScreenWidth  = 640;
static const int cScreenHeight = 480;
static const int cBytesPerPixel = 4;
static const int sourceStride = cScreenWidth * sizeof(long);

static const float g_JointThickness = 3.0f;
static const float g_TrackedBoneThickness = 6.0f;
static const float g_InferredBoneThickness = 1.0f;


#define MIN_DEPTH                   400
#define MAX_DEPTH                   16383
#define UNKNOWN_DEPTH               0
#define UNKNOWN_DEPTH_COLOR         0x003F3F07
#define TOO_NEAR_COLOR              0x001F7FFF
#define TOO_FAR_COLOR               0x007F0F3F
#define NEAREST_COLOR               0x00FFFFFF

#define BYTES_PER_PIXEL_RGB         4
#define BYTES_PER_PIXEL_INFRARED    2
#define BYTES_PER_PIXEL_BAYER       1
#define BYTES_PER_PIXEL_DEPTH       sizeof(NUI_DEPTH_IMAGE_PIXEL)

#define COLOR_INDEX_BLUE            0
#define COLOR_INDEX_GREEN           1
#define COLOR_INDEX_RED             2
#define COLOR_INDEX_ALPHA           3

class CRenderBase
{
public:
	CRenderBase(void);
	virtual ~CRenderBase(void);

public:
	virtual HRESULT Initlize(HWND hWnd, float windowWidth, float windowHeight);
	virtual void UnInitlize();

	//color frame 可能调用 skeleton的draw 这时候不能让skeleton再去begindraw enddraw
	virtual HRESULT Draw(bool bHasDraw = false) {return S_OK;};
	
	void SetNearMode(bool bNearMode) { m_bNearMode = bNearMode; };
	void SetShowMode(bool bShowFrame = true, bool bShowSkeleton = true) { m_bShowFrame = bShowFrame; m_bShowSkeleton = bShowSkeleton;};

	// Init Depth Color Table
	void InitDepthColorTable();
	BYTE GetIntensity(int depth);
	inline void SetColor(UINT* pColor, BYTE red, BYTE green, BYTE blue, BYTE alpha = 255);

protected:
	HWND	m_hWndShow;
	bool	m_bNearMode;
	bool	m_bShowFrame;	//是否显示图像 深度或者彩色
	bool	m_bShowSkeleton; // 是否显示骨骼

	// Direct2D
	ID2D1Factory*           m_pD2DFactory;
	ID2D1HwndRenderTarget*  m_pRenderTarget;
	//绘制文字
	IDWriteFactory*			m_pDWriteFactory;
	IDWriteTextFormat*		m_pTextFormat;

	static const BYTE    m_intensityShiftR[MAX_PLAYER_INDEX + 1];
	static const BYTE    m_intensityShiftG[MAX_PLAYER_INDEX + 1];
	static const BYTE    m_intensityShiftB[MAX_PLAYER_INDEX + 1];
	UINT                 m_depthColorTable[MAX_PLAYER_INDEX + 1][USHRT_MAX + 1];
};

