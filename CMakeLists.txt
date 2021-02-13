cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(
	praxis
	DESCRIPTION "http server based on UNIX philosophy"
	HOMEPAGE_URL "github.com/jaenek/praxis"
)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE c_std_11)

add_library(mongoose INTERFACE)
target_include_directories(mongoose INTERFACE vendor/mongoose)

file(GLOB PROJECT_SOURCES
	vendor/mongoose/mongoose.c
	main.c
)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE project_options mongoose)