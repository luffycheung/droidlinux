#copy this file to target folder and run!
#navigate through child folders and delete empty ones
#save list of non-empty folders

import os
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

def get_file_list_of_folder(path):
    tlist = os.listdir(path)
    return tlist

def list_to_csv(tlist, path):
    tfile = open(path,"wb")
    writer = csv.writer(tfile)
    writer.writerows(tlist)
    
    tfile.close()

if __name__ == "__main__":

    #save current path
    path = os.getcwd()
    outpath = path + "/folder_list.csv"

    flist = get_file_list_of_folder(path)
    output_csv = []
    for f in flist:
        if os.path.isdir(f):
            tpath = path + "/" + f
            if os.listdir(tpath) == []:
                #os.rmdir(tpath)
                print "deleting: " + tpath
            else:
                #print "adding:" + f
                output_csv.append([f])
    output_csv.sort()
    list_to_csv(output_csv, outpath)
    raw_input("press any key to continue...")
            

        
