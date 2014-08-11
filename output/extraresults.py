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
dilute = 7500       # Number of galaxies to plot in scatter plots
sSFRcut = -11.0     # Divide quiescent from star forming galaxies (when plotmags=0)


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
            ('mergeType'                    , np.int32),                    
            ('mergeIntoID'                  , np.int32),                    
            ('mergeIntoSnapNum'             , np.int32),                    
            ('dT'                           , np.int32),                    
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
            ('SfrDisk'                      , np.float32),                  
            ('SfrBulge'                     , np.float32),                  
            ('SfrDiskZ'                     , np.float32),                  
            ('SfrBulgeZ'                    , np.float32),                  
            ('DiskRadius'                   , np.float32),                  
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

        w = np.where(G.StellarMass > 1.0)[0]
        print "Galaxies more massive than 10^10Msun/h:", len(w)

        print

        # Calculate the volume given the first_file and last_file
        self.volume = self.BoxSize**3.0 * goodfiles / self.MaxTreeFiles

        return G

# ---------------------------------------------------------
    
    def QuiescentFraction(self, G):
    
        print 'Plotting the quiescent fraction vs stellar mass'
    
        seed(2222)
    
        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure
        
        groupscale = 12.5
        
        w = np.where(G.StellarMass > 0.0)[0]
        StellarMass = np.log10(G.StellarMass[w] * 1.0e10 / self.Hubble_h)
        CentralMvir = np.log10(G.CentralMvir[w] * 1.0e10 / self.Hubble_h)
        Type = G.Type[w]
        sSFR = (G.SfrDisk[w] + G.SfrBulge[w]) / (G.StellarMass[w] * 1.0e10 / self.Hubble_h)

        MinRange = 9.5
        MaxRange = 12.0
        Interval = 0.1
        Nbins = int((MaxRange-MinRange)/Interval)
        Range = np.arange(MinRange, MaxRange, Interval)
        
        Mass = []
        Fraction = []
        CentralFraction = []
        SatelliteFraction = []
        SatelliteFractionLo = []
        SatelliteFractionHi = []

        for i in xrange(Nbins-1):
            
            w = np.where((StellarMass >= Range[i]) & (StellarMass < Range[i+1]))[0]
            if len(w) > 0:
                wQ = np.where((StellarMass >= Range[i]) & (StellarMass < Range[i+1]) & (sSFR < 10.0**sSFRcut))[0]
                Fraction.append(1.0*len(wQ) / len(w))
            else:
                Fraction.append(0.0)

            w = np.where((Type == 0) & (StellarMass >= Range[i]) & (StellarMass < Range[i+1]))[0]
            if len(w) > 0:
                wQ = np.where((Type == 0) & (StellarMass >= Range[i]) & (StellarMass < Range[i+1]) & (sSFR < 10.0**sSFRcut))[0]
                CentralFraction.append(1.0*len(wQ) / len(w))
            else:
                CentralFraction.append(0.0)

            w = np.where((Type == 1) & (StellarMass >= Range[i]) & (StellarMass < Range[i+1]))[0]
            if len(w) > 0:
                wQ = np.where((Type == 1) & (StellarMass >= Range[i]) & (StellarMass < Range[i+1]) & (sSFR < 10.0**sSFRcut))[0]
                SatelliteFraction.append(1.0*len(wQ) / len(w))
                wQ = np.where((Type == 1) & (StellarMass >= Range[i]) & (StellarMass < Range[i+1]) & (sSFR < 10.0**sSFRcut) & (CentralMvir < groupscale))[0]
                SatelliteFractionLo.append(1.0*len(wQ) / len(w))
                wQ = np.where((Type == 1) & (StellarMass >= Range[i]) & (StellarMass < Range[i+1]) & (sSFR < 10.0**sSFRcut) & (CentralMvir > groupscale))[0]
                SatelliteFractionHi.append(1.0*len(wQ) / len(w))                
            else:
                SatelliteFraction.append(0.0)
                SatelliteFractionLo.append(0.0)
                SatelliteFractionHi.append(0.0)
                
            Mass.append((Range[i] + Range[i+1]) / 2.0)
                
            print '  ', Mass[i], Fraction[i], CentralFraction[i], SatelliteFraction[i]
        
        Mass = np.array(Mass)
        Fraction = np.array(Fraction)
        CentralFraction = np.array(CentralFraction)
        SatelliteFraction = np.array(SatelliteFraction)
        SatelliteFractionLo = np.array(SatelliteFractionLo)
        SatelliteFractionHi = np.array(SatelliteFractionHi)
        
        w = np.where(Fraction > 0)[0]
        plt.plot(Mass[w], Fraction[w], label='All')

        w = np.where(CentralFraction > 0)[0]
        plt.plot(Mass[w], CentralFraction[w], color='Blue', label='Centrals')

        w = np.where(SatelliteFraction > 0)[0]
        plt.plot(Mass[w], SatelliteFraction[w], color='Red', label='Satellites')

        w = np.where(SatelliteFractionLo > 0)[0]
        plt.plot(Mass[w], SatelliteFractionLo[w], 'r--', label='Satellites-Lo')

        w = np.where(SatelliteFractionHi > 0)[0]
        plt.plot(Mass[w], SatelliteFractionHi[w], 'r-.', label='Satellites-Hi')
        
        plt.xlabel(r'$\log_{10} M_{\mathrm{stellar}}\ (M_{\odot})$')  # Set the x-axis label
        plt.ylabel(r'$\mathrm{Quescient\ Fraction}$')  # Set the y-axis label
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
            
        plt.axis([9.5, 12.0, 0.0, 1.05])
            
        leg = plt.legend(loc='lower right')
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')
            
        outputFile = OutputDir + 'E2.QuiescentFraction' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()
            
        # Add this plot to our output list
        OutputList.append(outputFile)


