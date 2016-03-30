#pragma once
#include <KinectInteraction.h>

class CInteractionClient :	public INuiInteractionClient
{
public:
	CInteractionClient(void){};
	~CInteractionClient(void){};

	STDMETHOD(GetInteractionInfoAtLocation)(THIS_ DWORD skeletonTrackingId, NUI_HAND_TYPE handType, FLOAT x, FLOAT y, _Out_ NUI_INTERACTION_INFO *pInteractionInfo)
	{        
#if 0
		if(pInteractionInfo)
		{
			pInteractionInfo->IsPressTarget         = true;
			pInteractionInfo->PressTargetControlId  = 0;
			pInteractionInfo->PressAttractionPointX = 0.f;
			pInteractionInfo->PressAttractionPointY = 0.f;
			pInteractionInfo->IsGripTarget          = true;
			return S_OK;
		}
#endif
		return S_OK;
		return E_POINTER;
	}

	STDMETHODIMP_(ULONG)    AddRef()                                    { return 1;     }
	STDMETHODIMP_(ULONG)    Release()                                   { return 0;     }
	STDMETHODIMP            QueryInterface(REFIID riid, void **ppv)     { return S_OK;  }
};

