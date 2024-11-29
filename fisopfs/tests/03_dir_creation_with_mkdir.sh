
# No debe existir new_dir

echo -n "03_dir_creation_with_mkdir - "


cd prueba

mkdir new_dir

ls -1 | grep -q new_dir

if [ $? -eq 0 ]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi

rmdir new_dir
