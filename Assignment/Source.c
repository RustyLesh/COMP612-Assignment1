/******************************************************************************
 *
 * Animation v1.0 (23/02/2021)
 *
 * This template provides a basic FPS-limited render loop for an animated scene.
 *
 ******************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#pragma region Structs

typedef struct
{
	float x;
	float y;
}vertex2f;

typedef struct
{
	float x;
	float y;
	float z;
}vertex3f;

typedef struct
{
	float r;
	float g;
	float b;
}rgb;

typedef struct
{
	float r;
	float g;
	float b;
	float a;
}rgba;

typedef struct
{
	int isOn;
	float timer;
}windowLight;

typedef struct
{
	int type; // 0 = empty, 1 = ellipse, 2 = rectangle
	float width;
	float height;
	vertex2f origin; //ellipse center, rectangle, top left corner
}cloudPiece;

typedef struct
{
	vertex2f origin;
	cloudPiece cloudPieces[100];
	float speed;
	float alpha;
}cloud;

typedef struct {
	vertex2f position;
	float size;
	float fallSpeed;
	float windSpeed;
	rgba colour;
	int isActive;
	int recycleDelay;
	int resetCounter;
	int isCounted;
} Particle_t;

typedef struct {
	vertex2f origin;
	int isActive;
	float alpha;
}star;
#pragma endregion

#pragma region Animation & Timing Setup

// Target frame rate (number of Frames Per Second).
#define TARGET_FPS 60
// Ideal time each frame should be displayed for (in milliseconds).
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;
// Frame time in fractional seconds.
// Note: This is calculated to accurately reflect the truncated integer value of
// FRAME_TIME, which is used for timing, rather than the more accurate fractional
// value we'd get if we simply calculated "FRAME_TIME_SEC = 1.0f / TARGET_FPS".
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;
// Time we started preparing the current frame (in milliseconds since GLUT was initialized).
unsigned int frameStartTime = 0;

#pragma endregion

#pragma region Global Variables

// The ratio of the circumference to the diameter of a circle.
#define PI 3.14159265
// Conversion multiplier for converting from degrees to Radians
// NOTE: this is wrong in the "OpenGL: A Primer" book, pg. 45
#define DEG_TO_RAD PI / 180.0f
// Speed of rotation in Degrees Per Second
const float ROTATION_SPEED = 40.0f;
// Current angle of rotationk
GLfloat theta = 0.0;

#define MAX_PARTICLES 100
Particle_t snowParticles[MAX_PARTICLES];

//Global variables
#define NUMBER_OF_CLOUDS 10
cloud clouds[NUMBER_OF_CLOUDS];

#pragma region Timers and counters

float nightFilterAlpha = 0;
float isGettingDarker = 0;
int nightAlphacounter = 0;

int timeCounter = 0;
int snowCounter = 0;
int snowDelay = 10;
int snowRecycleDelayCounter = 0;
int snowActiveParticlesCounter = 0;

int isWindActive = 0;

int starsAreDirty = 1;

int activeParticles = 0;
windowLight windowLights[500000];
int windowLightcounter = 0;

int isInfoPanelVisable = 1;

int isMountainsToggled = 0;

#define MAXIMUM_STARS 40
star stars[MAXIMUM_STARS];
#pragma endregion

vertex2f randomMountainPoint1;
vertex2f randomMountainPoint2;
vertex2f randomMountainPoint3;

//-----Settings----//

	//Snow//
int isSnowActive = 1;
float snowFallSpeed = 0.0015;

float snowWindSpeed = 0.001;
float snowWindSpeedMultiplier = 0.01;

//Day-Night//
float maxNightDarkness = .9;
float dayNightTransitionSpeed = 0.005;

int nightTimeLength = 1500;
int morningTimeLength = 500; // Always > 1 or error

//Lights//
float windowLightsMinTime = 1000;
float windowLightsMaxtime = 5000;
float windowLightsChanceToturnOff = 0.005; // out of 1
float windowLightsChanceToturnOn = 0.0001; // out of 1  //0.0001
float windowMinOffTime = 100;
float windowMaxOffTime = 1000000;

int areLightsOn = 0;
float lightsOnAtAlpha = 0; //At what night alpha to turn lights on -1 to 1 , 1 being dark

	//Clouds//
float cloudSpeed = 0.05;
float cloudMinHeight = -0.1;
float cloudMaxHeight = 1;
#pragma endregion

#pragma region Colours

#pragma region WindowLights

rgb windowsYellow = { 0.8,0.8,.5 };
rgb windowsPaleBlue = { 0.529, 0.808, 0.980 };
rgb windowsIceBlue = { 0.1, 0.6, 7.000 };
rgb windowsBlue = { 0.0344, 0.227, 0.860 };
rgb windowsFadedBlue = { 0.281, 0.425, 0.760 };
rgb windowsDarkBlue = { 0.1, 0.1, 0.390 };
#pragma endregion

#pragma endregion

#pragma region Keyboard Input Handling Setup

// Define all character keys used for input (add any new key definitions here).
// Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
// characters typed by the user to lowercase, so the SHIFT key is ignored.
#define KEY_EXIT 27 // Escape key.

#pragma endregion

#pragma region GLUT Callback Prototypes

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void idle(void);

#pragma endregion


#pragma region My Prototypes

void printText(float, float, struct rgb, char[]);
void printHotkeys(float, float, rgb);
void showInfoPanel(float, float);
void displayTimeOfDay(float x, float y, rgb colour);

void background();
void drawGround();
void drawSnowParticle(Particle_t);
void updateSnowParticle(Particle_t*);
void drawFrontSnow();

float randomFloat(float, float);
void drawNightFilter();
void updateNightFilter();

void drawBackCityScape();
void drawMiddleCityScape();
void drawFrontCityScape();

void drawFrontCityScapeLights();
void drawMiddleCityScapeLights();
void drawBackCityScapeLights();

void drawRectangle(vertex2f, float, float, rgb);
void drawTiltedRectangle(vertex2f origin, float width, float height, float leftTilt, float rightTilt, rgb colour);
void drawRectangleWithAlpha(vertex2f origin, float width, float height, rgba colour);
void drawCircle(vertex2f, float, rgb, rgb);
void drawEllipse(vertex2f, float, float, int, rgba, rgba);

void drawSnowman(vertex2f origin);

void drawBuildingStructure(vertex2f, float, float, float, float, float, rgb, rgb);
void drawWindows(vertex2f origin, float height, float width, float leftTilt, float rightTilt, float spacing, int columns, int rows, rgb colour);
void drawWindowsLights(vertex2f origin, float height, float width, float leftTilt, float rightTilt, float spacing, int columns, int rows, rgb colour);

void drawClouds();
void updateClouds(cloud *cloud);
void createCloudArray();
void createCloudPiece(int cloudNumber, int pieceNumber, float originX, float originY, int pieceType, float width, float height);

void toggleLights();
rgb changecolourOvertime(rgb colour1, rgb colour2);

void updateStars();
void drawStar(star star);
void drawAllStars();
void displayAmountOfActiveParticles(float, float, rgb);
void drawMountains();
void randmoiseMountainPoints();

void drawCrescentMoon(float x, float y, float step, float scale, float fullness);

void main(int argc, char **argv);
void init(void);
void think(void);
#pragma endregion

void main(int argc, char **argv)
{
	// Initialize the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(900, 900);
	glutCreateWindow("Animation");
	// Set up the scene.
	init();
	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutIdleFunc(idle);
	// Record when we started rendering the very first frame (which should happenafter we call glutMainLoop).
	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);
	// Enter the main drawing loop (this will never return).
	glutMainLoop();
}

#pragma region GLUT Callbacks

void display(void)
{
	// clear the screen 
	glClear(GL_COLOR_BUFFER_BIT);

	vertex2f origin = { 2.2 };
	drawWindowsLights(origin, 0, 0, 0, 0, 0, 1, 1, windowsYellow);

	background();

	drawClouds();

	if (!isMountainsToggled)
	{
		drawBackCityScape();

		drawMiddleCityScape();

		drawFrontCityScape();
	}
	else
	{
		drawMountains();
	}

	drawGround();

	drawFrontSnow();

	vertex2f snowManOrigin = { -0.8,-0.9 };
	drawSnowman(snowManOrigin);

	drawNightFilter();

	drawAllStars();

	if (!isMountainsToggled)
	{
		drawFrontCityScapeLights();

		drawMiddleCityScapeLights();

		drawBackCityScapeLights();
	}
	//Text

	drawCrescentMoon(-0.705f, 0.7f, 0.1f, -0.195f, -0.45f);

	showInfoPanel(-0.9, 0.9);



	timeCounter++;
	windowLightcounter = 0;
	// send all output to display 
	glutSwapBuffers();
}
/*
Called when the OpenGL window has been resized.
*/
void reshape(int width, int h)
{
}
/*
Called each time a character key (e.g. a letter, number, or symbol) is
pressed.
*/
void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {
		/*
		TEMPLATE: Add any new character key controls here.
		Rather than using literals (e.g. "d" for diagnostics), create a
		new KEY_
		definition in the "Keyboard Input Handling Setup" section of this
		file.
		*/
	case KEY_EXIT:
		exit(0);
		break;

	case 's':
		isSnowActive = !isSnowActive;
		break;

	case 'd':
		isInfoPanelVisable = !isInfoPanelVisable;
		break;

	case 'f':
		isMountainsToggled = !isMountainsToggled;
		randmoiseMountainPoints();
		break;

	default:
		;
	}


}

