#pragma once

#include "IState.h"
#include <memory>
#include <vector>
#include <SFML/Graphics/RectangleShape.hpp> 

class StateStack;
class Player;
class Enemy;
class Fireball;
class Orb;
class Boss;
namespace sf { class Text; class RenderTarget; struct Event; }

class StatePlaying : public IState
{
public:
	StatePlaying(StateStack& stateStack);
	~StatePlaying();

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

	// Boss invocation state
	std::unique_ptr<Boss> m_boss;
	bool m_bossInvoked = false;
	float m_bossTimer = 0.0f; // countdown until boss spawns when invoked

	int m_orbsCollected = 0;

	// Win screen timer: when boss is defeated show a big message for this many seconds
	float m_showWinTimer = 0.0f;
	bool m_showWin = false;

	// Instruction text shown at beginning of play for a few seconds
	std::unique_ptr<sf::Text> m_pText;
	float m_instructionTimeRemaining = 0.0f;
	
	sf::RectangleShape m_ground;
	bool m_hasPauseKeyBeenReleased = true;

	float m_fireChargeTime = 0.0f;


	void updateCollisions();
};
