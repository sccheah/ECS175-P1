/*
 * Simple glut demo that can be used as a template for
 * other projects by Garrett Aldrich
 */

// DOES NOT READ FILE IN MAC FOR SOME REASON FIX LATER... WORKING IN LINUX ENV

#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>


#else //linux
#include <GL/gl.h>
#include <GL/glut.h>
#endif

//other includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

using namespace std;

/****set in main()****/
//the number of pixels in the grid
int grid_width;
int grid_height;

//the size of pixels sets the inital window height and width
//don't make the pixels too large or the screen size will be larger than
//your display size
float pixel_size;

/*Window information*/
int win_height;
int win_width;

void init();
void idle();
void display();
void draw_pix(int x, int y);
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();

bool DDA_draw = false;
bool plot_verticies_draw = true;

void plot_verticies();



class Point {
	double x;
	double y;
	
public:
	void set_x(double new_x) {x = new_x;}
	void set_y(double new_y) {y = new_y;}
	int get_x() {return x;}
	int get_y() {return y;}
};

class Polygon {
public:
	int numberOfPoints;
	Point *points;
};

Polygon *polygons = NULL;
int numberOfPolygons = 0;
int user_input;
int num_of_polygons_loop = 0;


Polygon* read_file(Polygon *polygons, int &numberOfPolygons)
{
	ifstream file("data.txt");
	file >> numberOfPolygons;
	
	polygons = new Polygon [numberOfPolygons];
	
	for (int i = 0; i < numberOfPolygons; i++)
	{

		file >> polygons[i].numberOfPoints;
		polygons[i].points = new Point [polygons[i].numberOfPoints];
		
		for (int j = 0; j < polygons[i].numberOfPoints; j++)
		{
			double x, y;
			
			file >> x;
			file >> y;
			polygons[i].points[j].set_x(x);
			polygons[i].points[j].set_y(y);
		}
		
	}
	
	file.close();
	return polygons;
}

//inline int round (const double a) {return int (a + 0.5);}

int main(int argc, char **argv)
{  
	
	polygons = read_file(polygons, numberOfPolygons);


	if (polygons == NULL)
	{
		cout << "No polygons read in file. Exitting program." << endl;
		return 0;
	}
	
	
	//the number of pixels in the grid
	grid_width = 100;
	grid_height = 100;
	
	//the size of pixels sets the inital window height and width
	//don't make the pixels too large or the screen size will be larger than
	//your display size
	pixel_size = 5;
	
	/*Window information*/
	win_height = grid_height*pixel_size;
	win_width = grid_width*pixel_size;
	
	/*Set up glut functions*/
	/** See https://www.opengl.org/resources/libraries/glut/spec3/spec3.html ***/
	
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	/*initialize variables, allocate memory, create buffers, etc. */
	//create window of size (win_width x win_height
	glutInitWindowSize(win_width,win_height);
	//windown title is "glut demo"
	glutCreateWindow("glut demo");
	
	/*defined glut callback functions*/
	glutDisplayFunc(display); //rendering calls here

	glutReshapeFunc(reshape); //update GL on window size change
	glutMouseFunc(mouse);     //mouse button events
	glutMotionFunc(motion);   //mouse movement events
	glutKeyboardFunc(key);    //Keyboard events
	glutIdleFunc(idle);       //Function called while program is sitting "idle"
	
	//initialize opengl variables
	init();
	//start glut event loop
	glutMainLoop();

	
	return 0;
}

/*initialize gl stufff*/
void init()
{
	//set clear color (Default background to white)
	glClearColor(1.0,1.0,1.0,1.0);
	//checks for OpenGL errors
	check();
}

//called repeatedly when glut isn't doing anything else
void idle()
{
	//redraw the scene over and over again
	glutPostRedisplay();
}

//plot verticies
void plot_verticies()
{
	int num_of_polygons = 0;

	while (num_of_polygons < numberOfPolygons)
	{
		for (int k = 0; k < polygons[num_of_polygons].numberOfPoints; k++)
		{
			draw_pix((int)(polygons[num_of_polygons].points[k].get_x()), (int)(polygons[num_of_polygons].points[k].get_y()));
		}
        
        num_of_polygons++;
	}
}

//DDA Line Algorithm
void draw_DDA_line(int x0, int y0, int xEnd, int yEnd)
{
	int dx = xEnd - x0, dy = yEnd - y0, steps, k;
	double xIncrement, yIncrement, x = x0, y = y0;

	if ((double)(fabs(dx)) > (double)(fabs(dy)))
	{
		steps = (double)(fabs(dx));
	}
	else
	{
		steps = (double)(fabs(dy));
	}

	xIncrement = (double)(dx) / (double)(steps);
	yIncrement = (double)(dy) / (double)(steps);

	draw_pix(round(x), round(y));
	for (k = 0; k < steps; k++)
	{
		x += xIncrement;
		y += yIncrement;
		draw_pix(round(x), round(y));
	}
}


