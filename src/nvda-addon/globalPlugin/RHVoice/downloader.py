# Copyright (C) 2024-2025  Mateo Cedillo <angelitomateocedillo@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import wx
import gui
import addonHandler
from languageHandler import getLanguageDescription
from .download import get_packages, capitalize_dash
from .constants import addon_ver
import requests
from requests.exceptions import ConnectionError
import threading
import shutil
import os
from .sound_lib import stream
from .sound_lib import output
import synthDriverHandler
from synthDrivers import RHVoice
import globalVars
from . import extract_package
import core
import ui

addonHandler.initTranslation()

class LangDownloadError(Exception):
	pass

class VoiceDownloadError(Exception):
	pass

class VoiceDownloader(wx.Frame):
	def __init__(self, *args, **kw):
		super(VoiceDownloader, self).__init__(*args, **kw)
		self.voices = None
		self.installed_voices = []
		self.have_voices = RHVoice.SynthDriver.check()
		pkg_dir_downloader = threading.Thread(target=self._get_packages_worker).start()
		if self.have_voices:
			self.synth = RHVoice.SynthDriver()
			self.voices = self.synth._get_availableVoices()
		self.o = output.Output(device=-1)
		self.audio_stream=None
		self.installed = False
		self.download_working_folder = os.path.join(globalVars.appArgs.configPath, "RHVoices")
		os.makedirs(self.download_working_folder, exist_ok=True)
		self.addons_dir = os.path.join(globalVars.appArgs.configPath, "addons")
		self.save_path = None
		self.english_save_path = None
		self.lang_save_path = None
		self.voice_name = None
		self.selected_lang = None
		self.pseudo_english = None
		self.english_language = None
		self.voice_ver = None
		self.english_lang_ver = None
		self.lang_ver = None
		self.nvda_lang_ver = None
		self.addon_ver = addon_ver # this is the addon build number.
		self.stop_event = threading.Event()
		self.download_Cancelled = False
		panel = wx.Panel(self)
		vbox = wx.BoxSizer(wx.VERTICAL)
		lang_label = wx.StaticText(panel, label=_("Language"))
		vbox.Add(lang_label, flag=wx.LEFT | wx.TOP, border=10)
		self.lang_choice = wx.Choice(panel, choices=[_("obtaining languages and voices from the server...")])
		vbox.Add(self.lang_choice, flag=wx.LEFT | wx.EXPAND, border=10)
		self.lang_choice.Bind(wx.EVT_CHOICE, self.on_language_select)
		self.lang_choice.SetSelection(0)
		voice_label = wx.StaticText(panel, label=_("Voice"))
		vbox.Add(voice_label, flag=wx.LEFT | wx.TOP, border=10)
		self.voice_choice = wx.Choice(panel, choices=[])
		vbox.Add(self.voice_choice, flag=wx.LEFT | wx.EXPAND, border=10)
		self.voice_choice.Bind(wx.EVT_CHOICE, self.on_voice_select)
		self.play_button = wx.Button(panel, label=_("Play"))
		vbox.Add(self.play_button, flag=wx.LEFT | wx.TOP, border=10)
		self.play_button.Bind(wx.EVT_BUTTON, self.on_play_hnd)
		self.download_button = wx.Button(panel, label=_("Install"))
		vbox.Add(self.download_button, flag=wx.LEFT | wx.TOP, border=10)
		self.download_button.Bind(wx.EVT_BUTTON, self.on_download_voice_hnd)
		self.progress_bar = wx.Gauge(panel, range=100)
		vbox.Add(self.progress_bar, flag=wx.LEFT | wx.EXPAND, border=10)
		panel.SetSizer(vbox)
		self.SetSize((400, 300))
		self.SetTitle(_("Voice downloader"))
		self.Centre()
		self.Bind(wx.EVT_CLOSE, self.close_resources)

	def _get_packages_worker(self):
		try:
			self.package_dir = get_packages()
		except ConnectionError:
			gui.messageBox(
				_("You will need to be connected to internet to use the downloader."), _("Error"),
				wx.OK | wx.ICON_ERROR
			)
			return
		self.update_languages()
		ui.message(_("Language and voices list got successfully"))

	def update_languages(self):
		languages_for_UI = [
			getLanguageDescription(lang["lang2code"]) if getLanguageDescription(lang["lang2code"]) is not None else lang["name"]
			for lang in self.package_dir["languages"]
		]
		self.lang_choice.Set(languages_for_UI)

	def on_language_select(self, event):
		selected_lang_index = self.lang_choice.GetSelection()
		selected_lang = self.package_dir["languages"][selected_lang_index]
		voices = selected_lang.get("voices", [])
		self.voice_choice.Clear()
		if self.voices is not None:
			self.installed_voices = self.voices.keys()
		for voice in voices:
			voice_name = capitalize_dash(voice["name"])
			if voice_name in self.installed_voices:
				voice_name += " " + _("(installed)")
			self.voice_choice.Append(voice_name)

	def on_voice_select(self, event):
		selected_voice = self.voice_choice.GetStringSelection()
		if _("(installed)") in selected_voice:
			self.download_button.SetLabel(_("Uninstall"))
			self.installed = True
		else:
			self.download_button.SetLabel(_("Install"))
			self.installed = False

	def on_play_hnd(self, event):
		player_thread = threading.Thread(target=self.on_play).start()

	def on_play(self):
		# If the demo or the voice is speaking, stop playing.
		if self.audio_stream is not None and self.audio_stream.is_playing:
			self.audio_stream.stop()
			return
		if self.synth.isSpeaking():
			self.synth.cancel()
			return
		selected_lang_index = self.lang_choice.GetSelection()
		selected_lang = self.package_dir["languages"][selected_lang_index]
		installed_message = selected_lang["testMessage"]
		selected_voice_index = self.voice_choice.GetSelection()
		selected_voice = selected_lang["voices"][selected_voice_index]
		demo_url = selected_voice["demoUrl"]
		if not self.installed:
			try:
				self.audio_stream=stream.URLStream(url =demo_url)
				self.audio_stream.play()
			except Exception as e:
				gui.messageBox(_("Failed to play demo")+": " +e, "Error", wx.OK | wx.ICON_ERROR)
		else:
			self.synth._set_voice(capitalize_dash(selected_voice["name"]))
			self.synth.speak(installed_message)

	def on_download_voice_hnd(self, event):
		if self.download_button.GetLabel() == _("Cancel"):
			self.stop_event.set()
			self.download_button.SetLabel(_("Install"))
		else:
			self.stop_event.clear()
			download_thread = threading.Thread(target=self.on_download_voice, daemon=True)
			download_thread.start()

	def download_function(self, mode, url, save_path):
		progress_range = {
			"language": (0, 50) if not self.pseudo_english else (0, 33),
			"second_language": (34, 50),
			"voice": (51, 100),
		}
		error_classes = {
			"language": LangDownloadError, "second_language": LangDownloadError, "voice": VoiceDownloadError
		}
		with requests.get(url, stream=True) as response:
			if response.status_code != 200:
				raise error_classes[mode]("No internet")
			total_length = int(response.headers.get("content-length", 0))
			total_length = max(total_length, 1)
			downloaded = 0
			start_progress, end_progress = progress_range[mode]
			try:
				with open(save_path, "wb") as f:
					for chunk in response.iter_content(chunk_size=8192):
						if self.stop_event.is_set():
							ui.message(_("Download cancelled"))
							f.close()
							os.remove(save_path)
							self.download_Cancelled = True
							return
						f.write(chunk)
						downloaded += len(chunk)
						progress = int(start_progress + (downloaded / total_length) * (end_progress - start_progress))
						#progress = 0 if total_length == 1 else int(downloaded / total_length * 100)
						wx.CallAfter(self.progress_bar.SetValue, progress)
			except:
				if mode == "language" or mode == "second_language":
					raise LangDownloadError("Could not download language pack")
				if mode == "voice":
					raise VoiceDownloadError("Could not download voice")
		# Reset download state:
		self.download_Cancelled = False

	def on_download_voice(self):
		selected_lang_index = self.lang_choice.GetSelection()
		self.selected_lang = self.package_dir["languages"][selected_lang_index]
		self.pseudo_english = self.selected_lang.get("pseudoEnglish", False)
		if self.pseudo_english:
			english_lookup = next(
				(lang for lang in self.package_dir["languages"] if lang["name"] == "English"),
				None
			)
			if english_lookup:
				self.english_language = english_lookup
			english_lang_url = self.english_language["dataUrl"]
			self.english_lang_ver = f'{self.english_language["version"]["major"]}.{self.english_language["version"]["minor"]}'
			self.english_save_path = os.path.join(
				self.download_working_folder, f"RHVoice-language-{self.english_language['name']}-v{self.english_lang_ver}.zip"
			)
		lang_url = self.selected_lang["dataUrl"]
		self.lang_ver = f'{self.selected_lang["version"]["major"]}.{self.selected_lang["version"]["minor"]}'
		self.nvda_lang_ver = str(1000*int(self.selected_lang["version"]["major"]) + int(self.selected_lang["version"]["minor"]))
		self.lang_save_path = os.path.join(
			self.download_working_folder, f"RHVoice-language-{self.selected_lang['name']}-v{self.lang_ver}.zip"
		)
		selected_voice_index = self.voice_choice.GetSelection()
		selected_voice = self.selected_lang["voices"][selected_voice_index]
		voice_url = selected_voice["dataUrl"]
		self.voice_ver = f'{selected_voice["version"]["major"]}.{selected_voice["version"]["minor"]}'
		self.voice_name = capitalize_dash(selected_voice["name"])
		self.save_path = os.path.join(
			self.download_working_folder, 
			f"RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}-v{self.voice_ver}.zip"
		)
		if not self.installed:
			self.download_button.SetLabel(_("Cancel"))
			# Lang Pack:
			ui.message(_("Downloading {lng} language package...").format(lng=self.selected_lang['name']))
			try:
				self.download_function("language", lang_url, self.lang_save_path)
				if self.pseudo_english and not self.download_Cancelled:
					ui.message(_("Downloading second language package..."))
					self.download_function("second_language", english_lang_url, self.english_save_path)
			except LangDownloadError:
				gui.messageBox(
					_("Failed to download {language} language pack, which is required to run the voice(s) propperly.").format(language=self.selected_lang['name']),
					_("Error"), wx.OK | wx.ICON_ERROR
				)
			# Voice:
			if not self.download_Cancelled:
				ui.message(_("Downloading voice {voice}...").format(voice=self.voice_name))
				try:
					self.download_function("voice", voice_url, self.save_path)
				except VoiceDownloadError as e:
					gui.messageBox(
						_("Failed to download {voice}. Error message: {err}").format(voice=self.voice_name, err=e), 
						_("Error"), wx.OK | wx.ICON_ERROR
					)
					self.download_button.SetLabel(_("Install"))
					return
				self.unzip_and_install()
				self.generate_manifest()
				self.download_button.SetLabel(_("Uninstall"))
				restart = gui.messageBox(
					_(
						"Downloaded voice {voice} successfully!\nYou must restart NVDA to update your list of voices. Would you like to do it now?"
					).format(voice=self.voice_name),
					_("Success"),
					wx.YES_NO | wx.ICON_INFORMATION
				)
				if restart == wx.YES:
					core.restart()
		else:
			confirm = gui.messageBox(
				_("Are you sure you want to remove {voice}'s voice?").format(voice=self.voice_name),
				_("Confirm"), wx.YES_NO | wx.ICON_INFORMATION
			)
			if confirm == wx.YES:
				synth = synthDriverHandler.getSynth()
				if synth.voice != self.voice_name:
					try:
						shutil.rmtree(f"{self.addons_dir}/RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}")
					except:
						gui.messageBox(
							_("Could not remove {voice}.").format(voice=self.voice_name),
							_("Error"), wx.ICON_ERROR
						)
						return
				else:
					gui.messageBox(
						_("You are using {voice}, please change to other voice before removing.").format(voice=self.voice_name),
						_("Voice in use"), wx.ICON_ERROR
					)
					return
				gui.messageBox(
					_("{voice} was removed successfully.").format(voice=self.voice_name),
					_("Done"), wx.ICON_INFORMATION
				)

	def unzip_and_install(self):
		# Lang pack:
		extract_package.extract_zip(
			self.lang_save_path,
			f"{self.addons_dir}/RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}/langdata"
		)
		if self.pseudo_english:
			# Second lang pack:
			extract_package.extract_zip(
				self.english_save_path,
				f"{self.addons_dir}/RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}/lang2data"
			)
		# Voice:
		extract_package.extract_zip(
			self.save_path,
			f"{self.addons_dir}/RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}/data"
		)

	def generate_manifest(self):
		voice_prephix = f"RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}"
		voice_dir = f"{self.addons_dir}/{voice_prephix}"
		voice_lang = self.selected_lang['name']
		with open(f"{voice_dir}/manifest.ini", "w", encoding="utf-8") as f:
			f.write(
				f'name = {voice_prephix}\nsummary = "RHVoice {self.voice_name}, {voice_lang} voice"\ndescription = """RHVoice {self.voice_name}, {voice_lang} voice"""\nauthor = Contributors to the RHVoice project <rhvoice@rhvoice.org>\nurl = https://rhvoice.org\nversion = {self.voice_ver}.{self.nvda_lang_ver}.{self.addon_ver}\nminimumNVDAVersion = 2021.1.0\nlastTestedNVDAVersion = 2099.4.0'
			)

	def close_resources(self, event):
		self.o.free()
		self.Destroy()