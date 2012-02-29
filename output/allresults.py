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

# ================================================================================
# Set up some basic attributes of the run
# and define the dtype for the input structure.

SSFRcut = -11.0  # split type at a SSFR of 10^-11
dilute = 3500  # Number of galaxies to plot in scatter plots

# ================================================================================


class Results:

    """ The following methods of this class generate the figures and plot them.
    """

    def __init__(self):
        """Here we set up some of the variables which will be global to this
        class."""

        # self.BoxSize = 62.5     # Mpc/h
        # self.MaxTreeFiles = 8   # FilesPerSnapshot
        self.BoxSize = 500      # Mpc/h
        self.MaxTreeFiles = 512 # FilesPerSnapshot

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
        print

        # Convert the Galaxy array into a recarray
        G = G.view(np.recarray)

        # Calculate the volume given the first_file and last_file
        self.volume = self.BoxSize**3.0 * (last_file - first_file + 1.) /\
                        self.MaxTreeFiles * self.VolumeFactor 

        return G


# --------------------------------------------------------

    def StellarMassFunction(self, G):

        print 'Plotting the stellar mass function'

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        binwidth = 0.1  # mas function histogram bin width

        # calculate all
        w = np.where(G.StellarMass > 0.0)[0]
        mass = np.log10(G.StellarMass[w] * 1.0e10 * self.Hubble_h)
        sSFR = G.Sfr[w] / (G.StellarMass[w] * 1.0e10 * self.Hubble_h)
        type = G.Type[w]

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = (ma - mi) / binwidth

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # additionally calculate red
        w = np.where(sSFR < 1.0e-11)[0]
        massRED = mass[w]
        (countsRED, binedges) = np.histogram(massRED, range=(mi, ma), bins=NB)

        # additionally calculate blue
        w = np.where(sSFR > 1.0e-11)[0]
        massBLU = mass[w]
        (countsBLU, binedges) = np.histogram(massBLU, range=(mi, ma), bins=NB)

        # Baldry+ 2008 modified data used for the MCMC fitting
        Baldry = np.array([
            [7.05, 1.3531e-01, 6.0741e-02],
            [7.15, 1.3474e-01, 6.0109e-02],
            [7.25, 2.0971e-01, 7.7965e-02],
            [7.35, 1.7161e-01, 3.1841e-02],
            [7.45, 2.1648e-01, 5.7832e-02],
            [7.55, 2.1645e-01, 3.9988e-02],
            [7.65, 2.0837e-01, 4.8713e-02],
            [7.75, 2.0402e-01, 7.0061e-02],
            [7.85, 1.5536e-01, 3.9182e-02],
            [7.95, 1.5232e-01, 2.6824e-02],
            [8.05, 1.5067e-01, 4.8824e-02],
            [8.15, 1.3032e-01, 2.1892e-02],
            [8.25, 1.2545e-01, 3.5526e-02],
            [8.35, 9.8472e-02, 2.7181e-02],
            [8.45, 8.7194e-02, 2.8345e-02],
            [8.55, 7.0758e-02, 2.0808e-02],
            [8.65, 5.8190e-02, 1.3359e-02],
            [8.75, 5.6057e-02, 1.3512e-02],
            [8.85, 5.1380e-02, 1.2815e-02],
            [8.95, 4.4206e-02, 9.6866e-03],
            [9.05, 4.1149e-02, 1.0169e-02],
            [9.15, 3.4959e-02, 6.7898e-03],
            [9.25, 3.3111e-02, 8.3704e-03],
            [9.35, 3.0138e-02, 4.7741e-03],
            [9.45, 2.6692e-02, 5.5029e-03],
            [9.55, 2.4656e-02, 4.4359e-03],
            [9.65, 2.2885e-02, 3.7915e-03],
            [9.75, 2.1849e-02, 3.9812e-03],
            [9.85, 2.0383e-02, 3.2930e-03],
            [9.95, 1.9929e-02, 2.9370e-03],
            [10.05, 1.8865e-02, 2.4624e-03],
            [10.15, 1.8136e-02, 2.5208e-03],
            [10.25, 1.7657e-02, 2.4217e-03],
            [10.35, 1.6616e-02, 2.2784e-03],
            [10.45, 1.6114e-02, 2.1783e-03],
            [10.55, 1.4366e-02, 1.8819e-03],
            [10.65, 1.2588e-02, 1.8249e-03],
            [10.75, 1.1372e-02, 1.4436e-03],
            [10.85, 9.1213e-03, 1.5816e-03],
            [10.95, 6.1125e-03, 9.6735e-04],
            [11.05, 4.3923e-03, 9.6254e-04],
            [11.15, 2.5463e-03, 5.0038e-04],
            [11.25, 1.4298e-03, 4.2816e-04],
            [11.35, 6.4867e-04, 1.6439e-04],
            [11.45, 2.8294e-04, 9.9799e-05],
            [11.55, 1.0617e-04, 4.9085e-05],
            [11.65, 3.2702e-05, 2.4546e-05],
            [11.75, 1.2571e-05, 1.2571e-05],
            [11.85, 8.4589e-06, 8.4589e-06],
            [11.95, 7.4764e-06, 7.4764e-06],
            ], dtype=np.float32)

        plt.yscale('log', nonposy='clip')
        
        # Finally plot the data
        # plt.errorbar(
        #     Baldry[:, 0],
        #     Baldry[:, 1],
        #     yerr=Baldry[:, 2],
        #     color='g',
        #     linestyle=':',
        #     lw = 1.5,
        #     label='Baldry et al. 2008',
        #     )

        plt.fill_between(Baldry[:, 0], Baldry[:, 1]+Baldry[:, 2], Baldry[:, 1]-Baldry[:, 2], 
            facecolor='purple', alpha=0.25, label='Baldry et al. 2008')

        # This next line is just to get the shaded region to appear correctly in the legend
        plt.plot(xaxeshisto, counts / self.volume / binwidth, label='Baldry et al. 2008', color='purple', alpha=0.3)

        # Cole et al. 2001 SMF
        # M = np.arange(7.0, 13.0, 0.01)
        # Mstar = np.log10(7.07*1.0e10)
        # alpha = -1.18
        # phistar = 0.009
        # xval = 10.0 ** (M-Mstar)
        # yval = np.log(10.) * phistar * xval ** (alpha+1) * np.exp(-xval)      
        # plt.plot(M, yval, 'g:', lw=1.5, label='Cole et al. 2001')  # Plot the SMF
        
        # Overplot the model histograms
        plt.plot(xaxeshisto, counts / self.volume / binwidth, 'k-', label='Model - All')
        plt.plot(xaxeshisto, countsRED / self.volume / binwidth, 'r:', lw=2, label='Model - Red')
        plt.plot(xaxeshisto, countsBLU / self.volume / binwidth, 'b:', lw=2, label='Model - Blue')

        plt.axis([8.5, 12.5, 2.0e-6, 2.0e-1])

        # Set the x-axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.1))

        plt.ylabel(r'$\phi(h^3\ \mathrm{Mpc}^{-3}\ \mathrm{dex}^{-1}$)')  # Set the y...
        plt.xlabel(r'$\log_{10} M_{\mathrm{stars}} (h^{-2}M_{\odot})$')  # and the x-axis labels

        leg = plt.legend(loc='lower left', numpoints=1,
                         labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')

        outputFile = OutputDir + '1.StellarMassFunction' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)


