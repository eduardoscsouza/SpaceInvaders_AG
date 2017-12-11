#include <SaceInvaders.h>
#include <GL/glut.h>
#include <cstdlib>
#include <cmath>

#define POS(i, j) (a * ALIEN_FLEET_COLUMNS + j)
#define EUCL_DIST(x1, y1, x2, y2) (sqrt(((x1)-(x2)) * ((x1)-(x2)) + ((y1)-(y2)) * ((y1)-(y2))))

GLfloat ship_x;
char ship_dir;
AlienShip fleet[ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS];
int fleet_direction;
GLfloat missile_x, missile_y;
bool missile_firing;
bool game_over;
GLfloat alien_missile_x, alien_missile_y;
char alien_missile_state;
int ship_lives, alien_lives;
int current_game = 0;
double time_multiplier = 1.0f;



/*
Desenha a nave centrada em (0, 0)
*/
void draw_ship()
{
	glBegin(GL_LINE_STRIP);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(0.0f, 0.5f);
	glVertex2f(0.2f, -0.2f);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(-0.2f, -0.2f);
	glVertex2f(0.0f, 0.5f);

	glEnd();
}

/*
Desenha um alien na posicao x_coord, y_coord, de design definido por 'type'
*/
void draw_alien(GLfloat x_coord, GLfloat y_coord, int type)
{
	if (type == 0)
	{
		glBegin(GL_LINE_STRIP);

		glColor3f(1.0f, 0.4f, 0.0f);
		glVertex2f(x_coord + 0.1, y_coord);
		glVertex2f(x_coord + 0.3, y_coord);
		glVertex2f(x_coord + 0.3, y_coord - 0.07);
		glVertex2f(x_coord + 0.4, y_coord - 0.2);
		glVertex2f(x_coord, y_coord - 0.2);
		glVertex2f(x_coord + 0.1, y_coord - 0.07);
		glVertex2f(x_coord + 0.1, y_coord);

		glEnd();

		glBegin(GL_LINES);

		glVertex2f(x_coord + 0.05, y_coord - 0.14);
		glVertex2f(x_coord + 0.35, y_coord - 0.14);

		glEnd();
	}
	else if (type == 1)
	{
		glBegin(GL_LINE_STRIP);
		
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex2f(x_coord + 0.05, y_coord);
		glVertex2f(x_coord + 0.35, y_coord);
		glVertex2f(x_coord + 0.4, y_coord - 0.08);
		glVertex2f(x_coord + 0.35, y_coord - 0.2);
		glVertex2f(x_coord + 0.275, y_coord - 0.1);
		glVertex2f(x_coord + 0.215, y_coord - 0.1);
		glVertex2f(x_coord + 0.2, y_coord - 0.2);
		glVertex2f(x_coord + 0.185, y_coord - 0.1);
		glVertex2f(x_coord + 0.125, y_coord - 0.1);
		glVertex2f(x_coord + 0.05, y_coord - 0.2);
		glVertex2f(x_coord, y_coord - 0.08);
		glVertex2f(x_coord + 0.05, y_coord);
		
		glEnd();
	}
	else if (type == 2)
	{
		glBegin(GL_LINE_STRIP);

		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex2f(x_coord, y_coord);
		glVertex2f(x_coord + 0.4, y_coord);
		glVertex2f(x_coord + 0.3, y_coord - 0.05);
		glVertex2f(x_coord + 0.25, y_coord - 0.2);
		glVertex2f(x_coord + 0.15, y_coord - 0.2);
		glVertex2f(x_coord + 0.1, y_coord - 0.05);
		glVertex2f(x_coord, y_coord);
		

		glEnd();

		glBegin(GL_LINE_STRIP);

		glVertex2f(x_coord + 0.06, y_coord - 0.03);
		glVertex2f(x_coord + 0.06, y_coord - 0.14);
		glVertex2f(x_coord + 0.13, y_coord - 0.14);
		
		glEnd();

		glBegin(GL_LINE_STRIP);

		glVertex2f(x_coord + 0.34, y_coord - 0.03);
		glVertex2f(x_coord + 0.34, y_coord - 0.14);
		glVertex2f(x_coord + 0.27, y_coord - 0.14);

		glEnd();
	}
}

/*
Desenha a frota de aliens, pulando os aliens que ja estao mortos
*/
void draw_fleet()
{
	int i;

	for (i = 0; i < ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS; i++)
	{
		if (fleet[i].alive){
			glPushMatrix();

			glTranslatef(fleet[i].x_pos + 0.2, fleet[i].y_pos - 0.1, 0.0f);
			glScalef(ALIEN_BOX_X / 0.4, ALIEN_BOX_Y / 0.2, 1.0f);
			glTranslatef(-(fleet[i].x_pos + 0.2), -(fleet[i].y_pos - 0.1), 0.0f);

			draw_alien(fleet[i].x_pos, fleet[i].y_pos, (i/ALIEN_FLEET_COLUMNS)/2);

			glPopMatrix();
		}
	}
}

