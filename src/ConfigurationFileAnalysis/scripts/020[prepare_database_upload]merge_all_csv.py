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

def csv_to_list(path):
    fw_info = []
    data = open_file(path)
    lines = csv.reader(data)
    for i in lines:
        fw_info.append(i)
    return fw_info

def list_to_csv(tlist, path):
    tfile = open(path,"wb")
    writer = csv.writer(tfile)
    writer.writerows(tlist)
    
    tfile.close()

def get_target_configurations(path):
    tlist = listdir(path)
    return tlist

def check_uniqueness_of_dev(olist, tdev):
    for j in range(2, len(olist), 1):
        #print tdev, " ", olist[j][0]
        if tdev == olist[j][0]:
            return False
    return True
    


if __name__ == "__main__":
    #delimiter = "|"
    delimiter = ","
    target_configuration_path_home = "C:/seltests/projects/compare_configuration_files/csv"
    trg_conf_list = get_target_configurations(target_configuration_path_home)
    output_csv = []
    for fname in trg_conf_list:
        
        c_path = target_configuration_path_home + "/" + fname
        cmp_output = csv_to_list(c_path)
        for i in range(0,len(cmp_output),1):
            output_csv.append(cmp_output[i])

    
    output_csv.sort()
    list_to_csv(output_csv, target_configuration_path_home + "/../ALL_FWLIST.csv")
                 

