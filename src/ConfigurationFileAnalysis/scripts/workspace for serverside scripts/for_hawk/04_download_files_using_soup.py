import urllib2
import urllib
from bs4 import BeautifulSoup
import cookielib
import os
import csv
import time
import math

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

def add_log(lstr, fname):
    log_file = open(fname,"w")
    log_file.write(lstr + "\n")
    log_file.close()
    
def downloadChunks(url, path, file_name):
    """Helper to download large files
        the only arg is a url
       this file will go to a temp directory
       the file will also be downloaded
       in chunks and print out how much remains
    """
    if not os.path.isdir(path):
        os.mkdir(path)
    baseFile = os.path.basename(url)

    #move the file to a more uniq path
    #os.umask(0002)
    temp_path = path
    try:
        file = os.path.join(temp_path,baseFile)

        req = urllib2.urlopen(url)
        total_size = int(req.info().getheader('Content-Length').strip())
        downloaded = 0
        CHUNK = 2048 * 10240
        with open(file, 'wb') as fp:
            while True:
                chunk = req.read(CHUNK)
                downloaded = downloaded + len(chunk)
                #print math.floor( (downloaded / total_size) * 100 )
                print math.floor(downloaded)
                if not chunk: break
                fp.write(chunk)
    except urllib2.HTTPError, e:
        print "HTTP Error:",e.code , url
        return False
    except urllib2.URLError, e:
        print "URL Error:",e.reason , url
        return False

    return file


if __name__ == "__main__":
    cj = cookielib.CookieJar()
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))

    opener.addheaders = [('User-agent', 'RedditTesting')]

    urllib2.install_opener(opener)

    authentication_url = 'http://hotfile.com/login.php'
    
    payload = {
            'returnto' : '/',
            'user' : 'luc2yj',
            'pass' : 'rnwjddlraud85'
        }

    pdata = urllib.urlencode(payload)
    req1 = urllib2.Request(authentication_url, pdata)
    resp = urllib2.urlopen(req1)

    contents = resp.read()

    #read download list and start download
    dwdata = open_file("download_list.csv")
    fwlist = csv.reader(dwdata)
    cnt = 0
    for fw in fwlist:
        data = urllib2.urlopen(fw[9]).read()
        soup = BeautifulSoup(data)

        links = soup.find_all("a", {"class":"click_download"})

        #links[0].get('href')
        #print links[0].get('href')
        #urllib2.urlopen(links[0].get('href'))
        #dlfile(links[0].get('href'), fw[8], "downloaded_firmwares")
        #urllib.urlretrieve(fw[9])
        add_log("downloading : " + fw[8], "download_progress.log")
        downloadChunks(links[0].get('href'), "downloaded_firmwares", fw[8])
        cnt = cnt + 1
        if cnt % 10 == 0:
            time.sleep(300.0)
