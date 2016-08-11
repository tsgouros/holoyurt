#!/usr/bin/env python

import socket
import time
import os.path

TCP_IP = '172.20.160.24'
TCP_PORT = 1975
BUFFER_SIZE = 1024

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))

MESSAGE = "SET_API_VERSION 2\n"
s.send(MESSAGE)

time.sleep(0.1)

hologram="Z:/data/holoyurt/4Deep_Training/capn_bert_july13_4m_7us/capt_burt_jul13-0g-7us_13-Jul-2015_08-23-44-984.bmp"

# try:
#     with open(hologram) as file:
#         pass
# except IOError as e:
#     print "can't find" + hologram

# if os.path.exists(hologram):
#     print hologram + " file found!\n"
# else:
#     print hologram + " file not found...\n"


MESSAGE = "RECONSTRUCT_HOLOGRAMS " + hologram + "\n0\n"
s.send(MESSAGE)
data = s.recv(BUFFER_SIZE)
print "received data>>", data, "<<"

time.sleep(0.1)

MESSAGE = "OUTPUT_MODE 0\n0\n"
s.send(MESSAGE)
data = s.recv(BUFFER_SIZE)
print "received data>>", data, "<<"

time.sleep(0.1)

MESSAGE = "STREAM_RECONSTRUCTION 9000\n0\n"
s.send(MESSAGE)
data = s.recv(BUFFER_SIZE)
print "received data>>", data, "<<"

time.sleep(0.1)

data = s.recv(BUFFER_SIZE)
print "received data>>", data, "<<"

time.sleep(0.1)

data = s.recv(BUFFER_SIZE)
print "received data>>", data, "<<"


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

