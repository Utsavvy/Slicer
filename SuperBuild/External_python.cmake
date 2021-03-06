
set(proj python)

# Set dependency list
set(${proj}_DEPENDENCIES zlib)
if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_python)
  list(APPEND ${proj}_DEPENDENCIES CTKAPPLAUNCHER)
endif()
if(Slicer_USE_PYTHONQT_WITH_TCL)
  if(WIN32)
    list(APPEND ${proj}_DEPENDENCIES tcl)
  else()
    list(APPEND ${proj}_DEPENDENCIES tcl tk)
  endif()
endif()
if(PYTHON_ENABLE_SSL)
  list(APPEND ${proj}_DEPENDENCIES OpenSSL)
endif()
# if(${CMAKE_PROJECT_NAME}_USE_CONDA_${proj})
# 	list(APPEND ${proj}_DEPENDENCIES python-conda)
# endif()



# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(PYTHON_INCLUDE_DIR CACHE)
  unset(PYTHON_LIBRARY CACHE)
  unset(PYTHON_EXECUTABLE CACHE)
  find_package(PythonLibs REQUIRED)
  find_package(PythonInterp REQUIRED)
  set(PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIRS})
  set(PYTHON_LIBRARY ${PYTHON_LIBRARIES})
  set(PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE})
endif()

#################### 
## This code is working if the variable PYTHONHOME is set in the terminal, reusing previously installed Miniconda
#################### 
# if (${CMAKE_PROJECT_NAME}_USE_CONDA_${proj})  
# 	set (CONDA_VERSION "python-miniconda")
	
# 	# Install miniconda if needed
# 	if (APPLE)      
# 	  set (CONDA_HOME ${CMAKE_BINARY_DIR}/${CONDA_VERSION})
	  
# 	  if (NOT IS_DIRECTORY "${CONDA_HOME}")
# 	    message("Installing ${CONDA_VERSION}...")
	
# 	    # Create directory to store the installation script
# 	    file(MAKE_DIRECTORY ${CONDA_HOME}-install)
# 	    set(CONDA_SCRIPT ${CONDA_HOME}-install/${CONDA_VERSION}.sh)
	  
# 	    # Download script        
# 	    file (DOWNLOAD http://repo.continuum.io/miniconda/Miniconda-3.8.3-MacOSX-x86_64.sh ${CONDA_SCRIPT} SHOW_PROGRESS)
	
# 	    # Execute the batch mode installation        
# 	    execute_process(COMMAND bash ${CONDA_SCRIPT} -f -b -p ${CONDA_HOME})
	
# 			# Fix rpath 
# 			message("COMMAND install_name_tool -id ${CONDA_HOME}/lib/libpython2.7.dylib ${CONDA_HOME}/lib/libpython2.7.dylib")
# 			execute_process(COMMAND install_name_tool -id ${CONDA_HOME}/lib/libpython2.7.dylib ${CONDA_HOME}/lib/libpython2.7.dylib)
			
# 		  message("${CONDA_VERSION} installed")             
# 		endif()
		  
# 	  # Set python variables
# 	  set(PYTHON_INCLUDE_DIR ${CONDA_HOME}/include/python2.7)
# 	  set(PYTHON_LIBRARY ${CONDA_HOME}/lib/libpython2.7.dylib)  
# 	  set(PYTHON_EXECUTABLE ${CONDA_HOME}/bin/python)   
# 	endif() 
# endif()

