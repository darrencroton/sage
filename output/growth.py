#!/usr/bin/env python

import matplotlib
matplotlib.use('Agg')

import h5py as h5
import numpy as np
import pylab as plt
from random import sample, seed
from os.path import getsize as getFileSize

# ================================================================================
# Basic variables
# ================================================================================

# Set up some basic attributes of the run

whichsimulation = 0
whichimf = 1        # 0=Slapeter; 1=Chabrier

tolerance = 10.0

matplotlib.rcdefaults()
plt.rc('axes', color_cycle=[
    'k',
    'b',
    'r',
    'g',
    'm',
    '0.5',
    ], labelsize='x-large')
plt.rc('xtick', labelsize='x-large')
plt.rc('ytick', labelsize='x-large')
plt.rc('lines', linewidth='2.0')
# plt.rc('font', variant='monospace')
plt.rc('legend', numpoints=1, fontsize='x-large')
plt.rc('text', usetex=True)

OutputDir = '' # set in main below

OutputFormat = '.png'
TRANSPARENT = False

OutputList = []


class Results:

    """ The following methods of this class generate the figures and plot them.
    """

    def __init__(self):
        """Here we set up some of the variables which will be global to this
        class."""

        self.Hubble_h = 0.73

        if whichsimulation == 0:    # Mini-Millennium
          self.BoxSize = 62.5       # Mpc/h
          self.MaxTreeFiles = 8     # FilesPerSnapshot

        elif whichsimulation == 1:  # Millennium
          self.BoxSize = 500        # Mpc/h
          self.MaxTreeFiles = 512   # FilesPerSnapshot

        elif whichsimulation == 2:  # Bolshoi
          self.BoxSize = 250.0      # Mpc/h
          self.MaxTreeFiles = 12987 # FilesPerSnapshot

        elif whichsimulation == 3:  # GiggleZ MR
          self.BoxSize = 125.0      # Mpc/h
          self.MaxTreeFiles = 8 # FilesPerSnapshot

        else:
          print "Please pick a valid simulation!"
          exit(1)


        if whichsimulation == 0 or whichsimulation == 1 :
          
          self.SMFsnaps = xrange(63, 0, -1)
          self.SMFselect = [63, 37, 32, 27, 23, 20, 18, 16]

          self.redshift_file = ['_z127.000', '_z79.998', '_z50.000', '_z30.000', '_z19.916', '_z18.244', '_z16.725', '_z15.343', '_z14.086', '_z12.941', '_z11.897', '_z10.944', '_z10.073', '_z9.278', '_z8.550', '_z7.883', '_z7.272', '_z6.712', '_z6.197', '_z5.724', '_z5.289', '_z4.888', '_z4.520', '_z4.179', '_z3.866', '_z3.576', '_z3.308', '_z3.060', '_z2.831', '_z2.619', '_z2.422', '_z2.239', '_z2.070', '_z1.913', '_z1.766', '_z1.630', '_z1.504', '_z1.386', '_z1.276', '_z1.173', '_z1.078', '_z0.989', '_z0.905', '_z0.828', '_z0.755', '_z0.687', '_z0.624', '_z0.564', '_z0.509', '_z0.457', '_z0.408', '_z0.362', '_z0.320', '_z0.280', '_z0.242', '_z0.208', '_z0.175', '_z0.144', '_z0.116', '_z0.089', '_z0.064', '_z0.041', '_z0.020', '_z0.000']

          self.redshift = [127.000, 79.998, 50.000, 30.000, 19.916, 18.244, 16.725, 15.343, 14.086, 12.941, 11.897, 10.944, 10.073, 9.278, 8.550, 7.883, 7.272, 6.712, 6.197, 5.724, 5.289, 4.888, 4.520, 4.179, 3.866, 3.576, 3.308, 3.060, 2.831, 2.619, 2.422, 2.239, 2.070, 1.913, 1.766, 1.630, 1.504, 1.386, 1.276, 1.173, 1.078, 0.989, 0.905, 0.828, 0.755, 0.687, 0.624, 0.564, 0.509, 0.457, 0.408, 0.362, 0.320, 0.280, 0.242, 0.208, 0.175, 0.144, 0.116, 0.089, 0.064, 0.041, 0.020, 0.000]
          
        elif whichsimulation == 2 :
          
          self.SMFsnaps = xrange(180, 0, -1)
          self.SMFselect = [180, 56, 39, 26, 18, 12, 8, 6]
          
          self.redshift_file = ['_z14.083', '_z11.771', '_z9.384', '_z8.775', '_z8.234', '_z7.749', '_z7.313', '_z6.559', '_z6.231', '_z5.930', '_z5.653', '_z5.398', '_z5.161', '_z4.942', '_z4.737', '_z4.546', '_z4.368', '_z4.200', '_z4.043', '_z3.943', '_z3.895', '_z3.755', '_z3.623', '_z3.498', '_z3.380', '_z3.268', '_z3.060', '_z2.964', '_z2.871', '_z2.784', '_z2.700', '_z2.619', '_z2.542', '_z2.469', '_z2.398', '_z2.265', '_z2.202', '_z2.142', '_z2.084', '_z2.028', '_z1.974', '_z1.921', '_z1.871', '_z1.822', '_z1.775', '_z1.730', '_z1.686', '_z1.643', '_z1.602', '_z1.562', '_z1.523', '_z1.486', '_z1.449', '_z1.414', '_z1.379', '_z1.346', '_z1.313', '_z1.282', '_z1.251', '_z1.221', '_z1.192', '_z1.163', '_z1.135', '_z1.108', '_z1.082', '_z1.056', '_z1.007', '_z0.983', '_z0.960', '_z0.937', '_z0.915', '_z0.893', '_z0.879', '_z0.858', '_z0.837', '_z0.817', '_z0.798', '_z0.778', '_z0.760', '_z0.741', '_z0.723', '_z0.706', '_z0.688', '_z0.671', '_z0.655', '_z0.639', '_z0.623', '_z0.607', '_z0.592', '_z0.577', '_z0.562', '_z0.547', '_z0.533', '_z0.519', '_z0.505', '_z0.492', '_z0.479', '_z0.466', '_z0.453', '_z0.440', '_z0.428', '_z0.416', '_z0.404', '_z0.392', '_z0.381', '_z0.369', '_z0.358', '_z0.347', '_z0.336', '_z0.326', '_z0.315', '_z0.305', '_z0.295', '_z0.285', '_z0.275', '_z0.265', '_z0.256', '_z0.246', '_z0.237', '_z0.233', '_z0.228', '_z0.224', '_z0.219', '_z0.215', '_z0.210', '_z0.206', '_z0.201', '_z0.197', '_z0.193', '_z0.189', '_z0.184', '_z0.180', '_z0.176', '_z0.172', '_z0.168', '_z0.164', '_z0.160', '_z0.156', '_z0.152', '_z0.148', '_z0.144', '_z0.140', '_z0.136', '_z0.132', '_z0.128', '_z0.124', '_z0.121', '_z0.117', '_z0.113', '_z0.110', '_z0.106', '_z0.102', '_z0.099', '_z0.095', '_z0.091', '_z0.088', '_z0.084', '_z0.081', '_z0.077', '_z0.074', '_z0.070', '_z0.067', '_z0.060', '_z0.057', '_z0.053', '_z0.050', '_z0.044', '_z0.040', '_z0.037', '_z0.034', '_z0.031', '_z0.027', '_z0.024', '_z0.021', '_z0.018', '_z0.015', '_z0.012', '_z0.009', '_z0.006', '_z0.003', '_z0.000']

          self.redshift = [14.083, 11.771, 9.384, 8.775, 8.234, 7.749, 7.313, 6.559, 6.231, 5.930, 5.653, 5.398, 5.161, 4.942, 4.737, 4.546, 4.368, 4.200, 4.043, 3.943, 3.895, 3.755, 3.623, 3.498, 3.380, 3.268, 3.060, 2.964, 2.871, 2.784, 2.700, 2.619, 2.542, 2.469, 2.398, 2.265, 2.202, 2.142, 2.084, 2.028, 1.974, 1.921, 1.871, 1.822, 1.775, 1.730, 1.686, 1.643, 1.602, 1.562, 1.523, 1.486, 1.449, 1.414, 1.379, 1.346, 1.313, 1.282, 1.251, 1.221, 1.192, 1.163, 1.135, 1.108, 1.082, 1.056, 1.007, 0.983, 0.960, 0.937, 0.915, 0.893, 0.879, 0.858, 0.837, 0.817, 0.798, 0.778, 0.760, 0.741, 0.723, 0.706, 0.688, 0.671, 0.655, 0.639, 0.623, 0.607, 0.592, 0.577, 0.562, 0.547, 0.533, 0.519, 0.505, 0.492, 0.479, 0.466, 0.453, 0.440, 0.428, 0.416, 0.404, 0.392, 0.381, 0.369, 0.358, 0.347, 0.336, 0.326, 0.315, 0.305, 0.295, 0.285, 0.275, 0.265, 0.256, 0.246, 0.237, 0.233, 0.228, 0.224, 0.219, 0.215, 0.210, 0.206, 0.201, 0.197, 0.193, 0.189, 0.184, 0.180, 0.176, 0.172, 0.168, 0.164, 0.160, 0.156, 0.152, 0.148, 0.144, 0.140, 0.136, 0.132, 0.128, 0.124, 0.121, 0.117, 0.113, 0.110, 0.106, 0.102, 0.099, 0.095, 0.091, 0.088, 0.084, 0.081, 0.077, 0.074, 0.070, 0.067, 0.060, 0.057, 0.053, 0.050, 0.044, 0.040, 0.037, 0.034, 0.031, 0.027, 0.024, 0.021, 0.018, 0.015, 0.012, 0.009, 0.006, 0.003, 0.000]
          
        elif whichsimulation == 3 :
          
          self.SMFsnaps = xrange(116, 0, -1)
          self.SMFselect = [116, 65, 54, 44, 37, 31, 26, 22]
          
          self.redshift_file = ['_z18.244', '_z17.545', '_z16.725', '_z16.000', '_z15.343', '_z14.704', '_z14.086', '_z13.499', '_z12.941', '_z12.407', '_z11.897', '_z11.409', '_z10.944', '_z10.499', '_z10.073', '_z9.667', '_z9.278', '_z8.906', '_z8.550', '_z8.209', '_z7.883', '_z7.571', '_z7.272', '_z6.986', '_z6.712', '_z6.449', '_z6.197', '_z5.955', '_z5.724', '_z5.502', '_z5.289', '_z5.085', '_z4.888', '_z4.700', '_z4.520', '_z4.346', '_z4.179', '_z4.019', '_z3.866', '_z3.718', '_z3.576', '_z3.439', '_z3.308', '_z3.182', '_z3.060', '_z2.944', '_z2.831', '_z2.519', '_z2.422', '_z2.723', '_z2.619', '_z2.329', '_z2.239', '_z2.153', '_z2.070', '_z1.990', '_z1.913', '_z1.838', '_z1.766', '_z1.697', '_z1.630', '_z1.566', '_z1.504', '_z1.444', '_z1.386', '_z1.330', '_z1.276', '_z1.224', '_z1.173', '_z1.125', '_z1.078', '_z1.033', '_z0.989', '_z0.946', '_z0.905', '_z0.866', '_z0.828', '_z0.791', '_z0.755', '_z0.721', '_z0.687', '_z0.655', '_z0.624', '_z0.593', '_z0.564', '_z0.536', '_z0.509', '_z0.482', '_z0.457', '_z0.432', '_z0.408', '_z0.385', '_z0.362', '_z0.341', '_z0.320', '_z0.299', '_z0.280', '_z0.261', '_z0.242', '_z0.225', '_z0.208', '_z0.191', '_z0.175', '_z0.159', '_z0.144', '_z0.130', '_z0.116', '_z0.102', '_z0.089', '_z0.077', '_z0.064', '_z0.053', '_z0.041', '_z0.030', '_z0.020', '_z0.010', '_z0.000']

          self.redshift = [18.244, 17.545, 16.725, 16.000, 15.343, 14.704, 14.086, 13.499, 12.941, 12.407, 11.897, 11.409, 10.944, 10.499, 10.073, 9.667, 9.278, 8.906, 8.550, 8.209, 7.883, 7.571, 7.272, 6.986, 6.712, 6.449, 6.197, 5.955, 5.724, 5.502, 5.289, 5.085, 4.888, 4.700, 4.520, 4.346, 4.179, 4.019, 3.866, 3.718, 3.576, 3.439, 3.308, 3.182, 3.060, 2.944, 2.831, 2.519, 2.422, 2.723, 2.619, 2.329, 2.239, 2.153, 2.070, 1.990, 1.913, 1.838, 1.766, 1.697, 1.630, 1.566, 1.504, 1.444, 1.386, 1.330, 1.276, 1.224, 1.173, 1.125, 1.078, 1.033, 0.989, 0.946, 0.905, 0.866, 0.828, 0.791, 0.755, 0.721, 0.687, 0.655, 0.624, 0.593, 0.564, 0.536, 0.509, 0.482, 0.457, 0.432, 0.408, 0.385, 0.362, 0.341, 0.320, 0.299, 0.280, 0.261, 0.242, 0.225, 0.208, 0.191, 0.175, 0.159, 0.144, 0.130, 0.116, 0.102, 0.089, 0.077, 0.064, 0.053, 0.041, 0.030, 0.020, 0.010, 0.000]



    def read_gals(self, model_name, first_file, last_file, thissnap):

        # The input galaxy structure:
        Galdesc_full = [
            ('Type'                         , np.int32),                    
            ('GalaxyIndex'                  , np.int64),                    
            ('HaloIndex'                    , np.int32),                    
            ('FOFHaloIdx'                   , np.int32),                    
            ('TreeIdx'                      , np.int32),                    
            ('SnapNum'                      , np.int32),                    
            ('CentralGal'                   , np.int32),                    
            ('CentralMvir'                  , np.float32),                  
            ('mergeType'                    , np.int32),                    
            ('mergeIntoID'                  , np.int32),                    
            ('mergeIntoSnapNum'             , np.int32),                    
            ('dT'                           , np.float32),                    
            ('Pos'                          , (np.float32, 3)),             
            ('Vel'                          , (np.float32, 3)),             
            ('Spin'                         , (np.float32, 3)),             
            ('Len'                          , np.int32),                    
            ('Mvir'                         , np.float32),                  
            ('Rvir'                         , np.float32),                  
            ('Vvir'                         , np.float32),                  
            ('Vmax'                         , np.float32),                  
            ('VelDisp'                      , np.float32),                  
            ('ColdGas'                      , np.float32),                  
            ('StellarMass'                  , np.float32),                  
            ('ClassicalBulgeMass'           , np.float32),                  
            ('SecularBulgeMass'             , np.float32),                  
            ('HotGas'                       , np.float32),                  
            ('EjectedMass'                  , np.float32),                  
            ('BlackHoleMass'                , np.float32),                  
            ('IntraClusterStars'            , np.float32),                  
            ('MetalsColdGas'                , np.float32),                  
            ('MetalsStellarMass'            , np.float32),                  
            ('ClassicalMetalsBulgeMass'     , np.float32),                  
            ('SecularMetalsBulgeMass'       , np.float32),                  
            ('MetalsHotGas'                 , np.float32),                  
            ('MetalsEjectedMass'            , np.float32),                  
            ('MetalsIntraClusterStars'      , np.float32),                  
            ('SfrDisk'                      , np.float32),                  
            ('SfrBulge'                     , np.float32),                  
            ('SfrDiskZ'                     , np.float32),                  
            ('SfrBulgeZ'                    , np.float32),                  
            ('DiskRadius'                   , np.float32),                  
            ('BulgeRadius'                  , np.float32),                  
            ('Cooling'                      , np.float32),                  
            ('Heating'                      , np.float32),
            ('LastMajorMerger'              , np.float32),
            ('OutflowRate'                  , np.float32),
            ('infallMvir'                   , np.float32),
            ('infallVvir'                   , np.float32),
            ('infallVmax'                   , np.float32)
            ]
        names = [Galdesc_full[i][0] for i in xrange(len(Galdesc_full))]
        formats = [Galdesc_full[i][1] for i in xrange(len(Galdesc_full))]
        Galdesc = np.dtype({'names':names, 'formats':formats}, align=True)

        # The internal galaxy structure:
        Galdesc_internal_full = [
            ('Type'                         , np.int32),                    
            ('GalaxyIndex'                  , np.int64),                    
            ('SnapNum'                      , np.int32),                    
            ('CentralGal'                   , np.int32),                    
            ('dT'                           , np.int32),                  
            ('CentralMvir'                  , np.float32),                  
            ('deltaCentralMvir'             , np.float32),                  
            ('Mvir'                         , np.float32),                  
            ('deltaMvir'                    , np.float32),                  
            ('dMvirdT'                      , np.float32),                  
            ('ColdGas'                      , np.float32),                  
            ('deltaColdGas'                 , np.float32),                  
            ('StellarMass'                  , np.float32),                  
            ('deltaMstars'                  , np.float32),                  
            ('dMstarsdT'                    , np.float32),                  
            ('HotGas'                       , np.float32),                  
            ('deltaHotGas'                  , np.float32),                  
            ('BlackHoleMass'                , np.float32),                  
            ('deltaBlackHoleMass'           , np.float32)
            ]
        names_internal = [Galdesc_internal_full[i][0] for i in xrange(len(Galdesc_internal_full))]
        formats_internal = [Galdesc_internal_full[i][1] for i in xrange(len(Galdesc_internal_full))]
        Galdesc_internal = np.dtype({'names':names_internal, 'formats':formats_internal}, align=True)


        # Initialize variables.
        TotNTrees = 0
        TotNGals = 0
        FileIndexRanges = []
        goodfiles = 0
            
        if thissnap in self.SMFsnaps:

            print
            print "Determining array storage requirements."
        
            # Read each file and determine the total number of galaxies to be read in
            for fnr in xrange(first_file,last_file+1):
                fname = model_name+'_'+str(fnr)  # Complete filename
                
                if not os.path.isfile(fname):
                    # print "File\t%s  \tdoes not exist!  Skipping..." % (fname)
                    continue
                
                if getFileSize(fname) == 0:
                    print "File\t%s  \tis empty!  Skipping..." % (fname)
                    continue
                
                fin = open(fname, 'rb')  # Open the file
                Ntrees = np.fromfile(fin,np.dtype(np.int32),1)  # Read number of trees in file
                NtotGals = np.fromfile(fin,np.dtype(np.int32),1)[0]  # Read number of gals in file.
                TotNTrees = TotNTrees + Ntrees  # Update total sim trees number
                TotNGals = TotNGals + NtotGals  # Update total sim gals number
                goodfiles = goodfiles + 1  # Update number of files read for volume calculation
                fin.close()

            print "Input files contain:\t%d trees ;\t%d galaxies ." % (TotNTrees, TotNGals)

        # Initialize the storage array
        G = np.empty(TotNGals, dtype=Galdesc_internal)
        G = G.view(np.recarray)

        if thissnap in self.SMFsnaps:

            offset = 0  # Offset index for storage array

            # Open each file in turn and read in the preamble variables and structure.
            print "Reading in files."
            for fnr in xrange(first_file,last_file+1):
                fname = model_name+'_'+str(fnr)  # Complete filename
    
                if not os.path.isfile(fname):
                    continue
        
                if getFileSize(fname) == 0:
                    continue
        
                fin = open(fname, 'rb')  # Open the file
                Ntrees = np.fromfile(fin, np.dtype(np.int32), 1)  # Read number of trees in file
                NtotGals = np.fromfile(fin, np.dtype(np.int32), 1)[0]  # Read number of gals in file.
                GalsPerTree = np.fromfile(fin, np.dtype((np.int32, Ntrees)),1) # Read the number of gals in each tree
                print ":   Reading N=", NtotGals, "   \tgalaxies from file: ", fname
                GG = np.fromfile(fin, Galdesc, NtotGals)  # Read in the galaxy structures
                GG = GG.view(np.recarray)
                        
                FileIndexRanges.append((offset,offset+NtotGals))

                # Slice the file array into the global array
                # N.B. the copy() part is required otherwise we simply point to
                # the GG data which changes from file to file
                # NOTE THE WAY PYTHON WORKS WITH THESE INDICES!

                G.Type[offset:offset+NtotGals] = GG.Type[0:NtotGals].copy()
                G.GalaxyIndex[offset:offset+NtotGals] = GG.GalaxyIndex[0:NtotGals].copy()
                G.SnapNum[offset:offset+NtotGals] = GG.SnapNum[0:NtotGals].copy()
                G.CentralGal[offset:offset+NtotGals] = GG.CentralGal[0:NtotGals].copy()
                G.dT[offset:offset+NtotGals] = GG.dT[0:NtotGals].copy()
                G.CentralMvir[offset:offset+NtotGals] = GG.CentralMvir[0:NtotGals].copy()
                G.Mvir[offset:offset+NtotGals] = GG.Mvir[0:NtotGals].copy()
                G.ColdGas[offset:offset+NtotGals] = GG.ColdGas[0:NtotGals].copy()
                G.StellarMass[offset:offset+NtotGals] = GG.StellarMass[0:NtotGals].copy()
                G.HotGas[offset:offset+NtotGals] = GG.HotGas[0:NtotGals].copy()
                G.BlackHoleMass[offset:offset+NtotGals] = GG.BlackHoleMass[0:NtotGals].copy()
            
                del(GG)
                offset = offset + NtotGals  # Update the offset position for the global array
        
                fin.close()  # Close the file

            print "Total galaxies considered:", TotNGals
            print

        # Calculate the volume given the first_file and last_file
        self.volume = self.BoxSize**3.0 * goodfiles / self.MaxTreeFiles

        return G


