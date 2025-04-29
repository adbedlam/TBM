cmake_minimum_required(VERSION 3.28)
project(trade_bot)

set(CMAKE_CXX_STANDARD 20)


if (MSVC)
    add_compile_definitions(_WIN32_WINNT=0x0A00)  # Windows 10+
    add_compile_options(/bigobj)
else ()
    add_compile_options(-Wa,-mbig-obj)
endif ()

find_package(Boost REQUIRED COMPONENTS system)


find_package(OpenSSL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_package(libpqxx CONFIG REQUIRED)

add_executable(trade_bot src/main.cpp
        src/APIread.cpp
        headers/APIread.h
        headers/BinanceWebsocket.h
        src/BinanceWebsocket.cpp
        headers/common.hpp
        headers/BinanceAPI.h
        src/BinanceAPI.cpp
        headers/AccountManager.h
        src/AccountManager.cpp
        headers/OrderManager.h
        src/OrderManager.cpp
        headers/DataBase.h
        src/DataBase.cpp
        headers/INDICATORS/AbstractStrategy.h
        headers/INDICATORS/EMAIndicator.h
        headers/INDICATORS/RSIIndicator.h
        headers/INDICATORS/BBIndicator.h
        headers/INDICATORS/MACDIndicator.h
        src/INDICATORS/EMAIndicator.cpp
        src/INDICATORS/RSIIndicator.cpp
        src/INDICATORS/BBIndicator.cpp
        src/INDICATORS/MACDIndicator.cpp
        src/AnalysisHandler.cpp
        headers/AnalysisHandler.h
        headers/INDICATORS/Supertrend.h
        src/INDICATORS/Supertrend.cpp
)

target_include_directories(trade_bot PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/headers" )


target_link_libraries(trade_bot PRIVATE ws2_32)
target_link_libraries(trade_bot PRIVATE
        libpqxx::pqxx
        PostgreSQL::PostgreSQL
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
)