//this is where we render the screen
void display()
{
	//clears the screen
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	//clears the opengl Modelview transformation matrix
	glLoadIdentity();
    
    if (plot_verticies_draw)
		plot_verticies();
	if (DDA_draw)
	{
		num_of_polygons_loop = 0;

		while (num_of_polygons_loop < numberOfPolygons)
		{
			for (int k = 1; k < polygons[num_of_polygons_loop].numberOfPoints; k++)
			{
				draw_DDA_line((int)(polygons[num_of_polygons_loop].points[k - 1].get_x()), (int)(polygons[num_of_polygons_loop].points[k].get_x()), (int)(polygons[num_of_polygons_loop].points[k - 1].get_y()), (int)(polygons[num_of_polygons_loop].points[k].get_y()));
			}
		       
			draw_DDA_line((int)(polygons[num_of_polygons_loop].points[0].get_x()), (int)(polygons[num_of_polygons_loop].points[polygons[num_of_polygons_loop].numberOfPoints - 1].get_x()), (int)(polygons[num_of_polygons_loop].points[0].get_y()), (int)(polygons[num_of_polygons_loop].points[polygons[num_of_polygons_loop].numberOfPoints - 1].get_y()));

		       num_of_polygons_loop++;
		}
	}

	//blits the current opengl framebuffer on the screen
	glutSwapBuffers();
	//checks for opengl errors
	check();
}



//Draws a single "pixel" given the current grid size
//don't change anything in this for project 1
void draw_pix(int x, int y){
	glBegin(GL_POINTS);
	glColor3f(.2,.2,1.0);
	glVertex3f(x+.5,y+.5,0);
	glEnd();
}

/*Gets called when display size changes, including initial craetion of the display*/
void reshape(int width, int height)
{
	/*set up projection matrix to define the view port*/
	//update the ne window width and height
	win_width = width;
	win_height = height;
	
	//creates a rendering area across the window
	glViewport(0,0,width,height);
	// up an orthogonal projection matrix so that
	// the pixel space is mapped to the grid space
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,grid_width,0,grid_height,-10,10);
	
	//clear the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//set pixel size based on width, if the aspect ratio
	//changes this hack won't work as well
	pixel_size = width/(float)grid_width;
	
	//set pixel size relative to the grid cell size
	glPointSize(pixel_size);
	//check for opengl errors
	check();
}

//gets called when a key is pressed on the keyboard
void key(unsigned char ch, int x, int y)
{
	switch(ch)
	{
		case '0':
			cout << "Exitting." << endl;
			exit(0);
			break;
		case '1':
			cout << "DDA Line Drawing." << endl;
			DDA_draw = true;
			break;

		default:
			//prints out which key the user hit
			printf("User hit the \"%c\" key\n",ch);
			break;
	}
	//redraw the scene after keyboard input
	glutPostRedisplay();
}


//gets called when a mouse button is pressed
void mouse(int button, int state, int x, int y)
{
	//print the pixel location, and the grid location
	printf ("MOUSE AT PIXEL: %d %d, GRID: %d %d\n",x,y,(int)(x/pixel_size),(int)((win_height-y)/pixel_size));
	switch(button)
	{
		case GLUT_LEFT_BUTTON: //left button
			printf("LEFT ");
			break;
		case GLUT_RIGHT_BUTTON: //right button
			printf("RIGHT ");
		default:
			printf("UNKNOWN "); //any other mouse button
			break;
	}
	if(state !=GLUT_DOWN)  //button released
		printf("BUTTON UP\n");
	else
		printf("BUTTON DOWN\n");  //button clicked
	
	//redraw the scene after mouse click
	glutPostRedisplay();
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
	//redraw the scene after mouse movement
	glutPostRedisplay();
}

//checks for any opengl errors in the previous calls and
//outputs if they are present
void check()
{
	GLenum err = glGetError();
	if(err != GL_NO_ERROR)
	{
		printf("GLERROR: There was an error %s\n",gluErrorString(err) );
		exit(1);
	}
}


	// in main after read_file function to check polygon points
	/*cout << numberOfPolygons << endl;
	cout << polygons[0].numberOfPoints << endl;
	for (int i = 0; i < numberOfPolygons; i++)
	{
		for (int j = 0; j < polygons[i].numberOfPoints; j++)
		{
			//cout << "here" << endl;
			//cout << "x: ";
			cout << polygons[i].points[j].get_x() << endl;
			//cout << "y: ";
			cout << polygons[i].points[j].get_y() << endl;

		}
	}*/
