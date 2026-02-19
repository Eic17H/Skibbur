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

const int Y0 = 112;		// Startin Y
const int X0 = 32;		// Starting X
const int G  = 2;		// Gravitational acceleration

int Situation = 0; // 0=title | 1=menu | 2=extras | 3=game | 4=lose
bool Pause = 0;
int Enemy = 0, EnemyX = 200;
int HoleW = 7, NewW=0, rng=0, speed=2;
int score = 0;

void title()									// title screen: "SKIBBUR"
{
	printf("\n\n\n\n\n\n\n\n\n\n    SKIBBUR\n\n\n\n\n\n\n\n");
	bool keep = true;
	while(keep)
	{
		if(joypad()&J_A||joypad()&J_START)	// if A or START, start the game
			keep = false;
    };
	Situation = 1;								// main menu
}


void menu()										// main menu
{
	score = 0;
    int choice = 0;								// 0=game | 1=extras
    const int CHOICENO = 2;						// number of options
    bool keep = true;
	printf("\n\n\n\n\n\n\n    Play\n\n\n    Extras\n\n\n\n\n\n\n\n");
	set_sprite_data(0, 1, Cursor);
	set_sprite_tile(0, 0);
	SHOW_SPRITES;
    while(keep)
    {
		move_sprite(0, 32, 2*32+24*choice);

		if(joypad()&J_UP)		choice = 0;
		delay(60);
		if(joypad()&J_DOWN)	choice = 1;

		if(joypad()&J_A||joypad()&J_START)	// select option
		{
			switch(choice)
			{
				case 0:
					Situation = 3;				// game
					break;
				case 1:
					Situation = 2;				// extras
					break;
				default:
					Situation = 0;				// title screen
					break;
			}
			keep = false;
		}

		if(joypad()&J_B)						// go back
		{
			Situation = 0;						// title screen
			keep = false;
		}
	}

	HIDE_SPRITES;
}


void extras()									// credits, info, etc
{
	bool keep = true;
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nVedro' che scrivere\n2022 TSOR\nSkibbur");
	while(keep)
	{
		if(joypad()&J_B)						// go back
		{
			Situation = 1;						// menu
			keep = false;
		}
	}
}


void game()										// the game
{
	bool keep = true;
	int vel = 0, GroundNow = Y0;
	int spr = 0;
	int rng;
	int spriteset=0;

	set_bkg_data(0, 7, Ground);
	set_bkg_tiles(0, 0, 20, 18, TestMap);

	set_sprite_data(0, 10, Skateboard);
	set_sprite_tile(0, 2*spr);
	set_sprite_tile(1, 2*spr+1);
	int x = X0, y = Y0;
	move_sprite(0, x, y);
	move_sprite(1, x+8, y);	
	int i,j;

	SHOW_SPRITES;

	while(keep)
	{
		{// move and set skateboard sprites
		move_sprite(0, x, y);									// move left sprite
		move_sprite(1, x+8, y);									// move right sprite
		set_sprite_tile(0, 2*spr-2);							// set left sprite
		set_sprite_tile(1, 2*spr-1);							// set right sprite
		}

		{// set sprite based on velocity
		if(vel==0) spr = 3;
		if(vel> 0) spr = 4;
		if(vel> 5) spr = 5;
		if(vel< 0) spr = 2;
		if(vel<-5) spr = 1;
		}

		if(!Pause){												// make sure things don't happen when paused

		score++;

		if(joypad()&J_A && y==GroundNow)	{vel = -10;};		// input A: accelerate upwards
		y += vel;												// change position according to velocity

		{// influence speed
			if     (joypad()&J_LEFT ) speed = 1;
			else if(joypad()&J_RIGHT) speed = 3;
			else                      speed = 2;
		}
		
		{//interact with the ground and with obstacles
			if(y>GroundNow+16)	{Situation = 4; keep = 0; vel=0;};				// lose
			if(y>GroundNow&&keep==1)		{y = GroundNow; vel = 0;};			// if slightly below the ground, stop and land
			if(y<GroundNow)		{vel += G;};									// if above the ground, accelerate
			if(y==GroundNow)	{vel = 0;};										// if on the ground, stop
			if(Enemy>0)			{EnemyX = EnemyX - 5*speed;};					// move obstacle
			if(EnemyX <= -16*HoleW && Enemy == 1) Enemy = 0;
		}
		
		{// generate obstacles
			if(Enemy==0) {rng = rand()%20; EnemyX = 200; NewW=rand()%2;};
			spriteset=0;
			if(rng == 1)
			{
				// buco
				Enemy = 1;
				set_sprite_data(10, 16, Hole2);

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
						move_sprite(3+j+5*i, EnemyX+8*i, Y0+8+8*j);
					}
					if(EnemyX <= -8*HoleW) Enemy = 0;
					if(EnemyX-8*HoleW<X0 && EnemyX>X0) GroundNow = Y0+24; else GroundNow = Y0;
				}
			};
			if(rng == 2)
			{
				// rampa
			}
		}
		};

		if(joypad()&J_START) Pause = !Pause;					// input START: (un)pause
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
	while(abs(x-80)>=1||abs(y-72)>=1)						// move skateboard to the center on the screen
		{
			x=(81+x)/2;
			y=(72+y)/2;
			move_sprite(0, x, y);
			move_sprite(1, x+8, y);
			delay(30);
		};
	}
	return;
}


void lose()										// lose screen
{
	bool keep = true;
	printf("\n\n\n\n\n\n\n\n     GAME  OVER\n\n\n\n\n\n\n\n\n\n");
	while(keep)
	{
		if(joypad()&J_A||joypad()&J_START)
		{
			keep = false;
		}
	};
	Situation = 1;
	set_sprite_tile(0, 10);							// set left sprite
	set_sprite_tile(1, 10);							// set right sprite
	return;
}


void main()
{
	while(1)
	{
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
		switch(Situation)
		{
			case 0:
				title();
				break;
			case 1:
				menu();
				break;
			case 2:
				extras();
				break;
			case 3:
				game();
				break;
			case 4:
				lose();
				break;
		}
	}
}