#include <stdio.h>
#include <SDL2/SDL.h>
#include <switch.h>

#define WIDTH 1280
#define HEIGHT 720

SDL_Window* win;
SDL_Renderer* rend;
const int sizeX = 200;
const int sizeY = 200;

int viewX = 50;
int viewY = 50;

bool table[sizeX][sizeY];
bool nextTable[sizeX][sizeY];
int neighbours;

u32 lastGen;
float speed=1.00;
bool generateNext = false;
bool keepGenerating = false;

bool editMode = false;
int cursorX = 50;
int cursorY = 50;
bool showGrid = true;
bool nightMode = false;

int init();
void setColor(int index);


int main(void){
	//Init everything
	if(init()<0){
		return 0;
	}

	//Create box SDL_Rect
	SDL_Rect boxRect;
	int boxSize = 30;

	//Clear table array
	for(int i=0;i<sizeX;i++){
		for(int j=0;j<sizeY;j++){
			table[i][j] = 0;
		}
	}
	table[55][55]=1;
	table[56][55]=1;
	table[56][56]=1;
	table[56][57]=1;
	table[57][56]=1;


	//Game loop
	bool running = true;
	SDL_Event event;
	while(running){
		
		//Event handeling
		while(SDL_PollEvent(&event)!=0){
			switch(event.type){
				case SDL_QUIT:
					running = false;
					break;
				case SDL_JOYBUTTONDOWN:
					printf("You pressed %d button!\n", event.jbutton.button);
					switch(event.jbutton.button){
						case 10:
							running = false;
							break;
						//LZ and RZ
						case 9:
							if(boxSize<80){ boxSize+=5; }
							break;
						case 8:
							if(boxSize>10){ boxSize-=5; }
							break;
						//Arrows
						case 12:
							if(viewX>-30){ viewX-=2; }
							break;
						case 13:
							if(viewY>-30){ viewY-=2; }
							break;
						case 14:
							if(viewX<200){ viewX+=2; }
							break;
						case 15:
							if(viewY<200){ viewY+=2; }
							break;
						//Generating
						case 0:	//A
							if(editMode){ table[cursorX][cursorY]=!table[cursorX][cursorY]; }
							else{ generateNext=true; }
							break;
						case 1:	//B
							keepGenerating=!keepGenerating&!editMode;
							generateNext=true&!editMode;
							break;
						case 3:
							showGrid=!showGrid;
							break;
						//Set speed
						case 6:	//L
							if(speed<4){ speed+=0.25; }
							break;
						case 7:	//R
							if(speed>0){ speed-=0.25; }
							break;
						//Edit mode
						case 2:	//X
							editMode=!editMode;
							keepGenerating=false;
							generateNext=false;
							break;
						//Right Joystick
						case 20:
							if(cursorX>0 && editMode){ cursorX--; }
							break;
						case 21:
							if(cursorY>0 && editMode){ cursorY--; }
							break;
						case 22:
							if(cursorX<sizeX && editMode){ cursorX++; }
							break;
						case 23:
							if(cursorY<sizeY && editMode){ cursorY++; }
							break;
						//Change theme
						case 5:
							nightMode=!nightMode;
							break;
					}
					break;
			}
		}

		//Game processing
		boxRect.w=boxSize;
		boxRect.h=boxSize;

		if(generateNext&&!editMode){
				lastGen = SDL_GetTicks();

				for(int i=0; i<sizeX; i++){
					for(int j=0; j<sizeY; j++){
						nextTable[i][j]=table[i][j];
					}
				}
		
				for(int i=1; i<sizeX-1; i++){
					for(int j=1; j<sizeY-1; j++){
						neighbours = 	(1*table[i-1][j-1]+
										1*table[i][j-1]+
										1*table[i+1][j-1]+
										1*table[i-1][j]+
										1*table[i+1][j]+
										1*table[i-1][j+1]+
										1*table[i][j+1]+
										1*table[i+1][j+1]);
		
						if((table[i][j]==0)&&(neighbours==3)){
							nextTable[i][j]=1;
						}
						if(table[i][j]==1){
							if((neighbours==2)||(neighbours==3)){
								nextTable[i][j]=1;
							} else {
								nextTable[i][j]=0;
							}
						}
					}
				}
				for(int i=0; i<sizeX; i++){
					for(int j=0; j<sizeY; j++){
						table[i][j]=nextTable[i][j];
					}
				}
				generateNext=false;
		}

		if((SDL_GetTicks()>=lastGen+speed*100)&&keepGenerating) { generateNext=true; }


		/////////////
		//Rendering//
		/////////////
		if(showGrid){ setColor(1); }
		else{ setColor(0); }
		SDL_RenderClear(rend);
		setColor(0);

		for(int x=0;x<=WIDTH/boxSize;x++){
			for(int y=0;y<=HEIGHT/boxSize;y++){
				
				boxRect.x=x*boxSize+x;
				boxRect.y=y*boxSize+y;


				//color
				if( (viewX+x<sizeX)&&(viewY+y<sizeY) && (0<=viewX+x)&&(0<=viewY+y) ){
					if(table[viewX+x][viewY+y]==1){ setColor(1); }
					else{ setColor(0); }
				} else {
					SDL_SetRenderDrawColor(rend, 200,200,200,255); 
				}

				if((viewX+x==cursorX && viewY+y==cursorY)&&editMode){
					SDL_SetRenderDrawColor(rend, 200,50,50,255);
				}

				//render
				SDL_RenderDrawRect(rend,&boxRect);
				SDL_RenderFillRect(rend,&boxRect);
			}
		}
		SDL_RenderPresent(rend);

		//printf("Speed: %f\n", speed);

	}

	//Close app
	SDL_Quit();
	SDL_DestroyWindow(win); SDL_DestroyRenderer(rend);
	socketExit();
	return 0;
}

int init(){
	//Init nxlink communication socket
	socketInitializeDefault();
	nxlinkStdio();

	//Init SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)<0){ SDL_Log("Error in SDL_Init(): %s\n",
														SDL_GetError());
														return -1; }

	win = SDL_CreateWindow("grid",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,WIDTH,HEIGHT,SDL_WINDOW_SHOWN);
	if(win==NULL){ SDL_Log("Error creating window: %s\n",
					SDL_GetError()); SDL_Quit();
					return -1; }

	rend = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);
	if(rend==NULL){ SDL_Log("Error creating renderer: %s\n", SDL_GetError());
					SDL_DestroyWindow(win); SDL_Quit();
					return -1; }

	//Init Joystick
	for(int i=0;i<=4;i++){
		if(SDL_JoystickOpen(i)==NULL){
			SDL_Log("No joystick connected!\n");
			SDL_DestroyWindow(win);
			SDL_DestroyRenderer(rend);
			SDL_Quit();
			return -1;
		}
	}
	return 0;

}

void setColor(int index){
	switch(index){
		case 0:
			SDL_SetRenderDrawColor(rend,255*!nightMode,255*!nightMode,255*!nightMode,255);
			break;
		case 1:
			SDL_SetRenderDrawColor(rend,255*nightMode,255*nightMode,255*nightMode,255);
			break;
	}
}