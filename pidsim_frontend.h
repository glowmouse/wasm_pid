#ifndef __PIDSIM_FRONTEND_H__
#define __PIDSIM_FRONTEND_H__

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
#include <memory>
#include <utility>
#include <chrono>
#include <numeric>
#include "pidsim_utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

constexpr double ShaderRed    = 1.0;
constexpr double ShaderGreen  = 2.0;
constexpr double ShaderPurple = 3.0;
constexpr double ShaderOrange = 4.0;
constexpr double ShaderBlue   = 5.0;

class PidSimFrontEnd: public nanogui::Screen {
public:
  PidSimFrontEnd();
  ~PidSimFrontEnd(); 

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

  bool isReset()
  {
    bool result = mReset;
    mReset=false;
    return result;
  }

  bool isNudgeDown()
  {
    bool result = mNudgeDown;
    mNudgeDown=false;
    return result;
  }

  bool isNudgeUp()
  {
    bool result = mNudgeUp;
    mNudgeUp=false;
    return result;
  }

  bool isWackDown()
  {
    bool result = mWackDown;
    mWackDown=false;
    return result;
  }

  bool isWackUp()
  {
    bool result = mWackUp;
    mWackUp=false;
    return result;
  }


  bool isNewSettings()
  {
    bool result = mHardReset;
    mHardReset=false;
    return result;
  }

  bool isSlowTime()
  {
    bool result = mSlowTime;
    mSlowTime =false;

    if ( result ) {
      mSlowTimeState = !mSlowTimeState;
      mSlowTimeButton->setCaption( mSlowTimeState ? "Speed Time" : "Slow Time" );
    }
    return mSlowTimeState;
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

  void recordActualError( double pError, double iError, double dError, double motor )
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

  std::pair<int,int> populateGraphIndices( 
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

  int getSamplesPerSecond() { return samplesPerSecond; }

  double getP() { return mPidP; }
  double getI() { return mPidI; }
  double getD() { return mPidD; }
  double getRollingFriction() { return mRollingFriction; }
  double getStaticFriction() { return mStaticFriction; }

private:

  static constexpr int       secondsToDisplay = 5;
  static constexpr int       samplesPerSecond = 10;
  static constexpr size_t    samplesToRecord = samplesPerSecond * secondsToDisplay;
  static constexpr size_t    axisSamples = 30;

  // For each sample, record top, middle, botton for graph
  static constexpr int numGraphPositions = 
      3 * (4*samplesToRecord + axisSamples );
  static constexpr int numGraphIndices   =
      4 * (4*(samplesToRecord-1)+(axisSamples-1)); 

  std::vector<std::optional<double>> mPError;
  std::vector<std::optional<double>> mIError;
  std::vector<std::optional<double>> mDError;
  std::vector<std::optional<double>> mMotor;
  std::vector<std::optional<double>> mAxis;

  double              mArmAngle = 0.0;
  double              mStartAngle;
  double              mTargetAngle;
  double              mSensorDelay;
  double              mPidP;
  double              mPidI;
  double              mPidD;
  double              mStaticFriction;
  double              mRollingFriction;
  bool                mReset = false;
  bool                mHardReset = false;
  bool                mSlowTime = false;
  bool                mSlowTimeState = false;
  Button*             mSlowTimeButton = nullptr;
  bool                mNudgeDown = false;
  bool                mNudgeUp = false;
  bool                mWackDown = false;
  bool                mWackUp = false;
  nanogui::GLShader   mShader;
  nanogui::GLShader   mGrapher;
  nanogui::TextBox*   mAngleCurrent; 

  nanogui::MatrixXf graphPositions;
  nanogui::MatrixXu graphIndices;
};

#endif

