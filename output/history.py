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

# Set some nice rc params

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
plt.rc('font', variant='monospace')
plt.rc('legend', numpoints=1, fontsize='x-large')
plt.rc('text', usetex=True)

OutputDir = '' # set in main below

OutputFormat = '.png'
TRANSPARENT = False

OutputList = []

Redshift = ['_z127.00', '_z80.00', '_z50.00', '_z30.00', '_z19.92', '_z18.24', '_z16.72', '_z15.34', '_z14.09', '_z12.94', '_z11.90', '_z10.94', '_z10.07', '_z9.28', '_z8.55', '_z7.88', '_z7.27', '_z6.71', '_z6.20', '_z5.72', '_z5.29', '_z4.89', '_z4.52', '_z4.18', '_z3.87', '_z3.58', '_z3.31', '_z3.06', '_z2.83', '_z2.62', '_z2.42', '_z2.24', '_z2.07', '_z1.91', '_z1.77', '_z1.63', '_z1.50', '_z1.39', '_z1.28', '_z1.17', '_z1.08', '_z0.99', '_z0.91', '_z0.83', '_z0.76', '_z0.69', '_z0.62', '_z0.56', '_z0.51', '_z0.46', '_z0.41', '_z0.36', '_z0.32', '_z0.28', '_z0.24', '_z0.21', '_z0.17', '_z0.14', '_z0.12', '_z0.09', '_z0.06', '_z0.04', '_z0.02', '_z0.00']


# ================================================================================
# Set up some basic attributes of the run
# and define the dtype for the input structure.

class Results:

    """ The following methods of this class generate the figures and plot them.
    """

    def __init__(self):
        """Here we set up some of the variables which will be global to this
        class."""

        self.BoxSize = 62.5     # Mpc/h
        self.MaxTreeFiles = 8   # FilesPerSnapshot
        # self.BoxSize = 500      # Mpc/h
        # self.MaxTreeFiles = 512 # FilesPerSnapshot

        self.Hubble_h = 0.73
        # This is to allow for variable size random tree files
        self.VolumeFactor = 1.0


    def read_gals(self, model_name, first_file, last_file):

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
            ('BulgeMass'                    , np.float32),                  
            ('HotGas'                       , np.float32),                  
            ('EjectedMass'                  , np.float32),                  
            ('BlackHoleMass'                , np.float32),                  
            ('IntraClusterStars'            , np.float32),                  
            ('MetalsColdGas'                , np.float32),                  
            ('MetalsStellarMass'            , np.float32),                  
            ('MetalsBulgeMass'              , np.float32),                  
            ('MetalsHotGas'                 , np.float32),                  
            ('MetalsEjectedMass'            , np.float32),                  
            ('MetalsIntraClusterStars'      , np.float32),                  
            ('Sfr'                          , np.float32),                  
            ('SfrBulge'                     , np.float32),                  
            ('SfrIntraClusterStars'         , np.float32),                  
            ('DiskRadius'                   , np.float32),                  
            ('Cooling'                      , np.float32),                  
            ('Heating'                      , np.float32)          
            ]
        names = [Galdesc_full[i][0] for i in xrange(len(Galdesc_full))]
        formats = [Galdesc_full[i][1] for i in xrange(len(Galdesc_full))]
        Galdesc = np.dtype({'names':names, 'formats':formats}, align=True)


        # Initialize variables.
        TotNTrees = 0
        TotNGals = 0
        FileIndexRanges = []

        print "Determining array storage requirements."
        
        # Read each file and determine the total number of galaxies to be read in
        for fnr in xrange(first_file,last_file+1):
            fname = model_name+'_'+str(fnr)  # Complete filename
        
            if getFileSize(fname) == 0:
                print "File\t%s  \tis empty!  Skipping..." % (fname)
                continue
        
            fin = open(fname, 'rb')  # Open the file
            Ntrees = np.fromfile(fin,np.dtype(np.int32),1)  # Read number of trees in file
            NtotGals = np.fromfile(fin,np.dtype(np.int32),1)[0]  # Read number of gals in file.
            TotNTrees = TotNTrees + Ntrees  # Update total sim trees number
            TotNGals = TotNGals + NtotGals  # Update total sim gals number
            fin.close()

        print "Input files contain:\t%d trees ;\t%d galaxies ." % (TotNTrees, TotNGals)

        # Initialize the storage array
        G = np.empty(TotNGals, dtype=Galdesc)

        offset = 0  # Offset index for storage array

        # Open each file in turn and read in the preamble variables and structure.
        print "Reading in files."
        for fnr in xrange(first_file,last_file+1):
            fname = model_name+'_'+str(fnr)  # Complete filename
        
            if getFileSize(fname) == 0:
                continue
        
            fin = open(fname, 'rb')  # Open the file
            Ntrees = np.fromfile(fin, np.dtype(np.int32), 1)  # Read number of trees in file
            NtotGals = np.fromfile(fin, np.dtype(np.int32), 1)[0]  # Read number of gals in file.
            GalsPerTree = np.fromfile(fin, np.dtype((np.int32, Ntrees)),1) # Read the number of gals in each tree
            print ":   Reading N=", NtotGals, "   \tgalaxies from file: ", fname
            GG = np.fromfile(fin, Galdesc, NtotGals)  # Read in the galaxy structures
        
            FileIndexRanges.append((offset,offset+NtotGals))
        
            # Slice the file array into the global array
            # N.B. the copy() part is required otherwise we simply point to
            # the GG data which changes from file to file
            # NOTE THE WAY PYTHON WORKS WITH THESE INDICES!
            G[offset:offset+NtotGals]=GG[0:NtotGals].copy()
            
            del(GG)
            offset = offset + NtotGals  # Update the offset position for the global array
        
            fin.close()  # Close the file


        print "Total galaxies considered:", TotNGals
        print

        # Convert the Galaxy array into a recarray
        G = G.view(np.recarray)

        return G


# ---------------------------------------------------------
    
    # def PlotStellarMassHistory(self, hist):
    # 
    #         
    #     outputFile = OutputDir + 'History' + OutputFormat
    #     plt.savefig(outputFile)  # Save the figure
    #     print 'Saved file to', outputFile
    #     plt.close()
    #         
    #     # Add this plot to our output list
    #     OutputList.append(outputFile)


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
        default='./results/fullmillennium/',
        help='input directory name (default: ./results/fullmillennium/)',
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
        default=(18, 63),
        help='first and last snapshots (default: 18 63)',
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

    G_history = [0]*64
    for snap in xrange(FirstSnap,LastSnap+1):

      print
      print 'SNAPSHOT NUMBER:  ', snap
      print
      
      fin_base = opt.DirName + opt.FileName + Redshift[snap]
      G_history[snap] = res.read_gals(fin_base, FirstFile, LastFile)

    print


    # Pick a galaxy and check that its histories have been indexed correctly

    Index = 0 + 0 * 1e6 + 0 * 1e12    # see line 66 in core_save.c
    for snap in xrange(FirstSnap, LastSnap+1):
      
      wTemp = np.where(G_history[snap].GalaxyIndex == Index)[0]
      
      if(len(wTemp) > 1): 
        print 'WARINING - non-unique galaxy with index', Index, len(wTemp)      

      if(len(wTemp) != 0):
        w = wTemp[0]
        print snap, Index, G_history[snap].TreeIdx[w], G_history[snap].Type[w], np.log10(G_history[snap].Mvir[w]*1.0e10), np.log10(G_history[snap].StellarMass[w]*1.0e10)
       
   

    # res.PlotStellarMassHistory(hist)


