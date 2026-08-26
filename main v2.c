#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <rand.h>
#include <gb/gb.h>
#include "Cursor.c"
#include "GroundMap.c"
#include "GroundTiles.c"
#include "Ground.c"
#include "Map1.c"
#include "Skateboard.c"
#include "Hole.c"
#include "Hole2.c"

// ====== GAME CONSTANTS ======
#define Y0 112		// Starting Y
#define X0  32		// Starting X
#define G    2		// Gravitational acceleration
#define JUMPST -10	// Jump strength

// ===== SYSTEM CONSTANTS =====
// Screen width and height
#define SCREENW 160
#define SCREENH 144
// Menu selection (OK) button
#define MENUSEL joypad()&J_A||joypad()&J_START
// Jump button
#define JUMP joypad()&J_A
// Back button joypad()&J_B
#define BACK 
// Normal buttons:
#define START joypad()&J_START
#define SELECT joypad()&J_SELECT
#define UP joypad()&J_UP
#define DOWN joypad()&J_DOWN
#define LEFT joypad()&J_LEFT
#define RIGHT joypad()&J_RIGHT
#define A joypad()&J_A
#define B joypad()&J_B
// The size of a tile
#define TILESIZE 8

// Wait until a is pressed
#define waitFor(a) while(keep) if(a) keep=false
#define waitForAnd(a, b) while(keep) if(a) {b keep=false}

typedef enum {TITLE, MENU, EXTRAS, GAME, LOSE} Situation;
typedef enum {JUMP2=1, JUMP1, STILL, FALL1, FALL2} Sprite;
typedef enum {NONE, HOLE} Obstacle;

Situation situation = TITLE;
bool Pause = 0;
Obstacle Enemy = 0, EnemyX = 200;
int HoleW = 7, NewW=0, rng=0, speed=2;
int score = 0;

void title()
{
	printf("\n\n\n\n\n\n\n\n\n\n    SKIBBUR\n\n\n\n\n\n\n\n");
	bool keep = true;
	waitForAnd(MENUSEL, situation = MENU;);
}


void menu()		// main menu
{
	// Reset score and cursor position, set the number of choices, show text, show cursor
	score = 0;
    int choice = 0; // 0=game | 1=extras
    const int CHOICENO = 2;
    bool keep = true;
	printf("\n\n\n\n\n\n\n    Play\n\n\n    Extras\n\n\n\n\n\n\n\n");
    /**
     * Non ricordo bene come funzioni ma mi sa che:
     * Partendo dallo slot 0, carica 1 tile dal tileset Cursor;
     * Imposta allo slot 0 il tile numero 0.
     */
	set_sprite_data(0, 1, Cursor);
	set_sprite_tile(0, 0);
	SHOW_SPRITES;
	// Loop
    while(keep)
    {
    	// Move cursor up or down, wrapping around
		if(UP){
			choice = (choice+CHOICENO+1)%CHOICENO;
			move_sprite(0, 32, 2*32+24*choice);
		}
		delay(60);
		if(DOWN){
			choice = (choice+CHOICENO-1)%CHOICENO;
			move_sprite(0, 32, 2*32+24*choice);
		}

		// When the selection button is pressed, the loop is stopped and another situation is selected
		if(MENUSEL)
		{
			keep = false;
			switch(choice)
			{
				case 0:
					situation = GAME;
					break;
				case 1:
					situation = EXTRAS;
					break;
				default:
					situation = TITLE;
					break;
			}
		}
		// When the back button is pressed, go to the menu
		waitForAnd(BACK, situation=MENU;)
	}
	// Empty screen
	HIDE_SPRITES;
}


void extras()	// credits, info, etc
{
	bool keep = true;
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nVedro' che scrivere\n2022 TSOR\nSkibbur");
	// Back to the menu
	waitForAnd(BACK, situation=MENU;)
}


