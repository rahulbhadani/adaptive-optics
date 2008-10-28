// GH

#include "StdAfx.h"
#include "ZernikeSquarePolynomial.h"
#include "SLMProject.h"

#include "Logger.h"
#include <sstream>

// Constructor.
ZernikeSquarePolynomial::ZernikeSquarePolynomial()
{
  resetCoefficients();
}


/**
 * Resets the Zernike coefficients to default values.
 */
void ZernikeSquarePolynomial::resetCoefficients()
{
  setPiston(0.0);
  setSecondaryAstigmatismX(0.0);
  setSecondaryAstigmatismY(0.0);
  setPower(0.0);
  setAstigmatismX(0.0);
  setAstigmatismY(0.0);
  setComaX(0.0);
  setComaY(0.0);
  setSphericalAberration(0.0);
  setTrefoilX(0.0);
  setTrefoilY(0.0);
  setSecondaryComaX(0.0);
  setSecondaryComaY(0.0);
  setSecondarySphericalAberration(0.0);
  setTiltX(50.0);
  setTiltY(50.0);
  //setTiltX(0.0);
  //setTiltY(0.0);
}

/**
 * Round off to ensure dat is in range -256 to 256.
 * GH: (replacable with modulus opration?)
 */
double round256(double dat)
{
  if (dat > 0) {
    while (dat >= 256){
      dat = dat - 256;
    }
  }
  else {
    while (dat <= -256) {
      dat = dat + 256;
    }
  }
  
  return dat;
}


/**
 * ???
 * Converts double-valued matrix to an unsigned 255 bit matrix, which is the input
 * for the Spatial light modulator.
 * N.B. A double value of 1 will be 255 in the SLM (the range is 0-255).
 */
void DtoI(double *Dbuf, int length, unsigned char *phaseData)
{
  unsigned char temp;
  int i;
  
#ifdef DEBUG_OUTPUT
  int tp, tp2;
  FILE *pfold;
  pfold = fopen("c:/slmcontrol/dumpout/VC_output_beforeD2I.txt","wt");
  for (tp = 0; tp < 512; tp++) {
    fprintf(pfold, "\n");
    for (tp2 = 0; tp2 < 512; tp2++)
      fprintf(pfold, "%f, ", Dbuf[tp*512 + tp2]);
  }
  fclose(pfold);
#endif
  
  for (i = 0; i < SLMSIZE * SLMSIZE; i++) {
    Dbuf[i] = round256(Dbuf[i] * 256);
    if (Dbuf[i] < 0) {
      Dbuf[i] = Dbuf[i] + 256;
    }
  } 
  
  // GH: Appears to be some sort of a rounding scheme.  Bit weird though.
  for (i = 0; i < length; i ++) {
    temp = unsigned char(*(Dbuf+i));
    if ((*(Dbuf+i) - temp) < 0.5)
      *(phaseData+i) = temp;
    else
      *(phaseData+i) = temp + 1;
  }
}


/**
 * Generate the buffer from the parameters.
 *
 * @param phaseData The output phase data image buffer.
 */
