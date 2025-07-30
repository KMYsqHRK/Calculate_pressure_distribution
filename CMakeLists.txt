cmake_minimum_required(VERSION 3.16)
project(PressureDistSolver)

# C++17を使用
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# third_party/eigenディレクトリの存在確認
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/third_party/eigen/CMakeLists.txt")
    message(FATAL_ERROR "Eigen library not found in third_party/eigen directory. Please run 'git submodule update --init --recursive'")
endif()

# EigenをCMakeプロジェクトに追加
add_subdirectory(third_party/eigen)

# すべてのソースファイルを追加
set(SOURCES
    src/main.cpp
    src/pressuredistsolver.cpp
    src/csv_reader.cpp
)

# すべてのヘッダーファイルを追加
set(HEADERS
    src/pressuredistsolver.hpp
    src/csv_reader.hpp
)

# 実行ファイルを作成
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Eigenライブラリをリンク
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen)

# コンパイラ警告を有効化
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# デバッグ情報を含める
set(CMAKE_BUILD_TYPE Debug)

# インストール設定（オプション）
install(TARGETS ${PROJECT_NAME} DESTINATION bin)