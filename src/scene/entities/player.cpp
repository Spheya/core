#include "player.hpp"
#include "input/input_ids.hpp"
#include "rendering/graphics_context.hpp"
#include "rendering/surface_manager.hpp"

constexpr static float speed = 500.0f;
constexpr static float friction = 18.0f;
constexpr static float jumpHeight = 500.0f;
constexpr static float jumpDuration = 1.0f;

constexpr static float jumpForce = 4.0f * jumpHeight / jumpDuration;
constexpr static float gravity = 2.0f * jumpForce / jumpDuration;

Player::Player(CharacterAnimations animations, const Input* input) :
    m_movementInput(input ? input->getAxis1D(InputId_PlayerMovement) : nullptr),
    m_jumpInput(input ? input->getAction(InputId_PlayerJump) : nullptr),
    m_duckInput(input ? input->getAction(InputId_PlayerDuck) : nullptr),
    m_velocity(0.0f),
    m_flipped(false),
    m_animator(std::move(animations)) {
	localPhysicsBounds = { .min = glm::vec2(-10.0f, 16.0f), .max = glm::vec2(10.0f, 48.0f) };
}

void Player::setInput(const Input* input) {
	m_movementInput = input ? input->getAxis1D(InputId_PlayerMovement) : nullptr;
	m_jumpInput = input ? input->getAction(InputId_PlayerJump) : nullptr;
	m_duckInput = input ? input->getAction(InputId_PlayerDuck) : nullptr;
}

void Player::onUpdate(const Time& time) {
	updateClickableRegion();

	float inputDir = m_movementInput ? m_movementInput->getValue() : 0.0f;
	bool jump = m_jumpInput ? m_jumpInput->isDown() : false;
	// bool jumpPress = m_jumpInput ? m_jumpInput->isPressed() : false;
	// bool duck = m_duckInput ? m_duckInput->isDown() : false;

	float horAcc = inputDir * speed * friction;
	float fric = m_velocity.x * std::min(friction * time.deltaTime(), 1.0f);

	glm::vec2 prevVelocity = m_velocity;

	if(jump && m_grounded) {
		m_velocity.y = -jumpForce;
		prevVelocity.y -= jumpForce; // Do not interpolate jumping
	}

	m_velocity.x -= fric;
	m_velocity.x += horAcc * time.deltaTime();
	m_velocity.y += gravity * time.deltaTime();

	move((m_velocity + prevVelocity) * 0.5f * time.deltaTime());

	if(m_grounded) {
		if(inputDir != 0.0f) {
			m_animator.setAnimation(CharacterAnimation::Run);
			if(inputDir < 0.0f) {
				m_flipped = true;
			} else if(inputDir > 0.0f) {
				m_flipped = false;
			}
		} else {
			m_animator.setAnimation(CharacterAnimation::Idle);
		}
	} else {
		if(m_velocity.y < 0.0f) {
			m_animator.setAnimation(CharacterAnimation::Jump);
		} else {
			m_animator.setAnimation(CharacterAnimation::Fall);
		}
	}

	m_animator.update(time);

	m_sprite.sprite = m_animator.getCurrentFrame();
	m_sprite.matrix =
	    glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f)), glm::vec3(m_flipped ? -96.0f : 96.0f, 96.0f, 1.0f));
}

std::span<const SpriteDrawable> Player::getSprites() const {
	return std::span<const SpriteDrawable>(&m_sprite, 1);
}

void Player::updateClickableRegion() {
	BoundingBox clickBounds = { .min = glm::vec2(-32.0f, -8.0f) + position, .max = glm::vec2(32.0f, 48.0f) + position };
#ifdef _DEBUG
	GraphicsContext::getInstance().getDebugRenderer().box(clickBounds);
#endif
	if(SurfaceManager::getInstance().canPushClickableRegion()) SurfaceManager::getInstance().pushClickableRegion(clickBounds);
}

void Player::move(glm::vec2 delta) {
	m_grounded = false;

	float movLength = glm::length(delta);
	glm::vec2 movDirection = delta / movLength;

	Intersection hit = scene->boxCast(getPhysicsBounds(), movDirection, movLength, ~0u, this);
	if(hit.distance != movLength) {
		position += movDirection * std::max(hit.distance - 0.25f, 0.0f);
		m_velocity -= glm::dot(hit.normal, m_velocity) * hit.normal;

		if(hit.normal.y < -0.5f) m_grounded = true;

		delta -= glm::dot(hit.normal, delta) * hit.normal;
		if(dot(delta, delta) > 0.1f) {
			movLength = glm::length(delta);
			movDirection = delta / movLength;

			hit = scene->boxCast(getPhysicsBounds(), movDirection, movLength, ~0u, this);
			if(hit.distance != movLength) {
				position += movDirection * std::max(hit.distance - 0.25f, 0.0f);
				m_velocity -= glm::dot(hit.normal, m_velocity) * hit.normal;

				if(hit.normal.y < -0.5f) m_grounded = true;
			} else {
				position += movDirection * hit.distance;
			}
		}
	} else {
		position += movDirection * hit.distance;
	}
}
