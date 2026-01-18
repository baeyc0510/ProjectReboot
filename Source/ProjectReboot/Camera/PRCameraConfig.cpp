// PRCameraConfig.cpp
#include "PRCameraConfig.h"


const FPRCameraTagSetting* UPRCameraConfig::GetHighestPrioritySetting(const FGameplayTagContainer& ActiveTags) const
{
	const FPRCameraTagSetting* BestSetting = nullptr;
	int32 HighestPriority = TNumericLimits<int32>::Min();

	for (const FPRCameraTagSetting& Setting : CameraSettings)
	{
		// 모든 조건 태그를 가지고 있는지 확인
		if (ActiveTags.HasAll(Setting.ConditionTags))
		{
			// 우선순위 비교
			if (Setting.Priority > HighestPriority)
			{
				BestSetting = &Setting;
				HighestPriority = Setting.Priority;
			}
		}
	}
	return BestSetting;
}
