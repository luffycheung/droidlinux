

import os
import glob
import collections
from os import listdir
import csv

def open_file(filename):
    
    while True:
        try:
            file = open(filename, "r")
        except :
            filename = raw_input("That filename doesn't exist. Please enter a filename: ")
        else:
            break

    lines = file.readlines()
    file.close()
    for i in range(0, len(lines), 1):
        lines[i] = lines[i].strip()

    return lines

def get_base_ueventd(path):
    b_ueventd = open_file(path + "/ueventd.rc")
    b_ueventd = remove_blank_lines(b_ueventd)
    return b_ueventd
    

def get_cst_ueventd(path):
    orgpath = os.getcwd()
    os.chdir(path)
    lst_ueventd = []
    for files in glob.glob("ueventd.*.rc"):
        lst_ueventd.append(files)
    os.chdir(orgpath)
    return lst_ueventd    

def remove_blank_lines(slist):
    tlist = []
    
    for lines in slist:
        if len(lines) != 0 and lines[:1] != "#":
            tlist.append(lines)
                    
    return tlist
            
def ueventd2dic(lines):
    tdict = {}
    for line in lines:
        tmp = line.strip().split()
        if len(tmp) == 4:
            tdict[tmp[0]] = [tmp[1],tmp[2],tmp[3]]
        if len(tmp) == 5:
            tdict[tmp[0]] = [tmp[1],tmp[2],tmp[3],tmp[4]]
    return tdict

def print_ueventd_dict(tdict):
    od = collections.OrderedDict(sorted(tdict.items()))
    for i in od:
        print i, 
        for j in od[i]:
            print j,
        print ""


def merge_ueventd(sdict, conf_files, path):
    for filename in conf_files:
        lines = open_file(path + "/" + filename)
        lines = remove_blank_lines(lines)
        tdict = ueventd2dic(lines)
        for i in range(0, len(tdict),1):
            sdict[tdict.keys()[i]] = tdict.items()[i][1]
##            if tdict.keys()[i] in sdict:
##                sdict[tdict.keys()[i]] = tdict.items()[i][1]
##            else:
##                sdict[tdict.keys()[i]] = tdict.items()[i][1]
    return sdict

def list_to_csv(tlist, path):
    tfile = open(path,"wb")
    writer = csv.writer(tfile)
    writer.writerows(tlist)
    
    tfile.close()

def dict2list(tdict, ndict):
    od = collections.OrderedDict(sorted(tdict.items()))
    tlist = []
    
    for verk in tdict:
        for j in od[verk]:
            tmp = []
            if j[1:4] == "sys":
                
                continue
            tmp.append(verk)
            tmp.append(ndict[verk])
            tmp.append(j)
            for k in od[verk][j]:
                tmp.append(k)
            tlist.append(tmp)
    return tlist

if __name__ == "__main__":
    #delimiter = "|"
    delimiter = ","

    
    nexus_ueventd = []
    nexus_dict = {}
    nexus_dicts = {}
##    nexus_ueventd_path = "/home/luc2yj/projects/images/scripts/test/image-occam-jdq39"
    nexus_path_home = "C:\\seltests\\projects\\compare_configuration_files\\scripts\\nexus_conf"
    nexus_device_dict = {
        "4.3": "image-occam-jwr66y",
        "4.2.2": "image-takju-jdq39",
        "4.2.1": "signed-toroplus-img-ga02",
        "4.1.2": "image-takju-jzo54k",
        "4.1.1": "image-mysid-jro03o",
        "4.0.4": "image-takju-imm76i"
                       }
    nexus_path_dict = {
        "4.3": nexus_path_home + "/" + nexus_device_dict["4.3"],
        "4.2.2": nexus_path_home + "/" + nexus_device_dict["4.2.2"],
        "4.2.1": nexus_path_home + "/" + nexus_device_dict["4.2.1"],
        "4.1.2": nexus_path_home + "/" + nexus_device_dict["4.1.2"],
        "4.1.1": nexus_path_home + "/" + nexus_device_dict["4.1.1"],
        "4.0.4": nexus_path_home + "/" + nexus_device_dict["4.0.4"]
                       }
##    nexus_path_dict = {
##        "4.3": nexus_path_home + "/image-occam-jwr66y",
##        "4.2.2": nexus_path_home + "/image-takju-jdq39",
##        "4.2.1": nexus_path_home + "/signed-toroplus-img-ga02",
##        "4.1.2": nexus_path_home + "/image-takju-jzo54k",
##        "4.1.1": nexus_path_home + "/image-mysid-jro03o",
##        "4.0.4": nexus_path_home + "/image-takju-imm76i"
##                       }

    for item in nexus_path_dict.items():
        #print item[0]," ", item[1]
        nexus_ueventd_path = item[1]
        nexus_ueventd = get_base_ueventd(nexus_ueventd_path)

        ueventd_list = get_cst_ueventd(nexus_ueventd_path)
        nexus_dict = ueventd2dic(nexus_ueventd)
        nexus_dict = merge_ueventd(nexus_dict, ueventd_list, nexus_ueventd_path)
        #print nexus_dict
        #nexus_dicts = {item[0]:nexus_dict}
        nexus_dicts[item[0]] = nexus_dict
    #print nexus_dicts

    out = dict2list(nexus_dicts, nexus_device_dict)#, os.getcwd() + "/nexus_conf.csv"
    list_to_csv(out, os.getcwd() + "/nexus_conf.csv")
