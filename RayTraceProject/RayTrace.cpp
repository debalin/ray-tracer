#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include "RayTrace.h"

/*********************************************************************************************************************
* Ray tracing example. Not much to do with OpenGL.
*********************************************************************************************************************/

#define MTL_HASHMAP std::unordered_map<string, MTL *>

RayTrace::RayTrace(void) {

	vnPresent = false;
	clock_t time = clock();
	if (parseSmoothingOpt("files/smoothing.txt") != 0) {
		cout << "Error in reading smoothing option. Vertex normals won't be calculated if they are absent." << endl;
		smoothingOpt = false;
	}
	if (parseOBJMTL("files/input.obj") != 0) {
		cout << "Error occurred while parsing." << endl;
		exit(1);
	}
	if (parseInterfaceWindow("files/window.txt") != 0) {
		cout << "Error in reading interface window height and width. Default values of 512x512 taken." << endl;
		windowHeight = 512;
		windowWidth = 512;
	}
	if (parseEyeLocation() != 0) {
		cout << "Error in reading eye location. Default values of [0, 0, -2] taken." << endl;
		eyeLocation.x = 0;
		eyeLocation.y = 0;
		eyeLocation.z = -2;
	}
	if (parseLightSources("files/lights.txt") != 0) {
		cout << "Error in reading light sources. Default values of [0, 5, 0] and [1, 1, 1] taken." << endl;
		Light *light = new Light();
		light->location.x = 0.0;
		light->location.y = 5.0;
		light->location.z = 0.0;
		light->la = 1.0;
		light->ls = 1.0;
		light->ld = 1.0;
		lights.push_back(light);
	}
	time = clock() - time;
	double seconds = time/(double)1000;
	cout << "Parse duration is " << seconds << " seconds." << endl;
	intersections = new Intersection**[windowWidth];
	for(int i = 0; i < windowWidth; ++i)
		intersections[i] = new Intersection*[windowHeight];
	windowz = -1;
	pixels = new PixelInfo[(windowWidth * windowHeight)];
	memset(pixels, 0, sizeof(PixelInfo) * windowWidth * windowHeight);

}

bool RayTrace::compareNoCase(string first, string second) {

	if (first.size() != second.size()) {
		return (false);
	}
	else {
		for (size_t i = 0; i < first.size(); i++) {
			if (tolower(first[i]) != tolower(second[i])) {
				return (false);
			}
		}
	}
	return (true);

}

void RayTrace::calculateIntersections() {

	GLfloat stepx = (GLfloat)(2) / windowWidth;
	GLfloat stepy = (GLfloat)(2) / windowHeight;
	int pixelx = (windowWidth - 1), pixely = 0;

	for (GLfloat i = -1.0; i <= 1.0 && pixelx >= 0; i += stepx, pixelx -= 1) {
		for (GLfloat j = -1.0; j <= 1.0 && pixely < windowHeight; j += stepy, pixely += 1) {
			glm::vec3 rayDir, baryIntersect, vertexIntesect;
			glm::vec3 windowPoint(i, j, windowz);
			float windowDistance = glm::distance(eyeLocation, windowPoint);
			Intersection *intersection = new Intersection();
			intersection->triangleIndex = -1;
			int k = 0, check = 0;
			rayDir.x = i - eyeLocation.x;
			rayDir.y = j - eyeLocation.y;
			rayDir.z = windowz - eyeLocation.z;
			rayDir = glm::normalize(rayDir);
			for (Triangle *triangle : triangles) {
				if (glm::intersectRayTriangle(eyeLocation, rayDir, verticesVec[triangle->vertexIndices[0]], verticesVec[triangle->vertexIndices[1]], verticesVec[triangle->vertexIndices[2]], baryIntersect)) {
					vertexIntesect = (baryIntersect.x * verticesVec[triangle->vertexIndices[1]]) + (baryIntersect.y * verticesVec[triangle->vertexIndices[2]]) + ((1 - baryIntersect.x - baryIntersect.y) * verticesVec[triangle->vertexIndices[0]]);
					float intersectDistance = glm::distance(eyeLocation, vertexIntesect);
					if (intersectDistance >= windowDistance) {
						if (check == 0) {
							intersection->triangleIndex = k;
							intersection->vertexIntesect = vertexIntesect;
							intersection->baryIntersect = baryIntersect;
							intersection->intersectDistance = intersectDistance;
							check = 1;
						}
						else {
							if (intersectDistance < intersection->intersectDistance) {
								intersection->triangleIndex = k;
								intersection->vertexIntesect = vertexIntesect;
								intersection->baryIntersect = baryIntersect;
								intersection->intersectDistance = intersectDistance;
							}
						}
					}
				}
				k++;
			}
			if (intersection->triangleIndex != -1) {
				intersections[pixelx][pixely] = intersection;
			}
			else {
				intersections[pixelx][pixely] = NULL;
			}
		}
		pixely = 0;
	}

}

