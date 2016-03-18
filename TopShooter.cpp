#include <iostream>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include "imageloader.h"
#include <string.h>
#include "vec3f.h"
#define pi 3.14
#define DEG2RAD(x) x*pi/180

using namespace std;
				
int Score=10;
int isLaunched=0;
float power=0;

void printNumber(int sc) {
	char string[10];

    glColor3f(0,0,0);
    if(sc==0)
        return;
    printNumber(sc/10);
    glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, '0'+sc%10);
}

//Represents a terrain, by storing a set of heights and normals at 2D locations
class Terrain {
	private:
		int w; //Width
		int l; //Length
		float** hs; //Heights
		Vec3f** normals;
		bool computedNormals; //Whether normals is up-to-date

	public:
		Terrain(int w2, int l2) {
			w = w2;
			l = l2;
			
			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}
			
			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}
			
			computedNormals = false;
		}
		
		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;
			
			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}
		
		int width() {
			return w;
		}
		
		int length() {
			return l;
		}
		
		//Sets the height at (x, z) to y
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}
		
		//Returns the height at (x, z)
		float getHeight(int x, int z) {
			return hs[z][x];
		}
		
		//Computes the normals, if they haven't been computed yet
		void computeNormals() {
			if (computedNormals) {
				return;
			}
			
			//Compute the rough version of the normals
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}
			
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);
					
					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}
					
					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}
					
					normals2[z][x] = sum;
				}
			}
			
			//Smooth out the normals
			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];
					
					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}
					
					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}	
					normals[z][x] = sum;
				}
			}
			
			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;
			
			computedNormals = true;
		}
		
		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};
			
int flag=0,camera_flag=3;
//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(int x = 0; x < image->width; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}
	delete image;
	t->computeNormals();
	return t;
}	

Terrain* _terrain;
class Lattu {
    public:
	   float	vel_x;
		float	vel_z;
		float   vel_y;
		float lattu_x;
		float lattu_y;
		float lattu_z;
		float Launching_Angle;
	Lattu() {
		vel_x=0;
		vel_y=0;
		vel_z=0;
		lattu_x=3;
		lattu_z=4;
		lattu_y=0;
		Launching_Angle=90;
	}

  	void Apply_Gravity() {
    	Vec3f normal=_terrain->getNormal(lattu_x,lattu_z).normalize();
		float cosa=(-normal[1]);
		float sina=sqrt(1- cosa*cosa);
		lattu_x+=-0.005*cosa*normal[0]/2*0.01+vel_x,lattu_z+=-0.005*cosa*normal[2]/2*0.01+vel_z,lattu_y+=-0.005*sina*sina/2*0.01+vel_y;
		
		vel_x+=-0.005*cosa*normal[0],vel_z+=-0.005*cosa*normal[2],vel_y+=-0.005*(sina*sina);
	}

  	int CheckBoundary() {

	  	if(lattu_z>57 || lattu_z < 0) {
			vel_z*=-1;
			return 1;
		}
			
		if(lattu_x>57 || lattu_x < 0) {
				vel_x*=-1;
				return 1;
				//lattu_x=30;
		}
		return 0;
	}

  	void Apply_Velocity() {

	  	vel_x+=0.4*power/3*cos(DEG2RAD(Launching_Angle));
	  	vel_z+=-0.4*power/3*sin(DEG2RAD(Launching_Angle));
	}

  	void Apply_Friction() {

	  	if(vel_x*vel_x+vel_z*vel_z< 0.0001)
	  		vel_x=0,vel_z=0,isLaunched=0,lattu_x=3,lattu_z=4,Score-=10;
	  	
	  	vel_x*=0.99;
	  	vel_z*=0.99;
	}

};


Lattu _lattu;

class target {
	public:
		int x;
		int z;
		target() {	
			x=rand()%60;
			z=rand()%60;
		}

		void checkCollision() {
			if((_lattu.lattu_x-x)*(_lattu.lattu_x-x)+(_lattu.lattu_z-z)*(_lattu.lattu_z -z) < 5)
				x=rand()%60,z=rand()%60,Score+=10,isLaunched=0;
		}
		
};

target _Target;
float _angle = 60.0f;
				
float rotate_angle=0.0f;

void cleanup() {
	delete _terrain;
}

void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: //Escape key
			cleanup();
			exit(0);		
			 
	}

	if(key==' ' && isLaunched==0)
		_lattu.Apply_Velocity(),isLaunched=1;
	if(key>50 && key!='p')
		camera_flag++,camera_flag=camera_flag%4;
	if(key=='p')
		rotate_angle++;

}

