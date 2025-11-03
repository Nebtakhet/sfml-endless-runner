#pragma once

#include <SFML/Window/Event.hpp>

namespace sf { class RenderTarget; };
class StateStack;
class IState
{
public:
	virtual ~IState() = default;

	virtual bool init() = 0;
	virtual void update(float dt) = 0;
	virtual void render(sf::RenderTarget& target) const = 0;
	
	// Added event handling method for states to override
	virtual void handleEvent(const sf::Event&) { }
};
