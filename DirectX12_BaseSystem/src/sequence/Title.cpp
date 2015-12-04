#include "Title.h"
#include "../object/Sprite.h"
#include "../object/fbx/Model.h"
#include "../object/Camera.h"
#include "../core/ConstantBufferManager.h"
#include "../core/d3d.h"

Title::Title() : Sequence(typeid(this).name())
{
	camera = std::make_shared<DX12::Camera>();
	for (int i = 0; i < 12; ++i)
	{
		sprites.emplace_back(std::make_shared<DX12::Sprite>());
	}
	model = std::make_shared<DX12::Model>("resource/model/sphere.fbx");
	objet = std::make_shared<DX12::Model>("resource/model/objet.fbx");
}

void Title::Init(const std::string& beforeSequenceName)
{
	camera->Init(
		DirectX::XMFLOAT3(0.f, 5.f, -10.f),
		DirectX::XMFLOAT3(0.f, 0.f, 0.f));
	for (int i = 0; i < 12; ++i)
	{
		sprites.at(i)->Init(
			DirectX::XMFLOAT3((i % 3) * 300.f, (i / 4) * 300.f, 0.f),
			DirectX::XMFLOAT2(300.f, 300.f),
			DirectX::XMFLOAT4(0.f, 0.f, 1.f, 1.f),
			"resource/texture/sample2.dds"
			);
	}
	model->Init(
		DirectX::XMFLOAT3(0.f,1.f,0.f),
		DirectX::XMFLOAT3(2.f,1.f,1.f),
		DirectX::XMFLOAT3(0.f,0.f,30.f)
		);
	objet->Init();
	state = State::updatable;
}

bool Title::Update(std::unordered_map<std::string, std::shared_ptr<Sequence>>* sequenceContainer)
{
	state = State::active;
	return true;
}

void Title::Draw(ID3D12GraphicsCommandList* commandList)
{
	camera->SetConstantBuffer(commandList);
	DX12::ConstantBuffer::SetBuffer(commandList,DirectX::XMFLOAT4(0.f, 100.f, 0.f, 1.f) , DX12::ConstantBuffer::BufferSlot::Light);
	/*for (auto& sprite : sprites)
	{
		//sprite->Draw(commandList);
	}*/
	model->Draw(commandList);
	objet->Draw(commandList);
}

