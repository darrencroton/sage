# Routines for reading and plotting to produce comparable figures to the SAGE paper
from pylab import *
from scipy import signal as ss


def galdtype():
    # Define the data-type for the public version of SAGE
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
    return Galdesc


def sageoutsingle(fname):
    # Read a single SAGE output file, intended only as a subroutine of read_sagesnap
    Galdesc = galdtype()
    fin = open(fname, "rb")  # Open the file
    Ntrees = np.fromfile(fin, np.dtype(np.int32), 1)  # Read number of trees in file
    NtotGals = np.fromfile(fin, np.dtype(np.int32), 1)[
        0
    ]  # Read number of gals in file.
    GalsPerTree = np.fromfile(
        fin, np.dtype((np.int32, Ntrees)), 1
    )  # Read the number of gals in each tree
    G = np.fromfile(fin, Galdesc, NtotGals)  # Read all the galaxy data
    return G, NtotGals


def read_sagesnap(fpre, firstfile=0, lastfile=7):
    # Read full SAGE snapshot, going through each file and compiling into 1 array
    Galdesc = galdtype()
    Glist = []
    Ngal = np.array([])
    for i in range(firstfile, lastfile + 1):
        G1, N1 = sageoutsingle(fpre + "_" + str(i))
        Glist += [G1]
        Ngal = np.append(Ngal, N1)
    G = np.empty(sum(Ngal), dtype=Galdesc)
    for i in range(firstfile, lastfile + 1):
        j = i - firstfile
        G[sum(Ngal[:j]) : sum(Ngal[: j + 1])] = Glist[j][0 : Ngal[j]].copy()
    G = G.view(np.recarray)
    return G


def sphere2dk(R, Lbin, Nbin):
    # Make a square 2d kernel of a collapsed sphere of radius R with Nbin bins of length Lbin.
    Nbin = int(Nbin)  # Ensure an integer number of bins
    k = np.zeros((Nbin, Nbin))  # k is the convolution kernel to be output
    for i in range(Nbin):
        for j in range(Nbin):
            r = Lbin * np.sqrt(
                (i - (Nbin - 1) / 2) ** 2 + (j - (Nbin - 1) / 2) ** 2
            )  # Average distance of the pixel from the centre
            if r < R:
                k[i, j] = np.sqrt(R**2 - r**2)
    k /= np.sum(k)  # Make it normalised
    return k


def contour(
    x, y, Nbins=None, weights=None, range=None, Nlevels=25, c="k", ls="-", lw=2
):
    # Plot a 2D contour by first doing a 2D histogram of data with axis positions x and y
    if range == None:
        range = [[np.min(x), np.max(x)], [np.min(y), np.max(y)]]
    if Nbins == None:
        Nbins = len(x) // 10
    im, xedges, yedges = np.histogram2d(x, y, bins=Nbins, weights=weights, range=range)
    xd, yd = xedges[1] - xedges[0], yedges[1] - yedges[0]
    xp, yp = xedges[1:] - xd, yedges[1:] - yd
    k = sphere2dk(3, 1, 7)
    im = ss.convolve2d(im, k, mode="same")  # Smooth the image for cleaner contours
    plt.contour(xp, yp, im.transpose(), Nlevels, colors=c, linestyles=ls, linewidths=lw)


def massfunction(mass, Lbox, Nbins=50, c="k", lw=2, ls="-", label=r"\textsc{sage}"):
    masslog = np.log10(mass[mass > 0])
    lbound, ubound = max(8, np.min(masslog)), min(12.5, np.max(masslog))
    N, edges = np.histogram(masslog, bins=Nbins, range=[lbound, ubound])
    binwidth = edges[1] - edges[0]
    x = edges[:-1] + binwidth / 2
    y = N / (binwidth * Lbox**3)
    plt.plot(x, y, c + ls, linewidth=lw, label=label)


