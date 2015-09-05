#include "TextureContainer.h"
#include "DDSTextureLoader.h"
#include "../core/d3d.h"
#include <mutex>

namespace DX12
{
	static std::mutex mutex;

	void TextureContainer::Init(std::shared_ptr<ID3D12GraphicsCommandList> commandList)
	{
		this->commandList = commandList;
	}

	void TextureContainer::CreateTexture(const std::string& fileName)
	{
		std::lock_guard<std::mutex> lock(mutex);
		if ((fileName.size() - static_cast<int>(fileName.rfind(".dds")) == 4) && (textureContainer.find(fileName) == textureContainer.end() && fileName != ""))
		{
			UINT width = 0;
			UINT height = 0;
			std::vector<UINT8> textureData;
			DirectX::CreateTextureData(fileName, &textureData,&width,&height);

			textureContainer.emplace(fileName, textureData);

			textureResourceContainer.emplace(fileName,d3d::CreateTextureResoruce(width, height));

			heapResourceContainer.emplace(fileName, d3d::CreateResource(textureResourceContainer.at(fileName).get()));

			D3D12_SUBRESOURCE_DATA subResource = {};
			subResource.pData = &textureData[0];
			subResource.RowPitch = width * 4;
			subResource.SlicePitch = subResource.RowPitch * height;

			subResourceContainer.emplace(fileName, subResource);

			auto srvDescriptorHeap = d3d::CreateSRVDescriptorHeap();

			d3d::CreateShaderResourceView(textureResourceContainer.at(fileName).get(), srvDescriptorHeap.get());

			srvContainer.emplace(fileName, srvDescriptorHeap);

			d3d::UpdateSubresources(commandList.get(), textureResourceContainer.at(fileName).get(), heapResourceContainer.at(fileName).get(), 0, 0, 1, &subResourceContainer.at(fileName));
		}
	}

	void TextureContainer::SetTexture(ID3D12GraphicsCommandList* bundle, const std::string& fileName, int slotNum)
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (textureContainer.find(fileName) != textureContainer.end())
		{
			lastSetSRVName = fileName;
			auto srvDescriptorHeap = srvContainer.at(fileName).get();

			bundle->SetDescriptorHeaps(1, &srvDescriptorHeap);
			bundle->SetGraphicsRootDescriptorTable(1, srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		}
	}
}