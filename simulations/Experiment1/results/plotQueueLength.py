#!/usr/bin/env python

import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import random

def parse_if_number(s):
    try: return float(s)
    except: return True if s=="true" else False if s=="false" else s if s else None

def parse_ndarray(s):
    return np.fromstring(s, sep=' ') if s else None
    
def getResults(file):
    resultsFile = pd.read_csv(file, converters = {
    'attrvalue': parse_if_number,
    'binedges': parse_ndarray,
    'binvalues': parse_ndarray,
    'vectime': parse_ndarray,
    'vecvalue': parse_ndarray})
    vectors = resultsFile[resultsFile.type=='vector']
    vec = vectors[vectors.name == 'queueLength:vector']
    return vec;

if __name__ == "__main__":
    results = []
    nList = [1,2,10,40]
    for arg in sys.argv[1:]:
        results.append(getResults(arg))
    i = 0
    plt.figure(figsize=(11,8))
    plt.xticks(fontsize=20)
    plt.yticks(fontsize=20)
    for result in results:
        colorNum = 0
        #result.index = np.arange(1, len(result) + 1)
        lastQueue = len(result.vecvalue.to_numpy())
        lastQueue = lastQueue - 1
        print(lastQueue)
        yAxis = result.vecvalue.to_numpy()[lastQueue]
        #print(results.vecvalue)
        plt.plot(result.vectime.to_numpy()[lastQueue],yAxis
        , drawstyle='steps-post', linewidth=1)
        colorNum += 1
        
        axes = plt.gca()
                #axes.set_xlim([5, 5.2])
        #axes.set_ylim([0,150])
        plt.xlabel('Time (s)', fontsize=28)
        plt.ylabel('Queue Length (packets)', fontsize=28)
        plt.legend(loc = "upper right")
        plt.title("N = 1", fontsize=35)
        #plt.xticks((np.arange(0, result["Goodput"].idxmax(), step=250)))
        plt.savefig('N=1QueueLength.png')
        i += 1
    

