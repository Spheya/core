#include "input_responder.hpp"

void InputAction::notify(InputButton input, bool value) {
	if(input == binding) { m_value = value; }
}

void InputAction::clearState() {
	m_value = false;
	m_prevFrameValue = false;
}

void InputAction::update() {
	m_prevFrameValue = m_value;
}

std::span<const InputButton> InputAction::bindings() const {
	return std::span(&binding, 1);
}

void InputAxis1D::notify(InputButton input, bool value) {
	if(input == bindingPos) m_valuePos = value;
	if(input == bindingNeg) m_valueNeg = value;
}

void InputAxis1D::clearState() {
	m_valuePos = false;
	m_valueNeg = false;
}

std::span<const InputButton> InputAxis1D::bindings() const {
	return std::span(&bindingPos, 2);
}

void InputAxis2D::notify(InputButton input, bool value) {
	if(input == bindingUp) m_valueUp = value;
	if(input == bindingDown) m_valueDown = value;
	if(input == bindingLeft) m_valueLeft = value;
	if(input == bindingRight) m_valueRight = value;
}

void InputAxis2D::clearState() {
	m_valueUp = false;
	m_valueDown = false;
	m_valueLeft = false;
	m_valueRight = false;
}

std::span<const InputButton> InputAxis2D::bindings() const {
	return std::span(&bindingUp, 4);
}
