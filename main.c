#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <math.h>
#include <time.h>

#define WIDTH (800.00)
#define HEIGHT (700.00)
#define G (0.01)
//Don't start at top of the screen offset is where the first pendulum starts on the y-axis
#define OFFSET (300)
#define RADIUS (10.0)

double normalize_angle(double val){
	while (val < 0){
		val += 2 * M_PI;
	}
	while (val > 2 * M_PI){
		val -= 2 * M_PI;
	}
	return val;
}

typedef struct State{
	double *ls; //lengths
	double *ms; //masses
	double *as; //theta (angles)
	double *avs; //Angular velocity
	uint32_t *canvas;

} State;

static void draw_circle(SDL_Renderer *renderer, int32_t cx, int32_t cy){
	for (int32_t dy=1; dy<= RADIUS; ++dy){
		double dx = floor(sqrt((2.0 * RADIUS *dy) - (dy * dy)));
		//change draw color to red
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red: 255, Green: 0, Blue: 0, Alpha: 255
		SDL_RenderDrawLine(renderer, cx - dx, cy + dy - RADIUS, cx + dx, cy + dy - RADIUS);
		SDL_RenderDrawLine(renderer, cx - dx, cy - dy + RADIUS, cx + dx, cy - dy + RADIUS);
	}

}


static void update_state(State *state){
	double num1 = -G * (2 * state->ms[0] + state->ms[1]) * sin(state->as[0]);
	double num2 = -state->ms[1] * G * sin(state->as[0] -2 * state->as[1]);
	double num3 = -2 *sin(state->as[0] - state->as[1]) * state->ms[1];
	double num4 = state->avs[1] * state->avs[1] * state->ls[1]
				+ state->avs[0] * state->avs[0] * state->ls[0] * cos(state->as[0] - state->as[1]);
	double den = state->ls[0] * (2 * state->ms[0] + state->ms[1] - state->ms[1] * cos(2 *
		state->as[0] - 2 * state->as[1]));

	double update1 = (num1 + num2 + num3 * num4) / den;

	double num1_2 = 2 * sin(state->as[0] - state->as[1]);
	double num2_2 = (state->avs[0] * state->avs[0] * state->ls[0] * (state->ms[0] + state->ms[1]));
	double num3_2 = G * (state->ms[0] + state->ms[1]) * cos(state->as[0]);
	double num4_2 = state->avs[1] * state->avs[1] * state->ls[1] * state->ms[1] * cos(state->as[0] - state->as[1]);
	double den_2 = state->ls[1] * (2 * state->ms[0] + state->ms[1] - state->ms[1] * cos(2 * state->as[0] - 2
		* state->as[1]));

	double update2 = (num1_2 * (num2_2+num3_2+num4_2)) / den_2;

	state->avs[0] += update1;
	state->avs[1] += update2;
	state->as[0] += state->avs[0];
	state->as[1] += state->avs[1];
}

void draw_path(SDL_Renderer *renderer, uint32_t *canvas, SDL_Texture *texture, double x, double y){
	canvas[(int)(floor(x) * WIDTH + floor(y))] = 0x1d1d1b;
	SDL_UpdateTexture(texture, 0, canvas, sizeof(uint32_t) * WIDTH);

	/* draw the canvas */
	SDL_RenderCopy(renderer, texture, NULL, NULL);
}

static void draw_pen(State *state, SDL_Renderer *renderer,SDL_Texture *texture){
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);

	double x1_1 = (double) WIDTH/2;
	double y1_1 = (double) OFFSET;
	double x2_1 = x1_1 + state->ls[0] * sin(state->as[0]);
	double y2_1 = y1_1 + state->ls[0] * cos(state->as[0]);

	double ballX = x1_1 + state->ls[0] * sin(state->as[0]);
	double ballY = OFFSET + state->ls[0] * cos(state->as[0]);

	double x1_2 = ballX + RADIUS * sin(state->as[0]);
	double y1_2 = ballY + RADIUS * cos(state->as[0]);
	double x2_2 = x1_2 + state->ls[1] * sin(state->as[1]);
	double y2_2 = y1_2 + state->ls[1] * cos(state->as[1]);

	draw_path(renderer, state->canvas, texture, y2_2, x2_2);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderDrawLine(renderer, x1_1, y1_1, x2_1, y2_1);
	draw_circle(renderer, (uint32_t)x2_1, (uint32_t)y2_1);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderDrawLine(renderer, x1_2, y1_2, x2_2, y2_2);
	draw_circle(renderer, (uint32_t)x2_2, (uint32_t)y2_2);
	SDL_RenderPresent(renderer);
}


State create_state(double l1,double l2, double m1, double m2, double a1, double a2){
	State temp;
	temp.ls = (double*)malloc(sizeof(double) * 2);
	temp.ls[0] = l1;
	temp.ls[1] = l2;
	temp.ms = (double*)malloc(sizeof(double) * 2);
	temp.ms[0] = m1;
	temp.ms[1] = m2;
	temp.as = (double*)malloc(sizeof(double) * 2);
	temp.as[0] = a1 * (M_PI/180.0); //converting degrees to radians
	temp.as[1] = a2 * (M_PI/180.0);
	temp.avs = (double*)malloc(sizeof(double) * 2);
	temp.avs[0] = 0.0;
	temp.avs[1] = 0.0;
	temp.canvas = (uint32_t*)malloc(sizeof(uint32_t) * WIDTH * HEIGHT);
	memset(temp.canvas, 255, sizeof(uint32_t) * WIDTH * HEIGHT);
	return temp;

}

void free_state(State *state){
	free(state->ls);
	free(state->ms);
	free(state->as);
	free(state->avs);
	free(state->canvas);
}



int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		/* print error message to stderr */
		perror(SDL_GetError());
		return EXIT_FAILURE;
	}

	/* setup SDL */
#define create_SDL(type, var, ...) \
    type var = __VA_ARGS__;        \
    if (var == NULL)               \
    {                              \
        perror(SDL_GetError());	   \
        return EXIT_FAILURE;       \
    }

	create_SDL(SDL_Window *, window,
		SDL_CreateWindow("DOUBLE PENDULUM",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			WIDTH, HEIGHT, SDL_WINDOW_OPENGL));

	create_SDL(SDL_Renderer *, renderer,
		SDL_CreateRenderer(window, -1,
			SDL_RENDERER_ACCELERATED));

	create_SDL(SDL_Texture *, texture,
		SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			WIDTH, HEIGHT));

#undef create_SDL
	int quit = 0;
	//State state = create_state(100.0,100.0,5.0,10.0,45.0,90.0);
	State state = create_state(150.0,150.0,20.0,20.0,180,90.0);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

	while (!quit)
	{
		for (SDL_Event event = { 0 }; SDL_PollEvent(&event); )
			switch (event.type)
			{
				case SDL_QUIT:
				{
					quit = 1;
				} break;
			}
		draw_pen(&state,renderer, texture);
			update_state(&state);
	}


	/* cleanup SDL and free memory */
	free_state(&state);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
//gcc main.c -o runthis -I /usr/local/include/SDL2 -L /usr/local/lib -lSDL2
