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

dilute = 5000       # Number of galaxies to plot in scatter plots


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


    def read_gals(self, model_name, first_file, last_file):

        # The input galaxy structure:
        Galdesc_full = [
                        ('SnapNum'                      , np.int32),
                        ('Type'                         , np.int32),
                        ('GalaxyIndex'                  , np.int64),
                        ('CentralGalaxyIndex'           , np.int64),
                        ('CtreesHaloID'                 , np.int64),
                        ('TreeIndex'                    , np.int32),
                        ('CtreesCentralID'              , np.int64),
                        ('mergeType'                    , np.int32),
                        ('mergeIntoID'                  , np.int32),
                        ('mergeIntoSnapNum'             , np.int32),
                        ('dT'                           , np.float32),
                        ('Pos'                          , (np.float32, 3)),
                        ('Vel'                          , (np.float32, 3)),
                        ('Spin'                         , (np.float32, 3)),
                        ('Len'                          , np.int32),
                        ('Mvir'                         , np.float32),
                        ('CentralMvir'                  , np.float32),
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
                        ('SfrDisk'                      , np.float32),
                        ('SfrBulge'                     , np.float32),
                        ('SfrDiskZ'                     , np.float32),
                        ('SfrBulgeZ'                    , np.float32),
                        ('DiskRadius'                   , np.float32),
                        ('Cooling'                      , np.float32),
                        ('Heating'                      , np.float32),
                        ('QuasarModeBHaccretionMass'    , np.float32),
                        ('TimeOfLastMajorMerger'         , np.float32),
                        ('TimeOfLastMinorMerger'         , np.float32),
                        ('OutflowRate'                  , np.float32),
                        ('MeanStarAge'                  , np.float32),
                        ('infallMvir'                   , np.float32),
                        ('infallVvir'                   , np.float32),
                        ('infallVmax'                   , np.float32)
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
        goodfiles = 0
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

        print
        print "Input files contain:\t%d trees ;\t%d galaxies ." % (TotNTrees, TotNGals)
        print

        # Initialize the storage array
        G = np.empty(TotNGals, dtype=Galdesc)

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
        
            FileIndexRanges.append((offset,offset+NtotGals))
        
            # Slice the file array into the global array
            # N.B. the copy() part is required otherwise we simply point to
            # the GG data which changes from file to file
            # NOTE THE WAY PYTHON WORKS WITH THESE INDICES!
            G[offset:offset+NtotGals]=GG[0:NtotGals].copy()
            
            del(GG)
            offset = offset + NtotGals  # Update the offset position for the global array
        
            fin.close()  # Close the file


        print
        print "Total galaxies considered:", TotNGals

        # Convert the Galaxy array into a recarray
        G = G.view(np.recarray)

        print

        return G


# ---------------------------------------------------------

    def CoolingLuminosity(self, G, H):

        print 'Plotting cooling luminosity'

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        # Peres et al. 1998 (tables 2 & 5)
        Obs = np.array([
          [6.2, 2.7, 3.7, 0.2, 6.2, 4.8, 1.1, 1.2, 46.0, 58.0            ], 
          [0.0, 0.0, 0.0, 0.0, 5.1, 0.0, 0.0, 0.0, -1.0, 0.0             ], 
          [0.0, 0.0, 0.0, 0.0, 2.4, 0.3, 0.02, 0.02, -1.0, 131.0         ],
          [3.6, 0.3, 0.7, 0.1, 3.6, 0.5, 0.03, 0.04, -1.0, -1.0          ],
          [5.8, 0.0, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0             ],
          [0.0, 0.0, 0.0, 0.0, 7.8, 0.9, 1.6, 0.9, -1.0, 0.0             ],
          [4.1, 7.8, 0.3, 0.7, 4.1, 7.0, 1.2, 1.2, 690.0, -1.0           ], 
          [5.5, 6.8, 999.9, 0.0, 5.5, 13.7, 0.4, 0.4, 42000.0, 21200.0   ], 
          [3.0, 3.8, 1.3, 0.3, 3.0, 5.0, 0.3, 0.3, -1.0, 24.1            ],
          [0.0, 0.0, 0.0, 0.0, 5.5, 0.4, 1.0, 0.4, -1.0, -1.0            ],
          [6.8, 14.5, 0.5, 0.3, 6.8, 17.3, 1.7, 2.7, -1.0, 35.5          ],
          [6.2, 0.1, 0.03, 0.03, 6.2, 0.0, 0.7, 0.0, -1.0, -1.0          ],
          [4.7, 1.9, 0.2, 0.1, 4.7, 2.0, 0.1, 0.2, 44.0, -1.0            ], 
          [0.0, 0.0, 0.0, 0.0, 5.2, 0.0, 0.1, 0.0, 1922.0, -1.0          ], 
          [4.3, 0.1, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0             ],
          [8.5, 34.9, 1.8, 1.8, 8.5, 43.6, 4.6, 2.8, 480.0, 2370.0       ],
          [6.6, 4.5, 0.6, 1.0, 6.6, 4.1, 2.3, 0.6, -1.0, -1.0            ],
          [8.7, 0.0, 0.0, 0.0, 8.7, 0.0, 0.7, 0.0, -1.0, 9.0             ],
          [3.8, 4.7, 999.9, 0.0, 3.8, 4.7, 1.0, 1.2, 14000.0, 40800.0    ], 
          [3.3, 0.1, 0.01, 0.01, 3.3, 0.15, 0.04, 0.1, -1.0, 0.0         ], 
          [0.0, 0.0, 0.0, 0.0, 3.5, 0.0, 0.0, 0.0, -1.0, 0.0             ],
          [2.4, 0.1, 999.9, 0.0, 2.4, 0.3, 0.01, 0.01, 61000.0, 22400.0  ],
          [3.6, 0.4, 999.9, 0.0, 3.6, 0.5, 0.1, 0.1, 1530.0, -1.0        ],
          [8.0, 0.01, 0.0, 0.0, 8.0, 0.0, 0.0, 0.0, 84.0, 207.0          ],
          [4.7, 0.2, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0, 112.0, 99.0           ], 
          [0.0, 0.0, 0.0, 0.0, 4.4, 0.0, 0.3, 0.0, 440.0, 1160.0         ], 
          [5.5, 5.4, 4.1, 0.9, 0.0, 0.0, 0.0, 0.0, -1.0, -1.0            ],
          [0.0, 0.0, 0.0, 0.0, 7.0, 3.3, 1.0, 1.0, -1.0, -1.0            ],
          [10.1, 18.0, 1.0, 1.0, 10.1, 26.1, 9.5, 1.5, -1.0, 0.0         ],
          [4.6, 0.02, 0.03, 999.9, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0         ],
          [6.5, 0.8, 0.4, 0.6, 6.5, 1.0, 0.9, 0.2, -1.0, 4.5             ], 
          [0.0, 0.0, 0.0, 0.0, 3.8, 0.5, 0.3, 0.3, -1.0, 0.0             ], 
          [7.6, 1.8, 0.6, 0.7, 7.6, 1.6, 1.2, 0.5, -1.0, 8.4             ],
          [5.1, 10.4, 0.4, 0.4, 5.1, 8.5, 0.6, 0.2, 260.0, 930.0         ],
          [7.8, 16.8, 3.5, 1.1, 7.8, 17.8, 1.2, 2.9, 89.0, 550.0         ],
          [3.4, 1.7, 0.2, 0.1, 3.4, 1.9, 0.4, 0.02, 1030.0, 5400.0       ],
          [3.0, 1.4, 0.4, 0.5, 3.0, 2.3, 0.1, 0.6, -1.0, 126.0           ], 
          [8.4, 0.5, 0.2, 0.2, 0.0, 0.0, 0.0, 0.0, -1.0, 14.0            ], 
          [0.0, 0.0, 0.0, 0.0, 4.1, 0.6, 0.1, 0.3, -1.0, 15.0            ],
          [11.0, 11.2, 2.1, 2.8, 11.0, 13.0, 1.8, 4.6, -1.0, 0.0         ],
          [4.7, 3.0, 0.2, 0.5, 4.7, 2.7, 0.3, 0.1, 480.0, 3700.0         ],
          [9.0, 40.3, 8.8, 4.2, 9.0, 40.1, 4.6, 4.6, -1.0, 70.1          ],
          [0.0, 0.0, 0.0, 0.0, 7.9, 0.7, 1.7, 999.9, -1.0, -1.0          ], 
          [0.0, 0.0, 0.0, 0.0, 7.1, 6.6, 0.8, 4.0, -1.0, 2.41            ], 
          [7.5, 0.0, 0.26, 0.0, 7.5, 0.0, 0.2, 0.0, -1.0, 0.0            ],
          [9.0, 1.3, 0.1, 0.01, 9.0, 3.8, 0.9, 2.6, -1.0, 29.1           ],
          [0.0, 0.0, 0.0, 0.0, 7.3, 0.0, 0.1, 0.0, -1.0, 0.0             ],
          [9.9, 0.1, 2.6, 999.9, 9.9, 0.5, 1.5, 999.9, -1.0, 0.0         ],
          [7.3, 8.1, 0.3, 0.4, 7.3, 9.3, 0.8, 1.1, 210000.0, -1.0        ], 
          [0.0, 0.0, 0.0, 0.0, 6.5, 0.0, 0.2, 0.0, -1.0, -1.0            ], 
          [6.0, 8.2, 0.7, 0.9, 6.0, 8.2, 1.3, 1.9, 410.0, 1880.0         ],
          [0.0, 0.0, 0.0, 0.0, 3.3, 1.1, 0.3, 0.2, -1.0, 28.4            ],
          [3.5, 1.5, 0.4, 0.1, 3.5, 1.6, 0.2, 0.2, 117.0, 1290.0         ], 
        ], dtype=np.float32)

        cut = 10000.0

        OtempH = Obs[:, 0]
        OlumH = Obs[:, 1] * 1.0e4
        OlumHerrU = Obs[:, 2] * 1.0e4
        OlumHerrD = Obs[:, 3] * 1.0e4

        OtempP = Obs[:, 4]
        OlumP = Obs[:, 5] * 1.0e4
        OlumPerrU = Obs[:, 6] * 1.0e4
        OlumPerrD = Obs[:, 7] * 1.0e4

        radioL = Obs[:, 8]
        radioS = Obs[:, 9]

        w = np.where((OlumHerrU/OtempH < cut) & ((OlumHerrD/OtempH < cut)))[0]
        plt.errorbar(OtempH[w], OlumH[w], yerr=[OlumHerrD[w], OlumHerrU[w]], color='r', lw=2.0, alpha=0.4, marker='s', markersize=10, ls='none', label='HRI')

        w = np.where((OlumPerrU/OtempP < cut) & ((OlumPerrD/OtempP < cut)))[0]
        plt.errorbar(OtempP[w], OlumP[w], yerr=[OlumPerrU[w], OlumPerrD[w]], color='b', lw=2.0, alpha=0.4, marker='*', markersize=15, ls='none', label='PSPC')


        HaloTemp = 35.9*(G.Vvir*G.Vvir) / 11604 / 1.0e3
        CoolingLum = 10**(G.Cooling - 40.0)
        HeatingLum = 10**(G.Heating - 40.0)

        w = np.where((HaloTemp > 0.3) & (CoolingLum > 0.1) & (abs(CoolingLum/HeatingLum) < 1.01))[0]
        if(len(w) > dilute): w = sample(w, dilute)
        plt.scatter(HaloTemp[w], CoolingLum[w], marker='o', s=5, c='g', alpha=0.5)



        HaloTemp = 35.9*(H.Vvir*H.Vvir) / 11604 / 1.0e3
        CoolingLum = 10**(H.Cooling - 40.0)
        HeatingLum = 10**(H.Heating - 40.0)

        w = np.where((HaloTemp > 0.3) & (CoolingLum > 0.1) & (abs(CoolingLum/HeatingLum) < 1.01))[0]
        if(len(w) > dilute): w = sample(w, dilute)
        plt.scatter(HaloTemp[w], CoolingLum[w], marker='o', s=3, c='k', alpha=0.5)

        
        plt.text(0.85, 1300.0, r'$\mathrm{2006\ model}$', fontsize=16)
        plt.text(2.2, 4.5, r'$\mathrm{SAGE}$', fontsize=16)
        
        plt.xlabel(r'$\mathrm{Halo\ T_{vir}\ (keV)}$') 
        plt.ylabel(r'$\mathrm{cooling-heating\ balance\ (10^{40}\ erg/s)}$')

        plt.xscale('log', nonposy='clip')
        plt.yscale('log', nonposy='clip')

        plt.axis([0.03, 13.0, 0.1, 1000000.0])   
            
        leg = plt.legend(loc='upper left', title='Peres et al. 1998')
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')

        outputFile = OutputDir + 'X.CoolingLuminosity' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)


