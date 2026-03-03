#pragma once

#include <span>

#include "input_buttons.hpp"
#include "math.hpp"

class InputResponder {
public:
	virtual ~InputResponder() = default;
	virtual void notify(InputButton input, bool value) = 0;
	virtual void clearState() = 0;
	virtual void update() = 0;
	[[nodiscard]] virtual std::span<const InputButton> bindings() const = 0;
};

class InputAction final : public InputResponder {
public:
	InputAction() = default;
	explicit InputAction(InputButton binding) : binding(binding) {}

	virtual void notify(InputButton input, bool value) override;
	virtual void clearState() override;
	virtual void update() override;
	[[nodiscard]] virtual std::span<const InputButton> bindings() const override;

	[[nodiscard]] bool isPressed() const { return m_value && !m_prevFrameValue; }
	[[nodiscard]] bool isReleased() const { return !m_value && m_prevFrameValue; }
	[[nodiscard]] bool isDown() const { return m_value; }

public:
	InputButton binding = InputButton::None;

private:
	bool m_value = false;
	bool m_prevFrameValue = false;
};

class InputAxis1D final : public InputResponder {
public:
	explicit InputAxis1D(InputButton bindingPos, InputButton bindingNeg) : bindingPos(bindingPos), bindingNeg(bindingNeg) {}

	virtual void notify(InputButton input, bool value) override;
	virtual void clearState() override;
	virtual void update() override {};
	[[nodiscard]] virtual std::span<const InputButton> bindings() const override;

	[[nodiscard]] float getValue() const { return (m_valuePos ? 1.0f : 0.0f) - (m_valueNeg ? 1.0f : 0.0f); }

public:
	InputButton bindingPos = InputButton::None;
	InputButton bindingNeg = InputButton::None;

private:
	bool m_valuePos = false;
	bool m_valueNeg = false;
};

class InputAxis2D final : public InputResponder {
public:
	explicit InputAxis2D(InputButton bindingUp, InputButton bindingDown, InputButton bindingLeft, InputButton bindingRight) :
	    bindingUp(bindingUp), bindingDown(bindingDown), bindingLeft(bindingLeft), bindingRight(bindingRight) {}

	virtual void notify(InputButton input, bool value) override;
	virtual void clearState() override;
	virtual void update() override {};
	[[nodiscard]] virtual std::span<const InputButton> bindings() const override;

	[[nodiscard]] glm::vec2 getValue() const {
		return glm::vec2((m_valueRight ? 1.0f : 0.0f) - (m_valueLeft ? 1.0f : 0.0f), (m_valueDown ? 1.0f : 0.0f) - (m_valueUp ? 1.0f : 0.0f));
	}

public:
	InputButton bindingUp = InputButton::None;
	InputButton bindingDown = InputButton::None;
	InputButton bindingLeft = InputButton::None;
	InputButton bindingRight = InputButton::None;

private:
	bool m_valueUp = false;
	bool m_valueDown = false;
	bool m_valueLeft = false;
	bool m_valueRight = false;
};