def massfunction_obsdata(h=0.678):
    B = np.array(
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
    plt.fill_between(
        B[:, 0] + np.log10(0.7**2) - np.log10(h**2),
        (B[:, 1] + B[:, 2]) * h**3,
        (B[:, 1] - B[:, 2]) * h**3,
        facecolor="purple",
        alpha=0.2,
    )
    plt.plot(
        [1, 1],
        [1, 2],
        color="purple",
        linewidth=8,
        alpha=0.3,
        label=r"Baldry et al.~(2008)",
    )  # Just for the legend


def btf_obsdata(h=0.678):
    x_obs = np.linspace(1, 3, 100)
    y_obs_arr = np.array(
        [
            [4.09 * x_obs + 2.3],
            [4.09 * x_obs + 1.28],
            [3.79 * x_obs + 2.3],
            [3.79 * x_obs + 1.28],
        ]
    )  # Random + systematic
    y_obs_min = np.min(y_obs_arr, axis=0)[0] + 2 * np.log10(0.75 / h)
    y_obs_max = np.max(y_obs_arr, axis=0)[0] + 2 * np.log10(
        0.75 / h
    )  # h=0.75 used in the Stark+ paper
    plt.fill_between(x_obs, y_obs_max, y_obs_min, color="purple", alpha=0.2)
    plt.plot(
        [-1, -1],
        [-1, -2],
        color="purple",
        ls="-",
        lw=8,
        alpha=0.3,
        label=r"Stark et al.~(2009)",
    )  # Just for legend


def bhbulge_obsdata(h=0.678):
    M_BH_obs = (
        (0.7 / h) ** 2
        * 1e8
        * np.array(
            [
                39,
                11,
                0.45,
                25,
                24,
                0.044,
                1.4,
                0.73,
                9.0,
                58,
                0.10,
                8.3,
                0.39,
                0.42,
                0.084,
                0.66,
                0.73,
                15,
                4.7,
                0.083,
                0.14,
                0.15,
                0.4,
                0.12,
                1.7,
                0.024,
                8.8,
                0.14,
                2.0,
                0.073,
                0.77,
                4.0,
                0.17,
                0.34,
                2.4,
                0.058,
                3.1,
                1.3,
                2.0,
                97,
                8.1,
                1.8,
                0.65,
                0.39,
                5.0,
                3.3,
                4.5,
                0.075,
                0.68,
                1.2,
                0.13,
                4.7,
                0.59,
                6.4,
                0.79,
                3.9,
                47,
                1.8,
                0.06,
                0.016,
                210,
                0.014,
                7.4,
                1.6,
                6.8,
                2.6,
                11,
                37,
                5.9,
                0.31,
                0.10,
                3.7,
                0.55,
                13,
                0.11,
            ]
        )
    )
    M_BH_hi = (
        (0.7 / h) ** 2
        * 1e8
        * np.array(
            [
                4,
                2,
                0.17,
                7,
                10,
                0.044,
                0.9,
                0.0,
                0.9,
                3.5,
                0.10,
                2.7,
                0.26,
                0.04,
                0.003,
                0.03,
                0.69,
                2,
                0.6,
                0.004,
                0.02,
                0.09,
                0.04,
                0.005,
                0.2,
                0.024,
                10,
                0.1,
                0.5,
                0.015,
                0.04,
                1.0,
                0.01,
                0.02,
                0.3,
                0.008,
                1.4,
                0.5,
                1.1,
                30,
                2.0,
                0.6,
                0.07,
                0.01,
                1.0,
                0.9,
                2.3,
                0.002,
                0.13,
                0.4,
                0.08,
                0.5,
                0.03,
                0.4,
                0.38,
                0.4,
                10,
                0.2,
                0.014,
                0.004,
                160,
                0.014,
                4.7,
                0.3,
                0.7,
                0.4,
                1,
                18,
                2.0,
                0.004,
                0.001,
                2.6,
                0.26,
                5,
                0.005,
            ]
        )
    )
    M_BH_lo = (
        (0.7 / h) ** 2
        * 1e8
        * np.array(
            [
                5,
                2,
                0.10,
                7,
                10,
                0.022,
                0.3,
                0.0,
                0.8,
                3.5,
                0.05,
                1.3,
                0.09,
                0.04,
                0.003,
                0.03,
                0.35,
                2,
                0.6,
                0.004,
                0.13,
                0.1,
                0.05,
                0.005,
                0.2,
                0.012,
                2.7,
                0.06,
                0.5,
                0.015,
                0.06,
                1.0,
                0.02,
                0.02,
                0.3,
                0.008,
                0.6,
                0.5,
                0.6,
                26,
                1.9,
                0.3,
                0.07,
                0.01,
                1.0,
                2.5,
                1.5,
                0.002,
                0.13,
                0.9,
                0.08,
                0.5,
                0.09,
                0.4,
                0.33,
                0.4,
                10,
                0.1,
                0.014,
                0.004,
                160,
                0.007,
                3.0,
                0.4,
                0.7,
                1.5,
                1,
                11,
                2.0,
                0.004,
                0.001,
                1.5,
                0.19,
                4,
                0.005,
            ]
        )
    )
    M_sph_obs = (
        (0.7 / h) ** 2
        * 1e10
        * np.array(
            [
                69,
                37,
                1.4,
                55,
                27,
                2.4,
                0.46,
                1.0,
                19,
                23,
                0.61,
                4.6,
                11,
                1.9,
                4.5,
                1.4,
                0.66,
                4.7,
                26,
                2.0,
                0.39,
                0.35,
                0.30,
                3.5,
                6.7,
                0.88,
                1.9,
                0.93,
                1.24,
                0.86,
                2.0,
                5.4,
                1.2,
                4.9,
                2.0,
                0.66,
                5.1,
                2.6,
                3.2,
                100,
                1.4,
                0.88,
                1.3,
                0.56,
                29,
                6.1,
                0.65,
                3.3,
                2.0,
                6.9,
                1.4,
                7.7,
                0.9,
                3.9,
                1.8,
                8.4,
                27,
                6.0,
                0.43,
                1.0,
                122,
                0.30,
                29,
                11,
                20,
                2.8,
                24,
                78,
                96,
                3.6,
                2.6,
                55,
                1.4,
                64,
                1.2,
            ]
        )
    )
    M_sph_hi = (
        (0.7 / h) ** 2
        * 1e10
        * np.array(
            [
                59,
                32,
                2.0,
                80,
                23,
                3.5,
                0.68,
                1.5,
                16,
                19,
                0.89,
                6.6,
                9,
                2.7,
                6.6,
                2.1,
                0.91,
                6.9,
                22,
                2.9,
                0.57,
                0.52,
                0.45,
                5.1,
                5.7,
                1.28,
                2.7,
                1.37,
                1.8,
                1.26,
                1.7,
                4.7,
                1.7,
                7.1,
                2.9,
                0.97,
                7.4,
                3.8,
                2.7,
                86,
                2.1,
                1.30,
                1.9,
                0.82,
                25,
                5.2,
                0.96,
                4.9,
                3.0,
                5.9,
                1.2,
                6.6,
                1.3,
                5.7,
                2.7,
                7.2,
                23,
                5.2,
                0.64,
                1.5,
                105,
                0.45,
                25,
                10,
                17,
                2.4,
                20,
                67,
                83,
                5.2,
                3.8,
                48,
                2.0,
                55,
                1.8,
            ]
        )
    )
    M_sph_lo = (
        (0.7 / h) ** 2
        * 1e10
        * np.array(
            [
                32,
                17,
                0.8,
                33,
                12,
                1.4,
                0.28,
                0.6,
                9,
                10,
                0.39,
                2.7,
                5,
                1.1,
                2.7,
                0.8,
                0.40,
                2.8,
                12,
                1.2,
                0.23,
                0.21,
                0.18,
                2.1,
                3.1,
                0.52,
                1.1,
                0.56,
                0.7,
                0.51,
                0.9,
                2.5,
                0.7,
                2.9,
                1.2,
                0.40,
                3.0,
                1.5,
                1.5,
                46,
                0.9,
                0.53,
                0.8,
                0.34,
                13,
                2.8,
                0.39,
                2.0,
                1.2,
                3.2,
                0.6,
                3.6,
                0.5,
                2.3,
                1.1,
                3.9,
                12,
                2.8,
                0.26,
                0.6,
                57,
                0.18,
                13,
                5,
                9,
                1.3,
                11,
                36,
                44,
                2.1,
                1.5,
                26,
                0.8,
                30,
                0.7,
            ]
        )
    )
    core = np.array(
        [
            1,
            1,
            0,
            1,
            1,
            0,
            0,
            0,
            1,
            1,
            0,
            1,
            0,
            0,
            0,
            0,
            0,
            1,
            1,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            0,
            0,
            1,
            1,
            1,
            0,
            0,
            0,
            1,
            1,
            0,
            0,
            0,
            0,
            0,
            1,
            0,
            1,
            0,
            0,
            1,
            0,
            0,
            0,
            1,
            0,
            1,
            0,
            1,
            0,
            1,
            1,
            1,
            0,
            0,
            1,
            0,
            1,
            0,
        ]
    )
    yerr2, yerr1 = np.log10((M_BH_obs + M_BH_hi) / M_BH_obs), -np.log10(
        (M_BH_obs - M_BH_lo) / M_BH_obs
    )
    xerr2, xerr1 = np.log10((M_sph_obs + M_sph_hi) / M_sph_obs), -np.log10(
        (M_sph_obs - M_sph_lo) / M_sph_obs
    )
    plt.errorbar(
        np.log10(M_sph_obs[core == 0]),
        np.log10(M_BH_obs[core == 0]),
        yerr=[yerr1[core == 0], yerr2[core == 0]],
        xerr=[xerr1[core == 0], xerr2[core == 0]],
        color="purple",
        alpha=0.3,
        label=r"S13 core",
        ls="none",
        lw=2,
        ms=0,
    )
    plt.errorbar(
        np.log10(M_sph_obs[core == 1]),
        np.log10(M_BH_obs[core == 1]),
        yerr=[yerr1[core == 1], yerr2[core == 1]],
        xerr=[xerr1[core == 1], xerr2[core == 1]],
        color="c",
        alpha=0.3,
        label=r"S13 S\`{e}rsic",
        ls="none",
        lw=2,
        ms=0,
    )


def massmet_obsdata(h=0.673):
    x_obs = np.array(
        [
            8.52,
            8.57,
            8.67,
            8.76,
            8.86,
            8.96,
            9.06,
            9.16,
            9.26,
            9.36,
            9.46,
            9.57,
            9.66,
            9.76,
            9.86,
            9.96,
            10.06,
            10.16,
            10.26,
            10.36,
            10.46,
            10.56,
            10.66,
            10.76,
            10.86,
            10.95,
            11.05,
            11.15,
            11.25,
            11.30,
        ]
    )
    y_low = np.array(
        [
            8.25,
            8.25,
            8.28,
            8.32,
            8.37,
            8.46,
            8.56,
            8.59,
            8.60,
            8.63,
            8.66,
            8.69,
            8.72,
            8.76,
            8.80,
            8.83,
            8.85,
            8.88,
            8.92,
            8.94,
            8.96,
            8.98,
            9.00,
            9.01,
            9.02,
            9.03,
            9.03,
            9.04,
            9.03,
            9.03,
        ]
    )
    y_high = np.array(
        [
            8.64,
            8.64,
            8.65,
            8.70,
            8.73,
            8.75,
            8.82,
            8.82,
            8.86,
            8.88,
            8.92,
            8.94,
            8.96,
            8.99,
            9.01,
            9.05,
            9.06,
            9.09,
            9.10,
            9.11,
            9.12,
            9.14,
            9.15,
            9.15,
            9.16,
            9.17,
            9.17,
            9.18,
            9.18,
            9.18,
        ]
    )
    x_obs += np.log10(1.5 / 1.8) + 2 * np.log10(
        0.7 / h
    )  # Accounts for difference in Kroupa & Chabrier IMFs and the difference in h
    plt.fill_between(x_obs, y_high, y_low, color="purple", alpha=0.2)
    plt.plot(
        [-1, -1],
        [-1, -2],
        color="purple",
        ls="-",
        lw=8,
        alpha=0.3,
        label=r"Tremonti et al.~(2004)",
    )  # Just for legend
    plt.xlim(np.min(x_obs), np.max(x_obs))


def quiescent(M_star, SFR, sSFRcut=1e-11, c="k", ls="-", lw=2, h=0.678, Nbins=25):
    M_star, SFR = M_star[M_star > 0], SFR[M_star > 0]
    sSFR = SFR / M_star
    logM = np.log10(M_star)
    range = [9.0 + 2 * np.log10(0.7 / h), 11.5 + 2 * np.log10(0.7 / h)]
    Ntot, edge = np.histogram(logM, bins=Nbins, range=range)
    Nred, edge = np.histogram(logM[sSFR < sSFRcut], bins=Nbins, range=range)
    logMplot = (edge[1:] + edge[:-1]) / 2.0
    plt.plot(
        logMplot[Ntot > 0],
        (1.0 * Nred[Ntot > 0]) / Ntot[Ntot > 0],
        c + ls,
        linewidth=lw,
        label=r"\textsc{sage}",
    )
    plt.axis([range[0], range[1], 0, 1])


def quiescent_obs(h=0.678):
    xplot = np.array(
        [
            9.01355036,
            9.11355036,
            9.21355036,
            9.31355036,
            9.41355036,
            9.51355036,
            9.61355036,
            9.71355036,
            9.81355036,
            9.91355036,
            10.01355036,
            10.11355036,
            10.21355036,
            10.31355036,
            10.41355036,
            10.51355036,
            10.61355036,
            10.71355036,
            10.81355036,
            10.91355036,
            11.01355036,
            11.11355036,
            11.21355036,
            11.31355036,
            11.41355036,
        ]
    ) + 2 * np.log10(0.73 / h)
    ymax = np.array(
        [
            0.01573072,
            0.02670944,
            0.03486613,
            0.05387423,
            0.06631411,
            0.10754735,
            0.14996351,
            0.19991417,
            0.24246808,
            0.2786477,
            0.34340491,
            0.37078458,
            0.42101923,
            0.4627398,
            0.53563742,
            0.58856477,
            0.64181569,
            0.71478586,
            0.77162649,
            0.81487748,
            0.89033055,
            0.92382114,
            0.96371631,
            1.03993788,
            1.09048146,
        ]
    )
    ymin = np.array(
        [
            0.01193108,
            0.02189671,
            0.02953616,
            0.04715393,
            0.05877118,
            0.09789483,
            0.13853287,
            0.1864227,
            0.2276533,
            0.26262365,
            0.32536941,
            0.35174427,
            0.40059964,
            0.44076193,
            0.51158711,
            0.5615463,
            0.61213584,
            0.68116556,
            0.73224148,
            0.7676331,
            0.8323257,
            0.84779387,
            0.86470192,
            0.89376378,
            0.84981705,
        ]
    )
    plt.fill_between(xplot, ymax, ymin, color="purple", alpha=0.3)
    plt.plot(
        [-1, -1], [-1, -2], "-", color="purple", lw=8, alpha=0.3, label=r"Observations"
    )
    plt.axis([np.min(xplot), np.max(xplot), 0, 1])


def percentiles(
    x, y, low=0.16, med=0.5, high=0.84, bins=20, xrange=None, yrange=None, Nmin=10
):
    # Given some values to go on x and y axes, bin them along x and return the percentile ranges
    f = np.isfinite(x) * np.isfinite(y)
    if xrange is not None:
        f = (x >= xrange[0]) * (x <= xrange[1]) * f
    if yrange is not None:
        f = (y >= yrange[0]) * (y <= yrange[1]) * f
    x, y = x[f], y[f]
    if type(bins) == int:
        indices = np.array(np.linspace(0, len(x) - 1, bins + 1), dtype=int)
        bins = np.sort(x)[indices]
    elif Nmin > 0:  # Ensure a minimum number of data in each bin
        Nhist, bins = np.histogram(x, bins)
        while len(Nhist[Nhist < Nmin]) > 0:
            ii = np.where(Nhist < Nmin)[0][0]
            if ii == 0 or (ii != len(Nhist) - 1 and Nhist[ii + 1] < Nhist[ii - 1]):
                bins = np.delete(bins, ii + 1)
            else:
                bins = np.delete(bins, ii)
            Nhist, bins = np.histogram(x, bins)
    Nbins = len(bins) - 1
    y_low, y_med, y_high = np.zeros(Nbins), np.zeros(Nbins), np.zeros(Nbins)
    x_av, N = np.zeros(Nbins), np.zeros(Nbins)
    for i in range(Nbins):
        f = (x >= bins[i]) * (x < bins[i + 1])
        yy = np.sort(y[f])
        if len(yy) > 2:
            i_low, i_med, i_high = (
                int(low * len(yy)) - 1,
                int(med * len(yy)) - 1,
                int(high * len(yy)) - 1,
            )
            frac_low, frac_med, frac_high = (
                low * len(yy) - i_low - 1,
                med * len(yy) - i_med - 1,
                high * len(yy) - i_high - 1,
            )
            if i_high <= i_med or i_med <= i_low or i_high <= i_low:
                print("i_low, i_med, i_high = ", i_low, i_med, i_high)
            y_low[i] = (
                yy[i_low] * (1 - frac_low) + yy[i_low + 1] * frac_low
                if i_low > 0
                else yy[0]
            )
            y_med[i] = (
                yy[i_med] * (1 - frac_med) + yy[i_med + 1] * frac_med
                if i_med > 0
                else yy[0]
            )
            y_high[i] = (
                yy[i_high] * (1 - frac_high) + yy[i_high + 1] * frac_high
                if i_high < len(yy) - 1
                else yy[-1]
            )
            x_av[i] = np.mean(x[f])
            N[i] = len(x[f])
    fN = N > 0
    return x_av[fN], y_high[fN], y_med[fN], y_low[fN]
