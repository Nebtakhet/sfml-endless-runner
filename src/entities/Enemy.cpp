#include "Enemy.h"
#include "ResourceManager.h"
#include <SFML/Graphics/RenderTarget.hpp>


bool Enemy::init()
{
	const sf::Texture* pTexture = ResourceManager::getOrLoadTexture("enemy.png");
	if (pTexture == nullptr)
		return false;

	m_pSprite = std::make_unique<sf::Sprite>(*pTexture);
	if (!m_pSprite)
		return false;

	sf::FloatRect localBounds = m_pSprite->getLocalBounds();
	m_pSprite->setOrigin(sf::Vector2f(localBounds.size.x / 2.0f, localBounds.size.y / 2.0f));
	m_pSprite->setPosition(m_position);
	m_pSprite->setScale(sf::Vector2f(2.5f, 2.5f));
	m_collisionRadius = collisionRadius;

	setHealth(3);

	return true;
}

void Enemy::update(float dt)
{
	m_position.x -= m_speed * dt;
}

void Enemy::setSpeed(float s)
{
	m_speed = s;
}

float Enemy::getSpeed() const
{
	return m_speed;
}

void Enemy::render(sf::RenderTarget& target) const
{
	m_pSprite->setPosition(m_position);
	target.draw(*m_pSprite);
}