# ---------------------------------------------------------

    def BaryonicMassFunction(self, G):

        print 'Plotting the stellar mass function'

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        binwidth = 0.1  # mas function histogram bin width
      
        # calculate all
        w = np.where(G.StellarMass + G.ColdGas > 0.0)[0]
        mass = np.log10((G.StellarMass[w] + G.ColdGas[w]) * 1.0e10 * self.Hubble_h)

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = (ma - mi) / binwidth

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # Panter+ 2008 modified data used for the MCMC fitting
        Panter = np.array([
            [7.48980, 0.0327944,   0.0115254],
            [7.61225, 0.0748767,   0.0251771],
            [7.73469, 0.0756796,   0.0241978],
            [7.85714, 0.0411328,   0.0155465],
            [7.97959, 0.0754211,   0.0179399],
            [8.10204, 0.0629690,   0.0169421],
            [8.22449, 0.0632424,   0.0177860],
            [8.34694, 0.0629777,   0.0148224],
            [8.46939, 0.0617884,   0.00919052],
            [8.59184, 0.0488406,   0.00912303],
            [8.71429, 0.0373191,   0.00507392],
            [8.83673, 0.0367175,   0.00345630],
            [8.95918, 0.0326738,   0.00286759],
            [9.08163, 0.0401299,   0.00381169],
            [9.20408, 0.0380100,   0.00382040],
            [9.32653, 0.0347280,   0.00218399],
            [9.44898, 0.0296694,   0.00212078],
            [9.57143, 0.0273758,   0.00149546],
            [9.69388, 0.0247872,   0.00129821],
            [9.81633, 0.0242011,   0.00111468],
            [9.93878, 0.0218386,   0.000709138],
            [10.0612, 0.0195494,   0.000955437],
            [10.1837, 0.0179089,   0.000406569],
            [10.3061, 0.0165243,   0.000404797],
            [10.4286, 0.0150767,   0.000276546],
            [10.5510, 0.0132291,   0.000252791],
            [10.6735, 0.0111267,   0.000157898],
            [10.7959, 0.00863139,  0.000161130],
            [10.9184, 0.00597131,  0.000105420],
            [11.0408, 0.00382644,  9.81123e-05],
            [11.1633, 0.00217414,  7.44576e-05],
            [11.2857, 0.00105827,  4.42597e-05],
            [11.4082, 0.000470682, 2.40572e-05],
            [11.5306, 0.000176789, 8.92424e-06],
            [11.6531, 6.44065e-05, 6.11563e-06],
            [11.7755, 1.65784e-05, 1.46587e-06],
            [11.8980, 6.59601e-06, 1.04489e-06],
            [12.0204, 5.09457e-06, 2.26169e-06],
            ], dtype=np.float32)

        plt.yscale('log', nonposy='clip')

        # Finally plot the data
        # plt.errorbar(
        #     Panter[:, 0],
        #     Panter[:, 1],
        #     yerr=Panter[:, 2],
        #     color='g',
        #     lw=1.5,
        #     marker='o',
        #     ls='none',
        #     label='Panter et al. 2004',
        #     )

        plt.fill_between(Panter[:, 0], Panter[:, 1]+Panter[:, 2], Panter[:, 1]-Panter[:, 2], 
             facecolor='purple', alpha=0.25, label='Panter et al. 2004')
 
        # This next line is just to get the shaded region to appear correctly in the legend
        plt.plot(xaxeshisto, counts / self.volume / binwidth, label='Panter et al. 2004', color='purple', alpha=0.3)

        # Panter et al. 2004 BMF
        # M = np.arange(7.0, 13.0, 0.01)
        # Mstar = np.log10(7.53*1.0e10)
        # alpha = -1.17
        # phistar = 0.00777
        # xval = 10.0 ** (M-Mstar)
        # yval = np.log(10.) * phistar * xval ** (alpha+1) * np.exp(-xval)      
        # plt.plot(M, yval, 'g:', lw=1.5, label='Panter et al. 2004')  # Plot the SMF

        # Overplot the model histograms
        plt.plot(xaxeshisto, counts / self.volume / binwidth, 'k-', label='Model')

        plt.axis([8.5, 12.5, 2.0e-6, 2.0e-1])

        # Set the x-axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.1))

        plt.ylabel(r'$\phi(h^3\ \mathrm{Mpc}^{-3}\ \mathrm{dex}^{-1}$)')  # Set the y...
        plt.xlabel(r'$\log_{10}\ M_{\mathrm{bar}} (h^{-2}M_{\odot})$')  # and the x-axis labels

        leg = plt.legend(loc='lower left', numpoints=1,
                         labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')

        outputFile = OutputDir + '2.BaryonicMassFunction' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)


