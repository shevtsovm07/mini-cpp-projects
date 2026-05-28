add_executable(
    hamarc
    main.cpp
    ../lib/archive.cpp
)

target_compile_features(hamarc PRIVATE cxx_std_20)