######
## This code is not building because of the libsqlite3.0.dylib library, 
## so we remove it right after installing Miniconda 
######
if((NOT DEFINED PYTHON_INCLUDE_DIR
    OR NOT DEFINED PYTHON_LIBRARY
    OR NOT DEFINED PYTHON_EXECUTABLE) 
  AND (NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} OR ${CMAKE_PROJECT_NAME}_USE_CONDA_${proj}))
    
  if (${CMAKE_PROJECT_NAME}_USE_CONDA_${proj})  	
    set (python_DIR ${CMAKE_BINARY_DIR}/python-conda)    
    file(MAKE_DIRECTORY ${python_DIR})
    
    # Choose install file depending on the platform version
    # TODO: host this file in a controlled repository
    if (UNIX)      
      if (APPLE)
        set (PYTHON-INSTALL-FILE "https://www.dropbox.com/s/j68c3b9rhmvgqwv/Miniconda-Mac.tar.gz")
      else()
        set (PYTHON-INSTALL-FILE "https://www.dropbox.com/s/1xcwejg40u6b5mu/Miniconda-linux.tar.gz")
      endif()
      set (INSTALL_COMMAND bash ${python_DIR}-install/Miniconda-install.sh -f -b -p ${python_DIR})
    else()
      # Windows
      set (PYTHON-INSTALL-FILE "https://www.dropbox.com/s/34n1771as727pj0/Miniconda-Windows.tar.gz")
      set (INSTALL_COMMAND ${python_DIR}-install/Miniconda-3.8.3-Windows-x86_64.exe /InstallationType=AllUsers /S /D=${python_DIR})
    endif()


    # TODO: modify file names
    ExternalProject_Add(${proj}
      URL ${PYTHON-INSTALL-FILE}
      #URL_MD5 "d3ad8868836e177ee4f9bd8bbd0c827a"     
      DOWNLOAD_DIR ${python_DIR}-install
      SOURCE_DIR ${python_DIR}-install      
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      #INSTALL_COMMAND bash ${python_DIR}-install/Miniconda-install.sh -f -b -p ${python_DIR}
      INSTALL_COMMAND ${INSTALL_COMMAND}
    )               
      
    if (APPLE)
      ExternalProject_Add_Step(${proj} fix_rpath
        COMMAND install_name_tool -id ${python_DIR}/lib/libpython2.7.dylib ${python_DIR}/lib/libpython2.7.dylib
        DEPENDEES install
      ) 
  
      # hack: rename libsqlite3.dylib in order that CTK is building. Apparently is not needed, but it is referenced 
      # from /System/Library/Frameworks/CoreData.framework/Versions/A/CoreData, neccesary to build CTK
      ExternalProject_Add_Step(${proj} rename_libsqlite
        COMMAND mv ${python_DIR}/lib/libsqlite3.dylib ${python_DIR}/lib/libsqlite3.dylib.bak
        DEPENDEES install
      )
    endif()

    # Test: install numpy (requires to disable Slicer Numpy).... OK  
    ExternalProject_Add_Step(${proj} installNumpy
      COMMAND ${python_DIR}/bin/conda install --yes --quiet numpy  
      DEPENDEES install
    )
    
    # Test: install scipy... OK
    # ExternalProject_Add_Step(${proj} installSciPy
    #   COMMAND ${python_DIR}/bin/conda install --yes --quiet scipy
    #   DEPENDEES installNumpy
    # )	
    
    ExternalProject_Add_Step(${proj} installSciPy
      COMMAND ${python_DIR}/bin/conda install --yes --quiet scikit-learn
      DEPENDEES installNumpy
    )	
    
  else()
    # Regular Slicer behaviour
    # Download Python source
    set(python_SOURCE_DIR "${CMAKE_BINARY_DIR}/Python-2.7.3")
  
    set(EXTERNAL_PROJECT_OPTIONAL_ARGS)
    if(MSVC)
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
        PATCH_COMMAND ${CMAKE_COMMAND}
          -DPYTHON_SRC_DIR:PATH=${python_SOURCE_DIR}
          -P ${CMAKE_CURRENT_LIST_DIR}/${proj}_patch.cmake
        )
    endif()
  
    ExternalProject_Add(python-source
      URL "https://www.python.org/ftp/python/2.7.3/Python-2.7.3.tgz"
      URL_MD5 "2cf641732ac23b18d139be077bd906cd"
      DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
      SOURCE_DIR ${python_SOURCE_DIR}
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      )
  
    if(NOT DEFINED git_protocol)
      set(git_protocol "git")
    endif()
  
    set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS CMAKE_ARGS)
  
    if(Slicer_USE_PYTHONQT_WITH_TCL)
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
        -DTCL_LIBRARY:FILEPATH=${TCL_LIBRARY}
        -DTCL_INCLUDE_PATH:PATH=${CMAKE_CURRENT_BINARY_DIR}/tcl-build/include
        -DTK_LIBRARY:FILEPATH=${TK_LIBRARY}
        -DTK_INCLUDE_PATH:PATH=${CMAKE_CURRENT_BINARY_DIR}/tcl-build/include
        )
    endif()
  
    if(PYTHON_ENABLE_SSL)
      list_to_string(${EP_LIST_SEPARATOR} "${OPENSSL_LIBRARIES}" EP_OPENSSL_LIBRARIES)
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
        -DOPENSSL_INCLUDE_DIR:PATH=${OPENSSL_INCLUDE_DIR}
        -DOPENSSL_LIBRARIES:STRING=${EP_OPENSSL_LIBRARIES}
        )
    endif()
  
 
    if(APPLE)    
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
        -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON
        )
    endif()
  
    set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
  
    # Force Python build to "Release".
    if(CMAKE_CONFIGURATION_TYPES)
      set(SAVED_CMAKE_CFG_INTDIR ${CMAKE_CFG_INTDIR})
      set(CMAKE_CFG_INTDIR "Release")
    else()
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=Release)
    endif()
  
    ExternalProject_Add(${proj}
      ${${proj}_EP_ARGS}
      GIT_REPOSITORY "${git_protocol}://github.com/davidsansome/python-cmake-buildsystem.git"
      GIT_TAG "ee53f791f5f5057a36e4277dc7e00d5350a0d234"
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
      BINARY_DIR ${proj}-build
      CMAKE_CACHE_ARGS
        -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
        #-DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags} # Not used
        -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/${proj}-install
        #-DBUILD_TESTING:BOOL=OFF
        -DBUILD_SHARED:BOOL=ON
        -DBUILD_STATIC:BOOL=OFF
        -DUSE_SYSTEM_LIBRARIES:BOOL=OFF
        -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
        -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
        -DENABLE_TKINTER:BOOL=${Slicer_USE_PYTHONQT_WITH_TCL}
        -DENABLE_SSL:BOOL=${PYTHON_ENABLE_SSL}
        ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS}
      DEPENDS
        python-source ${${proj}_DEPENDENCIES}
      )
      
    set(python_DIR ${CMAKE_BINARY_DIR}/${proj}-install)		
	endif()
  

  if(UNIX)
    set(python_IMPORT_SUFFIX so)
    if(APPLE)
      set(python_IMPORT_SUFFIX dylib)
    endif()
    set(slicer_PYTHON_SHARED_LIBRARY_DIR ${python_DIR}/lib)
    set(PYTHON_INCLUDE_DIR ${python_DIR}/include/python2.7)
    set(PYTHON_LIBRARY ${python_DIR}/lib/libpython2.7.${python_IMPORT_SUFFIX})
    set(PYTHON_EXECUTABLE ${python_DIR}/bin/SlicerPython)
    set(slicer_PYTHON_REAL_EXECUTABLE ${python_DIR}/bin/python)
  elseif(WIN32)
    set(slicer_PYTHON_SHARED_LIBRARY_DIR ${python_DIR}/bin)
    set(PYTHON_INCLUDE_DIR ${python_DIR}/include)
    set(PYTHON_LIBRARY ${python_DIR}/libs/python27.lib)
    set(PYTHON_EXECUTABLE ${python_DIR}/bin/SlicerPython.exe)
    set(slicer_PYTHON_REAL_EXECUTABLE ${python_DIR}/bin/python.exe)
  else()
    message(FATAL_ERROR "Unknown system !")
  endif()

  if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_python)

    # Configure python launcher
    configure_file(
      SuperBuild/python_customPython_configure.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/python_customPython_configure.cmake
      @ONLY)
    set(python_customPython_configure_args)

    if(PYTHON_ENABLE_SSL)
      set(python_customPython_configure_args -DOPENSSL_EXPORT_LIBRARY_DIR:PATH=${OPENSSL_EXPORT_LIBRARY_DIR})
    endif()

    ExternalProject_Add_Step(${proj} python_customPython_configure
      COMMAND ${CMAKE_COMMAND} ${python_customPython_configure_args}
        -P ${CMAKE_CURRENT_BINARY_DIR}/python_customPython_configure.cmake
      DEPENDEES install
      )

  endif()

  if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CFG_INTDIR ${SAVED_CMAKE_CFG_INTDIR}) # Restore CMAKE_CFG_INTDIR
  endif()

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(_lib_subdir lib)
  if(WIN32)
    set(_lib_subdir bin)
  endif()

  # library paths
  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${python_DIR}/${_lib_subdir})
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  # paths
  set(${proj}_PATHS_LAUNCHER_BUILD ${python_DIR}/bin)
  mark_as_superbuild(
    VARS ${proj}_PATHS_LAUNCHER_BUILD
    LABELS "PATHS_LAUNCHER_BUILD"
    )
  
  # pythonpath
  set(_pythonhome ${CMAKE_BINARY_DIR}/python-install)
  set(pythonpath_subdir lib/python2.7)
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(pythonpath_subdir Lib)
  endif()

  set(${proj}_PYTHONPATH_LAUNCHER_BUILD
    ${_pythonhome}/${pythonpath_subdir}
    ${_pythonhome}/${pythonpath_subdir}/lib-dynload
    ${_pythonhome}/${pythonpath_subdir}/site-packages
    )
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
    LABELS "PYTHONPATH_LAUNCHER_BUILD"
    )

  # environment variables
  set(${proj}_ENVVARS_LAUNCHER_BUILD "PYTHONHOME=${python_DIR}")
  mark_as_superbuild(
    VARS ${proj}_ENVVARS_LAUNCHER_BUILD
    LABELS "ENVVARS_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  # library paths
  if(UNIX)
    # On windows, python libraries are installed along with the executable
    set(${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED <APPLAUNCHER_DIR>/lib/Python/lib)
    mark_as_superbuild(
      VARS ${proj}_LIBRARY_PATHS_LAUNCHER_INSTALLED
      LABELS "LIBRARY_PATHS_LAUNCHER_INSTALLED"
      )
  endif()

  # pythonpath
  set(${proj}_PYTHONPATH_LAUNCHER_INSTALLED
    <APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}
    <APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/lib-dynload
    <APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages
    )
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_INSTALLED
    LABELS "PYTHONPATH_LAUNCHER_INSTALLED"
    )

  # environment variables
  set(${proj}_ENVVARS_LAUNCHER_INSTALLED "PYTHONHOME=<APPLAUNCHER_DIR>/lib/Python")
  mark_as_superbuild(
    VARS ${proj}_ENVVARS_LAUNCHER_INSTALLED
    LABELS "ENVVARS_LAUNCHER_INSTALLED"
    )

