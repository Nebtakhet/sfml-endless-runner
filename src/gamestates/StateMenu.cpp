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
    m_pText->setOrigin(sf::Vector2f(localBounds.size.x / 2.0f, localBounds.size.y / 2.0f));

    // Load menu background image
    m_pBackgroundTexture = ResourceManager::getOrLoadTexture("Start.png");
    if (m_pBackgroundTexture)
    {
        m_pBg = std::make_unique<sf::Sprite>(*m_pBackgroundTexture);
        if (m_pBg)
        {
            // Set origin to texture center
            sf::Vector2u ts = m_pBackgroundTexture->getSize();
            m_pBg->setOrigin(sf::Vector2f(static_cast<float>(ts.x) * 0.5f, static_cast<float>(ts.y) * 0.5f));
        }
    }

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
    // Draw background image (if loaded) scaled to fit the target while preserving aspect ratio
    if (m_pBg && m_pBackgroundTexture)
    {
        sf::Vector2u ts = m_pBackgroundTexture->getSize();
        float tw = static_cast<float>(ts.x);
        float th = static_cast<float>(ts.y);
        float vw = static_cast<float>(target.getSize().x);
        float vh = static_cast<float>(target.getSize().y);

        float wScale = vw / tw;
        float hScale = vh / th;
        float scale = std::min(wScale, hScale);

        m_pBg->setScale(sf::Vector2f(scale, scale));
        m_pBg->setPosition(sf::Vector2f(vw * 0.5f, vh * 0.5f));
        target.draw(*m_pBg);
    }

    m_pText->setPosition({target.getSize().x * 0.5f, target.getSize().y * 0.5f});
    target.draw(*m_pText);
}
