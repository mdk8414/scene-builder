//Multi-Object, Multi-Texture Example
//Stephen J. Guy, 2021

//This example demonstrates:
// Loading multiple models (a cube and a knot)
// Using multiple textures (wood and brick)
// Instancing (the teapot is drawn in two locations)
// Continuous keyboard input - arrows (moves knot up/down/left/right continuous when being held)
// Keyboard modifiers - shift (up/down arrows move knot in/out of screen when shift is pressed)
// Single key events - pressing 'c' changes color of a random teapot
// Mixing textures and colors for models
// Phong lighting
// Binding multiple textures to one shader

const char* INSTRUCTIONS = 
"***************\n"
"This demo shows multiple objects being draw at once along with user interaction.\n"
"\n"
"Up/down/left/right - Moves the knot.\n"
"c - Changes to teapot to a random color.\n"
"***************\n"
;

//Mac OS build: g++ multiObjectTest.cpp -x c glad/glad.c -g -F/Library/Frameworks -framework SDL2 -framework OpenGL -o MultiObjTest
//Linux build:  g++ multiObjectTest.cpp -x c glad/glad.c -g -lSDL2 -lSDL2main -lGL -ldl -I/usr/include/SDL2/ -o MultiObjTest

#include "glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
 #include <SDL2/SDL.h>
 #include <SDL2/SDL_opengl.h>
#else
 #include <SDL.h>
 #include <SDL_opengl.h>
#endif
#include <cstdio>

#define GLM_FORCE_RADIANS
#include <GLUT/glut.h>
//#include ""



#include "Object.h"

#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <unistd.h>
using namespace std;

int screenWidth = 800; 
int screenHeight = 600;  
float cur_time = 0;

struct Color{
  float r,g,b;

  Color(float r, float g, float b) : r(r), g(g), b(b) {}
  Color() : r(0), g(0), b(0) {}
};

//int startVert = 0;

class Model{

public:
	int numVerts;
    int start;
	float* model;

    Model(const std::string &fileName){
		start = 0;
        ifstream modelFile;
        modelFile.open(fileName);
        int numLines = 0;
        modelFile >> numLines;
        model = new float[numLines];
        for (int i = 0; i < numLines; i++){
            modelFile >> model[i];
        }
        printf("%d\n",numLines);
        numVerts = numLines/8;
        modelFile.close();

	//	model.start = startVert;
	//	startVert += numVerts;
    }

};

int isZero(double val) {
    if(val == 0){
		return 0;
	}
	return 1;
}
//SJG: Store the object coordinates
//You should have a representation for the state of each object

float turn_x, turn_y;
float time_past;
int mapWidth, mapHeight;
float keyTurn = 0;
bool goal = false;
glm::vec3 mapStart;
Object player;
std::vector<Model> models;
std::vector<Object> objects;
std::vector<Object> victoryMap;
//std::vector<Object> blocks;
Object* map;
bool have_keys[6] = {false,false,false,false,false,false};
char keys[6] = {'a', 'b', 'c', 'd', 'e', 'f'};
bool doors[6] = {false,false,false,false,false,false};
int cur_key = 0;
int cur_door = 0;
//MOUSE
int mouse_x = 0;
int mouse_y = 0;
float xoffset = 0.0f;
float yoffset = 0.0f;
bool selected = false;
bool scaling = false;
bool rotating = false;
int cur_selection = -1;
//std::vector<std::vector<Object>> objects;
glm::vec3 fwd;

bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;
void Win2PPM(int width, int height);

//srand(time(NULL));
float rand01(){
	return rand()/(float)RAND_MAX;
}

int texturedShader;

void drawGeometry(int shaderProgram, int model1_start, int model1_numVerts, int model2_start, int model2_numVerts);
//void draw(int shaderProgram, int model_start, int model_numVerts, int texNum, glm::vec3 pos, glm::vec3 scale);
void draw(int shaderProgram, Object obj, int texNum, glm::vec3 color);
void loadMap(string fileName);
void loadVictoryMap();
bool canWalk(float x, float y);
int pickUpKey(glm::vec3 pos);
int unlockDoor(glm::vec3 pos);
void goalReached();
int pickUp(glm::vec3 &start_pos);
glm::vec3 rayCast(int mouse_x, int mouse_y, glm::vec3 pos);
bool rayPlaneIntersection(Object obj);
bool raySphereIntersection(Object obj);
float distToSqr(glm::vec3 vec);
glm::vec3 hitIntersect(Object obj);
float hitT(Object obj);
//void mouse_click(int button, int state, int x, int y);
//void mouse_move(int x, int y);

