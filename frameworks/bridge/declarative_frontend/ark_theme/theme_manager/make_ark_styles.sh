#!/bin/bash

# release build by default
isRelease=true

# out path
out=./out

# release file path
release=./out/arkTheme.js

# debug file path
debug=./out/arkTheme_debug.js

# engine output path
engine_output=../../engine/arkTheme.js

# read arguments
# -d means debug build with console.log usages
while getopts d OPT
do
    case "$OPT" in
        d) isRelease=false ;;
        ?) ;;
    esac
done

# remove last output
if [ -d "$out" ]; then
    rm -rf $out
fi

# compile solution
tsc --build tsconfig.json

# add header
sed -i '1i  /*'                                                                             $release
sed -i '2i \ * Copyright (c) 2024 Huawei Device Co., Ltd.'                                  $release
sed -i '3i \ * Licensed under the Apache License, Version 2.0 (the "License");'             $release
sed -i '4i \ * you may not use this file except in compliance with the License.'            $release
sed -i '5i \ * You may obtain a copy of the License at'                                     $release
sed -i '6i \ *'                                                                             $release
sed -i '7i \ *     http://www.apache.org/licenses/LICENSE-2.0'                              $release
sed -i '8i \ *'                                                                             $release
sed -i '9i \ * Unless required by applicable law or agreed to in writing, software'         $release
sed -i '10i\ * distributed under the License is distributed on an "AS IS" BASIS,'           $release
sed -i '11i\ * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.'    $release
sed -i '12i\ * See the License for the specific language governing permissions and'         $release
sed -i '13i\ * limitations under the License.'                                              $release
sed -i '14i\ */'                                                                            $release
sed -i '15i /* THIS IS AUTOGENERATED FILE, PLEASE DON`T CHANGE IT MANUALLY */'              $release

# copy debug version
cp $release $debug

# remove logs for release version
sed -i '/console.log/d' $release

# move library to 'engine' directory
if "$isRelease"
then
    cp $release $engine_output
else
    cp $debug $engine_output
fi

# build arkThemeControl.js module
tsc -b tsconfig-themecontrol.json
release_themecontrol=./out/theme/ArkThemeControl.js
# add header
sed -i '1i  /*'                                                                             $release_themecontrol
sed -i '2i \ * Copyright (c) 2024 Huawei Device Co., Ltd.'                                  $release_themecontrol
sed -i '3i \ * Licensed under the Apache License, Version 2.0 (the "License");'             $release_themecontrol
sed -i '4i \ * you may not use this file except in compliance with the License.'            $release_themecontrol
sed -i '5i \ * You may obtain a copy of the License at'                                     $release_themecontrol
sed -i '6i \ *'                                                                             $release_themecontrol
sed -i '7i \ *     http://www.apache.org/licenses/LICENSE-2.0'                              $release_themecontrol
sed -i '8i \ *'                                                                             $release_themecontrol
sed -i '9i \ * Unless required by applicable law or agreed to in writing, software'         $release_themecontrol
sed -i '10i\ * distributed under the License is distributed on an "AS IS" BASIS,'           $release_themecontrol
sed -i '11i\ * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.'    $release_themecontrol
sed -i '12i\ * See the License for the specific language governing permissions and'         $release_themecontrol
sed -i '13i\ * limitations under the License.'                                              $release_themecontrol
sed -i '14i\ */'                                                                            $release_themecontrol
sed -i '15i /* THIS IS AUTOGENERATED FILE, PLEASE DON`T CHANGE IT MANUALLY */'              $release_themecontrol
# move module to 'engine' folder
cp $release_themecontrol ../../engine/arkThemeControl.js