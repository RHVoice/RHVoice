# Compiling universal xcframework

## System requirements

* iOS 13.0 or newer
* macOS 11.0 or newer

## Required tools

To compile RHVoice the following programs must be installed on your system:

* [Xcode](https://developer.apple.com/xcode/)
* [Pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)
* [SCons](https://www.scons.org)


## Get sources

1. One line
    ```bash
    git clone --recursive https://github.com/RHVoice/RHVoice.git
    ```
2. Step by step
    ```bash
    git clone https://github.com/RHVoice/RHVoice.git
    cd RHVoice
    git submodule update --init
    ```

## Compilation

Open next folder in repository:

```bash
cd src/apple/RHVoice
```

And then run create_xcframework.sh by executing command:

```bash
./RHVoice/BuildScripts/Scripts/create_xcframework.sh
```

You can specify languages and voices that you want to be included into framework by adding next parameters

```bash
./RHVoice/BuildScripts/Scripts/create_xcframework.sh english Alan,Lyubov
```

Also to save some space on disk you can place data into separate folder passing additional parameter

```bash
./RHVoice/BuildScripts/Scripts/create_xcframework.sh english Alan,Lyubov true
```
However you would need to include it to bundle of your app separately and specify that path in code:
```swift
let initParams = RHVoiceBridgeParams.default()
if let dataPath = Bundle.main.path(forResource: "RHVoiceData", ofType: nil) {
    initParams.dataPath = dataPath
}
RHVoiceBridge.sharedInstance().params = initParams
```

## Usage

### Add generated `RHVoice.xcframework` to your project

Path to the framework will be printed in Terminal:
```bash
xcframework successfully written out to: {path_to_repository}/src/ios/RHVoice/RHVoice.xcframework
```
Just copy to your project or zip and upload to your third party storage.

Add `RHVoiceData` to you project as folder reference if it is decided to keep it outside of `RHVoice.xcframework`

```swift
import RHVoice
```

### `RHSpeechSynthesizer`

Create `RHSpeechSynthesizer` instance
```swift
let synthesizer = RHSpeechSynthesizer()
```
It is needed to retain `synthesizer` until "speaking" is finished

### Select needed voice

```swift
let voice = RHSpeechSynthesisVoice.speechVoices().first { voice in
    return voice.language == "US" && voice.gender == RHSpeechSynthesisVoiceGenderMale
}
```
### `RHSpeechUtterance` 
Create `RHSpeechUtterance` with required text to speak assign voice and let it speek using `synthesizer`
```swift
let utterance = RHSpeechUtterance(text: "Sample Text")
utterance.voice = voice
synthesizer.speak(utterance)
```
### Samples

Please refer to samples for more details
 1. [iOS](/src/apple/ios/Sample/iOSSample)
 2. [macOS](/src/apple/macOS/Sample/macOSSample)
