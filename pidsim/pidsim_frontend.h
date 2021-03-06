#ifndef __PIDSIM_FRONTEND_H__
#define __PIDSIM_FRONTEND_H__

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-in-bool-context"
#pragma clang diagnostic ignored "-Wdeprecated-copy"
#pragma clang diagnostic ignored "-Wunused-parameter"

#include <nanogui/screen.h>     // for nanogui::Screen
#include <nanogui/glutil.h>     // for GLShader
#pragma clang diagnostic pop
#include <optional>             // for std::optional
#include <vector>               // for std::vector

namespace PidSim {

constexpr double ShaderRed    = 1.0;
constexpr double ShaderGreen  = 2.0;
constexpr double ShaderPurple = 3.0;
constexpr double ShaderOrange = 4.0;
constexpr double ShaderBlue   = 5.0;

class FrontEnd: public nanogui::Screen {
public:
  FrontEnd();
  ~FrontEnd(); 

  virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) override; 

  virtual void draw(NVGcontext *ctx) override;

  virtual void drawContents() override;

  bool isReset();

  bool isNudgeDown();

  bool isNudgeUp();

  bool isWackDown();

  bool isWackUp();


  bool isNewSettings();

  bool isSlowTime();

  void setArmAngle( double angle );

  double getStartAngle(); 

  double getTargetAngle();

  void resetErrorRecord();

  void recordActualError( double pError, double iError, double dError, double motor );

  std::pair<int,int> populateGraphIndices( 
    const std::vector<std::optional<double>> toPlot,
    std::pair<int,int> startData,
    double color
  );

  int getSamplesPerSecond() const;
  double getP() const;
  double getI() const;
  double getD() const;
  double getRollingFriction() const;
  double getStaticFriction() const;
  double getSensorNoise() const;
  double getSensorDelay() const;
  double getMotorDelay() const;

private:

  static constexpr int       secondsToDisplay = 5;
  static constexpr int       samplesPerSecond = 25;
  static constexpr size_t    samplesToRecord = samplesPerSecond * secondsToDisplay;
  static constexpr size_t    axisSamples = 30;

  // For each sample, record top, middle, bottom for graph
  static constexpr int numGraphPositions = 
      3 * (4*samplesToRecord + axisSamples );
  static constexpr int numGraphIndices   =
      4 * (4*(samplesToRecord-1)+(axisSamples-1)); 

  std::vector<std::optional<double>> mPError;
  std::vector<std::optional<double>> mIError;
  std::vector<std::optional<double>> mDError;
  std::vector<std::optional<double>> mMotor;
  std::vector<std::optional<double>> mAxis;

  double              mArmAngle           = 0.0;
  double              mStartAngle         = -90.0;
  double              mTargetAngle        = 0.0;
  double              mSensorDelay        = 0.0;
  double              mMotorDelay         = 0.0;
  double              mPidP               = 0.0;
  double              mPidI               = 0.0;
  double              mPidD               = 0.0;
  double              mStaticFriction     = 0.0;
  double              mRollingFriction    = 0.0;
  double              mSensorNoise        = 0.0;
  bool                mReset              = false;
  bool                mHardReset          = false;
  bool                mSlowTime           = false;
  bool                mSlowTimeState      = false;
  bool                mNudgeDown          = false;
  bool                mNudgeUp            = false;
  bool                mWackDown           = false;
  bool                mWackUp             = false;

  nanogui::Button*    mSlowTimeButton     = nullptr;
  nanogui::TextBox*   mAngleCurrent       = nullptr;

  nanogui::GLShader   mShader;
  nanogui::GLShader   mGrapher;

  nanogui::MatrixXf graphPositions;
  nanogui::MatrixXu graphIndices;
};

} // end PidSim Namespace

#endif

