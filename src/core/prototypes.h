#ifndef CORE_PROTO_H
#define CORE_PROTO_H

#include "config.h"
#include "globals.h"
#include "types.h"
#include "memory.h"

size_t myfread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t myfwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
int myfseek(FILE *stream, long offset, int whence);

void construct_galaxies(int halonr, int tree);
void evolve_galaxies(int halonr, int ngal, int tree);
void apply_physical_processes(int ngal, int centralgal, int halonr,
                              double infallingGas, int step);
void handle_mergers(int ngal, int centralgal, int halonr, int step);
void update_galaxy_properties(int ngal, int centralgal, double deltaT);
int join_galaxies_of_progenitors(int halonr, int nstart);
int find_most_massive_progenitor(int halonr);
int copy_galaxies_from_progenitors(int halonr, int nstart, int first_occupied);
void set_galaxy_centrals(int ngalstart, int ngal);
void init(void);
void set_units(void);

void load_tree_table(int filenr, enum Valid_TreeTypes TreeType);
void load_tree(int filenr, int treenr, enum Valid_TreeTypes TreeType);
void save_galaxies(int filenr, int tree);

void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g,
                               struct GALAXY_OUTPUT *o);

void free_galaxies_and_tree(void);
void free_tree_table(enum Valid_TreeTypes TreeType);
void print_allocated(void);

void read_parameter_file(char *fname);
void init_memory_system(unsigned long max_blocks);
void *mymalloc(size_t n);
void *mymalloc_cat(size_t size, MemoryCategory category);
void *mycalloc(size_t nmemb, size_t size);
void *mycalloc_cat(size_t nmemb, size_t size, MemoryCategory category);
void *myrealloc(void *p, size_t n);
void *myrealloc_cat(void *p, size_t size, MemoryCategory category);
void myfree(void *p);
void set_memory_reporting(int level);
void print_allocated(void);
void print_allocated_by_category(void);
void print_memory_brief(void);
void check_memory_leaks(void);
int validate_memory_block(void *ptr);
int validate_all_memory(void);
void cleanup_memory_system(void);
void myexit(int signum);

void finalize_galaxy_file(int filenr);

void starformation_and_feedback(int p, int centralgal, double time, double dt,
                                int halonr, int step);
void add_galaxies_together(int t, int p);
void init_galaxy(int p, int halonr);
double infall_recipe(int centralgal, int ngal, double Zcurr);
void add_infall_to_hot(int centralgal, double infallingGas);
double cooling_recipe(int centralgal, double dt);
void cool_gas_onto_galaxy(int centralgal, double coolingGas);
void reincorporate_gas(int centralgal, double dt);
double estimate_merging_time(int prog, int mother_halo, int ngal);
void deal_with_galaxy_merger(int p, int merger_centralgal, int centralgal,
                             double time, double dt, int halonr, int step);
double dmax(double x, double y);
double do_reionization(int centralgal, double Zcurr);
double do_AGN_heating(double coolingGas, int centralgal, double dt, double x,
                      double rcool);
void collisional_starburst_recipe(double mass_ratio, int merger_centralgal,
                                  int centralgal, double time, double dt,
                                  int halonr, int mode, int step);
void update_from_star_formation(int p, double stars, double metallicity);
void update_from_feedback(int p, int centralgal, double reheated_mass,
                          double ejected_mass, double metallicity);
void make_bulge_from_burst(int p);
void grow_black_hole(int merger_centralgal, double mass_ratio);
void check_disk_instability(int p, int centralgal, int halonr, double time,
                            double dt, int step);

void strip_from_satellite(int halonr, int centralgal, int gal);
void disrupt_satellite_to_ICS(int centralgal, int gal);
void quasar_mode_wind(int gal, float BHaccrete);

double get_metallicity(double gas, double metals);
double get_virial_velocity(int halonr);
double get_virial_radius(int halonr);
double get_virial_mass(int halonr);
double get_disk_radius(int halonr, int p);

void read_output_snaps(void);
void read_snap_list(void);
void read_cooling_functions(void);
double get_metaldependent_cooling_rate(double logTemp, double logZ);
double get_rate(int tab, double logTemp);

double time_to_present(double z);
double integrand_time_to_present(double a, void *param);

double metallicity_dependent_star_formation(int p);
double Z_dependent_SF(float lower_limit, float upper_limit, float Sigma_c0,
                      float Xi, float gamma);
double integrand_Z_dependent_SF(double q, void *p);

/* Simulation state functions */
void initialize_sim_state(void);
void sync_sim_state_to_globals(void);
void sync_globals_to_sim_state(void);

#endif /* #ifndef CORE_PROTO_H */
