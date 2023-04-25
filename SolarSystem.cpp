#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

using namespace std;

typedef float point3[3];
static GLfloat viewer[] = { 0.0, 0.0, 0.0 };
GLfloat theta = 0.0;
GLfloat phi = 0.0;
GLfloat R = 100.0;
static GLfloat pix2angle;
static GLint status = 0;
static int x_pos_old = 0;
static int y_pos_old = 0;
static int z_pos_old = 0;
float location = 1.0;

static int deltaX = 0;
static int deltaY = 0;
static int deltaZ = 0;

float speedMultiplier = 1;
float speedRotationMultiplier = 1;

boolean elipse = 1;
int planetCurrent = 0;
boolean realPlanetSize = true;
int cameraType = 1;

float moon_speedAroundAxis = 1.0;
float moon_move_x = 0.0;
float moon_move_z = 0.0;

float speedAroundAxis[9] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 };

float planetAxisAngle[9] = { 0.00, 0.0, 2.0, 23.0, 25.0, 3.0, 26.0, 82.0, 28.0 };

float planetBaseRotationSpeed[9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

float speedAroundSun[9] = { 0.0, 0.0047, 0.0035, 0.0029, 0.0024, 0.0013, 0.0010, 0.0007, 0.0005 };

float planetMov[9][2] = {{0.0, 0.0},{0.0, 0.0},{0.0, 0.0},{0.0, 0.0},{0.0, 0.0},{0.0, 0.0},{0.0, 0.0},{0.0, 0.0},{0.0, 0.0}};

float radius[9] = { 696.0 / 100, 2.5 / 100 * 2, 6.0 / 100 * 2, 6.3 / 100 * 2, 3.3 / 100 * 2, 70.0 / 100, 60.0 / 100 , 25.0 / 100 , 24.0 / 100 };

float planetBaseRotationSpeedChanger[9] = { 6793.0 / 1000, 11.0 / 1000, 6.52 / 1000, 1670.0 / 1000, 868.0 / 1000, 45000.0 / 1000, 35000.0 / 1000, 9000.0 / 1000, 8000.0 / 1000 };

float bigRadius[9] = { 600.0 / 100, 4 * 2.5 / 100 * 2, 4 * 6.0 / 100 * 2, 4 * 6.3 / 100 * 2, 4 * 3.3 / 100 * 2, 4 * 70.0 / 100, 4 * 60.0 / 100 , 4 * 25.0 / 100 , 4 * 24.0 / 100 };

float planetPos[9][2] = { {0,0},{7.0 + 3.0, 7.0 + 4.6},{7.0 + 7.2, 7.0 + 7.3},{7.0 + 9.8, 7.0 + 10.1},{7.0 + 13.8, 7.0 + 16.8},{7.0 + 50.0, 7.0 + 54.0},{7.0 + 92.0, 7.0 + 99.5},{7.0 + 186.0 , 7.0 + 197.0 },{7.0 + 298.0 , 7.0 + 302.0 } };

GLbyte* LoadTGAImage(const char* FileName, GLint* ImWidth, GLint* ImHeight, GLint* ImComponents, GLenum* ImFormat);
void ChangeSize(GLsizei horizontal, GLsizei vertical);
float calculateSpeed(float speed);

void CalculatePlanetPos() {

	for (int i = 0; i < 9; i++) {

		planetMov[i][0] = cos(speedAroundAxis[i]);
		planetMov[i][1] = sin(speedAroundAxis[i]);
		speedAroundAxis[i] += speedAroundSun[i] * (cos(speedAroundAxis[i]) / 1.5 + 1) * calculateSpeed(speedMultiplier);
		if (i == 2 || i == 0) {
			planetBaseRotationSpeed[i] += planetBaseRotationSpeedChanger[i] * calculateSpeed(speedRotationMultiplier);
		}
		else
		{
			planetBaseRotationSpeed[i] -= planetBaseRotationSpeedChanger[i] * calculateSpeed(speedRotationMultiplier);
		}
		if (planetBaseRotationSpeed[i] > 360.0) {
			planetBaseRotationSpeed[i] = 0.0;
		}
		if (planetBaseRotationSpeed[i] < 0.0) {
			planetBaseRotationSpeed[i] = 359.0;
		}
	}

	if (realPlanetSize == true) {
		moon_move_x = planetMov[3][0] * (planetPos[3][0] + planetPos[3][1])
			- planetPos[3][1] + radius[3] * 3 * cos(moon_speedAroundAxis) - radius[3];
		moon_move_z = planetPos[3][1] * planetMov[3][1] + radius[3] * 3 * sin(moon_speedAroundAxis) / 1.5;
		moon_speedAroundAxis += (0.1 * cos(moon_speedAroundAxis) + 1) / 10 * calculateSpeed(speedMultiplier);
	}
	else {
		moon_move_x = planetMov[3][0] * (planetPos[3][0] + planetPos[3][1])
			- planetPos[3][1] + bigRadius[3] * 3 * cos(moon_speedAroundAxis) - bigRadius[3];
		moon_move_z = planetPos[3][1] * planetMov[3][1] + bigRadius[3] * 3 * sin(moon_speedAroundAxis) / 1.5;
		moon_speedAroundAxis += (0.1 * cos(moon_speedAroundAxis) + 1) / 10 * calculateSpeed(speedMultiplier);
	}

	if (moon_speedAroundAxis > 360)
		moon_speedAroundAxis = 0.1;

	glutPostRedisplay(); 
}


void Planet(int id, const char* texturePath) {

	if (id == 0) {
		GLfloat sun_light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat sun_light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat sun_light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat sun_light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glLightfv(GL_LIGHT0, GL_POSITION, sun_light_position);
		glLightfv(GL_LIGHT0, GL_AMBIENT, sun_light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, sun_light_specular);

		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);

		GLfloat sun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat sun_mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat sun_mat_specular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		GLfloat sun_mat_emission[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		GLfloat sun_mat_shininess = 0.0f;

		glMaterialfv(GL_FRONT, GL_AMBIENT, sun_mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, sun_mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, sun_mat_specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, sun_mat_emission);
		glMaterialf(GL_FRONT, GL_SHININESS, sun_mat_shininess);
	}

	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;
	pBytes = LoadTGAImage(texturePath, &ImWidth, &ImHeight, &ImComponents, &ImFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);
	glEnable(GL_TEXTURE_2D);

	GLUquadricObj* pObj = gluNewQuadric();

	gluQuadricDrawStyle(pObj, GLU_TRUE);
	gluQuadricTexture(pObj, GL_TRUE);

	glPushMatrix();

	glTranslatef(planetMov[id][0] * (planetPos[id][0] + planetPos[id][1]) - planetPos[id][1], 0, planetMov[id][1] * planetPos[id][1]);

	glRotatef(90 + planetAxisAngle[id], 1, 0, 0);
	glRotatef(planetBaseRotationSpeed[id], 0, 0, 1 - planetAxisAngle[id]);

	if (realPlanetSize == true) {
		gluSphere(pObj, radius[id], 100, 100);
	}
	else {
		gluSphere(pObj, bigRadius[id], 100, 100);
	}
	
	gluDeleteQuadric(pObj);
	glPopMatrix();

	if (elipse) {
		glBegin(GL_LINE_STRIP);
		for (float i = 0.1; i < 20.0; i += 0.1) {
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(cos(i) * (planetPos[id][0] + planetPos[id][1]) - planetPos[id][1], 0, planetPos[id][1] * sin(i));
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
}

void Moon(const char* texturePath) {
	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;
	pBytes = LoadTGAImage(texturePath, &ImWidth, &ImHeight, &ImComponents, &ImFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);
	glEnable(GL_TEXTURE_2D);

	GLUquadricObj* pObj = gluNewQuadric();

	gluQuadricDrawStyle(pObj, GLU_TRUE);
	gluQuadricTexture(pObj, GL_TRUE);

	glPushMatrix();

	glTranslatef(moon_move_x, 0, moon_move_z);
	glRotatef(90, 1, 0, 0);
	glRotatef(planetBaseRotationSpeed[3] / 2, 0, 0, 1);
	if (realPlanetSize == true) {
		gluSphere(pObj, 0.040, 100, 100);
	}
	else {
		gluSphere(pObj, 0.040*3, 100, 100);
	}
	
	gluDeleteQuadric(pObj);
	glPopMatrix();

	if (elipse) {
		glBegin(GL_LINE_STRIP);
		for (float i = 0.1; i < 360.0; i += 0.1) {
			glColor3f(1.0, 1.0, 1.0);
			if (realPlanetSize == true) {
				glVertex3f(planetMov[3][0] * (planetPos[3][0] + planetPos[3][1]) - planetPos[3][1] + radius[3] * 3 * cos(i) - radius[3], 0, planetPos[3][1] * planetMov[3][1] + radius[3] * 3 * sin(i) / 1.5);
			}
			else {
				glVertex3f(planetMov[3][0] * (planetPos[3][0] + planetPos[3][1]) - planetPos[3][1] + bigRadius[3] * 3 * cos(i) - bigRadius[3], 0, planetPos[3][1] * planetMov[3][1] + bigRadius[3] * 3 * sin(i) / 1.5);
			}
			
		}
		glEnd();
	}


	glDisable(GL_TEXTURE_2D);
}

void Mouse(int btn, int state, int x, int y)
{
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		x_pos_old = x;
		y_pos_old = y;
		status = 1;
	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		y_pos_old = y;
		status = 2;
	}
	else
		status = 0;
}

void Motion(GLsizei x, GLsizei y)
{
	deltaX = x - x_pos_old;
	deltaY = y - y_pos_old;
	x_pos_old = x;
	y_pos_old = y;
	glutPostRedisplay();
}


void RenderScene(void)
{
	
	if (R + deltaY * pix2angle / 3.0 < 2 * radius[planetCurrent]) {
		if (realPlanetSize == true) {
			R = 2 * radius[planetCurrent];
		}
		else {
			R = 2 * bigRadius[planetCurrent];
		}
	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if (status == 1) {

		theta += deltaX * pix2angle / 60.0;
		phi += deltaY * pix2angle / 60;
		if (phi > 2 * M_PI)
			phi = 0;

		if (phi < 0)
			phi = 2 * M_PI;

		if (phi > M_PI / 2)
			location = -1.0;
		else
			location = 1.0;

		if (phi > M_PI + (M_PI / 2))
			location = 1.0;
	}
	else if (status == 2)
	{	
		if (realPlanetSize == true) {
			if (R + deltaY * pix2angle / 3.0 < 2 * radius[planetCurrent]) {
				R = 2 * radius[planetCurrent];
			}
			else {
				R += deltaY * pix2angle / 3.0;
			}
		}
		else if(realPlanetSize == false) {
			if (R + deltaY * pix2angle / 3.0 < 2 * bigRadius[planetCurrent]) {
				R = 2 * bigRadius[planetCurrent];
			}
			else {
				R += deltaY * pix2angle / 3.0;
			}
		}
	}

	if (cameraType) {
		viewer[0] = R * cos(theta) * cos(phi);
		viewer[1] = R * sin(phi);
		viewer[2] = R * sin(theta) * cos(phi);

		gluLookAt(planetMov[planetCurrent][0] * (planetPos[planetCurrent][0] + planetPos[planetCurrent][1]) - planetPos[planetCurrent][1] + viewer[0], viewer[1], planetPos[planetCurrent][1] * planetMov[planetCurrent][1] + viewer[2],
			planetMov[planetCurrent][0] * (planetPos[planetCurrent][0] + planetPos[planetCurrent][1]) - planetPos[planetCurrent][1], 0.0, planetPos[planetCurrent][1] * planetMov[planetCurrent][1],
			0.0, location, 0.0);
	}
	else {
		viewer[0] = R * cos(theta) * cos(phi);
		viewer[1] = R * sin(phi);
		viewer[2] = R * sin(theta) * cos(phi);

		if (realPlanetSize == true) {
			gluLookAt(planetMov[planetCurrent][0] * (planetPos[planetCurrent][0] + planetPos[planetCurrent][1]) - planetPos[planetCurrent][1],
				radius[planetCurrent] * 2,
				planetMov[planetCurrent][1] * planetPos[planetCurrent][1],
				planetMov[planetCurrent][0] * (planetPos[planetCurrent][0] + planetPos[planetCurrent][1]) - planetPos[planetCurrent][1] + viewer[0],
				-viewer[1],
				planetPos[planetCurrent][1] * planetMov[planetCurrent][1] + viewer[2],
				0.0, location, 0.0);
		}
		else {
			gluLookAt(planetMov[planetCurrent][0] * (planetPos[planetCurrent][0] + planetPos[planetCurrent][1]) - planetPos[planetCurrent][1],
				bigRadius[planetCurrent] * 2,
				planetMov[planetCurrent][1] * planetPos[planetCurrent][1],
				planetMov[planetCurrent][0] * (planetPos[planetCurrent][0] + planetPos[planetCurrent][1]) - planetPos[planetCurrent][1] + viewer[0],
				-viewer[1],
				planetPos[planetCurrent][1] * planetMov[planetCurrent][1] + viewer[2],
				0.0, location, 0.0);
		}
	}

	Planet(0, "sun.tga");
	Planet(1, "mercury.tga");
	Planet(2, "venus.tga");
	Planet(3, "earth.tga");
	Planet(4, "mars.tga");
	Planet(5, "jupiter.tga");
	Planet(6, "saturn.tga");
	Planet(7, "uranus.tga");
	Planet(8, "neptune.tga");
	Moon("moon.tga");
	glFlush();
	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
	if (key == 'e') elipse = !elipse;
	if (key == 's') speedMultiplier += 1;
	if (key == 'w') speedMultiplier -= 1;
	if (key == 'r') speedRotationMultiplier += 1;
	if (key == 'f') speedRotationMultiplier -= 1;
	if (key == '0') planetCurrent = 0;
	if (key == '1') planetCurrent = 1;
	if (key == '2') planetCurrent = 2;
	if (key == '3') planetCurrent = 3;
	if (key == '4') planetCurrent = 4;
	if (key == '5') planetCurrent = 5;
	if (key == '6') planetCurrent = 6;
	if (key == '7') planetCurrent = 7;
	if (key == '8') planetCurrent = 8;
	if (key == 'k') cameraType = !cameraType;
	if (key == 'a') realPlanetSize = !realPlanetSize;

	RenderScene();
}

void MyInit(void)
{
	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;

	glEnable(GL_CULL_FACE);

	pBytes = LoadTGAImage("D1_t.tga", &ImWidth, &ImHeight, &ImComponents, &ImFormat);

	glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);

	free(pBytes);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void Menu() {
	cout << "	SYSTEM CONTROL\n";
	cout << " 		e - toggle orbit view\n";
	cout << " 		a - toggle larger planets\n";
	cout << " 		s - increase planet rotation speed around the Sun\n";
	cout << " 		w - decrease planet rotation speed around the Sun\n";
	cout << " 		r - increase planet rotation speed around its own axis\n";
	cout << " 		f - decrease planet rotation speed around its own axis\n\n";
	cout << "	CAMERA\n";
	cout << "		LMB - moving camera\n";
	cout << "		RMB - camera zoom\n";
	cout << "		k - second kamera mode\n";
	cout << "			default - camera on choosen planet\n";
	cout << "			second mode - camera from planet pov\n";
	cout << "			(sun is choosen by default)\n\n";
	cout << "	PLANET NUMBER\n";
	cout << "	to choose a planet select a number\n";
	cout << "		0 - Sun\n";
	cout << "		1 - Mercury\n";
	cout << "		2 - Venus\n";
	cout << "		3 - Earth\n";
	cout << "		4 - Mars\n";
	cout << "		5 - Jupiter\n";
	cout << "		6 - Saturn\n";
	cout << "		7 - Uranus\n";
	cout << "		8 - Neptune\n";
}

void main(void)
{
	Menu();
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Solar System");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	MyInit();
	glutIdleFunc(CalculatePlanetPos);
	glEnable(GL_DEPTH_TEST);
	glutKeyboardFunc(keys);
	glutMainLoop();
}

GLbyte* LoadTGAImage(const char* FileName, GLint* ImWidth, GLint* ImHeight, GLint* ImComponents, GLenum* ImFormat){
	#pragma pack(1)           
	typedef struct{
		GLbyte    idlength;
		GLbyte    colormaptype;
		GLbyte    datatypecode;
		unsigned short    colormapstart;
		unsigned short    colormaplength;
		unsigned char     colormapdepth;
		unsigned short    x_orgin;
		unsigned short    y_orgin;
		unsigned short    width;
		unsigned short    height;
		GLbyte    bitsperpixel;
		GLbyte    descriptor;
	}TGAHEADER;
	#pragma pack(8)

	FILE* pFile;
	TGAHEADER tgaHeader;
	unsigned long lImageSize;
	short sDepth;
	GLbyte* pbitsperpixel = NULL;

	*ImWidth = 0;
	*ImHeight = 0;
	*ImFormat = GL_BGR_EXT;
	*ImComponents = GL_RGB8;

	fopen_s(&pFile, FileName, "rb");
	if (pFile == NULL)return NULL;

	fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);

	*ImWidth = tgaHeader.width;
	*ImHeight = tgaHeader.height;
	sDepth = tgaHeader.bitsperpixel / 8;

	if (tgaHeader.bitsperpixel != 8 && tgaHeader.bitsperpixel != 24 && tgaHeader.bitsperpixel != 32)return NULL;

	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

	pbitsperpixel = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));

	if (pbitsperpixel == NULL)return NULL;

	if (fread(pbitsperpixel, lImageSize, 1, pFile) != 1)
	{
		free(pbitsperpixel);
		return NULL;
	}

	switch (sDepth) {
	case 3:
		*ImFormat = GL_BGR_EXT;
		*ImComponents = GL_RGB8;
		break;
	case 4:
		*ImFormat = GL_BGRA_EXT;
		*ImComponents = GL_RGBA8;
		break;
	case 1:
		*ImFormat = GL_LUMINANCE;
		*ImComponents = GL_LUMINANCE8;
		break;
	};

	fclose(pFile);

	return pbitsperpixel;
}

void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
	pix2angle = 360.0 / (float)horizontal;
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(70, 1.0, 0.001, 3000.0);


	if (horizontal <= vertical)
		glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

	else
		glViewport((horizontal - vertical) / 2, 0, vertical, vertical);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
}

float calculateSpeed(float speed) {
	if (speed < 0.0) {
		speed *= -1;
		speed = fabs(speed) / 10;
		if (speed > 1.0) {
			speed = 0.9999;
		}
	}
	return speed;
}