void handleKeypress2(int key, int x, int y) {

    if (key == GLUT_KEY_LEFT)
        _lattu.lattu_z-=0.1;
    if (key == GLUT_KEY_RIGHT) 
        _lattu.lattu_z+=0.1;
    if(key==GLUT_KEY_DOWN)
    	_lattu.Launching_Angle--;
    if(key==GLUT_KEY_UP)
    	_lattu.Launching_Angle++;
}


void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}

void drawTarget() {
	
	float bwheelrad = 1.4f;
	//glRotatef(90,1,0,1);
	glPushMatrix();
	glColor3f(1.0,0.0,0.2);
	glBegin(GL_TRIANGLE_FAN);
	for(int i = 0; i < 360;i++) { 
		glVertex2f((bwheelrad * cos(DEG2RAD(i))), (bwheelrad * sin(DEG2RAD(i))));
	}

	glColor3f(1.0,1.0,1.0);
	glBegin(GL_TRIANGLE_FAN);
	for(int i = 0; i < 360;i++) { 
		glVertex2f(
	            (bwheelrad/2 * cos(DEG2RAD(i))), 
		    (bwheelrad/2 * sin(DEG2RAD(i)))
		);
	}

	glColor3f(1.0,0.0,0.2);
	glBegin(GL_TRIANGLE_FAN);
	for(int i = 0; i < 360;i++) { 
		glVertex2f(
	            (bwheelrad/4 * cos(DEG2RAD(i))), 
		    (bwheelrad/4 * sin(DEG2RAD(i)))
		);
	}
	glColor3f(1.0,1.0,1.0);
	glBegin(GL_TRIANGLE_FAN);
	for(int i = 0; i < 360;i++) { 
		glVertex2f(
	            (bwheelrad/8 * cos(DEG2RAD(i))), 
		    (bwheelrad/8 * sin(DEG2RAD(i)))
		);
	}			
	glEnd();
	glPopMatrix();
}

               			

void drawLattu() {

	glPushMatrix();
	//glTranslatef(0,0,2);
	glColor3f(1.0, 0.0, 0.0);
	glutSolidTorus( 0.3,0.5, 100,100);

	glPushMatrix();
	glTranslatef(0,0,-0.3);
	glRotatef(180,0,1,0);
	glutSolidCone(0.2, 0.4, 10, 2);
	glPopMatrix();

	glColor3f(1.0,1.0,0.0);
	glutSolidTorus(0.1,0.8,25,30);
	glColor3f(0,0,0);
	glutWireTorus( 0.1,0.8, 15, 60);

	glColor3f(1.0, 0.5, 0.0);

	glPushMatrix();
	glTranslatef(0,0,-0.3);
	glutSolidTorus( 0.2,0.2, 25, 30);
	glColor3f(0,0,0);
	glutWireTorus( 0.2,0.2, 15, 60);
	glPopMatrix();

	float i;
	for(i=45;i<90;i++) {
		glPushMatrix();
		glTranslatef( 0.0 , 0 ,0.5);
		glRotatef(i,0,0,1);
		glRotatef(90,0,1,0);
		glColor3f(0.0, 0.0, 0.0); 
		glScalef(1.2 , 0.2, 0.2);
		glutSolidCube(1.0);
		glColor3f(0.9, 0.8, 0.0); 
		glutWireCube(1.0);
		glPopMatrix();
	}

	glPopMatrix();
}

