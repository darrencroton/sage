#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <mpi.h>

#include "core_allvars.h"
#include "core_proto.h"



void construct_galaxies(int halonr)
{
  static int halosdone = 0;
  int prog, fofhalo, ngal;

  HaloAux[halonr].DoneFlag = 1;
  halosdone++;

  prog = Halo[halonr].FirstProgenitor;
  while(prog >= 0)
  {
    if(HaloAux[prog].DoneFlag == 0)
      construct_galaxies(prog);
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
          construct_galaxies(prog);
        prog = Halo[prog].NextProgenitor;
      }

      fofhalo = Halo[fofhalo].NextHaloInFOFgroup;
    }
  }

  // At this point, the galaxies for all progenitors of this halo have been
  // properly constructed. Also, the galaxies of the progenitors of all other 
  // halos in the same FOF-group have been constructed as well. We can hence go
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

    evolve_galaxies(Halo[halonr].FirstHaloInFOFgroup, ngal);
  }

}



int join_galaxies_of_progenitors(int halonr, int ngalstart)
{
  int ngal, prog, mother_halo=-1, i, j, first_occupied, lenmax, lenoccmax, centralgal;

  lenmax = 0;
  lenoccmax = 0;
  first_occupied = Halo[halonr].FirstProgenitor;
  prog = Halo[halonr].FirstProgenitor;

  if(prog >=0)
    if(HaloAux[prog].NGalaxies > 0)
    lenoccmax = -1;

  // Find most massive progenitor that contains an actual galaxy
  // Maybe FirstProgenitor never was FirstHaloInFOFGroup and thus got no galaxy

  while(prog >= 0)
  {
    if(Halo[prog].Len > lenmax)
    {
      lenmax = Halo[prog].Len;
      mother_halo = prog;
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
      if(ngal >= FoF_MaxGals)
      {
        printf("Opps. We reached the maximum number FoF_MaxGals=%d of galaxies in FoF... exiting.\n", FoF_MaxGals);
        ABORT(1);
      }

      // This is the cruical line in which the properties of the progenitor galaxies 
      // are copied over (as a whole) to the (temporary) galaxies Gal[xxx] in the current snapshot 
      // After updating their properties and evolving them 
      // they are copied to the end of the list of permanent galaxies HaloGal[xxx] 

      Gal[ngal] = HaloGal[HaloAux[prog].FirstGalaxy + i];
      Gal[ngal].HaloNr = halonr;      

      // this deals with the central galaxies of (sub)halos 
      if(Gal[ngal].Type == 0 || Gal[ngal].Type == 1)
      {
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

          // Gal[ngal].deltaMvir = get_virial_mass(halonr) - Gal[ngal].Mvir;  // XXX - New infall mass determination
          
          Gal[ngal].Mvir = get_virial_mass(halonr);
          Gal[ngal].Rvir = get_virial_radius(halonr);
          Gal[ngal].Vvir = get_virial_velocity(halonr);

          Gal[ngal].DiskScaleRadius = get_disk_radius(halonr, ngal);

          if(halonr == Halo[halonr].FirstHaloInFOFgroup)
          {
            // central galaxy
            Gal[ngal].AlreadyMerged = 0;
            Gal[ngal].Type = 0;

            // // XXX - New stable galaxy growth: type 0 halos retain their maximum values
            // if(get_virial_mass(halonr) > Gal[ngal].Mvir) 
            // {
            //   Gal[ngal].Mvir = get_virial_mass(halonr);
            //   Gal[ngal].Rvir = get_virial_radius(halonr);
            //   Gal[ngal].Vvir = get_virial_velocity(halonr);
            // }

          }
          else
          {
            // subhalo satellite galaxy
            if(Gal[ngal].Type == 0 || Gal[ngal].MergTime > 999.0)
              // here the galaxy has gone from type 1 to type 2 or otherwise doesn't have a merging time.
              Gal[ngal].MergTime = estimate_merging_time(halonr, Halo[halonr].FirstHaloInFOFgroup, ngal);
            Gal[ngal].Type = 1;

            // // XXX - New stable galaxy growth: type 1 halos are allowed to decay
            // Gal[ngal].Mvir = get_virial_mass(halonr);
            // Gal[ngal].Rvir = get_virial_radius(halonr);
            // Gal[ngal].Vvir = get_virial_velocity(halonr);

          }
        }
        else
        {
          // orhpan satellite galaxy
          if(Gal[ngal].MergTime > 999.0)
            // Here the galaxy has gone from type 0 to type 2. Merge it!
            Gal[ngal].MergTime = 0.0;
          Gal[ngal].Type = 2;
          Gal[ngal].Mvir = 0.0;
        }
      }

      // Note: Galaxies that are already type 2 do not need special treatment at this point 
      if(Gal[ngal].Type < 0 || Gal[ngal].Type > 2)
      {
        printf("what's that????\n");
        ABORT(88);
      }

      ngal++;

    }

    prog = Halo[prog].NextProgenitor;
  }

  if(ngal == 0)
  {
    // We have no progenitors with galaxies. This means we create a new galaxy. 
    // However, if it's a subhalo, we don't place a galaxy in it,
    // because it would stay at zero luminosity anyway.

    if(Halo[halonr].FirstHaloInFOFgroup == halonr)
    {
      init_galaxy(ngal, halonr);
      ngal++;
    }
  }

  // Per Halo there can be only one Type 0 or 1 galaxy, all others are Type 2 
  // In fact this galaxy is very likely to be the first galaxy in the halo if first_occupied==FirstProgenitor 
  // and the Type0/1 galaxy in FirstProgenitor was also the first one 
  // This cannot be guaranteed though for the pathological first_occupied != FirstProgenitor case 

  for(i = ngalstart, centralgal = -1; i < ngal; i++)
  {
    if(Gal[i].Type == 0 || Gal[i].Type == 1)
    {
      if(centralgal != -1)
      {
        printf("can't be\n");
        ABORT(8);
      }
      centralgal = i;
    }
  }

  for(i = ngalstart; i < ngal; i++)
    Gal[i].CentralGal = centralgal;

  return ngal;

}



