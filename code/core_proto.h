#ifndef CORE_PROTO_H
#define CORE_PROTO_H

#include "config.h"
#include "globals.h"
#include "types.h"
#include "util_memory.h"

size_t myfread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t myfwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
int myfseek(FILE *stream, long offset, int whence);

void build_halo_tree(int halonr, int tree);
void process_halo_evolution(int halonr, int ngal, int tree);
void update_halo_properties(int ngal, int centralgal, double deltaT);
int join_progenitor_halos(int halonr, int nstart);
int find_most_massive_progenitor(int halonr);
int copy_progenitor_halos(int halonr, int nstart, int first_occupied);
void set_halo_centrals(int ngalstart, int ngal);
void init(void);
void set_units(void);

void load_tree_table(int filenr, enum Valid_TreeTypes TreeType);
void load_tree(int filenr, int treenr, enum Valid_TreeTypes TreeType);
void save_halos(int filenr, int tree);

void prepare_halo_for_output(int filenr, int tree, struct GALAXY *g,
                             struct GALAXY_OUTPUT *o);

void free_halos_and_tree(void);
void free_tree_table(enum Valid_TreeTypes TreeType);
void print_allocated(void);

void read_parameter_file(char *fname);
void init_memory_system(unsigned long max_blocks);
void *mymalloc(size_t n);
void *mymalloc_cat(size_t size, MemoryCategory category);
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

void finalize_halo_file(int filenr);

void init_halo(int p, int halonr);

double get_virial_velocity(int halonr);
double get_virial_radius(int halonr);
double get_virial_mass(int halonr);

void read_output_snaps(void);
void read_snap_list(void);

double time_to_present(double z);
double integrand_time_to_present(double a, void *param);

/* Simulation state functions */
void initialize_sim_state(void);
void sync_sim_state_to_globals(void);
void sync_globals_to_sim_state(void);

#endif /* #ifndef CORE_PROTO_H */
