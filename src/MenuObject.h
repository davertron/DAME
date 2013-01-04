#pragma once
#include <string>
#include "cinder/gl/Texture.h"
#include "cinder/Font.h"

using namespace ci;
using namespace std;

class MenuObject
{
private:
	string title;
	Vec3f position;
	gl::Texture renderedTitle;
	Font titleFont;
public:
	MenuObject(void);
	~MenuObject(void);
	void setPosition(Vec3f);
	Vec3f getPosition();
	void setTitle(string);
	string getTitle();
	gl::Texture getRenderedTitle();
};

