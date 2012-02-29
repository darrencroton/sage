#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"



void reincorporate_gas(int centralgal, double dt)
{
  double reincorporated, metallicity;
  
  // // XXX - New ejection/reincorporation scheme
  // // SN velocity is 630km/s, and the condition for reincorporation is that the 
  // // halo has an escape velocity greater than this, i.e. V_SN/sqrt(2) = 445.48km/s
  // double Vcrit = 445.48;  
  // if(Gal[centralgal].Vvir > Vcrit)
  // {
  //   // reincorporate ejected gas back into the central galaxy hot halo 
  //   reincorporated = 
  //     ( pow(Gal[centralgal].Vvir / Vcrit, 3.0) - 1.0 ) *
  //     Gal[centralgal].EjectedMass / (Gal[centralgal].Rvir / Gal[centralgal].Vvir) * dt; 
  {  
    reincorporated =
      ReIncorporationFactor * Gal[centralgal].EjectedMass / (Gal[centralgal].Rvir / Gal[centralgal].Vvir) * dt;

    if(reincorporated > Gal[centralgal].EjectedMass)
      reincorporated = Gal[centralgal].EjectedMass;

    metallicity = get_metallicity(Gal[centralgal].EjectedMass, Gal[centralgal].MetalsEjectedMass);
    Gal[centralgal].EjectedMass -= reincorporated;
    Gal[centralgal].MetalsEjectedMass -= metallicity * reincorporated;
    Gal[centralgal].HotGas += reincorporated;
    Gal[centralgal].MetalsHotGas += metallicity * reincorporated;
  }

}
