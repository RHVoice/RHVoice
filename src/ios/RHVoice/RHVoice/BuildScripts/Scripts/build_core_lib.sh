
if ! command -v scons &> /dev/null
then
   echo "scons can not be found in your enviroment"
   echo "please run 'sudo ln -s /opt/homebrew/bin/scons /usr/local/bin/'"
   exit 1
fi

pushd ${PROJECT_DIR}/../../../
./src/ios/build_ios_core_lib.sh $LANGUAGES
popd
