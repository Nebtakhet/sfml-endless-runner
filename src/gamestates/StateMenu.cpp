#include "StateMenu.h"
#include "StatePlaying.h"
#include "StateStack.h"
#include "ResourceManager.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

StateMenu::StateMenu(StateStack& stateStack)
    : m_stateStack(stateStack)
{
    
}

bool StateMenu::init()
{
    const sf::Font* pFont = ResourceManager::getOrLoadFont("Lavigne.ttf");
    if (pFont == nullptr)
        return false;

    m_pText = std::make_unique<sf::Text>(*pFont);
    if (!m_pText)
        return false;

    m_pText->setString("PRESS <ENTER> TO PLAY");
    m_pText->setStyle(sf::Text::Bold);
    sf::FloatRect localBounds = m_pText->getLocalBounds();
    m_pText->setOrigin({localBounds.size.x / 2.0f, localBounds.size.y / 2.0f});

    return true;
}

void StateMenu::update(float dt)
{
    (void)dt;
    m_hasStartKeyBeenPressed |= sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    if (m_hasStartKeyBeenReleased)
    {
        m_hasStartKeyBeenPressed = false;
        m_hasStartKeyBeenReleased = false;
        m_stateStack.push<StatePlaying>();
    }
    m_hasStartKeyBeenReleased |= m_hasStartKeyBeenPressed && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
}

void StateMenu::render(sf::RenderTarget& target) const
{
    m_pText->setPosition({target.getSize().x * 0.5f, target.getSize().y * 0.5f});
    target.draw(*m_pText);
}
