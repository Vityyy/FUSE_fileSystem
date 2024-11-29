
# Debe existir un directorio llamado 'example_dir', con cualquier contenido dentro

echo -n "11_non_empty_dir_deletion_with_rm - "


cp -r example_dir tmp
rm -r example_dir

ls -1 | grep -q example_dir

if [ $? -ne 0 ]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi

cp -r tmp example_dir
rm -r tmp