# --------------------------------------------------------

    def StellarMassFunction(self, G_history):

        print 'Plotting the stellar mass function'

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        binwidth = 0.1  # mass function histogram bin width

        # Marchesini et al. 2009ApJ...701.1765M SMF, h=0.7
        M = np.arange(7.0, 11.8, 0.01)
        Mstar = np.log10(10.0**10.96)
        alpha = -1.18
        phistar = 30.87*1e-4
        xval = 10.0 ** (M-Mstar)
        yval = np.log(10.) * phistar * xval ** (alpha+1) * np.exp(-xval)
        if(whichimf == 0):
            plt.plot(np.log10(10.0**M *1.6), yval, ':', lw=10, alpha=0.5, label='Marchesini et al. 2009 z=[0.1]')
        elif(whichimf == 1):
            plt.plot(np.log10(10.0**M *1.6 /1.8), yval, ':', lw=10, alpha=0.5, label='Marchesini et al. 2009 z=[0.1]')
            

        M = np.arange(9.3, 11.8, 0.01)
        Mstar = np.log10(10.0**10.91)
        alpha = -0.99
        phistar = 10.17*1e-4
        xval = 10.0 ** (M-Mstar)
        yval = np.log(10.) * phistar * xval ** (alpha+1) * np.exp(-xval)
        if(whichimf == 0):
            plt.plot(np.log10(10.0**M *1.6), yval, 'b:', lw=10, alpha=0.5, label='... z=[1.3,2.0]')
        elif(whichimf == 1):
            plt.plot(np.log10(10.0**M *1.6/1.8), yval, 'b:', lw=10, alpha=0.5, label='... z=[1.3,2.0]')

        M = np.arange(9.7, 11.8, 0.01)
        Mstar = np.log10(10.0**10.96)
        alpha = -1.01
        phistar = 3.95*1e-4
        xval = 10.0 ** (M-Mstar)
        yval = np.log(10.) * phistar * xval ** (alpha+1) * np.exp(-xval)
        if(whichimf == 0):
            plt.plot(np.log10(10.0**M *1.6), yval, 'g:', lw=10, alpha=0.5, label='... z=[2.0,3.0]')
        elif(whichimf == 1):
            plt.plot(np.log10(10.0**M *1.6/1.8), yval, 'g:', lw=10, alpha=0.5, label='... z=[2.0,3.0]')

        M = np.arange(10.0, 11.8, 0.01)
        Mstar = np.log10(10.0**11.38)
        alpha = -1.39
        phistar = 0.53*1e-4
        xval = 10.0 ** (M-Mstar)
        yval = np.log(10.) * phistar * xval ** (alpha+1) * np.exp(-xval)
        if(whichimf == 0):
            plt.plot(np.log10(10.0**M *1.6), yval, 'r:', lw=10, alpha=0.5, label='... z=[3.0,4.0]')
        elif(whichimf == 1):
            plt.plot(np.log10(10.0**M *1.6/1.8), yval, 'r:', lw=10, alpha=0.5, label='... z=[3.0,4.0]')


        ###### z=0
        
        w = np.where(G_history[self.SMFselect[0]].StellarMass > 0.0)[0]
        mass = np.log10(G_history[self.SMFselect[0]].StellarMass[w] * 1.0e10 /self.Hubble_h)

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = (ma - mi) / binwidth

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # Overplot the model histograms
        plt.plot(xaxeshisto, counts / self.volume *self.Hubble_h*self.Hubble_h*self.Hubble_h / binwidth, 'k-', label='Model galaxies')

        ###### z=1.3
        
        w = np.where(G_history[self.SMFselect[1]].StellarMass > 0.0)[0]
        mass = np.log10(G_history[self.SMFselect[1]].StellarMass[w] * 1.0e10 /self.Hubble_h)

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = (ma - mi) / binwidth

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # Overplot the model histograms
        plt.plot(xaxeshisto, counts / self.volume *self.Hubble_h*self.Hubble_h*self.Hubble_h / binwidth, 'b-')

        ###### z=2
        
        w = np.where(G_history[self.SMFselect[2]].StellarMass > 0.0)[0]
        mass = np.log10(G_history[self.SMFselect[2]].StellarMass[w] * 1.0e10 /self.Hubble_h)

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = (ma - mi) / binwidth

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # Overplot the model histograms
        plt.plot(xaxeshisto, counts / self.volume *self.Hubble_h*self.Hubble_h*self.Hubble_h / binwidth, 'g-')

        ###### z=3
        
        w = np.where(G_history[self.SMFselect[3]].StellarMass > 0.0)[0]
        mass = np.log10(G_history[self.SMFselect[3]].StellarMass[w] * 1.0e10 /self.Hubble_h)

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = (ma - mi) / binwidth

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # Overplot the model histograms
        plt.plot(xaxeshisto, counts / self.volume *self.Hubble_h*self.Hubble_h*self.Hubble_h / binwidth, 'r-')

        ######        

        plt.yscale('log', nonposy='clip')

        plt.axis([8.0, 12.5, 1.0e-6, 1.0e-1])

        # Set the x-axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.1))

        plt.ylabel(r'$\phi\ (\mathrm{Mpc}^{-3}\ \mathrm{dex}^{-1}$)')  # Set the y...
        plt.xlabel(r'$\log_{10} M_{\mathrm{stars}}\ (M_{\odot})$')  # and the x-axis labels

        leg = plt.legend(loc='lower left', numpoints=1,
                         labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')

        outputFile = OutputDir + 'smf_check' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)


