#include <stdio.h>
#include <stdbool.h>
#include <GL/glut.h>
#include <math.h>

#define SCALE_X 0.03f
#define SCALE_Y 0.03f
#define SCALE_Z 1.0f
#define CIRCLE_DRAW_STEP 0.1f
#define FLOOR_COLOR    0.5f, 0.0f, 0.1f, 1.0f // dark red
#define WALL_COLOR     2.0f, 2.0f, 2.0f, 1.0f // white
#define BUMPER_COLOR   2.0f, 0.5f, 2.0f, 1.0f // magenta
#define BALL_COLOR     0.0f, 2.0f, 0.5f, 1.0f // green
#define MARKER_COLOR   2.0f, 2.0f, 0.5f, 1.0f // yellow
#define LAUNCHER_COLOR 2.0f, 2.0f, 0.5f, 1.0f // yellow
#define FLIPPER_COLOR  2.0f, 0.6f, 0.5f, 1.0f // red
#define Z0 -0.5f
#define Z1 +0.5f
#define ROTATE_X 3.0f
#define ROTATE_Y 0.0f
#define ROTATE_TILT_Y 4.0f
//#define DEBUG_MARKERS
#define DRAW_CYLINDER_TOP
#define WALL_X_OFFSET -0.3f
#define WALL_Y_OFFSET 0.0f

#define PHYSICS_INTERVAL 25
#define PHYSICS_REPEAT 2
#define BUMPER_RADIUS 2.0f
#define BALL_RADIUS 1.25f
#define BUMPER_OLD_VEL 0.2f
#define BUMPER_NEW_VEL 1.1f
#define BUMPER_MIN_VEL 0.35f
#define WALL_NEW_VEL 2.0f
#define BUMPER_WALL_NEW_VEL 3.0f
#define FLIPPER_WALL_NEW_VEL 2.0f
#define FLIPPER_WALL_MIN_VEL 0.35f
#define WALL_MIN_VEL 0.08f
#define MAX_VEL 0.6f
#define BOARD_W 20.0f
#define BOARD_H 30.0f
#define BOUNDS_W 25.0f
#define BOUNDS_H 30.0f
#define GRAVITY_X 0.002f
#define GRAVITY_Y 0.008f
#define LAUNCH_GRAVITY_Y 0.0035f
#define BALL_INIT_X ((BOARD_W+BOUNDS_W)/2.0f)
#define BALL_INIT_Y (BOARD_H * -0.4)
#define LAUNCHER_INIT_Y (BALL_INIT_Y-BALL_RADIUS)
#define STEPS 20
#define LAUNCH_PULL_STEP 0.0045f
#define LAUNCH_RELEASE_STEP 0.05f
#define FLIPPER_MOVE_STEP 0.065f
#define FLIPPER_ANGLE 0.125f
#define FLIPPER_MOVE_ANGLE 0.25f
#define FLIPPER_LENGTH 0.4f

#define WALL_BUMPER 0
#define WALL_BARRIER 1
#define WALL_LAUNCHER 2
#define WALL_BOTTOM 3
#define WALL_TOP 4
#define WALL_LEFT 5
#define WALL_TOP_RIGHT 6
#define WALL_RIGHT 7
#define WALL_FLIPPER_L 8
#define WALL_FLIPPER_R 9