void ZernikeSquarePolynomial::generateImageBufferForSLM(unsigned char *phaseData)
{
  double x, y, radius;
  double divX, divY, XSquPlusYSqu, divXSqu, divYSqu, XPYSquSqu;
  double terms[16];
  double total;
  //std::ostringstream logSS;
  
  
  //  char msgbuf[1024];
  
  //int defocusbins, stigxbins, stigybins, comaxbins, comaybins, speribins;
  
  int ActSize, start, end;
  double *zern = new double[SLMSIZE*SLMSIZE];
  memset(zern, 0, sizeof(double)*SLMSIZE*SLMSIZE);
  
  
  ActSize = SLMSIZE;
  /* GH: FIX.  5/07/2008.  Should be 256, not 300.
  * radius = ActSize*300/512;
  */
/*XXX/NOTE:!!!!!!!!!!!!!!
 *Normally we would use:
 *  radius = ActSize*256/512;
 *BUT DUE TO MIRROR OVERFILLING, WE USE:
 */
  radius = ActSize*100/512;
  
  start = (SLMSIZE - ActSize)/2;
  end = start + ActSize;
  
  y = ActSize/2;

  //START
  double Ad=(getPower()/2)*(2) + (getSphericalAberration()/2)*(-6);
  double As=(getSphericalAberration()/2)*(6);
  std::ostringstream logSS;
  logSS << "Ad is: " <<  Ad;
  LOGME( logSS.str() );
  logSS.str("");
  logSS << "As is: " <<  As;
  LOGME( logSS.str() );
  logSS.str("");
  logSS << "radius is: " <<  radius;
  LOGME( logSS.str() );
  logSS.str("");
  //EOF
  for (int row = start; row < end; row++) {
    // Reset x.
    x = -(ActSize/2);
    
    for (int col = start; col < end; col++) {
      // Build some terms that are repeated through the equations.
      divX = x/radius;
      divY = y/radius;
      XSquPlusYSqu = divX*divX + divY*divY;
	  rho = XSquPlusYSqu;
      XPYSquSqu = XSquPlusYSqu*XSquPlusYSqu;
      divXSqu = divX*divX;
      divYSqu = divY*divY;
      
      if ((divXSqu + divYSqu) <= 1) {     
        terms[0] = (getPiston()/2);                            // Constant term / Piston
        terms[1] = (getTiltX()/2)*divX;                        // Tilt X
        terms[2] = (getTiltY()/2)*divY;                        // Tilt Y
        terms[3] = (getPower()/2)*(3*rho*rho - 1);        // Defocus?               
        terms[4] = (getAstigmatismY()/2)*(divX*divY);
        terms[5] = (getAstigmatismX()/2)*(divX*divX- divY*divY);
        terms[6] = (getComaY()/2)*(15*rho*rho - 7)*divY;
        terms[7] = (getComaX()/2)*(15*rho*rho - 7)*divX;
        terms[8] = (getSphericalAberration()/2)*(31 - 240*rho*rho + 315*rho*rho*rho*rho);
        terms[9] = 0; //(getTrefoilX()/2)*(divXSqu*divX - 3*divX*divYSqu);
        terms[10] = 0; //(getTrefoilY()/2)*(3*divXSqu*divY - divYSqu*divY);
        terms[11] = 0; //(getSecondaryAstigmatismX()/2)*(3*divYSqu - 3*divXSqu + 4*divXSqu*XSquPlusYSqu - 4*divYSqu*XSquPlusYSqu);
        terms[12] = 0; //(getSecondaryAstigmatismY()/2)*(8*divX*divY*XSquPlusYSqu - 6*divX*divY);
        terms[13] = 0; //(getSecondaryComaX()/2)*(3*divX - 12*divX*XSquPlusYSqu + 10*divX*XPYSquSqu);
        terms[14] = 0; //(getSecondaryComaY()/2)*(3*divY - 12*divY*XSquPlusYSqu + 10*divY*XPYSquSqu);
        terms[15] = 0; //(getSecondarySphericalAberration()/2)*(12*XSquPlusYSqu - 1 - 30*XPYSquSqu + 20*XSquPlusYSqu*XPYSquSqu);        


        
        // Add the terms together.
        total = 0;
        for (int i = 0; i < 16; i++) {
          total += terms[i];
        }
      } else {
        total = 0;
      }
      
      zern[row*SLMSIZE+col] = total;
      total = 0;
      x++;
    }
    
    y--;
  }
  
  memset(phaseData, 0, sizeof(unsigned char)*SLMSIZE*SLMSIZE);
  DtoI(zern, SLMSIZE*SLMSIZE, phaseData);
  delete zern;
  
  return;
}


