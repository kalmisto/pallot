#include <sys/stat.h>

#include <SDL/SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define RADIUS	55
#define WIDHT	1024
#define HEIGHT	640
#define GRAV	0.25
#define	KIMMO	0.7
#define KATTO	55
#define POHJA	HEIGHT - RADIUS
#define O_REUNA	WIDHT - RADIUS
#define V_REUNA	RADIUS
#define PUOLIVALI WIDHT/2
#define TAUSTAN_LEVEYS 5680 /* = viimeisen rotaatiospritekuvan aloituskohta blittaukselle(koska blitataan surfacen vasemman yläkulman koordinaattien mukaan) */
#define round(p) ((p)>=0?(long)((p)+0.5):(long)((p)-0.5))

struct poltsi {
	long	x;		/* aluksen paikka screenillä */
	long	y;
	double	dy;		/* nopeusvektori */
	double	dx;
	int	rad;
};

double	time_scale;
double	matka_x, matka_y;
int	suunta;
static	SDL_Surface *screen;
static	SDL_Surface *luotain_surface;
static	SDL_Surface *background;
struct	poltsi	pallo5;

struct poltsi *pallo;
static SDL_Surface *kuva;

static void
alusta_pallot()
{
	int i = 0;
	int k = 0;
	for (i = 0; i < 5; i++) {
		pallo[i].x = PUOLIVALI - 300 + k;
		pallo[i].y = HEIGHT/2;
		pallo[i].dx = 0;
		pallo[i].dy = 0;
		pallo[i].rad = 44;
		k = k +130;
	}
}

static void
xpomppu(struct poltsi *h)
{
	if (h->x > O_REUNA) {
		h->x = O_REUNA;
		h->dx = h->dx * -KIMMO;
	}
	if (h->x < V_REUNA) {
		h->x = V_REUNA;
		h->dx = h->dx * -KIMMO;
	}



}

static void
ypomppu(struct poltsi *v)
{
	if (v->y > POHJA) {
		v->y = POHJA;
		v->dy = v->dy * -KIMMO;
	}
	if (v->y < KATTO) {
		v->y = KATTO;
		v->dy = v->dy * -KIMMO;
	}

}
static void
grav(struct poltsi *p)
{
	switch (suunta) {
		case 1:
			if (p->y < POHJA) {
				p->dy = p->dy + GRAV;
			}
			if (p->y >= POHJA && (p->dy >= -2.5 && p->dy <= 2.5)) {
				p->dy = 0;
				p->y = POHJA;
			}
			ypomppu(p);
			xpomppu(p);
			break;
		case 2:
			if (p->x < WIDHT - RADIUS) {
				p->dx = p->dx + GRAV;
			}
			if (p->x >= WIDHT - RADIUS && (p->dx >= -2.5 && p->dx <= 2.5)) {
				p->dx = 0;
				p->x = WIDHT - RADIUS;
			}
			xpomppu(p);
			ypomppu(p);
			break;
		case 3:
			if (p->y > KATTO) {
				p->dy = p->dy - GRAV;
			}
			if (p->y <= KATTO && (p->dy >= -2.5 && p->dy <= 2.5)) {
				p->dy = 0;
				p->y = KATTO;
			}
			ypomppu(p);
			xpomppu(p);
			break;
		case 4:
			if (p->x > RADIUS) {
				p->dx = p->dx - GRAV;
			}
			if (p->x <= RADIUS && (p->dx >= -2.5 && p->dx <= 2.5)) {
				p->dx = 0;
				p->x = RADIUS;
			}
			ypomppu(p);
			xpomppu(p);
			break;

	}
}

static void
draw_background()
{
	SDL_Rect src, dest;
	src.x = 0;
        src.y = 0;
        src.w = background->w;
        src.h = background->h;
        dest = src;

        SDL_BlitSurface(background, &src, screen, &dest);
}

static void
draw_pallo(SDL_Surface *bpic, struct poltsi *b)
{
	SDL_Rect src, dest;

	matka_y = b->dy * time_scale;
	b->y = b->y + round(matka_y);
	matka_x = b->dx * time_scale;
	b->x = b->x + round(matka_x);

	src.x = 0;
	src.y = 0;
	src.w = bpic->w;
	src.h = bpic->h;


      /* Alusta liikutetaan muuttamalla aluksen keskipisteen sijaintia. Aluksen piirtäminen
	 sen sijaan määräytyy aluksen kuvan vasemman yläkulman mukaan */
	dest.x = b->x - bpic->w / 2;
	dest.y = b->y - bpic->h / 2;
	dest.w = bpic->w;
	dest.h = bpic->h;
	SDL_BlitSurface(bpic, &src, screen, &dest);
}