int main(int argc, char *argv[]){
	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);
	
	//Load OpenGL extentions with GLAD
	if (gladLoadGLLoader(SDL_GL_GetProcAddress)){
		printf("\nOpenGL loaded\n");
		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n\n", glGetString(GL_VERSION));
	}
	else {
		printf("ERROR: Failed to initialize OpenGL context.\n");
		return -1;
	}
	
	//Here we will load two different model files 
	
	string mapName = "maps/blank.txt";
	ifstream in;
	in.open(mapName);
	in >> mapWidth >> mapHeight;
	in.close();

	Model teapot = Model("models/teapot.txt");
	Model cube = Model("models/cube.txt");
	Model knot = Model("models/knot.txt");
	Model sphere = Model("models/sphere.txt");
	models.push_back(teapot);
	models.push_back(cube);
	models.push_back(knot);
	models.push_back(sphere);

	
	//SJG: I load each model in a different array, then concatenate everything in one big array
	// This structure works, but there is room for improvement here. Eg., you should store the start
	// and end of each model a data structure or array somewhere.
	//Concatenate model arrays
	int totalNumVerts = 0;
	for(int i = 0; i < models.size(); i++){
		totalNumVerts += models[i].numVerts;
	}

	float* modelData = new float[totalNumVerts*8];
	int vertCount = 0;
	for(int i = 0; i < models.size(); i++){
		copy(models[i].model, models[i].model+models[i].numVerts*8, modelData + vertCount*8); //this line
		models[i].start = vertCount;
		vertCount += models[i].numVerts;
	}
	
	
	//// Allocate Texture 0 (Wood) ///////
	SDL_Surface* surface = SDL_LoadBMP("wood.bmp");
	if (surface==NULL){ //If it failed, print the error
        printf("Error: \"%s\"\n",SDL_GetError()); return 1;
    }
    GLuint tex0;
    glGenTextures(1, &tex0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //Load the texture into memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w,surface->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture
    
    SDL_FreeSurface(surface);
    //// End Allocate Texture ///////


	//// Allocate Texture 1 (Brick) ///////
	SDL_Surface* surface1 = SDL_LoadBMP("brick.bmp");
	if (surface==NULL){ //If it failed, print the error
        printf("Error: \"%s\"\n",SDL_GetError()); return 1;
    }
    GLuint tex1;
    glGenTextures(1, &tex1);
    
    //Load the texture into memory
    glActiveTexture(GL_TEXTURE1);
    
    glBindTexture(GL_TEXTURE_2D, tex1);
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //How to filter
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface1->w,surface1->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface1->pixels);
    glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture
    
    SDL_FreeSurface(surface1);
	//// End Allocate Texture ///////
	
	//Build a Vertex Array Object (VAO) to store mapping of shader attributes to VBO
	GLuint vao;
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context

	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	GLuint vbo[1];
	glGenBuffers(1, vbo);  //Create 1 buffer called vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
	glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo
	//GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
	//GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used
	
	texturedShader = InitShader("textured-Vertex.glsl", "textured-Fragment.glsl");	
	
	//Tell OpenGL how to set fragment shader input 
	GLint posAttrib = glGetAttribLocation(texturedShader, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
	  //Attribute, vals/attrib., type, isNormalized, stride, offset
	glEnableVertexAttribArray(posAttrib);
	
	//GLint colAttrib = glGetAttribLocation(phongShader, "inColor");
	//glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(colAttrib);
	
	GLint normAttrib = glGetAttribLocation(texturedShader, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(normAttrib);
	
	GLint texAttrib = glGetAttribLocation(texturedShader, "inTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

	GLint uniView = glGetUniformLocation(texturedShader, "view");
	GLint uniProj = glGetUniformLocation(texturedShader, "proj");

	glBindVertexArray(0); //Unbind the VAO in case we want to create a new one	
                       
	
	glEnable(GL_DEPTH_TEST);  

	printf("%s\n",INSTRUCTIONS);
	
	loadVictoryMap();
	loadMap(mapName);
	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	bool quit = false;

	//float step=0.4f;
	glm::vec3 move;
	glm::vec3 forward = glm::vec3(0,0,-1);
	float prev_time = 0;
	float prev_y;
	bool jumping = false;
	bool falling = false;
	Object obj;
	int model_num = 0;
	int tex_num = 0;
	//turn_x = 0;
	//glutMouseFunc(mouse_click);
	//glutPassiveMotionFunc(mouse_move);
	while (!quit){
		move = glm::vec3(0,0,0);
		turn_x = 0;
		turn_y = 0;
		fwd = forward;
		while (SDL_PollEvent(&windowEvent)){  //inspect all events in the queue
			if (windowEvent.type == SDL_QUIT) quit = true;
			//List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
			//Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE) 
				quit = true; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f){ //If "f" is pressed
				fullscreen = !fullscreen;
				SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen 
			}

			//SJG: Use key input to change the state of the object
			//     We can use the ".mod" flag to see if modifiers such as shift are pressed
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_1){ //If 0 is pressed
				model_num = 0; //teapot
				if(selected){
					objects[cur_selection].numVerts = models[model_num].numVerts;
					objects[cur_selection].index = models[model_num].start;
				}
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_2){ //If 1 is pressed
				model_num = 1; //cube
				if(selected){
					objects[cur_selection].numVerts = models[model_num].numVerts;
					objects[cur_selection].index = models[model_num].start;
				}
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_3){ //If 2 is pressed
				model_num = 2; //knot
				if(selected){
					objects[cur_selection].numVerts = models[model_num].numVerts;
					objects[cur_selection].index = models[model_num].start;
				}
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_4){ //If 3 is pressed
				model_num = 3; //sphere
				if(selected){
					objects[cur_selection].numVerts = models[model_num].numVerts;
					objects[cur_selection].index = models[model_num].start;
				}
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_5){
				tex_num = 0; //wood
				if(selected){
					objects[cur_selection].texNum = tex_num;
				}
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_6){
				tex_num = 1; //brick
				if(selected){
					objects[cur_selection].texNum = tex_num;
				}
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_SPACE){ //If "space" is pressed
				obj = Object(player.pos + 2.f*fwd, glm::vec3(0.5,0.5,0.5), glm::vec3(1,1,1), 0, models[model_num].start, models[model_num].numVerts, tex_num, glm::vec3(1,0,0), 'w');
				objects.push_back(obj);
			}
			/*if (windowEvent.type == SDL_MOUSEBUTTONUP){ //If "g" is pressed
				if(!selected)
					pickUp();
				
				if(selected){
					selected = false;
					for(int i = 0; i < objects.size(); i++){
						objects[i].isSelected = false;
					}
				}
			}*/
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_c){ //If "c" is pressed
				selected = false;
				for(int i = 0; i < objects.size(); i++){
					objects[i].isSelected = false;
				}
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_LSHIFT){ //If left shift is pressed
				scaling = true;
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_LSHIFT){ //If left shift is pressed
				scaling = false;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_z){ //If "z" is pressed
				rotating = true;
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_z){ //If "z" is pressed
				rotating = false;
			}

		}
		if(!goal){
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) {
				move = glm::vec3(forward.x, forward.y, forward.z);
				printf("\nposition: (%f,%f,%f)\n", player.getPos().x, player.getPos().y, player.getPos().z);
			}
			if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
				move = -1.f*glm::vec3(forward.x, forward.y, forward.z);
				printf("\nposition: (%f,%f,%f)\n", player.getPos().x, player.getPos().y, player.getPos().z);
			}
			if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
				move = glm::cross(glm::vec3(forward.x, forward.y, forward.z), glm::vec3(0,1,0));
			}
			if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
				move = glm::cross(-1.f*glm::vec3(forward.x, forward.y, forward.z), glm::vec3(0,1,0));
			}
			/*if (state[SDL_SCANCODE_SPACE]) {
				if(!jumping && !falling){
					jumping = true;
					prev_y = player.pos.y;
				}
			}*/
			/*if (state[SDL_SCANCODE_G] && state[SDL_KEYUP]) {
				Object wall = Object(player.pos + 2.f*fwd, glm::vec3(0.5,0.5,0.5), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, 1, glm::vec3(1,0,0), 'w');
				int x = (int) floorf(wall.pos.x + mapWidth / 2.f);
				int z = (int) floorf(wall.pos.z + mapHeight / 2.f);
				map[x + z * mapWidth] = wall;
				objects.push_back(map[x + z*mapWidth]);
				usleep(20000);
			}*/

			cur_time = SDL_GetTicks()/1000.f;
			time_past = cur_time - prev_time;
			prev_time = cur_time;

			Uint8 mouse;
			int cur_x, cur_y; 
			mouse = SDL_GetMouseState(&cur_x, &cur_y);
			turn_x = (mouse_x - cur_x);
			turn_y = (mouse_y - cur_y);
			//glm::vec3 look = forward - rayCast(mouse_x, mouse_y);
			mouse_x = cur_x;
			mouse_y = cur_y;
			//printf("\nmouse x: %d\n", mouse_x);
			if(mouse_x <= 5){
				turn_x += 0.5;
				forward = (glm::rotate(5.f*turn_x*time_past, glm::vec3(0, 1, 0)) * glm::vec4(forward, 0.f));
				//usleep(10000);
			}
			if(mouse_x >= screenWidth - 5){
				turn_x -= 0.5;
				forward = (glm::rotate(5.f*turn_x*time_past, glm::vec3(0, 1, 0)) * glm::vec4(forward, 0.f));
				//usleep(10000);
			}
			glm::vec3 start_pos;
			if(mouse & SDL_BUTTON(SDL_BUTTON_LEFT)){
				//cur_key = pickUpKey(player.pos);
				//cur_door = unlockDoor(player.pos);
				//pickUp(player.pos);
				if(!selected)
					cur_selection = pickUp(start_pos);
			}
			if(mouse & SDL_BUTTON(SDL_BUTTON_RIGHT));
			else {
				forward = (glm::rotate(turn_x*time_past, glm::vec3(0, 1, 0)) * glm::vec4(forward, 0.f));
				//forward = (glm::rotate(5*sin(turn_x)*time_past, glm::vec3(0, 1, 0)) * glm::vec4(forward, 0.f));
				forward = (glm::rotate(turn_y*time_past, glm::cross(fwd, glm::vec3(0,1,0))) * glm::vec4(forward, 0.f));
				//forward = glm::vec3(sin(turn_x)*time_past, forward.y, (cos(turn_x)) * time_past);
				//keyTurn += 0.01;
			}

			// Clear the screen to default color
			glClearColor(.2f, 0.4f, 0.8f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(texturedShader);

			//printf("\n%f\n", timePast);

			glm::mat4 view = glm::lookAt(
			//glm::vec3(0.f, 8.f, 0.0f),  //Cam Position
			//glm::vec3(0.f,0.f,-0.01f),		//<---birds eye view of map
			player.pos,
			player.pos + forward,  //Look at point
			glm::vec3(0.0f, 1.0f, 0.0f)); //Up
			glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

			glm::mat4 proj = glm::perspective(3.14f/4, screenWidth / (float) screenHeight, 0.1f, 10.0f); //FOV, aspect, near, far
			glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex0);
			glUniform1i(glGetUniformLocation(texturedShader, "tex0"), 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex1);
			glUniform1i(glGetUniformLocation(texturedShader, "tex1"), 1);

			glBindVertexArray(vao);
			//drawGeometry(texturedShader, startVertTeapot, numVertsTeapot, startVertKnot, numVertsKnot);
			//glm::vec3 playerPos = glm::vec3(mapStart.x, mapStart.y, mapStart.z) + forward;
			glm::vec3 playerPos = player.pos;
			glm::vec3 newPos = player.pos + 2.f*move*time_past;
			if(canWalk(newPos.x, newPos.z)){
				playerPos = newPos;
			}

			float playerAng = player.rotAngle + 5.f*turn_x*time_past;
			glm::vec3 color = glm::vec3(0.5,0.5,0.5);

			for(int i = 0; i < objects.size(); i++){
				draw(texturedShader, objects[i], objects[i].tex(), objects[i].color);
				if(objects[i].type >= 'A' && objects[i].type <= 'F'){
					if(doors[objects[i].type - 65]){
						glm::vec3 pos;
						if(objects[i].pos.y > -0.5f)
							pos = glm::vec3(objects[i].pos.x, objects[i].pos.y - 0.01f, objects[i].pos.z);
						//glm::vec3 color = glm::vec3(0.5,0.5,0.5);
						objects[i] = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, -1, objects[i].color, objects[i].type);
						//doors[cur_door] = false;
						//have_keys[cur_door] = false;
						//obj.pos = glm::vec3(0,-10,0);
					}
				}
			}
			glm::vec3 mouse_ray = rayCast(mouse_x, mouse_y, player.pos);
			if(selected && !scaling && !rotating){
				//start_pos = isZero(turn_x + turn_y)*mouse_ray;
				//glm::vec3 intersect = hitIntersect(objects[cur_selection]);
				/*if(raySphereIntersection(objects[cur_selection])){
					printf("Object start position: (%f, %f, %f)\n", start_pos.x, start_pos.y, start_pos.z);
					printf("Object intersect position: (%f, %f, %f)\n", intersect.x, intersect.y, intersect.z);
				}*/
				//printf("Mouse ray position: (%f, %f, %f)\n", mouse_ray.x, mouse_ray.y, mouse_ray.z);
				objects[cur_selection].pos = start_pos + rayCast(mouse_x, mouse_y, player.pos);// + 2.f*glm::vec3(fwd.x, fwd.y, fwd.z);
				//objects[cur_selection].pos = hitIntersect(objects[cur_selection]);
			}
			else if(selected && scaling && !rotating){
				objects[cur_selection].scale += 0.01*(turn_x + turn_y)/2.0;
			}
			else if(selected && !scaling && rotating){
				if(turn_x){
					objects[cur_selection].rotAxis = glm::vec3(0,1,0);
					objects[cur_selection].rotAngle += 0.1*turn_x;
				}
				else if(turn_y){
					objects[cur_selection].rotAxis = glm::vec3(0,0,1);
					objects[cur_selection].rotAngle += 0.1*turn_y;
				}
				else if(turn_x && turn_y){
					objects[cur_selection].rotAxis = glm::vec3(1,1,1);
					objects[cur_selection].rotAngle += 0.1*turn_y;
				}
			}
			else if(selected && scaling && rotating){
				glm::vec3 mouse_ray = rayCast(mouse_x, mouse_y, start_pos);
				start_pos += 0.25f * turn_y * glm::vec3(mouse_ray.x, 0, mouse_ray.z);
				objects[cur_selection].pos = start_pos; 
			}



			/// JUMPING ///

			/*for(int i = 0; i < sizeof(*map) / sizeof(map[0]); i++){
				draw(texturedShader, map[i], map[i].tex(), map[i].color);
			}*/
			if(jumping && playerPos.y - prev_y <= 0.5){
				playerPos.y += 0.025;
			}
			if(jumping && playerPos.y - prev_y >= 0.5){ 
				jumping = false;
				falling = true;
			}
			if(falling && playerPos.y - prev_y >= 0 ){
				playerPos.y -= 0.025;
			}
			if(falling && playerPos.y - prev_y <= 0 ){
				falling = false;
			}
			player = Object(playerPos, glm::vec3(0.2,0.2,0.2), glm::vec3(0,1,0), playerAng, models[0].start, models[0].numVerts, 0, color, 'p');
			//draw(texturedShader, player, 0, player.color);

			goalReached();
			SDL_GL_SwapWindow(window); //Double buffering
		}
		else{
			//string victory = "Victory!";
			//objects.clear();
			// Clear the screen to default color
			glClearColor(.2f, 0.4f, 0.8f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(texturedShader);

			//printf("\n%f\n", timePast);

			glm::mat4 view = glm::lookAt(
			glm::vec3(0.f, 30.f, 0.0f),  //Cam Position
			glm::vec3(0.f,0.f,-0.01f),		//<---birds eye view of map
			glm::vec3(0.0f, 1.0f, 0.0f)); //Up
			glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

			glm::mat4 proj = glm::perspective(3.14f/4, screenWidth / (float) screenHeight, 0.07f, 40.0f); //FOV, aspect, near, far
			glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex0);
			glUniform1i(glGetUniformLocation(texturedShader, "tex0"), 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex1);
			glUniform1i(glGetUniformLocation(texturedShader, "tex1"), 1);

			glBindVertexArray(vao);

			for(int i = 0; i < victoryMap.size(); i++){
				draw(texturedShader, victoryMap[i], victoryMap[i].tex(), victoryMap[i].color);
			}

			SDL_GL_SwapWindow(window);
			/*printf("\n _____ _    _  _____ _____ ______  _____ _____ _ \n"
					"/ ____| |  | |/ ____/ ____|  ____|/ ____/ ____| |\n"
					"| (___ | |  | | |   | |    | |__  | (___| (___ | |\n"
					"\\___ \\| |  | | |   | |    |  __|  \\___ \\___ \\| |\n"
					" ____) | |__| | |___| |____| |____ ____) |___) |_|\n"
					"|_____/ \\____/ \\_____\\_____|______|_____/_____/(_)\n");*/
			
			//break;
		}
	}
	
	//Clean Up
	glDeleteProgram(texturedShader);
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}