void ZernikeSquarePolynomial::dumpString()
{
  std::ostringstream logSS;
  // Piston.
  logSS << "Piston: " <<  getPiston();
  LOGME( logSS.str() );
  logSS.str("");
  // Power
  logSS << "Power: " << getPower();
  LOGME( logSS.str() );
  logSS.str("");
  // Astigmatism X
  logSS << "AstigX: " << getAstigmatismX();
  LOGME( logSS.str() );
  logSS.str("");
  // Astigmatism Y
  logSS << "AstigY: " << getAstigmatismY();
  LOGME( logSS.str() );
  logSS.str("");
  // Coma X
  logSS << "ComaX: " << getComaX();
  LOGME( logSS.str() );
  logSS.str("");
  // Coma Y
  logSS << "ComaY: " << getComaY();
  LOGME( logSS.str() );
  logSS.str("");
  // Spherical ab.
  logSS << "SphericalAb: " << getSphericalAberration();
  LOGME( logSS.str() );
  logSS.str("");
  // Trefoil X
  logSS << "TrefoilX: " << getTrefoilX();
  LOGME( logSS.str() );
  logSS.str("");
  // Trefoil Y
  logSS << "TrefoilY: " << getTrefoilY();
  LOGME( logSS.str() );
  logSS.str("");
  // 2nd coma x
  logSS << "2nd ComaX: " << getSecondaryComaX();
  LOGME( logSS.str() );
  logSS.str("");
  // 2nd coma y
  logSS << "2nd ComaY: " << getSecondaryComaY();
  LOGME( logSS.str() );
  logSS.str("");
  // 2nd spherical ab.
  logSS << "2nd SpherAb: " << getSecondarySphericalAberration();
  LOGME( logSS.str() );
  logSS.str("");
  // 2nd astigm. x
  logSS << "2nd AstigX: " << getSecondaryAstigmatismX();
  LOGME( logSS.str() );
  logSS.str("");
  // 2nd astigm. y
  logSS << "2nd AstigY: " << getSecondaryAstigmatismY();
  LOGME( logSS.str() );
  logSS.str("");
  // tilt x
  logSS << "Tilt X: " << getTiltX();
  LOGME( logSS.str() );
  logSS.str("");
  // tilt y
  logSS << "Tilt Y: " << getTiltY();
  LOGME( logSS.str() );
  logSS.str("");
  // Strehl estimate
  logSS << "Strehl estimate: " << StrehlEstimate();
  LOGME( logSS.str() );
  logSS.str("");
}


/* Estimation of the Strehl ratio (works for small aberrations). */
/*
 * Only takes into account (for now):
 * - Astigmatism X
 * - Astigmatism Y
 * - Coma Y
 * - Coma X
 * - Spherical Aberration
 */
double ZernikeSquarePolynomial::StrehlEstimate()
{
	double MAY = getAstigmatismY();
	double MAX = getAstigmatismX();
	double MCY = getComaY();
	double MCX = getComaY();
	double MSA = getSphericalAberration();

	double ZAY = MAY / 2.449;
	double ZAX = MAX / 2.449;
	double ZCY = MCY / 5.657;
	double ZCX = MCX / 5.657;
	double ZSA = MSA / 4.472;

	// The variance is the sum of the squared coefficients (using Mahajan's notation)
	double variance = (ZAY*ZAY)+(ZAX*ZAX)+(ZCY*ZCY)+(ZCX*ZCX)+(ZSA*ZSA);

	// Empirical formula, see Mahajan's paper for instance.
	double strehlRatio = exp(-variance);
	return strehlRatio;
}


double ZernikeSquarePolynomial::focusCorrection()
{
  double SA, D;

  D=0;

  //NB. this version is based on SA only. (not including S2A).
  SA = getSphericalAberration()*3;
  if (abs(SA) <= 2.30) {
    D = 0;
  } else if (abs(SA) <= 3.80) {
    D = 1;
  } else if (abs(SA) <= 5.40) {
    D = 2;
  } else if (abs(SA) <= 6.60) {
    D = 3;
  } else if (abs(SA) <= 7.90) {
    D = 4;
  } else if (abs(SA) <= 9.10) {
    D = 5;
  } else if (abs(SA) <= 10.40) {
    D = 6;
  } else if (abs(SA) <= 11.60) {
    D = 7;
  } else if (abs(SA) <= 12.80) {
    D = 8;
  } else if (abs(SA) <= 14.00) {
    D = 9;
  } else if (abs(SA) <= 15.20) {
    D = 10;
  } else if (abs(SA) <= 16.40) {
    D = 11;
  } else if (abs(SA) <= 17.60) {
    D = 12;
  } else {
    D = 12;
  }

  if (SA < 0) {
    D = -D;
  }

  return D;
  /*return (3*getSphericalAberration() 
    + sqrt(getAstigmatismX()*getAstigmatismX() + getAstigmatismY()*getAstigmatismY())/2.0);*/
}


