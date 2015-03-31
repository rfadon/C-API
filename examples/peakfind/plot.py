#!/usr/bin/env python

import sys
import matplotlib.pyplot as plt

# load a file
fp = open(sys.argv[2], 'r')
values = []

for line in fp:
    if sys.argv[1] == 'cpx':
        (index, r, i) = line.split(",")
        values.append( (float(r), float(i)) )
    elif sys.argv[1] == 'scalar':
        (index, r) = line.split(",")
        values.append(float(r))

# plot the values
plt.plot(values)
plt.show()
