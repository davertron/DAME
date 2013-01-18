#include "Game.h"
#include "cinder/ImageIO.h"
#include "cinder/gl/Texture.h"
#include "cinder/Text.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Font.h"
#include <string>
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Game::Game(void){
}

Game::Game(string _filename){
	filename = _filename;
    DataSourceRef imageAssetRef;
    bool downloaded = false;
    bool errorFindingImage = false;
    try {
        // Try to load the image from assets...
        imageAssetRef = loadAsset(filename + ".png");
    } catch(...){
        errorFindingImage = true;
        console() << "Could not find rom image asset for " << filename << " , attempting to download..." << endl;
        // If we don't already ahve it, try to download it from www.mamedb.com
        try {
            imageAssetRef = loadUrl("http://www.mamedb.com/snap/" + filename + ".png");
            downloaded = true;
        } catch(...){
            console() << "Could not download rom image for " << filename << " , using 'noimage.png'." << endl;
        }
    }
    try {
        image = gl::Texture(loadImage(imageAssetRef));
    } catch(...){
        // Use the 'no image' image...
        imageAssetRef = loadAsset("noimage.png");
        image = gl::Texture(loadImage(imageAssetRef));
    }

    if(!errorFindingImage){
        console() << "Found image for " << filename << " in assets dir." << endl;
    } else if(downloaded){
        // If we downloaded it, save it out to the assets dir so we can re-use it later.
        console() << "Downloaded image, writing to asset dir..." << endl;
        try {
            string noImagePath = getAssetPath("noimage.png").string();
            writeImage(noImagePath.substr(0, noImagePath.find("noimage.png")) + filename + ".png", image);
        } catch(...){
            console() << "Unable to write image to asset dir." << endl;
        }
    }
}

Game::~Game(void){
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
