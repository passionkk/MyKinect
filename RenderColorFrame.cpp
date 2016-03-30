#include "StdAfx.h"
#include "RenderColorFrame.h"

CRenderColorFrame::CRenderColorFrame(void)
{
	m_RenderSkeleton.SetCameraDirection(CamDir_Front);
}


CRenderColorFrame::~CRenderColorFrame(void)
{
}

HRESULT CRenderColorFrame::Initlize(HWND hWnd, float windowWidth, float windowHeight)
{
	HRESULT hr = S_OK;
	hr = __super::Initlize(hWnd, windowWidth, windowHeight);
	D2D1_SIZE_U size = D2D1::SizeU( (int)windowWidth, (int)windowHeight );
	hr = m_pRenderTarget->CreateBitmap(
		size,
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE) ),
		&m_pBitmap
		);
	if (FAILED(hr))
	{
		SafeRelease(m_pRenderTarget);
		return hr;
	}

	m_RenderSkeleton.SetRenderTarget(m_pRenderTarget);
	hr = m_RenderSkeleton.Initlize(hWnd, cScreenWidth * 1.0, cScreenHeight * 1.0);
	return hr;
}

HRESULT CRenderColorFrame::SetColorFrame(NUI_IMAGE_FRAME imageFrame)
{
	HRESULT hr = S_OK;
	m_imageFrame = imageFrame;

	INuiFrameTexture* pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	if (LockedRect.Pitch != 0)
	{
		if ( LockedRect.size < ((cScreenHeight - 1) * sourceStride) + (cScreenWidth * 4) )
		{
			return E_INVALIDARG;
		}
		hr = m_pBitmap->CopyFromMemory(NULL, static_cast<BYTE*>(LockedRect.pBits), sourceStride);
		if ( FAILED(hr) )
		{
			return hr;
		}
	}
	pTexture->UnlockRect(0);
	return hr;
}

void CRenderColorFrame::SetSkeletonFrame(NUI_SKELETON_FRAME skeletonFrame)
{
	m_RenderSkeleton.SetSkeletonFrame(skeletonFrame);
}

HRESULT CRenderColorFrame::Draw(bool bHasDraw)
{
	HRESULT hr = S_OK;
	if (m_pRenderTarget != nullptr)
	{
		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->DrawBitmap(m_pBitmap);
		m_RenderSkeleton.Draw(true);
		m_pRenderTarget->EndDraw();
	}
	return hr;
}