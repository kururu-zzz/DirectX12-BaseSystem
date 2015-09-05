#ifndef TEXTURECONTAINER___
#define TEXTURECONTAINER___

#include <unordered_map>
#include <string>
#include <memory>

struct ID3D12GraphicsCommandList;
struct D3D12_SUBRESOURCE_DATA;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;

namespace DX12
{
	class TextureContainer
	{
	private:
		std::shared_ptr<ID3D12GraphicsCommandList> commandList;
	private:
		std::string lastSetSRVName;
		using UINT8 = unsigned char;
		std::unordered_map<std::string, std::vector<UINT8>> textureContainer;
		std::unordered_map<std::string, D3D12_SUBRESOURCE_DATA> subResourceContainer;
		std::unordered_map<std::string, std::shared_ptr<ID3D12Resource>> textureResourceContainer;
		std::unordered_map<std::string, std::shared_ptr<ID3D12Resource>> heapResourceContainer;
		std::unordered_map<std::string, std::shared_ptr<ID3D12DescriptorHeap>> srvContainer;
	private:
		TextureContainer() {}
		TextureContainer(const TextureContainer& rhs);
		TextureContainer& operator=(const TextureContainer& rhs);
	public:
		static TextureContainer* GetInstance()
		{
			static TextureContainer msp_instance;
			return &msp_instance;
		}
		void Init(std::shared_ptr<ID3D12GraphicsCommandList> commandList);
		void CreateTexture(const std::string& fileName);
		void SetTexture(ID3D12GraphicsCommandList* bundle, const std::string& fileName, int textureNum);
		ID3D12DescriptorHeap* GetLastUseSRV()
		{
			auto srv = srvContainer.at(lastSetSRVName).get();
			return srv;
		}
	};

	inline void InitTextureContainer(std::shared_ptr<ID3D12GraphicsCommandList> commandList)
	{
		return TextureContainer::GetInstance()->Init(commandList);
	}

	inline void SetTexture(ID3D12GraphicsCommandList* bundle, const std::string& fileName, int slotNum = 0)
	{
		return TextureContainer::GetInstance()->SetTexture(bundle,fileName,slotNum);
	}

	inline void CreateTexture(const std::string& fileName)
	{
		TextureContainer::GetInstance()->CreateTexture(fileName);
	}

	inline ID3D12DescriptorHeap* GetLastUseSRV()
	{
		return TextureContainer::GetInstance()->GetLastUseSRV();
	}
}

#endif