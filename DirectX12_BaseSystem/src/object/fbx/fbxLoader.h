#ifndef FBXLOADER___
#define FBXLOADER___

#include <string>
#include <fbxsdk.h>
#include <memory>

namespace fbx
{
	namespace resource
	{
		FbxScene* LoadModel(const std::string& fileName);
	}
};

#endif