if [[ $3 ]]; then
    echo "Clearing build folder."
    rm -rfv build/*
fi

IFS=',' read -r -a VOICES <<< "$2"

LANGUAGES="$1"
echo "Building for LANGUAGES=$LANGUAGES and VOICES=$2"

scons enable_shared=no languages=$LANGUAGES audio_libs=none platform=ios simulator=False arch=arm64
scons enable_shared=no languages=$LANGUAGES audio_libs=none platform=ios simulator=True arch=arm64
scons enable_shared=no languages=$LANGUAGES audio_libs=none platform=ios simulator=True arch=x86_64

echo "Building complete."
BUILD_FOLDER=build
PRODUCT_NAME=RHVoice
ARTICFACT_NAME="${PRODUCT_NAME}Core"

IOS_OUTPUT_FOLDER=$BUILD_FOLDER/ios_output

rm -rfv $IOS_OUTPUT_FOLDER
mkdir $IOS_OUTPUT_FOLDER

echo "Copying headers..."
HEADERS_FOLDER=$IOS_OUTPUT_FOLDER/headers
mkdir $HEADERS_FOLDER
function syncHeadersToFrom()
{
   TO=$1
   FROM=$2
   rsync -auzv --include '*/' --include '*.h' --include '*.hpp' --include '*.inc'   --exclude '*' $FROM/ $TO/
}

syncHeadersToFrom $HEADERS_FOLDER ./src/include/
syncHeadersToFrom $HEADERS_FOLDER ./src/third-party/rapidxml/
syncHeadersToFrom $HEADERS_FOLDER ./src/third-party/utf8/

echo "Copping headers complete."

echo "Creating xcframework with name $ARTICFACT_NAME."
lipo $BUILD_FOLDER/ios_arm64_simulator/core/libRHVoice_core.a $BUILD_FOLDER/ios_X86_64_simulator/core/libRHVoice_core.a -create -output $IOS_OUTPUT_FOLDER/libRHVoice_core.a
lipo $BUILD_FOLDER/ios_arm64_simulator/audio/libRHVoice_audio.a $BUILD_FOLDER/ios_X86_64_simulator/audio/libRHVoice_audio.a -create -output $IOS_OUTPUT_FOLDER/libRHVoice_audio.a
libtool -static -o $IOS_OUTPUT_FOLDER/$ARTICFACT_NAME.a $IOS_OUTPUT_FOLDER/libRHVoice_core.a $IOS_OUTPUT_FOLDER/libRHVoice_audio.a
libtool -static -o $BUILD_FOLDER/$ARTICFACT_NAME.a $BUILD_FOLDER/ios_arm64/core/libRHVoice_core.a $BUILD_FOLDER/ios_arm64/audio/libRHVoice_audio.a
xcodebuild -create-xcframework -library $IOS_OUTPUT_FOLDER/$ARTICFACT_NAME.a -headers $HEADERS_FOLDER -library $BUILD_FOLDER/$ARTICFACT_NAME.a -headers $HEADERS_FOLDER -output $IOS_OUTPUT_FOLDER/$ARTICFACT_NAME.xcframework
echo "Creating xcframework complete."

rm $IOS_OUTPUT_FOLDER/libRHVoice_core.a $IOS_OUTPUT_FOLDER/libRHVoice_audio.a $IOS_OUTPUT_FOLDER/$ARTICFACT_NAME.a $BUILD_FOLDER/$ARTICFACT_NAME.a
rm -rf $HEADERS_FOLDER

echo "Copying data..."
IOS_DATA_FOLDER=$IOS_OUTPUT_FOLDER/RHVoiceData
IOS_LANGUAGES_FOLDER=$IOS_DATA_FOLDER/languages
IOS_VOICES_FOLDER=$IOS_DATA_FOLDER/voices
mkdir $IOS_DATA_FOLDER
mkdir $IOS_LANGUAGES_FOLDER
mkdir $IOS_VOICES_FOLDER

PACKAGES_FOLDER=$BUILD_FOLDER/ios_arm64/packages/zip
for filename in $PACKAGES_FOLDER/*.zip; do
    if  [[ $filename == *"/$PRODUCT_NAME-language"*  ]] ;
    then
        unzip $filename -d $IOS_LANGUAGES_FOLDER/$(basename  -s .zip $filename)
    elif [[ $filename == *"/$PRODUCT_NAME-voice"*  ]] ;
    then
        if [ ${#VOICES[@]} -eq 0 ]; then
            unzip $filename -d $IOS_VOICES_FOLDER/$(basename  -s .zip $filename)
        else
            for voice in "${VOICES[@]}"
            do
                echo "$voice"
                if  [[ $filename == *"$voice"*  ]] ;
                then
                    unzip $filename -d $IOS_VOICES_FOLDER/$(basename  -s .zip $filename)
                    break
                fi
            done
        fi
    fi
done

echo "Copping data complete."

echo "Building is completed. Resulst can be found here:\n$IOS_OUTPUT_FOLDER"