# ---------------------------------------------------------

    def HeatingLuminosity(self, G, H):

        print 'Plotting heating luminosity'

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        HaloTemp = 35.9*(G.Vvir*G.Vvir) / 11604 / 1.0e3
        HeatingLum = 10**(G.Heating - 41.0)

        w = np.where((HaloTemp > 0.3) & (HeatingLum > 0.03))[0]
        if(len(w) > dilute): w = sample(w, dilute)
        plt.scatter(HaloTemp[w], HeatingLum[w], marker='o', s=3, c='k', alpha=0.5)
        
        plt.xlabel(r'$\mathrm{T_{vir}\ (keV)}$') 
        plt.ylabel(r'$\mathrm{heating\ luminosity\ (10^{41}\ erg/s)}$')

        plt.xscale('log', nonposy='clip')
        plt.yscale('log', nonposy='clip')

        plt.axis([0.3, 20.0, 0.03, 100.0])   

        outputFile = OutputDir + 'Y.HeatingLuminosity' + OutputFormat
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
        default='model_z0.000',
        help='filename base (default: model_z0.000)',
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

    (opt, args) = parser.parse_args()

    if opt.DirName[-1] != '/':
        opt.DirName += '/'

    OutputDir = opt.DirName + 'plots/'

    if not os.path.exists(OutputDir):
        os.makedirs(OutputDir)

    res = Results()

    print 'Running cooling_heating...'

    FirstFile = opt.FileRange[0]
    LastFile = opt.FileRange[1]

    fin_base = opt.DirName + opt.FileName
    G = res.read_gals(fin_base, FirstFile, LastFile)

    fin_base = opt.DirName + '2006_heating/' + opt.FileName
    H = res.read_gals(fin_base, FirstFile, LastFile)

    res.CoolingLuminosity(G, H)
    # res.HeatingLuminosity(G, H)


