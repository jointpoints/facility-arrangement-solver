'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from sys import argv
from os import system
from time import time
from math import ceil
import random
import networkx
import json



def generate_instance(idx):
	# Choose the number of subject groups
	subject_group_count = random.randint(2, 10)
	# Choose the number of subjects in each group
	subject_count = {i : random.randint(1, 5) for i in range(subject_group_count)}
	# Construct a random production pipeline
	pipeline = networkx.gn_graph(subject_group_count)
	total_flows = {i : {j : 0 for j in range(subject_group_count)} for i in range(subject_group_count)}
	group_queue = [i for i in range(subject_group_count) if pipeline.in_degree(i) == 0]
	total_input = {i : 100 if i in group_queue else 0 for i in range(subject_group_count)}
	while group_queue != []:
		i = group_queue[0]
		for edge in pipeline.edges(i):
			total_flows[i][edge[1]] = total_input[i] // pipeline.out_degree(i)
			total_input[edge[1]] += total_flows[i][edge[1]]
			group_queue.append(edge[1])
		del group_queue[0]
	# Determine the capacities and areas of the subject groups
	capacities = {i : ceil(total_input[i] / subject_count[i]) for i in range(subject_group_count)}
	areas = {i : random.randint(1, 2) for i in range(subject_group_count)}
	# Save the instance
	meta = {'created_by' : 'FAS benchmark random instance generator', 'spec' : '1.0.0', 'type' : 'fasg'}
	stuff = {str(i) : {'area' : areas[i], 'input_capacity' : capacities[i], 'output_capacity' : capacities[i]} for i in range(subject_group_count)}
	with open(f'benchmark_random_{idx}.fasg', 'w') as f:
		json.dump({'stuff' : stuff, 'meta': meta}, f, indent='\t', sort_keys=True)
	meta['type'] = 'fast'
	stuff = {str(i) : {str(j) : total_flows[i][j] for j in range(subject_group_count)} for i in range(subject_group_count)}
	with open(f'benchmark_random_{idx}.fast', 'w') as f:
		json.dump({'stuff' : stuff, 'meta': meta}, f, indent='\t', sort_keys=True)
	return



def main():
	random.seed(218199)
	valid_instances_count = int(argv[1])
	if len(argv) == 2:
		for valid_instance_i in range(valid_instances_count):
			generate_instance(valid_instance_i + 1)
	else:
		for valid_instance_i in range(valid_instances_count):
			generate_instance(0)
			for a in ('linear --forcevanilla', 'cpr_linear --forcevanilla', 'linear', 'cpr_linear', 'linear_gfred', 'cpr_linear_gfred'):
				for h in (5, 7, 9, 11, 13):
					start_time = time()
					system(f'{argv[2]} ../fas/fas.py -o arrangement.sol -f g{h}:1x{h}:1x2 -g benchmark_random_0.fasg -t benchmark_random_0.fast -d m1 -a {a}')
					runtime = time() - start_time
					with open(f'report_{valid_instance_i + 1}.txt', 'a') as f:
						f.write(f'{a} {h} {runtime}\n')
					if runtime >= 1200:
						break
	return



if __name__ == '__main__':
	main()
