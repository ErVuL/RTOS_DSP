#include <signalProc_cortexM4.h>
#include "stm32f407xx.h"
#include "arm_math.h"
#include <stdint.h>


/* Exported functions */

/*
 * Generate n Gaussian random numbers with user define std_dev and mean.
 * Uses the Box-Muller transformation of two uniform random numbers
 * to Gaussian random numbers.
 *
 *	REF:
 *		- "C ALGORITHMS FOR REAL TIME DSP", Paul M.EMBREE, p.158.
 *		  Prentice Hall, ISBN : 0-13-337353-3.
 *
 *	ARG:
 * 		- q31_t std_dev       : Variance of the Gaussian distribution.
 * 		- q31_t mean          : Mean of the Gaussian distribution.
 *		- q31_t *outputBuffer : Output buffer of size n.
 *		- uint32_t n          : Number of generated random numbers.
 *
 * 	RETURN:
 * 		- void
 *
 * 	TODO:
 * 		- Test it !
 *
 */
#include "usbd_cdc_if.h"

void randGauss_q31(q31_t std_dev, q31_t mean, q31_t *outputBuffer, uint32_t n)
{
	static _Bool ready = FALSE; 										// Flag to indicated stored value
	static q31_t gstore;     											// Place to store other value
	static const float32_t rconst1 = (float32_t) (2.0/RAND_MAX);
	static const float32_t rconst2 = (float32_t) (RAND_MAX/2.0);
	float32_t fac ,v1, v2, r;
	//srand(HAL_GetTick());

	for(uint32_t i=0; i<n; i++)
	{
		/* Make two numbers if none stored */
		if(!ready)
		{
			do
			{
				v1 = (rand()-rconst2)*rconst1;
				v2 = (rand()-rconst2)*rconst1;
				r  = v1*v1 + v2*v2;
			} while(r > 1.0f); 										    // Make radius less than 1

			/* Remap v1 and v2 to two Gaussian numbers */
			arm_sqrt_f32((float32_t)(-2.0*log((double)r)/r), &fac);
			gstore = (q31_t) (std_dev*v1*fac + mean); 					// Store one
			outputBuffer[i] = (q31_t) (std_dev*v2*fac + mean);			// Return one
			ready = TRUE;												// Set ready flag
		}
		else
		{
			ready = FALSE;												// Reset ready flag for next pair
			outputBuffer[i] = gstore;								    // Return the stored one
		}
	}
}



//===========================================================================
//=  Function to generate normally distributed random variable using the    =
//=  Box-Muller method                                                      =
//=    - Input: mean and standard deviation                                 =
//=    - Output: Returns with normally distributed random variable          =
//===========================================================================
void norm(double std_dev, double mean, q31_t *outputBuffer, uint32_t n)
{
  double   u, r, theta;           // Variables for Box-Muller method
  double   x;                     // Normal(0, 1) rv
  double   norm_rv;               // The adjusted normal rv

  //rand_val(HAL_GetTick());

  for(uint32_t i=0; i<n; i++)
  {
	  // Generate u
	  u = 0.0;
	  while (u == 0.0)
	    u = rand_val(0);

	  // Compute r
	  r = sqrt(-2.0 * log(u));

	  // Generate theta
	  theta = 0.0;
	  while (theta == 0.0)
	    theta = 2.0 * PI * rand_val(0);

	  // Generate x value
	  x = r * cos(theta);

	  // Adjust x value for specified mean and variance
	  norm_rv = (x * std_dev) + mean;

	  // Return the normally distributed RV value
	  outputBuffer[i] = (q31_t) norm_rv;
  }
}


//=========================================================================
//= Multiplicative LCG for generating uniform(0.0, 1.0) random numbers    =
//=   - x_n = 7^5*x_(n-1)mod(2^31 - 1)                                    =
//=   - With x seeded to 1 the 10000th x value should be 1043618065       =
//=   - From R. Jain, "The Art of Computer Systems Performance Analysis," =
//=     John Wiley & Sons, 1991. (Page 443, Figure 26.2)                  =
//=========================================================================
double rand_val(int seed)
{
  const long  a =      16807;  // Multiplier
  const long  m = 2147483647;  // Modulus
  const long  q =     127773;  // m div a
  const long  r =       2836;  // m mod a
  static long x;               // Random int value
  long        x_div_q;         // x divided by q
  long        x_mod_q;         // x modulo q
  long        x_new;           // New x value

  // Set the seed if argument is non-zero and then return zero
  if (seed > 0)
  {
    x = seed;
    return(0.0);
  }

  // RNG using integer arithmetic
  x_div_q = x / q;
  x_mod_q = x % q;
  x_new = (a * x_mod_q) - (r * x_div_q);
  if (x_new > 0)
    x = x_new;
  else
    x = x_new + m;

  // Return a random value between 0.0 and 1.0
  return((double) x / m);
}



