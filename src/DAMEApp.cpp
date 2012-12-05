#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIO.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "Game.h"
#include "cinder/ObjLoader.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Text.h"
#include "Resources.h"
#include "cinder/Utilities.h"
//#include "Resources.h"

#include <vector>
#include <string>
#include <tchar.h>
#include <sstream>
#include <fstream>

#include <process.h>
#include <Windows.h>

#include "boost\program_options\parsers.hpp"
#include "boost\program_options\variables_map.hpp"
#include "boost\ptr_container\ptr_list.hpp"


using namespace ci;
using namespace ci::app;
using namespace std;
namespace po = boost::program_options;

class DAMEAppApp : public AppBasic {
  public:
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
	void keyDown( KeyEvent event );	
	void update();
	void draw();
	void drawBackground();
	void drawGrid();
	void drawLine();
	void runSelectedGame();
	bool backgroundNeedsUpdate();
	Vec3f getCenterOfCurrentGame();

	CameraPersp camera;
	CameraPersp backgroundCamera;
	unsigned int currentGameIndex;
	double lastFrameTime;
	double lastBackgroundDraw;

	vector<string> gameNames;
	vector<string> gameTitles;
	vector<Game> games;

	string mamePath;
	string romPath;

	float cabinetRotation;
	float cabinetScale;

	TriMesh arcadeCabinet;

	// Shaders for Toon Shading of background
	gl::GlslProg depthShader;
	gl::GlslProg convolutionShader;
	gl::GlslProg normalShader;
	gl::GlslProg normalEdgeShader;
	gl::GlslProg silhouetteShader;
	gl::GlslProg compositeShader;
	gl::GlslProg passThruShader;
	gl::GlslProg phongShader;

	gl::Fbo normalBuffer;
	gl::Fbo normalEdgeBuffer;
	gl::Fbo depthBuffer;
	gl::Fbo depthEdgeBuffer;
	gl::Fbo silhouetteBuffer;
	gl::Fbo shadedModelBuffer;
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

	unsigned int i = 0;
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
		games[i].setPosition(Vec3f((float)xPosition, (float)(-1.0 * games[i].getImage().getHeight() / 2), 0.0));
		//games[i].setTitleFont(Font( loadResource( RES_CHUNKFIVE_FONT ), 48));
		games[i].setTitle(gameTitles[i]);
	}

	currentGameIndex = 0;

	camera.setPerspective(60.0f, getWindowAspectRatio(), 5.0f, 5000.0f);
	backgroundCamera.setPerspective(60.0f, getWindowAspectRatio(), 5.0f, 5000.0f);
	backgroundCamera.lookAt(Vec3f(0.0f, 0.0f, -500.0f), Vec3f::zero(), -1*Vec3f::yAxis());

	Vec3f centerOfFirstGame = getCenterOfCurrentGame();
	Vec3f cameraPos = centerOfFirstGame + Vec3f(0.0f, 0.0f, -500.0f);
	camera.lookAt(cameraPos, getCenterOfCurrentGame(), -1*Vec3f::yAxis());

	// Load mame path and mame rom path
	po::variables_map configOptions;
	// Setup options.
	po::options_description desc("Options");
	desc.add_options()
		("mamePath", po::value< std::string >( &mamePath ), "mamePath" )
		("romPath", po::value< std::string >( &romPath ), "romPath");

	std::ifstream settings_file( "settings.ini" );
	po::store( po::parse_config_file( settings_file , desc ), configOptions );
	settings_file.close();
	po::notify( configOptions );

	lastFrameTime = getElapsedSeconds();
	lastBackgroundDraw = -1;

	// Load 3d model of arcade cabinet
	ObjLoader loader(loadAsset("cab.obj"));
	loader.load(&arcadeCabinet);

	// Setup our fbos and shaders
	try {
		depthShader = gl::GlslProg( loadAsset( "depth_vert.glsl" ), loadAsset( "depth_frag.glsl" ) );
		convolutionShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "convolution_frag.glsl" ) );
		normalShader = gl::GlslProg( loadAsset( "normal_vert.glsl" ), loadAsset( "normal_frag.glsl" ) );
		normalEdgeShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "normal_edge_frag.glsl" ) );
		silhouetteShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "render_silhouette_frag.glsl" ) );
		compositeShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "composite_frag.glsl" ) );
		passThruShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "passThru_frag.glsl" ) );
		phongShader = gl::GlslProg( loadAsset( "phong_vert.glsl" ), loadAsset( "phong_frag.glsl" ) );
	}
	catch( gl::GlslProgCompileExc &exc ) {
		console() << "Shader compile error: " << std::endl;
		console() << exc.what();
	}
	catch( ... ) {
		console() << "Unable to load shader" << std::endl;
	}

	gl::Fbo::Format format;
	format.enableMipmapping(false);
	format.setCoverageSamples(16);
	format.setSamples(4);

	depthBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	depthEdgeBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	normalBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	normalEdgeBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	silhouetteBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	shadedModelBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);

	cabinetRotation = -40.0;
	cabinetScale = 175.0;
}

