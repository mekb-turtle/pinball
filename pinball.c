#include <stdbool.h>
#include <GL/glut.h>
#include <math.h>

#define SCALE_X 0.02f
#define SCALE_Y 0.02f
#define SCALE_Z 0.5f
#define CIRCLE_DRAW_STEP 0.1f
#define FLOOR_COLOR    0.5f, 0.0f, 0.1f, 1.0f
#define WALL_COLOR     1.0f, 1.0f, 1.0f, 1.0f
#define BUMPER_COLOR   1.0f, 0.0f, 1.0f, 1.0f
#define BALL_COLOR     0.0f, 1.0f, 0.0f, 1.0f
#define MARKER_COLOR   1.0f, 1.0f, 0.0f, 1.0f
#define LAUNCHER_COLOR 1.0f, 1.0f, 0.0f, 1.0f
#define Z0 -0.5f
#define Z1 +0.5f
#define ROTATE_X 7.5f
#define ROTATE_Y 1.0f
#define ROTATE_TILT_Y 4.0f
//#define DEBUG_MARKERS
#define DRAW_CYLINDER_TOP

#define PHYSICS_INTERVAL 25
#define PHYSICS_REPEAT 2
#define BUMPER_RADIUS 2.0f
#define BALL_RADIUS 1.25f
#define BUMPER_OLD_VEL 0.2f
#define BUMPER_NEW_VEL 1.1f
#define BUMPER_MIN_VEL 0.35f
#define WALL_NEW_VEL 2.0f
#define BUMPER_WALL_NEW_VEL 3.0f
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

