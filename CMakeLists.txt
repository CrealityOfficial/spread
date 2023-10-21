__enable_cxx17()

__cc_find(trimesh2)
__assert_target(trimesh2)

__cc_find(Eigen)
__assert_target(eigen)
	
set(ILIBS trimesh2) 
set(LIBS eigen) 

__files_group(${CMAKE_CURRENT_SOURCE_DIR}/spread/ SRCS1)
__files_group(${CMAKE_CURRENT_SOURCE_DIR}/internal/ SRCS2)

set(SRCS ${SRCS1} ${SRCS2})		 
set(INCS ${CMAKE_CURRENT_SOURCE_DIR})
set(DEFS BOOST_ALL_NO_LIB)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR})

include(Slice3rBase.cmake)

__add_common_library(spread OPENMP)
								