/*
 * Simple glut demo that can be used as a template for
 * other projects by Garrett Aldrich
 */

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
#include <list>
#include <vector>

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
void display_menu();

bool DDA_draw = false;
bool plot_verticies_draw = true;
bool Bres_draw = false;
bool scanline_draw = false;
bool clip_window = false;
bool rotate_draw = false;

bool is_button_down = false;
bool is_button_up = false;


void plot_verticies();


class Point 
{
	double x;
	double y;
	
public:
	void set_x(double new_x) {x = new_x;}
	void set_y(double new_y) {y = new_y;}
	int get_x() {return x;}
	int get_y() {return y;}
};

class Polygon 
{
public:
	int numberOfPoints;
	Point *points;
};


Polygon *polygons = NULL;
int numberOfPolygons = 0;
int user_input;
int num_of_polygons_loop = 0;

int **frame_buffer;
int **edge_buffer;
list<Point> scanline_points;
list<Point> ignore_points;
list<Point> consecutive_points;
Point button_down;
Point button_up;


Point make_point(int x, int y)
{
	Point pt;
	pt.set_x(x);
	pt.set_y(y);
	return pt;
}


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
			frame_buffer[(int)x][(int)y] = 1;
			edge_buffer[(int)x][(int)y] = 1;
		}
		
	}
	
	file.close();
	return polygons;
}

void find_ignore_points()
{
	for (int i = 0; i < numberOfPolygons; i++)
	{
		for (int j = 1; j < polygons[i].numberOfPoints; j++)
		{
			//check for horizontal lines
			if (polygons[i].points[j - 1].get_y() == polygons[i].points[j].get_y())
			{
				for (int k = (int)(polygons[i].points[j - 1].get_x()); k <= (int)(polygons[i].points[j].get_x()); k++)
				{
					ignore_points.push_back(make_point(k, (int)(polygons[i].points[j - 1].get_y())));
				}
			}
		}

		// check if starting vertex to end vertex is a horizontal line
		if (polygons[i].points[0].get_y() == polygons[i].points[polygons[i].numberOfPoints - 1].get_y())
		{
			for (int k = (int)(polygons[i].points[0].get_x()); k <= (int)(polygons[i].points[polygons[i].numberOfPoints - 1].get_x()); k++)
			{
				ignore_points.push_back(make_point(k, (int)(polygons[i].points[0].get_y())));
			}
		}

		//check for local max/min points
		for (int j = 2; j < polygons[i].numberOfPoints; j++)
		{
			// local min check
			if ((polygons[i].points[j - 1].get_y() < polygons[i].points[j - 2].get_y()) && (polygons[i].points[j - 1].get_y() < polygons[i].points[j].get_y()))
			{
				ignore_points.push_back(make_point((int)(polygons[i].points[j - 1].get_x()), (int)(polygons[i].points[j - 1].get_y())));
			}

			// local max check
			if ((polygons[i].points[j - 1].get_y() > polygons[i].points[j - 2].get_y()) && (polygons[i].points[j - 1].get_y() > polygons[i].points[j].get_y()))
			{
				ignore_points.push_back(make_point((int)(polygons[i].points[j - 1].get_x()), (int)(polygons[i].points[j - 1].get_y())));
			}
		}

		// check if start and end verticies are local max/min points
		if (polygons[i].numberOfPoints > 2)
		{
			// starting pt local min check
			if ((polygons[i].points[0].get_y() < polygons[i].points[1].get_y()) && (polygons[i].points[0].get_y() < polygons[i].points[polygons[i].numberOfPoints - 1].get_y()))
			{
				ignore_points.push_back(make_point((int)(polygons[i].points[0].get_x()), (int)(polygons[i].points[0].get_y())));
			}

			// starting pt local max check
			if ((polygons[i].points[0].get_y() > polygons[i].points[1].get_y()) && (polygons[i].points[0].get_y() > polygons[i].points[polygons[i].numberOfPoints - 1].get_y()))
			{
				ignore_points.push_back(make_point((int)(polygons[i].points[0].get_x()), (int)(polygons[i].points[0].get_y())));
			}

			// end pt local min check
			if ((polygons[i].points[polygons[i].numberOfPoints - 1].get_y() < polygons[i].points[0].get_y()) && (polygons[i].points[polygons[i].numberOfPoints - 1].get_y() < polygons[i].points[polygons[i].numberOfPoints - 2].get_y()))
			{
				ignore_points.push_back(make_point((int)(polygons[i].points[polygons[i].numberOfPoints - 1].get_x()), (int)(polygons[i].points[polygons[i].numberOfPoints - 1].get_y())));
			}

			// end pt local max check
			if ((polygons[i].points[polygons[i].numberOfPoints - 1].get_y() > polygons[i].points[0].get_y()) && (polygons[i].points[polygons[i].numberOfPoints - 1].get_y() > polygons[i].points[polygons[i].numberOfPoints - 2].get_y()))
			{
				ignore_points.push_back(make_point((int)(polygons[i].points[polygons[i].numberOfPoints - 1].get_x()), (int)(polygons[i].points[polygons[i].numberOfPoints - 1].get_y())));
			}
		}
	}
}

