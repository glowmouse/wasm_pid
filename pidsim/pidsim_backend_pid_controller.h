#ifndef __PIDSIM_BACKEND_PID_CONTROLLER_H__
#define __PIDSIM_BACKEND_PID_CONTROLLER_H__

namespace PidSim {

///
/// @brief A class that implements a PID controller
/// 
class PidController
{
  public:

  ///
  /// @brief the output of a PID controller update
  /// 
  struct Output
  {
    Output() = delete;
    Output( double pError, double iError, double dError, double motorPower ) :
      mPError{pError}, 
      mIError{iError}, 
      mDError{dError}, 
      mMotorPower{motorPower}
    {}

    /// @brief The proportional error from the PID controller.  For reporting.
    double mPError;
    /// @brief The integral error from the PID controller. For reporting
    double mIError;
    /// @brief The derivative error from the PID controller. For reporting
    double mDError;
    /// @brief The motor power setting from the PID controller
    double mMotorPower;
  };

  ///
  /// @brief Update the PID controller settings
  ///
  /// @param[in] pidP         - New PID "P" Gain
  /// @param[in] pidI         - New PID "I" Gain
  /// @param[in] pidD         - New PID "D" Gain
  /// @param[in] targetAngle  - New Set Point
  ///
  void updatePidSettings(double pidP, double pidI, double pidD, double targetAngle );

  ///
  /// @brief Apply the PID controller to sensorInputAngle for one time slice 
  /// 
  /// @param[in] timeSlice    - The length of the time slice, in seconds
  /// @param[in] sensorAngle  - The current angle from the sensor, in radians
  /// @return    The PID Controller error terms @ the motor power 
  ///
  Output updatePidController( double timeSlice, double sensorAngle );

  ///
  /// @brief Reset the PID controller.
  ///
  void reset();

  private:
  double mPidP = 0;
  double mPidI = 0;
  double mPidD = 0;
  double mIError = 0;
  double mLastPError = 0;
  double mTargetAngle = 0;
};

}

#endif

