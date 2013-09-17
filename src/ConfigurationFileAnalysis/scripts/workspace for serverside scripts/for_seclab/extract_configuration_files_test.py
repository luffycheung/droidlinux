from subprocess import call
from subprocess import check_call
from subprocess import Popen, PIPE
import os
import zipfile
import tarfile
import glob
import time
import shutil
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


def add_log(lstr, path):
    log_file = open(path+"/error_list.txt","a")
    log_file.write(lstr + "\n")
    log_file.close()

def find_zipfile(tpath, completed_list):
    os.chdir(tpath)
    lstZip = []
    for files in glob.glob("*.zip"):
        if files[:-4] not in completed_list:
            if files[:7] != "Samsung":
                lstZip.append(files)
    return lstZip


def find_tarfile(tpath, completed_list):
    os.chdir(tpath)
    lstTar = []
    for files in glob.glob("*.tar.*"):
        if files[:-4] not in completed_list:
            lstTar.append(files)
    for files in glob.glob("*.tar"):
        if files[:-4] not in completed_list:
            lstTar.append(files)

    return lstTar

def find_bootimg(tZip):
    tlist = tZip.namelist()
    if "boot.img" in tlist:
        return "boot.img"
    elif "boot.img.md5" in tlist:
        return "boot.img.md5"
    else:
        return False

def find_bootimg4tar(tTar):
    tlist = tTar.getnames()
    if "boot.img" in tlist:
        return "boot.img"
    elif "boot.img.md5" in tlist:
        return "boot.img.md5"
    else:
        return False

def find_zimage4tar(tTar):
    tlist = tTar.getnames()
    if "zImage" in tlist:
        return "zImage"
    elif "kernel.bin.md5" in tlist:
        return "kernel.bin.md5"
    else:
        return False

def find_tarfiles(tZip):
    lstTar = []
    for files in tZip.namelist():
        print files
        if files.lower().find("tar") != -1:
            lstTar.append(files) 
    return lstTar

def find_bootimgramdisk(tpath, bootimg):
    os.chdir(tpath)
    ramdisk = bootimg + "-ramdisk.gz"
    if len(glob.glob(ramdisk)):
        return True
    else:
        return False

def copy_conf_files(tar_dir):
        
    filelist = glob.glob("*.rc")
    print filelist
    for filename in filelist:
        shutil.copy(filename, tar_dir)

def csv_to_strlist(path):
    fw_info = []
    data = open_file(path)
    lines = csv.reader(data)
    for i in lines:
        fw_info.append(i[0])
    return fw_info

def strlist_to_csv(tlist, path):
    tfile = open(path,"wb")
    writer = csv.writer(tfile)
    lines = []
    for i in tlist:
        lines.append([i])
    writer.writerows(lines)
    
    tfile.close()

def add_to_extract_completed_firmwares(lstr, path):
    log_file = open(path+"/extract_completed_firmwares.txt","a")
    log_file.write(lstr + "\n")
    log_file.close()

def make_dir(strDir):
    if os.path.isdir(strDir):
        pass
    else:
        os.mkdir(strDir)


