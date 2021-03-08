import json

class BytesEncoder(json.JSONEncoder):
	def default(self, obj):
		if isinstance(obj, bytes):
			return f"{obj!r}"[2:-1]
		return super().default(obj)
