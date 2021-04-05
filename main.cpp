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
vec3 objcolor     = vec3(  .5,   .5, 1.0);
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
  vec3 colorRGB  = objcolor  * ambientScale +
                   objcolor  * diffuse * diffuseScale +
                   white     * spec    * specularScale;

  //
  // Output in RGBA.  The A (Alpha) is 1.0 for non-transparent.
  // 
  color           = vec4( colorRGB, 1.0 );
}
)";

const std::string vertexShaderGrapher =
R"(#version 300 es
#ifdef GL_ES
  precision highp float;
#endif
in vec4 position;
out float colorIndex;
out vec3 FragNormalIn;
out vec3 FragPos;

void main() {
  FragPos = vec3(position.x, position.y*.04, 0.0 );
  gl_Position = vec4(FragPos, 1 );
  colorIndex = position.w;
  FragNormalIn = vec3( 0.0, position.z, -(1.0-abs(position.z)) );
}
)";

const std::string fragmentShaderGrapher=
R"(#version 300 es
#ifdef GL_ES
  precision highp float;
#endif
out vec4 color;
in float colorIndex;
in vec3 FragPos;
in vec3 FragNormalIn;

//
// Three classic lighting models, ambient, diffuse, and specular.
// 
// TODO, maybe accept as arguments instead of hard coding
//
float diffuseScale  = .4;
float ambientScale  = .3;
float specularScale = .5;

vec3 lightpos     = vec3( 1.0, 1.0, -5.0 );
vec3 white        = vec3( 1.0, 1.0, 1.0); 
vec3 objcolor;
vec3 viewpos      = vec3( 0.0, 0.0, -5.0);
float alpha;

