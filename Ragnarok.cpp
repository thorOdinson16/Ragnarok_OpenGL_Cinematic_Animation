#include<gl/freeglut.h>
#include <vector>
#include <cstdlib>	
#include <algorithm>

float Tx = 0.0f, Ty = 0.0f, Tz = 0.0f; // Translation amounts
float angleX = 0.0f; // Rotation angle around X axis
float angleY = 0.0f; // Rotation angle around Y axis
float angleZ = 0.0f; // Rotation angle around Z axis

float Hy = 650.0f;

float cameraDistance = 0.0f; // Camera distance for zooming
int lastX = -1, lastY = -1; // Last mouse positions for dragging

float angle = 0.0f; // Rotation angle for animation

float cameraPosition = 605.0f; // Camera position along Z axis
bool reachedPosition = false; // Flag to track if camera reached the position

int windowWidth = 900;  // Window width
int windowHeight = 700; // Window height

bool lightningActive = false;  // Is lightning currently visible
float lightningDuration = 0.0f; // How long lightning lasts

float rotationSpeed = 2.0f; // Rotation speed
int rotations = 0; // Count of rotations completed

float surturEmergenceY = -700.0f; // Surtur's vertical position (starts below ground)
bool surturEmerging = false;

float surturScale = 0.1f; // Start small
bool surturScaling = false; // Track if we're in scaling phase

float swordRotation = 0.0f; // Sword rotation angle

float armRotation = 0.0f;
bool surturRaisingArms = false;
bool surturStriking = false;
float swordStrikeY = 0.0f;
float postStrikeTimer = 0.0f;

bool surturLiftingSword = false;
float swordLiftY = 0.0f;

float surturSideShift = 0.0f;
bool surturShifting = false;

float shakeTime = 0.0f;
float shakeStrength = 0.0f;

float shakeX = 0.0f;
float shakeY = 0.0f;

struct Piece {
	float x, y, z;      // Current position
	float vx, vy, vz;   // Velocity
	float ax, ay, az;   // Acceleration (gravity)
	float sizeX, sizeY, sizeZ; // dimensions (optional)
	float rx, ry, rz;     // rotation (degrees)
	float wrx, wry, wrz;  // angular velocity (deg/sec)
	int type; // 0 = spire, 1 = hammer, 2 = bridge, 3 = platform
};

std::vector<Piece> pieces;   // Stores all pieces
bool exploded = false;       // Explosion flag

float explosionTimer = 0.0f;