void clear_frame_buffer()
{
	for (int i = 0; i < grid_width; i++)
	{
		for (int j = 0; j < grid_height; j++)
		{
			frame_buffer[i][j] = 0;
		}
	}
}

void clear_edge_buffer()
{
	for (int i = 0; i < grid_width; i++)
	{
		for (int j = 0; j < grid_height; j++)
		{
			edge_buffer[i][j] = 0;
		}
	}
}


int main(int argc, char **argv)
{  
	display_menu();

	//the number of pixels in the grid
	grid_width = 100;
	grid_height = 100;

	button_down.set_x(0);
	button_down.set_y(0);
	button_up.set_x(grid_width - 1);
	button_up.set_y(grid_height - 1);

	frame_buffer = new int* [grid_width];
	edge_buffer = new int* [grid_width];
	for (int i = 0; i < grid_width; i++)
	{
		frame_buffer[i] = new int [grid_height];
		edge_buffer[i] = new int [grid_height];
	}


	polygons = read_file(polygons, numberOfPolygons);


	if (polygons == NULL)
	{
		cout << "No polygons read in file. Exitting program." << endl;
		return 0;
	}

	find_ignore_points();

	
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
	glutCreateWindow("Polygons");
	
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

void identify_consecutive_points()
{
	for (int j = 0; j < grid_height; j++)
	{
		for (int i = 0; i < grid_width - 1; i++)
		{
			if ((edge_buffer[i][j] == 1) && (edge_buffer[i + 1][j] == 1))
			{
				consecutive_points.push_back(make_point(i, j));
			}

			if (i > 0)
			{
				if ((edge_buffer[i][j] == 1) && (edge_buffer[i + 1][j] == 0))
				{
					if ((edge_buffer[i][j] == 1) && (edge_buffer[i - 1][j] == 1))
					{
						consecutive_points.push_back(make_point(i, j));
					}
				}
			}
		}
	}
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
	frame_buffer[(int)round(x)][(int)round(y)] = 1;
	edge_buffer[(int)round(x)][(int)round(y)] = 1;


	for (k = 0; k < steps; k++)
	{
		x += xIncrement;
		y += yIncrement;
		draw_pix(round(x), round(y));
		frame_buffer[(int)round(x)][(int)round(y)] = 1;
		edge_buffer[(int)round(x)][(int)round(y)] = 1;
		
	}
}

void draw_Bres_line(int x0, int y0, int xEnd, int yEnd)
{

	int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
	dx = xEnd - x0;
	dy = yEnd - y0;
	dx1 = fabs(dx);
	dy1 = fabs(dy);
	px = 2 * dy1 - dx1;
	py = 2 * dx1 - dy1;

	int temp_y = y0;

	//draw vertical lines
	if (x0 == xEnd)
	{
		if (temp_y < yEnd)
		{
			while (temp_y < yEnd)
			{
				temp_y++;
				draw_pix(x0, temp_y);
				frame_buffer[x0][temp_y] = 1;
				edge_buffer[x0][temp_y] = 1;
				
			}
		}
		if (temp_y > yEnd)
		{
			while (temp_y > yEnd)
			{
				temp_y--;
				draw_pix(x0, temp_y);
				frame_buffer[x0][temp_y] = 1;
				edge_buffer[x0][temp_y] = 1;
			}
		}
	}

	if (dy1 <= dx1)
	{
		if (dx >= 0)
		{
			x = x0; 
			y = y0;
			xe = xEnd;
		}
		else
		{
			x = xEnd;
			y = yEnd;
			xe = x0;
		}

		draw_pix(x, y);
		frame_buffer[x][y] = 1;
		edge_buffer[x][y] = 1;
		

		for (i = 0; x < xe; i++)
		{
			x++;
			if (px < 0)
				px = px + 2 * dy1;
			else
			{
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
					y++;
				else
					y--;

				px = px + 2 * (dy1 - dx1);
			}

			draw_pix(x, y);
			frame_buffer[x][y] = 1;
			edge_buffer[x][y] = 1;
		}
	}
	else
	{
		if (dy >= 0)
		{
			x = x0;
			y = y0;
			ye = yEnd;
		}
		else
		{
			x = xEnd;
			y = yEnd;
			ye = yEnd;
		}

		draw_pix(x, y);
		frame_buffer[x][y] = 1;
		edge_buffer[x][y] = 1;
		
		for (i = 0; y < ye; i++)
		{
			y++;

			if (py <= 0)
				py = py + 2 * dx1;
			else
			{
				if ((dx < 0) && (dy < 0) || (dx > 0) && dy > 0)
					x++;
				else
					x--;

				py = py + 2 * (dx1 - dy1);
			}
			draw_pix(x, y);
			frame_buffer[x][y] = 1;
			edge_buffer[x][y] = 1;
			
		}
	}

}



bool check_ignore_point(int x, int y)
{
	for (list<Point>::iterator it = ignore_points.begin(); it != ignore_points.end(); it++)
	{
		if ((it->get_x() == x) && (it->get_y() == y))
		{
			//cout << "(" << it->get_x() << ", " << it->get_y() << ")" << endl;
			return true;
		}
	}

	return false;
}

bool check_consecutive_points(int x, int y)
{
	for (list<Point>::iterator it = consecutive_points.begin(); it != consecutive_points.end(); it++)
	{
		if ((it->get_x() == x) && (it->get_y() == y))
		{
			//cout << "(" << it->get_x() << ", " << it->get_y() << ")" << endl;
			return true;
		}
	}

	return false;
}

void get_scanline_points()
{
	bool draw_scanline = false;
	int ctr = 0;
	int check_value = 0;

	// left to right scanline
	for (int j = 0; j < grid_height; j++)
	{
		ctr = 0;
		for (int i = 0; i < grid_width; i++)
		{

			if (check_consecutive_points(i, j))
			{
				
				ctr++;

				if ((ctr % 2) == 0)
				{
					draw_scanline = true;
				}

				if ((ctr % 2) == 1)
				{
					draw_scanline = false;
				}

				for (int k = i; k < grid_width; k++)
				{
					if (edge_buffer[k][j] != 1)
					{
						i = k;
						continue;
					}
										
					if ((check_consecutive_points(k, j) == false) && check_ignore_point(k, j) == false)
					{
						i = k;
						continue;
					}

				}

			}

			if (check_ignore_point(i, j))
			{
				if (check_consecutive_points(i, j) == false)
				{
					continue;
				}

			}

			if (edge_buffer[i][j] == 1)
			{

				draw_scanline = !draw_scanline;
			}

			if (draw_scanline)
			{
				scanline_points.push_back(make_point(i, j));
				frame_buffer[i][j] = 1;
			}

		}
	}

	draw_scanline = false;
	for (int j = 0; j < grid_height; j++)
	{
		ctr = 0;
		for (int i = grid_width - 1; i >= 0; i--)
		{

			if (check_consecutive_points(i, j))
			{
				
				ctr++;

				if ((ctr % 2) == 0)
				{
					draw_scanline = true;
				}

				if ((ctr % 2) == 1)
				{
					draw_scanline = false;
				}

				for (int k = i; k >= 0; k--)
				{
					if (edge_buffer[k][j] != 1)
					{
						i = k;
						continue;
					}
										
					if ((check_consecutive_points(k, j) == false) && check_ignore_point(k, j) == false)
					{
						i = k;
						continue;
					}

				}

			}

			if (check_ignore_point(i, j))
			{
				if (check_consecutive_points(i, j) == false)
				{
					continue;
				}

			}

			if (edge_buffer[i][j] == 1)
			{

				draw_scanline = !draw_scanline;
			}

			if (draw_scanline)
			{
				scanline_points.push_back(make_point(i, j));
				frame_buffer[i][j] = 1;
			}

		}
	}
	
	return;
}

void print_edge_buffer()
{
	for(int j = 0; j < grid_height; j++)
	{
		for(int i = 0; i < grid_width; i++)
		{
			if (edge_buffer[i][j] == 1)
			{
				cout << "(" << i << ", " << j << ")" << endl;
			}
		}
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
				draw_DDA_line(polygons[num_of_polygons_loop].points[k-1].get_x(), polygons[num_of_polygons_loop].points[k-1].get_y(), polygons[num_of_polygons_loop].points[k].get_x(), polygons[num_of_polygons_loop].points[k].get_y());
			}
		       
			draw_DDA_line(polygons[num_of_polygons_loop].points[0].get_x(), polygons[num_of_polygons_loop].points[0].get_y(), polygons[num_of_polygons_loop].points[polygons[num_of_polygons_loop].numberOfPoints - 1].get_x(), polygons[num_of_polygons_loop].points[polygons[num_of_polygons_loop].numberOfPoints - 1].get_y());

		       num_of_polygons_loop++;
		}

		//print_edge_buffer();
	}

	if (Bres_draw)
	{
		num_of_polygons_loop = 0;

		while (num_of_polygons_loop < numberOfPolygons)
		{
			for (int k = 1; k < polygons[num_of_polygons_loop].numberOfPoints; k++)
			{
				draw_Bres_line(polygons[num_of_polygons_loop].points[k-1].get_x(), polygons[num_of_polygons_loop].points[k-1].get_y(), polygons[num_of_polygons_loop].points[k].get_x(), polygons[num_of_polygons_loop].points[k].get_y());
			}
		       
			draw_Bres_line(polygons[num_of_polygons_loop].points[0].get_x(), polygons[num_of_polygons_loop].points[0].get_y(), polygons[num_of_polygons_loop].points[polygons[num_of_polygons_loop].numberOfPoints - 1].get_x(), polygons[num_of_polygons_loop].points[polygons[num_of_polygons_loop].numberOfPoints - 1].get_y());

		       num_of_polygons_loop++;
		}

		//print_edge_buffer();
	}

	if (scanline_draw)
	{
		for (list<Point>::iterator it = scanline_points.begin(); it != scanline_points.end(); it++)
		{
			draw_pix(it->get_x(), it->get_y());
		}
	}

	if (clip_window)
	{
		for (int i = 0; i < grid_width; i++)
		{
			for (int j = 0; j < grid_height; j++)
			{
					glBegin(GL_POINTS);
					glColor3f(1.0,1.0,1.0);
					glVertex3f(i+.5,j+.5,0);
					glEnd();
			}
		}

		if (button_down.get_x() <= button_up.get_x())
		{
			if (button_down.get_y() <= button_up.get_y())
			{
				for (int j = button_down.get_y(); j <= button_up.get_y(); j++)
				{
					for (int i = button_down.get_x(); i <= button_up.get_x(); i++)
					{
						if (frame_buffer[i][j] == 1)
						{
							draw_pix(i, j);
						}
					}
				}
			}

			if (button_down.get_y() > button_up.get_y())
			{
				for (int j = button_up.get_y(); j <= button_down.get_y(); j++)
				{
					for (int i = button_down.get_x(); i <= button_up.get_x(); i++)
					{
						if (frame_buffer[i][j] == 1)
						{
							draw_pix(i, j);
						}
					}
				}
			}
		}

		if (button_down.get_x() > button_up.get_x())
		{
			if (button_down.get_y() <= button_up.get_y())
			{
				for (int j = button_down.get_y(); j <= button_up.get_y(); j++)
				{
					for (int i = button_up.get_x(); i <= button_down.get_x(); i++)
					{
						if (frame_buffer[i][j] == 1)
						{
							draw_pix(i, j);
						}
					}
				}
			}

			if (button_down.get_y() > button_up.get_y())
			{
				for (int j = button_up.get_y(); j <= button_down.get_y(); j++)
				{
					for (int i = button_up.get_x(); i <= button_down.get_x(); i++)
					{
						if (frame_buffer[i][j] == 1)
						{
							draw_pix(i, j);
						}
					}
				}
			}
		}
	}


	//blits the current opengl framebuffer on the screen
	glutSwapBuffers();
	//checks for opengl errors
	check();
}

