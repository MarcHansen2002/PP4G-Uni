# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/MarcH/Desktop/Projects/Uni/Masters/PP4G/PP4G-Uni/cmake/_deps/glm-src"
  "C:/Users/MarcH/Desktop/Projects/Uni/Masters/PP4G/PP4G-Uni/cmake/_deps/glm-build"
  "C:/Users/MarcH/Desktop/Projects/Uni/Masters/PP4G/PP4G-Uni/cmake/_deps/glm-subbuild/glm-populate-prefix"
  "C:/Users/MarcH/Desktop/Projects/Uni/Masters/PP4G/PP4G-Uni/cmake/_deps/glm-subbuild/glm-populate-prefix/tmp"
  "C:/Users/MarcH/Desktop/Projects/Uni/Masters/PP4G/PP4G-Uni/cmake/_deps/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp"
  "C:/Users/MarcH/Desktop/Projects/Uni/Masters/PP4G/PP4G-Uni/cmake/_deps/glm-subbuild/glm-populate-prefix/src"
  "C:/Users/MarcH/Desktop/Projects/Uni/Masters/PP4G/PP4G-Uni/cmake/_deps/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/MarcH/Desktop/Projects/Uni/Masters/PP4G/PP4G-Uni/cmake/_deps/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/MarcH/Desktop/Projects/Uni/Masters/PP4G/PP4G-Uni/cmake/_deps/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