# ---------------------------------------------------------
    
    def BaryonicTullyFisher(self, G):
    
        print 'Plotting the baryonic TF relationship'
    
        seed(2222)
    
        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure
    
        # w = np.where((G.Type == 0) & (G.StellarMass + G.ColdGas > 0.0) & (G.Vmax > 0.0))[0]
        w = np.where((G.Type == 0) & (G.StellarMass + G.ColdGas > 0.0) & 
          (G.BulgeMass / G.StellarMass > 0.1) & (G.BulgeMass / G.StellarMass < 0.5))[0]
        if(len(w) > dilute): w = sample(w, dilute)
    
        mass = np.log10((G.StellarMass[w] + G.ColdGas[w]) * 1.0e10 / self.Hubble_h)
        vel = np.log10(G.Vmax[w])
                    
        plt.scatter(vel, mass, marker='o', s=1, c='k', alpha=0.5, label='Model Sb/c galaxies')
                
        # overplot McGaugh 2011
        w = np.arange(0.5, 10.0, 0.5)
        TF = 47.0 * (10.0 ** w) ** 4.0
        plt.plot(w, np.log10(TF), 'b-', lw=2.0, label='McGaugh 2011')
            
        plt.ylabel(r'$\log_{10}\ M_{\mathrm{bar}}\ (h^{-1}M_{\odot})$')  # Set the y...
        plt.xlabel(r'$\log_{10}V_{max}\ (km/s)$')  # and the x-axis labels
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
            
        plt.axis([1.4, 2.6, 8.0, 12.0])
            
        leg = plt.legend(loc='lower right')
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')
            
        outputFile = OutputDir + '3.BaryonicTullyFisher' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()
            
        # Add this plot to our output list
        OutputList.append(outputFile)