/*
Called by GLUT when it's not rendering a frame.
Note: We use this to handle animation and timing. You shouldn't need to
modify
this callback at all. Instead, place your animation logic (e.g. moving or
rotating
things) within the think() method provided with this template.
*/
void idle(void)
{
	// Wait until it's time to render the next frame.
	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) -
		frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		// This frame took less time to render than the ideal FRAME_TIME: we'llsuspend this thread for the remaining time,
		// so we're not taking up the CPU until we need to render another frame.
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}
	// Begin processing the next frame.
	frameStartTime = glutGet(GLUT_ELAPSED_TIME); // Record when we started work on the new frame.
	think(); // Update our simulated world before the next call to display().
	glutPostRedisplay(); // Tell OpenGL there's a new frame ready to be drawn.
}

/*
Initialise OpenGL and set up our scene before we begin the render loop.
*/
void init(void)
{
	srand(time(0));

	glClearColor(0.0, 0.0, 0.0, 0.1);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	createCloudArray();

	float n = 0;
	float size = 0;
	float fallSpeed = 0;

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		size = randomFloat(2, 10);

		n = randomFloat(-1, 1);
		vertex2f pos = { n , 1 };

		fallSpeed = size / 2000.f;

		rgba snowColour = { 1, 1, 1, 1 };

		Particle_t snowParticle = { pos, size, fallSpeed, .01 ,snowColour, 0 , 1 };
		snowParticles[i] = snowParticle;
	}
	randmoiseMountainPoints();
}

/*
Advance our animation by FRAME_TIME milliseconds.
Note: Our template's GLUT idle() callback calls this once before each new
frame is drawn, EXCEPT the very first frame drawn after our application
starts. Any setup required before the first frame is drawn should be placed
in init().
*/

void think(void)
{
	toggleLights();//TODO better light logic , eg slowly switch off and on the lights as it gets darker / lighter

	updateNightFilter();

	updateStars();


	if (snowCounter > MAX_PARTICLES)
		snowCounter = 0;

	if (timeCounter % snowDelay == 0 && !snowParticles[snowCounter].isActive && isSnowActive)
	{
		snowParticles[snowCounter].isActive = 1;
		snowParticles[snowCounter].recycleDelay = (int)randomFloat(1, 100);
		snowDelay = (int)randomFloat(1, 15);

		snowActiveParticlesCounter++;
		snowParticles[snowCounter].isCounted = 1;
		snowCounter++;
	}


	for (int i = 0; i < MAX_PARTICLES; i++)
	{

		if (snowParticles[i].isActive)
		{
			//fall speed
			snowParticles[i].position.y -= snowParticles[i].fallSpeed;

			updateSnowParticle(&snowParticles[i]);

			//Drift //TODO: drift
		}
	}

	for (int i = 0; i < NUMBER_OF_CLOUDS; i++)
	{
		updateClouds(&clouds[i]);
	}
}

