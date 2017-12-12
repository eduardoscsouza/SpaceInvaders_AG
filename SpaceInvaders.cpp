#include "SpaceInvaders.h"

#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <ctime>
#include <queue>
#include <unistd.h>
#include <sys/time.h>
#include <cstdio>
#include <algorithm>

#define EUCL_DIST(x1, y1, x2, y2) (sqrt(((x1)-(x2)) * ((x1)-(x2)) + ((y1)-(y2)) * ((y1)-(y2))))

using namespace std;

typedef struct
{
	GLfloat x_pos, y_pos;
	bool alive;
}AlienShip;

class Event
{
public:
	pair<double, pair<void (*)(unsigned long long), unsigned long long> > event;

	Event(double time, void (*func)(unsigned long long), unsigned long long arg)
	{
		event = pair<double, pair<void (*)(unsigned long long), unsigned long long> >(time, pair<void (*)(unsigned long long), unsigned long long>(func, arg));
	}

	bool operator > (const Event& other) const
	{
		return this->event > other.event;
	}
};

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

unsigned long long current_game = 0;
double time_multiplier = 0.0001;
bool clean_events = false;
priority_queue<Event, vector<Event>, greater<Event> > events;

int i_pop = 0, i_gen = 0, id[POP_SIZE];
Network * cur_network = NULL, * best = NULL, * pop[POP_SIZE];
float fit_best, fit[POP_SIZE];
nn_float_t network_input[NEURAL_NETWORK_INPUT_SIZE];

void end_game()
{
	if (game_over) return;
	game_over = true;

	current_game++;
	clean_events = true;

	fit[i_pop] = fitness();
	if (fit[i_pop] > fit_best) {
		fit_best = fit[i_pop];
		best = copy_network(pop[i_pop]);
	}
	printf ("Gen: %d  Pop: %d  Fit: %.3f  Best %.3f\n", i_gen, i_pop, fit[i_pop], fit_best);

	i_pop++;
	if (i_pop >= POP_SIZE) {
		reproduction();
		i_pop = 0;
		i_gen++;
		if (i_gen >= GEN) {
			time_multiplier = 0.5;
			glutTimerFunc(0, &redraw, 0);
		}
	}
	
	if (i_gen < GEN)	cur_network = pop[i_pop];
	else 				cur_network = best;
	glutTimerFunc(0, &reset, 0);
}

void init () 
{
	for (int i = 0; i < POP_SIZE; i++) {
		pop[i] = build_network();
		id[i] = i;
	}
	cur_network = pop[0];
	best = pop[0];
	fit_best = FLT_MIN;
}

float fitness () 
{
	float fit = 0;

	fit += ALIEN_KILLS * (ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS - alien_lives);	//alien kills

	return fit;
}

bool raffle (float prob)
{
	return (rand()%RAFFLE_SIZE) < ceil (prob * RAFFLE_SIZE);
}

bool comp (int a, int b)
{
	return fit[a] < fit[b];
}

void cross_neuron (Neuron * child, Neuron * mother, Neuron * father)
{
	for (int i = 0; i < child->n_dim + 1; i++)
		child->weights[i] = (mother->weights[i] + father->weights[i]) / 2.0;
}

void cross_layer (Layer * child, Layer * mother, Layer * father)
{
	for (int i = 0; i < child->n_neurons; i++)
		cross_neuron (child->neurons[i], mother->neurons[i], father->neurons[i]);
}

Network * cross_network (Network * mother, Network * father)
{
	Network * child = build_network();

	for (int i = 1; i < child->n_layers; i++) 
		cross_layer (child->layers[i], mother->layers[i], father->layers[i]);

	return child;
}

