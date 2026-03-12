// platform game.....
// Author :Noel O' Hara

#ifdef _DEBUG 
#pragma comment(lib,"sfml-graphics-d.lib") 
#pragma comment(lib,"sfml-audio-d.lib") 
#pragma comment(lib,"sfml-system-d.lib") 
#pragma comment(lib,"sfml-window-d.lib") 
#pragma comment(lib,"sfml-network-d.lib") 
#else 
#pragma comment(lib,"sfml-graphics.lib") 
#pragma comment(lib,"sfml-audio.lib") 
#pragma comment(lib,"sfml-system.lib") 
#pragma comment(lib,"sfml-window.lib") 
#pragma comment(lib,"sfml-network.lib") 
#endif 

#include <SFML/Graphics.hpp>
#include <iostream>
#include <time.h>
#include <vector>

class Particle
{
public:
	int timetoLive;
	sf::Vector2f velocity;
	sf::RectangleShape shape;

	void Draw(sf::RenderWindow& win)
	{
		if (timetoLive > 0)
			win.draw(shape);
	}

	void Update()
	{
		if (timetoLive > 0)
		{
			shape.move(velocity);
			timetoLive--;
		}
	}

	Particle() {}

	Particle(sf::Vector2f pos, sf::Vector2f vel, sf::Color color)
	{
		shape.setSize(sf::Vector2f(6, 6));
		shape.setPosition(pos);
		shape.setFillColor(color);
		velocity = vel;
		timetoLive = rand() % 30;
	}
};

#define maxParticles 50
class ParticleSystem
{
public:
	Particle particles[maxParticles];
	sf::Vector2f position;

	// Spawns all particles at pos with random vels
	void Initialise(sf::Vector2f pos, sf::Color color)
	{
		position = pos;
		for (int i = 0; i < maxParticles; i++)
		{
			particles[i] = Particle(position,
				sf::Vector2f((float)(rand() / double(RAND_MAX) * 12 - 6),
					(float)(rand() / double(RAND_MAX) * 12 - 6)),
				color);
		}
	}

	void Update()
	{
		for (int i = 0; i < maxParticles; i++)
			particles[i].Update();
	}
	
	void Draw(sf::RenderWindow& win)
	{
		for (int i = 0; i < maxParticles; i++)
			particles[i].Draw(win);
	}

	ParticleSystem() {}
};

class LavaBlock
{
public:
	int row, col;
	sf::Vector2f position;
	float velocityX; // positive - moving right, negative - left
};

class Game
{
public:
	//create Window
	sf::RenderWindow window;
	sf::View view;
	float randomNum;

	sf::RectangleShape playerShape;

	// Player physics
	float velocityX = 0, velocityY = 0, gravity = 0.3;
	bool onIce = false;
	bool isDying = false;
	int deathTimer = 0; // counts down to 0 then calls init()

	// Level grid dimensions and tile size constants
	static const int numRows = 20;
	static const int numCols = 45;
	const float tileWidth = 70.f;
	const float tileHeight = 30.f;
	const float worldScrollSpeed = 3.7f; // pixels per frame the world scrolls left
	const float lavaSpeed = 3.0f;
	std::vector<LavaBlock> lavaBlocks;
	
