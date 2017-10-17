Menu
1. DDA Line Drawing
2. Bresenham Line Drawing
3. Scanline Polygon Filling
4. Translate Polygon
5. Rotate Polygon
6. Scale Polygon
7. Clip Window
8. Change Viewport by coordinates
9. Resetting
0. Exitting and outputting to output.txt

I programmed this in Ubuntu. It will seg fault if any shape goes out of the window. Scanline is a little buggy. Rotation is not precise (if I rotate a square by 90 degrees, it will be slightly off point - I think it is a rounding issue).

1. Using DDA Line Drawing
	Press "1" in the "Polygon" window.

2. Using Bresenham Line Drawing
	Press "2" in the "Polygon" window.

3. Scanline Polygon Filling 
	Press "3" in the "Polygon" window.

4. Translate Polygon
	Press "4" in the "Polygon" window.

	In the CONSOLE window:
	Enter from [0 - (number_of_polygons - 1)] to select the polygon.
	Type the "x" transition value, then press "ENTER".
	Type the "y" transition value, then press "ENTER".

5. Rotate Polygon
	Press "5" in the "Polygon" window.

	In the CONSOLE window:
	Enter from [0 - (number_of_polygons - 1)] to select the polygon.
	Type the "angle" value, then press "ENTER".

6. Scale Polygon
	Press "6" in the "Polygon" window.

	In the CONSOLE window:
	Enter from [0 - (number_of_polygons - 1)] to select the polygon.
	Type the "Scale Factor" value, then press "ENTER".

7. Clip Window
	Press "7" in the "Polygon" window.

	Using the mouse in the "Polygon" window:
	"BUTTON DOWN" sets the first coordinate for clipping.
	"BUTTON UP" sets the second coordinate for clipping.
	// On program startup, BUTTON_DOWN coordinate is set at (0, 0) and BUTTON_UP coordinate   
	// is set at (grid_width - 1, grid_height - 1).

8. Change Viewport by Coordinates
	Press "8" in the "Polygon" window.

	In the CONSOLE window:
	Type in the first "x" coordinate and press ENTER.
	Type in the first "y" coordinate and press ENTER.
	Type in the second "x" coordinate and press ENTER.
	Type in the second "y" coordinate and press ENTER.

9. Reset
	Press "9" in the "Polygon" window.

	// This displays only the polygon verticies on the screen. 
	// Viewport is changed back to the entire window. 

0. Exitting and outputting to "output.txt"
	Press "0" in the "Polygon" window.

	// This exits the program and outputs the polygon verticies into "output.txt".