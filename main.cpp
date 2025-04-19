//Scene Builder
//Michael Karb, Kevin Bradt 2021

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
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "imgui/backends/imgui_impl_opengl3.h"

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

class Selected{
public:
	Object* object_ptr;
	glm::vec3 color;
	float scaleX;
	float scaleY;
	float scaleZ;
	float yaw;
	float pitch;
	float roll;
	int texNum;
	int modelNum;

	Selected(){
		object_ptr = nullptr;
		color = glm::vec3();
		scaleX = 1.0;
		scaleY = 1.0;
		scaleZ = 1.0;
		yaw = 0.0;
		pitch = 0.0;
		roll = 0.0;
		texNum = -1;
		modelNum = 0;
	}
};

Selected selectedObject;

class PointLight {
public:
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
	int id;

	PointLight(glm::vec3 pos, glm::vec3 col, float tensity, int _id) {
		position = pos;
		color = col;
		intensity = tensity;
		id = _id;
	}
};

std::vector<PointLight> lights;
int numLights = 0;
std::vector<const char*> lightIDs;

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

ImVec4 newLight_color = ImVec4(1.0f,1.0f,1.0f,1.0f);
float newLightPos[3] = {1.0f,1.0f,1.0f};
float newLightIntensity = 1.0f;
int selectedLight = 0;

ImVec4 back_color = ImVec4(0.2f, 0.4f, 0.8f, 1.0f);
float ambient = 0.3;
float spec = 1;
float diff = 1;
float cam_yaw = -90.f;
float cam_pitch = 0.f;

bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;
void Win2PPM(int width, int height);

//srand(time(NULL));
float rand01(){
	return rand()/(float)RAND_MAX;
}

int texturedShader;

void draw(int shaderProgram, Object obj, int texNum, glm::vec3 color);
void drawGUI(ImVec4 clear_color);
void loadMap(string fileName);
void loadVictoryMap();
bool canWalk(float x, float y);
int pickUp();
glm::vec3 rayCast(int mouse_x, int mouse_y, glm::vec3 pos);
bool rayPlaneIntersection(Object obj);
bool raySphereIntersection(Object obj);
float distToSqr(glm::vec3 vec);
glm::vec3 hitIntersect(Object obj);
float hitT(Object obj);
void Lights(int shaderProgram, std::vector<PointLight> light);
void addLight(glm::vec3 pos, glm::vec3 color, float tensity);

