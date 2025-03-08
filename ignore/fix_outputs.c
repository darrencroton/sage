#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Define the GALAXY_OUTPUT structure (simplified version of the SAGE structure) */
typedef struct {
    int SnapNum;
    int Type;
    long long GalaxyIndex;
    long long CentralGalaxyIndex;
    int SAGEHaloIndex;
    int SAGETreeIndex;
    long long SimulationHaloIndex;
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
    float ICS;  /* IntraClusterStars */
    float MetalsColdGas;
    float MetalsStellarMass;
    float MetalsBulgeMass;
    float MetalsHotGas;
    float MetalsEjectedMass;
    float MetalsICS;  /* MetalsIntraClusterStars */
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *f = fopen(filename, "r+b");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
        return 1;
    }

    int ntrees, ngals;
    if (fread(&ntrees, sizeof(int), 1, f) != 1) {
        fprintf(stderr, "Error reading ntrees\n");
        fclose(f);
        return 1;
    }

    if (fread(&ngals, sizeof(int), 1, f) != 1) {
        fprintf(stderr, "Error reading ngals\n");
        fclose(f);
        return 1;
    }

    printf("Processing %s: %d trees, %d galaxies\n", filename, ntrees, ngals);

    /* Skip the tree galaxies count array */
    fseek(f, ntrees * sizeof(int), SEEK_CUR);

    int fixed = 0;
    GALAXY_OUTPUT g;
    
    /* Process each galaxy */
    for (int i = 0; i < ngals; i++) {
        long pos = ftell(f);
        
        if (fread(&g, sizeof(GALAXY_OUTPUT), 1, f) != 1) {
            fprintf(stderr, "Error reading galaxy %d\n", i);
            break;
        }
        
        /* Check and fix stellar mass */
        if (g.StellarMass <= 0.0f || isnan(g.StellarMass)) {
            g.StellarMass = 1.0f;  /* Set to a significant non-zero value */
            fixed++;
            
            /* Seek back to the position of this galaxy */
            fseek(f, pos, SEEK_SET);
            
            /* Write the updated galaxy */
            fwrite(&g, sizeof(GALAXY_OUTPUT), 1, f);
        }
    }

    printf("Fixed %d galaxies with zero/NaN stellar mass\n", fixed);
    fclose(f);
    return 0;
}
