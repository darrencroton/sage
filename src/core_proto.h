#include "core_allvars.h"


size_t myfread(void  *ptr,  size_t  size,  size_t  nmemb,  FILE *stream);
size_t myfwrite(void  *ptr,  size_t  size,  size_t  nmemb,  FILE *stream);
int myfseek(FILE *stream, long offset, int whence);

#ifdef OLD_VERSION
void construct_galaxies(int halonr, int tree);
#else
void construct_galaxies(const int halonr, struct halo_data *halos,
                        struct halo_aux_data *haloaux, struct GALAXY *galaxies, struct GALAXY *halogal);
#endif

#ifdef OLD_VERSION
void evolve_galaxies(int halonr, int ngal);
#else
void evolve_galaxies(const int halonr, const int ngal, struct halo_data *halos,
                     struct halo_aux_data *haloaux, struct GALAXY *galaxies, struct GALAXY *halogal);
#endif    


#ifdef OLD_VERSION
int join_galaxies_of_progenitors(int halonr, int ngalstart);
#else
int join_galaxies_of_progenitors(const int halonr, const int ngalstart, struct halo_data *halos,
                                 struct halo_aux_data *haloaux, struct GALAXY *galaxies, struct GALAXY *halogal);
#endif


void init(void);
void set_units(void);

void load_tree_table(int filenr, enum Valid_TreeTypes TreeType);
void load_tree(int treenr, enum Valid_TreeTypes TreeType);


/* Functions in core_save.c */
#ifdef OLD_VERSION
void save_galaxies(const int filenr, const int tree);
#else
void save_galaxies(const int filenr, const int tree, struct halo_data *halos,
                   struct halo_aux_data *haloaux, struct GALAXY *halogal);
#endif    

#ifdef OLD_VERSION
void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o);
#else
void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o, struct halo_data *halos,
                               struct halo_aux_data *haloaux, struct GALAXY *halogal);
#endif    



void free_galaxies_and_tree(void);
void free_tree_table(enum Valid_TreeTypes TreeType);
void print_allocated(void);

void read_parameter_file(char *fname);
void *mymalloc(size_t n);
void *myrealloc(void *p, size_t n);
void myfree(void *p);
void myexit(int signum);

void finalize_galaxy_file(void);

/* functions in model_starformation_and_feedback.c */
void starformation_and_feedback(const int p, const int centralgal, const double time, const double dt, const int halonr, const int step, struct GALAXY *galaxies);
void update_from_star_formation(const int p, const double stars, const double metallicity, struct GALAXY *galaxies);
void update_from_feedback(const int p, const int centralgal, const double reheated_mass, double ejected_mass, const double metallicity, struct GALAXY *galaxies);

double infall_recipe(int centralgal, int ngal, double Zcurr);
void add_infall_to_hot(int centralgal, double infallingGas);
double cooling_recipe(int centralgal, double dt);
void cool_gas_onto_galaxy(int centralgal, double coolingGas);
void reincorporate_gas(int centralgal, double dt);
double dmax(double x, double y);
double do_reionization(int centralgal, double Zcurr);
double do_AGN_heating(double coolingGas, int centralgal, double dt, double x, double rcool);


/* functions in model_disk_instability.c */
void check_disk_instability(const int p, const int centralgal, const int halonr, const double time, const double dt, const int step, struct GALAXY *galaxies);

void strip_from_satellite(int halonr, int centralgal, int gal);

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
#ifdef OLD_VERSION
void init_galaxy(const int p, const int halonr);
#else
void init_galaxy(const int p, const int halonr, struct halo_data *halos, struct GALAXY *galaxies);
#endif
double get_metallicity(const double gas, const double metals);
double get_virial_velocity(const int halonr, struct halo_data *halos);
double get_virial_radius(const int halonr, struct halo_data *halos);
double get_virial_mass(const int halonr, struct halo_data *halos);
double get_disk_radius(const int halonr, const int p, struct halo_data *halos, struct GALAXY *galaxies);

void read_output_snaps(void);
void read_snap_list(void);
void read_cooling_functions(void);
double get_metaldependent_cooling_rate(double logTemp, double logZ);
double get_rate(int tab, double logTemp);

double time_to_present(double z);
double integrand_time_to_present(double a, void *param);

double metallicity_dependent_star_formation(int p);
double Z_dependent_SF(float lower_limit, float upper_limit, float Sigma_c0, float Xi, float gamma);
double integrand_Z_dependent_SF(double q, void *p);
