import wx
import gui
from .download import get_packages
import requests
from requests.exceptions import ConnectionError
import threading 
import shutil
import os
from .sound_lib import stream
from .sound_lib import output
from synthDrivers import RHVoice
import globalVars
from . import extract_package
import core

class VoiceDownloader(wx.Frame):
	def __init__(self, *args, **kw):
		super(VoiceDownloader, self).__init__(*args, **kw)
		self.voices = None
		self.installed_voices = []
		self.have_voices = RHVoice.SynthDriver.check()
		if self.have_voices:
			self.synth = RHVoice.SynthDriver()
		try:
			self.package_dir = get_packages()
		except ConnectionError:
			gui.messageBox("You will need to be connected to internet to use the downloader.", "Error", wx.OK | wx.ICON_ERROR)
			return
		if self.have_voices:
			self.voices = self.synth._get_availableVoices()
		self.o = output.Output(device=-1)
		self.audio_stream=None
		self.installed = False
		self.download_working_folder = os.path.join(globalVars.appArgs.configPath, "RHVoices")
		os.makedirs(self.download_working_folder, exist_ok=True)
		self.addons_dir = os.path.join(globalVars.appArgs.configPath, "addons")
		self.save_path = None
		self.lang_save_path = None
		self.voice_name = None
		self.selected_lang = None
		self.voice_ver = None
		self.lang_ver = None
		panel = wx.Panel(self)
		vbox = wx.BoxSizer(wx.VERTICAL)
		lang_label = wx.StaticText(panel, label="Language")
		vbox.Add(lang_label, flag=wx.LEFT | wx.TOP, border=10)
		self.lang_choice = wx.Choice(panel, choices=[lang["name"] for lang in self.package_dir["languages"]])
		vbox.Add(self.lang_choice, flag=wx.LEFT | wx.EXPAND, border=10)
		self.lang_choice.Bind(wx.EVT_CHOICE, self.on_language_select)
		self.lang_choice.SetSelection(0)
		voice_label = wx.StaticText(panel, label="Voice")
		vbox.Add(voice_label, flag=wx.LEFT | wx.TOP, border=10)
		self.voice_choice = wx.Choice(panel, choices=[])
		vbox.Add(self.voice_choice, flag=wx.LEFT | wx.EXPAND, border=10)
		self.voice_choice.Bind(wx.EVT_CHOICE, self.on_voice_select)
		self.play_button = wx.Button(panel, label="Play")
		vbox.Add(self.play_button, flag=wx.LEFT | wx.TOP, border=10)
		self.play_button.Bind(wx.EVT_BUTTON, self.on_play)
		self.download_button = wx.Button(panel, label="Install")
		vbox.Add(self.download_button, flag=wx.LEFT | wx.TOP, border=10)
		self.download_button.Bind(wx.EVT_BUTTON, self.on_download_voice_hnd)
		panel.SetSizer(vbox)
		self.SetSize((400, 300))
		self.SetTitle("Voice downloader")
		self.Centre()
		self.Bind(wx.EVT_CLOSE, self.close_resources)

	def on_language_select(self, event):
		selected_lang_index = self.lang_choice.GetSelection()
		selected_lang = self.package_dir["languages"][selected_lang_index]
		voices = selected_lang.get("voices", [])
		self.voice_choice.Clear()
		if self.voices is not None:
			self.installed_voices = [voice.lower() for voice in self.voices.keys()]
		for voice in voices:
			voice_name = voice["id"]
			if voice_name in self.installed_voices:
				voice_name += " (installed)"
			self.voice_choice.Append(voice_name)
		self.voice_choice.SetSelection(0)

	def on_voice_select(self, event):
		selected_voice = self.voice_choice.GetStringSelection()
		if " (installed)" in selected_voice:
			self.download_button.SetLabel("Uninstall")
			self.installed = True
		else:
			self.download_button.SetLabel("Install")
			self.installed = False

	def on_play(self, event):
		selected_lang_index = self.lang_choice.GetSelection()
		selected_lang = self.package_dir["languages"][selected_lang_index]
		selected_voice_index = self.voice_choice.GetSelection()
		selected_voice = selected_lang["voices"][selected_voice_index]
		demo_url = selected_voice["demoUrl"]
		try:
			self.audio_stream=stream.URLStream(url =demo_url)
			self.audio_stream.play()
		except Exception as e:
			gui.messageBox(f"Failed to play demo: {e}", "Error", wx.OK | wx.ICON_ERROR)

	def on_download_voice_hnd(self, event):
		download_thread = threading.Thread(target=self.on_download_voice)
		download_thread.start()

	def on_download_voice(self):
		selected_lang_index = self.lang_choice.GetSelection()
		self.selected_lang = self.package_dir["languages"][selected_lang_index]
		lang_url = self.selected_lang["dataUrl"]
		self.lang_ver = f'{self.selected_lang["version"]["major"]}.{self.selected_lang["version"]["minor"]}'
		self.lang_save_path = os.path.join(self.download_working_folder, f"RHVoice-language-{self.selected_lang['name']}-v{self.lang_ver}.zip")
		selected_voice_index = self.voice_choice.GetSelection()
		selected_voice = self.selected_lang["voices"][selected_voice_index]
		voice_url = selected_voice["dataUrl"]
		self.voice_ver = f'{selected_voice["version"]["major"]}.{selected_voice["version"]["minor"]}'
		self.voice_name = selected_voice["id"]
		self.save_path = os.path.join(self.download_working_folder, f"RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}-v{self.voice_ver}.zip")
		if not self.installed:
			self.download_button.SetLabel("Cancel")
			# Lang Pack:
			try:
				response = requests.get(lang_url)
				with open(self.lang_save_path, "wb") as f:
					f.write(response.content)
			except Exception as e:
				gui.messageBox(f"Failed to download {self.selected_lang['name']} language pack, which is required to run the voice(s) propperly.).", "Error", wx.OK | wx.ICON_ERROR)
			# Voice:
			try:
				response = requests.get(voice_url)
				with open(self.save_path, "wb") as f:
					f.write(response.content)
				self.unzip_and_install()
				self.generate_manifest()
				self.download_button.SetLabel("Uninstall")
				restart = gui.messageBox(
					f"Downloaded voice {self.voice_name} successfully!\nYou must restart NVDA to update your list of voices. Would you like to restart NVDA now?",
					"Success",
					wx.YES_NO | wx.ICON_INFORMATION
				)
				if restart == wx.YES:
					core.restart()
			except Exception as e:
				gui.messageBox(f"Failed to download {self.voice_name}. Error message: {e}", "Error", wx.OK | wx.ICON_ERROR)
				self.download_button.SetLabel("Install")
		else:
			confirm = gui.messageBox(
				f"Are you sure you want to remove {self.voice_name}'s voice?",
				"Confirm",
				wx.YES_NO | wx.ICON_INFORMATION
			)
			if confirm == wx.YES:
				self.synth = synthDriverHandler.getSynth()
				if self.synth.voice != self.voice_name:
					try:
						shutil.rmtree(f"{self.addons_dir}/RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}")
					except:
						gui.messageBox(
							f"Could not remove {self.voice_name}.",
							"Error",
							wx.ICON_ERROR
						)
						return
				else:
					gui.messageBox(
						f"You are using {self.voice_name}, please change to other voice before removing.",
						"Voice in use",
						wx.ICON_ERROR
					)
					return
				gui.messageBox(
					f"{self.voice_name} was removed successfully.",
					"Done",
					wx.ICON_INFORMATION
				)

	def unzip_and_install(self):
		# Lang pack:
		# Todo: handle cases of multilingual voices, for example Vietnamese and English.
		extract_package.extract_zip(self.lang_save_path, f"{self.addons_dir}/RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}/langdata")
		# Voice:
		extract_package.extract_zip(self.save_path, f"{self.addons_dir}/RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}/data")

	def generate_manifest(self):
		voice_prephix = f"RHVoice-voice-{self.selected_lang['name']}-{self.voice_name}"
		voice_dir = f"{self.addons_dir}/{voice_prephix}"
		voice_lang = self.selected_lang['name']
		with open(f"{voice_dir}/manifest.ini", "w", encoding="utf-8") as f:
			f.write(f'name = {voice_prephix}\nsummary = "RHVoice {self.voice_name}, {voice_lang} voice"\ndescription = """RHVoice {self.voice_name}, {voice_lang} voice"""\nauthor = Contributors to the RHVoice project <rhvoice@rhvoice.org>\nurl = https://rhvoice.org\nversion = {self.voice_ver}.{self.lang_ver}\nminimumNVDAVersion = 2012.2.0\nlastTestedNVDAVersion = 2099.4.0')

	def close_resources(self, event):
		self.o.free()
		self.Destroy()