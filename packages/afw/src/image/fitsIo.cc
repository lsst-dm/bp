// -*- lsst-c++ -*-

/* 
 * LSST Data Management System
 * Copyright 2008, 2009, 2010 LSST Corporation.
 * 
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the LSST License Statement and 
 * the GNU General Public License along with this program.  If not, 
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */
 
/// \file
/// \brief  Utilities that use cfitsio
/// \author Robert Lupton (rhl@astro.princeton.edu)\n
///         Princeton University
/// \date   September 2008
#include <cstring>
#include "boost/format.hpp"
#include "boost/regex.hpp"

#include "lsst/base.h"
#include "lsst/pex/exceptions.h"

#include "lsst/afw/image/fits/fits_io_private.h"


namespace lsst {
namespace afw {
namespace image {
namespace cfitsio {
                
std::string err_msg(std::string const& fileName, ///< (possibly empty) file name
                    int const status, ///< cfitsio error status (default 0 => no error)
                    std::string const & errMsg ///< optional error description
                   ) {
    std::ostringstream os;
    os << "cfitsio error";
    if (fileName != "") {
        os << " (" << fileName << ")";
    }
    if (status != 0) {
        char fitsErrMsg[FLEN_ERRMSG];
        (void)lsst::afw::image::cfitsio::fits_get_errstatus(status, fitsErrMsg);
        os << ": " << fitsErrMsg << " (" << status << ")";
    }
    if (errMsg != "") {
        os << " : " << errMsg;
    }
    return os.str();
}

std::string err_msg(lsst::afw::image::cfitsio::fitsfile const * fd, ///< (possibly invalid) file descriptor
                    int const status, ///< cfitsio error status (default 0 => no error)
                    std::string const & errMsg ///< optional error description
                   ) {
    std::string fileName = "";
    if (fd != 0 && fd->Fptr != 0 && fd->Fptr->filename != 0) {
        fileName = fd->Fptr->filename;
    }
    return err_msg(fileName, status, errMsg);
}

/************************************************************************************************************/
    
int ttypeFromBitpix(const int bitpix) {
    switch (bitpix) {
      case BYTE_IMG:
        return TBYTE;
      case SHORT_IMG:                   // int16
        return TSHORT;
      case USHORT_IMG:                  // uint16
        return TUSHORT;                 // n.b. cfitsio does magic things with bzero/bscale to make Uint16
      case LONG_IMG:                    // int32
        return TINT;
      case ULONG_IMG:                   // uint32
        return TUINT;                   // n.b. cfitsio does magic things with bzero/bscale to make Uint32
      case FLOAT_IMG:                   // float
        return TFLOAT;
      case DOUBLE_IMG:                  // double
        return TDOUBLE;
      default:
        throw LSST_EXCEPT(FitsException, (boost::format("Unsupported value BITPIX==%d") % bitpix).str());
    }
}

/******************************************************************************/
//! \brief Move to the specified HDU
void move_to_hdu(lsst::afw::image::cfitsio::fitsfile *fd, //!< cfitsio file descriptor
                 int hdu,               //!< desired HDU
                 bool relative          //!< Is move relative to current HDU? (default: false)
                ) {
    int status = 0;     // cfitsio status
        
    if (relative) {
        if (fits_movrel_hdu(fd, hdu, NULL, &status) != 0) {
            throw LSST_EXCEPT(FitsException,
                          err_msg(fd, status, boost::format("Attempted to select relative HDU %d") % hdu));
        }
    } else {
        int const real_hdu = (hdu == 0) ? 1 : hdu;
        
        if (fits_movabs_hdu(fd, real_hdu, NULL, &status) != 0) {
            throw LSST_EXCEPT(FitsException,
                              err_msg(fd, status,
                                      boost::format("Attempted to select absolute HDU %d") % real_hdu));
        }
        if (hdu == 0) {                 // they asked for the "default" HDU
            int nAxis = 0;              // number of axes in file
            if (fits_get_img_dim(fd, &nAxis, &status) != 0) {
                throw LSST_EXCEPT(FitsException, cfitsio::err_msg(fd, status, "Getting NAXIS"));
            }

            if (nAxis == 0) {
                if (fits_movrel_hdu(fd, 1, NULL, &status) != 0) {
                    throw LSST_EXCEPT(FitsException,
                                      err_msg(fd, status,
                                              boost::format("Attempted to skip data-less hdu %d") % hdu));
                }
            }
        }
    }
}

/************************************************************************************************************/
// append a record to the FITS header.   Note the specialization to string values

void appendKey(lsst::afw::image::cfitsio::fitsfile* fd, std::string const &keyWord,
               std::string const &keyComment, boost::shared_ptr<const lsst::daf::base::PropertySet> metadata) {

    // NOTE:  the sizes of arrays are tied to FITS standard
    // These shenanigans are required only because fits_write_key does not take const args...
    
    char keyWordChars[80];
    char keyCommentChars[80];
    
    strncpy(keyWordChars, keyWord.c_str(), 80);
    CONST_PTR(lsst::daf::base::PropertyList) pl =
        boost::dynamic_pointer_cast<lsst::daf::base::PropertyList const,
        lsst::daf::base::PropertySet const>(metadata);
    if (keyComment == std::string() && pl) {
        strncpy(keyCommentChars, pl->getComment(keyWord).c_str(), 80);
    }
    else {
        strncpy(keyCommentChars, keyComment.c_str(), 80);
    }
    
    int status = 0;
    std::type_info const & valueType = metadata->typeOf(keyWord); 
    if (valueType == typeid(bool)) {
        if (metadata->isArray(keyWord)) {
            std::vector<bool> tmp = metadata->getArray<bool>(keyWord);
            for (unsigned int i = 0; i != tmp.size(); ++i) {
                bool tmp_i = tmp[i];    // avoid icc warning; is vector<bool> special as it only needs 1 bit?
                fits_write_key(fd, TLOGICAL, keyWordChars, &tmp_i, keyCommentChars, &status);
            }
        } else {
            int tmp = metadata->get<bool>(keyWord); // cfitsio can't handle tmp being a bool
            fits_write_key(fd, TLOGICAL, keyWordChars, &tmp, keyCommentChars, &status);
        }
    } else if (valueType == typeid(int)) {
        if (metadata->isArray(keyWord)) {
            std::vector<int> tmp = metadata->getArray<int>(keyWord);
            for (unsigned int i = 0; i != tmp.size(); ++i) {
                fits_write_key(fd, TINT, keyWordChars, &tmp[i], keyCommentChars, &status);
            }
        } else {
            int tmp = metadata->get<int>(keyWord);

            fits_write_key(fd, TINT, keyWordChars, &tmp, keyCommentChars, &status);
        }
    } else if (valueType == typeid(long)) {
        if (metadata->isArray(keyWord)) {
            std::vector<long> tmp = metadata->getArray<long>(keyWord);
            for (unsigned long i = 0; i != tmp.size(); ++i) {
                fits_write_key(fd, TLONG, keyWordChars, &tmp[i], keyCommentChars, &status);
            }
        } else {
            long tmp = metadata->get<long>(keyWord);

            fits_write_key(fd, TLONG, keyWordChars, &tmp, keyCommentChars, &status);
        }
    } else if (valueType == typeid(boost::int64_t)) {
        if (metadata->isArray(keyWord)) {
            std::vector<boost::int64_t> tmp = metadata->getArray<boost::int64_t>(keyWord);
            for (unsigned int i = 0; i != tmp.size(); ++i) {
                fits_write_key(fd, TLONG, keyWordChars, &tmp[i], keyCommentChars, &status);
            }
        } else {
            boost::int64_t tmp = metadata->get<boost::int64_t>(keyWord);

            fits_write_key(fd, TLONG, keyWordChars, &tmp, keyCommentChars, &status);
        }
    } else if (valueType == typeid(double)) {
        if (metadata->isArray(keyWord)) {
            std::vector<double> tmp = metadata->getArray<double>(keyWord);
            for (unsigned int i = 0; i != tmp.size(); ++i) {
                fits_write_key(fd, TDOUBLE, keyWordChars, &tmp[i], keyCommentChars, &status);
            }
        } else {
            double tmp = metadata->get<double>(keyWord);
            fits_write_key(fd, TDOUBLE, keyWordChars, &tmp, keyCommentChars, &status);
        }
    } else if (valueType == typeid(std::string)) {
        char* cval;
        int N;
        if (metadata->isArray(keyWord)) {
            std::vector<std::string> tmp = metadata->getArray<std::string>(keyWord);
            for (unsigned int i = 0; i != tmp.size(); ++i) {
	      N = tmp[i].size();
	      cval = new char[N+1];
	      strncpy(cval, tmp[i].c_str(), N+1);

                if (keyWord == "COMMENT") {
                    fits_write_comment(fd, cval, &status);
                } else if (keyWord == "HISTORY") {
                    fits_write_history(fd, cval, &status);
                } else {
                    fits_write_key_longstr(fd, keyWordChars, cval, keyCommentChars, &status);
                }
	      delete[] cval;
            }
        } else {
            std::string tmp = metadata->get<std::string>(keyWord);
	    N = tmp.size();
	    cval = new char[N+1];
	    strncpy(cval, tmp.c_str(), N+1);
            if (keyWord == "COMMENT") {
                fits_write_comment(fd, cval, &status);
            } else if (keyWord == "HISTORY") {
                fits_write_history(fd, cval, &status);
            } else {
                fits_write_key_longstr(fd, keyWordChars, cval, keyCommentChars, &status);
            }
	    delete[] cval;
        }

    } else {
        std::cerr << "In " << BOOST_CURRENT_FUNCTION << " Unknown type: " << valueType.name() <<
            " for keyword " << keyWord << std::endl;
    }

    if (status) {
        throw LSST_EXCEPT(FitsException, err_msg(fd, status));
    }
}

//! Get the number of keywords in the header
int getNumKeys(fitsfile* fd) {
     int keynum = 0;
     int numKeys = 0;
     int status = 0;
 
     if (fits_get_hdrpos(fd, &numKeys, &keynum, &status) != 0) {
          throw LSST_EXCEPT(FitsException, err_msg(fd, status));
     }

     return numKeys;
}

void getKey(fitsfile* fd,
            int n, std::string & keyWord, std::string & keyValue, std::string & keyComment) {
     // NOTE:  the sizes of arrays are tied to FITS standard
     char keyWordChars[80];
     char keyValueChars[80];
     char keyCommentChars[80];

     int status = 0;
     if (fits_read_keyn(fd, n, keyWordChars, keyValueChars, keyCommentChars, &status) != 0) {
         throw LSST_EXCEPT(FitsException, err_msg(fd, status));
     }

     // There's no long-string equivalent to fits_read_keyn, so hack it...

     keyWord = keyWordChars;
     keyValue = keyValueChars;
     keyComment = keyCommentChars;

     // FIXME -- what about multi-line CONTINUEs?
     status = 0;
     char cval[80];
     cval[0] = '\0';
     if (ffgcnt(fd, cval, &status)) {
         throw LSST_EXCEPT(FitsException, err_msg(fd, status));
     }
     if (cval[0]) {
         //printf("got CONTINUE: \"%s\"\n", cval);
         std::string more = cval;
         //printf("  key \"%s\"; value \"%s\"; continued value \"%s\"\n",
         //keyWord.c_str(), keyValue.c_str(), more.c_str());
         // value "'/home/dalang/lsst/astrometry_net_data/imsim-2010-11-09-0/index-1011&'"; continued value "09003.fits"
         // the last characters in a CONTINUE'd line are  &'  -- trim
         keyValue = keyValue.substr(0, keyValue.size()-2) + more + "'";
         //printf("Returning: key \"%s\", vale \"%s\"\n", keyWord.c_str(), keyValue.c_str());
     }

}

void addKV(lsst::daf::base::PropertySet::Ptr metadata, std::string const& key, std::string const& value, std::string const& comment) {
    static boost::regex const boolRegex("[tTfF]");
    static boost::regex const intRegex("[+-]?[0-9]+");
    static boost::regex const doubleRegex("[+-]?([0-9]*\\.[0-9]+|[0-9]+\\.[0-9]*)([eE][+-]?[0-9]+)?");
    static boost::regex const fitsStringRegex("'(.*)'");

    boost::smatch matchStrings;
    std::istringstream converter(value);

    PTR(lsst::daf::base::PropertyList) pl =
        boost::dynamic_pointer_cast<lsst::daf::base::PropertyList,
        lsst::daf::base::PropertySet>(metadata);

    if (boost::regex_match(value, boolRegex)) {
        // convert the string to an bool
#if 0
        bool val;
        converter >> val;               // converter doesn't handle bool; T/F always return 255
#else
        bool val = (value == "T" || value == "t");
#endif
        if (pl) {
            pl->add(key, val, comment);
        } else {
            metadata->add(key, val);
        }
    } else if (boost::regex_match(value, intRegex)) {
        // convert the string to an int
        boost::int64_t val;
        converter >> val;
        if (val < (1L << 31) && val > -(1L << 31)) {
            int v = static_cast<int>(val);
            if (pl) {
                pl->add(key, v, comment);
            } else {
                metadata->add(key, v);
            }
        } else {
            if (pl) {
                pl->add(key, val, comment);
            } else {
                metadata->add(key, val);
            }
        }
    } else if (boost::regex_match(value, doubleRegex)) {
        // convert the string to a double
        double val;
        converter >> val;
        if (pl) {
            pl->add(key, val, comment);
        } else {
            metadata->add(key, val);
        }
    } else if (boost::regex_match(value, matchStrings, fitsStringRegex)) {
        // strip off the enclosing single quotes and return the string
        if (pl) {
            pl->add(key, matchStrings[1].str(), comment);
        } else {
            metadata->add(key, matchStrings[1].str());
        }
    } else if (key == "HISTORY" ||
               (key == "COMMENT" &&
                comment != "  FITS (Flexible Image Transport System) format is defined in 'Astronomy" &&
                comment != "  and Astrophysics', volume 376, page 359; bibcode: 2001A&A...376..359H")) {
        if (pl) {
            pl->add(key, comment);
        } else {
            metadata->add(key, comment);
        }
    }
}

// Private function to build a PropertySet that contains all the FITS kw-value pairs
    void getMetadata(fitsfile* fd, lsst::daf::base::PropertySet::Ptr metadata, bool strip) {
    // Get all the kw-value pairs from the FITS file, and add each to DataProperty
    if (metadata.get() == NULL) {
        return;
    }

    for (int i=1; i<=getNumKeys(fd); i++) {
        std::string keyName;
        std::string val;
        std::string comment;
        getKey(fd, i, keyName, val, comment);
        // I could use std::tr1::unordered_map, but it probably isn't worth the trouble
        if (strip && (keyName == "SIMPLE" || keyName == "BITPIX" || keyName == "EXTEND" ||
                      keyName == "NAXIS" || keyName == "NAXIS1" || keyName == "NAXIS2" ||
                      keyName == "GCOUNT" || keyName == "PCOUNT" || keyName == "XTENSION" ||
                      keyName == "BSCALE" || keyName == "BZERO")) {
            ;
        } else {
            addKV(metadata, keyName, val, comment);
        }
    }

}
} // namespace cfitsio

/************************************************************************************************************/

/**
 * \brief Return the metadata from a fits file
 */
lsst::daf::base::PropertySet::Ptr readMetadata(std::string const& fileName, ///< File to read
                                               const int hdu,               ///< HDU to read
                                               bool strip       ///< Should I strip e.g. NAXIS1 from header?
                                              ) {
    lsst::daf::base::PropertySet::Ptr metadata(new lsst::daf::base::PropertyList);

    detail::fits_reader m(fileName, metadata, hdu);
    cfitsio::getMetadata(m.get(), metadata, strip);

    return metadata;
}

/**
 * \brief Return the metadata from a fits RAM file
 */
lsst::daf::base::PropertySet::Ptr readMetadata(char **ramFile,				///< RAM buffer to receive RAM FITS file
												size_t *ramFileLen,			///< RAM buffer length
												const int hdu,              ///< HDU to read
												bool strip       ///< Should I strip e.g. NAXIS1 from header?
                                              ) {
    lsst::daf::base::PropertySet::Ptr metadata(new lsst::daf::base::PropertySet);

    detail::fits_reader m(ramFile, ramFileLen, metadata, hdu);
    cfitsio::getMetadata(m.get(), metadata, strip);

    return metadata;
}
    
}}} // namespace lsst::afw::image
