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

#include <sys/times.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <iostream>

#include "lsst/pex/logging/BlockTimingLog.h"
#include "lsst/pex/logging/ScreenLog.h"

using std::cout;
using std::endl;
using lsst::pex::logging::BlockTimingLog;
using lsst::pex::logging::LogRecord;
using lsst::pex::logging::Log;
using lsst::pex::logging::ScreenLog;

int main() {

    struct timeval tv0, tv;
    struct rusage usage0, usage;
    struct tms syst0;
    struct tms syst;
    long tps = sysconf(_SC_CLK_TCK);
    long t0, t;
    long long d;
    int i;

    cout << "Clock ticks: " << tps << " ticks per sec" << endl;

    // measure the cost of gettimeofday()
    gettimeofday(&tv0, NULL);
    for(i=0; i < 1000000; i++) {
        gettimeofday(&tv, NULL);
    }
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "gettimeofday(): " << (t - t0)/1000000.0 << " ms per call" << endl;

    // measure the cost of times()
    gettimeofday(&tv0, NULL);
    times(&syst0);
    for(i=0; i < 1000000; i++) {
        times(&syst);
    }
    gettimeofday(&tv, NULL);
    /*
    cout << "(" << syst.tms_utime << " + " << syst.tms_stime << ") - "
         << "(" << syst0.tms_utime << " + " << syst0.tms_stime << ")" << endl;
    */
    d = ((syst.tms_utime + syst.tms_stime) -    
         (syst0.tms_utime + syst0.tms_stime));
    cout << "time(): " << 1.0 * d / tps << " ms cpu per call" << endl;
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "        " << (t - t0)/1000000.0 << " ms wall-clock time per call" << endl;

    // measure the cost of getrusage()
    gettimeofday(&tv0, NULL);
    times(&syst0);
    getrusage(RUSAGE_SELF, &usage0);
    for(i=0; i < 1000000; i++) {
        getrusage(RUSAGE_SELF, &usage);
    }
    times(&syst);
    gettimeofday(&tv, NULL);
    /*
    cout << "(" << syst.tms_utime << " + " << syst.tms_stime << ") - "
         << "(" << syst0.tms_utime << " + " << syst0.tms_stime << ")" << endl;
    */
    d = ((syst.tms_utime + syst.tms_stime) -    
         (syst0.tms_utime + syst0.tms_stime));
    cout << "getrusage(): " << 1.0 * d / tps << " ms cpu per call" << endl;
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "             " << (t - t0)/1000000.0 << " ms wall-clock time per call" << endl;
    t = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec)*1000000 +
        usage.ru_utime.tv_usec + usage.ru_stime.tv_usec;
    t0 = (usage0.ru_utime.tv_sec + usage0.ru_stime.tv_sec)*1000000 +
        usage0.ru_utime.tv_usec + usage0.ru_stime.tv_usec;
    cout << "             " << (t - t0)/1000000.0 << " ms cpu per call" << endl;

    BlockTimingLog btlog(ScreenLog(false, Log::DEBUG), "test");
    LogRecord *lr = new LogRecord(0, 0, true);
    btlog.addUsageProps(*lr);

    delete lr; lr = new LogRecord(0, 0, true);
    gettimeofday(&tv0, NULL);
    for(i=0; i < 100; i++) 
        btlog.addUsageProps(*lr);
    gettimeofday(&tv, NULL);
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "addUsageProps(), no sysdata: " << (t - t0)/100.0 << " ms" << endl;

    delete lr; lr = new LogRecord(0, 0, true);
    btlog.setUsageFlags(BlockTimingLog::SUTIME);
    gettimeofday(&tv0, NULL);
    for(i=0; i < 100; i++) 
        btlog.addUsageProps(*lr);
    gettimeofday(&tv, NULL);
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "addUsageProps(), cpu time: " << (t - t0)/100.0 << " ms" << endl;
    
    delete lr; lr = new LogRecord(0, 0, true);
    btlog.setUsageFlags(BlockTimingLog::ALLUDATA);
    gettimeofday(&tv0, NULL);
    for(i=0; i < 100; i++) 
        btlog.addUsageProps(*lr);
    gettimeofday(&tv, NULL);
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "addUsageProps(), all sysdata: " << (t - t0)/100.0 << " ms" << endl;
    delete lr;

    Log::getDefaultLog().setThreshold(Log::DEBUG);
    btlog.setUsageFlags(BlockTimingLog::NOUDATA);
    btlog.start();
    gettimeofday(&tv0, NULL);
    btlog.start();
    gettimeofday(&tv, NULL);
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "start(), no sysdata: " << (t - t0) << " ms" << endl;

    btlog.setUsageFlags(BlockTimingLog::SUTIME);
    gettimeofday(&tv0, NULL);
    btlog.start();
    gettimeofday(&tv, NULL);
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "start(), all sysdata: " << (t - t0) << " ms" << endl;

    btlog.setUsageFlags(BlockTimingLog::ALLUDATA);
    gettimeofday(&tv0, NULL);
    btlog.start();
    gettimeofday(&tv, NULL);
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "start(), cpu time: " << (t - t0) << " ms" << endl;

    Log log(Log::getDefaultLog(), "plain");
    log.log(Log::INFO, "hello");
    gettimeofday(&tv0, NULL);
    log.log(Log::INFO, "hello");
    gettimeofday(&tv, NULL);
    t = (tv.tv_sec * 1000000L + tv.tv_usec);
    t0 = (tv0.tv_sec * 1000000L + tv0.tv_usec);
    cout << "normal log: " << (t - t0) << " ms" << endl;
}