void draw(int shaderProgram, Object obj, int texNum, glm::vec3 color){
	
	if(obj.type >= 'a' && obj.type <= 'f'){
		if(have_keys[obj.type - 97]){
			//printf("%c", obj.type);
			//printf("\njoe mama\n");
			//float angle = atan(fwd.x / fwd.z);
			obj = Object(player.pos + fwd + glm::vec3(0,0.35,0), glm::vec3(0.1,0.1,0.1), obj.rotAxis, 0, obj.index, obj.numVerts, obj.texNum, obj.color, obj.type);
			//obj.pos = player.pos + fwd;
		}
	}
	
	//printf("\ncarry: %d\n", carry);
	GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");
	//glm::vec3 colVec(colR,colG,colB);
	glUniform3fv(uniColor, 1, glm::value_ptr(color));
      
  	GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");
	
	glm::mat4 model = glm::mat4(1);

	GLint uniModel = glGetUniformLocation(shaderProgram, "model");

	model = glm::translate(model, obj.getPos());
	model = glm::scale(model, obj.getScale());
	model = glm::rotate(model, obj.rotAngle, obj.rotAxis);

	//model = glm::scale(model,2.f*glm::vec3(1.f,1.f,0.5f)); //scale example
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
	//Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)
	glUniform1i(uniTexID, texNum);

  //Draw an instance of the model (at the position & orientation specified by the model matrix above)
	glDrawArrays(GL_TRIANGLES, obj.getIndex(), obj.getNumVerts()); //(Primitive Type, Start Vertex, Num Verticies)

}



// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile){
	FILE *fp;
	long length;
	char *buffer;

	// open the file containing the text of the shader code
	fp = fopen(shaderFile, "r");

	// check for errors in opening the file
	if (fp == NULL) {
		printf("can't open shader source file %s\n", shaderFile);
		return NULL;
	}

	// determine the file size
	fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
	length = ftell(fp);  // return the value of the current position

	// allocate a buffer with the indicated number of bytes, plus one
	buffer = new char[length + 1];

	// read the appropriate number of bytes from the file
	fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
	fread(buffer, 1, length, fp); // read all of the bytes

	// append a NULL character to indicate the end of the string
	buffer[length] = '\0';

	// close the file
	fclose(fp);

	// return the string
	return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName){
	GLuint vertex_shader, fragment_shader;
	GLchar *vs_text, *fs_text;
	GLuint program;

	// check GLSL version
	printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Create shader handlers
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read source code from shader files
	vs_text = readShaderSource(vShaderFileName);
	fs_text = readShaderSource(fShaderFileName);

	// error check
	if (vs_text == NULL) {
		printf("Failed to read from vertex shader file %s\n", vShaderFileName);
		exit(1);
	} else if (DEBUG_ON) {
		printf("Vertex Shader:\n=====================\n");
		printf("%s\n", vs_text);
		printf("=====================\n\n");
	}
	if (fs_text == NULL) {
		printf("Failed to read from fragent shader file %s\n", fShaderFileName);
		exit(1);
	} else if (DEBUG_ON) {
		printf("\nFragment Shader:\n=====================\n");
		printf("%s\n", fs_text);
		printf("=====================\n\n");
	}

	// Load Vertex Shader
	const char *vv = vs_text;
	glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
	glCompileShader(vertex_shader); // Compile shaders
	
	// Check for errors
	GLint  compiled;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		printf("Vertex shader failed to compile:\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}
	
	// Load Fragment Shader
	const char *ff = fs_text;
	glShaderSource(fragment_shader, 1, &ff, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	
	//Check for Errors
	if (!compiled) {
		printf("Fragment shader failed to compile\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}

	// Create the program
	program = glCreateProgram();

	// Attach shaders to program
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	// Link and set program to use
	glLinkProgram(program);

	return program;
}

void loadMap(string fileName){
	ifstream in;
	in.open(fileName);
	in >> mapWidth >> mapHeight;
	map = new Object[mapHeight * mapWidth];
	char env;
	for(int i = 0; i < mapHeight; i++){
		for(int j = 0; j < mapWidth; j++){
			int index = i * mapWidth + j;
			in >> env;
			if(env == 'W'){		//wall
				//int texturedShader = InitShader("textured-Vertex.glsl", "textured-Fragment.glsl");
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, 0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(0.5,0.5,0.5);
				Object wall = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, 1, color, 'w');
				map[index] = wall;
				objects.push_back(map[index]);
				//blocks.push_back(wall);
				//draw(texturedShader, wall, 1);
			}
			else if(env == '0'){	//floor
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, -0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(0.5,0.5,0.5);
				Object floor = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, 0, color, 'z');
				objects.push_back(floor);
				map[index] = floor;
			}
			else if(env == 'G'){	//goal
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, -0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(1,1,0);
				Object goal = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, -1, color, 'g');
				objects.push_back(goal);
				map[index] = goal;
			}
			else if(env == 'S'){	//start
				//make start
				mapStart = glm::vec3(j - mapWidth / 2.0f + 0.5f, 0.49f, i + 0.5f - mapHeight/2.0f);
				player.pos = mapStart;
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, -0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(1,0.2,0.2);
				Object start = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, -1, color, 's');
				objects.push_back(start);
				map[index] = start;
				printf("\nStart Position: (%f,%f,%f)\n", pos.x, pos.y, pos.z);
			}
			else if(env >= 'A' && env <= 'F'){	//doors
				//make door
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, 0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color;
				if(env == 'A') color = glm::vec3(1,.3,0);
				else if(env == 'B') color = glm::vec3(0,0,1);
				else if(env == 'C') color = glm::vec3(0,1,1);
				else if(env == 'D') color = glm::vec3(1,1,0);
				else if(env == 'E') color = glm::vec3(0.5,0,1);
				else if(env == 'F') color = glm::vec3(0,1,0);
				Object door = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, -1, color, env);
				objects.push_back(door);
				//blocks.push_back(door);
				map[index] = door;
			}
			else if(env >= 'a' && env <= 'f'){	//keys
				//make key
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, 0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color;
				if(env == 'a') color = glm::vec3(1,.3,0);
				else if(env == 'b') color = glm::vec3(0,0,1);
				else if(env == 'c') color = glm::vec3(0,1,1);
				else if(env == 'd') color = glm::vec3(1,1,0);
				else if(env == 'e') color = glm::vec3(0.5,0,1);
				else if(env == 'f') color = glm::vec3(0,1,0);
				Object key = Object(pos, glm::vec3(.4,.4,.4), glm::vec3(0,1,0), 0, models[0].start, models[0].numVerts, -1, color, env);
				objects.push_back(key);
				map[index] = key;
				pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, -0.5f, i + 0.5f - mapHeight/2.0f);
				Object floor = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, 0, color, 'z');
				objects.push_back(floor);
			}
		}
	}

	/*for(int i = 0; i < mapWidth*mapHeight; i++){
		if(i % mapWidth == 0){
			printf("%c\n", map[i].type);
		}
		else{
			printf("%c ", map[i].type);
		}
	}*/

	///BORDER WALLS FOR LATER///
	/*for(int i = 0; i < mapHeight+2; i++){
		glm::vec3 pos1 = glm::vec3(mapWidth / 2.0f + 1.f, 0.5f, i + 0.5f - mapHeight/2.0f);
		glm::vec3 pos2 = glm::vec3(-mapWidth / 2.0f - 1.f, 0.5f, i + 0.5f - mapHeight/2.0f);
		glm::vec3 color = glm::vec3(0.5,0.5,0.5);
		Object wall = Object(pos1, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, 1, color, 'w');
		objects.push_back(wall);
		wall = Object(pos2, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, 1, color, 'w');
		objects.push_back(wall);
	}
	for(int i = 0; i < mapWidth+2; i++){
		glm::vec3 pos1 = glm::vec3(i - mapWidth / 2.0f + 0.5f, 0.5f, -1.f - mapHeight/2.0f);
		glm::vec3 pos2 = glm::vec3(i - mapWidth / 2.0f + 0.5f, 0.5f, 1.f + mapHeight/2.0f);
		glm::vec3 color = glm::vec3(0.5,0.5,0.5);
		Object wall = Object(pos1, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, 1, color, 'w');
		objects.push_back(wall);
		wall = Object(pos2, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, 1, color, 'w');
		objects.push_back(wall);
	}*/
}

