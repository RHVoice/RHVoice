# Copyright (C) 2023  Olga Yakovleva <olga@rhvoice.org>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


import argparse
import babel
import base64
import datetime
import filelock
import hashlib
import json
import locale
import os
import pathlib
import requests
import shutil
import sys
import zipfile

def get_args():
    global args
    p=argparse.ArgumentParser()
    g=p.add_mutually_exclusive_group(required=True)
    g.add_argument("-l", "--installed", help="List installed voices", action="store_true")
    g.add_argument("-a", "--all", help="List all available voices", action="store_true")
    g.add_argument("-i", "--install", help="Install a voice")
    g.add_argument("-r", "--uninstall", help="Uninstall a voice")
    g.add_argument("-u", "--update", help="Check for and install updates", action="store_true")
    args=p.parse_args()

pkg_dir_url="https://rhvoice.org/download/packages-1.16.json"
pkg_dir=None

installed={}
voices={}

eng_pkg=None

def mkdir(p):
    p.mkdir(parents=True, exist_ok=True)

def make_voice_index():
    global voices
    global eng_pkg
    for lang in pkg_dir["languages"]:
        if lang["name"]=="English":
            eng_pkg=lang
        for v in lang["voices"]:
            key=v["name"].lower()
            voices[key]=(v, lang)

def load_cached_pkg_dir():
    try:
        with pkg_dir_file.open("r", encoding="utf-8") as f:
            return json.load(f)
    except:
        return None

def should_update_pkg_dir():
    if not pkg_dir:
        return True
    if args.update:
        return True
    if not (args.install or args.all):
        return False
    then=datetime.datetime.fromisoformat(pkg_dir["timestamp"])+pkg_dir["ttl"]*datetime.timedelta(seconds=1)
    now=datetime.datetime.now(datetime.timezone.utc)
    if then>now:
        return False
    return True

def download_pkg_dir():
    global pkg_dir
    print("Downloading package directory")
    try:
        t=datetime.datetime.now(datetime.timezone.utc)
        r=requests.get(pkg_dir_url)
        r.raise_for_status()
        new_pkg_dir=r.json()
        new_pkg_dir["timestamp"]=t.isoformat()
        tmp_file =pkg_dir_file.with_suffix(".new")
        with tmp_file.open("w", encoding="utf-8") as f:
            json.dump(new_pkg_dir, f, ensure_ascii=False)
        tmp_file.replace(pkg_dir_file)
        pkg_dir=new_pkg_dir
        return True
    except Exception as e:
        print("Failed to get the package directory:",e)
        return False

def get_lang_display_name(lng):
    tag=lng["lang2code"]
    loc=babel.Locale(tag)
    name=loc.display_name
    return name

def list_all():
    for lng in sorted(pkg_dir["languages"], key=lambda l: l["lang3code"]):
        for v in sorted(lng["voices"], key=lambda x: (x.get("ctry3code", ""), x.get("accent", ""), x["name"])):
            print("{} ({})".format(v["name"], get_lang_display_name(lng)))

def get_pkg_id(p):
    return p.get("id", p["name"].lower().replace("-","_"))

def load_inst_info():
    global installed
    try:
        with inst_info_file.open("r", encoding="utf-8") as f:
            installed=json.load(f)
    except:
        pass

def save_inst_info():
    tmp_file=inst_info_file.with_suffix(".new")
    with tmp_file.open("w", encoding="utf-8") as f:
        json.dump(installed, f)
    tmp_file.replace(inst_info_file)

def get_pkg_inst_path(pkg, voice_or_lang):
    id=get_pkg_id(pkg)
    type="voice" if voice_or_lang else "language"
    name="{}-{}-{}-{}".format(type, id, pkg["version"]["major"], pkg["version"]["minor"])
    return (inst_dir / name)

def get_pkg_data_path(pkg, voice_or_lang):
    type="voices" if voice_or_lang else "languages"
    id=get_pkg_id(pkg)
    return ((data_dir / type) / id)

def get_rel_inst_path(p):
    return (pathlib.Path("../../../install") / p.name)

