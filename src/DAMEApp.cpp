#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/Timeline.h"
#include "cinder/Sphere.h"
#include "Game.h"
#include "Console.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Text.h"
#include "cinder/Xml.h"
#include "Resources.h"
#include "cinder/Utilities.h"
//#include "Resources.h"

#include <vector>
#include <string>
#include <tchar.h>
#include <sstream>
#include <fstream>
#include <iostream>

#include <process.h>
#include <Windows.h>
#include <conio.h>

#include "boost\program_options\parsers.hpp"
#include "boost\program_options\variables_map.hpp"
#include "boost\ptr_container\ptr_list.hpp"
#include "boost\filesystem\config.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;
namespace bfs = boost::filesystem;
namespace po = boost::program_options;

class DAMEApp : public AppBasic {
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
    void resize(ResizeEvent event);
    void runSelectedGame();
    bool backgroundNeedsUpdate();
    Vec3f getCenterOfCurrentGame();

    enum { CONSOLE_SELECT, GAME_SELECT };
    int gameMode;

    int CONSOLES[2];

    CameraPersp camera;
    CameraPersp backgroundCamera;
    Anim<Vec3f> cameraPos;
    Anim<Vec3f> backgroundCameraPos;
    Anim<Vec3f> backgroundCameraLookAt;
    unsigned int currentGameIndex;
    unsigned int currentConsoleIndex;
    double lastFrameTime;

    vector<string> gameNames;
    vector<string> gameTitles;
    vector<Game> games;
    vector<Console> consoles;

    string mamePath;
    string romPath;

    // Shaders for Toon Shading of background
    gl::GlslProg depthShader;
    gl::GlslProg convolutionShader;
    gl::GlslProg normalShader;
    gl::GlslProg normalEdgeShader;
    gl::GlslProg silhouetteShader;
    gl::GlslProg compositeShader;
    gl::GlslProg passThruShader;
    gl::GlslProg phongShader;

    gl::Fbo::Format format;

    gl::Fbo normalBuffer;
    gl::Fbo normalEdgeBuffer;
    gl::Fbo depthBuffer;
    gl::Fbo depthEdgeBuffer;
    gl::Fbo silhouetteBuffer;
    gl::Fbo shadedModelBuffer;
};

void DAMEApp::prepareSettings( Settings *settings )
{
    settings->setFullScreen(true);
    settings->setFrameRate( 60.0f );
}

