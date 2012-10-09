#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIO.h"
#include "cinder/gl/Texture.h"
#include <vector>
#include <string>
#include "Game.h"
#include <process.h>
#include <Windows.h>
#include <tchar.h>
//#include "boost\program_options\parsers.hpp"
#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;

class DAMEAppApp : public AppBasic {
  public:
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
	void keyDown( KeyEvent event );	
	void update();
	void draw();
	void drawGrid();
	void drawLine();
	void runSelectedGame();

	Vec3f cameraPosition;
	int currentGameIndex;

	vector<string> gameNames;
	vector<string> gameTitles;
	vector<Game> games;

	//boost::program_options::basic_parsed_options<wchar_t> configOptions;
};

void DAMEAppApp::prepareSettings( Settings *settings )
{
	settings->setFullScreen(true);
	//settings->setWindowSize(800, 600);
	settings->setFrameRate( 60.0f );
}

void DAMEAppApp::setup()
{
	gameNames.push_back("starwars");
	gameTitles.push_back("Star Wars");
	gameNames.push_back("rastan");
	gameTitles.push_back("Rastan");
	gameNames.push_back("galaxian");
	gameTitles.push_back("Galaxian");
	gameNames.push_back("zaxxon");
	gameTitles.push_back("Zaxxon");
	gameNames.push_back("sonicwi2");
	gameTitles.push_back("Sonic Something");
	gameNames.push_back("1942");
	gameTitles.push_back("1942");
	gameNames.push_back("combat");
	gameTitles.push_back("Combat");
	gameNames.push_back("gauntlet");
	gameTitles.push_back("Gauntlet");
	gameNames.push_back("gng");
	gameTitles.push_back("Ghosts 'n Goblins");
	gameNames.push_back("nemesis"); // gradius
	gameTitles.push_back("Gradius");
	gameNames.push_back("gunsmoke");
	gameTitles.push_back("Gunsmoke");
	gameNames.push_back("hangon");
	gameTitles.push_back("Hangon");
	gameNames.push_back("sharrier");
	gameTitles.push_back("Space Harrier");
	gameNames.push_back("astorm");
	gameTitles.push_back("A Storm");
	gameNames.push_back("aliens");
	gameTitles.push_back("Aliens");
	gameNames.push_back("ddragon3");
	gameTitles.push_back("Double Dragon 3");
	gameNames.push_back("msword");
	gameTitles.push_back("Msword");
	gameNames.push_back("mercs");
	gameTitles.push_back("Mercs");
	gameNames.push_back("pitfight");
	gameTitles.push_back("Pit Fight");
	gameNames.push_back("smashtv");
	gameTitles.push_back("Smash TV");
	gameNames.push_back("tmnt");
	gameTitles.push_back("Teenage Mutant Ninja Turtles");
	gameNames.push_back("btoads");
	gameTitles.push_back("Battletoads");
	gameNames.push_back("crusnusa");
	gameTitles.push_back("Cruisin' USA");
	gameNames.push_back("kinst");
	gameTitles.push_back("Killer Instinct");
	gameNames.push_back("mk3");
	gameTitles.push_back("Mortal Kombat");
	gameNames.push_back("tekken");
	gameTitles.push_back("Tekken");
	gameNames.push_back("kof94");
	gameTitles.push_back("King of Fighters '94");
	gameNames.push_back("xmcota");
	gameTitles.push_back("Xmen: Children of the Atom");
	gameNames.push_back("ffight");
	gameTitles.push_back("Final Fight");
	gameNames.push_back("goldnaxe");
	gameTitles.push_back("Golden Axe");
	gameNames.push_back("slyspy");
	gameTitles.push_back("Sly Spy");
	gameNames.push_back("strider");
	gameTitles.push_back("Strider");
	gameNames.push_back("aof");
	gameTitles.push_back("AOF");
	gameNames.push_back("nss_con3");
	gameTitles.push_back("Contra 3");
	gameNames.push_back("sf2ce");
	gameTitles.push_back("Street Fighter 2 Championship Edition");
	gameNames.push_back("xmen");
	gameTitles.push_back("Xmen");

	int i = 0;
	for(; i < gameNames.size(); i++){
		try {
			games.push_back(Game(gameNames[i]));
		} catch(...){
			console() << "Failed to load image for " << gameNames[i] << endl;
		}
	}

	const int gap = 20;
	int xPosition;
	for(i=0; i < games.size(); i++){
		if(i != 0){
			xPosition += games[i-1].getImage().getWidth() + gap;
		} else {
			xPosition = -1*games[i].getImage().getWidth() / 2;
		}
		games[i].setPosition(Vec3f(xPosition, -1 * games[i].getImage().getHeight() / 2, 0));
		games[i].setTitle(gameTitles[i]);
	}

	cameraPosition = games[0].getPosition() + Vec3f(games[0].getImageWidth()/2, games[0].getImageHeight()/2, 0);

	currentGameIndex = 0;

	// Load mame path and mame rom path
	//ifstream file;
    //file.open("config.ini");
	//boost::program_options::options_description description;
	//boost::program_options::parse_config_file(file, description, false);
}

