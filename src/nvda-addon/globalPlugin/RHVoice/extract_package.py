import re
import tarfile
import zipfile
import os
from pathlib import Path

VOICE_INFO_REGEX = re.compile(
    r"(?P<language>[a-z]+(_|-)?([a-z]+)?)(-|_)"
    r"(?P<name>[a-z]+)(-|_)"
    r"(?P<quality>(high|medium|low|x-low|x_low))",
    re.I
)

def extract_tar(file, destination):
	tar = tarfile.open(file, 'r:gz')
	filenames = {f.name: f for f in tar.getmembers()}
	try:
		onnx_file = next(filter(
			lambda fname:  Path(fname).suffix == ".onnx",
			filenames
		))
		config_file = next(filter(
			lambda fname:  Path(fname).suffix == ".json",
			filenames
		))
	except StopIteration:
		raise FileNotFoundError("Required files not found in archive")
	voice_info = VOICE_INFO_REGEX.match(Path(onnx_file).stem)
	if voice_info is None:
		raise FileNotFoundError("Required files not found in archive")
	info = voice_info.groupdict()
	voice_key = "-".join([
		info["language"],
		info["name"].replace("-", "_"),
		info["quality"].replace("-", "_"),
	])
	voice_folder_name = Path(destination).joinpath(voice_key)
	voice_folder_name.mkdir(parents=True, exist_ok=True)
	voice_folder_name = os.fspath(voice_folder_name)
	files_to_extract = [onnx_file, config_file]
	if "MODEL_CARD" in filenames:
		files_to_extract.append("MODEL_CARD")
	for file in files_to_extract:
		tar.extract(
			filenames[file],
			path=voice_folder_name,
			set_attrs=False,
		)
	return voice_key

def extract_zip(file, destination):
	if not zipfile.is_zipfile(file):
		raise ValueError(f"The package {file} is not valid.")
	with zipfile.ZipFile(file, 'r') as zip_ref:
		zip_ref.extractall(destination)