//Ant Simulation is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Authors: Guillaume Martinet, Lucas Tittmann
// Date: 01/2013

// Some code is based on the tutorials from:
//http://www.youtube.com/user/thecplusplusguy
// If so it is stated in the header. The original files are distributed under LPGL.
//
//The source files are also GPL v3.0 with 2 exceptions:
//grass.bmp and sand.bmp are taken from
//http://www.public-domain-image.com/full-image/textures-and-patterns-public-domain-images-pictures/grass-texture-public-domain-images-pictures/buffalo-grass-texture.jpg-copyright-friendly-photo.html
//http://www.public-domain-image.com/textures-and-patterns-public-domain-images-pictures/earth-texture-public-domain-images-pictures/yellow-sand-texture.jpg.html
//by Titus Tscharntke

#pragma once
#include "windows.h"
#include <SDL.h>
#include "SDL_image.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"
#include <cmath>
#include <iostream>

// our external depencies
#include "camera.h"
#include "skybox.h"
#include "models.h"

using namespace std;

// user cahngeable parameters
int FPS = 40;
const int screen_width = 960;
const int screen_height = 640;
const int ant_number = 1000;
const int board_size = 1000;
int cam_velocity = 1;

// system variables
int round_cnt = 0;
bool mousein = false;
unsigned int tex_board;
unsigned int tex_border;
Uint8 *keystates = SDL_GetKeyState( NULL );

// just for testing / not important
float ant_posx[ant_number];
float ant_posz[ant_number];
float ant_posy = 2;
float ant_size = 4;
float ant_angley = 180;

// advanced image load (also JPEG and PNG)
SDL_Surface *load_image( std::string filename ) { 
	//The image that's loaded 
	SDL_Surface* loadedImage = NULL; 
	//The optimized image that will be used 
	SDL_Surface* optimizedImage = NULL; 
	//Load the image using SDL_image 
	loadedImage = IMG_Load( filename.c_str() ); 
	//If the image loaded 
	if( loadedImage != NULL ) { 
		//Create an optimized image 
		optimizedImage = SDL_DisplayFormat( loadedImage ); 
		//Free the old image 
		SDL_FreeSurface( loadedImage ); 
	} //Return the optimized image 
	return optimizedImage; 
}

void move_ants()
{
	int velocity = (round_cnt%450)/45;	 
	switch(velocity) {
	case 0:
		ant_size += 0.2;
		ant_posy += 0.1; break;
	case 1:
		for (int cnt = 0; cnt < ant_number; cnt++) {ant_posz[cnt] += 1;}
		break;
	case 3:
		for (int cnt = 0; cnt < ant_number; cnt++) {ant_posx[cnt] += 1;}
		break;
	case 5:
		ant_size -= 0.2;
		ant_posy -= 0.1; break;
	case 6:
		for (int cnt = 0; cnt < ant_number; cnt++) {ant_posz[cnt] -= 1;}
		break;
	case 8:
		for (int cnt = 0; cnt < ant_number; cnt++) {ant_posx[cnt] -= 1;}
		break;
	default:
		ant_angley += 90/45; break;
	}
}

void init()
{
	SDL_WM_SetCaption( "Ant Simulation", NULL );
	glClearColor(0.2,0.2,0.8,1.0); //background color and alpha
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,(1.0*screen_width)/screen_height,1.0,5000.0);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE,GL_LINEAR);
	glFogf(GL_FOG_START,100.0);
	glFogf(GL_FOG_END,800.0);
	float fog_color[] = {0.8,0.8,1.0,0.7};
	glFogfv(GL_FOG_COLOR,fog_color);
	init_skybox();
	tex_border=load_texture("src/border.bmp", false);
	tex_board=load_texture("src/grass.bmp", false);

	for (int cnt = 0; cnt < ant_number; cnt++)
	{
		ant_posx[cnt] = rand() % (board_size-40) + 20;
		ant_posz[cnt] = rand() % (board_size-40) + 20;
	}
}

void display()
{
	// in color_buffer the color of every pixel is saved
	// in depth buffer the depth of every pixel is saved (which px is in front of which)
	// usually, it makes sense to clean the buffers after every frame
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	int recent_cam_velocity = 1;
	if(keystates[SDLK_LSHIFT])
		recent_cam_velocity *= 4;
	camera_control(recent_cam_velocity,0.5,board_size,screen_width,screen_height,mousein);
	draw_skybox(3000.0); // don't make it bigger than the far-view-plane (see gluPerspective)
	update_camera();
	draw_board(board_size, tex_board);
	draw_border(board_size, tex_border);

	for (int cnt = 0; cnt < ant_number; cnt++) 
	{
		glPushMatrix();
			glTranslatef(ant_posx[cnt],ant_posy,ant_posz[cnt]);
			glRotatef(ant_angley,0.0,1.0,0.0);
			draw_ant(ant_size);
		glPopMatrix();
	}
}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Surface *screen;
	// SDL_SWSURFACE|SDL_OPENGL means: do both options
	screen = SDL_SetVideoMode(screen_width, screen_height, 32, SDL_SWSURFACE|SDL_OPENGL);
	//screen = SDL_SetVideoMode(screen_width, screen_height, 32, SDL_SWSURFACE|SDL_FULLSCREEN);
	
	bool running = true;
	Uint32 start;
	Uint32 time_before_last_100_frames = 0;
	SDL_Event event;
	init();

	while(running) {
		start = SDL_GetTicks();
		round_cnt += 1;
		////////////////////////////////////////////////////////
		/////////////         EVENT HANDLING      //////////////
		////////////////////////////////////////////////////////
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					running = false; break;
				case SDL_MOUSEBUTTONDOWN:
					mousein = true;
					SDL_ShowCursor(SDL_DISABLE);
					SDL_WarpMouse(screen_width/2,screen_height/2);
					break;
				case SDL_MOUSEBUTTONUP:
					mousein = false;
					SDL_ShowCursor(SDL_ENABLE);
					break;
				case SDL_KEYUP:
					switch(event.key.keysym.sym) {
						case SDLK_p:
							print_camera_pos(); break;	
						case SDLK_n:
							FPS = max(FPS-20,10); break;						
						case SDLK_m:
							FPS = min(FPS+20,100); break;
						case SDLK_ESCAPE:
							running = false; break;
						default:
							break;
				    }
					break;
				default:
					break;
			}
		}
 
		////////////////////////////////////////////////////////
		/////////////            GAME LOGIC       //////////////
		////////////////////////////////////////////////////////

		move_ants();

		////////////////////////////////////////////////////////
		/////////////        GRAPHIC RENDERING    //////////////
		////////////////////////////////////////////////////////
		display();
		SDL_GL_SwapBuffers(); // blits the buffer to the screen
		
		// hold constant frame rate
		if(1000/FPS > SDL_GetTicks()-start) SDL_Delay(1000/FPS-(SDL_GetTicks()-start));
		
		// calculate and print frame rate
		if (round_cnt%100 == 0) {
			cout << "Average Frames Per Second: " << 100 / ( (SDL_GetTicks()-time_before_last_100_frames) / 1000.f ) <<endl;
			time_before_last_100_frames = SDL_GetTicks();
		}
	}
	SDL_Quit();
	kill_skybox();
	glDeleteTextures(1,&tex_board);
	glDeleteTextures(1,&tex_border);
	return 0;
}