void reproduction ()
{
	vector <Network *> next_gen;
	sort (id, id + POP_SIZE, comp);

	for (int i = 0; i < POP_SIZE; i++) {
		float chance = (float)(i + 1) / (float)POP_SIZE;
		if (raffle(chance)) 
			next_gen.push_back(pop[id[i]]);
	}

	int ngen = next_gen.size();

	while ((int)next_gen.size() < POP_SIZE) {
		Network * mother = next_gen[rand()%ngen];
		Network * father = next_gen[rand()%ngen];
		next_gen.push_back(cross_network(mother, father));
	}

	for (int i = 0; i < POP_SIZE; i++)
		pop[i] = next_gen[i];
}

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
	if (type == 0){
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
	else if (type == 1){
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
	for (int i=0; i<ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS; i++){
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

	if(alien_lives==0 || ship_lives==0) end_game();
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
	else if (key == 'r') reset(0);
}



/*
Funcao que move o seu missel e o
missel dos alienigenas
*/
void move_missile(unsigned long long value)
{
	if (value != current_game || game_over) return;

	if (missile_firing) missile_y += MISSILE_STEP;
	if (alien_missile_state==2) alien_missile_y -= MISSILE_STEP;
	
	if (missile_firing || (alien_missile_state==2)) detect_colision();
	if (alien_missile_state==0){
		alien_missile_state = 1;
		if (!game_over) add_event(ALIEN_MISSILE_WAIT_TIME, &alien_fire, value);
	}

	if (!game_over) add_event(MISSILE_DELAY, &move_missile, value);
}

/*
Funcao que move a sua nave de acordo
com as teclas pressionadas
*/
void move_ship(unsigned long long value)
{
	if (value != current_game || game_over) return;

	if (ship_dir) {
		ship_x += ship_dir * SHIP_STEP;
		ship_x = (ship_x>1.0f) ? 1.0f : ((ship_x<-1.0f) ? -1.0f : ship_x);
	}

	if (!game_over) add_event(SHIP_DELAY, &move_ship, value);
}

/*
Essa funcao move a frota de aliens atraves da tela, movendo para baixo quando atinge a borda
*/
void move_alien_fleet(unsigned long long value)
{
	if ((value>>1) != current_game || game_over) return;

	bool move_down = false;
	if (((fleet[ALIEN_FLEET_COLUMNS - 1].x_pos + 2 * ALIEN_BOX_X >= 1.0f) || (fleet[0].x_pos <= -1.08f)) && !(value & 1)) move_down = true;

	if (move_down){
		for (int i = 0; i < ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS; i++){
			fleet[i].y_pos -= ALIEN_BOX_Y;
			if (fleet[i].alive && fleet[i].y_pos<=SHIP_Y_OFFSET+0.25) end_game();
		}
		fleet_direction *= -1;
		if (!game_over) add_event(ALIEN_FLEET_DELAY, move_alien_fleet, value | 1);
	}
	else{
		for (int i = 0; i < ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS; i++) fleet[i].x_pos += 0.25 * ALIEN_BOX_X * fleet_direction;
		if (!game_over) add_event(ALIEN_FLEET_DELAY, move_alien_fleet, value & (~1));
	}
}



/*
Funcao que escolhe aleatoriamente um
alienigena para atirar
*/
void alien_fire(unsigned long long value)
{	
	if (value != current_game || game_over) return;

	int alien = -1;
	do{
		alien = rand() % (ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS);
	}while(!fleet[alien].alive);

	alien_missile_state = 2;
	alien_missile_x = fleet[alien].x_pos + 0.2;
	alien_missile_y = fleet[alien].y_pos - 0.1;
}



Network * build_network()
{
	nn_size_t aux_layers_sizes[] = NEURAL_NETWORK_LAYERS_SIZES;
	nn_float_t (*aux_layers_actvs[])(nn_float_t) = NEURAL_NETWORK_LAYERS_ACTVS;
	return new_network(NEURAL_NETWORK_N_LAYERS, aux_layers_sizes, aux_layers_actvs, NEURAL_NETWORK_INPUT_SIZE);
}

void get_input()
{
	int n_aliens = ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS;
	for (int i=0; i<n_aliens; i++){
		network_input[i*3] = fleet[i].x_pos;
		network_input[i*3 + 1] = fleet[i].y_pos;
		network_input[i*3 + 2] = fleet[i].alive;
	}
	network_input[3*n_aliens] = ship_x;
	network_input[3*n_aliens + 1] = fleet_direction;
	network_input[3*n_aliens + 2] = alien_missile_x;
	network_input[3*n_aliens + 3] = alien_missile_y;
	network_input[3*n_aliens + 4] = alien_missile_state;
	network_input[3*n_aliens + 5] = ship_lives;
	network_input[3*n_aliens + 6] = alien_lives;
}

void network_keypress(unsigned long long value)
{
	switch (value) {
		case 'l':
			special_down_call(GLUT_KEY_LEFT, 0, 0);
			break;
		case -'l':
			special_up_call(GLUT_KEY_LEFT, 0, 0);
			break;
		case 'r':
			special_down_call(GLUT_KEY_RIGHT, 0, 0);
			break;
		case -'r':
			special_up_call(GLUT_KEY_RIGHT, 0, 0);
			break;
		case ' ':
			keyboard_down_call(' ', 0, 0);
			break;
		default:
			break;
	}
}

void network_action(unsigned long long value)
{
	if (value != current_game || game_over) return;
	
	get_input();
	nn_size_t output_size = cur_network->layers[cur_network->n_layers-1]->n_neurons;
	nn_float_t * output = network_forward(cur_network, network_input);

	unsigned long long key = -1, max_id = -1;
	nn_float_t max = FLT_MIN;
	for (int i=0; i<output_size-1; i++){
		if (output[i] >= max){
			max = output[i];
			max_id = i;
		}
	}
	if (max_id == 0) key = 'l';
	else if (max_id == 1) key = 'r';
	else key = 'n';
	add_event(0, &network_keypress, key);
	add_event(NEURAL_NETWORK_KEY_UP_DELAY, &network_keypress, -key);

	if (output[output_size-1] >= 0.5f) add_event(0, &network_keypress, ' ');

	free(output);
	if (!game_over) add_event(NEURAL_NETWORK_DELAY, &network_action, value);
}

void reset(int value)
{
	if (clean_events){
		glutTimerFunc(0, &reset, 0);
		return;
	}

	ship_x = 0.0f;
	ship_dir = 0;
	fleet_direction = ALIEN_FLEET_RIGHT_MOV;
	alien_missile_state = missile_firing = 0;
	alien_missile_x = alien_missile_y = missile_x = missile_y = -1.0f;
	game_over = false;
	ship_lives = 3;
	alien_lives = ALIEN_FLEET_COLUMNS * ALIEN_FLEET_ROWS;

//	if (cur_network != NULL) delete_network(cur_network);
//	cur_network = build_network();
//
	for (int i = 0; i<ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS; i++){
		fleet[i].alive = true;
		fleet[i].x_pos = ALIEN_FLEET_START_POS_X + ((i % ALIEN_FLEET_COLUMNS) * (ALIEN_BOX_X + ALIEN_SPACING));
		fleet[i].y_pos = ALIEN_FLEET_START_POS_Y - ((i / ALIEN_FLEET_COLUMNS) * (ALIEN_BOX_Y + ALIEN_SPACING));
	}

	add_event(0, &move_ship, current_game);
	add_event(0, &move_missile, current_game);
	add_event(0, &move_alien_fleet, current_game<<1);
	add_event(0, &network_action, current_game);
}



double get_curtime()
{
	struct timeval aux_time;
	gettimeofday(&aux_time, NULL);
	return (aux_time.tv_sec*1000000.0 + aux_time.tv_usec);
}

void add_event(double delay, void (*func)(unsigned long long), unsigned long long arg)
{
	double curtime = get_curtime();
	events.push(Event(curtime + (delay*time_multiplier), func, arg));
}

void event_handler()
{
	if (clean_events){
		while(!events.empty()) events.pop();
		clean_events = false;
	}
	else{
		if (!events.empty()){
			if (get_curtime() >= events.top().event.first){
				events.top().event.second.first(events.top().event.second.second);
				events.pop();
			}
		}
	}
}



int main(int argc, char * argv[])
{
	srand(time(NULL));
	init();
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
	glutIdleFunc(&event_handler);
	reset(0);

	//Incializar o desenho
	//glutTimerFunc(0, &redraw, 0);
	glutMainLoop();
	return 0;
}
