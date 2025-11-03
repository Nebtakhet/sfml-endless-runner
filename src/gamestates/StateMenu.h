#pragma once

#include "IState.h"
#include <memory>

namespace sf { class Text; class Sprite; class Texture; };

class StateMenu : public IState
{
public:
	StateMenu(StateStack& stateStack);
	~StateMenu() = default;

	bool init() override;
	void update(float dt) override;
	void render(sf::RenderTarget& target) const override;

public:
	StateStack& m_stateStack;
	std::unique_ptr<sf::Text> m_pText;

	bool m_hasStartKeyBeenPressed = false;
	bool m_hasStartKeyBeenReleased = false;

	// Background image for the menu
	const sf::Texture* m_pBackgroundTexture = nullptr;
	std::unique_ptr<sf::Sprite> m_pBg;
};