void loadVictoryMap(){
	ifstream in;
	in.open("maps/victory.txt");
	in >> mapWidth >> mapHeight;
	//map = new Object[mapHeight * mapWidth];
	char env;
	for(int i = 0; i < mapHeight; i++){
		for(int j = 0; j < mapWidth; j++){
			//int index = i * mapWidth + j;
			in >> env;
			if(env == 'W'){		//wall
				//int texturedShader = InitShader("textured-Vertex.glsl", "textured-Fragment.glsl");
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, 0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(0.5,0.5,0.5);
				Object wall = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), 0, models[1].start, models[1].numVerts, 1, color, 'w');
				victoryMap.push_back(wall);
				//blocks.push_back(wall);
				//map[index] = wall;
				//draw(texturedShader, wall, 1);
			}
		}
	}
}

bool canWalk(float x, float z){
	//float playerRadius = 0.002;
	//if(x > mapWidth/2.f + 0.5f || z > mapHeight/2.f + 0.5f || x < -mapWidth/2.f - 0.5f || z < -mapHeight/2.f - 0.5f) return false;
	if(abs(x - mapWidth/2.f) < 0.1f || abs(z - mapHeight/2.f) < 0.1f || abs(x + mapWidth/2.f) < 0.1f || abs(z + mapHeight/2.f) < 0.1f) return false;

	for(int i = 0; i < objects.size(); i++){
		if(objects[i].type == 'w' || (objects[i].type >= 'A' && objects[i].type <= 'F')){
			if(abs(x - objects[i].pos.x) < (0.6f * objects[i].scale.x) && abs(z - objects[i].pos.z) < (0.6f * objects[i].scale.z) && (objects[i].pos.y - player.pos.y) >= (-1.f * objects[i].scale.y) && (objects[i].pos.y - player.pos.y) <= (0.6f * objects[i].scale.y)) return false;
		}
	}
	return true;
}

