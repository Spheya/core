#include "player.hpp"

#include <cmath>

#include "input/input_ids.hpp"
#include "rendering/graphics_context.hpp"
#include "rendering/sprite_atlas.hpp"
#include "rendering/surface_manager.hpp"

constexpr static float speed = 500.0f;
constexpr static float friction = 18.0f;
constexpr static float slideFriction = 4.0f;
constexpr static float airFriction = 4.0f;
constexpr static float jumpHeight = 250.0f;
constexpr static float duckJumpHeight = 500.0f;
constexpr static float jumpDuration = 0.8f;
constexpr static float slideJumpSpeed = 2000.0f;
constexpr static float slideJumpHeight = 125.0f;
constexpr static float slideVelocityMult = 3.5f;
constexpr static float slideCooldown = 0.25f;
constexpr static float downwardsGravityMult = 1.2f;
constexpr static float releaseGravityMult = 3.0f;

constexpr static float slideBufferTime = 0.25f;
constexpr static float duckJumpBufferTime = 0.25f;
constexpr static float slideJumpBufferTime = 0.25f;
constexpr static float jumpBuffertime = 0.25f;
constexpr static float coyoteTime = 0.25f;

constexpr static float jumpForce = 4.0f * jumpHeight / jumpDuration;
constexpr static float gravity = 2.0f * jumpForce / jumpDuration;
const static float duckJumpForce = std::sqrt(2.0f * gravity * duckJumpHeight); // sqrt isnt constexpr were cooked
const static float slideJumpForce = std::sqrt(2.0f * gravity * slideJumpHeight);

Player::Player(CharacterAnimations animations, const Input* input) :
    m_input(input),
    m_movementInput(input ? input->getAxis1D(InputId_PlayerMovement) : nullptr),
    m_jumpInput(input ? input->getAction(InputId_PlayerJump) : nullptr),
    m_duckInput(input ? input->getAction(InputId_PlayerDuck) : nullptr),
    m_velocity(0.0f),
    m_slideCooldown(0.0f),
    m_slideBuffer(0.0f),
    m_duckJumpBuffer(0.0f),
    m_slideJumpBuffer(0.0f),
    m_jumpBuffer(0.0f),
    m_coyoteTime(0.0f),
    m_isDucked(false),
    m_flipped(false),
    m_animator(std::move(animations)),
    m_squisher(0.25f, 5.0f, 14.0f, glm::vec2(0.0f, 0.5f)) {
	const Sprite clickAnim[] = { SpriteAtlas::getInstance().get("mouse.png"), SpriteAtlas::getInstance().get("mouse_left.png") };
	m_clickAnimation = Animation(clickAnim, 24, 8); // todo: not hardcode this or sth

	localPhysicsBounds = { .min = glm::vec2(-10.0f, 16.0f), .max = glm::vec2(10.0f, 48.0f) };
}

void Player::setInput(const Input* input) {
	m_input = input;
	m_movementInput = input ? input->getAxis1D(InputId_PlayerMovement) : nullptr;
	m_jumpInput = input ? input->getAction(InputId_PlayerJump) : nullptr;
	m_duckInput = input ? input->getAction(InputId_PlayerDuck) : nullptr;
}