# ---------------------------------------------------------

    def PlotHistory_deltaMvir(self, G_history):
    
        print 'Plotting halo mass growth evolution'

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure
        
        z = np.array(self.redshift)


        ##############
        # low mass bin

        print '    ... low mass'

        deltaMvir = np.zeros((LastSnap+1-FirstSnap)) 
        dMvirdT = np.zeros((LastSnap+1-FirstSnap)) 
        num = np.zeros((LastSnap+1-FirstSnap))       

        w = np.where((np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) > 11.0) & (np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) < 11.5) & (G_history[LastSnap].Type == 0))[0]
        if(len(w) > 0):
          for which in xrange(len(w)):
            targetID = G_history[LastSnap].GalaxyIndex[w[which]]
            for snap in xrange(FirstSnap,LastSnap+1):
              ww = np.where(G_history[snap].GalaxyIndex == targetID)[0]
              if(len(ww) > 1):
                print 'there is more than one progenitor for galaxy ', targetID, G_history[snap].GalaxyIndex[ww]
                exit(1)
              if(len(ww) > 0):                
                deltaMvir[snap-FirstSnap] = deltaMvir[snap-FirstSnap] + G_history[snap].deltaMvir[ww[0]]
                dMvirdT[snap-FirstSnap] = dMvirdT[snap-FirstSnap] + G_history[snap].dMvirdT[ww[0]]
                num[snap-FirstSnap] = num[snap-FirstSnap] + 1.0
    
        for snap in xrange(FirstSnap,LastSnap+1):
          if (num[snap-FirstSnap] > 0.0):
            deltaMvir[snap-FirstSnap] = deltaMvir[snap-FirstSnap] / num[snap-FirstSnap]
            dMvirdT[snap-FirstSnap] = dMvirdT[snap-FirstSnap] / num[snap-FirstSnap]

        nonzero = np.where(deltaMvir > 0.0)[0]
        # plt.plot(z[nonzero]+1.0, np.log10(deltaMvir[nonzero]*1.0e10/self.Hubble_h), 'b-', lw=3.0)
        plt.plot(z[nonzero]+1.0, np.log10(dMvirdT[nonzero]), 'b-', lw=3.0)

        print np.log10(deltaMvir*1.0e10/self.Hubble_h)
        print dMvirdT


        ##############
        # mid mass bin

        print '    ... mid mass'

        deltaMvir = np.zeros((LastSnap+1-FirstSnap)) 
        dMvirdT = np.zeros((LastSnap+1-FirstSnap)) 
        num = np.zeros((LastSnap+1-FirstSnap))       

        w = np.where((np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) > 11.5) & (np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) < 12.5) & (G_history[LastSnap].Type == 0))[0]
        if(len(w) > 0):
          for which in xrange(len(w)):
            targetID = G_history[LastSnap].GalaxyIndex[w[which]]
            for snap in xrange(FirstSnap,LastSnap+1):
              ww = np.where(G_history[snap].GalaxyIndex == targetID)[0]
              if(len(ww) > 1):
                print 'there is more than one progenitor for galaxy ', targetID, G_history[snap].GalaxyIndex[ww]
                exit(1)
              if(len(ww) > 0):
                deltaMvir[snap-FirstSnap] = deltaMvir[snap-FirstSnap] + G_history[snap].deltaMvir[ww[0]]
                dMvirdT[snap-FirstSnap] = dMvirdT[snap-FirstSnap] + G_history[snap].dMvirdT[ww[0]]
                num[snap-FirstSnap] = num[snap-FirstSnap] + 1.0
    
        for snap in xrange(FirstSnap,LastSnap+1):
          if (num[snap-FirstSnap] > 0.0):
            deltaMvir[snap-FirstSnap] = deltaMvir[snap-FirstSnap] / num[snap-FirstSnap]
            dMvirdT[snap-FirstSnap] = dMvirdT[snap-FirstSnap] / num[snap-FirstSnap]

        nonzero = np.where(deltaMvir > 0.0)[0]
        # plt.plot(z[nonzero]+1.0, np.log10(deltaMvir[nonzero]*1.0e10/self.Hubble_h), lw=3.0)
        plt.plot(z[nonzero]+1.0, np.log10(dMvirdT[nonzero]), lw=3.0)

        print np.log10(deltaMvir*1.0e10/self.Hubble_h)
        print dMvirdT



        ##############
        # high mass bin

        print '    ... high mass'

        deltaMvir = np.zeros((LastSnap+1-FirstSnap)) 
        dMvirdT = np.zeros((LastSnap+1-FirstSnap)) 
        num = np.zeros((LastSnap+1-FirstSnap))       

        w = np.where((np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) > 12.5) & (G_history[LastSnap].Type == 0))[0]
        if(len(w) > 0):
          for which in xrange(len(w)):
            targetID = G_history[LastSnap].GalaxyIndex[w[which]]
            for snap in xrange(FirstSnap,LastSnap+1):
              ww = np.where(G_history[snap].GalaxyIndex == targetID)[0]
              if(len(ww) > 1):
                print 'there is more than one progenitor for galaxy ', targetID, G_history[snap].GalaxyIndex[ww]
                exit(1)
              if(len(ww) > 0):
                deltaMvir[snap-FirstSnap] = deltaMvir[snap-FirstSnap] + G_history[snap].deltaMvir[ww[0]]
                dMvirdT[snap-FirstSnap] = dMvirdT[snap-FirstSnap] + G_history[snap].dMvirdT[ww[0]]
                num[snap-FirstSnap] = num[snap-FirstSnap] + 1.0
    
        for snap in xrange(FirstSnap,LastSnap+1):
          if (num[snap-FirstSnap] > 0.0):
            deltaMvir[snap-FirstSnap] = deltaMvir[snap-FirstSnap] / num[snap-FirstSnap]
            dMvirdT[snap-FirstSnap] = dMvirdT[snap-FirstSnap] / num[snap-FirstSnap]

        nonzero = np.where(deltaMvir > 0.0)[0]
        # plt.plot(z[nonzero]+1.0, np.log10(deltaMvir[nonzero]*1.0e10/self.Hubble_h), 'r-', lw=3.0)
        plt.plot(z[nonzero]+1.0, np.log10(dMvirdT[nonzero]), 'r-', lw=3.0)

        print np.log10(deltaMvir*1.0e10/self.Hubble_h)
        print dMvirdT


        plt.xscale('log', nonposy='clip')
   
        plt.ylabel(r'$\log_{10} \mathrm{d}M_{\mathrm{vir}} / \mathrm{dt}$')  # Set the y...
        plt.xlabel(r'$\mathrm{1+z}$')  # and the x-axis labels
    
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(1))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.5))
    
        # plt.axis([0.0, 10.0, 8.5, 12.0])
        plt.axis([0.0, 10.0, 0.0, 3.5])
    
        outputFile = OutputDir + '_A.History-deltaMvir' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()
    
        # Add this plot to our output list
        OutputList.append(outputFile)