# ---------------------------------------------------------
    
    def SpecificStarFormationRate(self, G):
    
        print 'Plotting the specific SFR'
    
        seed(2222)
    
        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        w = np.where(G.StellarMass > 0.01)[0]
        if(len(w) > dilute): w = sample(w, dilute)
        
        mass = np.log10(G.StellarMass[w] * 1.0e10)
        sSFR = np.log10(G.Sfr[w] / (G.StellarMass[w] * 1.0e10))
                    
        plt.scatter(mass, sSFR, marker='o', s=1, c='k', alpha=0.5, label='Model')
                
        # overplot dividing line between SF and passive
        w = np.arange(7.0, 13.0, 1.0)
        plt.plot(w, w/w*(-11.0), 'b:', lw=2.0)
            
        plt.ylabel(r'$\log_{10}\ s\mathrm{SFR}\ (\mathrm{h\ yr^{-1}})$')  # Set the y...
        plt.xlabel(r'$\log_{10} M_{\mathrm{stars}}\ (h^{-1}M_{\odot})$')  # and the x-axis labels
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
            
        plt.axis([8.0, 12.0, -16.0, -8.0])
            
        leg = plt.legend(loc='lower right')
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')
            
        outputFile = OutputDir + '4.SpecificStarFormationRate' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()
            
        # Add this plot to our output list
        OutputList.append(outputFile)


# ---------------------------------------------------------

    def GasFraction(self, G):
    
        print 'Plotting the gas fractions'
    
        seed(2222)
    
        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        w = np.where((G.Type == 0) & (G.StellarMass + G.ColdGas > 0.0) & 
          (G.BulgeMass / G.StellarMass > 0.1) & (G.BulgeMass / G.StellarMass < 0.5))[0]
        if(len(w) > dilute): w = sample(w, dilute)
        
        mass = np.log10(G.StellarMass[w] * 1.0e10)
        fraction = G.ColdGas[w] / (G.StellarMass[w] + G.ColdGas[w])
                    
        plt.scatter(mass, fraction, marker='o', s=1, c='k', alpha=0.5, label='Model Sb/c galaxies')
            
        plt.ylabel(r'$\mathrm{Cold\ Mass\ /\ (Cold+Stellar\ Mass)}$')  # Set the y...
        plt.xlabel(r'$\log_{10} M_{\mathrm{stars}}\ (h^{-1}M_{\odot})$')  # and the x-axis labels
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
            
        plt.axis([8.0, 12.0, 0.0, 1.0])
            
        leg = plt.legend(loc='upper right')
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')
            
        outputFile = OutputDir + '5.GasFraction' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()
            
        # Add this plot to our output list
        OutputList.append(outputFile)


