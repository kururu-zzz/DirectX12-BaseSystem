#ifndef CONFIG___
#define CONFIG___

#include <vector>

namespace config
{
	static std::vector<std::pair<const UINT,const UINT>> wndResolution = 
	{
		{1200,  900 },
		{ 800,  600 },
		{1920, 1080 },
	};
	enum class FrameRate
	{
		fps_60 = 60,
		fps_30 = 30,
	};
}

#endif