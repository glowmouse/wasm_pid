/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <emscripten.h>
#include "model.h"  // auto generated code.

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>
#include <chrono>

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

class GLTexture {
public:
    using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
    GLTexture() = default;
    GLTexture(const std::string& textureName)
        : mTextureName(textureName), mTextureId(0) {}

    GLTexture(const std::string& textureName, GLint textureId)
        : mTextureName(textureName), mTextureId(textureId) {}

    GLTexture(const GLTexture& other) = delete;
    GLTexture(GLTexture&& other) noexcept
        : mTextureName(std::move(other.mTextureName)),
        mTextureId(other.mTextureId) {
        other.mTextureId = 0;
    }
    GLTexture& operator=(const GLTexture& other) = delete;
    GLTexture& operator=(GLTexture&& other) noexcept {
        mTextureName = std::move(other.mTextureName);
        std::swap(mTextureId, other.mTextureId);
        return *this;
    }
    ~GLTexture() noexcept {
        if (mTextureId)
            glDeleteTextures(1, &mTextureId);
    }

    GLuint texture() const { return mTextureId; }
    const std::string& textureName() const { return mTextureName; }

    /**
    *  Load a file in memory and create an OpenGL texture.
    *  Returns a handle type (an std::unique_ptr) to the loaded pixels.
    */
    handleType load(const std::string& fileName) {
        if (mTextureId) {
            glDeleteTextures(1, &mTextureId);
            mTextureId = 0;
        }
        int force_channels = 0;
        int w, h, n;
        handleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);
        if (!textureData){
            throw std::invalid_argument("Could not load texture data from file " + fileName);}
        glGenTextures(1, &mTextureId);
        glBindTexture(GL_TEXTURE_2D, mTextureId);
        GLint internalFormat;
        GLint format;
        switch (n) {
            case 1: internalFormat = GL_R8; format = GL_RED; break;
            case 2: internalFormat = GL_RG8; format = GL_RG; break;
            case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
            case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
            default: internalFormat = 0; format = 0; break;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return textureData;
    }

private:
    std::string mTextureName;
    GLuint mTextureId;
};

//
// Some helper/ utility functions
//

///
/// @brief Convert Degrees to Radians
/// 
/// @param[in] degrees - An angle in degrees
/// @return            - Same angle in radians
///
double degToRad( double degrees ) 
{
  return degrees / 180.0 * M_PI; 
}

///
/// @brief Convert Radians to Degrees
/// 
/// @param[in] radians - An angle in radians
/// @return            - Same angle in degrees
///
double radToDeg( double radians ) 
{
  return radians / M_PI * 180.0;
}


nanogui::TextBox* make_slider( 
  nanogui::Widget*                    window,
  const std::string&                  labelText,
  float                               slider_start_pos,
  std::function<std::string(float)>   slider_to_label,
  const std::string&                  units,
  bool                                noSlider = false
)
{
  using namespace nanogui;
  Widget *panel = new Widget(window);
  panel->setLayout(new BoxLayout(Orientation::Horizontal,
      Alignment::Middle, 0, 20));
  Label *label = new Label(panel, labelText, "sans-bold");
  label->setFixedWidth(120);

  Slider *slider;
  if ( !noSlider ) {
    slider = new Slider(panel);
    slider->setValue(slider_start_pos);
    slider->setFixedWidth(80);
  }
  else {
    Widget *deadSpace = new Widget(panel);
    deadSpace->setFixedWidth(80);
  }

  TextBox *textBox = new TextBox(panel);
  textBox->setFixedSize(Vector2i(90, 25));
  textBox->setValue(slider_to_label(slider_start_pos));
  textBox->setUnits(units);

  if ( !noSlider ) {
    slider->setCallback([textBox,slider_to_label](float value) 
      { textBox->setValue(slider_to_label(value)); } 
    );
    slider->setFinalCallback([slider_to_label](float value) {
      cout << "Final slider value: " << slider_to_label(value) << endl;
    });
  }
  textBox->setFixedSize(Vector2i(80,25));
  textBox->setFontSize(20);
  textBox->setAlignment(TextBox::Alignment::Right);
  return textBox;
}

//
// @brief OpenGL style Vertex Shader 
//
// @param[in]  projection - Maps from camera space to screen space
// @param[in]  camera     - Maps from global space to camera space
// @param[in]  model      - Maps from model local space to global space
// @param[in]  normal     - All model normals, in model space
// @param[in]  position   - All model positions, in model space
// @param[out] FragNormal - All model normals in global space.  For shading.
// @param[out] FragPos    - All model positions in global space.  For shading.
//
const std::string vertexShader = 
R"(#version 300 es
#ifdef GL_ES
  precision highp float;