# ---------------------------------------------------------

    def Metallicity(self, G):
    
        print 'Plotting the metallicities'
    
        seed(2222)
    
        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        w = np.where((G.Type == 0) & (G.ColdGas / (G.StellarMass + G.ColdGas) > 0.1) & (G.StellarMass > 0.01))[0]
        if(len(w) > dilute): w = sample(w, dilute)
        
        mass = np.log10(G.StellarMass[w] * 1.0e10 / self.Hubble_h)
        Z = np.log10((G.MetalsColdGas[w] / G.ColdGas[w]) / 0.02) + 9.0
                    
        plt.scatter(mass, Z, marker='o', s=1, c='k', alpha=0.5, label='Model Sb/c galaxies')
            
        # overplot Tremonti et al. 2003
        w = np.arange(7.0, 13.0, 0.1)
        Zobs = -1.492 + 1.847*w - 0.08026*w*w
        plt.plot(w, Zobs, 'b-', lw=2.0, label='Tremonti et al. 2003')
            
        plt.ylabel(r'$12\ +\ \log_{10}[\mathrm{O/H}]$')  # Set the y...
        plt.xlabel(r'$\log_{10} M_{\mathrm{stars}}\ (h^{-1}M_{\odot})$')  # and the x-axis labels
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
            
        plt.axis([8.0, 12.0, 8.0, 9.5])
            
        leg = plt.legend(loc='lower right')
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')
            
        outputFile = OutputDir + '6.Metallicity' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()
            
        # Add this plot to our output list
        OutputList.append(outputFile)
    

# ---------------------------------------------------------

    def BlackHoleBulgeRelationship(self, G):
    
        print 'Plotting the black hole-bulge relationship'
    
        seed(2222)
    
        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure
    
        w = np.where((G.BulgeMass > 0.01) & (G.BlackHoleMass > 0.00001))[0]
        if(len(w) > dilute): w = sample(w, dilute)
    
        bh = np.log10(G.BlackHoleMass[w] * 1.0e10 / self.Hubble_h)
        bulge = np.log10(G.BulgeMass[w] * 1.0e10 / self.Hubble_h)
                    
        plt.scatter(bulge, bh, marker='o', s=1, c='k', alpha=0.5, label='Model')
                
        # overplot Haring & Rix 2004
        w = 10. ** np.arange(20)
        BHdata = 10. ** (8.2 + 1.12 * np.log10(w / 1.0e11))
        plt.plot(np.log10(w), np.log10(BHdata), 'b-', label="Haring \& Rix 2004")

        plt.ylabel(r'$\log\ m_{\mathrm{BH}}\ (M_{\odot})$')  # Set the y...
        plt.xlabel(r'$\log\ m_{\mathrm{bulge}}\ (M_{\odot})$')  # and the x-axis labels
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
            
        plt.axis([8.0, 12.0, 6.0, 10.0])
            
        leg = plt.legend(loc='upper left')
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize('medium')
            
        outputFile = OutputDir + '7.BlackHoleBulgeRelationship' + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print 'Saved file to', outputFile
        plt.close()
            
        # Add this plot to our output list
        OutputList.append(outputFile)