# ---------------------------------------------------------

    def PlotHistory_deltaMstars(self, G_history):
    
        print 'Plotting stellar mass growth evolution'

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure
        
        z = np.array(self.redshift)


        ##############
        # low mass bin

        print '    ... low mass'

        deltaMstars = np.zeros((LastSnap+1-FirstSnap)) 
        dMstarsdT = np.zeros((LastSnap+1-FirstSnap)) 
        num = np.zeros((LastSnap+1-FirstSnap))       

        w = np.where((np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) > 11.0) & (np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) < 11.5) & (G_history[LastSnap].Type == 0))[0]
        if(len(w) > 0):
          for which in xrange(len(w)):
            targetID = G_history[LastSnap].GalaxyIndex[w[which]]
            for snap in xrange(FirstSnap,LastSnap+1):
              ww = np.where(G_history[snap].GalaxyIndex == targetID)[0]
              if(len(ww) > 1):
                print 'there is more than one progenitor for galaxy ', targetID, G_history[snap].GalaxyIndex[ww]
                exit(1)
              if(len(ww) > 0):                
                deltaMstars[snap-FirstSnap] = deltaMstars[snap-FirstSnap] + G_history[snap].deltaMstars[ww[0]]
                dMstarsdT[snap-FirstSnap] = dMstarsdT[snap-FirstSnap] + G_history[snap].dMstarsdT[ww[0]]
                num[snap-FirstSnap] = num[snap-FirstSnap] + 1.0
    
        for snap in xrange(FirstSnap,LastSnap+1):
          if (num[snap-FirstSnap] > 0.0):
            deltaMstars[snap-FirstSnap] = deltaMstars[snap-FirstSnap] / num[snap-FirstSnap]
            dMstarsdT[snap-FirstSnap] = dMstarsdT[snap-FirstSnap] / num[snap-FirstSnap]

        nonzero = np.where(deltaMstars > 0.0)[0]
        # plt.plot(z[nonzero]+1.0, np.log10(deltaMstars[nonzero]*1.0e10/self.Hubble_h), 'b-', lw=3.0)
        plt.plot(z[nonzero]+1.0, np.log10(dMstarsdT[nonzero]), 'b-', lw=3.0)

        print np.log10(deltaMstars*1.0e10/self.Hubble_h)
        print dMstarsdT


        ##############
        # mid mass bin

        print '    ... mid mass'

        deltaMstars = np.zeros((LastSnap+1-FirstSnap)) 
        dMstarsdT = np.zeros((LastSnap+1-FirstSnap)) 
        num = np.zeros((LastSnap+1-FirstSnap))       

        w = np.where((np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) > 11.5) & (np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) < 12.5) & (G_history[LastSnap].Type == 0))[0]
        if(len(w) > 0):
          for which in xrange(len(w)):
            targetID = G_history[LastSnap].GalaxyIndex[w[which]]
            for snap in xrange(FirstSnap,LastSnap+1):
              ww = np.where(G_history[snap].GalaxyIndex == targetID)[0]
              if(len(ww) > 1):
                print 'there is more than one progenitor for galaxy ', targetID, G_history[snap].GalaxyIndex[ww]
                exit(1)
              if(len(ww) > 0):
                deltaMstars[snap-FirstSnap] = deltaMstars[snap-FirstSnap] + G_history[snap].deltaMstars[ww[0]]
                dMstarsdT[snap-FirstSnap] = dMstarsdT[snap-FirstSnap] + G_history[snap].dMstarsdT[ww[0]]
                num[snap-FirstSnap] = num[snap-FirstSnap] + 1.0
    
        for snap in xrange(FirstSnap,LastSnap+1):
          if (num[snap-FirstSnap] > 0.0):
            deltaMstars[snap-FirstSnap] = deltaMstars[snap-FirstSnap] / num[snap-FirstSnap]
            dMstarsdT[snap-FirstSnap] = dMstarsdT[snap-FirstSnap] / num[snap-FirstSnap]

        nonzero = np.where(deltaMstars > 0.0)[0]
        # plt.plot(z[nonzero]+1.0, np.log10(deltaMstars[nonzero]*1.0e10/self.Hubble_h), lw=3.0)
        plt.plot(z[nonzero]+1.0, np.log10(dMstarsdT[nonzero]), lw=3.0)

        print np.log10(deltaMstars*1.0e10/self.Hubble_h)
        print dMstarsdT



        ##############
        # high mass bin

        print '    ... high mass'

        deltaMstars = np.zeros((LastSnap+1-FirstSnap)) 
        dMstarsdT = np.zeros((LastSnap+1-FirstSnap)) 
        num = np.zeros((LastSnap+1-FirstSnap))       

        w = np.where((np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) > 12.5) & (G_history[LastSnap].Type == 0))[0]
        if(len(w) > 0):
          for which in xrange(len(w)):
            targetID = G_history[LastSnap].GalaxyIndex[w[which]]
            for snap in xrange(FirstSnap,LastSnap+1):
              ww = np.where(G_history[snap].GalaxyIndex == targetID)[0]
              if(len(ww) > 1):
                print 'there is more than one progenitor for galaxy ', targetID, G_history[snap].GalaxyIndex[ww]
                exit(1)
              if(len(ww) > 0):
                deltaMstars[snap-FirstSnap] = deltaMstars[snap-FirstSnap] + G_history[snap].deltaMstars[ww[0]]
                dMstarsdT[snap-FirstSnap] = dMstarsdT[snap-FirstSnap] + G_history[snap].dMstarsdT[ww[0]]
                num[snap-FirstSnap] = num[snap-FirstSnap] + 1.0
    
        for snap in xrange(FirstSnap,LastSnap+1):
          if (num[snap-FirstSnap] > 0.0):
            deltaMstars[snap-FirstSnap] = deltaMstars[snap-FirstSnap] / num[snap-FirstSnap]
            dMstarsdT[snap-FirstSnap] = dMstarsdT[snap-FirstSnap] / num[snap-FirstSnap]

        nonzero = np.where(deltaMstars > 0.0)[0]
        # plt.plot(z[nonzero]+1.0, np.log10(deltaMstars[nonzero]*1.0e10/self.Hubble_h), 'r-', lw=3.0)
        plt.plot(z[nonzero]+1.0, np.log10(dMstarsdT[nonzero]), 'r-', lw=3.0)

        print np.log10(deltaMstars*1.0e10/self.Hubble_h)
        print dMstarsdT


        plt.xscale('log', nonposy='clip')
   
        plt.ylabel(r'$\log_{10} \mathrm{d}M_{\mathrm{stars}} / \mathrm{dt}$')  # Set the y...
        plt.xlabel(r'$\mathrm{1+z}$')  # and the x-axis labels
    
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(1))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.5))
    
        # plt.axis([0.0, 10.0, 8.5, 12.0])
        plt.axis([0.0, 10.0, -1.5, 2.0])
    
        outputFile = OutputDir + '_B.History-deltaMstars' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()
    
        # Add this plot to our output list
        OutputList.append(outputFile)



