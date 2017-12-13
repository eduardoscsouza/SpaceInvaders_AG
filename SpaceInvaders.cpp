#include "SpaceInvaders.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <ctime>
#include <queue>
#include <unistd.h>
#include <sys/time.h>

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

bool display_on;

unsigned long long current_game;
double time_multiplier;
bool clean_events;
priority_queue<Event, vector<Event>, greater<Event> > events;

Network * cur_network;
nn_float_t network_input[NEURAL_NETWORK_INPUT_SIZE];

int cur_gen, cur_ind, cur_test;
double cur_fit;
priority_queue<pair<double, int> > population_fitness;
vector<Network *> population(POPULATION_SIZE);
Network * global_best;
double global_best_fit;



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
	if (display_on) glutTimerFunc(1000/FPS, &redraw, 0);
}

void turn_display(bool state)
{
	if (display_on && !state) display_on = state;
	else if (!display_on && state){
		display_on = state;
		glutTimerFunc(0, &redraw, 0);
	}
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
	else if (key == 'r') reset();
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
	/*
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
    */
    network_input[0] = ship_x;
    bool found = false;
    for (int i=(ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS)-1; i>=0 && !found; i--){
        if(fleet[i].alive){
            network_input[1] = fleet[i].x_pos;
            found = true;
        }
    }
    //network_input[2] = alien_missile_x;
    //network_input[3] = alien_missile_y;
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
	add_event(NEURAL_NETWORK_KEY_DELAY, &network_keypress, -key);

	if (output[output_size-1] >= 0.5f) add_event(0, &network_keypress, ' ');

	free(output);
	if (!game_over) add_event(NEURAL_NETWORK_DELAY, &network_action, value);
}



double get_fitness()
{
	return (ALIEN_FLEET_ROWS * ALIEN_FLEET_COLUMNS) - alien_lives;
}

double normalized_random_generator(unsigned long long precision)
{
	return (rand() % (precision+1)) / (double)precision;
}

double probability_function(double x, double coef)
{
	return exp(coef*x - 1.0)/exp(coef - 1.0);
	//return log(coef*x + 1)/log(coef + 1);
}

Network * make_child(Network * mother, Network * father)
{
	Network * child = build_network();

	for(int i=0; i<mother->n_layers; i++){
		for (int j=0; j<mother->layers[i]->n_neurons; j++){
			for (int k=0; k<mother->layers[i]->neurons[j]->n_dim+1; k++){
				child->layers[i]->neurons[j]->weights[k] =
				mother->layers[i]->neurons[j]->weights[k] +
				father->layers[i]->neurons[j]->weights[k] / 2.0;

				if (normalized_random_generator(PRECISION) <= MUTATION_PROB)
					child->layers[i]->neurons[j]->weights[k] += MUTATION_VAL;
			}
		}
	}

	return child;
}

void get_next_generation()
{	
	double cur_rank = POPULATION_SIZE-1;
	vector<Network *> next_gen;
	while(!population_fitness.empty()){
		if (normalized_random_generator(PRECISION) <= probability_function(cur_rank/(POPULATION_SIZE-1), PROB_COEF))
			next_gen.push_back(population[population_fitness.top().second]);
		else delete_network(population[population_fitness.top().second]);
		
		population_fitness.pop();
		cur_rank--;
	}

	int parents_count = next_gen.size();
	while(next_gen.size() < POPULATION_SIZE){
		Network * mother = next_gen[rand()%parents_count];
		Network * father = next_gen[rand()%parents_count];
		while(mother == father) father = next_gen[rand()%parents_count];
		next_gen.push_back(make_child(mother, father));
	}

	population.swap(next_gen);
}



void end_game()
{
	if (game_over) return;
	game_over = true;
	current_game++;
	clean_events = true;
	glutTimerFunc(0, &wait_events_end, 0);
}

void wait_events_end(int value)
{
	if (clean_events){
		glutTimerFunc(0, &wait_events_end, 0);
		return;
	}
	post_end_game_operations();
}

void post_end_game_operations()
{
	//printf("End of game %llu; %d %d %d %p %lf\n", current_game-1, cur_gen, cur_ind, cur_test, cur_network, get_fitness());

	if (cur_gen < N_GENERATIONS){
		cur_test++;
		cur_fit += get_fitness();
		if (cur_test == N_TESTS){
			population_fitness.push(pair<double, int>(cur_fit/N_TESTS, cur_ind));
			cur_fit = cur_test = 0;
			cur_ind++;
		}
		if (cur_ind == POPULATION_SIZE){
			if (population_fitness.top().first >= global_best_fit){
				global_best = copy_network(population[population_fitness.top().second]);
				global_best_fit = population_fitness.top().first;
			}
			printf("Generation Best: %lf --- Global Best: %lf\n", population_fitness.top().first, global_best_fit);
			get_next_generation();
			
			while(!population_fitness.empty()) population_fitness.pop();
			cur_ind = 0;
			cur_gen++;
		}

		cur_network = population[cur_ind];
	}
	else{
		if (cur_gen == N_GENERATIONS){
			cur_network = global_best;
			for (unsigned long long i=0; i<population.size(); i++) delete_network(population[i]);
			time_multiplier = 0.5;
			turn_display(true);
			cur_gen++;
		}
		else{
			printf("Last fitness %lf\n", get_fitness());
			delete_network(global_best);
			exit(0);
		}
	}

	reset();
}

void reset()
{
	ship_x = 0.0f;
	ship_dir = 0;
	fleet_direction = ALIEN_FLEET_INIT_MOV;
	alien_missile_state = missile_firing = 0;
	alien_missile_x = alien_missile_y = missile_x = missile_y = MISSILE_NULL_POS;
	game_over = false;
	ship_lives = SHIP_INIT_LIVES;
	alien_lives = ALIEN_FLEET_COLUMNS * ALIEN_FLEET_ROWS;
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



void init()
{
	srand(time(NULL));
	for (int i=0; i<POPULATION_SIZE; i++) population[i] = build_network();
	turn_display(DISPLAY_ON);
	current_game = 0;
	time_multiplier = TIME_MULTIPLIER;
	clean_events = false;
	cur_gen = cur_ind = cur_test = 0;
	cur_fit = 0.0;
	while(!population_fitness.empty()) population_fitness.pop();
	cur_network = population[0];
	global_best = NULL;
	global_best_fit = FLT_MIN;
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
	glutIdleFunc(&event_handler);
	init();
	reset();

	//Incializar o desenho
	glutMainLoop();
	return 0;
}