void Player::onUpdate(const Time& time) {
	bool grounded = scene->boxCast(getPhysicsBounds(), glm::vec2(0.0f, 1.0f), 1.0f, ~0u, this).distance != 1.0f;
	m_slideCooldown -= time.deltaTime();
	m_slideBuffer -= time.deltaTime();
	m_duckJumpBuffer -= time.deltaTime();
	m_slideJumpBuffer -= time.deltaTime();
	m_jumpBuffer -= time.deltaTime();
	m_coyoteTime -= time.deltaTime();

	float inputDir = m_movementInput ? m_movementInput->getValue() : 0.0f;
	bool jump = m_jumpInput ? m_jumpInput->isDown() : false;
	bool jumpPressed = m_jumpInput ? m_jumpInput->isPressed() : false;
	bool duck = m_duckInput ? m_duckInput->isDown() : false;
	bool duckPressed = m_duckInput ? m_duckInput->isPressed() : false;
	bool slide = grounded && duck && std::abs(m_velocity.x) > 200.0f;

	// ducking
	if(m_isDucked != (duck && grounded)) {
		float intensity = slide ? 1.0f : 0.6f;
		if(m_isDucked) {
			m_squisher.squish(time, intensity);
		} else {
			m_squisher.squash(time, intensity);
		}
	}
	m_isDucked = duck && grounded;

	// buffering
	if(m_isDucked && !slide) m_duckJumpBuffer = duckJumpBufferTime;
	if(slide) m_slideJumpBuffer = slideJumpBufferTime;
	if(duckPressed) m_slideBuffer = slideBufferTime;
	if(jumpPressed) m_jumpBuffer = jumpBuffertime;
	if(grounded) m_coyoteTime = coyoteTime;

	// slide boost
	if(slide && m_slideBuffer > 0.0f && m_slideCooldown <= 0.0f) {
		m_slideBuffer = 0.0f;
		m_velocity.x *= slideVelocityMult;
		m_slideCooldown = slideCooldown;
	}

	// wahoo (jump)
	if((jump || m_jumpBuffer > 0.0f) && m_coyoteTime > 0.0f) {
		m_jumpBuffer = 0.0f;
		m_coyoteTime = 0.0f;

		float force = jumpForce;
		if(m_duckJumpBuffer > 0.0f && !slide) force = duckJumpForce;
		if(m_slideJumpBuffer > std::max(0.0f, m_duckJumpBuffer)) {
			m_velocity.x = glm::sign(m_velocity.x) * slideJumpSpeed;
			force = slideJumpForce;
		}

		m_velocity.y = -force;
		m_squisher.squish(time, 2.5f);
	}

	// select physics constants based on state
	float frictionCoefficient = friction;
	if(!grounded) frictionCoefficient = airFriction;
	if(m_isDucked) {
		frictionCoefficient = slideFriction;
		inputDir = 0.0f;
	}

	float gravityCoefficient = gravity;
	if(m_velocity.y > 0.0f) {
		gravityCoefficient *= downwardsGravityMult;
	} else {
		if(!jump) gravityCoefficient *= releaseGravityMult;
	}

	// move the rabbit
	glm::vec2 prevVelocity = m_velocity;                                                   // we do this after all instantanious forces are applied
	m_velocity.x += inputDir * speed * frictionCoefficient * time.deltaTime();             // movement
	m_velocity.x -= m_velocity.x * std::min(frictionCoefficient * time.deltaTime(), 1.0f); // friction
	m_velocity.y += gravityCoefficient * time.deltaTime();                                 // gravity
	move(time, (m_velocity + prevVelocity) * 0.5f * time.deltaTime());

	// update orientation of the sprite
	bool prevFlipped = m_flipped;
	if(inputDir < 0.0f) {
		m_flipped = true;
	} else if(inputDir > 0.0f) {
		m_flipped = false;
	}
	if(m_flipped != prevFlipped) m_squisher.squish(time);

	// select the correct animation for our bunny
	if(grounded) {
		if(m_isDucked) {
			if(slide) {
				m_animator.setAnimation(CharacterAnimation::Slide);
			} else {
				m_animator.setAnimation(CharacterAnimation::Duck);
			}
		} else {
			if(inputDir != 0.0f) {
				m_animator.setAnimation(CharacterAnimation::Run);
			} else {
				m_animator.setAnimation(CharacterAnimation::Idle);
			}
		}
	} else {
		if(m_velocity.y < 0.0f) {
			m_animator.setAnimation(CharacterAnimation::Jump);
		} else {
			m_animator.setAnimation(CharacterAnimation::Fall);
		}
	}

	m_animator.update(time);

	m_clickAnimation.sync(m_animator.getCurrentAnimation());
	m_clickAnimation.update(time);

	// update the visuals
	m_sprite.sprite = m_animator.getCurrentFrame();
	m_sprite.matrix =
	    glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f)), glm::vec3(m_flipped ? -96.0f : 96.0f, 96.0f, 1.0f))
	    * m_squisher.calcMatrix(time);

	m_clickSprite.sprite = m_clickAnimation.getCurrentFrame();
	m_clickSprite.matrix =
	    glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(position.x + 16.0f, position.y, 0.0f)), glm::vec3(32.0f, 32.0f, 1.0f));

	updateClickableRegion();
}

std::span<const SpriteDrawable> Player::getSprites() const {
	return std::span<const SpriteDrawable>(&m_sprite, (m_input && !m_input->hasFocus()) ? 2 : 1);
}

void Player::updateClickableRegion() {
	BoundingBox clickBounds = { .min = glm::vec2(-32.0f, -14.0f) + position, .max = glm::vec2(32.0f, 48.0f) + position };
#ifdef _DEBUG
	GraphicsContext::getInstance().getDebugRenderer().box(clickBounds);
#endif
	if(SurfaceManager::getInstance().canPushClickableRegion()) SurfaceManager::getInstance().pushClickableRegion(clickBounds);
}

void Player::move(const Time& time, glm::vec2 delta) {
	if(dot(delta, delta) < 0.001f) return;

	constexpr float err = 0.25f;

	float movLength = glm::length(delta);
	glm::vec2 movDirection = delta / movLength;
	float testLength = movLength + err;

	Intersection hit = scene->boxCast(getPhysicsBounds(), movDirection, testLength, ~0u, this);
	if(hit.distance == testLength) {
		position += movDirection * movLength;
	} else {
		position += movDirection * (hit.distance - err);
		onImpact(time, hit.normal);

		delta -= glm::dot(hit.normal, delta) * hit.normal;
		if(dot(delta, delta) > 0.001f) {
			movLength = glm::length(delta);
			movDirection = delta / movLength;
			testLength = movLength + err;

			hit = scene->boxCast(getPhysicsBounds(), movDirection, testLength, ~0u, this);
			if(hit.distance == testLength) {
				position += movDirection * movLength;
			} else {
				position += movDirection * (hit.distance - err);
				onImpact(time, hit.normal);
			}
		}
	}
}

void Player::onImpact(const Time& time, glm::vec2 normal) {
	glm::vec2 impactForce = glm::dot(normal, m_velocity) * normal;
	m_velocity -= impactForce;
	float intensity = glm::length(impactForce) / 800.0f;

	if(intensity < 0.5f) return;

	if(abs(normal.x) > abs(normal.y)) {
		m_squisher.squish(time, std::min(intensity, 2.0f));
	} else {
		m_squisher.squash(time, std::min(intensity, 3.0f));
	}
}
