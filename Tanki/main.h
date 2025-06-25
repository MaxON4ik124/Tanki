#ifndef MAIN_H
#define MAIN_H

#define _CRT_SECURE_NO_WARNINGS
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <GLFW/glfw3.h>



#define TILE_EMPTY 0
#define TILE_WALL 1
#define TILE_BREAKABLE 2
#define TILE_WATER 3
#define TILE_ICE 4

#define GAME_STATE_MENU 0
#define GAME_STATE_PLAYING 1
#define GAME_STATE_PAUSED 2
#define GAME_STATE_GAME_OVER 3
#define GAME_STATE_WIN 4

#define LEVEL_START 1
#define LEVEL_TOWN 2
#define LEVEL_TOWN_UP 3
#define LEVEL_MAZE 4
#define LEVEL_ARENA 5
#define MAX_LEVEL 5

#define WIDTH 1920
#define HEIGHT 1080
#define TANK_SIZE 40
#define BULLET_SIZE 8
#define TANK_SPEED 120.0f;
#define BOT_SPEED 100.0f
#define BULLET_SPEED 800.0f
#define MAX_BULLETS 10
#define MAP_WIDTH 39
#define MAP_HEIGHT 23
#define MAX_BOTS 5
#define MAX_POWERUPS 5
#define POWERUP_SIZE 25
#define RESPAWN_TIME 12
#define RELOAD_TIME 60
#define TILE_SIZE (WIDTH / MAP_WIDTH)
#define M_PI 3.14f


#define TRANSITION_DURATION 5.0f
#define FADE_OUT_TIME 3.0f        
#define FADE_IN_TIME 3.0f        

#include "types.h"
#include "map.h"
#include "tank.h"
#include "input.h"
#include "particles.h"
#include "powerup.h"
#include "lighting.h"
#include "render.h"
#include "game.h"

extern Tank player;
extern Tank bots[MAX_BOTS];
extern Bullet bullets[MAX_BULLETS * (MAX_BOTS + 1)];
extern Powerup powerups[MAX_POWERUPS];
extern bool keys[2048];
extern double last_time;
extern double delta_time;
extern GameState game_state;
extern int level;
extern int max_level;
extern char game_message[100];
extern int message_timer;
extern int powerup_spawn_timer;
extern int menu_selection;
extern float animation_time;
extern float menu_animation;
extern float transition_timer;
extern int next_level;
extern bool transition_fade_out;


#define MAX_PARTICLES 1000
extern Particle particles[MAX_PARTICLES];
extern int next_particle;


extern LevelInfo level_info[MAX_LEVEL + 1];


extern float player_color[3];
extern float bot_colors[BOT_COUNT][3];
extern float powerup_colors[POWERUP_COUNT][3];

#endif