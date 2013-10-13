#!/usr/bin/env python  
""" 
turn the traceview data into a jpg pic, showing methods call relationship 
This script is for Linux 64 bit, if run on 32 machine, please adjust
add_one_record function when unpack the data. 

Should be OO in the future
"""  
import sys  
import os  
import struct  
import re  
import subprocess

def generate_graph(commands):
	'''
	commands = ["/usr/bin/dot -Tpdf ./cam.trace.main.dot -o ./cam.trace.main.pdf",
	"/usr/bin/dot -Tpdf ./cam.trace.GC.dot -o ./cam.trace.GC.pdf",
	"/usr/bin/dot -Tpdf ./cam.trace.Signal_Catcher.dot -o ./cam.trace.Signal_Catcher.pdf",
	"/usr/bin/dot -Tpdf ./cam.trace.JDWP.dot -o ./cam.trace.JDWP.pdf",
	"/usr/bin/dot -Tpdf ./cam.trace.Compiler.dot -o ./cam.trace.Compiler.pdf",
	"/usr/bin/dot -Tpdf ./cam.trace.ReferenceQueueDaemon.dot -o ./cam.trace.ReferenceQueueDaemon.pdf",
	"/usr/bin/dot -Tpdf ./cam.trace.FinalizerDaemon.dot -o ./cam.trace.FinalizerDaemon.pdf",
	"/usr/bin/dot -Tpdf ./cam.trace.FinalizerWatchdogDaemon.dot -o ./cam.trace.FinalizerWatchdogDaemon.pdf",
	"/usr/bin/dot -Tpdf ./cam.trace.Binder_1.dot -o ./cam.trace.Binder_1.pdf",
	"/usr/bin/dot -Tpdf ./cam.trace.Binder_2.dot -o ./cam.trace.Binder_2.pdf"]
	'''
	for c in commands:
		print c
		subprocess.call(c, shell = True)

def extract_binary(trace_file_name, all_threads, all_methods):
	
	all_thread_records = {}

	# initialize the 
	for thread in all_threads.keys():
		all_thread_records[thread] = []
	#all_records = []

	mybinfile = open(trace_file_name, "rb")   
	alldata = mybinfile.read()
	mybinfile.close()
	pos=alldata.find("SLOW")
	
	offset,=struct.unpack("H", alldata[pos+6:pos+8])  
	pos2 = pos+offset #pos2 is where the record begin  
	numofrecords = len(alldata) - pos2  
	numofrecords = numofrecords / 14 

	print "offset " + str(offset) + " records: " + str(numofrecords) + "\n"

	for i in xrange(numofrecords):
		a_record = alldata[pos2 + i * 14:pos2 + i * 14 + 14]
		thread_id,=struct.unpack("H", a_record[0:2])
		tmp,=struct.unpack("I", a_record[2:6])  
		method_id= (tmp / 4) * 4;  
		method_action= tmp % 4;  
		time_offset,=struct.unpack("Q", a_record[6:])
		#print thread_id, method_id, method_action, time_offset
		all_thread_records[thread_id].append([thread_id, method_id, method_action, time_offset])
	
	#print all_thread_records
	
	return all_thread_records


def extract_all_threads(file_name):
	all_threads = {}  
	all_methods = {}  
	#Extract threads and methods. 
	current_section=0  
	for line in open(file_name):  
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
			fields = line2.split("\t")  
			all_threads[int(fields[0],10)]=fields  
		if current_section==3:  
			fields = line2.split("\t")  
			all_methods[int(fields[0],16)]=fields
			
	all_thread_records = extract_binary(file_name, all_threads, all_methods);
	
	# plot the call graph
	commands = []
	for thread_id in all_threads.keys():
		thread_name = all_threads[thread_id][1].replace(' ', '_')
		#print "generating graph for thread: " + thread_name
		
		dot_file_name = file_name + "." + thread_name + ".dot"
		pdf_file_name = file_name + "." + thread_name + ".pdf"
		
		all_records = all_thread_records[thread_id]
		parent_methods = {}  
		child_methods = {}  
		method_calls = {}

		call_stack = [0];
		# get method calls
		for onerecord in all_records:  
			thread_id=onerecord[0];      
			method_id=onerecord[1];  
			action=onerecord[2];  
			time=onerecord[3];
			
			if(action==0):  
				if(len(call_stack) > 1):  
					parent_method_id=call_stack[-1]
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
				call_stack.append(method_id)  
			else:  
				if(action==1):  
					if(len(call_stack) > 1):  
						call_stack.pop()
		
		# using python dynamic scope
		def gen_funcname(method_id):  
			r1=re.compile(r'[/$<>]')  
			str1=r1.sub("_", all_methods[method_id][1])  
			str2=r1.sub("_", all_methods[method_id][2])  
			return str1+"_"+str2

		myoutfile = open(dot_file_name, "w")  
		myoutfile.write("digraph vanzo {\n\n");  
		for a_record in all_methods.keys():  
			if parent_methods.has_key(a_record):  
				myoutfile.write(gen_funcname(a_record)+"  [shape=rectangle];\n")  
			else:  
				if child_methods.has_key(a_record):  
					myoutfile.write(gen_funcname(a_record)+"  [shape=ellipse];\n")  
		for a_record in method_calls.keys():  
			for two in method_calls[a_record]:  
				myoutfile.write(gen_funcname(a_record) + " -> " + gen_funcname(two) +  " [label=\"" + str(method_calls[a_record][two]) + "\"  fontsize=\"10\"];\n")
		myoutfile.write("\n}\n");  
		myoutfile.close
		c = "/usr/bin/dot -Tpdf ./" + dot_file_name + " -o ./" + pdf_file_name
		#TODO for some strange reason, I can not call it here
		#subprocess.call(c, shell = True)
		commands.append(c)
		
	generate_graph(commands)
		
if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "no input file specified"
		sys.exit()
	if not (os.path.exists(sys.argv[1])):  
		print "input file not exists"  
		sys.exit()
	extract_all_threads(sys.argv[1])
	#generate_graph()
	
	