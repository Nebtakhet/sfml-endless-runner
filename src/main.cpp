#include "ResourceManager.h"
#include "gamestates/StateStack.h"
#include "gamestates/IState.h"
#include "gamestates/StateMenu.h"
#include "gamestates/StatePaused.h"
#include <memory>
#include <stack>
#include <optional>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Audio/Sound.hpp>

int main(int argc, char* argv[])
{
	(void)argc;

	// ResourceManager must be instantiated here -- DO NOT CHANGE
	ResourceManager::init(argv[0]);

	sf::RenderWindow window(sf::VideoMode({1024, 1024}), "Runes of Fire");
	window.setKeyRepeatEnabled(false);

	StateStack gamestates;
	if (!gamestates.push<StateMenu>())
		return -1;

	// Play background music
	std::unique_ptr<sf::Sound> pBackgroundSound;
	if (const sf::SoundBuffer* pBuf = ResourceManager::getOrLoadSoundBuffer("Gore.mp3"))
	{
		pBackgroundSound = std::make_unique<sf::Sound>(*pBuf);
		pBackgroundSound->play();
	}

	sf::Clock clock;
	while (window.isOpen())
	{
		sf::Time elapsedTime = clock.restart();

		IState* pState = gamestates.getCurrentState();
		if (!pState) return -1;

		while (auto optEvent = window.pollEvent())
		{
			const sf::Event &event = *optEvent;

			if (event.is<sf::Event::Closed>() || (event.is<sf::Event::KeyPressed>()
				&& event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape))
			{
				return 0;
			}

			if (event.is<sf::Event::FocusLost>())
			{
				// Pause the game when the window loses focus, if not already paused
				IState* currentState = gamestates.getCurrentState();
				if (!(currentState && dynamic_cast<StatePaused*>(currentState)))
					gamestates.push<StatePaused>();
			}

			// Sends the event to the current state for handling
			pState->handleEvent(event);
		}

		pState->update(elapsedTime.asSeconds());

		// Ensure background music is playing, in case it stopped
		if (pBackgroundSound && pBackgroundSound->getStatus() == sf::SoundSource::Status::Stopped)
			pBackgroundSound->play();
		window.clear(sf::Color::Blue);
		pState->render(window);
		window.display();

		gamestates.performDeferredPops();
	}
	
	return 0;
}