void RayTrace::calculateLighting() {

	for (int i = 0; i < windowWidth; i++) {
		for (int j = 0; j < windowHeight; j++) {
			Intersection *intersection = intersections[windowWidth - i - 1][j];
			glm::vec3 lightVector, halfVector;
			glm::mediump_float diffuseDot, specularDot;
			string groupName;
			if (NULL != intersection && intersection->triangleIndex != -1) {
				groupName = triangles[intersection->triangleIndex]->groupName;
				glm::vec3 ambient(0.0, 0.0, 0.0), diffuse(0.0, 0.0, 0.0), specular(0.0, 0.0, 0.0);
				for (Light *light : lights) {
					lightVector = glm::normalize(light->location - intersection->vertexIntesect);
					ambient += light->la * materials[groupName]->ka;
					bool shadow = false;
					int count = 0;
					float distanceToLight = glm::distance(intersection->vertexIntesect, light->location);
					for (Triangle *triangle : triangles) {
						if (count != intersection->triangleIndex) {
							glm::vec3 baryShadow, vertexShadow;
							if (glm::intersectRayTriangle(intersection->vertexIntesect, lightVector, verticesVec[triangle->vertexIndices[0]], verticesVec[triangle->vertexIndices[1]], verticesVec[triangle->vertexIndices[2]], baryShadow)) {
								vertexShadow = (baryShadow.x * verticesVec[triangle->vertexIndices[1]]) + (baryShadow.y * verticesVec[triangle->vertexIndices[2]]) + ((1 - baryShadow.x - baryShadow.y) * verticesVec[triangle->vertexIndices[0]]);
								float distanceToShadowInt = glm::distance(intersection->vertexIntesect, vertexShadow);
								if (distanceToShadowInt < distanceToLight) {
									shadow = true;
									break;
								}
							}
						}
						count++;
					}
					if (!shadow) {
						halfVector = glm::normalize((light->location - intersection->vertexIntesect) + (eyeLocation - intersection->vertexIntesect));
						glm::vec3 normal;
						if (!vnPresent) {
							normal = triangles[intersection->triangleIndex]->faceNormal;
						}
						else {
							normal = glm::normalize((intersection->baryIntersect.x * normalsVec[triangles[intersection->triangleIndex]->normalIndices[1]]) + (intersection->baryIntersect.y * normalsVec[triangles[intersection->triangleIndex]->normalIndices[2]]) + ((1 - intersection->baryIntersect.x - intersection->baryIntersect.y) * normalsVec[triangles[intersection->triangleIndex]->normalIndices[0]]));
						}
						diffuseDot = glm::dot(lightVector, normal);
						if (diffuseDot < 0) {
							diffuseDot = 0;
						}
						else if (diffuseDot > 1) {
							diffuseDot = 1;
						}
						diffuse += light->ld * materials[groupName]->kd * diffuseDot;
						specularDot = glm::dot(normal, halfVector);
						if (specularDot < 0) {
							specularDot = 0;
						}
						else if (specularDot > 1) {
							specularDot = 1;
						}
						specular += light->ls * materials[groupName]->ks * pow(specularDot, ((materials[groupName]->N / 1000) * 128));
					}
				}
				pixels[i + (j * windowHeight)].r = ((ambient.x + diffuse.x + specular.x) > 1 ? 1 : (ambient.x + diffuse.x + specular.x));
				pixels[i + (j * windowHeight)].g = ((ambient.y + diffuse.y + specular.y) > 1 ? 1 : (ambient.y + diffuse.y + specular.y));
				pixels[i + (j * windowHeight)].b = ((ambient.z + diffuse.z + specular.z) > 1 ? 1 : (ambient.z + diffuse.z + specular.z));
				free(intersection);
			}
		}
	}

}

void RayTrace::renderScene() {

	glDrawPixels(windowWidth, windowHeight, GL_RGB, GL_FLOAT, pixels);
	glutSwapBuffers();

}