void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Vec3f normal=_terrain->getNormal(_lattu.lattu_x,_lattu.lattu_z).normalize();

	glTranslatef(0.0f, 0.0f, -6.0f);	
	char string[10];
	glPushMatrix();
	glTranslatef(-1.0,1.1,-2.0);
 	strcpy(string,"Score: ");
 	glColor3f(1.0,1.0,0.0);
    glRasterPos3f(0,0,5);
    for(int j=0;j<8;j++)
    	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,string[j]);
    		
	if(Score<0)
        glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, '-');
    if(Score)
	    printNumber(abs(Score));
	else
        glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, '0');

	glPopMatrix();
			
	if(camera_flag==1)
		glRotatef(40,1,0,0);

	if(camera_flag==2)
		glTranslatef(0.0,2.5,2.0);

	GLfloat ambientColor[] = {1.4f, 0.4f, 0.4f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
	
	GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	
	float scale = 5.0f / max(_terrain->width() - 1, _terrain->length() - 1);
	if(camera_flag == 0) {
	 gluLookAt(
		 (_lattu.lattu_x -30)*scale - 0.5, _terrain->getHeight(_lattu.lattu_x,_lattu.lattu_z)*scale + 2 , (_lattu.lattu_z-30)*scale,
		 (_lattu.lattu_x -30)*scale, _terrain->getHeight(_lattu.lattu_x,_lattu.lattu_z)*scale + 2 , (_lattu.lattu_z-30)*scale,
		 0,1,0
		 );
	 }
	 
 	if(camera_flag==3) {
	 	gluLookAt(
		(_lattu.lattu_x -30)*scale , (_terrain->getHeight(_lattu.lattu_x,_lattu.lattu_z)+1)*scale +1 , (_lattu.lattu_z-30)*scale,
		(_Target.x -30)*scale, _terrain->getHeight(_Target.x,_Target.z)*scale , (_Target.z-30)*scale,
		0,1,0
		);
 	}

	glScalef(scale, scale, scale);
	glTranslatef(
		-(float)(_terrain->width() - 1) / 2,
		0.0f,
		-(float)(_terrain->length() - 1) / 2
		);
	
	if(camera_flag==1) {
		glTranslatef(0,0,(float)(_terrain->length() - 1) / 2+20);
		gluLookAt(
                0,0,0,
               	5,0,0,
                0,1,0
                );
				glBegin(GL_LINES);				
	}

	else if(camera_flag==2) {
		gluLookAt(
            0,1,0,
            0,-1,0,
            0,0,-1
            );
	}


  
	for(int j=0;j<(int)power;j++) {
		
		for(int i=0;i<90;i++) {
			glPushMatrix();
			//glRotatef(90,0,1,0);
			glTranslatef( 62 , 4 ,50 - j*5);
			glRotatef(i,0,0,1);
			glRotatef(90,0,1,0);
			glColor3f(0.0, 0.0, 0.0); 
			glScalef(1 , 1, 1);
			glutSolidCube(3.0);
			glColor3f(0.9, 0.8, 0.0); 
			glutWireCube(3.0);
			glPopMatrix();
		}

	}

			
	glPushMatrix();
						
	for(int z = 0; z < _terrain->length() - 1; z++) {
		glColor3f(0, 0.4, 0.0f);
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < _terrain->width(); x++) {
			Vec3f normal = _terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _terrain->getHeight(x, z), z);
			normal = _terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
		
	}
	
	glPushMatrix();		
	glTranslatef(_Target.x,_terrain->getHeight(_Target.x,_Target.z)+2,_Target.z);
	glRotatef(90,0,1,0);
	//glRotatef(_angle,0,1,0);
	drawTarget();
	glPopMatrix();	
	normal=_terrain->getNormal(_lattu.lattu_x,_lattu.lattu_z).normalize();
	glTranslatef(_lattu.lattu_x, _terrain->getHeight(_lattu.lattu_x,_lattu.lattu_z) , _lattu.lattu_z);
	glPushMatrix();
	glRotatef(_lattu.Launching_Angle,0,1,0);
	glBegin(GL_LINES);
	glColor3f(1.0,0.0,0.0);
	glLineWidth(2);
    glVertex3f( 10,2,0);
   	glVertex3f( -10,-2,0);
    glEnd();
    glPopMatrix();

	glRotatef(-acos(normal[2])*180/pi,normal[1],-normal[0],0);
	glTranslatef(0,0,2);
	glScalef(2,2,2);
	glRotatef(_angle,0,0,1);
	drawLattu();
	glPopMatrix();
	glEnd();
	glutSwapBuffers();
	
}

void update(int value) {
	_angle += 0.01f;
	if (_angle > 360) {
		_angle -= 360;
	}

	if(isLaunched) {
		_lattu.CheckBoundary();
		_lattu.Apply_Gravity();
		_lattu.Apply_Friction();
		_Target.checkCollision();
	}	
					
	else
		power+=0.1,power=(power>7)?0:power;
	glutPostRedisplay();
	glutTimerFunc(20, update, 0);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	
	glutCreateWindow("Top Shooter");
	initRendering();
	_terrain = loadTerrain("heightmap.bmp", 20);
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleKeypress2);
	glutReshapeFunc(handleResize);
	glutTimerFunc(25, update, 0);
	glutMainLoop();
	return 0;
}