# ---------------------------------------------------------

    def MassReservoirScatter(self, G):
    
        print 'Plotting the mass in stellar, cold, hot, ejected, ICS reservoirs'
    
        seed(2222)
    
        plt.figure()  # New figure
        ax = plt.subplot(221)  # 4 plots on the figure
    
        w = np.where((G.Type == 0) & (G.Mvir > 1.0) & (G.StellarMass > 0.0))[0]
        if(len(w) > dilute): w = sample(w, dilute)
    
        # mvir = np.log10(G.Mvir[w] * 1.0e10)
        # plt.scatter(mvir, np.log10(G.StellarMass[w] * 1.0e10), marker='o', s=0.1, c='k', alpha=0.5, label='Stars')
        # plt.scatter(mvir, np.log10(G.ColdGas[w] * 1.0e10), marker='o', s=0.1, color='blue', alpha=0.5, label='Cold gas')
        # plt.scatter(mvir, np.log10(G.HotGas[w] * 1.0e10), marker='o', s=0.1, color='red', alpha=0.5, label='Hot gas')
        # plt.scatter(mvir, np.log10(G.EjectedMass[w] * 1.0e10), marker='o', s=0.1, color='green', alpha=0.5, label='Ejected gas')
        # plt.scatter(mvir, np.log10(G.IntraClusterStars[w] * 1.0e10), marker='o', s=10, color='yellow', alpha=0.5, label='Intracluster stars')    
        mvir = np.log10(G.Mvir[w] * 1.0e10)
        plt.scatter(mvir, np.log10(G.StellarMass[w] * 1.0e10), marker='o', s=0.1, c='k', alpha=0.5)
        plt.scatter(mvir, np.log10(G.ColdGas[w] * 1.0e10), marker='o', s=0.1, color='blue', alpha=0.5)
        plt.scatter(mvir, np.log10(G.HotGas[w] * 1.0e10), marker='o', s=0.1, color='red', alpha=0.5)
        plt.scatter(mvir, np.log10(G.EjectedMass[w] * 1.0e10), marker='o', s=0.1, color='green', alpha=0.5)
        plt.scatter(mvir, np.log10(G.IntraClusterStars[w] * 1.0e10), marker='o', s=2, color='yellow', alpha=0.5)

        plt.ylabel(r'$\mathrm{stellar,\ cold,\ hot,\ ejected,\ ICS\ mass}$', size='x-small')  # Set the y...
        plt.xlabel(r'$\log\ M_{\mathrm{vir}}\ (h^{-1}\ M_{\odot})$', size='x-small')  # and the x-axis labels
        
        plt.text(10.3, 11.8, r'$\mathrm{All}', size='x-small')
        
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
        for t in ax.xaxis.get_ticklabels(): t.set_fontsize('x-small')            
        for t in ax.yaxis.get_ticklabels(): t.set_fontsize('x-small')            

        plt.axis([10.0, 14.0, 7.5, 12.5])

        # leg = plt.legend(loc='upper left', labelspacing=0.0001)
        # leg.draw_frame(False)  # Don't want a box frame
        # for t in leg.get_texts():  # Reduce the size of the text
        #     t.set_fontsize('xx-small')


        ax = plt.subplot(222)  # 4 plots on the figure

        w = np.where((G.Type == 0) & (G.Mvir > 1.0) & (G.BulgeMass / G.StellarMass <= 0.1))[0]
        if(len(w) > dilute): w = sample(w, dilute)

        mvir = np.log10(G.Mvir[w] * 1.0e10)
        plt.scatter(mvir, np.log10(G.StellarMass[w] * 1.0e10), marker='o', s=0.1, c='k', alpha=0.5)
        plt.scatter(mvir, np.log10(G.ColdGas[w] * 1.0e10), marker='o', s=0.1, color='blue', alpha=0.5)
        plt.scatter(mvir, np.log10(G.HotGas[w] * 1.0e10), marker='o', s=0.1, color='red', alpha=0.5)
        plt.scatter(mvir, np.log10(G.EjectedMass[w] * 1.0e10), marker='o', s=0.1, color='green', alpha=0.5)
        plt.scatter(mvir, np.log10(G.IntraClusterStars[w] * 1.0e10), marker='o', s=2, color='yellow', alpha=0.5)

        plt.ylabel(r'$\mathrm{stellar,\ cold,\ hot,\ ejected,\ ICS\ mass}$', size='x-small')  # Set the y...
        plt.xlabel(r'$\log\ M_{\mathrm{vir}}\ (h^{-1}\ M_{\odot})$', size='x-small')  # and the x-axis labels
            
        plt.text(10.3, 11.8, r'$\mathrm{B/T} < 0.1$', size='x-small')
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
        for t in ax.xaxis.get_ticklabels(): t.set_fontsize('x-small')            
        for t in ax.yaxis.get_ticklabels(): t.set_fontsize('x-small')            
            
        plt.axis([10.0, 14.0, 7.5, 12.5])


        ax = plt.subplot(223)  # 4 plots on the figure

        w = np.where((G.Type == 0) & (G.Mvir > 1.0) & (G.BulgeMass / G.StellarMass > 0.1) & (G.BulgeMass / G.StellarMass <= 0.5))[0]
        if(len(w) > dilute): w = sample(w, dilute)
    
        mvir = np.log10(G.Mvir[w] * 1.0e10)
        plt.scatter(mvir, np.log10(G.StellarMass[w] * 1.0e10), marker='o', s=0.1, c='k', alpha=0.5)
        plt.scatter(mvir, np.log10(G.ColdGas[w] * 1.0e10), marker='o', s=0.1, color='blue', alpha=0.5)
        plt.scatter(mvir, np.log10(G.HotGas[w] * 1.0e10), marker='o', s=0.1, color='red', alpha=0.5)
        plt.scatter(mvir, np.log10(G.EjectedMass[w] * 1.0e10), marker='o', s=0.1, color='green', alpha=0.5)
        plt.scatter(mvir, np.log10(G.IntraClusterStars[w] * 1.0e10), marker='o', s=2, color='yellow', alpha=0.5)

        plt.ylabel(r'$\mathrm{stellar,\ cold,\ hot,\ ejected,\ ICS\ mass}$', size='x-small')  # Set the y...
        plt.xlabel(r'$\log\ M_{\mathrm{vir}}\ (h^{-1}\ M_{\odot})$', size='x-small')  # and the x-axis labels

        plt.text(10.3, 11.8, r'$0.1 < \mathrm{B/T} < 0.5$', size='x-small')
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
        for t in ax.xaxis.get_ticklabels(): t.set_fontsize('x-small')            
        for t in ax.yaxis.get_ticklabels(): t.set_fontsize('x-small')            
            
        plt.axis([10.0, 14.0, 7.5, 12.5])


        ax = plt.subplot(224)  # 4 plots on the figure

        w = np.where((G.Type == 0) & (G.Mvir > 1.0) & (G.BulgeMass / G.StellarMass > 0.5))[0]
        if(len(w) > dilute): w = sample(w, dilute)

        mvir = np.log10(G.Mvir[w] * 1.0e10)
        plt.scatter(mvir, np.log10(G.StellarMass[w] * 1.0e10), marker='o', s=0.1, c='k', alpha=0.5)
        plt.scatter(mvir, np.log10(G.ColdGas[w] * 1.0e10), marker='o', s=0.1, color='blue', alpha=0.5)
        plt.scatter(mvir, np.log10(G.HotGas[w] * 1.0e10), marker='o', s=0.1, color='red', alpha=0.5)
        plt.scatter(mvir, np.log10(G.EjectedMass[w] * 1.0e10), marker='o', s=0.1, color='green', alpha=0.5)
        plt.scatter(mvir, np.log10(G.IntraClusterStars[w] * 1.0e10), marker='o', s=2, color='yellow', alpha=0.5)

        plt.ylabel(r'$\mathrm{stellar,\ cold,\ hot,\ ejected,\ ICS\ mass}$', size='x-small')  # Set the y...
        plt.xlabel(r'$\log\ M_{\mathrm{vir}}\ (h^{-1}\ M_{\odot})$', size='x-small')  # and the x-axis labels

        plt.text(10.3, 11.8, r'$\mathrm{B/T} > 0.5$', size='x-small')
            
        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))
        for t in ax.xaxis.get_ticklabels(): t.set_fontsize('x-small')            
        for t in ax.yaxis.get_ticklabels(): t.set_fontsize('x-small')            
            
        plt.axis([10.0, 14.0, 7.5, 12.5])

            
        outputFile = OutputDir + '8.MassReservoirScatter' + OutputFormat
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
        default='./results/fullmillennium/',
        help='input directory name (default: ./results/fullmillennium/)',
        metavar='DIR',
        )
    parser.add_option(
        '-f',
        '--file_base',
        dest='FileName',
        default='model_z0.00',
        help='filename base (default: model_z0.00)',
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

    OutputDir = opt.DirName + '/plots/'

    if not os.path.exists(OutputDir):
        os.makedirs(OutputDir)

    res = Results()

    print 'Running allresults...'

    FirstFile = opt.FileRange[0]
    LastFile = opt.FileRange[1]

    fin_base = opt.DirName + opt.FileName
    G = res.read_gals(fin_base, FirstFile, LastFile)

    res.StellarMassFunction(G)
    res.BaryonicMassFunction(G)
    res.BaryonicTullyFisher(G)
    res.SpecificStarFormationRate(G)
    res.GasFraction(G)
    res.Metallicity(G)
    res.BlackHoleBulgeRelationship(G)
    res.MassReservoirScatter(G)


