/**
 * The Defocus Measurement class. 
 */

#include "StdAfx.h"
#include "DefocusMeasurement.h"
#include "Logger.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sstream>

/**
 * Constructor.
 */
DefocusMeasurement::DefocusMeasurement()
{
  // Prepare the random number generator.
  srand(time(0));

  // Prepare the optimization.
  prepareMeasurement();

  // Setup the SLM.
  SLMInstance = new SLMController;
  SLMInstance->initSLM();
  Sleep(500); // Give it a bit to get started.
}

/**
 * Prepares the measurement (resets).
 * Neccessary because in the current model: WiscScan drives the evolution.
 */
void DefocusMeasurement::prepareMeasurement()
{
  isDone = false;
  As=0;
  Ad=0.5;
}

bool DefocusMeasurement::isFinished()
{
  return isDone;
}


/**
 * Run one iteration.
 *
 * @param intensity The intensity of the last image that was captured.
 */
void DefocusMeasurement::iterateOnce(double intensity)
{
  std::ostringstream logSS;

  if (!firstIterationDone) {
    // The very first iteration.  Setup the SLM and return.
    firstIterationDone = true;

    // Prepare the next image on the SLM. 
    unsigned char *phaseData = new unsigned char [SLMSIZE*SLMSIZE];
    LOGME( "First iteration is now approximately done " )
    LOGME( "- Generating data for SLM " )
    SeidelSet.generateImageBufferForSLM(phaseData);
    LOGME( "- preparing data to be sent " )
    SLMInstance->receiveData(phaseData);
    LOGME( "- preparing sending... " )
    SLMInstance->sendToSLM(true);
    LOGME( "- done " )
    delete phaseData;

    Sleep(150); // Seems to take a bit longer occasionally.
    LOGME( "- SLM ready for action! " )
    return; // Return immediately.
  }

  if ((-As-Ad) > 0.5) {
    As+=0.1;
    Ad=-As+0.5;
    if (As > 20) {
      isDone = true;
    }
  } else {
    Ad -= 0.1;
  }

  if (!isDone) {
    // Prepare the next image on the SLM. 
    unsigned char *phaseData = new unsigned char [SLMSIZE*SLMSIZE];

    SeidelSet.resetCoefficients();
    SeidelSet.setPower(Ad);
    SeidelSet.setSphericalAberration(As);

    logSS.str("");
    logSS << "Sending SLM. As: " << As << " Ad: " << Ad;
    LOGME( logSS.str() );
    
    SeidelSet.generateImageBufferForSLM(phaseData);
    SLMInstance->receiveData(phaseData);
    SLMInstance->sendToSLM(true);
    delete phaseData;

    Sleep(100); // Takes approx. 100 ms for SLM to "prepare".
  }
}