if __name__ == "__main__":

    #Change Foldernames in different environment
    strTrgDirHome = "/u/yl52/firmwares/download_selected_firmwares/target"
    strSrcDir = "/u/yl52/firmwares/download_selected_firmwares/source"
    strConfDirHome = "/u/yl52/firmwares/download_selected_firmwares/target/configuration_files"
    strBootImgToolPwd = "/u/yl52/firmwares/tools/bootimgtools"
    strZimageToolPwd = "/u/yl52/firmwares/tools/zImagetools"

    lstCmdUnpackZimage1 = [strZimageToolPwd + "/unpack.sh", "zImage", "ramdisk"]
    lstCmdUnpackZimage2 = [strZimageToolPwd + "/unpack.sh", "kernel.bin.md5", "ramdisk"]
    
    lstCmdSplitBootImg1 = ["perl", strBootImgToolPwd + "/split_bootimg.pl", "boot.img"]
    lstCmdSplitBootImg2 = ["perl", strBootImgToolPwd + "/split_bootimg.pl", "boot.img.md5"]
    lstCmdUnpackRamdisk11 = ["gzip", "-dc", "../boot.img-ramdisk.gz"]
    lstCmdUnpackRamdisk12 = ["gzip", "-dc", "../boot.img.md5-ramdisk.gz"]
    lstCmdUnpackRamdisk2 = ["cpio", "-i"]
    #strCompletedListPwd = strSrcDir + "/extract_completed_firmwares.txt"
    csvCompletedListPwd = os.getcwd() + "/extract_completed.csv"
    
    completed_list = []
    #if os.path.exists(strCompletedListPwd):
    #    completed_list = open_file(strSrcDir+"/extract_completed_firmwares.txt")
    if os.path.exists(csvCompletedListPwd):
        completed_list = csv_to_strlist(csvCompletedListPwd)

    #print completed_list
    os.chdir(strSrcDir)

    raw_input("press any key to continue...")






    tar_list = find_tarfile(strSrcDir, completed_list)

        
    print "nubmer of tarfiles: " + str(len(tar_list))
    for i in range(0, len(tar_list),1):
        os.chdir(strSrcDir)
        strTrgDir = strTrgDirHome + "/" + tar_list[i][0:-4]
        strConfDir = strConfDirHome + "/" + tar_list[i][0:-4]
        trgTar = tarfile.TarFile(tar_list[i])

        tarFiles = []
        f_status = 0
        try:
            bootimg = find_bootimg4tar(trgTar)
            zimage = find_zimage4tar(trgTar)
            if bootimg != False:
                #trgZip.extract("boot.img", os.getcwd())
                trgTar.extract(bootimg, strTrgDir)
                os.chdir(strTrgDir)
                if bootimg == "boot.img":
                    call(lstCmdSplitBootImg1)
                else:
                    call(lstCmdSplitBootImg2)
                while not (find_bootimgramdisk(os.getcwd(),bootimg)):
                        time.sleep(5.0)
                make_dir("ramdisk")
                print "created ramdisk"
                os.chdir("ramdisk")
                #print os.getcwd()
                print "changed to ramdisk"
                #print check_call(lstCmdUnpackRamdisk)
                #print Popen(lstCmdUnpackRamdisk, stdin=PIPE)
                if bootimg == "boot.img":
                    gzipProc = Popen(lstCmdUnpackRamdisk11, stdout=PIPE)
                    cpioProc = Popen(lstCmdUnpackRamdisk2, stdin=gzipProc.stdout)
                else:
                    gzipProc = Popen(lstCmdUnpackRamdisk12, stdout=PIPE)
                    cpioProc = Popen(lstCmdUnpackRamdisk2, stdin=gzipProc.stdout)
                f_status = 1
            elif zimage != False:
                trgTar.extract(zimage, strTrgDir)
                os.chdir(strTrgDir)
                if zimage == "zImage":
                    call(lstCmdUnpackZimage1)
                else:
                    call(lstCmdUnpackZimage2)
                os.chdir("ramdisk")
                #print os.getcwd()
                print "changed to ramdisk"
                f_status = 1
            else:
                print "error"                

            if f_status == 1:
                make_dir(strConfDir)
                time.sleep(3.0)
                copy_conf_files(strConfDir)
                completed_list.append(tar_list[i][:-4])
            else:
                add_log(tar_list[i],strSrcDir)
            #add_to_extract_completed_firmwares(tar_list[i],strSrcDir)

        except:
            add_log(tar_list[i],strSrcDir)
    


    zip_list = find_zipfile(strSrcDir, completed_list)
    #zip_list = find_zipfile(os.getcwd())

    for i in zip_list:
        print i

    raw_input("press any key to continue...")

    #unzip files
    print "nubmer of zipfiles: " + str(len(zip_list))
    #open target zipfile
    for i in range(0, len(zip_list),1):
        print zip_list[i][0:-4]
        os.chdir(strSrcDir)
        strTrgDir = strTrgDirHome + "/" + zip_list[i][0:-4]
        strConfDir = strConfDirHome + "/" + zip_list[i][0:-4]
        f_status = 0
        try:
            print zip_list[i]
            trgZip = zipfile.ZipFile(zip_list[i])
            tarFiles = []
            bootimg = find_bootimg(trgZip)
            if bootimg != False:
                #trgZip.extract("boot.img", os.getcwd())
                trgZip.extract(bootimg, strTrgDir)
                os.chdir(strTrgDir)
                if bootimg == "boot.img":
                    call(lstCmdSplitBootImg1)
                else:
                    call(lstCmdSplitBootImg2)
                while not (find_bootimgramdisk(os.getcwd(),bootimg)):
                        time.sleep(5.0)
                make_dir("ramdisk")
                print "created ramdisk"
                os.chdir("ramdisk")
                print os.getcwd()
                print "changed to ramdisk"
                #print check_call(lstCmdUnpackRamdisk)
                #print Popen(lstCmdUnpackRamdisk, stdin=PIPE)
                if bootimg == "boot.img":
                    gzipProc = Popen(lstCmdUnpackRamdisk11, stdout=PIPE)
                    cpioProc = Popen(lstCmdUnpackRamdisk2, stdin=gzipProc.stdout)
                else:
                    gzipProc = Popen(lstCmdUnpackRamdisk12, stdout=PIPE)
                    cpioProc = Popen(lstCmdUnpackRamdisk2, stdin=gzipProc.stdout)
                make_dir(strConfDir)
                time.sleep(3.0)
                copy_conf_files(strConfDir)
                completed_list.append(zip_list[i][:-4])

                #add_to_extract_completed_firmwares(zip_list[i],strSrcDir)
            else:
                print "21"
                tarFiles = find_tarfiles(trgZip)
                if len(tarFiles) == 1:
                    print "22"
                    trgZip.extract(tarFiles[0], strTrgDir)
                    os.chdir(strTrgDir)
                    trgTar = tarfile.TarFile(tarFiles[0])
                    bootimg = find_bootimg4tar(trgTar)
                    zimage = find_zimage4tar(trgTar)
                    if bootimg != False:
                        trgTar.extract(bootimg, os.getcwd())
                        print "spliting boot.img"
                        if bootimg == "boot.img":
                            call(lstCmdSplitBootImg1)
                        else:
                            call(lstCmdSplitBootImg2)
                        while not (find_bootimgramdisk(os.getcwd(),bootimg)):
                            time.sleep(5.0)
                        make_dir("ramdisk")
                        
                        print "created ramdisk"
                        os.chdir("ramdisk")
                    
                        #print os.getcwd()
                        print "changed to ramdisk"
                        #print check_call(lstCmdUnpackRamdisk)
                        #print Popen(lstCmdUnpackRamdisk, stdin=PIPE)
                        if bootimg == "boot.img":
                            gzipProc = Popen(lstCmdUnpackRamdisk11, stdout=PIPE)
                            cpioProc = Popen(lstCmdUnpackRamdisk2, stdin=gzipProc.stdout)
                        else:
                            gzipProc = Popen(lstCmdUnpackRamdisk12, stdout=PIPE)
                            cpioProc = Popen(lstCmdUnpackRamdisk2, stdin=gzipProc.stdout)
                    elif zimage != False:
                        trgTar.extract(zimage, os.getcwd())
                        if zimage == "zImage":
                            call(lstCmdUnpackZimage1)
                        else:
                            call(lstCmdUnpackZimage2)
                        time.sleep(3.0)
                        os.chdir("ramdisk")
                        #print os.getcwd()
                    else:
                        print "error"
                        
                    make_dir(strConfDir)
                    time.sleep(3.0)
                    copy_conf_files(strConfDir)
                    completed_list.append(zip_list[i][:-4])
                    #add_to_extract_completed_firmwares(zip_list[i],strSrcDir)
        except Exception, e:
            print str(e)
            add_log(zip_list[i],strSrcDir)

    strlist_to_csv(completed_list, csvCompletedListPwd)
                    


    

        
                     
 
