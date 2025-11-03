#pragma once

#include "Entity.h"
#include <memory>

// forward-declare SFML types used by reference/pointer in headers
namespace sf { class Sprite; class RenderTarget; }

class Boss final : public Entity
{
	public:
		static constexpr float collisionRadius = 24.0f;
		Boss() = default;
		virtual ~Boss() = default;

		bool init() override;
		void update(float dt) override;
		void render(sf::RenderTarget& target) const override;

	private:
		float m_time = 0.0f;
		float m_baseY = 0.0f;
};