#pragma once

#include <memory>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace sf { class RenderTarget; };

class Entity
{
public:
    Entity() = default;
    Entity(const sf::Vector2f& position, const sf::Angle& rotation);

    virtual bool init() = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderTarget& target) const = 0;

    const sf::Vector2f& getPosition() const { return m_position; }
    void setPosition(const sf::Vector2f& position) { m_position = position; };

    const sf::Angle& getRotation() const { return m_rotation; }
    void setRotation(const sf::Angle& rotation) { m_rotation = rotation; };

    float getCollisionRadius() const { return m_collisionRadius; }

    int getHealth() const { return m_health; }
    void setHealth(int health) { m_health = health; }

protected:
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    sf::Angle m_rotation;
    float m_collisionRadius = 0.0f;
    std::unique_ptr<sf::Sprite> m_pSprite;
    int m_health = 0;
};
