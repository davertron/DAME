#pragma once
#include <string>
#include "cinder/gl/Texture.h"
#include "cinder/Font.h"
#include "MenuObject.h"
#include "cinder/TriMesh.h"
#include <string>

using namespace ci;
using namespace std;

class Console : public MenuObject
{
private:
	string asset;
	TriMesh mesh;
	float rotation;
	float scale;
public:
	Console(void);
	~Console(void);
	void initialize();
	void setAsset(string);
	string getAsset();
	TriMesh getMesh();
	void setRotation(float);
	float getRotation();
	void setScale(float);
	float getScale();
};

