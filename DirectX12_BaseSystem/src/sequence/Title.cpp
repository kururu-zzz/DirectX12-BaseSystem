#include "Title.h"
#include "../object/Sprite.h"
#include <future>

Title::Title() : Sequence(typeid(this).name())
{
	sprite = std::make_shared<DX12::Sprite>();
}

void Title::Init(const std::string& beforeSequenceName)
{
	sprite->Init(
		DirectX::XMFLOAT3(0.f,0.f, 0.f),
		DirectX::XMFLOAT2(300.f, 300.f),
		DirectX::XMFLOAT4(0.f, 0.f, 1.f, 1.f),
		"resource\\texture\\sample2.dds"
		);
	state = State::updatable;
}

bool Title::Update(std::unordered_map<std::string, std::shared_ptr<Sequence>>* sequenceContainer)
{
	state = State::active;
	return true;
}

void Title::Draw(ID3D12GraphicsCommandList* commandList)
{
	sprite->Draw(commandList);
}

