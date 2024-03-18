#include "pong.h"
#include "textures.h"
#include "particles.h"
#include "audio.h"
#include <algorithm>

namespace pong
{
    gamePlayer players[NUMPLAYERS];
    gameBall ball;

    const int centeredPlayer = PLAYER_SIZE / 2;

    void init() // init the game
    {
        loopi(NUMPLAYERS)
        {
            players[i].pos = vec2((i == PL_HUMAN) ? 30 - centeredPlayer : SCR_W - 30- centeredPlayer, SCR_H / 2.0f - centeredPlayer);
            players[i].movingUp = false;
            players[i].movingDown = false;
        }

        ball.pos = vec2(SCR_W / 2.0f - BALL_SIZE / 2.0f, SCR_H / 2.0f - BALL_SIZE / 2.0f);
        ball.rot = 0.f;

        vec2 randomStart(((-15 + rnd(31)) / 250.f), ((-15 + rnd(31)) / 250.f));

        if(rnd(2)) // Change the start direction on new game
        {
            ball.vel = vec2(-1.5f, rnd(2) ? 1.5f : -1.5f) + randomStart;
            ball.toLeft = true;
            ball.toRight = false;
        }
        else
        {
            ball.vel = vec2(1.5f, rnd(2) ? 1.5f : -1.5f) + randomStart;
            ball.toLeft = false;
            ball.toRight = true;
        }
    }

    bool collide(const vec2& pos1, const vec2& size1, const vec2& pos2, const vec2& size2)
    {
        return pos1.x < pos2.x + size2.x && pos1.x + size1.x > pos2.x && pos1.y < pos2.y + size2.y && pos1.y + size1.y > pos2.y;
    }

    bool checkPlayerCollide()
    {
        bool collided = false;

        if(ball.toLeft && collide(ball.pos, ball.boundingBox, players[PL_HUMAN].pos, players[PL_HUMAN].boundingBox)) // Check collision with the human player
        {
            collided = true;
            ball.toRight = true;
            ball.toLeft = false;
            audio::playSound(S_PLAYER, players[PL_HUMAN].pos);
        }
        else if(ball.toRight && collide(ball.pos, ball.boundingBox, players[PL_AI].pos, players[PL_AI].boundingBox)) // Check collision with the AI
        {
            collided = true;
            ball.toLeft = true;
            ball.toRight = false;
            audio::playSound(S_PLAYER, players[PL_AI].pos);
        }

        if(collided)
        {
            ball.vel.x = -ball.vel.x; // Reverse direction
            ball.vel.x += (ball.vel.x > 0) ? 0.5f : -0.5f;
            ball.vel.y += (ball.vel.y > 0) ? 0.5f : -0.5f;
        }

        return collided;
    }

    void checkBallCollisions()
    {
        checkPlayerCollide();

        if(ball.pos.y <= 0 || ball.pos.y >= SCR_H) // Ball hits the top or bottom wall
        {
            ball.vel.y = -ball.vel.y;
            particles::explosion(PART_SPARK, ball.pos, 0xFFBB11FF, 5, 15 + rnd(5), 150, 1000);
            audio::playSound(S_GRENADE, ball.pos);
        }

        bool scored = false;

        if(ball.pos.x <= 0) // Ball goes past the player (left side)
        {
            players[PL_AI].score++;
            scored = true;
        }
        else if(ball.pos.x >= SCR_W) // Ball goes past the AI (right side)
        {
            players[PL_HUMAN].score++;
            scored = true;
        }

        if(scored)
        {
            particles::explosion(TEX_BALL_GLOW, ball.pos, 0xFFFFFFFF, 50, 30, 150, 750);
            particles::explosion(PART_SPARK, ball.pos, 0xFFAA11FF, 5, 40, 200, 1500);
            audio::playSound(S_EXPLOSION, ball.pos);
            init();
        }

        if(players[PL_AI].score > 9 || players[PL_HUMAN].score > 9) playPong = false;
    }

    void updateBall() // Move the ball and checks for collisions
    {
        ball.pos.x += ball.vel.x;
        ball.pos.y += ball.vel.y;
        ball.rot += ball.vel.x;
        checkBallCollisions();
    }

    void movePlayer(int player, float speed, bool horizontal = false)
    {
        if(!horizontal) players[player].pos.y = std::clamp(players[player].pos.y + speed, 0.f, (float)SCR_H - PLAYER_SIZE);
        else if (player == PL_HUMAN) players[player].pos.x = std::clamp(players[player].pos.x + speed, 0.f, static_cast<float>(SCR_W / 2) - PLAYER_SIZE);
        else if (player == PL_AI) players[player].pos.x = std::clamp(players[player].pos.x + speed, static_cast<float>(SCR_W / 2), static_cast<float>(SCR_W) - PLAYER_SIZE);

        particles::explosion(PART_SMOKE, players[player].pos, 0x885511FF, 40, 1, 20, 500);
    }