// Getters and setters.

double ZernikeSquarePolynomial::getPiston()
{
  return piston;
}

void ZernikeSquarePolynomial::setPiston(double piston)
{
  this->piston = piston;
}

double ZernikeSquarePolynomial::getPower()
{
  return power;
}

void ZernikeSquarePolynomial::setPower(double power)
{
  this->power = power;
}

double ZernikeSquarePolynomial::getAstigmatismX()
{
  return astigmatismX;
}

void ZernikeSquarePolynomial::setAstigmatismX(double astigmatismX)
{
  this->astigmatismX = astigmatismX;
}

double ZernikeSquarePolynomial::getAstigmatismY()
{
  return astigmatismY;
}

void ZernikeSquarePolynomial::setAstigmatismY(double astigmatismY)
{
  this->astigmatismY = astigmatismY;
}

double ZernikeSquarePolynomial::getComaX()
{
  return comaX;
}

void ZernikeSquarePolynomial::setComaX(double comaX)
{
  this->comaX = comaX;
}

double ZernikeSquarePolynomial::getComaY()
{
  return comaY;
}

void ZernikeSquarePolynomial::setComaY(double comaY)
{
  this->comaY = comaY;
}

double ZernikeSquarePolynomial::getSphericalAberration()
{
  return sphericalAberration;
}

void ZernikeSquarePolynomial::setSphericalAberration(double sphericalAberration)
{
  this->sphericalAberration = sphericalAberration;
}

double ZernikeSquarePolynomial::getTrefoilX()
{
  return trefoilX;
}

void ZernikeSquarePolynomial::setTrefoilX(double trefoilX)
{
  this->trefoilX = trefoilX;
}

double ZernikeSquarePolynomial::getTrefoilY()
{
  return trefoilY;
}

void ZernikeSquarePolynomial::setTrefoilY(double trefoilY)
{
  this->trefoilY = trefoilY;
}

double ZernikeSquarePolynomial::getSecondaryComaX()
{
  return secondaryComaX;
}

void ZernikeSquarePolynomial::setSecondaryComaX(double secondaryComaX)
{
  this->secondaryComaX = secondaryComaX;
}

double ZernikeSquarePolynomial::getSecondaryComaY()
{
  return secondaryComaY;
}

void ZernikeSquarePolynomial::setSecondaryComaY(double secondaryComaY)
{
  this->secondaryComaY = secondaryComaY;
}

double ZernikeSquarePolynomial::getSecondarySphericalAberration()
{
  return secondarySphericalAberration;
}

void ZernikeSquarePolynomial::setSecondarySphericalAberration(double secondarySphericalAberration)
{
  this->secondarySphericalAberration = secondarySphericalAberration;
}

double ZernikeSquarePolynomial::getSecondaryAstigmatismX()
{
  return secondaryAstigmatismX;
}

void ZernikeSquarePolynomial::setSecondaryAstigmatismX(double secondaryAstigmatismX)
{
  this->secondaryAstigmatismX = secondaryAstigmatismX;
}

double ZernikeSquarePolynomial::getSecondaryAstigmatismY()
{
  return secondaryAstigmatismY;
}

void ZernikeSquarePolynomial::setSecondaryAstigmatismY(double secondaryAstigmatismY)
{
  this->secondaryAstigmatismY = secondaryAstigmatismY;
}

double ZernikeSquarePolynomial::getTiltX()
{
  return tiltX;
}

void ZernikeSquarePolynomial::setTiltX(double tiltX)
{
  this->tiltX = tiltX;
}

double ZernikeSquarePolynomial::getTiltY()
{
  return tiltY;
}

void ZernikeSquarePolynomial::setTiltY(double tiltY)
{
  this->tiltY = tiltY;
}

