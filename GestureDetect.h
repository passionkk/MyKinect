#pragma once

#include "NuiApi.h"
#include "KinectInteraction.h"
#include <deque>

#if 0
enum GESTURE_TYPE
{
	GESTURE_NONE = 0x00000001,
	GESTURE_ACTIVE = 0x00000002,
	GESTURE_WAVE = 0x00000004,
	GESTURE_STAY = 0x00000008,
	GESTURE_TOLEFT = 0x00000010,
	GESTURE_TORIGHT = 0x00000020,
	GESTURE_TOTOP = 0x00000040,
	GESTURE_TOBOTTOM = 0x00000080,
	GESTURE_TOFRONT = 0x00000100,
	GESTURE_TOBACK = 0x00000200
};
#endif

#define GESTURE_NONE	 0x00000001
#define GESTURE_ACTIVE	 0x00000002
#define GESTURE_WAVE	 0x00000004
#define GESTURE_STAY	 0x00000008
#define GESTURE_TOLEFT	 0x00000010
#define GESTURE_TORIGHT	 0x00000020
#define GESTURE_TOTOP	 0x00000040
#define GESTURE_TOBOTTOM 0x00000080
#define GESTURE_TOFRONT  0x00000100
#define GESTURE_TOBACK	 0x00000200

const int cSkeletonFrameMax = 20;	//存储20个骨骼帧数据，据此算出当前手势
const int cGestrueLimit = cSkeletonFrameMax / 2;

class CGestureDetect
{
public:
	CGestureDetect(void);
	~CGestureDetect(void);

public:
	//设置要判断的身体部位
	void SetJudgePos(NUI_SKELETON_POSITION_INDEX posIndex);
	//设置骨骼信息
	void SetSkeletonFrame(const NUI_SKELETON_FRAME& SkeletonFrame);
	//获取当前手势
	unsigned int GetCurrentGesture();

	bool IsInValidRange();
	//判断数据
	bool IsInValidRange(NUI_SKELETON_DATA data, NUI_SKELETON_POSITION_INDEX indexLimitSmaller, NUI_SKELETON_POSITION_INDEX indexLimitBigger, 
						NUI_SKELETON_POSITION_INDEX indexIn, bool bIsVertical = false);

	void GestureCallback(void(* GestureCalc)(unsigned int));
	CString GetStringFromGestrue(unsigned int gesture);

	void JudgeGesture(NUI_SKELETON_POSITION_INDEX posIndex = NUI_SKELETON_POSITION_HAND_LEFT);
	
	void SetHandPointerInfo(NUI_HANDPOINTER_INFO data, int nIndex = 0);
private:
	std::deque<NUI_SKELETON_DATA>	m_dequeSkeletonData;
	unsigned int					m_gestureType;
	NUI_SKELETON_POSITION_INDEX		m_skeletonPosIndex;
	NUI_HANDPOINTER_INFO			m_HandPointerInfo[2];
};

