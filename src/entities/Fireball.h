#pragma once

#include "Entity.h"
#include <memory>

class Fireball : public Entity
{
public:
	Fireball(const sf::Vector2f& pos, const sf::Vector2f& direction, float speed, int damage);
	virtual ~Fireball() = default;

	bool init() override;
	void update(float dt) override;
	void render(sf::RenderTarget& target) const override;

	int getDamage() const { return m_damage; }
	bool isAlive() const { return m_alive; }
	void kill() { m_alive = false; }

	// Charging API
	bool isCharging() const { return m_charging; }
	void setCharging(bool c) { m_charging = c; }

	// Allow damage to be adjusted while charging
	void setDamage(int dmg) { m_damage = dmg; }

	// Configure angular speed
	void setAngularSpeed(float degPerSec) { m_angularSpeedDeg = degPerSec; }

private:
	sf::Vector2f m_direction;
	sf::Vector2f m_velocity;
	float m_speed = 0.0f;
	float m_lifetime = 0.3f; // seconds
	float m_gravity = 400.0f;

	int m_damage = 0;
	bool m_alive = true;
	int m_bounces = 0;
	int m_maxBounces = 3;

	// When true, the fireball is attached to the player and growing in damage
	bool m_charging = false;

	// Rotation speed in degrees per second
	float m_angularSpeedDeg = 360.0f;
};
