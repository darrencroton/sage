#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "accessors.h"
#include "globals.h"
#include "types.h"
#include "error_handling.h"

/*
 * SimulationState accessor implementations
 */

int get_galaxy_counter(void)
{
    return SimState.GalaxyCounter;
}

void set_galaxy_counter(int value)
{
    if (value < 0) {
        WARNING_LOG("Attempt to set GalaxyCounter to negative value (%d)", value);
        return;
    }
    SimState.GalaxyCounter = value;
    GalaxyCounter = value; // Update global for backward compatibility
}

void increment_galaxy_counter(void)
{
    SimState.GalaxyCounter++;
    GalaxyCounter = SimState.GalaxyCounter; // Update global for backward compatibility
}

int get_current_tree_id(void)
{
    return SimState.TreeID;
}

void set_current_tree_id(int tree_id)
{
    if (tree_id < 0) {
        WARNING_LOG("Attempt to set TreeID to negative value (%d)", tree_id);
        return;
    }
    SimState.TreeID = tree_id;
    TreeID = tree_id; // Update global for backward compatibility
}

int get_current_file_num(void)
{
    return SimState.FileNum;
}

void set_current_file_num(int file_num)
{
    if (file_num < 0) {
        WARNING_LOG("Attempt to set FileNum to negative value (%d)", file_num);
        return;
    }
    SimState.FileNum = file_num;
    FileNum = file_num; // Update global for backward compatibility
}

int get_num_galaxies(void)
{
    return SimState.NumGals;
}

void set_num_galaxies(int count)
{
    if (count < 0) {
        WARNING_LOG("Attempt to set NumGals to negative value (%d)", count);
        return;
    }
    SimState.NumGals = count;
    NumGals = count; // Update global for backward compatibility
}

void increment_num_galaxies(int delta)
{
    if (SimState.NumGals + delta < 0) {
        WARNING_LOG("Attempt to increment NumGals below zero (current: %d, delta: %d)", 
                    SimState.NumGals, delta);
        return;
    }
    
    if (SimState.NumGals + delta > SimState.MaxGals) {
        WARNING_LOG("Attempt to increment NumGals beyond MaxGals (current: %d, delta: %d, max: %d)", 
                    SimState.NumGals, delta, SimState.MaxGals);
        return;
    }
    
    SimState.NumGals += delta;
    NumGals = SimState.NumGals; // Update global for backward compatibility
}

int get_max_galaxies(void)
{
    return SimState.MaxGals;
}

void set_max_galaxies(int count)
{
    if (count <= 0) {
        WARNING_LOG("Attempt to set MaxGals to invalid value (%d)", count);
        return;
    }
    SimState.MaxGals = count;
    MaxGals = count; // Update global for backward compatibility
}

int get_total_halos(void)
{
    return SimState.TotHalos;
}

void set_total_halos(int count)
{
    if (count < 0) {
        WARNING_LOG("Attempt to set TotHalos to negative value (%d)", count);
        return;
    }
    SimState.TotHalos = count;
    TotHalos = count; // Update global for backward compatibility
}

int get_ntrees(void)
{
    return SimState.Ntrees;
}

void set_ntrees(int count)
{
    if (count < 0) {
        WARNING_LOG("Attempt to set Ntrees to negative value (%d)", count);
        return;
    }
    SimState.Ntrees = count;
    Ntrees = count; // Update global for backward compatibility
}

int* get_output_snap_list(void)
{
    return SimState.ListOutputSnaps;
}

int get_num_output_snaps(void)
{
    return SimState.NOUT;
}

struct SimulationState* get_simulation_state(void)
{
    return &SimState;
}


/*
 * SageConfig accessor implementations
 */

double get_hubble_h(void)
{
    return SageConfig.Hubble_h;
}

double get_omega(void)
{
    return SageConfig.Omega;
}

double get_omega_lambda(void)
{
    return SageConfig.OmegaLambda;
}

double get_baryon_fraction(void)
{
    printf("ACCESSOR DEBUG: get_baryon_fraction called, returning %e\n", SageConfig.BaryonFrac);
    return SageConfig.BaryonFrac;
}

double get_box_size(void)
{
    return SageConfig.BoxSize;
}

double get_unit_length_in_cm(void)
{
    return SageConfig.UnitLength_in_cm;
}

double get_unit_time_in_s(void)
{
    return SageConfig.UnitTime_in_s;
}

double get_unit_mass_in_g(void)
{
    return SageConfig.UnitMass_in_g;
}

double get_unit_velocity_in_cm_per_s(void)
{
    return SageConfig.UnitVelocity_in_cm_per_s;
}

double get_sn_energy(void)
{
    return SageConfig.EnergySN;
}

double get_sn_energy_code_units(void)
{
    return SageConfig.EnergySNcode;
}

double get_sn_mass_fraction(void)
{
    return SageConfig.EtaSN;
}

double get_sn_mass_fraction_code_units(void)
{
    return SageConfig.EtaSNcode;
}

double get_recycle_fraction(void)
{
    return SageConfig.RecycleFraction;
}

double get_sfr_efficiency(void)
{
    printf("ACCESSOR DEBUG: get_sfr_efficiency called, returning %e\n", SageConfig.SfrEfficiency);
    return SageConfig.SfrEfficiency;
}

double get_feedback_reheat_efficiency(void)
{
    return SageConfig.FeedbackReheatingEpsilon;
}

double get_feedback_ejection_efficiency(void)
{
    return SageConfig.FeedbackEjectionEfficiency;
}

int get_reionization_on(void)
{
    return SageConfig.ReionizationOn;
}

double get_reionization_z0(void)
{
    return SageConfig.Reionization_z0;
}

double get_reionization_zr(void)
{
    return SageConfig.Reionization_zr;
}

int get_disk_instability_on(void)
{
    return SageConfig.DiskInstabilityOn;
}

int get_agn_recipe_on(void)
{
    return SageConfig.AGNrecipeOn;
}

double get_radio_mode_efficiency(void)
{
    return SageConfig.RadioModeEfficiency;
}

double get_quasar_mode_efficiency(void)
{
    return SageConfig.QuasarModeEfficiency;
}

double get_black_hole_growth_rate(void)
{
    return SageConfig.BlackHoleGrowthRate;
}

double get_threshold_major_merger(void)
{
    return SageConfig.ThreshMajorMerger;
}

const char* get_output_directory(void)
{
    return SageConfig.OutputDir;
}

const char* get_simulation_directory(void)
{
    return SageConfig.SimulationDir;
}

const char* get_tree_name(void)
{
    return SageConfig.TreeName;
}

const char* get_galaxies_filename(void)
{
    return SageConfig.FileNameGalaxies;
}

struct SageConfig* get_sage_config(void)
{
    return &SageConfig;
}