/*
Desenha o missel centrado em (0, 0)
*/
void draw_missile()
{
	glBegin(GL_LINE_STRIP);

	glVertex2f(0.0f, 0.5f);
	glVertex2f(0.1f, 0.4f);
	glVertex2f(0.1f, -0.2f);
	glVertex2f(0.2f, -0.3f);
	glVertex2f(-0.2f, -0.3f);
	glVertex2f(-0.2f, -0.3f);
	glVertex2f(-0.1f, -0.2f);
	glVertex2f(-0.1f, 0.4f);
	glVertex2f(0.0f, 0.5f);

	glEnd();
}



/*
Desenha a cena completa
*/
void draw_all()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	if (!game_over){
		glPushMatrix();
		glTranslatef(ship_x, SHIP_Y_OFFSET, 0.0f);
		glScalef(SHIP_SCALE, SHIP_SCALE, 1.0f);
		draw_ship();
		glPopMatrix();

		if (missile_firing) {
			glPushMatrix();
			glTranslatef(missile_x, missile_y, 0.0f);
			glScalef(MISSILE_SCALE, MISSILE_SCALE, 1.0f);
			glColor3f(0.0f, 1.0f, 0.0f);
			draw_missile();
			glPopMatrix();
		}

		if (alien_missile_state==2){
			glPushMatrix();
			glTranslatef(alien_missile_x, alien_missile_y, 0.0f);
			glScalef(MISSILE_SCALE, -MISSILE_SCALE, 1.0f);
			glColor3f(1.0f, 0.0f, 0.0f);
			draw_missile();
			glPopMatrix();
		}

		draw_fleet();
	}
	else{
		glColor3f(1.0f, 0.0f, 0.0f);
		glRasterPos2d(-0.1, 0.0);
		char message[] = "GAME OVER";
		for (int i=0; message[i]!='\0'; i++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, message[i]);
	}

	glutSwapBuffers();
	glFlush();
}

/*
Funcao que constantemente desenha a cena
com uma taxa de 60FPS
*/
void redraw(int value)
{
	glutPostRedisplay();
	glutTimerFunc(1000/FPS, &redraw, 0);
}



/*
Detecta as colisoes entre o missel da sua nave
e os alienagenas, e entre o missel dos alienigenas
e sua nave
*/
void detect_colision()
{
	if (missile_firing && missile_y > 1.0f) {
		missile_firing = false;
		missile_x = missile_y = -1.0f;
	}

	for (int i=0; i<ALIEN_FLEET_COLUMNS*ALIEN_FLEET_ROWS && missile_firing; i++){
		if (fleet[i].alive && EUCL_DIST(missile_x-0.2, missile_y+0.1, fleet[i].x_pos, fleet[i].y_pos) < ((ALIEN_BOX_X+ALIEN_BOX_Y)/2)*0.8) {
			missile_firing = false;
			missile_x = missile_y = -1.0f;
			fleet[i].alive = false;
			alien_lives--;
		}
	}

	if (alien_missile_state==2){
		if (alien_missile_y<-1.0f) {
			alien_missile_state = 0;
			alien_missile_x = alien_missile_y = -1.0f;
		}
		if (EUCL_DIST(alien_missile_x, alien_missile_y, ship_x, SHIP_Y_OFFSET) < 0.1){
			alien_missile_state = 0;
			alien_missile_x = alien_missile_y = -1.0f;
			ship_lives--;
		}
	}

	if(alien_lives==0 || ship_lives==0) game_over=true;
}



/*
Funcao chamada quando uma tecla especial
do teclado e solta
*/
void special_up_call(int key, int x, int y)
{
	if (key == GLUT_KEY_LEFT) ship_dir++;
	else if (key == GLUT_KEY_RIGHT) ship_dir--;
}

/*
Funcao chamada quando uma tecla especial
do teclado e apertada
*/
void special_down_call(int key, int x, int y)
{
	if (key == GLUT_KEY_LEFT) ship_dir--;
	else if (key == GLUT_KEY_RIGHT) ship_dir++;
}

/*
Funcao chamada quando uma tecla
do teclado e apertada
*/
void keyboard_down_call(unsigned char key, int x, int y)
{
	if (key == ' ') {
		if (!missile_firing) {
			missile_firing = true;
			missile_x = ship_x;
			missile_y = SHIP_Y_OFFSET + (SHIP_SCALE * 0.5f) + (MISSILE_SCALE * 0.3f);
		}
	}
	else if (key == 'r') reset();
}



