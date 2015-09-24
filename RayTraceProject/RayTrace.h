#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "glm/gtx/intersect.hpp"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "glut/glut.h"
using namespace std;
using namespace glm;

#define MTL_HASHMAP std::unordered_map<string, MTL *>

class RayTrace {

private:
	static bool compareNoCase(string first, string second);

public:
	struct ShaderInfo {
		GLuint programId;
		GLuint vertexArray;
		GLuint vertexBuffer;
	};
	struct Triangle {
		int vertexIndices[3];
		int normalIndices[3];
		glm::vec3 faceNormal;
		string groupName;
	};
	struct Intersection {
		glm::vec3 vertexIntesect;
		glm::vec3 baryIntersect;
		float intersectDistance;
		int triangleIndex;
	};
	struct Light {
		glm::vec3 location;
		GLfloat la;
		GLfloat ls;
		GLfloat ld;
	};
	struct MTL {
		glm::vec3 ka;
		glm::vec3 ks;
		glm::vec3 kd;
		float N;
	};
	struct PixelInfo {
		float r;
		float g;
		float b;
	};

	std::vector<glm::vec3> verticesVec;
	std::vector<glm::vec3> normalsVec;
	std::vector<Triangle *> triangles;
	std::vector<Light *> lights;
	glm::vec3 eyeLocation;
	GLfloat windowz;
	int windowHeight;
	int windowWidth;
	PixelInfo *pixels;
	MTL_HASHMAP materials;
	Intersection ***intersections;
	bool vnPresent, smoothingOpt;

	RayTrace(void);
	int parseOBJMTL(string objPath);
	int parseInterfaceWindow(string windowPath);
	int parseEyeLocation();
	int parseLightSources(string lightPath);
	int parseSmoothingOpt(string smoothingPath);
	void calculateIntersections();
	void calculateLighting();
	void renderScene();

};