# ---------------------------------------------------------

    def PlotHistory_massratio(self, G_history):
    
        print 'Plotting mass ratio evolution'

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure
        
        z = np.array(self.redshift)


        ##############
        # low mass bin
        
        print '    ... low mass'

        massratio = np.zeros((LastSnap+1-FirstSnap)) 
        num = np.zeros((LastSnap+1-FirstSnap))       

        w = np.where((np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) > 11.0) & (np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) < 11.5) & (G_history[LastSnap].Type == 0))[0]
        if(len(w) > 0):
          for which in xrange(len(w)):
            targetID = G_history[LastSnap].GalaxyIndex[w[which]]
            for snap in xrange(FirstSnap,LastSnap+1):
              ww = np.where(G_history[snap].GalaxyIndex == targetID)[0]
              if(len(ww) > 1):
                print 'there is more than one progenitor for galaxy ', targetID, G_history[snap].GalaxyIndex[ww]
                exit(1)
              if(len(ww) > 0):
                massratio[snap-FirstSnap] = massratio[snap-FirstSnap] + G_history[snap].StellarMass[ww[0]] / G_history[snap].Mvir[ww[0]]
                num[snap-FirstSnap] = num[snap-FirstSnap] + 1.0
    
        for snap in xrange(FirstSnap,LastSnap+1):
          if (num[snap-FirstSnap] > 0.0):
            massratio[snap-FirstSnap] = massratio[snap-FirstSnap] / num[snap-FirstSnap]

        nonzero = np.where(massratio > 0.0)[0]
        plt.plot(z[nonzero]+1.0, np.log10(massratio[nonzero]), 'b-', lw=3.0)

        print massratio


        ##############
        # mid mass bin

        print '    ... mid mass'

        massratio = np.zeros((LastSnap+1-FirstSnap)) 
        num = np.zeros((LastSnap+1-FirstSnap))       

        w = np.where((np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) > 11.5) & (np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) < 12.5) & (G_history[LastSnap].Type == 0))[0]
        if(len(w) > 0):
          for which in xrange(len(w)):
            targetID = G_history[LastSnap].GalaxyIndex[w[which]]
            for snap in xrange(FirstSnap,LastSnap+1):
              ww = np.where(G_history[snap].GalaxyIndex == targetID)[0]
              if(len(ww) > 1):
                print 'there is more than one progenitor for galaxy ', targetID, G_history[snap].GalaxyIndex[ww]
                exit(1)
              if(len(ww) > 0):
                massratio[snap-FirstSnap] = massratio[snap-FirstSnap] + G_history[snap].StellarMass[ww[0]] / G_history[snap].Mvir[ww[0]]
                num[snap-FirstSnap] = num[snap-FirstSnap] + 1.0
    
        for snap in xrange(FirstSnap,LastSnap+1):
          if (num[snap-FirstSnap] > 0.0):
            massratio[snap-FirstSnap] = massratio[snap-FirstSnap] / num[snap-FirstSnap]

        nonzero = np.where(massratio > 0.0)[0]
        plt.plot(z[nonzero]+1.0, np.log10(massratio[nonzero]), lw=3.0)

        print massratio


        ##############
        # high mass bin

        print '    ... high mass'

        massratio = np.zeros((LastSnap+1-FirstSnap)) 
        num = np.zeros((LastSnap+1-FirstSnap))       

        w = np.where((np.log10(G_history[LastSnap].Mvir/self.Hubble_h*1.0e10) > 12.5) & (G_history[LastSnap].Type == 0))[0]
        if(len(w) > 0):
          for which in xrange(len(w)):
            targetID = G_history[LastSnap].GalaxyIndex[w[which]]
            for snap in xrange(FirstSnap,LastSnap+1):
              ww = np.where(G_history[snap].GalaxyIndex == targetID)[0]
              if(len(ww) > 1):
                print 'there is more than one progenitor for galaxy ', targetID, G_history[snap].GalaxyIndex[ww]
                exit(1)
              if(len(ww) > 0):
                massratio[snap-FirstSnap] = massratio[snap-FirstSnap] + G_history[snap].StellarMass[ww[0]] / G_history[snap].Mvir[ww[0]]
                num[snap-FirstSnap] = num[snap-FirstSnap] + 1.0
    
        for snap in xrange(FirstSnap,LastSnap+1):
          if (num[snap-FirstSnap] > 0.0):
            massratio[snap-FirstSnap] = massratio[snap-FirstSnap] / num[snap-FirstSnap]

        nonzero = np.where(massratio > 0.0)[0]
        plt.plot(z[nonzero]+1.0, np.log10(massratio[nonzero]), 'r-', lw=3.0)

        print massratio
   
   
        plt.xscale('log', nonposy='clip')
   
        plt.ylabel(r'$\log_{10} M_{\mathrm{stars}}\ /\ M_{\mathrm{vir}}$')  # Set the y...
        plt.xlabel(r'$\mathrm{1+z}$')  # and the x-axis labels
    
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(1))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.5))
    
        plt.axis([0.0, 10.0, -3.2, -1.0])
    
        outputFile = OutputDir + '_C.History-massratio' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()
    
        # Add this plot to our output list
        OutputList.append(outputFile)