#pragma endregion

//--Functions--//

//Sky
void background()
{
	// Top
	glBegin(GL_POLYGON);

	rgb colour1a = { 0.529, 0.808, 0.980 };
	rgb colour1b = { -maxNightDarkness, -maxNightDarkness, -maxNightDarkness };

	colour1a = changecolourOvertime(colour1a, colour1b);

	glColor3f(colour1a.r, colour1a.g, colour1a.b);

	glVertex2f(-1.0, 1.0f);
	glVertex2f(1.0, 1.0f);

	//Bottom
	rgb colour2a = { 0.000, 0.749, 1.000f };
	rgb colour2b = { 0.1, 0.1, 0.7 };

	colour2a = changecolourOvertime(colour2a, colour2b);
	glColor3f(colour2a.r, colour2a.g, colour2a.b);

	glVertex2f(1.0, -1.0f);
	glVertex2f(-1.0, -1.0f);

	glEnd();
}

void drawGround()
{
	glBegin(GL_POLYGON);
	glColor3f(0.2, 0.2, 0.2);

	glVertex2f(-1.0, -0.9f);
	glVertex2f(-0.3, -0.8f);
	glVertex2f(0.5, -1.0f);
	glVertex2f(-1.0f, -2.0f);

	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.1, 0.1);

	glVertex2f(-0.9, -1);
	glVertex2f(-0.3, -0.85f);
	glVertex2f(0.7, -1.1f);
	glVertex2f(-0.9, -2.1f);

	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(0.15, 0.15, 0.15);

	glVertex2f(-0.9, -1);
	glVertex2f(-0.3, -0.85f);

	glVertex2f(-0.3, -2);

	glEnd();
}

//--Snow Particles--//
void drawFrontSnow()
{
	//TODO: Make snow gradually fade when deactivated
	{
		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			if (snowParticles[i].isActive)
			{
				glPointSize(snowParticles[i].size);
				drawSnowParticle(snowParticles[i]);
			}
		}
	}
}

void drawSnowParticle(Particle_t particle)
{
	glBegin(GL_POINTS);

	glColor4f(particle.colour.r, particle.colour.g, particle.colour.b, particle.colour.a);
	glVertex2f(particle.position.x, particle.position.y);

	glEnd();
}

void updateSnowParticle(Particle_t *particle)
{
	if (particle->position.y <= -1)
	{
		if (particle->resetCounter > particle->recycleDelay && isSnowActive)
		{
			particle->recycleDelay = (int)randomFloat(100, 500);
			particle->position.y = 1;
			particle->position.x = randomFloat(-1, 1);
			particle->windSpeed = 0;
			particle->resetCounter = 0;

			if (!particle->isCounted)
			{
				particle->isCounted = 1;
				snowActiveParticlesCounter++;
			}

		}

		if (!isSnowActive && particle->isCounted)
		{
			snowActiveParticlesCounter--;
			particle->isCounted = 0;
		}
	}

	if (particle->position.y < -1 && isSnowActive)
		particle->resetCounter++;

	if (isWindActive)
	{
		particle->windSpeed += snowWindSpeed;

		particle->position.x -= (snowWindSpeedMultiplier*particle->windSpeed);
	}

}

//--Nightfilter--//
void drawNightFilter()
{
	glBegin(GL_POLYGON);

	glColor4f(0, 0, 0, nightFilterAlpha);
	glVertex2f(-1.0, 1.0f);
	glVertex2f(1.0, 1.0f);

	glColor4f(0, 0, 0, nightFilterAlpha / 2);
	glVertex2f(1.0, -1.0f);
	glVertex2f(-1.0, -1.0f);

	glEnd();
}

void updateNightFilter()
{
	//getting darker
	if (nightFilterAlpha >= maxNightDarkness)
	{
		nightAlphacounter++;
		if (nightAlphacounter % nightTimeLength == 0)
		{
			isGettingDarker = 0;
			nightAlphacounter = 0;
			nightFilterAlpha = maxNightDarkness;
		}

	}

	//getting brighter
	if (nightFilterAlpha <= -1)
	{
		nightAlphacounter++;

		if (nightAlphacounter % morningTimeLength == 0)
		{
			isGettingDarker = 1;
			nightAlphacounter = 0;
			nightFilterAlpha = -1;
		}
	}

	//night time toggle
	if (isGettingDarker)
	{
		//set to sunset
		nightFilterAlpha += dayNightTransitionSpeed;

		if (nightFilterAlpha >= maxNightDarkness)
			nightFilterAlpha = maxNightDarkness;
	}
	else
		//set to sunrise
		nightFilterAlpha -= dayNightTransitionSpeed;

	if (nightFilterAlpha < -1)
		nightFilterAlpha = -1;

	if (nightFilterAlpha > 1)
		nightFilterAlpha = 1;
}

//--City--//
void drawFrontCityScape()	//front right buildings
{
#pragma region Back

	vertex2f building2 = { 0.6, -1.0 };
	rgb building2Colour1 = { .1, .1 ,.8 };
	rgb building2Colour2 = { 0.1, 0, 0.2 };
	drawBuildingStructure(building2, 1.7f, 0.25f, 0.2f, 0.05f, 0.05f, building2Colour1, building2Colour2);

	vertex2f windowBackOrigin = { 0.612, 0.5 };
	drawWindows(windowBackOrigin, 1.3, 0.37, 0.005, 0, 0.05, 5, 10, windowsPaleBlue);

#pragma endregion

#pragma region Middle

	vertex2f building3 = { 0.65, -1.0 };
	rgb building3Colour1 = { .1, .1 ,.1 };
	rgb building4Colour2 = { 0.05, 0, 0.1 };
	drawBuildingStructure(building3, 1.0f, 0.25f, 0.2f, 0.05f, 0.05f, building3Colour1, building4Colour2);

	vertex2f windowsOrigin2 = { 0.665, -0.19 };
	drawWindows(windowsOrigin2, 1.2, 0.34, 0.007, 0, 0.05, 4, 10, windowsFadedBlue);

#pragma endregion

#pragma region Front

	vertex2f building1 = { 0.18, -1 };
	rgb building1Colour1 = { 0.1, 0.05, 0.4 };
	rgb building1Colour2 = { 0.1, 0, 0.2 };
	drawBuildingStructure(building1, 0.8f, 0.25f, 0.235f, 0.05f, 0.05f, building1Colour1, building1Colour2);

	vertex2f windowsOrigin1 = { 0.215, -.32 };
	drawWindows(windowsOrigin1, 0.9, 0.27, 0.006, 0, 0.035, 4, 10, windowsFadedBlue);

#pragma endregion

}