void main() {
  if ( colorIndex > .99 && colorIndex < 1.01 ) {
    objcolor  = vec3( 1.0, 0.2, 0.2 );
    alpha = 1.0;
  }
  else if ( colorIndex > 1.99 && colorIndex < 2.01 ) {
    objcolor  = vec3( 0.0, 0.0, 1.0 );
    alpha = 1.0;
  }
  else {
    objcolor  = vec3( 0.0, 0.0, 0.0 );
    alpha = 0.0;
  }

  vec3 FragNormal = normalize( FragNormalIn );

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
  vec3 colorRGB  = objcolor  * ambientScale +
                   objcolor  * diffuse * diffuseScale +
                   white     * spec    * specularScale * alpha;

  //
  // Output in RGBA.  The A (Alpha) is 1.0 for non-transparent.
  // 
  color           = vec4( colorRGB, alpha );
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
        /*glMinor*/     0),
      graphPositions{ 4, numGraphPositions },
      graphIndices{ 3, numGraphIndices}
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
    auto sliderToFloat= []( double& storage, float slider, float scale ) 
    {
      std::stringstream stream;
      storage = slider * scale;
      stream << std::fixed << std::setprecision(1) << ( storage );
      return stream.str();
    };
    auto sliderToInt = []( double& storage, float slider, int scale, int offset=0 )
    {
      storage = slider * scale + offset;
      return std::to_string((int) ( storage ));
    };
    auto sliderToDegrees = [sliderToInt]( double& storage, float slider ) 
    {
      return sliderToInt( storage, slider, 180, -90 );
    };
    auto sliderTo1000ms = [sliderToInt]( double& storage, float slider ) 
    {
      return sliderToInt( storage, slider, 1000 );
    };
    auto sliderTo10s = [sliderToFloat]( double& storage, float slider ) 
    {
      return sliderToFloat( storage, slider, 10.0 );
    };
    auto sliderToPid = [sliderToFloat]( double& storage, float slider ) {
      return sliderToFloat( storage, slider, 4.0 );
    };

    new Label(window, "Arm Start Position", "sans-bold");
    make_slider( window, "Start Arm Angle", 0.0, 
      [&](float slider) { return sliderToDegrees( mStartAngle, slider ); } ,
      "deg" );

    make_slider( window, "Target Arm Angle",  0.5,
      [&](float slider) { return sliderToDegrees( mTargetAngle, slider ); }, 
      "deg" );

    new Label(window, "Arm Current Position", "sans-bold");
    mAngleCurrent = make_slider( window, "Arm Angle", 0.0, 
      [&](float slider) { return ""; }, "deg", true );

    new Label(window, "PID Settings", "sans-bold");
    make_slider( window, "P", 0, 
      [&](float slider) { return sliderToPid( mPidP, slider ); }, "" );
    make_slider( window, "I", 0, 
      [&](float slider) { return sliderToPid( mPidI, slider ); }, "" );
    make_slider( window, "D", 0, 
      [&](float slider) { return sliderToPid( mPidD, slider ); }, "" );

    new Label(window, "Simulation Settings", "sans-bold");
    make_slider( window, "Sensor Delay", .1, 
      [&](float slider ) { return sliderTo1000ms( mSensorDelay, slider );}, 
      "ms" );

    //mIMemory      = make_slider( window, "I Memory", .5, sliderTo10s, "s" );

    new Label(window, "Simulation Control", "sans-bold" );

    Widget *panel = new Widget(window);
    panel->setLayout(new BoxLayout(Orientation::Horizontal,
        Alignment::Middle, 0, 20));
    mStart= new Button( panel, "Start" );
    mStart->setFlags( Button::ToggleButton );
    auto reset = new Button( panel, "Reset" );
    reset->setCallback( [&] (void) { mReset =true; }); 

    //Window *chartWindow = new Window(this, "PID Stats over Time");
    //chartWindow->setPosition(Vector2i( mSize.x()/2, 15));
    //chartWindow->setFixedSize(Vector2i( mSize.x()/2-15, mSize.y()*.4-30 ));

    performLayout();

    /* All NanoGUI widgets are initialized at this point. Now
       create an OpenGL shader to draw the main window contents.

       NanoGUI comes with a simple Eigen-based wrapper around OpenGL 3,
       which eliminates most of the tedious and error-prone shader and
       buffer object management.
    */
  
    mShader.init( "shader", vertexShader, fragmentShader ); 
    mGrapher.init( "grapher", vertexShaderGrapher, fragmentShaderGrapher ); 

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
    mGrapher.free();
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

    double angleRad = -mArmAngle + M_PI/2;

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
    mShader.uploadIndices(indices);
    mShader.setUniform("projection", projection);
    mShader.setUniform("camera", camera);
    mShader.setUniform("model", baseModel );
    static int count = 0;
    if ( count < 3 ) { std::cout << "c0\n"; }
    mShader.drawIndexed(GL_TRIANGLES, base_TRIANGLE_START, base_TRIANGLE_END);
    if ( count < 3 ) { std::cout << "c1\n"; }

    mShader.bind();
    mShader.setUniform("projection", projection);
    mShader.setUniform("camera", camera);
    mShader.setUniform("model", armModel );
    if ( count < 3 ) { std::cout << "c2\n"; }
    mShader.drawIndexed(GL_TRIANGLES, arm_TRIANGLE_START, arm_TRIANGLE_END - arm_TRIANGLE_START );
    if ( count < 3 ) { std::cout << "c3\n"; }
    ++count;

    glViewport( 
      mSize.x()/2, viewPortHeight,
      viewPortWidth, mSize.y()-viewPortHeight );
    glDisable( GL_DEPTH_TEST );
 
    populateGraphIndices();
    mGrapher.bind();
    mGrapher.uploadIndices(graphIndices);
    mGrapher.uploadAttrib("position", graphPositions);
    mGrapher.drawIndexed(GL_TRIANGLES, 0, numGraphIndices );
  }

  bool isReset()
  {
    bool result = mReset;
    mReset=false;
    return result;
  }

  void setArmAngle( double angle ) {
    int intAngle = radToDeg(angle);
    mAngleCurrent->setValue( std::to_string( intAngle ));
    mArmAngle = angle;
  }

  double getStartAngle() {
    return mStartAngle;
  }

  double getTargetAngle() {
    return mTargetAngle;
  }

  void resetErrorRecord()
  {
    mActualError.clear();
    for ( size_t i = 0; i < samplesToRecord; ++i ) {
      mActualError.push_back( std::nullopt );
    }
  }

  void recordActualError( double errorAngle )
  {
    //Not the cleverist way to do this, but CPU is cheap.
    size_t samplesToMove = samplesToRecord - 1;
    for ( size_t i = 0; i < samplesToMove; ++i ) {
      mActualError.at( i ) = mActualError.at( i+1 );
    }
    mActualError.at( samplesToRecord-1 ) = errorAngle;
  }

  void populateGraphIndices()
  {
    double dim = 2.0f;
    double xLeft  = -dim/2;
    double xRight =  dim/2;
    double x = xLeft;
    double xInc = dim / ((double) (samplesToRecord-1) );
    int validCount = 0;
    double lineThickness = 0.5;

    const double zero = 0.0;
    graphPositions.col(0) << xLeft,  zero-lineThickness, -1, 2;
    graphPositions.col(1) << xLeft,  zero              ,  0, 2;
    graphPositions.col(2) << xLeft,  zero+lineThickness,  1, 2;
    graphPositions.col(3) << xRight, zero-lineThickness, -1, 2;
    graphPositions.col(4) << xRight, zero              ,  0, 2;
    graphPositions.col(5) << xRight, zero+lineThickness,  1, 2;

    graphIndices.col( 0 ) << 0, 3, 4; 
    graphIndices.col( 1 ) << 0, 4, 1;
    graphIndices.col( 2 ) << 1, 4, 5; 
    graphIndices.col( 3 ) << 1, 5, 2; 

    for ( int i = 0; i < samplesToRecord; ++i ) 
    {
      int base = i*3 + 6;

      int before = i > 0 ? i-1 : i;
      int after  = i < samplesToRecord -1 ? i+1 : samplesToRecord-1;

      const bool sampleValid = mActualError.at( before ) && mActualError.at( i ) && mActualError.at( after );

      if ( sampleValid ) {
        ++validCount;
        double y = mActualError.at( i ).value() / 10.0f;
        graphPositions.col(base + 0) << x, y-lineThickness, -1.0, 1.0;
        graphPositions.col(base + 1) << x, y+0,              0.0, 1.0;
        graphPositions.col(base + 2) << x, y+lineThickness,  1.0, 1.0;
      }
      else {
        graphPositions.col(base + 0) << x, 0, -1.0, 0;
        graphPositions.col(base + 1) << x, 0,  0.0, 0;
        graphPositions.col(base + 2) << x, 0,  1.0, 0;
      }
      x+=xInc;
    }
    for ( int i = 0; i < samplesToRecord-1; ++i ) 
    {
      int base = 4+i*4;
      int basePos = i*3+6;

      graphIndices.col( base + 0 ) << basePos+0, basePos+3, basePos+ 4; 
      graphIndices.col( base + 1 ) << basePos+0, basePos+4, basePos+ 1;
      graphIndices.col( base + 2 ) << basePos+1, basePos+4, basePos+ 5; 
      graphIndices.col( base + 3 ) << basePos+1, basePos+5, basePos+ 2; 

    }
  }

