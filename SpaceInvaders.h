#ifndef SPACEINVADERS_H
#define SPACEINVADERS_H



#define SHIP_SCALE 0.20f
#define SHIP_Y_OFFSET -0.9f
#define SHIP_STEP 0.02f
#define SHIP_DELAY (30*time_multiplier)

#define MISSILE_SCALE 0.07f
#define MISSILE_STEP 0.05f
#define MISSILE_DELAY (30*time_multiplier)

#define ALIEN_MISSILE_WAIT_TIME (1000*time_multiplier)

#define ALIEN_FLEET_ROWS 6
#define ALIEN_FLEET_COLUMNS 8
#define ALIEN_BOX_X 0.15f
#define ALIEN_BOX_Y 0.07875f
#define ALIEN_SPACING 0.05f
#define ALIEN_FLEET_START_POS_X (ALIEN_BOX_X - 1)
#define ALIEN_FLEET_START_POS_Y 1.0f
#define ALIEN_FLEET_RIGHT_MOV 1
#define ALIEN_FLEET_LEFT_MOV -1
#define ALIEN_FLEET_DELAY (300*time_multiplier)

#define FPS 30

typedef struct
{
	GLfloat x_pos;
	GLfloat y_pos;
	bool alive;
}AlienShip;

void draw_ship();
void draw_alien(GLfloat, GLfloat);
void draw_fleet();
void draw_missile();
void draw_all();
void redraw();

void detect_colision();

void build_alien_fleet();

void special_up_call(int, int, int);
void special_down_call(int, int, int);
void keyboard_down_call(unsigned char, int, int);

void move_missile(int);
void move_ship(int);
void move_alien_fleet(int);

void alien_fire(int);

void reset();



#endif