void drawMiddleCityScape()
{
#pragma region Far left

	vertex2f building1 = { -0.95, -1 };
	float building1Height = 0.8f;
	rgb building1Colour1 = { 0.1, 0.05, 0.4 };
	rgb building1Colour2 = { 0.1, 0, 0.2 };
	drawBuildingStructure(building1, building1Height, 0.08f, 0.1f, 0.0f, 0.0f, building1Colour1, building1Colour2);

	vertex2f rectangle1 = { building1.x + 0.02, (building1Height + building1.y) };
	rgb rectangle1Colour1 = { (building1Colour1.r - 0.02), (building1Colour1.g - 0.02), (building1Colour1.b) - 0.02 };
	drawRectangle(rectangle1, 0.12f, .03f, rectangle1Colour1);

	vertex2f rectangle2 = { building1.x + 0.03, (building1Height + building1.y) };
	drawRectangle(rectangle2, 0.01f, .15f, rectangle1Colour1);

	vertex2f windowsOriginFL = { -0.86, -0.21 };
	drawWindows(windowsOriginFL, building1Height, 0.12, 0, 0, 0.02, 3, 5, windowsDarkBlue);
#pragma endregion

#pragma region Second from left

	vertex2f building3 = { -0.75, -1 };
	float building3Height = 1.1f;
	rgb building3Colour = { 0.1, 0.05, 0.4 };

	drawTiltedRectangle(building3, 0.13, building3Height, 0.0, 0.1, building3Colour);

#pragma endregion

#pragma region Third from left

	vertex2f building4 = { -0.58, -1 };
	rgb building4Colour = { 0.1, 0.05, 0.4 };

	drawRectangle(building4, 0.13, 0.7, building4Colour);
#pragma endregion

#pragma region Third from left

	vertex2f building5 = { -0.43, -1 };
	rgb building5Colour = { 0.1, 0.05, 0.4 };

	drawRectangle(building5, 0.2, 0.5, building5Colour);
#pragma endregion

#pragma region Far right

	//far right
	float building2Height = 0.9f;
	vertex2f building2 = { -0.15, -1.0 };
	rgb building2Colour1 = { .2, .1 ,.8 };
	rgb building2Colour2 = { 0.163, 0.110, 0.460 };
	drawBuildingStructure(building2, building2Height, 0.15f, 0.1f, 0.01f, 0.01f, building2Colour1, building2Colour2);

	//(vertex2f origin, float width, float height, float leftTilt, float rightTilt, float spacing, int columns, int rows, rgb colour);

	vertex2f windowOrigin2 = { building2.x + .01, building2Height - 1.01 };

	rgb windowsColour1 = { 0.3, 0.0, 0.8 };
	drawWindows(windowOrigin2, building2Height, 0.178, 0.0015, 0.0, 0.02, 4, 1, windowsColour1);
#pragma endregion

}

void drawBackCityScape()
{
	rgb buildingColour1 = { 0.000, 0.7, 1.000f };
	rgb buildingColour2 = { 0.529, 0.808, 0.980 };

#pragma region BG Rectangles
	float darker = 0.07;
	rgb rectangleColour = { buildingColour1.r - darker, buildingColour1.g - darker, buildingColour1.b - darker };

	float rectangle1Height = 0.9;
	vertex2f rectangle1 = { -1, -1 };
	drawRectangle(rectangle1, .1, rectangle1Height, rectangleColour);

	float rectangle1Height2 = 0.9;
	vertex2f rectangle2 = { -0.8, -1 };
	drawRectangle(rectangle2, .1, rectangle1Height2, rectangleColour);
#pragma endregion

#pragma region Left1

	float building1Height = 1.0f;
	vertex2f building1 = { -0.94, -1.0 };
	drawBuildingStructure(building1, building1Height, 0.07f, 0.03f, 0.0f, 0.0f, buildingColour1, buildingColour2);

#pragma endregion

#pragma region LeftBack1

	float buildingB1Height = .8f;
	vertex2f buildingB1 = { -0.57, -1.0 };
	drawBuildingStructure(buildingB1, buildingB1Height, 0.07f, 0.03f, 0.0f, 0.007f, buildingColour1, buildingColour2);

#pragma endregion

#pragma region Left2

	float building2Height = .95f;
	vertex2f building2 = { -0.65, -1.0 };
	drawBuildingStructure(building2, building2Height, 0.07f, 0.03f, 0.0f, 0.007f, buildingColour1, buildingColour2);

#pragma endregion

#pragma region LeftBack2

	float buildingB2Height = 1.3f;
	vertex2f buildingB2 = { -0.45, -1.0 };
	float LB2ColourDark = 0.03;
	rgb leftBack2Colour = { buildingColour1.r - LB2ColourDark, buildingColour1.g - LB2ColourDark, buildingColour1.b - LB2ColourDark };
	drawBuildingStructure(buildingB2, buildingB2Height, 0.2f, 0.12f, 0.0f, 0.007f, leftBack2Colour, buildingColour2);

#pragma endregion

#pragma region Left3

	float building3Height = .9f;
	vertex2f building3 = { -0.465, -1.0 };
	drawBuildingStructure(building3, building3Height, 0.1f, 0.05f, 0.0f, 0.007f, buildingColour1, buildingColour2);

#pragma endregion

#pragma region Left4

	float building4Height = 1.2f;
	vertex2f building4 = { -.05, -1.0 };
	drawBuildingStructure(building4, building4Height, 0.2f, 0.07f, 0.0f, 0.007f, buildingColour1, buildingColour2);

#pragma endregion

#pragma region Right1

	float building5Height = 1.15;
	vertex2f building5 = { 0.5, -1.0 };
	drawBuildingStructure(building5, building5Height, 0.07f, 0.03f, 0.0f, 0.007f, buildingColour1, buildingColour2);

#pragma endregion

#pragma region Right2

	float building6Height = 0.95;
	vertex2f building6 = { 0.4, -1.0 };
	drawBuildingStructure(building6, building6Height, 0.07f, 0.03f, 0.0f, 0.007f, buildingColour1, buildingColour2);

#pragma endregion
}