void DAMEApp::setup()
{
    gameMode = CONSOLE_SELECT;

    // Load mame path and mame rom path
    po::variables_map configOptions;
    // Setup options.
    po::options_description desc("Options");
    desc.add_options()
        ("mamePath", po::value< string >( &mamePath ), "mamePath" )
        ("romPath", po::value< string >( &romPath ), "romPath");

    ifstream settings_file( "settings.ini" );
    po::store( po::parse_config_file( settings_file , desc ), configOptions );
    settings_file.close();
    po::notify( configOptions );

    bfs::path someDir(romPath.c_str());
    bfs::directory_iterator end_iter;

    // Load xml file to use as title lookup
    double start = getElapsedSeconds();

    console() << endl << endl << "Loading mame game info from " << mamePath + "/mygames_test.xml" << "..." << endl << endl;
    XmlTree mameInfo(loadAsset("names-to-titles.xml"));

    double minutesToParse = getElapsedSeconds() - start / 60.0;
    console() << endl << endl << "Mame info loaded (took " << minutesToParse << " minutes)." << endl << endl;

    start = getElapsedSeconds();
    console() << endl << endl << "Loading games..." << endl << endl;

    // Search rom path for roms
    if ( bfs::exists(someDir) && bfs::is_directory(someDir))
    {
      for( bfs::directory_iterator dir_iter(someDir) ; dir_iter != end_iter ; ++dir_iter)
      {
        if (bfs::is_regular_file(dir_iter->status()))
        {
            if(dir_iter->path().extension() == ".zip"){
                string name = dir_iter->path().filename().string();
                name = name.substr(0, name.find(".zip"));
                console() << "Found rom: " << name << endl;

                string title = name;

                for( XmlTree::Iter game = mameInfo.begin("games/game"); game != mameInfo.end(); ++game ){
                    if(game->getAttributeValue<string>("name") == name){
                        title = game->getValue();
                        break;
                    }
                }
                gameTitles.push_back(title);
                gameNames.push_back(name);
            }
        }
      }
    }

    minutesToParse = getElapsedSeconds() - start / 60.0;
    console() << endl << endl << "Games loaded (took " << minutesToParse << " minutes)." << endl << endl;

    unsigned int i = 0;
    for(; i < gameNames.size(); i++){
      games.push_back(Game(gameNames[i]));
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
        games[i].setTitle(gameTitles[i]);
    }

    currentGameIndex = 0;
    currentConsoleIndex = 0;

    camera.setPerspective(60.0f, getWindowAspectRatio(), 5.0f, 5000.0f);
    backgroundCamera.setPerspective(60.0f, getWindowAspectRatio(), 5.0f, 5000.0f);
    backgroundCameraPos = Vec3f(0.0f, 0.0f, -1000.0f);
    backgroundCameraLookAt = Vec3f::zero();
    backgroundCamera.lookAt(backgroundCameraPos, backgroundCameraLookAt, -1*Vec3f::yAxis());

    Vec3f centerOfFirstGame = getCenterOfCurrentGame();
    cameraPos = centerOfFirstGame + Vec3f(0.0f, 0.0f, -500.0f);
    camera.lookAt(cameraPos, getCenterOfCurrentGame(), -1*Vec3f::yAxis());

    lastFrameTime = getElapsedSeconds();

    // Load Consoles
    Console MAME;
    MAME.setAsset("cab.obj");
    MAME.initialize();
    MAME.setRotation(-40.0f);
    MAME.setScale(175.0f);
    MAME.setPosition(Vec3f::zero());
    consoles.push_back(MAME);

    Console SNES;
    SNES.setAsset("snes.obj");
    SNES.initialize();
    SNES.setRotation(40.0f);
    SNES.setScale(150.0f);
    SNES.setPosition(Vec3f(650.0f, 0.0f, 0.0f));
    consoles.push_back(SNES);

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
        console() << "Shader compile error: " << endl;
        console() << exc.what();
    }
    catch( ... ) {
        console() << "Unable to load shader" << endl;
    }

    format.enableMipmapping(false);
    format.setCoverageSamples(0);
    format.setSamples(0);

    depthBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    depthEdgeBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    normalBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    normalEdgeBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    silhouetteBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    shadedModelBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
}

void DAMEApp::mouseDown( MouseEvent event )
{
}

Vec3f DAMEApp::getCenterOfCurrentGame(){
    return games[currentGameIndex].getPosition() + Vec3f((float)games[currentGameIndex].getImageWidth()/2, (float)games[currentGameIndex].getImageHeight()/2, 0.0);
}

void DAMEApp::keyDown( KeyEvent event ) {
    if(gameMode == CONSOLE_SELECT){
        bool changedConsole = false;

        if( event.getCode() == KeyEvent::KEY_ESCAPE ){
            quit();
        } else if(event.getCode() == KeyEvent::KEY_RIGHT){
            if(currentConsoleIndex < consoles.size()-1){
                currentConsoleIndex++;
                changedConsole = true;
            }
        } else if(event.getCode() == KeyEvent::KEY_LEFT){
            if(currentConsoleIndex != 0){
                currentConsoleIndex--;
                changedConsole = true;
            }
        } else if(event.getCode() == KeyEvent::KEY_RETURN){
            gameMode = GAME_SELECT;
        }

        if(changedConsole){
            timeline().apply( &backgroundCameraPos, Vec3f(consoles[currentConsoleIndex].getPosition().x, backgroundCameraPos().y, backgroundCameraPos().z), 0.5f, EaseOutCubic() );
            timeline().apply( &backgroundCameraLookAt, consoles[currentConsoleIndex].getPosition(), 0.5f, EaseOutCubic());
        }
    } else if(gameMode == GAME_SELECT){
        bool changedGame = false;

        if(event.getCode() == KeyEvent::KEY_ESCAPE){
            gameMode = CONSOLE_SELECT;
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
            timeline().apply( &cameraPos, Vec3f(getCenterOfCurrentGame().x, cameraPos().y, cameraPos().z), 0.5f, EaseOutCubic() );
        }
    }
}

