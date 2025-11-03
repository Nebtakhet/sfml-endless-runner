#include "StatePlaying.h"
#include "StatePaused.h"
#include "StateStack.h"
#include "ResourceManager.h"
#include "../entities/Orb.h"
#include <memory>
#include <iostream>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <cmath>

StatePlaying::StatePlaying(StateStack& stateStack)
	: m_stateStack(stateStack)
{
}

bool StatePlaying::init()
{
	m_ground.setSize({1024.0f, 256.0f});
	m_ground.setPosition({0.0f, Entity::getGroundY()});
	m_ground.setFillColor(sf::Color::Green);

	(void)ResourceManager::getOrLoadTexture("fireball.png"); // Preload fireball texture

	m_pPlayer = std::make_unique<Player>();
	if (!m_pPlayer || !m_pPlayer->init())
		return false;

	m_pPlayer->setPosition(sf::Vector2f(200, Entity::getGroundY()));

	// Instruction text: show for a few seconds at the start of the playing state
	{
		const sf::Font* pFont = ResourceManager::getOrLoadFont("Lavigne.ttf");
		if (pFont != nullptr)
		{
			m_pText = std::make_unique<sf::Text>(*pFont);
			if (m_pText)
			{
				m_pText->setString("MOVE: ARROWS / WASD   |   FIRE: SPACE (HOLD TO CHARGE)");
				m_pText->setStyle(sf::Text::Bold);
				sf::FloatRect localBounds = m_pText->getLocalBounds();
				m_pText->setOrigin(sf::Vector2f(localBounds.size.x / 2.0f, localBounds.size.y / 2.0f));
				m_instructionTimeRemaining = 7.5f; // seconds
			}
		}
	}

	return true;
}

void StatePlaying::update(float dt)
{
	m_timeUntilEnemySpawn -= dt;

	if (m_timeUntilEnemySpawn < 0.0f)
	{
		m_timeUntilEnemySpawn = enemySpawnInterval;
		std::unique_ptr<Enemy> pEnemy = std::make_unique<Enemy>();
		pEnemy->setPosition(sf::Vector2f(1000, Entity::getGroundY()));
		if (pEnemy->init())
			m_enemies.push_back(std::move(pEnemy));
	}

	bool isPauseKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
	m_hasPauseKeyBeenReleased |= !isPauseKeyPressed;
	if (m_hasPauseKeyBeenReleased && isPauseKeyPressed)
	{
		m_hasPauseKeyBeenReleased = false;
		m_stateStack.push<StatePaused>();
	}

	m_pPlayer->update(dt);

	// If the player charged a shot and released, initiate a new fireball
	if (m_pPlayer)
	{
		auto newFb = m_pPlayer->releaseFireball();
		if (newFb)
		{
			// Only one fireball at a time
			if (!m_fireball)
			{
				newFb->setCharging(false);
				m_fireChargeTime = 0.0f;
				m_fireball = std::move(newFb);
			}
		}
	}

	// Charging: if a fireball is present and charging, grow it in power
	if (m_fireball && m_fireball->isCharging())
	{
		// advance charge timer
		static constexpr float fireMaxChargeTime = 10.0f; // seconds
		static constexpr int fireMaxDamage = 10;

		// Update charge time
		m_fireChargeTime += dt;
		if (m_fireChargeTime > fireMaxChargeTime)
			m_fireChargeTime = fireMaxChargeTime;

		// Update damage based on charge time
		float t = m_fireChargeTime / fireMaxChargeTime;
		int damage = 1 + static_cast<int>(std::floor(t * (fireMaxDamage - 1)));
		if (damage > m_fireball->getDamage())
			m_fireball->setDamage(damage);

		// Attach to player position while charging
		sf::Vector2f spawnPos = m_pPlayer->getPosition() + sf::Vector2f((m_pPlayer->isFacingRight() ? Player::collisionRadius + 8.f : -Player::collisionRadius - 8.f), -16.f);
		m_fireball->setPosition(spawnPos);
	}

	// Update launched fireball
	else if (m_fireball)
	{
		m_fireball->update(dt);
		if (!m_fireball->isAlive())
			m_fireball.reset();
	}

	for (const std::unique_ptr<Enemy>& pEnemy : m_enemies)
	{
		pEnemy->update(dt);
	}

	// Update orbs
	for (const std::unique_ptr<Orb>& o : m_orbs)
		o->update(dt);

	updateCollisions();

	// Instruction timer: decrement and clamp
	if (m_instructionTimeRemaining > 0.0f)
	{
		m_instructionTimeRemaining -= dt;
		if (m_instructionTimeRemaining < 0.0f)
			m_instructionTimeRemaining = 0.0f;
	}
}

