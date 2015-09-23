#ifndef FBXLOADER___
#define FBXLOADER___

#include <string>
#include <fbxsdk.h>
#include <memory>

namespace fbx
{
	namespace resource
	{
		std::shared_ptr<FbxScene> LoadModel(const std::string& fileName);
	}
};

#endif