#!/usr/bin/env python

# 
# LSST Data Management System
# Copyright 2008, 2009, 2010 LSST Corporation.
# 
# This product includes software developed by the
# LSST Project (http://www.lsst.org/).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the LSST License Statement and 
# the GNU General Public License along with this program.  If not, 
# see <http://www.lsstcorp.org/LegalNotices/>.
#

# -*- python -*-

"""This module defines the Butler class."""

from __future__ import with_statement
import cPickle
import os
import lsst.daf.base as dafBase
import lsst.pex.logging as pexLog
import lsst.pex.policy as pexPolicy
from lsst.daf.persistence import StorageList, LogicalLocation, ReadProxy

class Butler(object):
    """Butler provides a generic mechanism for persisting and retrieving data using mappers.

    Butlers should always be created using ButlerFactory.create().
    
    A Butler manages an collection of datasets.  Each dataset has a type
    representing its intended usage and a location.  Note that the dataset
    type is not the same as the C++ or Python type of the object containing
    the data.  For example, an ExposureF object might be used to hold the data
    for a raw image, a post-ISR image, a calibrated science image, or a
    difference image.  These would all be different dataset types.

    Each Butler is responsible for a subset of its collection defined by the
    partial data identifier used to create it.  A Butler can produce a
    collection of possible values for a key (or tuples of values for multiple
    keys) if given a partial data identifier.  It can check for the existence
    of a file containing a data set given its type and data identifier.  The
    Butler can then retrieve the data set.  Similarly, it can persist an
    object to an appropriate location when given its associated data
    identifier.

    Note that the Butler has two more advanced features when retrieving a data
    set.  First, the retrieval is lazy.  Input does not occur until the data
    set is actually accessed.  This allows data sets to be retrieved and
    placed on a clipboard prospectively with little cost, even if the
    algorithm of a stage ends up not using them.  Second, the Butler will call
    a standardization hook upon retrieval of the data set.  This function,
    contained in the input mapper object, must perform any necessary
    manipulations to force the retrieved object to conform to standards,
    including translating metadata.

    Public methods:

    getKeys(self)

    queryMetadata(self, datasetType, keys, format=None, dataId={}, **rest)

    datasetExists(self, datasetType, dataId={}, **rest)

    get(self, datasetType, dataId={}, **rest)

    put(self, obj, datasetType, dataId={}, **rest)
    """

    def __init__(self, mapper, persistence, partialId={}):
        """Construct the Butler.  Only called via the ButlerFactory."""

        self.mapper = mapper
        self.persistence = persistence
        self.partialId = partialId
        self.log = pexLog.Log(pexLog.Log.getDefaultLog(),
                "daf.persistence.butler")

    def getKeys(self):
        """Returns the valid data id keys for the dataset collection."""

        return self.mapper.getKeys()

    def queryMetadata(self, datasetType, key, format=None, dataId={}, **rest):
        """Returns the valid values for one or more keys when given a partial
        input collection data id.
        
        @param datasetType the type of data set to inquire about.
        @param key         a key giving the level of granularity of the inquiry.
        @param format      an optional key or tuple of keys to be returned. 
        @param dataId      the partial data id.
        @param **rest      keyword arguments for the partial data id.
        @returns a list of valid values or tuples of valid values as specified
        by the format (defaulting to the same as the key) at the key's level
        of granularity.
        """

        dataId = self._combineDicts(dataId, **rest)
        if format is None:
            format = (key,)
        elif not hasattr(format, '__iter__'):
            format = (format,)
        tuples = self.mapper.queryMetadata(datasetType, key, format, dataId)
        if len(format) == 1:
            return [x[0] for x in tuples]
        return tuples

    def datasetExists(self, datasetType, dataId={}, **rest):
        """Determines if a data set file exists.

        @param datasetType    the type of data set to inquire about.
        @param dataId         the data id of the data set.
        @param **rest         keyword arguments for the data id.
        @returns True if the data set exists or is non-file-based.
        """

        dataId = self._combineDicts(dataId, **rest)
        location = self.mapper.map(datasetType, dataId)
        additionalData = location.getAdditionalData()
        storageName = location.getStorageName()
        if storageName in ('BoostStorage', 'FitsStorage', 'PafStorage',
                'PickleStorage'):
            locations = location.getLocations()
            for locationString in locations:
                logLoc = LogicalLocation(locationString, additionalData)
                if not os.path.exists(logLoc.locString()):
                    return False
            return True
        self.log.log(pexLog.Log.WARN,
                "datasetExists() for non-file storage %s, " +
                "dataset type=%s, keys=%s""" %
                (storageName, datasetType, str(dataId)))
        return True

    def get(self, datasetType, dataId={}, **rest):
        """Retrieves a data set given an input collection data id.
        
        @param datasetType    the type of data set to retrieve.
        @param dataId         the data id.
        @param **rest         keyword arguments for the data id.
        @returns an object retrieved from the data set (or a proxy for one).
        """
        dataId = self._combineDicts(dataId, **rest)
        location = self.mapper.map(datasetType, dataId)
        self.log.log(pexLog.Log.DEBUG, "Get type=%s keys=%s from %s" %
                (datasetType, dataId, str(location)))

        if location.getPythonType() is not None:
            # import this pythonType dynamically 
            pythonTypeTokenList = location.getPythonType().split('.')
            importClassString = pythonTypeTokenList.pop()
            importClassString = importClassString.strip()
            importPackage = ".".join(pythonTypeTokenList)
            importType = __import__(importPackage, globals(), locals(), \
                    [importClassString], -1) 
            pythonType = getattr(importType, importClassString)
        else:
            pythonType = None
        if hasattr(self.mapper, "bypass_" + datasetType):
            bypassFunc = getattr(self.mapper, "bypass_" + datasetType)
            callback = lambda: bypassFunc(datasetType, pythonType,
                    location, dataId)
        elif self.mapper.canStandardize(datasetType):
            callback = lambda: self.mapper.standardize(datasetType,
                    self._read(pythonType, location), dataId)
        else:
            callback = lambda: self._read(pythonType, location)
        return ReadProxy(callback)

    def put(self, obj, datasetType, dataId={}, **rest):
        """Persists a data set given an output collection data id.
        
        @param obj            the object to persist.
        @param datasetType    the type of data set to persist.
        @param dataId         the data id.
        @param **rest         keyword arguments for the data id.
        """
        dataId = self._combineDicts(dataId, **rest)
        location = self.mapper.map(datasetType, dataId)
        self.log.log(pexLog.Log.DEBUG, "Put type=%s keys=%s to %s" %
                (datasetType, dataId, str(location)))
        additionalData = location.getAdditionalData()
        storageName = location.getStorageName()
        locations = location.getLocations()
        # TODO support multiple output locations
        locationString = locations[0]
        logLoc = LogicalLocation(locationString, additionalData)
        trace = pexLog.BlockTimingLog(self.log, "put",
                                      pexLog.BlockTimingLog.INSTRUM+1)
        trace.setUsageFlags(trace.ALLUDATA)

        if storageName == "PickleStorage":
            trace.start("write to %s(%s)" % (storageName, logLoc.locString()))
            outDir = os.path.dirname(logLoc.locString())
            if outDir != "" and not os.path.exists(outDir):
                try:
                    os.makedirs(outDir)
                except OSError, e:
                    # Don't fail if directory exists due to race
                    if e.errno != 17:
                        raise e
            with open(logLoc.locString(), "wb") as outfile:
                cPickle.dump(obj, outfile, cPickle.HIGHEST_PROTOCOL)
            trace.done()
            return

        # Create a list of Storages for the item.
        storageList = StorageList()
        storage = self.persistence.getPersistStorage(storageName, logLoc)
        storageList.append(storage)
        trace.start("write to %s(%s)" % (storageName, logLoc.locString()))

        # Persist the item.
        if hasattr(obj, '__deref__'):
            # We have a smart pointer, so dereference it.
            self.persistence.persist(
                    obj.__deref__(), storageList, additionalData)
        else:
            self.persistence.persist(obj, storageList, additionalData)
        trace.done()

    def _combineDicts(self, dataId, **rest):
        finalId = {}
        finalId.update(self.partialId)
        finalId.update(dataId)
        finalId.update(rest)
        return finalId

    def _map(self, mapper, datasetType, dataId):
        return mapper.map(datasetType, dataId)

    def _read(self, pythonType, location):
        trace = pexLog.BlockTimingLog(self.log, "read",
                                      pexLog.BlockTimingLog.INSTRUM+1)
        
        additionalData = location.getAdditionalData()
        # Create a list of Storages for the item.
        storageName = location.getStorageName()
        results = []
        locations = location.getLocations()
        returnList = True
        if len(locations) == 1:
            returnList = False

        for locationString in locations:
            logLoc = LogicalLocation(locationString, additionalData)
            trace.start("read from %s(%s)" % (storageName, logLoc.locString()))
            
            if storageName == "PafStorage":
                finalItem = pexPolicy.Policy.createPolicy(logLoc.locString())
            elif storageName == "PickleStorage":
                if not os.path.exists(logLoc.locString()):
                    raise RuntimeError, \
                            "No such pickle file: " + logLoc.locString()
                with open(logLoc.locString(), "rb") as infile:
                    finalItem = cPickle.load(infile)
            else:
                storageList = StorageList()
                storage = self.persistence.getRetrieveStorage(storageName, logLoc)
                storageList.append(storage)
                itemData = self.persistence.unsafeRetrieve(
                        location.getCppType(), storageList, additionalData)
                finalItem = pythonType.swigConvert(itemData)
            trace.done()
            results.append(finalItem)

        if not returnList:
            results = results[0]
        return results