float clampf(float v, float lo, float hi) {
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

enum RagnarokState {
	CAMERA_DOLLY,
	HAMMER_DESCENT,
	LIGHTNING,
	WORLD_ROTATION,
	SURTUR_EMERGE,
	SURTUR_SCALE,
	SURTUR_RAISE_ARMS,
	SURTUR_SHIFT,
	SWORD_LIFT,
	SWORD_STRIKE,
	POST_STRIKE_WAIT,
	DEBRIS
};

RagnarokState state = CAMERA_DOLLY;

void drawSword() {
	// Sword Blade
	glColor3f(1.0f, 0.33f, 0.0f);
	glBegin(GL_QUADS);
	// Front face
	glVertex3f(-4.0f, 0.0f, 3.0f);
	glVertex3f(4.0f, 0.0f, 3.0f);
	glVertex3f(3.0f, 80.0f, 3.0f);
	glVertex3f(-3.0f, 80.0f, 3.0f);

	// Back face
	glVertex3f(-4.0f, 0.0f, -3.0f);
	glVertex3f(4.0f, 0.0f, -3.0f);
	glVertex3f(3.0f, 80.0f, -3.0f);
	glVertex3f(-3.0f, 80.0f, -3.0f);

	// Left face
	glVertex3f(-4.0f, 0.0f, -3.0f);
	glVertex3f(-4.0f, 0.0f, 3.0f);
	glVertex3f(-3.0f, 80.0f, 3.0f);
	glVertex3f(-3.0f, 80.0f, -3.0f);

	// Right face
	glVertex3f(4.0f, 0.0f, -3.0f);
	glVertex3f(4.0f, 0.0f, 3.0f);
	glVertex3f(3.0f, 80.0f, 3.0f);
	glVertex3f(3.0f, 80.0f, -3.0f);
	glEnd();

	// Sword Tip (triangle)
	glBegin(GL_TRIANGLES);
	// Front triangle
	glVertex3f(-3.0f, 80.0f, 3.0f);
	glVertex3f(3.0f, 80.0f, 3.0f);
	glVertex3f(0.0f, 95.0f, 3.0f);  // Front tip

	// Back triangle
	glVertex3f(-3.0f, 80.0f, -3.0f);
	glVertex3f(3.0f, 80.0f, -3.0f);
	glVertex3f(0.0f, 95.0f, -3.0f);  // Back tip

	// Left triangle
	glVertex3f(-3.0f, 80.0f, 3.0f);
	glVertex3f(-3.0f, 80.0f, -3.0f);
	glVertex3f(0.0f, 95.0f, 0.0f);  // Center tip

	// Right triangle
	glVertex3f(3.0f, 80.0f, 3.0f);
	glVertex3f(3.0f, 80.0f, -3.0f);
	glVertex3f(0.0f, 95.0f, 0.0f);  // Center tip
	glEnd();

	// Crossguard
	glColor3f(0.4f, 0.2f, 0.0f);
	glBegin(GL_QUADS);
	// Horizontal bar
	glVertex3f(-17.5f, 0.0f, 7.0f);
	glVertex3f(17.5f, 0.0f, 7.0f);
	glVertex3f(17.5f, 3.0f, 7.0f);
	glVertex3f(-17.5f, 3.0f, 7.0f);

	glVertex3f(-17.5f, 0.0f, -7.0f);
	glVertex3f(17.5f, 0.0f, -7.0f);
	glVertex3f(17.5f, 3.0f, -7.0f);
	glVertex3f(-17.5f, 3.0f, -7.0f);

	glVertex3f(-17.5f, 0.0f, -7.0f);
	glVertex3f(-17.5f, 0.0f, 7.0f);
	glVertex3f(-17.5f, 3.0f, 7.0f);
	glVertex3f(-17.5f, 3.0f, -7.0f);

	glVertex3f(17.5f, 0.0f, -7.0f);
	glVertex3f(17.5f, 0.0f, 7.0f);
	glVertex3f(17.5f, 3.0f, 7.0f);
	glVertex3f(17.5f, 3.0f, -7.0f);

	glVertex3f(-17.5f, 3.0f, 7.0f);
	glVertex3f(17.5f, 3.0f, 7.0f);
	glVertex3f(17.5f, 3.0f, -7.0f);
	glVertex3f(-17.5f, 3.0f, -7.0f);

	glVertex3f(-17.5f, 0.0f, 7.0f);
	glVertex3f(17.5f, 0.0f, 7.0f);
	glVertex3f(17.5f, 0.0f, -7.0f);
	glVertex3f(-17.5f, 0.0f, -7.0f);

	glEnd();

	glBegin(GL_QUADS);
	// Smaller Horizontal bar
	glVertex3f(-15.0f, 8.0f, 7.0f);  // Adjusted the height and size
	glVertex3f(15.0f, 8.0f, 7.0f);
	glVertex3f(15.0f, 10.0f, 7.0f);
	glVertex3f(-15.0f, 10.0f, 7.0f);

	glVertex3f(-15.0f, 8.0f, -7.0f);
	glVertex3f(15.0f, 8.0f, -7.0f);
	glVertex3f(15.0f, 10.0f, -7.0f);
	glVertex3f(-15.0f, 10.0f, -7.0f);

	glVertex3f(-15.0f, 8.0f, -7.0f);
	glVertex3f(-15.0f, 8.0f, 7.0f);
	glVertex3f(-15.0f, 10.0f, 7.0f);
	glVertex3f(-15.0f, 10.0f, -7.0f);

	glVertex3f(15.0f, 8.0f, -7.0f);
	glVertex3f(15.0f, 8.0f, 7.0f);
	glVertex3f(15.0f, 10.0f, 7.0f);
	glVertex3f(15.0f, 10.0f, -7.0f);

	glVertex3f(-15.0f, 10.0f, 7.0f);
	glVertex3f(15.0f, 10.0f, 7.0f);
	glVertex3f(15.0f, 10.0f, -7.0f);
	glVertex3f(-15.0f, 10.0f, -7.0f);

	glVertex3f(-15.0f, 8.0f, 7.0f);
	glVertex3f(15.0f, 8.0f, 7.0f);
	glVertex3f(15.0f, 8.0f, -7.0f);
	glVertex3f(-15.0f, 8.0f, -7.0f);
	glEnd();

	// Handle
	glColor3f(1.0f, 0.33f, 0.0f);
	glBegin(GL_QUADS);
	glVertex3f(-2.5f, -20.0f, 2.5f);
	glVertex3f(2.5f, -20.0f, 2.5f);
	glVertex3f(2.5f, 0.0f, 2.5f);
	glVertex3f(-2.5f, 0.0f, 2.5f);

	glVertex3f(-2.5f, -20.0f, -2.5f);
	glVertex3f(2.5f, -20.0f, -2.5f);
	glVertex3f(2.5f, 0.0f, -2.5f);
	glVertex3f(-2.5f, 0.0f, -2.5f);

	glVertex3f(-2.5f, -20.0f, -2.5f);
	glVertex3f(-2.5f, -20.0f, 2.5f);
	glVertex3f(-2.5f, 0.0f, 2.5f);
	glVertex3f(-2.5f, 0.0f, -2.5f);

	glVertex3f(2.5f, -20.0f, -2.5f);
	glVertex3f(2.5f, -20.0f, 2.5f);
	glVertex3f(2.5f, 0.0f, 2.5f);
	glVertex3f(2.5f, 0.0f, -2.5f);
	glEnd();

	// Pommel (sphere at bottom)
	glColor3f(1.0f, 0.33f, 0.0f);
	glTranslatef(0.0f, -22.0f, 0.0f);
	glutSolidSphere(5.0, 20, 20);
}

void initializePieces() {
	pieces.clear();

	auto randf = [](float a, float b) {
		return a + (b - a) * (rand() / (float)RAND_MAX);
		};

	const float VMAX = 12.0f;

	auto clampVel = [&](Piece& p) {
		p.vx = clampf(p.vx, -VMAX, VMAX);
		p.vy = clampf(p.vy, -VMAX, VMAX);
		p.vz = clampf(p.vz, -VMAX, VMAX);
		};

	auto addRotation = [&](Piece& p) {
		p.rx = randf(0, 360);
		p.ry = randf(0, 360);
		p.rz = randf(0, 360);

		p.wrx = randf(-3.0f, 3.0f);
		p.wry = randf(-3.0f, 3.0f);
		p.wrz = randf(-3.0f, 3.0f);
		};

	auto applyWeight = [&](Piece& p) {
		float scale = 1.0f;
		if (p.type == 1) scale = 0.6f;  // hammer (heavy)
		if (p.type == 3) scale = 0.8f;  // platform

		p.vx *= scale;
		p.vy *= scale;
		p.vz *= scale;
		};

	// Palace pieces
	for (float x = -40.0f; x <= 40.0f; x += 5.0f) {
		for (float z = -32.0f; z <= 4.0f; z += 4.0f) {
			Piece p;
			p.x = x;
			p.y = 0.0f;
			p.z = z;

			p.vx = randf(-2.0f, 2.0f);
			p.vy = randf(0.0f, 6.0f);
			p.vz = randf(-2.0f, 2.0f);

			p.ax = 0.0f;
			p.ay = 0.0f; // zero-gravity (Asgard)
			p.az = 0.0f;

			p.type = 0;
			applyWeight(p);
			clampVel(p);
			addRotation(p);

			pieces.push_back(p);
		}
	}

	// Hammer
	Piece hammer;
	hammer.x = 0.0f;
	hammer.y = Hy;
	hammer.z = 50.0f;

	hammer.vx = randf(-1.5f, 1.5f);
	hammer.vy = randf(1.0f, 4.0f);
	hammer.vz = randf(-1.5f, 1.5f);

	hammer.ax = hammer.ay = hammer.az = 0.0f;
	hammer.type = 1;
	applyWeight(hammer);
	clampVel(hammer);
	addRotation(hammer);

	pieces.push_back(hammer);

	// --- Bridge pieces ---
	for (float z = 2.0f; z <= 600.0f; z += 20.0f) {
		Piece p;
		p.x = randf(-6.0f, 6.0f);
		p.y = 0.5f;
		p.z = z;

		p.vx = randf(-2.5f, 2.5f);
		p.vy = randf(0.0f, 5.0f);
		p.vz = randf(-2.5f, 2.5f);

		p.ax = p.ay = p.az = 0.0f;
		p.type = 2;
		applyWeight(p);
		clampVel(p);
		addRotation(p);

		pieces.push_back(p);
	}

	// Platform pieces
	for (float x = -1000.0f; x <= 1000.0f; x += 100.0f) {
		for (float z = -80.0f; z <= 100.0f; z += 30.0f) {
			Piece p;
			p.x = x;
			p.y = -2.0f;
			p.z = z;

			p.vx = randf(-1.8f, 1.8f);
			p.vy = randf(0.0f, 4.0f);
			p.vz = randf(-1.8f, 1.8f);

			p.ax = p.ay = p.az = 0.0f;
			p.type = 3;
			applyWeight(p);
			clampVel(p);
			addRotation(p);

			pieces.push_back(p);
		}
	}
}

void animation() {

	switch (state) {

	case CAMERA_DOLLY:
		if (cameraPosition > 250.0f) {
			cameraPosition -= 0.3f;
		}
		else {
			state = HAMMER_DESCENT;
		}
		break;

	case HAMMER_DESCENT:
		if (Hy > Ty - 649.5f) {
			Hy -= 2.5f;
		}
		else {
			lightningDuration = 10.0f;
			lightningActive = true;
			state = LIGHTNING;
		}
		break;

	case LIGHTNING:
		if (lightningActive) {
			lightningDuration -= 0.016f;
			if (lightningDuration <= 0.0f) {
				lightningActive = false;
				rotations = 0;
				rotationSpeed = 2.0f;
				state = WORLD_ROTATION;
			}
		}
		break;

	case WORLD_ROTATION:
		angle += rotationSpeed;

		if (angle >= 360.0f) {
			angle = 0.0f;
			rotations++;

			if (rotations <= 7)
				rotationSpeed += 0.3f;
			else
				rotationSpeed -= 0.3f;

			if (rotations == 14) {
				rotationSpeed = 0.0f;
				surturEmergenceY = -50.0f;
				state = SURTUR_EMERGE;
			}
		}
		break;

	case SURTUR_EMERGE:
		surturEmergenceY += 1.0f;
		if (surturEmergenceY >= 0.0f) {
			surturEmergenceY = 0.0f;
			surturScale = 0.1f;
			state = SURTUR_SCALE;
		}
		break;

	case SURTUR_SCALE:
		surturScale += 0.005f;
		if (surturScale >= 1.0f) {
			surturScale = 1.0f;
			armRotation = 0.0f;
			state = SURTUR_RAISE_ARMS;
		}
		break;

	case SURTUR_RAISE_ARMS:
		armRotation += 0.5f;
		swordRotation += 1.0f;
		if (armRotation >= 90.0f) {
			armRotation = 90.0f;
			surturSideShift = 0.0f;
			state = SURTUR_SHIFT;
		}
		break;

	case SURTUR_SHIFT:
		surturSideShift += 0.4f;
		if (surturSideShift >= 95.0f) {
			surturSideShift = 95.0f;
			swordLiftY = 0.0f;
			state = SWORD_LIFT;
		}
		break;

	case SWORD_LIFT:
		swordLiftY += 0.3f;
		if (swordLiftY >= 40.0f) {
			swordLiftY = 40.0f;
			swordStrikeY = swordLiftY;
			state = SWORD_STRIKE;
		}
		break;

	case SWORD_STRIKE:
		swordStrikeY -= 7.0f;
		armRotation -= 4.0f;
		if (swordStrikeY <= -150.0f) {
			swordStrikeY = -150.0f;
			explosionTimer = 5.0f;
			state = POST_STRIKE_WAIT;
		}
		break;

	case POST_STRIKE_WAIT:
		explosionTimer -= 0.04f;
		if (explosionTimer <= 0.0f) {
			initializePieces();
			exploded = true;
			// Camera shake
			shakeTime = 5.0f;        // seconds
			shakeStrength = 15.0f;    // intensity
			state = DEBRIS;
		}
		break;

	case DEBRIS:
		for (auto& p : pieces) {
			p.vx += p.ax;
			p.vy += p.ay;
			p.vz += p.az;

			p.x += p.vx;
			p.y += p.vy;
			p.z += p.vz;

			// zero-G damping
			p.vx *= 0.995f;
			p.vy *= 0.995f;
			p.vz *= 0.995f;

			// rotation update (if present)
			p.rx += p.wrx;
			p.ry += p.wry;
			p.rz += p.wrz;
		}
		break;
	}
	
	if (shakeTime > 0.0f) {
		shakeTime -= 0.016f;

		shakeX = (rand() % 200 / 100.0f - 1.0f) * shakeStrength;
		shakeY = (rand() % 200 / 100.0f - 1.0f) * shakeStrength;

		shakeStrength *= 0.97f;
	}
	else {
		shakeX = 0.0f;
		shakeY = 0.0f;
	}

	glutPostRedisplay();
}

void drawSurtur()
{
	glBegin(GL_QUADS);
	glColor3f(0.5f, 0.0f, 0.0f);
	glVertex3f(5.0f, 300.0f, -100.0f);
	glVertex3f(50.0f, 320.0f, -100.0f);
	glVertex3f(50.0f, 310.0f, -100.0f);
	glVertex3f(5.0f, 290.0f, -100.0f);

	glVertex3f(5.0f, 300.0f, -90.0f);
	glVertex3f(5.0f, 290.0f, -90.0f);
	glVertex3f(50.0f, 310.0f, -90.0f);
	glVertex3f(50.0f, 320.0f, -90.0f);

	glVertex3f(5.0f, 300.0f, -100.0f);
	glVertex3f(5.0f, 290.0f, -100.0f);
	glVertex3f(5.0f, 290.0f, -90.0f);
	glVertex3f(5.0f, 300.0f, -90.0f);

	glVertex3f(50.0f, 320.0f, -100.0f);
	glVertex3f(50.0f, 310.0f, -100.0f);
	glVertex3f(50.0f, 310.0f, -90.0f);
	glVertex3f(50.0f, 320.0f, -90.0f);

	glVertex3f(5.0f, 300.0f, -100.0f);
	glVertex3f(50.0f, 320.0f, -100.0f);
	glVertex3f(50.0f, 320.0f, -90.0f);
	glVertex3f(5.0f, 300.0f, -90.0f);

	glVertex3f(5.0f, 290.0f, -100.0f);
	glVertex3f(50.0f, 310.0f, -100.0f);
	glVertex3f(50.0f, 310.0f, -90.0f);
	glVertex3f(5.0f, 290.0f, -90.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.5f, 0.0f, 0.0f);
	glVertex3f(-5.0f, 300.0f, -100.0f);
	glVertex3f(-50.0f, 320.0f, -100.0f);
	glVertex3f(-50.0f, 310.0f, -100.0f);
	glVertex3f(-5.0f, 290.0f, -100.0f);

	glVertex3f(-5.0f, 300.0f, -90.0f);
	glVertex3f(-5.0f, 290.0f, -90.0f);
	glVertex3f(-50.0f, 310.0f, -90.0f);
	glVertex3f(-50.0f, 320.0f, -90.0f);

	glVertex3f(-5.0f, 300.0f, -100.0f);
	glVertex3f(-5.0f, 290.0f, -100.0f);
	glVertex3f(-5.0f, 290.0f, -90.0f);
	glVertex3f(-5.0f, 300.0f, -90.0f);

	glVertex3f(-50.0f, 320.0f, -100.0f);
	glVertex3f(-50.0f, 310.0f, -100.0f);
	glVertex3f(-50.0f, 310.0f, -90.0f);
	glVertex3f(-50.0f, 320.0f, -90.0f);

	glVertex3f(-5.0f, 300.0f, -100.0f);
	glVertex3f(-50.0f, 320.0f, -100.0f);
	glVertex3f(-50.0f, 320.0f, -90.0f);
	glVertex3f(-5.0f, 300.0f, -90.0f);

	glVertex3f(-5.0f, 290.0f, -100.0f);
	glVertex3f(-50.0f, 310.0f, -100.0f);
	glVertex3f(-50.0f, 310.0f, -90.0f);
	glVertex3f(-5.0f, 290.0f, -90.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3f(0.5f, 0.0f, 0.0f);
	glVertex3f(50.0f, 310.0f, -100.0f);
	glVertex3f(50.0f, 390.0f, -100.0f);
	glVertex3f(35.0f, 310.0f, -100.0f);

	glVertex3f(50.0f, 310.0f, -90.0f);
	glVertex3f(50.0f, 390.0f, -90.0f);
	glVertex3f(35.0f, 310.0f, -90.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.5f, 0.0f, 0.0f);
	glVertex3f(35.0f, 310.0f, -100.0f);
	glVertex3f(50.0f, 390.0f, -100.0f);
	glVertex3f(50.0f, 390.0f, -90.0f);
	glVertex3f(35.0f, 310.0f, -90.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glColor3f(0.5f, 0.0f, 0.0f);
	glVertex3f(-50.0f, 310.0f, -100.0f);
	glVertex3f(-50.0f, 390.0f, -100.0f);
	glVertex3f(-35.0f, 310.0f, -100.0f);

	glVertex3f(-50.0f, 310.0f, -90.0f);
	glVertex3f(-50.0f, 390.0f, -90.0f);
	glVertex3f(-35.0f, 310.0f, -90.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.5f, 0.0f, 0.0f);
	glVertex3f(-35.0f, 310.0f, -100.0f);
	glVertex3f(-50.0f, 390.0f, -100.0f);
	glVertex3f(-50.0f, 390.0f, -90.0f);
	glVertex3f(-35.0f, 310.0f, -90.0f);
	glEnd();
}

void setMaterial(float ambientR, float ambientG, float ambientB,
	float diffuseR, float diffuseG, float diffuseB,
	float specularR, float specularG, float specularB, float shine)
{
	GLfloat ambient[] = { ambientR, ambientG, ambientB };
	GLfloat diffuse[] = { diffuseR, diffuseG, diffuseB };
	GLfloat specular[] = { specularR, specularG, specularB };

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shine);
}

void drawSpire(float x, float y, float z, float baseWidth, float depth, float height) {
	glColor3f(0.8f, 0.6f, 0.15f);

	// Base square - Platform of the Spire
	glBegin(GL_QUADS);
	glVertex3f(x - baseWidth / 2, y, z + depth / 2);  // Front-left corner
	glVertex3f(x + baseWidth / 2, y, z + depth / 2);  // Front-right corner
	glVertex3f(x + baseWidth / 2, y, z - depth / 2);  // Back-right corner
	glVertex3f(x - baseWidth / 2, y, z - depth / 2);  // Back-left corner
	glEnd();

	// Shaft of the spire - 4 Walls
	glColor3f(0.6f, 0.6f, 0.0f);

	// Front face
	glBegin(GL_QUADS);
	glVertex3f(x - baseWidth / 2, y, z + depth / 2);     // Bottom-left corner
	glVertex3f(x + baseWidth / 2, y, z + depth / 2);     // Bottom-right corner
	glVertex3f(x + baseWidth / 2, y + height, z + depth / 2); // Top-right corner
	glVertex3f(x - baseWidth / 2, y + height, z + depth / 2); // Top-left corner
	glEnd();

	// Back face
	glBegin(GL_QUADS);
	glVertex3f(x + baseWidth / 2, y, z - depth / 2);     // Bottom-right corner
	glVertex3f(x - baseWidth / 2, y, z - depth / 2);     // Bottom-left corner
	glVertex3f(x - baseWidth / 2, y + height, z - depth / 2); // Top-left corner
	glVertex3f(x + baseWidth / 2, y + height, z - depth / 2); // Top-right corner
	glEnd();

	// Left face
	glBegin(GL_QUADS);
	glVertex3f(x - baseWidth / 2, y, z - depth / 2);    // Bottom-left corner
	glVertex3f(x - baseWidth / 2, y, z + depth / 2);    // Bottom-right corner
	glVertex3f(x - baseWidth / 2, y + height, z + depth / 2); // Top-right corner
	glVertex3f(x - baseWidth / 2, y + height, z - depth / 2); // Top-left corner
	glEnd();

	// Right face
	glBegin(GL_QUADS);
	glVertex3f(x + baseWidth / 2, y, z + depth / 2);    // Bottom-left corner
	glVertex3f(x + baseWidth / 2, y, z - depth / 2);    // Bottom-right corner
	glVertex3f(x + baseWidth / 2, y + height, z - depth / 2); // Top-right corner
	glVertex3f(x + baseWidth / 2, y + height, z + depth / 2); // Top-left corner
	glEnd();

	// Spire - Top Triangle
	glColor3f(0.92f, 0.77f, 0.42f);

	// First triangular face
	glBegin(GL_TRIANGLES);
	glVertex3f(x, y + height + 10.0f, z);                // Apex of the spire
	glVertex3f(x - baseWidth / 2, y + height, z + depth / 2); // Bottom-left corner
	glVertex3f(x + baseWidth / 2, y + height, z + depth / 2); // Bottom-right corner
	glEnd();

	// Second triangular face
	glBegin(GL_TRIANGLES);
	glVertex3f(x, y + height + 10.0f, z);                // Apex of the spire
	glVertex3f(x + baseWidth / 2, y + height, z + depth / 2); // Bottom-left corner
	glVertex3f(x + baseWidth / 2, y + height, z - depth / 2); // Bottom-right corner
	glEnd();

	// Third triangular face
	glBegin(GL_TRIANGLES);
	glVertex3f(x, y + height + 10.0f, z);                // Apex of the spire
	glVertex3f(x + baseWidth / 2, y + height, z - depth / 2); // Bottom-left corner
	glVertex3f(x - baseWidth / 2, y + height, z - depth / 2); // Bottom-right corner
	glEnd();

	// Fourth triangular face
	glBegin(GL_TRIANGLES);
	glVertex3f(x, y + height + 10.0f, z);                // Apex of the spire
	glVertex3f(x - baseWidth / 2, y + height, z - depth / 2); // Bottom-left corner
	glVertex3f(x - baseWidth / 2, y + height, z + depth / 2); // Bottom-right corner
	glEnd();
}

void display() {
	glEnable(GL_SCISSOR_TEST);        // Enable scissor test
	glScissor(0, windowHeight / 2, windowWidth, windowHeight / 2); // x, y, width, height
	glClearColor(0.7f, 0.7f, 1.0f, 1.0f); // Blue sky background
	glClear(GL_COLOR_BUFFER_BIT);     // Clear only the scissored area
	glDisable(GL_SCISSOR_TEST);       // Disable scissor test

	// Clear bottom half with black 
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, windowWidth, windowHeight / 2); // Bottom half
	glClearColor(0.11f, 0.21f, 0.28f, 1.0f); // Sea blue background
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	glClear(GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();  // Reset the modelview each frame
	gluLookAt(
		0.0f, 10.0f, cameraPosition,
		shakeX * 8.0f, shakeY * 8.0f, -1.0f,
		0, 1, 0
	);

	// Lightning 
	if (lightningActive) {
		glPushMatrix();
		glDisable(GL_LIGHTING);

		// Main bright core
		glLineWidth(4.0f);
		glColor3f(0.0f, 0.0f, 0.6f);  // Dark blue core

		// Multiple jagged segments for realistic lightning
		float startX = 0.0f, startY = 700.0f, startZ = 50.0f;
		float endX = 0.0f, endY = Hy, endZ = 50.0f;

		glBegin(GL_LINE_STRIP);
		glVertex3f(startX, startY, startZ);

		// Create 4-6 jagged segments
		int segments = 5;
		for (int i = 1; i < segments; i++) {
			float t = (float)i / segments;
			float offsetX = (rand() % 40 - 20) * (1.0f - t);  // Less jitter near bottom
			float offsetZ = (rand() % 30 - 15) * (1.0f - t);
			float y = startY + (endY - startY) * t;
			glVertex3f(startX + offsetX, y, startZ + offsetZ);
		}

		glVertex3f(endX, endY, endZ);
		glEnd();

		// Outer glow
		glLineWidth(12.0f);
		glColor3f(0.0f, 0.0f, 0.6f); // Darker blue glow

		glBegin(GL_LINE_STRIP);
		glVertex3f(startX, startY, startZ);
		for (int i = 1; i < segments; i++) {
			float t = (float)i / segments;
			float offsetX = (rand() % 40 - 20) * (1.0f - t);
			float offsetZ = (rand() % 30 - 15) * (1.0f - t);
			float y = startY + (endY - startY) * t;
			glVertex3f(startX + offsetX, y, startZ + offsetZ);
		}
		glVertex3f(endX, endY, endZ);
		glEnd();

		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

	if (!exploded) {
		glPushMatrix();
		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);
		glScalef(2.0f, 2.0f, 2.0f);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.7f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			16.0f              // Shininess 
		);

		// Middle Layer
		drawSpire(-40.0f, 0.0f, 0.0f, 5.0f, 5.0f, 10.0f);
		drawSpire(-35.0f, 0.0f, -4.0f, 5.0f, 5.0f, 17.5f);
		drawSpire(-30.0f, 0.0f, -8.0f, 5.0f, 5.0f, 25.0f);
		drawSpire(-25.0f, 0.0f, -12.0f, 5.0f, 5.0f, 30.0f);
		drawSpire(-20.0f, 0.0f, -16.0f, 5.0f, 5.0f, 35.0f);
		drawSpire(-15.0f, 0.0f, -20.0f, 5.0f, 5.0f, 40.0f);
		drawSpire(-10.0f, 0.0f, -24.0f, 5.0f, 5.0f, 52.50f);
		drawSpire(-5.0f, 0.0f, -28.0f, 5.0f, 5.0f, 62.5f);
		drawSpire(0.0f, 0.0f, -32.0f, 5.0f, 5.0f, 72.5f);
		drawSpire(5.0f, 0.0f, -28.0f, 5.0f, 5.0f, 62.5f);
		drawSpire(10.0f, 0.0f, -24.0f, 5.0f, 5.0f, 52.5f);
		drawSpire(15.0f, 0.0f, -20.0f, 5.0f, 5.0f, 40.0f);
		drawSpire(20.0f, 0.0f, -16.0f, 5.0f, 5.0f, 35.0f);
		drawSpire(25.0f, 0.0f, -12.0f, 5.0f, 5.0f, 30.0f);
		drawSpire(30.0f, 0.0f, -8.0f, 5.0f, 5.0f, 25.0f);
		drawSpire(35.0f, 0.0f, -4.0f, 5.0f, 5.0f, 17.5f);
		drawSpire(40.0f, 0.0f, 0.0f, 5.0f, 5.0f, 10.0f);

		// Front Layer
		drawSpire(-30.0f, 0.0f, 4.0f, 5.0f, 5.0f, 2.5f);
		drawSpire(-25.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f);
		drawSpire(-20.0f, 0.0f, -4.0f, 5.0f, 5.0f, 10.0f);
		drawSpire(-15.0f, 0.0f, -8.0f, 5.0f, 5.0f, 15.0f);
		drawSpire(-10.0f, 0.0f, -12.0f, 5.0f, 5.0f, 20.0f);
		drawSpire(-5.0f, 0.0f, -16.0f, 5.0f, 5.0f, 30.0f);
		drawSpire(0.0f, 0.0f, -20.0f, 5.0f, 5.0f, 40.0f);
		drawSpire(5.0f, 0.0f, -16.0f, 5.0f, 5.0f, 30.0f);
		drawSpire(10.0f, 0.0f, -12.0f, 5.0f, 5.0f, 20.0f);
		drawSpire(15.0f, 0.0f, -8.0f, 5.0f, 5.0f, 15.0f);
		drawSpire(20.0f, 0.0f, -4.0f, 5.0f, 5.0f, 10.0f);
		drawSpire(25.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f);
		drawSpire(30.0f, 0.0f, 4.0f, 5.0f, 5.0f, 2.5f);

		// Frontmost Layer
		drawSpire(-15.0f, 0.0f, 2.5f, 5.0f, 5.0f, 2.5f);
		drawSpire(-10.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f);
		drawSpire(-5.0f, 0.0f, -2.5f, 5.0f, 5.0f, 10.0f);
		drawSpire(0.0f, 0.0f, -5.0f, 5.0f, 5.0f, 17.0f);
		drawSpire(5.0f, 0.0f, -2.5f, 5.0f, 5.0f, 10.0f);
		drawSpire(10.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f);
		drawSpire(15.0f, 0.0f, 2.5f, 5.0f, 5.0f, 2.5f);
		glPopMatrix();

		// Draw Sword on top of palace
		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);

		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		glTranslatef(-100.0f, 270.0f + swordLiftY + swordStrikeY, -120.0f);
		glRotatef(swordRotation, 0, 0, 1);

		setMaterial(
			0.8f, 0.5f, 0.2f,
			0.8f, 0.5f, 0.2f,
			0.5f, 0.5f, 0.5f,
			32.0f
		);
		glScalef(1.5f, 1.5f, 1.5f);
		drawSword();
		glPopMatrix();

		// Bifrost Bridge
		glPushMatrix();
		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			128.0f              // Shininess 
		);

		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glBegin(GL_QUADS);
		// Bottom face
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-6.0f, 0.0f, 2.0f);
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex3f(-6.0f, 0.0f, 600.0f);
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f(6.0f, 0.0f, 600.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(6.0f, 0.0f, 2.0f);
		// Top face (VIBGYOR right to left)
		glColor3f(0.0f, 0.0f, 0.8f); // Blue
		glVertex3f(6.0f, 0.5f, 2.0f);
		glColor3f(0.0f, 1.0f, 0.75f); // Cyan
		glVertex3f(6.0f, 0.5f, 600.0f);
		glColor3f(0.0f, 0.1f, 0.95f); // Blue
		glVertex3f(-6.0f, 0.5f, 600.0f);
		glColor3f(0.0f, 1.0f, 0.3f); // Green
		glVertex3f(-6.0f, 0.5f, 2.0f);
		// Front face
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-6.0f, 0.0f, 2.0f);
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex3f(6.0f, 0.0f, 2.0f);
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f(6.0f, 0.5f, 2.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-6.0f, 0.5f, 2.0f);
		// Back face
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-6.0f, 0.0f, 600.0f);
		glColor3f(0.29f, 0.0f, 0.51f);
		glVertex3f(6.0f, 0.0f, 600.0f);
		glColor3f(0.56f, 0.0f, 1.0f);
		glVertex3f(6.0f, 0.5f, 600.0f);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-6.0f, 0.5f, 600.0f);
		// Left face
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-6.0f, 0.0f, 2.0f);
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex3f(-6.0f, 0.0f, 600.0f);
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f(-6.0f, 0.5f, 600.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-6.0f, 0.5f, 2.0f);
		// Right face
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(6.0f, 0.0f, 2.0f);
		glColor3f(0.29f, 0.0f, 0.51f);
		glVertex3f(6.0f, 0.0f, 600.0f);
		glColor3f(0.56f, 0.0f, 1.0f);
		glVertex3f(6.0f, 0.5f, 600.0f);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(6.0f, 0.5f, 2.0f);
		glEnd();
		glPopMatrix();

		// Golden Platform under palace
		glPushMatrix();
		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		setMaterial(
			0.8f, 0.6f, 0.15f,  // Ambient (gold)
			0.9f, 0.7f, 0.2f,   // Diffuse (gold)
			1.0f, 0.9f, 0.5f,   // Specular (shiny gold)
			128.0f              // High shininess
		);

		glColor3f(1.0f, 0.84f, 0.0f);  // Gold color

		// Main platform
		glBegin(GL_QUADS);
		// Top face
		glVertex3f(-1000.0f, -2.0f, 100.0f);
		glVertex3f(1000.0f, -2.0f, 100.0f);
		glVertex3f(1000.0f, -2.0f, -80.0f);
		glVertex3f(-1000.0f, -2.0f, -80.0f);

		// Bottom face
		glVertex3f(-1000.0f, -5.0f, 100.0f);
		glVertex3f(1000.0f, -5.0f, 100.0f);
		glVertex3f(1000.0f, -5.0f, -80.0f);
		glVertex3f(-1000.0f, -5.0f, -80.0f);

		// Front face
		glVertex3f(-1000.0f, -5.0f, 100.0f);
		glVertex3f(1000.0f, -5.0f, 100.0f);
		glVertex3f(1000.0f, -2.0f, 100.0f);
		glVertex3f(-1000.0f, -2.0f, 100.0f);

		// Back face
		glVertex3f(-1000.0f, -5.0f, -80.0f);
		glVertex3f(1000.0f, -5.0f, -80.0f);
		glVertex3f(1000.0f, -2.0f, -80.0f);
		glVertex3f(-1000.0f, -2.0f, -80.0f);

		// Left face
		glVertex3f(-1000.0f, -5.0f, 100.0f);
		glVertex3f(-1000.0f, -5.0f, -80.0f);
		glVertex3f(-1000.0f, -2.0f, -80.0f);
		glVertex3f(-1000.0f, -2.0f, 100.0f);

		// Right face
		glVertex3f(1000.0f, -5.0f, 100.0f);
		glVertex3f(1000.0f, -5.0f, -80.0f);
		glVertex3f(1000.0f, -2.0f, -80.0f);
		glVertex3f(1000.0f, -2.0f, 100.0f);
		glEnd();
		glPopMatrix();

		// Hammer
		glPushMatrix();
		//Local Animation
		glTranslatef(0.0f, Hy, 0.0f);

		//Global Animation
		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex3f(-3.0f, 650.0f, 50.0f);
		glVertex3f(-3.0f, 650.0f, 53.0f);
		glVertex3f(3.0f, 650.0f, 53.0f);
		glVertex3f(3.0f, 650.0f, 50.0f);

		glVertex3f(-3.0f, 655.0f, 50.0f);
		glVertex3f(-3.0f, 655.0f, 53.0f);
		glVertex3f(3.0f, 655.0f, 53.0f);
		glVertex3f(3.0f, 655.0f, 50.0f);

		glVertex3f(-3.0f, 650.0f, 50.0f);
		glVertex3f(-3.0f, 655.0f, 50.0f);
		glVertex3f(-3.0f, 655.0f, 53.0f);
		glVertex3f(-3.0f, 650.0f, 53.0f);

		glVertex3f(3.0f, 650.0f, 50.0f);
		glVertex3f(3.0f, 655.0f, 50.0f);
		glVertex3f(3.0f, 655.0f, 53.0f);
		glVertex3f(3.0f, 650.0f, 53.0f);

		glVertex3f(-3.0f, 650.0f, 50.0f);
		glVertex3f(3.0f, 650.0f, 50.0f);
		glVertex3f(3.0f, 655.0f, 50.0f);
		glVertex3f(-3.0f, 655.0f, 50.0f);

		glVertex3f(-3.0f, 650.0f, 53.0f);
		glVertex3f(3.0f, 650.0f, 53.0f);
		glVertex3f(3.0f, 655.0f, 53.0f);
		glVertex3f(-3.0f, 655.0f, 53.0f);
		glEnd();

		glColor3f(0.66f, 0.33f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(-0.5f, 655.0f, 51.5f);
		glVertex3f(0.5f, 655.0f, 51.5f);
		glVertex3f(0.5f, 655.0f, 52.5f);
		glVertex3f(-0.5f, 655.0f, 52.5f);

		glVertex3f(-0.5f, 670.0f, 51.5f);
		glVertex3f(0.5f, 670.0f, 51.5f);
		glVertex3f(0.5f, 670.0f, 52.5f);
		glVertex3f(-0.5f, 670.0f, 52.5f);

		glVertex3f(-0.5f, 655.0f, 51.5f);
		glVertex3f(-0.5f, 670.0f, 51.5f);
		glVertex3f(-0.5f, 670.0f, 52.5f);
		glVertex3f(-0.5f, 655.0f, 52.5f);

		glVertex3f(0.5f, 655.0f, 51.5f);
		glVertex3f(0.5f, 670.0f, 51.5f);
		glVertex3f(0.5f, 670.0f, 52.5f);
		glVertex3f(0.5f, 655.0f, 52.5f);

		glVertex3f(-0.5f, 655.0f, 51.5f);
		glVertex3f(0.5f, 655.0f, 51.5f);
		glVertex3f(0.5f, 670.0f, 51.5f);
		glVertex3f(-0.5f, 670.0f, 51.5f);

		glVertex3f(-0.5f, 655.0f, 52.5f);
		glVertex3f(0.5f, 655.0f, 52.5f);
		glVertex3f(0.5f, 670.0f, 52.5f);
		glVertex3f(-0.5f, 670.0f, 52.5f);
		glEnd();
		glPopMatrix();

		// Surtur starting
		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);

		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		glTranslatef(0.0f, 380.0f, -125.0f);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			128.0f              // Shininess 
		);

		glColor3f(0.5f, 0.0f, 0.0f);
		glutSolidSphere(33.0, 50, 50);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);

		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			16.0f              // Shininess 
		);

		// Left Eye Socket - sunken
		glPushMatrix();
		glTranslatef(-13.0f, 365.0f, -88.0f);
		glScalef(1.2f, 1.0f, 0.8f);
		glColor3f(1.0f, 0.33f, 0.0f);
		glutSolidSphere(7.0, 30, 30);
		glPopMatrix();

		// Left Eye - solid dark red
		glPushMatrix();
		glTranslatef(-13.0f, 365.0f, -85.0f);
		glColor3f(1.0f, 0.33f, 0.0f);
		glutSolidSphere(5.5, 30, 30);
		glPopMatrix();

		// Left pupil - pitch black
		glPushMatrix();
		glTranslatef(-13.0f, 365.0f, -82.0f);
		glColor3f(1.0f, 0.33f, 0.0f);
		glutSolidSphere(2.5, 20, 20);
		glPopMatrix();

		// Right Eye Socket
		glPushMatrix();
		glTranslatef(13.0f, 365.0f, -88.0f);
		glScalef(1.2f, 1.0f, 0.8f);
		glColor3f(1.0f, 0.33f, 0.0f);
		glutSolidSphere(7.0, 30, 30);
		glPopMatrix();

		// Right Eye
		glPushMatrix();
		glTranslatef(13.0f, 365.0f, -85.0f);
		glColor3f(1.0f, 0.33f, 0.0f);
		glutSolidSphere(5.5, 30, 30);
		glPopMatrix();

		// Right pupil
		glPushMatrix();
		glTranslatef(13.0f, 365.0f, -82.0f);
		glColor3f(1.0f, 0.33f, 0.0f);
		glutSolidSphere(2.5, 20, 20);
		glPopMatrix();
		glPopMatrix();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);

		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			128.0f              // Shininess 
		);

		// Realistic Dark Orange Nose
		glPushMatrix();
		glColor3f(1.0f, 0.5f, 0.0f);

		// Nose bridge (upper part)
		glPushMatrix();
		glTranslatef(0.0f, 360.0f, -90.0f);
		glScalef(0.6f, 1.5f, 0.8f);
		glutSolidSphere(4.0, 20, 20);
		glPopMatrix();

		// Nose tip (lower part - main bulb)
		glPushMatrix();
		glTranslatef(0.0f, 350.0f, -88.0f);
		glScalef(1.2f, 1.0f, 1.3f);
		glutSolidSphere(4.5, 20, 20);
		glPopMatrix();
		glPopMatrix();

		glPushMatrix();
		glColor3f(0.1f, 0.05f, 0.0f);
		// Left side of frown (going DOWN)
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f(-15.0f, 325.0f, -82.0f);  // Left corner LOW
		glVertex3f(-15.0f, 323.0f, -82.0f);
		glVertex3f(-8.0f, 329.0f, -80.0f);   // Moving UP toward center
		glVertex3f(-8.0f, 327.0f, -80.0f);
		glVertex3f(0.0f, 333.0f, -78.0f);    // Center HIGH
		glVertex3f(0.0f, 331.0f, -78.0f);
		glEnd();

		// Right side of frown (going DOWN)
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f(0.0f, 333.0f, -78.0f);    // Center HIGH
		glVertex3f(0.0f, 331.0f, -78.0f);
		glVertex3f(8.0f, 329.0f, -80.0f);    // Moving DOWN from center
		glVertex3f(8.0f, 327.0f, -80.0f);
		glVertex3f(15.0f, 325.0f, -82.0f);   // Right corner LOW
		glVertex3f(15.0f, 323.0f, -82.0f);
		glEnd();

		// Upper lip
		glColor3f(0.8f, 0.4f, 0.0f);
		glBegin(GL_QUAD_STRIP);
		glVertex3f(-12.0f, 327.0f, -81.0f);
		glVertex3f(-12.0f, 329.0f, -81.0f);
		glVertex3f(-6.0f, 331.0f, -79.0f);
		glVertex3f(-6.0f, 333.0f, -79.0f);
		glVertex3f(6.0f, 331.0f, -79.0f);
		glVertex3f(6.0f, 333.0f, -79.0f);
		glVertex3f(12.0f, 332.0f, -81.0f);
		glVertex3f(12.0f, 334.0f, -81.0f);
		glEnd();
		glPopMatrix();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);
		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		glTranslatef(0.0f, 90.0f, 0.0f);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			128.0f              // Shininess 
		);

		drawSurtur();
		glPopMatrix();

		// Draw body under the sphere head
		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);
		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		glTranslatef(0.0f, 270.0f, -125.0f);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			128.0f              // Shininess 
		);

		glColor3f(0.5f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(-40.0f, -60.0f, 20.0f);
		glVertex3f(40.0f, -60.0f, 20.0f);
		glVertex3f(40.0f, 60.0f, 20.0f);
		glVertex3f(-40.0f, 60.0f, 20.0f);

		glVertex3f(-40.0f, -60.0f, -20.0f);
		glVertex3f(40.0f, -60.0f, -20.0f);
		glVertex3f(40.0f, 60.0f, -20.0f);
		glVertex3f(-40.0f, 60.0f, -20.0f);

		glVertex3f(-40.0f, -60.0f, -20.0f);
		glVertex3f(-40.0f, -60.0f, 20.0f);
		glVertex3f(-40.0f, 60.0f, 20.0f);
		glVertex3f(-40.0f, 60.0f, -20.0f);

		glVertex3f(40.0f, -60.0f, -20.0f);
		glVertex3f(40.0f, -60.0f, 20.0f);
		glVertex3f(40.0f, 60.0f, 20.0f);
		glVertex3f(40.0f, 60.0f, -20.0f);

		glVertex3f(-40.0f, 60.0f, -20.0f);
		glVertex3f(40.0f, 60.0f, -20.0f);
		glVertex3f(40.0f, 60.0f, 20.0f);
		glVertex3f(-40.0f, 60.0f, 20.0f);

		glVertex3f(-40.0f, -60.0f, -20.0f);
		glVertex3f(40.0f, -60.0f, -20.0f);
		glVertex3f(40.0f, -60.0f, 20.0f);
		glVertex3f(-40.0f, -60.0f, 20.0f);
		glEnd();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);
		// Left Hand
		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		glTranslatef(-60.0f, 285.0f, -125.0f);
		glRotatef(30.0f + armRotation, 0, 0, -1);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			128.0f              // Shininess 
		);

		glColor3f(0.5f, 0.0f, 0.0f);

		glBegin(GL_QUADS);
		glVertex3f(-10.0f, -50.0f, 10.0f);
		glVertex3f(10.0f, -50.0f, 10.0f);
		glVertex3f(10.0f, 50.0f, 10.0f);
		glVertex3f(-10.0f, 50.0f, 10.0f);

		glVertex3f(-10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, 50.0f, -10.0f);
		glVertex3f(-10.0f, 50.0f, -10.0f);

		glVertex3f(-10.0f, -50.0f, -10.0f);
		glVertex3f(-10.0f, -50.0f, 10.0f);
		glVertex3f(-10.0f, 50.0f, 10.0f);
		glVertex3f(-10.0f, 50.0f, -10.0f);

		glVertex3f(10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, -50.0f, 10.0f);
		glVertex3f(10.0f, 50.0f, 10.0f);
		glVertex3f(10.0f, 50.0f, -10.0f);

		glVertex3f(-10.0f, 50.0f, -10.0f);
		glVertex3f(10.0f, 50.0f, -10.0f);
		glVertex3f(10.0f, 50.0f, 10.0f);
		glVertex3f(-10.0f, 50.0f, 10.0f);

		glVertex3f(-10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, -50.0f, 10.0f);
		glVertex3f(-10.0f, -50.0f, 10.0f);
		glEnd();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);
		// Right Hand
		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		glTranslatef(60.0f, 285.0f, -125.0f);
		glRotatef(30.0f + armRotation, 0, 0, 1);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			128.0f              // Shininess 
		);

		glColor3f(0.5f, 0.0f, 0.0f);

		glBegin(GL_QUADS);
		glVertex3f(-10.0f, -50.0f, 10.0f);
		glVertex3f(10.0f, -50.0f, 10.0f);
		glVertex3f(10.0f, 50.0f, 10.0f);
		glVertex3f(-10.0f, 50.0f, 10.0f);

		glVertex3f(-10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, 50.0f, -10.0f);
		glVertex3f(-10.0f, 50.0f, -10.0f);

		glVertex3f(-10.0f, -50.0f, -10.0f);
		glVertex3f(-10.0f, -50.0f, 10.0f);
		glVertex3f(-10.0f, 50.0f, 10.0f);
		glVertex3f(-10.0f, 50.0f, -10.0f);

		glVertex3f(10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, -50.0f, 10.0f);
		glVertex3f(10.0f, 50.0f, 10.0f);
		glVertex3f(10.0f, 50.0f, -10.0f);

		glVertex3f(-10.0f, 50.0f, -10.0f);
		glVertex3f(10.0f, 50.0f, -10.0f);
		glVertex3f(10.0f, 50.0f, 10.0f);
		glVertex3f(-10.0f, 50.0f, 10.0f);

		glVertex3f(-10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, -50.0f, -10.0f);
		glVertex3f(10.0f, -50.0f, 10.0f);
		glVertex3f(-10.0f, -50.0f, 10.0f);
		glEnd();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);
		// Left Leg
		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		glTranslatef(-25.0f, 110.0f, -125.0f);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			128.0f              // Shininess 
		);

		glColor3f(0.5f, 0.0f, 0.0f);

		glBegin(GL_QUADS);
		glVertex3f(-15.0f, -110.0f, 10.0f);
		glVertex3f(15.0f, -110.0f, 10.0f);
		glVertex3f(15.0f, 110.0f, 10.0f);
		glVertex3f(-15.0f, 110.0f, 10.0f);

		glVertex3f(-15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, 110.0f, -10.0f);
		glVertex3f(-15.0f, 110.0f, -10.0f);

		glVertex3f(-15.0f, -110.0f, -10.0f);
		glVertex3f(-15.0f, -110.0f, 10.0f);
		glVertex3f(-15.0f, 110.0f, 10.0f);
		glVertex3f(-15.0f, 110.0f, -10.0f);

		glVertex3f(15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, -110.0f, 10.0f);
		glVertex3f(15.0f, 110.0f, 10.0f);
		glVertex3f(15.0f, 110.0f, -10.0f);

		glVertex3f(-15.0f, 110.0f, -10.0f);
		glVertex3f(15.0f, 110.0f, -10.0f);
		glVertex3f(15.0f, 110.0f, 10.0f);
		glVertex3f(-15.0f, 110.0f, 10.0f);

		glVertex3f(-15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, -110.0f, 10.0f);
		glVertex3f(-15.0f, -110.0f, 10.0f);
		glEnd();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(surturSideShift, surturEmergenceY, 0.0f);
		// Right Leg
		glTranslatef(Tx, Ty, cameraDistance);
		glRotatef(angle, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		glScalef(surturScale, surturScale, surturScale);

		glTranslatef(25.0f, 110.0f, -125.0f);

		setMaterial(
			0.8f, 0.5f, 0.2f,  // Ambient 
			0.8f, 0.5f, 0.2f,  // Diffuse 
			0.1f, 0.1f, 0.1f,  // Specular 
			128.0f              // Shininess 
		);

		glColor3f(0.5f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(-15.0f, -110.0f, 10.0f);
		glVertex3f(15.0f, -110.0f, 10.0f);
		glVertex3f(15.0f, 110.0f, 10.0f);
		glVertex3f(-15.0f, 110.0f, 10.0f);

		glVertex3f(-15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, 110.0f, -10.0f);
		glVertex3f(-15.0f, 110.0f, -10.0f);

		glVertex3f(-15.0f, -110.0f, -10.0f);
		glVertex3f(-15.0f, -110.0f, 10.0f);
		glVertex3f(-15.0f, 110.0f, 10.0f);
		glVertex3f(-15.0f, 110.0f, -10.0f);

		glVertex3f(15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, -110.0f, 10.0f);
		glVertex3f(15.0f, 110.0f, 10.0f);
		glVertex3f(15.0f, 110.0f, -10.0f);

		glVertex3f(-15.0f, 110.0f, -10.0f);
		glVertex3f(15.0f, 110.0f, -10.0f);
		glVertex3f(15.0f, 110.0f, 10.0f);
		glVertex3f(-15.0f, 110.0f, 10.0f);

		glVertex3f(-15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, -110.0f, -10.0f);
		glVertex3f(15.0f, -110.0f, 10.0f);
		glVertex3f(-15.0f, -110.0f, 10.0f);
		glEnd();
		glPopMatrix();
		// Surtur Ending
	}
	else {
		glPushMatrix();
		for (auto& p : pieces) {
			glPushMatrix();
			glTranslatef(p.x, p.y, p.z);

			if (p.type == 0) { // Spire
				glScalef(2.0f, 2.0f, 2.0f);
				setMaterial(0.8f, 0.5f, 0.2f, 0.7f, 0.5f, 0.2f, 0.1f, 0.1f, 0.1f, 16.0f);
				drawSpire(0, 0, 0, 5.0f, 5.0f, 5.0f);
			}
			else if (p.type == 1) { // Hammer
				glColor3f(1.0f, 1.0f, 1.0f);
				glutSolidCube(10.0);
			}
			else if (p.type == 2) { // Bridge piece
				glColor3f(0.5f, 0.5f, 1.0f);
				glutSolidCube(8.0);
			}
			else if (p.type == 3) { // Platform piece
				glColor3f(1.0f, 0.84f, 0.0f);
				glutSolidCube(12.0);
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
	glutSwapBuffers(); //Double buffering. For single buffering use glFlush instead of glutSwapBuffers
}

void initLight()
{
	GLfloat ambient[] = { 0.3f, 0.3f, 0.3f }; // Ambient light
	GLfloat diffuse[] = { 0.35f, 0.35f, 0.35f }; // Diffuse light
	GLfloat specular[] = { 0.05f, 0.05f, 0.05f }; // Specular light
	GLfloat position[] = { 0.0f, 5.0f, 40.0f };  // Light Position
	GLfloat position2[] = { 0.0f, 5.0f, -40.0f }; // Light Position

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT1, GL_POSITION, position2);

	GLfloat directionalLightPos[] = { 0.0f, 50.0f, 650.0f, 0.0f }; // w = 0 for directional
	GLfloat directionalLightDiffuse[] = { 0.7f, 0.7f, 0.7f };
	glLightfv(GL_LIGHT2, GL_POSITION, directionalLightPos);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, directionalLightDiffuse);

	glEnable(GL_LIGHTING); // Enable lighting 
	glEnable(GL_LIGHT0); // Enable light source - Light0
	glEnable(GL_LIGHT1); // Enable light source - Light1
	glEnable(GL_LIGHT2); // Enable light source - Light2
	glEnable(GL_COLOR_MATERIAL); // Enable material colour tracking
}

void keyboardEvents(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // Escape key
		exit(0);
		break;
	case 'A':
	case 'a':
		angleY -= 5.0f;
		break;
	case 'D':
	case 'd':
		angleY += 5.0f;
		break;
	case 'W':
	case 'w':
		angleX -= 5.0f;
		break;
	case 'S':
	case 's':
		angleX += 5.0f;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void arrowEvents(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		Ty -= 0.5f;
		break;
	case GLUT_KEY_DOWN:
		Ty += 0.5f;
		break;
	case GLUT_KEY_LEFT:
		Tx -= 0.5f;
		break;
	case GLUT_KEY_RIGHT:
		Tx += 0.5f;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void mouseWheelEvents(int wheel, int direction, int x, int y) {
	if (direction > 0) {
		cameraDistance += 2.0f;
	}
	else {
		cameraDistance -= 2.0f;
	}
	glutPostRedisplay(); // Request a redraw
}

void mouseDragEvents(int x, int y) {
	if (lastX >= 0 && lastY >= 0) {
		// Calculate the difference in mouse position
		int deltaX = x - lastX;
		int deltaY = y - lastY;

		// Update the angles based on mouse movement
		angleX += deltaY * 0.1f;
		angleY += deltaX * 0.1f;
	}

	// Update last mouse position
	lastX = x;
	lastY = y;

	glutPostRedisplay(); // Request a redraw
}

int main(int argC, char* argV[])
{
	glutInit(&argC, argV);

	//Initialize display mode
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	//Set ModelView matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Initialize window position and size
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(10, 10);

	//Create window with title
	glutCreateWindow("Ragnarok");

	//Enable Depth test and Lighting
	glEnable(GL_DEPTH_TEST);
	initLight();

	// Defining a callback function for looping into the rasterizer
	glutDisplayFunc(display);

	// Register the animation function for idle updates
	glutIdleFunc(animation);

	// Setup callbacks for keyboard and mouse events
	glutKeyboardFunc(keyboardEvents);

	// Setup callback for arrow events
	glutSpecialFunc(arrowEvents);

	// Setup callback for mouse click events
	// glutMouseFunc(mouseClickEvents);

	// Setup callback for mouse motion events
	glutMotionFunc(mouseDragEvents);

	// Setup callback for mouse wheel events
	glutMouseWheelFunc(mouseWheelEvents);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// glOrtho(-10, 10, -10, 10, -10, 10);
	gluPerspective(120, 1, 0.001, 1000);

	glMatrixMode(GL_MODELVIEW);

	glutMainLoop();
	return 0;
}