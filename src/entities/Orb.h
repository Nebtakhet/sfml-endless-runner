#pragma once

#include "Entity.h"
#include <SFML/Graphics/CircleShape.hpp>

class Orb : public Entity
{
public:
    Orb() = default;
    virtual ~Orb() = default;

    bool init() override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) const override;

    bool isAlive() const { return m_alive; }
    void collect() { m_alive = false; }

private:
    sf::CircleShape m_shape;
    float m_lifetime = 10.0f; // seconds before auto-expire
    float m_age = 0.0f;
    bool m_alive = true;
};