/*
Funcao que move o seu missel e o
missel dos alienigenas
*/
void move_missile(int value)
{
	if (value != current_game) return;

	if (missile_firing) missile_y += MISSILE_STEP;
	if (alien_missile_state==2) alien_missile_y -= MISSILE_STEP;
	
	if (missile_firing || (alien_missile_state==2)) detect_colision();
	if (alien_missile_state==0){
		alien_missile_state = 1;
		if (!game_over) glutTimerFunc(ALIEN_MISSILE_WAIT_TIME, &alien_fire, value);
	}

	if (!game_over) glutTimerFunc(MISSILE_DELAY, &move_missile, value);
}

/*
Funcao que move a sua nave de acordo
com as teclas pressionadas
*/
void move_ship(int value)
{
	if (value != current_game) return;

	if (ship_dir) {
		ship_x += ship_dir * SHIP_STEP;
		ship_x = (ship_x>1.0f) ? 1.0f : ((ship_x<-1.0f) ? -1.0f : ship_x);
	}

	if (!game_over) glutTimerFunc(SHIP_DELAY, &move_ship, value);
}

/*
Essa funcao move a frota de aliens atraves da tela, movendo para baixo quando atinge a borda
*/
void move_alien_fleet(int value)
{
	if ((value>>1) != current_game) return;

	int i;
	bool move_down = false;
	
	if ((fleet[ALIEN_FLEET_COLUMNS - 1].x_pos + 2 * ALIEN_BOX_X >= 1.0f
		|| fleet[0].x_pos <= -1.08f) && !(value & 1))
		move_down = true;

	if (move_down)
	{
		for (i = 0; i < ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS; i++){
			fleet[i].y_pos -= ALIEN_BOX_Y;
			if (fleet[i].alive && fleet[i].y_pos<=SHIP_Y_OFFSET+0.25) game_over=true;
		}
		fleet_direction *= -1;
		if (!game_over) glutTimerFunc(ALIEN_FLEET_DELAY, move_alien_fleet, value | 1);
	}
	else
	{
		for (i = 0; i < ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS; i++)
			fleet[i].x_pos += 0.25 * ALIEN_BOX_X * fleet_direction;
		if (!game_over) glutTimerFunc(ALIEN_FLEET_DELAY, move_alien_fleet, value & (~1));
	}
}



/*
Funcao que escolhe aleatoriamente um
alienigena para atirar
*/
void alien_fire(int value)
{
	if (value != current_game) return;
	
	int alien = -1;
	do {
		alien = rand() % (ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS);
	}while(!fleet[alien].alive);

	alien_missile_state = 2;
	alien_missile_x = fleet[alien].x_pos + 0.2;
	alien_missile_y = fleet[alien].y_pos - 0.1;
}



void reset()
{
	current_game++;
	ship_x = 0.0f;
	ship_dir = 0;
	fleet_direction = ALIEN_FLEET_RIGHT_MOV;
	alien_missile_state = missile_firing = 0;
	alien_missile_x = alien_missile_y = missile_x = missile_y = -1.0f;
	game_over = false;
	ship_lives = 3;
	alien_lives = ALIEN_FLEET_COLUMNS * ALIEN_FLEET_ROWS;
	for (int i = 0; i<ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS; i++){
		fleet[i].alive = true;
		fleet[i].x_pos = ALIEN_FLEET_START_POS_X + ((i % ALIEN_FLEET_COLUMNS) * (ALIEN_BOX_X + ALIEN_SPACING));
		fleet[i].y_pos = ALIEN_FLEET_START_POS_Y - ((i / ALIEN_FLEET_COLUMNS) * (ALIEN_BOX_Y + ALIEN_SPACING));
	}

	glutTimerFunc(0, &move_ship, current_game);
	glutTimerFunc(0, &move_missile, current_game);
	glutTimerFunc(750, &move_alien_fleet, current_game<<1);
}



int main(int argc, char * argv[])
{
	//Inicializacao do glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(20, 20);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Space Invaders");

	//Setar as variabeis globais e parametros do OpenGL
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);

	//Setar as funcoes de callback
	glutIgnoreKeyRepeat(1);
	glutSpecialFunc(&special_down_call);
	glutKeyboardFunc(&keyboard_down_call);
	glutSpecialUpFunc(&special_up_call);
	glutDisplayFunc(&draw_all);
	reset();

	//Incializar o desenho
	glutTimerFunc(0, &redraw, 0);
	glutMainLoop();
	return 0;
}
