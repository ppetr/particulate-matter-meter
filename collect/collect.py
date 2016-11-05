#!/usr/bin/python
# vim: set fileencoding=UTF-8 :

import datetime
import logging
import serial
import time

SERIAL='/dev/ttyUSB0'

class Sample(object):
    def __init__(self):
        self._time = time.time()
        self._temperature = None
        self._humidity = None
        self._mq9 = None
        self._dust_pc = None
        self._dust_raw = None

    def __str__(self):
        return "Sample({0}, {1}°C, {2}%, {3}, {4}pc/ft³, {5})".format(
                datetime.datetime.fromtimestamp(self._time).isoformat(),
                self._temperature, self._humidity, self._mq9,
                self._dust_pc, self._dust_raw)

    def ParseLine(self, line):
        """Parses a given line and updates the representation.

        Returns 'True' iff the line contains the dust sensor value.
        """
        # TODO: Handle times prepended in front.
        words = line.strip().split(",")
        tag = words[0]
        if tag == 'dust':
            self._dust_pc = words[1]
            self._dust_raw = words[3]
            return True
        elif tag == 'dustcycles':
            pass
        elif tag == 'temperature':
            self._temperature = words[1]
        elif tag == 'humidity':
            self._humidity = words[1]
        elif tag == 'MQ9':
            self._mq9 = words[3]
        else:
            logging.warning("Don't know how to parse '%s'", line)
        return False

def SerialLines():
    with serial.Serial(SERIAL, 9600) as ser:
        for line in ser:
            yield line

def LinesToSamples(lines):
    sample = Sample()
    for line in lines:
        if sample.ParseLine(line):
            yield sample
            sample = Sample()

def main():
    for sample in LinesToSamples(SerialLines()):
        print sample

if __name__ == "__main__":
    main()
