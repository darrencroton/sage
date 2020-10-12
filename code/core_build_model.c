#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include "core_allvars.h"
#include "core_proto.h"



void construct_galaxies(int halonr, int tree)
{
  static int halosdone = 0;
  int prog, fofhalo, ngal;

  HaloAux[halonr].DoneFlag = 1;
  halosdone++;

  prog = Halo[halonr].FirstProgenitor;
  while(prog >= 0)
  {
    if(HaloAux[prog].DoneFlag == 0)
      construct_galaxies(prog, tree);
    prog = Halo[prog].NextProgenitor;
  }

  fofhalo = Halo[halonr].FirstHaloInFOFgroup;
  if(HaloAux[fofhalo].HaloFlag == 0)
  {
    HaloAux[fofhalo].HaloFlag = 1;
    while(fofhalo >= 0)
    {
      prog = Halo[fofhalo].FirstProgenitor;
      while(prog >= 0)
      {
        if(HaloAux[prog].DoneFlag == 0)
          construct_galaxies(prog, tree);
        prog = Halo[prog].NextProgenitor;
      }

      fofhalo = Halo[fofhalo].NextHaloInFOFgroup;
    }
  }

  // At this point, the galaxies for all progenitors of this halo have been
  // properly constructed. Also, the galaxies of the progenitors of all other 
  // halos in the same FOF group have been constructed as well. We can hence go
  // ahead and construct all galaxies for the subhalos in this FOF halo, and
  // evolve them in time. 

  fofhalo = Halo[halonr].FirstHaloInFOFgroup;
  if(HaloAux[fofhalo].HaloFlag == 1)
  {
    ngal = 0;
    HaloAux[fofhalo].HaloFlag = 2;

    while(fofhalo >= 0)
    {
      ngal = join_galaxies_of_progenitors(fofhalo, ngal);
      fofhalo = Halo[fofhalo].NextHaloInFOFgroup;
    }

    evolve_galaxies(Halo[halonr].FirstHaloInFOFgroup, ngal, tree);
  }

}



