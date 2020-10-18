# SDL build platform includes three necessary components:
# OS, Architecture and SDK. Build parameters and sources code base
# could be varied depends on these three components combination.
#
# Platform helpers provide functionality to get supported by SDL
# OS, Architecture or SDK lists and specified for actual build these
# variables values. Values could be passed to cmake as parameters
# (f.e. "-DOS_WINDOWS=1 -DSDK_QT=1"), otherwise they will be detected
# related to current build environment.
#
# Full list of supported cmake flags:
# OS_POSIX
# OS_WINDOWS
# ARCH_X86
# ARCH_X64
# SDK_QT

function(get_supported_os OS_LIST)
  set(${OS_LIST} "posix" "win" PARENT_SCOPE)
endfunction()

function(get_supported_arch ARCH_LIST)
  set(${ARCH_LIST} "x86" "x64" PARENT_SCOPE)
endfunction()

function(get_supported_sdk SDK_LIST)
  set(${SDK_LIST} "qt" PARENT_SCOPE)
endfunction()

function(get_os OS)
  if(OS_POSIX)
    set(${OS} "posix" PARENT_SCOPE)
  elseif(OS_WINDOWS)
    set(${OS} "win" PARENT_SCOPE)
  else()
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      set(${OS} "posix" PARENT_SCOPE)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
      set(${OS} "win" PARENT_SCOPE)
    else()
      message(FATAL_ERROR "Unsupported operation system")
    endif()
  endif()
endfunction()

function(get_arch ARCH)
  if( CMAKE_SIZEOF_VOID_P MATCHES 8 )
    # void ptr = 8 byte --> x86_64
    set(${ARCH} "x64" PARENT_SCOPE)
  elseif( CMAKE_SIZEOF_VOID_P MATCHES 4 )
    # void ptr = 4 byte --> x86
    set(${ARCH} "x86" PARENT_SCOPE)
  else()
      message(FATAL_ERROR "Unsupported architecture")
  endif()
endfunction(get_arch ARCH)

function(get_sdk SDK)
  if(SDK_QT)
    set(${SDK} "qt" PARENT_SCOPE)
  endif()
endfunction()