void DAMEAppApp::mouseDown( MouseEvent event )
{
}

Vec3f DAMEAppApp::getCenterOfCurrentGame(){
	return games[currentGameIndex].getPosition() + Vec3f((float)games[currentGameIndex].getImageWidth()/2, (float)games[currentGameIndex].getImageHeight()/2, 0.0);
}

void DAMEAppApp::keyDown( KeyEvent event ) {
	bool changedGame = false;

	if( event.getCode() == KeyEvent::KEY_ESCAPE ){
        quit();
	} else if(event.getCode() == KeyEvent::KEY_RIGHT){
		if(currentGameIndex < games.size()-1){
			currentGameIndex++;
			changedGame = true;
		}
	} else if(event.getCode() == KeyEvent::KEY_LEFT){
		if(currentGameIndex != 0){
			currentGameIndex--;
			changedGame = true;
		}
	} else if(event.getCode() == KeyEvent::KEY_RETURN){
		runSelectedGame();
	}

	if(changedGame){
		lastBackgroundDraw = -1;
		camera.lookAt(getCenterOfCurrentGame() + Vec3f(0.0f, 0.0f, -500.0f), getCenterOfCurrentGame(), -1*Vec3f::yAxis());
	}
}

void DAMEAppApp::update()
{
	double now = getElapsedSeconds();
	hideCursor();
	//camera.update(now - lastFrameTime);
	/*boost::ptr_list<Animation>::iterator it;
	for(it = animations.begin(); it != animations.end();){
		console() << endl << endl << "Calling tick function on animation " << endl << endl;
		if(!it->isDone()){
			it->tick(now - lastFrameTime);
			it++;
		} else {
			console() << endl << endl << "Removing finished animation " << endl << endl;
			it = animations.erase(it);
		}
	}*/

	lastFrameTime = now;
}

void DAMEAppApp::draw()
{
	drawLine();
}

