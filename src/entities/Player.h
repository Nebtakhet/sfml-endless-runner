#pragma once

#include "Entity.h"
#include <memory>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/Graphics/Sprite.hpp>


namespace sf { class Sprite; }
namespace sf { class Event; }
class Fireball;

class Player final : public Entity
{
	public:
		static constexpr float collisionRadius = 42.0f;

		Player();
		virtual ~Player() = default;
		
		bool init() override;
		void update(float dt) override;
		void render(sf::RenderTarget& target) const override;

		void handleEvent(const sf::Event& event);

		bool isFacingRight() const { return facingRight; } // Boolean indicating if the player is facing right

		std::unique_ptr<Fireball> releaseFireball(); // Releases a charged fireball if requested

	private:
	
	// Animation parameters
		int frameCol = 4;
		int frameRow = 2;
		int currentCol = 0;
		int currentRow = 0;
		float frameTime = 0.1f; // Time per frame in seconds
		float animationTimer = 0.0f;
		bool facingRight = true;

	// Sprite scaling factors	
		float spriteScaleX = 0.25f;
		float spriteScaleY = 0.25f;

	// Jumping and gravity parameters	
		sf::Vector2f	velocity{0.0f, 0.0f};
		float gravity = 400.0f; // pixels per second squared
		float jumpStrength = 350.0f; // initial jump velocity
		bool onGround = true;

	// Movement parameters. Pixels per second squared	
		float acceleration = 100.0f;
		float maxSpeed = 100.0f; 
		float friction = 1000.0f; 

	// Input flags
		bool isMovingLeft = false;
		bool isMovingRight = false;
		bool hitPressed = false;
		bool m_isJumping = false;

	// Fire/charge state
		bool m_fireRequested = false;
		float m_charge = 0.0f; // accumulated charge time in seconds
		float m_maxChargeTime = 3.0f; // clamp for charge
		int m_maxDamage = 10; // max damage from a fully charged shot

};
