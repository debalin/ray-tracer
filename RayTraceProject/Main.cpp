#include <windows.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <iostream>
#include <unordered_map>
#include <time.h>
#include "RayTrace.h"
using namespace std;
using namespace glm;

/*********************************************************************************************************************
* Main funtion running the show!
*********************************************************************************************************************/

RayTrace rayTrace;
struct timeval parser_start;
struct timeval parser_end;

void renderWrapper() {

	rayTrace.renderScene();

}

void keyboard(unsigned char key, int x, int y) {
	switch(key) {
		case 27:
			exit( 0 );
			break;
		default:
			break;
	}
}

int main(int argc, char** argv) {

	clock_t time;
	double seconds;
	time = clock();
	rayTrace.calculateIntersections();
	time = clock() - time;
	seconds = time/(double)1000;
	
	cout << "Ray casting duration is " << seconds << " seconds." << endl;

	time = clock();
	rayTrace.calculateLighting();
	time = clock() - time;
	seconds = time/(double)1000;
	cout << "Lighting duration is " << seconds << " seconds." << endl;
	
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE );
	glutInitWindowSize(rayTrace.windowWidth, rayTrace.windowHeight);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Hello Adam!");

	glClearColor(0.f, 0.f, 0.f, 0.f);

	glutDisplayFunc(renderWrapper);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
