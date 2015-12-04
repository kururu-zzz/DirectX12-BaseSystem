#ifndef TITLE___
#define TITLE___

#include "Sequence.h"

namespace DX12
{
	class Sprite;
	class Model;
	class Camera;
}

class Title : public Sequence
{
private:
	int time = 0;
	std::shared_ptr<DX12::Camera> camera;
	std::vector<std::shared_ptr<DX12::Sprite>> sprites;
	std::shared_ptr<DX12::Model> model;
	std::shared_ptr<DX12::Model> objet;
public:
	Title();
	void Init(const std::string& beforeSequenceName);
	bool Update(std::unordered_map<std::string, std::shared_ptr<Sequence>>* sequenceContainer);
	void Draw(ID3D12GraphicsCommandList* commandList);
};

#endif