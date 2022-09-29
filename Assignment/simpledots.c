/************************************************************************************

File: 			points.c

Description:	A complete OpenGL program to draw three dots on the screen.

Author:			Edward Angel (modified by Stephen Brooks and Jacqui Whalley)

*************************************************************************************/


/* include the library header files */
#include <freeglut.h>




/************************************************************************

Function:		initializeGL

Description:	Initializes the OpenGL rendering context for display.

*************************************************************************/
void initializeGL(void)
{
	// set background color to be black
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// set the drawing to be blue
	glColor3f(0.0f, 0.0f, 1.0f);

	// set the dot size to be 4 by 4 pixels
	glPointSize(4.0);

	// set to projection mode
	glMatrixMode(GL_PROJECTION);

	// load the identity matrix
	glLoadIdentity();

	// set window mode to 2D orthographica and set the window size 
	gluOrtho2D(0.0, 640.0, 0.0, 480.0);
}


/************************************************************************

Function:		display

Description:	Displays 3 blue dots on a white background.

*************************************************************************/
void display(void)
{
	// clear the screen 
	glClear(GL_COLOR_BUFFER_BIT);
	
	// draw three points
	glBegin(GL_POINTS);
	glVertex2i(100, 50);
	glVertex2i(100, 130);
	glVertex2i(150, 130);
	glEnd();
	// send all output to display 
	glFlush();
}


/************************************************************************

Function:		main

Description:	Sets up the openGL rendering context and the windowing
system, then begins the display loop.

*************************************************************************/
void main(int argc, char** argv)
{
	// initialize the toolkit
	glutInit(&argc, argv);
	// set display mode
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	// set window size
	glutInitWindowSize(640, 480);
	// set window position on screen
	glutInitWindowPosition(100, 150);
	// open the screen window
	glutCreateWindow("my first attempt");
	// register redraw function
	glutDisplayFunc(display);
	//initialize the rendering context
	initializeGL();
	// go into a perpetual loop
	glutMainLoop();
}
