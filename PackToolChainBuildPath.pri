win32-msvc*{

CONFIG(debug,debug|release){
PackToolChainBuildPath=$$PWD/binlib/bind/msvc
}else{
PackToolChainBuildPath=$$PWD/binlib/bin/msvc
}

}else{

CONFIG(debug,debug|release){
PackToolChainBuildPath=$$PWD/binlib/bind/common
}else{
PackToolChainBuildPath=$$PWD/binlib/bin/common
}

}