#endif
uniform mat4 projection;
uniform mat4 camera;
uniform mat4 model;
in vec3 normal;
in vec3 position;
out vec3 FragNormal;
out vec3 FragPos;

void main() {
  gl_Position = projection * camera * model * vec4(position, 1.0);
  FragNormal = vec3(model * vec4( normal,   0.0 ));
  FragPos    = vec3(model * vec4( position, 1.0 ));
}

)";

//
// @brief OpenGL style Fragment Shader
//
// @param[in]  lightpos   - Single point light source in global space
// @param[in]  vewpos     - Location of camera in global space
// @param[in]  FragNormal - All model normals in global space
// @param[in]  FragPos    - All model positions in global space
// @param[out] color      - The shaded color
//
const std::string fragmentShader =
R"(#version 300 es
#ifdef GL_ES
  precision highp float;
#endif
uniform vec3 lightpos;
uniform vec3 viewpos;
in vec3 FragNormal;
in vec3 FragPos;
out vec4 color;

//
// Three classic lighting models, ambient, diffuse, and specular.
// 
// TODO, maybe accept as arguments instead of hard coding
//
float diffuseScale  = .4;
float ambientScale  = .3;
float specularScale = .5;

// Use lightBlue for ambient & diffuse, white for specilar.
//
vec3 lightBlue    = vec3(  .5,   .5, 1.0);
vec3 white        = vec3( 1.0, 1.0, 1.0); 

void main() {
  //
  // Diffuse calculation
  //
  vec3 lightdir   = normalize( lightpos - FragPos );
  float diffuse   = max(dot(FragNormal, lightdir), 0.0 );

  //
  // Specular calculation
  //
  vec3 viewdir    = normalize( viewpos - FragPos );
  vec3 reflectdir = reflect( -lightdir, FragNormal);
  float spec      = pow(max(dot(viewdir, reflectdir), 0.0), 32.0 );

  //
  // Combine into a single RGB value
  //
  vec3 colorRGB  = lightBlue * ambientScale +
                   lightBlue * diffuse * diffuseScale +
                   white     * spec    * specularScale;

  //
  // Output in RGBA.  The A (Alpha) is 1.0 for non-transparent.
  // 
  color           = vec4( colorRGB, 1.0 );
}
)";