//--City Lights--//  //TODO: Reset windows each night (add update windows)
void drawFrontCityScapeLights()
{
#pragma region MyRegion
	vertex2f windowBackOrigin = { 0.612, 0.5 };
	drawWindowsLights(windowBackOrigin, 0.781, 0.37, 0.005, 0, 0.05, 5, 6, windowsYellow);

#pragma endregion

#pragma region Middle

	vertex2f windowsOrigin2 = { 0.665, -0.19 };
	drawWindowsLights(windowsOrigin2, 1.2, 0.34, 0.007, 0, 0.05, 4, 10, windowsYellow);
#pragma endregion

#pragma region Front

	vertex2f windowsOrigin1 = { 0.215, -.32 };
	drawWindowsLights(windowsOrigin1, 0.81, 0.27, 0.006, 0, 0.035, 4, 9, windowsYellow);

#pragma endregion

}

void drawMiddleCityScapeLights()
{
#pragma region Far left

	vertex2f windowOriginFL = { -0.86, -0.21 };
	//drawWindowsLights(windowOriginFL, 0.35, 0.12, 0, 0, 0.02, 3, 3, windowsPaleBlue);

#pragma endregion

#pragma region farRightBuilding

	vertex2f windowOrigin2 = { -0.14, -0.11 };
	drawWindowsLights(windowOrigin2, 0.7, 0.178, 0.0015, 0.0, 0.02, 4, 1, windowsYellow);

#pragma endregion
}

void drawBackCityScapeLights()
{
#pragma region Back building 2

	vertex2f windowsBB2A = { -0.43, 0.28 };
	drawWindowsLights(windowsBB2A, 0.45f, 0.2, 0, 0, 0.02, 4, 8, windowsYellow);

	vertex2f windowsBB2B = { -0.31, -0.09 };
	drawWindowsLights(windowsBB2B, 0.45f, 0.05, 0, 0, 0.02, 1, 8, windowsYellow);

#pragma endregion

#pragma region Left4
	vertex2f windowsL4A = { -0.041, 0.19 };
	drawWindowsLights(windowsL4A, 0.4f, 0.274, 0, 0, 0.04, 4, 5, windowsYellow);

	vertex2f windowsL4B = { .10559, -0.11 };
	drawWindowsLights(windowsL4B, 0.64f, 0.068, 0, 0, 0.04, 1, 8, windowsYellow);

#pragma endregion

#pragma region Right1

	vertex2f originR1Windows = { 0.508, 0.135 };
	drawWindowsLights(originR1Windows, 0.45, 0.08, 0, 0, 0.015, 3, 13, windowsYellow);

#pragma endregion

#pragma region Right2

	vertex2f originR2Windows = { 0.4085, -0.06 };
	drawWindowsLights(originR2Windows, 0.17, 0.08, 0, 0, 0.015, 3, 5, windowsYellow);

#pragma endregion
}

//--Building Parts--//
void drawBuildingStructure(vertex2f origin, float height, float frontWidth, float sideWidth, float topLeftOffset, float topRightOffset, rgb colour1, rgb colour2)
{
	//building front
	glBegin(GL_POLYGON);

	glColor3f(colour1.r, colour1.g, colour1.b);

	glVertex2f(origin.x, origin.y);
	glVertex2f(origin.x, origin.y + height - topLeftOffset);
	glVertex2f(origin.x + frontWidth, origin.y + height);
	glVertex2f(origin.x + frontWidth, origin.y);

	glEnd();

	//buuilding side
	glBegin(GL_POLYGON);

	glColor3f(colour2.r, colour2.g, colour2.b);

	glVertex2f(origin.x + frontWidth, origin.y);
	glVertex2f(origin.x + frontWidth, origin.y + height);
	glVertex2f(origin.x + frontWidth + sideWidth, origin.y + height - topRightOffset);
	glVertex2f(origin.x + frontWidth + sideWidth, origin.y);

	glEnd();
}

void drawWindows(vertex2f origin, float height, float width, float leftTilt, float rightTilt, float spacing, int columns, int rows, rgb colour)
{
	glColor3f(colour.r, colour.g, colour.b);

	float halfSpacing = spacing / 2;
	float windowWidth = (width / columns) - spacing;
	float windowHeight = (height / rows) - spacing;

	float m = ((origin.y - rightTilt) - (origin.y - leftTilt)) / (windowWidth);

	for (int j = 0; j < rows; ++j)
	{
		for (int i = 0; i < columns; ++i) //TODO: simplify like above (if time)
		{
			glBegin(GL_POLYGON);

			float  x = origin.x + (windowWidth * i + halfSpacing * i);
			float y = (m * x) - ((windowHeight * j) + (halfSpacing * j)) + origin.y;

			glVertex2f(x, y - leftTilt);

			glVertex2f(x + windowWidth, y - rightTilt);

			glVertex2f(x + windowWidth, y - windowHeight - rightTilt);

			glVertex2f(x, y - windowHeight - leftTilt);

			glEnd();
		}
	}
}