int pickUpKey(glm::vec3 pos){
	int x = (int) floorf(pos.x + mapWidth / 2.f);
	int z = (int) floorf(pos.z + mapHeight / 2.f);
	char keyChar = map[x + z * mapWidth].type;
	int num = 0;
	if(keyChar >= 'a' && keyChar <= 'f'){
		//printf("\nin here\n");
		//if(!carry){
			//printf("\nno here\n");
		if(keyChar == 'a'){ 
			//printf("\nayyyyy\n");
			have_keys[0] = true;
			num = 0;
		}
		else if(keyChar == 'b'){
			//printf("\nbeeee\n");
			have_keys[1] = true;
			num = 1;
		}
		else if(keyChar == 'c'){
			//printf("\ncccccccc\n");
			have_keys[2] = true;
			num = 2;
		}
		else if(keyChar == 'd'){
			have_keys[3] = true;
			num = 3;
		}
		else if(keyChar == 'e'){
			have_keys[4] = true;
			num = 4;
		}
		else if(keyChar == 'f'){
			have_keys[5] = true;
			num = 5;
		}
		//carry = true;
		//}
	}
	//map[x + z * mapWidth].pos = player.pos;
	return num;
}

int unlockDoor(glm::vec3 pos){
	int x = (int) floorf(pos.x + mapWidth / 2.f + fwd.x);
	int z = (int) floorf(pos.z + mapHeight / 2.f + fwd.z);
	char doorChar = map[x + z * mapWidth].type;
	int num = 0;
	if(doorChar >= 'A' && doorChar <= 'F'){
		if(doorChar == 'A' && have_keys[0]){ 
			doors[0] = true;
			num = 0;
		}
		else if(doorChar == 'B' && have_keys[1]){
			doors[1] = true;
			num = 1;
		}
		else if(doorChar == 'C' && have_keys[2]){
			doors[2] = true;
			num = 2;
		}
		else if(doorChar == 'D' && have_keys[3]){
			doors[3] = true;
			num = 3;
		}
		else if(doorChar == 'E' && have_keys[4]){
			doors[4] = true;
			num = 4;
		}
		else if(doorChar == 'F' && have_keys[5]){
			doors[5] = true;
			num = 5;
		}
	}
	return num;
}

