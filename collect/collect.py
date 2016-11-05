#!/usr/bin/python
# vim: set fileencoding=UTF-8 :

import datetime
import logging
from pyrrd.rrd import DataSource, RRA, RRD
import serial
import time

SERIAL='/dev/ttyUSB0'
RRDFILE='meteo.rrd'

class Sample(object):
    def __init__(self):
        self.time = time.time()
        self.temperature = None
        self.humidity = None
        self.mq9 = None
        self.dust_pc = None
        self.dust_raw = None

    def __str__(self):
        return "Sample({0}, {1}°C, {2}%, {3}, {4}pc/ft³, {5})".format(
                datetime.datetime.fromtimestamp(self.time).isoformat(),
                self.temperature, self.humidity, self.mq9,
                self.dust_pc, self.dust_raw)

    def ParseLine(self, line):
        """Parses a given line and updates the representation.

        Returns 'True' iff the line contains the dust sensor value.
        """
        # TODO: Handle times prepended in front.
        words = line.split(",")
        tag = words[0]
        if tag == 'dust':
            self.dust_pc = words[1]
            self.dust_raw = words[3]
            return True
        elif tag == 'dustcycles':
            pass
        elif tag == 'temperature':
            self.temperature = words[1]
        elif tag == 'humidity':
            self.humidity = words[1]
        elif tag == 'MQ9':
            self.mq9 = words[3]
        else:
            logging.warning("Don't know how to parse '%s'", line)
        return False

def RrdCreate(rrdfile):
    '''Creates a RRD database.'''
    dataSources = []
    roundRobinArchives = []
    dataSources.append(DataSource(
        dsName='temperature', dsType='GAUGE', heartbeat=600,
        minval=-50, maxval=100))
    dataSources.append(DataSource(
        dsName='humidity', dsType='GAUGE', heartbeat=600,
        minval=0, maxval=100))
    dataSources.append(DataSource(
        dsName='mq9', dsType='GAUGE', heartbeat=600))
    dataSources.append(DataSource(
        dsName='dust_pc', dsType='GAUGE', heartbeat=600, minval=0))
    dataSources.append(DataSource(
        dsName='dust_raw', dsType='GAUGE', heartbeat=600))
    # Keep all values for 10 days
    roundRobinArchives.append(RRA(cf='AVERAGE', xff=0.5, steps=1,
                                  rows=10*24*60))
    # Keep 15-minute averages for one year days
    roundRobinArchives.append(RRA(cf='AVERAGE', xff=0.5, steps=15,
                                  rows=365*24*4))
    # Keep 1-hour averages for 10 years
    roundRobinArchives.append(RRA(cf='AVERAGE', xff=0.5, steps=60,
                                  rows=10*365*24))
    myRRD = RRD(
        rrdfile, step=60, ds=dataSources, rra=roundRobinArchives)
    myRRD.create()

def RrdProcess(rrdfile, samples):
    '''Reads given samples and stores them in the RRD database.'''
    # TODO: Optionally update the database only periodically.
    rrd = RRD(rrdfile)
    for sample in samples:
        logging.debug("Saving sample %s", sample)
        rrd.bufferValue(sample.time, sample.temperature, sample.humidity,
                        sample.mq9, sample.dust_pc, sample.dust_raw)
        rrd.update(debug=True)

def SerialLines():
    with serial.Serial(SERIAL, 9600) as ser:
        for line in ser:
            line = line.strip()
            print line
            yield line

def LinesToSamples(lines):
    sample = Sample()
    for line in lines:
        if sample.ParseLine(line):
            yield sample
            sample = Sample()

def main():
    RrdProcess(RRDFILE, LinesToSamples(SerialLines()))

if __name__ == "__main__":
    main()
