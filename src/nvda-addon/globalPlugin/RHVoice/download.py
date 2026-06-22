"""RHVoice downloader"""
import requests
import json
import hashlib
import zipfile

import base64
from .constants import pkg_dir_url

def get_packages():
	response = requests.get(pkg_dir_url)
	return json.loads(response.text)

def calculate_md5(input_file):
	with open(input_file, 'rb') as f:
		data = f.read()
		md5hash = hashlib.md5(data).hexdigest()
	return md5hash

def calculate_base64(md5_hex):
	base64_hash = base64.b64encode(bytes.fromhex(md5_hex)).decode()
	return base64_hash

def check_pack_hash(zip_path, json_hash):
	""" This can be used for the voices and languages """
	if not zipfile.is_zipfile(zip_path):
		raise Exception("Could not check hash for this operation. The data pack is not a zip file.")
	if calculate_base64(calculate_md5(zip_path)) == json_hash:
		return True
	else:
		return False

def capitalize_dash(text):
	return '-'.join([word.capitalize() for word in text.split('-')])