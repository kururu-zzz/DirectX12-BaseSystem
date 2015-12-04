#include "Sequence.h"
#include "Title.h"
#include <future>

SequenceManager::SequenceManager()
{
	auto sequence = std::make_shared<Title>();
	sequenceContainer.emplace(sequence->GetSequenceName(), sequence);
	sequence->Init("");
}

bool SequenceManager::Update(){
	bool continueFlag = true;
	std::vector<Sequence*> updatableSequence;
	for (auto& sequence : sequenceContainer)
	{
		if (sequence.second->isUpdatable())
			updatableSequence.emplace_back(sequence.second.get());
	}
	for (auto& sequence : updatableSequence)
	{
		continueFlag &= sequence->Update(&sequenceContainer);
	}
	return continueFlag;
}

void SequenceManager::Render(ID3D12GraphicsCommandList* commandList){
	int drawableSequenceNum = 0;
	for (auto& sequence : sequenceContainer)
	{
		drawableSequenceNum += static_cast<int>(sequence.second->isDrawable());
	}
	std::vector<Sequence*> drawableSequence;
	drawableSequence.resize(drawableSequenceNum);
	for (auto& sequence : sequenceContainer)
	{
		if (sequence.second->isDrawable())
		{
			auto priority = sequence.second->GetPriority();
			priority = (priority >= drawableSequence.size() - 1) ? drawableSequence.size() - 1 : priority;
			while (drawableSequence[priority] != nullptr && priority != drawableSequence.size() - 1)
				priority++;
			if (drawableSequence[priority] != nullptr)
			{
				while (drawableSequence[priority] != nullptr && priority != 0)
					priority--;
			}
			if (drawableSequence[priority] == nullptr)
				drawableSequence[priority] = sequence.second.get();
		}
	}
	for (auto& sequence : drawableSequence)
	{
		if (sequence != nullptr)
			sequence->Draw(commandList);
	}
}