// Not currently used, but I left it here in case I wanted to go back to it at some point
void DAMEAppApp::drawGrid()
{
	gl::clear( Color( 0, 0, 0 ), true );

	const int gap = 20;
	unsigned int i;
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

void DAMEAppApp::drawBackground(){
	gl::pushModelView();
		gl::translate( Vec2f(0.0, (float)getWindowHeight()) );
		gl::scale( Vec3f(1, -1, 1) );
		phongShader.bind();
		phongShader.uniform("quantize", 1.0f);
		phongShader.uniform("quantLevels", 10.0f);
		phongShader.uniform("lightPosition", Vec3f(0.0f, 0.0f, 5.0f));
			gl::color(Color::white());
			gl::drawSolidRect( getWindowBounds() );
		phongShader.unbind();
	gl::popModelView();

	gl::pushMatrices();
	// Render depth info to a texture
	depthBuffer.bindFramebuffer();
		gl::enableDepthRead();
		gl::enableDepthWrite();
		gl::clear(Color::black(), true);

		depthShader.bind();
			gl::pushMatrices();
				gl::setMatrices(backgroundCamera);
				gl::translate(Vec3f(getWindowWidth()/-8.0f, getWindowHeight()/9.0f, 0.0f));
				gl::rotate(Vec3f(180.0, cabinetRotation, 0.0));
				gl::scale(cabinetScale, cabinetScale, cabinetScale);
				gl::draw(arcadeCabinet);
			gl::popMatrices();
		depthShader.unbind();
	depthBuffer.unbindFramebuffer();

	gl::disableDepthRead();
	gl::disableDepthWrite();

	// Run an edge detection shader against the depth image to get an
	// outline around the object (Note: this will not give you interior
	// edges, we'll get those from the normal rendering below)
	gl::pushModelView();
		depthEdgeBuffer.bindFramebuffer();
			gl::translate( Vec2f(0.0, (float)getWindowHeight()) );
			gl::scale( Vec3f(1, -1, 1) );
			convolutionShader.bind();
				convolutionShader.uniform("depthImage", 0);
				convolutionShader.uniform("textureSizeX", (float)depthBuffer.getTexture().getWidth());
				convolutionShader.uniform("textureSizeY", (float)depthBuffer.getTexture().getHeight());

				depthBuffer.getTexture().bind();
					gl::color(Color::white());
					gl::drawSolidRect( getWindowBounds() );
				depthBuffer.getTexture().unbind();
			convolutionShader.unbind();
		depthEdgeBuffer.unbindFramebuffer();
	gl::popModelView();

	// Render model with faces colored using normals to a texture
	normalBuffer.bindFramebuffer();
		gl::enableDepthRead();
		gl::enableDepthWrite();
		gl::clear(Color::black(), true);

		normalShader.bind();
			gl::pushMatrices();
				gl::setMatrices(backgroundCamera);
				gl::translate(Vec3f(getWindowWidth()/-8.0f, getWindowHeight()/9.0f, 0.0f));
				gl::rotate(Vec3f(180.0, cabinetRotation, 0.0));
				gl::scale(cabinetScale, cabinetScale, cabinetScale);
				gl::draw(arcadeCabinet);
			gl::popMatrices();
		normalShader.unbind();
	normalBuffer.unbindFramebuffer();
    
	gl::clear(Color::white(), true);
	gl::disableDepthRead();
	gl::disableDepthWrite();

	// Now run an edge detection against the normal texture to get edges,
	// including interior ones
	gl::pushModelView();
		normalEdgeBuffer.bindFramebuffer();
			gl::translate( Vec2f(0.0, (float)getWindowHeight()) );
			gl::scale( Vec3f(1, -1, 1) );
			normalEdgeShader.bind();
				normalEdgeShader.uniform("normalImage", 0);
				normalEdgeShader.uniform("textureSizeX", (float)depthBuffer.getTexture().getWidth());
				normalEdgeShader.uniform("textureSizeY", (float)depthBuffer.getTexture().getHeight());
				normalEdgeShader.uniform("normalEdgeThreshold", 0.20f);

				normalBuffer.getTexture().bind();
					gl::color(Color::white());
					gl::drawSolidRect( getWindowBounds() );
				normalBuffer.getTexture().unbind();
			normalEdgeShader.unbind();
		normalEdgeBuffer.unbindFramebuffer();
	gl::popModelView();

	// Now combine the depth edge texture data with the normal edge data,
	// using the stronger value from either texture so that we get a solid 
	// outline and solid interior edges
	gl::pushModelView();
		silhouetteBuffer.bindFramebuffer();
			gl::translate( Vec2f(0.0, (float)getWindowHeight()) );
			gl::scale( Vec3f(1, -1, 1) );
			silhouetteShader.bind();
				silhouetteShader.uniform("normalEdgeImage", 0);
				silhouetteShader.uniform("depthEdgeImage", 1);
				silhouetteShader.uniform("textureSizeX", (float)depthEdgeBuffer.getTexture().getWidth());
				silhouetteShader.uniform("textureSizeY", (float)depthEdgeBuffer.getTexture().getHeight());

				normalEdgeBuffer.getTexture().bind();
				depthEdgeBuffer.getTexture().bind(1);
					gl::color(Color::white());
					gl::drawSolidRect( getWindowBounds() );
				depthEdgeBuffer.getTexture().unbind(1);
				normalBuffer.getTexture().unbind();
			silhouetteShader.unbind();
		silhouetteBuffer.unbindFramebuffer();
	gl::popModelView();

	// Now render the model to a texture
	shadedModelBuffer.bindFramebuffer();
		gl::enableDepthRead();
		gl::enableDepthWrite();
		gl::clear(Color(255.0f, 136.0f/255.0f, 44.0f/255.0f), true);
		phongShader.bind();

		gl::pushMatrices();
			gl::setMatrices(backgroundCamera);
			gl::translate(Vec3f(getWindowWidth()/-8.0f, getWindowHeight()/9.0f, 0.0f));
			gl::rotate(Vec3f(180.0, cabinetRotation, 0.0));
			gl::scale(cabinetScale, cabinetScale, cabinetScale);
			gl::draw(arcadeCabinet);
		gl::popMatrices();
		phongShader.unbind();
	shadedModelBuffer.unbindFramebuffer();

	// Finally, composite the shaded texture with the combined edge
	// textures to get our final, "Toon Shaded" texture.
	depthBuffer.bindFramebuffer();
		gl::pushModelView();
			gl::translate( Vec2f(0.0, (float)getWindowHeight()) );
			gl::scale( Vec3f(1, -1, 1) );
			compositeShader.bind();
				compositeShader.uniform("shadedModelImage", 1);
				compositeShader.uniform("silhouetteImage", 0);
				compositeShader.uniform("silhouetteColor", Vec4f(0.0, 0.0, 0.0, 1.0));

				silhouetteBuffer.getTexture().bind();
				shadedModelBuffer.getTexture().bind(1);
					gl::color(Color::white());
					gl::drawSolidRect( getWindowBounds() );
				silhouetteBuffer.getTexture().unbind(1);
				shadedModelBuffer.getTexture().unbind();
			compositeShader.unbind();
		gl::popModelView();
	depthBuffer.unbindFramebuffer();
	gl::popMatrices();
}

bool DAMEAppApp::backgroundNeedsUpdate(){
	if(getElapsedSeconds() - lastBackgroundDraw > 1){
		lastBackgroundDraw = getElapsedSeconds();
		return true;
	} else {
		return false;
	}
}

void DAMEAppApp::drawLine(){
	gl::clear(Color::black(), true);
	if(backgroundNeedsUpdate()){
		drawBackground();
	}
	
	// Draw foreground to the same texture as background
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::pushMatrices();
		gl::setMatrices(camera);
		depthBuffer.bindFramebuffer();
		unsigned int i;
		const int borderSize = 3;
		for(i=0; i < games.size(); i++){
			if(i == currentGameIndex){
				//gl::scale(Vec3f(2.0, 2.0, 1.0));
				gl::pushMatrices();
				gl::translate(0.0, 0.0, 1.0);
				gl::drawSolidRect(Rectf(games[i].getPosition().x-borderSize, games[i].getPosition().y-borderSize, games[i].getPosition().x + games[i].getImage().getWidth()+borderSize, games[i].getPosition().y + games[i].getImage().getHeight()+borderSize), false);
				gl::popMatrices();
				games[i].getRenderedTitle().enableAndBind();
				gl::draw(games[i].getRenderedTitle(), Vec2f(games[i].getPosition().x + games[i].getImageWidth()/2 - games[i].getRenderedTitle().getWidth()/2, games[i].getPosition().y+games[i].getImageHeight()+25));
				games[i].getRenderedTitle().unbind();
			}
			games[i].enableAndBindImage();
			gl::draw(games[i].getImage(), Vec2f(games[i].getPosition().x, games[i].getPosition().y));
			games[i].getImage().unbind();
		}
		depthBuffer.unbindFramebuffer();
	gl::popMatrices();

	gl::pushModelView();
		gl::translate( Vec2f(0.0f, (float)getWindowHeight()) );
		gl::scale( Vec3f(1, -1, 1) );
		passThruShader.bind();
			depthBuffer.getTexture().bind();
			passThruShader.uniform("texture", 0);
				gl::color(Color::white());
				gl::drawSolidRect( getWindowBounds() );
			depthBuffer.getTexture().unbind();
		passThruShader.unbind();
	gl::popModelView();

	// Draw FPS counter
	gl::pushModelView();
		gl::translate(Vec2f(getWindowWidth() - 100.0f, 50.0f));
		TextLayout layout;
		layout.setFont(Font( loadResource( RES_AKASHI_FONT ), 30));
		layout.setColor( Color( 1.0f, 1.0f, 1.0f) );
		layout.addCenteredLine(toString((int)getAverageFps()));
		gl::draw(gl::Texture(layout.render(true, true)));
	gl::popModelView();
}

void DAMEAppApp::runSelectedGame(){
	console() << endl << endl << endl;
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
	wstringstream ss;
	ss << mamePath.c_str() << "/mame.exe -rompath " << romPath.c_str() << " mk";
	wstring mame = ss.str();

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
