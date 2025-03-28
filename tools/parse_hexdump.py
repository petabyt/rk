def parse_hex_to_bin(input_file, output_file):
	with open(input_file, 'r') as f:
		lines = f.readlines()
		last_line = lines[-1].strip()
	
	byte_values = last_line.split()
	
	with open(output_file, 'wb') as f:
		for byte in byte_values:
			f.write(bytes([int(byte, 16)]))

input_file = 'log.txt'
output_file = 'dump.bin'

parse_hex_to_bin(input_file, output_file)