int main(int argc, char *argv[]){
	printf("\nLine 184\n");
	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	SDL_Window* window = SDL_CreateWindow("Scene Builder", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);
	SDL_SetRelativeMouseMode(SDL_TRUE);
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
	
	// IMGUI setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 150");

	bool show_demo_window = true;
	//bool show_another_window = false;
	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	// End IMGUI setup

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
	//float prev_y;
	//bool jumping = false;
	//bool falling = false;
	Object obj;
	//int model_num = 0;
	//int tex_num = 0;
	while (!quit){
		move = glm::vec3(0,0,0);
		turn_x = 0;
		turn_y = 0;
		fwd = forward;

		if(cur_selection >= 0){
			objects[cur_selection].modelNum = selectedObject.modelNum;
			objects[cur_selection].texNum = selectedObject.texNum;
			objects[cur_selection].color = selectedObject.color;
			objects[cur_selection].scale = glm::vec3(selectedObject.scaleX, selectedObject.scaleY, selectedObject.scaleZ);
			objects[cur_selection].rotAngle = glm::vec3(selectedObject.roll, selectedObject.pitch, selectedObject.yaw);
		}

		while (SDL_PollEvent(&windowEvent)){  //inspect all events in the queue
			if (windowEvent.type == SDL_QUIT) quit = true;
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE) 
				quit = true; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f){ //If "f" is pressed
				fullscreen = !fullscreen;
				SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen 
			}

			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_SPACE){ //If "space" is pressed
				obj = Object(player.pos + rayCast(mouse_x, mouse_y, player.pos), glm::vec3(0.5,0.5,0.5), glm::vec3(1,1,1), glm::vec3(0,0,0), selectedObject.modelNum, selectedObject.texNum, glm::vec3(1,0,0), 'w');
				objects.push_back(obj);
				//selected = true;
				//cur_selection = objects.size() - 1;
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_c){ //If "c" is pressed
				selected = false;
				for(int i = 0; i < objects.size(); i++){
					objects[i].isSelected = false;
				}
				cur_selection = -1;
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_m) {
				show_demo_window = !show_demo_window;
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
				move = forward;
				//printf("\nposition: (%f,%f,%f)\n", player.getPos().x, player.getPos().y, player.getPos().z);
			}
			if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
				move = -1.f*forward;
				//printf("\nposition: (%f,%f,%f)\n", player.getPos().x, player.getPos().y, player.getPos().z);
			}
			if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
				move = glm::cross(forward, glm::vec3(0,1,0));
			}
			if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
				move = glm::cross(-1.f*forward, glm::vec3(0,1,0));
			}
			/*if (state[SDL_SCANCODE_SPACE]) {
				if(!jumping && !falling){
					jumping = true;
					prev_y = player.pos.y;
				}
			}*/
			cur_time = SDL_GetTicks()/1000.f;
			time_past = cur_time - prev_time;
			prev_time = cur_time;
			glm::vec3 start_pos;
			if(!show_demo_window){
				Uint8 mouse;
				int cur_x, cur_y; 
				mouse = SDL_GetMouseState(&cur_x, &cur_y);
				turn_x = (mouse_x - cur_x);
				turn_y = (mouse_y - cur_y);
				//glm::vec3 look = forward - rayCast(mouse_x, mouse_y);
				mouse_x = cur_x;
				mouse_y = cur_y;
				//printf("\nmouse x: %d\n", mouse_x);
				/*if(mouse_x <= 5){
					turn_x += 0.5;
					forward = (glm::rotate(5.f*turn_x*time_past, glm::vec3(0, 1, 0)) * glm::vec4(forward, 0.f));
					//usleep(10000);
				}
				if(mouse_x >= screenWidth - 5){
					turn_x -= 0.5;
					forward = (glm::rotate(5.f*turn_x*time_past, glm::vec3(0, 1, 0)) * glm::vec4(forward, 0.f));
					//usleep(10000);
				}*/
				if(mouse & SDL_BUTTON(SDL_BUTTON_LEFT)){
					//pickUp(player.pos);
					if(!selected){
						cur_selection = pickUp();
						start_pos = objects[cur_selection].pos;
						if(cur_selection >= 0){
							selectedObject.color = objects[cur_selection].color;
							selectedObject.modelNum = objects[cur_selection].modelNum;
							selectedObject.texNum = objects[cur_selection].texNum;
							selectedObject.roll = objects[cur_selection].rotAngle.x;
							selectedObject.pitch = objects[cur_selection].rotAngle.y;
							selectedObject.yaw = objects[cur_selection].rotAngle.z;
							selectedObject.scaleX = objects[cur_selection].scale.x;
							selectedObject.scaleY = objects[cur_selection].scale.y;
							selectedObject.scaleZ = objects[cur_selection].scale.z;
						}
					}
				}
				if(mouse & SDL_BUTTON(SDL_BUTTON_RIGHT));
				else {
					cam_yaw -= turn_x;
					cam_pitch += turn_y;
					if(cam_pitch > 89.f)
						cam_pitch = 89.f;
					if(cam_pitch < -89.f)
						cam_pitch = -89.f;
					//forward = rayCast(mouse_x, mouse_y, player.pos);
					//forward = (glm::rotate(turn_x*time_past, glm::vec3(0, 1, 0)) * glm::vec4(forward, 0.f));
					//forward = (glm::rotate(turn_y*time_past, glm::cross(fwd, glm::vec3(0,1,0))) * glm::vec4(forward, 0.f));
					forward.x = cos(glm::radians(cam_yaw)) * cos(glm::radians(cam_pitch));
					forward.y = sin(glm::radians(cam_pitch));
					forward.z = sin(glm::radians(cam_yaw)) * cos(glm::radians(cam_pitch));
					forward = glm::normalize(forward);
				}
			}

			// Clear the screen to default color
			glClearColor(back_color.x, back_color.y, back_color.z, 1.0f);
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
			//glm::vec3 playerPos = glm::vec3(mapStart.x, mapStart.y, mapStart.z) + forward;
			glm::vec3 playerPos = player.pos;
			glm::vec3 newPos = player.pos + 2.f*move*time_past;
			if(canWalk(newPos.x, newPos.z)){
				playerPos = newPos;
			}

			glm::vec3 playerAng = player.rotAngle + 5.f*turn_x*time_past;
			glm::vec3 color = glm::vec3(0.5,0.5,0.5);

			for(int i = 0; i < objects.size(); i++){
				draw(texturedShader, objects[i], objects[i].tex(), objects[i].color);
			}
			Lights(texturedShader, lights);

			glm::vec3 mouse_ray = rayCast(mouse_x, mouse_y, player.pos);
			Object laser = Object(playerPos + rayCast(mouse_x, mouse_y, player.pos), glm::vec3(0.01), glm::vec3(0,0,0), glm::vec3(0,0,0), 3, -1, glm::vec3(1,0,0), 'z');
			if(selected && !scaling && !rotating){
				objects[cur_selection].pos = start_pos + float((player.pos + mouse_ray - start_pos).length())*mouse_ray;
				//forward = glm::normalize(forward);

				start_pos += 2.f*move*time_past;
			}
			else if(selected && scaling && !rotating){
				objects[cur_selection].scale += 0.01*(turn_x + turn_y)/2.0;
			}
			else if(selected && !scaling && rotating){
				if(turn_x){
					//objects[cur_selection].rotAxis = glm::vec3(0,1,0);
					objects[cur_selection].rotAngle.x += 0.1*turn_x;
				}
				if(turn_y){
					//objects[cur_selection].rotAxis = glm::vec3(0,0,1);
					objects[cur_selection].rotAngle.y += 0.1*turn_y;
				}
			}
			else if(selected && scaling && rotating){
				//glm::vec3 mouse_ray = rayCast(mouse_x, mouse_y, start_pos);
				start_pos += 2.f * turn_y * time_past * glm::vec3(mouse_ray.x, 0, mouse_ray.z);
				objects[cur_selection].pos = start_pos; 
			}
			draw(texturedShader, laser, laser.texNum, laser.color);
			/// JUMPING ///
			/*
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
			}*/
			player = Object(playerPos, glm::vec3(0.2,0.2,0.2), glm::vec3(0,1,0), playerAng, 0, 0, color, 'p');
			//draw(texturedShader, player, 0, player.color);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame(window);
			ImGui::NewFrame();
			if (show_demo_window) {
				drawGUI(ImVec4(selectedObject.color.r, selectedObject.color.g, selectedObject.color.b, 1.00f));
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}
			else{
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			SDL_GL_SwapWindow(window); //Double buffering
		}
		else{
			// Clear the screen to default color
			glClearColor(.2f, 0.4f, 0.8f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(texturedShader);

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

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}

void Lights(int shaderProgram, std::vector<PointLight> lights) {
	GLint positions = glGetUniformLocation(shaderProgram, "lightsPos");
	GLint scales = glGetUniformLocation(shaderProgram, "lightsScale");
	GLint colors = glGetUniformLocation(shaderProgram, "lightsColor");

	glm::vec3 pos[11];
	glm::vec3 col[11];
	float scl[11];
	for (int i = 0; i < 11; i++) {
		if (i >= numLights) {
			pos[i] = glm::vec3(0.0);
			col[i] = glm::vec3(0.0);
			scl[i] = 0.0f;
		} else {
			pos[i] = lights[i].position;
			col[i] = lights[i].color;
			scl[i] = lights[i].intensity;
		}
	}

	/*glUniform3fv(positions, 11, glm::value_ptr(pos));
	glUniform3fv(colors, 11, glm::value_ptr(col));
	glUniform1fv(scales, 11, glm::value_ptr(scl));*/
	glUniform3fv(positions,11, (const GLfloat*)pos);
	glUniform3fv(colors,11, (const GLfloat*)col);
	glUniform1fv(scales,11, scl);
}

void addLight(glm::vec3 pos, glm::vec3 color, float tensity) {
	lights.push_back(PointLight(pos,color,tensity,numLights));
	lightIDs.push_back((std::to_string(numLights)).c_str());
	numLights++;
}

void draw(int shaderProgram, Object obj, int texNum, glm::vec3 color){
	GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");
	glUniform3fv(uniColor, 1, glm::value_ptr(color));
      
  	GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");
	
	glm::mat4 model = glm::mat4(1);

	GLint uniModel = glGetUniformLocation(shaderProgram, "model");

	model = glm::translate(model, obj.getPos());
	model = glm::scale(model, obj.getScale());
	//model = glm::rotate(model, obj.rotAngle.x, obj.rotAxis);
	model = glm::rotate(model, obj.rotAngle.x, glm::vec3(1,0,0));
	model = glm::rotate(model, obj.rotAngle.y, glm::vec3(0,1,0));
	model = glm::rotate(model, obj.rotAngle.z, glm::vec3(0,0,1));

	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
	//Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)
	glUniform1i(uniTexID, texNum);
	GLint ambience = glGetUniformLocation(shaderProgram, "ambient");
	glUniform1f(ambience, ambient);
	GLint specularity = glGetUniformLocation(shaderProgram, "specularity");
	glUniform1f(specularity, spec);
	GLint diffuse = glGetUniformLocation(shaderProgram, "diffuse");
	glUniform1f(diffuse, diff);

  //Draw an instance of the model (at the position & orientation specified by the model matrix above)
	glDrawArrays(GL_TRIANGLES, models[obj.modelNum].start, models[obj.modelNum].numVerts); //(Primitive Type, Start Vertex, Num Verticies)

}

void drawGUI(ImVec4 clear_color){
	//ImGui::ShowDemoWindow(&show_demo_window);
	bool start = true;
	ImGui::Begin("Selection Menu", &start);

	ImGui::Text("Scale & Rotate");
	ImGui::SliderFloat("Scale X", &(selectedObject.scaleX), 0.0f, 10.0f);
	ImGui::SliderFloat("Scale Y", &(selectedObject.scaleY), 0.0f, 10.0f);
	ImGui::SliderFloat("Scale Z", &(selectedObject.scaleZ), 0.0f, 10.0f);
	ImGui::SliderFloat("Roll", &(selectedObject.roll), -3.14f, 3.14f);
	ImGui::SliderFloat("Pitch", &(selectedObject.pitch), -3.14f, 3.14f);
	ImGui::SliderFloat("Yaw", &(selectedObject.yaw), -3.14f, 3.14f);

	ImGui::Text("Global Lighting");
	ImGui::SliderFloat("Ambient", &(ambient), 0.0f, 2.0f);
	ImGui::SliderFloat("Diffuse", &(diff), 0.0f, 5.0f);
	ImGui::SliderFloat("Specularity", &(spec), 0.0f, 5.0f);
	ImGui::ColorEdit3("Background Color", (float*)&back_color);
	//back_color = glm::vec3(backgr_color.x, backgr_color.y, backgr_color.z);

	ImGui::Text("Selected Object Color & Texture");
	ImGui::ColorEdit3("Object Color", (float*)&clear_color);
	selectedObject.color = glm::vec3(clear_color.x, clear_color.y, clear_color.z);

	const char* textureList[] = {"None", "Wood", "Brick"};
	if (ImGui::Button("Texture"))
		ImGui::OpenPopup("texture_list");
	ImGui::SameLine();
	ImGui::TextUnformatted(textureList[selectedObject.texNum+1]);
	if (ImGui::BeginPopup("texture_list")) {
		for (int i = 0; i < IM_ARRAYSIZE(textureList); i++) {
			if (ImGui::Selectable(textureList[i])) {
				selectedObject.texNum = i - 1;
				printf("Selected tex: %s texNum: %i\n", textureList[i], selectedObject.texNum);
			}
		}
		ImGui::EndPopup();
	}

	ImGui::Text("Model Selection");
	const char* modelList[] = {"Teapot", "Cube", "Knot", "Sphere"};
	if (ImGui::Button("Model"))
		ImGui::OpenPopup("model_list");
	ImGui::SameLine();
	ImGui::TextUnformatted(modelList[selectedObject.modelNum]);
	if (ImGui::BeginPopup("model_list")) {
		for (int i = 0; i < IM_ARRAYSIZE(modelList); i++) {
			if (ImGui::Selectable(modelList[i])) {
				selectedObject.modelNum = i;
			}
		}
		ImGui::EndPopup();
	}

	ImGui::Text("Lights");
	/*bool lightsEsist = false;
	if (numLights > 0) {
		const char* lightList[lightIDs.size()] = 
	}*/
	ImGui::SliderFloat3("New Light Pos", newLightPos,-10.0f,10.0f);
	/*ImGui::SliderFloat("New Light X", &newLightPos[0], -10.0f,10.0f);
	ImGui::SameLine();
	ImGui::SliderFloat("New Light Y", &newLightPos[1],-10.0f,10.0f);
	ImGui::SameLine();
	ImGui::SliderFloat("New Light Z", &newLightPos[2],-10.0f,10.0f);*/
	ImGui::ColorEdit3("New Light Color", (float*)&newLight_color);
	ImGui::SliderFloat("New Light Intensity", &newLightIntensity, 0.0f, 5.0f);
	if (ImGui::Button("Add Light")) {
			addLight(glm::vec3(newLightPos[0],newLightPos[1],newLightPos[2]),
				glm::vec3(newLight_color.x,newLight_color.y,newLight_color.z), newLightIntensity);
	}
	if (numLights > 0) {
		ImGui::Text("Select Light");
		//const char* lightList[lights.size()];
		const char* lightNums[11] = {"0","1","2","3","4","5","6","7","8","9","10"};
		const char* lightList[lights.size()];
		for (int i = 0; i < lights.size(); i++) {
			lightList[i] = lightNums[i];
		}
		//std::copy(lightIDs.begin(), lightIDs.end(), lightList);
		if (ImGui::Button("Light"))
			ImGui::OpenPopup("light_list");
		ImGui::SameLine();
		ImGui::TextUnformatted(lightList[selectedLight]);
		if (ImGui::BeginPopup("light_list")) {
			for (int i = 0; i < IM_ARRAYSIZE(lightList); i++) {
				if (ImGui::Selectable(lightList[i])) {
					selectedLight = i;
				}
			}
			ImGui::EndPopup();
		}
		float selectedLightPos[3] = {lights[selectedLight].position.x, lights[selectedLight].position.y, lights[selectedLight].position.z};
		ImVec4 selectedLight_color = ImVec4(lights[selectedLight].color.x, lights[selectedLight].color.y, lights[selectedLight].color.z,1.0);
		float selectedLightIntensity = lights[selectedLight].intensity;
		ImGui::SliderFloat3("Selected Light Pos", selectedLightPos,-10.0f,10.0f);
		/*ImGui::SliderFloat("Selected Light X", &selectedLightPos[0],-10.0f,10.0f);
		ImGui::SameLine();
		ImGui::SliderFloat("Selected Light Y", &selectedLightPos[1],-10.0f,10.0f);
		ImGui::SameLine();
		ImGui::SliderFloat("Selected Light Z", &selectedLightPos[2],-10.0f,10.0f);*/
		ImGui::ColorEdit3("Selected Light Color", (float*)&selectedLight_color);
		ImGui::SliderFloat("Selected Light Intensity", &selectedLightIntensity, 0.0f, 5.0f);
		lights[selectedLight].position = glm::vec3(selectedLightPos[0],selectedLightPos[1],selectedLightPos[2]);
		lights[selectedLight].color = glm::vec3(selectedLight_color.x,selectedLight_color.y,selectedLight_color.z);
		lights[selectedLight].intensity = selectedLightIntensity;
	}
	ImGui::End();
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
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, 0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(0.5,0.5,0.5);
				Object wall = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), glm::vec3(0,0,0), 1, 1, color, 'w');
				map[index] = wall;
				objects.push_back(map[index]);
			}
			else if(env == '0'){	//floor
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, -0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(0.5,0.5,0.5);
				Object floor = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), glm::vec3(0,0,0), 1, 0, color, 'z');
				objects.push_back(floor);
				map[index] = floor;
			}
			else if(env == 'G'){	//goal
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, -0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(1,1,0);
				Object goal = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), glm::vec3(0,0,0), 1, -1, color, 'g');
				objects.push_back(goal);
				map[index] = goal;
			}
			else if(env == 'S'){	//start
				//make start
				mapStart = glm::vec3(j - mapWidth / 2.0f + 0.5f, 0.49f, i + 0.5f - mapHeight/2.0f);
				player.pos = mapStart;
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, -0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(1,0.2,0.2);
				Object start = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), glm::vec3(0,0,0), 1, -1, color, 's');
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
				Object door = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), glm::vec3(0,0,0), 1, -1, color, env);
				objects.push_back(door);
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
				Object key = Object(pos, glm::vec3(.4,.4,.4), glm::vec3(0,1,0), glm::vec3(0,0,0), 0, -1, color, env);
				objects.push_back(key);
				map[index] = key;
				pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, -0.5f, i + 0.5f - mapHeight/2.0f);
				Object floor = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), glm::vec3(0,0,0), 1, 0, color, 'z');
				objects.push_back(floor);
			}
		}
	}

	///BORDER WALLS FOR MAP///
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
	char env;
	for(int i = 0; i < mapHeight; i++){
		for(int j = 0; j < mapWidth; j++){
			in >> env;
			if(env == 'W'){		//wall
				glm::vec3 pos = glm::vec3(j - mapWidth / 2.0f + 0.5f, 0.5f, i + 0.5f - mapHeight/2.0f);
				glm::vec3 color = glm::vec3(0.5,0.5,0.5);
				Object wall = Object(pos, glm::vec3(1,1,1), glm::vec3(1,1,1), glm::vec3(0,0,0), 1, 1, color, 'w');
				victoryMap.push_back(wall);
			}
		}
	}
}

bool canWalk(float x, float z){
	//float playerRadius = 0.002;
	if(abs(x - mapWidth/2.f) < 0.1f || abs(z - mapHeight/2.f) < 0.1f || abs(x + mapWidth/2.f) < 0.1f || abs(z + mapHeight/2.f) < 0.1f) return false;

	for(int i = 0; i < objects.size(); i++){
		if(objects[i].type == 'w' || (objects[i].type >= 'A' && objects[i].type <= 'F')){
			if(abs(x - objects[i].pos.x) < (0.6f * objects[i].scale.x) && abs(z - objects[i].pos.z) < (0.6f * objects[i].scale.z) && (objects[i].pos.y - player.pos.y) >= (-1.f * objects[i].scale.y) && (objects[i].pos.y - player.pos.y) <= (0.6f * objects[i].scale.y)) return false;
		}
	}
	return true;
}

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

int pickUp(){
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
				//start_pos = objects[i].pos;
				//printf("\nObject type: %c\n", objects[i].type);
				return i;
			}
		}
	}
	return -1;
}