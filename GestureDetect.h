#pragma once

#include "NuiApi.h"
#include <deque>

enum GESTURE_TYPE
{
	GESTURE_NONE = 0,
	GESTURE_ACTIVE,
	GESTURE_WAVE,
	GESTURE_STAY,
	GESTURE_TOLEFT,
	GESTURE_TORIGHT,
	GESTURE_TOTOP,
	GESTURE_TOBOTTOM,
	GESTURE_TOFRONT,
	GESTURE_TOBACK
};

const int cSkeletonFrameMax = 20;	//存储20个骨骼帧数据，据此算出当前手势

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
	void GetCurrentGesture(GESTURE_TYPE& GestureType, bool isLeftHand = true);

private:
	void JudgeGesture(NUI_SKELETON_POSITION_INDEX posIndex = NUI_SKELETON_POSITION_HAND_LEFT);

private:
	std::deque<NUI_SKELETON_FRAME>	m_dequeSkeletonFrame;
	GESTURE_TYPE					m_gestureType;
	NUI_SKELETON_POSITION_INDEX		m_skeletonPosIndex;
};

