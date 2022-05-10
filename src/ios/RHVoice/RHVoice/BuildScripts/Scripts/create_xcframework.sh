
if ! command -v scons &> /dev/null
then
   echo "scons can not be found in your enviroment"
   echo "please run 'sudo ln -s /opt/homebrew/bin/scons /usr/local/bin/'"
   echo "or install scons"
   exit 1
fi

TARGET_NAME="RHVoice"
PROJECT_NAME=$TARGET_NAME
CONFIGURATION="Release"

IPHONE_SDK_NAME="iphoneos"
IPHONESIMULATOR_SDK_NAME="iphonesimulator"

IPHONEOS_OUTPUT="build_$IPHONE_SDK_NAME"
IPHONESIMULATOR_OUTPUT="build_$IPHONESIMULATOR_SDK_NAME"

SDK_NAMES=($IPHONE_SDK_NAME $IPHONESIMULATOR_SDK_NAME)

rm -rfv $IPHONEOS_OUTPUT/*
rm -rfv $IPHONESIMULATOR_OUTPUT/*
rm -rfv $TARGET_NAME.xcframework/*

pushd ../../../
./src/ios/build_ios_core_lib.sh "$1" "$2" true
popd


for SDK_NAME in "${SDK_NAMES[@]}"
do

xcodebuild clean build -configuration $CONFIGURATION -project "$TARGET_NAME.xcodeproj" -target $TARGET_NAME -sdk $SDK_NAME ONLY_ACTIVE_ARCH="NO" CONFIGURATION_BUILD_DIR="build_$SDK_NAME" LANGUAGES=$1 VOICES=$2

done

xcodebuild -create-xcframework -framework "$IPHONEOS_OUTPUT/$TARGET_NAME.framework" \
                               -framework "$IPHONESIMULATOR_OUTPUT/$TARGET_NAME.framework" \
                               -output "$TARGET_NAME.xcframework"