#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "input_buttons.hpp"
#include "input_responder.hpp"

class Input {
public:
	void notifyButtonPress(InputButton button);
	void notifyButtonRelease(InputButton button);

	void update();

	void add(unsigned id, std::unique_ptr<InputResponder> responder);
	const InputAction* getAction(unsigned id) const;
	const InputAxis1D* getAxis1D(unsigned id) const;
	const InputAxis2D* getAxis2D(unsigned id) const;

private:
	struct QueuedButton {
		InputButton id;
		bool state;
	};

private:
	std::vector<std::unique_ptr<InputResponder>> m_responders;
	std::unordered_map<unsigned, InputResponder*> m_responderIdMap;
	std::unordered_map<InputButton, std::vector<InputResponder*>> m_responderInputMap;

	std::mutex m_inputMutex;
	std::vector<QueuedButton> m_inputButtonQueue;
};