class PidSimFrontEnd: public nanogui::Screen {
public:
  PidSimFrontEnd() : 
      nanogui::Screen(
        Eigen::Vector2i(800, 600), 
        "NanoGUI Test", 
        /*resizable*/   true, 
        /*fullscreen*/  false, 
        /*colorBits*/   8,
        /*alphaBits*/   8,
        /*depthBits*/   24,
        /*stencilBits*/ 8,
        /*nSamples*/    0,
        /*glMajor*/     3,
        /*glMinor*/     0) 
  {
    initModel();
    using namespace nanogui;
    Window *window = new Window(this, "Robot Arm PID Simulator");
    window->setPosition(Vector2i(15, 15));
    window->setLayout(new GroupLayout());
    window->setFixedSize(Vector2i( mSize.x()/2-30, mSize.y()-30 ));

    /* No need to store a pointer, the data structure will be automatically
       freed when the parent window is deleted */

    // Same lambdas to format text.
    auto sliderToFloat= []( float slider, float scale ) 
    {
      std::stringstream stream;
      stream << std::fixed << std::setprecision(1) << ( slider * scale );
      return stream.str();
    };
    auto sliderToInt = []( float slider, int scale )
    {
      return std::to_string((int) ( slider * scale ));
    };
    auto sliderToDegrees = [sliderToInt]( float slider ) 
    {
      return sliderToInt( slider, 90 );
    };
    auto sliderTo1000ms = [sliderToInt]( float slider ) 
    {
      return sliderToInt( slider, 1000 );
    };
    auto sliderTo10s = [sliderToFloat]( float slider ) 
    {
      return sliderToFloat( slider, 10.0 );
    };
    auto sliderToPid = [sliderToFloat]( float slider ) {
      return sliderToFloat( slider, 4.0 );
    };

    new Label(window, "Arm Start Position", "sans-bold");
    mAngleStart   = make_slider( window, "Start Arm Angle", 0.0, sliderToDegrees, "deg" );
    mAngleTarget  = make_slider( window, "Target Arm Angle",  1.0, sliderToDegrees, "deg" );

    new Label(window, "Arm Current Position", "sans-bold");
    mAngleCurrent = make_slider( window, "Arm Angle", 0.0, sliderToDegrees, "deg", true );

    new Label(window, "PID Settings", "sans-bold");
    mPID_P        = make_slider( window, "P", .5, sliderToPid, "");
    mPID_I        = make_slider( window, "I", .5, sliderToPid, "");
    mPID_D        = make_slider( window, "D", .5, sliderToPid, "");

    new Label(window, "Simulation Settings", "sans-bold");
    mSensorDelay  = make_slider( window, "Sensor Delay", .1, sliderTo1000ms, "ms" );
    mIMemory      = make_slider( window, "I Memory", .5, sliderTo10s, "s" );

    new Label(window, "Simulation Control", "sans-bold" );

    Widget *panel = new Widget(window);
    panel->setLayout(new BoxLayout(Orientation::Horizontal,
        Alignment::Middle, 0, 20));
    mStart= new Button( panel, "Start" );
    mStart->setFlags( Button::ToggleButton );
    auto reset = new Button( panel, "Reset" );
    reset->setCallback( [&] (void) { mReset =true; }); 

    Window *chartWindow = new Window(this, "PID Stats over Time");
    chartWindow->setPosition(Vector2i( mSize.x()/2, 15));
    chartWindow->setFixedSize(Vector2i( mSize.x()/2-15, mSize.y()*.4-30 ));

    performLayout();

    /* All NanoGUI widgets are initialized at this point. Now
       create an OpenGL shader to draw the main window contents.

       NanoGUI comes with a simple Eigen-based wrapper around OpenGL 3,
       which eliminates most of the tedious and error-prone shader and
       buffer object management.
    */
  
    mShader.init( "shader", vertexShader, fragmentShader ); 

    Matrix4f light;
    light.col(0) <<  0.5,  0.5, 0.5, 0.0;
    light.col(1) <<  -0.5,  -0.5, -0.5, 0.0;
    light.col(2) <<  .5,  .5, .5, 0.0;
    light.col(3) <<  0.0,  0.0, 0.0, 1.0;

    Vector3f lightpos;
    lightpos << 10, -40, 50;
    //lightpos << 5, -15, 10;
    // 10, 0, 25
    // TODO, not found in fragment shader unless set here.  Pass through vertex shader?
    // TODO, is not found period.
    Vector3f viewpos;
    viewpos << 0, 15, 40;

    mShader.bind();
    mShader.uploadIndices(indices);
    mShader.uploadAttrib("position", positions);
    mShader.uploadAttrib("normal", normals );
    mShader.setUniform("viewpos", viewpos);
    mShader.setUniform("lightpos", lightpos );
  }

  ~PidSimFrontEnd() 
  {
    mShader.free();
  }

  virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) 
  {
    if (Screen::keyboardEvent(key, scancode, action, modifiers))
      return true;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		  std::cout<<"Exit(ESC) called"<<std::endl;
      //setVisible(false);
      return true;
    }
    return false;
  }

  virtual void draw(NVGcontext *ctx) {
    /* Draw the user interface */
    Screen::draw(ctx);
  }

  virtual void drawContents() {
    using namespace nanogui;

    double angleRad = -mArmAngle + M_PI;

    double viewPortWidth =mSize.x()/2;
    double viewPortHeight=mSize.y()*2/3;

    Matrix4f baseModel;
    baseModel.setIdentity();
    //baseModel.topLeftCorner<3,3>() = Matrix3f(Eigen::AngleAxisf((float) glfwGetTime(),  Vector3f::UnitZ()));

    Matrix4f armLocal;
    armLocal.setIdentity();
    armLocal.topLeftCorner<3,3>() = Matrix3f(Eigen::AngleAxisf(angleRad,  Vector3f::UnitY()));
    armLocal(0,3) = 10.6066;    // X Join Point
    armLocal(1,3) = 0;          // Y Join Point
    armLocal(2,3) = 25.6066;    // Z Join Point
    Matrix4f armModel = baseModel * armLocal;

    Matrix4f camera;
    camera.setIdentity();
    camera.topLeftCorner<3,3>() = Matrix3f(Eigen::AngleAxisf(-M_PI/2 + M_PI/8,  Vector3f::UnitX()));
    camera(0,3) = -7*1.4;
    camera(1,3) = -12*1.4;
    camera(2,3) = -40*1.4;
    Vector3f viewpos;
    viewpos << 0, 15, 40;

    double aspect = viewPortWidth / viewPortHeight;
    double fov = M_PI/6;   // 30 degrees
    double near = 1;
    double far = 400;

    Matrix4f projection;
    projection.setIdentity();
    projection(0,0) = near/ ( aspect * tan(fov/2));
    projection(1,1) = near/ ( tan(fov/2));
    projection(2,2) = -(far+near)   / (far-near);
    projection(3,2) =  (-2*far*near)/ (far-near);
    projection(2,3) = -1;
    projection(3,3) = 0;

    glViewport( 
      mSize.x()/2, 0,
      viewPortWidth, viewPortHeight );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );

    /* Draw the window contents using OpenGL */
    mShader.bind();
    mShader.setUniform("projection", projection);
    mShader.setUniform("camera", camera);
    //mShader.setUniform("viewpos", viewpos);
    mShader.setUniform("model", baseModel );
    mShader.drawIndexed(GL_TRIANGLES, base_TRIANGLE_START, base_TRIANGLE_END);

    mShader.bind();
    mShader.setUniform("projection", projection);
    mShader.setUniform("camera", camera);
    //mShader.setUniform("viewpos", viewpos);
    mShader.setUniform("model", armModel );
    mShader.drawIndexed(GL_TRIANGLES, arm_TRIANGLE_START, arm_TRIANGLE_END - arm_TRIANGLE_START );
  }

  bool isReset()
  {
    bool result = mReset;
    mReset=false;
    return result;
  }

  void setArmAngle( double angle ) {
    int intAngle = angle;
    mAngleCurrent->setValue( std::to_string( intAngle ));
    double angleDegrees = angle;
    mArmAngle = angleDegrees / 180.0 * M_PI;
  }

