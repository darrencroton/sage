/**
 * Debug utility to inspect SAGE binary output files
 * Compile with: gcc -o debug_output debug_output.c
 * Run with: ./debug_output <path_to_model_file>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Match the output structure from the C code
typedef struct {
  int SnapNum;
  int Type;
  int64_t GalaxyIndex;
  int64_t CentralGalaxyIndex;
  int SAGEHaloIndex;
  int SAGETreeIndex;
  int64_t SimulationHaloIndex;
  int mergeType;
  int mergeIntoID;
  int mergeIntoSnapNum;
  float dT;
  float Pos[3];
  float Vel[3];
  float Spin[3];
  int Len;
  float Mvir;
  float CentralMvir;
  float Rvir;
  float Vvir;
  float Vmax;
  float VelDisp;
  float ColdGas;
  float StellarMass;
  float BulgeMass;
  float HotGas;
  float EjectedMass;
  float BlackHoleMass;
  float ICS;  // IntraClusterStars in Python
  float MetalsColdGas;
  float MetalsStellarMass;
  float MetalsBulgeMass;
  float MetalsHotGas;
  float MetalsEjectedMass;
  float MetalsICS;  // MetalsIntraClusterStars in Python
  float SfrDisk;
  float SfrBulge;
  float SfrDiskZ;
  float SfrBulgeZ;
  float DiskScaleRadius;
  float Cooling;
  float Heating;
  float QuasarModeBHaccretionMass;
  float TimeOfLastMajorMerger;
  float TimeOfLastMinorMerger;
  float OutflowRate;
  float infallMvir;
  float infallVvir;
  float infallVmax;
} GALAXY_OUTPUT;

int main(int argc, char **argv) {
  FILE *fp;
  GALAXY_OUTPUT gal;
  int ntrees, ngals, *ngals_per_tree;
  int i, j, count = 0, non_zero_stellar = 0;
  
  if (argc != 2) {
    printf("Usage: %s <path_to_model_file>\n", argv[0]);
    return 1;
  }
  
  fp = fopen(argv[1], "rb");
  if (!fp) {
    printf("Error: Cannot open file %s\n", argv[1]);
    return 1;
  }
  
  // Read header data
  fread(&ntrees, sizeof(int), 1, fp);
  fread(&ngals, sizeof(int), 1, fp);
  
  printf("File contains %d trees and %d galaxies\n", ntrees, ngals);
  
  // Read number of galaxies per tree
  ngals_per_tree = (int*)malloc(ntrees * sizeof(int));
  fread(ngals_per_tree, sizeof(int), ntrees, fp);
  
  // Print first 5 trees' galaxy counts
  printf("First 5 trees galaxy counts: ");
  for (i = 0; i < (ntrees < 5 ? ntrees : 5); i++) {
    printf("%d ", ngals_per_tree[i]);
  }
  printf("\n");
  
  // Read galaxies
  for (i = 0; i < ngals; i++) {
    size_t bytes_read = fread(&gal, sizeof(GALAXY_OUTPUT), 1, fp);
    if (bytes_read != 1) {
      printf("Error reading galaxy %d\n", i);
      break;
    }
    
    count++;
    
    // Count galaxies with non-zero stellar mass
    if (gal.StellarMass > 0.0) {
      non_zero_stellar++;
      
      // Print details of first 5 galaxies with non-zero stellar mass
      if (non_zero_stellar <= 5) {
        printf("Galaxy %d:\n", i);
        printf("  Type: %d\n", gal.Type);
        printf("  StellarMass: %e\n", gal.StellarMass);
        printf("  ColdGas: %e\n", gal.ColdGas);
        printf("  HotGas: %e\n", gal.HotGas);
        printf("  BlackHoleMass: %e\n", gal.BlackHoleMass);
        printf("  ICS: %e\n", gal.ICS);
        printf("\n");
      }
    }
  }
  
  printf("Read %d galaxies, %d with non-zero stellar mass\n", count, non_zero_stellar);
  
  free(ngals_per_tree);
  fclose(fp);
  
  return 0;
}