struct ball_ {
	float x, y, vx, vy;
	bool launch;
} ball;
struct pos {
	float x, y;
};
struct bumper {
	float x, y, bx, by;
} bumpers[] = {
	{ .x=+BOARD_W*0.25f, .y=+BOARD_H*0.30 },
	{ .x=-BOARD_W*0.25f, .y=+BOARD_H*0.30 },
	{ .x=+BOARD_W*0.15f, .y=-BOARD_H*0.15 },
};
struct pos2 {
	float x1, y1, x2, y2;
} walls[] = {
	{ .x1=+BOARD_W, .y1=+BOARD_H, .x2=+BOUNDS_W, .y2=+BOUNDS_H-(BOUNDS_W-BOARD_W)*1.25 }, // top right bumper wall
	{ .x1=+BOARD_W, .y1=+BOARD_H, .x2=+BOARD_W, .y2= +BOARD_H -(BOUNDS_W-BOARD_W)*2 }, // barrier
	{ .x1=+BOARD_W, .y1=LAUNCHER_INIT_Y, .x2=+BOUNDS_W, .y2=LAUNCHER_INIT_Y }, // launcher
	{ .x1=+BOARD_W, .y1=-BOARD_H, .x2=+BOUNDS_W, .y2=-BOUNDS_H }, // bottom wall of launcher
	{ .x1=+BOARD_W, .y1=+BOARD_H, .x2=+BOUNDS_W, .y2=+BOUNDS_H }, // top wall of launcher
	{ .x1=+BOARD_W, .y1=-BOARD_H, .x2=+BOARD_W, .y2=+BOARD_H-(BOUNDS_W-BOARD_W)*2 }, // left wall of launcher
	{ .x1=+BOUNDS_W, .y1=+BOARD_H, .x2=+BOUNDS_W, .y2=+BOUNDS_H-(BOUNDS_W-BOARD_W)*1.25 }, // top right wall of launcher
	{ .x1=+BOUNDS_W, .y1=-BOARD_H, .x2=+BOUNDS_W, .y2=+BOUNDS_H-(BOUNDS_W-BOARD_W)*1.25 }, // right wall of launcher
	{ .x1=-BOARD_W*0.5, .y1=-BOARD_H*0.8 }, // left flipper
	{ .x1=+BOARD_W*0.5, .y1=-BOARD_H*0.8 }, // right flipper
	{ .x1=+BOARD_W, .y1=+BOARD_H, .x2=-BOARD_W, .y2=+BOARD_H }, // top wall of game
	{ .x1=-BOARD_W, .y1=+BOARD_H, .x2=-BOARD_W, .y2=-BOARD_H }, // left wall of game
	{ .x1=+BOARD_W*0.5, .y1=-BOARD_H*0.3, .x2=+BOARD_W, .y2=-BOARD_H*0.2 },
	{ .x1=-BOARD_W*0.5, .y1=-BOARD_H*0.6, .x2=-BOARD_W, .y2=-BOARD_H*0.5 },
	{ .x1=-BOARD_W, .y1=-BOARD_H*0.7, .x2=-BOARD_W*0.5, .y2=-BOARD_H*0.8 },
	{ .x1=+BOARD_W, .y1=-BOARD_H*0.7, .x2=+BOARD_W*0.5, .y2=-BOARD_H*0.8 },
	{ .x1=-BOARD_W*0.8, .y1=+BOARD_H*0.3, .x2=-BOARD_W*0.5, .y2=+BOARD_H*0.4 },
	{ .x1=+BOARD_W*0.8, .y1=+BOARD_H*0.3, .x2=+BOARD_W*0.5, .y2=+BOARD_H*0.4 },
}, floors[] = {
	{ .x1=-BOARD_W, .y1=-BOARD_H, .x2=+BOARD_W, .y2=+BOARD_H },
	{ .x1=+BOARD_W, .y1=-BOARD_H, .x2=+BOUNDS_W, .y2=+BOARD_H },
};
bool flipper_l, flipper_r;
struct flipper {
	size_t i;
	float length;
	float angle;
	float move_angle;
	float move;
	bool *active;
} flippers[] = {
	{ .i=WALL_FLIPPER_L, .length=BOARD_W*FLIPPER_LENGTH, .angle=-M_PI*  (FLIPPER_ANGLE), .move_angle=+M_PI*FLIPPER_MOVE_ANGLE, .move=0.0f, .active=&flipper_l },
	{ .i=WALL_FLIPPER_R, .length=BOARD_W*FLIPPER_LENGTH, .angle=-M_PI*(1-FLIPPER_ANGLE), .move_angle=-M_PI*FLIPPER_MOVE_ANGLE, .move=0.0f, .active=&flipper_r },
};
float distance(float x1, float y1, float x2, float y2) {
	return sqrtf(((x2-x1)*(x2-x1))+((y2-y1)*(y2-y1)));
}
float angle(float x1, float y1, float x2, float y2) {
	return atan2f(y2-y1, x2-x1);
}
float lerp(float a, float b, float t) {
	return (b-a)*t+a;
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
bool started = 0;
bool pulling_launch;
float launch = 0.0f;
bool releasing = 0;
float release_at = 0.0f;
float get_tilt() {
	return tilt_r && tilt_l ? 0.0f : tilt_r ? 1.0f : tilt_l ? -1.0f : 0.0f;
}
void draw_circle(float radius, float x, float y) {
	for (float j = -M_PI; j < M_PI; j += CIRCLE_DRAW_STEP) {
		float k = j + CIRCLE_DRAW_STEP,
		jx = cosf(j) * radius + x,
		jy = sinf(j) * radius + y,
		kx = cosf(k) * radius + x,
		ky = sinf(k) * radius + y,
		hx = cosf(0) * radius + x,
		hy = sinf(0) * radius + y;
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
#ifdef DRAW_CYLINDER_TOP
		if (j != 0.0f && k != 0.0f) {
			glBegin(GL_POLYGON);
			glVertex3f(jx, jy, Z0);
			glVertex3f(kx, ky, Z0);
			glVertex3f(hx, hy, Z0);
			glEnd();
		}
#endif
	}
}
void display() {
	w  = glutGet(GLUT_WINDOW_WIDTH);
	h  = glutGet(GLUT_WINDOW_HEIGHT);
	s  = w > h ? h : w;
	ow = w > h ? (w - s) / 2 : 0;
	oh = w < h ? (h - s) / 2 : 0;
	glViewport(ow, oh, s, s);
	glPointSize(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	float tilt = get_tilt();
	glRotatef(tilt != 0.0f ? ROTATE_TILT_Y * -tilt : ROTATE_Y, 0.0f, 1.0f, 0.0f);
	glRotatef(ROTATE_X, 2.0f, 0.0f, 0.0f);
	glScalef(SCALE_X, SCALE_Y, SCALE_Z);
	glLightfv(GL_LIGHT0, GL_POSITION, (float[]) {0.0f,0.0f,0.0f,1.0f});
	glColor4f(BUMPER_COLOR);
	for (size_t i = 0; i < sizeof(bumpers) / sizeof(struct bumper); ++i) {
		draw_circle(BUMPER_RADIUS, bumpers[i].x - bumpers[i].bx, bumpers[i].y - bumpers[i].by);
		bumpers[i].bx = bumpers[i].by = 0.0f;
	}
	glColor4f(WALL_COLOR);
	glBegin(GL_POLYGON);
	glVertex3f(walls[WALL_BUMPER].x1, walls[WALL_BUMPER].y1, Z0);
	glVertex3f(walls[WALL_BUMPER].x2, walls[WALL_BUMPER].y2, Z0);
	glVertex3f(walls[WALL_BUMPER].x2, walls[WALL_BUMPER].y1, Z0);
	glEnd();
	glColor4f(BALL_COLOR);
	draw_circle(BALL_RADIUS, ball.x, ball.y);
	for (size_t i = 0; i < sizeof(walls) / sizeof(struct pos2); ++i) {
		if (i == WALL_BARRIER && !started) continue;
		switch (i) {
			case WALL_LAUNCHER:
				glColor4f(LAUNCHER_COLOR);
				break;
			case WALL_FLIPPER_L:
			case WALL_FLIPPER_R:
				glColor4f(FLIPPER_COLOR);
				break;
			default:
				glColor4f(WALL_COLOR);
				break;
		}
		if (i == WALL_BOTTOM && launch >= 1.0f) continue;
		float off = (walls[i].x1 == walls[i].x2 && tilt == 0.0f) * WALL_X_OFFSET;
		if (i == WALL_LEFT || i == WALL_BARRIER) off = -off;
		glBegin(GL_POLYGON);
		glVertex3f(walls[i].x1, walls[i].y1, Z0);
		glVertex3f(walls[i].x2, walls[i].y2, Z0);
		glVertex3f(walls[i].x2+off, walls[i].y2+WALL_Y_OFFSET, Z1);
		glVertex3f(walls[i].x1+off, walls[i].y1+WALL_Y_OFFSET, Z1);
		glEnd();
	}
#ifdef DEBUG_MARKERS
	glColor4f(MARKER_COLOR);
	for (size_t i = 0; i < sizeof(walls) / sizeof(struct pos2); ++i) {
		struct pos l = nearest(walls[i], ball.x, ball.y);
		draw_circle(1, l.x, l.y);
	}
#endif
	glColor4f(FLOOR_COLOR);
	for (size_t i = 0; i < sizeof(floors) / sizeof(struct pos2); ++i) {
		glBegin(GL_POLYGON);
		glVertex3f(floors[i].x1, floors[i].y1, Z1);
		glVertex3f(floors[i].x1, floors[i].y2, Z1);
		glVertex3f(floors[i].x2, floors[i].y2, Z1);
		glVertex3f(floors[i].x2, floors[i].y1, Z1);
		glEnd();
	}
	glPopMatrix();
	glutSwapBuffers();
}
void keyboard(unsigned char key, bool down) {
	switch (key) {
		case 'q': case 'Q': flipper_l = down; break;
		case 'e': case 'E': flipper_r = down; break;
		case 'z': tilt_l = down; case 'Z': break;
		case 'c': tilt_r = down; case 'C': break;
		case ' ': pulling_launch = down; break;
		default: return;
	}
	glutPostRedisplay();
}
void keyboardDown(unsigned char key, int x, int y) {
	keyboard(key, 1);
}
void keyboardUp(unsigned char key, int x, int y) {
	keyboard(key, 0);
}
void reset_ball(struct ball_ *b) {
	b->vx = b->vy = 0;
	b->launch = 0;
	b->x = BALL_INIT_X;
	b->y = BALL_INIT_Y;
}
void reset(struct ball_ *b) {
	started = 0;
	reset_ball(b);
}
void physics(int bla) {
	for (int i = 0; i < STEPS * PHYSICS_REPEAT; ++i) {
		if (pulling_launch && !releasing) {
			launch += LAUNCH_PULL_STEP / STEPS;
			if (launch > 1.0f) launch = 1.0f;
		} else {
			if (!releasing) {
				release_at = lerp(launch, sqrtf(launch), 0.6);
			}
			if (launch > 0.0f) {
				releasing = 1;
				launch -= LAUNCH_RELEASE_STEP * release_at / STEPS;
				if (launch < 0.0f) launch = 0.0f;
				if (ball.launch) ball.vy = LAUNCH_RELEASE_STEP * (BOARD_H+LAUNCHER_INIT_Y) * release_at;
			} else { releasing = 0; }
		}
		walls[2].y1 = walls[2].y2 = lerp(LAUNCHER_INIT_Y, -BOARD_H, launch);
		ball.x += ball.vx / STEPS;
		ball.y += ball.vy / STEPS;
		if (started) ball.vx += (GRAVITY_X * get_tilt()) / STEPS;
		ball.vy -= (started ? GRAVITY_Y : LAUNCH_GRAVITY_Y) / STEPS;
		for (size_t i = 0; i < sizeof(bumpers) / sizeof(struct bumper); ++i) {
			float d = distance(ball.x, ball.y, bumpers[i].x, bumpers[i].y);
			if (d <= BALL_RADIUS + BUMPER_RADIUS) {
				float t = angle(bumpers[i].x, bumpers[i].y, ball.x, ball.y); // bumper angle
				float vd = distance(0, 0, ball.vx, ball.vy); // magnitude of velocity
				float vv = vd * BUMPER_NEW_VEL; // new velocity
				if (vv < BUMPER_MIN_VEL) vv = BUMPER_MIN_VEL; // set minimum new velocity
				ball.vx = (cosf(t) * vv) + (ball.vx * BUMPER_OLD_VEL), // set the velocity from the old and bounce velocity
				ball.vy = (sinf(t) * vv) + (ball.vx * BUMPER_OLD_VEL),
				ball.x = bumpers[i].x + cosf(t) * (BALL_RADIUS + BUMPER_RADIUS), // push the ball so we don't collide again instantly
				ball.y = bumpers[i].y + sinf(t) * (BALL_RADIUS + BUMPER_RADIUS);
				bumpers[i].bx = cosf(t);
				bumpers[i].by = sinf(t);
			}
		}
		ball.launch = 0;
		for (size_t i = 0; i < sizeof(flippers) / sizeof(struct flipper); ++i) {
			if (*flippers[i].active) {
				flippers[i].move += FLIPPER_MOVE_STEP / STEPS;
				if (flippers[i].move > 1.0f) flippers[i].move = 1.0f;
			} else {
				flippers[i].move -= FLIPPER_MOVE_STEP / STEPS;
				if (flippers[i].move < 0.0f) flippers[i].move = 0.0f;
			}
			walls[flippers[i].i].x2 = cosf(flippers[i].move_angle * flippers[i].move + flippers[i].angle) * flippers[i].length + walls[flippers[i].i].x1;
			walls[flippers[i].i].y2 = sinf(flippers[i].move_angle * flippers[i].move + flippers[i].angle) * flippers[i].length + walls[flippers[i].i].y1;
		}
		for (size_t i = 0; i < sizeof(walls) / sizeof(struct pos2); ++i) {
			if (i == WALL_BARRIER && !started) continue;
			if (i == WALL_BOTTOM && launch >= 1.0f) continue;
			struct pos l = nearest(walls[i], ball.x, ball.y); // get point to collide with
			float d = distance(ball.x, ball.y, l.x, l.y);
			if (d <= BALL_RADIUS) {
				if (i == WALL_LAUNCHER) ball.launch = 1;
				float t = angle(l.x, l.y, ball.x, ball.y); // wall angle
				if (i == WALL_LAUNCHER) {
					ball.vx = 0.0f;
				} else {
					float vd = distance(0, 0, ball.vx, ball.vy); // magnitude of velocity
					bool isFlipper = i == WALL_FLIPPER_L || i == WALL_FLIPPER_R;
					float v = WALL_NEW_VEL;
					if (i == WALL_BUMPER) v = BUMPER_WALL_NEW_VEL;
					if (isFlipper) v = FLIPPER_WALL_NEW_VEL;
					float vv = vd * v; // new velocity
					if (isFlipper) {
						if (vv < FLIPPER_WALL_MIN_VEL) vv = FLIPPER_WALL_MIN_VEL; // set minimum new velocity
					} else {
						if (vv < WALL_MIN_VEL) vv = WALL_MIN_VEL; // set minimum new velocity
					}
					ball.vx -= (cosf(t) * vd) - (cosf(t) * vv), // cancel out the velocity with the wall angle
					ball.vy -= (sinf(t) * vd) - (sinf(t) * vv); // set the velocity from subtracted old and bounce velocity
				}
				ball.x = l.x + cosf(t) * BALL_RADIUS, // push the ball so we don't collide again instantly
				ball.y = l.y + sinf(t) * BALL_RADIUS;
			}
		}
		float vd = distance(0, 0, ball.vx, ball.vy);
		if (vd >= MAX_VEL) {
			ball.vx /= vd;
			ball.vy /= vd;
			ball.vx *= MAX_VEL;
			ball.vy *= MAX_VEL;
		}
		if (!started) {
			if (ball.x <= BOARD_W && ball.y <= BOARD_H && ball.x >= -BOARD_W && ball.y >= -BOARD_H) {
				struct pos l = nearest(walls[1], ball.x, ball.y);
				float d = distance(ball.x, ball.y, l.x, l.y);
				if (d > BALL_RADIUS)
					started = 1;
			}
		}
		if ((ball.y < -BOUNDS_H || ball.y > BOUNDS_H || ball.x < -BOUNDS_W || ball.x > BOUNDS_W) || (started && (ball.y < -BOARD_H || ball.y > BOARD_H || ball.x < -BOARD_W || ball.x > BOARD_W))) {
			reset(&ball);
		}
	}
	glutPostRedisplay();
	glutTimerFunc(PHYSICS_INTERVAL, physics, 0);
}
int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Pinball");
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glLightfv(GL_LIGHT0, GL_AMBIENT,  (float[]){0.0f,0.0f,0.0f,1.0f});
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  (float[]){1.0f,1.0f,1.0f,1.0f});
	glLightfv(GL_LIGHT0, GL_SPECULAR, (float[]){1.0f,1.0f,1.0f,1.0f});
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_LIGHTING);
	glutDisplayFunc(display);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);
	tilt_l = tilt_r = flipper_l = flipper_r = 0;
	reset(&ball);
	physics(0);
	printf("Q/E to use flippers\n");
	printf("Z/C to flip board\n");
	printf("Space to launch\n");
	glutMainLoop();
	return 0;
}
