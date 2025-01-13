import json

def format_name(name):
	return name.lower().replace("vop2_posto_", "")

def format_offset(offset):
	return f"0{int(offset, 16):02x}"

def main():
	with open("post.json", "r") as file:
		data = json.load(file)
	
	for entry in data:
		name = format_name(entry["name"])
		offset = format_offset(entry["offset"])
		print(f"@.{name}(offset:{offset})[i4]")

if __name__ == "__main__":
	main()
