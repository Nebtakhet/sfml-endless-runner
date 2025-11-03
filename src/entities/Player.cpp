#include "Player.h"
#include "Fireball.h"
#include "ResourceManager.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <cmath>
#include <SFML/Window/Event.hpp>

Player::Player()
{
}

bool Player::init()
{
    const sf::Texture* pTexture = ResourceManager::getOrLoadTexture("caveman.png");
    if (pTexture == nullptr)
        return false;

    m_pSprite = std::make_unique<sf::Sprite>(*pTexture);
    if (!m_pSprite)
        return false;

    m_rotation = sf::degrees(0);
    sf::FloatRect localBounds = m_pSprite->getLocalBounds();

	// Calculate frame dimensions for animation 
	int frameWidth = static_cast<int>(localBounds.size.x) / frameCol;
	int frameHeight = static_cast<int>(localBounds.size.y) / frameRow;

	m_pSprite->setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(frameWidth, frameHeight)));

    m_pSprite->setOrigin({frameWidth / 2.0f, frameHeight / 2.0f});
    m_pSprite->setPosition(m_position);

	sf::Vector2f spriteScale = {spriteScaleX, spriteScaleY};
    m_pSprite->setScale(spriteScale);
    m_collisionRadius = collisionRadius;

    return true;
}

void Player::update(float dt)
{
    // Charging logic
    if (hitPressed)
    {
        m_charge += dt;
        if (m_charge > m_maxChargeTime)
            m_charge = m_maxChargeTime;
    }
    // apply gravity if airborne
    if (!onGround)
    {
        velocity.y += gravity * dt;
    }

    // horizontal movement: acceleration when input
    float targetAcc = 0.f;
    if (isMovingLeft)
        targetAcc -= acceleration;
    if (isMovingRight)
        targetAcc += acceleration;

    // integrate horizontal velocity
    velocity.x += targetAcc * dt;

    // apply friction when no input
    if (!isMovingLeft && !isMovingRight)
    {
        if (velocity.x > 0.f)
        {
            velocity.x -= friction * dt;
            if (velocity.x < 0.f) velocity.x = 0.f;
        }
        else if (velocity.x < 0.f)
        {
            velocity.x += friction * dt;
            if (velocity.x > 0.f) velocity.x = 0.f;
        }
    }

    // clamp horizontal speed
    if (velocity.x > maxSpeed)
		velocity.x = maxSpeed;
    if (velocity.x < -maxSpeed)
		velocity.x = -maxSpeed;

    // integrate position
    m_position += velocity * dt;

    // world bounds and ground collision
    const float worldWidth = 1024.0f;
    // Player sprite baseline offset: original code used 775 while ground rect was 800.
    // Keep the same visual alignment by applying a small offset from Entity::groundY.
    const float groundY = Entity::getGroundY() - 25.0f;

    if (m_pSprite)
    {
        sf::Vector2u texSize = m_pSprite->getTexture().getSize();
        int fw = static_cast<int>(texSize.x) / frameCol;
        int fh = static_cast<int>(texSize.y) / frameRow;

		// world bounds
        float halfW = (fw * std::abs(m_pSprite->getScale().x)) * 0.5f;
        if (m_position.x < halfW)
        {
            m_position.x = halfW;
            velocity.x = 0.f;
        }
        if (m_position.x > worldWidth - halfW)
        {
            m_position.x = worldWidth - halfW;
            velocity.x = 0.f;
        }

        // ground collision
        if (m_position.y >= groundY)
        {
            m_position.y = groundY;
            velocity.y = 0.f;
            onGround = true;
        }

        // animation update
        if (!onGround)
        {
            currentRow = 1;
            currentCol = 0;
            animationTimer = 0.f;
        }
        else if (std::abs(velocity.x) > 1.f)
        {
            currentRow = 0;
            animationTimer += dt;
            if (animationTimer >= frameTime)
            {
                animationTimer -= frameTime;
                currentCol = (currentCol + 1) % frameCol;
            }
        }
        else
        {
            currentRow = 0;
            currentCol = 0;
            animationTimer = 0.f;
        }

		// update sprite texture rect and orientation
        m_pSprite->setTextureRect(sf::IntRect(
            sf::Vector2i(currentCol * fw, currentRow * fh),
            sf::Vector2i(fw, fh)
        ));

        if (velocity.x < -1.f)
            facingRight = false;
        else if (velocity.x > 1.f)
            facingRight = true;

        float sx = spriteScaleX * (facingRight ? 1.f : -1.f);
        m_pSprite->setScale({sx, spriteScaleY});
    }
}

std::unique_ptr<Fireball> Player::releaseFireball()
{
    if (!m_fireRequested)
        return nullptr;

    m_fireRequested = false;

    // If no charge accumulated, don't fire
    if (m_charge <= 0.0f)
        return nullptr;

    // Map charge time to damage
    float t = m_charge / m_maxChargeTime;
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    int damage = std::max(1, static_cast<int>(std::round(t * m_maxDamage)));

    // Speed scales with damage
    float baseSpeed = 300.0f;
    float speed = baseSpeed + damage * 30.0f;

    sf::Vector2f dir = facingRight ? sf::Vector2f(1.f, 0.f) : sf::Vector2f(-1.f, 0.f);
    sf::Vector2f spawnPos = m_position + sf::Vector2f((facingRight ? m_collisionRadius + 8.f : -m_collisionRadius - 8.f), -16.f);

    auto fb = std::make_unique<Fireball>(spawnPos, dir, speed, damage);
    if (!fb->init())
        return nullptr;

    // reset charge
    m_charge = 0.0f;

    return fb;
}

void Player::handleEvent(const sf::Event& event)
{
    if (event.is<sf::Event::KeyPressed>())
    {
        if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
        {
            auto code = kp->code;
            if ((code == sf::Keyboard::Key::Up || code == sf::Keyboard::Key::W) && onGround)
            {
                velocity.y = -jumpStrength;
                onGround = false;
            }
            else if (code == sf::Keyboard::Key::Left || code == sf::Keyboard::Key::A)
            {
                isMovingLeft = true;
            }
            else if (code == sf::Keyboard::Key::Right || code == sf::Keyboard::Key::D)
            {
                isMovingRight = true;
            }
            else if (code == sf::Keyboard::Key::Space)
            {
                hitPressed = true;
            }
        }
    }
    else if (event.is<sf::Event::KeyReleased>())
    {
        if (const auto* kr = event.getIf<sf::Event::KeyReleased>())
        {
            auto code = kr->code;
            if (code == sf::Keyboard::Key::Left || code == sf::Keyboard::Key::A)
                isMovingLeft = false;
            else if (code == sf::Keyboard::Key::Right || code == sf::Keyboard::Key::D)
                isMovingRight = false;
            else if (code == sf::Keyboard::Key::Space)
			{
                hitPressed = false;
                m_fireRequested = true;
            }
        }
    }
}

void Player::render(sf::RenderTarget& target) const
{
    m_pSprite->setRotation(m_rotation);
    m_pSprite->setPosition(m_position);
    target.draw(*m_pSprite);
}