void translatePolygon(int selected_polygon, int tx, int ty)
{
	for (int k = 0; k < polygons[selected_polygon].numberOfPoints; k++)
	{
		polygons[selected_polygon].points[k].set_x(polygons[selected_polygon].points[k].get_x() + tx);
		polygons[selected_polygon].points[k].set_y(polygons[selected_polygon].points[k].get_y() + ty);
	}
	
	scanline_points.clear();
	get_scanline_points();
}

void rotatePolygon(int selected_polygon, double theta)
{
	double pivot_x = 0.0;
	double pivot_y = 0.0;

	double x;
	double y;

	for (int k = 0; k < polygons[selected_polygon].numberOfPoints; k++)
	{
		pivot_x += polygons[selected_polygon].points[k].get_x();
		pivot_y += polygons[selected_polygon].points[k].get_y();
	}

	pivot_x = pivot_x / polygons[selected_polygon].numberOfPoints;
	pivot_y = pivot_y / polygons[selected_polygon].numberOfPoints;


	for (int k = 0; k < polygons[selected_polygon].numberOfPoints; k++)
	{
		x = pivot_x + ((((double)(polygons[selected_polygon].points[k].get_x()) - pivot_x) * cos(theta)) - (((double)(polygons[selected_polygon].points[k].get_y()) - pivot_y) * sin(theta)));
		y = pivot_y + ((((double)(polygons[selected_polygon].points[k].get_x()) - pivot_x) * sin(theta)) + (((double)(polygons[selected_polygon].points[k].get_y()) - pivot_y) * cos(theta)));
	
		polygons[selected_polygon].points[k].set_x(x);
		polygons[selected_polygon].points[k].set_y(y);
	}
}

