#!/usr/bin/env python
from __future__ import print_function

import numpy as np
import os
import sys
from os.path import getsize as getFileSize
try:
    xrange
except NameError:
    xrange = range

class sageResults(object):

    """ The following methods of this class generate the figures and plot them.
    """

    def __init__(self, filename, ignored_fields=[]):
        """
        Set up instance variables
        """
        # The input galaxy structure:
        Galdesc_full = [
            ('SnapNum'                      , np.int32),                    
            ('Type'                         , np.int32),
            ('GalaxyIndex'                  , np.int64),                    
            ('CentralGalaxyIndex'           , np.int64),                    
            ('SAGEHaloIndex'                , np.int32),                    
            ('SAGETreeIndex'                , np.int32),                    
            ('SimulationHaloIndex'          , np.int64),                    
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
            ('infallMvir'                   , np.float32),
            ('infallVvir'                   , np.float32),
            ('infallVmax'                   , np.float32)
            ]
        _names = [g[0] for g in Galdesc_full]
        _formats = [g[1] for g in Galdesc_full]
        _galdesc = np.dtype({'names':_names, 'formats':_formats}, align=True)
        
        self.filename = filename
        self.dtype = _galdesc
        self.totntrees = None
        self.totngals = None
        self.ngal_per_tree = None
        self.bytes_offset_per_tree = None
        self.ignored_fields = ignored_fields
        try:
            _mode = os.O_RDONLY | os.O_BINARY
        except AttributeError:
            _mode = os.O_RDONLY
            
        self.fd = os.open(filename, _mode)
        self.fp = open(filename, 'rb')
        

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        os.close(self.fd)
        close(self.fp)

    def read_header(self):
        """
        Read the initial header from the LHaloTree binary file
        """
        import numpy as np

        # Read number of trees in file
        totntrees = np.fromfile(self.fp, dtype=np.int32, count=1)[0]
        
        # Read number of gals in file.
        totngals = np.fromfile(self.fp, dtype=np.int32, count=1)[0]
    
        # Read the number of gals in each tree
        ngal_per_tree = np.fromfile(self.fp, dtype=np.int32, count=totntrees)
        
        self.totntrees = totntrees
        self.totngals = totngals
        self.ngal_per_tree = ngal_per_tree
        
        # First calculate the bytes size of each tree
        bytes_offset_per_tree = ngal_per_tree * self.dtype.itemsize

        # then compute the cumulative sum across the sizes to
        # get an offset to any tree
        # However, tmp here will show the offset to the 0'th
        # tree as the size of the 0'th tree. What we want is the
        # offset to the 1st tree as the size of the 0'th tree
        tmp = bytes_offset_per_tree.cumsum()
        
        # Now assign the cumulative sum of sizes for 0:last-1 (inclusive)
        # as the offsets to 1:last 
        bytes_offset_per_tree[1:-1] = tmp[0:-2]
        
        # And set the offset of the 0'th tree as 0
        bytes_offset_per_tree[0] = 0
        
        # Now add the initial offset that we need to get to the
        # 0'th tree -- i.e., the size of the headers
        header_size  = 4 + 4 + totntrees*4
        bytes_offset_per_tree[:] += header_size

        # Now assign to the instance variable
        self.bytes_offset_per_tree = bytes_offset_per_tree
        
    def read_tree(self, treenum):
        """
        Read a single tree specified by the tree number
        """
        import os
        if self.totntrees is None:
            self.read_header()

        if treenum < 0 or treenum >= self.totntrees:
            msg = "The requested tree index = {0} should be within [0, {2})"\
                .format(treenum, self.totntrees)
            raise ValueError(msg)
        
        ngal = self.ngal_per_tree[treenum]
        if ngal == 0:
            return None

        # This section could have been used to
        # read trees in arbitrary order.
        ## WARNING: DO NOT mix reads from self.fd
        ## and self.fp 
        # nbytes = ngal * self.dtype.itemsize
        # offset = self.bytes_offset_per_tree[treenum]
        # try:
        #     tree = os.pread(self.fd, nbytes, offset)
        # except AttributeError:
        #     # seek to the offset (where offset is
        #     # defined from the beginnng of file)
        #     os.lseek(self.fd, offset, os.SEEK_SET)
        #     tree = os.read(self.fd, nbytes)
        # 
        # tree = np.asarray(tree, dtype=self.dtype)
        
        # This assumes sequential reads
        tree = np.fromfile(self.fp, dtype=self.dtype, count=ngal)
        return tree

