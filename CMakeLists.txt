cmake_minimum_required(VERSION 3.16)
project(ShelfLock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PostgreSQL REQUIRED)

add_executable(shelflock
    src/Logger.cpp
    src/Metrics.cpp
    src/OrderState.cpp
    src/Fulfillment.cpp
    src/HoldHeap.cpp
    src/Repository.cpp
    src/DemandForecast.cpp
    src/ReservationService.cpp
    src/main.cpp
)

target_include_directories(shelflock PRIVATE include ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(shelflock PRIVATE ${PostgreSQL_LIBRARIES})
