#!/usr/bin/env bash
set -e
ROOT=..
META_FOLDER=$ROOT/meta

TEMPLATES_FOLDER=$ROOT/meta/templates
USER_META_FOLDER=$ROOT/user/meta

res_imports=""
res_arr_entry=""
defines_index=""
apps_names=""
def_executables=""

count=0
count_executables=0

COUNT_DEF_APPS="$1"
shift   # remove COUNT_DEF_APPS from $@
# COUNT_DEF_APPS="0"

REGISTERED_APPS=("$@")   # array of app names

echo "Generate metadata with $COUNT_DEF_APPS def executables"

for app_name in "${REGISTERED_APPS[@]}";do
    # Prefijo limpio coincidente con la copia temporal
    prefix="_binary_${app_name}_app_bin"

    res_imports+="extern char ${prefix}_start[],${prefix}_size[];\n"
    res_arr_entry+="    {${prefix}_start, (size_t) ${prefix}_size},\n"

    defines_index+="#define APP_IND_${app_name^^} $count\n"

    apps_names+="\"$app_name\",\n"

    # -------------------------
    # DEF-APP-ONLY LOGIC
    # -------------------------

    if (( count < COUNT_DEF_APPS )); then
        def_executables+="\"$app_name\",\n"
        echo "ADD META FOR ind: $count DEF APP '$app_name'"
        count_executables=$(( count_executables + 1 ))
    else
        echo "ADD META FOR ind: $count APP '$app_name'"
    fi
    count=$(( count + 1 ))
done

defines_index+="#define APP_COUNT $count\n"

###
### GENERATE apps_info.h file!
###

# Define the path to your template file
TEMPLATE_FILE="$TEMPLATES_FOLDER/apps_info.h"
OUTPUT_FILE="$META_FOLDER/gen/meta/apps_info.h"

# Read the template file into a variable
template=$(<"$TEMPLATE_FILE")
# Replace the placeholders with the dynamically generated content

template=$(echo "$template" | sed "s|{DEFINES_APPS_INDEXS}|$defines_index|g")

# Write the final result to the output file
echo "$template" > "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE with the apps indexs defines."



###
### GENERATE .c file!
###

# Define the path to your template file
TEMPLATE_FILE="$TEMPLATES_FOLDER/apps_meta.c"
OUTPUT_FILE="$META_FOLDER/gen/apps_meta.c"

# Read the template file into a variable
template=$(<"$TEMPLATE_FILE")

# Replace the placeholders with the dynamically generated content
template=$(echo "$template" | sed "s|{IMPORTS_APPS_DEF}|$res_imports|g")
template=$(echo "$template" | sed "s|{IMPORTS_APPS_ENTRIES}|$res_arr_entry|g")

# Write the final result to the output file
echo "$template" > "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE with the apps information."



###
### Lastly generate... the app names used by shell and user apps in general maybe..
###

TEMPLATE_FILE="$TEMPLATES_FOLDER/app_names.c"
OUTPUT_FILE="$USER_META_FOLDER/app_names.c"

# Read the template file into a variable
template=$(<"$TEMPLATE_FILE")
# Replace the placeholders with the dynamically generated content
template=$(echo "$template" | sed "s|{APP_NAMES}|$apps_names|g")

template=$(echo "$template" | sed "s|{DEF_EXECUTABLES}|$def_executables|g")


# Write the final result to the output file
echo "$template" > "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE with the app names."


###
### Also create a .h file with the count of generated apps
###

OUTPUT_FILE="$USER_META_FOLDER/app_names.h"

# Write the final result to the output file
defs="#define APP_COUNT $count"
echo "$defs" > "$OUTPUT_FILE"
echo "#define EXECUTABLE_COUNT $count_executables" >> "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE with the apps info."