# =================================================================


#  'Main' section of code.  This if statement executes if the code is run from the 
#   shell command line, i.e. with 'python allresults.py'

if __name__ == '__main__':

    from optparse import OptionParser
    import os

    parser = OptionParser()
    parser.add_option(
        '-d',
        '--dir_name',
        dest='DirName',
        default='./results/millennium/',
        help='input directory name (default: ./results/millennium/)',
        metavar='DIR',
        )
    parser.add_option(
        '-f',
        '--file_base',
        dest='FileName',
        default='model',
        help='filename base (default: model)',
        metavar='FILE',
        )
    parser.add_option(
        '-n',
        '--file_range',
        type='int',
        nargs=2,
        dest='FileRange',
        default=(0, 7),
        help='first and last filenumbers (default: 0 7)',
        metavar='FIRST LAST',
        )
    parser.add_option(
        '-s',
        '--snap_range',
        type='int',
        nargs=2,
        dest='SnapRange',
        default=(0, 63),
        help='first and last snapshots (default: 0 63)',
        metavar='FIRST LAST',
        )


    (opt, args) = parser.parse_args()

    if opt.DirName[-1] != '/':
        opt.DirName += '/'

    OutputDir = opt.DirName + '/plots/'

    if not os.path.exists(OutputDir):
      os.makedirs(OutputDir)

    res = Results()

    print 'Running history...'

    FirstFile = opt.FileRange[0]
    LastFile = opt.FileRange[1]
    FirstSnap = opt.SnapRange[0]
    LastSnap = opt.SnapRange[1]

    # read in all files and put in G_history
    G_history = [0]*(LastSnap-FirstSnap+1)
    for snap in xrange(FirstSnap,LastSnap+1):

      print
      print 'SNAPSHOT NUMBER:  ', snap
      
      fin_base = opt.DirName + opt.FileName + res.redshift_file[snap]
      G_history[snap] = res.read_gals(fin_base, FirstFile, LastFile, snap)
      
    print


    # Estimate property growth
    print 'estimating the growth of select galaxies'
    
    # Initialise the growth structure properties
    for snap in xrange(FirstSnap,LastSnap+1):
      G_history[snap].deltaCentralMvir = 0.0
      G_history[snap].deltaMvir = 0.0
      G_history[snap].dMvirdT = 0.0
      G_history[snap].deltaColdGas = 0.0
      G_history[snap].deltaMstars = 0.0
      G_history[snap].dMstarsdT = 0.0
      G_history[snap].deltaHotGas = 0.0
      G_history[snap].deltaBlackHoleMass = 0.0
          
    # loop over selected z=0 halos (central, well resolved)
    for gal in xrange(len(G_history[LastSnap].Type)):

      if((G_history[LastSnap].Mvir[gal] > 10.0) & (G_history[LastSnap].Type[gal] == 0)):

        current_Mvir = G_history[LastSnap].Mvir[gal]
        current_Mstars = G_history[LastSnap].StellarMass[gal]
        current_dT = G_history[LastSnap].dT[gal] / 1.0e4
        gal_id = G_history[LastSnap].GalaxyIndex[gal]
        done = 0

        # With the galaxy ID in hand, find the progenitors
        for snap in xrange(LastSnap-1, FirstSnap, -1):

          if(done == 1):
            continue
            
          w = np.where(G_history[snap].GalaxyIndex == gal_id)[0]

          # Multiple progenitors ... gulp!
          if (len(w) > 1):
            print 'more than one progenitor!', len(w), snap, gal_id
            exit(1)

          # Skip if a skipped snapshot or no more progenitors
          if (len(w) == 0):
            continue

          prog = w[0]

          # Skip the unusual case of central->satellite - should i ignore the history of these altogether?
          if (G_history[snap].Type[prog] != 0):
            # print 'central->satellite: ', snap, G_history[snap].Type[prog], G_history[snap].Mvir[prog], current_mass
            done = 1
            continue

          # # Skip when the halo mass change exceeds tolerance ... potenital misclassification
          # if ( (G_history[snap].Mvir[prog] > 0.0) & ((G_history[snap].Mvir[prog]/current_mass > tolerance) | (current_mass/G_history[snap].Mvir[prog] > tolerance)) ):
          #   print 'halo mass change greater than tolerance: ', snap, tolerance, G_history[snap].Mvir[prog], current_mass
          #   done = 1
          #   continue

          if(G_history[snap].Mvir[prog] < 0.0):
            done = 1
            continue

          # All good! - record the growth and set-up for next progenitor
          if(done == 0):
                        
            G_history[snap].deltaMvir[prog] = current_Mvir - G_history[snap].Mvir[prog]
            G_history[snap].deltaMstars[prog] = current_Mstars - G_history[snap].StellarMass[prog]
            
            if(current_dT > 0.0):
              G_history[snap].dMvirdT[prog] = (current_Mvir - G_history[snap].Mvir[prog]) / current_dT
              G_history[snap].dMstarsdT[prog] = (current_Mstars - G_history[snap].StellarMass[prog]) / current_dT
              
            current_Mvir = G_history[snap].Mvir[prog]
            current_Mstars = G_history[snap].StellarMass[prog]
            current_dT = G_history[snap].dT[prog] / 1.0e4


    # res.StellarMassFunction(G_history)
    res.PlotHistory_deltaMvir(G_history)
    res.PlotHistory_deltaMstars(G_history)
    res.PlotHistory_massratio(G_history)
    
    

