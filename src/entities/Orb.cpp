#include "Orb.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <cmath>

bool Orb::init()
{
    const float radius = 12.0f;
    m_shape.setRadius(radius);
    m_shape.setFillColor(sf::Color(255, 215, 0, 215)); // gold/yellow for visibility
    m_shape.setOrigin({radius, radius});
    m_collisionRadius = radius;
    m_age = 0.0f;
    m_alive = true;
    m_shape.setPosition(m_position);
    return true;
}

void Orb::update(float dt)
{
    if (!m_alive)
        return;

    m_age += dt;
    if (m_age >= m_lifetime)
    {
        m_alive = false;
        return;
    }

    // simple floating effect
    float bob = std::sin(m_age * 6.0f) * 4.0f;
    sf::Vector2f pos = m_position;
    pos.y += bob;
    m_shape.setPosition(pos);
}

void Orb::render(sf::RenderTarget& target) const
{
    if (!m_alive)
        return;
    target.draw(m_shape);
}