void StatePlaying::updateCollisions()
{
	// Detect collisions: player vs enemies
	bool playerDied = false;
	for (const std::unique_ptr<Enemy>& pEnemy : m_enemies)
	{
		float distance = (m_pPlayer->getPosition() - pEnemy->getPosition()).lengthSquared();
		float minDistance = std::pow(Player::collisionRadius + pEnemy->getCollisionRadius(), 2.0f);

		if (distance <= minDistance)
		{
			playerDied = true;
			break;
		}
	}

	// End Playing State on player death
	if (playerDied)
		m_stateStack.popDeferred();

	// Fireball vs enemies collisions
	if (m_fireball)
	{
		for (auto it = m_enemies.begin(); it != m_enemies.end(); )
		{
			const std::unique_ptr<Enemy>& pEnemy = *it;
			float distance = (m_fireball->getPosition() - pEnemy->getPosition()).lengthSquared();
			float minDistance = std::pow(m_fireball->getCollisionRadius() + pEnemy->getCollisionRadius(), 2.0f);
			if (distance <= minDistance)
			{
				bool died = pEnemy->takeDamage(m_fireball->getDamage());
				m_fireball->kill();
				if (died)
				{
					// spawn an orb at the enemy position
					auto orb = std::make_unique<Orb>();
					orb->setPosition(pEnemy->getPosition());
					if (orb->init())
						m_orbs.push_back(std::move(orb));
					it = m_enemies.erase(it);
				}
				else
				{
					++it;
				}
				break; // fireball consumed
			}
			else
			{
				++it;
			}
		}
		if (m_fireball && !m_fireball->isAlive())
			m_fireball.reset();
	}

		// Player vs orb collisions
		for (auto it = m_orbs.begin(); it != m_orbs.end(); )
		{
			const std::unique_ptr<Orb>& o = *it;
			if (!o->isAlive())
			{
				it = m_orbs.erase(it);
				continue;
			}

			float distance = (m_pPlayer->getPosition() - o->getPosition()).lengthSquared();
			float minDistance = std::pow(Player::collisionRadius + o->getCollisionRadius(), 2.0f);
			if (distance <= minDistance)
			{
				// collected
				++m_orbsCollected;
				o->collect();
				it = m_orbs.erase(it);
			}
			else
			{
				++it;
			}
		}
}

void StatePlaying::render(sf::RenderTarget& target) const
{
	target.draw(m_ground);
	for (const std::unique_ptr<Enemy>& pEnemy : m_enemies)
		pEnemy->render(target);

	// draw orbs
	for (const std::unique_ptr<Orb>& o : m_orbs)
		o->render(target);
	if (m_fireball)
		m_fireball->render(target);
	m_pPlayer->render(target);

	// Draw instruction text for the first few seconds
	if (m_instructionTimeRemaining > 0.0f && m_pText)
	{
		sf::Text txt = *m_pText;
		sf::FloatRect localBounds = txt.getLocalBounds();
		txt.setOrigin(sf::Vector2f(localBounds.size.x / 2.0f, localBounds.size.y / 2.0f));
		auto viewSize = target.getView().getSize();
		txt.setPosition(sf::Vector2f(viewSize.x / 2.0f, viewSize.y / 5.0f));
		target.draw(txt);
	}
}

void StatePlaying::handleEvent(const sf::Event& event)
{
	if (m_pPlayer)
		m_pPlayer->handleEvent(event);
}
