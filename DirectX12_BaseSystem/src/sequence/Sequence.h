#ifndef __SEQUENCE__
#define __SEQUENCE__

#include <unordered_map>
#include <string>
#include <memory>

struct ID3D12GraphicsCommandList;

class Sequence{
protected:
	enum class State
	{
		wait = 0x01,
		drawable = 0x02,
		updatable = 0x04,
		active = 0x06,
	};
protected:
	State state = State::wait;
	std::string sequenceName;
	std::string nextSequenceName;
	std::string beforeSequenceName;
	unsigned int priority = 0;
protected:
	template<typename T>
	static void EmplaceSequence(std::unordered_map<std::string, std::shared_ptr<Sequence>>* sequenceContainer)
	{
		std::string sequenceName = typeid(T).name();
		for (auto& info : *sequenceContainer)
		{
			if (info.first == sequenceName)
				return;
		}
		auto sequence = std::make_shared<T>();
		sequenceContainer->emplace(sequence->GetSequenceName(),sequence);
	}
public:
	Sequence(const std::string& sequenceName)
	{
		//ƒNƒ‰ƒX–¼‚Ì" *"‚ðÁ‹Ž‚µ‚Ä“o˜^
		this->sequenceName = sequenceName;
#ifdef _M_X64
		int loopNum = 10;
#else
		int loopNum = 3;
#endif
		for (int i = 0; i < loopNum;++i)
			this->sequenceName.pop_back();
	}
	virtual ~Sequence(){}
	virtual void Init(const std::string& beforeSequenceName) = 0;
	virtual bool Update(std::unordered_map<std::string, std::shared_ptr<Sequence>>* sequenceContainer) = 0;
	virtual void Draw(ID3D12GraphicsCommandList* commandList) = 0;
public:
	const unsigned int GetPriority() { return priority; }
	const std::string& GetSequenceName() const { return sequenceName; }
	bool isUpdatable(){ return ((static_cast<int>(state)& static_cast<int>(State::updatable)) == static_cast<int>(State::updatable)); }
	bool isDrawable() { return ((static_cast<int>(state)& static_cast<int>(State::drawable))  == static_cast<int>(State::drawable)); }
};

class SequenceManager{
private:
	std::unordered_map<std::string, std::shared_ptr<Sequence>> sequenceContainer;
public:
	SequenceManager();
	bool Update();
	void Render(ID3D12GraphicsCommandList* commandList);
};

#endif