#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <GL/glut.h>
#include <time.h>
#include <openacc.h>

clock_t start;

#define WIDTH 1920
#define HEIGHT 1080

#define ACC 1
#define ACC_DEVICE_TYPE nvidia

#define XY_TO_I(y, x) ((y)*WIDTH + (x))

uint8_t map_a[HEIGHT * WIDTH];

uint8_t map_b[HEIGHT * WIDTH];

static int map_flag = 0;

void draw();
void update_map();

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glViewport(0, 0, WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(600, 400);
	glutInitWindowPosition(500, 500);
	glutCreateWindow("Game of life");
	gluOrtho2D(0, WIDTH, HEIGHT, 0);
	//glutFullScreen();
	glutDisplayFunc(draw);
	int x = WIDTH / 2, y;
	for (y = 0; y < HEIGHT; y++)
	{
		map_a[XY_TO_I(y, x)] = 1;
	}
	y = HEIGHT / 2;
	for (x = 0; x < WIDTH; x++)
	{
		map_a[XY_TO_I(y, x)] = 1;
	}
#if ACC
#pragma acc enter data copyin(map_a, map_b)
#endif
	glutMainLoop();
#if ACC
#pragma acc exit data delete (map_a, map_b)
#endif
}

void draw()
{
	start = clock();
	update_map();
	printf("Dauer: %li\n", clock() - start);
	int x, y, index;
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
	for (y = 0; y < HEIGHT; y++)
	{
		for (x = 0; x < WIDTH; x++)
		{
			index = XY_TO_I(y, x);
			if (map_flag)
			{
				glColor3f((float)map_a[index], (float)map_a[index], (float)map_a[index]);
			}
			else
			{
				glColor3f((float)map_b[index], (float)map_b[index], (float)map_b[index]);
			}
			glVertex2i(x, y);
			glVertex2i(x + 1, y);
			glVertex2i(x + 1, y + 1);
			glVertex2i(x, y + 1);
		}
	}
	glEnd();
	map_flag = (map_flag + 1) % 2;
	glutSwapBuffers();
	glutPostRedisplay();
}

void update_map()
{
	if (map_flag)
	{
#pragma acc parallel loop gang present(map_a, map_b)
		for (int y = 1; y < HEIGHT - 1; y++)
		{
#pragma acc loop worker vector
			for (int x = 1; x < WIDTH - 1; x++)
			{
				int neighbors =
					map_b[XY_TO_I(y - 1, x - 1)] +
					map_b[XY_TO_I(y + 0, x - 1)] +
					map_b[XY_TO_I(y + 1, x - 1)] +
					map_b[XY_TO_I(y - 1, x - 0)] +
					map_b[XY_TO_I(y + 1, x - 0)] +
					map_b[XY_TO_I(y - 1, x + 1)] +
					map_b[XY_TO_I(y + 0, x + 1)] +
					map_b[XY_TO_I(y + 1, x + 1)];
				switch (neighbors)
				{
				case 2:
					map_a[XY_TO_I(y, x)] = map_b[XY_TO_I(y, x)] * 1;
					break;
				case 3:
					map_a[XY_TO_I(y, x)] = 1;
					break;

				default:
					map_a[XY_TO_I(y, x)] = 0;
					break;
				}
			}
		}
#pragma acc update device(map_a)
	}
	else
	{
#pragma acc parallel loop gang present(map_a, map_b)
		for (int y = 1; y < HEIGHT - 1; y++)
		{
#pragma acc loop worker vector
			for (int x = 1; x < WIDTH - 1; x++)
			{
				int neighbors =
					map_a[XY_TO_I(y - 1, x - 1)] +
					map_a[XY_TO_I(y + 0, x - 1)] +
					map_a[XY_TO_I(y + 1, x - 1)] +
					map_a[XY_TO_I(y - 1, x - 0)] +
					map_a[XY_TO_I(y + 1, x - 0)] +
					map_a[XY_TO_I(y - 1, x + 1)] +
					map_a[XY_TO_I(y + 0, x + 1)] +
					map_a[XY_TO_I(y + 1, x + 1)];
				switch (neighbors)
				{
				case 2:
					map_b[XY_TO_I(y, x)] = map_a[XY_TO_I(y, x)] * 1;
					break;
				case 3:
					map_b[XY_TO_I(y, x)] = 1;
					break;

				default:
					map_b[XY_TO_I(y, x)] = 0;
					break;
				}
			}
		}
#pragma acc update device(map_b)
	}
}