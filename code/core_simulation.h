
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
  float M_Mean200, Mvir, M_TopHat;  // for Millennium, Mvir=M_Crit200
  float Pos[3];
  float Vel[3];
  float VelDisp;
  float Vmax;
  float Spin[3];
  union{
      long long MostBoundID;//for LHaloTrees, this is the ID for the most bound particle
      long long SimulationHaloID;//for other mergertree codes, this contains an unique haloid. 
  };

  // original position in simulation tree files
  int SnapNum;
  int FileNr;
  int SubhaloIndex;
  float SubHalfMass;
}
*Halo;



