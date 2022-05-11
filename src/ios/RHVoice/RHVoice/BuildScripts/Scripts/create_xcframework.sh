
if ! command -v scons &> /dev/null
then
   echo "scons can not be found in your enviroment"
   echo "please run 'sudo ln -s /opt/homebrew/bin/scons /usr/local/bin/'"
   echo "or install scons"
   exit 1
fi

TARGET_NAME="RHVoice"
CONFIGURATION="Release"

IPHONE_SDK_NAME="iphoneos"
IPHONESIMULATOR_SDK_NAME="iphonesimulator"

IPHONEOS_OUTPUT="build_$IPHONE_SDK_NAME.xcarchive"
IPHONESIMULATOR_OUTPUT="build_$IPHONESIMULATOR_SDK_NAME.xcarchive"

SDK_NAMES=($IPHONE_SDK_NAME $IPHONESIMULATOR_SDK_NAME)

rm -vrf $IPHONEOS_OUTPUT
rm -vrf $IPHONESIMULATOR_OUTPUT
rm -vrf $TARGET_NAME.xcframework

pushd ../../../
./src/ios/build_ios_core_lib.sh "$1" "$2" true
popd

for SDK_NAME in "${SDK_NAMES[@]}"
do
           
xcodebuild archive -project "$TARGET_NAME.xcodeproj" \
           -configuration $CONFIGURATION  \
           -scheme $TARGET_NAME \
           -sdk $SDK_NAME -destination="iOS" \
           -archivePath "build_$SDK_NAME" \
           LANGUAGES=$1 VOICES=$2 \
           ONLY_ACTIVE_ARCH="NO"\
           BUILD_LIBRARIES_FOR_DISTRIBUTION="YES" \
           BUILD_VIA_SCRIPT=true \
           SKIP_INSTALL="NO"

done

xcodebuild -create-xcframework -framework "$IPHONEOS_OUTPUT/Products/Library/Frameworks/$TARGET_NAME.framework" \
                               -framework "$IPHONESIMULATOR_OUTPUT/Products/Library/Frameworks/$TARGET_NAME.framework" \
                               -output "$TARGET_NAME.xcframework"