import csv
import sys
import matplotlib.pyplot as plt
import math 
import numpy as np


ssn_in = sys.argv[1]

valid_set = set(str(i) for i in range(1,15))
valid_row = 0
found = 0
ssn_ary = []
ts_ary = []
ax_ary = []
ay_ary = []
az_ary = []
at_ary = []
v_ary = []



with open('cleanData.csv') as csvfile:
	reader = csv.DictReader(csvfile)
	for row in reader:
		cur_ssn = row['session']
		if row['session'] == sys.argv[1]:
			#print(row['session'],row['ts'],row['ax'],row['ay'],row['az'],row['at'],row['v'])
			ssn_ary.append(int(row['session']))
			ts_ary.append(int(row['ts']))
			ax_ary.append(float(row['ax']))
			ay_ary.append(float(row['ay']))
			az_ary.append(float(row['az']))
			at_ary.append(float(row['at']))
			v_ary.append(int(row['v']))
			valid_row+=1
			found = 1

slist = sorted(at_ary)
pull_thresh = (slist[-1] + slist[-2])/4
print(pull_thresh)


found = [i for i in range(len(at_ary)) if at_ary[i] > pull_thresh]
print(found)
first_pull_index = min(found)
print(first_pull_index)
begin_acc = np.mean([at_ary[i] for i in range(0,first_pull_index)])
print(begin_acc)


under_found = 1
catch_found = 0
under_count = 0
catch_begin = 0 
recovery_found = 0
rec_under_count = 0

catch_indexes = [];
pull_indexes = [];
pull_after_catch = 0

for i in range(len(at_ary)): 
	if at_ary[i] < begin_acc and i > first_pull_index:
		if not under_found: 
			catch_begin_index = i

		under_count+=1
		under_found = 1
		if not catch_found and under_count > 1 and pull_after_catch > 0:
			#catch_begin_index = i
			catch_found = 1
			catch_indexes.append(i)
			pull_after_catch = 0; 
	else:
		if at_ary[i] > pull_thresh and pull_after_catch < 1:
			pull_indexes.append(i)
			pull_after_catch = 1
		under_count = 0
		under_found = 0
		catch_found = 0;

#catch_indexes[0] = pull_indexes[0] - 1
#min_ts = min(ts_ary)
#for i in range(len(ts_ary)):
#	ts_ary[i] = ts_ary[i] - min_ts

#min_ts = min(ts_ary)
#for i in range(len(ts_ary)):
#	ts_ary[i] = ts_ary[i] - ts_ary[catch_indexes[0]]



#print(catch_begin_index) 


plt.figure(1)
plt.plot(ts_ary,at_ary)
plt.axis([ts_ary[pull_indexes[0] - 15],math.ceil(max(ts_ary)),math.floor(min(at_ary)),math.ceil(max(at_ary))])

lap_time = ts_ary[pull_indexes[-1] + 10]  - ts_ary[pull_indexes[0] - 15]
num_strokes = len(pull_indexes)
dist_per_stroke = 25.0/num_strokes
time_per_stroke = lap_time/num_strokes
print(lap_time)
print(num_strokes) 
print(dist_per_stroke)
print(time_per_stroke)
#dist_per_stroke = 25/

for i in range(len(pull_indexes)):
	plt.annotate('pull', xy=(ts_ary[pull_indexes[i]], at_ary[pull_indexes[i]]), xytext=(ts_ary[pull_indexes[i]] + 1000, at_ary[pull_indexes[i]] + 0.5),
           arrowprops=dict(facecolor='blue', shrink=0.05),
            )


for i in range(len(catch_indexes)):
	plt.annotate('catch', xy=(ts_ary[catch_indexes[i]], at_ary[catch_indexes[i]]), xytext=(ts_ary[catch_indexes[i]] + 1000, at_ary[catch_indexes[i]] + 0.5),
    	       arrowprops=dict(facecolor='red', shrink=0.05),
        	    )
txt = 'Run ' + str(ssn_in) + ' stroke rate = '+ str(time_per_stroke) + ' distance per stroke = ' + str(dist_per_stroke)
print(txt)
plt.title(txt)
plt.text(60, .025, txt)

plt.show()