int RayTrace::parseLightSources(string lightPath) {

	ifstream inputFile;
	string line, tempString;
	bool emptyLine;
	vector<string> *tokens;
	
	inputFile.open(lightPath);
	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			Light *light = new Light();
			light->location.x = std::stof((*tokens)[0]);
			light->location.y = std::stof((*tokens)[1]);
			light->location.z = std::stof((*tokens)[2]);
			light->la = std::stof((*tokens)[3]);
			light->ls = std::stof((*tokens)[4]);
			light->ld = std::stof((*tokens)[5]);
			lights.push_back(light);
		}
	}
	return 0;

}

int RayTrace::parseEyeLocation() {

	eyeLocation.x = 0;
	eyeLocation.y = 0;
	eyeLocation.z = -2;
	return 0;

}

int RayTrace::parseSmoothingOpt(string smoothingPath) {

	ifstream inputFile;
	string line, tempString;
	bool emptyLine;
	vector<string> *tokens;
	
	inputFile.open(smoothingPath);
	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			if (std::stoi((*tokens)[1]) != 1)
				smoothingOpt = false;
			else
				smoothingOpt = true;
			return 0;
		}
	}
	return 1;

}

int RayTrace::parseInterfaceWindow(string winPath) {

	ifstream inputFile;
	string line, tempString;
	bool emptyLine;
	vector<string> *tokens;
	
	inputFile.open(winPath);
	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			windowWidth = std::stoi((*tokens)[0]);
			windowHeight = std::stoi((*tokens)[1]);
			return 0;
		}
	}
	return 1;

}