void goalReached(){
	int x = (int) floorf(player.pos.x + mapWidth / 2.f);
	int z = (int) floorf(player.pos.z + mapHeight / 2.f);
	char type = map[x + z * mapWidth].type;
	if(type == 'g'){
		goal = true;
	}
}

/*void pickUp(glm::vec3 pos){
	int x = (int) floorf(pos.x + mapWidth / 2.f);
	int z = (int) floorf(pos.z + mapHeight / 2.f);
	char keyChar = map[x + z * mapWidth].type;
	if(keyChar != 'z'){
		map[x + z * mapWidth].pos = player.pos + 2.f*fwd;
	}
	//map[x + z * mapWidth].pos = player.pos;
	//return num;
}*/

glm::vec3 rayCast(int mouse_x, int mouse_y, glm::vec3 pos){
	//Normalized device coords
	float x = (2.0f * mouse_x) / screenWidth - 1.0f;
	float y = 1.0f - (2.0f * mouse_y) / screenHeight;
	float z = 1.0f;
	glm::vec3 ray = glm::vec3(x,y,z);

	//Clip coords
	glm::vec4 ray_clip = glm::vec4(ray.x, ray.y, -1.0, 1.0); //may cause errors

	//Eye coords
	glm::mat4 proj = glm::perspective(3.14f/4, screenWidth / (float) screenHeight, 0.1f, 10.0f);
	glm::vec4 ray_eye = inverse(proj) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
	
	//view matrix
	glm::mat4 view = glm::lookAt(
	pos,
	pos + fwd,  //Look at point
	glm::vec3(0.0f, 1.0f, 0.0f)); //Up

	//World coords
	glm::vec4 ray_w = (inverse(view) * ray_eye);
	glm::vec3 ray_world = glm::vec3(ray_w.x, ray_w.y, ray_w.z);
	ray_world = glm::normalize(ray_world);

	return ray_world;
}

