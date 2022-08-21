'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
import json






# Componenets






class SubjectGroup:
	def __init__(self, input_capacity: int, output_capacity: int, area: int):
		self.input_capacity, self.output_capacity, self.area = input_capacity, output_capacity, area
		return



class Point:
	def __init__(self, x: int, y: int, area: int):
		self.x, self.y, self.area = x, y, area
		return



class TotalFlows:
	def __init__(self, groups: "dict[str, SubjectGroup]" = None, flows: "dict[str, dict[str, int]]" = None):
		if groups != None:
			self.total_flow = {(i, j) : 0 for i in groups for j in groups}
			self.groups = tuple(groups.keys())
		else:
			self.total_flow = {(i, j) : flows[i][j] for i in flows for j in flows}
			self.groups = tuple(flows.keys())
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
			return {group_a : {group_b: o[(group_a, group_b)] for group_b in o.groups} for group_a in o.groups}
		return super().default(o)






# I/O






def fas_load(path: str, type: str):
	assert type in {'fasf', 'fasg', 'fast', 'fasd'}
	type_class = \
	{
		'fasf' : Point,
		'fasg' : SubjectGroup,
		'fast' : TotalFlows,
		'fasd' : None
	}
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
		or (answer['meta']['type'] != type):
			raise RuntimeError(f'ERROR: File must have a {type.upper()} format.')
		if (answer['meta']['spec'] != '1.0.0'):
			raise RuntimeError('ERROR: This version of file is not supported.')
		# Deserialise the data
		answer = answer['stuff']
		if type == 'fast':
			answer = TotalFlows(flows=answer)
		else:
			for name in answer:
				answer[name] = type_class[type](**answer[name])
	except:
		raise RuntimeError(f'ERROR: Invalid {type.upper()} file.')
	return answer



def fas_save(object, path: str, type: str):
	assert type in {'fasf', 'fasg', 'fast', 'fasd'}
	facility_meta = \
	{
		'created_by' : 'Facility Arrangement Solver for Python, 1.0.0',
		'type'       : type,
		'spec'       : '1.0.0',
	}
	try:
		with open(path, 'w') as f:
			json.dump({'stuff' : object, 'meta': facility_meta}, f, indent='\t', sort_keys=True, cls=FASJSONEncoder)
	except:
		raise RuntimeError('ERROR: Invalid path.')
	return
