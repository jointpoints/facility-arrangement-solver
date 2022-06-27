'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from collections import namedtuple
import json



SubjectGroup = namedtuple('SubjectGroup', ('input_capacity', 'output_capacity', 'area'))






# Point/facility






class Point:
	def __init__(self, x, y, area):
		self.x, self.y, self.area = x, y, area
		return



def load_facility(path: str):
	answer = {}
	try:
		with open(path, 'r') as f:
			answer = json.load(f)
		# Verify the format in metadata
		if (not isinstance(answer, dict)) \
		or (set(answer.keys()) != {'meta', 'stuff'}) \
		or (not isinstance(answer['meta'], dict)) \
		or (not isinstance(answer['stuff'], dict)) \
		or ('created_by' not in answer['meta']) \
		or ('spec' not in answer['meta']) \
		or ('type' not in answer['meta']) \
		or (answer['meta']['type'] != 'fasf'):
			raise RuntimeError('ERROR: Facility file must have a FASF format.')
		if (answer['meta']['spec'] != '1.0.0'):
			raise RuntimeError('ERROR: This version of FASF files is not supported.')
		# Turn all other values to Points
		answer = answer['stuff']
		for point_name in answer:
			answer[point_name] = Point(**answer[point_name])
	except:
		raise RuntimeError('ERROR: Invalid facility file.')
	return answer



def save_facility(facility: "dict[str, Point]", path: str):
	facility_meta = \
	{
		'created_by' : 'Facility Arrangement Solver for Python, 1.0.0',
		'type'       : 'fasf',
		'spec'       : '1.0.0',
	}
	try:
		with open(path, 'w') as f:
			json.dump({'stuff' : facility, 'meta': facility_meta}, f, indent='\t', sort_keys=True, cls=FASJSONEncoder)
	except:
		print('ERROR: Invalid path to save facility.')
	return






class TotalFlows:
	def __init__(self, groups: "dict[str, SubjectGroup]"):
		self.total_flow = {(i, j) : 0 for i in groups for j in groups}
		self.groups = tuple(groups.keys())
		return

	def set_flow(self, group_a: str, group_b: str, new_flow: int):
		if (group_a, group_b) not in self.total_flow:
			raise KeyError
		self.total_flow[(group_a, group_b)] = new_flow
		return
	
	def __getitem__(self, key: "tuple[str, str]"):
		return self.total_flow[key]
	
	def get_in_flow(self, group: str):
		return sum(self.total_flow[(i, group)] for i in self.groups)
	
	def get_out_flow(self, group: str):
		return sum(self.total_flow[(group, j)] for j in self.groups)






# JSON encoder and decoder






class FASJSONEncoder(json.JSONEncoder):
	def default(self, o):
		if isinstance(o, SubjectGroup):
			return {'input_capacity' : o.input_capacity, 'output_capacity' : o.output_capacity, 'area' : o.area}
		elif isinstance(o, Point):
			return {'x' : o.x, 'y' : o.y, 'area' : o.area}
		elif isinstance(o, TotalFlows):
			return {group_a : {o[(group_a, group_b)] for group_b in o.groups} for group_a in o.groups}
		return super().default(o)
