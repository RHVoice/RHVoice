# Creating a new voice for RHVoice.

# Introduction
Below tutorial is a detailed guide of creating new voice for RHVoice synthesizer.
It assumes that user has some practical skills regarding command-line environments, copying and editing files, and so on.
Also the access to some Linux system is needed, with sudo privileges for installing packages.
Windows WSL2 is enough.

Also the knowledge of digital audio processing, recording, mastering, cutting is obvious as needed for preparing new voice audio files, but in this tutorial we'll work with ready to use datasets.

This tutorial  is divided into 3 parts:

- Part 1. Installation of common tools.
- Part 2. Running original HTS demo CMU ARCTIC SLT, to be sure, that everything works as it should.
- Part 3. Generating the voice for RHVoice.

## Warning: work in progress
This text is a work in progress, version 0.01.
Part 1 and 2 are complete, but some important questions related to RHVoice, are still open.

- What did I wrong? After voice-building-utils export-voice and calling RHVoice-test -p Myvoice I get Error: HTS106_Model_load_pdf: Failed to load header of pdfs.
- Is 16000 voice variant needed if we have 24000, and how to generate it without repeating audio importing and training?
- What is purpose of realign step in voice-building-utils?
- How to solve the [Assertion error during F0 extraction](#assertion_error) and is proposed workaround correct?
- Strange artifacts (clicks and very short beeps)  in data/synth [after resynthetizing](#synth): does it mean, that f0 range must be changed, or is it only the weakness of resynthetizing process itself, or maybe the result of low sample rate of CMU audio?
- x2x : warning: input data is over the range of type 'short'! Should this warning be ignored, or means that something went wrong?
- Is Python2 still needed for RHVoice tools, or will Python3 work ([issue 187](https://github.com/RHVoice/RHVoice/issues/187) is still open)?
- What is `"test"` key in training.cfg for, when the `$engin=1` step is commented out in Config.pm?

To help answer these questions, contact me directly at grzezlo (at) gmail.com or leave a comment in the [RHVoice issue 95: How to add your own voices?](https://github.com/RHVoice/RHVoice/issues/95#).

# Part 1.  Installation of common tools.
Login to your Linux system.
If not using WSL2, but connecting to external server, run the screen session in case of connection interruption.

```
screen
```

To send screen session into background press `ctrl+a` then `d` key.
To restore screen (even after reconnection) type `screen -r`

Below instructions assume, that `/home/user/` is the user's home folder.
In case of other user name, simply change `/home/user/` to e.g. `/home/john/` in all occurences.

Using tilde `~` character can be conveniently used in commands for replacing path to user home directory.
It probably will not work placed in some configuration files, so we'll sometimes need to use full path.

## Main working directory
We'll start from creating the folder, where everything will be placed.

```
mkdir ~/tts
cd ~/tts
```

## Speech dispatcher and Praat
Now it's time to install two needed packages: Speech Dispatcher, and Praat program.

```
sudo apt-get install libspeechd-dev
sudo apt-get install praat
```

## Festival and Speech tools

### Problematic 2.4 version.
Official packages in 2.4 caused after compilation on newer compilers: siod error variable EOF Unbounded.
Instead of 2.4 from below archives, version 2.5.1 must be downloaded from github.

<!--
do not download http://festvox.org/packed/festival/2.4/festival-2.4-release.tar.gz
donot download http://festvox.org/packed/festival/2.4/speech_tools-2.4-release.tar.gz
-->

### Festival 2.5.1
```
git clone --recurse https://github.com/festvox/speech_tools.git
git clone --recurse https://github.com/festvox/festival.git
```

Having downloaded newer Speech tools and Festival repos, we'll configure and compile each of them.

```
cd speech_tools
./configure
make
cd ../festival
./configure
make
cd ..
```

## Speech Signal Processing Toolkit (SPTK)
> [Speech Signal Processing Toolkit (SPTK)](http://sp-tk.sourceforge.net/) is a suite of speech signal processing tools for UNIX environments, e.g., LPC analysis, PARCOR analysis, LSP analysis, PARCOR synthesis filter, LSP synthesis filter, vector quantization techniques, and other extended versions of them.

CSH package is required by SPTK 3.11, so we'll install it:

```
sudo apt-get install csh
```

Now, we'll download and uncompress the SPTK source files.

```
curl -O http://kumisystems.dl.sourceforge.net/project/sp-tk/SPTK/SPTK-3.11/SPTK-3.11.tar.gz
tar -xvf SPTK-3.11.tar.gz
cd SPTK-3.11
```

As earlier, we'll configure the package building:

```
./configure --prefix=$(pwd)/build
```

The prefix parameter causes preparation of installing procedure (which we'll call later) to place final files in given directory: `$(pwd)` means present working directory, so install location will be in subfolder `build` of current directory.

### Patching the SPTK
Next step would be to call make.

Unfortunately before, we must correct some errors which would arise during compilation on newer GCC (Gnu C Compiler).

Some much older GCC would compile our SPTK without errors, but installation of so old compiler could be problematic itself.

#### Patching psgr.h.
First patch is to add extern before struct in line 46 of file psgr.h.

We could open the file, press `ctrl+shift+-` and manually enter line number 46, or use interesting feature of nano editor allowing to open the file in desired line:

```
nano +46 bin/psgr/psgr.h
```

The line after modification should look like below one:

```
extern struct bbmargin {               /*  Bounding Box Margin  */
```

Save file and exit nano by pressing ctrl+x, then y, and confirming old file name by pressing Enter.

#### Patching psgr.c.
```
nano +80 bin/psgr/psgr.c
```

We must add following line in line 80:

```
struct bbmargin bbm;
```

Save and exit nano.

### Compiling SPTK.
Now we're ready to compile SPTK:

```
make
make install
cd ..
```

The step `make install` is needed to have all files in `build` subfolder, instead of each tool in separate folder.
This step was not needed earlier, when we compiled the Festival.

Last `cd ..` places us in our base `tts` directory, so we're ready to go further.

## HTS installation
HMM/DNN-based Speech Synthesis System (HTS) is the library used by RHVoice to generate speech.

Unfortunately, it's not a library itself, but a patch for HTK library, which must be extended by other component, called HD Code.

To make things even more interesting, getting the HTK and HD Code components, requires registration on the authors' website.

Seems a bit complicated, so let's go step by step.

### Getting the HTS
```
curl -O http://hts.sp.nitech.ac.jp/archives/2.3/HTS-2.3_for_HTK-3.4.1.tar.bz2
mkdir HTS-2.3_for_HTK-3.4.1
tar -xvf HTS-2.3_for_HTK-3.4.1.tar.bz2 -C HTS-2.3_for_HTK-3.4.1
```

To have HTS uncompressed to separate folder, we had to create one; previously, each package was placed in separate folder in archive, so we could safely uncompress it in current directory.

### HTK registration
- First, go to [HTK registration page](https://htk.eng.cam.ac.uk/register.shtml).
- Fill in the registration form.
- Wait for the automatically generated email message containing password for entered user id.

### Downloading HTK and HD Code
Having the password, you can download below files directly, using curl, after replacing the userid and password by apropriate information in below commands:

```
curl -O https://userid:password@htk.eng.cam.ac.uk/ftp/software/HTK-3.4.1.tar.gz
curl -O https://userid:password@htk.eng.cam.ac.uk/ftp/software/hdecode/HDecode-3.4.1.tar.gz
```

### Uncompressing
Here things get simple:
```
tar -xvf HTK-3.4.1.tar.gz
tar -xvf HDecode-3.4.1.tar.gz
```

### applying hts patch
Seems difficult, but it's a matter of few simple commands:

```
cd htk
patch -p1 -d . < ../HTS-2.3_for_HTK-3.4.1/HTS-2.3_for_HTK-3.4.1.patch
```


### Compiling HTS and HD Code
```
./configure
make all
make hdecode
cd ..
```

## Downloading and compiling HTS engine
> [The HTS engine](http://hts-engine.sourceforge.net) is software to synthesize speech waveform from HMMs trained by the HMM-based speech synthesis system (HTS).

```
curl -O https://kumisystems.dl.sourceforge.net/project/hts-engine/hts_engine%20API/hts_engine_API-1.10/hts_engine_API-1.10.tar.gz
tar -xvf hts_engine_API-1.10.tar.gz
cd hts_engine_API-1.10
./configure
make
cd ..
```
After succesful compilation, we're ready to go to part 2 of this tutorial.

# Part 2. Running original HTS demo CMU ARCTIC SLT.
The goal of this part is to ensure, that all downloaded components work correctly, so we'll be able to start in RHVoice way in next part.
CMU arctic SLT is one of sample voice data used by Festival speech synthesis system.

HTS demo CMU ARCTIC SLT is a package generated from Festival's one, but prepared for easy usage in HTS.
It demonstrates the process of voice generation for HTS.

RHVoice uses some steps from this demo, some steps are modified, but for sure understanding original HTS workflow will help us to understand RHVoice's way.

## Downloading
Ensure that we're still in our base working directory (/home/user/tts), download and uncompress the file :

```
cd ~/tts
curl -O http://hts.sp.nitech.ac.jp/archives/2.3/HTS-demo_CMU-ARCTIC-SLT.tar.bz2
tar -xvf HTS-demo_CMU-ARCTIC-SLT.tar.bz2
cd HTS-demo_CMU-ARCTIC-SLT
```

## Patching and configuring
Originally, most time-consuming training stage is run in background.
We're using WSL or screen on real server, so background is not needed, but makes things a bit more complicated.

Second problem which we'll solve is to ensure, that PERL interpreter correctly finds the configuration file.

For this to happen, we must patch line 52 of Makefile.in, adding `./` before scripts (in both occurences), and deleting last `&` character.

So:
```
nano +52 Makefile.in
```

and the line after modifications should look like below:

```
	@PERL@ ./scripts/Training.pl ./scripts/Config.pm > log 2>&1
```

After saving the modified file, we can run configuration script, giving as parameters paths to all needed tools.

Probably the tilde character used in this command-line could be problematic, so we'll write full paths:

Remember to preserve backslashes at the end of lines, they're instructing shell, that started command will be continued in next line.

```
./configure --with-fest-search-path=/home/user/tts/festival/examples \
  --with-sptk-search-path=/home/user/tts/SPTK-3.11/build/bin \
  --with-hts-engine-search-path=/home/user/tts/hts_engine_API-1.10/bin \
  --with-hts-search-path=/home/user/tts/htk/HTKTools
```

If everything went well, and configure found all needed tools, the Makefile was generated.

## Running
After we run Makefile processing, the data will be analyzed, statistical voice synthesis model will be built, and finally some sample sentences will be synthesized in wav files.
It may take few hours, especially the training.pl script, which call we seen before in Makefile.in.

Starting the operation is as simple as executing:
```
make
```

Now, having pretty much time, we can try to understand a bit what is already going on...

## Waiting and understanding
`make` command processes the Makefile and runs so-called targets, described there.
We can think about targets as named procedures. For source code compilation, for which make is often used, targets are also a single or multiple source or object files.


The configure command is a Bash script (analogous to batch scripts in Windows).
One of processes there is to take `Makefile.in` as a template, and generate real Makefile with given parameters.
For example @perl@ in line 52 which we modified, will be replaced by path to PERL tool.

The configure script is also generated from configure.ac template, by other tool autoconf.

### Data types

The [Preparing data to train an HTS voice](http://www.cs.columbia.edu/~ecooper/tts/data.html) excellent document, contains short explanation of the process and used file formats.

To summarize used data types:

- Text - textual phrase.
- utterance - linguistic representation of textual phrase (Festival .utt format).
- lab: hts.lab format, flattened .utt, showing phonemes in context.
- raw: audio representation of textual phrase (.raw format). The raw audio is data-only format, without any header, so we must know the files parameters (sample rate, bits per sample, mono or stereo, little or big endian) to process or play such file. For some, perhaps historical reasons, Festival and other speech tools works on such files, not on wav.
- lf0: log fundamental frequency. Derived from raw audio analysis.
- bap: band a-periodicities. Derived from raw audio analysis.
- mgc: Mel-Generalised Cepstrum. Derived from raw audio analysis.
- questions: language-specific files used in decision trees of training.

Some more thorough explanations of some specific file formats can be found in [Getting started with the Merlin Speech Synthesis Toolkit](http://jrmeyer.github.io/tts/2017/02/14/Installing-Merlin.html)

### Folder structure
In original HTS-demo_CMU-ARCTIC-SLT we have two folders: data and scripts.

The `scripts` folder contains the Training.pl script, and its' configuration in Config.pm file.

The `data` folder contains few set of files.

- data/raw - audio in raw format.
Each file has name consisting of some constant prefix and number,e.g. `cmu_us_arctic_slt_a0001.raw`.
- data/scripts: scripts related to this set of data in languages such interesting as PERL, SIOD (Festival system, files .scm), AWK.
- data/txt: text of each audio file, with file basename same as files in data/raw, but with txt extension.
- data/utts: utterances in utt format. Most probably generated by the text2utt tool from Festival based on data/txt.
- data/labels/gen:  labels for test synthesis generation. Named alice01 to alice40. Generated at the end testing wav files will have the same base names.
- data/win: window files for parameter generation: bap, lf0, mgc .win1, .win2 and .win3.

### Main Makefile
The main Makefile contains 3 interesting targets: all, data and voice.
- all: target used when no other is passed as a parameter to make command. After colon, targets belonging to all are listed: data, voice.

Alternatively in Makefile syntax, indented lines below the target name, are the commands which should be executed as given target procedure.

### make clean
Calling `make clean` would execute the cleaning procedure, often used in compilation processes to clear all intermediate and result files, to later be able to generate everything from scratch.

### make data
`make data` enters the data folder, and executes `make` on Makefile present there.

Investigating the Makefile, we have:
```
all: analysis labels
analysis: features cmp
labels: lab mlf list scp
```

So, all targets in order are: features, cmp, lab, mlf, list, scp.

#### Data make features
For each .raw file:

- Saves base frequency to lf0/basename.lf0;
- Saves Mel Generalised Cepstrum to mgc/basename.mgc;
- Saves band a-periodicities to bap/basename.bap.

#### Data make cmp: composing training data.
For each .raw file: composes data from bap, lf0 and mgc to cmp/basename.cmp using scripts/window.pl script.

#### Data make lab: Extracting monophone and fullcontext labels
For each .utt file: generates labels/mono/basename.lab and labels/full/basename.lab.

labels/full are phonemes in context, and labels/mono are each single phonemes without context.

#### Data make mlf: Generating monophone and fullcontext Master Label Files (MLF)
Creates labels/mono.mlf and labels/full.mlf files.

#### Data make list: Generating a fullcontext model list file
Creates list of unique labels to lists/full.list and "list file which includes unseen models" to lists/full_all.list.

Also monophone list file in lists/mono.list

For each labels/full/basename.lab and labels/mono/basename.lab: requires cmp/basename.cmp, but doesn't use it.

#### Data make scp: Generating a training data script
Lists all cmp/basename.cmp to file train.scp.

Lists all label/gen/basename.lab to file gen.scp.

### make voice
`make voice` would simply run the `./scripts/Training.pl` perl script.

#### Config.pm
This file contains configuration of training process.

Generated by configure command from Config.in template file.

- Settings section contains some variables related to used synthesis (e.g. `$spkr        = 'slt';`), many related to mathematical model statistics and internal operation arrays.
- Two especially interesting are: `qnum = '001'; ver = '1';` All files generated by Training, will be stored in somefolder/q{qnum}/ver{ver}.
- Section "Directories & Commands" (starting in line 210) shows how many external utils will be used in the training process. Every path is properly setup by configure command, so nothing to change here.
- Finally, section "Switch" allows to switch off some stages of training process, if they were done earlier. It shows main stages of the Training.pl script.

#### Training.pl
- Works on data/scp/train.scp files list.
- Generates voice in HTS format.
- Generates sample synthesis based on  data/scp/gen.scp files list.

All files are stored in somefolder/q001/ver1/, where somefolder are:

- gen: generated samples. `gen/qst001/ver1/hts_engine` folder will contain alice01 to alice40 in few formats, final audio in .raw and .wav. Listening to it, we can say, that it sounds very similar to SLT voice from RHVoice, which is rather expected observation.
- voices: final voices. `voices/qst001/ver1` contains `cmu_us_arctic_slt.htsvoice`, and many other files.
- Other folders, e.g. configs, edfiles, proto, are most probably intermediate files which can be used for continuing interrupted training, or running its' different stages after some advanced parameters tuning.

Comparing the `voices/q001/ver1` folder to some RHVoice voice folder, e.g. `AppData\Roaming\nvda\addons\RHVoice-voice-English-Slt\data\24000` we see, that files lists are similar:
- Both contains: dur.pdf, lf0 (.pdf, .win1, .win2, .win3), mgc (.pdf, .win1, .win2, .win3), tree-dur.inf, tree-lf0.inf, tree-mgc.inf.
- Only RHVoice contains: bap (.pdf, .win1, .win2, .win3), bpf.txt, tree-bap.inf, and voice.data.

# Part 3. Creating the real voice for RHVoice.
The goal of this part is to create fully qualified voice for RHVoice.

## Installation
We'll need few more things installed.

```
sudo apt-get install scons
sudo apt-get install sox
```

### Python 2 and pip2
This step probably already or at some near future point in time will be not needed, if everything will be Python3 compatible.
```
sudo apt-get install python2
curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py
sudo python2 get-pip.py
pip2 install numpy
pip2 install scipy
```

### Cloning the repo
We'll clone the RHVoice repository with recurse downloads of needed submodules placed in other repositories.

```
cd ~/tts
git clone --recurse https://github.com/RHVoice/RHVoice.git
```

### Building RHVoice
To compile everything, RHVoice uses scons, we installed before. The scons tool has similar purpose that make, but is a few decades newer, and based on Python language.

```
cd RHVoice
scons dev=True
cd ..
```

We called the scons with `dev=True` to compile tools needed  for voice development.

### REAPER
The tool name may be a bit confusing, as the Reaper is a very accessible and advanced sound editor (www.reaper.fm)...
Here we'll need other one: "REAPER: Robust Epoch And Pitch EstimatoR".

```
cd ~/tts
git clone https://github.com/google/REAPER.git
cd REAPER
mkdir build
cd build
cmake ..
make
```

Instead of the configure step executed before make, we did the `cmake` command. It's an other one doing work similar to configure, but in an other way, and possibly better for its' authors and users.

### Audio dataset
To make results easier to compare, we'll use the English CMU SLT dataset, in originaly published version, not the "demo" prepared by HTS team.

```
cd ~/tts
curl -O http://festvox.org/cmu_arctic/cmu_arctic/packed/cmu_us_slt_arctic-0.95-release.tar.bz2
tar -xvf cmu_us_slt_arctic-0.95-release.tar.bz2
```

Archive contains many files and folders, but for this tutorial goal, only interesting are audio and textual transcription:

- wav/: audio recordings in wav format. Each file contains short phrase.
- etc/arctic.data, etc/txt.done.data: transcript of recording as text, both files are identical.

After successful processing, we'd like to get the RHVoice of the same quality as SLT already present in the synthesizer.

## Processing
Having RHVoice and audio dataset, we're now ready to start creating the voice.

### Creating working directory
We'll now create a folder for our RHVoice related experiments. Let's call it rhwork.

```
cd ~/tts
mkdir rhwork
cd rhwork
```

### Initializing directory
To initialize the working directory, we'll call the voice-building-utils script from RHVoice repo.
Other RHVoice specific tasks are also programmed in this script.

```
../RHVoice/src/scripts/general/voice-building-utils  init
```

The data and scripts folders were created.

it's the basic structure of HTS-demo_CMU-ARCTIC-SLT, without speaker-related data.

Here we have also training.cfg file, used by the voice-building-utils, and scripts/convert-audio new script.

### Common bin directory
As we know from part 2, the next step would be to call the configure script, giving various paths as parameters.

in RHVoice this step is defined in the `configure` command of voice-building-utils.
This command expects the parameter `"bindir"` to contain path to common directory with binaries from various programs we have currently under separate folders.

This "bindir" is set as value of multiple parameters of configure script:

```
		for name in ["--with-sptk-search-path","--with-hts-search-path","--with-hts-engine-search-path"]:
			params[name]=self.settings["bindir"]
```

The same "bindir" is used in other steps of the script, so instead patching the voice-building-utils, we'll create the common bin directory as is expected by the tool.

To do it, we'll create a directory, and then copy as symbolic links needed files from separate folders.

```
mkdir bin
cd bin
cp --symbolic-link ../../SPTK-3.11/build/bin/* ./
cp --symbolic-link ../../hts_engine_API-1.10/bin/* ./
cp --symbolic-link --no-clobber ../../htk/HTKTools/* ./
cd ..
```

### Editing the training.cfg
The training.cfg is the main configuration file of voice building in RHVoice.

Initially, some parameters are set, but many of them requires setup (has a null value).

We'll return to setting some parameters in this file few times in further reading, but ofcourse, instead of editing the file many times, it can be done in one step.

At the moment, we'll set the festdir, bindir, htk_bindir and hts22_bindir.

Festdir, a path to Festival is separate path, the rest in fact can be set to our common bin directory for simplicity.

Make sure, that after pasting new key: value, you delete old key: null from the file.

```
"festdir": "/home/user/tts/festival/",
"bindir": "/home/user/tts/rhwork/bin",
"htk_bindir": "/home/user/tts/rhwork/bin",
"hts22_bindir": "/home/user/tts/rhwork/bin",
```

### Patching Makefile.in
It's the same step, we know from part 2, when we patched the Makefile.in of "HTS-demo_CMU-ARCTIC-SLT" before running configure:

So:
```
nano +52 Makefile.in
```

and the line after modifications should look like below:

```
	@PERL@ ./scripts/Training.pl ./scripts/Config.pm >> log 2>&1
```

### Configure
This step is managed by voice-building-utils, which automatically sets needed parameters calling ./configure.
After calling it, the confirmation will be displayed, and if the given paths are correct, simple confirm by pressing "y" and Enter.

```
../RHVoice/src/scripts/general/voice-building-utils  configure
```

### Prepare wav folder
Normally we'd create the wav folder, and copy all our audio files to it.
Having this folder ready, extracted from cmu_us_slt_arctic, we can simply symlink to it:

```
mkdir wav
cd wav
cp -s ../../cmu_us_slt_arctic/wav/* ./
cd ..
```

### Import recordings
Next step is to convert our wave files to raw format, used by HTS training process.

Before doing it, we must edit our training.cfg and set:

```
"wavedir": "/home/user/tts/rhwork/wav",
"speaker": "myvoice", 
"language": "English", 
"gender": "female", 
```

"myvoice" is the test name, in real scenario we would set an other.

Now we can call:

```
../RHVoice/src/scripts/general/voice-building-utils import-recordings
```

`data/wav` and `data/raw` directories were created, each containing all audio files named RHVoice_myvoice_ (0001 to 1132) (.raw or .wav).

### Guess f0 range
F0 parameter is the base frequency of speaker's voice.
It's changing in time due to intonation.

One of "features" step in HTS-demo was extraction of lf0 (which is derived from F0 itself).
To have success in this process, the possible range of F0 must be defined.

In HTS-demo, it was set in configure.ac file as a parameters which user can change, but having default values:

```
AC_ARG_VAR([LOWERF0],[lower limit for F0 extraction in Hz (default=110)])
AC_ARG_VAR([UPPERF0],[upper limit for F0 extraction in Hz (default=280)])
```

In the file `cmu_us_slt_arctic/f0range` we find values from 110 to 250 Hz.

Those values, as calculated by scientists, are probably correct for this dataset, but what about our own recordings?
We have no clue about minimum and maximum f0 of the voice.

It's where the REAPER tool is used.
in RHVoice this step is automated: min and max F0 is calculated for each file, and then values are stored in training.cfg.

```
../RHVoice/src/scripts/general/voice-building-utils guess-f0-range
```

Summary of the process is:

```
Saved in training.cfg: 109 - 273 HZ.
```

### Extracting LF0 {#f0-extract}
This step is one of steps replacing the original features target in data Makefile we discussed in HTS-demo in part 2.
```
../RHVoice/src/scripts/general/voice-building-utils extract-f0
```

#### Assertion error {#assertion_error}

After few hundrets of files, we get unexpected output:
```
Processing RHVoice_myvoice_0626
Residual symmetry: P:3322.265869  N:2191.555420  MEAN:0.140663
Inverting signal
Traceback (most recent call last):
  File "../RHVoice/src/scripts/general/voice-building-utils", line 1542, in <module>
    args.func(args)
  File "../RHVoice/src/scripts/general/voice-building-utils", line 750, in __call__
    assert(len(nzvalues)>0)
AssertionError
```

It seems, that only solution is to change a bit earlier calculated F0 range, so edit the train.cfg and round to nearest tenth:

```
"lower_f0": 110, 
"upper_f0": 280, 
```

Then we'll repeat the last voice-building-utils step.

#### Two news: good and bad.
Good news is, that the F0 extraction process will continue from problematic file, not from beginning.
Bad news is, that assertion error persists.

Finally the process continues after setting:

```
"lower_f0": 100, 
"upper_f0": 280, 
```

Process finishes without error, and we have data/lf0 folder created.

### Bap extraction
```
../RHVoice/src/scripts/general/voice-building-utils extract-bap
```

After processing, data/bap folder will be created.

### Rest of analysis
For completeness, we lack the `mgc` generation, which was a part of target features in data/Makefile of original HTS-demo.

Comparing original HTS-demo and RHVoice template, we can observe changes in data/Makefile.in:

- The features target in RHVoice has commented step of generation of lf0/basename.lf0, which is obvious as it's generated inside voice-building-utils.
- The mgc generation is performed in RHVoice version using scripts/convert-audio, not by x2x tool.

So, the mgc generation will be processed by make, without risk of overwriting anything done previously.

```
cd data
make analysis
cd ..
```

Analysis consist of features target (in our case mgc generation), and cmp target (combined training features in cmp folder).

### Resynthetizing {#synth}
```
../RHVoice/src/scripts/general/voice-building-utils synth
```

Resynthesizes the audio phrases from .raw files processing it using .lf0 and .mgc files generated before.

Result is placed in data/synth/*.wav.

Sounds similar to the result voice synthesizer which will be created.

If the result is inacceptable, following steps must be repeated:

- Edit training.cfg and change F0 range.
- Remove data/lf0 folder, to force whole set recalculation.
- Running F0 extraction.
- Repeat resynthesize step and inspect the results.

So, after changing the F0 range in training.cfg, run following commands:

```
rm -r data/lf0
../RHVoice/src/scripts/general/voice-building-utils extract-f0
../RHVoice/src/scripts/general/voice-building-utils synth
```

After processing the audio, it's time to work on our transcript, and label files generation.

### Converting transcript to ssml
In RHVoice the transcript should be stored in ssml file.

Each single phrase of this file should be synchronized with ordered alphabetically audio files, so first phrase should transcribe wav/arctic_a0001.wav, after conversion being RHVoice_myvoice_0001.wav, and last one the file arctic_b0539.wav after conversion being RHVoice_myvoice_1132.wav.

In cmu_us_slt_arctic/etc/txt.done.data we have each phrase in single line, formatted as follows:

```
( arctic_a0001 "Author of the danger trail, Philip Steels, etc." )
( arctic_a0002 "Not at this particular case, Tom, apologized Whittemore." )
...
( arctic_b0538 "At the best, they were necessary accessories." )
( arctic_b0539 "You were making them talk shop, Ruth charged him." )
```

We'd like to convert it into ssml format, as follows:

```
<speak xml:lang="en">
<s> Author of the danger trail, Philip Steels, etc. </s>
<s> Not at this particular case, Tom, apologized Whittemore. </s>
...
<s> At the best, they were necessary accessories. </s>
<s> You were making them talk shop, Ruth charged him. </s>
</speak>
```

The task is simple using any programming language, but even simpler from the command line using sed and regular expressions.

```
printf "<speak xml:lang=\"en\">\n" >prompts.ssml &&
sed -re 's/\([^"]*\"/<s> /g;s/\" \)/ <\/s>/g' \
<../cmu_us_slt_arctic/etc/txt.done.data \
>> prompts.ssml &&
printf "</speak>\n" >>prompts.ssml
```

We have now prompts.ssml in our current directory.

### Text in training.cfg
Now in training.cfg we must put path to our prompts file:

```
"text": "/home/user/tts/rhwork/prompts.ssml",
```

### Segmenting
We start this step by executing:

```
../RHVoice/src/scripts/general/voice-building-utils segment
```

The data/labels/mono folder with content was created.

Also the htk folder with many files was created in our rhwork working directory.

### Labelling
```
../RHVoice/src/scripts/general/voice-building-utils label
```

`data/labels/gen` and `data/labels/full` were created. 

Also in data folder make was called with following targets:

- make mlf: Creates labels/mono.mlf and labels/full.mlf files.
- make list: Creates list of unique labels to lists/full.list and "list file which includes unseen models" to lists/full_all.list. Also monophone list file in lists/mono.list.
- make scp: Lists all cmp/basename.cmp to file train.scp. Lists all label/gen/basename.lab to file gen.scp.

So functionally this step replaces Makefile label target from data folder.
Because we finished previously the Makefile analysis step, we're done with Makefile data target at this point.

### Making questions
```
../RHVoice/src/scripts/general/voice-building-utils make-questions
```

data/questions will be created.

###  Creating LPF
```
../RHVoice/src/scripts/general/voice-building-utils make-lpf
```

voices/qst001/ver1/lpf.pdf and others were created.

### Training
We run training process by executing:
```
make voice
```

After some hours, models, trees and other training-related folders will be created.

Look into the huge log file, to inspect are there some errors in the process.

One of typical could be information about too small count of observations, meaning, that our audio dataset not contains enough audio to build statistical model.

The `gen` folder is empty, no generated waves.
It's not due to some error, but the fact, that many tasks are commented out in file scripts/Config.pm in RHVoice version.

### Exporting voice
```
../RHVoice/src/scripts/general/voice-building-utils export-voice
```

Voice files are exported to path set as outdir in training.cfg, set up automatically as data/voices in `~/tts/RHVoice` folder.

### Testing the voice
To test the voice, we'll use RHVoice-test program, saving output in wav file:
```
echo "This is just a simple test of this speech synthesis." |
../RHVoice/local/bin/RHVoice-test -p Myvoice -o ./test.wav
```

Unfortunately, at the moment we get:
Error: HTS106_Model_load_pdf: Failed to load header of pdfs.

When we call the rhvoice-test with Slt profile (`-p Slt`) everything works well and output is created.

To be continued.