void drawWindowsLights(vertex2f origin, float height, float width, float leftTilt, float rightTilt, float spacing, int columns, int rows, rgb colour)
{
	glColor3f(colour.r, colour.g, colour.b);

	float halfSpacing = spacing / 2;
	float windowWidth = (width / columns) - spacing;
	float windowHeight = (height / rows) - spacing;

	float m = ((origin.y - rightTilt) - (origin.y - leftTilt)) / (windowWidth);

	for (int j = 0; j < rows; ++j)
	{

		for (int i = 0; i < columns; ++i) //TODO: seperate logic from drawing
		{
			if (windowLights[windowLightcounter].timer > windowLightsMaxtime)
			{
				windowLights[windowLightcounter].isOn = 0;
				windowLights[windowLightcounter].timer = 0;
			}

			if (windowLights[windowLightcounter].timer > windowLightsMinTime)
			{
				float roll = randomFloat(0, 1);
				if (roll < windowLightsChanceToturnOff)
				{
					windowLights[windowLightcounter].isOn = 0;
					windowLights[windowLightcounter].timer = 0;
				}
			}

			//if light is not on roll to turn it on
			if (!windowLights[windowLightcounter].isOn)
			{
				if (windowLights[windowLightcounter].timer > windowMaxOffTime)
				{
					windowLights[windowLightcounter].isOn = 1;
					windowLights[windowLightcounter].timer = 0;
				}

				if (windowLights[windowLightcounter].timer > windowMinOffTime)
				{
					float roll = randomFloat(0, 1);
					if (roll < windowLightsChanceToturnOn)
					{
						windowLights[windowLightcounter].isOn = 1;
						windowLights[windowLightcounter].timer = 0;
					}
				}
			}
			else
			{
				if (areLightsOn)
				{
					glBegin(GL_POLYGON);

					float  x = origin.x + (windowWidth * i + halfSpacing * i);
					float y = (m * x) - ((windowHeight * j) + (halfSpacing * j)) + origin.y;

					glVertex2f(x, y - leftTilt);

					glVertex2f(x + windowWidth, y - rightTilt);

					glVertex2f(x + windowWidth, y - windowHeight - rightTilt);

					glVertex2f(x, y - windowHeight - leftTilt);

					glEnd();
				}

			}
			windowLightcounter++;
			windowLights[windowLightcounter].timer++;
		}
	}
}

void toggleLights()
{
	if (nightFilterAlpha > lightsOnAtAlpha)
		areLightsOn = 1;
	else
		areLightsOn = 0;
}

//SnowMan
void drawSnowman(vertex2f origin)
{
	float headHeight = 0.25;
	float botBoddyHeight = 0.6;
	rgb colour = { 1,1,1 };
	rgb colour2 = { 0.0,0.0,0.0 };
	rgb colour3 = { .99,.99,.99 };

	float headXOffset = +0.1;
	float headYOffset = -0.15;
	vertex2f bodyOrigin = { -0.9,-1.3 }; //BottomBody
	vertex2f headOrigin = { bodyOrigin.x + headXOffset ,bodyOrigin.y + botBoddyHeight + headHeight + headYOffset }; //MiddleBodyb

#pragma region Nose

	glBegin(GL_POLYGON);

	glColor3f(1.00, 0.520, 0.0400);

	float noseTopffset = 0.15;
	float noseBotoffset = -0.1;
	vertex2f noseTipOffset = { .35,-0.1 }; //length, height

	glVertex2f(headOrigin.x, headOrigin.y + noseTopffset);

	glVertex2f(headOrigin.x + noseTipOffset.x, headOrigin.y + noseTipOffset.y);

	glVertex2f(headOrigin.x, headOrigin.y + noseBotoffset);

	glEnd();
#pragma endregion

	//Head
	drawCircle(headOrigin, headHeight, colour3, colour);

	//Body
	drawCircle(bodyOrigin, botBoddyHeight, colour2, colour);


	//TODO: TopBody
}

//Shapes
void drawRectangle(vertex2f origin, float width, float height, rgb colour)
{
	glBegin(GL_POLYGON);

	glColor3f(colour.r, colour.g, colour.b);

	glVertex2f(origin.x, origin.y);
	glVertex2f(origin.x, origin.y + height);
	glVertex2f(origin.x + width, origin.y + height);
	glVertex2f(origin.x + width, origin.y);

	glEnd();
}

void drawRectangleWithAlpha(vertex2f origin, float width, float height, rgba colour)
{
	glBegin(GL_POLYGON);

	glColor4f(colour.r, colour.g, colour.b, colour.a);

	glVertex2f(origin.x, origin.y);
	glVertex2f(origin.x, origin.y - height);
	glVertex2f(origin.x + width, origin.y - height);
	glVertex2f(origin.x + width, origin.y);

	glEnd();
}

//Origin bottom Left
void drawTiltedRectangle(vertex2f origin, float width, float height, float leftTilt, float rightTilt, rgb colour)
{
	glBegin(GL_POLYGON);

	glColor3f(colour.r, colour.g, colour.b);

	glVertex2f(origin.x, origin.y);
	glVertex2f(origin.x, origin.y + height - leftTilt);
	glVertex2f(origin.x + width, origin.y + height - rightTilt);
	glVertex2f(origin.x + width, origin.y);

	glEnd();
}

void drawCircle(vertex2f origin, float radius, rgb centerColour, rgb outerColour)
{
	glBegin(GL_TRIANGLE_FAN);

	glColor3f(centerColour.r, centerColour.g, centerColour.b);

	glVertex2f(origin.x, origin.y);

	glColor3f(outerColour.r, outerColour.g, outerColour.b);

	for (int theta = 0; theta <= 360; theta += 10)
	{
		glVertex2f((cos(theta * DEG_TO_RAD) * radius + origin.x), (sin(theta * DEG_TO_RAD) * radius + origin.y));
	}

	glEnd();
}

void drawEllipse(vertex2f origin, float width, float height, int num_segments, rgba insideColour, rgba outsideColour)
{
	float theta = 2 * 3.1415926 / (float)(num_segments);
	float c = cosf(theta);//precalculate the sine and cosine
	float s = sinf(theta);
	float t;

	float x = 1;//we start at angle = 0 
	float y = 0;

	glBegin(GL_TRIANGLE_FAN);

	glColor4f(insideColour.r, insideColour.g, insideColour.b, insideColour.a);
	glVertex2f(origin.x, origin.y);

	glColor4f(outsideColour.r, outsideColour.g, outsideColour.b, outsideColour.a);

	for (int i = 0; i <= num_segments; i++)
	{
		//apply radius and offset
		glVertex2f(x * width + origin.x, y * height + origin.y);//output vertex 

		//apply the rotation matrix
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}

	glEnd();
}

