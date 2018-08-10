
#!/usr/bin/python

import sys
import math

   
print( 'Converting file: ', str(sys.argv[1]))

#read alog file line by line into array
with open(sys.argv[1]) as inp:
   data = inp.read().splitlines() 

#split each line at spaces keeping only the state and action info
reports = []
for report in data:
   split_report = report.split()
   state_action=[]
   for index in range(len(split_report)):
      if(split_report[index]=="state:" or split_report[index]=="action:"):
         state_action.append(split_report[index+1])
   #print "state_action"+str(tuple(state_action))
   if len(state_action)>0:
      reports.append(tuple(state_action))

#isolate state info for each node report and write to .csv
newfile = open(str(sys.argv[2]), 'w')

for index in range(len(reports)-1):
   state_elm=reports[index][0]
   action_elm=reports[index][1]
   next_state=reports[index+1][0]
   newfile.write(state_elm+" "+action_elm+" "+next_state+"\n")
   
newfile.close()

 