int join_galaxies_of_progenitors(int halonr, int ngalstart)
{
  int ngal, prog, i, j, first_occupied, lenmax, lenoccmax, centralgal;
  /* int mother_halo=-1 */
  double previousMvir, previousVvir, previousVmax;
  int step;

  lenmax = 0;
  lenoccmax = 0;
  first_occupied = Halo[halonr].FirstProgenitor;
  prog = Halo[halonr].FirstProgenitor;

  if(prog >=0)
    if(HaloAux[prog].NGalaxies > 0)
    lenoccmax = -1;

  // Find most massive progenitor that contains an actual galaxy
  // Maybe FirstProgenitor never was FirstHaloInFOFGroup and thus has no galaxy

  while(prog >= 0)
  {
    if(Halo[prog].Len > lenmax)
    {
      lenmax = Halo[prog].Len;
      /* mother_halo = prog; */
    }
    if(lenoccmax != -1 && Halo[prog].Len > lenoccmax && HaloAux[prog].NGalaxies > 0)
    {
      lenoccmax = Halo[prog].Len;
      first_occupied = prog;
    }
    prog = Halo[prog].NextProgenitor;
  }

  ngal = ngalstart;
  prog = Halo[halonr].FirstProgenitor;

  while(prog >= 0)
  {
    for(i = 0; i < HaloAux[prog].NGalaxies; i++)
    {
        if(ngal == (FoF_MaxGals-1)) {
            FoF_MaxGals += 10000;
            Gal = myrealloc(Gal, FoF_MaxGals * sizeof(struct GALAXY));
        }
        assert(ngal < FoF_MaxGals);
            

      // This is the cruical line in which the properties of the progenitor galaxies 
      // are copied over (as a whole) to the (temporary) galaxies Gal[xxx] in the current snapshot 
      // After updating their properties and evolving them 
      // they are copied to the end of the list of permanent galaxies HaloGal[xxx] 

      Gal[ngal] = HaloGal[HaloAux[prog].FirstGalaxy + i];
      Gal[ngal].HaloNr = halonr;

      Gal[ngal].dT = -1.0;

      // this deals with the central galaxies of (sub)halos 
      if(Gal[ngal].Type == 0 || Gal[ngal].Type == 1)
      {
        // this halo shouldn't hold a galaxy that has already merged; remove it from future processing
        if(Gal[ngal].mergeType != 0)
        {
          Gal[ngal].Type = 3;
          continue;
        }

        // remember properties from the last snapshot
        previousMvir = Gal[ngal].Mvir;
        previousVvir = Gal[ngal].Vvir;
        previousVmax = Gal[ngal].Vmax;

        if(prog == first_occupied)
        {
          // update properties of this galaxy with physical properties of halo 
          Gal[ngal].MostBoundID = Halo[halonr].MostBoundID;

          for(j = 0; j < 3; j++)
          {
            Gal[ngal].Pos[j] = Halo[halonr].Pos[j];
            Gal[ngal].Vel[j] = Halo[halonr].Vel[j];
          }
					
          Gal[ngal].Len = Halo[halonr].Len;
          Gal[ngal].Vmax = Halo[halonr].Vmax;

					Gal[ngal].deltaMvir = get_virial_mass(halonr) - Gal[ngal].Mvir;

          if(get_virial_mass(halonr) > Gal[ngal].Mvir)
          {
            Gal[ngal].Rvir = get_virial_radius(halonr);  // use the maximum Rvir in model
            Gal[ngal].Vvir = get_virial_velocity(halonr);  // use the maximum Vvir in model
          }
          Gal[ngal].Mvir = get_virial_mass(halonr);

          Gal[ngal].Cooling = 0.0;
          Gal[ngal].Heating = 0.0;
          Gal[ngal].QuasarModeBHaccretionMass = 0.0;
          Gal[ngal].OutflowRate = 0.0;

          for(step = 0; step < STEPS; step++)
          {
            Gal[ngal].SfrDisk[step] = Gal[ngal].SfrBulge[step] = 0.0;
            Gal[ngal].SfrDiskColdGas[step] = Gal[ngal].SfrDiskColdGasMetals[step] = 0.0;
            Gal[ngal].SfrBulgeColdGas[step] = Gal[ngal].SfrBulgeColdGasMetals[step] = 0.0;
          }

          if(halonr == Halo[halonr].FirstHaloInFOFgroup)
          {
            // a central galaxy
            Gal[ngal].mergeType = 0;
            Gal[ngal].mergeIntoID = -1;
            Gal[ngal].MergTime = 999.9;            

            Gal[ngal].DiskScaleRadius = get_disk_radius(halonr, ngal);

            Gal[ngal].Type = 0;
          }
          else
          {
            // a satellite with subhalo
            Gal[ngal].mergeType = 0;
            Gal[ngal].mergeIntoID = -1;

            if(Gal[ngal].Type == 0)  // remember the infall properties before becoming a subhalo
            {
              Gal[ngal].infallMvir = previousMvir;
              Gal[ngal].infallVvir = previousVvir;
              Gal[ngal].infallVmax = previousVmax;
            }

            if(Gal[ngal].Type == 0 || Gal[ngal].MergTime > 999.0)
              // here the galaxy has gone from type 1 to type 2 or otherwise doesn't have a merging time.
              Gal[ngal].MergTime = estimate_merging_time(halonr, Halo[halonr].FirstHaloInFOFgroup, ngal);
            
            Gal[ngal].Type = 1;
          }
        }
        else
        {
          // an orhpan satellite galaxy - these will merge or disrupt within the current timestep
          Gal[ngal].deltaMvir = -1.0*Gal[ngal].Mvir;
          Gal[ngal].Mvir = 0.0;

          if(Gal[ngal].MergTime > 999.0 || Gal[ngal].Type == 0)
          {
            // here the galaxy has gone from type 0 to type 2 - merge it!
            Gal[ngal].MergTime = 0.0;
          
            Gal[ngal].infallMvir = previousMvir;
            Gal[ngal].infallVvir = previousVvir;
            Gal[ngal].infallVmax = previousVmax;
          }

          Gal[ngal].Type = 2;
        }
      }

      ngal++;

    }

    prog = Halo[prog].NextProgenitor;
  }

  if(ngal == 0)
  {
    // We have no progenitors with galaxies. This means we create a new galaxy. 
    init_galaxy(ngal, halonr);
    ngal++;
  }

  // Per Halo there can be only one Type 0 or 1 galaxy, all others are Type 2  (orphan)
  // In fact, this galaxy is very likely to be the first galaxy in the halo if 
	// first_occupied==FirstProgenitor and the Type0/1 galaxy in FirstProgenitor was also the first one 
  // This cannot be guaranteed though for the pathological first_occupied!=FirstProgenitor case 

  for(i = ngalstart, centralgal = -1; i < ngal; i++)
  {
    if(Gal[i].Type == 0 || Gal[i].Type == 1)
    {
			assert(centralgal == -1);
      centralgal = i;
    }
  }

  for(i = ngalstart; i < ngal; i++)
    Gal[i].CentralGal = centralgal;

  return ngal;

}



