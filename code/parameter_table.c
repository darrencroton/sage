#include <string.h>
#include <stdio.h>
#include "parameter_table.h"
#include "globals.h"
#include "types.h"
#include "constants.h"
#include "config.h"

static ParameterDefinition ParameterTable[] = {
  // Format: {name, description, type, address, required, min_value, max_value}
  
  // File information parameters
  {"FileNameGalaxies", "Base name of galaxy output files", STRING, &SageConfig.FileNameGalaxies, 1, 0.0, 0.0},
  {"OutputDir", "Directory for output files", STRING, &SageConfig.OutputDir, 1, 0.0, 0.0},
  {"TreeName", "Base name of merger tree files", STRING, &SageConfig.TreeName, 1, 0.0, 0.0},
  {"TreeType", "Type of merger tree files (lhalo_binary or genesis_lhalo_hdf5)", STRING, NULL, 1, 0.0, 0.0}, // Special handling needed
  {"SimulationDir", "Directory containing simulation data", STRING, &SageConfig.SimulationDir, 1, 0.0, 0.0},
  {"FileWithSnapList", "File containing snapshot list", STRING, &SageConfig.FileWithSnapList, 1, 0.0, 0.0},
  
  // Simulation parameters
  {"LastSnapShotNr", "Last snapshot number", INT, &SageConfig.LastSnapShotNr, 1, 0.0, ABSOLUTEMAXSNAPS-1},
  {"FirstFile", "First file to process", INT, &SageConfig.FirstFile, 1, 0.0, 0.0},
  {"LastFile", "Last file to process", INT, &SageConfig.LastFile, 1, 0.0, 0.0},
  
  // Output parameters
  {"NumOutputs", "Number of outputs (-1 for all snapshots)", INT, &SageConfig.NOUT, 1, -1.0, ABSOLUTEMAXSNAPS},
  
  // Merger parameters
  {"ThreshMajorMerger", "Threshold for major merger classification", DOUBLE, &SageConfig.ThreshMajorMerger, 1, 0.0, 1.0},
  {"ThresholdSatDisruption", "Mass ratio threshold for satellite disruption", DOUBLE, &SageConfig.ThresholdSatDisruption, 1, 0.0, 0.0},
  
  // Cosmology parameters
  {"Omega", "Matter density parameter", DOUBLE, &SageConfig.Omega, 1, 0.0, 1.0},
  {"OmegaLambda", "Dark energy density parameter", DOUBLE, &SageConfig.OmegaLambda, 1, 0.0, 1.0},
  {"BaryonFrac", "Baryon fraction", DOUBLE, &SageConfig.BaryonFrac, 1, 0.0, 1.0},
  {"Hubble_h", "Hubble parameter (H0/100)", DOUBLE, &SageConfig.Hubble_h, 1, 0.0, 0.0},
  {"PartMass", "Particle mass in simulation", DOUBLE, &SageConfig.PartMass, 1, 0.0, 0.0},
  
  // Unit parameters
  {"UnitVelocity_in_cm_per_s", "Velocity unit in cm/s", DOUBLE, &SageConfig.UnitVelocity_in_cm_per_s, 1, 0.0, 0.0},
  {"UnitLength_in_cm", "Length unit in cm", DOUBLE, &SageConfig.UnitLength_in_cm, 1, 0.0, 0.0},
  {"UnitMass_in_g", "Mass unit in g", DOUBLE, &SageConfig.UnitMass_in_g, 1, 0.0, 0.0},
  
  // Flags (switches)
  {"ReionizationOn", "Reionization modeling switch (0=off, 1=on)", INT, &SageConfig.ReionizationOn, 1, 0.0, 1.0},
  {"SupernovaRecipeOn", "Supernova recipe switch (0=off, 1=on)", INT, &SageConfig.SupernovaRecipeOn, 1, 0.0, 1.0},
  {"DiskInstabilityOn", "Disk instability switch (0=off, 1=on)", INT, &SageConfig.DiskInstabilityOn, 1, 0.0, 1.0},
  {"SFprescription", "Star formation prescription (0=original)", INT, &SageConfig.SFprescription, 1, 0.0, 0.0},
  {"AGNrecipeOn", "AGN feedback switch (0=off, 1/2/3=different models)", INT, &SageConfig.AGNrecipeOn, 1, 0.0, 3.0},
  
  // Model parameters
  {"RecycleFraction", "Mass recycled fraction from stars", DOUBLE, &SageConfig.RecycleFraction, 1, 0.0, 1.0},
  {"ReIncorporationFactor", "Gas reincorporation factor", DOUBLE, &SageConfig.ReIncorporationFactor, 1, 0.0, 0.0},
  {"EnergySN", "Energy per supernova (erg)", DOUBLE, &SageConfig.EnergySN, 1, 0.0, 0.0},
  {"EtaSN", "Supernova efficiency", DOUBLE, &SageConfig.EtaSN, 1, 0.0, 0.0},
  {"Yield", "Chemical yield fraction", DOUBLE, &SageConfig.Yield, 1, 0.0, 1.0},
  {"FracZleaveDisk", "Fraction of metals directly ejected", DOUBLE, &SageConfig.FracZleaveDisk, 1, 0.0, 1.0},
  {"SfrEfficiency", "Star formation efficiency", DOUBLE, &SageConfig.SfrEfficiency, 1, 0.0, 0.0},
  {"FeedbackReheatingEpsilon", "SN feedback reheating efficiency", DOUBLE, &SageConfig.FeedbackReheatingEpsilon, 1, 0.0, 0.0},
  {"FeedbackEjectionEfficiency", "SN feedback ejection efficiency", DOUBLE, &SageConfig.FeedbackEjectionEfficiency, 1, 0.0, 0.0},
  {"BlackHoleGrowthRate", "Black hole growth rate during mergers", DOUBLE, &SageConfig.BlackHoleGrowthRate, 1, 0.0, 0.0},
  {"RadioModeEfficiency", "Radio mode AGN efficiency", DOUBLE, &SageConfig.RadioModeEfficiency, 1, 0.0, 0.0},
  {"QuasarModeEfficiency", "Quasar mode AGN efficiency", DOUBLE, &SageConfig.QuasarModeEfficiency, 1, 0.0, 0.0},
  {"Reionization_z0", "Reionization redshift parameter z0", DOUBLE, &SageConfig.Reionization_z0, 1, 0.0, 0.0},
  {"Reionization_zr", "Reionization redshift parameter zr", DOUBLE, &SageConfig.Reionization_zr, 1, 0.0, 0.0},
};

