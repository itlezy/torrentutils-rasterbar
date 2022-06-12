project ("libtorrent-metadata-downloader")

# CMakeList.txt : CMake project for CMakeProject1, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_FLAGS_RELEASE "/MT")

find_package(Threads REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(LibtorrentRasterbar CONFIG REQUIRED)

# Add source to this project's executable.
#add_executable (ltmt "libtorrent-metadata-downloader.cpp")
add_executable (ltmt "bt-get2.cpp")

# TODO: Add tests and install targets if needed.

target_link_libraries(ltmt PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(ltmt PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(ltmt PRIVATE LibtorrentRasterbar::torrent-rasterbar)
