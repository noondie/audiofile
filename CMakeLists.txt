
cmake_minimum_required (VERSION 3.5..3.26)


project(libaudiofile VERSION 0.3.6)

#
# Variables
#

set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED TRUE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set (PACKAGE_NAME ${PROJECT_NAME})
set (CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set (CPACK_PACKAGE_VERSION_STAGE "")
set (CPACK_PACKAGE_VERSION_FULL "${PROJECT_VERSION}${CPACK_PACKAGE_VERSION_STAGE}")

#
# System-wide includes
#

include (GNUInstallDirs)
include (FeatureSummary)
include (CMakeDependentOption)
include (CTest)
include (CheckCCompilerFlag)



#
# Options
#

option (BUILD_SHARED_LIBS "Build shared libraries" OFF)

if (BUILD_SHARED_LIBS AND BUILD_TESTING)
        set (BUILD_TESTING OFF)
        message ("Build testing required static libraries. To prevent build errors BUILD_TESTING disabled.")
endif ()
option (BUILD_COMMANDS "Build commands" ON)
option (BUILD_EXAMPLES "Build examples" ON)
option (ENABLE_CPACK "Enable CPack support" ON)
option (ENABLE_BOW_DOCS "Enable black-on-white html docs" OFF)
if (MSVC AND (DEFINED ENABLE_STATIC_RUNTIME))
        option (ENABLE_STATIC_RUNTIME "Enable static runtime" ${ENABLE_STATIC_RUNTIME})
elseif (MINGW)
        option (ENABLE_STATIC_RUNTIME "Enable static runtime" OFF)
endif ()
option (ENABLE_PACKAGE_CONFIG "Generate and install package config file" ON)
option (INSTALL_PKGCONFIG_MODULE "Generate and install pkg-config module" ON)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
if (CMAKE_VERSION VERSION_LESS 3.14)
        list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sqlite")
endif ()


#
# Setup configuration
#

configure_file (libaudiofile/config.h.cmake libaudiofile/config.h)

if (INSTALL_PKGCONFIG_MODULE)

        set (prefix ${CMAKE_INSTALL_PREFIX})
        set (exec_prefix "\$\{prefix\}")
        set (libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
        set (includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
        set (VERSION ${PROJECT_VERSION})
        if (ENABLE_EXTERNAL_LIBS)
                set (EXTERNAL_XIPH_REQUIRE "flac ogg vorbis vorbisenc opus")
                if (ENABLE_EXPERIMENTAL)
                        set (EXTERNAL_XIPH_REQUIRE "${EXTERNAL_XIPH_REQUIRE} speex")
                endif ()
        endif ()
        if (ENABLE_MPEG)
                set (EXTERNAL_MPEG_REQUIRE "libmpg123")
                set (EXTERNAL_MPEG_LIBS "-lmp3lame")
        endif ()

#        configure_file (sndfile.pc.in sndfile.pc @ONLY)

endif ()

#
# libsndfile
#

# Public libaudiofile headers
set (audiofile_HDRS
     libaudiofile/audiofile.h
    )


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libaudiofile })
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/libaudiofile)


#target_include_directories(Tutorial PUBLIC
#                          "${PROJECT_BINARY_DIR}"
#                          "${PROJECT_SOURCE_DIR}/MathFunctions"
#                          )

#
# libaudiofile static library
#