struct ball_ {
	float x, y, vx, vy;
} ball;
struct pos {
	float x, y;
};
struct bumper {
	float x, y, bx, by;
} bumpers[] = {
	{ +BOARD_W*0.25f, +BOARD_H*0.30, 0.0f, 0.0f },
	{ -BOARD_W*0.25f, +BOARD_H*0.30, 0.0f, 0.0f },
	{ +BOARD_W*0.15f, -BOARD_H*0.15, 0.0f, 0.0f },
	{ -BOARD_W*0.12f, -BOARD_H*0.35, 0.0f, 0.0f },
};
struct pos2 {
	float x1, y1, x2, y2;
} walls[] = {
	{ +BOARD_W, +BOARD_H, +BOUNDS_W, +BOUNDS_H-(BOUNDS_W-BOARD_W)*1.25 }, // top right bumper wall
	{ +BOARD_W, +BOARD_H, +BOARD_W,  +BOARD_H -(BOUNDS_W-BOARD_W)*2 }, // barrier
	{ +BOARD_W, LAUNCHER_INIT_Y, +BOUNDS_W, LAUNCHER_INIT_Y }, // launcher
	{ +BOARD_W, -BOARD_H, +BOUNDS_W, -BOUNDS_H }, // bottom wall of launcher
	{ +BOARD_W, +BOARD_H, +BOUNDS_W, +BOUNDS_H }, // top wall of launcher
	{ +BOARD_W, -BOARD_H, +BOARD_W, +BOARD_H-(BOUNDS_W-BOARD_W)*2 }, // left wall of launcher
	{ +BOUNDS_W, -BOARD_H, +BOUNDS_W, +BOUNDS_H }, // right wall of launcher
	{ +BOARD_W, +BOARD_H, -BOARD_W, +BOARD_H }, // top wall of game
	{ -BOARD_W, +BOARD_H, -BOARD_W, -BOARD_H }, // left wall of game
	{ +BOARD_W*0.5, -BOARD_H*0.3, +BOARD_W, -BOARD_H*0.2 },
	{ -BOARD_W, -BOARD_H*0.7, -BOARD_W*0.5, -BOARD_H*0.8 },
	{ +BOARD_W, -BOARD_H*0.7, +BOARD_W*0.5, -BOARD_H*0.8 },
	{ -BOARD_W*0.5, -BOARD_H*0.9, +BOARD_W*0.5, -BOARD_H*0.8 },
	{ -BOARD_W*0.8, +BOARD_H*0.3, -BOARD_W*0.5, +BOARD_H*0.4 },
	{ +BOARD_W*0.8, +BOARD_H*0.3, +BOARD_W*0.5, +BOARD_H*0.4 },
}, floors[] = {
	{ -BOARD_W, -BOARD_H, +BOARD_W, +BOARD_H },
	{ +BOARD_W, -BOARD_H, +BOUNDS_W, +BOARD_H },
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
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glPointSize(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPushMatrix();
	float tilt = get_tilt();
	glRotatef(tilt != 0.0f ? ROTATE_TILT_Y : ROTATE_Y, 0.0f, tilt || 1.0f, 0.0f);
	glRotatef(ROTATE_X, 2.0f, 0.0f, 0.0f);
	glScalef(SCALE_X, SCALE_Y, SCALE_Z);
	glLightfv(GL_LIGHT0, GL_POSITION, (float[]) {0.0f,0.0f,0.0f,1.0f});
	glColor4f(BUMPER_COLOR);
	for (size_t i = 0; i < sizeof(bumpers) / sizeof(struct bumper); ++i) {
		draw_circle(BUMPER_RADIUS, bumpers[i].x - bumpers[i].bx, bumpers[i].y - bumpers[i].by);
		bumpers[i].bx = bumpers[i].by = 0.0f;
	}
	glColor4f(BALL_COLOR);
	draw_circle(BALL_RADIUS, ball.x, ball.y);
	for (size_t i = 0; i < sizeof(walls) / sizeof(struct pos2); ++i) {
		if (i == 1 && !started) continue;
		switch (i) {
			case 0: glColor4f(BUMPER_COLOR); break; 
			case 2: glColor4f(LAUNCHER_COLOR); break; 
			case 1: case 3: glColor4f(WALL_COLOR); break; 
		}
		if (i == 3 && launch >= 1.0f) continue;
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
		glVertex3f(floors[i].x2, floors[i].y1, Z1);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex3f(floors[i].x1, floors[i].y2, Z1);
		glVertex3f(floors[i].x2, floors[i].y1, Z1);
		glVertex3f(floors[i].x2, floors[i].y2, Z1);
		glEnd();
	}
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
void test(int bla) {
	pulling_launch = 0;
}
void reset(struct ball_ *b) {
	started = 0;
	launch = 0.0f;
	release_at = 0.0f;
	releasing = 0;
	pulling_launch = 0;
	reset_ball(b);
	pulling_launch = 1;
	glutTimerFunc(3000, test, 0);
}
void update_ball(struct ball_ *b) {
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
				b->vy = LAUNCH_RELEASE_STEP * (BOARD_H+LAUNCHER_INIT_Y) * release_at;
			} else { releasing = 0; }
		}
		walls[2].y1 = walls[2].y2 = lerp(LAUNCHER_INIT_Y, -BOARD_H, launch);
		b->x += b->vx / STEPS;
		b->y += b->vy / STEPS;
		if (started) b->vx += (GRAVITY_X * get_tilt()) / STEPS;
		b->vy -= (started ? GRAVITY_Y : LAUNCH_GRAVITY_Y) / STEPS;
		for (size_t i = 0; i < sizeof(bumpers) / sizeof(struct bumper); ++i) {
			float d = distance(b->x, b->y, bumpers[i].x, bumpers[i].y);
			if (d <= BALL_RADIUS + BUMPER_RADIUS) {
				float t = angle(bumpers[i].x, bumpers[i].y, b->x, b->y); // bumper angle
				float vd = distance(0, 0, b->vx, b->vy); // magnitude of velocity
				float vv = vd * BUMPER_NEW_VEL; // new velocity
				if (vv < BUMPER_MIN_VEL) vv = BUMPER_MIN_VEL; // set minimum new velocity
				b->vx = (cosf(t) * vv) + (b->vx * BUMPER_OLD_VEL), // set the velocity from the old and bounce velocity
				b->vy = (sinf(t) * vv) + (b->vx * BUMPER_OLD_VEL),
				b->x = bumpers[i].x + cosf(t) * (BALL_RADIUS + BUMPER_RADIUS), // push the ball so we don't collide again instantly
				b->y = bumpers[i].y + sinf(t) * (BALL_RADIUS + BUMPER_RADIUS);
				bumpers[i].bx = cosf(t);
				bumpers[i].by = sinf(t);
			}
		}
		for (size_t i = 0; i < sizeof(walls) / sizeof(struct pos2); ++i) {
			if (i == 1 && !started) continue;
			struct pos l = nearest(walls[i], b->x, b->y); // get point to collide with
			float d = distance(b->x, b->y, l.x, l.y);
			if (d <= BALL_RADIUS) {
				float t = angle(l.x, l.y, b->x, b->y); // wall angle
				if (i == 2) {
					b->vx = 0.0f;
				} else {
					float vd = distance(0, 0, b->vx, b->vy); // magnitude of velocity
					float vv = vd * (i == 0 ? BUMPER_WALL_NEW_VEL : WALL_NEW_VEL); // new velocity
					if (vv < WALL_MIN_VEL) vv = WALL_MIN_VEL; // set minimum new velocity
					b->vx -= (cosf(t) * vd) - (cosf(t) * vv), // cancel out the velocity with the wall angle
					b->vy -= (sinf(t) * vd) - (sinf(t) * vv); // set the velocity from subtracted old and bounce velocity
				}
				b->x = l.x + cosf(t) * BALL_RADIUS, // push the ball so we don't collide again instantly
				b->y = l.y + sinf(t) * BALL_RADIUS;
			}
		}
		float vd = distance(0, 0, b->vx, b->vy);
		if (vd >= MAX_VEL) {
			b->vx /= vd;
			b->vy /= vd;
			b->vx *= MAX_VEL;
			b->vy *= MAX_VEL;
		}
		if (!started) {
			if (b->x <= BOARD_W && b->y <= BOARD_H && b->x >= -BOARD_W && b->y >= -BOARD_H) {
				struct pos l = nearest(walls[1], b->x, b->y);
				float d = distance(b->x, b->y, l.x, l.y);
				if (d > BALL_RADIUS)
					started = 1;
			}
		}
		if ((b->y < -BOUNDS_H || b->y > BOUNDS_H || b->x < -BOUNDS_W || b->x > BOUNDS_W) || (started && (b->y < -BOARD_H || b->y > BOARD_H || b->x < -BOARD_W || b->x > BOARD_W))) {
			return reset(b);
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
	reset(&ball);
	physics(0);
	glutMainLoop();
	return 0;
}
