file(GLOB_RECURSE project_sources "*.c")

idf_component_register(SRCS ${project_sources}
                       INCLUDE_DIRS "." "ui" "hardware" "backend" "frontend"
                       REQUIRES esp_lcd driver lvgl)

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)