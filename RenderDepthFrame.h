#pragma once
#include "renderbase.h"
#include "RenderSkeleton.h"

class CRenderDepthFrame :
	public CRenderBase
{
public:
	CRenderDepthFrame(void);
	virtual ~CRenderDepthFrame(void);
	HRESULT Initlize(HWND hWnd, float windowWidth, float windowHeight);
	HRESULT SetDepthFrame( void* pNuiSensor, HANDLE handle );
	HRESULT SetDepthData(const NUI_LOCKED_RECT& LockedRect, BOOL bNearMode = FALSE);
	void SetSkeletonFrame(NUI_SKELETON_FRAME skeletonFrame);
	HRESULT Draw(bool bHasDraw = false);

protected:
	NUI_IMAGE_FRAME		m_depthFrame;
	CRenderSkeleton		m_RenderSkeleton;
	ID2D1Bitmap*        m_pBitmap;
	BYTE*				m_pDepthRGBX;
};

