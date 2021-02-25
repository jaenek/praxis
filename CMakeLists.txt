cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(
	praxis
	DESCRIPTION "http server based on UNIX philosophy"
	HOMEPAGE_URL "github.com/jaenek/praxis"
)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE c_std_11)
target_compile_options(project_options INTERFACE -ggdb -Wall -Werror -pedantic -O0)

add_library(mongoose vendor/mongoose/mongoose.c)
target_include_directories(mongoose PUBLIC vendor/mongoose)
target_compile_definitions(mongoose PUBLIC -D MG_ENABLE_SSI)

add_library(sha256 vendor/sha256/sha256.c)
target_include_directories(sha256 PUBLIC vendor/sha256)

file(GLOB PROJECT_SOURCES
	command.c
	http.c
	mqtt.c
	main.c
)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE project_options mongoose sha256)