private:

  static constexpr int       secondsToDisplay = 5;
  static constexpr size_t    samplesToRecord = secondsToDisplay * 5;
  static constexpr int       axisSegments = 2;

  // For each sample, record top, middle, botton for graph
  static constexpr int numGraphPositions = 
      3 * samplesToRecord +       // For each sample
      6;                          // For the axis line                     
  static constexpr int numGraphIndices   = 
      4 * (samplesToRecord-1) +   // For each sample
      4;                          // For the axis line

  std::vector<std::optional<double>> mActualError{ samplesToRecord };

  double              mArmAngle = 0.0;
  double              mStartAngle;
  double              mTargetAngle;
  double              mSensorDelay;
  double              mPidP;
  double              mPidI;
  double              mPidD;
  bool                mReset = false;
  nanogui::GLShader   mShader;
  nanogui::GLShader   mGrapher;
  nanogui::TextBox*   mAngleCurrent; 
  nanogui::Button*    mStart;

  nanogui::MatrixXf graphPositions;
  nanogui::MatrixXu graphIndices;
};

class PidSimBackEnd
{
  public:

  PidSimBackEnd( nanogui::ref<PidSimFrontEnd> frontEnd ) : mFrontEnd{ frontEnd }
  {
    reset();
  }

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
    mAngle       = degToRad(mFrontEnd->getStartAngle());
    mTargetAngle = degToRad(mFrontEnd->getTargetAngle());
    mAngleVel= 0;
    mFrontEnd->resetErrorRecord();
  }

  void updateOneTick()
  {
    // Run simulation 50x a second.
    double timeSlice = 1.0/50.0;

    if ( mFrontEnd->isReset() ) {
      reset();
    }
    double armX = cos( mAngle );
    double armY = sin( mAngle );
    // Gravity = < 0   , -9.8 >
    // Arm     = < armX, armY >
    // Angular accelleration = Arm x Gravity
    double AngleAccel = armX * -9.8;

    // Accellaration to Velocity integration
    mAngleVel += AngleAccel * timeSlice;
    // damping.  Lose 3% of angular vel every 50th/ second.
    mAngleVel *= .97; 

    // Velocity to Angle integration
    mAngle += mAngleVel * timeSlice;

    //
    // Impose some hard limits.  -120 degrees is about the angle where
    // the final segment of the robot arm hits the second segment,
    // visually.
    //
    if ( mAngle < degToRad( -120 )) {
      mAngle    = degToRad( -120 );
      mAngleVel = 0.0f;  // A hard stop kills all velocity
    }

    //
    // Second limit, where the final segment hits the second segment, but
    // in the other direction.
    //
    if ( mAngle > degToRad( 210 )) {
      mAngle    = degToRad( 210 );
      mAngleVel = 0.0f;  // Again, a hard stop kills all velocity
    }

    // Compute and record the error.
    double error = mAngle - mTargetAngle;

    static int counter=0;
    if( (counter % 10) == 0 ) {
      mFrontEnd->recordActualError( radToDeg(error) );
    }
    ++counter;

    updateFrontEnd();
  }

  void updateFrontEnd()
  {
    mFrontEnd->setArmAngle( mAngle );
  }

  double time = 0.0f;
  // map mAngle to "screen space" with <x,y> = <cos(mAngle),sin(mAngle)>
  double mAngle = 0;
  double mAngleVel = 0;

  // From the front end
  double mTargetAngle = 0;

  nanogui::ref<PidSimFrontEnd> mFrontEnd;
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