set(libaudiofile_src
        libaudiofile/AIFF.cpp
        libaudiofile/FileHandle.h
        libaudiofile/PacketTable.h
        libaudiofile/aes.cpp
        libaudiofile/error.h
        libaudiofile/AIFF.h
        libaudiofile/IFF.cpp
        libaudiofile/Raw.cpp
        libaudiofile/af_vfs.cpp
        libaudiofile/extended.c
        libaudiofile/AVR.cpp
        libaudiofile/IFF.h
        libaudiofile/Raw.h
        libaudiofile/af_vfs.h
        libaudiofile/extended.h
        libaudiofile/AVR.h
        libaudiofile/IRCAM.cpp
        libaudiofile/SampleVision.cpp
        libaudiofile/afinternal.h
        libaudiofile/format.cpp
        libaudiofile/AudioFormat.cpp
        libaudiofile/IRCAM.h
        libaudiofile/SampleVision.h
        libaudiofile/g711.c
        libaudiofile/AudioFormat.h
        libaudiofile/Instrument.cpp
        libaudiofile/Setup.cpp
        libaudiofile/g711.h
        libaudiofile/Buffer.cpp
        libaudiofile/Instrument.h
        libaudiofile/Setup.h
        libaudiofile/audiofile.h
        libaudiofile/Buffer.h
        libaudiofile/Loop.cpp
        libaudiofile/Shared.h
        libaudiofile/aupv.c
        libaudiofile/openclose.cpp
        libaudiofile/CAF.cpp
        libaudiofile/Tag.h
        libaudiofile/aupvinternal.h
        libaudiofile/pcm.cpp
        libaudiofile/CAF.h
        libaudiofile/Marker.cpp
        libaudiofile/Track.cpp
        libaudiofile/aupvlist.h
        libaudiofile/pcm.h
        libaudiofile/Compiler.h
        libaudiofile/Marker.h
        libaudiofile/Track.h
        libaudiofile/byteorder.h
        libaudiofile/query.cpp
        libaudiofile/FLACFile.cpp
        libaudiofile/Miscellaneous.cpp
        libaudiofile/UUID.cpp
        libaudiofile/compression.cpp
        libaudiofile/units.cpp
        libaudiofile/FLACFile.h
        libaudiofile/NIST.cpp
        libaudiofile/UUID.h
        libaudiofile/compression.h
        libaudiofile/units.h
        libaudiofile/Features.h
        libaudiofile/NIST.h
        libaudiofile/VOC.cpp
        libaudiofile/data.cpp
        libaudiofile/util.cpp
        libaudiofile/File.cpp
        libaudiofile/NeXT.cpp
        libaudiofile/VOC.h
        libaudiofile/debug.cpp
        libaudiofile/util.h
        libaudiofile/File.h
        libaudiofile/NeXT.h
        libaudiofile/WAVE.cpp
        libaudiofile/debug.h
        libaudiofile/FileHandle.cpp
        libaudiofile/PacketTable.cpp
        libaudiofile/WAVE.h
        libaudiofile/error.c
        libaudiofile/modules/ALAC.cpp
        libaudiofile/modules/FileModule.cpp
        libaudiofile/modules/MSADPCM.cpp
        libaudiofile/modules/ModuleState.h
        libaudiofile/modules/SimpleModule.h
        libaudiofile/modules/ALAC.h
        libaudiofile/modules/FileModule.h
        libaudiofile/modules/MSADPCM.h
        libaudiofile/modules/PCM.cpp
        libaudiofile/modules/BlockCodec.cpp
        libaudiofile/modules/G711.cpp
        libaudiofile/modules/PCM.h
        libaudiofile/modules/BlockCodec.h
        libaudiofile/modules/G711.h
        libaudiofile/modules/Module.cpp
        libaudiofile/modules/RebufferModule.cpp
        libaudiofile/modules/FLAC.cpp
        libaudiofile/modules/IMA.cpp
        libaudiofile/modules/Module.h
        libaudiofile/modules/RebufferModule.h
        libaudiofile/modules/FLAC.h
        libaudiofile/modules/IMA.h
        libaudiofile/modules/ModuleState.cpp
        libaudiofile/modules/SimpleModule.cpp
        libaudiofile/alac/ALACAudioTypes.h
        libaudiofile/alac/ALACDecoder.h
        libaudiofile/alac/EndianPortable.h
        libaudiofile/alac/aglib.h
        libaudiofile/alac/matrix_dec.c
        libaudiofile/alac/ALACBitUtilities.c
        libaudiofile/alac/ALACEncoder.cpp
        libaudiofile/alac/dp_dec.c
        libaudiofile/alac/matrix_enc.c
        libaudiofile/alac/ALACBitUtilities.h
        libaudiofile/alac/ALACEncoder.h
        libaudiofile/alac/ag_dec.c
        libaudiofile/alac/dp_enc.c
        libaudiofile/alac/matrixlib.h
        libaudiofile/alac/ALACDecoder.cpp
        libaudiofile/alac/EndianPortable.c
        libaudiofile/alac/ag_enc.c
        libaudiofile/alac/dplib.h
        ${audiofile_HDRS}
        ${CMAKE_CURRENT_BINARY_DIR}/libaudiofile/config.h
)


