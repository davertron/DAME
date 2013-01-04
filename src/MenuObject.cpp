#include "MenuObject.h"
#include "cinder/Text.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Font.h"
#include <string>
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

MenuObject::MenuObject(void){
}

MenuObject::~MenuObject(void){
}

void MenuObject::setPosition(Vec3f pos){
	position = pos;
}

Vec3f MenuObject::getPosition(){
	return position;
}

void MenuObject::setTitle(string tit){
	title = tit;
	TextLayout layout;
	layout.setFont(Font( loadResource( RES_AKASHI_FONT ), 48));
	layout.setColor( Color( 1.0f, 1.0f, 1.0f) );
	layout.clear(ColorA(1.0f, 1.0f, 1.0f, 0.0f));
	layout.addCenteredLine(title);
	renderedTitle = gl::Texture(layout.render(true, true));
}

string MenuObject::getTitle(){
	return title;
}

gl::Texture MenuObject::getRenderedTitle(){
	return renderedTitle;
}