/*
bool rayPlaneIntersection(Object obj){
	float denom = glm::dot(-1.f*fwd, rayCast(mouse_x, mouse_y));
	glm::vec3 sub = obj.pos - player.pos;
	float dist = glm::dot(sub, rayCast(mouse_x, mouse_y));
	return (dist >= 0);
}*/

bool raySphereIntersection(Object obj){
	float radius = 0.5;
	glm::vec3 ray = rayCast(mouse_x, mouse_y, player.pos);
	float a = glm::dot(ray,ray);
	glm::vec3 toStart = player.pos - obj.pos;
	float b = 2 * glm::dot(ray, toStart);
	float c = glm::dot(toStart, toStart) - radius*radius;
	float discr = b*b - 4*a*c;
	if(discr < 0) return false;
	else{
		float t0 = (-b + sqrt(discr))/(2*a);
   		float t1 = (-b - sqrt(discr))/(2*a);
    	if (t0 > 0 || t1 > 0) return true;
  	}
  	return false;
}

float hitT(Object obj){
	float radius = 0.5;
	glm::vec3 ray = rayCast(mouse_x, mouse_y, player.pos);
	float a = glm::dot(ray,ray);
	glm::vec3 toStart = player.pos - obj.pos;
	float b = 2 * glm::dot(ray, toStart);
	float c = glm::dot(toStart, toStart) - radius*radius;
	float discr = b*b - 4*a*c;
	if(discr < 0) return -1;
	else{
		float t0 = (-b + sqrt(discr))/(2*a);
   		float t1 = (-b - sqrt(discr))/(2*a);
    	if(t0 > 0 && t1 > 0) return std::min(t0, t1);
    	else if (t0 > 0) return t0;
    	else if(t1 > 0) return t1;
    	else return -1;
  	}
}

float distToSqr(glm::vec3 vec){
	return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
}

glm::vec3 hitIntersect(Object obj){ //use only if hit = true
	float radius = 0.5;
	glm::vec3 rayLine = rayCast(mouse_x, mouse_y, player.pos);
	glm::vec3 projPoint = glm::dot(rayLine,obj.pos)*rayLine;      //Project to find closest point between circle center and line [proj(sphereCenter,rayLine);]
	float distSqr = distToSqr(projPoint - obj.pos);          //Point-line distance (squared)
	float d2 = distSqr/(radius*radius);             //If distance is larger than radius, then...
	//if (d2 > 1) return glm::vec3();                                   //... the ray missed the sphere
	float w = radius*sqrt(1-d2);                          //Pythagorean theorem to determine dist between proj point and intersection points
	glm::vec3 p1 = projPoint - rayLine*w;                   //Add/subtract above distance to find hit points
	glm::vec3 p2 = projPoint + rayLine*w; 

	//float t = hitT(obj);
	float dist1 = abs(glm::dot((p1-player.pos),rayLine));
	float dist2 = abs(glm::dot((p2-player.pos),rayLine));

	if(dot((p1-player.pos),rayLine) >= 0 && dot((p2-player.pos),rayLine) >= 0){
		if(dist1 < dist2)
			return p1;
		else
			return p2;
	}
	else if(dot((p1-player.pos),rayLine) >= 0){
		return p1;
	}
	else if(dot((p2-player.pos),rayLine) >= 0){
		return p2;
	}
	else{
		return glm::vec3();
	}
}

/*void pickUp(){
	for(int i = 0; i < objects.size(); i++){
		if(objects[i].type != 'z' && player.pos.x - objects[i].pos.x < 1 && player.pos.y - objects[i].pos.y < 1 && player.pos.z - objects[i].pos.z < 1){
			objects[i].pos = player.pos + fwd;
			printf("\nObject type: %c\n", objects[i].type);
		}
	}
}*/

int pickUp(glm::vec3 &start_pos){
	float min_dist = INFINITY;
	for(int i = 0; i < objects.size(); i++){
		float dist = (objects[i].pos - player.pos).length();
		if(dist < min_dist){
			min_dist = dist;
		}
		if(dist == min_dist){
			if(raySphereIntersection(objects[i]) && objects[i].type != 'z'){
				//objects[i].pos = player.pos + rayCast(mouse_x, mouse_y);
				objects[i].isSelected = true;
				selected = true;
				start_pos = objects[i].pos;
				//printf("\nObject type: %c\n", objects[i].type);
				return i;
			}
		}
	}
	return -1;
}