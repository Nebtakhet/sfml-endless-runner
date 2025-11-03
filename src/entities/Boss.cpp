#include "Boss.h"
#include "ResourceManager.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <SFML/Graphics/RenderTarget.hpp>

bool Boss::init()
{
	const sf::Texture* pTexture = ResourceManager::getOrLoadTexture("Boss.png");
	if (pTexture == nullptr)
		return false;
	
	m_pSprite = std::make_unique<sf::Sprite>(*pTexture);
	if (!m_pSprite)
		return false;

	sf::FloatRect localBounds = m_pSprite->getLocalBounds();
	m_pSprite->setOrigin(sf::Vector2f(localBounds.size.x / 2.0f, localBounds.size.y / 2.0f));
	m_pSprite->setPosition(m_position);

	const sf::Vector2f spriteScale = sf::Vector2f(0.4f, 0.4f);
	m_pSprite->setScale(spriteScale);

	float rawRadius = std::max(localBounds.size.x * spriteScale.x, localBounds.size.y * spriteScale.y) * 0.5f;
	m_collisionRadius = rawRadius * 0.75f;

	// Initialize base Y so bobbing is calculated around the spawn point
	m_baseY = m_position.y;

	setHealth(30);

	return true;
}

void Boss::update(float dt)
{
	// Update internal timer and bob up/down around base Y
	m_time += dt;
	const float bobFreq = 2.0f;
	const float bobAmp = 8.0f;
	float bob = std::sin(m_time * bobFreq) * bobAmp;
	m_position.y = m_baseY + bob;

}

void Boss::render(sf::RenderTarget& target) const
{
	m_pSprite->setPosition(m_position);
	target.draw(*m_pSprite);
}