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


matplotlib.rcdefaults()
# plt.rc('axes', color_cycle=[
#     'k',
#     'b',
#     'r',
#     'g',
#     'm',
#     '0.5',
#     ], labelsize='x-large')
plt.rc("xtick", labelsize="x-large")
plt.rc("ytick", labelsize="x-large")
plt.rc("lines", linewidth="2.0")
# plt.rc('font', variant='monospace')
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

        if whichsimulation == 0 or whichsimulation == 1:

            self.SMFsnaps = [63, 37, 32, 27, 23, 20, 18, 16]

            self.redshift_file = [
                "_z127.000",
                "_z79.998",
                "_z50.000",
                "_z30.000",
                "_z19.916",
                "_z18.244",
                "_z16.725",
                "_z15.343",
                "_z14.086",
                "_z12.941",
                "_z11.897",
                "_z10.944",
                "_z10.073",
                "_z9.278",
                "_z8.550",
                "_z7.883",
                "_z7.272",
                "_z6.712",
                "_z6.197",
                "_z5.724",
                "_z5.289",
                "_z4.888",
                "_z4.520",
                "_z4.179",
                "_z3.866",
                "_z3.576",
                "_z3.308",
                "_z3.060",
                "_z2.831",
                "_z2.619",
                "_z2.422",
                "_z2.239",
                "_z2.070",
                "_z1.913",
                "_z1.766",
                "_z1.630",
                "_z1.504",
                "_z1.386",
                "_z1.276",
                "_z1.173",
                "_z1.078",
                "_z0.989",
                "_z0.905",
                "_z0.828",
                "_z0.755",
                "_z0.687",
                "_z0.624",
                "_z0.564",
                "_z0.509",
                "_z0.457",
                "_z0.408",
                "_z0.362",
                "_z0.320",
                "_z0.280",
                "_z0.242",
                "_z0.208",
                "_z0.175",
                "_z0.144",
                "_z0.116",
                "_z0.089",
                "_z0.064",
                "_z0.041",
                "_z0.020",
                "_z0.000",
            ]

            self.redshift = [
                127.000,
                79.998,
                50.000,
                30.000,
                19.916,
                18.244,
                16.725,
                15.343,
                14.086,
                12.941,
                11.897,
                10.944,
                10.073,
                9.278,
                8.550,
                7.883,
                7.272,
                6.712,
                6.197,
                5.724,
                5.289,
                4.888,
                4.520,
                4.179,
                3.866,
                3.576,
                3.308,
                3.060,
                2.831,
                2.619,
                2.422,
                2.239,
                2.070,
                1.913,
                1.766,
                1.630,
                1.504,
                1.386,
                1.276,
                1.173,
                1.078,
                0.989,
                0.905,
                0.828,
                0.755,
                0.687,
                0.624,
                0.564,
                0.509,
                0.457,
                0.408,
                0.362,
                0.320,
                0.280,
                0.242,
                0.208,
                0.175,
                0.144,
                0.116,
                0.089,
                0.064,
                0.041,
                0.020,
                0.000,
            ]

    def read_gals(self, model_name, first_file, last_file, thissnap):

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
        goodfiles = 0

        if thissnap in self.SMFsnaps:

            print
            print("Determining array storage requirements.")

            # Read each file and determine the total number of galaxies to be read in
            for fnr in range(first_file, last_file + 1):
                fname = model_name + "_" + str(fnr)  # Complete filename

                if not os.path.isfile(fname):
                    # print ("File\t%s  \tdoes not exist!  Skipping..." % (fname))
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

            print(
                "Input files contain:\t%d trees ;\t%d galaxies ."
                % (TotNTrees, TotNGals)
            )

        # Initialize the storage array
        G = np.empty(TotNGals, dtype=Galdesc)

        if thissnap in self.SMFsnaps:

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
                GG = np.fromfile(
                    fin, Galdesc, NtotGals
                )  # Read in the galaxy structures

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

            print("Total galaxies considered:", TotNGals)
            print

        # Convert the Galaxy array into a recarray
        G = G.view(np.recarray)

        # Calculate the volume given the first_file and last_file
        self.volume = self.BoxSize**3.0 * goodfiles / self.MaxTreeFiles

        return G

    # --------------------------------------------------------

    def StellarMassFunction(self, G_history):

        print("Plotting the stellar mass function")

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        binwidth = 0.1  # mass function histogram bin width

        # Marchesini et al. 2009ApJ...701.1765M SMF, h=0.7
        M = np.arange(7.0, 11.8, 0.01)
        Mstar = np.log10(10.0**10.96)
        alpha = -1.18
        phistar = 30.87 * 1e-4
        xval = 10.0 ** (M - Mstar)
        yval = np.log(10.0) * phistar * xval ** (alpha + 1) * np.exp(-xval)
        if whichimf == 0:
            plt.plot(
                np.log10(10.0**M * 1.6),
                yval,
                ":",
                lw=10,
                alpha=0.5,
                label="Marchesini et al. 2009 z=[0.1]",
            )
        elif whichimf == 1:
            plt.plot(
                np.log10(10.0**M * 1.6 / 1.8),
                yval,
                ":",
                lw=10,
                alpha=0.5,
                label="Marchesini et al. 2009 z=[0.1]",
            )

        M = np.arange(9.3, 11.8, 0.01)
        Mstar = np.log10(10.0**10.91)
        alpha = -0.99
        phistar = 10.17 * 1e-4
        xval = 10.0 ** (M - Mstar)
        yval = np.log(10.0) * phistar * xval ** (alpha + 1) * np.exp(-xval)
        if whichimf == 0:
            plt.plot(
                np.log10(10.0**M * 1.6),
                yval,
                "b:",
                lw=10,
                alpha=0.5,
                label="... z=[1.3,2.0]",
            )
        elif whichimf == 1:
            plt.plot(
                np.log10(10.0**M * 1.6 / 1.8),
                yval,
                "b:",
                lw=10,
                alpha=0.5,
                label="... z=[1.3,2.0]",
            )

        M = np.arange(9.7, 11.8, 0.01)
        Mstar = np.log10(10.0**10.96)
        alpha = -1.01
        phistar = 3.95 * 1e-4
        xval = 10.0 ** (M - Mstar)
        yval = np.log(10.0) * phistar * xval ** (alpha + 1) * np.exp(-xval)
        if whichimf == 0:
            plt.plot(
                np.log10(10.0**M * 1.6),
                yval,
                "g:",
                lw=10,
                alpha=0.5,
                label="... z=[2.0,3.0]",
            )
        elif whichimf == 1:
            plt.plot(
                np.log10(10.0**M * 1.6 / 1.8),
                yval,
                "g:",
                lw=10,
                alpha=0.5,
                label="... z=[2.0,3.0]",
            )

        M = np.arange(10.0, 11.8, 0.01)
        Mstar = np.log10(10.0**11.38)
        alpha = -1.39
        phistar = 0.53 * 1e-4
        xval = 10.0 ** (M - Mstar)
        yval = np.log(10.0) * phistar * xval ** (alpha + 1) * np.exp(-xval)
        if whichimf == 0:
            plt.plot(
                np.log10(10.0**M * 1.6),
                yval,
                "r:",
                lw=10,
                alpha=0.5,
                label="... z=[3.0,4.0]",
            )
        elif whichimf == 1:
            plt.plot(
                np.log10(10.0**M * 1.6 / 1.8),
                yval,
                "r:",
                lw=10,
                alpha=0.5,
                label="... z=[3.0,4.0]",
            )

        ###### z=0

        w = np.where(G_history[self.SMFsnaps[0]].StellarMass > 0.0)[0]
        mass = np.log10(
            G_history[self.SMFsnaps[0]].StellarMass[w] * 1.0e10 / self.Hubble_h
        )

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = int((ma - mi) / binwidth)

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

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
            label="Model galaxies",
        )

        ###### z=1.3

        w = np.where(G_history[self.SMFsnaps[1]].StellarMass > 0.0)[0]
        mass = np.log10(
            G_history[self.SMFsnaps[1]].StellarMass[w] * 1.0e10 / self.Hubble_h
        )

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = int((ma - mi) / binwidth)

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # Overplot the model histograms
        plt.plot(
            xaxeshisto,
            counts
            / self.volume
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
            / binwidth,
            "b-",
        )

        ###### z=2

        w = np.where(G_history[self.SMFsnaps[2]].StellarMass > 0.0)[0]
        mass = np.log10(
            G_history[self.SMFsnaps[2]].StellarMass[w] * 1.0e10 / self.Hubble_h
        )

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = int((ma - mi) / binwidth)

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # Overplot the model histograms
        plt.plot(
            xaxeshisto,
            counts
            / self.volume
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
            / binwidth,
            "g-",
        )

        ###### z=3

        w = np.where(G_history[self.SMFsnaps[3]].StellarMass > 0.0)[0]
        mass = np.log10(
            G_history[self.SMFsnaps[3]].StellarMass[w] * 1.0e10 / self.Hubble_h
        )

        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        NB = int((ma - mi) / binwidth)

        (counts, binedges) = np.histogram(mass, range=(mi, ma), bins=NB)

        # Set the x-axis values to be the centre of the bins
        xaxeshisto = binedges[:-1] + 0.5 * binwidth

        # Overplot the model histograms
        plt.plot(
            xaxeshisto,
            counts
            / self.volume
            * self.Hubble_h
            * self.Hubble_h
            * self.Hubble_h
            / binwidth,
            "r-",
        )

        ######

        plt.yscale("log")

        plt.axis([8.0, 12.5, 1.0e-6, 1.0e-1])

        # Set the x-axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(0.1))

        plt.ylabel(r"$\phi\ (\mathrm{Mpc}^{-3}\ \mathrm{dex}^{-1}$)")  # Set the y...
        plt.xlabel(
            r"$\log_{10} M_{\mathrm{stars}}\ (M_{\odot})$"
        )  # and the x-axis labels

        leg = plt.legend(loc="lower left", numpoints=1, labelspacing=0.1)
        leg.draw_frame(False)  # Don't want a box frame
        for t in leg.get_texts():  # Reduce the size of the text
            t.set_fontsize("medium")

        outputFile = OutputDir + "A.StellarMassFunction_z" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def PlotHistory_SFRdensity(self, G_history):

        print("Plotting SFR density evolution for all galaxies")

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        ObsSFRdensity = np.array(
            [
                [0, 0.0158489, 0, 0, 0.0251189, 0.01000000],
                [0.150000, 0.0173780, 0, 0.300000, 0.0181970, 0.0165959],
                [0.0425000, 0.0239883, 0.0425000, 0.0425000, 0.0269153, 0.0213796],
                [0.200000, 0.0295121, 0.100000, 0.300000, 0.0323594, 0.0269154],
                [0.350000, 0.0147911, 0.200000, 0.500000, 0.0173780, 0.0125893],
                [0.625000, 0.0275423, 0.500000, 0.750000, 0.0331131, 0.0229087],
                [0.825000, 0.0549541, 0.750000, 1.00000, 0.0776247, 0.0389045],
                [0.625000, 0.0794328, 0.500000, 0.750000, 0.0954993, 0.0660693],
                [0.700000, 0.0323594, 0.575000, 0.825000, 0.0371535, 0.0281838],
                [1.25000, 0.0467735, 1.50000, 1.00000, 0.0660693, 0.0331131],
                [0.750000, 0.0549541, 0.500000, 1.00000, 0.0389045, 0.0776247],
                [1.25000, 0.0741310, 1.00000, 1.50000, 0.0524807, 0.104713],
                [1.75000, 0.0562341, 1.50000, 2.00000, 0.0398107, 0.0794328],
                [2.75000, 0.0794328, 2.00000, 3.50000, 0.0562341, 0.112202],
                [4.00000, 0.0309030, 3.50000, 4.50000, 0.0489779, 0.0194984],
                [0.250000, 0.0398107, 0.00000, 0.500000, 0.0239883, 0.0812831],
                [0.750000, 0.0446684, 0.500000, 1.00000, 0.0323594, 0.0776247],
                [1.25000, 0.0630957, 1.00000, 1.50000, 0.0478630, 0.109648],
                [1.75000, 0.0645654, 1.50000, 2.00000, 0.0489779, 0.112202],
                [2.50000, 0.0831764, 2.00000, 3.00000, 0.0512861, 0.158489],
                [3.50000, 0.0776247, 3.00000, 4.00000, 0.0416869, 0.169824],
                [4.50000, 0.0977237, 4.00000, 5.00000, 0.0416869, 0.269153],
                [5.50000, 0.0426580, 5.00000, 6.00000, 0.0177828, 0.165959],
                [3.00000, 0.120226, 2.00000, 4.00000, 0.173780, 0.0831764],
                [3.04000, 0.128825, 2.69000, 3.39000, 0.151356, 0.109648],
                [4.13000, 0.114815, 3.78000, 4.48000, 0.144544, 0.0912011],
                [0.350000, 0.0346737, 0.200000, 0.500000, 0.0537032, 0.0165959],
                [0.750000, 0.0512861, 0.500000, 1.00000, 0.0575440, 0.0436516],
                [1.50000, 0.0691831, 1.00000, 2.00000, 0.0758578, 0.0630957],
                [2.50000, 0.147911, 2.00000, 3.00000, 0.169824, 0.128825],
                [3.50000, 0.0645654, 3.00000, 4.00000, 0.0776247, 0.0512861],
            ],
            dtype=np.float32,
        )

        ObsRedshift = ObsSFRdensity[:, 0]
        xErrLo = np.abs(ObsSFRdensity[:, 0] - ObsSFRdensity[:, 2])
        xErrHi = np.abs(ObsSFRdensity[:, 3] - ObsSFRdensity[:, 0])

        ObsSFR = np.log10(ObsSFRdensity[:, 1])
        yErrLo = np.abs(np.log10(ObsSFRdensity[:, 1]) - np.log10(ObsSFRdensity[:, 4]))
        yErrHi = np.abs(np.log10(ObsSFRdensity[:, 5]) - np.log10(ObsSFRdensity[:, 1]))

        # plot observational data (compilation used in Croton et al. 2006)
        plt.errorbar(
            ObsRedshift,
            ObsSFR,
            yerr=[yErrLo, yErrHi],
            xerr=[xErrLo, xErrHi],
            color="g",
            lw=1.0,
            alpha=0.3,
            marker="o",
            ls="none",
            label="Observations",
        )

        SFR_density = np.zeros((LastSnap + 1 - FirstSnap))
        for snap in range(FirstSnap, LastSnap + 1):
            SFR_density[snap - FirstSnap] = (
                sum(G_history[snap].SfrDisk + G_history[snap].SfrBulge)
                / self.volume
                * self.Hubble_h
                * self.Hubble_h
                * self.Hubble_h
            )

        z = np.array(self.redshift)
        nonzero = np.where(SFR_density > 0.0)[0]
        plt.plot(z[nonzero], np.log10(SFR_density[nonzero]), lw=3.0)

        plt.ylabel(
            r"$\log_{10} \mathrm{SFR\ density}\ (M_{\odot}\ \mathrm{yr}^{-1}\ \mathrm{Mpc}^{-3})$"
        )  # Set the y...
        plt.xlabel(r"$\mathrm{redshift}$")  # and the x-axis labels

        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(1))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.5))

        plt.axis([0.0, 8.0, -3.0, -0.4])

        outputFile = OutputDir + "B.History-SFR-density" + OutputFormat
        plt.savefig(outputFile)  # Save the figure
        print("Saved file to", outputFile)
        plt.close()

        # Add this plot to our output list
        OutputList.append(outputFile)

    # ---------------------------------------------------------

    def StellarMassDensityEvolution(self, G_history):

        print("Plotting stellar mass density evolution")

        plt.figure()  # New figure
        ax = plt.subplot(111)  # 1 plot on the figure

        # SMD observations taken from Marchesini+ 2009, h=0.7
        # Values are (minz, maxz, rho,-err,+err)
        dickenson2003 = np.array(
            (
                (0.6, 1.4, 8.26, 0.08, 0.08),
                (1.4, 2.0, 7.86, 0.22, 0.33),
                (2.0, 2.5, 7.58, 0.29, 0.54),
                (2.5, 3.0, 7.52, 0.51, 0.48),
            ),
            float,
        )
        drory2005 = np.array(
            (
                (0.25, 0.75, 8.3, 0.15, 0.15),
                (0.75, 1.25, 8.16, 0.15, 0.15),
                (1.25, 1.75, 8.0, 0.16, 0.16),
                (1.75, 2.25, 7.85, 0.2, 0.2),
                (2.25, 3.0, 7.75, 0.2, 0.2),
                (3.0, 4.0, 7.58, 0.2, 0.2),
            ),
            float,
        )
        # Perez-Gonzalez (2008)
        pg2008 = np.array(
            (
                (0.2, 0.4, 8.41, 0.06, 0.06),
                (0.4, 0.6, 8.37, 0.04, 0.04),
                (0.6, 0.8, 8.32, 0.05, 0.05),
                (0.8, 1.0, 8.24, 0.05, 0.05),
                (1.0, 1.3, 8.15, 0.05, 0.05),
                (1.3, 1.6, 7.95, 0.07, 0.07),
                (1.6, 2.0, 7.82, 0.07, 0.07),
                (2.0, 2.5, 7.67, 0.08, 0.08),
                (2.5, 3.0, 7.56, 0.18, 0.18),
                (3.0, 3.5, 7.43, 0.14, 0.14),
                (3.5, 4.0, 7.29, 0.13, 0.13),
            ),
            float,
        )
        glazebrook2004 = np.array(
            (
                (0.8, 1.1, 7.98, 0.14, 0.1),
                (1.1, 1.3, 7.62, 0.14, 0.11),
                (1.3, 1.6, 7.9, 0.14, 0.14),
                (1.6, 2.0, 7.49, 0.14, 0.12),
            ),
            float,
        )
        fontana2006 = np.array(
            (
                (0.4, 0.6, 8.26, 0.03, 0.03),
                (0.6, 0.8, 8.17, 0.02, 0.02),
                (0.8, 1.0, 8.09, 0.03, 0.03),
                (1.0, 1.3, 7.98, 0.02, 0.02),
                (1.3, 1.6, 7.87, 0.05, 0.05),
                (1.6, 2.0, 7.74, 0.04, 0.04),
                (2.0, 3.0, 7.48, 0.04, 0.04),
                (3.0, 4.0, 7.07, 0.15, 0.11),
            ),
            float,
        )
        rudnick2006 = np.array(
            (
                (0.0, 1.0, 8.17, 0.27, 0.05),
                (1.0, 1.6, 7.99, 0.32, 0.05),
                (1.6, 2.4, 7.88, 0.34, 0.09),
                (2.4, 3.2, 7.71, 0.43, 0.08),
            ),
            float,
        )
        elsner2008 = np.array(
            (
                (0.25, 0.75, 8.37, 0.03, 0.03),
                (0.75, 1.25, 8.17, 0.02, 0.02),
                (1.25, 1.75, 8.02, 0.03, 0.03),
                (1.75, 2.25, 7.9, 0.04, 0.04),
                (2.25, 3.0, 7.73, 0.04, 0.04),
                (3.0, 4.0, 7.39, 0.05, 0.05),
            ),
            float,
        )

        obs = (
            dickenson2003,
            drory2005,
            pg2008,
            glazebrook2004,
            fontana2006,
            rudnick2006,
            elsner2008,
        )

        for o in obs:
            xval = ((o[:, 1] - o[:, 0]) / 2.0) + o[:, 0]
            if whichimf == 0:
                ax.errorbar(
                    xval,
                    np.log10(10 ** o[:, 2] * 1.6),
                    xerr=(xval - o[:, 0], o[:, 1] - xval),
                    yerr=(o[:, 3], o[:, 4]),
                    alpha=0.3,
                    lw=1.0,
                    marker="o",
                    ls="none",
                )
            elif whichimf == 1:
                ax.errorbar(
                    xval,
                    np.log10(10 ** o[:, 2] * 1.6 / 1.8),
                    xerr=(xval - o[:, 0], o[:, 1] - xval),
                    yerr=(o[:, 3], o[:, 4]),
                    alpha=0.3,
                    lw=1.0,
                    marker="o",
                    ls="none",
                )

        smd = np.zeros((LastSnap + 1 - FirstSnap))

        for snap in range(FirstSnap, LastSnap + 1):
            w = np.where(
                (G_history[snap].StellarMass / self.Hubble_h > 0.01)
                & (G_history[snap].StellarMass / self.Hubble_h < 1000.0)
            )[0]
            if len(w) > 0:
                smd[snap - FirstSnap] = (
                    sum(G_history[snap].StellarMass[w])
                    * 1.0e10
                    / self.Hubble_h
                    / (self.volume / self.Hubble_h / self.Hubble_h / self.Hubble_h)
                )

        z = np.array(self.redshift)
        nonzero = np.where(smd > 0.0)[0]
        plt.plot(z[nonzero], np.log10(smd[nonzero]), "k-", lw=3.0)

        plt.ylabel(r"$\log_{10}\ \phi\ (M_{\odot}\ \mathrm{Mpc}^{-3})$")  # Set the y...
        plt.xlabel(r"$\mathrm{redshift}$")  # and the x-axis labels

        # Set the x and y axis minor ticks
        ax.xaxis.set_minor_locator(plt.MultipleLocator(1))
        ax.yaxis.set_minor_locator(plt.MultipleLocator(0.5))

        plt.axis([0.0, 4.2, 6.5, 9.0])

        outputFile = OutputDir + "C.History-stellar-mass-density" + OutputFormat
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
        help="input directory name (default: ./results/millennium/)",
        metavar="DIR",
    )
    parser.add_option(
        "-f",
        "--file_base",
        dest="FileName",
        default="model",
        help="filename base (default: model)",
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
    parser.add_option(
        "-s",
        "--snap_range",
        type="int",
        nargs=2,
        dest="SnapRange",
        default=(0, 63),
        help="first and last snapshots (default: 0 63)",
        metavar="FIRST LAST",
    )

    (opt, args) = parser.parse_args()

    if opt.DirName[-1] != "/":
        opt.DirName += "/"

    OutputDir = opt.DirName + "/plots/"

    if not os.path.exists(OutputDir):
        os.makedirs(OutputDir)

    res = Results()

    print("Running history...")

    FirstFile = opt.FileRange[0]
    LastFile = opt.FileRange[1]
    FirstSnap = opt.SnapRange[0]
    LastSnap = opt.SnapRange[1]

    # read in all files and put in G_history
    G_history = [0] * (LastSnap - FirstSnap + 1)
    for snap in range(FirstSnap, LastSnap + 1):

        print
        print("SNAPSHOT NUMBER:  ", snap)

        fin_base = opt.DirName + opt.FileName + res.redshift_file[snap]
        G_history[snap] = res.read_gals(fin_base, FirstFile, LastFile, snap)

    print

    res.StellarMassFunction(G_history)
    res.PlotHistory_SFRdensity(G_history)
    res.StellarMassDensityEvolution(G_history)
