#pragma once
#include "RenderBase.h"
#include "RenderSkeleton.h"

class CRenderColorFrame :
	public CRenderBase
{
public:
	CRenderColorFrame(void);
	virtual ~CRenderColorFrame(void);

	HRESULT Initlize(HWND hWnd, float windowWidth, float windowHeight);
	HRESULT SetColorFrame(NUI_IMAGE_FRAME imageFrame);
	void SetSkeletonFrame(NUI_SKELETON_FRAME skeletonFrame);
	HRESULT Draw(bool bHasDraw = false);

	CRenderSkeleton		m_RenderSkeleton;
protected:
	NUI_IMAGE_FRAME		m_imageFrame;
	ID2D1Bitmap*        m_pBitmap;
};