void scalePolygon(int selected_polygon, double scale_factor)
{
	int k;
	double sx = scale_factor, sy = scale_factor;
	int fixed_x, fixed_y;
	double new_x, new_y;

	for (int k = 0; k < polygons[selected_polygon].numberOfPoints; k++)
	{
		fixed_x += polygons[selected_polygon].points[k].get_x();
		fixed_y += polygons[selected_polygon].points[k].get_y();
	}

	fixed_x = fixed_x / polygons[selected_polygon].numberOfPoints;
	fixed_y = fixed_y / polygons[selected_polygon].numberOfPoints;

	for (k = 0; k < polygons[selected_polygon].numberOfPoints; k++)
	{
		new_x = polygons[selected_polygon].points[k].get_x() * sx + polygons[selected_polygon].points[0].get_x() * (1 - sx);
		new_y = polygons[selected_polygon].points[k].get_y() * sy + polygons[selected_polygon].points[0].get_y() * (1 - sy);

		polygons[selected_polygon].points[k].set_x(new_x);
		polygons[selected_polygon].points[k].set_y(new_y);
	}

}

void output_to_file()
{
	fstream file;
	file.open("output.txt", fstream::out);
	file << numberOfPolygons;
	file << endl;

	for (int i = 0; i < numberOfPolygons; i++)
	{
		file << endl;
		file << polygons[i].numberOfPoints;

		for (int j = 0; j < polygons[i].numberOfPoints; j++)
		{
			file << endl;
			file << polygons[i].points[j].get_x() << " " << polygons[i].points[j].get_y();
		}
		file << endl;
	}
	file.close();
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

void display_menu()
{
	cout << "Menu" << endl;
	cout << "1. DDA Line Drawing" << endl;
	cout << "2. Bresenham Line Drawing" << endl;
	cout << "3. Scanline Polygon Filling" << endl;
	cout << "4. Translate Polygon" << endl;
	cout << "5. Rotate Polygon" << endl;
	cout << "6. Scale Polygon" << endl;
	cout << "7. Clip Window" << endl;
	cout << "8. Change Viewport by coordinates" << endl;
	cout << "9. Resetting" << endl;
	cout << "0. Exitting and outputting to output.txt" << endl;
	cout << endl;

}

//gets called when a key is pressed on the keyboard
void key(unsigned char ch, int x, int y)
{
	switch(ch)
	{
		case '0':
			cout << "Exitting." << endl;
			cout << endl;
			cout << "Outputting to \"output.txt\"..." << endl;
			output_to_file();
			exit(0);
			break;
		case '1':
		{
			cout << "DDA Line Drawing." << endl;
			cout << endl;
			scanline_draw = false;
			Bres_draw = false;
			clear_edge_buffer();
			clear_frame_buffer();
			scanline_points.clear();
			DDA_draw = true;
			display_menu();
			break;
		}
		case '2':
		{
			cout << "Bresenham Line Drawing." << endl;
			cout << endl;
			scanline_draw = false;
			DDA_draw = false;
			clear_edge_buffer();
			clear_frame_buffer();
			scanline_points.clear();
			Bres_draw = true;
			display_menu();
			break;
		}
		case '3':
		{
			cout << "Scanline Polygon Filling Algorithm" << endl;
			cout << endl;
			scanline_points.clear();
			consecutive_points.clear();
			identify_consecutive_points();
			get_scanline_points();
			scanline_draw = true;
			display_menu();
			break;
		}
		case '4':
		{
			int selected_polygon, tx, ty;
			cout << "Translating Polygon" << endl;
			cout << endl;
			cout << "Enter selected polygon [0-" << numberOfPolygons - 1 << "]: " << endl;
			cin >> selected_polygon;

			if ((selected_polygon < 0) || (selected_polygon > (numberOfPolygons - 1)))
			{
				cout << "Invalid Polygon Selection" << endl;
				break;
			}

			cout << "Enter x translation value: ";
			cin >> tx;
			cout << "Enter y translation value: ";
			cin >> ty;

			clear_frame_buffer();
			clear_edge_buffer();

			translatePolygon(selected_polygon, tx, ty);
			display_menu();
			break;
		}
		case '5':
		{
			double pi = 3.1415926535897;
			double angle = 0;
			int selected_polygon;
			cout << "Rotating polygon" << endl;
			cout << endl;
			cout << "Enter selected polygon [0-" << numberOfPolygons - 1 << "]: " << endl;
			cin >> selected_polygon;

			if ((selected_polygon < 0) || (selected_polygon > (numberOfPolygons - 1)))
			{
				cout << "Invalid Polygon Selection" << endl;
				break;
			}

			cout << "Enter angle: ";
			cin >> angle;
			clear_frame_buffer();
			clear_edge_buffer();
			rotatePolygon(selected_polygon, (angle / 180) * pi);
			rotate_draw = true;
			display_menu();
			break;
		}
		case '6':
		{
			double scale_factor = 1;
			int selected_polygon;
			cout << "Scaling Polygon" << endl;
			cout << endl;
			cout << "Enter selected polygon [0-" << numberOfPolygons - 1 << "]: " << endl;
			cin >> selected_polygon;

			if ((selected_polygon < 0) || (selected_polygon > (numberOfPolygons - 1)))
			{
				cout << "Invalid Polygon Selection" << endl;
				break;
			}

			cout << "Enter scale factor: ";
			cin >> scale_factor;
			clear_frame_buffer();
			clear_edge_buffer();
			scalePolygon(selected_polygon, scale_factor);
			display_menu();
			break;
		}
		case '7':
		{
			cout << "Clipping Window" << endl;
			cout << endl;
			clip_window = true;
			display_menu();			
			break;
		}
		case '8':
		{
			int x1, y1, x2, y2;
			cout << "Change viewport" << endl;
			cout << endl;
			cout << "Enter first point x coordinate: ";
			cin >> x1;
			cout << "Enter first point y coordinate: ";
			cin >> y1;
			cout << "Enter second point x coordinate: ";
			cin >> x2;
			cout << "Enter second point y coordinate: ";
			cin >> y2;

			button_down.set_x(x1);
			button_down.set_y(y1);
			button_up.set_x(x2);
			button_up.set_y(y2);
			display_menu();
			break;
		}

		case '9':
		{
			cout << "Resetting" << endl;
			cout << endl;
			DDA_draw = false;
			Bres_draw = false;
			scanline_draw = false;
			clip_window = false;
			rotate_draw = false;


			button_down.set_x(0);
			button_down.set_y(0);
			button_up.set_x(grid_width - 1);
			button_up.set_y(grid_height - 1);
			display_menu();
			break;
		}

		default:
			//prints out which key the user hit
			printf("User hit the \"%c\" key\n",ch);
			display_menu();
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
	{
		printf("BUTTON UP\n");
		is_button_up = true;
		button_up.set_x((int)(x/pixel_size));
		button_up.set_y((int)((win_height-y)/pixel_size));
	}
	else
	{
		printf("BUTTON DOWN\n");  //button clicked
		is_button_down = true;
		button_down.set_x((int)(x/pixel_size));
		button_down.set_y((int)((win_height-y)/pixel_size));
	}
	
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

