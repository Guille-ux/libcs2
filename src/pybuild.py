import json
import os
import subprocess

print("[+] Pybuild [+]")
try:
	with open("pybuild.json") as f:
		data=json.load(f)
	c_compiler=data["cpath"]
	c_flags=data["cflags"]
	build_dir=data["build-dir"]
	source_dir=data["source-dir"]
	
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)
	
	for file in os.listdir(source_dir):
		if file.endswith(".c"):
			src_path=os.path.join(source_dir, file)
			obj_name = os.path.splitext(file)[0] + ".o"
			obj_path = os.path.join(build_dir, obj_name)
			print(f"[+] Compiling {file} -> {obj_path} ... [+]")
		result=subprocess.run(
			[c_compiler] + c_flags.split() + ["-c", src_path, "-o", obj_path],
			capture_output=True,
			text=True
		)
		
		if result.returncode != 0:
			print(f"[!] Error Compiling {file} [!]")
			print(result.stderr)
		else:
			print(f"[+] {file} Compiled Successfully [+]")
except FileNotFoundError:
	print("[+] ´pybuild.json´ not found [+]")
else:
    pass
