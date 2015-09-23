#include "fbxLoader.h"

#pragma comment( lib, "wininet.lib" )
#pragma comment( lib, "advapi32.lib" )
#if _DEBUG 
#pragma comment( lib, "libfbxsdk-md.lib" )
#else
#pragma comment( lib, "libfbxsdk-md.lib" )
#endif

namespace fbx
{
	namespace resource
	{
		std::shared_ptr<FbxScene> LoadModel(const std::string& fileName)
		{
			static std::shared_ptr<FbxManager> manager;
			if (manager == nullptr)
			{
				manager = std::shared_ptr<FbxManager>(FbxManager::Create(), [](FbxManager *p)
				{
					p->Destroy();
				});
				manager->SetIOSettings(FbxIOSettings::Create(manager.get(), IOSROOT));

				manager->LoadPluginsDirectory(FbxGetApplicationDirectory().Buffer(), "dll"); // windows‚Ì‚Ý‘Î‰ž
			}

			auto importer = std::shared_ptr<FbxImporter>(FbxImporter::Create(manager.get(), ""), [](FbxImporter* p)
			{
				p->Destroy();
			});
			if (!importer->Initialize(fileName.c_str(), -1, manager->GetIOSettings()))
				throw;

			auto scene = FbxScene::Create(manager.get(), fileName.c_str());

			importer->Import(scene);

			auto meshAxisSystem = scene->GetGlobalSettings().GetAxisSystem();

			FbxGeometryConverter(manager.get()).Triangulate(scene, true);
			return std::shared_ptr<FbxScene>(scene, [](FbxScene *p)
			{
				//p->Destroy();
			});
		}
	}
}