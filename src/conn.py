#!/usr/bin/env python

import socket
import time
import os.path
import struct
import cv2
import numpy as np


TCP_IP = '172.20.160.24'
TCP_PORT = 1975
BUFFER_SIZE = 1024

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))

def octosend(message, nExpected):
    if len(message) > 0:
        print "SENDING>>", message, "<<"
        s.send(message)

    if nExpected == 0:
        return("")

    buf = bytearray(b" " * nExpected)
    view = memoryview(buf)

    time.sleep(0.1)
    s.recv_into(view, nExpected)

    return(buf.decode("utf-8"))

def octoread(nExpected):

    out = ""
    bytesRead = 0

    bytesExpected = nExpected

    while bytesExpected > 0:
        data = s.recv(BUFFER_SIZE)
        bytesRead += len(data)
        bytesExpected -= len(data)

        out += data

    if bytesRead != nExpected:
        print "ouch!"

    return out


octosend("SET_API_VERSION 2\n", 0)

hologram="Z:/data/holoyurt/4Deep_Training/capn_bert_july13_4m_7us/capt_burt_jul13-0g-7us_13-Jul-2015_08-23-44-984.bmp"

nDataBytes = 0
dataBytes = ""

outBytes =  ""

def setup(hologram):

    out = octosend("RECONSTRUCT_HOLOGRAMS " + hologram + "\n0\n", 6)

    if out == "RECONS":

        print "1>>>", out, "<<<"
        out = octosend("", 20)
        print "2>>>", out, "<<<"

        out = octosend("STREAM_RECONSTRUCTION 9000\n0\n", 41)

        nDataBytes = int(out[32:40])
        print "N=", nDataBytes

    elif out == "STREAM":

        print "1>>>", out, "<<<"
        out = octosend("", 35)
        print "2>>>", out, "<<<"

        nDataBytes = int(out[26:34])
        print "N=", nDataBytes

    outBytes = octoread(nDataBytes)

    #print "received: ", len(outBytes), " bytes"

    #outBytes = outBytes[0:-26]
    print outBytes[-28:]

    return(outBytes)

outBytes = setup(hologram)

outFloats = np.frombuffer(outBytes, dtype='>f4', count=-1)

print outFloats[0:12]

maxZ = np.max(outFloats)

z = np.zeros((2048, 2048), dtype=np.dtype('b'))

i = j = 0
for k in range(z.shape[0]*z.shape[1]):
    z[i, j] = int(255 * (outFloats[k]/maxZ))
    i += 1
    if i == 2048:
        i = 0
        j += 1

cv2.imshow('image',z)
k = cv2.waitKey(0)
if k == 27:         # wait for ESC key to exit
    cv2.destroyAllWindows()


# RECONSTRUCT_HOLOGRAMS Val\n

# Reconstructs holograms. Value is a list of hologram file names, 
# separated by * character. You can either use absolute paths, or 
# paths relative to the currently selected hologram directory. If 
# more than 1 file is contained in the list, background subtraction 
# will be applied.

# SAVE_RESULT_IMAGE Val\n

# Saves reconstructed image to file. Value is the absolute path to 
# the image. Image file extension determines the type of image file 
# (PNG, JPEG, TIFF, BMP) that is being saved.

# OUTPUT_MODE Val\n

# Sets output mode for reconstructions. Value = 0 intensity 
# reconstructions; 1 amplitude reconstructions; 2 phase reconstructions.


# # When you first connect to Octopus from your program, you need to send
# SET_API_VERSION 2\n

# that switches server to support second version of our api

# then you can send 

# STREAM_RECONSTRUCTION Pos\n

# where Pos is a reconstruction position (in um from the light source)

# Octopus will respond with
# STREAM_RECONSTRUCTION 2048 2048\ndatasize\ndata
# first 2 numbers is the size of reconstructed image (should be 2048 for our standard holograms)
# datasize is size of the data array in bytes (sent as string)
# data is array of floats (2048*2048), that you will need to collect and convert to e.g., cv::Mat (type CV_32F).