int RayTrace::parseOBJMTL(string objPath) {

	string line, mtlPath, tempString, mtlName1, mtlName2;
	ifstream inputFile;
	float firstCoords, secondCoords, thirdCoords, limitVertex, normalizeLimit = 1.5;
	int firstVertex, secondVertex, thirdVertex;
	Triangle *triangle;
	MTL *mtl = new MTL();
	bool firstTime = true, emptyLine, ka = false, ks = false, kd = false, N = false;
	vector<string> *tokens;
	vector<string> *groupName;
	MTL_HASHMAP materialsTemp;

	limitVertex = normalizeLimit;
	inputFile.open(objPath);
	if (!inputFile.is_open()) {
		cout << "The OBJ path " << objPath << " is incorrect." << endl;
		return 1;
	}

	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			if (compareNoCase((*tokens)[0], "mtllib")) {
				mtlPath = (*tokens)[1];
				break;
			}
		}
	}

	inputFile.close();
	if (mtlPath.empty()) {
		cout << "MTL path not provided in OBJ file. Please provide it after the mtllib token." << endl;
		return 1;
	}
	else {
		mtlPath = "files/" + mtlPath ;
		inputFile.open(mtlPath);
	}
	if (!inputFile.is_open()) {
		cout << "The MTL path " << mtlPath << " is incorrect." << endl;
		return 1;
	}

	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			if (compareNoCase((*tokens)[0], "newmtl")) {
				if (!firstTime) {
					if (!ka) {
						mtl->ka.x = 0.0;
						mtl->ka.y = 0.0;
						mtl->ka.z = 0.0;
					}
					if (!ks) {
						mtl->ks.x = 0.0;
						mtl->ks.y = 0.0;
						mtl->ks.z = 0.0;
					}
					if (!kd) {
						mtl->kd.x = 0.0;
						mtl->kd.y = 0.0;
						mtl->kd.z = 0.0;
					}
					if (!N) {
						mtl->N = 0;
					}
					ka = ks = kd = N = false;
					materialsTemp.emplace(mtlName2, mtl);
				}
				mtl = new MTL();
				mtlName2 = (*tokens)[1];
				firstTime = false;
			}
			else if (compareNoCase((*tokens)[0], "ka") && !firstTime) {
				mtl->ka.x = std::stof((*tokens)[1]);
				mtl->ka.y = std::stof((*tokens)[2]);
				mtl->ka.z = std::stof((*tokens)[3]);
				ka = true;
			}
			else if (compareNoCase((*tokens)[0], "ks") && !firstTime) {
				mtl->ks.x = std::stof((*tokens)[1]);
				mtl->ks.y = std::stof((*tokens)[2]);
				mtl->ks.z = std::stof((*tokens)[3]);
				ks = true;
			}
			else if (compareNoCase((*tokens)[0], "kd") && !firstTime) {
				mtl->kd.x = std::stof((*tokens)[1]);
				mtl->kd.y = std::stof((*tokens)[2]);
				mtl->kd.z = std::stof((*tokens)[3]);
				kd = true;
			}
			else if ((((*tokens)[0].compare("N") == 0) || compareNoCase((*tokens)[0], "Ns")) && !firstTime) {
				mtl->N = std::stof((*tokens)[1]);
				N = true;
			}
		}
		free(tokens);
	}
	if (!ka) {
		mtl->ka.x = 0.0;
		mtl->ka.y = 0.0;
		mtl->ka.z = 0.0;
	}
	if (!ks) {
		mtl->ks.x = 0.0;
		mtl->ks.y = 0.0;
		mtl->ks.z = 0.0;
	}
	if (!kd) {
		mtl->kd.x = 0.0;
		mtl->kd.y = 0.0;
		mtl->kd.z = 0.0;
	}
	if (!N) {
		mtl->N = 0;
	}
	materialsTemp.emplace(mtlName2, mtl);

	inputFile.close();
	inputFile.open(objPath);

	groupName = new vector<string>();
	bool usemtlCheck = false;

	while (getline(inputFile, line)) {
		emptyLine = true;
		stringstream stream(line);
		tokens = new vector<string>();
		glm::vec3 tempVertex;
		while (stream >> tempString) {
			tokens->push_back(tempString);
			emptyLine = false;
		}
		if (!emptyLine) {
			if (compareNoCase((*tokens)[0], "v")) {
				firstCoords = std::stof((*tokens)[1]);
				secondCoords = std::stof((*tokens)[2]);
				thirdCoords = std::stof((*tokens)[3]);
				tempVertex.x = firstCoords;
				tempVertex.y = secondCoords; 
				tempVertex.z = thirdCoords;
				if (abs(tempVertex.x) > limitVertex) {
					limitVertex = abs(tempVertex.x);
				}
				if (abs(tempVertex.y) > limitVertex) {
					limitVertex = abs(tempVertex.y);
				}
				if (abs(tempVertex.z) > limitVertex && tempVertex.z < -1) {
					limitVertex = abs(tempVertex.z);
				}
				verticesVec.push_back(tempVertex);
			}
			else if (compareNoCase((*tokens)[0], "vn")) {
				vnPresent = true;
				firstCoords = std::stof((*tokens)[1]);
				secondCoords = std::stof((*tokens)[2]);
				thirdCoords = std::stof((*tokens)[3]);
				tempVertex.x = firstCoords;
				tempVertex.y = secondCoords; 
				tempVertex.z = thirdCoords;
				normalsVec.push_back(tempVertex);
			}
			else if (compareNoCase((*tokens)[0], "g") || compareNoCase((*tokens)[0], "group")) {
				if (usemtlCheck || compareNoCase((*tokens)[0], "group")) {
					free(groupName);
					groupName = new vector<string>;
					usemtlCheck = false;
				}
				int groupSize = tokens->size();
				for (int i = 1; i < groupSize; i++) {
					groupName->push_back((*tokens)[i]);
				}
			}
			else if (compareNoCase((*tokens)[0], "f")) {
				int size = tokens->size();
				size_t pos, secondPos, fTokenPos, fTokenSecondPos;
				if (vnPresent) {
					string subToken, firstToken = (*tokens)[1];;
					fTokenPos = 0, fTokenSecondPos = 0;
					fTokenPos = firstToken.find('/', fTokenPos);
					fTokenSecondPos = firstToken.find('/', (fTokenPos + 1));
					for (int i = 2; i <= size - 2; i++) {
						triangle = new Triangle();
						triangle->groupName = groupName->at(groupName->size() - 1);
						triangle->vertexIndices[0] = std::stoi(firstToken.substr(0, fTokenPos)) - 1;
						triangle->normalIndices[0] = std::stoi(firstToken.substr((fTokenSecondPos + 1), firstToken.length())) - 1;
						pos = 0, secondPos = 0;
						subToken = (*tokens)[i];
						pos = subToken.find('/', pos);
						secondPos = subToken.find('/', (pos + 1));
						triangle->vertexIndices[1] = std::stoi(subToken.substr(0, pos)) - 1;
						triangle->normalIndices[1] = std::stoi(subToken.substr((secondPos + 1), subToken.length())) - 1;
						pos = 0, secondPos = 0;
						subToken = (*tokens)[i + 1];
						pos = subToken.find('/', pos);
						secondPos = subToken.find('/', (pos + 1));
						triangle->vertexIndices[2] = std::stoi(subToken.substr(0, pos)) - 1;
						triangle->normalIndices[2] = std::stoi(subToken.substr((secondPos + 1), subToken.length())) - 1;
						triangles.push_back(triangle);
					}
				}
				else {
					for (int i = 2; i <= size - 2; i++) {
						triangle = new Triangle();
						triangle->groupName = groupName->at(groupName->size() - 1);
						firstVertex = std::stoi((*tokens)[1]) - 1;
						secondVertex = std::stoi((*tokens)[i]) - 1;
						thirdVertex = std::stoi((*tokens)[i + 1]) - 1;
						triangle->vertexIndices[0] = firstVertex;
						triangle->vertexIndices[1] = secondVertex;
						triangle->vertexIndices[2] = thirdVertex;
						triangles.push_back(triangle);
					}
				}
			}
			else if (compareNoCase((*tokens)[0], "usemtl")) {
				mtlName1 = (*tokens)[1];
				int totalGroupSize = groupName->size();
				MTL_HASHMAP::const_iterator it = materialsTemp.find(mtlName1);
				if (it != materialsTemp.end()) {
					for (int i = 0; i < totalGroupSize; i++) {
						materials.emplace(groupName->at(i), (*it).second);
					}
				}
				else {
					MTL *mtlTemp = new MTL();
					mtlTemp->ka.x = 1.0;
					mtlTemp->ka.y = 1.0;
					mtlTemp->ka.z = 1.0;
					mtlTemp->ks.x = 1.0;
					mtlTemp->ks.y = 1.0;
					mtlTemp->ks.z = 1.0;
					mtlTemp->kd.x = 1.0;
					mtlTemp->kd.y = 1.0;
					mtlTemp->kd.z = 1.0;
					mtlTemp->N = 1;
					for (int i = 0; i < totalGroupSize; i++) {
						materials.emplace(groupName->at(i), mtlTemp);
					}
				}
				usemtlCheck = true;
			}
		}
		free(tokens);
	}

	if (!usemtlCheck && groupName->size() > 0) {
		int totalGroupSize = groupName->size();
		MTL *mtlTemp = new MTL();
		mtlTemp->ka.x = 1.0;
		mtlTemp->ka.y = 1.0;
		mtlTemp->ka.z = 1.0;
		mtlTemp->ks.x = 1.0;
		mtlTemp->ks.y = 1.0;
		mtlTemp->ks.z = 1.0;
		mtlTemp->kd.x = 1.0;
		mtlTemp->kd.y = 1.0;
		mtlTemp->kd.z = 1.0;
		mtlTemp->N = 1;
		for (int i = 0; i < totalGroupSize; i++) {
			materials.emplace(groupName->at(i), mtlTemp);
		}
	}

	inputFile.close();

	int count = 0;
	if (limitVertex > normalizeLimit) {
		for (glm::vec3 vertex : verticesVec) {
			vertex.x = (vertex.x / limitVertex) * normalizeLimit;
			vertex.y = (vertex.y / limitVertex) * normalizeLimit;
			vertex.z = (vertex.z / limitVertex) * normalizeLimit;
			verticesVec[count].x = vertex.x;
			verticesVec[count].y = vertex.y;
			verticesVec[count].z = vertex.z;
			count++;
		}
	}

	for (Triangle *triangle : triangles) {
		firstVertex = triangle->vertexIndices[0];
		secondVertex = triangle->vertexIndices[1];
		thirdVertex = triangle->vertexIndices[2];
		triangle->faceNormal = glm::normalize(glm::cross((verticesVec[secondVertex] - verticesVec[firstVertex]), (verticesVec[thirdVertex] - verticesVec[firstVertex])));
	}

	if (!vnPresent && smoothingOpt) {
		int vertexCount = 0, normalCount = 0;
		for (glm::vec3 vertex : verticesVec) {
			glm::vec3 vertexNormal(0.0, 0.0, 0.0);
			for (Triangle *triangle : triangles) {
				if (triangle->vertexIndices[0] == vertexCount) {
					vertexNormal += triangle->faceNormal;
					triangle->normalIndices[0] = normalCount;
				}
				else if (triangle->vertexIndices[1] == vertexCount) {
					vertexNormal += triangle->faceNormal;
					triangle->normalIndices[1] = normalCount;
				}
				else if (triangle->vertexIndices[2] == vertexCount) {
					vertexNormal += triangle->faceNormal;
					triangle->normalIndices[2] = normalCount;
				}
			}
			if (vertexNormal.x != 0 && vertexNormal.y != 0 && vertexNormal.z != 0)
				vertexNormal = glm::normalize(vertexNormal);
			normalsVec.push_back(vertexNormal);
			vertexCount++;
			normalCount++;
		}
		vnPresent = true;
	}

	return 0;

}
