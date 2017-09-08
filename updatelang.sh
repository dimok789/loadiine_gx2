#! /bin/bash
#
find . -iname "*.cpp" -or -iname "*.c" | xargs xgettext --no-wrap -ktr -ktrNOOP -o languages/english.lang -j --no-location --omit-header
echo "Updated lang"
find . -iname "*.cpp" -or -iname "*.c" | xargs xgettext --no-wrap -ktr -ktrNOOP -o languages/english.lang -j --omit-header --sort-by-file


for fn in `find languages/*.lang`; do
    if [ "$fn" != "languages/english.lang" ]; then
        echo "Updated $fn"
        msgmerge --output-file=$fn $fn languages/english.lang  --no-wrap
    fi
    
done