void evolve_galaxies(int halonr, int ngal, int tree)	// Note: halonr is here the FOF-background subhalo (i.e. main halo) 
{
  int p, i, step, centralgal, merger_centralgal, currenthalo, offset;
  double infallingGas, coolingGas, deltaT, time, galaxyBaryons, currentMvir;

  centralgal = Gal[0].CentralGal;
	assert(Gal[centralgal].Type == 0 && Gal[centralgal].HaloNr == halonr);

  infallingGas = infall_recipe(centralgal, ngal, ZZ[Halo[halonr].SnapNum]);

  // We integrate things forward by using a number of intervals equal to STEPS 
  for(step = 0; step < STEPS; step++)
  {

    // Loop over all galaxies in the halo 
    for(p = 0; p < ngal; p++)
    {
      // Don't treat galaxies that have already merged 
      if(Gal[p].mergeType > 0)
        continue;

      deltaT = Age[Gal[p].SnapNum] - Age[Halo[halonr].SnapNum];
      time = Age[Gal[p].SnapNum] - (step + 0.5) * (deltaT / STEPS);
      
      if(Gal[p].dT < 0.0)
        Gal[p].dT = deltaT;

      // For the central galaxy only 
      if(p == centralgal)
      {
        add_infall_to_hot(centralgal, infallingGas / STEPS);

        if(ReIncorporationFactor > 0.0)
          reincorporate_gas(centralgal, deltaT / STEPS);
      }
			else 
				if(Gal[p].Type == 1 && Gal[p].HotGas > 0.0)
					strip_from_satellite(halonr, centralgal, p);

      // Determine the cooling gas given the halo properties 
      coolingGas = cooling_recipe(p, deltaT / STEPS);
      cool_gas_onto_galaxy(p, coolingGas);

      // stars form and then explode! 
      starformation_and_feedback(p, centralgal, time, deltaT / STEPS, halonr, step);
    }

    // check for satellite disruption and merger events 
    for(p = 0; p < ngal; p++)
    {

      if((Gal[p].Type == 1 || Gal[p].Type == 2) && Gal[p].mergeType == 0)  // satellite galaxy!
      {
				assert(Gal[p].MergTime < 999.0);

        deltaT = Age[Gal[p].SnapNum] - Age[Halo[halonr].SnapNum];
        Gal[p].MergTime -= deltaT / STEPS;
        
        // only consider mergers or disruption for halo-to-baryonic mass ratios below the threshold
        // or for satellites with no baryonic mass (they don't grow and will otherwise hang around forever)
        currentMvir = Gal[p].Mvir - Gal[p].deltaMvir * (1.0 - ((double)step + 1.0) / (double)STEPS);
        galaxyBaryons = Gal[p].StellarMass + Gal[p].ColdGas;
        if((galaxyBaryons == 0.0) || (galaxyBaryons > 0.0 && (currentMvir / galaxyBaryons <= ThresholdSatDisruption)))        
        {
          if(Gal[p].Type==1) 
            merger_centralgal = centralgal;
          else
            merger_centralgal = Gal[p].CentralGal;

          if(Gal[merger_centralgal].mergeType > 0) 
            merger_centralgal = Gal[merger_centralgal].CentralGal;

          Gal[p].mergeIntoID = NumGals + merger_centralgal;  // position in output 

          if(Gal[p].MergTime > 0.0)  // disruption has occured!
          {
            disrupt_satellite_to_ICS(merger_centralgal, p);
          }
          else
          {
            if(Gal[p].MergTime <= 0.0)  // a merger has occured! 
            {
              time = Age[Gal[p].SnapNum] - (step + 0.5) * (deltaT / STEPS);   
              deal_with_galaxy_merger(p, merger_centralgal, centralgal, time, deltaT / STEPS, halonr, step);
            }
          } 
        }
        
      }
    }

  } // Go on to the next STEPS substep


  // Extra miscellaneous stuff before finishing this halo
	Gal[centralgal].TotalSatelliteBaryons = 0.0;
  deltaT = Age[Gal[0].SnapNum] - Age[Halo[halonr].SnapNum];
	
  for(p = 0; p < ngal; p++)
  {

    // Don't bother with galaxies that have already merged 
    if(Gal[p].mergeType > 0)
      continue;
		
    Gal[p].Cooling /= deltaT;
    Gal[p].Heating /= deltaT;
    Gal[p].OutflowRate /= deltaT;    
		
    if(p != centralgal)
			Gal[centralgal].TotalSatelliteBaryons += 
				(Gal[p].StellarMass + Gal[p].BlackHoleMass + Gal[p].ColdGas + Gal[p].HotGas);
  }


  // Attach final galaxy list to halo 
  offset = 0;
  for(p = 0, currenthalo = -1; p < ngal; p++)
  {
    if(Gal[p].HaloNr != currenthalo)
    {
      currenthalo = Gal[p].HaloNr;
      HaloAux[currenthalo].FirstGalaxy = NumGals;
      HaloAux[currenthalo].NGalaxies = 0;
    }

    // Merged galaxies won't be output. So go back through its history and find it
    // in the previous timestep. Then copy the current merger info there.
    offset = 0;
    i = p-1;
    while(i >= 0)
    {
     if(Gal[i].mergeType > 0) 
       if(Gal[p].mergeIntoID > Gal[i].mergeIntoID)
         offset++;  // these galaxies won't be kept so offset mergeIntoID below
     i--;
    }
    
    i = -1;
    if(Gal[p].mergeType > 0)
    {
      i = HaloAux[currenthalo].FirstGalaxy - 1;
      while(i >= 0)
      {
        if(HaloGal[i].GalaxyNr == Gal[p].GalaxyNr)
          break;
        else
          i--;
      }
      
			assert(i >= 0);
      
      HaloGal[i].mergeType = Gal[p].mergeType;
      HaloGal[i].mergeIntoID = Gal[p].mergeIntoID - offset;
      HaloGal[i].mergeIntoSnapNum = Halo[currenthalo].SnapNum;
    }
    
    if(Gal[p].mergeType == 0)
    {
			assert(NumGals < MaxGals);

      Gal[p].SnapNum = Halo[currenthalo].SnapNum;
      HaloGal[NumGals++] = Gal[p];
      HaloAux[currenthalo].NGalaxies++;
    }
  }


}
