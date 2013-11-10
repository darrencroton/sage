
#ifdef MILLENNIUM
#define  MAXSNAPS  64
#endif

#ifdef BOLSHOI
#define  MAXSNAPS  181
#endif

#ifdef GIGGLEZ
#define  MAXSNAPS  117    //GiggleZ_MR
#endif


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



