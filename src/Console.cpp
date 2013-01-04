#pragma once
#include "Console.h"
#include "cinder/app/AppBasic.h"
#include "cinder/ObjLoader.h"
#include <string>
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Console::Console(void){
}

Console::~Console(void){
}

void Console::initialize(){
	// Load 3d models; mesh will be populated with the result
	ObjLoader loader(loadAsset(asset));
	loader.load(&mesh);
}

void Console::setAsset(string _asset){
	asset = _asset;
}

string Console::getAsset(){
	return asset;
}

TriMesh Console::getMesh(){
	return mesh;
}

void Console::setRotation(float _rotation){
	rotation = _rotation;
}

float Console::getRotation(){
	return rotation;
}

void Console::setScale(float _scale){
	scale = _scale;
}

float Console::getScale(){
	return scale;
}