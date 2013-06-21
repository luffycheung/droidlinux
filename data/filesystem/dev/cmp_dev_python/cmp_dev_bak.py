import os

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
    return lines

def make_list(lines1, lines2):
    tlist = []
    lines = lines1 + lines2
#    print lines
    for line in lines:
	tmp = line.split()
        try:
            if tmp[-1] not in tlist:
                tlist.append(tmp[-1])
	except:
            continue
    tlist.sort()
    return tlist


def split2dict(lines):
    tdict = {} 
    for line in lines:
        tmp = line.strip().split()
        if len(tmp) > 0:
            tdict[tmp[-1]] = [tmp[0],line.strip()]
    return tdict

def cmp_files(file_name1, file_name2, dev_list, f1_dict, f2_dict):
    strout = ""
    stat = ""

    strout = "dev name|" + file_name1 + "|" + file_name2 + "|" + "phone1dev[Y/N]|" + "phone2dev[Y/N]|" + "permission[Y/N]|" + "phone2devPermission" + "\n" 
    for dev in dev_list:
        strout = strout + dev + "|"
        if dev in f1_dict.keys():
            strout = strout + f1_dict[dev][1] + "|"
            stat = stat + "0"
        else:
            strout = strout + "|"
            stat = stat + "1"
        if dev in f2_dict.keys(): 
            strout = strout + f2_dict[dev][1] + "|"
            stat = stat + "|0"
        else:
            strout = strout + "|"
            stat = stat + "|1"
    

        if stat == "0|0":
            if f1_dict[dev][0] == f2_dict[dev][0]:
                stat = stat + "|0"
            else:
                stat = stat + "|1"
                strout = strout + "|" + f2_dict[dev][0]                
        else:
            stat = stat + "|9"
            if stat == "1|0|9":
                strout = strout + "|" + f2_dict[dev][0]                

        strout = strout + stat

        strout = strout + "\n"
        stat = ""

    file3 = open("./output/"+file_name2 + ".csv", "w")
    file3.writelines(strout)
    file3.close()       	




#main
file_name1 = "galaxy_nexus.dev"
#file_name2 = "galaxyS2kr"
#file_name2 = "galaxy_note_2"
#file_name2 = "OptimusG_Snow"
#file_name2 = "test_phone"

print os.listdir(os.curdir)

f1_lines = open_file(file_name1)
for file_name2 in os.listdir(os.curdir):
    if file_name2.endswith(".dev")==True and file_name2 <> file_name1:
        print "---processing file " + file_name2 + "---",
        
        f2_lines = open_file(file_name2)

        dev_list = make_list(f1_lines, f2_lines)

        f1_dict = split2dict(f1_lines)
        f2_dict = split2dict(f2_lines)
        cmp_files(file_name1, file_name2, dev_list, f1_dict, f2_dict)
