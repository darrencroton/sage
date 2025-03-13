#!/usr/bin/env python

import matplotlib

matplotlib.use("Agg")

from os.path import getsize as getFileSize
from random import sample, seed

# import h5py as h5
import numpy as np
import pylab as plt

# ================================================================================
# Basic variables
# ================================================================================

# Set up some basic attributes of the run

whichsimulation = 0
whichimf = 1  # 0=Slapeter; 1=Chabrier
dilute = 7500  # Number of galaxies to plot in scatter plots
sSFRcut = -11.0  # Divide quiescent from star forming galaxies (when plotmags=0)


matplotlib.rcdefaults()
plt.rc("xtick", labelsize="x-large")
plt.rc("ytick", labelsize="x-large")
plt.rc("lines", linewidth="2.0")
plt.rc("legend", numpoints=1, fontsize="x-large")
plt.rc("text", usetex=True)

OutputDir = ""  # set in main below

OutputFormat = ".png"
TRANSPARENT = False

OutputList = []


class Results:
    """The following methods of this class generate the figures and plot them."""

    def __init__(self):
        """Here we set up some of the variables which will be global to this
        class."""

        if whichsimulation == 0:  # Mini-Millennium
            self.Hubble_h = 0.73
            self.BoxSize = 62.5  # Mpc/h
            self.MaxTreeFiles = 8  # FilesPerSnapshot

        elif whichsimulation == 1:  # Full Millennium
            self.Hubble_h = 0.73
            self.BoxSize = 500  # Mpc/h
            self.MaxTreeFiles = 512  # FilesPerSnapshot

        else:
            print("Please pick a valid simulation!")
            exit(1)

    def read_gals(self, model_name, first_file, last_file):

        # The input galaxy structure:
        Galdesc_full = [
            ("SnapNum", np.int32),
            ("Type", np.int32),
            ("GalaxyIndex", np.int64),
            ("CentralGalaxyIndex", np.int64),
            ("SAGEHaloIndex", np.int32),
            ("SAGETreeIndex", np.int32),
            ("SimulationHaloIndex", np.int64),
            ("mergeType", np.int32),
            ("mergeIntoID", np.int32),
            ("mergeIntoSnapNum", np.int32),
            ("dT", np.float32),
            ("Pos", (np.float32, 3)),
            ("Vel", (np.float32, 3)),
            ("Spin", (np.float32, 3)),
            ("Len", np.int32),
            ("Mvir", np.float32),
            ("CentralMvir", np.float32),
            ("Rvir", np.float32),
            ("Vvir", np.float32),
            ("Vmax", np.float32),
            ("VelDisp", np.float32),
            ("ColdGas", np.float32),
            ("StellarMass", np.float32),
            ("BulgeMass", np.float32),
            ("HotGas", np.float32),
            ("EjectedMass", np.float32),
            ("BlackHoleMass", np.float32),
            ("IntraClusterStars", np.float32),
            ("MetalsColdGas", np.float32),
            ("MetalsStellarMass", np.float32),
            ("MetalsBulgeMass", np.float32),
            ("MetalsHotGas", np.float32),
            ("MetalsEjectedMass", np.float32),
            ("MetalsIntraClusterStars", np.float32),
            ("SfrDisk", np.float32),
            ("SfrBulge", np.float32),
            ("SfrDiskZ", np.float32),
            ("SfrBulgeZ", np.float32),
            ("DiskRadius", np.float32),
            ("Cooling", np.float32),
            ("Heating", np.float32),
            ("QuasarModeBHaccretionMass", np.float32),
            ("TimeOfLastMajorMerger", np.float32),
            ("TimeOfLastMinorMerger", np.float32),
            ("OutflowRate", np.float32),
            ("infallMvir", np.float32),
            ("infallVvir", np.float32),
            ("infallVmax", np.float32),
        ]
        names = [Galdesc_full[i][0] for i in range(len(Galdesc_full))]
        formats = [Galdesc_full[i][1] for i in range(len(Galdesc_full))]
        Galdesc = np.dtype({"names": names, "formats": formats}, align=True)

        # Initialize variables.
        TotNTrees = 0
        TotNGals = 0
        FileIndexRanges = []

        print("Determining array storage requirements.")

        # Read each file and determine the total number of galaxies to be read in
        goodfiles = 0
        for fnr in range(first_file, last_file + 1):
            fname = model_name + "_" + str(fnr)  # Complete filename

            if not os.path.isfile(fname):
                # print("File\t%s  \tdoes not exist!  Skipping..." % (fname))
                continue

            if getFileSize(fname) == 0:
                print("File\t%s  \tis empty!  Skipping..." % (fname))
                continue

            fin = open(fname, "rb")  # Open the file
            Ntrees = np.fromfile(
                fin, np.dtype(np.int32), 1
            )  # Read number of trees in file
            NtotGals = np.fromfile(fin, np.dtype(np.int32), 1)[
                0
            ]  # Read number of gals in file.
            TotNTrees = TotNTrees + Ntrees  # Update total sim trees number
            TotNGals = TotNGals + NtotGals  # Update total sim gals number
            goodfiles = (
                goodfiles + 1
            )  # Update number of files read for volume calculation
            fin.close()

        print
        print("Input files contain:\t%d trees ;\t%d galaxies ." % (TotNTrees, TotNGals))
        print

        # Initialize the storage array
        G = np.empty(TotNGals, dtype=Galdesc)

        offset = 0  # Offset index for storage array

        # Open each file in turn and read in the preamble variables and structure.
        print("Reading in files.")
        for fnr in range(first_file, last_file + 1):
            fname = model_name + "_" + str(fnr)  # Complete filename

            if not os.path.isfile(fname):
                continue

            if getFileSize(fname) == 0:
                continue

            fin = open(fname, "rb")  # Open the file
            Ntrees = np.fromfile(
                fin, np.dtype(np.int32), 1
            )  # Read number of trees in file
            NtotGals = np.fromfile(fin, np.dtype(np.int32), 1)[
                0
            ]  # Read number of gals in file.
            GalsPerTree = np.fromfile(
                fin, np.dtype((np.int32, Ntrees)), 1
            )  # Read the number of gals in each tree
            print(":   Reading N=", NtotGals, "   \tgalaxies from file: ", fname)
            GG = np.fromfile(fin, Galdesc, NtotGals)  # Read in the galaxy structures

            FileIndexRanges.append((offset, offset + NtotGals))

            # Slice the file array into the global array
            # N.B. the copy() part is required otherwise we simply point to
            # the GG data which changes from file to file
            # NOTE THE WAY PYTHON WORKS WITH THESE INDICES!
            G[offset : offset + NtotGals] = GG[0:NtotGals].copy()

            del GG
            offset = (
                offset + NtotGals
            )  # Update the offset position for the global array

            fin.close()  # Close the file

        print
        print("Total galaxies considered:", TotNGals)

        # Convert the Galaxy array into a recarray
        G = G.view(np.recarray)

        w = np.where(G.StellarMass > 1.0)[0]
        print("Galaxies more massive than 10^10Msun/h:", len(w))

        print

        # Calculate the volume given the first_file and last_file
        self.volume = self.BoxSize**3.0 * goodfiles / self.MaxTreeFiles

        return G

    # --------------------------------------------------------

    def StellarMassFunction(self, G):

        print("Plotting the stellar mass function")

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        binwidth = 0.1  # mass function histogram bin width

        # calculate all
        w = np.where(G.StellarMass > 0.0)[0]
        mass = np.log10(G.StellarMass[w] * 1.0e10 / self.Hubble_h)
        sSFR = (G.SfrDisk[w] + G.SfrBulge[w]) / (
            G.StellarMass[w] * 1.0e10 / self.Hubble_h
        )

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = int((ma - mi) / binwidth)

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # additionally calculate red
        w = np.where(sSFR < 10.0**sSFRcut)[0]
        massRED = mass[w]
        (countsRED, binedges) = np.histogram(massRED, range=(mi, ma), bins=NB)

        # additionally calculate blue
        w = np.where(sSFR > 10.0**sSFRcut)[0]
        massBLU = mass[w]
        (countsBLU, binedges) = np.histogram(massBLU, range=(mi, ma), bins=NB)

        # Baldry+ 2008 modified data used for the MCMC fitting
        Baldry = np.array(
            [
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
            ],
            dtype=np.float32,
        )

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

        Baldry_xval = np.log10(10 ** Baldry[:, 0] / self.Hubble_h / self.Hubble_h)
        if whichimf == 1:
            Baldry_xval = Baldry_xval - 0.26  # convert back to Chabrier IMF
        Baldry_yvalU = (
            (Baldry[:, 1] + Baldry[:, 2])
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
        )
        Baldry_yvalL = (
            (Baldry[:, 1] - Baldry[:, 2])
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
        )

        plt.fill_between(
            Baldry_xval,
            Baldry_yvalU,
            Baldry_yvalL,
            facecolor="purple",
            alpha=0.25,
            label="Baldry et al. 2008 (z=0.1)",
        )

        # This next line is just to get the shaded region to appear correctly in the legend
        plt.plot(
            xaxeshisto,
            counts
            / self.volume
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
            / binwidth,
            label="Baldry et al. 2008",
            color="purple",
            alpha=0.3,
        )

        # # Cole et al. 2001 SMF (h=1.0 converted to h=0.73)
        # M = np.arange(7.0, 13.0, 0.01)
        # Mstar = np.log10(7.07*1.0e10 /self.Hubble_h/self.Hubble_h)
        # alpha = -1.18
        # phistar = 0.009 *self.Hubble_h*self.Hubble_h*self.Hubble_h
        # xval = 10.0 ** (M-Mstar)
        # yval = np.log(10.) * phistar * xval ** (alpha+1) * np.exp(-xval)
        # plt.plot(M, yval, 'g--', lw=1.5, label='Cole et al. 2001')  # Plot the SMF

        # Overplot the model histograms
        plt.plot(
            xaxeshisto,
            counts
            / self.volume
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
            / binwidth,
            "k-",
            label="Model - All",
        )
        plt.plot(
            xaxeshisto,
            countsRED
            / self.volume
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
            / binwidth,
            "r:",
            lw=2,
            label="Model - Red",
        )
        plt.plot(
            xaxeshisto,
            countsBLU
            / self.volume
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
            / binwidth,
            "b:",
            lw=2,
            label="Model - Blue",
        )

        plt.yscale("log")
        plt.axis([8.0, 12.5, 1.0e-6, 1.0e-1])

        # Set the x-axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.1))

        plt.ylabel(r"$\phi\ (\mathrm{Mpc}^{-3}\ \mathrm{dex}^{-1})$")  # Set the y...
        plt.xlabel(
            r"$\log_{10} M_{\mathrm{stars}}\ (M_{\odot})$"
        )  # and the x-axis labels

        plt.text(12.2, 0.03, whichsimulation, size="large")

        leg = plt.legend(loc="lower left", numpoints=1, labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "1.StellarMassFunction" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def BaryonicMassFunction(self, G):

        print("Plotting the baryonic mass function")

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        binwidth = 0.1  # mass function histogram bin width

        # calculate BMF
        w = np.where(G.StellarMass + G.ColdGas > 0.0)[0]
        mass = np.log10((G.StellarMass[w] + G.ColdGas[w]) * 1.0e10 / self.Hubble_h)

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = int((ma - mi) / binwidth)

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # Bell et al. 2003 BMF (h=1.0 converted to h=0.73)
        M = np.arange(7.0, 13.0, 0.01)
        Mstar = np.log10(5.3 * 1.0e10 / self.Hubble_h / self.Hubble_h)
        alpha = -1.21
        phistar = 0.0108 * self.Hubble_h * self.Hubble_h * self.Hubble_h
        xval = 10.0 ** (M - Mstar)
        yval = np.log(10.0) * phistar * xval ** (alpha + 1) * np.exp(-xval)

        if whichimf == 0:
            # converted diet Salpeter IMF to Salpeter IMF
            plt.plot(
                np.log10(10.0**M / 0.7), yval, "b-", lw=2.0, label="Bell et al. 2003"
            )  # Plot the SMF
        elif whichimf == 1:
            # converted diet Salpeter IMF to Salpeter IMF, then to Chabrier IMF
            plt.plot(
                np.log10(10.0**M / 0.7 / 1.8),
                yval,
                "g--",
                lw=1.5,
                label="Bell et al. 2003",
            )  # Plot the SMF

        # Overplot the model histograms
        plt.plot(
            xaxeshisto,
            counts
            / self.volume
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
            / binwidth,
            "k-",
            label="Model",
        )

        plt.yscale("log")
        plt.axis([8.0, 12.5, 1.0e-6, 1.0e-1])

        # Set the x-axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.1))

        plt.ylabel(r"$\phi\ (\mathrm{Mpc}^{-3}\ \mathrm{dex}^{-1})$")  # Set the y...
        plt.xlabel(
            r"$\log_{10}\ M_{\mathrm{bar}}\ (M_{\odot})$"
        )  # and the x-axis labels

        leg = plt.legend(loc="lower left", numpoints=1, labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "2.BaryonicMassFunction" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def GasMassFunction(self, G):

        print("Plotting the cold gas mass function")

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        binwidth = 0.1  # mass function histogram bin width

        # calculate all
        w = np.where(G.ColdGas > 0.0)[0]
        mass = np.log10(G.ColdGas[w] * 1.0e10 / self.Hubble_h)
        sSFR = (G.SfrDisk[w] + G.SfrBulge[w]) / (
            G.StellarMass[w] * 1.0e10 / self.Hubble_h
        )
        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = int((ma - mi) / binwidth)

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # additionally calculate red
        w = np.where(sSFR < 10.0**sSFRcut)[0]
        massRED = mass[w]
        (countsRED, binedges) = np.histogram(massRED, range=(mi, ma), bins=NB)

        # additionally calculate blue
        w = np.where(sSFR > 10.0**sSFRcut)[0]
        massBLU = mass[w]
        (countsBLU, binedges) = np.histogram(massBLU, range=(mi, ma), bins=NB)

        # Baldry+ 2008 modified data used for the MCMC fitting
        Zwaan = np.array(
            [
                [6.933, -0.333],
                [7.057, -0.490],
                [7.209, -0.698],
                [7.365, -0.667],
                [7.528, -0.823],
                [7.647, -0.958],
                [7.809, -0.917],
                [7.971, -0.948],
                [8.112, -0.927],
                [8.263, -0.917],
                [8.404, -1.062],
                [8.566, -1.177],
                [8.707, -1.177],
                [8.853, -1.312],
                [9.010, -1.344],
                [9.161, -1.448],
                [9.302, -1.604],
                [9.448, -1.792],
                [9.599, -2.021],
                [9.740, -2.406],
                [9.897, -2.615],
                [10.053, -3.031],
                [10.178, -3.677],
                [10.335, -4.448],
                [10.492, -5.083],
            ],
            dtype=np.float32,
        )

        ObrRaw = np.array(
            [
                [7.300, -1.104],
                [7.576, -1.302],
                [7.847, -1.250],
                [8.133, -1.240],
                [8.409, -1.344],
                [8.691, -1.479],
                [8.956, -1.792],
                [9.231, -2.271],
                [9.507, -3.198],
                [9.788, -5.062],
            ],
            dtype=np.float32,
        )

        ObrCold = np.array(
            [
                [8.009, -1.042],
                [8.215, -1.156],
                [8.409, -0.990],
                [8.604, -1.156],
                [8.799, -1.208],
                [9.020, -1.333],
                [9.194, -1.385],
                [9.404, -1.552],
                [9.599, -1.677],
                [9.788, -1.812],
                [9.999, -2.312],
                [10.172, -2.656],
                [10.362, -3.500],
                [10.551, -3.635],
                [10.740, -5.010],
            ],
            dtype=np.float32,
        )

        ObrCold_xval = np.log10(10 ** (ObrCold[:, 0]) / self.Hubble_h / self.Hubble_h)
        ObrCold_yval = (
            10 ** (ObrCold[:, 1]) * self.Hubble_h * self.Hubble_h * self.Hubble_h
        )
        Zwaan_xval = np.log10(10 ** (Zwaan[:, 0]) / self.Hubble_h / self.Hubble_h)
        Zwaan_yval = 10 ** (Zwaan[:, 1]) * self.Hubble_h * self.Hubble_h * self.Hubble_h
        ObrRaw_xval = np.log10(10 ** (ObrRaw[:, 0]) / self.Hubble_h / self.Hubble_h)
        ObrRaw_yval = (
            10 ** (ObrRaw[:, 1]) * self.Hubble_h * self.Hubble_h * self.Hubble_h
        )

        plt.plot(
            ObrCold_xval,
            ObrCold_yval,
            color="black",
            lw=7,
            alpha=0.25,
            label="Obr. \& Raw. 2009 (Cold Gas)",
        )
        plt.plot(
            Zwaan_xval,
            Zwaan_yval,
            color="cyan",
            lw=7,
            alpha=0.25,
            label="Zwaan et al. 2005 (HI)",
        )
        plt.plot(
            ObrRaw_xval,
            ObrRaw_yval,
            color="magenta",
            lw=7,
            alpha=0.25,
            label="Obr. \& Raw. 2009 (H2)",
        )

        # Overplot the model histograms
        plt.plot(
            xaxeshisto,
            counts
            / self.volume
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
            / binwidth,
            "k-",
            label="Model - Cold Gas",
        )

        plt.yscale("log")
        plt.axis([8.0, 11.5, 1.0e-6, 1.0e-1])

        # Set the x-axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.1))

        plt.ylabel(r"$\phi\ (\mathrm{Mpc}^{-3}\ \mathrm{dex}^{-1})$")  # Set the y...
        plt.xlabel(r"$\log_{10} M_{\mathrm{X}}\ (M_{\odot})$")  # and the x-axis labels

        leg = plt.legend(loc="lower left", numpoints=1, labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "3.GasMassFunction" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def BaryonicTullyFisher(self, G):

        print("Plotting the baryonic TF relationship")

        seed(2222)

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        # w = np.where((G.Type == 0) & (G.StellarMass + G.ColdGas > 0.0) & (G.Vmax > 0.0))[0]
        w = np.where(
            (G.Type == 0)
            & (G.StellarMass + G.ColdGas > 0.0)
            & (G.BulgeMass / G.StellarMass > 0.1)
            & (G.BulgeMass / G.StellarMass < 0.5)
        )[0]
        if len(w) > dilute:
            w = sample(list(w), dilute)

        mass = np.log10((G.StellarMass[w] + G.ColdGas[w]) * 1.0e10 / self.Hubble_h)
        vel = np.log10(G.Vmax[w])

        plt.scatter(
            vel, mass, marker="o", s=1, c="k", alpha=0.5, label="Model Sb/c galaxies"
        )

        # overplot Stark, McGaugh & Swatters 2009 (assumes h=0.75? ... what IMF?)
        w = np.arange(0.5, 10.0, 0.5)
        TF = 3.94 * w + 1.79
        plt.plot(w, TF, "b-", lw=2.0, label="Stark, McGaugh \& Swatters 2009")

        plt.ylabel(r"$\log_{10}\ M_{\mathrm{bar}}\ (M_{\odot})$")  # Set the y...
        plt.xlabel(r"$\log_{10}V_{max}\ (km/s)$")  # and the x-axis labels

        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))

        plt.axis([1.4, 2.6, 8.0, 12.0])

        leg = plt.legend(loc="lower right")
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "4.BaryonicTullyFisher" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def SpecificStarFormationRate(self, G):

        print("Plotting the specific SFR")

        seed(2222)

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        w = np.where(G.StellarMass > 0.01)[0]
        if len(w) > dilute:
            w = sample(list(w), dilute)

        mass = np.log10(G.StellarMass[w] * 1.0e10 / self.Hubble_h)
        sSFR = np.log10(
            (G.SfrDisk[w] + G.SfrBulge[w]) / (G.StellarMass[w] * 1.0e10 / self.Hubble_h)
        )
        plt.scatter(
            mass, sSFR, marker="o", s=1, c="k", alpha=0.5, label="Model galaxies"
        )

        # overplot dividing line between SF and passive
        w = np.arange(7.0, 13.0, 1.0)
        plt.plot(w, w / w * sSFRcut, "b:", lw=2.0)

        plt.ylabel(r"$\log_{10}\ s\mathrm{SFR}\ (\mathrm{yr^{-1}})$")  # Set the y...
        plt.xlabel(
            r"$\log_{10} M_{\mathrm{stars}}\ (M_{\odot})$"
        )  # and the x-axis labels

        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))

        plt.axis([8.0, 12.0, -16.0, -8.0])

        leg = plt.legend(loc="lower right")
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "5.SpecificStarFormationRate" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def GasFraction(self, G):

        print("Plotting the gas fractions")

        seed(2222)

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        w = np.where(
            (G.Type == 0)
            & (G.StellarMass + G.ColdGas > 0.0)
            & (G.BulgeMass / G.StellarMass > 0.1)
            & (G.BulgeMass / G.StellarMass < 0.5)
        )[0]
        if len(w) > dilute:
            w = sample(list(w), dilute)

        mass = np.log10(G.StellarMass[w] * 1.0e10 / self.Hubble_h)
        fraction = G.ColdGas[w] / (G.StellarMass[w] + G.ColdGas[w])

        plt.scatter(
            mass,
            fraction,
            marker="o",
            s=1,
            c="k",
            alpha=0.5,
            label="Model Sb/c galaxies",
        )

        plt.ylabel(r"$\mathrm{Cold\ Mass\ /\ (Cold+Stellar\ Mass)}$")  # Set the y...
        plt.xlabel(
            r"$\log_{10} M_{\mathrm{stars}}\ (M_{\odot})$"
        )  # and the x-axis labels

        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))

        plt.axis([8.0, 12.0, 0.0, 1.0])

        leg = plt.legend(loc="upper right")
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "6.GasFraction" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def Metallicity(self, G):

        print("Plotting the metallicities")

        seed(2222)

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        w = np.where(
            (G.Type == 0)
            & (G.ColdGas / (G.StellarMass + G.ColdGas) > 0.1)
            & (G.StellarMass > 0.01)
        )[0]
        if len(w) > dilute:
            w = sample(list(w), dilute)

        mass = np.log10(G.StellarMass[w] * 1.0e10 / self.Hubble_h)
        Z = np.log10((G.MetalsColdGas[w] / G.ColdGas[w]) / 0.02) + 9.0

        plt.scatter(mass, Z, marker="o", s=1, c="k", alpha=0.5, label="Model galaxies")

        # overplot Tremonti et al. 2003 (h=0.7)
        w = np.arange(7.0, 13.0, 0.1)
        Zobs = -1.492 + 1.847 * w - 0.08026 * w * w
        if whichimf == 0:
            # Conversion from Kroupa IMF to Slapeter IMF
            plt.plot(
                np.log10((10**w * 1.5)),
                Zobs,
                "b-",
                lw=2.0,
                label="Tremonti et al. 2003",
            )
        elif whichimf == 1:
            # Conversion from Kroupa IMF to Slapeter IMF to Chabrier IMF
            plt.plot(
                np.log10((10**w * 1.5 / 1.8)),
                Zobs,
                "b-",
                lw=2.0,
                label="Tremonti et al. 2003",
            )

        plt.ylabel(r"$12\ +\ \log_{10}[\mathrm{O/H}]$")  # Set the y...
        plt.xlabel(
            r"$\log_{10} M_{\mathrm{stars}}\ (M_{\odot})$"
        )  # and the x-axis labels

        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))

        plt.axis([8.0, 12.0, 8.0, 9.5])

        leg = plt.legend(loc="lower right")
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "7.Metallicity" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def BlackHoleBulgeRelationship(self, G):

        print("Plotting the black hole-bulge relationship")

        seed(2222)

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        w = np.where((G.BulgeMass > 0.01) & (G.BlackHoleMass > 0.00001))[0]
        if len(w) > dilute:
            w = sample(list(w), dilute)

        bh = np.log10(G.BlackHoleMass[w] * 1.0e10 / self.Hubble_h)
        bulge = np.log10(G.BulgeMass[w] * 1.0e10 / self.Hubble_h)

        plt.scatter(
            bulge, bh, marker="o", s=1, c="k", alpha=0.5, label="Model galaxies"
        )

        # overplot Haring & Rix 2004
        w = 10.0 ** np.arange(20)
        BHdata = 10.0 ** (8.2 + 1.12 * np.log10(w / 1.0e11))
        plt.plot(np.log10(w), np.log10(BHdata), "b-", label="Haring \& Rix 2004")

        plt.ylabel(r"$\log\ M_{\mathrm{BH}}\ (M_{\odot})$")  # Set the y...
        plt.xlabel(r"$\log\ M_{\mathrm{bulge}}\ (M_{\odot})$")  # and the x-axis labels

        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))

        plt.axis([8.0, 12.0, 6.0, 10.0])

        leg = plt.legend(loc="upper left")
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "8.BlackHoleBulgeRelationship" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def QuiescentFraction(self, G):

        print("Plotting the quiescent fraction vs stellar mass")

        seed(2222)

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        groupscale = 12.5

        w = np.where(G.StellarMass > 0.0)[0]
        StellarMass = np.log10(G.StellarMass[w] * 1.0e10 / self.Hubble_h)
        CentralMvir = np.log10(G.CentralMvir[w] * 1.0e10 / self.Hubble_h)
        Type = G.Type[w]
        sSFR = (G.SfrDisk[w] + G.SfrBulge[w]) / (
            G.StellarMass[w] * 1.0e10 / self.Hubble_h
        )

        MinRange = 9.5
        MaxRange = 12.0
        Interval = 0.1
        Nbins = int((MaxRange - MinRange) / Interval)
        Range = np.arange(MinRange, MaxRange, Interval)

        Mass = []
        Fraction = []
        CentralFraction = []
        SatelliteFraction = []
        SatelliteFractionLo = []
        SatelliteFractionHi = []

        for i in range(Nbins - 1):

            w = np.where((StellarMass >= Range[i]) & (StellarMass < Range[i + 1]))[0]
            if len(w) > 0:
                wQ = np.where(
                    (StellarMass >= Range[i])
                    & (StellarMass < Range[i + 1])
                    & (sSFR < 10.0**sSFRcut)
                )[0]
                Fraction.append(1.0 * len(wQ) / len(w))
            else:
                Fraction.append(0.0)

            w = np.where(
                (Type == 0) & (StellarMass >= Range[i]) & (StellarMass < Range[i + 1])
            )[0]
            if len(w) > 0:
                wQ = np.where(
                    (Type == 0)
                    & (StellarMass >= Range[i])
                    & (StellarMass < Range[i + 1])
                    & (sSFR < 10.0**sSFRcut)
                )[0]
                CentralFraction.append(1.0 * len(wQ) / len(w))
            else:
                CentralFraction.append(0.0)

            w = np.where(
                (Type == 1) & (StellarMass >= Range[i]) & (StellarMass < Range[i + 1])
            )[0]
            if len(w) > 0:
                wQ = np.where(
                    (Type == 1)
                    & (StellarMass >= Range[i])
                    & (StellarMass < Range[i + 1])
                    & (sSFR < 10.0**sSFRcut)
                )[0]
                SatelliteFraction.append(1.0 * len(wQ) / len(w))
                wQ = np.where(
                    (Type == 1)
                    & (StellarMass >= Range[i])
                    & (StellarMass < Range[i + 1])
                    & (sSFR < 10.0**sSFRcut)
                    & (CentralMvir < groupscale)
                )[0]
                SatelliteFractionLo.append(1.0 * len(wQ) / len(w))
                wQ = np.where(
                    (Type == 1)
                    & (StellarMass >= Range[i])
                    & (StellarMass < Range[i + 1])
                    & (sSFR < 10.0**sSFRcut)
                    & (CentralMvir > groupscale)
                )[0]
                SatelliteFractionHi.append(1.0 * len(wQ) / len(w))
            else:
                SatelliteFraction.append(0.0)
                SatelliteFractionLo.append(0.0)
                SatelliteFractionHi.append(0.0)

            Mass.append((Range[i] + Range[i + 1]) / 2.0)
            # print ('  ', Mass[i], Fraction[i], CentralFraction[i], SatelliteFraction[i])

        Mass = np.array(Mass)
        Fraction = np.array(Fraction)
        CentralFraction = np.array(CentralFraction)
        SatelliteFraction = np.array(SatelliteFraction)
        SatelliteFractionLo = np.array(SatelliteFractionLo)
        SatelliteFractionHi = np.array(SatelliteFractionHi)

        w = np.where(Fraction > 0)[0]
        plt.plot(Mass[w], Fraction[w], label="All")

        w = np.where(CentralFraction > 0)[0]
        plt.plot(Mass[w], CentralFraction[w], color="Blue", label="Centrals")

        w = np.where(SatelliteFraction > 0)[0]
        plt.plot(Mass[w], SatelliteFraction[w], color="Red", label="Satellites")

        w = np.where(SatelliteFractionLo > 0)[0]
        plt.plot(Mass[w], SatelliteFractionLo[w], "r--", label="Satellites-Lo")

        w = np.where(SatelliteFractionHi > 0)[0]
        plt.plot(Mass[w], SatelliteFractionHi[w], "r-.", label="Satellites-Hi")

        plt.xlabel(
            r"$\log_{10} M_{\mathrm{stellar}}\ (M_{\odot})$"
        )  # Set the x-axis label
        plt.ylabel(r"$\mathrm{Quescient\ Fraction}$")  # Set the y-axis label

        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))

        plt.axis([9.5, 12.0, 0.0, 1.05])

        leg = plt.legend(loc="lower right")
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "9.QuiescentFraction" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # --------------------------------------------------------

    def BulgeMassFraction(self, G):

        print("Plotting the mass fraction of galaxies")

        seed(2222)

        fBulge = G.BulgeMass / G.StellarMass
        fDisk = 1.0 - (G.BulgeMass) / G.StellarMass
        mass = np.log10(G.StellarMass * 1.0e10 / self.Hubble_h)
        sSFR = np.log10(
            (G.SfrDisk + G.SfrBulge) / (G.StellarMass * 1.0e10 / self.Hubble_h)
        )

        binwidth = 0.2
        shift = binwidth / 2.0
        mass_range = np.arange(8.5 - shift, 12.0 + shift, binwidth)
        bins = len(mass_range)

        fBulge_ave = np.zeros(bins)
        fBulge_var = np.zeros(bins)
        fDisk_ave = np.zeros(bins)
        fDisk_var = np.zeros(bins)

        for i in range(bins - 1):
            w = np.where((mass >= mass_range[i]) & (mass < mass_range[i + 1]))[0]
            # w = np.where( (mass >= mass_range[i]) & (mass < mass_range[i+1]) & (sSFR < sSFRcut))[0]
            if len(w) > 0:
                fBulge_ave[i] = np.mean(fBulge[w])
                fBulge_var[i] = np.var(fBulge[w])
                fDisk_ave[i] = np.mean(fDisk[w])
                fDisk_var[i] = np.var(fDisk[w])

        w = np.where(fBulge_ave > 0.0)[0]
        plt.plot(mass_range[w] + shift, fBulge_ave[w], "r-", label="bulge")
        plt.fill_between(
            mass_range[w] + shift,
            fBulge_ave[w] + fBulge_var[w],
            fBulge_ave[w] - fBulge_var[w],
            facecolor="red",
            alpha=0.25,
        )

        w = np.where(fDisk_ave > 0.0)[0]
        plt.plot(mass_range[w] + shift, fDisk_ave[w], "k-", label="disk stars")
        plt.fill_between(
            mass_range[w] + shift,
            fDisk_ave[w] + fDisk_var[w],
            fDisk_ave[w] - fDisk_var[w],
            facecolor="black",
            alpha=0.25,
        )

        plt.axis([mass_range[0], mass_range[bins - 1], 0.0, 1.05])

        plt.ylabel(r"$\mathrm{Stellar\ Mass\ Fraction}$")  # Set the y...
        plt.xlabel(
            r"$\log_{10} M_{\mathrm{stars}}\ (M_{\odot})$"
        )  # and the x-axis labels

        leg = plt.legend(loc="upper right", numpoints=1, labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "10.BulgeMassFraction" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def BaryonFraction(self, G):

        print("Plotting the average baryon fraction vs halo mass")

        seed(2222)

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        HaloMass = np.log10(G.Mvir * 1.0e10 / self.Hubble_h)
        Baryons = (
            G.StellarMass
            + G.ColdGas
            + G.HotGas
            + G.EjectedMass
            + G.IntraClusterStars
            + G.BlackHoleMass
        )

        MinHalo = 11.0
        MaxHalo = 16.0
        Interval = 0.1
        Nbins = int((MaxHalo - MinHalo) / Interval)
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

        for i in range(Nbins - 1):

            w1 = np.where(
                (G.Type == 0)
                & (HaloMass >= HaloRange[i])
                & (HaloMass < HaloRange[i + 1])
            )[0]
            HalosFound = len(w1)

            if HalosFound > 2:

                BaryonFraction = []
                CentralHaloMass = []

                Stars = []
                Cold = []
                Hot = []
                Ejected = []
                ICS = []
                BH = []

                for j in range(HalosFound):

                    w2 = np.where(G.CentralGalaxyIndex == G.CentralGalaxyIndex[w1[j]])[
                        0
                    ]
                    CentralAndSatellitesFound = len(w2)

                    if CentralAndSatellitesFound > 0:
                        BaryonFraction.append(sum(Baryons[w2]) / G.Mvir[w1[j]])
                        CentralHaloMass.append(
                            np.log10(G.Mvir[w1[j]] * 1.0e10 / self.Hubble_h)
                        )

                        Stars.append(sum(G.StellarMass[w2]) / G.Mvir[w1[j]])
                        Cold.append(sum(G.ColdGas[w2]) / G.Mvir[w1[j]])
                        Hot.append(sum(G.HotGas[w2]) / G.Mvir[w1[j]])
                        Ejected.append(sum(G.EjectedMass[w2]) / G.Mvir[w1[j]])
                        ICS.append(sum(G.IntraClusterStars[w2]) / G.Mvir[w1[j]])
                        BH.append(sum(G.BlackHoleMass[w2]) / G.Mvir[w1[j]])

                MeanCentralHaloMass.append(np.mean(CentralHaloMass))
                MeanBaryonFraction.append(np.mean(BaryonFraction))
                MeanBaryonFractionU.append(
                    np.mean(BaryonFraction) + np.var(BaryonFraction)
                )
                MeanBaryonFractionL.append(
                    np.mean(BaryonFraction) - np.var(BaryonFraction)
                )

                MeanStars.append(np.mean(Stars))
                MeanCold.append(np.mean(Cold))
                MeanHot.append(np.mean(Hot))
                MeanEjected.append(np.mean(Ejected))
                MeanICS.append(np.mean(ICS))
                MeanBH.append(np.mean(BH))

                print("  ", i, HaloRange[i], HalosFound, np.mean(BaryonFraction))

        plt.plot(
            MeanCentralHaloMass, MeanBaryonFraction, "k-", label="TOTAL"
        )  # , color='purple', alpha=0.3)
        plt.fill_between(
            MeanCentralHaloMass,
            MeanBaryonFractionU,
            MeanBaryonFractionL,
            facecolor="purple",
            alpha=0.25,
            label="TOTAL",
        )

        plt.plot(MeanCentralHaloMass, MeanStars, "k--", label="Stars")
        plt.plot(MeanCentralHaloMass, MeanCold, label="Cold", color="blue")
        plt.plot(MeanCentralHaloMass, MeanHot, label="Hot", color="red")
        plt.plot(MeanCentralHaloMass, MeanEjected, label="Ejected", color="green")
        plt.plot(MeanCentralHaloMass, MeanICS, label="ICS", color="yellow")
        # plt.plot(MeanCentralHaloMass, MeanBH, 'k:', label='BH')

        plt.xlabel(
            r"$\mathrm{Central}\ \log_{10} M_{\mathrm{vir}}\ (M_{\odot})$"
        )  # Set the x-axis label
        plt.ylabel(r"$\mathrm{Baryon\ Fraction}$")  # Set the y-axis label

        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.05))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.25))

        plt.axis([10.8, 15.0, 0.0, 0.23])

        leg = plt.legend(bbox_to_anchor=[0.99, 0.6])
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "11.BaryonFraction" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # --------------------------------------------------------

    def SpinDistribution(self, G):

        print("Plotting the spin distribution of all galaxies")

        # set up figure
        plt.figure()
        ax = plt.subplot(111)

        SpinParameter = np.sqrt(
            G.Spin[:, 0] * G.Spin[:, 0]
            + G.Spin[:, 1] * G.Spin[:, 1]
            + G.Spin[:, 2] * G.Spin[:, 2]
        ) / (np.sqrt(2) * G.Vvir * G.Rvir)

        mi = -0.02
        ma = 0.5
        binwidth = 0.01
        NB = int((ma - mi) / binwidth)

        (counts, binedges) = np.histogram(SpinParameter, range=(mi, ma), bins=NB)
        xaxeshisto = binedges[:-1] + 0.5 * binwidth
        plt.plot(xaxeshisto, counts, "k-", label="simulation")

        plt.axis([mi, ma, 0.0, max(counts) * 1.15])

        plt.ylabel(r"$\mathrm{Number}$")  # Set the y...
        plt.xlabel(r"$\mathrm{Spin\ Parameter}$")  # and the x-axis labels

        leg = plt.legend(loc="upper right", numpoints=1, labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "12.SpinDistribution" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # --------------------------------------------------------

    def VelocityDistribution(self, G):

        print("Plotting the velocity distribution of all galaxies")

        seed(2222)

        mi = -40.0
        ma = 40.0
        binwidth = 0.5
        NB = int((ma - mi) / binwidth)

        # set up figure
        plt.figure()
        ax = plt.subplot(111)

        pos_x = G.Pos[:, 0] / self.Hubble_h
        pos_y = G.Pos[:, 1] / self.Hubble_h
        pos_z = G.Pos[:, 2] / self.Hubble_h

        vel_x = G.Vel[:, 0]
        vel_y = G.Vel[:, 1]
        vel_z = G.Vel[:, 2]

        dist_los = np.sqrt(pos_x * pos_x + pos_y * pos_y + pos_z * pos_z)
        vel_los = (
            (pos_x / dist_los) * vel_x
            + (pos_y / dist_los) * vel_y
            + (pos_z / dist_los) * vel_z
        )
        dist_red = dist_los + vel_los / (self.Hubble_h * 100.0)

        tot_gals = len(pos_x)

        (counts, binedges) = np.histogram(
            vel_los / (self.Hubble_h * 100.0), range=(mi, ma), bins=NB
        )
        xaxeshisto = binedges[:-1] + 0.5 * binwidth
        plt.plot(xaxeshisto, counts / binwidth / tot_gals, "k-", label="los-velocity")

        (counts, binedges) = np.histogram(
            vel_x / (self.Hubble_h * 100.0), range=(mi, ma), bins=NB
        )
        xaxeshisto = binedges[:-1] + 0.5 * binwidth
        plt.plot(xaxeshisto, counts / binwidth / tot_gals, "r-", label="x-velocity")

        (counts, binedges) = np.histogram(
            vel_y / (self.Hubble_h * 100.0), range=(mi, ma), bins=NB
        )
        xaxeshisto = binedges[:-1] + 0.5 * binwidth
        plt.plot(xaxeshisto, counts / binwidth / tot_gals, "g-", label="y-velocity")

        (counts, binedges) = np.histogram(
            vel_z / (self.Hubble_h * 100.0), range=(mi, ma), bins=NB
        )
        xaxeshisto = binedges[:-1] + 0.5 * binwidth
        plt.plot(xaxeshisto, counts / binwidth / tot_gals, "b-", label="z-velocity")

        plt.yscale("log")
        plt.axis([mi, ma, 1e-5, 0.5])
        # plt.axis([mi, ma, 0, 0.13])

        plt.ylabel(r"$\mathrm{Box\ Normalised\ Count}$")  # Set the y...
        plt.xlabel(r"$\mathrm{Velocity / H}_{0}$")  # and the x-axis labels

        leg = plt.legend(loc="upper left", numpoints=1, labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "13.VelocityDistribution" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # --------------------------------------------------------

    def MassReservoirScatter(self, G):

        print("Plotting the mass in stellar, cold, hot, ejected, ICS reservoirs")

        seed(2222)

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        w = np.where((G.Type == 0) & (G.Mvir > 1.0) & (G.StellarMass > 0.0))[0]
        if len(w) > dilute:
            w = sample(list(w), dilute)

        mvir = np.log10(G.Mvir[w] * 1.0e10)
        plt.scatter(
            mvir,
            np.log10(G.StellarMass[w] * 1.0e10),
            marker="o",
            s=0.3,
            c="k",
            alpha=0.5,
            label="Stars",
        )
        plt.scatter(
            mvir,
            np.log10(G.ColdGas[w] * 1.0e10),
            marker="o",
            s=0.3,
            color="blue",
            alpha=0.5,
            label="Cold gas",
        )
        plt.scatter(
            mvir,
            np.log10(G.HotGas[w] * 1.0e10),
            marker="o",
            s=0.3,
            color="red",
            alpha=0.5,
            label="Hot gas",
        )
        plt.scatter(
            mvir,
            np.log10(G.EjectedMass[w] * 1.0e10),
            marker="o",
            s=0.3,
            color="green",
            alpha=0.5,
            label="Ejected gas",
        )
        plt.scatter(
            mvir,
            np.log10(G.IntraClusterStars[w] * 1.0e10),
            marker="o",
            s=10,
            color="yellow",
            alpha=0.5,
            label="Intracluster stars",
        )

        plt.ylabel(
            r"$\mathrm{stellar,\ cold,\ hot,\ ejected,\ ICS\ mass}$"
        )  # Set the y...
        plt.xlabel(
            r"$\log\ M_{\mathrm{vir}}\ (h^{-1}\ M_{\odot})$"
        )  # and the x-axis labels

        plt.axis([10.0, 14.0, 7.5, 12.5])

        leg = plt.legend(loc="upper left")
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        plt.text(13.5, 8.0, r"$\mathrm{All}$")

        outputFile = OutputDir + "14.MassReservoirScatter" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # --------------------------------------------------------

    def SpatialDistribution(self, G):

        print("Plotting the spatial distribution of all galaxies")

        seed(2222)

        plt.figure()  # New figure

        w = np.where((G.Mvir > 0.0) & (G.StellarMass > 0.1))[0]
        if len(w) > dilute:
            w = sample(list(w), dilute)

        xx = G.Pos[w, 0]
        yy = G.Pos[w, 1]
        zz = G.Pos[w, 2]

        buff = self.BoxSize * 0.1

        ax = plt.subplot(221)  # 1 plot on the figure
        plt.scatter(xx, yy, marker="o", s=0.3, c="k", alpha=0.5)
        plt.axis([0.0 - buff, self.BoxSize + buff, 0.0 - buff, self.BoxSize + buff])

        plt.ylabel(r"$\mathrm{x}$")  # Set the y...
        plt.xlabel(r"$\mathrm{y}$")  # and the x-axis labels

        ax = plt.subplot(222)  # 1 plot on the figure
        plt.scatter(xx, zz, marker="o", s=0.3, c="k", alpha=0.5)
        plt.axis([0.0 - buff, self.BoxSize + buff, 0.0 - buff, self.BoxSize + buff])

        plt.ylabel(r"$\mathrm{x}$")  # Set the y...
        plt.xlabel(r"$\mathrm{z}$")  # and the x-axis labels

        ax = plt.subplot(223)  # 1 plot on the figure
        plt.scatter(yy, zz, marker="o", s=0.3, c="k", alpha=0.5)
        plt.axis([0.0 - buff, self.BoxSize + buff, 0.0 - buff, self.BoxSize + buff])
        plt.ylabel(r"$\mathrm{y}$")  # Set the y...
        plt.xlabel(r"$\mathrm{z}$")  # and the x-axis labels

        outputFile = OutputDir + "15.SpatialDistribution" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)


# =================================================================


#  'Main' section of code.  This if statement executes if the code is run from the
#   shell command line, i.e. with 'python allresults.py'

if __name__ == "__main__":

    import os
    from optparse import OptionParser

    parser = OptionParser()
    parser.add_option(
        "-d",
        "--dir_name",
        dest="DirName",
        default="./millennium/",
        help="input directory name (default: ./millennium/)",
        metavar="DIR",
    )
    parser.add_option(
        "-f",
        "--file_base",
        dest="FileName",
        default="model_z0.000",
        help="filename base (default: model_z0.000)",
        metavar="FILE",
    )
    parser.add_option(
        "-n",
        "--file_range",
        type="int",
        nargs=2,
        dest="FileRange",
        default=(0, 7),
        help="first and last filenumbers (default: 0 7)",
        metavar="FIRST LAST",
    )

    (opt, args) = parser.parse_args()

    if opt.DirName[-1] != "/":
        opt.DirName += "/"

    OutputDir = opt.DirName + "plots/"

    if not os.path.exists(OutputDir):
        os.makedirs(OutputDir)

    res = Results()

    print("Running allresults...")

    FirstFile = opt.FileRange[0]
    LastFile = opt.FileRange[1]

    fin_base = opt.DirName + opt.FileName
    G = res.read_gals(fin_base, FirstFile, LastFile)

    res.StellarMassFunction(G)
    res.BaryonicMassFunction(G)
    res.GasMassFunction(G)
    res.BaryonicTullyFisher(G)
    res.SpecificStarFormationRate(G)
    res.GasFraction(G)
    res.Metallicity(G)
    res.BlackHoleBulgeRelationship(G)
    res.QuiescentFraction(G)
    res.BulgeMassFraction(G)
    res.BaryonFraction(G)
    res.SpinDistribution(G)
    res.VelocityDistribution(G)
    res.MassReservoirScatter(G)
    res.SpatialDistribution(G)
