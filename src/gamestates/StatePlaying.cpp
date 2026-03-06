#include "StatePlaying.h"
#include "StatePaused.h"
#include "StateMenu.h"
#include "StateStack.h"

#include "ResourceManager.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "../entities/Player.h"
#include "../entities/Enemy.h"
#include "../entities/Fireball.h"
#include "../entities/Orb.h"
#include "../entities/Boss.h"


StatePlaying::StatePlaying(StateStack& stateStack)
	: m_stateStack(stateStack)
{
}

StatePlaying::~StatePlaying() = default;

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
	// If we're showing the win screen, count down and then return to menu
	if (m_showWinTimer > 0.0f)
	{
		m_showWinTimer -= dt;
		if (m_showWinTimer <= 0.0f)
		{
			m_showWinTimer = 0.0f;
			m_showWin = false;
			// End this playing state and go back to the menu
			m_stateStack.popDeferred();
			m_stateStack.push<StateMenu>();
		}
		// while showing win, freeze game logic (but still render)
		return;
	}

	m_timeUntilEnemySpawn -= dt;

	// Spawn enemies. When a boss is being invoked, make enemies come slightly faster and move faster
	float spawnInterval = enemySpawnInterval;
	float enemyBaseSpeed = 200.0f;
	if (m_bossInvoked)
	{
		spawnInterval = enemySpawnInterval * 0.95f;
		enemyBaseSpeed *= 1.05f; 
	}

	if (m_timeUntilEnemySpawn < 0.0f)
	{
		m_timeUntilEnemySpawn = spawnInterval;
		std::unique_ptr<Enemy> pEnemy = std::make_unique<Enemy>();
		pEnemy->setPosition(sf::Vector2f(1000, Entity::getGroundY()));
		if (pEnemy->init())
		{
			pEnemy->setSpeed(enemyBaseSpeed);
			m_enemies.push_back(std::move(pEnemy));
		}
	}

	bool isPauseKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
	m_hasPauseKeyBeenReleased |= !isPauseKeyPressed;
	if (m_hasPauseKeyBeenReleased && isPauseKeyPressed)
	{
		m_hasPauseKeyBeenReleased = false;
		m_stateStack.push<StatePaused>();
	}

	m_pPlayer->update(dt);

	// If boss has been invoked, count down to spawn
	if (m_bossInvoked)
	{
		if (m_bossTimer > 0.0f)
		{
			m_bossTimer -= dt;
			if (m_bossTimer <= 0.0f && !m_boss)
			{
				// spawn boss near right side above ground
				auto boss = std::make_unique<Boss>();
				boss->setPosition(sf::Vector2f(1000.0f, Entity::getGroundY() - 100.0f));
				if (boss->init())
				{
					m_boss = std::move(boss);
					m_bossFireballTimer = 0.2f;
				}
			}
		}
	}

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

	// Update boss if present
	if (m_boss)
	{
		m_boss->update(dt);

		// Boss throws fireballs to the left
		m_bossFireballTimer -= dt;
		if (m_bossFireballTimer <= 0.0f)
		{
			m_bossFireballTimer = 1.2f;
			sf::Vector2f fireDir(-1.0f, -0.2f);
			float lenSq = fireDir.lengthSquared();
			if (lenSq > 0.0001f)
				fireDir /= std::sqrt(lenSq);

			sf::Vector2f spawnPos = m_boss->getPosition() + sf::Vector2f(-(m_boss->getCollisionRadius() + 12.0f), -16.0f);
			auto bossFireball = std::make_unique<Fireball>(spawnPos, fireDir, 260.0f, 4);
			bossFireball->addLifetime(4.0f);
			if (bossFireball->init())
				m_bossFireballs.push_back(std::move(bossFireball));
		}
	}

	for (auto it = m_bossFireballs.begin(); it != m_bossFireballs.end(); )
	{
		(*it)->update(dt);
		if (!(*it)->isAlive())
			it = m_bossFireballs.erase(it);
		else
			++it;
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

	for (const std::unique_ptr<Fireball>& fb : m_bossFireballs)
	{
		float distance = (m_pPlayer->getPosition() - fb->getPosition()).lengthSquared();
		float minDistance = std::pow(Player::collisionRadius + fb->getCollisionRadius(), 2.0f);
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

	// Fireball vs boss collisions
	if (m_fireball && m_boss)
	{
		float distance = (m_fireball->getPosition() - m_boss->getPosition()).lengthSquared();
		float minDistance = std::pow(m_fireball->getCollisionRadius() + m_boss->getCollisionRadius(), 2.0f);
		if (distance <= minDistance)
		{
			bool died = m_boss->takeDamage(m_fireball->getDamage());
			m_fireball->kill();
			if (died)
			{
				// boss died: remove it and start win timer to show 'YOU WIN!'
				m_boss.reset();
				m_bossFireballs.clear();
				m_showWin = true;
				m_showWinTimer = 3.0f; // show for 3 seconds
				std::cout << "Boss defeated! YOU WIN!" << std::endl;
			}
			// consume fireball whether boss died or not
			if (m_fireball && !m_fireball->isAlive())
				m_fireball.reset();
		}
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
					if (m_orbsCollected < 10)
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

	// Render boss if present
	if (m_boss)
		m_boss->render(target);

	for (const std::unique_ptr<Fireball>& fb : m_bossFireballs)
		fb->render(target);

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

	// If showing win message, draw it on top
	if (m_showWinTimer > 0.0f)
	{
		const sf::Font* pFont = ResourceManager::getOrLoadFont("Lavigne.ttf");
		if (pFont)
		{
			std::string winStr = "YOU WIN!";
			sf::Text winText(*pFont, winStr, 96);
			winText.setStyle(sf::Text::Bold);
			winText.setFillColor(sf::Color::Yellow);
			winText.setOutlineColor(sf::Color::Black);
			winText.setOutlineThickness(4.0f);
			sf::FloatRect wb = winText.getLocalBounds();
			winText.setOrigin(sf::Vector2f(wb.size.x / 2.0f, wb.size.y / 2.0f));
			sf::Vector2f viewSize = target.getView().getSize();
			winText.setPosition(sf::Vector2f(viewSize.x * 0.5f, viewSize.y * 0.5f));
			target.draw(winText);
		}
	}

	// Simple centered counter at the top: "Orb: X/10"
	{
		const sf::Font* pFont = ResourceManager::getOrLoadFont("Lavigne.ttf");
		if (pFont)
		{
			int displayCount = std::min(m_orbsCollected, 10);
			std::string hudString = "Orbs: " + std::to_string(displayCount) + "/10";

			sf::Text hudText(*pFont, hudString, 20);
			hudText.setCharacterSize(20);
			hudText.setStyle(sf::Text::Bold);
			hudText.setFillColor(sf::Color::White);
			hudText.setOutlineColor(sf::Color::Black);
			hudText.setOutlineThickness(1.0f);

			sf::Vector2f viewSize = target.getView().getSize();
			sf::FloatRect tb = hudText.getLocalBounds();
			float centerX = viewSize.x * 0.5f;
			float y = 12.0f + hudText.getCharacterSize() * 0.5f;
			hudText.setOrigin(sf::Vector2f(tb.size.x / 2.0f, tb.size.y / 2.0f));
			hudText.setPosition(sf::Vector2f(centerX, y));
			target.draw(hudText);

			// If we've collected the maximum orbs and haven't invoked the boss yet, show a larger centered prompt
			if (displayCount >= 10 && !m_bossInvoked)
			{
				std::string promptStr = "Press 'F' to invoke Boss";
				sf::Text prompt(*pFont, promptStr, 36);
				prompt.setStyle(sf::Text::Bold);
				prompt.setFillColor(sf::Color::White);
				prompt.setOutlineColor(sf::Color::Black);
				prompt.setOutlineThickness(2.0f);

				sf::FloatRect pb = prompt.getLocalBounds();
				prompt.setOrigin(sf::Vector2f(pb.size.x / 2.0f, pb.size.y / 2.0f));
				sf::Vector2f viewSize = target.getView().getSize();
				prompt.setPosition(sf::Vector2f(viewSize.x * 0.5f, viewSize.y * 0.5f));
				target.draw(prompt);
			}
		}
	}
}

void StatePlaying::handleEvent(const sf::Event& event)
{
	if (m_pPlayer)
		m_pPlayer->handleEvent(event);

	// Handle boss invocation
	if (event.is<sf::Event::KeyPressed>())
	{
		if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
		{
			if (kp->code == sf::Keyboard::Key::F)
			{
				if (!m_bossInvoked && m_orbsCollected >= 10)
				{
					m_bossInvoked = true;
					m_bossTimer = 1.5f; // seconds until boss spawns
				}
			}
		}
	}
}
