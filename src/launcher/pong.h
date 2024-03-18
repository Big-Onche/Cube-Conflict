#ifndef PONG_H
#define PONG_H

#include "sdl.h"
#include "main.h"

namespace pong
{
    enum { PL_HUMAN = 0, PL_AI, NUMPLAYERS };
    enum { UP = 0, DOWN, LEFT, RIGHT, NUMMOVES };

    const int PLAYER_SIZE = 50;
    const int PLAYER_SPEED = 10;
    const int BALL_SIZE = 25;

    struct gamePlayer
    {
        vec2 pos, boundingBox; int score; bool move[NUMMOVES];
        gamePlayer() : pos(0.0f, 0.0f), boundingBox(PLAYER_SIZE, PLAYER_SIZE), score(0)
        {
            loopi(NUMMOVES) move[i] = false;
        }
    };

    struct gameBall
    {
        vec2 pos, vel, boundingBox; float rot; bool toLeft, toRight;
        gameBall() : pos(0.0f, 0.0f), vel(0.0f, 0.0f), boundingBox(BALL_SIZE, BALL_SIZE), rot(0.5f), toLeft(false), toRight(false) {}
    };

    extern void init();
    extern void play(SDL_Event *e);
}

#endif
