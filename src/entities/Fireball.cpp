#include "Fireball.h"
#include "ResourceManager.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Angle.hpp>

Fireball::Fireball(const sf::Vector2f& pos, const sf::Vector2f& direction, float speed, int damage)
{
	m_position = pos;
	m_direction = direction;
	m_speed = speed;
	m_damage = damage;
	m_velocity = m_direction * m_speed;
	m_collisionRadius = 12.0f;
	// Increase lifetime slightly based on damage: each damage point adds 0.05s
	m_lifetime += static_cast<float>(m_damage) * 0.05f;
}

bool Fireball::init()
{
	const sf::Texture* pTexture = ResourceManager::getOrLoadTexture("fireball.png");
	if (pTexture == nullptr)
		return false;

	m_pSprite = std::make_unique<sf::Sprite>(*pTexture);
	if (!m_pSprite)
		return false;

	m_rotation = sf::degrees(0);
	sf::FloatRect localBounds = m_pSprite->getLocalBounds();

	m_pSprite->setOrigin({ localBounds.size.x / 2.0f, localBounds.size.y / 2.0f });
	m_pSprite->setPosition(m_position);

	sf::Vector2f spriteScale = { 0.01f, 0.01f };
	m_pSprite->setScale(spriteScale);

	return true;
}

void Fireball::update(float dt)
{
    // advance rotation
    m_rotation += sf::degrees(m_angularSpeedDeg * dt);

	// Apply gravity
	m_velocity.y += m_gravity * dt;

	// Update position
	m_position += m_velocity * dt;

	// Ground interaction (simple bounce)
	if (m_position.y >= Entity::getGroundY())
	{
		m_position.y = Entity::getGroundY();
		if (m_bounces < m_maxBounces)
		{
			m_velocity.y = -m_velocity.y * 0.6f;
			m_bounces++;
		}
		else
		{
			m_alive = false;
		}
	}

	m_lifetime -= dt;
	if (m_lifetime <= 0.f)
		m_alive = false;
}

void Fireball::render(sf::RenderTarget& target) const
{
	if (m_pSprite && m_alive)
	{
		m_pSprite->setPosition(m_position);
		m_pSprite->setRotation(m_rotation);
		target.draw(*m_pSprite);
	}
}
