#include <stdbool.h>
#include <GL/glut.h>
#include <math.h>

#define SCALE_X 0.02f
#define SCALE_Y 0.02f
#define SCALE_Z 0.5f
#define CIRCLE_DRAW_STEP 0.1f
#define FLOOR_COLOR  0.5f, 0.0f, 0.1f, 1.0f
#define WALL_COLOR   1.0f, 1.0f, 1.0f, 1.0f
#define BUMPER_COLOR 1.0f, 0.0f, 1.0f, 1.0f
#define BALL_COLOR   0.0f, 1.0f, 0.0f, 1.0f
#define MARKER_COLOR 1.0f, 1.0f, 0.0f, 1.0f
#define Z0 -0.5f
#define Z1 +0.5f
#define ROTATE_X 7.5f
#define ROTATE_Y 4.0f

#define PHYSICS_INTERVAL 25
#define PHYSICS_REPEAT 2
#define BUMPER_RADIUS 2.0f
#define BALL_RADIUS 1.25f
#define BUMPER_OLD_VEL 0.2f
#define BUMPER_NEW_VEL 0.9f
#define BUMPER_MIN_VEL 0.35f
#define WALL_OLD_VEL 0.35f
#define WALL_NEW_X_VEL 0.6f
#define WALL_NEW_Y_VEL 0.2f
#define WALL_MIN_VEL 0.08f
#define MAX_VEL 0.6f
#define BOARD_W 20.0f
#define BOARD_H 30.0f
#define GRAVITY_X 0.003f
#define GRAVITY_Y 0.008f
#define BALL_INIT_X 0.0f
#define BALL_INIT_Y (BOARD_H * 0.8)
#define STEPS 20
//#define DEBUG_MARKER

struct ball_ {
	float x, y, vx, vy;
} ball;
struct pos {
	float x, y;
} bumpers[] = {
	{ +BOARD_W*0.25f, +BOARD_H*0.3 },
	{ -BOARD_W*0.25f, +BOARD_H*0.3 },
	{ 2.0f, -BOARD_H*0.3 },
};
struct pos2 {
	float x1, y1, x2, y2;
} walls[] = {
	{ +BOARD_W, -BOARD_H, +BOARD_W, +BOARD_H },
	{ +BOARD_W, +BOARD_H, -BOARD_W, +BOARD_H },
	{ -BOARD_W, +BOARD_H, -BOARD_W, -BOARD_H },
	{ -BOARD_W, -BOARD_H*0.7, -BOARD_W*0.5, -BOARD_H*0.8 },
	{ +BOARD_W, -BOARD_H*0.7, +BOARD_W*0.5, -BOARD_H*0.8 },
	{ -BOARD_W*0.5, -BOARD_H*0.8, +BOARD_W*0.5, -BOARD_H*0.9 },
	{ -BOARD_W*0.8, +BOARD_H*0.3, -BOARD_W*0.5, +BOARD_H*0.4 },
	{ +BOARD_W*0.8, +BOARD_H*0.3, +BOARD_W*0.5, +BOARD_H*0.4 },
}, floor_ = { -BOARD_W, -BOARD_H, +BOARD_W, +BOARD_H };
float distance(float x1, float y1, float x2, float y2) {
	return sqrtf(((x2-x1)*(x2-x1))+((y2-y1)*(y2-y1)));
}
float angle(float x1, float y1, float x2, float y2) {
	return atan2f(y2-y1, x2-x1);
}
struct pos nearest(struct pos2 line, float px, float py) {
	// https://stackoverflow.com/a/51906100
	float hx = line.x2 - line.x1, hy = line.y2 - line.y1;
	float hm = distance(0, 0, hx, hy);
	hx /= hm, hy /= hm;
	float lx = px - line.x1, ly = py - line.y1;
	float dot = (lx*hx)+(ly*hy);
	if (dot < 0.0f) dot = 0.0f;
	else if (dot > hm) dot = hm;
	return (struct pos) { line.x1 + hx * dot, line.y1 + hy * dot };
}
int w, h, s, ow, oh;
bool tilt_l, tilt_r;
void draw_circle(float radius, float x, float y) {
	for (float j = -M_PI; j < M_PI; j += CIRCLE_DRAW_STEP) {
		float k = j + CIRCLE_DRAW_STEP;
		float jx = cosf(j) * radius + x;
		float jy = sinf(j) * radius + y;
		float kx = cosf(k) * radius + x;
		float ky = sinf(k) * radius + y;
		float hx = cosf(0) * radius + x;
		float hy = sinf(0) * radius + y;
		glBegin(GL_POLYGON);
		glVertex3f(jx, jy, Z0);
		glVertex3f(kx, ky, Z0);
		glVertex3f(kx, ky, Z1);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex3f(jx, jy, Z0);
		glVertex3f(jx, jy, Z1);
		glVertex3f(kx, ky, Z1);
		glEnd();
		if (j != 0.0f && k != 0.0f) {
			glBegin(GL_POLYGON);
			glVertex3f(jx, jy, Z0);
			glVertex3f(kx, ky, Z0);
			glVertex3f(hx, hy, Z0);
			glEnd();
		}
	}
}
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
	glRotatef(ROTATE_Y, 0.0f, tilt_l ? 1.0f : tilt_r ? -1.0f : 0.0f, 0.0f);
	glRotatef(ROTATE_X, 2.0f, 0.0f, 0.0f);
	glScalef(SCALE_X, SCALE_Y, SCALE_Z);
	glLightfv(GL_LIGHT0, GL_POSITION, (float[]) {0.0f,0.0f,0.0f,1.0f});
	glColor4f(BUMPER_COLOR);
	for (size_t i = 0; i < sizeof(bumpers) / sizeof(struct pos); ++i) {
		draw_circle(BUMPER_RADIUS, bumpers[i].x, bumpers[i].y);
	}
	glColor4f(BALL_COLOR);
	draw_circle(BALL_RADIUS, ball.x, ball.y);
	glColor4f(WALL_COLOR);
	for (size_t i = 0; i < sizeof(walls) / sizeof(struct pos2); ++i) {
		glBegin(GL_POLYGON);
		glVertex3f(walls[i].x1, walls[i].y1, Z0);
		glVertex3f(walls[i].x2, walls[i].y2, Z0);
		glVertex3f(walls[i].x2, walls[i].y2, Z1);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex3f(walls[i].x1, walls[i].y1, Z0);
		glVertex3f(walls[i].x1, walls[i].y1, Z1);
		glVertex3f(walls[i].x2, walls[i].y2, Z1);
		glEnd();
	}