//--Clouds--//
void createCloudArray()
{
#pragma region Cloud0

	createCloudPiece(0, 0, 0, 0, 1, .6, 0.05);
	createCloudPiece(0, 1, 0.2, 0.03, 1, .3, .05);
	createCloudPiece(0, 2, -.26, 0.01, 1, .35, .05);
	createCloudPiece(0, 3, -0.07, 0.02, 1, .2, .05);

#pragma endregion

#pragma region Cloud1

	createCloudPiece(1, 0, 0.05, 0.01, 1, 0.07, .045);
	createCloudPiece(1, 1, 0.0, 0., 1, 0.1, .05);
	createCloudPiece(1, 2, -0.02, 0.02, 1, 0.1, .05);

#pragma endregion

#pragma region Cloud2

	createCloudPiece(2, 0, 0, 0, 1, .4, 0.05);
	createCloudPiece(2, 1, 0.1, 0.03, 1, .2, .05);
	createCloudPiece(2, 2, -.16, 0.01, 1, .2, .05);
	createCloudPiece(2, 3, -0.03, 0.02, 1, .1, .05);

#pragma endregion

#pragma region Cloud3

	createCloudPiece(3, 0, 0.08, 0.01, 1, 0.15, .06);
	createCloudPiece(3, 1, 0.0, 0., 1, 0.15, .05);
	createCloudPiece(3, 2, -0.1, -0.0, 1, 0.15, .05);
	createCloudPiece(3, 3, -0.01, 0.03, 1, 0.13, .06);

#pragma endregion

#pragma region Cloud4

	createCloudPiece(4, 0, 0.08, 0.01, 1, 0.15, .06);
	createCloudPiece(4, 1, 0.0, 0., 1, 0.15, .05);
	createCloudPiece(4, 2, -0.1, -0.0, 1, 0.15, .05);
	createCloudPiece(4, 3, -0.01, 0.03, 1, 0.13, .06);

#pragma endregion

#pragma region Cloud5

	createCloudPiece(5, 0, 0.05, 0.01, 1, 0.07, .045);
	createCloudPiece(5, 1, 0.0, 0., 1, 0.1, .05);
	createCloudPiece(5, 2, -0.02, 0.02, 1, 0.1, .05);

#pragma endregion

	for (int i = 0; i < NUMBER_OF_CLOUDS; i++)
	{
		clouds[i].origin.x = randomFloat(-15, -2);
		clouds[i].origin.y = randomFloat(cloudMinHeight, cloudMaxHeight);
		clouds[i].alpha = 1;
		clouds[i].speed = randomFloat(0.01, 0.05);
	}
}

void drawClouds()
{
	rgba colour = { 1,1,1,1 };
	rgb colour1 = { 1,1,1 };
	for (int i = 0; i < NUMBER_OF_CLOUDS; i++)
	{
		cloud cloudTemp = clouds[i];

		for (int j = 0; cloudTemp.cloudPieces[j].type > 0; j++)
		{
			cloudPiece cloudPieceTemp = cloudTemp.cloudPieces[j];

			vertex2f origin = { cloudTemp.origin.x + cloudPieceTemp.origin.x, cloudTemp.origin.y + cloudPieceTemp.origin.y };

			colour.a = cloudTemp.alpha;

			if (cloudPieceTemp.type == 1) //ellipse
				drawEllipse(origin, cloudPieceTemp.width, cloudPieceTemp.height, 30, colour, colour);

			if (cloudPieceTemp.type == 2)
				drawRectangle(origin, cloudPieceTemp.width, cloudPieceTemp.height, colour1);
		}
	}
}

void updateClouds(cloud *cloud)
{
	cloud->origin.x += cloud->speed;
	if (cloud->origin.x >= 1.5 && !isGettingDarker)
	{
		cloud->speed = randomFloat(0.005, 0.03);
		cloud->origin.x = randomFloat(-15, -2);
		cloud->origin.y = randomFloat(cloudMinHeight, cloudMaxHeight);
	}
	if (isGettingDarker)
	{
		cloud->speed += 0.00035;
	}
}

void createCloudPiece(int cloudNumber, int pieceNumber, float originX, float originY, int pieceType, float width, float height)
{
	cloudPiece tempPiece = clouds[cloudNumber].cloudPieces[pieceNumber];

	tempPiece.origin.x = originX;
	tempPiece.origin.y = originY;
	tempPiece.type = pieceType;
	tempPiece.width = width;
	tempPiece.height = height;

	clouds[cloudNumber].cloudPieces[pieceNumber] = tempPiece;
	//clouds[cloudNumber] = tempCloud;
}

//--Stars--//
void drawAllStars()
{
	for (int i = 0; i < MAXIMUM_STARS; i++)
	{
		drawStar(stars[i]);
	}
}

void drawStar(star star)
{
	vertex2f temp = star.origin;

	rgba colour = { 1,1,1, nightFilterAlpha };
	rgba colour2 = { 1,1,1,star.alpha * nightFilterAlpha };
	drawEllipse(temp, 0.01, 0.01, 15, colour, colour2); //Randomise star sise
	glEnd();
}

void updateStars()
{
	int isValidStar = 0;

	if (!isGettingDarker && !starsAreDirty)
	{
		starsAreDirty = 1;
	}

	if (starsAreDirty && isGettingDarker)
	{
		for (int i = 0; i < MAXIMUM_STARS; i++) //TODO: move to think and only call once a night
		{

			do
			{
				stars[i].origin.x = randomFloat(-1, 1);
				stars[i].origin.y = randomFloat(-.2, 1);
				stars[i].alpha = stars[i].origin.y * 0.5;

				vertex2f temp = { stars[i].origin.x,stars[i].origin.y };


				//Front Right
				if (temp.x > 0.6 && temp.y < 0.7);

				//Front Left
				else if (temp.x > -0.46 && temp.x < -.125 && temp.y < 0.32);

				//Middle Gap
				else if (temp.x > -0.3 && temp.x < 0.23 && temp.y < -0.1);

				//Back1
				else if (temp.x > -1 && temp.x < -0.55 && temp.y < 0.1);

				//Back2
				else if (temp.x > -0.064 && temp.x < 0.23 && temp.y < 0.2);

				//Back Right
				else if (temp.x > 0.4 && temp.x < 0.6 && temp.y < 0.15);


				else
					isValidStar = 1;

			} while (!isValidStar);

			isValidStar = 0;
		}

		starsAreDirty = 0;
	}
}


float randomFloat(float min, float max) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = max - min;
	float r = random * diff;
	return min + r;
}

