
if ! command -v scons &> /dev/null
then
   echo "scons can not be found in your enviroment"
   echo "please run 'sudo ln -s /opt/homebrew/bin/scons /usr/local/bin/'"
   echo "or install scons"
   exit 1
fi

TARGET_NAME_IOS="RHVoiceiOS"
TARGET_NAME_MACOS="RHVoiceMacOS"
PROJECT_NAME="RHVoice"
CONFIGURATION="Release"
RH_VOICE_DATA_FOLDER="RHVoiceData"

IPHONE_SDK_NAME="iphoneos"
IPHONESIMULATOR_SDK_NAME="iphonesimulator"
MACOS_SDK_NAME="macosx"

OUTPUT="build_output"
IPHONEOS_OUTPUT="build_$IPHONE_SDK_NAME.xcarchive"
IPHONESIMULATOR_OUTPUT="build_$IPHONESIMULATOR_SDK_NAME.xcarchive"
MACOS_OUTPUT="build_$MACOS_SDK_NAME.xcarchive"

SDK_NAMES=($IPHONE_SDK_NAME $IPHONESIMULATOR_SDK_NAME)

rm -vrf $IPHONEOS_OUTPUT
rm -vrf $IPHONESIMULATOR_OUTPUT
rm -vrf $MACOS_OUTPUT
rm -vrf $OUTPUT

pushd ../../../
./src/apple/build_apple_core_lib.sh "$1" "$2" true
popd

for SDK_NAME in "${SDK_NAMES[@]}"
do
           
xcodebuild archive -project "$PROJECT_NAME.xcodeproj" \
           -configuration $CONFIGURATION  \
           -scheme $TARGET_NAME_IOS \
           -sdk $SDK_NAME -destination="iOS" \
           -archivePath "build_$SDK_NAME" \
           LANGUAGES=$1 VOICES=$2 \
           ONLY_ACTIVE_ARCH="NO"\
           BUILD_LIBRARIES_FOR_DISTRIBUTION="YES" \
           BUILD_VIA_SCRIPT=true \
           SKIP_INSTALL="NO"

done

xcodebuild archive -project "$PROJECT_NAME.xcodeproj" \
           -configuration $CONFIGURATION  \
           -scheme $TARGET_NAME_MACOS \
           -sdk "macosx" -destination="macOS" \
           -archivePath "build_$MACOS_SDK_NAME" \
           LANGUAGES=$1 VOICES=$2 \
           ONLY_ACTIVE_ARCH="NO"\
           BUILD_LIBRARIES_FOR_DISTRIBUTION="YES" \
           BUILD_VIA_SCRIPT=true \
           SKIP_INSTALL="NO"
     
mkdir $OUTPUT    
if [[ $3 ]]; then
    mkdir $OUTPUT/$RH_VOICE_DATA_FOLDER/
    cp -a $IPHONEOS_OUTPUT/Products/Library/Frameworks/$PROJECT_NAME.framework/$RH_VOICE_DATA_FOLDER/. $OUTPUT/$RH_VOICE_DATA_FOLDER/
    rm -rfv $IPHONEOS_OUTPUT/Products/Library/Frameworks/$PROJECT_NAME.framework/$RH_VOICE_DATA_FOLDER/*
    rm -rfv $IPHONESIMULATOR_OUTPUT/Products/Library/Frameworks/$PROJECT_NAME.framework/$RH_VOICE_DATA_FOLDER/*
    rm -rfv $MACOS_OUTPUT/Products/Library/Frameworks/$PROJECT_NAME.framework/Versions/A/Resources/$RH_VOICE_DATA_FOLDER/*
fi

xcodebuild -create-xcframework -framework "$IPHONEOS_OUTPUT/Products/Library/Frameworks/$PROJECT_NAME.framework" \
                               -framework "$IPHONESIMULATOR_OUTPUT/Products/Library/Frameworks/$PROJECT_NAME.framework" \
                               -framework "$MACOS_OUTPUT/Products/Library/Frameworks/$PROJECT_NAME.framework" \
                               -output "$OUTPUT/$PROJECT_NAME.xcframework"

rm -rf $IPHONEOS_OUTPUT
rm -rf $IPHONESIMULATOR_OUTPUT
rm -rf $MACOS_OUTPUT
