#ifndef ACC
#define ACC 1
#endif

#ifndef MP
#define MP 0
#endif

#ifndef GUI
#define GUI 0
#endif

#ifndef LOG
#define LOG 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if GUI
#include <GL/glut.h>
#endif
#if ACC
#include <openacc.h>
#endif
#if MP
#include <omp.h>
#endif
#include <time.h>

clock_t start, cycles, min = LONG_MAX, max = 0, avg, current;
size_t runs = 0;

#ifndef WIDTH
#define WIDTH (int)(1920 * 2)
#endif

#ifndef HEIGHT
#define HEIGHT (int)(1080 * 2)
#endif

#define XY_TO_I(y, x) ((y)*WIDTH + (x))

uint8_t map_a[HEIGHT * WIDTH];

uint8_t map_b[HEIGHT * WIDTH];

static int map_flag = 0;

void draw();
void update_map();

int main(int argc, char **argv)
{
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
	switch (acc_get_device_type())
	{
	case acc_device_current:
		printf("OpenACC: %s\n", "current");
		break;
	case acc_device_none:
		printf("OpenACC: %s\n", "none");
		break;
	case acc_device_default:
		printf("OpenACC: %s\n", "default");
		break;
	case acc_device_host:
		printf("OpenACC: %s\n", "host");
		break;
	case acc_device_not_host:
		printf("OpenACC: %s\n", "not_host");
		break;
	case acc_device_nvidia:
		printf("OpenACC: %s\n", "nvidia");
		break;
	case acc_device_radeon:
		printf("OpenACC: %s\n", "radeon");
		break;
	}
	static const char title[] = "Game of life openACC: ON";
#elif MP
	omp_set_num_threads(omp_get_num_procs());
	printf("Threads: %d\n", omp_get_num_procs());
	static const char title[] = "Game of life openMP: ON";
#else
	static const char title[] = "Game of life";
#endif
#if GUI
	glutInit(&argc, argv);
	glViewport(0, 0, WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(500, 500);
	glutCreateWindow(title);
	gluOrtho2D(0, WIDTH, HEIGHT, 0);
	//glutFullScreen();
	glutDisplayFunc(draw);
	glutMainLoop();
#else
	for (;;)
	{
		draw();
		if (LOG)
		{
			avg = cycles / runs;
			min = (min <= current) * min + (min > current) * current;
			max = (max >= current) * max + (max < current) * current;
			printf("Cycles: %.2li - avg: %.2li - min: %.2li - max: %.2li\n", (long)current, (long)avg, (long)min, (long)max);
		}
	}
#endif
#if ACC
#pragma acc exit data delete (map_a, map_b)
#endif
}

void draw()
{
	start = clock();
	update_map();
	current = clock() - start;
	cycles += current;
	runs++;
	if (LOG)
	{
		avg = cycles / runs;
		min = (min <= current) * min + (min > current) * current;
		max = (max >= current) * max + (max < current) * current;
		printf("Cycles: %.2li - avg: %.2li - min: %.2li - max: %.2li\n", (long)current, (long)avg, (long)min, (long)max);
	}
#if GUI
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
	glutSwapBuffers();
	glutPostRedisplay();
#endif
	map_flag = (map_flag + 1) % 2;
}

void update_map()
{
	if (map_flag)
	{
#if ACC
#pragma acc parallel loop independent present(map_a, map_b)
#endif
#if MP
#pragma omp parallel for collapse(2) shared(map_a, map_b)
#endif
		for (int y = 1; y < HEIGHT - 1; y++)
		{
#if ACC
#pragma acc loop independent worker vector
#endif
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
#if ACC
#pragma acc update device(map_a)
#endif
	}
	else
	{
#if ACC
#pragma acc parallel loop independent present(map_a, map_b)
#endif
#if MP
#pragma omp parallel for collapse(2) shared(map_a, map_b)
#endif
		for (int y = 1; y < HEIGHT - 1; y++)
		{
#if ACC
#pragma acc loop independent worker vector
#endif
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
#if ACC
#pragma acc update device(map_b)
#endif
	}
}