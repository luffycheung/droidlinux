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
    tpath = "C:\\seltests\\projects\\compare_configuration_files\\scripts"
    target_csv = tpath + "/nexus_conf.csv"
    output_sql = []
    sql_head = "INSERT INTO `linuxdroid`.`TB_NEXUS_UEVENTD` (`OS_VERSION`, `FILE_NAME`, `DEV_NODE`, `PERMISSION`, `USER`, `GROUP`) VALUES ("
    sql_tail = ")"
    fwlist = csv_to_list(target_csv)

    for i in range(0,len(fwlist),1):
        tmp = sql_head
        for j in range(0,len(fwlist[i]),1):
            tmp = tmp + "'" + fwlist[i][j] + "'"
            if j == len(fwlist[i])-1:
                tmp = tmp + sql_tail + "; \n"
            else:
                tmp = tmp + ","
        output_sql.append(tmp)

    
    file_sql = open(tpath + "/insert_nexus_list.sql","wb")
    file_sql.writelines(output_sql)
    file_sql.close()
    
                 

