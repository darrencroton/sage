#include "core_allvars.h"


/* functions in core_build_model.c */
void construct_galaxies(const int halonr, int *numgals, int *galaxycounter, int *maxgals, struct halo_data *halos,
                       struct halo_aux_data *haloaux, struct GALAXY **ptr_to_galaxies, struct GALAXY **ptr_to_halogal);
void evolve_galaxies(const int halonr, const int ngal, int *numgals, int *maxgals, struct halo_data *halos,
                     struct halo_aux_data *haloaux, struct GALAXY **ptr_to_galaxies, struct GALAXY **ptr_to_halogal);
int join_galaxies_of_progenitors(const int halonr, const int ngalstart, int *galaxycounter, int *maxgals, struct halo_data *halos,
                                 struct halo_aux_data *haloaux, struct GALAXY **ptr_to_galaxies, struct GALAXY **ptr_to_halogal);



/* functions in core_init.c */
void init(void);
   

/* core_io_tree.c */
void load_tree_table(const int filenr, const enum Valid_TreeTypes my_TreeType, int *ntrees, int **treenhalos, int **treefirsthalo, int **treengals, int *totgalaxies);
int load_tree(const int treenr, const int nhalos, enum Valid_TreeTypes my_TreeType, struct halo_data **halos,
               struct halo_aux_data **haloaux, struct GALAXY **galaxies, struct GALAXY **halogal);
void free_tree_table(const enum Valid_TreeTypes my_TreeType, int **treengals, int *treenhalos, int *treefirsthalo);
void free_galaxies_and_tree(struct GALAXY *galaxies, struct GALAXY *halogal, struct halo_aux_data *haloaux, struct halo_data *halos);

/* utility functions in core_io_tree.c -> perhaps should be moved to core_utils.c */
size_t myfread(void *ptr, const size_t size, const size_t nmemb, FILE * stream);
size_t myfwrite(const void *ptr, const size_t size, const size_t nmemb, FILE * stream);
int myfseek(FILE * stream, const long offset, const int whence);


/* Functions in core_save.c */
void save_galaxies(const int filenr, const int tree, const int ntrees, const int numgals, struct halo_data *halos,
                   struct halo_aux_data *haloaux, struct GALAXY *halogal, int **treengals, int *totgalaxies);
void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o, struct halo_data *halos,
                               struct halo_aux_data *haloaux, struct GALAXY *halogal);
void finalize_galaxy_file(const int ntrees, const int *totgalaxies, const int **treengals);



/* functions in core_read_parameter_file.c */
void read_parameter_file(const char *fname);


/* functions in core_mymalloc.c */
void *mymalloc(size_t n);
void *myrealloc(void *p, size_t n);
void myfree(void *p);
void print_allocated(void);



/* main */
void myexit(int signum);



/* functions in model_starformation_and_feedback.c */
void starformation_and_feedback(const int p, const int centralgal, const double time, const double dt, const int halonr, const int step, struct GALAXY *galaxies);
void update_from_star_formation(const int p, const double stars, const double metallicity, struct GALAXY *galaxies);
void update_from_feedback(const int p, const int centralgal, const double reheated_mass, double ejected_mass, const double metallicity, struct GALAXY *galaxies);

/* functions in model_reincorporation.c */
void reincorporate_gas(const int centralgal, const double dt, struct GALAXY *galaxies);

/* functions in model_disk_instability.c */
void check_disk_instability(const int p, const int centralgal, const int halonr, const double time, const double dt, const int step, struct GALAXY *galaxies);


/* functions in model_infall.c */
double infall_recipe(const int centralgal, const int ngal, const double Zcurr, struct GALAXY *galaxies);
void strip_from_satellite(const int centralgal, const int gal, const double Zcurr, struct GALAXY *galaxies);
double do_reionization(const int gal, const double Zcurr, struct GALAXY *galaxies);
void add_infall_to_hot(const int gal, double infallingGas, struct GALAXY *galaxies);


/* functions in model_cooling_heating.c */
double cooling_recipe(const int gal, const double dt, struct GALAXY *galaxies);
double do_AGN_heating(double coolingGas, const int centralgal, const double dt, const double x, const double rcool, struct GALAXY *galaxies);
void cool_gas_onto_galaxy(const int centralgal, const double coolingGas, struct GALAXY *galaxies);
    
/* functions in model_mergers.c*/
void disrupt_satellite_to_ICS(const int centralgal, const int gal, struct GALAXY *galaxies);
double estimate_merging_time(const int sat_halo, const int mother_halo, const int ngal, struct halo_data *halos, struct GALAXY *galaxies);
void deal_with_galaxy_merger(const int p, int merger_centralgal, const int centralgal, const double time, const double dt, const int halonr, const int step, struct GALAXY *galaxies);
void quasar_mode_wind(const int gal, const double BHaccrete, struct GALAXY *galaxies);
void add_galaxies_together(const int t, const int p, struct GALAXY *galaxies);
void make_bulge_from_burst(const int p, struct GALAXY *galaxies);
void grow_black_hole(const int merger_centralgal, const double mass_ratio, struct GALAXY *galaxies);
void collisional_starburst_recipe(const double mass_ratio, const int merger_centralgal, const int centralgal, const double time,
                                  const double dt, const int halonr, const int mode, const int step, struct GALAXY *galaxies);


/* functions in model_misc.c */
void init_galaxy(const int p, const int halonr, int *galaxycounter, struct halo_data *halos, struct GALAXY *galaxies);
double get_metallicity(const double gas, const double metals);
double get_virial_velocity(const int halonr, struct halo_data *halos);
double get_virial_radius(const int halonr, struct halo_data *halos);
double get_virial_mass(const int halonr, struct halo_data *halos);
double get_disk_radius(const int halonr, const int p, struct halo_data *halos, struct GALAXY *galaxies);
double dmax(const double x, const double y);


/* functions in core_cool_func.c */
void read_cooling_functions(void);
double get_metaldependent_cooling_rate(const double logTemp, double logZ);


