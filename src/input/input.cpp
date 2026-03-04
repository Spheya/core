#include "input.hpp"

#include <cassert>

#include "platform.hpp"

void Input::notifyButtonPress(InputButton button) {
	std::lock_guard lock(m_inputMutex);
	m_inputButtonQueue.emplace_back(button, true);
}

void Input::notifyButtonRelease(InputButton button) {
	std::lock_guard lock(m_inputMutex);
	m_inputButtonQueue.emplace_back(button, false);
}

void Input::notifyFocus(bool focus) {
	m_focus = focus;
	if(!focus)
		for(const auto& responder : m_responders) responder->clearState();
}

void Input::update() {
	std::lock_guard lock(m_inputMutex);

	for(auto& responder : m_responders) responder->update();

	for(auto& input : m_inputButtonQueue)
		for(auto* responder : m_responderInputMap[input.id]) responder->notify(input.id, input.state);

	m_inputButtonQueue.clear();

	POINT p;
	GetCursorPos(&p);
	m_mousePos = glm::vec2(p.x, p.y);
}

void Input::add(unsigned id, std::unique_ptr<InputResponder> responder) {
	assert(!m_responderIdMap.contains(id));
	for(auto binding : responder->bindings()) m_responderInputMap[binding].push_back(responder.get());
	m_responderIdMap.emplace(id, responder.get());
	m_responders.emplace_back(std::move(responder));
}

const InputAction* Input::getAction(unsigned id) const {
	auto it = m_responderIdMap.find(id);
	if(it == m_responderIdMap.end()) return nullptr;
	return static_cast<InputAction*>(it->second);
}

const InputAxis1D* Input::getAxis1D(unsigned id) const {
	auto it = m_responderIdMap.find(id);
	if(it == m_responderIdMap.end()) return nullptr;
	return static_cast<InputAxis1D*>(it->second);
}

const InputAxis2D* Input::getAxis2D(unsigned id) const {
	auto it = m_responderIdMap.find(id);
	if(it == m_responderIdMap.end()) return nullptr;
	return static_cast<InputAxis2D*>(it->second);
}