def install_package(pkg, voice_or_lang):
    name=pkg["name"].replace("-", " ")
    id=get_pkg_id(pkg)
    download_path=state_dir / "download.zip"
    md5=base64.b64decode(pkg["dataMd5"])
    md5digest=hashlib.md5()
    with download_path.open("wb") as f:
        print("Downloading {} {}.{}".format(name, pkg["version"]["major"], pkg["version"]["minor"]))
        r=requests.get(pkg["dataUrl"])
        r.raise_for_status()
        for b in r.iter_content(8092):
            md5digest.update(b)
            f.write(b)
    assert(md5==md5digest.digest())
    print("Installing {} {}.{}".format(name, pkg["version"]["major"], pkg["version"]["minor"]))
    inst_path=get_pkg_inst_path(pkg, voice_or_lang)
    if inst_path.exists():
        shutil.rmtree(inst_path)
    mkdir(inst_path)
    zipfile.ZipFile(download_path).extractall(inst_path)
    if id in installed:
        uninstall_package(pkg, voice_or_lang)
    data_path=get_pkg_data_path(pkg, voice_or_lang)
    mkdir(data_path.parent)
    if data_path.exists():
        data_path.unlink()
    data_path.symlink_to(get_rel_inst_path(inst_path))
    installed[id]=pkg["version"]
    save_inst_info()
    if download_path.exists():
        download_path.unlink()

def install():
    key=args.install.lower()
    if key not in voices:
        print("Unknown voice", args.install)
        return
    v, lng=voices[key]
    if get_pkg_id(lng) not in installed:
        install_package(lng, False)
    if lng.get("pseudoEnglish", False) and get_pkg_id(eng_pkg) not in installed:
        install_package(eng_pkg, False)
    if get_pkg_id(v) in installed:
        print("Already installed")
    else:
        install_package(v, True)

def list_installed():
    names=[]
    for lng in pkg_dir["languages"]:
        for v in lng["voices"]:
            if get_pkg_id(v) in installed:
                names.append(v["name"])
    if not names:
        print("No installed voices")
    else:
        names.sort()
        for name in names:
            print(name)

def uninstall_package(pkg, voice_or_lang):
    id=get_pkg_id(pkg)
    attrs={"id":id, "name":pkg["name"]}
    attrs["version"]=installed[id]
    print("Uninstalling {} {}.{}".format(pkg["name"], attrs["version"]["major"], attrs["version"]["minor"]))
    data_path=get_pkg_data_path(attrs, voice_or_lang)
    inst_path=get_pkg_inst_path(attrs, voice_or_lang)
    del installed[id]
    save_inst_info()
    if data_path.exists():
        data_path.unlink()
    if inst_path.exists():
        shutil.rmtree(inst_path)

def uninstall():
    key=args.uninstall.lower()
    if key not in voices:
        print("No such voice")
        return
    v, lng=voices[key]
    if get_pkg_id(v) not in installed:
        print("This voice is not installed")
        return
    uninstall_package(v, True)
    if lng["name"]=="English":
        return
    for v1 in lng["voices"]:
        if get_pkg_id(v1) in installed:
            return
    uninstall_package(lng, False)

def is_new_version(v1, v2):
    if v2["major"]>v1["major"]:
        return True
    if v2["major"]==v1["major"] and v2["minor"]>v1["minor"]:
        return True
    return False

def maybe_update_package(pkg, voice_or_lang):
    id=get_pkg_id(pkg)
    if id not in installed:
        return
    v1=installed[id]
    v2=pkg["version"]
    if is_new_version(v1, v2):
        install_package(pkg, voice_or_lang)

def update():
    for lng in pkg_dir["languages"]:
        maybe_update_package(lng, False)
        for v in lng["voices"]:
            maybe_update_package(v, True)

def main():
    global state_dir, pkg_dir_file, pkg_dir, args, inst_dir, inst_info_file, data_dir
    get_args()
    var_state_dir=os.environ.get("RHVOICE_STATE_DIR", "/var/lib/RHVoice")
    state_dir=pathlib.Path(var_state_dir)
    mkdir(state_dir)
    pkg_dir_file=state_dir / "packages.json"
    inst_dir=state_dir / "install"
    inst_info_file=state_dir / "installed.json"
    data_dir=(state_dir / "data") / "RHVoice"
    lock_file=state_dir / "vm.lock"
    lock=filelock.FileLock(lock_file)
    try:
        with lock.acquire(timeout=1):
            pkg_dir=load_cached_pkg_dir()
            if should_update_pkg_dir():
                download_pkg_dir()
            if not pkg_dir:
                print("No package directory")
                sys.exit(1)
            make_voice_index()
            load_inst_info()
            if args.all:
                    list_all()
            elif args.installed:
                list_installed()
            elif args.install:
                install()
            elif args.uninstall:
                uninstall()
            elif args.update:
                update()
    except filelock.Timeout:
        print("Some other instance of this application is currently running")



