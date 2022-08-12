#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#define PHYSICS_INTERVAL 20
#define SCALE 0.4f
struct pos1 {
	float x, y;
} ball;
struct pos2 {
	float x1, y1, x2, y2;
} walls[] = {
	{ -1.0f, -1.5f, +1.0f, -1.5f },
	{ +1.0f, -1.5f, +1.0f, +1.5f },
	{ +1.0f, +1.5f, -1.0f, +1.5f },
	{ -1.0f, +1.5f, -1.0f, -1.5f }
}, floor_ = { -1.0f, -1.5f, +1.0f, +1.5f };
int w, h, s, ow, oh;
float rotatetest = 0.0f;
void display() {
	w  = glutGet(GLUT_WINDOW_WIDTH);
	h  = glutGet(GLUT_WINDOW_HEIGHT);
	s  = w > h ? h : w;
	ow = w > h ? (w - s) / 2 : 0;
	oh = w < h ? (h - s) / 2 : 0;
	glViewport(ow, oh, s, s);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glPointSize(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPushMatrix();
	glScalef(SCALE, SCALE, SCALE);
	glRotatef(rotatetest, 1.0f, 1.0f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLightfv(GL_LIGHT0, GL_POSITION, (float[]){0.0f,0.0f,0.0f,1.0f});
	for (size_t i = 0; i < sizeof(walls) / sizeof(struct pos2); ++i) {
		glBegin(GL_POLYGON);
		glVertex3f(walls[i].x1, walls[i].y1, -0.5f);
		glVertex3f(walls[i].x2, walls[i].y2, -0.5f);
		glVertex3f(walls[i].x2, walls[i].y2, +0.5f);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex3f(walls[i].x1, walls[i].y1, -0.5f);
		glVertex3f(walls[i].x1, walls[i].y1, +0.5f);
		glVertex3f(walls[i].x2, walls[i].y2, +0.5f);
		glEnd();
	}
	glBegin(GL_POLYGON);
	glVertex3f(floor_.x1, floor_.y1, +0.5f);
	glVertex3f(floor_.x1, floor_.y2, +0.5f);
	glVertex3f(floor_.x2, floor_.y1, +0.5f);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(floor_.x1, floor_.y2, +0.5f);
	glVertex3f(floor_.x2, floor_.y1, +0.5f);
	glVertex3f(floor_.x2, floor_.y2, +0.5f);
	glEnd();
	glPopMatrix();
	glutSwapBuffers();
}
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'q': case 'Q': break;
		case 'e': case 'E': break;
		default: return;
	}
	glutPostRedisplay();
}
void physics(int bla) {
	rotatetest += 0.5f;
	glutPostRedisplay();
	glutTimerFunc(PHYSICS_INTERVAL, physics, 0);
}
int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Pinball");
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT,  (float[]){0.0f,0.0f,0.0f,1.0f});
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  (float[]){1.0f,1.0f,1.0f,1.0f});
	glLightfv(GL_LIGHT0, GL_SPECULAR, (float[]){1.0f,1.0f,1.0f,1.0f});
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_LIGHTING);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	physics(1337);
	glutMainLoop();
	return 0;
}