static int NParameters = sizeof(ParameterTable) / sizeof(ParameterDefinition);

// Return the parameter table
ParameterDefinition* get_parameter_table(void)
{
  return ParameterTable;
}

// Return the number of parameters in the table
int get_parameter_table_size(void)
{
  return NParameters;
}

// Validate a parameter value
int is_parameter_valid(ParameterDefinition *param, void *value)
{
  if (param->type == INT) {
    int val = *((int*)value);
    // Check min value if specified
    if (param->min_value != 0.0 && val < param->min_value) {
      return 0;
    }
    // Check max value if specified
    if (param->max_value != 0.0 && val > param->max_value) {
      return 0;
    }
  } else if (param->type == DOUBLE) {
    double val = *((double*)value);
    // Check min value if specified
    if (param->min_value != 0.0 && val < param->min_value) {
      return 0;
    }
    // Check max value if specified
    if (param->max_value != 0.0 && val > param->max_value) {
      return 0;
    }
  }
  // String type doesn't need validation here
  return 1;
}

// Get string representation of parameter type
const char* get_parameter_type_string(int type)
{
  switch (type) {
    case INT:
      return "INT";
    case DOUBLE:
      return "DOUBLE";
    case STRING:
      return "STRING";
    default:
      return "UNKNOWN";
  }
}