else()
	if (NOT ${CMAKE_PROJECT_NAME}_USE_CONDA_${proj})
  	ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
  endif()
endif()

mark_as_superbuild(
  VARS
    PYTHON_EXECUTABLE:FILEPATH
    PYTHON_INCLUDE_DIR:PATH
    PYTHON_LIBRARY:FILEPATH
  LABELS "FIND_PACKAGE"
  )

ExternalProject_Message(${proj} "PYTHON_EXECUTABLE:${PYTHON_EXECUTABLE}")
ExternalProject_Message(${proj} "PYTHON_INCLUDE_DIR:${PYTHON_INCLUDE_DIR}")
ExternalProject_Message(${proj} "PYTHON_LIBRARY:${PYTHON_LIBRARY}")

if(WIN32)
  set(PYTHON_DEBUG_LIBRARY ${PYTHON_LIBRARY})
  mark_as_superbuild(VARS PYTHON_DEBUG_LIBRARY LABELS "FIND_PACKAGE")
  ExternalProject_Message(${proj} "PYTHON_DEBUG_LIBRARY:${PYTHON_DEBUG_LIBRARY}")
endif()

#!
#! ExternalProject_PythonModule_InstallTreeCleanup(<proj> <modname> "[<dirname1>;[<dirname2>;[...]]]"))
#!
#! Add post-install cleanup step to project <proj>. For each <dirname>, this step will
#! import the module <modname> and delete the <dirname> folder located in the module
#! directory.
#!
#! This function is particularly useful to remove option and too long directories
#! from python module install tree. This function was first developer to address
#! issue #3749.
#!
function(ExternalProject_PythonModule_InstallTreeCleanup proj modname dirnames)
  ExternalProject_Get_Property(${proj} tmp_dir)
  set(_file "${tmp_dir}/${proj}_install_tree_cleanup.py")
  set(_content
"import ${modname}
import os.path
import shutil
")
  foreach(dirname ${dirnames})
    set(_content "${_content}
dir=os.path.join(os.path.dirname(${modname}.__file__), '${dirname}')
print('Removing %s' % dir)
shutil.rmtree(dir, True)
print('Removing %s [done]' % dir)
")
  endforeach()
  file(WRITE ${_file} ${_content})

  ExternalProject_Add_Step(${proj} install_tree_cleanup
    COMMAND ${PYTHON_EXECUTABLE} ${_file}
    COMMENT "Performing install tree cleanup for '${proj}'"
    DEPENDEES install
    )
endfunction()
