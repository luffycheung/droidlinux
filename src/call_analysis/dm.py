#!/usr/bin/env python  
""" 
turn the traceview data into a jpg pic, showing methods call relationship 
This script is for Linux 64 bit, if run on 32 machine, please adjust
add_one_record function when unpack the data. 
"""  
import sys  
import os  
import struct  
import re  
################################################################################  
########################  Global Variable  #####################################  
################################################################################  

target_thread=10 #the thread that we want to track, filt out other threads  
#all_actions = ["enter","exit","exception","reserved"]  
all_threads = {}  
all_methods = {}  
all_records = []  
parent_methods = {}  
child_methods = {}  
method_calls = {}  

################################################################################  
##############################   Methods   #####################################  
################################################################################  

def add_one_thread(line):  
    fields = line.split("\t")  
    all_threads[int(fields[0],10)]=fields  

def add_one_method(line):  
    fields = line.split("\t")  
    all_methods[int(fields[0],16)]=fields
    #print fields  

def add_one_record(one):
    thread_id,=struct.unpack("H",one[0:2])  
    if (thread_id == target_thread):  
        tmp,=struct.unpack("I",one[2:6])  
        method_id= (tmp / 4) * 4;  
        method_action= tmp % 4;  
        time_offset,=struct.unpack("Q",one[6:])
	#print thread_id, method_id, method_action, time_offset
        all_records.append([thread_id, method_id, method_action, time_offset])  

def handle_one_call(parent_method_id, method_id):  
    if not (parent_methods.has_key(parent_method_id)):  
        parent_methods[parent_method_id]=1  
    if not (child_methods.has_key(method_id)):  
        child_methods[method_id]=1  
    if method_calls.has_key(parent_method_id):  
        if method_calls[parent_method_id].has_key(method_id):  
            method_calls[parent_method_id][method_id]+=1  
        else:  
            method_calls[parent_method_id][method_id]=1  
    else:  
        method_calls[parent_method_id]={}  
        method_calls[parent_method_id][method_id]=1  


def gen_funcname(method_id):  
    r1=re.compile(r'[/$<>]')  
    str1=r1.sub("_",all_methods[method_id][1])  
    str2=r1.sub("_",all_methods[method_id][2])  
    return str1+"_"+str2  


def gen_dot_script_file(dot_file_name):  
    myoutfile = open(dot_file_name, "w")  
    myoutfile.write("digraph vanzo {\n\n");  
    for one in all_methods.keys():  
        if parent_methods.has_key(one):  
            myoutfile.write(gen_funcname(one)+"  [shape=rectangle];\n")  
        else:  
            if child_methods.has_key(one):  
                myoutfile.write(gen_funcname(one)+"  [shape=ellipse];\n")  
    for one in method_calls.keys():  
        for two in method_calls[one]:  
            myoutfile.write(gen_funcname(one) + " -> " + gen_funcname(two) +  " [label=\"" + str(method_calls[one][two]) + "\"  fontsize=\"10\"];\n")
            
    myoutfile.write("\n}\n");  
    myoutfile.close  
    
################################################################################  
########################## Script starts from here #############################  
################################################################################  
if len(sys.argv) < 2:    
    print 'No input file specified.'    
    sys.exit()  
if not (os.path.exists(sys.argv[1])):  
    print "input file not exists"  
    sys.exit()  
#Now handle the text part  
current_section=0  
for line in open(sys.argv[1]):  
    line2 = line.strip()  
    if (line2.startswith("*")):  
        if (line2.startswith("*version")):  
            current_section=1  
        else:  
            if (line2.startswith("*threads")):  
                current_section=2  
            else:  
                if (line2.startswith("*methods")):  
                    current_section=3  
                else:   
                    if (line2.startswith("*end")):  
                        current_section=4  
                        break  
        continue  
    if current_section==2:  
        add_one_thread(line2)  
    if current_section==3:  
        add_one_method(line2)  

#Now handle the binary part  
input_file_name = sys.argv[1]
dot_file_name = input_file_name + ".dot"
pdf_file_name = input_file_name + ".pdf"

mybinfile = open(sys.argv[1], "rb")   
alldata = mybinfile.read()  
mybinfile.close()
pos=alldata.find("SLOW")  
offset,=struct.unpack("H",alldata[pos+6:pos+8])  
pos2=pos+offset #pos2 is where the record begin  
numofrecords = len(alldata) - pos2  
numofrecords = numofrecords / 14 

print "offset " + str(offset) + " records: " + str(numofrecords) + "\n"

for i in xrange(numofrecords):  
    add_one_record(alldata[pos2 + i * 14:pos2 + i * 14 + 14])  

my_stack=[0]  
for onerecord in all_records:  
    thread_id=onerecord[0];      
    method_id=onerecord[1];  
    action=onerecord[2];  
    time=onerecord[3];  
    if(action==0):  
        if(len(my_stack) > 1):  
            parent_method_id=my_stack[-1]  
            handle_one_call(parent_method_id,method_id)  
        my_stack.append(method_id)  
    else:  
        if(action==1):  
            if(len(my_stack) > 1):  
                my_stack.pop()  
gen_dot_script_file(dot_file_name)  
os.system("dot -Tpdf " + dot_file_name + " -o " + pdf_file_name);
 