	int levelData[numRows][numCols] =
	{
	 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
	 {0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
	 {0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0},
	 {0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0},
	 {0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0},
	 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,1,0,0,1,1,0,3},
	 {0,0,0,0,0,0,0,0,0,0,2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,4,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 	 {0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,6,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
	};

	sf::RectangleShape level[numRows][numCols];

	ParticleSystem particles;  

	Game()
	{
		window.create(sf::VideoMode({ 800, 600 }), "Endless Runner Game");
	}

	void init()
	{
		view = window.getDefaultView();
		playerShape.setSize(sf::Vector2f(20, 20));
		playerShape.setPosition(sf::Vector2f(160, 500));

		// Set size, pos and colour for every tile based on its type
		for (int row = 0; row < numRows; row++)
		{
			for (int col = 0; col < numCols; col++)
			{
				if (levelData[row][col] == 1)
				{
					level[row][col].setSize(sf::Vector2f(tileWidth, tileHeight));
					level[row][col].setPosition(sf::Vector2f(col * tileWidth, row * tileHeight));
					level[row][col].setFillColor(sf::Color::Red);
				}
				if (levelData[row][col] == 0)
				{
					level[row][col].setSize(sf::Vector2f(tileWidth, tileHeight));
					level[row][col].setPosition(sf::Vector2f(col * tileWidth, row * tileHeight));
					level[row][col].setFillColor(sf::Color::Black);
				}

				if (levelData[row][col] == 2)
				{
					level[row][col].setSize(sf::Vector2f(tileWidth, tileHeight));
					level[row][col].setPosition(sf::Vector2f(col * tileWidth, row * tileHeight));
					level[row][col].setFillColor(sf::Color::Blue);
				}

				if (levelData[row][col] == 3)
				{
					level[row][col].setSize(sf::Vector2f(tileWidth, tileHeight));
					level[row][col].setPosition(sf::Vector2f(col * tileWidth, row * tileHeight));
					level[row][col].setFillColor(sf::Color::Green);
				}

				if (levelData[row][col] == 4)
				{
					level[row][col].setSize(sf::Vector2f(tileWidth, tileHeight));
					level[row][col].setPosition(sf::Vector2f(col * tileWidth, row * tileHeight));
					level[row][col].setFillColor(sf::Color::Yellow);
				}

				if (levelData[row][col] == 5)
				{
					level[row][col].setSize(sf::Vector2f(tileWidth, tileHeight));
					level[row][col].setPosition(sf::Vector2f(col * tileWidth, row * tileHeight));
					level[row][col].setFillColor(sf::Color::Cyan);
				}

				if (levelData[row][col] == 6)
				{
					level[row][col].setSize(sf::Vector2f(tileWidth, tileHeight));
					level[row][col].setPosition(sf::Vector2f(col * tileWidth, row * tileHeight));
					level[row][col].setFillColor(sf::Color(255, 140, 0));
				}
			}
			std::cout << std::endl;
		}
		rebuildLavaBlocks(0);
	}

	// Moves each lava block, bounces off solid tiles and kills player on contact
	void updateLavaBlocks()
	{
		for (auto& lava : lavaBlocks)
		{
			// Project next position
			float movedX = lava.position.x + lava.velocityX - worldScrollSpeed;
			sf::FloatRect futureBounds(sf::Vector2f(movedX, lava.position.y), sf::Vector2f(tileWidth, tileHeight));
			bool hitWall = false;

			// Check every tile in the same row for a collision(skips empty and other lava)
			for (int col = 0; col < numCols && !hitWall; col++)
			{
				int tileType = levelData[lava.row][col];
				if (tileType == 0 || tileType == 6) 
				{
					continue;
				}
				if (futureBounds.findIntersection(level[lava.row][col].getGlobalBounds()))
				{
					hitWall = true;
				}
			}

			if (hitWall)
			{
				lava.velocityX = -lava.velocityX;
				movedX = lava.position.x + lava.velocityX - worldScrollSpeed;
			}

			lava.position.x = movedX;
			level[lava.row][lava.col].setPosition(lava.position);

			// Kill player
			if (!isDying && playerShape.getGlobalBounds().findIntersection(level[lava.row][lava.col].getGlobalBounds()))
			{
				particles.Initialise(playerShape.getPosition(), sf::Color::Red);
				isDying = true;
				deathTimer = 10;
			}
		}
	}

	// Scans levelData for lava tiles(6) and populates the lavaBlocks vector
	void rebuildLavaBlocks(float startX)
	{
		lavaBlocks.clear();
		for (int row = 0; row < numRows; row++)
		{
			for (int col = 0; col < numCols; col++)
			{
				if (levelData[row][col] == 6)
				{
					LavaBlock block;
					block.row = row;
					block.col = col;
					block.position = sf::Vector2f(col * tileWidth + startX, row * tileHeight);
					block.velocityX = lavaSpeed;
					lavaBlocks.push_back(block);
				}
			}
		}
	}

	void run()
	{
		sf::Time timePerFrame = sf::seconds(1.0f / 60.0f);

		sf::Time timeSinceLastUpdate = sf::Time::Zero;

		sf::Clock clock;
		clock.restart();

		while (window.isOpen())
		{
			// check if the close window button is clicked on. 
			while (const std::optional event = window.pollEvent())
			{
				if (event->is<sf::Event::Closed>())
				{
					window.close();
				}
				else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
				{
					if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
						window.close();
				}
			}

			timeSinceLastUpdate += clock.restart();

			if (timeSinceLastUpdate > timePerFrame)
			{
				// Count down then reset when player is dying
				if (isDying)
				{
					deathTimer--;
					if (deathTimer == 0)
					{
						isDying = false;
						init();
					}
				}
				else
				{
				// Scroll all tiles left(forward movement)
				for (int row = 0; row < numRows; row++)
				{
					for (int col = 0; col < numCols; col++)
					{
						level[row][col].move(sf::Vector2f(-worldScrollSpeed, 0));
					}
				}

				// Smaller jump on ice, full jump otherwise
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Space) && velocityY == 0)
				{
					if (onIce)
					{
						velocityY = -7.0f;
						onIce = false;
					}
					else
					{
						velocityY = -11.8f;
					}
				}
				velocityY = velocityY + gravity;
				playerShape.move(sf::Vector2f(0, velocityY));

				gravity = 0.6; // restore value

				for (int row = 0; row < numRows; row++)
				{
					for (int col = 0; col < numCols; col++)
					{
						// Tile 1 - land on top, die on side orbottom
						if (velocityY >= 0)
						{
							if (levelData[row][col] == 1)
							{

								if (playerShape.getGlobalBounds().findIntersection(level[row][col].getGlobalBounds()))
								{
									if (playerShape.getPosition().y < level[row][col].getPosition().y)
									{
										// Landed on top
										gravity = 0;
										velocityY = 0;
										playerShape.setPosition(sf::Vector2f(playerShape.getPosition().x, level[row][col].getPosition().y));
										playerShape.move(sf::Vector2f(0, -playerShape.getGlobalBounds().size.y));
										break;
									}
									else {
										// Hit side
										particles.Initialise(playerShape.getPosition(), sf::Color::Red);
										isDying = true;
										deathTimer = 10;
									}
								}
							}

						}
						// Tile 1 - die if jumping up into it
						if (velocityY < 0)
						{
							if (levelData[row][col] == 1)
							{
								if (playerShape.getGlobalBounds().findIntersection(level[row][col].getGlobalBounds()))
								{
									particles.Initialise(playerShape.getPosition(), sf::Color::Red);
									isDying = true;
									deathTimer = 10;
								}
							}

						}
						// Tile 2 - always kills the player
						if (levelData[row][col] == 2)
						{
							if (playerShape.getGlobalBounds().findIntersection(level[row][col].getGlobalBounds()))
							{
								particles.Initialise(playerShape.getPosition(), sf::Color::Blue);
								isDying = true;
								deathTimer = 10;
							}
						}

						// Tile 3 - player wins
						if (levelData[row][col] == 3)
						{
							if (playerShape.getGlobalBounds().findIntersection(level[row][col].getGlobalBounds()))
							{
								window.close();
							}
						}

						// Tile 4 - boost jump, launch player upward if on top
						if (levelData[row][col] == 4)
						{
							if (playerShape.getGlobalBounds().findIntersection(level[row][col].getGlobalBounds()))
							{
								if (playerShape.getPosition().y < level[row][col].getPosition().y)
								{
									playerShape.setPosition(sf::Vector2f(playerShape.getPosition().x, level[row][col].getPosition().y));
									playerShape.move(sf::Vector2f(0, -playerShape.getGlobalBounds().size.y));
									velocityY = -20.0;
								}
								else
								{
									init();
								}
							}
						}

						// Tile 5 - ice, reduced jump
						if (levelData[row][col] == 5)
						{
							if (playerShape.getGlobalBounds().findIntersection(level[row][col].getGlobalBounds()))
							{
								if (playerShape.getPosition().y < level[row][col].getPosition().y)
								{
									gravity = 0;
									velocityY = 0;
									playerShape.setPosition(sf::Vector2f(playerShape.getPosition().x, level[row][col].getPosition().y));
									playerShape.move(sf::Vector2f(0, -playerShape.getGlobalBounds().size.y));
									onIce = true;
								}
								else
								{
									init();
								}
							}
						}
					}
				}

				if (playerShape.getPosition().y > 600)
				{
					init();
				}
				updateLavaBlocks();
				} 

				window.clear();

				for (int row = 0; row < numRows; row++)
				{
					for (int col = 0; col < numCols; col++)
					{
						window.draw(level[row][col]);

					}
				}
				window.draw(playerShape);
				particles.Update();
				particles.Draw(window);

				window.display();

				timeSinceLastUpdate = sf::Time::Zero;
			}
		}
	}
};


int main()
{
	Game game;

	game.init();
	game.run();

	return 0;
}