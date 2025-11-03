#pragma once

#include "IState.h"
#include "entities/Player.h"
#include "entities/Enemy.h"
#include "entities/Fireball.h"
#include "entities/Orb.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

class StatePlaying : public IState
{
public:
	StatePlaying(StateStack& stateStack);
	~StatePlaying() = default;

	bool init() override;
	void update(float dt) override;
	void render(sf::RenderTarget& target) const override;
	void handleEvent(const sf::Event& event) override;

private:
	static constexpr const float enemySpawnInterval = 2.0f;
	float m_timeUntilEnemySpawn = enemySpawnInterval;

	StateStack &m_stateStack;
	std::unique_ptr<Player> m_pPlayer;
	std::vector<std::unique_ptr<Enemy>> m_enemies;
	std::vector<std::unique_ptr<Orb>> m_orbs;
	std::unique_ptr<Fireball> m_fireball;

	int m_orbsCollected = 0;

	// Instruction text shown at beginning of play for a few seconds
	std::unique_ptr<sf::Text> m_pText;
	float m_instructionTimeRemaining = 0.0f;
	
	sf::RectangleShape m_ground;
	bool m_hasPauseKeyBeenReleased = true;

	float m_fireChargeTime = 0.0f;


	void updateCollisions();
};