static void
PlayGame()
{
	Uint8 *keystate;
	int quit = 0;
	int prev_ticks = 0, cur_ticks = 0; /* muuttujat ajastuksen ylläpitoon */

	/* muuttujat frameratea varten  */
	int start_time, end_time;
	int frames_drawn = 0;

	prev_ticks = SDL_GetTicks();

	while (quit==0) {
		int i = 0;
		/* Determine how many milliseco5nds have passed since
		   the last frame, and update our motion scaling. */

		prev_ticks = cur_ticks;
		cur_ticks = SDL_GetTicks();
		time_scale = (double)(cur_ticks-prev_ticks)/30.0;

		/* Update SDL's internal input state information. */
		/* SDL_PumpEvents(); */
		SDL_PumpEvents();

		/* Grab a snapshot of the keyboard. */
		keystate = SDL_GetKeyState(NULL);

		/* Respond to input. */
		if (keystate[SDLK_q] || keystate[SDLK_ESCAPE]) quit = 1;
		if (keystate[SDLK_LEFT]) suunta = 4;
		if (keystate[SDLK_UP]) suunta = 3;
		if (keystate[SDLK_RIGHT]) suunta = 2;
		if (keystate[SDLK_DOWN]) suunta = 1;

		for (i = 0; i < 5; i++) {
			grav(&pallo[i]);
		}

		i = 0;
		/* Forward and back arrow keys activate thrusters. */
		draw_background();
		for (i = 0; i < 5; i++) {
			draw_pallo(kuva[i], &pallo[i]);
		}

	        SDL_UpdateRect(screen, 0, 0, 0, 0);
	}
}

int
main(void)
{
	kuvat = calloc(5, sizeof(SDL_Surface));
	if (kuvat == NULL) {
		err(1, "muistin varaus kuville epäonnistui");		
	}
	alusta_pallot();
	suunta = 1;

	/* Initialise the SDL and error check */
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
		printf("Unable to initialise SDL: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
		// PFJ - Changed from return 1;
	}

	/* Ensure SDL_Quit is called on termination */
	atexit(SDL_Quit);

	/* Attempt to set a WIDHT x HEIGHT hicolor (16-bit) video mode */
	screen = SDL_SetVideoMode(WIDHT, HEIGHT, 8, SDL_SWSURFACE);
	if (screen == NULL)
		{
		printf("Unable to set video mode: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
		// PFJ - Changed from return 1;
	}

	/* Load the bitmaps */
	background = IMG_Load("bg1024.bmp");
	if (background == NULL)
		{
		printf("Unable to load background image\n");
		exit(EXIT_FAILURE);
		// Both lines changed to give a corrected exit and descriptive error
	}

	kuva[4] = IMG_Load("pallero5.gif");
	if (luotain_surface == NULL)
		{
		printf("Unable to load the pallero5.gif\n");
		exit(EXIT_FAILURE);
		// Both lines changed as above
	}

	kuva[3] = IMG_Load("pallero4.gif");
	if (luotain_surface == NULL)
		{
		printf("Unable to load the pallero4.gif\n");
		exit(EXIT_FAILURE);
		// Both lines changed as above
	}

	kuva[2] = IMG_Load("pallero3.gif");
	if (luotain_surface == NULL)
		{
		printf("Unable to load the pallero3.gif\n");
		exit(EXIT_FAILURE);
		// Both lines changed as above
	}

	kuva[1] = IMG_Load("pallero2.gif");
	if (luotain_surface == NULL)
		{
		printf("Unable to load the pallero2.gif\n");
		exit(EXIT_FAILURE);
		// Both lines changed as above
	}

	kuva[0] = IMG_Load("pallero1.gif");
	if (luotain_surface == NULL)
		{
		printf("Unable to load the pallero1.gif\n");
		exit(EXIT_FAILURE);
		// Both lines changed as above
	}
	alusta_pallot();
	PlayGame();
	SDL_FreeSurface(background);
	SDL_FreeSurface(luotain_surface);

	return 0;
}