def compare_catalogs(g1, g2):
    """
    Compares two SAGE catalogs exactly
    """
    
    if not (isinstance(g1, sageResults) and 
            isinstance(g2, sageResults)):
        msg = "Both inputs must be objects the class 'sageResults'"\
            "type(Object1) = {0} type(Object2) = {1}\n"\
            .format(type(g1), type(g2))
        raise ValueError
    
    msg = "Total number of trees must be identical\n"
    if g1.totntrees != g2.totntrees:
        msg += "catalog1 has {0} trees while catalog2 has {1} trees\n"\
            .format(g1.totntrees, g2.totntrees)
        raise ValueError(msg)
    
    msg = "Total number of galaxies must be identical\n"    
    if g1.totngals != g2.totngals:
        msg += "catalog1 has {0} galaxies while catalog2 has {1} "\
               "galaxies\n".format(g1.totngals, g2.totngals)
        raise ValueError(msg)

    for treenum, (n1, n2) in enumerate(zip(g1.ngal_per_tree,
                                           g2.ngal_per_tree)):
        msg = "Treenumber {0} should have exactly the same number of "\
            "galaxies\n".format(treenum)
        if n1 != n2:
            msg += "catalog1 has {0} galaxies while catalog2 has {1} "\
                   "galaxies\n".format(n1, n2)
            raise ValueError(msg)

    try:
        from tqdm import trange
    except ImportError:
        trange = xrange

    ignored_fields = [a for a in g1.ignored_fields if a in g2.ignored_fields]
    for treenum in trange(g1.totntrees):
        if g1.ngal_per_tree[treenum] == 0:
            continue
        
        t1 = g1.read_tree(treenum)
        t2 = g2.read_tree(treenum)
        if (t1 is None and t1 != t2) or \
           (t2 is None and t1 != t2):
            msg = "Error: Exactly one of the trees contains "\
                  "no galaxies.\nt1 = {0}\nt2 = {1}\n"\
                  .format(t1, t2)
            raise ValueError(msg)

        xx = np.argsort(t1['SimulationHaloIndex'])
        t1 = t1[xx]

        xx = np.argsort(t2['SimulationHaloIndex'])
        t2 = t2[xx]
        
        if t1.shape != t2.shape:
            msg = "Error: Bug in read routine or corrupted/truncated file\n"\
                "Expected to find exactly {0} galaxies in both catalogs\n"\
                "Instead first catalog has the shape {1}\n"\
                "and the second catalog has the shape {2}\n"\
                .format(g1.ngal_per_tree[treenum],
                        t1.shape,
                        t2.shape)
            raise ValueError(msg)
        
        dtype = g1.dtype
        for fld in g1.dtype.names:
            if fld in ignored_fields:
                continue

            f1 = t1[fld]
            f2 = t2[fld]

            msg = "Field = `{0}` not the same between the two catalogs\n"\
                .format(fld)
            if np.allclose(f1, f2): continue

            # If control reaches here, then the fields are not equal
            print("Printing values that were different side-by side\n",
                  file=sys.stderr)
            print("#######################################", file=sys.stderr)
            print("# index          field1          field2", file=sys.stderr)
            numbad = 0
            for _ii, (_f1, _f2) in enumerate(zip(f1, f2)):
                if np.allclose(_f1, _f2): continue

                # if control reaches here -> not equal values
                print("{0} {1} {2}".format(_ii, _f1, _f2), file=sys.stderr)
                numbad += 1

            print("------ Found {0} mis-matched values out of a total of {1} "
                  "------".format(numbad, len(f1)), file=sys.stderr)
            print("#######################################\n", file=sys.stderr)
            # printed out the offending values
            # now raise the error
            raise ValueError(msg)
                
    
#  'Main' section of code.  This if statement executes
#   if the code is run from the 
#   shell command line, i.e. with 'python allresults.py'
if __name__ == '__main__':

    import argparse
    description = "Show differences between two SAGE catalogs"
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('file1', metavar='FILE',
                        help='the first file (say, model1_z0.000.0)')
    parser.add_argument('file2', metavar='FILE',
                        help='the second file (say, model2_z0.000.0)')

    args = parser.parse_args()

    ignored_fields = ["SAGEHaloIndex", "GalaxyIndex", "CentralGalaxyIndex"]
    
    g1 = sageResults(args.file1, ignored_fields)
    g2 = sageResults(args.file2, ignored_fields)

    g1.read_header()
    g2.read_header()
    
    print('Running sagediff on files {0} and {1}'.format(args.file1,
                                                         args.file2))

    compare_catalogs(g1, g2)
    print("")
