#include "StatePlaying.h"
#include "StatePaused.h"
#include "StateStack.h"
#include "ResourceManager.h"
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

    m_pPlayer = std::make_unique<Player>();
    if (!m_pPlayer || !m_pPlayer->init())
        return false;

    m_pPlayer->setPosition(sf::Vector2f(200, Entity::getGroundY()));

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

    // If the player released a charged shot, take ownership of the returned fireball
    if (m_pPlayer)
    {
        auto newFb = m_pPlayer->releaseFireball();
        if (newFb)
        {
            // Only accept a new fireball if we don't already have one active.
            // If a fireball is already present, discard the newly created one
            // so the player cannot fire again until the existing projectile is gone.
            if (!m_fireball)
            {
                // ensure it's treated as an active projectile
                newFb->setCharging(false);
                m_fireChargeTime = 0.0f;
                m_fireball = std::move(newFb);
            }
        }
    }

    // Charging: if a fireball is present and charging, grow it and keep it attached to player
    if (m_fireball && m_fireball->isCharging())
    {
        // advance charge timer
        static constexpr float fireMaxChargeTime = 1.0f;
        static constexpr int fireMaxDamage = 10;
        m_fireChargeTime += dt;
        if (m_fireChargeTime > fireMaxChargeTime) m_fireChargeTime = fireMaxChargeTime;

        float t = m_fireChargeTime / fireMaxChargeTime;
        int damage = 1 + static_cast<int>(std::floor(t * (fireMaxDamage - 1)));
        if (damage > m_fireball->getDamage())
            m_fireball->setDamage(damage);

        // attach to player position while charging
        sf::Vector2f spawnPos = m_pPlayer->getPosition() + sf::Vector2f((m_pPlayer->isFacingRight() ? Player::collisionRadius + 8.f : -Player::collisionRadius - 8.f), -16.f);
        m_fireball->setPosition(spawnPos);
    }
    else if (m_fireball)
    {
        // Update active (launched) fireball
        m_fireball->update(dt);
        if (!m_fireball->isAlive())
            m_fireball.reset();
    }

    for (const std::unique_ptr<Enemy>& pEnemy : m_enemies)
    {
        pEnemy->update(dt);
    }

    // Detect collisions
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
                // Apply damage
                int newHealth = pEnemy->getHealth() - m_fireball->getDamage();
                pEnemy->setHealth(newHealth);
                m_fireball->kill();
                if (newHealth <= 0)
                    it = m_enemies.erase(it);
                else
                    ++it;
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
}

void StatePlaying::render(sf::RenderTarget& target) const
{
    target.draw(m_ground);
    for (const std::unique_ptr<Enemy>& pEnemy : m_enemies)
        pEnemy->render(target);
    if (m_fireball)
        m_fireball->render(target);
    m_pPlayer->render(target);
}

void StatePlaying::handleEvent(const sf::Event& event)
{
    if (m_pPlayer)
        m_pPlayer->handleEvent(event);
}
