cmake_minimum_required(VERSION 2.8.1)
cmake_policy(SET CMP0015 NEW)

FIND_PACKAGE( Boost COMPONENTS program_options chrono filesystem thread system log_setup log date_time timer regex iostreams REQUIRED )

include_directories(${CSI_INCLUDE_PATH} ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIR})

#add_definitions(-D_FILE_OFFSET_BITS=64 -D_REENTRANT -DEXTERNAL_LOCKS -DMULTITHREAD)
#add_definitions(-DBOOST_ASIO_HAS_STD_CHRONO)

if(WIN32)

    add_definitions(-DMARCH_x86_64) 
    add_definitions(-D_WIN32_WINNT=0x0602) # win8.1 to get nthll 64 bit
    add_definitions(-DWINVER=0x0601)
    add_definitions(-DCF_WINDOWS)
    add_definitions(/MP)
    add_definitions(/bigobj)
    add_definitions(-D_UNICODE)
    add_definitions(-DUNICODE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-DSECURITY_WIN32)
    add_definitions(/wd4996) #'strdup': The POSIX name for this item is deprecated
    add_definitions(/wd4197) # 'cf_atomic32' : top-level volatile in cast is ignored
    add_definitions(/wd4200) #nonstandard extension used : zero-sized array in struct/union 

    link_directories(${Boost_LIBRARY_DIRS}/$(Platform)/lib)

elseif(APPLE)

    add_definitions(-DBOOST_LOG_DYN_LINK)
    add_definitions(-std=c++11)

    link_directories(${Boost_LIBRARY_DIRS})

    if (NOT DEFINED CSI_BOOST_LIBS)
        set(CSI_BOOST_LIBS ${Boost_LIBRARIES} pthread)
    endif()

else()

    add_definitions(-D__LINUX__)
    add_definitions(-std=c++11)

    #add_definitions(-fPIC)
    #add_definitions(-DBOOST_LOG_USE_NATIVE_SYSLOG)
    #add_definitions(-Wno-deprecated)

    link_directories(${Boost_LIBRARY_DIRS})

    if (NOT DEFINED CSI_BOOST_LIBS)
        set(CSI_BOOST_LIBS ${Boost_LIBRARIES} pthread)
    endif()

endif()


add_subdirectory(csi_kafka)

# if(__BUILD_EXAMPLES__)
add_subdirectory(examples)
# endif()