    void updateAi()
    {
        float yDiff = ball.pos.y - (players[PL_AI].pos.y + PLAYER_SIZE / 2);
        float yStep = std::clamp(yDiff * 0.4f, (float)-PLAYER_SPEED, (float)PLAYER_SPEED);
        movePlayer(PL_AI, yStep);

        bool attack = ((ball.toRight && (std::abs(ball.vel.x) <= 4.0f)) || (ball.toLeft && (std::abs(ball.vel.x) <= 8.f))) && (ball.pos.x < SCR_W / 2);
        bool retreat = !attack && (std::abs(ball.vel.x) > 8.f) || (ball.toRight && ball.vel.x < 2.5f); // Retreat if ball speed is high

        if(retreat || attack)
        {
            float xTarget = retreat ? (SCR_W - (30 + (PLAYER_SIZE / 2.f))) : ((SCR_W / 2) - (20 * ball.vel.x));
            float xDiff = xTarget - players[PL_AI].pos.x;
            float xStep = std::clamp(xDiff * 0.25f, (float)-PLAYER_SPEED, (float)PLAYER_SPEED);

            if(retreat) movePlayer(PL_AI, xStep, true);
            else movePlayer(PL_AI, xStep, true);
        }
    }

    void updateEvents(SDL_Event *e) // Handle events on queue
    {
        while(SDL_PollEvent(e) != 0)
        {
            if(e->type == SDL_KEYDOWN) // Handle key down for the player's paddle
            {
                switch(e->key.keysym.sym)
                {
                    case SDLK_ESCAPE: playPong = false; break;
                    case SDLK_UP: players[PL_HUMAN].movingUp = true; break;
                    case SDLK_DOWN: players[PL_HUMAN].movingDown = true; break;
                    case SDLK_LEFT: players[PL_HUMAN].movingLeft = true; break;
                    case SDLK_RIGHT: players[PL_HUMAN].movingRight = true; break;
                }
            }
            else if(e->type == SDL_KEYUP) // Handle key up for the player's paddle
            {
                switch(e->key.keysym.sym)
                {
                    case SDLK_UP: players[PL_HUMAN].movingUp = false; break;
                    case SDLK_DOWN: players[PL_HUMAN].movingDown = false; break;
                    case SDLK_LEFT: players[PL_HUMAN].movingLeft = false; break;
                    case SDLK_RIGHT: players[PL_HUMAN].movingRight = false; break;
                }
            }
        }
    }

    void render() // render the game
    {
        SDL_RenderClear(sdl::renderer);

        SDL_SetRenderDrawColor(sdl::renderer, 0, 0x88, 0, 0);

        int textureFlags = TEX_ALPHA|TEX_SHADOW;

        texture::render(TEX_PONG, 0, 0, SCR_W, SCR_H);
        particles::update();
        texture::render(TEX_HAP, players[PL_HUMAN].pos.x, players[PL_HUMAN].pos.y, PLAYER_SIZE, PLAYER_SIZE, textureFlags);
        texture::render(TEX_NOEL, players[PL_AI].pos.x, players[PL_AI].pos.y, PLAYER_SIZE, PLAYER_SIZE, textureFlags);
        texture::render(TEX_BALL_GLOW, ball.pos.x - BALL_SIZE * 2, ball.pos.y - BALL_SIZE * 2, BALL_SIZE * 4, BALL_SIZE * 4, TEX_ALPHA);
        texture::render(TEX_BALL, ball.pos.x - BALL_SIZE / 2, ball.pos.y - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE, textureFlags, ball.rot);

        std::string scores = std::to_string(players[PL_HUMAN].score) + " - " + std::to_string(players[PL_AI].score);

        SDL_Color textColor = {255, 255, 255};
        SDL_Color shadowColor = {50, 50, 50};

        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(sdl::fontMain, scores.c_str(), textColor);
        SDL_Surface* shadowSurface = TTF_RenderUTF8_Blended(sdl::fontMain, scores.c_str(), shadowColor);

        if(textSurface != nullptr || shadowSurface != nullptr)
        {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(sdl::renderer, textSurface);
            SDL_Texture* shadowTexture = SDL_CreateTextureFromSurface(sdl::renderer, shadowSurface);

            if(textTexture != nullptr || shadowTexture != nullptr)
            {
                int textWidth = textSurface->w;
                int textHeight = textSurface->h;

                SDL_Rect shadowRect = {(SCR_W - textWidth) / 2 + 3, 13, textWidth, textHeight};
                SDL_Rect textRect = {(SCR_W - textWidth) / 2, 10, textWidth, textHeight};

                SDL_RenderCopy(sdl::renderer, shadowTexture, nullptr, &shadowRect);
                SDL_RenderCopy(sdl::renderer, textTexture, nullptr, &textRect);

                SDL_DestroyTexture(textTexture);
                SDL_DestroyTexture(shadowTexture);
            }

            SDL_FreeSurface(textSurface);
            SDL_FreeSurface(shadowSurface);
        }

        SDL_RenderPresent(sdl::renderer); // update screen
    }

    void play(SDL_Event *e)
    {
        updateEvents(e);
        if(players[PL_HUMAN].movingUp) movePlayer(PL_HUMAN, -PLAYER_SPEED);
        if(players[PL_HUMAN].movingDown) movePlayer(PL_HUMAN, PLAYER_SPEED);
        if(players[PL_HUMAN].movingLeft) movePlayer(PL_HUMAN, -PLAYER_SPEED, true);
        if(players[PL_HUMAN].movingRight) movePlayer(PL_HUMAN, PLAYER_SPEED, true);
        updateBall();
        updateAi();
        render();
    }
}
