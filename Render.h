#pragma once

#include "NuiApi.h"
#include <d2d1.h>

#define NUI_SKELETON_POSITION_COUNT 20

//Ä¬ÈÏ²ÎÊý
static const int cScreenWidth  = 640;
static const int cScreenHeight = 480;
static const int cBytesPerPixel = 4;

static const float g_JointThickness = 3.0f;
static const float g_TrackedBoneThickness = 6.0f;
static const float g_InferredBoneThickness = 1.0f;

class CRender
{
public:
	CRender(void);
	virtual ~CRender(void);

	HRESULT Initlize(HWND hWnd, float windowWidth, float windowHeight);
	void UnInitlize();

	void SetSkeletonFrame(NUI_SKELETON_FRAME skeletonFrame);
	HRESULT SetImageFrame(NUI_IMAGE_FRAME imageFrame);
	HRESULT SetDepthFrame(void* pNuiSensor, /*NUI_IMAGE_FRAME depthFrame,*/ HANDLE handle);
	HRESULT SetDepthData(const BYTE* pDepthByte, unsigned long cbImage);
	void SetShow(bool bShowColorFrame = true, bool bShowSkeleton = true, bool bShowDepth = true);

	HRESULT Draw();
	//
	void DrawSkeleton(const NUI_SKELETON_DATA & skel, int windowWidth, int windowHeight);
	D2D1_POINT_2F SkeletonToScreen(Vector4 skeletonPoint, int width, int height);
	void DrawBone(const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1);

private:
	// Direct2D
	ID2D1Factory*           m_pD2DFactory;
	ID2D1HwndRenderTarget*   m_pRenderTarget;
	ID2D1SolidColorBrush*    m_pBrushJointTracked;
	ID2D1SolidColorBrush*    m_pBrushJointInferred;
	ID2D1SolidColorBrush*    m_pBrushBoneTracked;
	ID2D1SolidColorBrush*    m_pBrushBoneInferred;
	D2D1_POINT_2F            m_Points[NUI_SKELETON_POSITION_COUNT];
	ID2D1Bitmap*             m_pBitmap;

	NUI_SKELETON_FRAME	m_skeletonFrame;
	NUI_IMAGE_FRAME		m_imageFrame;
	NUI_IMAGE_FRAME		m_depthFrame;
	bool m_bShowColorFrame;
	bool m_bShowSkeleton;
	bool m_bShowDepthFrame;
	BYTE* m_pDepthRGBX;
	unsigned long m_ulDepth;
};