# ---------------------------------------------------------
    
    def BaryonFraction(self, G):
    
        print 'Plotting the average baryon fraction vs halo mass'
    
        seed(2222)
    
        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure
        
        HaloMass = np.log10(G.Mvir * 1.0e10 / self.Hubble_h)
        Baryons = G.StellarMass + G.ColdGas + G.HotGas + G.EjectedMass + G.IntraClusterStars + G.BlackHoleMass
        fileNr = np.floor(G.GalaxyIndex / 1.0e12)

        MinHalo = 11.0
        MaxHalo = 15.0
        Interval = 0.1
        Nbins = int((MaxHalo-MinHalo)/Interval)
        HaloRange = np.arange(MinHalo, MaxHalo, Interval)
        
        MeanCentralHaloMass = []
        MeanBaryonFraction = []
        MeanBaryonFractionU = []
        MeanBaryonFractionL = []

        MeanStars = []
        MeanCold = []
        MeanHot = []
        MeanEjected = []
        MeanICS = []
        MeanBH = []

        for i in xrange(Nbins-1):
            
            w1 = np.where((G.Type == 0) & (HaloMass >= HaloRange[i]) & (HaloMass < HaloRange[i+1]))[0]
            HalosFound = len(w1)
            
            if HalosFound > 1:  
                
                BaryonFraction = []
                CentralHaloMass = []
                
                Stars = []
                Cold = []
                Hot = []
                Ejected = []
                ICS = []
                BH = []
                
                for j in xrange(HalosFound):
                    
                    w2 = np.where((G.FOFHaloIdx == G.FOFHaloIdx[w1[j]]) & (G.TreeIdx == G.TreeIdx[w1[j]]) & (fileNr == fileNr[w1[j]]))[0]
                    CentralAndSatellitesFound = len(w2)
                    
                    if CentralAndSatellitesFound > 0:
                        BaryonFraction.append(sum(Baryons[w2]) / G.Mvir[w1[j]])
                        CentralHaloMass.append(np.log10(G.Mvir[w1[j]] * 1.0e10 / self.Hubble_h))

                        Stars.append(sum(G.StellarMass[w2]) / G.Mvir[w1[j]])
                        Cold.append(sum(G.ColdGas[w2]) / G.Mvir[w1[j]])
                        Hot.append(sum(G.HotGas[w2]) / G.Mvir[w1[j]])
                        Ejected.append(sum(G.EjectedMass[w2]) / G.Mvir[w1[j]])
                        ICS.append(sum(G.IntraClusterStars[w2]) / G.Mvir[w1[j]])
                        BH.append(sum(G.BlackHoleMass[w2]) / G.Mvir[w1[j]])                        
                                
                MeanCentralHaloMass.append(np.mean(CentralHaloMass))
                MeanBaryonFraction.append(np.mean(BaryonFraction))
                MeanBaryonFractionU.append(np.mean(BaryonFraction) + np.var(BaryonFraction))
                MeanBaryonFractionL.append(np.mean(BaryonFraction) - np.var(BaryonFraction))
                
                MeanStars.append(np.mean(Stars))
                MeanCold.append(np.mean(Cold))
                MeanHot.append(np.mean(Hot))
                MeanEjected.append(np.mean(Ejected))
                MeanICS.append(np.mean(ICS))
                MeanBH.append(np.mean(BH))
                
                print '  ', HaloRange[i], HaloRange[i+1], HalosFound#, MeanCentralHaloMass[i], MeanBaryonFraction[i], MeanStars[i], MeanCold[i], MeanHot[i], MeanEjected[i], MeanICS[i], MeanBH[i]
        
        plt.plot(MeanCentralHaloMass, MeanBaryonFraction, 'k-', label='TOTAL')#, color='purple', alpha=0.3)
        plt.fill_between(MeanCentralHaloMass, MeanBaryonFractionU, MeanBaryonFractionL, 
            facecolor='purple', alpha=0.25, label='TOTAL')
        
        plt.plot(MeanCentralHaloMass, MeanStars, 'k--', label='Stars')
        plt.plot(MeanCentralHaloMass, MeanCold, label='Cold', color='blue')
        plt.plot(MeanCentralHaloMass, MeanHot, label='Hot', color='red')
        plt.plot(MeanCentralHaloMass, MeanEjected, label='Ejected', color='green')
        plt.plot(MeanCentralHaloMass, MeanICS, label='ICS', color='yellow')
        # plt.plot(MeanCentralHaloMass, MeanBH, 'k:', label='BH')
        
        plt.xlabel(r'$\mathrm{Central}\ \log_{10} M_{\mathrm{vir}}\ (M_{\odot})$')  # Set the x-axis label
        plt.ylabel(r'$\mathrm{Baryon\ Fraction}$')  # Set the y-axis label
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
            
        plt.axis([10.8, 15.0, 0.0, 0.23])
            
        leg = plt.legend(bbox_to_anchor=[0.99, 0.6])
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')
            
        outputFile = OutputDir + 'E1.BaryonFraction' + OutputFormat
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

    print 'Running allresults...'

    FirstFile = opt.FileRange[0]
    LastFile = opt.FileRange[1]

    fin_base = opt.DirName + opt.FileName
    G = res.read_gals(fin_base, FirstFile, LastFile)

    res.QuiescentFraction(G)
    res.BaryonFraction(G)


