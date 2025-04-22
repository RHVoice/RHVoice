
if ! command -v scons &> /dev/null
then
   echo "scons can not be found in your enviroment"
   echo "please run 'sudo ln -s /opt/homebrew/bin/scons /usr/local/bin/'"
   echo "or install scons"
   exit 1
fi

if [ "$BUILD_VIA_SCRIPT" = true ] ; then
    exit 0
fi

pushd ${PROJECT_DIR}/../../../
./src/ios/build_apple_core_lib.sh $LANGUAGES $VOICES
popd