void game()
{
	bool keep = true;	// Keep looping
	int vel = 0;		// Downwards velocity
	int GroundNow = Y0;	// Current height of the ground
	Sprite spr = STILL;	// Current sprite
	int rng=0;			// Stores the current rng value for later use
	int spriteset=0;	// Obstacle spriteset

	// Setting tileset and map
	set_bkg_data(0, 7, Ground);
	set_bkg_tiles(0, 0, 20, 18, TestMap);

	// Setting player tileset and sprites
	set_sprite_data(0, 10, Skateboard);
	set_sprite_tile(0, 2*spr);
	set_sprite_tile(1, 2*spr+1);
	
	// Initializing position
	int x = X0, y = Y0;
	move_sprite(0, x, y);
	move_sprite(1, x+TILESIZE, y);	
	int i=0, j=0;

	SHOW_SPRITES;

	while(keep)
	{
		{// set sprite based on velocity
		if(vel<0)
			if(vel>-5)
				spr = JUMP1;
			else
				spr = JUMP2;
		else if(vel==0)
			spr = STILL;
		else if(vel>0)
			if(vel<5)
				spr = FALL1;
			else
				spr = FALL2;
		}
		
		{// move and set skateboard sprites
		move_sprite(0, x, y);			// move left sprite
		move_sprite(1, x+TILESIZE, y);			// move right sprite
		set_sprite_tile(0, 2*spr-2);	// set left sprite
		set_sprite_tile(1, 2*spr-1);	// set right sprite
		}

		if(!Pause){												// make sure things don't happen when paused

			// If you play faster, you get more points
			score += speed;
	
			// jump button
			if(JUMP && y==GroundNow) vel = JUMPST;
			// change position according to velocity
			y += vel;
	
			{// influence speed
				if      (LEFT ) speed = 1;
				else if (RIGHT) speed = 3;
				else            speed = 2;
			}
			
			{//interact with the ground and with obstacles
				if(y>GroundNow+2*TILESIZE)						{situation = LOSE; keep = 0; vel=0;}			// if far below the ground, lose
				if(y>GroundNow&&keep==1)				{y = GroundNow; vel = 0;}			// if slightly below the ground, stop and land
				if(y<GroundNow)							vel += G;									// if above the ground, accelerate downwards
				if(y==GroundNow)						vel = 0;										// if on the ground, stop
				if(Enemy>0)								EnemyX = EnemyX - 5*speed;					// move obstacle
				if(EnemyX <= -2*TILESIZE*HoleW && Enemy == 1)	Enemy = 0;
				
				if(y>GroundNow+2*TILESIZE){ // Far below the ground
					situation = LOSE;
					keep = 0;
					vel = 0;
				}else if(y>GroundNow&&keep==1){ // If slightly below the ground, pretend you landed safely
					y = GroundNow;
					vel = 0;
				}else if(y<GroundNow) // If above the ground, accelerate downwards
					vel += G;
				else if(y==GroundNow){ // If on the ground, stop
					vel = 0;
				}
				
				if(Enemy!=NONE) // If there is an enemy, move it to the left
					EnemyX -= 5*speed;
				else if(EnemyX <= -2*TILESIZE*HoleW && Enemy == HOLE) // If the enemy is offscreen, delete it
					Enemy = NONE;
			}
			
			{// generate obstacles
				if(Enemy==0) {rng = rand()%20; EnemyX = 200; NewW=rand()%2;};
				spriteset=0;
				if(rng == HOLE)
				{
					// hole
					Enemy = HOLE;
					set_sprite_data(10, 2*TILESIZE, Hole2);
	
					for(i=0; i<HoleW-NewW&&!spriteset; i++)
					{
						for(j=0; j<5; j++)
						{
							if(i*5+j>=0					&&		i*5+j<=5)				// left border
								set_sprite_tile(3+j+5*i, 10+j+1);
							if(i*5+j>=5					&&		i*5+j<5*(HoleW-NewW-1))	// middle
								set_sprite_tile(3+j+5*i, 10+j+5+1);
							if(i*5+j>=5*(HoleW-NewW-1)	&&		i*5+j<5*(HoleW-NewW))	// right border
								set_sprite_tile(3+j+5*i, 10+j+10+1);
						}
					}
	
					for(i=0; i<HoleW-NewW; i++)
					{
						for(j=0; j<5; j++)
						{
							move_sprite(3+j+5*i, EnemyX+TILESIZE*i, Y0+TILESIZE+TILESIZE*j);
						}
						if(EnemyX <= -TILESIZE*HoleW) Enemy = 0;
						if(EnemyX-TILESIZE*HoleW<X0 && EnemyX>X0) GroundNow = Y0+24; else GroundNow = Y0;
					}
				};
				if(rng == 2)
				{
					// rampa
				}
			}
		};

		if(START) Pause = !Pause;					// input START: (un)pause
		delay(30);												// don't make the game go too fast
	};
	{// lose
	set_sprite_tile(0, 4);									// set left  skateboard appearence
	set_sprite_tile(1, 5);									// set right skateboard appearence
	for(i=0; i<HoleW; i++)									// hide hole
				{
					set_sprite_tile(3+5*i, 10);
					set_sprite_tile(4+5*i, 10);
					set_sprite_tile(5+5*i, 10);
					set_sprite_tile(6+5*i, 10);
					set_sprite_tile(7+5*i, 10);
				}
	// Move skateboard to the center on the screen
	while(abs(x-SCREENW/2)>=1||abs(y-SCREENH/2)>=1)
		{
			// Smoothly move it to the center of the screen
			// (sometimes it gets stuck if you don't add +1 to the width
			x=(SCREENW/2+1+x)/2;
			y=(SCREENH/2+y)/2;
			move_sprite(0, x, y);
			move_sprite(1, x+TILESIZE, y);
			delay(30);
		};
	}
	return;
}


void lose()										// lose screen
{
	bool keep = true;
	printf("\n\n\n\n\n\n\n\n     GAME  OVER\n\n\n\n\n\n\n\n\n\n");
	waitFor(MENUSEL);
	situation = MENU;
	set_sprite_tile(0, 10);							// set left sprite
	set_sprite_tile(1, 10);							// set right sprite
	return;
}


void main()
{
	while(1)
	{
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
		switch(situation)
		{
			case TITLE:
				title();
				break;
			case MENU:
				menu();
				break;
			case EXTRAS:
				extras();
				break;
			case GAME:
				game();
				break;
			case LOSE:
				lose();
				break;
		}
	}
}