void DAMEAppApp::mouseDown( MouseEvent event )
{
}

void DAMEAppApp::keyDown( KeyEvent event ) {
	if( event.getCode() == KeyEvent::KEY_ESCAPE ){
        quit();
	} else if(event.getCode() == KeyEvent::KEY_RIGHT){
		currentGameIndex++;
	} else if(event.getCode() == KeyEvent::KEY_LEFT){
		currentGameIndex--;
	} else if(event.getCode() == KeyEvent::KEY_RETURN){
		runSelectedGame();
	}

	if(currentGameIndex < 0){
		currentGameIndex = 0;
	} else if(currentGameIndex >= games.size()){
		currentGameIndex = games.size()-1;
	}

	cameraPosition = games[currentGameIndex].getPosition() + Vec3f(games[currentGameIndex].getImageWidth()/2, games[currentGameIndex].getImageHeight()/2, 0);
}

void DAMEAppApp::update()
{
	hideCursor();
}

void DAMEAppApp::draw()
{
	//drawGrid();
	drawLine();
}

// Not currently used, but I left it here in case I wanted to go back to it at some point
void DAMEAppApp::drawGrid()
{
	gl::clear( Color( 0, 0, 0 ), true );

	const int gap = 20;
	int i;
	int widthSoFar = gap;
	int currentY = gap;
	int maxHeight = 0;
	for(i=0; i < games.size(); i++){
		if(games[i].getImageWidth() + widthSoFar >= getWindowWidth()){
			currentY += maxHeight + gap;
			maxHeight = 0;
			widthSoFar = gap;
		}

		games[i].enableAndBindImage();
		gl::draw( games[i].getImage(), Vec2i(widthSoFar, currentY ));
		widthSoFar += games[i].getImageWidth() + gap;
		if(games[i].getImageHeight() > maxHeight){
			maxHeight = games[i].getImageHeight();
		}
	}
}

void DAMEAppApp::drawLine(){
	gl::clear(Color(0,0,0), true);

	gl::pushMatrices();
	gl::translate(Vec3f(getWindowWidth()/2-cameraPosition.x, getWindowHeight()/2-cameraPosition.y, -1*cameraPosition.z));
	int i;
	const int borderSize = 3;
	for(i=0; i < games.size(); i++){
		gl::pushMatrices();
		if(i == currentGameIndex){
			//gl::scale(Vec3f(2.0, 2.0, 1.0));
			gl::drawSolidRect(Rectf(games[i].getPosition().x-borderSize, games[i].getPosition().y-borderSize, games[i].getPosition().x + games[i].getImage().getWidth()+borderSize, games[i].getPosition().y + games[i].getImage().getHeight()+borderSize), false);
			games[i].getRenderedTitle().enableAndBind();
			gl::draw(games[i].getRenderedTitle(), Vec2f(games[i].getPosition().x + games[i].getImageWidth()/2 - games[i].getRenderedTitle().getWidth()/2, games[i].getPosition().y+games[i].getImageHeight()+25));
			games[i].getRenderedTitle().unbind();
		}
		games[i].enableAndBindImage();
		gl::draw(games[i].getImage(), Vec2f(games[i].getPosition().x, games[i].getPosition().y));
		games[i].getImage().unbind();
		gl::popMatrices();
	}

	gl::popMatrices();
}

void DAMEAppApp::runSelectedGame(){
	console() << endl << endl << endl;
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
	std::wstring mame = _T("C:\\Users\\Dave\\Desktop\\HyperSpin\\Emulators\\MAME\\mame.exe -rompath C:\\Users\\Dave\\Desktop\\HyperSpin\\Emulators\\MAME\\roms mk");

	// We have to drop out of full screen to run mame
	setFullScreen(false);
	setWindowSize(800, 600);

    if( !CreateProcess(NULL,    // Use the command line
        &mame[0],			    // Command line
        NULL,					// Process handle not inheritable
        NULL,					// Thread handle not inheritable
        FALSE,					// Set handle inheritance to FALSE
        0,						// No creation flags
        NULL,					// Use parent's environment block
        NULL,					// User parent's directory
        &si,					// Pointer to STARTUPINFO structure
        &pi )					// Pointer to PROCESS_INFORMATION structure
    ) 
    {
        console() << "CreateProcess failed: " << GetLastError() << endl << endl;
		return;
    } else {
		console() << "Created process, you should see mame right now" << endl;
	}

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

	setFullScreen(true);
}

CINDER_APP_BASIC( DAMEAppApp, RendererGl )
