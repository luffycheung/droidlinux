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

def get_dev_info(dlist, devname):
    for line in dlist:
        if devname in line[8][:-4]:
            return line
    return "not found"


if __name__ == "__main__":
    #delimiter = "|"
    delimiter = ","
    target_configuration_path_home = "C:\\seltests\\projects\\compare_configuration_files\\scripts\\configuration_files"
    trg_conf_list = get_target_configurations(target_configuration_path_home)
    dev_info_list = csv_to_list("result_dev_list.csv")

    for folder in trg_conf_list:
        r1_cmp_output = []
        r2_cmp_output = []
        c_path = target_configuration_path_home + "/" + folder
        cmp_output = csv_to_list(c_path + "/cmp_output.csv")
        for i in range(0,len(cmp_output),1):
            if i == 0:
                r1_cmp_output.append(get_dev_info(dev_info_list, folder))
                r2_cmp_output.append(get_dev_info(dev_info_list, folder))
                r1_cmp_output.append(cmp_output[i])
                r2_cmp_output.append(cmp_output[i])
                continue
            if cmp_output[i][7] == 'O' and cmp_output[i][8] == 'O':
                r1_cmp_output.append(cmp_output[i])
            if cmp_output[i][7] == 'O':
                r2_cmp_output.append(cmp_output[i])
        list_to_csv(r1_cmp_output, c_path + "/r1_cmp_output.csv")
        list_to_csv(r2_cmp_output, c_path + "/r2_cmp_output.csv")            

    
        
        
    
