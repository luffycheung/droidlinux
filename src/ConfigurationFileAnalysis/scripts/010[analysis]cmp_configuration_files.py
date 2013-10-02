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

def cmp_ueventd(sdict, tdict):
    pass

def make_dev_list(lines1, lines2):
    tlist = []
    lines = lines1 + lines2
#    print lines
    for line in lines:
        tmp = line.split()
        try:
            # for now ignore sysfs configuration
            if tmp[-1] not in tlist and tmp[-1][:4] != "/sys":
                tlist.append(tmp[-1])
        except:
            continue
    tlist.sort()
    return tlist    

def make_cmp_csv(sdict, tdict, path, delimiter):
    if delimiter == "|":
        strout = "dev|permission|user|group|permission|user|group|only in custom version|alert_permission_abuse|permission modification|alert_input |alert_exynos\n" 
    if delimiter == ",":
        strout = "dev,permission,user,group,permission,user,group,only in custom version,alert_permission_abuse,permission modification,alert_input,alert_exynos\n" 
    afl_input = False
    afl_exynos = False
    afl_permission_abuse = False

    
    dev_list = make_dev_list(sdict.keys(), tdict.keys())
    
    #start making csv
    for dev in dev_list:
        strout = strout + dev + delimiter
        if dev in sdict.keys():
            strout = strout + sdict[dev][0] + delimiter
            strout = strout + sdict[dev][1] + delimiter
            strout = strout + sdict[dev][2] + delimiter
        else:
            strout = strout + delimiter
            strout = strout + delimiter
            strout = strout + delimiter
        if dev in tdict.keys():
            strout = strout + tdict[dev][0] + delimiter
            strout = strout + tdict[dev][1] + delimiter
            strout = strout + tdict[dev][2] + delimiter
        else:
            strout = strout + delimiter
            strout = strout + delimiter
            strout = strout + delimiter

        #check for known vulnerability patterns and add alert
        if dev in tdict.keys():
            if dev not in sdict.keys():
                strout = strout + "O" + delimiter
            else:
                strout = strout + delimiter

            #permission abuse
            if tdict[dev][0] in ["0666", "0777"]:
                strout = strout + "O" + delimiter
            else:
                strout = strout + delimiter

            if  (dev in sdict.keys()) and (tdict[dev][0] != sdict[dev][0]):
                strout = strout + "O" + delimiter
            else:
                strout = strout + delimiter
            
            if dev.find("/input") != -1:
                if tdict[dev][0] in ["0666"]:
                    strout = strout + "O" + delimiter
                else:
                    strout = strout + delimiter
            else:
                strout = strout + delimiter
            if dev.find("exynos") != -1:
                if tdict[dev][0] in ["0666"]:
                    strout = strout + "O"
                else:
                    strout = strout + delimiter   
            else:
                strout = strout + delimiter
        strout = strout + "\n"
    csv_file = open(path + "/cmp_output.csv", "wb")
    csv_file.writelines(strout)
    csv_file.close()

def get_target_configurations(path):
    tlist = listdir(path)
    return tlist

def get_download_list_info(path):
    fw_info = []
    data = open_file(path)
    lines = csv.reader(data)
    for i in lines:
        fw_info.append(i)
    return fw_info
    
def get_trg_dev_info(trg_fw, dwlist):
    #print len(trg_fw), " ", len(trg_fw.strip())
    for line in dwlist:
        if line[8][:-4] == trg_fw.strip():
            return line[4]

def make_cmp_result_list(path, dwlist):
    tlist = listdir(path)
    outlist = []
    for line in dwlist:
        if line[8][:-4] in tlist:
            outlist.append(line)

    
    tfile = open("result_dev_list.csv","wb")
    writer = csv.writer(tfile)
    writer.writerows(outlist)
    
    tfile.close()
    



if __name__ == "__main__":
    #delimiter = "|"
    delimiter = ","
    target_configuration_path_home = "C:\\seltests\\projects\\compare_configuration_files\\scripts\\configuration_files"
    download_list_file_path = "C:\\seltests\\projects\\compare_configuration_files\\scripts\\download_list.csv"

    download_list_info = get_download_list_info(download_list_file_path)
    #print download_list_info
    
    nexus_ueventd = []
    nexus_dict = {}
    nexus_dicts = {}
##    nexus_ueventd_path = "/home/luc2yj/projects/images/scripts/test/image-occam-jdq39"
    nexus_path_home = "C:\\seltests\\projects\\compare_configuration_files\\scripts\\nexus_conf"
    nexus_path_dict = {
        "4.3": nexus_path_home + "/image-occam-jwr66y",
        "4.2.2": nexus_path_home + "/image-takju-jdq39",
        "4.2.1": nexus_path_home + "/signed-toroplus-img-ga02",
        "4.1.2": nexus_path_home + "/image-takju-jzo54k",
        "4.1.1": nexus_path_home + "/image-mysid-jro03o",
        "4.0.4": nexus_path_home + "/image-takju-imm76i"
        
        
                       }

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

    #print print_ueventd_dict(nexus_dicts.get("4.1.2"))
    
## now dealing with all the nexus 4.* series using dict inside a dict: above for loop does the job
##        
##    nexus_ueventd_path = nexus_path_dict.get("4.1.2")
##    nexus_ueventd = get_base_ueventd(nexus_ueventd_path)
##
##    ueventd_list = get_cst_ueventd(nexus_ueventd_path)
##    nexus_dict = ueventd2dic(nexus_ueventd)
##    nexus_dict = merge_ueventd(nexus_dict, ueventd_list, nexus_ueventd_path)
##    #print_ueventd_dict(nexus_dict)

    trg_conf_list = get_target_configurations(target_configuration_path_home)
    for folder in trg_conf_list:
        trg_ueventd_path = target_configuration_path_home + "/" + folder
        trg_ueventd = []
        trg_dict = {}

        trg_ueventd = get_base_ueventd(trg_ueventd_path)
        trg_ueventd_list = get_cst_ueventd(trg_ueventd_path)
        trg_dict = ueventd2dic(trg_ueventd)
        trg_dict = merge_ueventd(trg_dict, trg_ueventd_list, trg_ueventd_path)

        ver = get_trg_dev_info(folder,download_list_info)
        
        if ver == "4.1.1":
            pass
            #ver = "4.1.2"
        elif ver == "4.0.3":
            ver = "4.0.4"
        elif ver == "4.2.1":
            pass
            #ver = "4.2.2"
        make_cmp_csv(nexus_dicts.get(ver), trg_dict, trg_ueventd_path, delimiter)

##        try:
##            trg_ueventd = get_base_ueventd(trg_ueventd_path)
##            trg_ueventd_list = get_cst_ueventd(trg_ueventd_path)
##            trg_dict = ueventd2dic(trg_ueventd)
##            trg_dict = merge_ueventd(trg_dict, trg_ueventd_list, trg_ueventd_path)
##            make_cmp_csv(nexus_dict, trg_dict, trg_ueventd_path, delimiter)
##        except:
##            print "error: " + folder
    #cmp_ueventd(nexus_dict, trg_dict)
    
    
    make_cmp_result_list(target_configuration_path_home, download_list_info)
