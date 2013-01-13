#pragma once
#include <string>
#include "cinder/gl/Texture.h"
#include "cinder/Font.h"
#include "MenuObject.h"

using namespace ci;
using namespace std;

class Game : public MenuObject
{
private:
	string filename;
	gl::Texture image;
public:
	Game(void);
	Game(string filename);
	~Game(void);
	int getImageWidth();
	int getImageHeight();
	void enableAndBindImage();
	gl::Texture getImage();
};

