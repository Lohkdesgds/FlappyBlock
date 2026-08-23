include(FetchContent)

message(STATUS "Declaring SDL3 and dependencies...")

# Declare SDL3
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-3.4.14
)

# Declare SDL3_image with explicit cleanup handling
FetchContent_Declare(
    SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG        release-3.4.4
)

# Declare SDL3_ttf
FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-3.2.2 
)

message(STATUS "Setting SDL3 flags...")

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_VENDORED ON CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)

set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TIFF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)

set(BUILD_SHARED_LIBS OFF)

message(STATUS "Making SDL3 available...")
FetchContent_MakeAvailable(SDL3)
message(STATUS "Making SDL3_image available...")
FetchContent_MakeAvailable(SDL3_image)
message(STATUS "Making SDL3_ttf available...")
FetchContent_MakeAvailable(SDL3_ttf)

# Re-enable it for other libs to have their own settings
set(BUILD_SHARED_LIBS ON)

message(STATUS "SDL3 is complete.")

# target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
#target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)