void DAMEApp::update()
{
    double now = getElapsedSeconds();
    double lessNow = now / 5;
    hideCursor();
    camera.setEyePoint(cameraPos);
    //backgroundCameraPos = Vec3f(sin(lessNow + cos(lessNow * 0.83)) * 300.0 + 300.0, sin(lessNow + cos(lessNow * 0.73)) * 500.0 - 500.0, sin(lessNow + cos(lessNow * 0.41)) * 500.0 - 1000.0);
    backgroundCamera.setEyePoint(backgroundCameraPos);
    // NOTE: Should be looking at "currentConsole" once it exists...
    backgroundCamera.lookAt(backgroundCameraPos, backgroundCameraLookAt, -1*Vec3f::yAxis());
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

void DAMEApp::resize(ResizeEvent event){
    depthBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    depthEdgeBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    normalBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    normalEdgeBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    silhouetteBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
    shadedModelBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
}

void DAMEApp::draw()
{
    drawLine();
}

// Not currently used, but I left it here in case I wanted to go back to it at some point
void DAMEApp::drawGrid()
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

void DAMEApp::drawBackground(){
    unsigned int i = 0;
    gl::pushMatrices();
    // Render depth info to a texture
    gl::color(Color::white());
    depthBuffer.bindFramebuffer();
        gl::enableDepthRead();
        gl::enableDepthWrite();
        gl::clear(Color::black(), true);

        depthShader.bind();
            for(i=0; i < consoles.size(); i++){
                gl::pushMatrices();
                    gl::setMatrices(backgroundCamera);
                    gl::translate(consoles[i].getPosition());
                    gl::rotate(Vec3f(180.0, consoles[i].getRotation(), 0.0));
                    gl::scale(consoles[i].getScale(), consoles[i].getScale(), consoles[i].getScale());
                    gl::draw(consoles[i].getMesh());
                gl::popMatrices();
            }
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
            i = 0;
            for(i=0; i < consoles.size(); i++){
                gl::pushMatrices();
                    gl::setMatrices(backgroundCamera);
                    gl::translate(consoles[i].getPosition());
                    gl::rotate(Vec3f(180.0, consoles[i].getRotation(), 0.0));
                    gl::scale(consoles[i].getScale(), consoles[i].getScale(), consoles[i].getScale());
                    gl::draw(consoles[i].getMesh());
                gl::popMatrices();
            }
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

    // Render "sky box" sphere as background
    gl::enable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    shadedModelBuffer.bindFramebuffer();
        phongShader.bind();
        phongShader.uniform("drawColor", Vec4f(1.0f, 136.0f/255.0f, 44.0f/255.0f, 1.0f));
        phongShader.uniform("colorInside", 1.0f);
        phongShader.uniform("quantize", 0.0f);
        phongShader.uniform("lightPosition", Vec3f(0.0f, 0.0f, -1500.0f));
        phongShader.uniform("attenuationFactor", 0.0000002f);
        gl::pushMatrices();
            gl::clear(Color::black());
            gl::setMatrices(backgroundCamera);
            gl::draw(Sphere(Vec3f(0.0f, 0.0f, 0.0f), 2000.0f));
        gl::popMatrices();
        phongShader.unbind();
    shadedModelBuffer.unbindFramebuffer();
    gl::disable(GL_CULL_FACE);

    // Now render the model to a texture
    shadedModelBuffer.bindFramebuffer();
        gl::enableDepthRead();
        gl::enableDepthWrite();
        phongShader.bind();
        phongShader.uniform("drawColor", Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
        phongShader.uniform("colorInside", 0.0f);
        phongShader.uniform("quantize", 1.0f);
        phongShader.uniform("quantLevels", 10.0f);
        phongShader.uniform("lightPosition", Vec3f(30.0f, 100.0f, 50.0f));
        phongShader.uniform("attenuationFactor", 0.0000007f);

        i = 0;
        for(i=0; i < consoles.size(); i++){
            gl::pushMatrices();
                gl::setMatrices(backgroundCamera);
                gl::translate(consoles[i].getPosition());
                gl::rotate(Vec3f(180.0, consoles[i].getRotation(), 0.0));
                gl::scale(consoles[i].getScale(), consoles[i].getScale(), consoles[i].getScale());
                gl::draw(consoles[i].getMesh());
            gl::popMatrices();
        }
        phongShader.unbind();
    shadedModelBuffer.unbindFramebuffer();

    gl::disable(GL_CULL_FACE);

    // Finally, composite the shaded texture with the combined edge
    // textures to get our final, "Toon Shaded" texture.
    // NOTE: We're reusing texture 0 here...
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

void DAMEApp::drawLine(){
    gl::clear(Color::black(), true);
    drawBackground();

    if(gameMode == GAME_SELECT){
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
                }
                games[i].enableAndBindImage();
                gl::draw(games[i].getImage(), Vec2f(games[i].getPosition().x, games[i].getPosition().y));
                games[i].getImage().unbind();
            }
            depthBuffer.unbindFramebuffer();
        gl::popMatrices();
    }

    // Draw full screen rect with texture
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

    // Draw game title
    gl::pushModelView();
        gl::translate(Vec2f(getWindowWidth()/2.0f - games[currentGameIndex].getRenderedTitle().getWidth()/2.0f, getWindowHeight()-games[currentGameIndex].getRenderedTitle().getHeight() - 20.0f));
        gl::enableAlphaBlending();
        games[currentGameIndex].getRenderedTitle().enableAndBind();
        gl::draw(games[currentGameIndex].getRenderedTitle());
        games[currentGameIndex].getRenderedTitle().unbind();
        gl::disableAlphaBlending();
    gl::popModelView();

    // Draw FPS counter
    gl::pushModelView();
        gl::translate(Vec2f(getWindowWidth() - 100.0f, 50.0f));
        TextLayout layout;
        layout.setFont(Font( loadResource( RES_AKASHI_FONT ), 48));
        layout.setColor( Color( 1.0f, 1.0f, 1.0f) );
        layout.clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));
        layout.addCenteredLine(toString((int)getAverageFps()));
        gl::enableAlphaBlending();
        gl::draw(gl::Texture(layout.render(true, true)));
        gl::disableAlphaBlending();
    gl::popModelView();
}

void DAMEApp::runSelectedGame(){
    console() << endl << endl << endl;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process.
    wstringstream ss;

    console() << "Mame path: " << mamePath.c_str() << endl << endl;
    ss << mamePath.c_str() << "/mame.exe -rompath " << romPath.c_str() << " " << gameNames[currentGameIndex].c_str();
    wstring mame = ss.str();

    // We have to drop out of full screen to run mame
    setFullScreen(false);
    setWindowSize(800, 600);

    if( !CreateProcess(NULL,    // Use the command line
        &mame[0],       // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,          // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // User parent's directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
        console() << "CreateProcess failed: " << GetLastError() << endl << endl;
        setFullScreen(true);
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

CINDER_APP_BASIC( DAMEApp, RendererGl(0) )
