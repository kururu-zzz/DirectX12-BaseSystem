#pragma comment( lib, "d3d12.lib" )

#include "d3d.h"
#include "dxgi.h"
#include <PlatformHelpers.h>

namespace asset
{
	void ReleaseIUnknown(IUnknown* p)
	{
		p->Release();
	}
}