private:
  double              mArmAngle = 0.0;
  bool                mReset = false;
  nanogui::GLShader   mShader;
  nanogui::TextBox*   mAngleTarget; 
  nanogui::TextBox*   mAngleStart; 
  nanogui::TextBox*   mAngleCurrent; 
  nanogui::TextBox*   mPID_P; 
  nanogui::TextBox*   mPID_I; 
  nanogui::TextBox*   mPID_D; 
  nanogui::TextBox*   mSensorDelay;
  nanogui::TextBox*   mIMemory;
  nanogui::Button*    mStart;
};

class PidSimBackEnd
{
  public:

  PidSimBackEnd( nanogui::ref<PidSimFrontEnd> frontEnd ) : mfrontEnd{ frontEnd }
  {}

  void update( std::chrono::duration<double> delta )
  {
    double intervalSeconds= delta.count();
    int lastTicks= (int) (time * 50);
    time += intervalSeconds;
    int curTicks= (int) (time * 50);
    int ticksDiff = curTicks - lastTicks;
    for ( int i = 0; i < ticksDiff; ++i ) {
      updateOneTick();
    }
  } 
 
  private:

  void reset()
  {
    mAngle = 170;
    mAngleVel= 0;
  }

  void updateOneTick()
  {
    if ( mfrontEnd->isReset() ) {
      reset();
    }
    double mRadAngle = ( mAngle - 90 ) / 180 * M_PI;
    double AngleAccel= -cos( mRadAngle ) * .01;
    mAngleVel += AngleAccel;
    mAngleVel *= .999; // damping
    mAngle += mAngleVel;
    if ( mAngle < -30 ) {
      mAngle = -30;
      mAngleVel = 0.0f;   // Hit the bottom of the holder arm
    }

    updateFrontEnd();
  }

  void updateFrontEnd()
  {
    mfrontEnd->setArmAngle( mAngle );
  }

  double time = 0.0f;
  double mAngle = 0;
  double mAngleVel = 0;

  nanogui::ref<PidSimFrontEnd> mfrontEnd;
};

std::unique_ptr<PidSimBackEnd> backEndSingleton;

static std::chrono::high_resolution_clock::time_point lastFpsTime;
static std::chrono::high_resolution_clock::time_point lastTickTime;
double frameRateSmoothing = 1.0;
double numFrames = 0;

void mainloop(){
	std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>> (std::chrono::high_resolution_clock::now() - lastFpsTime);
  std::chrono::high_resolution_clock::time_point timeNow = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> backDelta = std::chrono::duration_cast<std::chrono::duration<double>> (timeNow - lastTickTime );
  lastTickTime = timeNow;

	numFrames++;
	if (delta.count() > frameRateSmoothing) {
    numFrames = 0;
    lastFpsTime = std::chrono::high_resolution_clock::now();
  }

  backEndSingleton->update( backDelta ); 
	nanogui::mainloop();
}

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();
        {
          //
          // Note - ref is a custom shared pointer.  The reference
          // count is integrated into nanogui's object.
          //
          nanogui::ref<PidSimFrontEnd> pidSimFrontEnd = new PidSimFrontEnd();
          backEndSingleton = std::make_unique<PidSimBackEnd>( pidSimFrontEnd );
          pidSimFrontEnd->drawAll();
          pidSimFrontEnd->setVisible(true);
          emscripten_set_main_loop(mainloop, 0,1);
        }
        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}
