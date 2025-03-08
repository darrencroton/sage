#ifndef ACCESSORS_H
#define ACCESSORS_H

#include "types.h"
#include "error_handling.h"

/*
 * Accessor functions for SimulationState
 * 
 * These functions provide controlled access to the SimulationState structure,
 * ensuring proper validation and synchronization with global variables.
 */

/* Galaxy counter - ensures unique IDs for galaxies */
int get_galaxy_counter(void);
void set_galaxy_counter(int value);
void increment_galaxy_counter(void);

/* Current tree and file being processed */
int get_current_tree_id(void);
void set_current_tree_id(int tree_id);
int get_current_file_num(void);
void set_current_file_num(int file_num);

/* Galaxy count management */
int get_num_galaxies(void);
void set_num_galaxies(int count);
void increment_num_galaxies(int delta);
int get_max_galaxies(void);
void set_max_galaxies(int count);

/* Halo count management */
int get_total_halos(void);
void set_total_halos(int count);

/* Tree counts and statistics */
int get_ntrees(void);
void set_ntrees(int count);

/* Access to output snapshot list */
int* get_output_snap_list(void);
int get_num_output_snaps(void);

/* SimulationState structure manipulation */
struct SimulationState* get_simulation_state(void);

/*
 * Accessor functions for SageConfig
 * 
 * These functions provide controlled access to the SageConfig structure,
 * ensuring proper validation and synchronization with global variables.
 */

/* Physical constants from config */
double get_hubble_h(void);
double get_omega(void);
double get_omega_lambda(void);
double get_baryon_fraction(void);
double get_box_size(void);

/* Unit conversions */
double get_unit_length_in_cm(void);
double get_unit_time_in_s(void);
double get_unit_mass_in_g(void);
double get_unit_velocity_in_cm_per_s(void);

/* Recipe parameters */
double get_sn_energy(void);
double get_sn_energy_code_units(void);
double get_sn_mass_fraction(void);
double get_sn_mass_fraction_code_units(void);
double get_recycle_fraction(void);
double get_sfr_efficiency(void);
double get_feedback_reheat_efficiency(void);
double get_feedback_ejection_efficiency(void);
int get_reionization_on(void);
double get_reionization_z0(void);
double get_reionization_zr(void);
int get_disk_instability_on(void);
int get_agn_recipe_on(void);
double get_radio_mode_efficiency(void);
double get_quasar_mode_efficiency(void);
double get_black_hole_growth_rate(void);
double get_threshold_major_merger(void);

/* File paths */
const char* get_output_directory(void);
const char* get_simulation_directory(void);
const char* get_tree_name(void);
const char* get_galaxies_filename(void);

/* SageConfig structure manipulation */
struct SageConfig* get_sage_config(void);

#endif /* #ifndef ACCESSORS_H */
