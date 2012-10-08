#pragma once
#include <string>
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace std;

class Game
{
private:
	string url;
	string title;
	gl::Texture image;
	Vec3f position;
	gl::Texture renderedTitle;
public:
	Game(void);
	Game(string url);
	~Game(void);
	int getImageWidth();
	int getImageHeight();
	void enableAndBindImage();
	gl::Texture getImage();
	void setPosition(Vec3f);
	Vec3f getPosition();
	void setTitle(string);
	string getTitle();
	gl::Texture getRenderedTitle();
};

