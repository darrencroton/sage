
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
  float M_200b, M_BN98, M_200c;
  float Pos[3];
  float Vel[3];
  float VelDisp;
  float Vmax;
  float Spin[3];
  long long MostBoundID;

  // original position in simulation tree files
  int SnapNum;
  int FileNr;
  int SubhaloIndex;
  float SubHalfMass;
}
*Halo;