void evolve_galaxies(int halonr, int ngal)	// note: halonr is here the FOF-background subhalo (i.e. main halo) 
{
  int p, nstep, centralgal, merger_centralgal, currenthalo;
  double infallingGas, coolingGas, deltaT, time, galaxyBaryons;

  centralgal = Gal[0].CentralGal;
  if(Gal[centralgal].Type != 0 || Gal[centralgal].HaloNr != halonr)
  {
    printf("Something wrong here ..... \n");
    ABORT(54);
  }

  // basically compute the diff between the hot gas obtained at the end of the 
  // previous snapshot and the one obtained at the beginning of the new snapshot
  // using the conservation of baryons 

  infallingGas = infall_recipe(centralgal, ngal, ZZ[Halo[halonr].SnapNum]);

  // we integrate things forward by using a number of intervals equal to STEPS 
  for(nstep = 0; nstep < STEPS; nstep++)
  {

    // Loop over all galaxies in the halo 
    for(p = 0; p < ngal; p++)
    {
      // don't treat galaxies that have already merged 
      if(Gal[p].Type == 3 || Gal[p].AlreadyMerged == 1)
        continue;

      deltaT = Age[Gal[p].SnapNum] - Age[Halo[halonr].SnapNum];
      time = Age[Gal[p].SnapNum] - (nstep + 0.5) * (deltaT / STEPS);

      // for central galaxy only 
      if(p == centralgal)
      {
        add_infall_to_hot(centralgal, infallingGas / STEPS);

        if(ReIncorporationFactor > 0.0)
          reincorporate_gas(centralgal, deltaT / STEPS);
      }
      else if(Gal[p].Type == 1 && Gal[p].HotGas > 0.0)
        strip_from_satellite(halonr, centralgal, p);

      // determine cooling gas given halo properties 
      coolingGas = cooling_recipe(p, deltaT / STEPS);
      cool_gas_onto_galaxy(p, coolingGas);

      // stars form and then explode! 
      starformation_and_feedback(p, centralgal, time, deltaT / STEPS, halonr);
    }

    // check for satellite disruption and merger events 
    for(p = 0; p < ngal; p++)
    {

      if(Gal[p].Type == 2 && Gal[p].AlreadyMerged == 1)
        Gal[p].Type = 3;

      if((Gal[p].Type == 1 || Gal[p].Type == 2) && Gal[p].AlreadyMerged == 0)  // satellite galaxy!
      {
        if(Gal[p].MergTime > 999.0)
        {
          printf("satellite doesn't have a merging time! %f\n", Gal[p].MergTime);
          ABORT(77);
        }

        deltaT = Age[Gal[p].SnapNum] - Age[Halo[halonr].SnapNum];
        Gal[p].MergTime -= deltaT / STEPS;

        // only consider mergers or disruption for halo-to-baryonic mass ratios below the threshold
        galaxyBaryons = Gal[p].StellarMass + Gal[p].ColdGas;
        if(galaxyBaryons > 0.0 && (Gal[p].Mvir / galaxyBaryons <= ThresholdSatDisruption))
        {
          if(Gal[p].MergTime > 0.0)  // disruption has occured!
          {
            disrupt_satellite_to_ICS(centralgal, p);
            Gal[p].AlreadyMerged = 1;
          }
          else
          {
            if(Gal[p].MergTime < 0.0)  // a merger has occured! 
            {
              merger_centralgal = Gal[p].CentralGal;
              if(Gal[merger_centralgal].AlreadyMerged == 1) 
                merger_centralgal = Gal[merger_centralgal].CentralGal;

              time = Age[Gal[p].SnapNum] - (nstep + 0.5) * (deltaT / STEPS);
	 
	      // TIBO: deal with embarrassing case where galaxy merge with itself... merger_centralgal = p
	      // deal_with_galaxy_merger(p, merger_centralgal, centralgal, time, deltaT / STEPS, halonr);
              // Gal[p].AlreadyMerged = 1;
	      
	      //////////////////////// Make sure that it no longer happens
	      if (p != merger_centralgal)
		{
		  deal_with_galaxy_merger(p, merger_centralgal, centralgal, time, deltaT / STEPS, halonr);
		}
              Gal[p].AlreadyMerged = 1;
	      ////////////////////////

            // flag galaxy as finished 
              if(Gal[p].Type == 2) 
                Gal[p].Type = 3;
            }
          }
        }

      }
      
    }

  } // end move forward in interval STEPS 


  // extra miscellaneous stuff 
  deltaT = Age[Gal[0].SnapNum] - Age[Halo[halonr].SnapNum];
  for(p = 0; p < ngal; p++)
  {
    Gal[p].Cooling /= deltaT;
    Gal[p].Heating /= deltaT;
  }


  // attach final galaxy list to halo 
  for(p = 0, currenthalo = -1; p < ngal; p++)
  {
    if(Gal[p].HaloNr != currenthalo)
    {
      currenthalo = Gal[p].HaloNr;
      HaloAux[currenthalo].FirstGalaxy = NumGals;
      HaloAux[currenthalo].NGalaxies = 0;
    }

    if(Gal[p].Type != 3)
    {
      if(NumGals >= MaxGals)
      {
        printf("maximum number of galaxies reached... exiting.\n");
        ABORT(1);
      }

      Gal[p].SnapNum = Halo[currenthalo].SnapNum;
      // Gal[p].GalaxyNr = NumGals;
      HaloGal[NumGals++] = Gal[p];
      HaloAux[currenthalo].NGalaxies++;

    }
  }


}

