#include "StdAfx.h"
#include "RenderDepthFrame.h"


CRenderDepthFrame::CRenderDepthFrame(void)
{
	m_RenderSkeleton.SetCameraDirection(CamDir_Front);
	m_pDepthRGBX = new BYTE[cScreenWidth * cScreenHeight * cBytesPerPixel];
}

CRenderDepthFrame::~CRenderDepthFrame(void)
{
	delete[] m_pDepthRGBX;
}

HRESULT CRenderDepthFrame::Initlize( HWND hWnd, float windowWidth, float windowHeight )
{
	HRESULT hr = __super::Initlize(hWnd, windowWidth, windowHeight);
	
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
		TRACE(L"Couldn't create Direct2D render target!");
		return hr;
	}

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
	hr = m_RenderSkeleton.Initlize(m_hWndShow, cScreenWidth * 1.0, cScreenHeight * 1.0);

	return hr;
}

HRESULT CRenderDepthFrame::SetDepthFrame( void* pNuiSensor, HANDLE handle )
{
	HRESULT hr = S_OK;
	INuiSensor* pINuiSensor = (INuiSensor*)pNuiSensor;
	NUI_IMAGE_FRAME depthFrame;
	hr = pINuiSensor->NuiImageStreamGetNextFrame(handle, 0, &depthFrame);

	m_depthFrame = depthFrame;
	if (pNuiSensor == NULL)
	{
		return S_FALSE;
	}
	
	BOOL nearMode;
	INuiFrameTexture* pTexture;
	hr = pINuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
		handle, &m_depthFrame, &nearMode, &pTexture);

	NUI_LOCKED_RECT LockedRect;

	pTexture->LockRect(0, &LockedRect, NULL, 0);

	if (LockedRect.Pitch != 0)
	{
		int minDepth = (nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
		int maxDepth = (nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

		BYTE * rgbrun = m_pDepthRGBX;
		//UINT* rgbrun = (UINT*)m_pDepthRGBX;
		const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits);

		// end pixel is start + width*height - 1
		const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (cScreenWidth * cScreenHeight);

		while ( pBufferRun < pBufferEnd )
		{
			// discard the portion of the depth that contains only the player index
			USHORT depth = pBufferRun->depth;
			USHORT index = pBufferRun->playerIndex;
			// To convert to a byte, we're discarding the most-significant
			// rather than least-significant bits.
			// We're preserving detail, although the intensity will "wrap."
			// Values outside the reliable depth range are mapped to 0 (black).

			// Note: Using conditionals in this loop could degrade performance.
			// Consider using a lookup table instead when writing production code.
#if 1
			//之前处理的只是黑白色
			BYTE intensity = static_cast<BYTE>(depth >= minDepth && depth <= maxDepth ? depth % 256 : 0);
			if (depth <= minDepth)
			{
				// Write out blue byte
				*(rgbrun++) = static_cast<BYTE>(248);
				// Write out green byte
				*(rgbrun++) = static_cast<BYTE>(174);
				// Write out red byte
				*(rgbrun++) = static_cast<BYTE>(37);
			}
			else if (depth >= maxDepth)
			{
				// Write out blue byte
				*(rgbrun++) = static_cast<BYTE>(223);
				// Write out green byte
				*(rgbrun++) = static_cast<BYTE>(62);
				// Write out red byte
				*(rgbrun++) = static_cast<BYTE>(179);
			}
			else
			{
				intensity = static_cast<BYTE>((depth-minDepth) * 1.0 / (maxDepth-minDepth) * 255);
				// Write out blue byte
				*(rgbrun++) = intensity;
				// Write out green byte
				*(rgbrun++) = intensity;
				// Write out red byte
				*(rgbrun++) = intensity;
			}
			/*
			// Write out blue byte
			*(rgbrun++) = intensity;
			// Write out green byte
			*(rgbrun++) = intensity;
			// Write out red byte
			*(rgbrun++) = intensity;
			*/
#endif
			// We're outputting BGR, the last byte in the 32 bits is unused so skip it
			// If we were outputting BGRA, we would write alpha here.
			++rgbrun;

			// Increment our index into the Kinect's depth buffer
			++pBufferRun;
		}

		// Draw the data with Direct2D
		//SetDepthData(rgbrun, cScreenWidth * cScreenHeight * cBytesPerPixel);
		if ( cScreenWidth * cScreenHeight * cBytesPerPixel < ((cScreenHeight - 1) * sourceStride) + (cScreenWidth * 4) )
		{
			return E_INVALIDARG;
		}

		// Copy the image that was passed in into the direct2d bitmap
		hr = m_pBitmap->CopyFromMemory(NULL, m_pDepthRGBX, sourceStride);
		if ( FAILED(hr) )
		{
			return hr;
		}
	}

	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	pTexture->Release();

	// Release the frame
	pINuiSensor->NuiImageStreamReleaseFrame(handle, &depthFrame);
	return hr;
}

