#!/usr/bin/env python3
# Shoddy directory-wise run script

import os
import json
import sys
from pathlib import Path

from bytes_encoder import BytesEncoder

from tf2_dem_py.flags import FLAGS
from tf2_dem_py.demo_parser import DemoParser, ParserError

p = sys.argv[1]
cwd = Path.cwd()

for i, file in enumerate(os.path.join(p, i) for i in os.listdir(p) if i.endswith(".dem")):
	name = os.path.splitext(os.path.basename(file))[0]
	print(f"{name} ...", end="", flush=True)

	dp = DemoParser(FLAGS.CHAT | FLAGS.COMPACT_CHAT)
	try:
		res = dp.parse(file)
	except ParserError as exc:
		print("!!!")
		print(exc)
		print("!!!\n")
		continue

	with (cwd / "_res" / f"{name}.json").open("w") as h:
		json.dump(res, h, cls = BytesEncoder)

	with (cwd / "_res" / f"{name}.txt").open("w", encoding="utf-8") as h:
		for m in res["chat"]:
			if not m[0]: # Non-normal message
				continue
			t = [None, None, None]
			for i, x in enumerate(m[3:6]):
				try:
					t[i] = x.decode("utf-8")
				except UnicodeDecodeError:
					t[i] = "<DECODING FAILURE>"
			chat, name, text = t
			h.write(f"*{chat:<20}* {name:<31}: {text}\n")

	# print(res)
