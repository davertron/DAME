#include "Game.h"
#include "cinder/ImageIO.h"
#include "cinder/gl/Texture.h"
#include "cinder/Text.h"
#include <string>

using namespace ci;
using namespace std;

Game::Game(void){
}

Game::Game(string _url)
{
	url = _url;
	image = gl::Texture(loadImage(loadUrl(Url("http://www.mamedb.com/snap/" + url + ".png"))));
}


Game::~Game(void)
{
}

int Game::getImageWidth(){
	return image.getWidth();
}

int Game::getImageHeight(){
	return image.getHeight();
}

void Game::enableAndBindImage(){
	image.enableAndBind();
}

gl::Texture Game::getImage(){
	return image;
}

void Game::setPosition(Vec3f pos){
	position = pos;
}

Vec3f Game::getPosition(){
	return position;
}

void Game::setTitle(string tit){
	title = tit;
	TextLayout layout;
	layout.setFont(Font("Verdana", 24.0));
	layout.setColor( Color( 1.0f, 1.0f, 1.0f) );
	layout.addCenteredLine(title);
	renderedTitle = gl::Texture(layout.render(true, true));
}

string Game::getTitle(){
	return title;
}

gl::Texture Game::getRenderedTitle(){
	return renderedTitle;
}