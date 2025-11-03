#pragma once

#include "Entity.h"
#include <memory>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace sf { class Sprite; }

class Enemy final : public Entity
{
public:
	static constexpr float collisionRadius = 24.0f;

	Enemy() = default;
	virtual ~Enemy() = default;
	
	bool init() override;
	void update(float dt) override;
	void render(sf::RenderTarget& target) const override;
	
	// Health (HP) so levels can configure enemy durability
	int getHealth() const { return m_health; }
	void setHealth(int hp) { m_health = hp; }

private:
	int m_health = 3;
};
