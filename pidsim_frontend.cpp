#include "pidsim_frontend.h"
#include "nanogui/button.h"
#include "nanogui/label.h"
#include "nanogui/layout.h"
#include "nanogui/slider.h"
#include "nanogui/textbox.h"
#include "nanogui/widget.h"
#include "nanogui/window.h"
#include "pidsim_model.h"
#include "pidsim_utils.h"
#include <sstream>
#include <iomanip>

///
/// Create a slider with a label
///
/// @param[in] parent          The object that will own the slider+label
/// @param[in] labelText       The text for the slider.
/// @param[in] sliderStartPos  Where the slider should start.  From 0-1
/// @param[in] sliderToLabel   Function that creates a label for a slider
/// @param[in] units           What units does the slider represent?
/// @param[in] noSlider        Just make a label and output, but keep the
///                            formatting consistent.
///
/// @return A non-owning pointer to the text box that displayers the 
///         slider's value.  Ownership of objects created is held by 
///         parent.
///
/// TODO - the noSlider objects is ugly.  Find a way to refactor?
/// 
nanogui::TextBox* makeSlider( 
  nanogui::Widget*                    parent,
  const std::string&                  labelText,
  float                               sliderStartPos,
  std::function<std::string(float)>   sliderToLabel,
  const std::string&                  units,
  bool                                noSlider = false
)
{
  using namespace nanogui;
  Widget *panel = new Widget(parent);
  panel->setLayout(new BoxLayout(Orientation::Horizontal,
      Alignment::Middle, 0, 20));
  Label *label = new Label(panel, labelText, "sans-bold");
  label->setFixedWidth(120);

  Slider *slider;
  if ( !noSlider ) {
    slider = new Slider(panel);
    slider->setValue(sliderStartPos);
    slider->setFixedWidth(80);
  }
  else {
    Widget *deadSpace = new Widget(panel);
    deadSpace->setFixedWidth(80);
  }

  TextBox *textBox = new TextBox(panel);
  textBox->setFixedSize(Vector2i(90, 25));
  textBox->setValue(sliderToLabel(sliderStartPos));
  textBox->setUnits(units);

  if ( !noSlider ) {
    slider->setCallback([textBox,sliderToLabel](float value) 
      { textBox->setValue(sliderToLabel(value)); } 
    );
    slider->setFinalCallback([sliderToLabel](float value) {
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
  FragPos = vec3(position.x, position.y*.08, 0.0 );
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

vec3 lightpos     = vec3( 1.0, 3.0, -5.0 );
vec3 white        = vec3( 1.0, 1.0, 1.0); 
vec3 objcolor;
vec3 viewpos      = vec3( 0.0, 0.0, -5.0);
float alpha;

void main() {
  //
  // TODO - LOL, this works, but ++ugly.  Right now we seem
  // to have CPU to burn.
  //
  if ( colorIndex > .99 && colorIndex < 1.01 ) {
    objcolor  = vec3( 1.0, 0.2, 0.2 );  // Red
    alpha = 1.0;
  }
  else if ( colorIndex > 1.99 && colorIndex < 2.01 ) {
    objcolor  = vec3( 0.0, 1.0, 0.0 );  // Green
    alpha = 1.0;
  }
  else if ( colorIndex > 2.99 && colorIndex < 3.01 ) {
    objcolor  = vec3( 1.0, 0.0, 1.0 );  // Magenta/ Purple
    alpha = 1.0;
  }
  else if ( colorIndex > 3.99 && colorIndex < 4.01 ) {
    objcolor  = vec3( 1.0, 1.0, 0.0 );  // Orange
    alpha = 1.0;
  }
  else if ( colorIndex > 4.99 && colorIndex < 5.01 ) {
    objcolor  = vec3( 0.5, 0.5, 1.0 );  // Light Blue
    alpha = 1.0;
  }
  else {
    objcolor  = vec3( 0.0, 0.0, 0.0 );  // Black
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

PidSimFrontEnd::PidSimFrontEnd() : 
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
    auto sliderTo10 = [sliderToFloat]( double& storage, float slider ) 
    {
      return sliderToFloat( storage, slider, 10.0 );
    };
    auto sliderToPid = [sliderToFloat]( double& storage, float slider ) {
      return sliderToFloat( storage, slider, 4.0 );
    };

    new Label(window, "Arm Start Position", "sans-bold");
    makeSlider( window, "Start Arm Angle", 0.0, 
      [&](float slider) { return sliderToDegrees( mStartAngle, slider ); } ,
      "deg" );

    makeSlider( window, "Target Arm Angle",  0.5,
      [&](float slider) { return sliderToDegrees( mTargetAngle, slider ); }, 
      "deg" );

    new Label(window, "Arm Current Position", "sans-bold");
    mAngleCurrent = makeSlider( window, "Arm Angle", 0.0, 
      [&](float slider) { return ""; }, "deg", true );

    new Label(window, "PID Settings", "sans-bold");
    makeSlider( window, "P", 0, 
      [&](float slider) { return sliderToPid( mPidP, slider ); }, "" );
    makeSlider( window, "I", 0, 
      [&](float slider) { return sliderToPid( mPidI, slider ); }, "" );
    makeSlider( window, "D", 0, 
      [&](float slider) { return sliderToPid( mPidD, slider ); }, "" );

    new Label(window, "Simulation Settings", "sans-bold");
    makeSlider( window, "Rolling Friction", .2, 
      [&](float slider ) { return sliderTo10( mRollingFriction, slider );}, 
      "" );
    makeSlider( window, "Static Friction", 0, 
      [&](float slider ) { return sliderTo10( mStaticFriction, slider );}, 
      "" );

    new Label(window, "Simulation Control", "sans-bold" );

    Widget *panel = new Widget(window);
    panel->setLayout(new BoxLayout(Orientation::Horizontal,
        Alignment::Middle, 0, 20));
    auto hardReset = new Button( panel, "Hard Reset" );
    hardReset->setCallback( [&] (void) { mHardReset = true; }); 
    auto reset = new Button( panel, "Reset" );
    reset->setCallback( [&] (void) { mReset =true; }); 
    mSlowTimeButton = new Button( panel, "Slow Time" );
    mSlowTimeButton->setCallback( [&] (void) { mSlowTime =true; }); 

    Widget *panel2 = new Widget(window);
    panel2->setLayout(new BoxLayout(Orientation::Horizontal,
        Alignment::Middle, 0, 20));
    auto nudgeUp = new Button( panel2, "Small Push Up" );
    nudgeUp->setCallback( [&] (void) { mNudgeUp =true; }); 
    auto nudgeDown = new Button( panel2, "Small Push Down" );
    nudgeDown ->setCallback( [&] (void) { mNudgeDown =true; }); 

    Widget *panel3 = new Widget(window);
    panel3->setLayout(new BoxLayout(Orientation::Horizontal,
        Alignment::Middle, 0, 20));
    auto wackUp = new Button( panel3, "Large Push Up" );
    wackUp->setCallback( [&] (void) { mWackUp =true; }); 
    auto wackDown = new Button( panel3, "Large Push Down" );
    wackDown->setCallback( [&] (void) { mWackDown =true; }); 

    Widget *keyLayout= new Widget(this);
    keyLayout->setLayout(new BoxLayout(Orientation::Horizontal,
        Alignment::Middle, 0, 20));
    keyLayout->setPosition(Vector2i( mSize.x()/2, 15));
    auto pError= new Label(keyLayout, "P Error", "sans-bold" );
    pError ->setColor( Color( 255, 0, 0, 255 ));
    auto iError= new Label(keyLayout, "I Error", "sans-bold" );
    iError ->setColor( Color( 255, 0, 255, 255 ));
    auto dError= new Label(keyLayout, "D Error", "sans-bold" );
    dError ->setColor( Color( 255, 255, 0, 255 ));
    auto motorOutKey = new Label(keyLayout, "Motor Output", "sans-bold" );
    motorOutKey ->setColor( Color( 128, 128, 255, 255 ));
    auto dAxis = new Label(keyLayout, "Error = 0 Axis", "sans-bold" );
    dAxis ->setColor( Color( 0, 255, 0, 255 ));

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

    assert( mAxis.size() == 0 );
    for ( int i = 0; i < axisSamples; ++i ) {
      mAxis.push_back( 0.0 );
    }
    assert( mAxis.size() == axisSamples );
  }

PidSimFrontEnd::~PidSimFrontEnd() 
  {
    mShader.free();
    mGrapher.free();
  }

bool PidSimFrontEnd::keyboardEvent(int key, int scancode, int action, int modifiers) 
  {
    if (Screen::keyboardEvent(key, scancode, action, modifiers))
      return true;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      //setVisible(false);
      return true;
    }
    return false;
  }

  void PidSimFrontEnd::draw(NVGcontext *ctx) {
    /* Draw the user interface */
    Screen::draw(ctx);
  }

  void PidSimFrontEnd::drawContents() {
    using namespace nanogui;

    double angleRad = -mArmAngle + M_PI/2;

    double viewPortWidth =mSize.x()/2;
    double viewPortHeight=mSize.y()*3/4;

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
    camera.topLeftCorner<3,3>() = Matrix3f(Eigen::AngleAxisf(-M_PI/2 + M_PI/10,  Vector3f::UnitX()));
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
    mShader.drawIndexed(GL_TRIANGLES, base_TRIANGLE_START, base_TRIANGLE_END);

    mShader.bind();
    mShader.setUniform("projection", projection);
    mShader.setUniform("camera", camera);
    mShader.setUniform("model", armModel );
    mShader.drawIndexed(GL_TRIANGLES, arm_TRIANGLE_START, arm_TRIANGLE_END - arm_TRIANGLE_START );
    ++count;

    double graphViewPortHeight = mSize.y()/3;
    glViewport( 
      mSize.x()/2, mSize.y()-graphViewPortHeight,
      viewPortWidth, graphViewPortHeight );
    glDisable( GL_DEPTH_TEST );
 
    assert( mAxis.size() == axisSamples );
    std::pair<int,int> position(0,0);
    position = populateGraphIndices( mAxis,   position, ShaderGreen  );
    position = populateGraphIndices( mPError, position, ShaderRed    );
    position = populateGraphIndices( mIError, position, ShaderPurple );
    position = populateGraphIndices( mDError, position, ShaderOrange );
    position = populateGraphIndices( mMotor,  position, ShaderBlue   );
    assert( position.first  == numGraphPositions );
    assert( position.second == numGraphIndices );

    mGrapher.bind();
    mGrapher.uploadIndices(graphIndices);
    mGrapher.uploadAttrib("position", graphPositions);
    mGrapher.drawIndexed(GL_TRIANGLES, 0, numGraphIndices );
  }

  bool PidSimFrontEnd::isReset()
  {
    bool result = mReset;
    mReset=false;
    return result;
  }

  bool PidSimFrontEnd::isNudgeDown()
  {
    bool result = mNudgeDown;
    mNudgeDown=false;
    return result;
  }

  bool PidSimFrontEnd::isNudgeUp()
  {
    bool result = mNudgeUp;
    mNudgeUp=false;
    return result;
  }

  bool PidSimFrontEnd::isWackDown()
  {
    bool result = mWackDown;
    mWackDown=false;
    return result;
  }

  bool PidSimFrontEnd::isWackUp()
  {
    bool result = mWackUp;
    mWackUp=false;
    return result;
  }


  bool PidSimFrontEnd::isNewSettings()
  {
    bool result = mHardReset;
    mHardReset=false;
    return result;
  }

  bool PidSimFrontEnd::isSlowTime()
  {
    bool result = mSlowTime;
    mSlowTime =false;

    if ( result ) {
      mSlowTimeState = !mSlowTimeState;
      mSlowTimeButton->setCaption( mSlowTimeState ? "Speed Time" : "Slow Time" );
    }
    return mSlowTimeState;
  }

  void PidSimFrontEnd::setArmAngle( double angle ) {
    int intAngle = radToDeg(angle);
    mAngleCurrent->setValue( std::to_string( intAngle ));
    mArmAngle = angle;
  }

  double PidSimFrontEnd::getStartAngle() {
    return mStartAngle;
  }

  double PidSimFrontEnd::getTargetAngle() {
    return mTargetAngle;
  }

  void PidSimFrontEnd::resetErrorRecord()
  {
    mPError.clear();
    mIError.clear();
    mDError.clear();
    mMotor.clear();
    for ( size_t i = 0; i < samplesToRecord; ++i ) {
      mPError.push_back( std::nullopt );
      mIError.push_back( std::nullopt );
      mDError.push_back( std::nullopt );
      mMotor.push_back( std::nullopt );
    }
  }

  void PidSimFrontEnd::recordActualError( double pError, double iError, double dError, double motor )
  {
    //Not the cleverist way to do this, but CPU is cheap.
    size_t samplesToMove = samplesToRecord - 1;
    for ( size_t i = 0; i < samplesToMove; ++i ) {
      mPError.at( i ) = mPError.at( i+1 );
      mIError.at( i ) = mIError.at( i+1 );
      mDError.at( i ) = mDError.at( i+1 );
      mMotor.at( i ) = mMotor.at( i+1 );
    }
    mPError.at( samplesToRecord-1 ) = pError;
    mIError.at( samplesToRecord-1 ) = iError;
    mDError.at( samplesToRecord-1 ) = dError;
    mMotor.at( samplesToRecord-1 ) = motor;
  }

  std::pair<int,int> PidSimFrontEnd::populateGraphIndices( 
    const std::vector<std::optional<double>> toPlot,
    std::pair<int,int> startData,
    double color
  )
  {
    const int startPos = startData.first;
    const int startIndex = startData.second;
    const int samples = toPlot.size();
    const double dim = 2.0f;
    const double xLeft  = -dim/2;
    const double xRight =  dim/2;
    const double lineThickness = 0.5;
    const double xInc = dim / ((double) (samples-1) );

    double x = xLeft;
    for ( int i = 0; i < samples; ++i ) 
    {
      int base = i*3 + startPos;

      int before = i > 0 ? i-1 : i;
      int after  = i < samples-1 ? i+1 : samples-1;

      const bool sampleValid = toPlot.at( before ) && toPlot.at( i ) && toPlot.at( after );

      if ( sampleValid ) {
        double y = toPlot.at( i ).value() / 10.0f;
        graphPositions.col(base + 0) << x, y-lineThickness, -1.0, color;
        graphPositions.col(base + 1) << x, y+0,              0.0, color;
        graphPositions.col(base + 2) << x, y+lineThickness,  1.0, color;
      }
      else {
        graphPositions.col(base + 0) << x, 0, -1.0, 0;
        graphPositions.col(base + 1) << x, 0,  0.0, 0;
        graphPositions.col(base + 2) << x, 0,  1.0, 0;
      }
      x+=xInc;
    }
    for ( int i = 0; i < samples-1; ++i ) 
    {
      int base =    startIndex + i*4;
      int basePos = startPos   + i*3;

      graphIndices.col( base + 0 ) << basePos+0, basePos+3, basePos+ 4; 
      graphIndices.col( base + 1 ) << basePos+0, basePos+4, basePos+ 1;
      graphIndices.col( base + 2 ) << basePos+1, basePos+4, basePos+ 5; 
      graphIndices.col( base + 3 ) << basePos+1, basePos+5, basePos+ 2; 
    }

    const int endPos = startPos + samples*3;
    const int endIndex = startIndex + (samples-1)*4;

    return std::pair<int,int>( endPos, endIndex );
  }

  int PidSimFrontEnd::getSamplesPerSecond() { 
    return samplesPerSecond; 
  }

  double PidSimFrontEnd::getP() {
     return mPidP; 
  }
  double PidSimFrontEnd::getI() {
     return mPidI; 
  }
  double PidSimFrontEnd::getD() {
     return mPidD; 
  }
  double PidSimFrontEnd::getRollingFriction() {
     return mRollingFriction; 
  }
  double PidSimFrontEnd::getStaticFriction() {
     return mStaticFriction; 
  }


