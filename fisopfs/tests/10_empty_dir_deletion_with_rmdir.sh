
# Debe existir un directorio vacío llamado 'example_empty_dir'

echo -n "10_empty_dir_deletion_with_rmdir - "


rmdir example_empty_dir

ls -1 | grep -q example_empty_dir

if [ $? -ne 0 ]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi

mkdir example_empty_dir