#ifdef DEBUG_MARKER
	glColor4f(MARKER_COLOR);
	for (size_t i = 0; i < sizeof(walls) / sizeof(struct pos2); ++i) {
		struct pos l = nearest(walls[i], ball.x, ball.y);
		draw_circle(1, l.x, l.y);
	}
#endif
	glColor4f(FLOOR_COLOR);
	glBegin(GL_POLYGON);
	glVertex3f(floor_.x1, floor_.y1, Z1);
	glVertex3f(floor_.x1, floor_.y2, Z1);
	glVertex3f(floor_.x2, floor_.y1, Z1);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(floor_.x1, floor_.y2, Z1);
	glVertex3f(floor_.x2, floor_.y1, Z1);
	glVertex3f(floor_.x2, floor_.y2, Z1);
	glEnd();
	glPopMatrix();
	glutSwapBuffers();
}
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'q': case 'Q': break;
		case 'e': case 'E': break;
		case 'z': case 'Z': break;
		case 'c': case 'C': break;
		default: return;
	}
	glutPostRedisplay();
}
void reset_ball(struct ball_ *b) {
	b->vx = b->vy = 0;
	b->x = BALL_INIT_X;
	b->y = BALL_INIT_Y;
}
void update_ball(struct ball_ *b) {
	for (int i = 0; i < STEPS * PHYSICS_REPEAT; ++i) {
		b->x += b->vx / STEPS;
		b->y += b->vy / STEPS;
		b->vx += (GRAVITY_X * (tilt_r ? 1.0f : tilt_l ? -1.0f : 0.0f)) / STEPS;
		b->vy -= (GRAVITY_Y) / STEPS;
		for (size_t i = 0; i < sizeof(bumpers) / sizeof(struct pos); ++i) {
			float d = distance(b->x, b->y, bumpers[i].x, bumpers[i].y);
			if (d <= BALL_RADIUS + BUMPER_RADIUS) {
				float t = angle(bumpers[i].x, bumpers[i].y, b->x, b->y);
				float vd = distance(0, 0, b->vx, b->vy);
				float vv = vd * BUMPER_NEW_VEL;
				if (vv < BUMPER_MIN_VEL) vv = BUMPER_MIN_VEL;
				b->vx = (cos(t) * vv) + (b->vx * BUMPER_OLD_VEL);
				b->vy = (sin(t) * vv) + (b->vx * BUMPER_OLD_VEL);
			}
		}
		for (size_t i = 0; i < sizeof(walls) / sizeof(struct pos2); ++i) {
			struct pos l = nearest(walls[i], b->x, b->y);
			float d = distance(b->x, b->y, l.x, l.y);
			if (d <= BALL_RADIUS) {
				float t = angle(l.x, l.y, b->x, b->y);
				float vd = distance(0, 0, b->vx, b->vy);
				float vvx = vd * WALL_NEW_X_VEL;
				float vvy = vd * WALL_NEW_Y_VEL;
				if (vvx < WALL_MIN_VEL) vvx = WALL_MIN_VEL;
				if (vvy < WALL_MIN_VEL) vvy = WALL_MIN_VEL;
				b->vx = (cos(t) * vvx) + (b->vx * WALL_OLD_VEL);
				b->vy = (sin(t) * vvy) + (b->vx * WALL_OLD_VEL);
			}
		}
		float vd = distance(0, 0, b->vx, b->vy);
		if (vd >= MAX_VEL) {
			b->vx /= vd;
			b->vy /= vd;
			b->vx *= MAX_VEL;
			b->vy *= MAX_VEL;
		}
		if (b->y < -BOARD_H) {
			return reset_ball(b);
		}
	}
}
void physics(int bla) {
	update_ball(&ball);
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
	reset_ball(&ball);
	glutTimerFunc(2000, physics, 0);
	glutMainLoop();
	return 0;
}
