#pragma once

#include "Entity.h"
#include <memory>

// forward-declare SFML types used by reference/pointer in headers
namespace sf { class Sprite; class RenderTarget; }

class Enemy final : public Entity
{
public:
	static constexpr float collisionRadius = 24.0f;

	Enemy() = default;
	virtual ~Enemy() = default;
	
	bool init() override;
	void update(float dt) override;
	void render(sf::RenderTarget& target) const override;
	
	// Set movement speed (pixels per second). Default is 200.
	void setSpeed(float s);
	float getSpeed() const;

private:
	float m_speed = 200.0f;
};
