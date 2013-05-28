
#ifdef MILLENNIUM
// millennium halo input structure

#define  MAXSNAPS  64
struct halo_data
{
  // merger tree pointers 
  int Descendant;
  int FirstProgenitor;
  int NextProgenitor;
  int FirstHaloInFOFgroup;
  int NextHaloInFOFgroup;

  // properties of halo 
  int Len;
  float M_Mean200, Mvir, M_TopHat;  // Mean 200 values (Mvir=M_Crit200)
  float Pos[3];
  float Vel[3];
  float VelDisp;
  float Vmax;
  float Spin[3];
  long long MostBoundID;

  // original position in subfind output 
  int SnapNum;
  int FileNr;
  int SubhaloIndex;
  float SubHalfMass;
}
*Halo;

#endif

#ifdef BOLSHOI
// bolshoi halo input structure (same as millennium)

#define  MAXSNAPS  181
struct halo_data
{
  // merger tree pointers 
  int Descendant;
  int FirstProgenitor;
  int NextProgenitor;
  int FirstHaloInFOFgroup;
  int NextHaloInFOFgroup;

  // properties of halo 
  int Len;
  float M_Mean200, Mvir, M_TopHat;
  float Pos[3];
  float Vel[3];
  float VelDisp;
  float Vmax;
  float Spin[3];
  long long MostBoundID;

  // original position in subfind output 
  int SnapNum;
  int FileNr;
  int SubhaloIndex;
  float SubHalfMass;
}
*Halo;

#endif

#ifdef GIGGLEZ
// gigglez halo input structure

//#define  MAXSNAPS  117    //GiggleZ_NR and GiggleZ_MR
#define  MAXSNAPS  61   //GiggleZ_HR
struct halo_data
{
  // merger tree pointers
  int Descendant;
  int FirstProgenitor;
  int NextProgenitor;
  int FirstHaloInFOFgroup;
  int NextHaloInFOFgroup;

  // properties of halo
  int       Len;
  float     Mvir;  // Be careful of definition here!
  float     Rvir;

  float     Pos[3];
  float     Vel[3];
  float     VelDisp;
  float     Vmax;
  float     Spin[3];
  long long MostBoundID;

  // original position in halo-finder output
  int   SnapNum;
  int   SubHaloIndex;
  int   halo_id;
  int   group_id;
}
*Halo;

#endif


