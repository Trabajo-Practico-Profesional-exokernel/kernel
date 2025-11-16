#!/bin/bash

res_imports=""
res_arr_entry=""
defines_index=""
apps_names=""

count=0

REGISTERED_APPS=""

# first build apps that have many .c files i.e have their own folder....
for app_dir in apps/*/; do

    app_name=$(basename $app_dir)
	if [[ "$app_name" == "build" ]];then
		continue
	fi
    REGISTERED_APPS+=" $app_name"
done


### Then take the mono .c file like hello_world
for app_file in apps/*.c; do
    app_name=$(basename "$app_file" .c)
    REGISTERED_APPS+=" $app_name"
done

REGISTERED_APPS="${REGISTERED_APPS:1}"

for app_name in $REGISTERED_APPS;do
    echo "GOT REGISTERED APP '$app_name' ind: $count"
    prefix="_binary_apps_build_${app_name}_app_bin"

    res_imports+="extern char ${prefix}_start[],${prefix}_size[];\n"
    res_arr_entry+="    {${prefix}_start, (size_t) ${prefix}_size},\n"

    defines_index+="#define APP_IND_${app_name^^} $count\n"

    apps_names+="\"$app_name\",\n"

    count=$(( count + 1 ))
done

###
### GENERATE apps_info.h file!
###

# Define the path to your template file
TEMPLATE_FILE="meta/templates/apps_info.h"
OUTPUT_FILE="meta/gen/meta/apps_info.h"

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
TEMPLATE_FILE="meta/templates/apps_meta.c"
OUTPUT_FILE="meta/gen/apps_meta.c"

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

TEMPLATE_FILE="meta/templates/app_names.c"
OUTPUT_FILE="meta/user_gen/app_names.c"

# Read the template file into a variable
template=$(<"$TEMPLATE_FILE")
# Replace the placeholders with the dynamically generated content
template=$(echo "$template" | sed "s|{APP_NAMES}|$apps_names|g")

# Write the final result to the output file
echo "$template" > "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE with the app names."


###
### Also create a .h file with the count of generated apps
###

OUTPUT_FILE="meta/user_gen/app_names.h"

# Write the final result to the output file
echo "#define APP_COUNT $count" > "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE with the apps info."
