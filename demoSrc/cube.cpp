#include "../include/main.h"
#include "../include/Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
//#include <glm/mat4x4.hpp>
//#include <glm/gtc/matrix_transform.hpp>

// define constants and common resources---------------------------------------

//window params
int xSize = 1366;
int ySize = 768;

// sdl window, context
SDL_Window* window;
SDL_GLContext mainContext;

//buffer data (temp)
const uint32_t points = 4;
const uint32_t floatsPerPoint = 3;
const uint32_t floatsPerColor = 4;

//a simple square with colors
/*const GLfloat diamond[points][floatsPerPoint] = {
	{ -0.5,  0.5,  0.5 }, // Top left
	{  0.5,  0.5,  0.5 }, // Top right
	{  0.5, -0.5,  0.5 }, // Bottom right 
	{ -0.5, -0.5,  0.5 }, // Bottom left
};

const GLfloat colors[points][floatsPerColor] = {
	{ 0.0, 1.0, 0.0, 1.0 }, // Top left
	{ 1.0, 1.0, 0.0, 1.0  }, // Top right
	{ 1.0, 0.0, 0.0, 1.0  }, // Bottom right 
	{ 0.0, 0.0, 1.0, 1.0  }, // Bottom left
};*/

//for later loading as a cube
std::vector<GLfloat> positions;
std::vector<GLfloat> colors;


//vertex buffer and array id data structures
GLuint vbo[2], vao[1]; 

//position and color data indices
const uint32_t positionAttributeIndex = 0, colorAttributeIndex = 1;

//shader loader reference
Shader shader;

//initialize resources
bool initialize(){

        //Initialize Library
        if(SDL_Init(SDL_INIT_EVERYTHING) !=0){
                std::cout << "SDL Initialization failure: " << SDL_GetError() << std::endl;
                return false;
        } 

        //Set up window
        window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, xSize, ySize, SDL_WINDOW_OPENGL);

        if(window == nullptr){
                std::cout << "SDL Window Initialization failure: " << SDL_GetError() << std::endl;
                return false;
        }
               
        //Set Up open gl context
        mainContext = SDL_GL_CreateContext(window);

        //Set GL Attributes
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetSwapInterval(1);

        #ifndef __APPLE__
	glewExperimental = GL_TRUE;
	glewInit();
	#endif 

        return true;
}

//read coordinates from a file into a vertex buffer
std::vector<GLfloat> readFileIntoBuffer(std::string fileName){
        // Open file
        std::ifstream t(fileName);

        std::vector<GLfloat> result;
        std::cout << "Reading : " << fileName << std::endl;

        while (t.good())
        {
                std::string str;
                t >> str;

                GLfloat f = std::atof(str.c_str());

                result.push_back(f);
        }

        return result;
}

//setup gl buffer data structures
bool setupBuffers(){
        
        positions = readFileIntoBuffer("data/positions.txt");
        colors = readFileIntoBuffer("data/colors.txt");
        
        // Generate and assign two Vertex Buffer Objects to our handle
	glGenBuffers(2, vbo);

        // Generate and assign a Vertex Array Object to our handle
	glGenVertexArrays(1, vao);

        //bind vao as current vertex array
        glBindVertexArray(vao[0]);

        //bind the first buffer as vertex data, and necessary bookkeeping -----
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

        //copy diamond structure into bound vertex buffer
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), &positions[0], GL_STATIC_DRAW);

        //specify position index, and number of floats er index
	glVertexAttribPointer(positionAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

        //enable attribute within the vertex array
	glEnableVertexAttribArray(positionAttributeIndex);
	//glEnableVertexAttribArray(colorAttributeIndex);

        //setup color buffer and data -----------------------------------------
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), &colors[0], GL_STATIC_DRAW);

	glVertexAttribPointer(colorAttributeIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);

        // run shader setup
	if (!shader.Init())
                return false;

        shader.UseProgram();

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return true;
}

void setUpMatrices(){
        glm::mat4 model = glm::translate( glm::mat4(1.0f), glm::vec3( 0, 0, 0) );
         
        glm::mat4 view = glm::lookAt
        (
            glm::vec3(0, 0,-5), // Camera is at ( 0, 0, -5), in World Space
            glm::vec3(0, 0, 0), // And looks at the center
            glm::vec3(0,-1, 0)  // Camera is upside-down
        );
         
        glm::mat4 projection = glm::perspective
        (
            45.0f,              // 45º field of view
            1920.0f / 1080.0f,  // 16:9 aspect ratio
            0.1f,               // Only render what's 0.1 or more away from camera
            100.0f              // Only render what's 100 or less away from camera
        );
         
        glm::mat4 modelViewProjection = projection * view * model;

        // Get the ID
        int matrixID = glGetUniformLocation(shader.shaderProgram, "mvp");
         
        //Set the matrix
        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &modelViewProjection[0][0]);
}

//draw array data (temp)
void render(){
        //grey background
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        //enable colors
        glEnableVertexAttribArray(positionAttributeIndex);
        glEnableVertexAttribArray(colorAttributeIndex);

        glDrawArrays(GL_TRIANGLES, 0, positions.size()/3);

        SDL_GL_SwapWindow(window);
}

// clean resources on exit
void cleanup(){
        
        shader.CleanUp();

        SDL_GL_DeleteContext(mainContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
}

int main(int argc, char *argv[]){
        
        // run intialization
        initialize();        

        // setup a black backround
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);

        //setup the vertex buffer and vertex array
        if(!setupBuffers())
                return -1;

        setUpMatrices();

        //render square
        render();

        bool loop = true;

        while(loop){

                //Process input
                SDL_Event event;

                while(SDL_PollEvent(&event)){

                        //exit
                        if(event.type == SDL_QUIT)
                                loop = false;

                }

                SDL_Delay( 16 );
        }

        cleanup();

        std::cout << "Game Exit!" << std::endl;
        return 0;
}