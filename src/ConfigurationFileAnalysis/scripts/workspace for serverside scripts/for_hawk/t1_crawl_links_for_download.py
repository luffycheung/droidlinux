from selenium import webdriver
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support.ui import Select
import selenium.webdriver.support.ui as ui
from selenium.webdriver.common.keys import Keys
import threading
import time
import os
import sys
from selenium.webdriver.common.by import By
    

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

def make_dev_page_url_list(lines):
    base_url = "http://samsung-updates.com/device/?id="
    tlist = []
    for line in lines:
        tlist.append(base_url+line)
    return tlist
        

def add_log(lstr):
    log_file = open("download_log.txt","a")
    log_file.write(lstr + "\n")
    log_file.close()

def add_link(lnstr):
    link_file = open("firmware_file_link_for_downloading.txt", "a")
    link_file.write(lnstr+"\n")
    link_file.close()
    


if __name__ == "__main__":
#read device list stored in text.
#make a list of device page urls
    dev_list = open_file("samsung-updates_devices_list.txt")
    dev_url_list = make_dev_page_url_list(dev_list)
    #print "number of devices: ", len(dev_url_list)

#configure firefox
    fp = webdriver.FirefoxProfile()
    fp.set_preference("dom.successive_dialog_time_limit", 0)
    fp.set_preference("browser.download.folderList",2)
    fp.set_preference("browser.helperApps.neverAsk.saveToDisk", "application/octet-stream, text/xml, text/csv, text/plain, text/log, application/zlib, application/x-gzip, application/x-compressed, application/x-gtar, multipart/x-gzip, application/tgz, application/gnutar, application/x-tar, application/gzip")
    fp.set_preference("browser.download.dir", os.getcwd())  
    dr = webdriver.Firefox(firefox_profile=fp)


#open device url
    for i in range(35, len(dev_url_list), 1):
        dev_link = dev_url_list[i]
        dr.get(dev_link)
        add_log("dev_url_list:" + str(i) + " / " +  "opening url: " + dev_link)

#find download links 
        while True:
            if len(dr.find_elements_by_xpath("//td[@align='center']/a[@title='Download from Hotfile Mirror']")) == 0:
                time.sleep(0.5)
            else:
                break
        vlist = dr.find_elements_by_xpath("//td[@align='center']/a[@title='Download from Hotfile Mirror']")
        #for j in range(len(vlist)-2, len(vlist), 1):
        #for j in range(24, len(vlist), 1):
        if i == 35:
            jstart = 103
        else:
            jstart = 0

        for j in range(jstart, len(vlist), 1):
            vlist[j].click()
            add_log("vlist:" + str(j) + " / " + dev_list[i])
    
            while True:
                flist = dr.find_elements_by_xpath("//a[@target='_new']")
                if len(flist) == 0:
                    time.sleep(0.5)
                elif len(flist) >= 0:
                    break


            flist[0].click()

            prv_url = "now starting"
            whandles = dr.window_handles
            dr.switch_to_window(whandles[1])
            cnt = 0
            while True:
                if cnt == 10.0:
                    break
		if prv_url == "now starting":
                    time.sleep(5.0)
                    break
                if prv_url == dr.current_url:
                    time.sleep(0.5)
                    cnt = cnt + 0.5
                else:
                    break
            prv_url = dr.current_url
            add_link(dr.current_url)
            add_log("           -->" + dr.current_url)
            dr.switch_to_window(whandles[0])
            dr.back()
            while True:
                if len(dr.find_elements_by_xpath("//a[@target='_new']")) == 0:
                    time.sleep(0.5)
                else:
                    break
            vlist = dr.find_elements_by_xpath("//td[@align='center']/a[@title='Download from Hotfile Mirror']")
        
            