HRESULT CRenderDepthFrame::SetDepthData(const NUI_LOCKED_RECT& LockedRect, BOOL nearMode /* = FALSE */)
{
	HRESULT hr = S_OK;
	if (LockedRect.Pitch != 0)
	{
		int minDepth = (nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
		int maxDepth = (nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

		BYTE * rgbrun = m_pDepthRGBX;
		//UINT* rgbrun = (UINT*)m_pDepthRGBX;
		const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits);

		// end pixel is start + width*height - 1
		const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (cScreenWidth * cScreenHeight);

		while ( pBufferRun < pBufferEnd )
		{
			// discard the portion of the depth that contains only the player index
			USHORT depth = pBufferRun->depth;
			USHORT index = pBufferRun->playerIndex;
			// To convert to a byte, we're discarding the most-significant
			// rather than least-significant bits.
			// We're preserving detail, although the intensity will "wrap."
			// Values outside the reliable depth range are mapped to 0 (black).

			// Note: Using conditionals in this loop could degrade performance.
			// Consider using a lookup table instead when writing production code.
#if 1
			//之前处理的只是黑白色
			BYTE intensity = static_cast<BYTE>(depth >= minDepth && depth <= maxDepth ? depth % 256 : 0);
			if (depth <= minDepth)
			{
				// Write out blue byte
				*(rgbrun++) = static_cast<BYTE>(248);
				// Write out green byte
				*(rgbrun++) = static_cast<BYTE>(174);
				// Write out red byte
				*(rgbrun++) = static_cast<BYTE>(37);
			}
			else if (depth >= maxDepth)
			{
				// Write out blue byte
				*(rgbrun++) = static_cast<BYTE>(223);
				// Write out green byte
				*(rgbrun++) = static_cast<BYTE>(62);
				// Write out red byte
				*(rgbrun++) = static_cast<BYTE>(179);
			}
			else
			{
				intensity = static_cast<BYTE>((depth-minDepth) * 1.0 / (maxDepth-minDepth) * 255);
				// Write out blue byte
				*(rgbrun++) = intensity;
				// Write out green byte
				*(rgbrun++) = intensity;
				// Write out red byte
				*(rgbrun++) = intensity;
			}
			/*
			// Write out blue byte
			*(rgbrun++) = intensity;
			// Write out green byte
			*(rgbrun++) = intensity;
			// Write out red byte
			*(rgbrun++) = intensity;
			*/
#endif
			// We're outputting BGR, the last byte in the 32 bits is unused so skip it
			// If we were outputting BGRA, we would write alpha here.
			++rgbrun;

			// Increment our index into the Kinect's depth buffer
			++pBufferRun;
		}

		// Draw the data with Direct2D
		//SetDepthData(rgbrun, cScreenWidth * cScreenHeight * cBytesPerPixel);
		if ( cScreenWidth * cScreenHeight * cBytesPerPixel < ((cScreenHeight - 1) * sourceStride) + (cScreenWidth * 4) )
		{
			return E_INVALIDARG;
		}

		// Copy the image that was passed in into the direct2d bitmap
		hr = m_pBitmap->CopyFromMemory(NULL, m_pDepthRGBX, sourceStride);
		if ( FAILED(hr) )
		{
			return hr;
		}
		return hr;
	}
	return S_FALSE;
}

void CRenderDepthFrame::SetSkeletonFrame( NUI_SKELETON_FRAME skeletonFrame )
{
	m_RenderSkeleton.SetSkeletonFrame(skeletonFrame);
}

HRESULT CRenderDepthFrame::Draw(bool bHasDraw)
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