add_library (libaudiofile_shared SHARED ${libaudiofile_src} )

SET_TARGET_PROPERTIES (libaudiofile_shared  PROPERTIES CLEAN_DIRECT_OUTPUT 1)


target_include_directories (libaudiofile_shared
        PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
                libaudiofile_shared
                $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/libaudiofile>
        )
        
  
  
#install(
#    TARGETS foo
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib
#    RUNTIME DESTINATION bin
#)




#
# Testing
#

if (BUILD_TESTING_)

  
  enable_testing ()
  ### test_main

  add_executable (test_main
           test/ADPCM.cpp
           test/instparamwrite.c
           test/AES.cpp
           test/Marker.cpp
           test/printmarkers.c
           test/ALAC.cpp
           test/Miscellaneous.cpp
           test/query2.c
           test/ChannelMatrix.cpp
           test/NeXT.cpp
           test/sixteen-to-eight.c
           test/Error.cpp
           test/PCMData.cpp
           test/testaupv.c
           test/FLAC.cpp
           test/PCMMapping.cpp
           test/testchannelmatrix.c
           test/FloatToInt.cpp
           test/Pipe.cpp
           test/testdouble.c
           test/FormatWithChannels.cpp
           test/Query.cpp
           test/testfloat.c
           test/Identify.cpp
           test/SampleFormat.cpp
           test/testmarkers.c
           test/Instrument.cpp
           test/Seek.cpp
           test/twentyfour.c
           test/IntToFloat.cpp
           test/Sign.cpp
           test/twentyfour2.c
           test/InvalidCompressionFormat.cpp
           test/TestUtilities.cpp
           test/writealaw.c
           test/InvalidSampleFormat.cpp
           test/TestUtilities.h
           test/writeraw.c
           test/Large.cpp
           test/VirtualFile.cpp
           test/writeulaw.c
           test/Loop.cpp
           test/floatto24.c
           test/Lossless.h
           test/instparamtest.c
    )

    target_include_directories (test_main
                PUBLIC
                        src
                        ${CMAKE_CURRENT_BINARY_DIR}/libaudiofile
                        ${CMAKE_CURRENT_BINARY_DIR}/test
                )
        target_link_libraries (test_main PRIVATE libaudiofile_shared)
        if (MSVC)
                target_compile_definitions (test_main PRIVATE _USE_MATH_DEFINES)
        endif ()
        add_test (test_main test_main)


endif ()




#
# Programs
#
if (BUILD_COMMANDS)


# libaudiofile-info

    add_executable (sfinfo
                sfcommands/sfinfo.c
                sfcommands/printinfo.c
                sfcommands/printinfo.h
                )

    target_link_libraries (sfinfo
                PRIVATE
                libaudiofile_shared
                $<$<BOOL:${LIBM_REQUIRED}>:m>
                )
                
         
# sfconvert

        add_executable (sfconvert
                    sfcommands/sfconvert.c
                    sfcommands/printinfo.c
                    sfcommands/printinfo.h
             )
        
        target_link_libraries (sfconvert
                PRIVATE
                libaudiofile_shared
                $<$<BOOL:${LIBM_REQUIRED}>:m>
            )

endif ()

