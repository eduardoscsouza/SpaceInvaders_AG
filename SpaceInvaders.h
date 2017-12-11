#ifndef SPACEINVADERS_H
#define SPACEINVADERS_H



#include "neuralnetwork.h"
#include <GL/glut.h>

#define SHIP_SCALE 0.20f
#define SHIP_Y_OFFSET -0.9f
#define SHIP_STEP 0.02f
#define SHIP_DELAY 30000.0

#define MISSILE_SCALE 0.07f
#define MISSILE_STEP 0.05f
#define MISSILE_DELAY 30000.0

#define ALIEN_MISSILE_WAIT_TIME 1000000.0

#define ALIEN_FLEET_ROWS 6
#define ALIEN_FLEET_COLUMNS 8
#define ALIEN_BOX_X 0.15f
#define ALIEN_BOX_Y 0.07875f
#define ALIEN_SPACING 0.05f
#define ALIEN_FLEET_START_POS_X (ALIEN_BOX_X - 1)
#define ALIEN_FLEET_START_POS_Y 1.0f
#define ALIEN_FLEET_RIGHT_MOV 1
#define ALIEN_FLEET_LEFT_MOV -1
#define ALIEN_FLEET_DELAY 300000.0

#define NEURAL_NETWORK_DELAY 10000
#define NEURAL_NETWORK_KEY_UP_DELAY 2000
#define NEURAL_NETWORK_N_LAYERS 3
#define NEURAL_NETWORK_INPUT_SIZE (3*ALIEN_FLEET_ROWS*ALIEN_FLEET_COLUMNS + 7)
#define NEURAL_NETWORK_LAYERS_SIZES {100, 100, 4}
#define NEURAL_NETWORK_LAYERS_ACTVS {&sigm, &sigm, &sigm}

#define EVENT_HANDLER_DELAY 20

#define MAX_GAMES 1000

#define FPS 30

void draw_ship();
void draw_alien(GLfloat, GLfloat);
void draw_fleet();
void draw_missile();
void draw_all();
void redraw(int);

void detect_colision();

void special_up_call(int, int, int);
void special_down_call(int, int, int);
void keyboard_down_call(unsigned char, int, int);

void move_missile(unsigned long long);
void move_ship(unsigned long long);
void move_alien_fleet(unsigned long long);

void alien_fire(unsigned long long);

Network * build_network();
void get_input();
void network_keypress(unsigned long long);
void network_action(unsigned long long);

void end_game();
void reset(int);

double get_curtime();
void add_event(double, void (*)(unsigned long long), unsigned long long);
void event_handler();



#endif