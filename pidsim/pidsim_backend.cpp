#include "pidsim_backend.h"
#include "pidsim_backend_physics_sim.h"
#include "pidsim_backend_pid_controller.h"
#include "pidsim_utils.h"

namespace PidSim {

BackEnd::BackEnd( nanogui::ref<FrontEnd> frontEnd ) : 
  mFrontEnd{ frontEnd },
  mPhysicsSim{ std::make_unique<PhysicsSim>( Utils::degToRad(mFrontEnd->getStartAngle())) },
  mPidController{ std::make_unique<PidController>() }
{
}

// Declared here so PhysicsSim & PidController don't need concrete
// definitions in header file.
BackEnd::~BackEnd()
{
}

// Main update loop for the back end simulation.
//
// 1. How many "single updates" are needed to catch the simulation up
// 2. Run that many single updates
// 
void BackEnd::update( std::chrono::duration<double> delta )
{
  // 1. How many "single updates" are needed to catch the simulation up
  //
  const double updatesPerSecondF = static_cast<double>(updatesPerSecond);
  const unsigned lastUpdatesSinceStart = static_cast<unsigned>( time * updatesPerSecondF );
  time += delta.count();
  const unsigned curUpdatesSinceStart = static_cast<unsigned>( time * updatesPerSecondF );
  const unsigned updatesSinceLast = curUpdatesSinceStart - lastUpdatesSinceStart;

  // 2. Run that many single updates
  //
  for ( unsigned update = 0; update < updatesSinceLast; ++update ) {
    updateOneTick();
  }
} 
 
void BackEnd::reset()
{
  // Completely replace the old physics simulation & pid controller
  const double startAngle = Utils::degToRad( mFrontEnd->getStartAngle() );
  mPhysicsSim      = std::make_unique< PhysicsSim  >( startAngle ); 
  mPidController = std::make_unique< PidController >();
  // Reset the error graph on the front end
  mFrontEnd->resetErrorRecord();
}

void BackEnd::getInputFromFrontEnd()
{
  if ( mFrontEnd->isReset() ) {
    reset();
  }

  mPidController->updatePidSettings(
      mFrontEnd->getP(),
      mFrontEnd->getI(),
      mFrontEnd->getD(),
      Utils::degToRad(mFrontEnd->getTargetAngle())
  );

  mRollingFriction = mFrontEnd->getRollingFriction()/50.0;
  mPhysicsSim->setSensorNoise( mFrontEnd->getSensorNoise() );
  mPhysicsSim->setSensorDelay( mFrontEnd->getSensorDelay() );
  mPhysicsSim->setMotorDelay( mFrontEnd->getMotorDelay() );
  mSlowTime = mFrontEnd->isSlowTime();

  if ( mFrontEnd->isNudgeUp()) {
    mPhysicsSim->bump( 3 );
  }
  if ( mFrontEnd->isNudgeDown()) {
    mPhysicsSim->bump( -3 );
  }
  if ( mFrontEnd->isWackUp()) {
    mPhysicsSim->bump( 10 );
  }
  if ( mFrontEnd->isWackDown()) {
    mPhysicsSim->bump( -10 );
  }
}

void BackEnd::updateOneTick()
{
  // Check for input
  getInputFromFrontEnd();

  // Slow time logic
  ++mCounter0;
  if ( mSlowTime && ( mCounter0 % slowTimeScale ) != 0 ) { return; }

  // Advance the simulation 1/50th of a second
  const double timeSlice = 1.0/((double) updatesPerSecond);

  // Run the PID controller
  const PidController::Output pOut = mPidController->updatePidController( timeSlice, mPhysicsSim->getSensorAngle() );

  // Update the error graph
  sendErrorToFrontEnd( pOut.mPError, pOut.mIError, pOut.mDError );

  // Advance the robot arm simulation
  updateRobotArmSimulation( timeSlice, pOut.mMotorPower );

  // Send new positions to the front end.
  updateFrontEnd();
}

void BackEnd::updateRobotArmSimulation( double timeSlice, double motorPower )
{
  mPhysicsSim->startSimulationIteration();
  mPhysicsSim->applyGravity();
  mPhysicsSim->applyMotor( motorPower, timeSlice );
  mPhysicsSim->updateAngleVel( timeSlice );
  mPhysicsSim->applyFriction( mRollingFriction );
  mPhysicsSim->updateAngle( timeSlice );
  mPhysicsSim->imposePositionHardLimits();
  mPhysicsSim->endSimulationIteration();
}

void BackEnd::sendErrorToFrontEnd( double pError, double iError, double dError )
{
  const int sampleInterval = updatesPerSecond / mFrontEnd->getSamplesPerSecond();

  ++mCounter1;
  if( (mCounter1 % sampleInterval ) == 0 ) {
    mFrontEnd->recordActualError( 
      Utils::radToDeg( pError ), 
      Utils::radToDeg( iError ), 
      Utils::radToDeg( dError ), 
      mPhysicsSim->getMotorPower() * 150 );
  }
}

void BackEnd::updateFrontEnd()
{
  mFrontEnd->setArmAngle( mPhysicsSim->getActualAngle() );
}

}