rgb changecolourOvertime(rgb colour1, rgb colour2)
{
	rgb temp =
	{
		(colour2.r - colour1.r)*nightFilterAlpha + colour1.r,
		(colour2.g - colour1.g)*nightFilterAlpha + colour1.g,
		(colour2.b - colour1.b)*nightFilterAlpha + colour1.b
	};

	if (temp.r > colour1.r)
		temp.r = colour1.r;

	if (temp.g > colour1.g)
		temp.g = colour1.g;

	if (temp.b > colour1.b)
		temp.b = colour1.b;

	//
	if (temp.r < colour2.r)
		temp.r = colour2.r;

	if (temp.g < colour2.g)
		temp.g = colour2.g;

	if (temp.b < colour2.b)
		temp.b = colour2.b;

	return temp;
}

// -- Display Panels -- //
void printText(float x, float y, rgb colour, char text[])
{
	glColor3f(colour.r, colour.g, colour.b);
	glRasterPos2d(x, y);
	glutBitmapString(GLUT_BITMAP_8_BY_13, text);
}

void printHotkeys(float x, float y, rgb textColour)
{
	float ySpacing = 0.05;

	char hotkeyTextString1[] = "Hot Keys";
	printText(x, y, textColour, hotkeyTextString1);

	char hotkeyTextString2[] = "S - Toggle snow";
	printText(x, y - ySpacing, textColour, hotkeyTextString2);

	char hotkeyTextString3[] = "D - Toggle Info box";
	printText(x, y - ySpacing * 2, textColour, hotkeyTextString3);

	char hotkeyTextString4[] = "F - Toggle City Scape";
	printText(x, y - ySpacing * 3, textColour, hotkeyTextString4);
}

void showInfoPanel(float x, float y)
{
	float xMargin = x + 0.05;

		rgb colour1a = windowsIceBlue;
		rgb colour1b = { -maxNightDarkness, -maxNightDarkness, -maxNightDarkness };

		colour1a = changecolourOvertime(colour1a, colour1b);

	if (isInfoPanelVisable)
	{
		vertex2f bgPos = { x, y };
		rgba bgColour = { 1,1,1,0.5 };
		drawRectangleWithAlpha(bgPos, 0.45, 0.4, bgColour);

		char hotkeyTextString1[] = "---Info Box---";
		printText(x +0.08, y - 0.05, colour1a, hotkeyTextString1);

		printHotkeys(xMargin, y - 0.2, colour1a);

		displayAmountOfActiveParticles(xMargin, 0.8, colour1a);

	}
	else
	{
		rgba colosedColour = { 1,1,1, 0.3};

		vertex2f bgPos = { -1,1 };
		drawRectangleWithAlpha(bgPos, 0.56, 0.1, colosedColour);

		char openInfoBox[] = "Press D to open Info Panel";
		printText(-0.95, 0.95, colour1a, openInfoBox);
	}
}

void displayAmountOfActiveParticles(float x, float y, rgb colour)
{
	char particleString[40] = "particles: ";
	char activeParticleString[6];
	_itoa(snowActiveParticlesCounter, activeParticleString, 10);
	strcat(particleString, activeParticleString);
	strcat(particleString, " of 1000");

	glColor3f(colour.r, colour.g, colour.b);
	glRasterPos2f(x, y);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12, particleString);
}

void drawMountains()
{
	rgb baseColour1 = { 0.175, 0.570, 0.154 };
	rgb tipColour1 = { 0.125, 0.990, 0.0792 };
	vertex2f origin = { -1, -1 };
	vertex2f origin2 = { 0.5, -1 };
	glColor3f(baseColour1.r, baseColour1.g, baseColour1.b);
	//Left mountain
	glBegin(GL_POLYGON);

	//Leftvertex
	glVertex2f(origin.x, origin.y);

	//Mountain Tip
	glColor3f(tipColour1.r, tipColour1.g, tipColour1.b);
	glVertex2f(randomMountainPoint1.x, randomMountainPoint1.y);
	
	glColor3f(baseColour1.r, baseColour1.g, baseColour1.b);
	//right vertex
	glVertex2f(origin2.x, origin2.y);

	glEnd();


	glBegin(GL_POLYGON);

	glColor3f(1,1,1);

	float m = ((randomMountainPoint1.y - origin.y) / (randomMountainPoint1.x - origin.x));
	float c = -((randomMountainPoint1.x * m) - randomMountainPoint1.y);

	float xPoint = randomMountainPoint1.x - .4;
	float yPoint = (m * xPoint) + c;
	glVertex2f(xPoint, yPoint);

	glVertex2f(randomMountainPoint1.x, randomMountainPoint1.y);

	//---------
	m = ((origin2.y - randomMountainPoint1.y) / (origin2.x - randomMountainPoint1.x));
	c = -((randomMountainPoint1.x * m) - randomMountainPoint1.y);

	xPoint = randomMountainPoint1.x + .3;
	yPoint = (m * xPoint) + c;
	glVertex2f(xPoint, yPoint);

	glEnd();
}

void randmoiseMountainPoints()
{
	vertex2f temp1 = { randomFloat(-0.2, 0.), randomFloat(-0.2, 0.2) };
	vertex2f temp2 = { randomFloat(-0.2, 0.2), randomFloat(-0.2, 0.2) };
	vertex2f temp3 = { randomFloat(-0.2, 0.2), randomFloat(-0.2, 0.2) };

	randomMountainPoint1 = temp1;
	randomMountainPoint2 = temp2;
	randomMountainPoint3 = temp3;
}

void drawCrescentMoon(float x, float y, float step, float scale, float fullness) 
{
	glColor4f(0.9f, 0.95f, 0.9f, nightFilterAlpha);
	glLineWidth(4);
	glBegin(GL_LINE_STRIP);
	{
		glVertex2f(x, scale + y);
		float angle = step;
		while (angle < PI) {
			float sinAngle = sinf(angle);
			float cosAngle = cosf(angle);
			glVertex2f(scale*sinAngle + x, scale*cosAngle + y);
			glVertex2f(-fullness * scale*sinAngle + x, scale*cosAngle + y);
			angle += step;
		}
		glVertex2f(x, -scale + y);
	}
	glEnd();
}
